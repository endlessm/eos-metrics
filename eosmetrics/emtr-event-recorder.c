/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */

/* Copyright 2014, 2015 Endless Mobile, Inc. */

/* This file is part of eos-metrics.
 *
 * eos-metrics is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * eos-metrics is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with eos-metrics.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

/* For CLOCK_BOOTTIME */
#if !defined(_POSIX_C_SOURCE) || _POSIX_C_SOURCE < 200112L
#error "This code requires _POSIX_C_SOURCE to be 200112L or later."
#endif

#include "emtr-event-recorder.h"
#include "emer-event-recorder-server.h"
#include "eosmetrics/emtr-aggregate-timer-private.h"
#include "eosmetrics/emtr-util.h"

#include <string.h>
#include <time.h>

#include <gio/gio.h>
#include <glib.h>
#include <glib-object.h>
#include <uuid/uuid.h>

/* Convenience macro to check that @ptr is a #GVariant */
#define _IS_VARIANT(ptr) (g_variant_is_of_type ((ptr), G_VARIANT_TYPE_ANY))

/*
 * The number of elements in a uuid_t. uuid_t is assumed to be a fixed-length
 * array of guchar.
 */
#define UUID_LENGTH (sizeof (uuid_t) / sizeof (guchar))

/**
 * SECTION:emtr-event-recorder
 * @title: Event Recorder
 * @short_description: Records metric events to metric system daemon.
 * @include: eosmetrics/eosmetrics.h
 *
 * The event recorder asynchronously sends metric events to the metric system
 * daemon via D-Bus. The system daemon then delivers metrics to the server on
 * a best-effort basis. No feedback is given regarding the outcome of delivery.
 * The event recorder is thread-safe.
 *
 * This API may be called from JavaScript as follows.
 *
 * |[
 * const EosMetrics = imports.gi.EosMetrics;
 * const GLib = imports.gi.GLib;
 * const MEANINGLESS_EVENT = "fb59199e-5384-472e-af1e-00b7a419d5c2";
 * const MEANINGLESS_AGGREGATED_EVENT = "01ddd9ad-255a-413d-8c8c-9495d810a90f";
 * const MEANINGLESS_EVENT_WITH_AUX_DATA =
 *   "9f26029e-8085-42a7-903e-10fcd1815e03";
 * let eventRecorder = EosMetrics.EventRecorder.get_default();
 * // Records a single instance of MEANINGLESS_EVENT along with the current
 * // time.
 * eventRecorder.record_event(MEANINGLESS_EVENT, null);
 * // Records the fact that MEANINGLESS_AGGREGATED_EVENT occurred for some
 * // duration
 * let timer = eventRecorder.start_aggregate_timer(
 *   MEANINGLESS_AGGREGATED_EVENT, null);
 * timer.stop()
 * // Records MEANINGLESS_EVENT_WITH_AUX_DATA along with some auxiliary data and
 * // the current time.
 * eventRecorder.record_event(MEANINGLESS_EVENT_WITH_AUX_DATA,
 *   new GLib.Variant('a{sv}', {
 *     units_of_smoke_ground: new GLib.Variant('u', units),
 *     grinding_time: new GLib.Variant('u', time)
 *   }););
 * ]|
 *
 * Event submission may be disabled at runtime by setting the
 * `EOS_DISABLE_METRICS` environment variable to the empty string or `1`. This
 * is intended to be set when running unit tests in other modules, for example,
 * to avoid submitting metrics from unit test runs. It will skip submitting
 * metrics to the D-Bus daemon, but otherwise all eos-metrics functions will
 * report success.
 */

typedef struct EmtrEventRecorderPrivate
{
  /*
   * D-Bus doesn't support maybe types, so a boolean is used to indicate whether
   * the auxiliary_payload field should be ignored. A non-NULL auxiliary_payload
   * must be passed even when it will be ignored, and this is the arbitrary
   * variant that is used for that purpose.
   */
  GVariant *empty_auxiliary_payload;

  GHashTable * volatile events_by_id_with_key;
  GMutex events_by_id_with_key_lock;

  gboolean recording_enabled;

  EmerEventRecorderServer *dbus_proxy;
} EmtrEventRecorderPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (EmtrEventRecorder, emtr_event_recorder, G_TYPE_OBJECT)

/* Callback to make the finish call after async D-Bus calls */
typedef gboolean (*FinishCallback) (EmerEventRecorderServer *, GAsyncResult *, GError **);

static void
emtr_event_recorder_finalize (GObject *object)
{
  EmtrEventRecorder *self = EMTR_EVENT_RECORDER (object);
  EmtrEventRecorderPrivate *priv =
    emtr_event_recorder_get_instance_private (self);

  g_hash_table_destroy (priv->events_by_id_with_key);
  g_mutex_clear (&priv->events_by_id_with_key_lock);

  g_variant_unref (priv->empty_auxiliary_payload);
  g_clear_object (&priv->dbus_proxy);

  G_OBJECT_CLASS (emtr_event_recorder_parent_class)->finalize (object);
}



/*
 * https://developer.gnome.org/glib/2.40/glib-GVariant.html#g-variant-hash
 * does not work on container types, so we implement our own more general, hash
 * function. Note that the GVariant is trusted to be in fully-normalized form.
 * The implementation is inspired by the GLib implementations of g_str_hash and
 * g_bytes_hash.
 */
static guint
general_variant_hash (gconstpointer key)
{
  GVariant *variant = (GVariant *) key;
  const gchar *type_string = g_variant_get_type_string (variant);
  guint hash_value = g_str_hash (type_string);
  GBytes *serialized_data = g_variant_get_data_as_bytes (variant);
  if (serialized_data != NULL)
    {
      hash_value = (hash_value * 33) + g_bytes_hash (serialized_data);
      g_bytes_unref (serialized_data);
    }
  return hash_value;
}

static void
emtr_event_recorder_class_init (EmtrEventRecorderClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  object_class->finalize = emtr_event_recorder_finalize;
}

static void
emtr_event_recorder_init (EmtrEventRecorder *self)
{
  EmtrEventRecorderPrivate *priv =
    emtr_event_recorder_get_instance_private (self);

  priv->events_by_id_with_key =
    g_hash_table_new_full (general_variant_hash, g_variant_equal,
                           (GDestroyNotify) g_variant_unref,
                           (GDestroyNotify) g_ptr_array_unref);
  g_mutex_init (&priv->events_by_id_with_key_lock);

  GVariant *unboxed_variant = g_variant_new_boolean (FALSE);
  priv->empty_auxiliary_payload = g_variant_new_variant (unboxed_variant);
  g_variant_ref_sink (priv->empty_auxiliary_payload);

  /* If getting the D-Bus connection fails, mark self as a no-op object. */
  GError *error = NULL;
  priv->dbus_proxy =
    emer_event_recorder_server_proxy_new_for_bus_sync (G_BUS_TYPE_SYSTEM,
                                                       G_DBUS_PROXY_FLAGS_NONE,
                                                       "com.endlessm.Metrics",
                                                       "/com/endlessm/Metrics",
                                                       NULL /* GCancellable */,
                                                       &error);
  if (priv->dbus_proxy == NULL)
    {
      g_critical ("Unable to connect to the D-Bus event recorder server: %s",
                  error->message);
      g_error_free (error);
      priv->recording_enabled = FALSE;
      return;
    }

  priv->recording_enabled = TRUE;
}

static gboolean
parse_event_id (const gchar *unparsed_event_id,
                uuid_t       parsed_event_id)
{
  gint parse_failed = uuid_parse (unparsed_event_id, parsed_event_id);
  if (parse_failed != 0)
    {
      g_warning ("Attempt to parse UUID \"%s\" failed. Make sure you created "
                 "this UUID with uuidgen -r. You may need to sudo apt-get "
                 "install uuid-runtime first.", unparsed_event_id);
      return FALSE;
    }

  return TRUE;
}

static GVariant *
get_normalized_form_of_variant (GVariant *variant)
{
  if (variant == NULL)
    return NULL;

  g_variant_ref_sink (variant);
  GVariant *normalized_variant = g_variant_get_normal_form (variant);
  g_variant_unref (variant);
  return normalized_variant;
}

/*
 * Initializes the given uuid_builder and populates it with the contents of
 * uuid.
 */
static void
get_uuid_builder (uuid_t           uuid,
                  GVariantBuilder *uuid_builder)
{
  g_variant_builder_init (uuid_builder, G_VARIANT_TYPE ("ay"));
  for (size_t i = 0; i < UUID_LENGTH; ++i)
    g_variant_builder_add (uuid_builder, "y", uuid[i]);
}

static GVariant *
combine_event_id_with_key (uuid_t    event_id,
                           GVariant *key)
{
  GVariantBuilder event_id_builder;
  get_uuid_builder (event_id, &event_id_builder);
  return g_variant_new ("(aymv)", &event_id_builder, key);
}

static gboolean
contains_maybe_variant (GVariant *variant)
{
  if (variant == NULL)
    return FALSE;

  // type_string belongs to the GVariant and should not be freed.
  const gchar *type_string = g_variant_get_type_string (variant);
  gchar *found_character = strchr (type_string, 'm');
  if (found_character != NULL)
    {
      g_critical ("Maybe type found in auxiliary payload. These are not "
                  "compatible with D-Bus!");
      return TRUE;
    }
  return FALSE;
}

static void
append_event_to_sequence (EmtrEventRecorder *self,
                          GPtrArray         *event_sequence,
                          gint64             relative_time,
                          GVariant          *auxiliary_payload)
{
  EmtrEventRecorderPrivate *priv =
    emtr_event_recorder_get_instance_private (self);

  /* Variants sent to D-Bus are not allowed to be NULL or maybe types. */
  gboolean has_payload = (auxiliary_payload != NULL);
  GVariant *maybe_payload =
    has_payload ? auxiliary_payload : priv->empty_auxiliary_payload;
  GVariant *event =
    g_variant_new ("(xbv)", relative_time, has_payload, maybe_payload);

  g_variant_ref_sink (event);
  g_ptr_array_add (event_sequence, event);
}

/*
 * The callback for the asynchronous D-Bus calls, send_events_to_dbus and
 * send_event_sequence_to_dbus. Calls the finish method.
 * @finish_callback: the finish D-Bus method call, will be called in this
 * function.
 */
static void
send_events_to_dbus_finish_callback (EmerEventRecorderServer *dbus_proxy,
                                     GAsyncResult            *res,
                                     FinishCallback           finish_callback)
{
  GError *error = NULL;
  gboolean success = finish_callback (dbus_proxy, res, &error);

  if (!success)
    {
      g_warning ("Failed to send event to event recorder daemon: %s.",
                 error->message);
      g_error_free (error);
    }
}

/* Check the EOS_DISABLE_METRICS environment variable to see if we should
 * skip submitting any metrics. This is intended to be set when running unit
 * tests in other modules, for example, to avoid submitting metrics from unit
 * test runs. */
static gboolean
disable_event_submission (void)
{
  const gchar *val = g_getenv ("EOS_DISABLE_METRICS");

  return (val != NULL &&
          (g_str_equal (val, "") || g_str_equal (val, "1")));
}

/* Send either singular or aggregate event to D-Bus.
   num_events parameter is ignored if is_aggregate is FALSE. */
static void
send_events_to_dbus (EmtrEventRecorder *self,
                     uuid_t             parsed_event_id,
                     GVariant          *auxiliary_payload,
                     gint64             relative_time,
                     gboolean           is_synchronous,
                     gboolean           is_aggregate,
                     gint               num_events)
{
  EmtrEventRecorderPrivate *priv =
    emtr_event_recorder_get_instance_private (self);

  GVariantBuilder uuid_builder;
  GVariant *event_id_variant;

  /* Variants sent to D-Bus are not allowed to be NULL or maybe types. */
  gboolean has_payload = auxiliary_payload != NULL;
  GVariant *maybe_auxiliary_payload;

  if (disable_event_submission ())
    {
      g_debug ("Skipping submitting %i events as submission is disabled",
               num_events);
      return;
    }

  get_uuid_builder (parsed_event_id, &uuid_builder);
  event_id_variant = g_variant_builder_end (&uuid_builder);
  maybe_auxiliary_payload = has_payload ?
    g_variant_new_variant (auxiliary_payload) : priv->empty_auxiliary_payload;

  if (is_synchronous)
    {
      GError *error = NULL;
      gboolean success;
      if (is_aggregate)
        success =
          emer_event_recorder_server_call_record_aggregate_event_sync (priv->dbus_proxy,
                                                                       getuid (),
                                                                       event_id_variant,
                                                                       num_events,
                                                                       relative_time,
                                                                       has_payload,
                                                                       maybe_auxiliary_payload,
                                                                       NULL /* GCancellable */,
                                                                       &error);
      else
        success =
          emer_event_recorder_server_call_record_singular_event_sync (priv->dbus_proxy,
                                                                      getuid (),
                                                                      event_id_variant,
                                                                      relative_time,
                                                                      has_payload,
                                                                      maybe_auxiliary_payload,
                                                                      NULL /* GCancellable */,
                                                                      &error);

      if (!success)
        {
          g_warning ("Failed to send event to event recorder daemon: %s.",
                     error->message);
          g_error_free (error);
        }
    }
  else
    {
      if (is_aggregate)
        emer_event_recorder_server_call_record_aggregate_event (priv->dbus_proxy,
                                                                getuid (),
                                                                event_id_variant,
                                                                num_events,
                                                                relative_time,
                                                                has_payload,
                                                                maybe_auxiliary_payload,
                                                                NULL /* GCancellable */,
                                                                (GAsyncReadyCallback) send_events_to_dbus_finish_callback,
                                                                (FinishCallback) emer_event_recorder_server_call_record_aggregate_event_finish);
      else
        emer_event_recorder_server_call_record_singular_event (priv->dbus_proxy,
                                                               getuid (),
                                                               event_id_variant,
                                                               relative_time,
                                                               has_payload,
                                                               maybe_auxiliary_payload,
                                                               NULL /* GCancellable */,
                                                               (GAsyncReadyCallback) send_events_to_dbus_finish_callback,
                                                               (FinishCallback) emer_event_recorder_server_call_record_singular_event_finish);
    }
}

/*
 * Sends the corresponding event_sequence GVariant to D-Bus.
 */
static void
send_event_sequence_to_dbus (EmtrEventRecorder *self,
                             GVariant          *event_id,
                             GPtrArray         *event_sequence,
                             gboolean           is_synchronous)
{
  EmtrEventRecorderPrivate *priv =
    emtr_event_recorder_get_instance_private (self);

  GVariantBuilder event_sequence_builder;
  GVariant *event_sequence_variant;

  if (disable_event_submission ())
    {
      g_debug ("Skipping submitting event sequence as submission is disabled");
      return;
    }

  g_variant_builder_init (&event_sequence_builder, G_VARIANT_TYPE ("a(xbv)"));
  for (gint i = 0; i < event_sequence->len; i++)
    {
      GVariant *event = g_ptr_array_index (event_sequence, i);
      g_variant_builder_add_value (&event_sequence_builder, event);
    }

  event_sequence_variant = g_variant_builder_end (&event_sequence_builder);

  if (is_synchronous)
    {
      GError *error = NULL;
      gboolean success =
        emer_event_recorder_server_call_record_event_sequence_sync (priv->dbus_proxy,
                                                                    getuid (),
                                                                    event_id,
                                                                    event_sequence_variant,
                                                                    NULL /* GCancellable */,
                                                                    &error);

      if (!success)
        {
          g_warning ("Failed to send event to event recorder daemon: %s.",
                     error->message);
          g_error_free (error);
        }
    }
  else
    {
      emer_event_recorder_server_call_record_event_sequence (priv->dbus_proxy,
                                                             getuid (),
                                                             event_id,
                                                             event_sequence_variant,
                                                             NULL /* GCancellable */,
                                                             (GAsyncReadyCallback) send_events_to_dbus_finish_callback,
                                                             (FinishCallback) emer_event_recorder_server_call_record_event_sequence_finish);
    }
}

#ifdef DEBUG
/*
 * This is only needed for extra-helpful debug messages. Free the return value
 * with g_free().
 */
static inline gchar *
pretty_print_variant_or_null (GVariant *variant)
{
  if (variant != NULL)
    return g_variant_print (variant, TRUE);
  return g_strdup ("(nil)");
}
#endif /* DEBUG */

static void
record_events (EmtrEventRecorder *self,
               const gchar       *event_id,
               GVariant          *auxiliary_payload,
               gint64             relative_time,
               gboolean           is_synchronous,
               gboolean           is_aggregate,
               gint64             num_events)
{
  EmtrEventRecorderPrivate *priv =
    emtr_event_recorder_get_instance_private (self);

  if (!priv->recording_enabled)
    return;

  uuid_t parsed_event_id;
  if (!parse_event_id (event_id, parsed_event_id))
    return;

  auxiliary_payload = get_normalized_form_of_variant (auxiliary_payload);

  send_events_to_dbus (self,
                       parsed_event_id,
                       auxiliary_payload,
                       relative_time,
                       is_synchronous,
                       is_aggregate,
                       num_events);

  if (auxiliary_payload != NULL)
    g_variant_unref (auxiliary_payload);
}

static void
record_stop (EmtrEventRecorder *self,
             const gchar       *event_id,
             GVariant          *key,
             GVariant          *auxiliary_payload,
             gboolean           is_synchronous)
{
  EmtrEventRecorderPrivate *priv =
    emtr_event_recorder_get_instance_private (self);

  if (!priv->recording_enabled)
    return;

  /* Acquire this lock before getting the time so that event sequences are
     guaranteed to be chronologically sorted. */
  g_mutex_lock (&priv->events_by_id_with_key_lock);

  // Get the time as soon as possible because it will change during execution.
  gint64 relative_time;
  if (!emtr_util_get_current_time (CLOCK_BOOTTIME, &relative_time))
    {
      g_critical ("Getting relative timestamp failed.");
      goto finally;
    }

  uuid_t parsed_event_id;
  if (!parse_event_id (event_id, parsed_event_id))
    goto finally;

  key = get_normalized_form_of_variant (key);

  GVariant *event_id_with_key = combine_event_id_with_key (parsed_event_id,
                                                           key);
  GPtrArray *event_sequence =
    g_hash_table_lookup (priv->events_by_id_with_key, event_id_with_key);

  if (event_sequence == NULL)
    {
      g_variant_unref (event_id_with_key);
      if (key != NULL)
        {
          gchar *key_as_string = g_variant_print (key, TRUE);
          g_variant_unref (key);

          g_warning ("Ignoring request to stop event of type %s with key %s "
                     "because there is no corresponding unstopped start "
                     "event.", event_id, key_as_string);

          g_free (key_as_string);
        }
      else
        {
          g_warning ("Ignoring request to stop event of type %s with NULL key "
                     "because there is no corresponding unstopped start "
                     "event.", event_id);
        }
      goto finally;
    }

  if (key != NULL)
    g_variant_unref (key);

  auxiliary_payload = get_normalized_form_of_variant (auxiliary_payload);

  append_event_to_sequence (self, event_sequence, relative_time,
                            auxiliary_payload);

  if (auxiliary_payload != NULL)
    g_variant_unref (auxiliary_payload);

  GVariant *event_id_variant = g_variant_get_child_value (event_id_with_key, 0);
  send_event_sequence_to_dbus (self, event_id_variant, event_sequence,
                               is_synchronous);
  g_variant_unref (event_id_variant);

  g_assert (g_hash_table_remove (priv->events_by_id_with_key,
                                 event_id_with_key));
  g_variant_unref (event_id_with_key);

finally:
  g_mutex_unlock (&priv->events_by_id_with_key_lock);
}

/* PUBLIC API */

/**
 * emtr_event_recorder_new:
 *
 * Testing function for creating a new #EmtrEventRecorder in the C API.
 * You only need to use this if you are creating an event recorder for use in
 * unit testing.
 *
 * For all normal uses, you should use emtr_event_recorder_get_default()
 * instead.
 *
 * Returns: (transfer full): a new #EmtrEventRecorder.
 * Free with g_object_unref() if using C when done with it.
 */
EmtrEventRecorder *
emtr_event_recorder_new (void)
{
  return g_object_new (EMTR_TYPE_EVENT_RECORDER, NULL);
}

/**
 * emtr_event_recorder_get_default:
 *
 * Gets the event recorder object that you should use to record all metrics.
 *
 * Returns: (transfer none): the default #EmtrEventRecorder.
 * This object is owned by the metrics library; do not free it.
 */
EmtrEventRecorder *
emtr_event_recorder_get_default (void)
{
  static EmtrEventRecorder *singleton = NULL;

  if (g_once_init_enter (&singleton))
    {
      EmtrEventRecorder *retval = g_object_new (EMTR_TYPE_EVENT_RECORDER, NULL);
      g_once_init_leave (&singleton, retval);
    }

  return singleton;
}

/**
 * emtr_event_recorder_record_event:
 * @self: (in): the event recorder
 * @event_id: (in): an RFC 4122 UUID representing the type of event that took
 * place
 * @auxiliary_payload: (allow-none) (in): miscellaneous data to associate with
 * the event. Must not contain maybe variants as they are not compatible with
 * D-Bus.
 *
 * Make a best-effort to record the fact that an event of type @event_id
 * happened at the current time. Optionally, associate arbitrary data,
 * @auxiliary_payload, with this particular instance of the event. Under no
 * circumstances should personally-identifiable information be included in the
 * @auxiliary_payload or @event_id. Large auxiliary payloads dominate the size
 * of the event and should therefore be used sparingly. Events for which precise
 * timing information is not required should instead be recorded using
 * emtr_event_recorder_record_events() to conserve bandwidth.
 *
 * At the discretion of the metrics system, the event may be discarded before
 * being reported to the metrics server. The event may take arbitrarily long to
 * reach the server and may be persisted unencrypted on the client for
 * arbitrarily long. There is no guarantee that the event is delivered via the
 * network; for example, it may instead be delivered manually on a USB drive.
 * No indication of successful or failed delivery is provided, and no
 * application should rely on successful delivery. The event will not be
 * aggregated with other events before reaching the server.
 */
void
emtr_event_recorder_record_event (EmtrEventRecorder *self,
                                  const gchar       *event_id,
                                  GVariant          *auxiliary_payload)
{
  /* Get the time before doing anything else because it will change during
  execution. */
  gint64 relative_time;
  if (!emtr_util_get_current_time (CLOCK_BOOTTIME, &relative_time))
    {
      g_critical ("Getting relative timestamp failed.");
      return;
    }

  g_return_if_fail (EMTR_IS_EVENT_RECORDER (self));
  g_return_if_fail (event_id != NULL);
  g_return_if_fail (auxiliary_payload == NULL ||
                    _IS_VARIANT (auxiliary_payload));

  if (contains_maybe_variant (auxiliary_payload))
    return;

#ifdef DEBUG
  {
    gchar *payload_string = pretty_print_variant_or_null (auxiliary_payload);
    g_debug ("%s: Event ID: %s, payload: %s", G_STRFUNC, event_id,
             payload_string);
    g_free (payload_string);
  }
#else
  {
    g_debug ("%s: Event ID: %s, payload: %p", G_STRFUNC, event_id,
             auxiliary_payload);
  }
#endif /* DEBUG */

  record_events (self, event_id, auxiliary_payload, relative_time,
                 FALSE /* is_synchronous */, FALSE /* is_aggregate */,
                 -1 /* num_events (ignored) */);
}

/**
 * emtr_event_recorder_record_event_sync:
 * @self: (in): the event recorder
 * @event_id: (in): an RFC 4122 UUID representing the type of event that took
 * place
 * @auxiliary_payload: (allow-none) (in): miscellaneous data to associate with
 * the event. Must not contain maybe variants as they are not compatible with
 * D-Bus.
 *
 * Make a best-effort to record the fact that an event of type @event_id
 * occurred at the current time. Behaves like emtr_event_recorder_record_event()
 * but executes synchronously, blocking until either a timeout expires or the
 * event recorder daemon has received the event. Generally prefer
 * emtr_event_recorder_record_event() in UI threads, but use
 * emtr_event_recorder_record_event_sync() instead for the sake of reliability
 * when recording an event from a process that is about to close.
 *
 * Since: 0.4
 */
void
emtr_event_recorder_record_event_sync (EmtrEventRecorder *self,
                                       const gchar       *event_id,
                                       GVariant          *auxiliary_payload)
{
  /* Get the time before doing anything else because it will change during
   * execution.
   */
  gint64 relative_time;
  if (!emtr_util_get_current_time (CLOCK_BOOTTIME, &relative_time))
    {
      g_critical ("Getting relative timestamp failed.");
      return;
    }

  g_return_if_fail (EMTR_IS_EVENT_RECORDER (self));
  g_return_if_fail (event_id != NULL);
  g_return_if_fail (auxiliary_payload == NULL ||
                    _IS_VARIANT (auxiliary_payload));

  if (contains_maybe_variant (auxiliary_payload))
    return;

#ifdef DEBUG
  {
    gchar *payload_string = pretty_print_variant_or_null (auxiliary_payload);
    g_debug ("%s: Event ID: %s, payload: %s", G_STRFUNC, event_id,
             payload_string);
    g_free (payload_string);
  }
#else
  {
    g_debug ("%s: Event ID: %s, payload: %p", G_STRFUNC, event_id,
             auxiliary_payload);
  }
#endif /* DEBUG */

  record_events (self, event_id, auxiliary_payload, relative_time,
                 TRUE /* is_synchronous */, FALSE /* is_aggregate */,
                 -1 /* num_events (ignored) */);
}

/**
 * emtr_event_recorder_record_events:
 * @self: (in): the event recorder
 * @event_id: (in): an RFC 4122 UUID representing the type of event that took
 * place
 * @num_events: (in): the number of times the event type took place
 * @auxiliary_payload: (allow-none) (in): miscellaneous data to associate with
 * the events. Must not contain maybe variants as they are not compatible with
 * D-Bus.
 *
 * Make a best-effort to record the fact that @num_events events of type
 * @event_id happened between the current time and the previous such recording.
 * Optionally, associate arbitrary data, @auxiliary_payload, with these
 * particular instances of the event. Under no circumstances should
 * personally-identifiable information be included in the @auxiliary_payload,
 * the @event_id, or @num_events. Large auxiliary payloads dominate the size of
 * the event and should therefore be used sparingly. Events for which precise
 * timing information is required should instead be recorded using
 * emtr_event_recorder_record_event().
 *
 * At the discretion of the metrics system, the events may be discarded before
 * being reported to the metrics server. The events may take arbitrarily long to
 * reach the server and may be persisted unencrypted on the client for
 * arbitrarily long. There is no guarantee that the events are delivered via the
 * network; for example, they may instead be delivered manually on a USB drive.
 * No indication of successful or failed delivery is provided, and no
 * application should rely on successful delivery. To conserve bandwidth, the
 * events may be aggregated in a lossy fashion with other events with the same
 * @event_id before reaching the server.
 */
void
emtr_event_recorder_record_events (EmtrEventRecorder *self,
                                   const gchar       *event_id,
                                   gint64             num_events,
                                   GVariant          *auxiliary_payload)
{
  /* Get the time before doing anything else because it will change during
  execution. */
  gint64 relative_time;
  if (!emtr_util_get_current_time (CLOCK_BOOTTIME, &relative_time))
    {
      g_critical ("Getting relative timestamp failed.");
      return;
    }

  g_return_if_fail (EMTR_IS_EVENT_RECORDER (self));
  g_return_if_fail (event_id != NULL);
  g_return_if_fail (auxiliary_payload == NULL ||
                    _IS_VARIANT (auxiliary_payload));

  if (contains_maybe_variant (auxiliary_payload))
    return;

#ifdef DEBUG
  {
    gchar *payload_string = pretty_print_variant_or_null (auxiliary_payload);
    g_debug ("%s: Event ID: %s, number of events: %" G_GINT64_FORMAT ", "
             "payload: %s", G_STRFUNC, event_id, num_events, payload_string);
    g_free (payload_string);
  }
#else
  {
    g_debug ("%s: Event ID: %s, number of events: %" G_GINT64_FORMAT ", "
             "payload: %p", G_STRFUNC, event_id, num_events, auxiliary_payload);
  }
#endif /* DEBUG */

  record_events (self, event_id, auxiliary_payload, relative_time,
                 FALSE /* is_synchronous */, TRUE /* is_aggregate */,
                 num_events);
}

/**
 * emtr_event_recorder_record_events_sync:
 * @self: (in): the event recorder
 * @event_id: (in): an RFC 4122 UUID representing the type of event that took
 * place
 * @num_events: (in): the number of times the event type took place
 * @auxiliary_payload: (allow-none) (in): miscellaneous data to associate with
 * the events. Must not contain maybe variants as they are not compatible with
 * D-Bus.
 *
 * Make a best-effort to record the fact that @num_events events of type
 * @event_id happened between the current time and the previous such recording.
 * Behaves like emtr_event_recorder_record_events() but executes synchronously,
 * blocking until either a timeout expires or the event recorder daemon has
 * received the event. Generally prefer emtr_event_recorder_record_events() in
 * UI threads, but use emtr_event_recorder_record_events_sync() instead for the
 * sake of reliability when recording events from a process that is about to
 * close.
 *
 * Since: 0.4
 */
void
emtr_event_recorder_record_events_sync (EmtrEventRecorder *self,
                                        const gchar       *event_id,
                                        gint64             num_events,
                                        GVariant          *auxiliary_payload)
{
  /* Get the time before doing anything else because it will change during
   * execution.
   */
  gint64 relative_time;
  if (!emtr_util_get_current_time (CLOCK_BOOTTIME, &relative_time))
    {
      g_critical ("Getting relative timestamp failed.");
      return;
    }

  g_return_if_fail (EMTR_IS_EVENT_RECORDER (self));
  g_return_if_fail (event_id != NULL);
  g_return_if_fail (auxiliary_payload == NULL ||
                    _IS_VARIANT (auxiliary_payload));

  if (contains_maybe_variant (auxiliary_payload))
    return;

#ifdef DEBUG
  {
    gchar *payload_string = pretty_print_variant_or_null (auxiliary_payload);
    g_debug ("%s: Event ID: %s, number of events: %" G_GINT64_FORMAT ", "
             "payload: %s", G_STRFUNC, event_id, num_events, payload_string);
    g_free (payload_string);
  }
#else
  {
    g_debug ("%s: Event ID: %s, number of events: %" G_GINT64_FORMAT ", "
             "payload: %p", G_STRFUNC, event_id, num_events, auxiliary_payload);
  }
#endif /* DEBUG */

  record_events (self, event_id, auxiliary_payload, relative_time,
                 TRUE /* is_synchronous */, TRUE /* is_aggregate */,
                 num_events);
}

/**
 * emtr_event_recorder_record_start:
 * @self: (in): the event recorder
 * @event_id: (in): an RFC 4122 UUID representing the type of event that took
 * place
 * @key: (allow-none) (in): the identifier used to associate the start of the
 * event with the stop and any progress
 * @auxiliary_payload: (allow-none) (in): miscellaneous data to associate with
 * the events. Must not contain maybe variants as they are not compatible with
 * D-Bus.
 *
 * Make a best-effort to record the fact that an event of type @event_id
 * started at the current time. The event's stop must be reported using
 * emtr_event_recorder_record_stop() or memory will be leaked. If starts and
 * stops of events of type @event_id can be nested, then @key should be used to
 * disambiguate the stop and any progress that corresponds to this start. For
 * example, if one were recording how long processes remained open, process IDs
 * would be a suitable choice for the @key. Within the lifetime of each process,
 * process IDs are unique within the scope of PROCESS_OPEN events. If starts and
 * stops of events of type @event_id can not be nested, then @key can be %NULL.
 *
 * Optionally, associate arbitrary data, @auxiliary_payload, with this
 * particular instance of the event. Under no circumstances should
 * personally-identifiable information be included in the @auxiliary_payload or
 * @event_id. Large auxiliary payloads dominate the size of the event and should
 * therefore be used sparingly. Events for which precise timing information is
 * not required should instead be recorded using
 * emtr_event_recorder_record_events() to conserve bandwidth.
 *
 * At the discretion of the metrics system, the event may be discarded before
 * being reported to the metrics server. However, an event start, the
 * corresponding stop, and any corresponding progress either will be delivered
 * or dropped atomically. The event may take arbitrarily long to reach the
 * server and may be persisted unencrypted on the client for arbitrarily long.
 * There is no guarantee that the event is delivered via the network; for
 * example, it may instead be delivered manually on a USB drive. No indication
 * of successful or failed delivery is provided, and no application should rely
 * on successful delivery. The event will not be aggregated with other events
 * before reaching the server.
 */
void
emtr_event_recorder_record_start (EmtrEventRecorder *self,
                                  const gchar       *event_id,
                                  GVariant          *key,
                                  GVariant          *auxiliary_payload)
{
  /* Validate inputs before acquiring the lock below to avoid verbose error
     handling that releases the lock and logs a custom error message. */
  g_return_if_fail (EMTR_IS_EVENT_RECORDER (self));
  g_return_if_fail (event_id != NULL);
  g_return_if_fail (key == NULL || _IS_VARIANT (key));
  g_return_if_fail (auxiliary_payload == NULL ||
                    _IS_VARIANT (auxiliary_payload));

  if (contains_maybe_variant (auxiliary_payload))
    return;

#ifdef DEBUG
  {
    gchar *payload_string = pretty_print_variant_or_null (auxiliary_payload);
    gchar *key_string = pretty_print_variant_or_null (key);
    g_debug ("%s: Event ID: %s, key: %s, payload: %s", G_STRFUNC, event_id,
             key_string, payload_string);
    g_free (key_string);
    g_free (payload_string);
  }
#else
  {
    g_debug ("%s: Event ID: %s, key: %p, payload: %p", G_STRFUNC, event_id, key,
             auxiliary_payload);
  }
#endif /* DEBUG */

  EmtrEventRecorderPrivate *priv =
    emtr_event_recorder_get_instance_private (self);

  if (!priv->recording_enabled)
    return;

  /* Acquire this lock before getting the time so that event sequences are
     guaranteed to be chronologically sorted. */
  g_mutex_lock (&priv->events_by_id_with_key_lock);

  // Get the time as soon as possible because it will change during execution.
  gint64 relative_time;
  if (!emtr_util_get_current_time (CLOCK_BOOTTIME, &relative_time))
    {
      g_critical ("Getting relative timestamp failed.");
      goto finally;
    }

  uuid_t parsed_event_id;
  if (!parse_event_id (event_id, parsed_event_id))
    goto finally;

  key = get_normalized_form_of_variant (key);

  GVariant *event_id_with_key = combine_event_id_with_key (parsed_event_id,
                                                           key);

  auxiliary_payload = get_normalized_form_of_variant (auxiliary_payload);
  GPtrArray *event_sequence =
    g_ptr_array_new_full (2u, (GDestroyNotify) g_variant_unref);
  append_event_to_sequence (self, event_sequence, relative_time,
                            auxiliary_payload);
  if (auxiliary_payload != NULL)
    g_variant_unref (auxiliary_payload);

  if (!g_hash_table_insert (priv->events_by_id_with_key, event_id_with_key,
                            event_sequence))
    {
      if (key != NULL)
        {
          gchar *key_as_string = g_variant_print (key, TRUE);
          g_variant_unref (key);

          g_warning ("Restarted event of type %s with key %s; there was "
                     "already an unstopped start event with this "
                     "type and key.", event_id, key_as_string);

          g_free (key_as_string);
        }
      else
        {
          g_warning ("Restarted event of type %s with NULL key; there was "
                     "already an unstopped start event with this type and key.",
                     event_id);
        }
      goto finally;
    }

  if (key != NULL)
    g_variant_unref (key);

finally:
  g_mutex_unlock (&priv->events_by_id_with_key_lock);
}

/**
 * emtr_event_recorder_record_progress:
 * @self: (in): the event recorder
 * @event_id: (in): an RFC 4122 UUID representing the type of event that took
 * place
 * @key: (allow-none) (in): the identifier used to associate the event progress
 * with the start, stop, and any other progress
 * @auxiliary_payload: (allow-none) (in): miscellaneous data to associate with
 * the events. Must not contain maybe variants as they are not compatible with
 * D-Bus.
 *
 * Make a best-effort to record the fact that an event of type @event_id
 * progressed at the current time. May be called arbitrarily many times between
 * a corresponding start and stop. Behaves like
 * emtr_event_recorder_record_start().
 */
void
emtr_event_recorder_record_progress (EmtrEventRecorder *self,
                                     const gchar       *event_id,
                                     GVariant          *key,
                                     GVariant          *auxiliary_payload)
{
  /* Validate inputs before acquiring the lock below to avoid verbose error
     handling that releases the lock and logs a custom error message. */
  g_return_if_fail (EMTR_IS_EVENT_RECORDER (self));
  g_return_if_fail (event_id != NULL);
  g_return_if_fail (key == NULL || _IS_VARIANT (key));
  g_return_if_fail (auxiliary_payload == NULL ||
                    _IS_VARIANT (auxiliary_payload));

  if (contains_maybe_variant (auxiliary_payload))
    return;

#ifdef DEBUG
  {
    gchar *payload_string = pretty_print_variant_or_null (auxiliary_payload);
    gchar *key_string = pretty_print_variant_or_null (key);
    g_debug ("%s: Event ID: %s, key: %s, payload: %s", G_STRFUNC, event_id,
             key_string, payload_string);
    g_free (key_string);
    g_free (payload_string);
  }
#else
  {
    g_debug ("%s: Event ID: %s, key: %p, payload: %p", G_STRFUNC, event_id, key,
             auxiliary_payload);
  }
#endif /* DEBUG */

  EmtrEventRecorderPrivate *priv =
    emtr_event_recorder_get_instance_private (self);

  if (!priv->recording_enabled)
    return;

  /* Acquire this lock before getting the time so that event sequences are
     guaranteed to be chronologically sorted. */
  g_mutex_lock (&priv->events_by_id_with_key_lock);

  // Get the time as soon as possible because it will change during execution.
  gint64 relative_time;
  if (!emtr_util_get_current_time (CLOCK_BOOTTIME, &relative_time))
    {
      g_critical ("Getting relative timestamp failed.");
      goto finally;
    }

  uuid_t parsed_event_id;
  if (!parse_event_id (event_id, parsed_event_id))
    goto finally;

  key = get_normalized_form_of_variant (key);

  GVariant *event_id_with_key =
    combine_event_id_with_key (parsed_event_id, key);
  GPtrArray *event_sequence =
    g_hash_table_lookup (priv->events_by_id_with_key, event_id_with_key);
  g_variant_unref (event_id_with_key);

  if (event_sequence == NULL)
    {
      if (key != NULL)
        {
          gchar *key_as_string = g_variant_print (key, TRUE);
          g_variant_unref (key);

          g_warning ("Ignoring request to record progress for event of type %s "
                     "with key %s because there is no corresponding unstopped "
                     "start event.", event_id, key_as_string);

          g_free (key_as_string);
        }
      else
        {
          g_warning ("Ignoring request to record progress for event of type %s "
                     "with NULL key because there is no corresponding "
                     "unstopped start event.", event_id);
        }
      goto finally;
    }

  if (key != NULL)
    g_variant_unref (key);

  auxiliary_payload = get_normalized_form_of_variant (auxiliary_payload);

  append_event_to_sequence (self, event_sequence, relative_time,
                            auxiliary_payload);

  if (auxiliary_payload != NULL)
    g_variant_unref (auxiliary_payload);

finally:
  g_mutex_unlock (&priv->events_by_id_with_key_lock);
}

/**
 * emtr_event_recorder_record_stop:
 * @self: (in): the event recorder
 * @event_id: (in): an RFC 4122 UUID representing the type of event that took
 * place
 * @key: (allow-none) (in): the identifier used to associate the stop of the
 * event with the start and any progress
 * @auxiliary_payload: (allow-none) (in): miscellaneous data to associate with
 * the events. Must not contain maybe variants as they are not compatible with
 * D-Bus.
 *
 * Make a best-effort to record the fact that an event of type @event_id
 * stopped at the current time. Behaves like emtr_event_recorder_record_start().
 */
void
emtr_event_recorder_record_stop (EmtrEventRecorder *self,
                                 const gchar       *event_id,
                                 GVariant          *key,
                                 GVariant          *auxiliary_payload)
{
  /* Validate inputs before acquiring the lock in record_stop to avoid verbose
     error handling that releases the lock and logs a custom error message. */
  g_return_if_fail (EMTR_IS_EVENT_RECORDER (self));
  g_return_if_fail (event_id != NULL);
  g_return_if_fail (key == NULL || _IS_VARIANT (key));
  g_return_if_fail (auxiliary_payload == NULL ||
                    _IS_VARIANT (auxiliary_payload));

  if (contains_maybe_variant (auxiliary_payload))
    return;

#ifdef DEBUG
  {
    gchar *payload_string = pretty_print_variant_or_null (auxiliary_payload);
    gchar *key_string = pretty_print_variant_or_null (key);
    g_debug ("%s: Event ID: %s, key: %s, payload: %s", G_STRFUNC, event_id,
             key_string, payload_string);
    g_free (key_string);
    g_free (payload_string);
  }
#else
  {
    g_debug ("%s: Event ID: %s, key: %p, payload: %p", G_STRFUNC, event_id, key,
             auxiliary_payload);
  }
#endif /* DEBUG */

  record_stop (self, event_id, key, auxiliary_payload,
               FALSE /* is_synchronous */);
}

/**
 * emtr_event_recorder_record_stop_sync:
 * @self: (in): the event recorder
 * @event_id: (in): an RFC 4122 UUID representing the type of event that took
 * place
 * @key: (allow-none) (in): the identifier used to associate the stop of the
 * event with the start and any progress
 * @auxiliary_payload: (allow-none) (in): miscellaneous data to associate with
 * the events. Must not contain maybe variants as they are not compatible with
 * D-Bus.
 *
 * Make a best-effort to record the fact that an event of type @event_id stopped
 * at the current time. Behaves like emtr_event_recorder_record_stop() but
 * executes synchronously, blocking until a timeout expires or the event
 * recorder daemon has received the event sequence. Generally prefer
 * emtr_event_recorder_record_stop() in UI threads, but use
 * emtr_event_recorder_record_stop_sync() instead for the sake of reliability
 * when recording an event sequence from a process that is about to close.
 *
 * Since: 0.4
 */
void
emtr_event_recorder_record_stop_sync (EmtrEventRecorder *self,
                                      const gchar       *event_id,
                                      GVariant          *key,
                                      GVariant          *auxiliary_payload)
{
  /* Validate inputs before acquiring the lock in record_stop to avoid verbose
     error handling that releases the lock and logs a custom error message. */
  g_return_if_fail (EMTR_IS_EVENT_RECORDER (self));
  g_return_if_fail (event_id != NULL);
  g_return_if_fail (key == NULL || _IS_VARIANT (key));
  g_return_if_fail (auxiliary_payload == NULL ||
                    _IS_VARIANT (auxiliary_payload));

  if (contains_maybe_variant (auxiliary_payload))
    return;

#ifdef DEBUG
  {
    gchar *payload_string = pretty_print_variant_or_null (auxiliary_payload);
    gchar *key_string = pretty_print_variant_or_null (key);
    g_debug ("%s: Event ID: %s, key: %s, payload: %s", G_STRFUNC, event_id,
             key_string, payload_string);
    g_free (key_string);
    g_free (payload_string);
  }
#else
  {
    g_debug ("%s: Event ID: %s, key: %p, payload: %p", G_STRFUNC, event_id, key,
             auxiliary_payload);
  }
#endif /* DEBUG */

  record_stop (self, event_id, key, auxiliary_payload,
               TRUE /* is_synchronous */);
}

/**
 * emtr_event_recorder_start_aggregate_timer:
 * @self: an #EmtrEventRecorder
 * @event_id: an RFC 4122 UUID representing the type of event that took place
 * @auxiliary_payload: (nullable): miscellaneous data to associate with the
 * events. Must not contain maybe variants as they are not compatible with
 * D-Bus.
 *
 * Requests the metrics daemon to create an aggregate timer.
 *
 * Returns: (transfer full)(nullable): a #EmtrAggregateTimer
 */
EmtrAggregateTimer *
emtr_event_recorder_start_aggregate_timer (EmtrEventRecorder *self,
                                           const gchar       *event_id,
                                           GVariant          *auxiliary_payload)
{
  return emtr_event_recorder_start_aggregate_timer_with_uid (self, getuid (), event_id, auxiliary_payload);
}

/**
 * emtr_event_recorder_start_aggregate_timer_with_uid:
 * @self: an #EmtrEventRecorder
 * @uid: the UID to ascribe the event to
 * @event_id: an RFC 4122 UUID representing the type of event that took place
 * @auxiliary_payload: (nullable): miscellaneous data to associate with the
 * events. Must not contain maybe variants as they are not compatible with
 * D-Bus.
 *
 * Requests the metrics daemon to create an aggregate timer.
 *
 * Returns: (transfer full)(nullable): a #EmtrAggregateTimer
 */
EmtrAggregateTimer *
emtr_event_recorder_start_aggregate_timer_with_uid (EmtrEventRecorder *self,
                                                    uid_t              uid,
                                                    const gchar       *event_id,
                                                    GVariant          *auxiliary_payload)
{
  EmtrEventRecorderPrivate *priv =
    emtr_event_recorder_get_instance_private (self);
  GVariantBuilder event_id_builder;
  GVariant *maybe_payload;
  uuid_t parsed_event_id;

  g_return_val_if_fail (EMTR_IS_EVENT_RECORDER (self), NULL);
  g_return_val_if_fail (auxiliary_payload == NULL ||
                        _IS_VARIANT (auxiliary_payload), NULL);

  if (contains_maybe_variant (auxiliary_payload))
    return NULL;

  if (!priv->recording_enabled)
    return NULL;

  if (!parse_event_id (event_id, parsed_event_id))
    return NULL;

  get_uuid_builder (parsed_event_id, &event_id_builder);

  if (auxiliary_payload == NULL)
    maybe_payload = priv->empty_auxiliary_payload;
  else
    maybe_payload = g_variant_new_variant (auxiliary_payload);

  return emtr_aggregate_timer_new (priv->dbus_proxy,
                                   uid,
                                   g_variant_builder_end (&event_id_builder),
                                   auxiliary_payload != NULL,
                                   maybe_payload);
}

#undef _IS_VARIANT

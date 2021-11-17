/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */

/* Copyright 2021 Endless OS Foundation, LLC. */

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

#include "emtr-aggregate-timer-private.h"


/**
 * SECTION:emtr-aggregate-timer
 * @title: Aggregate events timer
 * @short_description: Timer to aggregate events.
 * @include: eosmetrics/eos-aggregate-timer.h
 *
 * EmtrAggregateTimer is a simple timer to aggregate events.
 */

struct _EmtrAggregateTimer
{
  GObject parent_instance;

  GDBusConnection *connection; /* (unowned) */
  GCancellable *cancellable; /* (owned) */

  gchar *timer_object_path;
  gboolean stopped;
};

G_DEFINE_TYPE (EmtrAggregateTimer, emtr_aggregate_timer, G_TYPE_OBJECT)

static void
call_stop_timer (EmtrAggregateTimer *self)
{
  if (!self->timer_object_path)
    return;

  g_dbus_connection_call (self->connection,
                          "com.endlessm.Metrics",
                          self->timer_object_path,
                          "com.endlessm.Metrics.AggregateTimer",
                          "StopTimer",
                          NULL, NULL,
                          G_DBUS_CALL_FLAGS_NO_AUTO_START,
                          G_MAXINT,
                          NULL, NULL, NULL);
}

static void
emtr_aggregate_timer_finalize (GObject *object)
{
  EmtrAggregateTimer *self = (EmtrAggregateTimer *)object;

  if (!self->stopped)
    call_stop_timer (self);

  g_cancellable_cancel (self->cancellable);
  g_clear_object (&self->cancellable);
  g_clear_pointer (&self->timer_object_path, g_free);

  G_OBJECT_CLASS (emtr_aggregate_timer_parent_class)->finalize (object);
}

static void
emtr_aggregate_timer_class_init (EmtrAggregateTimerClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = emtr_aggregate_timer_finalize;
}

static void
emtr_aggregate_timer_init (EmtrAggregateTimer *self)
{
}

static void
on_server_aggregate_timer_started_cb (GObject      *source_object,
                                      GAsyncResult *result,
                                      gpointer      user_data)
{
  EmtrAggregateTimer *self;
  EmerEventRecorderServer *event_recorder_server;
  g_autofree gchar *timer_object_path = NULL;
  g_autoptr(GError) error = NULL;

  event_recorder_server = EMER_EVENT_RECORDER_SERVER (source_object);
  emer_event_recorder_server_call_start_aggregate_timer_finish (event_recorder_server,
                                                                &timer_object_path,
                                                                result,
                                                                &error);

  if (error)
    {
      if (!g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
        g_warning ("Error creating aggregate timer: %s", error->message);
      return;
    }

  self = EMTR_AGGREGATE_TIMER (user_data);

  /* If the timer creation wasn't cancelled yet, we can't cancel the
   * proxy creation. From now on, if we need to stop the timer, we need
   * wait for the proxy to be created, then call the Stop() D-Bus method.
   */
  g_clear_object (&self->cancellable);

  self->timer_object_path = g_steal_pointer (&timer_object_path);
}

EmtrAggregateTimer *
emtr_aggregate_timer_new (EmerEventRecorderServer *dbus_proxy,
                          GVariant                *event_id,
                          GVariant                *aggregate_key,
                          GVariant                *auxiliary_payload)
{
  EmtrAggregateTimer *self;

  self = g_object_new (EMTR_TYPE_AGGREGATE_TIMER, NULL);
  self->cancellable = g_cancellable_new ();
  self->connection = g_dbus_proxy_get_connection (G_DBUS_PROXY (dbus_proxy));

  emer_event_recorder_server_call_start_aggregate_timer (dbus_proxy,
                                                         event_id,
                                                         aggregate_key,
                                                         TRUE,
                                                         auxiliary_payload,
                                                         self->cancellable,
                                                         on_server_aggregate_timer_started_cb,
                                                         self);

  return self;
}

/**
 * emtr_aggregate_timer_stop:
 * @self: an #EmtrAggregateTimer
 *
 * Stops the aggregation timer. Timers must be stopped at most once.
 */
void
emtr_aggregate_timer_stop (EmtrAggregateTimer *self)
{
  g_return_if_fail (EMTR_IS_AGGREGATE_TIMER (self));
  g_return_if_fail (!self->stopped);

  g_cancellable_cancel (self->cancellable);
  call_stop_timer (self);
  self->stopped = TRUE;
}

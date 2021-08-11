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

  EmerAggregateTimer *timer_proxy; /* (owned) */

  gboolean stopped;
};

G_DEFINE_TYPE (EmtrAggregateTimer, emtr_aggregate_timer, G_TYPE_OBJECT)

static void
emtr_aggregate_timer_finalize (GObject *object)
{
  EmtrAggregateTimer *self = (EmtrAggregateTimer *)object;

  if (!self->stopped && self->timer_proxy)
    emer_aggregate_timer_call_stop_timer (self->timer_proxy, NULL, NULL, NULL);

  g_clear_object (&self->timer_proxy);

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
on_aggregate_timer_proxy_created_cb (GObject      *source_object,
                                     GAsyncResult *result,
                                     gpointer      user_data)
{
  g_autoptr(EmtrAggregateTimer) self = EMTR_AGGREGATE_TIMER (user_data);
  g_autoptr(GError) error = NULL;

  self->timer_proxy =
    emer_aggregate_timer_proxy_new_for_bus_finish (result, &error);

  if (error)
    g_warning ("Error creating aggregate timer: %s", error->message);

  if (self->stopped && self->timer_proxy)
    emer_aggregate_timer_call_stop_timer (self->timer_proxy, NULL, NULL, NULL);
}

static void
on_server_aggregate_timer_started_cb (GObject      *source_object,
                                      GAsyncResult *result,
                                      gpointer      user_data)
{
  g_autoptr(EmtrAggregateTimer) self = EMTR_AGGREGATE_TIMER (user_data);
  EmerEventRecorderServer *event_recorder_server;
  g_autofree gchar *timer_object_path = NULL;
  g_autoptr(GError) error = NULL;
  GDBusConnection *connection;
  GDBusProxy *proxy;

  event_recorder_server = EMER_EVENT_RECORDER_SERVER (source_object);
  emer_event_recorder_server_call_start_aggregate_timer_finish (event_recorder_server,
                                                                &timer_object_path,
                                                                result,
                                                                &error);

  if (error)
    {
      g_warning ("Error creating aggregate timer: %s", error->message);
      return;
    }

  proxy = G_DBUS_PROXY (source_object);
  connection = g_dbus_proxy_get_connection (proxy);
  emer_aggregate_timer_proxy_new (connection,
                                  G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES |
                                  G_DBUS_PROXY_FLAGS_DO_NOT_CONNECT_SIGNALS |
                                  G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START,
                                  g_dbus_proxy_get_name (proxy),
                                  timer_object_path,
                                  NULL,
                                  on_aggregate_timer_proxy_created_cb,
                                  g_object_ref (self));
}

EmtrAggregateTimer *
emtr_aggregate_timer_new (EmerEventRecorderServer *dbus_proxy,
                          GVariant                *event_id,
                          GVariant                *aggregate_key,
                          GVariant                *auxiliary_payload)
{
  EmtrAggregateTimer *self;

  self = g_object_new (EMTR_TYPE_AGGREGATE_TIMER, NULL);

  emer_event_recorder_server_call_start_aggregate_timer (dbus_proxy,
                                                         event_id,
                                                         aggregate_key,
                                                         TRUE,
                                                         auxiliary_payload,
                                                         NULL,
                                                         on_server_aggregate_timer_started_cb,
                                                         g_object_ref (self));

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

  if (self->timer_proxy)
    emer_aggregate_timer_call_stop_timer (self->timer_proxy, NULL, NULL, NULL);
  self->stopped = TRUE;
}

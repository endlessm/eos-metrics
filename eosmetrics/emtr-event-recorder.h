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

#ifndef EMTR_EVENT_RECORDER_H
#define EMTR_EVENT_RECORDER_H

#if !(defined(_EMTR_INSIDE_EOSMETRICS_H) || defined(COMPILING_EOS_METRICS))
#error "Please do not include this header file directly."
#endif

#include "emtr-types.h"
#include <glib-object.h>
#include <gio/gio.h>

G_BEGIN_DECLS

#define EMTR_TYPE_EVENT_RECORDER emtr_event_recorder_get_type()

#define EMTR_EVENT_RECORDER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  EMTR_TYPE_EVENT_RECORDER, EmtrEventRecorder))

#define EMTR_EVENT_RECORDER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  EMTR_TYPE_EVENT_RECORDER, EmtrEventRecorderClass))

#define EMTR_IS_EVENT_RECORDER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  EMTR_TYPE_EVENT_RECORDER))

#define EMTR_IS_EVENT_RECORDER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  EMTR_TYPE_EVENT_RECORDER))

#define EMTR_EVENT_RECORDER_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  EMTR_TYPE_EVENT_RECORDER, EmtrEventRecorderClass))

/**
 * EmtrEventRecorder:
 *
 * This instance structure contains no public members.
 */
typedef struct _EmtrEventRecorder EmtrEventRecorder;

/**
 * EmtrEventRecorderClass:
 *
 * This class structure contains no public members.
 */
typedef struct _EmtrEventRecorderClass EmtrEventRecorderClass;

struct _EmtrEventRecorder
{
  /*< private >*/
  GObject parent;
};

struct _EmtrEventRecorderClass
{
  /*< private >*/
  GObjectClass parent_class;
};

EMTR_AVAILABLE_IN_0_0
GType              emtr_event_recorder_get_type           (void) G_GNUC_CONST;

EMTR_AVAILABLE_IN_0_0
EmtrEventRecorder *emtr_event_recorder_new                (void);

EMTR_AVAILABLE_IN_0_0
EmtrEventRecorder *emtr_event_recorder_get_default        (void);

EMTR_AVAILABLE_IN_0_0
void               emtr_event_recorder_record_event       (EmtrEventRecorder *self,
                                                           const gchar       *event_id,
                                                           GVariant          *auxiliary_payload);

EMTR_AVAILABLE_IN_0_4
void               emtr_event_recorder_record_event_sync  (EmtrEventRecorder *self,
                                                           const gchar       *event_id,
                                                           GVariant          *auxiliary_payload);

EMTR_AVAILABLE_IN_0_0
void               emtr_event_recorder_record_events      (EmtrEventRecorder *self,
                                                           const gchar       *event_id,
                                                           gint64             num_events,
                                                           GVariant          *auxiliary_payload);

EMTR_AVAILABLE_IN_0_4
void               emtr_event_recorder_record_events_sync (EmtrEventRecorder *self,
                                                           const gchar       *event_id,
                                                           gint64             num_events,
                                                           GVariant          *auxiliary_payload);

EMTR_AVAILABLE_IN_0_0
void               emtr_event_recorder_record_start       (EmtrEventRecorder *self,
                                                           const gchar       *event_id,
                                                           GVariant          *key,
                                                           GVariant          *auxiliary_payload);

EMTR_AVAILABLE_IN_0_0
void               emtr_event_recorder_record_progress    (EmtrEventRecorder *self,
                                                           const gchar       *event_id,
                                                           GVariant          *key,
                                                           GVariant          *auxiliary_payload);

EMTR_AVAILABLE_IN_0_0
void               emtr_event_recorder_record_stop        (EmtrEventRecorder *self,
                                                           const gchar       *event_id,
                                                           GVariant          *key,
                                                           GVariant          *auxiliary_payload);

EMTR_AVAILABLE_IN_0_4
void               emtr_event_recorder_record_stop_sync   (EmtrEventRecorder *self,
                                                           const gchar       *event_id,
                                                           GVariant          *key,
                                                           GVariant          *auxiliary_payload);

EMTR_AVAILABLE_IN_0_5
EmtrAggregateTimer *emtr_event_recorder_start_aggregate_timer (EmtrEventRecorder *self,
                                                               const gchar       *event_id,
                                                               GVariant          *auxiliary_payload);
EMTR_AVAILABLE_IN_0_5
EmtrAggregateTimer *emtr_event_recorder_start_aggregate_timer_with_uid (EmtrEventRecorder *self,
                                                                        uid_t              uid,
                                                                        const gchar       *event_id,
                                                                        GVariant          *auxiliary_payload);
G_END_DECLS

#endif /* EMTR_EVENT_RECORDER_H */

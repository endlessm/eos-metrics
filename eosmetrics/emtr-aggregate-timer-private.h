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

#pragma once

#include "emtr-aggregate-timer.h"
#include "emer-event-recorder-server.h"

G_BEGIN_DECLS

EmtrAggregateTimer *emtr_aggregate_timer_new (EmerEventRecorderServer *dbus_proxy,
                                              GVariant                *event_id,
                                              GVariant                *aggregate_key,
                                              gboolean                 has_payload,
                                              GVariant                *auxiliary_payload);

G_END_DECLS

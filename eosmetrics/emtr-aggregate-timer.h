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

#if !(defined(_EMTR_INSIDE_EOSMETRICS_H) || defined(COMPILING_EOS_METRICS))
#error "Please do not include this header file directly."
#endif

#include "emtr-types.h"
#include <gio/gio.h>

G_BEGIN_DECLS

#define EMTR_TYPE_AGGREGATE_TIMER (emtr_aggregate_timer_get_type())
G_DECLARE_FINAL_TYPE (EmtrAggregateTimer, emtr_aggregate_timer, EMTR, AGGREGATE_TIMER, GObject)

EMTR_AVAILABLE_IN_0_5
void emtr_aggregate_timer_stop (EmtrAggregateTimer *self);

G_END_DECLS

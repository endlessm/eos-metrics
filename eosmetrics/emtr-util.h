/* -*- mode: C; c-file-style: "gnu"; indent-tabs-: nil; -*- */

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

#ifndef EMTR_UTIL_PRIVATE_H
#define EMTR_UTIL_PRIVATE_H

#include <sys/types.h>

/* For clockid_t */
#if !defined(_POSIX_C_SOURCE) || _POSIX_C_SOURCE < 199309L
#error "This code requires _POSIX_C_SOURCE to be 199309L or later."
#endif

#include <glib.h>

G_BEGIN_DECLS

gboolean emtr_util_get_current_time (clockid_t  clock_id,
                                     gint64    *current_time);

G_END_DECLS

#endif /* EMTR_UTIL_PRIVATE_H */

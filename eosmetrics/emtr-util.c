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

#include "eosmetrics/emtr-util.h"

/* For clock_gettime() */
#if !defined(_POSIX_C_SOURCE) || _POSIX_C_SOURCE < 199309L
#error "This code requires _POSIX_C_SOURCE to be 199309L or later."
#endif

#include <errno.h>
#include <time.h>

#include <glib.h>

#define NANOSECONDS_PER_SECOND 1000000000L

/**
 * SECTION:emtr-util
 * @title: Util
 * @short_description: Utilities available for use by consumers of the event
 * recorder API.
 * @include: eosmetrics/eosmetrics.h
 *
 * Refer to method documentation for more details.
 */

/**
 * emtr_util_get_current_time: (skip)
 * @clock_id: (in): the clock from which to read the current time
 * @current_time: (out): a space in which to store the current time
 *
 * Populates @current_time with the current time according to @clock_id.
 * Guarantees that the difference between any two successfully fetched times
 * fits in a gint64.
 *
 * Returns: TRUE if the current time was successfully read and FALSE otherwise.
 * Since: 0.2
 */
gboolean
emtr_util_get_current_time (clockid_t clock_id,
                            gint64   *current_time)
{
  g_return_val_if_fail (current_time != NULL, FALSE);

  // Get the time before doing anything else because it will change during
  // execution.
  struct timespec ts;
  int gettime_failed = clock_gettime (clock_id, &ts);
  if (gettime_failed != 0)
    {
      int error_code = errno;
      g_critical ("Attempt to get current time failed: %s",
                  g_strerror (error_code));
      return FALSE;
    }

  // Ensure that the clock provides a time that can be safely represented in a
  // gint64 in nanoseconds.
  if (ts.tv_sec < G_MININT64 / NANOSECONDS_PER_SECOND ||
      ts.tv_sec > G_MAXINT64 / NANOSECONDS_PER_SECOND ||
      ts.tv_nsec < 0 ||
      ts.tv_nsec >= NANOSECONDS_PER_SECOND ||
      // We already know that ts.tv_sec <= G_MAXINT64 / NANOSECONDS_PER_SECOND.
      // This handles the edge case where
      // ts.tv_sec == G_MAXINT64 / NANOSECONDS_PER_SECOND.
      (ts.tv_sec == G_MAXINT64 / NANOSECONDS_PER_SECOND &&
       ts.tv_nsec > G_MAXINT64 % NANOSECONDS_PER_SECOND))
    {
      /* The (gint64) conversion is to handle the fact that time_t's size is
      platform-defined; so we cast it to 64 bits. tv_nsec is defined as long. */
      g_critical ("Clock returned a time that does not fit in a 64-bit integer "
                  "in nanoseconds (seconds %" G_GINT64_FORMAT ", nanoseconds "
                  "%ld.)", (gint64) ts.tv_sec, ts.tv_nsec);
      return FALSE;
    }

  gint64 detected_time =
    (NANOSECONDS_PER_SECOND * ((gint64) ts.tv_sec)) + ((gint64) ts.tv_nsec);
  if (detected_time < (G_MININT64 / 2) || detected_time > (G_MAXINT64 / 2))
    {
      g_critical ("Clock returned a time that may result in arithmetic that "
                  "causes 64-bit overflow. This machine may have been running "
                  "for over 100 years!");
      return FALSE;
    }
  *current_time = detected_time;
  return TRUE;
}

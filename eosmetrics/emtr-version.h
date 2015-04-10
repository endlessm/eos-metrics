/* Copyright 2013, 2014, 2015 Endless Mobile, Inc. */

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

#ifndef EMTR_VERSION_H
#define EMTR_VERSION_H

#if !(defined(_EMTR_INSIDE_EOSMETRICS_H) || defined(COMPILING_EOS_METRICS))
#error "Please do not include this header file directly."
#endif

#include <glib.h>

#ifdef EMTR_DISABLE_DEPRECATION_WARNINGS
#define EMTR_DEPRECATED
#define EMTR_DEPRECATED_FOR(f)
#define EMTR_UNAVAILABLE(maj,min)
#else
#define EMTR_DEPRECATED G_DEPRECATED
#define EMTR_DEPRECATED_FOR(f) G_DEPRECATED_FOR(f)
#define EMTR_UNAVAILABLE(maj,min) G_UNAVAILABLE(maj,min)
#endif

/* Each new stable series should add a new version symbol here. If necessary,
define EMTR_VERSION_MIN_REQUIRED and EMTR_VERSION_MAX_ALLOWED to one of these
macros. */
#define EMTR_VERSION_0_0 (G_ENCODE_VERSION (0, 0))
#define EMTR_VERSION_0_2 (G_ENCODE_VERSION (0, 2))
#define EMTR_VERSION_0_4 (G_ENCODE_VERSION (0, 4))

#if (EMTR_MINOR_VERSION == 99)
#define EMTR_VERSION_CUR_STABLE (G_ENCODE_VERSION (EMTR_MAJOR_VERSION + 1, 0))
#elif (EMTR_MINOR_VERSION % 2)
#define EMTR_VERSION_CUR_STABLE (G_ENCODE_VERSION (EMTR_MAJOR_VERSION, EMTR_MINOR_VERSION + 1))
#else
#define EMTR_VERSION_CUR_STABLE (G_ENCODE_VERSION (EMTR_MAJOR_VERSION, EMTR_MINOR_VERSION))
#endif

/* evaluates to the previous stable version */
#if (EMTR_MINOR_VERSION == 99)
#define EMTR_VERSION_PREV_STABLE (G_ENCODE_VERSION (EMTR_MAJOR_VERSION + 1, 0))
#elif (EMTR_MINOR_VERSION % 2)
#define EMTR_VERSION_PREV_STABLE (G_ENCODE_VERSION (EMTR_MAJOR_VERSION, EMTR_MINOR_VERSION - 1))
#else
#define EMTR_VERSION_PREV_STABLE (G_ENCODE_VERSION (EMTR_MAJOR_VERSION, EMTR_MINOR_VERSION - 2))
#endif

#ifndef EMTR_VERSION_MIN_REQUIRED
# define EMTR_VERSION_MIN_REQUIRED (EMTR_VERSION_CUR_STABLE)
#endif

#ifndef EMTR_VERSION_MAX_ALLOWED
# if EMTR_VERSION_MIN_REQUIRED > EMTR_VERSION_PREV_STABLE
#  define EMTR_VERSION_MAX_ALLOWED (EMTR_VERSION_MIN_REQUIRED)
# else
#  define EMTR_VERSION_MAX_ALLOWED (EMTR_VERSION_CUR_STABLE)
# endif
#endif

/* sanity checks */
#if EMTR_VERSION_MAX_ALLOWED < EMTR_VERSION_MIN_REQUIRED
#error "EMTR_VERSION_MAX_ALLOWED must be >= EMTR_VERSION_MIN_REQUIRED"
#endif
#if EMTR_VERSION_MIN_REQUIRED < EMTR_VERSION_0_0
#error "EMTR_VERSION_MIN_REQUIRED must be >= EMTR_VERSION_0_0"
#endif

/* Every new stable minor release should add a set of macros here */

#if EMTR_VERSION_MIN_REQUIRED >= EMTR_VERSION_0_0
# define EMTR_DEPRECATED_IN_0_0        EMTR_DEPRECATED
# define EMTR_DEPRECATED_IN_0_0_FOR(f) EMTR_DEPRECATED_FOR(f)
#else
# define EMTR_DEPRECATED_IN_0_0
# define EMTR_DEPRECATED_IN_0_0_FOR(f)
#endif

#if EMTR_VERSION_MAX_ALLOWED < EMTR_VERSION_0_0
# define EMTR_AVAILABLE_IN_0_0 EMTR_UNAVAILABLE(0, 0)
#else
# define EMTR_AVAILABLE_IN_0_0
#endif

#if EMTR_VERSION_MIN_REQUIRED >= EMTR_VERSION_0_2
# define EMTR_DEPRECATED_IN_0_2        EMTR_DEPRECATED
# define EMTR_DEPRECATED_IN_0_2_FOR(f) EMTR_DEPRECATED_FOR(f)
#else
# define EMTR_DEPRECATED_IN_0_2
# define EMTR_DEPRECATED_IN_0_2_FOR(f)
#endif

#if EMTR_VERSION_MAX_ALLOWED < EMTR_VERSION_0_2
# define EMTR_AVAILABLE_IN_0_2 EMTR_UNAVAILABLE(0, 2)
#else
# define EMTR_AVAILABLE_IN_0_2
#endif

#if EMTR_VERSION_MIN_REQUIRED >= EMTR_VERSION_0_4
# define EMTR_DEPRECATED_IN_0_4        EMTR_DEPRECATED
# define EMTR_DEPRECATED_IN_0_4_FOR(f) EMTR_DEPRECATED_FOR(f)
#else
# define EMTR_DEPRECATED_IN_0_4
# define EMTR_DEPRECATED_IN_0_4_FOR(f)
#endif

#if EMTR_VERSION_MAX_ALLOWED < EMTR_VERSION_0_4
# define EMTR_AVAILABLE_IN_0_4 EMTR_UNAVAILABLE(0, 4)
#else
# define EMTR_AVAILABLE_IN_0_4
#endif

#endif /* EMTR_VERSION_H */

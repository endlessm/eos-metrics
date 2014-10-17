/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2014 Endless Mobile, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
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

#endif /* EMTR_VERSION_H */

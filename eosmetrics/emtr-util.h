/* -*- mode: C; c-file-style: "gnu"; indent-tabs-: nil; -*- */

#ifndef EMTR_UTIL_PRIVATE_H
#define EMTR_UTIL_PRIVATE_H

/* For clockid_t */
#if !defined(_POSIX_C_SOURCE) || _POSIX_C_SOURCE < 199309L
#error "This code requires _POSIX_C_SOURCE to be 199309L or later."
#endif

#include <sys/types.h>

#include <glib.h>

G_BEGIN_DECLS

gboolean emtr_util_get_current_time (clockid_t  clock_id,
                                     gint64    *current_time);

G_END_DECLS

#endif /* EMTR_UTIL_PRIVATE_H */

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

#include "eosmetrics/eosmetrics.h"

#include <glib.h>
#include <gio/gio.h>

#define EOS_METRICS_LOG_DOMAIN "EosMetrics"

#define MEANINGLESS_EVENT "350ac4ff-3026-4c25-9e7e-e8103b4fd5d8"
#define MEANINGLESS_EVENT_2 "d936cd5c-08de-4d4e-8a87-8df1f4a33cba"

#define TESTING_FILE_PATH "/tmp/testing-machine-id"
#define TESTING_ID        "04448f74fde24bd7a16f8da17869d5c3\n"
/*
 * The expected size in bytes of the file located at
 * #EmtrMachineIdProvider:path.
 * According to http://www.freedesktop.org/software/systemd/man/machine-id.html
 * the file should be 32 lower-case hexadecimal characters followed by a
 * newline character.
 */
#define FILE_LENGTH 33

struct RecorderFixture
{
  EmtrEventRecorder *recorder;
};

static gboolean
write_testing_machine_id ()
{
  GFile *file = g_file_new_for_path (TESTING_FILE_PATH);
  gboolean success = g_file_replace_contents (file,
                                              TESTING_ID,
                                              FILE_LENGTH,
                                              NULL, 
                                              FALSE, 
                                              G_FILE_CREATE_REPLACE_DESTINATION | G_FILE_CREATE_PRIVATE,
                                              NULL, 
                                              NULL, 
                                              NULL);
  if (!success)
    g_critical ("Testing code failed to write testing machine id.\n");
  g_object_unref (file);
  return success;
}

static void
setup (struct RecorderFixture *fixture,
       gconstpointer           unused)
{
  write_testing_machine_id ();
  fixture->recorder = emtr_event_recorder_new ();
}

static void
teardown (struct RecorderFixture *fixture,
          gconstpointer           unused)
{
  g_object_unref (fixture->recorder);
}

static void
test_event_recorder_new_succeeds (struct RecorderFixture *fixture,
                                  gconstpointer           unused)
{
    g_assert (fixture->recorder != NULL);
}

static void
test_event_recorder_get_default_is_singleton (void)
{
  write_testing_machine_id ();
  EmtrEventRecorder *event_recorder1 = emtr_event_recorder_get_default ();
  EmtrEventRecorder *event_recorder2 = emtr_event_recorder_get_default ();
  g_assert (event_recorder1 == event_recorder2);
  // A singleton shouldn't be unref'd.
}

static void
test_event_recorder_record_event (struct RecorderFixture *fixture,
                                  gconstpointer           unused)
{
  emtr_event_recorder_record_event (fixture->recorder, MEANINGLESS_EVENT, NULL);
}

static void
test_event_recorder_record_events (struct RecorderFixture *fixture,
                                   gconstpointer           unused)
{
  emtr_event_recorder_record_events (fixture->recorder, MEANINGLESS_EVENT,
                                     G_GINT64_CONSTANT (12), NULL);
}

static void
test_event_recorder_record_start_stop (struct RecorderFixture *fixture,
                                       gconstpointer           unused)
{
  emtr_event_recorder_record_start (fixture->recorder, MEANINGLESS_EVENT, NULL,
                                    NULL);
  emtr_event_recorder_record_stop (fixture->recorder, MEANINGLESS_EVENT, NULL,
                                   NULL);
}

static void
test_event_recorder_record_progress (struct RecorderFixture *fixture,
                                     gconstpointer           unused)
{
  emtr_event_recorder_record_start (fixture->recorder, MEANINGLESS_EVENT, NULL,
                                    NULL);
  emtr_event_recorder_record_progress (fixture->recorder, MEANINGLESS_EVENT, NULL,
                                       NULL);
  emtr_event_recorder_record_stop (fixture->recorder, MEANINGLESS_EVENT, NULL,
                                   NULL);
}

static void
test_event_recorder_record_start_stop_with_key (struct RecorderFixture *fixture,
                                                gconstpointer           unused)
{
  GVariant *key = g_variant_new ("{sd}", "Power Level", 9320.73);
  g_variant_ref_sink (key);
  emtr_event_recorder_record_start (fixture->recorder, MEANINGLESS_EVENT, key,
                                    NULL);
  emtr_event_recorder_record_stop (fixture->recorder, MEANINGLESS_EVENT, key,
                                   NULL);
  g_variant_unref (key);
}

static void
test_event_recorder_record_progress_with_key (struct RecorderFixture *fixture,
                                              gconstpointer           unused)
{
  GVariant *key =
    g_variant_new ("s", "NaNNaNNaNNaNNaNNaNNaNNaNNaNNaNNaNNaNNaN BATMAN!!!");
  g_variant_ref_sink (key);
  emtr_event_recorder_record_start (fixture->recorder, MEANINGLESS_EVENT, key,
                                    NULL);
  for (gint i = 0; i < 10; ++i)
   {
    emtr_event_recorder_record_progress (fixture->recorder, MEANINGLESS_EVENT, key,
                                         NULL);
   }
  emtr_event_recorder_record_stop (fixture->recorder, MEANINGLESS_EVENT, key,
                                   NULL);
  g_variant_unref (key);
}

static void
test_event_recorder_record_start_stop_with_floating_key (struct RecorderFixture *fixture,
                                                         gconstpointer           unused)
{
  emtr_event_recorder_record_start (fixture->recorder, MEANINGLESS_EVENT,
                                    g_variant_new ("i", 6170), NULL);
  emtr_event_recorder_record_stop (fixture->recorder, MEANINGLESS_EVENT,
                                   g_variant_new ("i", 6170), NULL);
}

static void
test_event_recorder_record_progress_with_floating_key (struct RecorderFixture *fixture,
                                                       gconstpointer           unused)
{
  emtr_event_recorder_record_start (fixture->recorder, MEANINGLESS_EVENT,
                                    g_variant_new ("mv", NULL), NULL);
  emtr_event_recorder_record_progress (fixture->recorder, MEANINGLESS_EVENT,
                                       g_variant_new ("mv", NULL), NULL);
  emtr_event_recorder_record_stop (fixture->recorder, MEANINGLESS_EVENT,
                                   g_variant_new ("mv", NULL), NULL);
}

static void
test_event_recorder_record_auxiliary_payload (struct RecorderFixture *fixture,
                                              gconstpointer           unused)
{
  emtr_event_recorder_record_event (fixture->recorder, MEANINGLESS_EVENT,
                                    g_variant_new ("b", TRUE));
  emtr_event_recorder_record_events (fixture->recorder, MEANINGLESS_EVENT,
                                     G_GINT64_CONSTANT (7),
                                     g_variant_new ("b", FALSE));
  emtr_event_recorder_record_start (fixture->recorder, MEANINGLESS_EVENT, NULL,
                                    g_variant_new ("d", 5812.512));
  emtr_event_recorder_record_progress (fixture->recorder, MEANINGLESS_EVENT, NULL,
                                       g_variant_new ("d", -12.0));
  emtr_event_recorder_record_stop (fixture->recorder, MEANINGLESS_EVENT, NULL,
                                       g_variant_new ("(xt)",
                                       G_GINT64_CONSTANT (-82),
                                       G_GUINT64_CONSTANT (19)));
}

static void
test_event_recorder_record_multiple_metric_sequences (struct RecorderFixture *fixture,
                                                      gconstpointer           unused)
{
  GVariant *key = g_variant_new ("^ay", "Anna Breytenbach, Animal Whisperer");
  g_variant_ref_sink (key);
  emtr_event_recorder_record_start (fixture->recorder, MEANINGLESS_EVENT, key,
                                    NULL);
  emtr_event_recorder_record_progress (fixture->recorder, MEANINGLESS_EVENT, key,
                                       NULL);
  emtr_event_recorder_record_start (fixture->recorder, MEANINGLESS_EVENT_2, key,
                                    NULL);
  emtr_event_recorder_record_progress (fixture->recorder, MEANINGLESS_EVENT_2, key,
                                       NULL);
  emtr_event_recorder_record_stop (fixture->recorder, MEANINGLESS_EVENT_2, key,
                                   NULL);
  emtr_event_recorder_record_stop (fixture->recorder, MEANINGLESS_EVENT, key,
                                   NULL);
  g_variant_unref (key);
}

static void
test_event_recorder_record_auxiliary_payload_with_maybe_throws_critical (struct RecorderFixture *fixture,
                                                                         gconstpointer           unused)
{
  const gchar *error_message = "*Maybe type found in auxiliary payload. These "
                               "are not compatible with DBus!*";

  g_test_expect_message (EOS_METRICS_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL,
                         error_message);
  emtr_event_recorder_record_event (fixture->recorder, MEANINGLESS_EVENT,
                                    g_variant_new ("mb", TRUE));
  g_test_assert_expected_messages ();

  g_test_expect_message (EOS_METRICS_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL,
                         error_message);
  emtr_event_recorder_record_events (fixture->recorder, MEANINGLESS_EVENT,
                                     G_GINT64_CONSTANT (7),
                                     g_variant_new ("mb", NULL));
  g_test_assert_expected_messages ();

  g_test_expect_message (EOS_METRICS_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL,
                         error_message);
  emtr_event_recorder_record_start (fixture->recorder, MEANINGLESS_EVENT, NULL,
                                    g_variant_new ("md", 5812.512));
  g_test_assert_expected_messages ();

  g_test_expect_message (EOS_METRICS_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL,
                         error_message);
  emtr_event_recorder_record_progress (fixture->recorder, MEANINGLESS_EVENT, NULL,
                                       g_variant_new ("md", -12.0));
  g_test_assert_expected_messages ();

  g_test_expect_message (EOS_METRICS_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL,
                         error_message);
  emtr_event_recorder_record_stop (fixture->recorder, MEANINGLESS_EVENT, NULL,
                                   g_variant_new ("(xmt)",
                                                  G_GINT64_CONSTANT (-82),
                                                  G_GUINT64_CONSTANT (19)));
  g_test_assert_expected_messages ();
}

int
main (int    argc,
      char **argv)
{
  g_test_init (&argc, &argv, NULL);

#define ADD_RECORDER_TEST_FUNC(path, func) \
  g_test_add ((path), struct RecorderFixture, NULL, setup, (func), teardown)

  ADD_RECORDER_TEST_FUNC ("/event-recorder/new-succeeds",
                          test_event_recorder_new_succeeds);
  g_test_add_func ("/event-recorder/get-default-is-singleton", 
                   test_event_recorder_get_default_is_singleton);
  ADD_RECORDER_TEST_FUNC ("/event-recorder/record-event",
                          test_event_recorder_record_event);
  ADD_RECORDER_TEST_FUNC ("/event-recorder/record-events",
                          test_event_recorder_record_events);
  ADD_RECORDER_TEST_FUNC ("/event-recorder/record-start-stop",
                          test_event_recorder_record_start_stop);
  ADD_RECORDER_TEST_FUNC ("/event-recorder/record-progress",
                          test_event_recorder_record_progress);
  ADD_RECORDER_TEST_FUNC ("/event-recorder/record-start-stop-with-key",
                          test_event_recorder_record_start_stop_with_key);
  ADD_RECORDER_TEST_FUNC ("/event-recorder/record-progress-with-key",
                          test_event_recorder_record_progress_with_key);
  ADD_RECORDER_TEST_FUNC ("/event-recorder/record-start-stop-with-floating-key",
                          test_event_recorder_record_start_stop_with_floating_key);
  ADD_RECORDER_TEST_FUNC ("/event-recorder/record-progress-with-floating-key",
                          test_event_recorder_record_progress_with_floating_key);
  ADD_RECORDER_TEST_FUNC ("/event-recorder/record-auxiliary-payload",
                          test_event_recorder_record_auxiliary_payload);
  ADD_RECORDER_TEST_FUNC ("/event-recorder/record-multiple-metric-sequences",
                          test_event_recorder_record_multiple_metric_sequences);
  ADD_RECORDER_TEST_FUNC ("/event-recorder/record-auxiliary-payload-with-maybe-throws-critical",
                          test_event_recorder_record_auxiliary_payload_with_maybe_throws_critical);

#undef ADD_RECORDER_TEST_FUNC

  return g_test_run ();
}

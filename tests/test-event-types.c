/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2014 Endless Mobile, Inc.
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

#include <glib.h>
#include "eosmetrics/eosmetrics.h"

#define REAL_EVENT EMTR_EVENT_USER_IS_LOGGED_IN
#define FAKE_BUT_VALID_EVENT "3c9478e8-028e-46d7-95fe-f86e71f95f3f"
#define FAKE_AND_INVALID_EVENT "abracada-braa-laka-zami-amazombiehah"

static void
test_event_id_to_name_works_with_valid_input (void)
{
  const gchar *event_name;
  g_assert (emtr_event_id_to_name (REAL_EVENT, &event_name));
}

static void
test_event_id_to_name_handles_unknown_event_ids (void)
{
  g_test_expect_message (G_LOG_DOMAIN, G_LOG_LEVEL_WARNING,
                         "*Unknown ID Given*");
  const gchar *event_name;
  g_assert_false (emtr_event_id_to_name (FAKE_BUT_VALID_EVENT,
                                                     &event_name));
  g_test_assert_expected_messages ();
}

static void
test_event_id_to_name_handles_invalid_event_ids (void)
{
  g_test_expect_message (G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL,
                         "*Invalid event ID given*");
  const gchar *event_name;
  g_assert_false (emtr_event_id_to_name (FAKE_AND_INVALID_EVENT,
                                                     &event_name));
  g_test_assert_expected_messages ();
}

int
main (int                argc,
      const char * const argv[])
{
  g_test_init (&argc, (char ***) &argv, NULL);

  g_test_add_func ("/event-types/id-to-name-works-with-valid-input",
                   test_event_id_to_name_works_with_valid_input);
  g_test_add_func ("/event-types/id-to-name-handles-unknown-event-ids",
                   test_event_id_to_name_handles_unknown_event_ids);
  g_test_add_func ("/event-types/id-to-name-handles-invalid-event-ids",
                   test_event_id_to_name_handles_invalid_event_ids);

  return g_test_run ();
}

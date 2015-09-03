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

#include <glib.h>

#define EMTR_DISABLE_DEPRECATION_WARNINGS
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

gint
main (gint                argc,
      const gchar * const argv[])
{
  g_test_init (&argc, (gchar ***) &argv, NULL);

  g_test_add_func ("/event-types/id-to-name-works-with-valid-input",
                   test_event_id_to_name_works_with_valid_input);
  g_test_add_func ("/event-types/id-to-name-handles-unknown-event-ids",
                   test_event_id_to_name_handles_unknown_event_ids);
  g_test_add_func ("/event-types/id-to-name-handles-invalid-event-ids",
                   test_event_id_to_name_handles_invalid_event_ids);

  return g_test_run ();
}

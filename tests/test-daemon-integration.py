# Copyright 2014, 2015 Endless Mobile, Inc.

# This file is part of eos-metrics.
#
# eos-metrics is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published
# by the Free Software Foundation, either version 2.1 of the License, or
# (at your option) any later version.
#
# eos-metrics is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with eos-metrics.  If not, see
# <http://www.gnu.org/licenses/>.

import dbus
import dbusmock
import dbus.mainloop.glib
import gi
import os
import subprocess
import time
import unittest
import uuid

gi.require_version('EosMetrics', '0')

from gi.repository import EosMetrics
from gi.repository import GLib


class TestDaemonIntegration(dbusmock.DBusTestCase):
    _MOCK_EVENT_NOTHING_HAPPENED = '5071dd96-bdad-4ee5-9c26-3dfef34a9963'
    _MOCK_EVENT_NOTHING_HAPPENED_UUID = uuid.UUID(_MOCK_EVENT_NOTHING_HAPPENED)
    _METRICS_BUS_NAME = 'com.endlessm.Metrics'
    _METRICS_OBJECT_PATH = '/com/endlessm/Metrics'
    _METRICS_IFACE = 'com.endlessm.Metrics.EventRecorderServer'
    _TIMER_OBJECT_PATH = "/com/endlessm/Metrics/Timer"
    _TIMER_IFACE = "com.endlessm.Metrics.AggregateTimer"

    """
    Makes sure that the app-facing EosMetrics.EventRecorder interface calls
    the com.endlessm.Metrics.EventRecorder D-Bus interface and marshals all its
    arguments properly.
    """
    @classmethod
    def setUpClass(klass):
        """Set up mainloop for blocking on D-Bus send events so that we can test
        asynchronous calls."""
        dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)

        """Set up a mock system bus."""
        # The dbusmock library generates its own D-Bus configuration when
        # using DBusTestCase.start_system_bus, which breaks in environments
        # where D-Bus needs special configuration, such as our buildstream
        # test environment. Instead, we will use start_session_bus and treat
        # it like a system bus in a similar way to dbusmock.
        klass.start_session_bus()
        os.environ['DBUS_SYSTEM_BUS_ADDRESS'] = os.environ['DBUS_SESSION_BUS_ADDRESS']
        klass.dbus_con = klass.get_dbus(system_bus=True)

    def setUp(self):
        self.dbus_mock = \
            self.spawn_server(self._METRICS_BUS_NAME, self._METRICS_OBJECT_PATH,
                              self._METRICS_IFACE, system_bus=True,
                              stdout=subprocess.PIPE)

        event_recorder_daemon_dbus_object = \
            self.dbus_con.get_object(self._METRICS_BUS_NAME,
                                     self._METRICS_OBJECT_PATH)
        self.interface_mock = \
            dbus.Interface(event_recorder_daemon_dbus_object,
                           dbusmock.MOCK_IFACE)

        self.dbus_con.add_signal_receiver(self.handle_dbus_event_received,
                                          signal_name='MethodCalled',
                                          dbus_interface=dbusmock.MOCK_IFACE,
                                          bus_name=self._METRICS_BUS_NAME,
                                          path=self._METRICS_OBJECT_PATH)
        self.dbus_con.add_signal_receiver(self.handle_dbus_event_received,
                                          signal_name='MethodCalled',
                                          dbus_interface=dbusmock.MOCK_IFACE,
                                          bus_name=self._METRICS_BUS_NAME,
                                          path=self._TIMER_OBJECT_PATH)

        self.interface_mock.AddMethod('', 'RecordSingularEvent', 'uayxbv',
                                      '', '')
        self.interface_mock.AddMethod('', 'RecordAggregateEvent', 'uayxxbv',
                                      '', '')
        self.interface_mock.AddMethod('', 'RecordEventSequence', 'uaya(xbv)',
                                      '', '')
        self.interface_mock.AddMethod('', 'StartAggregateTimer', 'uaybv', 'o',
                                      f"ret = '{self._TIMER_OBJECT_PATH}'")

        self.interface_mock.AddObject(
            self._TIMER_OBJECT_PATH,
            self._TIMER_IFACE,
            {},
            [
                ("StopTimer", "", "", ""),
            ],
        )

        self.event_recorder = EosMetrics.EventRecorder()
        self.interface_mock.ClearCalls()
        self.mainloop = GLib.MainLoop()
        self._quit_on_method = ''

    def tearDown(self):
        self.dbus_con.remove_signal_receiver(self.handle_dbus_event_received,
                                             signal_name='MethodCalled')
        self.dbus_mock.terminate()
        self.dbus_mock.wait()

    def await_method_call(self, method_name, timeout_seconds=20):
        """Quit the main loop when the D-Bus method @method_name is called.
        Timeout after waiting for 'timeout_seconds'. Use like this:
            self.await_method_call('MyMethod')
            # now MyMethod has been called
        """
        self._quit_on_method = method_name
        self._timed_out = False
        def _timeout():
            self._timed_out = True
            self.mainloop.quit()
            return False
        timeout_id = GLib.timeout_add_seconds(timeout_seconds, _timeout)
        self.mainloop.run()
        if self._timed_out:
            self.fail(f"Test timed out after waiting {timeout_seconds} seconds for D-Bus method call.")
        else:
            GLib.Source.remove(timeout_id)
            return self.interface_mock.GetCalls()

    def handle_dbus_event_received(self, name, *args):
        if name == self._quit_on_method:
            self._quit_on_method = ''
            self.mainloop.quit()

    def call_singular_event(self, payload=None):
        self.event_recorder.record_event(self._MOCK_EVENT_NOTHING_HAPPENED,
                                         payload)
        return self.await_method_call('RecordSingularEvent')

    def call_singular_event_sync(self, payload=None):
        self.event_recorder.record_event_sync(self._MOCK_EVENT_NOTHING_HAPPENED,
                                              payload)
        return self.interface_mock.GetCalls()

    def call_aggregate_event(self, num_events=2, payload=None):
        self.event_recorder.record_events(self._MOCK_EVENT_NOTHING_HAPPENED,
                                          num_events, payload)
        return self.await_method_call('RecordAggregateEvent')

    def call_aggregate_event_sync(self, num_events=2, payload=None):
        self.event_recorder.record_events_sync(self._MOCK_EVENT_NOTHING_HAPPENED,
                                               num_events, payload)
        return self.interface_mock.GetCalls()

    def call_start_stop_event(self, payload_start=None, payload_stop=None):
        self.event_recorder.record_start(self._MOCK_EVENT_NOTHING_HAPPENED,
                                         None, payload_start)
        self.event_recorder.record_stop(self._MOCK_EVENT_NOTHING_HAPPENED,
                                        None, payload_stop)
        return self.await_method_call('RecordEventSequence')

    def call_start_stop_event_sync(self, payload_start=None,
                                   payload_stop=None):
        self.event_recorder.record_start(self._MOCK_EVENT_NOTHING_HAPPENED,
                                         None, payload_start)
        self.event_recorder.record_stop_sync(self._MOCK_EVENT_NOTHING_HAPPENED,
                                             None, payload_stop)
        return self.interface_mock.GetCalls()

    def call_start_progress_stop_event(self,
                                       payload_start=None,
                                       payload_progress=None,
                                       payload_stop=None):
        self.event_recorder.record_start(self._MOCK_EVENT_NOTHING_HAPPENED,
                                         None, payload_start)
        self.event_recorder.record_progress(self._MOCK_EVENT_NOTHING_HAPPENED,
                                            None, payload_progress)
        self.event_recorder.record_stop(self._MOCK_EVENT_NOTHING_HAPPENED,
                                        None, payload_stop)
        return self.await_method_call('RecordEventSequence')

    def call_start_progress_stop_event_sync(self,
                                            payload_start=None,
                                            payload_progress=None,
                                            payload_stop=None):
        self.event_recorder.record_start(self._MOCK_EVENT_NOTHING_HAPPENED,
                                         None, payload_start)
        self.event_recorder.record_progress(self._MOCK_EVENT_NOTHING_HAPPENED,
                                            None, payload_progress)
        self.event_recorder.record_stop_sync(self._MOCK_EVENT_NOTHING_HAPPENED,
                                             None, payload_stop)
        return self.interface_mock.GetCalls()

    def call_start_timer(self, payload=None):
        timer = self.event_recorder.start_aggregate_timer(self._MOCK_EVENT_NOTHING_HAPPENED,
                                                          payload)
        return timer, self.await_method_call("StartAggregateTimer")

    def call_start_timer_with_uid(self, uid, payload=None):
        timer = self.event_recorder.start_aggregate_timer_with_uid(
            uid,
            self._MOCK_EVENT_NOTHING_HAPPENED,
            payload,
        )
        return timer, self.await_method_call("StartAggregateTimer")

    # Recorder calls D-Bus at all.
    def test_record_singular_event_calls_dbus(self):
        calls = self.call_singular_event()
        self.assertEqual(len(calls), 1)
        self.assertEqual(calls[0][1], 'RecordSingularEvent')

    def test_record_singular_event_sync_calls_dbus(self):
        calls = self.call_singular_event_sync()
        self.assertEqual(len(calls), 1)
        self.assertEqual(calls[0][1], 'RecordSingularEvent')

    def test_record_aggregate_event_calls_dbus(self):
        calls = self.call_aggregate_event()
        self.assertEqual(len(calls), 1)
        self.assertEqual(calls[0][1], 'RecordAggregateEvent')

    def test_record_aggregate_event_sync_calls_dbus(self):
        calls = self.call_aggregate_event_sync()
        self.assertEqual(len(calls), 1)
        self.assertEqual(calls[0][1], 'RecordAggregateEvent')

    def test_record_event_sequence_calls_dbus(self):
        calls = self.call_start_stop_event()
        self.assertEqual(len(calls), 1)  # D-Bus is only called from "stop".
        self.assertEqual(calls[0][1], 'RecordEventSequence')

    def test_record_event_sequence_sync_calls_dbus(self):
        calls = self.call_start_stop_event_sync()
        self.assertEqual(len(calls), 1)  # D-Bus is only called from "stop".
        self.assertEqual(calls[0][1], 'RecordEventSequence')

    # User Id isn't garbled.
    def test_record_singular_event_passes_uid(self):
        calls = self.call_singular_event()
        self.assertEqual(calls[0][2][0], os.getuid())

    def test_record_singular_event_sync_passes_uid(self):
        calls = self.call_singular_event_sync()
        self.assertEqual(calls[0][2][0], os.getuid())

    def test_record_aggregate_event_passes_uid(self):
        calls = self.call_aggregate_event()
        self.assertEqual(calls[0][2][0], os.getuid())

    def test_record_aggregate_event_sync_passes_uid(self):
        calls = self.call_aggregate_event_sync()
        self.assertEqual(calls[0][2][0], os.getuid())

    def test_record_event_sequence_passes_uid(self):
        calls = self.call_start_stop_event()
        self.assertEqual(calls[0][2][0], os.getuid())

    def test_record_event_sequence_sync_passes_uid(self):
        calls = self.call_start_stop_event_sync()
        self.assertEqual(calls[0][2][0], os.getuid())

    def dbus_bytes_to_uuid(self, dbus_byte_array):
        bytes_as_bytes = bytes(dbus_byte_array)
        return uuid.UUID(bytes=bytes_as_bytes)

    # Event Id is't garbled.
    def test_record_singular_event_passes_event_id(self):
        calls = self.call_singular_event()
        actual_uuid = self.dbus_bytes_to_uuid(calls[0][2][1])
        self.assertEqual(self._MOCK_EVENT_NOTHING_HAPPENED_UUID, actual_uuid)

    def test_record_singular_event_sync_passes_event_id(self):
        calls = self.call_singular_event_sync()
        actual_uuid = self.dbus_bytes_to_uuid(calls[0][2][1])
        self.assertEqual(self._MOCK_EVENT_NOTHING_HAPPENED_UUID, actual_uuid)

    def test_record_aggregate_event_passes_event_id(self):
        calls = self.call_aggregate_event()
        actual_uuid = self.dbus_bytes_to_uuid(calls[0][2][1])
        self.assertEqual(self._MOCK_EVENT_NOTHING_HAPPENED_UUID, actual_uuid)

    def test_record_aggregate_event_sync_passes_event_id(self):
        calls = self.call_aggregate_event_sync()
        actual_uuid = self.dbus_bytes_to_uuid(calls[0][2][1])
        self.assertEqual(self._MOCK_EVENT_NOTHING_HAPPENED_UUID, actual_uuid)

    def test_record_event_sequence_passes_event_id(self):
        calls = self.call_start_stop_event()
        actual_uuid = self.dbus_bytes_to_uuid(calls[0][2][1])
        self.assertEqual(self._MOCK_EVENT_NOTHING_HAPPENED_UUID, actual_uuid)

    def test_record_event_sequence_sync_passes_event_id(self):
        calls = self.call_start_stop_event_sync()
        actual_uuid = self.dbus_bytes_to_uuid(calls[0][2][1])
        self.assertEqual(self._MOCK_EVENT_NOTHING_HAPPENED_UUID, actual_uuid)

    # Aggregated events' count isn't garbled.
    def test_record_aggregate_event_passes_event_count(self):
        leet_count = 1337
        calls = self.call_aggregate_event(num_events=leet_count)
        self.assertEqual(calls[0][2][2], leet_count)

    def test_record_aggregate_event_sync_passes_event_count(self):
        leet_count = 1337
        calls = self.call_aggregate_event_sync(num_events=leet_count)
        self.assertEqual(calls[0][2][2], leet_count)

    # Timestamps are monotonically increasing.
    def test_record_singular_event_has_increasing_relative_timestamp(self):
        calls = self.call_singular_event()
        first_time = calls[0][2][2]
        self.interface_mock.ClearCalls()
        calls = self.call_singular_event()
        second_time = calls[0][2][2]
        self.assertLessEqual(first_time, second_time)

    def test_record_singular_event_sync_has_increasing_relative_timestamp(self):
        calls = self.call_singular_event_sync()
        first_time = calls[0][2][2]
        self.interface_mock.ClearCalls()
        calls = self.call_singular_event_sync()
        second_time = calls[0][2][2]
        self.assertLessEqual(first_time, second_time)

    def test_record_aggregate_event_has_increasing_relative_timestamp(self):
        calls = self.call_aggregate_event()
        first_time = calls[0][2][3]
        self.interface_mock.ClearCalls()
        calls = self.call_aggregate_event()
        second_time = calls[0][2][3]
        self.assertLessEqual(first_time, second_time)

    def test_record_aggregate_event_sync_has_increasing_relative_timestamp(self):
        calls = self.call_aggregate_event_sync()
        first_time = calls[0][2][3]
        self.interface_mock.ClearCalls()
        calls = self.call_aggregate_event_sync()
        second_time = calls[0][2][3]
        self.assertLessEqual(first_time, second_time)

    def test_record_event_sequence_has_increasing_relative_timestamp(self):
        calls = self.call_start_stop_event()
        first_time = calls[0][2][2][0][0]
        second_time = calls[0][2][2][1][0]
        self.interface_mock.ClearCalls()
        calls = self.call_start_stop_event()
        third_time = calls[0][2][2][0][0]
        fourth_time = calls[0][2][2][1][0]
        self.assertLessEqual(first_time, second_time)
        self.assertLessEqual(second_time, third_time)
        self.assertLessEqual(third_time, fourth_time)

    def test_record_event_sequence_sync_has_increasing_relative_timestamp(self):
        calls = self.call_start_stop_event_sync()
        first_time = calls[0][2][2][0][0]
        second_time = calls[0][2][2][1][0]
        self.interface_mock.ClearCalls()
        calls = self.call_start_stop_event_sync()
        third_time = calls[0][2][2][0][0]
        fourth_time = calls[0][2][2][1][0]
        self.assertLessEqual(first_time, second_time)
        self.assertLessEqual(second_time, third_time)
        self.assertLessEqual(third_time, fourth_time)

    # The difference between two relative timestamps is non-negative and less
    # than that of two surrounding monotonic timestamps.
    def test_record_singular_event_has_reasonable_relative_timestamp(self):
        monotonic_time_first = time.clock_gettime(time.CLOCK_MONOTONIC)

        calls = self.call_singular_event()
        relative_time_first = calls[0][2][2]
        self.interface_mock.ClearCalls()
        calls = self.call_singular_event()
        relative_time_second = calls[0][2][2]

        monotonic_time_second = time.clock_gettime(time.CLOCK_MONOTONIC)

        relative_time_difference = relative_time_second - relative_time_first
        self.assertLessEqual(0, relative_time_difference)
        monotonic_time_difference = 1e9 * \
            (monotonic_time_second - monotonic_time_first)
        self.assertLessEqual(relative_time_difference,
                             monotonic_time_difference)

    def test_record_singular_event_sync_has_reasonable_relative_timestamp(self):
        monotonic_time_first = time.clock_gettime(time.CLOCK_MONOTONIC)

        calls = self.call_singular_event_sync()
        relative_time_first = calls[0][2][2]
        self.interface_mock.ClearCalls()
        calls = self.call_singular_event_sync()
        relative_time_second = calls[0][2][2]

        monotonic_time_second = time.clock_gettime(time.CLOCK_MONOTONIC)

        relative_time_difference = relative_time_second - relative_time_first
        self.assertLessEqual(0, relative_time_difference)
        monotonic_time_difference = 1e9 * \
            (monotonic_time_second - monotonic_time_first)
        self.assertLessEqual(relative_time_difference,
                             monotonic_time_difference)

    def test_record_aggregate_event_has_reasonable_relative_timestamp(self):
        monotonic_time_first = time.clock_gettime(time.CLOCK_MONOTONIC)

        calls = self.call_aggregate_event()
        relative_time_first = calls[0][2][3]
        self.interface_mock.ClearCalls()
        calls = self.call_aggregate_event()
        relative_time_second = calls[0][2][3]

        monotonic_time_second = time.clock_gettime(time.CLOCK_MONOTONIC)

        relative_time_difference = relative_time_second - relative_time_first
        self.assertLessEqual(0, relative_time_difference)
        monotonic_time_difference = 1e9 * \
            (monotonic_time_second - monotonic_time_first)
        self.assertLessEqual(relative_time_difference,
                             monotonic_time_difference)

    def test_record_aggregate_event_sync_has_reasonable_relative_timestamp(self):
        monotonic_time_first = time.clock_gettime(time.CLOCK_MONOTONIC)

        calls = self.call_aggregate_event_sync()
        relative_time_first = calls[0][2][3]
        self.interface_mock.ClearCalls()
        calls = self.call_aggregate_event_sync()
        relative_time_second = calls[0][2][3]

        monotonic_time_second = time.clock_gettime(time.CLOCK_MONOTONIC)

        relative_time_difference = relative_time_second - relative_time_first
        self.assertLessEqual(0, relative_time_difference)
        monotonic_time_difference = 1e9 * \
            (monotonic_time_second - monotonic_time_first)
        self.assertLessEqual(relative_time_difference,
                             monotonic_time_difference)

    def test_record_event_sequence_has_reasonable_relative_timestamp(self):
        monotonic_time_first = time.clock_gettime(time.CLOCK_MONOTONIC)

        calls = self.call_start_stop_event()
        relative_time_first = calls[0][2][2][0][0]
        self.interface_mock.ClearCalls()
        calls = self.call_start_stop_event()
        relative_time_second = calls[0][2][2][1][0]

        monotonic_time_second = time.clock_gettime(time.CLOCK_MONOTONIC)

        relative_time_difference = relative_time_second - relative_time_first
        self.assertLessEqual(0, relative_time_difference)
        monotonic_time_difference = 1e9 * \
            (monotonic_time_second - monotonic_time_first)
        self.assertLessEqual(relative_time_difference,
                             monotonic_time_difference)

    def test_record_event_sequence_sync_has_reasonable_relative_timestamp(self):
        monotonic_time_first = time.clock_gettime(time.CLOCK_MONOTONIC)

        calls = self.call_start_stop_event_sync()
        relative_time_first = calls[0][2][2][0][0]
        self.interface_mock.ClearCalls()
        calls = self.call_start_stop_event_sync()
        relative_time_second = calls[0][2][2][1][0]

        monotonic_time_second = time.clock_gettime(time.CLOCK_MONOTONIC)

        relative_time_difference = relative_time_second - relative_time_first
        self.assertLessEqual(0, relative_time_difference)
        monotonic_time_difference = 1e9 * \
            (monotonic_time_second - monotonic_time_first)
        self.assertLessEqual(relative_time_difference,
                             monotonic_time_difference)

    # The maybe type is emulated correctly for empty payloads.
    def test_record_singular_event_maybe_flag_is_false_when_payload_is_empty(self):
        calls = self.call_singular_event(payload=None)
        self.assertEqual(calls[0][2][3], False)

    def test_record_singular_event_sync_maybe_flag_is_false_when_payload_is_empty(self):
        calls = self.call_singular_event_sync(payload=None)
        self.assertEqual(calls[0][2][3], False)

    def test_record_aggregate_event_maybe_flag_is_false_when_payload_is_empty(self):
        calls = self.call_aggregate_event(payload=None)
        self.assertEqual(calls[0][2][4], False)

    def test_record_aggregate_event_sync_maybe_flag_is_false_when_payload_is_empty(self):
        calls = self.call_aggregate_event_sync(payload=None)
        self.assertEqual(calls[0][2][4], False)

    def test_record_event_sequence_maybe_flags_are_false_when_payloads_are_empty(self):
        calls = self.call_start_progress_stop_event()
        self.assertEqual(calls[0][2][2][0][1], False)
        self.assertEqual(calls[0][2][2][1][1], False)
        self.assertEqual(calls[0][2][2][2][1], False)

    def test_record_event_sequence_sync_maybe_flags_are_false_when_payloads_are_empty(self):
        calls = self.call_start_progress_stop_event_sync()
        self.assertEqual(calls[0][2][2][0][1], False)
        self.assertEqual(calls[0][2][2][1][1], False)
        self.assertEqual(calls[0][2][2][2][1], False)

    # The maybe type is emulated correctly for non-empty payloads.
    def test_record_singluar_event_maybe_flag_is_true_when_payload_is_not_empty(self):
        # Contains both a Matt and not a Matt until viewed.
        payload = GLib.Variant.new_string("Quantum Dalio")
        calls = self.call_singular_event(payload=payload)
        self.assertEqual(calls[0][2][3], True)

    def test_record_singluar_event_sync_maybe_flag_is_true_when_payload_is_not_empty(self):
        payload = GLib.Variant.new_string("Schrodinger's Ghost")
        calls = self.call_singular_event_sync(payload=payload)
        self.assertEqual(calls[0][2][3], True)

    def test_record_aggregate_event_maybe_flag_is_true_when_payload_is_not_empty(self):
        payload = GLib.Variant.new_string("Fiscally Responsible Mime")
        calls = self.call_aggregate_event(payload=payload)
        self.assertEqual(calls[0][2][4], True)

    def test_record_aggregate_event_sync_maybe_flag_is_true_when_payload_is_not_empty(self):
        payload = GLib.Variant.new_string("MIME Type: Fiscally Responsible")
        calls = self.call_aggregate_event_sync(payload=payload)
        self.assertEqual(calls[0][2][4], True)

    def test_record_event_sequence_maybe_flag_is_true_when_payload_is_not_empty(self):
        payload_start = GLib.Variant.new_string("Flagrant n00b")
        payload_progress = GLib.Variant.new_string("Murphy")
        payload_stop = GLib.Variant.new_string("What's that blue thing?")
        calls = self.call_start_progress_stop_event(payload_start,
                                                    payload_progress,
                                                    payload_stop)

        # +5 Bonus points if you know where that last one comes from.
        self.assertEqual(calls[0][2][2][0][1], True)
        self.assertEqual(calls[0][2][2][1][1], True)
        self.assertEqual(calls[0][2][2][2][1], True)

    def test_record_event_sequence_sync_maybe_flag_is_true_when_payload_is_not_empty(self):
        payload_start = GLib.Variant.new_string("Jimminy Cricket")
        payload_progress = GLib.Variant.new_string("Had a bad day")
        payload_stop = GLib.Variant.new_string("What was he thinking?")
        calls = self.call_start_progress_stop_event_sync(payload_start,
                                                         payload_progress,
                                                         payload_stop)

        self.assertEqual(calls[0][2][2][0][1], True)
        self.assertEqual(calls[0][2][2][1][1], True)
        self.assertEqual(calls[0][2][2][2][1], True)

    # The payloads are not garbled.
    def test_record_singular_event_passes_payload(self):
        string = "Occam's Razor"
        # It has gotten dull with use.
        payload = GLib.Variant.new_string(string)
        calls = self.call_singular_event(payload=payload)
        self.assertEqual(calls[0][2][4], string)

    def test_record_singular_event_sync_passes_payload(self):
        string = "Occam's Shaving Cream"
        payload = GLib.Variant.new_string(string)
        calls = self.call_singular_event_sync(payload=payload)
        self.assertEqual(calls[0][2][4], string)

    def test_record_aggregate_event_passes_payload(self):
        string = "Prince Rupert's Drop"
        # If you haven't seen this, you need to.
        payload = GLib.Variant.new_string(string)
        calls = self.call_aggregate_event(payload=payload)
        self.assertEqual(calls[0][2][5], string)

    def test_record_aggregate_event_sync_passes_payload(self):
        string = "Hercules (1983)"
        payload = GLib.Variant.new_string(string)
        calls = self.call_aggregate_event_sync(payload=payload)
        self.assertEqual(calls[0][2][5], string)

    def test_record_event_sequence_passes_payloads(self):
        start_string = "I am a jelly donut(sic)."
        start_payload = GLib.Variant.new_string(start_string)
        progress_string = "It's in a better place."
        progress_payload = GLib.Variant.new_string(progress_string)
        stop_string = "How dare you dodge the barrel!"
        stop_payload = GLib.Variant.new_string(stop_string)
        calls = self.call_start_progress_stop_event(start_payload,
                                                    progress_payload,
                                                    stop_payload)

        self.assertEqual(calls[0][2][2][0][2], start_string)
        self.assertEqual(calls[0][2][2][1][2], progress_string)
        self.assertEqual(calls[0][2][2][2][2], stop_string)

    def test_record_event_sequence_sync_passes_payloads(self):
        start_string = "What do you suggest?"
        start_payload = GLib.Variant.new_string(start_string)
        progress_string = "Do a barrel roll."
        progress_payload = GLib.Variant.new_string(progress_string)
        stop_string = "Don't mind if I do."
        stop_payload = GLib.Variant.new_string(stop_string)
        calls = self.call_start_progress_stop_event_sync(start_payload,
                                                         progress_payload,
                                                         stop_payload)

        self.assertEqual(calls[0][2][2][0][2], start_string)
        self.assertEqual(calls[0][2][2][1][2], progress_string)
        self.assertEqual(calls[0][2][2][2][2], stop_string)

    def test_start_timer_passes_payload(self):
        payload_string = "com.example.Payload"
        payload_variant = GLib.Variant.new_string(payload_string)
        timer, calls = self.call_start_timer(payload_variant)
        self.assertIsInstance(timer, EosMetrics.AggregateTimer)
        self.assertEqual(calls[0][2][0], os.getuid())
        actual_uuid = self.dbus_bytes_to_uuid(calls[0][2][1])
        self.assertEqual(self._MOCK_EVENT_NOTHING_HAPPENED_UUID, actual_uuid)
        self.assertEqual(calls[0][2][2], True)
        self.assertEqual(calls[0][2][3], payload_string)

        # Now stop it
        timer.stop()
        self.await_method_call("StopTimer")

    def test_start_timer_passes_null_payload(self):
        timer, calls = self.call_start_timer(None)
        self.assertIsInstance(timer, EosMetrics.AggregateTimer)
        self.assertEqual(calls[0][2][2], False)
        self.assertEqual(calls[0][2][3], dbus.Boolean(False, variant_level=1))

        # Now stop it
        timer.stop()
        self.await_method_call("StopTimer")

    def test_timer_calls_stop_when_disposed(self):
        payload_variant = None
        timer, calls = self.call_start_timer(payload_variant)
        self.assertIsInstance(timer, EosMetrics.AggregateTimer)

        del timer
        self.await_method_call("StopTimer")

    def test_start_timer_with_uid_passes_uid(self):
        payload_string = "org.gnome.Builder.desktop"
        payload_variant = GLib.Variant.new_string(payload_string)
        timer, calls = self.call_start_timer_with_uid(0x656f73, payload_variant)
        self.assertIsInstance(timer, EosMetrics.AggregateTimer)
        self.assertEqual(calls[0][2][0], 0x656f73)
        self.assertEqual(calls[0][2][2], True)
        self.assertEqual(calls[0][2][3], dbus.String(payload_string, variant_level=1))

        # Now stop it
        timer.stop()
        self.await_method_call("StopTimer")


if __name__ == '__main__':
    unittest.main()

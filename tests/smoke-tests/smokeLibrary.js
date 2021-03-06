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

let GLib = imports.gi.GLib;

const MOCK_SINGULAR_EVENT_A = 'fb59199e-5384-472e-af1e-00b7a419d5c2';
const MOCK_SINGULAR_EVENT_B = 'b89f9a4a-3035-4fc3-9bef-584367fe2c96';
const MOCK_AGGREGATE_EVENT_A = '9a0cf836-12cd-4887-95d8-e48ccdf6e552';
const MOCK_AGGREGATE_EVENT_B = 'b1f87a3f-a464-48d4-8e35-35dd45659010';
const MOCK_SEQUENCE_EVENT_A = '72fea371-15d1-401d-8a40-c47f379f64fd';
const MOCK_SEQUENCE_EVENT_B = 'b2b17dfd-c30e-4789-abcc-4a38323127f6';

const CONFIGURATION_FILE = '/etc/metrics/eos-metrics-permissions.conf';

GLib.random_set_seed(42);

const randfloat = function (scale) { return GLib.random_double_range(0, scale); };
const randint = function (scale) { return GLib.random_int_range(0, scale); };
const randbool = function () { return GLib.random_int_range(0, 2) === 1; };

const make_key = function () { return new GLib.Variant('d', randfloat(50000)); };

function record_many_events (recorder, iterations, payload_func) {
    if (!environment_is_permitted())
        return;

    if (payload_func === undefined) {
        payload_func = function (x) { return null; };
    }

    for (let i = 0; i < iterations; i++) {
        recorder.record_event(MOCK_SINGULAR_EVENT_A, null);
        recorder.record_event(MOCK_SINGULAR_EVENT_B, payload_func('cows'));

        recorder.record_events(MOCK_AGGREGATE_EVENT_A, randint(5), null);
        recorder.record_events(MOCK_AGGREGATE_EVENT_B, randint(3),
                               payload_func('sand'));

        let key_A = make_key();
        recorder.record_start(MOCK_SEQUENCE_EVENT_A, key_A, null);
        recorder.record_progress(MOCK_SEQUENCE_EVENT_A, key_A, null);
        recorder.record_progress(MOCK_SEQUENCE_EVENT_A, key_A, null);
        recorder.record_stop(MOCK_SEQUENCE_EVENT_A, key_A, null);

        let key_B = make_key();
        recorder.record_start(MOCK_SEQUENCE_EVENT_B, key_B,
                              payload_func('moody horse'));
        recorder.record_progress(MOCK_SEQUENCE_EVENT_B, key_B,
                                 payload_func('moody horse'));
        recorder.record_progress(MOCK_SEQUENCE_EVENT_B, key_B,
                                 payload_func('moody horse'));
        recorder.record_stop(MOCK_SEQUENCE_EVENT_B, key_B,
                             payload_func('moody horse'));
    }
}

/*
 * next_letter:
 *   Will return the "next" character as far as Unicode is concerned.
 *   Should never be passed the final Unicode character.
 */
function next_letter (character) {
    return String.fromCharCode(character.charCodeAt(0) + 1);
}

/*
 * make_next_string:
 *   Takes a lowercase alphabetic (or empty) string and produces the next
 *   alphabetically ordered string, increasing the length by one if necessary.
 *   Every given string has a unique output and thus its invocations may be
 *   chained together to generate a non-repeating series of strings.
 *
 *   Example:
 *     let prev_string = '';
 *     while (true) {
 *         prev_string = make_next_string(prev_string);
 *         print(prev_string);
 *     }
 */
function make_next_string (prev_string) {
    if (!prev_string.match(/^[a-z]*$/)) {
        throw 'Error: make_next_string given non-lower-case-alphabetic input.';
    }
    let elements = prev_string.split('');
    for (let i = elements.length - 1; i >= 0; i--) {
        if (elements[i] !== 'z') {
            elements[i] = next_letter(elements[i]);
            return elements.join('');
        }
        else {
            elements[i] = 'a';
        }
    }

    elements.push('a');
    return elements.join('');
}

/*
 * test_make_next_string:
 *   Meta-smoke-test for make_next_string().
 */
function test_make_next_string () {
    let start = '';
    for (let i = 0; i < 3000; i++) {
        start = make_next_string(start);
        print(start);
    }
}

function read_environment () {
    let configuration_key_file = new GLib.KeyFile();
    let loaded_key_file =
        configuration_key_file.load_from_file(CONFIGURATION_FILE,
                                              GLib.KeyFileFlags.NONE);
    if (!loaded_key_file)
        {
            printerr('Could not load ' + CONFIGURATION_FILE + '.');
            return null;
        }

    return configuration_key_file.get_value('global', 'environment');
}

function environment_is_permitted () {
    let environment = read_environment();
    switch (environment) {
            case 'dev':
                // Fall through.
            case 'test':
                return true;
            case 'production':
                printerr('Metrics environment must be dev or test in order ' +
                         'to run smoke tests. Otherwise the production ' +
                         'database would be polluted with metric events ' +
                         'generated by smoke tests. Please run sudo ' +
                         'eos-select-metrics-env dev or sudo ' +
                         'eos-select-metrics-env test and reboot before ' +
                         'running any smoke tests.');
                return false;
            default:
                printerr('Metrics environment could not be determined. ' +
                         'Metrics environment must be dev or test in order ' +
                         'to run smoke tests. Otherwise the production ' +
                         'database would be polluted with metric events ' +
                         'generated by smoke tests. Please ensure that ' +
                         'environment is set to dev or test in ' +
                         CONFIGURATION_FILE + '. Changes to said file are ' +
                         'only picked after a reboot.');
                return false;
        }
}

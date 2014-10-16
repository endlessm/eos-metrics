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

imports.searchPath.unshift('.');

const GLib = imports.gi.GLib;
const SmokeLibrary = imports.smokeLibrary;

function stress_test_program_is_open (total_test_time) {
    if (!SmokeLibrary.environment_is_permitted())
        return;

    if (total_test_time <= 0)
        throw 'Non-positive test time given; cannot run program is open stress test.';
    for (let i = 0; i < total_test_time; i++) {
        GLib.spawn_async(null, ['gjs', 'smokeProgram.js'], null,
                         GLib.SpawnFlags.SEARCH_PATH, null);
        GLib.usleep(1000000); // 1,000,000 microseconds = 1 sec.
    }
}

// Parameter: How long to run test in seconds.
//   A value of 43,200 seconds is 12 hours which might be useful for long-running
//   tests for memory leaks.
stress_test_program_is_open(8);

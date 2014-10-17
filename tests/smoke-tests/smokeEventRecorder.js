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
const EosMetrics = imports.gi.EosMetrics;
const SmokeLibrary = imports.smokeLibrary;

function make_payload (flavor) {
    switch (flavor) {
        case 'cows':
            return new GLib.Variant('a{sv}', {
                    cows: new GLib.Variant('u', SmokeLibrary.randint(10)),
                    pounds_of_grass: new GLib.Variant('d', SmokeLibrary.randfloat(7.5)),
                    hungry: new GLib.Variant('b', SmokeLibrary.randbool())
                });
        case 'sand':
            return new GLib.Variant('a{sv}', {
                    grains_of_sand: new GLib.Variant('t', SmokeLibrary.randint(1543)),
                    secret_message: new GLib.Variant('s', 'The Sand-Cake is a Lie!')
                });
        case 'moody horse':
            let possible_moods = ['Happy', 'Nonplussed', 'Moody', 'Angry'];
            return new GLib.Variant('s', possible_moods[SmokeLibrary.randint(4)]);
        default:
            return null;
    }
}

SmokeLibrary.record_many_events(EosMetrics.EventRecorder.get_default(), 20,
                                make_payload);

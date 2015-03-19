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

imports.searchPath.unshift('.');

const GLib = imports.gi.GLib;
const EosMetrics = imports.gi.EosMetrics;
const SmokeLibrary = imports.smokeLibrary;

// Change this value to adjust size of GVariant payload.
const SIZE = 300;

function make_payload () {
    let make_big64 = function () {
        return new GLib.Variant('x', SmokeLibrary.randint(80000));
    };
    let str = '';
    let variant_data = {};
    for (let i = 0; i < SIZE; i++) {
        str = SmokeLibrary.make_next_string(str);
        variant_data[str] = make_big64();
    }
    return new GLib.Variant('a{sv}', variant_data);
}

SmokeLibrary.record_many_events(EosMetrics.EventRecorder.get_default(), 20,
                                make_payload);

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

const Lang = imports.lang;
const GLib = imports.gi.GLib;
const Gio = imports.gi.Gio;
const SmokeLibrary = imports.smokeLibrary;

const TEST_APPLICATION_ID = 'com.endlessm.example.smoketest';

const TestApplication = new Lang.Class ({
    Name: 'TestApplication',
    Extends: Gio.Application,

    vfunc_startup: function() {
        this.parent();

        GLib.timeout_add(GLib.PRIORITY_HIGH, 500, function () {
            this.quit();
        }.bind(this));
    },
    vfunc_activate: function() {}
});

function run_application () {
    if (!SmokeLibrary.environment_is_permitted())
        return;

    let app = new TestApplication({ application_id: TEST_APPLICATION_ID,
                                    flags: 0 });
    app.run(ARGV);
}

run_application();

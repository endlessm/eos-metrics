'''
The MIT License (MIT)

Copyright (c) 2014 Endless Mobile, Inc.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
'''

import sys
from gi.repository import GLib, Gio


DBUS_NAME = 'com.endlessm.Metrics'


def on_name_appeared(connection, name, owner):
    print DBUS_NAME, 'appeared'
    Gio.bus_unwatch_name(watcher_id)
    loop.quit()


def on_timeout():
    print 'Timed out'
    Gio.bus_unwatch_name(watcher_id)
    sys.exit(1)

print 'Watching for name', DBUS_NAME
watcher_id = Gio.bus_watch_name(Gio.BusType.SYSTEM, DBUS_NAME,
    Gio.BusNameWatcherFlags.NONE, on_name_appeared, None)
GLib.timeout_add_seconds(5, on_timeout)

loop = GLib.MainLoop()
loop.run()
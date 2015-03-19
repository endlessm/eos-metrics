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

#!/bin/bash -e

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

function finally() {
    kill $DBUS_SESSION_BUS_PID
}

# This brings up a new session bus and pretends that it is the system bus.
# dbus-launch initializes DBUS_SESSION_BUS_PID.
eval `dbus-launch`
export DBUS_SYSTEM_BUS_ADDRESS=$DBUS_SESSION_BUS_ADDRESS
export DBUS_SESSION_BUS_PID

# Take down the mock DBus, no matter whether we exit successfully or
# fail the tests; think of a bash trap as a "finally" clause.
trap finally EXIT

# Start the mock service and add its methods.
python3 -m dbusmock --system \
    com.endlessm.Metrics \
    /com/endlessm/Metrics \
    com.endlessm.Metrics.EventRecorderServer &

# Wait for the service to come up
python3 `dirname $0`/wait-for-service-helper.py

gdbus call --system \
    -d com.endlessm.Metrics \
    -o /com/endlessm/Metrics \
    -m org.freedesktop.DBus.Mock.AddMethods \
    com.endlessm.Metrics.EventRecorderServer \
    '[("RecordSingularEvent", "uayxbv", "", ""),
    ("RecordAggregateEvent", "uayxxbv", "", ""),
    ("RecordEventSequence", "uaya(xbv)", "", "")]'

gtester "$@"

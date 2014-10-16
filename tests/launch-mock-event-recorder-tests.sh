#!/bin/bash -e

# The MIT License (MIT)
#
# Copyright (c) 2014 Endless Mobile, Inc.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

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
python -m dbusmock --system \
    com.endlessm.Metrics \
    /com/endlessm/Metrics \
    com.endlessm.Metrics.EventRecorderServer &

# Wait for the service to come up
python `dirname $0`/wait-for-service-helper.py

gdbus call --system \
    -d com.endlessm.Metrics \
    -o /com/endlessm/Metrics \
    -m org.freedesktop.DBus.Mock.AddMethods \
    com.endlessm.Metrics.EventRecorderServer \
    '[("RecordSingularEvent", "uayxbv", "", ""),
    ("RecordAggregateEvent", "uayxxbv", "", ""),
    ("RecordEventSequence", "uaya(xbv)", "", "")]'

gtester "$@"

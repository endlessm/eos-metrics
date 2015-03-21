# Copyright 2013, 2014, 2015 Endless Mobile, Inc.

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

# Bootstrap script for EosMetrics
# Run this script on a clean source checkout to get ready for building.

FILE_MUST_EXIST=configure.ac

test -n "$srcdir" || srcdir=`dirname "$0"`
test -n "$srcdir" || srcdir=.
olddir=`pwd`

cd $srcdir
test -f $FILE_MUST_EXIST || {
    echo "You must run this script in the top-level checkout directory"
    exit 1
}

# NOCONFIGURE is used by gnome-common
if test -z "$NOCONFIGURE"; then
    echo "This script will run ./configure automatically. If you wish to pass "
    echo "any arguments to it, please specify them on the $0 "
    echo "command line. To disable this behavior, have NOCONFIGURE=1 in your "
    echo "environment."
fi

# Run the actual tools to prepare the clean checkout
gtkdocize || exit $?
autoreconf -fi || exit $?

cd "$olddir"
test -n "$NOCONFIGURE" || "$srcdir/configure" "$@"

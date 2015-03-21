## Copyright 2014, 2015 Endless Mobile, Inc.

## This file is part of eos-metrics.
##
## eos-metrics is free software: you can redistribute it and/or modify
## it under the terms of the GNU Lesser General Public License as published
## by the Free Software Foundation, either version 2.1 of the License, or
## (at your option) any later version.
##
## eos-metrics is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU Lesser General Public License for more details.
##
## You should have received a copy of the GNU Lesser General Public
## License along with eos-metrics.  If not, see
## <http://www.gnu.org/licenses/>.

daemon_dbus_name = emer-event-recorder-server
daemon_dbus_sources = $(daemon_dbus_name).c $(daemon_dbus_name).h
BUILT_SOURCES += $(daemon_dbus_sources)
$(daemon_dbus_sources): data/com.endlessm.Metrics.xml
	$(AM_V_GEN)$(GDBUS_CODEGEN) --generate-c-code $(daemon_dbus_name) \
		--c-namespace Emer \
		--interface-prefix com.endlessm.Metrics. \
		$<

dist_pkgdata_DATA = data/com.endlessm.Metrics.xml

CLEANFILES += \
	$(daemon_dbus_sources) \
	$(NULL)

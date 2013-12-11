# Copyright 2014 Endless Mobile, Inc.

daemon_dbus_name = emer-event-recorder-server
daemon_dbus_sources = $(daemon_dbus_name).c $(daemon_dbus_name).h
BUILT_SOURCES += $(daemon_dbus_sources)
$(daemon_dbus_sources): data/com.endlessm.Metrics.xml
	$(AM_V_GEN)$(GDBUS_CODEGEN) --generate-c-code $(daemon_dbus_name) \
		--c-namespace Emer \
		--interface-prefix com.endlessm.Metrics. \
		$<

EXTRA_DIST += \
	data/com.endlessm.Metrics.xml \
	$(NULL)

CLEANFILES += \
	$(daemon_dbus_sources) \
	$(NULL)

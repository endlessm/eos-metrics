## Copyright 2013, 2014, 2015 Endless Mobile, Inc.

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

eosmetrics_public_installed_headers = eosmetrics/eosmetrics.h

eosmetrics_private_installed_headers = \
	eosmetrics/emtr-apiversion.h \
	eosmetrics/emtr-aggregate-timer.h \
	eosmetrics/emtr-enums.h \
	eosmetrics/emtr-event-recorder.h \
	eosmetrics/emtr-event-types.h \
	eosmetrics/emtr-macros.h \
	eosmetrics/emtr-types.h \
	eosmetrics/emtr-util.h \
	eosmetrics/emtr-version.h \
	emer-event-recorder-server.h \
	$(NULL)

eosmetrics_library_sources = \
	eosmetrics/emtr-aggregate-timer-private.h \
	eosmetrics/emtr-aggregate-timer.c \
	eosmetrics/emtr-event-recorder.c \
	eosmetrics/emtr-util.c \
	emer-event-recorder-server.c \
	$(NULL)

lib_LTLIBRARIES = libeosmetrics-@EMTR_API_VERSION@.la
libeosmetrics_@EMTR_API_VERSION@_la_SOURCES = \
	$(eosmetrics_public_installed_headers) \
	$(eosmetrics_private_installed_headers) \
	$(eosmetrics_library_sources) \
	$(NULL)
# Defining G_LOG_DOMAIN allows you to run any program with
# G_MESSAGES_DEBUG=EosMetrics ./myprogram
# and turn on debug messages
libeosmetrics_@EMTR_API_VERSION@_la_CPPFLAGS = \
	@EOSMETRICS_CFLAGS@ \
	@EOS_C_COVERAGE_CFLAGS@ \
	-I$(top_builddir)/eosmetrics \
	-DG_LOG_DOMAIN=\"EosMetrics\" \
	-DCOMPILING_EOS_METRICS \
	-D_POSIX_C_SOURCE=200112L \
	$(NULL)
libeosmetrics_@EMTR_API_VERSION@_la_CFLAGS = $(AM_CFLAGS)
libeosmetrics_@EMTR_API_VERSION@_la_LIBADD = @EOSMETRICS_LIBS@
libeosmetrics_@EMTR_API_VERSION@_la_LDFLAGS = \
	-version-info @EMTR_LT_VERSION_INFO@ \
	-export-symbols-regex "^emtr_" \
	@EOS_C_COVERAGE_LDFLAGS@ \
	$(NULL)

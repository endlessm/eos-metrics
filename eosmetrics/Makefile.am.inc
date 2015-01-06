# The MIT License (MIT)
#
# Copyright (c) 2013-2014 Endless Mobile, Inc.
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

eosmetrics_public_installed_headers = eosmetrics/eosmetrics.h

eosmetrics_private_installed_headers = \
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

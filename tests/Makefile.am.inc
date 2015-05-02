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

noinst_PROGRAMS = \
	tests/test-event-types \
	tests/test-library.dbuseventrecorder \
	$(NULL)

LIBRARY_TEST_FLAGS = \
	@EOSMETRICS_CFLAGS@ \
	@EOS_C_COVERAGE_CFLAGS@ \
	-I$(top_srcdir) \
	-DCOMPILING_EOS_METRICS \
	-D_POSIX_C_SOURCE=200112L \
	$(NULL)
LIBRARY_TEST_LIBS = \
	@EOSMETRICS_LIBS@ \
	$(top_builddir)/libeosmetrics-@EMTR_API_VERSION@.la \
	$(NULL)

tests_test_library_dbuseventrecorder_SOURCES = tests/test-event-recorder.c
tests_test_library_dbuseventrecorder_CPPFLAGS = $(LIBRARY_TEST_FLAGS)
tests_test_library_dbuseventrecorder_LDADD = $(LIBRARY_TEST_LIBS)

EOSMETRICS_TEST_FLAGS = \
	@EOSMETRICS_CFLAGS@ \
	@EOS_C_COVERAGE_CFLAGS@ \
	-I$(top_srcdir) \
	$(NULL)
EOSMETRICS_TEST_LIBS = @EOSMETRICS_LIBS@

tests_test_event_types_SOURCES = \
	eosmetrics/emtr-event-types.c eosmetrics/emtr-event-types.h \
	eosmetrics/eosmetrics.h \
	tests/test-event-types.c \
	$(NULL)
tests_test_event_types_CPPFLAGS = $(EOSMETRICS_TEST_FLAGS)
tests_test_event_types_LDADD = $(EOSMETRICS_TEST_LIBS)

dist_noinst_SCRIPTS = \
	tests/launch-mock-event-recorder-tests.sh \
	$(NULL)

# Run tests when running 'make check'
TESTS = \
	tests/test-library.dbuseventrecorder \
	tests/test-daemon-integration.py \
	tests/test-event-types \
	$(NULL)

TEST_EXTENSIONS = .dbuseventrecorder .py
DBUSEVENTRECORDER_LOG_COMPILER = $(top_srcdir)/tests/launch-mock-event-recorder-tests.sh
AM_DBUSEVENTRECORDER_LOG_FLAGS = -k --verbose
LOG_COMPILER = gtester
AM_LOG_FLAGS = -k --verbose
PY_LOG_COMPILER = python3

# Use locally built versions of EosMetrics-0.gir and libraries; this may need to
# be changed to AM_TESTS_ENVIRONMENT in a future version of Automake.
TESTS_ENVIRONMENT = \
	export GI_TYPELIB_PATH="$(top_builddir)$${GI_TYPELIB_PATH:+:$$GI_TYPELIB_PATH}"; \
	export LD_LIBRARY_PATH="$(top_builddir)/.libs$${LD_LIBRARY_PATH:+:$$LD_LIBRARY_PATH}"; \
	$(NULL)

EXTRA_DIST += \
	tests/test-daemon-integration.py \
	tests/smoke-tests/event-types.js \
	tests/smoke-tests/smokeEventRecorderHeavyPayload.js \
	tests/smoke-tests/smokeEventRecorder.js \
	tests/smoke-tests/smokeLibrary.js \
	tests/wait-for-service-helper.py \
	$(NULL)

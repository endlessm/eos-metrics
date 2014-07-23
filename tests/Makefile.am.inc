# Copyright 2013 Endless Mobile, Inc.

noinst_PROGRAMS = \
	tests/test-library.dbus \
	$(NULL)

LIBRARY_TEST_FLAGS = \
	@EOSMETRICS_CFLAGS@ \
	-I$(top_srcdir) \
	-DCOMPILING_EOS_METRICS \
	-D_POSIX_C_SOURCE=200112L \
	$(NULL)
LIBRARY_TEST_LIBS = \
	@EOSMETRICS_LIBS@ \
	$(top_builddir)/libeosmetrics-@EMTR_API_VERSION@.la \
	$(NULL)

tests_test_library_dbus_SOURCES = tests/test-event-recorder.c
tests_test_library_dbus_CPPFLAGS = $(LIBRARY_TEST_FLAGS)
tests_test_library_dbus_LDADD = $(LIBRARY_TEST_LIBS)

dist_noinst_SCRIPTS = \
	tests/launch-mock-dbus-tests.sh \
	$(NULL)

# Run tests when running 'make check'
TESTS = \
	tests/test-library.dbus \
	tests/test-daemon-integration.py \
	$(NULL)
TEST_EXTENSIONS = .dbus .py
DBUS_LOG_COMPILER = $(top_srcdir)/tests/launch-mock-dbus-tests.sh
AM_DBUS_LOG_FLAGS = -k --verbose
LOG_COMPILER = gtester
AM_LOG_FLAGS = -k --verbose
PY_LOG_COMPILER = python

# Use locally built versions of EosMetrics-0.gir and libraries; this may need to
# be changed to AM_TESTS_ENVIRONMENT in a future version of Automake
TESTS_ENVIRONMENT = \
	export GI_TYPELIB_PATH="$(top_builddir)$${GI_TYPELIB_PATH:+:$$GI_TYPELIB_PATH}"; \
	export LD_LIBRARY_PATH="$(top_builddir)/.libs$${LD_LIBRARY_PATH:+:$$LD_LIBRARY_PATH}"; \
	$(NULL)

EXTRA_DIST += \
	tests/test-daemon-integration.py \
	tests/smoke-tests/event-types.js \
	tests/wait-for-service-helper.py \
	$(NULL)

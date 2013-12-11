# Copyright 2013 Endless Mobile, Inc.

noinst_PROGRAMS = \
	tests/run-tests \
	$(NULL)

TEST_FLAGS = @EOSMETRICS_CFLAGS@ -I$(top_srcdir) -DCOMPILING_EOS_METRICS
TEST_LIBS = @EOSMETRICS_LIBS@ $(top_builddir)/libeosmetrics-@EMTR_API_VERSION@.la

tests_run_tests_SOURCES = \
	tests/run-tests.c tests/run-tests.h \
	tests/test-event-recorder.c \
	$(NULL)
tests_run_tests_CPPFLAGS = $(TEST_FLAGS)
tests_run_tests_LDADD = $(TEST_LIBS)

# Run tests when running 'make check'
TESTS = \
	tests/test-daemon-integration.py \
	$(NULL)
TEST_EXTENSIONS = .py
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
	$(NULL)

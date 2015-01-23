## Process this file with automake to produce Makefile.in

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

## -----------
## Makefile.am
## -----------
## Please keep this file well-commented.

# Subdirectories where we also want to run make
# (Make sure that the library is built before the docs)
SUBDIRS = . docs/reference/eosmetrics

# Our Autoconf macros are kept in the m4/ directory
ACLOCAL_AMFLAGS = -I m4

# Extra files to distribute in the tarball
EXTRA_DIST = README.md @EMTR_API_NAME@.pc.in

# Compiler flags
AM_CFLAGS = @STRICT_CFLAGS@

# Make sure to run Gtk-doc tests and build the documentation when doing
# 'make distcheck'
AM_DISTCHECK_CONFIGURE_FLAGS = --enable-gtk-doc

# Generated files that 'make clean' should erase
CLEANFILES =
DISTCLEANFILES =

# Generated sources that should be built before other sources
BUILT_SOURCES =

# Make sure that 'make dist' includes documentation
if CAN_MAKE_DIST
dist-hook:
else
dist-hook:
	@echo "***"
	@echo "*** You must configure with --enable-gtk-doc to run make dist or"
	@echo "*** make distcheck."
	@echo "***"
	@false
endif

# # # DATA # # #

include $(top_srcdir)/data/Makefile.am.inc

# # # COVERAGE # # #

EOS_COVERAGE_BLACKLIST_PATTERNS =	\
	*/tests/*			\
	*emer-event-recorder-server.c	\
	$(NULL)

# These need to be defined before eosmetrics/Makefile.am.inc is included
# so that we can use EOS_C_COVERAGE_CFLAGS and EOS_C_COVERAGE_LDFLAGS.
@EOS_COVERAGE_RULES@

AM_LDFLAGS = @EOS_C_COVERAGE_LDFLAGS@

# # # LIBRARY # # #

# Metrics library
include $(top_srcdir)/eosmetrics/Makefile.am.inc

# Public header files
eosmetricsincludedir = $(includedir)/@EMTR_API_NAME@
nobase_eosmetricsinclude_HEADERS = \
	$(eosmetrics_public_installed_headers) \
	$(eosmetrics_private_installed_headers) \
	$(NULL)

# Pkg-config file
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = @EMTR_API_NAME@.pc
DISTCLEANFILES += @EMTR_API_NAME@.pc

# # # INTROSPECTION FILES # # #

-include $(INTROSPECTION_MAKEFILE)
INTROSPECTION_GIRS =
INTROSPECTION_SCANNER_ARGS = --add-include-path=$(srcdir) --warn-all
INTROSPECTION_COMPILER_ARGS = --includedir=$(srcdir)

if HAVE_INTROSPECTION
introspection_sources = \
	$(filter-out %.h, $(eosmetrics_library_sources)) \
	$(eosmetrics_public_installed_headers) \
	$(filter-out emer-event-recorder-server.h, $(eosmetrics_private_installed_headers)) \
	$(NULL)

EosMetrics-@EMTR_API_VERSION@.gir: libeosmetrics-@EMTR_API_VERSION@.la
EosMetrics_@EMTR_API_VERSION@_gir_INCLUDES = \
	GObject-2.0 \
	GLib-2.0 \
	Gio-2.0 \
	$(NULL)
EosMetrics_@EMTR_API_VERSION@_gir_SCANNERFLAGS = \
	--identifier-prefix=Emtr \
	--symbol-prefix=emtr \
	-DCOMPILING_EOS_METRICS \
	$(NULL)
EosMetrics_@EMTR_API_VERSION@_gir_LIBS = libeosmetrics-@EMTR_API_VERSION@.la
EosMetrics_@EMTR_API_VERSION@_gir_FILES = $(introspection_sources)
EosMetrics_@EMTR_API_VERSION@_gir_EXPORT_PACKAGES = @EMTR_API_NAME@
INTROSPECTION_GIRS += EosMetrics-@EMTR_API_VERSION@.gir

girdir = $(datadir)/gir-1.0
gir_DATA = $(INTROSPECTION_GIRS)

typelibdir = $(libdir)/girepository-1.0
typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES += $(gir_DATA) $(typelib_DATA)
endif

# # # GOBJECT INTROSPECTION DOCUMENTATION # # #

if ENABLE_GIR_DOC

docs/reference/eosmetrics-js/index.page: $(INTROSPECTION_GIRS)
	$(MKDIR_P) $(@D)
	g-ir-doc-tool -o $(@D) -l gjs $<

docs/reference/eosmetrics-js/html/index.html: docs/reference/eosmetrics-js/index.page
	$(MKDIR_P) $(@D)
	yelp-build html -o $(@D) $(<D)

all-local: docs/reference/eosmetrics-js/html/index.html

EXTRA_DIST += \
	docs/reference/eosmetrics-js/html/*.html \
	docs/reference/eosmetrics-js/html/*.css \
	docs/reference/eosmetrics-js/html/*.js \
	$(NULL)

CLEANFILES += \
	docs/reference/eosmetrics-js/*.page \
	docs/reference/eosmetrics-js/html/*.html \
	docs/reference/eosmetrics-js/html/*.css \
	docs/reference/eosmetrics-js/html/*.js \
	$(NULL)
endif

# # # TESTS # # #

include $(top_srcdir)/tests/Makefile.am.inc

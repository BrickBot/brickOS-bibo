### ==========================================================================
###  $Id: Makefile 886 2009-04-21 13:00:15Z hoenicke $
###  FILE: Makefile - make all parts of the bibo distribution
###  bibo - another LEGO MindStorms RCX OS
### --------------------------------------------------------------------------
###   (This is the top-level Makefile.  All actions are performed from here)

include Makefile.config

#  org and package/distribution names (all lowercase by convention)
ORG = brickbot
PACKAGE ?= brickos
KERNEL ?= bibo

#  version of this release
VERSION = 0.10.0

# Default values to use when building prior to installation
LIBDIR?=lib
UTILDIR?=util
INCLUDEDIR?=include
KERNELDIR?=kernel
KERNELCONFIGDIR?=kconfig/$(KERNEL)
KERNELCONFIG_HEADER = $(KERNELCONFIGDIR)/kernel-config.h


#
#  Define our default install locations (overridden by packaging systems)
#
DESTDIR ?= 
prefix ?= /opt/stow/${PACKAGE}
exec_prefix ?= ${prefix}

bindir ?= ${exec_prefix}/bin
sbindir ?= ${exec_prefix}/sbin
libexecdir ?= ${exec_prefix}/lib
datarootdir ?= ${prefix}/share
docdir ?= ${prefix}/share/doc
targetdir ?= ${prefix}/${ARCH}
sysconfdir ?= ${prefix}/etc
sharedstatedir ?= ${prefix}/com
localstatedir ?= ${prefix}/var
mandir ?= ${datarootdir}/man
includedir ?= ${prefix}/include

pkgdatadir ?= ${datarootdir}/${PACKAGE}
pkgdocdir ?= ${docdir}/packages/${PACKAGE}
pkghtmldir ?= ${pkgdocdir}/html
pkgexampledir ?= ${pkgdocdir}/examples
pkgincludedir ?= ${targetdir}/include
pkglibdir ?= ${targetdir}/lib

pkgkconfigdir ?= ${targetdir}/kconfig
pkgkconfigkerneldir ?= ${pkgkconfigdir}/$(KERNEL)


# ------------------------------------------------------------
#  No user customization below here...
# ------------------------------------------------------------

#
#  makefile to build the brickOS operating system and demo files
# 
SUBDIRS=util lib include kconfig kernel xs

# Only build the demo programs if program support is enabled
ifeq ($(shell grep --quiet '^\#define CONF_PROGRAM' include/config.h && echo 1),1)
  SUBDIRS += demo
else
  $(info NOTE: CONF_PROGRAM support for programs disabled; skipping building of demo programs)
endif

# Add docs to the subdirs list last
SUBDIRS += doc

# The "all" targets
MAKE_ALL_TARGETS=host kernel demo
# Excluded targets: docs (currently broken?)

all:: $(MAKE_ALL_TARGETS)


Makefile.config:
	./configure

include Makefile.common
include $(SUBDIRS:%=%/Makefile.sub)
include Makefile.doxygen
include Makefile.utility


clean:: $(MAKE_ALL_TARGETS:%=%-clean)
	rm -f Makefile.common.dist demo/c/Makefile.dist demo/c++/Makefile.dist config.h.dist

realclean:: clean $(MAKE_ALL_TARGETS:%=%-realclean)
	rm -f Makefile.config tags TAGS *.bak *~ *.bak

install:: $(MAKE_ALL_TARGETS:%=%-install)

uninstall:: $(MAKE_ALL_TARGETS:%=%-uninstall)


#
# Setup the relationships between "host" targets and "makefiles" targets
#
host:: makefiles

host-install:: makefiles-install

host-uninstall:: makefiles-uninstall

host-clean:: makefiles-clean

host-realclean:: makefiles-realclean

#
# install/unnistall makefiles
#
makefiles::

makefiles-install::
	test -d $(DESTDIR)$(pkgdatadir) || mkdir -p $(DESTDIR)$(pkgdatadir)
	install -m 644 Makefile.common $(DESTDIR)$(pkgdatadir)
	sed -e '/Installation Variables/a \
	ORG = $(ORG)\
	PACKAGE = $(PACKAGE)\
	CROSSTOOLPREFIX = $(CROSSTOOLPREFIX)\
	CROSSTOOLSUFFIX = $(CROSSTOOLSUFFIX)\
	CROSSTOOLEXT    = $(CROSSTOOLEXT)'\
		< Makefile.lxprog  > $(DESTDIR)$(pkgdatadir)/Makefile
	chmod 644 $(DESTDIR)$(pkgdatadir)/Makefile
	test -d $(DESTDIR)$(bindir) || mkdir -p $(DESTDIR)$(bindir)
	sed -e '/Installation Variables/a \
	ORG=$(ORG)\
	PACKAGE=$(PACKAGE)\
	CROSSTOOLPREFIX = $(CROSSTOOLPREFIX)\
	CROSSTOOLSUFFIX = $(CROSSTOOLSUFFIX)\
	CROSSTOOLEXT    = $(CROSSTOOLEXT)'\
		< makelx.sh  > $(DESTDIR)$(bindir)/makelx
	chmod 755 $(DESTDIR)$(bindir)/makelx

makefiles-uninstall::
	rm -f $(DESTDIR)$(pkgdatadir)/Makefile
	rm -f $(DESTDIR)$(pkgdatadir)/Makefile.common
	rm -f $(DESTDIR)$(bindir)/makelx

makefiles-clean::

makefiles-realclean:: makefiles-clean

#  NOTE: --format=1 is not supported on Linux ([ce]tags in emacs2[01] packages)
#   please set in your own environment
tag::
	-ctags kernel/*.c include/*.h include/*/*.h
	-etags kernel/*.c include/*.h include/*/*.h


.PHONY: all depend tag clean realclean html tag c++ html html-c html-c++ html-kern html-dist 


# ------------------------------------------------------------
#  Components of this release to be packaged
# ------------------------------------------------------------
#

DISTFILES += h8300-rcx.ld configure \
        makelx.sh README CONTRIBUTORS LICENSE \
    Makefile Makefile.common Makefile.lxprog

#  locations for this package build effort
DISTDIR = $(PACKAGE)-$(VERSION)

#  tools we use to make distribution image
TAR = tar
GZIP_ENV = --best

DISTDISTFILES = $(DISTFILES:%=$(DISTDIR)/%)

#  make our distribution tarball
dist: distclean $(DISTDISTFILES)
	GZIP=$(GZIP_ENV) $(TAR) chozf $(DISTDIR).tar.gz $(DISTDIR)
	@echo "---------------------------------------------------------"
	@echo "     MD5 sum for $(PACKAGE) v$(VERSION)"
	@md5sum $(DISTDIR).tar.gz
	@echo "---------------------------------------------------------"

distclean:
	rm -rf $(DISTDIR)

#  build a copy of the source tree which can be zipped up and then deleted
$(DISTDISTFILES): $(DISTDIR)/%: %
	@echo $@
	@mkdir -p $(dir $@)
	@cp -p $< $@


###
### generic rules
###

# cancel standard C source rule
%.o: %.c

# how to compile native C source
$(NATIVEOBJS): %.o: %.c
	$(CC) -MMD $(CFLAGS) -c $< -o $@

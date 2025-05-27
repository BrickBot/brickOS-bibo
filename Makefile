### ==========================================================================
###  $Id: Makefile 886 2009-04-21 13:00:15Z hoenicke $
###  FILE: Makefile - make all parts of the bibo distribution
###  bibo - another LEGO MindStorms RCX OS
### --------------------------------------------------------------------------
###   (This is the top-level Makefile.  All actions are performed from here)

# Project-related names (all lowercase by convention)
ORG = brickbot
PACKAGE = brickos
TARGET_ARCH = h8300-lego-coff
KERNEL ?= bibo

# Version of this release
VERSION_MAJOR = 1
VERSION_MINOR = 0
VERSION_PATCH = 0
VERSION = $(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_PATCH)

VERSION_SERIES = v$(VERSION_MAJOR)


# ------------------------------------------------------------
#  No user customization below here...
# ------------------------------------------------------------

#
#  makefile to build the brickOS operating system and demo files
# 
SUBDIRS=util lib include kconfig kernel rom xs

# Only build the demo programs if program support is enabled
ifeq ($(shell grep --quiet '^\#define CONF_PROGRAM' include/config.h && echo 1),1)
  SUBDIRS += demo
else
  $(info NOTE: CONF_PROGRAM support for programs disabled; skipping building of demo programs)
endif

# Add docs to the subdirs list last
SUBDIRS += doc

# The "all" targets
MAKE_ALL_TARGETS=host headers kernel rom demo
# Excluded targets: docs (currently broken?)

all:: $(MAKE_ALL_TARGETS)


include Makefile.common
include $(SUBDIRS:%=%/Makefile.sub)
include Makefile.doxygen
include Makefile.utility


clean:: $(MAKE_ALL_TARGETS:%=%-clean)

realclean:: clean $(MAKE_ALL_TARGETS:%=%-realclean)
	rm -f tags TAGS *.bak *~ *.bak

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
	test -d $(DESTDIR)$(pkgtargetsysconfdir) || mkdir -p $(DESTDIR)$(pkgtargetsysconfdir)
	install -m 644 Makefile.common $(DESTDIR)$(pkgtargetsysconfdir)
	sed -e '/Installation Variables/a \
	ORG = $(ORG)\
	PACKAGE = $(PACKAGE)\
	TARGET_ARCH = $(TARGET_ARCH)\
	CROSSTOOLPREFIX = $(CROSSTOOLPREFIX)\
	CROSSTOOLSUFFIX = $(CROSSTOOLSUFFIX)\
	CROSSTOOLEXT    = $(CROSSTOOLEXT)'\
		< Makefile.lxprog  > $(DESTDIR)$(pkgtargetsysconfdir)/Makefile
	chmod 644 $(DESTDIR)$(pkgtargetsysconfdir)/Makefile
	test -d $(DESTDIR)$(bindir) || mkdir -p $(DESTDIR)$(bindir)
	sed -e '/Installation Variables/a \
	ORG=$(ORG)\
	PACKAGE=$(PACKAGE)\
	SYSCONF_SUBDIR=$(ARCH_SYSCONF_PACKAGESUBDIR)\
	TARGET_ARCH=$(TARGET_ARCH)'\
		< makelx.sh  > $(DESTDIR)$(bindir)/makelx
	chmod 755 $(DESTDIR)$(bindir)/makelx

makefiles-uninstall::
	rm -f $(DESTDIR)$(pkgtargetsysconfdir)/Makefile
	rm -f $(DESTDIR)$(pkgtargetsysconfdir)/Makefile.common
	rm -f $(DESTDIR)$(pkgtargetsysconfdir)/$(HOST_PROPERTIES_FILE)
	rm -f $(DESTDIR)$(bindir)/makelx

makefiles-clean::

makefiles-realclean:: makefiles-clean
	rm -f $(HOST_PROPERTIES_PATH)

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

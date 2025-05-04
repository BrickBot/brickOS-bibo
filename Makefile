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
KERNELDIR=kernel

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


# ------------------------------------------------------------
#  No user customization below here...
# ------------------------------------------------------------

#
#  makefile to build the brickOS operating system and demo files
# 
SUBDIRS=util lib include kernel xs

# Only build the demo programs if program support is enabled
ifeq ($(shell grep --quiet '^\#define CONF_PROGRAM' include/config.h && echo 1),1)
  SUBDIRS += demo
else
  $(info NOTE: CONF_PROGRAM support for programs disabled; skipping building of demo programs)
endif

# Add docs to the subdirs list last
SUBDIRS += doc


all::

Makefile.config:
	./configure

include Makefile.common
include $(SUBDIRS:%=%/Makefile.sub)

install::
	test -d $(DESTDIR)$(pkgdatadir) || mkdir -p $(DESTDIR)$(pkgdatadir)
	install -m 644 Makefile.common $(DESTDIR)$(pkgdatadir)
	sed -e '/Installation Variables/a \
	ORG = $(ORG)\
	PACKAGE = $(PACKAGE)\
	CROSSTOOLPREFIX = $(CROSSTOOLPREFIX)'\
		< Makefile.dist  > $(DESTDIR)$(pkgdatadir)/Makefile
	chmod 644 $(DESTDIR)$(pkgdatadir)/Makefile
	test -d $(DESTDIR)$(bindir) || mkdir -p $(DESTDIR)$(bindir)
	sed -e '/Installation Variables/a \
	ORG=$(ORG)\
	PACKAGE=$(PACKAGE)\
	CROSSTOOLPREFIX=$(CROSSTOOLPREFIX)'\
		< makelx.sh  > $(DESTDIR)$(bindir)/makelx
	chmod 755 $(DESTDIR)$(bindir)/makelx

realclean:: clean
	rm -f Makefile.config tags TAGS *.bak *~ *.bak

docs-build docs-install::
	$(MAKE) $(MFLAGS) -C doc $@


#  API generation support using Doxygen
#
#  when we get a new version of doxygen, run this target once
#
upgrade-doxygen:
	doxygen -u Doxyfile-c 
	doxygen -u Doxyfile-c++
	doxygen -u Doxyfile

#
#  doc/html-c subdirectory: make C docs
#

html-c: Doxyfile-c-report

realclean::
	rm -rf doc/html-c doc/rtf-c
	rm -f -- Doxyfile-c.log Doxyfile-c.rpt .Doxyfile-c-doneflag *.out

brickos-html-c-dist.tar.gz: html-c 
	cd doc;tar --gzip -cf $@ html-c;mv $@ ..;cd -

html-c-dist: brickos-html-c-dist.tar.gz

html-c-install: html-c
	cp -r doc/html-c ${pkghtmldir}

Doxyfile-c.log: Doxyfile-c
	doxygen $? >$@ 2>&1

Doxyfile-c.rpt: Doxyfile-c.log
	@grep Warn $? | sed -e 's/^.*brickos\///' | cut -f1 -d: | sort | \
	 uniq -c | sort -rn | tee $@

.Doxyfile-c-doneflag:  Doxyfile-c.rpt
	@for FIL in `cat $? | cut -c9-99`; do \
	  OUTFILE=`echo $$FIL | sed -e 's/[\/\.]/-/g'`.out; \
	  echo "# FILE: $$OUTFILE" >$$OUTFILE; \
	  grep "$$FIL" $? >>$$OUTFILE; \
	  grep "$$FIL" Doxyfile-c.log | grep Warn >>$$OUTFILE; \
	done
	@touch $@

Doxyfile-c-report: .Doxyfile-c-doneflag
	-ls -ltr *.out 2>/dev/null

#
#  doc/html-c++ subdirectory: make C++ docs
#
html-c++: Doxyfile-c++-report

realclean::
	rm -rf doc/html-c++ doc/rtf-c++
	rm -f -- Doxyfile-c++.log Doxyfile-c++.rpt .Doxyfile-c++-doneflag *.out

brickos-html-c++-dist.tar.gz: html-c++
	cd doc;tar --gzip -cf $@ html-c++;mv $@ ..;cd -

html-c++-dist: brickos-html-c++-dist.tar.gz

html-c++-install: html-c++
	cp -r doc/html-c++ ${pkghtmldir}

Doxyfile-c++.log: 
	doxygen  Doxyfile-c++ >$@ 2>&1

Doxyfile-c++.rpt: Doxyfile-c++.log
	@grep Warn $? | sed -e 's/^.*brickos\///' | cut -f1 -d: | sort | \
	uniq -c | sort -rn | tee $@

.Doxyfile-c++-doneflag:  Doxyfile-c++.rpt
	@for FIL in `cat Doxyfile-c++.rpt | cut -c9-99`; do \
       OUTFILE=`echo $$FIL | sed -e 's/[\/\.]/-/g'`.out; \
       echo "# FILE: $$OUTFILE" >$$OUTFILE; \
       grep "$$FIL" Doxyfile-c++.rpt >>$$OUTFILE; \
       grep "$$FIL" Doxyfile-c++.log | grep Warn >>$$OUTFILE; \
    done
	@touch $@

Doxyfile-c++-report: .Doxyfile-c++-doneflag
	-ls -ltr *.out 2>/dev/null

#
#  doc/html-kern subdirectory: make kernel developer docs
#
html-kern: Doxyfile-kern-report

realclean::
	rm -rf doc/html-kern doc/rtf-kern
	rm -f -- Doxyfile-kern.log Doxyfile-kern.rpt .Doxyfile-kern-doneflag *.out

brickos-html-kern-dist.tar.gz: html-kern
	cd doc;tar --gzip -cf $@ html-kern;mv $@ ..;cd -

html-kern-dist: brickos-html-kern-dist.tar.gz

html-kern-install: html-kern
	cp -r doc/html-kern ${pkghtmldir}

Doxyfile-kern.log: 
	doxygen  Doxyfile >$@ 2>&1

Doxyfile-kern.rpt: Doxyfile-kern.log
	@grep Warn $? | sed -e 's/^.*brickos\///' | cut -f1 -d: | sort | \
	uniq -c | sort -rn | tee $@

.Doxyfile-kern-doneflag:  Doxyfile-kern.rpt
	@for FIL in `cat Doxyfile-kern.rpt | cut -c9-99`; do \
       OUTFILE=`echo $$FIL | sed -e 's/[\/\.]/-/g'`.out; \
       echo "# FILE: $$OUTFILE" >$$OUTFILE; \
       grep "$$FIL" Doxyfile-kern.rpt >>$$OUTFILE; \
       grep "$$FIL" Doxyfile-kern.log | grep Warn >>$$OUTFILE; \
    done
	@touch $@

Doxyfile-kern-report: .Doxyfile-kern-doneflag
	-ls -ltr *.out 2>/dev/null

#
#  make all API documentation
#
api-docs-build: html-c html-c++ html-kern

docs: docs-build api-docs-build

#
#  make distribution files for all API documentation
#
api-dist: html-c-dist html-c++-dist html-kern-dist

#
#  install all API documentation
#
docs-install:: html-c-install html-c++-install html-kern-install

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

DISTFILES += Doxyfile Doxyfile-c Doxyfile-c++ h8300-rcx.ld configure \
        makelx.sh README CONTRIBUTORS LICENSE \
    Makefile Makefile.common Makefile.dist

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

#
# install makefiles
#
uninstall::
	rm -rf ${pkglibdir} ${pkgincludedir} ${pkgdocdir}

clean::
	rm -f Makefile.common.dist demo/Makefile.dist demo/c++/Makefile.dist config.h.dist


###
### generic rules
###

# cancel standard C source rule
%.o: %.c

# how to compile native C source
$(NATIVEOBJS): %.o: %.c
	$(CC) -MMD $(CFLAGS) -c $< -o $@

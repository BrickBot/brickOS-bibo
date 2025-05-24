### ==========================================================================
###  FILE: lib/cyc/Makefile - make the Cyclone runtime and libraries
###  brickOS - the independent LEGO Mindstorms OS
### --------------------------------------------------------------------------


include ../Makefile.common

# Add config.h to include path
CINC += -I$(BRICKOS_ROOT)/boot

CYCLONE_SRC=
BUILDLIB=$(CYCLONE_SRC)/bin/buildlib -b $(H8_ARCH) $(CINC)
BUILDSPECS=$(CYCLONE_SRC)/config/buildspecs -b $(H8_ARCH)

cyc-lib: \
	precore_c.h \
	cycspecs \
	cyc_setjmp.h \
	libcyc.a \
	nogc.a \
	runtime_cyc.o

precore_c.h: $(CYCLONE_SRC)/bin/genfiles/precore_c.h
	-mkdir include
	cp $< $@

cycspecs:
	$(BUILDSPECS)> $@

cyc_setjmp.h: precore_c.h brick_c.cys
	$(BUILDLIB) -d include -setjmp > $@
	$(BUILDLIB) -d include brick_c.cys

nogc.a: nogc.o
	-rm $@
	$(AR) rc $@ $<
	$(RANLIB) $@

include/cycstubs.o: include/cycstubs.cyc
	$(CYCLONE) -B. -Iinclude -c -o $@ $<

libcyc.a: include/cycstubs.o
	-rm $@
	$(AR) rc $@ $<
	$(RANLIB) $@

clean:
	-rm -rf include
	-rm precore_c.h cycspecs cyc_setjmp.h nogc.a libcyc.a *.o

.PHONY: cyc-lib clean 


### --------------------------------------------------------------------------
###                          End of FILE: cyc-lib/Makefile
### ==========================================================================

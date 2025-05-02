##
## brickOS - the independent LEGO Mindstorms OS
## Makefile - allows you to keep the upper hand
## (c) 1998, 1999 by Markus L. Noga <markus@noga.de>    
##


PROGRAMS=helloworld2.lx rover2.lx linetrack2.lx robots2.lx oop.lx sound2.lx trailerbot2.lx
# extra dynamic sources
DOBJECTS=Fred.o JCH.o

.PHONY: all oop html depend clean realclean

all:: $(PROGRAMS) oop

include ../Makefile.common
include ../Makefile.user

# doc/html subdirectory

oop:
	$(MAKE) -C oop all

html:
	doxygen Doxyfile

clean:
	rm -rf .depend* *.o *.dis *.dis2 *~ *.bak *.tgz *.s tags *.ds1 *.ds2 *.dmap *.dcoff *.srec *.map *.coff *.gpi
	$(MAKE) -C oop $@

realclean: clean
	rm -f *.lx
	$(MAKE) -C oop $@

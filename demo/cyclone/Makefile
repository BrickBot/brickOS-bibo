### ==========================================================================
###  FILE: demo/cyclone/Makefile - make the Cyclong Language Demo Programs
###  brickOS - the independent LEGO Mindstorms OS
### --------------------------------------------------------------------------

# name for the brickOS kernel
# #LIBDIR#         ###   DO NOT REMOVE THIS LINE  ###
KERNEL=../../boot/brickOS
CYCLIB=../../cyc-lib

# extra dynamic sources
DOBJECTS=$(CYCLIB)/runtime_cyc.o $(CYCLIB)/nogc.a $(CYCLIB)/libcyc.a

include ../../Makefile.common
include ../../Makefile.user

install uninstall:
	@# nothing to do here but do it silently

#  NOTE: --format=1 is not supported on Linux ([ce]tags in emacs2[01] packages)
#   please set in your own environment
tag:
	-ctags *.c ../include/*.h ../include/*/*.h 
	-etags *.c ../include/*.h ../include/*/*.h 

clean::
	rm -f *.o *.dis *.dis2 *~ *.bak *.tgz *.s *.ds1 *.ds2 *.dmap *.dcoff 
	rm -f *.srec *.map *.coff

realclean:: clean
	rm -f *.lx .depend tags TAGS


### --------------------------------------------------------------------------
###                          End of FILE: demo/cyclone/Makefile
### ==========================================================================

##
## brickOS - the independent LEGO Mindstorms OS
## Makefile - allows you to keep the upper hand
## (c) 1998, 1999 by Markus L. Noga <markus@noga.de>    
##

KERNEL = ../../boot/brickOS

PROGRAMS = oopRover.lx oopBatt.lx oopSensor.lx oopLightSensor.lx \
	oopRotSensor.lx \
	oopTouchSensor.lx oopTouchPressedSensor.lx \
	oopMotor.lx oopSound.lx oopActiveSensor.lx oopTempSensor.lx \
	oopRaceTrack.lx

# extra dynamic sources
DOBJECTS= 

.PHONY: all html clean realclean

all:: $(PROGRAMS)

include ../../Makefile.common
include ../../Makefile.user

# doc/html subdirectory
html:
	doxygen Doxyfile

clean:
	rm -rf .depend* *.o *.dis *.dis2 *~ *.bak *.tgz *.s tags *.ds1 *.ds2 *.dmap *.dcoff *.srec *.map *.coff

realclean: clean
	rm -f *.lx

LIBS=-loogpc -lgpc -lc -lmint -lfloat -lc++
PFLAGS=$(CFLAGS) --extended-syntax --unit-path=$(BRICKOS_ROOT)/lib/p --automake

GPC=$(TOOLPREFIX)gpc

# how to compile pas source
%.o: %.pas
	$(GPC) $(PFLAGS) -c $< -o $@

# how to generate an assembly listing of pascal source
%.s: %.pas
	$(GPC) $(PFLAGS) -c $< -S

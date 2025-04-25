##
## brickOS - the independent LEGO Mindstorms OS
## Makefile - allows you to keep the upper hand
## (c) 1998 by Markus L. Noga <markus@noga.de>    
##

# our target

LIBTARGET=gpc
OLIBTARGET=oogpc

# sources & objects

OBJECTS= stub.o types.o irq.o vis.o \
  bitops.o battery.o conio.o cstring.o dbutton.o dkey.o dmotor.o dsound.o h8.o mm.o programs.o \
  registers.o remote.o romlcd.o romsystem.o semaphore.o sound.o stdlib.o time.o timeout.o tm.o unistd.o \
  critsec.o dlcd.o dsensor.o lcd.o \
  lnp.o

OO_OBJECTS= ObjBattery.o ObjSound.o Motor.o MotorPair.o Sensor.o TemperatureSensor.o \
  LightSensor.o RotationSensor.o TouchSensor.o

##
## no user servicable parts below
##

all:: lib$(LIBTARGET).a $(OBJECTS) lib$(OLIBTARGET).a $(OO_OBJECTS)
	cp -f lib$(LIBTARGET).a lib$(OLIBTARGET).a ../

# how to compile pas source
%.o: %.pas
	$(GPC) $(PFLAGS) -c $< -o $@

# how to generate an assembly listing of pascal source
%.s: %.pas
	$(GPC) $(PFLAGS) -c $< -S

include ../../Makefile.common
	
lib$(LIBTARGET).a: $(OBJECTS)
	rm -f lib$(LIBTARGET).a
	$(AR) -sq lib$(LIBTARGET).a $(OBJECTS)

lib$(OLIBTARGET).a: $(OO_OBJECTS)
	rm -f lib$(OLIBTARGET).a
	$(AR) -sq lib$(OLIBTARGET).a $(OO_OBJECTS)

clean:
	rm -rf *.o *.map *.coff *.srec *.dis* *~ *.bak *.tgz *.s tags *.a

LIBS=-loogpc -lgpc -lc -lmint -lfloat -lc++
PFLAGS=$(CFLAGS) --extended-syntax --unit-path=$(BRICKOS_ROOT)/lib/p --automake

GPC=$(TOOLPREFIX)gpc




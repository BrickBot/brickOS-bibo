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

include ../../Makefile.common
	
lib$(LIBTARGET).a: $(OBJECTS)
	rm -f lib$(LIBTARGET).a
	$(CROSSAR) -sq lib$(LIBTARGET).a $(OBJECTS)

lib$(OLIBTARGET).a: $(OO_OBJECTS)
	rm -f lib$(OLIBTARGET).a
	$(CROSSAR) -sq lib$(OLIBTARGET).a $(OO_OBJECTS)

clean:
	rm -rf *.o *.map *.coff *.srec *.dis* *~ *.bak *.tgz *.s tags *.a

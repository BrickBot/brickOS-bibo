CC=/cygnus/cygwin-b20/H-i586-cygwin32/bin/h8300-hms-gcc
LD=/cygnus/cygwin-b20/H-i586-cygwin32/bin/h8300-hms-ld
CFLAGS=-O2 -Wall -I../include -fno-builtin -fomit-frame-pointer -Wno-unused -Wno-uninitialized
LDFLAGS=-T ../h8300.rcx
KOBJECTS=../kmain.o ../mm.o ../systime.o ../tm.o ../semaphore.o ../conio.o \
         ../lcd.o \
         ../direct-ir.o ../direct-motor.o ../direct-sensor.o ../direct-sound.o

all: onscreen.srec onscreen.dis2

onscreen.o: onscreen.c abstractcmd.h macro.h rcx.h rcxstate.h
	$(CC) $(CFLAGS) -c onscreen.c

onscreen.coff: onscreen.o
	$(LD) $(LDFLAGS) $(KOBJECTS) onscreen.o -o onscreen.coff --oformat coff-h8300
	chmod a-x onscreen.coff

onscreen.srec: onscreen.coff
	/cygnus/cygwin-b20/H-i586-cygwin32/bin/h8300-hms-objcopy -I coff-h8300 -O srec  onscreen.coff onscreen.srec
	chmod a-x onscreen.srec

onscreen.dis2: onscreen.coff
	/cygnus/cygwin-b20/H-i586-cygwin32/bin/h8300-hms-nm onscreen.coff | sort -u > onscreen.map
	/cygnus/cygwin-b20/H-i586-cygwin32/bin/h8300-hms-objdump --disassemble-all --no-show-raw-insn -m h8300 onscreen.coff > onscreen.dis
	../util/merge-map onscreen.map onscreen.dis > onscreen.dis2
	rm onscreen.map onscreen.dis

download: all
	../util/firmdl onscreen.srec

clean:
	rm *.srec *.o *.coff *.dis2 *.bak *.core

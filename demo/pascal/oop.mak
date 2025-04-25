ROOT=/brickos
KERNEL=$(ROOT)/boot/brickOS
PROGRAMS=oop.lx
DOBJECTS=Fred.o JCH.o 

all:: $(DOBJECTS) $(PROGRAMS)

download:: all
	$(ROOT)/util/dll --program=1 --tty=usb  --rcxaddr=0 $(PROGRAMS)

set_addr::
	$(ROOT)/util/dll --tty=usb --rcxaddr=0 --node=-1

include $(ROOT)/Makefile.common
include $(ROOT)/Makefile.user

LIBS=-loogpc -lgpc -lc -lmint -lfloat -lc++
PFLAGS=$(CFLAGS) --extended-syntax --unit-path=$(BRICKOS_ROOT)/lib/p --automake

PTOOLPREFIX=/usr/local/bin/h8300-hitachi-hms-
GPC=$(PTOOLPREFIX)gpc

# how to compile pas source
%.o: %.pas
	$(GPC) $(PFLAGS) -c $< -o $@

# how to generate an assembly listing of pascal source
%.s: %.pas
	$(GPC) $(PFLAGS) -c $< -S

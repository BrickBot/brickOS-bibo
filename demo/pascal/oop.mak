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

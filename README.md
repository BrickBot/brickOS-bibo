 bibo isn’t brickOS
====================
From the BrickBot/brickOS-bibo repository on GitHub

INTRODUCTION
------------
Bibo is an alternative Lego MindStorms RCX operating system and firmware that
is a brickOS clone.  The kernel was written from scratch to give better
performance, but to the average brickOS programmer, not much should change.
A lot of code was borrowed from brickOS, and numerous patches originally
targeted for brickOS have been conceptually incorporated.  While this project
is based on bibo, as brickOS still seems to be the more commonly recognizable
name, it has been retained as part of this repository’s name.

Of the various brickOS-tweaking academic projects that are known to exist,
many pertained to some aspect of task scheduling.  It is perhaps less
surprising, then, that this was also the main differentiator between the
original brickOS and bibo.

Comparison with brickOS from the original bibo developer, Jochen Hoenicke:
> [!TIP]
> Bibo is mostly compatible to BrickOS.
>
> The main incompatibility is that there are no time slices any more.
You have to explicitly give the control to other tasks of same priority by
calling yield or by waiting for events. It would be possible to implement time
slices, but I see no sense in tracking a line for 100 ms and then interrupting
the task for 100 ms to check if a button is pressed while driving on, blindly.
>
> On the positive side, Bibo should give you better performance (less CPU overhead),
allows malloc/free in handler (e.g. when receiving IR packets), and
low priority tasks can be preempted even if their time slice has not yet expired.

The original bibo project site is accessible at the following link:
* https://jochen-hoenicke.de/rcx/bibo.html

This repository is believed to be the most progressed version of either brickOS
or bibo.  Since the last official release, over 40 updates to bibo have been
incorporated by Matthew Sheets and are part of this code base.  While these
updates are enumerated at https://news.lugnet.com/robotics/rcx/legos/?n=4057 on
the LEGO Users Group Network (LUGNET), some key changes include the following:

* Support for Power Functions IR control (Carl Troein)
* Updated utils to better facilitate using TCP as a tty.
* Made NCD tty types a runtime option instead of a compile-time #define.
* Merged patches for BrickOS with Bibo, including
    LDCC, lnp_printf, and lnpmsg (formerly LegOShrink)
* Reworked firmdl3, dll, and lnpmsg under utils as well as
    lnp and lnp-logic in the kernel to facilitate better code reuse,
    remove essentially duplicate code, and establish a set of common
    functions for rcx_comm and LNP.  Merged the resulting code into
    util/host, deprecating util/firmld, util/dll-src, and util/fontdesign.c.
* Util dll: made fast mode a runtime option instead of a compile-time #define.
* Added ability to send LNP messages directly from the lnpmsg command line.
* Added ir-server from Jochen Hoenicke's BrickEmu package and used
    the code reworked from above to add support for tty downlinking
    (e.g. serial, USB, TCP) or TCP uplinking.
* Merged XS (based on BrickOS 0.2.6.10) with Bibo.  (XS offers LISP programming support.)
* Changed baud rate, carrier state, and timeout settings from compile-time
    defines to run-time options
* Standardized parity for all baud rates to "odd"
* Added support for RCX-based communication that does not echo (e.g. bluetooth hacks)
* Enabled more flexible communication options for downloading firmware
* Modified conio to remove the CONF_ASCII dependency from CONF_PROGRAM
* Updated Makefiles to support building programs written in Esterel

The inclusion of the enhanced ir-server is expected to supercede lnpd.
As an alternative to ir-server, there is also lnphost, which was itself design to replace lnpd.


HISTORY
-------
The brickOS project itself was originally named legOS.  Early information
and documentation about the project are available
[here](https://arcb.csc.ncsu.edu/~mueller/rt/mindstorm/www.multimania.com/legos/).
The project then moved to SourceForge as legOS
([homepage](http://legos.sf.net/) / [project site](http://sf.net/projects/legos))
before taking up residence in a new SourceForge site as brickOS
([homepage](http://brickos.sf.net/) / [project site](https://sf.net/projects/brickos/)).


ENVIRONMENT CONFIGURATION
-------------------------
A DJGPP setup for legOS on Windows—with downloads—was described at
http://legos.sf.net/files/windows/djgpp/

Slightly more recent setup instructions for Windows and Debian/Linux are
available at http://brickos.sourceforge.net/documents.htm#INSTALLATION

Note that Windows users now also have the option of using the
[Windows Subsystem for Linux](https://docs.microsoft.com/en-us/windows/wsl/initialize-distro).

Regardless of platform, key toolchain components include the following:
* binutils-h8300-hms
  + Provided by the [binutils-cil project](https://github.com/BrickBot/binutils-cil)
* gcc-h8300-hms
  + provided by the [gcc-cil project](https://github.com/BrickBot/gcc-cil)
* make
* gcc
  + linux-libc-dev
* g++


SETUP
-----

This software will run on Linux and Windows (using the Cygwin tool suite.)

To start with brickOS:

1. Type ./configure. Environment should be autodetected (hopefully).
2. Type make. Utilities, libraries, a default kernel and some demos will
  be made.
3. Type make install.  You can change the destination directory by
  editing prefix in Makefile.config.
4. Be sure to add ${prefix}/bin to your PATH. Set RCXTTY if you use a non-
  standard port for the IR tower. Use firmdl3 to download boot/brickOS.srec
  to your RCX.
5. Type "dll demo/helloworld.lx". If all is well, it should be automatically
  relocated and transmitted to program slot 1. Press the RUN key to run. The
  program stops automatically.
6. Type "dll -p2 demo/rover.lx" to transmit another demo to program slot 2.
  Use the PRGM key to select the desired program. Try RUN.

7. Stop your program with the RUN or ON/OFF key. Programs are preserved over
  ON/OFF.  Press ON/OFF and VIEW simultaneously to erase brickOS.

If things don't quite work, work at it.  :-)

For your own projects,

1. Make a directory in the brickOS root directory.
2. Copy /usr/local/brickos/lib/bibo/Makefile.
3. Put your main routine into myproject.c. (.C if using C++)
4. If you need more source files, add them in the Makefile.
5. "make myproject.lx && dll myproject.lx".

In case anyone is wondering, dll is short for dynamic linker and loader.


USB support on Linux for bibo / brickOS
---------------------------------------
This section is taken from the original brickOS repository on SourceForge.

—by Stephen M Moraco (stephen@debian.org) - Mon, 17 Jan 2005 20:18:52 -0700


USB support for IR Towers is now working for Linux and 
Windows (using the Cygwin tool suite.)

This file describes the Linux environment setup.

Recognition of the LEGO USB Infrared Tower is now built into the
Linux kernel starting with 2.6.x.

When you plug the USB IR Tower into a USB port the device will
automatically be connected.

I have usbutils installed which provides 'lsusb'. In my case
lsusb shows the following when I have the IR Tower connected:

	$ lsusb
	Bus 004 Device 001: ID 0000:0000
	Bus 003 Device 001: ID 0000:0000
	Bus 002 Device 001: ID 0000:0000
	Bus 001 Device 002: ID 0694:0001 Lego Group Mindstorms Tower
	Bus 001 Device 001: ID 0000:0000


I had to create my device file with something like:

	sudo mknod /dev/usb/legotower0 c 180 160
	sudo chmod 666 /dev/usb/legotower0


To use the Tower then, I set RCXTTY to point to it:

	export RCXTTY=/dev/usb/legotower0

And then I use firmdl3 and dll just as before.

1. to load brickos
	1. reset RCX removing prior brickOS firmware
	   1. press and hold [Prgm] and simultaneously press [On-Off] to turn the power off and rest
	   2. press [On-Off] to turn the power back on
	   3. listen for the two startup beeps, indicating that you are now
	   running original firmware not brickOS.
	2. run the OS downloader
		```
		$ firmdl3 /usr/local/lib/brickos/brickOS.srec
		```
2. to load a program into slot 2
	1. assuming you build the demos in the current directory, run
		```
		$ dll -p2 sound.lx
		```
	2. you should see the standing-man and “2” indicating that the download was successful.
	3. press [Run] to play the song...

I hope this works for you. Please report issues with the USB support
per the instructions found at http://brickos.sourceforge.net/rptrvwbugs.htm


------------------------------------------------------------------------
bibo Copyright © 2007 by Jochen Hoenicke
legOS/brickOS Copyright © 1998-2005 by Markus L. Noga

Major bibo updates integrated by Matthew Sheets

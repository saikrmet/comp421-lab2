#
#	Sample Makefile for COMP 421 Yalnix kernel and user programs.
#
#	The Yalnix kernel built will be named "yalnix".  *ALL* kernel
#	Makefiles for this lab must have a "yalnix" rule in them, and
#	must produce a kernel executable named "yalnix" -- we will run
#	your Makefile and will grade the resulting executable
#	named "yalnix".
#
#	Your project must be implemented using the C programming
#	language (e.g., not in C++ or other languages).
#

#
#	Define the list of everything to be made by this Makefile.
#	The list should include "yalnix" (the name of your kernel),
#	plus the list of user test programs you also want to be mae
#	by this Makefile.  For example, the definition below
#	specifies to make Yalnix test user programs test1, test2,
#	and test3.  You should modify this list to the list of your
#	own test programs.
#
#	For each user test program, the Makefile will make the
#	program out of a single correspondingly named sourc file.
#	For example, the Makefile will make test1 out of test1.c,
#	if you have a file named test1.c in this directory.
#
ALL = yalnix idle init samples-lab2/bigstack samples-lab2/blowstack samples-lab2/brktest samples-lab2/console samples-lab2/delaytest samples-lab2/exectest samples-lab2/forktest0 samples-lab2/forktest1 samples-lab2/forktest1b samples-lab2/forktest2 samples-lab2/forktest2b samples-lab2/forktest3 samples-lab2/forkwait0c samples-lab2/forkwait0p samples-lab2/forkwait1 samples-lab2/forkwait1b samples-lab2/forkwait1c samples-lab2/forkwait1d samples-lab2/init samples-lab2/init1 samples-lab2/init2 samples-lab2/init3 samples-lab2/shell samples-lab2/trapillegal samples-lab2/trapmath samples-lab2/trapmemory samples-lab2/ttyread1 samples-lab2/ttywrite1 samples-lab2/ttywrite2 samples-lab2/ttywrite3

#
#	You must modify the KERNEL_OBJS and KERNEL_SRCS definitions
#	below.  KERNEL_OBJS should be a list of the .o files that
#	make up your kernel, and KERNEL_SRCS should  be a list of
#	the corresponding source files that make up your kernel.
#
KERNEL_OBJS = contextSwitch.o handleProcesses.o kernel.o load.o pageTableController.o memory.o pcb.o terminalHandler.o trapHandling.o
KERNEL_SRCS = contextSwitch.c handleProcesses.c kernel.c load.c pageTableController.c memory.c pcb.c terminalHandler.c trapHandling.c samples-lab2/bigstack.c samples-lab2/blowstack.c samples-lab2/brktest.c samples-lab2/console.c samples-lab2/delaytest.c samples-lab2/exectest.c samples-lab2/forktest0.c samples-lab2/forktest1.c samples-lab2/forktest1b.c samples-lab2/forktest2.c samples-lab2/forktest2b.c samples-lab2/forktest3.c samples-lab2/forkwait0c.c samples-lab2/forkwait0p.c samples-lab2/forkwait1.c samples-lab2/forkwait1b.c samples-lab2/forkwait1c.c samples-lab2/forkwait1d.c samples-lab2/init.c samples-lab2/init1.c samples-lab2/init2.c samples-lab2/init3.c samples-lab2/shell.c samples-lab2/trapillegal.c samples-lab2/trapmath.c samples-lab2/trapmemory.c samples-lab2/ttyread1.c samples-lab2/ttywrite1.c samples-lab2/ttywrite2.c samples-lab2/ttywrite3.c

#
#	You should not have to modify anything else in this Makefile
#	below here.  If you want to, however, you may modify things
#	such as the definition of CFLAGS, for example.
#

PUBLIC_DIR = /clear/courses/comp421/pub

CPPFLAGS = -I$(PUBLIC_DIR)/include
CFLAGS = -g -Wall -Wextra -Werror

LANG = gcc

%: %.o
	$(LINK.o) -o $@ $^ $(LOADLIBES) $(LDLIBS)

LINK.o = $(PUBLIC_DIR)/bin/link-user-$(LANG) $(LDFLAGS) $(TARGET_ARCH)

%: %.c
%: %.cc
%: %.cpp

all: $(ALL)

yalnix: $(KERNEL_OBJS)
	$(PUBLIC_DIR)/bin/link-kernel-$(LANG) -o yalnix $(KERNEL_OBJS)

clean:
	rm -f $(KERNEL_OBJS) $(ALL)

depend:
	$(CC) $(CPPFLAGS) -M $(KERNEL_SRCS) > .depend

#include .depend

TARGET = ptextviewer
PSP_FW_VERSION = 500
BUILD_PRX = 1

OBJS =	\
	main.o \
	file.o \
	pprefsmenu.o \
	button.o \
	exports.o \
	thread.o \
	conf.o \
	syslibc/syslibc.o \
	syslibc/strncasecmp.o \
	syslibc/qsort.o \
	syslibc/memory.o \

PRX_EXPORTS = exports.exp
USE_KERNEL_LIBC	= 1
USE_KERNEL_LIBS	= 1

CLASSG_LIBS = libs
#CLASSG_LIBS = libc




INCDIR = $(CLASSG_LIBS)
CFLAGS = -Os -G0 -Wall -fno-strict-aliasing -fno-builtin-printf -DLNGJPN
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)
#INCDIR			=	$(DIR_CLASSG)
#CFLAGS		=	-Os -G 0 -Wall -fno-strict-aliasing -DLNGJPN
##CFLAGS 	= -O3 -G0 -Wall -DLNGJPN
#ASFLAGS		=	$(CFLAGS)

LIBDIR			= 
LDFLAGS		=	-mno-crt0 -nostartfiles
LIBS = libmenuj.a -lpspreg -lpspumd -lpsprtc -lpspsystemctrl_kernel -lpspkubridge -lm -lz


PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build_prx.mak

all:
	psp-packer *.prx
clean:
	\rm *.o *.elf *.prx

TARGET_LIB = usb.a

OBJS += usb.o

INCDIR		= 
CFLAGS 		= -O3 -G0 -Wall
CXXFLAGS 	= $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS 	= $(CFLAGS)

LIBDIR		= 
LDFLAGS		= 
all:
	cp usb.a usb.h ../pprefs-prx

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

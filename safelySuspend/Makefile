TARGET_LIB = safelySuspend.a

OBJS += safelySuspend.o

INCDIR		= 
CFLAGS 		= -O3 -G0 -Wall -DLNGJPN
CXXFLAGS 	= $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS 	= $(CFLAGS)

LIBDIR		= 
LDFLAGS		= 
all:
	cp safelySuspend.a safelySuspend.h ../pprefs-prx

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

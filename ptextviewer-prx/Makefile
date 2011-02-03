TARGET = ptextviewer
PSP_FW_VERSION = 500
BUILD_PRX = 1

#6.35 Pro用で作る場合は PSP_HEN_VERSION を有効にしてコンパイル (先頭の#を外してコメントアウト）
#PSP_HEN_VERSION	= 	635

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

ifeq ($(PSP_HEN_VERSION),635)
OBJS		+=	../nidResolve/635/sceCtrl_driver.o
OBJS		+=	../nidResolve/635/sceDisplay_driver.o 
endif

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
LIBS = ../libmenu/libmenuj.a -lpspreg -lpspumd -lpsprtc -lpspsystemctrl_kernel -lpspkubridge -lm -lz


PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build_prx.mak

ifeq ($(PSP_HEN_VERSION),635)
all:

else
all:
	psp-packer $(TARGET).prx
endif

TARGET = pprefs
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
	configmenu.o \
	sepluginstxt.o \
	editpergame.o \
	fileselecter.o \
	syslibc/syslibc.o \
	syslibc/strncasecmp.o \
	syslibc/qsort.o \
	syslibc/memory.o \

#	sceModuleManager_driver.o \


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
LIBS = ../libmenu/libmenuj.a -lpspsystemctrl_kernel -lm -lz 



PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build_prx.mak

ifeq ($(PSP_HEN_VERSION),635)
all:

else
all:
	psp-packer pprefs.prx
endif

main.o: main.c main.h libmenu.h memory.h file.h conf.h button.h thread.h color.h

file.o: file.c main.h libmenu.h button.h file.h

button.o: button.c button.h

thread.o: thread.c thread.h

conf.o: conf.c conf.h button.h file.h

configmenu.o: configmenu.c common.h libmenu.h button.h main.h language.h

editpergame.o: editpergame.c common.h libmenu.h memory.h button.h fileselecter.h  main.h language.h

fileselecter.o: fileselecter.c fileselecter.h common.h button.h libmenu.h  language.h 


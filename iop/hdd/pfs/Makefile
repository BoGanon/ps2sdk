# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004.
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.


IOP_BIN  = bin/ps2fs.irx
IOP_OBJS = obj/bitmap.o obj/dir.o obj/inode.o obj/journal.o obj/misc.o \
	 obj/pfs.o obj/pfs_fio.o obj/super.o obj/cache.o obj/block.o \
	 obj/pfs_fioctl.o obj/imports.o

IOP_CFLAGS  += -Wall -fno-builtin-printf
IOP_LDFLAGS += -s
IOP_INCS += -I$(PS2SDKSRC)/iop/dev9/atad/include
IOP_LIBS += -lgcc

all: $(IOP_BIN)

include $(PS2SDKSRC)/Defs.make
include $(PS2SDKSRC)/iop/Rules.make

# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

EE_OBJS_DIR ?= obj/
EE_SRC_DIR ?= src/
EE_INC_DIR ?= include/
EE_SAMPLE_DIR ?= samples/

EE_INCS := $(EE_INCS) -I$(EE_SRC_DIR) -I$(EE_SRC_DIR)include -I$(EE_INC_DIR) \
	  -I$(PS2SDKSRC)/ee/kernel/include -I$(PS2SDKSRC)/common/include \
	  -I$(PS2SDKSRC)/ee/erl/include

# C compiler flags
EE_CFLAGS := -D_EE -G0 -O2 -Wall $(EE_INCS) $(EE_CFLAGS)

# C++ compiler flags
EE_CXXFLAGS := -D_EE -G0 -O2 -Wall $(EE_INCS) $(EE_CXXFLAGS)

# Linker flags
# EE_LDFLAGS := $(EE_LDFLAGS)

# Assembler flags
EE_ASFLAGS := $(EE_ASFLAGS)

EE_SAMPLES := $(EE_SAMPLES:%=$(EE_SAMPLE_DIR)%)

EE_OBJS := $(EE_OBJS:%=$(EE_OBJS_DIR)%)

ERL_OBJS := $(ERL_OBJS:%=$(EE_OBJS_DIR)%)

# Externally defined variables: EE_BIN, EE_OBJS, EE_LIB

# These macros can be used to simplify certain build rules.
EE_C_COMPILE := $(EE_CC) $(EE_CFLAGS)
EE_CXX_COMPILE := $(EE_CXX) $(EE_CXXFLAGS)

$(EE_OBJS_DIR)%.o: $(EE_SRC_DIR)%.c
	$(EE_C_COMPILE) -c $< -o $@

$(EE_OBJS_DIR)%.o: $(EE_SRC_DIR)%.cpp
	$(EE_CXX_COMPILE) -c $< -o $@

$(EE_OBJS_DIR)%.o: $(EE_SRC_DIR)%.S
	$(EE_C_COMPILE) -c $< -o $@

$(EE_OBJS_DIR)%.o: $(EE_SRC_DIR)%.s
	$(EE_AS) $(EE_ASFLAGS) $< -o $@

$(EE_LIB_DIR):
	$(MKDIR) -p $(EE_LIB_DIR)

$(EE_BIN_DIR):
	$(MKDIR) -p $(EE_BIN_DIR)

$(EE_OBJS_DIR):
	$(MKDIR) -p $(EE_OBJS_DIR)

$(EE_OBJS): | $(EE_OBJS_DIR)

$(ERL_OBJS): | $(EE_OBJS_DIR)

$(EE_BIN): $(EE_OBJS) | $(EE_BIN_DIR)
	$(EE_CC) -nodefaultlibs $(EE_CFLAGS) -o $(EE_BIN) $(EE_OBJS) $(EE_LDFLAGS) $(EE_LIBS)

$(EE_LIB): $(EE_OBJS) $(EE_LIB:%.a=%.erl) | $(EE_LIB_DIR)
	$(EE_AR) cru $(EE_LIB) $(EE_OBJS)

$(EE_LIB:%.a=%.erl): $(EE_OBJS) $(ERL_OBJS) | $(EE_LIB_DIR)
	$(EE_CC) -nostdlib -Wl,-r -Wl,-d -o $(EE_LIB:%.a=%.erl) $(EE_OBJS) $(ERL_OBJS)
	$(EE_STRIP) --strip-unneeded -R .mdebug.eabi64 -R .reginfo -R .comment $(EE_LIB:%.a=%.erl)

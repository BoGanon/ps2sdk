# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

ifeq (x$(PS2SDKSRC), x)
  export PS2SDKSRC=$(shell pwd)
endif

all: build
	@$(ECHO) .;
	@$(ECHO) .PS2SDK Built.;
	@$(ECHO) .;

# Environment rules

env_build_check:
	@if test -z $(PS2SDKSRC) ; \
	then \
	  $(ECHO) PS2SDKSRC environment variable should be defined. ; \
	fi

env_release_check:
	@if test -z $(PS2SDK) ; \
	then \
	  $(ECHO) PS2SDK environment variable must be defined. ; \
	  exit 1; \
	fi

# Common rules shared by all build targets.
SUBDIRS = tools
SUBDIRS += iop
SUBDIRS += ee
SUBDIRS += common
SUBDIRS += samples

.PHONY: dummy tools iop ee common samples

# Use SUBDIRS to descend into subdirectories.
subdir_clean = $(patsubst %,clean-%,$(SUBDIRS))
subdir_release = $(patsubst %,release-%,$(SUBDIRS))
subdirs: dummy $(subdir_list)

$(subdir_clean): dummy
	$(MAKE) -C $(patsubst clean-%,%,$@) clean
	$(MAKE) -C ee/erl-loader clean
$(subdir_release): dummy
	$(MAKE) -C $(patsubst release-%,%,$@) release
	$(MAKE) -C ee/erl-loader release

# Build rules
tools: env_build_check
	$(MAKE) -C tools all

ee_lib: tools
	$(MAKE) -C ee all

ee: ee_lib
	$(MAKE) -C ee/erl-loader all

iop_lib: tools
	$(MAKE) -C iop all

iop: iop_lib

common: tools
	$(MAKE) -C common all

build: common ee iop

rebuild: env_build_check $(subdir_clean) build

# Clean rules
clean: env_build_check $(subdir_clean)

# Release rules
install: release

release_base_dir: env_release_check
	$(MKDIR) -p $(PS2SDK)

release_ports_dir: env_release_check
	$(MKDIR) -p $(PS2SDK)/ports

release_base: release_base_dir
	cp -f README.md $(PS2SDK)
	cp -f CHANGELOG $(PS2SDK)
	cp -f AUTHORS $(PS2SDK)
	cp -f LICENSE.md $(PS2SDK)
	cp -f ID $(PS2SDK)

release-clean: env_release_check
	$(MAKE) -C common release-clean
	$(MAKE) -C iop release-iop-clean
	$(MAKE) -C ee release-ee-clean
	$(MAKE) -C samples release-clean
	$(MAKE) -C tools release-clean
	rm -f $(PS2SDK)/README.md
	rm -f $(PS2SDK)/CHANGELOG
	rm -f $(PS2SDK)/AUTHORS
	rm -f $(PS2SDK)/LICENSE.md
	rm -f $(PS2SDK)/ID

release: build release-clean release_base release_ports_dir $(subdir_release)
	@$(ECHO) .;
	@$(ECHO) .PS2SDK Installed.;
	@$(ECHO) .;

# install-headers rules
headers_clean: env_release_check
	$(MAKE) -C ee release-ee-clean
	$(MAKE) -C common release-clean
	rm -f $(PS2SDK)/README.md
	rm -f $(PS2SDK)/CHANGELOG
	rm -f $(PS2SDK)/AUTHORS
	rm -f $(PS2SDK)/LICENSE.md
	rm -f $(PS2SDK)/ID

header_dirs: release_base
	$(MKDIR) -p $(PS2SDK)/ee
	$(MKDIR) -p $(PS2SDK)/ee/include
	$(MAKE) -C common release-dirs

install-headers: headers_clean header_dirs
	$(MAKE) -C ee/libc release-ee-include
	$(MAKE) -C ee/fdlibm release-ee-include
	$(MAKE) -C common install-include
	$(MAKE) -C common link-ee-include
	@$(ECHO) .;
	@$(ECHO) PS2SDK LIBC headers installed.;
	@$(ECHO) .;

# Miscellaneous rules
help:
	@$(ECHO) "usage: make <options> <target>";
	@$(ECHO) "";
	@$(ECHO) "  Variables:        Value:";
	@$(ECHO) "    PS2SDK            Install directory";
	@$(ECHO) "    PS2SDKSRC         Source directory";
	@$(ECHO) "";
	@$(ECHO) "  Targets:";
	@$(ECHO) "    all               Build ps2sdk";
	@$(ECHO) "    install           Install ps2sdk";
	@$(ECHO) "    clean             Clean source directory";
	@$(ECHO) "    ee                Build only EE libraries";
	@$(ECHO) "    iop               Build only IOP libraries";
	@$(ECHO) "    common            Build only common libraries";
	@$(ECHO) "    tools             Build only ps2sdk tools";
	@$(ECHO) "    rebuild           Clean then build ps2sdk";
	@$(ECHO) "    release-clean     Clean installed files";
	@$(ECHO) "    install-headers   Install libc headers";
	@$(ECHO) "    help              Print this message";

include Defs.make

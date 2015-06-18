# Usage:
# /path/to/component$ make PLATFORM=qemu

# Directories
TOPDIR := ../..
COMPILE_DIR := $(TOPDIR)/compile

# include make initialization
include $(COMPILE_DIR)/MakeInit

# include the Makefile vars.
include $(COMPILE_DIR)/MakeVars

# call MakeVars
include MakeVars

# Target
# lets just inspect if any default target guessed by make
ifeq ($(VERBOSE),1)
$(info .DEFAULT_GOAL := $(.DEFAULT_GOAL))
endif

clean:
	@rm -rf $(OBJDIR)
	@if [ $$? -eq 0 ] ; then echo; echo "$$(tput setaf 2)Clean!$$(tput sgr0)"; echo; fi

install:
	$(install_template)

#end

# Usage:
# /path/to/compile$ make PLATFORM=qemu

# Directories
TOPDIR := ..
COMPILE_DIR := $(TOPDIR)/compile

# components to compile
# it will be assigned further
COMPS :=

# include make initialization
include $(COMPILE_DIR)/MakeInit

# include the Makefile vars.
include $(COMPILE_DIR)/MakeVars

# call MakeVars of components
include $(COMPS:%=$(TOPDIR)/%/MakeVars)

# include make targets
include $(COMPILE_DIR)/MakeTargets

#end

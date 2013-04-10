ARCH =i586-elf-
CC = ${ARCH}gcc
CPP = ${ARCH}g++
AS = ${ARCH}as
LD = ${ARCH}ld
AR = ${ARCH}ar
OBJCOPY = ${ARCH}objcopy
STRIP = ${ARCH}strip


PLATFORM = x86

#Release Version -> Optimize
#CFLAGS = -O3 -std=gnu99 -Wunused  -Werror -D__$(PLATFORM)__ -DRASPBERRY_PI -fno-builtin
#ASFLAGS =


#debug version -> with -g
#CFLAGS = -O0 -g -std=gnu99 -Werror -D__$(PLATFORM)__ -DRASPBERRY_PI -fno-builtin -lgcc #-fno-exceptions -fnon-call-exceptions
#ASFLAGS = -g

#test version
CFLAGS =  -nostdlib -nostdinc -fno-builtin -fno-stack-protector -std=gnu99 -Werror -DUSEASSERT
ASFLAGS =


CFLAGS_FOR_TARGET =
ASFLAGS_FOR_TARGET =
LDFLAGS =


# Directories
OBJDIR = bin
SRCDIR = src

# Project name
PROJECT = kernel

# Files and folders
CHDRS    = $(shell find $(SRCDIR) -name '*.h')
CHDRDIRS = $(shell find $(SRCDIR) -type d | sed 's/$(SRCDIR)/-I$(SRCDIR)/g' )

CSRCS    = $(shell find $(SRCDIR) -name '*.c')
CSRCDIRS = $(shell find $(SRCDIR) -type d | sed 's/$(SRCDIR)/./g' )
COBJS    = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(CSRCS))

ASRCS    = $(shell find $(SRCDIR) -name '*.s')
ASRCDIRS = $(shell find $(SRCDIR) -type d | sed 's/$(SRCDIR)/./g' )
AOBJS    = $(patsubst $(SRCDIR)/%.s,$(OBJDIR)/%.o,$(ASRCS))

NSRCS    = $(shell find $(SRCDIR) -name '*.asm')
NSRCDIRS = $(shell find $(SRCDIR) -type d | sed 's/$(SRCDIR)/./g' )
NOBJS    = $(patsubst $(SRCDIR)/%.asm,$(OBJDIR)/%.o,$(NSRCS))

OBJS = $(NOBJS) $(AOBJS) $(COBJS)

INCLUDES := -Isrc $(CHDRDIRS)


$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS_FOR_TARGET) $(INCLUDES) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/%.o: $(SRCDIR)/%.s
	$(AS) $(ASFLAGS_FOR_TARGET) $(INCLUDES) $(ASFLAGS) -o $@ $<

$(OBJDIR)/%.o: $(SRCDIR)/%.asm
	nasm $(INCLUDES) -felf -o $@ $<

# Target
$(PROJECT): buildrepo $(OBJDIR)/kernel.img
	cp $(OBJDIR)/kernel.img $@

$(OBJDIR)/kernel.img: $(OBJDIR)/kernel.bin
	${OBJCOPY} -O binary $< $@
	#${STRIP} $<

$(OBJDIR)/kernel.bin: arch_x86.ld $(OBJS)
	${LD} ${LDFLAGS} $(OBJS) -Map bin/kernel.map -o $@ -T arch_x86.ld

clean:
	rm -rf $(OBJDIR)

buildrepo:
	$(call make-repo)
	
vmware:
	qemu-img convert -f raw harddisk.img -O vmdk kernel.vmdk
	cp kernel.vmdk ~/vmware/Other/Other.vmdk

# Create obj directory structure
define make-repo
	mkdir -p $(OBJDIR)
	for dir in $(CSRCDIRS) $(ASRCDIRS) $(NSRCDIRS); \
	do \
		mkdir -p $(OBJDIR)/$$dir; \
	done
endef

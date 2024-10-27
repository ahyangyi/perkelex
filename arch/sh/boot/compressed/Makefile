# SPDX-License-Identifier: GPL-2.0
#
# linux/arch/sh/boot/compressed/Makefile
#
# create a compressed perkelex image from the original perkelex
#

OBJECTS := head_32.o misc.o piggy.o \
           ashiftrt.o ashldi3.o ashrsi3.o ashlsi3.o lshrsi3.o

targets := perkelex perkelex.bin perkelex.bin.gz perkelex.bin.bz2 \
           perkelex.bin.lzma perkelex.bin.xz perkelex.bin.lzo $(OBJECTS)

#
# IMAGE_OFFSET is the load offset of the compression loader
#
ifeq ($(CONFIG_32BIT),y)
IMAGE_OFFSET	:= $(shell /bin/bash -c 'printf "0x%08x" \
		     $$[$(CONFIG_MEMORY_START)  + \
			$(CONFIG_BOOT_LINK_OFFSET)]')
else
IMAGE_OFFSET	:= $(shell /bin/bash -c 'printf "0x%08x" \
		     $$[$(CONFIG_PAGE_OFFSET)  + \
			$(KERNEL_MEMORY) + \
			$(CONFIG_BOOT_LINK_OFFSET)]')
endif

ccflags-remove-$(CONFIG_MCOUNT) += -pg

LDFLAGS_perkelex := --oformat $(ld-bfd) -Ttext $(IMAGE_OFFSET) -e startup \
		   -T $(obj)/../../kernel/perkelex.lds

KBUILD_CFLAGS += -DDISABLE_BRANCH_PROFILING

$(obj)/perkelex: $(addprefix $(obj)/, $(OBJECTS)) FORCE
	$(call if_changed,ld)

$(obj)/perkelex.bin: perkelex FORCE
	$(call if_changed,objcopy)

$(obj)/perkelex.bin.gz: $(obj)/perkelex.bin FORCE
	$(call if_changed,gzip)
$(obj)/perkelex.bin.bz2: $(obj)/perkelex.bin FORCE
	$(call if_changed,bzip2_with_size)
$(obj)/perkelex.bin.lzma: $(obj)/perkelex.bin FORCE
	$(call if_changed,lzma_with_size)
$(obj)/perkelex.bin.xz: $(obj)/perkelex.bin FORCE
	$(call if_changed,xzkern_with_size)
$(obj)/perkelex.bin.lzo: $(obj)/perkelex.bin FORCE
	$(call if_changed,lzo_with_size)

OBJCOPYFLAGS += -R .empty_zero_page

LDFLAGS_piggy.o := -r --format binary --oformat $(ld-bfd) -T

$(obj)/piggy.o: $(obj)/perkelex.scr $(obj)/perkelex.bin.$(suffix_y) FORCE
	$(call if_changed,ld)

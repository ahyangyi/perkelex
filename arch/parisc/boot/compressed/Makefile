# SPDX-License-Identifier: GPL-2.0-only
#
# linux/arch/parisc/boot/compressed/Makefile
#
# create a compressed self-extracting perkelex image from the original perkelex
#

OBJECTS := head.o real2.o firmware.o misc.o piggy.o
targets := perkelex.lds perkelex perkelex.bin perkelex.bin.gz perkelex.bin.bz2
targets += perkelex.bin.xz perkelex.bin.lzma perkelex.bin.lzo perkelex.bin.lz4
targets += $(OBJECTS) sizes.h

KBUILD_CFLAGS := -D__KERNEL__ -O2 -DBOOTLOADER
KBUILD_CFLAGS += -DDISABLE_BRANCH_PROFILING
KBUILD_CFLAGS += -fno-strict-aliasing
KBUILD_CFLAGS += $(cflags-y) -fno-delete-null-pointer-checks -fno-builtin-printf
KBUILD_CFLAGS += -fno-PIE -mno-space-regs -mdisable-fpregs -Os
ifndef CONFIG_64BIT
KBUILD_CFLAGS += -mfast-indirect-calls
endif

LDFLAGS_perkelex := -X -e startup --as-needed -T
$(obj)/perkelex: $(obj)/perkelex.lds $(addprefix $(obj)/, $(OBJECTS)) $(LIBGCC) FORCE
	$(call if_changed,ld)

sed-sizes := -e 's/^\([0-9a-fA-F]*\) . \(__bss_start\|_end\|parisc_kernel_start\)$$/\#define SZ\2 0x\1/p'

quiet_cmd_sizes = GEN $@
      cmd_sizes = $(NM) $< | sed -n $(sed-sizes) > $@

$(obj)/sizes.h: perkelex FORCE
	$(call if_changed,sizes)

AFLAGS_head.o += -I$(objtree)/$(obj) -DBOOTLOADER
$(obj)/head.o: $(obj)/sizes.h

CFLAGS_misc.o += -I$(objtree)/$(obj)
$(obj)/misc.o: $(obj)/sizes.h

AFLAGS_real2.o += -DBOOTLOADER

CPPFLAGS_perkelex.lds += -I$(objtree)/$(obj) -DBOOTLOADER
$(obj)/perkelex.lds: $(obj)/sizes.h

OBJCOPYFLAGS_perkelex.bin := -R .comment -R .note -S
$(obj)/perkelex.bin: perkelex FORCE
	$(call if_changed,objcopy)

suffix-$(CONFIG_KERNEL_GZIP)  := gz
suffix-$(CONFIG_KERNEL_BZIP2) := bz2
suffix-$(CONFIG_KERNEL_LZ4)  := lz4
suffix-$(CONFIG_KERNEL_LZMA)  := lzma
suffix-$(CONFIG_KERNEL_LZO)  := lzo
suffix-$(CONFIG_KERNEL_XZ)  := xz

$(obj)/perkelex.bin.gz: $(obj)/perkelex.bin FORCE
	$(call if_changed,gzip)
$(obj)/perkelex.bin.bz2: $(obj)/perkelex.bin FORCE
	$(call if_changed,bzip2_with_size)
$(obj)/perkelex.bin.lz4: $(obj)/perkelex.bin FORCE
	$(call if_changed,lz4_with_size)
$(obj)/perkelex.bin.lzma: $(obj)/perkelex.bin FORCE
	$(call if_changed,lzma_with_size)
$(obj)/perkelex.bin.lzo: $(obj)/perkelex.bin FORCE
	$(call if_changed,lzo_with_size)
$(obj)/perkelex.bin.xz: $(obj)/perkelex.bin FORCE
	$(call if_changed,xzkern_with_size)

LDFLAGS_piggy.o := -r --format binary --oformat $(LD_BFD) -T
$(obj)/piggy.o: $(obj)/perkelex.scr $(obj)/perkelex.bin.$(suffix-y) FORCE
	$(call if_changed,ld)

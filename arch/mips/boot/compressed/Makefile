#
# This file is subject to the terms and conditions of the GNU General Public
# License.
#
# Adapted for MIPS Pete Popov, Dan Malek
#
# Copyright (C) 1994 by Linus Torvalds
# Adapted for PowerPC by Gary Thomas
# modified by Cort (cort@cs.nmt.edu)
#
# Copyright (C) 2009 Lemote Inc. & DSLab, Lanzhou University
# Author: Wu Zhangjin <wuzhangjin@gmail.com>
#

include $(srctree)/arch/mips/Kbuild.platforms

# set the default size of the mallocing area for decompressing
BOOT_HEAP_SIZE := 0x400000

# Disable Function Tracer
KBUILD_CFLAGS := $(filter-out $(CC_FLAGS_FTRACE), $(KBUILD_CFLAGS))

KBUILD_CFLAGS := $(filter-out -fstack-protector, $(KBUILD_CFLAGS))

# Disable lq/sq in zboot
ifdef CONFIG_CPU_LOONGSON64
KBUILD_CFLAGS := $(filter-out -march=loongson3a, $(KBUILD_CFLAGS)) -march=mips64r2
endif

KBUILD_CFLAGS := $(KBUILD_CFLAGS) -D__KERNEL__ -D__DISABLE_EXPORTS \
	-DBOOT_HEAP_SIZE=$(BOOT_HEAP_SIZE) -D"VMLINUX_LOAD_ADDRESS_ULL=$(VMLINUX_LOAD_ADDRESS)ull"

KBUILD_AFLAGS := $(KBUILD_AFLAGS) -D__ASSEMBLY__ \
	-DBOOT_HEAP_SIZE=$(BOOT_HEAP_SIZE) \
	-DKERNEL_ENTRY=$(VMLINUX_ENTRY_ADDRESS)

# decompressor objects (linked with perkelez)
perkelezobjs-y := $(obj)/head.o $(obj)/decompress.o $(obj)/string.o $(obj)/bswapsi.o

ifdef CONFIG_DEBUG_ZBOOT
perkelezobjs-$(CONFIG_DEBUG_ZBOOT)		   += $(obj)/dbg.o
perkelezobjs-$(CONFIG_SYS_SUPPORTS_ZBOOT_UART16550) += $(obj)/uart-16550.o
perkelezobjs-$(CONFIG_SYS_SUPPORTS_ZBOOT_UART_PROM) += $(obj)/uart-prom.o
perkelezobjs-$(CONFIG_MIPS_ALCHEMY)		   += $(obj)/uart-alchemy.o
perkelezobjs-$(CONFIG_ATH79)			   += $(obj)/uart-ath79.o
endif

perkelezobjs-$(CONFIG_KERNEL_XZ) += $(obj)/ashldi3.o

perkelezobjs-$(CONFIG_KERNEL_ZSTD) += $(obj)/bswapdi.o $(obj)/ashldi3.o $(obj)/clz_ctz.o

targets := $(notdir $(perkelezobjs-y))

targets += perkelex.bin

OBJCOPYFLAGS_perkelex.bin := $(OBJCOPYFLAGS) -O binary -R .comment -S

$(obj)/perkelex.bin: $(KBUILD_IMAGE) FORCE
	$(call if_changed,objcopy)

tool_$(CONFIG_KERNEL_GZIP)    = gzip
tool_$(CONFIG_KERNEL_BZIP2)   = bzip2_with_size
tool_$(CONFIG_KERNEL_LZ4)     = lz4_with_size
tool_$(CONFIG_KERNEL_LZMA)    = lzma_with_size
tool_$(CONFIG_KERNEL_LZO)     = lzo_with_size
tool_$(CONFIG_KERNEL_XZ)      = xzkern_with_size
tool_$(CONFIG_KERNEL_ZSTD)    = zstd22_with_size

targets += perkelex.bin.z

$(obj)/perkelex.bin.z: $(obj)/perkelex.bin FORCE
	$(call if_changed,$(tool_y))

targets += piggy.o dummy.o

OBJCOPYFLAGS_piggy.o := --add-section=.image=$(obj)/perkelex.bin.z \
			--set-section-flags=.image=contents,alloc,load,readonly,data

$(obj)/piggy.o: $(obj)/dummy.o $(obj)/perkelex.bin.z FORCE
	$(call if_changed,objcopy)

HOSTCFLAGS_calc_perkelez_load_addr.o += $(LINUXINCLUDE)

# Calculate the load address of the compressed kernel image
hostprogs := calc_perkelez_load_addr

ifneq (0x0,$(CONFIG_ZBOOT_LOAD_ADDRESS))
zload-y	= $(CONFIG_ZBOOT_LOAD_ADDRESS)
endif

ifneq ($(zload-y),)
VMLINUZ_LOAD_ADDRESS := $(zload-y)
else
VMLINUZ_LOAD_ADDRESS = $(shell $(obj)/calc_perkelez_load_addr \
		$(obj)/perkelex.bin $(LINKER_LOAD_ADDRESS))
endif
UIMAGE_LOADADDR = $(VMLINUZ_LOAD_ADDRESS)

perkelezobjs-y += $(obj)/piggy.o

targets += ../../../../perkelez

quiet_cmd_zld = LD      $@
      cmd_zld = $(LD) $(KBUILD_LDFLAGS) -Ttext $(VMLINUZ_LOAD_ADDRESS) -T $< $(perkelezobjs-y) -o $@
quiet_cmd_strip = STRIP   $@
      cmd_strip = $(STRIP) -s $@

$(objtree)/perkelez: $(src)/ld.script $(perkelezobjs-y) $(obj)/calc_perkelez_load_addr
	$(call cmd,zld)
	$(call cmd,strip)

objboot := $(objtree)/arch/mips/boot

$(objboot)/perkelez: $(objtree)/perkelez FORCE

#
# Some DECstations need all possible sections of an ECOFF executable
#
ifdef CONFIG_MACH_DECSTATION
  e2eflag := -a
endif

# elf2ecoff can only handle 32bit image
hostprogs += ../elf2ecoff

ifdef CONFIG_32BIT
	VMLINUZ = $(objtree)/perkelez
else
	VMLINUZ = $(objboot)/perkelez.32
endif

targets += ../perkelez.32

quiet_cmd_32 = OBJCOPY $@
      cmd_32 = $(OBJCOPY) -O $(32bit-bfd) $(OBJCOPYFLAGS) $< $@

$(objboot)/perkelez.32: $(objtree)/perkelez
	$(call cmd,32)

targets += ../perkelez.ecoff

quiet_cmd_ecoff = ECOFF	  $@
      cmd_ecoff = $< $(VMLINUZ) $@ $(e2eflag)

$(objboot)/perkelez.ecoff: $(objboot)/elf2ecoff $(VMLINUZ)
	$(call cmd,ecoff)

targets += ../perkelez.bin

OBJCOPYFLAGS_perkelez.bin := $(OBJCOPYFLAGS) -O binary

$(objboot)/perkelez.bin: $(objtree)/perkelez
	$(call cmd,objcopy)

targets += ../perkelez.srec

OBJCOPYFLAGS_perkelez.srec := $(OBJCOPYFLAGS) -S -O srec

$(objboot)/perkelez.srec: $(objtree)/perkelez
	$(call cmd,objcopy)

targets += ../uzImage.bin

$(objboot)/uzImage.bin: $(objboot)/perkelez.bin FORCE
	$(call if_changed,uimage,none)

#
# Flattened Image Tree (.itb) image
#

ifeq ($(ADDR_BITS),32)
itb_addr_cells = 1
endif
ifeq ($(ADDR_BITS),64)
itb_addr_cells = 2
endif

targets += ../perkelez.its.S

quiet_cmd_its_cat = CAT     $@
      cmd_its_cat = cat $(real-prereqs) >$@

$(objboot)/perkelez.its.S: $(addprefix $(srctree)/arch/mips/$(PLATFORM)/,$(ITS_INPUTS)) FORCE
	$(call if_changed,its_cat)

targets += ../perkelez.its

quiet_cmd_cpp_its_S = ITS     $@
      cmd_cpp_its_S = $(CPP) -P -C -o $@ $< \
		        -DKERNEL_NAME="\"Linux $(KERNELRELEASE)\"" \
			-DVMLINUX_BINARY="\"$(2)\"" \
			-DVMLINUX_COMPRESSION="\"none\"" \
			-DVMLINUX_LOAD_ADDRESS=$(VMLINUZ_LOAD_ADDRESS) \
			-DVMLINUX_ENTRY_ADDRESS=$(VMLINUZ_LOAD_ADDRESS) \
			-DADDR_BITS=$(ADDR_BITS) \
			-DADDR_CELLS=$(itb_addr_cells)

$(objboot)/perkelez.its: $(objboot)/perkelez.its.S FORCE
	$(call if_changed,cpp_its_S,perkelez.bin)

targets += ../perkelez.itb

quiet_cmd_itb-image = ITB     $@
      cmd_itb-image = \
		env PATH="$(objtree)/scripts/dtc:$(PATH)" \
		$(BASH) $(MKIMAGE) \
		-D "-I dts -O dtb -p 500 \
			--include $(objtree)/arch/mips \
			--warning no-unit_address_vs_reg" \
		-f $(2) $@

$(objboot)/perkelez.itb: $(objboot)/perkelez.its $(objboot)/perkelez.bin FORCE
	$(call if_changed,itb-image,$<)

#
# This file is subject to the terms and conditions of the GNU General Public
# License.  See the file "COPYING" in the main directory of this archive
# for more details.
#
# Copyright (C) 1995, 1998, 2001, 2002 by Ralf Baechle
# Copyright (C) 2004  Maciej W. Rozycki
#

#
# Some DECstations need all possible sections of an ECOFF executable
#
ifdef CONFIG_MACH_DECSTATION
  e2eflag := -a
endif

#
# Drop some uninteresting sections in the kernel.
# This is only relevant for ELF kernels but doesn't hurt a.out
#
drop-sections := .reginfo .mdebug .comment .note .pdr .options .MIPS.options
strip-flags   := $(addprefix --remove-section=,$(drop-sections))

hostprogs := elf2ecoff

suffix-y			:= bin
suffix-$(CONFIG_KERNEL_BZIP2)	:= bz2
suffix-$(CONFIG_KERNEL_GZIP)	:= gz
suffix-$(CONFIG_KERNEL_LZMA)	:= lzma
suffix-$(CONFIG_KERNEL_LZO)	:= lzo

targets := perkelex.ecoff
quiet_cmd_ecoff = ECOFF	  $@
      cmd_ecoff = $(obj)/elf2ecoff $(VMLINUX) $@ $(e2eflag)
$(obj)/perkelex.ecoff: $(obj)/elf2ecoff $(VMLINUX) FORCE
	$(call if_changed,ecoff)

targets += perkelex.bin
quiet_cmd_bin = OBJCOPY $@
      cmd_bin = $(OBJCOPY) -O binary $(strip-flags) $(VMLINUX) $@
$(obj)/perkelex.bin: $(VMLINUX) FORCE
	$(call if_changed,bin)

targets += perkelex.srec
quiet_cmd_srec = OBJCOPY $@
      cmd_srec = $(OBJCOPY) -S -O srec $(strip-flags) $(VMLINUX) $@
$(obj)/perkelex.srec: $(VMLINUX) FORCE
	$(call if_changed,srec)

UIMAGE_LOADADDR  = $(VMLINUX_LOAD_ADDRESS)
UIMAGE_ENTRYADDR = $(VMLINUX_ENTRY_ADDRESS)

#
# Compressed perkelex images
#

extra-y += perkelex.bin.bz2
extra-y += perkelex.bin.gz
extra-y += perkelex.bin.lzma
extra-y += perkelex.bin.lzo

$(obj)/perkelex.bin.bz2: $(obj)/perkelex.bin FORCE
	$(call if_changed,bzip2)

$(obj)/perkelex.bin.gz: $(obj)/perkelex.bin FORCE
	$(call if_changed,gzip)

$(obj)/perkelex.bin.lzma: $(obj)/perkelex.bin FORCE
	$(call if_changed,lzma)

$(obj)/perkelex.bin.lzo: $(obj)/perkelex.bin FORCE
	$(call if_changed,lzo)

#
# Compressed u-boot images
#

targets += uImage
targets += uImage.bin
targets += uImage.bz2
targets += uImage.gz
targets += uImage.lzma
targets += uImage.lzo

$(obj)/uImage.bin: $(obj)/perkelex.bin FORCE
	$(call if_changed,uimage,none)

$(obj)/uImage.bz2: $(obj)/perkelex.bin.bz2 FORCE
	$(call if_changed,uimage,bzip2)

$(obj)/uImage.gz: $(obj)/perkelex.bin.gz FORCE
	$(call if_changed,uimage,gzip)

$(obj)/uImage.lzma: $(obj)/perkelex.bin.lzma FORCE
	$(call if_changed,uimage,lzma)

$(obj)/uImage.lzo: $(obj)/perkelex.bin.lzo FORCE
	$(call if_changed,uimage,lzo)

$(obj)/uImage: $(obj)/uImage.$(suffix-y)
	@ln -sf $(notdir $<) $@
	@echo '  Image $@ is ready'

#
# Flattened Image Tree (.itb) images
#

ifeq ($(ADDR_BITS),32)
itb_addr_cells = 1
endif
ifeq ($(ADDR_BITS),64)
itb_addr_cells = 2
endif

targets += perkelex.its.S

quiet_cmd_its_cat = CAT     $@
      cmd_its_cat = cat $(real-prereqs) >$@

$(obj)/perkelex.its.S: $(addprefix $(srctree)/arch/mips/$(PLATFORM)/,$(ITS_INPUTS)) FORCE
	$(call if_changed,its_cat)

targets += perkelex.its
targets += perkelex.gz.its
targets += perkelex.bz2.its
targets += perkelex.lzma.its
targets += perkelex.lzo.its

quiet_cmd_cpp_its_S = ITS     $@
      cmd_cpp_its_S = $(CPP) -P -C -o $@ $< \
		        -DKERNEL_NAME="\"Linux $(KERNELRELEASE)\"" \
			-DVMLINUX_BINARY="\"$(3)\"" \
			-DVMLINUX_COMPRESSION="\"$(2)\"" \
			-DVMLINUX_LOAD_ADDRESS=$(VMLINUX_LOAD_ADDRESS) \
			-DVMLINUX_ENTRY_ADDRESS=$(VMLINUX_ENTRY_ADDRESS) \
			-DADDR_BITS=$(ADDR_BITS) \
			-DADDR_CELLS=$(itb_addr_cells)

$(obj)/perkelex.its: $(obj)/perkelex.its.S $(VMLINUX) FORCE
	$(call if_changed,cpp_its_S,none,perkelex.bin)

$(obj)/perkelex.gz.its: $(obj)/perkelex.its.S $(VMLINUX) FORCE
	$(call if_changed,cpp_its_S,gzip,perkelex.bin.gz)

$(obj)/perkelex.bz2.its: $(obj)/perkelex.its.S $(VMLINUX)  FORCE
	$(call if_changed,cpp_its_S,bzip2,perkelex.bin.bz2)

$(obj)/perkelex.lzma.its: $(obj)/perkelex.its.S $(VMLINUX) FORCE
	$(call if_changed,cpp_its_S,lzma,perkelex.bin.lzma)

$(obj)/perkelex.lzo.its: $(obj)/perkelex.its.S $(VMLINUX) FORCE
	$(call if_changed,cpp_its_S,lzo,perkelex.bin.lzo)

targets += perkelex.itb
targets += perkelex.gz.itb
targets += perkelex.bz2.itb
targets += perkelex.lzma.itb
targets += perkelex.lzo.itb

quiet_cmd_itb-image = ITB     $@
      cmd_itb-image = \
		env PATH="$(objtree)/scripts/dtc:$(PATH)" \
		$(BASH) $(MKIMAGE) \
		-D "-I dts -O dtb -p 500 \
			--include $(objtree)/arch/mips \
			--warning no-unit_address_vs_reg" \
		-f $(2) $@

$(obj)/perkelex.itb: $(obj)/perkelex.its $(obj)/perkelex.bin FORCE
	$(call if_changed,itb-image,$<)

$(obj)/perkelex.%.itb: $(obj)/perkelex.%.its $(obj)/perkelex.bin.% FORCE
	$(call if_changed,itb-image,$<)

# for cleaning
subdir- += compressed tools

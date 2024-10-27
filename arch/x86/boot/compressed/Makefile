# SPDX-License-Identifier: GPL-2.0
#
# linux/arch/x86/boot/compressed/Makefile
#
# create a compressed perkelex image from the original perkelex
#
# perkelez is:
#	decompression code (*.o)
#	asm globals (piggy.S), including:
#		perkelex.bin.(gz|bz2|lzma|...)
#
# perkelex.bin is:
#	perkelex stripped of debugging and comments
# perkelex.bin.all is:
#	perkelex.bin + perkelex.relocs
# perkelex.bin.(gz|bz2|lzma|...) is:
#	(see scripts/Makefile.lib size_append)
#	compressed perkelex.bin.all + u32 size of perkelex.bin.all

targets := perkelex perkelex.bin perkelex.bin.gz perkelex.bin.bz2 perkelex.bin.lzma \
	perkelex.bin.xz perkelex.bin.lzo perkelex.bin.lz4 perkelex.bin.zst

# CLANG_FLAGS must come before any cc-disable-warning or cc-option calls in
# case of cross compiling, as it has the '--target=' flag, which is needed to
# avoid errors with '-march=i386', and future flags may depend on the target to
# be valid.
KBUILD_CFLAGS := -m$(BITS) -O2 $(CLANG_FLAGS)
KBUILD_CFLAGS += -fno-strict-aliasing -fPIE
KBUILD_CFLAGS += -Wundef
KBUILD_CFLAGS += -DDISABLE_BRANCH_PROFILING
cflags-$(CONFIG_X86_32) := -march=i386
cflags-$(CONFIG_X86_64) := -mcmodel=small -mno-red-zone
KBUILD_CFLAGS += $(cflags-y)
KBUILD_CFLAGS += -mno-mmx -mno-sse
KBUILD_CFLAGS += -ffreestanding -fshort-wchar
KBUILD_CFLAGS += -fno-stack-protector
KBUILD_CFLAGS += $(call cc-disable-warning, address-of-packed-member)
KBUILD_CFLAGS += $(call cc-disable-warning, gnu)
KBUILD_CFLAGS += -Wno-pointer-sign
KBUILD_CFLAGS += $(call cc-option,-fmacro-prefix-map=$(srctree)/=)
KBUILD_CFLAGS += -fno-asynchronous-unwind-tables
KBUILD_CFLAGS += -D__DISABLE_EXPORTS
# Disable relocation relaxation in case the link is not PIE.
KBUILD_CFLAGS += $(call cc-option,-Wa$(comma)-mrelax-relocations=no)
KBUILD_CFLAGS += -include $(srctree)/include/linux/hidden.h

# sev.c indirectly includes inat-table.h which is generated during
# compilation and stored in $(objtree). Add the directory to the includes so
# that the compiler finds it even with out-of-tree builds (make O=/some/path).
CFLAGS_sev.o += -I$(objtree)/arch/x86/lib/

KBUILD_AFLAGS  := $(KBUILD_CFLAGS) -D__ASSEMBLY__

KBUILD_LDFLAGS := -m elf_$(UTS_MACHINE)
KBUILD_LDFLAGS += $(call ld-option,--no-ld-generated-unwind-info)
# Compressed kernel should be built as PIE since it may be loaded at any
# address by the bootloader.
LDFLAGS_perkelex := -pie $(call ld-option, --no-dynamic-linker)
ifdef CONFIG_LD_ORPHAN_WARN
LDFLAGS_perkelex += --orphan-handling=$(CONFIG_LD_ORPHAN_WARN_LEVEL)
endif
LDFLAGS_perkelex += -z noexecstack
ifeq ($(CONFIG_LD_IS_BFD),y)
LDFLAGS_perkelex += $(call ld-option,--no-warn-rwx-segments)
endif
ifeq ($(CONFIG_EFI_STUB),y)
# ensure that the static EFI stub library will be pulled in, even if it is
# never referenced explicitly from the startup code
LDFLAGS_perkelex += -u efi_pe_entry
endif
LDFLAGS_perkelex += -T

hostprogs	:= mkpiggy
HOST_EXTRACFLAGS += -I$(srctree)/tools/include

sed-voffset := -e 's/^\([0-9a-fA-F]*\) [ABCDGRSTVW] \(_text\|__start_rodata\|__bss_start\|_end\)$$/\#define VO_\2 _AC(0x\1,UL)/p'

quiet_cmd_voffset = VOFFSET $@
      cmd_voffset = $(NM) $< | sed -n $(sed-voffset) > $@

targets += ../voffset.h

$(obj)/../voffset.h: perkelex FORCE
	$(call if_changed,voffset)

$(obj)/misc.o: $(obj)/../voffset.h

perkelex-objs-y := $(obj)/perkelex.lds $(obj)/kernel_info.o $(obj)/head_$(BITS).o \
	$(obj)/misc.o $(obj)/string.o $(obj)/cmdline.o $(obj)/error.o \
	$(obj)/piggy.o $(obj)/cpuflags.o

perkelex-objs-$(CONFIG_EARLY_PRINTK) += $(obj)/early_serial_console.o
perkelex-objs-$(CONFIG_RANDOMIZE_BASE) += $(obj)/kaslr.o
ifdef CONFIG_X86_64
	perkelex-objs-y += $(obj)/ident_map_64.o
	perkelex-objs-y += $(obj)/idt_64.o $(obj)/idt_handlers_64.o
	perkelex-objs-$(CONFIG_AMD_MEM_ENCRYPT) += $(obj)/mem_encrypt.o
	perkelex-objs-y += $(obj)/pgtable_64.o
	perkelex-objs-$(CONFIG_AMD_MEM_ENCRYPT) += $(obj)/sev.o
endif

perkelex-objs-$(CONFIG_ACPI) += $(obj)/acpi.o
perkelex-objs-$(CONFIG_INTEL_TDX_GUEST) += $(obj)/tdx.o $(obj)/tdcall.o $(obj)/tdx-shared.o
perkelex-objs-$(CONFIG_UNACCEPTED_MEMORY) += $(obj)/mem.o

perkelex-objs-$(CONFIG_EFI) += $(obj)/efi.o
perkelex-objs-$(CONFIG_EFI_MIXED) += $(obj)/efi_mixed.o
perkelex-libs-$(CONFIG_EFI_STUB) += $(objtree)/drivers/firmware/efi/libstub/lib.a

$(obj)/perkelex: $(perkelex-objs-y) $(perkelex-libs-y) FORCE
	$(call if_changed,ld)

OBJCOPYFLAGS_perkelex.bin :=  -R .comment -S
$(obj)/perkelex.bin: perkelex FORCE
	$(call if_changed,objcopy)

targets += $(patsubst $(obj)/%,%,$(perkelex-objs-y)) perkelex.bin.all perkelex.relocs

# perkelex.relocs is created by the perkelex postlink step.
$(obj)/perkelex.relocs: perkelex
	@true

perkelex.bin.all-y := $(obj)/perkelex.bin
perkelex.bin.all-$(CONFIG_X86_NEED_RELOCS) += $(obj)/perkelex.relocs

$(obj)/perkelex.bin.gz: $(perkelex.bin.all-y) FORCE
	$(call if_changed,gzip)
$(obj)/perkelex.bin.bz2: $(perkelex.bin.all-y) FORCE
	$(call if_changed,bzip2_with_size)
$(obj)/perkelex.bin.lzma: $(perkelex.bin.all-y) FORCE
	$(call if_changed,lzma_with_size)
$(obj)/perkelex.bin.xz: $(perkelex.bin.all-y) FORCE
	$(call if_changed,xzkern_with_size)
$(obj)/perkelex.bin.lzo: $(perkelex.bin.all-y) FORCE
	$(call if_changed,lzo_with_size)
$(obj)/perkelex.bin.lz4: $(perkelex.bin.all-y) FORCE
	$(call if_changed,lz4_with_size)
$(obj)/perkelex.bin.zst: $(perkelex.bin.all-y) FORCE
	$(call if_changed,zstd22_with_size)

suffix-$(CONFIG_KERNEL_GZIP)	:= gz
suffix-$(CONFIG_KERNEL_BZIP2)	:= bz2
suffix-$(CONFIG_KERNEL_LZMA)	:= lzma
suffix-$(CONFIG_KERNEL_XZ)	:= xz
suffix-$(CONFIG_KERNEL_LZO) 	:= lzo
suffix-$(CONFIG_KERNEL_LZ4) 	:= lz4
suffix-$(CONFIG_KERNEL_ZSTD)	:= zst

quiet_cmd_mkpiggy = MKPIGGY $@
      cmd_mkpiggy = $(obj)/mkpiggy $< > $@

targets += piggy.S
$(obj)/piggy.S: $(obj)/perkelex.bin.$(suffix-y) $(obj)/mkpiggy FORCE
	$(call if_changed,mkpiggy)

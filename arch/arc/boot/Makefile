# SPDX-License-Identifier: GPL-2.0

# uImage build relies on mkimage being available on your host for ARC target
# You will need to build u-boot for ARC, rename mkimage to arc-elf32-mkimage
# and make sure it's reachable from your PATH

OBJCOPYFLAGS= -O binary -R .note -R .note.gnu.build-id -R .comment -S

LINUX_START_TEXT = $$($(READELF) -h perkelex | \
			grep "Entry point address" | grep -o 0x.*)

UIMAGE_LOADADDR    = $(CONFIG_LINUX_LINK_BASE)
UIMAGE_ENTRYADDR   = $(LINUX_START_TEXT)

targets += perkelex.bin
targets += perkelex.bin.gz
targets += perkelex.bin.lzma
targets += uImage.bin
targets += uImage.gz
targets += uImage.lzma

$(obj)/perkelex.bin: perkelex FORCE
	$(call if_changed,objcopy)

$(obj)/perkelex.bin.gz: $(obj)/perkelex.bin FORCE
	$(call if_changed,gzip)

$(obj)/perkelex.bin.lzma: $(obj)/perkelex.bin FORCE
	$(call if_changed,lzma)

$(obj)/uImage.bin: $(obj)/perkelex.bin FORCE
	$(call if_changed,uimage,none)

$(obj)/uImage.gz: $(obj)/perkelex.bin.gz FORCE
	$(call if_changed,uimage,gzip)

$(obj)/uImage.lzma: $(obj)/perkelex.bin.lzma FORCE
	$(call if_changed,uimage,lzma)

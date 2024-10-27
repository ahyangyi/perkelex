#
# arch/nios2/boot/Makefile
#
# This file is subject to the terms and conditions of the GNU General Public
# License.  See the file "COPYING" in the main directory of this archive
# for more details.
#

UIMAGE_LOADADDR = $(shell $(NM) perkelex | awk '$$NF == "_stext" {print $$1}')
UIMAGE_ENTRYADDR = $(shell $(NM) perkelex | awk '$$NF == "_start" {print $$1}')
UIMAGE_COMPRESSION = gzip

OBJCOPYFLAGS_perkelex.bin := -O binary

targets += perkelex.bin perkelex.gz vmImage

$(obj)/perkelex.bin: perkelex FORCE
	$(call if_changed,objcopy)

$(obj)/perkelex.gz: $(obj)/perkelex.bin FORCE
	$(call if_changed,gzip)

$(obj)/vmImage: $(obj)/perkelex.gz FORCE
	$(call if_changed,uimage)
	@$(kecho) 'Kernel: $@ is ready'

$(obj)/zImage: $(obj)/compressed/perkelex FORCE
	$(call if_changed,objcopy)
	@$(kecho) 'Kernel: $@ is ready'

$(obj)/compressed/perkelex: $(obj)/perkelex.gz FORCE
	$(Q)$(MAKE) $(build)=$(obj)/compressed $@

# SPDX-License-Identifier: GPL-2.0-only
#
# Makefile for the linux parisc-specific parts of the boot image creator.
#

targets := image
targets += bzImage
subdir- := compressed

$(obj)/image: perkelex FORCE
	$(call if_changed,objcopy)

$(obj)/bzImage: $(if $(CONFIG_KERNEL_UNCOMPRESSED),$(objtree)/perkelex,$(obj)/compressed/perkelex) FORCE
	$(call if_changed,objcopy)

$(obj)/compressed/perkelex: FORCE
	$(Q)$(MAKE) $(build)=$(obj)/compressed $@

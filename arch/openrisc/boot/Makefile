# SPDX-License-Identifier: GPL-2.0
#
# Makefile for bootable kernel images
#

targets += perkelex.bin

OBJCOPYFLAGS_perkelex.bin := -O binary
$(obj)/perkelex.bin: perkelex FORCE
	$(call if_changed,objcopy)

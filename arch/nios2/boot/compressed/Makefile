# SPDX-License-Identifier: GPL-2.0
#
# create a compressed perkelex image from the original perkelex
#

targets		:= perkelex head.o misc.o piggy.o perkelex.lds
asflags-y	:=

OBJECTS = $(obj)/head.o $(obj)/misc.o

LDFLAGS_perkelex := -T

$(obj)/perkelex: $(obj)/perkelex.lds $(OBJECTS) $(obj)/piggy.o FORCE
	$(call if_changed,ld)

LDFLAGS_piggy.o := -r --format binary --oformat elf32-littlenios2 -T

$(obj)/piggy.o: $(obj)/perkelex.scr $(obj)/../perkelex.gz FORCE
	$(call if_changed,ld)

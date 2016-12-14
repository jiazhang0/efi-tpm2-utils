include $(TOPDIR)/version.mk

SUBDIRS := efi

.DEFAULT_GOAL := all
.PHONE: all clean install

all clean install:
	@for x in $(SUBDIRS); do $(MAKE) -C $$x $@ || exit $?; done
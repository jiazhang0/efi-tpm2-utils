include version.mk

TOPDIR := $(shell pwd)
export TOPDIR

SUBDIRS := src

.DEFAULT_GOAL := all
.PHONE: all clean install tag

all clean install:
	@for x in $(SUBDIRS); do $(MAKE) -C $$x $@ || exit $?; done

tag:
	@git tag -a $(ETET_VERSION) -m $(ETET_VERSION) refs/heads/master
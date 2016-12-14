CROSS_COMPILE ?=
CC := $(CROSS_COMPILE)gcc
LD := $(CROSS_COMPILE)ld
AR := $(CROSS_COMPILE)ar
OBJCOPY := $(CROSS_COMPILE)objcopy
NM := $(CROSS_COMPILE)nm
INSTALL ?= install

EXTRA_CFLAGS ?=
EXTRA_LDFLAGS ?=

DEBUG_BUILD ?=
DESTDIR ?=
# Installation location for pdet.efi
EFI_DESTDIR ?= /boot/efi
prefix ?= /usr
libdir ?= $(prefix)/lib64
bindir ?= $(prefix)/bin
sbindir ?= $(prefix)/sbin
includedir ?= $(prefix)/include

gnuefi_includedir ?= $(includedir)/efi $(includedir)/efi/$(EFI_ARCH)
gnuefi_libdir ?= $(libdir)

LDFLAGS := --warn-common --no-undefined --fatal-warnings \
	   $(patsubst $(join -Wl,,)%,%,$(EXTRA_LDFLAGS))
CFLAGS := -std=gnu11 -O2 -Wall -Werror \
	  $(addprefix -I, $(TOPDIR)/src/include) \
	  $(EXTRA_CFLAGS) $(addprefix $(join -Wl,,),$(LDFLAGS))

ifneq ($(DEBUG_BUILD),)
	CFLAGS += -ggdb -DDEBUG_BUILD
endif
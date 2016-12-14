include $(TOPDIR)/env.mk
include $(TOPDIR)/rules.mk

EFI_ARCH = $(shell $(CC) -dumpmachine | cut -f1 -d- | sed s,i[3456789]86,ia32,)

CFLAGS += -mno-mmx -mno-sse -maccumulate-outgoing-args \
	  -fpic -fno-stack-protector -fno-stack-check \
	  -fno-merge-constants -ffreestanding -fshort-wchar \
	  -fno-builtin -nostdinc \
	  $(addprefix -I, $(TOPDIR)/src/include $(gnuefi_includedir) \
	  $(shell $(CC) -print-file-name=include)) \
	  -DCONFIG_$(EFI_ARCH) -DGNU_EFI_USE_MS_ABI \
	  -DETET_VERSION=L\"$(ETET_VERSION)\"

ifeq ($(EFI_ARCH),x86_64)
    DEFAULT_ARCH = x64
    CFLAGS += -m64 -mno-red-zone -DDEFAULT_ARCH=\"$(DEFAULT_ARCH)\"
else ifeq ($(EFI_ARCH),ia32)
    DEFAULT_ARCH = ia32
    CFLAGS += -m32 -DDEFAULT_ARCH=\"$(DEFAULT_ARCH)\"
else
    $(error Unsupported EFI_ARCH $(EFI_ARCH) specified)
endif

LDFLAGS += -nostdlib -shared -Bsymbolic -L$(gnuefi_libdir) \
	   -T elf_$(EFI_ARCH)_efi.lds $(gnuefi_libdir)/crt0-efi-$(EFI_ARCH).o \
	   $(EXTRA_LDFLAGS)

# TODO: __attribute__((ms_abi)) is only available since gcc 4.7

TARGETS = fake-etet.efi
FAKE_ETET_OBJS = fake-etet.o

.PHONE: all clean install

all: $(TARGETS)
	@echo "EFI_ARCH: $(EFI_ARCH)"

clean:
	$(RM) $(TARGETS) *.so *.debug $(FAKE_ETET_OBJS)

install: fake-etet.efi
	cp -f fake-etet.efi $(EFI_DESTDIR)/fake-etet.efi

fake-etet.so: $(FAKE_ETET_OBJS)
	$(LD) $(LDFLAGS) -o $@ --start-group $^ $(shell $(CC) -print-libgcc-file-name) \
-lgnuefi -lefi --end-group
	@echo '--------------- List unresolved symbols ---------------'
	@! $(NM) $@ | grep -iw u
	@echo '-------------------------------------------------------'

%.efi: %.so
	$(OBJCOPY) -j .text -j .sdata -j .data \
		-j .dynamic -j .dynsym -j .rel* \
		-j .rela* -j .reloc -j .eh_frame \
		-j .debug_info -j .debug_abbrev -j .debug_aranges \
		-j .debug_line -j .debug_str -j .debug_ranges \
		-j .note.gnu.build-id \
		$^ $@.debug
	$(OBJCOPY) -j .text -j .sdata -j .data \
		-j .dynamic -j .dynsym -j .rel* \
		-j .rela* -j .reloc -j .eh_frame \
		--target efi-app-$(EFI_ARCH) $^ $@
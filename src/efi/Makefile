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
	  -DETU_VERSION=L\"$(ETU_VERSION)\"

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

LIB_NAME := libefitcg2

LIB_TARGET := $(LIB_NAME).a

OBJS_$(LIB_NAME) := \
	build_info.o \
	tcg2.o

TARGETS = fake-etet.efi tpm2-capability.efi
FAKE_ETET_OBJS = fake-etet.o
TPM2_CAPABILITY_OBJS = tpm2-capability.o

.PHONE: all clean install

all: $(TARGETS) Makefile
	@echo "EFI_ARCH: $(EFI_ARCH)"

clean:
	$(RM) $(TARGETS) *.so *.debug $(FAKE_ETET_OBJS) \
		$(TPM2_CAPABILITY_OBJS) build_info.c
	$(RM) $(LIB_TARGET) $(OBJS_$(LIB_NAME)) \
		$(addsuffix .*, $(LIB_TARGET))

install: fake-etet.efi tpm2-capability.efi
	cp -f $^ $(EFI_DESTDIR)

$(LIB_NAME).a: $(OBJS_$(LIB_NAME))
	$(AR) rcs $@ $^

fake-etet.so: $(FAKE_ETET_OBJS) $(LIB_TARGET)
tpm2-capability.so: $(TPM2_CAPABILITY_OBJS) $(LIB_TARGET)

%.so: %.o
	$(LD) $(LDFLAGS) -o $@ --start-group $^ $(shell $(CC) -print-libgcc-file-name) \
-lgnuefi -lefi $(LIB_TARGET) --end-group
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

build_info.c: build_info.c.in
	sed -e "s~@@GIT_COMMIT@@~$(shell if [ -d $(TOPDIR)/.git ]; then git log -1 --pretty=format:%H | tr -d '\n'; elif [ -f $(TOPDIR)/commit ]; then cat $(TOPDIR)/commit | tr -d '\n'; else echo -n ???????; fi)~" \
		-e "s~@@BUILD_MACHINE@@~$(shell whoami | tr -d '\n'; echo -n @; uname -a | tr -d '\n')~" < $^ > $@
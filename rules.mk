include $(TOPDIR)/version.mk

.DEFAULT_GOAL := all
.PHONE: all clean install

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
	$(OBJCOPY) -j .text -j .sdata -j .data -j .data.ident \
	    -j .dynamic -j .dynsym -j .rel* \
	    -j .rela* -j .reloc -j .eh_frame \
	    --target efi-app-$(EFI_ARCH) $^ $@

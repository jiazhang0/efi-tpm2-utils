/*
 * Fake ETET EFI application
 *
 * Copyright (c) 2016, Lans Zhang
 * All rights reserved.
 *
 * See "LICENSE" for license terms.
 *
 * Author:
 *      Lans Zhang <jia.zhang@windriver.com>
 */

#include <efi.h>
#include <efilib.h>

#include <etet.h>

static VOID *
FindETET(VOID)
{
	EFI_GUID Guid = EFI_TCG2_FINAL_EVENTS_TABLE_GUID;
	VOID *Table;
	EFI_STATUS Status;

	Status = LibGetSystemConfigurationTable(&Guid, &Table);
	if (EFI_ERROR(Status)) {
		Print(L"Unable to find ETET: %r\n", Status);
		return NULL;
	}

	return Table;
}

static EFI_STATUS
InstallETET(VOID)
{
	struct {
		EFI_TCG2_FINAL_EVENTS_TABLE v1;
	} __attribute__((packed)) Table = {
		.v1 = { 1, 0, },
	};

	UINTN NumberOfPages;
	EFI_PHYSICAL_ADDRESS BootMemoryAddress = 0;
	EFI_STATUS Status;
	
	NumberOfPages = (sizeof(Table) + EFI_PAGE_SIZE - 1) >> EFI_PAGE_SHIFT;
	Status = uefi_call_wrapper(BS->AllocatePages, 4, AllocateAnyPages,
				   EfiACPIMemoryNVS, NumberOfPages,
				   &BootMemoryAddress);
	if (EFI_ERROR(Status)) {
		Print(L"Unable to allocate memory for ETET table: %r\n",
		      Status);
		return Status;
	}

	CopyMem((VOID *)BootMemoryAddress, &Table, sizeof(Table));

	EFI_GUID Guid = EFI_TCG2_FINAL_EVENTS_TABLE_GUID;
	Status = uefi_call_wrapper(BS->InstallConfigurationTable, 2, &Guid,
				   (VOID *)BootMemoryAddress);
	if (EFI_ERROR(Status)) {
		Print(L"Unable to install ETET table: %r\n", Status);
		uefi_call_wrapper(BS->FreePages, 2,
				  BootMemoryAddress, NumberOfPages);
		return Status;
	}

	return EFI_SUCCESS;
}

EFI_STATUS
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *Systab)
{
	InitializeLib(ImageHandle, Systab);

	EFI_TCG2_FINAL_EVENTS_TABLE *Table;
	Table = (EFI_TCG2_FINAL_EVENTS_TABLE *)FindETET();
	if (Table) {
		UINT64 Version = Table->Version;

		if (!Version || Version >
				EFI_TCG2_FINAL_EVENTS_TABLE_VERSION) {
			Print(L"Unsupported ETET (version %llx)\n", Version);
			return EFI_UNSUPPORTED;
		}

		Print(L"ETET (version %lld, number of events %lld) detected\n",
		      Version, Table->NumberOfEvents);
		Print(L"Skip to install a fake ETET\n");

		return EFI_SUCCESS;
	}

	EFI_STATUS Status = InstallETET();
	if (!EFI_ERROR(Status))
		Print(L"Fake ETET installed\n");
	else
		Print(L"Unable to install the fake ETET: %r\n", Status);

	return Status;
}
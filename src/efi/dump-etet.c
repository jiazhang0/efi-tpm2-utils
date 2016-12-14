/*
 * Dump the content of ETET
 *
 * Copyright (c) 2016, Wind River Systems, Inc.
 * All rights reserved.
 *
 * See "LICENSE" for license terms.
 *
 * Author:
 *      Lans Zhang <jia.zhang@windriver.com>
 */

#include <efi.h>
#include <efilib.h>

#include <tcg2.h>
#include <etet.h>

static EFI_STATUS
DumpEtetV1(EFI_TCG2_FINAL_EVENTS_TABLE *Table)
{
	Print(L"ETET Structure Version 1\n");
	Print(L"  Number of Events: %lld\n", Table->NumberOfEvents);

	return EFI_SUCCESS;
}

EFI_STATUS
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *Systab)
{
	InitializeLib(ImageHandle, Systab);

	EFI_TCG2_FINAL_EVENTS_TABLE *Table;

	EFI_STATUS Status = EtetLocateAddress((VOID **)&Table);
	if (EFI_ERROR(Status))
		return Status;

	UINT64 Version = Table->Version;

	if (!Version || Version >
			EFI_TCG2_FINAL_EVENTS_TABLE_VERSION) {
		Print(L"Unsupported ETET (version %llx)\n", Version);
		return EFI_UNSUPPORTED;
	}

	if (Version == EFI_TCG2_FINAL_EVENTS_TABLE_VERSION)
		DumpEtetV1(Table);

	return EFI_SUCCESS;
}
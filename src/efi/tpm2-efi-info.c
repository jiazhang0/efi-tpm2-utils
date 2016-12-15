/*
 * Detect TPM device status 
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

STATIC VOID
DetectETET(VOID)
{
	VOID *Table;
	EFI_STATUS Status = EtetLocateAddress(&Table);

	if (!EFI_ERROR(Status))
		Print(L"ETET is detected\n");
}

EFI_STATUS
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *Systab)
{
	EFI_STATUS Status;

	InitializeLib(ImageHandle, Systab);

	EFI_TCG2_PROTOCOL *Tcg2;
	Status = Tcg2LocateProtocol(&Tcg2);
	if (EFI_ERROR(Status))
		goto next;

	Print(L"EFI TCG2 protocol is installed\n");

	UINT8 TpmCapabilitySize = 0;
	Status = Tcg2GetCapability(NULL, &TpmCapabilitySize);
	if (EFI_ERROR(Status))
		return Status;

	EFI_TCG2_BOOT_SERVICE_CAPABILITY *TpmCapability;
	TpmCapability = AllocatePool(TpmCapabilitySize);
	if (!TpmCapability)
		return Status;

	Status = Tcg2GetCapability(TpmCapability, &TpmCapabilitySize);
	if (EFI_ERROR(Status)) {
		FreePool(TpmCapability);
		return Status;
	}

	if (!EFI_ERROR(Status)) {
		UINT8 Major = TpmCapability->StructureVersion.Major;
		UINT8 Minor = TpmCapability->StructureVersion.Minor;

		if (Major == 1 && Minor == 0) {
			TREE_BOOT_SERVICE_CAPABILITY *TrEECapability;

			TrEECapability = (TREE_BOOT_SERVICE_CAPABILITY *)TpmCapability;
			if (TrEECapability->TrEEPresentFlag)
				Print(L"TPM 2.0 (TrEE) device is present\n");
			else
				Print(L"TPM 2.0 (TrEE) device is absent\n");
		} else if (Major == 1 && Minor == 1) {
			if (TpmCapability->TPMPresentFlag)
				Print(L"TPM 2.0 device is present\n");
			else
				Print(L"TPM 2.0 device is absent\n");
		} else
			Print(L"Unsupported capability structure version: "
			      L"%d.%d\n", Major, Minor);
	}

next:
	DetectETET();

	return EFI_SUCCESS;
}
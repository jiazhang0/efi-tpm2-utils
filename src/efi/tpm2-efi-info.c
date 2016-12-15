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

EFI_STATUS
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *Systab)
{
	InitializeLib(ImageHandle, Systab);

	EFI_TCG2_PROTOCOL *Tcg2;
	Tcg2LocateProtocol(&Tcg2);
#if 0
	EFI_TCG2_BOOT_SERVICE_CAPABILITY TpmCapability;
	UINT8 TpmCapabilitySize = sizeof(TpmCapability);
	EFI_STATUS Status;

	Status = Tcg2GetCapability(&TpmCapability,
				   &TpmCapabilitySize);
#else
	UINT8 TpmCapabilitySize = 0;
	EFI_STATUS Status;

	Status = Tcg2GetCapability(NULL, &TpmCapabilitySize);
	if (EFI_ERROR(Status))
		return Status;

	EFI_TCG2_BOOT_SERVICE_CAPABILITY *TpmCapability;
	TpmCapability = AllocatePool(TpmCapabilitySize);
	if (!TpmCapability)
		return Status;

	Status = Tcg2GetCapability(TpmCapability, &TpmCapabilitySize);
	if (EFI_ERROR(Status))
		return Status;	
#endif
	if (!EFI_ERROR(Status)) {
		UINT8 Major = TpmCapability->StructureVersion.Major;
		UINT8 Minor = TpmCapability->StructureVersion.Minor;

		if (Major == 1 && Minor == 0) {
			EFI_TCG_BOOT_SERVICE_CAPABILITY *Tpm1Capability;

			Tpm1Capability = (EFI_TCG_BOOT_SERVICE_CAPABILITY *)TpmCapability;
			if (Tpm1Capability->TPMPresentFlag) {
				Print(L"TPM 1.x device is present and ");

				if (Tpm1Capability->TPMDeactivatedFlag)
					Print(L"deactivated\n");
				else
					Print(L"activated\n");
			} else
				Print(L"TPM 1.x device is absent\n");			
		} else if (Major == 1 && Minor == 1) {
			if (TpmCapability->TPMPresentFlag)
				Print(L"TPM 2.0 device is present\n");
			else
				Print(L"TPM 2.0 device is absent\n");
		} else
			Print(L"Unsupported capability structure version: "
			      L"%d.%d\n", Major, Minor);
	}

	VOID *Table;
	Status = EtetLocateAddress(&Table);
	if (!EFI_ERROR(Status))
		Print(L"ETET is detected\n");

	return EFI_SUCCESS;
}
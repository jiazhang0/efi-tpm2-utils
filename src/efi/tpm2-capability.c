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

#include <tcg2.h>

static EFI_STATUS
DetectTPM(EFI_TCG2_PROTOCOL *Tcg2)
{
	EFI_TCG2_BOOT_SERVICE_CAPABILITY TpmCapability;
	EFI_STATUS Status;

	TpmCapability.Size = (UINT8)sizeof(TpmCapability);
	Status = uefi_call_wrapper(Tcg2->GetCapability, 2, Tcg2,
				   &TpmCapability);
	if (EFI_ERROR(Status)) {
		Print(L"Unable to get the TPM capability: %r\n",
		      Status);
		return Status;
	}

	if (TpmCapability.StructureVersion.Major == 1 &&
			TpmCapability.StructureVersion.Minor == 0) {
		EFI_TCG2_BOOT_SERVICE_CAPABILITY_1_X *Tpm1Capability;

		Tpm1Capability = (EFI_TCG2_BOOT_SERVICE_CAPABILITY_1_X *)&TpmCapability;
		if (Tpm1Capability->TPMPresentFlag) {
			Print(L"TPM 1.x device detected\n");
			return EFI_SUCCESS;
		}
	} else {
		if (TpmCapability.TPMPresentFlag) {
			Print(L"TPM 2.0 device detected\n");
			return EFI_SUCCESS;
		}
	}

	Print(L"No TPM device detected\n");

	return EFI_NOT_FOUND;
}

EFI_STATUS
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *Systab)
{
	InitializeLib(ImageHandle, Systab);

	EFI_STATUS Status;
	EFI_TCG2_PROTOCOL *Tcg2;
	EFI_GUID Guid = EFI_TCG2_PROTOCOL_GUID;

	/* Try to get efi tpm2 protocol */
	Status = LibLocateProtocol(&Guid, (VOID **)&Tcg2);
	if (EFI_ERROR(Status)) {
		Print(L"Unable to locate EFI TPM2 Protocol: %r\n",
		      Status);
		return Status;
	}

	Print(L"EFI TPM2 Protocol already installed\n");

	return DetectTPM(Tcg2);
}
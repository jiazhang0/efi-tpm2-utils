/*
 * EFI TCG2 common functions
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

EFI_STATUS
Tcg2LocateProtocol(EFI_TCG2_PROTOCOL **Tcg2)
{
	EFI_GUID Guid = EFI_TCG2_PROTOCOL_GUID;
	EFI_STATUS Status;

	/* Try to get efi tpm2 protocol */
	Status = LibLocateProtocol(&Guid, (VOID **)Tcg2);
	if (EFI_ERROR(Status)) {
		Print(L"Unable to locate EFI TCG2 Protocol: %r\n", Status);
		return Status;
	}

	return Status;
}

EFI_STATUS
Tcg2GetCapability(EFI_TCG2_BOOT_SERVICE_CAPABILITY *Capability,
		  UINT8 *Size)
{
	EFI_TCG2_PROTOCOL *Tcg2;
	EFI_STATUS Status;

	Status = Tcg2LocateProtocol(&Tcg2);
	if (EFI_ERROR(Status)) {
		Print(L"Unable to locate EFI TCG2 Protocol: %r\n", Status);
		return Status;
	}

	EFI_TCG2_BOOT_SERVICE_CAPABILITY TpmCapability;

	if (!*Size) {
		TpmCapability.Size = (UINT8)sizeof(TpmCapability);
		Capability = &TpmCapability;
	}

	Status = uefi_call_wrapper(Tcg2->GetCapability, 2, Tcg2,
				   Capability);
	if (EFI_ERROR(Status)) {
		Print(L"Unable to get the TPM capability: %r\n", Status);
		return Status;
	}

	if (!*Size)
		*Size = TpmCapability.Size;

	return Status;
}

EFI_STATUS
Tcg2DetectTPM(VOID)
{
	EFI_TCG2_BOOT_SERVICE_CAPABILITY TpmCapability;
	UINTN TpmCapabilitySize = sizeof(TpmCapability);
	EFI_STATUS Status;

	Status = Tcg2GetCapability(&TpmCapability,
				   (UINT8 *)&TpmCapabilitySize);
	if (EFI_ERROR(Status))
		return Status;

	if (TpmCapability.StructureVersion.Major == 1 &&
			TpmCapability.StructureVersion.Minor == 0) {
		EFI_TCG_BOOT_SERVICE_CAPABILITY *Tpm1Capability;

		Tpm1Capability = (EFI_TCG_BOOT_SERVICE_CAPABILITY *)&TpmCapability;
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
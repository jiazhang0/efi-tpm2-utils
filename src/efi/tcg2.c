/*
 * EFI TCG2 common functions
 *
 * Copyright (c) 2016, Wind River Systems, Inc.
 * All rights reserved.
 *
 * See "LICENSE" for license terms.
 *
 * Author:
 *      Jia Zhang <zhang.jia@linux.alibaba.com>
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
		  UINT8 *CapabilitySize)
{
	EFI_TCG2_PROTOCOL *Tcg2;
	EFI_STATUS Status;

	Status = Tcg2LocateProtocol(&Tcg2);
	if (EFI_ERROR(Status)) {
		Print(L"Unable to locate EFI TCG2 Protocol: %r\n", Status);
		return Status;
	}

	EFI_TCG2_BOOT_SERVICE_CAPABILITY TpmCapability;

	if (!*CapabilitySize)
		TpmCapability.Size = sizeof(TpmCapability);
	else
		TpmCapability.Size = *CapabilitySize;

	Status = uefi_call_wrapper(Tcg2->GetCapability, 2, Tcg2,
				   &TpmCapability);
	if (EFI_ERROR(Status)) {
		Print(L"Unable to get the TPM capability: %r\n", Status);
		if (Status == EFI_BUFFER_TOO_SMALL)
			Print(L"Only %d-byte is filled in the capability "
			      L"structure\n", TpmCapability.Size);
		return Status;
	}

	if (!*CapabilitySize)
		*CapabilitySize = TpmCapability.Size;
	else if (Capability) {
		UINTN CopySize = TpmCapability.Size;

		if (*CapabilitySize < CopySize)
			CopySize = *CapabilitySize;

		CopyMem(Capability, &TpmCapability, CopySize);
	}

	return Status;
}
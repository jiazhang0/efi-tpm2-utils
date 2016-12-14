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
/*
 * ETET common functions
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

#include <etet.h>

EFI_STATUS
EtetLocateAddress(VOID **Table)
{
	EFI_GUID Guid = EFI_TCG2_FINAL_EVENTS_TABLE_GUID;
	EFI_STATUS Status;

	Status = LibGetSystemConfigurationTable(&Guid, Table);
	if (EFI_ERROR(Status)) {
		Print(L"Unable to find ETET: %r\n", Status);
		return Status;
	}

	return Status;
}
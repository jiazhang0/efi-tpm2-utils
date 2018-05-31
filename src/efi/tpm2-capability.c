/*
 * Show the TPM capability information
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
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *Systab)
{
	InitializeLib(ImageHandle, Systab);

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
		goto out;

	if (TpmCapability->StructureVersion.Major == 1 &&
			TpmCapability->StructureVersion.Minor == 0) {
		TREE_BOOT_SERVICE_CAPABILITY *TrEECapability;

		TrEECapability = (TREE_BOOT_SERVICE_CAPABILITY *)TpmCapability;

		Print(L"Structure Size: %d-byte\n",
		      (UINT8)TrEECapability->Size);

		Print(L"Structure Version: %d.%d\n",
		      (UINT8)TrEECapability->StructureVersion.Major,
		      (UINT8)TrEECapability->StructureVersion.Minor);

		Print(L"Protocol Version: %d.%d\n",
		      (UINT8)TrEECapability->ProtocolVersion.Major,
		      (UINT8)TrEECapability->ProtocolVersion.Minor);

		UINT32 Hash = TrEECapability->HashAlgorithmBitmap;
		Print(L"Supported Hash Algorithm: 0x%x (%s%s%s%s%s)\n",
		      Hash,
		      Hash & TREE_BOOT_HASH_ALG_SHA1 ? L"SHA-1" : L"",
		      Hash & TREE_BOOT_HASH_ALG_SHA256 ? L" SHA-256" : L"",
		      Hash & TREE_BOOT_HASH_ALG_SHA384 ? L" SHA-384" : L"",
		      Hash & TREE_BOOT_HASH_ALG_SHA512 ? L" SHA-512" : L"",
		      (Hash & ~TREE_BOOT_HASH_ALG_MASK) || !Hash ?
			L" dirty" : L"");

		EFI_TCG2_EVENT_LOG_BITMAP Format = TrEECapability->SupportedEventLogs;
		Print(L"Supported Event Log Format: 0x%x (%s%s%s)\n", Format,
		      Format & TREE_EVENT_LOG_FORMAT_TCG_1_2 ?
		      L"TCG1.2" : L"",
		      (Format & ~TREE_EVENT_LOG_FORMAT_MASK) || !Format ?
		      L" dirty" : L"");

		Print(L"TrEE Present: %s\n",
		      TrEECapability->TrEEPresentFlag ?
		      L"True" : L"False");

		Print(L"Max Command Size: %d-byte\n",
		      TrEECapability->MaxCommandSize);

		Print(L"Max Response Size: %d-byte\n",
		      TrEECapability->MaxResponseSize);

		Print(L"Manufacturer ID: 0x%x\n",
		      TrEECapability->ManufacturerID);
	} else if (TpmCapability->StructureVersion.Major == 1 &&
			TpmCapability->StructureVersion.Minor == 1) {
		Print(L"Structure Size: %d-byte\n",
		      TpmCapability->Size);

		Print(L"Structure Version: %d.%d\n",
		      TpmCapability->StructureVersion.Major,
		      TpmCapability->StructureVersion.Minor);

		Print(L"Protocol Version: %d.%d\n",
		      TpmCapability->ProtocolVersion.Major,
		      TpmCapability->ProtocolVersion.Minor);

		UINT8 Hash = TpmCapability->HashAlgorithmBitmap;
		Print(L"Supported Hash Algorithm: 0x%x (%s%s%s%s%s%s)\n",
		      Hash,
		      Hash & EFI_TCG2_BOOT_HASH_ALG_SHA1 ? L"SHA-1" : L"",
		      Hash & EFI_TCG2_BOOT_HASH_ALG_SHA256 ? L" SHA-256" : L"",
		      Hash & EFI_TCG2_BOOT_HASH_ALG_SHA384 ? L" SHA-384" : L"",
		      Hash & EFI_TCG2_BOOT_HASH_ALG_SHA512 ? L" SHA-512" : L"",
		      Hash & EFI_TCG2_BOOT_HASH_ALG_SM3_256 ? L" SM3-256" : L"",
		      (Hash & ~EFI_TCG2_BOOT_HASH_ALG_MASK) || !Hash ?
		      L" dirty" : L"");

		EFI_TCG2_EVENT_LOG_BITMAP Format = TpmCapability->SupportedEventLogs;
		Print(L"Supported Event Log Format: 0x%x (%s%s%s)\n", Format,
		      Format & EFI_TCG2_EVENT_LOG_FORMAT_TCG_1_2 ?
		      L"TCG1.2" : L"",
		      Format & EFI_TCG2_EVENT_LOG_FORMAT_TCG_2 ?
		      L" TCG2.0" : L"",
		      (Format & ~EFI_TCG2_EVENT_LOG_FORMAT_MASK) || !Format ?
		      L" dirty" : L"");

		Print(L"TPM Present: %s\n",
		      TpmCapability->TPMPresentFlag ?
		      L"True" : L"False");

		Print(L"Max Command Size: %d-byte\n",
		      TpmCapability->MaxCommandSize);

		Print(L"Max Response Size: %d-byte\n",
		      TpmCapability->MaxResponseSize);

		Print(L"Manufacturer ID: 0x%x\n",
		      TpmCapability->ManufacturerID);

		Print(L"Number of PCR Banks: %d%s\n",
		      TpmCapability->NumberOfPcrBanks,
		      !TpmCapability->NumberOfPcrBanks ? L"(dirty)" : L"");

		EFI_TCG2_EVENT_ALGORITHM_BITMAP Bank = TpmCapability->ActivePcrBanks;
		Print(L"Active PCR Banks: 0x%x (%s%s%s%s%s%s)\n",
		      Bank,
		      Bank & EFI_TCG2_BOOT_HASH_ALG_SHA1 ? L"SHA-1" : L"",
		      Bank & EFI_TCG2_BOOT_HASH_ALG_SHA256 ? L" SHA-256" : L"",
		      Bank & EFI_TCG2_BOOT_HASH_ALG_SHA384 ? L" SHA-384" : L"",
		      Bank & EFI_TCG2_BOOT_HASH_ALG_SHA512 ? L" SHA-512" : L"",
		      Bank & EFI_TCG2_BOOT_HASH_ALG_SM3_256 ? L" SM3-256" : L"",
		      (Bank & ~EFI_TCG2_BOOT_HASH_ALG_MASK) || !Bank ?
		      L" dirty" : L"");
	} else {
		Print(L"Unsupported structure version: %d.%d\n",
		      TpmCapability->StructureVersion.Major,
		      TpmCapability->StructureVersion.Minor);

		Status = EFI_UNSUPPORTED;
	}

out:
	FreePool(TpmCapability);

	return Status;
}
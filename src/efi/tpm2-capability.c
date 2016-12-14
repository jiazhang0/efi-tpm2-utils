/*
 * Show the TPM capability information
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

EFI_STATUS
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *Systab)
{
	InitializeLib(ImageHandle, Systab);

	UINTN TpmCapabilitySize = 0;
	EFI_STATUS Status;

	Status = Tcg2GetCapability(NULL, (UINT8 *)&TpmCapabilitySize);
	if (EFI_ERROR(Status))
		return Status;

	EFI_TCG2_BOOT_SERVICE_CAPABILITY *TpmCapability;
	TpmCapability = AllocatePool(TpmCapabilitySize);
	if (!TpmCapability)
		return Status;

	Status = Tcg2GetCapability(TpmCapability, (UINT8 *)&TpmCapabilitySize);
	if (EFI_ERROR(Status))
		goto out;

	if (TpmCapability->StructureVersion.Major == 1 &&
			TpmCapability->StructureVersion.Minor == 0) {
		EFI_TCG_BOOT_SERVICE_CAPABILITY *Tpm1Capability;

		Tpm1Capability = (EFI_TCG_BOOT_SERVICE_CAPABILITY *)&TpmCapability;

		Print(L"Structure Size: %d-byte\n",
		      (UINT8)Tpm1Capability->Size);

		Print(L"Structure Version: %d.%d (Rev %d.%d)\n",
		      (UINT8)Tpm1Capability->StructureVersion.Major,
		      (UINT8)Tpm1Capability->StructureVersion.Minor,
		      (UINT8)Tpm1Capability->StructureVersion.RevMajor,
		      (UINT8)Tpm1Capability->StructureVersion.RevMinor);

		Print(L"Protocol Version: %d.%d (Rev %d.%d)\n",
		      (UINT8)Tpm1Capability->ProtocolVersion.Major,
		      (UINT8)Tpm1Capability->StructureVersion.Minor,
		      (UINT8)Tpm1Capability->ProtocolVersion.RevMajor,
		      (UINT8)Tpm1Capability->StructureVersion.RevMinor);

		UINT8 Hash = (UINT8)Tpm1Capability->HashAlgorithmBitmap;
		Print(L"Hash Algorithm Bitmap: 0x%x (%s%s)\n", Hash,
		      Hash & EFI_TCG_BOOT_HASH_ALG_SHA1 ? L"SHA-1" : L"N/A",
		      Hash != EFI_TCG_BOOT_HASH_ALG_SHA1 ? L", dirty" : L"");

		Print(L"TPM Present: %s\n",
		      (BOOLEAN)Tpm1Capability->TPMPresentFlag ?
			L"True" : L"False");

		Print(L"TPM Deactivated: %s\n",
		      (BOOLEAN)Tpm1Capability->TPMDeactivatedFlag ?
			L"True" : L"False");
	} else if (TpmCapability->StructureVersion.Major == 1 &&
			TpmCapability->StructureVersion.Minor == 1) {
		Print(L"Structure Size: %d-byte\n",
		      (UINT8)TpmCapability->Size);

		Print(L"Structure Version: %d.%d\n",
		      (UINT8)TpmCapability->StructureVersion.Major,
		      (UINT8)TpmCapability->StructureVersion.Minor);

		Print(L"Protocol Version: %d.%d\n",
		      (UINT8)TpmCapability->ProtocolVersion.Major,
		      (UINT8)TpmCapability->StructureVersion.Minor);

		UINT8 Hash = (UINT8)TpmCapability->HashAlgorithmBitmap;
		Print(L"Supported Hash Algorithm Bitmap: 0x%x (%s%s%s%s%s%s)\n",
		      Hash,
		      Hash & EFI_TCG2_BOOT_HASH_ALG_SHA1 ? L"SHA-1" : L"",
		      Hash & EFI_TCG2_BOOT_HASH_ALG_SHA256 ? L"SHA-256" : L"N/A",
		      Hash & EFI_TCG2_BOOT_HASH_ALG_SHA384 ? L"SHA-384" : L"",
		      Hash & EFI_TCG2_BOOT_HASH_ALG_SHA512 ? L"SHA-512" : L"N/A",
		      Hash & EFI_TCG2_BOOT_HASH_ALG_SM3_256 ? L"SM3-256" : L"N/A",
		      (Hash & ~EFI_TCG2_BOOT_HASH_ALG_MASK) || !Hash ?
			L", dirty" : L"");

		EFI_TCG2_EVENT_LOG_BITMAP Format = TpmCapability->SupportedEventLogs;
		Print(L"Supported Event Log Format: 0x%x (%s%s%s)\n", Format,
		      Format & EFI_TCG2_EVENT_LOG_FORMAT_TCG_1_2
			? L"TCG1.2" : L"",
		      Format & EFI_TCG2_EVENT_LOG_FORMAT_TCG_2
			? L"TCG2.0" : L"",
		      (Format & ~EFI_TCG2_EVENT_LOG_FORMAT_MASK) || !Format
			? L", dirty" : L"");

		Print(L"TPM Present: %s\n",
		      (BOOLEAN)TpmCapability->TPMPresentFlag ?
			L"True" : L"False");

		Print(L"Max Command Size: %d-byte\n",
		      (UINT16)TpmCapability->MaxCommandSize);

		Print(L"Max Response Size: %d-byte\n",
		      (UINT16)TpmCapability->MaxResponseSize);

		Print(L"Manufacturer ID: 0x%x\n",
		      TpmCapability->ManufacturerID);

		Print(L"Number of PCR Banks: %d%s\n",
		      TpmCapability->NumberOfPcrBanks,
		      !TpmCapability->NumberOfPcrBanks ? L"(dirty)" : L"");

		EFI_TCG2_EVENT_ALGORITHM_BITMAP Bank = TpmCapability->ActivePcrBanks;
		Print(L"Bitmap of Active PCR Banks: 0x%x (%s%s%s%s%s%s)\n",
		      Bank,
		      Bank & EFI_TCG2_BOOT_HASH_ALG_SHA1 ? L"SHA-1" : L"",
		      Bank & EFI_TCG2_BOOT_HASH_ALG_SHA256 ? L"SHA-256" : L"N/A",
		      Bank & EFI_TCG2_BOOT_HASH_ALG_SHA384 ? L"SHA-384" : L"",
		      Bank & EFI_TCG2_BOOT_HASH_ALG_SHA512 ? L"SHA-512" : L"N/A",
		      Bank & EFI_TCG2_BOOT_HASH_ALG_SM3_256 ? L"SM3-256" : L"N/A",
		      (Bank & ~EFI_TCG2_BOOT_HASH_ALG_MASK) || !Bank ?
			L", dirty" : L"");
	} else {
		Print(L"Unsupported structure version: %d.%d\n",
		      (UINT8)TpmCapability->StructureVersion.Major,
		      (UINT8)TpmCapability->StructureVersion.Minor);

		Status = EFI_UNSUPPORTED;
	}

out:
	FreePool(TpmCapability);

	return Status;
}
/*
 * Show the event logs
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

#include <etet.h>

STATIC EFI_TCG2_BOOT_SERVICE_CAPABILITY *
GetCapability(VOID)
{
	UINTN TpmCapabilitySize = 0;
	EFI_STATUS Status;

	Status = Tcg2GetCapability(NULL, (UINT8 *)&TpmCapabilitySize);
	if (EFI_ERROR(Status))
		return NULL;

	EFI_TCG2_BOOT_SERVICE_CAPABILITY *TpmCapability;
	TpmCapability = AllocatePool(TpmCapabilitySize);
	if (!TpmCapability)
		return NULL;

	Status = Tcg2GetCapability(TpmCapability, (UINT8 *)&TpmCapabilitySize);
	if (EFI_ERROR(Status)) {
		FreePool(TpmCapability);
		return NULL;
	}

	return TpmCapability;
}

STATIC EFI_TCG2_EVENT_ALGORITHM_BITMAP
GetActivePcrBanks(EFI_TCG2_BOOT_SERVICE_CAPABILITY *TpmCapability)
{
	EFI_TCG2_EVENT_ALGORITHM_BITMAP Banks = 0;

	if (TpmCapability->StructureVersion.Major == 1 &&
			TpmCapability->StructureVersion.Minor == 0) {
		TREE_BOOT_SERVICE_CAPABILITY *TrEECapability;

		TrEECapability = (TREE_BOOT_SERVICE_CAPABILITY *)&TpmCapability;
		Banks = TrEECapability->HashAlgorithmBitmap & TREE_BOOT_HASH_ALG_MASK;
	} else if (TpmCapability->StructureVersion.Major == 1 &&
			TpmCapability->StructureVersion.Minor == 1)
		Banks = TpmCapability->ActivePcrBanks & EFI_TCG2_BOOT_HASH_ALG_MASK;
	else
		Print(L"Unsupported structure version: %d.%d\n",
		      (UINT8)TpmCapability->StructureVersion.Major,
		      (UINT8)TpmCapability->StructureVersion.Minor);

	return Banks;
}

STATIC EFI_TCG2_EVENT_LOG_BITMAP
GetSupportedEventLogFormat(EFI_TCG2_BOOT_SERVICE_CAPABILITY *TpmCapability)
{
	EFI_TCG2_EVENT_LOG_BITMAP Format = 0;

	if (TpmCapability->StructureVersion.Major == 1 &&
			TpmCapability->StructureVersion.Minor == 0) {
		TREE_BOOT_SERVICE_CAPABILITY *TrEECapability;

		TrEECapability = (TREE_BOOT_SERVICE_CAPABILITY *)TpmCapability;
		Format = TrEECapability->SupportedEventLogs & TREE_BOOT_HASH_ALG_MASK;
	} else if (TpmCapability->StructureVersion.Major == 1 &&
			TpmCapability->StructureVersion.Minor == 1)
		Format = TpmCapability->SupportedEventLogs & EFI_TCG2_EVENT_LOG_FORMAT_MASK;
	else
		Print(L"Unsupported structure version: %d.%d\n",
		      (UINT8)TpmCapability->StructureVersion.Major,
		      (UINT8)TpmCapability->StructureVersion.Minor);

	return Format;
}

STATIC CHAR16 *
GetEventTypeName(TCG_EVENTTYPE EventType)
{
	CHAR16 *Name;

	switch (EventType) {
	case EV_POST_CODE:
		Name = L"Post code";
		break;
	case EV_NO_ACTION:
		Name = L"No action";
		break;
	case EV_SEPARATOR:
		Name = L"Separator";
		break;
	case EV_S_CRTM_CONTENTS:
		Name = L"S-CRTM contents";
		break;
	case EV_S_CRTM_VERSION:
		Name = L"S-CRTM version";
		break;
	case EV_CPU_MICROCODE:
		Name = L"CPU microcode";
		break;
	case EV_TABLE_OF_DEVICES:
		Name = L"Table of devices";
		break;
	case EV_EFI_EVENT_BASE:
		Name = L"EFI event base";
		break;
	case EV_EFI_VARIABLE_DRIVER_CONFIG:
		Name = L"EFI variable driver config";
		break;
	case EV_EFI_VARIABLE_BOOT:
		Name = L"EFI variable boot";
		break;
	case EV_EFI_BOOT_SERVICES_APPLICATION:
		Name = L"EFI boot services application";
		break;
	case EV_EFI_BOOT_SERVICES_DRIVER:
		Name = L"EFI boot services driver";
		break;
	case EV_EFI_RUNTIME_SERVICES_DRIVER:
		Name = L"EFI runtime services driver";
		break;
	case EV_EFI_GPT_EVENT:
		Name = L"EFI GPT event";
		break;
	case EV_EFI_ACTION:
		Name = L"EFI action";
		break;
	case EV_EFI_PLATFORM_FIRMWARE_BLOB:
		Name = L"EFI platform firmware blob";
		break;
	case EV_EFI_HANDOFF_TABLES:
		Name = L"EFI handoff tables";
		break;
	case EV_EFI_VARIABLE_AUTHORITY:
		Name = L"EFI variable authority";
		break;
	default:
		Name = L"Unknown";
	}

	return Name;
}

STATIC VOID
ShowString(CHAR16 *String, UINT32 StringLength)
{
	CHAR16 *Buffer = AllocatePool((StringLength + 1) * sizeof(CHAR16));

	if (!Buffer)
		return;

	CopyMem(Buffer, String, StringLength * sizeof(CHAR16));
	Buffer[StringLength] = 0;

	Print(Buffer);

	FreePool(Buffer);
}

STATIC VOID
ShowAsciiString(CHAR8 *String, UINT32 StringLength)
{
	CHAR8 *Buffer = AllocatePool((StringLength + 1) * sizeof(CHAR8));

	if (!Buffer)
		return;

	CopyMem(Buffer, String, StringLength * sizeof(CHAR8));
	Buffer[StringLength] = 0;

	Print(L"%a", Buffer);

	FreePool(Buffer);
}

STATIC VOID
ShowHexString(UINT8 *Binary, UINT32 BinarySize)
{
	for (UINT32 Index = 0; Index < BinarySize; ++Index)
		Print(L"%02x", Binary[Index]);
}

STATIC VOID
ShowVariableLog(VOID *Binary, UINT32 BinarySize)
{
	EFI_VARIABLE_DATA_TREE *VarLog;

	VarLog = (EFI_VARIABLE_DATA_TREE *)Binary;

	Print(L"    Variable name: %g\n", &VarLog->VariableName);
	Print(L"    Unicode name length: %lld-byte\n",
	      VarLog->UnicodeNameLength * sizeof(CHAR16));
	Print(L"    Variable data length: %lld-byte\n",
	      VarLog->VariableDataLength);

	Print(L"    Unicode name: ");
	ShowString(VarLog->UnicodeName, VarLog->UnicodeNameLength);
	Print(L"\n");

	Print(L"    Variable data:\n");
	if (VarLog->VariableDataLength) {
		Print(L"      ");
		UINT8 *VariableData = (UINT8 *)VarLog + sizeof(*VarLog) -
				      sizeof(VarLog->UnicodeName) -
				      sizeof(VarLog->VariableData) +
				      VarLog->UnicodeNameLength;
		ShowHexString(VariableData, VarLog->VariableDataLength);
		Print(L"\n");
	}
}

STATIC VOID
ShowImageLog(VOID *Binary, UINT32 BinarySize)
{
	EFI_IMAGE_LOAD_EVENT *ImageLog;

	ImageLog = (EFI_IMAGE_LOAD_EVENT *)Binary;

	Print(L"    Image location: 0x%llx\n",
	      ImageLog->ImageLocationInMemory);
	Print(L"    Image length: %d-byte\n",
	      ImageLog->ImageLengthInMemory);
	Print(L"    Image link time address: 0x%x\n",
	      ImageLog->ImageLinkTimeAddress);
	Print(L"    Device path length: %d-byte\n",
	      ImageLog->LengthOfDevicePath);

	if (ImageLog->LengthOfDevicePath) {
		Print(L"    Device path:\n");
		Print(L"      ");
		Print(DevicePathToStr(ImageLog->DevicePath));
		Print(L"\n");
	}
}

STATIC VOID
ShowEventData(TCG_EVENTTYPE EventType, UINT8 *EventData,
	      UINTN EventDataSize)
{
	Print(L"  Event Data:\n");

	if (EventType == EV_POST_CODE ||
	    EventType == EV_S_CRTM_VERSION ||
	    EventType == EV_SEPARATOR) {
		Print(L"    ");
		ShowHexString(EventData, EventDataSize);
		Print(L"\n");
	} else if (EventType == EV_EFI_VARIABLE_DRIVER_CONFIG ||
		   EventType == EV_EFI_VARIABLE_BOOT)
		ShowVariableLog(EventData, EventDataSize);
	else if (EventType == EV_EFI_BOOT_SERVICES_APPLICATION ||
		 EventType == EV_EFI_BOOT_SERVICES_DRIVER ||
		 EventType == EV_EFI_RUNTIME_SERVICES_DRIVER)
		ShowImageLog(EventData, EventDataSize);
	else {
		Print(L"    ");
		ShowHexString(EventData, EventDataSize);
		Print(L"\n");
	}
}

STATIC UINTN
GetTcgDigestAlgorithmLength(TPM_ALG_ID AlgId)
{
	UINTN Length;

	switch (AlgId) {
	case TPM_ALG_SHA1:
		Length = SHA1_DIGEST_SIZE;
		break;
	case TPM_ALG_SHA256:
		Length = SHA256_DIGEST_SIZE;
		break;
	case TPM_ALG_SHA384:
		Length = SHA384_DIGEST_SIZE;
		break;
	case TPM_ALG_SHA512:
		Length = SHA512_DIGEST_SIZE;
		break;
	case TPM_ALG_SM3_256:
		Length = SM3_256_DIGEST_SIZE;
		break;
	default:
		Length = 0;
	}

	return Length;
}

STATIC CHAR16 *
GetTcgAlgorithmName(TPM_ALG_ID AlgId)
{
	CHAR16 *Name;

	switch (AlgId) {
	case TPM_ALG_SHA1:
		Name = L"SHA1";
		break;
	case TPM_ALG_AES:
		Name = L"AES";
		break;
	case TPM_ALG_KEYEDHASH:
		Name = L"KEYEDHASH";
		break;
	case TPM_ALG_SHA256:
		Name = L"SHA256";
		break;
	case TPM_ALG_SHA384:
		Name = L"SHA384";
		break;
	case TPM_ALG_SHA512:
		Name = L"SHA512";
		break;
	case TPM_ALG_NULL:
		Name = L"NULL";
		break;
	case TPM_ALG_SM3_256:
		Name = L"SM3-256";
		break;
	case TPM_ALG_SM4:
		Name = L"SM4";
		break;
	case TPM_ALG_RSASSA:
		Name = L"RSASSA";
		break;
	case TPM_ALG_RSAES:
		Name = L"RSAES";
		break;
	case TPM_ALG_RSAPSS:
		Name = L"RSAPSS";
		break;
	case TPM_ALG_OAEP:
		Name = L"OAEP";
		break;
	case TPM_ALG_ECDSA:
		Name = L"ECDSA";
		break;
	case TPM_ALG_ECDH:
		Name = L"ECDH";
		break;
	case TPM_ALG_ECDAA:
		Name = L"ECDAA";
		break;
	case TPM_ALG_SM2:
		Name = L"SM2";
		break;
	case TPM_ALG_ECSCHNORR:
		Name = L"ECSCHNORR";
		break;
	case TPM_ALG_ECMQV:
		Name = L"ECMQV";
		break;
	case TPM_ALG_KDF1_SP800_56a:
		Name = L"KDF1_SP800_56a";
		break;
	case TPM_ALG_KDF2:
		Name = L"KDF2";
		break;
	case TPM_ALG_KDF1_SP800_108:
		Name = L"KDF1_SP800_108";
		break;
	case TPM_ALG_ECC:
		Name = L"ECC";
		break;
	case TPM_ALG_SYMCIPHER:
		Name = L"SYMCIPHER";
		break;
	case TPM_ALG_CTR:
		Name = L"CTR";
		break;
	case TPM_ALG_OFB:
		Name = L"OFB";
		break;
	case TPM_ALG_CBC:
		Name = L"CBC";
		break;
	case TPM_ALG_CFB:
		Name = L"CFB";
		break;
	case TPM_ALG_ECB:
		Name = L"ECB";
		break;
	default:
		Name = L"Unknown";
	}

	return Name;
}

STATIC VOID
ShowTcg2EventLogs(EFI_PHYSICAL_ADDRESS StartEntry,
		  EFI_PHYSICAL_ADDRESS LastEntry)
{
	Print(L"TCG 2.0 Normal Event Logs:\n");

	TCG_PCR_EVENT *Event = (TCG_PCR_EVENT *)StartEntry;
	Print(L"  Event Log Header: %d-byte\n", Event->EventSize);

	TCG_EfiSpecIDEventStruct *EventLogHeader;
	EventLogHeader = (TCG_EfiSpecIDEventStruct *)Event->Event;

	Print(L"    Signature: ");
	ShowAsciiString(EventLogHeader->signature,
			sizeof(EventLogHeader->signature));
	Print(L"\n");

	Print(L"    Platform class: 0x%x (%s)\n",
	      EventLogHeader->platformClass,
	      EventLogHeader->platformClass == TCG_PLATFORM_TYPE_CLIENT ?
	      L"Client platform" :
	      (EventLogHeader->platformClass == TCG_PLATFORM_TYPE_SERVER
	       ? L"Server platform" : L"dirty"));
	Print(L"    Spec version: %d.%d errata %d\n",
	      EventLogHeader->specVersionMajor,
	      EventLogHeader->specVersionMinor,
	      EventLogHeader->specErrata);
	Print(L"    UINTN size: %d-byte\n",
	      EventLogHeader->uintnSize * sizeof(UINT32));
	Print(L"    Number of algorithms: %d\n",
	      EventLogHeader->numberOfAlgorithms);

	for (UINTN Idx = 0; Idx < EventLogHeader->numberOfAlgorithms; ++Idx) {
		TCG_EfiSpecIdEventAlgorithmSize *Alg;

		Alg = EventLogHeader->digestSizes + Idx;
		Print(L"      Digest algorithm ID: 0x%x, %d-byte (%s)\n",
		      Alg->algorithmId, Alg->digestSize,
		      GetTcgAlgorithmName(Alg->algorithmId));
	}

	UINT8 *Pointer = (UINT8 *)(EventLogHeader->digestSizes
			 + EventLogHeader->numberOfAlgorithms);
	UINT8 VendorInfoSize = *Pointer++;
	Print(L"    Vendor info size: %d-byte\n", VendorInfoSize);

	Print(L"    Vendor info: ");
	ShowHexString(Pointer, VendorInfoSize);
	Print(L"\n");

	UINTN Index = 1;
	TCG_PCR_EVENT2 *Event2;

	Event2 = (TCG_PCR_EVENT2 *)((UINT8 *)(Event + 1) + Event->EventSize);

	while ((EFI_PHYSICAL_ADDRESS)Event2 < LastEntry) {
		Print(L"[%02d]\n", Index);
		Print(L"  PCR Index: %d\n", Event2->PCRIndex);
		Print(L"  Event Type: 0x%x (%s)\n", Event2->EventType,
		      GetEventTypeName(Event2->EventType));

		UINT8 *Pointer = (UINT8 *)Event2->Digests.Digests;

		Print(L"  Digests: %d\n", Event2->Digests.Count);
		for (UINTN Idx = 0; Idx < Event2->Digests.Count; ++Idx) {
			TPMT_HA *Digest = (TPMT_HA *)Pointer;
			UINTN DigestAlgLength;
			CHAR16 *DigestAlgName;

			DigestAlgLength = GetTcgDigestAlgorithmLength(Digest->AlgorithmId);
			DigestAlgName = GetTcgAlgorithmName(Digest->AlgorithmId);

			Print(L"    %s: ", DigestAlgName);
			ShowHexString(Digest->Digest, DigestAlgLength);
			Print(L"\n");

			Pointer += sizeof(TPMT_HA) + DigestAlgLength;
		}

		UINT32 EventDataSize = *(UINT32 *)Pointer;
		Pointer += sizeof(UINT32);
		Print(L"  Event Size: %d-byte\n", EventDataSize);

		ShowEventData(Event2->EventType, Pointer, EventDataSize);

		Event2 = (TCG_PCR_EVENT2 *)(Pointer + EventDataSize);
		++Index;
	}
}

STATIC VOID
ShowTcg1EventLogs(EFI_PHYSICAL_ADDRESS StartEntry,
		  EFI_PHYSICAL_ADDRESS LastEntry)
{
	TCG_PCR_EVENT *Event = (TCG_PCR_EVENT *)StartEntry;
	UINTN Index = 1;

	Print(L"TCG 1.2 Normal Event Logs:\n");
	while ((EFI_PHYSICAL_ADDRESS)Event <= LastEntry) {
		Print(L"[%02d]\n", Index);
		Print(L"  PCR Index: %d\n", Event->PCRIndex);
		Print(L"  Event Type: 0x%x (%s)\n", Event->EventType,
		      GetEventTypeName(Event->EventType));
		Print(L"  Digest: ");
		for (UINTN Idx = 0; Idx < sizeof(Event->Digest); ++Idx)
			Print(L"%02x", Event->Digest[Idx]);
		Print(L"\n");
		Print(L"  Event Size: %d-byte\n", Event->EventSize);
		ShowEventData(Event->EventType, Event->Event,
			      Event->EventSize);

		Event = (TCG_PCR_EVENT *)((UINT8 *)(Event + 1) +
			Event->EventSize);
		++Index;
	}
}

STATIC VOID
ShowEventLogs(EFI_TCG2_EVENT_LOG_BITMAP Format,
	      EFI_PHYSICAL_ADDRESS StartEntry,
	      EFI_PHYSICAL_ADDRESS LastEntry)
{
	if (Format == EFI_TCG2_EVENT_LOG_FORMAT_TCG_1_2)
		ShowTcg1EventLogs(StartEntry, LastEntry);
	else
		ShowTcg2EventLogs(StartEntry, LastEntry);
}

EFI_STATUS
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *Systab)
{
	InitializeLib(ImageHandle, Systab);

	EFI_TCG2_BOOT_SERVICE_CAPABILITY *TpmCapability;
	TpmCapability = GetCapability();
	if (!TpmCapability)
		return EFI_UNSUPPORTED;

	EFI_TCG2_EVENT_LOG_BITMAP SupportedFormats;
	SupportedFormats = GetSupportedEventLogFormat(TpmCapability);
	FreePool(TpmCapability);
	if (!SupportedFormats)
		return EFI_UNSUPPORTED;

	EFI_TCG2_PROTOCOL *Tcg2;
	EFI_STATUS Status = Tcg2LocateProtocol(&Tcg2);
	if (EFI_ERROR(Status))
		return Status;

	EFI_TCG2_EVENT_LOG_BITMAP FormatList[] = {
		EFI_TCG2_EVENT_LOG_FORMAT_TCG_1_2,
		EFI_TCG2_EVENT_LOG_FORMAT_TCG_2,
	};
	for (UINTN Index = 0;
	     Index < sizeof(FormatList) / sizeof(EFI_TCG2_EVENT_LOG_BITMAP);
	     ++Index) {
		if (!(SupportedFormats & FormatList[Index]))
			continue;

		EFI_PHYSICAL_ADDRESS StartEntry;
		EFI_PHYSICAL_ADDRESS LastEntry;
		BOOLEAN Truncated;
		Status = uefi_call_wrapper(Tcg2->GetEventLog, 5, Tcg2,
					   FormatList[Index], &StartEntry,
					   &LastEntry, &Truncated);
		if (EFI_ERROR(Status))
			break;

		if (StartEntry == LastEntry) {
			CONST CHAR16 *Ver[] = {
				L"1.2", L"2.0",
			};

			Print(L"Skip the empty TCG %s event log area\n",
			      Ver[Index]);
			continue;
		}

		ShowEventLogs(FormatList[Index], StartEntry, LastEntry);

		if (Truncated)
			Print(L"The event log area is truncated\n");
	}

	return Status;
}
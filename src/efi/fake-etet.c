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
#include "etet.h"

typedef UINT32 EFI_TCG2_EVENT_ALGORITHM_BITMAP;
typedef UINT32 EFI_TCG2_EVENT_LOG_BITMAP;
typedef UINT32 EFI_TCG2_EVENT_LOG_FORMAT;

typedef struct tdEFI_TCG2_VERSION {
	UINT8 Major;
	UINT8 Minor;
} EFI_TCG2_VERSION;

typedef struct tdEFI_TCG2_BOOT_SERVICE_CAPABILITY {
	UINT8 Size;
	EFI_TCG2_VERSION StructureVersion;
	EFI_TCG2_VERSION ProtocolVersion;
	EFI_TCG2_EVENT_ALGORITHM_BITMAP HashAlgorithmBitmap;
	EFI_TCG2_EVENT_LOG_BITMAP SupportedEventLogs;
	BOOLEAN TPMPresentFlag;
	UINT16 MaxCommandSize;
	UINT16 MaxResponseSize;
	UINT32 ManufacturerID;
	UINT32 NumberOfPcrBanks;
	EFI_TCG2_EVENT_ALGORITHM_BITMAP ActivePcrBanks;
} EFI_TCG2_BOOT_SERVICE_CAPABILITY;

typedef struct tdEFI_TCG2_EVENT_HEADER {
	UINT32 HeaderSize;
	UINT16 HeaderVersion;
	TCG_PCRINDEX PCRIndex;
	TCG_EVENTTYPE EventType;
} EFI_TCG2_EVENT_HEADER;

typedef struct tdEFI_TCG2_EVENT { 
	UINT32 Size; 
	EFI_TCG2_EVENT_HEADER Header; 
	UINT8 Event[]; 
} EFI_TCG2_EVENT;

struct tdEFI_TCG2_PROTOCOL;

typedef EFI_STATUS
(EFIAPI *EFI_TCG2_GET_CAPABILITY) (
	IN struct tdEFI_TCG2_PROTOCOL *This,
	IN OUT EFI_TCG2_BOOT_SERVICE_CAPABILITY *ProtocolCapability
);

typedef EFI_STATUS
(EFIAPI *EFI_TCG2_GET_EVENT_LOG) ( 
	IN struct tdEFI_TCG2_PROTOCOL *This,
	IN EFI_TCG2_EVENT_LOG_FORMAT EventLogFormat,
	OUT EFI_PHYSICAL_ADDRESS *EventLogLocation,
	OUT EFI_PHYSICAL_ADDRESS *EventLogLastEntry,
	OUT BOOLEAN *EventLogTruncated
);

typedef EFI_STATUS
(EFIAPI *EFI_TCG2_HASH_LOG_EXTEND_EVENT) (
	IN struct tdEFI_TCG2_PROTOCOL *This,
	IN UINT64 Flags,
	IN EFI_PHYSICAL_ADDRESS DataToHash,
	IN UINT64 DataToHashLen,
	IN EFI_TCG2_EVENT *EfiTcgEvent
);

typedef EFI_STATUS
(EFIAPI *EFI_TCG2_SUBMIT_COMMAND) (
	IN struct tdEFI_TCG2_PROTOCOL *This,
	IN UINT32 InputParameterBlockSize,
	IN UINT8 *InputParameterBlock,
	IN UINT32 OutputParameterBlockSize,
	IN UINT8 *OutputParameterBlock
);

typedef EFI_STATUS
(EFIAPI *EFI_TCG2_GET_ACTIVE_PCR_BANKS) (
	IN struct tdEFI_TCG2_PROTOCOL *This,
	OUT UINT32 *ActivePcrBanks
);

typedef EFI_STATUS
(EFIAPI *EFI_TCG2_SET_ACTIVE_PCR_BANKS) (
	IN struct tdEFI_TCG2_PROTOCOL *This,
	IN UINT32 ActivePcrBanks
);

typedef EFI_STATUS
(EFIAPI * EFI_TCG2_GET_RESULT_OF_SET_ACTIVE_PCR_BANKS) (
	IN struct tdEFI_TCG2_PROTOCOL *This,
	OUT UINT32 *OperationPresent,
	OUT UINT32 *Response
);

typedef struct tdEFI_TCG2_PROTOCOL {
	EFI_TCG2_GET_CAPABILITY GetCapability;
	EFI_TCG2_GET_EVENT_LOG GetEventLog;
	EFI_TCG2_HASH_LOG_EXTEND_EVENT HashLogExtendEvent;
	EFI_TCG2_SUBMIT_COMMAND SubmitCommand;
	EFI_TCG2_GET_ACTIVE_PCR_BANKS GetActivePcrBanks;
	EFI_TCG2_SET_ACTIVE_PCR_BANKS SetActivePcrBanks;
	EFI_TCG2_GET_RESULT_OF_SET_ACTIVE_PCR_BANKS GetResultOfSetActivePcrBanks;
} EFI_TCG2_PROTOCOL;

#define EFI_TCG2_PROTOCOL_GUID	\
	{ 0x607f766c, 0x7455, 0x42be,	\
	{ 0x93, 0x0b, 0xe4, 0xd7, 0x6d, 0xb2, 0x72, 0x0f } }

static void *
find_etet(void)
{
	EFI_GUID guid = EFI_TCG2_FINAL_EVENTS_TABLE_GUID;
	void *table;
	EFI_STATUS rc;

	rc = LibGetSystemConfigurationTable(&guid, &table);
	if (EFI_ERROR(rc)) {
		Print(L"Unable to find ETET: %r\n", rc);
		return NULL;
	}

	return table;
}

static EFI_STATUS
install_etet(void)
{
	struct {
		EFI_TCG2_FINAL_EVENTS_TABLE v1;
	} __attribute__((packed)) etet = {
		.v1 = { 1, 0,  },
	};
	UINTN etet_nr_page;
	EFI_PHYSICAL_ADDRESS etet_boot_mem = 0;
	EFI_GUID guid = EFI_TCG2_FINAL_EVENTS_TABLE_GUID;
	EFI_STATUS rc;
	
	etet_nr_page = (sizeof(etet) + EFI_PAGE_SIZE - 1) >> EFI_PAGE_SHIFT;

	rc = uefi_call_wrapper(BS->AllocatePages, 4, AllocateAnyPages,
			       EfiACPIMemoryNVS, 1,
			       &etet_boot_mem);
	if (EFI_ERROR(rc)) {
		Print(L"AllocatePages failed: %r\n", rc);
		return rc;
	}

	CopyMem((void *)etet_boot_mem, &etet, sizeof(etet));

	rc = uefi_call_wrapper(BS->InstallConfigurationTable, 2, &guid,
			       (VOID *)etet_boot_mem);
	if (EFI_ERROR(rc)) {
		Print(L"Unable to install ETET table: %r\n", rc);
		uefi_call_wrapper(BS->FreePages, 2,
				  etet_boot_mem, etet_nr_page);
		return rc;
	}

	return EFI_SUCCESS;
}

EFI_STATUS
efi_main(EFI_HANDLE image_handle, EFI_SYSTEM_TABLE *systab)
{
	EFI_STATUS rc;
	EFI_TCG2_FINAL_EVENTS_TABLE *table;
	EFI_TCG2_PROTOCOL *tcg2;
	EFI_GUID guid = EFI_TCG2_PROTOCOL_GUID;

	InitializeLib(image_handle, systab);

	/* Try to get efi tpm2 protocol */
	rc = LibLocateProtocol(&guid, (VOID **)&tcg2);
	if (EFI_ERROR(rc))
		Print(L"Unable to locate EFI TPM2 Protocol: %r\n", rc);
	else
		Print(L"EFI TPM2 Protocol installed\n");

	table = (EFI_TCG2_FINAL_EVENTS_TABLE *)find_etet();
	if (table) {
		UINT64 version = table->Version;

		if (!version || version > EFI_TCG2_FINAL_EVENTS_TABLE_VERSION) {
			Print(L"Unsupported ETET (version %llx)\n", version);
			uefi_call_wrapper(BS->Stall, 1, 3000000);
			return EFI_UNSUPPORTED;
		}

		Print(L"ETET (version %lld, number of events %lld) detected\n",
		      version, table->NumberOfEvents);
		Print(L"Launching fake-etet ignored\n");
		uefi_call_wrapper(BS->Stall, 1, 3000000);

		return EFI_SUCCESS;
	}

	rc = install_etet();
	if (!EFI_ERROR(rc))
		Print(L"Fake ETET installed\n");

	uefi_call_wrapper(BS->Stall, 1, 3000000);

	return rc;
}

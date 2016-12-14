/*
 * The definitions for EFI TCG2 Protocol
 *
 * Copyright (c) 2016, Lans Zhang
 * All rights reserved.
 *
 * See "LICENSE" for license terms.
 *
 * Author:
 *      Lans Zhang <jia.zhang@windriver.com>
 */

#ifndef TCG2_H
#define TCG2_H

#include <efi.h>
#include <efilib.h>

#define EFI_TCG2_PROTOCOL_GUID	\
	{ 0x607f766c, 0x7455, 0x42be,	\
	{ 0x93, 0x0b, 0xe4, 0xd7, 0x6d, 0xb2, 0x72, 0x0f } }

typedef struct {
	UINT8 Major;
	UINT8 Minor;
} EFI_TCG2_VERSION;

typedef UINT32				EFI_TCG2_EVENT_ALGORITHM_BITMAP;
typedef UINT32				EFI_TCG2_EVENT_LOG_BITMAP;
typedef UINT32				EFI_TCG2_EVENT_LOG_FORMAT;

/*
 * According to TCG EFI Protocol Specification for TPM 2.0 family,
 * the EFI_TCG2_BOOT_SERVICE_CAPABILITY structure must not be packed.
 */
typedef struct {
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

typedef struct {
	UINT8 Major;
	UINT8 Minor;
	UINT8 RevMajor;
	UINT8 RevMinor;
} EFI_TCG_VERSION;

typedef struct {
	UINT8 Size;
	EFI_TCG_VERSION StructureVersion;
	EFI_TCG_VERSION ProtocolVersion;
	UINT8 HashAlgorithmBitmap;
	BOOLEAN TPMPresentFlag;
	BOOLEAN TPMDeactivatedFlag;
} EFI_TCG_BOOT_SERVICE_CAPABILITY;

#pragma pack(1)

typedef UINT32				TCG_PCRINDEX;
typedef UINT32				TCG_EVENTTYPE;

typedef struct {
	UINT32 HeaderSize;
	UINT16 HeaderVersion;
	TCG_PCRINDEX PCRIndex;
	TCG_EVENTTYPE EventType;
} EFI_TCG2_EVENT_HEADER;

typedef struct {
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

#define EFI_TCG2_EVENT_LOG_FORMAT_TCG_1_2	0x00000001
#define EFI_TCG2_EVENT_LOG_FORMAT_TCG_2		0x00000002

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

extern EFI_STATUS
Tcg2LocateProtocol(EFI_TCG2_PROTOCOL **Tcg2);

extern EFI_STATUS
Tcg2DetectTPM(VOID);

extern EFI_STATUS
Tcg2GetCapability(EFI_TCG2_BOOT_SERVICE_CAPABILITY *Capability,
		  UINT8 *Size);

#endif	/* TCG2_H */
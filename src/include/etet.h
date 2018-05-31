/*
 * The definitions for EFI TCG2 Final events table
 *
 * Copyright (c) 2016, Wind River Systems, Inc.
 * All rights reserved.
 *
 * See "LICENSE" for license terms.
 *
 * Author:
 *      Jia Zhang <zhang.jia@linux.alibaba.com>
 */

#ifndef ETET_H
#define ETET_H

#include <efi.h>
#include <efilib.h>

#include <tcg2.h>

#pragma pack(1)

#define EFI_TCG2_FINAL_EVENTS_TABLE_GUID \
	{ 0x1e2ed096, 0x30e2, 0x4254, \
	{ 0xbd, 0x89, 0x86, 0x3b, 0xbe, 0xf8, 0x23, 0x25 } }

#define EFI_TCG2_FINAL_EVENTS_TABLE_VERSION	1

/*
 * The definitions for TCG 2.0
 */

typedef struct {
	UINT16 AlgorithmId;
	UINT8 Digest[];
} TPMT_HA;

typedef struct {
	UINT32 Count;
	TPMT_HA Digests[];
} TPML_DIGEST_VALUES;

typedef struct {
	UINT16 algorithmId;
	UINT16 digestSize;
} TCG_EfiSpecIdEventAlgorithmSize;

#define TCG_PLATFORM_TYPE_CLIENT		0
#define TCG_PLATFORM_TYPE_SERVER		1

/*
 * The definition for event log header
 */
typedef struct {
	UINT8 signature[16];
	UINT32 platformClass;
	UINT8 specVersionMinor;
	UINT8 specVersionMajor;
	UINT8 specErrata;
	UINT8 uintnSize;
	/* Added since "Spec ID Event03" */
	UINT32 numberOfAlgorithms;
	TCG_EfiSpecIdEventAlgorithmSize digestSizes[0];
	//UINT8 vendorInfoSize;
	//UINT8 vendorInfo[vendorInfoSize];
 } TCG_EfiSpecIDEventStruct;

typedef struct {
	TCG_PCRINDEX PCRIndex;
	TCG_EVENTTYPE EventType;
	TPML_DIGEST_VALUES Digests;
	UINT32 EventSize;
	UINT8 Event[];
} TCG_PCR_EVENT2;

typedef struct {
	UINT64 Version;
	UINT64 NumberOfEvents;
	TCG_PCR_EVENT2 Event[];
} EFI_TCG2_FINAL_EVENTS_TABLE;

/*
 * The definitions for TCG 1.x
 */

typedef UINT8				TCG_DIGEST[20];

typedef struct {
	TCG_PCRINDEX PCRIndex;
	TCG_EVENTTYPE EventType;
	TCG_DIGEST Digest;
	UINT32 EventSize;
	UINT8 Event[];
} TCG_PCR_EVENT;

union EFI_TCG2_FINAL_EVENTS_TABLE_UNION {
	EFI_TCG2_FINAL_EVENTS_TABLE v1;
};

/*
 * Common definitions for the event data
 */

typedef struct {
	EFI_GUID VariableName;
	UINT64 UnicodeNameLength;
	UINT64 VariableDataLength;
	CHAR16 UnicodeName[1];
	INT8 VariableData[1];
} EFI_VARIABLE_DATA_TREE;

typedef struct {
	EFI_PHYSICAL_ADDRESS ImageLocationInMemory;
	UINTN ImageLengthInMemory;
	UINTN ImageLinkTimeAddress;
	INTN LengthOfDevicePath;
	EFI_DEVICE_PATH DevicePath[0];
} EFI_IMAGE_LOAD_EVENT;

#pragma pack()

extern EFI_STATUS
EtetLocateAddress(VOID **Table);

#endif	/* ETET_H */
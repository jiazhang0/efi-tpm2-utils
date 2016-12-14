/*
 * The definitions for EFI TCG2 Final events table
 *
 * Copyright (c) 2016, Wind River Systems, Inc.
 * All rights reserved.
 *
 * See "LICENSE" for license terms.
 *
 * Author:
 *      Lans Zhang <jia.zhang@windriver.com>
 */

#ifndef ETET_H
#define ETET_H

#include <efi.h>
#include <efilib.h>

#pragma pack(1)

#define EFI_TCG2_FINAL_EVENTS_TABLE_GUID \
	{ 0x1e2ed096, 0x30e2, 0x4254, \
	{ 0xbd, 0x89, 0x86, 0x3b, 0xbe, 0xf8, 0x23, 0x25 } }

#define EFI_TCG2_FINAL_EVENTS_TABLE_VERSION	1

typedef struct {
	UINT16 AlgorithmId;
	UINT8 Digest[];
} TPMT_HA;

typedef struct {
	UINT32 Count;
	TPMT_HA Digests[];
} TPML_DIGEST_VALUES;

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

union EFI_TCG2_FINAL_EVENTS_TABLE_UNION {
	EFI_TCG2_FINAL_EVENTS_TABLE v1;
};

#pragma pack()

#endif	/* ETET_H */
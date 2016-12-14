/*
 * Fake ETET
 *
 * Copyright (c) 2016, Lans Zhang
 * All rights reserved.
 *
 * See "LICENSE" for license terms.
 *
 * Author:
 *      Lans Zhang <jia.zhang@windriver.com>
 */

#ifndef __ETET_H__
#define __ETET_H__

typedef uint32_t	TCG_PCRINDEX;
typedef uint32_t	TCG_EVENTTYPE;

#pragma pack(1)

typedef struct tdTPMT_HA {
	uint16_t AlgorithmId;
	uint8_t Digest[];
} TPMT_HA;

typedef struct tdTPML_DIGEST_VALUES {
	uint32_t Count;
	TPMT_HA Digests[];
} TPML_DIGEST_VALUES;

typedef struct tdTCG_PCR_EVENT2 {
	TCG_PCRINDEX PCRIndex;
	TCG_EVENTTYPE EventType;
	TPML_DIGEST_VALUES Digests;
	uint32_t EventSize;
	uint8_t Event[];
} TCG_PCR_EVENT2; 

typedef struct tdEFI_TCG2_FINAL_EVENTS_TABLE {
	uint64_t Version;
	uint64_t NumberOfEvents;
	TCG_PCR_EVENT2 Event[];
} EFI_TCG2_FINAL_EVENTS_TABLE;

#pragma pack()

union efi_tcg2_final_events_table {
	EFI_TCG2_FINAL_EVENTS_TABLE v1;
};

#define EFI_TCG2_FINAL_EVENTS_TABLE_VERSION		1

#define EFI_TCG2_FINAL_EVENTS_TABLE_GUID \
	{ 0x1e2ed096, 0x30e2, 0x4254, \
	{ 0xbd, 0x89, 0x86, 0x3b, 0xbe, 0xf8, 0x23, 0x25 } }

#endif	/* __ETET_H__ */
/** ------------------------------------------------------------------------- *
    ------------------------------------------------------------------------- *


    \file pttFrameGen.h

    \brief Definitions for PTT frame generation

    $Id$

    Copyright (C) 2006 Airgo Networks, Incorporated


   ========================================================================== */

#ifndef PTTFRAMEGEN_H
#define PTTFRAMEGEN_H


//#define MAX_PKT_GEN_BUF_ENTRY  (HAL_HIF_MAX_TX_RING_ENTRY >> 1)



#define MAX_PAYLOAD_SIZE 4096

typedef enum {
   TEST_PAYLOAD_NONE,
   TEST_PAYLOAD_FILL_BYTE,
   TEST_PAYLOAD_RANDOM,
   TEST_PAYLOAD_RAMP,
   TEST_PAYLOAD_TEMPLATE,
   TEST_PAYLOAD_MAX = 0X3FFFFFFF,   //dummy value to set enum to 4 bytes
} ePayloadContents;


typedef PACKED_PRE struct PACKED_POST {
   tANI_U32 numTestPackets;
   tANI_U32 interFrameSpace;
   eHalPhyRates rate;
   ePayloadContents payloadContents;
   tANI_U16 payloadLength;
   tANI_U8 payloadFillByte;
   tANI_BOOLEAN pktAutoSeqNum;  //seq num setting (hw or not) for packet gen

   tANI_U8 addr1[ANI_MAC_ADDR_SIZE];
   tANI_U8 addr2[ANI_MAC_ADDR_SIZE];
   tANI_U8 addr3[ANI_MAC_ADDR_SIZE];
   tANI_U8 pktScramblerSeed;    //for tx scrambler
   tANI_BOOLEAN crc;            //0 = no FCS calculated = power detector works = receive won't work?,
   //1 = crc calculated = receive works, but power detector workaround doesn't

   ePhyDbgPreamble preamble;
} sPttFrameGenParams;


typedef PACKED_PRE struct PACKED_POST {
   tANI_U32 legacy;             //11g OFDM preamble
   tANI_U32 gfSimo20;           //greenfield preamble
   tANI_U32 gfMimo20;           //greenfield preamble
   tANI_U32 mmSimo20;           //mixed mode preamble
   tANI_U32 mmMimo20;           //mixed mode preamble
   tANI_U32 txbShort;
   tANI_U32 txbLong;
   tANI_U32 txbSlr;
   tANI_U32 total;
} sTxFrameCounters;

#endif

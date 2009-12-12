/** ------------------------------------------------------------------------- * 
    ------------------------------------------------------------------------- *  

  
    \file pttFrameGen.h
  
    \brief Definitions for PTT frame generation
  
    $Id$ 
  
    Copyright (C) 2006 Airgo Networks, Incorporated
  
  
   ========================================================================== */

#ifndef PTTFRAMEGEN_H
#define PTTFRAMEGEN_H

#include "asicPhyDbg.h"

//#define MAX_PKT_GEN_BUF_ENTRY  (HAL_HIF_MAX_TX_RING_ENTRY >> 1)



#define MAX_PAYLOAD_SIZE 4096

typedef enum
{
    TEST_PAYLOAD_NONE,
    TEST_PAYLOAD_FILL_BYTE,
    TEST_PAYLOAD_RANDOM,
    TEST_PAYLOAD_RAMP,
    TEST_PAYLOAD_TEMPLATE
}ePayloadContents;


typedef struct
{
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
    tANI_BOOLEAN crc;   //0 = no FCS calculated = power detector works = receive won't work?, 
                        //1 = crc calculated = receive works, but power detector workaround doesn't
    
    ePhyDbgPreamble preamble;
}sPttFrameGenParams;


typedef struct
{
    tANI_U32 legacy;
    tANI_U32 airgoSimo40;
    tANI_U32 airgoMimo20;
    tANI_U32 airgoMimo40;
    tANI_U32 ewcGfSimo20;
    tANI_U32 ewcGfMimo20;
    tANI_U32 ewcGfSimo40;
    tANI_U32 ewcGfMimo40;
    tANI_U32 ewcMmSimo20;
    tANI_U32 ewcMmMimo20;
    tANI_U32 ewcMmSimo40;
    tANI_U32 ewcMmMimo40;
    tANI_U32 txbShort;
    tANI_U32 txbLong;
    tANI_U32 total;
}sTxFrameCounters;

#endif


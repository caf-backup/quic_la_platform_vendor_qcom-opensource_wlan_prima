/** ------------------------------------------------------------------------- *
    ------------------------------------------------------------------------- *


    \file halFrameInfo.h

    \brief  Hardware Abstraction Layer interfaces for frame information.

    $Id$


    Copyright (c) 2011 Qualcomm Atheros, Inc. 
    All Rights Reserved. 
    Qualcomm Atheros Confidential and Proprietary. 
  
    Copyright (C) 2006 Airgo Networks, Incorporated
    
    This file contains all the interfaces for thge Hardware Abstration Layer
    functions.  It is intended to be included in all modules that are using
    the external HAL interfaces.

   ========================================================================== */

#ifndef HALFRAMEINFO_H
#define HALFRAMEINFO_H

/* Standard include files */

/* Application Specific include files */

#include "halTypes.h"

/* Constant Macros */
#define HAL_RX_FRAME_INFO_MAX_VALID_STA_IDX (252)
#define HAL_RX_FRAME_INFO_INVALID_STA_IDX (0xff)

/* Function Macros */

typedef struct sHalRxFrameInfo {
    eFrameType frameType;

    tANI_BOOLEAN srcStaAuth;
    tANI_U8 dstStaIdx;
    tANI_U8 taStaIdx;
    tANI_U8 rxReorderHeadIdx;

    tANI_U8 rxReorderEnqIdx;
    tANI_U8 rxReorderOpcode;
    tANI_U8 isAmsduSubframe;
    tANI_U8 rxReorderFastFwd; //for byte alignment.

    // Identifies the A-MPDU/BA Session ID
    tANI_U16 baSessionID;
    tANI_U16 totalAmsduSize;

    tANI_U8 resv1;
    tANI_U8 resv2;
    tANI_U8 rxPktNumValid;
    tANI_U8 tid;

    tANI_U32 rxpFlags;

    //For performing replay check to workaround a DPU bug
    //Note: 64bit number should be at 8 byte boundary otherwise the size of structure
    //is 4byte larger.
    tANI_U64 rxPktNum;

    tANI_U32 rxpTstamp;

} tHalRxFrameInfo, *tpHalRxFrameInfo;


#endif /* HALFRAMEINFO_H */


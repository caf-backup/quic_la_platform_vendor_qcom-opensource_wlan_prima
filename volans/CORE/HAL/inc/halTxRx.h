/** ------------------------------------------------------------------------- *
    ------------------------------------------------------------------------- *


    \file halTxRx.h

    \brief  Hardware Abstraction Layer interfaces for frame transmission and reception.

    $Id$


    Copyright (c) 2011 Qualcomm Atheros, Inc. 
    All Rights Reserved. 
    Qualcomm Atheros Confidential and Proprietary. 
  
    Copyright (C) 2006 Airgo Networks, Incorporated
  
    This file contains all the interfaces for thge Hardware Abstration Layer
    functions.  It is intended to be included in all modules that are using
    the external HAL interfaces.

   ========================================================================== */

#ifndef HALTXRX_H
#define HALTXRX_H

/* Standard include files */

/* Application Specific include files */

#include "halTypes.h"
#include "halGlobal.h"
#include "vos_status.h"
#include "vos_packet.h"
#include "vos_types.h"
/* Constant Macros */

typedef void (*pHalTxRxCompFunc)( tHalHandle hHal, void *pData );

eHalStatus halTxFrame(tHalHandle hHal,
                      void *pFrmBuf,
                      tANI_U16 frmLen,
                      eFrameType frmType,
                      eFrameTxDir txDir,
                      tANI_U8 tid,
                      pHalTxRxCompFunc pCompFunc,
                      void *pData, tANI_U8 txFlag);

eHalStatus halTxFrameWithTxComplete(tHalHandle hHal,
                      void *pFrmBuf,
                      tANI_U16 frmLen,
                      eFrameType frmType,
                      eFrameTxDir txDir,
                      tANI_U8 tid,
                      pHalTxRxCompFunc pCompFunc,
                      void *pData,
                      tpCBackFnTxComp pCBackFnTxComp,
                      tANI_U8 txFlag);

//VOS_STATUS halTxComplete(v_CONTEXT_t pVosGCtx, void* pData, VOS_STATUS status);
VOS_STATUS halTxComplete( v_PVOID_t pVosGCtx, vos_pkt_t *pData, VOS_STATUS status );
#endif /* HALTXRX_H */


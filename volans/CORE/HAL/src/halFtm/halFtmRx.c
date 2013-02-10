/** ------------------------------------------------------------------------- *
    ------------------------------------------------------------------------- *


    \file halFtmRx.c

    \brief halFtmRx.c

    $Id$

    Copyright (C) 2010 Qualcomm Technologies, Inc.

    Created by Viral Modi   07/06/2010  This file incorporates new functions related to new FTM requirement
                                        development. The new requirement is to have FCS count for unicast frames received in FTM mode
   ========================================================================== */
#ifndef WLAN_FTM_STUB
#include "halFtmRx.h"
#include "halDebug.h"
#include "halCommonApi.h"

eHalStatus halFtmRx_Start(tHalHandle hHal, void *arg)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tpAniSirGlobal pMac = (tpAniSirGlobal) hHal;

    HALLOGE( halLog(pMac, LOGE, FL("halFtm_Start\n")));
    if ( halFtm_AddStaSelf(pMac) != eHAL_STATUS_SUCCESS)
    {
        HALLOGE( halLog(pMac, LOGE, FL("halFtm_AddStaSelf() fail\n")));
        return eHAL_STATUS_FAILURE;
    }

    return status;
}

eHalStatus halFtm_AddStaSelf(tpAniSirGlobal     pMac)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tSirMacAddr  staMac;
    tANI_U8      staIdx = FTM_STA_ID;
    tANI_U8 rmfBit = 0;
    tANI_U8 dpuIdx = HAL_DPU_SELF_STA_DEFAULT_IDX;  //dpu index is DONT CARE in our case since dpu is not initialized and started in ftm
    tANI_U8 dpuSignature = 0;   //dpu signature is DONT CARE as well
    tANI_U8 ftBit;  //check if frame translation needs to be enabled or not
    tANI_BOOLEAN wep_keyId_extract = 0; //No encryption.   

    palCopyMemory(pMac->hHdd,
                (void *)staMac, (void *)pMac->ptt.frameGenParams.addr2,
                sizeof(tSirMacAddr));

    HALLOGE( halLog(pMac, LOGE, FL("mac addr from NV %X %X %X %X %X %X\n"),
        staMac[0], staMac[1], staMac[2], staMac[3], staMac[4], staMac[5]));
    
    ftBit = halGetFrameTranslation(pMac);
    //Add RXP entry.
    if (halRxp_AddEntry(pMac, (tANI_U8) staIdx, staMac, eRXP_SELF, rmfBit,
                        dpuIdx, dpuIdx, dpuIdx,
                        dpuSignature, dpuSignature, dpuSignature,
                        0, ftBit, wep_keyId_extract) != eHAL_STATUS_SUCCESS) {
        return eHAL_STATUS_FAILURE;
    }
    return status;
}
#endif

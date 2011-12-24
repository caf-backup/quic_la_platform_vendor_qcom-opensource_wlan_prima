/*
 * Copyright (c) 2011 Qualcomm Atheros, Inc. 
 * All Rights Reserved. 
 * Qualcomm Atheros Confidential and Proprietary. 
 * 
 * Copyright (C) 2006 Airgo Networks, Incorporated
 * 
 * This file halTimer.h contains the utility function HAL
 * uses for timer related functions.
 *
 * Author:        Susan Tsao
 * Date:          01/11/07
 * --------------------------------------------------------------------
 */
#ifndef _HALUTILS_H_
#define _HALUTILS_H_

#include  "aniGlobal.h"
#include  "palTypes.h"
#include  "halTypes.h"
#include  "sirParams.h"     // tSirMsgQ


typedef enum
{
    eHAL_MSG_DROP,
    eHAL_MSG_DEFER,
    eHAL_MSG_VALID
} tHalMsgDecision;


eRfBandMode   halUtil_GetRfBand( tpAniSirGlobal pMac, tANI_U8 channel );
tANI_BOOLEAN  halUtil_CurrentlyInPowerSave( tpAniSirGlobal pMac );
eHalStatus    halUtil_deferMsg(tpAniSirGlobal pMac, tSirMsgQ *pMsg);
void          halUtil_processDeferredMsgQ(tpAniSirGlobal pMac);
tHalMsgDecision  halUtil_MsgDecision(tpAniSirGlobal pMac, tSirMsgQ *pMsg, tANI_U8* pMutexAcquired);
tANI_U32  halUtil_GetBtqmQueueIdForStaidTid( tpAniSirGlobal pMac, tANI_U8 staId, tANI_U8 tid);

tANI_U32 halUtil_GetCardType(tpAniSirGlobal pMac);

void halUtil_EndianConvert(tpAniSirGlobal pMac, tANI_U32 *pBuf, tANI_U32 nLen);
void halSetChainConfig(tpAniSirGlobal pMac, tANI_U32 powerStatePerChain);
void halUtil_getProtectionMode(tpAniSirGlobal pMac, tTpeProtPolicy *pProtPolicy);
tANI_U16 halUtil_GetGCD(tANI_U16 num1, tANI_U16 num2);
void halUtil_GetDtimTbtt(tpAniSirGlobal pMac, tANI_U64 tbtt, tANI_U8 bssIdx,
        tANI_U8 dtimPeriod, tANI_U8 dtimCount, tANI_U64 *pDtimTbtt);
void halUtil_GetLeastRefDtimTbtt(tpAniSirGlobal pMac, tANI_U8 bssIdx, tANI_U64 dtimTbtt, tANI_U64 *pRefDtimTbtt, tANI_U16 dtimPeriod);
void halUtil_GetRegPowerLimit(tpAniSirGlobal pMac, tANI_U8 currChannel,
        tANI_U8 localPwrConstraint, tANI_S8 *pRegLimit);
void halUtil_DumpFwCorexLogs(void *pData);
#ifdef WLAN_DEBUG
char* halUtil_getMsgString(tANI_U16 msgId);
#endif
#endif /* _HALUTILS_H_ */




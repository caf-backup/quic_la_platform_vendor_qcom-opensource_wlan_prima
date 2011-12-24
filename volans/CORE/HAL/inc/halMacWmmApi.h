
/*
 * Copyright (c) 2011 Qualcomm Atheros, Inc. 
 * All Rights Reserved. 
 * Qualcomm Atheros Confidential and Proprietary. 
 * 
 * Copyright (C) 2006 Airgo Networks, Incorporated
 * 
 * halMacWmmApi.h: Header for HAL WMM API's
 * Author:  Neelay Das
 * Date:    10/22/2006
 * History:-
 * Date     Modified by         Modification Information
 * --------------------------------------------------------------------------
 *
 */

#ifndef _HALMAC_WMM_API_H
#define _HALMAC_WMM_API_H

// Take care to avoid redefinition of this type, if it is
// already defined in "aniGlobal.h"
#if !defined(_ANIGLOBAL_H)
typedef struct sAniSirGlobal *tpAniSirGlobal;
#endif

#include "sirTypes.h"
#include "halMsgApi.h"

#define HAL_APSD_AC_VO_MASK  0x01
#define HAL_APSD_AC_VI_MASK  0x02
#define HAL_APSD_AC_BK_MASK  0x04
#define HAL_APSD_AC_BE_MASK  0x08

// HAL WMM macros for TID to AC map manipulation

#define HAL_WMM_MAP_ALL_TID_TO_BE_MAP   0xffff0000
#define    HAL_WMM_DEFAULT_TID_AC_MAP    0xfffffa14
#define SET_AC_FOR_THIS_TID(_tid, _ac) ( ((_ac) & 3) << ((_tid)*2) )
#define CLEAR_AC(_tidAcMap, _tid) ((_tidAcMap) & (~(3 << ((_tid)*2))))
#define HAL_WMM_TID_TO_AC(_tidAcMap, _tid) ( ((_tidAcMap) >> ((_tid)*2)) & 3 )
#define HAL_WMM_SET_TID_TO_AC(_tidAcMap, _tid, _ac) (SET_AC_FOR_THIS_TID(_tid, _ac) | CLEAR_AC(_tidAcMap, _tid))


typedef struct sCfgTrafficClass {
    //Use Block ACK on this STA/TID
    // Fields used to store the default TC parameters for this TSPEC.
    // They will be used when the TSPEC is deleted.
    tANI_U8 fDisableTx:1;
    tANI_U8 fDisableRx:1;
    tANI_U8 fUseBATx:1;
    tANI_U8 fUseBARx:1;

    // 1: expect to see frames with compressed BA coming from this peer MAC
    tANI_U8 fRxCompBA:1;
    tANI_U8 fTxCompBA:1;

    // immediate ACK or delayed ACK for frames from this peer MAC
    tANI_U8 fRxBApolicy:1;

    // immediate ACK or delayed ACK for frames to this peer MAC
    tANI_U8 fTxBApolicy:1;

    //Initiator or recipient
    tANI_U8 role;

    //Max # of MSDU received from this STA, negotiated at ADDBA
    // used for maintaining block ack state info
    tANI_U16 rxBufSize;

    //Max # of MSDU send to this STA, negotiated at ADDBA
    tANI_U16 txBufSize;

    //BA timeout negotiated at ADDBA. Unit: TU
    tANI_U16 tuTxBAWaitTimeout; //Time for Tx to wait for BA. 0 means no timeout

    tANI_U16 tuRxBAWaitTimeout; //Time for Rx to wait for explicit/implicit BAR. 0 means no timeout

} tCfgTrafficClass;

// TSPEC table entry
typedef struct
{
    tANI_BOOLEAN     valid;                           // Used/free flag
    tANI_U16         staIdx;        // STA-ID for this TSPEC
    tANI_U16         tspecIdx; //Index into the Global TSPEC table    
    tSirMacTSInfo      tsinfo;
} tTspecTblEntry, *tpTspecTblEntry;


// Routine to be invoked on addition of a TSPEC 
tSirRetStatus
halWmmAddTspec(
    tpAniSirGlobal  pMac,
    tAddTsParams *pTSParams);

// Routine to be invoked on deletion of a TSPEC
tSirRetStatus
halWmmDelTspec(
    tpAniSirGlobal  pMac,
    tDelTsParams *pTSParams);

eHalStatus
halWmmAggrAddTspec(
    tpAniSirGlobal  pMac,
    tAggrAddTsParams *pTSParams);

#endif    // #ifndef _HALMAC_WMM_API_H

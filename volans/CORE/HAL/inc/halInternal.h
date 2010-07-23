/** ------------------------------------------------------------------------- *
    ------------------------------------------------------------------------- *


    \file halInternal.h

    \brief Header file that contains all things needed internally to build the
    HAL modules

    $Id$

    Copyright (C) 2006 Airgo Networks, Incorporated


   ========================================================================== */
#ifndef HALINTERNAL_H__
#define HALINTERNAL_H__


// Common includes for all of the HAL/PAL modules...
//#include "halCommonApi.h"
#include "halTypes.h"
#include "palApi.h"

//legacy Hal support
#include "halDataStruct.h"
#include "halTypes.h"
#include "halPhy.h"
#include "aniGlobal.h"
#include "halNvApi.h"
#include "halQFuseApi.h"
#include "halMailbox.h"

#include "halBmu.h"
#include "halMIF.h"
#include "halPMU.h"
#include "halRxp.h"
#include "halMCU.h"
#include "halDPU.h"
#include "halMTU.h"
#include "halDXE.h"
#include "halAdu.h"
#include "halMCPU.h"
#include "halInterrupts.h"
#include "halRateAdaptApi.h"
#include "halStaTable.h"
#include "halStaTableApi.h"
#include "halRateTable.h"
#include "halMsgApiInternal.h"
#include "halMemoryMap.h"
//#include "halTxRx.h"
#include "phyGlobal.h"

//#include "hal.h"
#include "halStaTable.h"

#include "utilsApi.h"
#include "halMsgApi.h"
#include "halHddApis.h"
#include "vos_types.h"

#define PMAC_STRUCT( _hHal )  (  (tpAniSirGlobal)_hHal )

typedef enum
{
    eDONT_UPDATE_SMAC_CFG = 0,
    eUPDATE_SMAC_CFG
} eUpdateSmacCfg;

eHalStatus    halGlobalInit( tpAniSirGlobal pMac );
tSystemRole   halGetSystemRole( tpAniSirGlobal );
void          halSetSystemRole( tpAniSirGlobal, tSystemRole );
eHalStatus halSetPowerSaveMode(tpAniSirGlobal pMac, tSirMacHTMIMOPowerSaveState powerState);
eHalStatus    halInitEdcaProfile(tHalHandle hHal, void *arg);
eHalStatus    halPhy_ChangeChannel(tpAniSirGlobal pMac, tANI_U8 newChannel, ePhyChanBondState cbState, tANI_U8 calRequired, funcHalSetChanCB pFunc, void* pData, tANI_U16 dialog_token);
void halPhy_HandleSetChannelRsp(tHalHandle hHal,  void* pFwMsg);
void halSetBcnRateIdx(tpAniSirGlobal pMac, tTpeRateIdx rateIndex);
void halSetNonBcnRateIdx(tpAniSirGlobal pMac, tTpeRateIdx rateIndex);
void halSetMulticastRateIdx(tpAniSirGlobal pMac, tTpeRateIdx rateIndex);
void halGetBcnRateIdx(tpAniSirGlobal pMac, tTpeRateIdx *pRateIndex);
void halGetNonBcnRateIdx(tpAniSirGlobal pMac, tTpeRateIdx *pRateIndex);
eHalStatus hal_SendDummyInitScan(tpAniSirGlobal pMac, tANI_BOOLEAN setPMbit);
eHalStatus hal_SendDummyFinishScan(tpAniSirGlobal pMac);
tSirRetStatus halConfigCalPeriod(tpAniSirGlobal pMac);
tSirRetStatus halConfigCalControl(tpAniSirGlobal pMac);
tSirRetStatus halPerformTempMeasurement(tpAniSirGlobal pMac);
tANI_BOOLEAN halIsSelfHtCapable(tpAniSirGlobal pMac);
eHalStatus halGetDefaultAndMulticastRates(tpAniSirGlobal pMac, eRfBandMode rfBand, tTpeRateIdx* pRateIndex, tTpeRateIdx* pMcastRateIndex);

void  halRate_changeStaRate(tpAniSirGlobal pMac, tANI_U32 staid, tANI_U32 chnl, tHalMacRate halPriRateIdx, tHalMacRate halSecRateIdx,  tHalMacRate halTerRateIdx );
eHalStatus halRate_sendStaRateInfoMsg(tHalHandle hHal, tANI_U32 startStaIdx, tANI_U32 staCount, tANI_U32 nextReportPktCount, tANI_U32 nextReportMsec);
void  halRateDbg_changeStaRate(tpAniSirGlobal pMac,tANI_U32 chnl, tANI_U32 staid,  tANI_U32 halPriRateIdx, tANI_U32 halSecRateIdx,  tANI_U32 halTerRateIdx );


tANI_U32 getInternalMemory(tpAniSirGlobal pMac);
tANI_U32 halTlPostMsgApi(tpAniSirGlobal pMac, tSirMsgQ *pMsg);

//dummy function for now to register to BAL as fatal error callback.
VOS_STATUS halFatalErrorHandler(v_PVOID_t pVosGCtx, v_U32_t errorCode);

#endif



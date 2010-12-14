/**
 *
 *  @file:         halCommonApi.h
 *
 *  @brief:       Exports and types for the Hardware Abstraction Layer interfaces.
 *                   This file contains all the interfaces for thge Hardware Abstration Layer
 *                   functions.  It is intended to be included in all modules that are using
 *                   the external HAL interfaces.
 *
 *  @author:    Susan Tsao
 *
 *  Copyright (C) 2008, Qualcomm, Inc. All rights reserved.
 *
 *  Change History:
 * ---------------------------------------
 * 02/06/2006  File created.
 * 01/24/2008  Virgo related chages.
 */

#ifndef HALCOMMONAPI_H__
#define HALCOMMONAPI_H__

// MAC header files
#include "halTypes.h"
#include "halPhy.h"

#include "halMacWmmApi.h"
#include "halMacUtilsApi.h"
#include "halMsgApi.h"
#include "halMacSecurityApi.h"
#include "halTxRx.h"
#include "halBDApi.h"
#include "halMacBA.h"
#include "halHddApis.h"
#include "halFw.h"

#define HAL_MMH_MB_MSG_TYPE_MASK    0xFF00
#define LIBRA_CHIP_REV_ID_1_0       0x0
#define LIBRA_CHIP_REV_ID_2_0       0x1

extern tpSirBoardCapabilities halGetBoardCapabilities(void *pMacGlobal);
extern tSirRetStatus halMmhPostMsgApi(tpAniSirGlobal, tSirMsgQ* , tANI_U8);

/* ----------------- Global and per BSS System Role ---------- */
extern tBssSystemRole halGetBssSystemRole(tpAniSirGlobal pMac, tANI_U8 bssIdx);
extern void halSetBssSystemRole(tpAniSirGlobal pMac, tBssSystemRole role, 
    tANI_U8 bssIdx);
extern tBssSystemRole halGetGlobalSystemRole(tpAniSirGlobal pMac);
extern void halSetGlobalSystemRole(tpAniSirGlobal pMac, tBssSystemRole role);
extern tBssSystemRole halGetSystemRoleFromStaIdx(tpAniSirGlobal pMac, tANI_U8 staIdx);
extern tBssSystemRole halGetBssSystemRoleFromStaIdx(tpAniSirGlobal pMac, tANI_U8 staIdx);

/** =========================================================================
    Function prototypes
    =========================================================================*/

/** ------------------------------------------------------------------------- * 
    ------------------------------------------------------------------------- *  

    \func halOpen

    \brief - Open an instance of the HAL and startup basic HAL functionality.  

    \param tHalHandle * - pointer to the HAL handle. This is where the 
    HAL will store the tHalHandle.  Upon successful return from halOpen,
    this tHalHandle will be a handle to an open HAL instance.
        
    \param tHddHandle  - an opaque "handle" to the Hdd interface.  This handle
    is required input to the HAL.  This handle will be used on any calls from 
    the MAC/HAL into the HDD (including the PAL).

    \return eHalStatus
    
        eHAL_STATUS_SUCCESS - 

        eHAL_FAILURE - general failure not covered by the following failing
        return codes.
        
        eHAL_STATUS_CARD_NOT_PRESENT - The card associated with this instance
        of the driver has been removed and is not accessible.
    
    This is the first call that needs to be made into the HAL Api to get the
    HAL to initialize enough resources that we can talk to the chip through
    basic register and interrupt interfaces.  After return from this function, 
    the caller can expect to interface to the chip through halReadRegister,
    halWriteRegister, halReadMemory, halWriteMemory, and ???? interrupt
    funcitons.  
    
    Note that the MAC has not been 'started' after return from halOpen.  A 
    subsequent call  to halStart will get the MAC started.
    
    -------------------------------------------------------------------------- */
eHalStatus halOpen( tpAniSirGlobal pMac, tHalHandle *pHalHandle, tHddHandle hHdd, tMacOpenParameters *pMacOpenParms );


/** ------------------------------------------------------------------------- * 
    ------------------------------------------------------------------------- *  

    \func halClose

    \brief - Close an instance of the HAL.  

    \param tHalHandle - handle to an open HAL instance.   This is the instance
    of the HAL that is being closed.
    
    \return eHalStatus
        eHAL_STATUS_SUCCESS - 

        eHAL_FAILURE - general failure not covered by the following failing
        return codes.
        
        eHAL_STATUS_NOT_OPEN -
    
    All handles to the HAL that were opened via halOpen need to be closed by 
    the caller.  Following the call to halClose, the halHandle is invalid and  
    all resources associated with the HAL instance will be freed.
    
    Note the basic functionality of the HAL such as Register, Memory, and 
    Interrupts will not function after halClose returns.
   
    -------------------------------------------------------------------------- */
eHalStatus halClose( tHalHandle hHal );

/** ------------------------------------------------------------------------- * 
    ------------------------------------------------------------------------- *  

    \func halStart

    \brief - Start the HAL (MAC, PHY and RF)  

    \param tHalHandle - handle to an open HAL instance.
    
    \param tHalMacStartParameters - parameters needed for the HAL Mac Start.
    
    \return eHalStatus
        eHAL_STATUS_SUCCESS - 
        
        eHAL_FAILURE - general failure not covered by the following failing
        return codes.
        
        eHAL_STATUS_NOT_OPEN -
    
    
    ... description ... 
   
    -------------------------------------------------------------------------- */
eHalStatus halStart( tHalHandle hHal, tHalMacStartParameters *pHalMacStartParms );


/** ------------------------------------------------------------------------- * 
    ------------------------------------------------------------------------- *  

    \func halStop

    \brief - Stop the HAL (MAC, PHY and RF)  

    \param tHalHandle - handle to an open HAL instance.
    
    \param tHalMacStartParameters - parameters needed for the HAL Mac Start.
    
    \return eHalStatus
        eHAL_STATUS_SUCCESS - 

        eHAL_FAILURE - general failure not covered by the following failing
        return codes.
        
        eHAL_STATUS_NOT_OPEN -
    
    
    ... description ... 
   
    -------------------------------------------------------------------------- */
eHalStatus halStop( tHalHandle hHal , tHalStopType stopType);


/** ------------------------------------------------------------------------- * 
    ------------------------------------------------------------------------- *  

    \func halReset

    \brief - Reset the MAC.  

    \param tHalHandle - handle to an open HAL instance.
    
    \return eHalStatus
        eHAL_STATUS_SUCCESS - 
        
        eHAL_FAILURE - general failure not covered by the following failing
        return codes.
        
        eHAL_STATUS_NOT_OPEN -
    
    We haven't talked about this one but there is a need to be able to "Reset"
    the MAC.  This functionally will put the MAC back to it's initial state,
    same as it is after halStart is called.  All blocks in the MAC hardware
    and software are reinitialized back to their initial state. 
   
    -------------------------------------------------------------------------- */
eHalStatus halReset( tHalHandle hHal,  tANI_U32 rc);


extern tSirRetStatus halPostMsgApi(tpAniSirGlobal pMac, tSirMsgQ *pMsg);
extern void halCleanup(tpAniSirGlobal pMac);
extern void halPerformSystemReset(tHalHandle hMac);
void macSysResetReq(tpAniSirGlobal pMac, tANI_U32 rc);

extern void halHandleStatsReq(tpAniSirGlobal pMac, tANI_U16 msgType, tpAniGetStatsReq pMsg);
extern void halHandlePEStatisticsReq(tpAniSirGlobal pMac, tANI_U16 msgType, tpAniGetPEStatsReq pMsg);
extern void halMacClearDpuStats(tpAniSirGlobal pMac, tANI_U8 id);
extern void halMacClearStaStats(tpAniSirGlobal pMac, tANI_U8 staId);
extern void halMacPeriodicStatCollection(tpAniSirGlobal pMac);
extern void halMacWrapAroundStatCollection(tpAniSirGlobal pMac);
extern eHalStatus halMacCollectAndClearStaStats( tpAniSirGlobal pMac, tANI_U8 staIdx );


// -------------------------------------------------------------
/// MNT APIs

tSirRetStatus halMntPostMsgApi(tpAniSirGlobal, tSirMsgQ*);
tSirRetStatus halMntProcessMsgs(tpAniSirGlobal, tSirMsgQ*);
tSirRetStatus halMntGetPerStaStats(tpAniSirGlobal, tANI_U16);

// HAL APIS
//----
tSirRetStatus halProcessMsg(tpAniSirGlobal pMac, tSirMsgQ *pMsg);
void halFreeMsg( tpAniSirGlobal pMac, tSirMsgQ* pMsg);
tSirRetStatus halHandleMsg(tpAniSirGlobal pMac, tSirMsgQ *pMsg);
tSirRetStatus halPostMsgApi(tpAniSirGlobal, tSirMsgQ*);

tANI_U8 halStateGet(tpAniSirGlobal pMac);
void halStateSet(tpAniSirGlobal pMac, tANI_U8 state);

void halMntTempCheck(tpAniSirGlobal);
void halMntOpenTpcTempCheck(tpAniSirGlobal);
void halForceSetNwType(tpAniSirGlobal pMac, tSirNwType forceNwType);

tANI_BOOLEAN halIsLinkBusy(tpAniSirGlobal pMac);
tANI_U32 halGetChipRevNum(tpAniSirGlobal pMac);
tANI_U8 halGetCardType(tpAniSirGlobal pMac);
tANI_U8 halGetFrameTranslation(tpAniSirGlobal pMac);

eHalStatus halEnableListenMode(tpAniSirGlobal pMac, tANI_BOOLEAN listenModeEnable);   
eHalStatus halBmu_EnableIdleBdPduInterrupt(tpAniSirGlobal pMac, tANI_U8 threshold);
void halTLHandleIdleBdPduInterrupt(tpAniSirGlobal pMac);
#endif


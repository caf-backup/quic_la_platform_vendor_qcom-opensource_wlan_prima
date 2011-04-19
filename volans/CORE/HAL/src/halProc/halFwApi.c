/*
 * File:        halFwApi.c
 * Description: This file contains all the interface functions to
 *              interact with the firmware
 *
 * Copyright (c) 2008 QUALCOMM Incorporated.
 * All Rights Reserved.
 * Qualcomm Confidential and Proprietary
 *
 *
 * History:
 *
 * When       Who         What/Where/Why
 * -------------------------------------------------------------------
 * 07/21/2008 lawrie      Created the functions for sending message to
 *                        FW, configuring FW sys config.
 *
 *
 */

#include "palApi.h"
#include "halDebug.h"
#include "halInterrupts.h"
#include "halFwApi.h"
#include "halPwrSave.h"
#include "halFw.h"
#include "halMailbox.h"
#include "halPhyVos.h"

#ifdef FEATURE_INNAV_SUPPORT
#include "halInNav.h"
#endif

#ifndef VERIFY_HALPHY_SIMV_MODEL

/* Static Functions */
static eHalStatus halFW_DownloadImage(tpAniSirGlobal pMac, void* arg);
//static void halFW_RspTimeoutFunc(void* pData);

/*
 * DESCRIPTION:
 *      Firmware Initialization funtion for initializing memory map,
 *
 * PARAMETERS:
 *      hHal:   (pMac)Pointer to the global adapter context
 *
 * RETURN:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */
eHalStatus halFW_Init(tHalHandle hHal, void *arg)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal) hHal;
    tHalFwParams   *pFw = &pMac->hal.FwParam;
    eHalStatus status   = eHAL_STATUS_SUCCESS;
    Qwlanfw_SysCfgType *pFwConfig = NULL;

    pFw->fwSysConfigAddr = QWLANFW_MEM_SYS_CONFIG_ADDR_OFFSET;

    pMac->hal.memMap.fwSystemConfig_offset = QWLANFW_MEM_SYS_CONFIG_ADDR_OFFSET;
    pMac->hal.memMap.fwSystemConfig_size = QWLAN_FW_SYS_CONFIG_MMAP_SIZE;
    pMac->hal.halMac.isFwInitialized = eANI_BOOLEAN_FALSE;

    status = palAllocateMemory( pMac, (void **) &pFwConfig,
            sizeof(Qwlanfw_SysCfgType));
    if (status != eHAL_STATUS_SUCCESS) {
        HALLOGE( halLog( pMac, LOGE, FL("Memory Allocation Failed!!!") ));
        return status;
    }

    pFw->pFwConfig = pFwConfig;

    // Zero out the system config parameters
    palZeroMemory( pMac, pFw->pFwConfig, sizeof(Qwlanfw_SysCfgType));

    // Initialize the number of TX/RX antennas
    pFwConfig->ucNumTxAntennas = HAL_FW_NUM_TX_ANTENNAS;
    pFwConfig->ucNumRxAntennas = HAL_FW_NUM_RX_ANTENNAS;
    pFwConfig->ucOpenLoopTxGain = pMac->hphy.phy.openLoopTxGain;

    pFwConfig->usOfdmCmdPwrOffset = pMac->hphy.nvCache.tables.ofdmCmdPwrOffset.ofdmPwrOffset;
    pFwConfig->usTxbbFilterMode = (tANI_U16)(pMac->hphy.nvCache.tables.txbbFilterMode.txFirFilterMode);

    // configure the current regulatory domain
    //pFwConfig->ucRegDomain = (tANI_U8)( halPhyGetRegDomain(hHal) );

    //pFwConfig->bClosedLoop = CLOSED_LOOP_CONTROL;

    pFwConfig->bRfXoOn = TRUE;

    // Start the FW image download
    status = halFW_DownloadImage(pMac, arg);

    return status;
}



eHalStatus halFw_PostFwRspMsg(tpAniSirGlobal pMac, void *pFwMsg)
{
    tSirMsgQ msg;

    msg.type     =  SIR_HAL_HANDLE_FW_MBOX_RSP;
    msg.reserved = 0;
    msg.bodyptr  = pFwMsg;
    msg.bodyval  = 0;

    if(halPostMsgApi(pMac,&msg) != eSIR_SUCCESS) {
        HALLOGE(halLog(pMac, LOGE, FL("Posting SIR_HAL_HANDLE_FW_MBOX_RSP msg failed")));
        return eHAL_STATUS_FAILURE;
    }

    HALLOGW(halLog(pMac, LOGW, FL("Posting SIR_HAL_HANDLE_FW_MBOX_RSP msg\n")));
    return eHAL_STATUS_SUCCESS;
}


/*
 * DESCRIPTION:
 *      Firmware exit function
 *
 * PARAMETERS:
 *      hHal:   (pMac)Pointer to the global adapter context
 *
 * RETURN:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */
eHalStatus halFW_Exit(tHalHandle hHal, void *arg)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal) hHal;
    tHalFwParams   *pFw = &pMac->hal.FwParam;
    eHalStatus status = eHAL_STATUS_SUCCESS;

    pMac->hal.halMac.isFwInitialized = eANI_BOOLEAN_FALSE;

    palFreeMemory(pMac, pFw->pFwConfig);

    return status;
}

#if 0
/*
 * DESCRIPTION:
 *      Callback function called on timer expiry of FW response message
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *
 * RETURN:
 *      VOID
 */
static void halFW_RspTimeoutFunc(void* pData)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)pData;

    // Should do a LOGP here, if the response timeout occured
    HALLOGP( halLog(pMac, LOGP, FL("CRITICAL: FW Init done confirmation timedout !!!\n") ));
}
#endif

/*
 * DESCRIPTION:
 *      Function to download the firmware image
 *
 * PARAMETERS:
 *      pMac:       Pointer to the global adapter context
 *
 * RETURN:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */
static eHalStatus halFW_DownloadImage(tpAniSirGlobal pMac, void *arg)
{
#ifdef FW_PRESENT
    tHalFwParams   *pFw = &pMac->hal.FwParam;
    eHalStatus status = eHAL_STATUS_FAILURE;
    tHalMacStartParameters *pStartParams;
    Qwlanfw_SysCfgType *pFwConfig = (Qwlanfw_SysCfgType *)pMac->hal.FwParam.pFwConfig;

    tANI_U8 *fwImage;
    tANI_U32 *tempPtr;
    tANI_U32 value = 0, length = 0;
#ifdef VERIFY_FW_DOWNLOAD
    tANI_U8 *verifyBuf;
#endif
    tANI_U32 len, offset,i;

    pStartParams = (tHalMacStartParameters *) arg;

    fwImage = pStartParams->FW.pImage;
    length  = pStartParams->FW.cbImage;

    if (fwImage == NULL) {
        HALLOGW( halLog(pMac, LOGW, FL("No FW biinary image => no download!\n") ));
        return eHAL_STATUS_SUCCESS;
    }

    //Put the ARM back in reset.
    halReadRegister(pMac,
                QWLAN_MCU_MCPU_RESET_CONTROL_REG, &value) ;

    value |= QWLAN_MCU_MCPU_RESET_CONTROL_MCPU_SOFT_RESET_MASK;

    halWriteRegister(pMac,
            QWLAN_MCU_MCPU_RESET_CONTROL_REG , value) ;

    value = 0;

    //Disable the aCPU 'wait for Interrupt' instruction generation at the boot address.
    halReadRegister(pMac, QWLAN_MCU_ACPU_CONTROL_REG, &value);
    //QWLAN_MCU_ACPU_CONTROL_MCU_ACPU_STANDBY_P_MASK should be cleared by hardware
    //assert(!(value & QWLAN_MCU_ACPU_CONTROL_MCU_ACPU_STANDBY_P_MASK));

    //SW must clear boot looping enable
    value &= ~QWLAN_MCU_ACPU_CONTROL_MCU_MCU_ACPU_BOOT_LOOPING_EN_MASK;
    halWriteRegister(pMac, QWLAN_MCU_ACPU_CONTROL_REG , value);
    value = 0;

    offset = 0;
    len = 0x800 ;/*2K*/

    /* Allocate 2K memory to store the firmware fragment and Endian conversion */
    status = palAllocateMemory( pMac, (void **) &tempPtr, len);
    if (status != eHAL_STATUS_SUCCESS) {
        HALLOGE( halLog( pMac, LOGE, FL("Memory Allocation Failed!!!") ));
        return status;
    }

#ifdef VERIFY_FW_DOWNLOAD
    if (eHAL_STATUS_SUCCESS != palAllocateMemory(pMac->hHdd, (void **)&verifyBuf, len))
    {
        /* Free the allocated memory */
        palFreeMemory(pMac, tempPtr);
        HALLOGE( halLog( pMac, LOGE, FL("Memory Allocation Failed!!!") ));
        return (eHAL_STATUS_FAILURE);   //couldn't allocate
    }
#endif

    while (length) {

      if (length < len)
          len = length;

      /* Copy firmware fragment */
      palCopyMemory(pMac, (void*)tempPtr, (void*)((tANI_U8 *)fwImage + offset), len);

      /* Swap the bytes before writing to HW */
      for (i=0; i<len; i+=4) {
          *tempPtr = sirSwapU32(*tempPtr);
          tempPtr+=1;
      }

      tempPtr -= i/4;

      halWriteDeviceMemory(pMac, FW_IMAGE_MEMORY_BASE_ADDRESS + offset,
               tempPtr, len);

#ifdef VERIFY_FW_DOWNLOAD
      if (halReadDeviceMemory(pMac, FW_IMAGE_MEMORY_BASE_ADDRESS + offset, verifyBuf, len) != eHAL_STATUS_SUCCESS)
      {
          /* Free the allocated memory */
          palFreeMemory(pMac, tempPtr);
          palFreeMemory(pMac, verifyBuf);
          return eHAL_STATUS_FAILURE;
      }
      else
      {
          if (vos_mem_compare((tANI_U8 *)tempPtr, verifyBuf, len) != VOS_TRUE)
          {
              // !VERIFY ERROR
              halLog( pMac, LOGE, FL("Verify ERROR\n"));
              /* Free the allocated memory */
              palFreeMemory(pMac, tempPtr);
              palFreeMemory(pMac, verifyBuf);
              return (eHAL_STATUS_FAILURE);
          }
      }
#endif
      offset += len;
      length -= len;
    }
    /* Free the allocated memory */
    palFreeMemory(pMac, tempPtr);
#ifdef VERIFY_FW_DOWNLOAD
    palFreeMemory(pMac, verifyBuf);
#endif

    // Update the Traffic monitoring
    pFwConfig->ucRegulateTrafficMonitorMsec = QWLANFW_TRAFFIC_MONITOR_TIMEOUT_MSEC;
#ifdef WLAN_SOFTAP_FEATURE
    pFwConfig->ucApLinkMonitorMsec = QWLANFW_AP_LINK_MONITOR_TIMEOUT_MSEC;
    pFwConfig->ucUnknownAddr2CreditIntvMsec = QWLANFW_UNKNOWN_ADDR2_NOTIFCATION_INTERVAL_MS;
#endif
    pFwConfig->usBdPduOffset = QWLANFW_NUM_BDPDU_THRESHOLD;

    pFwConfig->ucMaxBss = HAL_NUM_BSSID;
    pFwConfig->ucMaxSta = HAL_NUM_STA;

#ifdef WLAN_SOFTAP_FEATURE
    pFwConfig->uBssTableOffset = pMac->hal.memMap.bssTable_offset;
    pFwConfig->uStaTableOffset = pMac->hal.memMap.staTable_offset;
    pFwConfig->bFwProcProbeReqDisabled = 1;
    halZeroDeviceMemory(pMac,pMac->hal.memMap.bssTable_offset, pMac->hal.memMap.bssTable_size);
    halZeroDeviceMemory(pMac,pMac->hal.memMap.staTable_offset, pMac->hal.memMap.staTable_size);
#endif

    // Write the required system config parameters into the
    // device memory
    status = halFW_UpdateSystemConfig(pMac, pFw->fwSysConfigAddr,
            (tANI_U8*)pFwConfig, sizeof(Qwlanfw_SysCfgType));
    if (status != eHAL_STATUS_SUCCESS) {
        HALLOGE( halLog( pMac, LOGE, FL("FW system config update failed!!!") ));
        return status;
    }

    {
        Qwlanfw_CalControlBitmask calControl =
        {
#ifdef BYTE_ORDER_BIG_ENDIAN
            1/* Channel Number to tune to after cal */,
            0/* Reserved */,
            1/* Uses 6 gain settings from process monitor table which are associated with Tx gain LUTs */,

            1/* Channel Tune after cal */,
            1/* Temperature Measure Periodically */,
            1/* Temperature Measure at Init */,
            0/* Tx DPD */,
            0/* CLPC Temp Adjustment */,
            0/* CLPC */,
            1/* Rx IQ */,
            0/* Rx GM-stage linearity */,

            1/* Tx IQ */,
            1/* Tx Lo Leakage */,
            1/* Rx DCO */,
            0/* RxDCO + IM2 Cal w/ Noise */,
            1/* RxDCO + IM2 Cal w/ Rx Tone Gen */,
            0/* LNA Gain adjust */,
            0/* LNA Band tuning */,
            0/* LNA Bias Setting */,

            1/* PLL VCO Freq Linearity Cal */,
            1/* Process Monitor */,
            1/* C Tuner */,
            0/* In-Situ Tuner */,
            1/* Rtuner */,
            1/* HDET DCO */,
            1/* Firmware Initialization */,
            0/* Use cal data prior to calibrations */

#else
            0/* Use cal data prior to calibrations */,
            1/* Firmware Initialization */,
            1/* HDET DCO */,
            1/* Rtuner */,
            0/* In-Situ Tuner */,
            1/* C Tuner */,
            1/* Process Monitor */,
            1/* PLL VCO Freq Linearity Cal */,

            0/* LNA Bias Setting */,
            0/* LNA Band tuning */,
            0/* LNA Gain adjust */,
            1/* RxDCO + IM2 Cal w/ Rx Tone Gen */,
            0/* RxDCO + IM2 Cal w/ Noise */,
            1/* Rx DCO */,
            1/* Tx Lo Leakage */,
            1/* Tx IQ */,

            0/* Rx GM-stage linearity */,
            1/* Rx IQ */,
            0/* CLPC */,
            0/* CLPC Temp Adjustment */,
            0/* Tx DPD */,
            1/* Temperature Measure at Init */,
            1/* Temperature Measure Periodically */,
            1/* Channel Tune after cal */,

            1/* Uses 6 gain settings from process monitor table which are associated with Tx gain LUTs */,
            0/* Reserved */,
            1/* Channel Number to tune to after cal */
#endif
        };

        if (RF_CHIP_VERSION(RF_CHIP_ID_VOLANS1))
        {
            calControl.rTuner = 0;
            calControl.cTuner = 0;
            calControl.tempMeasureAtInit = 0;
            calControl.processMonitor = 0;
        }

        /* Configure calControl Bitmask */
        halWriteDeviceMemory(pMac, QWLANFW_MEM_PHY_CAL_CONTROL_BITMAP_ADDR_OFFSET,
                                (tANI_U8 *)&calControl, sizeof(calControl));
    }

    /* Update the CalMemory with values from NV table */
    {
        uNvTables   nvTables;
        tANI_U32    len, *tempPtr = (tANI_U32 *)&(nvTables.rFCalValues.calData);

        halReadNvTable((tHalHandle)pMac, NV_TABLE_RF_CAL_VALUES, &nvTables);

        len = sizeof(nvTables.rFCalValues.calData);

        halWriteDeviceMemory(pMac, QWLANFW_MEM_PHY_CAL_CORRECTIONS_ADDR_OFFSET,
                                (tANI_U8 *)tempPtr, len);
    }

    // Take the ARM out of reset after FW is downloaded
    halReadRegister(pMac,
            QWLAN_MCU_MCPU_RESET_CONTROL_REG, &value);

    value &= ~QWLAN_MCU_MCPU_RESET_CONTROL_MCPU_SOFT_RESET_MASK;

    halWriteRegister(pMac,
            QWLAN_MCU_MCPU_RESET_CONTROL_REG , value) ;

    // Initialize Fw rsp status as failure now, this status should be set as
    // success on receiving the INIT_DONE message from FW after downloading the
    // image
    pFw->fwRspStatus = eHAL_STATUS_FAILURE;
#endif //FW_PRESENT
    return eHAL_STATUS_SUCCESS;
}

/*
 * DESCRIPTION:
 *      Function to handle the INIT DONE message from FW
 *      after successfull FW image download
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *
 * RETURN:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */
static eHalStatus halFW_InitComplete(tpAniSirGlobal pMac, void* pMsgInfo, eHalStatus fwStatus)
{
    eHalStatus      status = eHAL_STATUS_SUCCESS;
#ifdef FW_PRESENT
    tHalFwParams   *pFw = &pMac->hal.FwParam;

     if(fwStatus != eHAL_STATUS_SUCCESS) {
        HALLOGP( halLog( pMac, LOGP, FL(" FW Init after download FAILED!!!") ));
        pFw->fwRspStatus = eHAL_STATUS_FAILURE;
    } else {
        pMac->hal.halMac.isFwInitialized = eANI_BOOLEAN_TRUE;
        pFw->fwRspStatus = eHAL_STATUS_SUCCESS;
    }

    // Check if FW has additional information along with the message
    if (pMsgInfo != NULL) {
        StatusMsgInfo *pStatusMsg = (StatusMsgInfo *)(pMsgInfo);
        if(pStatusMsg->uStatusCode == QWLANFW_STATUS_FW_VERSION)
        {
            HALLOGE( FwVersionInfo *pVer = (FwVersionInfo *)(pStatusMsg->aStatusInfo));
            HALLOGE( halLog( pMac, LOGE, FL("**FW VERSION: MJ %d MN %d PT %d BD %d**\n"),
                        pVer->uMj, pVer->uMn, pVer->uPatch, pVer->uBuild));

           vos_mem_copy((v_VOID_t*)&pFw->fwVersion,(v_VOID_t*)pStatusMsg->aStatusInfo,sizeof(FwVersionInfo));
        }
        if ((status = halPhyFwInitDone((tHalHandle)pMac)) != eHAL_STATUS_SUCCESS)
        {
            HALLOGP(halLog (pMac, LOGP, FL("Could not initialize the halPhy module post fwinit\n")));
        }
    }
#endif //FW_PRESENT
    return status;
}

/*
 * DESCRIPTION:
 *      Function to handle the check whether FW has sent the INIT DONE message
 *      after successfull FW image download
 *
 * PARAMETERS:
 *      hHal:   (pMac)Pointer to the global adapter context
 *      arg:    Pointer to any argument to be passed
 *
 * RETURN:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */
eHalStatus halFW_CheckInitComplete(tHalHandle hHal, void *arg)
{
#ifdef FW_PRESENT
    tpAniSirGlobal pMac = (tpAniSirGlobal) hHal;
    eHalStatus status = eHAL_STATUS_FAILURE;
    tANI_U32 value = 0, i;
    tANI_U32 readCount = 0, writeCount = 0;

    for(i=0; i<HAL_MB_REG_READ_POLL_COUNT; i++) {
        // Poll for the read count of the Mbox register
        halReadRegister(pMac, QWLAN_MCU_MB1_CONTROL_COUNTERS_REG, &value);

        // Get the write count and the read count of the mailbox
        readCount = (value & QWLAN_MCU_MB1_CONTROL_COUNTERS_READ_CNT_MASK) >> QWLAN_MCU_MB1_CONTROL_COUNTERS_READ_CNT_OFFSET;
        writeCount = (value & QWLAN_MCU_MB1_CONTROL_COUNTERS_WRITE_CNT_MASK) >> QWLAN_MCU_MB1_CONTROL_COUNTERS_WRITE_CNT_OFFSET;

        // Is there any message received?
        if (writeCount > readCount) {
            // Parse the message type of the received message
            status = halMbox_RecvMsg(hHal, eANI_BOOLEAN_TRUE);
            break;
        }
        // Introduce a wait of 20ms
        vos_sleep(20);
    }

    /*FIXME_GEN6*/

    /* After FW is downloaded, and init done message is handled,
     * clear the MB related interrupt and ASIC interrupt here.
     * This may require cleaner way of clearing the ASIC interrupt.
     */
    halWriteRegister(pMac,
                     QWLAN_MCU_MAC_HOST_INT_CLEAR_REG,
                     QWLAN_MCU_MAC_HOST_INT_CLEAR_ACPU_TO_HOST_MB1_INT_CLEAR_MASK);

    halIntClearStatus(hHal, eHAL_INT_SIF_ASIC);

    if ((status != eHAL_STATUS_SUCCESS) || (i >= HAL_MB_REG_READ_POLL_COUNT)) {
        HALLOGP(halLog (pMac, LOGP, FL("Could not receive FW INIT DONE mesg")));
    }
    else
    {
        /*
           send Mbox msg to fw to inform halRateInfo table was updated.
           This is required, because firmware will look into halRateInfoTable in shared
           memory and manipulates the table into fast accessible format.
           note: halRateInfotable itself was sent in halRate_Start(). At that time,
           this message can't be sent because firmware was not started yet.
        */
        halMacRaUpdateParamReq(pMac, RA_UPDATE_RATE_INFO, 0);
    }

    return status;
#else
    return eHAL_STATUS_SUCCESS;
#endif //FW_PRESENT
}


eHalStatus halFW_SendScanStartMesg(tpAniSirGlobal pMac)
{
    Qwlanfw_ScanStartType sScanStartNotification;
    tANI_U16 dialogToken = 0;
    eHalStatus status = halFW_SendMsg(pMac, HAL_MODULE_ID_BTC, QWLANFW_HOST2FW_SCAN_START, dialogToken,
                                 sizeof(sScanStartNotification), &sScanStartNotification, FALSE, NULL);
    if(status != eHAL_STATUS_SUCCESS) {
        VOS_ASSERT(eANI_BOOLEAN_FALSE == pMac->hal.halMac.isFwInitialized);
    }
    return status;
}

eHalStatus halFW_SendScanStopMesg(tpAniSirGlobal pMac){

    tANI_U16 dialogToken = 0;
    Qwlanfw_ScanEndType sScanEndNotification;
    eHalStatus status = halFW_SendMsg(pMac, HAL_MODULE_ID_BTC, QWLANFW_HOST2FW_SCAN_END, dialogToken,
                                 sizeof(sScanEndNotification), &sScanEndNotification, FALSE, NULL);
    if(status != eHAL_STATUS_SUCCESS) {
        VOS_ASSERT(eANI_BOOLEAN_FALSE == pMac->hal.halMac.isFwInitialized);
    }
    return status;
}


eHalStatus halFW_SendConnectionStatusMesg(tpAniSirGlobal pMac, tSirLinkState linkStatus)
{
    tANI_U8    uFwMesgType = QWLANFW_HOST2FW_MSG_TYPES_END + 1;
    union {
        Qwlanfw_ConnectionSetupStartType  sConnSetupStartNotify;
        Qwlanfw_ConnectionSetupEndType    sConnSetupEndNotify;
        Qwlanfw_ConnectionTerminatedType  sConnTerminatedNotify;
    } u;
    void *pMsg = NULL;
    tANI_U16 size = 0;

    switch(linkStatus){
        case eSIR_LINK_IDLE_STATE:
            pMsg = (void *)&u.sConnTerminatedNotify;
            size = sizeof(u.sConnTerminatedNotify);
            uFwMesgType = QWLANFW_HOST2FW_CONNECTION_NONE;
            break;
        case eSIR_LINK_PREASSOC_STATE:
            pMsg = (void *)&u.sConnSetupStartNotify;
            size = sizeof(u.sConnSetupStartNotify);
            uFwMesgType = QWLANFW_HOST2FW_CONNECTION_SETUP_START;
            break;

        case eSIR_LINK_POSTASSOC_STATE:
            pMsg = (void *)&u.sConnSetupEndNotify;
            size = sizeof(u.sConnSetupEndNotify);
            uFwMesgType = QWLANFW_HOST2FW_CONNECTION_SETUP_END;
            break;

        default:
            //only above three states are connection setup related.
            //assert here.
            VOS_ASSERT(0);
    }
    if(size){
        tANI_U16 dialogToken = 0;
        if(eHAL_STATUS_SUCCESS != halFW_SendMsg(pMac, HAL_MODULE_ID_BTC, uFwMesgType, dialogToken,
                                     size, pMsg, FALSE, NULL)){
            if(pMac->hal.halMac.isFwInitialized){
                //if FW already initialized, should never fail. Assert here.
                VOS_ASSERT(0);
            }

        }
    }
    return eHAL_STATUS_SUCCESS;
}


eHalStatus halFW_SendConnectionEndMesg(tpAniSirGlobal pMac)
{
    Qwlanfw_ConnectionSetupEndType    sConnSetupEndNotify;
    tANI_U8 uFwMesgType = QWLANFW_HOST2FW_CONNECTION_SETUP_END;
    void *pMsg = (void *)&sConnSetupEndNotify;
    tANI_U16 size = sizeof(sConnSetupEndNotify);

    if(eHAL_STATUS_SUCCESS != halFW_SendMsg(pMac, HAL_MODULE_ID_BTC, uFwMesgType, 0, size, pMsg, FALSE, NULL)) {
        VOS_ASSERT(0);
    }

    return eHAL_STATUS_SUCCESS;
}

/*
 * DESCRIPTION:
 *      Send Mbox message to FW
 *
 * PARAMETERS:
 *      pMac:       Pointer to the global adapter context
 *      senderId:   Module ID of the message sender
 *      msgType:    Message Type
 *      dialogToken:Token to be used for the message
 *      dataLen:    Length of the message data
 *      pData:      Pointer to the data
 *      respNeeded: Is FW response needed to this msg
 *      cbFunc:     Function pointer to a callback function
 *
 * RETURN:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */
eHalStatus halFW_SendMsg(tpAniSirGlobal pMac,
        tANI_U8 senderId, tANI_U8 msgType, tANI_U16 dialogToken, tANI_U16 msgLen,
        void *pMsg, tANI_U8 respNeeded, void *cbFunc)
{
#ifdef FW_PRESENT
    eHalStatus status = eHAL_STATUS_FAILURE;
    tMBoxMsgHdr *pMbox = NULL;

    // Do a sanity check on the message length
    if(pMsg == NULL || msgLen < sizeof(tMBoxMsgHdr))
    {
        HALLOGE( halLog(pMac, LOGE, FL("Invalid FW message length\n") ));
        return eHAL_STATUS_FAILURE;
    }


    pMbox = (tMBoxMsgHdr *)pMsg;

    // Fill in the message header
    pMbox->Ver = HAL_MAILBOX_VERSION;
    pMbox->MsgType = msgType;
    pMbox->MsgLen = msgLen;
    pMbox->MsgSerialNum = dialogToken;
    pMbox->SenderID = senderId;
    pMbox->RespNeeded = respNeeded;
    *((tHalFwMsgCallback *)&pMbox->Callback0) = (tHalFwMsgCallback)cbFunc;

    // Send the mailbox message
    status = halMbox_SendReliableMsg(pMac, (void *)pMsg);
    if(eHAL_STATUS_SUCCESS != status)
    {
        HALLOGE( halLog( pMac, LOGE, FL("halMbox_SendReliableMsg failed with status = %d\n"), status));
    }
    return status;
#else
    return eHAL_STATUS_SUCCESS;
#endif //FW_PRESENT
}



/*
 * DESCRIPTION:
 *      Update the re-init address in the FW system config space
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *      value:  Address value to be written into the system config
 *
 * RETURN:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */
eHalStatus halFW_UpdateReInitRegListStartAddr(tpAniSirGlobal pMac, tANI_U32 value)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tHalFwParams *pFw = &pMac->hal.FwParam;
    Qwlanfw_SysCfgType *pFwConfig;
#ifdef VOLANS_PHY_TX_OPT_ENABLED
    tHalRegBckup *pRegBckup = &pMac->hal.RegBckupParam;
#endif /* VOLANS_PHY_TX_OPT_ENABLED */


    pFwConfig = (Qwlanfw_SysCfgType *)pFw->pFwConfig;
    pFwConfig->uAduReinitAddress = value;
#ifdef VOLANS_PHY_TX_OPT_ENABLED
    pFwConfig->uPhyTxAduReinitAddress = pRegBckup->phyRFTxRegListStartAddr;
#endif /* VOLANS_PHY_TX_OPT_ENABLED */
    status = halFW_UpdateSystemConfig(pMac,
            pMac->hal.FwParam.fwSysConfigAddr, (tANI_U8 *)pFwConfig,
            sizeof(*pFwConfig));

    return status;
}

#endif
/*
 * DESCRIPTION:
 *      FW System Config update
 *
 * PARAMETERS:
 *      pMac:     Pointer to the global adapter context
 *      address:  Address in the device system config memory space
 *      data:     Data to be written in the system config
 *      size:     Size of the data
 *
 * RETURN:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */
eHalStatus halFW_UpdateSystemConfig(tpAniSirGlobal pMac,
        tANI_U32 address, tANI_U8* data, tANI_U32 size)
{
    tANI_U32 configEndAddr = 0;

    // Compute the end address of the system config space
    configEndAddr = pMac->hal.memMap.fwSystemConfig_offset +
                    pMac->hal.memMap.fwSystemConfig_size;

    // Check the address boundaries
    if((address <  pMac->hal.memMap.fwSystemConfig_offset) ||
            (address > configEndAddr)) {
        HALLOGE( halLog(pMac, LOGE, FL("Invalid System config address") ));
        return eHAL_STATUS_FAILURE;
    }

    // Check whether the size of the config is within the boundaries
    // assigned for system config
    if((address + size) > configEndAddr) {
        HALLOGE( halLog(pMac, LOGE, FL("Invalid size for updating system config[%X][%X]"), (address + size),configEndAddr  ));
        return eHAL_STATUS_FAILURE;
    }

    // Write it to the device memory
    return halWriteDeviceMemory(pMac, address, data, size);

}

/*
 * DESCRIPTION:
 *      Handle FW messages to the Host
 *
 * PARAMETERS:
 *      pMac:     Pointer to the global adapter context
 *      pFwMsg:   Pointer to the FW message
 *
 * RETURN:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */
/*  */
eHalStatus halFW_HandleFwMessages(tpAniSirGlobal pMac, void *pFwMsg)
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    tMBoxMsgHdr *pMsgHdr = (tMBoxMsgHdr*)pFwMsg;

    // Handle the type of FW message received
    switch(pMsgHdr->MsgType) {
#ifndef VERIFY_HALPHY_SIMV_MODEL
        case QWLANFW_FW2HOST_ENTER_IMPS_RSP:
            status = halPS_PostponeFwEnterImpsRsp(pMac, pFwMsg);
            break;

        case QWLANFW_FW2HOST_IMPS_EXITED:
            status = halPS_HandleFwImpsExited(pMac, pFwMsg);
            break;

        case QWLANFW_FW2HOST_ENTER_BMPS_RSP:
            status = halPS_HandleFwEnterBmpsRsp(pMac, pFwMsg);
            break;

        case QWLANFW_FW2HOST_EXIT_BMPS_RSP:
            status = halPS_HandleFwExitBmpsRsp(pMac, pFwMsg);
            break;

        case QWLANFW_FW2HOST_SUSPEND_BMPS_RSP:
            status = halPS_HandleFwSuspendBmpsRsp(pMac, pFwMsg);
            break;

        case QWLANFW_FW2HOST_RESUME_BMPS_RSP:
            status = halPS_HandleFwResumeBmpsRsp(pMac, pFwMsg);
            break;

        case QWLANFW_FW2HOST_ENTER_UAPSD_RSP:
            status = halPS_HandleFwEnterUapsdRsp(pMac, pFwMsg);
            break;

        case QWLANFW_FW2HOST_EXIT_UAPSD_RSP:
            status = halPS_HandleFwExitUapsdRsp(pMac, pFwMsg);
            break;

        case QWLANFW_FW2HOST_RSSI_NOTIFICATION:
            status = halPS_HandleFwRssiNotification(pMac, pFwMsg);
            break;

        case QWLANFW_FW2HOST_STATUS:
            status = halFW_HandleFwStatusMsg(pMac, pFwMsg);
            break;

        case QWLANFW_FW2HOST_PERFORM_PERIODIC_CAL:
            break;

#endif
        case QWLANFW_FW2HOST_CAL_UPDATE_RSP:
        case QWLANFW_FW2HOST_SET_CHAIN_SELECT_RSP:
        case QWLANFW_FW2HOST_SET_CHANNEL_RSP:
           status = halPhy_HandlerFwRspMsg(pMac, pFwMsg);
           break;

#ifdef WLAN_SOFTAP_FEATURE
        case QWLANFW_FW2HOST_DEL_STA_CONTEXT:
            status = halFW_HandleFwDelStaMsg(pMac, pFwMsg);
              break;
#endif
#ifdef FEATURE_INNAV_SUPPORT
        case QWLANFW_FW2HOST_INNAV_MEAS_RSP:
            status = halInNav_HandleFwStartInNavMeasRsp(pMac, pFwMsg);
            break;
#endif

        default:
            status = eHAL_STATUS_FW_MSG_INVALID;
            break;
    }

    return status;
}
#ifndef VERIFY_HALPHY_SIMV_MODEL
/*
 * DESCRIPTION:
 *      Function to handle all the FW status messages
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *      pFwMsg: Pointer to the FW message
 *
 * RETURN:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */
eHalStatus halFW_HandleFwStatusMsg(tpAniSirGlobal pMac, void* pFwMsg)
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    Qwlanfw_StatusMsgType *pFwStatusMsg = (Qwlanfw_StatusMsgType*)pFwMsg;
    tANI_U32 reasonCode = pFwStatusMsg->uStatus;
    void* pMsgInfo = NULL;

    HALLOG1( halLog( pMac, LOG1, FL("Received FW_STATUS message with reason code %d"),
            reasonCode));

    // Check if the FW has more info along with the status
    if(pFwStatusMsg->hdr.usMsgLen >  sizeof(Qwlanfw_StatusMsgType)) {
        pMsgInfo = (tANI_U8*)(pFwMsg) + sizeof(Qwlanfw_StatusMsgType);
    }

    switch(reasonCode) {
        case QWLANFW_STATUS_BMPS_ENTER_FAILED:
        case QWLANFW_STATUS_BMPS_NO_REPLY_TO_NULLFRAME_FROM_AP:
        case QWLANFW_STATUS_BMPS_NO_REPLY_TO_PSPOLLFRAME_FROM_AP:
            status = halPS_HandleFwBmpsStatusMsg(pMac, pFwMsg);
            break;

        case QWLANFW_STATUS_INITDONE:
            status = halFW_InitComplete(pMac, pMsgInfo, eHAL_STATUS_SUCCESS);
            break;

        case QWLANFW_STATUS_INIT_FAILURE:
            status = halFW_InitComplete(pMac, pMsgInfo, eHAL_STATUS_FAILURE);
            break;

        case QWLANFW_STATUS_BMPS_RSSI_DROPPED_BELOW_THRESHOLD:
            status = halPS_SendLowRssiInd(pMac);
            break;
        case QWLANFW_STATUS_BMPS_MAX_MISSED_BEACONS:
            status = halPS_SendBeaconMissInd(pMac);
            break;

        case QWLANFW_STATUS_NOT_ENOUGH_ADU_MEMORY:
            break;

        case QWLANFW_STATUS_IMPROPER_MSGLEN:
            break;

        case QWLANFW_STATUS_TOOMANY_FILTERS:
            break;

        case QWLANFW_STATUS_TOOMANY_PTRNS:
            break;

        case QWLANFW_STATUS_INCONSISTENT_FWSTATE:
            break;

        case QWLANFW_STATUS_FAILED_TOSEND_NULLDATA_FRM:
            break;

        default:
            HALLOGE( halLog( pMac, LOGE, FL("Reason code %d from FW not defined\n"),
                    reasonCode));
            break;
    }


    return status;
}



#define  HAL_FW_HEARTBEAT_MONITOR_TH        4
/**
 *  @brief : This is the timer handler to monitor FW HeartBeat.
 *  @param : pMac - Mac Global Handle.
 */
void halFW_HeartBeatMonitor(tpAniSirGlobal pMac)
{
#ifdef FW_PRESENT
    tANI_U32  uFwHeartBeat;
    tANI_U8   psState = 0;

    /** Don't check for heartbeat if the device is in a Idle mode power save.*/
    psState = halPS_GetState(pMac);
    if((psState == HAL_PWR_SAVE_IMPS_STATE) || (psState == HAL_PWR_SAVE_IMPS_REQUESTED)) {
        return;
    }

    /** Read the Error Info and Address from Error Statistics in FW.*/
    halReadDeviceMemory(pMac, QWLANFW_MEM_ERR_STATS_ADDR_OFFSET,
                (tANI_U8 *)&uFwHeartBeat, sizeof(tANI_U32));


    if (pMac->hal.halMac.fwHeartBeatPrev == uFwHeartBeat)
    {
        VOS_ASSERT(0);
        VOS_TRACE( VOS_MODULE_ID_HAL, VOS_TRACE_LEVEL_FATAL,
			"%s Firmware Not Responding!!!uFwHeartBeat %d", __func__, uFwHeartBeat);

        pMac->hal.halMac.fwMonitorthr++;
        if (pMac->hal.halMac.fwMonitorthr > HAL_FW_HEARTBEAT_MONITOR_TH)
        {
            HALLOGE(halLog(pMac, LOGE, FL("Firmware Not Responding. Trigger LOGP!\n")));
            macSysResetReq(pMac, eSIR_FW_EXCEPTION);
            return;
        }
    }
    else
    {
        pMac->hal.halMac.fwMonitorthr = 0;
        pMac->hal.halMac.fwHeartBeatPrev = uFwHeartBeat;
    }
#endif //FW_PRESENT
}


// Function to start the periodic CHIP monitor
void halFW_StartChipMonitor(tpAniSirGlobal pMac)
{
    tx_timer_activate(&pMac->hal.halMac.chipMonitorTimer);
}

// Function to stop the periodic CHIP monitor
void halFW_StopChipMonitor(tpAniSirGlobal pMac)
{
    tx_timer_deactivate(&pMac->hal.halMac.chipMonitorTimer);
}

#ifdef WLAN_SOFTAP_FEATURE
/*
 * DESCRIPTION:
 *      Function to handle the FW delete STA context messages
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *      pFwMsg: Pointer to the FW message
 *
 * RETURN:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */
eHalStatus halFW_HandleFwDelStaMsg(tpAniSirGlobal pMac, void* pFwMsg)
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    tQwlanfw_DeleteStaContextType *pFwDelStaMsg = (tQwlanfw_DeleteStaContextType *) pFwMsg;
    tpDeleteStaContext  pDeleteStaMsg;

    HALLOGE(halLog(pMac, LOGE, FL("DelStaInd aid=%d, stId=%d, rc=%d\n"),
            pFwDelStaMsg->assocId , pFwDelStaMsg->staIdx ,pFwDelStaMsg->uReasonCode ));

    if( eHAL_STATUS_SUCCESS != palAllocateMemory(pMac->hHdd, (void **)&pDeleteStaMsg, sizeof(tDeleteStaContext)) )
    {
       /* No need for LOGP since its not a fatal error if we
       * can't send the indication to LIM. And if palAllocateMemory()
       * fails again, the next LOGP will take care of it.
       */
       HALLOGE( halLog(pMac, LOGE, FL("Failed to allocate memory \r\n")));
       status = eHAL_STATUS_FAILURE;
    }
    else
    {
       pDeleteStaMsg->assocId    = (tANI_U16)pFwDelStaMsg->assocId;
       pDeleteStaMsg->staId      = (tANI_U16)pFwDelStaMsg->staIdx;
       pDeleteStaMsg->reasonCode = (tANI_U16)pFwDelStaMsg->uReasonCode;

       if( eHAL_STATUS_SUCCESS != halTable_FindAddrByBssid(pMac, (tANI_U8)pFwDelStaMsg->bssIdx, pDeleteStaMsg->bssId))
       {
           HALLOGE(halLog(pMac, LOGE, FL(" Failed to Delete STA bssIdx[%d] \r\n"), pFwDelStaMsg->bssIdx));
           status = eHAL_STATUS_FAILURE;
       }
       else
       {
           halMsg_GenerateRsp( pMac, SIR_LIM_DELETE_STA_CONTEXT_IND, (tANI_U16) 0, (void *)pDeleteStaMsg, 0);
           status = eHAL_STATUS_SUCCESS;
       }
    }

    return status;
}
#endif



#ifdef WLAN_SOFTAP_FEATURE

//Dinesh move the following defintions to the right place.
#define PROBE_RSP_MPDU_MAX_LEN 0x400 //Dinesh needs to revisit. this length should be less tx bd header and 4 bytes length field. than the lengh tat FW.
#define PROBE_RSP_MPDU_HDR_LEN 24

eHalStatus halFW_WriteProbeRspToMemory(tpAniSirGlobal pMac, tANI_U8 *probeRsp,
                                    tANI_U8 selfStaIdxBss, tANI_U16 probeRspIndex, tANI_U32 mpduLen)
{
    /** Update Beacon Memory */
    tANI_U32 probeRspOffset;
    tANI_U32 alignedLen;
    halTxBd_type txBd;
    tANI_U32 templateLen = 0;
    tpBssStruct pBss = &(((tpBssStruct) pMac->hal.halMac.bssTable)[0 /*probeRspIndex 0 for now*/]);

    if(mpduLen > PROBE_RSP_MPDU_MAX_LEN)
    {
        HALLOGE(halLog(pMac, LOGE, FL("the probeRsp mpdu length (%u) is greater than the max length (%u)"),
            mpduLen, PROBE_RSP_MPDU_MAX_LEN));
        return eHAL_STATUS_FAILURE;
    }
    vos_mem_zero(&txBd, sizeof(txBd));
    //Fill in txbd.
    txBd.fwTxComplete0 = 0;
    txBd.queueId = BTQM_QUEUE_SELF_STA_PROBE_RSP; /*Using Low priority Mgmt Q*/
    txBd.mpduHeaderOffset = sizeof(halTxBd_type);
    txBd.staIndex = pBss->bssSelfStaIdx; //self station index for the BSS.
    txBd.bdRate = 0x0;
    txBd.bd_ssn = 1;
    txBd.dpuRF = BMUWQ_BTQM_TX_MGMT;
    txBd.mpduHeaderLength = PROBE_RSP_MPDU_HDR_LEN;
    txBd.mpduLength = mpduLen;
    txBd.mpduDataOffset = sizeof(halTxBd_type)+ txBd.mpduHeaderLength;

    probeRspOffset = pMac->hal.memMap.probeRspTemplate_offset + (probeRspIndex * PROBE_RSP_TEMPLATE_MAX_SIZE);

    templateLen = mpduLen + sizeof(halTxBd_type);  //mpdu length + TX bd header

    halWriteDeviceMemory(pMac, probeRspOffset ,
                            (tANI_U8 *)&templateLen, 4); //4 //4 bytes template length.

    halWriteDeviceMemory(pMac, probeRspOffset + 4,
                            (tANI_U8 *)&txBd, sizeof(halTxBd_type)); //4 //TxBd


    //halWriteDevicememory requires length to be mulltiple of four and aligned to 4 byte boundry.
    alignedLen = ( mpduLen + 3 ) & ~3 ;

    // probeRsp body need to be swapped sicne there is another swap occurs while BAL writes
    // the probeRsp to Libra.
    sirSwapU32BufIfNeeded((tANI_U32*)probeRsp, alignedLen>>2);

    halWriteDeviceMemory(pMac, probeRspOffset + 4 + sizeof(halTxBd_type),
                            (tANI_U8 *)probeRsp, alignedLen );

    return eHAL_STATUS_SUCCESS;
}

eHalStatus halFW_MsgReq(tpAniSirGlobal pMac, tFwMsgTypeEnum msgType, tANI_U16 msgLen, tANI_U8* msgBody)
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    Qwlanfw_CommonMsgType msg;

    /* send message to firmware */
    msg.msgType = (tANI_U16)msgType;
    msg.msgLen = msgLen;

    if(msgLen)
       vos_mem_copy(&msg.u.addStaMsg, msgBody, msgLen);

    // Send common message down to FW.
    status = halFW_SendMsg(pMac, HAL_MODULE_ID_FW, QWLANFW_HOST2FW_COMMON_MSG,
            0, sizeof(Qwlanfw_CommonMsgType), &msg, FALSE, NULL);
    return status;
}

eHalStatus halFW_UpdateBeaconReq(tpAniSirGlobal pMac, tANI_U8 bssIdx,
                                           tANI_U16 timIeOffset)
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    Qwlanfw_UpdateBeaconMsgType msgBody;
    tBssInfo bssInfo;
    tpBssStruct pBss;
    halReadDeviceMemory(pMac, (pMac->hal.memMap.bssTable_offset + (bssIdx * BSS_INFO_SIZE)), &bssInfo, sizeof(bssInfo));
    if(bssIdx < HAL_NUM_BSSID)
    {
        pBss = &(((tpBssStruct) (pMac->hal.halMac.bssTable))[bssIdx]);
        if(pBss && pBss->valid)
        {
            msgBody.bssIdx = pBss->bssIdx;
            bssInfo.timIeOffset = timIeOffset;
            halWriteDeviceMemory(pMac, (pMac->hal.memMap.bssTable_offset + (bssIdx * BSS_INFO_SIZE)), &bssInfo, sizeof(bssInfo));
            /* send message to firmware */
            HALLOGE(halLog(pMac, LOGE, FL("Sending message QWLANFW_UPDATE_BEACON to FW for bssIdx = %u, bssTable Address = 0x%x\n"), pBss->bssIdx,
                    (pMac->hal.memMap.bssTable_offset + (bssIdx * BSS_INFO_SIZE))));
            status = halFW_MsgReq(pMac, QWLANFW_COMMON_UPDATE_BEACON, sizeof(Qwlanfw_UpdateBeaconMsgType), (tANI_U8 *)&msgBody);
        }
    }
    return status;
}

#ifdef WLAN_SOFTAP_FW_BA_PROCESSING_FEATURE
eHalStatus halFW_UpdateBAMsg(tpAniSirGlobal pMac, tANI_U8 staIdx, tANI_U8 queueId, tANI_U8 code)
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    Qwlanfw_UpdateBaMsgType msgBody;

    msgBody.staIdx  = staIdx;
    msgBody.queueId = queueId;
    msgBody.code    = code;

    status = halFW_MsgReq(pMac, QWLANFW_COMMON_UPDATE_BA, sizeof(Qwlanfw_UpdateBaMsgType), (tANI_U8 *)&msgBody);

    HALLOGE( halLog(pMac, LOGE, FL("Update BA %d:%d:%d\n"), msgBody.staIdx, msgBody.queueId, msgBody.code ));
    return status;
}
#endif

tANI_U8 mapHostToFwBssSystemRole(tpAniSirGlobal pMac, tBssSystemRole hostBssSystemRole)
{
    tANI_U8 fwRole = FW_SYSTEM_UNKNOWN_ROLE;
    switch(hostBssSystemRole)
    {
        case eSYSTEM_UNKNOWN_ROLE:
            fwRole = FW_SYSTEM_UNKNOWN_ROLE;
            break;

        case eSYSTEM_AP_ROLE:
            fwRole = FW_SYSTEM_AP_ROLE;
            break;

        case eSYSTEM_STA_IN_IBSS_ROLE:
            fwRole = FW_SYSTEM_STA_IN_IBSS_ROLE;
            break;

        case eSYSTEM_STA_ROLE:
            fwRole = FW_SYSTEM_STA_ROLE;
            break;

        case eSYSTEM_BTAMP_STA_ROLE:
            fwRole = FW_SYSTEM_BTAMP_STA_ROLE;
            break;

        case eSYSTEM_BTAMP_AP_ROLE:
            fwRole = FW_SYSTEM_BTAMP_AP_ROLE;
            break;

        case eSYSTEM_MULTI_BSS_ROLE:
            fwRole = FW_SYSTEM_MULTI_BSS_ROLE;
            break;
        default:
            HALLOGE(halLog(pMac, LOGE, FL("invalid bss system role = %d \n"), hostBssSystemRole));
            fwRole = FW_SYSTEM_UNKNOWN_ROLE;
            break;
    }
    return fwRole;
}
eHalStatus halFW_AddBssReq(tpAniSirGlobal pMac, tANI_U8 bssIdx)
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    tBssInfo bssInfo;
    Qwlanfw_AddBssMsgType msgBody;
    tpBssStruct pBss;
    tpStaStruct pSta;
    tHalFwParams *pFw = &pMac->hal.FwParam;
    Qwlanfw_SysCfgType *pFwConfig;

    pFwConfig = (Qwlanfw_SysCfgType *)pFw->pFwConfig;

    vos_mem_zero(&bssInfo, sizeof(bssInfo));
    if(bssIdx < HAL_NUM_BSSID)
    {
        pBss = &(((tpBssStruct) (pMac->hal.halMac.bssTable))[bssIdx]);
        if(pBss && pBss->valid)
        {
            //in AP mode enable link monitoring and unknown addr2 handling.
            if(pBss->bssSystemRole == eSYSTEM_AP_ROLE)
            {
                pFwConfig->fDisLinkMonitor = 0;
                pFwConfig->fEnableFwUnknownAddr2Handling = 1;
            }

            msgBody.bssIdx = pBss->bssIdx;
            bssInfo.bssIdx = pBss->bssIdx;
            bssInfo.selfStaIdx = pBss->bssSelfStaIdx;
            bssInfo.bcastStaIdx = pBss->bcastStaIdx;
            bssInfo.bssRole = mapHostToFwBssSystemRole(pMac, pBss->bssSystemRole);
            bssInfo.tuBcnIntv = pBss->tuBeaconInterval;
            bssInfo.hiddenSsid = pBss->hiddenSsid;
            bssInfo.valid = 1;
            pSta = &(((tpStaStruct) (pMac->hal.halMac.staTable))[pBss->staIdForBss]);
            bssInfo.selfStaAddrLo = *((tANI_U32*)pSta->staAddr);
            bssInfo.selfStaAddrHi = *((tANI_U16*)(pSta->staAddr + 4));

            vos_mem_copy(bssInfo.ssId, pBss->ssId.ssId, pBss->ssId.length);
            bssInfo.ssIdLen = pBss->ssId.length;
            //need to take care of multiple BSSid.
            bssInfo.bcnTemplateAddr = (tANI_U8*)pMac->hal.memMap.beaconTemplate_offset;
            bssInfo.probeRspTemplateAddr = (tANI_U8*)pMac->hal.memMap.probeRspTemplate_offset;
            halWriteDeviceMemory(pMac, (pMac->hal.memMap.bssTable_offset + (bssIdx * BSS_INFO_SIZE)) , &bssInfo, sizeof(bssInfo));
            status = halFW_UpdateSystemConfig(pMac,
                    pMac->hal.FwParam.fwSysConfigAddr, (tANI_U8 *)pFwConfig,
                    sizeof(*pFwConfig));

            if(eHAL_STATUS_SUCCESS == status)
            {
                HALLOGE(halLog(pMac, LOGE, FL("Sending message QWLANFW_COMMON_ADD_BSS to FW for bssIdx = %u, staIdxForBss = %u, bcastStaIdxForBss %u\n"),
                    pBss->bssIdx, pBss->bssSelfStaIdx, pBss->bcastStaIdx));
                /* send message to firmware */
                status = halFW_MsgReq(pMac, QWLANFW_COMMON_ADD_BSS, sizeof(Qwlanfw_AddBssMsgType), (tANI_U8 *)&msgBody);
            }
        }
    }
    return status;
}

eHalStatus halFW_DelBssReq(tpAniSirGlobal pMac, tANI_U8 bssIdx)
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    Qwlanfw_DelBssMsgType msgBody;
    tpBssStruct pBss;
    tHalFwParams *pFw = &pMac->hal.FwParam;
    Qwlanfw_SysCfgType *pFwConfig = (Qwlanfw_SysCfgType *)pFw->pFwConfig;
    if(bssIdx < HAL_NUM_BSSID)
    {
        pBss = &(((tpBssStruct) (pMac->hal.halMac.bssTable))[bssIdx]);
        if(pBss && pBss->valid)
        {
            //in AP is getting deleted disable link monitoring and unknown addr2 handling.
            if(pBss->bssSystemRole == eSYSTEM_AP_ROLE)
            {
                pFwConfig->fDisLinkMonitor = 1;
                pFwConfig->fEnableFwUnknownAddr2Handling = 0;
            }

             msgBody.bssIdx = bssIdx;
            /* send message to firmware */
            status = halFW_MsgReq(pMac, QWLANFW_COMMON_DEL_BSS, sizeof(Qwlanfw_DelBssMsgType), (tANI_U8 *)&msgBody);
            status = halFW_UpdateSystemConfig(pMac,
                    pMac->hal.FwParam.fwSysConfigAddr, (tANI_U8 *)pFwConfig,
                    sizeof(*pFwConfig));
        }
    }
    return status;
}

eHalStatus halFW_AddStaReq(tpAniSirGlobal pMac, tANI_U8 staIdx, tANI_U8 raGlobalUpdate, tANI_U8 raStaUpdate)
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    tStaInfo staInfo;
    Qwlanfw_AddStaMsgType msgBody;
    tpStaStruct pSta;
    tANI_U8 uSig;
    vos_mem_zero(&staInfo, sizeof(staInfo));
    vos_mem_zero(&msgBody, sizeof(Qwlanfw_AddStaMsgType));

    if(staIdx < HAL_NUM_STA)
    {
        pSta = &(((tpStaStruct)(pMac->hal.halMac.staTable))[staIdx]);
        if((pSta) && pSta->valid)
        {
            msgBody.staIdx = pSta->staId;
            staInfo.bssIdx = pSta->bssIdx;
            staInfo.staIdx = pSta->staId;
            staInfo.aid = (tANI_U8)pSta->assocId;
            staInfo.valid = 1;
            staInfo.delEnbQidMask = pSta->delEnbQidMask;
            staInfo.qosEnabled = pSta->qosEnabled;
            staInfo.peerEntry = ((pSta->staType == STA_ENTRY_PEER)?1:0);
            staInfo.macAddrLo = *((tANI_U32*)pSta->staAddr);
            staInfo.macAddrHi = *((tANI_U16*)(pSta->staAddr + 4));

            staInfo.dpuDescIndx = pSta->dpuIndex;

            halDpu_GetSignature(pMac, pSta->dpuIndex, &uSig);
            staInfo.dpuSig = uSig;

            msgBody.bssIdx = pSta->bssIdx;
            if(raGlobalUpdate)
            {
                tHalRaGlobalInfo *pGlobRaInfo = &pMac->hal.halRaInfo;

                /* Download the table to the target. */
                /* Note : As a workaround, update globalInfo here. This is supposed to be called
                               after initializtaion of RaGlobalInfo in halMacRaStart().
                               However, rtsThreshold and protPolicy is not initialized properly at that moment,
                               because Cfg is not initialized yet.
                            */
                halMacRaGlobalInfoToFW(pMac, pGlobRaInfo, 0, sizeof(tHalRaGlobalInfo));
                msgBody.raGlobalUpdate = raGlobalUpdate;
            }

            halWriteDeviceMemory(pMac, (pMac->hal.memMap.staTable_offset + (staIdx * STA_INFO_SIZE)), &staInfo, sizeof(staInfo));

            /* send message to firmware */
            HALLOGE(halLog(pMac, LOGE, FL("Sending message QWLANFW_COMMON_ADD_STA to FW for staId = %u\n"), pSta->staId));
            status = halFW_MsgReq(pMac, QWLANFW_COMMON_ADD_STA, sizeof(Qwlanfw_AddStaMsgType), (tANI_U8 *)&msgBody);
        }
    }
    return status;
}

eHalStatus halFW_DelStaReq(tpAniSirGlobal pMac, tANI_U8 staIdx)
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    Qwlanfw_DelStaMsgType msgBody;
//    tpStaStruct pSta;
    if(staIdx < HAL_NUM_STA)
    {
//Dinesh no need to check.
//        pSta = &(((tpStaStruct)(pMac->hal.halMac.staTable))[staIdx]);
//        if((pSta) && pSta->valid)
        {
             msgBody.staIdx = staIdx;
            /* send message to firmware */
            status = halFW_MsgReq(pMac, QWLANFW_COMMON_DEL_STA, sizeof(Qwlanfw_DelStaMsgType), (tANI_U8 *)&msgBody);
        }
    }
    return status;
}

/*
 * DESCRIPTION:
 *      Update bitmap for unhandled IEs in probeRsp by FW.
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *      value:  Address value to be written into the system config
 *
 * RETURN:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */
eHalStatus halFW_UpdateProbeRspIeBitmap(tpAniSirGlobal pMac, tANI_U32* pIeBitmap)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tHalFwParams *pFw = &pMac->hal.FwParam;
    Qwlanfw_SysCfgType *pFwConfig;

    pFwConfig = (Qwlanfw_SysCfgType *)pFw->pFwConfig;
    vos_mem_copy(pFwConfig->apProbeReqValidIEBitmap, pIeBitmap, sizeof(pFwConfig->apProbeReqValidIEBitmap));
    pFwConfig->bFwProcProbeReqDisabled = 0; //enable the feature;

    status = halFW_UpdateSystemConfig(pMac,
            pMac->hal.FwParam.fwSysConfigAddr, (tANI_U8 *)pFwConfig,
            sizeof(*pFwConfig));
    return status;
}

#endif
#endif //#ifndef VERIFY_HALPHY

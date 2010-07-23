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

    status = palAllocateMemory( pMac, (void *) &pFwConfig,
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

    //if the qFuse is blown, then update the closedLoop param
    if(halIsQFuseBlown(pMac) == eHAL_STATUS_SUCCESS)
    {
        pFwConfig->bClosedLoop = CLOSED_LOOP_CONTROL;
    }

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
    //tANI_U32 verifyLength;
    tANI_U32 len, offset,i;

    pStartParams = (tHalMacStartParameters *) arg;

    fwImage = pStartParams->FW.pImage;
    length  = pStartParams->FW.cbImage;
    //verifyLength = length;

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

    tempPtr = (tANI_U32 *)fwImage;
    for (i=0; i<length; i+=4) {
       *tempPtr = sirSwapU32(*tempPtr);
       tempPtr+=1;
    }
    HALLOGW( halLog(pMac, LOGW, FL("Download firmware image (%d bytes) \n"),  length ));

    offset = 0;
    len = 0x800 ;/*16K*/
    while (length) {
      if (length < len)
          len = length;

      halWriteDeviceMemory(pMac, FW_IMAGE_MEMORY_BASE_ADDRESS + offset,
               ((tANI_U8 *)fwImage + offset), len) ;
      offset += len;
      length -= len;
    }


/*  some code to verify that firmware was written correctly to the device memory
        {
            tANI_U8 *verifyBuf;

            if (eHAL_STATUS_SUCCESS == palAllocateMemory(pMac->hHdd, (void **)&verifyBuf, 0x4000))  //verify 16K byte chunks
            {

                offset = 0;
                len = 0x800;
                while (verifyLength)
                {
                    if (verifyLength < len)
                    {
                        len = verifyLength;
                    }

                    if (halReadDeviceMemory(pMac, FW_IMAGE_MEMORY_BASE_ADDRESS + offset, verifyBuf, len) != eHAL_STATUS_SUCCESS)
                    {
                        return eHAL_STATUS_FAILURE;
                    }
                    else
                    {
                        if (memcmp(((tANI_U8 *)fwImage + offset), verifyBuf, len) != 0)
                        {
                            // !VERIFY ERROR
                            halLog( pMac, LOGE, FL("Verify ERROR\n"));
                            return (eHAL_STATUS_FAILURE);
                        }
                    }

                    offset += len;
                    verifyLength -= len;
                }

                if (palFreeMemory(pMac->hHdd, verifyBuf) != eHAL_STATUS_SUCCESS)
                {
                    return (eHAL_STATUS_FAILURE);   //couldn't free
                }
            }
            else
            {
                return (eHAL_STATUS_FAILURE);   //couldn't allocate
            }
        }
*/

    // Update the Traffic monitoring
    pFwConfig->ucRegulateTrafficMonitorMsec = QWLANFW_TRAFFIC_MONITOR_TIMEOUT_MSEC;
    pFwConfig->usBdPduOffset = QWLANFW_NUM_BDPDU_THRESHOLD;

    pFwConfig->ucMaxBss = HAL_NUM_BSSID;
    pFwConfig->ucMaxSta = HAL_NUM_STA;

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
            1/* Channel Number to tune to after cal */, 0/* Reserved */,
            0/* Channel Tune after cal */, 0/* Temperature Measure Periodically */,
            0/* Temperature Measure at Init */, 0/* Tx DPD */,
            0/* CLPC Temp Adjustment */, 0/* CLPC */,
            1/* Rx IQ */, 0/* Rx GM-stage linearity */,
            1/* Tx IQ */, 1/* Tx Lo Leakage */,
            1/* Rx DCO */, 0/* RxDCO + IM2 Cal w/ Noise */,
            0/* RxDCO + IM2 Cal w/ Rx Tone Gen */, 0/* LNA Gain adjust */,
            0/* LNA Band tuning */, 0/* LNA Bias Setting */,
            0/* PLL VCO Freq Linearity Cal */, 0/* Process Monitor */,
            0/* C Tuner */, 0/* In-Situ Tuner */,
            0/* Rtuner */, 0/* HDET DCO */,
            1/* Firmware Initialization */, 0/* Use cal data prior to calibrations */

#else
            0/* Use cal data prior to calibrations */, 1/* Firmware Initialization */,
            0/* HDET DCO */, 0/* Rtuner */,
            0/* In-Situ Tuner */, 0/* C Tuner */,
            0/* Process Monitor */, 0/* PLL VCO Freq Linearity Cal */,
            0/* LNA Bias Setting */, 0/* LNA Band tuning */,
            0/* LNA Gain adjust */, 0/* RxDCO + IM2 Cal w/ Rx Tone Gen */,
            0/* RxDCO + IM2 Cal w/ Noise */, 1/* Rx DCO */,
            1/* Tx Lo Leakage */, 1/* Tx IQ */,
            0/* Rx GM-stage linearity */, 1/* Rx IQ */,
            0/* CLPC */, 0/* CLPC Temp Adjustment */,
            0/* Tx DPD */, 0/* Temperature Measure at Init */,
            0/* Temperature Measure Periodically */, 0/* Channel Tune after cal */,
            0/* Reserved */, 1/* Channel Number to tune to after cal */
#endif
        };

        /* Configure calControl Bitmask */
        halWriteDeviceMemory(pMac, QWLANFW_MEM_PHY_CAL_CONTROL_BITMAP_ADDR_OFFSET,
                                (tANI_U8 *)&calControl, sizeof(calControl));
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
    }
#endif //FW_PRESENT
    return eHAL_STATUS_SUCCESS;
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
            status = halMbox_RecvMsg(hHal);
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



eHalStatus halFW_SendConnectionStatusMesg(tpAniSirGlobal pMac, tSirLinkState linkStatus){
#if defined(LIBRA_WAPI_SUPPORT)
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
#endif
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
    if(pMsg == NULL && msgLen < sizeof(tMBoxMsgHdr)) {
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
        HALLOGE( halLog(pMac, LOGE, FL("Invalid size for updating system config") ));
        return eHAL_STATUS_FAILURE;
    }

    // Write it to the device memory
    halWriteDeviceMemory(pMac, address, data, size);

    return eHAL_STATUS_SUCCESS;
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
eHalStatus halFW_HandleFwMessages(tpAniSirGlobal pMac, void *pFwMsg, tANI_U8* bufConsumed)
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    tMBoxMsgHdr *pMsgHdr = (tMBoxMsgHdr*)pFwMsg;
    *bufConsumed = TRUE;

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
           status = halPhy_HandlerFwRspMsg(pMac, pFwMsg);
           break;

        case QWLANFW_FW2HOST_SET_CHANNEL_RSP:
#ifdef ANI_MANF_DIAG
           status = halPhy_HandlerFwRspMsg(pMac, pFwMsg);
#else
            status = halFw_PostFwRspMsg(pMac, pFwMsg);
            *bufConsumed = (status == eHAL_STATUS_SUCCESS) ? FALSE : TRUE;
#endif
            break;

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
        pMac->hal.halMac.fwMonitorthr++;
        if (pMac->hal.halMac.fwMonitorthr > HAL_FW_HEARTBEAT_MONITOR_TH)
        {
            HALLOGE(halLog(pMac, LOGE, FL("Firmware Not Responding. Invoking macSysReset...!!!\n")));
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
#endif

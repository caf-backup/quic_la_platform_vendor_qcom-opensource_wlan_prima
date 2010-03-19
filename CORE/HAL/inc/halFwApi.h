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

#ifndef _HALFWAPI_H_
#define _HALFWAPI_H_

#include "halTypes.h"
#include "aniGlobal.h"
#include "sirApi.h"
#include "vos_event.h"
#include "vos_status.h"
#include "vos_types.h"


// FW system config memory map
#define QWLAN_FW_SYS_CONFIG_MMAP_OFFSET  0x1200
#define QWLAN_FW_SYS_CONFIG_MMAP_SIZE    256

/* Increased this value for FPGA. Hence TBD*/

// FW response message timeout value
#define HAL_FW_RSP_TIMEOUT              10000 /*TBD*/

// FW download response timeout value
#define HAL_FW_DOWNLOAD_RSP_TIMEOUT     10000

#define HAL_MB_REG_READ_POLL_COUNT      50

// Number of TX/RX antennas
#define HAL_FW_NUM_TX_ANTENNAS      1
#define HAL_FW_NUM_RX_ANTENNAS      2

// Close loop TPC flag
#define CLOSED_LOOP_CONTROL         1

#define HAL_MODULE_ID_PHY       0
#define HAL_MODULE_ID_PWR_SAVE  1
#define HAL_MODULE_ID_BTC       2
#define HAL_MODULE_ID_RA        3


// Define FW endianess here
//#define FW_LITTLE_BYTE_ENDIAN       1
#undef FW_LITTLE_BYTE_ENDIAN

// Endianess conversion between HOST and FW
#ifdef FW_LITTLE_BYTE_ENDIAN
#define halConvertU16HostToFw(x)    ani_cpu_to_le16(x)
#define halConvertU32HostToFw(x)    ani_cpu_to_le32(x)
#define halConvertU16FwToHost(x)    ani_le16_to_cpu(x)
#define halConvertU32FwToHost(x)    ani_le32_to_cpu(x)
#else
#define halConvertU16HostToFw(x)    ani_cpu_to_be16(x)
#define halConvertU32HostToFw(x)    ani_cpu_to_be32(x)
#define halConvertU16FwToHost(x)    ani_be16_to_cpu(x)
#define halConvertU32FwToHost(x)    ani_be32_to_cpu(x)
#endif

/* HAL-FW parameters */
typedef struct sHalFwParams
{
    // Start address of the System config space in FW
    tANI_U32    fwSysConfigAddr;

    // Status of the FW response message to be stored
    tANI_U8     fwRspStatus;

    // Pointer to hold the Fw Config parameters
    void*       pFwConfig;

   // FW version info to be stored
   FwVersionInfo fwVersion;
} tHalFwParams;


/* Firmware Initialization/Exit Functions */
eHalStatus halFW_Init(tHalHandle hHal, void *arg);
eHalStatus halFW_Exit(tHalHandle hHal, void *arg);

/* Check Firmware init done */
eHalStatus halFW_CheckInitComplete(tHalHandle hHal, void *arg);

/* Send Mbox message to FW */
eHalStatus halFW_SendMsg(tpAniSirGlobal pMac,
        tANI_U8 senderId, tANI_U8 msgType, tANI_U16 dialogToken, tANI_U16 msgLen,
        void *pMsg, tANI_U8 respNeeded, void *cbFunc);

/* FW System Config update */
eHalStatus halFW_UpdateSystemConfig(tpAniSirGlobal pMac,
        tANI_U32 address, tANI_U8* data, tANI_U32 size);

/* Update the re-init address in the FW system config space */
eHalStatus halFW_UpdateReInitRegListStartAddr(tpAniSirGlobal pMac, tANI_U32 value);

/* Handle FW messages to the Host */
eHalStatus halFW_HandleFwMessages(tpAniSirGlobal pMac, void* pFwMsg, tANI_U8* bufConsumed);
eHalStatus halFw_PostFwRspMsg(tpAniSirGlobal pMac, void *pFwMsg);

/* Function to handle the FW status message */
eHalStatus halFW_HandleFwStatusMsg(tpAniSirGlobal pMac, void* pFwMsg);

/* timer handler to monitor FW HeartBeat */
void halFW_HeartBeatMonitor(tpAniSirGlobal pMac);

void halFW_StartChipMonitor(tpAniSirGlobal pMac);
void halFW_StopChipMonitor(tpAniSirGlobal pMac);
eHalStatus halFW_SendScanStartMesg(tpAniSirGlobal pMac);
eHalStatus halFW_SendScanStopMesg(tpAniSirGlobal pMac);
eHalStatus halFW_SendConnectionEndMesg(tpAniSirGlobal pMac);
eHalStatus halFW_SendConnectionStatusMesg(tpAniSirGlobal pMac, tSirLinkState linkStatus);


#endif //_HALFWAPI_H_

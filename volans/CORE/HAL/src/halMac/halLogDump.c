/*
 * Airgo Networks, Inc proprietary. All rights reserved.
 * halLogDump.c:  Contains logDump functions
 *
 * ---------------------------------------------------------------
 */
#include "halInternal.h"
#include "halDebug.h"
#include "halUtils.h"
#ifdef WLAN_HAL_VOLANS
#include "volansDefs.h"
#else
#include "libraDefs.h"
#endif
#include "cfgApi.h"
#include "logDump.h"
#include "halLogDump.h"
#include "halFw.h"
#include "halFwApi.h"
#include "halTpe.h"
#include "halRFBringup.h"
#include "halRegBckup.h"
#include "btcApi.h"
#include "halTLApi.h"
//halPhy header files
#include "asicApi.h"

#if (defined(ANI_OS_TYPE_ANDROID) || defined(ANI_OS_TYPE_LINUX))
#include <linux/firmware.h>
#include <linux/string.h>
#include <../../../HDD/inc/wlan_hdd_main.h>
#include <linux/mmc/sdio_func.h>


typedef struct
{
    char templateFileName[50];
    tANI_U32 template_type;
    tANI_U32 template_subtype;
    tANI_U32 template_resp_type;
    tANI_U32 template_resp_subtype;
    tANI_U32 template_ignore_expected_resp;
    tANI_U32 template_resp_is_expected;    
}tTemplateStruct;

typedef struct
{
    char templateFileName[50];
    tANI_U32 mpduHdrLen;
}tBtqmTemplateStruct;

tTemplateStruct templateList[10] = {
 {"wlan/psPoll", SIR_MAC_CTRL_FRAME, SIR_MAC_CTRL_PS_POLL, SIR_MAC_CTRL_FRAME, SIR_MAC_CTRL_ACK, 0, 0},
 {"wlan/probeRsp", SIR_MAC_MGMT_FRAME, SIR_MAC_MGMT_PROBE_RSP, SIR_MAC_CTRL_FRAME, SIR_MAC_CTRL_ACK, 0, 0},
 {"wlan/data", SIR_MAC_DATA_FRAME, SIR_MAC_DATA_DATA, SIR_MAC_CTRL_FRAME, SIR_MAC_CTRL_ACK, 0, 0},
 {"wlan/qosData", SIR_MAC_DATA_FRAME, SIR_MAC_DATA_QOS_DATA, SIR_MAC_CTRL_FRAME, SIR_MAC_CTRL_ACK, 0, 0},      
 {"wlan/probeReq", SIR_MAC_MGMT_FRAME, SIR_MAC_MGMT_PROBE_REQ, SIR_MAC_CTRL_FRAME, SIR_MAC_CTRL_ACK, 0, 0},

 };

tBtqmTemplateStruct btqmTemplateList[10] = {
 {"wlan/psPoll", 24},
 {"wlan/probeRsp", 24},
 {"wlan/data", 24}, 
 {"wlan/qosData", 26}

 };

#endif

/* ---------------------------------------------
 * FUNCTION:  halLog_printRxpBinarySearchTable()
 *
 * NOTE:
 *   Prints the entire RXP Binary Search Table
 *   from HW register.
 * ---------------------------------------------
 */
    void halLog_printRxpBinarySearchTable(tpAniSirGlobal pMac)
    {
        tANI_U32    value;
        tANI_U8     lowPtr, highPtr;

        HALLOGW( halLog(pMac, LOG1, FL("==========================\n")));
        HALLOGW( halLog(pMac, LOG1, FL("  RXP BINARY SEARCH TABLE \n")));
        HALLOGW( halLog(pMac, LOG1, FL("==========================\n")));

        // ADDRESS 1
        halReadRegister(pMac, QWLAN_RXP_SEARCH_ADDR1_PTR_REG, &value);
        HALLOGW( halLog( pMac, LOGW, FL("ADDR1 POINTER = 0x%x\n"),  value ));
        if (value & QWLAN_RXP_SEARCH_ADDR1_PTR_ADDR1_POINTERS_VALID_MASK)
        {
            lowPtr = (tANI_U8) (value & QWLAN_RXP_SEARCH_ADDR1_PTR_ADDR1_SEARCH_LO_PTR_MASK);
            highPtr = (tANI_U8) ((value & QWLAN_RXP_SEARCH_ADDR1_PTR_ADDR1_SEARCH_HI_PTR_MASK) >> QWLAN_RXP_SEARCH_ADDR1_PTR_ADDR1_SEARCH_HI_PTR_OFFSET);
            HALLOGW( halLog( pMac, LOGW, FL("ADDR1 POINTER: lowPtr=0x%x,  highPtr=0x%x\n"),  lowPtr, highPtr ));
            halLog_printRxpTableEntry(pMac, lowPtr, highPtr);
        }

        // ADDRESS 2
        halReadRegister(pMac, QWLAN_RXP_SEARCH_ADDR2_PTR_REG, &value);

        HALLOGW( halLog( pMac, LOGW, FL("ADDR2 POINTER = 0x%x"),  value ));
        if (value & QWLAN_RXP_SEARCH_ADDR2_PTR_ADDR2_POINTERS_VALID_MASK)
        {
            lowPtr = (tANI_U8) (value & QWLAN_RXP_SEARCH_ADDR2_PTR_ADDR2_SEARCH_LO_PTR_MASK);
            highPtr = (tANI_U8) ((value & QWLAN_RXP_SEARCH_ADDR2_PTR_ADDR2_SEARCH_HI_PTR_MASK) >> QWLAN_RXP_SEARCH_ADDR2_PTR_ADDR2_SEARCH_HI_PTR_OFFSET);
            HALLOGW( halLog( pMac, LOGW, FL("ADDR2 POINTER:  lowPtr=0x%x,  highPtr=0x%x\n"),  lowPtr, highPtr ));
            halLog_printRxpTableEntry(pMac, lowPtr, highPtr);
        }

        // ADDRESS 3
        halReadRegister(pMac, QWLAN_RXP_SEARCH_ADDR3_PTR_REG, &value);
        HALLOGW( halLog( pMac, LOGW, FL("ADDR3 POINTER = 0x%x\n"),  value ));
        if (value & QWLAN_RXP_SEARCH_ADDR3_PTR_ADDR3_POINTERS_VALID_MASK)
        {
            lowPtr = (tANI_U8) (value & QWLAN_RXP_SEARCH_ADDR3_PTR_ADDR3_SEARCH_LO_PTR_MASK);
            highPtr = (tANI_U8) ((value & QWLAN_RXP_SEARCH_ADDR3_PTR_ADDR3_SEARCH_HI_PTR_MASK) >> QWLAN_RXP_SEARCH_ADDR3_PTR_ADDR3_SEARCH_HI_PTR_OFFSET);
            HALLOGW( halLog( pMac, LOGW, FL("ADDR3 POINTER:  lowPtr=0x%x,  highPtr=0x%x\n"),  lowPtr, highPtr ));
            halLog_printRxpTableEntry(pMac, lowPtr, highPtr);
        }
        return;

    }


    /* -----------------------------------------------
     * FUNCTION:  halLog_printRxpTableEntry()
     *
     * NOTE:
     *   Prints the entries in the "RXP Binary Search
     *   Table" that's between lowIndex and highIndex.
     * ------------------------------------------------
     */
    void halLog_printRxpTableEntry(tpAniSirGlobal pMac, tANI_U8 lowIndex, tANI_U8 highIndex)
    {
        tANI_U16   i;
        tANI_U32    data0;
        tANI_U32    data1;
        tANI_U32    data2;
        tANI_U8     staid, valid, dropBit, rmfBit, ptkIdx, gtkIdx, igtkIdx, ptkTag, gtkTag, igtkTag, wepKeyExt, dpuNE;

        for (i=lowIndex; i<=highIndex; i++)
        {
            halWriteRegister(pMac, QWLAN_RXP_SEARCH_TABLE_CMD_REG, i) ;

            halReadRegister(pMac, QWLAN_RXP_SEARCH_TABLE_DATA0_REG, &data0);

            halReadRegister(pMac, QWLAN_RXP_SEARCH_TABLE_DATA1_REG, &data1);


            halReadRegister(pMac, QWLAN_RXP_SEARCH_TABLE_DATA2_REG, &data2);

            staid = (tANI_U8) ((data1 & QWLAN_RXP_SEARCH_TABLE_DATA1_ADDR_ID_MASK) >> QWLAN_RXP_SEARCH_TABLE_DATA1_ADDR_ID_OFFSET);
            valid = (tANI_U8) ((data1 & 0x0800000) >> QWLAN_RXP_SEARCH_TABLE_DATA1_VALID_OFFSET);
            igtkTag = (tANI_U8) ((data1 & 0xE0000000) >> 0x1D);
            rmfBit = (tANI_U8) ((data1 & 0x04000000) >> QWLAN_RXP_SEARCH_TABLE_DATA1_RMF_OFFSET);
            dropBit = (tANI_U8)((data1 & 0x00000100) >> QWLAN_RXP_SEARCH_TABLE_DATA1_DROP_OFFSET);
            ptkIdx = (tANI_U8) (data2 & 0xFF);
            gtkIdx = (tANI_U8) ((data2 & 0xFF00) >> QWLAN_RXP_SEARCH_TABLE_DATA2_DPU_MC_BC_DPU_DESC_OFFSET);
            ptkTag = (tANI_U8) ((data2 & 0x70000) >> QWLAN_RXP_SEARCH_TABLE_DATA2_DPU_TAG_OFFSET);
            wepKeyExt = (tANI_U8) ((data2 & 0x80000) >> QWLAN_RXP_SEARCH_TABLE_DATA2_WEP_KEY_ID_EXTRACT_ENABLE_OFFSET);
            dpuNE = (tANI_U8) ((data2 & 0x100000) >> QWLAN_RXP_SEARCH_TABLE_DATA2_DPU_NE_OFFSET);
            igtkIdx = (tANI_U8) ((data2 & 0x1FE00000) >> QWLAN_RXP_SEARCH_TABLE_DATA2_DPU_MAN_MC_BC_DESCRIPTOR_INDEX_OFFSET);
            gtkTag = (tANI_U8) ((data2 & 0xE0000000) >> QWLAN_RXP_SEARCH_TABLE_DATA2_DPU_MC_BC_TAG_OFFSET);

            HALLOGW( halLog( pMac, LOGW, FL("\t Data0 = %08x, Data1 = %08x, Data2 = %08x\n"),  data0, data1, data2 ));
            HALLOGW( halLog(pMac, LOGW, FL("\t\t[%d]: macAddr={%x %x %x %x %x %x}, staid=%d\n"), i, (data0 & 0xFF),
                   ((data0 & 0xFF00) >> 8), ((data0 & 0xFF0000) >> 16), ((data0 & 0xFF000000) >>24),
                   (data1 & 0xFF), ((data1 & 0xFF00) >> 8), staid));
            HALLOGW( halLog(pMac, LOGW, FL("\t valid=0x%x, drop=0x%x, rmfBit=0x%x, wepKeyExt=0x%x, dpuNE=0x%x\n"
                               "\t ptkIdx=0x%x, gtkIdx=0x%x, igtkIdx=0x%x\n"
                               "\t ptkTag=0x%x, gtkTag=0x%x, igtkTag=0x%x\n"),
                               valid, dropBit, rmfBit, wepKeyExt, dpuNE, ptkIdx, gtkIdx, igtkIdx, ptkTag, gtkTag, igtkTag));
            HALLOGW( halLog(pMac, LOGW, FL("\t")));
        }
        return;

    }

#ifdef FIXME_GEN6

/* ----------------------------------------------------
 * FUNCTION:  halLog_sendDeleteBss()
 *
 * NOTE:
 *   Compose a delete bss message, and send it to mCPU.
 * ----------------------------------------------------
 */
eHalStatus halLog_sendDeleteBss(tpAniSirGlobal pMac, tANI_U32 msgType, tANI_U32 bssid)
{
    tSmacHostMesg_DelBSS msgBuf, *delBss;
    unsigned char *printmsg;
    tANI_U32 i;

    delBss = (tSmacHostMesg_DelBSS *) &msgBuf;
    if (palFillMemory(pMac->hHdd, (void *)delBss, sizeof (tSmacHostMesg_DelBSS), 0) != eHAL_STATUS_SUCCESS)
    {
        HALLOGW( halLog(pMac, LOGW, FL("palFillMemory() failed \n")));
        return eHAL_STATUS_FAILURE;
    }

    // Fill message header
    delBss->hdr.ver     = SMAC_HOSTMESG_VER;
    delBss->hdr.msgType = msgType;
    delBss->hdr.msgLen  = sizeof(tSmacHostMesg_DelBSS);

    // Fill message body
    delBss->idxBss = bssid;

    HALLOGW( halLog(pMac, LOGW, FL("version[6 bits]: 0x%x\n"), delBss->hdr.ver));
    HALLOGW( halLog(pMac, LOGW, FL("msgType[10 bits]: 0x%x\n"), delBss->hdr.msgType));
    HALLOGW( halLog(pMac, LOGW, FL("msgLen[16 bits]:  0x%x\n"), delBss->hdr.msgLen));
    HALLOGW( halLog(pMac, LOGW, FL("msgBody-bssid[16 bits]: 0x%x \n"), delBss->idxBss));

    // Send message to mCPU
    if (halMbox_SendMsg(pMac, (void *) &msgBuf) != eHAL_STATUS_SUCCESS)
    {
        HALLOGW( halLog(pMac, LOGW, FL("halSendMsgToMcup() failed \n")));
        return eHAL_STATUS_FAILURE;
    }

    printmsg = (unsigned char *)&msgBuf;
    for (i=0; i< sizeof(tSmacHostMesg_DelBSS); i++)
        HALLOGW( halLog( pMac, LOGW, FL("message[byte %d]: %02x\n"),  i, printmsg[i] ));

    return eHAL_STATUS_SUCCESS;
}


/* --------------------------
 * halLog_sendInitScan()
 * --------------------------
 */
eHalStatus halLog_sendInitScan(tpAniSirGlobal pMac, tANI_U32  sendMgmtFrame)
{
    tSirMsgQ         msg;
    tInitScanParams  *scan;
    tANI_U16         durationInMicroSeconds = 32767;
    tANI_U8          ap_macAddr[6]  = {2,3,4,5,6,7};
    tANI_U8          sta_macAddr[6] = {0xd, 0xe, 0xa, 0xd, 0xb, 0xe};


    if (palAllocateMemory(pMac->hHdd, (void **)&scan, sizeof(tInitScanParams)) != eHAL_STATUS_SUCCESS)
    {
       HALLOGW( halLog(pMac, LOGW, FL("palAllocateMemory() failed \n")));
       return eHAL_STATUS_FAILURE;
    }

    HALLOGW( halLog(pMac, LOGW, FL("composing INIT SCAN message to softmac")));
    scan->scanMode = eHAL_SYS_MODE_SCAN;
    palCopyMemory(pMac->hHdd, (void *)&scan->bssid, (void *)ap_macAddr, 6);
    scan->notifyBss = 0;
    scan->notifyHost = 0;
    scan->frameLength = 0;
    scan->frameType = 0;
    scan->scanDuration = 30;
    palZeroMemory(pMac->hHdd, (void *)&scan->macMgmtHdr, sizeof(tSirMacMgmtHdr));

    if (sendMgmtFrame == 1)
    {
        HALLOGW( halLog(pMac, LOGW, FL("Appending DATA_NULL \n")));
        scan->macMgmtHdr.fc.type = SIR_MAC_MGMT_FRAME;
        scan->macMgmtHdr.fc.subType = SIR_MAC_DATA_NULL;
        scan->macMgmtHdr.fc.protVer = 0;
        scan->macMgmtHdr.fc.order = 0;
        scan->macMgmtHdr.fc.wep = 0;
        scan->macMgmtHdr.fc.moreData =0;
        scan->macMgmtHdr.fc.powerMgmt = 1;   // Needed for station
        scan->macMgmtHdr.fc.retry = 0;
        scan->macMgmtHdr.fc.moreFrag = 0;
        scan->macMgmtHdr.fc.fromDS = 0;
        scan->macMgmtHdr.fc.toDS = 0;
        scan->macMgmtHdr.durationLo = (tANI_U8) (durationInMicroSeconds & 0xff);
        scan->macMgmtHdr.durationHi = (tANI_U8) ((durationInMicroSeconds & 0xff00) >> 8);
        palCopyMemory(pMac->hHdd, (void *)&scan->macMgmtHdr.da, (void *)ap_macAddr, 6);
        palCopyMemory(pMac->hHdd, (void *)&scan->macMgmtHdr.sa, (void *)sta_macAddr, 6);
        palCopyMemory(pMac->hHdd, (void *)&scan->macMgmtHdr.bssId, (void *)ap_macAddr, 6);
        scan->macMgmtHdr.seqControl.fragNum = 0;
        scan->macMgmtHdr.seqControl.seqNumLo = 0;
        scan->macMgmtHdr.seqControl.seqNumHi = 2;
    }  else
    HALLOGW( halLog(pMac, LOGW, FL("No frames appended \n")));


    msg.type = SIR_HAL_INIT_SCAN_REQ;
    msg.reserved = 99;
    msg.bodyptr = scan;
    msg.bodyval = 0;

 //   if (halPostMsgApi(pMac, &msg) != eSIR_SUCCESS)
 //       halLog(pMac, LOGW, "halLog_sendInitScan: halPostMsgApi() failed ");
 //   else
        HALLOGW( halLog(pMac, LOGW, FL("halPostMsgApi() success\n")));

   return eHAL_STATUS_SUCCESS;
}


/* ----------------------------------------------------
 * FUNCTION:  halLog_setScanMode()
 *
 * NOTE:
 *   Compose a ChangeSysMode message with mode set to
 *   "eHAL_SYS_MODE_SCAN", and send it to MCPU.
 * ----------------------------------------------------
 */
eHalStatus halLog_setScanMode(tpAniSirGlobal pMac)
{
    tSmacHostMesg_ChangeSysMode msgBuf, *pScan;

    pScan = (tSmacHostMesg_ChangeSysMode *) &msgBuf;
    if (palFillMemory(pMac->hHdd, (void *)pScan, sizeof (tSmacHostMesg_ChangeSysMode), 0) != eHAL_STATUS_SUCCESS)
    {
        HALLOGW( halLog(pMac, LOGW, FL("palFillMemory() failed \n")));
        return eHAL_STATUS_FAILURE;
    }

    // Fill message header
    pScan->hdr.ver     = SMAC_HOSTMESG_VER;
    pScan->hdr.msgType = SMAC_HOSTMESG_CHANGE_SYSMODE;
    pScan->hdr.msgLen  = sizeof(tSmacHostMesg_ChangeSysMode);

     // Fill message body
    pScan->frameLen = 0;               // No frame to send
    pScan->dialogToken = 0;
    pScan->noResponse = SMAC_ACK_RESPONSE_REQUIRED;              // No frame to send
    pScan->mode = eHAL_SYS_MODE_SCAN;
    pScan->flushMgmtQueue = 0;
    // pScan->frameMpdu = U8[32]

    // Send message to mCPU
    if (halMbox_SendMsg(pMac, (void *) &msgBuf) != eHAL_STATUS_SUCCESS)
    {
        HALLOGW( halLog(pMac, LOGW, FL("halSendMsgToMcup() failed \n")));
        return eHAL_STATUS_FAILURE;
    }

    return eHAL_STATUS_SUCCESS;
}


/* ----------------------------------------------------
 * FUNCTION:  halLog_setNormalMode()
 *
 * NOTE:
 *   Compose a ChangeSysMode message with mode set to
 *   "eHAL_SYS_MODE_NORMAL", and send it to MCPU.
 * ----------------------------------------------------
 */
eHalStatus halLog_setNormalMode(tpAniSirGlobal pMac)
{
    tSmacHostMesg_ChangeSysMode msgBuf, *pScan;

    pScan = (tSmacHostMesg_ChangeSysMode *) &msgBuf;
    if (palFillMemory(pMac->hHdd, (void *)pScan, sizeof (tSmacHostMesg_ChangeSysMode), 0) != eHAL_STATUS_SUCCESS)
    {
        HALLOGW( halLog(pMac, LOGW, FL("palFillMemory() failed \n")));
        return eHAL_STATUS_FAILURE;
    }

    // Fill message header
    pScan->hdr.ver     = SMAC_HOSTMESG_VER;
    pScan->hdr.msgType = SMAC_HOSTMESG_CHANGE_SYSMODE;
    pScan->hdr.msgLen  = sizeof(tSmacHostMesg_ChangeSysMode);

     // Fill message body
    pScan->frameLen = 0;               // No frame to send
    pScan->dialogToken = 0;
    pScan->noResponse = SMAC_ACK_RESPONSE_REQUIRED;              // No frame to send
    pScan->mode = eHAL_SYS_MODE_NORMAL;
    pScan->flushMgmtQueue = 0;
    // pScan->frameMpdu = U8[32]

    // Send message to mCPU
    if (halMbox_SendMsg(pMac, (void *) &msgBuf) != eHAL_STATUS_SUCCESS)
    {
        HALLOGW( halLog(pMac, LOGW, FL("halSendMsgToMcup() failed \n")));
        return eHAL_STATUS_FAILURE;
    }

    return eHAL_STATUS_SUCCESS;
}


/* ----------------------------------------------
 * FUNCTION:  halLog_printMtu()
 * ----------------------------------------------
 */
eHalStatus halLog_printMtu(tpAniSirGlobal pMac)
{
    tANI_U32   difs_0to3, difs_4to7;
    tANI_U16   cwMin[8], cwMax[8];
    tANI_U8    i;
    tANI_U32   value, addr;

    halLog(pMac, LOGW, "**** MTU REGISTER VALUES **** \n");
    halReadRegister(pMac, MTU_DIFS_LIMIT_0TO3_REG, &difs_0to3);

    halReadRegister(pMac, MTU_DIFS_LIMIT_4TO7_REG, &difs_4to7);

    HALLOGW( halLog(pMac, LOGW, FL("DIFS:  %4d  %4d  %4d  %4d  %4d  %4d  %4d  %4d \n"),
           (difs_0to3 & MTU_DIFS_LIMIT_0TO3_SW_MTU_DIFS_LIMIT_0_MASK),
           ((difs_0to3 & MTU_DIFS_LIMIT_0TO3_SW_MTU_DIFS_LIMIT_1_MASK) >>
            MTU_DIFS_LIMIT_0TO3_SW_MTU_DIFS_LIMIT_1_OFFSET),
           ((difs_0to3 & MTU_DIFS_LIMIT_0TO3_SW_MTU_DIFS_LIMIT_2_MASK) >>
            MTU_DIFS_LIMIT_0TO3_SW_MTU_DIFS_LIMIT_2_OFFSET),
           ((difs_0to3 & MTU_DIFS_LIMIT_0TO3_SW_MTU_DIFS_LIMIT_3_MASK) >>
            MTU_DIFS_LIMIT_0TO3_SW_MTU_DIFS_LIMIT_3_OFFSET),
           (difs_4to7 & MTU_DIFS_LIMIT_4TO7_SW_MTU_DIFS_LIMIT_4_MASK),
           ((difs_4to7 & MTU_DIFS_LIMIT_4TO7_SW_MTU_DIFS_LIMIT_5_MASK) >>
            MTU_DIFS_LIMIT_4TO7_SW_MTU_DIFS_LIMIT_5_OFFSET),
           ((difs_4to7 & MTU_DIFS_LIMIT_4TO7_SW_MTU_DIFS_LIMIT_6_MASK) >>
            MTU_DIFS_LIMIT_4TO7_SW_MTU_DIFS_LIMIT_6_OFFSET),
           ((difs_4to7 & MTU_DIFS_LIMIT_4TO7_SW_MTU_DIFS_LIMIT_7_MASK) >>
            MTU_DIFS_LIMIT_4TO7_SW_MTU_DIFS_LIMIT_7_OFFSET)));


    for (i=0; i < 8; i++)
    {
        addr = MTU_SW_CW_MIN_CW_MAX_0_REG + (i * sizeof(tANI_U32));
        halReadRegister(pMac, addr, &value);

        cwMin[i] = (tANI_U16) (value & MTU_SW_CW_MIN_CW_MAX_0_SW_CW_MIN_0_MASK);
        cwMax[i] = (tANI_U16) ( (value & MTU_SW_CW_MIN_CW_MAX_0_SW_CW_MAX_0_MASK) >>
                                MTU_SW_CW_MIN_CW_MAX_0_SW_CW_MAX_0_OFFSET);
    }
    HALLOGW( halLog(pMac, LOGW, FL("cwMin: %4d  %4d  %4d  %4d  %4d  %4d  %4d  %4d \n"),
           cwMin[0], cwMin[1], cwMin[2], cwMin[3], cwMin[4], cwMin[5], cwMin[6], cwMin[7]));
    HALLOGW( halLog(pMac, LOGW, FL("cwMax: %4d  %4d  %4d  %4d  %4d  %4d  %4d  %4d \n"),
           cwMax[0], cwMax[1], cwMax[2], cwMax[3], cwMax[4], cwMax[5], cwMax[6], cwMax[7]));


    halReadRegister(pMac, MTU_EIFS_PIFS_SLOT_LIMIT_REG, &value);
    HALLOGW( halLog(pMac, LOGW, FL("EIFS: %d,   PIFS: %d,   SLOT: %d   \n"),
           ((value & MTU_EIFS_PIFS_SLOT_LIMIT_SW_MTU_EIFS_LIMIT_MASK) >>
            MTU_EIFS_PIFS_SLOT_LIMIT_SW_MTU_EIFS_LIMIT_OFFSET),
           ((value & MTU_EIFS_PIFS_SLOT_LIMIT_SW_PIFS_LIMIT_MASK) >>
            MTU_EIFS_PIFS_SLOT_LIMIT_SW_PIFS_LIMIT_OFFSET),
           (value & MTU_EIFS_PIFS_SLOT_LIMIT_SW_MTU_SLOT_LIMIT_MASK) ));


    halReadRegister(pMac, MTU_SW_MTU_BCN_SLOT_USEC_SIFS_LIMIT_REG, &value);
    HALLOGW( halLog(pMac, LOGW, FL("CCA miss limit: %d,  SIFS; %d,  1us_limit: %d,  Beacon Slot Limit %d \n"),
           ((value & MTU_SW_MTU_BCN_SLOT_USEC_SIFS_LIMIT_SW_MTU_EARLY_PKT_DET_MISS_LIMIT_MASK) >>
            MTU_SW_MTU_BCN_SLOT_USEC_SIFS_LIMIT_SW_MTU_EARLY_PKT_DET_MISS_LIMIT_OFFSET),
           ((value & MTU_SW_MTU_BCN_SLOT_USEC_SIFS_LIMIT_SW_MTU_SIFS_LIMIT_MASK) >>
            MTU_SW_MTU_BCN_SLOT_USEC_SIFS_LIMIT_SW_MTU_SIFS_LIMIT_OFFSET),
           ((value & MTU_SW_MTU_BCN_SLOT_USEC_SIFS_LIMIT_SW_MTU_ONE_USEC_LIMIT_MASK) >>
            MTU_SW_MTU_BCN_SLOT_USEC_SIFS_LIMIT_SW_MTU_ONE_USEC_LIMIT_OFFSET),
           (value & MTU_SW_MTU_BCN_SLOT_USEC_SIFS_LIMIT_SW_MTU_BCN_SLOT_LIMIT_MASK) ));

    return eHAL_STATUS_SUCCESS;
}


/* ----------------------------------------------
 * FUNCTION:  halLog_printStaTable()
 * ----------------------------------------------
 */
void halLog_printStaTable(tpAniSirGlobal pMac)
{
    tANI_U8   i, j;
    tANI_U8 rateModeStr[][8] = { "TAURUS", "TITAN", "POLARIS", "11b", "11bg", "11a", "11n"};
    tpStaStruct table = (tpStaStruct) pMac->hal.halMac.staTable;


    HALLOGW( halLog(pMac, LOGW, FL("\n ------------------------------ \n")));
    HALLOGW( halLog( pMac, LOGW, FL("   STATION TABLE (%d entries)     "),  pMac->hal.halMac.maxSta ));
    HALLOGW( halLog(pMac, LOGW, FL("\n ------------------------------ \n")));

    for (i=0; i < pMac->hal.halMac.maxSta; i++, table++)
    {
        if(table->valid == 0)
            continue;

        HALLOGW( halLog(pMac, LOGW, FL("STA #%2d (%02x:%02x:%02x:%02x:%02x:%02x)  type:%d  staId:%d  aId:%d\n"),
               i,
               table->staAddr[0], table->staAddr[1], table->staAddr[2],
               table->staAddr[3], table->staAddr[4], table->staAddr[5],
               table->staType, table->staId, table->assocId));

        HALLOGW( halLog(pMac, LOGW, FL("\tBssIdx %d (%02x:%02x:%02x:%02x:%02x:%02x)\n"),
               table->bssIdx,
               table->bssId[0], table->bssId[1], table->bssId[2],
               table->bssId[3], table->bssId[4], table->bssId[5]));

        // Parameters exchanged with SoftMAC
        HALLOGW( halLog( pMac, LOGW, FL("\tSTA sig: %d, DPU sig: %d\n"),
                table->addStaParam.staSignature, table->addStaParam.dpuSignature ));
        HALLOGW( halLog( pMac, LOGW, FL("\tencMode: %d, DPU idx: %d\n"),
                table->addStaParam.encMode, table->dpuIndex));
        HALLOGW( halLog( pMac, LOGW, FL("\tListen interval:    %d\n"),
                table->addStaParam.listenInterval ));
        HALLOGW( halLog( pMac, LOGW, FL("\tForce RTS: %d, CTS2S: %d\n"),
                table->addStaParam.fForceRts, table->addStaParam.fForceCts2s ));
        HALLOGW( halLog(  pMac, LOGW, FL("\tQoS: %c\n"),  table->addStaParam.fQosSupport? 'Y':'N'  ));

        if(table->addStaParam.fQosSupport){
            halLog( pMac, LOGW, "\t   Delayed BA: %c, UAPSD: %c,UAPSD Max SP %d\n",
            HALLOGW( halLog( pMac, LOGW, FL("\t   Delayed BA: %c, UAPSD: %c,UAPSD Max SP %d\n"),
                    table->addStaParam.fDelayedBAsupport?'Y':'N', table->addStaParam.acUAPSD?'Y':'N',
                    table->addStaParam.acUAPSDMaxSp);

            HALLOGW( halLog( pMac, LOGW, FL("\t   prt TID BA sessionId:  %d,%d,%d,%d - %d,%d,%d,%d"),
                table->baSessionID[0],table->baSessionID[1],table->baSessionID[2],table->baSessionID[3],
                table->baSessionID[4],table->baSessionID[5],table->baSessionID[6],table->baSessionID[7] ));
            if(table->addStaParam.cap11nHT){
                HALLOGW( halLog(  pMac, LOGW, FL("\t 11n HT Capable %c\n"),  table->addStaParam.cap11nHT?'Y':'N'  ));
                HALLOGW( halLog(  pMac, LOGW, FL("\t    MIMO Power Save mode %u\n"),  table->addStaParam.mimoPwrSaveMode  ));
                HALLOGW( halLog(  pMac, LOGW, FL("\t    MAX AMPDU Duration %d, size %d, density: %d\n"),  table->addStaParam.us32MaxAmpduDuration, table->addStaParam.maxAmpduSize, table->addStaParam.maxAmpduDensity  ));
                HALLOGW( halLog(  pMac, LOGW, FL("\t    MAX AMSDU Size %d\n"),  table->addStaParam.maxAmsduSzie  ));
                HALLOGW( halLog(  pMac, LOGW, FL("\t    MAX AMSDU Size %d\n"),  table->addStaParam.maxAmsduSzie  ));
                HALLOGW( halLog(  pMac, LOGW, FL("\t    GF: %c  40Mhz: %c  SGI20: %c SGI40:%c\n"),  table->halRaInfo.gfEnabled?'Y':'N', table->halRaInfo.cbMode?'Y':'N', table->halRaInfo.shortGI20?'Y':'N',table->halRaInfo.shortGI40?'Y':'N' ));
            }
        }
        HALLOGW( halLog( pMac, LOGW, FL("\tMIMO: %c:  ShrtPreamble: %c\n"),
                 table->halRaInfo.mimoMode?'Y':'N', table->halRaInfo.shortPreamble?'Y':'N'));

        HALLOGW( halLog(  pMac, LOGW, FL("\t   Stations TxAckPackets count:  %u \n"),  table->txAckPkts ));

    }
    return;
}


/* ----------------------------------------------
 * FUNCTION:  halLog_getStaSignature()
 * ----------------------------------------------
 */
void halLog_getStaSignature(tpAniSirGlobal pMac, tANI_U8 staId)
{
    tANI_U8     dpuIdx;
    tANI_U8    signature;

    halTable_GetStaDpuIdx(pMac, staId, &dpuIdx);
    halDpu_GetSignature(pMac, dpuIdx, &signature);

    HALLOGW( halLog(pMac, LOGW, FL("For Station Id %d, DPU Id %d, Signature = %d \n"),
          staId, dpuIdx, signature);

    return;
}


/* ---------------------------------------
 * FUNCTION:  halLog_printDpuDescriptor()
 * ---------------------------------------
 */
eHalStatus halLog_printDpuDescriptor(tpAniSirGlobal pMac, tANI_U32 index)
{
    tANI_U32  address;

    tDpuDescriptor  msg, *pMsg;
    pMsg = (tDpuDescriptor *)&msg;
    palFillMemory(pMac->hHdd, (void *)pMsg, sizeof(tDpuDescriptor), 0);

    address = DPU_DESCRIPTOR_OFFSET + (index * sizeof(tDpuDescriptor));

    halLog(pMac, LOGW, "\n ------------------------------------------ \n");
    HALLOGW( halLog( pMac, LOGW, FL("  DPU Descriptor[%d] - address 0x%x "),  index, address ));
    halLog(pMac, LOGW, "\n ------------------------------------------ \n");

    halReadDeviceMemory(pMac, address, (tANI_U8 *)pMsg,
                            sizeof(tDpuDescriptor));

    HALLOGW( halLog(pMac, LOGW, FL("[word 0 ]: ppi %d,  pli %d, txFragThreshold %d DWORD, tag %d\n"),
           pMsg->ppi, pMsg->pli, pMsg->txFragThreshold4B, pMsg->signature));

    HALLOGW( halLog(pMac, LOGW, FL("[word 1 ]: Decompression enable 0x%x,  Compression enable 0x%x \n"),
           pMsg->enablePerTidDecomp, pMsg->enablePerTidComp));

    HALLOGW( halLog(pMac, LOGW, FL("[word 2 ]: Concat Removal 0x%x,  Concat Insert 0x%x \n"),
           pMsg->enableTxPerTidInsertConcatSeqNum, pMsg->enableRxPerTidRemoveConcatSeqNum));

    HALLOGW( halLog(pMac, LOGW, FL("[word 3 ]: Replay Count Set %d, MIC key index %d, key index %d \n"),
           pMsg->replayCountSet, pMsg->mickeyIndex, pMsg->keyIndex));

    HALLOGW( halLog(pMac, LOGW, FL("[word 3 ]: TxkeyID %d, encrypt Mode %d \n"),
       pMsg->txKeyId,  pMsg->encryptMode));

    HALLOGW( halLog( pMac, LOGW, FL("[word 8 ]: SentBlocks 0x%x \n"),  pMsg->txSentBlocks ));
    HALLOGW( halLog( pMac, LOGW, FL("[word 9 ]: RcvdBlocks 0x%x \n"),  pMsg->rxRcvddBlocks ));
    HALLOGW( halLog(pMac, LOGW, FL("[word 10]: wepRxKeyId %u, %u, %u, %u \n"),
           pMsg->wepRxKeyIdx0, pMsg->wepRxKeyIdx1,
           pMsg->wepRxKeyIdx2, pMsg->wepRxKeyIdx3));
    HALLOGW( halLog( pMac, LOGW, FL("[word 11]: excludedCount 0x%x \n"),  pMsg->excludedCount ));
    HALLOGW( halLog(pMac, LOGW, FL("[word 12]: formatErrorCount 0x%x, undecryptable count 0x%x \n"),
           pMsg->formatErrorCount, pMsg->undecryptableCount));
    HALLOGW( halLog( pMac, LOGW, FL("[word 13]: Decrypt Error Count 0x%x \n"),  pMsg->decryptErrorCount ));
    HALLOGW( halLog( pMac, LOGW, FL("[word 14]: packet Transmitted Count 0x%x \n"),  pMsg->decryptSuccessCount ));
    HALLOGW( halLog( pMac, LOGW, FL("[word 15]: packet Reserved 0x%x \n"),  pMsg->reserved ));

    return eHAL_STATUS_SUCCESS;
}


/* -----------------------------------------------------------
 * FUNCTION:  halLog_setBmuBdPduThreshold()
 *
 * NOTE:
 *     index: BD_PDU_threshold index
 *     bd:    BD threshold
 *     pdu:   PDU threshold
 * -----------------------------------------------------------
 */
eHalStatus halLog_setBmuBdPduThreshold(tpAniSirGlobal pMac, tANI_U32 index, tANI_U16 bd, tANI_U16 pdu)
{
    tANI_U32  address, value;

    address = BMU_BD_PDU_THRESHOLD0_REG + (index * 4);
    value = ( (pdu << BMU_BD_PDU_THRESHOLD0_PDU_THRESHOLD_0_OFFSET) |
              (bd & BMU_BD_PDU_THRESHOLD0_BD_THRESHOLD_0_MASK) );

    halWriteRegister(pMac, address, value);

    HALLOGW( halLog(pMac, LOGW, FL("Setting BD_PDU_threshold%d to 0x%x \n"), index, value));
    return eHAL_STATUS_SUCCESS;
}

void halLog_dumpBmuWqHead(tpAniSirGlobal pMac)
{
    tANI_U32 wq, regValue;

    HALLOGW( halLog(pMac, LOGW, FL("********* BMU WQ HEAD *********** \n"))));
    for(wq=0; wq<QWLAN_BMU_WQ_NUM; wq++)
    {
        bmuCommand_read_wq_head(pMac, wq, &regValue);
        HALLOGW( halLog(pMac, LOGW, FL("WQ %d: 0x%x (%d) \n"), wq, regValue, regValue));
    }
    return;
}
#endif //FIXME_GEN6

/* ---------------------------
 * FUNCTION:  halLog_setCfg()
 * ---------------------------
 */
void halLog_setCfg(tpAniSirGlobal pMac, tANI_U16 cfgId, tANI_U32 val)
{
    HALLOGE( halLog(pMac, LOGE, FL("Set %s(0x%x) to value 0x%x\n"),
           gCfgParamName[cfgId], cfgId, val));

    if (cfgSetInt(pMac, (tANI_U16)cfgId, val) != eHAL_STATUS_SUCCESS)
        HALLOGE( halLog(pMac, LOGE, FL("setting cfgId 0x%x to value 0x%x failed \n"),
               cfgId, val));
     return;
}

/* ---------------------------
 * FUNCTION:  halLog_getCfg()
 * ---------------------------
 */
void halLog_getCfg(tpAniSirGlobal pMac, tANI_U16 cfgId)
{
#define CFG_CTL_INT           0x00080000
    if ((pMac->cfg.gCfgEntry[cfgId].control & CFG_CTL_INT) != 0)
    {
    tANI_U32  val;

        // Get integer parameter
    if (wlan_cfgGetInt(pMac, (tANI_U16)cfgId, &val) != eHAL_STATUS_SUCCESS)
        {
            HALLOGE( halLog(pMac, LOGE, FL("Get cfgId 0x%x failed\n"), cfgId));
        }
        else
        {
            HALLOGE( halLog( pMac, LOGE, FL("WNI_CFG_%s(%d  0x%x) = %ld\n"),  gCfgParamName[cfgId], cfgId, cfgId, val ));
        }
    }
    else
    {
        tANI_U8 buf[CFG_MAX_STR_LEN] = {0} ;
        tANI_U32 valueLen ;

        // Get string parameter
        valueLen = CFG_MAX_STR_LEN ;
        if (wlan_cfgGetStr(pMac, cfgId, buf, &valueLen) != eSIR_SUCCESS)
        {
            HALLOGE( halLog(pMac, LOGE, FL("Get cfgId 0x%x failed\n"), cfgId));
        }
        else
        {
            HALLOGE( halLog( pMac, LOGE, FL("WNI_CFG_%s(%d  0x%x) len=%ld\n"),  gCfgParamName[cfgId], cfgId, cfgId, valueLen ));
            sirDumpBuf(pMac, SIR_HAL_MODULE_ID, LOGW, buf, valueLen) ;
        }
    }

    return;
}

#ifdef FIXME_GEN6
/* -------------------------------------
 * FUNCTION:  halLog_printEdcaProfile()
 * -------------------------------------
 */
void halLog_printEdcaProfile(tpAniSirGlobal pMac)
{
    tANI_U8  i;

    for (i=0; i < MAX_NUM_AC; i++)
    {
        halLog(pMac, LOGW, "EdcaParam[%d]: ACM %d, AIFS %d, CWMin %d, CWMax %d, txop %d \n", i,
               pMac->hal.edcaParam[i].aci.acm,
               pMac->hal.edcaParam[i].aci.aifsn,
               pMac->hal.edcaParam[i].cw.min,
               pMac->hal.edcaParam[i].cw.max,
               pMac->hal.edcaParam[i].txoplimit);
    }

    return;
}

void halLog_memDump(tpAniSirGlobal pMac, const unsigned char *buffer, unsigned int len )
{
        tANI_U32 offset;
        tANI_U32 *pU32;
        for (offset = 0; offset < len; offset += 16) {
            pU32 = (tANI_U32 *)&buffer[offset];

            if((len - offset) >= 16){
                HALLOGW( halLog(  pMac, LOGW, FL("\t%04x: %08x %08x - %08x %08x\n"),  offset, pU32[0], pU32[1], pU32[2], pU32[3] ));
            }else if((len - offset) >= 12){
                HALLOGW( halLog(  pMac, LOGW, FL("\t%04x: %08x %08x - %08x\n"),  offset, pU32[0], pU32[1], pU32[2] ));
            }else if((len - offset) >= 8){
                HALLOGW( halLog(  pMac, LOGW, FL("\t%04x: %08x %08x\n"),  offset, pU32[0], pU32[1] ));
            }else if((len - offset) >= 4){
                HALLOGW( halLog(  pMac, LOGW, FL("\t%04x: %08x\n"),  offset, pU32[0] ));
            }
        }

}



/**
    \brief  This function is used to dump the WMM Profile and the related Information
            like DPU WQ, Soft Mac WQ, BMU Thresholds.
    \sa     halLog_dumpWMMProfile
    \param  pMac Mac structure
    \return none
*/
void halLog_dumpWMMProfile(tpAniSirGlobal pMac)
{
    const char AC2STR[][3] = {"BE", "BK", "VI", "VO"};
    tpStaStruct  t = (tpStaStruct) pMac->hal.halMac.staTable;
    tANI_U16     staid;
    tANI_U32   difs_0to3, difs_4to7;
    tANI_U32   bdPdu[TOT_NUM_AC];
    tANI_U16   cwMin[SMAC_STACFG_MAX_TC], cwMax[SMAC_STACFG_MAX_TC];
    tANI_U32   value, addr, idx;
    tANI_U32   tidToAcMap;

    HALLOGW( halLog(pMac, LOGW, FL("-|--------------------------------------------------------------------------|-\n")));
    HALLOGW( halLog(pMac, LOGW, FL(" |                              WMM Host Parameter                          |\n")));
    HALLOGW( halLog(pMac, LOGW, FL(" |--------------------------------------------------------------------------|\n")));
    HALLOGW( halLog(pMac, LOGW, FL(" |                      AC_BE         AC_BK         AC_VI        AC_VO      |\n")));
    HALLOGW( halLog(pMac, LOGW, FL(" |--------------------------------------------------------------------------|\n")));
    HALLOGW( halLog(pMac, LOGW, FL(" | ACM           :  0x%-4x(%-4d)| 0x%-4x(%-4d)| 0x%-4x(%-4d)| 0x%-4x(%-4d)  |\n"),
                       pMac->hal.edcaParam[EDCA_AC_BE].aci.acm, pMac->hal.edcaParam[EDCA_AC_BE].aci.acm,
                       pMac->hal.edcaParam[EDCA_AC_BK].aci.acm, pMac->hal.edcaParam[EDCA_AC_BK].aci.acm,
                       pMac->hal.edcaParam[EDCA_AC_VI].aci.acm, pMac->hal.edcaParam[EDCA_AC_VI].aci.acm,
                       pMac->hal.edcaParam[EDCA_AC_VO].aci.acm, pMac->hal.edcaParam[EDCA_AC_VO].aci.acm));
    HALLOGW( halLog(pMac, LOGW, FL(" | AIFSN         :  0x%-4x(%-4d)| 0x%-4x(%-4d)| 0x%-4x(%-4d)| 0x%-4x(%-4d)  |\n"),
                       pMac->hal.edcaParam[EDCA_AC_BE].aci.aifsn, pMac->hal.edcaParam[EDCA_AC_BE].aci.aifsn,
                       pMac->hal.edcaParam[EDCA_AC_BK].aci.aifsn, pMac->hal.edcaParam[EDCA_AC_BK].aci.aifsn,
                       pMac->hal.edcaParam[EDCA_AC_VI].aci.aifsn, pMac->hal.edcaParam[EDCA_AC_VI].aci.aifsn,
                       pMac->hal.edcaParam[EDCA_AC_VO].aci.aifsn, pMac->hal.edcaParam[EDCA_AC_VO].aci.aifsn));
    HALLOGW( halLog(pMac, LOGW, FL(" | CWMin         :  0x%-4x(%-4d)| 0x%-4x(%-4d)| 0x%-4x(%-4d)| 0x%-4x(%-4d)  |\n"),
                       pMac->hal.edcaParam[EDCA_AC_BE].cw.min, pMac->hal.edcaParam[EDCA_AC_BE].cw.min,
                       pMac->hal.edcaParam[EDCA_AC_BK].cw.min, pMac->hal.edcaParam[EDCA_AC_BK].cw.min,
                       pMac->hal.edcaParam[EDCA_AC_VI].cw.min, pMac->hal.edcaParam[EDCA_AC_VI].cw.min,
                       pMac->hal.edcaParam[EDCA_AC_VO].cw.min, pMac->hal.edcaParam[EDCA_AC_VO].cw.min));
    HALLOGW( halLog(pMac, LOGW, FL(" | CWMax         :  0x%-4x(%-4d)| 0x%-4x(%-4d)| 0x%-4x(%-4d)| 0x%-4x(%-4d)  |\n"),
                       pMac->hal.edcaParam[EDCA_AC_BE].cw.max, pMac->hal.edcaParam[EDCA_AC_BE].cw.max,
                       pMac->hal.edcaParam[EDCA_AC_BK].cw.max, pMac->hal.edcaParam[EDCA_AC_BK].cw.max,
                       pMac->hal.edcaParam[EDCA_AC_VI].cw.max, pMac->hal.edcaParam[EDCA_AC_VI].cw.max,
                       pMac->hal.edcaParam[EDCA_AC_VO].cw.max, pMac->hal.edcaParam[EDCA_AC_VO].cw.max));
    HALLOGW( halLog(pMac, LOGW, FL(" | TXOP          :  0x%-4x(%-4d)| 0x%-4x(%-4d)| 0x%-4x(%-4d)| 0x%-4x(%-4d)  |\n"),
                       pMac->hal.edcaParam[EDCA_AC_BE].txoplimit, pMac->hal.edcaParam[EDCA_AC_BE].txoplimit,
                       pMac->hal.edcaParam[EDCA_AC_BK].txoplimit, pMac->hal.edcaParam[EDCA_AC_BK].txoplimit,
                       pMac->hal.edcaParam[EDCA_AC_VI].txoplimit, pMac->hal.edcaParam[EDCA_AC_VI].txoplimit,
                       pMac->hal.edcaParam[EDCA_AC_VO].txoplimit, pMac->hal.edcaParam[EDCA_AC_VO].txoplimit));
    HALLOGW( halLog(pMac, LOGW, FL(" | BackOff ID    :  0x%-4x(%-4d)| 0x%-4x(%-4d)| 0x%-4x(%-4d)| 0x%-4x(%-4d)  |\n"),
                       MTU_BKID_AC_BE, MTU_BKID_AC_BE, MTU_BKID_AC_BK, MTU_BKID_AC_BK,
                       MTU_BKID_AC_VI, MTU_BKID_AC_VI, MTU_BKID_AC_VO, MTU_BKID_AC_VO));

    halReadRegister(pMac, MTU_DIFS_LIMIT_0TO3_REG, &difs_0to3);
    halReadRegister(pMac, MTU_DIFS_LIMIT_4TO7_REG, &difs_4to7) ;
    for (idx=0; idx < STACFG_MAX_TC; idx++)
    {
        addr = MTU_SW_CW_MIN_CW_MAX_0_REG + (idx * sizeof(tANI_U32));
        halReadRegister(pMac, addr, &value);

        cwMin[idx] = (tANI_U16) (value & MTU_SW_CW_MIN_CW_MAX_0_SW_CW_MIN_0_MASK);
        cwMax[idx] = (tANI_U16) ( (value & MTU_SW_CW_MIN_CW_MAX_0_SW_CW_MAX_0_MASK) >>
                                MTU_SW_CW_MIN_CW_MAX_0_SW_CW_MAX_0_OFFSET);
    }

    HALLOGW( halLog(pMac, LOGW, FL("-|--------------------------------------------------------------------------|-\n")));
    HALLOGW( halLog(pMac, LOGW, FL(" |                       WMM HW (MTU)REGISTER PARAMETER                     |\n")));
    HALLOGW( halLog(pMac, LOGW, FL("-|--------------------------------------------------------------------------|-\n")));
    HALLOGW( halLog(pMac, LOGW, FL(" | BKOFF ID      :    0      1      2      3      4      5      6      7    |\n")));
    HALLOGW( halLog(pMac, LOGW, FL("-|--------------------------------------------------------------------------|-\n")));
    HALLOGW( halLog(pMac, LOGW, FL(" | DIFS          :   0x%-4x 0x%-4x 0x%-4x 0x%-4x 0x%-4x 0x%-4x 0x%-4x 0x%-4x|\n"),
                       (difs_0to3 & MTU_DIFS_LIMIT_0TO3_SW_MTU_DIFS_LIMIT_0_MASK),
                       ((difs_0to3 & MTU_DIFS_LIMIT_0TO3_SW_MTU_DIFS_LIMIT_1_MASK) >>
                       MTU_DIFS_LIMIT_0TO3_SW_MTU_DIFS_LIMIT_1_OFFSET),
                       ((difs_0to3 & MTU_DIFS_LIMIT_0TO3_SW_MTU_DIFS_LIMIT_2_MASK) >>
                       MTU_DIFS_LIMIT_0TO3_SW_MTU_DIFS_LIMIT_2_OFFSET),
                       ((difs_0to3 & MTU_DIFS_LIMIT_0TO3_SW_MTU_DIFS_LIMIT_3_MASK) >>
                       MTU_DIFS_LIMIT_0TO3_SW_MTU_DIFS_LIMIT_3_OFFSET),
                       (difs_4to7 & MTU_DIFS_LIMIT_4TO7_SW_MTU_DIFS_LIMIT_4_MASK),
                       ((difs_4to7 & MTU_DIFS_LIMIT_4TO7_SW_MTU_DIFS_LIMIT_5_MASK) >>
                       MTU_DIFS_LIMIT_4TO7_SW_MTU_DIFS_LIMIT_5_OFFSET),
                       ((difs_4to7 & MTU_DIFS_LIMIT_4TO7_SW_MTU_DIFS_LIMIT_6_MASK) >>
                       MTU_DIFS_LIMIT_4TO7_SW_MTU_DIFS_LIMIT_6_OFFSET),
                       ((difs_4to7 & MTU_DIFS_LIMIT_4TO7_SW_MTU_DIFS_LIMIT_7_MASK) >>
                       MTU_DIFS_LIMIT_4TO7_SW_MTU_DIFS_LIMIT_7_OFFSET)));
    HALLOGW( halLog(pMac, LOGW, FL(" | CWMin         :   0x%-4x 0x%-4x 0x%-4x 0x%-4x 0x%-4x 0x%-4x 0x%-4x 0x%-4x|\n"),
                       cwMin[0], cwMin[1], cwMin[2], cwMin[3], cwMin[4], cwMin[5], cwMin[6], cwMin[7]));
    HALLOGW( halLog(pMac, LOGW, FL(" | CWMax         :   0x%-4x 0x%-4x 0x%-4x 0x%-4x 0x%-4x 0x%-4x 0x%-4x 0x%-4x|\n"),
                       cwMax[0], cwMax[1], cwMax[2], cwMax[3], cwMax[4], cwMax[5], cwMax[6], cwMax[7]));
    HALLOGW( halLog(pMac, LOGW, FL(" |--------------------------------------------------------------------------|\n")));
    HALLOGW( halLog(pMac, LOGW, FL(" |                   AC_BE          AC_BK           AC_VI          AC_VO    |\n")));
    HALLOGW( halLog(pMac, LOGW, FL(" |--------------------------------------------------------------------------|\n")));

    addr = BMU_BD_PDU_THRESHOLD0_REG + (SMAC_BMU_MASTERID_DXE_1 * 0x4);
    halReadRegister(pMac, addr, &bdPdu[EDCA_AC_BE]) ;
    addr = BMU_BD_PDU_THRESHOLD0_REG + (SMAC_BMU_MASTERID_DXE_0 * 0x4);
    halReadRegister(pMac, addr, &bdPdu[EDCA_AC_BK]);
    addr = BMU_BD_PDU_THRESHOLD0_REG + (SMAC_BMU_MASTERID_DXE_2 * 0x4);
    halReadRegister(pMac, addr, &bdPdu[EDCA_AC_VI]);
    addr = BMU_BD_PDU_THRESHOLD0_REG + (SMAC_BMU_MASTERID_DXE_3 * 0x4);
    halReadRegister(pMac, addr, &bdPdu[EDCA_AC_VO]);
    HALLOGW( halLog(pMac, LOGW, FL(" | BD Threshold  :   0x%-4x         0x%-4x          0x%-4x          0x%-4x  |\n"),
                       (bdPdu[EDCA_AC_BE] & 0x7ff),
                       (bdPdu[EDCA_AC_BK] & 0x7ff),
                       (bdPdu[EDCA_AC_VI] & 0x7ff),
                       (bdPdu[EDCA_AC_VO] & 0x7ff)));
    HALLOGW( halLog(pMac, LOGW, FL(" | PDU Threshold :   0x%-4x         0x%-4x          0x%-4x          0x%-4x  |\n"),
                       (bdPdu[EDCA_AC_BE] & 0xffff0000) >> 16,
                       (bdPdu[EDCA_AC_BK] & 0xffff0000) >> 16,
                       (bdPdu[EDCA_AC_VI] & 0xffff0000) >> 16,
                       (bdPdu[EDCA_AC_VO] & 0xffff0000) >> 16));
    HALLOGW( halLog(pMac, LOGW, FL("-|--------------------------------------------------------------------------|-\n")));
}


/**
    \brief   This function returns the WQ type string for a paticular WQ number.
    \sa      BmuWQType
    \param   nWQ Work Queue number
    \return  Work Queue type string
*/
static const char *BmuWQType(tANI_U8 nWQ)
{
    switch (nWQ)
    {
        case 0:
            return "IDLE BD";
        case 1:
            return "IDLE PDU";
        case 2:
        case 24:
        case 16:
            return "MCPU RX";
        case 3:
            return "DPU RX";
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
            return "DPU TX";
        case 17:
            return "DPU ERR";
        case 11:
        case 12:
        case 13:
        case 14:
            return "DXE RX";
        case 18:
        case 19:
        case 20:
        case 22:
        case 23:
            return "MCPU TX";
        default:
            return "INVALID";
    }
}

typedef struct
{
    tANI_U32    headBdIndex;
    tANI_U32    tailBdIndex;
    tANI_U32    availBDs;
    tANI_U32    isEnabled;
    tANI_U32    isDpuWq;
} stBMUWQInfo;

typedef struct
{
    tANI_U32    bdThreshold;
    tANI_U32    pduThreshold;
    tANI_U32    bdReserved;
    tANI_U32    pduReserved;
} stBMUMasterInfo;

typedef struct
{
    tANI_U32    control;
    tANI_U32    freeBD;
    tANI_U32    freePDU;
    tANI_U32    errIntrStatus;
    tANI_U32    errIntrEnable;
    tANI_U32    errIntrAddr;
    tANI_U32    errIntrWData;
    stBMUWQInfo  wqInfo[25];
    stBMUMasterInfo  masterInfo[11];
    tANI_U32    bdPduBaseAddr;
} stBMUInfo;

/**
     \brief     This function reads out all the information regarding BMU from the chip like,
                WQ  head WQ tail number of alailable BD and PDUs, BMU thresholds for various masterid.
     \sa        GetBmuInfo
     \param     pMac Mac Structure.
     \param     pBmuInfo  Pointer to the BMUInfo structure.
     \return    none.
*/
static void GetBmuInfo(tpAniSirGlobal pMac, stBMUInfo *pBmuInfo)
{
    tANI_U32 i, wqEnable, isDpuWq, bdPduThreshold, bdPduReserved;

    halReadRegister(pMac, BMU_CONTROL_REG, &(pBmuInfo->control) );

    bmuCommand_read_wq_nr(pMac, 0, &(pBmuInfo->freeBD)) != eHAL_STATUS_SUCCESS); /** \note wq0: idle BDs*/

    bmuCommand_read_wq_nr(pMac, 1, &(pBmuInfo->freePDU)) != eHAL_STATUS_SUCCESS); /** \note wq1: idle PDUs*/

    halReadRegister(pMac, BMU_ERR_INTR_STATUS_REG, &(pBmuInfo->errIntrStatus) ) ;
        /** \note masking the release_fifo_full_warning and not_enough_BD_PDU_warning bits.*/
        //pBmuInfo->errIntrStatus = pBmuInfo->errIntrStatus & 0xFFFFFDFE;
        pBmuInfo->errIntrStatus = pBmuInfo->errIntrStatus;

    halReadRegister(pMac, BMU_ERR_INTR_ENABLE_REG, &(pBmuInfo->errIntrEnable) );

    halReadRegister(pMac, BMU_ERR_INT_ADDR_REG, &(pBmuInfo->errIntrAddr) );

    halReadRegister(pMac, BMU_ERR_INT_WDATA_REG, &(pBmuInfo->errIntrWData) ) ;


    for (i=0; i< BMUWQ_NUM; i++)
    {
#ifdef WLAN_HAL_VOLANS
        if((1 << i) & BMUWQ_NOT_SUPPORTED_MASK)
            continue;
#endif
        bmuCommand_read_wq_head(pMac, i, &pBmuInfo->wqInfo[i].headBdIndex);
        bmuCommand_read_wq_tail(pMac, i, &pBmuInfo->wqInfo[i].tailBdIndex);

        bmuCommand_read_wq_nr(pMac, i, &pBmuInfo->wqInfo[i].availBDs);
        halReadRegister(pMac, BMU_WQ_ENABLE_REG, &wqEnable );

        pBmuInfo->wqInfo[i].isEnabled = (wqEnable & (1<<i)) ? 1 : 0;

        halReadRegister(pMac, BMU_DPU_WQ_ASSIGNMENT_REG, &isDpuWq );

        pBmuInfo->wqInfo[i].isDpuWq = (isDpuWq & (1<<i)) ? 1 : 0;;
    }

    for (i=0; i<11; i++)
    {
        halReadRegister(pMac, (BMU_BD_PDU_THRESHOLD0_REG + (i*4)), &bdPduThreshold );

        pBmuInfo->masterInfo[i].bdThreshold = (bdPduThreshold & BMU_BD_PDU_THRESHOLD0_BD_THRESHOLD_0_MASK) >> BMU_BD_PDU_THRESHOLD0_BD_THRESHOLD_0_OFFSET;
        pBmuInfo->masterInfo[i].pduThreshold = (bdPduThreshold & BMU_BD_PDU_THRESHOLD0_PDU_THRESHOLD_0_MASK) >> BMU_BD_PDU_THRESHOLD0_PDU_THRESHOLD_0_OFFSET;

        halReadRegister(pMac, (BMU_BD_PDU_RESERVED0_REG + (i*4)), &bdPduReserved );

        pBmuInfo->masterInfo[i].bdReserved = (bdPduReserved & BMU_BD_PDU_RESERVED0_NUMBER_RESERVED_BD_MASK) >> BMU_BD_PDU_RESERVED0_NUMBER_RESERVED_BD_DEFAULT;
        pBmuInfo->masterInfo[i].pduReserved = (bdPduReserved & BMU_BD_PDU_RESERVED0_NUMBER_RESERVED_PDU_MASK) >> BMU_BD_PDU_RESERVED0_NUMBER_RESERVED_PDU_OFFSET;
    }

    halReadRegister(pMac, MCU_BD_PDU_BASE_ADDR_REG, &pBmuInfo->bdPduBaseAddr);

    return;
}

/**
    /brief   This function is to dump the entire information regarding the BMU Work Queue.
    /sa      halLog_dumpBmuWqHead
    /param   pMac   Mac structure
    /return  none
*/
void halLog_dumpBmuWqInfo(tpAniSirGlobal pMac)
{
    tANI_U8   i;
    tANI_U32  total=0;
    tANI_U32  numOfBD;
    tANI_U32  numOfPDU;
    stBMUInfo  BmuInfo;

    GetBmuInfo(pMac, &BmuInfo);

    halBMU_getNumOfBdPdu(pMac, total, &numOfBD, &numOfPDU);

    HALLOGW( halLog(pMac, LOGW, FL("-|---------------------------------------------------------------------------|-\n")));
    HALLOGW( halLog(pMac, LOGW, FL(" |                                BMU WQ INFO                                |\n")));
    HALLOGW( halLog(pMac, LOGW, FL(" |---------------------------------------------------------------------------|\n")));
    HALLOGW( halLog( pMac, LOGW, FL(" | BMU CONTROL REGISTER    :         0x%-12x                          |\n"), BmuInfo.control ));
    HALLOGW( halLog(pMac, LOGW, FL(" |---------------------------------------------------------------------------|\n")));
    HALLOGW( halLog( pMac, LOGW, FL(" | BMU TOTAL BD            :         0x%-12x                          |\n"), numOfBD ));
    HALLOGW( halLog(pMac, LOGW, FL(" |---------------------------------------------------------------------------|\n")));
    HALLOGW( halLog( pMac, LOGW, FL(" | BMU TOTAL PDU           :         0x%-12x                          |\n"), numOfPDU ));
    HALLOGW( halLog(pMac, LOGW, FL(" |---------------------------------------------------------------------------|\n")));
    HALLOGW( halLog( pMac, LOGW, FL(" | BMU FREE BD             :         0x%-12x                          |\n"), BmuInfo.freeBD ));
    HALLOGW( halLog(pMac, LOGW, FL(" |---------------------------------------------------------------------------|\n")));
    HALLOGW( halLog( pMac, LOGW, FL(" | BMU FREE PDU            :         0x%-12x                          |\n"), BmuInfo.freePDU ));
    HALLOGW( halLog(pMac, LOGW, FL(" |---------------------------------------------------------------------------|\n")));
    HALLOGW( halLog( pMac, LOGW, FL(" | BMU BD/PDU BASE ADDRESS :         0x%-12x                          |\n"), BmuInfo.bdPduBaseAddr ));
    HALLOGW( halLog(pMac, LOGW, FL(" |---------------------------------------------------------------------------|\n")));
    HALLOGW( halLog( pMac, LOGW, FL(" | BMU ERR INTR STATUS     :         0x%-12x                          |\n"), BmuInfo.errIntrStatus ));
    HALLOGW( halLog(pMac, LOGW, FL(" |---------------------------------------------------------------------------|\n")));
    HALLOGW( halLog( pMac, LOGW, FL(" | BMU ERR INTR ENABLE     :         0x%-12x                          |\n"), BmuInfo.errIntrEnable ));
    HALLOGW( halLog(pMac, LOGW, FL(" |---------------------------------------------------------------------------|\n")));
    HALLOGW( halLog( pMac, LOGW, FL(" | BMU ERR INTR ADDRESS    :         0x%-12x                          |\n"), BmuInfo.errIntrAddr ));
    HALLOGW( halLog(pMac, LOGW, FL(" |---------------------------------------------------------------------------|\n");
    HALLOGW( halLog( pMac, LOGW, FL(" | BMU ERR INTR WDATA      :         0x%-12x                          |\n"), BmuInfo.errIntrWData ));
    HALLOGW( halLog(pMac, LOGW, FL( " |---------------------------------------------------------------------------|\n")));
    HALLOGW( halLog(pMac, LOGW, FL(" |  WQ No    WQ TYPE     WQ Head     WQ Tail      #WQ entry    WQ Enabled    |\n")));
    HALLOGW( halLog(pMac, LOGW, FL(" |---------------------------------------------------------------------------|\n")));
    for (i=0; i<25; i++)
    {
        HALLOGW( halLog(pMac, LOGW, FL(" |  0x%-2x    %-10s    0x%-8x  0x%-8x   0x%-8x  %-9s     |\n"),
                            i, BmuWQType(i), BmuInfo.wqInfo[i].headBdIndex, BmuInfo.wqInfo[i].tailBdIndex,
                            BmuInfo.wqInfo[i].availBDs, (BmuInfo.wqInfo[i].isEnabled) ? "ENABLED":"DISABLED"));
    }

    HALLOGW( halLog(pMac, LOGW, FL(" |---------------------------------------------------------------------------|\n")));
    HALLOGW( halLog(pMac, LOGW, FL(" |MASTERID:   0    1     2     3     4     5     6     7     8     9     10  |\n")));
    HALLOGW( halLog(pMac, LOGW, FL(" |---------------------------------------------------------------------------|\n")));
    HALLOGW( halLog(pMac, LOGW, FL(" |BD THRS :0x%-4x0x%-4x0x%-4x0x%-4x0x%-4x0x%-4x0x%-4x0x%-4x0x%-4x0x%-4x0x%-4x|\n"),
                         BmuInfo.masterInfo[0].bdThreshold, BmuInfo.masterInfo[1].bdThreshold,
                         BmuInfo.masterInfo[2].bdThreshold, BmuInfo.masterInfo[3].bdThreshold,
                         BmuInfo.masterInfo[4].bdThreshold, BmuInfo.masterInfo[5].bdThreshold,
                         BmuInfo.masterInfo[6].bdThreshold, BmuInfo.masterInfo[7].bdThreshold,
                         BmuInfo.masterInfo[8].bdThreshold, BmuInfo.masterInfo[9].bdThreshold,
                         BmuInfo.masterInfo[10].bdThreshold));
    HALLOGW( halLog(pMac, LOGW, FL(" |PDU THRS:0x%-4x0x%-4x0x%-4x0x%-4x0x%-4x0x%-4x0x%-4x0x%-4x0x%-4x0x%-4x0x%-4x|\n"),
                         BmuInfo.masterInfo[0].pduThreshold, BmuInfo.masterInfo[1].pduThreshold,
                         BmuInfo.masterInfo[2].pduThreshold, BmuInfo.masterInfo[3].pduThreshold,
                         BmuInfo.masterInfo[4].pduThreshold, BmuInfo.masterInfo[5].pduThreshold,
                         BmuInfo.masterInfo[6].pduThreshold, BmuInfo.masterInfo[7].pduThreshold,
                         BmuInfo.masterInfo[8].pduThreshold, BmuInfo.masterInfo[9].pduThreshold,
                         BmuInfo.masterInfo[10].pduThreshold));
    HALLOGW( halLog(pMac, LOGW, FL(" |BD RESV :0x%-4x0x%-4x0x%-4x0x%-4x0x%-4x0x%-4x0x%-4x0x%-4x0x%-4x0x%-4x0x%-4x|\n"),
                         BmuInfo.masterInfo[0].bdReserved, BmuInfo.masterInfo[1].bdReserved,
                         BmuInfo.masterInfo[2].bdReserved, BmuInfo.masterInfo[3].bdReserved,
                         BmuInfo.masterInfo[4].bdReserved, BmuInfo.masterInfo[5].bdReserved,
                         BmuInfo.masterInfo[6].bdReserved, BmuInfo.masterInfo[7].bdReserved,
                         BmuInfo.masterInfo[8].bdReserved, BmuInfo.masterInfo[9].bdReserved,
                         BmuInfo.masterInfo[10].bdReserved));
    HALLOGW( halLog(pMac, LOGW, FL(" |PDU RESV:0x%-4x0x%-4x0x%-4x0x%-4x0x%-4x0x%-4x0x%-4x0x%-4x0x%-4x0x%-4x0x%-4x|\n"),
                         BmuInfo.masterInfo[0].pduReserved, BmuInfo.masterInfo[1].pduReserved,
                         BmuInfo.masterInfo[2].pduReserved, BmuInfo.masterInfo[3].pduReserved,
                         BmuInfo.masterInfo[4].pduReserved, BmuInfo.masterInfo[5].pduReserved,
                         BmuInfo.masterInfo[6].pduReserved, BmuInfo.masterInfo[7].pduReserved,
                         BmuInfo.masterInfo[8].pduReserved, BmuInfo.masterInfo[9].pduReserved,
                         BmuInfo.masterInfo[10].pduReserved));
    HALLOGW( halLog(pMac, LOGW, FL("-|---------------------------------------------------------------------------|-\n");
}

/**
\brief  This function prints information about the
\       power save info and deferred queue info.
\param  tpAniSirGlobal  pMac structure
\return none
*/
void halLog_dumpPsInfo(tpAniSirGlobal pMac)
{
    HALLOGW( halLog( pMac, LOGW, FL("Idle Mode PS: %d \n"),  pMac->hal.currentlyInBmps ));
    HALLOGW( halLog( pMac, LOGW, FL("Beacon Mode PS: %d \n"),  pMac->hal.currentlyInImps ));
    HALLOGW( halLog(pMac, LOGW, FL("Defer Msg Queue:  queue size %d, read %d, write %d \n"),
           pMac->hal.halDeferMsgQ.size, pMac->hal.halDeferMsgQ.read, pMac->hal.halDeferMsgQ.write));
}

char *
dump_hal_print_adaptive_threshold( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    tANI_U32  value;
    (void) arg1; (void)arg2; (void)arg3; (void)arg4;

    p += log_sprintf( pMac,p, "******** Adaptive Threshold Algorithm ******** \n");

    p += log_sprintf( pMac,p, "Adaptive Threshold: %s \n",

                      (pMac->hal.halAdaptThresh.fThresholdAdaptionEnabled) ? "Enabled" : "Disabled");

    p += log_sprintf( pMac,p, "Algorithm:   %s \n",
             (pMac->hal.halAdaptThresh.algoSelection == ANI_SCH_ADAPTIVE_THRESHOLD_TH_CD) ? "Carrier Detect" :
             ((pMac->hal.halAdaptThresh.algoSelection == ANI_SCH_ADAPTIVE_THRESHOLD_TH_D0) ? "Correlation Detection" : "Unknown"));

    if ( pMac->hal.halAdaptThresh.bandSelection == ANI_SCH_ADAPTIVE_ALGO_BAND_2GHZ )
       p += log_sprintf( pMac,p, "Band Selection: 2GHZ \n");
    else if ( pMac->hal.halAdaptThresh.bandSelection == ANI_SCH_ADAPTIVE_ALGO_BAND_5GHZ )
       p += log_sprintf( pMac,p, "Band Selection: 5GHZ \n");
    else if ( pMac->hal.halAdaptThresh.bandSelection == ANI_SCH_ADAPTIVE_ALGO_BAND_ALL )
       p += log_sprintf( pMac,p, "Band Selection: 2GHZ & 5GHZ \n");
    else
       p += log_sprintf( pMac,p, "Band Selection: Unknown (%d) \n", pMac->hal.halAdaptThresh.bandSelection);

    p += log_sprintf( pMac,p, "Sample Interval(us):   %d \n", pMac->hal.halAdaptThresh.sampleInterval);
    p += log_sprintf( pMac,p, "Logging:   %s\n", (pMac->hal.halAdaptThresh.fEnableSamplesLogging) ? "ON" : "OFF");
    p += log_sprintf( pMac,p, "False Detect Threshold:   min %d,  max %d \n", pMac->hal.halAdaptThresh.lowFalseDetect, pMac->hal.halAdaptThresh.highFalseDetect);


    halReadRegister(pMac, AGC_TH_CD_REG, &value);
    p += log_sprintf( pMac,p, "Carrier Detect:  {current reg: 0x%x,  current hal: %3d} {min: %d, max: %d, step: %d} \n",
                       value, pMac->hal.halAdaptThresh.currentTH_CD, pMac->hal.halAdaptThresh.minTH_CD,
                       pMac->hal.halAdaptThresh.maxTH_CD, pMac->hal.halAdaptThresh.stepTH_CD);

    halReadRegister(pMac, AGC_TH_D0_A_REG, &value);
    p += log_sprintf( pMac,p, "11a   Correlation Detect:  {current reg: %3d, current hal: %3d} {min: %3d, max: %3d, step: %d} \n",
                       value, pMac->hal.halAdaptThresh.currentTH_D0_A, pMac->hal.halAdaptThresh.minTH_D0_A,
                       pMac->hal.halAdaptThresh.maxTH_D0_A, pMac->hal.halAdaptThresh.stepTH_D0_A);

    halReadRegister(pMac, AGC_TH_D0_B_REG, &value);
    p += log_sprintf( pMac,p, "11b   Correlation Detect:  {current reg: %3d, current hal: %3d} {min: %3d, max: %3d, step: %d} \n",
                       value, pMac->hal.halAdaptThresh.currentTH_D0_B, pMac->hal.halAdaptThresh.minTH_D0_B,
                       pMac->hal.halAdaptThresh.maxTH_D0_B, pMac->hal.halAdaptThresh.stepTH_D0_B);

    halReadRegister(pMac, AGC_TH_D0_11N_REG, &value);
    p += log_sprintf( pMac,p, "11n   Correlation Detect:  {current reg: %3d, current hal: %3d} {min: %3d, max: %3d, step: %d} \n",
                       value, pMac->hal.halAdaptThresh.currentTH_D0_N, pMac->hal.halAdaptThresh.minTH_D0_N,
                       pMac->hal.halAdaptThresh.maxTH_D0_N, pMac->hal.halAdaptThresh.stepTH_D0_N);
#ifdef FIXME_GEN5

    halReadRegister(pMac, AGC_TH_D0_DBL_REG, &value);
    p += log_sprintf( pMac,p, "11DBL Correlation Detect:  {current reg: %3d, current hal: %3d} {min: %3d, max: %3d,  step: %d} \n\n",
                       value, pMac->hal.halAdaptThresh.currentTH_D0_DBL, pMac->hal.halAdaptThresh.minTH_D0_DBL,
                       pMac->hal.halAdaptThresh.maxTH_D0_DBL, pMac->hal.halAdaptThresh.stepTH_D0_DBL);
#endif
    return p;
}

char *
dump_hal_mpi_pmi_stats( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    tLogdRegList mpistats[] = {
        MKENTRY(MPI_TXP_MPI_START_REG),
        MKENTRY(MPI_MPI_TXP_REQ_REG),
        MKENTRY(MPI_TXA_MPI_DATA_REQ_REG),
        MKENTRY(MPI_TXB_MPI_DATA_REQ_REG),
        MKENTRY(MPI_MPI_TXA_DATA_VAL_REG),
        MKENTRY(MPI_MPI_TXB_DATA_VAL_REG),
#ifdef FIXME_GEN5
        MKENTRY(MPI_MPI_TXB_LASTBYTE_REG),
        MKENTRY(MPI_TXEN_ALARM_REG),
        MKENTRY(MPI_TXBUSY_ALARM_REG),
#endif
        MKENTRY(MPI_REQ_ALARM_REG),
        MKENTRY(MPI_WATCHDOG_ALARM_REG),
        MKENTRY(MPI_COMMAND_ALARM_REG),
        MKENTRY(MPI_TXP_ABORT_REG)
    };
    tLogdRegList pmistats[] = {
        MKENTRY(PMI_PMI_STATE_REG),
#ifdef FIXME_GEN5
        MKENTRY(PMI_RXA_BURSTS_REG),
        MKENTRY(PMI_RXA_SINGLE_PKTS_REG),
        MKENTRY(PMI_RXA_CONCAT_PKTS_REG),
#endif
        MKENTRY(PMI_RXB_PACKETS_REG),
        MKENTRY(PMI_RXA_DATA_VALID_REG),
        MKENTRY(PMI_RXB_DATA_VALID_REG),
        MKENTRY(PMI_RXP_SHUTOFF_REG),
        MKENTRY(PMI_PHY_STARTS_REG),
        MKENTRY(PMI_PHY_DATA_VAL_REG),
        MKENTRY(PMI_PHY_ABORT_REG),
        MKENTRY(PMI_PHY_INT_REG),
        MKENTRY(PMI_WATCHDOG_TIMEOUT_REG)
    };
    tLogdRegList txctlstats[] = {
        MKENTRY(TXCTL_WATCHDOG_ALARM_REG),
        MKENTRY(TXCTL_PKT_INVALID_ALARM_REG),
        MKENTRY(TXCTL_TXA_ABORT_ALARM_REG),
        MKENTRY(TXCTL_LEGACY_PKTS_REG),
#ifdef FIXME_GEN5
        MKENTRY(TXCTL_AIRGO_SIMO_40_PKTS_REG),
        MKENTRY(TXCTL_AIRGO_MIMO_20_PKTS_REG),
        MKENTRY(TXCTL_AIRGO_MIMO_40_PKTS_REG),
        MKENTRY(TXCTL_EWC_GF_SIMO_20_PKTS_REG),
        MKENTRY(TXCTL_EWC_GF_MIMO_20_PKTS_REG),
        MKENTRY(TXCTL_EWC_GF_SIMO_40_PKTS_REG),
        MKENTRY(TXCTL_EWC_GF_MIMO_40_PKTS_REG),
        MKENTRY(TXCTL_EWC_MM_SIMO_20_PKTS_REG),
        MKENTRY(TXCTL_EWC_MM_MIMO_20_PKTS_REG),
        MKENTRY(TXCTL_EWC_MM_SIMO_40_PKTS_REG),
        MKENTRY(TXCTL_EWC_MM_MIMO_40_PKTS_REG),
#endif
        MKENTRY(TXCTL_TXB_SHORT_PKTS_REG),
        MKENTRY(TXCTL_TXB_LONG_PKTS_REG)
    };
    (void) arg1; (void) arg2; (void) arg3; (void) arg4;
    p = dump_hal_reglist(p, pMac, &mpistats[0], sizeof(mpistats)/sizeof(mpistats[0]),
                     "MPI Stats");
    p = dump_hal_reglist(p, pMac, &txctlstats[0], sizeof(txctlstats)/sizeof(txctlstats[0]),
                     "TXCTL Stats");
    p = dump_hal_reglist(p, pMac, &pmistats[0], sizeof(pmistats)/sizeof(pmistats[0]),
                     "PMI Stats");
    return p;
}

char *
dump_hal_radar_regs( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    (void) arg1; (void) arg2; (void) arg3; (void) arg4;
#ifdef ANI_PRODUCT_TYPE_AP
    p += tx_sprintf(p, "Reading RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG...\n");
    p = dump_hal_reg_read(pMac, RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, arg2, arg3, arg4, p);
    p += tx_sprintf(p, "Reading RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_AGC_REG...\n");
    p = dump_hal_reg_read(pMac, RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_AGC_REG, arg2, arg3, arg4, p);
    p += tx_sprintf(p, "Reading AGC_CW_DETECT_REG...\n");
    p = dump_hal_reg_read(pMac, AGC_CW_DETECT_REG, arg2, arg3, arg4, p);
    p += tx_sprintf(p, "Reading AGC_RDET_RESET_REG...\n");
    p = dump_hal_reg_read(pMac, AGC_RDET_RESET_REG, arg2, arg3, arg4, p);
    p += tx_sprintf(p, "Reading AGC_RDET_MIN_RADAR_PULSE_WIDTH_WRITE_REG...\n");
    p = dump_hal_reg_read(pMac, AGC_RDET_MIN_RADAR_PULSE_WIDTH_WRITE_REG, arg2, arg3, arg4, p);
    p += tx_sprintf(p, "Reading AGC_RDET_MAX_RADAR_PULSE_WIDTH_WRITE_REG...\n");
    p = dump_hal_reg_read(pMac, AGC_RDET_MAX_RADAR_PULSE_WIDTH_WRITE_REG, arg2, arg3, arg4, p);
    p += tx_sprintf(p, "Reading AGC_RDET_MIN_IAT_WRITE_REG...\n");
    p = dump_hal_reg_read(pMac, AGC_RDET_MIN_IAT_WRITE_REG, arg2, arg3, arg4, p);
    p += tx_sprintf(p, "Reading AGC_RDET_MAX_IAT_WRITE_REG...\n");
    p = dump_hal_reg_read(pMac, AGC_RDET_MAX_IAT_WRITE_REG, arg2, arg3, arg4, p);
    p += tx_sprintf(p, "Reading AGC_RDET_MIN_PULSE_TRAIN_COUNT1_WRITE_REG...\n");
    p = dump_hal_reg_read(pMac, AGC_RDET_MIN_PULSE_TRAIN_COUNT1_WRITE_REG, arg2, arg3, arg4, p);
    p += tx_sprintf(p, "Reading AGC_RDET_MIN_PULSE_TRAIN_COUNT2_WRITE_REG...\n");
    p = dump_hal_reg_read(pMac, AGC_RDET_MIN_PULSE_TRAIN_COUNT2_WRITE_REG, arg2, arg3, arg4, p);
    p += tx_sprintf(p, "Reading AGC_RDET_MIN_PULSE_TRAIN_COUNT3_WRITE_REG...\n");
    p = dump_hal_reg_read(pMac, AGC_RDET_MIN_PULSE_TRAIN_COUNT3_WRITE_REG, arg2, arg3, arg4, p);
    p += tx_sprintf(p, "Reading AGC_RDET_MIN_PULSE_TRAIN_COUNT4_WRITE_REG...\n");
    p = dump_hal_reg_read(pMac, AGC_RDET_MIN_PULSE_TRAIN_COUNT4_WRITE_REG, arg2, arg3, arg4, p);
    p += tx_sprintf(p, "Reading AGC_RDET_PULSE_WIDTH_MARGIN_WRITE_REG...\n");
    p = dump_hal_reg_read(pMac, AGC_RDET_PULSE_WIDTH_MARGIN_WRITE_REG, arg2, arg3, arg4, p);

    p += tx_sprintf(p, "More........................\n");
    p += tx_sprintf(p, "Reading AGC_RDET_RSSI_THRESHOLD_REG...\n");
    p = dump_hal_reg_read(pMac, AGC_RDET_RSSI_THRESHOLD_REG, arg2, arg3, arg4, p);
    p += tx_sprintf(p, "Reading AGC_RDET_MIN_IAT_READ_REG...\n");
    p = dump_hal_reg_read(pMac, AGC_RDET_MIN_IAT_READ_REG, arg2, arg3, arg4, p);
    p += tx_sprintf(p, "Reading AGC_RDET_MAX_IAT_READ_REG...\n");
    p = dump_hal_reg_read(pMac, AGC_RDET_MAX_IAT_READ_REG, arg2, arg3, arg4, p);
    p += tx_sprintf(p, "Reading AGC_RDET_PULSE_WIDTH_MARGIN_READ_REG...\n");
    p = dump_hal_reg_read(pMac, AGC_RDET_PULSE_WIDTH_MARGIN_READ_REG, arg2, arg3, arg4, p);
    p += tx_sprintf(p, "Reading AGC_RDET_MIN_PULSE_TRAIN_COUNT1_READ_REG...\n");
    p = dump_hal_reg_read(pMac, AGC_RDET_MIN_PULSE_TRAIN_COUNT1_READ_REG, arg2, arg3, arg4, p);
    p += tx_sprintf(p, "Reading AGC_RDET_MIN_PULSE_TRAIN_COUNT2_READ_REG...\n");
    p = dump_hal_reg_read(pMac, AGC_RDET_MIN_PULSE_TRAIN_COUNT2_READ_REG, arg2, arg3, arg4, p);
    p += tx_sprintf(p, "Reading AGC_RDET_MIN_PULSE_TRAIN_COUNT3_READ_REG...\n");
    p = dump_hal_reg_read(pMac, AGC_RDET_MIN_PULSE_TRAIN_COUNT3_READ_REG, arg2, arg3, arg4, p);
    p += tx_sprintf(p, "Reading AGC_RDET_MIN_PULSE_TRAIN_COUNT4_READ_REG...\n");
    p = dump_hal_reg_read(pMac, AGC_RDET_MIN_PULSE_TRAIN_COUNT4_READ_REG, arg2, arg3, arg4, p);
    p += tx_sprintf(p, "Reading AGC_RDET_MIN_RADAR_PULSE_WIDTH_READ_REG...\n");
    p = dump_hal_reg_read(pMac, AGC_RDET_MIN_RADAR_PULSE_WIDTH_READ_REG, arg2, arg3, arg4, p);
    p += tx_sprintf(p, "Reading AGC_RDET_MAX_RADAR_PULSE_WIDTH_READ_REG...\n");
    p = dump_hal_reg_read(pMac, AGC_RDET_MAX_RADAR_PULSE_WIDTH_READ_REG, arg2, arg3, arg4, p);
    p += tx_sprintf(p, "Reading AGC_RDET_RADAR_DETECTED_REG...\n");
    p = dump_hal_reg_read(pMac, AGC_RDET_RADAR_DETECTED_REG, arg2, arg3, arg4, p);
    p += tx_sprintf(p, "Reading AGC_RDET_PULSEWIDTH_REG...\n");
    p = dump_hal_reg_read(pMac, AGC_RDET_PULSEWIDTH_REG, arg2, arg3, arg4, p);
    p += tx_sprintf(p, "Reading AGC_RDET_NUM_DETECTED_REG...\n");
    p = dump_hal_reg_read(pMac, AGC_RDET_NUM_DETECTED_REG, arg2, arg3, arg4, p);
    p += tx_sprintf(p, "Reading AGC_RDET_MEASUREMENT_DELAY_REG...\n");
    p = dump_hal_reg_read(pMac, AGC_RDET_MEASUREMENT_DELAY_REG, arg2, arg3, arg4, p);
    p += tx_sprintf(p, "Reading AGC_RDET_FLAG_RESET_REG...\n");
    p = dump_hal_reg_read(pMac, AGC_RDET_FLAG_RESET_REG, arg2, arg3, arg4, p);

    p += tx_sprintf(p, "Reading rxclkctrl apb blk clock en register....\n");
    p = dump_hal_reg_read(pMac, RXCLKCTRL_APB_BLOCK_CLK_EN_REG, arg2, arg3, arg4, p);
    p += tx_sprintf(p, "Reading phy int status register....\n");
    p = dump_hal_reg_read(pMac, PHYINT_STATUS_REG, arg2, arg3, arg4, p);
    p += tx_sprintf(p, "Reading Phy int fast mask register....\n");
    p = dump_hal_reg_read(pMac, PHYINT_FAST_MASK_REG, arg2, arg3, arg4, p);
    p += tx_sprintf(p, "Reading Phy int slow mask register....\n");
    p = dump_hal_reg_read(pMac, PHYINT_SLOW_MASK_REG, arg2, arg3, arg4, p);
    p += tx_sprintf(p, "Reading Phy int host mask register....\n");
    p = dump_hal_reg_read(pMac, PHYINT_HOST_MASK_REG, arg2, arg3, arg4, p);
    p += tx_sprintf(p, "Reading Mcu sys group 0 enable register....\n");
    p = dump_hal_reg_read(pMac, MCU_SYS_GROUP_0_INT_ENABLE_REG, arg2, arg3, arg4, p);
    p += tx_sprintf(p, "Reading Mcu sys group 0 status register....\n");
    p = dump_hal_reg_read(pMac, MCU_SYS_GROUP_0_INT_STATUS_REG, arg2, arg3, arg4, p);

    p += tx_sprintf(p, "Reading Agc Rssi threshold clipped register....\n");
    p = dump_hal_reg_read(pMac, AGC_RSSI_THRESHOLD_CLIPPED_REG, arg2, arg3, arg4, p);
    p += tx_sprintf(p, "Reading Agc d_staturated register....\n");
    p = dump_hal_reg_read(pMac, AGC_D_SATURATED_REG, arg2, arg3, arg4, p);
    p += tx_sprintf(p, "Reading Agc max gain register....\n");
    p = dump_hal_reg_read(pMac, AGC_MAX_GAIN_REG, arg2, arg3, arg4, p);
    p += tx_sprintf(p, "Reading Agc max gain ref register....\n");
    p = dump_hal_reg_read(pMac, AGC_MAX_GAIN_REF_REG, arg2, arg3, arg4, p);
    p += tx_sprintf(p, "Reading Agc cw pulse width register....\n");
    p = dump_hal_reg_read(pMac, AGC_CW_PULSE_WIDTH_REG, arg2, arg3, arg4, p);
    p += tx_sprintf(p, "Reading Agc rdet rssi threshold register....\n");
    p = dump_hal_reg_read(pMac, AGC_RDET_RSSI_THRESHOLD_REG, arg2, arg3, arg4, p);
#endif

    return p;
}

char *
dump_force_hal_nwtype24g( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    tSirNwType nwType = (tSirNwType) arg1;
    halForceSetNwType(pMac, nwType);
    tpBssStruct bssTable = (tpBssStruct) pMac->hal.halMac.bssTable;

    switch(nwType){

        case eSIR_11B_NW_TYPE: //11B

            //not 11g mode, just update the MTU and ignore arg2
            halMTU_updateTimingParams(pMac, halMTU_getMode(pMac));

            break;

        case eSIR_11G_NW_TYPE://11G
        case eSIR_11N_NW_TYPE://11N

            if(arg2){

                //change to mixed mode
                bssTable->llbCoexist = 1;

                //update MTU with mixed mode timing param
                halMTU_update11gSlotTime(pMac,0);  //no short slot

            }else{

                //setting to pure mode
                bssTable->llbCoexist = 0;

                //update MTU with mixed mode timing param
                halMTU_update11gSlotTime(pMac,1); //short slot enabled

            }

            break;

        default:

            break;
    }

    return p;
}

char *
dump_smac_globalcfg_send( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    (void) arg3; (void) arg4;
    halSetDefaultBcnRateIdx(pMac, (tTpeRateIdx) arg1);
    halSetDefaultNonBcnRateIdx(pMac, (tTpeRateIdx) arg2);
    halSetDefaultMulticastRateIdx(pMac, (tTpeRateIdx)arg3);
    return p;
}

char *
dump_hal_rate_set_txPower( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    (void) arg3; (void)arg4;
    halRate_SetTpeTxPower(pMac, (tTpeRateIdx)arg1, (tPwrTemplateIndex)arg2 );
    return p;
}

 char *
dump_hal_set_global_rates_maxTput
( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p){
    (void) arg4;
    (void) arg3;
    if(arg1==0 && arg2==0){
        return p;
    }else{
        halRate_changeTpeRateMaxActualTput(pMac, arg2, arg1);
    }
    return p;
}
char *
dump_hal_enable_radar_interrupt( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    (void) arg2; (void) arg3; (void) arg4;
#ifdef FIXME_GEN5
#ifdef ANI_PRODUCT_TYPE_AP
    if (arg1)
    {
        p += tx_sprintf(p, "Radar interrupt Enabled, Handler registered\n");
        halRadar_Init(pMac);
    }
    else
    {
        p += tx_sprintf(p, "Radar interrupt Disabled\n");
        halIntDisable((tHalHandle) pMac, eHAL_INT_PHYINT_HOST_MASK_AGC_RADARDET_P);
    }
#endif
#endif
    return p;
}

char *
dump_phy_rx_counters( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    tANI_U32 i;

    typedef enum
    {
        RXA_BURSTS_REG,
        RXA_SINGLE_PKTS_REG,
        RXA_CONCAT_PKTS_REG,
        RXB_PACKETS_REG,
        RXA_DATA_VALID_REG,
        RXB_DATA_VALID_REG,
        RXP_SHUTOFF_REG,
        PHY_STARTS_REG,
        PHY_DATA_VAL_REG,
        PHY_ABORT_REG,
        PHY_INT_REG,
        NUM_PHYRX_REGS
    }eReadPhyRxRegs;

    tANI_U32 regValue[NUM_PHYRX_REGS];
    tANI_U32 regAddr[NUM_PHYRX_REGS] =
    {
#ifdef FIXME_GEN5
        PMI_RXA_BURSTS_REG,
        PMI_RXA_SINGLE_PKTS_REG,
        PMI_RXA_CONCAT_PKTS_REG,
#endif
        PMI_RXB_PACKETS_REG,
        PMI_RXA_DATA_VALID_REG,
        PMI_RXB_DATA_VALID_REG,
        PMI_RXP_SHUTOFF_REG,
        PMI_PHY_STARTS_REG,
        PMI_PHY_DATA_VAL_REG,
        PMI_PHY_ABORT_REG,
        PMI_PHY_INT_REG
    };

    char regStr[NUM_PHYRX_REGS][40] =
    {
        "PMI_RXA_BURSTS_REG",
        "PMI_RXA_SINGLE_PKTS_REG",
        "PMI_RXA_CONCAT_PKTS_REG",
        "PMI_RXB_PACKETS_REG",
        "PMI_RXA_DATA_VALID_REG",
        "PMI_RXB_DATA_VALID_REG",
        "PMI_RXP_SHUTOFF_REG",
        "PMI_PHY_STARTS_REG",
        "PMI_PHY_DATA_VAL_REG",
        "PMI_PHY_ABORT_REG",
        "PMI_PHY_INT_REG"
    };

    (void) arg1; (void) arg2; (void) arg3; (void) arg4;

    p += log_sprintf( pMac,p, "Phy Rx Counters\n");

    for (i = 0; i < NUM_PHYRX_REGS; i++)
    {
        halReadRegister(pMac, regAddr[i], &regValue[i]);
        p += log_sprintf( pMac,p, "%s = 0x%08X", regStr[i], regValue[i]);
    }

    return p;
}

char *
dump_hal_tpe_transmitted_pktcount_by_rate
( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p){
    return p;
}

char *
dump_hal_tpe_ratetable_send( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    (void) arg1; (void) arg2; (void) arg3; (void) arg4;
    halRate_UpdateTpeRateTable(pMac);
    return p;
}

char *
dump_hal_tpe_rspratetable_send( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    (void) arg1; (void) arg2; (void) arg3; (void) arg4;
    halRate_updateResponseRateTable(pMac);
    halRate_sendResponseRateTable(pMac);
    return p;
}

char *
dump_hal_tpe_ratetable( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    (void) arg3; (void) arg4;
    return p;
}

char *
dump_hal_tpe_rspratetable( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    (void) arg3; (void) arg4;
    return p;
}

char *
dump_hal_tpe_rsprate_change( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    (void) arg4;
    return p;
}

char *
dump_hal_tpe_ratetable_subband( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    (void) arg2; (void) arg3; (void) arg4;
    halRate_updateSubbandMode(pMac, (tANI_U8)arg1);
    return p;
}
#endif //FIXME_GEN6


char *
dump_hal_set_global_enable_rates
( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p){
    tANI_U32 rate;
    (void) arg4;
    for(rate=arg2; rate<=arg3; rate++){
        halRate_enableTpeRate(pMac, arg1, rate);
    }
    halRate_halRateInfoTableToFW(pMac, arg2, arg3);

//#ifndef WLAN_SOFTAP_FEATURE
#if 0
    //now update global sample rate table
    if(HAL_STA_INVALID_IDX != pMac->hal.halMac.selfStaId)
        /* this will invoke firmware to re-organize sampling table */
        halMacRaAddBssReq(pMac, 0, (tANI_U8)pMac->hal.halMac.selfStaId); /* is this ok ? bssIdx == 0 ? */
#endif //FIXME_VOLANS
    //retry rates would be updated next time when HAL selects a new rate

    return p;
}


static char *
dump_pMac_Size( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    (void) arg1; (void) arg2; (void) arg3; (void) arg4;
    HALLOGW( halLog(pMac, LOGW, FL("******** pMac Size Breakup ******** \n")));
    HALLOGW( halLog(pMac, LOGW, FL("pMac = %d\n"), sizeof(tAniSirGlobal)));
    HALLOGW( halLog(pMac, LOGW, FL("pMac->hal = %d\n"), sizeof(pMac->hal)));            
    HALLOGW( halLog(pMac, LOGW, FL("pMac->lim = %d\n"), sizeof(pMac->lim)));    
    HALLOGW( halLog(pMac, LOGW, FL("pMac->ccm = %d\n"), sizeof(pMac->ccm)));  
    HALLOGW( halLog(pMac, LOGW, FL("pMac->cfg = %d\n"), sizeof(pMac->cfg)));    
    HALLOGW( halLog(pMac, LOGW, FL("pMac->dumpTableEntry = %d\n"), sizeof(pMac->dumpTableEntry[0]) * MAX_DUMP_TABLE_ENTRY));    
    HALLOGW( halLog(pMac, LOGW, FL("pMac->pmc = %d\n"), sizeof(pMac->pmc)));    
    HALLOGW( halLog(pMac, LOGW, FL("pMac->pmm = %d\n"), sizeof(pMac->pmm)));    
    HALLOGW( halLog(pMac, LOGW, FL("pMac->pResetMsg = %d\n"), sizeof(pMac->pResetMsg[0])));    
    HALLOGW( halLog(pMac, LOGW, FL("pMac->roam = %d\n"), sizeof(pMac->roam)));    
    HALLOGW( halLog(pMac, LOGW, FL("pMac->scan = %d\n"), sizeof(pMac->scan)));    
    HALLOGW( halLog(pMac, LOGW, FL("pMac->sch = %d\n"), sizeof(pMac->sch)));    
    HALLOGW( halLog(pMac, LOGW, FL("pMac->sme = %d\n"), sizeof(pMac->sme)));    
    HALLOGW( halLog(pMac, LOGW, FL("pMac->sys = %d\n"), sizeof(pMac->sys)));    
#ifdef ANI_OS_TYPE_WINDOWS
    HALLOGW( halLog(pMac, LOGW, FL("pMac->txWrapper = %d\n"), sizeof(pMac->txWrapper)));
#endif
    HALLOGW( halLog(pMac, LOGW, FL("pMac->utils = %d\n"), sizeof(pMac->utils)));    
    HALLOGW( halLog(pMac, LOGW, FL("pMac->hPhy = %d\n"), sizeof(pMac->hphy)));        
    HALLOGW( halLog(pMac, LOGW, FL("******** pMac->hal Breakup ******** \n")));
    HALLOGW( halLog(pMac, LOGW, FL("pMac->hal.halIntErrStats = %d\n"), sizeof(pMac->hal.halIntErrStats)));
    HALLOGW( halLog(pMac, LOGW, FL("pMac->hal.halMac = %d\n"), sizeof(pMac->hal.halMac)));
    HALLOGW( halLog(pMac, LOGW, FL("pMac->hal.halRaInfo = %d\n"), sizeof(pMac->hal.halRaInfo)));
    HALLOGW( halLog(pMac, LOGW, FL("pMac->hal.intEnableCache = %d\n"), sizeof(pMac->hal.intEnableCache)));
    HALLOGW( halLog(pMac, LOGW, FL("pMac->hal.intStatusCache = %d\n"), sizeof(pMac->hal.intStatusCache)));
    HALLOGW( halLog(pMac, LOGW, FL("pMac->hal.memMap = %d\n"), sizeof(pMac->hal.memMap)));
    HALLOGW( halLog(pMac, LOGW, FL("pMac->hal.PsParam = %d\n"), sizeof(pMac->hal.PsParam)));
    HALLOGW( halLog(pMac, LOGW, FL("pMac->hal.scanParam = %d\n"), sizeof(pMac->hal.scanParam)));
    HALLOGW( halLog(pMac, LOGW, FL("pMac->hal.TLParam = %d\n"), sizeof(pMac->hal.TLParam)));
    HALLOGW( halLog(pMac, LOGW, FL("pMac->hal.halAdaptThresh = %d\n"), sizeof(pMac->hal.halAdaptThresh)));

    HALLOGW( halLog(pMac, LOGW, FL("***pMac->hal.halMac breakup****\n")));
    HALLOGW( halLog(pMac, LOGW, FL("pMac->hal.halMac.tspecInfo = %d\n"), sizeof(pMac->hal.halMac.tspecInfo)));
    HALLOGW( halLog(pMac, LOGW, FL("pMac->hal.halMac.macStats = %d\n"), sizeof(pMac->hal.halMac.macStats)));

    HALLOGW( halLog(pMac, LOGW, FL("*** Other Big data structure****\n")));
    HALLOGW( halLog(pMac, LOGW, FL("HAL station table = %d\n"), sizeof(tStaStruct)));
    HALLOGW( halLog(pMac, LOGW, FL("HAL RA info = %d\n"), sizeof(tHalRaInfo)));
    HALLOGW( halLog(pMac, LOGW, FL("HAL cfgSta = %d\n"), sizeof(tHalCfgSta)));

    HALLOGW( halLog(pMac, LOGW, FL("**********HAL dynamically allocated memory during open****\n")));
    HALLOGW( halLog(pMac, LOGW, FL("pMac->hal.halMac.dpuInfo = %d\n"), sizeof(tDpuInfo)));
    HALLOGW( halLog(pMac, LOGW, FL("pMac->hal.halMac.dpuInfo->descTable = %d\n"), sizeof(tHalDpuDescEntry) * (pMac->hal.halMac.maxSta + pMac->hal.halMac.maxBssId)));
    HALLOGW( halLog(pMac, LOGW, FL("pMac->hal.halMac.dpuInfo->keyTable = %d\n"), sizeof(tHalDpuKeyEntry)* (pMac->hal.halMac.maxSta + pMac->hal.halMac.maxBssId)));
    HALLOGW( halLog(pMac, LOGW, FL("pMac->hal.halMac.dpuInfo->micKeyEntry = %d\n"), sizeof(tHalDpuMicKeyEntry) * (pMac->hal.halMac.maxSta + pMac->hal.halMac.maxBssId)));
    HALLOGW( halLog(pMac, LOGW, FL("pMac->hal.halMac.dpuInfo->rcEntry = %d\n"), sizeof(tHalDpuRCEntry)* (pMac->hal.halMac.maxSta + pMac->hal.halMac.maxBssId)));
    HALLOGW( halLog(pMac, LOGW, FL("pMac->hal.FwParam->pFwConfig = %d\n"), sizeof(Qwlanfw_SysCfgType)));
    HALLOGW( halLog(pMac, LOGW, FL("pMac->hal.halMac.macStats.pPerStaStats = %d\n"), sizeof(tAniStaStats)*pMac->hal.halMac.maxSta));
    HALLOGW( halLog(pMac, LOGW, FL("pMac->hal.halMac.mboxInfo = %d\n"), (sizeof(tMboxInfo) * 2)));
    HALLOGW( halLog(pMac, LOGW, FL("pMac->hal.halMac.rxpInfo = %d\n"),  sizeof(tRxpInfo)));
    HALLOGW( halLog(pMac, LOGW, FL("pMac->hal.halMac.bssTable = %d\n"), sizeof(tBssStruct) * pMac->hal.halMac.maxBssId));
    HALLOGW( halLog(pMac, LOGW, FL("pMac->hal.halMac.staTable = %d\n"), sizeof(tStaStruct) * pMac->hal.halMac.maxSta));
    HALLOGW( halLog(pMac, LOGW, FL("pMac->hal.pHalDxe points to tHalDxe = %d\n"), sizeof(tAniHalDxe)));


    HALLOGW( halLog(pMac, LOGW, FL("******** pMac->lim Breakup ******** \n")));
    HALLOGW( halLog(pMac, LOGW, FL("pMac->lim.admitPolicyInfo = %d\n"), sizeof(pMac->lim.admitPolicyInfo)));
    HALLOGW( halLog(pMac, LOGW, FL("pMac->lim.cfgProtection = %d\n"), sizeof(pMac->lim.cfgProtection)));
    HALLOGW( halLog(pMac, LOGW, FL("pMac->lim.gHTMIMOPSState = %d\n"), sizeof(pMac->lim.gHTMIMOPSState)));
    HALLOGW( halLog(pMac, LOGW, FL("pMac->lim.gLimAlternateRadio = %d\n"), sizeof(pMac->lim.gLimAlternateRadio)));
#if (WNI_POLARIS_FW_PACKAGE == ADVANCED)
    HALLOGW( halLog(pMac, LOGW, FL("pMac->lim.gLimAlternateRadioList = %d\n"), sizeof(pMac->lim.gLimAlternateRadioList)));
#endif
    HALLOGW( halLog(pMac, LOGW, FL("pMac->lim.gLimAssocResponseData = %d\n"), sizeof(tSirAssocRsp)));
    HALLOGW( halLog(pMac, LOGW, FL("pMac->lim.gLimCachedScanHashTable = %d\n"), sizeof(pMac->lim.gLimCachedScanHashTable)));
    HALLOGW( halLog(pMac, LOGW, FL("pMac->lim.gLimChannelSwitch = %d\n"), sizeof(pMac->lim.gLimChannelSwitch)));
    HALLOGW( halLog(pMac, LOGW, FL("pMac->lim.gLimHeartBeatBeaconStats = %d\n"), sizeof(pMac->lim.gLimHeartBeatBeaconStats)));
    HALLOGW( halLog(pMac, LOGW, FL("pMac->lim.gLimLegacyBssidList = %d\n"), sizeof(pMac->lim.gLimLegacyBssidList)));
    HALLOGW( halLog(pMac, LOGW, FL("pMac->lim.gLimSpecMgmt = %d\n"), sizeof(pMac->lim.gLimSpecMgmt)));
    HALLOGW( halLog(pMac, LOGW, FL("pMac->lim.gLimWdsInfo = %d\n"), sizeof(pMac->lim.gLimWdsInfo)));
    HALLOGW( halLog(pMac, LOGW, FL("pMac->lim.ibssInfo = %d\n"), sizeof(pMac->lim.ibssInfo)));
    HALLOGW( halLog(pMac, LOGW, FL("pMac->lim.limTimers = %d\n"), sizeof(pMac->lim.limTimers)));
    HALLOGW( halLog(pMac, LOGW, FL("pMac->lim.protStaCache = %d\n"), sizeof(pMac->lim.protStaCache)));
    HALLOGW( halLog(pMac, LOGW, FL("pMac->lim.protStaOverlapCache = %d\n"), sizeof(pMac->lim.protStaOverlapCache)));
    HALLOGW( halLog(pMac, LOGW, FL("pMac->lim.scanChnInfo = %d\n"), sizeof(pMac->lim.scanChnInfo)));
    HALLOGW( halLog(pMac, LOGW, FL("pMac->lim.tspecInfo = %d\n"), sizeof(pMac->lim.tspecInfo)));
    HALLOGW( halLog(pMac, LOGW, FL("pMac->lim.wscIeInfo = %d\n"), sizeof(pMac->lim.wscIeInfo)));
    HALLOGW( halLog(pMac, LOGW, FL("pMac->lim.gLimLegacyBssidList = %d\n"), sizeof(pMac->lim.gLimLegacyBssidList)));
    HALLOGW( halLog(pMac, LOGW, FL("pMac->lim.scanChnInfo = %d\n"), sizeof(pMac->lim.scanChnInfo)));
    HALLOGW( halLog(pMac, LOGW, FL("pMac->lim.gLimHeartBeatBeaconStats = %d\n"), sizeof(pMac->lim.gLimHeartBeatBeaconStats)));
    HALLOGW( halLog(pMac, LOGW, FL("pMac->lim.gLimDeferredMsgQ = %d\n"), sizeof(pMac->lim.gLimDeferredMsgQ)));
    HALLOGW( halLog(pMac, LOGW, FL("pMac->lim.gLimAddtsReq = %d\n"), sizeof(pMac->lim.gLimAddtsReq)));
    HALLOGW( halLog(pMac, LOGW, FL("pMac->lim.gLimPreAuthTimerTable = %d\n"), sizeof(pMac->lim.gLimPreAuthTimerTable)));

    HALLOGW( halLog(pMac, LOGW, FL("**********PE dynamically allocated memory during open****\n")));

#ifdef ANI_PRODUCT_TYPE_AP
#endif

    HALLOGW( halLog(pMac, LOGW, FL("pMac->lim.limTimers.gpLimCnfWaitTimer = %d\n"), sizeof(TX_TIMER)*pMac->lim.maxStation));
    HALLOGW( halLog(pMac, LOGW, FL("pMac->lim.gpLimAIDpool = %d\n"), sizeof(tANI_U8)*pMac->lim.maxStation));
    HALLOGW( halLog(pMac, LOGW, FL("pMac->dph.dphHashTable.pHashTable = %d\n"), sizeof(tpDphHashNode)*pMac->lim.maxStation));

    HALLOGW( halLog(pMac, LOGW, FL("pMac->dph.dphHashTable.pDphNodeArray = %d\n"), sizeof(tDphHashNode)*pMac->lim.maxStation));
#ifdef ANI_PRODUCT_TYPE_AP
    HALLOGW( halLog(pMac, LOGW, FL("pMac->pmm.gPmmTim.pTim = %d\n"), sizeof(tANI_U8)*pMac->lim.maxStation));
    HALLOGW( halLog(pMac, LOGW, FL("pMac->pmm.gPmmTim.pStaInfo = %d\n"), sizeof(*pMac->pmm.gPmmTim.pStaInfo) * pMac->lim.maxStation));
    HALLOGW( halLog(pMac, LOGW, FL("pMac->pmm.gpPmmStaState = %d\n"), sizeof(tPmmStaState)*pMac->lim.maxStation));
    HALLOGW( halLog(pMac, LOGW, FL("pMac->pmm.gpPmmPSState = %d\n"), sizeof(tANI_U8)*pMac->lim.maxStation));
#endif

    HALLOGW(halLog(pMac, LOGW, FL("**********PE Other big data structures****\n")));
    HALLOGW( halLog(pMac, LOGW, FL("pMac->lim.gpLimStartBssReq points to tSirSmeStartBssReqof size = %d\n"), sizeof(tSirSmeStartBssReq)));
    HALLOGW(halLog(pMac, LOGW, FL("dph hash node = %d \n"), sizeof(tDphHashNode)));
    HALLOGW(halLog(pMac, LOGW, FL("dph hash node field mlmStaContext = %d \n"), sizeof(tLimMlmStaContext)));
    HALLOGW(halLog(pMac, LOGW, FL("dph hash node field qos = %d \n"), sizeof(tDphQosParams)));
    HALLOGW(halLog(pMac, LOGW, FL("dph hash node field supportedRates = %d \n"), sizeof(tSirSupportedRates)));

    HALLOGW(halLog(pMac, LOGW, FL("**********pMac->ccm breakup****\n")));
    HALLOGW( halLog(pMac, LOGW, FL("pMac->ccm.comp = %d\n"), sizeof(pMac->ccm.comp)));


    HALLOGW(halLog(pMac, LOGW, FL("**********pMac->sch breakup****\n")));
    HALLOGW( halLog(pMac, LOGW, FL("pMac->sch.dataFeedbackHist(not in use - TBR) = %d\n"), sizeof(pMac->sch.dataFeedbackHist)));
    HALLOGW( halLog(pMac, LOGW, FL("pMac->sch.schObject = %d\n"), sizeof(pMac->sch.schObject)));
    HALLOGW( halLog(pMac, LOGW, FL("pMac->sch.pollFeedbackHist = %d\n"), sizeof(pMac->sch.pollFeedbackHist)));

    return p;
}
static char *
dump_hal_rxp_hw_searchtable( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    (void) arg1; (void) arg2; (void) arg3; (void) arg4;
    halLog_printRxpBinarySearchTable(pMac);
    return p;
}

#define NUM_BSSID 2
static tSirMacAddr addr_bssid[NUM_BSSID] = {
    {0,0,0,0,1,0},
    {0,0,0,0,2,0}};

#define NUM_STAID 24
static tSirMacAddr addr_staid[NUM_STAID] = {
    {0,0,0,0,0,3},
    {0,0,0,0,0,4},
    {0,0,0,0,0,5},
    {0,0,0,0,0,6},
    {0,0,0,0,0,7},
    {0,0,0,0,0,8},
    {0,0,0,0,0,9},
    {0,0,0,0,0,10},
    {0,0,0,0,0,11},
    {0,0,0,0,0,12},
    {0,0,0,0,0,13},
    {0,0,0,0,0,14},
    {0,0,0,0,0,15},
    {0,0,0,0,0,16},
    {0,0,0,0,0,17},
    {0,0,0,0,0,18},
    {0,0,0,0,0,19},
    {0,0,0,0,0,20},
    {0,0,0,0,0,21},
    {0,0,0,0,0,22},
    {0,1,0,0,0,4},
    {0,2,0,0,0,3},
    {0,3,0,0,0,2},
    {0,4,0,0,0,1}
};

static tANI_U16 dialogToken = 0;

static char *
dump_set_ba_activity_check_timeout( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    (void) arg2; (void) arg3; (void) arg4;

      if(arg1)
     {
        if(TX_SUCCESS !=  tx_timer_activate(&pMac->hal.halMac.baActivityChkTmr))
          {
              p += log_sprintf( pMac, p, "\n:  BA activity check timer could not get activated\n");
          }
          else
          {
              p += log_sprintf( pMac, p, "\n:  BA activity check timer activated\n");
          }
      }
      else
      {
        if(TX_SUCCESS != tx_timer_deactivate(&pMac->hal.halMac.baActivityChkTmr))
          {
              p += log_sprintf( pMac, p, "\n:  BA activity check timer could not get deactivated\n");
          }
          else
          {
              p += log_sprintf( pMac, p, "\n:  BA activity check timer deactivated\n");
          }
      }
      return p;
}


#if (defined(ANI_OS_TYPE_ANDROID) || defined(ANI_OS_TYPE_LINUX))
void getFrmTemplate(tpAniSirGlobal pMac, tANI_U32 templateType /*0 = templateList, 1 = btqmTemplateList*/,
            tANI_U32 templateNum, tANI_U8* templateBuf, tANI_U32* templateFrmSize)
{
    int status, i = 0, j = 0;

   /** Pointer for firmware image data */
   const struct firmware *fw;
   hdd_adapter_t* pAdapter = (hdd_adapter_t*)pMac->pAdapter; 
   char *buffer;
   char buffer1[2048];
   tANI_U32 val = 0;

   *templateFrmSize = 0;
   if(templateNum > 9)
     return;
   
   memset(buffer1, 0, sizeof(buffer1));

   if(templateType == 0)
   {
       status = request_firmware(&fw, templateList[templateNum].templateFileName, &(pAdapter->hsdio_func_dev->dev));
   }
   else if(templateType == 1)
   {
       status = request_firmware(&fw, btqmTemplateList[templateNum].templateFileName, &(pAdapter->hsdio_func_dev->dev));
   }
    
   
   if(!fw || !fw->data) 
   {
      halLog(pMac, LOGE, FL("%s: template download failed\n"));
	  return;
   } 

   buffer = (char *)fw->data;

   for(i = 0; i < fw->size; i++)
   {
       if((buffer[i] >= 48) && (buffer[i] <= 57))
       {
           buffer1[i] = buffer[i] - 48;
       }
       else if((buffer[i] >= 97) && (buffer[i] <= 102))
       {
           buffer1[i] = buffer[i] - 87;
       }
       else if((buffer[i] >= 65) && (buffer[i] <= 70))
       {
           buffer1[i] = buffer[i] - 55;
       }
       
       else
           continue;
  
       if(j%2)
       {
           val += buffer1[i];
           templateBuf[*templateFrmSize] = val;
           (*templateFrmSize)++;
           halLog(pMac, LOGE, FL("%x\n"), val);
       }
       else
       {
           val = buffer1[i] * 16;
       }
       j++;
   }
  

   release_firmware(fw);
   return;
}


static char* SendTemplateFrame(tpAniSirGlobal pMac, tANI_U32 templateNum, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char* p)
{
#ifdef CONFIGURE_SW_TEMPLATE

#if (defined(ANI_OS_TYPE_ANDROID) || defined(ANI_OS_TYPE_LINUX))

    tSwTemplate swTemplate;
    tTpeRateIdx rateIndex = TPE_RT_IDX_11B_RATE_LONG_PR_BASE_OFFSET;
    tANI_U32    alignedLen;
    tANI_U32    templateFrmSize = 0;
    eHalStatus status = eHAL_STATUS_SUCCESS;
    static tANI_U8 swBaseTemplateInit = FALSE;
    tANI_U8 templateBuf[2048];

    (void)arg2; (void)arg3; (void)arg4;        
    /** Initialize SW Template base */
    if (swBaseTemplateInit == FALSE) {
        if (halTpe_InitSwTemplateBase(pMac, pMac->hal.memMap.swTemplate_offset) 
            != eHAL_STATUS_SUCCESS) {
            return p;    
        }
        swBaseTemplateInit = TRUE;
    }

    /** Zero out the SW Template memory */
    halZeroDeviceMemory(pMac, pMac->hal.memMap.swTemplate_offset, 
                                    sizeof (tSwTemplate) + sizeof( tANI_U32 ));

    palZeroMemory(pMac->hHdd, templateBuf, sizeof(templateBuf));

    palZeroMemory(pMac->hHdd, &swTemplate, sizeof(tSwTemplate));

    //allocate template buffer.
    //pass a stuct which will have buffer as well as ohter related filed like type/subtype etc.
    getFrmTemplate(pMac, 0 /*templateList*/, templateNum, templateBuf, &templateFrmSize);


    swTemplate.template_type     = templateList[templateNum].template_type;
    swTemplate.template_sub_type     = templateList[templateNum].template_subtype;
    swTemplate.expected_resp_sub_type = templateList[templateNum].template_resp_subtype;
    swTemplate.expected_resp_type     = templateList[templateNum].template_resp_type;
    swTemplate.ignore_expected_resp = templateList[templateNum].template_ignore_expected_resp;
    swTemplate.resp_is_expected    = templateList[templateNum].template_resp_is_expected;
    halGetNonBcnRateIdx(pMac, &rateIndex);
    swTemplate.primary_data_rate_index = rateIndex;
    swTemplate.template_len = templateFrmSize + SW_TEMPLATE_CRC;
    status = halWriteDeviceMemory(pMac, 
            pMac->hal.memMap.swTemplate_offset,
            (tANI_U8 *)&swTemplate, SW_TEMPLATE_HEADER_LEN);

    alignedLen = (templateFrmSize + 3 ) & ~3 ;

    //need to be swapped since there is another swap occurs while BAL writes
    sirSwapU32BufIfNeeded((tANI_U32*)&templateBuf, alignedLen >> 2);

    status = halWriteDeviceMemory(pMac, pMac->hal.memMap.swTemplate_offset + SW_TEMPLATE_HEADER_LEN,
            (tANI_U8 *)&templateBuf, alignedLen);

    halTpe_TriggerSwTemplate(pMac);

#endif
#endif //#ifdef CONFIGURE_SW_TEMPLATE
    return p;
}



static char* SendFrmBdIdx(tpAniSirGlobal pMac, tANI_U32 staIdx, tANI_U32 qid, tANI_U32 templateNum, tANI_U32 arg4, char *p)
{
    tANI_U32 bmuPushBdCmd, bmuGetBdCmd, regVal = 0;
    
    tANI_U32 alignedLen, templateOffset;
    halTxBd_type *pTxBd,  txBd;
    tANI_U32 bdIdx = 0;
    tANI_U32 mpduLen = 0;
    tANI_U8 buff[2048];

    if((staIdx >= HAL_NUM_STA)|| (qid > pMac->hal.memMap.maxHwQueues) ||(templateNum > 9))
        return p;
    vos_mem_zero(buff, sizeof(buff));

    getFrmTemplate(pMac, 1 /*btqmTemplateList*/, templateNum, buff, &mpduLen);

    bmuGetBdCmd = (BMU_COMMAND_BASE_ADDRESS    |
              1 << BMU_NUM_BD_GET_SHIFT   |
              8 << BMU_MASTER_ID_SHIFT    |
              GET_BD_PDU_CMDTYPE);

    palReadRegister(pMac->hHdd, bmuGetBdCmd, &bdIdx);

    palReadRegister(pMac->hHdd, QWLAN_MCU_BD_PDU_BASE_ADDR_REG, &regVal);
    pTxBd = (halTxBd_type*) (regVal + bdIdx * BMU_BD_SIZE);
    halZeroDeviceMemory(pMac, (tANI_U32)pTxBd, BMU_BD_SIZE);

    //Fill in txbd.
    vos_mem_zero(&txBd, sizeof(txBd));
    txBd.txComplete1 = 0;
    txBd.queueId = qid; 
    txBd.mpduHeaderOffset = sizeof(halTxBd_type);
    txBd.staIndex = staIdx;
    txBd.bdRate = 0x0;
    txBd.mpduHeaderLength = btqmTemplateList[templateNum].mpduHdrLen;
    txBd.mpduLength = mpduLen; 
    txBd.mpduDataOffset = sizeof(halTxBd_type)+ txBd.mpduHeaderLength;

    halWriteDeviceMemory(pMac, (tANI_U32)pTxBd, &txBd, sizeof(halTxBd_type));

    //halWriteDevicememory requires length to be mulltiple of four and aligned to 4 byte boundry.
    alignedLen = ( mpduLen + 3 ) & ~3 ;

    // beacon body need to be swapped sicne there is another swap occurs while BAL writes

    sirSwapU32BufIfNeeded((tANI_U32*)buff, alignedLen>>2);

    templateOffset = (tANI_U32) (((tANI_U8*)pTxBd) + sizeof(halTxBd_type));

    halWriteDeviceMemory(pMac, templateOffset,
                            (tANI_U8 *)buff, alignedLen );
    

    // send to BTQM
    bmuPushBdCmd = BMU_COMMAND_BASE_ADDRESS |
         BMUWQ_BTQM << 8 |
         PUSH_WQ_CMDTYPE;

    // push the frame to the next queue
    palWriteRegister(pMac->hHdd, bmuPushBdCmd, bdIdx);

    return p;
}

#endif

#ifdef WLAN_DEBUG
tANI_U8 beaconPayload[] = {0xbe, 0x00, 0x00, 0x00, //beacon length
                              0x80, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xde, 0xad, 0xbe, 0xef, 0x00,
                              0x00, 0xde, 0xad, 0xbe, 0xef, 0x00, 0x00, 0x00, 0x87, 0x51, 0xd0, 0x32, 0x11, 0x00, 0x00, 0x00,
                              0x64, 0x00, 0x21, 0x04, 0x00, 0x08, 0x51, 0x75, 0x61, 0x6c, 0x63, 0x6f, 0x6d, 0x6d, 0x01, 0x04,
                              0x82, 0x84, 0x8b, 0x96, 0x03, 0x01, 0x06, 0x05, 0x04, 0x00, 0x01, 0x00, 0x00, 0x2a, 0x01, 0x00,
                              0x32, 0x08, 0x0c, 0x12, 0x18, 0x24, 0x30, 0x48, 0x60, 0x6c, 0x2d, 0x1a, 0xec, 0x91, 0x03, 0xff,
                              0xff, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3d, 0x16, 0x06, 0x08, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00,
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xdd, 0x18,
                              0x00, 0x50, 0xf2, 0x02, 0x01, 0x01, 0x81, 0x00, 0x03, 0xa4, 0x00, 0x00, 0x27, 0xa4, 0x00, 0x00,
                              0x42, 0x43, 0x5e, 0x00, 0x62, 0x32, 0x2f, 0x00, 0xdd, 0x14, 0x00, 0x0a, 0xf5, 0x00, 0x03, 0x01,
                              0x03, 0x05, 0x0a, 0x02, 0x80, 0xc0, 0x12, 0x06, 0xff, 0xff, 0xff, 0xff, 0xb0, 0x0d, 0xdd, 0x0e,
                              0x00, 0x50, 0xf2, 0x04, 0x10, 0x4a, 0x00, 0x01, 0x10, 0x10, 0x44, 0x00, 0x01, 0x01};

tANI_U8 probeRspPayLoad[] = { 0x50, 0x00, 0x3A, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
                                 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x20, 0x4D, 0xC1, 0xFD, 0xBD, 0xF7, 0x0C, 0x00, 0x00, 0x00,
                                 0x64, 0x00, 0x21, 0x00, 0x00, 0x08, 0x51, 0x75, 0x61, 0x6C, 0x63, 0x6F, 0x6D, 0x6D, 0x01, 0x04,
                                 0x82, 0x84, 0x8B, 0x0C, 0x03, 0x01, 0x01, 0x2A, 0x01, 0x00, 0x85, 0x1E, 0x00, 0x00, 0x84, 0x00,
                                 0x0F, 0x00, 0xFF, 0x03, 0x19, 0x00, 0x61, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x25, 0xDD, 0x06, 0x00, 0x40, 0x96, 0x01,
                                 0x01, 0x00, 0xDD, 0x05, 0x00, 0x40, 0x96, 0x03, 0x04, 0xDD, 0x05, 0x00, 0x40, 0x96, 0x0B, 0x01,
                                 0xDD, 0x18, 0x00, 0x50, 0xF2, 0x02, 0x01, 0x01, 0x81, 0xDD, 0x18, 0x00, 0x50, 0xF2, 0x02, 0x01,
                                 0x01, 0x81, 0x00, 0x03, 0xA4, 0x00, 0x00, 0x27, 0xA4, 0x00, 0x00, 0x42, 0x43, 0x5E, 0x00, 0x62,
                                 0x32, 0x2F, 0x00};

#endif

#ifdef WLAN_SOFTAP_FEATURE
static char* dump_updateProbeRspTemplate(tpAniSirGlobal pMac, tANI_U32 bssIdx, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char* p)
{
#ifdef WLAN_DEBUG
    tSirMsgQ msgQ;
    tpUpdateProbeRspParams probeRspParams = NULL;

    if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd,
            (void **) &probeRspParams,
            sizeof( tUpdateProbeRspParams )))
      return p;

    msgQ.type = SIR_HAL_UPDATE_PROBE_RSP_TEMPLATE_IND;

    // No Dialog Token reqd, as a response is not solicited
    msgQ.reserved = 0;

    // Fill in tSendbeaconParams members
    /* Knock off all pMac global addresses */
    // limGetBssid( pMac, beaconParams->bssId);
    palCopyMemory(pMac, probeRspParams->bssId, &addr_bssid[bssIdx], sizeof(tSirMacAddr));
    probeRspParams->probeRsp = probeRspPayLoad;
    probeRspParams->probeRspLength = sizeof(probeRspPayLoad);
    msgQ.bodyptr = probeRspParams;
    msgQ.bodyval = 0;
    if(eSIR_SUCCESS != halPostMsgApi( pMac, &msgQ ))
    {
        palFreeMemory(pMac, (void*)probeRspParams);    
        halLog( pMac, LOGE,
              FL("could not post SIR_HAL_UPDATE_PROBE_RSP_TEMPLATE_IND to HAL\n"));
    }


#endif

    return p;
}
#endif
void halLog_SendBeaconReq( tpAniSirGlobal pMac, tANI_U8 bssIdx)
{
#ifdef WLAN_DEBUG
    tSirMsgQ msgQ;
    tpSendbeaconParams beaconParams = NULL;
#ifdef WLAN_SOFTAP_FEATURE    
    tANI_U32 timIeOffset = BEACON_TEMPLATE_HEADER + 55; //template header length + TIM ie offset starting from the beacon.
#endif    

  if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd,
          (void **) &beaconParams,
          sizeof( tSendbeaconParams )))
    return;

  msgQ.type = SIR_HAL_SEND_BEACON_REQ;

  // No Dialog Token reqd, as a response is not solicited
  msgQ.reserved = 0;

  // Fill in tSendbeaconParams members
  /* Knock off all pMac global addresses */
  // limGetBssid( pMac, beaconParams->bssId);
  palCopyMemory(pMac, beaconParams->bssId, &addr_bssid[bssIdx], sizeof(tSirMacAddr));
  beaconParams->beacon = beaconPayload;
  beaconParams->beaconLength = sizeof(beaconPayload);
#ifdef WLAN_SOFTAP_FEATURE  
  beaconParams->timIeOffset = timIeOffset;
#endif  
  msgQ.bodyptr = beaconParams;
  msgQ.bodyval = 0;
  if(eSIR_SUCCESS != halPostMsgApi( pMac, &msgQ ))
  {
    palFreeMemory(pMac, (void*)beaconParams);  
    halLog( pMac, LOGE,
            FL("could not post SIR_HAL_SEND_BEACON_REQ to HAL\n"));
  }
#endif
  return;
}

static char* dump_setLinkState( tpAniSirGlobal pMac, tANI_U32 bssIdx, tANI_U32 state, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    tSirMsgQ msgQ;
    tpLinkStateParams pLinkStateParams = NULL;
    tpBssStruct pBss;
    tpBssStruct t = (tpBssStruct) pMac->hal.halMac.bssTable;    


    if(bssIdx >= HAL_NUM_BSSID)
    {
        HALLOGE(halLog(pMac, LOGE, FL("invalid bssIdx requested\n")));
        return p;
    }
        
    pBss = &t[bssIdx];

    if(state >  eSIR_LINK_FINISH_CAL_STATE) //greater than the max state
    {
        HALLOGE(halLog(pMac, LOGE, FL("invalid link state requested\n")));
        return p;
    }
    // Allocate memory.
    if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd,
          (void **) &pLinkStateParams,
          sizeof(tLinkStateParams)))
    {
        halLog( pMac, LOGP,
        FL( "Unable to PAL allocate memory while sending Set Link State\n" ));
        return p;
    }

    palZeroMemory( pMac->hHdd, (tANI_U8 *) pLinkStateParams, sizeof(tLinkStateParams));

    pLinkStateParams->state = state;

    /* Copy Mac address */
    sirCopyMacAddr(pLinkStateParams->bssid, pBss->bssId);


    msgQ.type = SIR_HAL_SET_LINK_STATE;
    msgQ.reserved = 0;
    msgQ.bodyptr = pLinkStateParams;
    msgQ.bodyval = 0;
    
    if (halPostMsgApi(pMac, &msgQ) != eSIR_SUCCESS)
    {
        palFreeMemory(pMac, (void*)pLinkStateParams);
        halLog(pMac, LOGP, FL("Posting link state failed\n"));
    }

    return p;
}

#ifdef WLAN_SOFTAP_FEATURE
static char* dump_setProbeRspIeBitmap(tpAniSirGlobal pMac, tANI_U32 enable, tANI_U32 enableDisableAllIe, tANI_U32 arg3, tANI_U32 arg4, char* p)
{
    tpUpdateProbeRspIeBitmap pMsg;
    tSirMsgQ halMsg;

    if (palAllocateMemory(pMac->hHdd, (void **)&pMsg, sizeof(tpUpdateProbeRspIeBitmap)) != eHAL_STATUS_SUCCESS)
    {
        HALLOGW( halLog(pMac, LOGW, FL("palAllocateMemory Failed\n")));
        return p;
    }

    pMsg->fwProcessingdisabled = (!enable);
    if(!enableDisableAllIe) //disable all IEs
    {
        vos_mem_zero(pMsg->probeRspIeBitmap, sizeof(pMsg->probeRspIeBitmap));
    }
    else
    {//enable all IEs
        vos_mem_set(pMsg->probeRspIeBitmap, sizeof(pMsg->probeRspIeBitmap), 0xff);
    }
    halMsg.type = SIR_HAL_UPDATE_PROBE_RSP_IE_BITMAP_IND;
    halMsg.reserved = 0;
    halMsg.bodyptr = pMsg;
    halMsg.bodyval = 0;
    
    if (halPostMsgApi(pMac, &halMsg) != eSIR_SUCCESS)
    {
        palFreeMemory(pMac, (void*)pMsg);
        halLog(pMac, LOGP, FL("Posting  SIR_HAL_UPDATE_PROBE_RSP_IE_BITMAP_IND failed\n"));
    }
    return p;
}
#endif

static void
halLog_testAddBss(tpAniSirGlobal pMac, tANI_U32 bssType, tANI_U32 bssidNum)
{
    tpAddBssParams pMsg;
    tSirMsgQ halMsg;
    tSirMacAddr *bssid;
#define MAX_DUMP_ADD_BSS_BSSTYPE 6

    if (bssType > MAX_DUMP_ADD_BSS_BSSTYPE)
    {
        HALLOGW( halLog(pMac, LOGW, FL("ERROR - mode should be 1 for AP (START BSS), 2 for STA (Join BSS)\n"
            "3 for STA (Create IBSS)\n"
            "4 for BTAMP AP Add Bss Req\n"
            "5 for BTAMP STA Add Bss Req\n"
            )));
        return;
    }

    if (bssidNum >= NUM_BSSID)
    {
        HALLOGW( halLog( pMac, LOGW, FL("ERROR - bssid should be in [0,%d]\n"),  NUM_BSSID ));
        return;
    }

    if (palAllocateMemory(pMac->hHdd, (void **)&pMsg, sizeof(tAddBssParams)) != eHAL_STATUS_SUCCESS)
    {
        HALLOGW( halLog(pMac, LOGW, FL("palAllocateMemory Failed\n")));
        return;
    }

    bssid = &addr_bssid[bssidNum];
    palZeroMemory(pMac->hHdd, (void *) pMsg, sizeof (tAddBssParams));

    palCopyMemory(pMac->hHdd, (void *) &pMsg->bssId, (void *)bssid, 6);

    // Default
    pMsg->bssType = eSIR_INFRASTRUCTURE_MODE;
    pMsg->operMode = BSS_OPERATIONAL_MODE_STA;
    palCopyMemory( pMac->hHdd,  pMsg->selfMacAddr, bssid, sizeof(tSirMacAddr));

    switch (bssType) {
        case 1:
            pMsg->operMode = BSS_OPERATIONAL_MODE_AP;
            {
                int      i;
                tpAddStaParams  pSta = &pMsg->staContext;
                

                pSta->staType = STA_ENTRY_SELF; // Identifying self
                palCopyMemory( pMac->hHdd,  pSta->bssId,   bssid,  sizeof( tSirMacAddr ));
                palCopyMemory( pMac->hHdd,  pSta->staMac, bssid, sizeof(tSirMacAddr));
                pSta->listenInterval = 2;
                pSta->shortPreambleSupported = 1;
                pSta->assocId               = 0; 
                pSta->wmmEnabled            = 0;
                pSta->uAPSD                 = 0;
                pSta->maxSPLen              = 0;
                pSta->us32MaxAmpduDuration  = 0;
                pSta->maxAmpduSize          = 0; // 0: 8k, 1: 16k,2: 32k,3: 64k


#if 0 
                if(IS_DOT11_MODE_HT(psessionEntry->dot11mode)) 
                {
                    pSta->htCapable         = htCapable;
                    pSta->greenFieldCapable = limGetHTCapability( pMac, eHT_GREENFIELD );
                    pSta->txChannelWidthSet = limGetHTCapability( pMac, eHT_SUPPORTED_CHANNEL_WIDTH_SET );
                    pSta->mimoPS            = (tSirMacHTMIMOPowerSaveState)limGetHTCapability( pMac, eHT_MIMO_POWER_SAVE );
                    pSta->rifsMode          = limGetHTCapability( pMac, eHT_RIFS_MODE );
                    pSta->lsigTxopProtection = limGetHTCapability( pMac, eHT_LSIG_TXOP_PROTECTION );
                    pSta->delBASupport      = limGetHTCapability( pMac, eHT_DELAYED_BA );
                    pSta->maxAmpduDensity   = limGetHTCapability( pMac, eHT_MPDU_DENSITY );
                    pSta->maxAmsduSize      = limGetHTCapability( pMac, eHT_MAX_AMSDU_LENGTH );
                    pSta->fDsssCckMode40Mhz = limGetHTCapability( pMac, eHT_DSSS_CCK_MODE_40MHZ);
                    pSta->fShortGI20Mhz     = limGetHTCapability( pMac, eHT_SHORT_GI_20MHZ);
                    pSta->fShortGI40Mhz     = limGetHTCapability( pMac, eHT_SHORT_GI_40MHZ);
                }

                limPopulateOwnRateSet(pMac, &pSta->supportedRates, NULL, false,psessionEntry);
                limFillSupportedRatesInfo(pMac, NULL, &pSta->supportedRates,psessionEntry);
                
                limLog( pMac, LOGE, FL( "GF: %d, ChnlWidth: %d, MimoPS: %d, lsigTXOP: %d, dsssCCK: %d, SGI20: %d, SGI40%d\n") ,
                                                      pSta->greenFieldCapable, pSta->txChannelWidthSet, pSta->mimoPS, pSta->lsigTxopProtection, 
                                                      pSta->fDsssCckMode40Mhz,pSta->fShortGI20Mhz, pSta->fShortGI40Mhz);

#endif

                //Disable BA. It will be set as part of ADDBA negotiation.
                for( i = 0; i < STACFG_MAX_TC; i++ )
                {
                    pSta->staTCParams[i].txUseBA = eBA_DISABLE;
                    pSta->staTCParams[i].rxUseBA = eBA_DISABLE;
                }
                
            }            
            HALLOGW(halLog(pMac, LOGW, "AddBss infra adding bssid %d-%d-%d-%d-%d-%d as %s",
                (*bssid)[0], (*bssid)[1], (*bssid)[2],
                (*bssid)[3], (*bssid)[4], (*bssid)[5],
                "AP"));
            break;

        case 3:
            pMsg->bssType = eSIR_IBSS_MODE;
            HALLOGW(halLog(pMac, LOGW, "AddBss IBSS adding bssid %d-%d-%d-%d-%d-%d as %s",
                (*bssid)[0], (*bssid)[1], (*bssid)[2],
                (*bssid)[3], (*bssid)[4], (*bssid)[5],
                "STA"));
            break;

        case 4: // BTAMP-AP
            pMsg->bssType = eSIR_BTAMP_AP_MODE;
            pMsg->operMode = BSS_OPERATIONAL_MODE_AP;
            pMsg->staContext.staType = STA_ENTRY_SELF; // AP sends out only 1 type 
            HALLOGW(halLog(pMac, LOGW, "AddBss BTAMP-AP adding bssid %d-%d-%d-%d-%d-%d as %s",
                (*bssid)[0], (*bssid)[1], (*bssid)[2],
                (*bssid)[3], (*bssid)[4], (*bssid)[5],
                "AP")); 
            break;

        case 5:
            pMsg->bssType = eSIR_BTAMP_STA_MODE;
            pMsg->operMode = BSS_OPERATIONAL_MODE_STA;
            pMsg->staContext.staType = STA_ENTRY_SELF; // STA sends out 2 types. this is the 1st.
            HALLOGW(halLog(pMac, LOGW, "AddBss BTAMP-STA adding bssid %d-%d-%d-%d-%d-%d as %s",
                (*bssid)[0], (*bssid)[1], (*bssid)[2],
                (*bssid)[3], (*bssid)[4], (*bssid)[5],
                "STA")); 
            break;

        case 6:
            pMsg->bssType = eSIR_BTAMP_AP_MODE;  // BTAMP STA is requesting joining the BTAMP-AP
            pMsg->operMode = BSS_OPERATIONAL_MODE_STA;
            pMsg->staContext.staType = STA_ENTRY_PEER; // STA sends out 2 types. this is the 2nd.
            HALLOGW(halLog(pMac, LOGW, "AddBss BTAMP-STA adding bssid %d-%d-%d-%d-%d-%d as %s",
                (*bssid)[0], (*bssid)[1], (*bssid)[2],
                (*bssid)[3], (*bssid)[4], (*bssid)[5],
                "STA")); 
            break;

        default:
            HALLOGW(halLog(pMac, LOGW, "AddBss infra adding bssid %d-%d-%d-%d-%d-%d as %s",
                (*bssid)[0], (*bssid)[1], (*bssid)[2],
                (*bssid)[3], (*bssid)[4], (*bssid)[5],
                "STA")); 
            break;
    }


    pMsg->beaconInterval = 500;
    pMsg->dtimPeriod = 20;
    pMsg->respReqd = 0;
    pMsg->nwType = eSIR_11G_NW_TYPE;
    pMsg->currentOperChannel = 1;

    halMsg.type =  SIR_HAL_ADD_BSS_REQ;
    halMsg.reserved = dialogToken++;
    halMsg.bodyptr = pMsg;
    halMsg.bodyval = 0;

    if (halPostMsgApi(pMac, &halMsg) != eHAL_STATUS_SUCCESS)
        palFreeMemory(pMac->hHdd, pMsg);

    return;
}

#ifdef WLAN_SOFTAP_FEATURE
//uapsdMask : LSB 4 bits for delivery enabled AC. MSB 4 bits for trigger enabled AC. 1 bit per AC.
//  b7  b6  b5  b4  b3  b2  b1  b0
//  BE  BK VI   VO BE  BK VI  VO
static char* 
halLog_updateUapsd(tpAniSirGlobal pMac, tANI_U32 staIdx, tANI_U32 uapsdACMask, tANI_U32 maxSpLen, tANI_U32 arg4, char* p)
{
    tSirMsgQ halMsg;
    tpUpdateUapsdParams pMsg;

    (void) arg4;

    if (palAllocateMemory(pMac->hHdd, (void **)&pMsg, sizeof(tUpdateUapsdParams)) != eHAL_STATUS_SUCCESS)
    {
        HALLOGW( halLog(pMac, LOGW, FL("palAllocateMemory Failed\n")));
        return p;
    }

    pMsg->staIdx = (tANI_U16) staIdx;
    pMsg->uapsdACMask = (tANI_U8) uapsdACMask;
    pMsg->maxSpLen = (tANI_U8) maxSpLen;
    halMsg.type = SIR_HAL_UPDATE_UAPSD_IND;
    halMsg.reserved = dialogToken++;
    halMsg.bodyptr = pMsg;
    halMsg.bodyval = 0;

    if (halPostMsgApi(pMac, &halMsg) != eHAL_STATUS_SUCCESS)
        palFreeMemory(pMac->hHdd, pMsg);

    return p;
}

static char* 
dump_sendFcFrameToFw(tpAniSirGlobal pMac, tANI_U32 staIdx, tANI_U32 memTh, tANI_U32 fcConfig, tANI_U32 arg4, char* p)
{
    tANI_U32 bdIdx = 0, bmuGetBdCmd = 0, bmuPushBdCmd = 0, regVal = 0;
    halFcTxBd_type *pFcTxBd;
    halFcTxBd_type fcTxBd;

    if(staIdx >= HAL_NUM_STA)
        return p;

    bmuGetBdCmd = (BMU_COMMAND_BASE_ADDRESS    |
              1 << BMU_NUM_BD_GET_SHIFT   |
              8 << BMU_MASTER_ID_SHIFT    |
              GET_BD_PDU_CMDTYPE);

    palReadRegister(pMac->hHdd, bmuGetBdCmd, &bdIdx);

    palReadRegister(pMac->hHdd, QWLAN_MCU_BD_PDU_BASE_ADDR_REG, &regVal);
    pFcTxBd = (halFcTxBd_type*) (regVal + bdIdx * BMU_BD_SIZE);
    halZeroDeviceMemory(pMac, (tANI_U32)pFcTxBd, BMU_BD_SIZE);

    //fill flow control BD
    vos_mem_zero(&fcTxBd, sizeof(fcTxBd));
    fcTxBd.fc = 1;
    fcTxBd.dpuNE = HAL_NO_ENCRYPTION_ENABLED;
    fcTxBd.dpuRF = BMUWQ_FW_DPU_TX;
    fcTxBd.mpduHeaderLength = FC_REQUST_MPDU_HDR_LEN;
    fcTxBd.mpduHeaderOffset = FC_REQUST_MPDU_HDR_START_OFFSET;
    fcTxBd.mpduLength = FC_REQUST_MPDU_LEN;
    //fcTxBd.ft = 0;
    fcTxBd.fcConfig = (fcConfig & 0xff);
    fcTxBd.fcSTAThreshEnabledMask = (1 << staIdx); //for station id 2.
    fcTxBd.fcSTAThresh[staIdx] = (memTh & 0xff);

    halWriteDeviceMemory(pMac, (tANI_U32)pFcTxBd, &fcTxBd, sizeof(halFcTxBd_type));

    // send to FW queue.
    bmuPushBdCmd = BMU_COMMAND_BASE_ADDRESS |
         BMUWQ_FW_TRANSMIT << 8 |
         PUSH_WQ_CMDTYPE;

    // push the frame to the next queue
    palWriteRegister(pMac->hHdd, bmuPushBdCmd, bdIdx);
    HALLOGE( halLog( pMac, LOGE, FL("bdIdx=%d, addr=0x%x, staIdx=%d, memTh=%d, fcConfig=0x%x\n"), bdIdx, pFcTxBd, staIdx, memTh, fcConfig));
    return p;
}

static char*
dump_addStaWithUapsd(tpAniSirGlobal pMac, tANI_U32 staType, tANI_U32 staidNum, tANI_U32 bssidNum, tANI_U32 uapsdAcMask,char* p)
{
    tpAddStaParams pMsg;
    tSirMacAddr *bssid;
    tSirMacAddr *staid;
    tSirMsgQ halMsg;

    if ((staType != 0) && (staType != 1))
    {
        HALLOGW( halLog(pMac, LOGW, FL("ERROR - sta type should be 0 for STA (self) and 1 for Remote (BSS adding)\n")));
        return p;
    }

    if ((staidNum >= NUM_STAID) || (bssidNum >= NUM_BSSID))
    {
        HALLOGW( halLog(pMac, LOGW, FL("ERROR - bssid should be in [0,%d], staid should be in [0,%d]\n"),
               NUM_BSSID, NUM_STAID));
        return p;
    }

    if (palAllocateMemory(pMac->hHdd, (void **)&pMsg, sizeof(tAddStaParams)) != eHAL_STATUS_SUCCESS)
    {
        HALLOGW( halLog(pMac, LOGW, FL("palAllocateMemory Failed\n")));
        return p;
    }

    bssid = &addr_bssid[bssidNum];
    staid = &addr_staid[staidNum];

    palZeroMemory(pMac->hHdd, (void *) pMsg, sizeof (tAddStaParams));
    palCopyMemory(pMac->hHdd, (void *) &pMsg->bssId, (void *)bssid, 6);
    palCopyMemory(pMac->hHdd, (void *) &pMsg->staMac, (void *)staid, 6);

    pMsg->staIdx = HAL_STA_INVALID_IDX; 
    pMsg->assocId = staidNum + 1; // may not work in all the cases. 
    pMsg->staType = (staType == 0) ? STA_ENTRY_SELF : STA_ENTRY_PEER;
    HALLOGW( halLog(pMac, LOGW, FL("Adding STA %d-%d-%d-%d-%d-%d %s. "),
           (*staid)[0], (*staid)[1], (*staid)[2],
           (*staid)[3], (*staid)[4], (*staid)[5],

           ((staType == 0) ? "as STA (self)" : "on AP")));

    pMsg->listenInterval = 5;
    pMsg->supportedRates.opRateMode =  eSTA_TAURUS;
    pMsg->uAPSD = (uapsdAcMask & 0xff);


    pMsg->htCapable = 1;
    pMsg->wmmEnabled = 1;

    HALLOGW( halLog(pMac, LOGW, FL("Adding STA with 11n %s, Qos %s \n"),
           ((pMsg->htCapable == 1)  ? "ON" : "OFF"),
           ((pMsg->wmmEnabled == 1) ? "ON" : "OFF") ));

    pMsg->respReqd = 0;

    halMsg.type =  SIR_HAL_ADD_STA_REQ;
    halMsg.reserved = dialogToken++;
    halMsg.bodyptr = pMsg;
    halMsg.bodyval = 0;

    if (halPostMsgApi(pMac, &halMsg) != eHAL_STATUS_SUCCESS)
        palFreeMemory(pMac->hHdd, pMsg);

    return p;
}
#endif

static void
halLog_testAddSta(tpAniSirGlobal pMac, tANI_U32 staType, tANI_U32 staidNum, tANI_U32 bssidNum, tANI_U32 qos_11n)
{
    tpAddStaParams pMsg;
    tSirMacAddr *bssid;
    tSirMacAddr *staid;
    tSirMsgQ halMsg;

    if ((staType != 0) && (staType != 1))
    {
        HALLOGW( halLog(pMac, LOGW, FL("ERROR - sta type should be 0 for STA (self) and 1 for Remote (BSS adding)\n")));
        return;
    }

    if ((staidNum >= NUM_STAID) || (bssidNum >= NUM_BSSID))
    {
        HALLOGW( halLog(pMac, LOGW, FL("ERROR - bssid should be in [0,%d], staid should be in [0,%d]\n"),
               NUM_BSSID, NUM_STAID));
        return;
    }

    if (palAllocateMemory(pMac->hHdd, (void **)&pMsg, sizeof(tAddStaParams)) != eHAL_STATUS_SUCCESS)
    {
        HALLOGW( halLog(pMac, LOGW, FL("palAllocateMemory Failed\n")));
        return;
    }

    bssid = &addr_bssid[bssidNum];
    staid = &addr_staid[staidNum];

    palZeroMemory(pMac->hHdd, (void *) pMsg, sizeof (tAddStaParams));
    palCopyMemory(pMac->hHdd, (void *) &pMsg->bssId, (void *)bssid, 6);
    palCopyMemory(pMac->hHdd, (void *) &pMsg->staMac, (void *)staid, 6);

    pMsg->staIdx = HAL_STA_INVALID_IDX; 
    pMsg->assocId = staidNum + 1; // may not work in all the cases. 
    pMsg->staType = (staType == 0) ? STA_ENTRY_SELF : STA_ENTRY_PEER;
    HALLOGW( halLog(pMac, LOGW, FL("Adding STA %d-%d-%d-%d-%d-%d %s. "),
           (*staid)[0], (*staid)[1], (*staid)[2],
           (*staid)[3], (*staid)[4], (*staid)[5],

           ((staType == 0) ? "as STA (self)" : "on AP")));

    pMsg->listenInterval = 5;
    pMsg->supportedRates.opRateMode =  eSTA_TAURUS;


    if (qos_11n == 0)
    {
        pMsg->htCapable = 0;
        pMsg->wmmEnabled = 0;
    }
    else if (qos_11n == 1)
    {
        pMsg->htCapable = 0;
        pMsg->wmmEnabled = 1;
    }
    else if (qos_11n == 2)
    {
        pMsg->htCapable = 1;
        pMsg->wmmEnabled = 0;
    }
    else if (qos_11n == 3)
    {
        pMsg->htCapable = 1;
        pMsg->wmmEnabled = 1;
    }

    HALLOGW( halLog(pMac, LOGW, FL("Adding STA with 11n %s, Qos %s \n"),
           ((pMsg->htCapable == 1)  ? "ON" : "OFF"),
           ((pMsg->wmmEnabled == 1) ? "ON" : "OFF") ));

    pMsg->respReqd = 0;

    halMsg.type =  SIR_HAL_ADD_STA_REQ;
    halMsg.reserved = dialogToken++;
    halMsg.bodyptr = pMsg;
    halMsg.bodyval = 0;

    if (halPostMsgApi(pMac, &halMsg) != eHAL_STATUS_SUCCESS)
        palFreeMemory(pMac->hHdd, pMsg);

    return;
}

static void
halLog_testDeleteBss(tpAniSirGlobal pMac, tANI_U32 bssidNum)
{
    tpDeleteBssParams pMsg;
    tSirMsgQ halMsg;

    if (bssidNum >= NUM_BSSID)
    {
        HALLOGW( halLog( pMac, LOGW, FL("ERROR - bssid should be in [0,%d)\n"),  NUM_BSSID ));
        return;
    }

    if (palAllocateMemory(pMac->hHdd, (void **)&pMsg, sizeof(tDeleteBssParams)) != eHAL_STATUS_SUCCESS)
    {
        HALLOGW( halLog(pMac, LOGW, FL("palAllocateMemory Failed\n")));
        return;
    }

    HALLOGW( halLog( pMac, LOGW, FL("DelBss Index %d\n"),  bssidNum ));

    palZeroMemory(pMac->hHdd, (void *) pMsg, sizeof (tAddBssParams));

    pMsg->bssIdx = (tANI_U16) bssidNum;

    halMsg.type =  SIR_HAL_DELETE_BSS_REQ;
    halMsg.reserved = dialogToken++;
    halMsg.bodyptr = pMsg;
    halMsg.bodyval = 0;

    if (halPostMsgApi(pMac, &halMsg) != eHAL_STATUS_SUCCESS)
        palFreeMemory(pMac->hHdd, pMsg);

    return;
}

static void
halLog_testDeleteSta(tpAniSirGlobal pMac, tANI_U32 staid)
{
    tpDeleteStaParams pMsg;
    tSirMsgQ halMsg;

    if (staid >= NUM_STAID)
    {
        HALLOGW( halLog( pMac, LOGW, FL("ERROR - staid should be in [0,%d]\n"),  NUM_STAID ));
        return;
    }

    if (palAllocateMemory(pMac->hHdd, (void **)&pMsg, sizeof(tDeleteStaParams)) != eHAL_STATUS_SUCCESS)
    {
        HALLOGW( halLog(pMac, LOGW, FL("palAllocateMemory Failed\n")));
        return;
    }

    palZeroMemory(pMac->hHdd, (void *) pMsg, sizeof (tDeleteStaParams));
    pMsg->staIdx= (tANI_U16)staid;
    pMsg->respReqd = 0;
    HALLOGW( halLog( pMac, LOGW, FL("Deleting STA index:%d. "), staid ));

    halMsg.type =  SIR_HAL_DELETE_STA_REQ;
    halMsg.reserved = dialogToken++;
    halMsg.bodyptr = pMsg;
    halMsg.bodyval = 0;

    if (halPostMsgApi(pMac, &halMsg) != eHAL_STATUS_SUCCESS)
        palFreeMemory(pMac->hHdd, pMsg);

    return;
}


static char *
dump_hal_set_global_ra_config( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3,
                                     tANI_U32 arg4, char *p)
{
    (void) arg4;
    halMacRaSetGlobalCfg(pMac, arg1, arg2, arg3);
    return p;
}


char *
dump_hal_sta_rate_20_set(tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    halRate_changeStaRate(pMac, arg1,
            TPE_STA_20MHZ_RATE, halRate_tpeRate2HalRate(arg2),
            halRate_tpeRate2HalRate(arg3), halRate_tpeRate2HalRate(arg4));
    return p;
}


static char *
dump_hal_test_add_sta( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    halLog_testAddSta(pMac, arg1, arg2, arg3, arg4);
    return p;
}

static char *
dump_hal_test_del_sta( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    (void) arg2; (void) arg3; (void) arg4;
    halLog_testDeleteSta(pMac, arg1);
    return p;
}

static char *
dump_hal_test_add_bss( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    (void) arg3; (void) arg4;
    halLog_testAddBss(pMac, arg1, arg2);
    return p;
}

static char *
dump_hal_test_update_beacon_template( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    (void) arg2; (void) arg3; (void) arg4;
    halLog_SendBeaconReq(pMac, arg1);
    return p;
}




static char *
dump_hal_test_del_bss( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    (void) arg2; (void) arg3; (void) arg4;
    halLog_testDeleteBss(pMac, arg1);
    return p;
}

static char *
dump_hal_show_descr( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    tANI_U16 staId, i;
    tANI_U8 *ptr;
    tpStaStruct pSta = (tpStaStruct) pMac->hal.halMac.staTable;

    HALLOGW( halLog(pMac, LOGW, FL("StaTable starts at address: 0X%X\n"), pSta));
    HALLOGW( halLog(pMac, LOGW, FL("############ STATION DESCRIPTORS for all %d stations ##################\n\n"),
                            pMac->hal.memMap.maxStations));
    for (staId = 0; staId < pMac->hal.memMap.maxStations; staId++, pSta++)
    {
        if (pSta->valid)
        {
            HALLOGW( halLog(pMac, LOGW, FL("StaMacAddr: 0X%X %X %X %X %X %X\n"),
                pSta->staAddr[0], pSta->staAddr[1], pSta->staAddr[2], pSta->staAddr[3],
                pSta->staAddr[4], pSta->staAddr[5]));
            HALLOGW( halLog(pMac, LOGW, FL("BssMacAddr: 0X%X %X %X %X %X %X\n"),
                pSta->bssId[0], pSta->bssId[1], pSta->bssId[2], pSta->bssId[3],
                pSta->bssId[4], pSta->bssId[5]));
            HALLOGW( halLog(pMac, LOGW, FL("AssocId: 0X%X\n"), pSta->assocId));
            HALLOGW( halLog(pMac, LOGW, FL("RMF Enabled: 0X%X\n"), pSta->rmfEnabled));
            HALLOGW( halLog(pMac, LOGW, FL("bssIdx: 0X%X, staId: 0X%X\n"), pSta->bssIdx, pSta->staId));
            HALLOGW( halLog(pMac, LOGW, FL("StaType: 0X%X\n"), pSta->staType));
            HALLOGW( halLog(pMac, LOGW, FL("htEnabled: 0X%X, gfEnabled: 0X%X\n"), pSta->htEnabled, pSta->halRaInfo.gfEnabled));
            HALLOGW( halLog(pMac, LOGW, FL("DpuIdx: 0X%X, dpuBcIdx: 0X%X, dpuBcMgmtIdx: 0X%X\n"),
                pSta->dpuIndex, pSta->bcastDpuIndex, pSta->bcastMgmtDpuIndex));
            HALLOGW( halLog(pMac, LOGW, FL("DpuSig: 0X%X, StaSig: 0X%X\n\n"), pSta->dpuSig, pSta->staSig));

            HALLOGW( halLog(pMac, LOGW, FL("TPE STA DESCRIPTOR: STAID: 0X%X-----------------------------------------\n"), staId));
            HALLOGW( halLog(pMac, LOGW, FL("TPE STA DESC: MacAddr1Lo: 0X%X\n"), pSta->tpeStaDesc.macAddr1Lo));
            HALLOGW( halLog(pMac, LOGW, FL("TPE STA DESC: MacAddr1Hi: 0X%X\n"), pSta->tpeStaDesc.macAddr2Lo));
            HALLOGW( halLog(pMac, LOGW, FL("TPE STA DESC: MacAddr2Lo: 0X%X\n"), pSta->tpeStaDesc.macAddr1Hi));
            HALLOGW( halLog(pMac, LOGW, FL("TPE STA DESC: MacAddr2Lo: 0X%X\n"), pSta->tpeStaDesc.macAddr2Hi));
            HALLOGW( halLog(pMac, LOGW, FL("TPE STA DESC: Protection Type: 0X%X\n"), pSta->tpeStaDesc.protection_type));
            HALLOGW( halLog(pMac, LOGW, FL("TPE STA DESC: retry_threshold0: 0X%X\n"), pSta->tpeStaDesc.retry_threshold0));
            HALLOGW( halLog(pMac, LOGW, FL("TPE STA DESC: retry_threshold1: 0X%X\n"), pSta->tpeStaDesc.retry_threshold1));
            HALLOGW( halLog(pMac, LOGW, FL("TPE STA DESC: retry_threshold2: 0X%X\n"), pSta->tpeStaDesc.retry_threshold2));

            HALLOGW( halLog(pMac, LOGW, FL("TPE STA DESC: ack_policy_vector_lo: 0X%X\n"), pSta->tpeStaDesc.ack_policy_vectorLo));
            HALLOGW( halLog(pMac, LOGW, FL("TPE STA DESC: ack_policy_vector_hi: 0X%X\n"), pSta->tpeStaDesc.ack_policy_vectorHi));
            HALLOGW( halLog(pMac, LOGW, FL("TPE STA DESC: ampdu_valid: 0X%X\n\n"), pSta->tpeStaDesc.ampdu_valid));
            HALLOGW( halLog(pMac, LOGW, FL("TPE STA DESC: Rate Info 20MHz params\n")));
            for (i = 0;i < TPE_STA_MAX_RETRY_RATE; i++)
            {
                HALLOGW( halLog(pMac, LOGW, FL("TPE STA DESC: Protection mode: 0X%X\n"), pSta->tpeStaDesc.rate_params_20Mhz[i].protection_mode));
                HALLOGW( halLog(pMac, LOGW, FL("TPE STA DESC: ampdu density: 0X%X\n"), pSta->tpeStaDesc.rate_params_20Mhz[i].ampdu_density));
                HALLOGW( halLog(pMac, LOGW, FL("TPE STA DESC: tx power: 0X%X\n"), pSta->tpeStaDesc.rate_params_20Mhz[i].tx_power));
                HALLOGW( halLog(pMac, LOGW, FL("TPE STA DESC: tx antenna enable: 0X%X\n"), pSta->tpeStaDesc.rate_params_20Mhz[i].tx_antenna_enable));
                HALLOGW( halLog(pMac, LOGW, FL("TPE STA DESC: STBC valid: 0X%X\n"), pSta->tpeStaDesc.rate_params_20Mhz[i].STBC_Valid));
                HALLOGW( halLog(pMac, LOGW, FL("TPE STA DESC: rate index: 0X%X\n\n"), pSta->tpeStaDesc.rate_params_20Mhz[i].rate_index));
            }
            HALLOGW( halLog(pMac, LOGW, FL("TPE STA DESC: Rate Info 40MHz params\n")));
            for (i = 0;i < TPE_STA_MAX_RETRY_RATE; i++)
            {
                HALLOGW( halLog(pMac, LOGW, FL("TPE STA DESC: Protection mode: 0X%X\n"), pSta->tpeStaDesc.rate_params_40Mhz[i].protection_mode));
                HALLOGW( halLog(pMac, LOGW, FL("TPE STA DESC: ampdu density: 0X%X\n"), pSta->tpeStaDesc.rate_params_40Mhz[i].ampdu_density));
                HALLOGW( halLog(pMac, LOGW, FL("TPE STA DESC: tx power: 0X%X\n"), pSta->tpeStaDesc.rate_params_40Mhz[i].tx_power));
                HALLOGW( halLog(pMac, LOGW, FL("TPE STA DESC: tx antenna enable: 0X%X\n"), pSta->tpeStaDesc.rate_params_40Mhz[i].tx_antenna_enable));
                HALLOGW( halLog(pMac, LOGW, FL("TPE STA DESC: STBC valid: 0X%X\n"), pSta->tpeStaDesc.rate_params_40Mhz[i].STBC_Valid));
                HALLOGW( halLog(pMac, LOGW, FL("TPE STA DESC: rate index: 0X%X\n\n"), pSta->tpeStaDesc.rate_params_40Mhz[i].rate_index));
            }
            HALLOGW( halLog(pMac, LOGW, FL("TPE STA DESC: Rate Info BD params\n")));
            for (i = 0;i < TPE_STA_MAX_RETRY_RATE; i++)
            {
                HALLOGW( halLog(pMac, LOGW, FL("TPE STA DESC: Protection mode: 0X%X\n"), pSta->tpeStaDesc.bd_rate_params[i].protection_mode));
                HALLOGW( halLog(pMac, LOGW, FL("TPE STA DESC: ampdu density: 0X%X\n"), pSta->tpeStaDesc.bd_rate_params[i].ampdu_density));
                HALLOGW( halLog(pMac, LOGW, FL("TPE STA DESC: tx power: 0X%X\n"), pSta->tpeStaDesc.bd_rate_params[i].tx_power));
                HALLOGW( halLog(pMac, LOGW, FL("TPE STA DESC: tx antenna enable: 0X%X\n"), pSta->tpeStaDesc.bd_rate_params[i].tx_antenna_enable));
                HALLOGW( halLog(pMac, LOGW, FL("TPE STA DESC: STBC valid: 0X%X\n"), pSta->tpeStaDesc.bd_rate_params[i].STBC_Valid));
                HALLOGW( halLog(pMac, LOGW, FL("TPE STA DESC: rate index: 0X%X\n\n"), pSta->tpeStaDesc.bd_rate_params[i].rate_index));
            }

            HALLOGW( halLog(pMac, LOGW, FL("TPE STA DESC: Data wait cycles: 0X%X\n\n"), pSta->tpeStaDesc.data_wt_cycles));

            HALLOGW( halLog(pMac, LOGW, FL("TPE STA DESC: Probe rsp TSF correction for BD rate 0 or 1: 0X%X\n"),
                pSta->tpeStaDesc.tsfoffset_for_probe_resp_bd_rate_0_1));
            HALLOGW( halLog(pMac, LOGW, FL("TPE STA DESC: Probe rsp TSF correction for BD rate 2 or 3: 0X%X\n"),
                pSta->tpeStaDesc.tsfoffset_for_probe_resp_bd_rate_2_3));
            HALLOGW( halLog(pMac, LOGW, FL("TPE STA DESC: BSSID of sta: 0X%X\n"), pSta->tpeStaDesc.bssid_of_sta));
            HALLOGW( halLog(pMac, LOGW, FL("TPE STA DESC: Max AMPDU bytes: 0X%X\n\n"), pSta->tpeStaDesc.max_bytes_in_ampdu));

            HALLOGW( halLog(pMac, LOGW, FL("TPE STA DESC: AMPDU Window Size per Queue\n")));

            HALLOGW( halLog(pMac, LOGW, FL("TPE STA DESC: Q0: 0X%X, Q1: 0X%X, Q2: 0X%X, Q3: 0X%X\n"),
                pSta->tpeStaDesc.ampdu_window_size_qid0, pSta->tpeStaDesc.ampdu_window_size_qid1,
                pSta->tpeStaDesc.ampdu_window_size_qid2, pSta->tpeStaDesc.ampdu_window_size_qid3));
            HALLOGW( halLog(pMac, LOGW, FL("TPE STA DESC: Q4: 0X%X, Q5: 0X%X, Q6: 0X%X, Q7: 0X%X\n"),
                pSta->tpeStaDesc.ampdu_window_size_qid4, pSta->tpeStaDesc.ampdu_window_size_qid5,
                pSta->tpeStaDesc.ampdu_window_size_qid6, pSta->tpeStaDesc.ampdu_window_size_qid7));
            HALLOGW( halLog(pMac, LOGW, FL("TPE STA DESC: Q8: 0X%X, Q9: 0X%X, Q10: 0X%X, Q11: 0X%X\n"),
                pSta->tpeStaDesc.ampdu_window_size_qid8, pSta->tpeStaDesc.ampdu_window_size_qid9,
                pSta->tpeStaDesc.ampdu_window_size_qid10, pSta->tpeStaDesc.ampdu_window_size_qid11));
            HALLOGW( halLog(pMac, LOGW, FL("TPE STA DESC: Q12: 0X%X, Q13: 0X%X, Q14: 0X%X, Q15: 0X%X\n"),
                pSta->tpeStaDesc.ampdu_window_size_qid12, pSta->tpeStaDesc.ampdu_window_size_qid13,
                pSta->tpeStaDesc.ampdu_window_size_qid14, pSta->tpeStaDesc.ampdu_window_size_qid15));
            HALLOGW( halLog(pMac, LOGW, FL("TPE STA DESC: Q16: 0X%X, Q17: 0X%X, Q18: 0X%X\n"),
                pSta->tpeStaDesc.ampdu_window_size_qid16, pSta->tpeStaDesc.ampdu_window_size_qid17,
                pSta->tpeStaDesc.ampdu_window_size_qid18));

            HALLOGW( halLog(pMac, LOGW, FL("TPE STA DESC: 0X%X\n"), pSta->tpeStaDesc.bd_raw_mode));
            HALLOGW( halLog(pMac, LOGW, FL("TPE STA DESC: 0X%X\n\n"), pSta->tpeStaDesc.mcbcStatsQidMap));
            HALLOGW( halLog(pMac, LOGW, FL("---------------------------------------------------------------------\n\n\n")));

            HALLOGW( halLog(pMac, LOGW, FL("RPE STA DESCRIPTOR: STAID: 0X%X---------------------------------------------\n"), staId));
            HALLOGW( halLog(pMac, LOGW, FL("RPE STA DESC: It's so big, so dumping the whole memory...\n")));
            ptr = (tANI_U8 *) &pSta->rpeStaDesc;
            for (i= 0; i < sizeof(pSta->rpeStaDesc); i+=8, ptr+=8)
            {
                HALLOGW( halLog(pMac, LOGW, FL("%d: %2X %2X %2X %2X : %2X %2X %2X %2X\n"),
                    i, *ptr, *(ptr+1), *(ptr+2), *(ptr+3), *(ptr+4), *(ptr+5), *(ptr+6), *(ptr+7)));
            }
            HALLOGW( halLog(pMac, LOGW, FL("\n\n\n")));
        }
        else
        {
            HALLOGW( halLog(pMac, LOGW, FL("STAID %X NOT VALID\n"), staId));
        }
        HALLOGW( halLog(pMac, LOGW, FL("\n\n\n****************************************************************************\n")));
    }
    HALLOGW( halLog(pMac, LOGW, FL("-----------------------------------------------------------------------\n\n\n")));
    for (i = 0;i < HAL_NUM_UMA_DESC_ENTRIES; i++)
    {
        HALLOGW( halLog(pMac, LOGW, FL("UMA DESCRIPTOR: STAID: 0X%X-------------------------------------------\n"), i));
        HALLOGW( halLog(pMac, LOGW, FL("UMA DESC: DPU Signature: 0X%X\n"), pMac->hal.halMac.aduUmaDesc[i].dpuSig));
        HALLOGW( halLog(pMac, LOGW, FL("UMA DESC: Valid: 0X%X\n"), pMac->hal.halMac.aduUmaDesc[i].valid));
        HALLOGW( halLog(pMac, LOGW, FL("UMA DESC: BssId Upper 2 bytes: 0X%X\n"), pMac->hal.halMac.aduUmaDesc[i].bssidHi));
        HALLOGW( halLog(pMac, LOGW, FL("UMA DESC: BssId Lower 4 bytes: 0X%X\n"), pMac->hal.halMac.aduUmaDesc[i].bssidLo));
        HALLOGW( halLog(pMac, LOGW, FL("UMA DESC: Dpu Descr Index: 0X%X\n"), pMac->hal.halMac.aduUmaDesc[i].dpuDescIdx));
        HALLOGW( halLog(pMac, LOGW, FL("UMA DESC: StaId: 0X%X\n"), pMac->hal.halMac.aduUmaDesc[i].staIdx));
        HALLOGW( halLog(pMac, LOGW,
            FL("UMA DESC: Frame Ctl Type=0X%X SubType=0X%X\n\n"),
            pMac->hal.halMac.aduUmaDesc[i].type, pMac->hal.halMac.aduUmaDesc[i].subType));
        HALLOGW( halLog(pMac, LOGW,
            FL("UMA DESC: Frame Ctl toDS=0X%X fromDS=0X%X\n\n"),
            pMac->hal.halMac.aduUmaDesc[i].toDS, pMac->hal.halMac.aduUmaDesc[i].fromDS));
        HALLOGW( halLog(pMac, LOGW, FL("UMA DESC: Frame Ctl WEP=0X%X \n\n"),
            pMac->hal.halMac.aduUmaDesc[i].wep));
    }
    return p;
}

static char *
dump_hal_show_multi_bss_info( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
	tANI_U16 bssIdx;
    tpBssStruct t = (tpBssStruct) pMac->hal.halMac.bssTable;

	HALLOGW(halLog(pMac, LOGW, FL("Current global link state = %s\n"),
             ((pMac->hal.halGlobalSystemRole == eSYSTEM_AP_ROLE) ? "AP Mode" :
             ((pMac->hal.halGlobalSystemRole == eSYSTEM_STA_IN_IBSS_ROLE) ? "IBSS Mode" : 
             ((pMac->hal.halGlobalSystemRole == eSYSTEM_STA_ROLE) ? "Infra Station Mode" : 
             ((pMac->hal.halGlobalSystemRole == eSYSTEM_BTAMP_STA_ROLE) ? "BTAMP Station Mode" : 
             ((pMac->hal.halGlobalSystemRole == eSYSTEM_BTAMP_AP_ROLE) ? "BTAMP AP Mode" : 
             ((pMac->hal.halGlobalSystemRole == eSYSTEM_MULTI_BSS_ROLE) ? "Multi-BSS in Existance" : 
              "UNKNOWN"
             ))))))));


	for (bssIdx = 0; bssIdx < pMac->hal.halMac.maxBssId; bssIdx++)
	{
        if (t[bssIdx].valid)
		{
	        HALLOGW( halLog(pMac, LOGE, FL("BSS[%x] link state = %s\n"), bssIdx, 
                ((t[bssIdx].bssLinkState == eSIR_LINK_IDLE_STATE) ? "Idle state" :
                ((t[bssIdx].bssLinkState == eSIR_LINK_PREASSOC_STATE) ? "Pre-Assoc State" :
                ((t[bssIdx].bssLinkState == eSIR_LINK_POSTASSOC_STATE) ? "Post-Assoc State" :
                ((t[bssIdx].bssLinkState == eSIR_LINK_AP_STATE) ? "AP State" :
                ((t[bssIdx].bssLinkState == eSIR_LINK_IBSS_STATE) ? "IBSS STATE" :
                ((t[bssIdx].bssLinkState == eSIR_LINK_BTAMP_PREASSOC_STATE) ? "BTAMP_PREASSOC_STATE " :
                ((t[bssIdx].bssLinkState == eSIR_LINK_BTAMP_POSTASSOC_STATE) ? "BTAMP_POSTASSOC_STATE " :
                ((t[bssIdx].bssLinkState == eSIR_LINK_BTAMP_AP_STATE) ? "BTAMP_AP_STATE " :
                ((t[bssIdx].bssLinkState == eSIR_LINK_BTAMP_STA_STATE) ? "BTAMP STA STATE" :
                "UNKNOWN"
                )))))))))));

	        HALLOGW( halLog(pMac, LOGE, FL("BSS[%x] rxp Mode = %s\n"), bssIdx, 
                ((t[bssIdx].bssRxpMode == eRXP_IDLE_MODE) ? "Idle Mode" :
                ((t[bssIdx].bssRxpMode == eRXP_SCAN_MODE) ? "SCAN_MODE Mode" :
                ((t[bssIdx].bssRxpMode == eRXP_PRE_ASSOC_MODE) ? "PRE_ASSOC_MODE Mode" :
                ((t[bssIdx].bssRxpMode == eRXP_POST_ASSOC_MODE) ? "POST_ASSOC_MODE Mode" :
                ((t[bssIdx].bssRxpMode == eRXP_AP_MODE) ? "AP_MODE Mode" :
                ((t[bssIdx].bssRxpMode == eRXP_POWER_SAVE_MODE) ? "POWER_SAVE_MODE Mode" :
                ((t[bssIdx].bssRxpMode == eRXP_IBSS_MODE) ? "IBSS_MODE Mode" :
                ((t[bssIdx].bssRxpMode == eRXP_BTAMP_PREASSOC_MODE) ? "BTAMP_PREASSOC Mode" :
                ((t[bssIdx].bssRxpMode == eRXP_BTAMP_POSTASSOC_MODE) ? "BTAMP_POSTASSOC Mode" :
                ((t[bssIdx].bssRxpMode == eRXP_BTAMP_AP_MODE) ? "BTAMP_AP Mode" :
                ((t[bssIdx].bssRxpMode == eRXP_BTAMP_STA_MODE) ? "BTAMP_STA Mode" :
                "UNKNOWN"
                )))))))))))));

	        HALLOGW( halLog(pMac, LOGE, FL("BSS[%x] role = %s\n"), bssIdx, 
                ((t[bssIdx].bssSystemRole == eSYSTEM_AP_ROLE) ? "AP Mode" :
                ((t[bssIdx].bssSystemRole == eSYSTEM_STA_IN_IBSS_ROLE) ? "IBSS Mode" :
                ((t[bssIdx].bssSystemRole == eSYSTEM_STA_ROLE) ? "Station Mode" :
                ((t[bssIdx].bssSystemRole == eSYSTEM_BTAMP_STA_ROLE) ? "BTAMP Station Mode" :
                ((t[bssIdx].bssSystemRole == eSYSTEM_BTAMP_AP_ROLE) ? "BTAMP AP Mode" :
                "UNKNOWN"
                )))))));
            HALLOGW(halLog(pMac, LOGW, FL("bssSelfStaIdx = %u, bcastStaIdx = %u"), t[bssIdx].bssSelfStaIdx, t[bssIdx].bcastStaIdx));
		}
	}
	return p;
}

static char *dump_hal_show_TLTxRxStat(tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    tANI_U8 curSta = (tANI_U8) arg1;
    tANI_U32 txPcktCount = 0, rxPcktCount = 0;
    tANI_U8 tId = 0;
    HALLOGE(halLog(pMac, LOGE, FL("TL Tx/Rx stat for station = %d/n"), curSta));    
    for(tId =0; tId < 8; tId++)
    {
        halTLGetTxPktCount(pMac, curSta, tId, &txPcktCount);
        halTLGetRxPktCount(pMac, curSta, tId, &rxPcktCount);        
        HALLOGE(halLog(pMac, LOGE, FL("tid = %d, txPcktCount = %d, rxPcktCount = %d/n"), tId, txPcktCount, rxPcktCount));
    }
    return p;
}
static char *dump_hal_send_wowl_enter(tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    tSirMsgQ halMsg;
    tSirHalWowlEnterParams *pWowParams;
    tSirRetStatus status;
    tANI_U32 length;

    HALLOGW(halLog(pMac, LOGW, FL("Send WoWL Enter request to HAL\n")));
    HALLOGW(halLog(pMac, LOGW, FL("  ucPatternFilteringEnable (%d)\n"), arg1));
    HALLOGW(halLog(pMac, LOGW, FL("  ucWowDisassocRcv (%d)\n"), arg2));
    HALLOGW(halLog(pMac, LOGW, FL("  ucWowMaxMissedBeacons (%d)\n"), arg3));
    HALLOGW(halLog(pMac, LOGW, FL("  ucWowMaxSleepUsec (%d)\n"), arg4));

    if (eHAL_STATUS_SUCCESS != palAllocateMemory(pMac->hHdd, (void **)&pWowParams, sizeof(tSirHalWowlEnterParams)))
    {
        HALLOGW(halLog(pMac, LOGW, FL("palAllocateMemory() Failed\n")));
        return p;
    }

    palZeroMemory(pMac->hHdd, (void *)pWowParams, sizeof(tSirHalWowlEnterParams));

    /* magic packet */
    pWowParams->ucMagicPktEnable = TRUE;
    length = SIR_MAC_ADDR_LENGTH;
    status = wlan_cfgGetStr(pMac, WNI_CFG_STA_ID, (tANI_U8 *)pWowParams->magicPtrn, &length);
    if (eSIR_SUCCESS != status)
    {
        pWowParams->magicPtrn[0] = 0x00;
        pWowParams->magicPtrn[1] = 0x0a;
        pWowParams->magicPtrn[2] = 0xf5;
        pWowParams->magicPtrn[3] = 0x04;
        pWowParams->magicPtrn[4] = 0x05;
        pWowParams->magicPtrn[5] = 0x06;
    }

    /* broadcast pattern enable */
    pWowParams->ucPatternFilteringEnable = (tANI_U8) arg1;

    /* Channel Switch Action Frame, ucMagicPacketEnable must be TRUE */
    pWowParams->ucWowChnlSwitchRcv = FALSE;

    /* Deauthentication Frame, ucMagicPacketEnable must be TRUE */
    pWowParams->ucWowDeauthRcv = FALSE;

    /* Disassociation Frame, ucMagicPacketEnable must be TRUE */
    pWowParams->ucWowDisassocRcv = (tANI_U8) arg2;

    /* number cf wake after this many consecutive beacons, ucMagicPacketEnable must be TRUE */
    pWowParams->ucWowMaxMissedBeacons = (tANI_U8) arg3;

    /* force wake after this much time, ucMagicPacketEnable must be TRUE */
    pWowParams->ucWowMaxSleepUsec = (tANI_U8) arg4;

    /* fill in the HAL message */
    halMsg.type = SIR_HAL_WOWL_ENTER_REQ;
    halMsg.reserved = dialogToken++;
    halMsg.bodyptr = pWowParams;
    halMsg.bodyval = 0;

    /* dump the message */
    HALLOGW(halLog(pMac, LOGW, FL("[MSG] HAL type (0x%x)\n"), halMsg.type));
    HALLOGW(halLog(pMac, LOGW, FL("      Magic Pattern enable (%d), pattern (%02x:%02x:%02x:%02x:%02x:%02x)\n"),
           pWowParams->ucPatternFilteringEnable,
           pWowParams->magicPtrn[0],
           pWowParams->magicPtrn[1],
           pWowParams->magicPtrn[2],
           pWowParams->magicPtrn[3],
           pWowParams->magicPtrn[4],
           pWowParams->magicPtrn[5]));
    HALLOGW(halLog(pMac, LOGW, FL("      Broadcast Pattern Filter enable (%d)\n"), pWowParams->ucPatternFilteringEnable));
    HALLOGW(halLog(pMac, LOGW, FL("      Channel Switch Action Frame enable (%d)\n"), pWowParams->ucWowChnlSwitchRcv));
    HALLOGW(halLog(pMac, LOGW, FL("      Deauthentication Frame enable (%d)\n"), pWowParams->ucWowDeauthRcv));
    HALLOGW(halLog(pMac, LOGW, FL("      Disassociation Frame enable (%d)\n"), pWowParams->ucWowDisassocRcv));
    HALLOGW(halLog(pMac, LOGW, FL("      Max missed beacons (%d)\n"), pWowParams->ucWowMaxMissedBeacons));
    HALLOGW(halLog(pMac, LOGW, FL("      Max sleep (%d)\n"), pWowParams->ucWowMaxSleepUsec));

    /* send the message */
    if (eHAL_STATUS_SUCCESS != halPostMsgApi(pMac, &halMsg))
    {
        palFreeMemory(pMac->hHdd, pWowParams);
    }

    return p;
}

static char *dump_hal_send_wowl_exit(tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    tSirMsgQ halMsg;

    HALLOGW(halLog(pMac, LOGW, FL("Send WoWL Exit request to HAL\n")));

    /* fill in the HAL message */
    halMsg.type = SIR_HAL_WOWL_EXIT_REQ;
    halMsg.reserved = dialogToken++;
    halMsg.bodyptr = NULL;
    halMsg.bodyval = 0;

    /* dump the message */
    HALLOGW(halLog(pMac, LOGW, FL("[MSG] HAL type (0x%x)\n"), halMsg.type));

    /* send the message */
    halPostMsgApi(pMac, &halMsg);

    return p;
}

static char *dump_hal_send_wowl_add_ptrn(tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    tSirMsgQ halMsg;
    tSirWowlAddBcastPtrn *pBcastPat;

    HALLOGW(halLog(pMac, LOGW, FL("Send WoWL add pattern to HAL, ARP req target IP (%d.%d.%d.%d)\n"), arg1, arg2, arg3, arg4));

    if (eHAL_STATUS_SUCCESS != palAllocateMemory(pMac->hHdd, (void **)&pBcastPat, sizeof(tSirWowlAddBcastPtrn)))
    {
        HALLOGW(halLog(pMac, LOGW, FL("palAllocateMemory() Failed\n")));
        return p;
    }

    palZeroMemory(pMac->hHdd, (void *)pBcastPat, sizeof(tSirWowlAddBcastPtrn));

    /* fill the message body (ARP req) */
    pBcastPat->ucPatternId = 1;
    pBcastPat->ucPatternByteOffset = 0;
    pBcastPat->ucPatternSize = 28;
    pBcastPat->ucPattern[0] = 0x00;
    pBcastPat->ucPattern[1] = 0x01;  // HW type: ethernet
    pBcastPat->ucPattern[2] = 0x08;
    pBcastPat->ucPattern[3] = 0x00;  // proto: IP
    pBcastPat->ucPattern[4] = 0x06;  // HALEN: 6 bytes
    pBcastPat->ucPattern[5] = 0x04;  // PALEN: 4 bytes
    pBcastPat->ucPattern[6] = 0x00;
    pBcastPat->ucPattern[7] = 0x01;  // operation: ARP req
    pBcastPat->ucPattern[8] = 0x00;  // sender HA[0]
    pBcastPat->ucPattern[9] = 0xde;  // sender HA[1]
    pBcastPat->ucPattern[10] = 0xad;  // sender HA[2]
    pBcastPat->ucPattern[11] = 0xbe;  // sender HA[3]
    pBcastPat->ucPattern[12] = 0xef;  // sender HA[4]
    pBcastPat->ucPattern[13] = 0x00;  // sender HA[5]
    pBcastPat->ucPattern[14] = (tANI_U8) arg1;  // sender IP[0]
    pBcastPat->ucPattern[15] = (tANI_U8) arg2;  // sender IP[1]
    pBcastPat->ucPattern[16] = (tANI_U8) arg3;  // sender IP[2]
    pBcastPat->ucPattern[17] = (tANI_U8) (arg4 + 1);  // sender IP[3]
    pBcastPat->ucPattern[18] = 0xff;  // target HA[0]
    pBcastPat->ucPattern[19] = 0xff;  // target HA[1]
    pBcastPat->ucPattern[20] = 0xff;  // target HA[2]
    pBcastPat->ucPattern[21] = 0xff;  // target HA[3]
    pBcastPat->ucPattern[22] = 0xff;  // target HA[4]
    pBcastPat->ucPattern[23] = 0xff;  // target HA[5]
    pBcastPat->ucPattern[24] = (tANI_U8) arg1;  // target IP[0]
    pBcastPat->ucPattern[25] = (tANI_U8) arg2;  // target IP[1]
    pBcastPat->ucPattern[26] = (tANI_U8) arg3;  // target IP[2]
    pBcastPat->ucPattern[27] = (tANI_U8) arg4;  // target IP[3]
    pBcastPat->ucPatternMaskSize = 4;
    pBcastPat->ucPatternMask[0] = 0xff;
    pBcastPat->ucPatternMask[1] = 0x00;  // don't care about sender addr
    pBcastPat->ucPatternMask[2] = 0x3f;
    pBcastPat->ucPatternMask[3] = 0xf0;

    /* fill in the HAL message */
    halMsg.type = SIR_HAL_WOWL_ADD_BCAST_PTRN;
    halMsg.reserved = dialogToken++;
    halMsg.bodyptr = pBcastPat;
    halMsg.bodyval = 0;

    /* dump the message */
    HALLOGW(halLog(pMac, LOGW, FL("[MSG] HAL type (0x%x)\n"), halMsg.type));
    HALLOGW(halLog(pMac, LOGW, FL("      Add WoWL pattern:\n")));
    HALLOGW(halLog(pMac, LOGW, FL("      Pattern ID (%d)\n"), pBcastPat->ucPatternId));
    HALLOGW(halLog(pMac, LOGW, FL("      Pattern byte offset (%d)\n"), pBcastPat->ucPatternByteOffset));
    HALLOGW(halLog(pMac, LOGW, FL("      Pattern size(%d)"), pBcastPat->ucPatternSize));
    HALLOGW(halLog(pMac, LOGW, FL("      Pattern (%02x%02x%02x%02x %02x%02x%02x%02x)\n"),
           pBcastPat->ucPattern[0], pBcastPat->ucPattern[1], pBcastPat->ucPattern[2], pBcastPat->ucPattern[3],
           pBcastPat->ucPattern[4], pBcastPat->ucPattern[5], pBcastPat->ucPattern[6], pBcastPat->ucPattern[7]));
    HALLOGW(halLog(pMac, LOGW, FL("              (%02x%02x%02x%02x %02x%02x%02x%02x)\n"),
           pBcastPat->ucPattern[8], pBcastPat->ucPattern[9], pBcastPat->ucPattern[10], pBcastPat->ucPattern[11],
           pBcastPat->ucPattern[12], pBcastPat->ucPattern[13], pBcastPat->ucPattern[14], pBcastPat->ucPattern[15]));
    HALLOGW(halLog(pMac, LOGW, FL("              (%02x%02x%02x%02x %02x%02x%02x%02x)\n"),
           pBcastPat->ucPattern[16], pBcastPat->ucPattern[17], pBcastPat->ucPattern[18], pBcastPat->ucPattern[19],
           pBcastPat->ucPattern[20], pBcastPat->ucPattern[21], pBcastPat->ucPattern[22], pBcastPat->ucPattern[23]));
    HALLOGW(halLog(pMac, LOGW, FL("              (%02x%02x%02x%02x                 )\n"),
           pBcastPat->ucPattern[24], pBcastPat->ucPattern[25], pBcastPat->ucPattern[26], pBcastPat->ucPattern[27]));
    HALLOGW(halLog(pMac, LOGW, FL("      Pattern Mask size (%d)\n"), pBcastPat->ucPatternMaskSize));
    HALLOGW(halLog(pMac, LOGW, FL("      Mask (%02x%02x%02x%02x)\n"), pBcastPat->ucPatternMask[0],
                                                              pBcastPat->ucPatternMask[1],
                                                              pBcastPat->ucPatternMask[2],
                                                              pBcastPat->ucPatternMask[3]));

    /* send the message */
    if (eHAL_STATUS_SUCCESS != halPostMsgApi(pMac, &halMsg))
    {
        palFreeMemory(pMac->hHdd, pBcastPat);
    }

    return p;
}

static char *dump_hal_send_wowl_rm_ptrn(tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    tSirMsgQ halMsg;
    tSirWowlDelBcastPtrn *pDelBcastPat;

    HALLOGW(halLog(pMac, LOGW, FL("Send WoWL remove pattern to HAL, pattern id (%d)\n"), arg1));

    if (eHAL_STATUS_SUCCESS != palAllocateMemory(pMac->hHdd, (void **)&pDelBcastPat, sizeof(tSirWowlDelBcastPtrn)))
    {
        HALLOGW(halLog(pMac, LOGW, FL("palAllocateMemory() Failed\n")));
        return p;
    }

    palZeroMemory(pMac->hHdd, (void *)pDelBcastPat, sizeof(tSirWowlDelBcastPtrn));

    /* fill the message body */
    pDelBcastPat->ucPatternId = (tANI_U8) arg1;

    /* fill in the HAL message */
    halMsg.type = SIR_HAL_WOWL_DEL_BCAST_PTRN;
    halMsg.reserved = dialogToken++;
    halMsg.bodyptr = pDelBcastPat;
    halMsg.bodyval = 0;

    /* dump the message */
    HALLOGW(halLog(pMac, LOGW, FL("[MSG] HAL type (0x%x)\n"), halMsg.type));
    HALLOGW(halLog(pMac, LOGW, FL("      Remove WoWL pattern:\n")));
    HALLOGW(halLog(pMac, LOGW, FL("      Pattern ID (%d)\n"), pDelBcastPat->ucPatternId));

    /* send the message */
    if (eHAL_STATUS_SUCCESS != halPostMsgApi(pMac, &halMsg))
    {
        palFreeMemory(pMac->hHdd, pDelBcastPat);
    }

    return p;
}

static char *dump_hal_rx_dcocal(tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    (void) arg1; (void) arg2; (void) arg3; (void) arg4;

#ifdef LIBRA_RF
    halRF_InitRxDcoCal(pMac);
#else
    HALLOGE( halLog(pMac, LOGE, FL("Cannot execute rx dco cal for non RF boards\n")));
#endif

    return p;
}

static char *dump_hal_fw_initcal(tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    (void) arg1; (void) arg2; (void) arg3; (void) arg4;

#ifdef LIBRA_RF
    halPhyCalUpdate(pMac);
#else
    HALLOGE(halLog(pMac, LOGE, FL("Cannot execute Firmware init cal for non RF boards\n")));
#endif

    return p;
}

#ifdef WLAN_DEBUG
#ifndef WLAN_HAL_VOLANS //FIXME_VOLANS
static tANI_U32 aHalPhyRegs[] =
{
    QWLAN_AGC_AGC_WATCHDOG_TIMEOUT_CAP_A_REG,
    QWLAN_AGC_AGC_WATCHDOG_TIMEOUT_CAP_B_REG,
    QWLAN_AGC_AGC_WATCHDOG_TIMEOUT_CORR_REG,
    QWLAN_AGC_AGC_WATCHDOG_TIMEOUT_FIRCAL_DELAY_REG,
    QWLAN_AGC_AGC_WATCHDOG_TIMEOUT_GAIN_SETTLE_REG,
    QWLAN_AGC_AGC_WATCHDOG_TIMEOUT_IDLE_REG,
    QWLAN_AGC_AGC_WATCHDOG_TIMEOUT_RXA_DATA_REG,
    QWLAN_AGC_AGC_WATCHDOG_TIMEOUT_RXA_HDR_REG,
    QWLAN_AGC_AGC_WATCHDOG_TIMEOUT_RXB_DATA_REG,
    QWLAN_AGC_AGC_WATCHDOG_TIMEOUT_RXB_HDR_REG,
    QWLAN_AGC_AGC_WATCHDOG_TIMEOUT_STS_MEAS_REG,
    QWLAN_AGC_AGC_WATCHDOG_TIMEOUT_STS_SETTLE_REG,
    QWLAN_AGC_AGC_WATCHDOG_TIMEOUT_TF_EST_REG,
    QWLAN_AGC_AGC_WATCHDOG_TIMEOUT_TF_EST_SLR_REG,
    QWLAN_AGC_AGC_WATCHDOG_TIMEOUT_WAIT_A_REG,
    QWLAN_AGC_AGC_WATCHDOG_TIMEOUT_WAIT_B_REG,
    QWLAN_AGC_AGC_WATCHDOG_TIMEOUT_WAIT_DCO_A_REG,
    QWLAN_AGC_AGC_WATCHDOG_TIMEOUT_WAIT_DCO_B_REG,
    QWLAN_AGC_AGC_WATCHDOG_TIMEOUT_WAIT_DCO_N_REG,
    QWLAN_AGC_AGC_WATCHDOG_TIMEOUT_WAIT_DCO_SLR_REG,
    QWLAN_AGC_AGC_WATCHDOG_TIMEOUT_WAIT_EOP_REG,
    QWLAN_AGC_AGC_WATCHDOG_TIMEOUT_WAIT_SIFS_REG,
    QWLAN_AGC_AGC_WATCHDOG_TIMEOUT_WAIT_SLR_REG,
    QWLAN_AGC_CHANNEL_FREQ_REG,
    QWLAN_AGC_CONFIG_XBAR_REG,
    QWLAN_AGC_CW_DETECT_REG,
    QWLAN_AGC_DIS_MODE_REG,
    QWLAN_AGC_GAINSET_WRITE_REG,
    QWLAN_AGC_GAINSET0_REG,
    QWLAN_AGC_GAINSET1_REG,
    QWLAN_AGC_D_TXRX_REG,
    QWLAN_AGC_INIT_GAIN_REG,
    QWLAN_AGC_MAX_GAIN_REG,
    QWLAN_AGC_N_ACTIVE_REG,
    QWLAN_AGC_N_CAPTURE_REG,
    QWLAN_AGC_N_LISTEN_REG,
    QWLAN_AGC_N_MEASURE_REG,
    QWLAN_AGC_RX_OVERRIDE_REG,
    QWLAN_AGC_SIFS_TIME_A_REG,
    QWLAN_AGC_SIFS_TIME_B_REG,
    QWLAN_AGC_TH_D0_A_REG,
    QWLAN_AGC_TH_D0_B_REG,
    QWLAN_AGC_TH_D0_B_TF_EST_REG,
    QWLAN_AGC_TH_D0_SLRTFEST_REG,
    QWLAN_AGC_TH_D0_SLRWAITB_REG,
    QWLAN_AGC_TH_D0_SLR_REG,
    QWLAN_AGC_TH_DCO_CAL_REG,
    QWLAN_AGC_TH_MAXCORBSLR_REG,
    QWLAN_AGC_TH_MAXCORSLRB_REG,
    QWLAN_AGC_TH_SIGNAL_HIGH_REG,
    QWLAN_AGC_TH_SIGNAL_LOW_REG,
    QWLAN_AGC_WATCHDOG_MASK_REG,
    QWLAN_CAL_CALMODE_REG,
    QWLAN_CAL_CLR_DCOFFSET_REG,
    QWLAN_CAL_DEBUG_OUTPUT_EN_MASK_REG,
    QWLAN_CAL_LENGTH_REG,
    QWLAN_MCU_MCU_PMU_INFO_REG,
    QWLAN_MPI_COMMAND2_DELAY_REG,
    QWLAN_MPI_MPI_ENABLE_REG,
    QWLAN_PMI_WATCHDOG_TIMEOUT_11A_REG,
    QWLAN_PMU_RF_PA_TRSW_CTRL_REG_REG,
    QWLAN_RACTL_FSCALE_ANTSWITCH_CONTROL1_REG,
    QWLAN_RACTL_PTC_CHAN_FREQ_REG,
    QWLAN_RBAPB_ANTENNA_DIVERSITY_MODE_REG,
    QWLAN_RBAPB_LMS_AVERAGER_SHIFT_REG,
    QWLAN_RFIF_VSWR_OVERLOAD_REG,
    QWLAN_RXACLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG,
    QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_REG,
    QWLAN_RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_AGC_REG,
    QWLAN_RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG,
    QWLAN_TPC_TXPWR_ENABLE_REG,
    QWLAN_TPC_TXPWR_OVERRIDE0_REG,
    QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG,
    QWLAN_TXCTL_DAC_CONTROL_REG,
    QWLAN_TXCTL_FIR_MODE_REG,
    QWLAN_TXFIR_CFG_REG,
    QWLAN_TXFIR_LOLEAKAGE_SHIFT_REG
};

static tANI_U32 aHalRFRegs[] =
{
    QWLAN_RFAPB_BLOCK_ENABLE_REG,
    QWLAN_RFAPB_DA_BALUN_PA_CTL_REG,
    QWLAN_RFAPB_DA_GAIN_CTL_REG,
    QWLAN_RFAPB_HDET_BIAS_REG,
    QWLAN_RFAPB_HDET_CTL_REG,
    QWLAN_RFAPB_HDET_DCOC_REG,
    QWLAN_RFAPB_HDET_TEST_REG,
    QWLAN_RFAPB_HKADC_CFG_REG,
    QWLAN_RFAPB_MODE_SEL1_REG,
    QWLAN_RFAPB_MODE_SEL2_REG,
    QWLAN_RFAPB_MODE_SEL3_REG,
    QWLAN_RFAPB_PA_STG1_2_BIAS_REG,
    QWLAN_RFAPB_PA_STG3_BIAS_REG,
    QWLAN_RFAPB_PDET_CTL_REG,
    QWLAN_RFAPB_PDET_OVRD_REG,
    QWLAN_RFAPB_PLL_AC_REG0_REG,
    QWLAN_RFAPB_PLL_AC_REG3_REG,
    QWLAN_RFAPB_PLL_PC_REG0_REG,
    QWLAN_RFAPB_PLL_PC_REG2_REG,
    QWLAN_RFAPB_PLL_PC_REG4_REG,
    QWLAN_RFAPB_PLL_REG0_REG,
    QWLAN_RFAPB_PLL_REG10_REG,
    QWLAN_RFAPB_PLL_REG1_REG,
    QWLAN_RFAPB_PLL_REG20_REG,
    QWLAN_RFAPB_PLL_REG2_REG,
    QWLAN_RFAPB_PLL_REG4_REG,
    QWLAN_RFAPB_PLL_REG8_REG,
    QWLAN_RFAPB_PLL_REG9_REG,
    QWLAN_RFAPB_PLL_VB_REG0_REG,
    QWLAN_RFAPB_RX_DCOC_EN0_REG,
    QWLAN_RFAPB_RX_DCOC_EN1_REG,
    QWLAN_RFAPB_RX_IM2_VCM1_REG,
    QWLAN_RFAPB_TEST_PATH_CTL_REG,
    QWLAN_RFAPB_TX_DCOC_EN0_REG,
    QWLAN_RFAPB_TX_DCOC_RANGE0_REG,
    QWLAN_RFAPB_TX_DCOC_RANGE1_REG,
    QWLAN_RFAPB_XO_REG0_REG,
/*
    0xe02f884,//QWLAN_RFAPB_BLOCK_ENABLE_REG,
    0xe02fbd4,//QWLAN_RFAPB_DA_BALUN_PA_CTL_REG,
    0xe02fbcc,//QWLAN_RFAPB_DA_GAIN_CTL_REG,
    0xe02fbe4,//QWLAN_RFAPB_HDET_BIAS_REG,
    0xe02fbe0,//QWLAN_RFAPB_HDET_CTL_REG,
    0xe02fbec,//QWLAN_RFAPB_HDET_DCOC_REG,
    0xe02fbe8,//QWLAN_RFAPB_HDET_TEST_REG,
    0xe02fb2c,//QWLAN_RFAPB_HKADC_CFG_REG,
    0xe02f810,//QWLAN_RFAPB_MODE_SEL1_REG,
    0xe02f814,//QWLAN_RFAPB_MODE_SEL2_REG,
    0xe02f818,//QWLAN_RFAPB_MODE_SEL3_REG,
    0xe02fbc4,//QWLAN_RFAPB_PA_STG1_2_BIAS_REG,
    0xe02fbc8,//QWLAN_RFAPB_PA_STG3_BIAS_REG,
    0xe02fb88,//QWLAN_RFAPB_PDET_CTL_REG,
    0xe02fb8c,//QWLAN_RFAPB_PDET_OVRD_REG,
    0xe02f984,//QWLAN_RFAPB_PLL_AC_REG0_REG,
    0xe02f990,//QWLAN_RFAPB_PLL_AC_REG3_REG,
    0xe02f968,//QWLAN_RFAPB_PLL_PC_REG0_REG,
    0xe02f970,//QWLAN_RFAPB_PLL_PC_REG2_REG,
    0xe02f974,//QWLAN_RFAPB_PLL_PC_REG3_REG,
    0xe02f978,//QWLAN_RFAPB_PLL_PC_REG4_REG,
    0xe02f8c0,//QWLAN_RFAPB_PLL_REG0_REG,
    0xe02f8c4,//QWLAN_RFAPB_PLL_REG1_REG,
    0xe02f8e8,//QWLAN_RFAPB_PLL_REG10_REG,
    0xe02f8c8,//QWLAN_RFAPB_PLL_REG2_REG,
    0xe02f8d0,//QWLAN_RFAPB_PLL_REG4_REG,
    0xe02f8e0,//QWLAN_RFAPB_PLL_REG8_REG,
    0xe02f8e4,//QWLAN_RFAPB_PLL_REG9_REG,
    0xe02f944,//QWLAN_RFAPB_PLL_VB_REG0_REG
    0xe02fa90,//QWLAN_RFAPB_RX_DCOC_EN0_REG,
    0xe02fa94,//QWLAN_RFAPB_RX_DCOC_EN1_REG,
    0xe02fb44,//QWLAN_RFAPB_RX_IM2_VCM1_REG,
    0xe02fbd8,//QWLAN_RFAPB_TEST_PATH_CTL_REG,
    0xe02f880,//QWLAN_RFAPB_XO_REG0_REG
    0xe02f804 //QWLAN_RFAPB_TX_GAIN_CONTROL_REG
*/
};
#endif //WLAN_HAL_VOLANS
#endif

static char *dump_hal_phy_regs(tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
#ifdef WLAN_DEBUG
#ifndef WLAN_HAL_VOLANS //FIXME_VOLANS
    tANI_U32 i;
    (void) arg1; (void) arg2; (void) arg3; (void) arg4;

    {
        tANI_U32 regVal;

        HALLOGE(halLog(pMac, LOGE, FL("Dumping phy BB registers\n")));
        for (i=0; i< sizeof(aHalPhyRegs)/sizeof(aHalPhyRegs[0]); i++)
        {
            palReadRegister(pMac->hHdd, aHalPhyRegs[i], &regVal);
            HALLOGE(halLog(pMac, LOGE, FL("0x%x : 0x%x\n"), aHalPhyRegs[i], regVal));

        }

    }


    //txFir coefficients
    {
        tANI_U32 pBuf[4];
        palReadRegister(pMac->hHdd, QWLAN_TXFIR_COEFF20_0_REG, &pBuf[0]);
        palReadRegister(pMac->hHdd, QWLAN_TXFIR_COEFF20_0_REG+4, &pBuf[1]);
        palReadRegister(pMac->hHdd, QWLAN_TXFIR_COEFF20_0_REG+8, &pBuf[2]);
        palReadRegister(pMac->hHdd, QWLAN_TXFIR_COEFF20_0_REG+12, &pBuf[3]);
        HALLOGE(halLog(pMac, LOGE, FL("Dumping txfir 20MHz OFDM filter coefficients\n")));
        {
            HALLOGE(halLog(pMac, LOGE, FL("%x  %x  %x  %x\n"), pBuf[0], pBuf[1], pBuf[2], pBuf[3]));
        }
    }

    //tx IQ cal memory
    {
        tANI_U32 pBuf[8];
        palReadRegister(pMac->hHdd, QWLAN_TXFIR_TXCAL_MEM0_MREG, &pBuf[0]);
        palReadRegister(pMac->hHdd, QWLAN_TXFIR_TXCAL_MEM0_MREG+4, &pBuf[1]);
        palReadRegister(pMac->hHdd, QWLAN_TXFIR_TXCAL_MEM0_MREG+8, &pBuf[2]);
        palReadRegister(pMac->hHdd, QWLAN_TXFIR_TXCAL_MEM0_MREG+12, &pBuf[3]);
        palReadRegister(pMac->hHdd, QWLAN_TXFIR_TXCAL_MEM0_MREG+16, &pBuf[4]);
        palReadRegister(pMac->hHdd, QWLAN_TXFIR_TXCAL_MEM0_MREG+20, &pBuf[5]);
        palReadRegister(pMac->hHdd, QWLAN_TXFIR_TXCAL_MEM0_MREG+24, &pBuf[6]);
        palReadRegister(pMac->hHdd, QWLAN_TXFIR_TXCAL_MEM0_MREG+28, &pBuf[7]);
        HALLOGE(halLog(pMac, LOGE, FL("Dumping tx IQ cal memory\n")));
        {
            HALLOGE(halLog(pMac, LOGE, FL("%x  %x  %x  %x\n"), pBuf[0], pBuf[1], pBuf[2], pBuf[3]));
            HALLOGE(halLog(pMac, LOGE, FL("%x  %x  %x  %x\n"), pBuf[4], pBuf[5], pBuf[6], pBuf[7]));
        }
    }

    //rx IQ cal memory
    {
        tANI_U32 pBuf[8];
        palReadRegister(pMac->hHdd, QWLAN_CAL_IQ_CORR_COEFF_MEM_MREG, &pBuf[0]);
        palReadRegister(pMac->hHdd, QWLAN_CAL_IQ_CORR_COEFF_MEM_MREG+4, &pBuf[1]);
        palReadRegister(pMac->hHdd, QWLAN_CAL_IQ_CORR_COEFF_MEM_MREG+8, &pBuf[2]);
        palReadRegister(pMac->hHdd, QWLAN_CAL_IQ_CORR_COEFF_MEM_MREG+12, &pBuf[3]);
        palReadRegister(pMac->hHdd, QWLAN_CAL_IQ_CORR_COEFF_MEM_MREG+16, &pBuf[4]);
        palReadRegister(pMac->hHdd, QWLAN_CAL_IQ_CORR_COEFF_MEM_MREG+20, &pBuf[5]);
        palReadRegister(pMac->hHdd, QWLAN_CAL_IQ_CORR_COEFF_MEM_MREG+24, &pBuf[6]);
        palReadRegister(pMac->hHdd, QWLAN_CAL_IQ_CORR_COEFF_MEM_MREG+28, &pBuf[7]);
        HALLOGE(halLog(pMac, LOGE, FL("Dumping rx IQ cal memory\n")));
        {
            HALLOGE(halLog(pMac, LOGE, FL("%x  %x  %x  %x\n"), pBuf[0], pBuf[1], pBuf[2], pBuf[3]));
            HALLOGE(halLog(pMac, LOGE, FL("%x  %x  %x  %x\n"), pBuf[4], pBuf[5], pBuf[6], pBuf[7]));
        }
    }

    //AGC gain lut ram
    {

        tANI_U32 pBuf[4];
        palWriteRegister(pMac->hHdd, QWLAN_AGC_AGC_RESET_REG, 1);
        palWriteRegister(pMac->hHdd, QWLAN_AGC_GAIN_LUT_PROGRAM_REG, 1);
        palReadRegister(pMac->hHdd, QWLAN_AGC_GAINLUTRAM_MREG, &pBuf[0]);
        palReadRegister(pMac->hHdd, QWLAN_AGC_GAINLUTRAM_MREG+4, &pBuf[1]);
        palReadRegister(pMac->hHdd, QWLAN_AGC_GAINLUTRAM_MREG+8, &pBuf[2]);
        palReadRegister(pMac->hHdd, QWLAN_AGC_GAINLUTRAM_MREG+12, &pBuf[3]);

        HALLOGE(halLog(pMac, LOGE, FL("Dumping gainlutram for chain 0\n")));
        {
            HALLOGE(halLog(pMac, LOGE, FL("%x  %x  %x  %x\n"), pBuf[0], pBuf[1], pBuf[2], pBuf[3]));
        }
        palWriteRegister(pMac->hHdd, QWLAN_AGC_GAIN_LUT_PROGRAM_REG, 3);
        palReadRegister(pMac->hHdd, QWLAN_AGC_GAINLUTRAM_MREG, &pBuf[0]);
        palReadRegister(pMac->hHdd, QWLAN_AGC_GAINLUTRAM_MREG+4, &pBuf[1]);
        palReadRegister(pMac->hHdd, QWLAN_AGC_GAINLUTRAM_MREG+8, &pBuf[2]);
        palReadRegister(pMac->hHdd, QWLAN_AGC_GAINLUTRAM_MREG+12, &pBuf[3]);
        HALLOGE(halLog(pMac, LOGE, FL("Dumping gainlutram for chain 1\n")));
        {
            HALLOGE(halLog(pMac, LOGE, FL("%x  %x  %x  %x\n"), pBuf[0], pBuf[1], pBuf[2], pBuf[3]));
        }
        palWriteRegister(pMac->hHdd, QWLAN_AGC_GAIN_LUT_PROGRAM_REG, 0);
        palWriteRegister(pMac->hHdd, QWLAN_AGC_AGC_RESET_REG, 0);
    }
    //temporarily, disable the tx and rx gain commands coming from AGC
    palWriteRegister(pMac->hHdd, QWLAN_RFIF_GC_CFG_REG, 0);

    {
        tANI_U32 i, regVal;

        HALLOGE(halLog(pMac, LOGE, FL("Dumping rf config registers\n")));
        for (i=0; i< sizeof(aHalRFRegs)/sizeof(aHalRFRegs[0]); i++)
        {
            palReadRegister(pMac->hHdd, aHalRFRegs[i], &regVal);
            HALLOGE(halLog(pMac, LOGE, FL("0x%x : 0x%x\n"), aHalRFRegs[i], regVal));

        }

    }

    //gain luts
    {
        //rx gain luts
        tANI_U32 i, modeSel, rxGainctrl, txGainctrl, regVal;
        palReadRegister(pMac->hHdd, QWLAN_RFAPB_MODE_SEL1_REG, &modeSel);
        palReadRegister(pMac->hHdd, QWLAN_RFAPB_RX_GAIN_CONTROL_REG, &rxGainctrl);

        palWriteRegister(pMac->hHdd, QWLAN_RFAPB_MODE_SEL1_REG, modeSel|0x200);
        HALLOGE(halLog(pMac, LOGE, FL("Dumping rf rx gain luts\n")));
        for(i=0;i<128;i++){
            palWriteRegister(pMac->hHdd, QWLAN_RFAPB_RX_GAIN_CONTROL_REG, ((i << 7) | i));
            palReadRegister(pMac->hHdd, QWLAN_RFAPB_RX_GC_0_REG, &regVal);
            HALLOGE(halLog(pMac, LOGE, FL("0xe02f808[%d], 0xe02fa80 : 0x%x\n"), i, regVal));
        }

        palWriteRegister(pMac->hHdd, QWLAN_RFAPB_RX_GAIN_CONTROL_REG, rxGainctrl);
        palWriteRegister(pMac->hHdd, QWLAN_RFAPB_MODE_SEL1_REG, modeSel);

        //tx gain luts
        palReadRegister(pMac->hHdd, 0xe02f804, &txGainctrl);
        HALLOGE(halLog(pMac, LOGE, FL("Dumping rf tx gain luts\n")));
        for (i=0;i<16;i++)
        {
            palWriteRegister(pMac->hHdd, 0xe02f804, i);
            palReadRegister(pMac->hHdd, 0xe02fa84, &regVal);
            HALLOGE(halLog(pMac, LOGE, FL("0xe02f804[%d], 0xe02fa84 : 0x%x\n"), i, regVal));
            //palReadRegister(pMac->hHdd, 0xe02fa8c, &regVal);
            //HALLOGE(halLog(pMac, LOGE, FL("0xe02f804[%d], 0xe02fa8c : 0x%x\n"), i, regVal));
        }

        //tx lo
        HALLOGE(halLog(pMac, LOGE, FL("Dumping tx lo cal values\n")));
        palWriteRegister(pMac->hHdd, QWLAN_RFAPB_MODE_SEL1_REG, 0x4000);
        for (i=0;i<16;i++)
        {
            palWriteRegister(pMac->hHdd, 0xe02f804, i);
            palReadRegister(pMac->hHdd, 0xe02fa8c, &regVal);
            HALLOGE(halLog(pMac, LOGE, FL("0xe02f804[%d], 0xe02fa8c : 0x%x\n"), i, regVal));
        }

        palWriteRegister(pMac->hHdd, QWLAN_RFAPB_MODE_SEL1_REG, modeSel);
        palWriteRegister(pMac->hHdd, 0xe02f804, txGainctrl);

    }

    //cal values
    {
        //rx dco
        tANI_U32 i, rxChain, modeSel, rxGainctrl, regVal;
        palReadRegister(pMac->hHdd, QWLAN_RFAPB_MODE_SEL1_REG, &modeSel);
        palReadRegister(pMac->hHdd, QWLAN_RFAPB_RX_GAIN_CONTROL_REG, &rxGainctrl);

        for(rxChain=0;rxChain<2;rxChain++){
            HALLOGE(halLog(pMac, LOGE, FL("Dumping dco corr values for rxChain%d\n"), rxChain));
            palWriteRegister(pMac->hHdd, QWLAN_RFAPB_MODE_SEL1_REG, ((modeSel & ~0x300) | (rxChain << 0x8)));
            for(i=0;i<80;i++){
                palWriteRegister(pMac->hHdd, QWLAN_RFAPB_RX_GAIN_CONTROL_REG, ((i << 7) | i));
                palReadRegister(pMac->hHdd, 0xe02fa88, &regVal);
                HALLOGE(halLog(pMac, LOGE, FL("0xe02f808[%d], 0xe02fa88 : 0x%x\n"), i, regVal));
            }
        }
        palWriteRegister(pMac->hHdd, QWLAN_RFAPB_RX_GAIN_CONTROL_REG, rxGainctrl);
        palWriteRegister(pMac->hHdd, QWLAN_RFAPB_MODE_SEL1_REG, modeSel);
    }
    {
        tANI_U32 revId;
        palReadRegister(pMac->hHdd, QWLAN_RFAPB_REV_ID_REG, &revId);
    }
    palWriteRegister(pMac->hHdd, QWLAN_RFIF_GC_CFG_REG, 3);
#endif //WLAN_HAL_VOLANS
#endif

    return p;
}

static char *dump_hal_set_btc_config(tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
#ifndef WLAN_MDM_CODE_REDUCTION_OPT
    tSmeBtcConfig btcConfig;

    btcConfig.btcWlanIntervalMode1 = (v_U8_t)arg1;
    btcConfig.btcBtIntervalMode1 = (v_U8_t)arg2;
    btcConfig.btcExecutionMode = (v_U8_t)arg3;
    btcConfig.btcActionOnPmFail = (v_U8_t)arg4;

    btcSetConfig((tHalHandle)pMac, &btcConfig);
#endif
    return p;
}

static char *dump_hal_view_btc_config(tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    HALLOGW(tSmeBtcConfig *pBtcConfig = &(pMac->btc.btcConfig));

    HALLOGW(halLog(pMac, LOGW, FL("[BTC] WLAN interval (%d)\n"), pBtcConfig->btcWlanIntervalMode1));
    HALLOGW(halLog(pMac, LOGW, FL("      BT interval (%d)\n"), pBtcConfig->btcBtIntervalMode1));
    HALLOGW(halLog(pMac, LOGW, FL("      execution mode (%d)\n"), pBtcConfig->btcExecutionMode));
    HALLOGW(halLog(pMac, LOGW, FL("      action on PM fail (%d)\n"), pBtcConfig->btcActionOnPmFail));
    HALLOGW(halLog(pMac, LOGW, FL("[BTC] key: BTC_SMART_COEXISTENCE (%d)\n"), BTC_SMART_COEXISTENCE));
    HALLOGW(halLog(pMac, LOGW, FL("           BTC_WLAN_ONLY (%d)\n"), BTC_WLAN_ONLY));
    HALLOGW(halLog(pMac, LOGW, FL("           BTC_PTA_ONLY (%d)\n"), BTC_PTA_ONLY));
    HALLOGW(halLog(pMac, LOGW, FL("           BTC_RESTART_CURRENT (%d)\n"), BTC_RESTART_CURRENT));
    HALLOGW(halLog(pMac, LOGW, FL("           BTC_START_NEXT (%d)\n"), BTC_START_NEXT));

    return p;
}

static char *dump_hal_set_fw_log_filters(tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    tANI_U8 logDescBuffer[COREX_LOG_OVERHEAD];
    CorexLog_LogDescType *pLogDesc = (CorexLog_LogDescType *)&logDescBuffer;
    tANI_U32 index;

    /* modify args for endian conversion (little on host to big on device) */
    if (0 == (arg1 % 2))  /* even index */
    {
        index = arg1 + 1;
    }
    else  /* odd index */
    {
        index = arg1 - 1;
    }

    if (index < COREX_LOG_NUM_FILTERS)
    {
        /* read the current policy */
        palReadDeviceMemory(pMac->hHdd,
                            QWLANFW_MEM_FW_LOG_ADDR_OFFSET,
                            (tANI_U8 *)&logDescBuffer,
                            COREX_LOG_OVERHEAD);

        /* modify the entry we're interested in */
        pLogDesc->sEventFilter[index].nLogLevel = (tANI_U8) arg3;
        pLogDesc->sEventFilter[index].nEventTypeMask = (tANI_U8) arg2;

        /* write the new policy */
        palWriteDeviceMemory(pMac->hHdd,
                             QWLANFW_MEM_FW_LOG_ADDR_OFFSET,
                             (tANI_U8 *)&logDescBuffer,
                             COREX_LOG_OVERHEAD);
    }

    return p;
}

static char *dump_hal_view_fw_log_filters(tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    tANI_U8 logDescBuffer[COREX_LOG_OVERHEAD];
#ifdef WLAN_DEBUG
    CorexLog_LogDescType *pLogDesc = (CorexLog_LogDescType *)&logDescBuffer;
#endif
    tANI_U8 index;

    /* read the current policy */
    palReadDeviceMemory(pMac->hHdd,
                        QWLANFW_MEM_FW_LOG_ADDR_OFFSET,
                        (tANI_U8 *)&logDescBuffer,
                        COREX_LOG_OVERHEAD);

    /* print the current policy */
    for (index = 0; index < COREX_LOG_NUM_FILTERS; index++)
    {
        HALLOGW(halLog(pMac, LOGW, FL("[LOG] module index (%d) log level (%d) event mask (0x%x)\n"),
                       index,
                       pLogDesc->sEventFilter[index].nLogLevel,
                       pLogDesc->sEventFilter[index].nEventTypeMask));
    }

    return p;
}

static char *dump_hal_view_fw_log_records(tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    halUtil_DumpFwCorexLogs((void *)pMac);

    HALLOGW(halLog(pMac, LOGW, FL("[LOG] In Function dump_hal_view_fw_log_records\n")));
    return p;

}

static char *dump_hal_rf_change_channel(tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    (void) arg2; (void) arg3; (void) arg4;

#ifdef LIBRA_RF
    halRF_SetChannel(pMac, (tANI_U8)arg1);
#else
    HALLOGE(halLog(pMac, LOGE, FL("Cannot tune RF channel non RF boards\n")));
#endif

    return p;
}

static char *
dump_hal_control_fw_chip_powerdown( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    (void) arg2; (void) arg3; (void) arg4;
    halPS_CtrlChipPowerDown(pMac, (tANI_U8)arg1);
    return p;
}

static char *
dump_hal_zero_mem( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    (void) arg3, (void) arg4;
    halZeroDeviceMemory(pMac, arg1, arg2);
    return p;
}

static char *
dump_hal_set_max_ps_poll( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    Qwlanfw_SysCfgType *pFwConfig = (Qwlanfw_SysCfgType *)pMac->hal.FwParam.pFwConfig;
    (void) arg2; (void) arg3; (void) arg4;
    pFwConfig->ucMaxPsPoll = (tANI_U8)arg1;
    HALLOGW( halLog(pMac, LOGW, FL("Max Ps Poll being set to %d\n"), pFwConfig->ucMaxPsPoll));
    halFW_UpdateSystemConfig(pMac, pMac->hal.FwParam.fwSysConfigAddr, (tANI_U8 *)pFwConfig, sizeof(Qwlanfw_SysCfgType));
    return p;
}

#ifdef WLAN_SOFTAP_FEATURE
static char *
dump_hal_set_ap_link_monitor( tpAniSirGlobal pMac, tANI_U32 enable, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    Qwlanfw_SysCfgType *pFwConfig = (Qwlanfw_SysCfgType *)pMac->hal.FwParam.pFwConfig;
    (void) arg2; (void) arg3; (void) arg4;
    pFwConfig->fDisLinkMonitor = (!enable);
    HALLOGW( halLog(pMac, LOGW, FL("link monitoring enabled = %u  %d\n"), !pFwConfig->fDisLinkMonitor));
    halFW_UpdateSystemConfig(pMac, pMac->hal.FwParam.fwSysConfigAddr, (tANI_U8 *)pFwConfig, sizeof(Qwlanfw_SysCfgType));
    return p;
}

static char *
dump_hal_set_ap_unknown_addr2_handling( tpAniSirGlobal pMac, tANI_U32 enable, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    Qwlanfw_SysCfgType *pFwConfig = (Qwlanfw_SysCfgType *)pMac->hal.FwParam.pFwConfig;
    (void) arg2; (void) arg3; (void) arg4;
    pFwConfig->fEnableFwUnknownAddr2Handling = (!enable);
    HALLOGW( halLog(pMac, LOGW, FL("unknown addr2 handling enabled = %u  %d\n"), pFwConfig->fEnableFwUnknownAddr2Handling));
    halFW_UpdateSystemConfig(pMac, pMac->hal.FwParam.fwSysConfigAddr, (tANI_U8 *)pFwConfig, sizeof(Qwlanfw_SysCfgType));
    return p;
}

static char *
dump_hal_Fw_Stat( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    halFW_MsgReq(pMac, QWLANFW_COMMON_DUMP_STAT, 0, 0);
    return p;
}
#endif

static char *
dump_hal_set_fw_timeout( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    Qwlanfw_SysCfgType *pFwConfig = (Qwlanfw_SysCfgType *)pMac->hal.FwParam.pFwConfig;
    (void) arg3; (void) arg4;

    switch (arg1) {
       case DUMPCMD_UCAST_DATA_RECEPTION_TIMEOUT:
            pFwConfig->ucUcastDataRecepTimeoutMs = (tANI_U8)arg2;
            break;
       case DUMPCMD_BCAST_DATA_RECEPTION_TIMEOUT:
            pFwConfig->ucBcastDataRecepTimeoutMs = (tANI_U8)arg2;
            break;
       case DUMPCMD_BTQM_QEMPTY_TIMEOUT:
            pFwConfig->ucBtqmQueuesEmptyTimeoutMs = (tANI_U8)arg2;
            break;
       case DUMPCMD_SIF_UNFREEZE_TIMEOUT:
            pFwConfig->ucMaxSifUnfreezeTimeoutMs = (tANI_U8)arg2;
            break;
       case DUMPCMD_FRST_BEACON_TIMEOUT:
            pFwConfig->ucBmpsFirstBeaconTimeoutMs = (tANI_U8)arg2;
            break;
       case DUMPCMD_BPS_EARLY_MODE_TIMEOUT:
            pFwConfig->usBmpsModeEarlyTimeoutUs = (tANI_U16)arg2;
            break;
       case DUMPCMD_BPS_TX_ACTIVITY_TIMEOUT:
            pFwConfig->ucBdPduEmptyMonitorMs = (tANI_U8)arg2;
            break;

       default:
            break;
    }

    halFW_UpdateSystemConfig(pMac, pMac->hal.FwParam.fwSysConfigAddr, (tANI_U8 *)pFwConfig, sizeof(Qwlanfw_SysCfgType));
    return p;
}

static char *
dump_hal_set_fw_sleep_times( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    Qwlanfw_SysCfgType *pFwConfig = (Qwlanfw_SysCfgType *)pMac->hal.FwParam.pFwConfig;
    (void) arg3; (void) arg4;

    switch (arg1) {
       case DUMPCMD_SLEEPTIME_OVERHEADS:
            pFwConfig->usBmpsSleepTimeOverheadsUs= (tANI_U16)arg2;
            break;
       case DUMPCMD_FORCED_SLEEPTIME_OVERHEADS:
            pFwConfig->usBmpsForcedSleepTimeOverheadsUs = (tANI_U16)arg2;
            break;
       case 2:
            pFwConfig->ucListenInterval = (tANI_U16)arg2;
			break;
	   case 3:
            pFwConfig->ucRfSupplySettlingTimeClk = (tANI_U16)arg2;
			break;
       case 4:
            pFwConfig->usPmuSleepTimeoutMsec = (tANI_U16)arg2;
			break;

       default:
            break;
    }

    halFW_UpdateSystemConfig(pMac, pMac->hal.FwParam.fwSysConfigAddr, (tANI_U8 *)pFwConfig, sizeof(Qwlanfw_SysCfgType));
    return p;
}

static char *
dump_hal_set_bcnflt_rssi( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    Qwlanfw_SysCfgType *pFwConfig = (Qwlanfw_SysCfgType *)pMac->hal.FwParam.pFwConfig;
    (void) arg3; (void) arg4;
    pFwConfig->bBeaconFilterEnable = (tANI_U8)arg1;
    pFwConfig->bRssiFilterEnable = (tANI_U8)arg2;
    halFW_UpdateSystemConfig(pMac, pMac->hal.FwParam.fwSysConfigAddr, (tANI_U8 *)pFwConfig, sizeof(Qwlanfw_SysCfgType));
    return p;
}


static char *
dump_hal_sta_rate_info
( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p){
    (void) arg3; (void) arg4;
    halMacRaDumpStaRateInfo(pMac, arg1, arg2);
    return p;
}

static char *
dump_hal_rate_table( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    (void) arg1; (void) arg2; (void) arg3; (void) arg4;
    halMacRaDumpHalRateTable(pMac);
    return p;
}

static char *
dump_hal_sta_suppvalid_rates
( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p){
    (void) arg2;(void) arg3; (void) arg4;
    halMacRaDumpStaAllSupporetedRates(pMac, (tANI_U16)arg1 /*staidx*/);
    return p;
}

static char *
dump_hal_sampling_rate_table( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    (void) arg3;
    (void) arg4;
    halMacRaDumpHalSamplingRateTable(pMac, 0,  arg1, arg2);
    return p;
}

static char *
dump_hal_tpe_rate_entry( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    (void) arg2;
    (void) arg3;
    (void) arg4;

#ifdef WLAN_HAL_VOLANS //This change should go to Libra also.
    if(arg1> MAX_LIBRA_TX_RATE_NUM) {
#else
    if(arg1> MAX_LIBRA_RATE_NUM) {
#endif
        halTpe_PrintMpiCmdTable(pMac);
    } else {
        halTpe_DumpMpiCmdTableEntry(pMac, arg1);
    }

    return p;
}

static char *
dump_hal_ra_control( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    tANI_U32 rateIndex = 0;

    (void) arg2; (void) arg3; (void) arg4;

    if(arg1) {
        rateIndex = 0;
        if(cfgSetInt(pMac, WNI_CFG_FIXED_RATE, rateIndex) != eSIR_SUCCESS) {
            HALLOGW(halLog( pMac, LOGW, " Could not set WNI_CFG_FIXED_RATE=%d", rateIndex));
        }
        halRateAdaptStart(pMac);
    } else {
        if (wlan_cfgGetInt(pMac, WNI_CFG_DEFAULT_RATE_INDEX_24GHZ, &rateIndex) != eSIR_SUCCESS) {
            HALLOGW(halLog( pMac, LOGW, " Could not set WNI_CFG_FIXED_RATE=%d", rateIndex));
        } else {
            if(cfgSetInt(pMac, WNI_CFG_FIXED_RATE, rateIndex) != eSIR_SUCCESS) {
                HALLOGW(halLog( pMac, LOGW, " Could not set WNI_CFG_FIXED_RATE=%d", rateIndex));
            }
            halRateAdaptStop(pMac);
        }
    }
    return p;
}

static char *
dump_hal_ra_stats( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    (void) arg2; (void) arg3; (void) arg4;

    halMacRaDumpStats(pMac, arg1);

    return p;
}

static char *
dump_hal_set_tx_pwr( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    tANI_U32 index;

    (void) arg2; (void) arg3; (void) arg4;
    if((arg1 < 32) && (arg1 >= 0))  {
        /* Update the Tx power for the in the Hal rate table */
        for(index = (tTpeRateIdx)MIN_LIBRA_RATE_NUM; index < (tTpeRateIdx)MAX_LIBRA_TX_RATE_NUM; index++) {
            halRate_UpdateRateTxPower(pMac, index, (tPwrTemplateIndex)arg1);
        }
        /* RA in FW needs to have update for tx power index change */
        halRate_TxPwrIndexToFW(pMac, MIN_LIBRA_RATE_NUM, MAX_LIBRA_TX_RATE_NUM);
        halMacRaUpdateParamReq(pMac, RA_UPDATE_TXPWR_INFO, (MIN_LIBRA_RATE_NUM << 24) | (MAX_LIBRA_TX_RATE_NUM << 16) | ((tPwrTemplateIndex)arg1 << 8));
        // Update the cntrl/rsp rate tx power locally and in the TPE if the updateTpeHw flag is set.
        halRate_UpdateCtrlRspTxPower(pMac, index, (tPwrTemplateIndex)arg1, TRUE);
    } else {
        HALLOGE(halLog(pMac, LOGE, FL("Invalid pwr Index. make sure it is less than 32 \n")));
    }
    return p;
}

static char *
dump_hal_update_rate_cmd_table( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    if (halRate_UpdateTpeRateTable(pMac,
                MIN_LIBRA_RATE_NUM, MAX_LIBRA_TX_RATE_NUM) != eHAL_STATUS_SUCCESS )
    {
        HALLOGE(halLog(pMac, LOGE, FL("halRate_UpdateTpeRateTable() failed \n")));
    }
    return p;
}

static char *
dump_hal_get_sta_rate( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    tpTpeStaDescRateInfo pTpeRateInfo = NULL;
    tANI_U8 i = 0;
    (void) arg2; (void) arg3; (void) arg4;

    halTpe_GetStaDescRateInfo(pMac, arg1, (tTpeRateType)arg2, &pTpeRateInfo);

    for(i=0; i<TPE_STA_MAX_RETRY_RATE; i++) {
        HALLOGW(halLog(pMac, LOGW, "Prot = %d, AmpduDen = %d, TxPwr = %d, TxAntn = %d, STBC = %d, Rate = %d",
            pTpeRateInfo->protection_mode, pTpeRateInfo->ampdu_density, pTpeRateInfo->tx_power,
            pTpeRateInfo->tx_antenna_enable, pTpeRateInfo->STBC_Valid, pTpeRateInfo->rate_index));

        pTpeRateInfo++;
    }

    return p;
}

static char *
dump_hal_read_register( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    tANI_U32 regValue = 0;
    (void) arg2; (void) arg3; (void) arg4;

    halReadRegister(pMac, arg1, &regValue);

    HALLOGE(halLog(pMac, LOGE, FL("Register Value = 0x%08x\n"), regValue));

    return p;
}

static char *
dump_hal_write_register( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    tANI_U32 regValue = 0;
    (void) arg2; (void) arg3; (void) arg4;

    halWriteRegister(pMac, arg1, arg2);
    halReadRegister(pMac, arg1, &regValue);

    HALLOGE(halLog(pMac, LOGE, FL("Register Value Written = 0x%08x\n"), regValue));
    return p;
}

static char *
dump_hal_read_device_memory( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    tANI_U32 addr = (arg1 & ~(0x3));
    tANI_U32 value1, value2, value3, value4, dwords, count;

    (void) arg3; (void) arg4;

    if (arg1) {
        addr = (arg1 & ~(0x3));
    } else {
        HALLOGE(halLog(pMac, LOGE, FL("Invalid address 0x%08x\n"), arg1));
        return p;
    }

    if (!arg2) {
        dwords = 1;
    }

    if (arg3==4) {
        dwords = (arg2+3)/4;
        for (count=0; count<dwords; count++) {
            halReadDeviceMemory(pMac, addr,    &value1, 4);
            halReadDeviceMemory(pMac, addr+4,  &value2, 4);
            halReadDeviceMemory(pMac, addr+8,  &value3, 4);
            halReadDeviceMemory(pMac, addr+12, &value4, 4);
            HALLOGE(halLog(pMac, LOGE, "0x%08x:  %08x %08x %08x %08x\n", addr, value1, value2, value3, value4));
            addr += 16;
        }
    } else if (arg3 == 2) {
        dwords = (arg2+1)/2;
        for (count=0; count<dwords; count++) {
            halReadDeviceMemory(pMac, addr,    &value1, 4);
            halReadDeviceMemory(pMac, addr+4,  &value2, 4);
            HALLOGE(halLog(pMac, LOGE, "0x%08x:  %08x %08x\n", addr, value1, value2));
            addr += 8;
        }
    } else {
        dwords = arg2;
        for (count=0; count<dwords; count++) {
            halReadDeviceMemory(pMac, addr,    &value1, 4);
            HALLOGE(halLog(pMac, LOGE, "0x%08x:  %08x\n", addr, value1));
            addr += 4;
        }
    }

    return p;
}


static char *
dump_hal_stop_fw_heartbeat( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    (void) arg2; (void) arg3; (void) arg4;

    if(arg1) {
        halFW_StartChipMonitor(pMac);
    } else {
        halFW_StopChipMonitor(pMac);
    }
    return p;
}
static char *
dump_hal_set_rf_xo( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    Qwlanfw_SysCfgType *pFwConfig = (Qwlanfw_SysCfgType *)pMac->hal.FwParam.pFwConfig;
    (void) arg2; (void) arg3; (void) arg4;

    if (arg1) {
        pFwConfig->bRfXoOn = TRUE;
        pFwConfig->usBmpsSleepTimeOverheadsUs = HAL_PWR_SAVE_FW_BMPS_SLEEP_TIME_OVERHEADS_RFXO_US;
        pFwConfig->usBmpsForcedSleepTimeOverheadsUs = HAL_PWR_SAVE_FW_FORCED_SLEEP_TIME_OVERHEADS_RFXO_US;
    } else {
        pFwConfig->bRfXoOn = FALSE;
        pFwConfig->usBmpsSleepTimeOverheadsUs = HAL_PWR_SAVE_FW_BMPS_SLEEP_TIME_OVERHEADS_US;
        pFwConfig->usBmpsForcedSleepTimeOverheadsUs = HAL_PWR_SAVE_FW_FORCED_SLEEP_TIME_OVERHEADS_US;
    }

    halFW_UpdateSystemConfig(pMac, pMac->hal.FwParam.fwSysConfigAddr, (tANI_U8 *)pFwConfig, sizeof(Qwlanfw_SysCfgType));

    return p;
}

static char *
dump_hal_pmu_rssi_regs( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    tANI_U32 value[WNI_CFG_NUM_BEACON_PER_RSSI_AVERAGE_STAMAX], i, startPtr, count;
	tANI_S32 ant0_rssi, ant1_rssi, ant0_valid, ant1_valid;
    (void) arg2; (void) arg3; (void) arg4;

	for(count = 0; count < arg1; count++)
	{
		halReadRegister(pMac, QWLAN_PMU_RSSI_ANT_PTR_REG, &startPtr);
		HALLOGE( halLog(pMac, LOGE, FL("rssi_ant_ptr : %d "), startPtr));
		for(i = 0; i < WNI_CFG_NUM_BEACON_PER_RSSI_AVERAGE_STAMAX; i++)
		{
			halReadRegister(pMac, QWLAN_PMU_PMU_RSSI_ANT_STORE_REG0_REG + (4*i), &value[i]);
		}

		for(i = 0; i < WNI_CFG_NUM_BEACON_PER_RSSI_AVERAGE_STAMAX; i++)
		{
			ant0_rssi = (value[i] & 0xFF);
			ant1_rssi = ((value[i] >> 9) & 0xFF);
			ant0_valid = ((value[i] >> 8) & 0x1);
			ant1_valid = ((value[i] >> 17) & 0x1);
			HALLOGE( halLog(pMac, LOGE, FL("rssi_reg%d, ant0_valid : %d, ant0_rssi : %d, ant1_valid : %d, ant1_rssi : %d "), i, ant0_valid, (tANI_S8)(ant0_rssi-100), ant1_valid, (tANI_S8)(ant1_rssi-100)));
		}
	}
    return p;
}

static char *
dump_hal_fw_periodic_chan_tune( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    Qwlanfw_SysCfgType *pFwConfig = (Qwlanfw_SysCfgType *)pMac->hal.FwParam.pFwConfig;
    (void) arg3; (void) arg4;
    pFwConfig->bPeriodicChanTune = (tANI_U8)arg1;
    halFW_UpdateSystemConfig(pMac, pMac->hal.FwParam.fwSysConfigAddr, (tANI_U8 *)pFwConfig, sizeof(Qwlanfw_SysCfgType));
    return p;

}

extern const eHalPhyRates macPhyRateIndex[TPE_RT_IDX_MAX_RATES];
static char *
dump_hal_phy_update_rate2pwr_table( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    eHalPhyRates       phyRate;
    tPwrTemplateIndex  power=0;
    (void) arg3; (void) arg4;

    if(arg1 >= MAX_LIBRA_TX_RATE_NUM)
    {
        HALLOGE(halLog(pMac, LOGE, FL("Invalid rate idx \n")));
        return p;
    }

    phyRate = macPhyRateIndex[arg1];
    pMac->hphy.phy.pwrOptimal[RF_SUBBAND_2_4_GHZ][phyRate].reported = (t2Decimal)arg2;

    halRate_GetTxPwrForRate(pMac, (tTpeRateIdx)arg1, 30, &power);

    // Update the TX power for the rate in the local cache,
    // which will be used as a reference to update the tx power in the
    // TPE station descriptor when the rate changes.
    halRate_UpdateRateTxPower(pMac, (tTpeRateIdx)arg1, power);
    /* RA in FW needs to have update for tx power index change */
    halRate_TxPwrIndexToFW(pMac, arg1, arg1+1);
    halMacRaUpdateParamReq(pMac, RA_UPDATE_TXPWR_INFO, ((tANI_U8)arg1 << 24) | ((tANI_U8)(arg1+1) << 16) | ((tPwrTemplateIndex)power << 8));

    // Update the cntrl/rsp rate tx power locally and in the TPE
    halRate_UpdateCtrlRspTxPower(pMac, (tTpeRateIdx)arg1, power, TRUE);

    return p;
}

static char *
dump_hal_phy_get_power_for_rate( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    eHalPhyRates       phyRate;
    tPwrTemplateIndex  power=0;
    (void) arg3; (void) arg4;

    if(arg1 >= MAX_LIBRA_TX_RATE_NUM)
    {
        HALLOGE(halLog(pMac, LOGE, FL("Invalid rate idx \n")));
        return p;
    }

    phyRate = macPhyRateIndex[arg1];

    //make sure to update the rate cmd table
    if (halPhyGetPowerForRate( pMac,
                               phyRate,
                               POWER_MODE_HIGH_POWER,
                               (tANI_S8)arg2,
                &power) != eHAL_STATUS_SUCCESS) {
        HALLOGE( halLog(pMac, LOGE, FL("halPhyGetPowerForRate(rateIndex %d) failed \n"), arg1));
        return p;

    } else {
        /* For some reason if AP is advertizing wrong local power constraint value
         * which is larger than the power we are operating or larger than regulatory
         * transmit power, then, just use power value returned by PHY
         */
        HALLOGE( halLog(pMac, LOGE, FL("HAL: **** The actual power being set for rateIdx[%d] is - %d ****"), arg1, power));

    }

    return p;

}

static char *
dump_hal_phy_open_close_tpc_loop( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    (void) arg2; (void) arg3; (void) arg4;

    if (arg1)
    {
        //open the TPC loop
        halWriteRegister(pMac, QWLAN_TPC_TXPWR_ENABLE_REG, QWLAN_TPC_TXPWR_ENABLE_OVERRIDE_MASK);
    }
    else
    {
        //close the TPC loop
        halWriteRegister(pMac, QWLAN_TPC_TXPWR_ENABLE_REG, QWLAN_TPC_TXPWR_ENABLE_EN_MASK);
    }
    return p;

}

static char *
dump_hal_phy_set_open_loop_gain( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    (void) arg3; (void) arg4;

    if (arg1 > 15 || arg2 > 15)
    {
        HALLOGE(halLog(pMac, LOGE, FL("RF/Digital gain cannot be more than 15\n")));
        return p;
    }

    {
        tTxGain tx;

        tx.coarsePwr    = (eTxCoarseGain)arg1;
        tx.finePwr      = (eTxFineGain)arg2;

        if (asicTPCPowerOverride(pMac, tx, tx, tx, tx) != eHAL_STATUS_SUCCESS)
        {
            HALLOGE(halLog(pMac, LOGE, FL("Setting the open loop gain failed!\n")));
        }
        else
        {
            HALLOGE(halLog(pMac, LOGE, FL("Setting the open loop gain successful!\n")));
        }
    }
    return p;
}

static char *
dump_hal_rate_tx_power( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    (void) arg1; (void) arg2; (void) arg3; (void) arg4;
    halRate_DumpRateTxPower(pMac);
    return p;
}

static char *
dump_hal_phy_update_tpcCfg_cal_point( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
/*
    if((arg1 < 2) && (arg2 < 4))
    {
        pMac->hphy.nvCache.tables.tpcConfig[arg1].empirical[0][arg2].pwrDetAdc = (tPowerDetect)arg3;
        pMac->hphy.nvCache.tables.tpcConfig[arg1].empirical[0][arg2].adjustedPwrDet = (tPowerDetect)((arg4 * 8)/100 - 16);
    }
    else
    {
        HALLOGE(halLog(pMac, LOGE, FL("Invalid arguments. Make sure the freqIdx < 2 and pointIdx < 4 \n")));
    }
*/
    HALLOGE(halLog(pMac, LOGE, FL("Obsolete dump command \n")));
    return p;

}


static char *
dump_hal_phy_update_start_end_cal_freq( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
/*
    const tANI_U16 rfChans[NUM_RF_CHANNELS] =
    {
        //RF_SUBBAND_2_4_GHZ
        //freq, chan#, band
        2412,        //RF_CHAN_1,
        2417,        //RF_CHAN_2,
        2422,        //RF_CHAN_3,
        2427,        //RF_CHAN_4,
        2432,        //RF_CHAN_5,
        2437,        //RF_CHAN_6,
        2442,        //RF_CHAN_7,
        2447,        //RF_CHAN_8,
        2452,        //RF_CHAN_9,
        2457,        //RF_CHAN_10,
        2462,        //RF_CHAN_11,
        2467,        //RF_CHAN_12,
        2472,        //RF_CHAN_13,
        2484,        //RF_CHAN_14,
    };
    (void) arg3; (void) arg4;

    if((arg1 < 2) && (arg2 > 0) && (arg2 <= NUM_RF_CHANNELS))
    {
        pMac->hphy.nvCache.tables.tpcConfig[arg1].freq = rfChans[arg2-1];
    }
    else
    {
        HALLOGE(halLog(pMac, LOGE, FL("Invalid arguments. Make sure the freqIdx < 2 and chan <= 14 \n")));
    }
*/
    return p;

}

static char *
dump_hal_phy_config_tpc( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
/*
    Qwlanfw_SysCfgType *pFwConfig = (Qwlanfw_SysCfgType *)pMac->hal.FwParam.pFwConfig;
    (void) arg1; (void) arg2; (void) arg3; (void) arg4;

    //change as if qFuse is blown
    pMac->hphy.nvCache.tables.qFuseData.dword3_sw_fuse_pgm_dsbl = 1;

    //update the sysconfig, so that fw is sync with the host
    pFwConfig->bClosedLoop = CLOSED_LOOP_CONTROL;
    halFW_UpdateSystemConfig(pMac, pMac->hal.FwParam.fwSysConfigAddr, (tANI_U8 *)pFwConfig, sizeof(Qwlanfw_SysCfgType));

    //make sure we close the loop
    halWriteRegister(pMac, QWLAN_TPC_TXPWR_ENABLE_REG, TPC_TXPWR_ENABLE_MASK);

    //configure TPC from NV cache
    //halPhyConfigureTpc(pMac);

    //update whole TPE rate power table.
#ifdef FEATURE_TX_PWR_CONTROL
    halRate_UpdateRateTablePower(pMac, (tTpeRateIdx)HALRATE_MODE_START, (tTpeRateIdx)HAL_MAC_MAX_TX_RATES, TRUE);
#else
    halRate_UpdateRateTablePower(pMac, (tTpeRateIdx)MIN_LIBRA_RATE_NUM, (tTpeRateIdx)MAX_LIBRA_TX_RATE_NUM, TRUE);
#endif
*/

    return p;

}

static char *
dump_hal_phy_set_chain_select( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    ePhyChainSelect chainSelect;
    (void) arg3; (void) arg4;

    chainSelect = halPhyGetChainSelect(pMac, (tANI_U8)arg1, (tANI_U8)arg2);

    if (chainSelect != INVALID_PHY_CHAIN_SEL)
    {
        if (halPhySetChainSelect(pMac, chainSelect) != eHAL_STATUS_SUCCESS){
            HALLOGE( halLog(pMac, LOGE, FL("halPhySetChainSelect() failed \n")));
        }
    }
    else
    {
        HALLOGE( halLog(pMac, LOGE, FL("Incorrect Tx/Rx chain selection")));
    }
    return p;

}

static char *
dump_hal_insert_adu_reinit_reg_entry( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{

    halRegBckup_InsertRegEntry(pMac, arg1/*index*/, arg2/*regAddr*/, arg3/*regValue*/, arg4/*hostFilled*/);

    return p;
}

static char *
dump_hal_get_cfg( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    halLog_getCfg(pMac, (tANI_U16)arg1);
    return p;
}

static char *
dump_hal_set_cfg( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    halLog_setCfg(pMac, (tANI_U16)arg1, arg2);
    return p;
}

/* The dump command gets the power save related counters from firmware */
static char *
dump_hal_get_pwr_save_counters( tpAniSirGlobal pMac, 
                                tANI_U32 arg1, 
                                tANI_U32 arg2, 
                                tANI_U32 arg3, 
                                tANI_U32 arg4, 
                                char *p)
{
    /* The dump command reads the power-save counters and dumps on the console.
     * Currently, the routine does not care for the power save status of the chip. 
     * It will wake up the chip to read power save counters.
     */
    eHalStatus status;
    unsigned int i=0;
    unsigned int startAddr = QWLANFW_MEM_FW_PS_COUNTERS_ADDR_OFFSET;
    unsigned int size = QWLANFW_MEM_FW_PS_COUNTERS_SIZE;
    unsigned char buffer[QWLANFW_MEM_FW_PS_COUNTERS_SIZE] = { 0 };
    unsigned int *puBuf;

    status = halReadDeviceMemory(pMac, startAddr, (void*)buffer, size);
    
    if (status != eHAL_STATUS_SUCCESS)
    {
        HALLOGE( halLog(pMac, LOGE, FL("***** PWRSAVE counters could not be read *****")));
        return p;
    }

    puBuf = (unsigned int *)buffer;

    for (i=0; i < (size/sizeof(unsigned int)); i++)
    {
        HALLOGE( halLog(pMac, LOGE, FL("buffer[%d] = %d"), i, *puBuf++));
    }
    
    return p;
}

static tDumpFuncEntry halMenuDumpTable[] = {
    {0,     "HAL Specific (50-299)",                                    NULL},
    //----------------------------
    {0,     "HAL Basic Procedures (60-80)",                             NULL},
    {7,     "HAL:Read Register <address>",                              dump_hal_read_register},
    {8,     "HAL:Write Register <address> <value>",                     dump_hal_write_register},
    {9,     "HAL:Read Memory <address> <# of dwords> <1/2/4 block>",    dump_hal_read_device_memory},
    {10,    "HAL.Basic: zero memory arg1 = address, arg2 = length",     dump_hal_zero_mem},
    {11,    "HAL:Set CFG <cfg> <value>",                                dump_hal_set_cfg},
    {12,    "HAL:Get CFG <cfg>",                                        dump_hal_get_cfg},
    {50,    "HAL.Basic: pMac size details",                             dump_pMac_Size},
#ifdef WLAN_SOFTAP_FEATURE    
    {55,    "send flow contorl frame to fw <staIdx> <memUsage Threshold> <fcConfig>", dump_sendFcFrameToFw},    
    {56,    "add sta with Uapsd <staType> <staNum> <bssIdx> <uapsdAcMask>", dump_addStaWithUapsd},        
    {57,    "update probe response template <bssIdx>",                  dump_updateProbeRspTemplate},    
    {58,    "set probeRsp IE bitmap for FW: <flag: enable/disable the feature>, , <flag: enableDisableAllIes>",
                                                                        dump_setProbeRspIeBitmap},    
    {59,    "update UAPSD setting for a peer station(staIdx, uapsdACMask, maxSpLen)", halLog_updateUapsd},    
#endif    
    {60,    "Test AddSta (staType, staId, bssId, qos_11n)",             dump_hal_test_add_sta},
    {61,    "Test DelSta (staId)",                                      dump_hal_test_del_sta},
    {62,    "Test AddBss (bssType, bssId)",                             dump_hal_test_add_bss},
    {63,    "Test DelBss (bssId)",                                      dump_hal_test_del_bss},
    {64,    "Show all descriptors",                                     dump_hal_show_descr},
    {65,    "Show TL Tx/Rx counters",                                   dump_hal_show_TLTxRxStat},    
    {66,    "update beacon template (bssIdx)",                          dump_hal_test_update_beacon_template},     
#if (defined(ANI_OS_TYPE_ANDROID) || defined(ANI_OS_TYPE_LINUX))    
    {67,    "transmit frame template (templateFile number < 10)",       SendTemplateFrame},         
    {68,    "transmit frame template using BTQM (staIdx, templateFile number < 10)",            SendFrmBdIdx},  
#endif    
    {69,    "set link state of a bss (bssIdx, link sate <= 13)",        dump_setLinkState},  


    {0,    "Multi-BSS Info (70-80)",        NULL},
    {70,   "Print current active Bss information", 
                                            dump_hal_show_multi_bss_info}, 

    {0,     "PowerSave (90-100)",                                       NULL},
    {90,    "Chip Power down Control <1/0>",                            dump_hal_control_fw_chip_powerdown},
    {91,    "Set Max Ps Poll <0-255>",                                  dump_hal_set_max_ps_poll},
    {93,    "Set timeouts of Fw",                                       dump_hal_set_fw_timeout},
    {94,    "Set Sleep Times of Fw",                                    dump_hal_set_fw_sleep_times},
    {95,    "Enable BcnFilter <0/1> RssiMonitor <0/1>",                 dump_hal_set_bcnflt_rssi},
    {96,    "HAL.FW: Stop the FW heartbeat",                            dump_hal_stop_fw_heartbeat},
    {97,    "HAL.FW: toggle RfXo at FW",                                dump_hal_set_rf_xo},
    {98,    "HAL.FW: Insert ADU-reinit regEntry <index><Addr><value><hostfilled>",  dump_hal_insert_adu_reinit_reg_entry},
    {99,    "dump RSSI register values>",                               dump_hal_pmu_rssi_regs},
    {100,   "Dump Power-save Counters>",                                dump_hal_get_pwr_save_counters},
#ifdef WLAN_SOFTAP_FEATURE    
    {101,   "ap link monitor at FW <1/0>",                             dump_hal_set_ap_link_monitor},   
    {102,   "ap unknown addr2 handling at FW <1/0>",                   dump_hal_set_ap_unknown_addr2_handling}, 
    {103,   "dump FW stat",                                            dump_hal_Fw_Stat},
#endif    
    
    {127,   "HAL: enable/disable BA activity check timer. arg1 = enable(1)/disable(0)",  (tpFunc)dump_set_ba_activity_check_timeout},

    {0,     "RateAdaptation (180-220)",                                                        NULL},
    {180,   "HAL.RATE: Set STA 20M Rate <staid> <PriRateIdx> <SecRateIdx> <TerRateIdx>", dump_hal_sta_rate_20_set},
    {181,   "HAL.RATE: Get STA 20M Rate <staid>", (tpFunc)dump_hal_get_sta_rate},
    {182,   "HAL.RATE: Enable/Disable RA Global rates <0: disable, 1:enable>  <tpeRateStart> <tpeRateEnd>", dump_hal_set_global_enable_rates},
    {195,   "HAL.RATE: Set Global RA cfg <0: show help, [id] >  <value>", (tpFunc)dump_hal_set_global_ra_config},
    {200,   "HAL.RATE: Dump STA rate info <startStaIdx> <endStaIdx>",  dump_hal_sta_rate_info},
    {202,   "HAL.RATE: Dump HAL rate table & Tx pkt cnt",  dump_hal_rate_table},
    {203,   "HAL.RATE: Dump STA supported/valid/retry rates <staid>", dump_hal_sta_suppvalid_rates },
    {204,   "HAL.RATE: Dump HAL sampling rate table <maxLowRate> <maxHighRate>", dump_hal_sampling_rate_table},
    {205,   "HAL RATE: Dump the TPE rate entry <rateIndex>", dump_hal_tpe_rate_entry},
    {206,   "HAL.RATE: Enable/Disable periodic RA <1=Enable, 0=Disable>", (tpFunc)dump_hal_ra_control},
    {207,   "HAL.RATE: Get the stats <staid>", (tpFunc)dump_hal_ra_stats},
    {208,   "HAL.RATE: force transmit power idx <pwrIdx>", (tpFunc)dump_hal_set_tx_pwr},
    {209,   "HAL.RATE: update rate command table", (tpFunc)dump_hal_update_rate_cmd_table},
    {210,   "HAL.RATE: dump rate to power",                             dump_hal_rate_tx_power},

    {0,     "RXP (220-229)",                                            NULL},
    {222,   "HAL.RXP: Print hardware's RXP Binary Search Table",        dump_hal_rxp_hw_searchtable},

    {0,     "WoWLAN (230-233)",                                         NULL},
    {230,   "send PE->HAL WoWL Enter Req (bcastEnable, disassoc, maxBeacons, maxSleep)", dump_hal_send_wowl_enter},
    {231,   "send PE->HAL WoWL Exit Req",                               dump_hal_send_wowl_exit},
    {232,   "send PE->HAL WoWL Add Ptrn (IPaddr[0] IPaddr[1] IPaddr[2] IPaddr[3])", dump_hal_send_wowl_add_ptrn},
    {233,   "send PE->HAL WoWL Remove Ptrn (ptrn id)",                  dump_hal_send_wowl_rm_ptrn},

    {0,     "HAL Phy (234-249)",                                        NULL},
    {234,   "Tune RF Channel 1",                                        dump_hal_rf_change_channel},
    {235,   "Init Rx DCO cal",                                          dump_hal_rx_dcocal},
    {236,   "Firmware init cal",                                        dump_hal_fw_initcal},
    {237,   "Firmware Periodic Channel tuning <1/0>",                   dump_hal_fw_periodic_chan_tune},
    {238,   "update rate2Pwr table <rateIdx> <dBm>",                    dump_hal_phy_update_rate2pwr_table},
    {239,   "tpc config cal point <freqIdx><pointIdx><PADC><outputPwr upto two decimals>",  dump_hal_phy_update_tpcCfg_cal_point},
    {240,   "modify start/end cal frequencies <freqIdx><chanIdx>",      dump_hal_phy_update_start_end_cal_freq},
    {241,   "Configure Tpc",                                            dump_hal_phy_config_tpc},
    {242,   "dump halPhy regs",                                         dump_hal_phy_regs},
    {243,   "halphySetChainSelect<numTx><numRx>",                       dump_hal_phy_set_chain_select},
    {244,   "getPwr <rateidx><pwrCap>",                                 dump_hal_phy_get_power_for_rate},
    {245,   "open TPC loop <1/0>",                                      dump_hal_phy_open_close_tpc_loop},
    {246,   "Set open loop gain<rfGgain><digGain>",                     dump_hal_phy_set_open_loop_gain},

    {0,     "BTC (250-259)",                                            NULL},
    {250,   "change BTC paramters (WLAN interval, BT interval, mode, action)", dump_hal_set_btc_config},
    {251,   "view BTC paramters",                                       dump_hal_view_btc_config},
    {252,   "set FW log collection filters (module index, log level, event mask)", dump_hal_set_fw_log_filters},
    {253,   "view FW log collection filters",                           dump_hal_view_fw_log_filters},
    {255,   "dump FW Logs",                                             dump_hal_view_fw_log_records},
};

void halDumpInit(tpAniSirGlobal pMac)
{

    logDumpRegisterTable( pMac, &halMenuDumpTable[0],
                          sizeof(halMenuDumpTable)/sizeof(halMenuDumpTable[0]) );
}



/**
 *  @fn         halIbssRelatedRegisterDump
 *  @brief  Provides dump of IBSS related Libra registers for debugging.
 *
 *  @param  pMac - Global MAC handler
 *
 *  @return void.
 */
void
halIbssRelatedRegisterDump(tpAniSirGlobal pMac)
{
   tANI_U32 r1, r2, r3, r4, r5, r6;
   tANI_U32 r7, r8, r9, r10, r11, r12;

   // Dump TPE registers
   halReadRegister(pMac, QWLAN_TPE_MTU_TPE_INTERFACE_CNTS_REG, &r1);
   halReadRegister(pMac, QWLAN_RXP_PHY_ABORT_CNT_REG, &r2);
   halReadRegister(pMac, QWLAN_RXP_DMA_SEND_CNT_REG, &r3);
   halReadRegister(pMac, QWLAN_TXP_TXP_PHY_ABORTS_REG, &r4);
   halReadRegister(pMac, QWLAN_TXP_TXP_NR_FRAMES_XMIT_REG, &r5);
   halReadRegister(pMac, QWLAN_RXP_BEACON_TSF_TIM_EXTRACT_CTRL_REG, &r6);

      // MTU registers
   halReadRegister(pMac, QWLAN_MTU_SW_MTU_BCN_SLOT_USEC_SIFS_LIMIT_REG, &r7);
    // IBSS and Mutliple BSS bits
   halReadRegister(pMac, QWLAN_MTU_MTU_FOR_HMAC_CONTROLS_REG, &r8);
   halReadRegister(pMac, QWLAN_MTU_TSF_TIMER_LO_REG, &r9);
   halReadRegister(pMac, QWLAN_MTU_TBTT_L_REG, &r10);
   halReadRegister(pMac, QWLAN_RXP_PHY_MPDU_CNT_REG, &r11);
   halReadRegister(pMac, QWLAN_MTU_VALID_BSSID_BITMAP_REG, &r12);

   HALLOGW( halLog(pMac, LOGW, FL("\nIBSS <<< MTU->TPE bcn req valids = %d\n" \
                      "IBSS <<< TXP PHY ABORTS        = %d\n" \
                      "IBSS <<< TXP NR FRAME XMIT Reg = %d\n" \
                      "IBSS <<< RXP DMA Send counter  = %d\n" \
                      "IBSS <<< RXP Frame Counter     = %d\n" \
                      "IBSS <<< RXP PHY Abort counter = %d\n"),
          ((r1 & QWLAN_TPE_MTU_TPE_INTERFACE_CNTS_MTU_TPE_BCN_REQ_VALID_CNT_MASK) >>
          QWLAN_TPE_MTU_TPE_INTERFACE_CNTS_MTU_TPE_BCN_REQ_VALID_CNT_OFFSET), r4, r5, r3, r11, r2));

   HALLOGW( halLog(pMac, LOGW, FL("\nIBSS <<< MTU Beacon slot   = 0x%08X\n" \
                      "IBSS <<< MTU HMAC controls = 0x%08X\n" \
                      "IBSS <<< MTU TSF LO        = 0x%08X\n" \
                      "IBSS <<< MTU TBTT LO       = 0x%08X\n" \
                      "IBSS <<< RXP BCN TSF TIM EX REG= 0x%08X\n" \
                         "IBSS <<< MTU VALID BSSID BITMAP = 0x%08X\n"),
          r7, r8, r9, r10, r6, r12));
}

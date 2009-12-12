/*
 * Airgo Networks, Inc proprietary. All rights reserved.
 * This file limSendSmeRspMessages.cc contains the functions
 * for sending SME response/notification messages to applications
 * above MAC software.
 * Author:        Chandra Modumudi
 * Date:          02/13/02
 * History:-
 * Date           Modified by    Modification Information
 * --------------------------------------------------------------------
 */

#include "wniApi.h"
#include "halDataStruct.h"
#include "sirCommon.h"
#include "aniGlobal.h"

#if (WNI_POLARIS_FW_PRODUCT == AP)
#include "wniCfgAp.h"
#else
#include "wniCfgSta.h"
#endif
#include "sysDef.h"
#include "cfgApi.h"

#include "halCommonApi.h"
#include "schApi.h"
#include "utilsApi.h"
#include "limUtils.h"
#include "limSecurityUtils.h"
#include "limSerDesUtils.h"
#include "limSendSmeRspMessages.h"



/**
 * limSendSmeRsp()
 *
 *FUNCTION:
 * This function is called by limProcessSmeReqMessages() to send
 * eWNI_SME_START_RSP, eWNI_SME_MEASUREMENT_RSP, eWNI_SME_STOP_BSS_RSP
 * or eWNI_SME_SWITCH_CHL_RSP messages to applications above MAC
 * Software.
 *
 *PARAMS:
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 * NA
 *
 *NOTE:
 * NA
 *
 * @param pMac         Pointer to Global MAC structure
 * @param msgType      Indicates message type
 * @param resultCode   Indicates the result of previously issued
 *                     eWNI_SME_msgType_REQ message
 *
 * @return None
 */

void
limSendSmeRsp(tpAniSirGlobal pMac, tANI_U16 msgType,
              tSirResultCodes resultCode)
{
    tSirMsgQ    mmhMsg;
    tSirSmeRsp  *pSirSmeRsp;

    PELOG1(limLog(pMac, LOG1,
           FL("Sending message %s with reasonCode %s\n"),
           limMsgStr(msgType), limResultCodeStr(resultCode));)

    if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pSirSmeRsp, sizeof(tSirSmeRsp)))
    {
        /// Buffer not available. Log error
        limLog(pMac, LOGP,
               FL("call to palAllocateMemory failed for eWNI_SME_*_RSP\n"));

        return;
    }

#if defined (ANI_PRODUCT_TYPE_AP) && defined (ANI_LITTLE_BYTE_ENDIAN)
    sirStoreU16N((tANI_U8*)&pSirSmeRsp->messageType, msgType);
    sirStoreU16N((tANI_U8*)&pSirSmeRsp->length, sizeof(tSirSmeRsp));
#else
    pSirSmeRsp->messageType = msgType;
    pSirSmeRsp->length      = sizeof(tSirSmeRsp);
#endif
    pSirSmeRsp->statusCode  = resultCode;

    mmhMsg.type = msgType;
    mmhMsg.bodyptr = pSirSmeRsp;
    mmhMsg.bodyval = 0;
    MTRACE(macTraceMsgTx(pMac, 0, mmhMsg.type));

#ifdef FEATURE_WLAN_DIAG_SUPPORT 
    switch(msgType)
    {
        case eWNI_PMC_ENTER_BMPS_RSP:
            limDiagEventReport(pMac, WLAN_PE_DIAG_ENTER_BMPS_RSP_EVENT, NULL, (tANI_U16)resultCode, 0);
            break;
        case eWNI_PMC_EXIT_BMPS_RSP:
            limDiagEventReport(pMac, WLAN_PE_DIAG_EXIT_BMPS_RSP_EVENT, NULL, (tANI_U16)resultCode, 0);
            break;
        case eWNI_PMC_ENTER_IMPS_RSP:
            limDiagEventReport(pMac, WLAN_PE_DIAG_ENTER_IMPS_RSP_EVENT, NULL, (tANI_U16)resultCode, 0);
            break;        
        case eWNI_PMC_EXIT_IMPS_RSP:
            limDiagEventReport(pMac, WLAN_PE_DIAG_EXIT_IMPS_RSP_EVENT, NULL, (tANI_U16)resultCode, 0);
            break;
        case eWNI_PMC_ENTER_UAPSD_RSP:
            limDiagEventReport(pMac, WLAN_PE_DIAG_ENTER_UAPSD_RSP_EVENT, NULL, (tANI_U16)resultCode, 0);
            break;
        case eWNI_PMC_EXIT_UAPSD_RSP:
            limDiagEventReport(pMac, WLAN_PE_DIAG_EXIT_UAPSD_RSP_EVENT, NULL, (tANI_U16)resultCode, 0);
            break;
        case eWNI_SME_SWITCH_CHL_RSP:
            limDiagEventReport(pMac, WLAN_PE_DIAG_SWITCH_CHL_RSP_EVENT, NULL, (tANI_U16)resultCode, 0);
            break;
        case eWNI_SME_STOP_BSS_RSP:
            limDiagEventReport(pMac, WLAN_PE_DIAG_STOP_BSS_RSP_EVENT, NULL, (tANI_U16)resultCode, 0);
            break;		
        case eWNI_PMC_ENTER_WOWL_RSP:
            limDiagEventReport(pMac, WLAN_PE_DIAG_ENTER_WOWL_RSP_EVENT, NULL, (tANI_U16)resultCode, 0);
            break;
        case eWNI_PMC_EXIT_WOWL_RSP:
            limDiagEventReport(pMac, WLAN_PE_DIAG_EXIT_WOWL_RSP_EVENT, NULL, (tANI_U16)resultCode, 0);
            break;			
    }	
#endif //FEATURE_WLAN_DIAG_SUPPORT
	
    limHalMmhPostMsgApi(pMac, &mmhMsg,  ePROT);
} /*** end limSendSmeRsp() ***/



/**
 * limSendSmeJoinReassocRsp()
 *
 *FUNCTION:
 * This function is called by limProcessSmeReqMessages() to send
 * eWNI_SME_JOIN_RSP or eWNI_SME_REASSOC_RSP messages to applications
 * above MAC Software.
 *
 *PARAMS:
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 * NA
 *
 *NOTE:
 * NA
 *
 * @param pMac         Pointer to Global MAC structure
 * @param msgType      Indicates message type
 * @param resultCode   Indicates the result of previously issued
 *                     eWNI_SME_msgType_REQ message
 *
 * @return None
 */

void
limSendSmeJoinReassocRsp(tpAniSirGlobal pMac, tANI_U16 msgType,
                         tSirResultCodes resultCode, tANI_U16 protStatusCode)
{
    tSirMsgQ         mmhMsg;
    tpSirSmeJoinRsp  pSirSmeJoinRsp;
    tANI_U32 rspLen;
    tpDphHashNode pStaDs    = NULL;

#ifdef FEATURE_WLAN_DIAG_SUPPORT 
    if (msgType == eWNI_SME_REASSOC_RSP)
        limDiagEventReport(pMac, WLAN_PE_DIAG_REASSOC_RSP_EVENT, NULL, (tANI_U16)resultCode, 0);
    else
        limDiagEventReport(pMac, WLAN_PE_DIAG_JOIN_RSP_EVENT, NULL, (tANI_U16)resultCode, 0);
#endif //FEATURE_WLAN_DIAG_SUPPORT

    PELOG1(limLog(pMac, LOG1,
           FL("Sending message %s with reasonCode %s\n"),
           limMsgStr(msgType), limResultCodeStr(resultCode));)

    rspLen = pMac->lim.assocReqLen + pMac->lim.assocRspLen + pMac->lim.bcnLen + sizeof(tSirSmeJoinRsp) - sizeof(tANI_U8) ;
    
    if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pSirSmeJoinRsp, rspLen))
    {
        /// Buffer not available. Log error
        limLog(pMac, LOGP,
               FL("call to palAllocateMemory failed for JOIN/REASSOC_RSP\n"));

        return;
    }

#if defined (ANI_PRODUCT_TYPE_AP) && defined (ANI_LITTLE_BYTE_ENDIAN)
    sirStoreU16N((tANI_U8*)&pSirSmeJoinRsp->messageType, msgType);
    sirStoreU16N((tANI_U8*)&pSirSmeJoinRsp->length, rspLen);
#else
    pSirSmeJoinRsp->messageType = msgType;
    pSirSmeJoinRsp->length = (tANI_U16) rspLen;
#endif

    pSirSmeJoinRsp->statusCode  = resultCode;
#if (WNI_POLARIS_FW_PRODUCT == WLAN_STA)
    if (resultCode == eSIR_SME_SUCCESS)
    {
        pStaDs = dphGetHashEntry(pMac, DPH_STA_HASH_INDEX_PEER);
        if (pStaDs == NULL)
        {
            PELOGE(limLog(pMac, LOGE, FL("could not Get Self Entry for the station\n"));)
        }
        else
        {
                //Pass the peer's staId
            pSirSmeJoinRsp->staId = pStaDs->staIndex;
        }
    }
#endif

#if (WNI_POLARIS_FW_PACKAGE == ADVANCED)
    if (resultCode == eSIR_SME_TRANSFER_STA)
    {
        palCopyMemory( pMac->hHdd, pSirSmeJoinRsp->alternateBssId,
                      pMac->lim.gLimAlternateRadio.bssId,
                      sizeof(tSirMacAddr));
        pSirSmeJoinRsp->alternateChannelId =
                               pMac->lim.gLimAlternateRadio.channelId;
    }
#endif

pSirSmeJoinRsp->beaconLength = 0;
pSirSmeJoinRsp->assocReqLength = 0;
pSirSmeJoinRsp->assocRspLength = 0;

if(resultCode == eSIR_SME_SUCCESS)
{

    if(pMac->lim.beacon != NULL)
    {
        pSirSmeJoinRsp->beaconLength = pMac->lim.bcnLen;
        palCopyMemory(pMac->hHdd, pSirSmeJoinRsp->frames, pMac->lim.beacon, pSirSmeJoinRsp->beaconLength);
        palFreeMemory(pMac->hHdd, pMac->lim.beacon);
        pMac->lim.beacon = NULL;
    }

    if(pMac->lim.assocReq != NULL)
    {
        pSirSmeJoinRsp->assocReqLength = pMac->lim.assocReqLen;
        palCopyMemory(pMac->hHdd, pSirSmeJoinRsp->frames + pMac->lim.bcnLen, pMac->lim.assocReq, pSirSmeJoinRsp->assocReqLength);
        palFreeMemory(pMac->hHdd, pMac->lim.assocReq);
        pMac->lim.assocReq = NULL;
    }
    if(pMac->lim.assocRsp != NULL)
    {
        pSirSmeJoinRsp->assocRspLength = pMac->lim.assocRspLen;
        palCopyMemory(pMac->hHdd, pSirSmeJoinRsp->frames + pMac->lim.bcnLen + pMac->lim.assocReqLen, pMac->lim.assocRsp, pSirSmeJoinRsp->assocRspLength);
        palFreeMemory(pMac->hHdd, pMac->lim.assocRsp);
        pMac->lim.assocRsp = NULL;
    }
}
#if defined (ANI_PRODUCT_TYPE_AP) && defined(ANI_LITTLE_BYTE_ENDIAN)
    sirStoreU16N((tANI_U8*)&pSirSmeJoinRsp->protStatusCode, protStatusCode);
    sirStoreU16N((tANI_U8*)&pSirSmeJoinRsp->aid, pMac->lim.gLimAID);
#else
    pSirSmeJoinRsp->protStatusCode = protStatusCode;
    pSirSmeJoinRsp->aid = pMac->lim.gLimAID;
#endif

    mmhMsg.type = msgType;
    mmhMsg.bodyptr = pSirSmeJoinRsp;
    mmhMsg.bodyval = 0;
    MTRACE(macTraceMsgTx(pMac, 0, mmhMsg.type));
    limHalMmhPostMsgApi(pMac, &mmhMsg,  ePROT);
} /*** end limSendSmeJoinReassocRsp() ***/



/**
 * limSendSmeStartBssRsp()
 *
 *FUNCTION:
 * This function is called to send eWNI_SME_START_BSS_RSP
 * message to applications above MAC Software.
 *
 *PARAMS:
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 * NA
 *
 *NOTE:
 * NA
 *
 * @param pMac         Pointer to Global MAC structure
 * @param msgType      Indicates message type
 * @param resultCode   Indicates the result of previously issued
 *                     eWNI_SME_msgType_REQ message
 *
 * @return None
 */

void
limSendSmeStartBssRsp(tpAniSirGlobal pMac,
                      tANI_U16 msgType, tSirResultCodes resultCode)
{
    tANI_U16            size = 0;
    tSirMsgQ            mmhMsg;
    tSirSmeStartBssRsp  *pSirSmeRsp;
    tANI_U16            ieLen;
    tANI_U32            len;
    tANI_U16  ieOffset, curLen;

    PELOG1(limLog(pMac, LOG1, FL("Sending message %s with reasonCode %s\n"),
           limMsgStr(msgType), limResultCodeStr(resultCode));)

    size = sizeof(tSirSmeStartBssRsp);

    //subtract size of beaconLength + Mac Hdr + Fixed Fields before SSID
    ieOffset = sizeof(tAniBeaconStruct) + SIR_MAC_B_PR_SSID_OFFSET;
    ieLen = pMac->sch.schObject.gSchBeaconOffsetBegin + pMac->sch.schObject.gSchBeaconOffsetEnd -
                ieOffset;
    //calculate the memory size to allocate
    size += ieLen;

    if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pSirSmeRsp, size))
    {
        /// Buffer not available. Log error
        limLog(pMac, LOGP,
           FL("call to palAllocateMemory failed for eWNI_SME_START_BSS_RSP\n"));

        return;
    }

    size = sizeof(tSirSmeStartBssRsp);

    if (resultCode == eSIR_SME_SUCCESS)
    {
        len  = SIR_MAC_ADDR_LENGTH;
        if (wlan_cfgGetStr(pMac, WNI_CFG_BSSID, pSirSmeRsp->bssDescription.bssId, &len)
            != eSIR_SUCCESS)
            limLog(pMac, LOGP, FL("could not retrive BSSID\n"));

        if (wlan_cfgGetInt(pMac, WNI_CFG_BEACON_INTERVAL, &len) != eSIR_SUCCESS)
            limLog(pMac, LOGP, FL("could not retrieve Beacon interval\n"));
        pSirSmeRsp->bssDescription.beaconInterval = (tANI_U16) len;

        if (cfgGetCapabilityInfo( pMac, &pSirSmeRsp->bssDescription.capabilityInfo)
            != eSIR_SUCCESS)
            limLog(pMac, LOGP, FL("could not retrieve Capabilities value\n"));

        if (wlan_cfgGetInt(pMac, WNI_CFG_PHY_MODE, (tANI_U32 *) &pSirSmeRsp->bssDescription.nwType)
            != eSIR_SUCCESS)
            limLog(pMac, LOGP, FL("could not retrieve nwType from CFG\n"));

        if (wlan_cfgGetInt(pMac, WNI_CFG_CURRENT_CHANNEL, &len) != eSIR_SUCCESS)
            limLog(pMac, LOGP, FL("could not retrieve CURRENT_CHANNEL from CFG\n"));
        pSirSmeRsp->bssDescription.channelId = (tANI_U8)len;


        pSirSmeRsp->bssDescription.aniIndicator = 1;

        curLen = pMac->sch.schObject.gSchBeaconOffsetBegin - ieOffset;
        palCopyMemory( pMac->hHdd, (tANI_U8 *) &pSirSmeRsp->bssDescription.ieFields,
                       pMac->sch.schObject.gSchBeaconFrameBegin + ieOffset,
                      (tANI_U32)curLen);

        palCopyMemory( pMac->hHdd, ((tANI_U8 *) &pSirSmeRsp->bssDescription.ieFields) + curLen,
                       pMac->sch.schObject.gSchBeaconFrameEnd,
                      (tANI_U32)pMac->sch.schObject.gSchBeaconOffsetEnd);


        //subtracting size of length indicator itself and size of pointer to ieFields
        pSirSmeRsp->bssDescription.length = sizeof(tSirBssDescription) -
                                            sizeof(tANI_U16) - sizeof(tANI_U32) +
                                            ieLen;
        //This is the size of the message, subtracting the size of the pointer to ieFields
        size += ieLen - sizeof(tANI_U32);
    }

#if defined (ANI_PRODUCT_TYPE_AP) && defined (ANI_LITTLE_BYTE_ENDIAN)
    sirStoreU16N((tANI_U8*)&pSirSmeRsp->messageType, msgType);
    sirStoreU16N((tANI_U8*)&pSirSmeRsp->length, size);
#else
    pSirSmeRsp->messageType = msgType;
    pSirSmeRsp->length      = size;
#endif
    pSirSmeRsp->statusCode  = resultCode;

    mmhMsg.type = msgType;
    mmhMsg.bodyptr = pSirSmeRsp;
    mmhMsg.bodyval = 0;
    MTRACE(macTraceMsgTx(pMac, 0, mmhMsg.type));

#ifdef FEATURE_WLAN_DIAG_SUPPORT 
    limDiagEventReport(pMac, WLAN_PE_DIAG_START_BSS_RSP_EVENT, NULL, (tANI_U16)resultCode, 0);
#endif //FEATURE_WLAN_DIAG_SUPPORT

    limHalMmhPostMsgApi(pMac, &mmhMsg,  ePROT);
} /*** end limSendSmeStartBssRsp() ***/





/**
 * limSendSmeScanRsp()
 *
 *FUNCTION:
 * This function is called by limProcessSmeReqMessages() to send
 * eWNI_SME_SCAN_RSP message to applications above MAC
 * Software.
 *
 *PARAMS:
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 * NA
 *
 *NOTE:
 * NA
 *
 * @param pMac         Pointer to Global MAC structure
 * @param length       Indicates length of message
 * @param resultCode   Indicates the result of previously issued
 *                     eWNI_SME_SCAN_REQ message
 *
 * @return None
 */

void
limSendSmeScanRsp(tpAniSirGlobal pMac, tANI_U16 length,
                  tSirResultCodes resultCode)
{
    tSirMsgQ              mmhMsg;
    tpSirSmeScanRsp       pSirSmeScanRsp=NULL;
    tLimScanResultNode    *ptemp;
    tANI_U16                   msgLen;
    tANI_U8                    found = false;
    tANI_U16                   i;

    PELOG1(limLog(pMac, LOG1,
       FL("Sending message SME_SCAN_RSP with length=%d reasonCode %s\n"),
       length, limResultCodeStr(resultCode));)

    if (resultCode != eSIR_SME_SUCCESS)
    {
        limPostSmeScanRspMessage(pMac, length, resultCode);   
        return;
    }

    mmhMsg.type = eWNI_SME_SCAN_RSP;
    for (i = 0; i < LIM_MAX_NUM_OF_SCAN_RESULTS; i++)
    {
        if ((ptemp = pMac->lim.gLimCachedScanHashTable[i]) != NULL)
        {
            while(ptemp)
            {
                if (found)
                {
                    pSirSmeScanRsp->statusCode  =
                                        eSIR_SME_MORE_SCAN_RESULTS_FOLLOW;
                    mmhMsg.bodyptr = pSirSmeScanRsp;
                    mmhMsg.bodyval = 0;
                    MTRACE(macTraceMsgTx(pMac, 0, mmhMsg.type));
                    limHalMmhPostMsgApi(pMac, &mmhMsg, ePROT);
                    PELOG2(limLog(pMac, LOG2, FL("statusCode : eSIR_SME_MORE_SCAN_RESULTS_FOLLOW\n"));)
                }
                msgLen = sizeof(tSirSmeScanRsp) -
                         sizeof(tSirBssDescription) +
                         ptemp->bssDescription.length + 2;
                if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pSirSmeScanRsp, msgLen))
                {
                    // Log error
                    limLog(pMac, LOGP,
                        FL("call to palAllocateMemory failed for eWNI_SME_SCAN_RSP\n"));

                    return;
                }

               PELOG2(limLog(pMac, LOG2, FL("ScanRsp : msgLen %d, bssDescr Len=%d\n"),
                       msgLen, ptemp->bssDescription.length);)
#if defined (ANI_PRODUCT_TYPE_AP) && defined (ANI_LITTLE_BYTE_ENDIAN)
                sirStoreU16N((tANI_U8*)&pSirSmeScanRsp->bssDescription[0].length,
                             ptemp->bssDescription.length);
#else
                pSirSmeScanRsp->bssDescription[0].length
                    = ptemp->bssDescription.length;
#endif
                palCopyMemory( pMac->hHdd, (tANI_U8 *) &pSirSmeScanRsp->bssDescription[0].bssId,
                              (tANI_U8 *) &ptemp->bssDescription.bssId,
                              ptemp->bssDescription.length);

#if defined (ANI_PRODUCT_TYPE_AP) && defined (ANI_LITTLE_BYTE_ENDIAN)
                sirStoreU16N((tANI_U8*)&pSirSmeScanRsp->messageType,
                             eWNI_SME_SCAN_RSP);
                sirStoreU16N((tANI_U8*)&pSirSmeScanRsp->length, msgLen);
#else
                pSirSmeScanRsp->messageType = eWNI_SME_SCAN_RSP;
                pSirSmeScanRsp->length      = msgLen;
#endif
                PELOG2(limLog(pMac, LOG2, FL("BssId "));
                limPrintMacAddr(pMac, ptemp->bssDescription.bssId, LOG2);)

                found = true;

                ptemp = ptemp->next;
            }
        }
    }

    if (found)
    {
        // send last message
        pSirSmeScanRsp->statusCode  = eSIR_SME_SUCCESS;

        mmhMsg.type = eWNI_SME_SCAN_RSP;
        mmhMsg.bodyptr = pSirSmeScanRsp;
        mmhMsg.bodyval = 0;
        MTRACE(macTraceMsgTx(pMac, 0, mmhMsg.type));
        limHalMmhPostMsgApi(pMac, &mmhMsg, ePROT);
        PELOG2(limLog(pMac, LOG2, FL("statusCode : eSIR_SME_SUCCESS\n"));)
    }
    else  {
                  limPostSmeScanRspMessage(pMac, length, resultCode);    
             }
    return;

} /*** end limSendSmeScanRsp() ***/


/**
 * limPostSmeScanRspMessage()
 *
 *FUNCTION:
 * This function is called by limSendSmeScanRsp() to send
 * eWNI_SME_SCAN_RSP message with failed result code
 *
 *NOTE:
 * NA
 *
 * @param pMac         Pointer to Global MAC structure
 * @param length       Indicates length of message
 * @param resultCode   failed result code
 *
 * @return None
 */

void
limPostSmeScanRspMessage(tpAniSirGlobal    pMac,     
                      tANI_U16               length,
                      tSirResultCodes   resultCode)
{
    tpSirSmeScanRsp   pSirSmeScanRsp;
    tSirMsgQ          mmhMsg;

    PELOG1(limLog(pMac, LOG1,
       FL("limPostSmeScanRspMessage: send SME_SCAN_RSP (len %d, reasonCode %s). \n"),
       length, limResultCodeStr(resultCode));)

    if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pSirSmeScanRsp, length))
    {
        limLog(pMac, LOGP, FL("palAllocateMemory failed for eWNI_SME_SCAN_RSP\n"));
        return;
    }

#if defined (ANI_PRODUCT_TYPE_AP) && defined (ANI_LITTLE_BYTE_ENDIAN)
    sirStoreU16N((tANI_U8*)&pSirSmeScanRsp->messageType, eWNI_SME_SCAN_RSP);
    sirStoreU16N((tANI_U8*)&pSirSmeScanRsp->length, length);
#else
    pSirSmeScanRsp->messageType = eWNI_SME_SCAN_RSP;
    pSirSmeScanRsp->length      = length;
#endif

    pSirSmeScanRsp->statusCode  = resultCode;
    mmhMsg.type = eWNI_SME_SCAN_RSP;
    mmhMsg.bodyptr = pSirSmeScanRsp;
    mmhMsg.bodyval = 0;

    MTRACE(macTraceMsgTx(pMac, 0, mmhMsg.type));
#ifdef FEATURE_WLAN_DIAG_SUPPORT 
    limDiagEventReport(pMac, WLAN_PE_DIAG_SCAN_RSP_EVENT, NULL, (tANI_U16)resultCode, 0);
#endif //FEATURE_WLAN_DIAG_SUPPORT

    limHalMmhPostMsgApi(pMac, &mmhMsg, ePROT);
    return;

}  /*** limPostSmeScanRspMessage ***/



/**
 * limSendSmeAuthRsp()
 *
 *FUNCTION:
 * This function is called by limProcessSmeMessages() to send
 * eWNI_SME_AUTH_RSP message to host
 *
 *PARAMS:
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 * NA
 *
 *NOTE:
 * NA
 *
 * @param  pMac        Pointer to Global MAC structure
 * @param statusCode   Indicates the result of previously issued
 *                     eWNI_SME_AUTH_REQ message
 *
 * @return None
 */
void
limSendSmeAuthRsp(tpAniSirGlobal pMac,
                  tSirResultCodes statusCode,
                  tSirMacAddr peerMacAddr,
                  tAniAuthType authType,
                  tANI_U16   protStatusCode)
{
#if 0
    tSirMsgQ       mmhMsg;
    tSirSmeAuthRsp *pSirSmeAuthRsp;


    if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pSirSmeAuthRsp, sizeof(tSirSmeAuthRsp)))
    {
        // Log error
        limLog(pMac, LOGP,
               FL("call to palAllocateMemory failed for eWNI_SME_AUTH_RSP\n"));

        return;
    }

#if defined (ANI_PRODUCT_TYPE_AP) && defined (ANI_LITTLE_BYTE_ENDIAN)
    sirStoreU16N((tANI_U8*)&pSirSmeAuthRsp->messageType, eWNI_SME_AUTH_RSP);
    sirStoreU16N((tANI_U8*)&pSirSmeAuthRsp->length, sizeof(tSirSmeAuthRsp));
#else
    pSirSmeAuthRsp->messageType = eWNI_SME_AUTH_RSP;
    pSirSmeAuthRsp->length      = sizeof(tSirSmeAuthRsp);
#endif
    pSirSmeAuthRsp->statusCode  = statusCode;
    palCopyMemory( pMac->hHdd, (tANI_U8 *) pSirSmeAuthRsp->peerMacAddr,
                  (tANI_U8 *) peerMacAddr, sizeof(tSirMacAddr));
    pSirSmeAuthRsp->authType    = authType;
    pSirSmeAuthRsp->protStatusCode = protStatusCode;

    mmhMsg.type = eWNI_SME_AUTH_RSP;
    mmhMsg.bodyptr = pSirSmeAuthRsp;
    mmhMsg.bodyval = 0;
    MTRACE(macTraceMsgTx(pMac, 0, mmhMsg.type));
    limHalMmhPostMsgApi(pMac, &mmhMsg,  ePROT);
#endif
} /*** end limSendSmeAuthRsp() ***/



/**
 * limSendSmeDisassocNtf()
 *
 *FUNCTION:
 * This function is called by limProcessSmeMessages() to send
 * eWNI_SME_DISASSOC_RSP/IND message to host
 *
 *PARAMS:
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 * NA
 *
 *NOTE:
 * This function is used for sending eWNI_SME_DISASSOC_CNF,
 * or eWNI_SME_DISASSOC_IND to host depending on
 * disassociation trigger.
 *
 * @param peerMacAddr       Indicates the peer MAC addr to which
 *                          disassociate was initiated
 * @param reasonCode        Indicates the reason for Disassociation
 * @param disassocTrigger   Indicates the trigger for Disassociation
 * @param aid               Indicates the STAID. This parameter is
 *                          present only on AP.
 *
 * @return None
 */
void
limSendSmeDisassocNtf(tpAniSirGlobal pMac,
                      tSirMacAddr peerMacAddr,
                      tSirResultCodes reasonCode,
                      tANI_U16 disassocTrigger, tANI_U16 aid)
{
    tANI_U8            *pBuf;
    tSirMsgQ           mmhMsg;
    tSirSmeDisassocRsp *pSirSmeDisassocRsp;
    tSirSmeDisassocInd *pSirSmeDisassocInd;

    switch (disassocTrigger)
    {
        case eLIM_PEER_ENTITY_DISASSOC:
            return;

        case eLIM_HOST_DISASSOC:
            /**
             * Disassociation response due to
             * host triggered disassociation
             */
            if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pSirSmeDisassocRsp, sizeof(tSirSmeDisassocRsp)))
            {
                // Log error
                limLog(pMac, LOGP,
                   FL("call to palAllocateMemory failed for eWNI_SME_DISASSOC_RSP\n"));

                return;
            }


#if defined (ANI_PRODUCT_TYPE_AP) && defined (ANI_LITTLE_BYTE_ENDIAN)
            sirStoreU16N((tANI_U8*)&pSirSmeDisassocRsp->messageType,
                         eWNI_SME_DISASSOC_RSP);
            sirStoreU16N((tANI_U8*)&pSirSmeDisassocRsp->length,
                         sizeof(tSirSmeDisassocRsp));
#else
            pSirSmeDisassocRsp->messageType = eWNI_SME_DISASSOC_RSP;
            pSirSmeDisassocRsp->length      = sizeof(tSirSmeDisassocRsp);
#endif
            pBuf = (tANI_U8 *) pSirSmeDisassocRsp->peerMacAddr;
            palCopyMemory( pMac->hHdd, pBuf, peerMacAddr, sizeof(tSirMacAddr));
            pBuf += sizeof(tSirMacAddr);

            limCopyU32(pBuf, reasonCode);

#if (WNI_POLARIS_FW_PRODUCT == AP)
            pBuf += sizeof(tSirResultCodes);
            limCopyU16(pBuf, aid);
            pBuf += sizeof(tANI_U16);

            limStatSerDes(pMac, &pMac->hal.halMac.macStats.pPerStaStats[aid].staStat, pBuf);

#else
            // Clear Station Stats
            //for sta, it is always 1, IBSS is handled at halInitSta

#endif//#if (WNI_POLARIS_FW_PRODUCT == AP)
            mmhMsg.type = eWNI_SME_DISASSOC_RSP;
            mmhMsg.bodyptr = pSirSmeDisassocRsp;
            mmhMsg.bodyval = 0;

            break;

        default:
            /**
             * Disassociation indication due to Disassociation
             * frame reception from peer entity or due to
             * loss of link with peer entity.
             */
            if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pSirSmeDisassocInd, sizeof(tSirSmeDisassocInd)))
            {
                // Log error
                limLog(pMac, LOGP,
                   FL("call to palAllocateMemory failed for eWNI_SME_DISASSOC_IND\n"));

                return;
            }

#if defined (ANI_PRODUCT_TYPE_AP) && defined (ANI_LITTLE_BYTE_ENDIAN)
            sirStoreU16N((tANI_U8*)&pSirSmeDisassocInd->messageType,
                         eWNI_SME_DISASSOC_IND);
            sirStoreU16N((tANI_U8*)&pSirSmeDisassocInd->length,
                         sizeof(tSirSmeDisassocInd));
#else
            pSirSmeDisassocInd->messageType = eWNI_SME_DISASSOC_IND;
            pSirSmeDisassocInd->length      = sizeof(tSirSmeDisassocInd);
#endif
            pBuf = (tANI_U8 *) &pSirSmeDisassocInd->statusCode;

            limCopyU32(pBuf, reasonCode);
            pBuf += sizeof(tSirResultCodes);

            palCopyMemory( pMac->hHdd, pBuf, peerMacAddr, sizeof(tSirMacAddr));
#if (WNI_POLARIS_FW_PRODUCT == AP)
            pBuf += sizeof(tSirMacAddr);
            limCopyU16(pBuf, aid);
            pBuf += sizeof(tANI_U16);

            limStatSerDes(pMac, &pMac->hal.halMac.macStats.pPerStaStats[aid].staStat, pBuf);

#endif//#if (WNI_POLARIS_FW_PRODUCT == AP)

            mmhMsg.type = eWNI_SME_DISASSOC_IND;
            mmhMsg.bodyptr = pSirSmeDisassocInd;
            mmhMsg.bodyval = 0;

#if (WNI_POLARIS_FW_PRODUCT == AP)
            PELOG1(limLog(pMac, LOG1,
               FL("*** Sending DisAssocInd staId=%d, reasonCode=%d ***\n"),
               aid, reasonCode);)
#endif

            break;
    }
    MTRACE(macTraceMsgTx(pMac, 0, mmhMsg.type));

#ifdef FEATURE_WLAN_DIAG_SUPPORT 
    if (mmhMsg.type == eWNI_SME_DISASSOC_RSP)
        limDiagEventReport(pMac, WLAN_PE_DIAG_DISASSOC_RSP_EVENT, NULL, (tANI_U16)reasonCode, 0);
    else if (mmhMsg.type == eWNI_SME_DISASSOC_IND)
        limDiagEventReport(pMac, WLAN_PE_DIAG_DISASSOC_IND_EVENT, NULL, (tANI_U16)reasonCode, 0);		
#endif //FEATURE_WLAN_DIAG_SUPPORT
	
    limHalMmhPostMsgApi(pMac, &mmhMsg, ePROT);
} /*** end limSendSmeDisassocNtf() ***/


/** -----------------------------------------------------------------
  \brief limSendSmeDisassocInd() - sends SME_DISASSOC_IND
   
  After receiving disassociation frame from peer entity, this 
  function sends a eWNI_SME_DISASSOC_IND to SME with a specific
  reason code.  
    
  \param pMac - global mac structure
  \param pStaDs - station dph hash node 
  \return none 
  \sa
  ----------------------------------------------------------------- */
void
limSendSmeDisassocInd(tpAniSirGlobal pMac, tpDphHashNode pStaDs)
{
    tANI_U8  *pBuf;
    tSirMsgQ  mmhMsg;
    tSirSmeDisassocInd *pSirSmeDisassocInd;

    if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pSirSmeDisassocInd, sizeof(tSirSmeDisassocInd)))
    {
        limLog(pMac, LOGP, FL("palAllocateMemory failed for eWNI_SME_DISASSOC_IND\n"));
        return;
    }

#if defined (ANI_PRODUCT_TYPE_AP) && defined (ANI_LITTLE_BYTE_ENDIAN)
    sirStoreU16N((tANI_U8*)&pSirSmeDisassocInd->messageType, eWNI_SME_DISASSOC_IND);
    sirStoreU16N((tANI_U8*)&pSirSmeDisassocInd->length, sizeof(tSirSmeDisassocInd));
#else
    pSirSmeDisassocInd->messageType = eWNI_SME_DISASSOC_IND;
    pSirSmeDisassocInd->length = sizeof(tSirSmeDisassocInd);
#endif

    //statusCode
    pBuf = (tANI_U8 *) &pSirSmeDisassocInd->statusCode;
    limCopyU32(pBuf, pStaDs->mlmStaContext.disassocReason);
    pBuf += sizeof(tSirResultCodes);

    //peerMacAddr
    palCopyMemory( pMac->hHdd, pBuf, pStaDs->staAddr, sizeof(tSirMacAddr));

#ifdef ANI_PRODUCT_TYPE_AP
    pBuf += sizeof(tSirMacAddr);
    //aid
    limCopyU16(pBuf, pStaDs->assocId);
    pBuf += sizeof(tANI_U16);

    //perStaStats
    limStatSerDes(pMac, &pMac->hal.halMac.macStats.pPerStaStats[pStaDs->assocId].staStat, pBuf);
#endif

    mmhMsg.type = eWNI_SME_DISASSOC_IND;
    mmhMsg.bodyptr = pSirSmeDisassocInd;
    mmhMsg.bodyval = 0;

    MTRACE(macTraceMsgTx(pMac, 0, mmhMsg.type));
#ifdef FEATURE_WLAN_DIAG_SUPPORT 
    limDiagEventReport(pMac, WLAN_PE_DIAG_DISASSOC_IND_EVENT, NULL, 0, (tANI_U16)pStaDs->mlmStaContext.disassocReason);	
#endif //FEATURE_WLAN_DIAG_SUPPORT

    limHalMmhPostMsgApi(pMac, &mmhMsg, ePROT);
} /*** end limSendSmeDisassocInd() ***/


/** -----------------------------------------------------------------
  \brief limSendSmeDeauthInd() - sends SME_DEAUTH_IND
   
  After receiving deauthentication frame from peer entity, this 
  function sends a eWNI_SME_DEAUTH_IND to SME with a specific
  reason code.  
    
  \param pMac - global mac structure
  \param pStaDs - station dph hash node 
  \return none 
  \sa
  ----------------------------------------------------------------- */
void
limSendSmeDeauthInd(tpAniSirGlobal pMac, tpDphHashNode pStaDs)					
{
    tANI_U8  *pBuf;
    tSirMsgQ  mmhMsg;
    tSirSmeDeauthInd  *pSirSmeDeauthInd;

    if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pSirSmeDeauthInd, sizeof(tSirSmeDeauthInd)))
    {
        limLog(pMac, LOGP, FL("palAllocateMemory failed for eWNI_SME_DEAUTH_IND \n"));
        return;
    }

#if defined (ANI_PRODUCT_TYPE_AP) && defined (ANI_LITTLE_BYTE_ENDIAN)
    sirStoreU16N((tANI_U8*)&pSirSmeDeauthInd->messageType, eWNI_SME_DEAUTH_IND);
    sirStoreU16N((tANI_U8*)&pSirSmeDeauthInd->length, sizeof(tSirSmeDeauthInd));
#else
    pSirSmeDeauthInd->messageType = eWNI_SME_DEAUTH_IND;
    pSirSmeDeauthInd->length = sizeof(tSirSmeDeauthInd);
#endif
    
    //statusCode
    pBuf  = (tANI_U8 *) &pSirSmeDeauthInd->statusCode;
    limCopyU32(pBuf, pStaDs->mlmStaContext.cleanupTrigger);
    pBuf += sizeof(tSirResultCodes);

    //peerMacAddr
    palCopyMemory( pMac->hHdd, pBuf, pStaDs->staAddr, sizeof(tSirMacAddr));

#if (WNI_POLARIS_FW_PRODUCT == AP)
    pBuf += sizeof(tSirMacAddr);
    limCopyU16(pBuf, pStaDs->staAddr);
#endif

    mmhMsg.type = eWNI_SME_DEAUTH_IND;
    mmhMsg.bodyptr = pSirSmeDeauthInd;
    mmhMsg.bodyval = 0;

    MTRACE(macTraceMsgTx(pMac, 0, mmhMsg.type));
#ifdef FEATURE_WLAN_DIAG_SUPPORT 
    limDiagEventReport(pMac, WLAN_PE_DIAG_DEAUTH_IND_EVENT, NULL, 0, pStaDs->mlmStaContext.cleanupTrigger);
#endif //FEATURE_WLAN_DIAG_SUPPORT

    limHalMmhPostMsgApi(pMac, &mmhMsg, ePROT);
    return;
} /*** end limSendSmeDeauthInd() ***/


/**
 * limSendSmeDeauthNtf()
 *
 *FUNCTION:
 * This function is called by limProcessSmeMessages() to send
 * eWNI_SME_DISASSOC_RSP/IND message to host
 *
 *PARAMS:
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 * NA
 *
 *NOTE:
 * This function is used for sending eWNI_SME_DEAUTH_CNF or
 * eWNI_SME_DEAUTH_IND to host depending on deauthentication trigger.
 *
 * @param peerMacAddr       Indicates the peer MAC addr to which
 *                          deauthentication was initiated
 * @param reasonCode        Indicates the reason for Deauthetication
 * @param deauthTrigger     Indicates the trigger for Deauthetication
 * @param aid               Indicates the STAID. This parameter is present
 *                          only on AP.
 *
 * @return None
 */
void
limSendSmeDeauthNtf(tpAniSirGlobal pMac, tSirMacAddr peerMacAddr, tSirResultCodes reasonCode,
                    tANI_U16 deauthTrigger, tANI_U16 aid)
{
    tANI_U8                 *pBuf;
    tSirMsgQ           mmhMsg;
    tSirSmeDeauthRsp   *pSirSmeDeauthRsp;
    tSirSmeDeauthInd   *pSirSmeDeauthInd;

    switch (deauthTrigger)
    {
        case eLIM_PEER_ENTITY_DEAUTH:
            return;
			
        case eLIM_HOST_DEAUTH:
            /**
             * Deauthentication response to host triggered
             * deauthentication.
             */
            if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pSirSmeDeauthRsp, sizeof(tSirSmeDeauthRsp)))
            {
                // Log error
                limLog(pMac, LOGP,
                   FL("call to palAllocateMemory failed for eWNI_SME_DEAUTH_RSP\n"));

                return;
            }

#if defined (ANI_PRODUCT_TYPE_AP) && defined (ANI_LITTLE_BYTE_ENDIAN)
            sirStoreU16N((tANI_U8*) &(pSirSmeDeauthRsp->messageType),
                         eWNI_SME_DEAUTH_RSP);
                sirStoreU16N((tANI_U8*) &(pSirSmeDeauthRsp->length),
                             sizeof(tSirSmeDeauthRsp));
#else
            pSirSmeDeauthRsp->messageType = eWNI_SME_DEAUTH_RSP;
            pSirSmeDeauthRsp->length      = sizeof(tSirSmeDeauthRsp);
#endif
            pBuf  = (tANI_U8 *) pSirSmeDeauthRsp->peerMacAddr;
            palCopyMemory( pMac->hHdd, pBuf, peerMacAddr, sizeof(tSirMacAddr));
            pBuf += sizeof(tSirMacAddr);

            limCopyU32(pBuf, reasonCode);
#if (WNI_POLARIS_FW_PRODUCT == AP)
            pBuf += sizeof(tSirResultCodes);
            limCopyU16(pBuf, aid);
#endif
            mmhMsg.type = eWNI_SME_DEAUTH_RSP;
            mmhMsg.bodyptr = pSirSmeDeauthRsp;
            mmhMsg.bodyval = 0;

            break;

        default:
            /**
             * Deauthentication indication due to Deauthentication
             * frame reception from peer entity or due to
             * loss of link with peer entity.
             */
            if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pSirSmeDeauthInd, sizeof(tSirSmeDeauthInd)))
            {
                // Log error
                limLog(pMac, LOGP,
                   FL("call to palAllocateMemory failed for eWNI_SME_DEAUTH_Ind\n"));

                return;
            }

#if defined (ANI_PRODUCT_TYPE_AP) && defined (ANI_LITTLE_BYTE_ENDIAN)
            sirStoreU16N((tANI_U8*)&pSirSmeDeauthInd->messageType,
                         eWNI_SME_DEAUTH_IND);
            sirStoreU16N((tANI_U8*)&pSirSmeDeauthInd->length,
                         sizeof(tSirSmeDeauthInd));
#else
            pSirSmeDeauthInd->messageType = eWNI_SME_DEAUTH_IND;
            pSirSmeDeauthInd->length      = sizeof(tSirSmeDeauthInd);
#endif
            pBuf  = (tANI_U8 *) &pSirSmeDeauthInd->statusCode;
            limCopyU32(pBuf, reasonCode);
            pBuf += sizeof(tSirResultCodes);

            palCopyMemory( pMac->hHdd, pBuf, peerMacAddr, sizeof(tSirMacAddr));
#if (WNI_POLARIS_FW_PRODUCT == AP)
            pBuf += sizeof(tSirMacAddr);
            limCopyU16(pBuf, aid);
#endif
            mmhMsg.type = eWNI_SME_DEAUTH_IND;
            mmhMsg.bodyptr = pSirSmeDeauthInd;
            mmhMsg.bodyval = 0;

            break;
    }
    MTRACE(macTraceMsgTx(pMac, 0, mmhMsg.type));

#ifdef FEATURE_WLAN_DIAG_SUPPORT 
    if (mmhMsg.type == eWNI_SME_DEAUTH_RSP)
        limDiagEventReport(pMac, WLAN_PE_DIAG_DEAUTH_RSP_EVENT, NULL, 0, (tANI_U16)reasonCode);
    else if (mmhMsg.type == eWNI_SME_DEAUTH_IND)
        limDiagEventReport(pMac, WLAN_PE_DIAG_DEAUTH_IND_EVENT, NULL, 0, (tANI_U16)reasonCode);
#endif //FEATURE_WLAN_DIAG_SUPPORT
	
	
    limHalMmhPostMsgApi(pMac, &mmhMsg, ePROT);
} /*** end limSendSmeDeauthNtf() ***/


/**
 * limSendSmeWmStatusChangeNtf()
 *
 *FUNCTION:
 * This function is called by limProcessSmeMessages() to send
 * eWNI_SME_WM_STATUS_CHANGE_NTF message to host.
 *
 *PARAMS:
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 * NA
 *
 *NOTE:
 *
 * @param statusChangeCode   Indicates the change in the wireless medium.
 * @param statusChangeInfo   Indicates the information associated with
 *                           change in the wireless medium.
 * @param infoLen            Indicates the length of status change information
 *                           being sent.
 *
 * @return None
 */
void
limSendSmeWmStatusChangeNtf(tpAniSirGlobal pMac, tSirSmeStatusChangeCode statusChangeCode,
                                 tANI_U32 *pStatusChangeInfo, tANI_U16 infoLen)
{
    tSirMsgQ                  mmhMsg;
    tSirSmeWmStatusChangeNtf  *pSirSmeWmStatusChangeNtf;
    eHalStatus                status;
#if (WNI_POLARIS_FW_PACKAGE == ADVANCED) && (WNI_POLARIS_FW_PRODUCT == AP)
    tANI_U32                  bufLen;
    tANI_U16                  length=0;
    tANI_U8                   *pBuf;
#endif



    status = palAllocateMemory( pMac->hHdd, (void **)&pSirSmeWmStatusChangeNtf,
			                                        sizeof(tSirSmeWmStatusChangeNtf));
    if (status != eHAL_STATUS_SUCCESS)
    {
        limLog(pMac, LOGE,
          FL("call to palAllocateMemory failed for eWNI_SME_WM_STATUS_CHANGE_NTF, status = %d\n"),
          status);
          return;
    }

#if (WNI_POLARIS_FW_PACKAGE == ADVANCED) && (WNI_POLARIS_FW_PRODUCT == AP)
    pBuf = (tANI_U8 *)pSirSmeWmStatusChangeNtf;
#endif

    mmhMsg.type = eWNI_SME_WM_STATUS_CHANGE_NTF;
    mmhMsg.bodyval = 0;
    mmhMsg.bodyptr = pSirSmeWmStatusChangeNtf;

    switch(statusChangeCode)
    {
        case eSIR_SME_RADAR_DETECTED:

#if (WNI_POLARIS_FW_PACKAGE == ADVANCED) && (WNI_POLARIS_FW_PRODUCT == AP)
            bufLen = sizeof(tSirSmeWmStatusChangeNtf);
            if ((limSmeWmStatusChangeHeaderSerDes(pMac,
                                                  statusChangeCode,
                                          pBuf,
                                          &length,
                                          bufLen) != eSIR_SUCCESS))
            {
                palFreeMemory(pMac->hHdd, (void *) pSirSmeWmStatusChangeNtf);
                limLog(pMac, LOGP, FL("Header SerDes failed \n"));
                return;
            }
            pBuf += length;
            bufLen -= length;
            if ((limRadioInfoSerDes(pMac,
                                  (tpSirRadarInfo)pStatusChangeInfo,
                                  pBuf,
                                  &length,
                                  bufLen) != eSIR_SUCCESS))
            {
                palFreeMemory(pMac->hHdd, (void *) pSirSmeWmStatusChangeNtf);
                limLog(pMac, LOGP, FL("Radio Info SerDes failed \n"));
                return;
            }

            pBuf = (tANI_U8 *) pSirSmeWmStatusChangeNtf;
            pBuf += sizeof(tANI_U16);
            limCopyU16(pBuf, length);
#endif
            break;

        case eSIR_SME_CB_LEGACY_BSS_FOUND_BY_AP:
#if (WNI_POLARIS_FW_PACKAGE == ADVANCED) && (WNI_POLARIS_FW_PRODUCT == AP)

            if( eSIR_SUCCESS != nonTitanBssFoundSerDes( pMac,
                                (tpSirNeighborBssWdsInfo) pStatusChangeInfo,
                                pBuf,
                                &length ))
            {
                palFreeMemory(pMac->hHdd, (void *) pSirSmeWmStatusChangeNtf);
                limLog( pMac, LOGP,
                    FL("Unable to serialize nonTitanBssFoundSerDes!\n"));
                return;
            }
#endif
            break;

        case eSIR_SME_BACKGROUND_SCAN_FAIL:
            limPackBkgndScanFailNotify(pMac,
                                       statusChangeCode,
                                       (tpSirBackgroundScanInfo)pStatusChangeInfo,
                                       pSirSmeWmStatusChangeNtf);
            break;

        default:
#if defined (ANI_PRODUCT_TYPE_AP) && defined (ANI_LITTLE_BYTE_ENDIAN)
        sirStoreU16N((tANI_U8*)&pSirSmeWmStatusChangeNtf->messageType,
                    eWNI_SME_WM_STATUS_CHANGE_NTF );
        sirStoreU16N((tANI_U8*)&pSirSmeWmStatusChangeNtf->length,
                    (sizeof(tSirSmeWmStatusChangeNtf)));
        sirStoreU32N((tANI_U8*)&pSirSmeWmStatusChangeNtf->statusChangeCode,
                    statusChangeCode);
#else
        pSirSmeWmStatusChangeNtf->messageType = eWNI_SME_WM_STATUS_CHANGE_NTF;
        pSirSmeWmStatusChangeNtf->statusChangeCode = statusChangeCode;
        pSirSmeWmStatusChangeNtf->length = sizeof(tSirSmeWmStatusChangeNtf);
#endif
        if(sizeof(pSirSmeWmStatusChangeNtf->statusChangeInfo) >= infoLen)
        {
            palCopyMemory( pMac->hHdd, (tANI_U8 *)&pSirSmeWmStatusChangeNtf->statusChangeInfo, (tANI_U8 *)pStatusChangeInfo, infoLen);
        }
        limLog(pMac, LOGE, FL("***---*** StatusChg: code 0x%x, length %d ***---***\n"),
               statusChangeCode, infoLen);
        break;
    }

    MTRACE(macTraceMsgTx(pMac, 0, mmhMsg.type));
    if (eSIR_SUCCESS != limHalMmhPostMsgApi(pMac, &mmhMsg, ePROT))
    {
        palFreeMemory(pMac->hHdd, (void *) pSirSmeWmStatusChangeNtf);
        limLog( pMac, LOGP, FL("limHalMmhPostMsgApi failed\n"));
    }

} /*** end limSendSmeWmStatusChangeNtf() ***/


/**
 * limSendSmeSetContextRsp()
 *
 *FUNCTION:
 * This function is called by limProcessSmeMessages() to send
 * eWNI_SME_SETCONTEXT_RSP message to host
 *
 *PARAMS:
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 * NA
 *
 *NOTE:
 *
 * @param pMac         Pointer to Global MAC structure
 * @param peerMacAddr  Indicates the peer MAC addr to which
 *                     setContext was performed
 * @param aid          Indicates the aid corresponding to the peer MAC
 *                     address
 * @param resultCode   Indicates the result of previously issued
 *                     eWNI_SME_SETCONTEXT_RSP message
 *
 * @return None
 */
void
limSendSmeSetContextRsp(tpAniSirGlobal pMac,
                        tSirMacAddr peerMacAddr, tANI_U16 aid,
                        tSirResultCodes resultCode)
{
    tANI_U8                   *pBuf;
    tSirMsgQ             mmhMsg;
    tSirSmeSetContextRsp *pSirSmeSetContextRsp;

    if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pSirSmeSetContextRsp, sizeof(tSirSmeSetContextRsp)))
    {
        // Log error
        limLog(pMac, LOGP,
               FL("call to palAllocateMemory failed for SmeSetContextRsp\n"));

        return;
    }

#if defined (ANI_PRODUCT_TYPE_AP) && defined (ANI_LITTLE_BYTE_ENDIAN)
    sirStoreU16N((tANI_U8*)&pSirSmeSetContextRsp->messageType,
                 eWNI_SME_SETCONTEXT_RSP);
    sirStoreU16N((tANI_U8*)&pSirSmeSetContextRsp->length,
                 sizeof(tSirSmeSetContextRsp));
#else
    pSirSmeSetContextRsp->messageType = eWNI_SME_SETCONTEXT_RSP;
    pSirSmeSetContextRsp->length      = sizeof(tSirSmeSetContextRsp);
#endif
    pSirSmeSetContextRsp->statusCode  = resultCode;

    pBuf = pSirSmeSetContextRsp->peerMacAddr;

    palCopyMemory( pMac->hHdd, pBuf, (tANI_U8 *) peerMacAddr, sizeof(tSirMacAddr));
    pBuf += sizeof(tSirMacAddr);

    limCopyU32(pBuf, resultCode);
#if (WNI_POLARIS_FW_PRODUCT == AP)
    pBuf += sizeof(tSirResultCodes);

    limCopyU16(pBuf, aid);
    pBuf += sizeof(tANI_U16);
#endif

    mmhMsg.type = eWNI_SME_SETCONTEXT_RSP;
    mmhMsg.bodyptr = pSirSmeSetContextRsp;
    mmhMsg.bodyval = 0;
    MTRACE(macTraceMsgTx(pMac, 0, mmhMsg.type));

#ifdef FEATURE_WLAN_DIAG_SUPPORT 
    limDiagEventReport(pMac, WLAN_PE_DIAG_SETCONTEXT_RSP_EVENT, NULL, (tANI_U16)resultCode, 0);
#endif //FEATURE_WLAN_DIAG_SUPPORT
	
    limHalMmhPostMsgApi(pMac, &mmhMsg, ePROT);
} /*** end limSendSmeSetContextRsp() ***/

/**
 * limSendSmeRemoveKeyRsp()
 *
 *FUNCTION:
 * This function is called by limProcessSmeMessages() to send
 * eWNI_SME_REMOVEKEY_RSP message to host
 *
 *PARAMS:
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 * NA
 *
 *NOTE:
 *
 * @param pMac         Pointer to Global MAC structure
 * @param peerMacAddr  Indicates the peer MAC addr to which
 *                     Removekey was performed
 * @param aid          Indicates the aid corresponding to the peer MAC
 *                     address
 * @param resultCode   Indicates the result of previously issued
 *                     eWNI_SME_REMOVEKEY_RSP message
 *
 * @return None
 */
void
limSendSmeRemoveKeyRsp(tpAniSirGlobal pMac,
                        tSirMacAddr peerMacAddr,
                        tSirResultCodes resultCode)
{
    tANI_U8                   *pBuf;
    tSirMsgQ             mmhMsg;
    tSirSmeRemoveKeyRsp *pSirSmeRemoveKeyRsp;

    if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pSirSmeRemoveKeyRsp, sizeof(tSirSmeRemoveKeyRsp)))
    {
        // Log error
        limLog(pMac, LOGP,
               FL("call to palAllocateMemory failed for SmeRemoveKeyRsp\n"));

        return;
    }

#if defined (ANI_PRODUCT_TYPE_AP) && defined(ANI_LITTLE_BYTE_ENDIAN)
    sirStoreU16N((tANI_U8*)&pSirSmeRemoveKeyRsp->messageType,
                 eWNI_SME_REMOVEKEY_RSP);
    sirStoreU16N((tANI_U8*)&pSirSmeRemoveKeyRsp->length,
                 sizeof(tSirSmeRemoveKeyRsp));
#else
    pSirSmeRemoveKeyRsp->messageType = eWNI_SME_REMOVEKEY_RSP;
    pSirSmeRemoveKeyRsp->length      = sizeof(tSirSmeRemoveKeyRsp);
#endif
    pSirSmeRemoveKeyRsp->statusCode  = resultCode;

    pBuf = pSirSmeRemoveKeyRsp->peerMacAddr;

    palCopyMemory( pMac->hHdd, pBuf, (tANI_U8 *) peerMacAddr, sizeof(tSirMacAddr));
    pBuf += sizeof(tSirMacAddr);

    limCopyU32(pBuf, resultCode);

    mmhMsg.type = eWNI_SME_REMOVEKEY_RSP;
    mmhMsg.bodyptr = pSirSmeRemoveKeyRsp;
    mmhMsg.bodyval = 0;
    MTRACE(macTraceMsgTx(pMac, 0, mmhMsg.type));
    limHalMmhPostMsgApi(pMac, &mmhMsg, ePROT);
} /*** end limSendSmeSetContextRsp() ***/


/**
 * limSendSmePromiscuousModeRsp()
 *
 *FUNCTION:
 * This function is called by limProcessSmeMessages() to send
 * eWNI_PROMISCUOUS_MODE_RSP message to host
 *
 *PARAMS:
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 * NA
 *
 *NOTE:
 * This function is used for sending eWNI_SME_PROMISCUOUS_MODE_RSP to
 * host as a reply to eWNI_SME_PROMISCUOUS_MODE_REQ directive from it.
 *
 * @param None
 * @return None
 */
void
limSendSmePromiscuousModeRsp(tpAniSirGlobal pMac)
{
#if 0
    tSirMsgQ   mmhMsg;
    tSirMbMsg  *pMbMsg;

    if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pMbMsg, sizeof(tSirMbMsg)))
    {
        // Log error
        limLog(pMac, LOGP, FL("call to palAllocateMemory failed\n"));

        return;
    }

    pMbMsg->type   = eWNI_SME_PROMISCUOUS_MODE_RSP;
    pMbMsg->msgLen = 4;

    mmhMsg.type = eWNI_SME_PROMISCUOUS_MODE_RSP;
    mmhMsg.bodyptr = pMbMsg;
    mmhMsg.bodyval = 0;
    MTRACE(macTraceMsgTx(pMac, 0, mmhMsg.type));
    limHalMmhPostMsgApi(pMac, &mmhMsg, ePROT);
#endif
} /*** end limSendSmePromiscuousModeRsp() ***/



/**
 * limSendSmeNeighborBssInd()
 *
 *FUNCTION:
 * This function is called by limLookupNaddHashEntry() to send
 * eWNI_SME_NEIGHBOR_BSS_IND message to host
 *
 *PARAMS:
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 * NA
 *
 *NOTE:
 * This function is used for sending eWNI_SME_NEIGHBOR_BSS_IND to
 * host upon detecting new BSS during background scanning if CFG
 * option is enabled for sending such indication
 *
 * @param  pMac - Pointer to Global MAC structure
 * @return None
 */

void
limSendSmeNeighborBssInd(tpAniSirGlobal pMac,
                         tLimScanResultNode *pBssDescr)
{
    tSirMsgQ                 msgQ;
    tANI_U32                      val;
    tSirSmeNeighborBssInd    *pNewBssInd;

    if ((pMac->lim.gLimSmeState != eLIM_SME_LINK_EST_WT_SCAN_STATE) ||
        ((pMac->lim.gLimSmeState == eLIM_SME_LINK_EST_WT_SCAN_STATE) &&
         pMac->lim.gLimRspReqd))
    {
        // LIM is not in background scan state OR
        // current scan is initiated by HDD.
        // No need to send new BSS indication to HDD
        return;
    }

    if (wlan_cfgGetInt(pMac, WNI_CFG_NEW_BSS_FOUND_IND, &val) != eSIR_SUCCESS)
    {
        limLog(pMac, LOGP, FL("could not get NEIGHBOR_BSS_IND from CFG\n"));

        return;
    }

    if (val == 0)
        return;

    /**
     * Need to indicate new BSSs found during
     * background scanning to host.
     * Allocate buffer for sending indication.
     * Length of buffer is length of BSS description
     * and length of header itself
     */
    val = pBssDescr->bssDescription.length + sizeof(tANI_U16) + sizeof(tANI_U32);
    if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pNewBssInd, val))
    {
        // Log error
        limLog(pMac, LOGP,
           FL("call to palAllocateMemory failed for eWNI_SME_NEIGHBOR_BSS_IND\n"));

        return;
    }

#if defined (ANI_PRODUCT_TYPE_AP) && defined (ANI_LITTLE_BYTE_ENDIAN)
    sirStoreU16N((tANI_U8*) &pNewBssInd->messageType,
                 eWNI_SME_NEIGHBOR_BSS_IND);
    sirStoreU16N((tANI_U8*)&pNewBssInd->length, (tANI_U16)val );
#else
    pNewBssInd->messageType = eWNI_SME_NEIGHBOR_BSS_IND;
    pNewBssInd->length      = (tANI_U16) val;
#endif

#if (WNI_POLARIS_FW_PRODUCT == WLAN_STA)
    palCopyMemory( pMac->hHdd, (tANI_U8 *) pNewBssInd->bssDescription,
                  (tANI_U8 *) &pBssDescr->bssDescription,
                  pBssDescr->bssDescription.length + sizeof(tANI_U16));
#endif

    msgQ.type = eWNI_SME_NEIGHBOR_BSS_IND;
    msgQ.bodyptr = pNewBssInd;
    msgQ.bodyval = 0;
    MTRACE(macTraceMsgTx(pMac, 0, msgQ.type));
    limHalMmhPostMsgApi(pMac, &msgQ, ePROT);
} /*** end limSendSmeNeighborBssInd() ***/

/** -----------------------------------------------------------------
  \brief limSendSmeAddtsRsp() - sends SME ADDTS RSP    
  \      This function sends a eWNI_SME_ADDTS_RSP to SME.   
  \      SME only looks at rc and tspec field. 
  \param pMac - global mac structure
  \param rspReqd - is SmeAddTsRsp required
  \param status - status code of SME_ADD_TS_RSP
  \return tspec
  \sa
  ----------------------------------------------------------------- */
void
limSendSmeAddtsRsp(tpAniSirGlobal pMac, tANI_U8 rspReqd, tANI_U32 status, tSirMacTspecIE tspec)
{
    tpSirAddtsRsp  rsp;
    tSirMsgQ  mmhMsg;

    if (! rspReqd)
        return;

    if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&rsp, sizeof(tSirAddtsRsp)))
    {
        limLog(pMac, LOGP, FL("palAllocateMemory failed for ADDTS_RSP"));
        return;
    }

    palZeroMemory( pMac->hHdd, (tANI_U8 *) rsp, sizeof(*rsp));
    rsp->messageType = eWNI_SME_ADDTS_RSP;
    rsp->rc = status;
    rsp->rsp.status = (enum eSirMacStatusCodes) status;
    //palCopyMemory( pMac->hHdd, (tANI_U8 *) &rsp->rsp.tspec, (tANI_U8 *) &addts->tspec, sizeof(addts->tspec));  
    rsp->rsp.tspec = tspec;

    mmhMsg.type = eWNI_SME_ADDTS_RSP;
    mmhMsg.bodyptr = rsp;
    mmhMsg.bodyval = 0;
    MTRACE(macTraceMsgTx(pMac, 0, mmhMsg.type));
#ifdef FEATURE_WLAN_DIAG_SUPPORT 
    limDiagEventReport(pMac, WLAN_PE_DIAG_ADDTS_RSP_EVENT, NULL, 0, 0);
#endif //FEATURE_WLAN_DIAG_SUPPORT
	
    limHalMmhPostMsgApi(pMac, &mmhMsg, ePROT);
    return;
}

void
limSendSmeAddtsInd(tpAniSirGlobal pMac, tpSirAddtsReqInfo addts)
{
    tpSirAddtsRsp rsp;
    tSirMsgQ      mmhMsg;

    limLog(pMac, LOGW, "SendSmeAddtsInd (token %d, tsid %d, up %d)\n",
           addts->dialogToken,
           addts->tspec.tsinfo.traffic.tsid,
           addts->tspec.tsinfo.traffic.userPrio);

    if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&rsp, sizeof(tSirAddtsRsp)))
    {
        // Log error
        limLog(pMac, LOGP, FL("palAllocateMemory failed for ADDTS_IND\n"));
        return;
    }
    palZeroMemory( pMac->hHdd, (tANI_U8 *) rsp, sizeof(*rsp));

    rsp->messageType     = eWNI_SME_ADDTS_IND;

    palCopyMemory( pMac->hHdd, (tANI_U8 *) &rsp->rsp, (tANI_U8 *) addts, sizeof(*addts));

    mmhMsg.type = eWNI_SME_ADDTS_IND;
    mmhMsg.bodyptr = rsp;
    mmhMsg.bodyval = 0;
    MTRACE(macTraceMsgTx(pMac, 0, mmhMsg.type));
    limHalMmhPostMsgApi(pMac, &mmhMsg, ePROT);
}

void
limSendSmeDeltsRsp(tpAniSirGlobal pMac, tpSirDeltsReq delts, tANI_U32 status)
{
    tpSirDeltsRsp rsp;
    tSirMsgQ      mmhMsg;

    limLog(pMac, LOGW, "SendSmeDeltsRsp (aid %d, tsid %d, up %d) status %d\n",
           delts->aid,
           delts->req.tsinfo.traffic.tsid,
           delts->req.tsinfo.traffic.userPrio,
           status);
    if (! delts->rspReqd)
        return;

    if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&rsp, sizeof(tSirDeltsRsp)))
    {
        // Log error
        limLog(pMac, LOGP, FL("palAllocateMemory failed for DELTS_RSP\n"));
        return;
    }
    palZeroMemory( pMac->hHdd, (tANI_U8 *) rsp, sizeof(*rsp));

    rsp->messageType     = eWNI_SME_DELTS_RSP;
    rsp->rc              = status;
    rsp->aid             = delts->aid;
    palCopyMemory( pMac->hHdd, (tANI_U8 *) &rsp->macAddr[0], (tANI_U8 *) &delts->macAddr[0], 6);
    palCopyMemory( pMac->hHdd, (tANI_U8 *) &rsp->rsp, (tANI_U8 *) &delts->req, sizeof(tSirDeltsReqInfo));

    mmhMsg.type = eWNI_SME_DELTS_RSP;
    mmhMsg.bodyptr = rsp;
    mmhMsg.bodyval = 0;
    MTRACE(macTraceMsgTx(pMac, 0, mmhMsg.type));
#ifdef FEATURE_WLAN_DIAG_SUPPORT 
    limDiagEventReport(pMac, WLAN_PE_DIAG_DELTS_RSP_EVENT, NULL, (tANI_U16)status, 0);
#endif //FEATURE_WLAN_DIAG_SUPPORT
	
    limHalMmhPostMsgApi(pMac, &mmhMsg, ePROT);
}

void
limSendSmeDeltsInd(tpAniSirGlobal pMac, tpSirDeltsReqInfo delts, tANI_U16 aid)
{
    tpSirDeltsRsp rsp;
    tSirMsgQ      mmhMsg;

    limLog(pMac, LOGW, "SendSmeDeltsInd (aid %d, tsid %d, up %d)\n",
           aid,
           delts->tsinfo.traffic.tsid,
           delts->tsinfo.traffic.userPrio);

    if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&rsp, sizeof(tSirDeltsRsp)))
    {
        // Log error
        limLog(pMac, LOGP, FL("palAllocateMemory failed for DELTS_IND\n"));
        return;
    }
    palZeroMemory( pMac->hHdd, (tANI_U8 *) rsp, sizeof(*rsp));

    rsp->messageType     = eWNI_SME_DELTS_IND;
    rsp->rc              = eSIR_SUCCESS;
    rsp->aid             = aid;
    palCopyMemory( pMac->hHdd, (tANI_U8 *) &rsp->rsp, (tANI_U8 *) delts, sizeof(*delts));

    mmhMsg.type = eWNI_SME_DELTS_IND;
    mmhMsg.bodyptr = rsp;
    mmhMsg.bodyval = 0;
    MTRACE(macTraceMsgTx(pMac, 0, mmhMsg.type));
#ifdef FEATURE_WLAN_DIAG_SUPPORT 
    limDiagEventReport(pMac, WLAN_PE_DIAG_DELTS_IND_EVENT, NULL, 0, 0);
#endif //FEATURE_WLAN_DIAG_SUPPORT

    limHalMmhPostMsgApi(pMac, &mmhMsg, ePROT);
}

/**
 * limSendSmeStatsRsp()
 *
 *FUNCTION:
 * This function is called to send 802.11 statistics response to HDD.
 * This function posts the result back to HDD. This is a response to
 * HDD's request for statistics.
 *
 *PARAMS:
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 * NA
 *
 *NOTE:
 * NA
 *
 * @param pMac         Pointer to Global MAC structure
 * @param p80211Stats  Statistics sent in response 
 * @param resultCode   TODO:
 * 
 *
 * @return none
 */

void
limSendSmeStatsRsp(tpAniSirGlobal pMac, tANI_U16 msgType, void* stats)
{
    tSirMsgQ              mmhMsg;
    tSirSmeRsp           *pMsgHdr = (tSirSmeRsp*) stats;

    switch(msgType)
    {
        case SIR_HAL_STA_STAT_RSP:
            mmhMsg.type = eWNI_SME_STA_STAT_RSP;
            break;
        case SIR_HAL_AGGR_STAT_RSP:
            mmhMsg.type = eWNI_SME_AGGR_STAT_RSP;
            break;
        case SIR_HAL_GLOBAL_STAT_RSP:
            mmhMsg.type = eWNI_SME_GLOBAL_STAT_RSP;
            break;
        case SIR_HAL_STAT_SUMM_RSP:
            mmhMsg.type = eWNI_SME_STAT_SUMM_RSP;
            break;		
        default:
            mmhMsg.type = msgType; //Response from within PE
            break;
    }

    pMsgHdr->messageType = mmhMsg.type; 

    mmhMsg.bodyptr = stats;
    mmhMsg.bodyval = 0;
    MTRACE(macTraceMsgTx(pMac, 0, mmhMsg.type));
    limHalMmhPostMsgApi(pMac, &mmhMsg, ePROT);                                                	

    return;

} /*** end limSendSmeStatsRsp() ***/

/**
 * limSendSmePEStatisticsRsp()
 *
 *FUNCTION:
 * This function is called to send 802.11 statistics response to HDD.
 * This function posts the result back to HDD. This is a response to
 * HDD's request for statistics.
 *
 *PARAMS:
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 * NA
 *
 *NOTE:
 * NA
 *
 * @param pMac         Pointer to Global MAC structure
 * @param p80211Stats  Statistics sent in response 
 * @param resultCode   TODO:
 * 
 *
 * @return none
 */

void
limSendSmePEStatisticsRsp(tpAniSirGlobal pMac, tANI_U16 msgType, void* stats)
{
    tSirMsgQ              mmhMsg;
    tSirSmeRsp           *pMsgHdr = (tSirSmeRsp*) stats;

    //msgType should be SIR_HAL_GET_STATISTICS_RSP
    mmhMsg.type = eWNI_SME_GET_STATISTICS_RSP;
    pMsgHdr->messageType = mmhMsg.type; 

    mmhMsg.bodyptr = stats;
    mmhMsg.bodyval = 0;
    MTRACE(macTraceMsgTx(pMac, 0, mmhMsg.type));
    limHalMmhPostMsgApi(pMac, &mmhMsg, ePROT);                                                  

    return;

} /*** end limSendSmePEStatisticsRsp() ***/


void
limSendSmeIBSSPeerInd(
    tpAniSirGlobal      pMac,
    tSirMacAddr peerMacAddr,
    tANI_U16    staIndex, tANI_U8  *beacon, 
    tANI_U16 beaconLen, tANI_U16 msgType)
{
    tSirMsgQ                  mmhMsg;
    tSmeIbssPeerInd *pNewPeerInd;
    
    if(eSIR_SUCCESS !=
        palAllocateMemory(pMac->hHdd,(void * *) &pNewPeerInd,(sizeof(tSmeIbssPeerInd) + beaconLen)))
    {
        PELOGE(limLog(pMac, LOGE, FL("Failed to allocate memory"));)
        return;
    }
    
    palZeroMemory(pMac->hHdd, (void *) pNewPeerInd, (sizeof(tSmeIbssPeerInd) + beaconLen));

    palCopyMemory( pMac->hHdd, (tANI_U8 *) pNewPeerInd->peerAddr,
                   peerMacAddr, sizeof(tSirMacAddr));
    pNewPeerInd->staId= staIndex;
    pNewPeerInd->mesgLen = sizeof(tSmeIbssPeerInd) + beaconLen;
    pNewPeerInd->mesgType = msgType;

    if ( beacon != NULL )
    {
        palCopyMemory(pMac->hHdd, (void*) ((tANI_U8*)pNewPeerInd+sizeof(tSmeIbssPeerInd)), (void*)beacon, beaconLen);
    }

    mmhMsg.type    = msgType;
//    mmhMsg.bodyval = (tANI_U32) pNewPeerInd;
    mmhMsg.bodyptr = pNewPeerInd;
    MTRACE(macTraceMsgTx(pMac, 0, mmhMsg.type));
    limHalMmhPostMsgApi(pMac, &mmhMsg, ePROT);
    
}


/** -----------------------------------------------------------------
  \brief limSendExitBmpsInd() - sends exit bmps indication
   
  This function sends a eWNI_PMC_EXIT_BMPS_IND with a specific reason
  code to SME. This will trigger SME to get out of BMPS mode. 
    
  \param pMac - global mac structure
  \param reasonCode - reason for which PE wish to exit BMPS
  \return none 
  \sa
  ----------------------------------------------------------------- */
void limSendExitBmpsInd(tpAniSirGlobal pMac, tExitBmpsReason reasonCode)
{
    tSirMsgQ  mmhMsg;
    tANI_U16  msgLen = 0;
    tpSirSmeExitBmpsInd  pExitBmpsInd;
 
    msgLen = sizeof(tSirSmeExitBmpsInd);
    if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pExitBmpsInd, msgLen ))
    {
        limLog(pMac, LOGP, FL("palAllocateMemory failed for PMC_EXIT_BMPS_IND \n"));
        return;
    }
    palZeroMemory(pMac->hHdd, pExitBmpsInd, msgLen);

#if defined (ANI_PRODUCT_TYPE_AP) && defined (ANI_LITTLE_BYTE_ENDIAN)
    sirStoreU16N((tANI_U8*)&pExitBmpsInd->mesgType, eWNI_PMC_EXIT_BMPS_IND);
    sirStoreU16N((tANI_U8*)&pExitBmpsInd->mesgLen, msgLen);
#else
    pExitBmpsInd->mesgType = eWNI_PMC_EXIT_BMPS_IND;
    pExitBmpsInd->mesgLen = msgLen;
#endif
    pExitBmpsInd->exitBmpsReason = reasonCode;
    pExitBmpsInd->statusCode = eSIR_SME_SUCCESS;

    mmhMsg.type = eWNI_PMC_EXIT_BMPS_IND;
    mmhMsg.bodyptr = pExitBmpsInd;
    mmhMsg.bodyval = 0;
  
    PELOG1(limLog(pMac, LOG1, FL("Sending eWNI_PMC_EXIT_BMPS_IND to SME. \n"));)		
    MTRACE(macTraceMsgTx(pMac, 0, mmhMsg.type));
#ifdef FEATURE_WLAN_DIAG_SUPPORT 
    limDiagEventReport(pMac, WLAN_PE_DIAG_EXIT_BMPS_IND_EVENT, NULL, 0, (tANI_U16)reasonCode);
#endif //FEATURE_WLAN_DIAG_SUPPORT
	
    limHalMmhPostMsgApi(pMac, &mmhMsg,  ePROT);
    return;

} /*** end limSendExitBmpsInd() ***/






/*
 * Airgo Networks, Inc proprietary. All rights reserved.
 * This file contains the source code for composing and sending messages
 * to host.
 *
 * Author:      Kevin Nguyen
 * Date:        04/09/02
 * History:-
 * 04/09/02     Created.
 * --------------------------------------------------------------------
 */
#include "palTypes.h"
#include "cfgPriv.h"
#include "limTrace.h"
#include "cfgDebug.h"

extern void SysProcessMmhMsg(tpAniSirGlobal pMac, tSirMsgQ* pMsg);

/*--------------------------------------------------------------------*/
/* ATTENTION:  The functions contained in this module are to be used  */
/*             by CFG module ONLY.                                    */
/*--------------------------------------------------------------------*/


/**---------------------------------------------------------------------
 * cfgSendHostMsg()
 *
 * FUNCTION:
 * Send CNF/RSP to host.
 *
 * LOGIC:
 * Please see Configuration & Statistic Collection Micro-Architecture
 * specification for details.
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param msgType:     message type
 * @param msgLen:      message length
 * @param paramNum:    number of parameters
 * @param pParamList:  pointer to parameter list
 * @param dataLen:     data length
 * @param pData:       pointer to additional data
 *
 * @return None.
 *
 */
void
cfgSendHostMsg(tpAniSirGlobal pMac, tANI_U16 msgType, tANI_U32 msgLen, tANI_U32 paramNum, tANI_U32 *pParamList,
              tANI_U32 dataLen, tANI_U32 *pData)
{
    tANI_U32        *pMsg, *pEnd;
    tSirMsgQ    mmhMsg;

    // Allocate message buffer
    if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pMsg, msgLen))
    {
        return;
    }

    // Fill in message details
    mmhMsg.type = msgType;
    mmhMsg.bodyptr = pMsg;
    mmhMsg.bodyval = 0;
#if defined (ANI_PRODUCT_TYPE_AP) && defined (ANI_LITTLE_BYTE_ENDIAN)
    sirStoreU16N((tANI_U8*)pMsg, (tANI_U16)msgType);
    sirStoreU16N(((tANI_U8*)pMsg+2), (tANI_U16)msgLen);
#else
    ((tSirMbMsg*)pMsg)->type   = msgType;
    ((tSirMbMsg*)pMsg)->msgLen = (tANI_U16)msgLen;
#endif

   if ( paramNum > 0 && (pParamList == NULL))
   {
       PELOGE(cfgLog(pMac, LOGE, FL("pParamList cannot be NULL for paramNum gearter than 0!\n"));)
       palFreeMemory( pMac->hHdd, pMsg);
       return;    
   }

    switch (msgType)
    {
        case WNI_CFG_GET_RSP:
        case WNI_CFG_PARAM_UPDATE_IND:
        case WNI_CFG_DNLD_REQ:
        case WNI_CFG_DNLD_CNF:
        case WNI_CFG_SET_CNF:
            // Fill in parameters
            pMsg++;
            pEnd  = pMsg + paramNum;
            while (pMsg < pEnd)
                *pMsg++ = *pParamList++;

            // Copy data if there is any
            pEnd = pMsg + (dataLen >> 2);
            while (pMsg < pEnd)
                *pMsg++ = *pData++;

            break;

        default:
            palFreeMemory( pMac->hHdd, pMsg);
            return;
    }

    // Ship it
    MTRACE(macTraceMsgTx(pMac, 0, mmhMsg.type));
    SysProcessMmhMsg(pMac, &mmhMsg);

} /*** end cfgSendHostMsg() ***/





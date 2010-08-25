/** ------------------------------------------------------------------------- *
    ------------------------------------------------------------------------- *


    \file dvtMsgApi.c

    \brief Simply unpacks the message from the union and calls the appropriate functions with the parameters.
            Returns the response.

    $Id$

    Copyright (C) 2006 Airgo Networks, Incorporated


   ========================================================================== */

#ifdef ANI_DVT_DEBUG
#include "ani_assert.h"

#include "aniGlobal.h"

#include "dvtModuleApi.h"
#include "dvtMsgApi.h"

#include "utilsApi.h"
#include "halCommonApi.h"
#include "string.h"

// dvt msg buffer to convert the hal msg to dvt msg
tDvtMsgbuffer hDvtMsg;

// holds msg id to avoid confusion in the case, hal sends response back to one of the redundant dvtMsgIds
tANI_U16 dvtMsgId;

//This is invoked from HDD (mbSendMBMsg in hddMB.c)
tSirRetStatus dvtMmhForwardMBmsg(void* pSirGlobal, tSirMbMsg* pMb)
{
    tSirMsgQ msg;
    tSirRetStatus rc = eSIR_SUCCESS;
    tpAniSirGlobal pMac = (tpAniSirGlobal)pSirGlobal;
#if (WNI_POLARIS_FW_OS == SIR_WINDOWS)
    tSirMbMsg* pMbLocal;
    tANI_U16 msgWordLen;
#endif

    //halLog(pMac, LOG3, "dvtMmhForwardMBmsg: msgType %d, msgLen %d, first Word %d\n", pMb->type, pMb->msgLen, pMb->data[0]);

    msg.type = pMb->type;
    msg.bodyptr = pMb;
    msg.bodyval = 0;
	
#if (WNI_POLARIS_FW_OS == SIR_WINDOWS)

    // copy the message from host buffer to firmware buffer
    // this will make sure that firmware allocates, uses and frees
    // it's own buffers for mailbox message instead of working on
    // host buffer

    if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (tSirMbMsg **)&pMbLocal, pMb->msgLen))
    {
        //halLog(pMac, LOGP, "dvtMmhForwardMBmsg: Buffer Allocation failed!\n");
        return eSIR_FAILURE;
    }

    msgWordLen = (pMb->msgLen)/ 4;

    if (pMb->msgLen % 4)
        msgWordLen++;

    memcpy((tANI_U8 *)pMbLocal, (tANI_U8 *)pMb, (msgWordLen<<2));

    msg.bodyptr = pMbLocal;
    msg.bodyval = 0;
#endif

    // Posts a message to the NIM PTT MsgQ
    do
    {
#if (WNI_POLARIS_FW_OS != SIR_WINDOWS)

        // Posts message to the queue
        if (tx_queue_send(&pMac->sys.gSirHalMsgQ, pMsg, TX_NO_WAIT) != TX_SUCCESS)
        {
            rc = eSIR_FAILURE;
            //halLog(pMac, LOGP, "dvtPostMsgApi: Posting a Msg to halMsgQ failed!\n");
            break;
        }
#else
        // For Windows based MAC, instead of posting message to different
        // queues, we will call the handler routines directly
        if (dvtReceiveMsg(pMac, &msg ) != eSIR_SUCCESS)
        {
            rc = eSIR_FAILURE;
            //halLog(pMac, LOGP, "dvtMmhForwardMBmsg: dvtReceiveMsg() failed!\n");
            break;
        }
#endif
    }
    while (0);

    return rc;

} // dvtMmhForwardMBmsg()


tSirRetStatus dvtMmhPostMsgApi(tpAniSirGlobal pMac, tSirMsgQ *pMsg)
{
    tSirRetStatus rc = eSIR_SUCCESS;

    do
    {
#if (WNI_POLARIS_FW_OS == SIR_WINDOWS)
        // directly call the host module which handles mailbox receive
        mbReceiveMBMsg( pMac->pAdapter, (tSirMbMsg *)pMsg->bodyptr);

        if (eHAL_STATUS_SUCCESS != palFreeMemory( pMac->hHdd, (tANI_U8*)pMsg->bodyptr) )
            rc = eSIR_FAILURE;

        rc = eSIR_SUCCESS;
        break;
#else
        // RTAI
        if (tx_queue_send(&pMac->sys.gSirRxMsgQ, (void*)&pMsg->bodyptr,
                          TX_NO_WAIT) != TX_SUCCESS)
        {
            //halLog(pMac, LOGP, "dvtMmhPostMsgApi: Queue send Failed!\n");
            rc = eSIR_SYS_TX_Q_SEND_FAILED;
            break;
        }
        else
            tx_send_hdd_srq(pMac->rt);

#endif  // == SIR_WINDOWS
    }
    while (0);

    return rc;

} // dvtMmhPostMsgApi



void dvtSendMsgResponse(tpAniSirGlobal pMac, tDvtMsgbuffer *pDvtMsg)
{
/*
    This needs to be re-implemented to use the new Pal abstractions for Nova
*/

     tSirMsgQ mmhMsg;
     tDvtMsgbuffer *pRsp;
     tANI_U8 *pTemp;
     tANI_U32 *pBody, *pRspBody;
     tANI_U32 bodyLen;

    if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (tANI_U8 **)&pTemp, pDvtMsg->msgBodyLength))
        return;

     pRsp = (tDvtMsgbuffer *)pTemp;

     //save return code and data
 #if ((WNI_POLARIS_FW_OS == SIR_RTAI) && defined(ANI_LITTLE_BYTE_ENDIAN))
     sirStoreU16N((tANI_U8 *)&(pRsp->msgId), (tANI_U16)pDvtMsg->msgId);
     sirStoreU16N((tANI_U8 *)&(pRsp->msgBodyLength), (tANI_U16)pDvtMsg->msgBodyLength);
     sirStoreU32N((tANI_U8 *)&(pRsp->msgResponse), pDvtMsg->msgResponse);
 #else
     pRsp->msgId = (tANI_U16)pDvtMsg->msgId;
     pRsp->msgBodyLength = (tANI_U16)pDvtMsg->msgBodyLength;
     pRsp->msgResponse = pDvtMsg->msgResponse;
 #endif

     pBody = (tANI_U32 *)&(pDvtMsg->msgBody);
     pRspBody = (tANI_U32 *)&(pRsp->msgBody);
     bodyLen = (pDvtMsg->msgBodyLength-sizeof(pDvtMsg->msgId)-sizeof(pDvtMsg->msgBodyLength)-sizeof(pDvtMsg->msgResponse));
     memcpy((tANI_U8 *)pRspBody, (tANI_U8 *)pBody, bodyLen);

     mmhMsg.type = (tANI_U16)pDvtMsg->msgId;
     mmhMsg.bodyptr = pTemp;
     mmhMsg.bodyval = 0;

     dvtMmhPostMsgApi(pMac, &mmhMsg);
}


tSirRetStatus dvtHalReceiveMsg(tpAniSirGlobal pMac, tSirMsgQ *pMsg, tDvtMsgbuffer *hDvtMsg)
{
    switch(pMsg->type)
    {
        case SIR_HAL_GET_DPUINFO_RSP:
        {
            tGetDpuParams *hDpuParams;
            hDpuParams = (tGetDpuParams *)pMsg->bodyptr;

            hDvtMsg->msgId = DVT_MSG_GET_DPU_SIGNATURE;
            hDvtMsg->msgBodyLength = sizeof(tANI_U16) + sizeof(tANI_U16) + sizeof(eANI_DVT_STATUS) + sizeof(tDvtMsgGetDpuSignature);
            hDvtMsg->msgResponse = hDpuParams->status;
            hDvtMsg->msgBody.GetDpuSignature.staIdx = hDpuParams->staIdx;
            hDvtMsg->msgBody.GetDpuSignature.dpuDescIndx = hDpuParams->dpuDescIndx;
            hDvtMsg->msgBody.GetDpuSignature.dpuSignature = hDpuParams->dpuSignature;

            break;
        }

        case SIR_HAL_ADD_BSS_RSP:
        {
            tAddBssParams *hBssParams;
            hBssParams = (tAddBssParams *)pMsg->bodyptr;
            hDvtMsg->msgResponse = hBssParams->status;
            if(dvtMsgId == DVT_MSG_SIMPLE_BSS_CONFIG)
            {
                hDvtMsg->msgId = DVT_MSG_SIMPLE_BSS_CONFIG;
                hDvtMsg->msgBodyLength = sizeof(tANI_U16) + sizeof(tANI_U16) + sizeof(eANI_DVT_STATUS) + sizeof(tDvtMsgSimpleBssConfig);

                hDvtMsg->msgBody.SimpleBssConfig.bss.bssId[0] = hBssParams->bssId[0];
                hDvtMsg->msgBody.SimpleBssConfig.bss.bssId[1] = hBssParams->bssId[1];
                hDvtMsg->msgBody.SimpleBssConfig.bss.bssId[2] = hBssParams->bssId[2];
                hDvtMsg->msgBody.SimpleBssConfig.bss.bssId[3] = hBssParams->bssId[3];
                hDvtMsg->msgBody.SimpleBssConfig.bss.bssId[4] = hBssParams->bssId[4];
                hDvtMsg->msgBody.SimpleBssConfig.bss.bssId[5] = hBssParams->bssId[5];
                hDvtMsg->msgBody.SimpleBssConfig.bss.configured = eANI_BOOLEAN_TRUE;
                hDvtMsg->msgBody.SimpleBssConfig.bss.bssType = hBssParams->bssType;
                hDvtMsg->msgBody.SimpleBssConfig.bssIndex = hBssParams->bssIdx;
            }
            else
            {
                hDvtMsg->msgId = DVT_MSG_BSS_CONFIG;
                hDvtMsg->msgBodyLength = sizeof(tANI_U16) + sizeof(tANI_U16) + sizeof(eANI_DVT_STATUS) + sizeof(tDvtMsgBssConfig);

                palCopyMemory( pMac->hHdd, (void *)&(hDvtMsg->msgBody.BssConfig), (void *)(hBssParams), sizeof(tDvtMsgBssConfig));
            }
            break;
        }

        case SIR_HAL_ADD_STA_RSP:
        {
            tAddStaParams *hStaParams;
            hStaParams = (tAddStaParams *)pMsg->bodyptr;
            hDvtMsg->msgResponse = hStaParams->status;
            if(dvtMsgId == DVT_MSG_ADD_SIMPLE_STATION)
            {
                hDvtMsg->msgId = DVT_MSG_ADD_SIMPLE_STATION;
                hDvtMsg->msgBodyLength = sizeof(tANI_U16) + sizeof(tANI_U16) + sizeof(eANI_DVT_STATUS) + sizeof(tDvtMsgAddSimpleStation);

                hDvtMsg->msgBody.AddSimpleStation.sta.macAddr[0] = hStaParams->staMac[0];
                hDvtMsg->msgBody.AddSimpleStation.sta.macAddr[1] = hStaParams->staMac[1];
                hDvtMsg->msgBody.AddSimpleStation.sta.macAddr[2] = hStaParams->staMac[2];
                hDvtMsg->msgBody.AddSimpleStation.sta.macAddr[3] = hStaParams->staMac[3];
                hDvtMsg->msgBody.AddSimpleStation.sta.macAddr[4] = hStaParams->staMac[4];
                hDvtMsg->msgBody.AddSimpleStation.sta.macAddr[5] = hStaParams->staMac[5];
                hDvtMsg->msgBody.AddSimpleStation.sta.configured = eANI_BOOLEAN_TRUE;
                hDvtMsg->msgBody.AddSimpleStation.stationIndex = hStaParams->staIdx;
            }
            else
            {
                hDvtMsg->msgId = DVT_MSG_ADD_STATION;
                hDvtMsg->msgBodyLength = sizeof(tANI_U16) + sizeof(tANI_U16) + sizeof(eANI_DVT_STATUS) + sizeof(tDvtMsgAddStation);

                palCopyMemory( pMac->hHdd, (void *)&(hDvtMsg->msgBody.AddStation), (void *)(hStaParams), sizeof(tDvtMsgAddStation));
            }

            break;
        }

        case SIR_HAL_SET_STAKEY_RSP:
        {
            tSetStaKeyParams *hStaKeyParams;
            hStaKeyParams = (tSetStaKeyParams *)pMsg->bodyptr;

            hDvtMsg->msgId = DVT_MSG_SET_STA_KEY;
            hDvtMsg->msgBodyLength = sizeof(tANI_U16) + sizeof(tANI_U16) + sizeof(eANI_DVT_STATUS) + sizeof(tSetStaKeyParams);
            hDvtMsg->msgResponse = hStaKeyParams->status;

            palCopyMemory( pMac->hHdd, (void *)&(hDvtMsg->msgBody.SetStaKeyParams), (void *)(hStaKeyParams), sizeof(tSetStaKeyParams));

            break;
        }

        case SIR_HAL_SET_BSSKEY_RSP:
        {
            tSetBssKeyParams *hBssKeyParams;
            hBssKeyParams = (tSetBssKeyParams *)pMsg->bodyptr;

            hDvtMsg->msgId = DVT_MSG_SET_BSS_KEY;
            hDvtMsg->msgBodyLength = sizeof(tANI_U16) + sizeof(tANI_U16) + sizeof(eANI_DVT_STATUS) + sizeof(tSetBssKeyParams);
            hDvtMsg->msgResponse = hBssKeyParams->status;

            palCopyMemory( pMac->hHdd, (void *)&(hDvtMsg->msgBody.SetBssKeyParams), (void *)(hBssKeyParams), sizeof(tSetBssKeyParams));

            break;
        }

        case SIR_HAL_DELETE_STA_RSP:
        {
            tDeleteStaParams *hDeleteStaParams;
            hDeleteStaParams = (tDeleteStaParams *)pMsg->bodyptr;

            hDvtMsg->msgId = DVT_MSG_DELETE_STATION;
            hDvtMsg->msgBodyLength = sizeof(tANI_U16) + sizeof(tANI_U16) + sizeof(eANI_DVT_STATUS) + sizeof(tDvtMsgDeleteStation);
            hDvtMsg->msgResponse = hDeleteStaParams->status;
            hDvtMsg->msgBody.DeleteStation.stationIndex = hDeleteStaParams->staIdx;

            break;
        }
        default:
            break;
    }
    return eSIR_SUCCESS;
}

tSirRetStatus dvtReceiveMsg(tpAniSirGlobal pMac, tSirMsgQ *pMsg)
{
    tDvtMsgbuffer *pDvtMsg;
    tANI_U16 msgLength;
    eDvtMsgId msgId = 0;

    if((pMsg->type >= SIR_HAL_MSG_TYPES_BEGIN) && (pMsg->type <= SIR_HAL_MSG_TYPES_END))
    {
        dvtHalReceiveMsg(pMac, pMsg, &hDvtMsg);
        pDvtMsg = &hDvtMsg;
    }
    else
    {
        pDvtMsg   = (tDvtMsgbuffer *)(pMsg->bodyptr);
        msgId = (eDvtMsgId)(pDvtMsg->msgId);
        dvtMsgId = pDvtMsg->msgId;

        pDvtMsg->msgResponse = dvtProcessMsg(pMac, pDvtMsg);
    }

    if (msgId == DVT_MSG_WRITE_PIPE_FRAME_TRANSFER)
    {
        //save pMsg to dvtMsg and call it later when tx completes
        pDvtMsg->msgBodyLength = sizeof(tANI_U16) + sizeof(tANI_U16) + sizeof(eANI_DVT_STATUS);
        memcpy(&(pMac->dvt.dvtMsg), pMsg, sizeof(tSirMsgQ));
    }
    else if(msgId == DVT_MSG_GET_DPU_SIGNATURE)
    {
        //donot send response, as we are sending for SIR_HAL_GET_DPUINFO_RSP
    }
    else if(msgId == DVT_MSG_SIMPLE_BSS_CONFIG)
    {
        //donot send response, as we are sending for SIR_HAL_ADD_BSS_RSP
    }
    else if(msgId == DVT_MSG_ADD_SIMPLE_STATION)
    {
        //donot send response, as we are sending for SIR_HAL_ADD_STA_RSP
    }
    else if(msgId == DVT_MSG_SET_STA_KEY)
    {
        //donot send response, as we are sending for SIR_HAL_SET_STAKEY_RSP
    }
    else if(msgId == DVT_MSG_SET_BSS_KEY)
    {
        //donot send response, as we are sending for SIR_HAL_SET_BSSKEY_RSP
    }
    else if(msgId == DVT_MSG_BSS_CONFIG)
    {
        //donot send response, as we are sending for SIR_HAL_ADD_BSS_RSP
    }
    else if(msgId == DVT_MSG_ADD_STATION)
    {
        //donot send response, as we are sending for SIR_HAL_ADD_STA_RSP
    }
    else if(msgId == DVT_MSG_DELETE_STATION)
    {
        //donot send response, as we are sending for SIR_HAL_DELETE_STA_RSP
    }
    else
    {
        dvtSendMsgResponse(pMac, pDvtMsg);
        // donot free memory, if it is a hal response msg
        if (pMsg->bodyptr)
        {
            if (eHAL_STATUS_SUCCESS != palFreeMemory( pMac->hHdd, (tANI_U8*)pMsg->bodyptr))
                return eSIR_FAILURE;
        }
    }
    return eSIR_SUCCESS;
}

eANI_DVT_STATUS dvtProcessMsg(tpAniSirGlobal pMac, tDvtMsgbuffer *pDvtMsg)
{
    eANI_DVT_STATUS retVal = DVT_STATUS_SUCCESS;

    eDvtMsgId msgId = (eDvtMsgId)(pDvtMsg->msgId);
    uDvtMsgs *msgBody = (uDvtMsgs *)&(pDvtMsg->msgBody);

    switch (msgId)
    {
        case DVT_MSG_WRITE_REG:
        {
            retVal = dvtWriteReg(pMac, msgBody->WriteReg.addr, msgBody->WriteReg.value);
            break;
        }
        case DVT_MSG_READ_REG:
        {
            retVal = dvtReadReg(pMac, msgBody->ReadReg.addr, &(msgBody->ReadReg.value));
            break;
        }
        case DVT_MSG_WRITE_MEMORY:
        {
            retVal = dvtWriteMemory(pMac, msgBody->WriteMemory.macDestAddr, msgBody->WriteMemory.pBuf, msgBody->WriteMemory.bufSize);
            break;
        }
        case DVT_MSG_READ_MEMORY:
        {
            retVal = dvtReadMemory(pMac, msgBody->ReadMemory.macSourceAddr, msgBody->ReadMemory.pBuf, msgBody->ReadMemory.bufSize);
            break;
        }
        case DVT_MSG_CONFIG_PIPE:
        {
            retVal = dvtConfigPipe(pMac, msgBody->ConfigPipe.pipe, &(msgBody->ConfigPipe.pipeCfg));
            break;
        }
        case DVT_MSG_WRITE_PIPE_FRAME_TRANSFER:
        {
            retVal = dvtWritePipeFrameTransfer(pMac, msgBody->WritePipeFrameTransfer.pPacketArray, msgBody->WritePipeFrameTransfer.countPackets);
            break;
        }
        case DVT_MSG_CREATE_FRAME:
        {
            retVal = dvtCreateFrame(pMac, msgBody->CreateFrame.spec);
            break;
        }
        case DVT_MSG_NVI_WRITE_DATA:
        {
            retVal = dvtNviWriteData(pMac, msgBody->NviWriteData.eepromOffset, msgBody->NviWriteData.pBuf, msgBody->NviWriteData.nBytes);
            break;
        }
        case DVT_MSG_NVI_READ_DATA:
        {
            retVal = dvtNviReadData(pMac, msgBody->NviReadData.eepromOffset, msgBody->NviReadData.pBuf, msgBody->NviReadData.nBytes);
            break;
        }
        case DVT_MSG_NVI_WRITE_BURST_DATA:
        {
            retVal = dvtNviWriteBurstData(pMac, msgBody->NviWriteBurstData.eepromOffset, msgBody->NviWriteBurstData.pBuf, msgBody->NviWriteBurstData.nDwords);
            break;
        }
        case DVT_MSG_NVI_READ_BURST_DATA:
        {
            retVal = dvtNviReadBurstData(pMac, msgBody->NviReadBurstData.eepromOffset, msgBody->NviReadBurstData.pBuf, msgBody->NviReadBurstData.nDwords);
            break;
        }
        case DVT_MSG_GET_EEPROM_FIELD_SIZE:
        {
            retVal = dvtGetEepromFieldSize(pMac, msgBody->GetEepromFieldSize.field, &(msgBody->GetEepromFieldSize.fieldSize));
            break;
        }
        case DVT_MSG_GET_EEPROM_TABLE_SIZE:
        {
            retVal = dvtGetEepromTableSize(pMac, msgBody->GetEepromTableSize.table, &(msgBody->GetEepromTableSize.tableSize));
            break;
        }
        case DVT_MSG_GET_EEPROM_TABLE_DIR:
        {
            retVal = dvtGetEepromTableDir(pMac, msgBody->GetEepromTableDir.table, &(msgBody->GetEepromTableDir.dirEntry));
            break;
        }
        case DVT_MSG_READ_EEPROM_FIELD:
        {
            retVal = dvtReadEepromField(pMac, msgBody->ReadEepromField.field, &(msgBody->ReadEepromField.fieldData));
            break;
        }
        case DVT_MSG_WRITE_EEPROM_FIELD:
        {
            retVal = dvtWriteEepromField(pMac, msgBody->WriteEepromField.field, &(msgBody->WriteEepromField.fieldData));
            break;
        }
        case DVT_MSG_READ_EEPROM_TABLE:
        {
            retVal = dvtReadEepromTable(pMac, msgBody->ReadEepromTable.eepromTable, &(msgBody->ReadEepromTable.tableData), &(msgBody->ReadEepromTable.tableLen));
            break;
        }
        case DVT_MSG_WRITE_EEPROM_TABLE:
        {
            retVal = dvtWriteEepromTable(pMac, msgBody->WriteEepromTable.eepromTable, &(msgBody->WriteEepromTable.tableData));
            break;
        }
        case DVT_MSG_REMOVE_EEPROM_TABLE:
        {
            retVal = dvtRemoveEepromTable(pMac, msgBody->RemoveEepromTable.eepromTable);
            break;
        }
        case DVT_MSG_GATHER_INFO:
        {
            retVal = dvtGatherInfo(pMac, (sDvtGatherInfo *)&(msgBody->GatherInfo.dvtInfo), msgBody->GatherInfo.dbgInfoMask);
            break;
        }
        case DVT_MSG_CRC_CHECK_ENABLE:
        {
            retVal = dvtCrcCheckEnable(pMac, msgBody->CrcCheckEnable.crcEnable);
            break;
        }

		case DVT_MSG_DXE_TIMESTAMP_ENABLE:
        {
            retVal = dvtDxeTimestampEnable(pMac, msgBody->DxeTimestampEnable.dxeTimestampEnable);
            break;
        }

        case DVT_MSG_RESET_COUNTERS:
        {
            retVal = dvtResetCounters(pMac);
            break;
        }

        case DVT_MSG_DUMP_BD_PDU:
        {
            retVal = dvtDumpBdPdu(pMac, msgBody->DumpBdPdu.dumpMasknIndex, &msgBody->DumpBdPdu.pBuf[0]);
            break;
        }

        case DVT_MSG_SIMPLE_MAC_CONFIG:
        {
            retVal = dvtSimpleMacConfig(pMac, msgBody->SimpleMacConfig.mac);
            break;
        }

        case DVT_MSG_SIMPLE_BSS_CONFIG:
        {
            retVal = dvtSimpleBssConfig(pMac, msgBody->SimpleBssConfig.bss, &(msgBody->SimpleBssConfig.bssIndex));
            break;
        }

        case DVT_MSG_ADD_SIMPLE_STATION:
        {
            retVal = dvtAddSimpleStation(pMac, msgBody->AddSimpleStation.sta, &(msgBody->AddSimpleStation.stationIndex));
            break;
        }

        case DVT_MSG_DELETE_STATION:
        {
            retVal = dvtDeleteStation(pMac, msgBody->DeleteStation.stationIndex);
            break;
        }

        case DVT_MSG_INIT_ALL:
        {
            retVal = dvtInitAll(pMac);
            break;
        }

        case DVT_MSG_GET_STATION_TABLE:
        {
            retVal = dvtGetStationTable(pMac, &(msgBody->GetStationTable));
            break;
        }

        case DVT_MSG_MEM_TEST:
        {
            retVal = dvtMemTest(pMac, msgBody->MemTest.params, &msgBody->MemTest.failedAddr);
            break;
        }

        case DVT_MSG_GET_DPU_SIGNATURE:
        {
            retVal = dvtGetDpuSignature(pMac, msgBody->GetDpuSignature.staIdx, &(msgBody->GetDpuSignature.dpuSignature));
            break;
        }
        case DVT_MSG_SET_STA_KEY:
        {
            retVal = dvtSetStaKeyParams(pMac, msgBody->SetStaKeyParams.keyParams);
            break;
        }
        case DVT_MSG_SET_BSS_KEY:
        {
            retVal = dvtSetBssKeyParams(pMac, msgBody->SetBssKeyParams.keyParams);
            break;
        }
        case DVT_MSG_ADD_STATION:
        {
            retVal = dvtAddStation(pMac, msgBody->AddStation.sta);
            break;
        }
        case DVT_MSG_BSS_CONFIG:
        {
            retVal = dvtBssConfig(pMac, msgBody->BssConfig.bss);
            break;
        }
        case DVT_MSG_SET_EEPROM_BURST_PAGE_SIZE:
        {
            retVal = dvtSetEepromBurstPageSize(pMac, msgBody->SetEepromBurstPageSize.pageSize);
            break;
        }
        case DVT_MSG_GET_TX_COUNT_PER_RATE:
        {
            retVal = dvtGetTxCountPerRate(pMac, msgBody->GetTxCountPerRate.cTxCountPerRate, msgBody->GetTxCountPerRate.size);
            break;
        }
        case DVT_MSG_GET_RX_COUNT_PER_RATE:
        {
            retVal = dvtGetRxCountPerRate(pMac, msgBody->GetRxCountPerRate.cRxCountPerRate, msgBody->GetRxCountPerRate.size);
            break;
        }
        case DVT_MSG_GET_RX_COUNTERS:
        {
            retVal = dvtGetRxCounters(pMac, &(msgBody->GetRxCounters.rxCounters), msgBody->GetRxCounters.size);
            break;
        }
        case DVT_MSG_READ_REGS:
        {
            retVal = dvtReadRegs(pMac, &(msgBody->ReadRegs.regs), msgBody->ReadRegs.size);
            break;
        }
        case DVT_MSG_GET_COUNT_PER_STA:
        {
            retVal = dvtGetCountPerSta(pMac, msgBody->GetCountPerSta.countPerSta, msgBody->GetCountPerSta.isTx);
            break;
        }
        case DVT_MSG_GET_RX_PMI_RATE_IDX_MISMATCH_COUNT:
        {
            retVal = dvtGetRxPMIRateIdxMisMatchCount(pMac, msgBody->GetRxPMIRateIdxMisMatchCount.cRxPMIRateIdxMisMatchCount, msgBody->GetRxPMIRateIdxMisMatchCount.size);
            break;
        }
        case DVT_MSG_WRITE_BEACON_TO_MEMORY:
        {
            retVal = dvtWriteBeaconToMemory(pMac, msgBody->WriteBeaconToMemory.beacon, msgBody->WriteBeaconToMemory.bssIndex, msgBody->WriteBeaconToMemory.length);
            break;
        }
        case DVT_MSG_GET_SMAC_RUNTIME_STATS:
        {
            retVal = dvtGetSmacRuntimeStats(pMac, &(msgBody->GetSmacRuntimeStats.smacRuntimeStats), msgBody->GetSmacRuntimeStats.size);
            break;
        }
    }

    return (retVal);
}

#endif //ANI_DVT_DEBUG

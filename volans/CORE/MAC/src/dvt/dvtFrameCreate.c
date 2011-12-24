/** ------------------------------------------------------------------------- *
    ------------------------------------------------------------------------- *


    \file dvtFrameCreate.c

    \brief Frame creation/parsing

    $Id$

    Copyright (c) 2011 Qualcomm Atheros, Inc. 
    All Rights Reserved. 
    Qualcomm Atheros Confidential and Proprietary. 

    Copyright (C) 2006 Airgo Networks, Incorporated

   ========================================================================== */

#include <stdlib.h>
#include <string.h>
#include "ani_assert.h"
#include "mpdu.h"
#include "aniGlobal.h"
#include "dvtDebug.h"
#include "dvtTxRxHeap.h"
#include "dvtQueues.h"
#include "dvtFrames.h"
#include "dvtModuleApi.h"

static eANI_DVT_STATUS dvtCreateSingleFrame(tpAniSirGlobal pMac, sDvtFrameSpec *spec);
static eANI_DVT_STATUS dvtFormatFrame(tpAniSirGlobal pMac, sDvtFrameSpec *spec, tSmacBdHostTx *txBd, tANI_U32 mpduHdrOffset, sHddMemSegment *payloadList, tANI_U32 length);
static eANI_DVT_STATUS GetValue(tpAniSirGlobal pMac, sTestFormat *format, eFrameOptions param, tANI_U32 *value);
tANI_U32 SwapWord32Endianess( tANI_U32 dword );
void SwapBuffer32Endianess( tANI_U32 *pU32, tANI_U32 cBytes );


const sDvtTestBounds frameBounds[MAX_DVT_FRAME_OPTION] =
{
     //  min,     max,     last
    {  1,     8192,      0  },     //DVT_FRAME_SEGMENTATION,
    {  0,     8000,      0  },     //DVT_FRAME_LENGTH,
    {  0,     255,       0  },     //DVT_FRAME_PAYLOAD_BYTE,
    {  0,     0,         0  },     //DVT_FRAME_PROTOCOL_VER,
    {  0,     3,         0  },     //DVT_FRAME_FRAME_TYPE,
    {  0,     16,        0  },     //DVT_FRAME_FRAME_SUBTYPE,
    {  0,     3,         0  },     //DVT_FRAME_TO_FROM_DS,
    {  0,     1,         0  },     //DVT_FRAME_MORE_FRAG,
    {  0,     1,         0  },     //DVT_FRAME_RETRY,
    {  0,     1,         0  },     //DVT_FRAME_PWR_MGMT,
    {  0,     1,         0  },     //DVT_FRAME_MORE_DATA,
    {  0,     2,         0  },     //DVT_FRAME_PRIVACY,
    {  0,     0xFFFF,    0  },     //DVT_FRAME_DURATION_ID,
    {  0,     15,        0  },     //DVT_FRAME_ADDR1,
    {  0,     15,        0  },     //DVT_FRAME_ADDR2,
    {  0,     15,        0  },     //DVT_FRAME_ADDR3,
    {  0,     15,        0  },     //DVT_FRAME_ADDR4,
    {  0,     15,        0  },     //DVT_FRAME_FRAG_NUM,
    {  0,     4095,      0  },     //DVT_FRAME_SEQUENCE_NUM,
    {  0,     15,        0  },     //DVT_FRAME_QOS_TID,
    {  0,     1,         0  },     //DVT_FRAME_QOS_RSVD,
    {  0,     2,         0  },     //DVT_FRAME_QOS_ACK_POLICY,
    {  0,     1,         0  },     //DVT_FRAME_RSVD,
    {  0,     1,         0  },     //DVT_FRAME_QOS_EOSP,
    {  0,     2,         0  },     //DVT_FRAME_QOS_TXOP_LIMIT,
    {  0,     3,         0  },     //DVT_TX_BD_BDT,
    {  0,     1,         0  },     //DVT_TX_BD_DPU_NC,
    {  0,     1,         0  },     //DVT_TX_BD_DPU_NE,
    {  0,     3,         0  },     //DVT_TX_BD_SW_BD_TYPE,
    {  0,     1023,      0  },     //DVT_TX_BD_RESERVED1,
    {  0,     15,        0  },     //DVT_TX_BD_STA_SIGNATURE,
    {  0,     15,        0  },     //DVT_TX_BD_DPU_SIGNATURE,
    {  0,     255,       0  },     //DVT_TX_BD_DPU_RF,
    {  0,     65535,     0  },     //DVT_TX_BD_RESERVED3,
    {  0,     1,         0  },     //DVT_TX_BD_ENABLE_AGGREGATE,
    {  0,     1,         0  },     //DVT_TX_BD_OVERWRITE_PHY,
    {  0,     3,         0  },     //DVT_TX_BD_FORCE_MAC_PROT,
    {  0,     7,         0  },     //DVT_TX_BD_FORCE_ACK_POLICY,
    {  0,     1,         0  },     //DVT_TX_BD_TX_INDICATION,
    {  0,     1,         0  },     //DVT_TX_BD_KEEP_SEQ_NUM
    {  0,     255,       0  },     //DVT_TX_BD_RESERVED2,
    {  0,     65535,     0  },     //DVT_TX_BD_TAIL_PDU_IDX,
    {  0,     65535,     0  },     //DVT_TX_BD_HEAD_PDU_IDX,
    {  0,     127,       0  },     //DVT_TX_BD_PDU_COUNT,
    {  0,     511,       0  },     //DVT_TX_BD_MPDU_DATA_OFFSET,
    {  0,     255,       0  },     //DVT_TX_BD_MPDU_HEADER_OFFSET,
    {  0,     255,       0  },     //DVT_TX_BD_MPDU_HEADER_LENGTH,
    {  0,     255,       0  },     //DVT_TX_BD_RATE_INDEX,
    {  0,     15,        0  },     //DVT_TX_BD_TID,
    {  0,     1,         0  },     //DVT_TX_BD_FRAGMENT,
    {  0,     1,         0  },     //DVT_TX_BD_LAST_FRAGMENT,
    {  0,     3,         0  },     //DVT_TX_BD_RESERVED4,
    {  0,     65535,     0  },     //DVT_TX_BD_MPDU_LENGTH,
    {  0,     255,       0  },     //DVT_TX_BD_RESERVED5,
    {  0,     255,       0  },     //DVT_TX_BD_TA_INDEX,
    {  0,     255,       0  },     //DVT_TX_BD_RA_INDEX,
    {  0,     255,       0  },     //DVT_TX_BD_DPU_DESC_IDX,
    {  0,     65535,     0  },     //DVT_TX_BD_TIMESTAMP,
    {  0,     255,       0  },     //DVT_TX_BD_RESERVED6,
    {  0,     255,       0  },     //DVT_TX_BD_RETRY_COUNT,
};


void dvtFrameBoundsInit(tpAniSirGlobal pMac)
{
    tANI_U32 i;

    for (i = 0; i < MAX_DVT_FRAME_OPTION; i++)
    {
       pMac->dvt.frameBounds[i].min = frameBounds[i].min;
       pMac->dvt.frameBounds[i].max = frameBounds[i].max;
       pMac->dvt.frameBounds[i].last = frameBounds[i].last;
    }

}




eANI_DVT_STATUS dvtCreateFrame(tpAniSirGlobal pMac, sDvtFrameSpec spec)
{
    tANI_U32 framesCreated;

    assert(spec.txQueueId < NUM_DVT_TX_QUEUES);

    if (pMac->dvt.txQueues[spec.txQueueId].spec != NULL)
    {
        //previous frameSpec in play - must complete or be aborted
        dvtLog(pMac, LOGE, "ERROR: Tried to configure a new frameSpec but one already existed on queue %d\n", spec.txQueueId);
        return (DVT_STATUS_FAILURE);
    }

    if (eHAL_STATUS_SUCCESS == palAllocateMemory(pMac->hHdd, &pMac->dvt.txQueues[spec.txQueueId].spec, sizeof(sDvtFrameSpec)))
    {
        memcpy(pMac->dvt.txQueues[spec.txQueueId].spec, &spec, sizeof(sDvtFrameSpec));

        pMac->dvt.txQueues[spec.txQueueId].framesLeftToCreate = spec.totalNumber;
        pMac->dvt.txQueues[spec.txQueueId].framesPerThread = spec.threadNumber;
        framesCreated = 0;
        return (dvtContinueFrameSpec(pMac, &spec, &framesCreated));
    }
    else
    {
        dvtLog(pMac, LOGE, "ERROR: Couldn't allocate for a frame spec\n");
        return (DVT_STATUS_FAILURE);
    }
}


eANI_DVT_STATUS dvtAbortFrameSpec(tpAniSirGlobal pMac, eDvtTxQueue queue)
{
    //cease creating frames on current frameSpec
    assert(queue < NUM_DVT_TX_QUEUES);

    //check to see if the spec has already been freed first
    if (pMac->dvt.txQueues[queue].spec != NULL)
    {
        pMac->dvt.txQueues[queue].framesLeftToCreate = 0;
        pMac->dvt.txQueues[queue].framesPerThread = 0;
        palFreeMemory(pMac->hHdd, pMac->dvt.txQueues[queue].spec);

        pMac->dvt.txQueues[queue].spec = NULL;
    }

    // ! Note that we need to log a frame spec completion event to the application layer
    dvtLog(pMac, LOGE, " dvtAbortFrameSpec\n");
    
    return (DVT_STATUS_SUCCESS);
}


//returns the number of frames created
eANI_DVT_STATUS dvtContinueFrameSpec(tpAniSirGlobal pMac, sDvtFrameSpec *spec, tANI_U32 *framesCreated)
{
    tANI_U32 frameCount;

    assert(spec != NULL);

    if (pMac->dvt.txQueues[spec->txQueueId].framesLeftToCreate < pMac->dvt.txQueues[spec->txQueueId].framesPerThread)
    {
        frameCount = pMac->dvt.txQueues[spec->txQueueId].framesLeftToCreate;
    }
    else
    {
        frameCount = pMac->dvt.txQueues[spec->txQueueId].framesPerThread;
    }

    while ((frameCount > 0) &&
           (dvtCreateSingleFrame(pMac, spec) == DVT_STATUS_SUCCESS)
          )
    {
        pMac->dvt.txQueues[spec->txQueueId].framesLeftToCreate--;
        frameCount--;
        (*framesCreated)++;
    }
    dvtLog(pMac, LOGE, "dvtContinueFrameSpec: \n framesCreated = %d on queue %d\n", (*framesCreated), spec->txQueueId);
    dvtLog(pMac, LOGE, " framesLeftToCreate = %d\n", pMac->dvt.txQueues[spec->txQueueId].framesLeftToCreate);

    //check to see if all frames are completed
    if (pMac->dvt.txQueues[spec->txQueueId].framesLeftToCreate == 0)
    {
        dvtAbortFrameSpec(pMac, spec->txQueueId);
    }


    return (DVT_STATUS_SUCCESS);
}

static eANI_DVT_STATUS dvtCreateSingleFrame(tpAniSirGlobal pMac, sDvtFrameSpec *spec)
{
    sTxFrameTransfer *txFrame;
    eDvtTxQueue queue = spec->txQueueId;

    if (eHAL_STATUS_SUCCESS == palAllocateMemory(pMac->hHdd, &txFrame, sizeof(sTxFrameTransfer)))
    {
        assert(txFrame != NULL);
        {
            // ! Note that for the moment, we are allocating a single 128 byte segment for the BD
            sHddMemSegment *txBdSeg = GetHeapBytes(pMac, &pMac->dvt.txHeap, BD_SIZE, DVT_HEAP_DWORD_ACCESS);

            if (txBdSeg != NULL)
            {
                //BD allocated
                //now figure out how to segment the rest of the frame for transfer
                //to do this, we need to know what the payload size is going to be.

                // ! Note that for the moment this implementation chooses a single segment size for the frame
                // ! so if it is random, the same random number is used to allocate all the segments

                // ! Note that for the moment this implementation is expecting the mpduHdr to be inside the BD
                // ! it will return failure if it is not
                tANI_U32 specLength;
                tANI_U32 specHdrOffset;
                tANI_U32 length;
                tANI_U32 segSize;
                tANI_U32 value;
                tANI_U32 segCount = 0;
                sHddMemSegment *payloadHead = NULL;
                sHddMemSegment *payloadTail = NULL;

                 assert(txBdSeg->next == NULL);
                 assert(txBdSeg->length == BD_SIZE);

                if ((DVT_STATUS_SUCCESS != GetValue(pMac, &spec->payload.length, DVT_FRAME_LENGTH, &specLength)) ||
                    (DVT_STATUS_SUCCESS != GetValue(pMac, &spec->bdFormatOptions.mpduHeaderOffset, DVT_TX_BD_MPDU_HEADER_OFFSET, &specHdrOffset)) ||
                    (DVT_STATUS_SUCCESS != GetValue(pMac, &spec->payload.segmentation, DVT_FRAME_SEGMENTATION, &segSize)) ||
                    (specHdrOffset % 4 != 0) ||
                    (specHdrOffset > (BD_SIZE - sizeof(uMpduHeader)))
                   )
                {
                    //failed, free what has been allocated thus far
                    eHalStatus retVal = palFreeMemory(pMac->hHdd, txFrame);
                    assert(retVal == eHAL_STATUS_SUCCESS);

                    dvtLog(pMac, LOGE, "ERROR: frame spec parameter failure\n");
                    
                    PutHeapBytes(pMac, &pMac->dvt.txHeap, txBdSeg);
                    return(DVT_STATUS_FAILURE);
                }
                length = specLength;

                //loop until allocations add up to the payload size
                while (length > 0)
                {
                    sHddMemSegment *payloadSeg;

                    if (segSize > length)
                    {
                        //reduce segSize to remaining length
                        segSize = length;
                    }

                    payloadSeg = GetHeapBytes(pMac, &pMac->dvt.txHeap, segSize, DVT_HEAP_BYTE_ACCESS);

                    if (payloadSeg == NULL)
                    {
                        //failed, free what has been allocated thus far
                        eHalStatus retVal = palFreeMemory(pMac->hHdd, txFrame);
                        assert(retVal == eHAL_STATUS_SUCCESS);

                        dvtLog(pMac, LOGE, "ERROR: failed to allocate payload segment from txHeap on queue %d\n", queue);
                        PutHeapBytes(pMac, &pMac->dvt.txHeap, txBdSeg);

                        if (payloadHead != NULL)
                        {
                            //some heap bytes already allocated for payload
                            PutHeapBytes(pMac, &pMac->dvt.txHeap, payloadHead);
                        }

                        return(DVT_STATUS_FAILURE);
                    }
                    else
                    {
                        if (payloadHead == NULL)
                        {
                            //first segment of payload
                            payloadHead = payloadSeg;
                            payloadTail = payloadSeg;
                        }
                        else
                        {
                            assert(payloadTail->next == NULL);
                            payloadTail->next = payloadSeg;
                            payloadTail = payloadSeg;
                        }
                        length -= segSize;
                        segCount ++;
                    }
                }

                //we now have a bd segment and a payload list of segments
                //now format the bd and payload
                {
                    tSmacBdHostTx *txBd = (tSmacBdHostTx *)txBdSeg->hddContext;

                    if (dvtFormatFrame(pMac, spec, txBd, specHdrOffset, payloadHead, specLength) != DVT_STATUS_SUCCESS)
                    {
                        //failed, free what has been allocated thus far
                        eHalStatus retVal = palFreeMemory(pMac->hHdd, txFrame);
                        assert(retVal == eHAL_STATUS_SUCCESS);

                        dvtLog(pMac, LOGE, "ERROR: dvtFormatFrame failure\n");
                        PutHeapBytes(pMac, &pMac->dvt.txHeap, txBdSeg);

                        if (payloadHead != NULL)
                        {
                            //some heap bytes already allocated for payload
                            PutHeapBytes(pMac, &pMac->dvt.txHeap, payloadHead);
                        }

                        dvtAbortFrameSpec(pMac, queue); //bad frame spec

                        return(DVT_STATUS_BAD_FRAME_SPEC);

                    }
                    else
                    {
                        //frame BD and Payload allocated and formatted
                        //fill in txFrame contents
                        txFrame->bd = txBdSeg;
                        txFrame->descCount = segCount + 1;  //payload segments + 1 for the BD
                        txFrame->segment = payloadHead;
                        txFrame->next = NULL;

                        //Now, enqueue txFrame for transmit
                        if (dvtTxEnqueueFrame(pMac, queue, txFrame) != DVT_STATUS_SUCCESS)
                        {
                            //failed, free what has been allocated thus far
                            eHalStatus retVal = palFreeMemory(pMac->hHdd, txFrame);
                            assert(retVal == eHAL_STATUS_SUCCESS);
                            
                            dvtLog(pMac, LOGE, "ERROR: dvtTxEnqueueFrame failed on queue %d\n", queue);

                            PutHeapBytes(pMac, &pMac->dvt.txHeap, txBdSeg);

                            if (payloadHead != NULL)
                            {
                                //some heap bytes already allocated for payload
                                PutHeapBytes(pMac, &pMac->dvt.txHeap, payloadHead);
                            }

                            return(DVT_STATUS_FAILURE);
                        }
                    }
                }
            }
            else
            {
                //failed, free what has been allocated thus far
                eHalStatus retVal = palFreeMemory(pMac->hHdd, txFrame);
                dvtLog(pMac, LOGE, "ERROR: failed to allocate txBd on queue %d\n", queue);
                
                assert(retVal == eHAL_STATUS_SUCCESS);
            }
        }
    }
    else
    {
        //nothing allocated thus far
        dvtLog(pMac, LOGE, "ERROR: failed to allocate sTxFrameTransfer txFrame on queue %d\n", queue);
        return (DVT_STATUS_FAILURE);
    }


    return (DVT_STATUS_SUCCESS);
}

#define TEST_HDR_OFFSET

//INPUT spec
//INPUT txBd - where to put bd values
//INPUT mpduHdr - where to put mpdu header values
//INPUT payload - linked list of sHddMemSegments
static eANI_DVT_STATUS dvtFormatFrame(tpAniSirGlobal pMac, sDvtFrameSpec *spec, tSmacBdHostTx *txBd, tANI_U32 mpduHdrOffset, sHddMemSegment *payloadList, tANI_U32 length)
{
    eANI_DVT_STATUS retVal = DVT_STATUS_SUCCESS;
    eFrameOptions param;
    sTestFormat *format;
    tANI_U32 value;
    sMPDUHeader *mpduHdr = (sMPDUHeader *)((tANI_U8 *)txBd + 0x40);


    assert((tANI_U32)txBd % 4 == 0); //this must be word aligned
    assert(mpduHdrOffset % 4 == 0); //this must be word aligned

    {
        //DEBUG: for the moment ignore the frame spec to setup the bd and mpdu header
        //fill in BD and MPDU Header until we get other formatting in place
        tANI_U32 staIndex;

        for (staIndex = 0; staIndex < NUM_DEST_STATIONS; staIndex++)
        {
            if (pMac->dvt.simpleStaTable[staIndex].configured == eANI_BOOLEAN_TRUE)
            {
                //use this station to transmit to
                break;
            }
        }

        if ((staIndex == NUM_DEST_STATIONS) ||
            (pMac->dvt.bss[0].configured == eANI_BOOLEAN_FALSE) ||
            (pMac->dvt.mac.configured == eANI_BOOLEAN_FALSE)
           )
        {
            //we do not have everything configured that we need
            return (DVT_STATUS_FAILURE);
        }


        /* 0x00 */
        txBd->bdt = 0;                   //2 bits
        txBd->dpuNC = 1;                 //1 bits
        txBd->dpuNE = 1;                 //1 bits
        txBd->swBdType = 1;              //2 bits
        txBd->reserved1 = 0;             //10 bits
        txBd->staSignature = 0;          //4 bits
        txBd->dpuSignature = 0;          //4 bits
        txBd->dpuRF = SMAC_BMUWQ_MCPU_TX_WQ0;                 //8 bits

        /* 0x04 */
        txBd->enableAggregate = 0;       //1 bits
        txBd->overwritePhy = 0;          //1 bits
        txBd->forceMacProt = 1;          //2 bits
        txBd->forceAckPolicy = 0;        //3 bits
        txBd->txIndication = 0;          //1 bits
        txBd->keepSeqNum = 1;            //1 bits
        txBd->reserved2 = 0;             //7 bits

        /* 0x08 */
        txBd->tailPduIdx = 0;            //16 bits
        txBd->headPduIdx = 0;            //16 bits

        /* 0x0c */
        txBd->pduCount = 0;              //7 bits
        txBd->mpduDataOffset = 128;        //9 bits
        txBd->mpduHeaderOffset = 0x40;   //8 bits
        txBd->mpduHeaderLength = 24;      //8 bits

        /* 0x10 */
        txBd->rateIndex = 208;             //8 bits
        txBd->reserved3 = 0;              //4 bits
        txBd->tid = 0;                   //4 bits
        txBd->reserved4 = 0;             //2 bits
        txBd->mpduLength = length;       //16 bits

        /* 0x14 */
        txBd->reserved5 = 0;             //8 bits
        txBd->taIndex = 0;               //8 bits
        txBd->raIndex = staIndex;        //8 bits
        txBd->dpuDescIdx = 0;            //8 bits

        //set MPDU header fields
        SET_PROTOCOL_VERSION(mpduHdr->frameType, 0);
        SET_MPDU_TYPE(mpduHdr->frameType, DATA_FRAME_TYPE);
        SET_MPDU_SUBTYPE(mpduHdr->frameType, DATA);

        SET_TO_FROM_DS_BIT(mpduHdr->frameCtrl, DS_STA_TO_STA);
        SET_MORE_FRAGS_BIT(mpduHdr->frameCtrl, 0);
        SET_RETRY_BIT(mpduHdr->frameCtrl, 0);
        SET_PWR_MGMT_BIT(mpduHdr->frameCtrl, 0);
        SET_MORE_DATA_BIT(mpduHdr->frameCtrl, 0);
        SET_PRIVACY(mpduHdr->frameCtrl, 0);

        SET_DURATION(mpduHdr->duration, 0); //smac should fill in
        SET_AID(mpduHdr->duration, 0);      //smac should fill in

        SET_FRAG_NUM(mpduHdr->seqNum, 0);
        SET_SEQ_NUM(mpduHdr->seqNum, 0);    //smac should fill in

        {
            tANI_U8 *txAddr = GET_TX_ADDR_PTR(mpduHdr);
            tANI_U8 *srcAddr = GET_SRC_ADDR_PTR(mpduHdr);
            tANI_U8 *destAddr = GET_DEST_ADDR_PTR(mpduHdr);
            tANI_U8 *bssAddr = GET_BSSID_ADDR_PTR(mpduHdr);


            if (txAddr)
                memcpy(txAddr, pMac->dvt.mac.macAddr, sizeof(tSirMacAddr));

            if (srcAddr)
                memcpy(srcAddr, pMac->dvt.mac.macAddr, sizeof(tSirMacAddr));

            if (destAddr)
                memcpy(destAddr, pMac->dvt.simpleStaTable[staIndex].macAddr, sizeof(tSirMacAddr));

            if (bssAddr)
                memcpy(bssAddr, pMac->dvt.bss[0].bssId, sizeof(tSirMacAddr));
        }

    }




/*
    //construct frame contents based on each format option
    param = DVT_FRAME_PROTOCOL_VER;
    while(param < MAX_DVT_FRAME_OPTION)
    {
        switch (param)
        {

            //MPDU Header formatting
            case DVT_FRAME_PROTOCOL_VER:
                format = &spec->hdr.protocolVer;
                retVal = GetValue(pMac, format, param, &value);

                mpduHdr->basicMpduHdr.frameType =
                 SET_PROTOCOL_VERSION(mpduHdr->basicMpduHdr.frameType, (tMacFrameType)value);
                break;
            case DVT_FRAME_FRAME_TYPE:
                format = &spec->hdr.frameType;
                retVal = GetValue(pMac, format, param, &value);

                mpduHdr->basicMpduHdr.frameType =
                 SET_MPDU_TYPE(mpduHdr->basicMpduHdr.frameType, (tMacFrameType)value);
                break;
            case DVT_FRAME_FRAME_SUBTYPE:
                format = &spec->hdr.frameSubtype;
                retVal = GetValue(pMac, format, param, &value);

                mpduHdr->basicMpduHdr.frameType =
                 SET_MPDU_SUBTYPE(mpduHdr->basicMpduHdr.frameType, (tMacFrameType)value);
                break;
            case DVT_FRAME_TO_FROM_DS:
                format = &spec->hdr.toFromDs;
                retVal = GetValue(pMac, format, param, &value);

                mpduHdr->basicMpduHdr.frameCtrl =
                 SET_TO_FROM_DS_BIT(mpduHdr->basicMpduHdr.frameCtrl, (tMacFrameCtrl)value);
                break;
            case DVT_FRAME_MORE_FRAG:
                format = &spec->hdr.moreFrag;
                retVal = GetValue(pMac, format, param, &value);

                mpduHdr->basicMpduHdr.frameCtrl =
                 SET_MORE_FRAGS_BIT(mpduHdr->basicMpduHdr.frameCtrl, (tMacFrameCtrl)value);
                break;
            case DVT_FRAME_RETRY:
                format = &spec->hdr.retry;
                retVal = GetValue(pMac, format, param, &value);

                mpduHdr->basicMpduHdr.frameCtrl =
                 SET_RETRY_BIT(mpduHdr->basicMpduHdr.frameCtrl, (tMacFrameCtrl)value);
                break;
            case DVT_FRAME_PWR_MGMT:
                format = &spec->hdr.pwrMgmt;
                retVal = GetValue(pMac, format, param, &value);

                mpduHdr->basicMpduHdr.frameCtrl =
                 SET_PWR_MGMT_BIT(mpduHdr->basicMpduHdr.frameCtrl, (tMacFrameCtrl)value);
                break;
            case DVT_FRAME_MORE_DATA:
                format = &spec->hdr.moreData;
                retVal = GetValue(pMac, format, param, &value);

                mpduHdr->basicMpduHdr.frameCtrl =
                 SET_MORE_DATA_BIT(mpduHdr->basicMpduHdr.frameCtrl, (tMacFrameCtrl)value);
                break;
            case DVT_FRAME_PRIVACY:
                format = &spec->hdr.privacy;
                retVal = GetValue(pMac, format, param, &value);

                mpduHdr->basicMpduHdr.frameCtrl =
                 SET_PRIVACY(mpduHdr->basicMpduHdr.frameCtrl, (tMacFrameCtrl)value);
                break;
            case DVT_FRAME_DURATION_ID:
                format = &spec->hdr.durationId;
                retVal = GetValue(pMac, format, param, &value);

                mpduHdr->basicMpduHdr.duration =
                 SET_DURATION(mpduHdr->basicMpduHdr.duration, (tANI_U16)value);
                break;
            case DVT_FRAME_ADDR1:
                format = &spec->hdr.addr1;
                retVal = GetValue(pMac, format, param, &value);
                break;
            case DVT_FRAME_ADDR2:
                format = &spec->hdr.addr2;
                retVal = GetValue(pMac, format, param, &value);
                break;
            case DVT_FRAME_ADDR3:
                format = &spec->hdr.addr3;
                retVal = GetValue(pMac, format, param, &value);
                break;
            case DVT_FRAME_ADDR4:
                format = &spec->hdr.addr4;
                retVal = GetValue(pMac, format, param, &value);
                break;
            case DVT_FRAME_FRAG_NUM:
                format = &spec->hdr.fragNum;
                retVal = GetValue(pMac, format, param, &value);

                mpduHdr->basicMpduHdr.seqNum =
                 SET_FRAG_NUM(mpduHdr->basicMpduHdr.seqNum, (tANI_U16)value);
                break;
            case DVT_FRAME_SEQUENCE_NUM:
                format = &spec->hdr.sequenceNum;
                retVal = GetValue(pMac, format, param, &value);

                mpduHdr->basicMpduHdr.seqNum =
                 SET_SEQ_NUM(mpduHdr->basicMpduHdr.seqNum, (tANI_U16)value);
                break;
            case DVT_FRAME_QOS_TID:
                format = &spec->hdr.qosTID;
                retVal = GetValue(pMac, format, param, &value);

                //assumes that TO_FROM_DS has already been set
                if (TEST_AP_TO_AP(mpduHdr->basicMpduHdr.frameCtrl) == eANI_BOOLEAN_TRUE)
                {
                    //QOS WDS frame
                    mpduHdr->qosWdsMpduHdr.qosCtrl =
                     SET_TID(mpduHdr->qosWdsMpduHdr.qosCtrl, (tQosControl)value);
                }
                else
                {
                    //QOS frame
                    mpduHdr->qosMpduHdr.qosCtrl =
                     SET_TID(mpduHdr->qosMpduHdr.qosCtrl, (tQosControl)value);
                }
                break;
            case DVT_FRAME_QOS_RSVD:
                format = &spec->hdr.qosRsvd;
                retVal = GetValue(pMac, format, param, &value);
                break;
            case DVT_FRAME_QOS_ACK_POLICY:
                format = &spec->hdr.qosAckPolicy;
                retVal = GetValue(pMac, format, param, &value);

                //assumes that TO_FROM_DS has already been set
                if (TEST_AP_TO_AP(mpduHdr->basicMpduHdr.frameCtrl) == eANI_BOOLEAN_TRUE)
                {
                    //QOS WDS frame
                    mpduHdr->qosWdsMpduHdr.qosCtrl =
                     SET_ACK_POLICY(mpduHdr->qosWdsMpduHdr.qosCtrl, (tQosControl)value);
                }
                else
                {
                    //QOS frame
                    mpduHdr->qosMpduHdr.qosCtrl =
                     SET_ACK_POLICY(mpduHdr->qosMpduHdr.qosCtrl, (tQosControl)value);
                }
                break;
            case DVT_FRAME_RSVD:
                format = &spec->hdr.Rsvd;
                retVal = GetValue(pMac, format, param, &value);

                //assumes that TO_FROM_DS has already been set
                if (TEST_AP_TO_AP(mpduHdr->basicMpduHdr.frameCtrl) == eANI_BOOLEAN_TRUE)
                {
                    //QOS WDS frame
                    mpduHdr->qosWdsMpduHdr.qosCtrl =
                     SET_RSVD(mpduHdr->qosWdsMpduHdr.qosCtrl, (tQosControl)value);
                }
                else
                {
                    //QOS frame
                    mpduHdr->qosMpduHdr.qosCtrl =
                     SET_RSVD(mpduHdr->qosMpduHdr.qosCtrl, (tQosControl)value);
                }
                break;
            case DVT_FRAME_QOS_EOSP:
                format = &spec->hdr.qosEOSP;
                retVal = GetValue(pMac, format, param, &value);

                //assumes that TO_FROM_DS has already been set
                if (TEST_AP_TO_AP(mpduHdr->basicMpduHdr.frameCtrl) == eANI_BOOLEAN_TRUE)
                {
                    //QOS WDS frame
                    mpduHdr->qosWdsMpduHdr.qosCtrl =
                     SET_EOSP(mpduHdr->qosWdsMpduHdr.qosCtrl, (tQosControl)value);
                }
                else
                {
                    //QOS frame
                    mpduHdr->qosMpduHdr.qosCtrl =
                     SET_EOSP(mpduHdr->qosMpduHdr.qosCtrl, (tQosControl)value);
                }
                break;
            case DVT_FRAME_QOS_TXOP_LIMIT:
                format = &spec->hdr.qosTxopLimit;
                retVal = GetValue(pMac, format, param, &value);

                //assumes that TO_FROM_DS has already been set
                if (TEST_AP_TO_AP(mpduHdr->basicMpduHdr.frameCtrl) == eANI_BOOLEAN_TRUE)
                {
                    //QOS WDS frame
                    mpduHdr->qosWdsMpduHdr.qosCtrl =
                     SET_CF_POLL_TX_LIMIT(mpduHdr->qosWdsMpduHdr.qosCtrl, (tQosControl)value);
                }
                else
                {
                    //QOS frame
                    mpduHdr->qosMpduHdr.qosCtrl =
                     SET_CF_POLL_TX_LIMIT(mpduHdr->qosMpduHdr.qosCtrl, (tQosControl)value);
                }
                break;


            // Tx BD Formatting
            case DVT_TX_BD_BDT:
                format = &spec->bdFormatOptions.bdt;
                retVal = GetValue(pMac, format, param, &value);
                txBd->bdt = retVal;
                break;
            case DVT_TX_BD_DPU_NC:
                format = &spec->bdFormatOptions.dpuNC;
                retVal = GetValue(pMac, format, param, &value);
                txBd->dpuNC = retVal;
                break;
            case DVT_TX_BD_DPU_NE:
                format = &spec->bdFormatOptions.dpuNE;
                retVal = GetValue(pMac, format, param, &value);
                txBd->dpuNE = retVal;
                break;
            case DVT_TX_BD_SW_BD_TYPE:
                format = &spec->bdFormatOptions.swBdType;
                retVal = GetValue(pMac, format, param, &value);
                txBd->swBdType = retVal;
                break;
            case DVT_TX_BD_RESERVED1:
                format = &spec->bdFormatOptions.reserved1;
                retVal = GetValue(pMac, format, param, &value);
                txBd->reserved1 = retVal;
                break;
            case DVT_TX_BD_STA_SIGNATURE:
                format = &spec->bdFormatOptions.staSignature;
                retVal = GetValue(pMac, format, param, &value);
                txBd->staSignature = retVal;
                break;
            case DVT_TX_BD_DPU_SIGNATURE:
                format = &spec->bdFormatOptions.dpuSignature;
                retVal = GetValue(pMac, format, param, &value);
                txBd->dpuSignature = retVal;
                break;
            case DVT_TX_BD_DPU_RF:
                format = &spec->bdFormatOptions.dpuRF;
                retVal = GetValue(pMac, format, param, &value);
                txBd->dpuRF = retVal;
                break;
            case DVT_TX_BD_RESERVED3:
                format = &spec->bdFormatOptions.reserved3;
                retVal = GetValue(pMac, format, param, &value);
                txBd->reserved3 = retVal;
                break;
            case DVT_TX_BD_ENABLE_AGGREGATE:
                format = &spec->bdFormatOptions.enableAggregate;
                retVal = GetValue(pMac, format, param, &value);
                txBd->enableAggregate = retVal;
                break;
            case DVT_TX_BD_OVERWRITE_PHY:
                format = &spec->bdFormatOptions.overwritePhy;
                retVal = GetValue(pMac, format, param, &value);
                txBd->overwritePhy = retVal;
                break;
            case DVT_TX_BD_FORCE_MAC_PROT:
                format = &spec->bdFormatOptions.forceMacProt;
                retVal = GetValue(pMac, format, param, &value);
                txBd->forceMacProt = retVal;
                break;
            case DVT_TX_BD_FORCE_ACK_POLICY:
                format = &spec->bdFormatOptions.forceAckPolicy;
                retVal = GetValue(pMac, format, param, &value);
                txBd->forceAckPolicy = retVal;
                break;
            case DVT_TX_BD_TX_INDICATION:
                format = &spec->bdFormatOptions.txIndication;
                retVal = GetValue(pMac, format, param, &value);
                txBd->txIndication = retVal;
                break;
            case DVT_TX_BD_KEEP_SEQ_NUM:
                format = &spec->bdFormatOptions.keepSeqNum;
                retVal = GetValue(pMac, format, param, &value);
                txBd->keepSeqNum = retVal;
                break;
            case DVT_TX_BD_RESERVED2:
                format = &spec->bdFormatOptions.reserved2;
                retVal = GetValue(pMac, format, param, &value);
                txBd->reserved2 = retVal;
                break;
            case DVT_TX_BD_TAIL_PDU_IDX:
                format = &spec->bdFormatOptions.tailPduIdx;
                retVal = GetValue(pMac, format, param, &value);
                txBd->tailPduIdx = retVal;
                break;
            case DVT_TX_BD_HEAD_PDU_IDX:
                format = &spec->bdFormatOptions.headPduIdx;
                retVal = GetValue(pMac, format, param, &value);
                txBd->headPduIdx = retVal;
                break;
            case DVT_TX_BD_PDU_COUNT:
                format = &spec->bdFormatOptions.pduCount;
                retVal = GetValue(pMac, format, param, &value);
                txBd->pduCount = retVal;
                break;
            case DVT_TX_BD_MPDU_DATA_OFFSET:
                format = &spec->bdFormatOptions.mpduDataOffset;
                retVal = GetValue(pMac, format, param, &value);
                txBd->mpduDataOffset = retVal;
                break;
            case DVT_TX_BD_MPDU_HEADER_OFFSET:
                format = &spec->bdFormatOptions.mpduHeaderOffset;
                retVal = GetValue(pMac, format, param, &value);
                txBd->mpduHeaderOffset = retVal;
                break;
            case DVT_TX_BD_MPDU_HEADER_LENGTH:
                format = &spec->bdFormatOptions.mpduHeaderLength;
                retVal = GetValue(pMac, format, param, &value);
                txBd->mpduHeaderLength = retVal;
                break;
            case DVT_TX_BD_RATE_INDEX:
                format = &spec->bdFormatOptions.rateIndex;
                retVal = GetValue(pMac, format, param, &value);
                txBd->rateIndex = retVal;
                break;
            case DVT_TX_BD_TID:
                format = &spec->bdFormatOptions.tid;
                retVal = GetValue(pMac, format, param, &value);
                txBd->tid = retVal;
                break;
            case DVT_TX_BD_FRAGMENT:
                format = &spec->bdFormatOptions.fragment;
                retVal = GetValue(pMac, format, param, &value);
                txBd->fragment = retVal;
                break;
            case DVT_TX_BD_LAST_FRAGMENT:
                format = &spec->bdFormatOptions.lastFragment;
                retVal = GetValue(pMac, format, param, &value);
                txBd->lastFragment = retVal;
                break;
            case DVT_TX_BD_RESERVED4:
                format = &spec->bdFormatOptions.reserved4;
                retVal = GetValue(pMac, format, param, &value);
                txBd->reserved4 = retVal;
                break;
            case DVT_TX_BD_MPDU_LENGTH:
                format = &spec->bdFormatOptions.mpduLength;
                retVal = GetValue(pMac, format, param, &value);
                txBd->mpduLength = retVal;
                break;
            case DVT_TX_BD_RESERVED5:
                format = &spec->bdFormatOptions.reserved5;
                retVal = GetValue(pMac, format, param, &value);
                txBd->reserved5 = retVal;
                break;
            case DVT_TX_BD_TA_INDEX:
                format = &spec->bdFormatOptions.taIndex;
                retVal = GetValue(pMac, format, param, &value);
                txBd->taIndex = retVal;
                break;
            case DVT_TX_BD_RA_INDEX:
                format = &spec->bdFormatOptions.raIndex;
                retVal = GetValue(pMac, format, param, &value);
                txBd->raIndex = retVal;
                break;
            case DVT_TX_BD_DPU_DESC_IDX:
                format = &spec->bdFormatOptions.dpuDescIdx;
                retVal = GetValue(pMac, format, param, &value);
                txBd->dpuDescIdx = retVal;
                break;
            case DVT_TX_BD_TIMESTAMP:
                format = &spec->bdFormatOptions.timestamp;
                retVal = GetValue(pMac, format, param, &value);
                txBd->timestamp = retVal;
                break;
            case DVT_TX_BD_RESERVED6:
                format = &spec->bdFormatOptions.reserved6;
                retVal = GetValue(pMac, format, param, &value);
                txBd->reserved6 = retVal;
                break;
            case DVT_TX_BD_RETRY_COUNT:
                format = &spec->bdFormatOptions.retryCount;
                retVal = GetValue(pMac, format, param, &value);
                txBd->retryCount = retVal;
                break;

            default:

                break;
        }

        if (retVal != DVT_STATUS_SUCCESS)
        {
            //failure condition
            dvtLog(pMac, LOGE, "dvtFormatFrame Bad Param %d", param);
            return (retVal);
        }

        (tANI_U32)param++; //increment to set next parameter of frame
    }
*/
    //MPDU header and BD now formatted
    //create payload in payloadList
    {
        tANI_U32 i;
        sTestFormat *payloadByte = &spec->payload.payloadByte;
        sHddMemSegment *payloadSeg = payloadList;
        tANI_U32 payloadOffset = 0;                 //next location to write in the current payloadSeg

        for (i = 0; i < length; i++)
        {
            if (payloadOffset < payloadSeg->length)
            {
                //still room to write to this payloadSeg
                assert(payloadSeg != NULL);

                if (GetValue(pMac, payloadByte, DVT_FRAME_PAYLOAD_BYTE, (tANI_U32 *)((tANI_U8 *)(payloadSeg->hddContext) + payloadOffset)) != DVT_STATUS_SUCCESS)
                {
                    return (DVT_STATUS_FAILURE);
                }
                payloadOffset++;
            }
            else
            {
                //move to the next payloadSeg in the list
                payloadSeg = payloadSeg->next;              //this would be NULL if we have just written the last byte of the payload
                payloadOffset = 0;
            }
        }
    }
    
    //need to endian-swap the bd
    {
        assert(mpduHdrOffset % 4 == 0);
        
        SwapBuffer32Endianess((tANI_U32 *)txBd, BD_SIZE);
        
        //reverse swap mpduHeader
        //SwapBuffer32Endianess((tANI_U32 *)mpduHdr, txBd->mpduLength);
    }
    

    return (retVal);
}

void dvtFrameGenCallback(tHalHandle hHal, eDvtTxQueue queue)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hHal;

    if (pMac->dvt.txQueues[NUM_DVT_TX_QUEUES - queue - 1].spec != NULL)
    {
        tANI_U32 framesCreated = 0;
        
        dvtContinueFrameSpec(pMac, pMac->dvt.txQueues[NUM_DVT_TX_QUEUES - queue - 1].spec, &framesCreated);
    }
}

void dvtFrameReceivedCallback(tHalHandle hHal, eDvtRxQueue queue, sRxFrameTransfer *rxFrame)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hHal;
    dvtRxXfrFrames(pMac);
}


tANI_U32 SwapWord32Endianess( tANI_U32 dword )
{
    tANI_U8 * pBuffer;
    tANI_U32   nResult;
    tANI_U8   c, *pResult;

    pBuffer = (tANI_U8 * )&dword;

    assert(pBuffer);

    pResult = (tANI_U8 * )&nResult;

    c = ((tANI_U8 * )pBuffer)[ 0 ];
    ((tANI_U8 * )pResult)[ 0 ] = ((tANI_U8 * )pBuffer)[ 3 ];
    ((tANI_U8 * )pResult)[ 3 ] = c;

    c = ((tANI_U8 * )pBuffer)[ 1 ];
    ((tANI_U8 * )pResult)[ 1 ] = ((tANI_U8 * )pBuffer)[ 2 ];
    ((tANI_U8 * )pResult)[ 2 ] = c;

    return( nResult );
}

void SwapBuffer32Endianess( tANI_U32 *pU32, tANI_U32 cBytes )
{
    tANI_U32 cU32 = cBytes / sizeof( tANI_U32 );
    while( cU32 -- )
    {
        pU32[ cU32 ] = SwapWord32Endianess( pU32[ cU32 ] );
    }
}


static char paramNames[MAX_DVT_FRAME_OPTION][40] =
{
    "DVT_FRAME_SEGMENTATION",          //DVT_FRAME_SEGMENTATION
    "DVT_FRAME_LENGTH",                //DVT_FRAME_LENGTH
    "DVT_FRAME_PAYLOAD_BYTE",          //DVT_FRAME_PAYLOAD_BYTE
    "DVT_FRAME_PROTOCOL_VER",          //DVT_FRAME_PROTOCOL_VER
    "DVT_FRAME_FRAME_TYPE",            //DVT_FRAME_FRAME_TYPE
    "DVT_FRAME_FRAME_SUBTYPE",         //DVT_FRAME_FRAME_SUBTYPE
    "DVT_FRAME_TO_FROM_DS",            //DVT_FRAME_TO_FROM_DS
    "DVT_FRAME_MORE_FRAG",             //DVT_FRAME_MORE_FRAG
    "DVT_FRAME_RETRY",                 //DVT_FRAME_RETRY
    "DVT_FRAME_PWR_MGMT",              //DVT_FRAME_PWR_MGMT
    "DVT_FRAME_MORE_DATA",             //DVT_FRAME_MORE_DATA
    "DVT_FRAME_PRIVACY",               //DVT_FRAME_PRIVACY
    "DVT_FRAME_DURATION_ID",           //DVT_FRAME_DURATION_ID
    "DVT_FRAME_ADDR1",                 //DVT_FRAME_ADDR1
    "DVT_FRAME_ADDR2",                 //DVT_FRAME_ADDR2
    "DVT_FRAME_ADDR3",                 //DVT_FRAME_ADDR3
    "DVT_FRAME_ADDR4",                 //DVT_FRAME_ADDR4
    "DVT_FRAME_FRAG_NUM",              //DVT_FRAME_FRAG_NUM
    "DVT_FRAME_SEQUENCE_NUM",          //DVT_FRAME_SEQUENCE_NUM
    "DVT_FRAME_QOS_TID",               //DVT_FRAME_QOS_TID
    "DVT_FRAME_QOS_RSVD",              //DVT_FRAME_QOS_RSVD
    "DVT_FRAME_QOS_ACK_POLICY",        //DVT_FRAME_QOS_ACK_POLICY
    "DVT_FRAME_RSVD",                  //DVT_FRAME_RSVD
    "DVT_FRAME_QOS_EOSP",              //DVT_FRAME_QOS_EOSP
    "DVT_FRAME_QOS_TXOP_LIMIT",        //DVT_FRAME_QOS_TXOP_LIMIT
    "DVT_TX_BD_BDT",                   //DVT_TX_BD_BDT
    "DVT_TX_BD_DPU_NC",                //DVT_TX_BD_DPU_NC
    "DVT_TX_BD_DPU_NE",                //DVT_TX_BD_DPU_NE
    "DVT_TX_BD_SW_BD_TYPE",            //DVT_TX_BD_SW_BD_TYPE
    "DVT_TX_BD_RESERVED1",             //DVT_TX_BD_RESERVED1
    "DVT_TX_BD_STA_SIGNATURE",         //DVT_TX_BD_STA_SIGNATURE
    "DVT_TX_BD_DPU_SIGNATURE",         //DVT_TX_BD_DPU_SIGNATURE
    "DVT_TX_BD_DPU_RF",                //DVT_TX_BD_DPU_RF
    "DVT_TX_BD_RESERVED3",             //DVT_TX_BD_RESERVED3
    "DVT_TX_BD_ENABLE_AGGREGATE",      //DVT_TX_BD_ENABLE_AGGREGATE
    "DVT_TX_BD_OVERWRITE_PHY",         //DVT_TX_BD_OVERWRITE_PHY
    "DVT_TX_BD_FORCE_MAC_PROT",        //DVT_TX_BD_FORCE_MAC_PROT
    "DVT_TX_BD_FORCE_ACK_POLICY",      //DVT_TX_BD_FORCE_ACK_POLICY
    "DVT_TX_BD_TX_INDICATION",         //DVT_TX_BD_TX_INDICATION
    "DVT_TX_BD_KEEP_SEQ_NUM",          //DVT_TX_BD_KEEP_SEQ_NUM
    "DVT_TX_BD_RESERVED2",             //DVT_TX_BD_RESERVED2
    "DVT_TX_BD_TAIL_PDU_IDX",          //DVT_TX_BD_TAIL_PDU_IDX
    "DVT_TX_BD_HEAD_PDU_IDX",          //DVT_TX_BD_HEAD_PDU_IDX
    "DVT_TX_BD_PDU_COUNT",             //DVT_TX_BD_PDU_COUNT
    "DVT_TX_BD_MPDU_DATA_OFFSET",      //DVT_TX_BD_MPDU_DATA_OFFSET
    "DVT_TX_BD_MPDU_HEADER_OFFSET",    //DVT_TX_BD_MPDU_HEADER_OFFSET
    "DVT_TX_BD_MPDU_HEADER_LENGTH",    //DVT_TX_BD_MPDU_HEADER_LENGTH
    "DVT_TX_BD_RATE_INDEX",            //DVT_TX_BD_RATE_INDEX
    "DVT_TX_BD_TID",                   //DVT_TX_BD_TID
    "DVT_TX_BD_FRAGMENT",              //DVT_TX_BD_FRAGMENT
    "DVT_TX_BD_LAST_FRAGMENT",         //DVT_TX_BD_LAST_FRAGMENT
    "DVT_TX_BD_RESERVED4",             //DVT_TX_BD_RESERVED4
    "DVT_TX_BD_MPDU_LENGTH",           //DVT_TX_BD_MPDU_LENGTH
    "DVT_TX_BD_RESERVED5",             //DVT_TX_BD_RESERVED5
    "DVT_TX_BD_TA_INDEX",              //DVT_TX_BD_TA_INDEX
    "DVT_TX_BD_RA_INDEX",              //DVT_TX_BD_RA_INDEX
    "DVT_TX_BD_DPU_DESC_IDX",          //DVT_TX_BD_DPU_DESC_IDX
    "DVT_TX_BD_TIMESTAMP",             //DVT_TX_BD_TIMESTAMP
    "DVT_TX_BD_RESERVED6",             //DVT_TX_BD_RESERVED6
    "DVT_TX_BD_RETRY_COUNT",           //DVT_TX_BD_RETRY_COUNT

};

static eANI_DVT_STATUS GetValue(tpAniSirGlobal pMac, sTestFormat *format, eFrameOptions param, tANI_U32 *value)
{
    switch (format->option)
    {
        case FORMAT_RANDOMIZE:
            if (((format->params.rand.min >= pMac->dvt.frameBounds[param].min) && (format->params.rand.min <= pMac->dvt.frameBounds[param].max)) &&
                ((format->params.rand.max >= pMac->dvt.frameBounds[param].min) && (format->params.rand.max <= pMac->dvt.frameBounds[param].max))
               )
            {
                *value = ((tANI_U32)rand() / (format->params.rand.max - format->params.rand.min))
                            + 1 + format->params.rand.min;
            }
            else
            {
                //parameter out of bounds
                dvtLog(pMac, LOGW, "Random FrameSpec Param %s OUT OF BOUNDS\n",
                        &paramNames[param][0]
                      );
                dvtLog(pMac, LOGW, "\tBounds: min:%d max:%d  Spec:min:%d max:%d\n\n",
                        pMac->dvt.frameBounds[param].min,
                        pMac->dvt.frameBounds[param].max,
                        format->params.rand.min,
                        format->params.rand.max
                      );
                return (DVT_STATUS_FAILURE);
            }
            break;

        case FORMAT_SEQUENTIAL:
            if (((format->params.seq.min >= pMac->dvt.frameBounds[param].min) && (format->params.seq.min <= pMac->dvt.frameBounds[param].max)) &&
                ((format->params.seq.max >= pMac->dvt.frameBounds[param].min) && (format->params.seq.max <= pMac->dvt.frameBounds[param].max))
               )
            {
                *value = pMac->dvt.frameBounds[param].last;

                pMac->dvt.frameBounds[param].last++;

                if (pMac->dvt.frameBounds[param].last > format->params.seq.max)
                {
                    pMac->dvt.frameBounds[param].last = format->params.seq.min;
                }
            }
            else
            {
                //parameter out of bounds
                dvtLog(pMac, LOGW, "Seq FrameSpec Param %s OUT OF BOUNDS\n",
                        &paramNames[param][0]
                      );
                dvtLog(pMac, LOGW, "\tBounds: min:%d max:%d  Spec:min:%d max:%d\n\n",
                        pMac->dvt.frameBounds[param].min,
                        pMac->dvt.frameBounds[param].max,
                        format->params.seq.min,
                        format->params.seq.max
                      );
                return (DVT_STATUS_FAILURE);
            }
            break;

        case FORMAT_FIXED_VALUE:
            if (((format->params.fixed.value >= pMac->dvt.frameBounds[param].min) && (format->params.fixed.value <= pMac->dvt.frameBounds[param].max)))
            {
                *value = format->params.fixed.value;
            }
            else
            {
                //parameter out of bounds
                dvtLog(pMac, LOGW, "Fixed FrameSpec Param %s OUT OF BOUNDS\n",
                        &paramNames[param][0]
                      );
                dvtLog(pMac, LOGW, "\tBounds: min:%d max:%d  Spec:value:%d\n\n",
                        pMac->dvt.frameBounds[param].min,
                        pMac->dvt.frameBounds[param].max,
                        format->params.fixed.value
                      );
                return (DVT_STATUS_FAILURE);
            }
            break;

        case FORMAT_TEMPLATE:
            //TBD - use pointer to get value from other location
            return (DVT_STATUS_FAILURE);
            break;

        case FORMAT_NONE:
            *value = pMac->dvt.frameBounds[param].min;
            break;

        case FORMAT_FROM_SYSTEM:

            break;

        default:
            return (DVT_STATUS_FAILURE);
            break;

    }
    return (DVT_STATUS_SUCCESS);
}


void testDvtCreateFrames(tpAniSirGlobal pMac, tANI_U32 threadCount, tANI_U32 totalCount)
{

    sDvtFrameSpec spec =
    {
        DVT_TX_QUEUE_0,                 //eDvtTxQueue txQueueId;
        1,                             //tANI_U32 totalNumber;
        1,                             //tANI_U32 threadNumber;
        eANI_BOOLEAN_TRUE,              //tANI_BOOLEAN bdPresent;
        SMAC_BMUWQ_MCPU_TX_WQ0,         //eSmacBmuWqId dxeOutWQ;

        {                              //sFrameHdrFormatOptions hdr;
            24,                        //MPDU header length varies with Addr4 and Qos options
            {                          // sTestFormat protocolVer;
                FORMAT_FIXED_VALUE,    //  eTestFormat option;
                {                      //  uTestFormat params, sFixed;
                   0
                }
            },
            {                          // sTestFormat frameType;
                FORMAT_FIXED_VALUE,    //  eTestFormat option;
                {                      //  uTestFormat params, sFixed;
                   0
                }
            },
            {                          // sTestFormat frameSubtype;
                FORMAT_FIXED_VALUE,    //  eTestFormat option;
                {                      //  uTestFormat params, sFixed;
                   0
                }
            },
            {                          // sTestFormat toFromDs;
                FORMAT_FIXED_VALUE,    //  eTestFormat option;
                {                      //  uTestFormat params, sFixed;
                   0
                }
            },
            {                          // sTestFormat moreFrag;
                FORMAT_FIXED_VALUE,    //  eTestFormat option;
                {                      //  uTestFormat params, sFixed;
                   0
                }
            },
            {                          // sTestFormat retry;
                FORMAT_FIXED_VALUE,    //  eTestFormat option;
                {                      //  uTestFormat params, sFixed;
                   0
                }
            },
            {                          // sTestFormat pwrMgmt;
                FORMAT_FIXED_VALUE,    //  eTestFormat option;
                {                      //  uTestFormat params, sFixed;
                   0
                }
            },
            {                          // sTestFormat moreData;
                FORMAT_FIXED_VALUE,    //  eTestFormat option;
                {                      //  uTestFormat params, sFixed;
                   0
                }
            },
            {                          // sTestFormat privacy;
                FORMAT_FIXED_VALUE,    //  eTestFormat option;
                {                      //  uTestFormat params, sFixed;
                   0
                }
            },
            {                          // sTestFormat durationId;
                FORMAT_FIXED_VALUE,    //  eTestFormat option;
                {                      //  uTestFormat params, sFixed;
                   0
                }
            },
            {                          // sTestFormat addr1;
                FORMAT_FIXED_VALUE,    //  eTestFormat option;
                {                      //  uTestFormat params, sFixed;
                   0
                }
            },
            {                          // sTestFormat addr2;
                FORMAT_FIXED_VALUE,    //  eTestFormat option;
                {                      //  uTestFormat params, sFixed;
                   0
                }
            },
            {                          // sTestFormat addr3;
                FORMAT_FIXED_VALUE,    //  eTestFormat option;
                {                      //  uTestFormat params, sFixed;
                   0
                }
            },
            {                          // sTestFormat addr4;
                FORMAT_FIXED_VALUE,    //  eTestFormat option;
                {                      //  uTestFormat params, sFixed;
                   0
                }
            },
            {                          // sTestFormat fragNum;
                FORMAT_FIXED_VALUE,    //  eTestFormat option;
                {                      //  uTestFormat params, sFixed;
                   0
                }
            },
            {                          // sTestFormat sequenceNum;
                FORMAT_FIXED_VALUE,    //  eTestFormat option;
                {                      //  uTestFormat params, sFixed;
                   0
                }
            },
            {                          // sTestFormat qosTID;
                FORMAT_FIXED_VALUE,    //  eTestFormat option;
                {                      //  uTestFormat params, sFixed;
                   0
                }
            },
            {                          // sTestFormat qosRsvd;
                FORMAT_FIXED_VALUE,    //  eTestFormat option;
                {                      //  uTestFormat params, sFixed;
                   0
                }
            },
            {                          // sTestFormat qosAckPolicy;
                FORMAT_FIXED_VALUE,    //  eTestFormat option;
                {                      //  uTestFormat params, sFixed;
                   0
                }
            },
            {                          // sTestFormat Rsvd;
                FORMAT_FIXED_VALUE,    //  eTestFormat option;
                {                      //  uTestFormat params, sFixed;
                   0
                }
            },
            {                          // sTestFormat qosEOSP;
                FORMAT_FIXED_VALUE,    //  eTestFormat option;
                {                      //  uTestFormat params, sFixed;
                   0
                }
            },
            {                          // sTestFormat qosTxopLimit;
                FORMAT_FIXED_VALUE,    //  eTestFormat option;
                {                      //  uTestFormat params, sFixed;
                   0
                }
            },
        },

        {                              //sFramePayloadFormatOptions payload;
            {                          // sTestFormat segmentation;
                FORMAT_FIXED_VALUE,    //  eTestFormat option;
                {                      //  uTestFormat params, sFixed;
                   2000
                }
            },
            {                          // sTestFormat length;
                FORMAT_FIXED_VALUE,    //  eTestFormat option;
                {                      //  uTestFormat params, sFixed;
                   20
                }
            },
            {                          // sTestFormat payloadByte;
                FORMAT_FIXED_VALUE,    //  eTestFormat option;
                {                      //  uTestFormat params, sFixed;
                   0xA5
                }
            },
        },

        {                              //sDvtTxBdFormatOptions bdFormatOptions;
            // sTestFormat bdt;                   //2 bits
            {
                FORMAT_FIXED_VALUE,    //  eTestFormat option;
                {                      //  uTestFormat params, sFixed;
                   0
                }
            },
            // sTestFormat dpuNC;                 //1 bits
            {
                FORMAT_FIXED_VALUE,    //  eTestFormat option;
                {                      //  uTestFormat params, sFixed;
                   0
                }
            },
            // sTestFormat dpuNE;                 //1 bits
            {
                FORMAT_FIXED_VALUE,    //  eTestFormat option;
                {                      //  uTestFormat params, sFixed;
                   0
                }
            },
            // sTestFormat swBdType;              //2 bits
            {
                FORMAT_FIXED_VALUE,    //  eTestFormat option;
                {                      //  uTestFormat params, sFixed;
                   0
                }
            },
            // sTestFormat reserved1;             //10 bits
            {
                FORMAT_FIXED_VALUE,    //  eTestFormat option;
                {                      //  uTestFormat params, sFixed;
                   0
                }
            },
            // sTestFormat staSignature;          //4 bits
            {
                FORMAT_FIXED_VALUE,    //  eTestFormat option;
                {                      //  uTestFormat params, sFixed;
                   0
                }
            },
            // sTestFormat dpuSignature;          //4 bits
            {
                FORMAT_FIXED_VALUE,    //  eTestFormat option;
                {                      //  uTestFormat params, sFixed;
                   0
                }
            },
            // sTestFormat dpuRF;                 //8 bits
            {
                FORMAT_FIXED_VALUE,    //  eTestFormat option;
                {                      //  uTestFormat params, sFixed;
                   0
                }
            },

            //     /* 0x04 */
            //     sTestFormat reserved3;             //16 bits
            {
                FORMAT_FIXED_VALUE,    //  eTestFormat option;
                {                      //  uTestFormat params, sFixed;
                   0
                }
            },
            //     sTestFormat enableAggregate;       //1 bits
            {
                FORMAT_FIXED_VALUE,    //  eTestFormat option;
                {                      //  uTestFormat params, sFixed;
                   0
                }
            },
            //     sTestFormat overwritePhy;          //1 bits
            {
                FORMAT_FIXED_VALUE,    //  eTestFormat option;
                {                      //  uTestFormat params, sFixed;
                   0
                }
            },
            //     sTestFormat forceMacProt;          //2 bits
            {
                FORMAT_FIXED_VALUE,    //  eTestFormat option;
                {                      //  uTestFormat params, sFixed;
                   0
                }
            },
            //     sTestFormat forceAckPolicy;        //3 bits
            {
                FORMAT_FIXED_VALUE,    //  eTestFormat option;
                {                      //  uTestFormat params, sFixed;
                   0
                }
            },
            //     sTestFormat txIndication;          //1 bits
            {
                FORMAT_FIXED_VALUE,    //  eTestFormat option;
                {                      //  uTestFormat params, sFixed;
                   0
                }
            },
            //     sTestFormat keepSeqNum;            //1 bits
            {
                FORMAT_FIXED_VALUE,    //  eTestFormat option;
                {                      //  uTestFormat params, sFixed;
                   0
                }
            },
            //     sTestFormat reserved2;             //7 bits
            {
                FORMAT_FIXED_VALUE,    //  eTestFormat option;
                {                      //  uTestFormat params, sFixed;
                   0
                }
            },
            //
            //     /* 0x08 */
            //     sTestFormat tailPduIdx;            //16 bits
            {
                FORMAT_FIXED_VALUE,    //  eTestFormat option;
                {                      //  uTestFormat params, sFixed;
                   0
                }
            },
            //     sTestFormat headPduIdx;            //16 bits
            {
                FORMAT_FIXED_VALUE,    //  eTestFormat option;
                {                      //  uTestFormat params, sFixed;
                   0
                }
            },
            //
            //     /* 0x0c */
            //     sTestFormat pduCount;              //7 bits
            {
                FORMAT_FIXED_VALUE,    //  eTestFormat option;
                {                      //  uTestFormat params, sFixed;
                   0
                }
            },
            //     sTestFormat mpduDataOffset;        //9 bits
            {
                FORMAT_FIXED_VALUE,    //  eTestFormat option;
                {                      //  uTestFormat params, sFixed;
                   124
                }
            },
            //     sTestFormat mpduHeaderOffset;      //8 bits
            {
                FORMAT_FIXED_VALUE,    //  eTestFormat option;
                {                      //  uTestFormat params, sFixed;
                   0x40
                }
            },
            //     sTestFormat mpduHeaderLength;      //8 bits
            {
                FORMAT_FIXED_VALUE,    //  eTestFormat option;
                {                      //  uTestFormat params, sFixed;
                   24
                }
            },
            //
            //     /* 0x10 */
            //     sTestFormat rateIndex;             //8 bits
            {
                FORMAT_FIXED_VALUE,    //  eTestFormat option;
                {                      //  uTestFormat params, sFixed;
                   208
                }
            },
            //     sTestFormat tid;                   //4 bits
            {
                FORMAT_FIXED_VALUE,    //  eTestFormat option;
                {                      //  uTestFormat params, sFixed;
                   0
                }
            },
            //     sTestFormat fragment;              //1 bits
            {
                FORMAT_FIXED_VALUE,    //  eTestFormat option;
                {                      //  uTestFormat params, sFixed;
                   0
                }
            },
            //     sTestFormat lastFragment;          //1 bits
            {
                FORMAT_FIXED_VALUE,    //  eTestFormat option;
                {                      //  uTestFormat params, sFixed;
                   1
                }
            },
            //     sTestFormat reserved4;             //2 bits
            {
                FORMAT_FIXED_VALUE,    //  eTestFormat option;
                {                      //  uTestFormat params, sFixed;
                   0
                }
            },
            //     sTestFormat mpduLength;            //16 bits
            {
                FORMAT_FIXED_VALUE,    //  eTestFormat option;
                {                      //  uTestFormat params, sFixed;
                   20
                }
            },
            //
            //     /* 0x14 */
            //     sTestFormat reserved5;             //8 bits
            {
                FORMAT_FIXED_VALUE,    //  eTestFormat option;
                {                      //  uTestFormat params, sFixed;
                   0
                }
            },
            //     sTestFormat taIndex;               //8 bits
            {
                FORMAT_FIXED_VALUE,    //  eTestFormat option;
                {                      //  uTestFormat params, sFixed;
                   0
                }
            },
            //     sTestFormat raIndex;               //8 bits
            {
                FORMAT_FIXED_VALUE,    //  eTestFormat option;
                {                      //  uTestFormat params, sFixed;
                   0
                }
            },
            //     sTestFormat dpuDescIdx;            //8 bits
            {
                FORMAT_FIXED_VALUE,    //  eTestFormat option;
                {                      //  uTestFormat params, sFixed;
                   0
                }
            },
            //
            //     /* 0x18 */
            //     sTestFormat timestamp;             //16 bits
            {
                FORMAT_FIXED_VALUE,    //  eTestFormat option;
                {                      //  uTestFormat params, sFixed;
                   0
                }
            },
            //     sTestFormat reserved6;             //8 bits
            {
                FORMAT_FIXED_VALUE,    //  eTestFormat option;
                {                      //  uTestFormat params, sFixed;
                   0
                }
            },
            //     sTestFormat retryCount;            //8 bits
            {
                FORMAT_FIXED_VALUE,    //  eTestFormat option;
                {                      //  uTestFormat params, sFixed;
                   0
                }
            },
        }
    };

    spec.threadNumber = threadCount;
    spec.totalNumber = totalCount;

    if (dvtCreateFrame(pMac, spec) == DVT_STATUS_FAILURE)
    {
        dvtLog(pMac, LOGE, "ERROR: Could not create a frame from the frame spec\n");
        
        dvtAbortFrameSpec(pMac, DVT_TX_QUEUE_0);
        
        
    }
}



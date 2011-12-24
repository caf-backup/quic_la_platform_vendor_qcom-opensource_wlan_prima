/** ------------------------------------------------------------------------- * 
    ------------------------------------------------------------------------- *  

  
    \file dvtQueues.c
  
    \brief frame queueing functions
  
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
#include "dvtModuleApi.h"

static void TxFrameCompleted(tpAniSirGlobal pMac, sTxFrameTransfer *frame);
static eANI_DVT_STATUS txXfrFrame(tpAniSirGlobal pMac, ePipes pipe);
static eANI_DVT_STATUS rxXfrFrame(tpAniSirGlobal pMac);

extern tANI_U32 SwapWord32Endianess( tANI_U32 dword );
extern void SwapBuffer32Endianess( tANI_U32 *pU32, tANI_U32 cBytes );


eANI_DVT_STATUS dvtQueueInit(tpAniSirGlobal pMac)
{
    tANI_U32 q;

    for (q = DVT_TX_QUEUE_0; q < NUM_DVT_TX_QUEUES; q++)
    {
        pMac->dvt.txQueues[q].head = NULL;
        pMac->dvt.txQueues[q].tail = NULL;
        pMac->dvt.txQueues[q].pendingFrames = 0;
        pMac->dvt.txQueues[q].pipe = (ePipes)(q + PIPE_TX_1);
        pMac->dvt.txQueues[q].framesLeftToCreate = 0;
        pMac->dvt.txQueues[q].framesPerThread = 0;
        pMac->dvt.txQueues[q].spec = NULL;
        pMac->dvt.txQueues[q].loopback = DVT_P2P;
        pMac->dvt.txQueues[q].loopbackQueue = (eDvtRxQueue)(q);
        
        pMac->dvt.txQueues[q].callback = dvtFrameGenCallback;          //default callback to generate more frames based on a framespec
    }

    for (q = DVT_RX_QUEUE_0; q < NUM_DVT_RX_QUEUES; q++)
    {
        pMac->dvt.rxQueues[q].head = NULL;
        pMac->dvt.rxQueues[q].tail = NULL;
        pMac->dvt.rxQueues[q].pendingFrames = 0;
        pMac->dvt.rxQueues[q].loopback = DVT_P2P;   //set for P2P operation by default - dvtPipeCfg can change this to a loopback mode
        pMac->dvt.rxQueues[q].callback = dvtAppRxEnqueueCallback;   //for the moment, set the callback on all rx queues to the application packet callbac
    }

    return (DVT_STATUS_SUCCESS);
}

static void TxFrameCompleted(tpAniSirGlobal pMac, sTxFrameTransfer *frame)
{
    //free frame resources and log any corresponding events
    assert(frame->next == NULL);
    assert(frame->bd != NULL);  //must have a BD

    /* if this frame was allocated from the dvtTxRxHeap resources, then we should free the resources.
        Compare the address against the heap handle to determine if it is from our txHeap
    */
    
    if ((frame->bd >= (sHddMemSegment *)pMac->dvt.txHeap.head.virt) && (frame->bd < (sHddMemSegment *)pMac->dvt.txHeap.tail.virt))
    {
        //the BD is from our heap, so free all of the frames resources
        PutHeapBytes(pMac, &pMac->dvt.txHeap, frame->bd);
    
        if (frame->segment)
        {
            PutHeapBytes(pMac, &pMac->dvt.txHeap, frame->segment);
        }

        if (palFreeMemory(pMac->hHdd, frame) != eHAL_STATUS_SUCCESS)
        {
            assert(0);
        }
    }
    else
    {
        //this frame was queued through dvtWritePipeFrameTransfer from the protocol driver
        // which will free the resources when the getInfo message tells it the frames completed
    }
}



eANI_DVT_STATUS dvtTxEnqueueFrame(tpAniSirGlobal pMac, eDvtTxQueue queue, sTxFrameTransfer *frame)
{
    eANI_DVT_STATUS retVal;

    //dvtLog(pMac, LOGE, "DEBUG: dvtTxEnqueueFrame\n");

    assert(queue < NUM_DVT_TX_QUEUES);
    assert(frame != NULL);
    assert(frame->next == NULL);

    if (pMac->dvt.txQueues[queue].head == NULL)
    {
        //first frame in queue
        assert(pMac->dvt.txQueues[queue].tail == NULL);

        pMac->dvt.txQueues[queue].head = frame;
        pMac->dvt.txQueues[queue].tail = frame;
        pMac->dvt.txQueues[queue].pendingFrames = 1;
    }
    else
    {
        assert(pMac->dvt.txQueues[queue].tail->next == NULL);

        pMac->dvt.txQueues[queue].tail->next = frame;
        pMac->dvt.txQueues[queue].tail = frame;
        pMac->dvt.txQueues[queue].pendingFrames++;
    }

    //attempt to transfer on pipe assigned to this queue
    //dvtLog(pMac, LOGE, "DEBUG: calling txXfrFrame in dvtTxEnqueueFrame\n");
    retVal = txXfrFrame(pMac, pMac->dvt.txQueues[queue].pipe);

    return (retVal);
}

eHalStatus dvtTxDequeueFrame(tHalHandle hHal, ePipes pipe, uFrameTransfer *uframe)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hHal;
    eANI_DVT_STATUS retVal;
    sTxFrameTransfer *frame = (sTxFrameTransfer *)uframe;
    sTxFrameTransfer *nextFrame = frame;
    assert(uframe != NULL);
    
    //dvtLog(pMac, LOGE, "DEBUG: calling txXfrFrame in dvtTxDequeueFrame on txFrame %p\n", frame);
    retVal = txXfrFrame(pMac, pipe);    //attempt to send next frame

    while (nextFrame != NULL)
    {
        nextFrame = frame->next;
        frame->next = NULL;
        TxFrameCompleted(pMac, frame);      //release frame resources
    }

    //dvtLog(pMac, LOGE, "DEBUG: calling dvtContinueFrameSpec in dvtTxDequeueFrame on txFrame %p\n", frame);

    {
        tANI_U32 q = (eDvtTxQueue)((tANI_S32)pipe - PIPE_TX_1 + DVT_TX_QUEUE_0);

/*
         //we should have some more resources available, make callbacks in priority order
         for (q = DVT_TX_QUEUE_0; q < NUM_DVT_TX_QUEUES; q++)
         {
             tANI_U32 queue = NUM_DVT_TX_QUEUES - q - 1;
             pMac->dvt.txQueues[queue].callback(hHal, queue);
         }
*/
         //for the moment just callback the queue that completed
         pMac->dvt.txQueues[q].callback(hHal, q);
    }

    if (retVal == DVT_STATUS_SUCCESS)
    {
        //dvtLog(pMac, LOGE, "DEBUG: dvtTxDequeueFrame finished with txFrame %p\n", frame);
        return (eHAL_STATUS_SUCCESS);
    }
    else
    {
        //dvtLog(pMac, LOGE, "ERROR: failed to txCfrFrame from pipe %d\n", pipe);
        return (eHAL_STATUS_FAILURE);
    }
}




eHalStatus dvtRxEnqueueFrame(tHalHandle hHal, ePipes pipe, uFrameTransfer *uframe)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hHal;
    sRxFrameTransfer *frame = (sRxFrameTransfer *)uframe;

    //initially just use a single queue to receive frames
    eDvtRxQueue queue;
    
    
    queue = (eDvtRxQueue)((tANI_U32)pipe - PIPE_RX_1);

    {
        //this is only designed to receive one frame at a time, not a list
        assert(frame != NULL);
        assert(frame->next == NULL);

        //DEBUG code to verify that we are sending out what is expected
//         if (pMac->dvt.rxQueues[queue].loopback == DVT_DXE_LOOPBACK)
//         {
//             //should be able to compare the bd transmitted agains that received
//             assert(memcmp(pMac->dvt.txQueues[(eDvtTxQueue)queue].curBd, frame->rxFrame->pBbuffer, BD_SIZE) == 0);
//         }

        //swap the BD back around so we can read it
        //SwapBuffer32Endianess((tANI_U32 *)frame->rxFrame->pBbuffer, BD_SIZE);


        if (pMac->dvt.rxQueues[queue].head == NULL)
        {
            //first frame in queue
            assert(pMac->dvt.rxQueues[queue].tail == NULL);

            pMac->dvt.rxQueues[queue].head = frame;
            pMac->dvt.rxQueues[queue].tail = frame;

            assert(pMac->dvt.rxQueues[queue].pendingFrames == 0);
            pMac->dvt.rxQueues[queue].pendingFrames = 1;
        }
        else
        {
            assert(pMac->dvt.rxQueues[queue].tail->next == NULL);

            pMac->dvt.rxQueues[queue].tail->next = frame;
            pMac->dvt.rxQueues[queue].tail = frame;
            pMac->dvt.rxQueues[queue].pendingFrames++;
        }

        //notify that a frame is pending but do not dequeue it
        //if the callback processes the frame in this context, 
        // it will dequeue it through dvtRxDequeueFrame
        pMac->dvt.rxQueues[queue].callback(hHal, queue, frame);
    }

    assert(frame != NULL);
    assert(frame->next == NULL);
    
    return (eHAL_STATUS_SUCCESS);
}


eANI_DVT_STATUS dvtRxDequeueFrame(tpAniSirGlobal pMac, eDvtRxQueue queue, sRxFrameTransfer *frame)
{
    eHalStatus retVal;

    assert(frame != NULL);
    assert(frame->next == NULL);
    
    //the frame must belong to this queue and be the head
    assert (pMac->dvt.rxQueues[queue].head == frame);
    assert (pMac->dvt.rxQueues[queue].tail != NULL);

    //update queue head to next frame, which may be NULL
    //then decouple the frame from the queue
    //then free the frame resource to pal
    pMac->dvt.rxQueues[queue].head = pMac->dvt.rxQueues[queue].head->next;
    if (pMac->dvt.rxQueues[queue].head == NULL)
    {
        //nothing left in queue, set tail to NULL
        pMac->dvt.rxQueues[queue].tail = NULL;
    }
    frame->next = NULL;

    if ((retVal = palFreeRxFrame(pMac->hHdd, frame)) != eHAL_STATUS_SUCCESS)
    {
        assert(0);
        return (DVT_STATUS_FAILURE);
    }
    
    pMac->dvt.rxQueues[queue].pendingFrames--;
    pMac->dvt.rxQueues[queue].receivedFrames++;
    dvtLog(pMac, LOG2, "rx queue %d: pending = %d, received = %d\n", queue, 
            pMac->dvt.rxQueues[queue].pendingFrames, pMac->dvt.rxQueues[queue].receivedFrames);

    return (DVT_STATUS_SUCCESS);
}


//INPUT pipe - a specific pipe to transfer on or ANY_PIPE, meaning we may try all pipes in priority order.
//!! Currently, just implemented a specific pipe. Fill in later as this is verified.
static eANI_DVT_STATUS txXfrFrame(tpAniSirGlobal pMac, ePipes pipe)
{
    eHalStatus retVal;
    tANI_U32 q;

    assert(pipe <= ANY_PIPE);

    if (pipe == ANY_PIPE)
    {
    }
    else
    {
        //send on specific pipe
        //search queues in priority order looking for frames to be sent on this pipe
        for (q = DVT_TX_QUEUE_0; q < NUM_DVT_TX_QUEUES; q++)
        {
            tANI_U32 queue = NUM_DVT_TX_QUEUES - q - 1;

            if (pMac->dvt.txQueues[queue].pipe == pipe)
            {
                while (pMac->dvt.txQueues[queue].head != NULL)
                {
                    //we must unlink the frame and attempt it, so move the head to next
                    sTxFrameTransfer *frame = pMac->dvt.txQueues[queue].head;
                    pMac->dvt.txQueues[queue].head = pMac->dvt.txQueues[queue].head->next;  //might be NULL

                    frame->next = NULL;
                    
                    //DEBUG code to verify that we are sending out what is expected
                    //memcpy(pMac->dvt.txQueues[queue].curBd, frame->bd->hddContext, BD_SIZE);
                    //memcpy(pMac->dvt.txQueues[queue].curPayload, frame->segment->hddContext, frame->segment->length);

#if !defined(ANI_BUS_TYPE_USB)

					// Put the SW Timestamp in the BD
					if(pMac->dvt.dxeTimestampEnable)
					{
						tSmacBdHostTx *pBD = (tSmacBdHostTx *)frame->bd->hddContext;
						palReadRegister(pMac->hHdd, DXE_0_TIMESTAMP_REG, &pBD->swTimestamp);
					}
#endif

                    if (eHAL_STATUS_SUCCESS != (retVal = palWriteFrame(pMac->hHdd, pipe, frame)))
                    {
                        if (retVal == eHAL_STATUS_DXE_FAILED_NO_DESCS)
                        {
                            //pipe could not accept a new frame
                            //put frame back on queue and point head back to it
                            dvtLog(pMac, LOGE, "\t\tpalWriteFrame failed because there were no DXE descriptors on pipe %d\n", pipe);
                            frame->next = pMac->dvt.txQueues[queue].head;
                            pMac->dvt.txQueues[queue].head = frame;

                            return (DVT_STATUS_SUCCESS);
                        }
                        else
                        {
                            //some other type of failure
                            assert(0);
                            return (DVT_STATUS_FAILURE);
                        }
                    }
                    else
                    {
                        //pipe accepted frame, remove it from txQueue
                        //dvtLog(pMac, LOGE, "DEBUG: palWriteFrame success on pipe %d\n", pipe);
                        if (pMac->dvt.txQueues[queue].head == NULL)
                        {
                            //that was the last frame in this queue, set both head and tail back to NULL
                            pMac->dvt.txQueues[queue].tail = NULL;
                        }
                        pMac->dvt.txQueues[queue].pendingFrames--;
                        pMac->dvt.txQueues[queue].sentFrames++;
                    }
                }
                assert(pMac->dvt.txQueues[queue].pendingFrames == 0);
            }
        }
    }

    return (DVT_STATUS_SUCCESS);
}



eANI_DVT_STATUS dvtRxXfrFrames(tpAniSirGlobal pMac)
{
    tANI_U32 q;

    //send on specific pipe
    //search queues in priority order looking for frames to be sent on this pipe
    for (q = DVT_RX_QUEUE_0; q < NUM_DVT_RX_QUEUES; q++)
    {
        tANI_U32 queue = NUM_DVT_RX_QUEUES - q - 1;

        while (pMac->dvt.rxQueues[queue].head != NULL)
        {
            sRxFrameTransfer *frame = pMac->dvt.rxQueues[queue].head;
            
            //process received frame here and log appropriate frame recieved event
            if (pMac->dvt.rxQueues[queue].loopback == DVT_DXE_LOOPBACK)
            {
                //transmit BD, not receive BD
                tSmacBdHostTx *txBd = (tSmacBdHostTx *)(frame->rxFrame->pBbuffer);
                tANI_U8 mpduHdrOffset;
                tANI_U16 mdpuDataOffset;
                tANI_U16 mpduLength;
                sMPDUHeader *mpduHdr;
                tANI_U8 *srcAddr;
                tANI_U8 *destAddr;
                tANI_U8 *txAddr;
                tANI_U8 *bssIdAddr;

                assert(frame->rxFrame != NULL);
                assert(frame->rxFrame->pBbuffer != NULL);
                assert((tANI_U32)txBd % 4 == 0);
                assert(frame->rxFrame->nBytes >= BD_SIZE);

                mpduHdrOffset = (tANI_U8)(txBd->mpduHeaderOffset);
                mdpuDataOffset = (tANI_U16)(txBd->mpduDataOffset);
                mpduLength = (tANI_U16)(txBd->mpduLength);

                assert(mpduHdrOffset < (255 - sizeof(uMpduHeader)));
                assert(mdpuDataOffset <= 255); //data cannot start beyond the second PDU
                assert(mpduLength < PAL_RX_BUFFER_SIZE);

                mpduHdr = (sMPDUHeader *)((tANI_U8 *)txBd + mpduHdrOffset);
                assert((tANI_U32)mpduHdr % 4 == 0);

                srcAddr = GET_SRC_ADDR_PTR(mpduHdr);

                dvtLog(pMac, LOGW, "Frame Rx on Q=%d of length=%d from mac=%X %X %X %X %X %X\n",
                        queue, mpduLength,
                        srcAddr[0],
                        srcAddr[1],
                        srcAddr[2],
                        srcAddr[3],
                        srcAddr[4],
                        srcAddr[5]
                      );
            }
            else if (pMac->dvt.rxQueues[queue].loopback == DVT_DPU_LOOPBACK)
            {

            }
            else if (pMac->dvt.rxQueues[queue].loopback == DVT_MLC_LOOPBACK)
            {

            }
            else if (pMac->dvt.rxQueues[queue].loopback == DVT_P2P)
            {

            }
            else
            {
                assert(0);
            }

            //unlink the frame by moving the head to the next frame
            pMac->dvt.rxQueues[queue].head = pMac->dvt.rxQueues[queue].head->next;  //might be NULL
            frame->next = NULL;

            if (DVT_STATUS_SUCCESS != dvtRxDequeueFrame(pMac, queue, frame))
            {
                assert(0);
                return (DVT_STATUS_FAILURE);
            }
        }
    }
    return (DVT_STATUS_SUCCESS);
}

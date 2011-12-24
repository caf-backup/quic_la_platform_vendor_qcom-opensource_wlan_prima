/** ------------------------------------------------------------------------- * 
    ------------------------------------------------------------------------- *  

  
    \file halTx.h
  
    \brief Hal transmit queuing to allow prioritization of traffic through pal pipes
  
    $Id$ 
  
  
    Copyright (c) 2011 Qualcomm Atheros, Inc. 
    All Rights Reserved. 
    Qualcomm Atheros Confidential and Proprietary. 
  
    Copyright (C) 2006 Airgo Networks, Incorporated
  
   ========================================================================== */
#ifdef FIXME_GEN6 //Valid only for generations before GEN6

#ifndef HALTX_H
#define HALTX_H

#include "palTypes.h"

//fptr called when frame transfer is completed, resources can be freed
typedef void (*hddTransferCallback)(tHddHandle hHdd, sHddMemSegment *frameContents, sHddMemSegment *bd);


typedef struct
{
    uFrameTransfer *head;
    uFrameTransfer *tail;
    struct tagHalQueue *next;           //to create linked list of queues
    tANI_U32 nFramesQueued;             //number of frames enqueued
    tANI_U32 nFramesSent;               //number of frames sent to pipe
    tANI_U32 nFramesXfrComplete;        //number of frames sent to pipe
    hddTransferCallback callback;      //called after hal dequeues a frame.
}sTxQueue;

//  halTx.c functions....
eHalStatus halAddTxQueue(tHalHandle, sTxQueue *txQueue);
eHalStatus halRemoveTxQueue(tHalHandle, sTxQueue *txQueue);
eHalStatus halTxEnqueueFrame(tHalHandle, sTxQueue *txQueue, sHddMemSegment *frameContents, sHddMemSegment *bd);
eHalStatus halTxDequeueFrame(tHalHandle, ePipes pipe, uFrameTransfer *frame);
eHalStatus halTxXfrFrame(tHalHandle, ePipes pipe);

#endif
#endif /* #ifdef FIXME_GEN6*/

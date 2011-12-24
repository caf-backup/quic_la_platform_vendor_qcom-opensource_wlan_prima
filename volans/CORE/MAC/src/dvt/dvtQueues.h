

/** ------------------------------------------------------------------------- * 
    ------------------------------------------------------------------------- *  

  
    \file dvtQueues.h
  
    \brief queue types
  
    $Id$ 
  
    Copyright (c) 2011 Qualcomm Atheros, Inc. 
    All Rights Reserved. 
    Qualcomm Atheros Confidential and Proprietary.  

    Copyright (C) 2006 Airgo Networks, Incorporated
  
   ========================================================================== */

#ifndef DVTQUEUES_H
#define DVTQUEUES_H

#include "palPipes.h"


//Queue definitions
typedef enum
{
    DVT_RX_QUEUE_0,
    DVT_RX_QUEUE_1,
    DVT_RX_QUEUE_2,
    DVT_RX_QUEUE_3,
    DVT_RX_QUEUE_4,
    DVT_RX_QUEUE_5,
    DVT_RX_QUEUE_6,
    DVT_RX_QUEUE_TXOP,
    
    NUM_DVT_RX_QUEUES,
    ANY_DVT_RX_QUEUE
}eDvtRxQueue;

typedef enum
{
    DVT_TX_QUEUE_0,
    DVT_TX_QUEUE_1,
    DVT_TX_QUEUE_2,
    DVT_TX_QUEUE_3,
    DVT_TX_QUEUE_4,
    DVT_TX_QUEUE_5,
    DVT_TX_QUEUE_6,
    DVT_TX_QUEUE_TXOP,
    
    NUM_DVT_TX_QUEUES,
    ANY_DVT_TX_QUEUE
}eDvtTxQueue;

typedef enum
{
    DVT_DXE_LOOPBACK,
    DVT_DPU_LOOPBACK,
    DVT_MLC_LOOPBACK,
    DVT_P2P
}eDvtLoopbackOptions;

typedef void (*tTxQueueCallback)(tHalHandle hHal, eDvtTxQueue queue);

typedef struct
{
    sTxFrameTransfer *head;
    sTxFrameTransfer *tail;
    tANI_U32 pendingFrames;             //number of frames waiting to be sent
    tANI_U32 sentFrames;                //number of frames waiting to be sent
    ePipes pipe;                        //pipe that should be used for this queue
    tANI_U32 framesLeftToCreate;        //number of frames left to create on this queue per command
    tANI_U32 framesPerThread;           //don't make more than this at any one time
    eDvtLoopbackOptions loopback;
    eDvtRxQueue loopbackQueue;          //if this is looped back, this says which rx queue will get the frame
    void *spec;                         //sDvtFrameSpec
    tTxQueueCallback callback;          //callback gets called when frames are dequeued
//     tANI_U8 curBd[BD_SIZE];
//     tANI_U8 curPayload[MAX_TX_PAYLOAD];

}sDvtTxQueue;

typedef void (*tRxQueueCallback)(tHalHandle hHal, eDvtRxQueue queue, sRxFrameTransfer *rxFrame);

typedef struct
{
    sRxFrameTransfer *head;
    sRxFrameTransfer *tail;
    tANI_U32 pendingFrames;         //number of frames waiting to be received
    tANI_U32 receivedFrames;        //number of frames waiting to be sent
    eDvtLoopbackOptions loopback;   //this tells the receive queue if it is expecting loopback formatted frames
    tRxQueueCallback callback;      //callback gets called when frames are queued
}sDvtRxQueue;



#endif


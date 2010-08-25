/** ------------------------------------------------------------------------- * 
    ------------------------------------------------------------------------- *  

  
    \file halRx.h
  
    \brief Hal receive queuing to prioritize and to handle TXOP bursts
  
    $Id$ 
  
    Copyright (C) 2006 Airgo Networks, Incorporated
  
  
   ========================================================================== */
#ifdef FIXME_GEN6 //Valid only for generations before GEN6

#ifndef HALRX_H
#define HALRX_H


#include "halTypes.h"

typedef void (*hddRxProcessCallback)(tHddHandle);



typedef struct tagRxQueue
{
    hddRxProcessCallback rxFPtr;        //HDD function to call for queue processing
    tANI_U32 nFrameNotify;              //number of frames to accumulate in this queue before calling hddRxProcessCallback
    tANI_U32 timeoutNotify;             //notification timeout since head frame was received - expiration causes us to call the hddRxProcessCallback
    tANI_U32 nFrames;                   //number of frames queued
    uFrameTransfer *head;
    uFrameTransfer *tail;
    uFrameTransfer *nextRequest;        //next frame to transfer to the HDD layer via rxFptr
    struct tagRxQueue *next;            //to create linked list of queues
}sRxQueue;

typedef struct tagHalRxFrame
{
    sRxFrameTransfer *rxBuffer;          //raw receive buffer location
    //what contents does the HDD layer need?
}sHalRxFrame;


typedef struct sRxBdPhyStats //RXA
{
    tANI_U8 rssi0_fir;     //Byte 0
    tANI_U8 rssi1_fir;     //Byte 1
    tANI_U8 rssi2_fir;     //Byte 2
    tANI_U8 rx_gain0;     //Byte 3    //rx_gain0,rx_gain1[5:4] (msb 2 bits of rxgain1 on lsb of the stat byte, rxgain0 is on the msbs
    tANI_U8 rx_gain1;     //Byte 4    //rx_gain1[3:0],rx_gain2[5:2] (interprete similarly)
    tANI_U8 rx_gain2;     //Byte 5   //rx_gain2[1:0],6'b0  (-do-)
    tANI_U8 rssi0_adc;     //Byte 6
    tANI_U8 rssi1_adc;     //Byte 7
}tRxBdPhyStats, *tpRxBdPhyStats;


#define HAL_IS_FIRST_MPDU_IN_AMPDU(pFrameInfo) (((pFrameInfo)->rxpFlags) & SMAC_HWBD_RXFLAG_IS_AMPDU_FIRST)
#define HAL_IS_MPDU_IN_AMPDU(pFrameInfo) (((pFrameInfo)->rxpFlags) & SMAC_HWBD_RXFLAG_IS_AMPDU)

//halRx.c functions....
eHalStatus halAddRxQueue(tHalHandle, sRxQueue *rxQueue);
eHalStatus halRemoveRxQueue(tHalHandle, sRxQueue *rxQueue);
eHalStatus halRxEnqueueFrames(tHalHandle, uFrameTransfer *frames);
eHalStatus halRxDequeueFrame(tHalHandle, uFrameTransfer *frame);
eHalStatus halRxXfrFrame(tHalHandle, sHalRxFrame *frame);


#endif
#endif //#ifdef FIXME_GEN6


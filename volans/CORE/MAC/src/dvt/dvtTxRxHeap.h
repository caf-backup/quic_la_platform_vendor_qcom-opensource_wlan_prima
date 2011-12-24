/** ------------------------------------------------------------------------- * 
    ------------------------------------------------------------------------- *  

  
    \file dvtTxRxHeap.h
  
    \brief definitions to support the queuing of tx frames
  
    $Id$ 
  
    Copyright (c) 2011 Qualcomm Atheros, Inc. 
    All Rights Reserved. 
    Qualcomm Atheros Confidential and Proprietary.  

    Copyright (C) 2006 Airgo Networks, Incorporated
  
   ========================================================================== */

#ifndef DVTTXRXHEAP_H
#define DVTTXRXHEAP_H


#include "palPipes.h"


typedef struct
{
    tANI_U8 *virt;
    tANI_U8 *phys;
}sDvtSharedMem;

typedef enum
{
    DVT_HEAP_BYTE_ACCESS = 0,   //all accesses deal in bytes
    DVT_HEAP_DWORD_ACCESS = 4   //all accesses deal in DWORDs
}eDvtHeapAccess;



typedef struct
{
    void *poolHandle;
    sHddMemSegment *segPoolHead;
    sHddMemSegment *segPoolTail;
    tANI_U32 freeSegs;
    tANI_U32 usedSegs;
}sSegPoolQueue;

typedef struct
{
    //hPalSharedMemAllocation *handle;
    void *handle;
    sDvtSharedMem head;
    sDvtSharedMem tail;
    tANI_U32 freeBytes;
    tANI_U32 usedBytes;
    sHddMemSegment *freeList;
    sSegPoolQueue segPool;
}sDvtHeapResources;




#endif


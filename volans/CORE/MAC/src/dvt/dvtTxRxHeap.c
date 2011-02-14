/** ------------------------------------------------------------------------- *
    ------------------------------------------------------------------------- *


    \file dvtTxRxHeap.c

    \brief Implementation of queuing frames for DVT transmit

    $Id$

    Copyright (C) 2006 Airgo Networks, Incorporated


   ========================================================================== */

#include <stdlib.h>
#include <string.h>
#include "ani_assert.h"
#include "palApi.h"
#include "aniGlobal.h"
#include "dvtDebug.h"
#include "dvtTxRxHeap.h"
#include "dvtModule.h"
#include "dvtModuleApi.h"
#include "logDump.h"


#define SEG_GRANULARITY (20)
#define SEG_POOL_SIZE(numBytes) (2 * (numBytes / SEG_GRANULARITY) + 1)

#define FREE_SEGMENT_RECLAIM_THRESHOLD  (10)
#define FREE_BYTE_RECLAIM_THRESHOLD     (20 * 1024)

static sHddMemSegment *AllocSegmentFromPool(sSegPoolQueue *segPool);
static void FreeSegmentToPool(sSegPoolQueue *segPool, sHddMemSegment *freeSeg);
static void HeapGarbageCleanup(tpAniSirGlobal pMac, sDvtHeapResources *heap);
static sHddMemSegment *SplitFreeSegment(sDvtHeapResources *heap, sHddMemSegment *freeSeg, sHddMemSegment *prevFreeSeg, tANI_U32 numBytes);

//#define TX_HEAP_SIZE (1000*1024)    //1MB should do it
#define TX_HEAP_SIZE (100*1024)    //Reduced heap size for Tx since we are not using it anyway
								   // and a larger heap size memory allocation sometimes fails during
								   // driver initialization.
#define RX_HEAP_SIZE (100*1024)     //Right now, the wni_prot5 driver is allocating the shared memory for receive resources - shouldn't need this heap for now.

eHalStatus dvtOpen(tHalHandle hHal);
eHalStatus dvtClose(tHalHandle hHal);


eHalStatus dvtOpen(tHalHandle hHal)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hHal;

    if (dvtAllocTxRxHeaps(pMac, TX_HEAP_SIZE, RX_HEAP_SIZE) == DVT_STATUS_SUCCESS)
    {
        if (dvtQueueInit(pMac) == DVT_STATUS_SUCCESS)
        {
            dvtFrameBoundsInit(pMac);
			dvtDumpInit(hHal);
            return (eHAL_STATUS_SUCCESS);
        }
        else
        {
            return (eHAL_STATUS_FAILURE);
        }
    }
    else
    {
        return (eHAL_STATUS_FAILURE);
    }
}


eHalStatus dvtClose(tHalHandle hHal)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hHal;

    dvtFreeHeap(pMac, &pMac->dvt.txHeap);
    dvtFreeHeap(pMac, &pMac->dvt.rxHeap);
    return (eHAL_STATUS_SUCCESS);
}

eANI_DVT_STATUS dvtAllocTxRxHeaps(tpAniSirGlobal pMac, tANI_U32 txQueueHeapBytes, tANI_U32 rxQueueHeapBytes)
{
    if (eHAL_STATUS_SUCCESS != dvtAllocHeap(pMac, &pMac->dvt.txHeap, txQueueHeapBytes))
    {
        dvtLog(pMac, LOGE, "ERROR: could not allocate Tx heap!");
        return (DVT_STATUS_FAILURE);
    }
    else
    {
        if (eHAL_STATUS_SUCCESS != dvtAllocHeap(pMac, &pMac->dvt.rxHeap, rxQueueHeapBytes))
        {
            //since we couldn't get all the heaps then free the heap just allocated
            dvtFreeHeap(pMac, &pMac->dvt.txHeap);
            dvtLog(pMac, LOGE, "ERROR: could not allocate Rx heap!");
            return (DVT_STATUS_FAILURE);
        }
        else
        {
            return (DVT_STATUS_SUCCESS);
        }
    }
}


eANI_DVT_STATUS dvtAllocHeap(tpAniSirGlobal pMac, sDvtHeapResources *heap, tANI_U32 numBytes)
{
    tANI_U8 *virt = NULL;
    tANI_BUFFER_ADDR phys=0;
    hPalSharedMemAllocation phSharedMem = NULL;
    sSegPoolQueue *segPool = &heap->segPool;
    tANI_U32 numSegs = SEG_POOL_SIZE(numBytes);

    //allocate and link a segment pool to track heap fragments
    if (eHAL_STATUS_SUCCESS == palAllocateMemory(pMac->hHdd, &(segPool->poolHandle), numSegs * sizeof(sHddMemSegment)))
    {
        if (segPool->poolHandle != NULL)
        {
            sHddMemSegment *seg;
        
            segPool->segPoolHead = segPool->poolHandle;

            //loop through allocated memory creating segment pool for heap
            for (seg = segPool->segPoolHead; seg < (segPool->segPoolHead + numSegs); seg++)
            {
                seg->next = seg + 1;
                seg->addr = NULL;
                seg->hddContext = NULL;
                seg->length = 0;
            }

            segPool->segPoolTail = segPool->segPoolHead + numSegs - 1;
            segPool->freeSegs = numSegs;
            segPool->usedSegs = 0;
        }
        else
        {
            //couldn't allocate segment pool for heap
            dvtLog(pMac, LOGE, "ERROR: could not allocate segment pool for heap!");
            return (DVT_STATUS_FAILURE);
        }
    }
    else
    {
        //couldn't allocate segment pool for heap
        dvtLog(pMac, LOGE, "ERROR: could not allocate segment pool for heap!");
        return (DVT_STATUS_FAILURE);
    }

    //allocate heap space from physical contiguous memory
#if defined(ANI_BUS_TYPE_USB)   //shared memory not available for USB device
    if (eHAL_STATUS_SUCCESS == palAllocateMemory(pMac->hHdd, &virt, numBytes))
#elif defined(ANI_BUS_TYPE_PCI) //requires shared memory for DXE transfers
    if (eHAL_STATUS_SUCCESS == palAllocateSharedMemory(pMac->hHdd, &phSharedMem, &virt, &phys, numBytes, 1))
#endif
    {
        sHddMemSegment *freeList;
        
#if defined(ANI_BUS_TYPE_USB)   //shared memory not available for USB device
        phys = (tANI_BUFFER_ADDR) virt;
        heap->handle = virt;
#elif defined(ANI_BUS_TYPE_PCI) //requires shared memory for DXE transfers
        assert(phys != NULL);
        assert(phSharedMem != NULL);
        heap->handle = phSharedMem;
#endif
        assert(virt != NULL);
        
        heap->head.virt = virt;
        heap->tail.virt = virt + numBytes;
        heap->head.phys = (tANI_U8*)phys;
        heap->tail.phys = (tANI_U8*)phys + numBytes;

        freeList = AllocSegmentFromPool(&heap->segPool);
        assert(freeList != NULL);   //we just allocated the segPool, there has to be one resource available

        //starting with one free segment of contiguous memory
        assert(freeList->next == NULL);
        assert(freeList->addr == NULL);
        assert(freeList->hddContext == NULL);
        assert(freeList->length == 0);
        
        heap->freeList = freeList;
        heap->freeList->hddContext = heap->head.virt;        //using hddContext to hold virtual addresses
        heap->freeList->addr = heap->head.phys;              //using addr to hold physcial addresses
        heap->freeList->length = numBytes;

        heap->freeBytes = numBytes;
        heap->usedBytes = 0;

        return (DVT_STATUS_SUCCESS);
    }
    else
    {
        //free previously allocated segment pool
        palFreeMemory(pMac->hHdd, segPool->poolHandle);
        segPool->freeSegs = 0;
        segPool->usedSegs = 0;
        segPool->segPoolHead = NULL;
        segPool->segPoolTail = NULL;
        dvtLog(pMac, LOGE, "ERROR: could not allocate shared memory for heap!");
        return (DVT_STATUS_FAILURE);
    }
}

eANI_DVT_STATUS dvtFreeHeap(tpAniSirGlobal pMac, sDvtHeapResources *heap)
{
    assert(heap != NULL);
    
    if ((heap->handle != NULL) && (heap->usedBytes == 0))
    {
        sHddMemSegment *seg;
        tANI_U32 numBytes = 0;
        
        assert(heap->usedBytes == 0);
        
        //free all segments used in freeList to the segPool
        //don't worry about the shared memory attached because that will be freed from the heap's handle
        seg = heap->freeList;
        
        while (seg != NULL)
        {
            sHddMemSegment *nextSeg = seg->next;
            
            numBytes += seg->length;
            
            seg->next = NULL;
            seg->addr = NULL;
            seg->hddContext = NULL;
            seg->length = 0;
            
            FreeSegmentToPool(&heap->segPool, seg);
            
            seg = nextSeg;
        }
        
        //freeList disbanded and all bytes and segment resources should be accounted for
        assert(heap->freeBytes == numBytes);
        assert(heap->segPool.usedSegs == 0);
        assert(heap->segPool.freeSegs == SEG_POOL_SIZE(heap->freeBytes));

        //now free the sHddMemSegment resources which were all allocated at one time and attached to the segPool.poolHandle
        if (palFreeMemory(pMac->hHdd, heap->segPool.poolHandle) != eHAL_STATUS_SUCCESS)
        {
            assert(0);
        }
        else
        {
            heap->segPool.segPoolHead = NULL;
            heap->segPool.segPoolTail = NULL;
            heap->segPool.freeSegs = 0;
            heap->segPool.usedSegs = 0;
        }

        //heap allocated and no bytes missing
#if defined(ANI_BUS_TYPE_USB)   //shared memory not available for USB device
        if (palFreeMemory(pMac->hHdd, heap->handle) != eHAL_STATUS_SUCCESS)
#elif defined(ANI_BUS_TYPE_PCI) //requires shared memory for DXE transfers
        if (palFreeSharedMemory(pMac->hHdd, heap->handle) != eHAL_STATUS_SUCCESS)
#endif
        {
            dbgTrace(DBG_LEVEL_ERROR, DBG_MASK_MAC_DVT, ("!Failed to free heap\n"));
            return (DVT_STATUS_FAILURE);
        }
        else
        {
            //heap freed, reset heap variables
            heap->handle = NULL;
            heap->head.virt = NULL;
            heap->head.phys = NULL;
            heap->tail.virt = NULL;
            heap->tail.phys = NULL;
            heap->freeBytes = 0;

            return (DVT_STATUS_SUCCESS);
        }
    }
    else
    {
        dbgTrace(DBG_LEVEL_ERROR, DBG_MASK_MAC_DVT, ("!Failed to free heap\n"));
        return (DVT_STATUS_FAILURE);
    }
}

static sHddMemSegment *AllocSegmentFromPool(sSegPoolQueue *segPool)
{
    sHddMemSegment *retSeg = NULL;
    assert(segPool != NULL);

    //yes, I know that this always leaves one segment in the pool, but it makes the design easier
    if (segPool->segPoolHead != segPool->segPoolTail)
    {
        retSeg = segPool->segPoolHead;
        segPool->segPoolHead = segPool->segPoolHead->next;
        retSeg->next = NULL;               //unlink retSeg from pool

        segPool->freeSegs--;
        segPool->usedSegs++;
    }

    return (retSeg);
}

static void FreeSegmentToPool(sSegPoolQueue *segPool, sHddMemSegment *freeSeg)
{
    assert(freeSeg->addr == NULL);
    assert(freeSeg->hddContext == NULL);
    assert(freeSeg->next == NULL);
    assert(freeSeg->length == 0);

    segPool->segPoolTail->next = freeSeg;   //link freeSeg to pool
    segPool->segPoolTail = freeSeg;
    segPool->freeSegs++;
    segPool->usedSegs--;
}

//pass in the freeSeg to split, it's preceeding segment in the freeList, and the number of bytes to return
//returns NULL or an unlinked segment containing the requested number of bytes, and the freeList has been updated
static sHddMemSegment *SplitFreeSegment(sDvtHeapResources *heap, sHddMemSegment *freeSeg, sHddMemSegment *prevFreeSeg, tANI_U32 numBytes)
{
    sHddMemSegment *newSeg;
    
    assert(heap);
    assert(freeSeg);
    assert(numBytes);
    
    
    newSeg = AllocSegmentFromPool(&heap->segPool);

    if (newSeg != NULL)
    {
        //split freeSeg into two segments with the first equal to the requested numBytes
        //we will insert the newSeg in freeSeg's place and return the modified freeSeg
        newSeg->addr = (tANI_U8 *)(freeSeg->addr) + numBytes;
        newSeg->hddContext = (tANI_U8 *)(freeSeg->hddContext) + numBytes;
        newSeg->length = freeSeg->length - numBytes;
        newSeg->next = freeSeg->next;               //this may or may not be NULL

        //relink prevFreeSeg to newSeg
        if (prevFreeSeg == NULL)
        {
            //this was the first segment in the freeList
            //newSeg becomes the first segment in freeList
            assert(heap->freeList == freeSeg);
            heap->freeList = newSeg;
        }
        else
        {
            assert(prevFreeSeg->next == freeSeg);
            prevFreeSeg->next = newSeg;             //this may or may not be NULL
        }

        freeSeg->length = numBytes;
        heap->freeBytes -= numBytes;
        heap->usedBytes += numBytes;
        freeSeg->next = NULL;                      //unlink this segment from the list
        return (freeSeg);
    }
    else
    {
        return (NULL);
    }
}

sHddMemSegment *GetHeapBytes(tpAniSirGlobal pMac, sDvtHeapResources *heap, tANI_U32 numBytes, eDvtHeapAccess access)
{
    sHddMemSegment *freeSeg;
    sHddMemSegment *prevFreeSeg = NULL;

    assert(heap != NULL);
    assert(heap->handle != NULL);
    assert(heap->head.virt != NULL);
    assert(heap->head.phys != NULL);
    assert(heap->tail.virt != NULL);
    assert(heap->tail.phys != NULL);

    assert(numBytes != 0);

    if (heap->freeBytes < numBytes)
    {
        //this says that all the freeBytes added together don't add up to enough to satisfy the request
        assert(heap->usedBytes > numBytes);
        return (NULL);
    }

    freeSeg = heap->freeList;

    //loop through free segments looking for one that is big enough and aligned properly
    while (freeSeg != NULL)
    {
        assert(freeSeg->addr != NULL);
        assert(freeSeg->hddContext != NULL);
        assert(freeSeg->length != 0);

        if (((access == DVT_HEAP_DWORD_ACCESS) && ((tANI_U32)(freeSeg->addr) % 4 == 0)) ||
            (access == DVT_HEAP_BYTE_ACCESS)
           )
        {
            //this segment is 32-bit aligned OR it doesn't matter

            if (freeSeg->length > numBytes)
            {
                //this segment is larger than numBytes requested
                //split segment into two and return a pointer to the one sized to fit numBytes
                sHddMemSegment *retSeg = SplitFreeSegment(heap, freeSeg, prevFreeSeg, numBytes);

                if (retSeg != NULL)
                {
                    return (retSeg);
                }
                else
                {
                    //no sHddMemSegments available to split a free segment
                    //there may be equal sized segments later in the list
                    // ! fall through to continue looking for a segment of equal size
                }
            }
            else if (freeSeg->length == numBytes)
            {
                //this segment is equal to the numBytes requested
                if (prevFreeSeg == NULL)
                {
                    //first segment in free list, just unlink it and return it
                    assert(heap->freeList == freeSeg);
                    heap->freeList = freeSeg->next;
                }
                else
                {
                    //relink previous segment to freeSeg->next segment, and return the freeSeg
                    assert(prevFreeSeg->next == freeSeg);
                    prevFreeSeg->next = freeSeg->next;          //this may or may not be NULL
                }

                heap->freeBytes -= numBytes;
                heap->usedBytes += numBytes;
                freeSeg->next = NULL;
                return(freeSeg);
            }
        }
        else if (freeSeg->length > FREE_BYTE_RECLAIM_THRESHOLD)
        {
            //this segment contains more memory than our entire reclaim threshold, 
            // which means it wasn't aligned properly, so fragment it so that it is aligned for DVT_HEAP_DWORD_ACCESS
            //split segment into three pieces: 
            //  one small piece to get rid of non-aligned addess, 
            //  next piece that is now aligned properly needs to be split into numBytes and a remainder segment
            // and return a pointer to the one sized to fit numBytes
            tANI_U32 smallSize = ((tANI_U32)(freeSeg->addr) % 4) + SEG_GRANULARITY;
            sHddMemSegment *smallSeg = SplitFreeSegment(heap, freeSeg, prevFreeSeg, smallSize);
            
            
            if (smallSeg != NULL)
            {
                sHddMemSegment *retSeg = NULL;
                
                //relink smallSeg to end of freeList
                PutHeapBytes(pMac, heap, smallSeg);
                
                if (prevFreeSeg == NULL)
                {
                    //now get the retSeg from the top of the freeList, which should now be aligned for DVT_HEAP_DWORD_ACCESS
                    retSeg = SplitFreeSegment(heap, heap->freeList, NULL, numBytes);
                }
                else
                {
                    //now get the retSeg from the newSeg which was created from splitting the previous segment, 
                    // which should now be aligned for DVT_HEAP_DWORD_ACCESS
                    retSeg = SplitFreeSegment(heap, prevFreeSeg->next, prevFreeSeg, numBytes);
                }
                
                if (retSeg != NULL)
                {
                    return (retSeg);
                }
            }
            //else fall through and look for another segment of numBytes in length
        }

        //unsatisfied request - move to next segment
        prevFreeSeg = freeSeg;
        freeSeg = freeSeg->next;

    }   //while (freeSeg != NULL)

    return (NULL);

}

void PutHeapBytes(tpAniSirGlobal pMac, sDvtHeapResources *heap, sHddMemSegment *garbageList)
{
    sHddMemSegment *garbageSeg = garbageList;
    sHddMemSegment *freeSeg = heap->freeList;
    sHddMemSegment *prevFreeSeg = NULL;
    tANI_U32 garbageBytes = 0;
    tANI_U32 freeBytes = 0;

    assert(heap != NULL);
    assert(garbageList != NULL);

    while (garbageSeg != NULL)
    {
        assert(garbageSeg->addr != NULL);
        assert(garbageSeg->hddContext != NULL);
        assert(garbageSeg->length != 0);

        garbageBytes += garbageSeg->length;
        garbageSeg = garbageSeg->next;
    }

    //walk to end of list to find node to attach the garbage list to
    while (freeSeg != NULL)
    {
        assert(freeSeg->addr != NULL);
        assert(freeSeg->hddContext != NULL);
        assert(freeSeg->length != 0);

        freeBytes += freeSeg->length;
        prevFreeSeg = freeSeg;
        freeSeg = freeSeg->next;
    }

    assert(freeBytes == heap->freeBytes);
    assert((tANI_U8 *)heap->head.phys + (freeBytes + heap->usedBytes) == heap->tail.phys);
    assert((tANI_U8 *)heap->head.virt + (freeBytes + heap->usedBytes) == heap->tail.virt);

    //attach garbageList to freeList
    prevFreeSeg->next = garbageList;
    heap->freeBytes += garbageBytes;
    heap->usedBytes -= garbageBytes;

/*
     if ((heap->segPool.freeSegs < FREE_SEGMENT_RECLAIM_THRESHOLD) ||
         (heap->freeBytes < FREE_BYTE_RECLAIM_THRESHOLD)
        )
        
     This condition is not quite good enough as we still wind up with fragmented memory
     So for the moment, run garbage cleanup everytime, but this slows down processing considerably
*/
    {
        
        HeapGarbageCleanup(pMac, heap);
    }
}


static void HeapGarbageCleanup(tpAniSirGlobal pMac, sDvtHeapResources *heap)
{
    //algorithm to scan freeList and coallesce fragmented memory

    /*
        This algorithm does not assume any ordering of segments/physical addresses in the free list
        
        starting at the top of the free list, pick a segment,
            and then loop through all segments in the free list, checking to see
                if it's ending physical address is contiguous with other segment's beginning physical address or
                if it's beginning physical address is contiguous with other segment's ending physical address
                    if the segment's are adjacent, use the segment with the preceding address to combine the two memory pieces
                        and then free the other segment
                        and then restart the entire process from the top
    */


    tANI_BOOLEAN restart;
    sHddMemSegment *matchSeg;
    sHddMemSegment *prevMatchSeg;
    sHddMemSegment *testSeg;
    sHddMemSegment *prevTestSeg;

    if (&pMac->dvt.txHeap == heap)
    {
        dvtLog(pMac, LOGW, "DVT Tx Heap\n");
    }
    else if (&pMac->dvt.rxHeap == heap)
    {
        dvtLog(pMac, LOGW, "DVT Rx Heap\n");
    }

    dvtLog(pMac, LOGW, "Before Garbage Collection\n");
    dvtLog(pMac, LOGW, "freeSegs = %d\n", heap->segPool.freeSegs);
    dvtLog(pMac, LOGW, "usedSegs = %d\n", heap->segPool.usedSegs);

    do
    {
        restart = eANI_BOOLEAN_FALSE;
        matchSeg = heap->freeList;
        prevMatchSeg = NULL;

        while (matchSeg != NULL && restart == eANI_BOOLEAN_FALSE)
        {
            prevTestSeg = matchSeg;
            testSeg = matchSeg->next;

            while (testSeg != NULL && restart == eANI_BOOLEAN_FALSE)
            {

                if (((tANI_U8 *)matchSeg->addr + matchSeg->length) == (tANI_U8 *)testSeg->addr)
                {
                    //matchSeg's ending address is contigous with the testSeg's beginning address
                    //add testSeg's memory to matchSeg's
                    //matchSeg's linkage remains the same
                    //relink around testSeg and free testSeg
                    matchSeg->length = matchSeg->length + testSeg->length;

                    assert(prevTestSeg != NULL);
                    assert(prevTestSeg->next == testSeg);
                    prevTestSeg->next = testSeg->next;

                    testSeg->addr = NULL;
                    testSeg->hddContext = NULL;
                    testSeg->length = 0;
                    testSeg->next = NULL;
                    FreeSegmentToPool(&heap->segPool, testSeg);

                    //not necessary to restart because 
                    // the testSeg's beginning address has already been matched 
                    //  and the ending address has already been tested against previous matchSeg's in freeList
                    //! just continue to test the same matchSeg against remaining testSegs in the freeList
                    testSeg = prevTestSeg->next;
                }
                else if (((tANI_U8 *)testSeg->addr + testSeg->length) == (tANI_U8 *)matchSeg->addr)
                {
                    //matchSeg's beginning address is contiguous with the testSeg's ending address
                    //add matchSeg's memory to testSeg's
                    //testSeg's linkage remains the same
                    //relink around matchSeg and free matchSeg
                    testSeg->length = testSeg->length + matchSeg->length;

                    if (prevMatchSeg != NULL)
                    {
                        //this was not the first segment in freeList
                        prevMatchSeg->next = matchSeg->next;            //relink prevMatchSeg around matchSeg
                    }
                    else
                    {
                        //prevMatchSeg = NULL
                        //this means that matchSeg = freeList = first segment in the free list
                        assert(matchSeg == heap->freeList);
                        heap->freeList = matchSeg->next;                //relink freeList around matchSeg
                    }
                    
                    matchSeg->addr = NULL;
                    matchSeg->hddContext = NULL;
                    matchSeg->length = 0;
                    matchSeg->next = NULL;
                    FreeSegmentToPool(&heap->segPool, matchSeg);
                    
                    //we cannot backup far enough to continue from here
                    restart = eANI_BOOLEAN_TRUE;
                }
                else
                {
                    prevTestSeg = testSeg;
                    testSeg = testSeg->next;    //try the next one in the list
                }
            }
            prevMatchSeg = matchSeg;
            matchSeg = matchSeg->next;
        }
    }while(restart == eANI_BOOLEAN_TRUE);

    //no contigous free segments remaining - garbage collection complete

    dvtLog(pMac, LOGW, "After Garbage Collection\n");
    dvtLog(pMac, LOGW, "freeSegs = %d\n", heap->segPool.freeSegs);
    dvtLog(pMac, LOGW, "usedSegs = %d\n", heap->segPool.usedSegs);
}



void testForceDvtHeapGarbageCleanup(tpAniSirGlobal pMac)
{
    HeapGarbageCleanup(pMac, &(pMac->dvt.txHeap));
    dvtLog(pMac, LOGW, "txFreeBytes = %d\n", pMac->dvt.txHeap.freeBytes);
    dvtLog(pMac, LOGW, "txUsedBytes = %d\n", pMac->dvt.txHeap.usedBytes);

    HeapGarbageCleanup(pMac, &(pMac->dvt.rxHeap));
    dvtLog(pMac, LOGW, "rxFreeBytes = %d\n", pMac->dvt.rxHeap.freeBytes);
    dvtLog(pMac, LOGW, "rxUsedBytes = %d\n", pMac->dvt.rxHeap.usedBytes);

}

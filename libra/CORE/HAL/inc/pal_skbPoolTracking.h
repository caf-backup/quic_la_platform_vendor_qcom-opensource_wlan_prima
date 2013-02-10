/** ------------------------------------------------------------------------ *
    ------------------------------------------------------------------------ *
 

  
    \file pal_skbPoolTracking.h
  
    Definition of the Linux SKB Allocation Tracking Module
  
    Copyright (C) 2007 Qualcomm Technologies, Inc.
  
 
    ======================================================================== */

#ifndef _PAL_SKBPOOLTRACKING_H_
#define _PAL_SKBPOOLTRACKING_H_

// previously defined skb tracking markers
#define ANI_MARK_palRxPoll_1                   1
#define ANI_MARK_hdd_poll_slow_1               2
#define ANI_MARK_hdd_poll_slow_2               3
#define ANI_MARK_hdd_poll_slow_3               4
#define ANI_MARK_hdd_poll_slow_4               5
#define ANI_MARK_hdd_poll_slow_5               6
#define ANI_MARK_hdd_poll_slow_6               7
#define ANI_MARK_hdd_poll_slow_7               8
#define ANI_MARK_hdd_poll_slow_8               9
#define ANI_MARK_hdd_poll_slow_9              10
#define ANI_MARK_hdd_poll_slow_10             11
#define ANI_MARK_hdd_poll_slow_11             12
#define ANI_MARK_hdd_poll_1                   13
#define ANI_MARK_hdd_poll_2                   14
#define ANI_MARK_hdd_poll_3                   15
#define ANI_MARK_hdd_poll_4                   16
#define ANI_MARK_hdd_poll_5                   17
#define ANI_MARK_sysRecvPacket_1              18
#define ANI_MARK_sysRecvPacket_2              19
#define ANI_MARK_sysBbtProcessMessageCore_1   20
#define ANI_MARK_sysBbtProcessMessageCore_2   21
#define ANI_MARK_sysBbtProcessMessageCore_3   22
#define ANI_MARK_sysBbtProcessMessageCore_4   23
#define ANI_MARK_sysBbtProcessMessageCore_5   24
#define ANI_MARK_sysBbtProcessMessageCore_6   25
#define ANI_MARK_sysBbtProcessMessageCore_7   26
#define ANI_MARK_sysBbtProcessMessageCore_8   27
#define ANI_MARK_sysBbtProcessMessageCore_9   28
#define ANI_MARK_sysBbtProcessMessageCore_10  29
#define ANI_MARK_limHandle80211Frames_1       30
#define ANI_MARK_limHandleFramesInScanState_1 31
#define ANI_MARK_halFreeTxRxFrame_1           32
#define ANI_MARK_RxFrames_1                   33
#define ANI_MARK_RxFrames_2                   34
#define ANI_MARK_RxFrames_3                   35
#define ANI_MARK_RxFrames_4                   36
#define ANI_MARK_RxFrames_5                   37
#define ANI_MARK_RxFrames_6                   38
#define ANI_MARK_RxFrames_7                   39
#define ANI_MARK_RxFrames_8                   40
#define ANI_MARK_RxFrames_9                   41
#define ANI_MARK_RxFrames_10                  42
#define ANI_MARK_RxFrames_11                  43
#define ANI_MARK_RxFrames_12                  44
#define ANI_MARK_RxFrames_13                  45
#define ANI_MARK_RxFrames_14                  46
#define ANI_MARK_RxFrames_15                  47

#ifdef ANI_SKB_POOL_TRACKING
void aniSkbPoolTrackingOpen(int poolSize);
void aniSkbPoolTrackingClose(void);
void aniSkbPoolTrackingAlloc(void * skb, void * allocator);
void aniSkbPoolTrackingMark(void * skb, int mark);
void aniSkbPoolTrackingFree(void * skb);
void aniSkbPoolTrackingDump(void);
void aniSkbPoolTrackingDumpSkb(void * skb);
#else // ANI_SKB_POOL_TRACKING
static inline void aniSkbPoolTrackingOpen(int poolSize){};
static inline void aniSkbPoolTrackingClose(void){};
static inline void aniSkbPoolTrackingAlloc(void * skb, void * allocator){};
static inline void aniSkbPoolTrackingMark(void * skb, int mark){};
static inline void aniSkbPoolTrackingFree(void * skb){};
static inline void aniSkbPoolTrackingDump(void){};
static inline void aniSkbPoolTrackingDumpSkb(void * skb){};
#endif // ANI_SKB_POOL_TRACKING

#endif // _PAL_SKBPOOLTRACKING_H_

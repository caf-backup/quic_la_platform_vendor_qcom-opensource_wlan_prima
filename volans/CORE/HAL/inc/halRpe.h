/**
 *
 *  @file:         halRpe.h
 *
 *  @brief:       Export or Include file for all the MAC APIs to the 
 *                   RPE Hardware Block.
 *
 *  @author:    Madhava Reddy S
 *
 *  Copyright (C) 2002 - 2007, Qualcomm, Inc. All rights reserved.
 *
 *  Change History:
 * ---------------------------------------
 * 06/07/2007  File created.
 * 01/30/2008  Moved from bringup to virgo production.
 */

#ifndef _HALRPE_H_
#define _HALRPE_H_

#include "halTypes.h"
#include "palTypes.h"
#include "aniGlobal.h"
#include "sirMacProtDef.h"
#include "halMemoryMap.h"


typedef enum eRpeSwBlockReq
{
    eRPE_SW_DISABLE_DROP,
    eRPE_SW_ENABLE_DROP,
} tRpeSwBlockReq;

/**
  * 	RPE Queue Info
  */
#ifdef WLAN_HAL_VOLANS
typedef __ani_attr_pre_packed struct sRpeStaQueueInfo {

    /** Byte 0 - 3 */
#ifdef ANI_BIG_BYTE_ENDIAN
    tANI_U32 ba_ssn             : 12;
    tANI_U32 ssn_sval           : 1;
    tANI_U32 ba_window_size     : 6;
    tANI_U32 check_2k           : 1;
    tANI_U32 frg                : 1;
    tANI_U32 ord                : 1;
    tANI_U32 fsh                : 1;
    tANI_U32 rty                : 1;
    tANI_U32 psr                : 1;
    tANI_U32 bar                : 1;
    tANI_U32 reserved1          : 5;
    tANI_U32 val                : 1;

#else
    tANI_U32 val                : 1;
    tANI_U32 reserved1          : 5;
    tANI_U32 bar                : 1;
    tANI_U32 psr                : 1;
    tANI_U32 rty                : 1;
    tANI_U32 fsh                : 1;
    tANI_U32 ord                : 1;
    tANI_U32 frg                : 1;
    tANI_U32 check_2k           : 1;
    tANI_U32 ba_window_size     : 6;
    tANI_U32 ssn_sval           : 1;
    tANI_U32 ba_ssn             : 12;
#endif

//BYTE 4 - 7
#ifdef ANI_BIG_BYTE_ENDIAN
    tANI_U32 staId_queueId_Reorderbitmap:16;
    tANI_U32 staId_queueId_BAbitmap:16;
#else
    tANI_U32 staId_queueId_BAbitmap:16;
    tANI_U32 staId_queueId_Reorderbitmap:16;
#endif

//BYTE 8 - 11
#ifdef ANI_BIG_BYTE_ENDIAN
    tANI_U32 reserved2              : 6;
    tANI_U32 rod                    : 1;
    tANI_U32 current_index          : 6;
    tANI_U32 reorder_window_size    : 6;
    tANI_U32 reorder_sval           : 1;
    tANI_U32 reorder_ssn            : 12;
#else
    tANI_U32 reorder_ssn            : 12;
    tANI_U32 reorder_sval           : 1;
    tANI_U32 reorder_window_size    : 6;
    tANI_U32 current_index          : 6;
    tANI_U32 rod                    : 1;
    tANI_U32 reserved2              : 6;
#endif

//BYTE 12 - 15
#ifdef ANI_BIG_BYTE_ENDIAN
    tANI_U32 reserved3                    : 14;
    tANI_U32 mem_location_staId_qId       : 18;
#else
    tANI_U32 mem_location_staId_qId       : 18;
    tANI_U32 reserved3                    : 14;    
#endif

//BYTE 16 - 19
    tANI_U32 q_timestamp;
} __ani_attr_packed __ani_attr_aligned_4 tRpeStaQueueInfo, *tpRpeStaQueueInfo;
#else
typedef __ani_attr_pre_packed struct sRpeStaQueueInfo {

	/** Byte 0 - 3 */
#ifdef ANI_BIG_BYTE_ENDIAN
	tANI_U32 ba_ssn 			: 12;
	tANI_U32 ssn_sval			: 1;
	tANI_U32 ba_window_size 	: 6;
	tANI_U32 check_2k			: 1;
	tANI_U32 frg				: 1;
	tANI_U32 ord				: 1;
	tANI_U32 fsh				: 1;
	tANI_U32 rty				: 1;
	tANI_U32 psr				: 1;
	tANI_U32 bar				: 1;
	tANI_U32 reserved1			: 5;
	tANI_U32 val				: 1;

#else
	tANI_U32 val				: 1;
	tANI_U32 reserved1			: 5;
	tANI_U32 bar				: 1;
	tANI_U32 psr				: 1;
	tANI_U32 rty				: 1;
	tANI_U32 fsh				: 1;
	tANI_U32 ord				: 1;
	tANI_U32 frg				: 1;
	tANI_U32 check_2k			: 1;
	tANI_U32 ba_window_size		: 6;
	tANI_U32 ssn_sval			: 1;
	tANI_U32 ba_ssn				: 12;
#endif

	tANI_U32 staId_queueId_BAbitmapLo;
	tANI_U32 staId_queueId_BAbitmapHi;
	tANI_U32 staId_queueId_ReorderbitmapLo;
	tANI_U32 staId_queueId_ReorderbitmapHi;

#ifdef ANI_BIG_BYTE_ENDIAN
	tANI_U32 reserved2				: 13;
	tANI_U32 reorder_window_size	: 6;
	tANI_U32 reorder_sval			: 1;
	tANI_U32 reorder_ssn			: 12;
#else
	tANI_U32 reorder_ssn			: 12;
	tANI_U32 reorder_sval			: 1;
	tANI_U32 reorder_window_size	: 6;
	tANI_U32 reserved2				: 13;
#endif

	//tANI_U32 reserved3;		/** Temproy Additiion change for HW Design */
} __ani_attr_packed __ani_attr_aligned_4 tRpeStaQueueInfo, *tpRpeStaQueueInfo;
#endif
/**
  * 	RPE Partial BA Info
  */

typedef __ani_attr_pre_packed struct sRpePartialBAInfo {
#ifdef ANI_BIG_BYTE_ENDIAN
	tANI_U32 partial_ba_ssn 		: 12;
	tANI_U32 partial_sval			: 1;
	tANI_U32 reserved4				: 12;
	tANI_U32 partial_bar			: 1;
	tANI_U32 reserved3				: 6;
#else
	tANI_U32 reserved3				: 6;
	tANI_U32 partial_bar			: 1;
	tANI_U32 reserved4				: 12;
	tANI_U32 partial_sval			: 1;
	tANI_U32 partial_ba_ssn			: 12;

#endif

	tANI_U32 partialState_BAbitmapLo;
	tANI_U32 partialState_BAbitmapHi;

} __ani_attr_packed __ani_attr_aligned_4 tRpePartialBAInfo, *tpRpePartialBAInfo;


/**
  * 	RPE STA Desc
  */

typedef  __ani_attr_pre_packed struct sRpeStaDesc {
	tRpeStaQueueInfo	rpeStaQueueInfo[HW_MAX_QUEUES];
} __ani_attr_packed __ani_attr_aligned_4 tRpeStaDesc, *tpRpeStaDesc;

eHalStatus halRpe_Start(tHalHandle hHal, void *arg);
eHalStatus halRpe_CfgRoutingFlag(tpAniSirGlobal pMac, tANI_U32 drop_pkts,
									tANI_U32 good_pkts);
eHalStatus halRpe_program_staid_qid(tpAniSirGlobal pMac,
									tANI_U32 maxRpeStations, tANI_U32 maxRpeQueues);
eHalStatus halRpe_sta_desc_base(tpAniSirGlobal pMac, tANI_U32 rpeStaDesc_offset);
eHalStatus halRpe_cfgStaDesc(tpAniSirGlobal pMac, tANI_U32 staIdx,
									tpRpeStaDesc rpeStaDesc);
eHalStatus halRpe_GetStaDesc(tpAniSirGlobal pMac, tANI_U8 staId,
									tpRpeStaDesc rpeStaDesc);
eHalStatus halRpeinit_error_interrupt(tpAniSirGlobal pMac, tANI_U32 error_mask);

eHalStatus halRpe_SaveStaConfig(tpAniSirGlobal pMac, tpRpeStaDesc pRpeStaDesc, tANI_U8 staIdx);
eHalStatus halRpe_RestoreStaConfig(tpAniSirGlobal pMac, tpRpeStaDesc pRpeStaDesc, tANI_U8 staIdx);

 eHalStatus halRpe_UpdateStaDesc(tpAniSirGlobal pMac, tANI_U32 staIdx,
													tpRpeStaDesc rpeStaDesc);
 eHalStatus halRpe_UpdateStaDescQueueInfo(tpAniSirGlobal pMac, tANI_U32 staIdx,
											tANI_U32 queueId, tpRpeStaQueueInfo rpeStaQueueInfo);
 eHalStatus halRpe_GetStaDescQueueInfo(tpAniSirGlobal pMac, tANI_U32 staIdx,
											tANI_U32 queueId, tpRpeStaQueueInfo rpeStaQueueInfo);
 eHalStatus halRpe_SaveStaQueueConfig(tpAniSirGlobal pMac, tANI_U8 staIdx, tANI_U32 queueId,
															tpRpeStaQueueInfo rpeStaQueueInfo);
 eHalStatus halRpe_RestoreStaQueueConfig(tpAniSirGlobal pMac, tANI_U8 staIdx, tANI_U32 queueId,
															tpRpeStaQueueInfo rpeStaQueueInfo);
 eHalStatus halRpe_UpdateSwBlockReq(tpAniSirGlobal pMac, tANI_U8 staIdx, tANI_U8 queueId,
												tRpeSwBlockReq rpeSwBlockReq);
eHalStatus halRpe_BlockAndFlushFrames(tpAniSirGlobal pMac, tANI_U8 staIdx, tANI_U8 queueId,
												tRpeSwBlockReq rpeSwBlockReq);
 eHalStatus halRpe_FlushBitMapCache(tpAniSirGlobal pMac);
 eHalStatus halRpe_FlushrsrcEntry(tpAniSirGlobal pMac, tANI_U8 staIdx, tANI_U8 queueId);
eHalStatus halRpe_ErrIntHandler(tHalHandle hHalHandle, eHalIntSources intSource);

#endif


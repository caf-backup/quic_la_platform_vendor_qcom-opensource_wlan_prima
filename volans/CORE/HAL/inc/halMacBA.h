/*
 * =====================================================================================
 * 
 *       Filename:  halMacBA.h
 * 
 *    Description:  API's and Interfaces to support A-MPDU/BA functionality
 * 
 *        Version:  1.0
 *        Created:  11/16/2006 05:17:03 PM PST
 *       Revision:  none
 *       Compiler:  gcc
 * 
 *         Author:  Ashok Ranganath
 *        Company:  Airgo Networks, Inc.
 * 
 * =====================================================================================
 */

#ifndef _HALMAC_BA_H_
#define _HALMAC_BA_H_

#include "sirTypes.h"
#include "halMsgApi.h"

#ifdef WLAN_HAL_VOLANS
#define BA_DEFAULT_RX_BUFFER_SIZE 16
#else
#define BA_DEFAULT_RX_BUFFER_SIZE 16
#endif
#define BA_MAX_SESSIONS 8
#define BA_SESSION_ID_INVALID 0xFFFF

// Fragmented packets received in BA session 
#define HAL_BA_ERR_FRAG_PKT_IN_BA               0x1
// BAR frame received in NON BA session
#define HAL_BA_ERR_BAR_RCVD_IN_NON_BA           0x10
// BAR frame received with a 2k jump in SSN
#define HAL_BA_ERR_BAR_RCVD_WITH_2K_SSN_JUMP    0x100
// AMPDU frame received with 2k jump in Seq Number
#define HAL_BA_ERR_AMPDU_RCVD_WITH2K_SSN_JUMP   0x1000
// BA Timeout
#define HAL_BA_ERR_TIMEOUT                      0x10000

#define	HAL_BAR_FRM_CNT								2

#define HAL_BA_BLOCK_TIMEOUT          2

typedef struct sAddBaInfo
{
    tANI_U16 fBaEnable : 1;
    tANI_U16 startingSeqNum: 12;
    tANI_U16 reserved : 3;
}tAddBaInfo, *tpAddBaInfo;

typedef struct sAddBaCandidate
{
    tSirMacAddr staAddr;
    tAddBaInfo baInfo[STACFG_MAX_TC];
}tAddBaCandidate, *tpAddBaCandidate;

// in order to set BA session in rx direction
// HAL will send request to HDD first
// When HAL will receive response from HDD then only
// it will send request to softmac. 
//
// in transmit direction there is not need to send
// the request to HDD. So HAL will send the request
// directly to softmac. 
typedef enum
{
    eHAL_ADDBA_NORMAL,
    eHAL_ADDBA_WT_HDD_RSP, //waiting for hdd rsp.
}tHalAddBAState;

typedef enum
{
	eHALBA_TIMER_STARTED,
	eHALBA_TIMER_NOTSTARTED
}tHalBATimerState;
// to save addBAReqParams while HAL is waiting for resp from softmac.
typedef struct
{
    tANI_U32 msgSerialNo;
    tHalAddBAState addBAState;
    void* pAddBAReqParams;
}tSavedAddBAReqParamsStruct;

typedef struct sRxBASessionTable
{

  // Indicates if the current entry being probed is
  // a valid & active BA sesion or not
  tANI_U16 baValid:1;

  // Identifies the TID for which the current BA session
  // has been setup
  tANI_U16 baTID:4;

  // Identifies the HAL STA index for which the current BA
  // session has been setup
  tANI_U16 baStaIndex:11;

  // A globally unique object that is used to identify
  // each of the active BA sessions. This is termed as
  // a "BA Session ID".
  //
  // This ranges from 0..65535
  // Each time a new BA session is established:
  // - an Entry is marked as "valid" in the BA Session Table
  //tANI_U16 baSessionID:11;

#ifdef FEATURE_ON_CHIP_REORDERING
  tANI_BOOLEAN isReorderingDoneOnChip;
#endif

} tRxBASessionTable, *tpRxBASessionTable;

void baInit( tpAniSirGlobal pMac );

void baHandleCFG( tpAniSirGlobal pMac, tANI_U32 cfgId );

eHalStatus baAllocateBuffer( tpAniSirGlobal pMac,
    tANI_U16 *baBufferSize );

void baReleaseBuffer( tpAniSirGlobal pMac,
    tANI_U16 baBufferSize );

eHalStatus baAllocateSessionID( tpAniSirGlobal pMac,
    tANI_U16 baStaIndex,
    tANI_U8 baTID,
    tANI_U16 *baSessionID );

eHalStatus baReleaseSessionID( tpAniSirGlobal pMac,
    tANI_U16 baStaIndex,
    tANI_U8 baTID );

eHalStatus baReleaseSTA( tpAniSirGlobal pMac,
    tANI_U16 baStaIndex );

eHalStatus baAddBASession(tpAniSirGlobal pMac, tpAddBAParams pAddBAParams);
eHalStatus baDelBASession(tpAniSirGlobal pMac, tpDelBAParams pAddBAParams);

eHalStatus baAddReqTL( tpAniSirGlobal pMac, tANI_U16 baSessionID,
    tANI_U8 baTID, tANI_U16 staIdx, tANI_U8 baBufferSize );

eHalStatus baDelNotifyTL( tpAniSirGlobal pMac,
    tANI_U16 baSessionID );
eHalStatus baProcessTLAddBARsp(
                    tpAniSirGlobal pMac,
                    tANI_U16 baSessionID,
                    tANI_U16 tlWindowSize
                    #ifdef FEATURE_ON_CHIP_REORDERING
                    ,tANI_BOOLEAN isReorderingDoneOnChip
                    #endif
                    );
void halBaCheckActivity(tpAniSirGlobal pMac);
void  halGetBaCandidates(tpAniSirGlobal pMac, tANI_U8* pStaList,  tANI_U16* pBaCandidateCnt );
eHalStatus halStartBATimer(tpAniSirGlobal  pMac);
#ifdef CONFIGURE_SW_TEMPLATE
void halSendUnSolicitBARFrame(tpAniSirGlobal pMac, tANI_U16 staIdx, 
					tANI_U16 baTID, tANI_U16 queueId);
eHalStatus halGetUpdatedSSN(tpAniSirGlobal pMac, tANI_U16 staIdx, tANI_U16 baTID, 
						tANI_U16 queueId, tANI_U16 *ssn);
void fillBARCtrlInfo (barCtrlType		 *pBARCtrl, tANI_U16 baTID);
void fillFrameCtrlInfo (tSirMacFrameCtl *pfc);

eHalStatus halTpe_TriggerSwTemplate(tpAniSirGlobal pMac);
#endif //CONFIGURE_SW_TEMPLATE

#ifdef FEATURE_ON_CHIP_REORDERING
eHalStatus halGetBASession(tpAniSirGlobal pMac, tANI_U16 baStaIndex, tANI_U8 baTID, tpRxBASessionTable* pBA);
#endif
#endif // _HALMAC_BA_H_

/*
 * Airgo Networks, Inc proprietary. All rights reserved.
 * halInitApi.c:  Provide HAL APIs.
 * Author:    Susan Tsao
 * Date:      02/06/2006
 *
 * --------------------------------------------------------------------------
 */
#ifndef _HALBMU_H_
#define _HALBMU_H_

#include "halTypes.h"
#include "palTypes.h"
#include "aniGlobal.h"
#include "halMemoryMap.h"
#include "halMsgApi.h"		/** Temperpry Change: Remove after permanent fix */
#include "halInterrupts.h"

#define BMU_CONV_BD_PDU_IDX_TO_ADDR(x)     (pMac->hal.memMap.packetMemory_offset + (x)* HAL_BD_SIZE)

typedef struct sBmuStaQueueData {
    tANI_U8                 bssIdx;
    tANI_U8                 numOfSta;                /* Number of stations in power save, who have data pending*/
#ifdef FIXME_VOLANS
    tANI_U8                 numOfStaWithoutData; /* Number of stations in power save, who don't have any data pending*/
    tANI_U8                 fBroadcastTrafficPending ;
    tANI_U8                 dtimCount;
#endif /* FIXME_VOLANS */
    tANI_U8                 rsvd[3];                /** Align the Structure to 4 bytes as unalligned access will happen if
                                                    the staInfo is being Accessed */                                                        
    tANI_U16                assocId[HAL_NUM_STA];/* Station indices */
}tBmuStaQueueData, *tpBmuStaQueueData;


/*  
 * tBmuBtqmBdInfo: BTQM BD info structure: Contains raw BD info returned from BTQM.
 *   frameCtrl:  FC field in the BD but be aware this is not real FC field in BD. BTQM doesn't provide VER, Retry, PwrMgmt, MoreData bits 
 *  seqNum: Sequence number of  the frame
 *  fragNum: fragment number of  the frame
 *  frameSize: Frame size in the TxBD
 *  nxtBdIndex: Next BD in the BTQM queue
 *  BD rate offset: BD rate offset in the TxBD.
 *  bdIntrMask: TxComplete interrupt setting in the TxBD.
 *  retry: Current retry count of the frame.
  */
typedef struct sBmuBtqmBdInfo {
  tANI_U32  frameCtrl:   16; 
  tANI_U32  seqNum:      12;  
  tANI_U32  fragNum:      4;  
  tANI_U32  frameSize:   16;
  tANI_U32  nxtBdIndex:  10;
  tANI_U32  bdRateOffset: 2;
  tANI_U32  bdIntrMask:   2;
  tANI_U32  retry:        4;
} tBmuBtqmBdInfo, *tpBmuBtqmBdInfo;

#define QWLAN_BMU_BD_INDEX_INFO0_NEXT_BD_INDEX_MASK     0x3ff
#define QWLAN_BMU_BD_INDEX_INFO0_NEXT_BD_INDEX_OFFSET   0
#define QWLAN_BMU_BD_INDEX_INFO0_FRAME_SIZE_MASK        0x01fff800
#define QWLAN_BMU_BD_INDEX_INFO0_FRAME_SIZE_OFFSET      11
#define QWLAN_BMU_BD_INDEX_INFO0_RETRY_COUNT_MASK       0x1e000000
#define QWLAN_BMU_BD_INDEX_INFO0_RETRY_COUNT_OFFSET     25       
#define QWLAN_BMU_BD_INDEX_INFO0_FRAG_NUM_COUNT_LO_MASK  0xe0000000
#define QWLAN_BMU_BD_INDEX_INFO0_FRAG_NUM_COUNT_LO_OFFSET 29       
#define QWLAN_BMU_BD_INDEX_INFO0_FRAG_NUM_COUNT_LO_BITS 3

#define QWLAN_BMU_BD_INDEX_INFO1_FRAG_NUM_COUNT_HI_MASK  0x1
#define QWLAN_BMU_BD_INDEX_INFO1_FRAG_NUM_COUNT_HI_OFFSET 0
#define QWLAN_BMU_BD_INDEX_INFO1_FRAG_NUM_COUNT_HI_BITS 1

#define QWLAN_BMU_BD_INDEX_INFO1_SEQ_NUM_COUNT_MASK  0x00001ffe
#define QWLAN_BMU_BD_INDEX_INFO1_SEQ_NUM_COUNT_OFFSET 1       

#define QWLAN_BMU_BD_INDEX_INFO1_FRAME_CTRL_MASK         0xffffe000
#define QWLAN_BMU_BD_INDEX_INFO1_FRAME_CTRL_OFFSET       13 
#define QWLAN_BMU_BD_INDEX_INFO2_BD_RATE_MASK            0x3
#define QWLAN_BMU_BD_INDEX_INFO2_BD_RATE_OFFSET          0 
#define QWLAN_BMU_BD_INDEX_INFO2_BD_INTR_MASK            0xc
#define QWLAN_BMU_BD_INDEX_INFO2_BD_INTR_OFFSET          2 
#define QWLAN_BMU_BD_INDEX_INFO2_BD_OWNER_MASK            0x30
#define QWLAN_BMU_BD_INDEX_INFO2_BD_OWNER_OFFSET          4

#define DAFCA_MEMORY_16K                                0x4000
#define DAFCA_MEMORY_32K                                0x8000
#define DAFCA_MEMORY_48K                                0xC000
#define DAFCA_MEMORY_64K                                0x10000
#define DAFCA_MEMORY_80K                                0x14000
#define DAFCA_MEMORY_128K                               0x20000
#define MIN_DAFCA_MEMORY                                DAFCA_MEMORY_16K

// ----------------
// BMU COMMAND API
// ----------------
void bmuCommand_push_wq(tpAniSirGlobal pMac, tANI_U32 wqIndex, tANI_U32 value);
void bmuCommand_pop_wq(tpAniSirGlobal pMac, tANI_U32 wqIndex, tANI_U32 *readValue);
void bmuCommand_write_wq_head(tpAniSirGlobal pMac, tANI_U32 wqIndex, tANI_U32 value);
void bmuCommand_read_wq_head(tpAniSirGlobal pMac, tANI_U32 wqIndex, tANI_U32 *readValue);
void bmuCommand_write_wq_tail(tpAniSirGlobal pMac, tANI_U32 wqIndex, tANI_U32 value);
void bmuCommand_read_wq_tail(tpAniSirGlobal pMac, tANI_U32 wqIndex, tANI_U32 *readValue);
void bmuCommand_write_wq_nr(tpAniSirGlobal pMac, tANI_U32 wqIndex, tANI_U32 value);
void bmuCommand_read_wq_nr(tpAniSirGlobal pMac, tANI_U32 wqIndex, tANI_U32 *readValue);
void bmuCommand_write_bd_pointer(tpAniSirGlobal pMac, tANI_U32 bdIndex, tANI_U32 value);
void bmuCommand_read_bd_pointer(tpAniSirGlobal pMac, tANI_U32 wqIndex, tANI_U32 *readValue);
eHalStatus bmuCommand_request_bd_pdu(tpAniSirGlobal pMac, tANI_U32 pdu, tANI_U32 bd, tANI_U32 module, tANI_U32 *readValue);
eHalStatus bmuCommand_get_bd_pdu(tpAniSirGlobal pMac, tANI_U32 req1, tANI_U32 req2, tANI_U32 module, tANI_U32 *readValue);
eHalStatus bmuCommand_release_bd(tpAniSirGlobal pMac, tANI_U32 bd, tANI_U32 module, tANI_U32 head, tANI_U32 tail);
eHalStatus bmuCommand_release_pdu(tpAniSirGlobal pMac, tANI_U32 pdu, tANI_U32 module, tANI_U32 head, tANI_U32 tail);

// ----------------------------
// BMU REGISTER READ/WRITE API
// ----------------------------
eHalStatus bmuReg_set_bdpdu_threshold(tpAniSirGlobal pMac, tANI_U32 index, tANI_U32 bdThresh, tANI_U32 pduThresh);
eHalStatus halBmu_Start(tHalHandle hHal, void *arg);
eHalStatus halBmu_InitBdPduBaseAddress(tpAniSirGlobal pMac);
eHalStatus halBmu_enableWq(tpAniSirGlobal pMac, tANI_U32 startWqIndex,  tANI_U32 lastWqIndex);
eHalStatus halBmu_disableWq(tpAniSirGlobal pMac, tANI_U32 startWqIndex,  tANI_U32 lastWqIndex);
void halBMU_getNumOfBdPdu(tpAniSirGlobal pMac, tANI_U32 *numOfBD, tANI_U32 *numOfPDU);

eHalStatus halBmu_sta_enable_disable_control(tpAniSirGlobal pMac, tANI_U32 staId, tANI_U32 staCfgCmd);
eHalStatus bmuCommand_get_tx_queue_info(tpAniSirGlobal pMac, tANI_U8 backoffId, 
			tANI_U8 *totalPktCnt, tANI_U8 *pktCntFromHeadPtr, 
			tANI_U8 *firstFrameRetryCnt, tANI_U8 *queueId, tANI_U8 *txStaId);
eHalStatus bmuCommand_read_btqm_index_entry1(tpAniSirGlobal pMac, tANI_U8 queueId, 
				tANI_U8 txStaId, tANI_U8 *retryCnt, tANI_U16 *bdIndex);
eHalStatus bmuCommand_read_btqm_index_entry2(tpAniSirGlobal pMac, tANI_U8 queueId, 
				tANI_U8 txStaId, tANI_U16 *sequenceNum, tANI_U16 *frameSize);

eHalStatus halBmu_get_qid_for_qos_tid(tpAniSirGlobal pMac, tANI_U8 tid, tANI_U8 *queueId);

eHalStatus halBmu_btqmStaTxWqStatus(tpAniSirGlobal pMac ,tANI_U8 staIdx, tANI_U32 *pbmuBtqmStaQueues);
eHalStatus halBmu_GetStaWqStatus(tpAniSirGlobal pMac, tpBmuStaQueueData pStaQueueData );
void halBmu_StaCfgForPM(tpAniSirGlobal pMac, tANI_U16 staIdx, tANI_U32 mask, tANI_U16 delEnbQIdMask, tANI_U16 trigEnbQIdMask);
void halBmu_UpdateStaBMUApMode(tpAniSirGlobal pMac, 
                                 tANI_U8 staIdx, tANI_U8 uapsdACMask, 
                                 tANI_U8 maxSPLen, tANI_U8 updateUapsdOnly);


/**
    Dynamic BD PDU Threshold types
*/
typedef enum  eBmuBdPduTh {
    eBMU_BD_PDU_TH_ANI,          /**< All AC have the Threshold for VO.*/
    eBMU_BD_PDU_TH_WMM,          /**< All AC have Default Thresholds.*/
    eBMU_BD_PDU_TH_INVERSE_WMM   /**< BE and VI is swapped to support AC Parameter Inversion.*/
} tBmuBdPduTh;


// BMU enable/disable sta transmission commands
typedef enum eBmuStaTxCfgCmd {
    eBMU_ENB_TX_QUE_DONOT_ENB_TRANS       = 0x0,
    eBMU_ENB_TX_QUE_ENB_TRANS             = 0x1,
    eBMU_DIS_TX_QUE_DIS_TRANS             = 0x2,
    eBMU_DIS_TX_QUE_DIS_TRANS_CLEANUP_QUE = 0x3,
} tBmuStaTxCfgCmd;

/** Qid 0 -7  for TID 0-7, 
 *   Qid 8 for MGMT
 *   Qid 9 non QoS Data.
 */
 /* Enabling Queue Id 0 to 11 */
#define TX_QID_ENABLE			          0x7FF
#ifdef BMU_ERR_DEBUG
eHalStatus halBmuTrace_printConfig(tpAniSirGlobal pMac);
#endif
eHalStatus halBmuTrace_setMaxIndex(tpAniSirGlobal pMac, tANI_U32 index);
eHalStatus halIntBMUErrorHandler(tHalHandle hHalHandle, eHalIntSources intSource);

tANI_U16 halBmu_getQidMask(tANI_U8 acApsdflag);
#ifndef WLAN_SOFTAP_FW_BA_PROCESSING_FEATURE
eHalStatus halBmu_ConfigureToSendBAR(tpAniSirGlobal pMac, tANI_U8 staIdx, tANI_U8 queueId);
#endif
eHalStatus halBmu_ReadBtqmQFrmInfo(tpAniSirGlobal pMac, tANI_U8 staIdx, tANI_U8 queueId, tANI_U32 *pNumFrames, 
                                tANI_U32 *pHeadBdIndex, tANI_U32 *pTailBdIndex );
void halBmu_btqmStaClearState(tpAniSirGlobal pMac, tANI_U8 staIdx);

eHalStatus halBmu_ReadBdInfo(tpAniSirGlobal pMac, tANI_U32 bdIdx, tpBmuBtqmBdInfo pBdInfo, tANI_U8 fDetailed );

eHalStatus halIntBmuWqHandler(tHalHandle hHalHandle, eHalIntSources intSource);

eHalStatus halBmu_BckupBtqmStaConfig(tpAniSirGlobal pMac, tANI_U32 *pAddr);
eHalStatus halBmu_getBtqmStaWqStatus(tpAniSirGlobal pMac, tANI_U8 staIdx, tANI_U32 *pbmuBtqmStatus);
eHalStatus halIntBMUIdleBdPduHandler(tHalHandle hHalHandle, eHalIntSources intSource);

#endif /* _HALBMU_H_ */






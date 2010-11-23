/*
 * Airgo Networks, Inc proprietary. All rights reserved.
 * halDPU.c:  Provides all the MAC APIs to the DPU Hardware Block.
 * Author:    Satish Gune
 * Date:      02/09/2006
 *
 * --------------------------------------------------------------------------
 */
#ifndef _HALDPU_H_
#define _HALDPU_H_

#include "halTypes.h"
#include "aniGlobal.h"      // tpAniSirGlobal, tAniEdType
#include "halMsgApi.h"      // tDpuStatsParams, tStaRateMode

/* DPU Error codes, as defined in 'DPU Error WQ CR' doc */
#define SWBD_DPUERR_BAD_TKIP_MIC       6

/* BIP element ID for 802.11w protected robust management frames */
#define QWLAN_DPU_DPU_BIP_ELEMENT_ID   0x0000004C

// KeyIds 
#define HAL_DPU_KEY_ID_0    0
#define HAL_DPU_KEY_ID_1    1
#define HAL_DPU_KEY_ID_2    2
#define HAL_DPU_KEY_ID_3    3
#define HAL_DPU_KEY_ID_4    4
#define HAL_DPU_KEY_ID_5    5

//For Libra, this is 2, for Volans, this is 3. 
//It specifies the number of 16-byte units needed for one RC descriptor for WAPI
#ifdef FEATURE_WLAN_WAPI
#ifdef LIBRA_WAPI_SUPPORT
#define HAL_WAPI_RC_DESCRIPTOR_COUNT    2
#else
#define HAL_WAPI_RC_DESCRIPTOR_COUNT    3
#endif
#endif /* FEATURE_WLAN_WAPI.*/

#define HAL_DPU_SELF_STA_DEFAULT_IDX 0 //dpu index 0 is used for self station entry when not associated
/* Added Descriptor Sequence structure as part of Dpu Descriptor */

typedef __ani_attr_pre_packed struct _tDpuAutoSeqNumRecord {
#ifdef ANI_BIG_BYTE_ENDIAN
		tANI_U32	tid1:12;
		tANI_U32	res1:4;
		tANI_U32	tid0:12;
		tANI_U32	res0:4;
#else
		tANI_U32	tid0:12;
		tANI_U32	res0:4;
		tANI_U32	tid1:12;
		tANI_U32	res1:4;
#endif
} __ani_attr_packed __ani_attr_aligned_4 tDpuAutoSeqNumRecord ;

#define DPU_AUTOSEQ_FIELD_LEN      2
#define DPU_AUTOSEQ_FIELD_MASK     1

typedef __ani_attr_pre_packed struct sDpuDescriptor {
    //word 0
#ifdef ANI_BIG_BYTE_ENDIAN
#if defined(LIBRA_WAPI_SUPPORT)
    tANI_U32 wapi : 1;
    tANI_U32 resv1 : 3;
#else
    tANI_U32 resv1 : 4;
#endif
    tANI_U32 txFragThreshold4B : 12; /* in units of 4Bytes*/
    tANI_U32 resv2: 7;
    tANI_U32 signature:3;
#ifdef WLAN_HAL_VOLANS
    tANI_U32 resv3:3;
    tANI_U32 wapiStaID:3;
#else
    tANI_U32 resv3:6;
#endif
#else // ENDIAN
#ifdef WLAN_HAL_VOLANS
    tANI_U32 wapiStaID:3;
    tANI_U32 resv3:3;
#else
    tANI_U32 resv3:6;
#endif
    tANI_U32 signature:3;
    tANI_U32 resv2: 7;
    tANI_U32 txFragThreshold4B : 12;
#if defined(LIBRA_WAPI_SUPPORT)
    tANI_U32 resv1 : 3;
    tANI_U32 wapi : 1;
#else
    tANI_U32 resv1 : 4;
#endif
#endif // ENDIAN

    //word 1
#ifdef ANI_BIG_BYTE_ENDIAN
    tANI_U32 keyIndex5d:8;
    tANI_U32 keyIndex4d:8;
    tANI_U32 resv4:16;
#else
    tANI_U32 resv4:16;
    tANI_U32 keyIndex4d:8;
    tANI_U32 keyIndex5d:8;
#endif

    //word 2
#ifdef ANI_BIG_BYTE_ENDIAN
    tANI_U32 keyIndex3:8;
    tANI_U32 keyIndex2:8;
    tANI_U32 keyIndex1:8;
    tANI_U32 keyIndex0:8;
#else
    tANI_U32 keyIndex0:8;
    tANI_U32 keyIndex1:8;
    tANI_U32 keyIndex2:8;
    tANI_U32 keyIndex3:8;
#endif

    //word 3
#ifdef ANI_BIG_BYTE_ENDIAN
    tANI_U32 resv5:2;
    tANI_U32 singleTidRc:1;
    tANI_U32 keyBase:1;
    tANI_U32 replayCountSet:4;
    tANI_U32 keyIndex5:8;
    tANI_U32 keyIndex4:8;
    tANI_U32 txKeyId:3;
    tANI_U32 resv6:2;
    tANI_U32 encryptMode:3;
#else
    tANI_U32 encryptMode:3;
    tANI_U32 resv6:2;
    tANI_U32 txKeyId:3;
    tANI_U32 keyIndex4:8;
    tANI_U32 keyIndex5:8;
    tANI_U32 replayCountSet:4;
    tANI_U32 keyBase:1;
    tANI_U32 singleTidRc:1;
    tANI_U32 resv5:2;
#endif

    //word 4-7
    //for TID0-15, each 8 bits.
    //Use GET_DPU_RCIDX() or SET_DPU_RCIDX() defined above
    //to read/write this field. It takes care of endian problem.
    tANI_U32 idxPerTidReplayCount[4];

    //word 8
    tANI_U32 txSentBlocks;

    //word 9
    tANI_U32 rxRcvddBlocks;

    //word 10
    // BIP/AES/TKIP replay check failure counts
    tANI_U32  replayCheckFailCount;

    //word 11
#ifdef ANI_BIG_BYTE_ENDIAN
    tANI_U32 micErrCount:8;
    tANI_U32 excludedCount:24;
#else
    tANI_U32 excludedCount:24;
    tANI_U32 micErrCount:8;
#endif

    //word 12
#ifdef ANI_BIG_BYTE_ENDIAN
    tANI_U32 formatErrorCount:16;
    tANI_U32 undecryptableCount:16;
#else
    tANI_U32 undecryptableCount:16;
    tANI_U32 formatErrorCount:16;
#endif

    //word 13
    tANI_U32 decryptErrorCount;

    //word 14
    tANI_U32 decryptSuccessCount;

    //word 15
#ifdef ANI_BIG_BYTE_ENDIAN
    tANI_U32 keyIdErr:8;
    tANI_U32 extIVerror:8;
    tANI_U32 reserved:16;
#else
    tANI_U32 reserved:16;
    tANI_U32 extIVerror:8;
    tANI_U32 keyIdErr:8;
#endif

    tDpuAutoSeqNumRecord	sequenceField[MAX_NUM_OF_TIDS/DPU_AUTOSEQ_FIELD_LEN];
	
} __ani_attr_packed __ani_attr_aligned_4 tDpuDescriptor;


typedef __ani_attr_pre_packed struct sDpuKeyDescriptor {
    tANI_U32 key128bit[4];
} __ani_attr_packed __ani_attr_aligned_4 tDpuKeyDescriptor;


typedef __ani_attr_pre_packed struct sDpuMicKeyDescriptor {
    tANI_U32 txMicKey64bit[2];
    tANI_U32 rxMicKey64bit[2];
} __ani_attr_packed __ani_attr_aligned_4 tDpuMicKeyDescriptor;


#ifdef FEATURE_WLAN_WAPI
typedef __ani_attr_pre_packed struct sDpuWpiMicKeyDescriptor {
    tANI_U32 wpiMicKey[4];
} __ani_attr_packed __ani_attr_aligned_4 tDpuWpiMicKeyDescriptor;
#endif


typedef  __ani_attr_pre_packed struct sDpuReplayCounterDescriptor {
    tANI_U32  txReplayCount31to0;
#ifdef ANI_BIG_BYTE_ENDIAN
    tANI_U32  txReplayCount47to32:16;
    tANI_U32  resv1:16;
#else
    tANI_U32  resv1:16;
    tANI_U32  txReplayCount47to32:16;
#endif
    tANI_U32  rxReplayCount31to0;

#ifdef ANI_BIG_BYTE_ENDIAN
    tANI_U32  rxReplayCount47to32:16;
    tANI_U32  resv2:6;
    tANI_U32  replayChkEnabled:1;
    tANI_U32  winChkEnabled:1;
    tANI_U32  winChkSize:8;
#else
    tANI_U32  winChkSize:8;
    tANI_U32  winChkEnabled:1;
    tANI_U32  replayChkEnabled:1;
    tANI_U32  resv2:6;
    tANI_U32  rxReplayCount47to32:16;
#endif

} __ani_attr_packed __ani_attr_aligned_4 tDpuReplayCounterDescriptor;


#ifdef FEATURE_WLAN_WAPI
typedef  __ani_attr_pre_packed struct sDpuWpiReplayCounterDescriptor {
    tANI_U32   txReplayCount[4];
    tANI_U32   rxReplayCount[4];
#ifndef LIBRA_WAPI_SUPPORT
#ifdef ANI_BIG_BYTE_ENDIAN
        tANI_U32  resv1:22;
        tANI_U32  replayChkEnabled:1;
        tANI_U32  winChkEnabled:1;
        tANI_U32  winChkSize:8;
#else
        tANI_U32  winChkSize:8;
        tANI_U32  winChkEnabled:1;
        tANI_U32  replayChkEnabled:1;
        tANI_U32  resv1:22;
#endif
        tANI_U32  resv2[3];
#endif /* LIBRA_WAPI_SUPPORT */
} __ani_attr_packed __ani_attr_aligned_4 tDpuWpiReplayCounterDescriptor;
#endif

typedef struct sHalDpuDescEntry
{
    tANI_U8         hwIndex;        /* the actual index on the hw */
    tANI_U8         used;           /* zero if entry is not used */
    tAniWepType     wepType;        /* Key type: Per Station Key or Static Key */
    tANI_U8         keyIdx;         /* idx of keys in the local KeyTable */
    tANI_U8         derivedKeyIdx;  /* IDx of the derived keys 4d/5d */
    tANI_U8         micKeyIdx;      /* idx of Mic in the local MicKeyTable */
    tANI_U8         rcIdx;          /* idx of ReplyCounter in the local RCDescTable */
    tDpuDescriptor  halDpuDescriptor;
} tHalDpuDescEntry;

typedef struct sHalDpuKeyEntry
{
    tANI_U8             hwIndex;    /* the actual index on the hw */
    tANI_U8             used;       /* zero if entry is not used */
    tDpuKeyDescriptor   halDpuKey;
} tHalDpuKeyEntry;

typedef struct sHalDpuMicKeyEntry
{
    tANI_U8                 hwIndex;    /* the actual index on the hw */
    tANI_U8                 used;       /* zero if entry is not used */
    tDpuMicKeyDescriptor    halDpuMicKey;
} tHalDpuMicKeyEntry;

typedef struct sHalDpuRCEntry {
    tANI_U8     hwIndex;        /* RC Index in HW [0..255] */
    tANI_U8     hwRCBaseIndex;  /* RC Base Index in HW [0..16] */
    tANI_U8     used;           /* zero if entry is not used */
    union
    {
        tDpuReplayCounterDescriptor halDpuRC[16];
#ifdef FEATURE_WLAN_WAPI
        tDpuWpiReplayCounterDescriptor halWpiDpuRC[16];
#endif
    } u;
} tHalDpuRCEntry;

typedef struct sDpuInfo {
    tHalDpuDescEntry       *descTable;
    tHalDpuKeyEntry        *keyTable;
    tHalDpuMicKeyEntry     *micKeyTable;
    tHalDpuRCEntry         *rcDescTable;
    tANI_U8                maxEntries;
} tDpuInfo, *tpDpuInfo;

#ifdef ANI_BIG_BYTE_ENDIAN

    #define HAL_SET_DPU_RCIDX( dpuDesc, tid, value) do{\
        ((tANI_U8 *)((tDpuDescriptor*)(dpuDesc))->idxPerTidReplayCount)[(tid)] = (value);\
    }while(0)
    #define HAL_GET_DPU_RCIDX(dpuDesc, tid)  (((tANI_U8 *)(dpuDesc)->idxPerTidReplayCount)[(tid)])
#else
    #define HAL_RCIDX_TO_BYTEIDX(tid) ( ((tid)&~3) | (3-((tid)&3)) )
    #define HAL_SET_DPU_RCIDX( dpuDesc, tid, value) do{\
        ((tANI_U8 *)((tDpuDescriptor*)(dpuDesc))->idxPerTidReplayCount)[HAL_RCIDX_TO_BYTEIDX(tid)] = (value);\
    }while(0)
    #define HAL_GET_DPU_RCIDX(dpuDesc, tid)  (((tANI_U8 *)(dpuDesc)->idxPerTidReplayCount)[HAL_RCIDX_TO_BYTEIDX(tid)])
#endif

eHalStatus halDpu_Start(tHalHandle hHal, void *arg);
eHalStatus halDpu_Stop(tHalHandle hHal, void *arg);
eHalStatus halDpu_Open( tHalHandle hHal, void *arg);
eHalStatus halDpu_Close(tHalHandle hHal, void *arg);

eHalStatus halDpu_AllocId(tpAniSirGlobal pMac, tANI_U8 *id);
eHalStatus halDpu_ReleaseId(tpAniSirGlobal pMac, tANI_U8 id);
eHalStatus halDpu_SetWepKeys(tpAniSirGlobal pMac, tANI_U8 dpuId,
                        tAniEdType encType, tANI_U8 defWepId,
                        tANI_U8 keyId0, tANI_U8 keyId1,
                        tANI_U8 keyId2, tANI_U8 keyId3);
#if defined(FEATURE_WLAN_WAPI) && !defined(LIBRA_WAPI_SUPPORT)
eHalStatus halDpu_SetDescriptorAttributes(tpAniSirGlobal pMac, tANI_U8 dpuIdx,
                        tAniEdType encType,
                        tANI_U8 keyIdx, tANI_U8 derivedKeyIdx, tANI_U8 micKeyIdx,
                        tANI_U8 rcIdx, tANI_U8 singleTidRc, tANI_U8 defKeyId,
                        tANI_U8 wapiStaID, tANI_BOOLEAN fGTK);
#else
eHalStatus halDpu_SetDescriptorAttributes(tpAniSirGlobal pMac, tANI_U8 dpuIdx,
                        tAniEdType encType,
                        tANI_U8 keyIdx, tANI_U8 derivedKeyIdx, tANI_U8 micKeyIdx,
                        tANI_U8 rcIdx, tANI_U8 singleTidRc, tANI_U8 defKeyId);
#endif
eHalStatus halDpu_ReleaseDescriptor(tpAniSirGlobal pMac, tANI_U8 id);
eHalStatus halDpu_AllocKeyId(tpAniSirGlobal pMac, tANI_U8 *id);
eHalStatus halDpu_ReleaseKeyId(tpAniSirGlobal pMac, tANI_U8 id);
eHalStatus halDpu_SetKeyDescriptor(tpAniSirGlobal pMac, tANI_U8 id,
                        tAniEdType encryptMode, tANI_U8 *pKey);
eHalStatus halDpu_GetStatus( tpAniSirGlobal pMac, tANI_U8 dpuIdx,
                        tpDpuStatsParams pDpuStatus );
eHalStatus halDpu_AllocMicKeyId(tpAniSirGlobal pMac, tANI_U8 *id, tANI_U8 keyId);
eHalStatus halDpu_ReleaseMicKeyId(tpAniSirGlobal pMac, tANI_U8 id);
eHalStatus halDpu_SetMicKeyDescriptor(tpAniSirGlobal pMac, tANI_U8 id,
                        tANI_U8 *pKey, tANI_U8 paeRole );
#if defined(FEATURE_WLAN_WAPI)
eHalStatus halDpu_SetWPIMicKeyDescriptor(tpAniSirGlobal pMac, tANI_U8 id,
                        tANI_U8 *pKey, tANI_U8 paeRole );
eHalStatus halDpu_SetWapiQos( tpAniSirGlobal pMac, tANI_BOOLEAN fSet );
#endif
eHalStatus halDpu_GetRCId( tpAniSirGlobal pMac, tANI_U8 dpuIndex, tANI_U8 *rcIndex );
eHalStatus halDpu_EnableRCWinChk( tpAniSirGlobal pMac, tANI_U8 dpuIndex, tANI_U32 queueId );
eHalStatus halDpu_DisableRCWinChk( tpAniSirGlobal pMac, tANI_U8 dpuIndex, tANI_U32 queueId );
eHalStatus halDpu_GetKeyId( tpAniSirGlobal pMac, tANI_U8 dpuIndex, tANI_U8 *keyIndex );
eHalStatus halDpu_GetMicKeyId( tpAniSirGlobal pMac, tANI_U8 dpuIndex, tANI_U8 *micKeyIndex );
eHalStatus halDpu_AllocRCId(tpAniSirGlobal pMac, tAniEdType encType, tANI_U8 *id);
eHalStatus halDpu_ReleaseRCId(tpAniSirGlobal pMac, tANI_U8 dpuIdx, tANI_U8 id);
eHalStatus halDpu_SetRCDescriptor( tpAniSirGlobal pMac, tANI_U8 id, tANI_U16 bRCE, tANI_U16 bWCE, tANI_U8 *winChkSize );
#if defined(FEATURE_WLAN_WAPI)
eHalStatus halDpu_SetWAPIRCDescriptor(tpAniSirGlobal pMac, tANI_U8 id, tANI_U8 *pTxRC, tANI_U8 *pRxRC);
#endif
eHalStatus halDpu_SetFragThreshold(tpAniSirGlobal pMac, tANI_U8 dpuIdx,
                        tANI_U16 fragSize);
eHalStatus halDpu_GetSignature(tpAniSirGlobal pMac, tANI_U8 dpuId,
                        tANI_U8 *sig );
void halDpu_SetTxReservedBdPdu(tpAniSirGlobal pMac);
eHalStatus halIntDPUErrorHandler(tHalHandle hHalHandle, eHalIntSources intSource);
void halDpu_HandleMICErrorInterrupt(tpAniSirGlobal pMac);
void halDpu_MICErrorIndication(tpAniSirGlobal pMac);
eHalStatus halDpu_BdRoutingFlagOverride(tpAniSirGlobal  pMac, tANI_U8 enable, tANI_U32 wqIdx);
eHalStatus halDpu_GetSequence(tpAniSirGlobal pMac, tANI_U8 dpuIdx, tANI_U8 tId, tANI_U16 *sequenceNum);
eHalStatus halDpu_ResetEncryMode(tpAniSirGlobal pMac, tANI_U8 dpuIdx);
#ifdef FEATURE_ON_CHIP_REORDERING
eHalStatus halDpu_SetReplayCheckForTID( tpAniSirGlobal pMac, tANI_U8 rcId, tANI_U8 tid, tANI_U16 bRCE);
#endif

#endif /* _HALDPU_H_ */


/**
 *
 *  @file:         halStaTable.h
 *
 *  @brief:       This provides the HAL Station structure.
 *
 *  @author:    Qualcomm Inc
 *
 *  Copyright (C) 2008, Qualcomm, Inc. All rights reserved.
 *
 *  Change History:
 * ---------------------------------------
 * 01/04/2007  File created.
 * 01/24/2008  Virgo related chages.
 */

#ifndef _HAL_STA_TABLE_H_
#define _HAL_STA_TABLE_H_

#include "halInternal.h"
#include "sirMacProtDef.h"
#include "halMsgApi.h"
#include "halRateAdaptApi.h"  // tHalRaInfo
#include "halMacBA.h"
#include "halTpe.h"
#include "halRpe.h"
#include "halAdu.h"
#include "halMacWmmApi.h"      //tCfgTrafficClass

/* some simple useful macros */
#define GET_WORD_COUNT(nBytes)          (((nBytes) + 3)>>2)
#define SWAP_WORD(t, f)     {t[3]=f[0]; t[2]=f[1]; t[1]=f[2]; t[0]=f[3];}

/* determines endianness by testing lsb value */
#define IS_LE(word, bValue)  ((* ((tANI_U8 *) &word)) == bValue)

#define ETH2_MPDU_DATA_OFFSET (SWBD_TX_MPDUDATA_OFFSET - \
                               sizeof(tLlcSnapHdr))

// BSSID struct
typedef struct
{
    tSirMacAddr bssId;
    tANI_U8 bssIdx;
    tANI_U8 dpuIdx;            
    tANI_U8 dpuIdxBcastMgmt;   // DPU index for IGTK
    tANI_U8 staIdForBss;       // Sta Index for the BSS
	tANI_U32 staIdBitmap[8];   // Sta Index bitmap for this BSS, indicating the STAs
    tANI_U8 valid:1;	
    tANI_U8 ssid[32];

	// Per AC ACM parameters that are configured as part of the EDCA parameter set
	tANI_BOOLEAN acm[MAX_NUM_AC];
    
    /* encryptMode and wepKeyIds are used for WEP mode only */
    tAniEdType encryptMode;
    tANI_U8 wepKeyIds[4];
    tANI_U8 bssType;
    tANI_U8 bcnDtimPeriod;
    tANI_U16 tuBeaconInterval;
    tHalRaBssInfo bssRaInfo;

    // Number of 11b Ibss peers.
    tANI_U8 numIbssllbPeerCnt;
} tBssStruct, * tpBssStruct;

/*
 * Struct to overwrite certain features per BD per TC.
 */
typedef struct
{
    tANI_U8 disableEncrypt;         // Overrides encryption - dupNE flag
    tANI_U8 disableCompress;        // Overrides compression - dpuNC flag
    tANI_U8 forceAckPolicy;         // Ack policy PE wants to force via BD field.
    tANI_U8 forceMacProtection;     // Mac protection via BD
    tANI_U8 overwritePhy;           // Overwrite PHY command (Use one in BD)

    tANI_U8 enableAggregate;        // Enable aggregate. For Titan STAs, it's concat.
                               // For 11n STA, it's A-MPDU.
    tANI_U8 rateIndex;              // Rate Index
    tANI_U8 pad;                    // Pad for alignment.
} tTCBDFlags;

typedef  struct 
{
    tANI_U8 dsap;
    tANI_U8 ssap;
    tANI_U8 control;

    // snap
    tANI_U8 oui[3];
    tANI_U16 proto;
} __ani_attr_packed tLlcSnapHdr;

#define mh3a macHdr.h3a
#define mh4a macHdr.h4a

typedef struct sHalCfgStaRateInfo {
    /* Reserved for now. Later more fields can be defined, such as beamformEnable.
     * Or more modes can be added later.
     */
    tANI_U32 reserved1 : 9;

 
    /* As global rate table doesn't distinguish between green-field and mixed mode,
     * rateIndex is not enough for generation MPI command properly. Mode distinguish
     * whether it needs to transmit in mixed mode. To easily find number of spatial
     * streams without reading global rate table, this mode also include number of
     * spatial streams.
     */
    tANI_U32 mode : 2;

    /* see SMACCFG_RATE_PROTECTION_XXX description above */
    tANI_U32 protectionPolicy: 2;


    /* L-SIG TXOP is used to protect entire TXOP */
    tANI_U32 fPolicyLsigTxop : 1;

    /* A-MPDU is enabled for this rate
     * Global rate table is common for all station, and A-MPDU flag is determined
     * per station/rate for 11n rates.
     */
    tANI_U32 ampduFlag : 1;

    /* Valid only when protectionPolicy = SMACCFG_RATE_PROTECTION_RTSBYTHRESHOLD.
     *  If policyRtsThreshold is set to SMACCFG_RATE_RTS_BY_PSDULEN, RTS/CTS exchange should be used when 
     *  PSDU length is greater than the global rtsThreshold. If set to SMACCFG_RATE_RTS_BY_PSDUMEDIUMTIME, RTS/CTS 
     *  exchange should be used when medium time for PSDU is greater than the 
     *  global usRTSmediumTimeThreshold.
     */
    tANI_U32 policyRtsThreshold: 1;

    /* Rate index. This is index to global rate table. */
    tANI_U32 rateIndex : 8;

    tANI_U32 protectionRateIdx: 8;

} tHalCfgStaRateInfo;

typedef struct sHalCfgSta {
    //0:Don't send MIMO sequences to this STA
    //1:an RTS should precede all MIMO sequences to this STA
    //2:this STA doesn't support MIMO pwr save
    //3:this STA has no limit on receiving MIMO sequence usage
    //Status: TODO
    tANI_U8 mimoPwrSaveMode;

    /* TID to AC mapping. From LSB, each 2 bits are allocated to each TID.
     * 2 bits data has "AC" index for corresponding TID. For example, 
     * 0xfa41 maps TID 0 and 3 to AC_BE, TID 1 and 2 to AC_BK, and so on.
     */
    tANI_U32 txTidAcMap;

    /* Tpe Rate Information per sta for each of the primary, secondary & 
     * tertiary rates for 20MHz */
    tTpeStaDescRateInfo rateInfo[HAL_RA_MAX_RATES][HAL_RA_TXRATE_CHANNEL_NUM];

    //TC related config
    tCfgTrafficClass tcCfg[STACFG_MAX_TC];
} tHalCfgSta;

// Station structure
typedef struct
{
    tSirMacAddr staAddr;                // Sta Addr
    tSirMacAddr bssId;                  // BSSid
    tANI_U16 assocId;

    tANI_U8 dpuSig:4;                        // DPU signiture
    tANI_U8 staSig:4;                        // STA signature
    tANI_U8 valid:1;                           // Used/free flag    
    tANI_U8 rmfEnabled:1;
    tANI_U8 htEnabled:1;
    tANI_U8 gfEnabled:1;
    tANI_U8 qosEnabled:1;                      // 11e or WMM enabled, flag used for header length
    

    tANI_U8 bssIdx;                         // BSS Index
    tANI_U8 staId;
    tANI_U8 staType;
    tAniEdType encMode;

    /****************** RA FIELDS ************************/
    // Rate adaptation state
    tHalRaInfo      halRaInfo;

    /// Tx/fail counts read at the beginning of current rate adaptation window
    tANI_U32 prevRxCount;

    // HDD updates this counter for every packet received.
	tANI_U32 rxPkts; 

    //# of packets success transmitted to this STA with either an ACK or BA received
    //This is updated by rate adaptation every 20msec.
    tANI_U32 txAckPkts;

    /// Whether receive is active or not
    tANI_U8  rxActive;

    /*********************** RA FIELDS END *******************************/

    tANI_U8 dpuIndex;                        // DPU table index
    tANI_U8 bcastDpuIndex;
    tANI_U8 bcastMgmtDpuIndex;
  
    /** HAL Station Config.*/
    tHalCfgSta staParam;

    tANI_U8 maxAmpduDensity;

    /* Copy of Btqm Sta config parameter */
    tBmuStaTxCfgCmd txConfig;

	/** Maintan TPE STA Desc */
	tTpeStaDesc tpeStaDesc;

    /** Maintain a copy of RPE STA Desc */
	tRpeStaDesc rpeStaDesc;

    // BA Session ID maintained on a per-TID basis
    // initialized to BA_SESSION_ID_INVALID when we add a new station in the table.
    tANI_U16 baSessionID[STACFG_MAX_TC];

    //to store addBAReq message per tid to be used in processing response back
    tSavedAddBAReqParamsStruct addBAReqParams[STACFG_MAX_TC];

    //per TID activity monitoring
    tANI_U32 framesTxed[STACFG_MAX_TC];

    //per TID stat as of last polling at global BA timeout
    tANI_U32 framesTxedLastPoll[STACFG_MAX_TC];
    // per Sta the umaIdx and Bcast Idx used in IBSS 
    tANI_U8 umaIdx;
    tANI_U8 umaBcastIdx;

    /* Adding a bitmap of TIDs which stores BA sessions established */
    tANI_U8  baInitiatorTidBitMap;
    tANI_U8  baReceipientTidBitMap;

    // For IBSS the mode of the STA
    tStaRateMode  opRateMode;
} tStaStruct, *tpStaStruct;

#if defined(ANI_OS_TYPE_LINUX)
typedef struct _staCacheEntry {
    tANI_U32 macAddressLo;
    tANI_U16 macAddressHi;
    tANI_U8 nextCacheIdx;
    tANI_U8 staId;
} tStaCacheEntry  __ani_attr_aligned_32;

#define STA_CACHE_SIZE 256
#endif
#endif /* _HAL_STA_TABLE_H_ */

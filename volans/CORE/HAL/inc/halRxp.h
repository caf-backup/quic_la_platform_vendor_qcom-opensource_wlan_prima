/*
 * Airgo Networks, Inc proprietary. All rights reserved.
 * halRxp.c:  Provides all the MAC APIs to the RXP Hardware Block.
 * Author:    Susan Tsao
 * Date:      02/15/2006
 * --------------------------------------------------------------------------
 */
#ifndef _HALRXP_H_
#define _HALRXP_H_

#include "halTypes.h"
#include "palTypes.h"
#include "aniGlobal.h"
#include "sirMacProtDef.h" // tSirMacAddr
#include "halMsgApi.h"

#ifdef WLAN_SOFTAP_FEATURE
  #define   BROADCAST_STAID    252
#else
  #define   BROADCAST_STAID    255
#endif

#define   RXP_DROP_ALL_FRAME_TYPES   0xffffffff
#define   RXP_PASS_ALL_FRAME_TYPES   0x0

#define RXP_TABLE_ADDR1     1
#define RXP_TABLE_ADDR2     2
#define RXP_TABLE_ADDR3     3

#define RXP_TABLE_VALID     1
#define RXP_TABLE_EMPTY     0

typedef enum
{
    RX_QID0,
    RX_QID1,
    RX_QID2,
    RX_QID3,
    RX_QID4,
    RX_QID5,
    RX_QID6,
    RX_QID7,
    RX_QID8,
    RX_QID9,
    RX_QID10,
    RX_QID11,
    RX_QID12,
    RX_QID13,
    RX_QID14,
    RX_QID15,

    RX_QUEUE_TID_0 = RX_QID0,
    RX_QUEUE_TID_1,
    RX_QUEUE_TID_2,
    RX_QUEUE_TID_3,
    RX_QUEUE_TID_4,
    RX_QUEUE_TID_5,
    RX_QUEUE_TID_6,
    RX_QUEUE_TID_7,
    RX_QUEUE_MGMT_nQOS = RX_QID8,
}tRxQId;

/* role - used in filling rxp binary search */
typedef enum eRxpRole {
    eRXP_SELF = 0, //self station
    eRXP_PEER_AP = 1, //adding peer AP in infrastructure network.
    eRXP_PEER_STA = 2, //adding peer STA
    eRXP_BSSID = 3 //adding bssid in IBSS network.
} tRxpRole;

typedef enum eRxpBcnBssidFilter {

	eRXP_BCN_BSSID2,
	eRXP_BCN_BSSID3
} tRxpBcnBssidFilter;

typedef enum eRxpTsfProcessingMode {
    eRXP_TSF_MODE_STA =0,
    eRXP_TSF_MODE_IBSS =1
} tRxpTsfProcessingMode;


typedef enum sHalRxpModeFlag  
{
    eHAL_USE_BSS_RXP, // Implies that only BSS rxpMode is active
    eHAL_USE_GLOBAL_AND_BSS_RXP // Implies that BSS rxpMode and global rxpMode is active
} tHalRxpModeFlag;

typedef struct sRxpAddrTable {
    tANI_U8    macAddr[6];              // MAC address
    tANI_U8    staid;                   // Station ID
    tANI_U8    dropBit:1;                 // Drop the frame on hit
    tANI_U8    rmfBit:1;                  // For Robust Management Frames
    tANI_U8    ftBit:1;                   // Frame Translation bit
    tANI_U8    dpuNE:1;                   // No Encryptoion bit
    tANI_U8    wepKeyIdxExtractEnable:1;
    tANI_U8    rsvd[3];
    tANI_U32   dpuPTKDescIdx;           // DPU Desc Id for UC Data/Mgmt Frames
    tANI_U32   dpuGTKDpuIdx;            // Routing Flag OR DPU Desc Id for MC/BC Data Frames
    tANI_U32   dpuIGTKDpuIdx;           // DPU Desc Id for MC/BC Mgmt Frames
    tANI_U32   dpuPTKSig;               // DPU signature for UC Data/Mgmt Frames
    tANI_U32   dpuGTKSig;               // DPU signature for MC/BC Data Frames
    tANI_U32   dpuIGTKTag;              // DPU signature for MC/BC Mgmt Frames

} tRxpAddrTable;

typedef struct sAddrInfo {
    tANI_U8    numOfEntry;
    tANI_U8    lowPtr;
    tANI_U8    highPtr;
} tAddrInfo;

typedef struct sRxpInfo {
    tRxpAddrTable   addr1_table[RXP_MAX_TABLE_ENTRY];
    tRxpAddrTable   addr2_table[RXP_MAX_TABLE_ENTRY];
    tRxpAddrTable   addr3_table[RXP_MAX_TABLE_ENTRY];
    tAddrInfo       addr1;
    tAddrInfo       addr2;
    tAddrInfo       addr3;
} tRxpInfo, *tpRxpInfo;

#define RXP_WARMUP_COUNTER_START_WARMUP_COUNTER_START_2_4GHZ_BAND_DEFAULT  725
#define RXP_WARMUP_COUNTER_START_WARMUP_COUNTER_START_5GHZ_BAND_DEFAULT    1445
#define RXP_BROADCAST_ENTRY_PRESENT 					   0x1

// Function prototypes.
eHalStatus halRxp_Open( tHalHandle hHal, void *arg);
eHalStatus halRxp_Start(tHalHandle hHal, void *arg);
eHalStatus halRxp_Stop(tHalHandle hHal, void *arg);
eHalStatus halRxp_Close(tHalHandle hHal, void *arg);

eHalStatus halRxp_AddEntry(tpAniSirGlobal pMac, tANI_U8 staid, tSirMacAddr macAddr, tRxpRole role,
        tANI_U8 rmfBit,	tANI_U8 dpuIdx, tANI_U8 dpuRFOrMcBcIdx, tANI_U8 dpuMgmtMcBcIdx,
        tANI_U8 dpuTag, tANI_U8 dpuDataMcBcTag, tANI_U8 dpuMgmtMcBcTag,
        tANI_U8 dpuNE, tANI_U8 ftBit, tANI_BOOLEAN keyIdExtract);
eHalStatus halRxp_DelEntry(tpAniSirGlobal pMac, tSirMacAddr macAddr);
eHalStatus halRxp_UpdateEntry(tpAniSirGlobal pMac, tANI_U8 staid, tRxpRole role, tANI_BOOLEAN keyExtract);
eHalStatus halRxp_DelAllEntries(tpAniSirGlobal pMAc);
eHalStatus halRxp_enable(tpAniSirGlobal pMac);
eHalStatus halRxp_disable(tpAniSirGlobal pMac);
eHalStatus halRxp_addBroadcastEntry(tpAniSirGlobal pMac);
#if 0 //remove
void       halRxp_storeRxpMode(tpAniSirGlobal pMac, tRxpMode mode);
#endif
tRxpMode   halRxp_getSystemRxpMode(tpAniSirGlobal pMac);
eHalStatus halRxp_AddBeaconBssidFilter(tpAniSirGlobal pMac, tSirMacAddr bssId);

eHalStatus halRxp_setOperatingRfBand(tpAniSirGlobal pMac, eRfBandMode rfBand);
eHalStatus halRxp_setChannel(tpAniSirGlobal pMac, tANI_U8 channel);
tANI_U8    halRxp_getChannel(tpAniSirGlobal pMac);
void halRxp_setScanLearn(tpAniSirGlobal pMac, tANI_U8 scanLearn);



void halRxp_setRxpFilterMode(tpAniSirGlobal pMac, tRxpMode rxpMode);
eHalStatus halRxpDbg_PrintFilter(tpAniSirGlobal pMac);
void       halRxpDbg_PrintSearchTable(tpAniSirGlobal pMac);
void       halRxpDbg_dumpReg( tpAniSirGlobal pMac, tANI_U32 arg1);
eHalStatus halRxp_routeDpuControl( tpAniSirGlobal pMac);
void halRxp_SetTsfCompensationValues(tpAniSirGlobal pMac);
eHalStatus halRxp_ErrIntHandler(tHalHandle hHalHandle, eHalIntSources intSource);
eHalStatus halRxp_EnableDisableBmuBaUpdate(tpAniSirGlobal pMac, tANI_U32 enable);

eHalStatus halRxp_BckupRxpSearchTable(tpAniSirGlobal pMac, tANI_U32 *memAddr);

eHalStatus halRxp_DisableBssBeaconParamFilter( tpAniSirGlobal pMac, tANI_U32 uBssIdx);
eHalStatus halRxp_EnableBssBeaconParamFilter( tpAniSirGlobal pMac, tANI_U32 uBssIdx);
eHalStatus halRxp_EnableSSIDBasedFilter( tpAniSirGlobal pMac, tSirMacSSid *pSirMacSSid);

void halRxp_setSystemRxpFilterMode(tpAniSirGlobal pMac, 
        tRxpMode rxpMode, tHalRxpModeFlag mode_flag);
void halRxp_setBssRxpFilterMode(tpAniSirGlobal pMac, 
        tRxpMode rxpMode, tANI_U8 *bssid, tANI_U8 bssIdx);
void setFrameFilterMaskForScan (tpAniSirGlobal pMac, tHalRxpModeFlag rxpMode);
tANI_U32 halRxp_getFrameFilterMaskForMode (tpAniSirGlobal pMac, tANI_U32 rxpMode);
void halRxp_setFrameFilterMaskForBcnProbeRsp(tpAniSirGlobal pMac, tANI_U32 maskValue);
void halRxp_configureRxpFilterMcstBcst(tpAniSirGlobal pMac, tANI_BOOLEAN setFilter);
#endif /* _HALRXP_H_ */


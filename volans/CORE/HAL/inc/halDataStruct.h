/*
 * Woodside Networks, Inc proprietary. All rights reserved.
 * halDataStruct.h: Taurus related data structures and definitions
 * Author:  V. K. Kandarpa
 * Date:    03/01/2002
 * Copy Right:  WOODSIDE NETWORKS, Redwood City, CA
 * History:-
 * Date     Modified by         Modification Information
 * --------------------------------------------------------------------------
 *
 */

#ifndef __HAL_DATA_STRUCT_H__
#define __HAL_DATA_STRUCT_H__

#include "sirTypes.h"
#include "sirMacProtDef.h"
#include "aniCompiler.h"
#include "halMacWmmApi.h" //tCfgTrafficClass

#include "wlan_qct_hal.h"

#define HAL_MAX_NUM_TC       8

typedef tHalRxBd tHalBufDesc;
typedef tHalRxBd *tpHalBufDesc;

/// PHY mode enumeration
#define HAL_PHY_MODE_11A         0
#define HAL_PHY_MODE_11B_SHORT   1
#define HAL_PHY_MODE_11G         2
#define HAL_PHY_MODE_11A_MIMO    4
#define HAL_PHY_MODE_11B_LONG    5
#define HAL_PHY_MODE_11G_MIMO    6

/* 11b Phy rates */
#define HAL_PHY_RATE_1                  0x01
#define HAL_PHY_RATE_2                  0x02
#define HAL_PHY_RATE_5_5                0x03
#define HAL_PHY_RATE_11                 0x04

/* 11b Phy rates as used in buffer descriptors (100 Kbits/s) */
#define HAL_PHY_RATE_BUFDSC_1           0x0a
#define HAL_PHY_RATE_BUFDSC_2           0x14
#define HAL_PHY_RATE_BUFDSC_5_5         0x37
#define HAL_PHY_RATE_BUFDSC_11          0x6e

/* 11a/g Phy rates */
#define HAL_PHY_RATE_6                  0x0b
#define HAL_PHY_RATE_9                  0x0f
#define HAL_PHY_RATE_12                 0x0a
#define HAL_PHY_RATE_18                 0x0e
#define HAL_PHY_RATE_24                 0x09
#define HAL_PHY_RATE_36                 0x0d
#define HAL_PHY_RATE_48                 0x08
#define HAL_PHY_RATE_54                 0x0c

/* mimo Phy rates */
#define HAL_PHY_RATE_MIMO_72            HAL_PHY_RATE_36
#define HAL_PHY_RATE_MIMO_96            HAL_PHY_RATE_48
#define HAL_PHY_RATE_MIMO_108           HAL_PHY_RATE_54

/* simo-cb Phy rates */
#define HAL_PHY_RATE_CB_12              HAL_PHY_RATE_6
#define HAL_PHY_RATE_CB_18              HAL_PHY_RATE_9
#define HAL_PHY_RATE_CB_20              0x03
#define HAL_PHY_RATE_CB_24              HAL_PHY_RATE_12
#define HAL_PHY_RATE_CB_36              HAL_PHY_RATE_18
#define HAL_PHY_RATE_CB_40              0x02
#define HAL_PHY_RATE_CB_42              0x06
#define HAL_PHY_RATE_CB_48              HAL_PHY_RATE_24
#define HAL_PHY_RATE_CB_72              HAL_PHY_RATE_36
#define HAL_PHY_RATE_CB_80              0x01
#define HAL_PHY_RATE_CB_84              0x05
#define HAL_PHY_RATE_CB_96              HAL_PHY_RATE_48
#define HAL_PHY_RATE_CB_108             HAL_PHY_RATE_54
#define HAL_PHY_RATE_CB_120             0x00
#define HAL_PHY_RATE_CB_126             0x04

/* mimo-cb Phy rates */
#define HAL_PHY_RATE_MIMO_CB_48         HAL_PHY_RATE_CB_24
#define HAL_PHY_RATE_MIMO_CB_72         HAL_PHY_RATE_CB_36
#define HAL_PHY_RATE_MIMO_CB_80         HAL_PHY_RATE_CB_40
#define HAL_PHY_RATE_MIMO_CB_84         HAL_PHY_RATE_CB_42
#define HAL_PHY_RATE_MIMO_CB_96         HAL_PHY_RATE_CB_48
#define HAL_PHY_RATE_MIMO_CB_144        HAL_PHY_RATE_CB_72
#define HAL_PHY_RATE_MIMO_CB_160        HAL_PHY_RATE_CB_80
#define HAL_PHY_RATE_MIMO_CB_168        HAL_PHY_RATE_CB_84
#define HAL_PHY_RATE_MIMO_CB_192        HAL_PHY_RATE_CB_96
#define HAL_PHY_RATE_MIMO_CB_216        HAL_PHY_RATE_CB_108
#define HAL_PHY_RATE_MIMO_CB_240        HAL_PHY_RATE_CB_120


#define HAL_RSSI_OFFSET        100

#ifdef WLAN_HAL_VOLANS
#define HAL_RXFIR0_MASK             0xFF000000
#define HAL_RXFIR1_MASK             0x00000000
#define HAL_PHY_STATS0_RSSI_OFFSET  24
#else /* Libra */
#define HAL_RXFIR0_MASK       0x0000FF00
#define HAL_RXFIR1_MASK       0x000000FF
#define HAL_PHY_STATS0_RSSI_OFFSET  8
#endif

#define HAL_GET_RSSI0_DB(phystats0)  (((phystats0  & HAL_RXFIR0_MASK) >> HAL_PHY_STATS0_RSSI_OFFSET) - HAL_RSSI_OFFSET)
#define HAL_GET_RSSI1_DB(phystats0)  (((phystats0) & HAL_RXFIR1_MASK) - HAL_RSSI_OFFSET)
#define HAL_MAX_OF_TWO(val1, val2)   ( ((val1) > (val2)) ? (val1) : (val2))

#define HAL_GET_RSSI_DB(phystats0)    HAL_MAX_OF_TWO(HAL_GET_RSSI0_DB(phystats0), HAL_GET_RSSI1_DB(phystats0))


#define SIR_MAC_BD_TO_MPDUHEADER_OFFSET(pBd, offset) 	(tpSirMacMgmtHdr) ((tANI_U8 *) (pBd) + (offset))
#define SIR_MAC_BD_TO_MPDUHEADER(pBd)        			(tpSirMacMgmtHdr) ((tANI_U8*)pBd + (tANI_U8)((tpHalBufDesc) pBd)->mpduHeaderOffset)
#define SIR_MAC_BD_TO_PSPOLLHEADER(pBd)        			(tpSirMacPSpoll) ((tANI_U8*)pBd + (tANI_U8)((tpHalBufDesc) pBd)->mpduHeaderOffset)
#define SIR_MAC_BD_TO_MPDUHEADER3A(pBd)        		(tpSirMacDataHdr3a) ((tANI_U8*)pBd + (tANI_U8)((tpHalBufDesc) pBd)->mpduHeaderOffset)
#define SIR_MAC_BD_TO_MPDUHEADER4A(pBd)        		(tpSirMacDataHdr4a) ((tANI_U8*)pBd + (tANI_U8)((tpHalBufDesc) pBd)->mpduHeaderOffset)
#define SIR_MAC_BD_TO_MPDUDATA_OFFSET(pBd, offset)   	(tANI_U8 *) ((tANI_U8 *) (pBd) + (offset))
#define SIR_MAC_BD_TO_MPDUDATA(pBd)                  		(tANI_U8 *)((tANI_U8 *)pBd + (tANI_U8)((tpHalBufDesc) pBd)->mpduDataOffset)
#define SIR_MAC_BD_TO_MPDUHEADER_LEN(pBd)       		(tANI_U16)(((tpHalBufDesc) pBd)->mpduHeaderLength)
#define SIR_MAC_BD_TO_MPDU_LEN(pBd)       				(tANI_U16)(((tpHalBufDesc) pBd)->mpduLength)
#define SIR_MAC_BD_TO_PAYLOAD_LEN(pBd)       			SIR_MAC_BD_TO_MPDU_LEN(pBd)  - SIR_MAC_BD_TO_MPDUHEADER_LEN(pBd)
#define SIR_MAC_BD_TO_RX_CHANNEL(pBd)                   (tANI_U8)(((tpHalBufDesc) pBd)->rxChannel)
#define SIR_MAC_BD_TO_IBSS_TSF_LATER(pBd)            (tANI_U16)((((tpHalBufDesc) pBd)->rtsf))
#define SIR_MAC_BD_TO_IBSS_BCN_SENT(pBd)             (tANI_U16)(((tpHalBufDesc) pBd)->bsf)
#define SIR_MAC_BD_IS_UNKNOWN_UCAST_FRAME(pBd)          ((((tpHalBufDesc) pBd)->addr2Index == HWBD_RX_UNKNOWN_UCAST) ? true : false)
#define SIR_MAC_BD_TO_PHY_STATS0(pBd)            (((tpHalBufDesc) pBd)->phyStats0)
#define SIR_MAC_BD_TO_PHY_STATS1(pBd)            (((tpHalBufDesc) pBd)->phyStats1)
#define SIR_MAC_BD_TO_RATE_INDEX(pBd)            (tANI_U8)(((tpHalBufDesc) pBd)->rateIndex)
#define SIR_MAC_BD_TO_SCAN_LEARN(pBd)            (tANI_U8)(((tpHalBufDesc) pBd)->scanLearn)
#if defined WLAN_FEATURE_VOWIFI
#define SIR_MAC_BD_RX_TIMESTAMP(pBd)               (tANI_U32)(((tpHalBufDesc) pBd)->mclkRxTimestamp)
#endif

#define SIR_MAC_BD_TO_RSSI_DB(pBd)               HAL_GET_RSSI_DB(SIR_MAC_BD_TO_PHY_STATS0(pBd))

#define WLANTL_HO_IS_AN_AMPDU                    0x4000
#define WLANTL_HO_LAST_MPDU_OF_AMPDU             0x400

#define WLAN_HAL_IS_AN_AMPDU(pBD)                (WLANHAL_RX_BD_GET_RXP_FLAGS(pBD) & WLANTL_HO_IS_AN_AMPDU)
#define WLAN_HAL_IS_LAST_MPDU(pBD)               (WLANHAL_RX_BD_GET_RXP_FLAGS(pBD) & WLANTL_HO_LAST_MPDU_OF_AMPDU) 


/*
 * BMU master ID assignment
 *
 * BMU master ID is used to identify master sending commands to BMU.
 * Master index should be accompanied with BMU GET IDLE or RELEASE
 * commands. Each master ID is associated with BD_PDU threshold, and
 * BD_PDU reservation slot. And BD_PDU_AVAIL signal is controlled 
 * based on available BD/PDU count and BD/PDU threshold. That is 
 * threshold is set per master ID not per WQ index. This mechanism
 * guarantees high priority module can get BD or PDUs when the availale
 * BD/PDU counts low. 
 *
 * RXP and DPU have fixed IDs, and MCPU and DXE master IDs are
 * software programmable. Current assignment is as follows
 *
 * 0 : RXP (1)
 * 1 - 2 : DPU (2)
 * 3 - 9 : DXE (7)
 * 10 : mCPU (1)
 *
 * Taurus has 10 DXE channels, and half of DXE channels are configured
 * for TX. DXE TX channels only request BDs and PDUs, while DXE RX 
 * channels only release BDs and PDUs. So master IDs are used for TX 
 * channels to limit BD and PDU allocation. DXE RX channel can share
 * same master index, but won't be affected by any threshold.
 *
 * There are only 5 DXE TX channels, but 7 master indices are reserved
 * for DXE TX, for later DXE TX channel counts can be increased.
 */

enum {
    BMU_MASTERID_0,
    BMU_MASTERID_1,
    BMU_MASTERID_2,
    BMU_MASTERID_3,
    BMU_MASTERID_4,
    BMU_MASTERID_5,
    BMU_MASTERID_6,
    BMU_MASTERID_7,
    BMU_MASTERID_8,
    BMU_MASTERID_NUM,

    /* Hardware defined */
    BMU_MASTERID_RXP = BMU_MASTERID_0,
    BMU_MASTERID_DPU_TX = BMU_MASTERID_1,
    BMU_MASTERID_DPU_RX = BMU_MASTERID_2,

    /* Software defined - DXE */
    BMU_MASTERID_DXE_START = BMU_MASTERID_5,
    BMU_MASTERID_DXE_END = BMU_MASTERID_7,
    
    BMU_MASTERID_DXE_0 = BMU_MASTERID_5,  //TX	 
    BMU_MASTERID_DXE_1,                   //RX
    BMU_MASTERID_DXE_2,                   //FW
    BMU_MASTERID_DXE_RCV = BMU_MASTERID_DXE_1,
};

#endif  // __HAL_DATA_STRUCT_H__

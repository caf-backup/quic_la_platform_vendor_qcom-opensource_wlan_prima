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


#define HAL_RXFIR0_MASK       0x0000FF00
#define HAL_RXFIR1_MASK       0x000000FF
#define HAL_RSSI_OFFSET        100

#define HAL_GET_RSSI0_DB(phystats0)  (((phystats0  & HAL_RXFIR0_MASK) >> 8) - HAL_RSSI_OFFSET)
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

#define SIR_MAC_BD_TO_RSSI_DB(pBd)               HAL_GET_RSSI_DB(SIR_MAC_BD_TO_PHY_STATS0(pBd))
#define SIR_MAC_BD_IS_VALID_HDR_OFFSET(pBd)       ((tANI_U32)(((tpHalBufDesc) pBd)->mpduHeaderOffset) < HAL_BD_SIZE)
#define SIR_MAC_BD_IS_VALID_DATA_OFFSET(pBd)       ((tANI_U32)(((tpHalBufDesc) pBd)->mpduDataOffset) < (HAL_BD_SIZE+HAL_PDU_SIZE))

#define WLANTL_HO_IS_AN_AMPDU                    0x4000
#define WLANTL_HO_LAST_MPDU_OF_AMPDU             0x400

#define WLAN_HAL_IS_AN_AMPDU(pBD)                (WLANHAL_RX_BD_GET_RXP_FLAGS(pBD) & WLANTL_HO_IS_AN_AMPDU)
#define WLAN_HAL_IS_LAST_MPDU(pBD)               (WLANHAL_RX_BD_GET_RXP_FLAGS(pBD) & WLANTL_HO_LAST_MPDU_OF_AMPDU) 
#endif  // __HAL_DATA_STRUCT_H__

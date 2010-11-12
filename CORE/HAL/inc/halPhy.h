/**
 *
   Airgo Networks, Inc proprietary.
   All Rights Reserved, Copyright 2005
   This program is the confidential and proprietary product of Airgo Networks Inc.
   Any Unauthorized use, reproduction or transfer of this program is strictly prohibited.


   halPhy.h: Hal Phy data types and definitions for run-time interfaces in halPhy.c
   Author:  Mark Nelson
   Date:    4/1/05

   History -
   Date        Modified by              Modification Information
  --------------------------------------------------------------------------

 */

#ifndef HALPHY_H
#define HALPHY_H
#include <halRfTypes.h>

//for 30second periodic interrupt, do this every 5 minutes
#define HAL_PHY_PERIODIC_CAL_ITER_LIMIT       10

typedef enum
{
    //these show which rx and tx chains are enabled, other chains are disable accordingly
    //Production modes
    PHY_CHAIN_SEL_R0_T0_ON,
    PHY_CHAIN_SEL_R0R1_T0_ON,

    PHY_CHAIN_SEL_BT_R0_T0_ON,      //simultaneous bluetooth receive enabled
    PHY_CHAIN_SEL_BT_R0R1_T0_ON,    //simultaneous bluetooth receive enabled


    //test modes
    PHY_CHAIN_SEL_R0_ON,
    PHY_CHAIN_SEL_R0R1_ON,

    PHY_CHAIN_SEL_T0_ON,

    PHY_CHAIN_SEL_T0_R1_ON,

    PHY_CHAIN_SEL_R1_ON,

    PHY_CHAIN_SEL_NO_RX_TX,

    MAX_PHY_CHAIN_SEL,
    INVALID_PHY_CHAIN_SEL,
    PHY_MAX_CHAIN_SELECT = 0xFFFFFFFF  /* define as 4 bytes data */
}ePhyChainSelect;

typedef enum
{
    PHY_SINGLE_CHANNEL_CENTERED = 0,        // 20MHz IF bandwidth centered on IF carrier
    PHY_DOUBLE_CHANNEL_LOW_PRIMARY = 1,     // 40MHz IF bandwidth with lower 20MHz supporting the primary channel
    //not allowed PHY_DOUBLE_CHANNEL_CENTERED = 2,        // 40MHz IF bandwidth centered on IF carrier
    PHY_DOUBLE_CHANNEL_HIGH_PRIMARY = 3,     // 40MHz IF bandwidth with higher 20MHz supporting the primary channel
    PHY_MAX_CHANNEL_BOND_STATE       = 0xFFFFFFFF  /* define as 4 bytes data */
}ePhyChanBondState;

typedef enum
{
    PHY_CCA_40MHZ_SOURCE = 0,
    PHY_CCA_20MHZ_SOURCE = 1
}ePhyCCASource;

typedef enum
{
    PHY_CCA_FORCED_ON = 0,
    PHY_CCA_ED = 1,
    PHY_CCA_CD = 2,
    PHY_CCA_CD_AND_CS = 3,
    PHY_CCA_ED_AND_CD = 4,
    PHY_CCA_ED_OR_CD = 5,
    PHY_CCA_ED_AND_CD_AND_CS = 6,
    PHY_CCA_ED_OR_CD_AND_CS = 7,
    PHY_CCA_SEC_ED40_AND_NOR_PKTDET40_PKTDET20 = 8,
    PHY_CCA_SEC_BUSY = 9
}ePhyCCAMode;

typedef enum
{
    PHY_RX_DISABLE_NONE             = 0,
    PHY_RX_DISABLE_11AG             = 0x00000001,          // AGC_DIS_MODE_11AG_MASK
    PHY_RX_DISABLE_11B              = 0x00000002,          // AGC_DIS_MODE_11B_MASK
    PHY_RX_DISABLE_SLR              = 0x00000004,          // AGC_DIS_MODE_SLR_MASK

    PHY_RX_DISABLE_11ABG            = (PHY_RX_DISABLE_11AG | PHY_RX_DISABLE_11B),
    PHY_RX_DISABLE_11AG_SLR         = (PHY_RX_DISABLE_11AG | PHY_RX_DISABLE_SLR),
    PHY_RX_DISABLE_11B_SLR          = (PHY_RX_DISABLE_11B | PHY_RX_DISABLE_SLR),
    PHY_RX_DISABLE_ALL_TYPES        = (PHY_RX_DISABLE_11B | PHY_RX_DISABLE_11AG | PHY_RX_DISABLE_SLR),
}ePhyRxDisabledPktTypes;


// Enum for network density setting.
typedef enum
{
    PHY_NW_DENSITY_LOW = 0,
    PHY_NW_DENSITY_MED,
    PHY_NW_DENSITY_HIGH,
    PHY_NW_DENSITY_ADAPTIVE
} ePhyNwDensity;

typedef enum
{
    PHY_POWER_NORMAL,            //all power enabled for full functionality
    PHY_POWER_STATIC_RX_SM,      //receive 1-chain limitation for partial power savings
    PHY_POWER_DYNAMIC_RX_SM,     //SoftMac may change the receive chain configuration for dynamic SM mode
    PHY_POWER_RF_CHIP_DISABLED,  //Rf PLL disabled, but XO, XO_BUF enabled
    PHY_POWER_OFF                //XO off - Rf chip in lowest power state
}ePhyPowerSave;


typedef tPowerdBm tChannelPwrLimit;

typedef struct
{
    tANI_U8 chanId;
    tChannelPwrLimit pwr;
}tChannelListWithPower;


typedef enum
{
    ALL_CALS,           //RxDco 1st, TxLO 2nd
    RX_DCO_CAL_ONLY,
    TX_LO_CAL_ONLY,
    RX_IQ_CAL_ONLY,
    TX_IQ_CAL_ONLY,
    NO_CALS = 0xFF
}eInitCals;



#endif /* HALPHY_H */

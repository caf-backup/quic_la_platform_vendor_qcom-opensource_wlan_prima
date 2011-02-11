/**
 *
   Airgo Networks, Inc proprietary.
   All Rights Reserved, Copyright 2005
   This program is the confidential and proprietary product of Airgo Networks Inc.
   Any Unauthorized use, reproduction or transfer of this program is strictly prohibited.


   asicAGC.cc: Abstracts the Phy AGC Engine
   Author:  Mark Nelson
   Date:    3/4/05

   History -
   Date        Modified by              Modification Information
  --------------------------------------------------------------------------

 */

#ifndef ASICAGC_H
#define ASICAGC_H

#include <wlan_bit.h>


#define AGC_RX_OVERRIDE_MASK                    MSK_1

#define AGC_RESET_REG                           QWLAN_AGC_AGC_RESET_REG

//use AGC_RX_OVERRIDE_MASK when setting these RX_OVERRIDE fields
#define AGC_RX_OVERRIDE_EN_OFFSET               QWLAN_AGC_RX_OVERRIDE_OVERRIDE_EN_OFFSET
#define AGC_DIS_MODE_11AG_MASK                  QWLAN_AGC_DIS_MODE_DISABLE_11AG_MASK
#define AGC_DIS_MODE_11B_MASK                   QWLAN_AGC_DIS_MODE_DISABLE_11B_MASK
#define AGC_DIS_MODE_SLR_MASK                   QWLAN_AGC_DIS_MODE_DISABLE_SLR_MASK

#define AGC_SINGLE_CHANNEL_CENTERED         0
#define AGC_DOUBLE_CHANNEL_LOW_PRIMARY      (AGC_BANDWIDTH_CONFIG_CB_ENABLE_MASK | (AGC_BANDWIDTH_CONFIG_SUBBAND_ELOWER & AGC_BANDWIDTH_CONFIG_SUBBAND_MASK))
#define AGC_DOUBLE_CHANNEL_HIGH_PRIMARY     (AGC_BANDWIDTH_CONFIG_CB_ENABLE_MASK | (AGC_BANDWIDTH_CONFIG_SUBBAND_EUPPER & AGC_BANDWIDTH_CONFIG_SUBBAND_MASK))


#define AGC_GAIN_LUT_DEPTH 80

#define NUM_GAIN_LUT_SETTINGS AGC_GAIN_LUT_DEPTH
#define MAX_AGC_GAIN  (AGC_GAIN_LUT_DEPTH - 1)


#define AGC_CLOSE_PROXIMITY_SETTING                 0x5050

#define AGC_LOW_DENSITY_2_4_GHZ_20MHZ_SETTING       0x00
#define AGC_LOW_DENSITY_5_GHZ_20MHZ_SETTING         0x00

#define AGC_MED_DENSITY_2_4_GHZ_20MHZ_SETTING       0x25
#define AGC_MED_DENSITY_5_GHZ_20MHZ_SETTING         0x00

#define AGC_HIGH_DENSITY_2_4_GHZ_20MHZ_SETTING      0x40
#define AGC_HIGH_DENSITY_5_GHZ_20MHZ_SETTING        0x25

#define AGC_ADAPTIVE_DENSITY_2_4_GHZ_20MHZ_SETTING  0x00
#define AGC_ADAPTIVE_DENSITY_5_GHZ_20MHZ_SETTING    0x00

#define AGC_LOW_DENSITY_2_4_GHZ_40MHZ_SETTING       0x00
#define AGC_LOW_DENSITY_5_GHZ_40MHZ_SETTING         0x00

#define AGC_MED_DENSITY_2_4_GHZ_40MHZ_SETTING       0x25
#define AGC_MED_DENSITY_5_GHZ_40MHZ_SETTING         0x00

#define AGC_HIGH_DENSITY_2_4_GHZ_40MHZ_SETTING      0x40
#define AGC_HIGH_DENSITY_5_GHZ_40MHZ_SETTING        0x25

#define AGC_ADAPTIVE_DENSITY_2_4_GHZ_40MHZ_SETTING  0x00
#define AGC_ADAPTIVE_DENSITY_5_GHZ_40MHZ_SETTING    0x00
 
typedef enum
{
    AGC_NO_RX = 0,
    AGC_RX_0 = BIT_0,
    
    AGC_ALL_RX = AGC_RX_0,
    AGC_AUTO_RX_ENABLE = 0,
}eAGCRxChainMask;

typedef enum
{
    AGC_RX_ON_AUTO,
    AGC_RX_CALIBRATING,
    AGC_RX_CAL_RF_LOOPBACK
}eAGCRxMode;


typedef struct
{
    tANI_U8 maxGainIndex;
    tANI_U8 topGainDb;
    tANI_U8 bottomGainDb;
    tANI_U8 unused[1];
}tAsicAgc;

#define AGC_INIT_GAIN   0x47
#define AGC_MAX_GAIN    0x49
#endif /* ASICAGC_H */


/** ------------------------------------------------------------------------- *
    ------------------------------------------------------------------------- *


    \file rfQuasar.c

    \brief Quasar functionality

    $Id$

    Copyright (C) 2006 Airgo Networks, Incorporated


   ========================================================================== */

#include "string.h"
#include "sys_api.h"

//#define ONE_SHOT_AUTO_COARSE_TUNING


#define SET_SPI_REG(pMac, rfChip, regAddr, regVal) \
{                                                                                           \
    retVal = asicSpiWriteDataRegister(pMac, rfChip, regAddr, regVal);                       \
    if (retVal != eHAL_STATUS_SUCCESS)                                                      \
    {                                                                                       \
        return (retVal);                                                                    \
    }                                                                                       \
    if (pMac->hphy.phy.test.testLogSpiRegs == eANI_BOOLEAN_TRUE)                            \
    {                                                                                       \
        phyLog(pMac, LOGE, "SET_SPI_REG:%s = %08X\n", &quasarRegStr[regAddr][0], regVal);   \
    }                                                                                       \
}

#define GET_SPI_REG(pMac, rfChip, regAddr, pVal) \
{                                                                                           \
    retVal = asicSpiReadDataRegister(pMac, rfChip, regAddr, pVal);                          \
    if (retVal != eHAL_STATUS_SUCCESS)                                                      \
    {                                                                                       \
        return (retVal);                                                                    \
    }                                                                                       \
    if (pMac->hphy.phy.test.testLogSpiRegs == eANI_BOOLEAN_TRUE)                            \
    {                                                                                       \
        phyLog(pMac, LOGE, "GET_SPI_REG:%s = %08X\n", &quasarRegStr[regAddr][0], *pVal);    \
    }                                                                                       \
}

#define SET_SPI_FIELD(pMac, regAddr, mask, shift, regVal) \
{                                                                                           \
    retVal = rfWriteDataField(pMac, regAddr, mask, shift, regVal);                          \
    if (retVal != eHAL_STATUS_SUCCESS)                                                      \
    {                                                                                       \
        return (retVal);                                                                    \
    }                                                                                       \
}

#define GET_SPI_FIELD(pMac, regAddr, mask, shift, pVal) \
{                                                                       \
    retVal = rfReadDataField(pMac, regAddr, mask, shift, pVal);         \
    if (retVal != eHAL_STATUS_SUCCESS)                                  \
    {                                                                   \
        return (retVal);                                                \
    }                                                                   \
}



const char quasarRegStr[QUASAR_NUM_REGS][40] =
{
    "QUASAR_REG_PLL1",                      //QUASAR_REG_PLL1
    "QUASAR_REG_PLL2",                      //QUASAR_REG_PLL2
    "QUASAR_REG_PLL3",                      //QUASAR_REG_PLL3
    "QUASAR_REG_PLL4",                      //QUASAR_REG_PLL4
    "QUASAR_REG_PLL5",                      //QUASAR_REG_PLL5
    "QUASAR_REG_PLL6",                      //QUASAR_REG_PLL6
    "QUASAR_REG_LO_CONFIG",                 //QUASAR_REG_LO_CONFIG
    "QUASAR_REG_STATE",                     //QUASAR_REG_STATE
    "QUASAR_REG_GAIN_CONTROL_1",            //QUASAR_REG_GAIN_CONTROL_1
    "QUASAR_REG_GAIN_CONTROL_2",            //QUASAR_REG_GAIN_CONTROL_2
    "QUASAR_REG_GAIN_CONTROL_3",            //QUASAR_REG_GAIN_CONTROL_3
    "QUASAR_REG_GAIN_CONTROL_4",            //QUASAR_REG_GAIN_CONTROL_4
    "QUASAR_REG_DC_OFFSET_CHAIN_0",         //QUASAR_REG_DC_OFFSET_CHAIN_0
    "QUASAR_REG_DC_OFFSET_CHAIN_1",         //QUASAR_REG_DC_OFFSET_CHAIN_1
    "QUASAR_REG_DC_OFFSET_CHAIN_2",         //QUASAR_REG_DC_OFFSET_CHAIN_2
    "QUASAR_REG_CFG_1",                     //QUASAR_REG_CFG_1
    "QUASAR_REG_CFG_2",                     //QUASAR_REG_CFG_2
    "QUASAR_REG_CFG_3",                     //QUASAR_REG_CFG_3
    "QUASAR_REG_CFG_4",                     //QUASAR_REG_CFG_4
    "QUASAR_REG_CFG_5",                     //QUASAR_REG_CFG_5
    "QUASAR_REG_CFG_6",                     //QUASAR_REG_CFG_6
    "QUASAR_REG_TEST_1",                    //QUASAR_REG_TEST_1
    "QUASAR_REG_TEST_2",                    //QUASAR_REG_TEST_2
    "QUASAR_REG_TEST_3",                    //QUASAR_REG_TEST_3
    "QUASAR_REG_TEST_4",                    //QUASAR_REG_TEST_4
    "QUASAR_REG_TEST_5",                    //QUASAR_REG_TEST_5
    "QUASAR_REG_TEST_6",                    //QUASAR_REG_TEST_6
    "QUASAR_REG_SPARES",                    //QUASAR_REG_SPARES
    "QUASAR_REG_RESET",                     //QUASAR_REG_RESET
    "QUASAR_REG_SPARES2"                    //QUASAR_REG_SPARES2
};





/*
const tANI_U32 quasarInitVals[QUASAR_NUM_REGS] =
{
    0x2830E,    // QUASAR_REG_PLL1
    0x26666,    // QUASAR_REG_PLL2
    0x06000,    // QUASAR_REG_PLL3
    0x0C412,    // QUASAR_REG_PLL4
    0x00001,    // QUASAR_REG_PLL5
    0x00000,    // QUASAR_REG_PLL6
    0x062AA,    // QUASAR_REG_LO_CONFIG
    0x00FC1,    // QUASAR_REG_STATE
    0x00000,    // QUASAR_REG_GAIN_CONTROL_1
    0x00000,    // QUASAR_REG_GAIN_CONTROL_2
    0x00000,    // QUASAR_REG_GAIN_CONTROL_3
    0x00000,    // QUASAR_REG_GAIN_CONTROL_4
    0x02040,    // QUASAR_REG_DC_OFFSET_CHAIN_0
    0x02040,    // QUASAR_REG_DC_OFFSET_CHAIN_1
    0x02040,    // QUASAR_REG_DC_OFFSET_CHAIN_2
    0x38040,    // QUASAR_REG_CFG_1
    0x00000,    // QUASAR_REG_CFG_2
    0x38000,    // QUASAR_REG_CFG_3
    0x30000,    // QUASAR_REG_CFG_4
    0x38120,    // QUASAR_REG_CFG_5
    0x0040E,    // QUASAR_REG_CFG_6
    0x00000,    // QUASAR_REG_TEST_1
    0x1FC00,    // QUASAR_REG_TEST_2
    0x00000,    // QUASAR_REG_TEST_3
    0x1FFFF,    // QUASAR_REG_TEST_4
    0x18000,    // QUASAR_REG_TEST_5
    0x00000,    // QUASAR_REG_TEST_6
    0x00000,    // QUASAR_REG_SPARES
    0x00000,    // QUASAR_REG_RESET
    0x00000     // QUASAR_REG_SPARES2
};
*/


/*
    const tANI_U32 quasarInitVals[QUASAR_NUM_REGS] =
    {
        //from Robert's default values
        0x02830E & MSK_18,    // QUASAR_REG_PLL1
        0x0A6666 & MSK_18,    // QUASAR_REG_PLL2
        0x106000 & MSK_18,    // QUASAR_REG_PLL3
        0x18C412 & MSK_18,    // QUASAR_REG_PLL4
        0x204829 & MSK_18,    // QUASAR_REG_PLL5
        0x2802B8 & MSK_18,    // QUASAR_REG_PLL6
        0x3062AA & MSK_18,    // QUASAR_REG_LO_CONFIG
        0x380FC1 & MSK_18,    // QUASAR_REG_STATE
        0x400000 & MSK_18,    // QUASAR_REG_GAIN_CONTROL_1
        0x480000 & MSK_18,    // QUASAR_REG_GAIN_CONTROL_2
        0x500000 & MSK_18,    // QUASAR_REG_GAIN_CONTROL_3
        0x580000 & MSK_18,    // QUASAR_REG_GAIN_CONTROL_4
        0x602040 & MSK_18,    // QUASAR_REG_DC_OFFSET_CHAIN_0
        0x682040 & MSK_18,    // QUASAR_REG_DC_OFFSET_CHAIN_1
        0x702040 & MSK_18,    // QUASAR_REG_DC_OFFSET_CHAIN_2
        0x7B8040 & MSK_18,    // QUASAR_REG_CFG_1
        0x800000 & MSK_18,    // QUASAR_REG_CFG_2
        0x8B8000 & MSK_18,    // QUASAR_REG_CFG_3
        0x930000 & MSK_18,    // QUASAR_REG_CFG_4
        0x9B8120 & MSK_18,    // QUASAR_REG_CFG_5
        0xA0040E & MSK_18,    // QUASAR_REG_CFG_6
        0xA80000 & MSK_18,    // QUASAR_REG_TEST_1
        0xB1FC00 & MSK_18,    // QUASAR_REG_TEST_2
        0xB80000 & MSK_18,    // QUASAR_REG_TEST_3
        0xC1FFFF & MSK_18,    // QUASAR_REG_TEST_4
        0xC88000 & MSK_18,    // QUASAR_REG_TEST_5
        0xD00000 & MSK_18,    // QUASAR_REG_TEST_6
        0xD80000 & MSK_18,    // QUASAR_REG_SPARES
        0xE00000 & MSK_18,    // QUASAR_REG_RESET
        0xE80000 & MSK_18     // QUASAR_REG_SPARES2
    };
*/



const tANI_U32 quasarInitVals[QUASAR_NUM_REGS] =
{
    //from QuasarDefault.csv for 2.4GHz
    0x02830E & MSK_18,    // QUASAR_REG_PLL1
    0x0A6666 & MSK_18,    // QUASAR_REG_PLL2
    0x10E000 & MSK_18,    // QUASAR_REG_PLL3    -modified VCO_EN = 1 - see erata #35
    0x18C412 & MSK_18,    // QUASAR_REG_PLL4
    0x20C829 & MSK_18,    // QUASAR_REG_PLL5
    0x2802B8 & MSK_18,    // QUASAR_REG_PLL6
    0x305100 & MSK_18,    // QUASAR_REG_LO_CONFIG
    0x3800A1 & MSK_18,    // QUASAR_REG_STATE
    0x400000 & MSK_18,    // QUASAR_REG_GAIN_CONTROL_1
    0x4800FF & MSK_18,    // QUASAR_REG_GAIN_CONTROL_2
    0x500000 & MSK_18,    // QUASAR_REG_GAIN_CONTROL_3
    0x580000 & MSK_18,    // QUASAR_REG_GAIN_CONTROL_4
    0x602040 & MSK_18,    // QUASAR_REG_DC_OFFSET_CHAIN_0
    0x682040 & MSK_18,    // QUASAR_REG_DC_OFFSET_CHAIN_1
    0x702040 & MSK_18,    // QUASAR_REG_DC_OFFSET_CHAIN_2
    0x7B8240 & MSK_18,    // QUASAR_REG_CFG_1
    0x800010 & MSK_18,    // QUASAR_REG_CFG_2
    0x8B8000 & MSK_18,    // QUASAR_REG_CFG_3
    0x930000 & MSK_18,    // QUASAR_REG_CFG_4
    0x9B8000 & MSK_18,    // QUASAR_REG_CFG_5
    0xA0040E & MSK_18,    // QUASAR_REG_CFG_6
    0xA80000 & MSK_18,    // QUASAR_REG_TEST_1
    0xB1FE00 & MSK_18,    // QUASAR_REG_TEST_2
    0xB80000 & MSK_18,    // QUASAR_REG_TEST_3
    0xC1FFFF & MSK_18,    // QUASAR_REG_TEST_4
    0xC88010 & MSK_18,    // QUASAR_REG_TEST_5
    0xD00000 & MSK_18,    // QUASAR_REG_TEST_6
    0xD80000 & MSK_18,    // QUASAR_REG_SPARES
    0xE00000 & MSK_18,    // QUASAR_REG_RESET
    0xE80000 & MSK_18     // QUASAR_REG_SPARES2
};

const tRfChannelProps rfChannels[NUM_RF_CHANNELS] =
{
    //RF_SUBBAND_2_4_GHZ
    //freq, chan#, band
    { 2412, 1  , RF_SUBBAND_2_4_GHZ},        //RF_CHAN_1,
    { 2417, 2  , RF_SUBBAND_2_4_GHZ},        //RF_CHAN_2,
    { 2422, 3  , RF_SUBBAND_2_4_GHZ},        //RF_CHAN_3,
    { 2427, 4  , RF_SUBBAND_2_4_GHZ},        //RF_CHAN_4,
    { 2432, 5  , RF_SUBBAND_2_4_GHZ},        //RF_CHAN_5,
    { 2437, 6  , RF_SUBBAND_2_4_GHZ},        //RF_CHAN_6,
    { 2442, 7  , RF_SUBBAND_2_4_GHZ},        //RF_CHAN_7,
    { 2447, 8  , RF_SUBBAND_2_4_GHZ},        //RF_CHAN_8,
    { 2452, 9  , RF_SUBBAND_2_4_GHZ},        //RF_CHAN_9,
    { 2457, 10 , RF_SUBBAND_2_4_GHZ},        //RF_CHAN_10,
    { 2462, 11 , RF_SUBBAND_2_4_GHZ},        //RF_CHAN_11,
    { 2467, 12 , RF_SUBBAND_2_4_GHZ},        //RF_CHAN_12,
    { 2472, 13 , RF_SUBBAND_2_4_GHZ},        //RF_CHAN_13,
    { 2484, 14 , RF_SUBBAND_2_4_GHZ},        //RF_CHAN_14,

    //4.9GHz Band
    { 4920, 240 , RF_SUBBAND_4_9_GHZ},       //RF_CHAN_240,
    { 4940, 244 , RF_SUBBAND_4_9_GHZ},       //RF_CHAN_244,
    { 4960, 248 , RF_SUBBAND_4_9_GHZ},       //RF_CHAN_248,
    { 4980, 252 , RF_SUBBAND_4_9_GHZ},       //RF_CHAN_252,

    { 5040, 208 , RF_SUBBAND_4_9_GHZ},       //RF_CHAN_208,
    { 5060, 212 , RF_SUBBAND_4_9_GHZ},       //RF_CHAN_212,
    { 5080, 216 , RF_SUBBAND_4_9_GHZ},       //RF_CHAN_216,

    //5GHz Low Band
    { 5180, 36 , RF_SUBBAND_5_LOW_GHZ},      //RF_CHAN_36,
    { 5200, 40 , RF_SUBBAND_5_LOW_GHZ},      //RF_CHAN_40,
    { 5220, 44 , RF_SUBBAND_5_LOW_GHZ},      //RF_CHAN_44,
    { 5240, 48 , RF_SUBBAND_5_LOW_GHZ},      //RF_CHAN_48,
    { 5260, 52 , RF_SUBBAND_5_LOW_GHZ},      //RF_CHAN_52,
    { 5280, 56 , RF_SUBBAND_5_LOW_GHZ},      //RF_CHAN_56,
    { 5300, 60 , RF_SUBBAND_5_LOW_GHZ},      //RF_CHAN_60,
    { 5320, 64 , RF_SUBBAND_5_LOW_GHZ},      //RF_CHAN_64,

    //5GHz Mid Band
    { 5500, 100 , RF_SUBBAND_5_MID_GHZ},     //RF_CHAN_100,
    { 5520, 104 , RF_SUBBAND_5_MID_GHZ},     //RF_CHAN_104,
    { 5540, 108 , RF_SUBBAND_5_MID_GHZ},     //RF_CHAN_108,
    { 5560, 112 , RF_SUBBAND_5_MID_GHZ},     //RF_CHAN_112,
    { 5580, 116 , RF_SUBBAND_5_MID_GHZ},     //RF_CHAN_116,
    { 5600, 120 , RF_SUBBAND_5_MID_GHZ},     //RF_CHAN_120,
    { 5620, 124 , RF_SUBBAND_5_MID_GHZ},     //RF_CHAN_124,
    { 5640, 128 , RF_SUBBAND_5_MID_GHZ},     //RF_CHAN_128,
    { 5660, 132 , RF_SUBBAND_5_MID_GHZ},     //RF_CHAN_132,
    { 5680, 136 , RF_SUBBAND_5_MID_GHZ},     //RF_CHAN_136,
    { 5700, 140 , RF_SUBBAND_5_MID_GHZ},     //RF_CHAN_140,

    //5GHz High Band
    { 5745, 149 , RF_SUBBAND_5_HIGH_GHZ},    //RF_CHAN_149,
    { 5765, 153 , RF_SUBBAND_5_HIGH_GHZ},    //RF_CHAN_153,
    { 5785, 157 , RF_SUBBAND_5_HIGH_GHZ},    //RF_CHAN_157,
    { 5805, 161 , RF_SUBBAND_5_HIGH_GHZ},    //RF_CHAN_161,
    { 5825, 165 , RF_SUBBAND_5_HIGH_GHZ},    //RF_CHAN_165,

    //CHANNEL BONDED SETTINGS
    { 2422, 3  ,RF_SUBBAND_2_4_GHZ},         //RF_CHAN_BOND_3
    { 2427, 4  ,RF_SUBBAND_2_4_GHZ},         //RF_CHAN_BOND_4
    { 2432, 5  ,RF_SUBBAND_2_4_GHZ},         //RF_CHAN_BOND_5
    { 2437, 6  ,RF_SUBBAND_2_4_GHZ},         //RF_CHAN_BOND_6
    { 2442, 7  ,RF_SUBBAND_2_4_GHZ},         //RF_CHAN_BOND_7
    { 2447, 8  ,RF_SUBBAND_2_4_GHZ},         //RF_CHAN_BOND_8
    { 2452, 9  ,RF_SUBBAND_2_4_GHZ},         //RF_CHAN_BOND_9
    { 2457, 10 ,RF_SUBBAND_2_4_GHZ},         //RF_CHAN_BOND_10
    { 2462, 11 ,RF_SUBBAND_2_4_GHZ},         //RF_CHAN_BOND_11

    //4.9GHz Band
    { 4930, 242 ,RF_SUBBAND_4_9_GHZ},        //RF_CHAN_BOND_242
    { 4950, 246 ,RF_SUBBAND_4_9_GHZ},        //RF_CHAN_BOND_246
    { 4970, 250 ,RF_SUBBAND_4_9_GHZ},        //RF_CHAN_BOND_250

    { 5050, 210 ,RF_SUBBAND_4_9_GHZ},        //RF_CHAN_BOND_210
    { 5070, 214 ,RF_SUBBAND_4_9_GHZ},        //RF_CHAN_BOND_214

    //5GHz Low & Mid U-NII Band
    { 5190, 38 , RF_SUBBAND_5_LOW_GHZ},      //RF_CHAN_BOND_38,
    { 5210, 42 , RF_SUBBAND_5_LOW_GHZ},      //RF_CHAN_BOND_42,
    { 5230, 46 , RF_SUBBAND_5_LOW_GHZ},      //RF_CHAN_BOND_46,
    { 5250, 50 , RF_SUBBAND_5_LOW_GHZ},      //RF_CHAN_BOND_50,
    { 5270, 54 , RF_SUBBAND_5_LOW_GHZ},      //RF_CHAN_BOND_54
    { 5290, 58 , RF_SUBBAND_5_LOW_GHZ},      //RF_CHAN_BOND_58
    { 5310, 62 , RF_SUBBAND_5_LOW_GHZ},      //RF_CHAN_BOND_62

    //5GHz Mid Band
    { 5510, 102 , RF_SUBBAND_5_MID_GHZ},     //RF_CHAN_BOND_102
    { 5530, 106 , RF_SUBBAND_5_MID_GHZ},     //RF_CHAN_BOND_106
    { 5550, 110 , RF_SUBBAND_5_MID_GHZ},     //RF_CHAN_BOND_110
    { 5570, 114 , RF_SUBBAND_5_MID_GHZ},     //RF_CHAN_BOND_114
    { 5590, 118 , RF_SUBBAND_5_MID_GHZ},     //RF_CHAN_BOND_118
    { 5610, 122 , RF_SUBBAND_5_MID_GHZ},     //RF_CHAN_BOND_122
    { 5630, 126 , RF_SUBBAND_5_MID_GHZ},     //RF_CHAN_BOND_126
    { 5650, 130 , RF_SUBBAND_5_MID_GHZ},     //RF_CHAN_BOND_130
    { 5670, 134 , RF_SUBBAND_5_MID_GHZ},     //RF_CHAN_BOND_134
    { 5690, 138 , RF_SUBBAND_5_MID_GHZ},     //RF_CHAN_BOND_138

    //5GHz High Band
    { 5755, 151 , RF_SUBBAND_5_HIGH_GHZ},    //RF_CHAN_BOND_151
    { 5775, 155 , RF_SUBBAND_5_HIGH_GHZ},    //RF_CHAN_BOND_155
    { 5795, 159 , RF_SUBBAND_5_HIGH_GHZ},    //RF_CHAN_BOND_159
    { 5815, 163 , RF_SUBBAND_5_HIGH_GHZ}     //RF_CHAN_BOND_163
};



typedef struct
{
    tANI_U8 ndivInt;
    tANI_U32 ndivFrac;
}sQuasarDividers;

typedef enum
{
    RDIV_REF_CLK_80MHZ,
    RDIV_REF_CLK_53_333MHZ,
    RDIV_REF_CLK_40MHZ,
    RDIV_REF_CLK_26_667MHZ,
    NUM_RDIV_REF_CLK
}eRdivRefClock;

#define NDIV_DECIMAL_MULT   (1000000)

#define VCO_BAND_THRESHOLD(RDIV)                                  \
(RDIV == RDIV_REF_CLK_80MHZ ? 43524288UL :                        \
 (RDIV == RDIV_REF_CLK_53_333MHZ ? 65262144UL :                   \
  (RDIV == RDIV_REF_CLK_40MHZ ? 87000000UL :                      \
   (RDIV == RDIV_REF_CLK_26_667MHZ ? 130524288UL : 123000000UL    \
   )                                                              \
  )                                                               \
 )                                                                \
)


const sQuasarDividers chanNDiv[NUM_RF_CHANNELS][NUM_RDIV_REF_CLK] =
{
    { { 40, 209715 },   { 60, 314573 },   { 80, 419430 },   { 120, 629146 } },    //2412 RF_CHAN_1
    { { 40, 297097 },   { 60, 445645 },   { 80, 594193 },   { 120, 891290 } },    //2417 RF_CHAN_2
    { { 40, 384478 },   { 60, 576717 },   { 80, 768956 },   { 121, 104858 } },    //2422 RF_CHAN_3
    { { 40, 471859 },   { 60, 707789 },   { 80, 943718 },   { 121, 367002 } },    //2427 RF_CHAN_4
    { { 40, 559241 },   { 60, 838861 },   { 81,  69905 },   { 121, 629146 } },    //2432 RF_CHAN_5
    { { 40, 646622 },   { 60, 969933 },   { 81, 244668 },   { 121, 891290 } },    //2437 RF_CHAN_6
    { { 40, 734003 },   { 61,  52429 },   { 81, 419430 },   { 122, 104858 } },    //2442 RF_CHAN_7
    { { 40, 821385 },   { 61, 183501 },   { 81, 594193 },   { 122, 367002 } },    //2447 RF_CHAN_8
    { { 40, 908766 },   { 61, 314573 },   { 81, 768956 },   { 122, 629146 } },    //2452 RF_CHAN_9
    { { 40, 996147 },   { 61, 445645 },   { 81, 943718 },   { 122, 891290 } },    //2457 RF_CHAN_10
    { { 41,  34953 },   { 61, 576717 },   { 82,  69905 },   { 123, 104858 } },    //2462 RF_CHAN_11
    { { 41, 122334 },   { 61, 707789 },   { 82, 244668 },   { 123, 367002 } },    //2467 RF_CHAN_12
    { { 41, 209715 },   { 61, 838861 },   { 82, 419430 },   { 123, 629146 } },    //2472 RF_CHAN_13
    { { 41, 419430 },   { 62, 104858 },   { 82, 838861 },   { 124, 209715 } },    //2484 RF_CHAN_14
    { { 41, 000000 },   { 61, 524288 },   { 82, 000000 },   { 123, 000000 } },    //4920 RF_CHAN_240
    { { 41, 174763 },   { 61, 786432 },   { 82, 349525 },   { 123, 524288 } },    //4940 RF_CHAN_244
    { { 41, 349525 },   { 62, 000000 },   { 82, 699051 },   { 124, 000000 } },    //4960 RF_CHAN_248
    { { 41, 524288 },   { 62, 262144 },   { 83, 000000 },   { 124, 524288 } },    //4980 RF_CHAN_252
    { { 42, 000000 },   { 63, 000000 },   { 84, 000000 },   { 126, 000000 } },    //5040 RF_CHAN_208
    { { 42, 174763 },   { 63, 262144 },   { 84, 349525 },   { 126, 524288 } },    //5060 RF_CHAN_212
    { { 42, 349525 },   { 63, 524288 },   { 84, 699051 },   { 127, 000000 } },    //5080 RF_CHAN_216
    { { 43, 174763 },   { 64, 786432 },   { 86, 349525 },   { 129, 524288 } },    //5180 RF_CHAN_36
    { { 43, 349525 },   { 65, 000000 },   { 86, 699051 },   { 130, 000000 } },    //5200 RF_CHAN_40
    { { 43, 524288 },   { 65, 262144 },   { 87, 000000 },   { 130, 524288 } },    //5220 RF_CHAN_44
    { { 43, 699051 },   { 65, 524288 },   { 87, 349525 },   { 131, 000000 } },    //5240 RF_CHAN_48
    { { 43, 873813 },   { 65, 786432 },   { 87, 699051 },   { 131, 524288 } },    //5260 RF_CHAN_52
    { { 44, 000000 },   { 66, 000000 },   { 88, 000000 },   { 132, 000000 } },    //5280 RF_CHAN_56
    { { 44, 174763 },   { 66, 262144 },   { 88, 349525 },   { 132, 524288 } },    //5300 RF_CHAN_60
    { { 44, 349525 },   { 66, 524288 },   { 88, 699051 },   { 133, 000000 } },    //5320 RF_CHAN_64
    { { 45, 873813 },   { 68, 786432 },   { 91, 699051 },   { 137, 524288 } },    //5500 RF_CHAN_100
    { { 46, 000000 },   { 69, 000000 },   { 92, 000000 },   { 138, 000000 } },    //5520 RF_CHAN_104
    { { 46, 174763 },   { 69, 262144 },   { 92, 349525 },   { 138, 524288 } },    //5540 RF_CHAN_108
    { { 46, 349525 },   { 69, 524288 },   { 92, 699051 },   { 139, 000000 } },    //5560 RF_CHAN_112
    { { 46, 524288 },   { 69, 786432 },   { 93, 000000 },   { 139, 524288 } },    //5580 RF_CHAN_116
    { { 46, 699051 },   { 70, 000000 },   { 93, 349525 },   { 140, 000000 } },    //5600 RF_CHAN_120
    { { 46, 873813 },   { 70, 262144 },   { 93, 699051 },   { 140, 524288 } },    //5620 RF_CHAN_124
    { { 47, 000000 },   { 70, 524288 },   { 94, 000000 },   { 141, 000000 } },    //5640 RF_CHAN_128
    { { 47, 174763 },   { 70, 786432 },   { 94, 349525 },   { 141, 524288 } },    //5660 RF_CHAN_132
    { { 47, 349525 },   { 71, 000000 },   { 94, 699051 },   { 142, 000000 } },    //5680 RF_CHAN_136
    { { 47, 524288 },   { 71, 262144 },   { 95, 000000 },   { 142, 524288 } },    //5700 RF_CHAN_140
    { { 47, 917504 },   { 71, 851968 },   { 95, 786432 },   { 143, 655360 } },    //5745 RF_CHAN_149
    { { 48,  43691 },   { 72,  65536 },   { 96,  87381 },   { 144, 131072 } },    //5765 RF_CHAN_153
    { { 48, 218453 },   { 72, 327680 },   { 96, 436907 },   { 144, 655360 } },    //5785 RF_CHAN_157
    { { 48, 393216 },   { 72, 589824 },   { 96, 786432 },   { 145, 131072 } },    //5805 RF_CHAN_161
    { { 48, 567979 },   { 72, 851968 },   { 97,  87381 },   { 145, 655360 } },    //5825 RF_CHAN_165
    { { 40, 384478 },   { 60, 576717 },   { 80, 768956 },   { 121, 104858 } },    //2422 RF_CHAN_BOND_3
    { { 40, 471859 },   { 60, 707789 },   { 80, 943718 },   { 121, 367002 } },    //2427 RF_CHAN_BOND_4
    { { 40, 559241 },   { 60, 838861 },   { 81,  69905 },   { 121, 629146 } },    //2432 RF_CHAN_BOND_5
    { { 40, 646622 },   { 60, 969933 },   { 81, 244668 },   { 121, 891290 } },    //2437 RF_CHAN_BOND_6
    { { 40, 734003 },   { 61,  52429 },   { 81, 419430 },   { 122, 104858 } },    //2442 RF_CHAN_BOND_7
    { { 40, 821385 },   { 61, 183501 },   { 81, 594193 },   { 122, 367002 } },    //2447 RF_CHAN_BOND_8
    { { 40, 908766 },   { 61, 314573 },   { 81, 768956 },   { 122, 629146 } },    //2452 RF_CHAN_BOND_9
    { { 40, 996147 },   { 61, 445645 },   { 81, 943718 },   { 122, 891290 } },    //2457 RF_CHAN_BOND_10
    { { 41,  34953 },   { 61, 576717 },   { 82,  69905 },   { 123, 104858 } },    //2462 RF_CHAN_BOND_11
    { { 41,  87381 },   { 61, 655360 },   { 82, 174763 },   { 123, 262144 } },    //4930 RF_CHAN_BOND_242
    { { 41, 262144 },   { 61, 917504 },   { 82, 524288 },   { 123, 786432 } },    //4950 RF_CHAN_BOND_246
    { { 41, 436907 },   { 62, 131072 },   { 82, 873813 },   { 124, 262144 } },    //4970 RF_CHAN_BOND_250
    { { 42,  87381 },   { 63, 131072 },   { 84, 174763 },   { 126, 262144 } },    //5050 RF_CHAN_BOND_210
    { { 42, 262144 },   { 63, 393216 },   { 84, 524288 },   { 126, 786432 } },    //5070 RF_CHAN_BOND_214
    { { 43, 262144 },   { 64, 917504 },   { 86, 524288 },   { 129, 786432 } },    //5190 RF_CHAN_BOND_38
    { { 43, 436907 },   { 65, 131072 },   { 86, 873813 },   { 130, 262144 } },    //5210 RF_CHAN_BOND_42
    { { 43, 611669 },   { 65, 393216 },   { 87, 174763 },   { 130, 786432 } },    //5230 RF_CHAN_BOND_46
    { { 43, 786432 },   { 65, 655360 },   { 87, 524288 },   { 131, 262144 } },    //5250 RF_CHAN_BOND_50
    { { 43, 961195 },   { 65, 917504 },   { 87, 873813 },   { 131, 786432 } },    //5270 RF_CHAN_BOND_54
    { { 44,  87381 },   { 66, 131072 },   { 88, 174763 },   { 132, 262144 } },    //5290 RF_CHAN_BOND_58
    { { 44, 262144 },   { 66, 393216 },   { 88, 524288 },   { 132, 786432 } },    //5310 RF_CHAN_BOND_62
    { { 45, 961195 },   { 68, 917504 },   { 91, 873813 },   { 137, 786432 } },    //5510 RF_CHAN_BOND_102
    { { 46,  87381 },   { 69, 131072 },   { 92, 174763 },   { 138, 262144 } },    //5530 RF_CHAN_BOND_106
    { { 46, 262144 },   { 69, 393216 },   { 92, 524288 },   { 138, 786432 } },    //5550 RF_CHAN_BOND_110
    { { 46, 436907 },   { 69, 655360 },   { 92, 873813 },   { 139, 262144 } },    //5570 RF_CHAN_BOND_114
    { { 46, 611669 },   { 69, 917504 },   { 93, 174763 },   { 139, 786432 } },    //5590 RF_CHAN_BOND_118
    { { 46, 786432 },   { 70, 131072 },   { 93, 524288 },   { 140, 262144 } },    //5610 RF_CHAN_BOND_122
    { { 46, 961195 },   { 70, 393216 },   { 93, 873813 },   { 140, 786432 } },    //5630 RF_CHAN_BOND_126
    { { 47,  87381 },   { 70, 655360 },   { 94, 174763 },   { 141, 262144 } },    //5650 RF_CHAN_BOND_130
    { { 47, 262144 },   { 70, 917504 },   { 94, 524288 },   { 141, 786432 } },    //5670 RF_CHAN_BOND_134
    { { 47, 436907 },   { 71, 131072 },   { 94, 873813 },   { 142, 262144 } },    //5690 RF_CHAN_BOND_138
    { { 47, 1004885},   { 71, 983040 },   { 95, 961195 },   { 143, 917504 } },    //5755 RF_CHAN_BOND_151
    { { 48, 131072 },   { 72, 196608 },   { 96, 262144 },   { 144, 393216 } },    //5775 RF_CHAN_BOND_155
    { { 48, 305835 },   { 72, 458752 },   { 96, 611669 },   { 144, 917504 } },    //5795 RF_CHAN_BOND_159
    { { 48, 480597 },   { 72, 720896 },   { 96, 961195 },   { 145, 393216 } }     //5815 RF_CHAN_BOND_163
};



//this table defines which rdiv/ndiv numbers to use.
const tANI_U8 rdivPerChannel[NUM_RF_CHANNELS] =
{
    RDIV_REF_CLK_80MHZ,  //2412 RF_CHAN_1
    RDIV_REF_CLK_80MHZ,  //2417 RF_CHAN_2
    RDIV_REF_CLK_80MHZ,  //2422 RF_CHAN_3
    RDIV_REF_CLK_80MHZ,  //2427 RF_CHAN_4
    RDIV_REF_CLK_80MHZ,  //2432 RF_CHAN_5
    RDIV_REF_CLK_80MHZ,  //2437 RF_CHAN_6
    RDIV_REF_CLK_80MHZ,  //2442 RF_CHAN_7
    RDIV_REF_CLK_80MHZ,  //2447 RF_CHAN_8
    RDIV_REF_CLK_80MHZ,  //2452 RF_CHAN_9
    RDIV_REF_CLK_80MHZ,  //2457 RF_CHAN_10
    RDIV_REF_CLK_80MHZ,  //2462 RF_CHAN_11
    RDIV_REF_CLK_80MHZ,  //2467 RF_CHAN_12
    RDIV_REF_CLK_80MHZ,  //2472 RF_CHAN_13
    RDIV_REF_CLK_80MHZ,  //2484 RF_CHAN_14

    RDIV_REF_CLK_80MHZ,  //4920 RF_CHAN_240
    RDIV_REF_CLK_80MHZ,  //4940 RF_CHAN_244
    RDIV_REF_CLK_80MHZ,  //4960 RF_CHAN_248
    RDIV_REF_CLK_80MHZ,  //4980 RF_CHAN_252
    RDIV_REF_CLK_80MHZ,  //5040 RF_CHAN_208
    RDIV_REF_CLK_80MHZ,  //5060 RF_CHAN_212
    RDIV_REF_CLK_80MHZ,  //5080 RF_CHAN_216

    RDIV_REF_CLK_80MHZ,  //5180 RF_CHAN_36
    RDIV_REF_CLK_80MHZ,  //5200 RF_CHAN_40
    RDIV_REF_CLK_80MHZ,  //5220 RF_CHAN_44
    RDIV_REF_CLK_80MHZ,  //5240 RF_CHAN_48
    RDIV_REF_CLK_80MHZ,  //5260 RF_CHAN_52
    RDIV_REF_CLK_80MHZ,  //5280 RF_CHAN_56
    RDIV_REF_CLK_80MHZ,  //5300 RF_CHAN_60
    RDIV_REF_CLK_80MHZ,  //5320 RF_CHAN_64

    RDIV_REF_CLK_80MHZ,  //5500 RF_CHAN_100
    RDIV_REF_CLK_80MHZ,  //5520 RF_CHAN_104
    RDIV_REF_CLK_80MHZ,  //5540 RF_CHAN_108
    RDIV_REF_CLK_80MHZ,  //5560 RF_CHAN_112
    RDIV_REF_CLK_80MHZ,  //5580 RF_CHAN_116
    RDIV_REF_CLK_80MHZ,  //5600 RF_CHAN_120
    RDIV_REF_CLK_80MHZ,  //5620 RF_CHAN_124
    RDIV_REF_CLK_80MHZ,  //5640 RF_CHAN_128
    RDIV_REF_CLK_80MHZ,  //5660 RF_CHAN_132
    RDIV_REF_CLK_80MHZ,  //5680 RF_CHAN_136
    RDIV_REF_CLK_80MHZ,  //5700 RF_CHAN_140

    RDIV_REF_CLK_80MHZ,  //5745 RF_CHAN_149
    RDIV_REF_CLK_80MHZ,  //5765 RF_CHAN_153
    RDIV_REF_CLK_80MHZ,  //5785 RF_CHAN_157
    RDIV_REF_CLK_80MHZ,  //5805 RF_CHAN_161
    RDIV_REF_CLK_80MHZ,  //5825 RF_CHAN_165

    RDIV_REF_CLK_80MHZ,  //2422 RF_CHAN_BOND_3
    RDIV_REF_CLK_80MHZ,  //2427 RF_CHAN_BOND_4
    RDIV_REF_CLK_80MHZ,  //2432 RF_CHAN_BOND_5
    RDIV_REF_CLK_80MHZ,  //2437 RF_CHAN_BOND_6
    RDIV_REF_CLK_80MHZ,  //2442 RF_CHAN_BOND_7
    RDIV_REF_CLK_80MHZ,  //2447 RF_CHAN_BOND_8
    RDIV_REF_CLK_80MHZ,  //2452 RF_CHAN_BOND_9
    RDIV_REF_CLK_80MHZ,  //2457 RF_CHAN_BOND_10
    RDIV_REF_CLK_80MHZ,  //2462 RF_CHAN_BOND_11

    RDIV_REF_CLK_80MHZ,  //4930 RF_CHAN_BOND_242
    RDIV_REF_CLK_80MHZ,  //4950 RF_CHAN_BOND_246
    RDIV_REF_CLK_80MHZ,  //4970 RF_CHAN_BOND_250
    RDIV_REF_CLK_80MHZ,  //5050 RF_CHAN_BOND_210
    RDIV_REF_CLK_80MHZ,  //5070 RF_CHAN_BOND_214

    RDIV_REF_CLK_80MHZ,  //5190 RF_CHAN_BOND_38
    RDIV_REF_CLK_80MHZ,  //5210 RF_CHAN_BOND_42
    RDIV_REF_CLK_80MHZ,  //5230 RF_CHAN_BOND_46
    RDIV_REF_CLK_80MHZ,  //5250 RF_CHAN_BOND_50
    RDIV_REF_CLK_80MHZ,  //5270 RF_CHAN_BOND_54
    RDIV_REF_CLK_80MHZ,  //5290 RF_CHAN_BOND_58
    RDIV_REF_CLK_80MHZ,  //5310 RF_CHAN_BOND_62

    RDIV_REF_CLK_80MHZ,  //5510 RF_CHAN_BOND_102
    RDIV_REF_CLK_80MHZ,  //5530 RF_CHAN_BOND_106
    RDIV_REF_CLK_80MHZ,  //5550 RF_CHAN_BOND_110
    RDIV_REF_CLK_80MHZ,  //5570 RF_CHAN_BOND_114
    RDIV_REF_CLK_80MHZ,  //5590 RF_CHAN_BOND_118
    RDIV_REF_CLK_80MHZ,  //5610 RF_CHAN_BOND_122
    RDIV_REF_CLK_80MHZ,  //5630 RF_CHAN_BOND_126
    RDIV_REF_CLK_80MHZ,  //5650 RF_CHAN_BOND_130
    RDIV_REF_CLK_80MHZ,  //5670 RF_CHAN_BOND_134
    RDIV_REF_CLK_80MHZ,  //5690 RF_CHAN_BOND_138

    RDIV_REF_CLK_80MHZ,  //5755 RF_CHAN_BOND_151
    RDIV_REF_CLK_80MHZ,  //5775 RF_CHAN_BOND_155
    RDIV_REF_CLK_80MHZ,  //5795 RF_CHAN_BOND_159
    RDIV_REF_CLK_80MHZ   //5815 RF_CHAN_BOND_163
};


#define NUM_BAND_FIELDS (21)
typedef eQuasarFields tQuasarBandFields[NUM_BAND_FIELDS];
const tQuasarBandFields quasarBandFieldId =
{
    QUASAR_FIELD_IBUF_BIAS,     //QUASAR_REG_PLL5
    QUASAR_FIELD_VCO_BIAS,      //QUASAR_REG_PLL5
    QUASAR_FIELD_NDIV_BIAS,     //QUASAR_REG_PLL5
    QUASAR_FIELD_TXRX_BAND,     //QUASAR_REG_STATE
    QUASAR_FIELD_TWOG_HS_EN,    //QUASAR_REG_STATE
    QUASAR_FIELD_TX_RFLO,       //QUASAR_REG_CFG_1
    QUASAR_FIELD_TX_IFLO,       //QUASAR_REG_CFG_1
    QUASAR_FIELD_TX_RF_TUN,     //QUASAR_REG_CFG_5
    QUASAR_FIELD_TX_IF_TUN,     //QUASAR_REG_CFG_5
    QUASAR_FIELD_RX_CASC_GC,    //QUASAR_REG_CFG_6
    QUASAR_FIELD_TX_CASC_GC,    //QUASAR_REG_CFG_6
    QUASAR_FIELD_EXT_IFLO_EN,   //QUASAR_REG_LO_CONFIG
    QUASAR_FIELD_IQ_DIV_EN,     //QUASAR_REG_LO_CONFIG
    QUASAR_FIELD_NDIV_LO,       //QUASAR_REG_LO_CONFIG
    QUASAR_FIELD_EXT_LO,        //QUASAR_REG_LO_CONFIG
    QUASAR_FIELD_TX1_LO,        //QUASAR_REG_LO_CONFIG
    QUASAR_FIELD_TX0_LO,        //QUASAR_REG_LO_CONFIG
    QUASAR_FIELD_RX2_LO,        //QUASAR_REG_LO_CONFIG
    QUASAR_FIELD_RX1_LO,        //QUASAR_REG_LO_CONFIG
    QUASAR_FIELD_RX0_LO,        //QUASAR_REG_LO_CONFIG
    QUASAR_FIELD_MOD_INV        //QUASAR_CFG1_REG
};

const tQuasarBandFields quasarBandSettings[NUM_RF_SUBBANDS] =
{
    {
        4,  // QUASAR_FIELD_IBUF_BIAS
        4,  // QUASAR_FIELD_VCO_BIAS
        1,  // QUASAR_FIELD_NDIV_BIAS
        0,  // QUASAR_FIELD_TXRX_BAND
        1,  // QUASAR_FIELD_TWOG_HS_EN
        0,  // QUASAR_FIELD_TX_RFLO
        0,  // QUASAR_FIELD_TX_IFLO
        1,  // QUASAR_FIELD_TX_RF_TUN
        0,  // QUASAR_FIELD_TX_IF_TUN
        0,  // QUASAR_FIELD_RX_CASC_GC
        0,  // QUASAR_FIELD_TX_CASC_GC
        0,  // QUASAR_FIELD_EXT_IFLO_EN
        1,  // QUASAR_FIELD_IQ_DIV_EN
        2,  // QUASAR_FIELD_NDIV_LO
        0,  // QUASAR_FIELD_EXT_LO
        1,  // QUASAR_FIELD_TX1_LO
        1,  // QUASAR_FIELD_TX0_LO
        1,  // QUASAR_FIELD_RX2_LO
        1,  // QUASAR_FIELD_RX1_LO
        1,  // QUASAR_FIELD_RX0_LO
        7   // QUASAR_FIELD_MOD_INV
    },          // RF_SUBBAND_2_4_GHZ
    {
        4,  // QUASAR_FIELD_IBUF_BIAS
        4,  // QUASAR_FIELD_VCO_BIAS
        1,  // QUASAR_FIELD_NDIV_BIAS
        1,  // QUASAR_FIELD_TXRX_BAND
        0,  // QUASAR_FIELD_TWOG_HS_EN
        0,  // QUASAR_FIELD_TX_RFLO
        0,  // QUASAR_FIELD_TX_IFLO
        4,  // QUASAR_FIELD_TX_RF_TUN
        1,  // QUASAR_FIELD_TX_IF_TUN
        0,  // QUASAR_FIELD_RX_CASC_GC
        0,  // QUASAR_FIELD_TX_CASC_GC
        0,  // QUASAR_FIELD_EXT_IFLO_EN
        1,  // QUASAR_FIELD_IQ_DIV_EN
        2,  // QUASAR_FIELD_NDIV_LO
        0,  // QUASAR_FIELD_EXT_LO
        1,  // QUASAR_FIELD_TX1_LO
        1,  // QUASAR_FIELD_TX0_LO
        1,  // QUASAR_FIELD_RX2_LO
        1,  // QUASAR_FIELD_RX1_LO
        1,  // QUASAR_FIELD_RX0_LO
        0   // QUASAR_FIELD_MOD_INV
    },          // RF_SUBBAND_5_LOW_GHZ
    {
        4,  // QUASAR_FIELD_IBUF_BIAS
        4,  // QUASAR_FIELD_VCO_BIAS
        1,  // QUASAR_FIELD_NDIV_BIAS
        1,  // QUASAR_FIELD_TXRX_BAND
        0,  // QUASAR_FIELD_TWOG_HS_EN
        0,  // QUASAR_FIELD_TX_RFLO
        0,  // QUASAR_FIELD_TX_IFLO
        4,  // QUASAR_FIELD_TX_RF_TUN
        1,  // QUASAR_FIELD_TX_IF_TUN
        0,  // QUASAR_FIELD_RX_CASC_GC
        0,  // QUASAR_FIELD_TX_CASC_GC
        0,  // QUASAR_FIELD_EXT_IFLO_EN
        1,  // QUASAR_FIELD_IQ_DIV_EN
        2,  // QUASAR_FIELD_NDIV_LO
        0,  // QUASAR_FIELD_EXT_LO
        1,  // QUASAR_FIELD_TX1_LO
        1,  // QUASAR_FIELD_TX0_LO
        1,  // QUASAR_FIELD_RX2_LO
        1,  // QUASAR_FIELD_RX1_LO
        1,  // QUASAR_FIELD_RX0_LO
        0   // QUASAR_FIELD_MOD_INV
    },          // RF_SUBBAND_5_MID_GHZ
    {
        4,  // QUASAR_FIELD_IBUF_BIAS
        4,  // QUASAR_FIELD_VCO_BIAS
        1,  // QUASAR_FIELD_NDIV_BIAS
        1,  // QUASAR_FIELD_TXRX_BAND
        0,  // QUASAR_FIELD_TWOG_HS_EN
        0,  // QUASAR_FIELD_TX_RFLO
        0,  // QUASAR_FIELD_TX_IFLO
        4,  // QUASAR_FIELD_TX_RF_TUN
        1,  // QUASAR_FIELD_TX_IF_TUN
        0,  // QUASAR_FIELD_RX_CASC_GC
        0,  // QUASAR_FIELD_TX_CASC_GC
        0,  // QUASAR_FIELD_EXT_IFLO_EN
        1,  // QUASAR_FIELD_IQ_DIV_EN
        2,  // QUASAR_FIELD_NDIV_LO
        0,  // QUASAR_FIELD_EXT_LO
        1,  // QUASAR_FIELD_TX1_LO
        1,  // QUASAR_FIELD_TX0_LO
        1,  // QUASAR_FIELD_RX2_LO
        1,  // QUASAR_FIELD_RX1_LO
        1,  // QUASAR_FIELD_RX0_LO
        0   // QUASAR_FIELD_MOD_INV
    },          // RF_SUBBAND_5_HIGH_GHZ
    {
        4,  // QUASAR_FIELD_IBUF_BIAS
        4,  // QUASAR_FIELD_VCO_BIAS
        1,  // QUASAR_FIELD_NDIV_BIAS
        1,  // QUASAR_FIELD_TXRX_BAND
        0,  // QUASAR_FIELD_TWOG_HS_EN
        0,  // QUASAR_FIELD_TX_RFLO
        0,  // QUASAR_FIELD_TX_IFLO
        4,  // QUASAR_FIELD_TX_RF_TUN
        1,  // QUASAR_FIELD_TX_IF_TUN
        0,  // QUASAR_FIELD_RX_CASC_GC
        0,  // QUASAR_FIELD_TX_CASC_GC
        0,  // QUASAR_FIELD_EXT_IFLO_EN
        1,  // QUASAR_FIELD_IQ_DIV_EN
        2,  // QUASAR_FIELD_NDIV_LO
        0,  // QUASAR_FIELD_EXT_LO
        1,  // QUASAR_FIELD_TX1_LO
        1,  // QUASAR_FIELD_TX0_LO
        1,  // QUASAR_FIELD_RX2_LO
        1,  // QUASAR_FIELD_RX1_LO
        1,  // QUASAR_FIELD_RX0_LO
        0   // QUASAR_FIELD_MOD_INV
    }           // RF_SUBBAND_4_9_GHZ
};


#define SET_QUASAR_DIVIDERS(intPart, fracPart)                                                              \
{   tANI_U32 fracHi = (((fracPart) >> 18) & QUASAR_NDIV_FRAC_HI_MASK);                                        \
    tANI_U32 fracLo = (fracPart) & MSK_18;                                                                    \
                                                                                                            \
    SET_SPI_FIELD(pMac, QUASAR_REG_PLL1, QUASAR_NDIV_INT_MASK, QUASAR_NDIV_INT_OFFSET, intPart);            \
    SET_SPI_FIELD(pMac, QUASAR_REG_PLL1, QUASAR_NDIV_FRAC_HI_MASK, QUASAR_NDIV_FRAC_HI_OFFSET, fracHi);     \
    SET_SPI_FIELD(pMac, QUASAR_REG_PLL2, QUASAR_NDIV_FRAC_LO_MASK, QUASAR_NDIV_FRAC_LO_OFFSET, fracLo);     \
}


static eHalStatus PopulateQuasarRegistersFromBringupTable(tpAniSirGlobal pMac, eRfChannels chan);
static eHalStatus rfSetCurBand(tpAniSirGlobal pMac, eRfSubBand band);

static sQuasarDividers CalcNDiv(eRfChannels chan);






eHalStatus rfWriteQuasarField(tpAniSirGlobal pMac, eQuasarFields quasarField, tANI_U32 value)
{
    eHalStatus retVal;

    switch (quasarField)
    {
        case QUASAR_FIELD_NDIV_INT:
            SET_SPI_FIELD(pMac, QUASAR_REG_PLL1, QUASAR_NDIV_INT_MASK, QUASAR_NDIV_INT_OFFSET, (value & QUASAR_NDIV_INT_MASK));
            break;
        case QUASAR_FIELD_NDIV_FRAC:
            SET_SPI_FIELD(pMac, QUASAR_REG_PLL1, QUASAR_NDIV_FRAC_HI_MASK, QUASAR_NDIV_FRAC_HI_OFFSET, (value & (QUASAR_NDIV_FRAC_HI_MASK << 18)));
            SET_SPI_FIELD(pMac, QUASAR_REG_PLL2, QUASAR_NDIV_FRAC_LO_MASK, QUASAR_NDIV_FRAC_LO_OFFSET, (value & QUASAR_NDIV_FRAC_LO_MASK));
            break;
        case QUASAR_FIELD_CP_GAIN:
            SET_SPI_FIELD(pMac, QUASAR_REG_PLL1, QUASAR_CP_GAIN_MASK, QUASAR_CP_GAIN_OFFSET, (value & QUASAR_CP_GAIN_MASK));
            break;
        case QUASAR_FIELD_NDIV_EN:
            SET_SPI_FIELD(pMac, QUASAR_REG_PLL1, QUASAR_NDIV_EN_MASK, QUASAR_NDIV_EN_OFFSET, (value & QUASAR_NDIV_EN_MASK));
            break;
        case QUASAR_FIELD_PFD_EN:
            SET_SPI_FIELD(pMac, QUASAR_REG_PLL1, QUASAR_PFD_EN_MASK, QUASAR_PFD_EN_OFFSET, (value & QUASAR_PFD_EN_MASK));
            break;
        case QUASAR_FIELD_CP_EN:
            SET_SPI_FIELD(pMac, QUASAR_REG_PLL1, QUASAR_CP_EN_MASK, QUASAR_CP_EN_OFFSET, (value & QUASAR_CP_EN_MASK));
            break;
        case QUASAR_FIELD_FNZ:
            SET_SPI_FIELD(pMac, QUASAR_REG_PLL1, QUASAR_FNZ_MASK, QUASAR_FNZ_OFFSET, (value & QUASAR_FNZ_MASK));
            break;
        case QUASAR_FIELD_PLL_CT_ACTIVE:
            SET_SPI_FIELD(pMac, QUASAR_REG_PLL3, QUASAR_PLL_CT_ACTIVE_MASK, QUASAR_PLL_CT_ACTIVE_OFFSET, (value & QUASAR_PLL_CT_ACTIVE_MASK));
            break;
        case QUASAR_FIELD_VCO_EN:
            SET_SPI_FIELD(pMac, QUASAR_REG_PLL3, QUASAR_VCO_EN_MASK, QUASAR_VCO_EN_OFFSET, (value & QUASAR_VCO_EN_MASK));
            break;
        case QUASAR_FIELD_CT_EN:
            SET_SPI_FIELD(pMac, QUASAR_REG_PLL3, QUASAR_CT_EN_MASK, QUASAR_CT_EN_OFFSET, (value & QUASAR_CT_EN_MASK));
            break;
        case QUASAR_FIELD_RDIV:
            SET_SPI_FIELD(pMac, QUASAR_REG_PLL3, QUASAR_RDIV_MASK, QUASAR_RDIV_OFFSET, (value & QUASAR_RDIV_MASK));
            break;
        case QUASAR_FIELD_EXT_RFLO_EN:
            SET_SPI_FIELD(pMac, QUASAR_REG_PLL3, QUASAR_EXT_RFLO_EN_MASK, QUASAR_EXT_RFLO_EN_OFFSET, (value & QUASAR_EXT_RFLO_EN_MASK));
            break;
        case QUASAR_FIELD_RFLO_OUT_EN:
            SET_SPI_FIELD(pMac, QUASAR_REG_PLL3, QUASAR_RFLO_OUT_EN_MASK, QUASAR_RFLO_OUT_EN_OFFSET, (value & QUASAR_RFLO_OUT_EN_MASK));
            break;
        case QUASAR_FIELD_PLL_CT_START:
            SET_SPI_FIELD(pMac, QUASAR_REG_PLL3, QUASAR_PLL_CT_START_MASK, QUASAR_PLL_CT_START_OFFSET, (value & QUASAR_PLL_CT_START_MASK));
            break;
        case QUASAR_FIELD_PLL_ADC_AQ:
            SET_SPI_FIELD(pMac, QUASAR_REG_PLL3, QUASAR_PLL_ADC_AQ_MASK, QUASAR_PLL_ADC_AQ_OFFSET, (value & QUASAR_PLL_ADC_AQ_MASK));
            break;
        case QUASAR_FIELD_VTUNE_VAL:
            SET_SPI_FIELD(pMac, QUASAR_REG_PLL3, QUASAR_VTUNE_VAL_MASK, QUASAR_VTUNE_VAL_OFFSET, (value & QUASAR_VTUNE_VAL_MASK));
            break;
        case QUASAR_FIELD_CT_MAINT:
            SET_SPI_FIELD(pMac, QUASAR_REG_PLL3, QUASAR_CT_MAINT_MASK, QUASAR_CT_MAINT_OFFSET, (value & QUASAR_CT_MAINT_MASK));
            break;
        case QUASAR_FIELD_PLL_TUNE_ERR:
            SET_SPI_FIELD(pMac, QUASAR_REG_PLL4, QUASAR_PLL_TUNE_ERR_MASK, QUASAR_PLL_TUNE_ERR_OFFSET, (value & QUASAR_PLL_TUNE_ERR_MASK));
            break;
        case QUASAR_FIELD_PLL_VREG_D:
            SET_SPI_FIELD(pMac, QUASAR_REG_PLL4, QUASAR_PLL_VREG_D_MASK, QUASAR_PLL_VREG_D_OFFSET, (value & QUASAR_PLL_VREG_D_MASK));
            break;
        case QUASAR_FIELD_PLL_VREG_EN:
            SET_SPI_FIELD(pMac, QUASAR_REG_PLL4, QUASAR_PLL_VREG_EN_MASK, QUASAR_PLL_VREG_EN_OFFSET, (value & QUASAR_PLL_VREG_EN_MASK));
            break;
        case QUASAR_FIELD_VCO_VREG_EN:
            SET_SPI_FIELD(pMac, QUASAR_REG_PLL4, QUASAR_VCO_VREG_EN_MASK, QUASAR_VCO_VREG_EN_OFFSET, (value & QUASAR_VCO_VREG_EN_MASK));
            break;
        case QUASAR_FIELD_VTUNE_RANGE:
            SET_SPI_FIELD(pMac, QUASAR_REG_PLL4, QUASAR_VTUNE_RANGE_MASK, QUASAR_VTUNE_RANGE_OFFSET, (value & QUASAR_VTUNE_RANGE_MASK));
            break;
        case QUASAR_FIELD_CT_VAL:
            SET_SPI_FIELD(pMac, QUASAR_REG_PLL4, QUASAR_CT_VAL_MASK, QUASAR_CT_VAL_OFFSET, (value & QUASAR_CT_VAL_MASK));
            break;
        case QUASAR_FIELD_VTUNE_T:
            SET_SPI_FIELD(pMac, QUASAR_REG_PLL4, QUASAR_VTUNE_T_MASK, QUASAR_VTUNE_T_OFFSET, (value & QUASAR_VTUNE_T_MASK));
            break;
        case QUASAR_FIELD_VCO_SEL:
            SET_SPI_FIELD(pMac, QUASAR_REG_PLL5, QUASAR_VCO_SEL_MASK, QUASAR_VCO_SEL_OFFSET, (value & QUASAR_VCO_SEL_MASK));
            break;
        case QUASAR_FIELD_CT_SETIME:
            SET_SPI_FIELD(pMac, QUASAR_REG_PLL5, QUASAR_CT_SETIME_MASK, QUASAR_CT_SETIME_OFFSET, (value & QUASAR_CT_SETIME_MASK));
            break;
        case QUASAR_FIELD_THREE_FOURB:
            SET_SPI_FIELD(pMac, QUASAR_REG_PLL5, QUASAR_THREE_FOURB_MASK, QUASAR_THREE_FOURB_OFFSET, (value & QUASAR_THREE_FOURB_MASK));
            break;
        case QUASAR_FIELD_IBUF_BIAS:
            SET_SPI_FIELD(pMac, QUASAR_REG_PLL5, QUASAR_IBUF_BIAS_MASK, QUASAR_IBUF_BIAS_OFFSET, (value & QUASAR_IBUF_BIAS_MASK));
            break;
        case QUASAR_FIELD_VCO_BIAS:
            SET_SPI_FIELD(pMac, QUASAR_REG_PLL5, QUASAR_VCO_BIAS_MASK, QUASAR_VCO_BIAS_OFFSET, (value & QUASAR_VCO_BIAS_MASK));
            break;
        case QUASAR_FIELD_CP_LOGIC:
            SET_SPI_FIELD(pMac, QUASAR_REG_PLL5, QUASAR_CP_LOGIC_MASK, QUASAR_CP_LOGIC_OFFSET, (value & QUASAR_CP_LOGIC_MASK));
            break;
        case QUASAR_FIELD_FRAC_OFFS:
            SET_SPI_FIELD(pMac, QUASAR_REG_PLL5, QUASAR_FRAC_OFFS_MASK, QUASAR_FRAC_OFFS_OFFSET, (value & QUASAR_FRAC_OFFS_MASK));
            break;
        case QUASAR_FIELD_NDIV_BIAS:
            SET_SPI_FIELD(pMac, QUASAR_REG_PLL5, QUASAR_NDIV_BIAS_MASK, QUASAR_NDIV_BIAS_OFFSET, (value & QUASAR_NDIV_BIAS_MASK));
            break;
        case QUASAR_FIELD_LBW_CAL_SU:
            SET_SPI_FIELD(pMac, QUASAR_REG_PLL6, QUASAR_LBW_CAL_SU_MASK, QUASAR_LBW_CAL_SU_OFFSET, (value & QUASAR_LBW_CAL_SU_MASK));
            break;
        case QUASAR_FIELD_CP_GAIN_CAL:
            SET_SPI_FIELD(pMac, QUASAR_REG_PLL6, QUASAR_CP_GAIN_CAL_MASK, QUASAR_CP_GAIN_CAL_OFFSET, (value & QUASAR_CP_GAIN_CAL_MASK));
            break;
        case QUASAR_FIELD_DELTA_NDIV:
            SET_SPI_FIELD(pMac, QUASAR_REG_PLL6, QUASAR_DELTA_NDIV_MASK, QUASAR_DELTA_NDIV_OFFSET, (value & QUASAR_DELTA_NDIV_MASK));
            break;
        case QUASAR_FIELD_PLL_TRANGE:
            SET_SPI_FIELD(pMac, QUASAR_REG_PLL6, QUASAR_PLL_TRANGE_MASK, QUASAR_PLL_TRANGE_OFFSET, (value & QUASAR_PLL_TRANGE_MASK));
            break;
        case QUASAR_FIELD_PLL_TGOAL:
            SET_SPI_FIELD(pMac, QUASAR_REG_PLL6, QUASAR_PLL_TGOAL_MASK, QUASAR_PLL_TGOAL_OFFSET, (value & QUASAR_PLL_TGOAL_MASK));
            break;
        case QUASAR_FIELD_LOOP_CAL_EN:
            SET_SPI_FIELD(pMac, QUASAR_REG_PLL6, QUASAR_LOOP_CAL_EN_MASK, QUASAR_LOOP_CAL_EN_OFFSET, (value & QUASAR_LOOP_CAL_EN_MASK));
            break;
        case QUASAR_FIELD_EXT_IFLO_EN:
            SET_SPI_FIELD(pMac, QUASAR_REG_LO_CONFIG, QUASAR_EXT_IFLO_EN_MASK, QUASAR_EXT_IFLO_EN_OFFSET, (value & QUASAR_EXT_IFLO_EN_MASK));
            break;
        case QUASAR_FIELD_IQ_DIV_EN:
            SET_SPI_FIELD(pMac, QUASAR_REG_LO_CONFIG, QUASAR_IQ_DIV_EN_MASK, QUASAR_IQ_DIV_EN_OFFSET, (value & QUASAR_IQ_DIV_EN_MASK));
            break;
        case QUASAR_FIELD_NDIV_LO:
            SET_SPI_FIELD(pMac, QUASAR_REG_LO_CONFIG, QUASAR_NDIV_LO_MASK, QUASAR_NDIV_LO_OFFSET, (value & QUASAR_NDIV_LO_MASK));
            break;
        case QUASAR_FIELD_EXT_LO:
            SET_SPI_FIELD(pMac, QUASAR_REG_LO_CONFIG, QUASAR_EXT_LO_MASK, QUASAR_EXT_LO_OFFSET, (value & QUASAR_EXT_LO_MASK));
            break;
        case QUASAR_FIELD_TX1_LO:
            SET_SPI_FIELD(pMac, QUASAR_REG_LO_CONFIG, QUASAR_TX1_LO_MASK, QUASAR_TX1_LO_OFFSET, (value & QUASAR_TX1_LO_MASK));
            break;
        case QUASAR_FIELD_TX0_LO:
            SET_SPI_FIELD(pMac, QUASAR_REG_LO_CONFIG, QUASAR_TX0_LO_MASK, QUASAR_TX0_LO_OFFSET, (value & QUASAR_TX0_LO_MASK));
            break;
        case QUASAR_FIELD_RX2_LO:
            SET_SPI_FIELD(pMac, QUASAR_REG_LO_CONFIG, QUASAR_RX2_LO_MASK, QUASAR_RX2_LO_OFFSET, (value & QUASAR_RX2_LO_MASK));
            break;
        case QUASAR_FIELD_RX1_LO:
            SET_SPI_FIELD(pMac, QUASAR_REG_LO_CONFIG, QUASAR_RX1_LO_MASK, QUASAR_RX1_LO_OFFSET, (value & QUASAR_RX1_LO_MASK));
            break;
        case QUASAR_FIELD_RX0_LO:
            SET_SPI_FIELD(pMac, QUASAR_REG_LO_CONFIG, QUASAR_RX0_LO_MASK, QUASAR_RX0_LO_OFFSET, (value & QUASAR_RX0_LO_MASK));
            break;
        case QUASAR_FIELD_MS_REV:
            SET_SPI_FIELD(pMac, QUASAR_REG_STATE, QUASAR_MS_REV_MASK, QUASAR_MS_REV_OFFSET, (value & QUASAR_MS_REV_MASK));
            break;
        case QUASAR_FIELD_XO_DISABLE_ST:
            SET_SPI_FIELD(pMac, QUASAR_REG_STATE, QUASAR_XO_DISABLE_ST_MASK, QUASAR_XO_DISABLE_ST_OFFSET, (value & QUASAR_XO_DISABLE_ST_MASK));
            break;
        case QUASAR_FIELD_RX1_ST_EN:
            SET_SPI_FIELD(pMac, QUASAR_REG_STATE, QUASAR_RX1_ST_EN_MASK, QUASAR_RX1_ST_EN_OFFSET, (value & QUASAR_RX1_ST_EN_MASK));
            break;
        case QUASAR_FIELD_RX_ALL_ST_EN:
            SET_SPI_FIELD(pMac, QUASAR_REG_STATE, QUASAR_RX_ALL_ST_EN_MASK, QUASAR_RX_ALL_ST_EN_OFFSET, (value & QUASAR_RX_ALL_ST_EN_MASK));
            break;
        case QUASAR_FIELD_TX_ST_EN:
            SET_SPI_FIELD(pMac, QUASAR_REG_STATE, QUASAR_TX_ST_EN_MASK, QUASAR_TX_ST_EN_OFFSET, (value & QUASAR_TX_ST_EN_MASK));
            break;
        case QUASAR_FIELD_PLL_EN:
            SET_SPI_FIELD(pMac, QUASAR_REG_STATE, QUASAR_PLL_EN_MASK, QUASAR_PLL_EN_OFFSET, (value & QUASAR_PLL_EN_MASK));
            break;
        case QUASAR_FIELD_TXRX_BAND:
            SET_SPI_FIELD(pMac, QUASAR_REG_STATE, QUASAR_TXRX_BAND_MASK, QUASAR_TXRX_BAND_OFFSET, (value & QUASAR_TXRX_BAND_MASK));
            break;
        case QUASAR_FIELD_MREV:
            SET_SPI_FIELD(pMac, QUASAR_REG_STATE, QUASAR_MREV_MASK, QUASAR_MREV_OFFSET, (value & QUASAR_MREV_MASK));
            break;
        case QUASAR_FIELD_TWOG_HS_EN:
            SET_SPI_FIELD(pMac, QUASAR_REG_STATE, QUASAR_TWOG_HS_EN_MASK, QUASAR_TWOG_HS_EN_OFFSET, (value & QUASAR_TWOG_HS_EN_MASK));
            break;
        case QUASAR_FIELD_RXG_2:
            SET_SPI_FIELD(pMac, QUASAR_REG_GAIN_CONTROL_1, QUASAR_RXG_2_MASK, QUASAR_RXG_2_OFFSET, (value & QUASAR_RXG_2_MASK));
            break;
        case QUASAR_FIELD_RXG_1:
            SET_SPI_FIELD(pMac, QUASAR_REG_GAIN_CONTROL_1, QUASAR_RXG_1_MASK, QUASAR_RXG_1_OFFSET, (value & QUASAR_RXG_1_MASK));
            break;
        case QUASAR_FIELD_RXG_0:
            SET_SPI_FIELD(pMac, QUASAR_REG_GAIN_CONTROL_1, QUASAR_RXG_0_MASK, QUASAR_RXG_0_OFFSET, (value & QUASAR_RXG_0_MASK));
            break;
        case QUASAR_FIELD_GC_SELECT:
            SET_SPI_FIELD(pMac, QUASAR_REG_GAIN_CONTROL_2, QUASAR_GC_SELECT_MASK, QUASAR_GC_SELECT_OFFSET, (value & QUASAR_GC_SELECT_MASK));
            break;
        case QUASAR_FIELD_SPI_LUT_EN:
            SET_SPI_FIELD(pMac, QUASAR_REG_GAIN_CONTROL_2, QUASAR_SPI_LUT_EN_MASK, QUASAR_SPI_LUT_EN_OFFSET, (value & QUASAR_SPI_LUT_EN_MASK));
            break;
        case QUASAR_FIELD_SPI_LUT_INDEX:
            SET_SPI_FIELD(pMac, QUASAR_REG_GAIN_CONTROL_2, QUASAR_SPI_LUT_INDEX_MASK, QUASAR_SPI_LUT_INDEX_OFFSET, (value & QUASAR_SPI_LUT_INDEX_MASK));
            break;
        case QUASAR_FIELD_TX_GAIN_0:
            SET_SPI_FIELD(pMac, QUASAR_REG_GAIN_CONTROL_2, QUASAR_TX_GAIN_0_MASK, QUASAR_TX_GAIN_0_OFFSET, (value & QUASAR_TX_GAIN_0_MASK));
            break;
        case QUASAR_FIELD_TX_GAIN_1:
            SET_SPI_FIELD(pMac, QUASAR_REG_GAIN_CONTROL_2, QUASAR_TX_GAIN_1_MASK, QUASAR_TX_GAIN_1_OFFSET, (value & QUASAR_TX_GAIN_1_MASK));
            break;
        case QUASAR_FIELD_RX_GAIN_2:
            SET_SPI_FIELD(pMac, QUASAR_REG_GAIN_CONTROL_3, QUASAR_RX_GAIN_2_MASK, QUASAR_RX_GAIN_2_OFFSET, (value & QUASAR_RX_GAIN_2_MASK));
            break;
        case QUASAR_FIELD_RX_GAIN_1:
            SET_SPI_FIELD(pMac, QUASAR_REG_GAIN_CONTROL_3, QUASAR_RX_GAIN_1_MASK, QUASAR_RX_GAIN_1_OFFSET, (value & QUASAR_RX_GAIN_1_MASK));
            break;
        case QUASAR_FIELD_RX_GAIN_0:
            SET_SPI_FIELD(pMac, QUASAR_REG_GAIN_CONTROL_4, QUASAR_RX_GAIN_0_MASK, QUASAR_RX_GAIN_0_OFFSET, (value & QUASAR_RX_GAIN_0_MASK));
            break;
        case QUASAR_FIELD_RX0DC_RANGE:
            SET_SPI_FIELD(pMac, QUASAR_REG_DC_OFFSET_CHAIN_0, QUASAR_RX0DC_RANGE_MASK, QUASAR_RX0DC_RANGE_OFFSET, (value & QUASAR_RX0DC_RANGE_MASK));
            break;
        case QUASAR_FIELD_RX0DCI:
            SET_SPI_FIELD(pMac, QUASAR_REG_DC_OFFSET_CHAIN_0, QUASAR_RX0DCI_MASK, QUASAR_RX0DCI_OFFSET, (value & QUASAR_RX0DCI_MASK));
            break;
        case QUASAR_FIELD_RX0DCQ:
            SET_SPI_FIELD(pMac, QUASAR_REG_DC_OFFSET_CHAIN_0, QUASAR_RX0DCQ_MASK, QUASAR_RX0DCQ_OFFSET, (value & QUASAR_RX0DCQ_MASK));
            break;
        case QUASAR_FIELD_RX1DC_RANGE:
            SET_SPI_FIELD(pMac, QUASAR_REG_DC_OFFSET_CHAIN_1, QUASAR_RX1DC_RANGE_MASK, QUASAR_RX1DC_RANGE_OFFSET, (value & QUASAR_RX1DC_RANGE_MASK));
            break;
        case QUASAR_FIELD_RX1DCI:
            SET_SPI_FIELD(pMac, QUASAR_REG_DC_OFFSET_CHAIN_1, QUASAR_RX1DCI_MASK, QUASAR_RX1DCI_OFFSET, (value & QUASAR_RX1DCI_MASK));
            break;
        case QUASAR_FIELD_RX1DCQ:
            SET_SPI_FIELD(pMac, QUASAR_REG_DC_OFFSET_CHAIN_1, QUASAR_RX1DCQ_MASK, QUASAR_RX1DCQ_OFFSET, (value & QUASAR_RX1DCQ_MASK));
            break;
        case QUASAR_FIELD_RX2DC_RANGE:
            SET_SPI_FIELD(pMac, QUASAR_REG_DC_OFFSET_CHAIN_2, QUASAR_RX2DC_RANGE_MASK, QUASAR_RX2DC_RANGE_OFFSET, (value & QUASAR_RX2DC_RANGE_MASK));
            break;
        case QUASAR_FIELD_RX2DCI:
            SET_SPI_FIELD(pMac, QUASAR_REG_DC_OFFSET_CHAIN_2, QUASAR_RX2DCI_MASK, QUASAR_RX2DCI_OFFSET, (value & QUASAR_RX2DCI_MASK));
            break;
        case QUASAR_FIELD_RX2DCQ:
            SET_SPI_FIELD(pMac, QUASAR_REG_DC_OFFSET_CHAIN_2, QUASAR_RX2DCQ_MASK, QUASAR_RX2DCQ_OFFSET, (value & QUASAR_RX2DCQ_MASK));
            break;
        case QUASAR_FIELD_CLIP_EN_0:
            SET_SPI_FIELD(pMac, QUASAR_REG_CFG_1, QUASAR_CLIP_EN_0_MASK, QUASAR_CLIP_EN_0_OFFSET, (value & QUASAR_CLIP_EN_0_MASK));
            break;
        case QUASAR_FIELD_CLIP_EN_1:
            SET_SPI_FIELD(pMac, QUASAR_REG_CFG_1, QUASAR_CLIP_EN_1_MASK, QUASAR_CLIP_EN_1_OFFSET, (value & QUASAR_CLIP_EN_1_MASK));
            break;
        case QUASAR_FIELD_CLIP_EN_2:
            SET_SPI_FIELD(pMac, QUASAR_REG_CFG_1, QUASAR_CLIP_EN_2_MASK, QUASAR_CLIP_EN_2_OFFSET, (value & QUASAR_CLIP_EN_2_MASK));
            break;
        case QUASAR_FIELD_MOD_INV:
            SET_SPI_FIELD(pMac, QUASAR_REG_CFG_1, QUASAR_MOD_INV_MASK, QUASAR_MOD_INV_OFFSET, (value & QUASAR_MOD_INV_MASK));
            break;
        case QUASAR_FIELD_TX_RFLO:
            SET_SPI_FIELD(pMac, QUASAR_REG_CFG_1, QUASAR_TX_RFLO_MASK, QUASAR_TX_RFLO_OFFSET, (value & QUASAR_TX_RFLO_MASK));
            break;
        case QUASAR_FIELD_TX_IFLO:
            SET_SPI_FIELD(pMac, QUASAR_REG_CFG_1, QUASAR_TX_IFLO_MASK, QUASAR_TX_IFLO_OFFSET, (value & QUASAR_TX_IFLO_MASK));
            break;
        case QUASAR_FIELD_XO_DRV:
            SET_SPI_FIELD(pMac, QUASAR_REG_CFG_1, QUASAR_XO_DRV_MASK, QUASAR_XO_DRV_OFFSET, (value & QUASAR_XO_DRV_MASK));
            break;
        case QUASAR_FIELD_XO_BYP:
            SET_SPI_FIELD(pMac, QUASAR_REG_CFG_1, QUASAR_XO_BYP_MASK, QUASAR_XO_BYP_OFFSET, (value & QUASAR_XO_BYP_MASK));
            break;
        case QUASAR_FIELD_XO_BUF_EN:
            SET_SPI_FIELD(pMac, QUASAR_REG_CFG_1, QUASAR_XO_BUF_EN_MASK, QUASAR_XO_BUF_EN_OFFSET, (value & QUASAR_XO_BUF_EN_MASK));
            break;
        case QUASAR_FIELD_TOUT:
            SET_SPI_FIELD(pMac, QUASAR_REG_CFG_1, QUASAR_TOUT_MASK, QUASAR_TOUT_OFFSET, (value & QUASAR_TOUT_MASK));
            break;
        case QUASAR_FIELD_TEN:
            SET_SPI_FIELD(pMac, QUASAR_REG_CFG_1, QUASAR_TEN_MASK, QUASAR_TEN_OFFSET, (value & QUASAR_TEN_MASK));
            break;
        case QUASAR_FIELD_RC_ATUNE_VAL:
            SET_SPI_FIELD(pMac, QUASAR_REG_CFG_2, QUASAR_RC_ATUNE_VAL_MASK, QUASAR_RC_ATUNE_VAL_OFFSET, (value & QUASAR_RC_ATUNE_VAL_MASK));
            break;
        case QUASAR_FIELD_RC_TUNE_EN:
            SET_SPI_FIELD(pMac, QUASAR_REG_CFG_2, QUASAR_RC_TUNE_EN_MASK, QUASAR_RC_TUNE_EN_OFFSET, (value & QUASAR_RC_TUNE_EN_MASK));
            break;
        case QUASAR_FIELD_BBF_RX_TUNE:
            SET_SPI_FIELD(pMac, QUASAR_REG_CFG_2, QUASAR_BBF_RX_TUNE_MASK, QUASAR_BBF_RX_TUNE_OFFSET, (value & QUASAR_BBF_RX_TUNE_MASK));
            break;
        case QUASAR_FIELD_BBF_TX_TUNE:
            SET_SPI_FIELD(pMac, QUASAR_REG_CFG_2, QUASAR_BBF_TX_TUNE_MASK, QUASAR_BBF_TX_TUNE_OFFSET, (value & QUASAR_BBF_TX_TUNE_MASK));
            break;
        case QUASAR_FIELD_BBF_ATUNE_EN:
            SET_SPI_FIELD(pMac, QUASAR_REG_CFG_2, QUASAR_BBF_ATUNE_EN_MASK, QUASAR_BBF_ATUNE_EN_OFFSET, (value & QUASAR_BBF_ATUNE_EN_MASK));
            break;
        case QUASAR_FIELD_RX2_DCOC_EN:
            SET_SPI_FIELD(pMac, QUASAR_REG_CFG_3, QUASAR_RX2_DCOC_EN_MASK, QUASAR_RX2_DCOC_EN_OFFSET, (value & QUASAR_RX2_DCOC_EN_MASK));
            break;
        case QUASAR_FIELD_RX1_DCOC_EN:
            SET_SPI_FIELD(pMac, QUASAR_REG_CFG_3, QUASAR_RX1_DCOC_EN_MASK, QUASAR_RX1_DCOC_EN_OFFSET, (value & QUASAR_RX1_DCOC_EN_MASK));
            break;
        case QUASAR_FIELD_RX0_DCOC_EN:
            SET_SPI_FIELD(pMac, QUASAR_REG_CFG_3, QUASAR_RX0_DCOC_EN_MASK, QUASAR_RX0_DCOC_EN_OFFSET, (value & QUASAR_RX0_DCOC_EN_MASK));
            break;
        case QUASAR_FIELD_TX1_D:
            SET_SPI_FIELD(pMac, QUASAR_REG_CFG_3, QUASAR_TX1_D_MASK, QUASAR_TX1_D_OFFSET, (value & QUASAR_TX1_D_MASK));
            break;
        case QUASAR_FIELD_TX1D_D:
            SET_SPI_FIELD(pMac, QUASAR_REG_CFG_3, QUASAR_TX1D_D_MASK, QUASAR_TX1D_D_OFFSET, (value & QUASAR_TX1D_D_MASK));
            break;
        case QUASAR_FIELD_RX1_D:
            SET_SPI_FIELD(pMac, QUASAR_REG_CFG_3, QUASAR_RX1_D_MASK, QUASAR_RX1_D_OFFSET, (value & QUASAR_RX1_D_MASK));
            break;
        case QUASAR_FIELD_CLK_GATE:
            SET_SPI_FIELD(pMac, QUASAR_REG_CFG_4, QUASAR_CLK_GATE_MASK, QUASAR_CLK_GATE_OFFSET, (value & QUASAR_CLK_GATE_MASK));
            break;
        case QUASAR_FIELD_TX_DBIAS:
            SET_SPI_FIELD(pMac, QUASAR_REG_CFG_4, QUASAR_TX_DBIAS_MASK, QUASAR_TX_DBIAS_OFFSET, (value & QUASAR_TX_DBIAS_MASK));
            break;
        case QUASAR_FIELD_TX0_D:
            SET_SPI_FIELD(pMac, QUASAR_REG_CFG_4, QUASAR_TX0_D_MASK, QUASAR_TX0_D_OFFSET, (value & QUASAR_TX0_D_MASK));
            break;
        case QUASAR_FIELD_TX0D_D:
            SET_SPI_FIELD(pMac, QUASAR_REG_CFG_4, QUASAR_TX0D_D_MASK, QUASAR_TX0D_D_OFFSET, (value & QUASAR_TX0D_D_MASK));
            break;
        case QUASAR_FIELD_RX0_D:
            SET_SPI_FIELD(pMac, QUASAR_REG_CFG_4, QUASAR_RX0_D_MASK, QUASAR_RX0_D_OFFSET, (value & QUASAR_RX0_D_MASK));
            break;
        case QUASAR_FIELD_RX_IBIAS_EN:
            SET_SPI_FIELD(pMac, QUASAR_REG_CFG_5, QUASAR_RX_IBIAS_EN_MASK, QUASAR_RX_IBIAS_EN_OFFSET, (value & QUASAR_RX_IBIAS_EN_MASK));
            break;
        case QUASAR_FIELD_RX_RF_MIX_EN:
            SET_SPI_FIELD(pMac, QUASAR_REG_CFG_5, QUASAR_RX_RF_MIX_EN_MASK, QUASAR_RX_RF_MIX_EN_OFFSET, (value & QUASAR_RX_RF_MIX_EN_MASK));
            break;
        case QUASAR_FIELD_RX_IF_MIX_EN:
            SET_SPI_FIELD(pMac, QUASAR_REG_CFG_5, QUASAR_RX_IF_MIX_EN_MASK, QUASAR_RX_IF_MIX_EN_OFFSET, (value & QUASAR_RX_IF_MIX_EN_MASK));
            break;
        case QUASAR_FIELD_RX_IF_TUN:
            SET_SPI_FIELD(pMac, QUASAR_REG_CFG_5, QUASAR_RX_IF_TUN_MASK, QUASAR_RX_IF_TUN_OFFSET, (value & QUASAR_RX_IF_TUN_MASK));
            break;
        case QUASAR_FIELD_TX_RF_TUN:
            SET_SPI_FIELD(pMac, QUASAR_REG_CFG_5, QUASAR_TX_RF_TUN_MASK, QUASAR_TX_RF_TUN_OFFSET, (value & QUASAR_TX_RF_TUN_MASK));
            break;
        case QUASAR_FIELD_TX_IF_TUN:
            SET_SPI_FIELD(pMac, QUASAR_REG_CFG_5, QUASAR_TX_IF_TUN_MASK, QUASAR_TX_IF_TUN_OFFSET, (value & QUASAR_TX_IF_TUN_MASK));
            break;
        case QUASAR_FIELD_RX2_D:
            SET_SPI_FIELD(pMac, QUASAR_REG_CFG_5, QUASAR_RX2_D_MASK, QUASAR_RX2_D_OFFSET, (value & QUASAR_RX2_D_MASK));
            break;
        case QUASAR_FIELD_RX_CASC_GC:
            SET_SPI_FIELD(pMac, QUASAR_REG_CFG_6, QUASAR_RX_CASC_GC_MASK, QUASAR_RX_CASC_GC_OFFSET, (value & QUASAR_RX_CASC_GC_MASK));
            break;
        case QUASAR_FIELD_TX_CASC_GC:
            SET_SPI_FIELD(pMac, QUASAR_REG_CFG_6, QUASAR_TX_CASC_GC_MASK, QUASAR_TX_CASC_GC_OFFSET, (value & QUASAR_TX_CASC_GC_MASK));
            break;
        case QUASAR_FIELD_RX_IF_LODRV_EN:
            SET_SPI_FIELD(pMac, QUASAR_REG_CFG_6, QUASAR_RX_IF_LODRV_EN_MASK, QUASAR_RX_IF_LODRV_EN_OFFSET, (value & QUASAR_RX_IF_LODRV_EN_MASK));
            break;
        case QUASAR_FIELD_XO_GM:
            SET_SPI_FIELD(pMac, QUASAR_REG_CFG_6, QUASAR_XO_GM_MASK, QUASAR_XO_GM_OFFSET, (value & QUASAR_XO_GM_MASK));
            break;
        case QUASAR_FIELD_CLIP_TEST:
            SET_SPI_FIELD(pMac, QUASAR_REG_CFG_6, QUASAR_CLIP_TEST_MASK, QUASAR_CLIP_TEST_OFFSET, (value & QUASAR_CLIP_TEST_MASK));
            break;
        case QUASAR_FIELD_CLIP_FREQ:
            SET_SPI_FIELD(pMac, QUASAR_REG_CFG_6, QUASAR_CLIP_FREQ_MASK, QUASAR_CLIP_FREQ_OFFSET, (value & QUASAR_CLIP_FREQ_MASK));
            break;
        case QUASAR_FIELD_CLIP_DET_TH:
            SET_SPI_FIELD(pMac, QUASAR_REG_CFG_6, QUASAR_CLIP_DET_TH_MASK, QUASAR_CLIP_DET_TH_OFFSET, (value & QUASAR_CLIP_DET_TH_MASK));
            break;
        case QUASAR_FIELD_BBF_BIAS_MODE:
            SET_SPI_FIELD(pMac, QUASAR_REG_CFG_6, QUASAR_BBF_BIAS_MODE_MASK, QUASAR_BBF_BIAS_MODE_OFFSET, (value & QUASAR_BBF_BIAS_MODE_MASK));
            break;
        case QUASAR_FIELD_TX_REG_MODE:
            SET_SPI_FIELD(pMac, QUASAR_REG_CFG_6, QUASAR_TX_REG_MODE_MASK, QUASAR_TX_REG_MODE_OFFSET, (value & QUASAR_TX_REG_MODE_MASK));
            break;
        case QUASAR_FIELD_TX_VREG_BYP:
            SET_SPI_FIELD(pMac, QUASAR_REG_CFG_6, QUASAR_TX_VREG_BYP_MASK, QUASAR_TX_VREG_BYP_OFFSET, (value & QUASAR_TX_VREG_BYP_MASK));
            break;
        case QUASAR_FIELD_NDIV_OUT:
            SET_SPI_FIELD(pMac, QUASAR_REG_TEST_1, QUASAR_NDIV_OUT_MASK, QUASAR_NDIV_OUT_OFFSET, (value & QUASAR_NDIV_OUT_MASK));
            break;
        case QUASAR_FIELD_RX_LB_GAIN:
            SET_SPI_FIELD(pMac, QUASAR_REG_TEST_1, QUASAR_RX_LB_GAIN_MASK, QUASAR_RX_LB_GAIN_OFFSET, (value & QUASAR_RX_LB_GAIN_MASK));
            break;
        case QUASAR_FIELD_TX_LB_GAIN:
            SET_SPI_FIELD(pMac, QUASAR_REG_TEST_1, QUASAR_TX_LB_GAIN_MASK, QUASAR_TX_LB_GAIN_OFFSET, (value & QUASAR_TX_LB_GAIN_MASK));
            break;
        case QUASAR_FIELD_TEST_LB_RF:
            SET_SPI_FIELD(pMac, QUASAR_REG_TEST_1, QUASAR_TEST_LB_RF_MASK, QUASAR_TEST_LB_RF_OFFSET, (value & QUASAR_TEST_LB_RF_MASK));
            break;
        case QUASAR_FIELD_RX_IQ_TEST_0:
            SET_SPI_FIELD(pMac, QUASAR_REG_TEST_1, QUASAR_RX_IQ_TEST_0_MASK, QUASAR_RX_IQ_TEST_0_OFFSET, (value & QUASAR_RX_IQ_TEST_0_MASK));
            break;
        case QUASAR_FIELD_RX_IQ_TEST_1:
            SET_SPI_FIELD(pMac, QUASAR_REG_TEST_1, QUASAR_RX_IQ_TEST_1_MASK, QUASAR_RX_IQ_TEST_1_OFFSET, (value & QUASAR_RX_IQ_TEST_1_MASK));
            break;
        case QUASAR_FIELD_RX_IQ_TEST_2_1:
            SET_SPI_FIELD(pMac, QUASAR_REG_TEST_1, QUASAR_RX_IQ_TEST_2_1_MASK, QUASAR_RX_IQ_TEST_2_1_OFFSET, (value & QUASAR_RX_IQ_TEST_2_1_MASK));
            break;
        case QUASAR_FIELD_RX_IQ_TEST_2_0:
            SET_SPI_FIELD(pMac, QUASAR_REG_TEST_1, QUASAR_RX_IQ_TEST_2_0_MASK, QUASAR_RX_IQ_TEST_2_0_OFFSET, (value & QUASAR_RX_IQ_TEST_2_0_MASK));
            break;
        case QUASAR_FIELD_TX_CAL_EN_0:
            SET_SPI_FIELD(pMac, QUASAR_REG_TEST_1, QUASAR_TX_CAL_EN_0_MASK, QUASAR_TX_CAL_EN_0_OFFSET, (value & QUASAR_TX_CAL_EN_0_MASK));
            break;
        case QUASAR_FIELD_TX_CAL_EN_1:
            SET_SPI_FIELD(pMac, QUASAR_REG_TEST_1, QUASAR_TX_CAL_EN_1_MASK, QUASAR_TX_CAL_EN_1_OFFSET, (value & QUASAR_TX_CAL_EN_1_MASK));
            break;
        case QUASAR_FIELD_DET_OFFSET:
            SET_SPI_FIELD(pMac, QUASAR_REG_TEST_1, QUASAR_DET_OFFSET_MASK, QUASAR_DET_OFFSET_OFFSET, (value & QUASAR_DET_OFFSET_MASK));
            break;
        case QUASAR_FIELD_DET_GAIN:
            SET_SPI_FIELD(pMac, QUASAR_REG_TEST_1, QUASAR_DET_GAIN_MASK, QUASAR_DET_GAIN_OFFSET, (value & QUASAR_DET_GAIN_MASK));
            break;
        case QUASAR_FIELD_RDIV_TEST:
            SET_SPI_FIELD(pMac, QUASAR_REG_TEST_1, QUASAR_RDIV_TEST_MASK, QUASAR_RDIV_TEST_OFFSET, (value & QUASAR_RDIV_TEST_MASK));
            break;
        case QUASAR_FIELD_TX_DRVR_GAIN:
            SET_SPI_FIELD(pMac, QUASAR_REG_TEST_2, QUASAR_TX_DRVR_GAIN_MASK, QUASAR_TX_DRVR_GAIN_OFFSET, (value & QUASAR_TX_DRVR_GAIN_MASK));
            break;
        case QUASAR_FIELD_TX_VGA_GAIN:
            SET_SPI_FIELD(pMac, QUASAR_REG_TEST_2, QUASAR_TX_VGA_GAIN_MASK, QUASAR_TX_VGA_GAIN_OFFSET, (value & QUASAR_TX_VGA_GAIN_MASK));
            break;
        case QUASAR_FIELD_TX_IQ_GAIN:
            SET_SPI_FIELD(pMac, QUASAR_REG_TEST_2, QUASAR_TX_IQ_GAIN_MASK, QUASAR_TX_IQ_GAIN_OFFSET, (value & QUASAR_TX_IQ_GAIN_MASK));
            break;
        case QUASAR_FIELD_TX_GTEST:
            SET_SPI_FIELD(pMac, QUASAR_REG_TEST_2, QUASAR_TX_GTEST_MASK, QUASAR_TX_GTEST_OFFSET, (value & QUASAR_TX_GTEST_MASK));
            break;
        case QUASAR_FIELD_BBF_IBIAS_CTL:
            SET_SPI_FIELD(pMac, QUASAR_REG_TEST_2, QUASAR_BBF_IBIAS_CTL_MASK, QUASAR_BBF_IBIAS_CTL_OFFSET, (value & QUASAR_BBF_IBIAS_CTL_MASK));
            break;
        case QUASAR_FIELD_BBF_TEST_I:
            SET_SPI_FIELD(pMac, QUASAR_REG_TEST_3, QUASAR_BBF_TEST_I_MASK, QUASAR_BBF_TEST_I_OFFSET, (value & QUASAR_BBF_TEST_I_MASK));
            break;
        case QUASAR_FIELD_BBF_TEST_Q:
            SET_SPI_FIELD(pMac, QUASAR_REG_TEST_3, QUASAR_BBF_TEST_Q_MASK, QUASAR_BBF_TEST_Q_OFFSET, (value & QUASAR_BBF_TEST_Q_MASK));
            break;
        case QUASAR_FIELD_CT_VAL_AUTO:
            SET_SPI_FIELD(pMac, QUASAR_REG_TEST_3, QUASAR_CT_VAL_AUTO_MASK, QUASAR_CT_VAL_AUTO_OFFSET, (value & QUASAR_CT_VAL_AUTO_MASK));
            break;
        case QUASAR_FIELD_IO_TEST:
            SET_SPI_FIELD(pMac, QUASAR_REG_TEST_4, QUASAR_IO_TEST_MASK, QUASAR_IO_TEST_OFFSET, (value & QUASAR_IO_TEST_MASK));
            break;
        case QUASAR_FIELD_TX_RF_EN:
            SET_SPI_FIELD(pMac, QUASAR_REG_TEST_4, QUASAR_TX_RF_EN_MASK, QUASAR_TX_RF_EN_OFFSET, (value & QUASAR_TX_RF_EN_MASK));
            break;
        case QUASAR_FIELD_TX_DRVR_EN:
            SET_SPI_FIELD(pMac, QUASAR_REG_TEST_4, QUASAR_TX_DRVR_EN_MASK, QUASAR_TX_DRVR_EN_OFFSET, (value & QUASAR_TX_DRVR_EN_MASK));
            break;
        case QUASAR_FIELD_TX_IQ_EN:
            SET_SPI_FIELD(pMac, QUASAR_REG_TEST_4, QUASAR_TX_IQ_EN_MASK, QUASAR_TX_IQ_EN_OFFSET, (value & QUASAR_TX_IQ_EN_MASK));
            break;
        case QUASAR_FIELD_BBF_ENQ:
            SET_SPI_FIELD(pMac, QUASAR_REG_TEST_4, QUASAR_BBF_ENQ_MASK, QUASAR_BBF_ENQ_OFFSET, (value & QUASAR_BBF_ENQ_MASK));
            break;
        case QUASAR_FIELD_BBF_ENI:
            SET_SPI_FIELD(pMac, QUASAR_REG_TEST_4, QUASAR_BBF_ENI_MASK, QUASAR_BBF_ENI_OFFSET, (value & QUASAR_BBF_ENI_MASK));
            break;
        case QUASAR_FIELD_RX_GMTUNE_EN:
            SET_SPI_FIELD(pMac, QUASAR_REG_TEST_5, QUASAR_RX_GMTUNE_EN_MASK, QUASAR_RX_GMTUNE_EN_OFFSET, (value & QUASAR_RX_GMTUNE_EN_MASK));
            break;
        case QUASAR_FIELD_XO_REG_BYP:
            SET_SPI_FIELD(pMac, QUASAR_REG_TEST_5, QUASAR_XO_REG_BYP_MASK, QUASAR_XO_REG_BYP_OFFSET, (value & QUASAR_XO_REG_BYP_MASK));
            break;
        case QUASAR_FIELD_XO_PLL_BUF_EN:
            SET_SPI_FIELD(pMac, QUASAR_REG_TEST_5, QUASAR_XO_PLL_BUF_EN_MASK, QUASAR_XO_PLL_BUF_EN_OFFSET, (value & QUASAR_XO_PLL_BUF_EN_MASK));
            break;
        case QUASAR_FIELD_ATEST:
            SET_SPI_FIELD(pMac, QUASAR_REG_TEST_5, QUASAR_ATEST_MASK, QUASAR_ATEST_OFFSET, (value & QUASAR_ATEST_MASK));
            break;
        case QUASAR_FIELD_ATEST_EN:
            SET_SPI_FIELD(pMac, QUASAR_REG_TEST_5, QUASAR_ATEST_EN_MASK, QUASAR_ATEST_EN_OFFSET, (value & QUASAR_ATEST_EN_MASK));
            break;
        case QUASAR_FIELD_BBF_GAIN2:
            SET_SPI_FIELD(pMac, QUASAR_REG_TEST_5, QUASAR_BBF_GAIN2_MASK, QUASAR_BBF_GAIN2_OFFSET, (value & QUASAR_BBF_GAIN2_MASK));
            break;
        case QUASAR_FIELD_BBF_GAIN1:
            SET_SPI_FIELD(pMac, QUASAR_REG_TEST_5, QUASAR_BBF_GAIN1_MASK, QUASAR_BBF_GAIN1_OFFSET, (value & QUASAR_BBF_GAIN1_MASK));
            break;
        case QUASAR_FIELD_BBF_GAIN_TEST:
            SET_SPI_FIELD(pMac, QUASAR_REG_TEST_5, QUASAR_BBF_GAIN_TEST_MASK, QUASAR_BBF_GAIN_TEST_OFFSET, (value & QUASAR_BBF_GAIN_TEST_MASK));
            break;
        case QUASAR_FIELD_TX1_DCOC_EN:
            SET_SPI_FIELD(pMac, QUASAR_REG_TEST_6, QUASAR_TX1_DCOC_EN_MASK, QUASAR_TX1_DCOC_EN_OFFSET, (value & QUASAR_TX1_DCOC_EN_MASK));
            break;
        case QUASAR_FIELD_TX0_DCOC_EN:
            SET_SPI_FIELD(pMac, QUASAR_REG_TEST_6, QUASAR_TX0_DCOC_EN_MASK, QUASAR_TX0_DCOC_EN_OFFSET, (value & QUASAR_TX0_DCOC_EN_MASK));
            break;
        case QUASAR_FIELD_TX0_DCOC_I:
            SET_SPI_FIELD(pMac, QUASAR_REG_TEST_6, QUASAR_TX0_DCOC_I_MASK, QUASAR_TX0_DCOC_I_OFFSET, (value & QUASAR_TX0_DCOC_I_MASK));
            break;
        case QUASAR_FIELD_TX0_DCOC_Q:
            SET_SPI_FIELD(pMac, QUASAR_REG_TEST_6, QUASAR_TX0_DCOC_Q_MASK, QUASAR_TX0_DCOC_Q_OFFSET, (value & QUASAR_TX0_DCOC_Q_MASK));
            break;
        case QUASAR_FIELD_ADC_TEST:
            SET_SPI_FIELD(pMac, QUASAR_REG_TEST_6, QUASAR_ADC_TEST_MASK, QUASAR_ADC_TEST_OFFSET, (value & QUASAR_ADC_TEST_MASK));
            break;
        case QUASAR_FIELD_ADC_TEST_EN:
            SET_SPI_FIELD(pMac, QUASAR_REG_TEST_6, QUASAR_ADC_TEST_EN_MASK, QUASAR_ADC_TEST_EN_OFFSET, (value & QUASAR_ADC_TEST_EN_MASK));
            break;
        case QUASAR_FIELD_RX_MAINT:
            SET_SPI_FIELD(pMac, QUASAR_REG_SPARES, QUASAR_RX_MAINT_MASK, QUASAR_RX_MAINT_OFFSET, (value & QUASAR_RX_MAINT_MASK));
            break;
        case QUASAR_FIELD_SPARES:
            SET_SPI_FIELD(pMac, QUASAR_REG_SPARES, QUASAR_SPARES_MASK, QUASAR_SPARES_OFFSET, (value & QUASAR_SPARES_MASK));
            break;
        case QUASAR_FIELD_TX1_DCOC_I:
            SET_SPI_FIELD(pMac, QUASAR_REG_SPARES, QUASAR_TX1_DCOC_I_MASK, QUASAR_TX1_DCOC_I_OFFSET, (value & QUASAR_TX1_DCOC_I_MASK));
            break;
        case QUASAR_FIELD_TX1_DCOC_Q:
            SET_SPI_FIELD(pMac, QUASAR_REG_SPARES, QUASAR_TX1_DCOC_Q_MASK, QUASAR_TX1_DCOC_Q_OFFSET, (value & QUASAR_TX1_DCOC_Q_MASK));
            break;
/*
            case QUASAR_FIELD_RESET:
                SET_SPI_FIELD(pMac, QUASAR_REG_RESET, QUASAR_RESET_MASK, QUASAR_RESET_OFFSET, (value & QUASAR_RESET_MASK));
                break;
            case QUASAR_FIELD_SPARES2:
                SET_SPI_FIELD(pMac, QUASAR_REG_SPARES2, QUASAR_SPARES2_MASK, QUASAR_SPARES2_OFFSET, (value & QUASAR_SPARES2_MASK));
                break;
*/
        default:
            assert(0);
            return (eHAL_STATUS_FAILURE);
            break;
    }
    return (retVal);
}


//spi register access
eHalStatus rfWriteDataField(tpAniSirGlobal pMac, tANI_U16 regNum, tANI_U32 dataMask, tANI_U32 dataShift, tANI_U32 data)
{
    eHalStatus retVal;
    tANI_U32 regData;

#ifdef ANI_PHY_DEBUG
    //for debug builds we will always get the current value from Quasar, so we stay in sync with external settings through python
    //GET_SPI_REG(pMac, QUASAR_CHIP, regNum, &regData);
    regData = pMac->hphy.rf.quasarRegCache[regNum];
#else
    //for production release builds, we will use our cached copy of what we wrote to Quasar because it saves access time.
    regData = pMac->hphy.rf.quasarRegCache[regNum];
#endif
    regData &= ~(dataMask << dataShift);
    regData |= ((data & dataMask) << dataShift);


    SET_SPI_REG(pMac, QUASAR_CHIP, regNum, regData);

    return (retVal);
}


eHalStatus rfReadQuasarField(tpAniSirGlobal pMac, eQuasarFields quasarField, tANI_U32 *value)
{
    eHalStatus retVal;

    switch (quasarField)
    {
        case QUASAR_PLL1:
        case QUASAR_PLL2:
        case QUASAR_PLL3:
        case QUASAR_PLL4:
        case QUASAR_PLL5:
        case QUASAR_PLL6:
        case QUASAR_LO_CONFIG:
        case QUASAR_STATE:
        case QUASAR_GAIN_CONTROL_1:
        case QUASAR_GAIN_CONTROL_2:
        case QUASAR_GAIN_CONTROL_3:
        case QUASAR_GAIN_CONTROL_4:
        case QUASAR_DC_OFFSET_CHAIN_0:
        case QUASAR_DC_OFFSET_CHAIN_1:
        case QUASAR_DC_OFFSET_CHAIN_2:
        case QUASAR_CFG_1:
        case QUASAR_CFG_2:
        case QUASAR_CFG_3:
        case QUASAR_CFG_4:
        case QUASAR_CFG_5:
        case QUASAR_CFG_6:
        case QUASAR_TEST_1:
        case QUASAR_TEST_2:
        case QUASAR_TEST_3:
        case QUASAR_TEST_4:
        case QUASAR_TEST_5:
        case QUASAR_TEST_6:
        case QUASAR_SPARES:
            GET_SPI_REG(pMac, QUASAR_CHIP, quasarField, value);
            break;


        case QUASAR_FIELD_NDIV_INT:
            GET_SPI_FIELD(pMac, QUASAR_REG_PLL1, QUASAR_NDIV_INT_MASK, QUASAR_NDIV_INT_OFFSET, value);
            break;
        case QUASAR_FIELD_NDIV_FRAC:
            {
                tANI_U32 lo, hi;
                GET_SPI_FIELD(pMac, QUASAR_REG_PLL1, QUASAR_NDIV_FRAC_HI_MASK, QUASAR_NDIV_FRAC_HI_OFFSET, &hi);
                hi <<= 18;  //PLL1 holds upper 2 bits of 20bit number
                GET_SPI_FIELD(pMac, QUASAR_REG_PLL2, QUASAR_NDIV_FRAC_LO_MASK, QUASAR_NDIV_FRAC_LO_OFFSET, &lo);
                *value = hi | lo;
            }
            break;
        case QUASAR_FIELD_CP_GAIN:
            GET_SPI_FIELD(pMac, QUASAR_REG_PLL1, QUASAR_CP_GAIN_MASK, QUASAR_CP_GAIN_OFFSET, value);
            break;
        case QUASAR_FIELD_NDIV_EN:
            GET_SPI_FIELD(pMac, QUASAR_REG_PLL1, QUASAR_NDIV_EN_MASK, QUASAR_NDIV_EN_OFFSET, value);
            break;
        case QUASAR_FIELD_PFD_EN:
            GET_SPI_FIELD(pMac, QUASAR_REG_PLL1, QUASAR_PFD_EN_MASK, QUASAR_PFD_EN_OFFSET, value);
            break;
        case QUASAR_FIELD_CP_EN:
            GET_SPI_FIELD(pMac, QUASAR_REG_PLL1, QUASAR_CP_EN_MASK, QUASAR_CP_EN_OFFSET, value);
            break;
        case QUASAR_FIELD_FNZ:
            GET_SPI_FIELD(pMac, QUASAR_REG_PLL1, QUASAR_FNZ_MASK, QUASAR_FNZ_OFFSET, value);
            break;
        case QUASAR_FIELD_PLL_CT_ACTIVE:
            GET_SPI_FIELD(pMac, QUASAR_REG_PLL3, QUASAR_PLL_CT_ACTIVE_MASK, QUASAR_PLL_CT_ACTIVE_OFFSET, value);
            break;
        case QUASAR_FIELD_VCO_EN:
            GET_SPI_FIELD(pMac, QUASAR_REG_PLL3, QUASAR_VCO_EN_MASK, QUASAR_VCO_EN_OFFSET, value);
            break;
        case QUASAR_FIELD_CT_EN:
            GET_SPI_FIELD(pMac, QUASAR_REG_PLL3, QUASAR_CT_EN_MASK, QUASAR_CT_EN_OFFSET, value);
            break;
        case QUASAR_FIELD_RDIV:
            GET_SPI_FIELD(pMac, QUASAR_REG_PLL3, QUASAR_RDIV_MASK, QUASAR_RDIV_OFFSET, value);
            break;
        case QUASAR_FIELD_EXT_RFLO_EN:
            GET_SPI_FIELD(pMac, QUASAR_REG_PLL3, QUASAR_EXT_RFLO_EN_MASK, QUASAR_EXT_RFLO_EN_OFFSET, value);
            break;
        case QUASAR_FIELD_RFLO_OUT_EN:
            GET_SPI_FIELD(pMac, QUASAR_REG_PLL3, QUASAR_RFLO_OUT_EN_MASK, QUASAR_RFLO_OUT_EN_OFFSET, value);
            break;
        case QUASAR_FIELD_PLL_CT_START:
            GET_SPI_FIELD(pMac, QUASAR_REG_PLL3, QUASAR_PLL_CT_START_MASK, QUASAR_PLL_CT_START_OFFSET, value);
            break;
        case QUASAR_FIELD_PLL_ADC_AQ:
            GET_SPI_FIELD(pMac, QUASAR_REG_PLL3, QUASAR_PLL_ADC_AQ_MASK, QUASAR_PLL_ADC_AQ_OFFSET, value);
            break;
        case QUASAR_FIELD_VTUNE_VAL:
            GET_SPI_FIELD(pMac, QUASAR_REG_PLL3, QUASAR_VTUNE_VAL_MASK, QUASAR_VTUNE_VAL_OFFSET, value);
            break;
        case QUASAR_FIELD_CT_MAINT:
            GET_SPI_FIELD(pMac, QUASAR_REG_PLL3, QUASAR_CT_MAINT_MASK, QUASAR_CT_MAINT_OFFSET, value);
            break;
        case QUASAR_FIELD_PLL_TUNE_ERR:
            GET_SPI_FIELD(pMac, QUASAR_REG_PLL4, QUASAR_PLL_TUNE_ERR_MASK, QUASAR_PLL_TUNE_ERR_OFFSET, value);
            break;
        case QUASAR_FIELD_PLL_VREG_D:
            GET_SPI_FIELD(pMac, QUASAR_REG_PLL4, QUASAR_PLL_VREG_D_MASK, QUASAR_PLL_VREG_D_OFFSET, value);
            break;
        case QUASAR_FIELD_PLL_VREG_EN:
            GET_SPI_FIELD(pMac, QUASAR_REG_PLL4, QUASAR_PLL_VREG_EN_MASK, QUASAR_PLL_VREG_EN_OFFSET, value);
            break;
        case QUASAR_FIELD_VCO_VREG_EN:
            GET_SPI_FIELD(pMac, QUASAR_REG_PLL4, QUASAR_VCO_VREG_EN_MASK, QUASAR_VCO_VREG_EN_OFFSET, value);
            break;
        case QUASAR_FIELD_VTUNE_RANGE:
            GET_SPI_FIELD(pMac, QUASAR_REG_PLL4, QUASAR_VTUNE_RANGE_MASK, QUASAR_VTUNE_RANGE_OFFSET, value);
            break;
        case QUASAR_FIELD_CT_VAL:
            GET_SPI_FIELD(pMac, QUASAR_REG_PLL4, QUASAR_CT_VAL_MASK, QUASAR_CT_VAL_OFFSET, value);
            break;
        case QUASAR_FIELD_VTUNE_T:
            GET_SPI_FIELD(pMac, QUASAR_REG_PLL4, QUASAR_VTUNE_T_MASK, QUASAR_VTUNE_T_OFFSET, value);
            break;
        case QUASAR_FIELD_VCO_SEL:
            GET_SPI_FIELD(pMac, QUASAR_REG_PLL5, QUASAR_VCO_SEL_MASK, QUASAR_VCO_SEL_OFFSET, value);
            break;
        case QUASAR_FIELD_CT_SETIME:
            GET_SPI_FIELD(pMac, QUASAR_REG_PLL5, QUASAR_CT_SETIME_MASK, QUASAR_CT_SETIME_OFFSET, value);
            break;
        case QUASAR_FIELD_THREE_FOURB:
            GET_SPI_FIELD(pMac, QUASAR_REG_PLL5, QUASAR_THREE_FOURB_MASK, QUASAR_THREE_FOURB_OFFSET, value);
            break;
        case QUASAR_FIELD_IBUF_BIAS:
            GET_SPI_FIELD(pMac, QUASAR_REG_PLL5, QUASAR_IBUF_BIAS_MASK, QUASAR_IBUF_BIAS_OFFSET, value);
            break;
        case QUASAR_FIELD_VCO_BIAS:
            GET_SPI_FIELD(pMac, QUASAR_REG_PLL5, QUASAR_VCO_BIAS_MASK, QUASAR_VCO_BIAS_OFFSET, value);
            break;
        case QUASAR_FIELD_CP_LOGIC:
            GET_SPI_FIELD(pMac, QUASAR_REG_PLL5, QUASAR_CP_LOGIC_MASK, QUASAR_CP_LOGIC_OFFSET, value);
            break;
        case QUASAR_FIELD_FRAC_OFFS:
            GET_SPI_FIELD(pMac, QUASAR_REG_PLL5, QUASAR_FRAC_OFFS_MASK, QUASAR_FRAC_OFFS_OFFSET, value);
            break;
        case QUASAR_FIELD_NDIV_BIAS:
            GET_SPI_FIELD(pMac, QUASAR_REG_PLL5, QUASAR_NDIV_BIAS_MASK, QUASAR_NDIV_BIAS_OFFSET, value);
            break;
        case QUASAR_FIELD_LBW_CAL_SU:
            GET_SPI_FIELD(pMac, QUASAR_REG_PLL6, QUASAR_LBW_CAL_SU_MASK, QUASAR_LBW_CAL_SU_OFFSET, value);
            break;
        case QUASAR_FIELD_CP_GAIN_CAL:
            GET_SPI_FIELD(pMac, QUASAR_REG_PLL6, QUASAR_CP_GAIN_CAL_MASK, QUASAR_CP_GAIN_CAL_OFFSET, value);
            break;
        case QUASAR_FIELD_DELTA_NDIV:
            GET_SPI_FIELD(pMac, QUASAR_REG_PLL6, QUASAR_DELTA_NDIV_MASK, QUASAR_DELTA_NDIV_OFFSET, value);
            break;
        case QUASAR_FIELD_PLL_TRANGE:
            GET_SPI_FIELD(pMac, QUASAR_REG_PLL6, QUASAR_PLL_TRANGE_MASK, QUASAR_PLL_TRANGE_OFFSET, value);
            break;
        case QUASAR_FIELD_PLL_TGOAL:
            GET_SPI_FIELD(pMac, QUASAR_REG_PLL6, QUASAR_PLL_TGOAL_MASK, QUASAR_PLL_TGOAL_OFFSET, value);
            break;
        case QUASAR_FIELD_LOOP_CAL_EN:
            GET_SPI_FIELD(pMac, QUASAR_REG_PLL6, QUASAR_LOOP_CAL_EN_MASK, QUASAR_LOOP_CAL_EN_OFFSET, value);
            break;
        case QUASAR_FIELD_EXT_IFLO_EN:
            GET_SPI_FIELD(pMac, QUASAR_REG_LO_CONFIG, QUASAR_EXT_IFLO_EN_MASK, QUASAR_EXT_IFLO_EN_OFFSET, value);
            break;
        case QUASAR_FIELD_IQ_DIV_EN:
            GET_SPI_FIELD(pMac, QUASAR_REG_LO_CONFIG, QUASAR_IQ_DIV_EN_MASK, QUASAR_IQ_DIV_EN_OFFSET, value);
            break;
        case QUASAR_FIELD_NDIV_LO:
            GET_SPI_FIELD(pMac, QUASAR_REG_LO_CONFIG, QUASAR_NDIV_LO_MASK, QUASAR_NDIV_LO_OFFSET, value);
            break;
        case QUASAR_FIELD_EXT_LO:
            GET_SPI_FIELD(pMac, QUASAR_REG_LO_CONFIG, QUASAR_EXT_LO_MASK, QUASAR_EXT_LO_OFFSET, value);
            break;
        case QUASAR_FIELD_TX1_LO:
            GET_SPI_FIELD(pMac, QUASAR_REG_LO_CONFIG, QUASAR_TX1_LO_MASK, QUASAR_TX1_LO_OFFSET, value);
            break;
        case QUASAR_FIELD_TX0_LO:
            GET_SPI_FIELD(pMac, QUASAR_REG_LO_CONFIG, QUASAR_TX0_LO_MASK, QUASAR_TX0_LO_OFFSET, value);
            break;
        case QUASAR_FIELD_RX2_LO:
            GET_SPI_FIELD(pMac, QUASAR_REG_LO_CONFIG, QUASAR_RX2_LO_MASK, QUASAR_RX2_LO_OFFSET, value);
            break;
        case QUASAR_FIELD_RX1_LO:
            GET_SPI_FIELD(pMac, QUASAR_REG_LO_CONFIG, QUASAR_RX1_LO_MASK, QUASAR_RX1_LO_OFFSET, value);
            break;
        case QUASAR_FIELD_RX0_LO:
            GET_SPI_FIELD(pMac, QUASAR_REG_LO_CONFIG, QUASAR_RX0_LO_MASK, QUASAR_RX0_LO_OFFSET, value);
            break;
        case QUASAR_FIELD_MS_REV:
            GET_SPI_FIELD(pMac, QUASAR_REG_STATE, QUASAR_MS_REV_MASK, QUASAR_MS_REV_OFFSET, value);
            break;
        case QUASAR_FIELD_XO_DISABLE_ST:
            GET_SPI_FIELD(pMac, QUASAR_REG_STATE, QUASAR_XO_DISABLE_ST_MASK, QUASAR_XO_DISABLE_ST_OFFSET, value);
            break;
        case QUASAR_FIELD_RX1_ST_EN:
            GET_SPI_FIELD(pMac, QUASAR_REG_STATE, QUASAR_RX1_ST_EN_MASK, QUASAR_RX1_ST_EN_OFFSET, value);
            break;
        case QUASAR_FIELD_RX_ALL_ST_EN:
            GET_SPI_FIELD(pMac, QUASAR_REG_STATE, QUASAR_RX_ALL_ST_EN_MASK, QUASAR_RX_ALL_ST_EN_OFFSET, value);
            break;
        case QUASAR_FIELD_TX_ST_EN:
            GET_SPI_FIELD(pMac, QUASAR_REG_STATE, QUASAR_TX_ST_EN_MASK, QUASAR_TX_ST_EN_OFFSET, value);
            break;
        case QUASAR_FIELD_PLL_EN:
            GET_SPI_FIELD(pMac, QUASAR_REG_STATE, QUASAR_PLL_EN_MASK, QUASAR_PLL_EN_OFFSET, value);
            break;
        case QUASAR_FIELD_TXRX_BAND:
            GET_SPI_FIELD(pMac, QUASAR_REG_STATE, QUASAR_TXRX_BAND_MASK, QUASAR_TXRX_BAND_OFFSET, value);
            break;
        case QUASAR_FIELD_MREV:
            GET_SPI_FIELD(pMac, QUASAR_REG_STATE, QUASAR_MREV_MASK, QUASAR_MREV_OFFSET, value);
            break;
        case QUASAR_FIELD_TWOG_HS_EN:
            GET_SPI_FIELD(pMac, QUASAR_REG_STATE, QUASAR_TWOG_HS_EN_MASK, QUASAR_TWOG_HS_EN_OFFSET, value);
            break;
        case QUASAR_FIELD_RXG_2:
            GET_SPI_FIELD(pMac, QUASAR_REG_GAIN_CONTROL_1, QUASAR_RXG_2_MASK, QUASAR_RXG_2_OFFSET, value);
            break;
        case QUASAR_FIELD_RXG_1:
            GET_SPI_FIELD(pMac, QUASAR_REG_GAIN_CONTROL_1, QUASAR_RXG_1_MASK, QUASAR_RXG_1_OFFSET, value);
            break;
        case QUASAR_FIELD_RXG_0:
            GET_SPI_FIELD(pMac, QUASAR_REG_GAIN_CONTROL_1, QUASAR_RXG_0_MASK, QUASAR_RXG_0_OFFSET, value);
            break;
        case QUASAR_FIELD_GC_SELECT:
            GET_SPI_FIELD(pMac, QUASAR_REG_GAIN_CONTROL_2, QUASAR_GC_SELECT_MASK, QUASAR_GC_SELECT_OFFSET, value);
            break;
        case QUASAR_FIELD_SPI_LUT_EN:
            GET_SPI_FIELD(pMac, QUASAR_REG_GAIN_CONTROL_2, QUASAR_SPI_LUT_EN_MASK, QUASAR_SPI_LUT_EN_OFFSET, value);
            break;
        case QUASAR_FIELD_SPI_LUT_INDEX:
            GET_SPI_FIELD(pMac, QUASAR_REG_GAIN_CONTROL_2, QUASAR_SPI_LUT_INDEX_MASK, QUASAR_SPI_LUT_INDEX_OFFSET, value);
            break;
        case QUASAR_FIELD_TX_GAIN_0:
            GET_SPI_FIELD(pMac, QUASAR_REG_GAIN_CONTROL_2, QUASAR_TX_GAIN_0_MASK, QUASAR_TX_GAIN_0_OFFSET, value);
            break;
        case QUASAR_FIELD_TX_GAIN_1:
            GET_SPI_FIELD(pMac, QUASAR_REG_GAIN_CONTROL_2, QUASAR_TX_GAIN_1_MASK, QUASAR_TX_GAIN_1_OFFSET, value);
            break;
        case QUASAR_FIELD_RX_GAIN_2:
            GET_SPI_FIELD(pMac, QUASAR_REG_GAIN_CONTROL_3, QUASAR_RX_GAIN_2_MASK, QUASAR_RX_GAIN_2_OFFSET, value);
            break;
        case QUASAR_FIELD_RX_GAIN_1:
            GET_SPI_FIELD(pMac, QUASAR_REG_GAIN_CONTROL_3, QUASAR_RX_GAIN_1_MASK, QUASAR_RX_GAIN_1_OFFSET, value);
            break;
        case QUASAR_FIELD_RX_GAIN_0:
            GET_SPI_FIELD(pMac, QUASAR_REG_GAIN_CONTROL_4, QUASAR_RX_GAIN_0_MASK, QUASAR_RX_GAIN_0_OFFSET, value);
            break;
        case QUASAR_FIELD_RX0DC_RANGE:
            GET_SPI_FIELD(pMac, QUASAR_REG_DC_OFFSET_CHAIN_0, QUASAR_RX0DC_RANGE_MASK, QUASAR_RX0DC_RANGE_OFFSET, value);
            break;
        case QUASAR_FIELD_RX0DCI:
            GET_SPI_FIELD(pMac, QUASAR_REG_DC_OFFSET_CHAIN_0, QUASAR_RX0DCI_MASK, QUASAR_RX0DCI_OFFSET, value);
            break;
        case QUASAR_FIELD_RX0DCQ:
            GET_SPI_FIELD(pMac, QUASAR_REG_DC_OFFSET_CHAIN_0, QUASAR_RX0DCQ_MASK, QUASAR_RX0DCQ_OFFSET, value);
            break;
        case QUASAR_FIELD_RX1DC_RANGE:
            GET_SPI_FIELD(pMac, QUASAR_REG_DC_OFFSET_CHAIN_1, QUASAR_RX1DC_RANGE_MASK, QUASAR_RX1DC_RANGE_OFFSET, value);
            break;
        case QUASAR_FIELD_RX1DCI:
            GET_SPI_FIELD(pMac, QUASAR_REG_DC_OFFSET_CHAIN_1, QUASAR_RX1DCI_MASK, QUASAR_RX1DCI_OFFSET, value);
            break;
        case QUASAR_FIELD_RX1DCQ:
            GET_SPI_FIELD(pMac, QUASAR_REG_DC_OFFSET_CHAIN_1, QUASAR_RX1DCQ_MASK, QUASAR_RX1DCQ_OFFSET, value);
            break;
        case QUASAR_FIELD_RX2DC_RANGE:
            GET_SPI_FIELD(pMac, QUASAR_REG_DC_OFFSET_CHAIN_2, QUASAR_RX2DC_RANGE_MASK, QUASAR_RX2DC_RANGE_OFFSET, value);
            break;
        case QUASAR_FIELD_RX2DCI:
            GET_SPI_FIELD(pMac, QUASAR_REG_DC_OFFSET_CHAIN_2, QUASAR_RX2DCI_MASK, QUASAR_RX2DCI_OFFSET, value);
            break;
        case QUASAR_FIELD_RX2DCQ:
            GET_SPI_FIELD(pMac, QUASAR_REG_DC_OFFSET_CHAIN_2, QUASAR_RX2DCQ_MASK, QUASAR_RX2DCQ_OFFSET, value);
            break;
        case QUASAR_FIELD_CLIP_EN_0:
            GET_SPI_FIELD(pMac, QUASAR_REG_CFG_1, QUASAR_CLIP_EN_0_MASK, QUASAR_CLIP_EN_0_OFFSET, value);
            break;
        case QUASAR_FIELD_CLIP_EN_1:
            GET_SPI_FIELD(pMac, QUASAR_REG_CFG_1, QUASAR_CLIP_EN_1_MASK, QUASAR_CLIP_EN_1_OFFSET, value);
            break;
        case QUASAR_FIELD_CLIP_EN_2:
            GET_SPI_FIELD(pMac, QUASAR_REG_CFG_1, QUASAR_CLIP_EN_2_MASK, QUASAR_CLIP_EN_2_OFFSET, value);
            break;
        case QUASAR_FIELD_MOD_INV:
            GET_SPI_FIELD(pMac, QUASAR_REG_CFG_1, QUASAR_MOD_INV_MASK, QUASAR_MOD_INV_OFFSET, value);
            break;
        case QUASAR_FIELD_TX_RFLO:
            GET_SPI_FIELD(pMac, QUASAR_REG_CFG_1, QUASAR_TX_RFLO_MASK, QUASAR_TX_RFLO_OFFSET, value);
            break;
        case QUASAR_FIELD_TX_IFLO:
            GET_SPI_FIELD(pMac, QUASAR_REG_CFG_1, QUASAR_TX_IFLO_MASK, QUASAR_TX_IFLO_OFFSET, value);
            break;
        case QUASAR_FIELD_XO_DRV:
            GET_SPI_FIELD(pMac, QUASAR_REG_CFG_1, QUASAR_XO_DRV_MASK, QUASAR_XO_DRV_OFFSET, value);
            break;
        case QUASAR_FIELD_XO_BYP:
            GET_SPI_FIELD(pMac, QUASAR_REG_CFG_1, QUASAR_XO_BYP_MASK, QUASAR_XO_BYP_OFFSET, value);
            break;
        case QUASAR_FIELD_XO_BUF_EN:
            GET_SPI_FIELD(pMac, QUASAR_REG_CFG_1, QUASAR_XO_BUF_EN_MASK, QUASAR_XO_BUF_EN_OFFSET, value);
            break;
        case QUASAR_FIELD_TOUT:
            GET_SPI_FIELD(pMac, QUASAR_REG_CFG_1, QUASAR_TOUT_MASK, QUASAR_TOUT_OFFSET, value);
            break;
        case QUASAR_FIELD_TEN:
            GET_SPI_FIELD(pMac, QUASAR_REG_CFG_1, QUASAR_TEN_MASK, QUASAR_TEN_OFFSET, value);
            break;
        case QUASAR_FIELD_RC_ATUNE_VAL:
            GET_SPI_FIELD(pMac, QUASAR_REG_CFG_2, QUASAR_RC_ATUNE_VAL_MASK, QUASAR_RC_ATUNE_VAL_OFFSET, value);
            break;
        case QUASAR_FIELD_RC_TUNE_EN:
            GET_SPI_FIELD(pMac, QUASAR_REG_CFG_2, QUASAR_RC_TUNE_EN_MASK, QUASAR_RC_TUNE_EN_OFFSET, value);
            break;
        case QUASAR_FIELD_BBF_RX_TUNE:
            GET_SPI_FIELD(pMac, QUASAR_REG_CFG_2, QUASAR_BBF_RX_TUNE_MASK, QUASAR_BBF_RX_TUNE_OFFSET, value);
            break;
        case QUASAR_FIELD_BBF_TX_TUNE:
            GET_SPI_FIELD(pMac, QUASAR_REG_CFG_2, QUASAR_BBF_TX_TUNE_MASK, QUASAR_BBF_TX_TUNE_OFFSET, value);
            break;
        case QUASAR_FIELD_BBF_ATUNE_EN:
            GET_SPI_FIELD(pMac, QUASAR_REG_CFG_2, QUASAR_BBF_ATUNE_EN_MASK, QUASAR_BBF_ATUNE_EN_OFFSET, value);
            break;
        case QUASAR_FIELD_RX2_DCOC_EN:
            GET_SPI_FIELD(pMac, QUASAR_REG_CFG_3, QUASAR_RX2_DCOC_EN_MASK, QUASAR_RX2_DCOC_EN_OFFSET, value);
            break;
        case QUASAR_FIELD_RX1_DCOC_EN:
            GET_SPI_FIELD(pMac, QUASAR_REG_CFG_3, QUASAR_RX1_DCOC_EN_MASK, QUASAR_RX1_DCOC_EN_OFFSET, value);
            break;
        case QUASAR_FIELD_RX0_DCOC_EN:
            GET_SPI_FIELD(pMac, QUASAR_REG_CFG_3, QUASAR_RX0_DCOC_EN_MASK, QUASAR_RX0_DCOC_EN_OFFSET, value);
            break;
        case QUASAR_FIELD_TX1_D:
            GET_SPI_FIELD(pMac, QUASAR_REG_CFG_3, QUASAR_TX1_D_MASK, QUASAR_TX1_D_OFFSET, value);
            break;
        case QUASAR_FIELD_TX1D_D:
            GET_SPI_FIELD(pMac, QUASAR_REG_CFG_3, QUASAR_TX1D_D_MASK, QUASAR_TX1D_D_OFFSET, value);
            break;
        case QUASAR_FIELD_RX1_D:
            GET_SPI_FIELD(pMac, QUASAR_REG_CFG_3, QUASAR_RX1_D_MASK, QUASAR_RX1_D_OFFSET, value);
            break;
        case QUASAR_FIELD_CLK_GATE:
            GET_SPI_FIELD(pMac, QUASAR_REG_CFG_4, QUASAR_CLK_GATE_MASK, QUASAR_CLK_GATE_OFFSET, value);
            break;
        case QUASAR_FIELD_TX_DBIAS:
            GET_SPI_FIELD(pMac, QUASAR_REG_CFG_4, QUASAR_TX_DBIAS_MASK, QUASAR_TX_DBIAS_OFFSET, value);
            break;
        case QUASAR_FIELD_TX0_D:
            GET_SPI_FIELD(pMac, QUASAR_REG_CFG_4, QUASAR_TX0_D_MASK, QUASAR_TX0_D_OFFSET, value);
            break;
        case QUASAR_FIELD_TX0D_D:
            GET_SPI_FIELD(pMac, QUASAR_REG_CFG_4, QUASAR_TX0D_D_MASK, QUASAR_TX0D_D_OFFSET, value);
            break;
        case QUASAR_FIELD_RX0_D:
            GET_SPI_FIELD(pMac, QUASAR_REG_CFG_4, QUASAR_RX0_D_MASK, QUASAR_RX0_D_OFFSET, value);
            break;
        case QUASAR_FIELD_RX_IBIAS_EN:
            GET_SPI_FIELD(pMac, QUASAR_REG_CFG_5, QUASAR_RX_IBIAS_EN_MASK, QUASAR_RX_IBIAS_EN_OFFSET, value);
            break;
        case QUASAR_FIELD_RX_RF_MIX_EN:
            GET_SPI_FIELD(pMac, QUASAR_REG_CFG_5, QUASAR_RX_RF_MIX_EN_MASK, QUASAR_RX_RF_MIX_EN_OFFSET, value);
            break;
        case QUASAR_FIELD_RX_IF_MIX_EN:
            GET_SPI_FIELD(pMac, QUASAR_REG_CFG_5, QUASAR_RX_IF_MIX_EN_MASK, QUASAR_RX_IF_MIX_EN_OFFSET, value);
            break;
        case QUASAR_FIELD_RX_IF_TUN:
            GET_SPI_FIELD(pMac, QUASAR_REG_CFG_5, QUASAR_RX_IF_TUN_MASK, QUASAR_RX_IF_TUN_OFFSET, value);
            break;
        case QUASAR_FIELD_TX_RF_TUN:
            GET_SPI_FIELD(pMac, QUASAR_REG_CFG_5, QUASAR_TX_RF_TUN_MASK, QUASAR_TX_RF_TUN_OFFSET, value);
            break;
        case QUASAR_FIELD_TX_IF_TUN:
            GET_SPI_FIELD(pMac, QUASAR_REG_CFG_5, QUASAR_TX_IF_TUN_MASK, QUASAR_TX_IF_TUN_OFFSET, value);
            break;
        case QUASAR_FIELD_RX2_D:
            GET_SPI_FIELD(pMac, QUASAR_REG_CFG_5, QUASAR_RX2_D_MASK, QUASAR_RX2_D_OFFSET, value);
            break;
        case QUASAR_FIELD_RX_CASC_GC:
            GET_SPI_FIELD(pMac, QUASAR_REG_CFG_6, QUASAR_RX_CASC_GC_MASK, QUASAR_RX_CASC_GC_OFFSET, value);
            break;
        case QUASAR_FIELD_TX_CASC_GC:
            GET_SPI_FIELD(pMac, QUASAR_REG_CFG_6, QUASAR_TX_CASC_GC_MASK, QUASAR_TX_CASC_GC_OFFSET, value);
            break;
        case QUASAR_FIELD_RX_IF_LODRV_EN:
            GET_SPI_FIELD(pMac, QUASAR_REG_CFG_6, QUASAR_RX_IF_LODRV_EN_MASK, QUASAR_RX_IF_LODRV_EN_OFFSET, value);
            break;
        case QUASAR_FIELD_XO_GM:
            GET_SPI_FIELD(pMac, QUASAR_REG_CFG_6, QUASAR_XO_GM_MASK, QUASAR_XO_GM_OFFSET, value);
            break;
        case QUASAR_FIELD_CLIP_TEST:
            GET_SPI_FIELD(pMac, QUASAR_REG_CFG_6, QUASAR_CLIP_TEST_MASK, QUASAR_CLIP_TEST_OFFSET, value);
            break;
        case QUASAR_FIELD_CLIP_FREQ:
            GET_SPI_FIELD(pMac, QUASAR_REG_CFG_6, QUASAR_CLIP_FREQ_MASK, QUASAR_CLIP_FREQ_OFFSET, value);
            break;
        case QUASAR_FIELD_CLIP_DET_TH:
            GET_SPI_FIELD(pMac, QUASAR_REG_CFG_6, QUASAR_CLIP_DET_TH_MASK, QUASAR_CLIP_DET_TH_OFFSET, value);
            break;
        case QUASAR_FIELD_BBF_BIAS_MODE:
            GET_SPI_FIELD(pMac, QUASAR_REG_CFG_6, QUASAR_BBF_BIAS_MODE_MASK, QUASAR_BBF_BIAS_MODE_OFFSET, value);
            break;
        case QUASAR_FIELD_TX_REG_MODE:
            GET_SPI_FIELD(pMac, QUASAR_REG_CFG_6, QUASAR_TX_REG_MODE_MASK, QUASAR_TX_REG_MODE_OFFSET, value);
            break;
        case QUASAR_FIELD_TX_VREG_BYP:
            GET_SPI_FIELD(pMac, QUASAR_REG_CFG_6, QUASAR_TX_VREG_BYP_MASK, QUASAR_TX_VREG_BYP_OFFSET, value);
            break;
        case QUASAR_FIELD_NDIV_OUT:
            GET_SPI_FIELD(pMac, QUASAR_REG_TEST_1, QUASAR_NDIV_OUT_MASK, QUASAR_NDIV_OUT_OFFSET, value);
            break;
        case QUASAR_FIELD_RX_LB_GAIN:
            GET_SPI_FIELD(pMac, QUASAR_REG_TEST_1, QUASAR_RX_LB_GAIN_MASK, QUASAR_RX_LB_GAIN_OFFSET, value);
            break;
        case QUASAR_FIELD_TX_LB_GAIN:
            GET_SPI_FIELD(pMac, QUASAR_REG_TEST_1, QUASAR_TX_LB_GAIN_MASK, QUASAR_TX_LB_GAIN_OFFSET, value);
            break;
        case QUASAR_FIELD_TEST_LB_RF:
            GET_SPI_FIELD(pMac, QUASAR_REG_TEST_1, QUASAR_TEST_LB_RF_MASK, QUASAR_TEST_LB_RF_OFFSET, value);
            break;
        case QUASAR_FIELD_RX_IQ_TEST_0:
            GET_SPI_FIELD(pMac, QUASAR_REG_TEST_1, QUASAR_RX_IQ_TEST_0_MASK, QUASAR_RX_IQ_TEST_0_OFFSET, value);
            break;
        case QUASAR_FIELD_RX_IQ_TEST_1:
            GET_SPI_FIELD(pMac, QUASAR_REG_TEST_1, QUASAR_RX_IQ_TEST_1_MASK, QUASAR_RX_IQ_TEST_1_OFFSET, value);
            break;
        case QUASAR_FIELD_RX_IQ_TEST_2_1:
            GET_SPI_FIELD(pMac, QUASAR_REG_TEST_1, QUASAR_RX_IQ_TEST_2_1_MASK, QUASAR_RX_IQ_TEST_2_1_OFFSET, value);
            break;
        case QUASAR_FIELD_RX_IQ_TEST_2_0:
            GET_SPI_FIELD(pMac, QUASAR_REG_TEST_1, QUASAR_RX_IQ_TEST_2_0_MASK, QUASAR_RX_IQ_TEST_2_0_OFFSET, value);
            break;
        case QUASAR_FIELD_TX_CAL_EN_0:
            GET_SPI_FIELD(pMac, QUASAR_REG_TEST_1, QUASAR_TX_CAL_EN_0_MASK, QUASAR_TX_CAL_EN_0_OFFSET, value);
            break;
        case QUASAR_FIELD_TX_CAL_EN_1:
            GET_SPI_FIELD(pMac, QUASAR_REG_TEST_1, QUASAR_TX_CAL_EN_1_MASK, QUASAR_TX_CAL_EN_1_OFFSET, value);
            break;
        case QUASAR_FIELD_DET_OFFSET:
            GET_SPI_FIELD(pMac, QUASAR_REG_TEST_1, QUASAR_DET_OFFSET_MASK, QUASAR_DET_OFFSET_OFFSET, value);
            break;
        case QUASAR_FIELD_DET_GAIN:
            GET_SPI_FIELD(pMac, QUASAR_REG_TEST_1, QUASAR_DET_GAIN_MASK, QUASAR_DET_GAIN_OFFSET, value);
            break;
        case QUASAR_FIELD_RDIV_TEST:
            GET_SPI_FIELD(pMac, QUASAR_REG_TEST_1, QUASAR_RDIV_TEST_MASK, QUASAR_RDIV_TEST_OFFSET, value);
            break;
        case QUASAR_FIELD_TX_DRVR_GAIN:
            GET_SPI_FIELD(pMac, QUASAR_REG_TEST_2, QUASAR_TX_DRVR_GAIN_MASK, QUASAR_TX_DRVR_GAIN_OFFSET, value);
            break;
        case QUASAR_FIELD_TX_VGA_GAIN:
            GET_SPI_FIELD(pMac, QUASAR_REG_TEST_2, QUASAR_TX_VGA_GAIN_MASK, QUASAR_TX_VGA_GAIN_OFFSET, value);
            break;
        case QUASAR_FIELD_TX_IQ_GAIN:
            GET_SPI_FIELD(pMac, QUASAR_REG_TEST_2, QUASAR_TX_IQ_GAIN_MASK, QUASAR_TX_IQ_GAIN_OFFSET, value);
            break;
        case QUASAR_FIELD_TX_GTEST:
            GET_SPI_FIELD(pMac, QUASAR_REG_TEST_2, QUASAR_TX_GTEST_MASK, QUASAR_TX_GTEST_OFFSET, value);
            break;
        case QUASAR_FIELD_BBF_IBIAS_CTL:
            GET_SPI_FIELD(pMac, QUASAR_REG_TEST_2, QUASAR_BBF_IBIAS_CTL_MASK, QUASAR_BBF_IBIAS_CTL_OFFSET, value);
            break;
        case QUASAR_FIELD_BBF_TEST_I:
            GET_SPI_FIELD(pMac, QUASAR_REG_TEST_3, QUASAR_BBF_TEST_I_MASK, QUASAR_BBF_TEST_I_OFFSET, value);
            break;
        case QUASAR_FIELD_BBF_TEST_Q:
            GET_SPI_FIELD(pMac, QUASAR_REG_TEST_3, QUASAR_BBF_TEST_Q_MASK, QUASAR_BBF_TEST_Q_OFFSET, value);
            break;
        case QUASAR_FIELD_CT_VAL_AUTO:
            GET_SPI_FIELD(pMac, QUASAR_REG_TEST_3, QUASAR_CT_VAL_AUTO_MASK, QUASAR_CT_VAL_AUTO_OFFSET, value);
            break;
        case QUASAR_FIELD_IO_TEST:
            GET_SPI_FIELD(pMac, QUASAR_REG_TEST_4, QUASAR_IO_TEST_MASK, QUASAR_IO_TEST_OFFSET, value);
            break;
        case QUASAR_FIELD_TX_RF_EN:
            GET_SPI_FIELD(pMac, QUASAR_REG_TEST_4, QUASAR_TX_RF_EN_MASK, QUASAR_TX_RF_EN_OFFSET, value);
            break;
        case QUASAR_FIELD_TX_DRVR_EN:
            GET_SPI_FIELD(pMac, QUASAR_REG_TEST_4, QUASAR_TX_DRVR_EN_MASK, QUASAR_TX_DRVR_EN_OFFSET, value);
            break;
        case QUASAR_FIELD_TX_IQ_EN:
            GET_SPI_FIELD(pMac, QUASAR_REG_TEST_4, QUASAR_TX_IQ_EN_MASK, QUASAR_TX_IQ_EN_OFFSET, value);
            break;
        case QUASAR_FIELD_BBF_ENQ:
            GET_SPI_FIELD(pMac, QUASAR_REG_TEST_4, QUASAR_BBF_ENQ_MASK, QUASAR_BBF_ENQ_OFFSET, value);
            break;
        case QUASAR_FIELD_BBF_ENI:
            GET_SPI_FIELD(pMac, QUASAR_REG_TEST_4, QUASAR_BBF_ENI_MASK, QUASAR_BBF_ENI_OFFSET, value);
            break;
        case QUASAR_FIELD_RX_GMTUNE_EN:
            GET_SPI_FIELD(pMac, QUASAR_REG_TEST_5, QUASAR_RX_GMTUNE_EN_MASK, QUASAR_RX_GMTUNE_EN_OFFSET, value);
            break;
        case QUASAR_FIELD_XO_REG_BYP:
            GET_SPI_FIELD(pMac, QUASAR_REG_TEST_5, QUASAR_XO_REG_BYP_MASK, QUASAR_XO_REG_BYP_OFFSET, value);
            break;
        case QUASAR_FIELD_XO_PLL_BUF_EN:
            GET_SPI_FIELD(pMac, QUASAR_REG_TEST_5, QUASAR_XO_PLL_BUF_EN_MASK, QUASAR_XO_PLL_BUF_EN_OFFSET, value);
            break;
        case QUASAR_FIELD_ATEST:
            GET_SPI_FIELD(pMac, QUASAR_REG_TEST_5, QUASAR_ATEST_MASK, QUASAR_ATEST_OFFSET, value);
            break;
        case QUASAR_FIELD_ATEST_EN:
            GET_SPI_FIELD(pMac, QUASAR_REG_TEST_5, QUASAR_ATEST_EN_MASK, QUASAR_ATEST_EN_OFFSET, value);
            break;
        case QUASAR_FIELD_BBF_GAIN2:
            GET_SPI_FIELD(pMac, QUASAR_REG_TEST_5, QUASAR_BBF_GAIN2_MASK, QUASAR_BBF_GAIN2_OFFSET, value);
            break;
        case QUASAR_FIELD_BBF_GAIN1:
            GET_SPI_FIELD(pMac, QUASAR_REG_TEST_5, QUASAR_BBF_GAIN1_MASK, QUASAR_BBF_GAIN1_OFFSET, value);
            break;
        case QUASAR_FIELD_BBF_GAIN_TEST:
            GET_SPI_FIELD(pMac, QUASAR_REG_TEST_5, QUASAR_BBF_GAIN_TEST_MASK, QUASAR_BBF_GAIN_TEST_OFFSET, value);
            break;
        case QUASAR_FIELD_TX1_DCOC_EN:
            GET_SPI_FIELD(pMac, QUASAR_REG_TEST_6, QUASAR_TX1_DCOC_EN_MASK, QUASAR_TX1_DCOC_EN_OFFSET, value);
            break;
        case QUASAR_FIELD_TX0_DCOC_EN:
            GET_SPI_FIELD(pMac, QUASAR_REG_TEST_6, QUASAR_TX0_DCOC_EN_MASK, QUASAR_TX0_DCOC_EN_OFFSET, value);
            break;
        case QUASAR_FIELD_TX0_DCOC_I:
            GET_SPI_FIELD(pMac, QUASAR_REG_TEST_6, QUASAR_TX0_DCOC_I_MASK, QUASAR_TX0_DCOC_I_OFFSET, value);
            break;
        case QUASAR_FIELD_TX0_DCOC_Q:
            GET_SPI_FIELD(pMac, QUASAR_REG_TEST_6, QUASAR_TX0_DCOC_Q_MASK, QUASAR_TX0_DCOC_Q_OFFSET, value);
            break;
        case QUASAR_FIELD_ADC_TEST:
            GET_SPI_FIELD(pMac, QUASAR_REG_TEST_6, QUASAR_ADC_TEST_MASK, QUASAR_ADC_TEST_OFFSET, value);
            break;
        case QUASAR_FIELD_ADC_TEST_EN:
            GET_SPI_FIELD(pMac, QUASAR_REG_TEST_6, QUASAR_ADC_TEST_EN_MASK, QUASAR_ADC_TEST_EN_OFFSET, value);
            break;
        case QUASAR_FIELD_RX_MAINT:
            GET_SPI_FIELD(pMac, QUASAR_REG_SPARES, QUASAR_RX_MAINT_MASK, QUASAR_RX_MAINT_OFFSET, value);
            break;
        case QUASAR_FIELD_SPARES:
            GET_SPI_FIELD(pMac, QUASAR_REG_SPARES, QUASAR_SPARES_MASK, QUASAR_SPARES_OFFSET, value);
            break;
        case QUASAR_FIELD_TX1_DCOC_I:
            GET_SPI_FIELD(pMac, QUASAR_REG_SPARES, QUASAR_TX1_DCOC_I_MASK, QUASAR_TX1_DCOC_I_OFFSET, value);
            break;
        case QUASAR_FIELD_TX1_DCOC_Q:
            GET_SPI_FIELD(pMac, QUASAR_REG_SPARES, QUASAR_TX1_DCOC_Q_MASK, QUASAR_TX1_DCOC_Q_OFFSET, value);
            break;
        default:
            assert(0);
            return (eHAL_STATUS_FAILURE);
            break;
    }
    return (retVal);
}


eHalStatus rfReadDataField(tpAniSirGlobal pMac, tANI_U16 regNum, tANI_U32 dataMask, tANI_U32 dataShift, tANI_U32 *pData)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;

    assert(pMac != 0);

    switch (regNum)
    {

        case QUASAR_REG_PLL3:
            switch(dataShift)
            {
                case QUASAR_PLL_CT_ACTIVE_OFFSET:
                    GET_SPI_REG(pMac, QUASAR_CHIP, regNum, pData);
                    pMac->hphy.rf.quasarRegCache[regNum] = *pData;
                    break;
                default:
                    *pData = pMac->hphy.rf.quasarRegCache[regNum];
                    break;
            }
            break;
        case QUASAR_REG_PLL4:
            switch(dataShift)
            {
                case QUASAR_PLL_TUNE_ERR_OFFSET:
                case QUASAR_PLL_VREG_D_OFFSET:
                    GET_SPI_REG(pMac, QUASAR_CHIP, regNum, pData);
                    pMac->hphy.rf.quasarRegCache[regNum] = *pData;
                    break;
                default:
                    *pData = pMac->hphy.rf.quasarRegCache[regNum];
                    break;
            }
            break;
        case QUASAR_REG_STATE:
            switch(dataShift)
            {
                case QUASAR_STATE_MREV_OFFSET:
                case QUASAR_STATE_MS_REV_OFFSET:
                    GET_SPI_REG(pMac, QUASAR_CHIP, regNum, pData);
                    pMac->hphy.rf.quasarRegCache[regNum] = *pData;
                    break;
                default:
                    *pData = pMac->hphy.rf.quasarRegCache[regNum];
                    break;
            }
            break;
        case QUASAR_REG_CFG_1:
            switch(dataShift)
            {
                case QUASAR_CFG1_TEN_OFFSET:
                case QUASAR_CFG1_TOUT_OFFSET:
                    GET_SPI_REG(pMac, QUASAR_CHIP, regNum, pData);
                    pMac->hphy.rf.quasarRegCache[regNum] = *pData;
                    break;
                default:
                    *pData = pMac->hphy.rf.quasarRegCache[regNum];
                    break;
            }
            break;
        case QUASAR_REG_GAIN_CONTROL_1:
            switch(dataShift)
            {
                case QUASAR_GC1_RXG_0_OFFSET:
                case QUASAR_GC1_RXG_1_OFFSET:
                case QUASAR_GC1_RXG_2_OFFSET:
                    GET_SPI_REG(pMac, QUASAR_CHIP, regNum, pData);
                    pMac->hphy.rf.quasarRegCache[regNum] = *pData;
                    break;
                default:
                    *pData = pMac->hphy.rf.quasarRegCache[regNum];
                    break;
            }
            break;
        case QUASAR_REG_GAIN_CONTROL_2:
            switch(dataShift)
            {
                case QUASAR_GC2_TX_GAIN_0_OFFSET:
                case QUASAR_GC2_TX_GAIN_1_OFFSET:
                    GET_SPI_REG(pMac, QUASAR_CHIP, regNum, pData);
                    pMac->hphy.rf.quasarRegCache[regNum] = *pData;
                    break;
                default:
                    *pData = pMac->hphy.rf.quasarRegCache[regNum];
                    break;
            }
            break;
        case QUASAR_REG_GAIN_CONTROL_3:
            switch(dataShift)
            {
                case QUASAR_GC3_RX_GAIN_1_OFFSET:
                case QUASAR_GC3_RX_GAIN_2_OFFSET:
                    GET_SPI_REG(pMac, QUASAR_CHIP, regNum, pData);
                    pMac->hphy.rf.quasarRegCache[regNum] = *pData;
                    break;
                default:
                    *pData = pMac->hphy.rf.quasarRegCache[regNum];
                    break;
            }
            break;
        case QUASAR_REG_GAIN_CONTROL_4:
            switch(dataShift)
            {
                case QUASAR_GC4_RX_GAIN_0_OFFSET:
                    GET_SPI_REG(pMac, QUASAR_CHIP, regNum, pData);
                    pMac->hphy.rf.quasarRegCache[regNum] = *pData;
                    break;
                default:
                    *pData = pMac->hphy.rf.quasarRegCache[regNum];
                    break;
            }
            break;
            
        case QUASAR_REG_PLL1:
        case QUASAR_REG_PLL2:
        case QUASAR_REG_PLL5:
        case QUASAR_REG_PLL6:
        case QUASAR_REG_LO_CONFIG:
        case QUASAR_REG_DC_OFFSET_CHAIN_0:
        case QUASAR_REG_DC_OFFSET_CHAIN_1:
        case QUASAR_REG_DC_OFFSET_CHAIN_2:
        case QUASAR_REG_CFG_2:
        case QUASAR_REG_CFG_3:
        case QUASAR_REG_CFG_4:
        case QUASAR_REG_CFG_5:
        case QUASAR_REG_CFG_6:
        case QUASAR_REG_TEST_1:
        case QUASAR_REG_TEST_2:
        case QUASAR_REG_TEST_3:
        case QUASAR_REG_TEST_4:
        case QUASAR_REG_TEST_5:
        case QUASAR_REG_TEST_6:
        case QUASAR_REG_SPARES:
            *pData = pMac->hphy.rf.quasarRegCache[regNum];
            break;
        default:
            assert(0);
            return (eHAL_STATUS_FAILURE);
    }

    *pData = (*pData & (dataMask << dataShift)) >> dataShift;
    return (retVal);
}



//initialization, enable, and chain selection
eHalStatus rfConfig(tpAniSirGlobal pMac)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
    
    pMac->hphy.rf.curChannel = INVALID_RF_CHANNEL;  //init to no channel set
    
    halGetEepromTableLoc(pMac, EEPROM_TABLE_QUASAR_FILTERS, (uEepromTables **)&pMac->hphy.rf.chanFilterSettings);

    if ((retVal = asicSpiInit(pMac)) == eHAL_STATUS_SUCCESS)
    {
        if ((retVal = rfGetVersion(pMac, QUASAR_CHIP, &pMac->hphy.rf.rfChip0)) == eHAL_STATUS_SUCCESS)
        {

            phyLog(pMac, LOGE, "Quasar chip versions -> RF0: ver=%X mask=%X", pMac->hphy.rf.rfChip0.ver, pMac->hphy.rf.rfChip0.subVer);

            //don't check the return value - the quasarRegsTable pointer will be NULL if the table is not found
            halGetEepromTableLoc(pMac, EEPROM_TABLE_QUASAR_REGS, (uEepromTables **)&pMac->hphy.rf.quasarRegsTable);
            //pMac->hphy.rf.quasarRegsTable = NULL;
        }
    }
    return (retVal);
}


eHalStatus rfInit(tpAniSirGlobal pMac)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
/*
        tANI_U32 quasarDefaults[QUASAR_NUM_REGS];
    
        GET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_PLL1             , &quasarDefaults[QUASAR_REG_PLL1]             );
        GET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_PLL2             , &quasarDefaults[QUASAR_REG_PLL2]             );
        GET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_PLL3             , &quasarDefaults[QUASAR_REG_PLL3]             );
        GET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_PLL4             , &quasarDefaults[QUASAR_REG_PLL4]             );
        GET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_PLL5             , &quasarDefaults[QUASAR_REG_PLL5]             );
        GET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_PLL6             , &quasarDefaults[QUASAR_REG_PLL6]             );
        GET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_LO_CONFIG        , &quasarDefaults[QUASAR_REG_LO_CONFIG]        );
        GET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_STATE            , &quasarDefaults[QUASAR_REG_STATE]            );
        GET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_GAIN_CONTROL_1   , &quasarDefaults[QUASAR_REG_GAIN_CONTROL_1]   );
        GET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_GAIN_CONTROL_2   , &quasarDefaults[QUASAR_REG_GAIN_CONTROL_2]   );
        GET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_GAIN_CONTROL_3   , &quasarDefaults[QUASAR_REG_GAIN_CONTROL_3]   );
        GET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_GAIN_CONTROL_4   , &quasarDefaults[QUASAR_REG_GAIN_CONTROL_4]   );
        GET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_DC_OFFSET_CHAIN_0, &quasarDefaults[QUASAR_REG_DC_OFFSET_CHAIN_0]);
        GET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_DC_OFFSET_CHAIN_1, &quasarDefaults[QUASAR_REG_DC_OFFSET_CHAIN_1]);
        GET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_DC_OFFSET_CHAIN_2, &quasarDefaults[QUASAR_REG_DC_OFFSET_CHAIN_2]);
        GET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_CFG_1            , &quasarDefaults[QUASAR_REG_CFG_1]            );
        GET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_CFG_2            , &quasarDefaults[QUASAR_REG_CFG_2]            );
        GET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_CFG_3            , &quasarDefaults[QUASAR_REG_CFG_3]            );
        GET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_CFG_4            , &quasarDefaults[QUASAR_REG_CFG_4]            );
        GET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_CFG_5            , &quasarDefaults[QUASAR_REG_CFG_5]            );
        GET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_CFG_6            , &quasarDefaults[QUASAR_REG_CFG_6]            );
        GET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_TEST_1           , &quasarDefaults[QUASAR_REG_TEST_1]           );
        GET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_TEST_2           , &quasarDefaults[QUASAR_REG_TEST_2]           );
        GET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_TEST_3           , &quasarDefaults[QUASAR_REG_TEST_3]           );
        GET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_TEST_4           , &quasarDefaults[QUASAR_REG_TEST_4]           );
        GET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_TEST_5           , &quasarDefaults[QUASAR_REG_TEST_5]           );
        GET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_TEST_6           , &quasarDefaults[QUASAR_REG_TEST_6]           );
        GET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_SPARES           , &quasarDefaults[QUASAR_REG_SPARES]           );
*/

#ifdef ANI_PHY_DEBUG
/*
    //check chip defaults against quasar.h defaults to catch discrpancies
    if (QUASAR_PLL1_DEFAULT      != quasarDefaults[QUASAR_REG_PLL1]             ) { phyLog(pMac, LOGW, "ERROR: Quasar reg %d is supposed to have a default of %08X, but instead has %08X\n", QUASAR_REG_PLL1             , QUASAR_PLL1_DEFAULT     , quasarDefaults[QUASAR_REG_PLL1             ]); }
    if (QUASAR_PLL2_DEFAULT      != quasarDefaults[QUASAR_REG_PLL2]             ) { phyLog(pMac, LOGW, "ERROR: Quasar reg %d is supposed to have a default of %08X, but instead has %08X\n", QUASAR_REG_PLL2             , QUASAR_PLL2_DEFAULT     , quasarDefaults[QUASAR_REG_PLL2             ]); }
    if (QUASAR_PLL3_DEFAULT      != quasarDefaults[QUASAR_REG_PLL3]             ) { phyLog(pMac, LOGW, "ERROR: Quasar reg %d is supposed to have a default of %08X, but instead has %08X\n", QUASAR_REG_PLL3             , QUASAR_PLL3_DEFAULT     , quasarDefaults[QUASAR_REG_PLL3             ]); }
    if (QUASAR_PLL4_DEFAULT      != quasarDefaults[QUASAR_REG_PLL4]             ) { phyLog(pMac, LOGW, "ERROR: Quasar reg %d is supposed to have a default of %08X, but instead has %08X\n", QUASAR_REG_PLL4             , QUASAR_PLL4_DEFAULT     , quasarDefaults[QUASAR_REG_PLL4             ]); }
    if (QUASAR_PLL5_DEFAULT      != quasarDefaults[QUASAR_REG_PLL5]             ) { phyLog(pMac, LOGW, "ERROR: Quasar reg %d is supposed to have a default of %08X, but instead has %08X\n", QUASAR_REG_PLL5             , QUASAR_PLL5_DEFAULT     , quasarDefaults[QUASAR_REG_PLL5             ]); }
    if (QUASAR_PLL6_DEFAULT      != quasarDefaults[QUASAR_REG_PLL6]             ) { phyLog(pMac, LOGW, "ERROR: Quasar reg %d is supposed to have a default of %08X, but instead has %08X\n", QUASAR_REG_PLL6             , QUASAR_PLL6_DEFAULT     , quasarDefaults[QUASAR_REG_PLL6             ]); }
    if (QUASAR_LO_CONFIG_DEFAULT != quasarDefaults[QUASAR_REG_LO_CONFIG]        ) { phyLog(pMac, LOGW, "ERROR: Quasar reg %d is supposed to have a default of %08X, but instead has %08X\n", QUASAR_REG_LO_CONFIG        , QUASAR_LO_CONFIG_DEFAULT, quasarDefaults[QUASAR_REG_LO_CONFIG        ]); }
    if (QUASAR_STATE_DEFAULT     != quasarDefaults[QUASAR_REG_STATE]            ) { phyLog(pMac, LOGW, "ERROR: Quasar reg %d is supposed to have a default of %08X, but instead has %08X\n", QUASAR_REG_STATE            , QUASAR_STATE_DEFAULT    , quasarDefaults[QUASAR_REG_STATE            ]); }
    if (QUASAR_GC1_DEFAULT       != quasarDefaults[QUASAR_REG_GAIN_CONTROL_1]   ) { phyLog(pMac, LOGW, "ERROR: Quasar reg %d is supposed to have a default of %08X, but instead has %08X\n", QUASAR_REG_GAIN_CONTROL_1   , QUASAR_GC1_DEFAULT      , quasarDefaults[QUASAR_REG_GAIN_CONTROL_1   ]); }
    if (QUASAR_GC2_DEFAULT       != quasarDefaults[QUASAR_REG_GAIN_CONTROL_2]   ) { phyLog(pMac, LOGW, "ERROR: Quasar reg %d is supposed to have a default of %08X, but instead has %08X\n", QUASAR_REG_GAIN_CONTROL_2   , QUASAR_GC2_DEFAULT      , quasarDefaults[QUASAR_REG_GAIN_CONTROL_2   ]); }
    if (QUASAR_GC3_DEFAULT       != quasarDefaults[QUASAR_REG_GAIN_CONTROL_3]   ) { phyLog(pMac, LOGW, "ERROR: Quasar reg %d is supposed to have a default of %08X, but instead has %08X\n", QUASAR_REG_GAIN_CONTROL_3   , QUASAR_GC3_DEFAULT      , quasarDefaults[QUASAR_REG_GAIN_CONTROL_3   ]); }
    if (QUASAR_GC4_DEFAULT       != quasarDefaults[QUASAR_REG_GAIN_CONTROL_4]   ) { phyLog(pMac, LOGW, "ERROR: Quasar reg %d is supposed to have a default of %08X, but instead has %08X\n", QUASAR_REG_GAIN_CONTROL_4   , QUASAR_GC4_DEFAULT      , quasarDefaults[QUASAR_REG_GAIN_CONTROL_4   ]); }
    if (QUASAR_DCOC0_DEFAULT     != quasarDefaults[QUASAR_REG_DC_OFFSET_CHAIN_0]) { phyLog(pMac, LOGW, "ERROR: Quasar reg %d is supposed to have a default of %08X, but instead has %08X\n", QUASAR_REG_DC_OFFSET_CHAIN_0, QUASAR_DCOC0_DEFAULT    , quasarDefaults[QUASAR_REG_DC_OFFSET_CHAIN_0]); }
    if (QUASAR_DCOC1_DEFAULT     != quasarDefaults[QUASAR_REG_DC_OFFSET_CHAIN_1]) { phyLog(pMac, LOGW, "ERROR: Quasar reg %d is supposed to have a default of %08X, but instead has %08X\n", QUASAR_REG_DC_OFFSET_CHAIN_1, QUASAR_DCOC1_DEFAULT    , quasarDefaults[QUASAR_REG_DC_OFFSET_CHAIN_1]); }
    if (QUASAR_DCOC2_DEFAULT     != quasarDefaults[QUASAR_REG_DC_OFFSET_CHAIN_2]) { phyLog(pMac, LOGW, "ERROR: Quasar reg %d is supposed to have a default of %08X, but instead has %08X\n", QUASAR_REG_DC_OFFSET_CHAIN_2, QUASAR_DCOC2_DEFAULT    , quasarDefaults[QUASAR_REG_DC_OFFSET_CHAIN_2]); }
    if (QUASAR_CFG1_DEFAULT      != quasarDefaults[QUASAR_REG_CFG_1]            ) { phyLog(pMac, LOGW, "ERROR: Quasar reg %d is supposed to have a default of %08X, but instead has %08X\n", QUASAR_REG_CFG_1            , QUASAR_CFG1_DEFAULT     , quasarDefaults[QUASAR_REG_CFG_1            ]); }
    if (QUASAR_CFG2_DEFAULT      != quasarDefaults[QUASAR_REG_CFG_2]            ) { phyLog(pMac, LOGW, "ERROR: Quasar reg %d is supposed to have a default of %08X, but instead has %08X\n", QUASAR_REG_CFG_2            , QUASAR_CFG2_DEFAULT     , quasarDefaults[QUASAR_REG_CFG_2            ]); }
    if (QUASAR_CFG3_DEFAULT      != quasarDefaults[QUASAR_REG_CFG_3]            ) { phyLog(pMac, LOGW, "ERROR: Quasar reg %d is supposed to have a default of %08X, but instead has %08X\n", QUASAR_REG_CFG_3            , QUASAR_CFG3_DEFAULT     , quasarDefaults[QUASAR_REG_CFG_3            ]); }
    if (QUASAR_CFG4_DEFAULT      != quasarDefaults[QUASAR_REG_CFG_4]            ) { phyLog(pMac, LOGW, "ERROR: Quasar reg %d is supposed to have a default of %08X, but instead has %08X\n", QUASAR_REG_CFG_4            , QUASAR_CFG4_DEFAULT     , quasarDefaults[QUASAR_REG_CFG_4            ]); }
    if (QUASAR_CFG5_DEFAULT      != quasarDefaults[QUASAR_REG_CFG_5]            ) { phyLog(pMac, LOGW, "ERROR: Quasar reg %d is supposed to have a default of %08X, but instead has %08X\n", QUASAR_REG_CFG_5            , QUASAR_CFG5_DEFAULT     , quasarDefaults[QUASAR_REG_CFG_5            ]); }
    if (QUASAR_CFG6_DEFAULT      != quasarDefaults[QUASAR_REG_CFG_6]            ) { phyLog(pMac, LOGW, "ERROR: Quasar reg %d is supposed to have a default of %08X, but instead has %08X\n", QUASAR_REG_CFG_6            , QUASAR_CFG6_DEFAULT     , quasarDefaults[QUASAR_REG_CFG_6            ]); }
    if (QUASAR_TEST1_DEFAULT     != quasarDefaults[QUASAR_REG_TEST_1]           ) { phyLog(pMac, LOGW, "ERROR: Quasar reg %d is supposed to have a default of %08X, but instead has %08X\n", QUASAR_REG_TEST_1           , QUASAR_TEST1_DEFAULT    , quasarDefaults[QUASAR_REG_TEST_1           ]); }
    if (QUASAR_TEST2_DEFAULT     != quasarDefaults[QUASAR_REG_TEST_2]           ) { phyLog(pMac, LOGW, "ERROR: Quasar reg %d is supposed to have a default of %08X, but instead has %08X\n", QUASAR_REG_TEST_2           , QUASAR_TEST2_DEFAULT    , quasarDefaults[QUASAR_REG_TEST_2           ]); }
    if (QUASAR_TEST3_DEFAULT     != quasarDefaults[QUASAR_REG_TEST_3]           ) { phyLog(pMac, LOGW, "ERROR: Quasar reg %d is supposed to have a default of %08X, but instead has %08X\n", QUASAR_REG_TEST_3           , QUASAR_TEST3_DEFAULT    , quasarDefaults[QUASAR_REG_TEST_3           ]); }
    if (QUASAR_TEST4_DEFAULT     != quasarDefaults[QUASAR_REG_TEST_4]           ) { phyLog(pMac, LOGW, "ERROR: Quasar reg %d is supposed to have a default of %08X, but instead has %08X\n", QUASAR_REG_TEST_4           , QUASAR_TEST4_DEFAULT    , quasarDefaults[QUASAR_REG_TEST_4           ]); }
    if (QUASAR_TEST5_DEFAULT     != quasarDefaults[QUASAR_REG_TEST_5]           ) { phyLog(pMac, LOGW, "ERROR: Quasar reg %d is supposed to have a default of %08X, but instead has %08X\n", QUASAR_REG_TEST_5           , QUASAR_TEST5_DEFAULT    , quasarDefaults[QUASAR_REG_TEST_5           ]); }
    if (QUASAR_TEST6_DEFAULT     != quasarDefaults[QUASAR_REG_TEST_6]           ) { phyLog(pMac, LOGW, "ERROR: Quasar reg %d is supposed to have a default of %08X, but instead has %08X\n", QUASAR_REG_TEST_6           , QUASAR_TEST6_DEFAULT    , quasarDefaults[QUASAR_REG_TEST_6           ]); }
    if (QUASAR_SPARES_DEFAULT    != quasarDefaults[QUASAR_REG_SPARES]           ) { phyLog(pMac, LOGW, "ERROR: Quasar reg %d is supposed to have a default of %08X, but instead has %08X\n", QUASAR_REG_SPARES           , QUASAR_SPARES_DEFAULT   , quasarDefaults[QUASAR_REG_SPARES           ]); }


    if (memcmp(&quasarInitVals[QUASAR_REG_PLL1]             , &quasarDefaults[QUASAR_REG_PLL1]             , 4) != 0) { phyLog(pMac, LOGW, "WARN: Quasar reg %d is initialized to %08X, vs. a default value of %08X\n", QUASAR_REG_PLL1             , quasarInitVals[QUASAR_REG_PLL1             ], quasarDefaults[QUASAR_REG_PLL1             ]); }
    if (memcmp(&quasarInitVals[QUASAR_REG_PLL2]             , &quasarDefaults[QUASAR_REG_PLL2]             , 4) != 0) { phyLog(pMac, LOGW, "WARN: Quasar reg %d is initialized to %08X, vs. a default value of %08X\n", QUASAR_REG_PLL2             , quasarInitVals[QUASAR_REG_PLL2             ], quasarDefaults[QUASAR_REG_PLL2             ]); }
    if (memcmp(&quasarInitVals[QUASAR_REG_PLL3]             , &quasarDefaults[QUASAR_REG_PLL3]             , 4) != 0) { phyLog(pMac, LOGW, "WARN: Quasar reg %d is initialized to %08X, vs. a default value of %08X\n", QUASAR_REG_PLL3             , quasarInitVals[QUASAR_REG_PLL3             ], quasarDefaults[QUASAR_REG_PLL3             ]); }
    if (memcmp(&quasarInitVals[QUASAR_REG_PLL4]             , &quasarDefaults[QUASAR_REG_PLL4]             , 4) != 0) { phyLog(pMac, LOGW, "WARN: Quasar reg %d is initialized to %08X, vs. a default value of %08X\n", QUASAR_REG_PLL4             , quasarInitVals[QUASAR_REG_PLL4             ], quasarDefaults[QUASAR_REG_PLL4             ]); }
    if (memcmp(&quasarInitVals[QUASAR_REG_PLL5]             , &quasarDefaults[QUASAR_REG_PLL5]             , 4) != 0) { phyLog(pMac, LOGW, "WARN: Quasar reg %d is initialized to %08X, vs. a default value of %08X\n", QUASAR_REG_PLL5             , quasarInitVals[QUASAR_REG_PLL5             ], quasarDefaults[QUASAR_REG_PLL5             ]); }
    if (memcmp(&quasarInitVals[QUASAR_REG_PLL6]             , &quasarDefaults[QUASAR_REG_PLL6]             , 4) != 0) { phyLog(pMac, LOGW, "WARN: Quasar reg %d is initialized to %08X, vs. a default value of %08X\n", QUASAR_REG_PLL6             , quasarInitVals[QUASAR_REG_PLL6             ], quasarDefaults[QUASAR_REG_PLL6             ]); }
    if (memcmp(&quasarInitVals[QUASAR_REG_LO_CONFIG]        , &quasarDefaults[QUASAR_REG_LO_CONFIG]        , 4) != 0) { phyLog(pMac, LOGW, "WARN: Quasar reg %d is initialized to %08X, vs. a default value of %08X\n", QUASAR_REG_LO_CONFIG        , quasarInitVals[QUASAR_REG_LO_CONFIG        ], quasarDefaults[QUASAR_REG_LO_CONFIG        ]); }
    if (memcmp(&quasarInitVals[QUASAR_REG_STATE]            , &quasarDefaults[QUASAR_REG_STATE]            , 4) != 0) { phyLog(pMac, LOGW, "WARN: Quasar reg %d is initialized to %08X, vs. a default value of %08X\n", QUASAR_REG_STATE            , quasarInitVals[QUASAR_REG_STATE            ], quasarDefaults[QUASAR_REG_STATE            ]); }
    if (memcmp(&quasarInitVals[QUASAR_REG_GAIN_CONTROL_1]   , &quasarDefaults[QUASAR_REG_GAIN_CONTROL_1]   , 4) != 0) { phyLog(pMac, LOGW, "WARN: Quasar reg %d is initialized to %08X, vs. a default value of %08X\n", QUASAR_REG_GAIN_CONTROL_1   , quasarInitVals[QUASAR_REG_GAIN_CONTROL_1   ], quasarDefaults[QUASAR_REG_GAIN_CONTROL_1   ]); }
    if (memcmp(&quasarInitVals[QUASAR_REG_GAIN_CONTROL_2]   , &quasarDefaults[QUASAR_REG_GAIN_CONTROL_2]   , 4) != 0) { phyLog(pMac, LOGW, "WARN: Quasar reg %d is initialized to %08X, vs. a default value of %08X\n", QUASAR_REG_GAIN_CONTROL_2   , quasarInitVals[QUASAR_REG_GAIN_CONTROL_2   ], quasarDefaults[QUASAR_REG_GAIN_CONTROL_2   ]); }
    if (memcmp(&quasarInitVals[QUASAR_REG_GAIN_CONTROL_3]   , &quasarDefaults[QUASAR_REG_GAIN_CONTROL_3]   , 4) != 0) { phyLog(pMac, LOGW, "WARN: Quasar reg %d is initialized to %08X, vs. a default value of %08X\n", QUASAR_REG_GAIN_CONTROL_3   , quasarInitVals[QUASAR_REG_GAIN_CONTROL_3   ], quasarDefaults[QUASAR_REG_GAIN_CONTROL_3   ]); }
    if (memcmp(&quasarInitVals[QUASAR_REG_GAIN_CONTROL_4]   , &quasarDefaults[QUASAR_REG_GAIN_CONTROL_4]   , 4) != 0) { phyLog(pMac, LOGW, "WARN: Quasar reg %d is initialized to %08X, vs. a default value of %08X\n", QUASAR_REG_GAIN_CONTROL_4   , quasarInitVals[QUASAR_REG_GAIN_CONTROL_4   ], quasarDefaults[QUASAR_REG_GAIN_CONTROL_4   ]); }
    if (memcmp(&quasarInitVals[QUASAR_REG_DC_OFFSET_CHAIN_0], &quasarDefaults[QUASAR_REG_DC_OFFSET_CHAIN_0], 4) != 0) { phyLog(pMac, LOGW, "WARN: Quasar reg %d is initialized to %08X, vs. a default value of %08X\n", QUASAR_REG_DC_OFFSET_CHAIN_0, quasarInitVals[QUASAR_REG_DC_OFFSET_CHAIN_0], quasarDefaults[QUASAR_REG_DC_OFFSET_CHAIN_0]); }
    if (memcmp(&quasarInitVals[QUASAR_REG_DC_OFFSET_CHAIN_1], &quasarDefaults[QUASAR_REG_DC_OFFSET_CHAIN_1], 4) != 0) { phyLog(pMac, LOGW, "WARN: Quasar reg %d is initialized to %08X, vs. a default value of %08X\n", QUASAR_REG_DC_OFFSET_CHAIN_1, quasarInitVals[QUASAR_REG_DC_OFFSET_CHAIN_1], quasarDefaults[QUASAR_REG_DC_OFFSET_CHAIN_1]); }
    if (memcmp(&quasarInitVals[QUASAR_REG_DC_OFFSET_CHAIN_2], &quasarDefaults[QUASAR_REG_DC_OFFSET_CHAIN_2], 4) != 0) { phyLog(pMac, LOGW, "WARN: Quasar reg %d is initialized to %08X, vs. a default value of %08X\n", QUASAR_REG_DC_OFFSET_CHAIN_2, quasarInitVals[QUASAR_REG_DC_OFFSET_CHAIN_2], quasarDefaults[QUASAR_REG_DC_OFFSET_CHAIN_2]); }
    if (memcmp(&quasarInitVals[QUASAR_REG_CFG_1]            , &quasarDefaults[QUASAR_REG_CFG_1]            , 4) != 0) { phyLog(pMac, LOGW, "WARN: Quasar reg %d is initialized to %08X, vs. a default value of %08X\n", QUASAR_REG_CFG_1            , quasarInitVals[QUASAR_REG_CFG_1            ], quasarDefaults[QUASAR_REG_CFG_1            ]); }
    if (memcmp(&quasarInitVals[QUASAR_REG_CFG_2]            , &quasarDefaults[QUASAR_REG_CFG_2]            , 4) != 0) { phyLog(pMac, LOGW, "WARN: Quasar reg %d is initialized to %08X, vs. a default value of %08X\n", QUASAR_REG_CFG_2            , quasarInitVals[QUASAR_REG_CFG_2            ], quasarDefaults[QUASAR_REG_CFG_2            ]); }
    if (memcmp(&quasarInitVals[QUASAR_REG_CFG_3]            , &quasarDefaults[QUASAR_REG_CFG_3]            , 4) != 0) { phyLog(pMac, LOGW, "WARN: Quasar reg %d is initialized to %08X, vs. a default value of %08X\n", QUASAR_REG_CFG_3            , quasarInitVals[QUASAR_REG_CFG_3            ], quasarDefaults[QUASAR_REG_CFG_3            ]); }
    if (memcmp(&quasarInitVals[QUASAR_REG_CFG_4]            , &quasarDefaults[QUASAR_REG_CFG_4]            , 4) != 0) { phyLog(pMac, LOGW, "WARN: Quasar reg %d is initialized to %08X, vs. a default value of %08X\n", QUASAR_REG_CFG_4            , quasarInitVals[QUASAR_REG_CFG_4            ], quasarDefaults[QUASAR_REG_CFG_4            ]); }
    if (memcmp(&quasarInitVals[QUASAR_REG_CFG_5]            , &quasarDefaults[QUASAR_REG_CFG_5]            , 4) != 0) { phyLog(pMac, LOGW, "WARN: Quasar reg %d is initialized to %08X, vs. a default value of %08X\n", QUASAR_REG_CFG_5            , quasarInitVals[QUASAR_REG_CFG_5            ], quasarDefaults[QUASAR_REG_CFG_5            ]); }
    if (memcmp(&quasarInitVals[QUASAR_REG_CFG_6]            , &quasarDefaults[QUASAR_REG_CFG_6]            , 4) != 0) { phyLog(pMac, LOGW, "WARN: Quasar reg %d is initialized to %08X, vs. a default value of %08X\n", QUASAR_REG_CFG_6            , quasarInitVals[QUASAR_REG_CFG_6            ], quasarDefaults[QUASAR_REG_CFG_6            ]); }
    if (memcmp(&quasarInitVals[QUASAR_REG_TEST_1]           , &quasarDefaults[QUASAR_REG_TEST_1]           , 4) != 0) { phyLog(pMac, LOGW, "WARN: Quasar reg %d is initialized to %08X, vs. a default value of %08X\n", QUASAR_REG_TEST_1           , quasarInitVals[QUASAR_REG_TEST_1           ], quasarDefaults[QUASAR_REG_TEST_1           ]); }
    if (memcmp(&quasarInitVals[QUASAR_REG_TEST_2]           , &quasarDefaults[QUASAR_REG_TEST_2]           , 4) != 0) { phyLog(pMac, LOGW, "WARN: Quasar reg %d is initialized to %08X, vs. a default value of %08X\n", QUASAR_REG_TEST_2           , quasarInitVals[QUASAR_REG_TEST_2           ], quasarDefaults[QUASAR_REG_TEST_2           ]); }
    if (memcmp(&quasarInitVals[QUASAR_REG_TEST_3]           , &quasarDefaults[QUASAR_REG_TEST_3]           , 4) != 0) { phyLog(pMac, LOGW, "WARN: Quasar reg %d is initialized to %08X, vs. a default value of %08X\n", QUASAR_REG_TEST_3           , quasarInitVals[QUASAR_REG_TEST_3           ], quasarDefaults[QUASAR_REG_TEST_3           ]); }
    if (memcmp(&quasarInitVals[QUASAR_REG_TEST_4]           , &quasarDefaults[QUASAR_REG_TEST_4]           , 4) != 0) { phyLog(pMac, LOGW, "WARN: Quasar reg %d is initialized to %08X, vs. a default value of %08X\n", QUASAR_REG_TEST_4           , quasarInitVals[QUASAR_REG_TEST_4           ], quasarDefaults[QUASAR_REG_TEST_4           ]); }
    if (memcmp(&quasarInitVals[QUASAR_REG_TEST_5]           , &quasarDefaults[QUASAR_REG_TEST_5]           , 4) != 0) { phyLog(pMac, LOGW, "WARN: Quasar reg %d is initialized to %08X, vs. a default value of %08X\n", QUASAR_REG_TEST_5           , quasarInitVals[QUASAR_REG_TEST_5           ], quasarDefaults[QUASAR_REG_TEST_5           ]); }
    if (memcmp(&quasarInitVals[QUASAR_REG_TEST_6]           , &quasarDefaults[QUASAR_REG_TEST_6]           , 4) != 0) { phyLog(pMac, LOGW, "WARN: Quasar reg %d is initialized to %08X, vs. a default value of %08X\n", QUASAR_REG_TEST_6           , quasarInitVals[QUASAR_REG_TEST_6           ], quasarDefaults[QUASAR_REG_TEST_6           ]); }
    if (memcmp(&quasarInitVals[QUASAR_REG_SPARES]           , &quasarDefaults[QUASAR_REG_SPARES]           , 4) != 0) { phyLog(pMac, LOGW, "WARN: Quasar reg %d is initialized to %08X, vs. a default value of %08X\n", QUASAR_REG_SPARES           , quasarInitVals[QUASAR_REG_SPARES           ], quasarDefaults[QUASAR_REG_SPARES           ]); }
*/

#endif

    //reload all registers with initialization values
    SET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_STATE            , quasarInitVals[QUASAR_REG_STATE]            );
    SET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_PLL1             , quasarInitVals[QUASAR_REG_PLL1]             );
    SET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_PLL2             , quasarInitVals[QUASAR_REG_PLL2]             );
    SET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_PLL3             , quasarInitVals[QUASAR_REG_PLL3]             );
    SET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_PLL4             , quasarInitVals[QUASAR_REG_PLL4]             );
    SET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_PLL5             , quasarInitVals[QUASAR_REG_PLL5]             );
    SET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_PLL6             , quasarInitVals[QUASAR_REG_PLL6]             );
    SET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_LO_CONFIG        , quasarInitVals[QUASAR_REG_LO_CONFIG]        );
    SET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_GAIN_CONTROL_1   , quasarInitVals[QUASAR_REG_GAIN_CONTROL_1]   );
    SET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_GAIN_CONTROL_2   , quasarInitVals[QUASAR_REG_GAIN_CONTROL_2]   );
    SET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_GAIN_CONTROL_3   , quasarInitVals[QUASAR_REG_GAIN_CONTROL_3]   );
    SET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_GAIN_CONTROL_4   , quasarInitVals[QUASAR_REG_GAIN_CONTROL_4]   );
    SET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_DC_OFFSET_CHAIN_0, quasarInitVals[QUASAR_REG_DC_OFFSET_CHAIN_0]);
    SET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_DC_OFFSET_CHAIN_1, quasarInitVals[QUASAR_REG_DC_OFFSET_CHAIN_1]);
    SET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_DC_OFFSET_CHAIN_2, quasarInitVals[QUASAR_REG_DC_OFFSET_CHAIN_2]);
    SET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_CFG_1            , quasarInitVals[QUASAR_REG_CFG_1]            );
    SET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_CFG_2            , quasarInitVals[QUASAR_REG_CFG_2]            );
    SET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_CFG_3            , quasarInitVals[QUASAR_REG_CFG_3]            );
    SET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_CFG_4            , quasarInitVals[QUASAR_REG_CFG_4]            );
    SET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_CFG_5            , quasarInitVals[QUASAR_REG_CFG_5]            );
    SET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_CFG_6            , quasarInitVals[QUASAR_REG_CFG_6]            );
    SET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_TEST_1           , quasarInitVals[QUASAR_REG_TEST_1]           );
    SET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_TEST_2           , quasarInitVals[QUASAR_REG_TEST_2]           );
    SET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_TEST_3           , quasarInitVals[QUASAR_REG_TEST_3]           );
    SET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_TEST_4           , quasarInitVals[QUASAR_REG_TEST_4]           );
    SET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_TEST_5           , quasarInitVals[QUASAR_REG_TEST_5]           );
    SET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_SPARES           , quasarInitVals[QUASAR_REG_SPARES]           );

    //registers setup with default dividers, turn on PLL
    SET_SPI_FIELD(pMac, QUASAR_REG_CFG_1, QUASAR_XO_BUF_EN_MASK, QUASAR_XO_BUF_EN_OFFSET, 1);
    SET_SPI_FIELD(pMac, QUASAR_REG_STATE, QUASAR_PLL_EN_MASK, QUASAR_PLL_EN_OFFSET, eANI_BOOLEAN_ON);
#ifdef ONE_SHOT_AUTO_COARSE_TUNING
    SET_SPI_FIELD(pMac, QUASAR_REG_PLL3, QUASAR_CT_EN_MASK, QUASAR_CT_EN_OFFSET, 0);    //no automatic vco tuning
#endif

    SET_SPI_FIELD(pMac, QUASAR_REG_PLL5, QUASAR_THREE_FOURB_MASK, QUASAR_THREE_FOURB_OFFSET, 1); //per Rainer & Troy on 9/28/06

    //values taken from Betta - need more information on context of these
    rfWriteQuasarField(pMac, QUASAR_FIELD_MOD_INV, 7);
    rfWriteQuasarField(pMac, QUASAR_FIELD_XO_DRV, 0);
    rfWriteQuasarField(pMac, QUASAR_FIELD_RX_IF_TUN, 4);
    rfWriteQuasarField(pMac, QUASAR_FIELD_CLIP_DET_TH, 7);
    rfWriteQuasarField(pMac, QUASAR_FIELD_TX_IQ_GAIN, 6);
    rfWriteQuasarField(pMac, QUASAR_FIELD_BBF_GAIN1, 0);

    


    return (retVal);
}


eHalStatus rfGetVersion(tpAniSirGlobal pMac, eRfChipSelect chipSel, tRfChipVer *chipVer)
{
    eHalStatus retVal;
    tANI_U32 subVer, ver;

    GET_SPI_FIELD(pMac, QUASAR_REG_STATE, QUASAR_MS_REV_MASK, QUASAR_MS_REV_OFFSET, (tANI_U32 *)&subVer);
    GET_SPI_FIELD(pMac, QUASAR_REG_STATE, QUASAR_MREV_MASK, QUASAR_MREV_OFFSET, (tANI_U32 *)&ver);

    chipVer->subVer = (tANI_U8)subVer;
    chipVer->ver = (tANI_U8)ver;
    
    return (retVal);
}


eHalStatus rfSetXcvrState(tpAniSirGlobal pMac, eRfEnableState state)
{
    eHalStatus retVal;


    switch (state)
    {
        case RF_OFF:         //PLL, XO, XO_BUF disabled
            SET_SPI_FIELD(pMac, QUASAR_REG_STATE, QUASAR_PLL_EN_MASK, QUASAR_PLL_EN_OFFSET, 0);
            //TODO: verify we should not do this for beacon power save SET_SPI_FIELD(pMac, QUASAR_REG_CFG_1, QUASAR_XO_BUF_EN_MASK, QUASAR_XO_BUF_EN_OFFSET, 0);
            break;
        case RF_XO_ONLY:     //PLL disabled, XO_BUF enabled (assumes XO is enabled)
            SET_SPI_FIELD(pMac, QUASAR_REG_STATE, QUASAR_PLL_EN_MASK, QUASAR_PLL_EN_OFFSET, 0);
            break;
        case RF_ON:           //PLL, XO_BUF enabled (assumes XO is enabled)
            SET_SPI_FIELD(pMac, QUASAR_REG_STATE, QUASAR_PLL_EN_MASK, QUASAR_PLL_EN_OFFSET, 1);
            {
                tANI_U32 pllVregD;  //delay when enabling the PLL

                GET_SPI_FIELD(pMac, QUASAR_REG_PLL4, QUASAR_PLL_VREG_D_MASK, QUASAR_PLL_VREG_D_OFFSET, &pllVregD);

                if (pllVregD)
                {
                    sirBusyWait(500000);   //500us for PLL regulators to stabilize
                }
                else
                {
                    sirBusyWait(250000);   //250us for PLL regulators to stabilize
                }
            }

            {
                tANI_U32 ndivFrac;
                /* We need to force a coarse tune because temperature change since power down may have changed the conditions for the VCO band.
                    We can only force a coarse tune directly in auto-maintenance mode(erata #33),
                    but we cannot use auto-maintenance mode because the coarse tune only works after enabling the PLL(erata #36).

                    The plan is to use single-shot maintenance on a periodic basis rather than automatic maintenance.
                    Then we will change the NDIV bits here after PLL enable, because this will force a coarse tune.
                    Reportedly, this works even if the NDIV frac-mode is turned off.
                    We'll read the NDIV.fracHi part and XOR it once to change it, and again to set it back to the original.

                */
                GET_SPI_FIELD(pMac, QUASAR_REG_PLL1, QUASAR_NDIV_FRAC_HI_MASK, QUASAR_NDIV_FRAC_HI_OFFSET, &ndivFrac);
                ndivFrac ^= ndivFrac;
                SET_SPI_FIELD(pMac, QUASAR_REG_PLL1, QUASAR_NDIV_FRAC_HI_MASK, QUASAR_NDIV_FRAC_HI_OFFSET, ndivFrac);
                ndivFrac ^= ndivFrac;
                SET_SPI_FIELD(pMac, QUASAR_REG_PLL1, QUASAR_NDIV_FRAC_HI_MASK, QUASAR_NDIV_FRAC_HI_OFFSET, ndivFrac);
                sirBusyWait(500000);   //500us for coarse tuning
            }
            break;
        default:
            assert(0);
            return (eHAL_STATUS_FAILURE);
            break;
    }

    return (retVal);
}


eHalStatus rfSetChainSelectionMode(tpAniSirGlobal pMac, ePhyChainSelect rfSysRxTxAntennaMode)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;

    //these are bit masks to set in the STATE register
    tANI_U32 rxEnabled;
    tANI_U32 rx1Enabled;
    tANI_U32 txEnabled;
    tANI_U32 regVal;

    switch (rfSysRxTxAntennaMode)
    {
        case PHY_CHAIN_SEL_NO_RX_TX:
            rxEnabled = 0;
            rx1Enabled = 0;
            txEnabled = 0;
            break;

        case PHY_CHAIN_SEL_R0_T0_ON:
            rxEnabled = 1;
            rx1Enabled = 1;
            txEnabled = 1;
            break;

        case PHY_CHAIN_SEL_R0R1_T0_ON:
            rxEnabled = 3;
            rx1Enabled = 1;
            txEnabled = 1;
            break;

        case PHY_CHAIN_SEL_R0R1_T0T1_ON:
            rxEnabled = 3;
            rx1Enabled = 1;
            txEnabled = 3;
            break;

        case PHY_CHAIN_SEL_R0R1R2_T0T1_ON:
            rxEnabled = 7;
            rx1Enabled = 1;
            txEnabled = 3;
            break;

        case PHY_CHAIN_SEL_T0_ONLY:
            rxEnabled = 0;
            rx1Enabled = 0;
            txEnabled = 1;
            break;

        case PHY_CHAIN_SEL_T1_ONLY:
            rxEnabled = 0;
            rx1Enabled = 0;
            txEnabled = 2;
            break;

        case PHY_CHAIN_SEL_T0T1_ONLY:
            rxEnabled = 0;
            rx1Enabled = 0;
            txEnabled = 3;
            break;

        case PHY_CHAIN_SEL_R0_ONLY:
            rxEnabled = 1;
            rx1Enabled = 1;
            txEnabled = 0;
            break;

        case PHY_CHAIN_SEL_R1_ONLY:
            rxEnabled = 2;
            rx1Enabled = 2;
            txEnabled = 0;
            break;

        case PHY_CHAIN_SEL_R2_ONLY:
            rxEnabled = 4;
            rx1Enabled = 4;
            txEnabled = 0;
            break;

        case PHY_CHAIN_SEL_R0R1_ONLY:
            rxEnabled = 3;
            rx1Enabled = 1;
            txEnabled = 0;
            break;

        case PHY_CHAIN_SEL_R0R1R2_ONLY:
            rxEnabled = 7;
            rx1Enabled = 1;
            txEnabled = 0;
            break;

        default:
            assert(0);  //no other valid modes
            return (eHAL_STATUS_FAILURE);
    }

    retVal = rfReadDataField(pMac, QUASAR_REG_STATE, MSK_32, 0, &regVal);
    if (retVal != eHAL_STATUS_SUCCESS)
    {
        return retVal;
    }
    regVal &= ~((QUASAR_TX_ST_EN_MASK << QUASAR_TX_ST_EN_OFFSET) | 
                (QUASAR_RX_ALL_ST_EN_MASK << QUASAR_RX_ALL_ST_EN_OFFSET) |
                (QUASAR_RX1_ST_EN_MASK << QUASAR_RX1_ST_EN_OFFSET));

    SET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_STATE, regVal | ((txEnabled & QUASAR_TX_ST_EN_MASK) << QUASAR_TX_ST_EN_OFFSET)
                                                     | ((rxEnabled & QUASAR_RX_ALL_ST_EN_MASK) << QUASAR_RX_ALL_ST_EN_OFFSET)
                                                     | ((rx1Enabled & QUASAR_RX1_ST_EN_MASK) << QUASAR_RX1_ST_EN_OFFSET));

    return (retVal);
}


/* agcTable may be a single dimension if rxChain indicates a single chain or two-dimensions if rxChains = PHY_ALL_RX_CHAINS
    agcTable contains an ordered list of gains for the indicated chain(s) which correspond from minIndex to maxIndex as specified
    Note that the contents in agcTable does not necessarily match NUM_AGC_GAINS or start with minIndex = 0
*/
eHalStatus rfSetRxGainLut(tpAniSirGlobal pMac, ePhyRxChains rxChain, tANI_U8 minIndex, tANI_U8 maxIndex, const tRxGain *agcTable)
{
    eHalStatus retVal;

    
    SET_SPI_FIELD(pMac, QUASAR_REG_GAIN_CONTROL_2, QUASAR_SPI_LUT_EN_MASK, QUASAR_SPI_LUT_EN_OFFSET, 1);

    {
        tANI_U32 gainIndex;

        assert(maxIndex <= 44); //Quasar is limited to 45 gain indexes
        assert(minIndex <= 44);
        assert(minIndex <= maxIndex);

        for (gainIndex = minIndex; gainIndex <= maxIndex; gainIndex++)
        {
            SET_SPI_FIELD(pMac, QUASAR_REG_GAIN_CONTROL_2, QUASAR_SPI_LUT_INDEX_MASK, QUASAR_SPI_LUT_INDEX_OFFSET, gainIndex);

            {
                tANI_U32 i = gainIndex - minIndex;

                //write 7-bit gain values
                switch (rxChain)
                {
                    case PHY_RX_CHAIN_0:
                        SET_SPI_FIELD(pMac, QUASAR_REG_GAIN_CONTROL_4, QUASAR_RX_GAIN_0_MASK, QUASAR_RX_GAIN_0_OFFSET, agcTable[i].agcGain);
                        break;

                    case PHY_RX_CHAIN_1:
                        {
                            //since this register contains both chain 1 & 2 gains, 
                            // we need to read it back for this index in order to have the correct value in the cache, 
                            // which is used to write the field.
                            //Otherwise, the register will contain the value from the last index used, rather than this index's value.
                            tANI_U32 reg;
                            
                            GET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_GAIN_CONTROL_3, &reg);
                            reg &= ~(QUASAR_RX_GAIN_1_MASK << QUASAR_RX_GAIN_1_OFFSET);
                            reg |= (agcTable[i].agcGain << QUASAR_RX_GAIN_1_OFFSET);
                            SET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_GAIN_CONTROL_3, reg);
                        }
                        
                        break;

                    case PHY_RX_CHAIN_2:
                        {
                            //since this register contains both chain 1 & 2 gains, 
                            // we need to read it back for this index in order to have the correct value in the cache, 
                            // which is used to write the field.
                            //Otherwise, the register will contain the value from the last index used, rather than this index's value.
                            tANI_U32 reg;
                            
                            GET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_GAIN_CONTROL_3, &reg);
                            reg &= ~(QUASAR_RX_GAIN_2_MASK << QUASAR_RX_GAIN_2_OFFSET);
                            reg |= (agcTable[i].agcGain << QUASAR_RX_GAIN_2_OFFSET);
                            SET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_GAIN_CONTROL_3, reg);
                        }
                        break;

                    default:
                        assert(0);
                        return(eHAL_STATUS_FAILURE);
                }

                if (retVal != eHAL_STATUS_SUCCESS)
                {
                    return (retVal);
                }
            }
        }

        SET_SPI_FIELD(pMac, QUASAR_REG_GAIN_CONTROL_2, QUASAR_SPI_LUT_EN_MASK, QUASAR_SPI_LUT_EN_OFFSET, 0);
    }
    
    
    
/*      DEBUG ONLY Leave for later use
        rfWriteQuasarField(pMac, QUASAR_FIELD_SPI_LUT_EN, 1);
    
        {
            tANI_U32 gainIndex;
            phyLog(pMac, LOGE, " \tgain0   \tgain1   \tgain2");
    
            for (gainIndex = 0; gainIndex < NUM_QUASAR_RX_GAIN_STEPS; gainIndex++)
            {
                tANI_U32 gain0 = 0, gain1 = 0, gain2 = 0;
                rfWriteQuasarField(pMac, QUASAR_FIELD_SPI_LUT_INDEX, gainIndex);
    
                rfReadQuasarField(pMac, QUASAR_FIELD_RX_GAIN_0, &gain0);
                rfReadQuasarField(pMac, QUASAR_FIELD_RX_GAIN_1, &gain1);
                rfReadQuasarField(pMac, QUASAR_FIELD_RX_GAIN_2, &gain2);
    
                phyLog(pMac, LOGE, "%d: \t0x%04X \t0x%04X \t0x%04X\n", gainIndex, gain0, gain1, gain2);
            }
    
            rfWriteQuasarField(pMac, QUASAR_FIELD_SPI_LUT_EN, 0);
        }
*/

    return (retVal);
}



//channel/band/freq functions
eRfSubBand rfGetBand(tpAniSirGlobal pMac, eRfChannels chan)
{
    assert(chan < NUM_RF_CHANNELS);
    return(rfChannels[chan].band);
}

eRfSubBand rfGetAGBand(tpAniSirGlobal pMac)
{
    eRfSubBand  bandIndex;

    bandIndex = rfGetBand(pMac, rfGetCurChannel(pMac));

    switch (bandIndex)
    {
        case RF_SUBBAND_2_4_GHZ:
            bandIndex = RF_BAND_2_4_GHZ;
            break;
        case RF_SUBBAND_5_LOW_GHZ:
        case RF_SUBBAND_5_MID_GHZ:
        case RF_SUBBAND_5_HIGH_GHZ:
        case RF_SUBBAND_4_9_GHZ:
            bandIndex = RF_BAND_5_GHZ;
            break;
        default:
            phyLog(pMac, LOGE, "ERROR: band not found\n");
            return (INVALID_RF_SUBBAND);
    }
    return(bandIndex);
}



static eHalStatus PopulateQuasarRegistersFromBringupTable(tpAniSirGlobal pMac, eRfChannels chan)
{
    eHalStatus retVal;

    {
        tANI_U32 val;
        
        SET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_CFG_1,              pMac->hphy.rf.quasarRegsTable[chan][INDEX_QUASAR_REG_CFG_1]);
        SET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_PLL3,               pMac->hphy.rf.quasarRegsTable[chan][INDEX_QUASAR_REG_PLL3]);
        SET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_PLL2,               pMac->hphy.rf.quasarRegsTable[chan][INDEX_QUASAR_REG_PLL2]);
        SET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_PLL1,               pMac->hphy.rf.quasarRegsTable[chan][INDEX_QUASAR_REG_PLL1]);
        SET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_PLL4,               pMac->hphy.rf.quasarRegsTable[chan][INDEX_QUASAR_REG_PLL4]);
        SET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_PLL5,               pMac->hphy.rf.quasarRegsTable[chan][INDEX_QUASAR_REG_PLL5]);
        SET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_PLL6,               pMac->hphy.rf.quasarRegsTable[chan][INDEX_QUASAR_REG_PLL6]);
        SET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_LO_CONFIG,          pMac->hphy.rf.quasarRegsTable[chan][INDEX_QUASAR_REG_LO_CONFIG]);
        SET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_CFG_2,              pMac->hphy.rf.quasarRegsTable[chan][INDEX_QUASAR_REG_CFG_2]);
        SET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_CFG_3,              pMac->hphy.rf.quasarRegsTable[chan][INDEX_QUASAR_REG_CFG_3]);
        SET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_CFG_4,              pMac->hphy.rf.quasarRegsTable[chan][INDEX_QUASAR_REG_CFG_4]);
        SET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_CFG_5,              pMac->hphy.rf.quasarRegsTable[chan][INDEX_QUASAR_REG_CFG_5]);
        SET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_CFG_6,              pMac->hphy.rf.quasarRegsTable[chan][INDEX_QUASAR_REG_CFG_6]);
        SET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_TEST_2,             pMac->hphy.rf.quasarRegsTable[chan][INDEX_QUASAR_REG_TEST_2]);
        SET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_TEST_5,             pMac->hphy.rf.quasarRegsTable[chan][INDEX_QUASAR_REG_TEST_5]);
        sirBusyWait(500000);
        
        GET_SPI_FIELD(pMac, QUASAR_REG_PLL4, QUASAR_PLL_TUNE_ERR_MASK, QUASAR_PLL_TUNE_ERR_OFFSET, &val);
        assert(val == 0);
        
        GET_SPI_FIELD(pMac, QUASAR_REG_PLL3, QUASAR_PLL_CT_ACTIVE_MASK, QUASAR_PLL_CT_ACTIVE_OFFSET, &val);
        assert(val == 0);
    }

#ifdef ANI_PHY_DEBUG
    phyLog(pMac, LOG2, "Populating Quasar registers from bringup table for channel index %d\n", chan);

    phyLog(pMac, LOG2, "%d: %40s = %08X\n", INDEX_QUASAR_REG_PLL1,      &quasarRegStr[INDEX_QUASAR_REG_PLL1       ][0], pMac->hphy.rf.quasarRegsTable[chan][INDEX_QUASAR_REG_PLL1       ]);
    phyLog(pMac, LOG2, "%d: %40s = %08X\n", INDEX_QUASAR_REG_PLL2,      &quasarRegStr[INDEX_QUASAR_REG_PLL2       ][0], pMac->hphy.rf.quasarRegsTable[chan][INDEX_QUASAR_REG_PLL2       ]);
    phyLog(pMac, LOG2, "%d: %40s = %08X\n", INDEX_QUASAR_REG_PLL3,      &quasarRegStr[INDEX_QUASAR_REG_PLL3       ][0], pMac->hphy.rf.quasarRegsTable[chan][INDEX_QUASAR_REG_PLL3       ]);
    phyLog(pMac, LOG2, "%d: %40s = %08X\n", INDEX_QUASAR_REG_PLL4,      &quasarRegStr[INDEX_QUASAR_REG_PLL4       ][0], pMac->hphy.rf.quasarRegsTable[chan][INDEX_QUASAR_REG_PLL4       ]);
    phyLog(pMac, LOG2, "%d: %40s = %08X\n", INDEX_QUASAR_REG_PLL5,      &quasarRegStr[INDEX_QUASAR_REG_PLL5       ][0], pMac->hphy.rf.quasarRegsTable[chan][INDEX_QUASAR_REG_PLL5       ]);
    phyLog(pMac, LOG2, "%d: %40s = %08X\n", INDEX_QUASAR_REG_PLL6,      &quasarRegStr[INDEX_QUASAR_REG_PLL6       ][0], pMac->hphy.rf.quasarRegsTable[chan][INDEX_QUASAR_REG_PLL6       ]);
    phyLog(pMac, LOG2, "%d: %40s = %08X\n", INDEX_QUASAR_REG_LO_CONFIG, &quasarRegStr[INDEX_QUASAR_REG_LO_CONFIG  ][0], pMac->hphy.rf.quasarRegsTable[chan][INDEX_QUASAR_REG_LO_CONFIG  ]);
    phyLog(pMac, LOG2, "%d: %40s = %08X\n", INDEX_QUASAR_REG_CFG_1,     &quasarRegStr[INDEX_QUASAR_REG_CFG_1      ][0], pMac->hphy.rf.quasarRegsTable[chan][INDEX_QUASAR_REG_CFG_1      ]);
    phyLog(pMac, LOG2, "%d: %40s = %08X\n", INDEX_QUASAR_REG_CFG_2,     &quasarRegStr[INDEX_QUASAR_REG_CFG_2      ][0], pMac->hphy.rf.quasarRegsTable[chan][INDEX_QUASAR_REG_CFG_2      ]);
    phyLog(pMac, LOG2, "%d: %40s = %08X\n", INDEX_QUASAR_REG_CFG_3,     &quasarRegStr[INDEX_QUASAR_REG_CFG_3      ][0], pMac->hphy.rf.quasarRegsTable[chan][INDEX_QUASAR_REG_CFG_3      ]);
    phyLog(pMac, LOG2, "%d: %40s = %08X\n", INDEX_QUASAR_REG_CFG_4,     &quasarRegStr[INDEX_QUASAR_REG_CFG_4      ][0], pMac->hphy.rf.quasarRegsTable[chan][INDEX_QUASAR_REG_CFG_4      ]);
    phyLog(pMac, LOG2, "%d: %40s = %08X\n", INDEX_QUASAR_REG_CFG_5,     &quasarRegStr[INDEX_QUASAR_REG_CFG_5      ][0], pMac->hphy.rf.quasarRegsTable[chan][INDEX_QUASAR_REG_CFG_5      ]);
    phyLog(pMac, LOG2, "%d: %40s = %08X\n", INDEX_QUASAR_REG_CFG_6,     &quasarRegStr[INDEX_QUASAR_REG_CFG_6      ][0], pMac->hphy.rf.quasarRegsTable[chan][INDEX_QUASAR_REG_CFG_6      ]);
    phyLog(pMac, LOG2, "%d: %40s = %08X\n", INDEX_QUASAR_REG_TEST_2,    &quasarRegStr[INDEX_QUASAR_REG_TEST_2     ][0], pMac->hphy.rf.quasarRegsTable[chan][INDEX_QUASAR_REG_TEST_2     ]);
    phyLog(pMac, LOG2, "%d: %40s = %08X\n", INDEX_QUASAR_REG_TEST_5,    &quasarRegStr[INDEX_QUASAR_REG_TEST_5     ][0], pMac->hphy.rf.quasarRegsTable[chan][INDEX_QUASAR_REG_TEST_5     ]);

#endif

    return (eHAL_STATUS_SUCCESS);
}

static eHalStatus rfSetCurBand(tpAniSirGlobal pMac, eRfSubBand band)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
    tANI_U32 i;

    for (i = 0; i < NUM_BAND_FIELDS; i++)
    {
        if (eHAL_STATUS_SUCCESS != (retVal = rfWriteQuasarField(pMac, quasarBandFieldId[i], quasarBandSettings[band][i])))
        {
            return (retVal);
        }
    }

    return (retVal);
}

eHalStatus rfSetChanBondMode(tpAniSirGlobal pMac, tANI_BOOLEAN onOff)
{
    eHalStatus  retVal;
    tANI_U32    regVal;

    retVal = rfReadDataField(pMac, QUASAR_REG_CFG_2, MSK_32, 0, &regVal);
    if (retVal != eHAL_STATUS_SUCCESS)
    {
        return (retVal);
    }

    regVal &= ~((QUASAR_BBF_RX_TUNE_MASK << QUASAR_BBF_RX_TUNE_OFFSET) 
                | (QUASAR_BBF_TX_TUNE_MASK << QUASAR_BBF_TX_TUNE_OFFSET));
    //TODO: What needs to be set for CB mode?
    if (onOff == eANI_BOOLEAN_TRUE)
    {
        regVal |= (5 & QUASAR_BBF_RX_TUNE_MASK) << QUASAR_BBF_RX_TUNE_OFFSET;
        regVal |= (5 & QUASAR_BBF_TX_TUNE_MASK) << QUASAR_BBF_TX_TUNE_OFFSET;
    }
    else
    {
        regVal |= (11 & QUASAR_BBF_RX_TUNE_MASK) << QUASAR_BBF_RX_TUNE_OFFSET;
        regVal |= (11 & QUASAR_BBF_TX_TUNE_MASK) << QUASAR_BBF_TX_TUNE_OFFSET;
    }

    SET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_CFG_2, regVal);

    return (eHAL_STATUS_SUCCESS);
}


static sQuasarDividers CalcNDiv(eRfChannels chan)
{
    sQuasarDividers retVal;

    memcpy(&retVal, &chanNDiv[chan][rdivPerChannel[chan]], sizeof(sQuasarDividers));

    return (retVal);
}


eHalStatus rfSetCurChannel(tpAniSirGlobal pMac, eRfChannels chan)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
    sQuasarDividers ndiv;
    tANI_U32    regVal;

    if (pMac->hphy.rf.quasarRegsTable != NULL)
    {
        //for bringup, set Quasar to exactly what is in the table, and then follow it with whatever else needs to be changed

        retVal = rfReadDataField(pMac, QUASAR_REG_STATE, MSK_32, 0, &regVal);
        if (retVal != eHAL_STATUS_SUCCESS)
        {
            return (retVal);
        }
        regVal &= ~((QUASAR_TXRX_BAND_MASK << QUASAR_TXRX_BAND_OFFSET) 
                    | (QUASAR_TWOG_HS_EN_MASK << QUASAR_TWOG_HS_EN_OFFSET));

        //STATE register
        if (((chan >= MIN_2_4GHZ_CHANNEL) && (chan <= MAX_2_4GHZ_CHANNEL)) ||
            ((chan >= RF_CHAN_BOND_3) && (chan <= RF_CHAN_BOND_11))
           )
        {
            regVal |= (((0 & QUASAR_TXRX_BAND_MASK) << QUASAR_TXRX_BAND_OFFSET)
                        | ((1 & QUASAR_TWOG_HS_EN_MASK) << QUASAR_TWOG_HS_EN_OFFSET));
        }
        else
        {
            regVal |= (((1 & QUASAR_TXRX_BAND_MASK) << QUASAR_TXRX_BAND_OFFSET)
                        | ((0 & QUASAR_TWOG_HS_EN_MASK) << QUASAR_TWOG_HS_EN_OFFSET));
        }
        SET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_STATE, regVal);

        if ((retVal = PopulateQuasarRegistersFromBringupTable(pMac, chan)) != eHAL_STATUS_SUCCESS)
        {
            return (retVal);
        }
        else
        {
            tANI_U32 rdiv;
            tANI_U32 ndivInt;
            tANI_U32 ndivFrac;
            
            //read betta values for divider to compare against my table values
            GET_SPI_FIELD(pMac, QUASAR_REG_PLL3, QUASAR_RDIV_MASK, QUASAR_RDIV_OFFSET, &rdiv);
            rfReadQuasarField(pMac, QUASAR_FIELD_NDIV_INT, &ndivInt);
            rfReadQuasarField(pMac, QUASAR_FIELD_NDIV_FRAC, &ndivFrac);
            
            assert(rdiv <= 3);
            assert(chanNDiv[chan][rdiv].ndivInt == ndivInt);
            assert(chanNDiv[chan][rdiv].ndivFrac == ndivFrac);

        }
    }
    else if ((retVal = rfSetCurBand(pMac, rfChannels[chan].band)) == eHAL_STATUS_SUCCESS)
    {
        if (chan < MIN_CHAN_BOND_CHANNEL)
        {
            if ((retVal = rfSetChanBondMode(pMac, eANI_BOOLEAN_OFF)) != eHAL_STATUS_SUCCESS)
            {
                return (retVal);
            }
        }
        else if ((chan >= MIN_CHAN_BOND_CHANNEL) && (chan <= MAX_CHAN_BOND_CHANNEL))
        {
            if ((retVal = rfSetChanBondMode(pMac, eANI_BOOLEAN_ON)) != eHAL_STATUS_SUCCESS)
            {
                return (retVal);
            }
        }

        ndiv = CalcNDiv(chan);
        SET_SPI_FIELD(pMac, QUASAR_REG_PLL3, QUASAR_RDIV_MASK, QUASAR_RDIV_OFFSET, rdivPerChannel[chan]);
        if (ndiv.ndivFrac == 0)
        {
            //FNZ=1 means the ndiv fractional part is not used, which is what we want when ndivFrac = 0
            SET_SPI_FIELD(pMac, QUASAR_REG_PLL1, QUASAR_FNZ_MASK, QUASAR_FNZ_OFFSET, 1);
        }
        else
        {
            //FNZ=0 means the ndiv fractional part is used, which is what we want when ndivFrac != 0
            SET_SPI_FIELD(pMac, QUASAR_REG_PLL1, QUASAR_FNZ_MASK, QUASAR_FNZ_OFFSET, 0);
        }


        {
            tANI_U32 priorVcoMask;
            tANI_U32 ndivider = ((tANI_U32)ndiv.ndivInt * NDIV_DECIMAL_MULT)+ ndiv.ndivFrac;

            //workaround for quasar errata #33 - need to change the ndiv values in order to have coarse tune work
            //final values set further down
            SET_QUASAR_DIVIDERS(ndiv.ndivInt, ndiv.ndivFrac + 1);
            
            GET_SPI_FIELD(pMac, QUASAR_REG_PLL3, QUASAR_VCO_EN_MASK, QUASAR_VCO_EN_OFFSET, &priorVcoMask);

#ifdef ANI_PHY_DEBUG
            if (pMac->hphy.phy.test.testDisableSpiAccess == eANI_BOOLEAN_FALSE)
            {
                assert(priorVcoMask != 0);  //0=automatic - see VCO_EN errata #35 - we can't use this setting
            }
#endif
            if (ndivider < VCO_BAND_THRESHOLD(rdivPerChannel[chan]) )
            {
                //we need to see if the reqd vco mask is different than before, so that we can disable the PLL before changin it.
                if (priorVcoMask != 1)
                {
                    //need to disable the PLL before changing the vco mask
                    SET_SPI_FIELD(pMac, QUASAR_REG_STATE, QUASAR_PLL_EN_MASK, QUASAR_PLL_EN_OFFSET, 0);
                    SET_SPI_FIELD(pMac, QUASAR_REG_PLL3, QUASAR_VCO_EN_MASK, QUASAR_VCO_EN_OFFSET, 1);  //1 = low VCO mask

                    //need to re-enable the PLL now that we've changed the vco mask
                    SET_SPI_FIELD(pMac, QUASAR_REG_STATE, QUASAR_PLL_EN_MASK, QUASAR_PLL_EN_OFFSET, 1);
                    {
                        tANI_U32 pllVregD;  //delay when enabling the PLL

                        GET_SPI_FIELD(pMac, QUASAR_REG_PLL4, QUASAR_PLL_VREG_D_MASK, QUASAR_PLL_VREG_D_OFFSET, &pllVregD);

                        if (pllVregD)
                        {
                            sirBusyWait(500000);   //500us for PLL regulators to stabilize
                        }
                        else
                        {
                            sirBusyWait(250000);   //250us for PLL regulators to stabilize
                        }
                    }
                }
            }
            else if (priorVcoMask != 2)
            {
                //we need to see if the reqd vco mask is different than before, so that we can disable the PLL before changin it.
                {
                    //need to disable the PLL before changing the vco mask
                    SET_SPI_FIELD(pMac, QUASAR_REG_STATE, QUASAR_PLL_EN_MASK, QUASAR_PLL_EN_OFFSET, 0);
                    SET_SPI_FIELD(pMac, QUASAR_REG_PLL3, QUASAR_VCO_EN_MASK, QUASAR_VCO_EN_OFFSET, 2);  //2 = high VCO mask

                    //need to re-enable the PLL now that we've changed the vco mask
                    SET_SPI_FIELD(pMac, QUASAR_REG_STATE, QUASAR_PLL_EN_MASK, QUASAR_PLL_EN_OFFSET, 1);
                    {
                        tANI_U32 pllVregD;  //delay when enabling the PLL

                        GET_SPI_FIELD(pMac, QUASAR_REG_PLL4, QUASAR_PLL_VREG_D_MASK, QUASAR_PLL_VREG_D_OFFSET, &pllVregD);

                        if (pllVregD)
                        {
                            sirBusyWait(500000);   //500us for PLL regulators to stabilize
                        }
                        else
                        {
                            sirBusyWait(250000);   //250us for PLL regulators to stabilize
                        }
                    }
                }
            }
            else
            {
                assert(priorVcoMask < 3); //only allow 1 & 2
            }
        }

        SET_QUASAR_DIVIDERS(ndiv.ndivInt, ndiv.ndivFrac);
        phyLog(pMac, LOG2, "ChanIndex=%d    RDIV=%d  NDIV Int=%d  NDIV Frac=%d\n", chan, rdivPerChannel[chan], ndiv.ndivInt, ndiv.ndivFrac);

    }

    if (pMac->hphy.rf.chanFilterSettings)
    {
        sQuasarFilterSettings *pFilters = (sQuasarFilterSettings *)((tANI_U8 *)pMac->hphy.rf.chanFilterSettings + (chan * sizeof(sQuasarFilterSettings)));

        retVal = rfReadDataField(pMac, QUASAR_REG_CFG_5, MSK_32, 0, &regVal);
        if (retVal != eHAL_STATUS_SUCCESS)
        {
            return (retVal);
        }

        regVal &= ~((QUASAR_RX_IF_TUN_MASK << QUASAR_RX_IF_TUN_OFFSET) | 
                    (QUASAR_TX_IF_TUN_MASK << QUASAR_TX_IF_TUN_OFFSET) | 
                    (QUASAR_TX_RF_TUN_MASK << QUASAR_TX_RF_TUN_OFFSET));

        regVal |= (((pFilters->txIf & QUASAR_TX_IF_TUN_MASK) << QUASAR_TX_IF_TUN_OFFSET)
                    | ((pFilters->txRf & QUASAR_TX_RF_TUN_MASK) << QUASAR_TX_RF_TUN_OFFSET)
                    | ((pFilters->rxIf & QUASAR_RX_IF_TUN_MASK) << QUASAR_RX_IF_TUN_OFFSET));

        SET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_CFG_5, regVal);
    }
    
    pMac->hphy.rf.curChannel = chan;    //record the current channel

    sirBusyWait(500000);  //wait half ms for Quasar to stabilize on a channel

    //sirBusyWait(1000000000); //one-second wait so I can see the channel in my channel sweep test

    return(retVal);
}


eRfChannels rfGetCurChannel(tpAniSirGlobal pMac)
{
    return(pMac->hphy.rf.curChannel);
}



#define CT_MAINT_ITER_LIMIT 100

eHalStatus rfRelockSynth(tpAniSirGlobal pMac)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
#ifdef ONE_SHOT_AUTO_COARSE_TUNING // see Quasar a0 erata 22 regarding our use of Single-shot instead of continuous automatic mode
    {
        tANI_U32 i = 0;
        tANI_U32 ctMaint = 1;


        SET_SPI_FIELD(pMac, QUASAR_REG_SPARES, QUASAR_RX_MAINT_MASK, QUASAR_RX_MAINT_OFFSET, 1);    //we should be able to turn this on at init time, and leave it set - waiting for data from Tracy
        SET_SPI_FIELD(pMac, QUASAR_REG_PLL3, QUASAR_CT_MAINT_MASK, QUASAR_CT_MAINT_OFFSET, 1);

        while (ctMaint && i < CT_MAINT_ITER_LIMIT)
        {
            GET_SPI_FIELD(pMac, QUASAR_REG_PLL3, QUASAR_CT_MAINT_MASK, QUASAR_CT_MAINT_OFFSET, &ctMaint);
            i++;
        }

        if (i == CT_MAINT_ITER_LIMIT)
        {
            phyLog(pMac, LOGE, "ERROR: Unable to relock synth on channel %d\n", rfGetChannelIdFromIndex(rfGetCurChannel(pMac)));
            return (eHAL_STATUS_FAILURE);
        }
    }

#else

#endif

    return (retVal);
}


eHalStatus rfGetSynthLocks(tpAniSirGlobal pMac, eRfSynthLock *retSynthLock)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
    tANI_U32 reg;

    *retSynthLock = SYNTH_LOCK;

#ifdef ONE_SHOT_AUTO_COARSE_TUNING // see Quasar a0 erata 22 regarding our use of Single-shot instead of continuous automatic mode

    GET_SPI_FIELD(pMac, QUASAR_REG_PLL3, QUASAR_PLL_CT_ACTIVE_MASK, QUASAR_PLL_CT_ACTIVE_OFFSET, &reg);
    if (reg)
    {
        phyLog(pMac, LOGE, "ERROR: QUASAR_PLL_CT_ACTIVE should always be 0 when we test it outside of a single-shot maintenance\n");
        *retSynthLock =  SYNTH_UNLOCKED;
    }
#else

#endif
    GET_SPI_FIELD(pMac, QUASAR_REG_PLL4, QUASAR_PLL_TUNE_ERR_MASK, QUASAR_PLL_TUNE_ERR_OFFSET, &reg);
    if (reg)
    {
        phyLog(pMac, LOGE, "ERROR: QUASAR_PLL_TUNE_ERR indicates a problem!\n");
        *retSynthLock =  SYNTH_UNLOCKED;
    }

    return(retVal);
}


tANI_U16 rfChIdToFreqCoversion(tANI_U8 chanNum)
{
    int i;

    for (i = 0; i < NUM_RF_CHANNELS; i++)
    {
        if (rfChannels[i].channelNum == chanNum)
        {
            return rfChannels[i].targetFreq;
        }
    }

    return (0);
}

eRfChannels rfGetChannelIndex(tANI_U8 chanNum, ePhyChanBondState cbState)
{
    int i;
    int max;

    if (cbState == PHY_SINGLE_CHANNEL_CENTERED)
    {
        i = MIN_LEGIT_RF_CHANNEL;
        max = MAX_LEGIT_RF_CHANNEL;
    }
    else
    {
        i = MIN_CHAN_BOND_CHANNEL;
        max = MAX_CHAN_BOND_CHANNEL;

        if (cbState == PHY_DOUBLE_CHANNEL_LOW_PRIMARY)
        {
            //chanNum is the requested lower primary, add two channel numbers
            chanNum += 2; //center frequency 10MHz higher than requested channel number
        }
        else if (cbState == PHY_DOUBLE_CHANNEL_HIGH_PRIMARY)
        {
            //chanNum is the requested higher primary, subtract two channel numbers
            if (chanNum >= 3)
            {
                chanNum -= 2; //center frequency 10MHz lower than requested channel number
            }
            else
            {
                return (INVALID_RF_CHANNEL);
            }
        }
    }

    //linear search through the valid channels
    for (; (i <= max); i++)
    {
        if (rfChannels[i].channelNum == chanNum)
        {
            return ((eRfChannels)i);
        }
    }

    return INVALID_RF_CHANNEL;
}


eRfChannels rfGetIndexFromFreq(tANI_U16 chanFreq, ePhyChanBondState cbState)
{
    int i;
    int max;

    if (cbState == PHY_SINGLE_CHANNEL_CENTERED)
    {
        i = MIN_LEGIT_RF_CHANNEL;
        max = MAX_LEGIT_RF_CHANNEL;
    }
    else
    {
        i = MIN_CHAN_BOND_CHANNEL;
        max = MAX_CHAN_BOND_CHANNEL;
    }

    //linear search through the valid channels
    for (; i <= max; i++)
    {
        if (rfChannels[i].targetFreq == chanFreq)
        {
            return (eRfChannels)i;
        }
    }

    return INVALID_RF_CHANNEL;
}


tANI_U8 rfGetChannelIdFromIndex(eRfChannels chIndex)
{
    //assert(chIndex < NUM_RF_CHANNELS);

    return (rfChannels[chIndex].channelNum);
}



//calibration support functions
eHalStatus rfTakeTemp(tpAniSirGlobal pMac, ePhyTxChains txChain, tANI_U8 nSamples, tTempADCVal *retTemperature)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
    tANI_U32 avgTemp = 0;
    tANI_U32 i;
    //tANI_U32 agcRxOverride;

    if (nSamples == 0)
    {
        *retTemperature = 0;
        assert(0);
        return (eHAL_STATUS_FAILURE);
    }

/*
    #ifdef ANI_PHY_DEBUG
        //make sure we are not in automatic mode when taking a temp reading - this doesn't work.
        {
            tANI_U32 ctEn;
    
            GET_SPI_FIELD(pMac, QUASAR_REG_PLL3, QUASAR_CT_EN_MASK, QUASAR_CT_EN_OFFSET, &ctEn);
    
            if (ctEn)
            {
                //we are in Automatic mode
                phyLog(pMac, LOGE, "ERROR: Can't take temperature in automatic coarse tune mode");
                *retTemperature = 16;   //return some mid-range value
                return (eHAL_STATUS_SUCCESS);
            }
        }
    #endif
    
*/

    // GET_PHY_REG(pMac->hHdd, AGC_RX_OVERRIDE_REG, &agcRxOverride);
    // SET_PHY_REG(pMac->hHdd, AGC_RX_OVERRIDE_REG, );
    
    SET_SPI_FIELD(pMac, QUASAR_REG_SPARES, QUASAR_RX_MAINT_MASK, QUASAR_RX_MAINT_OFFSET, 1);
    SET_SPI_FIELD(pMac, QUASAR_REG_PLL3, QUASAR_CT_MAINT_MASK, QUASAR_CT_MAINT_OFFSET, 1);

    for (i = 0; i < nSamples; i++)
    {
        tANI_U32 temperature;
        tANI_U32 ten;
        tANI_U32 x = 500;

        SET_SPI_FIELD(pMac, QUASAR_REG_CFG_1, QUASAR_TEN_MASK, QUASAR_TEN_OFFSET, 1);
        do
        {
            GET_SPI_FIELD(pMac, QUASAR_REG_CFG_1, QUASAR_TEN_MASK, QUASAR_TEN_OFFSET, &ten);
            x--;
        }while(ten && x);
        
        if (!x)
        {
            phyLog(pMac, LOGE, "ERROR: Temperature reading did not complete!\n");
            return (eHAL_STATUS_SUCCESS);   //return success because we don't want to cause the driver to 
        }

        GET_SPI_FIELD(pMac, QUASAR_REG_CFG_1, QUASAR_TOUT_MASK, QUASAR_TOUT_OFFSET, &temperature);

        phyLog(pMac, LOG2, "temperature reading = %d\n", temperature);
        avgTemp += temperature;
    }
    avgTemp /= nSamples;

    phyLog(pMac, LOG1, "temperature = %d\n", avgTemp);
    *retTemperature = (tTempADCVal)avgTemp;

    // SET_PHY_REG(pMac->hHdd, AGC_RX_OVERRIDE_REG, agcRxOverride);

    SET_SPI_FIELD(pMac, QUASAR_REG_SPARES, QUASAR_RX_MAINT_MASK, QUASAR_RX_MAINT_OFFSET, 0);
    SET_SPI_FIELD(pMac, QUASAR_REG_PLL3, QUASAR_CT_MAINT_MASK, QUASAR_CT_MAINT_OFFSET, 0);
    
    return(retVal);
}


eRfCalMode rfGetCalMode(tpAniSirGlobal pMac)
{
    eRfCalMode calMode = RF_CAL_NORMAL;

    return(calMode);
}


eHalStatus rfSetCalMode(tpAniSirGlobal pMac, eRfCalMode calMode)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;

    return(retVal);
}




eHalStatus rfSetDCOffset(tpAniSirGlobal pMac, ePhyRxChains rxChain, eGainSteps gain, tRxDcoCorrect offset)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
    tANI_U32 value, value_gc2;

    assert (gain < NUM_QUASAR_RX_GAIN_STEPS);

    phyLog(pMac, LOG4, "Set  \tRX%d gain:%d  dcoI=%d dcoQ=%d dcoRange=%d\n", rxChain,
            gain, offset.IDcoCorrect, offset.QDcoCorrect, offset.dcRange
          );

    retVal = rfReadDataField(pMac, QUASAR_REG_GAIN_CONTROL_2, MSK_32, 0, &value_gc2);
    value_gc2 &= ~((QUASAR_SPI_LUT_EN_MASK << QUASAR_SPI_LUT_EN_OFFSET) | (QUASAR_SPI_LUT_INDEX_MASK << QUASAR_SPI_LUT_INDEX_OFFSET));
    SET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_GAIN_CONTROL_2, value_gc2 
                                                | ((eANI_BOOLEAN_ON & QUASAR_SPI_LUT_EN_MASK) << QUASAR_SPI_LUT_EN_OFFSET)
                                                | ((gain & QUASAR_SPI_LUT_INDEX_MASK) << QUASAR_SPI_LUT_INDEX_OFFSET));

    sirBusyWait(1000);  //wait 1 us

    switch (rxChain)
    {
        case PHY_RX_CHAIN_0:
            value = (((offset.dcRange & QUASAR_RX0DC_RANGE_MASK) << QUASAR_RX0DC_RANGE_OFFSET) |
                                ((offset.IDcoCorrect & QUASAR_RX0DCI_MASK) << QUASAR_RX0DCI_OFFSET) |
                                ((offset.QDcoCorrect & QUASAR_RX0DCQ_MASK) << QUASAR_RX0DCQ_OFFSET));
            SET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_DC_OFFSET_CHAIN_0, value);

            break;

        case PHY_RX_CHAIN_1:
            value = (((offset.dcRange & QUASAR_RX1DC_RANGE_MASK) << QUASAR_RX1DC_RANGE_OFFSET) |
                                ((offset.IDcoCorrect & QUASAR_RX1DCI_MASK) << QUASAR_RX1DCI_OFFSET) |
                                ((offset.QDcoCorrect & QUASAR_RX1DCQ_MASK) << QUASAR_RX1DCQ_OFFSET));
            SET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_DC_OFFSET_CHAIN_1, value);

           break;

        case PHY_RX_CHAIN_2:
            value = (((offset.dcRange & QUASAR_RX2DC_RANGE_MASK) << QUASAR_RX2DC_RANGE_OFFSET) |
                    ((offset.IDcoCorrect & QUASAR_RX2DCI_MASK) << QUASAR_RX2DCI_OFFSET) |
                    ((offset.QDcoCorrect & QUASAR_RX2DCQ_MASK) << QUASAR_RX2DCQ_OFFSET));
            SET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_DC_OFFSET_CHAIN_2, value);

            break;

        default:
            assert(0);
            return(eHAL_STATUS_FAILURE);
    }

    SET_SPI_FIELD(pMac, QUASAR_REG_GAIN_CONTROL_2, QUASAR_SPI_LUT_EN_MASK, QUASAR_SPI_LUT_EN_OFFSET, eANI_BOOLEAN_OFF);

    return(retVal);
}


eHalStatus rfGetDCOffset(tpAniSirGlobal pMac, ePhyRxChains rxChain, eGainSteps gain, tRxDcoCorrect *offset)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
    tANI_U32 range, i, q, value;

    assert (gain < NUM_QUASAR_RX_GAIN_STEPS);

    retVal = rfReadDataField(pMac, QUASAR_REG_GAIN_CONTROL_2, MSK_32, 0, &value);
    value &= ~((QUASAR_SPI_LUT_EN_MASK << QUASAR_SPI_LUT_EN_OFFSET) | (QUASAR_SPI_LUT_INDEX_MASK << QUASAR_SPI_LUT_INDEX_OFFSET));
    SET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_GAIN_CONTROL_2, value
                                                | ((eANI_BOOLEAN_ON & QUASAR_SPI_LUT_EN_MASK) << QUASAR_SPI_LUT_EN_OFFSET)
                                                | ((gain & QUASAR_SPI_LUT_INDEX_MASK) << QUASAR_SPI_LUT_INDEX_OFFSET));

    sirBusyWait(1000);  //wait 1 us
    switch (rxChain)
    {
        case PHY_RX_CHAIN_0:
            GET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_DC_OFFSET_CHAIN_0, &value);
            range = (value >> QUASAR_RX0DC_RANGE_OFFSET) & QUASAR_RX0DC_RANGE_MASK;
            i = (value >> QUASAR_RX0DCI_OFFSET) & QUASAR_RX0DCI_MASK;
            q = (value >> QUASAR_RX0DCQ_OFFSET) & QUASAR_RX0DCQ_MASK;

            break;

        case PHY_RX_CHAIN_1:
            GET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_DC_OFFSET_CHAIN_1, &value);
            range = (value >> QUASAR_RX1DC_RANGE_OFFSET) & QUASAR_RX1DC_RANGE_MASK;
            i = (value >> QUASAR_RX1DCI_OFFSET) & QUASAR_RX1DCI_MASK;
            q = (value >> QUASAR_RX1DCQ_OFFSET) & QUASAR_RX1DCQ_MASK;

             break;

        case PHY_RX_CHAIN_2:
            GET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_REG_DC_OFFSET_CHAIN_2, &value);
            range = (value >> QUASAR_RX2DC_RANGE_OFFSET) & QUASAR_RX2DC_RANGE_MASK;
            i = (value >> QUASAR_RX2DCI_OFFSET) & QUASAR_RX2DCI_MASK;
            q = (value >> QUASAR_RX2DCQ_OFFSET) & QUASAR_RX2DCQ_MASK;

            break;

        default:
            assert(0);
            return(eHAL_STATUS_FAILURE);
    }

    SET_SPI_FIELD(pMac, QUASAR_REG_GAIN_CONTROL_2, QUASAR_SPI_LUT_EN_MASK, QUASAR_SPI_LUT_EN_OFFSET, eANI_BOOLEAN_OFF);
    offset->dcRange =       (tANI_U8)range;
    offset->IDcoCorrect =   (tANI_S8)(i << 1) >> 1;  //shift up to align sign bit in MSB
    offset->QDcoCorrect =   (tANI_S8)(q << 1) >> 1;
    
    phyLog(pMac, LOG4, "Get \tRX%d gain:%d  dcoI=%d dcoQ=%d dcoRange=%d\n", rxChain,
            gain, offset->IDcoCorrect, offset->QDcoCorrect, offset->dcRange
          );

    return(retVal);
}



eHalStatus rfAdjustDCO(tpAniSirGlobal pMac, ePhyRxChains rxChain, eGainSteps gain, tIQAdc dco)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
    tANI_U8 corrReal, corrImag;
    tRxDcoCorrect dcoCorrection;


    if ((retVal = rfGetDCOffset(pMac, rxChain, gain, &dcoCorrection)) == eHAL_STATUS_SUCCESS)
    {
        corrReal = (tANI_U8)(((GET_MAG(dco.I) * 1000)/(1024 * pMac->hphy.rf.dcoStep)) + GET_ROUND((GET_MAG(dco.I) * 1000), (1024 * pMac->hphy.rf.dcoStep)));
        corrImag = (tANI_U8)(((GET_MAG(dco.Q) * 1000)/(1024 * pMac->hphy.rf.dcoStep)) + GET_ROUND((GET_MAG(dco.Q) * 1000), (1024 * pMac->hphy.rf.dcoStep)));
        
        dcoCorrection.dcRange = 1;  //133uA only used here

        //the correction values are signed 7-bit, so we need to shift them up one so the compiler performs the adjustment correctly, then shift them back down

        dcoCorrection.IDcoCorrect <<= 1;
        if (dco.I > 0)
        {
            //positive error = positive adjustment
            dcoCorrection.IDcoCorrect += (2 * corrReal);
        }
        else if (dco.I < 0)
        {
            //negative error = negative adjustment
            dcoCorrection.IDcoCorrect -= (2 * corrReal);
        }
        dcoCorrection.IDcoCorrect >>= 1;

        dcoCorrection.QDcoCorrect <<= 1;
        if (dco.Q > 0)
        {
            //positive error = positive adjustment
            dcoCorrection.QDcoCorrect += (2 * corrImag);
        }
        else if (dco.Q < 0)
        {
            //negative error = negative adjustment
            dcoCorrection.QDcoCorrect -= (2 * corrImag);
        }
        dcoCorrection.QDcoCorrect >>= 1;

        retVal = rfSetDCOffset(pMac, rxChain, gain, dcoCorrection);
    }

    return(retVal);
}

eHalStatus rfGetTxLoCorrect(tpAniSirGlobal pMac, ePhyTxChains txChain, tTxLoCorrect *corr)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
    tANI_U32    value;
    if(!(corr != NULL))
    {
        phyLog(pMac, LOGE, "ERROR: NULL pointer for transmit LO correction values.\n");
        return eHAL_STATUS_FAILURE;
    }

    if (txChain == PHY_TX_CHAIN_0)
    {
        retVal = rfReadDataField(pMac, QUASAR_TEST6_REG, MSK_32, 0, &value);
        if (retVal != eHAL_STATUS_SUCCESS)
        {
            return retVal;
        }
        corr->iLo = (tANI_OB5)((value >> QUASAR_TX0_DCOC_I_OFFSET) & QUASAR_TX0_DCOC_I_MASK);
        corr->qLo = (tANI_OB5)((value >> QUASAR_TX0_DCOC_Q_OFFSET) & QUASAR_TX0_DCOC_Q_MASK);
    }
    else if(txChain == PHY_TX_CHAIN_1)
    {
        retVal = rfReadDataField(pMac, QUASAR_SPARES_REG, MSK_32, 0, &value);
        if (retVal != eHAL_STATUS_SUCCESS)
        {
            return retVal;
        }
        corr->iLo = (tANI_OB5)((value >> QUASAR_TX1_DCOC_I_OFFSET) & QUASAR_TX1_DCOC_I_MASK);
        corr->qLo = (tANI_OB5)((value >> QUASAR_TX1_DCOC_Q_OFFSET) & QUASAR_TX1_DCOC_Q_MASK);
    }

    phyLog(pMac, LOG2, "rfGetTxLoCorrect Correction : I = %d, Q = %d\n", corr->iLo, corr->qLo);
    return retVal;
}


eHalStatus rfSetTxLoCorrect(tpAniSirGlobal pMac, ePhyTxChains txChain, tTxLoCorrect corr)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
    tANI_U32    value;

    phyLog(pMac, LOG4, "rfSetTxLoCorrect Correction : I = %d, Q = %d\n", corr.iLo, corr.qLo);
    
    if (txChain == PHY_TX_CHAIN_0)
    {

        retVal = rfReadDataField(pMac, QUASAR_TEST6_REG, MSK_32, 0, &value);
        if (retVal != eHAL_STATUS_SUCCESS)
        {
            return retVal;
        }
        value &= ((QUASAR_ADC_TEST_MASK << QUASAR_ADC_TEST_OFFSET) | QUASAR_ADC_TEST_EN_MASK);
        value |= ((3 << QUASAR_TX0_DCOC_EN_OFFSET) | ((corr.iLo & QUASAR_TX0_DCOC_I_MASK) << QUASAR_TX0_DCOC_I_OFFSET) |
                            ((corr.qLo & QUASAR_TX0_DCOC_Q_MASK) << QUASAR_TX0_DCOC_Q_OFFSET));
        SET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_TEST6_REG, value);
    }
    else if(txChain == PHY_TX_CHAIN_1)
    {

        retVal = rfReadDataField(pMac, QUASAR_SPARES_REG, MSK_32, 0, &value);
        if (retVal != eHAL_STATUS_SUCCESS)
        {
            return retVal;
        }
        value &= ((QUASAR_RX_MAINT_MASK << QUASAR_RX_MAINT_OFFSET) | QUASAR_SPARES_SPARES_MASK);
        value |= (((corr.iLo & QUASAR_TX1_DCOC_I_MASK) << QUASAR_TX1_DCOC_I_OFFSET) |
                            ((corr.qLo & QUASAR_TX1_DCOC_Q_MASK) << QUASAR_TX1_DCOC_Q_OFFSET));
        SET_SPI_REG(pMac, QUASAR_CHIP, QUASAR_SPARES_REG, value);
    }
    return retVal;
}

#ifdef ANI_PHY_DEBUG
eHalStatus dumpQuasarCorrectionValues(tpAniSirGlobal pMac, eInitCals calId)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
    
    switch (calId)
    {
        case TX_LO_CAL_ONLY:
        {
            tANI_U32 value = 0;
            
            if ((retVal = rfReadQuasarField(pMac, QUASAR_FIELD_TX0_DCOC_I, &value)) != eHAL_STATUS_SUCCESS) {  return retVal; }
            phyLog(pMac, LOG2, "CHAIN 0: TX0_DCOC_I = %d", value);
            if ((retVal = rfReadQuasarField(pMac, QUASAR_FIELD_TX0_DCOC_Q, &value)) != eHAL_STATUS_SUCCESS) {  return retVal; }
            phyLog(pMac, LOG2, "CHAIN 0: TX0_DCOC_Q = %d", value);

            if ((retVal = rfReadQuasarField(pMac, QUASAR_FIELD_TX1_DCOC_I, &value)) != eHAL_STATUS_SUCCESS) {  return retVal; }
            phyLog(pMac, LOG2, "CHAIN 1: TX1_DCOC_I = %d", value);
            if ((retVal = rfReadQuasarField(pMac, QUASAR_FIELD_TX1_DCOC_Q, &value)) != eHAL_STATUS_SUCCESS) {  return retVal; }
            phyLog(pMac, LOG2, "CHAIN 1: TX1_DCOC_Q = %d", value);
            break;
        }
        case RX_DCO_CAL_ONLY:
        {
            tRxDcoCorrect   offset;
            
            retVal = rfGetDCOffset(pMac, PHY_RX_CHAIN_0, NUM_QUASAR_RX_GAIN_STEPS - 1, &offset);
            if (retVal != eHAL_STATUS_SUCCESS)
            {
                return retVal;
            }
            
            phyLog(pMac, LOG2, "CHAIN 0: DC OFFSET Range = %d", offset.dcRange);
            phyLog(pMac, LOG2, "CHAIN 0: DC OFFSET I DCO Correct = %d", offset.IDcoCorrect);
            phyLog(pMac, LOG2, "CHAIN 0: DC OFFSET Q DCO Correct = %d", offset.QDcoCorrect);

            retVal = rfGetDCOffset(pMac, PHY_RX_CHAIN_1, NUM_QUASAR_RX_GAIN_STEPS - 1, &offset);
            if (retVal != eHAL_STATUS_SUCCESS)
            {
                return retVal;
            }
            phyLog(pMac, LOG2, "CHAIN 1: DC OFFSET Range = %d", offset.dcRange);
            phyLog(pMac, LOG2, "CHAIN 1: DC OFFSET I DCO Correct = %d", offset.IDcoCorrect);
            phyLog(pMac, LOG2, "CHAIN 1: DC OFFSET Q DCO Correct = %d", offset.QDcoCorrect);

            retVal = rfGetDCOffset(pMac, PHY_RX_CHAIN_2, NUM_QUASAR_RX_GAIN_STEPS - 1, &offset);
            if (retVal != eHAL_STATUS_SUCCESS)
            {
                return retVal;
            }
            phyLog(pMac, LOG2, "CHAIN 2: DC OFFSET Range = %d", offset.dcRange);
            phyLog(pMac, LOG2, "CHAIN 2: DC OFFSET I DCO Correct = %d", offset.IDcoCorrect);
            phyLog(pMac, LOG2, "CHAIN 2: DC OFFSET Q DCO Correct = %d", offset.QDcoCorrect);
            break;

        }
        default:
            phyLog(pMac, LOG2, "Invalid CalID");
    }
    return retVal;        
}
#endif



#ifndef RFGEMINI_H
#define RFGEMINI_H

#include "wlan_bit.h"


/* all enumerations, defines generated with convert_libra_regs.pl */



typedef enum
{
    GEMINI_REG_REV_ID                                      = 0,
    GEMINI_REG_TX_GAIN_CONTROL                             = 1,
    GEMINI_REG_RX_GAIN_CONTROL                             = 2,
    GEMINI_REG_RESET_CONTROL                               = 3,
    GEMINI_REG_MODE_SEL1                                   = 4,
    GEMINI_REG_MODE_SEL2                                   = 5,
    GEMINI_REG_MODE_SEL3                                   = 6,
    GEMINI_REG_TEST_MODE                                   = 7,
    GEMINI_REG_SW_OVERRIDE                                 = 8,
    GEMINI_REG_SPARE_1                                     = 9,
    GEMINI_REG_SPARE_2                                     = 10,
    GEMINI_REG_SPARE_3                                     = 11,
    GEMINI_REG_SPARE_4                                     = 12,
    //missing address between 12 and 32
    GEMINI_REG_XO_REG0                                     = 32,
    GEMINI_REG_BLOCK_ENABLE                                = 33,
    GEMINI_REG_XO_TCXO                                     = 34,
    GEMINI_REG_REFFM_REFDIV                                = 35,
    GEMINI_REG_REFFM                                       = 36,
    GEMINI_REG_CLK_REFFM_REFDIV1                           = 37,
    GEMINI_REG_DC_GPO                                      = 38,
    GEMINI_REG_CLK_REFFM_REFDIV2                           = 39,
    GEMINI_REG_XO_SPARE1                                   = 40,
    //missing address between 40 and 48
    GEMINI_REG_PLL_REG0                                    = 48,
    GEMINI_REG_PLL_REG1                                    = 49,
    GEMINI_REG_PLL_REG2                                    = 50,
    GEMINI_REG_PLL_REG3                                    = 51,
    GEMINI_REG_PLL_REG4                                    = 52,
    GEMINI_REG_PLL_REG5                                    = 53,
    GEMINI_REG_PLL_REG6                                    = 54,
    GEMINI_REG_PLL_REG7                                    = 55,
    GEMINI_REG_PLL_REG8                                    = 56,
    GEMINI_REG_PLL_REG9                                    = 57,
    GEMINI_REG_PLL_REG10                                   = 58,
    GEMINI_REG_PLL_REG11                                   = 59,
    GEMINI_REG_PLL_REG12                                   = 60,
    GEMINI_REG_PLL_REG13                                   = 61,
    GEMINI_REG_PLL_REG14                                   = 62,
    GEMINI_REG_PLL_REG15                                   = 63,
    GEMINI_REG_PLL_REG16                                   = 64,
    GEMINI_REG_PLL_REG17                                   = 65,
    GEMINI_REG_PLL_REG18                                   = 66,
    GEMINI_REG_PLL_REG19                                   = 67,
    GEMINI_REG_PLL_REG20                                   = 68,
    GEMINI_REG_PLL_REG21                                   = 69,
    GEMINI_REG_PLL_REG22                                   = 70,
    GEMINI_REG_PLL_REG23                                   = 71,
    GEMINI_REG_PLL_REG24                                   = 72,
    GEMINI_REG_PLL_REG25                                   = 73,
    GEMINI_REG_PLL_VFC_REG1                                = 74,
    GEMINI_REG_PLL_VFC_REG2                                = 75,
    GEMINI_REG_PLL_VFC_REG4                                = 76,
    GEMINI_REG_PLL_VFC_REG5                                = 77,
    GEMINI_REG_PLL_VFC_REG6                                = 78,
    GEMINI_REG_PLL_VFC_REG7                                = 79,
    GEMINI_REG_PLL_VFC_REG8                                = 80,
    GEMINI_REG_PLL_VB_REG0                                 = 81,
    GEMINI_REG_PLL_VB_REG1                                 = 82,
    GEMINI_REG_PLL_VB_REG2                                 = 83,
    GEMINI_REG_PLL_VB_REG3                                 = 84,
    GEMINI_REG_PLL_VB_REG4                                 = 85,
    GEMINI_REG_PLL_VB_REG5                                 = 86,
    GEMINI_REG_PLL_VB_REG6                                 = 87,
    GEMINI_REG_PLL_VB_REG7                                 = 88,
    GEMINI_REG_PLL_VB_REG8                                 = 89,
    GEMINI_REG_PLL_PC_REG0                                 = 90,
    GEMINI_REG_PLL_PC_REG1                                 = 91,
    GEMINI_REG_PLL_PC_REG2                                 = 92,
    GEMINI_REG_PLL_PC_REG3                                 = 93,
    GEMINI_REG_PLL_PC_REG4                                 = 94,
    GEMINI_REG_PLL_PC_REG5                                 = 95,
    GEMINI_REG_PLL_PC_REG6                                 = 96,
    GEMINI_REG_PLL_AC_REG0                                 = 97,
    GEMINI_REG_PLL_AC_REG1                                 = 98,
    GEMINI_REG_PLL_AC_REG2                                 = 99,
    GEMINI_REG_PLL_AC_REG3                                 = 100,
    GEMINI_REG_PLL_AC_REG4                                 = 101,
    GEMINI_REG_PLL_AC_REG5                                 = 102,
    GEMINI_REG_PLL_AC_REG6                                 = 103,
    GEMINI_REG_PLL_AC_REG7                                 = 104,
    GEMINI_REG_PLL_AC_REG8                                 = 105,
    GEMINI_REG_PLL_BW_REG1                                 = 106,
    GEMINI_REG_PLL_BW_REG2                                 = 107,
    GEMINI_REG_PLL_BW_REG3                                 = 108,
    GEMINI_REG_PLL_BW_REG4                                 = 109,
    GEMINI_REG_PLL_BW_REG5                                 = 110,
    GEMINI_REG_PLL_BW_REG6                                 = 111,
    //missing address between 111 and 144
    GEMINI_REG_RX_GAIN_CTL                                 = 144,
    GEMINI_REG_RX_PWR_CTL                                  = 145,
    GEMINI_REG_RX_LNA_CURRENT                              = 146,
    GEMINI_REG_RX_LNA_LB_LOAD                              = 147,
    GEMINI_REG_RX_LNA_HB_LOAD                              = 148,
    GEMINI_REG_RX_LNA_LB_MATCH                             = 149,
    GEMINI_REG_RX_LNA_HB_MATCH                             = 150,
    GEMINI_REG_RX_LNA_LB_VCM                               = 151,
    GEMINI_REG_RX_LNA_VG                                   = 152,
    GEMINI_REG_RXFE_CTL                                    = 153,
    GEMINI_REG_RXLB_LO_BIAS                                = 154,
    GEMINI_REG_RXHB_LO_BIAS                                = 155,
    GEMINI_REG_RXFE_SPARE1                                 = 156,
    GEMINI_REG_RXFE_SPARE2                                 = 157,
    GEMINI_REG_RXFE_SPARE3                                 = 158,
    GEMINI_REG_RXFE_SPARE4                                 = 159,
    GEMINI_REG_RX_GC_0                                     = 160,
    GEMINI_REG_PA_GC                                       = 161,
    GEMINI_REG_RX_DCOC_IQ                                  = 162,
    GEMINI_REG_TX_DCOC_IQ                                  = 163,
    GEMINI_REG_RX_DCOC_EN0                                 = 164,
    GEMINI_REG_RX_DCOC_EN1                                 = 165,
    GEMINI_REG_RX_DCOC_RANGE0                              = 166,
    GEMINI_REG_RX_DCOC_RANGE1                              = 167,
    GEMINI_REG_RX_DCOC_RANGE2                              = 168,
    GEMINI_REG_RX_DCOC_RANGE3                              = 169,
    GEMINI_REG_TX_DCOC_EN0                                 = 170,
    GEMINI_REG_TX_DCOC_RANGE0                              = 171,
    GEMINI_REG_TX_DCOC_RANGE1                              = 172,
    GEMINI_REG_TX_BBF_GAIN                                 = 173,
    GEMINI_REG_RX_DCOC_CTL                                 = 174,
    GEMINI_REG_BB_BW0                                      = 175,
    GEMINI_REG_BB_BW1                                      = 176,
    GEMINI_REG_BB_BW2                                      = 177,
    GEMINI_REG_BB_BW3                                      = 178,
    GEMINI_REG_BB_BW4                                      = 179,
    GEMINI_REG_BB_BW5                                      = 180,
    GEMINI_REG_BB_BW6                                      = 181,
    GEMINI_REG_BB_INSITU_RCM0                              = 182,
    GEMINI_REG_BB_INSITU_RCM3                              = 183,
    GEMINI_REG_BB_INSITU_RCM4                              = 184,
    GEMINI_REG_BBF_CTL0                                    = 185,
    GEMINI_REG_BBF_CTL1                                    = 186,
    GEMINI_REG_BBF_CTL2                                    = 187,
    GEMINI_REG_BBF_CTL3                                    = 188,
    GEMINI_REG_BBF_CTL4                                    = 189,
    GEMINI_REG_BBF_CTL5                                    = 190,
    GEMINI_REG_BBF_CTL6                                    = 191,
    GEMINI_REG_BBF_CTL7                                    = 192,
    GEMINI_REG_BBF_CTL8                                    = 193,
    GEMINI_REG_BBF_TEST0                                   = 194,
    GEMINI_REG_BBF_TEST1                                   = 195,
    GEMINI_REG_BBF_TEST2                                   = 196,
    GEMINI_REG_BBF_SAT1                                    = 197,
    GEMINI_REG_BBF_SAT2                                    = 198,
    GEMINI_REG_BBF_SAT3                                    = 199,
    GEMINI_REG_BBF_SAT4                                    = 200,
    GEMINI_REG_BBF_SAT5                                    = 201,
    GEMINI_REG_BBF_SAT6                                    = 202,
    GEMINI_REG_HKADC_CFG                                   = 203,
    GEMINI_REG_RX_IM2_I_CFG0                               = 204,
    GEMINI_REG_RX_IM2_I_CFG1                               = 205,
    GEMINI_REG_RX_IM2_Q_CFG0                               = 206,
    GEMINI_REG_RX_IM2_Q_CFG1                               = 207,
    GEMINI_REG_RX_IM2_VCM0                                 = 208,
    GEMINI_REG_RX_IM2_VCM1                                 = 209,
    GEMINI_REG_RX_IM2_EN_CTL                               = 210,
    GEMINI_REG_RX_IM2_RSB                                  = 211,
    GEMINI_REG_RX_IM2_SPARE0                               = 212,
    GEMINI_REG_RX_IM2_SPARE1                               = 213,
    GEMINI_REG_BBF_AUX1                                    = 214,
    GEMINI_REG_BBF_AUX2                                    = 215,
    GEMINI_REG_BB_INSITU_RCM1                              = 216,
    GEMINI_REG_BB_INSITU_RCM2                              = 217,
    //missing address between 217 and 224
    GEMINI_REG_TX_AUX_DCOC0                                = 224,
    GEMINI_REG_TX_AUX_DCOC1                                = 225,
    GEMINI_REG_PDET_CTL                                    = 226,
    GEMINI_REG_PDET_OVRD                                   = 227,
    GEMINI_REG_MIX_OVRD                                    = 228,
    GEMINI_REG_TX_RCTUNE_CTL                               = 229,
    GEMINI_REG_OVRD_CTL                                    = 230,
    GEMINI_REG_TX_SPARE1                                   = 231,
    GEMINI_REG_TX_SPARE2                                   = 232,
    GEMINI_REG_TX_SPARE3                                   = 233,
    GEMINI_REG_TX_SPARE4                                   = 234,
    GEMINI_REG_TX_SPARE5                                   = 235,
    //missing address between 235 and 240
    GEMINI_REG_TRSW_CTL                                    = 240,
    GEMINI_REG_PA_STG1_2_BIAS                              = 241,
    GEMINI_REG_PA_STG3_BIAS                                = 242,
    GEMINI_REG_DA_GAIN_CTL                                 = 243,
    GEMINI_REG_DA_BIAS                                     = 244,
    GEMINI_REG_DA_BALUN_PA_CTL                             = 245,
    GEMINI_REG_TEST_PATH_CTL                               = 246,
    GEMINI_REG_PA_EN                                       = 247,
    GEMINI_REG_HDET_CTL                                    = 248,
    GEMINI_REG_HDET_BIAS                                   = 249,
    GEMINI_REG_HDET_TEST                                   = 250,
    GEMINI_REG_HDET_DCOC                                   = 251,
    GEMINI_REG_CAL_DATA                                    = 252,
    GEMINI_REG_VSWR_CTL0                                   = 253,
    GEMINI_REG_VSWR_CTL1                                   = 254,

    GEMINI_NUM_REGS,
    GEMINI_REGS_MAX_VAL                                    = 0xFFFFFFFF, //dummy added to change enum to 4 bytes
} eGeminiRegs;


typedef struct
{
    tANI_U16 ver;
    tANI_U16 unused;
}tRfChipVer;





typedef enum
{
    GEMINI_FIELD_REV_ID_CHIP_ID,                           // 0
    GEMINI_FIELD_TX_GAIN_CONTROL_TX_GC,                    // 1
    GEMINI_FIELD_RX_GAIN_CONTROL_RX_GC1,                   // 2
    GEMINI_FIELD_RX_GAIN_CONTROL_RX_GC0,                   // 3
    GEMINI_FIELD_RESET_CONTROL_SRST_RTUNE_ON,              // 4
    GEMINI_FIELD_RESET_CONTROL_SRST_WO_RTUNE_ON,           // 5
    GEMINI_FIELD_RESET_CONTROL_SRST_RTUNE,                 // 6
    GEMINI_FIELD_RESET_CONTROL_SRST_WO_RTUNE,              // 7
    GEMINI_FIELD_MODE_SEL1_EXT_PA,                         // 8
    GEMINI_FIELD_MODE_SEL1_PLLEN_FORCE,                    // 9
    GEMINI_FIELD_MODE_SEL1_TXRX_BAND,                      // 10
    GEMINI_FIELD_MODE_SEL1_TXRX_WRITE_ALL,                 // 11
    GEMINI_FIELD_MODE_SEL1_TXRX_REG_SEL,                   // 12
    GEMINI_FIELD_MODE_SEL1_TXRX_CAL_MODE,                  // 13
    GEMINI_FIELD_MODE_SEL1_IQ_DIV_MODE,                    // 14
    GEMINI_FIELD_MODE_SEL1_EN_TXLO_MODE,                   // 15
    GEMINI_FIELD_MODE_SEL1_EN_RXLO_MODE,                   // 16
    GEMINI_FIELD_MODE_SEL2_RTUNER_CODE_BBF,                // 17
    GEMINI_FIELD_MODE_SEL2_RTUNER_CODE_BIAS,               // 18
    GEMINI_FIELD_MODE_SEL2_EN_RTX_BIAS,                    // 19
    GEMINI_FIELD_MODE_SEL2_PTAT_TUNED_BUF_EN,              // 20
    GEMINI_FIELD_MODE_SEL2_PTAT_BUF_EN,                    // 21
    GEMINI_FIELD_MODE_SEL2_BG_TUNED_BUF_EN,                // 22
    GEMINI_FIELD_MODE_SEL2_BG_BUF_EN,                      // 23
    GEMINI_FIELD_MODE_SEL2_PTAT_EN,                        // 24
    GEMINI_FIELD_MODE_SEL2_BG_EN,                          // 25
    GEMINI_FIELD_MODE_SEL3_SEL_DEMUX_BBDACIN,              // 26
    GEMINI_FIELD_MODE_SEL3_OVRD_CAL,                       // 27
    GEMINI_FIELD_MODE_SEL3_TXRX_BIAS_GPO_SEL,              // 28
    GEMINI_FIELD_MODE_SEL3_RFONLY_TESTEN,                  // 29
    GEMINI_FIELD_MODE_SEL3_DIG_DR_STR,                     // 30
    GEMINI_FIELD_MODE_SEL3_GPO_TEST_BLOCK_EN,              // 31
    GEMINI_FIELD_MODE_SEL3_TEST_SIGNAL_SLCT,               // 32
    GEMINI_FIELD_TEST_MODE_SEL_BBI_PN,                     // 33
    GEMINI_FIELD_TEST_MODE_SEL_BBQ_PN,                     // 34
    GEMINI_FIELD_SW_OVERRIDE_DCCAL__ENABLE,                // 35
    GEMINI_FIELD_SW_OVERRIDE_PLL__ENABLE,                  // 36
    GEMINI_FIELD_SW_OVERRIDE_TCXO_ENABLE,                  // 37
    GEMINI_FIELD_SW_OVERRIDE_XO_ENABLE,                    // 38
    GEMINI_FIELD_SW_OVERRIDE_PA_ENABLE,                    // 39
    GEMINI_FIELD_SW_OVERRIDE_DA_ENABLE,                    // 40
    GEMINI_FIELD_SW_OVERRIDE_TR_SW0_ENABLE,                // 41
    GEMINI_FIELD_SW_OVERRIDE_TR_SW1_ENABLE,                // 42
    GEMINI_FIELD_SW_OVERRIDE_TX_ENABLE,                    // 43
    GEMINI_FIELD_SW_OVERRIDE_RX0_ENABLE,                   // 44
    GEMINI_FIELD_SW_OVERRIDE_RX1_ENABLE,                   // 45
    GEMINI_FIELD_SW_OVERRIDE_OVRD_MC,                      // 46
    GEMINI_FIELD_SPARE_1_MC_DTO0_SEL,                      // 47
    GEMINI_FIELD_SPARE_1_MC_DTO1_SEL,                      // 48
    GEMINI_FIELD_SPARE_1_DTO0_SEL,                         // 49
    GEMINI_FIELD_SPARE_1_DTO1_SEL,                         // 50
    GEMINI_FIELD_SPARE_2_DTEST_SEL,                        // 51
    GEMINI_FIELD_SPARE_2_DTEST_GCSEL,                      // 52
    GEMINI_FIELD_SPARE_2_DTO_GPO_SEL,                      // 53
    GEMINI_FIELD_SPARE_2_AVAILABLE,                        // 54
    GEMINI_FIELD_SPARE_3_AVAILABLE,                        // 55
    GEMINI_FIELD_SPARE_4_AVAILABLE,                        // 56
    GEMINI_FIELD_XO_REG0_PLL_BUF_EN,                       // 57
    GEMINI_FIELD_XO_REG0_TCXO_BUF_EN_MASK,                 // 58
    GEMINI_FIELD_XO_REG0_XO_EN_MASK,                       // 59
    GEMINI_FIELD_XO_REG0_XO_EXTBUF_EN,                     // 60
    GEMINI_FIELD_XO_REG0_XO_EXTBUF_DRV,                    // 61
    GEMINI_FIELD_XO_REG0_XO_EXTBUF_CL,                     // 62
    GEMINI_FIELD_XO_REG0_XO_VREG_B_EXT,                    // 63
    GEMINI_FIELD_XO_REG0_REFFM_REG_EN,                     // 64
    GEMINI_FIELD_BLOCK_ENABLE_CLKPATH_SEL,                 // 65
    GEMINI_FIELD_BLOCK_ENABLE_CLK1X_EN,                    // 66
    GEMINI_FIELD_BLOCK_ENABLE_REFFM_EN_RF,                 // 67
    GEMINI_FIELD_BLOCK_ENABLE_REFDIV_SEL,                  // 68
    GEMINI_FIELD_BLOCK_ENABLE_REFDIV_EN,                   // 69
    GEMINI_FIELD_BLOCK_ENABLE_REFFM_EN,                    // 70
    GEMINI_FIELD_BLOCK_ENABLE_REFFM_EN2X,                  // 71
    GEMINI_FIELD_BLOCK_ENABLE_REFFM_EN4X,                  // 72
    GEMINI_FIELD_XO_TCXO_IKNOBPLL,                         // 73
    GEMINI_FIELD_XO_TCXO_IKNOBINP,                         // 74
    GEMINI_FIELD_XO_TCXO_TCXO_CCTRL,                       // 75
    GEMINI_FIELD_XO_TCXO_TCXO_BWSEL,                       // 76
    GEMINI_FIELD_XO_TCXO_XO_GM_TRIM,                       // 77
    GEMINI_FIELD_XO_TCXO_XO_D_CTRIM,                       // 78
    GEMINI_FIELD_XO_TCXO_XO_G_CTRIM,                       // 79
    GEMINI_FIELD_REFFM_REFDIV_REFFM_FASTSW,                // 80
    GEMINI_FIELD_REFFM_REFDIV_REFFM_CCTRL,                 // 81
    GEMINI_FIELD_REFFM_REFDIV_REFFM_BWSEL,                 // 82
    GEMINI_FIELD_REFFM_REFDIV_REFDIV_DIVR,                 // 83
    GEMINI_FIELD_REFFM_REFDIV_REFFM_RFDR,                  // 84
    GEMINI_FIELD_REFFM_REFDIV_IKNOBREFFM,                  // 85
    GEMINI_FIELD_REFFM_REFDIV_REFFM_LOCKBSEL,              // 86
    GEMINI_FIELD_REFFM_REFDIV_REFFM_LOCKB,                 // 87
    GEMINI_FIELD_REFFM_REFDIV_REFFM_LOCKA,                 // 88
    GEMINI_FIELD_REFFM_REFDIV_CLK_FREQ,                    // 89
    GEMINI_FIELD_REFFM_REFFM_DELAY2,                       // 90
    GEMINI_FIELD_REFFM_REFFM_DELAY1,                       // 91
    GEMINI_FIELD_REFFM_REFFM_S4X,                          // 92
    GEMINI_FIELD_CLK_REFFM_REFDIV1_CML2CMOS_REG_EN,        // 93
    GEMINI_FIELD_CLK_REFFM_REFDIV1_REFDIV_RST,             // 94
    GEMINI_FIELD_CLK_REFFM_REFDIV1_REFFM_RST,              // 95
    GEMINI_FIELD_CLK_REFFM_REFDIV1_REFFM_EN_PFD,           // 96
    GEMINI_FIELD_CLK_REFFM_REFDIV1_REFFM_EN_CPBOOST,       // 97
    GEMINI_FIELD_CLK_REFFM_REFDIV1_XO_DTO_MODE,            // 98
    GEMINI_FIELD_DC_GPO_NREFFM2_EN_PROBE,                  // 99
    GEMINI_FIELD_DC_GPO_NREFFM1_EN_PROBE,                  // 100
    GEMINI_FIELD_DC_GPO_NTCXO_EN_PROBE,                    // 101
    GEMINI_FIELD_DC_GPO_NXO_EN_PROBE,                      // 102
    GEMINI_FIELD_DC_GPO_NBIAS_EN_PROBE,                    // 103
    GEMINI_FIELD_DC_GPO_REFFM_VREG_EN_PROBE,               // 104
    GEMINI_FIELD_DC_GPO_XO_VREG_EN_PROBE,                  // 105
    GEMINI_FIELD_DC_GPO_TCXO_VREG_EN_PROBE,                // 106
    GEMINI_FIELD_DC_GPO_CML_VREG_EN_PROBE,                 // 107
    GEMINI_FIELD_DC_GPO_XO_VREG_PRB_EXT,                   // 108
    GEMINI_FIELD_CLK_REFFM_REFDIV2_DTOP_BUF_EN,            // 109
    GEMINI_FIELD_CLK_REFFM_REFDIV2_EN_DCLK,                // 110
    GEMINI_FIELD_CLK_REFFM_REFDIV2_EN_PLL_DCLK,            // 111
    GEMINI_FIELD_CLK_REFFM_REFDIV2_EN_PA_DCLK,             // 112
    GEMINI_FIELD_CLK_REFFM_REFDIV2_EN_RCM0_DCLK,           // 113
    GEMINI_FIELD_CLK_REFFM_REFDIV2_EN_RCM1_DCLK,           // 114
    GEMINI_FIELD_XO_SPARE1_XO_SPARE_1,                     // 115
    GEMINI_FIELD_PLL_REG_NFRAC,                            // 116
    GEMINI_FIELD_PLL_REG2_NB_6,                            // 117
    GEMINI_FIELD_PLL_REG3_PLL_READY,                       // 118
    GEMINI_FIELD_PLL_REG3_FREE,                            // 119
    GEMINI_FIELD_PLL_REG3_NBI_9_6,                         // 120
    GEMINI_FIELD_PLL_REG4_NB_5_0,                          // 121
    GEMINI_FIELD_PLL_REG_NA,                               // 122
    GEMINI_FIELD_PLL_REG5_BS_EN,                           // 123
    GEMINI_FIELD_PLL_REG5_SDMOGAIN,                        // 124
    GEMINI_FIELD_PLL_REG5_RDIV,                            // 125
    GEMINI_FIELD_PLL_REG_NF_OFFSET,                        // 126
    GEMINI_FIELD_PLL_REG8_BSSTART,                         // 127
    GEMINI_FIELD_PLL_REG8_BSMODE,                          // 128
    GEMINI_FIELD_PLL_REG8_BSBANK,                          // 129
    GEMINI_FIELD_PLL_REG8_BKSHFT,                          // 130
    GEMINI_FIELD_PLL_REG9_BSWAIT,                          // 131
    GEMINI_FIELD_PLL_REG9_BSSAMPLE,                        // 132
    GEMINI_FIELD_PLL_REG9_BSSETTIME,                       // 133
    GEMINI_FIELD_PLL_REG10_SDSTRT_DIS,                     // 134
    GEMINI_FIELD_PLL_REG10_SD_TESTEN,                      // 135
    GEMINI_FIELD_PLL_REG10_NF_LSB_SEL,                     // 136
    GEMINI_FIELD_PLL_REG10_NF_LSBB,                        // 137
    GEMINI_FIELD_PLL_REG10_SDM_SEL,                        // 138
    GEMINI_FIELD_PLL_REG10_SDM_RESET,                      // 139
    GEMINI_FIELD_PLL_REG10_SDM_EN,                         // 140
    GEMINI_FIELD_PLL_REG11_RSVD,                           // 141
    GEMINI_FIELD_PLL_REG11_SYNON_MAN,                      // 142
    GEMINI_FIELD_PLL_REG11_PRES_SEL,                       // 143
    GEMINI_FIELD_PLL_REG11_PLLMD,                          // 144
    GEMINI_FIELD_PLL_REG11_BSTESTEN,                       // 145
    GEMINI_FIELD_PLL_REG11_PLLRST,                         // 146
    GEMINI_FIELD_PLL_REG11_BSCLK_MUX_SEL,                  // 147
    GEMINI_FIELD_PLL_REG12_CPCTL,                          // 148
    GEMINI_FIELD_PLL_REG12_VCONFRCEN,                      // 149
    GEMINI_FIELD_PLL_REG12_VCONSTATE,                      // 150
    GEMINI_FIELD_PLL_REG13_ABW_MODE,                       // 151
    GEMINI_FIELD_PLL_REG13_ABW_TARGETCP_SEL,               // 152
    GEMINI_FIELD_PLL_REG13_ABW_TARGETR2_SEL,               // 153
    GEMINI_FIELD_PLL_REG13_ABW_TMASK,                      // 154
    GEMINI_FIELD_PLL_REG14_ABW_TIME,                       // 155
    GEMINI_FIELD_PLL_REG15_LIN_STEP_CP,                    // 156
    GEMINI_FIELD_PLL_REG15_START_CPGAIN,                   // 157
    GEMINI_FIELD_PLL_REG16_LIN_STEP_R2,                    // 158
    GEMINI_FIELD_PLL_REG16_START_R2,                       // 159
    GEMINI_FIELD_PLL_REG17_FREE,                           // 160
    GEMINI_FIELD_PLL_REG_DTEST,                            // 161
    GEMINI_FIELD_PLL_REG_TSTCNT,                           // 162
    GEMINI_FIELD_PLL_REG20_IVCOBK,                         // 163
    GEMINI_FIELD_PLL_REG21_FVCOBK,                         // 164
    GEMINI_FIELD_PLL_REG_BSTARGET,                         // 165
    GEMINI_FIELD_PLL_REG_BSCOUNT,                          // 166
    GEMINI_FIELD_PLL_VFC_REG1_VFC_START,                   // 167
    GEMINI_FIELD_PLL_VFC_REG1_VFC_TIME,                    // 168
    GEMINI_FIELD_PLL_VFC_REG1_VFC_CAL_MASK,                // 169
    GEMINI_FIELD_PLL_VFC_REG2_VFC_B0_SEL,                  // 170
    GEMINI_FIELD_PLL_VFC_REG2_VFC_B0_MCODE,                // 171
    GEMINI_FIELD_PLL_VFC_REG2_VFC_B0_MODE,                 // 172
    GEMINI_FIELD_PLL_VFC_REG2_VFC_B0_RES,                  // 173
    GEMINI_FIELD_PLL_VFC_REG2_VFC_B1_SEL,                  // 174
    GEMINI_FIELD_PLL_VFC_REG2_VFC_B1_MCODE,                // 175
    GEMINI_FIELD_PLL_VFC_REG2_VFC_B1_MODE,                 // 176
    GEMINI_FIELD_PLL_VFC_REG2_VFC_B1_RES,                  // 177
    GEMINI_FIELD_PLL_VFC_REG4_VFC_B2_SEL,                  // 178
    GEMINI_FIELD_PLL_VFC_REG4_VFC_B2_MCODE,                // 179
    GEMINI_FIELD_PLL_VFC_REG4_VFC_B2_MODE,                 // 180
    GEMINI_FIELD_PLL_VFC_REG4_VFC_B2_RES,                  // 181
    GEMINI_FIELD_PLL_VFC_REG4_VFC_B3_SEL,                  // 182
    GEMINI_FIELD_PLL_VFC_REG4_VFC_B3_MCODE,                // 183
    GEMINI_FIELD_PLL_VFC_REG4_VFC_B3_MODE,                 // 184
    GEMINI_FIELD_PLL_VFC_REG4_VFC_B3_RES,                  // 185
    GEMINI_FIELD_PLL_VFC_REG5_VFC_B0_ACODE,                // 186
    GEMINI_FIELD_PLL_VFC_REG5_VFC_B1_ACODE,                // 187
    GEMINI_FIELD_PLL_VFC_REG5_VFC_B2_ACODE,                // 188
    GEMINI_FIELD_PLL_VFC_REG5_VFC_B3_ACODE,                // 189
    GEMINI_FIELD_PLL_VFC_REG6_VFC_CBANK,                   // 190
    GEMINI_FIELD_PLL_VFC_REG7_VFC_CBANK_P1,                // 191
    GEMINI_FIELD_PLL_VFC_REG8_VFC_CBANK_M1,                // 192
    GEMINI_FIELD_PLL_VB_REG0_BIST_START,                   // 193
    GEMINI_FIELD_PLL_VB_REG0_BIST_TIME,                    // 194
    GEMINI_FIELD_PLL_VB_REG0_BIST_WAIT,                    // 195
    GEMINI_FIELD_PLL_VB_REG0_EN_MIDVGEN_TC,                // 196
    GEMINI_FIELD_PLL_VB_REG0_B0_ONLY,                      // 197
    GEMINI_FIELD_PLL_VB_REG0_VMID_CTRL,                    // 198
    GEMINI_FIELD_PLL_VB_REG0_BIST_LOW_VMID,                // 199
    GEMINI_FIELD_PLL_VB_REG0_BIST_HIGH_VMID,               // 200
    GEMINI_FIELD_PLL_VB_REG1_VB_SPARE_0,                   // 201
    GEMINI_FIELD_PLL_VB_REG2_BIST_OVERLAP,                 // 202
    GEMINI_FIELD_PLL_VB_REG2_BIST_MONO,                    // 203
    GEMINI_FIELD_PLL_VB_REG3_OVERLAP_CNT_DIFF,             // 204
    GEMINI_FIELD_PLL_VB_REG4_OVERLAP_CNT,                  // 205
    GEMINI_FIELD_PLL_VB_REG5_MONO_CNT_DIFF,                // 206
    GEMINI_FIELD_PLL_VB_REG6_MONO_CNT,                     // 207
    GEMINI_FIELD_PLL_VB_REG7_B0_MID_CNT,                   // 208
    GEMINI_FIELD_PLL_VB_REG8_BLAST_MID_CNT,                // 209
    GEMINI_FIELD_PLL_PC_REG0_FASTCH2,                      // 210
    GEMINI_FIELD_PLL_PC_REG0_FASTCH1,                      // 211
    GEMINI_FIELD_PLL_PC_REG0_VR_VCO_SEL,                   // 212
    GEMINI_FIELD_PLL_PC_REG0_EN_VCO_VR_2V,                 // 213
    GEMINI_FIELD_PLL_PC_REG0_EN_VCOCORE_2V,                // 214
    GEMINI_FIELD_PLL_PC_REG0_EN_VCODIV_2V,                 // 215
    GEMINI_FIELD_PLL_PC_REG0_EN_VCODIV,                    // 216
    GEMINI_FIELD_PLL_PC_REG1_EN_PSDIV_2V,                  // 217
    GEMINI_FIELD_PLL_PC_REG1_EN_PSDIV,                     // 218
    GEMINI_FIELD_PLL_PC_REG1_EN_PRNDIV_2V,                 // 219
    GEMINI_FIELD_PLL_PC_REG1_EN_PRNDIV,                    // 220
    GEMINI_FIELD_PLL_PC_REG1_EN_NDIV_S2D,                  // 221
    GEMINI_FIELD_PLL_PC_REG1_EN_BIAS,                      // 222
    GEMINI_FIELD_PLL_PC_REG1_EN_REF,                       // 223
    GEMINI_FIELD_PLL_PC_REG1_EN_PFDCP,                     // 224
    GEMINI_FIELD_PLL_PC_REG2_EN_TX_VCOBUF_2V,              // 225
    GEMINI_FIELD_PLL_PC_REG2_EN_TX_LO,                     // 226
    GEMINI_FIELD_PLL_PC_REG2_EN_TXCAL_VCOBUF_2V,           // 227
    GEMINI_FIELD_PLL_PC_REG2_EN_TXCAL_LO,                  // 228
    GEMINI_FIELD_PLL_PC_REG2_EN_RX_VCOBUF0_2V,             // 229
    GEMINI_FIELD_PLL_PC_REG2_EN_RX_LO0,                    // 230
    GEMINI_FIELD_PLL_PC_REG2_EN_RX_VCOBUF1_2V,             // 231
    GEMINI_FIELD_PLL_PC_REG2_EN_RX_LO1,                    // 232
    GEMINI_FIELD_PLL_PC_REG3_EN_BSCLK_MUX_2V,              // 233
    GEMINI_FIELD_PLL_PC_REG3_EN_BSCLK_MUX,                 // 234
    GEMINI_FIELD_PLL_PC_REG3_EN_SPARE_0,                   // 235
    GEMINI_FIELD_PLL_PC_REG3_EN_SPARE_1,                   // 236
    GEMINI_FIELD_PLL_PC_REG3_EN_SPARE_2,                   // 237
    GEMINI_FIELD_PLL_PC_REG3_EN_SPARE_3,                   // 238
    GEMINI_FIELD_PLL_PC_REG3_EN_SPARE_4,                   // 239
    GEMINI_FIELD_PLL_PC_REG3_EN_SPARE_5,                   // 240
    GEMINI_FIELD_PLL_PC_REG4_LO_ALWAYS_ON,                 // 241
    GEMINI_FIELD_PLL_PC_REG4_CP_MUX,                       // 242
    GEMINI_FIELD_PLL_PC_REG4_CP_GAIN,                      // 243
    GEMINI_FIELD_PLL_PC_REG_C1_SEL,                        // 244
    GEMINI_FIELD_PLL_PC_REG_R2_SEL,                        // 245
    GEMINI_FIELD_PLL_PC_REG_C3_SEL,                        // 246
    GEMINI_FIELD_PLL_PC_REG_R3_SEL,                        // 247
    GEMINI_FIELD_PLL_PC_REG5_EN_R2_BYP,                    // 248
    GEMINI_FIELD_PLL_PC_REG5_EN_R2_RTUNE,                  // 249
    GEMINI_FIELD_PLL_PC_REG6_PC_SPARE0,                    // 250
    GEMINI_FIELD_PLL_AC_REG0_VCODIV_RCTL,                  // 251
    GEMINI_FIELD_PLL_AC_REG0_VCODIV_CCTL,                  // 252
    GEMINI_FIELD_PLL_AC_REG0_PSDIV_RCTL,                   // 253
    GEMINI_FIELD_PLL_AC_REG0_PSDIV_CCTL,                   // 254
    GEMINI_FIELD_PLL_AC_REG1_TXLO_GMCTRL,                  // 255
    GEMINI_FIELD_PLL_AC_REG1_TXLOCAL_GMCTRL,               // 256
    GEMINI_FIELD_PLL_AC_REG1_RXLO0_GMCTRL,                 // 257
    GEMINI_FIELD_PLL_AC_REG1_RXLO1_GMCTRL,                 // 258
    GEMINI_FIELD_PLL_AC_REG2_TSTLO_GMCTRL,                 // 259
    GEMINI_FIELD_PLL_AC_REG2_EXTLO_GMCTRL,                 // 260
    GEMINI_FIELD_PLL_AC_REG2_EN_EXTLO,                     // 261
    GEMINI_FIELD_PLL_AC_REG2_EN_EXTLO_TX,                  // 262
    GEMINI_FIELD_PLL_AC_REG2_SEL_EXTLO_TXCAL,              // 263
    GEMINI_FIELD_PLL_AC_REG2_EN_EXTLO_RX0,                 // 264
    GEMINI_FIELD_PLL_AC_REG2_EN_EXTLO_RX1,                 // 265
    GEMINI_FIELD_PLL_AC_REG2_EN_EXTLO_TST,                 // 266
    GEMINI_FIELD_PLL_AC_REG2_EN_EXTLO_PDIV,                // 267
    GEMINI_FIELD_PLL_AC_REG2_SEL_EXTLO_PSDIV,              // 268
    GEMINI_FIELD_PLL_AC_REG3_CPBIAS_CTRL,                  // 269
    GEMINI_FIELD_PLL_AC_REG3_CP_UPPW,                      // 270
    GEMINI_FIELD_PLL_AC_REG3_CP_DNPW,                      // 271
    GEMINI_FIELD_PLL_AC_REG_CP_LEAK,                       // 272
    GEMINI_FIELD_PLL_AC_REG3_SEL_CPRST,                    // 273
    GEMINI_FIELD_PLL_AC_REG3_CTUNE_VAL,                    // 274
    GEMINI_FIELD_PLL_AC_REG4_EN_NDIV_S2DPWE,               // 275
    GEMINI_FIELD_PLL_AC_REG4_EN_PCHG,                      // 276
    GEMINI_FIELD_PLL_AC_REG4_EN_PCHGHALF,                  // 277
    GEMINI_FIELD_PLL_AC_REG4_EN_EXTVDD,                    // 278
    GEMINI_FIELD_PLL_AC_REG4_EN_VCONTEMP,                  // 279
    GEMINI_FIELD_PLL_AC_REG4_VCON_TEMP_CTRL,               // 280
    GEMINI_FIELD_PLL_AC_REG4_EN_VCO_TST,                   // 281
    GEMINI_FIELD_PLL_AC_REG4_EN_TSTLO_2V,                  // 282
    GEMINI_FIELD_PLL_AC_REG4_EN_TSTLO,                     // 283
    GEMINI_FIELD_PLL_AC_REG4_EN_REF_BWCAP,                 // 284
    GEMINI_FIELD_PLL_AC_REG4_SEL_VCO_TCXO,                 // 285
    GEMINI_FIELD_PLL_AC_REG_PLL_ATEST,                     // 286
    GEMINI_FIELD_PLL_AC_REG6_BSCLKDIV_RCTL,                // 287
    GEMINI_FIELD_PLL_AC_REG6_BSCLKDIV_CCTL,                // 288
    GEMINI_FIELD_PLL_AC_REG6_TXCALDIV_RCTL,                // 289
    GEMINI_FIELD_PLL_AC_REG6_TXCALDIV_CCTL,                // 290
    GEMINI_FIELD_PLL_BW_REG1_LBWC_START,                   // 291
    GEMINI_FIELD_PLL_BW_REG1_LBWC_MODE,                    // 292
    GEMINI_FIELD_PLL_BW_REG1_CP_WIDTH,                     // 293
    GEMINI_FIELD_PLL_BW_REG1_CP_SHFT,                      // 294
    GEMINI_FIELD_PLL_BW_REG2_DELTA_NDIV,                   // 295
    GEMINI_FIELD_PLL_BW_REG2_LBWC_SAMPLE,                  // 296
    GEMINI_FIELD_PLL_BW_REG2_LBWC_DELTA,                   // 297
    GEMINI_FIELD_PLL_BW_REG2_LBWC_SETTIME,                 // 298
    GEMINI_FIELD_PLL_BW_REG_LBWC_TGOAL,                    // 299
    GEMINI_FIELD_PLL_BW_REG5_LBWC_CAL_CP,                  // 300
    GEMINI_FIELD_PLL_BW_REG_LBWC_COUNT,                    // 301
    GEMINI_FIELD_RX_GAIN_CTL_DCSHUNT_EN,                   // 302
    GEMINI_FIELD_RX_GAIN_CTL_BAND_SEL,                     // 303
    GEMINI_FIELD_RX_GAIN_CTL_RXFE_GC_OVERRIDE,             // 304
    GEMINI_FIELD_RX_GAIN_CTL_RX_LNA_GAIN_CONTROL,          // 305
    GEMINI_FIELD_RX_GAIN_CTL_RX_MIX_GAIN_CONTROL,          // 306
    GEMINI_FIELD_RX_PWR_CTL_LNADUM_EN,                     // 307
    GEMINI_FIELD_RX_PWR_CTL_LNASHUNT_EN,                   // 308
    GEMINI_FIELD_RX_PWR_CTL_DCCAL_GAIN,                    // 309
    GEMINI_FIELD_RX_PWR_CTL_EN_RX_MX,                      // 310
    GEMINI_FIELD_RX_PWR_CTL_EN_RX_LNA,                     // 311
    GEMINI_FIELD_RX_LNA_CURRENT_RXHB_IREF_TRIM,            // 312
    GEMINI_FIELD_RX_LNA_CURRENT_RXLB_IREF_TRIM,            // 313
    GEMINI_FIELD_RX_LNA_LB_LOAD_RXLB_LNA_LOAD_CTRIM,       // 314
    GEMINI_FIELD_RX_LNA_LB_LOAD_RXLB_LNA_LOAD_RTRIM,       // 315
    GEMINI_FIELD_RX_LNA_HB_LOAD_RXHB_LNA_LOAD_CTRIM,       // 316
    GEMINI_FIELD_RX_LNA_HB_LOAD_RXHB_LNA_LOAD_RTRIM,       // 317
    GEMINI_FIELD_RX_LNA_LB_MATCH_RXLB_LNA_MATCH_CTRIM,     // 318
    GEMINI_FIELD_RX_LNA_LB_MATCH_RXLB_LNA_MATCH_LTRIM,     // 319
    GEMINI_FIELD_RX_LNA_HB_MATCH_RXHB_LNA_MATCH_CTRIM,     // 320
    GEMINI_FIELD_RX_LNA_HB_MATCH_RXHB_LNA_MATCH_LTRIM,     // 321
    GEMINI_FIELD_RX_LNA_LB_VCM_RXFE_VCM_TRIM,              // 322
    GEMINI_FIELD_RX_LNA_VG_RXHB_VG2_TRIM,                  // 323
    GEMINI_FIELD_RX_LNA_VG_RXLB_VG2_TRIM,                  // 324
    GEMINI_FIELD_RXFE_CTL_EN_HB_VG,                        // 325
    GEMINI_FIELD_RXFE_CTL_EN_LB_VG,                        // 326
    GEMINI_FIELD_RXFE_CTL_EN_LB_VCM,                       // 327
    GEMINI_FIELD_RXFE_CTL_LOOPBACK_EN,                     // 328
    GEMINI_FIELD_RXFE_CTL_LOW_LIN_EN,                      // 329
    GEMINI_FIELD_RXFE_CTL_IREF_SEL,                        // 330
    GEMINI_FIELD_RXFE_CTL_STAB_EN,                         // 331
    GEMINI_FIELD_RXLB_LO_BIAS_RXLB_LO_BCTRL,               // 332
    GEMINI_FIELD_RXLB_LO_BIAS_RXLB_LO_ICTRL,               // 333
    GEMINI_FIELD_RXLB_LO_BIAS_EN_RXLB_LOBUF,               // 334
    GEMINI_FIELD_RXHB_LO_BIAS_RXHB_LO_BCTRL,               // 335
    GEMINI_FIELD_RXHB_LO_BIAS_RXHB_LO_ICTRL,               // 336
    GEMINI_FIELD_RXHB_LO_BIAS_EN_RXHB_LOBUF,               // 337
    GEMINI_FIELD_RX_GC_0_RXFE_GAIN,                        // 338
    GEMINI_FIELD_RX_GC_0_BBF_TIA_GAIN_RX,                  // 339
    GEMINI_FIELD_RX_GC_0_BBF_BQ1_GAIN_RX,                  // 340
    GEMINI_FIELD_RX_GC_0_BBF_BQ2_GAIN_RX,                  // 341
    GEMINI_FIELD_RX_GC_0_BBF_PGA_GAIN_RX,                  // 342
    GEMINI_FIELD_PA_GC_PA_GC,                              // 343
    GEMINI_FIELD_PA_GC_IB_VTOI,                            // 344
    GEMINI_FIELD_PA_GC_G_VTOI,                             // 345
    GEMINI_FIELD_PA_GC_BBF_PGA_GAIN_TX,                    // 346
    GEMINI_FIELD_RX_DCOC_IQ_RX_DCOC_I,                     // 347
    GEMINI_FIELD_RX_DCOC_IQ_RX_DCOC_Q,                     // 348
    GEMINI_FIELD_TX_DCOC_IQ_TX_DCOC_I,                     // 349
    GEMINI_FIELD_TX_DCOC_IQ_TX_DCOC_Q,                     // 350
    GEMINI_FIELD_RX_DCOC_EN_32_INDICES,                    // 351
    GEMINI_FIELD_RX_DCOC_RANGE_SETTING,                    // 352
    GEMINI_FIELD_TX_DCOC_EN_16_INDICES,                    // 353
    GEMINI_FIELD_TX_DCOC_RANGE_SETTING,                    // 354
    GEMINI_FIELD_TX_BBF_GAIN_BBF_TIA_GAIN_TX,              // 355
    GEMINI_FIELD_TX_BBF_GAIN_BBF_BQ1_GAIN_TX,              // 356
    GEMINI_FIELD_TX_BBF_GAIN_BBF_BQ2_GAIN_TX,              // 357
    GEMINI_FIELD_RX_DCOC_CTL_RX_DCOC_IDX_SEL,              // 358
    GEMINI_FIELD_RX_DCOC_CTL_RX_DCOC_IDX_OVER,             // 359
    GEMINI_FIELD_RX_DCOC_CTL_RX_DCOC_INDEX,                // 360
    GEMINI_FIELD_BB_BW0_CM_TIA_TEST,                       // 361
    GEMINI_FIELD_BB_BW0_CM_TIA,                            // 362
    GEMINI_FIELD_BB_BW0_CM_BQ,                             // 363
    GEMINI_FIELD_BB_BW0_RT_CODE_TEST,                      // 364
    GEMINI_FIELD_BB_BW0_RT_CODE,                           // 365
    GEMINI_FIELD_BB_BW1_CT_TIA_8K_I,                       // 366
    GEMINI_FIELD_BB_BW2_CT_TIA_4K_I,                       // 367
    GEMINI_FIELD_BB_BW3_CT_BQ_I,                           // 368
    GEMINI_FIELD_BB_BW4_CT_TIA_8K_Q,                       // 369
    GEMINI_FIELD_BB_BW5_CT_TIA_4K_Q,                       // 370
    GEMINI_FIELD_BB_BW6_CT_BQ_Q,                           // 371
    GEMINI_FIELD_BB_INSITU_RCM0_INSITU_EN_I,               // 372
    GEMINI_FIELD_BB_INSITU_RCM0_INSITU_EN_Q,               // 373
    GEMINI_FIELD_BB_INSITU_RCM0_INSITU_INIT_I,             // 374
    GEMINI_FIELD_BB_INSITU_RCM0_INSITU_INIT_Q,             // 375
    GEMINI_FIELD_BB_INSITU_RCM3_INSITU_RCM_START,          // 376
    GEMINI_FIELD_BB_INSITU_RCM3_INSITU_RCM_COUNT,          // 377
    GEMINI_FIELD_BB_INSITU_RCM4_INSITU_RCM_WARMUP,         // 378
    GEMINI_FIELD_BB_INSITU_RCM4_INSITU_RCM_OSCILLATOR,     // 379
    GEMINI_FIELD_BB_INSITU_RCM4_INSITU_RCM_GPO,            // 380
    GEMINI_FIELD_BB_INSITU_RCM4_RCM_CAPACITOR,             // 381
    GEMINI_FIELD_BBF_CTL0_BBF_EN_I,                        // 382
    GEMINI_FIELD_BBF_CTL0_BBF_EN_Q,                        // 383
    GEMINI_FIELD_BBF_CTL1_BBF_HIZ_EN,                      // 384
    GEMINI_FIELD_BBF_CTL2_RX_MOD_INV,                      // 385
    GEMINI_FIELD_BBF_CTL2_TX_MOD_INV,                      // 386
    GEMINI_FIELD_BBF_CTL2_TX_DAC_DIV4_EN,                  // 387
    GEMINI_FIELD_BBF_CTL2_BBF_BIAS_MODE,                   // 388
    GEMINI_FIELD_BBF_CTL3_TIA_ICQ,                         // 389
    GEMINI_FIELD_BBF_CTL3_TIA_IBIAS,                       // 390
    GEMINI_FIELD_BBF_CTL3_BQ_ICQ,                          // 391
    GEMINI_FIELD_BBF_CTL3_BQ_IBIAS,                        // 392
    GEMINI_FIELD_BBF_CTL4_PGA_ICQ,                         // 393
    GEMINI_FIELD_BBF_CTL4_PGA_IBIAS_RX,                    // 394
    GEMINI_FIELD_BBF_CTL4_PGA_IBIAS_TX,                    // 395
    GEMINI_FIELD_BBF_CTL5_TIA_OTAC,                        // 396
    GEMINI_FIELD_BBF_CTL6_BQ_RCOMP,                        // 397
    GEMINI_FIELD_BBF_CTL6_BQ_OTAC,                         // 398
    GEMINI_FIELD_BBF_CTL7_RX_CAL_EN_OVERRIDE,              // 399
    GEMINI_FIELD_BBF_CTL7_PGA_ACC,                         // 400
    GEMINI_FIELD_BBF_CTL7_PGA_OTAC_TX,                     // 401
    GEMINI_FIELD_BBF_CTL8_RX_CAL_EN,                       // 402
    GEMINI_FIELD_BBF_CTL8_RX_OUT_EN_OVERRIDE,              // 403
    GEMINI_FIELD_BBF_CTL8_RX_OUT_EN,                       // 404
    GEMINI_FIELD_BBF_CTL8_PGA_OTAC_RX,                     // 405
    GEMINI_FIELD_BBF_TEST0_TEST_EN_I,                      // 406
    GEMINI_FIELD_BBF_TEST0_BYPASS_EN_I,                    // 407
    GEMINI_FIELD_BBF_TEST1_TEST_EN_Q,                      // 408
    GEMINI_FIELD_BBF_TEST1_BYPASS_EN_Q,                    // 409
    GEMINI_FIELD_BBF_TEST2_GPO_SW,                         // 410
    GEMINI_FIELD_BBF_SAT1_DET_IN_CHOICE,                   // 411
    GEMINI_FIELD_BBF_SAT1_SAT_AMP_GAIN,                    // 412
    GEMINI_FIELD_BBF_SAT1_SAT_THRES_GAIN,                  // 413
    GEMINI_FIELD_BBF_SAT1_MAN_SAT,                         // 414
    GEMINI_FIELD_BBF_SAT2_SAT_DET_EN,                      // 415
    GEMINI_FIELD_BBF_SAT2_SAT_GPO,                         // 416
    GEMINI_FIELD_BBF_SAT2_SAT_OUT_MUX,                     // 417
    GEMINI_FIELD_BBF_SAT2_SAT_ICQ,                         // 418
    GEMINI_FIELD_BBF_SAT3_SAT_IBIAS,                       // 419
    GEMINI_FIELD_BBF_SAT3_SAT_OTA_CONTROL,                 // 420
    GEMINI_FIELD_BBF_SAT4_EGY_OUT_MUX,                     // 421
    GEMINI_FIELD_BBF_SAT4_EGY_GPO,                         // 422
    GEMINI_FIELD_BBF_SAT4_EGY_DET_EN,                      // 423
    GEMINI_FIELD_BBF_SAT5_EGY_THRES_MAN,                   // 424
    GEMINI_FIELD_BBF_SAT5_EGY_THRES_IN,                    // 425
    GEMINI_FIELD_BBF_SAT6_SAT_TH,                          // 426
    GEMINI_FIELD_HKADC_CFG_HKADC_INPUT,                    // 427
    GEMINI_FIELD_HKADC_CFG_RTUNER_SOURCE,                  // 428
    GEMINI_FIELD_HKADC_CFG_RTUNER_EN,                      // 429
    GEMINI_FIELD_RX_IM2_I_CFG0_TSENSOR_ON,                 // 430
    GEMINI_FIELD_RX_IM2_I_CFG0_IM2_VCAL_IDAC_I,            // 431
    GEMINI_FIELD_RX_IM2_I_CFG1_IM2_VCAL_RES_I,             // 432
    GEMINI_FIELD_RX_IM2_Q_CFG0_IM2_VCAL_IDAC_Q,            // 433
    GEMINI_FIELD_RX_IM2_Q_CFG1_IM2_VCAL_RES_Q,             // 434
    GEMINI_FIELD_RX_IM2_VCM0_IM2_VCM_BUF,                  // 435
    GEMINI_FIELD_RX_IM2_VCM0_IM2_VREF,                     // 436
    GEMINI_FIELD_RX_IM2_VCM0_IM2_VBUF_IAMP_VGS,            // 437
    GEMINI_FIELD_RX_IM2_VCM0_IM2_VBUF_IAMP_VCM,            // 438
    GEMINI_FIELD_RX_IM2_VCM1_IM2_VCM_MIXER,                // 439
    GEMINI_FIELD_RX_IM2_VCM1_IM2_VCM_BQ,                   // 440
    GEMINI_FIELD_RX_IM2_VCM1_IM2_VGF,                      // 441
    GEMINI_FIELD_RX_IM2_EN_CTL_IM2_TEST_2,                 // 442
    GEMINI_FIELD_RX_IM2_EN_CTL_IM2_TEST,                   // 443
    GEMINI_FIELD_RX_IM2_EN_CTL_IM2_GPO_1,                  // 444
    GEMINI_FIELD_RX_IM2_EN_CTL_IM2_GPO_0,                  // 445
    GEMINI_FIELD_RX_IM2_EN_CTL_IM2_VCM_EN,                 // 446
    GEMINI_FIELD_RX_IM2_EN_CTL_IM2_VGF_EN,                 // 447
    GEMINI_FIELD_RX_IM2_RSB_IM2_VGF_SHARE,                 // 448
    GEMINI_FIELD_RX_IM2_RSB_IM2_VGF_SWAP,                  // 449
    GEMINI_FIELD_RX_IM2_RSB_IM2_RSB_RES,                   // 450
    GEMINI_FIELD_RX_IM2_RSB_IM2_RSB_IDAC,                  // 451
    GEMINI_FIELD_RX_IM2_SPARE0_RX_IM2CAL_SPARE0,           // 452
    GEMINI_FIELD_RX_IM2_SPARE1_RX_IM2CAL_SPARE1,           // 453
    GEMINI_FIELD_BBF_AUX1_BBF_AUX1,                        // 454
    GEMINI_FIELD_BBF_AUX2_BBF_AUX2,                        // 455
    GEMINI_FIELD_BB_INSITU_RCM1_INSITU_RCM_OUT_LSB,        // 456
    GEMINI_FIELD_BB_INSITU_RCM2_INSITU_RCM_OVERFLOW,       // 457
    GEMINI_FIELD_BB_INSITU_RCM2_INSITU_RCM_OUT_MSB,        // 458
    GEMINI_FIELD_TX_AUX_DCOC0_TX_DCOC_DAC_I,               // 459
    GEMINI_FIELD_TX_AUX_DCOC0_TX_DCOC_DAC_Q,               // 460
    GEMINI_FIELD_TX_AUX_DCOC1_TX_DCOC_RANGE_I,             // 461
    GEMINI_FIELD_TX_AUX_DCOC1_TX_DCOC_RANGE_Q,             // 462
    GEMINI_FIELD_TX_AUX_DCOC1_TX_ATEST_EN,                 // 463
    GEMINI_FIELD_TX_AUX_DCOC1_TX_DAC_EN,                   // 464
    GEMINI_FIELD_TX_AUX_DCOC1_VTOI_BW,                     // 465
    GEMINI_FIELD_TX_AUX_DCOC1_G_LPBK_M3,                   // 466
    GEMINI_FIELD_TX_AUX_DCOC1_RTRIM_LODIV,                 // 467
    GEMINI_FIELD_TX_AUX_DCOC1_ITRIM_LODIV,                 // 468
    GEMINI_FIELD_PDET_CTL_G_PDET_RF,                       // 469
    GEMINI_FIELD_PDET_CTL_VGA_GAIN,                        // 470
    GEMINI_FIELD_PDET_CTL_VGA_DCO,                         // 471
    GEMINI_FIELD_PDET_CTL_MIX_MULTI,                       // 472
    GEMINI_FIELD_PDET_CTL_TX_GPO_SEL,                      // 473
    GEMINI_FIELD_PDET_OVRD_OVRD_DCO,                       // 474
    GEMINI_FIELD_PDET_OVRD_G_PDET_FBR,                     // 475
    GEMINI_FIELD_PDET_OVRD_G_PDET_INPR,                    // 476
    GEMINI_FIELD_PDET_OVRD_DCO_PDET,                       // 477
    GEMINI_FIELD_PDET_OVRD_OVRD_MIX,                       // 478
    GEMINI_FIELD_MIX_OVRD_MIX_I_EN,                        // 479
    GEMINI_FIELD_MIX_OVRD_MIX_Q_EN,                        // 480
    GEMINI_FIELD_TX_RCTUNE_CTL_OVRD_RTUNE,                 // 481
    GEMINI_FIELD_TX_RCTUNE_CTL_RTUNE,                      // 482
    GEMINI_FIELD_TX_RCTUNE_CTL_CTUNE,                      // 483
    GEMINI_FIELD_OVRD_CTL_OVRD_TX,                         // 484
    GEMINI_FIELD_OVRD_CTL_TX_BIAS_ENABLE,                  // 485
    GEMINI_FIELD_OVRD_CTL_VTOI_ENABLE,                     // 486
    GEMINI_FIELD_OVRD_CTL_TX_LOBUF_ENABLE,                 // 487
    GEMINI_FIELD_OVRD_CTL_TX_MIX_ENABLE,                   // 488
    GEMINI_FIELD_OVRD_CTL_PDET_RF_ENABLE,                  // 489
    GEMINI_FIELD_OVRD_CTL_PDET_AMP_ENABLE,                 // 490
    GEMINI_FIELD_OVRD_CTL_LPBK0_EN,                        // 491
    GEMINI_FIELD_OVRD_CTL_LPBK1_EN,                        // 492
    GEMINI_FIELD_TX_SPARE1_SPARE,                          // 493
    GEMINI_FIELD_TX_SPARE2_SPARE,                          // 494
    GEMINI_FIELD_TX_SPARE3_SPARE,                          // 495
    GEMINI_FIELD_TX_SPARE4_SPARE,                          // 496
    GEMINI_FIELD_TX_SPARE5_SPARE,                          // 497
    GEMINI_FIELD_TRSW_CTL_SW_RX_CAP_TUNE,                  // 498
    GEMINI_FIELD_TRSW_CTL_SW_BIAS_TUNE,                    // 499
    GEMINI_FIELD_TRSW_CTL_SW_DEEP_SLEEP_B,                 // 500
    GEMINI_FIELD_TRSW_CTL_OVRD_SW,                         // 501
    GEMINI_FIELD_TRSW_CTL_SW_TX_ENABLE,                    // 502
    GEMINI_FIELD_TRSW_CTL_SW_RX_ENABLE,                    // 503
    GEMINI_FIELD_TRSW_CTL_SW_BT_ENABLE,                    // 504
    GEMINI_FIELD_TRSW_CTL_SW_GPO_SEL,                      // 505
    GEMINI_FIELD_TRSW_CTL_SW_TEST,                         // 506
    GEMINI_FIELD_PA_BIAS_STG1_TYPE,                        // 507
    GEMINI_FIELD_PA_BIAS_STG1_IBIAS,                       // 508
    GEMINI_FIELD_PA_BIAS_STG2_TYPE,                        // 509
    GEMINI_FIELD_PA_BIAS_STG2_IBIAS,                       // 510
    GEMINI_FIELD_PA_BIAS_STG12_VCM,                        // 511
    GEMINI_FIELD_PA_BIAS_STG3_TYPE,                        // 512
    GEMINI_FIELD_PA_BIAS_STG3_IBIAS,                       // 513
    GEMINI_FIELD_PA_BIAS_STG3_CAS_VBIAS,                   // 514
    GEMINI_FIELD_PA_STG3_BIAS_SEL_PU_BU_VCM,               // 515
    GEMINI_FIELD_PA_STG3_BIAS_SEL_PU_BU_VBIAS,             // 516
    GEMINI_FIELD_PA_STG3_BIAS_PA_TEMP_SEN_EN,              // 517
    GEMINI_FIELD_DA_GAIN_CTL_G_FNL,                        // 518
    GEMINI_FIELD_DA_GAIN_CTL_G_PRE,                        // 519
    GEMINI_FIELD_DA_GAIN_CTL_VCM_FNL,                      // 520
    GEMINI_FIELD_DA_GAIN_CTL_VCM_PRE,                      // 521
    GEMINI_FIELD_DA_BIAS_IBIAS_FNL,                        // 522
    GEMINI_FIELD_DA_BIAS_IBIAS_PRE,                        // 523
    GEMINI_FIELD_DA_BIAS_DA_GPIO,                          // 524
    GEMINI_FIELD_DA_BALUN_PA_CTL_PA_GC1,                   // 525
    GEMINI_FIELD_DA_BALUN_PA_CTL_PA_GC2,                   // 526
    GEMINI_FIELD_DA_BALUN_PA_CTL_INPUT_BALUN,              // 527
    GEMINI_FIELD_DA_BALUN_PA_CTL_OUTPUT_BALUN,             // 528
    GEMINI_FIELD_TEST_PATH_CTL_PA_GPIO,                    // 529
    GEMINI_FIELD_TEST_PATH_CTL_PA_GPO_MUX_SEL,             // 530
    GEMINI_FIELD_TEST_PATH_CTL_PA_ADC_MUX_SEL,             // 531
    GEMINI_FIELD_TEST_PATH_CTL_PA_TEST,                    // 532
    GEMINI_FIELD_TEST_PATH_CTL_PA_DTOMODE,                 // 533
    GEMINI_FIELD_PA_EN_OVRD_PA,                            // 534
    GEMINI_FIELD_PA_EN_STG1_ENABLE,                        // 535
    GEMINI_FIELD_PA_EN_STG2_ENABLE,                        // 536
    GEMINI_FIELD_PA_EN_STG3_ENABLE,                        // 537
    GEMINI_FIELD_PA_EN_AR_DA_ENABLE,                       // 538
    GEMINI_FIELD_PA_EN_DA_PDO_EN,                          // 539
    GEMINI_FIELD_PA_EN_PA_BIAS_ENABLE,                     // 540
    GEMINI_FIELD_PA_EN_VSWR_EN,                            // 541
    GEMINI_FIELD_HDET_CTL_HDET_CSTART,                     // 542
    GEMINI_FIELD_HDET_CTL_HDET_RESET,                      // 543
    GEMINI_FIELD_HDET_CTL_HDET_PATH_SEL,                   // 544
    GEMINI_FIELD_HDET_CTL_HDET_OUT_SEL,                    // 545
    GEMINI_FIELD_HDET_CTL_EXT_ATTEN,                       // 546
    GEMINI_FIELD_HDET_CTL_INT_ATTEN,                       // 547
    GEMINI_FIELD_HDET_CTL_HDET_TIA_GAIN,                   // 548
    GEMINI_FIELD_HDET_BIAS_HDET_FILTER_BW,                 // 549
    GEMINI_FIELD_HDET_BIAS_HDET_SQUARE_BIAS,               // 550
    GEMINI_FIELD_HDET_BIAS_HDET_ICAL_STEP,                 // 551
    GEMINI_FIELD_HDET_BIAS_HDET_TIA_BIAS,                  // 552
    GEMINI_FIELD_HDET_BIAS_HDET_OFFSET_VOL,                // 553
    GEMINI_FIELD_HDET_BIAS_HDET_REF_VOL,                   // 554
    GEMINI_FIELD_HDET_TEST_HDET_BYPASS,                    // 555
    GEMINI_FIELD_HDET_TEST_HDET_TEMP_SEN_EN,               // 556
    GEMINI_FIELD_HDET_TEST_HDET_CLK_DIV,                   // 557
    GEMINI_FIELD_HDET_TEST_HDET_GPO_SEL,                   // 558
    GEMINI_FIELD_HDET_TEST_HDET_GPO_EN,                    // 559
    GEMINI_FIELD_HDET_TEST_HDET_DTESTEN,                   // 560
    GEMINI_FIELD_HDET_DCOC_OVRD_HDET,                      // 561
    GEMINI_FIELD_HDET_DCOC_HDET_ENABLE,                    // 562
    GEMINI_FIELD_HDET_DCOC_IB_SCAL_EN,                     // 563
    GEMINI_FIELD_HDET_DCOC_IB_RCAL_EN,                     // 564
    GEMINI_FIELD_HDET_DCOC_DCOC_CODE,                      // 565
    GEMINI_FIELD_CAL_DATA_VSWR_SOFT_RECOVERY,              // 566
    GEMINI_FIELD_CAL_DATA_VSWR_SOFT_OVERLOAD,              // 567
    GEMINI_FIELD_CAL_DATA_VSWR_HARD_OVERLOAD,              // 568
    GEMINI_FIELD_CAL_DATA_HDET_IB_SCAL_EN,                 // 569
    GEMINI_FIELD_CAL_DATA_HDET_IB_RCAL_EN,                 // 570
    GEMINI_FIELD_CAL_DATA_HDET_CAL_CODE,                   // 571
    GEMINI_FIELD_VSWR_CTL0_VSWR_R_DAC1,                    // 572
    GEMINI_FIELD_VSWR_CTL0_VSWR_R_DAC2,                    // 573
    GEMINI_FIELD_VSWR_CTL0_VSWR_R_DAC3,                    // 574
    GEMINI_FIELD_VSWR_CTL0_VSWR_CPC,                       // 575
    GEMINI_FIELD_VSWR_CTL1_VSWR_I_DAC1,                    // 576
    GEMINI_FIELD_VSWR_CTL1_VSWR_I_DAC2,                    // 577
    GEMINI_FIELD_VSWR_CTL1_VSWR_I_DAC3,                    // 578
    GEMINI_FIELD_VSWR_CTL1_VSWR_TMR,                       // 579
    GEMINI_FIELD_VSWR_CTL1_VSWR_STATUS_RESET,              // 580

    NUM_RF_FIELDS,
    GEMINI_FIELD_MAX_VAL								= 0XFFFFFFFF, //dummy added to change enum to 4 bytes
} eRfFields;


typedef enum
{
    RF_CAL_NORMAL,
    RF_CAL_TX_LO_START, //setup necessary registers to start Tx carrier suppression cal
    RF_CAL_TX_LO_END,   //restore necessary registers after Tx carrier suppression cal
    RF_CAL_TX_IQ_START,
    RF_CAL_TX_IQ_END,
    RF_CAL_RX_IQ_START,
    RF_CAL_RX_IQ_END,

    MAX_RF_CAL_MODE,
    RF_CAL_MODE_MAX_VAL								= 0XFFFFFFFF, //dummy added to change enum to 4 bytes
}eRfCalMode;


//Miscellaneous definitions
#define DCO_CORRECTION_RANGE      MSK_7





//[0]  GEMINI_REG_REV_ID
#define GEMINI_REV_ID_CHIP_ID_MASK                                   MSK_16
#define GEMINI_REV_ID_CHIP_ID_OFFSET                                 0



//[1]  GEMINI_REG_TX_GAIN_CONTROL
#define GEMINI_TX_GAIN_CONTROL_RESERVED_MASK                         MSK_12
#define GEMINI_TX_GAIN_CONTROL_RESERVED_OFFSET                       4

#define GEMINI_TX_GAIN_CONTROL_TX_GC_MASK                            MSK_4
#define GEMINI_TX_GAIN_CONTROL_TX_GC_OFFSET                          0



//[2]  GEMINI_REG_RX_GAIN_CONTROL
#define GEMINI_RX_GAIN_CONTROL_RESERVED_MASK                         MSK_2
#define GEMINI_RX_GAIN_CONTROL_RESERVED_OFFSET                       14

#define GEMINI_RX_GAIN_CONTROL_RX_GC1_MASK                           MSK_7
#define GEMINI_RX_GAIN_CONTROL_RX_GC1_OFFSET                         7

#define GEMINI_RX_GAIN_CONTROL_RX_GC0_MASK                           MSK_7
#define GEMINI_RX_GAIN_CONTROL_RX_GC0_OFFSET                         0



//[3]  GEMINI_REG_RESET_CONTROL
#define GEMINI_RESET_CONTROL_RESERVED_MASK                           MSK_12
#define GEMINI_RESET_CONTROL_RESERVED_OFFSET                         4

#define GEMINI_RESET_CONTROL_SRST_RTUNE_ON_MASK                      MSK_1
#define GEMINI_RESET_CONTROL_SRST_RTUNE_ON_OFFSET                    3

#define GEMINI_RESET_CONTROL_SRST_WO_RTUNE_ON_MASK                   MSK_1
#define GEMINI_RESET_CONTROL_SRST_WO_RTUNE_ON_OFFSET                 2

#define GEMINI_RESET_CONTROL_SRST_RTUNE_MASK                         MSK_1
#define GEMINI_RESET_CONTROL_SRST_RTUNE_OFFSET                       1

#define GEMINI_RESET_CONTROL_SRST_WO_RTUNE_MASK                      MSK_1
#define GEMINI_RESET_CONTROL_SRST_WO_RTUNE_OFFSET                    0



//[4]  GEMINI_REG_MODE_SEL1
#define GEMINI_MODE_SEL1_EXT_PA_MASK                                 MSK_1
#define GEMINI_MODE_SEL1_EXT_PA_OFFSET                               15

#define GEMINI_MODE_SEL1_PLLEN_FORCE_MASK                            MSK_1
#define GEMINI_MODE_SEL1_PLLEN_FORCE_OFFSET                          14

#define GEMINI_MODE_SEL1_RESERVED_MASK                               MSK_3
#define GEMINI_MODE_SEL1_RESERVED_OFFSET                             11

#define GEMINI_MODE_SEL1_TXRX_BAND_MASK                              MSK_1
#define GEMINI_MODE_SEL1_TXRX_BAND_OFFSET                            10

#define GEMINI_MODE_SEL1_TXRX_WRITE_ALL_MASK                         MSK_1
#define GEMINI_MODE_SEL1_TXRX_WRITE_ALL_OFFSET                       9

#define GEMINI_MODE_SEL1_TXRX_REG_SEL_MASK                           MSK_1
#define GEMINI_MODE_SEL1_TXRX_REG_SEL_OFFSET                         8

#define GEMINI_MODE_SEL1_TXRX_CAL_MODE_MASK                          MSK_4
#define GEMINI_MODE_SEL1_TXRX_CAL_MODE_OFFSET                        4

#define GEMINI_MODE_SEL1_IQ_DIV_MODE_MASK                            MSK_1
#define GEMINI_MODE_SEL1_IQ_DIV_MODE_OFFSET                          3

#define GEMINI_MODE_SEL1_EN_TXLO_MODE_MASK                           MSK_1
#define GEMINI_MODE_SEL1_EN_TXLO_MODE_OFFSET                         2

#define GEMINI_MODE_SEL1_EN_RXLO_MODE_MASK                           MSK_2
#define GEMINI_MODE_SEL1_EN_RXLO_MODE_OFFSET                         0



//[5]  GEMINI_REG_MODE_SEL2
#define GEMINI_MODE_SEL2_RTUNER_CODE_BBF_MASK                        MSK_2
#define GEMINI_MODE_SEL2_RTUNER_CODE_BBF_OFFSET                      14

#define GEMINI_MODE_SEL2_RTUNER_CODE_BIAS_MASK                       MSK_6
#define GEMINI_MODE_SEL2_RTUNER_CODE_BIAS_OFFSET                     8

#define GEMINI_MODE_SEL2_EN_RTX_BIAS_MASK                            MSK_2
#define GEMINI_MODE_SEL2_EN_RTX_BIAS_OFFSET                          6

#define GEMINI_MODE_SEL2_PTAT_TUNED_BUF_EN_MASK                      MSK_1
#define GEMINI_MODE_SEL2_PTAT_TUNED_BUF_EN_OFFSET                    5

#define GEMINI_MODE_SEL2_PTAT_BUF_EN_MASK                            MSK_1
#define GEMINI_MODE_SEL2_PTAT_BUF_EN_OFFSET                          4

#define GEMINI_MODE_SEL2_BG_TUNED_BUF_EN_MASK                        MSK_1
#define GEMINI_MODE_SEL2_BG_TUNED_BUF_EN_OFFSET                      3

#define GEMINI_MODE_SEL2_BG_BUF_EN_MASK                              MSK_1
#define GEMINI_MODE_SEL2_BG_BUF_EN_OFFSET                            2

#define GEMINI_MODE_SEL2_PTAT_EN_MASK                                MSK_1
#define GEMINI_MODE_SEL2_PTAT_EN_OFFSET                              1

#define GEMINI_MODE_SEL2_BG_EN_MASK                                  MSK_1
#define GEMINI_MODE_SEL2_BG_EN_OFFSET                                0



//[6]  GEMINI_REG_MODE_SEL3
#define GEMINI_MODE_SEL3_RESERVED_15_MASK                            MSK_1
#define GEMINI_MODE_SEL3_RESERVED_15_OFFSET                          15

#define GEMINI_MODE_SEL3_SEL_DEMUX_BBDACIN_MASK                      MSK_1
#define GEMINI_MODE_SEL3_SEL_DEMUX_BBDACIN_OFFSET                    14

#define GEMINI_MODE_SEL3_RESERVED_13_12_MASK                         MSK_2
#define GEMINI_MODE_SEL3_RESERVED_13_12_OFFSET                       12

#define GEMINI_MODE_SEL3_OVRD_CAL_MASK                               MSK_1
#define GEMINI_MODE_SEL3_OVRD_CAL_OFFSET                             11

#define GEMINI_MODE_SEL3_TXRX_BIAS_GPO_SEL_MASK                      MSK_3
#define GEMINI_MODE_SEL3_TXRX_BIAS_GPO_SEL_OFFSET                    8

#define GEMINI_MODE_SEL3_RFONLY_TESTEN_MASK                          MSK_1
#define GEMINI_MODE_SEL3_RFONLY_TESTEN_OFFSET                        7

#define GEMINI_MODE_SEL3_DIG_DR_STR_MASK                             MSK_2
#define GEMINI_MODE_SEL3_DIG_DR_STR_OFFSET                           5

#define GEMINI_MODE_SEL3_GPO_TEST_BLOCK_EN_MASK                      MSK_1
#define GEMINI_MODE_SEL3_GPO_TEST_BLOCK_EN_OFFSET                    4

#define GEMINI_MODE_SEL3_TEST_SIGNAL_SLCT_MASK                       MSK_4
#define GEMINI_MODE_SEL3_TEST_SIGNAL_SLCT_OFFSET                     0



//[7]  GEMINI_REG_TEST_MODE
#define GEMINI_TEST_MODE_RESERVED_MASK                               MSK_4
#define GEMINI_TEST_MODE_RESERVED_OFFSET                             12

#define GEMINI_TEST_MODE_SEL_BBI_PN_MASK                             MSK_6
#define GEMINI_TEST_MODE_SEL_BBI_PN_OFFSET                           6

#define GEMINI_TEST_MODE_SEL_BBQ_PN_MASK                             MSK_6
#define GEMINI_TEST_MODE_SEL_BBQ_PN_OFFSET                           0



//[8]  GEMINI_REG_SW_OVERRIDE
#define GEMINI_SW_OVERRIDE_DCCAL__ENABLE_MASK                        MSK_1
#define GEMINI_SW_OVERRIDE_DCCAL__ENABLE_OFFSET                      11

#define GEMINI_SW_OVERRIDE_PLL__ENABLE_MASK                          MSK_1
#define GEMINI_SW_OVERRIDE_PLL__ENABLE_OFFSET                        10

#define GEMINI_SW_OVERRIDE_TCXO_ENABLE_MASK                          MSK_1
#define GEMINI_SW_OVERRIDE_TCXO_ENABLE_OFFSET                        9

#define GEMINI_SW_OVERRIDE_XO_ENABLE_MASK                            MSK_1
#define GEMINI_SW_OVERRIDE_XO_ENABLE_OFFSET                          8

#define GEMINI_SW_OVERRIDE_PA_ENABLE_MASK                            MSK_1
#define GEMINI_SW_OVERRIDE_PA_ENABLE_OFFSET                          7

#define GEMINI_SW_OVERRIDE_DA_ENABLE_MASK                            MSK_1
#define GEMINI_SW_OVERRIDE_DA_ENABLE_OFFSET                          6

#define GEMINI_SW_OVERRIDE_TR_SW0_ENABLE_MASK                        MSK_1
#define GEMINI_SW_OVERRIDE_TR_SW0_ENABLE_OFFSET                      5

#define GEMINI_SW_OVERRIDE_TR_SW1_ENABLE_MASK                        MSK_1
#define GEMINI_SW_OVERRIDE_TR_SW1_ENABLE_OFFSET                      4

#define GEMINI_SW_OVERRIDE_TX_ENABLE_MASK                            MSK_1
#define GEMINI_SW_OVERRIDE_TX_ENABLE_OFFSET                          3

#define GEMINI_SW_OVERRIDE_RX0_ENABLE_MASK                           MSK_1
#define GEMINI_SW_OVERRIDE_RX0_ENABLE_OFFSET                         2

#define GEMINI_SW_OVERRIDE_RX1_ENABLE_MASK                           MSK_1
#define GEMINI_SW_OVERRIDE_RX1_ENABLE_OFFSET                         1

#define GEMINI_SW_OVERRIDE_OVRD_MC_MASK                              MSK_1
#define GEMINI_SW_OVERRIDE_OVRD_MC_OFFSET                            0



//[9]  GEMINI_REG_SPARE_1
#define GEMINI_SPARE_1_MC_DTO0_SEL_MASK                              MSK_5
#define GEMINI_SPARE_1_MC_DTO0_SEL_OFFSET                            11

#define GEMINI_SPARE_1_MC_DTO1_SEL_MASK                              MSK_5
#define GEMINI_SPARE_1_MC_DTO1_SEL_OFFSET                            6

#define GEMINI_SPARE_1_DTO0_SEL_MASK                                 MSK_3
#define GEMINI_SPARE_1_DTO0_SEL_OFFSET                               3

#define GEMINI_SPARE_1_DTO1_SEL_MASK                                 MSK_3
#define GEMINI_SPARE_1_DTO1_SEL_OFFSET                               0



//[10]  GEMINI_REG_SPARE_2
#define GEMINI_SPARE_2_DTEST_SEL_MASK                                MSK_2
#define GEMINI_SPARE_2_DTEST_SEL_OFFSET                              14

#define GEMINI_SPARE_2_DTEST_GCSEL_MASK                              MSK_2
#define GEMINI_SPARE_2_DTEST_GCSEL_OFFSET                            12

#define GEMINI_SPARE_2_DTO_GPO_SEL_MASK                              MSK_1
#define GEMINI_SPARE_2_DTO_GPO_SEL_OFFSET                            11

#define GEMINI_SPARE_2_AVAILABLE_MASK                                MSK_11
#define GEMINI_SPARE_2_AVAILABLE_OFFSET                              0



//[11]  GEMINI_REG_SPARE_3
#define GEMINI_SPARE_3_AVAILABLE_MASK                                MSK_16
#define GEMINI_SPARE_3_AVAILABLE_OFFSET                              0



//[12]  GEMINI_REG_SPARE_4
#define GEMINI_SPARE_4_AVAILABLE_MASK                                MSK_16
#define GEMINI_SPARE_4_AVAILABLE_OFFSET                              0



//[32]  GEMINI_REG_XO_REG0
#define GEMINI_XO_REG0_RESERVED_MASK                                 MSK_1
#define GEMINI_XO_REG0_RESERVED_OFFSET                               15

#define GEMINI_XO_REG0_PLL_BUF_EN_MASK                               MSK_2
#define GEMINI_XO_REG0_PLL_BUF_EN_OFFSET                             13

#define GEMINI_XO_REG0_TCXO_BUF_EN_MASK_MASK                         MSK_1
#define GEMINI_XO_REG0_TCXO_BUF_EN_MASK_OFFSET                       12

#define GEMINI_XO_REG0_XO_EN_MASK_MASK                               MSK_1
#define GEMINI_XO_REG0_XO_EN_MASK_OFFSET                             11

#define GEMINI_XO_REG0_XO_EXTBUF_EN_MASK                             MSK_2
#define GEMINI_XO_REG0_XO_EXTBUF_EN_OFFSET                           9

#define GEMINI_XO_REG0_XO_EXTBUF_DRV_MASK                            MSK_2
#define GEMINI_XO_REG0_XO_EXTBUF_DRV_OFFSET                          7

#define GEMINI_XO_REG0_XO_EXTBUF_CL_MASK                             MSK_2
#define GEMINI_XO_REG0_XO_EXTBUF_CL_OFFSET                           5

#define GEMINI_XO_REG0_XO_VREG_B_EXT_MASK                            MSK_3
#define GEMINI_XO_REG0_XO_VREG_B_EXT_OFFSET                          2

#define GEMINI_XO_REG0_REFFM_REG_EN_MASK                             MSK_2
#define GEMINI_XO_REG0_REFFM_REG_EN_OFFSET                           0



//[33]  GEMINI_REG_BLOCK_ENABLE
#define GEMINI_BLOCK_ENABLE_RESERVED_MASK                            MSK_1
#define GEMINI_BLOCK_ENABLE_RESERVED_OFFSET                          15

#define GEMINI_BLOCK_ENABLE_CLKPATH_SEL_MASK                         MSK_2
#define GEMINI_BLOCK_ENABLE_CLKPATH_SEL_OFFSET                       13

#define GEMINI_BLOCK_ENABLE_CLK1X_EN_MASK                            MSK_2
#define GEMINI_BLOCK_ENABLE_CLK1X_EN_OFFSET                          11

#define GEMINI_BLOCK_ENABLE_REFFM_EN_RF_MASK                         MSK_2
#define GEMINI_BLOCK_ENABLE_REFFM_EN_RF_OFFSET                       9

#define GEMINI_BLOCK_ENABLE_REFDIV_SEL_MASK                          MSK_2
#define GEMINI_BLOCK_ENABLE_REFDIV_SEL_OFFSET                        7

#define GEMINI_BLOCK_ENABLE_REFDIV_EN_MASK                           MSK_1
#define GEMINI_BLOCK_ENABLE_REFDIV_EN_OFFSET                         6

#define GEMINI_BLOCK_ENABLE_REFFM_EN_MASK                            MSK_2
#define GEMINI_BLOCK_ENABLE_REFFM_EN_OFFSET                          4

#define GEMINI_BLOCK_ENABLE_REFFM_EN2X_MASK                          MSK_2
#define GEMINI_BLOCK_ENABLE_REFFM_EN2X_OFFSET                        2

#define GEMINI_BLOCK_ENABLE_REFFM_EN4X_MASK                          MSK_2
#define GEMINI_BLOCK_ENABLE_REFFM_EN4X_OFFSET                        0



//[34]  GEMINI_REG_XO_TCXO
#define GEMINI_XO_TCXO_IKNOBPLL_1_0_MASK                             MSK_2
#define GEMINI_XO_TCXO_IKNOBPLL_1_0_OFFSET                           14

#define GEMINI_XO_TCXO_IKNOBINP_1_0_MASK                             MSK_2
#define GEMINI_XO_TCXO_IKNOBINP_1_0_OFFSET                           12

#define GEMINI_XO_TCXO_TCXO_CCTRL_MASK                               MSK_1
#define GEMINI_XO_TCXO_TCXO_CCTRL_OFFSET                             11

#define GEMINI_XO_TCXO_TCXO_BWSEL_MASK                               MSK_1
#define GEMINI_XO_TCXO_TCXO_BWSEL_OFFSET                             10

#define GEMINI_XO_TCXO_XO_GM_TRIM_MASK                               MSK_2
#define GEMINI_XO_TCXO_XO_GM_TRIM_OFFSET                             8

#define GEMINI_XO_TCXO_XO_D_CTRIM_MASK                               MSK_4
#define GEMINI_XO_TCXO_XO_D_CTRIM_OFFSET                             4

#define GEMINI_XO_TCXO_XO_G_CTRIM_MASK                               MSK_4
#define GEMINI_XO_TCXO_XO_G_CTRIM_OFFSET                             0



//[35]  GEMINI_REG_REFFM_REFDIV
#define GEMINI_REFFM_REFDIV_RESERVED_MASK                            MSK_1
#define GEMINI_REFFM_REFDIV_RESERVED_OFFSET                          15

#define GEMINI_REFFM_REFDIV_REFFM_FASTSW_MASK                        MSK_1
#define GEMINI_REFFM_REFDIV_REFFM_FASTSW_OFFSET                      14

#define GEMINI_REFFM_REFDIV_REFFM_CCTRL_MASK                         MSK_1
#define GEMINI_REFFM_REFDIV_REFFM_CCTRL_OFFSET                       13

#define GEMINI_REFFM_REFDIV_REFFM_BWSEL_MASK                         MSK_1
#define GEMINI_REFFM_REFDIV_REFFM_BWSEL_OFFSET                       12

#define GEMINI_REFFM_REFDIV_REFDIV_DIVR_1_0_MASK                     MSK_2
#define GEMINI_REFFM_REFDIV_REFDIV_DIVR_1_0_OFFSET                   10

#define GEMINI_REFFM_REFDIV_REFFM_RFDR_MASK                          MSK_2
#define GEMINI_REFFM_REFDIV_REFFM_RFDR_OFFSET                        8

#define GEMINI_REFFM_REFDIV_IKNOBREFFM_MASK                          MSK_2
#define GEMINI_REFFM_REFDIV_IKNOBREFFM_OFFSET                        6

#define GEMINI_REFFM_REFDIV_REFFM_LOCKBSEL_MASK                      MSK_1
#define GEMINI_REFFM_REFDIV_REFFM_LOCKBSEL_OFFSET                    5

#define GEMINI_REFFM_REFDIV_REFFM_LOCKB_MASK                         MSK_1
#define GEMINI_REFFM_REFDIV_REFFM_LOCKB_OFFSET                       4

#define GEMINI_REFFM_REFDIV_REFFM_LOCKA_MASK                         MSK_1
#define GEMINI_REFFM_REFDIV_REFFM_LOCKA_OFFSET                       3

#define GEMINI_REFFM_REFDIV_CLK_FREQ_MASK                            MSK_3
#define GEMINI_REFFM_REFDIV_CLK_FREQ_OFFSET                          0



//[36]  GEMINI_REG_REFFM
#define GEMINI_REFFM_REFFM_DELAY2_MASK                               MSK_2
#define GEMINI_REFFM_REFFM_DELAY2_OFFSET                             14

#define GEMINI_REFFM_REFFM_DELAY1_MASK                               MSK_2
#define GEMINI_REFFM_REFFM_DELAY1_OFFSET                             12

#define GEMINI_REFFM_REFFM_S4X_MASK                                  MSK_12
#define GEMINI_REFFM_REFFM_S4X_OFFSET                                0

#define GEMINI_CLK_REFFM_REFDIV1_RESERVED_MASK                       MSK_6
#define GEMINI_CLK_REFFM_REFDIV1_RESERVED_OFFSET                     10

#define GEMINI_CLK_REFFM_REFDIV1_CML2CMOS_REG_EN_MASK                MSK_2
#define GEMINI_CLK_REFFM_REFDIV1_CML2CMOS_REG_EN_OFFSET              8

#define GEMINI_CLK_REFFM_REFDIV1_REFDIV_RST_MASK                     MSK_1
#define GEMINI_CLK_REFFM_REFDIV1_REFDIV_RST_OFFSET                   7

#define GEMINI_CLK_REFFM_REFDIV1_REFFM_RST_MASK                      MSK_1
#define GEMINI_CLK_REFFM_REFDIV1_REFFM_RST_OFFSET                    6

#define GEMINI_CLK_REFFM_REFDIV1_REFFM_EN_PFD_MASK                   MSK_2
#define GEMINI_CLK_REFFM_REFDIV1_REFFM_EN_PFD_OFFSET                 4

#define GEMINI_CLK_REFFM_REFDIV1_REFFM_EN_CPBOOST_MASK               MSK_1
#define GEMINI_CLK_REFFM_REFDIV1_REFFM_EN_CPBOOST_OFFSET             3

#define GEMINI_CLK_REFFM_REFDIV1_XO_DTO_MODE_MASK                    MSK_3
#define GEMINI_CLK_REFFM_REFDIV1_XO_DTO_MODE_OFFSET                  0



//[37]  GEMINI_REG_CLK_REFFM_REFDIV1


//[38]  GEMINI_REG_DC_GPO
#define GEMINI_DC_GPO_RESERVED_MASK                                  MSK_4
#define GEMINI_DC_GPO_RESERVED_OFFSET                                12

#define GEMINI_DC_GPO_NREFFM2_EN_PROBE_MASK                          MSK_1
#define GEMINI_DC_GPO_NREFFM2_EN_PROBE_OFFSET                        11

#define GEMINI_DC_GPO_NREFFM1_EN_PROBE_MASK                          MSK_1
#define GEMINI_DC_GPO_NREFFM1_EN_PROBE_OFFSET                        10

#define GEMINI_DC_GPO_NTCXO_EN_PROBE_MASK                            MSK_1
#define GEMINI_DC_GPO_NTCXO_EN_PROBE_OFFSET                          9

#define GEMINI_DC_GPO_NXO_EN_PROBE_MASK                              MSK_1
#define GEMINI_DC_GPO_NXO_EN_PROBE_OFFSET                            8

#define GEMINI_DC_GPO_NBIAS_EN_PROBE_MASK                            MSK_1
#define GEMINI_DC_GPO_NBIAS_EN_PROBE_OFFSET                          7

#define GEMINI_DC_GPO_REFFM_VREG_EN_PROBE_MASK                       MSK_1
#define GEMINI_DC_GPO_REFFM_VREG_EN_PROBE_OFFSET                     6

#define GEMINI_DC_GPO_XO_VREG_EN_PROBE_MASK                          MSK_1
#define GEMINI_DC_GPO_XO_VREG_EN_PROBE_OFFSET                        5

#define GEMINI_DC_GPO_TCXO_VREG_EN_PROBE_MASK                        MSK_1
#define GEMINI_DC_GPO_TCXO_VREG_EN_PROBE_OFFSET                      4

#define GEMINI_DC_GPO_CML_VREG_EN_PROBE_MASK                         MSK_1
#define GEMINI_DC_GPO_CML_VREG_EN_PROBE_OFFSET                       3

#define GEMINI_DC_GPO_XO_VREG_PRB_EXT_MASK                           MSK_3
#define GEMINI_DC_GPO_XO_VREG_PRB_EXT_OFFSET                         0



//[39]  GEMINI_REG_CLK_REFFM_REFDIV2
#define GEMINI_CLK_REFFM_REFDIV2_DTOP_BUF_EN_MASK                    MSK_2
#define GEMINI_CLK_REFFM_REFDIV2_DTOP_BUF_EN_OFFSET                  14

#define GEMINI_CLK_REFFM_REFDIV2_EN_DCLK_MASK                        MSK_2
#define GEMINI_CLK_REFFM_REFDIV2_EN_DCLK_OFFSET                      12

#define GEMINI_CLK_REFFM_REFDIV2_EN_PLL_DCLK_MASK                    MSK_2
#define GEMINI_CLK_REFFM_REFDIV2_EN_PLL_DCLK_OFFSET                  10

#define GEMINI_CLK_REFFM_REFDIV2_EN_PA_DCLK_MASK                     MSK_2
#define GEMINI_CLK_REFFM_REFDIV2_EN_PA_DCLK_OFFSET                   8

#define GEMINI_CLK_REFFM_REFDIV2_EN_RCM0_DCLK_MASK                   MSK_2
#define GEMINI_CLK_REFFM_REFDIV2_EN_RCM0_DCLK_OFFSET                 6

#define GEMINI_CLK_REFFM_REFDIV2_EN_RCM1_DCLK_MASK                   MSK_2
#define GEMINI_CLK_REFFM_REFDIV2_EN_RCM1_DCLK_OFFSET                 4

#define GEMINI_CLK_REFFM_REFDIV2_RESERVED_MASK                       MSK_4
#define GEMINI_CLK_REFFM_REFDIV2_RESERVED_OFFSET                     0



//[40]  GEMINI_REG_XO_SPARE1
#define GEMINI_XO_SPARE1_XO_SPARE_1_MASK                             MSK_16
#define GEMINI_XO_SPARE1_XO_SPARE_1_OFFSET                           0



//[48]  GEMINI_REG_PLL_REG0
#define GEMINI_PLL_REG0_NF_7_0_MASK                                  MSK_8
#define GEMINI_PLL_REG0_NF_7_0_OFFSET                                0



//[49]  GEMINI_REG_PLL_REG1
#define GEMINI_PLL_REG1_NF_15_8_MASK                                 MSK_8
#define GEMINI_PLL_REG1_NF_15_8_OFFSET                               0



//[50]  GEMINI_REG_PLL_REG2
#define GEMINI_PLL_REG2_NB_6_MASK                                    MSK_1
#define GEMINI_PLL_REG2_NB_6_OFFSET                                  7

#define GEMINI_PLL_REG2_NF_22_16_MASK                                MSK_7
#define GEMINI_PLL_REG2_NF_22_16_OFFSET                              0



//[51]  GEMINI_REG_PLL_REG3
#define GEMINI_PLL_REG3_PLL_READY_MASK                               MSK_1
#define GEMINI_PLL_REG3_PLL_READY_OFFSET                             7

#define GEMINI_PLL_REG3_FREE_MASK                                    MSK_3
#define GEMINI_PLL_REG3_FREE_OFFSET                                  4

#define GEMINI_PLL_REG3_NBI_9_6_MASK                                 MSK_4
#define GEMINI_PLL_REG3_NBI_9_6_OFFSET                               0



//[52]  GEMINI_REG_PLL_REG4
#define GEMINI_PLL_REG4_NB_5_0_MASK                                  MSK_6
#define GEMINI_PLL_REG4_NB_5_0_OFFSET                                2

#define GEMINI_PLL_REG4_NA_1_0_MASK                                  MSK_2
#define GEMINI_PLL_REG4_NA_1_0_OFFSET                                0



//[53]  GEMINI_REG_PLL_REG5
#define GEMINI_PLL_REG5_BS_EN_MASK                                   MSK_2
#define GEMINI_PLL_REG5_BS_EN_OFFSET                                 6

#define GEMINI_PLL_REG5_SDMOGAIN_MASK                                MSK_2
#define GEMINI_PLL_REG5_SDMOGAIN_OFFSET                              4

#define GEMINI_PLL_REG5_RDIV_MASK                                    MSK_4
#define GEMINI_PLL_REG5_RDIV_OFFSET                                  0



//[54]  GEMINI_REG_PLL_REG6
#define GEMINI_PLL_REG6_NF_OFFSET_7_0_MASK                           MSK_8
#define GEMINI_PLL_REG6_NF_OFFSET_7_0_OFFSET                         0



//[55]  GEMINI_REG_PLL_REG7
#define GEMINI_PLL_REG7_NF_OFFSET_15_8_MASK                          MSK_8
#define GEMINI_PLL_REG7_NF_OFFSET_15_8_OFFSET                        0



//[56]  GEMINI_REG_PLL_REG8
#define GEMINI_PLL_REG8_BSSTART_MASK                                 MSK_1
#define GEMINI_PLL_REG8_BSSTART_OFFSET                               7

#define GEMINI_PLL_REG8_BSMODE_MASK                                  MSK_3
#define GEMINI_PLL_REG8_BSMODE_OFFSET                                4

#define GEMINI_PLL_REG8_BSBANK_MASK                                  MSK_2
#define GEMINI_PLL_REG8_BSBANK_OFFSET                                2

#define GEMINI_PLL_REG8_BKSHFT_MASK                                  MSK_2
#define GEMINI_PLL_REG8_BKSHFT_OFFSET                                0



//[57]  GEMINI_REG_PLL_REG9
#define GEMINI_PLL_REG9_BSWAIT_MASK                                  MSK_3
#define GEMINI_PLL_REG9_BSWAIT_OFFSET                                5

#define GEMINI_PLL_REG9_BSSAMPLE_MASK                                MSK_3
#define GEMINI_PLL_REG9_BSSAMPLE_OFFSET                              2

#define GEMINI_PLL_REG9_BSSETTIME_MASK                               MSK_2
#define GEMINI_PLL_REG9_BSSETTIME_OFFSET                             0



//[58]  GEMINI_REG_PLL_REG10
#define GEMINI_PLL_REG10_SDSTRT_DIS_MASK                             MSK_1
#define GEMINI_PLL_REG10_SDSTRT_DIS_OFFSET                           7

#define GEMINI_PLL_REG10_SD_TESTEN_MASK                              MSK_1
#define GEMINI_PLL_REG10_SD_TESTEN_OFFSET                            6

#define GEMINI_PLL_REG10_NF_LSB_SEL_MASK                             MSK_1
#define GEMINI_PLL_REG10_NF_LSB_SEL_OFFSET                           5

#define GEMINI_PLL_REG10_NF_LSBB_MASK                                MSK_1
#define GEMINI_PLL_REG10_NF_LSBB_OFFSET                              4

#define GEMINI_PLL_REG10_SDM_SEL_MASK                                MSK_1
#define GEMINI_PLL_REG10_SDM_SEL_OFFSET                              3

#define GEMINI_PLL_REG10_SDM_RESET_MASK                              MSK_1
#define GEMINI_PLL_REG10_SDM_RESET_OFFSET                            2

#define GEMINI_PLL_REG10_SDM_EN_MASK                                 MSK_2
#define GEMINI_PLL_REG10_SDM_EN_OFFSET                               0



//[59]  GEMINI_REG_PLL_REG11
#define GEMINI_PLL_REG11_RSVD_MASK                                   MSK_1
#define GEMINI_PLL_REG11_RSVD_OFFSET                                 7

#define GEMINI_PLL_REG11_SYNON_MAN_MASK                              MSK_1
#define GEMINI_PLL_REG11_SYNON_MAN_OFFSET                            6

#define GEMINI_PLL_REG11_PRES_SEL_MASK                               MSK_1
#define GEMINI_PLL_REG11_PRES_SEL_OFFSET                             5

#define GEMINI_PLL_REG11_PLLMD_MASK                                  MSK_1
#define GEMINI_PLL_REG11_PLLMD_OFFSET                                4

#define GEMINI_PLL_REG11_BSTESTEN_MASK                               MSK_1
#define GEMINI_PLL_REG11_BSTESTEN_OFFSET                             3

#define GEMINI_PLL_REG11_PLLRST_MASK                                 MSK_1
#define GEMINI_PLL_REG11_PLLRST_OFFSET                               2

#define GEMINI_PLL_REG11_BSCLK_MUX_SEL_MASK                          MSK_2
#define GEMINI_PLL_REG11_BSCLK_MUX_SEL_OFFSET                        0



//[60]  GEMINI_REG_PLL_REG12
#define GEMINI_PLL_REG12_CPCTL_MASK                                  MSK_2
#define GEMINI_PLL_REG12_CPCTL_OFFSET                                6

#define GEMINI_PLL_REG12_VCONFRCEN_MASK                              MSK_1
#define GEMINI_PLL_REG12_VCONFRCEN_OFFSET                            5

#define GEMINI_PLL_REG12_VCONSTATE_MASK                              MSK_1
#define GEMINI_PLL_REG12_VCONSTATE_OFFSET                            4

#define GEMINI_PLL_REG12_RESERVED_MASK                               MSK_4
#define GEMINI_PLL_REG12_RESERVED_OFFSET                             0



//[61]  GEMINI_REG_PLL_REG13
#define GEMINI_PLL_REG13_ABW_MODE_MASK                               MSK_1
#define GEMINI_PLL_REG13_ABW_MODE_OFFSET                             7

#define GEMINI_PLL_REG13_ABW_TARGETCP_SEL_MASK                       MSK_1
#define GEMINI_PLL_REG13_ABW_TARGETCP_SEL_OFFSET                     6

#define GEMINI_PLL_REG13_ABW_TARGETR2_SEL_MASK                       MSK_1
#define GEMINI_PLL_REG13_ABW_TARGETR2_SEL_OFFSET                     5

#define GEMINI_PLL_REG13_RESERVED_MASK                               MSK_1
#define GEMINI_PLL_REG13_RESERVED_OFFSET                             4

#define GEMINI_PLL_REG13_ABW_TMASK_MASK                              MSK_4
#define GEMINI_PLL_REG13_ABW_TMASK_OFFSET                            0



//[62]  GEMINI_REG_PLL_REG14
#define GEMINI_PLL_REG14_RESERVED_MASK                               MSK_4
#define GEMINI_PLL_REG14_RESERVED_OFFSET                             4

#define GEMINI_PLL_REG14_ABW_TIME_MASK                               MSK_4
#define GEMINI_PLL_REG14_ABW_TIME_OFFSET                             0



//[63]  GEMINI_REG_PLL_REG15
#define GEMINI_PLL_REG15_LIN_STEP_CP_MASK                            MSK_3
#define GEMINI_PLL_REG15_LIN_STEP_CP_OFFSET                          5

#define GEMINI_PLL_REG15_START_CPGAIN_MASK                           MSK_5
#define GEMINI_PLL_REG15_START_CPGAIN_OFFSET                         0



//[64]  GEMINI_REG_PLL_REG16
#define GEMINI_PLL_REG16_LIN_STEP_R2_MASK                            MSK_3
#define GEMINI_PLL_REG16_LIN_STEP_R2_OFFSET                          5

#define GEMINI_PLL_REG16_START_R2_MASK                               MSK_5
#define GEMINI_PLL_REG16_START_R2_OFFSET                             0



//[65]  GEMINI_REG_PLL_REG17
#define GEMINI_PLL_REG17_FREE_MASK                                   MSK_2
#define GEMINI_PLL_REG17_FREE_OFFSET                                 4

#define GEMINI_PLL_REG17_DTEST_3_0_MASK                              MSK_4
#define GEMINI_PLL_REG17_DTEST_3_0_OFFSET                            0



//[66]  GEMINI_REG_PLL_REG18
#define GEMINI_PLL_REG18_TSTCNT_7_0_MASK                             MSK_8
#define GEMINI_PLL_REG18_TSTCNT_7_0_OFFSET                           0



//[67]  GEMINI_REG_PLL_REG19
#define GEMINI_PLL_REG19_TSTCNT_15_8_MASK                            MSK_8
#define GEMINI_PLL_REG19_TSTCNT_15_8_OFFSET                          0



//[68]  GEMINI_REG_PLL_REG20
#define GEMINI_PLL_REG20_IVCOBK_MASK                                 MSK_8
#define GEMINI_PLL_REG20_IVCOBK_OFFSET                               0



//[69]  GEMINI_REG_PLL_REG21
#define GEMINI_PLL_REG21_FVCOBK_MASK                                 MSK_8
#define GEMINI_PLL_REG21_FVCOBK_OFFSET                               0



//[70]  GEMINI_REG_PLL_REG22
#define GEMINI_PLL_REG22_BSTARGET_7_0_MASK                           MSK_8
#define GEMINI_PLL_REG22_BSTARGET_7_0_OFFSET                         0



//[71]  GEMINI_REG_PLL_REG23
#define GEMINI_PLL_REG23_BSTARGET_15_8_MASK                          MSK_8
#define GEMINI_PLL_REG23_BSTARGET_15_8_OFFSET                        0



//[72]  GEMINI_REG_PLL_REG24
#define GEMINI_PLL_REG24_BSCOUNT_7_0_MASK                            MSK_8
#define GEMINI_PLL_REG24_BSCOUNT_7_0_OFFSET                          0



//[73]  GEMINI_REG_PLL_REG25
#define GEMINI_PLL_REG25_BSCOUNT_15_8_MASK                           MSK_8
#define GEMINI_PLL_REG25_BSCOUNT_15_8_OFFSET                         0



//[74]  GEMINI_REG_PLL_VFC_REG1
#define GEMINI_PLL_VFC_REG1_VFC_START_MASK                           MSK_1
#define GEMINI_PLL_VFC_REG1_VFC_START_OFFSET                         15

#define GEMINI_PLL_VFC_REG1_RESERVED_MASK                            MSK_9
#define GEMINI_PLL_VFC_REG1_RESERVED_OFFSET                          6

#define GEMINI_PLL_VFC_REG1_VFC_TIME_MASK                            MSK_2
#define GEMINI_PLL_VFC_REG1_VFC_TIME_OFFSET                          4

#define GEMINI_PLL_VFC_REG1_VFC_CAL_MASK_MASK                        MSK_4
#define GEMINI_PLL_VFC_REG1_VFC_CAL_MASK_OFFSET                      0



//[75]  GEMINI_REG_PLL_VFC_REG2
#define GEMINI_PLL_VFC_REG2_VFC_B0_SEL_MASK                          MSK_1
#define GEMINI_PLL_VFC_REG2_VFC_B0_SEL_OFFSET                        15

#define GEMINI_PLL_VFC_REG2_VFC_B0_MCODE_MASK                        MSK_4
#define GEMINI_PLL_VFC_REG2_VFC_B0_MCODE_OFFSET                      11

#define GEMINI_PLL_VFC_REG2_VFC_B0_MODE_MASK                         MSK_1
#define GEMINI_PLL_VFC_REG2_VFC_B0_MODE_OFFSET                       10

#define GEMINI_PLL_VFC_REG2_VFC_B0_RES_MASK                          MSK_2
#define GEMINI_PLL_VFC_REG2_VFC_B0_RES_OFFSET                        8

#define GEMINI_PLL_VFC_REG2_VFC_B1_SEL_MASK                          MSK_1
#define GEMINI_PLL_VFC_REG2_VFC_B1_SEL_OFFSET                        7

#define GEMINI_PLL_VFC_REG2_VFC_B1_MCODE_MASK                        MSK_4
#define GEMINI_PLL_VFC_REG2_VFC_B1_MCODE_OFFSET                      3

#define GEMINI_PLL_VFC_REG2_VFC_B1_MODE_MASK                         MSK_1
#define GEMINI_PLL_VFC_REG2_VFC_B1_MODE_OFFSET                       2

#define GEMINI_PLL_VFC_REG2_VFC_B1_RES_MASK                          MSK_2
#define GEMINI_PLL_VFC_REG2_VFC_B1_RES_OFFSET                        0



//[76]  GEMINI_REG_PLL_VFC_REG4
#define GEMINI_PLL_VFC_REG4_VFC_B2_SEL_MASK                          MSK_1
#define GEMINI_PLL_VFC_REG4_VFC_B2_SEL_OFFSET                        15

#define GEMINI_PLL_VFC_REG4_VFC_B2_MCODE_MASK                        MSK_4
#define GEMINI_PLL_VFC_REG4_VFC_B2_MCODE_OFFSET                      11

#define GEMINI_PLL_VFC_REG4_VFC_B2_MODE_MASK                         MSK_1
#define GEMINI_PLL_VFC_REG4_VFC_B2_MODE_OFFSET                       10

#define GEMINI_PLL_VFC_REG4_VFC_B2_RES_MASK                          MSK_2
#define GEMINI_PLL_VFC_REG4_VFC_B2_RES_OFFSET                        8

#define GEMINI_PLL_VFC_REG4_VFC_B3_SEL_MASK                          MSK_1
#define GEMINI_PLL_VFC_REG4_VFC_B3_SEL_OFFSET                        7

#define GEMINI_PLL_VFC_REG4_VFC_B3_MCODE_MASK                        MSK_4
#define GEMINI_PLL_VFC_REG4_VFC_B3_MCODE_OFFSET                      3

#define GEMINI_PLL_VFC_REG4_VFC_B3_MODE_MASK                         MSK_1
#define GEMINI_PLL_VFC_REG4_VFC_B3_MODE_OFFSET                       2

#define GEMINI_PLL_VFC_REG4_VFC_B3_RES_MASK                          MSK_2
#define GEMINI_PLL_VFC_REG4_VFC_B3_RES_OFFSET                        0



//[77]  GEMINI_REG_PLL_VFC_REG5
#define GEMINI_PLL_VFC_REG5_VFC_B0_ACODE_MASK                        MSK_4
#define GEMINI_PLL_VFC_REG5_VFC_B0_ACODE_OFFSET                      12

#define GEMINI_PLL_VFC_REG5_VFC_B1_ACODE_MASK                        MSK_4
#define GEMINI_PLL_VFC_REG5_VFC_B1_ACODE_OFFSET                      8

#define GEMINI_PLL_VFC_REG5_VFC_B2_ACODE_MASK                        MSK_4
#define GEMINI_PLL_VFC_REG5_VFC_B2_ACODE_OFFSET                      4

#define GEMINI_PLL_VFC_REG5_VFC_B3_ACODE_MASK                        MSK_4
#define GEMINI_PLL_VFC_REG5_VFC_B3_ACODE_OFFSET                      0



//[78]  GEMINI_REG_PLL_VFC_REG6
#define GEMINI_PLL_VFC_REG6_VFC_CBANK_MASK                           MSK_16
#define GEMINI_PLL_VFC_REG6_VFC_CBANK_OFFSET                         0



//[79]  GEMINI_REG_PLL_VFC_REG7
#define GEMINI_PLL_VFC_REG7_VFC_CBANK_P1_MASK                        MSK_16
#define GEMINI_PLL_VFC_REG7_VFC_CBANK_P1_OFFSET                      0



//[80]  GEMINI_REG_PLL_VFC_REG8
#define GEMINI_PLL_VFC_REG8_VFC_CBANK_M1_MASK                        MSK_16
#define GEMINI_PLL_VFC_REG8_VFC_CBANK_M1_OFFSET                      0



//[81]  GEMINI_REG_PLL_VB_REG0
#define GEMINI_PLL_VB_REG0_BIST_START_MASK                           MSK_1
#define GEMINI_PLL_VB_REG0_BIST_START_OFFSET                         15

#define GEMINI_PLL_VB_REG0_BIST_TIME_MASK                            MSK_2
#define GEMINI_PLL_VB_REG0_BIST_TIME_OFFSET                          13

#define GEMINI_PLL_VB_REG0_BIST_WAIT_MASK                            MSK_2
#define GEMINI_PLL_VB_REG0_BIST_WAIT_OFFSET                          11

#define GEMINI_PLL_VB_REG0_EN_MIDVGEN_TC_MASK                        MSK_1
#define GEMINI_PLL_VB_REG0_EN_MIDVGEN_TC_OFFSET                      10

#define GEMINI_PLL_VB_REG0_B0_ONLY_MASK                              MSK_1
#define GEMINI_PLL_VB_REG0_B0_ONLY_OFFSET                            9

#define GEMINI_PLL_VB_REG0_VMID_CTRL_MASK                            MSK_3
#define GEMINI_PLL_VB_REG0_VMID_CTRL_OFFSET                          6

#define GEMINI_PLL_VB_REG0_BIST_LOW_VMID_MASK                        MSK_3
#define GEMINI_PLL_VB_REG0_BIST_LOW_VMID_OFFSET                      3

#define GEMINI_PLL_VB_REG0_BIST_HIGH_VMID_MASK                       MSK_3
#define GEMINI_PLL_VB_REG0_BIST_HIGH_VMID_OFFSET                     0



//[82]  GEMINI_REG_PLL_VB_REG1
#define GEMINI_PLL_VB_REG1_VB_SPARE_0_MASK                           MSK_16
#define GEMINI_PLL_VB_REG1_VB_SPARE_0_OFFSET                         0



//[83]  GEMINI_REG_PLL_VB_REG2
#define GEMINI_PLL_VB_REG2_BIST_OVERLAP_MASK                         MSK_8
#define GEMINI_PLL_VB_REG2_BIST_OVERLAP_OFFSET                       8

#define GEMINI_PLL_VB_REG2_BIST_MONO_MASK                            MSK_8
#define GEMINI_PLL_VB_REG2_BIST_MONO_OFFSET                          0



//[84]  GEMINI_REG_PLL_VB_REG3
#define GEMINI_PLL_VB_REG3_OVERLAP_CNT_DIFF_MASK                     MSK_16
#define GEMINI_PLL_VB_REG3_OVERLAP_CNT_DIFF_OFFSET                   0



//[85]  GEMINI_REG_PLL_VB_REG4
#define GEMINI_PLL_VB_REG4_OVERLAP_CNT_MASK                          MSK_16
#define GEMINI_PLL_VB_REG4_OVERLAP_CNT_OFFSET                        0



//[86]  GEMINI_REG_PLL_VB_REG5
#define GEMINI_PLL_VB_REG5_MONO_CNT_DIFF_MASK                        MSK_16
#define GEMINI_PLL_VB_REG5_MONO_CNT_DIFF_OFFSET                      0



//[87]  GEMINI_REG_PLL_VB_REG6
#define GEMINI_PLL_VB_REG6_MONO_CNT_MASK                             MSK_16
#define GEMINI_PLL_VB_REG6_MONO_CNT_OFFSET                           0



//[88]  GEMINI_REG_PLL_VB_REG7
#define GEMINI_PLL_VB_REG7_B0_MID_CNT_MASK                           MSK_16
#define GEMINI_PLL_VB_REG7_B0_MID_CNT_OFFSET                         0



//[89]  GEMINI_REG_PLL_VB_REG8
#define GEMINI_PLL_VB_REG8_BLAST_MID_CNT_MASK                        MSK_16
#define GEMINI_PLL_VB_REG8_BLAST_MID_CNT_OFFSET                      0



//[90]  GEMINI_REG_PLL_PC_REG0
#define GEMINI_PLL_PC_REG0_FASTCH2_MASK                              MSK_2
#define GEMINI_PLL_PC_REG0_FASTCH2_OFFSET                            14

#define GEMINI_PLL_PC_REG0_FASTCH1_MASK                              MSK_3
#define GEMINI_PLL_PC_REG0_FASTCH1_OFFSET                            11

#define GEMINI_PLL_PC_REG0_VR_VCO_SEL_MASK                           MSK_3
#define GEMINI_PLL_PC_REG0_VR_VCO_SEL_OFFSET                         8

#define GEMINI_PLL_PC_REG0_EN_VCO_VR_2V_MASK                         MSK_2
#define GEMINI_PLL_PC_REG0_EN_VCO_VR_2V_OFFSET                       6

#define GEMINI_PLL_PC_REG0_EN_VCOCORE_2V_MASK                        MSK_2
#define GEMINI_PLL_PC_REG0_EN_VCOCORE_2V_OFFSET                      4

#define GEMINI_PLL_PC_REG0_EN_VCODIV_2V_MASK                         MSK_2
#define GEMINI_PLL_PC_REG0_EN_VCODIV_2V_OFFSET                       2

#define GEMINI_PLL_PC_REG0_EN_VCODIV_MASK                            MSK_2
#define GEMINI_PLL_PC_REG0_EN_VCODIV_OFFSET                          0



//[91]  GEMINI_REG_PLL_PC_REG1
#define GEMINI_PLL_PC_REG1_EN_PSDIV_2V_MASK                          MSK_2
#define GEMINI_PLL_PC_REG1_EN_PSDIV_2V_OFFSET                        14

#define GEMINI_PLL_PC_REG1_EN_PSDIV_MASK                             MSK_2
#define GEMINI_PLL_PC_REG1_EN_PSDIV_OFFSET                           12

#define GEMINI_PLL_PC_REG1_EN_PRNDIV_2V_MASK                         MSK_2
#define GEMINI_PLL_PC_REG1_EN_PRNDIV_2V_OFFSET                       10

#define GEMINI_PLL_PC_REG1_EN_PRNDIV_MASK                            MSK_2
#define GEMINI_PLL_PC_REG1_EN_PRNDIV_OFFSET                          8

#define GEMINI_PLL_PC_REG1_EN_NDIV_S2D_MASK                          MSK_2
#define GEMINI_PLL_PC_REG1_EN_NDIV_S2D_OFFSET                        6

#define GEMINI_PLL_PC_REG1_EN_BIAS_MASK                              MSK_2
#define GEMINI_PLL_PC_REG1_EN_BIAS_OFFSET                            4

#define GEMINI_PLL_PC_REG1_EN_REF_MASK                               MSK_2
#define GEMINI_PLL_PC_REG1_EN_REF_OFFSET                             2

#define GEMINI_PLL_PC_REG1_EN_PFDCP_MASK                             MSK_2
#define GEMINI_PLL_PC_REG1_EN_PFDCP_OFFSET                           0



//[92]  GEMINI_REG_PLL_PC_REG2
#define GEMINI_PLL_PC_REG2_EN_TX_VCOBUF_2V_MASK                      MSK_2
#define GEMINI_PLL_PC_REG2_EN_TX_VCOBUF_2V_OFFSET                    14

#define GEMINI_PLL_PC_REG2_EN_TX_LO_MASK                             MSK_2
#define GEMINI_PLL_PC_REG2_EN_TX_LO_OFFSET                           12

#define GEMINI_PLL_PC_REG2_EN_TXCAL_VCOBUF_2V_MASK                   MSK_2
#define GEMINI_PLL_PC_REG2_EN_TXCAL_VCOBUF_2V_OFFSET                 10

#define GEMINI_PLL_PC_REG2_EN_TXCAL_LO_MASK                          MSK_2
#define GEMINI_PLL_PC_REG2_EN_TXCAL_LO_OFFSET                        8

#define GEMINI_PLL_PC_REG2_EN_RX_VCOBUF0_2V_MASK                     MSK_2
#define GEMINI_PLL_PC_REG2_EN_RX_VCOBUF0_2V_OFFSET                   6

#define GEMINI_PLL_PC_REG2_EN_RX_LO0_MASK                            MSK_2
#define GEMINI_PLL_PC_REG2_EN_RX_LO0_OFFSET                          4

#define GEMINI_PLL_PC_REG2_EN_RX_VCOBUF1_2V_MASK                     MSK_2
#define GEMINI_PLL_PC_REG2_EN_RX_VCOBUF1_2V_OFFSET                   2

#define GEMINI_PLL_PC_REG2_EN_RX_LO1_MASK                            MSK_2
#define GEMINI_PLL_PC_REG2_EN_RX_LO1_OFFSET                          0



//[93]  GEMINI_REG_PLL_PC_REG3
#define GEMINI_PLL_PC_REG3_EN_BSCLK_MUX_2V_MASK                      MSK_2
#define GEMINI_PLL_PC_REG3_EN_BSCLK_MUX_2V_OFFSET                    14

#define GEMINI_PLL_PC_REG3_EN_BSCLK_MUX_MASK                         MSK_2
#define GEMINI_PLL_PC_REG3_EN_BSCLK_MUX_OFFSET                       12

#define GEMINI_PLL_PC_REG3_EN_SPARE_0_MASK                           MSK_2
#define GEMINI_PLL_PC_REG3_EN_SPARE_0_OFFSET                         10

#define GEMINI_PLL_PC_REG3_EN_SPARE_1_MASK                           MSK_2
#define GEMINI_PLL_PC_REG3_EN_SPARE_1_OFFSET                         8

#define GEMINI_PLL_PC_REG3_EN_SPARE_2_MASK                           MSK_2
#define GEMINI_PLL_PC_REG3_EN_SPARE_2_OFFSET                         6

#define GEMINI_PLL_PC_REG3_EN_SPARE_3_MASK                           MSK_2
#define GEMINI_PLL_PC_REG3_EN_SPARE_3_OFFSET                         4

#define GEMINI_PLL_PC_REG3_EN_SPARE_4_MASK                           MSK_2
#define GEMINI_PLL_PC_REG3_EN_SPARE_4_OFFSET                         2

#define GEMINI_PLL_PC_REG3_EN_SPARE_5_MASK                           MSK_2
#define GEMINI_PLL_PC_REG3_EN_SPARE_5_OFFSET                         0



//[94]  GEMINI_REG_PLL_PC_REG4
#define GEMINI_PLL_PC_REG4_RESERVED_MASK                             MSK_8
#define GEMINI_PLL_PC_REG4_RESERVED_OFFSET                           8

#define GEMINI_PLL_PC_REG4_LO_ALWAYS_ON_MASK                         MSK_1
#define GEMINI_PLL_PC_REG4_LO_ALWAYS_ON_OFFSET                       7

#define GEMINI_PLL_PC_REG4_CP_MUX_MASK                               MSK_2
#define GEMINI_PLL_PC_REG4_CP_MUX_OFFSET                             5

#define GEMINI_PLL_PC_REG4_CP_GAIN_MASK                              MSK_5
#define GEMINI_PLL_PC_REG4_CP_GAIN_OFFSET                            0



//[95]  GEMINI_REG_PLL_PC_REG5
#define GEMINI_PLL_PC_REG5_C1_SEL_2_0_MASK                           MSK_3
#define GEMINI_PLL_PC_REG5_C1_SEL_2_0_OFFSET                         13

#define GEMINI_PLL_PC_REG5_R2_SEL_4_0_MASK                           MSK_5
#define GEMINI_PLL_PC_REG5_R2_SEL_4_0_OFFSET                         8

#define GEMINI_PLL_PC_REG5_C3_SEL_1_0_MASK                           MSK_2
#define GEMINI_PLL_PC_REG5_C3_SEL_1_0_OFFSET                         6

#define GEMINI_PLL_PC_REG5_R3_SEL_1_0_MASK                           MSK_2
#define GEMINI_PLL_PC_REG5_R3_SEL_1_0_OFFSET                         4

#define GEMINI_PLL_PC_REG5_EN_R2_BYP_MASK                            MSK_1
#define GEMINI_PLL_PC_REG5_EN_R2_BYP_OFFSET                          3

#define GEMINI_PLL_PC_REG5_EN_R2_RTUNE_MASK                          MSK_1
#define GEMINI_PLL_PC_REG5_EN_R2_RTUNE_OFFSET                        2

#define GEMINI_PLL_PC_REG5_RESERVED_MASK                             MSK_2
#define GEMINI_PLL_PC_REG5_RESERVED_OFFSET                           0



//[96]  GEMINI_REG_PLL_PC_REG6
#define GEMINI_PLL_PC_REG6_PC_SPARE0_MASK                            MSK_16
#define GEMINI_PLL_PC_REG6_PC_SPARE0_OFFSET                          0



//[97]  GEMINI_REG_PLL_AC_REG0
#define GEMINI_PLL_AC_REG0_VCODIV_RCTL_MASK                          MSK_4
#define GEMINI_PLL_AC_REG0_VCODIV_RCTL_OFFSET                        12

#define GEMINI_PLL_AC_REG0_VCODIV_CCTL_MASK                          MSK_4
#define GEMINI_PLL_AC_REG0_VCODIV_CCTL_OFFSET                        8

#define GEMINI_PLL_AC_REG0_PSDIV_RCTL_MASK                           MSK_4
#define GEMINI_PLL_AC_REG0_PSDIV_RCTL_OFFSET                         4

#define GEMINI_PLL_AC_REG0_PSDIV_CCTL_MASK                           MSK_4
#define GEMINI_PLL_AC_REG0_PSDIV_CCTL_OFFSET                         0



//[98]  GEMINI_REG_PLL_AC_REG1
#define GEMINI_PLL_AC_REG1_TXLO_GMCTRL_MASK                          MSK_4
#define GEMINI_PLL_AC_REG1_TXLO_GMCTRL_OFFSET                        12

#define GEMINI_PLL_AC_REG1_TXLOCAL_GMCTRL_MASK                       MSK_4
#define GEMINI_PLL_AC_REG1_TXLOCAL_GMCTRL_OFFSET                     8

#define GEMINI_PLL_AC_REG1_RXLO0_GMCTRL_MASK                         MSK_4
#define GEMINI_PLL_AC_REG1_RXLO0_GMCTRL_OFFSET                       4

#define GEMINI_PLL_AC_REG1_RXLO1_GMCTRL_MASK                         MSK_4
#define GEMINI_PLL_AC_REG1_RXLO1_GMCTRL_OFFSET                       0



//[99]  GEMINI_REG_PLL_AC_REG2
#define GEMINI_PLL_AC_REG2_TSTLO_GMCTRL_MASK                         MSK_4
#define GEMINI_PLL_AC_REG2_TSTLO_GMCTRL_OFFSET                       12

#define GEMINI_PLL_AC_REG2_EXTLO_GMCTRL_MASK                         MSK_4
#define GEMINI_PLL_AC_REG2_EXTLO_GMCTRL_OFFSET                       8

#define GEMINI_PLL_AC_REG2_EN_EXTLO_MASK                             MSK_1
#define GEMINI_PLL_AC_REG2_EN_EXTLO_OFFSET                           7

#define GEMINI_PLL_AC_REG2_EN_EXTLO_TX_MASK                          MSK_1
#define GEMINI_PLL_AC_REG2_EN_EXTLO_TX_OFFSET                        6

#define GEMINI_PLL_AC_REG2_SEL_EXTLO_TXCAL_MASK                      MSK_1
#define GEMINI_PLL_AC_REG2_SEL_EXTLO_TXCAL_OFFSET                    5

#define GEMINI_PLL_AC_REG2_EN_EXTLO_RX0_MASK                         MSK_1
#define GEMINI_PLL_AC_REG2_EN_EXTLO_RX0_OFFSET                       4

#define GEMINI_PLL_AC_REG2_EN_EXTLO_RX1_MASK                         MSK_1
#define GEMINI_PLL_AC_REG2_EN_EXTLO_RX1_OFFSET                       3

#define GEMINI_PLL_AC_REG2_EN_EXTLO_TST_MASK                         MSK_1
#define GEMINI_PLL_AC_REG2_EN_EXTLO_TST_OFFSET                       2

#define GEMINI_PLL_AC_REG2_EN_EXTLO_PDIV_MASK                        MSK_1
#define GEMINI_PLL_AC_REG2_EN_EXTLO_PDIV_OFFSET                      1

#define GEMINI_PLL_AC_REG2_SEL_EXTLO_PSDIV_MASK                      MSK_1
#define GEMINI_PLL_AC_REG2_SEL_EXTLO_PSDIV_OFFSET                    0



//[100]  GEMINI_REG_PLL_AC_REG3
#define GEMINI_PLL_AC_REG3_CPBIAS_CTRL_MASK                          MSK_2
#define GEMINI_PLL_AC_REG3_CPBIAS_CTRL_OFFSET                        14

#define GEMINI_PLL_AC_REG3_CP_UPPW_MASK                              MSK_3
#define GEMINI_PLL_AC_REG3_CP_UPPW_OFFSET                            11

#define GEMINI_PLL_AC_REG3_CP_DNPW_MASK                              MSK_3
#define GEMINI_PLL_AC_REG3_CP_DNPW_OFFSET                            8

#define GEMINI_PLL_AC_REG3_CP_LEAK_3_0_MASK                          MSK_4
#define GEMINI_PLL_AC_REG3_CP_LEAK_3_0_OFFSET                        4

#define GEMINI_PLL_AC_REG3_SEL_CPRST_MASK                            MSK_1
#define GEMINI_PLL_AC_REG3_SEL_CPRST_OFFSET                          3

#define GEMINI_PLL_AC_REG3_CTUNE_VAL_MASK                            MSK_3
#define GEMINI_PLL_AC_REG3_CTUNE_VAL_OFFSET                          0



//[101]  GEMINI_REG_PLL_AC_REG4
#define GEMINI_PLL_AC_REG4_EN_NDIV_S2DPWE_MASK                       MSK_1
#define GEMINI_PLL_AC_REG4_EN_NDIV_S2DPWE_OFFSET                     15

#define GEMINI_PLL_AC_REG4_EN_PCHG_MASK                              MSK_1
#define GEMINI_PLL_AC_REG4_EN_PCHG_OFFSET                            14

#define GEMINI_PLL_AC_REG4_EN_PCHGHALF_MASK                          MSK_1
#define GEMINI_PLL_AC_REG4_EN_PCHGHALF_OFFSET                        13

#define GEMINI_PLL_AC_REG4_EN_EXTVDD_MASK                            MSK_1
#define GEMINI_PLL_AC_REG4_EN_EXTVDD_OFFSET                          12

#define GEMINI_PLL_AC_REG4_EN_VCONTEMP_MASK                          MSK_1
#define GEMINI_PLL_AC_REG4_EN_VCONTEMP_OFFSET                        11

#define GEMINI_PLL_AC_REG4_VCON_TEMP_CTRL_MASK                       MSK_3
#define GEMINI_PLL_AC_REG4_VCON_TEMP_CTRL_OFFSET                     8

#define GEMINI_PLL_AC_REG4_EN_VCO_TST_MASK                           MSK_1
#define GEMINI_PLL_AC_REG4_EN_VCO_TST_OFFSET                         7

#define GEMINI_PLL_AC_REG4_EN_TSTLO_2V_MASK                          MSK_1
#define GEMINI_PLL_AC_REG4_EN_TSTLO_2V_OFFSET                        6

#define GEMINI_PLL_AC_REG4_EN_TSTLO_MASK                             MSK_1
#define GEMINI_PLL_AC_REG4_EN_TSTLO_OFFSET                           5

#define GEMINI_PLL_AC_REG4_EN_REF_BWCAP_MASK                         MSK_1
#define GEMINI_PLL_AC_REG4_EN_REF_BWCAP_OFFSET                       4

#define GEMINI_PLL_AC_REG4_SEL_VCO_TCXO_MASK                         MSK_1
#define GEMINI_PLL_AC_REG4_SEL_VCO_TCXO_OFFSET                       3

#define GEMINI_PLL_AC_REG4_RESERVED_MASK                             MSK_3
#define GEMINI_PLL_AC_REG4_RESERVED_OFFSET                           0



//[102]  GEMINI_REG_PLL_AC_REG5
#define GEMINI_PLL_AC_REG5_RESERVED_MASK                             MSK_11
#define GEMINI_PLL_AC_REG5_RESERVED_OFFSET                           5

#define GEMINI_PLL_AC_REG5_PLL_ATEST_4_0_MASK                        MSK_5
#define GEMINI_PLL_AC_REG5_PLL_ATEST_4_0_OFFSET                      0



//[103]  GEMINI_REG_PLL_AC_REG6
#define GEMINI_PLL_AC_REG6_BSCLKDIV_RCTL_MASK                        MSK_4
#define GEMINI_PLL_AC_REG6_BSCLKDIV_RCTL_OFFSET                      12

#define GEMINI_PLL_AC_REG6_BSCLKDIV_CCTL_MASK                        MSK_4
#define GEMINI_PLL_AC_REG6_BSCLKDIV_CCTL_OFFSET                      8

#define GEMINI_PLL_AC_REG6_TXCALDIV_RCTL_MASK                        MSK_4
#define GEMINI_PLL_AC_REG6_TXCALDIV_RCTL_OFFSET                      4

#define GEMINI_PLL_AC_REG6_TXCALDIV_CCTL_MASK                        MSK_4
#define GEMINI_PLL_AC_REG6_TXCALDIV_CCTL_OFFSET                      0



//[104]  GEMINI_REG_PLL_AC_REG7
#define GEMINI_PLL_AC_REG7_RESERVED_MASK                             MSK_16
#define GEMINI_PLL_AC_REG7_RESERVED_OFFSET                           0



//[105]  GEMINI_REG_PLL_AC_REG8
#define GEMINI_PLL_AC_REG8_RESERVED_MASK                             MSK_16
#define GEMINI_PLL_AC_REG8_RESERVED_OFFSET                           0



//[106]  GEMINI_REG_PLL_BW_REG1
#define GEMINI_PLL_BW_REG1_LBWC_START_MASK                           MSK_1
#define GEMINI_PLL_BW_REG1_LBWC_START_OFFSET                         7

#define GEMINI_PLL_BW_REG1_LBWC_MODE_MASK                            MSK_3
#define GEMINI_PLL_BW_REG1_LBWC_MODE_OFFSET                          4

#define GEMINI_PLL_BW_REG1_CP_WIDTH_MASK                             MSK_2
#define GEMINI_PLL_BW_REG1_CP_WIDTH_OFFSET                           2

#define GEMINI_PLL_BW_REG1_CP_SHFT_MASK                              MSK_2
#define GEMINI_PLL_BW_REG1_CP_SHFT_OFFSET                            0



//[107]  GEMINI_REG_PLL_BW_REG2
#define GEMINI_PLL_BW_REG2_DELTA_NDIV_MASK                           MSK_2
#define GEMINI_PLL_BW_REG2_DELTA_NDIV_OFFSET                         6

#define GEMINI_PLL_BW_REG2_LBWC_SAMPLE_MASK                          MSK_2
#define GEMINI_PLL_BW_REG2_LBWC_SAMPLE_OFFSET                        4

#define GEMINI_PLL_BW_REG2_LBWC_DELTA_MASK                           MSK_2
#define GEMINI_PLL_BW_REG2_LBWC_DELTA_OFFSET                         2

#define GEMINI_PLL_BW_REG2_LBWC_SETTIME_MASK                         MSK_2
#define GEMINI_PLL_BW_REG2_LBWC_SETTIME_OFFSET                       0



//[108]  GEMINI_REG_PLL_BW_REG3
#define GEMINI_PLL_BW_REG3_LBWC_TGOAL_7_0_MASK                       MSK_8
#define GEMINI_PLL_BW_REG3_LBWC_TGOAL_7_0_OFFSET                     0



//[109]  GEMINI_REG_PLL_BW_REG4
#define GEMINI_PLL_BW_REG4_RESERVED_MASK                             MSK_8
#define GEMINI_PLL_BW_REG4_RESERVED_OFFSET                           0



//[110]  GEMINI_REG_PLL_BW_REG5
#define GEMINI_PLL_BW_REG5_LBWC_CAL_CP_MASK                          MSK_8
#define GEMINI_PLL_BW_REG5_LBWC_CAL_CP_OFFSET                        0



//[111]  GEMINI_REG_PLL_BW_REG6
#define GEMINI_PLL_BW_REG6_LBWC_COUNT_7_0_MASK                       MSK_8
#define GEMINI_PLL_BW_REG6_LBWC_COUNT_7_0_OFFSET                     0



//[144]  GEMINI_REG_RX_GAIN_CTL
#define GEMINI_RX_GAIN_CTL_DCSHUNT_EN_MASK                           MSK_1
#define GEMINI_RX_GAIN_CTL_DCSHUNT_EN_OFFSET                         7

#define GEMINI_RX_GAIN_CTL_BAND_SEL_MASK                             MSK_2
#define GEMINI_RX_GAIN_CTL_BAND_SEL_OFFSET                           5

#define GEMINI_RX_GAIN_CTL_RXFE_GC_OVERRIDE_MASK                     MSK_1
#define GEMINI_RX_GAIN_CTL_RXFE_GC_OVERRIDE_OFFSET                   4

#define GEMINI_RX_GAIN_CTL_RX_LNA_GAIN_CONTROL_MASK                  MSK_2
#define GEMINI_RX_GAIN_CTL_RX_LNA_GAIN_CONTROL_OFFSET                2

#define GEMINI_RX_GAIN_CTL_RX_MIX_GAIN_CONTROL_MASK                  MSK_2
#define GEMINI_RX_GAIN_CTL_RX_MIX_GAIN_CONTROL_OFFSET                0



//[145]  GEMINI_REG_RX_PWR_CTL
#define GEMINI_RX_PWR_CTL_LNADUM_EN_MASK                             MSK_1
#define GEMINI_RX_PWR_CTL_LNADUM_EN_OFFSET                           7

#define GEMINI_RX_PWR_CTL_LNASHUNT_EN_MASK                           MSK_1
#define GEMINI_RX_PWR_CTL_LNASHUNT_EN_OFFSET                         6

#define GEMINI_RX_PWR_CTL_RESERVED_MASK                              MSK_1
#define GEMINI_RX_PWR_CTL_RESERVED_OFFSET                            5

#define GEMINI_RX_PWR_CTL_DCCAL_GAIN_MASK                            MSK_1
#define GEMINI_RX_PWR_CTL_DCCAL_GAIN_OFFSET                          4

#define GEMINI_RX_PWR_CTL_EN_RX_MX_MASK                              MSK_2
#define GEMINI_RX_PWR_CTL_EN_RX_MX_OFFSET                            2

#define GEMINI_RX_PWR_CTL_EN_RX_LNA_MASK                             MSK_2
#define GEMINI_RX_PWR_CTL_EN_RX_LNA_OFFSET                           0



//[146]  GEMINI_REG_RX_LNA_CURRENT
#define GEMINI_RX_LNA_CURRENT_RXHB_IREF_TRIM_MASK                    MSK_4
#define GEMINI_RX_LNA_CURRENT_RXHB_IREF_TRIM_OFFSET                  4

#define GEMINI_RX_LNA_CURRENT_RXLB_IREF_TRIM_MASK                    MSK_4
#define GEMINI_RX_LNA_CURRENT_RXLB_IREF_TRIM_OFFSET                  0



//[147]  GEMINI_REG_RX_LNA_LB_LOAD
#define GEMINI_RX_LNA_LB_LOAD_RXLB_LNA_LOAD_CTRIM_MASK               MSK_4
#define GEMINI_RX_LNA_LB_LOAD_RXLB_LNA_LOAD_CTRIM_OFFSET             4

#define GEMINI_RX_LNA_LB_LOAD_RXLB_LNA_LOAD_RTRIM_MASK               MSK_4
#define GEMINI_RX_LNA_LB_LOAD_RXLB_LNA_LOAD_RTRIM_OFFSET             0



//[148]  GEMINI_REG_RX_LNA_HB_LOAD
#define GEMINI_RX_LNA_HB_LOAD_RXHB_LNA_LOAD_CTRIM_MASK               MSK_4
#define GEMINI_RX_LNA_HB_LOAD_RXHB_LNA_LOAD_CTRIM_OFFSET             4

#define GEMINI_RX_LNA_HB_LOAD_RXHB_LNA_LOAD_RTRIM_MASK               MSK_4
#define GEMINI_RX_LNA_HB_LOAD_RXHB_LNA_LOAD_RTRIM_OFFSET             0



//[149]  GEMINI_REG_RX_LNA_LB_MATCH
#define GEMINI_RX_LNA_LB_MATCH_RXLB_LNA_MATCH_CTRIM_MASK             MSK_4
#define GEMINI_RX_LNA_LB_MATCH_RXLB_LNA_MATCH_CTRIM_OFFSET           4

#define GEMINI_RX_LNA_LB_MATCH_RXLB_LNA_MATCH_LTRIM_MASK             MSK_4
#define GEMINI_RX_LNA_LB_MATCH_RXLB_LNA_MATCH_LTRIM_OFFSET           0



//[150]  GEMINI_REG_RX_LNA_HB_MATCH
#define GEMINI_RX_LNA_HB_MATCH_RXHB_LNA_MATCH_CTRIM_MASK             MSK_4
#define GEMINI_RX_LNA_HB_MATCH_RXHB_LNA_MATCH_CTRIM_OFFSET           4

#define GEMINI_RX_LNA_HB_MATCH_RXHB_LNA_MATCH_LTRIM_MASK             MSK_4
#define GEMINI_RX_LNA_HB_MATCH_RXHB_LNA_MATCH_LTRIM_OFFSET           0



//[151]  GEMINI_REG_RX_LNA_LB_VCM
#define GEMINI_RX_LNA_LB_VCM_RESERVED_MASK                           MSK_4
#define GEMINI_RX_LNA_LB_VCM_RESERVED_OFFSET                         4

#define GEMINI_RX_LNA_LB_VCM_RXFE_VCM_TRIM_MASK                      MSK_4
#define GEMINI_RX_LNA_LB_VCM_RXFE_VCM_TRIM_OFFSET                    0



//[152]  GEMINI_REG_RX_LNA_VG
#define GEMINI_RX_LNA_VG_RXHB_VG2_TRIM_MASK                          MSK_4
#define GEMINI_RX_LNA_VG_RXHB_VG2_TRIM_OFFSET                        4

#define GEMINI_RX_LNA_VG_RXLB_VG2_TRIM_MASK                          MSK_4
#define GEMINI_RX_LNA_VG_RXLB_VG2_TRIM_OFFSET                        0



//[153]  GEMINI_REG_RXFE_CTL
#define GEMINI_RXFE_CTL_EN_HB_VG_MASK                                MSK_1
#define GEMINI_RXFE_CTL_EN_HB_VG_OFFSET                              7

#define GEMINI_RXFE_CTL_EN_LB_VG_MASK                                MSK_1
#define GEMINI_RXFE_CTL_EN_LB_VG_OFFSET                              6

#define GEMINI_RXFE_CTL_EN_LB_VCM_MASK                               MSK_1
#define GEMINI_RXFE_CTL_EN_LB_VCM_OFFSET                             5

#define GEMINI_RXFE_CTL_LOOPBACK_EN_MASK                             MSK_2
#define GEMINI_RXFE_CTL_LOOPBACK_EN_OFFSET                           3

#define GEMINI_RXFE_CTL_LOW_LIN_EN_MASK                              MSK_1
#define GEMINI_RXFE_CTL_LOW_LIN_EN_OFFSET                            2

#define GEMINI_RXFE_CTL_IREF_SEL_MASK                                MSK_1
#define GEMINI_RXFE_CTL_IREF_SEL_OFFSET                              1

#define GEMINI_RXFE_CTL_STAB_EN_MASK                                 MSK_1
#define GEMINI_RXFE_CTL_STAB_EN_OFFSET                               0



//[154]  GEMINI_REG_RXLB_LO_BIAS
#define GEMINI_RXLB_LO_BIAS_RXLB_LO_BCTRL_MASK                       MSK_2
#define GEMINI_RXLB_LO_BIAS_RXLB_LO_BCTRL_OFFSET                     6

#define GEMINI_RXLB_LO_BIAS_RXLB_LO_ICTRL_MASK                       MSK_4
#define GEMINI_RXLB_LO_BIAS_RXLB_LO_ICTRL_OFFSET                     2

#define GEMINI_RXLB_LO_BIAS_EN_RXLB_LOBUF_MASK                       MSK_2
#define GEMINI_RXLB_LO_BIAS_EN_RXLB_LOBUF_OFFSET                     0



//[155]  GEMINI_REG_RXHB_LO_BIAS
#define GEMINI_RXHB_LO_BIAS_RXHB_LO_BCTRL_MASK                       MSK_2
#define GEMINI_RXHB_LO_BIAS_RXHB_LO_BCTRL_OFFSET                     6

#define GEMINI_RXHB_LO_BIAS_RXHB_LO_ICTRL_MASK                       MSK_4
#define GEMINI_RXHB_LO_BIAS_RXHB_LO_ICTRL_OFFSET                     2

#define GEMINI_RXHB_LO_BIAS_EN_RXHB_LOBUF_MASK                       MSK_2
#define GEMINI_RXHB_LO_BIAS_EN_RXHB_LOBUF_OFFSET                     0



//[156]  GEMINI_REG_RXFE_SPARE1
#define GEMINI_RXFE_SPARE1_RESERVED_MASK                             MSK_8
#define GEMINI_RXFE_SPARE1_RESERVED_OFFSET                           0



//[157]  GEMINI_REG_RXFE_SPARE2
#define GEMINI_RXFE_SPARE2_RESERVED_MASK                             MSK_8
#define GEMINI_RXFE_SPARE2_RESERVED_OFFSET                           0



//[158]  GEMINI_REG_RXFE_SPARE3
#define GEMINI_RXFE_SPARE3_RESERVED_MASK                             MSK_8
#define GEMINI_RXFE_SPARE3_RESERVED_OFFSET                           0



//[159]  GEMINI_REG_RXFE_SPARE4
#define GEMINI_RXFE_SPARE4_RESERVED_MASK                             MSK_8
#define GEMINI_RXFE_SPARE4_RESERVED_OFFSET                           0



//[160]  GEMINI_REG_RX_GC_0
#define GEMINI_RX_GC_0_RXFE_GAIN_MASK                                MSK_4
#define GEMINI_RX_GC_0_RXFE_GAIN_OFFSET                              11

#define GEMINI_RX_GC_0_BBF_TIA_GAIN_RX_MASK                          MSK_1
#define GEMINI_RX_GC_0_BBF_TIA_GAIN_RX_OFFSET                        10

#define GEMINI_RX_GC_0_BBF_BQ1_GAIN_RX_MASK                          MSK_2
#define GEMINI_RX_GC_0_BBF_BQ1_GAIN_RX_OFFSET                        8

#define GEMINI_RX_GC_0_BBF_BQ2_GAIN_RX_MASK                          MSK_2
#define GEMINI_RX_GC_0_BBF_BQ2_GAIN_RX_OFFSET                        6

#define GEMINI_RX_GC_0_BBF_PGA_GAIN_RX_MASK                          MSK_6
#define GEMINI_RX_GC_0_BBF_PGA_GAIN_RX_OFFSET                        0



//[161]  GEMINI_REG_PA_GC
#define GEMINI_PA_GC_PA_GC_MASK                                      MSK_3
#define GEMINI_PA_GC_PA_GC_OFFSET                                    13

#define GEMINI_PA_GC_IB_VTOI_MASK                                    MSK_2
#define GEMINI_PA_GC_IB_VTOI_OFFSET                                  11

#define GEMINI_PA_GC_G_VTOI_MASK                                     MSK_5
#define GEMINI_PA_GC_G_VTOI_OFFSET                                   6

#define GEMINI_PA_GC_BBF_PGA_GAIN_TX_MASK                            MSK_6
#define GEMINI_PA_GC_BBF_PGA_GAIN_TX_OFFSET                          0



//[162]  GEMINI_REG_RX_DCOC_IQ
#define GEMINI_RX_DCOC_IQ_RX_DCOC_I_MASK                             MSK_8
#define GEMINI_RX_DCOC_IQ_RX_DCOC_I_OFFSET                           8

#define GEMINI_RX_DCOC_IQ_RX_DCOC_Q_MASK                             MSK_8
#define GEMINI_RX_DCOC_IQ_RX_DCOC_Q_OFFSET                           0



//[163]  GEMINI_REG_TX_DCOC_IQ
#define GEMINI_TX_DCOC_IQ_TX_DCOC_I_MASK                             MSK_8
#define GEMINI_TX_DCOC_IQ_TX_DCOC_I_OFFSET                           8

#define GEMINI_TX_DCOC_IQ_TX_DCOC_Q_MASK                             MSK_8
#define GEMINI_TX_DCOC_IQ_TX_DCOC_Q_OFFSET                           0



//[164]  GEMINI_REG_RX_DCOC_EN0
#define GEMINI_RX_DCOC_EN0_RX_DCOC_EN_V_15_MASK                      MSK_1
#define GEMINI_RX_DCOC_EN0_RX_DCOC_EN_V_15_OFFSET                    15

#define GEMINI_RX_DCOC_EN0_RX_DCOC_EN_V_14_MASK                      MSK_1
#define GEMINI_RX_DCOC_EN0_RX_DCOC_EN_V_14_OFFSET                    14

#define GEMINI_RX_DCOC_EN0_RX_DCOC_EN_V_13_MASK                      MSK_1
#define GEMINI_RX_DCOC_EN0_RX_DCOC_EN_V_13_OFFSET                    13

#define GEMINI_RX_DCOC_EN0_RX_DCOC_EN_V_12_MASK                      MSK_1
#define GEMINI_RX_DCOC_EN0_RX_DCOC_EN_V_12_OFFSET                    12

#define GEMINI_RX_DCOC_EN0_RX_DCOC_EN_V_11_MASK                      MSK_1
#define GEMINI_RX_DCOC_EN0_RX_DCOC_EN_V_11_OFFSET                    11

#define GEMINI_RX_DCOC_EN0_RX_DCOC_EN_V_10_MASK                      MSK_1
#define GEMINI_RX_DCOC_EN0_RX_DCOC_EN_V_10_OFFSET                    10

#define GEMINI_RX_DCOC_EN0_RX_DCOC_EN_V_9_MASK                       MSK_1
#define GEMINI_RX_DCOC_EN0_RX_DCOC_EN_V_9_OFFSET                     9

#define GEMINI_RX_DCOC_EN0_RX_DCOC_EN_V_8_MASK                       MSK_1
#define GEMINI_RX_DCOC_EN0_RX_DCOC_EN_V_8_OFFSET                     8

#define GEMINI_RX_DCOC_EN0_RX_DCOC_EN_V_7_MASK                       MSK_1
#define GEMINI_RX_DCOC_EN0_RX_DCOC_EN_V_7_OFFSET                     7

#define GEMINI_RX_DCOC_EN0_RX_DCOC_EN_V_6_MASK                       MSK_1
#define GEMINI_RX_DCOC_EN0_RX_DCOC_EN_V_6_OFFSET                     6

#define GEMINI_RX_DCOC_EN0_RX_DCOC_EN_V_5_MASK                       MSK_1
#define GEMINI_RX_DCOC_EN0_RX_DCOC_EN_V_5_OFFSET                     5

#define GEMINI_RX_DCOC_EN0_RX_DCOC_EN_V_4_MASK                       MSK_1
#define GEMINI_RX_DCOC_EN0_RX_DCOC_EN_V_4_OFFSET                     4

#define GEMINI_RX_DCOC_EN0_RX_DCOC_EN_V_3_MASK                       MSK_1
#define GEMINI_RX_DCOC_EN0_RX_DCOC_EN_V_3_OFFSET                     3

#define GEMINI_RX_DCOC_EN0_RX_DCOC_EN_V_2_MASK                       MSK_1
#define GEMINI_RX_DCOC_EN0_RX_DCOC_EN_V_2_OFFSET                     2

#define GEMINI_RX_DCOC_EN0_RX_DCOC_EN_V_1_MASK                       MSK_1
#define GEMINI_RX_DCOC_EN0_RX_DCOC_EN_V_1_OFFSET                     1

#define GEMINI_RX_DCOC_EN0_RX_DCOC_EN_V_0_MASK                       MSK_1
#define GEMINI_RX_DCOC_EN0_RX_DCOC_EN_V_0_OFFSET                     0



//[165]  GEMINI_REG_RX_DCOC_EN1
#define GEMINI_RX_DCOC_EN1_RX_DCOC_EN_V_31_MASK                      MSK_1
#define GEMINI_RX_DCOC_EN1_RX_DCOC_EN_V_31_OFFSET                    15

#define GEMINI_RX_DCOC_EN1_RX_DCOC_EN_V_30_MASK                      MSK_1
#define GEMINI_RX_DCOC_EN1_RX_DCOC_EN_V_30_OFFSET                    14

#define GEMINI_RX_DCOC_EN1_RX_DCOC_EN_V_29_MASK                      MSK_1
#define GEMINI_RX_DCOC_EN1_RX_DCOC_EN_V_29_OFFSET                    13

#define GEMINI_RX_DCOC_EN1_RX_DCOC_EN_V_28_MASK                      MSK_1
#define GEMINI_RX_DCOC_EN1_RX_DCOC_EN_V_28_OFFSET                    12

#define GEMINI_RX_DCOC_EN1_RX_DCOC_EN_V_27_MASK                      MSK_1
#define GEMINI_RX_DCOC_EN1_RX_DCOC_EN_V_27_OFFSET                    11

#define GEMINI_RX_DCOC_EN1_RX_DCOC_EN_V_26_MASK                      MSK_1
#define GEMINI_RX_DCOC_EN1_RX_DCOC_EN_V_26_OFFSET                    10

#define GEMINI_RX_DCOC_EN1_RX_DCOC_EN_V_25_MASK                      MSK_1
#define GEMINI_RX_DCOC_EN1_RX_DCOC_EN_V_25_OFFSET                    9

#define GEMINI_RX_DCOC_EN1_RX_DCOC_EN_V_24_MASK                      MSK_1
#define GEMINI_RX_DCOC_EN1_RX_DCOC_EN_V_24_OFFSET                    8

#define GEMINI_RX_DCOC_EN1_RX_DCOC_EN_V_23_MASK                      MSK_1
#define GEMINI_RX_DCOC_EN1_RX_DCOC_EN_V_23_OFFSET                    7

#define GEMINI_RX_DCOC_EN1_RX_DCOC_EN_V_22_MASK                      MSK_1
#define GEMINI_RX_DCOC_EN1_RX_DCOC_EN_V_22_OFFSET                    6

#define GEMINI_RX_DCOC_EN1_RX_DCOC_EN_V_21_MASK                      MSK_1
#define GEMINI_RX_DCOC_EN1_RX_DCOC_EN_V_21_OFFSET                    5

#define GEMINI_RX_DCOC_EN1_RX_DCOC_EN_V_20_MASK                      MSK_1
#define GEMINI_RX_DCOC_EN1_RX_DCOC_EN_V_20_OFFSET                    4

#define GEMINI_RX_DCOC_EN1_RX_DCOC_EN_V_19_MASK                      MSK_1
#define GEMINI_RX_DCOC_EN1_RX_DCOC_EN_V_19_OFFSET                    3

#define GEMINI_RX_DCOC_EN1_RX_DCOC_EN_V_18_MASK                      MSK_1
#define GEMINI_RX_DCOC_EN1_RX_DCOC_EN_V_18_OFFSET                    2

#define GEMINI_RX_DCOC_EN1_RX_DCOC_EN_V_17_MASK                      MSK_1
#define GEMINI_RX_DCOC_EN1_RX_DCOC_EN_V_17_OFFSET                    1

#define GEMINI_RX_DCOC_EN1_RX_DCOC_EN_V_16_MASK                      MSK_1
#define GEMINI_RX_DCOC_EN1_RX_DCOC_EN_V_16_OFFSET                    0



//[166]  GEMINI_REG_RX_DCOC_RANGE0
#define GEMINI_RX_DCOC_RANGE0_RX_DCOC_RANGE_V_7_MASK                 MSK_2
#define GEMINI_RX_DCOC_RANGE0_RX_DCOC_RANGE_V_7_OFFSET               14

#define GEMINI_RX_DCOC_RANGE0_RX_DCOC_RANGE_V_6_MASK                 MSK_2
#define GEMINI_RX_DCOC_RANGE0_RX_DCOC_RANGE_V_6_OFFSET               12

#define GEMINI_RX_DCOC_RANGE0_RX_DCOC_RANGE_V_5_MASK                 MSK_2
#define GEMINI_RX_DCOC_RANGE0_RX_DCOC_RANGE_V_5_OFFSET               10

#define GEMINI_RX_DCOC_RANGE0_RX_DCOC_RANGE_V_4_MASK                 MSK_2
#define GEMINI_RX_DCOC_RANGE0_RX_DCOC_RANGE_V_4_OFFSET               8

#define GEMINI_RX_DCOC_RANGE0_RX_DCOC_RANGE_V_3_MASK                 MSK_2
#define GEMINI_RX_DCOC_RANGE0_RX_DCOC_RANGE_V_3_OFFSET               6

#define GEMINI_RX_DCOC_RANGE0_RX_DCOC_RANGE_V_2_MASK                 MSK_2
#define GEMINI_RX_DCOC_RANGE0_RX_DCOC_RANGE_V_2_OFFSET               4

#define GEMINI_RX_DCOC_RANGE0_RX_DCOC_RANGE_V_1_MASK                 MSK_2
#define GEMINI_RX_DCOC_RANGE0_RX_DCOC_RANGE_V_1_OFFSET               2

#define GEMINI_RX_DCOC_RANGE0_RX_DCOC_RANGE_V_0_MASK                 MSK_2
#define GEMINI_RX_DCOC_RANGE0_RX_DCOC_RANGE_V_0_OFFSET               0



//[167]  GEMINI_REG_RX_DCOC_RANGE1
#define GEMINI_RX_DCOC_RANGE1_RX_DCOC_RANGE_V_15_MASK                MSK_2
#define GEMINI_RX_DCOC_RANGE1_RX_DCOC_RANGE_V_15_OFFSET              14

#define GEMINI_RX_DCOC_RANGE1_RX_DCOC_RANGE_V_14_MASK                MSK_2
#define GEMINI_RX_DCOC_RANGE1_RX_DCOC_RANGE_V_14_OFFSET              12

#define GEMINI_RX_DCOC_RANGE1_RX_DCOC_RANGE_V_13_MASK                MSK_2
#define GEMINI_RX_DCOC_RANGE1_RX_DCOC_RANGE_V_13_OFFSET              10

#define GEMINI_RX_DCOC_RANGE1_RX_DCOC_RANGE_V_12_MASK                MSK_2
#define GEMINI_RX_DCOC_RANGE1_RX_DCOC_RANGE_V_12_OFFSET              8

#define GEMINI_RX_DCOC_RANGE1_RX_DCOC_RANGE_V_11_MASK                MSK_2
#define GEMINI_RX_DCOC_RANGE1_RX_DCOC_RANGE_V_11_OFFSET              6

#define GEMINI_RX_DCOC_RANGE1_RX_DCOC_RANGE_V_10_MASK                MSK_2
#define GEMINI_RX_DCOC_RANGE1_RX_DCOC_RANGE_V_10_OFFSET              4

#define GEMINI_RX_DCOC_RANGE1_RX_DCOC_RANGE_V_9_MASK                 MSK_2
#define GEMINI_RX_DCOC_RANGE1_RX_DCOC_RANGE_V_9_OFFSET               2

#define GEMINI_RX_DCOC_RANGE1_RX_DCOC_RANGE_V_8_MASK                 MSK_2
#define GEMINI_RX_DCOC_RANGE1_RX_DCOC_RANGE_V_8_OFFSET               0



//[168]  GEMINI_REG_RX_DCOC_RANGE2
#define GEMINI_RX_DCOC_RANGE2_RX_DCOC_RANGE_V_23_MASK                MSK_2
#define GEMINI_RX_DCOC_RANGE2_RX_DCOC_RANGE_V_23_OFFSET              14

#define GEMINI_RX_DCOC_RANGE2_RX_DCOC_RANGE_V_22_MASK                MSK_2
#define GEMINI_RX_DCOC_RANGE2_RX_DCOC_RANGE_V_22_OFFSET              12

#define GEMINI_RX_DCOC_RANGE2_RX_DCOC_RANGE_V_21_MASK                MSK_2
#define GEMINI_RX_DCOC_RANGE2_RX_DCOC_RANGE_V_21_OFFSET              10

#define GEMINI_RX_DCOC_RANGE2_RX_DCOC_RANGE_V_20_MASK                MSK_2
#define GEMINI_RX_DCOC_RANGE2_RX_DCOC_RANGE_V_20_OFFSET              8

#define GEMINI_RX_DCOC_RANGE2_RX_DCOC_RANGE_V_19_MASK                MSK_2
#define GEMINI_RX_DCOC_RANGE2_RX_DCOC_RANGE_V_19_OFFSET              6

#define GEMINI_RX_DCOC_RANGE2_RX_DCOC_RANGE_V_18_MASK                MSK_2
#define GEMINI_RX_DCOC_RANGE2_RX_DCOC_RANGE_V_18_OFFSET              4

#define GEMINI_RX_DCOC_RANGE2_RX_DCOC_RANGE_V_17_MASK                MSK_2
#define GEMINI_RX_DCOC_RANGE2_RX_DCOC_RANGE_V_17_OFFSET              2

#define GEMINI_RX_DCOC_RANGE2_RX_DCOC_RANGE_V_16_MASK                MSK_2
#define GEMINI_RX_DCOC_RANGE2_RX_DCOC_RANGE_V_16_OFFSET              0



//[169]  GEMINI_REG_RX_DCOC_RANGE3
#define GEMINI_RX_DCOC_RANGE3_RX_DCOC_RANGE_V_31_MASK                MSK_2
#define GEMINI_RX_DCOC_RANGE3_RX_DCOC_RANGE_V_31_OFFSET              14

#define GEMINI_RX_DCOC_RANGE3_RX_DCOC_RANGE_V_30_MASK                MSK_2
#define GEMINI_RX_DCOC_RANGE3_RX_DCOC_RANGE_V_30_OFFSET              12

#define GEMINI_RX_DCOC_RANGE3_RX_DCOC_RANGE_V_29_MASK                MSK_2
#define GEMINI_RX_DCOC_RANGE3_RX_DCOC_RANGE_V_29_OFFSET              10

#define GEMINI_RX_DCOC_RANGE3_RX_DCOC_RANGE_V_28_MASK                MSK_2
#define GEMINI_RX_DCOC_RANGE3_RX_DCOC_RANGE_V_28_OFFSET              8

#define GEMINI_RX_DCOC_RANGE3_RX_DCOC_RANGE_V_27_MASK                MSK_2
#define GEMINI_RX_DCOC_RANGE3_RX_DCOC_RANGE_V_27_OFFSET              6

#define GEMINI_RX_DCOC_RANGE3_RX_DCOC_RANGE_V_26_MASK                MSK_2
#define GEMINI_RX_DCOC_RANGE3_RX_DCOC_RANGE_V_26_OFFSET              4

#define GEMINI_RX_DCOC_RANGE3_RX_DCOC_RANGE_V_25_MASK                MSK_2
#define GEMINI_RX_DCOC_RANGE3_RX_DCOC_RANGE_V_25_OFFSET              2

#define GEMINI_RX_DCOC_RANGE3_RX_DCOC_RANGE_V_24_MASK                MSK_2
#define GEMINI_RX_DCOC_RANGE3_RX_DCOC_RANGE_V_24_OFFSET              0



//[170]  GEMINI_REG_TX_DCOC_EN0
#define GEMINI_TX_DCOC_EN0_TX_DCOC_EN_V_15_MASK                      MSK_1
#define GEMINI_TX_DCOC_EN0_TX_DCOC_EN_V_15_OFFSET                    15

#define GEMINI_TX_DCOC_EN0_TX_DCOC_EN_V_14_MASK                      MSK_1
#define GEMINI_TX_DCOC_EN0_TX_DCOC_EN_V_14_OFFSET                    14

#define GEMINI_TX_DCOC_EN0_TX_DCOC_EN_V_13_MASK                      MSK_1
#define GEMINI_TX_DCOC_EN0_TX_DCOC_EN_V_13_OFFSET                    13

#define GEMINI_TX_DCOC_EN0_TX_DCOC_EN_V_12_MASK                      MSK_1
#define GEMINI_TX_DCOC_EN0_TX_DCOC_EN_V_12_OFFSET                    12

#define GEMINI_TX_DCOC_EN0_TX_DCOC_EN_V_11_MASK                      MSK_1
#define GEMINI_TX_DCOC_EN0_TX_DCOC_EN_V_11_OFFSET                    11

#define GEMINI_TX_DCOC_EN0_TX_DCOC_EN_V_10_MASK                      MSK_1
#define GEMINI_TX_DCOC_EN0_TX_DCOC_EN_V_10_OFFSET                    10

#define GEMINI_TX_DCOC_EN0_TX_DCOC_EN_V_9_MASK                       MSK_1
#define GEMINI_TX_DCOC_EN0_TX_DCOC_EN_V_9_OFFSET                     9

#define GEMINI_TX_DCOC_EN0_TX_DCOC_EN_V_8_MASK                       MSK_1
#define GEMINI_TX_DCOC_EN0_TX_DCOC_EN_V_8_OFFSET                     8

#define GEMINI_TX_DCOC_EN0_TX_DCOC_EN_V_7_MASK                       MSK_1
#define GEMINI_TX_DCOC_EN0_TX_DCOC_EN_V_7_OFFSET                     7

#define GEMINI_TX_DCOC_EN0_TX_DCOC_EN_V_6_MASK                       MSK_1
#define GEMINI_TX_DCOC_EN0_TX_DCOC_EN_V_6_OFFSET                     6

#define GEMINI_TX_DCOC_EN0_TX_DCOC_EN_V_5_MASK                       MSK_1
#define GEMINI_TX_DCOC_EN0_TX_DCOC_EN_V_5_OFFSET                     5

#define GEMINI_TX_DCOC_EN0_TX_DCOC_EN_V_4_MASK                       MSK_1
#define GEMINI_TX_DCOC_EN0_TX_DCOC_EN_V_4_OFFSET                     4

#define GEMINI_TX_DCOC_EN0_TX_DCOC_EN_V_3_MASK                       MSK_1
#define GEMINI_TX_DCOC_EN0_TX_DCOC_EN_V_3_OFFSET                     3

#define GEMINI_TX_DCOC_EN0_TX_DCOC_EN_V_2_MASK                       MSK_1
#define GEMINI_TX_DCOC_EN0_TX_DCOC_EN_V_2_OFFSET                     2

#define GEMINI_TX_DCOC_EN0_TX_DCOC_EN_V_1_MASK                       MSK_1
#define GEMINI_TX_DCOC_EN0_TX_DCOC_EN_V_1_OFFSET                     1

#define GEMINI_TX_DCOC_EN0_TX_DCOC_EN_V_0_MASK                       MSK_1
#define GEMINI_TX_DCOC_EN0_TX_DCOC_EN_V_0_OFFSET                     0



//[171]  GEMINI_REG_TX_DCOC_RANGE0
#define GEMINI_TX_DCOC_RANGE0_TX_DCOC_RANGE_V_7_MASK                 MSK_2
#define GEMINI_TX_DCOC_RANGE0_TX_DCOC_RANGE_V_7_OFFSET               14

#define GEMINI_TX_DCOC_RANGE0_TX_DCOC_RANGE_V_6_MASK                 MSK_2
#define GEMINI_TX_DCOC_RANGE0_TX_DCOC_RANGE_V_6_OFFSET               12

#define GEMINI_TX_DCOC_RANGE0_TX_DCOC_RANGE_V_5_MASK                 MSK_2
#define GEMINI_TX_DCOC_RANGE0_TX_DCOC_RANGE_V_5_OFFSET               10

#define GEMINI_TX_DCOC_RANGE0_TX_DCOC_RANGE_V_4_MASK                 MSK_2
#define GEMINI_TX_DCOC_RANGE0_TX_DCOC_RANGE_V_4_OFFSET               8

#define GEMINI_TX_DCOC_RANGE0_TX_DCOC_RANGE_V_3_MASK                 MSK_2
#define GEMINI_TX_DCOC_RANGE0_TX_DCOC_RANGE_V_3_OFFSET               6

#define GEMINI_TX_DCOC_RANGE0_TX_DCOC_RANGE_V_2_MASK                 MSK_2
#define GEMINI_TX_DCOC_RANGE0_TX_DCOC_RANGE_V_2_OFFSET               4

#define GEMINI_TX_DCOC_RANGE0_TX_DCOC_RANGE_V_1_MASK                 MSK_2
#define GEMINI_TX_DCOC_RANGE0_TX_DCOC_RANGE_V_1_OFFSET               2

#define GEMINI_TX_DCOC_RANGE0_TX_DCOC_RANGE_V_0_MASK                 MSK_2
#define GEMINI_TX_DCOC_RANGE0_TX_DCOC_RANGE_V_0_OFFSET               0



//[172]  GEMINI_REG_TX_DCOC_RANGE1
#define GEMINI_TX_DCOC_RANGE1_TX_DCOC_RANGE_V_15_MASK                MSK_2
#define GEMINI_TX_DCOC_RANGE1_TX_DCOC_RANGE_V_15_OFFSET              14

#define GEMINI_TX_DCOC_RANGE1_TX_DCOC_RANGE_V_14_MASK                MSK_2
#define GEMINI_TX_DCOC_RANGE1_TX_DCOC_RANGE_V_14_OFFSET              12

#define GEMINI_TX_DCOC_RANGE1_TX_DCOC_RANGE_V_13_MASK                MSK_2
#define GEMINI_TX_DCOC_RANGE1_TX_DCOC_RANGE_V_13_OFFSET              10

#define GEMINI_TX_DCOC_RANGE1_TX_DCOC_RANGE_V_12_MASK                MSK_2
#define GEMINI_TX_DCOC_RANGE1_TX_DCOC_RANGE_V_12_OFFSET              8

#define GEMINI_TX_DCOC_RANGE1_TX_DCOC_RANGE_V_11_MASK                MSK_2
#define GEMINI_TX_DCOC_RANGE1_TX_DCOC_RANGE_V_11_OFFSET              6

#define GEMINI_TX_DCOC_RANGE1_TX_DCOC_RANGE_V_10_MASK                MSK_2
#define GEMINI_TX_DCOC_RANGE1_TX_DCOC_RANGE_V_10_OFFSET              4

#define GEMINI_TX_DCOC_RANGE1_TX_DCOC_RANGE_V_9_MASK                 MSK_2
#define GEMINI_TX_DCOC_RANGE1_TX_DCOC_RANGE_V_9_OFFSET               2

#define GEMINI_TX_DCOC_RANGE1_TX_DCOC_RANGE_V_8_MASK                 MSK_2
#define GEMINI_TX_DCOC_RANGE1_TX_DCOC_RANGE_V_8_OFFSET               0



//[173]  GEMINI_REG_TX_BBF_GAIN
#define GEMINI_TX_BBF_GAIN_RESERVED_MASK                             MSK_3
#define GEMINI_TX_BBF_GAIN_RESERVED_OFFSET                           5

#define GEMINI_TX_BBF_GAIN_BBF_TIA_GAIN_TX_MASK                      MSK_1
#define GEMINI_TX_BBF_GAIN_BBF_TIA_GAIN_TX_OFFSET                    4

#define GEMINI_TX_BBF_GAIN_BBF_BQ1_GAIN_TX_MASK                      MSK_2
#define GEMINI_TX_BBF_GAIN_BBF_BQ1_GAIN_TX_OFFSET                    2

#define GEMINI_TX_BBF_GAIN_BBF_BQ2_GAIN_TX_MASK                      MSK_2
#define GEMINI_TX_BBF_GAIN_BBF_BQ2_GAIN_TX_OFFSET                    0



//[174]  GEMINI_REG_RX_DCOC_CTL
#define GEMINI_RX_DCOC_CTL_RX_DCOC_IDX_SEL_MASK                      MSK_2
#define GEMINI_RX_DCOC_CTL_RX_DCOC_IDX_SEL_OFFSET                    6

#define GEMINI_RX_DCOC_CTL_RX_DCOC_IDX_OVER_MASK                     MSK_1
#define GEMINI_RX_DCOC_CTL_RX_DCOC_IDX_OVER_OFFSET                   5

#define GEMINI_RX_DCOC_CTL_RX_DCOC_INDEX_MASK                        MSK_5
#define GEMINI_RX_DCOC_CTL_RX_DCOC_INDEX_OFFSET                      0



//[175]  GEMINI_REG_BB_BW0
#define GEMINI_BB_BW0_CM_TIA_TEST_MASK                               MSK_1
#define GEMINI_BB_BW0_CM_TIA_TEST_OFFSET                             7

#define GEMINI_BB_BW0_CM_TIA_MASK                                    MSK_2
#define GEMINI_BB_BW0_CM_TIA_OFFSET                                  5

#define GEMINI_BB_BW0_CM_BQ_MASK                                     MSK_1
#define GEMINI_BB_BW0_CM_BQ_OFFSET                                   4

#define GEMINI_BB_BW0_RT_CODE_TEST_MASK                              MSK_1
#define GEMINI_BB_BW0_RT_CODE_TEST_OFFSET                            3

#define GEMINI_BB_BW0_RT_CODE_MASK                                   MSK_2
#define GEMINI_BB_BW0_RT_CODE_OFFSET                                 1

#define GEMINI_BB_BW0_RESERVED_MASK                                  MSK_1
#define GEMINI_BB_BW0_RESERVED_OFFSET                                0



//[176]  GEMINI_REG_BB_BW1
#define GEMINI_BB_BW1_CT_TIA_8K_I_MASK                               MSK_8
#define GEMINI_BB_BW1_CT_TIA_8K_I_OFFSET                             0



//[177]  GEMINI_REG_BB_BW2
#define GEMINI_BB_BW2_CT_TIA_4K_I_MASK                               MSK_8
#define GEMINI_BB_BW2_CT_TIA_4K_I_OFFSET                             0



//[178]  GEMINI_REG_BB_BW3
#define GEMINI_BB_BW3_RESERVED_MASK                                  MSK_1
#define GEMINI_BB_BW3_RESERVED_OFFSET                                7

#define GEMINI_BB_BW3_CT_BQ_I_MASK                                   MSK_7
#define GEMINI_BB_BW3_CT_BQ_I_OFFSET                                 0



//[179]  GEMINI_REG_BB_BW4
#define GEMINI_BB_BW4_CT_TIA_8K_Q_MASK                               MSK_8
#define GEMINI_BB_BW4_CT_TIA_8K_Q_OFFSET                             0



//[180]  GEMINI_REG_BB_BW5
#define GEMINI_BB_BW5_CT_TIA_4K_Q_MASK                               MSK_8
#define GEMINI_BB_BW5_CT_TIA_4K_Q_OFFSET                             0



//[181]  GEMINI_REG_BB_BW6
#define GEMINI_BB_BW6_RESERVED_MASK                                  MSK_1
#define GEMINI_BB_BW6_RESERVED_OFFSET                                7

#define GEMINI_BB_BW6_CT_BQ_Q_MASK                                   MSK_7
#define GEMINI_BB_BW6_CT_BQ_Q_OFFSET                                 0



//[182]  GEMINI_REG_BB_INSITU_RCM0
#define GEMINI_BB_INSITU_RCM0_RESERVED_MASK                          MSK_2
#define GEMINI_BB_INSITU_RCM0_RESERVED_OFFSET                        6

#define GEMINI_BB_INSITU_RCM0_INSITU_EN_I_MASK                       MSK_1
#define GEMINI_BB_INSITU_RCM0_INSITU_EN_I_OFFSET                     5

#define GEMINI_BB_INSITU_RCM0_INSITU_EN_Q_MASK                       MSK_1
#define GEMINI_BB_INSITU_RCM0_INSITU_EN_Q_OFFSET                     4

#define GEMINI_BB_INSITU_RCM0_INSITU_INIT_I_MASK                     MSK_2
#define GEMINI_BB_INSITU_RCM0_INSITU_INIT_I_OFFSET                   2

#define GEMINI_BB_INSITU_RCM0_INSITU_INIT_Q_MASK                     MSK_2
#define GEMINI_BB_INSITU_RCM0_INSITU_INIT_Q_OFFSET                   0


//[183]  GEMINI_REG_BB_INSITU_RCM3
#define GEMINI_BB_INSITU_RCM3_RESERVED_MASK                          MSK_1
#define GEMINI_BB_INSITU_RCM3_RESERVED_OFFSET                        7

#define GEMINI_BB_INSITU_RCM3_INSITU_RCM_START_MASK                  MSK_1
#define GEMINI_BB_INSITU_RCM3_INSITU_RCM_START_OFFSET                6

#define GEMINI_BB_INSITU_RCM3_INSITU_RCM_COUNT_MASK                  MSK_6
#define GEMINI_BB_INSITU_RCM3_INSITU_RCM_COUNT_OFFSET                0



//[184]  GEMINI_REG_BB_INSITU_RCM4
#define GEMINI_BB_INSITU_RCM4_INSITU_RCM_WARMUP_MASK                 MSK_2
#define GEMINI_BB_INSITU_RCM4_INSITU_RCM_WARMUP_OFFSET               6

#define GEMINI_BB_INSITU_RCM4_INSITU_RCM_OSCILLATOR_MASK             MSK_2
#define GEMINI_BB_INSITU_RCM4_INSITU_RCM_OSCILLATOR_OFFSET           4

#define GEMINI_BB_INSITU_RCM4_INSITU_RCM_GPO_MASK                    MSK_2
#define GEMINI_BB_INSITU_RCM4_INSITU_RCM_GPO_OFFSET                  2

#define GEMINI_BB_INSITU_RCM4_RCM_CAPACITOR_MASK                     MSK_2
#define GEMINI_BB_INSITU_RCM4_RCM_CAPACITOR_OFFSET                   0



//[185]  GEMINI_REG_BBF_CTL0
#define GEMINI_BBF_CTL0_BBF_EN_I_MASK                                MSK_4
#define GEMINI_BBF_CTL0_BBF_EN_I_OFFSET                              4

#define GEMINI_BBF_CTL0_BBF_EN_Q_MASK                                MSK_4
#define GEMINI_BBF_CTL0_BBF_EN_Q_OFFSET                              0



//[186]  GEMINI_REG_BBF_CTL1
#define GEMINI_BBF_CTL1_BBF_HIZ_EN_MASK                              MSK_8
#define GEMINI_BBF_CTL1_BBF_HIZ_EN_OFFSET                            0



//[187]  GEMINI_REG_BBF_CTL2
#define GEMINI_BBF_CTL2_RESERVED_MASK                                MSK_4
#define GEMINI_BBF_CTL2_RESERVED_OFFSET                              4

#define GEMINI_BBF_CTL2_RX_MOD_INV_MASK                              MSK_1
#define GEMINI_BBF_CTL2_RX_MOD_INV_OFFSET                            3

#define GEMINI_BBF_CTL2_TX_MOD_INV_MASK                              MSK_1
#define GEMINI_BBF_CTL2_TX_MOD_INV_OFFSET                            2

#define GEMINI_BBF_CTL2_TX_DAC_DIV4_EN_MASK                          MSK_1
#define GEMINI_BBF_CTL2_TX_DAC_DIV4_EN_OFFSET                        1

#define GEMINI_BBF_CTL2_BBF_BIAS_MODE_MASK                           MSK_1
#define GEMINI_BBF_CTL2_BBF_BIAS_MODE_OFFSET                         0



//[188]  GEMINI_REG_BBF_CTL3
#define GEMINI_BBF_CTL3_TIA_ICQ_MASK                                 MSK_2
#define GEMINI_BBF_CTL3_TIA_ICQ_OFFSET                               6

#define GEMINI_BBF_CTL3_TIA_IBIAS_MASK                               MSK_2
#define GEMINI_BBF_CTL3_TIA_IBIAS_OFFSET                             4

#define GEMINI_BBF_CTL3_BQ_ICQ_MASK                                  MSK_2
#define GEMINI_BBF_CTL3_BQ_ICQ_OFFSET                                2

#define GEMINI_BBF_CTL3_BQ_IBIAS_MASK                                MSK_2
#define GEMINI_BBF_CTL3_BQ_IBIAS_OFFSET                              0



//[189]  GEMINI_REG_BBF_CTL4
#define GEMINI_BBF_CTL4_PGA_ICQ_MASK                                 MSK_2
#define GEMINI_BBF_CTL4_PGA_ICQ_OFFSET                               6

#define GEMINI_BBF_CTL4_PGA_IBIAS_RX_MASK                            MSK_2
#define GEMINI_BBF_CTL4_PGA_IBIAS_RX_OFFSET                          4

#define GEMINI_BBF_CTL4_PGA_IBIAS_TX_MASK                            MSK_2
#define GEMINI_BBF_CTL4_PGA_IBIAS_TX_OFFSET                          2

#define GEMINI_BBF_CTL4_RESERVED_MASK                                MSK_2
#define GEMINI_BBF_CTL4_RESERVED_OFFSET                              0



//[190]  GEMINI_REG_BBF_CTL5
#define GEMINI_BBF_CTL5_RESERVED_MASK                                MSK_3
#define GEMINI_BBF_CTL5_RESERVED_OFFSET                              5

#define GEMINI_BBF_CTL5_TIA_OTAC_MASK                                MSK_5
#define GEMINI_BBF_CTL5_TIA_OTAC_OFFSET                              0



//[191]  GEMINI_REG_BBF_CTL6
#define GEMINI_BBF_CTL6_RESERVED_MASK                                MSK_1
#define GEMINI_BBF_CTL6_RESERVED_OFFSET                              7

#define GEMINI_BBF_CTL6_BQ_RCOMP_MASK                                MSK_2
#define GEMINI_BBF_CTL6_BQ_RCOMP_OFFSET                              5

#define GEMINI_BBF_CTL6_BQ_OTAC_MASK                                 MSK_5
#define GEMINI_BBF_CTL6_BQ_OTAC_OFFSET                               0



//[192]  GEMINI_REG_BBF_CTL7
#define GEMINI_BBF_CTL7_RX_CAL_EN_OVERRIDE_MASK                      MSK_1
#define GEMINI_BBF_CTL7_RX_CAL_EN_OVERRIDE_OFFSET                    7

#define GEMINI_BBF_CTL7_PGA_ACC_MASK                                 MSK_2
#define GEMINI_BBF_CTL7_PGA_ACC_OFFSET                               5

#define GEMINI_BBF_CTL7_PGA_OTAC_TX_MASK                             MSK_5
#define GEMINI_BBF_CTL7_PGA_OTAC_TX_OFFSET                           0



//[193]  GEMINI_REG_BBF_CTL8
#define GEMINI_BBF_CTL8_RX_CAL_EN_MASK                               MSK_1
#define GEMINI_BBF_CTL8_RX_CAL_EN_OFFSET                             7

#define GEMINI_BBF_CTL8_RX_OUT_EN_OVERRIDE_MASK                      MSK_1
#define GEMINI_BBF_CTL8_RX_OUT_EN_OVERRIDE_OFFSET                    6

#define GEMINI_BBF_CTL8_RX_OUT_EN_MASK                               MSK_1
#define GEMINI_BBF_CTL8_RX_OUT_EN_OFFSET                             5

#define GEMINI_BBF_CTL8_PGA_OTAC_RX_MASK                             MSK_5
#define GEMINI_BBF_CTL8_PGA_OTAC_RX_OFFSET                           0



//[194]  GEMINI_REG_BBF_TEST0
#define GEMINI_BBF_TEST0_TEST_EN_I_MASK                              MSK_5
#define GEMINI_BBF_TEST0_TEST_EN_I_OFFSET                            3

#define GEMINI_BBF_TEST0_BYPASS_EN_I_MASK                            MSK_3
#define GEMINI_BBF_TEST0_BYPASS_EN_I_OFFSET                          0



//[195]  GEMINI_REG_BBF_TEST1
#define GEMINI_BBF_TEST1_TEST_EN_Q_MASK                              MSK_5
#define GEMINI_BBF_TEST1_TEST_EN_Q_OFFSET                            3

#define GEMINI_BBF_TEST1_BYPASS_EN_Q_MASK                            MSK_3
#define GEMINI_BBF_TEST1_BYPASS_EN_Q_OFFSET                          0



//[196]  GEMINI_REG_BBF_TEST2
#define GEMINI_BBF_TEST2_GPO_SW_MASK                                 MSK_8
#define GEMINI_BBF_TEST2_GPO_SW_OFFSET                               0



//[197]  GEMINI_REG_BBF_SAT1
#define GEMINI_BBF_SAT1_DET_IN_CHOICE_MASK                           MSK_2
#define GEMINI_BBF_SAT1_DET_IN_CHOICE_OFFSET                         6

#define GEMINI_BBF_SAT1_SAT_AMP_GAIN_MASK                            MSK_2
#define GEMINI_BBF_SAT1_SAT_AMP_GAIN_OFFSET                          4

#define GEMINI_BBF_SAT1_SAT_THRES_GAIN_MASK                          MSK_3
#define GEMINI_BBF_SAT1_SAT_THRES_GAIN_OFFSET                        1

#define GEMINI_BBF_SAT1_MAN_SAT_MASK                                 MSK_1
#define GEMINI_BBF_SAT1_MAN_SAT_OFFSET                               0



//[198]  GEMINI_REG_BBF_SAT2
#define GEMINI_BBF_SAT2_SAT_DET_EN_MASK                              MSK_1
#define GEMINI_BBF_SAT2_SAT_DET_EN_OFFSET                            7

#define GEMINI_BBF_SAT2_SAT_GPO_MASK                                 MSK_4
#define GEMINI_BBF_SAT2_SAT_GPO_OFFSET                               3

#define GEMINI_BBF_SAT2_SAT_OUT_MUX_MASK                             MSK_1
#define GEMINI_BBF_SAT2_SAT_OUT_MUX_OFFSET                           2

#define GEMINI_BBF_SAT2_SAT_ICQ_MASK                                 MSK_2
#define GEMINI_BBF_SAT2_SAT_ICQ_OFFSET                               0



//[199]  GEMINI_REG_BBF_SAT3
#define GEMINI_BBF_SAT3_SAT_IBIAS_MASK                               MSK_2
#define GEMINI_BBF_SAT3_SAT_IBIAS_OFFSET                             5

#define GEMINI_BBF_SAT3_SAT_OTA_CONTROL_MASK                         MSK_5
#define GEMINI_BBF_SAT3_SAT_OTA_CONTROL_OFFSET                       0



//[200]  GEMINI_REG_BBF_SAT4
#define GEMINI_BBF_SAT4_RESERVED_MASK                                MSK_2
#define GEMINI_BBF_SAT4_RESERVED_OFFSET                              6

#define GEMINI_BBF_SAT4_EGY_OUT_MUX_MASK                             MSK_1
#define GEMINI_BBF_SAT4_EGY_OUT_MUX_OFFSET                           5

#define GEMINI_BBF_SAT4_EGY_GPO_MASK                                 MSK_4
#define GEMINI_BBF_SAT4_EGY_GPO_OFFSET                               1

#define GEMINI_BBF_SAT4_EGY_DET_EN_MASK                              MSK_1
#define GEMINI_BBF_SAT4_EGY_DET_EN_OFFSET                            0



//[201]  GEMINI_REG_BBF_SAT5
#define GEMINI_BBF_SAT5_EGY_THRES_MAN_MASK                           MSK_1
#define GEMINI_BBF_SAT5_EGY_THRES_MAN_OFFSET                         7

#define GEMINI_BBF_SAT5_EGY_THRES_IN_MASK                            MSK_7
#define GEMINI_BBF_SAT5_EGY_THRES_IN_OFFSET                          0



//[202]  GEMINI_REG_BBF_SAT6
#define GEMINI_BBF_SAT6_RESERVED_MASK                                MSK_4
#define GEMINI_BBF_SAT6_RESERVED_OFFSET                              4

#define GEMINI_BBF_SAT6_SAT_TH_MASK                                  MSK_4
#define GEMINI_BBF_SAT6_SAT_TH_OFFSET                                0



//[203]  GEMINI_REG_HKADC_CFG
#define GEMINI_HKADC_CFG_HKADC_INPUT_MASK                            MSK_4
#define GEMINI_HKADC_CFG_HKADC_INPUT_OFFSET                          4

#define GEMINI_HKADC_CFG_RTUNER_SOURCE_MASK                          MSK_3
#define GEMINI_HKADC_CFG_RTUNER_SOURCE_OFFSET                        1

#define GEMINI_HKADC_CFG_RTUNER_EN_MASK                              MSK_1
#define GEMINI_HKADC_CFG_RTUNER_EN_OFFSET                            0



//[204]  GEMINI_REG_RX_IM2_I_CFG0
#define GEMINI_RX_IM2_I_CFG0_TSENSOR_ON_MASK                         MSK_1
#define GEMINI_RX_IM2_I_CFG0_TSENSOR_ON_OFFSET                       7

#define GEMINI_RX_IM2_I_CFG0_IM2_VCAL_IDAC_I_MASK                    MSK_7
#define GEMINI_RX_IM2_I_CFG0_IM2_VCAL_IDAC_I_OFFSET                  0



//[205]  GEMINI_REG_RX_IM2_I_CFG1
#define GEMINI_RX_IM2_I_CFG1_RESERVED_MASK                           MSK_6
#define GEMINI_RX_IM2_I_CFG1_RESERVED_OFFSET                         2

#define GEMINI_RX_IM2_I_CFG1_IM2_VCAL_RES_I_MASK                     MSK_2
#define GEMINI_RX_IM2_I_CFG1_IM2_VCAL_RES_I_OFFSET                   0



//[206]  GEMINI_REG_RX_IM2_Q_CFG0
#define GEMINI_RX_IM2_Q_CFG0_RESERVED_MASK                           MSK_1
#define GEMINI_RX_IM2_Q_CFG0_RESERVED_OFFSET                         7

#define GEMINI_RX_IM2_Q_CFG0_IM2_VCAL_IDAC_Q_MASK                    MSK_7
#define GEMINI_RX_IM2_Q_CFG0_IM2_VCAL_IDAC_Q_OFFSET                  0



//[207]  GEMINI_REG_RX_IM2_Q_CFG1
#define GEMINI_RX_IM2_Q_CFG1_RESERVED_MASK                           MSK_6
#define GEMINI_RX_IM2_Q_CFG1_RESERVED_OFFSET                         2

#define GEMINI_RX_IM2_Q_CFG1_IM2_VCAL_RES_Q_MASK                     MSK_2
#define GEMINI_RX_IM2_Q_CFG1_IM2_VCAL_RES_Q_OFFSET                   0



//[208]  GEMINI_REG_RX_IM2_VCM0
#define GEMINI_RX_IM2_VCM0_RESERVED_MASK                             MSK_1
#define GEMINI_RX_IM2_VCM0_RESERVED_OFFSET                           7

#define GEMINI_RX_IM2_VCM0_IM2_VCM_BUF_MASK                          MSK_4
#define GEMINI_RX_IM2_VCM0_IM2_VCM_BUF_OFFSET                        3

#define GEMINI_RX_IM2_VCM0_IM2_VREF_MASK                             MSK_1
#define GEMINI_RX_IM2_VCM0_IM2_VREF_OFFSET                           2

#define GEMINI_RX_IM2_VCM0_IM2_VBUF_IAMP_VGS_MASK                    MSK_1
#define GEMINI_RX_IM2_VCM0_IM2_VBUF_IAMP_VGS_OFFSET                  1

#define GEMINI_RX_IM2_VCM0_IM2_VBUF_IAMP_VCM_MASK                    MSK_1
#define GEMINI_RX_IM2_VCM0_IM2_VBUF_IAMP_VCM_OFFSET                  0



//[209]  GEMINI_REG_RX_IM2_VCM1
#define GEMINI_RX_IM2_VCM1_RESERVED_MASK                             MSK_1
#define GEMINI_RX_IM2_VCM1_RESERVED_OFFSET                           7

#define GEMINI_RX_IM2_VCM1_IM2_VCM_MIXER_MASK                        MSK_2
#define GEMINI_RX_IM2_VCM1_IM2_VCM_MIXER_OFFSET                      5

#define GEMINI_RX_IM2_VCM1_IM2_VCM_BQ_MASK                           MSK_2
#define GEMINI_RX_IM2_VCM1_IM2_VCM_BQ_OFFSET                         3

#define GEMINI_RX_IM2_VCM1_IM2_VGF_MASK                              MSK_3
#define GEMINI_RX_IM2_VCM1_IM2_VGF_OFFSET                            0



//[210]  GEMINI_REG_RX_IM2_EN_CTL
#define GEMINI_RX_IM2_EN_CTL_IM2_TEST_2_MASK                         MSK_1
#define GEMINI_RX_IM2_EN_CTL_IM2_TEST_2_OFFSET                       7

#define GEMINI_RX_IM2_EN_CTL_IM2_TEST__1_0_MASK                      MSK_2
#define GEMINI_RX_IM2_EN_CTL_IM2_TEST__1_0_OFFSET                    5

#define GEMINI_RX_IM2_EN_CTL_IM2_GPO_1_MASK                          MSK_1
#define GEMINI_RX_IM2_EN_CTL_IM2_GPO_1_OFFSET                        4

#define GEMINI_RX_IM2_EN_CTL_IM2_GPO_0_MASK                          MSK_1
#define GEMINI_RX_IM2_EN_CTL_IM2_GPO_0_OFFSET                        3

#define GEMINI_RX_IM2_EN_CTL_IM2_VCM_EN_MASK                         MSK_1
#define GEMINI_RX_IM2_EN_CTL_IM2_VCM_EN_OFFSET                       2

#define GEMINI_RX_IM2_EN_CTL_IM2_VGF_EN_MASK                         MSK_1
#define GEMINI_RX_IM2_EN_CTL_IM2_VGF_EN_OFFSET                       1

#define GEMINI_RX_IM2_EN_CTL_RESERVED_MASK                           MSK_1
#define GEMINI_RX_IM2_EN_CTL_RESERVED_OFFSET                         0



//[211]  GEMINI_REG_RX_IM2_RSB
#define GEMINI_RX_IM2_RSB_RESERVED_MASK                              MSK_1
#define GEMINI_RX_IM2_RSB_RESERVED_OFFSET                            7

#define GEMINI_RX_IM2_RSB_IM2_VGF_SHARE_MASK                         MSK_1
#define GEMINI_RX_IM2_RSB_IM2_VGF_SHARE_OFFSET                       6

#define GEMINI_RX_IM2_RSB_IM2_VGF_SWAP_MASK                          MSK_1
#define GEMINI_RX_IM2_RSB_IM2_VGF_SWAP_OFFSET                        5

#define GEMINI_RX_IM2_RSB_IM2_RSB_RES_MASK                           MSK_2
#define GEMINI_RX_IM2_RSB_IM2_RSB_RES_OFFSET                         3

#define GEMINI_RX_IM2_RSB_IM2_RSB_IDAC_MASK                          MSK_3
#define GEMINI_RX_IM2_RSB_IM2_RSB_IDAC_OFFSET                        0



//[212]  GEMINI_REG_RX_IM2_SPARE0
#define GEMINI_RX_IM2_SPARE0_RX_IM2CAL_SPARE0_MASK                   MSK_8
#define GEMINI_RX_IM2_SPARE0_RX_IM2CAL_SPARE0_OFFSET                 0



//[213]  GEMINI_REG_RX_IM2_SPARE1
#define GEMINI_RX_IM2_SPARE1_RX_IM2CAL_SPARE1_MASK                   MSK_8
#define GEMINI_RX_IM2_SPARE1_RX_IM2CAL_SPARE1_OFFSET                 0



//[214]  GEMINI_REG_BBF_AUX1
#define GEMINI_BBF_AUX1_BBF_AUX1_MASK                                MSK_8
#define GEMINI_BBF_AUX1_BBF_AUX1_OFFSET                              0



//[215]  GEMINI_REG_BBF_AUX2
#define GEMINI_BBF_AUX2_BBF_AUX2_MASK                                MSK_8
#define GEMINI_BBF_AUX2_BBF_AUX2_OFFSET                              0



//[216]  GEMINI_REG_BB_INSITU_RCM1
#define GEMINI_BB_INSITU_RCM1_INSITU_RCM_OUT_LSB_MASK                MSK_8
#define GEMINI_BB_INSITU_RCM1_INSITU_RCM_OUT_LSB_OFFSET              0



//[217]  GEMINI_REG_BB_INSITU_RCM2
#define GEMINI_BB_INSITU_RCM2_RESERVED_MASK                          MSK_5
#define GEMINI_BB_INSITU_RCM2_RESERVED_OFFSET                        3

#define GEMINI_BB_INSITU_RCM2_INSITU_RCM_OVERFLOW_MASK               MSK_1
#define GEMINI_BB_INSITU_RCM2_INSITU_RCM_OVERFLOW_OFFSET             2

#define GEMINI_BB_INSITU_RCM2_INSITU_RCM_OUT_MSB_MASK                MSK_2
#define GEMINI_BB_INSITU_RCM2_INSITU_RCM_OUT_MSB_OFFSET              0



//[224]  GEMINI_REG_TX_AUX_DCOC0
#define GEMINI_TX_AUX_DCOC0_TX_DCOC_DAC_I_MASK                       MSK_8
#define GEMINI_TX_AUX_DCOC0_TX_DCOC_DAC_I_OFFSET                     8

#define GEMINI_TX_AUX_DCOC0_TX_DCOC_DAC_Q_MASK                       MSK_8
#define GEMINI_TX_AUX_DCOC0_TX_DCOC_DAC_Q_OFFSET                     0



//[225]  GEMINI_REG_TX_AUX_DCOC1
#define GEMINI_TX_AUX_DCOC1_TX_DCOC_RANGE_I_MASK                     MSK_2
#define GEMINI_TX_AUX_DCOC1_TX_DCOC_RANGE_I_OFFSET                   14

#define GEMINI_TX_AUX_DCOC1_TX_DCOC_RANGE_Q_MASK                     MSK_2
#define GEMINI_TX_AUX_DCOC1_TX_DCOC_RANGE_Q_OFFSET                   12

#define GEMINI_TX_AUX_DCOC1_TX_ATEST_EN_MASK                         MSK_1
#define GEMINI_TX_AUX_DCOC1_TX_ATEST_EN_OFFSET                       11

#define GEMINI_TX_AUX_DCOC1_TX_DAC_EN_MASK                           MSK_1
#define GEMINI_TX_AUX_DCOC1_TX_DAC_EN_OFFSET                         10

#define GEMINI_TX_AUX_DCOC1_VTOI_BW_MASK                             MSK_4
#define GEMINI_TX_AUX_DCOC1_VTOI_BW_OFFSET                           6

#define GEMINI_TX_AUX_DCOC1_G_LPBK_M3_MASK                           MSK_1
#define GEMINI_TX_AUX_DCOC1_G_LPBK_M3_OFFSET                         5

#define GEMINI_TX_AUX_DCOC1_RTRIM_LODIV_MASK                         MSK_1
#define GEMINI_TX_AUX_DCOC1_RTRIM_LODIV_OFFSET                       4

#define GEMINI_TX_AUX_DCOC1_ITRIM_LODIV_MASK                         MSK_4
#define GEMINI_TX_AUX_DCOC1_ITRIM_LODIV_OFFSET                       0



//[226]  GEMINI_REG_PDET_CTL
#define GEMINI_PDET_CTL_G_PDET_RF_MASK                               MSK_3
#define GEMINI_PDET_CTL_G_PDET_RF_OFFSET                             13

#define GEMINI_PDET_CTL_VGA_GAIN_MASK                                MSK_3
#define GEMINI_PDET_CTL_VGA_GAIN_OFFSET                              10

#define GEMINI_PDET_CTL_VGA_DCO_MASK                                 MSK_3
#define GEMINI_PDET_CTL_VGA_DCO_OFFSET                               7

#define GEMINI_PDET_CTL_MIX_MULTI_MASK                               MSK_3
#define GEMINI_PDET_CTL_MIX_MULTI_OFFSET                             4

#define GEMINI_PDET_CTL_TX_GPO_SEL_MASK                              MSK_4
#define GEMINI_PDET_CTL_TX_GPO_SEL_OFFSET                            0



//[227]  GEMINI_REG_PDET_OVRD
#define GEMINI_PDET_OVRD_OVRD_DCO_MASK                               MSK_1
#define GEMINI_PDET_OVRD_OVRD_DCO_OFFSET                             15

#define GEMINI_PDET_OVRD_G_PDET_FBR_MASK                             MSK_3
#define GEMINI_PDET_OVRD_G_PDET_FBR_OFFSET                           12

#define GEMINI_PDET_OVRD_G_PDET_INPR_MASK                            MSK_4
#define GEMINI_PDET_OVRD_G_PDET_INPR_OFFSET                          8

#define GEMINI_PDET_OVRD_DCO_PDET_MASK                               MSK_6
#define GEMINI_PDET_OVRD_DCO_PDET_OFFSET                             2

#define GEMINI_PDET_OVRD_OVRD_MIX_MASK                               MSK_1
#define GEMINI_PDET_OVRD_OVRD_MIX_OFFSET                             1

#define GEMINI_PDET_OVRD_RESERVED_MASK                               MSK_1
#define GEMINI_PDET_OVRD_RESERVED_OFFSET                             0



//[228]  GEMINI_REG_MIX_OVRD
#define GEMINI_MIX_OVRD_MIX_I_EN_MASK                                MSK_8
#define GEMINI_MIX_OVRD_MIX_I_EN_OFFSET                              8

#define GEMINI_MIX_OVRD_MIX_Q_EN_MASK                                MSK_8
#define GEMINI_MIX_OVRD_MIX_Q_EN_OFFSET                              0



//[229]  GEMINI_REG_TX_RCTUNE_CTL
#define GEMINI_TX_RCTUNE_CTL_OVRD_RTUNE_MASK                         MSK_1
#define GEMINI_TX_RCTUNE_CTL_OVRD_RTUNE_OFFSET                       12

#define GEMINI_TX_RCTUNE_CTL_RTUNE_MASK                              MSK_4
#define GEMINI_TX_RCTUNE_CTL_RTUNE_OFFSET                            8

#define GEMINI_TX_RCTUNE_CTL_CTUNE_MASK                              MSK_8
#define GEMINI_TX_RCTUNE_CTL_CTUNE_OFFSET                            0



//[230]  GEMINI_REG_OVRD_CTL
#define GEMINI_OVRD_CTL_OVRD_TX_MASK                                 MSK_1
#define GEMINI_OVRD_CTL_OVRD_TX_OFFSET                               15

#define GEMINI_OVRD_CTL_TX_BIAS_ENABLE_MASK                          MSK_1
#define GEMINI_OVRD_CTL_TX_BIAS_ENABLE_OFFSET                        14

#define GEMINI_OVRD_CTL_VTOI_ENABLE_MASK                             MSK_1
#define GEMINI_OVRD_CTL_VTOI_ENABLE_OFFSET                           13

#define GEMINI_OVRD_CTL_TX_LOBUF_ENABLE_MASK                         MSK_1
#define GEMINI_OVRD_CTL_TX_LOBUF_ENABLE_OFFSET                       12

#define GEMINI_OVRD_CTL_TX_MIX_ENABLE_MASK                           MSK_1
#define GEMINI_OVRD_CTL_TX_MIX_ENABLE_OFFSET                         11

#define GEMINI_OVRD_CTL_PDET_RF_ENABLE_MASK                          MSK_1
#define GEMINI_OVRD_CTL_PDET_RF_ENABLE_OFFSET                        10

#define GEMINI_OVRD_CTL_PDET_AMP_ENABLE_MASK                         MSK_1
#define GEMINI_OVRD_CTL_PDET_AMP_ENABLE_OFFSET                       9

#define GEMINI_OVRD_CTL_LPBK0_EN_MASK                                MSK_1
#define GEMINI_OVRD_CTL_LPBK0_EN_OFFSET                              8

#define GEMINI_OVRD_CTL_LPBK1_EN_MASK                                MSK_1
#define GEMINI_OVRD_CTL_LPBK1_EN_OFFSET                              7

#define GEMINI_OVRD_CTL_RESERVED_MASK                                MSK_7
#define GEMINI_OVRD_CTL_RESERVED_OFFSET                              0



//[231]  GEMINI_REG_TX_SPARE1
#define GEMINI_TX_SPARE1_SPARE_MASK                                  MSK_8
#define GEMINI_TX_SPARE1_SPARE_OFFSET                                0



//[232]  GEMINI_REG_TX_SPARE2
#define GEMINI_TX_SPARE2_SPARE_MASK                                  MSK_8
#define GEMINI_TX_SPARE2_SPARE_OFFSET                                0



//[233]  GEMINI_REG_TX_SPARE3
#define GEMINI_TX_SPARE3_SPARE_MASK                                  MSK_8
#define GEMINI_TX_SPARE3_SPARE_OFFSET                                0



//[234]  GEMINI_REG_TX_SPARE4
#define GEMINI_TX_SPARE4_SPARE_MASK                                  MSK_8
#define GEMINI_TX_SPARE4_SPARE_OFFSET                                0



//[235]  GEMINI_REG_TX_SPARE5
#define GEMINI_TX_SPARE5_SPARE_MASK                                  MSK_8
#define GEMINI_TX_SPARE5_SPARE_OFFSET                                0



//[240]  GEMINI_REG_TRSW_CTL
#define GEMINI_TRSW_CTL_SW_RX_CAP_TUNE_3_0_MASK                      MSK_4
#define GEMINI_TRSW_CTL_SW_RX_CAP_TUNE_3_0_OFFSET                    12

#define GEMINI_TRSW_CTL_SW_BIAS_TUNE_3_0_MASK                        MSK_4
#define GEMINI_TRSW_CTL_SW_BIAS_TUNE_3_0_OFFSET                      8

#define GEMINI_TRSW_CTL_SW_DEEP_SLEEP_B_MASK                         MSK_1
#define GEMINI_TRSW_CTL_SW_DEEP_SLEEP_B_OFFSET                       7

#define GEMINI_TRSW_CTL_OVRD_SW_MASK                                 MSK_1
#define GEMINI_TRSW_CTL_OVRD_SW_OFFSET                               6

#define GEMINI_TRSW_CTL_SW_TX_ENABLE_MASK                            MSK_1
#define GEMINI_TRSW_CTL_SW_TX_ENABLE_OFFSET                          5

#define GEMINI_TRSW_CTL_SW_RX_ENABLE_MASK                            MSK_1
#define GEMINI_TRSW_CTL_SW_RX_ENABLE_OFFSET                          4

#define GEMINI_TRSW_CTL_SW_BT_ENABLE_MASK                            MSK_1
#define GEMINI_TRSW_CTL_SW_BT_ENABLE_OFFSET                          3

#define GEMINI_TRSW_CTL_SW_GPO_SEL_1_0_MASK                          MSK_2
#define GEMINI_TRSW_CTL_SW_GPO_SEL_1_0_OFFSET                        1

#define GEMINI_TRSW_CTL_SW_TEST_MASK                                 MSK_1
#define GEMINI_TRSW_CTL_SW_TEST_OFFSET                               0



//[241]  GEMINI_REG_PA_STG1_2_BIAS
#define GEMINI_PA_STG1_2_BIAS_STG1_TYPE_1_0_MASK                     MSK_2
#define GEMINI_PA_STG1_2_BIAS_STG1_TYPE_1_0_OFFSET                   14

#define GEMINI_PA_STG1_2_BIAS_STG1_IBIAS_3_0_MASK                    MSK_4
#define GEMINI_PA_STG1_2_BIAS_STG1_IBIAS_3_0_OFFSET                  10

#define GEMINI_PA_STG1_2_BIAS_STG2_TYPE_1_0_MASK                     MSK_2
#define GEMINI_PA_STG1_2_BIAS_STG2_TYPE_1_0_OFFSET                   8

#define GEMINI_PA_STG1_2_BIAS_STG2_IBIAS_3_0_MASK                    MSK_4
#define GEMINI_PA_STG1_2_BIAS_STG2_IBIAS_3_0_OFFSET                  4

#define GEMINI_PA_STG1_2_BIAS_STG12_VCM_2_0_MASK                     MSK_3
#define GEMINI_PA_STG1_2_BIAS_STG12_VCM_2_0_OFFSET                   1

#define GEMINI_PA_STG1_2_BIAS_RESERVED_MASK                          MSK_1
#define GEMINI_PA_STG1_2_BIAS_RESERVED_OFFSET                        0



//[242]  GEMINI_REG_PA_STG3_BIAS
#define GEMINI_PA_STG3_BIAS_STG3_TYPE_1_0_MASK                       MSK_2
#define GEMINI_PA_STG3_BIAS_STG3_TYPE_1_0_OFFSET                     14

#define GEMINI_PA_STG3_BIAS_STG3_IBIAS_3_0_MASK                      MSK_4
#define GEMINI_PA_STG3_BIAS_STG3_IBIAS_3_0_OFFSET                    10

#define GEMINI_PA_STG3_BIAS_STG3_CAS_VBIAS_3_0_MASK                  MSK_4
#define GEMINI_PA_STG3_BIAS_STG3_CAS_VBIAS_3_0_OFFSET                6

#define GEMINI_PA_STG3_BIAS_SEL_PU_BU_VCM_MASK                       MSK_1
#define GEMINI_PA_STG3_BIAS_SEL_PU_BU_VCM_OFFSET                     5

#define GEMINI_PA_STG3_BIAS_SEL_PU_BU_VBIAS_MASK                     MSK_1
#define GEMINI_PA_STG3_BIAS_SEL_PU_BU_VBIAS_OFFSET                   4

#define GEMINI_PA_STG3_BIAS_PA_TEMP_SEN_EN_1_0_MASK                  MSK_2
#define GEMINI_PA_STG3_BIAS_PA_TEMP_SEN_EN_1_0_OFFSET                2

#define GEMINI_PA_STG3_BIAS_RESERVED_MASK                            MSK_2
#define GEMINI_PA_STG3_BIAS_RESERVED_OFFSET                          0



//[243]  GEMINI_REG_DA_GAIN_CTL
#define GEMINI_DA_GAIN_CTL_G_FNL_3_0_MASK                            MSK_4
#define GEMINI_DA_GAIN_CTL_G_FNL_3_0_OFFSET                          12

#define GEMINI_DA_GAIN_CTL_G_PRE_3_0_MASK                            MSK_4
#define GEMINI_DA_GAIN_CTL_G_PRE_3_0_OFFSET                          8

#define GEMINI_DA_GAIN_CTL_VCM_FNL_2_0_MASK                          MSK_3
#define GEMINI_DA_GAIN_CTL_VCM_FNL_2_0_OFFSET                        5

#define GEMINI_DA_GAIN_CTL_VCM_PRE_2_0_MASK                          MSK_3
#define GEMINI_DA_GAIN_CTL_VCM_PRE_2_0_OFFSET                        2

#define GEMINI_DA_GAIN_CTL_RESERVED_MASK                             MSK_2
#define GEMINI_DA_GAIN_CTL_RESERVED_OFFSET                           0



//[244]  GEMINI_REG_DA_BIAS
#define GEMINI_DA_BIAS_IBIAS_FNL_2_0_MASK                            MSK_3
#define GEMINI_DA_BIAS_IBIAS_FNL_2_0_OFFSET                          13

#define GEMINI_DA_BIAS_IBIAS_PRE_2_0_MASK                            MSK_3
#define GEMINI_DA_BIAS_IBIAS_PRE_2_0_OFFSET                          10

#define GEMINI_DA_BIAS_DA_GPIO_4_0_MASK                              MSK_5
#define GEMINI_DA_BIAS_DA_GPIO_4_0_OFFSET                            5

#define GEMINI_DA_BIAS_RESERVED_MASK                                 MSK_5
#define GEMINI_DA_BIAS_RESERVED_OFFSET                               0



//[245]  GEMINI_REG_DA_BALUN_PA_CTL
#define GEMINI_DA_BALUN_PA_CTL_PA_GC1_2_0_MASK                       MSK_3
#define GEMINI_DA_BALUN_PA_CTL_PA_GC1_2_0_OFFSET                     13

#define GEMINI_DA_BALUN_PA_CTL_PA_GC2_2_0_MASK                       MSK_3
#define GEMINI_DA_BALUN_PA_CTL_PA_GC2_2_0_OFFSET                     10

#define GEMINI_DA_BALUN_PA_CTL_INPUT_BALUN_3_0_MASK                  MSK_4
#define GEMINI_DA_BALUN_PA_CTL_INPUT_BALUN_3_0_OFFSET                6

#define GEMINI_DA_BALUN_PA_CTL_OUTPUT_BALUN_3_0_MASK                 MSK_4
#define GEMINI_DA_BALUN_PA_CTL_OUTPUT_BALUN_3_0_OFFSET               2

#define GEMINI_DA_BALUN_PA_CTL_RESERVED_MASK                         MSK_2
#define GEMINI_DA_BALUN_PA_CTL_RESERVED_OFFSET                       0



//[246]  GEMINI_REG_TEST_PATH_CTL
#define GEMINI_TEST_PATH_CTL_PA_GPIO_3_0_MASK                        MSK_4
#define GEMINI_TEST_PATH_CTL_PA_GPIO_3_0_OFFSET                      12

#define GEMINI_TEST_PATH_CTL_PA_GPO_MUX_SEL_MASK                     MSK_3
#define GEMINI_TEST_PATH_CTL_PA_GPO_MUX_SEL_OFFSET                   9

#define GEMINI_TEST_PATH_CTL_PA_ADC_MUX_SEL_MASK                     MSK_3
#define GEMINI_TEST_PATH_CTL_PA_ADC_MUX_SEL_OFFSET                   6

#define GEMINI_TEST_PATH_CTL_PA_TEST_2_0_MASK                        MSK_3
#define GEMINI_TEST_PATH_CTL_PA_TEST_2_0_OFFSET                      3

#define GEMINI_TEST_PATH_CTL_PA_DTOMODE_2_0_MASK                     MSK_3
#define GEMINI_TEST_PATH_CTL_PA_DTOMODE_2_0_OFFSET                   0



//[247]  GEMINI_REG_PA_EN
#define GEMINI_PA_EN_OVRD_PA_MASK                                    MSK_1
#define GEMINI_PA_EN_OVRD_PA_OFFSET                                  15

#define GEMINI_PA_EN_STG1_ENABLE_MASK                                MSK_1
#define GEMINI_PA_EN_STG1_ENABLE_OFFSET                              14

#define GEMINI_PA_EN_STG2_ENABLE_MASK                                MSK_1
#define GEMINI_PA_EN_STG2_ENABLE_OFFSET                              13

#define GEMINI_PA_EN_STG3_ENABLE_MASK                                MSK_1
#define GEMINI_PA_EN_STG3_ENABLE_OFFSET                              12

#define GEMINI_PA_EN_AR_DA_ENABLE_MASK                               MSK_1
#define GEMINI_PA_EN_AR_DA_ENABLE_OFFSET                             11

#define GEMINI_PA_EN_DA_PDO_EN_MASK                                  MSK_1
#define GEMINI_PA_EN_DA_PDO_EN_OFFSET                                10

#define GEMINI_PA_EN_PA_BIAS_ENABLE_MASK                             MSK_1
#define GEMINI_PA_EN_PA_BIAS_ENABLE_OFFSET                           9

#define GEMINI_PA_EN_VSWR_EN_MASK                                    MSK_1
#define GEMINI_PA_EN_VSWR_EN_OFFSET                                  8

#define GEMINI_PA_EN_RESERVED_MASK                                   MSK_8
#define GEMINI_PA_EN_RESERVED_OFFSET                                 0



//[248]  GEMINI_REG_HDET_CTL
#define GEMINI_HDET_CTL_HDET_CSTART_MASK                             MSK_1
#define GEMINI_HDET_CTL_HDET_CSTART_OFFSET                           15

#define GEMINI_HDET_CTL_HDET_RESET_MASK                              MSK_1
#define GEMINI_HDET_CTL_HDET_RESET_OFFSET                            14

#define GEMINI_HDET_CTL_HDET_PATH_SEL_MASK                           MSK_1
#define GEMINI_HDET_CTL_HDET_PATH_SEL_OFFSET                         13

#define GEMINI_HDET_CTL_HDET_OUT_SEL_MASK                            MSK_1
#define GEMINI_HDET_CTL_HDET_OUT_SEL_OFFSET                          12

#define GEMINI_HDET_CTL_EXT_ATTEN_3_0_MASK                           MSK_4
#define GEMINI_HDET_CTL_EXT_ATTEN_3_0_OFFSET                         8

#define GEMINI_HDET_CTL_INT_ATTEN_3_0_MASK                           MSK_4
#define GEMINI_HDET_CTL_INT_ATTEN_3_0_OFFSET                         4

#define GEMINI_HDET_CTL_HDET_TIA_GAIN_3_0_MASK                       MSK_4
#define GEMINI_HDET_CTL_HDET_TIA_GAIN_3_0_OFFSET                     0



//[249]  GEMINI_REG_HDET_BIAS
#define GEMINI_HDET_BIAS_HDET_FILTER_BW_2_0_MASK                     MSK_3
#define GEMINI_HDET_BIAS_HDET_FILTER_BW_2_0_OFFSET                   13

#define GEMINI_HDET_BIAS_HDET_SQUARE_BIAS_2_0_MASK                   MSK_3
#define GEMINI_HDET_BIAS_HDET_SQUARE_BIAS_2_0_OFFSET                 10

#define GEMINI_HDET_BIAS_HDET_ICAL_STEP_1_0_MASK                     MSK_2
#define GEMINI_HDET_BIAS_HDET_ICAL_STEP_1_0_OFFSET                   8

#define GEMINI_HDET_BIAS_HDET_TIA_BIAS_2_0_MASK                      MSK_3
#define GEMINI_HDET_BIAS_HDET_TIA_BIAS_2_0_OFFSET                    5

#define GEMINI_HDET_BIAS_HDET_OFFSET_VOL_MASK                        MSK_1
#define GEMINI_HDET_BIAS_HDET_OFFSET_VOL_OFFSET                      4

#define GEMINI_HDET_BIAS_HDET_REF_VOL_3_0_MASK                       MSK_4
#define GEMINI_HDET_BIAS_HDET_REF_VOL_3_0_OFFSET                     0



//[250]  GEMINI_REG_HDET_TEST
#define GEMINI_HDET_TEST_HDET_BYPASS_MASK                            MSK_1
#define GEMINI_HDET_TEST_HDET_BYPASS_OFFSET                          15

#define GEMINI_HDET_TEST_HDET_TEMP_SEN_EN_MASK                       MSK_1
#define GEMINI_HDET_TEST_HDET_TEMP_SEN_EN_OFFSET                     14

#define GEMINI_HDET_TEST_HDET_CLK_DIV_1_0_MASK                       MSK_2
#define GEMINI_HDET_TEST_HDET_CLK_DIV_1_0_OFFSET                     12

#define GEMINI_HDET_TEST_HDET_GPO_SEL_1_0_MASK                       MSK_2
#define GEMINI_HDET_TEST_HDET_GPO_SEL_1_0_OFFSET                     10

#define GEMINI_HDET_TEST_HDET_GPO_EN_MASK                            MSK_1
#define GEMINI_HDET_TEST_HDET_GPO_EN_OFFSET                          9

#define GEMINI_HDET_TEST_HDET_DTESTEN_MASK                           MSK_1
#define GEMINI_HDET_TEST_HDET_DTESTEN_OFFSET                         8

#define GEMINI_HDET_TEST_RESERVED_MASK                               MSK_8
#define GEMINI_HDET_TEST_RESERVED_OFFSET                             0



//[251]  GEMINI_REG_HDET_DCOC
#define GEMINI_HDET_DCOC_OVRD_HDET_MASK                              MSK_1
#define GEMINI_HDET_DCOC_OVRD_HDET_OFFSET                            15

#define GEMINI_HDET_DCOC_HDET_ENABLE_MASK                            MSK_1
#define GEMINI_HDET_DCOC_HDET_ENABLE_OFFSET                          14

#define GEMINI_HDET_DCOC_IB_SCAL_EN_MASK                             MSK_1
#define GEMINI_HDET_DCOC_IB_SCAL_EN_OFFSET                           13

#define GEMINI_HDET_DCOC_IB_RCAL_EN_MASK                             MSK_1
#define GEMINI_HDET_DCOC_IB_RCAL_EN_OFFSET                           12

#define GEMINI_HDET_DCOC_DCOC_CODE_5_0_MASK                          MSK_6
#define GEMINI_HDET_DCOC_DCOC_CODE_5_0_OFFSET                        6

#define GEMINI_HDET_DCOC_RESERVED_MASK                               MSK_6
#define GEMINI_HDET_DCOC_RESERVED_OFFSET                             0



//[252]  GEMINI_REG_CAL_DATA
#define GEMINI_CAL_DATA_RESERVED_MASK                                MSK_5
#define GEMINI_CAL_DATA_RESERVED_OFFSET                              11

#define GEMINI_CAL_DATA_VSWR_SOFT_RECOVERY_MASK                      MSK_1
#define GEMINI_CAL_DATA_VSWR_SOFT_RECOVERY_OFFSET                    10

#define GEMINI_CAL_DATA_VSWR_SOFT_OVERLOAD_MASK                      MSK_1
#define GEMINI_CAL_DATA_VSWR_SOFT_OVERLOAD_OFFSET                    9

#define GEMINI_CAL_DATA_VSWR_HARD_OVERLOAD_MASK                      MSK_1
#define GEMINI_CAL_DATA_VSWR_HARD_OVERLOAD_OFFSET                    8

#define GEMINI_CAL_DATA_HDET_IB_SCAL_EN_MASK                         MSK_1
#define GEMINI_CAL_DATA_HDET_IB_SCAL_EN_OFFSET                       7

#define GEMINI_CAL_DATA_HDET_IB_RCAL_EN_MASK                         MSK_1
#define GEMINI_CAL_DATA_HDET_IB_RCAL_EN_OFFSET                       6

#define GEMINI_CAL_DATA_HDET_CAL_CODE_5_0_MASK                       MSK_6
#define GEMINI_CAL_DATA_HDET_CAL_CODE_5_0_OFFSET                     0



//[253]  GEMINI_REG_VSWR_CTL0
#define GEMINI_VSWR_CTL0_VSWR_R_DAC1_3_0_MASK                        MSK_4
#define GEMINI_VSWR_CTL0_VSWR_R_DAC1_3_0_OFFSET                      12

#define GEMINI_VSWR_CTL0_VSWR_R_DAC2_3_0_MASK                        MSK_4
#define GEMINI_VSWR_CTL0_VSWR_R_DAC2_3_0_OFFSET                      8

#define GEMINI_VSWR_CTL0_VSWR_R_DAC3_3_0_MASK                        MSK_4
#define GEMINI_VSWR_CTL0_VSWR_R_DAC3_3_0_OFFSET                      4

#define GEMINI_VSWR_CTL0_VSWR_CPC_3_0_MASK                           MSK_4
#define GEMINI_VSWR_CTL0_VSWR_CPC_3_0_OFFSET                         0



//[254]  GEMINI_REG_VSWR_CTL1
#define GEMINI_VSWR_CTL1_VSWR_I_DAC1_3_0_MASK                        MSK_4
#define GEMINI_VSWR_CTL1_VSWR_I_DAC1_3_0_OFFSET                      12

#define GEMINI_VSWR_CTL1_VSWR_I_DAC2_3_0_MASK                        MSK_4
#define GEMINI_VSWR_CTL1_VSWR_I_DAC2_3_0_OFFSET                      8

#define GEMINI_VSWR_CTL1_VSWR_I_DAC3_3_0_MASK                        MSK_4
#define GEMINI_VSWR_CTL1_VSWR_I_DAC3_3_0_OFFSET                      4

#define GEMINI_VSWR_CTL1_VSWR_TMR_2_0_MASK                           MSK_3
#define GEMINI_VSWR_CTL1_VSWR_TMR_2_0_OFFSET                         1

#define GEMINI_VSWR_CTL1_VSWR_STATUS_RESET_MASK                      MSK_1
#define GEMINI_VSWR_CTL1_VSWR_STATUS_RESET_OFFSET                    0





#endif //RFGEMINI_H

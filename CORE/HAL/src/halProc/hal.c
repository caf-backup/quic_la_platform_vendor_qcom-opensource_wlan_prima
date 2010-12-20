/*
 * Airgo Networks, Inc proprietary. All rights reserved.
 * hal.c:    HAL thread startup file.
 * Author:   V. K. Kandarpa
 * Date:     01/29/2002
 * History:-
 * Date      Modified by            Modification Information
 * --------------------------------------------------------------------------
 *
 */

/* Standard include files */

/* Application Specific include files */
#include "halTypes.h"
#include "halHddApis.h"

#include "sirCommon.h"
#include "aniGlobal.h"
#include "limApi.h"
#include "utilsApi.h"
#include "halCommonApi.h"
#include "halInternal.h"
#include "halDebug.h"
#include "halPhyApi.h"
#include "halMnt.h"
#include "halMTU.h"
#include "halRxp.h"
#include "halTpe.h"
#include "sirApi.h"
#include "cfgApi.h"
#include "halTimer.h"
#include "halUtils.h"
#include "halMacBA.h"
#include "macInitApi.h"
#include "halAdaptThrsh.h"
#include "halRFKill.h"
#include "halPwrSave.h"
#include "halBtc.h"
#include "vos_memory.h"
#include "vos_types.h"
#include "halFw.h"
#include "rfApi.h"

#ifdef ANI_PRODUCT_TYPE_AP
#include "halRadar.h"
#endif

#ifdef ANI_DVT_DEBUG
#include "dvtModuleApi.h"
#endif

#include "pttModuleApi.h"
#include "pttMsgApi.h"
#include "halPhyUtil.h"

#ifdef ANI_OS_TYPE_RTAI_LINUX
#include "sysRtaiStartup.h"
#endif

/** Temp Measurment timer interval value 30 sec.*/
#define HAL_TEMPMEAS_TIMER_VAL_SEC          30

static void halPhy_setNwDensityAndProximity(tpAniSirGlobal pMac);
static void halSetChainPowerState(tpAniSirGlobal pMac);
extern eHalStatus halPrepareForBmpsEntry(tpAniSirGlobal pMac);
extern eHalStatus halPrepareForBmpsExit(tpAniSirGlobal pMac);

static 
eHalStatus halHandleMcastBcastFilterSetting(tpAniSirGlobal pMac, tANI_U32 cfgId);

/* Constant Macros */
/* Redefine OFF -> __OFF, ON-> __ON to avoid redefinition on AMSS */
#define  MAX_VALID_CHAIN_STATE  8
#define  __OFF                    WNI_CFG_POWER_STATE_PER_CHAIN_OFF
#define  __ON                     WNI_CFG_POWER_STATE_PER_CHAIN_ON
#define  TX                     WNI_CFG_POWER_STATE_PER_CHAIN_TX
#define  RX                     WNI_CFG_POWER_STATE_PER_CHAIN_RX
#define  MAX_ALLOWED_BD_FOR_IDLE         5

/* Function Macros */
#define  GET_CHAIN_0(chain)  ((chain >> WNI_CFG_POWER_STATE_PER_CHAIN_CHAIN_0_OFFSET) & WNI_CFG_POWER_STATE_PER_CHAIN_MASK)
#define  GET_CHAIN_1(chain)  ((chain >> WNI_CFG_POWER_STATE_PER_CHAIN_CHAIN_1_OFFSET) & WNI_CFG_POWER_STATE_PER_CHAIN_MASK)
#define  GET_CHAIN_2(chain)  ((chain >> WNI_CFG_POWER_STATE_PER_CHAIN_CHAIN_2_OFFSET) & WNI_CFG_POWER_STATE_PER_CHAIN_MASK)

#define  SET_CHAIN(c0, c1, c2)  \
  ( (c0 << WNI_CFG_POWER_STATE_PER_CHAIN_CHAIN_0_OFFSET) |  \
    (c1 << WNI_CFG_POWER_STATE_PER_CHAIN_CHAIN_1_OFFSET) |  \
    (c2 << WNI_CFG_POWER_STATE_PER_CHAIN_CHAIN_2_OFFSET)    \
  )

typedef struct eChainState{
    tANI_U32         encoding;
    ePhyChainSelect  halPhyDef;
} tChainState;

static tChainState chainPwrStateTable[MAX_VALID_CHAIN_STATE] = {
 { SET_CHAIN(__OFF, __OFF, __OFF),  PHY_CHAIN_SEL_NO_RX_TX    },
 { SET_CHAIN(__OFF, RX,  __OFF),  PHY_CHAIN_SEL_R1_ON     },
 { SET_CHAIN(TX,  __OFF, __OFF),  PHY_CHAIN_SEL_T0_ON     },
 { SET_CHAIN(RX,  __OFF, __OFF),  PHY_CHAIN_SEL_R0_ON     },
 { SET_CHAIN(RX,  RX,  __OFF),  PHY_CHAIN_SEL_R0R1_ON   },
 { SET_CHAIN(__ON,  __OFF, __OFF),  PHY_CHAIN_SEL_R0_T0_ON    },
 { SET_CHAIN(__ON,  RX,  __OFF),  PHY_CHAIN_SEL_R0R1_T0_ON  },
 { SET_CHAIN(__ON,  __ON,  RX),   PHY_CHAIN_SEL_R0R1_T0_ON  }
};


typedef struct {
    tANI_U32 reg;
    tANI_U32 val;
}tRegList;

#ifdef LIBRA_FPGA

#if 0
tRegList aLibraRFSetup[] =
{
    {0xe033008, 0x60},          //#rxclkctrl.apb_block_dyn_clkg_disable(xbar=1,gpio=1)
    {0xe031418, 0x1},           //#mpi.mpi_enable(value=1)
    {0xe030400, 0x3748},        //#btcf.btcf_config(pkt_det_dshift=1, pkt_det_ratio=0)
    {0xe033008, 0x78},          //#rxclkctrl.apb_block_dyn_clkg_disable(cal=1, rxfir=1)
    {0xe03300c, 0x77},          //#rxclkctrl.apb_block_dyn_clkg_disable_agc(always_on=1, det11a=1, det11b=1, det11n40=1, pwr=1, rdet=1)
    {0x0e02e004, 0x0},          //#agc.agc_reset(reset=0)
    {0x0e02e004, 0x1},          //#agc.agc_reset(reset=1)
    {0xe02e820, 0x1},           //#cal.clr_dcoffset(1)
    {0xe033008, 0xe0},          //#rxclkctrl.apb_block_dyn_clkg_disable(cal=0, rxfir=0,pmicca=1)
    {0xe03300c, 0x1},           //#rxclkctrl.apb_block_dyn_clkg_disable_agc(always_on=1, det11a=0, det11b=0, det11n40=0, pwr=0, rdet=0)
    {0xe020e140, 0x6e},         //#agc.th_d0_a(110)
    {0xe02e150, 0x5a},          //#agc.th_d0_11n(90)
    {0xe02e180, 0x14},          //#agc.th_maxcorab(20)
    {0xe02e170, 0x78},          //#agc.th_maxcorn(120)
    {0xe02e148, 0x8c},          //#agc.th_d0_b(140)
    {0xe02e14c, 0x3c},          //#agc.th_d0_b_tf_est(60)
    {0xe02e10c, 0x32},          //#agc.th_signal_low(50)
    {0x0e02e108, 0xb4},         //#agc.th_signal_high(180)
    {0xe02e158, 0x28},          //#agc.th_d0_slr(40)
    {0xe02e15c, 0x1e},          //#agc.th_d0_slrtfest(30)
    {0xe02e160, 0x32},          //#agc.th_d0_slrwaitb(50)
    {0xe02e068, 0xa0a},         //#agc.th_edet
    {0x0e02e004, 0x0},          //#agc_reset(0)
    {0x0e02e060, 0x32},         //#agc.th_cd(50)
    {0x0e02e158, 0x96},         //#agc.th_d0_slr(0x96)
    {0x0e028bfc, 0x0},          //# CRC Bug Fix register
    {0x0e028a50, 0x01039ddb},   //# RXP Config2  register
    {0x0e02e008, 0x0},          //#agc.dis_mode(0x0)
    {0x0e02813c, 0x35d}         //#bmu.enhanced_tracing_control1
};
#endif
#if 1  //FIXME : With this config we know that LIBRA works TX/RX with Virgo in IBSS FPGA setup. Not sure what should be the production values???
tRegList aLibraRFSetup[] =
{
    { 0xe02e004,  0x1}, { 0xe033008,  0x20},
    { 0xe031414,  0x26c}, { 0xe031418,  0x1},
    { 0xe033400,  0x62}, { 0xe032808,  0x10},
    { 0xe032808,  0x0}, { 0xe030c08,  0x87e},
    { 0xe030d18,  0x640}, { 0xe030058,  0x0},
    { 0xe02e03c,  0x2}, { 0xe02e040,  0x12},
    { 0xe02e048,  0x12}, { 0xe02e044,  0x2},
    { 0xe02e038,  0x96c}, { 0xe033008,  0x38},
    { 0xe03300c,  0x63}, { 0xe02e004,  0x0},
    { 0xe02e004,  0x1}, { 0xe02e824,  0x1},
    { 0xe033008,  0xa0}, { 0xe03300c,  0x1},
    { 0xe02e190,  0x78}, { 0xe02e198,  0x78},
    { 0xe02e19c,  0x50}, { 0xe02e160,  0x32},
    { 0xe02e15c,  0xb4}, { 0xe02e1a0,  0x28},
    { 0xe02e1a4,  0x1e}, { 0xe02e1a8,  0x32},
    { 0xe02e1b8,  0x6e}, { 0xe02e1b4,  0x32},
    { 0xe02e008,  0x0}, { 0xe02e2b8,  0x1},
    { 0xe02e2c0,  0x0}, { 0xe02e2c4,  0x0},
    { 0xe02e2c8,  0x0}, { 0xe02e2cc,  0x0},
    { 0xe02e2d0,  0x0}, { 0xe02e2d4,  0x0},
    { 0xe02e2d8,  0x0}, { 0xe02e2dc,  0x0},
    { 0xe02e2e0,  0x0}, { 0xe02e2e4,  0x0},
    { 0xe02e2e8,  0x0}, { 0xe02e2ec,  0x0},
    { 0xe02e2f0,  0x0}, { 0xe02e2f4,  0x0},
    { 0xe02e2f8,  0x0}, { 0xe02e2fc,  0x0},
    { 0xe02e300,  0x0}, { 0xe02e304,  0x0},
    { 0xe02e308,  0x0}, { 0xe02e30c,  0x0},
    { 0xe02e310,  0x0}, { 0xe02e314,  0x0},
    { 0xe02e318,  0x0}, { 0xe02e014,  0x10},
    { 0xe032c08,  0x2}, { 0xe032c08,  0x0},
    { 0xe02e004,  0x1}, { 0xe02e058,  0xa0a},
    { 0xe02e050,  0x32}, { 0xe02e1a0,  0x96},
    { 0xe030028,  0x96}, { 0xe03002c,  0x1ff},
    { 0xe02e004,  0x0}
};
#endif
#elif defined(LIBRA_BB) //LIBRA EVB
tRegList aLibraRFSetup[] =
{
    {0xe02e004, 0x1},    {0xe02e004, 0x0},
    {0xe02e004, 0x1},    {0xe03300c, 0x1},
    {0xe033008, 0xa0},   {0xe03100c, 0x1},
    {0xe02e17c, 0xa},    {0xe031414, 0x26c},
    {0xe031418, 0x1},    {0xe033400, 0x62},
    {0xe032808, 0x10},   {0xe032808, 0x0},
    {0xe030c08, 0x87e},  {0xe030d18, 0x640},
    {0xe030058, 0x0},    {0xe030064, 0x14},
    {0xe031830, 0x15},   {0xe02e040, 0x1},
    {0xe02e044, 0x11},   {0xe02e04c, 0x11},
    {0xe02e048, 0x1},    {0xe02e03c, 0x96c},
    {0xe033008, 0xb8},   {0xe03300c, 0x63},
    {0xe02e824, 0x1},    {0xe033008, 0xa0},
    {0xe03300c, 0x1},    {0xe02e198, 0x82},
    {0xe02e1a0, 0x8c},   {0xe02e1a4, 0x50},
    {0xe02e168, 0x19},   {0xe02e164, 0xb4},
    {0xe02e1a8, 0x2d},   {0xe02e1ac, 0x1e},
    {0xe02e1b0, 0x3c},   {0xe02e1bc, 0x78},
    {0xe02e1b8, 0x28},   {0xe02e008, 0x0},
    {0xe02e2bc, 0x1},    {0xe02e2c4, 0x0},
    {0xe02e2c8, 0x0},    {0xe02e2cc, 0x0},
    {0xe02e2d0, 0x0},    {0xe02e2d4, 0x0},
    {0xe02e2d8, 0x0},    {0xe02e2dc, 0x0},
    {0xe02e2e0, 0x0},    {0xe02e2e4, 0x0},
    {0xe02e2e8, 0x0},    {0xe02e2ec, 0x0},
    {0xe02e2f0, 0x0},    {0xe02e2f4, 0x0},
    {0xe02e2f8, 0x0},    {0xe02e2fc, 0x0},
    {0xe02e300, 0x0},    {0xe02e304, 0x0},
    {0xe02e308, 0x0},    {0xe02e30c, 0x0},
    {0xe02e310, 0x0},    {0xe02e314, 0x0},
    {0xe02e318, 0x0},    {0xe02e31c, 0x0},
    {0xe02e018, 0x10},   {0xe032c08, 0x2},
    {0xe032c08, 0x0},    {0xe02e004, 0x1},
    {0xe02e844, 0x9},    {0xe02e05c, 0xa},
    {0xe02e050, 0x5},    {0xe02e008, 0x4},
    {0xe02e054, 0xa},
    {0xe02e1a8, 0x64},   {0xe02e030, 0xc7},
    {0xe02e034, 0xc7},   {0xe02e320, 0x1},
    {0xe02e51c, 0x0L},   {0xe02e320, 0x0},
    {0xe03c000, 0xc},    {0xe03c400, 0x1d8},
    {0xe03c404, 0x760},  {0xe03c408, 0x0},
    {0xe03c40c, 0x0},    {0xe02c000, 0xf},
    {0xe02c004, 0x0},    {0xe02c004, 0x2},
    {0xe02f810, 0x400f}, {0xe02f81c, 0xc30},
    {0xe02f820, 0x211},  {0xe02fb04, 0x78},
    {0xe02fb08, 0x8},    {0xe02fb0c, 0x8},
    {0xe02e004, 0x0}
};
#elif defined(LIBRA_RF)
tRegList aLibraRFSetup[] =
{
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},/*
    {0xe02f820, 0x601},
    {0xe02f984, 0xff71},
    {0xe02f968, 0xa500},
    {0xe02f880, 0x1890},
    {0xe02f884, 0x773e},
    {0xe02f8c0, 0x80},
    {0xe02f8c4, 0xfc},
    {0xe02f8c8, 0x4f},
    {0xe02f8d0, 0x7d},
    {0xe02f820, 0x600},*/
    {0xe02f820, 0x601},
    {0xe02f984, 0xff71},
    {0xe02f968, 0xa500},
    {0xe02f820, 0x601},
    {0xe02f810, 0x420f},
    {0xe02f820, 0x600},
    {0xe02b118, 0x26L},
    {0xe02b118, 0x26L},
    {0xe028eb0, 0x1},
    {0xe02e004, 0x1},
    {0xe02e004, 0x0},
    {0xe02e004, 0x1},
    {0xe03300c, 0x1},
    {0xe033008, 0xa0},
    {0xe03100c, 0x1},
    {0xe02e17c, 0xa},
    {0xe031414, 0x26c},
    {0xe031418, 0x1},
    {0xe033400, 0x62},
    {0xe032808, 0x10},
    {0xe032808, 0x0},
    {0xe030c08, 0x87e},
    {0xe030d18, 0x640},
    {0xe030058, 0x0},
    {0xe030064, 0x13},
    {0xe031830, 0x15},
    {0xe02e040, 0x2},
    {0xe02e044, 0x12},
    {0xe02e04c, 0x12},
    {0xe02e048, 0x2},
    {0xe02e03c, 0x96c},
    {0xe033008, 0xb8},
    {0xe03300c, 0x63},
    {0xe02e824, 0x1},
    {0xe033008, 0xa0},
    {0xe03300c, 0x1},
    {0xe02e198, 0x78},
    {0xe02e1a0, 0x78},
    {0xe02e1a4, 0x50},
    {0xe02e168, 0x32},
    {0xe02e164, 0xb4},
    {0xe02e1a8, 0x28},
    {0xe02e1ac, 0x1e},
    {0xe02e1b0, 0x32},
    {0xe02e1bc, 0x6e},
    {0xe02e1b8, 0x32},
    {0xe02e008, 0x0},
    {0xe02e2bc, 0x1},
    {0xe02e2c4, 0x0},
    {0xe02e2c8, 0x0},
    {0xe02e2cc, 0x0},
    {0xe02e2d0, 0x0},
    {0xe02e2d4, 0x0},
    {0xe02e2d8, 0x0},
    {0xe02e2dc, 0x0},
    {0xe02e2e0, 0x0},
    {0xe02e2e4, 0x0},
    {0xe02e2e8, 0x0},
    {0xe02e2ec, 0x0},
    {0xe02e2f0, 0x0},
    {0xe02e2f4, 0x0},
    {0xe02e2f8, 0x0},
    {0xe02e2fc, 0x0},
    {0xe02e300, 0x0},
    {0xe02e304, 0x0},
    {0xe02e308, 0x0},
    {0xe02e30c, 0x0},
    {0xe02e310, 0x0},
    {0xe02e314, 0x0},
    {0xe02e318, 0x0},
    {0xe02e31c, 0x0},
    {0xe02e018, 0x10},
    {0xe032c08, 0x2},
    {0xe032c08, 0x0},
    {0xe02e004, 0x0},
    {0xe02e004, 0x1},
    {0xe02f810, 0x20f},
    {0xe02f808, 0x2ddb},
    {0xe02fa80, 0x7fdf},
    {0xe02f808, 0x2d5a},
    {0xe02fa80, 0x7fd6},
    {0xe02f808, 0x2cd9},
    {0xe02fa80, 0x7fcd},
    {0xe02f808, 0x2c58},
    {0xe02fa80, 0x7edf},
    {0xe02f808, 0x2bd7},
    {0xe02fa80, 0x7ed6},
    {0xe02f808, 0x2b56},
    {0xe02fa80, 0x7ecd},
    {0xe02f808, 0x2ad5},
    {0xe02fa80, 0x7e9f},
    {0xe02f808, 0x2a54},
    {0xe02fa80, 0x7e96},
    {0xe02f808, 0x29d3},
    {0xe02fa80, 0x7e8d},
    {0xe02f808, 0x2952},
    {0xe02fa80, 0x7e5f},
    {0xe02f808, 0x28d1},
    {0xe02fa80, 0x7e56},
    {0xe02f808, 0x2850},
    {0xe02fa80, 0x7e4d},
    {0xe02f808, 0x27cf},
    {0xe02fa80, 0x7d5f},
    {0xe02f808, 0x274e},
    {0xe02fa80, 0x7d56},
    {0xe02f808, 0x26cd},
    {0xe02fa80, 0x7d4d},
    {0xe02f808, 0x264c},
    {0xe02fa80, 0x7d1f},
    {0xe02f808, 0x25cb},
    {0xe02fa80, 0x7d16},
    {0xe02f808, 0x254a},
    {0xe02fa80, 0x7d0d},
    {0xe02f808, 0x24c9},
    {0xe02fa80, 0x7d1e},
    {0xe02f808, 0x2448},
    {0xe02fa80, 0x7d15},
    {0xe02f808, 0x23c7},
    {0xe02fa80, 0x7d0c},
    {0xe02f808, 0x2346},
    {0xe02fa80, 0x7d1d},
    {0xe02f808, 0x22c5},
    {0xe02fa80, 0x7d14},
    {0xe02f808, 0x2244},
    {0xe02fa80, 0x7d0b},
    {0xe02f808, 0x21c3},
    {0xe02fa80, 0x7d1c},
    {0xe02f808, 0x2142},
    {0xe02fa80, 0x7d13},
    {0xe02f808, 0x20c1},
    {0xe02fa80, 0x7d24},
    {0xe02f808, 0x2040},
    {0xe02fa80, 0x7d1b},
    {0xe02f808, 0x1fbf},
    {0xe02fa80, 0x7d12},
    {0xe02f808, 0x1f3e},
    {0xe02fa80, 0x7d23},
    {0xe02f808, 0x1ebd},
    {0xe02fa80, 0x7d1a},
    {0xe02f808, 0x1e3c},
    {0xe02fa80, 0x7d2b},
    {0xe02f808, 0x1dbb},
    {0xe02fa80, 0x7d22},
    {0xe02f808, 0x1d3a},
    {0xe02fa80, 0x7c1a},
    {0xe02f808, 0x1cb9},
    {0xe02fa80, 0x7c2b},
    {0xe02f808, 0x1c38},
    {0xe02fa80, 0x752b},
    {0xe02f808, 0x1bb7},
    {0xe02fa80, 0x7522},
    {0xe02f808, 0x1b36},
    {0xe02fa80, 0x6d1a},
    {0xe02f808, 0x1ab5},
    {0xe02fa80, 0x6d2b},
    {0xe02f808, 0x1a34},
    {0xe02fa80, 0x6d22},
    {0xe02f808, 0x19b3},
    {0xe02fa80, 0x6c1a},
    {0xe02f808, 0x1932},
    {0xe02fa80, 0x6523},
    {0xe02f808, 0x18b1},
    {0xe02fa80, 0x651a},
    {0xe02f808, 0x1830},
    {0xe02fa80, 0x652b},
    {0xe02f808, 0x17af},
    {0xe02fa80, 0x6522},
    {0xe02f808, 0x172e},
    {0xe02fa80, 0x641a},
    {0xe02f808, 0x16ad},
    {0xe02fa80, 0x5d2b},
    {0xe02f808, 0x162c},
    {0xe02fa80, 0x5d22},
    {0xe02f808, 0x15ab},
    {0xe02fa80, 0x5c1a},
    {0xe02f808, 0x152a},
    {0xe02fa80, 0x552b},
    {0xe02f808, 0x14a9},
    {0xe02fa80, 0x5522},
    {0xe02f808, 0x1428},
    {0xe02fa80, 0x4d1a},
    {0xe02f808, 0x13a7},
    {0xe02fa80, 0x4d2b},
    {0xe02f808, 0x1326},
    {0xe02fa80, 0x4d22},
    {0xe02f808, 0x12a5},
    {0xe02fa80, 0x4c1a},
    {0xe02f808, 0x1224},
    {0xe02fa80, 0x4523},
    {0xe02f808, 0x11a3},
    {0xe02fa80, 0x451a},
    {0xe02f808, 0x1122},
    {0xe02fa80, 0x452b},
    {0xe02f808, 0x10a1},
    {0xe02fa80, 0x4522},
    {0xe02f808, 0x1020},
    {0xe02fa80, 0x441a},
    {0xe02f808, 0xf9f},
    {0xe02fa80, 0x3d24},
    {0xe02f808, 0xf1e},
    {0xe02fa80, 0x3d1b},
    {0xe02f808, 0xe9d},
    {0xe02fa80, 0x3d12},
    {0xe02f808, 0xe1c},
    {0xe02fa80, 0x3d23},
    {0xe02f808, 0xd9b},
    {0xe02fa80, 0x3d1a},
    {0xe02f808, 0xd1a},
    {0xe02fa80, 0x3d2b},
    {0xe02f808, 0xc99},
    {0xe02fa80, 0x3d22},
    {0xe02f808, 0xc18},
    {0xe02fa80, 0x352b},
    {0xe02f808, 0xb97},
    {0xe02fa80, 0x3522},
    {0xe02f808, 0xb16},
    {0xe02fa80, 0x2d1a},
    {0xe02f808, 0xa95},
    {0xe02fa80, 0x2d2b},
    {0xe02f808, 0xa14},
    {0xe02fa80, 0x2d22},
    {0xe02f808, 0x993},
    {0xe02fa80, 0x2512},
    {0xe02f808, 0x912},
    {0xe02fa80, 0x2523},
    {0xe02f808, 0x891},
    {0xe02fa80, 0x251a},
    {0xe02f808, 0x810},
    {0xe02fa80, 0x252b},
    {0xe02f808, 0x78f},
    {0xe02fa80, 0x2522},
    {0xe02f808, 0x70e},
    {0xe02fa80, 0x1d1a},
    {0xe02f808, 0x68d},
    {0xe02fa80, 0x1d2b},
    {0xe02f808, 0x60c},
    {0xe02fa80, 0x1d22},
    {0xe02f808, 0x58b},
    {0xe02fa80, 0x1c1a},
    {0xe02f808, 0x50a},
    {0xe02fa80, 0x152b},
    {0xe02f808, 0x489},
    {0xe02fa80, 0x1522},
    {0xe02f808, 0x408},
    {0xe02fa80, 0x141a},
    {0xe02f808, 0x387},
    {0xe02fa80, 0xd2b},
    {0xe02f808, 0x306},
    {0xe02fa80, 0xd22},
    {0xe02f808, 0x285},
    {0xe02fa80, 0xc1a},
    {0xe02f808, 0x204},
    {0xe02fa80, 0x523},
    {0xe02f808, 0x183},
    {0xe02fa80, 0x51a},
    {0xe02f808, 0x102},
    {0xe02fa80, 0x52b},
    {0xe02f808, 0x81},
    {0xe02fa80, 0x522},
    {0xe02f808, 0x0},
    {0xe02fa80, 0x41a},
    {0xe02f808, 0x0},
    {0xe02e400, 0x0},
    {0xe02e404, 0x0},
    {0xe02e408, 0x0},
    {0xe02e40c, 0x0},
    {0xe02e410, 0x0},
    {0xe02e414, 0x0},
    {0xe02e418, 0x0},
    {0xe02e41c, 0x0},
    {0xe02e420, 0x0},
    {0xe02e424, 0x0},
    {0xe02e428, 0x0},
    {0xe02e42c, 0x0},
    {0xe02e430, 0x0},
    {0xe02e434, 0x0},
    {0xe02e438, 0x0},
    {0xe02e43c, 0x0},
    {0xe02e440, 0x0},
    {0xe02e444, 0x0},
    {0xe02e448, 0x0},
    {0xe02e44c, 0x0},
    {0xe02e450, 0x0},
    {0xe02e454, 0x0},
    {0xe02e458, 0x0},
    {0xe02e45c, 0x0},
    {0xe02e460, 0x0},
    {0xe02e464, 0x0},
    {0xe02e468, 0x0},
    {0xe02e46c, 0x0},
    {0xe02e470, 0x0},
    {0xe02e474, 0x0},
    {0xe02e478, 0x0},
    {0xe02e47c, 0x0},
    {0xe02e480, 0x0},
    {0xe02e484, 0x0},
    {0xe02e488, 0x0},
    {0xe02e48c, 0x0},
    {0xe02e490, 0x0},
    {0xe02e494, 0x0},
    {0xe02e498, 0x0},
    {0xe02e49c, 0x0},
    {0xe02e4a0, 0x0},
    {0xe02e4a4, 0x0},
    {0xe02e4a8, 0x0},
    {0xe02e4ac, 0x0},
    {0xe02e4b0, 0x0},
    {0xe02e4b4, 0x0},
    {0xe02e4b8, 0x0},
    {0xe02e4bc, 0x0},
    {0xe02e4c0, 0x0},
    {0xe02e4c4, 0x0},
    {0xe02e4c8, 0x0},
    {0xe02e4cc, 0x0},
    {0xe02e4d0, 0x0},
    {0xe02e4d4, 0x0},
    {0xe02e4d8, 0x0},
    {0xe02e4dc, 0x0},
    {0xe02e4e0, 0x0},
    {0xe02e4e4, 0x0},
    {0xe02e4e8, 0x0},
    {0xe02e4ec, 0x0},
    {0xe02e4f0, 0x0},
    {0xe02e4f4, 0x0},
    {0xe02e4f8, 0x0},
    {0xe02e4fc, 0x0},
    {0xe02e500, 0x0},
    {0xe02e504, 0x0},
    {0xe02e508, 0x0},
    {0xe02e50c, 0x0},
    {0xe02e510, 0x0},
    {0xe02e514, 0x0},
    {0xe02e518, 0x0},
    {0xe02e51c, 0x0},
    {0xe02e520, 0x0},
    {0xe02e524, 0x0},
    {0xe02e528, 0x0},
    {0xe02e52c, 0x0},
    {0xe02e530, 0x0},
    {0xe02e534, 0x0},
    {0xe02e538, 0x0},
    {0xe02e53c, 0x0},
    {0xe02e140, 0x5a},
    {0xe02e004, 0x0},
    {0xe02f808, 0x2e5},
    {0xe02fa80, 0x7fd},
    {0xe02f808, 0x2ed},
    {0xe02fa80, 0x7fd},
    {0xe02f808, 0x2f5},
    {0xe02fa80, 0x7fd},
    {0xe02f808, 0x2fd},
    {0xe02fa80, 0x7fd},
    {0xe02f808, 0x306},
    {0xe02fa80, 0x7fd},
    {0xe02f808, 0x30e},
    {0xe02fa80, 0x7fd},
    {0xe02f808, 0x316},
    {0xe02fa80, 0x7fd},
    {0xe02f808, 0x31e},
    {0xe02fa80, 0x7fd},
    {0xe02f808, 0x326},
    {0xe02fa80, 0x7fd},
    {0xe02f808, 0x32e},
    {0xe02fa80, 0x7fd},
    {0xe02f808, 0x336},
    {0xe02fa80, 0x7fd},
    {0xe02f808, 0x33e},
    {0xe02fa80, 0x7fd},
    {0xe02f808, 0x346},
    {0xe02fa80, 0x7fd},
    {0xe02f808, 0x34e},
    {0xe02fa80, 0x7fd},
    {0xe02f808, 0x356},
    {0xe02fa80, 0x7fd},
    {0xe02f808, 0x35e},
    {0xe02fa80, 0x7fd},
    {0xe02f808, 0x366},
    {0xe02fa80, 0x7fd},
    {0xe02f808, 0x36e},
    {0xe02fa80, 0x7fd},
    {0xe02f808, 0x376},
    {0xe02fa80, 0x7fd},
    {0xe02f808, 0x37e},
    {0xe02fa80, 0x7fd},
    {0xe02f808, 0x387},
    {0xe02fa80, 0x7fd},
    {0xe02f808, 0x38f},
    {0xe02fa80, 0x7fd},
    {0xe02f808, 0x397},
    {0xe02fa80, 0x7fd},
    {0xe02f808, 0x39f},
    {0xe02fa80, 0x7fd},
    {0xe02f808, 0x3a7},
    {0xe02fa80, 0x7fd},
    {0xe02f808, 0x3af},
    {0xe02fa80, 0x7fd},
    {0xe02f808, 0x3b7},
    {0xe02fa80, 0x7fd},
    {0xe02f808, 0x3bf},
    {0xe02fa80, 0x7fd},
    {0xe02f808, 0x3c7},
    {0xe02fa80, 0x7fd},
    {0xe02f808, 0x3cf},
    {0xe02fa80, 0x7fd},
    {0xe02f808, 0x3d7},
    {0xe02fa80, 0x7fd},
    {0xe02f808, 0x3df},
    {0xe02fa80, 0x7fd},
    {0xe02f808, 0x3e7},
    {0xe02fa80, 0x7fd},
    {0xe02f808, 0x3ef},
    {0xe02fa80, 0x7fd},
    {0xe02f808, 0x3f7},
    {0xe02fa80, 0x7fd},
    {0xe02f808, 0x3ff},
    {0xe02fa80, 0x7fd},
    {0xe02f808, 0x0},
    {0xe02e320, 0x1},
    {0xe02e400, 0x0},
    {0xe02e404, 0x0},
    {0xe02e408, 0x0},
    {0xe02e40c, 0x0},
    {0xe02e410, 0x0},
    {0xe02e414, 0x0},
    {0xe02e418, 0x0},
    {0xe02e41c, 0x0},
    {0xe02e420, 0x0},
    {0xe02e424, 0x0},
    {0xe02e428, 0x0},
    {0xe02e42c, 0x0},
    {0xe02e430, 0x0},
    {0xe02e434, 0x0},
    {0xe02e438, 0x0},
    {0xe02e43c, 0x0},
    {0xe02e440, 0x0},
    {0xe02e444, 0x0},
    {0xe02e448, 0x0},
    {0xe02e44c, 0x0},
    {0xe02e450, 0x0},
    {0xe02e454, 0x0},
    {0xe02e458, 0x0},
    {0xe02e45c, 0x0},
    {0xe02e460, 0x0},
    {0xe02e464, 0x0},
    {0xe02e468, 0x0},
    {0xe02e46c, 0x0},
    {0xe02e470, 0x0},
    {0xe02e474, 0x0},
    {0xe02e478, 0x0},
    {0xe02e47c, 0x0},
    {0xe02e480, 0x0},
    {0xe02e484, 0x0},
    {0xe02e488, 0x0},
    {0xe02e48c, 0x0},
    {0xe02e490, 0x0},
    {0xe02e494, 0x0},
    {0xe02e498, 0x0},
    {0xe02e49c, 0x0},
    {0xe02e4a0, 0x0},
    {0xe02e4a4, 0x0},
    {0xe02e4a8, 0x0},
    {0xe02e4ac, 0x0},
    {0xe02e4b0, 0x0},
    {0xe02e4b4, 0x0},
    {0xe02e4b8, 0x0},
    {0xe02e4bc, 0x0},
    {0xe02e4c0, 0x0},
    {0xe02e4c4, 0x0},
    {0xe02e4c8, 0x0},
    {0xe02e4cc, 0x0},
    {0xe02e4d0, 0x0},
    {0xe02e4d4, 0x0},
    {0xe02e4d8, 0x0},
    {0xe02e4dc, 0x0},
    {0xe02e4e0, 0x0},
    {0xe02e4e4, 0x0},
    {0xe02e4e8, 0x0},
    {0xe02e4ec, 0x0},
    {0xe02e4f0, 0x0},
    {0xe02e4f4, 0x0},
    {0xe02e4f8, 0x0},
    {0xe02e4fc, 0x0},
    {0xe02e500, 0x0},
    {0xe02e504, 0x0},
    {0xe02e508, 0x0},
    {0xe02e50c, 0x0},
    {0xe02e510, 0x0},
    {0xe02e514, 0x0},
    {0xe02e518, 0x0},
    {0xe02e51c, 0x0},
    {0xe02e520, 0x0},
    {0xe02e524, 0x0},
    {0xe02e528, 0x0},
    {0xe02e52c, 0x0},
    {0xe02e530, 0x0},
    {0xe02e534, 0x0},
    {0xe02e538, 0x0},
    {0xe02e53c, 0x0},
    {0xe02e140, 0x5a},
    {0xe02e004, 0x0},
    {0xe02f804, 0xf},
    {0xe02fa84, 0x3fdf},
    {0xe02f804, 0xe},
    {0xe02fa84, 0x3fd6},
    {0xe02f804, 0xd},
    {0xe02fa84, 0x3fcd},
    {0xe02f804, 0xc},
    {0xe02fa84, 0x3fde},
    {0xe02f804, 0xb},
    {0xe02fa84, 0x3fd5},
    {0xe02f804, 0xa},
    {0xe02fa84, 0x3fcc},
    {0xe02f804, 0x9},
    {0xe02fa84, 0x3fdd},
    {0xe02f804, 0x8},
    {0xe02fa84, 0x3fd4},
    {0xe02f804, 0x7},
    {0xe02fa84, 0x3fcb},
    {0xe02f804, 0x6},
    {0xe02fa84, 0x3fdc},
    {0xe02f804, 0x5},
    {0xe02fa84, 0x3fd3},
    {0xe02f804, 0x4},
    {0xe02fa84, 0x3fe4},
    {0xe02f804, 0x3},
    {0xe02fa84, 0x3fdb},
    {0xe02f804, 0x2},
    {0xe02fa84, 0x3fd2},
    {0xe02f804, 0x1},
    {0xe02fa84, 0x3fe3},
    {0xe02f804, 0x0},
    {0xe02fa84, 0x3fda},
    {0xe02e320, 0x1},
    {0xe02e400, 0x0L},
    {0xe02e404, 0x0L},
    {0xe02e408, 0x0L},
    {0xe02e40c, 0x0L},
    {0xe02e410, 0x0L},
    {0xe02e414, 0x0L},
    {0xe02e418, 0x0L},
    {0xe02e41c, 0x0L},
    {0xe02e420, 0x0L},
    {0xe02e424, 0x0L},
    {0xe02e428, 0x0L},
    {0xe02e42c, 0x0L},
    {0xe02e430, 0x0L},
    {0xe02e434, 0x0L},
    {0xe02e438, 0x0L},
    {0xe02e43c, 0x0L},
    {0xe02e440, 0x0L},
    {0xe02e444, 0x0L},
    {0xe02e448, 0x0L},
    {0xe02e44c, 0x0L},
    {0xe02e450, 0x0L},
    {0xe02e454, 0x0L},
    {0xe02e458, 0x0L},
    {0xe02e45c, 0x0L},
    {0xe02e460, 0x0L},
    {0xe02e464, 0x0L},
    {0xe02e468, 0x0L},
    {0xe02e46c, 0x0L},
    {0xe02e470, 0x0L},
    {0xe02e474, 0x0L},
    {0xe02e478, 0x0L},
    {0xe02e47c, 0x0L},
    {0xe02e480, 0x0L},
    {0xe02e484, 0x0L},
    {0xe02e488, 0x0L},
    {0xe02e48c, 0x0L},
    {0xe02e490, 0x0L},
    {0xe02e494, 0x0L},
    {0xe02e498, 0x0L},
    {0xe02e49c, 0x0L},
    {0xe02e4a0, 0x0L},
    {0xe02e4a4, 0x0L},
    {0xe02e4a8, 0x0L},
    {0xe02e4ac, 0x0L},
    {0xe02e4b0, 0x0L},
    {0xe02e4b4, 0x0L},
    {0xe02e4b8, 0x0L},
    {0xe02e4bc, 0x0L},
    {0xe02e4c0, 0x0L},
    {0xe02e4c4, 0x0L},
    {0xe02e4c8, 0x0L},
    {0xe02e4cc, 0x0L},
    {0xe02e4d0, 0x0L},
    {0xe02e4d4, 0x0L},
    {0xe02e4d8, 0x0L},
    {0xe02e4dc, 0x0L},
    {0xe02e4e0, 0x0L},
    {0xe02e4e4, 0x0L},
    {0xe02e4e8, 0x0L},
    {0xe02e4ec, 0x0L},
    {0xe02e4f0, 0x0L},
    {0xe02e4f4, 0x0L},
    {0xe02e4f8, 0x0L},
    {0xe02e4fc, 0x0L},
    {0xe02e500, 0x0L},
    {0xe02e504, 0x0L},
    {0xe02e508, 0x0L},
    {0xe02e50c, 0x0L},
    {0xe02e510, 0x0L},
    {0xe02e514, 0x0L},
    {0xe02e518, 0x0L},
    {0xe02e51c, 0x0L},
    {0xe02e520, 0x0L},
    {0xe02e524, 0x0L},
    {0xe02e528, 0x0L},
    {0xe02e52c, 0x0L},
    {0xe02e530, 0x0L},
    {0xe02e534, 0x0L},
    {0xe02e538, 0x0L},
    {0xe02e53c, 0x0L},
    {0xe02e320, 0x3},
    {0xe02e400, 0x0L},
    {0xe02e404, 0x0L},
    {0xe02e408, 0x0L},
    {0xe02e40c, 0x0L},
    {0xe02e410, 0x0L},
    {0xe02e414, 0x0L},
    {0xe02e418, 0x0L},
    {0xe02e41c, 0x0L},
    {0xe02e420, 0x0L},
    {0xe02e424, 0x0L},
    {0xe02e428, 0x0L},
    {0xe02e42c, 0x0L},
    {0xe02e430, 0x0L},
    {0xe02e434, 0x0L},
    {0xe02e438, 0x0L},
    {0xe02e43c, 0x0L},
    {0xe02e440, 0x0L},
    {0xe02e444, 0x0L},
    {0xe02e448, 0x0L},
    {0xe02e44c, 0x0L},
    {0xe02e450, 0x0L},
    {0xe02e454, 0x0L},
    {0xe02e458, 0x0L},
    {0xe02e45c, 0x0L},
    {0xe02e460, 0x0L},
    {0xe02e464, 0x0L},
    {0xe02e468, 0x0L},
    {0xe02e46c, 0x0L},
    {0xe02e470, 0x0L},
    {0xe02e474, 0x0L},
    {0xe02e478, 0x0L},
    {0xe02e47c, 0x0L},
    {0xe02e480, 0x0L},
    {0xe02e484, 0x0L},
    {0xe02e488, 0x0L},
    {0xe02e48c, 0x0L},
    {0xe02e490, 0x0L},
    {0xe02e494, 0x0L},
    {0xe02e498, 0x0L},
    {0xe02e49c, 0x0L},
    {0xe02e4a0, 0x0L},
    {0xe02e4a4, 0x0L},
    {0xe02e4a8, 0x0L},
    {0xe02e4ac, 0x0L},
    {0xe02e4b0, 0x0L},
    {0xe02e4b4, 0x0L},
    {0xe02e4b8, 0x0L},
    {0xe02e4bc, 0x0L},
    {0xe02e4c0, 0x0L},
    {0xe02e4c4, 0x0L},
    {0xe02e4c8, 0x0L},
    {0xe02e4cc, 0x0L},
    {0xe02e4d0, 0x0L},
    {0xe02e4d4, 0x0L},
    {0xe02e4d8, 0x0L},
    {0xe02e4dc, 0x0L},
    {0xe02e4e0, 0x0L},
    {0xe02e4e4, 0x0L},
    {0xe02e4e8, 0x0L},
    {0xe02e4ec, 0x0L},
    {0xe02e4f0, 0x0L},
    {0xe02e4f4, 0x0L},
    {0xe02e4f8, 0x0L},
    {0xe02e4fc, 0x0L},
    {0xe02e500, 0x0L},
    {0xe02e504, 0x0L},
    {0xe02e508, 0x0L},
    {0xe02e50c, 0x0L},
    {0xe02e510, 0x0L},
    {0xe02e514, 0x0L},
    {0xe02e518, 0x0L},
    {0xe02e51c, 0x0L},
    {0xe02e520, 0x0L},
    {0xe02e524, 0x0L},
    {0xe02e528, 0x0L},
    {0xe02e52c, 0x0L},
    {0xe02e530, 0x0L},
    {0xe02e534, 0x0L},
    {0xe02e538, 0x0L},
    {0xe02e53c, 0x0L},
    {0xe02e320, 0x2},
    {0xe02e13c, 0x50},
    {0xe03c000, 0xc},
    {0xe02e840, 0x1},
    {0xe02e004, 0x1},
    {0xe02fbd4, 0x2c81},
    {0xe02fbc8, 0x9e30},
    {0xe02fbc4, 0xfe},
    {0xe02f804, 0x0},
    {0xe02e004, 0x0},
    {0xe03c400, 0x1c9},
    {0xe03c404, 0x404},
    {0xe03c408, 0x0},
    {0xe03c40c, 0x0},
    {0xe03c410, 0x1c9},
    {0xe03c414, 0x404},
    {0xe03c418, 0x0},
    {0xe03c41c, 0x0},
    {0xe03c420, 0x1c9},
    {0xe03c424, 0x404},
    {0xe03c428, 0x0},
    {0xe03c42c, 0x0},
    {0xe03c430, 0x1c9},
    {0xe03c434, 0x404},
    {0xe03c438, 0x0},
    {0xe03c43c, 0x0},
    {0xe03c440, 0x1c9},
    {0xe03c444, 0x404},
    {0xe03c448, 0x0},
    {0xe03c44c, 0x0},
    {0xe03c450, 0x1c9},
    {0xe03c454, 0x404},
    {0xe03c458, 0x0},
    {0xe03c45c, 0x0},
    {0xe03c460, 0x1c9},
    {0xe03c464, 0x404},
    {0xe03c468, 0x0},
    {0xe03c46c, 0x0},
    {0xe03c470, 0x1c9},
    {0xe03c474, 0x404},
    {0xe03c478, 0x0},
    {0xe03c47c, 0x0},
    {0xe03c480, 0x1c9},
    {0xe03c484, 0x404},
    {0xe03c488, 0x0},
    {0xe03c48c, 0x0},
    {0xe03c490, 0x1c9},
    {0xe03c494, 0x404},
    {0xe03c498, 0x0},
    {0xe03c49c, 0x0},
    {0xe03c4a0, 0x1c9},
    {0xe03c4a4, 0x404},
    {0xe03c4a8, 0x0},
    {0xe03c4ac, 0x0},
    {0xe03c4b0, 0x1c9},
    {0xe03c4b4, 0x404},
    {0xe03c4b8, 0x0},
    {0xe03c4bc, 0x0},
    {0xe03c4c0, 0x1c9},
    {0xe03c4c4, 0x404},
    {0xe03c4c8, 0x0},
    {0xe03c4cc, 0x0},
    {0xe03c4d0, 0x1c9},
    {0xe03c4d4, 0x404},
    {0xe03c4d8, 0x0},
    {0xe03c4dc, 0x0},
    {0xe03c4e0, 0x1c9},
    {0xe03c4e4, 0x404},
    {0xe03c4e8, 0x0},
    {0xe03c4ec, 0x0},
    {0xe03c4f0, 0x1c9},
    {0xe03c4f4, 0x404},
    {0xe03c4f8, 0x0},
    {0xe03c4fc, 0x0},
    {0xe03c400, 0x1c9},
    {0xe03c404, 0x404},
    {0xe03c408, 0x0},
    {0xe03c40c, 0x0},
    {0xe03c410, 0x1c9},
    {0xe03c414, 0x404},
    {0xe03c418, 0x0},
    {0xe03c41c, 0x0},
    {0xe03c420, 0x1c9},
    {0xe03c424, 0x404},
    {0xe03c428, 0x0},
    {0xe03c42c, 0x0},
    {0xe03c430, 0x1c9},
    {0xe03c434, 0x404},
    {0xe03c438, 0x0},
    {0xe03c43c, 0x0},
    {0xe03c440, 0x1c9},
    {0xe03c444, 0x404},
    {0xe03c448, 0x0},
    {0xe03c44c, 0x0},
    {0xe03c450, 0x1c9},
    {0xe03c454, 0x404},
    {0xe03c458, 0x0},
    {0xe03c45c, 0x0},
    {0xe03c460, 0x1c9},
    {0xe03c464, 0x404},
    {0xe03c468, 0x0},
    {0xe03c46c, 0x0},
    {0xe03c470, 0x1c9},
    {0xe03c474, 0x404},
    {0xe03c478, 0x0},
    {0xe03c47c, 0x0},
    {0xe03c480, 0x1c9},
    {0xe03c484, 0x404},
    {0xe03c488, 0x0},
    {0xe03c48c, 0x0},
    {0xe03c490, 0x1c9},
    {0xe03c494, 0x404},
    {0xe03c498, 0x0},
    {0xe03c49c, 0x0},
    {0xe03c4a0, 0x1c9},
    {0xe03c4a4, 0x404},
    {0xe03c4a8, 0x0},
    {0xe03c4ac, 0x0},
    {0xe03c4b0, 0x1c9},
    {0xe03c4b4, 0x404},
    {0xe03c4b8, 0x0},
    {0xe03c4bc, 0x0},
    {0xe03c4c0, 0x1c9},
    {0xe03c4c4, 0x404},
    {0xe03c4c8, 0x0},
    {0xe03c4cc, 0x0},
    {0xe03c4d0, 0x1c9},
    {0xe03c4d4, 0x404},
    {0xe03c4d8, 0x0},
    {0xe03c4dc, 0x0},
    {0xe03c4e0, 0x1c9},
    {0xe03c4e4, 0x404},
    {0xe03c4e8, 0x0},
    {0xe03c4ec, 0x0},
    {0xe03c4f0, 0x1c9},
    {0xe03c4f4, 0x404},
    {0xe03c4f8, 0x0},
    {0xe03c4fc, 0x0},
    {0xe03c400, 0x1c9},
    {0xe03c404, 0x404},
    {0xe03c408, 0x0},
    {0xe03c40c, 0x0},
    {0xe03c410, 0x1c9},
    {0xe03c414, 0x404},
    {0xe03c418, 0x0},
    {0xe03c41c, 0x0},
    {0xe03c420, 0x1c9},
    {0xe03c424, 0x404},
    {0xe03c428, 0x0},
    {0xe03c42c, 0x0},
    {0xe03c430, 0x1c9},
    {0xe03c434, 0x404},
    {0xe03c438, 0x0},
    {0xe03c43c, 0x0},
    {0xe03c440, 0x1c9},
    {0xe03c444, 0x404},
    {0xe03c448, 0x0},
    {0xe03c44c, 0x0},
    {0xe03c450, 0x1c9},
    {0xe03c454, 0x404},
    {0xe03c458, 0x0},
    {0xe03c45c, 0x0},
    {0xe03c460, 0x1c9},
    {0xe03c464, 0x404},
    {0xe03c468, 0x0},
    {0xe03c46c, 0x0},
    {0xe03c470, 0x1c9},
    {0xe03c474, 0x404},
    {0xe03c478, 0x0},
    {0xe03c47c, 0x0},
    {0xe03c480, 0x1c9},
    {0xe03c484, 0x404},
    {0xe03c488, 0x0},
    {0xe03c48c, 0x0},
    {0xe03c490, 0x1c9},
    {0xe03c494, 0x404},
    {0xe03c498, 0x0},
    {0xe03c49c, 0x0},
    {0xe03c4a0, 0x1c9},
    {0xe03c4a4, 0x404},
    {0xe03c4a8, 0x0},
    {0xe03c4ac, 0x0},
    {0xe03c4b0, 0x1c9},
    {0xe03c4b4, 0x404},
    {0xe03c4b8, 0x0},
    {0xe03c4bc, 0x0},
    {0xe03c4c0, 0x1c9},
    {0xe03c4c4, 0x404},
    {0xe03c4c8, 0x0},
    {0xe03c4cc, 0x0},
    {0xe03c4d0, 0x1c9},
    {0xe03c4d4, 0x404},
    {0xe03c4d8, 0x0},
    {0xe03c4dc, 0x0},
    {0xe03c4e0, 0x1c9},
    {0xe03c4e4, 0x404},
    {0xe03c4e8, 0x0},
    {0xe03c4ec, 0x0},
    {0xe03c4f0, 0x1c9},
    {0xe03c4f4, 0x404},
    {0xe03c4f8, 0x0},
    {0xe03c4fc, 0x0},
    {0xe02fa90, 0xffff},
    {0xe02fa94, 0xffff},
    {0xe02e004, 0x0},
    {0xe033004, 0x7b9},
    {0xe02e004, 0x1},
    {0xe02f808, 0x0},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x81},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x102},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x183},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x204},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x285},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x306},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x387},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x408},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x489},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x50a},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x58b},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x60c},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x68d},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x70e},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x78f},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x810},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x891},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x912},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x993},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0xa14},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0xa95},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0xb16},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0xb97},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0xc18},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0xc99},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0xd1a},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0xd9b},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0xe1c},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0xe9d},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0xf1e},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0xf9f},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x1020},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x10a1},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x1122},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x11a3},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x1224},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x12a5},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x1326},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x13a7},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x1428},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x14a9},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x152a},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x15ab},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x162c},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x16ad},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x172e},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x17af},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x1830},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x18b1},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x1932},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x19b3},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x1a34},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x1ab5},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x1b36},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x1bb7},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x1c38},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x1cb9},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x1d3a},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x1dbb},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x1e3c},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x1ebd},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x1f3e},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x1fbf},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x2040},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x20c1},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x2142},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x21c3},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x2244},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x22c5},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x2346},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x23c7},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x2448},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x24c9},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x254a},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x25cb},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x264c},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x26cd},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x274e},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x27cf},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x2850},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x28d1},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x2952},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x29d3},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x2a54},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x2ad5},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x2b56},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x2bd7},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x2c58},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x2cd9},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x2d5a},
    {0xe02f810, 0xf},
    {0xe02fa88, 0x91aa},
    {0xe02fa88, 0x91aa},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x0},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaaaa},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x81},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x102},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x183},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x204},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x285},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x306},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x387},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x408},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x489},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x50a},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x58b},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x60c},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x68d},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x70e},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x78f},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x810},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x891},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x912},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x993},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0xa14},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0xa95},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0xb16},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0xb97},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0xc18},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0xc99},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0xd1a},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0xd9b},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0xe1c},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0xe9d},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0xf1e},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0xf9f},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x1020},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x10a1},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x1122},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x11a3},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x1224},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x12a5},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x1326},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x13a7},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x1428},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x14a9},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x152a},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x15ab},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x162c},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x16ad},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x172e},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x17af},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x1830},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x18b1},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x1932},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x19b3},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x1a34},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x1ab5},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x1b36},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x1bb7},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x1c38},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x1cb9},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x1d3a},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x1dbb},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x1e3c},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x1ebd},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x1f3e},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x1fbf},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x2040},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x20c1},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x2142},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x21c3},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x2244},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x22c5},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x2346},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x23c7},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x2448},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x24c9},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x254a},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x25cb},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x264c},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x26cd},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x274e},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x27cf},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x2850},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x28d1},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x2952},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x29d3},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x2a54},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x2ad5},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x2b56},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x2bd7},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x2c58},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02f808, 0x2cd9},
    {0xe02f810, 0x10f},
    {0xe02fa88, 0xaa7a},
    {0xe02fa88, 0xaa7a},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},/*
    {0xe02f820, 0x601},
    {0xe02f984, 0xff71},
    {0xe02f968, 0xa500},
    {0xe02f880, 0x1890},
    {0xe02f884, 0x773e},
    {0xe02f8c0, 0x80},
    {0xe02f8c4, 0xfc},
    {0xe02f8c8, 0x4f},
    {0xe02f8d0, 0x7d},
    {0xe02f820, 0x600},*/
    {0xe02f820, 0x601},
    {0xe02e004, 0x1},
    {0xe02e004, 0x1},/*
    {0xe02f820, 0x601},
    {0xe02f984, 0xff71},
    {0xe02f968, 0xa500},
    {0xe02f880, 0x1890},
    {0xe02f884, 0x773e},
    {0xe02f8c0, 0x80},
    {0xe02f8c4, 0xfc},
    {0xe02f8c8, 0x4f},
    {0xe02f8d0, 0x7d},
    {0xe02f820, 0x600}*/

};
#endif //LIBRA_FPGA

#if 0
static tANI_BOOLEAN
__halIsChipBusy(tpAniSirGlobal pMac)
{
    tANI_U32    freeBdPduCnt;
    tANI_U32    regVal = 0;

    halReadRegister((tHalHandle) pMac, QWLAN_BMU_AVAILABLE_BD_PDU_AFTER_RSV_REG, &regVal);
    /** Calculate the Free BDS From the Register*/
    freeBdPduCnt = ((regVal & QWLAN_BMU_AVAILABLE_BD_PDU_AFTER_RSV_AVAILABLE_BDS_MASK) >>
                                                    QWLAN_BMU_AVAILABLE_BD_PDU_AFTER_RSV_AVAILABLE_BDS_OFFSET);

    /** Calculate the Free PDUs from the register and sum up BDs Count*/
    freeBdPduCnt += ((regVal & QWLAN_BMU_AVAILABLE_BD_PDU_AFTER_RSV_AVAILABLE_PDUS_MASK) >>
                                                    QWLAN_BMU_AVAILABLE_BD_PDU_AFTER_RSV_AVAILABLE_PDUS_OFFSET);

    if (freeBdPduCnt < (pMac->hal.halMac.halMaxBdPduAvail - MAX_ALLOWED_BD_FOR_IDLE))
       return eANI_BOOLEAN_TRUE;

    return eANI_BOOLEAN_FALSE;
}
#endif

tANI_BOOLEAN halIsSelfHtCapable(tpAniSirGlobal pMac)
{
    tpStaStruct pSta;
    if(NULL == pMac)
    {
        HALLOGP( halLog(pMac, LOGP, FL("pMac is NULL\n")));
        return eANI_BOOLEAN_FALSE;
    }
    pSta = (tpStaStruct)pMac->hal.halMac.staTable;
    if(true == pSta[pMac->hal.halMac.selfStaId].htEnabled)
        return eANI_BOOLEAN_TRUE;
    else
        return eANI_BOOLEAN_FALSE;
}

/** -------------------------------------------------------------
\fn      halPhy_setNwDensityAndProximity
\brief   This function sets the network density and proximity
\        configurations. When there is a change in network
\        density setting, we need to also update the adaptive
\        threshold setting as well.
\param   tpAniSirGlobal pMac
\return  none
  -------------------------------------------------------------*/
static void halPhy_setNwDensityAndProximity(tpAniSirGlobal pMac)
{
    tANI_U32      proximity;
    tANI_U32      nwDensity;
    tANI_BOOLEAN  densityOn;
    eHalStatus    status;

    if (wlan_cfgGetInt(pMac, WNI_CFG_PROXIMITY, (tANI_U32 *) &proximity) != eSIR_SUCCESS)
        HALLOGP( halLog(pMac, LOGP, FL("cfgGet(WNI_CFG_PROXIMITY) failed \n")));

    if (wlan_cfgGetInt(pMac, WNI_CFG_NETWORK_DENSITY, (tANI_U32 *) &nwDensity) != eSIR_SUCCESS)
        HALLOGP( halLog(pMac, LOGP, FL("cfgGet(WNI_CFG_NETWORK_DENSITY) failed \n")));

    if (proximity == WNI_CFG_PROXIMITY_OFF)
        densityOn = eANI_BOOLEAN_TRUE;
    else
        densityOn = eANI_BOOLEAN_FALSE;

#ifdef LIBRA_RF
    {
        HALLOG1( halLog(pMac, LOG1, FL("nwDensity=%d, proximity=%d, densityOn=%d \n"), nwDensity, proximity, densityOn));

        status = halPhySetNwDensity( pMac, densityOn, (ePhyNwDensity)nwDensity, (ePhyNwDensity)nwDensity );
        if (status != eHAL_STATUS_SUCCESS)
            HALLOGP( halLog(pMac, LOGP, FL("halPhySetNwDensity() failed")));
    }
#endif //LIBRA_RF
    //halRate_updateRateTable(pMac);

    return;
}



// -------------------------------------------------------------
/**
 * halDoCfgInit
 *
 * FUNCTION:
 *     Initiates configuration and waits till configuration is done.
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 *
 * NOTE:
 *
 * @param pMac MAC Global instance
 * @return tSirRetStatus SUCCESS or FAILURE
 */
tSirRetStatus halDoCfgInit(tpAniSirGlobal pMac)
{
    tSirRetStatus rc = eSIR_SUCCESS;
    tSirMsgQ msg;

    // Initializes calibration and waits until cfg is done.
    // Send Mailbox message START to host
    msg.type = SIR_HAL_APP_SETUP_NTF;

    // Construct the message and post it
    msg.bodyptr = NULL;
    msg.bodyval = 0;

    halMmhPostMsgApi(pMac, &msg, eHI_PRI);

    // Initialize config and stats module
    wlan_cfgInit(pMac);

    // we're done for now.
    // we'll be notified via a SIR_CFG_DOWNLOAD_COMPLETE_IND when the
    // config & stats module has finish its exchange with the host.

            return rc;
        }

// -------------------------------------------------------------
/**
 * halProcessCfgDownloadComplete
 *
 * FUNCTION:
 *     Processing performed after configuration is done.
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 *
 * NOTE:
 *
 * @param pMac MAC Global instance
 * @return tSirRetStatus SUCCESS or FAILURE
 */
tSirRetStatus halProcessCfgDownloadComplete(tpAniSirGlobal pMac)
        {
    tSirRetStatus rc = eSIR_SUCCESS;
    tANI_U32 val;

    pMac->hal.halMac.macStats.periodicStats = false; //FALSE by default. Use Dump to enable.
    if ((rc = wlan_cfgGetInt(pMac, WNI_CFG_STATS_PERIOD,
                        &val)) != eSIR_SUCCESS)
    {
        HALLOGE( halLog(pMac, LOGE, FL("CFG Failed STATS collection period\n")));
        macSysResetReq(pMac, rc);
        goto end;
    }

    pMac->hal.halMac.macStats.statTmrVal = SYS_SEC_TO_TICKS(val);
    pMac->hal.halMac.wrapStats.statTmrVal = SYS_SEC_TO_TICKS(val);
    pMac->hal.halMac.tempMeasTmrVal = SYS_SEC_TO_TICKS(HAL_TEMPMEAS_TIMER_VAL_SEC);

    if(pMac->gDriverType != eDRIVER_TYPE_MFG) // Enable periodic calibration only if it is not the Manufacturing Diagnostics
    {
                          // driver build.
        rc = halConfigCalControl(pMac);
        if (rc != eSIR_SUCCESS)
        {
            HALLOGE(halLog(pMac, LOGE, FL("halConfigCalControl: CFG Failed Calibration Control\n")));
            macSysResetReq(pMac, rc);
            goto end;
        }
        //pMac->hal.trigCalFlag = (tANI_U8) val;

    } // #ifndef eDRIVER_TYPE_MFG


    if ((rc = wlan_cfgGetInt(pMac, WNI_CFG_CAL_PERIOD, &val)) != eSIR_SUCCESS)
    {
        HALLOGE( halLog(pMac, LOGE, FL("Failed to get CFG CAL PERIOD\n")));
        macSysResetReq(pMac, rc);
        goto end;
    }
    else
    {
        if (val == 0)
        {
            pMac->hal.halMac.tempMeasTmrVal = SYS_SEC_TO_TICKS(HAL_TEMPMEAS_TIMER_VAL_SEC);
        }
        else
        {
            pMac->hal.halMac.tempMeasTmrVal = SYS_MIN_TO_TICKS(val)/HAL_PHY_PERIODIC_CAL_ITER_LIMIT;
        }
    }
    halStateSet(pMac, eHAL_CFG);
end:

    /** Initialize the Firmware Heart Beat Monitor Values.*/
    pMac->hal.halMac.fwMonitorthr = 0;
    pMac->hal.halMac.fwHeartBeatPrev = 0;

    /** Initialize the Phy MPI tx counter values to detect PHY hang.*/
    pMac->hal.halMac.phyHangThr = 0;
    pMac->hal.halMac.mpiTxSent = 0;
    pMac->hal.halMac.mpiTxAbort = 0;

    // Post START event to HAL's event queue
    rc = halProcessStartEvent(pMac);

    return rc;
}

//dummy function for now to register to BAL as fatal error callback.
VOS_STATUS halFatalErrorHandler(v_PVOID_t pVosGCtx, v_U32_t errorCode)
{
    return VOS_STATUS_SUCCESS;
}

/** -------------------------------------------------------------
\fn halSetReadyToHandleInt
\brief      Enables interrupt and also set hal state to normal. resets if fails to enable interrupt.
\param   tpAniSirGlobal pMac
\return none
  -------------------------------------------------------------*/
static void halSetReadyToHandleInt(tpAniSirGlobal pMac)
{
    if(eHAL_STATUS_SUCCESS != halIntChipEnable((tHalHandle)pMac))
    {
        HALLOGP( halLog(pMac, LOGP, FL("halIntChipEnable failed\n")));
    }
    else
        halStateSet(pMac, eHAL_NORMAL);
}

// -------------------------------------------------------------
/**
 * halProcessStartEvent
 *
 * FUNCTION:
 *     Initializes the HW and the system.
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 *
 * NOTE:
 *
 * @param pMac MAC parameter structure pointer
 * @return NONE
 */

tSirRetStatus halProcessStartEvent(tpAniSirGlobal pMac)
{
    tSirMsgQ msg;
    tSirMbMsg *mb;
    tANI_U32 status;
    tSirRetStatus rc = eSIR_SUCCESS;
    tANI_U32    cfgVal = 0;

    do
    {
        HALLOG3( halLog(pMac, LOG3, FL("Entered!\n")));
        {
            // shall be called only AFTER CFG download is finsihed
            if (halInitWmParam(pMac, NULL) != eHAL_STATUS_SUCCESS)
            {
               HALLOGW( halLog(pMac, LOGW, FL("Failed at halInitWmParam() \n")));
               rc = eSIR_FAILURE;
               break;
            }

            /** Get the RTS Threshold */
            if ( wlan_cfgGetInt(pMac, WNI_CFG_RTS_THRESHOLD, &cfgVal) != eSIR_SUCCESS) {
                HALLOGE(halLog(pMac, LOGE, FL("cfgGet WNI_CFG_RTS_THRESHOLD Failed\n")));
            }

            /** Set the Protection threshold */
            if (halTpe_SetProtectionThreshold(pMac, cfgVal) != eHAL_STATUS_SUCCESS) {
                return eSIR_FAILURE;
            }

            if(pMac->gDriverType != eDRIVER_TYPE_MFG)
            {
                // Init BA parameters
                baInit( pMac );
            }

            pMac->hal.halMac.nonRifsBssCount = pMac->hal.halMac.rifsBssCount= 0;

#if defined(ANI_PRODUCT_TYPE_CLIENT)
            // Initialize Adaptive Threshold related globals
            halATH_initialize(pMac);
#endif

#ifdef ANI_SUPPORT_SMPS
            halMsg_InitRxChainsReg(pMac);
#endif

            if(pMac->gDriverType != eDRIVER_TYPE_MFG)
            {
                halPhy_setNwDensityAndProximity(pMac);
            }
            /*overwrite default behaviour to be compatible with marvell chipset*/
            halPhyRxSoundingBitFrames( pMac, eANI_BOOLEAN_TRUE );

            msg.type = SIR_LIM_RESUME_ACTIVITY_NTF;
            status = limPostMsgApi(pMac, &msg);
            if (status != eSIR_SUCCESS)
            {
                // above api does not post LOGP on error
                HALLOGP( halLog(pMac, LOGP,
                       FL("Failed limPostMsgApi=0%X\n"),
                       status));
                rc = eSIR_FAILURE;
                break;
            }
            HALLOGW( halLog(pMac, LOGW, FL("limresumeactivityntf is sent from hal\n")));

#if !defined(LOOPBACK) && !defined(ANI_DVT_DEBUG)
            if(pMac->gDriverType != eDRIVER_TYPE_MFG)
            {

                if (halMsg_AddStaSelf(pMac) != eHAL_STATUS_SUCCESS)
                {
                    HALLOGW( halLog(pMac, LOGW, FL("Failed at halMsg_AddStaSelf() \n")));
                    rc = eSIR_FAILURE;
                    break;
                }
            }
#endif
            if(pMac->gDriverType != eDRIVER_TYPE_MFG)
            {
                if (halRxp_addBroadcastEntry(pMac) != eHAL_STATUS_SUCCESS)
                {
                    HALLOGW( halLog(pMac, LOGW, FL("Failed at halRxp_addBroadcastEntry() \n")));
                    rc = eSIR_FAILURE;
                    break;
                }
            }
            // Start HAL timers create
            if ((rc = halTimersCreate(pMac)) != eSIR_SUCCESS)
                break;

#if defined(ANI_LED_ENABLE)
            // Init LED.
            halInitLed(pMac);
#endif
            // Post NIC_OPERATIONAL message to HDD
            msg.type = SIR_HAL_NIC_OPER_NTF;
            if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&mb, 8))
            {
                rc = eSIR_MEM_ALLOC_FAILED;
                HALLOGW( halLog(pMac, LOGP,
                       FL("Alloc failed for OPER_NTF\n")));
                break;
            }
#if defined (ANI_PRODUCT_TYPE_AP) && defined (ANI_LITTLE_BYTE_ENDIAN)
            sirStoreU16N((tANI_U8*)mb, SIR_HAL_NIC_OPER_NTF);
            sirStoreU16N(((tANI_U8*)mb+2), 8);
            sirStoreU32N(((tANI_U8*)mb+4), 0);
#else
            mb->type    = SIR_HAL_NIC_OPER_NTF;
            mb->msgLen  = 8;    // len in bytes
            mb->data[0] = 0;
#endif
            msg.bodyptr = mb;
            msg.bodyval = 0;
            halMmhPostMsgApi(pMac, &msg, eHI_PRI);
            //if we have already received sys_ready then we can go to normal state.
            if(eHAL_SYS_READY == halStateGet(pMac))
                halSetReadyToHandleInt(pMac);
            else
                halStateSet(pMac, eHAL_STARTED);

            HALLOGP( halLog(pMac, LOGE,
                   FL("halProcessStartEvent: Completed, State %d!\n"),
                    halStateGet(pMac)));
#ifndef LIBRA_RF
            {
                tANI_U32 i;

                for (i=0; i< sizeof(aLibraRFSetup)/sizeof(aLibraRFSetup[0]); i++)
                    halWriteRegister(pMac, aLibraRFSetup[i].reg, aLibraRFSetup[i].val);

            }
#endif
        }
    }
    while (0);
    return rc;

} // halProcessStartEvent

/** -------------------------------------------------------------
\fn halProcessSysReadyInd
\brief handles the notification from HDD forwaded by PE.
\        right now here we just enable all the interrupts.
\param   tpAniSirGlobal pMac
\return eSirRetStatus - status
  -------------------------------------------------------------*/
tSirRetStatus halProcessSysReadyInd(tpAniSirGlobal pMac)
{
    tSirRetStatus rc = eSIR_SUCCESS;
    //if we have already received start event then we can enable interrupts and
    //change HAL state to normal.
    if(eHAL_STARTED == halStateGet(pMac))
        halSetReadyToHandleInt(pMac);
    else
        halStateSet(pMac, eHAL_SYS_READY);

    HALLOGW( halLog(pMac, LOGW, FL("After SYS_READY process halState = %d\n"),
          halStateGet(pMac)));
    return rc;
}

/** -------------------------------------------------------------
\fn halProcessMulticastRateChange
\brief handles the CFG change for mlticast rates.
\param  tpAniSirGlobal pMac
\param  tANI_U32 cfgId
\return eHalStatus status
  -------------------------------------------------------------*/
static
eHalStatus halProcessMulticastRateChange(tpAniSirGlobal pMac, tANI_U32 cfgId)
{
    tANI_U32 val;
    eHalStatus status = eHAL_STATUS_SUCCESS;
        eRfBandMode curBand;
        curBand = halUtil_GetRfBand(pMac, pMac->hal.currentChannel);
        //if the multicast rate got changed for the current band then inform softmac.
        if(((WNI_CFG_FIXED_RATE_MULTICAST_24GHZ == cfgId) &&
            (eRF_BAND_2_4_GHZ == curBand)) ||
          ((WNI_CFG_FIXED_RATE_MULTICAST_5GHZ == cfgId) &&
            (eRF_BAND_5_GHZ == curBand)))
        {
        if(eSIR_SUCCESS != wlan_cfgGetInt(pMac, (tANI_U16)cfgId, &val))
        {
            HALLOGP( halLog(pMac, LOGP, FL("Get cfg id (%d) failed \n"), cfgId));
            return eHAL_STATUS_FAILURE;
        }
        else
        {
            tTpeRateIdx tpeRateIdx = halRate_cfgFixedRate2TpeRate(val);
            halSetMulticastRateIdx(pMac, tpeRateIdx);
        }
    }
    return status;
}

// -------------------------------------------------------------
/**
 * halInitStartReqHandler
 *
 * FUNCTION:
 *     Handles init start request
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 *
 * NOTE:
 *
 * @param pMac MAC parameter structure pointer
 * @return NONE
 */

tSirRetStatus halInitStartReqHandler(tpAniSirGlobal pMac)
{
    tSirRetStatus rc = eSIR_SUCCESS;
    tANI_U8 halState = halStateGet(pMac);

    if (halState != eHAL_INIT)
    {
        HALLOGP( halLog(pMac, LOGE,
               FL("halInitStartReqHandler: INIT_START_REQ: Invalid HAL state %d\n"),
               halState));
        rc = eSIR_FAILURE;
        goto end;
    }


    /* Initializes all static HW initializations & some of the
     * global variables.
     * Enable BMU command processing
     * Enable all the master work queues
     * Enable RHP for receiving packets on the receive path.
     * Enables Interrupts
     */

#ifdef ANI_OS_TYPE_RTAI_LINUX

    // Resume all tasks
    if (tx_thread_resume(&pMac->sys.gSirMntThread) != TX_SUCCESS)
    {
        HALLOGE( halLog(pMac, LOGE, FL("Could not resume MNT thread!\n")));
        rc = eSIR_SYS_TX_THREAD_RESUME_FAILED;
        macSysResetReq(pMac, rc);
    }
#endif
    // Initializes and performs config download
    rc = halDoCfgInit(pMac);

  end:
    return rc;
}

/** ------------------------------------------------------
\fn      halProcessMsg
\brief   This function process HAL messages
\param   tpAniSirGlobal  pMac
\param   tSirMsgQ  *pMsg - HAL message to be processed
\return  status
\ -------------------------------------------------------- */
tSirRetStatus halProcessMsg(tpAniSirGlobal pMac, tSirMsgQ *pMsg )
{
    tSirRetStatus   rc = eSIR_SUCCESS;
    tHalMsgDecision  msgStatus;
    tANI_U8 mutexAcquired = FALSE;

    // If hal state is IDLE, do not process any messages.
    // free the body pointer and return success
    if(pMac->gDriverType == eDRIVER_TYPE_PRODUCTION)
    {
        if(eHAL_IDLE == halStateGet(pMac)) {
            if(pMsg->bodyptr) {
                vos_mem_free((v_VOID_t*)pMsg->bodyptr);
            }
            return eSIR_SUCCESS;
        }
    }

#ifndef WLAN_FTM_STUB
    if(pMac->gDriverType == eDRIVER_TYPE_MFG)
    {
        tANI_U32                    pttType;
        tPttMsgbuffer               *pPttMsg;
        tANI_U8                     *pReq;
        pReq = (tANI_U8*) pMsg->bodyptr;
        pPttMsg = (tPttMsgbuffer *)pReq;
        pttType = pMsg->type & HAL_MMH_MB_MSG_TYPE_MASK;

        if (pttType >= PTT_MSG_TYPES_BEGIN_30 &&  pttType <= PTT_MSG_TYPES_BEGIN_32)
        {
            pttProcessMsg(pMac, pPttMsg);
            return(rc);
        }
        //return rc;
    }
#endif

    msgStatus = halUtil_MsgDecision(pMac, pMsg, &mutexAcquired);
    if (msgStatus == eHAL_MSG_DROP) {
        /**@todo free message.*/
        return eSIR_SUCCESS;
    }

    if (msgStatus == eHAL_MSG_DEFER)
    {
      if(eHAL_STATUS_SUCCESS != halUtil_deferMsg(pMac, pMsg))
      {
        rc = eSIR_FAILURE;
      }
    }
    else
    {

        rc = halHandleMsg(pMac, pMsg);

//        halUtil_processDeferredMsgQ(pMac);

        // Release the mutex if acquired.
        if (mutexAcquired) {
            halPS_ReleaseHostBusy(pMac, HAL_PS_BUSY_GENERIC);
        }
    }

    return rc;
}


/** ------------------------------------------------------
\fn      halHandleMsg
\brief   This function process HAL messages
\param   tpAniSirGlobal  pMac
\param   tSirMsgQ  *pMsg - HAL message to be processed
\return  status
\ -------------------------------------------------------- */
tSirRetStatus halHandleMsg(tpAniSirGlobal pMac, tSirMsgQ *pMsg )
{
    tSirRetStatus   rc = eSIR_SUCCESS;
    eHalStatus      status = eHAL_STATUS_SUCCESS;
    tANI_U32        val;
    tANI_U16        dialogToken = pMsg->reserved;

    if(pMac->gDriverType == eDRIVER_TYPE_MFG)
    {
        switch (pMsg->type)
        {
           case SIR_HAL_HANDLE_FW_MBOX_RSP:
                HALLOGE( halLog(pMac, LOGE, FL("Fw Rsp Msg \n")));
                halFW_HandleFwMessages(pMac, pMsg->bodyptr);
                vos_mem_free((v_VOID_t*)pMsg->bodyptr);
                pMsg->bodyptr = NULL;
                break;
    
           case SIR_HAL_SEND_MSG_COMPLETE:
                HALLOGE( halLog(pMac, LOGE, FL("Fw Rsp Msg \n")));
                halMbox_SendMsgComplete(pMac);
                break; 

           case SIR_HAL_TIMER_ADC_RSSI_STATS:
                tx_timer_deactivate(&pMac->ptt.adcRssiStatsTimer);
                halPhyAdcRssiStatsCollection(pMac);
                tx_timer_activate(&pMac->ptt.adcRssiStatsTimer);
                break;
        }
        return eSIR_SUCCESS;
    }

    switch (pMsg->type)
    {
        case SIR_HAL_INIT_START_REQ:
            {
                vos_mem_free((v_VOID_t*)pMsg->bodyptr);
                pMsg->bodyptr = NULL;

                rc = halInitStartReqHandler(pMac);
            }
            break;

        case SIR_HAL_HDD_ADDBA_RSP:
            status = baProcessTLAddBARsp(pMac, ((tpAddBARsp)pMsg->bodyptr)->baSessionID, ((tpAddBARsp)pMsg->bodyptr)->replyWinSize);
            vos_mem_free((v_VOID_t*)pMsg->bodyptr);
            pMsg->bodyptr = NULL;
            break;

        case SIR_HAL_SYS_READY_IND:
            {
                vos_mem_free((v_VOID_t*)pMsg->bodyptr);
                pMsg->bodyptr = NULL;

                rc = halProcessSysReadyInd(pMac);
            }
            break;

        case SIR_CFG_PARAM_UPDATE_IND:
            if (wlan_cfgGetInt(pMac, (tANI_U16) pMsg->bodyval, &val) != eSIR_SUCCESS)
            {
                HALLOGP( halLog(pMac, LOGP, FL("Failed to cfg get id %d\n"), pMsg->bodyval));
                return eSIR_FAILURE;
            }

            switch (pMsg->bodyval)
            {
                case WNI_CFG_STA_ID:
                    if( (status = halMsg_AddStaSelf(pMac)) != eHAL_STATUS_SUCCESS)
                        HALLOGW( halLog(pMac, LOGW, FL("halMsg_AddStaSelf() failed \n")));
                    break;

                case WNI_CFG_PACKET_CLASSIFICATION:
                    pMac->hal.halMac.frameClassifierEnabled = (tANI_U16) val;
                    break;

                case WNI_CFG_RTS_THRESHOLD:
                    if ( wlan_cfgGetInt(pMac, WNI_CFG_RTS_THRESHOLD, &val) != eSIR_SUCCESS) {
                        HALLOGE(halLog(pMac, LOGE, FL("cfgGet WNI_CFG_RTS_THRESHOLD Failed\n")));
                    }

                    if (halMsg_UpdateTpeProtectionThreshold(pMac, val) != eHAL_STATUS_SUCCESS) {
                        return eSIR_FAILURE;
                    }
                    break;

                case WNI_CFG_SHORT_RETRY_LIMIT:
                case WNI_CFG_LONG_RETRY_LIMIT:
                    if( (status = halMsg_updateRetryLimit(pMac)) != eHAL_STATUS_SUCCESS){
                        HALLOGW( halLog(pMac, LOGW, FL("halMsg_sendWmParam() failed \n")));
                    }
                    break;

                case WNI_CFG_FRAGMENTATION_THRESHOLD:
                    if ( (status = halMsg_updateFragThreshold(pMac)) != eHAL_STATUS_SUCCESS){
                        HALLOGW( halLog(pMac, LOGW, FL("halDPU_updateFragThreshold() failed \n")));
                    }
                    break;

                case WNI_CFG_CURRENT_TX_ANTENNA:
                    pMac->hal.cfgTxAntenna = val;
                    {
                        ePhyChainSelect chainSelect = halPhyGetChainSelect(pMac, (tANI_U8)pMac->hal.cfgTxAntenna, (tANI_U8)pMac->hal.cfgRxAntenna);
                        if (chainSelect != INVALID_PHY_CHAIN_SEL)
                        {
                            if (halPhySetChainSelect(pMac, chainSelect) != eHAL_STATUS_SUCCESS){
                                HALLOGE( halLog(pMac, LOGE, FL("halPhySetChainSelect() failed \n")));
                            }
                        }
                        else
                        {
                            HALLOGE( halLog(pMac, LOGE, FL("WNI_CFG_CURRENT_TX_ANTENNA: Incorrect chain selection")));
                        }
                    }
                    break;


                case WNI_CFG_CURRENT_RX_ANTENNA:
                    pMac->hal.cfgRxAntenna = val;
                    {
                        ePhyChainSelect chainSelect = halPhyGetChainSelect(pMac, (tANI_U8)pMac->hal.cfgTxAntenna, (tANI_U8)pMac->hal.cfgRxAntenna);

                        if (chainSelect != INVALID_PHY_CHAIN_SEL)
                        {
                            if (halPhySetChainSelect(pMac, chainSelect) != eHAL_STATUS_SUCCESS){
                                HALLOGE( halLog(pMac, LOGE, FL("halPhySetChainSelect() failed \n")));
                            }
                        }
                        else
                        {
                            HALLOGE( halLog(pMac, LOGE, FL("WNI_CFG_CURRENT_RX_ANTENNA: Incorrect chain selection")));
                        }
                    }
                    break;

                case WNI_CFG_LOW_GAIN_OVERRIDE:
                    if ( (status = halPhyUpdateTxGainOverride(pMac, (val == 0) ? OPEN_LOOP_TX_HIGH_GAIN_OVERRIDE :
                                                                OPEN_LOOP_TX_LOW_GAIN_OVERRIDE)) != eHAL_STATUS_SUCCESS){
                        HALLOGE( halLog(pMac, LOGE, FL("halPhyUpdateTxGainOverride() failed \n")));
                    }
                    break;

                case WNI_CFG_POWER_STATE_PER_CHAIN:
                    halSetChainPowerState(pMac);
                    break;

               case WNI_CFG_CAL_PERIOD:
                       rc = halConfigCalPeriod(pMac);
                    break;

                case WNI_CFG_CAL_CONTROL:
                       rc = halConfigCalControl(pMac);
                    break;

                case WNI_CFG_STATS_PERIOD:
                    tx_timer_deactivate(&pMac->hal.halMac.wrapStats.statTimer);
                    pMac->hal.halMac.wrapStats.statTmrVal = SYS_SEC_TO_TICKS(val);
                    tx_timer_change(&pMac->hal.halMac.wrapStats.statTimer,
                                    pMac->hal.halMac.wrapStats.statTmrVal,
                                    pMac->hal.halMac.wrapStats.statTmrVal);
                    tx_timer_activate(&pMac->hal.halMac.wrapStats.statTimer);

#ifdef FIXME_GEN5
                    tx_timer_deactivate(&pMac->hal.halMac.macStats.statTimer);
                    pMac->hal.halMac.macStats.statTmrVal = SYS_SEC_TO_TICKS(val);
                    HALLOGW( halLog(pMac, LOGW, FL("WNI_CFG_STATS_PERIOD %d seconds\n"),
                           val));
                    tx_timer_change(&pMac->hal.halMac.macStats.statTimer,
                                    pMac->hal.halMac.macStats.statTmrVal,
                                    pMac->hal.halMac.macStats.statTmrVal);
                    tx_timer_activate(&pMac->hal.halMac.macStats.statTimer);
#endif
                    break;

                case WNI_CFG_CFP_MAX_DURATION:
                    break;

                // ----------------------------
                // Rate Adaptation related CFG
                // ----------------------------
                case WNI_CFG_DYNAMIC_THRESHOLD_ZERO:
                    HALLOG4( halLog(pMac, LOG4, FL("setDynamicThresh0: %d\n"),  val ));
                    break;

                case WNI_CFG_DYNAMIC_THRESHOLD_ONE:
                    HALLOG4( halLog( pMac, LOG4, FL("setDynamicThresh1: %d\n"),  val ));
                    break;

                case WNI_CFG_PROXIMITY:
                case WNI_CFG_NETWORK_DENSITY:
                    halPhy_setNwDensityAndProximity(pMac);
                    halATH_setAlgorithm(pMac);
                    break;

                case WNI_CFG_ADAPTIVE_THRESHOLD_ALGORITHM:
                    halATH_setAlgorithm(pMac);
                    break;

                case WNI_CFG_FIXED_RATE:
                case WNI_CFG_RETRYRATE_POLICY:
                case WNI_CFG_FORCE_POLICY_PROTECTION:
                    halMacRaCfgChange(pMac, pMsg->bodyval);
                    break;

                case WNI_CFG_BA_TIMEOUT:
                case WNI_CFG_MAX_BA_BUFFERS:
                case WNI_CFG_MAX_BA_SESSIONS:
                case WNI_CFG_BA_THRESHOLD_HIGH:
                case WNI_CFG_BA_ACTIVITY_CHECK_TIMEOUT:
                case WNI_CFG_BA_AUTO_SETUP:
                case WNI_CFG_MAX_MEDIUM_TIME:
                case WNI_CFG_MAX_MPDUS_IN_AMPDU:
                    baHandleCFG( pMac, pMsg->bodyval );
                    break;

                case WNI_CFG_FIXED_RATE_MULTICAST_24GHZ:
                case WNI_CFG_FIXED_RATE_MULTICAST_5GHZ:
                    halProcessMulticastRateChange(pMac, pMsg->bodyval);
                    break;
/* In firmware RA, this is not supported.
        case WNI_CFG_RA_PERIODICITY_TIMEOUT_IN_PS:
                     halRAHandleCfg( pMac, pMsg->bodyval);
                     break;
*/
                case WNI_CFG_PS_DATA_INACTIVITY_TIMEOUT:
            halPSDataInActivityTimeout(pMac, pMsg->bodyval);
            break;

        case WNI_CFG_PS_ENABLE_HEART_BEAT:
            halPSFWHeartBeatCfg(pMac, pMsg->bodyval);
            break;

        case WNI_CFG_PS_ENABLE_BCN_FILTER:
            halPSBcnFilterCfg(pMac, pMsg->bodyval);
            break;

        case WNI_CFG_PS_ENABLE_RSSI_MONITOR:
            halPSRssiMonitorCfg(pMac, pMsg->bodyval);
            break;

                case WNI_CFG_MCAST_BCAST_FILTER_SETTING:
                   halHandleMcastBcastFilterSetting(pMac, pMsg->bodyval);
                   break;

                default:
                    HALLOGE( halLog(pMac, LOGE, FL("Cfg Id %d is not handled\n"), pMsg->bodyval));
                    break;
            }

            break;

        case SIR_CFG_DOWNLOAD_COMPLETE_IND:
            {
                rc = halProcessCfgDownloadComplete(pMac);
            }
            break;

        /*
         * Taurus related messages
         */
        case SIR_HAL_ADD_STA_REQ:
            halMsg_AddSta(pMac, pMsg->reserved, (tpAddStaParams) (pMsg->bodyptr), eANI_BOOLEAN_TRUE);
            break;

        case SIR_HAL_DELETE_STA_REQ:
            halMsg_DelSta(pMac, pMsg->reserved, (tpDeleteStaParams) (pMsg->bodyptr));
            break;

        case SIR_HAL_UPDATE_STARATEINFO_REQ:
            halMsg_UpdateTxCmdTemplate(pMac, pMsg->reserved, (tpUpdateTxCmdTemplParams) (pMsg->bodyptr));
            break;

        case SIR_HAL_ADD_BSS_REQ:
            halMsg_AddBss(pMac, pMsg->reserved, (tpAddBssParams) (pMsg->bodyptr));
            break;

        case SIR_HAL_DELETE_BSS_REQ:
            halMsg_DelBss(pMac, pMsg->reserved, (tpDeleteBssParams) (pMsg->bodyptr));
            break;

        case SIR_HAL_INIT_SCAN_REQ:
            halMsg_InitScan(pMac, pMsg->reserved, (tpInitScanParams)(pMsg->bodyptr));
            break;

        case SIR_HAL_START_SCAN_REQ:
            halMsg_StartScan(pMac, pMsg->reserved, (tpStartScanParams)(pMsg->bodyptr));
            break;

        case SIR_HAL_END_SCAN_REQ:
            halMsg_EndScan(pMac, pMsg->reserved, (tpEndScanParams)(pMsg->bodyptr));
            break;

        case SIR_HAL_FINISH_SCAN_REQ:
            halMsg_FinishScan(pMac, pMsg->reserved, (tpFinishScanParams)(pMsg->bodyptr));
            break;

        case SIR_HAL_SET_LINK_STATE:
            halMsg_setLinkState(pMac, (tpLinkStateParams)(pMsg->bodyptr));
            break;

        case SIR_HAL_SEND_BEACON_REQ:
            halMsg_SendBeacon(pMac, (tpSendbeaconParams)(pMsg->bodyptr));
            break;

        case SIR_HAL_INIT_CFG_REQ:
            break;

        case SIR_HAL_INIT_WM_CFG_REQ:
            halInitWmParam((tHalHandle)pMac, NULL);
            break;

        case SIR_HAL_SET_BSSKEY_REQ:
            halMsg_SetBssKey(pMac, pMsg->reserved, (tpSetBssKeyParams) (pMsg->bodyptr));
            break;

        case SIR_HAL_SET_STAKEY_REQ:
            halMsg_SetStaKey(pMac, pMsg->reserved, (tpSetStaKeyParams) (pMsg->bodyptr));
            break;

       case SIR_HAL_SET_STA_BCASTKEY_REQ:
                halMsg_SetStaBcastKey(pMac, pMsg->reserved, (tpSetStaKeyParams) (pMsg->bodyptr));
                break;

        case SIR_HAL_REMOVE_BSSKEY_REQ:
            halMsg_RemoveBssKey(pMac, pMsg->reserved, (tpRemoveBssKeyParams) (pMsg->bodyptr));
            break;

        case SIR_HAL_REMOVE_STAKEY_REQ:
            halMsg_RemoveStaKey(pMac, pMsg->reserved, (tpRemoveStaKeyParams) (pMsg->bodyptr));
            break;

        case SIR_HAL_DPU_STATS_REQ:
            halMsg_GetDpuStats(pMac, pMsg->reserved, (tpDpuStatsParams) (pMsg->bodyptr));
            break;

        case SIR_HAL_GET_DPUINFO_REQ:
            halMsg_GetDpuParams(pMac, pMsg->reserved, (tpGetDpuParams) (pMsg->bodyptr));
            break;

        case SIR_HAL_UPDATE_EDCA_PROFILE_IND:
            halMsg_updateEdcaParam(pMac, (tEdcaParams *) (pMsg->bodyptr));
            break;

        case SIR_HAL_UPDATE_BEACON_IND :
            halMsg_updateBeaconParam(pMac, (tpUpdateBeaconParams) (pMsg->bodyptr));
            break;

        case SIR_HAL_UPDATE_CF_IND:
            vos_mem_free((v_VOID_t*)pMsg->bodyptr);
            pMsg->bodyptr = NULL;

            break;

        case SIR_HAL_CHNL_SWITCH_REQ:
            halMsg_ChannelSwitch(pMac, (tpSwitchChannelParams)(pMsg->bodyptr));
            //NO Response is needed for these messages, as these are just indications.
            //HAL needs to free the memory, after having handled these messages.
            //palFreeMemory( pMac->hHdd, (tANI_U8 *) pMsg->bodyptr );
            break;

        case SIR_HAL_SET_TX_POWER_REQ:
            HALLOGW( halLog(pMac, LOGW, FL("Got Set Tx Power Request \n")));
            halMsg_setTxPower(pMac, (tpSirSetTxPowerReq)pMsg->bodyptr);
            break;

        case SIR_HAL_GET_TX_POWER_REQ:
            HALLOGW( halLog(pMac, LOGW, FL("Got Get Tx Power Request \n")));
            halMsg_getTxPower(pMac, (tpSirGetTxPowerReq)pMsg->bodyptr);
            break;

        case SIR_HAL_SET_KEY_DONE:
            HALLOGW( halLog(pMac, LOGW, FL("Set Key Done \n")));
            halMsg_SetKeyDone(pMac);
            break;


       case SIR_HAL_HANDLE_FW_MBOX_RSP:
            HALLOGE( halLog(pMac, LOGE, FL("Fw Rsp Msg \n")));
            halFW_HandleFwMessages(pMac, pMsg->bodyptr);
            vos_mem_free((v_VOID_t*)pMsg->bodyptr);
            pMsg->bodyptr = NULL;
            break;

       case SIR_HAL_SEND_MSG_COMPLETE:
            HALLOGE( halLog(pMac, LOGE, FL("Fw Rsp Msg \n")));
            halMbox_SendMsgComplete(pMac);
            break; 

       case SIR_HAL_GET_NOISE_REQ:
            HALLOGW( halLog(pMac, LOGW, FL("Got Get Noise Request \n")));
            halMsg_sendGetNoiseRsp(pMac);
            break;

// Start of Power Save related messages
        case SIR_HAL_PWR_SAVE_CFG:
            status = halPS_Config(pMac, (tpSirPowerSaveCfg)pMsg->bodyptr);
            break;

        case SIR_HAL_ENTER_IMPS_REQ:
            status = halPS_HandleEnterImpsReq(pMac, dialogToken);
            break;

        case SIR_HAL_EXIT_IMPS_REQ:
            status = halPS_HandleExitImpsReq(pMac, dialogToken);
            break;

        case SIR_HAL_POSTPONE_ENTER_IMPS_RSP:
            status = halPS_HandleFwEnterImpsRsp(pMac, pMsg->bodyptr);
            break;

        case SIR_HAL_ENTER_BMPS_REQ:
            status = halPS_HandleEnterBmpsReq(pMac, dialogToken, (tpEnterBmpsParams)pMsg->bodyptr);
        break;

        case SIR_HAL_EXIT_BMPS_REQ:
            status = halPS_HandleExitBmpsReq(pMac, dialogToken, (tpExitBmpsParams)pMsg->bodyptr);
            break;

        case SIR_HAL_SUSPEND_BMPS:
            status = halPS_SuspendBmps(pMac, dialogToken, NULL, NULL);
            break;

        case SIR_HAL_RESUME_BMPS:
            status = halPS_ResumeBmps(pMac, dialogToken, NULL, NULL, FALSE);
            break;

        case SIR_HAL_ENTER_UAPSD_REQ:
            status = halPS_HandleEnterUapsdReq(pMac, dialogToken,
                    (tpUapsdParams)pMsg->bodyptr);
            break;

        case SIR_HAL_EXIT_UAPSD_REQ:
            status = halPS_HandleExitUapsdReq(pMac, dialogToken);
            break;

        case SIR_HAL_BEACON_FILTER_IND:
            status = halPS_HandleAddBeaconFilter(pMac, dialogToken,
                    (void*)pMsg->bodyptr);
            break;

        case SIR_HAL_WOWL_ADD_BCAST_PTRN:
            status = halPS_AddWowlPatternToFw(pMac, (tpSirWowlAddBcastPtrn)(pMsg->bodyptr));
            break;

        case SIR_HAL_WOWL_DEL_BCAST_PTRN:
            status = halPS_RemoveWowlPatternAtFw(pMac, (tpSirWowlDelBcastPtrn)(pMsg->bodyptr));
            break;

        case SIR_HAL_WOWL_ENTER_REQ:
            status = halPS_EnterWowlReq(pMac, dialogToken, (tpSirHalWowlEnterParams)(pMsg->bodyptr));
            break;

        case SIR_HAL_WOWL_EXIT_REQ:
            status = halPS_ExitWowlReq(pMac, dialogToken);
            break;

// End of Power Save releated messages

        case SIR_HAL_ADD_TS_REQ:
            halMsg_AddTs(pMac, pMsg->reserved, (tpAddTsParams) (pMsg->bodyptr));
            break;

        case SIR_HAL_DEL_TS_REQ:
            halMsg_DelTs(pMac, pMsg->reserved, (tpDelTsParams) (pMsg->bodyptr));
            break;

        case SIR_HAL_REGISTER_PE_CALLBACK:
            if (pMsg->bodyptr)
            {
                halMsg_RegisterPECallback(pMac, pMsg->bodyptr);
            }
            break;

        case SIR_HAL_ADDBA_REQ:
            halMsg_AddBA(pMac, pMsg->reserved, (tpAddBAParams) (pMsg->bodyptr));
            break;

        case SIR_HAL_DELBA_IND:
            halMsg_DelBA(pMac, pMsg->reserved, (tpDelBAParams) (pMsg->bodyptr));
            break;

        case SIR_HAL_BA_FAIL_IND:
            halMsg_BAFail(pMac, pMsg->reserved, (tpAddBARsp) (pMsg->bodyptr));
            break;

#ifdef ANI_SUPPORT_SMPS
        case SIR_HAL_SET_MIMOPS_REQ:
            halMsg_SetMimoPs(pMac, (tpSetMIMOPS)(pMsg->bodyptr));
            break;
#endif

        case SIR_HAL_BEACON_PRE_IND:
            halMsg_BeaconPre(pMac);
            break;

        case SIR_HAL_STA_STAT_REQ:
        case SIR_HAL_AGGR_STAT_REQ:
        case SIR_HAL_GLOBAL_STAT_REQ:
        case SIR_HAL_STAT_SUMM_REQ:
            halHandleStatsReq(pMac, pMsg->type, (tpAniGetStatsReq) (pMsg->bodyptr));
            break;

        case SIR_HAL_GET_STATISTICS_REQ:
            halHandlePEStatisticsReq(pMac, pMsg->type, (tpAniGetPEStatsReq) (pMsg->bodyptr));
            break;

        /** ---- This timer messages posted by MTU Timer. ---- */
        case SIR_HAL_TIMER_ADJUST_ADAPTIVE_THRESHOLD_IND:
            halATH_adjustAdaptiveThreshold(pMac);
            break;

        /** ---- These are timer messages posted by HAL Timer. ---- */
        case SIR_HAL_TIMER_TEMP_MEAS_REQ:
            if ((pMac->hphy.phy.phyPeriodicCalEnable)
                    && (! pMac->hphy.phy.test.testDisableSpiAccess))
            {
                halPerformTempMeasurement(pMac);
            }
            break;

        case SIR_HAL_TIMER_BA_ACTIVITY_REQ:
            halBaCheckActivity(pMac);
            break;

        case SIR_HAL_TIMER_PERIODIC_STATS_COLLECT_REQ:
            halMacPeriodicStatCollection(pMac);
            break;

        case SIR_HAL_TIMER_WRAP_AROUND_STATS_COLLECT_REQ:
            //halMacWrapAroundStatCollection(pMac);
            /**
                    * Periodic stat Timer does not work.
                    * So deactivating it and acitvating again
                    */
                    tx_timer_deactivate(&pMac->hal.halMac.wrapStats.statTimer);
            //pMac->hal.halMac.wrapStats.statTmrVal = SYS_SEC_TO_TICKS(val);
            //halLog(pMac, LOGW, FL("WNI_CFG_STATS_PERIOD %d seconds\n"),
            //       val);
            //tx_timer_change(&pMac->hal.halMac.wrapStats.statTimer,
            //                pMac->hal.halMac.wrapStats.statTmrVal,
            //                pMac->hal.halMac.wrapStats.statTmrVal);
            tx_timer_activate(&pMac->hal.halMac.wrapStats.statTimer);
            break;

        case SIR_HAL_TIMER_CHIP_MONITOR_TIMEOUT:
            halMsg_ChipMonitorTimeout(pMac);
            /**
            * Periodic chip monitor does not work.
            * So deactivating it and acitvating again
            */
            tx_timer_deactivate(&pMac->hal.halMac.chipMonitorTimer);
            tx_timer_activate(&pMac->hal.halMac.chipMonitorTimer);
            break;
        case SIR_HAL_TIMER_TRAFFIC_ACTIVITY_REQ:
            halMsg_HandleTrafficActivity(pMac);
            break;
        /**  ---------  End of HAL Timer Messages.  ---------- */

        case SIR_HAL_DPU_MIC_ERROR:
            halDpu_MICErrorIndication(pMac);
            break;

        case SIR_HAL_TRANSMISSION_CONTROL_IND:
            halMsg_FrameTransmitControlInd(pMac, (void *) pMsg->bodyptr);
            break;

        case SIR_HAL_TX_COMPLETE_IND:
            halMsg_TXCompleteInd(pMac, pMsg->bodyval);
            break;
        case SIR_HAL_BTC_SET_CFG:
            halBtc_SetBtcCfg(pMac, (void *)pMsg->bodyptr);
            break;

        case SIR_HAL_SIGNAL_BT_EVENT:
            halBtc_SendBtEventToFW(pMac, (void *)pMsg->bodyptr);
            break;


#ifdef ANI_SUPPORT_5GHZ
        case SIR_HAL_INIT_RADAR_IND:
            halRadar_Init(pMac);
            break;
#endif
        default:
            HALLOGW( halLog(pMac, LOGW, FL("Errored Type 0x%X\n"), pMsg->type));
            vos_mem_free((v_VOID_t*)pMsg->bodyptr);
            pMsg->bodyptr = NULL;
            break;
    }

    if (status != eHAL_STATUS_SUCCESS) {
        rc = eSIR_FAILURE;
    }

    HALLOG4( halLog(pMac, LOG4, FL("Success Returns!\n")));
    return rc;
} // halHandleMsg()

// --------------------------------------------------------
/**
 * halCleanup
 *
 * FUNCTION:
 *     Cleans up HAL state and timers.
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 *
 * NOTE:
 *
 * @param pMac MAC parameter structure pointer
 * @return NONE
 */

void
halCleanup(tpAniSirGlobal pMac)
{
    halTimersDestroy(pMac);
    // Start up in eSYSTEM_STA_ROLE
    // This should get updated appropriately at a later stage
    halSetSystemRole( pMac, eSYSTEM_STA_ROLE );
    pMac->hal.currentChannel = 0;
    pMac->hal.currentRfBand = eRF_BAND_UNKNOWN;
    pMac->hal.currentCBState = PHY_SINGLE_CHANNEL_CENTERED;
    //resetting the halDeferMsgQ.
    //There will be memory leak here if there are messages in the queue.
    pMac->hal.halDeferMsgQ.size = 0;
    pMac->hal.halDeferMsgQ.read = 0;
    pMac->hal.halDeferMsgQ.write= 0;

    halStateSet(pMac, eHAL_IDLE);
    //halMac Wmm related initializations
    // Initializing TSPEC info table array
    palZeroMemory(pMac->hHdd, pMac->hal.halMac.tspecInfo, sizeof(tTspecTblEntry) * LIM_NUM_TSPEC_MAX);
    pMac->hal.halMac.tsActivityChkTmrStarted = 0;
    pMac->hal.halMac.frameClassifierEnabled = 0;

    //halMac BA related initializations.
    pMac->hal.halMac.baRxMaxAvailBuffers = 0;
    pMac->hal.halMac.baNumActiveSessions = 0;
    pMac->hal.halMac.baTimeout = 0;
    pMac->hal.halMac.baSetupThresholdHigh = 0;
    palZeroMemory(pMac->hHdd, pMac->hal.halMac.baSessionTable, sizeof(tRxBASessionTable) * BA_MAX_SESSIONS);
    pMac->hal.halMac.baAutoSetupEnabled = false;

    //adaptive threshold structure initialization.
    palZeroMemory(pMac->hHdd, &pMac->hal.halAdaptThresh, sizeof(tAniHalAdaptThresh));
}


/*
 * TAURUS related code.
 */
/*
 * halMsg_GenerateRsp - Generate response to PE for messages.
 */
void halMsg_GenerateRsp( tpAniSirGlobal pMac, tANI_U16 msgType, tANI_U16 dialog_token, void *pBodyptr, tANI_U32 bodyVal)
{
    tSirMsgQ msg;
    tANI_U32 status;

    msg.type        = msgType;
    msg.reserved    = dialog_token;
    msg.bodyval     = bodyVal;
    msg.bodyptr     = pBodyptr;

    status = limPostMsgApi(pMac, &msg);

    if (status != TX_SUCCESS)
        HALLOGP( halLog(pMac, LOGP, FL("Failed limPostMsgApi=0%X\n"), status));
    return;
}



/** -------------------------------------------------------------
\fn halGetDefaultAndMulticastRates
\brief reads config for defualt and multicast rates. Converts user config rate for multicast rate to softmac rate index.
\param   tpAniSirGlobal pMac
\param   eRfBandMode rfBand.
\param   tTpeRateIdx* pRateIndex
\param   tTpeRateIdx* pMcastRateIndex
\return eHalStatus - status
  -------------------------------------------------------------*/
eHalStatus halGetDefaultAndMulticastRates(tpAniSirGlobal pMac, eRfBandMode rfBand,
        tTpeRateIdx* pRateIndex, tTpeRateIdx* pMcastRateIndex)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;

#ifdef LIBRA_FPGA
    //For LIBRA FPGA 11B rates are not working. We need to set 6M.
    *pRateIndex = TPE_RT_IDX_11A_6_MBPS;
    *pMcastRateIndex = TPE_RT_IDX_11A_6_MBPS;
#else

    tANI_U16 cfgMcastRateKey, cfgDefRateKey;
    tANI_U32 cfgRateIndex, rateIndex = TPE_RT_IDX_INVALID, defRate ;

        if (rfBand == eRF_BAND_2_4_GHZ ){
        cfgMcastRateKey = WNI_CFG_FIXED_RATE_MULTICAST_24GHZ;
        cfgDefRateKey = WNI_CFG_DEFAULT_RATE_INDEX_24GHZ;
        defRate = TPE_RT_IDX_11B_LONG_1_MBPS;
        }else{
        cfgMcastRateKey = WNI_CFG_FIXED_RATE_MULTICAST_5GHZ;
        cfgDefRateKey = WNI_CFG_DEFAULT_RATE_INDEX_5GHZ;
        defRate = TPE_RT_IDX_11A_6_MBPS;
        }

    // Read the multicast/broadcast rate from the CFG
    if (wlan_cfgGetInt(pMac, cfgMcastRateKey, &cfgRateIndex) == eSIR_SUCCESS) {
        rateIndex = halRate_cfgFixedRate2TpeRate(cfgRateIndex);
        if (rateIndex != TPE_RT_IDX_INVALID) {
            *pMcastRateIndex = (tTpeRateIdx)rateIndex;
        } else {
            HALLOGE(halLog(pMac, LOGE, FL("Default Mcast rate CFG is invalid rate %d !! Override to index %d. \n"), cfgRateIndex, defRate));
            *pMcastRateIndex = (tTpeRateIdx)defRate;
        }
    }
    else {
            HALLOGP( halLog(pMac, LOGP, FL("Get WNI_CFG_FIXED_RATE_MULTICAST_24GHZ failed \n")));
            return eHAL_STATUS_FAILURE;
    }

    // Read the default rate from the CFG
    if (wlan_cfgGetInt(pMac, cfgDefRateKey, &cfgRateIndex) == eSIR_SUCCESS) {
        rateIndex = halRate_cfgFixedRate2TpeRate(cfgRateIndex);
        // If from CFG the rate index given is an invalid one, override it
        if (rateIndex != TPE_RT_IDX_INVALID) {
            *pRateIndex = (tTpeRateIdx)rateIndex;
        } else {
            HALLOGE( halLog(pMac, LOGE, FL("Default rate CFG is invalid rate %d !! Override to index %d. \n"), cfgRateIndex, defRate));
            *pRateIndex = (tTpeRateIdx)defRate;
        }
    }
    else {
            return eHAL_STATUS_FAILURE;
    }
#endif //LIBRA_FPGA
    return status;
}


eHalStatus halSetNewChannelParams(tpAniSirGlobal pMac)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    eRfBandMode oldRfBand;
    tMtuMode    mtuMode;

    oldRfBand = halUtil_GetRfBand(pMac, pMac->hal.currentChannel);

    if (pMac->hphy.setChanCntx.newRfBand != oldRfBand) {
        tTpeRateIdx rateIndex, mcastRateIndex;
        if ((status = halGetDefaultAndMulticastRates(pMac,
                        pMac->hphy.setChanCntx.newRfBand, &rateIndex,
                        &mcastRateIndex)) != eHAL_STATUS_SUCCESS) {
            HALLOGE( halLog(pMac, LOGE, FL("halGetDefaultAndMulticastRates failed with status code: %d\n"), status));
            return status;
        }

        halSetBcnRateIdx(pMac, rateIndex);
        halSetNonBcnRateIdx(pMac, rateIndex);
        halSetMulticastRateIdx(pMac, mcastRateIndex);
    }

    // Set the channel number to be carried in the RxBd
    halRxp_setChannel(pMac, pMac->hphy.setChanCntx.newChannel);

    // Configure Rxp when doing channel switch
    halRxp_setOperatingRfBand(pMac, pMac->hphy.setChanCntx.newRfBand);

    // Caching the new and band information
    pMac->hal.currentChannel = pMac->hphy.setChanCntx.newChannel;
    pMac->hal.currentRfBand  = pMac->hphy.setChanCntx.newRfBand;
    pMac->hal.currentCBState = pMac->hphy.setChanCntx.newCbState;

    // Update MTU timing parameters when channel is changed and only
    // when MTU mode needs to be changed.
    mtuMode = halMTU_getMode(pMac);
    if(pMac->hal.halMac.lastMtuMode != mtuMode) {
        halMTU_updateTimingParams(pMac, mtuMode);
    }

    return status;
}

/* -------------------------------------------------------
 * FUNCTION:  halPhy_ChangeChannel()
 *
 * NOTE:
 *  1) If RF band changes or invoked from halMsg_AddBss()
 *     then we need to update the default beaconRateIndex
 *     and nonBeaconRateIndex in the hal Global
 *  2) Cache this new channel and RF band info
 *  3) If we're using RF link, then:
  *      - set the new channel via halPhySetChannel()
 *       - update rateTable with new power value
 *       - update Calibration
 * -------------------------------------------------------
 */
eHalStatus halPhy_ChangeChannel(tpAniSirGlobal pMac,
        tANI_U8 newChannel, ePhyChanBondState newCbState,
        tANI_U8 calRequired, funcHalSetChanCB pFunc,
        void* pData, tANI_U16 dialog_token)
{
    eHalStatus status;

    // Cache the channel information and the context to return
    pMac->hphy.setChanCntx.newChannel   = newChannel;
    pMac->hphy.setChanCntx.newRfBand    = halUtil_GetRfBand(pMac, newChannel);
    pMac->hphy.setChanCntx.newCbState   = newCbState;
    pMac->hphy.setChanCntx.pFunc        = pFunc;
    pMac->hphy.setChanCntx.pData        = pData;
    pMac->hphy.setChanCntx.dialog_token = dialog_token;

    // Check if the current channel is same as the channel requested
    if( (newChannel == pMac->hal.currentChannel) &&
        (newCbState == pMac->hal.currentCBState)) {
        HALLOG1( halLog(pMac, LOG1,
                    FL("Channel(%d) and CB State(%d) are the same as the previous settings, so not changing.\n"),
                                              newChannel, newCbState));
        return eHAL_STATUS_SET_CHAN_ALREADY_ON_REQUESTED_CHAN;
    }

    HALLOG1( halLog(pMac, LOG1, FL("(chId %d, band %d, cbState %d) --> (chId %d, band %d cbState %d) "),
           pMac->hal.currentChannel, pMac->hal.currentRfBand, pMac->hal.currentCBState,
                newChannel, pMac->hphy.setChanCntx.newRfBand, newCbState));

#ifdef ANI_PRODUCT_TYPE_AP
    halRadar_SetInterrupt((tHalHandle) pMac, eANI_BOOLEAN_FALSE);
#endif

    // Do not perform channel change if its FPGA/ANALOG link
#ifdef LIBRA_RF
    halPhyDisableAllPackets(pMac);
    if (newCbState == PHY_SINGLE_CHANNEL_CENTERED) {
        halPhySetAgcCCAMode(pMac, PHY_CCA_ED_OR_CD_AND_CS, PHY_CCA_FORCED_ON);
    } else {
        halPhySetAgcCCAMode(pMac, PHY_CCA_ED_OR_CD_AND_CS, PHY_CCA_SEC_ED40_AND_NOR_PKTDET40_PKTDET20);
    }

    HALLOGW( halLog(pMac, LOGW, FL("halPhySetChannel(channel %d, cbState %d) \n"), newChannel, newCbState));

    status = halPhySetChannel(pMac, newChannel, newCbState, calRequired);

#else // LIBRA_RF
    status = halSetNewChannelParams(pMac);
#endif // LIBRA_RF

    return status;
}

// Function to handle the set channel response from FW
void halPhy_HandleSetChannelRsp(tHalHandle hHal,  void* pFwMsg)
{
    eHalStatus  status = eHAL_STATUS_SUCCESS;
#ifdef LIBRA_RF
    tANI_U8 bottomGainDb;
    tpAniSirGlobal pMac = (tpAniSirGlobal) hHal;
    Qwlanfw_SetChannelRspType *setChanRsp = (Qwlanfw_SetChannelRspType *)pFwMsg;
    tpPhySetChanCntx pSetChanCtx = &pMac->hphy.setChanCntx;

    // Check the FW response status
    if(setChanRsp->uStatus == eHAL_STATUS_SUCCESS) {
        //convert it to phy index before saving it
        pMac->hphy.rf.curChannel = rfGetChannelIndex(pMac->hphy.setChanCntx.newChannel, pMac->hphy.setChanCntx.newCbState);

        // Set few halMac parameters for the new channel
        (void) halSetNewChannelParams(pMac);

    } else {
        HALLOGE( halLog(pMac, LOGE, FL("halPhySetChannel failed with status code: %d\n"), status));
        status = eHAL_STATUS_FAILURE;

        if (pMac->hal.currentCBState == PHY_SINGLE_CHANNEL_CENTERED) {
            halPhySetAgcCCAMode(pMac, PHY_CCA_ED_OR_CD_AND_CS, PHY_CCA_FORCED_ON);
        } else {
            halPhySetAgcCCAMode(pMac, PHY_CCA_ED_OR_CD_AND_CS, PHY_CCA_SEC_ED40_AND_NOR_PKTDET40_PKTDET20);
        }
    }

    //Enable the packet reception.
    (void)halPhySetRxPktsDisabled(pMac, pMac->hphy.modTypes);
    halPhyGetRxGainRange(pMac, &pMac->hal.halMac.maxGainIndex, &pMac->hal.halMac.topGainDb, &bottomGainDb);

    // Resume to the context of the caller
    pSetChanCtx->pFunc(pMac, pSetChanCtx->pData, status, pSetChanCtx->dialog_token);

    return;
#endif //LIBRA_RF
}

void halForceSetNwType(tpAniSirGlobal pMac, tSirNwType forceNwType)
{
    if(forceNwType <= eSIR_11N_NW_TYPE){
        pMac->hal.nwType = forceNwType;
        HALLOGP( halLog( pMac, LOGE,
            FL("Forced set nwType to %d\n"),
            forceNwType ));

    }
}
/**
    @brief    : This function sends a dummy Init Scan request to the softmac followed by DATA_NULL or CTS2SELF
    @param    : pMac-The Global Structure for MAC
              setPMbit-If this is set, DATA_NULL packet is sent out with init scan request

    @return    : eHAL_STATUS_SUCCESS or eHAL_STATUS_FAILURE based on message sent correctly or not

*/

eHalStatus hal_SendDummyInitScan(tpAniSirGlobal pMac, tANI_BOOLEAN setPMbit)
{
    tpStaStruct pSta = (tpStaStruct) pMac->hal.halMac.staTable;
    tInitScanParams initParam, *initScanParam;
    tANI_U32 waitForTxComp = 0;

    initScanParam = &initParam;

    if (pMac->hal.scanParam.linkState == eSIR_LINK_SCAN_STATE ||
                        pMac->hal.scanParam.linkState == eSIR_LINK_LEARN_STATE)
        return eHAL_STATUS_FAILURE;

    palZeroMemory(pMac->hHdd, (void *)&initParam, sizeof(initParam));
    palCopyMemory(pMac->hHdd, (void *)&initScanParam->bssid, (void *)pSta[0].bssId, 6);


    if (pMac->hal.halSystemRole == eSYSTEM_STA_ROLE)
    {
        initScanParam->scanMode = eHAL_SYS_MODE_SCAN;
        initScanParam->notifyBss = TRUE;
        initScanParam->notifyHost = FALSE;
        initScanParam->frameType = SIR_MAC_DATA_NULL;
        initScanParam->scanDuration = 30;
        initScanParam->frameLength = 0;

        if (setPMbit == 1)
        {
            HALLOG1( halLog(pMac, LOG1, FL("Appending DATA_NULL \n")));
            CreateInitScanRawFrame(pMac, &initScanParam->macMgmtHdr, eSYSTEM_STA_ROLE);
            initScanParam->frameLength = sizeof(tSirMacMgmtHdr);
        }  else
            HALLOG1( halLog(pMac, LOG1, FL("No frames appended \n")));
    }
    else if (pMac->hal.halSystemRole == eSYSTEM_AP_ROLE || pMac->hal.halSystemRole == eSYSTEM_STA_IN_IBSS_ROLE)
    {
        initScanParam->scanMode = eHAL_SYS_MODE_LEARN;
        initScanParam->notifyBss = TRUE;
        initScanParam->notifyHost = FALSE;
        initScanParam->frameType = SIR_MAC_CTRL_FRAME;
        initScanParam->scanDuration = 30;
        initScanParam->frameLength = 0;

        HALLOG1( halLog(pMac, LOG1, FL("Appending CTS Frame \n")));
        initScanParam->frameType = SIR_MAC_CTRL_CTS;
        CreateInitScanRawFrame(pMac, &initScanParam->macMgmtHdr, eSYSTEM_AP_ROLE);
        initScanParam->frameLength = sizeof(tSirMacMgmtHdr);
    }

    pMac->hal.scanParam.linkState = eSIR_LINK_INIT_CAL_STATE;;

    return halMsg_HandleInitScan(pMac, initScanParam, &waitForTxComp);
}

/**
    @brief    : This function sends a dummy Finish Scan request to the softmac followed by DATA_NULL
    @param    : pMac-The Global Structure for MAC
    @return    : eHAL_STATUS_SUCCESS or eHAL_STATUS_FAILURE based on message sent correctly or not


*/

void hal_SendDummyFinishScanPostSetChan(tpAniSirGlobal pMac, void* pData, tANI_U32 status, tANI_U16 dialog_token)
{
    tANI_U32 waitForTxComp = 0;
    tpFinishScanParams pFinishScanParam = (tpFinishScanParams)pData;

    pMac->hal.scanParam.linkState = eSIR_LINK_FINISH_CAL_STATE;

    halMsg_HandleFinishScan(pMac, pFinishScanParam, &waitForTxComp);

    return;
}

eHalStatus hal_SendDummyFinishScan(tpAniSirGlobal pMac)
{
    tpStaStruct pSta = (tpStaStruct) pMac->hal.halMac.staTable;
    eHalStatus    status = eHAL_STATUS_SUCCESS;
    tpFinishScanParams pFinishScanParam = NULL;
    tANI_U8    saved_channel = pMac->hal.currentChannel;

    palAllocateMemory(pMac->hHdd, (void*)pFinishScanParam, sizeof(tFinishScanParams));

    palZeroMemory(pMac->hHdd, (void *)pFinishScanParam, sizeof(tFinishScanParams));
    palCopyMemory(pMac->hHdd, (void *)&pFinishScanParam->bssid, (void *)pSta[0].bssId, 6);

    if (pMac->hal.halSystemRole == eSYSTEM_STA_ROLE)
    {
        pFinishScanParam->scanMode = eHAL_SYS_MODE_NORMAL;
        pFinishScanParam->notifyBss = 1;
        pFinishScanParam->notifyHost = 0;
        CreateFinishScanRawFrame(pMac, &pFinishScanParam->macMgmtHdr, eSYSTEM_STA_ROLE);
        pFinishScanParam->frameLength = sizeof(tSirMacMgmtHdr);
    }
    else if (pMac->hal.halSystemRole == eSYSTEM_AP_ROLE || pMac->hal.halSystemRole == eSYSTEM_STA_IN_IBSS_ROLE)
    {
        pFinishScanParam->scanMode = eHAL_SYS_MODE_NORMAL;
        pFinishScanParam->notifyBss = 0;
        pFinishScanParam->notifyHost = 0;
        pFinishScanParam->frameType = SIR_MAC_CTRL_CTS;
        pFinishScanParam->frameLength = 0;
    }

    pFinishScanParam->currentOperChannel = pMac->hal.currentChannel;
    pFinishScanParam->cbState = pMac->hphy.phy.chanBondState;

    HALLOG2( halLog( pMac, LOG2, FL("Current Channel = %d\n"),  pMac->hal.currentChannel ));

    status = halPhy_ChangeChannel(pMac, saved_channel, pFinishScanParam->cbState, TRUE, hal_SendDummyFinishScanPostSetChan, pFinishScanParam, 0);
    // If channel is already on the request channel, proceed further with
    // post set channel configuration
    if (status == eHAL_STATUS_SET_CHAN_ALREADY_ON_REQUESTED_CHAN) {
        hal_SendDummyFinishScanPostSetChan(pMac, (void*)pFinishScanParam, eHAL_STATUS_SUCCESS, 0);
    }

        return status;
}


/**
    @brief    : This function will read the configuration from Advanced properties and update the
              global variable for CAL period
    @param    : pMac-The Global Structure for MAC

    @return    : eSIR_SUCCESS or eSIR_FAILURE based on message sent correctly or not

*/

tSirRetStatus halConfigCalPeriod(tpAniSirGlobal pMac)
{
    tANI_U32    val;

    if (wlan_cfgGetInt(pMac, (tANI_U16) WNI_CFG_CAL_PERIOD, &val) != eSIR_SUCCESS)
    {
        HALLOGP( halLog(pMac, LOGP, FL("Failed to cfg get id %d\n"), WNI_CFG_CAL_PERIOD));
        return eSIR_FAILURE;
    }
#ifdef FIXME_GEN5
    tx_timer_deactivate(&pMac->hal.halMac.tempMeasTimer);

    if (val == 0)
    {
        pMac->hal.halMac.tempMeasTmrVal = SYS_SEC_TO_TICKS(HAL_TEMPMEAS_TIMER_VAL_SEC);
        HALLOGW( halLog(pMac, LOGW, FL("Invalid CAL Period Set, Setting to default period(5 min)\n")));
    }
    else
        pMac->hal.halMac.tempMeasTmrVal = SYS_MIN_TO_TICKS(val)/HAL_PHY_PERIODIC_CAL_ITER_LIMIT;

    //every 30 seconds to see if the RF synth is still locked
    HALLOG1( halLog(pMac, LOG1, FL("WNI_CFG_CAL_PERIOD %d minutes\n"), val));
    tx_timer_change(&pMac->hal.halMac.tempMeasTimer,
                    pMac->hal.halMac.tempMeasTmrVal,
                    pMac->hal.halMac.tempMeasTmrVal);
    tx_timer_activate(&pMac->hal.halMac.tempMeasTimer);
#endif
    pMac->hphy.calPeriodTicks = 0;

    return eSIR_SUCCESS;
}

/**
    @brief    : This function will read the configuration from Advanced properties and update the
              global variable for CAL Control
    @param    : pMac-The Global Structure for MAC

    @return    : eSIR_SUCCESS or eSIR_FAILURE

*/

tSirRetStatus halConfigCalControl(tpAniSirGlobal pMac)
{
    tANI_U32    val;

    if (wlan_cfgGetInt(pMac, (tANI_U16) WNI_CFG_CAL_CONTROL, &val) != eSIR_SUCCESS)
    {
        HALLOGP( halLog(pMac, LOGP, FL("Failed to cfg get id %d\n"), WNI_CFG_CAL_CONTROL));
        return eSIR_FAILURE;
    }
    else
    {
        if(val == WNI_CFG_CAL_CONTROL_CAL_ON)
            pMac->hphy.phy.phyPeriodicCalEnable= eANI_BOOLEAN_TRUE;
        else
            pMac->hphy.phy.phyPeriodicCalEnable = eANI_BOOLEAN_FALSE;
        HALLOG1( halLog(pMac, LOG1, FL("WNI_CFG_CAL_CONTROL %d (0-on/1-off)\n"), val));
    }

#ifdef ANI_BUS_TYPE_USB
    pMac->hphy.phy.phyPeriodicCalEnable = eANI_BOOLEAN_FALSE;
#endif

    return eSIR_SUCCESS;
}

/**
    @brief    : This function will perform the periodic calibration when calibration timer triggers.
              It assesses whether the CAL is needed or not and performs the CAL based on
              the current RXP mode
    @param    : pMac-The Global Structure for MAC

    @return    : eSIR_SUCCESS or eSIR_FAILURE based on message sent correctly or not

*/

tSirRetStatus halPerformTempMeasurement(tpAniSirGlobal pMac)
{
/*
    tANI_BOOLEAN    performCal = eANI_BOOLEAN_TRUE;

    if (!pMac->hphy.phy.phyPeriodicCalEnable)
    {
        pMac->hphy.phy.phyPeriodicCalNeeded = eANI_BOOLEAN_FALSE;
        return eSIR_FAILURE;
    }

    if (halUtil_CurrentlyInPowerSave(pMac))
    {
        pMac->hphy.phy.phyPeriodicCalNeeded = eANI_BOOLEAN_FALSE;
        HALLOGW( halLog(pMac, LOGW, FL("Station currently in Powersave Mode, Skipping periodic Cal...\n")));
        return eSIR_FAILURE;
    }

    if (!pMac->hphy.phy.phyPeriodicCalNeeded)
    {
        if (halPhyAssessCal(pMac, &performCal) != eHAL_STATUS_SUCCESS)
        {
            return eSIR_FAILURE;
        }
    }

    //assess calibration every 30 seconds
    if ((performCal == eANI_BOOLEAN_TRUE) && (pMac->hphy.phy.phyPeriodicCalNeeded == eANI_BOOLEAN_TRUE))
    {
        tRxpMode rxpMode;

        rxpMode = halRxp_getRxpMode(pMac);

        HALLOG2( halLog( pMac, LOG2, FL("Current RXP Mode = %d\n"),  rxpMode ));
        if ((rxpMode == eRXP_POST_ASSOC_MODE) || (rxpMode == eRXP_AP_MODE) || (rxpMode == eRXP_IBSS_MODE))
        {
            if (hal_SendDummyInitScan(pMac, 1) != eHAL_STATUS_SUCCESS)
            {
                HALLOGE( halLog(pMac, LOGE, FL("Send Init Scan failed\n")));
                return eSIR_FAILURE;
            }
        }
        else if ((rxpMode == eRXP_PROMISCUOUS_MODE) || (rxpMode == eRXP_IDLE_MODE))
        {
            halPhyCalUpdate(pMac);
        }
        else
        {
            HALLOGW( halLog(pMac, LOGW, FL("Skipping periodic Cal since RXP is in %d mode\n"), rxpMode));
            return eSIR_SUCCESS;
        }

        HALLOG4(halLog(pMac, LOG4, FL("Periodic Cal done")));
    }
*/
    return eSIR_SUCCESS;
}

tANI_BOOLEAN halIsRadioSwitchOn(tpAniSirGlobal pMac)
{

#if 0 //FIXME_NO_VIRGO

    tANI_U32    regValue;
    eHalStatus  status = eHAL_STATUS_SUCCESS;
    tANI_BOOLEAN switchOn = FALSE;

    do
    {
        status = halReadRegister(pMac, MCU_RF_ON_OFF_CONTROL_REG, (tANI_U32 *)&regValue);
        if (status != eHAL_STATUS_SUCCESS)
        {
            break;
        }

        if ( (MCU_RF_ON_OFF_CONTROL_RD_ON_OFF_HWPIN_STAT_MASK & regValue) )
        {
            switchOn = TRUE;
        }

    } while (0);

    return( switchOn );
#else
    return TRUE;
#endif
}

tANI_U8 halStateGet(tpAniSirGlobal pMac)
{
    return pMac->hal.halState;
}

void halStateSet(tpAniSirGlobal pMac, tANI_U8 state)
{
    pMac->hal.halState = state;
}


/** --------------------------------------------
\fn      halSetChainPowerState
\brief   This function gets the CFG and calls
\        halSetChainConfig().
\param   tpAniSirGlobal  pMac
\return  none
\ ---------------------------------------------- */
static void halSetChainPowerState(tpAniSirGlobal pMac)
{
    tANI_U32  chainState;

    if (wlan_cfgGetInt(pMac, WNI_CFG_POWER_STATE_PER_CHAIN, &chainState) != eSIR_SUCCESS )
        HALLOGP( halLog(pMac, LOGP, FL("Failed to get WNI_CFG_POWER_STATE_PER_CHAIN \n")));

    pMac->hal.cfgPowerStatePerChain = chainState;
    halSetChainConfig(pMac, chainState);
    return;
}

/** ------------------------------------------------------
\fn      halSetChainConfig
\brief   This function checks if the powerStatePerChain
\        setting is a valid entry.  If so, it will call
\        halPhy to set it appropraitely.
\
\param   tpAniSirGlobal  pMac
\param   tANI_U32        powerStatePerChain
\return  none
\ -------------------------------------------------------- */
void halSetChainConfig(tpAniSirGlobal pMac, tANI_U32 powerStatePerChain)
{
    tANI_U16         chain0, chain1, chain2;
    tANI_BOOLEAN     matchFound = eANI_BOOLEAN_FALSE;
    tANI_U8          i;
    tChainState      *pChainState = NULL;

    chain0 = (tANI_U16) GET_CHAIN_0(powerStatePerChain);
    chain1 = (tANI_U16) GET_CHAIN_1(powerStatePerChain);
    chain2 = (tANI_U16) GET_CHAIN_2(powerStatePerChain);

    HALLOGW( halLog(pMac, LOGW, FL("chain0=0x%x, chain1=0x%x, chain2=0x%x \n"), chain0, chain1, chain2));

    for (i = 0;  i < MAX_VALID_CHAIN_STATE; i++)
    {
        pChainState = &chainPwrStateTable[i];
        if (powerStatePerChain == pChainState->encoding)
        {
            HALLOGW( halLog(pMac, LOGW, FL("Match Found[%d], [0x%x,  %d] \n"), i, pChainState->encoding, pChainState->halPhyDef));
            matchFound = eANI_BOOLEAN_TRUE;
            break;
        }
    }

    if (matchFound)
    {
        if ( halPhySetChainSelect(pMac, pChainState->halPhyDef) != eHAL_STATUS_SUCCESS)
        {
            HALLOGE( halLog(pMac, LOGE, FL("halPhySetChainSelect(0x%x) failed \n"), pChainState->halPhyDef));
        }
    }
    else
    {
        HALLOGE( halLog(pMac, LOGE, FL("Invalid powerStatePerChain 0x%x \n"), powerStatePerChain));
    }

    return;
}







tSirRetStatus halReadWscMiscCfg(tHalHandle hMac,
                                tANI_U32 *wscApConfigMethod,
                                tANI_U8 *manufacturer,
                                tANI_U8 *modelName,
                                tANI_U8 *modelNumber,
                                tANI_U8 *serialNumber,
                                tANI_U8 *devicename)

{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hMac;
    tANI_U32 len;

    if (wlan_cfgGetInt(pMac, (tANI_U16) WNI_CFG_WPS_CFG_METHOD, wscApConfigMethod) != eSIR_SUCCESS)
    {
        HALLOGP( halLog( pMac, LOGP, FL("halReadWscInfoApCfgMethod: Failed to cfg get id %d\n"),  WNI_CFG_WPS_CFG_METHOD ));
        return eSIR_FAILURE;
    }

    HALLOG1( halLog( pMac, LOG1, FL("halReadWscInfoApCfgMethod: [%x]\n"),  *wscApConfigMethod ));

    len = WNI_CFG_MANUFACTURER_NAME_LEN - 1; /* excluding the null terminator */
    if (wlan_cfgGetStr(pMac, WNI_CFG_MANUFACTURER_NAME, manufacturer, &len) == eSIR_SUCCESS)
    {
        manufacturer[len] = '\0';
    }
    else
    {
        HALLOGP( halLog(pMac, LOGP, FL("could not retrieve manufacturer name\n")));
        return eSIR_FAILURE;
    }

    len = WNI_CFG_MODEL_NAME_LEN - 1;     /* excluding the null terminator */
    if (wlan_cfgGetStr(pMac, WNI_CFG_MODEL_NAME, modelName, &len) == eSIR_SUCCESS)
    {
        modelName[len] = '\0';
    }
    else
    {
        HALLOGP( halLog(pMac, LOGP, FL("could not retrieve model name\n")));
        return eSIR_FAILURE;
    }


    len = WNI_CFG_MODEL_NUMBER_LEN - 1;   /* excluding the null terminator */
    if (wlan_cfgGetStr(pMac, WNI_CFG_MODEL_NUMBER, modelNumber, &len) == eSIR_SUCCESS)
    {
        modelNumber[len] = '\0';
    }
    else
    {
        HALLOGP( halLog(pMac, LOGP, FL("could not retrieve model number\n")));
        return eSIR_FAILURE;
    }

    len = WNI_CFG_MANUFACTURER_PRODUCT_VERSION_LEN - 1; /* excluding the null terminator */
    if (wlan_cfgGetStr(pMac, WNI_CFG_MANUFACTURER_PRODUCT_VERSION, serialNumber, &len) == eSIR_SUCCESS)
    {
        serialNumber[len] = '\0';
    }
    else
    {
        HALLOGP( halLog(pMac, LOGP, FL("could not retrieve serial number\n")));
        return eSIR_FAILURE;
    }

    len = WNI_CFG_MANUFACTURER_PRODUCT_NAME_LEN - 1; /* excluding the null terminator */
    if (wlan_cfgGetStr(pMac, WNI_CFG_MANUFACTURER_PRODUCT_NAME, devicename, &len) == eSIR_SUCCESS)
    {
        devicename[len] = '\0';
    }
    else
    {
        HALLOGP( halLog(pMac, LOGP, FL("could not retrieve device namer\n")));
        return eSIR_FAILURE;
    }

    return eSIR_SUCCESS;
}


tANI_U32
halTlPostMsgApi(tpAniSirGlobal pMac, tSirMsgQ *pMsg)
{
#ifdef VOSS_ENABLED
    return  vos_mq_post_message(VOS_MQ_ID_TL, (vos_msg_t *) pMsg);
#endif
}

static 
eHalStatus halHandleMcastBcastFilterSetting(tpAniSirGlobal pMac, tANI_U32 cfgId)
{
    tANI_U32 val;
    eHalStatus status = eHAL_STATUS_SUCCESS;

    if(eSIR_SUCCESS != wlan_cfgGetInt(pMac, (tANI_U16)cfgId, &val))
    {
        HALLOGP( halLog(pMac, LOGP, FL("Get cfg id (%d) failed \n"), cfgId));
        return eHAL_STATUS_FAILURE;
    }
    else
    {    
        pMac->hal.mcastBcastFilterSetting = (tANI_BOOLEAN)val;
    }
    
    return status;
}

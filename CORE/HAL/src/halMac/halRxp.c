/**
 *
 *  @file:         halRxp.c
 *
 *  @brief:       Provides all the MAC APIs to the RXP Hardware Block.
 *
 *  @author:    Susan Tsao
 *
 *  Copyright (C) 2008, Qualcomm, Inc. All rights reserved.
 *
 *  Change History:
 * ---------------------------------------
 * 02/15/2006  File created.
 * 01/17/2009  Virgo related chages made.
 */

#include "halTypes.h"
#include "palTypes.h"
#include "aniGlobal.h"
#include "libraDefs.h"
#include "halRxp.h"
#include "sirMacProtDef.h" // tSirMacAddr
#include "halDebug.h"
#include "halAdu.h"

#include "halRegBckup.h"

/* -------------------------------------------------------------------------
 * local definitions
 */

#define GET_RXP_INFO(pMac) ( pMac ? ( tpRxpInfo ) pMac->hal.halMac.rxpInfo : 0 )
#define RXP_CMD_MAX_RETRY 100
#define PHY_CLOCK_FREQ_MHZ  160 // The Clock Frequency at which the NOVA PHY runs
#define RXP_STALL_TIMEOUT_US    12 // RXP STALL Timeout duration in usec
#define RXP_STALL_TIMEOUT_VALUE 0x2000
#define HAL_RXP_TYPE_SUBTYPE_MASK 0xffff0000

typedef struct sRxpFilterConfig {
    tFrameType  frameType;
    tANI_U32    configValue;
} tRxpFilterConfig;


typedef struct sRxpFilterName {
    tFrameType  frameType;
    char        frameName[30];
} tRxpFilterName;


typedef struct sRxpModeName {
    tRxpMode    mode;
    char        modeName[30];
} tRxpModeName;

typedef struct sRxpRegList {
    tANI_U32    addr;
    char       *name;
} tRxpRegList;
#define MKENTRY(addr) {addr, #addr}

#ifdef WLAN_DEBUG
static tRxpFilterName pktType[] = {

    {eMGMT_ASSOC_REQ,       STR("eMGMT_ASSOC_REQ")    },     // 0
    {eMGMT_ASSOC_RSP,       STR("eMGMT_ASSOC_RSP")    },     // 1
    {eMGMT_REASSOC_REQ,     STR("eMGMT_REASSOC_REQ")  },     // 2
    {eMGMT_REASSOC_RSP,     STR("eMGMT_REASSOC_RSP")  },     // 3
    {eMGMT_PROBE_REQ,       STR("eMGMT_PROBE_REQ")    },     // 4
    {eMGMT_PROBE_RSP,       STR("eMGMT_PROBE_RSP")    },     // 5
    {eMGMT_RSVD1,           STR("eMGMT_RSVD1")        },     // 6
    {eMGMT_RSVD2,           STR("eMGMT_RSVD2")        },     // 7
    {eMGMT_BEACON,          STR("eMGMT_BEACON")       },     // 8
    {eMGMT_ATIM,            STR("eMGMT_ATIM")         },     // 9
    {eMGMT_DISASSOC,        STR("eMGMT_DISASSOC")     },     // 10
    {eMGMT_AUTH,            STR("eMGMT_AUTH")         },     // 11
    {eMGMT_DEAUTH,          STR("eMGMT_DEAUTH")       },     // 12
    {eMGMT_ACTION,          STR("eMGMT_ACTION")       },     // 13
    {eMGMT_ACTION_NOACK,           STR("eMGMT_ACTION_NOACK")        },     // 14
    {eMGMT_RSVD4,           STR("eMGMT_RSVD4")        },     // 15
    {eCTRL_RSVD1,           STR("eCTRL_RSVD1")        },     // 16
    {eCTRL_RSVD2,           STR("eCTRL_RSVD2")        },     // 17
    {eCTRL_RSVD3,           STR("eCTRL_RSVD3")        },     // 18
    {eCTRL_RSVD4,           STR("eCTRL_RSVD4")        },     // 19
    {eCTRL_RSVD5,           STR("eCTRL_RSVD5")        },     // 20
    {eCTRL_RSVD6,           STR("eCTRL_RSVD6")        },     // 21
    {eCTRL_RSVD7,           STR("eCTRL_RSVD7")        },     // 22
    {eCTRL_CONTROL_WRAPPER,  STR("eCTRL_CONTROL_WRAPPER")        },     // 23
    {eCTRL_BAR,             STR("eCTRL_BAR")          },     // 24
    {eCTRL_BA,              STR("eCTRL_BA")           },     // 25
    {eCTRL_PSPOLL,          STR("eCTRL_PSPOLL")       },     // 26
    {eCTRL_RTS,             STR("eCTRL_RTS")          },     // 27
    {eCTRL_CTS,             STR("eCTRL_CTS")          },     // 28
    {eCTRL_ACK,             STR("eCTRL_ACK")          },     // 29
    {eCTRL_CFEND,           STR("eCTRL_CFEND")        },     // 30
    {eCTRL_CFEND_CFACK,     STR("eCTRL_CFEND_CFACK")  },     // 31
    {eDATA_DATA,            STR("eDATA_DATA")         },     // 32
    {eDATA_DATA_CFACK,      STR("eDATA_DATA_CFACK")   },     // 33
    {eDATA_DATA_CFPOLL,     STR("eDATA_DATA_CFPOLL")  },     // 34
    {eDATA_DATA_CFACK_CFPOLL, STR("eDATA_DATA_CFACK_CFPOLL") },    // 35
    {eDATA_NULL,            STR("eDATA_NULL")         },     // 36
    {eDATA_CFACK,           STR("eDATA_CFACK")        },     // 37
    {eDATA_CFPOLL,          STR("eDATA_CFPOLL")       },     // 38
    {eDATA_CFACK_CFPOLL,    STR("eDATA_CFACK_CFPOLL") },     // 39
    {eDATA_QOSDATA,         STR("eDATA_QOSDATA")      },     // 40
    {eDATA_QOSDATA_CFACK,   STR("eDATA_QOSDATA_CFACK")},     // 41
    {eDATA_QOSDATA_CFPOLL,  STR("eDATA_QOSDATA_CFPOLL")},    // 42
    {eDATA_QOSDATA_CFACK_CFPOLL, STR("eDATA_QOSDATA_CFACK_CFPOLL")}, // 43
    {eDATA_QOSNULL,         STR("eDATA_QOSNULL")      },     // 44
    {eDATA_RSVD1,           STR("eDATA_RSVD1")        },     // 45
    {eDATA_QOS_CFPOLL,      STR("eDATA_QOS_CFPOLL")   },     // 46
    {eDATA_QOS_CFACK_CFPOLL,STR("eDATA_QOS_CFACK_CFPOLL")},   // 47
    {eRSVD_RSVD0,    STR("eRSVD_RSVD0")},// 48
    {eRSVD_RSVD1,    STR("eRSVD_RSVD1")},// 49
    {eRSVD_RSVD2,    STR("eRSVD_RSVD2")},// 50
    {eRSVD_RSVD3,    STR("eRSVD_RSVD3")},// 51
    {eRSVD_RSVD4,    STR("eRSVD_RSVD4")},// 52
    {eRSVD_RSVD5,    STR("eRSVD_RSVD5")},// 53
    {eRSVD_RSVD6,    STR("eRSVD_RSVD6")},// 54
    {eRSVD_RSVD7,    STR("eRSVD_RSVD7")},// 55
    {eRSVD_RSVD8,    STR("eRSVD_RSVD8")},// 56
    {eRSVD_RSVD9,    STR("eRSVD_RSVD9")},// 57
    {eRSVD_RSVD10,    STR("eRSVD_RSVD10")},// 58
    {eRSVD_RSVD11,    STR("eRSVD_RSVD11")},// 59
    {eRSVD_RSVD12,    STR("eRSVD_RSVD12")},// 60
    {eRSVD_RSVD13,    STR("eRSVD_RSVD13")},// 61
    {eRSVD_RSVD14,    STR("eRSVD_RSVD14")},// 62
    {eRSVD_RSVD15,    STR("eRSVD_RSVD15")}// 63


};
#endif

#if 0
static tRxpModeName modeType[] =
{
    {eRXP_SCAN_MODE,        STR("SCAN_MODE")         },     // 0
    {eRXP_PRE_ASSOC_MODE,   STR("PRE_ASSOC_MODE")    },     // 1
    {eRXP_POST_ASSOC_MODE,  STR("POST_ASSOC_MODE")   },     // 2
    {eRXP_AP_MODE,          STR("AP_MODE")           },     // 3
    {eRXP_PROMISCUOUS_MODE, STR("PROMISCUOUS_MODE")  },     // 4
    {eRXP_LEARN_MODE,       STR("LEARN_MODE")        },     // 5
    {eRXP_POWER_SAVE_MODE,  STR("POWER_SAVE_MODE")   }      // 6
};
#endif


typedef struct sRxQIdMapping {
    tANI_U8     tid;
} tRxQIdMapping;

/* BTQM QID->TID mapping */
static tRxQIdMapping rxQIdMapping[] =
{ 
    /* BTQM QID 0-7 => TID 0 - 7 */
    {0}, {1}, {2}, {3}, {4}, {5}, {6}, {7},
    /* BTQM QID 8 => nonQos, mgmt */
    {0},
    /* BTQM QID9-18: All map to TID 0 */
    {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}
};

/*
 * Local functions
 */
static eHalStatus halRxp_setFilterMask(tpAniSirGlobal pMac, tRxpFilterConfig *table, tANI_U32 rsvd );
static eHalStatus halRxp_setFrameFilterMask(tpAniSirGlobal pMac, tANI_U32 frameType, tANI_U32 value);
static void print_table(tpAniSirGlobal pMac, tRxpAddrTable *table, tANI_U8 numOfEntry);
static void sort_table(tpAniSirGlobal pMac, tRxpAddrTable *table, tANI_U8 numOfEntry);
static eHalStatus write_rxp_search_table_reg(tpAniSirGlobal pMac, tRxpAddrTable *table, tANI_U8 index);
static eHalStatus delete_rxp_search_table_entry(tpAniSirGlobal pMac, tANI_U8 index);
static tANI_U8 compareMacAddr(tpAniSirGlobal pMac, tSirMacAddr addr1, tSirMacAddr addr2);
static tANI_U8 compareMacAddrRev(tpAniSirGlobal pMac, tSirMacAddr addr1, tSirMacAddr addr2);
static eHalStatus setRegister(tpAniSirGlobal pMac, tANI_U32 address, tANI_U32 value);
static tANI_U32 checkIfAddrExist(tpAniSirGlobal pMac, tSirMacAddr addr, tRxpAddrTable *table, tANI_U8 numOfEntry, tANI_U8* pEntryNum);
static eHalStatus halRxp_config_control_reg(tpAniSirGlobal pMac);
static eHalStatus halRxp_enableDpuParam(tpAniSirGlobal pMac);
static void setRxFrameDisableRegs( tpAniSirGlobal pMac, tANI_U32 regLo, tANI_U32 regHi );
static void halRxp_qid_mapping(tpAniSirGlobal pMac);
static eHalStatus searchAndDeleteTableEntry(tpAniSirGlobal pMac, tSirMacAddr macAddr,
        tANI_U8 addrType, tRxpAddrTable* pTable, tANI_U8 *pNumOfEntry, tANI_U8 *pFound);
static eHalStatus writeAddrTable(tpAniSirGlobal pMac, tRxpAddrTable* pTable, tANI_U8 addrType,
        tANI_U8 validTable, tANI_U8 highPtr, tANI_U8 lowPtr, tANI_U8 numOfEntry);
static eHalStatus fillAddrTableEntry(tpAniSirGlobal pMac, tRxpAddrTable* pRxpAddrTable,
        tANI_U8 staid, tSirMacAddr macAddr, tANI_U8 dropBit, tANI_U8 rmfBit,
        tANI_U8 dpuIdx, tANI_U8 dpuRFOrBcMcIdx, tANI_U8 dpuMgmtBcMcIdx,
        tANI_U8 dpuTag, tANI_U8 dpuDataBcMcTag, tANI_U8 dpuMgmtBcMcTag,
        tANI_U8 dpuNE, tANI_U8 ftBit, tANI_BOOLEAN keyIdExtract);

static eHalStatus halRxp_enable_multicast_broadcast(tpAniSirGlobal pMac);

#define  LESS             0
#define  SAME             1
#define  GREATER          2
#define  NOT_FOUND        3
#define DUPLICATE_ENTRY   0
#define ENTRY_NOT_FOUND   1
#define RXP_PUSH_WQ_CTRL_WQ3    0x03030303
#define RXP_PUSH_WQ_CTRL2_WQ3   0x00030303
#define ENTRY_FOUND     4


tANI_U32 rxpDisableFrameScanModeHi =
(
    RXP_TYPE_SUBTYPE_MASK(eMGMT_ASSOC_REQ) |
    RXP_TYPE_SUBTYPE_MASK(eMGMT_ASSOC_RSP) |
    RXP_TYPE_SUBTYPE_MASK(eMGMT_REASSOC_REQ) |
    RXP_TYPE_SUBTYPE_MASK(eMGMT_REASSOC_RSP) |
    RXP_TYPE_SUBTYPE_MASK(eMGMT_PROBE_REQ) |
    RXP_TYPE_SUBTYPE_MASK(eMGMT_RSVD1) |
    RXP_TYPE_SUBTYPE_MASK(eMGMT_RSVD2) |
    RXP_TYPE_SUBTYPE_MASK(eMGMT_ATIM) |
    RXP_TYPE_SUBTYPE_MASK(eMGMT_DISASSOC) |
    RXP_TYPE_SUBTYPE_MASK(eMGMT_AUTH) |
    RXP_TYPE_SUBTYPE_MASK(eMGMT_DEAUTH) |
    RXP_TYPE_SUBTYPE_MASK(eMGMT_ACTION) |
    RXP_TYPE_SUBTYPE_MASK(eMGMT_ACTION_NOACK) |
    RXP_TYPE_SUBTYPE_MASK(eMGMT_RSVD4) |
    RXP_TYPE_SUBTYPE_MASK(eCTRL_RSVD1) |
    RXP_TYPE_SUBTYPE_MASK(eCTRL_RSVD2) |
    RXP_TYPE_SUBTYPE_MASK(eCTRL_RSVD3) |
    RXP_TYPE_SUBTYPE_MASK(eCTRL_RSVD4) |
    RXP_TYPE_SUBTYPE_MASK(eCTRL_RSVD5) |
    RXP_TYPE_SUBTYPE_MASK(eCTRL_RSVD6) |
    RXP_TYPE_SUBTYPE_MASK(eCTRL_RSVD7) |
    RXP_TYPE_SUBTYPE_MASK(eCTRL_CONTROL_WRAPPER) |
    RXP_TYPE_SUBTYPE_MASK(eCTRL_BAR) |
    RXP_TYPE_SUBTYPE_MASK(eCTRL_BA) |
    RXP_TYPE_SUBTYPE_MASK(eCTRL_PSPOLL) |
    RXP_TYPE_SUBTYPE_MASK(eCTRL_CFEND) |
    RXP_TYPE_SUBTYPE_MASK(eCTRL_CFEND_CFACK)
);

tANI_U32 rxpDisableFrameScanModeLow =
{
    RXP_TYPE_SUBTYPE_MASK(eDATA_DATA) |
    RXP_TYPE_SUBTYPE_MASK(eDATA_DATA_CFACK) |
    RXP_TYPE_SUBTYPE_MASK(eDATA_DATA_CFPOLL) |
    RXP_TYPE_SUBTYPE_MASK(eDATA_DATA_CFACK_CFPOLL) |
    RXP_TYPE_SUBTYPE_MASK(eDATA_NULL) |
    RXP_TYPE_SUBTYPE_MASK(eDATA_CFACK) |
    RXP_TYPE_SUBTYPE_MASK(eDATA_CFPOLL) |
    RXP_TYPE_SUBTYPE_MASK(eDATA_CFACK_CFPOLL) |
    RXP_TYPE_SUBTYPE_MASK(eDATA_QOSDATA) |
    RXP_TYPE_SUBTYPE_MASK(eDATA_QOSDATA_CFACK) |
    RXP_TYPE_SUBTYPE_MASK(eDATA_QOSDATA_CFPOLL) |
    RXP_TYPE_SUBTYPE_MASK(eDATA_QOSDATA_CFACK_CFPOLL) |
    RXP_TYPE_SUBTYPE_MASK(eDATA_QOSNULL) |
    RXP_TYPE_SUBTYPE_MASK(eDATA_RSVD1) |
    RXP_TYPE_SUBTYPE_MASK(eDATA_QOS_CFPOLL) |
    RXP_TYPE_SUBTYPE_MASK(eDATA_QOS_CFACK_CFPOLL)
};


tANI_U32 rxpDisableFrameStaPostAssocModeHi =
(
    RXP_TYPE_SUBTYPE_MASK(eMGMT_ASSOC_REQ) |
    RXP_TYPE_SUBTYPE_MASK(eMGMT_REASSOC_REQ) |
    RXP_TYPE_SUBTYPE_MASK(eMGMT_PROBE_REQ) |
    RXP_TYPE_SUBTYPE_MASK(eMGMT_RSVD1) |
    RXP_TYPE_SUBTYPE_MASK(eMGMT_RSVD2) |
    RXP_TYPE_SUBTYPE_MASK(eMGMT_ATIM) |
    RXP_TYPE_SUBTYPE_MASK(eMGMT_ACTION_NOACK) |
    RXP_TYPE_SUBTYPE_MASK(eMGMT_RSVD4) |
    RXP_TYPE_SUBTYPE_MASK(eCTRL_RSVD1) |
    RXP_TYPE_SUBTYPE_MASK(eCTRL_RSVD2) |
    RXP_TYPE_SUBTYPE_MASK(eCTRL_RSVD3) |
    RXP_TYPE_SUBTYPE_MASK(eCTRL_RSVD4) |
    RXP_TYPE_SUBTYPE_MASK(eCTRL_RSVD5) |
    RXP_TYPE_SUBTYPE_MASK(eCTRL_RSVD6) |
    RXP_TYPE_SUBTYPE_MASK(eCTRL_RSVD7) |
    RXP_TYPE_SUBTYPE_MASK(eCTRL_CONTROL_WRAPPER) |
    RXP_TYPE_SUBTYPE_MASK(eCTRL_PSPOLL)
);

tANI_U32 rxpDisableFrameStaPostAssocModeLow =
(
    RXP_TYPE_SUBTYPE_MASK(eDATA_DATA_CFACK) |
    RXP_TYPE_SUBTYPE_MASK(eDATA_DATA_CFPOLL) |
    RXP_TYPE_SUBTYPE_MASK(eDATA_DATA_CFACK_CFPOLL) |
    RXP_TYPE_SUBTYPE_MASK(eDATA_CFACK) |
    RXP_TYPE_SUBTYPE_MASK(eDATA_CFPOLL) |
    RXP_TYPE_SUBTYPE_MASK(eDATA_CFACK_CFPOLL) |
    RXP_TYPE_SUBTYPE_MASK(eDATA_RSVD1)
);

tANI_U32 rxpDisableFrameStaPreAssocModeHi =
(
    RXP_TYPE_SUBTYPE_MASK(eMGMT_ASSOC_REQ) |
    RXP_TYPE_SUBTYPE_MASK(eMGMT_REASSOC_REQ) |
    RXP_TYPE_SUBTYPE_MASK(eMGMT_PROBE_REQ) |
    RXP_TYPE_SUBTYPE_MASK(eMGMT_RSVD1) |
    RXP_TYPE_SUBTYPE_MASK(eMGMT_RSVD2) |
    RXP_TYPE_SUBTYPE_MASK(eMGMT_ATIM) |
    RXP_TYPE_SUBTYPE_MASK(eMGMT_DISASSOC) |
    RXP_TYPE_SUBTYPE_MASK(eMGMT_ACTION) |
    RXP_TYPE_SUBTYPE_MASK(eMGMT_ACTION_NOACK) |
    RXP_TYPE_SUBTYPE_MASK(eMGMT_RSVD4) |
    RXP_TYPE_SUBTYPE_MASK(eCTRL_RSVD1) |
    RXP_TYPE_SUBTYPE_MASK(eCTRL_RSVD2) |
    RXP_TYPE_SUBTYPE_MASK(eCTRL_RSVD3) |
    RXP_TYPE_SUBTYPE_MASK(eCTRL_RSVD4) |
    RXP_TYPE_SUBTYPE_MASK(eCTRL_RSVD5) |
    RXP_TYPE_SUBTYPE_MASK(eCTRL_RSVD6) |
    RXP_TYPE_SUBTYPE_MASK(eCTRL_RSVD7) |
    RXP_TYPE_SUBTYPE_MASK(eCTRL_CONTROL_WRAPPER) |
    RXP_TYPE_SUBTYPE_MASK(eCTRL_BAR) |
    RXP_TYPE_SUBTYPE_MASK(eCTRL_BA) |
    RXP_TYPE_SUBTYPE_MASK(eCTRL_PSPOLL) |
    RXP_TYPE_SUBTYPE_MASK(eCTRL_CFEND) |
    RXP_TYPE_SUBTYPE_MASK(eCTRL_CFEND_CFACK)
);


tANI_U32 rxpDisableFrameStaPreAssocModeLow =
(
    RXP_TYPE_SUBTYPE_MASK(eDATA_DATA) |
    RXP_TYPE_SUBTYPE_MASK(eDATA_DATA_CFACK) |
    RXP_TYPE_SUBTYPE_MASK(eDATA_DATA_CFPOLL) |
    RXP_TYPE_SUBTYPE_MASK(eDATA_DATA_CFACK_CFPOLL) |
    RXP_TYPE_SUBTYPE_MASK(eDATA_NULL) |
    RXP_TYPE_SUBTYPE_MASK(eDATA_CFACK )|
    RXP_TYPE_SUBTYPE_MASK(eDATA_CFPOLL) |
    RXP_TYPE_SUBTYPE_MASK(eDATA_CFACK_CFPOLL) |
    RXP_TYPE_SUBTYPE_MASK(eDATA_QOSDATA) |
    RXP_TYPE_SUBTYPE_MASK(eDATA_QOSDATA_CFACK) |
    RXP_TYPE_SUBTYPE_MASK(eDATA_QOSDATA_CFPOLL) |
    RXP_TYPE_SUBTYPE_MASK(eDATA_QOSDATA_CFACK_CFPOLL) |
    RXP_TYPE_SUBTYPE_MASK(eDATA_QOSNULL) |
    RXP_TYPE_SUBTYPE_MASK(eDATA_RSVD1) |
    RXP_TYPE_SUBTYPE_MASK(eDATA_QOS_CFPOLL) |
    RXP_TYPE_SUBTYPE_MASK(eDATA_QOS_CFACK_CFPOLL)
);


tANI_U32 rxpDisableFrameIbssModeHi =
{
    RXP_TYPE_SUBTYPE_MASK(eMGMT_ASSOC_RSP) |
    RXP_TYPE_SUBTYPE_MASK(eMGMT_REASSOC_RSP) |
    RXP_TYPE_SUBTYPE_MASK(eMGMT_RSVD1) |
    RXP_TYPE_SUBTYPE_MASK(eMGMT_RSVD2) |
    RXP_TYPE_SUBTYPE_MASK(eMGMT_ATIM) |
    RXP_TYPE_SUBTYPE_MASK(eMGMT_ACTION_NOACK) |
    RXP_TYPE_SUBTYPE_MASK(eMGMT_RSVD4) |
    RXP_TYPE_SUBTYPE_MASK(eCTRL_RSVD1) |
    RXP_TYPE_SUBTYPE_MASK(eCTRL_RSVD2) |
    RXP_TYPE_SUBTYPE_MASK(eCTRL_RSVD3) |
    RXP_TYPE_SUBTYPE_MASK(eCTRL_RSVD4) |
    RXP_TYPE_SUBTYPE_MASK(eCTRL_RSVD5) |
    RXP_TYPE_SUBTYPE_MASK(eCTRL_RSVD6) |
    RXP_TYPE_SUBTYPE_MASK(eCTRL_RSVD7) |
    RXP_TYPE_SUBTYPE_MASK(eCTRL_CONTROL_WRAPPER)
};

tANI_U32 rxpDisableFrameIbssModeLow =
{
    RXP_TYPE_SUBTYPE_MASK(eDATA_DATA_CFACK) |
    RXP_TYPE_SUBTYPE_MASK(eDATA_DATA_CFPOLL) |
    RXP_TYPE_SUBTYPE_MASK(eDATA_DATA_CFACK_CFPOLL) |
    RXP_TYPE_SUBTYPE_MASK(eDATA_CFACK) |
    RXP_TYPE_SUBTYPE_MASK(eDATA_CFPOLL) |
    RXP_TYPE_SUBTYPE_MASK(eDATA_CFACK_CFPOLL) |
    RXP_TYPE_SUBTYPE_MASK(eDATA_QOSDATA_CFACK) |
    RXP_TYPE_SUBTYPE_MASK(eDATA_QOSDATA_CFPOLL) |
    RXP_TYPE_SUBTYPE_MASK(eDATA_QOSDATA_CFACK_CFPOLL) |
    RXP_TYPE_SUBTYPE_MASK(eDATA_RSVD1) |
    RXP_TYPE_SUBTYPE_MASK(eDATA_QOS_CFPOLL) |
    RXP_TYPE_SUBTYPE_MASK(eDATA_QOS_CFACK_CFPOLL)
};

//dummy value
tANI_U32 rxpDisableFrameApModeHi = 0;
tANI_U32 rxpDisableFrameApModeLow = 0;

tANI_U32 rxpDisableFrameFtmModeHi =
{
    RXP_TYPE_SUBTYPE_MASK(eMGMT_RSVD1) |
    RXP_TYPE_SUBTYPE_MASK(eMGMT_RSVD2) |
    RXP_TYPE_SUBTYPE_MASK(eMGMT_RSVD4) |
    RXP_TYPE_SUBTYPE_MASK(eMGMT_ASSOC_REQ) |
    RXP_TYPE_SUBTYPE_MASK(eMGMT_ASSOC_RSP) |
    RXP_TYPE_SUBTYPE_MASK(eMGMT_REASSOC_REQ) |
    RXP_TYPE_SUBTYPE_MASK(eMGMT_REASSOC_RSP) |
    RXP_TYPE_SUBTYPE_MASK(eMGMT_PROBE_REQ) |
    RXP_TYPE_SUBTYPE_MASK(eMGMT_PROBE_RSP) |
    RXP_TYPE_SUBTYPE_MASK(eMGMT_BEACON) |
    RXP_TYPE_SUBTYPE_MASK(eMGMT_ATIM) |
    RXP_TYPE_SUBTYPE_MASK(eMGMT_DISASSOC) |
    RXP_TYPE_SUBTYPE_MASK(eMGMT_AUTH) |
    RXP_TYPE_SUBTYPE_MASK(eMGMT_DEAUTH) |
    RXP_TYPE_SUBTYPE_MASK(eMGMT_ACTION) |
    RXP_TYPE_SUBTYPE_MASK(eMGMT_ACTION_NOACK) |
    RXP_TYPE_SUBTYPE_MASK(eCTRL_CONTROL_WRAPPER) |
    RXP_TYPE_SUBTYPE_MASK(eCTRL_BAR) |
    RXP_TYPE_SUBTYPE_MASK(eCTRL_BA) |
    RXP_TYPE_SUBTYPE_MASK(eCTRL_PSPOLL) |
    RXP_TYPE_SUBTYPE_MASK(eCTRL_RTS) |
    RXP_TYPE_SUBTYPE_MASK(eCTRL_CTS) |
    RXP_TYPE_SUBTYPE_MASK(eCTRL_ACK) |
    RXP_TYPE_SUBTYPE_MASK(eCTRL_CFEND) |
    RXP_TYPE_SUBTYPE_MASK(eCTRL_CFEND_CFACK) |        
    RXP_TYPE_SUBTYPE_MASK(eCTRL_RSVD1) |
    RXP_TYPE_SUBTYPE_MASK(eCTRL_RSVD2) |
    RXP_TYPE_SUBTYPE_MASK(eCTRL_RSVD3) |
    RXP_TYPE_SUBTYPE_MASK(eCTRL_RSVD4) |
    RXP_TYPE_SUBTYPE_MASK(eCTRL_RSVD5) |
    RXP_TYPE_SUBTYPE_MASK(eCTRL_RSVD6) |
    RXP_TYPE_SUBTYPE_MASK(eCTRL_RSVD7)
};

tANI_U32 rxpDisableFrameFtmModeLow =
{
    RXP_TYPE_SUBTYPE_MASK(eDATA_DATA_CFACK) |         
    RXP_TYPE_SUBTYPE_MASK(eDATA_DATA_CFPOLL) |        
    RXP_TYPE_SUBTYPE_MASK(eDATA_DATA_CFACK_CFPOLL) |  
    RXP_TYPE_SUBTYPE_MASK(eDATA_NULL) |
    RXP_TYPE_SUBTYPE_MASK(eDATA_CFACK) |              
    RXP_TYPE_SUBTYPE_MASK(eDATA_CFPOLL) |             
    RXP_TYPE_SUBTYPE_MASK(eDATA_CFACK_CFPOLL) |       
    RXP_TYPE_SUBTYPE_MASK(eDATA_QOSDATA_CFACK) |      
    RXP_TYPE_SUBTYPE_MASK(eDATA_QOSDATA_CFPOLL) |     
    RXP_TYPE_SUBTYPE_MASK(eDATA_QOSDATA_CFACK_CFPOLL) |
    RXP_TYPE_SUBTYPE_MASK(eDATA_QOSNULL) |
    RXP_TYPE_SUBTYPE_MASK(eDATA_QOS_CFPOLL) |         
    RXP_TYPE_SUBTYPE_MASK(eDATA_QOS_CFACK_CFPOLL) |   
    RXP_TYPE_SUBTYPE_MASK(eDATA_RSVD1) |
    RXP_TYPE_SUBTYPE_MASK(eRSVD_RSVD0) |
    RXP_TYPE_SUBTYPE_MASK(eRSVD_RSVD1) |
    RXP_TYPE_SUBTYPE_MASK(eRSVD_RSVD2) |
    RXP_TYPE_SUBTYPE_MASK(eRSVD_RSVD3) |
    RXP_TYPE_SUBTYPE_MASK(eRSVD_RSVD4) |
    RXP_TYPE_SUBTYPE_MASK(eRSVD_RSVD5) |
    RXP_TYPE_SUBTYPE_MASK(eRSVD_RSVD6) |
    RXP_TYPE_SUBTYPE_MASK(eRSVD_RSVD7) |
    RXP_TYPE_SUBTYPE_MASK(eRSVD_RSVD8) |
    RXP_TYPE_SUBTYPE_MASK(eRSVD_RSVD9) |
    RXP_TYPE_SUBTYPE_MASK(eRSVD_RSVD10) |
    RXP_TYPE_SUBTYPE_MASK(eRSVD_RSVD11) |
    RXP_TYPE_SUBTYPE_MASK(eRSVD_RSVD12) |
    RXP_TYPE_SUBTYPE_MASK(eRSVD_RSVD13) |
    RXP_TYPE_SUBTYPE_MASK(eRSVD_RSVD14) |
    RXP_TYPE_SUBTYPE_MASK(eRSVD_RSVD15)
};

/* ----------------------------------
 * RXP Frame Filter Table: ALL MODE
 * ----------------------------------
 */
tRxpFilterConfig rxpFilterTable_AllMode[] = {
    {eMGMT_ASSOC_REQ,            (RXP_VERSION|RXP_NAV_SET|RXP_ADDR1_BLOCK_BROADCAST|RXP_ADDR1_BLOCK_MULTICAST|RXP_ADDR1_FILTER|RXP_ACCEPT_ALL_ADDR2|RXP_FCS) },
    {eMGMT_ASSOC_RSP,            (RXP_VERSION|RXP_NAV_SET|RXP_FCS|RXP_ADDR1_BLOCK_BROADCAST|RXP_ADDR1_BLOCK_MULTICAST|
                                  RXP_ADDR1_FILTER|RXP_ACCEPT_ALL_ADDR2|RXP_ACCEPT_ALL_ADDR3) },
    {eMGMT_REASSOC_REQ,          (RXP_VERSION|RXP_NAV_SET|RXP_ADDR1_BLOCK_BROADCAST|RXP_ADDR1_BLOCK_MULTICAST|RXP_ADDR1_FILTER|RXP_ACCEPT_ALL_ADDR2|RXP_FCS) },
    {eMGMT_REASSOC_RSP,          (RXP_VERSION|RXP_NAV_SET|RXP_FCS|RXP_ADDR1_BLOCK_BROADCAST|RXP_ADDR1_BLOCK_MULTICAST|
                                  RXP_ADDR1_FILTER|RXP_ACCEPT_ALL_ADDR2|RXP_ACCEPT_ALL_ADDR3) },
    {eMGMT_PROBE_REQ,            (RXP_VERSION|RXP_NAV_SET|RXP_ADDR1_BLOCK_MULTICAST|RXP_ADDR1_FILTER|RXP_ADDR1_ACCEPT_MULTICAST|RXP_ADDR1_ACCEPT_UNICAST|RXP_ACCEPT_ALL_ADDR2|RXP_FCS) },
    {eMGMT_PROBE_RSP,            (RXP_VERSION|RXP_NAV_SET|RXP_ADDR1_FILTER|RXP_ADDR1_ACCEPT_MULTICAST|RXP_ADDR1_ACCEPT_UNICAST|RXP_FCS) },
    {eMGMT_RSVD1,                (RXP_VERSION|RXP_NAV_SET|RXP_ADDR1_BLOCK_UNICAST|RXP_ADDR1_BLOCK_BROADCAST|RXP_ADDR1_BLOCK_MULTICAST|RXP_FCS) },
    {eMGMT_RSVD2,                (RXP_VERSION|RXP_NAV_SET|RXP_ADDR1_BLOCK_UNICAST|RXP_ADDR1_BLOCK_BROADCAST|RXP_ADDR1_BLOCK_MULTICAST|RXP_FCS) },
    {eMGMT_BEACON,               (RXP_VERSION|RXP_NAV_SET|RXP_FCS|RXP_ADDR1_FILTER|RXP_ACCEPT_ALL_ADDR2|RXP_ACCEPT_ALL_ADDR3) },
    {eMGMT_ATIM,                 (RXP_VERSION|RXP_NAV_SET|RXP_ADDR1_BLOCK_UNICAST|RXP_ADDR1_BLOCK_BROADCAST|RXP_ADDR1_BLOCK_MULTICAST|RXP_FCS) },
    {eMGMT_DISASSOC,             (RXP_VERSION|RXP_NAV_SET|RXP_ADDR1_FILTER|RXP_ADDR2_FILTER|RXP_ACCEPT_ALL_ADDR3|RXP_FCS) },
    {eMGMT_AUTH,                 (RXP_VERSION|RXP_NAV_SET|RXP_FCS|RXP_ADDR1_BLOCK_BROADCAST|RXP_ADDR1_BLOCK_MULTICAST|
                                  RXP_ADDR1_FILTER|RXP_ACCEPT_ALL_ADDR2|RXP_ACCEPT_ALL_ADDR3) },
    {eMGMT_DEAUTH,               (RXP_VERSION|RXP_NAV_SET|RXP_FCS| RXP_ADDR1_FILTER|RXP_ACCEPT_ALL_ADDR2|RXP_ACCEPT_ALL_ADDR3) },
    {eMGMT_ACTION,               (RXP_VERSION|RXP_NAV_SET|RXP_ADDR1_FILTER|RXP_ADDR1_ACCEPT_MULTICAST|RXP_ADDR2_FILTER|RXP_ADDR2_ACCEPT_REMAIN|RXP_FCS) },
    {eMGMT_ACTION_NOACK,         (RXP_VERSION|RXP_NAV_SET|RXP_ADDR1_BLOCK_UNICAST|RXP_ADDR1_BLOCK_BROADCAST|RXP_ADDR1_BLOCK_MULTICAST|RXP_FCS) },
    {eMGMT_RSVD4,                (RXP_VERSION|RXP_NAV_SET|RXP_ADDR1_BLOCK_UNICAST|RXP_ADDR1_BLOCK_BROADCAST|RXP_ADDR1_BLOCK_MULTICAST|RXP_FCS) },
    {eCTRL_RSVD1,                (RXP_VERSION|RXP_NAV_SET|RXP_ADDR1_BLOCK_UNICAST|RXP_ADDR1_BLOCK_BROADCAST|RXP_ADDR1_BLOCK_MULTICAST|RXP_FCS) },
    {eCTRL_RSVD2,                (RXP_VERSION|RXP_NAV_SET|RXP_ADDR1_BLOCK_UNICAST|RXP_ADDR1_BLOCK_BROADCAST|RXP_ADDR1_BLOCK_MULTICAST|RXP_FCS) },
    {eCTRL_RSVD3,                (RXP_VERSION|RXP_NAV_SET|RXP_ADDR1_BLOCK_UNICAST|RXP_ADDR1_BLOCK_BROADCAST|RXP_ADDR1_BLOCK_MULTICAST|RXP_FCS) },
    {eCTRL_RSVD4,                (RXP_VERSION|RXP_NAV_SET|RXP_ADDR1_BLOCK_UNICAST|RXP_ADDR1_BLOCK_BROADCAST|RXP_ADDR1_BLOCK_MULTICAST|RXP_FCS) },
    {eCTRL_RSVD5,                (RXP_VERSION|RXP_NAV_SET|RXP_ADDR1_BLOCK_UNICAST|RXP_ADDR1_BLOCK_BROADCAST|RXP_ADDR1_BLOCK_MULTICAST|RXP_FCS) },
    {eCTRL_RSVD6,                (RXP_VERSION|RXP_NAV_SET|RXP_ADDR1_BLOCK_UNICAST|RXP_ADDR1_BLOCK_BROADCAST|RXP_ADDR1_BLOCK_MULTICAST|RXP_FCS) },
    {eCTRL_RSVD7,                (RXP_VERSION|RXP_NAV_SET|RXP_ADDR1_BLOCK_UNICAST|RXP_ADDR1_BLOCK_BROADCAST|RXP_ADDR1_BLOCK_MULTICAST|RXP_FCS) },
    {eCTRL_CONTROL_WRAPPER,      (RXP_VERSION|RXP_NAV_SET|RXP_ADDR1_BLOCK_UNICAST|RXP_ADDR1_BLOCK_BROADCAST|RXP_ADDR1_BLOCK_MULTICAST|RXP_FCS) },
    {eCTRL_BAR,                  (RXP_VERSION|RXP_NAV_SET|RXP_ADDR1_BLOCK_BROADCAST|RXP_ADDR1_BLOCK_MULTICAST|RXP_ADDR1_FILTER|RXP_ADDR2_FILTER|RXP_ADDR2_ACCEPT_REMAIN|RXP_FCS) },
    {eCTRL_BA,                   (RXP_VERSION|RXP_NAV_SET|RXP_ADDR1_BLOCK_BROADCAST|RXP_ADDR1_BLOCK_MULTICAST|RXP_ADDR1_FILTER|RXP_ADDR2_FILTER|RXP_FCS|RXP_DROP_AT_DMA) },
    {eCTRL_PSPOLL,               (RXP_VERSION|RXP_ADDR1_FILTER|RXP_ADDR1_ACCEPT_UNICAST|RXP_ADDR1_BLOCK_BROADCAST|RXP_ADDR1_BLOCK_MULTICAST|RXP_ADDR2_FILTER|RXP_FCS|RXP_DROP_AT_DMA) },
    {eCTRL_RTS,                  (RXP_VERSION|RXP_NAV_SET|RXP_ADDR1_BLOCK_BROADCAST|RXP_ADDR1_BLOCK_MULTICAST|RXP_ADDR1_FILTER|RXP_ACCEPT_ALL_ADDR2|RXP_FCS|RXP_DROP_AT_DMA) },
    {eCTRL_CTS,                  (RXP_VERSION|RXP_NAV_SET|RXP_ADDR1_BLOCK_BROADCAST|RXP_ADDR1_BLOCK_MULTICAST|RXP_ADDR1_FILTER|RXP_FCS|RXP_DROP_AT_DMA) },
    {eCTRL_ACK,                  (RXP_VERSION|RXP_NAV_SET|RXP_ADDR1_BLOCK_BROADCAST|RXP_ADDR1_BLOCK_MULTICAST|RXP_ADDR1_FILTER|RXP_FCS|RXP_DROP_AT_DMA) },
    {eCTRL_CFEND,                (RXP_VERSION|RXP_NAV_CLEAR|RXP_ADDR1_BLOCK_UNICAST|RXP_ADDR1_BLOCK_BROADCAST|RXP_ADDR1_BLOCK_MULTICAST|RXP_ADDR1_FILTER|RXP_ADDR2_FILTER|RXP_ADDR2_ACCEPT_REMAIN|RXP_DROP_AT_DMA|RXP_FCS) },
    {eCTRL_CFEND_CFACK,          (RXP_VERSION|RXP_NAV_CLEAR|RXP_ADDR1_BLOCK_UNICAST|RXP_ADDR1_BLOCK_BROADCAST|RXP_ADDR1_BLOCK_MULTICAST|RXP_ADDR1_FILTER|RXP_ADDR2_FILTER|RXP_ADDR2_ACCEPT_REMAIN|RXP_DROP_AT_DMA|RXP_FCS) },
    {eDATA_DATA,                 (RXP_VERSION|RXP_NAV_SET|RXP_ADDR1_FILTER| RXP_ADDR1_ACCEPT_MULTICAST|RXP_ADDR2_FILTER|RXP_ACCEPT_ALL_ADDR3|RXP_FCS|RXP_FRAME_TRANSLATION) },
    {eDATA_DATA_CFACK,           (RXP_VERSION|RXP_NAV_SET|RXP_ADDR1_BLOCK_UNICAST|RXP_ADDR1_BLOCK_BROADCAST|RXP_ADDR1_BLOCK_MULTICAST|RXP_FCS) },
    {eDATA_DATA_CFPOLL,          (RXP_VERSION|RXP_NAV_SET|RXP_ADDR1_BLOCK_UNICAST|RXP_ADDR1_BLOCK_BROADCAST|RXP_ADDR1_BLOCK_MULTICAST|RXP_FCS) },
    {eDATA_DATA_CFACK_CFPOLL,    (RXP_VERSION|RXP_NAV_SET|RXP_ADDR1_BLOCK_UNICAST|RXP_ADDR1_BLOCK_BROADCAST|RXP_ADDR1_BLOCK_MULTICAST|RXP_FCS) },
    {eDATA_NULL,                 (RXP_VERSION|RXP_NAV_SET|RXP_ADDR1_FILTER|RXP_ADDR2_FILTER|RXP_ACCEPT_ALL_ADDR3|RXP_FCS|RXP_DROP_AT_DMA) },
    {eDATA_CFACK,                (RXP_VERSION|RXP_NAV_SET|RXP_ADDR1_BLOCK_UNICAST|RXP_ADDR1_BLOCK_BROADCAST|RXP_ADDR1_BLOCK_MULTICAST|RXP_FCS|RXP_DROP_AT_DMA) },
    {eDATA_CFPOLL,               (RXP_VERSION|RXP_NAV_SET|RXP_ADDR1_BLOCK_UNICAST|RXP_ADDR1_BLOCK_BROADCAST|RXP_ADDR1_BLOCK_MULTICAST|RXP_FCS|RXP_DROP_AT_DMA) },
    {eDATA_CFACK_CFPOLL,         (RXP_VERSION|RXP_NAV_SET|RXP_ADDR1_BLOCK_UNICAST|RXP_ADDR1_BLOCK_BROADCAST|RXP_ADDR1_BLOCK_MULTICAST|RXP_FCS|RXP_DROP_AT_DMA) },
    {eDATA_QOSDATA,              (RXP_VERSION|RXP_NAV_SET|RXP_ADDR1_FILTER|RXP_ADDR1_ACCEPT_MULTICAST|RXP_ADDR2_FILTER|RXP_ACCEPT_ALL_ADDR3|RXP_FCS|RXP_FRAME_TRANSLATION) },
    {eDATA_QOSDATA_CFACK,        (RXP_VERSION|RXP_NAV_SET|RXP_ADDR1_BLOCK_UNICAST|RXP_ADDR1_BLOCK_BROADCAST|RXP_ADDR1_BLOCK_MULTICAST|RXP_ADDR1_FILTER|RXP_ADDR2_FILTER|RXP_ACCEPT_ALL_ADDR3|RXP_FCS) },
    {eDATA_QOSDATA_CFPOLL,       (RXP_VERSION|RXP_NAV_SET|RXP_ADDR1_BLOCK_UNICAST|RXP_ADDR1_BLOCK_BROADCAST|RXP_ADDR1_BLOCK_MULTICAST|RXP_ADDR1_FILTER|RXP_ADDR2_FILTER|RXP_ACCEPT_ALL_ADDR3|RXP_FCS) },
    {eDATA_QOSDATA_CFACK_CFPOLL, (RXP_VERSION|RXP_NAV_SET|RXP_ADDR1_BLOCK_UNICAST|RXP_ADDR1_BLOCK_BROADCAST|RXP_ADDR1_BLOCK_MULTICAST|RXP_ADDR1_FILTER|RXP_ADDR2_FILTER|RXP_ACCEPT_ALL_ADDR3|RXP_FCS) },
    {eDATA_QOSNULL,              (RXP_VERSION|RXP_NAV_SET|RXP_ADDR1_FILTER|RXP_ADDR2_FILTER|RXP_ACCEPT_ALL_ADDR3|RXP_FCS|RXP_DROP_AT_DMA) },
    {eDATA_RSVD1,                (RXP_VERSION|RXP_NAV_SET|RXP_ADDR1_BLOCK_UNICAST|RXP_ADDR1_BLOCK_BROADCAST|RXP_ADDR1_BLOCK_MULTICAST|RXP_FCS|RXP_DROP_AT_DMA) },
    {eDATA_QOS_CFPOLL,           (RXP_VERSION|RXP_NAV_SET|RXP_ADDR1_BLOCK_UNICAST|RXP_ADDR1_BLOCK_BROADCAST|RXP_ADDR1_BLOCK_MULTICAST|RXP_ADDR1_FILTER|RXP_ADDR2_FILTER|RXP_ACCEPT_ALL_ADDR3|RXP_FCS|RXP_DROP_AT_DMA) },
    {eDATA_QOS_CFACK_CFPOLL,     (RXP_VERSION|RXP_NAV_SET|RXP_ADDR1_BLOCK_UNICAST|RXP_ADDR1_BLOCK_BROADCAST|RXP_ADDR1_BLOCK_MULTICAST|RXP_ADDR1_FILTER|RXP_ADDR2_FILTER|RXP_ACCEPT_ALL_ADDR3|RXP_FCS|RXP_DROP_AT_DMA) },
    {eRSVD_RSVD0,                (RXP_VERSION|RXP_NAV_SET|RXP_ADDR1_BLOCK_UNICAST|RXP_ADDR1_BLOCK_BROADCAST|RXP_ADDR1_BLOCK_MULTICAST|RXP_FCS) },
    {eRSVD_RSVD1,                (RXP_VERSION|RXP_NAV_SET|RXP_ADDR1_BLOCK_UNICAST|RXP_ADDR1_BLOCK_BROADCAST|RXP_ADDR1_BLOCK_MULTICAST|RXP_FCS) },
    {eRSVD_RSVD2,                (RXP_VERSION|RXP_NAV_SET|RXP_ADDR1_BLOCK_UNICAST|RXP_ADDR1_BLOCK_BROADCAST|RXP_ADDR1_BLOCK_MULTICAST|RXP_FCS) },
    {eRSVD_RSVD3,                (RXP_VERSION|RXP_NAV_SET|RXP_ADDR1_BLOCK_UNICAST|RXP_ADDR1_BLOCK_BROADCAST|RXP_ADDR1_BLOCK_MULTICAST|RXP_FCS) },
    {eRSVD_RSVD4,                (RXP_VERSION|RXP_NAV_SET|RXP_ADDR1_BLOCK_UNICAST|RXP_ADDR1_BLOCK_BROADCAST|RXP_ADDR1_BLOCK_MULTICAST|RXP_FCS) },
    {eRSVD_RSVD5,                (RXP_VERSION|RXP_NAV_SET|RXP_ADDR1_BLOCK_UNICAST|RXP_ADDR1_BLOCK_BROADCAST|RXP_ADDR1_BLOCK_MULTICAST|RXP_FCS) },
    {eRSVD_RSVD6,                (RXP_VERSION|RXP_NAV_SET|RXP_ADDR1_BLOCK_UNICAST|RXP_ADDR1_BLOCK_BROADCAST|RXP_ADDR1_BLOCK_MULTICAST|RXP_FCS) },
    {eRSVD_RSVD7,                (RXP_VERSION|RXP_NAV_SET|RXP_ADDR1_BLOCK_UNICAST|RXP_ADDR1_BLOCK_BROADCAST|RXP_ADDR1_BLOCK_MULTICAST|RXP_FCS) },
    {eRSVD_RSVD8,                (RXP_VERSION|RXP_NAV_SET|RXP_ADDR1_BLOCK_UNICAST|RXP_ADDR1_BLOCK_BROADCAST|RXP_ADDR1_BLOCK_MULTICAST|RXP_FCS) },
    {eRSVD_RSVD9,                (RXP_VERSION|RXP_NAV_SET|RXP_ADDR1_BLOCK_UNICAST|RXP_ADDR1_BLOCK_BROADCAST|RXP_ADDR1_BLOCK_MULTICAST|RXP_FCS) },
    {eRSVD_RSVD10,               (RXP_VERSION|RXP_NAV_SET|RXP_ADDR1_BLOCK_UNICAST|RXP_ADDR1_BLOCK_BROADCAST|RXP_ADDR1_BLOCK_MULTICAST|RXP_FCS) },
    {eRSVD_RSVD11,               (RXP_VERSION|RXP_NAV_SET|RXP_ADDR1_BLOCK_UNICAST|RXP_ADDR1_BLOCK_BROADCAST|RXP_ADDR1_BLOCK_MULTICAST|RXP_FCS) },
    {eRSVD_RSVD12,               (RXP_VERSION|RXP_NAV_SET|RXP_ADDR1_BLOCK_UNICAST|RXP_ADDR1_BLOCK_BROADCAST|RXP_ADDR1_BLOCK_MULTICAST|RXP_FCS) },
    {eRSVD_RSVD13,               (RXP_VERSION|RXP_NAV_SET|RXP_ADDR1_BLOCK_UNICAST|RXP_ADDR1_BLOCK_BROADCAST|RXP_ADDR1_BLOCK_MULTICAST|RXP_FCS) },
    {eRSVD_RSVD14,               (RXP_VERSION|RXP_NAV_SET|RXP_ADDR1_BLOCK_UNICAST|RXP_ADDR1_BLOCK_BROADCAST|RXP_ADDR1_BLOCK_MULTICAST|RXP_FCS) },
    {eRSVD_RSVD15,               (RXP_VERSION|RXP_NAV_SET|RXP_ADDR1_BLOCK_UNICAST|RXP_ADDR1_BLOCK_BROADCAST|RXP_ADDR1_BLOCK_MULTICAST|RXP_FCS) }
};

/* -------------------------------------------------------
 * halRxp_Open()
 *
 * FUNCTION:
 *   RXP initialization procedures.
 * -------------------------------------------------------
 */
eHalStatus halRxp_Open(tHalHandle hHal, void *arg)
{
    eHalStatus status;
    tpAniSirGlobal pMac = (tpAniSirGlobal)hHal;

    (void) arg;

    status = palAllocateMemory(pMac->hHdd, &pMac->hal.halMac.rxpInfo,
                               sizeof(tRxpInfo));
    if (status != eHAL_STATUS_SUCCESS){
        HALLOGE(halLog(pMac, LOGE, FL("RXP Open failed\n")));
        return status;
    }
    (void) palZeroMemory(pMac->hHdd, pMac->hal.halMac.rxpInfo, sizeof(tRxpInfo));

    return eHAL_STATUS_SUCCESS;
}

/* -------------------------------------------------------
 * halRxp_Close()
 *
 * FUNCTION:
 *   RXP initialization procedures.
 * -------------------------------------------------------
 */
eHalStatus halRxp_Close(tHalHandle hHal, void *arg)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tpAniSirGlobal pMac = (tpAniSirGlobal)hHal;

    (void) arg;
    if (pMac->hal.halMac.rxpInfo != NULL)
        status = palFreeMemory(pMac->hHdd, pMac->hal.halMac.rxpInfo);
    pMac->hal.halMac.rxpInfo = NULL;
    return status;
}

/* -------------------------------------------------------
 * halRxp_Start()
 *
 * FUNCTION:
 *   RXP initialization procedures.
 * -------------------------------------------------------
 */
eHalStatus halRxp_Start(tHalHandle hHal, void *arg)
{
    tpAniSirGlobal  pMac = (tpAniSirGlobal)hHal;
    tANI_U32        value;
    tANI_U32 rxpStallTmoutCycles;

    (void) arg;

    // enable bmu reservation requests from RXP
    halReadRegister(pMac, QWLAN_RXP_CONFIG_REG, &value);
    value |= QWLAN_RXP_CONFIG_CFG_RSV_BMU_EN_MASK;
    halWriteRegister(pMac,  QWLAN_RXP_CONFIG_REG,  value );

    // set the max reserve PDU default to a recommended value
    halWriteRegister(pMac,  QWLAN_RXP_DMA_MAX_RSV_PDU_REG,  RXP_DMA_MAX_RSV_PDU );

    value = sizeof(tHalRxBd);
    halWriteRegister(pMac,  QWLAN_RXP_MPDU_HEADER_START_OFFSET_REG,  value );

    // Enable RXP stall timeout
    //rxpStallTmoutCycles = QWLAN_RXP_STALL_TIMEOUT_US * PHY_CLOCK_FREQ_MHZ;

    rxpStallTmoutCycles = RXP_STALL_TIMEOUT_VALUE;

    value = QWLAN_RXP_STALL_TIMEOUT_STALL_TIMEOUT_EN_MASK | rxpStallTmoutCycles;

    halWriteRegister(pMac,  QWLAN_RXP_STALL_TIMEOUT_REG,  value );

    /** Enable the configuration required for Gen6 */
    halRxp_config_control_reg(pMac);

    /* Enable All broadcast/multicast frames to comein, address 2 lookup 
     * will follow after this */
    halRxp_enable_multicast_broadcast(pMac);

    // Enable the Gen6 DPU param bit in RXP config 3 register
    if (halRxp_enableDpuParam(pMac) != eHAL_STATUS_SUCCESS)
        return eHAL_STATUS_FAILURE;

    /** Enable the WQ3 for routing the received flags */
    halWriteRegister(pMac, QWLAN_RXP_PUSH_WQ_CTRL_REG, RXP_PUSH_WQ_CTRL_WQ3);

    halWriteRegister(pMac, QWLAN_RXP_PUSH_WQ_CTRL2_REG, RXP_PUSH_WQ_CTRL2_WQ3);

    /** Configure the QID mapping */
    halRxp_qid_mapping(pMac);

    halRxp_setFilterMask(pMac, rxpFilterTable_AllMode, 0);
    
    if(pMac->gDriverType == eDRIVER_TYPE_MFG)
    {
        // Initialize rxpMode to FTM mode
        halRxp_setRxpFilterMode(pMac, eRXP_FTM_MODE, NULL);    
    }
    else
    {
        // Initialize rxpMode to IDLE mode
        halRxp_setRxpFilterMode(pMac, eRXP_IDLE_MODE, NULL);
    }
    // Enable NAV set from CF parameter value in beacons
    halReadRegister(pMac, QWLAN_RXP_BEACON_TSF_TIM_EXTRACT_CTRL_REG, &value);
    value |= QWLAN_RXP_BEACON_TSF_TIM_EXTRACT_CTRL_BEACON_CFP_AWARENESS_ENABLE_MASK;
    halWriteRegister(pMac, QWLAN_RXP_BEACON_TSF_TIM_EXTRACT_CTRL_REG, value);


    return eHAL_STATUS_SUCCESS;
}

/* -------------------------------------------------------
\brief     Reads the channel from Rxp Register and returns it.
\param  pMac
\return  Channel number.
---------------------------------------------------------*/
tANI_U8 halRxp_getChannel(tpAniSirGlobal pMac)
{
    tANI_U32   regVal;
    tpHalRxBd  pRxBd;

    halReadRegister(pMac, QWLAN_RXP_ROUTING_FLAG_REG, &regVal);

    pRxBd = (tpHalRxBd) &regVal;

    return ((tANI_U8) pRxBd->rxChannel);
}

/* -------------------------------------------------------
 * halRxp_setChannel()
 *
 * FUNCTION:
 *   This function sets the channel number in RXP, this channel number is
 * added by RXP to BD of every frame it receives.
 * -------------------------------------------------------
 */
eHalStatus halRxp_setChannel(tpAniSirGlobal pMac, tANI_U8 channel)
{
    tANI_U32   value;
    tpHalRxBd  pRxBd;

    halReadRegister(pMac, QWLAN_RXP_ROUTING_FLAG_REG, &value);

    pRxBd = (tpHalRxBd)&value;
    pRxBd->rxChannel = channel;

    halWriteRegister(pMac, QWLAN_RXP_ROUTING_FLAG_REG, value);

    return eHAL_STATUS_SUCCESS;
}


/* -------------------------------------------------------
 * halRxp_setScanLearn()
 *
 * FUNCTION:
 *   This function sets the scan bit in RXP routing flag register.
 *   RXP copies this in BD as it is.
 * -------------------------------------------------------
 */
void halRxp_setScanLearn(tpAniSirGlobal pMac, tANI_U8 scanLearn)
{
    tANI_U32        value;
    tpHalRxBd pRxBd;

    halReadRegister(pMac, QWLAN_RXP_ROUTING_FLAG_REG, &value);

    pRxBd = (tpHalRxBd)&value;
    pRxBd->scanLearn = scanLearn;

    halWriteRegister(pMac, QWLAN_RXP_ROUTING_FLAG_REG, value);

}


/* -------------------------------------------------------
 * halRxp_Stop()
 *
 * FUNCTION:
 *   Zero out rxpInfo structure.
 * -------------------------------------------------------
 */
eHalStatus halRxp_Stop(tHalHandle hHal, void *arg)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hHal;
    (void) arg;

    if(pMac->hal.halMac.rxpInfo)
    {
        (void) palZeroMemory(pMac->hHdd, pMac->hal.halMac.rxpInfo, sizeof(tRxpInfo));
    }

    return eHAL_STATUS_SUCCESS;
}

/* ------------------------------------------------------
 * FUNCTION:  halRxp_SetFilter()
 *
 * NOTE:
 *   Set RXP Filter to the specified mode configuration.
 * ------------------------------------------------------
 */
eHalStatus halRxp_SetFilter(tpAniSirGlobal pMac, tRxpMode mode)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tANI_U32 value = 0;

    HALLOG1( halLog(pMac, LOG1, FL("set RXP to mode = %d. \n"), mode));

    /* Disable Rxp, this would block all frames while toggling between modes */
    if (halRxp_disable(pMac) == eHAL_STATUS_FAILURE) {
        HALLOGE(halLog(pMac, LOGE, FL("RXP Disable Failed\n")));
        return eHAL_STATUS_FAILURE;
    }

    switch(mode)
    {
        case eRXP_IDLE_MODE:
            setRxFrameDisableRegs(pMac, RXP_DROP_ALL_FRAME_TYPES, RXP_DROP_ALL_FRAME_TYPES);
            // Put it back to the old default setting  
            value = (RXP_VERSION|RXP_NAV_SET|RXP_FCS|RXP_ADDR1_FILTER|RXP_ACCEPT_ALL_ADDR2|RXP_ACCEPT_ALL_ADDR3);
            halRxp_setFrameFilterMask(pMac, eMGMT_BEACON, value);
            break;

        case eRXP_SCAN_MODE:
            setRxFrameDisableRegs(pMac, rxpDisableFrameScanModeLow , rxpDisableFrameScanModeHi);
            // Unblock the A2 filter for beacons & probe repsonses, 
            // Accept beacons and probe responses from all address2
            value = (RXP_VERSION|RXP_NAV_SET|RXP_FCS|RXP_ADDR1_FILTER|RXP_ACCEPT_ALL_ADDR2|RXP_ACCEPT_ALL_ADDR3);
            halRxp_setFrameFilterMask(pMac, eMGMT_BEACON, value);
            halRxp_setFrameFilterMask(pMac, eMGMT_PROBE_RSP, value);
            break;

        case eRXP_PRE_ASSOC_MODE:
            setRxFrameDisableRegs(pMac, rxpDisableFrameStaPreAssocModeLow , rxpDisableFrameStaPreAssocModeHi);
            // Accept only beacons & probe responses from specific address2, rest filter it out.
            value = (RXP_VERSION|RXP_NAV_SET|RXP_FCS|RXP_ADDR1_FILTER|RXP_ADDR2_FILTER|RXP_ACCEPT_ALL_ADDR3);
            halRxp_setFrameFilterMask(pMac, eMGMT_BEACON, value);
            halRxp_setFrameFilterMask(pMac, eMGMT_PROBE_RSP, value);
            break;

        case eRXP_POST_ASSOC_MODE:
            setRxFrameDisableRegs(pMac, rxpDisableFrameStaPostAssocModeLow , rxpDisableFrameStaPostAssocModeHi);
            // Accept only beacons & probe responses from address2 BSS to which 
            // are now associated, rest filter it out.
            value = (RXP_VERSION|RXP_NAV_SET|RXP_FCS|RXP_ADDR1_FILTER|RXP_ADDR2_FILTER|RXP_ACCEPT_ALL_ADDR3);
            halRxp_setFrameFilterMask(pMac, eMGMT_BEACON, value);
            halRxp_setFrameFilterMask(pMac, eMGMT_PROBE_RSP, value);
            break;

        case eRXP_AP_MODE:
            break;

        case eRXP_PROMISCUOUS_MODE:
            break;

        case eRXP_LEARN_MODE:
            break;

        case eRXP_POWER_SAVE_MODE:
            break;

        case eRXP_IBSS_MODE:
            setRxFrameDisableRegs(pMac, rxpDisableFrameIbssModeLow , rxpDisableFrameIbssModeHi);
            // Force to accept all A2 beacons. 
            value = (RXP_VERSION|RXP_NAV_SET|RXP_FCS|RXP_ADDR1_FILTER|RXP_ACCEPT_ALL_ADDR2|RXP_ACCEPT_ALL_ADDR3);
            halRxp_setFrameFilterMask(pMac, eMGMT_BEACON, value);
            break;

        case eRXP_FTM_MODE:
            HALLOG1( halLog(pMac, LOG1, FL("Disable all frames except data and qos data from coming in\n")));
            setRxFrameDisableRegs(pMac, rxpDisableFrameFtmModeLow , rxpDisableFrameFtmModeHi);
            break;

        default:
            HALLOGE( halLog( pMac, LOGE, FL("halRxp_SetFilter: unknown mode specified (%d)\n"),  mode ));
            status = eHAL_STATUS_FAILURE;
            break;
    }

    /* Enable Rxp back, this would un-block all frames as per the filter */
    halRxp_enable(pMac);
    return( status );
}


/* -----------------------
 * halRxp_storeRxpMode()
 * halRxp_getRxpMode()
 * -----------------------
 */
void halRxp_storeRxpMode(tpAniSirGlobal pMac, tRxpMode mode)
{
    HALLOG1( halLog( pMac, LOG1, FL("%s: store halMac.rxpMode to %d \n"),  __FUNCTION__, mode ));
    pMac->hal.halMac.rxpMode = (tANI_U32) mode;
    return;
}

tRxpMode halRxp_getRxpMode(tpAniSirGlobal pMac)
{
    return ((tRxpMode)pMac->hal.halMac.rxpMode);
}

eHalStatus halRxp_AddPreAssocAddr2Entry(tpAniSirGlobal pMac, tSirMacAddr macAddr)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tANI_U8 staIdx = (tANI_U8)pMac->hal.halMac.selfStaId;
    tANI_U8 dpuIdx, bcastDpuIdx, bcastMgmtDpuIdx;
    tANI_U8 dpuSignature = 0;
    tANI_U8 rmfBit = 0, ftBit = 0;
    eAniBoolean wep_keyId_extract = eANI_BOOLEAN_FALSE;

    if(macAddr == NULL) {
        return status;
    }

    halTable_GetStaDpuIdx(pMac, staIdx, &dpuIdx);
    halTable_GetStaBcastDpuIdx(pMac, staIdx, &bcastDpuIdx);
    halTable_GetStaBcastMgmtDpuIdx(pMac, staIdx, &bcastMgmtDpuIdx);

    // Get the frame translation setting
    ftBit = halGetFrameTranslation(pMac);
    halDpu_GetSignature(pMac, dpuIdx, &dpuSignature);

    status = halRxp_AddEntry(pMac, staIdx, macAddr, eRXP_PEER_AP, rmfBit,
            dpuIdx, bcastDpuIdx, bcastMgmtDpuIdx, 
            dpuSignature, dpuSignature, dpuSignature,
            0, ftBit, wep_keyId_extract);

    if (status != eHAL_STATUS_SUCCESS ){
        HALLOGE(halLog(pMac, LOGE, FL("halRxp_addEntry failed, status = %x"), status));
    }
    return status;
}

eHalStatus halRxp_RemovePreAssocAddr2Entry(tpAniSirGlobal pMac, tSirMacAddr macAddr)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;

    if(macAddr != NULL) {
        status = halRxp_DelEntry(pMac, macAddr);
        if(status != eHAL_STATUS_SUCCESS) {
            HALLOGE(halLog(pMac, LOGE, FL("Failed at halRxp_DelEntry()  %X \n"), status));
        }
    }

    return status;
}

static void halRxp_qid_tid_mapping(tpAniSirGlobal pMac)
{
    tANI_U32 qid_map_0to3, qid_map4to7, qid_map8to11, qid_map12to15;

    qid_map_0to3 = rxQIdMapping[0].tid << QWLAN_RXP_QID_MAPPING1_QOS_TID0_QID_OFFSET |
                   rxQIdMapping[1].tid << QWLAN_RXP_QID_MAPPING1_QOS_TID1_QID_OFFSET |
                   rxQIdMapping[2].tid << QWLAN_RXP_QID_MAPPING1_QOS_TID2_QID_OFFSET |
                   rxQIdMapping[3].tid << QWLAN_RXP_QID_MAPPING1_QOS_TID3_QID_OFFSET;

    halWriteRegister(pMac, QWLAN_RXP_QID_MAPPING1_REG, qid_map_0to3);

    qid_map4to7 = rxQIdMapping[4].tid << QWLAN_RXP_QID_MAPPING2_QOS_TID4_QID_OFFSET |
                  rxQIdMapping[5].tid << QWLAN_RXP_QID_MAPPING2_QOS_TID5_QID_OFFSET |
                  rxQIdMapping[6].tid << QWLAN_RXP_QID_MAPPING2_QOS_TID6_QID_OFFSET |
                  rxQIdMapping[7].tid << QWLAN_RXP_QID_MAPPING2_QOS_TID7_QID_OFFSET;

    halWriteRegister(pMac, QWLAN_RXP_QID_MAPPING2_REG, qid_map4to7);

    qid_map8to11 = rxQIdMapping[8].tid << QWLAN_RXP_QID_MAPPING3_QOS_TID8_QID_OFFSET |
                   rxQIdMapping[9].tid << QWLAN_RXP_QID_MAPPING3_QOS_TID9_QID_OFFSET |
                   rxQIdMapping[10].tid << QWLAN_RXP_QID_MAPPING3_QOS_TID10_QID_OFFSET |
                   rxQIdMapping[11].tid << QWLAN_RXP_QID_MAPPING3_QOS_TID11_QID_OFFSET;

    halWriteRegister(pMac, QWLAN_RXP_QID_MAPPING3_REG, qid_map8to11);


    qid_map12to15 = rxQIdMapping[12].tid << QWLAN_RXP_QID_MAPPING4_QOS_TID12_QID_OFFSET |
                    rxQIdMapping[13].tid << QWLAN_RXP_QID_MAPPING4_QOS_TID13_QID_OFFSET |
                    rxQIdMapping[14].tid << QWLAN_RXP_QID_MAPPING4_QOS_TID14_QID_OFFSET |
                    rxQIdMapping[15].tid << QWLAN_RXP_QID_MAPPING4_QOS_TID15_QID_OFFSET;

    halWriteRegister(pMac, QWLAN_RXP_QID_MAPPING4_REG, qid_map12to15);


}

static void halRxp_qid_mapping(tpAniSirGlobal pMac)
{
    halWriteRegister(pMac, QWLAN_RXP_QID_MAPPING0_REG, RX_QUEUE_MGMT_nQOS) ;

    halRxp_qid_tid_mapping(pMac);

}

static eHalStatus halRxp_config_control_reg(tpAniSirGlobal pMac)
{
    tANI_U32 cfgValue;

    // Program the CONFIG2 register
    if(pMac->gDriverType == eDRIVER_TYPE_MFG)
    {    
        cfgValue = QWLAN_RXP_CONFIG2_CFG_BD_DPU_SOFTMACFIELDS_UPDATE_ENABLE_MASK |
                    QWLAN_RXP_CONFIG2_BMU_PWR_UPDATE_ENABLED_MASK |
                    QWLAN_RXP_CONFIG2_CFG_FLT_GENERATE_HW_RESPONSE_ENABLED_MASK |
                    QWLAN_RXP_CONFIG2_CFG_FLT_HW_SW_BASED_RESPONSE_SELECTION_ENABLE_MASK |
                    QWLAN_RXP_CONFIG2_CFG_FLT_FILTER_GO_TO_IDLE_ON_CLEAR_ENABLED_MASK |
                    QWLAN_RXP_CONFIG2_BD_CLEAR_ENABLE_ENABLE_MASK |
                    QWLAN_RXP_CONFIG2_CFG_MPR_BMU_BA_UPDATE_ENABLED_MASK |
                    QWLAN_RXP_CONFIG2_DEFAULT_PUSH_WQ_OVERWRITE_ENABLE_MASK |
                    (BMUWQ_DPU_RX << QWLAN_RXP_CONFIG2_DEFAULT_PUSH_WQ_OFFSET);
                    //RXP_CONFIG2_CFG_MPDU_PROC_WAIT_FOR_IDLE_FILTER_ENABLED_MASK  <** FIXME: Check if we require this ? */
    }
    else
    {
        cfgValue = QWLAN_RXP_CONFIG2_CFG_BD_DPU_SOFTMACFIELDS_UPDATE_ENABLE_MASK |
                    QWLAN_RXP_CONFIG2_CFG_RPE_INTERFACE_ENABLE_MASK |
                    QWLAN_RXP_CONFIG2_BMU_PWR_UPDATE_ENABLED_MASK |
                    QWLAN_RXP_CONFIG2_CFG_FLT_GENERATE_HW_RESPONSE_ENABLED_MASK |
                    QWLAN_RXP_CONFIG2_CFG_MPR_TPE_INTERFACE_ENABLED_MASK |
                    QWLAN_RXP_CONFIG2_CFG_FLT_HW_SW_BASED_RESPONSE_SELECTION_ENABLE_MASK |
                    QWLAN_RXP_CONFIG2_CFG_FLT_FILTER_GO_TO_IDLE_ON_CLEAR_ENABLED_MASK |
                    QWLAN_RXP_CONFIG2_BD_CLEAR_ENABLE_ENABLE_MASK |
                    QWLAN_RXP_CONFIG2_CFG_MPR_BMU_BA_UPDATE_ENABLED_MASK |
                    QWLAN_RXP_CONFIG2_DEFAULT_PUSH_WQ_OVERWRITE_ENABLE_MASK |
                    (BMUWQ_DPU_RX << QWLAN_RXP_CONFIG2_DEFAULT_PUSH_WQ_OFFSET);
                    //RXP_CONFIG2_CFG_MPDU_PROC_WAIT_FOR_IDLE_FILTER_ENABLED_MASK  <** FIXME: Check if we require this ? */
    }
    halWriteRegister(pMac,  QWLAN_RXP_CONFIG2_REG,  cfgValue );

    // Program the CONFIG3 register
    // Basically for the HW bug fix:
    // CR-0000142348: BAR handling during BD/PDU not available is not correct which is fixed in LIBRA2.0
	// CR-0000144751: METAL ECO: RxP stops parsing TIM element in beacon as soon as an IE's Element ID > TIM
    if (halGetChipRevNum(pMac) == LIBRA_CHIP_REV_ID_2_0) {
        halReadRegister(pMac, QWLAN_RXP_CONFIG3_REG, &cfgValue);
        cfgValue |= (QWLAN_RXP_CONFIG3_BAR_DMA_CONFIRM_ECO_EN_MASK | QWLAN_RXP_CONFIG3_TIMORDER_ECO_EN_MASK);
        halWriteRegister(pMac,  QWLAN_RXP_CONFIG3_REG,  cfgValue );
    }

    return eHAL_STATUS_SUCCESS;
}

static eHalStatus halRxp_enable_multicast_broadcast(tpAniSirGlobal pMac)
{
	tANI_U32 cfgValue;

	cfgValue = BROADCAST_STAID << QWLAN_RXP_CONFIG4_UNKNOWN_MC_STAID_OFFSET;
	cfgValue |= RXP_BROADCAST_ENTRY_PRESENT;

	halWriteRegister(pMac,  QWLAN_RXP_CONFIG4_REG,  cfgValue );

	return eHAL_STATUS_SUCCESS;
}


static eHalStatus halRxp_enableDpuParam(tpAniSirGlobal pMac)
{
    tANI_U32 value;

    halReadRegister(pMac, QWLAN_RXP_CONFIG3_REG, &value);
    value |= QWLAN_RXP_CONFIG3_CFG_GEN5_6_NEW_DPU_PARAM_EN_MASK;
    halWriteRegister(pMac, QWLAN_RXP_CONFIG3_REG, value);

    return eHAL_STATUS_SUCCESS;
}

eHalStatus halRxp_setMimoPwrSaveCtrlReg(tpAniSirGlobal pMac, tANI_U32 mask)
{
    tANI_U32 value;

    halReadRegister(pMac, QWLAN_RXP_MIMO_PWR_SAVE_CTRL_REG, &value);

    value |= mask;

    halWriteRegister(pMac, QWLAN_RXP_MIMO_PWR_SAVE_CTRL_REG, value);

    return eHAL_STATUS_SUCCESS;
}

static eHalStatus halRxp_getFrameFilterMask(tpAniSirGlobal pMac, tANI_U32 frameType, tANI_U32* pRegValue)
{
    tANI_U32 address;

    address = QWLAN_RXP_FRAME_FILTER_CONFIG_REG + (sizeof(tANI_U32)* frameType);
    return(halReadRegister(pMac, address, pRegValue));
}

static eHalStatus halRxp_setFrameFilterMask(tpAniSirGlobal pMac, tANI_U32 frameType, tANI_U32 value)
{
    tANI_U32 address;

    address = QWLAN_RXP_FRAME_FILTER_CONFIG_REG + (4 * frameType);
    if (setRegister(pMac, address, value) != eHAL_STATUS_SUCCESS)
    {
        HALLOGE(halLog(pMac, LOGE, FL("halRxp_setFrameFilterMask: setRegister() failed \n")));
        return eHAL_STATUS_FAILURE;
    }
    return eHAL_STATUS_SUCCESS;
}

eHalStatus halRxp_configureRxpFilterMcstBcst(tpAniSirGlobal pMac, tANI_BOOLEAN setFilter)
{
    tANI_U32 reg_value;
    tANI_U32 mask = 0;
    eHalStatus halStatus;

    if (setFilter && !(IS_PWRSAVE_STATE_IN_BMPS))
    {
        HALLOGE(halLog(pMac, LOGE, 
           FL("%s: Cannot set McastBcast filter, as device is not in BMPS\n"), __FUNCTION__));
        return eHAL_STATUS_FAILURE;
    }

    switch(pMac->hal.mcastBcastFilterSetting)
    {
        case FILTER_ALL_MULTICAST:
            mask = RXP_ADDR1_BLOCK_MULTICAST;   
        break;

        case FILTER_ALL_BROADCAST:
            mask = RXP_ADDR1_BLOCK_BROADCAST;   
        break;

        case FILTER_ALL_MULTICAST_BROADCAST:
            mask = RXP_ADDR1_BLOCK_MULTICAST | RXP_ADDR1_BLOCK_BROADCAST;   
        break;
    }

    if (IS_PWRSAVE_STATE_IN_BMPS)
        halPS_SetHostBusy(pMac, HAL_PS_BUSY_GENERIC); 

    halStatus = halRxp_getFrameFilterMask(pMac, eDATA_DATA, &reg_value);
    if(eHAL_STATUS_SUCCESS == halStatus)
    {
        if(setFilter)
            halRxp_setFrameFilterMask(pMac, eDATA_DATA, reg_value | mask);
        else
            halRxp_setFrameFilterMask(pMac, eDATA_DATA, reg_value & ~mask);
    }

    halStatus = halRxp_getFrameFilterMask(pMac, eDATA_QOSDATA, &reg_value);
    if(eHAL_STATUS_SUCCESS == halStatus)
    {
        if(setFilter)
            halRxp_setFrameFilterMask(pMac, eDATA_QOSDATA, reg_value | mask);
        else
            halRxp_setFrameFilterMask(pMac, eDATA_QOSDATA, reg_value & ~mask);
    }

    if (IS_PWRSAVE_STATE_IN_BMPS)
        halPS_ReleaseHostBusy(pMac, HAL_PS_BUSY_GENERIC);
 
    return eHAL_STATUS_SUCCESS;
}

/* --------------------------------
 * halRxp_setFilterMask
 * --------------------------------
 */
static eHalStatus halRxp_setFilterMask(tpAniSirGlobal pMac, tRxpFilterConfig *table, tANI_U32 rsvd )
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    tANI_U32  frameType, regValue;

    for (frameType=0; frameType < RXP_FRAME_TYPES_MAX; frameType++)
    {
        if (frameType <= eDATA_QOS_CFACK_CFPOLL)
            regValue = table[frameType].configValue;
        else
            regValue = rsvd;        

        if(pMac->gDriverType == eDRIVER_TYPE_MFG)
        {
            if (frameType == eDATA_DATA || frameType == eDATA_QOSDATA)
            {
                regValue = regValue | RXP_ADDR2_ACCEPT_REMAIN | RXP_ADDR1_BLOCK_BROADCAST | RXP_ADDR1_BLOCK_MULTICAST | RXP_DROP_AT_DMA;
            }
        }
        status = halRxp_setFrameFilterMask(pMac, frameType, regValue);
        if(status != eHAL_STATUS_SUCCESS)
        {
            HALLOGE( halLog(pMac, LOGE, FL("halRxp_setFilterMask: setRegister() failed \n")));
            return eHAL_STATUS_FAILURE;
        }
    }
    return eHAL_STATUS_SUCCESS;
}


/* --------------
 * setRegister
 * --------------
 */
static eHalStatus setRegister(tpAniSirGlobal pMac, tANI_U32 address, tANI_U32 value)
{
    tANI_U32  readValue;

    halWriteRegister(pMac, address, value);

    halReadRegister(pMac, address, &readValue);

    return eHAL_STATUS_SUCCESS;
}

/* ---------------------------------------------------
 * FUNCTION:  delete_rxp_search_table_entry()
 *
 * NOTE:
 *   1) Write zero to "search_table_data0 and data1" register:
 *         - station address valid bit (set to 0)
 *   2) Write to action register "search_table_cmd"
 *         - write (set this bit to 1, indicating we're writting)
 *         - entry number in the rxp binary search table for which
 *           this belongs to.
 * ----------------------------------------------------
 */
static eHalStatus delete_rxp_search_table_entry(tpAniSirGlobal pMac,tANI_U8 index)
{
    tANI_U32  value = 0;
    tANI_U32  regValue = 0;

    HALLOG4( halLog( pMac, LOG4, FL("delete_rxp_search_table_entry: entry=%d\n"),  index ));

    // "search_table_data_0"
    halWriteRegister(pMac, QWLAN_RXP_SEARCH_TABLE_DATA0_REG, value);
    // "search_table_data_1"
    halWriteRegister(pMac, QWLAN_RXP_SEARCH_TABLE_DATA1_REG, value);

    // "search_table_cmd"
    value = QWLAN_RXP_SEARCH_TABLE_CMD_WRITE_MASK | index;

     //status should be zero before writing to command reg
    halReadRegister(pMac, QWLAN_RXP_SEARCH_TABLE_CMD_REG, &regValue);
    regValue &= QWLAN_RXP_SEARCH_TABLE_CMD_STATUS_MASK;
    if(regValue == 0)
    {
      halWriteRegister(pMac, QWLAN_RXP_SEARCH_TABLE_CMD_REG, value);
    }
    else
    {
      HALLOGW( halLog(pMac, LOGW, FL("delete_rxp_search_table_entry failed: cmd reg status is not zero before write\n")));
      return eHAL_STATUS_FAILURE;
    }

    return eHAL_STATUS_SUCCESS;
}

static eHalStatus update_rxp_search_table_reg(tpAniSirGlobal pMac, tRxpAddrTable *table, tANI_U8 index)
{
    tANI_U32  value;
    tANI_U32 regValue;
    tANI_U8 count;

    HALLOGW( halLog( pMac, LOGW, FL("rxp search table2 table->wepKeyIdxExtractEnable is 0x%x\n"),  table->wepKeyIdxExtractEnable ));
    HALLOGW( halLog( pMac, LOGW, FL("rxp search table2 table->staid is 0x%x\n"),  table->staid ));
    HALLOGW( halLog( pMac, LOGW, FL("rxp search table2 table->dpuPTKDescIdx is 0x%x\n"),  table->dpuPTKDescIdx ));
    HALLOGW( halLog( pMac, LOGW, FL("rxp search table2 index is 0x%x\n"),  index ));

    // "search_table_cmd"
    value = index;
    //status should be zero before writing to command reg
    for(count = 0; count < RXP_CMD_MAX_RETRY; count++)
    {
      halReadRegister(pMac, QWLAN_RXP_SEARCH_TABLE_CMD_REG, &regValue);
      if(!(regValue &= QWLAN_RXP_SEARCH_TABLE_CMD_STATUS_MASK))
        break;
    }

    if(count < RXP_CMD_MAX_RETRY)
    {
      halWriteRegister(pMac, QWLAN_RXP_SEARCH_TABLE_CMD_REG, value);
    }
    else
    {
      HALLOGE( halLog(pMac, LOGE, FL("write_rxp_search_table_reg failed: cmd reg status is not zero before write\n")));
      return eHAL_STATUS_FAILURE;
    }

    halReadRegister(pMac, QWLAN_RXP_SEARCH_TABLE_DATA2_REG, &value);

    HALLOGW( halLog( pMac, LOGW, FL("rxp search table2 register is 0x%x\n"),  value ));
    HALLOGW( halLog( pMac, LOGW, FL("rxp search table2 table->wepKeyIdxExtractEnable is 0x%x\n"),  table->wepKeyIdxExtractEnable ));
   /** search_table_data2 */
   value &= ~QWLAN_RXP_SEARCH_TABLE_DATA2_WEP_KEY_ID_EXTRACT_ENABLE_MASK;
   value |= (table->wepKeyIdxExtractEnable << QWLAN_RXP_SEARCH_TABLE_DATA2_WEP_KEY_ID_EXTRACT_ENABLE_OFFSET);

   HALLOGW( halLog( pMac, LOGW, FL("rxp search table2 register is 0x%x\n"),  value ));

    halWriteRegister(pMac, QWLAN_RXP_SEARCH_TABLE_DATA2_REG, value);


    // "search_table_cmd"
    value = QWLAN_RXP_SEARCH_TABLE_CMD_WRITE_MASK | index;
    //status should be zero before writing to command reg
    for(count = 0; count < RXP_CMD_MAX_RETRY; count++)
    {
      halReadRegister(pMac, QWLAN_RXP_SEARCH_TABLE_CMD_REG, &regValue);
      if(!(regValue &= QWLAN_RXP_SEARCH_TABLE_CMD_STATUS_MASK))
        break;
    }

    if(count < RXP_CMD_MAX_RETRY)
    {
      halWriteRegister(pMac, QWLAN_RXP_SEARCH_TABLE_CMD_REG, value);
    }
    else
    {
      HALLOGE( halLog(pMac, LOGE, FL("write_rxp_search_table_reg failed: cmd reg status is not zero before write\n")));
      return eHAL_STATUS_FAILURE;
    }

    return eHAL_STATUS_SUCCESS;
}

/* ---------------------------------------------------
 * FUNCTION:  write_rxp_search_table_reg()
 *
 * NOTE:
 *   1) Write to "search_table_data0" register: mac_address[31:0]
 *   2) Write to "search_table_data1" register:
 *         - station address valid bit (set to 1)
 *         - set station drop bit accordingly
 *         - station id
 *         - mac_address[32:47]
 *   3) Write to action register "search_table_cmd"
 *         - write (set this bit to 1, indicating we're writting)
 *         - entry number in the rxp binary search table for which
 *           this belongs to.
 * ----------------------------------------------------
 */
static eHalStatus write_rxp_search_table_reg(tpAniSirGlobal pMac, tRxpAddrTable *table, tANI_U8 index)
{
    tANI_U32  value;
    tANI_U32 regValue;
    tANI_U8 count;

    HALLOG1( halLog(pMac, LOG1, FL("write_rxp_search_table_reg: mac=%x:%x:%x:%x:%x:%x, staid=%d, entry=%d dpuIGTKTag %d  dpuGTKDpuIdx %d dpuPTKDescIdx %d\n"),
            table->macAddr[0], table->macAddr[1], table->macAddr[2],
            table->macAddr[3], table->macAddr[4], table->macAddr[5],
            table->staid, index, table->dpuIGTKTag,table->dpuGTKDpuIdx, 
            table->dpuPTKDescIdx));

    // "search_table_data0"
    value = (table->macAddr[3] << 24) | (table->macAddr[2] << 16) |
        (table->macAddr[1] << 8)  | table->macAddr[0];
    halWriteRegister(pMac, QWLAN_RXP_SEARCH_TABLE_DATA0_REG, value);

    // "search_table_data_1"
    value = ((table->dpuIGTKTag << QWLAN_RXP_SEARCH_TABLE_DATA1_DPU_MAN_MC_BC_SIG_OFFSET) |
            (QWLAN_RXP_SEARCH_TABLE_DATA1_VALID_MASK) |
            (table->staid << QWLAN_RXP_SEARCH_TABLE_DATA1_ADDR_ID_OFFSET) |
            (table->rmfBit << QWLAN_RXP_SEARCH_TABLE_DATA1_RMF_OFFSET) |
            (table->ftBit << QWLAN_RXP_SEARCH_TABLE_DATA1_FRAME_TRANSLATION_OFFSET) |
            (table->dropBit << QWLAN_RXP_SEARCH_TABLE_DATA1_DROP_OFFSET)  |
            (table->macAddr[5] << 8) |
            (table->macAddr[4]));
    halWriteRegister(pMac, QWLAN_RXP_SEARCH_TABLE_DATA1_REG, value);

    /** search_table_data2 */
    value = ((table->dpuPTKDescIdx) |
            (table->dpuGTKDpuIdx << QWLAN_RXP_SEARCH_TABLE_DATA2_DPU_MC_BC_DPU_DESC_OFFSET) |
            (table->dpuPTKSig << QWLAN_RXP_SEARCH_TABLE_DATA2_DPU_TAG_OFFSET) |
            (table->wepKeyIdxExtractEnable << QWLAN_RXP_SEARCH_TABLE_DATA2_WEP_KEY_ID_EXTRACT_ENABLE_OFFSET) |
            (table->dpuNE << QWLAN_RXP_SEARCH_TABLE_DATA2_DPU_NE_OFFSET) |
            (table->dpuIGTKDpuIdx << QWLAN_RXP_SEARCH_TABLE_DATA2_DPU_MAN_MC_BC_DESCRIPTOR_INDEX_OFFSET) |
            (table->dpuGTKSig << QWLAN_RXP_SEARCH_TABLE_DATA2_DPU_MC_BC_TAG_OFFSET));
    //HALLOGW( halLog( pMac, LOGW, FL("rxp search table2 register is 0x%x\n"),  value ));

    halWriteRegister(pMac, QWLAN_RXP_SEARCH_TABLE_DATA2_REG, value);

    // "search_table_cmd"
    value = QWLAN_RXP_SEARCH_TABLE_CMD_WRITE_MASK | index;
    //status should be zero before writing to command reg
    for(count = 0; count < RXP_CMD_MAX_RETRY; count++)
    {
        halReadRegister(pMac, QWLAN_RXP_SEARCH_TABLE_CMD_REG, &regValue);
        if(!(regValue &= QWLAN_RXP_SEARCH_TABLE_CMD_STATUS_MASK))
            break;
    }

    if(count < RXP_CMD_MAX_RETRY)
    {
        halWriteRegister(pMac, QWLAN_RXP_SEARCH_TABLE_CMD_REG, value);
    }
    else
    {
        HALLOGE( halLog(pMac, LOGE, FL("write_rxp_search_table_reg failed: cmd reg status is not zero before write\n")));
        return eHAL_STATUS_FAILURE;
    }

    return eHAL_STATUS_SUCCESS;
}


/* ---------------------------------------------------
 * FUNCTION:  sort_table()
 *
 * NOTE:
 *    Sort the table such that the mac addresses
 *    are in ascending order.
 * ----------------------------------------------------
 */
static void sort_table(tpAniSirGlobal pMac, tRxpAddrTable *table, tANI_U8 numOfEntry)
{
    tANI_U8         i, j;
    tRxpAddrTable   temp;

    for(i = (numOfEntry - 1); i > 0 ;i--)
    {
        for(j=0; i>j; j++)
        {
            if (compareMacAddrRev(pMac, table[j].macAddr, table[j+1].macAddr) == GREATER)
            {
               palCopyMemory(pMac->hHdd, (void *)&temp, (void *)&table[j], sizeof(tRxpAddrTable));
               palCopyMemory(pMac->hHdd, (void *)&table[j], (void *)&table[j+1], sizeof(tRxpAddrTable));
               palCopyMemory(pMac->hHdd, (void *)&table[j+1], (void *)&temp, sizeof(tRxpAddrTable));
            }
        }
    }
    // print_table(pMac, table, numOfEntry);
}


/* -------------------------------------------------
 * FUNCTION:  checkIfAddrExist()
 *
 * NOTE:
 *    Check to see if the mac address already exist
 *    in the search table.
 * --------------------------------------------------
 */
static tANI_U32 checkIfAddrExist(tpAniSirGlobal pMac, tSirMacAddr addr, tRxpAddrTable *table, tANI_U8 numOfEntry, tANI_U8* pEntryNum)
{
    tANI_U8   i;

    for(i=0; i<=numOfEntry; i++)
    {
        if (compareMacAddr(pMac, addr, table[i].macAddr) == SAME)
        {
            HALLOGW( halLog(pMac, LOGW, FL("Duplicate Entry \n")));
            *pEntryNum = i;
            return DUPLICATE_ENTRY;
        }
    }
    return ENTRY_NOT_FOUND;
}


/* -----------------------------------
 * FUNCTION:  compareMacAddrRev()
 *
 * NOTE:
 *    Comparing two mac addresses:
 *      if (addr1 > addr2)
 *          return GREATER
 *      else (addr1 < addr2)
 *          return LESS
 *      else (addr1 == addr2)
 *          return SAME
 *    But here byte0 of addr is LSB, byte5 is MSB
 * ------------------------------------
 */
static tANI_U8 compareMacAddrRev(tpAniSirGlobal pMac, tSirMacAddr addr1, tSirMacAddr addr2)
{
    tANI_U32  addr1_hi, addr2_hi;   // macAddr[47:16]
    tANI_U16  addr1_lo, addr2_lo;   // macAddr[15:0]

    addr1_hi = ( (((tANI_U32)addr1[5]) << 24) | (((tANI_U32)addr1[4]) << 16) |
                 (((tANI_U32)addr1[3]) << 8 ) | addr1[2] );

    addr2_hi = ( (((tANI_U32)addr2[5]) << 24) | (((tANI_U32)addr2[4]) << 16) |
                 (((tANI_U32)addr2[3]) << 8 ) | addr2[2] );

    addr1_lo = ( (((tANI_U16)addr1[1]) << 8) | addr1[0] );
    addr2_lo = ( (((tANI_U16)addr2[1]) << 8) | addr2[0] );

    if (addr1_hi == addr2_hi)
    {
        if (addr1_lo > addr2_lo)
            return GREATER;
        else if (addr1_lo < addr2_lo)
            return LESS;
        else // (addr1_lo == addr2_lo)
            return SAME;
    }
    else if (addr1_hi > addr2_hi)
        return GREATER;
    else  // (addr1_hi < addr2_hi)
        return LESS;
}



/* -----------------------------------
 * FUNCTION:  compareMacAddr()
 *
 * NOTE:
 *    Comparing two mac addresses:
 *      if (addr1 > addr2)
 *          return GREATER
 *      else (addr1 < addr2)
 *          return LESS
 *      else (addr1 == addr2)
 *          return SAME
 * ------------------------------------
 */
static tANI_U8 compareMacAddr(tpAniSirGlobal pMac, tSirMacAddr addr1, tSirMacAddr addr2)
{
    tANI_U32  addr1_hi, addr2_hi;   // macAddr[47:16]
    tANI_U16  addr1_lo, addr2_lo;   // macAddr[15:0]

    addr1_hi = ( (((tANI_U32)addr1[0]) << 24) | (((tANI_U32)addr1[1]) << 16) |
                 (((tANI_U32)addr1[2]) << 8 ) | addr1[3] );

    addr2_hi = ( (((tANI_U32)addr2[0]) << 24) | (((tANI_U32)addr2[1]) << 16) |
                 (((tANI_U32)addr2[2]) << 8 ) | addr2[3] );

    addr1_lo = ( (((tANI_U16)addr1[4]) << 8) | addr1[5] );
    addr2_lo = ( (((tANI_U16)addr2[4]) << 8) | addr2[5] );

    if (addr1_hi == addr2_hi)
    {
        if (addr1_lo > addr2_lo)
            return GREATER;
        else if (addr1_lo < addr2_lo)
            return LESS;
        else // (addr1_lo == addr2_lo)
            return SAME;
    }
    else if (addr1_hi > addr2_hi)
        return GREATER;
    else  // (addr1_hi < addr2_hi)
        return LESS;
}


/* -------------------------------------
 * FUNCTION:  print_table()
 *
 * NOTE:
 *    Print address table from index 0.
 * -------------------------------------
 */
static void print_table(tpAniSirGlobal pMac, tRxpAddrTable *table, tANI_U8 numOfEntry)
{
    tANI_U8    i;

    for (i=0; i < numOfEntry; i++)
    {
        HALLOGW( halLog(pMac, LOGW, FL("table entry[%d]: macAddr {%x %x %x %x %x %x}, staid {%d} \n"), i,
              table[i].macAddr[0], table[i].macAddr[1], table[i].macAddr[2],
              table[i].macAddr[3], table[i].macAddr[4], table[i].macAddr[5], table[i].staid));
    }
    return;
}

static tANI_U32 checkIfDupAddrExist(tpAniSirGlobal pMac, tANI_U32 staId, tRxpAddrTable *table,
                                                tANI_U8 numOfEntry, tANI_U8 *entryNum)
{
    tANI_U8   i;

    for(i=0; i<=numOfEntry; i++)
    {
        if (staId == table[i].staid)
        {
            *entryNum = i;
            return ENTRY_FOUND;
        }
    }
    return ENTRY_NOT_FOUND;
}

eHalStatus halRxp_UpdateEntry(tpAniSirGlobal pMac, tANI_U8 staid, tRxpRole role, tANI_BOOLEAN keyExtract)
{
    tANI_U32     value;
    tpRxpInfo  pRxp = GET_RXP_INFO(pMac);
    tANI_U8  addr1Found = FALSE, addr2Found = FALSE;
    tANI_U8 entryNum_self = 0, entryNum_sta = 0;

    if(pRxp == NULL) {
        HALLOGW( halLog(pMac, LOGW, FL(" pRXP is NULL\n")));
        return eHAL_STATUS_FAILURE;
    }

    switch(role) {
        case eRXP_SELF:
            {
                if (checkIfDupAddrExist(pMac, staid, pRxp->addr1_table, pRxp->addr1.numOfEntry,
                            &entryNum_self) == ENTRY_FOUND)
                {
                    pRxp->addr1_table[entryNum_self].wepKeyIdxExtractEnable = keyExtract;
                    addr1Found = TRUE;
                }
                break;
            }

        case eRXP_PEER_AP:
        case eRXP_PEER_STA:
            {
                if (checkIfDupAddrExist(pMac, staid, pRxp->addr2_table, pRxp->addr2.numOfEntry, &entryNum_sta) == ENTRY_FOUND)
                {
                    pRxp->addr2_table[entryNum_sta].wepKeyIdxExtractEnable = keyExtract;
                    addr2Found = TRUE;
                }
            }
            break;
        case eRXP_BSSID:
            break;
        default:
            HALLOGW( halLog(pMac, LOGW, FL("%s: received unknown rxp role = %d \n"), __FUNCTION__, role));
            return eHAL_STATUS_FAILURE;

    }

    /* ------------------------------
     *   Update Addr1 Table to memory
     * ------------------------------
     */
    if(addr1Found)
    {
        update_rxp_search_table_reg(pMac, &pRxp->addr1_table[entryNum_self], (pRxp->addr1.lowPtr + entryNum_self));

        value = QWLAN_RXP_SEARCH_ADDR1_PTR_ADDR1_POINTERS_VALID_MASK |
            (pRxp->addr1.highPtr << QWLAN_RXP_SEARCH_ADDR1_PTR_ADDR1_SEARCH_HI_PTR_OFFSET) | pRxp->addr1.lowPtr;
        halWriteRegister(pMac, QWLAN_RXP_SEARCH_ADDR1_PTR_REG, value);

    }

    /* ------------------------------
     *   Update Addr2 Table to memory
     * ------------------------------
     */
    if(addr2Found)
    {
        update_rxp_search_table_reg(pMac, &pRxp->addr2_table[entryNum_sta], (pRxp->addr2.lowPtr + entryNum_sta));

        value = QWLAN_RXP_SEARCH_ADDR2_PTR_ADDR2_POINTERS_VALID_MASK |
            (pRxp->addr2.highPtr << QWLAN_RXP_SEARCH_ADDR2_PTR_ADDR2_SEARCH_HI_PTR_OFFSET) | pRxp->addr2.lowPtr;
        halWriteRegister(pMac, QWLAN_RXP_SEARCH_ADDR2_PTR_REG, value);
    }

    return eHAL_STATUS_SUCCESS;
}

/*
 * FUNCTION: fillAddrTableEntry()
 *
 * Function to fill the values in the selected address table entry
 *
 */
eHalStatus fillAddrTableEntry(tpAniSirGlobal pMac, tRxpAddrTable* pRxpAddrTable,
        tANI_U8 staid, tSirMacAddr macAddr, tANI_U8 dropBit, tANI_U8 rmfBit,
        tANI_U8 dpuIdx, tANI_U8 dpuRFOrBcMcIdx, tANI_U8 dpuMgmtBcMcIdx,
        tANI_U8 dpuTag, tANI_U8 dpuDataBcMcTag, tANI_U8 dpuMgmtBcMcTag,
        tANI_U8 dpuNE, tANI_U8 ftBit, tANI_BOOLEAN keyIdExtract)
{

    palCopyMemory(pMac->hHdd,
                (void *)pRxpAddrTable->macAddr, (void *)macAddr,
                sizeof(tSirMacAddr));

    pRxpAddrTable->staid    = staid;
    pRxpAddrTable->dropBit  = dropBit;
    pRxpAddrTable->rmfBit   = rmfBit;
    pRxpAddrTable->ftBit    = ftBit;
    pRxpAddrTable->dpuPTKDescIdx    = dpuIdx;
    pRxpAddrTable->dpuGTKDpuIdx     = dpuRFOrBcMcIdx;
    pRxpAddrTable->dpuIGTKDpuIdx    = dpuMgmtBcMcIdx;
    pRxpAddrTable->dpuPTKSig        = dpuTag;
    pRxpAddrTable->dpuGTKSig        = dpuDataBcMcTag;
    pRxpAddrTable->dpuIGTKTag       = dpuMgmtBcMcTag;
    pRxpAddrTable->dpuNE            = dpuNE;
    pRxpAddrTable->wepKeyIdxExtractEnable = keyIdExtract;

    return eHAL_STATUS_SUCCESS;
}



/* ---------------------------------------------------
 * FUNCTION:  halRxp_AddEntry()
 *
 * NOTE:
 *    if ( AP and "role = myself (0)" )
 *      - add macAddr and staid entry to A1 table
 *      - set drop bit to 0
 *
 *    if ( STATION and "role = myself(0)" )
 *      - add macAddr and staid entry to A1 and A3 table
 *      - set drop bit to 1
 *
 *    if (role = peers (1))
 *      - add macAddr and staid entry to A2 and A3 table
 *      - set drop bit to 0
 *
 *    Write All table into RXP Binary Search Table.
 * ----------------------------------------------------
 */
eHalStatus halRxp_AddEntry(tpAniSirGlobal pMac, tANI_U8 staid, tSirMacAddr macAddr, tRxpRole role,
        tANI_U8 rmfBit, tANI_U8 dpuIdx, tANI_U8 dpuRFOrBcMcIdx, tANI_U8 dpuMgmtBcMcIdx,
        tANI_U8 dpuTag, tANI_U8 dpuDataBcMcTag, tANI_U8 dpuMgmtBcMcTag,
        tANI_U8 dpuNE, tANI_U8 ftBit, tANI_BOOLEAN keyIdExtract)
{
    tpRxpInfo  pRxp = GET_RXP_INFO(pMac);
    tANI_U8    addr1Added = FALSE, addr2Added = FALSE, addr3Added = FALSE;
    tANI_U8    entryUpdate = 0; //set when an entry exists and getting updated.
    tANI_U8    entryNum = 0;

    if(pRxp == NULL)
    {
        HALLOGW( halLog(pMac, LOGW, FL("pRXP is NULL\n")));
        return eHAL_STATUS_FAILURE;
    }

    HALLOGW( halLog(pMac, LOGW, FL("staid = %u, role = %u, rmfBit = %u, dpuIdx = %u, \
        dpuRFOrBcMcIdx = %u, dpuMgmtBcMcIdx = %u, dpuTag = %u, dpuDataBcMcTag = %u, \
        dpuMgmtBcMcTag = %u, dpuNE = %u, ftBit = %u, keyIdExtract = %u\n"),
        staid, role, rmfBit, dpuIdx, dpuRFOrBcMcIdx, dpuMgmtBcMcIdx, dpuTag,
        dpuDataBcMcTag, dpuMgmtBcMcTag, dpuNE, ftBit, keyIdExtract));

    switch(role)
    {
        case eRXP_SELF:
            {
                entryUpdate = 0;
                if(checkIfAddrExist(pMac, macAddr, pRxp->addr1_table, pRxp->addr1.numOfEntry, &entryNum) == DUPLICATE_ENTRY)
                {
                    entryUpdate = 1;
                }
                else
                {
                    entryNum = pRxp->addr1.numOfEntry; //entry num if new station is getting added.
                }
                // Fill entries in the Address 1 table
                if(fillAddrTableEntry(pMac, &pRxp->addr1_table[entryNum],
                            staid, macAddr, 0, rmfBit,
                            dpuIdx, dpuRFOrBcMcIdx, dpuMgmtBcMcIdx,
                            dpuTag, dpuDataBcMcTag, dpuMgmtBcMcTag,
                            dpuNE, ftBit, keyIdExtract) != eHAL_STATUS_SUCCESS) {
                    return eHAL_STATUS_FAILURE;
                }

                if(!entryUpdate)
                {
                    pRxp->addr1.numOfEntry++;

                    HALLOG1( halLog(pMac, LOG1, FL("====== Added to ADDR1 TABLE ======\n")));
                    sort_table(pMac, pRxp->addr1_table, pRxp->addr1.numOfEntry);
                }
                else{
                    HALLOG1( halLog(pMac, LOG1, FL("====== Updated to ADDR1 TABLE ======\n")));                    
                }
                addr1Added = TRUE;

                entryUpdate = 0;
                if(checkIfAddrExist(pMac, macAddr, pRxp->addr3_table, pRxp->addr3.numOfEntry, &entryNum) == DUPLICATE_ENTRY)
                {
                    entryUpdate = 1;
                }
                else
                {
                    entryNum = pRxp->addr3.numOfEntry; //entry num if new station is getting added.
                }


                // Fill entries in the Address 3 table
                if(fillAddrTableEntry(pMac, &pRxp->addr3_table[entryNum],
                            WLANHAL_RX_BD_ADDR3_SELF_IDX, macAddr, 0, 0,
                            0, 0, 0,
                            0, 0, 0,
                            0, 0, 0) != eHAL_STATUS_SUCCESS) {
                    return eHAL_STATUS_FAILURE;
                }
                if(!entryUpdate)
                {
                    pRxp->addr3.numOfEntry++;

                    HALLOGW(halLog(pMac, LOGW, FL("====== Added ADDR3 TABLE ======\n")));
                    sort_table(pMac, pRxp->addr3_table, pRxp->addr3.numOfEntry);
                }
                else{
                    HALLOGW( halLog(pMac, LOGW, FL("====== Updated ADDR3 TABLE ======\n")));                    
                }
                addr3Added = TRUE;


                break;
            }
        case eRXP_PEER_AP:
            {
                entryUpdate = 0;
                if(checkIfAddrExist(pMac, macAddr, pRxp->addr2_table, pRxp->addr2.numOfEntry, &entryNum) == DUPLICATE_ENTRY)
                {
                    entryUpdate = 1;
                }
                else
                {
                    entryNum = pRxp->addr2.numOfEntry; //entry num if new station is getting added.
                }
                // Fill entries in the Address 2 table
                if(fillAddrTableEntry(pMac, &pRxp->addr2_table[entryNum],
                            staid, macAddr, 0, rmfBit,
                            dpuIdx, dpuRFOrBcMcIdx, dpuMgmtBcMcIdx,
                            dpuTag, dpuDataBcMcTag, dpuMgmtBcMcTag,
                            dpuNE, ftBit, keyIdExtract) != eHAL_STATUS_SUCCESS) {
                    return eHAL_STATUS_FAILURE;
                }

                HALLOG1(halLog(pMac, LOG1, FL("Key Extract =0x%d\n"),
                        pRxp->addr2_table[pRxp->addr2.numOfEntry].wepKeyIdxExtractEnable));

                if(!entryUpdate)
                {
                    pRxp->addr2.numOfEntry++;
                    HALLOG1(halLog(pMac, LOG1, FL("====== Added to ADDR2 TABLE ======\n")));
                    sort_table(pMac, pRxp->addr2_table, pRxp->addr2.numOfEntry);
                }else{
                    HALLOG1(halLog(pMac, LOG1, FL("====== Updated to ADDR2 TABLE ======\n")));
                }

                addr2Added = TRUE;

                entryUpdate = 0;
                if(checkIfAddrExist(pMac, macAddr, pRxp->addr3_table, pRxp->addr3.numOfEntry, &entryNum) == DUPLICATE_ENTRY)
                {
                    entryUpdate = 1;
                }
                else
                {
                    entryNum = pRxp->addr3.numOfEntry; //entry num if new station is getting added.
                }


                // Fill entries in the Address 3 table
                if(fillAddrTableEntry(pMac, &pRxp->addr3_table[entryNum],
                            staid, macAddr, 0, rmfBit,
                            dpuIdx, dpuRFOrBcMcIdx, dpuMgmtBcMcIdx,
                            dpuTag, dpuDataBcMcTag, dpuMgmtBcMcTag,
                            dpuNE, ftBit, keyIdExtract) != eHAL_STATUS_SUCCESS) {
                    return eHAL_STATUS_FAILURE;
                }
                if(!entryUpdate)
                {
                    pRxp->addr3.numOfEntry++;

                    HALLOG1( halLog(pMac, LOG1, FL("====== Added ADDR3 TABLE ======\n")));
                    sort_table(pMac, pRxp->addr3_table, pRxp->addr3.numOfEntry);
                }
                else{
                    HALLOG1(halLog(pMac, LOG1, FL("====== Updated ADDR3 TABLE ======\n")));                    
                }
                addr3Added = TRUE;
            }
            break;
        case eRXP_PEER_STA:
            {
                entryUpdate = 0;
                if(checkIfAddrExist(pMac, macAddr, pRxp->addr2_table, pRxp->addr2.numOfEntry, &entryNum) == DUPLICATE_ENTRY)
                {
                    entryUpdate = 1;
                }
                else
                {
                    entryNum = pRxp->addr2.numOfEntry; //entry num if new station is getting added.
                }
                // Fill entries in the Address 2 table
                if(fillAddrTableEntry(pMac, &pRxp->addr2_table[entryNum],
                            staid, macAddr, 0, rmfBit,
                            dpuIdx, dpuRFOrBcMcIdx, dpuMgmtBcMcIdx,
                            dpuTag, dpuDataBcMcTag, dpuMgmtBcMcTag,
                            dpuNE, ftBit, keyIdExtract) != eHAL_STATUS_SUCCESS) {
                    return eHAL_STATUS_FAILURE;
                }
                if(!entryUpdate)
                {
                    pRxp->addr2.numOfEntry++;
                    HALLOG1( halLog(pMac, LOG1, FL("====== Added to ADDR2 TABLE ======\n")));
                    sort_table(pMac, pRxp->addr2_table, pRxp->addr2.numOfEntry);
                }
                else{
                    HALLOG1( halLog(pMac, LOG1, FL("====== Updated to ADDR2 TABLE ======\n")));
                }
                addr2Added = TRUE;


                entryUpdate = 0;
                if(checkIfAddrExist(pMac, macAddr, pRxp->addr3_table, pRxp->addr3.numOfEntry, &entryNum) == DUPLICATE_ENTRY)
                {
                    entryUpdate = 1;
                }
                else
                {
                    entryNum = pRxp->addr3.numOfEntry; //entry num if new station is getting added.
                }
                // Fill entries in the Address 3 table
                if(fillAddrTableEntry(pMac, &pRxp->addr3_table[entryNum],
                            staid, macAddr, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) != eHAL_STATUS_SUCCESS) {
                    return eHAL_STATUS_FAILURE;
                }

                if(!entryUpdate)
                {
                    pRxp->addr3.numOfEntry++;
                    HALLOG1( halLog(pMac, LOG1, FL("====== Added to ADDR3 TABLE ======\n")));
                    sort_table(pMac, pRxp->addr3_table, pRxp->addr3.numOfEntry);
                }
                else{
                    HALLOG1(halLog(pMac, LOG1, FL("====== Updated to ADDR3 TABLE ======\n")));
                }
                addr3Added = TRUE;
            }
            break;
        case eRXP_BSSID:
            {
                entryUpdate = 0;
                if(checkIfAddrExist(pMac, macAddr, pRxp->addr3_table, pRxp->addr3.numOfEntry, &entryNum) == DUPLICATE_ENTRY)
                {
                    entryUpdate = 1;
                }
                else
                {
                    entryNum = pRxp->addr3.numOfEntry; //entry num if new station is getting added.
                }
                // Fill entries in the Address 3 table
                if(fillAddrTableEntry(pMac, &pRxp->addr3_table[entryNum],
                            staid, macAddr, 0, rmfBit,
                            dpuIdx, dpuRFOrBcMcIdx, dpuMgmtBcMcIdx,
                            dpuTag, dpuDataBcMcTag, dpuMgmtBcMcTag,
                            dpuNE, ftBit, keyIdExtract) != eHAL_STATUS_SUCCESS) {
                    return eHAL_STATUS_FAILURE;
                }
                if(!entryUpdate)
                {
                    pRxp->addr3.numOfEntry++;

                    HALLOG1(halLog(pMac, LOG1, FL("====== Added to ADDR3 TABLE ======\n")));
                    sort_table(pMac, pRxp->addr3_table, pRxp->addr3.numOfEntry);
                }
                else{
                    HALLOG1(halLog(pMac, LOG1, FL("====== Updated to ADDR3 TABLE ======\n")));
                }
                addr3Added = TRUE;
            }
            break;
        default:
            HALLOGE(halLog(pMac, LOGE, FL("received unknown rxp role = %d \n"), role));
            return eHAL_STATUS_FAILURE;

    }


    halRxp_disable(pMac);

    /* ------------------------------
     *   Write Addr1 Table to memory
     * ------------------------------
     */
    if(addr1Added)
    {
        pRxp->addr1.lowPtr = 0;
        pRxp->addr1.highPtr = pRxp->addr1.numOfEntry - 1;
        if(writeAddrTable(pMac, pRxp->addr1_table, RXP_TABLE_ADDR1, RXP_TABLE_VALID,
                    pRxp->addr1.highPtr, pRxp->addr1.lowPtr, pRxp->addr1.numOfEntry) != eHAL_STATUS_SUCCESS) {

            /* Enable Rxp back, this would un-block all frames as per the filter */
            halRxp_enable(pMac);
            return eHAL_STATUS_FAILURE;
        }
    }

    /* ------------------------------
     *   Write Addr2 Table to memory
     * ------------------------------
     */
    if(addr1Added || addr2Added)
    {
        pRxp->addr2.lowPtr = pRxp->addr1.highPtr + 1;
        pRxp->addr2.highPtr = pRxp->addr2.lowPtr + pRxp->addr2.numOfEntry - 1;
        if(pRxp->addr2.numOfEntry != 0) {
            if(writeAddrTable(pMac, pRxp->addr2_table, RXP_TABLE_ADDR2, RXP_TABLE_VALID,
                        pRxp->addr2.highPtr, pRxp->addr2.lowPtr, pRxp->addr2.numOfEntry) != eHAL_STATUS_SUCCESS) 
            {
                /* Enable Rxp back, this would un-block all frames as per the filter */
                halRxp_enable(pMac);
                return eHAL_STATUS_FAILURE;
            }
        }
    }

    /* ------------------------------
     *   Write Addr3 Table to memory
     * ------------------------------
     */
    if(addr1Added || addr2Added || addr3Added)
    {
        pRxp->addr3.lowPtr = pRxp->addr2.highPtr + 1;
        pRxp->addr3.highPtr = pRxp->addr3.lowPtr + pRxp->addr3.numOfEntry - 1;
        if(pRxp->addr3.numOfEntry != 0) {
            writeAddrTable(pMac, pRxp->addr3_table, RXP_TABLE_ADDR3, RXP_TABLE_VALID,
                        pRxp->addr3.highPtr, pRxp->addr3.lowPtr, pRxp->addr3.numOfEntry);
        }
    }

    /* Enable Rxp back, this would un-block all frames as per the filter */
    halRxp_enable(pMac);
    return eHAL_STATUS_SUCCESS;
}

/*
 * FUNCTION: writeAddrTable()
 *
 * Write the Addr table into the HW registers and update the
 * low and high pointers to the table
 */
eHalStatus writeAddrTable(tpAniSirGlobal pMac, tRxpAddrTable* pTable, tANI_U8 addrType,
        tANI_U8 validTable, tANI_U8 highPtr, tANI_U8 lowPtr, tANI_U8 numOfEntry)
{
    tANI_U32 regAddr, mask, value;
    tANI_U8 offset;
    tANI_U8 i;

//    if (numOfEntry == 0) {
//        return eHAL_STATUS_SUCCESS;
//    }

    // Set the valid bit mask, the high pointer offset and the addr
    // table register address depending upon the type of address table,
    // addr1, addr2 or addr3.
    switch(addrType) {
        case RXP_TABLE_ADDR1:
            mask    = QWLAN_RXP_SEARCH_ADDR1_PTR_ADDR1_POINTERS_VALID_MASK;
            offset  = QWLAN_RXP_SEARCH_ADDR1_PTR_ADDR1_SEARCH_HI_PTR_OFFSET;
            regAddr = QWLAN_RXP_SEARCH_ADDR1_PTR_REG;
            break;
        case RXP_TABLE_ADDR2:
            mask    = QWLAN_RXP_SEARCH_ADDR2_PTR_ADDR2_POINTERS_VALID_MASK;
            offset  = QWLAN_RXP_SEARCH_ADDR2_PTR_ADDR2_SEARCH_HI_PTR_OFFSET;
            regAddr = QWLAN_RXP_SEARCH_ADDR2_PTR_REG;
            break;
        case RXP_TABLE_ADDR3:
            mask    = QWLAN_RXP_SEARCH_ADDR3_PTR_ADDR3_POINTERS_VALID_MASK;
            offset  = QWLAN_RXP_SEARCH_ADDR3_PTR_ADDR3_SEARCH_HI_PTR_OFFSET;
            regAddr = QWLAN_RXP_SEARCH_ADDR3_PTR_REG;
            break;
        default:
            HALLOGE( halLog(pMac, LOGE, FL("Invalid type of addr table %d\n"), addrType));
            return eHAL_STATUS_FAILURE;
    }

    // If table not valid
    if(!validTable) {
        mask = 0;
    }

    // Write the into the RXP search table registers
    for(i=0; i < numOfEntry; i++) {
        write_rxp_search_table_reg(pMac, &pTable[i], lowPtr + i);
    }
    HALLOG1( halLog( pMac, LOG1, FL("addr%d ptr: lowPtr = %d, highPtr = %d \n"),  addrType, lowPtr, highPtr ));

    // Update the table pointers in the hw register
    value = mask | (highPtr << offset) | lowPtr;
    halWriteRegister(pMac, regAddr, value);
    return eHAL_STATUS_SUCCESS;
}


eHalStatus searchAndDeleteTableEntry(tpAniSirGlobal pMac, tSirMacAddr macAddr,
        tANI_U8 addrType, tRxpAddrTable* pTable, tANI_U8 *pNumOfEntry, tANI_U8 *pFound)
            {
    tANI_U8 i, j;
    tANI_U8 numOfEntry = *pNumOfEntry;

    *pFound = 0;
    for (i=0; i<numOfEntry; i++) {
        if( (compareMacAddr(pMac, pTable[i].macAddr, macAddr) == SAME) ) {
            HALLOG1(halLog(pMac, LOG1, FL("Match found in entry %d of addr%d table\n"), i, addrType));

            // Shift all the entries below the match up by one
            for (j=i; j<numOfEntry - 1; j++) {
                if( palCopyMemory(pMac->hHdd,
                            (void *)&pTable[j], (void *)&pTable[j+1],
                            sizeof(tRxpAddrTable)) != eHAL_STATUS_SUCCESS) {
                    return eHAL_STATUS_FAILURE;
                }
            }
            // Clear the last entry in the table
            if( palZeroMemory(pMac->hHdd, (void *)&pTable[numOfEntry - 1],
                        sizeof(tRxpAddrTable)) != eHAL_STATUS_SUCCESS) {
                return eHAL_STATUS_FAILURE;
        }

            // Decrement the number of entries in the table
            *pNumOfEntry = --numOfEntry;
            *pFound = 1;

            return eHAL_STATUS_SUCCESS;
    }
    }

    return eHAL_STATUS_SUCCESS;
}

/* ---------------------------------------------------
 * FUNCTION:  halRxp_DelEntry()
 *
 * NOTE:
 *   Delete the mac address from the RXP Binary Search
 *   table. Afterwards, write the updated table into
 *   HW.
 * ----------------------------------------------------
 */
eHalStatus halRxp_DelEntry(tpAniSirGlobal pMac, tSirMacAddr macAddr)
{
    tANI_U8      j;
    tANI_U8      validTable;
    tANI_U8      addr1MatchFound = 0;
    tANI_U8      addr2MatchFound = 0;
    tANI_U8      addr3MatchFound = 0;
    tpRxpInfo    pRxp = GET_RXP_INFO(pMac);
    if(pRxp == NULL)
    {
      HALLOGW(halLog(pMac, LOGW, FL("pRXP is NULL\n")));
      return eHAL_STATUS_FAILURE;
    }

    if(searchAndDeleteTableEntry(pMac, macAddr, RXP_TABLE_ADDR1,
                pRxp->addr1_table, &pRxp->addr1.numOfEntry,
                &addr1MatchFound) != eHAL_STATUS_SUCCESS) {
                    return eHAL_STATUS_FAILURE;
            }

    if(searchAndDeleteTableEntry(pMac, macAddr, RXP_TABLE_ADDR2,
                pRxp->addr2_table, &pRxp->addr2.numOfEntry,
                &addr2MatchFound) != eHAL_STATUS_SUCCESS) {
                    return eHAL_STATUS_FAILURE;
            }

    if(searchAndDeleteTableEntry(pMac, macAddr, RXP_TABLE_ADDR3,
                pRxp->addr3_table, &pRxp->addr3.numOfEntry,
                &addr3MatchFound) != eHAL_STATUS_SUCCESS) {
            return eHAL_STATUS_FAILURE;
        }

    if (!(addr1MatchFound || addr2MatchFound || addr3MatchFound))
    {
        HALLOGW(halLog(pMac, LOGW, FL("Match not found!\n")));
        return eHAL_STATUS_FAILURE;
    }

    /* ------------------------------
     *   Write Addr1 Table to memory
     * ------------------------------
     */
    if(addr1MatchFound)
    {
        pRxp->addr1.lowPtr = 0;

        if(pRxp->addr1.numOfEntry != 0) {
          pRxp->addr1.highPtr = pRxp->addr1.numOfEntry - 1;
            validTable = RXP_TABLE_VALID;
        } else {
          pRxp->addr1.highPtr = pRxp->addr1.lowPtr;
            validTable = RXP_TABLE_EMPTY;
        }

        writeAddrTable(pMac, pRxp->addr1_table, RXP_TABLE_ADDR1, validTable,
                    pRxp->addr1.highPtr, pRxp->addr1.lowPtr, pRxp->addr1.numOfEntry);
    }

    /* ------------------------------
     *   Write Addr2 Table to memory
     * ------------------------------
     */
      if(addr1MatchFound || addr2MatchFound)
      {
        pRxp->addr2.lowPtr = pRxp->addr1.highPtr + 1;
        if(pRxp->addr2.numOfEntry != 0) {
          pRxp->addr2.highPtr = pRxp->addr2.lowPtr + pRxp->addr2.numOfEntry - 1;
            validTable = RXP_TABLE_VALID;
        } else {
          pRxp->addr2.highPtr = pRxp->addr2.lowPtr = pRxp->addr1.highPtr;
            validTable = RXP_TABLE_EMPTY;
        }

        writeAddrTable(pMac, pRxp->addr2_table, RXP_TABLE_ADDR2, validTable,
                    pRxp->addr2.highPtr, pRxp->addr2.lowPtr, pRxp->addr2.numOfEntry);
    }

    /* ------------------------------
     *   Write Addr3 Table to memory
     * ------------------------------
     */
      pRxp->addr3.lowPtr = pRxp->addr2.highPtr + 1;
    if(pRxp->addr3.numOfEntry != 0) {
        pRxp->addr3.highPtr = pRxp->addr3.lowPtr + pRxp->addr3.numOfEntry - 1;
        validTable = RXP_TABLE_VALID;
    } else {
        pRxp->addr3.highPtr = pRxp->addr3.lowPtr = pRxp->addr2.highPtr;
        validTable = RXP_TABLE_EMPTY;
      }
    writeAddrTable(pMac, pRxp->addr3_table, RXP_TABLE_ADDR3, validTable,
                pRxp->addr3.highPtr, pRxp->addr3.lowPtr, pRxp->addr3.numOfEntry);

      //deleting (invalidating) by number of entries that we have shifted above
      for(j = 0;j < (addr1MatchFound + addr2MatchFound + addr3MatchFound); j++)
      {
        delete_rxp_search_table_entry(pMac,(pRxp->addr3.highPtr+ j + 1));
      }

    return eHAL_STATUS_SUCCESS;
}


/*
  This function will delete (invalidate) all the entries.
*/
eHalStatus halRxp_DelAllEntries(tpAniSirGlobal pMac)
{
    tANI_U8      i;
    tANI_U32    value;
    eHalStatus  status = (eHalStatus)eHAL_STATUS_SUCCESS;
    tpRxpInfo    pRxp = GET_RXP_INFO(pMac);

    if(pRxp == NULL)
    {
      HALLOGE(halLog(pMac, LOGE, FL("pRXP is NULL\n")));
      return eHAL_STATUS_FAILURE;
    }
    for(i=0; i < pRxp->addr1.numOfEntry; i++)
    {
      if( palZeroMemory(pMac->hHdd,
          (void *)&pRxp->addr1_table[i],
          sizeof(tRxpAddrTable)) != eHAL_STATUS_SUCCESS)
        return eHAL_STATUS_FAILURE;
    }
    for(i=0; i < pRxp->addr2.numOfEntry; i++)
    {
      if( palZeroMemory(pMac->hHdd,
          (void *)&pRxp->addr2_table[i],
          sizeof(tRxpAddrTable)) != eHAL_STATUS_SUCCESS)
        return eHAL_STATUS_FAILURE;
    }
    for(i=0; i < pRxp->addr3.numOfEntry; i++)
    {
      if( palZeroMemory(pMac->hHdd,
          (void *)&pRxp->addr3_table[i],
          sizeof(tRxpAddrTable)) != eHAL_STATUS_SUCCESS)
        return eHAL_STATUS_FAILURE;
    }

    //Deleting (invalidating) all entries
    for (i=0; i<RXP_MAX_TABLE_ENTRY; i++)
    {
        delete_rxp_search_table_entry(pMac, i);
        HALLOG4(halLog(pMac, LOG4, FL("deleted entry %d in address table 1\n"), i));
    }
    pRxp->addr1.numOfEntry = 0;
    pRxp->addr1.highPtr = pRxp->addr1.lowPtr = 0;

    pRxp->addr2.numOfEntry = 0;
    pRxp->addr2.highPtr = pRxp->addr2.lowPtr = 0;

    pRxp->addr3.numOfEntry = 0;
    pRxp->addr3.highPtr = pRxp->addr3.lowPtr = 0;

    value = (pRxp->addr1.highPtr << QWLAN_RXP_SEARCH_ADDR1_PTR_ADDR1_SEARCH_HI_PTR_OFFSET) | pRxp->addr1.lowPtr;
    halWriteRegister(pMac, QWLAN_RXP_SEARCH_ADDR1_PTR_REG, value);

    value = (pRxp->addr2.highPtr << QWLAN_RXP_SEARCH_ADDR2_PTR_ADDR2_SEARCH_HI_PTR_OFFSET) | pRxp->addr2.lowPtr;
    halWriteRegister(pMac, QWLAN_RXP_SEARCH_ADDR2_PTR_REG, value);

    value = (pRxp->addr3.highPtr << QWLAN_RXP_SEARCH_ADDR3_PTR_ADDR3_SEARCH_HI_PTR_OFFSET) | pRxp->addr3.lowPtr;
    halWriteRegister(pMac, QWLAN_RXP_SEARCH_ADDR3_PTR_REG, value);

    return status;
}



/* -----------------------------------------------------------
 * FUNCTION:  halRxp_enable()
 *
 * NOTE:
 *   Enable RXP to start receiving packet from PHY. This is
 *   done by setting the "cfg_rxp_en" bit to 1 in the RXP
 *   CONFIG register.
 *
 *   Have all packets go through the address filtering. This
 *   is done by setting the "cfg_addr_filter_en" bit to 1 in
 *   the RXP CONFIG register.
 * -----------------------------------------------------------
 */
eHalStatus halRxp_enable(tpAniSirGlobal pMac)
{
    tANI_U32   value;

    HALLOG1( halLog(pMac, LOG1, FL("Set cfg_rxp_en & cfg_addr_filter_en to 1 \n")));
    halReadRegister(pMac, QWLAN_RXP_CONFIG_REG, &value);

    value |= (QWLAN_RXP_CONFIG_CFG_HAS_PHY_CMD_MASK | QWLAN_RXP_CONFIG_CFG_RXP_EN_MASK | QWLAN_RXP_CONFIG_CFG_ADDR_FILTER_EN_MASK);
    halWriteRegister(pMac, QWLAN_RXP_CONFIG_REG, value);

    return eHAL_STATUS_SUCCESS;
}


/* ----------------------------------------------------------
 * FUNCTION:  halRxp_disable()
 *
 * NOTE:
 *   Disable RXP so it will not receive any packet from PHY.
 *
 *   Disable address filter so that all packets will be
 *   treated as if it passes the Address Filtering.
 * ----------------------------------------------------------
 */
eHalStatus halRxp_disable(tpAniSirGlobal pMac)
{
    tANI_U32   value;

    HALLOG1( halLog(pMac, LOG1, FL("Set cfg_rxp_en & cfg_addr_filter_en to 0 \n")));
    halReadRegister(pMac, QWLAN_RXP_CONFIG_REG, &value);

    value &= ~(QWLAN_RXP_CONFIG_CFG_HAS_PHY_CMD_MASK| QWLAN_RXP_CONFIG_CFG_RXP_EN_MASK | QWLAN_RXP_CONFIG_CFG_ADDR_FILTER_EN_MASK);
    halWriteRegister(pMac, QWLAN_RXP_CONFIG_REG, value);

    return eHAL_STATUS_SUCCESS;
}


/* ----------------------------------------------------------
 * FUNCTION:  halRxp_setOperatingRfBand()
 *
 * NOTE:
 *   Enable/Disable Rxp pktdet_n signal extension.
 *
 *   When operating in 2.4G, this feature should be enabled so RxP auto
 *   extends pktdet by preprogrammed value (in unit of usec)
 *
 * ----------------------------------------------------------
 */

eHalStatus halRxp_setOperatingRfBand(tpAniSirGlobal pMac, eRfBandMode rfBand)
{

	tANI_U32 value;

    if(rfBand != eRF_BAND_2_4_GHZ ){
        return eHAL_STATUS_FAILURE;
    }

	/* enable pktdet signal extension in in 2.4G band*/
	halReadRegister(pMac, QWLAN_RXP_CCA_AND_EXT_TIMEOUT_REG, &value);
	value |=  QWLAN_RXP_CCA_AND_EXT_TIMEOUT_CCA_PKDET_EXTENTION_ALLOWED_MASK;
	halWriteRegister(pMac, QWLAN_RXP_CCA_AND_EXT_TIMEOUT_REG, value);
	
	return eHAL_STATUS_SUCCESS;
}



/* -------------------------------------------
 * FUNCTION:  halRxpDbg_PrintSearchTable()
 *
 * NOTE:
 *    Display all 3 address table
 * -------------------------------------------
 */
void halRxpDbg_PrintSearchTable(tpAniSirGlobal pMac)
{
    tpRxpInfo pRxp = GET_RXP_INFO(pMac);

    HALLOGW( halLog(pMac, LOGW, FL("\n===============\nAddress 1 Table\n===============\n")));
    print_table(pMac, pRxp->addr1_table, pRxp->addr1.numOfEntry);

    HALLOGW(halLog(pMac, LOGW, FL("\n===============\nAddress 2 Table\n===============\n")));
    print_table(pMac, pRxp->addr2_table, pRxp->addr2.numOfEntry);

    HALLOGW( halLog(pMac, LOGW, FL("\n===============\nAddress 3 Table\n===============\n")));
    print_table(pMac, pRxp->addr3_table, pRxp->addr3.numOfEntry);
}

/* -------------------------------------------------
 * FUNCTION:  halRxpDbg_PrintFilter()
 *
 * NOTE:
 *   Print RXP Filter mask for index range [0, 47].
 *   Check if the register value match with the
 *   what's specified in the table.
 * -------------------------------------------------
 */
eHalStatus halRxpDbg_PrintFilter(tpAniSirGlobal pMac)
{
    tANI_U32  i = 0, readValue = 0, addr = 0;

    HALLOGW( halLog(pMac, LOGW, FL(" ---------------------------- \n")));
    HALLOGW( halLog(pMac, LOGW, FL("  RXP FILTER for All mode\n")));
    HALLOGW( halLog(pMac, LOGW, FL(" ---------------------------- \n")));

    for (i=0; i <= eDATA_QOS_CFACK_CFPOLL; i++)
    {
        addr = QWLAN_RXP_FRAME_FILTER_CONFIG_REG + (i * 4);
        halReadRegister(pMac, addr, &readValue);
        HALLOGW( halLog( pMac, LOGW, FL("\t\t (%-2d) %-25s:  (regAddr 0x%x) = 0x%x \n"),  i, pktType[i].frameName, addr, readValue ));

        if (readValue != rxpFilterTable_AllMode[i].configValue) {
            HALLOGW( halLog(pMac, LOGW, FL("Scan Mode: For %s, expect register value 0x%x, but get 0x%x \n"),
                   pktType[i].frameName, rxpFilterTable_AllMode[i].configValue, readValue));
            break;
		}
    }
    return eHAL_STATUS_SUCCESS;
}

/* ----------------------------------------------------------
 * FUNCTION:  halRxp_addBroadcastEntry()
 *
 * NOTE:
 *   Add broadcast address entry into the RXP Search Table.
 *   This is needed so that Softmac will receive broadcast
 *   packets.
 * ----------------------------------------------------------
 */
eHalStatus halRxp_addBroadcastEntry(tpAniSirGlobal pMac)
{
    tSirMacAddr    bcastAddr = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
    tANI_U8 ftBit;

    // FIXME_GEN6: 11w implementation and fill DPU signature for data MC/BC
    tANI_U8 rmfBit = 0;
    tANI_U8 dpuIdx = HAL_DPU_SELF_STA_DEFAULT_IDX;
    tANI_U8 dpuTag = 0;

    // Get the frame translation setting
    ftBit = halGetFrameTranslation(pMac);
    halDpu_GetSignature(pMac, dpuIdx, &dpuTag);
    if (halRxp_AddEntry(
                pMac, BROADCAST_STAID, bcastAddr, eRXP_SELF,
                rmfBit, dpuIdx, dpuIdx, dpuIdx, dpuTag, dpuTag,
                dpuTag, 0, ftBit, 0) != eHAL_STATUS_SUCCESS)
          return eHAL_STATUS_FAILURE;

    return eHAL_STATUS_SUCCESS;
}

static void
halRxpDbg_stats_clear (tpAniSirGlobal pMac)
{
    (void) halWriteRegister(pMac, QWLAN_RXP_CLEAR_STATS_REG, 1);
}

static void
halRxpDbg_stats_dump (tpAniSirGlobal pMac)
{
    tRxpRegList stats[] = {
        MKENTRY(QWLAN_RXP_PHY_AMPDU_CNT_REG),
        MKENTRY(QWLAN_RXP_PHY_ABORT_CNT_REG),
        MKENTRY(QWLAN_RXP_PHY_SHUTOFF_CNT_REG),
        MKENTRY(QWLAN_RXP_DLM_FIFO_FULL_CNT_REG),
        MKENTRY(QWLAN_RXP_DLM_ERR_CNT_REG),
        MKENTRY(QWLAN_RXP_FAIL_FILTER_CNT_REG),
        MKENTRY(QWLAN_RXP_FAIL_MAX_PKTLEN_CNT_REG),
        MKENTRY(QWLAN_RXP_FCS_ERR_CNT_REG),
        MKENTRY(QWLAN_RXP_DMA_SEND_CNT_REG),
        MKENTRY(QWLAN_RXP_DMA_DROP_CNT_REG),
        MKENTRY(QWLAN_RXP_DMA_GET_BMU_FAIL_CNT_REG),
        MKENTRY(QWLAN_RXP_PROTOCOL_VERSION_FILTER_CNT_REG),
        MKENTRY(QWLAN_RXP_TYPE_SUBTYPE_FILTER_CNT_REG),
        MKENTRY(QWLAN_RXP_INCORRECT_LENGTH_FILTER_CNT_REG),
        MKENTRY(QWLAN_RXP_ADDR1_BLOCK_FILTER_CNT_REG),
        MKENTRY(QWLAN_RXP_ADDR1_HIT_NO_PASS_CNT_REG),
        MKENTRY(QWLAN_RXP_ADDR1_DROP_CNT_REG),
        MKENTRY(QWLAN_RXP_ADDR2_HIT_NO_PASS_CNT_REG),
        MKENTRY(QWLAN_RXP_ADDR2_DROP_CNT_REG),
        MKENTRY(QWLAN_RXP_ADDR3_HIT_NO_PASS_CNT_REG),
        MKENTRY(QWLAN_RXP_ADDR3_DROP_CNT_REG),
        MKENTRY(QWLAN_RXP_MPDU_IN_AMPDU_CNT_REG),
        MKENTRY(QWLAN_RXP_PHY_ERR_DROP_CNT_REG),
        MKENTRY(QWLAN_RXP_START_ERR_CNT_REG),
        MKENTRY(QWLAN_RXP_TIMEOUT_ERROR_CNT_REG),
        MKENTRY(QWLAN_RXP_STALL_TIMEOUT_CNT_REG),
    };
    tANI_U32 i;
    tRxpRegList *pStats = &stats[0];

    for (i=0; i< sizeof(stats)/sizeof(stats[0]); i++, pStats++)
    {
        tANI_U32 value = 0;
        (void) halReadRegister(pMac, pStats->addr, &value);
        HALLOGW( halLog( pMac, LOGW, FL("%-50s (0x%08x)\t0x%08x\n"),  pStats->name, pStats->addr, value ));
    }
}

/* ----------------------------------------------------------
 * FUNCTION: halRxpDbg_dumpStats()
 *
 * NOTE:
 *   Dump all RXP stats registers
 * ----------------------------------------------------------
 */
void halRxpDbg_dumpReg( tpAniSirGlobal pMac, tANI_U32 arg1)
{
    switch (arg1)
    {
        case 1: halRxpDbg_stats_clear(pMac); break;
        // case 2: halRxpDbg_config_dump(pMac); break;
        case 0: /* fall through */
        default: halRxpDbg_stats_dump(pMac); break;
    }
}

/** -----------------------------------------------------
\fn     halRxp_setRxpFilterMode
\brief  This function is called by halMsg_setLinkState to
\       set the rxp filter mode. If its of IDLE, PREASSOC,
\       POSTASSOC, or AP mode, then HAL needs to keep a
\       record of it and set it. Else, HAL can just set it.
\param  tpAniSirGlobal pMac
\param  tRxpMode       mode
\return none.
  --------------------------------------------------------*/
void halRxp_setRxpFilterMode(tpAniSirGlobal pMac, tRxpMode rxpMode, tSirMacAddr macAddr)
{
    HALLOGW( halLog(  pMac, LOGW, FL("Set RxP mode = %d \n"),  rxpMode ));

    switch (rxpMode)
    {
        case eRXP_IDLE_MODE:
            // Delete the A2 RXP filter (if any) for filtering out the beacons.
            halRxp_RemovePreAssocAddr2Entry(pMac, macAddr);
            halRxp_storeRxpMode(pMac, rxpMode);
            break;
        case eRXP_PRE_ASSOC_MODE:
            // Add the A2 RXP filter to accept only beacons from the BSS to which
            // we are going to connect.
            halRxp_AddPreAssocAddr2Entry(pMac, macAddr);
            halRxp_storeRxpMode(pMac, rxpMode);
            break;
        case eRXP_POST_ASSOC_MODE:
        case eRXP_AP_MODE:
        case eRXP_PROMISCUOUS_MODE:
        case eRXP_IBSS_MODE:
            halRxp_storeRxpMode(pMac, rxpMode);
            break;

        case eRXP_SCAN_MODE:
        case eRXP_POWER_SAVE_MODE:
        case eRXP_LEARN_MODE:
        case eRXP_FTM_MODE:
            break;

        default:
            HALLOGE( halLog(pMac, LOGE, FL("invalid rxpMode %d \n"), rxpMode));
            break;
    }

    if ( halRxp_SetFilter(pMac, rxpMode) != eHAL_STATUS_SUCCESS)
        HALLOGE( halLog( pMac, LOGE, FL("halRxp_setFilter(%d) failed \n"),  rxpMode ));

    if ( halRxp_enable(pMac) != eHAL_STATUS_SUCCESS )
         HALLOGE( halLog( pMac, LOGE, FL("Failed to set ENABLE RXP \n")));

    return;
}

/** ----------------------------------------------------------
\fn     setRxFrameDisableRegs
\brief  This function is called by halRxp_setFilter() to set
\       the receive frame subtype filter to disable the respective type/susbtype.
\param  tpAniSirGlobal pMac
\param  tANI_U32       regLo  : value to be programmed in low register
\param  tANI_U32       regHi  : value to be programmed in Hi register
\return none
\ ------------------------------------------------------------ */
static void setRxFrameDisableRegs( tpAniSirGlobal pMac, tANI_U32 regLo, tANI_U32 regHi )
{
    HALLOG1( halLog(pMac, LOG1, FL(" regLo = 0x%x  regHi = 0x%x \n"), regLo, regHi));
    halWriteRegister(pMac, QWLAN_RXP_CFG_FLT_TYPE_SUBTYPE_RX_DISABLE0_REG, regLo  | HAL_RXP_TYPE_SUBTYPE_MASK);
    halWriteRegister(pMac, QWLAN_RXP_CFG_FLT_TYPE_SUBTYPE_RX_DISABLE1_REG, regHi);
}



void halRxp_SetTsfCompensationValues(tpAniSirGlobal pMac)
{

    tANI_U32 value;

    value = HAL_11B_1MBPS_TSF_COMPENSATION_VALUE |
            (HAL_11B_2MBPS_TSF_COMPENSATION_VALUE << QWLAN_RXP_TSF_COMPENSATION1_VALUE_TSF_11B_2MBPS_COMP_VALUE_OFFSET) |
            (HAL_11B_5_5MBPS_TSF_COMPENSATION_VALUE << QWLAN_RXP_TSF_COMPENSATION1_VALUE_TSF_11B_5_5MBPS_COMP_VALUE_OFFSET) |
            (HAL_11B_11MBPS_TSF_COMPENSATION_VALUE << QWLAN_RXP_TSF_COMPENSATION1_VALUE_TSF_11B_11MBPS_COMP_VALUE_OFFSET);

    halWriteRegister(pMac, QWLAN_RXP_TSF_COMPENSATION1_VALUE_REG, value);
    value = HAL_11AG_6MBPS_TSF_COMPENSATION_VALUE |
            (HAL_11AG_54MBPS_TSF_COMPENSATION_VALUE << QWLAN_RXP_TSF_COMPENSATION2_VALUE_TSF_11AG_54MBPS_COMP_VALUE_OFFSET) |
            (HAL_11AG_OTHER_TSF_COMPENSATION_VALUE << QWLAN_RXP_TSF_COMPENSATION2_VALUE_TSF_11AG_OTHER_RATE_COMP_VALUE_OFFSET);

    halWriteRegister(pMac, QWLAN_RXP_TSF_COMPENSATION2_VALUE_REG, value);
}

eHalStatus halRxp_ErrIntHandler(tHalHandle hHalHandle, eHalIntSources intSource)
{
    tANI_U32 intRegMask;
    tANI_U32 intRegStatus;
    HALLOGE( tpAniSirGlobal pMac = PMAC_STRUCT(hHalHandle));

    /** Read Interrupt Status.*/
    halIntGetErrorStatus(hHalHandle, intSource, &intRegStatus, &intRegMask);

    intRegStatus &= intRegMask;

    if(intRegStatus){
        /** Display Error Information.*/
        HALLOGE( halLog( pMac, LOGE, FL("Rxp Error Interrupt Status %x, enable %x\n"),  intRegStatus, intRegMask ));
    }

    return (eHAL_STATUS_SUCCESS);
}

eHalStatus halRxp_EnableDisableBmuBaUpdate(tpAniSirGlobal pMac, tANI_U32 enable)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tANI_U32 val;

    // Set BMU BA update enabled bit
    halReadRegister(pMac, QWLAN_RXP_CONFIG2_REG, &val);
    if(enable)
        val |= QWLAN_RXP_CONFIG2_CFG_MPR_BMU_BA_UPDATE_ENABLED_MASK;
    else
        val &= ~QWLAN_RXP_CONFIG2_CFG_MPR_BMU_BA_UPDATE_ENABLED_MASK;

    halWriteRegister(pMac, QWLAN_RXP_CONFIG2_REG, val);
        return status;
}


/** ----------------------------------------------------------
\fn     halRxp_EnableBssBeaconParamFilter
\brief  This function is called by halMsg_AddBss() to setup beacon
        related parameter filters.
        FIXME_GEN6: This function now only support single BSS: IBSS or STA mode
        When multiple BSS coexistnece (BTAMP) needs to be supported, it needs to be revised.
\param  tpAniSirGlobal  pMac
\return status
\ ------------------------------------------------------------ */

eHalStatus halRxp_EnableBssBeaconParamFilter( tpAniSirGlobal pMac, tANI_U32 uBssIdx, tANI_U8 staType )
{
    tpBssStruct pBss = (tpBssStruct) pMac->hal.halMac.bssTable;

    tANI_U32 bssidLo, bssidHi, tsfEnable = 1, timEnable = 0, tsfMode = 0, simIbssInfra = 0;
    tANI_U16 aid = 0, addr2Enable=0, addr3Enable=0, bcastEnable=0;
    tANI_U32 regVal = 0;

    pBss = &pBss[uBssIdx];

    //BSSID Has to be in little endian format.
    bssidLo = (pBss->bssId[0]) | (pBss->bssId[1] << 8) | (pBss->bssId[2] << 16) | (pBss->bssId[3] << 24);
    bssidHi = (pBss->bssId[4]) | (pBss->bssId[5] << 8);


    //FIXME_GEN6: Fixme when BTAMP/STA or IBSS/STA coexistence is enabled.
    //Basically in Gen6 HW, simultaneous IBSS/Infra bit should be set if and only if IBSS is active.
    //Only when this bit is set could HW differentiate IBSS beacons from Infra beacons and update TSF timer correctly.
    //When there is no IBSS, this bit should not be set. Otherwise Beacon mode power save would not work.

    //More details:
    //  1) in Libra, to support simul IBSS/Infra, MTU adds second TSF timer. When simul IBSS/Infra bit in RxP is enabled,
    //  the new TSF timer 0x0e0295d8 is used for Infra BSS and the old TSF timer 0x0e029418 is used by IBSS.
    //  when that bit is not set, either IBSS or Infra beacon updates the old TSF timer.
    //  2) When system in BPS mode, PMU restores the old TSF at every listen interval. It doesn't know when simul IBSS/Infra
    //  is enabled, Infra BSS should use second TSF timer. This implies IBSS just couldn't coexist with BPS at all.


    //FIXME_GEN6: When multiple BSS change comes in, Anu needs to make the following changes:
    // If IBSS exists, always enable the simul IBSS/Infra change.
    // If IBSS doesn't exist, don't enable simul IBSS/Infra bit.

    //For IBSS, HAL programs the BSSID into Addr2 registers.
    //For Infra, HAL programs BSSID to both Addr2 and Addr3 filters.
    if(pMac->hal.halSystemRole == eSYSTEM_STA_IN_IBSS_ROLE){
        tsfMode = eRXP_TSF_MODE_IBSS;
        //CR-0000142146 for Libra 1.0        
        //sim infra and ibss mode does not work for IBSS beacon filtering. Hence its left out

        // For the peer sta we dont need to do any setup. Everything is done by adding sta in add bss. 
        if (staType == STA_ENTRY_PEER)
        {
            halReadRegister(pMac, QWLAN_RXP_BEACON_TSF_TIM_EXTRACT_CTRL_REG, &regVal);
            HALLOG1(halLog(pMac, LOG1, FL("RXP unaltered setting = %08x\n"), regVal));
            return eHAL_STATUS_SUCCESS;
        }
        //FIXME : The workaround code for this issue is spread over multiple places. 
        //            one suggestion during review was to take out the chip revision check from the logic below
        //            and revert the settings from the if block below in routine halRxp_EnableSSIDBasedFilter()
        //             in that way the whole workaround will be limited to single routine halRxp_EnableSSIDBasedFilter() and this routine
        //             can easily be taken out in futher genertaion code.
        //             Not making this change at this moment because of extra testing effort when this code has already been validated for
        //             Libra 2.0. Should be taken care of in any next check-in opportunity which exercises this path and includes related testing.
        if (halGetChipRevNum(pMac) == LIBRA_CHIP_REV_ID_2_0) {        
            if(staType == STA_ENTRY_BSSID) {
                simIbssInfra = QWLAN_RXP_BEACON_TSF_TIM_EXTRACT_CTRL_SIM_IBSS_BSS_BEACON_PARS_EN_MASK;            
                halWriteRegister(pMac, QWLAN_RXP_BEACON_ADDR3_LOW_FIELD_REG, (tANI_U32)bssidLo);
                halWriteRegister(pMac, QWLAN_RXP_BEACON_ADDR3_HIGH_FIELD_REG, (tANI_U32)bssidHi);
                addr3Enable = QWLAN_RXP_BEACON_TSF_TIM_EXTRACT_CTRL_ENABLE_BEACON_ADDR3_CHECK_MASK;
                
                bcastEnable = QWLAN_RXP_BEACON_TSF_TIM_EXTRACT_CTRL_ENABLE_BEACON_BROADCAST_CHECK_MASK;
            }
        }
    }else if (eSYSTEM_STA_ROLE == pMac->hal.halSystemRole){
        timEnable = 1; /* enable TIM extraction in MTU */
        tsfMode = eRXP_TSF_MODE_STA;
        halTable_GetStaAssocId(pMac, (tANI_U8)pMac->hal.halMac.selfStaId, &aid);
        halWriteRegister(pMac, QWLAN_RXP_BEACON_ADDR2_LOW_FIELD_REG, (tANI_U32)bssidLo);
        halWriteRegister(pMac, QWLAN_RXP_BEACON_ADDR2_HIGH_FIELD_REG, (tANI_U32)bssidHi);
        addr2Enable = QWLAN_RXP_BEACON_TSF_TIM_EXTRACT_CTRL_ENABLE_BEACON_ADDR2_CHECK_MASK;

        halWriteRegister(pMac, QWLAN_RXP_BEACON_ADDR3_LOW_FIELD_REG, (tANI_U32)bssidLo);
        halWriteRegister(pMac, QWLAN_RXP_BEACON_ADDR3_HIGH_FIELD_REG, (tANI_U32)bssidHi);
        addr3Enable = QWLAN_RXP_BEACON_TSF_TIM_EXTRACT_CTRL_ENABLE_BEACON_ADDR3_CHECK_MASK;

        bcastEnable = QWLAN_RXP_BEACON_TSF_TIM_EXTRACT_CTRL_ENABLE_BEACON_BROADCAST_CHECK_MASK;
    }else{
        return eHAL_STATUS_FAILURE;
    }


    regVal |= (simIbssInfra | addr2Enable | addr3Enable | bcastEnable |
               ( tsfEnable << QWLAN_RXP_BEACON_TSF_TIM_EXTRACT_CTRL_ENABLE_BEACON_TSF_PARSING_OFFSET ) |
               ( timEnable << QWLAN_RXP_BEACON_TSF_TIM_EXTRACT_CTRL_ENABLE_BEACON_TIM_PARSING_OFFSET ) |
               ( tsfMode << QWLAN_RXP_BEACON_TSF_TIM_EXTRACT_CTRL_BEACON_TSF_PROCESSING_MODE_OFFSET ) |
               ( aid <<  QWLAN_RXP_BEACON_TSF_TIM_EXTRACT_CTRL_ASSOCIATION_ID_OFFSET ));

    halWriteRegister(pMac, QWLAN_RXP_BEACON_TSF_TIM_EXTRACT_CTRL_REG, regVal);
    HALLOG1(halLog(pMac, LOG1, FL("RXP setting = %08x\n"), regVal));

    return eHAL_STATUS_SUCCESS;
}

// Add SSID based filtering 
eHalStatus halRxp_EnableSSIDBasedFilter( tpAniSirGlobal pMac, 
    tSirMacSSid *pSirMacSSid)
{
    tANI_U32 ssidCheckEnable=0, ssidLength=0, ssidValue=0;

    tANI_U32 regVal = 0;


    //SSID Has to be in little endian format.
    (void) palZeroMemory(pMac->hHdd, 
        &(pSirMacSSid->ssId[pSirMacSSid->length]),
                         (32-(pSirMacSSid->length)));
    // Copy the ssid value  
    ssidValue = (pSirMacSSid->ssId[0]) | 
        (pSirMacSSid->ssId[1] << 8) | (pSirMacSSid->ssId[2] << 16) | 
        (pSirMacSSid->ssId[3] << 24);
    halWriteRegister(pMac, 
        QWLAN_RXP_SSID_VALUES_B0_B3_REG, ssidValue);
    HALLOG1(halLog(pMac, LOG1, FL("Low1=%x\n"), ssidValue));

    ssidValue = (pSirMacSSid->ssId[4]) | 
        (pSirMacSSid->ssId[5] << 8) | (pSirMacSSid->ssId[6] << 16) | 
        (pSirMacSSid->ssId[7] << 24);
    halWriteRegister(pMac, 
        QWLAN_RXP_SSID_VALUES_B4_B7_REG, ssidValue);
    HALLOG1(halLog(pMac, LOG1, FL("Low2=%x\n"), ssidValue));

    ssidValue = (pSirMacSSid->ssId[8]) | 
        (pSirMacSSid->ssId[9] << 8) | (pSirMacSSid->ssId[10] << 16) | 
        (pSirMacSSid->ssId[11] << 24);
    halWriteRegister(pMac, 
        QWLAN_RXP_SSID_VALUES_B8_B11_REG, ssidValue);
    HALLOG1(halLog(pMac, LOG1, FL("Low3=%x\n"), ssidValue));

    ssidValue = (pSirMacSSid->ssId[12]) | 
        (pSirMacSSid->ssId[13] << 8) | (pSirMacSSid->ssId[14] << 16) | 
        (pSirMacSSid->ssId[15] << 24);
    halWriteRegister(pMac, 
        QWLAN_RXP_SSID_VALUES_B12_B15_REG, ssidValue);
    HALLOG1(halLog(pMac, LOG1, FL("Low4=%x\n"), ssidValue));

    if (pSirMacSSid->length > 16) {
        ssidValue = (pSirMacSSid->ssId[16]) | 
            (pSirMacSSid->ssId[17] << 8) | (pSirMacSSid->ssId[18] << 16) | 
            (pSirMacSSid->ssId[19] << 24);
        halWriteRegister(pMac, 
            QWLAN_RXP_SSID_VALUES_B16_B19_REG, ssidValue);
        HALLOG1(halLog(pMac, LOG1, FL("Low5=%x\n"), ssidValue));

        ssidValue = (pSirMacSSid->ssId[20]) | 
            (pSirMacSSid->ssId[21] << 8) | (pSirMacSSid->ssId[22] << 16) | 
            (pSirMacSSid->ssId[23] << 24);
        halWriteRegister(pMac, 
            QWLAN_RXP_SSID_VALUES_B20_B23_REG, ssidValue);
        HALLOG1(halLog(pMac, LOG1, FL("Low6=%x\n"), ssidValue));

        ssidValue = (pSirMacSSid->ssId[24]) | 
            (pSirMacSSid->ssId[25] << 8) | (pSirMacSSid->ssId[26] << 16) | 
            (pSirMacSSid->ssId[27] << 24);
        halWriteRegister(pMac, 
            QWLAN_RXP_SSID_VALUES_B24_B27_REG, ssidValue);
        HALLOG1(halLog(pMac, LOG1, FL("Low7=%x\n"), ssidValue));

        ssidValue = (pSirMacSSid->ssId[28]) | 
            (pSirMacSSid->ssId[29] << 8) | (pSirMacSSid->ssId[30] << 16) | 
            (pSirMacSSid->ssId[31] << 24);
        halWriteRegister(pMac, 
            QWLAN_RXP_SSID_VALUES_B28_B31_REG, ssidValue);
        HALLOG1(halLog(pMac, LOG1, FL("Low8=%x\n"), ssidValue));

    }

    ssidCheckEnable = QWLAN_RXP_BEACON_TSF_TIM_EXTRACT_CTRL_ENABLE_BEACON_SSID_CHECK_MASK;
    ssidLength = (pSirMacSSid->length << QWLAN_RXP_BEACON_TSF_TIM_EXTRACT_CTRL_CFG_MPR_SSID_LENGTH_OFFSET);

    halReadRegister(pMac, QWLAN_RXP_BEACON_TSF_TIM_EXTRACT_CTRL_REG, &regVal);
    // Enable the SSID check and set the length.
    regVal |= (ssidCheckEnable | ssidLength );
    HALLOG1(halLog(pMac, LOG1, FL("RXP setting SSID = %08x\n"), regVal));

    halWriteRegister(pMac, QWLAN_RXP_BEACON_TSF_TIM_EXTRACT_CTRL_REG, regVal);

    return eHAL_STATUS_SUCCESS;
}

/** ----------------------------------------------------------
\fn     halRxp_DisableBssBeaconParamFilter
\brief  This function is called by halMsg_DelBss() to disable beacon
        related parameter filters.
        FIXME_GEN6: This function now only support single BSS: IBSS or STA mode
        When multiple BSS coexistnece (BTAMP) needs to be supported, it needs to be revised.
\param  tpAniSirGlobal  pMac
\return status
\ ------------------------------------------------------------ */

eHalStatus halRxp_DisableBssBeaconParamFilter( tpAniSirGlobal pMac, tANI_U32 uBssIdx)
{
    tpBssStruct pBss = (tpBssStruct) pMac->hal.halMac.bssTable;
    pBss = &pBss[uBssIdx];
    //FIXME_GEN6: Wen BTAMP/MultiBSS support is in, should use BSS index to check BSS role
    //and update these registers.
    halWriteRegister(pMac, QWLAN_RXP_BEACON_ADDR3_LOW_FIELD_REG, 0);
    halWriteRegister(pMac, QWLAN_RXP_BEACON_ADDR3_HIGH_FIELD_REG, 0);
    halWriteRegister(pMac, QWLAN_RXP_BEACON_ADDR2_LOW_FIELD_REG, 0);
    halWriteRegister(pMac, QWLAN_RXP_BEACON_ADDR2_HIGH_FIELD_REG, 0);

    halWriteRegister(pMac, QWLAN_RXP_BEACON_TSF_TIM_EXTRACT_CTRL_REG, 0);
    HALLOG1(halLog(pMac, LOG1, FL("RXP Clearing SSID based filter\n"))); 
    return eHAL_STATUS_SUCCESS;
}


/*
 * DESCRIPTION:
 *      Routine to write RXP binary search table into ADU memory.
 *      The contents of the BST table are written into the
 *      hardware through indirectly addressed registers.
 *      Basically by writting the value of the table entry
 *      into the data register and then writing into command
 *      the  command register. So the this table contains
 *      a repetition of the value in the data and command
 *      registers
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *      pAddr:  Starting address in the ADU memory from where the
 *              register/value tuple will be written
 *
 * RETURN:
 *      None
 */
void halRxp_BckupRxpTableEntry(tpAniSirGlobal pMac,
        tRxpAddrTable* ptable,  tANI_U8 index, tANI_U32 *pAddr)
{
    tANI_U32 value = 0;
    tANI_U32 startAddr = *pAddr;
    tANI_U32 *pMemAddr = (tANI_U32*)startAddr;

    // Write the Data0 Address and value
    value = QWLAN_RXP_SEARCH_TABLE_DATA0_REG | HAL_REG_RSVD_BIT | HAL_REG_HOST_FILLED;
    halWriteDeviceMemory(pMac, (tANI_U32)pMemAddr++,
                (tANI_U8*)&value, sizeof(tANI_U32));

    value = (ptable->macAddr[3] << 24) | (ptable->macAddr[2] << 16) |
        (ptable->macAddr[1] << 8)  | ptable->macAddr[0];
    halWriteDeviceMemory(pMac, (tANI_U32)pMemAddr++,
                (tANI_U8*)&value, sizeof(tANI_U32));

    // Write the Data1 Address and value
    value = QWLAN_RXP_SEARCH_TABLE_DATA1_REG | HAL_REG_RSVD_BIT | HAL_REG_HOST_FILLED;
    halWriteDeviceMemory(pMac, (tANI_U32)pMemAddr++,
                (tANI_U8*)&value, sizeof(tANI_U32));

    value = ((ptable->dpuIGTKTag << QWLAN_RXP_SEARCH_TABLE_DATA1_DPU_MAN_MC_BC_SIG_OFFSET) |
            (QWLAN_RXP_SEARCH_TABLE_DATA1_VALID_MASK) |
            (ptable->staid << QWLAN_RXP_SEARCH_TABLE_DATA1_ADDR_ID_OFFSET) |
            (ptable->rmfBit << QWLAN_RXP_SEARCH_TABLE_DATA1_RMF_OFFSET) |
            (ptable->ftBit << QWLAN_RXP_SEARCH_TABLE_DATA1_FRAME_TRANSLATION_OFFSET) |
            (ptable->dropBit << QWLAN_RXP_SEARCH_TABLE_DATA1_DROP_OFFSET)  |
            (ptable->macAddr[5] << 8) |
            (ptable->macAddr[4]));
    halWriteDeviceMemory(pMac, (tANI_U32)pMemAddr++,
                (tANI_U8*)&value, sizeof(tANI_U32));

    // Write the Data2 Address and value
    value = QWLAN_RXP_SEARCH_TABLE_DATA2_REG | HAL_REG_RSVD_BIT | HAL_REG_HOST_FILLED;
    halWriteDeviceMemory(pMac, (tANI_U32)pMemAddr++,
                (tANI_U8*)&value, sizeof(tANI_U32));

    value = ((ptable->dpuPTKDescIdx) |
            (ptable->dpuGTKDpuIdx << QWLAN_RXP_SEARCH_TABLE_DATA2_DPU_MC_BC_DPU_DESC_OFFSET) |
            (ptable->dpuPTKSig << QWLAN_RXP_SEARCH_TABLE_DATA2_DPU_TAG_OFFSET) |
            (ptable->wepKeyIdxExtractEnable << QWLAN_RXP_SEARCH_TABLE_DATA2_WEP_KEY_ID_EXTRACT_ENABLE_OFFSET) |
            (ptable->dpuNE << QWLAN_RXP_SEARCH_TABLE_DATA2_DPU_NE_OFFSET) |
            (ptable->dpuIGTKDpuIdx << QWLAN_RXP_SEARCH_TABLE_DATA2_DPU_MAN_MC_BC_DESCRIPTOR_INDEX_OFFSET) |
            (ptable->dpuGTKSig << QWLAN_RXP_SEARCH_TABLE_DATA2_DPU_MC_BC_TAG_OFFSET));
    halWriteDeviceMemory(pMac, (tANI_U32)pMemAddr++,
                (tANI_U8*)&value, sizeof(tANI_U32));

    // Write the Command Address and value
    value = QWLAN_RXP_SEARCH_TABLE_CMD_REG | HAL_REG_RSVD_BIT | HAL_REG_HOST_FILLED;
    halWriteDeviceMemory(pMac, (tANI_U32)pMemAddr++,
                (tANI_U8*)&value, sizeof(tANI_U32));
    value = QWLAN_RXP_SEARCH_TABLE_CMD_WRITE_MASK | index;
    halWriteDeviceMemory(pMac, (tANI_U32)pMemAddr++,
                (tANI_U8*)&value, sizeof(tANI_U32));

    // Update the start address pointer pointing to the
    // register re-init table
    *pAddr = (tANI_U32)pMemAddr;

    return;
}

/* DESCRIPTION:
 *      Routine to write RXP binary search table into ADU memory.
 *      Writing each of the address1, address2 and address3 BST
 *      table
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *      pAddr:  Starting address in the ADU memory from where the
 *              register/value tuple will be written
 *
 * RETURN:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */
eHalStatus halRxp_BckupRxpSearchTable(tpAniSirGlobal pMac, tANI_U32 *pMemAddr)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tANI_U8 i;
    tpRxpInfo pRxp = GET_RXP_INFO(pMac);

    if(pRxp == NULL)
    {
        HALLOGW( halLog(pMac, LOGW, FL("pRXP is NULL\n")));
        return eHAL_STATUS_FAILURE;
    }

    // Backup Address 1 table
    for(i=0; i<pRxp->addr1.numOfEntry; i++) {
        halRxp_BckupRxpTableEntry(pMac, &pRxp->addr1_table[i],
                pRxp->addr1.lowPtr + i, pMemAddr);
        }

    // Backup Address 2 table
    for(i=0; i<pRxp->addr2.numOfEntry; i++) {
        halRxp_BckupRxpTableEntry(pMac, &pRxp->addr2_table[i],
                pRxp->addr2.lowPtr + i, pMemAddr);
        }

    // Backup Address 3 table
    for(i=0; i<pRxp->addr3.numOfEntry; i++) {
        halRxp_BckupRxpTableEntry(pMac, &pRxp->addr3_table[i],
                pRxp->addr3.lowPtr + i, pMemAddr);
    }

    return status;
}



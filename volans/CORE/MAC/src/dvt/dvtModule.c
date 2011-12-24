/** ------------------------------------------------------------------------- *
    ------------------------------------------------------------------------- *


    \file dvtModule.c

    \brief dvt module provides the actual functionality to service the apis.

    $Id$

    Copyright (c) 2011 Qualcomm Atheros, Inc. 
    All Rights Reserved. 
    Qualcomm Atheros Confidential and Proprietary. 

    Copyright (C) 2006 Airgo Networks, Incorporated

   ========================================================================== */
#ifdef ANI_DVT_DEBUG
#include <stdlib.h>
#include <string.h>
#include "ani_assert.h"

#include "aniGlobal.h"
#include "halMsgApi.h"
#include "halCommonApi.h"

#include "cfgApi.h"
#include "sysStartup.h"
#include "dvtModule.h"
#include "dvtModuleApi.h"
#include "dvtDebug.h"

#include "ani_assert.h"

#include "halBmu.h"
#include "smacHostIf.h"

// ==============================================================
//
// copy these from PolarisStructs.h which has been deprecated....
// apparently coded in this module is using some of these.
//
// ==============================================================

typedef struct sPolBufDesc_L {

  unsigned long        nextBD:19;
  unsigned long        rsvd1:5;
  unsigned long        bassp:1;
  unsigned long        skipSP:1;
  unsigned long        relay:1;
  unsigned long        rsvd:1;
  unsigned long        ackPolicy:2;
  unsigned long        multicast:1;
  unsigned long        sendRTS:1;

  unsigned long        headPtr:19;
  unsigned long        rsvd2:2;
  unsigned long        hardwareSeqNum:1;
  unsigned long        dummy:1;
  unsigned long        crcFailed:1;
  unsigned long        phyHdrOverride:1;
  unsigned long        durationOverride:1;
  unsigned long        macHdrLength:6;

  unsigned long        tailPtr:19;
  unsigned long        rsvd3:1;
  unsigned long        defaultKey:1;
  unsigned long        defaultKeyId:2;
  unsigned long        edPolicy:3;
  unsigned long        pduCount:6;

  unsigned long        numFrags:4;
  unsigned long        payloadLength:12;
  unsigned long        fragmentSize:12;
  unsigned long        rsvd4:2;
  unsigned long        rtsInGmode:1;
  unsigned long        ctsInGmode:1;

  unsigned long        tcId:4;
  unsigned long        staId:12;
  unsigned long        relayStaId:12;
  unsigned long        rsvd5:4;

  unsigned long        routingFlags:24;
  unsigned long        rsvd6:8;


  unsigned long        phyA[2];
  unsigned long        phyB[2];

  unsigned long        macHdr[8];

  unsigned short       durRTS;
  unsigned short       durLast;

  unsigned short       durSecondLast;
  unsigned short       durOther;

  unsigned short       txDurLast;
  unsigned short       txDurOther;


  unsigned short       lengthLast11a;
  unsigned short       lengthOther11a;

  unsigned short       lengthLast11b;
  unsigned short       lengthOther11b;

  unsigned long        rsvd7[9];
} tPolBufDesc_L;



/// Frame control field format (2 bytes)
typedef struct sSirMacFrameCtl_L
{
    unsigned char protVer :2;
    unsigned char type :2;
    unsigned char subType :4;

    unsigned char toDS :1;
    unsigned char fromDS :1;
    unsigned char moreFrag :1;
    unsigned char retry :1;
    unsigned char powerMgmt :1;
    unsigned char moreData :1;
    unsigned char wep :1;
    unsigned char order :1;

} tSirMacFrameCtl_L, *tpSirMacFrameCtl_L;

/// Sequence control field
typedef struct sSirMacSeqCtl_L
{
    unsigned char fragNum : 4;
    unsigned char seqNumLo : 4;
    unsigned char seqNumHi : 8;

} tSirMacSeqCtl_L, *tpSirMacSeqCtl_L;




typedef struct sSirMacMgmtHdr_L
{
  tSirMacFrameCtl_L fc;
  unsigned short             duration;
  unsigned char              da[6];
  unsigned char              sa[6];
  unsigned char              bssId[6];
  tSirMacSeqCtl_L   seqControl;
} tSirMacMgmtHdr_L, *tpSirMacMgmtHdr_L;

// QoS control field
typedef struct sSirMacQosCtl_L
{
    unsigned char tid : 4;
    unsigned char fec : 1;
    unsigned char ackPolicy : 2;
    unsigned char amsduPresent : 1;

    unsigned char txop : 8;

} tSirMacQosCtl_L, *tpSirMacQosCtl_L;

// Link Adaptation control field
typedef struct sSirMacLinkAdptCtl_L
{
    unsigned short ma : 1;
    unsigned short trq : 1;
    unsigned short mrq : 1;
    unsigned short mrs : 3;
    unsigned short mfs : 3;
    unsigned short mfb : 7;
} tSirMacLinkAdptCtl_L, *tpSirMacLinkAdptCtl_L;

// HT control field
typedef struct sSirMacHtCtl_L
{
    tSirMacLinkAdptCtl_L lnkAdaptCtl;
    unsigned char calibPosition : 2;
    unsigned char calibSeq : 2;
    unsigned char fdbkReq : 2;
    unsigned char csi : 2;

    unsigned char zlfAnnouncement : 1;
    unsigned char rsvd : 5;
    unsigned char acConstraint : 1;
    unsigned char rdg : 1;

} tSirMacHtCtl_L, *tpSirMacHtCtl_L;




/// 3 address MAC data header format (24/26 bytes)
typedef struct sSirMacDataHdr3a_L
{
  tSirMacFrameCtl_L fc;
  unsigned short          duration;
  unsigned char           addr1[6];
  unsigned char           addr2[6];
  unsigned char           addr3[6];
  tSirMacSeqCtl_L   seqControl;
  tSirMacQosCtl_L   qosControl;
} tSirMacDataHdr3a_L, *tpSirMacDataHdr3a_L;


typedef struct sSirMacDataHdr3a_Ht_L
{
  tSirMacFrameCtl_L fc;
  unsigned short          duration;
  unsigned char           addr1[6];
  unsigned char           addr2[6];
  unsigned char           addr3[6];
  tSirMacSeqCtl_L   seqControl;
  tSirMacQosCtl_L   qosControl;
  tSirMacHtCtl_L   htControl;
} tSirMacDataHdr3a_Ht_L, *tpSirMacDataHdr3a_Ht_L;


/// 4 address MAC data header format
typedef struct sSirMacDataHdr4a_L
{
  tSirMacFrameCtl_L fc;
  unsigned short          duration;
  unsigned char           addr1[6];
  unsigned char           addr2[6];
  unsigned char           addr3[6];
  tSirMacSeqCtl_L         seqControl;
  unsigned char           addr4[6];
  tSirMacQosCtl_L         qosControl;
} tSirMacDataHdr4a_L, *tpSirMacDataHdr4a_L;

// ==============================================================
//
// copy these from PolarisStructs.h which has been deprecated....
// apparently coded in this module is using some of these.
//
// ==============================================================




static void dvtAppTxDequeueCallback(tHalHandle hHal, eDvtTxQueue queue);

tANI_U32 dpuRegs[MAX_DPU_REGS] =
{
    DPU_DPU_TXPKTCOUNT_REG,  //             DPU_BASE + 0x0
    DPU_DPU_RXPKTCOUNT_REG,  //             DPU_BASE + 0x4
    DPU_DPU_BD_CHECK_COUNT_REG,  //                 DPU_BASE + 0x8
    DPU_DPU_BASE_ADDR_REG,  //              DPU_BASE + 0xC
    DPU_DPU_KEYBASE_ADDR_REG,  //           DPU_BASE + 0x10
    DPU_DPU_MICKEYBASE_ADDR_REG,  //                DPU_BASE + 0x14
    DPU_DPU_RCBASE_ADDR_REG,  //            DPU_BASE + 0x18
    DPU_DPU_CONTROL_REG,  //                DPU_BASE + 0x1C
    DPU_DPU_INTERRUPT_MASK_REG,  //                 DPU_BASE + 0x20
    DPU_DPU_INTERRUPT_STATUS_REG,  //               DPU_BASE + 0x24
    DPU_DPU_REPLAY_CNT_THR_MSW_REG,  //             DPU_BASE + 0x2C
    DPU_DPU_REPLAY_CNT_THR_LSW_REG,  //             DPU_BASE + 0x30
    DPU_DPU_PPI_EXPAND_THRESHOLD_REG,  //           DPU_BASE + 0x34
    DPU_DPU_MAXIMUM_PKT_LEN_REG,  //                DPU_BASE + 0x38
    DPU_DPU_COMPRESSION_TYPE_CNT_REG,  //           DPU_BASE + 0x3C
    DPU_DPU_COMPRESSION_NUM_PKTS_REG,  //           DPU_BASE + 0x40
    DPU_DPU_DECOMPRESSION_NUM_PKTS_REG,  //                 DPU_BASE + 0x44
    DPU_DPU_WATCHDOG_REG,  //               DPU_BASE + 0x48
    DPU_DPU_BD_IN_COUNTS_REG,  //           DPU_BASE + 0x4C
    DPU_DPU_BD_OUT_COUNTS_REG,  //          DPU_BASE + 0x50
    DPU_DPU_WQ_3_RESERVE_REG,  //           DPU_BASE + 0x54
    DPU_DPU_WQ_4_RESERVE_REG,  //           DPU_BASE + 0x58
    DPU_DPU_WQ_5_RESERVE_REG,  //           DPU_BASE + 0x5C
    DPU_DPU_WQ_6_RESERVE_REG,  //           DPU_BASE + 0x60
    DPU_DPU_WQ_7_RESERVE_REG,  //           DPU_BASE + 0x64
    DPU_DPU_WQ_8_RESERVE_REG,  //           DPU_BASE + 0x68
    DPU_DPU_WQ_9_RESERVE_REG,  //           DPU_BASE + 0x6C
    DPU_DPU_WQ_10_RESERVE_REG,  //          DPU_BASE + 0x70
    DPU_DPU_ERROR_WQ_REG,  //               DPU_BASE + 0x74
    DPU_DPU_ERROR_WQ_SELECT_REG,  //                DPU_BASE + 0x78
    DPU_DPU_ERROR_WQ_COUNT_REG,  //                 DPU_BASE + 0x7C
    DPU_DPU_FRAG_COUNT_REG,  //             DPU_BASE + 0x80
    DPU_DPU_DBG_TESTBUS_HI_REG,  //                 DPU_BASE + 0x84
    DPU_DPU_DBG_TESTBUS_LO_REG,  //                 DPU_BASE + 0x88
    DPU_DPU_DBG_WQ_AVAIL_REG,  //           DPU_BASE + 0x8C
    DPU_DPU_DBG_WQ_ALLOW_REG,  //           DPU_BASE + 0x90
    DPU_DPU_DBG_SM_STATE_REG,  //           DPU_BASE + 0x94
    DPU_DPU_DBG_BUF_FILL_REG  //           DPU_BASE + 0x98
};

tANI_U32 bmuRegs[MAX_BMU_REGS] =
{
    BMU_CONTROL_REG,  //           BMU_BASE + 0x0
    BMU_ERR_INTR_STATUS_REG,  //           BMU_BASE + 0x4
    BMU_ERR_INTR_ENABLE_REG,  //           BMU_BASE + 0x8
    BMU_ERR_INT_ADDR_REG,  //              BMU_BASE + 0xC
    BMU_ERR_INT_WDATA_REG,  //             BMU_BASE + 0x10
    BMU_BD_PDU_THRESHOLD0_REG,  //         BMU_BASE + 0x14
    BMU_BD_PDU_THRESHOLD1_REG,  //         BMU_BASE + 0x18
    BMU_BD_PDU_THRESHOLD2_REG,  //         BMU_BASE + 0x1C
    BMU_BD_PDU_THRESHOLD3_REG,  //         BMU_BASE + 0x20
    BMU_BD_PDU_THRESHOLD4_REG,  //         BMU_BASE + 0x24
    BMU_BD_PDU_THRESHOLD5_REG,  //         BMU_BASE + 0x28
    BMU_BD_PDU_THRESHOLD6_REG,  //         BMU_BASE + 0x2C
    BMU_BD_PDU_THRESHOLD7_REG,  //         BMU_BASE + 0x30
    BMU_BD_PDU_THRESHOLD8_REG,  //         BMU_BASE + 0x34
    BMU_BD_PDU_THRESHOLD9_REG,  //         BMU_BASE + 0x38
    BMU_BD_PDU_THRESHOLD10_REG,  //                BMU_BASE + 0x3C
    BMU_BD_PDU_RESERVED0_REG,  //          BMU_BASE + 0x40
    BMU_BD_PDU_RESERVED1_REG,  //          BMU_BASE + 0x44
    BMU_BD_PDU_RESERVED2_REG,  //          BMU_BASE + 0x48
    BMU_BD_PDU_RESERVED3_REG,  //          BMU_BASE + 0x4C
    BMU_BD_PDU_RESERVED4_REG,  //          BMU_BASE + 0x50
    BMU_BD_PDU_RESERVED5_REG,  //          BMU_BASE + 0x54
    BMU_BD_PDU_RESERVED6_REG,  //          BMU_BASE + 0x58
    BMU_BD_PDU_RESERVED7_REG,  //          BMU_BASE + 0x5C
    BMU_BD_PDU_RESERVED8_REG,  //          BMU_BASE + 0x60
    BMU_BD_PDU_RESERVED9_REG,  //          BMU_BASE + 0x64
    BMU_BD_PDU_RESERVED10_REG,  //         BMU_BASE + 0x68
    BMU_WQ_ENABLE_REG,  //         BMU_BASE + 0x6C
    BMU_DPU_WQ_ASSIGNMENT_REG,  //         BMU_BASE + 0x70
    BMU_TEST_CTRL_REG,  //         BMU_BASE + 0x74
    BMU_TEST_OUT_LOW_REG,  //              BMU_BASE + 0x78
    BMU_TEST_OUT_HIGH_REG,  //             BMU_BASE + 0x7C
    BMU_GAS_CONFIGURATION_REG,  //         BMU_BASE + 0x80
    BMU_AVAILABLE_BD_PDU_AFTER_RSV_REG,  //                BMU_BASE + 0x84
    BMU_CONTROL2_REG,  //          BMU_BASE + 0x88
    BMU_DISABLE_WQ_DA_REG,  //             BMU_BASE + 0x8C
    BMU_DISABLE_BD_PDU_AVAIL_REG,  //              BMU_BASE + 0x90
    BMU_LAST_FOUR_POP_TRACE_CONTROL_REG,  //               BMU_BASE + 0x94
    BMU_POP_CMD_TRACE_BD_INDEX0_REG,  //           BMU_BASE + 0x98
    BMU_POP_CMD_TRACE_BD_INDEX1_REG,  //           BMU_BASE + 0x9C
    BMU_POP_CMD_TRACE_BD_INDEX2_REG,  //           BMU_BASE + 0xA0
    BMU_POP_CMD_TRACE_BD_INDEX3_REG,  //           BMU_BASE + 0xA4
    BMU_BMU_DBGC_TRACE_ADDR_REG,  //               BMU_BASE + 0xA8
    BMU_BMU_INTERNAL_MEM_TRACE_ADDR_REG,  //               BMU_BASE + 0xAC
    BMU_RELEASE_ERROR_DETAILS_REG,  //             BMU_BASE + 0xB0
    BMU_RELEASE_ERROR_DETAILS_LOW_REG,  //         BMU_BASE + 0xB4
    BMU_RELEASE_ERROR_DETAILS_HIGH_REG,  //                BMU_BASE + 0xB8
    BMU_RELEASE_FIFO_ENTRIES_REG,  //              BMU_BASE + 0xBC
    BMU_LAST_BD_PREFETCH_HEAD_REG,  //             BMU_BASE + 0xC0
    BMU_BD_PREFETCH_HEAD_BD_LINK_REG,  //          BMU_BASE + 0xC4
    BMU_BMU_PUSH_INTEGRITY_ERR_BD_INDEX_REG,  //           BMU_BASE + 0xC8
    BMU_BMU_WATCHDOG_TIMEOUT_TIME_REG,  //         BMU_BASE + 0xCC
    BMU_BMU_SM_STATES_ON_ERROR_LOW_REG,  //                BMU_BASE + 0xD0
    BMU_BMU_SM_STATES_ON_ERROR_HIGH_REG,  //               BMU_BASE + 0xD4
    BMU_BMU_PREFETCH_ERROR_DETAILS_REG,  //                BMU_BASE + 0xD8
    BMU_CONTROL3_REG,  //          BMU_BASE + 0xDC
    BMU_CONTROL4_REG,  //          BMU_BASE + 0xE0
    BMU_DPU_RSV_STATUS_REG,  //            BMU_BASE + 0xE0
    BMU_RECEIVE_DEBUG_CONTROL_REG,  //             BMU_BASE + 0xE8
    BMU_RESERVED_REG  //          BMU_BASE + 0xEC
};

tANI_U32 dxeRegs[MAX_DXE_REGS] =
{
    DXE_0_DMA_CSR_REG,  //         DXE_0_BASE + 0x0
    DXE_0_DMA_ENCH_REG,  //                DXE_0_BASE + 0x4
    DXE_0_DMA_CH_EN_REG,  //               DXE_0_BASE + 0x8
    DXE_0_DMA_CH_DONE_REG,  //             DXE_0_BASE + 0xC
    DXE_0_DMA_CH_ERR_REG,  //              DXE_0_BASE + 0x10
    DXE_0_DMA_CH_STOP_REG,  //             DXE_0_BASE + 0x14
    DXE_0_INT_MSK_REG,  //         DXE_0_BASE + 0x18
    DXE_0_INT_SRC_MSKD_REG,  //            DXE_0_BASE + 0x1C
    DXE_0_INT_SRC_RAW_REG,  //             DXE_0_BASE + 0x20
    DXE_0_INT_ED_SRC_REG,  //              DXE_0_BASE + 0x24
    DXE_0_INT_DONE_SRC_REG,  //            DXE_0_BASE + 0x28
    DXE_0_INT_ERR_SRC_REG,  //             DXE_0_BASE + 0x2C
    DXE_0_INT_CLR_REG,  //         DXE_0_BASE + 0x30
    DXE_0_INT_ED_CLR_REG,  //              DXE_0_BASE + 0x34
    DXE_0_INT_DONE_CLR_REG,  //            DXE_0_BASE + 0x38
    DXE_0_INT_ERR_CLR_REG,  //             DXE_0_BASE + 0x3C
    DXE_0_DMA_CH_PRES_REG,  //             DXE_0_BASE + 0x40
    DXE_0_TSTBUS_CTRL_REG,  //             DXE_0_BASE + 0x44
    DXE_0_TSTBUS_LOW_REG,  //              DXE_0_BASE + 0x48
    DXE_0_TSTBUS_HIGH_REG,  //             DXE_0_BASE + 0x4C
    DXE_0_PF_BD_REG,  //           DXE_0_BASE + 0x50
    DXE_0_PF_PDU_REG,  //          DXE_0_BASE + 0x54
    DXE_0_REVISION_REG,  //                DXE_0_BASE + 0x58
    DXE_0_BMU_SB_REG,  //          DXE_0_BASE + 0x5C
    DXE_0_STATE_REG,  //           DXE_0_BASE + 0x60
    DXE_0_ARB_CH_MSK_REG,  //              DXE_0_BASE + 0x64
    DXE_0_UIF_SB_REG,  //          DXE_0_BASE + 0x68
    DXE_0_TIMESTAMP_REG,  //               DXE_0_BASE + 0x6C
    DXE_0_CTR_PRES_REG,  //                DXE_0_BASE + 0x70
    DXE_0_CTR_CLR_REG,  //         DXE_0_BASE + 0x74
    DXE_0_COUNTER0_REG,  //                DXE_0_BASE + 0x200
    DXE_0_COUNTER1_REG,  //                DXE_0_BASE + 0x204
    DXE_0_COUNTER2_REG,  //                DXE_0_BASE + 0x208
    DXE_0_COUNTER3_REG,  //                DXE_0_BASE + 0x20C
    DXE_0_COUNTER4_REG,  //                DXE_0_BASE + 0x210
    DXE_0_COUNTER5_REG,  //                DXE_0_BASE + 0x214
    DXE_0_COUNTER6_REG,  //                DXE_0_BASE + 0x218
    DXE_0_COUNTER7_REG,  //                DXE_0_BASE + 0x21C
    DXE_0_COUNTER8_REG,  //                DXE_0_BASE + 0x220
    DXE_0_COUNTER9_REG,  //                DXE_0_BASE + 0x224
    DXE_0_CH_CTRL_REG,  //         DXE_0_BASE + 0x400
    DXE_0_CH_STATUS_REG,  //               DXE_0_BASE + 0x404
    DXE_0_CH_SZ_REG,  //           DXE_0_BASE + 0x408
    DXE_0_CH_SADRL_REG,  //                DXE_0_BASE + 0x40C
    DXE_0_CH_SADRH_REG,  //                DXE_0_BASE + 0x410
    DXE_0_CH_DADRL_REG,  //                DXE_0_BASE + 0x414
    DXE_0_CH_DADRH_REG,  //                DXE_0_BASE + 0x418
    DXE_0_CH_DESCL_REG,  //                DXE_0_BASE + 0x41C
    DXE_0_CH_DESCH_REG,  //                DXE_0_BASE + 0x420
    DXE_0_CH_LST_DESCL_REG,  //            DXE_0_BASE + 0x424
    DXE_0_CH_LST_DESCH_REG,  //            DXE_0_BASE + 0x428
    DXE_0_CH_BD_REG,  //           DXE_0_BASE + 0x42C
    DXE_0_CH_HEAD_REG,  //         DXE_0_BASE + 0x430
    DXE_0_CH_TAIL_REG,  //         DXE_0_BASE + 0x434
    DXE_0_CH_PDU_REG,  //          DXE_0_BASE + 0x438
    DXE_0_CH_TSTMP_REG  //                DXE_0_BASE + 0x43C
};

tANI_U32 rxpRegs[MAX_RXP_REGS] =
{
    RXP_CONFIG_REG,  //            RXP_BASE + 0x0
    RXP_PKT_FILTER_MASK_REG,  //           RXP_BASE + 0x4
    RXP_MAX_PKTLEN_REG,  //                RXP_BASE + 0x8
    RXP_ROUTING_FLAG_REG,  //              RXP_BASE + 0xC
    RXP_MPDU_HEADER_START_OFFSET_REG,  //          RXP_BASE + 0x10
    RXP_DLM_LEN_REG,  //           RXP_BASE + 0x14
    RXP_DLM_RESULT_REG,  //                RXP_BASE + 0x18
    RXP_DLM_DATA0_REG,  //         RXP_BASE + 0x1C
    RXP_DLM_DATA1_REG,  //         RXP_BASE + 0x20
    RXP_DLM_DATA2_REG,  //         RXP_BASE + 0x24
    RXP_DLM_DATA3_REG,  //         RXP_BASE + 0x28
    RXP_SEARCH_ADDR1_PTR_REG,  //          RXP_BASE + 0x2C
    RXP_SEARCH_ADDR2_PTR_REG,  //          RXP_BASE + 0x30
    RXP_SEARCH_ADDR3_PTR_REG,  //          RXP_BASE + 0x34
    RXP_SEARCH_TABLE_CMD_REG,  //          RXP_BASE + 0x38
    RXP_SEARCH_TABLE_DATA0_REG,  //                RXP_BASE + 0x3C
    RXP_SEARCH_TABLE_DATA1_REG,  //                RXP_BASE + 0x40
    RXP_MAX_LEN_AMPDU_REG,  //             RXP_BASE + 0x44
    RXP_MAX_LEN_MPDU_IN_AMPDU_REG,  //             RXP_BASE + 0x48
    RXP_DMA_MAX_RSV_PDU_REG,  //           RXP_BASE + 0x4C
    RXP_DMA_BURST_THRESHOLD_REG,  //               RXP_BASE + 0x50
    RXP_CLEAR_STATS_REG,  //               RXP_BASE + 0x54
    RXP_PHY_MPDU_CNT_REG,  //              RXP_BASE + 0x58
    RXP_PHY_AMPDU_CNT_REG,  //             RXP_BASE + 0x5C
    RXP_PHY_APPDU_CNT_REG,  //             RXP_BASE + 0x60
    RXP_PHY_ABORT_CNT_REG,  //             RXP_BASE + 0x64
    RXP_PHY_SHUTOFF_CNT_REG,  //           RXP_BASE + 0x68
    RXP_DLM_FIFO_FULL_CNT_REG,  //         RXP_BASE + 0x6C
    RXP_DLM_ERR_CNT_REG,  //               RXP_BASE + 0x70
    RXP_FAIL_FILTER_CNT_REG,  //           RXP_BASE + 0x74
    RXP_FAIL_MAX_PKTLEN_CNT_REG,  //               RXP_BASE + 0x78
    RXP_FCS_ERR_CNT_REG,  //               RXP_BASE + 0x7C
    RXP_DMA_SEND_CNT_REG,  //              RXP_BASE + 0x80
    RXP_DMA_DROP_CNT_REG,  //              RXP_BASE + 0x84
    RXP_DMA_GET_BMU_FAIL_CNT_REG,  //              RXP_BASE + 0x88
    RXP_PROTOCOL_VERSION_FILTER_CNT_REG,  //               RXP_BASE + 0x8C
    RXP_TYPE_SUBTYPE_FILTER_CNT_REG,  //           RXP_BASE + 0x90
    RXP_INCORRECT_LENGTH_FILTER_CNT_REG,  //               RXP_BASE + 0x94
    RXP_ADDR1_BLOCK_FILTER_CNT_REG,  //            RXP_BASE + 0x98
    RXP_ADDR1_HIT_NO_PASS_CNT_REG,  //             RXP_BASE + 0x9C
    RXP_ADDR1_DROP_CNT_REG,  //            RXP_BASE + 0xA0
    RXP_ADDR2_HIT_NO_PASS_CNT_REG,  //             RXP_BASE + 0xA4
    RXP_ADDR2_DROP_CNT_REG,  //            RXP_BASE + 0xA8
    RXP_ADDR3_HIT_NO_PASS_CNT_REG,  //             RXP_BASE + 0xAC
    RXP_ADDR3_DROP_CNT_REG,  //            RXP_BASE + 0xB0
    RXP_MPDU_IN_AMPDU_CNT_REG,  //         RXP_BASE + 0xB4
    RXP_PHY_ERR_DROP_CNT_REG,  //          RXP_BASE + 0xB8
    RXP_RADAR_DETECT_STATUS_REG,  //               RXP_BASE + 0xBC
    RXP_RADAR_DETECT_CONTROL_REG,  //              RXP_BASE + 0xC0
    RXP_RADAR_DETECT_LLOGS_REG,  //                RXP_BASE + 0xC4
    RXP_RADAR_DETECT_ULOGS_REG,  //                RXP_BASE + 0xC8
    RXP_RADAR_DETECT_LOGS_REG,  //         RXP_BASE + 0xCC
    RXP_SOFT_PMI_ABORT_REG,  //            RXP_BASE + 0xD0
    RXP_CFG_FLT_TYPE_SUBTYPE_RX_DISABLE0_REG,  //          RXP_BASE + 0xF0
    RXP_CFG_FLT_TYPE_SUBTYPE_RX_DISABLE1_REG,  //          RXP_BASE + 0xF4
    RXP_DIAG_STATUS_REG,  //               RXP_BASE + 0xF8
    RXP_DIAG_TESTBUS_SEL_REG,  //          RXP_BASE + 0xFC
    RXP_FRAME_FILTER_CONFIG_REG  //               RXP_BASE + 0x100
};

tANI_U32 txpRegs[MAX_TXP_REGS] =
{
    TXP_TXP_CMDF_DIN_REG,  //              TXP_BASE + 0x0
    TXP_TXP_CMDF_CONTROL_REG,  //          TXP_BASE + 0x4
    TXP_TXP_TIMEOUT_TIMER_REG,  //         TXP_BASE + 0x8
    TXP_TXP_STATUS_REG,  //                TXP_BASE + 0xC
    TXP_TXP_TESTBUS_REG,  //               TXP_BASE + 0x14
    TXP_RST_STAT_COUNTERS_REG,  //         TXP_BASE + 0x20
    TXP_TXP_NR_PHY_CMDS_REG,  //           TXP_BASE + 0x24
    TXP_TXP_NR_WAIT_CYCLES_CMDS_REG,  //           TXP_BASE + 0x28
    TXP_TXP_NR_WAIT_MTU_CMDS_REG,  //              TXP_BASE + 0x2C
    TXP_TXP_NR_TX_SIZE_CMDS_REG,  //               TXP_BASE + 0x30
    TXP_TXP_NR_TX_BD_CMDS_REG,  //         TXP_BASE + 0x34
    TXP_TXP_NR_TX_SW_CMDS_REG,  //         TXP_BASE + 0x38
    TXP_TXP_NR_FCS_CMDS_REG,  //           TXP_BASE + 0x3C
    TXP_TXP_NR_END_XMIT_CMDS_REG,  //              TXP_BASE + 0x40
    TXP_TXP_TIMER_ANALYSIS_CNT_REG,  //            TXP_BASE + 0x70
    TXP_TXP_TX_INIT_LATE_CNT_REG,  //              TXP_BASE + 0x74
    TXP_TXP_DEBUG_CLEAR_REG,  //           TXP_BASE + 0x80
    TXP_TXP_NR_FRAMES_XMIT_REG,  //                TXP_BASE + 0x84
    TXP_TXP_PHY_ABORTS_REG,  //            TXP_BASE + 0x88
    TXP_TXP_NR_END_INTERRUPTS_REG,  //             TXP_BASE + 0x8C
    TXP_TXP_NR_CMDS_REG,  //               TXP_BASE + 0x90
    TXP_TXP_NR_CMD_WORDS_WRITTEN_REG  //          TXP_BASE + 0x94};
};

tANI_U32 mtuRegs[MAX_MTU_REGS] =
{
    MTU_DIFS_LIMIT_0TO3_REG,  //           MTU_BASE + 0x0
    MTU_DIFS_LIMIT_4TO7_REG,  //           MTU_BASE + 0x4
    MTU_EIFS_PIFS_SLOT_LIMIT_REG   ,  //           MTU_BASE + 0x8
    MTU_SW_MTU_BCN_SLOT_USEC_SIFS_LIMIT_REG,  //           MTU_BASE + 0xC
    MTU_SW_MTU_MISC_LIMITS_REG     ,  //           MTU_BASE + 0x10
    MTU_NAV_CNT_REG,  //           MTU_BASE + 0x14
    MTU_TSF_TIMER_LO_REG   ,  //           MTU_BASE + 0x18
    MTU_TSF_TIMER_HI_REG   ,  //           MTU_BASE + 0x1C
    MTU_TIMER_0_REG,  //           MTU_BASE + 0x20
    MTU_TIMER_1_REG,  //           MTU_BASE + 0x24
    MTU_TIMER_2_REG,  //           MTU_BASE + 0x28
    MTU_TIMER_3_REG,  //           MTU_BASE + 0x2C
    MTU_TIMER_4_REG,  //           MTU_BASE + 0x30
    MTU_TIMER_5_REG,  //           MTU_BASE + 0x34
    MTU_TIMER_6_REG,  //           MTU_BASE + 0x38
    MTU_TIMER_7_REG,  //           MTU_BASE + 0x3C
    MTU_TIMER_CONTROL_REG  ,  //           MTU_BASE + 0x40
    MTU_BKOF_CNT_0_REG     ,  //           MTU_BASE + 0x44
    MTU_BKOF_CNT_1_REG     ,  //           MTU_BASE + 0x48
    MTU_BKOF_CNT_2_REG     ,  //           MTU_BASE + 0x4C
    MTU_BKOF_CNT_3_REG     ,  //           MTU_BASE + 0x50
    MTU_BKOF_CNT_4_REG     ,  //           MTU_BASE + 0x54
    MTU_BKOF_CNT_5_REG     ,  //           MTU_BASE + 0x58
    MTU_BKOF_CNT_6_REG     ,  //           MTU_BASE + 0x5C
    MTU_BKOF_CNT_7_REG     ,  //           MTU_BASE + 0x60
    MTU_BKOF_CONTROL_REG   ,  //           MTU_BASE + 0x64
    MTU_CW_REG_CONTROL_FOR_BACKOFF_0_REG   ,  //           MTU_BASE + 0x68
    MTU_CW_REG_CONTROL_FOR_BACKOFF_1_REG   ,  //           MTU_BASE + 0x6C
    MTU_CW_REG_CONTROL_FOR_BACKOFF_2_REG   ,  //           MTU_BASE + 0x70
    MTU_CW_REG_CONTROL_FOR_BACKOFF_3_REG   ,  //           MTU_BASE + 0x74
    MTU_CW_REG_CONTROL_FOR_BACKOFF_4_REG   ,  //           MTU_BASE + 0x78
    MTU_CW_REG_CONTROL_FOR_BACKOFF_5_REG   ,  //           MTU_BASE + 0x7C
    MTU_CW_REG_CONTROL_FOR_BACKOFF_6_REG   ,  //           MTU_BASE + 0x80
    MTU_CW_REG_CONTROL_FOR_BACKOFF_7_REG   ,  //           MTU_BASE + 0x84
    MTU_MTU_INTERRUPT_STATUS_REG   ,  //           MTU_BASE + 0x88
    MTU_SW_MATCH_REGISTER_0_REG    ,  //           MTU_BASE + 0x8C
    MTU_SW_MATCH_REGISTER_1_REG    ,  //           MTU_BASE + 0x90
    MTU_SW_MATCH_REGISTER_2_REG    ,  //           MTU_BASE + 0x94
    MTU_SW_MATCH_REGISTER_3_REG    ,  //           MTU_BASE + 0x98
    MTU_SW_MATCH_REGISTER_4_REG    ,  //           MTU_BASE + 0x9C
    MTU_SW_MATCH_REGISTER_5_REG    ,  //           MTU_BASE + 0xA0
    MTU_SW_MATCH_REGISTER_6_REG    ,  //           MTU_BASE + 0xA4
    MTU_SW_MATCH_REGISTER_7_REG    ,  //           MTU_BASE + 0xA8
    MTU_SW_CW_MIN_CW_MAX_0_REG     ,  //           MTU_BASE + 0xAC
    MTU_SW_CW_MIN_CW_MAX_1_REG     ,  //           MTU_BASE + 0xB0
    MTU_SW_CW_MIN_CW_MAX_2_REG     ,  //           MTU_BASE + 0xB4
    MTU_SW_CW_MIN_CW_MAX_3_REG     ,  //           MTU_BASE + 0xB8
    MTU_SW_CW_MIN_CW_MAX_4_REG     ,  //           MTU_BASE + 0xBC
    MTU_SW_CW_MIN_CW_MAX_5_REG     ,  //           MTU_BASE + 0xC0
    MTU_SW_CW_MIN_CW_MAX_6_REG     ,  //           MTU_BASE + 0xC4
    MTU_SW_CW_MIN_CW_MAX_7_REG     ,  //           MTU_BASE + 0xC8
    MTU_CCA_COUNTER0_REG           ,  //           MTU_BASE + 0xCC
    MTU_CCA_COUNTER1_REG           ,  //           MTU_BASE + 0xD0
    MTU_CCA_COUNTER2_REG           ,  //           MTU_BASE + 0xD4
    MTU_WATCH_DOG_TIMER_REG,  //           MTU_BASE + 0xD8
    MTU_WATCH_DOG_THR0_REG ,  //           MTU_BASE + 0xDC
    MTU_WATCH_DOG_THR1_REG ,  //           MTU_BASE + 0xE0
    MTU_WATCH_DOG_THR2_REG ,  //           MTU_BASE + 0xE4
    MTU_BKOF_SW_INT_TYPE_REG,  //          MTU_BASE + 0xE8
    MTU_CCA_CONTROL_REG_REG,  //           MTU_BASE + 0xEC
    MTU_TIMER_8_REG,  //           MTU_BASE + 0xF0
    MTU_TIMER_9_REG,  //           MTU_BASE + 0xF4
    MTU_TIMER_10_REG,  //          MTU_BASE + 0xF8
    MTU_TIMER_11_REG,  //          MTU_BASE + 0xFC
    MTU_BKOF_CNT_8_REG     ,  //           MTU_BASE + 0x100
    MTU_BKOF_CNT_9_REG     ,  //           MTU_BASE + 0x104
    MTU_BKOF_CNT_10_REG    ,  //           MTU_BASE + 0x108
    MTU_BKOF_CNT_11_REG    ,  //           MTU_BASE + 0x10C
    MTU_SW_MATCH_REGISTER_8_REG    ,  //           MTU_BASE + 0x110
    MTU_SW_MATCH_REGISTER_9_REG    ,  //           MTU_BASE + 0x114
    MTU_SW_MATCH_REGISTER_10_REG   ,  //           MTU_BASE + 0x118
    MTU_SW_MATCH_REGISTER_11_REG   ,  //           MTU_BASE + 0x11C
    MTU_DIFS_LIMIT_8TO11_REG,  //          MTU_BASE + 0x120
    MTU_CW_REG_CONTROL_FOR_BACKOFF_8_REG   ,  //           MTU_BASE + 0x124
    MTU_CW_REG_CONTROL_FOR_BACKOFF_9_REG   ,  //           MTU_BASE + 0x128
    MTU_CW_REG_CONTROL_FOR_BACKOFF_10_REG  ,  //           MTU_BASE + 0x12C
    MTU_CW_REG_CONTROL_FOR_BACKOFF_11_REG  ,  //           MTU_BASE + 0x130
    MTU_BKOF_CONTROL2_REG  ,  //           MTU_BASE + 0x134
    MTU_CCA_CONTROL_REG2_REG,  //          MTU_BASE + 0x138
    MTU_SW_CW_MIN_CW_MAX_8_REG,  //     ,  //           MTU_BASE + 0x13C
    MTU_SW_CW_MIN_CW_MAX_9_REG     ,  //           MTU_BASE + 0x140
    MTU_SW_CW_MIN_CW_MAX_10_REG    ,  //           MTU_BASE + 0x144
    MTU_SW_CW_MIN_CW_MAX_11_REG    ,  //           MTU_BASE + 0x148
    MTU_TIMER_CONTROL11TO8_REG     ,  //           MTU_BASE + 0x14C
    MTU_MTU_GLOBAL_CONTROL_REG     ,  //           MTU_BASE + 0x150
    MTU_MTU_INTERRUPT_STATUS11TO8_REG //        MTU_BASE + 0x154
};

eANI_DVT_STATUS dvtInitAll(tpAniSirGlobal pMac)
{
    eANI_DVT_STATUS retVal = DVT_STATUS_SUCCESS;



    return (retVal);
}


eANI_DVT_STATUS dvtWriteReg(tpAniSirGlobal pMac, tANI_U32 addr, tANI_U32 value)
{
    eANI_DVT_STATUS retVal = DVT_STATUS_SUCCESS;

    if ( palWriteRegister( pMac->hHdd, addr, value) != eHAL_STATUS_SUCCESS)
        return eHAL_STATUS_FAILURE;

    return (retVal);
}

eANI_DVT_STATUS dvtReadReg(tpAniSirGlobal pMac, tANI_U32 addr, tANI_U32 *value)
{
    eANI_DVT_STATUS retVal = DVT_STATUS_SUCCESS;

    if ( palReadRegister( pMac->hHdd, addr, value) != eHAL_STATUS_SUCCESS)
        retVal = DVT_STATUS_FAILURE;

    return (retVal);
}

eANI_DVT_STATUS dvtWriteMemory(tpAniSirGlobal pMac, tANI_U32 macDestAddr, tANI_U8 *pBuf, tANI_U32 bufSize)
{
    eANI_DVT_STATUS retVal = DVT_STATUS_SUCCESS;

    if ( palWriteDeviceMemory( pMac->hHdd, macDestAddr, pBuf, bufSize ) != eHAL_STATUS_SUCCESS)
        retVal = DVT_STATUS_FAILURE;

    return (retVal);
}

eANI_DVT_STATUS dvtReadMemory(tpAniSirGlobal pMac, tANI_U32 macSourceAddr, tANI_U8 *pBuf, tANI_U32 bufSize)
{
    eANI_DVT_STATUS retVal = DVT_STATUS_SUCCESS;

    if ( palReadDeviceMemory( pMac->hHdd, macSourceAddr, pBuf, bufSize ) != eHAL_STATUS_SUCCESS)
        retVal = DVT_STATUS_FAILURE;

    return (retVal);
}

static char * dpuErrorStrings[] = {
    "FORCE_FRAG_ERR",
    "WATCHDOG_ERR",
    "PDUBD_STALL_ERR",
    "IGR_ERR",
    "EGR_GBI_ERR",
    "EGR_WR_ERR",
    "PKT_BAD_TAG_ERR",
    "PKT_BAD_BD_ERR",
    "PKT_END_MARK_ERR",
    "PKT_MAX_LEN_ERR",
    "PKT_FRAG_CNT_ERR",
    "PKT_BAD_MIC_ERR",
    "PKT_BAD_PPI_ERR",
    "PKT_BAD_DECRYPT_ERR",
    "PKT_ENV_ONLY_ERR",
    "PKT_ENV_PART_ERR",
    "PKT_ZERO_LEN_ERR",
    "PKT_BAD_EXTIV_ERR",
    "PKT_BAD_KID_ERR",
    "PKT_BAD_TSC1_ERR",
    "PKT_UNPROT_ERR",
    "PKT_PROT_ERR",
    "PKT_BAD_RC_ERR",
    "PKT_RC_WRAP_ERR",
    "PKT_STALL_FLUSH",
    "PKT_BD_CHNG_ERR"
};

// DPU Error interrupt handler enrolled by DVT to catch any DPU errors and freeze BMU on
// seeing any DPU errors.
eHalStatus dvtHandleDpuErrorIntr( tHalHandle hHal, eHalIntSources dpuIntr )
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hHal;
    
    dvtLog(pMac, LOGE, "DPU error detected, INTR = %s.\n", 
           dpuErrorStrings[dpuIntr - eHAL_INT_DPU_FORCE_FRAG_ERR]);

    // Freeze BMU
    palWriteRegister(pMac->hHdd, BMU_DISABLE_WQ_DA_REG, 0xfffffc);
    palWriteRegister(pMac->hHdd, BMU_DISABLE_BD_PDU_AVAIL_REG, 0x7ff);

    return eHAL_STATUS_SUCCESS;
}

tANI_U32 rxCalculate32BitCRC(tpAniSirGlobal pMac, tANI_U8 *pBuffer, tANI_U32 ulCount )
{
    register tANI_U32 crc;
    tANI_U8 c;

    crc = 0xFFFFFFFF;

    while( ulCount )
    {
        c = *pBuffer;
        crc = ((crc>>8) & 0x00FFFFFF) ^ pMac->dvt.rxcrcTable[ (crc^c) & 0xFF ];

        ulCount--;
        pBuffer++;
    }

    return( crc^0xFFFFFFFF );
}


void rxCrcGen(tpAniSirGlobal pMac)
{
    tANI_U32 crc, poly;
    int     i, j;

    poly = 0xEDB88320L;

    for (i=0; i<256; i++)
    {
        crc = i;

        for (j=8; j>0; j--)
        {
            if (crc&1)
            {
                crc = (crc >> 1) ^ poly;
            }
            else
            {
                crc >>= 1;
            }
        }

        pMac->dvt.rxcrcTable[i] = crc;
    }
}

#if (WNI_POLARIS_FW_OS == SIR_WINDOWS)

#define ISPRINT(ch) (((ch) >= ' ') && ((ch) <= '~'))
#define PRINTCHAR(ch) (UCHAR)(ISPRINT(ch) ? (ch) : '.')
#define LINELEN 16


void dbgPrintData( unsigned char *bufptr, UINT buflen)
{
  UINT i, linei;


    DbgPrint("0x%08lX [0x%08lX bytes]\n", bufptr, buflen);

    for ( i = 0; i + LINELEN <= (UINT)buflen; i += LINELEN ) {

      UCHAR ch0  = bufptr[ i + 0 ],  ch1  = bufptr[ i + 1 ],  ch2  = bufptr[ i + 2 ],
            ch3  = bufptr[ i + 3 ],  ch4  = bufptr[ i + 4 ],  ch5  = bufptr[ i + 5 ],
            ch6  = bufptr[ i + 6 ],  ch7  = bufptr[ i + 7 ],  ch8  = bufptr[ i + 8 ],
            ch9  = bufptr[ i + 9 ],  ch10 = bufptr[ i + 10 ], ch11 = bufptr[ i + 11 ],
            ch12 = bufptr[ i + 12 ], ch13 = bufptr[ i + 13 ], ch14 = bufptr[ i + 14 ],
            ch15 = bufptr[ i + 15 ];

      DbgPrint("%04X: %02X %02X %02X %02X %02X %02X %02X %02X-"
                                    "%02X %02X %02X %02X %02X %02X %02X %02X"
                                    " %c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n",
                                    i,
                ch0, ch1,  ch2,  ch3,  ch4,  ch5,  ch6, ch7,
                ch8, ch9, ch10, ch11, ch12, ch13, ch14, ch15,
                PRINTCHAR(ch0),  PRINTCHAR(ch1),
                PRINTCHAR(ch2),  PRINTCHAR(ch3),
                PRINTCHAR(ch4),  PRINTCHAR(ch5),
                PRINTCHAR(ch6),  PRINTCHAR(ch7),
                PRINTCHAR(ch8),  PRINTCHAR(ch9),
                PRINTCHAR(ch10), PRINTCHAR(ch11),
                PRINTCHAR(ch12), PRINTCHAR(ch13),
                PRINTCHAR(ch14), PRINTCHAR(ch15) );

    }

    // Print final incomplete line
    DbgPrint("%04X: ", i);

    for (linei = 0; (linei < LINELEN) && (i < buflen); i++, linei++){
      if ( 7 == linei ) {
        DbgPrint("%02X-", (UINT)(bufptr[i]));
      }
      else {
        DbgPrint("%02X ", (UINT)(bufptr[i]));
      }
    }

    i -= linei;
    while (linei++ < LINELEN) DbgPrint("   ");

    for (linei = 0; (linei < LINELEN) && (i < buflen); i++, linei++){
      UCHAR ch = bufptr[i];
      DbgPrint("%c", PRINTCHAR(ch));
    }

    DbgPrint("\r\n");
}

void dbgPrintBD( tSmacBdHostRx *pBD )
{
   DbgPrint("-------- Rx Buffer Descriptor data follows... ------------\n" );

   dbgPrintData( (UCHAR *)pBD, sizeof( *pBD ) );
}

void dbgPrintMPDU( tSmacBdHostRx *pBD )
{
   DbgPrint("-------- Rx MPDU Header data follows... ------------\n" );

   dbgPrintData( (UCHAR *)pBD + pBD->mpduHeaderOffset, pBD->mpduHeaderLength );

   // If there is any MPDU payload print it
   if(pBD->mpduLength > pBD->mpduHeaderLength)
   {
       DbgPrint("\n-------- Rx MPDU Payload data follows... ------------\n" );

       dbgPrintData( (UCHAR *)pBD + pBD->mpduDataOffset, pBD->mpduLength - pBD->mpduHeaderLength );
   }
}

void dbgPrintPacket( tSmacBdHostRx *pBD )
{
    dbgPrintBD(pBD);

    if(pBD->mpduLength > 0)
    {
        dbgPrintMPDU(pBD);
    }
}

#endif// (WNI_POLARIS_FW_OS == SIR_WINDOWS)

tANI_U32 dvtGetHalPhyRate(tANI_U8 rateIdx)
{
    tANI_U32 i;
    for(i = 0;i<NUM_HAL_PHY_RATES;i++)
    {
        if(halPhyRateCodes[i] == rateIdx)
            return i;
    }
    return 0; // return HAL_PHY_RATE_SSF_SIMO_48_MBPS as default
}

void dvtUpdateRxpFlags(tpAniSirGlobal pMac, tANI_U32 rxpFlags, sRxFrameTransfer *pRxFrame)
{
#if defined( ANI_OS_TYPE_WINDOWS )
    if((rxpFlags >> 22) & 1)
        NdisInterlockedIncrement(&pMac->dvt.rxpFlags[pRxFrame->pipe - PIPE_RX_1].fcsEnCount);
    if((rxpFlags >> 21) & 1)
        NdisInterlockedIncrement(&pMac->dvt.rxpFlags[pRxFrame->pipe - PIPE_RX_1].navSetCount);
    if((rxpFlags >> 20) & 1)
        NdisInterlockedIncrement(&pMac->dvt.rxpFlags[pRxFrame->pipe - PIPE_RX_1].navClearedCount);
    if((rxpFlags >> 19) & 1)
        NdisInterlockedIncrement(&pMac->dvt.rxpFlags[pRxFrame->pipe - PIPE_RX_1].txWarmedupCount);
    if((rxpFlags >> 18) & 1)
        NdisInterlockedIncrement(&pMac->dvt.rxpFlags[pRxFrame->pipe - PIPE_RX_1].addr3InvalidCount);
    if((rxpFlags >> 17) & 1)
        NdisInterlockedIncrement(&pMac->dvt.rxpFlags[pRxFrame->pipe - PIPE_RX_1].addr2InvalidCount);
    if((rxpFlags >> 16) & 1)
        NdisInterlockedIncrement(&pMac->dvt.rxpFlags[pRxFrame->pipe - PIPE_RX_1].addr1InvalidCount);
    if((rxpFlags >> 15) & 1)
        NdisInterlockedIncrement(&pMac->dvt.rxpFlags[pRxFrame->pipe - PIPE_RX_1].plcpOverrideCount);
    if((rxpFlags >> 14) & 1)
        NdisInterlockedIncrement(&pMac->dvt.rxpFlags[pRxFrame->pipe - PIPE_RX_1].isAmpduCount);
    if((rxpFlags >> 13) & 1)
        NdisInterlockedIncrement(&pMac->dvt.rxpFlags[pRxFrame->pipe - PIPE_RX_1].isAppduCount);
    if((rxpFlags >> 12) & 1)
        NdisInterlockedIncrement(&pMac->dvt.rxpFlags[pRxFrame->pipe - PIPE_RX_1].isAppdu_lastCount);
    if((rxpFlags >> 11) & 1)
        NdisInterlockedIncrement(&pMac->dvt.rxpFlags[pRxFrame->pipe - PIPE_RX_1].isAppdu_firstCount);
    if((rxpFlags >> 10) & 1)
        NdisInterlockedIncrement(&pMac->dvt.rxpFlags[pRxFrame->pipe - PIPE_RX_1].isAmpdu_lastCount);
    if((rxpFlags >> 9) & 1)
        NdisInterlockedIncrement(&pMac->dvt.rxpFlags[pRxFrame->pipe - PIPE_RX_1].isAmpdu_firstCount);
    if((rxpFlags >> 8) & 1)
        NdisInterlockedIncrement(&pMac->dvt.rxpFlags[pRxFrame->pipe - PIPE_RX_1].hasPhycmdCount);
    if((rxpFlags >> 7) & 1)
        NdisInterlockedIncrement(&pMac->dvt.rxpFlags[pRxFrame->pipe - PIPE_RX_1].hasPhystatsCount);
    if((rxpFlags >> 6) & 1)
        NdisInterlockedIncrement(&pMac->dvt.rxpFlags[pRxFrame->pipe - PIPE_RX_1].hasDlmCount);
    if((rxpFlags >> 5) & 1)
        NdisInterlockedIncrement(&pMac->dvt.rxpFlags[pRxFrame->pipe - PIPE_RX_1].bypassDlmprocCount);
    if((rxpFlags >> 4) & 1)
        NdisInterlockedIncrement(&pMac->dvt.rxpFlags[pRxFrame->pipe - PIPE_RX_1].bypassMpduprocCount);
    if((rxpFlags >> 3) & 1)
        NdisInterlockedIncrement(&pMac->dvt.rxpFlags[pRxFrame->pipe - PIPE_RX_1].failFilterCount);
    if((rxpFlags >> 2) & 1)
        NdisInterlockedIncrement(&pMac->dvt.rxpFlags[pRxFrame->pipe - PIPE_RX_1].failMaxPktLenCount);
    if((rxpFlags >> 1) & 1)
        NdisInterlockedIncrement(&pMac->dvt.rxpFlags[pRxFrame->pipe - PIPE_RX_1].fcsErrorCount);
    if((rxpFlags) & 1)
        NdisInterlockedIncrement(&pMac->dvt.rxpFlags[pRxFrame->pipe - PIPE_RX_1].exceptionCount);
#else
    if((rxpFlags >> 22) & 1)
        pMac->dvt.rxpFlags[pRxFrame->pipe - PIPE_RX_1].fcsEnCount ++ ;
    if((rxpFlags >> 21) & 1)
        pMac->dvt.rxpFlags[pRxFrame->pipe - PIPE_RX_1].navSetCount ++ ;
    if((rxpFlags >> 20) & 1)
        pMac->dvt.rxpFlags[pRxFrame->pipe - PIPE_RX_1].navClearedCount ++ ;
    if((rxpFlags >> 19) & 1)
        pMac->dvt.rxpFlags[pRxFrame->pipe - PIPE_RX_1].txWarmedupCount ++ ;
    if((rxpFlags >> 18) & 1)
        pMac->dvt.rxpFlags[pRxFrame->pipe - PIPE_RX_1].addr3InvalidCount ++ ;
    if((rxpFlags >> 17) & 1)
        pMac->dvt.rxpFlags[pRxFrame->pipe - PIPE_RX_1].addr2InvalidCount ++ ;
    if((rxpFlags >> 16) & 1)
        pMac->dvt.rxpFlags[pRxFrame->pipe - PIPE_RX_1].addr1InvalidCount ++ ;
    if((rxpFlags >> 15) & 1)
        pMac->dvt.rxpFlags[pRxFrame->pipe - PIPE_RX_1].plcpOverrideCount ++ ;
    if((rxpFlags >> 14) & 1)
        pMac->dvt.rxpFlags[pRxFrame->pipe - PIPE_RX_1].isAmpduCount ++ ;
    if((rxpFlags >> 13) & 1)
        pMac->dvt.rxpFlags[pRxFrame->pipe - PIPE_RX_1].isAppduCount ++ ;
    if((rxpFlags >> 12) & 1)
        pMac->dvt.rxpFlags[pRxFrame->pipe - PIPE_RX_1].isAppdu_lastCount ++ ;
    if((rxpFlags >> 11) & 1)
        pMac->dvt.rxpFlags[pRxFrame->pipe - PIPE_RX_1].isAppdu_firstCount ++ ;
    if((rxpFlags >> 10) & 1)
        pMac->dvt.rxpFlags[pRxFrame->pipe - PIPE_RX_1].isAmpdu_lastCount ++ ;
    if((rxpFlags >> 9) & 1)
        pMac->dvt.rxpFlags[pRxFrame->pipe - PIPE_RX_1].isAmpdu_firstCount ++ ;
    if((rxpFlags >> 8) & 1)
        pMac->dvt.rxpFlags[pRxFrame->pipe - PIPE_RX_1].hasPhycmdCount ++ ;
    if((rxpFlags >> 7) & 1)
        pMac->dvt.rxpFlags[pRxFrame->pipe - PIPE_RX_1].hasPhystatsCount ++ ;
    if((rxpFlags >> 6) & 1)
        pMac->dvt.rxpFlags[pRxFrame->pipe - PIPE_RX_1].hasDlmCount ++ ;
    if((rxpFlags >> 5) & 1)
        pMac->dvt.rxpFlags[pRxFrame->pipe - PIPE_RX_1].bypassDlmprocCount ++ ;
    if((rxpFlags >> 4) & 1)
        pMac->dvt.rxpFlags[pRxFrame->pipe - PIPE_RX_1].bypassMpduprocCount ++ ;
    if((rxpFlags >> 3) & 1)
        pMac->dvt.rxpFlags[pRxFrame->pipe - PIPE_RX_1].failFilterCount ++ ;
    if((rxpFlags >> 2) & 1)
        pMac->dvt.rxpFlags[pRxFrame->pipe - PIPE_RX_1].failMaxPktLenCount ++ ;
    if((rxpFlags >> 1) & 1)
        pMac->dvt.rxpFlags[pRxFrame->pipe - PIPE_RX_1].fcsErrorCount ++ ;
    if((rxpFlags) & 1)
        pMac->dvt.rxpFlags[pRxFrame->pipe - PIPE_RX_1].exceptionCount ++ ;
#endif

}



// =======================================================================================
//
// all of the MAC_ADDRESS stuff has been eliminated and converted to use palProtTypes.h
// This code is still using the 'old' MAC_ADDRESS stuff so i've put these in here to 
// get them to build.   This all should be converted to palProtTypes at some point.
//
// =======================================================================================
typedef struct tMAC_ADDRESS 
{
  unsigned char Addr[ 6 ];

} MAC_ADDRESS;


static __inline tANI_BOOLEAN IsMacAddressGroup( MAC_ADDRESS *pMACAddr )
{
  return( pMACAddr->Addr[ 0 ] & 0x01 );
}




static __inline tANI_BOOLEAN IsMacAddressBroadcast( MAC_ADDRESS *pMacAddr )
{
  return( (tANI_BOOLEAN)( ( 0xffffffff == *                      (unsigned long *)pMacAddr ) &&
                     ( 0xffff     == *(unsigned short *)( ( (unsigned long *)pMacAddr ) + 1 ) ) ) );

}

// =======================================================================================
//
// all of the MAC_ADDRESS stuff has been eliminated and converted to use palProtTypes.h
// This code is still using the 'old' MAC_ADDRESS stuff so i've put these in here to 
// get them to build.   This all should be converted to palProtTypes at some point.
//
// =======================================================================================


void dvtUpdateRxDataFrames(tpAniSirGlobal pMac, tSirMacMgmtHdr_L *mac)
{
    if(mac->fc.type != 2) //it is not a data frame
        return;

#if defined( ANI_OS_TYPE_WINDOWS )
    NdisInterlockedIncrement(&pMac->dvt.cRxDataFrames[(mac->fc.subType) % 16]);
#else
    pMac->dvt.cRxDataFrames[(mac->fc.subType) % 16] ++;
#endif

    // Update the Multicast / Broadcast frame counter
    if(IsMacAddressBroadcast((MAC_ADDRESS *)mac->da))
    {
#if defined( ANI_OS_TYPE_WINDOWS )
        NdisInterlockedIncrement(&pMac->dvt.cRxBroadcastFrames);
#else
        pMac->dvt.cRxBroadcastFrames ++;
#endif        
    }
    else if(IsMacAddressGroup((MAC_ADDRESS *)mac->da))
    {
#if defined( ANI_OS_TYPE_WINDOWS )
        NdisInterlockedIncrement(&pMac->dvt.cRxMulticastFrames);
#else
        pMac->dvt.cRxMulticastFrames ++;
#endif        
    }
    else
    {
#if defined( ANI_OS_TYPE_WINDOWS )
        NdisInterlockedIncrement(&pMac->dvt.cRxUnicastFrames);
#else
        pMac->dvt.cRxUnicastFrames ++;
#endif        
    }
}

void dvtUpdateRxCtrlMgmtFrames(tpAniSirGlobal pMac, tSirMacMgmtHdr_L *mac)
{
    if(mac->fc.type == 0)
    {
        //management
#if defined( ANI_OS_TYPE_WINDOWS )
        NdisInterlockedIncrement(&pMac->dvt.cRxMgmtFrames);
#else
        pMac->dvt.cRxMgmtFrames ++;
#endif
        if(mac->fc.subType == 8)
        {
            //beacon
#if defined( ANI_OS_TYPE_WINDOWS )
            NdisInterlockedIncrement(&pMac->dvt.cRxBeaconFrames);
#else
            pMac->dvt.cRxBeaconFrames ++;
#endif
        }
    }

    if(mac->fc.type == 1)
    {
        //control
#if defined( ANI_OS_TYPE_WINDOWS )
        NdisInterlockedIncrement(&pMac->dvt.cRxControlFrames);
#else
        pMac->dvt.cRxControlFrames ++;
#endif
    }
}

tANI_U32 dvtGetBitValue(tANI_U32 dWord, tANI_U32 mask, tANI_U32 offset)
{
    return( (dWord & mask) >> offset);
}

tANI_U8 dvtComposeRateIdx(eDVTPhyModeRates mode, tDvtPmiRateIdxBits *rIdxBits)
{
    tANI_U8 rateIdx = 0;
    switch (mode)
    {
        case DVT_PHY_MODE_RATE_11_A_TITAN_NON_DUPLICATE:
        {
            rateIdx = (0 << 7) | (rIdxBits->concat_packet << 6) | ((rIdxBits->bandwidth_mode & 0x1) << 5) | ((rIdxBits->nss_11b & 0x1) << 4) | rIdxBits->psdu_rate;
            break;
        }
        case DVT_PHY_MODE_RATE_11_N_MCS_0_15_20MHZ:
        {
            rateIdx = (1 << 7) | (0 << 6) | ((rIdxBits->bandwidth_mode & 0x1) << 5) | (rIdxBits->psdu_rate << 1) | rIdxBits->short_guard_interval;
            break;
        }
        case DVT_PHY_MODE_RATE_11_N_MCS_0_15_40MHZ:
        {
            rateIdx = (1 << 7) | (0 << 6) | ((rIdxBits->bandwidth_mode & 0x1) << 5) | (rIdxBits->psdu_rate << 1) | rIdxBits->short_guard_interval;
            break;
        }
        case DVT_PHY_MODE_RATE_11_A_TITAN_LEGACY_DUPLICATE:
        {
            rateIdx = (1 << 7) | (1 << 6) | (0 << 5) | (0 << 4) | (0 << 3) | (rIdxBits->psdu_rate & 0x7);
            break;
        }
        case DVT_PHY_MODE_RATE_11_N_MCS_32_40MHZ_DUPLICATE:
        {
            rateIdx = (1 << 7) | (1 << 6) | (0 << 5) | (0 << 4) | (1 << 3) | (0 << 2) | (0 << 1) | rIdxBits->short_guard_interval;
            break;
        }
        case DVT_PHY_MODE_RATE_11_N_AIRGO_PROPRIETARY:
        {
            rateIdx = (1 << 7) | (1 << 6) | (0 << 5) | (0 << 4) | (1 << 3) | (1 << 2) | ((rIdxBits->bandwidth_mode & 0x1) << 1) | rIdxBits->short_guard_interval;
            break;
        }
        case DVT_PHY_MODE_RATE_11_B:
        {
            rateIdx = (1 << 7) | (1 << 6) | ( 0 << 5) | (1 << 4) | ((rIdxBits->psdu_rate & 0x3)<< 2) | (0 << 1) | (rIdxBits->nss_11b & 0x1);
            break;
        }
    }

    return rateIdx;
}

eANI_DVT_STATUS dvtCheckRateIdxOnRxPmiCmd(tANI_U32 *rxPmiCmd, tANI_U8 bdRateIdx)
{
    //byte0 : bit [1] -- Concat_packet --> i.e., 1st bit in the first DWord
    //byte1 : bit [7] -- short_guard_interval --> i.e., 15th bit in the first DWord
    //byte1 : bit [6,5] -- bandwidth_mode --> i.e., 14th bit and 13th in the first DWord
    //byte1 : bit [2,1,0] -- pkt_type --> i.e., 10th bit, 9th bit and 8th in the first DWord
    //byte2 : bit [7,6] -- Nss   --> i.e., 23rd and 22nd bits in the first DWord
    //byte2 : bit [4] -- airgo_11n_rates   --> i.e., 20th bit in the first DWord
    //byte2 : bit [3,2,1,0] -- PSDU rate --> i.e., 19th, 18th, 17th and 16th bits in the first DWord
    //byte4 : bit [7,6,5,4] -- PPDU_rate --> i.e., 7th, 6th, 5th, 4th bits in the second DWord

    eANI_DVT_STATUS retVal = DVT_STATUS_SUCCESS;
    tANI_U8 pkt_type, concat_packet, bandwidth_mode, nss_11b, psdu_rate, short_guard_interval, airgo_11n_rates, ppdu_rate, pmiRateIdx;
    tANI_U32 dWord_0, dWord_1;
    tDvtPmiRateIdxBits rIdxBits;
    eDVTPhyModeRates mode;

    dWord_0 = *rxPmiCmd;
    dWord_1 = *(++rxPmiCmd);

    rIdxBits.concat_packet = concat_packet =               (tANI_U8) dvtGetBitValue(dWord_0, PMI_CMD_RATE_IDX_CONCAT_PKT_MASK, PMI_CMD_RATE_IDX_CONCAT_PKT_OFFSET);
    rIdxBits.short_guard_interval = short_guard_interval = (tANI_U8) dvtGetBitValue(dWord_0, PMI_CMD_RATE_IDX_SHORT_GUARD_INTERVAL_MASK, PMI_CMD_RATE_IDX_SHORT_GUARD_INTERVAL_OFFSET);
    rIdxBits.bandwidth_mode = bandwidth_mode =             (tANI_U8) dvtGetBitValue(dWord_0, PMI_CMD_RATE_IDX_BANDWIDTH_MODE_MASK, PMI_CMD_RATE_IDX_BANDWIDTH_MODE_OFFSET);
    pkt_type =                                             (tANI_U8) dvtGetBitValue(dWord_0, PMI_CMD_RATE_IDX_PKT_TYPE_MASK, PMI_CMD_RATE_IDX_PKT_TYPE_OFFSET);
    rIdxBits.nss_11b = nss_11b =                           (tANI_U8) dvtGetBitValue(dWord_0, PMI_CMD_RATE_IDX_NSS_11B_MASK, PMI_CMD_RATE_IDX_NSS_11B_OFFSET);
    airgo_11n_rates =                                      (tANI_U8) dvtGetBitValue(dWord_0, PMI_CMD_RATE_IDX_AIRGO_11N_RATES_MASK, PMI_CMD_RATE_IDX_AIRGO_11N_RATES_OFFSET);
    rIdxBits.psdu_rate = psdu_rate =                       (tANI_U8) dvtGetBitValue(dWord_0, PMI_CMD_RATE_IDX_PSDU_RATE_MASK, PMI_CMD_RATE_IDX_PSDU_RATE_OFFSET);
    rIdxBits.ppdu_rate = ppdu_rate =                       (tANI_U8) dvtGetBitValue(dWord_1, PMI_CMD_RATE_IDX_PPDU_RATE_MASK, PMI_CMD_RATE_IDX_PPDU_RATE_OFFSET);
/*
    rIdxBits.concat_packet = concat_packet =               (tANI_U8) ((dWord_0 >> 1) & 0x1);
    rIdxBits.short_guard_interval = short_guard_interval = (tANI_U8) ((dWord_0 >> 15) & 0x1);
    rIdxBits.bandwidth_mode = bandwidth_mode =             (tANI_U8) ((dWord_0 >> 13) & 0x3);
    pkt_type =                                             (tANI_U8) ((dWord_0 >> 8) & 0x7);
    rIdxBits.nss_11b = nss_11b =                           (tANI_U8) ((dWord_0 >> 22) & 0x3);
    airgo_11n_rates =                                      (tANI_U8) ((dWord_0 >> 20) & 0x1);
    rIdxBits.psdu_rate = psdu_rate =                       (tANI_U8) ((dWord_0 >> 16) & 0xf);
    rIdxBits.ppdu_rate = ppdu_rate =                       (tANI_U8) ((dWord_1 >> 4) & 0xf);
*/
    //Compose rate
    if(pkt_type == 1) //11b rates
    {
        mode = DVT_PHY_MODE_RATE_11_B;
    }
    else if(pkt_type == 0)
    {
        // 11a Titan Rates (non-duplicate mode)
        if ((bandwidth_mode >> 1) == 0)
            mode = DVT_PHY_MODE_RATE_11_A_TITAN_NON_DUPLICATE;
        // 11a Titan Legacy Duplicate mode
      else if ((bandwidth_mode >> 1) == 1)
          mode = DVT_PHY_MODE_RATE_11_A_TITAN_LEGACY_DUPLICATE;
    }
    else if((pkt_type == 2) ||(pkt_type == 3))
    {
    // 11n Airgo Proprietary Rates
      if (airgo_11n_rates == 1)
         mode = DVT_PHY_MODE_RATE_11_N_AIRGO_PROPRIETARY;
      // 11n MCS#32 40MHz duplicate
      else if ((bandwidth_mode >> 1) == 1)
         mode = DVT_PHY_MODE_RATE_11_N_MCS_32_40MHZ_DUPLICATE;
    // 11n MCS#0-15 for 20MHz
      else if ((bandwidth_mode & 0x1) == 0)
        mode = DVT_PHY_MODE_RATE_11_N_MCS_0_15_20MHZ;
      // 11n MCS#0-15 for 40MHz
      else if ((bandwidth_mode & 0x1) == 1)
        mode = DVT_PHY_MODE_RATE_11_N_MCS_0_15_40MHZ;
    }

    if(dvtComposeRateIdx(mode, &rIdxBits) != bdRateIdx)
        return eHAL_STATUS_FAILURE;

    return (retVal);
}

static void dvtStoreDxeTimestampInfo(tpAniSirGlobal pMac, tDvtPktDxeTimestampStats *pDxeTimeStampinfo, tANI_U32 prevH2bEndTimeStamp, tANI_U32 prevB2hEndTimeStamp, tSmacBdHostTx *pBD)
{
    tANI_U32 readTimeStamp;
    pDxeTimeStampinfo->h2bEndTimeStamp = pBD->dxeH2BEndTimestamp;
    pDxeTimeStampinfo->b2hEndTimeStamp = pBD->dxeB2HEndTimestamp;
    pDxeTimeStampinfo->h2bStartDelayAfterDxeTrigger = pBD->dxeH2BStartTimestamp - pBD->swTimestamp;
#if !defined(ANI_BUS_TYPE_USB)
    palReadRegister(pMac->hHdd, DXE_0_TIMESTAMP_REG, &readTimeStamp);
    pDxeTimeStampinfo->rxProcessingDelay = readTimeStamp - pBD->dxeB2HEndTimestamp;
#endif
    pDxeTimeStampinfo->h2bStartDelayFromPrevPkt = pBD->dxeH2BStartTimestamp - prevH2bEndTimeStamp;
    pDxeTimeStampinfo->b2hStartDelayFromPrevPkt = pBD->dxeB2HStartTimestamp - prevB2hEndTimeStamp;
    pDxeTimeStampinfo->h2bXferTime = pBD->dxeH2BEndTimestamp - pBD->dxeH2BStartTimestamp;
    pDxeTimeStampinfo->bmuIdleTime = pBD->dxeB2HStartTimestamp - pBD->dxeH2BEndTimestamp;
    pDxeTimeStampinfo->b2hXferTime = pBD->dxeB2HEndTimestamp - pBD->dxeB2HStartTimestamp;
}

#define DVT_INCR_DXE_TIME_HIST_INDX(_indx) if( ++(_indx) == DVT_MAX_DXE_PKT_HISTORY) (_indx) = 0

static void dvtAddDxePktTimestampInfo(tpAniSirGlobal pMac, tSmacBdHostTx *pBD, ePipes pipe)
{
    int prevPktIndx;
    tANI_U32 prevH2bEndTimeStamp, prevB2hEndTimeStamp;
    pipe = pipe-PIPE_RX_1;
    prevPktIndx = pMac->dvt.dxePktTailIndx[pipe];

    if(-1 == prevPktIndx)
    {
        prevH2bEndTimeStamp = prevB2hEndTimeStamp = 0;
    }
    else
    {
        prevH2bEndTimeStamp = pMac->dvt.dxePktTimeHistogram[pipe][prevPktIndx].h2bEndTimeStamp;
        prevB2hEndTimeStamp = pMac->dvt.dxePktTimeHistogram[pipe][prevPktIndx].b2hEndTimeStamp;
    }

    DVT_INCR_DXE_TIME_HIST_INDX(pMac->dvt.dxePktTailIndx[pipe]);
    if((-1 == pMac->dvt.dxePktHeadIndx[pipe]) ||
       (pMac->dvt.dxePktTailIndx[pipe] == pMac->dvt.dxePktHeadIndx[pipe]))
    {
        DVT_INCR_DXE_TIME_HIST_INDX(pMac->dvt.dxePktHeadIndx[pipe]);
    }
    dvtStoreDxeTimestampInfo(pMac,
                             &pMac->dvt.dxePktTimeHistogram[pipe][pMac->dvt.dxePktTailIndx[pipe]],
                             prevH2bEndTimeStamp,
                             prevB2hEndTimeStamp,
                             pBD);
}

void dvtDumpDxePktTimeHistogram(tpAniSirGlobal pMac)
{
    int currIndx;
    int i;

    for(i=0; i < MAX_RX_PIPES; i++)
    {
        if (-1 != pMac->dvt.dxePktHeadIndx[i]) {
            dvtLog(pMac, LOGW, "\nDumping timestamp histogram for Rx pipe %d\n", i+1);
            dvtLog(pMac, LOGW, "Head Index = %d, Tail Index = %d\n\n", 
                   pMac->dvt.dxePktHeadIndx[i], pMac->dvt.dxePktTailIndx[i]);
    
            dvtLog(pMac, LOGW, "H2B Start Delay\tSW Rx processing Delay\tH2B Start Delay From Prev Pkt\tB2H Start Delay From Prev Pkt\tH2B Xfer Time\tBMU Idle Time\tB2H Xfer Time\n");
            dvtLog(pMac, LOGW, "----------------\t---------------------\t---------------------------\t-----------------------------\t-------------\t-------------\t-------------\n\n");
    
            currIndx = pMac->dvt.dxePktHeadIndx[i] - 1;
    
            do
            {
                DVT_INCR_DXE_TIME_HIST_INDX(currIndx);
    
                dvtLog(pMac, LOGW, "%010d\t%010d\t%010d\t%010d\t%010d\t%010d\t%010d\n",
                       pMac->dvt.dxePktTimeHistogram[i][currIndx].h2bStartDelayAfterDxeTrigger,
                       pMac->dvt.dxePktTimeHistogram[i][currIndx].rxProcessingDelay,
                       pMac->dvt.dxePktTimeHistogram[i][currIndx].h2bStartDelayFromPrevPkt,
                       pMac->dvt.dxePktTimeHistogram[i][currIndx].b2hStartDelayFromPrevPkt,
                       pMac->dvt.dxePktTimeHistogram[i][currIndx].h2bXferTime,
                       pMac->dvt.dxePktTimeHistogram[i][currIndx].bmuIdleTime,
                       pMac->dvt.dxePktTimeHistogram[i][currIndx].b2hXferTime);
            }
            while(currIndx != pMac->dvt.dxePktTailIndx[i]);
        }

    }
}

void dvtCheckCrcOnRxFrame(tpAniSirGlobal pMac, sRxFrameTransfer *pRxFrame)
{
    tANI_U32 crcInFrame, crcCalculated, pktSize;
    tSmacBdHostRx *pBD;
    tSmacBdHostTx *pTxBD; // This variable is used for generating the DXE timestamp histogram
    tSmacBdHostCtlMesg *pCtrlBD; // This variable is used to filter out mailbox message responses from SoftMAC
    tSirMacMgmtHdr_L *mac;
    tANI_U8 *pPayload;
    tANI_U32 *pPayloadCrc, seqNo, payloadLength, *leftOverBD;
    tANI_U8 bdDumpBuf[128];
    eHalPhyRates phyRate;
    tANI_U32 pmiOffset = offsetof(tSmacBdHostRx, rxPmiCmd);
    tANI_U32 ctrlBdLength = sizeof(tSmacBdHostCtlMesg);

    pBD = (tSmacBdHostRx *)pRxFrame->rxFrame->pBbuffer;
    pTxBD = (tSmacBdHostTx *)pBD;
    pCtrlBD = (tSmacBdHostCtlMesg *)pBD;

    // First SWAP only upto the CTRL BD size, in case this is a CTRL BD
    utilByteSwapU32((tANI_U32 *)pCtrlBD, ctrlBdLength);

    leftOverBD = (tANI_U32 *)((tANI_U8 *)(pCtrlBD) + ctrlBdLength);

    // Now check if this is a Control BD
    if ( SMAC_SWBD_TYPE_CTLMSG == pCtrlBD->swBdType )
    {
        // Ship the CTRL BD and return
        sysBbtProcessMessage(pMac, (tpHalBufDesc)pCtrlBD);
        return;
    }

    // If DXE timestamping is enabled only DXE loopback
    // is being used, so the entire BD needs to be swapped
    if(pMac->dvt.dxeTimestampEnable)
    {
        utilByteSwapU32(leftOverBD, offsetof(tSmacBdHostTx, swTimestamp) - ctrlBdLength);
    }
    else
    {
        // Do not byte swap rxPmiCmd in Rx BD
        utilByteSwapU32(leftOverBD, pmiOffset - ctrlBdLength);
        leftOverBD = (tANI_U32 *)((tANI_U8 *)(pBD) + (pmiOffset + 24));
        if(pBD->mpduHeaderOffset)
            utilByteSwapU32(leftOverBD, pBD->mpduHeaderOffset - (pmiOffset + 24));
        else
            utilByteSwapU32(leftOverBD, sizeof(tSmacBdHostRx) - (pmiOffset + 24));
    }

    pktSize = pBD->mpduDataOffset + (pBD->mpduLength - pBD->mpduHeaderLength);
    phyRate = (eHalPhyRates)dvtGetHalPhyRate((tANI_U8)pBD->rateIndex);
    mac = (tSirMacMgmtHdr_L *)((tANI_U8 *)pBD + pBD->mpduHeaderOffset);

    if(pMac->dvt.dxeTimestampEnable)
    {
        dvtAddDxePktTimestampInfo(pMac, pTxBD, pRxFrame->pipe);
    }
    // Since the Rx PMI fields in the Rx BD are also overloaded as the B2H DXE timestamp
    // fields the Rx PMI command checking is done only if DXE timestamping is not enabled.
    else
    if(dvtCheckRateIdxOnRxPmiCmd(pBD->rxPmiCmd, (tANI_U8)pBD->rateIndex) != DVT_STATUS_SUCCESS)
    {
#if defined( ANI_OS_TYPE_WINDOWS )
        NdisInterlockedIncrement(&pMac->dvt.cRxPMIRateIdxMisMatchCount[(eHalPhyRates)dvtGetHalPhyRate((tANI_U8)pBD->rateIndex)]);
#else
        pMac->dvt.cRxPMIRateIdxMisMatchCount[(eHalPhyRates)dvtGetHalPhyRate((tANI_U8)pBD->rateIndex)] ++;
#endif
    }

    dvtUpdateRxDataFrames(pMac, mac);
    dvtUpdateRxpFlags(pMac, pBD->rxpFlags, pRxFrame);

    if( (pBD->mpduHeaderLength != 0) && (pBD->mpduHeaderOffset != 0) && (pBD->isAmsduSubframe == 0) )
    {
        if((mac->fc.type == 0) || (mac->fc.type == 1)) // Control or Management frame
        {
            dvtUpdateRxCtrlMgmtFrames(pMac, mac);
            return; // Do not compute crc
        }
    }

    if(pBD->swBdType == SMAC_SWBD_TYPE_JUNK)
    {
#if defined( ANI_OS_TYPE_WINDOWS )
        NdisInterlockedIncrement(&pMac->dvt.dvtInfo.rxInfo.cRxAMSDUSubFrameDrpCounts[pRxFrame->pipe - PIPE_RX_1]);
        NdisInterlockedIncrement(&pMac->dvt.dvtInfo.rxInfo.cRXErrCounts[pRxFrame->pipe - PIPE_RX_1]);
#else
        pMac->dvt.dvtInfo.rxInfo.cRxAMSDUSubFrameDrpCounts[pRxFrame->pipe - PIPE_RX_1] ++;
        pMac->dvt.dvtInfo.rxInfo.cRXErrCounts[pRxFrame->pipe - PIPE_RX_1] ++;
#endif
        return; // Do not compute crc when swBdType is 2 i.e., SMAC_SWBD_TYPE_RESERVED
    }else if (pBD->swBdType == SMAC_SWBD_TYPE_CTLMSG){
        /* bypass HAL control messages from SOftmac */
        return;        
    }

    if (pMac->dvt.crcEnable == 0) //no crc checking
    {
#if defined( ANI_OS_TYPE_WINDOWS )
        NdisInterlockedIncrement(&pMac->dvt.dvtInfo.rxInfo.cRxOKCounts[pRxFrame->pipe - PIPE_RX_1]);
        NdisInterlockedIncrement(&pMac->dvt.dvtInfo.rxInfo.cRxCountPerTid[pBD->tid % MAX_TIDS]);
        NdisInterlockedIncrement(&pMac->dvt.cRxCountPerRate[phyRate][pRxFrame->pipe - PIPE_RX_1]);
        NdisInterlockedIncrement(&pMac->dvt.cRxCountperSta[pBD->tid % MAX_TIDS][pBD->addr1Index % NUM_DEST_STATIONS]);
#else
        pMac->dvt.dvtInfo.rxInfo.cRxOKCounts[pRxFrame->pipe - PIPE_RX_1] ++;
        pMac->dvt.dvtInfo.rxInfo.cRxCountPerTid[pBD->tid % MAX_TIDS] ++;
        pMac->dvt.cRxCountPerRate[phyRate][pRxFrame->pipe - PIPE_RX_1] ++;
        pMac->dvt.cRxCountperSta[pBD->tid % MAX_TIDS][pBD->addr1Index % NUM_DEST_STATIONS] ++;
#endif
        return;
    }

#if (WNI_POLARIS_FW_OS == SIR_WINDOWS)
    //dump packet contents
//    dbgPrintPacket(pBD);
#endif

    pPayload = (tANI_U8 *)((tANI_U32)pBD + pBD->mpduDataOffset);
    payloadLength = pBD->mpduLength - pBD->mpduHeaderLength;

    if(pBD->isAmsduSubframe)
    {
        // Make adjustments to skip the AMSDU sub-frame header
        pPayload += 14;
        payloadLength -= 14;
    }

    if (payloadLength < sizeof(crcInFrame))
    {
#if defined( ANI_OS_TYPE_WINDOWS )
        NdisInterlockedIncrement(&pMac->dvt.dvtInfo.rxInfo.cRxOKCounts[pRxFrame->pipe - PIPE_RX_1]);
        NdisInterlockedIncrement(&pMac->dvt.dvtInfo.rxInfo.cRxCountPerTid[pBD->tid % MAX_TIDS]);
        NdisInterlockedIncrement(&pMac->dvt.cRxCountPerRate[phyRate][pRxFrame->pipe - PIPE_RX_1]);
        NdisInterlockedIncrement(&pMac->dvt.cRxCountperSta[pBD->tid % MAX_TIDS][pBD->addr1Index % NUM_DEST_STATIONS]);
#else
        pMac->dvt.dvtInfo.rxInfo.cRxOKCounts[pRxFrame->pipe - PIPE_RX_1] ++;
        pMac->dvt.dvtInfo.rxInfo.cRxCountPerTid[pBD->tid % MAX_TIDS] ++;
        pMac->dvt.cRxCountPerRate[phyRate][pRxFrame->pipe - PIPE_RX_1] ++;
        pMac->dvt.cRxCountperSta[pBD->tid % MAX_TIDS][pBD->addr1Index % NUM_DEST_STATIONS] ++;
#endif
        return;
    }

    // CRC is now stored at the beginning of the packet
    pPayloadCrc = (tANI_U32 *)pPayload;
    crcInFrame = *pPayloadCrc;

    *pPayloadCrc = 0;
    crcCalculated = rxCalculate32BitCRC(pMac, pPayload, payloadLength );
    *pPayloadCrc = crcInFrame;

    if (crcCalculated != crcInFrame)
    {
#ifdef FREEZE_BMU_ON_CRC_ERR
        // Disable all the BMU WQs to aid in debugging the problem
        palWriteRegister(pMac->hHdd, BMU_DISABLE_BD_PDU_AVAIL_REG, 0x7ff);
        palWriteRegister(pMac->hHdd, BMU_DISABLE_WQ_DA_REG, 0x3fffffc);
#endif

#if defined( ANI_OS_TYPE_WINDOWS )
        NdisInterlockedIncrement(&pMac->dvt.dvtInfo.rxInfo.cRXErrCounts[pRxFrame->pipe - PIPE_RX_1]);
        NdisInterlockedIncrement(&pMac->dvt.dvtInfo.rxInfo.cRxErrCountPerTid[pBD->tid % MAX_TIDS]);
#else
        pMac->dvt.dvtInfo.rxInfo.cRXErrCounts[pRxFrame->pipe - PIPE_RX_1] ++;
        pMac->dvt.dvtInfo.rxInfo.cRxErrCountPerTid[pBD->tid % MAX_TIDS] ++;
#endif

#if (WNI_POLARIS_FW_OS == SIR_WINDOWS)

        seqNo = *((tANI_U32*)&((tANI_U8 *)pBD)[pBD->mpduDataOffset + 4]);

        KdPrint(("dvtCheckCrcOnRxFrame(): dumping CRC corrupted frame... seq. # %d\n", seqNo));
        dbgPrintPacket(pBD);

    //We cannot access CSR endpoint here
    #if !defined(ANI_BUS_TYPE_USB)
        // Now retrieve the first PDU for this packet and dump it.
        palReadDeviceMemory(pMac->hHdd, BMU_MEMORY_BASE_ADDRESS + BMU_PACKET_MEMORY_OFFSET + pBD->headPduIdx * 0x80,
                            bdDumpBuf, 0x80);


        KdPrint(("dvtCheckCrcOnRxFrame(): dumping first PDU of corrupted packet in BMU memory, at index 0x%x\n", pBD->headPduIdx));
        dbgPrintData(bdDumpBuf, 0x80);

        // Now retrieve the last PDU for this packet and dump it.
        palReadDeviceMemory(pMac->hHdd, BMU_MEMORY_BASE_ADDRESS + BMU_PACKET_MEMORY_OFFSET + pBD->tailPduIdx * 0x80,
                            bdDumpBuf, 0x80);


        KdPrint(("dvtCheckCrcOnRxFrame(): dumping last PDU of corrupted packet in BMU memory, at index 0x%x\n", pBD->tailPduIdx));
        dbgPrintData(bdDumpBuf, 0x80);
    #endif  //Not USB

#endif //Windows

    }
    else //good frame
    {
#if defined( ANI_OS_TYPE_WINDOWS )
        NdisInterlockedIncrement(&pMac->dvt.dvtInfo.rxInfo.cRxOKCounts[pRxFrame->pipe - PIPE_RX_1]);
        NdisInterlockedIncrement(&pMac->dvt.dvtInfo.rxInfo.cRxCountPerTid[pBD->tid % MAX_TIDS]);
        NdisInterlockedIncrement(&pMac->dvt.cRxCountPerRate[phyRate][pRxFrame->pipe - PIPE_RX_1]);
        NdisInterlockedIncrement(&pMac->dvt.cRxCountperSta[pBD->tid % MAX_TIDS][pBD->addr1Index % NUM_DEST_STATIONS]);
#else
        pMac->dvt.dvtInfo.rxInfo.cRxOKCounts[pRxFrame->pipe - PIPE_RX_1] ++;
        pMac->dvt.dvtInfo.rxInfo.cRxCountPerTid[pBD->tid % MAX_TIDS] ++;
        pMac->dvt.cRxCountPerRate[phyRate][pRxFrame->pipe - PIPE_RX_1] ++;
        pMac->dvt.cRxCountperSta[pBD->tid % MAX_TIDS][pBD->addr1Index % NUM_DEST_STATIONS] ++;
#endif
    }

    return;
}


void dvtAppRxEnqueueCallback(tHalHandle hHal, eDvtRxQueue queue, sRxFrameTransfer *rxFrame)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hHal;

    assert(rxFrame != NULL);
    assert(rxFrame->rxFrame->pBbuffer != NULL);

    dvtCheckCrcOnRxFrame(pMac, rxFrame);

    if (DVT_STATUS_SUCCESS != dvtRxDequeueFrame(pMac, queue, rxFrame))
    {
        assert(0);
    }
}


eANI_DVT_STATUS dvtConfigPipe(tpAniSirGlobal pMac, ePipes pipe, sDvtPipeCfg *pDvtPipeCfg)
{
    eANI_DVT_STATUS retVal = DVT_STATUS_SUCCESS;
    sPipeCfg pipeCfg;

    pipeCfg.nDescs = pDvtPipeCfg->nDescs;
    pipeCfg.nRxBuffers = pDvtPipeCfg->nRxBuffers;
    pipeCfg.preferCircular = pDvtPipeCfg->preferCircular;
    pipeCfg.bdPresent = pDvtPipeCfg->bdPresent;
    pipeCfg.refWQ = pDvtPipeCfg->refWQ;
    pipeCfg.xfrType = pDvtPipeCfg->xfrType;
    pipeCfg.chPriority = pDvtPipeCfg->chPriority;
    pipeCfg.bmuThreshold= 3; 
    /* DavidLiu: Quick patch after Ivica removed the hard coded config. DVT needs to set those threshold in per channel config properly */


    switch (pipe)
    {
        case PIPE_TX_1:
        case PIPE_TX_2:
        case PIPE_TX_3:
        case PIPE_TX_4:
        case PIPE_TX_5:
        case PIPE_TX_6:
        case PIPE_TX_7:
            {
                eDvtTxQueue queue = (eDvtTxQueue)((tANI_U32)pipe - 1);

                //DEBUG until we get loopback settings in application
                //force DXE loopback to come back on corresponding rx queue
//                 pMac->dvt.txQueues[queue].loopback = DVT_DXE_LOOPBACK;
//                 pMac->dvt.txQueues[queue].loopbackQueue = (eDvtRxQueue)queue;

                /* transmit queues may be setup to loopback at various levels
                   and the rxLoopbackQueue tells us where to expect these looped frames
                */
                if ((pMac->dvt.txQueues[queue].loopback == DVT_DXE_LOOPBACK) ||
                    (pMac->dvt.txQueues[queue].loopback == DVT_DPU_LOOPBACK) ||
                    (pMac->dvt.txQueues[queue].loopback == DVT_MLC_LOOPBACK)
                   )
                {
                    pMac->dvt.rxQueues[pMac->dvt.txQueues[queue].loopbackQueue].loopback = pMac->dvt.txQueues[queue].loopback;
                }
            }

            pipeCfg.callback = dvtTxDequeueFrame;
            break;

        case PIPE_RX_1:
        case PIPE_RX_2:
        case PIPE_RX_3:
        case PIPE_RX_4:
        case PIPE_RX_5:
        case PIPE_RX_6:
        case PIPE_RX_7:
            pipeCfg.callback = dvtRxEnqueueFrame;
            break;

        default:
            break;
    }



    if (palPipeCfg(pMac->hHdd, pipe, &pipeCfg) != eHAL_STATUS_SUCCESS)
        retVal = DVT_STATUS_FAILURE;

    return (retVal);
}

#ifdef USE_DVT_HEAP
eANI_DVT_STATUS dvtWritePipeFrameTransfer(tpAniSirGlobal pMac, tDvtSendPacket *pPacketArray, tANI_U32 countPackets)
{
    eANI_DVT_STATUS retVal = DVT_STATUS_SUCCESS;
    tANI_U32 i;
    sTxFrameTransfer *txFrame;
    sTxFrameTransfer *appFrame;
    eDvtTxQueue queue;
    tSmacBdHostTx *pBD;

    //each pPacketArray element should go on the same pipe
    if ((eDvtTxQueue)((tANI_U32)pPacketArray[0].pipe - 1) < NUM_DVT_TX_QUEUES)
    {
        queue = (eDvtTxQueue)((tANI_U32)pPacketArray[0].pipe - 1);

        //set the queue for application packets
        pMac->dvt.txQueues[queue].callback = dvtAppTxDequeueCallback;   //set callback for completion of application frames
        pMac->dvt.txQueues[queue].pipe = pPacketArray[0].pipe;
    }
    else
    {
        return (DVT_STATUS_FAILURE);
    }

    // Set the count before starting to pump the packets to PAL,
    // because , if we increment the packet counts one at a time
    // it may lead to spurious MSG responses being sent if the counter
    // goes down to zero, due to packet completions happening while this
    // routine is still pumping packets in the FOR loop.

    //need lock here
    pMac->dvt.cTxCompleteCount = countPackets;

    //translate packets to get txFrame structure for DXE
    for (i=0; i<countPackets; i++)
    {
        appFrame = &pPacketArray[i].txFrame;
        pBD = (tSmacBdHostTx *)(appFrame->bd->hddContext);

        //add tx attemp count
#if defined( ANI_OS_TYPE_WINDOWS )
        NdisInterlockedIncrement(&pMac->dvt.dvtInfo.txInfo.cTxAttempCounts[pPacketArray[i].pipe - CSR_PIPE]);
        NdisInterlockedIncrement(&pMac->dvt.cTxCountPerRate[(eHalPhyRates)dvtGetHalPhyRate((tANI_U8)pBD->rateIndex)][pPacketArray[i].pipe - CSR_PIPE]);
        NdisInterlockedIncrement(&pMac->dvt.cTxCountperSta[pBD->tid % MAX_TIDS][pBD->taIndex % NUM_DEST_STATIONS]);
#else
        pMac->dvt.dvtInfo.txInfo.cTxAttempCounts[pPacketArray[i].pipe - CSR_PIPE] ++;
        pMac->dvt.cTxCountPerRate[(eHalPhyRates)dvtGetHalPhyRate((tANI_U8)pBD->rateIndex)][pPacketArray[i].pipe - CSR_PIPE] ++;
        pMac->dvt.cTxCountperSta[pBD->tid % MAX_TIDS][pBD->taIndex % NUM_DEST_STATIONS] ++;
#endif

        //allocate heap resources and copy frame to it
        if (eHAL_STATUS_SUCCESS == palAllocateMemory(pMac->hHdd, &txFrame, sizeof(sTxFrameTransfer)))
        {
            sHddMemSegment *appSeg = appFrame->bd;
            sHddMemSegment *payloadHead = NULL;
            sHddMemSegment *payloadTail = NULL;
            tANI_U32 segCount = 0;

            assert(txFrame != NULL);

            txFrame->next = NULL;
            txFrame->bd = NULL;
            txFrame->segment = NULL;
            txFrame->descCount = 0;

            while(appSeg != NULL)
            {
                sHddMemSegment *payloadSeg;

                payloadSeg = GetHeapBytes(pMac, &pMac->dvt.txHeap, appSeg->length, DVT_HEAP_DWORD_ACCESS);

                if (payloadSeg == NULL)
                {
                    //failed, free what has been allocated thus far
                    eHalStatus retVal = palFreeMemory(pMac->hHdd, txFrame);
                    assert(retVal == eHAL_STATUS_SUCCESS);

                    dvtLog(pMac, LOGE, "ERROR: failed to allocate segment of length %d from txHeap\n", appSeg->length);
                    dvtLog(pMac, LOGE, "txHeap: freeList=%p freelist->next=%p\n", pMac->dvt.txHeap.freeList, pMac->dvt.txHeap.freeList->next);
                    dvtLog(pMac, LOGE, "txHeap: freeList.addr=%p\n", pMac->dvt.txHeap.freeList->addr);
                    dvtLog(pMac, LOGE, "txHeap: freeList.hddContext=%p\n", pMac->dvt.txHeap.freeList->hddContext);
                    dvtLog(pMac, LOGE, "txHeap: freeList.freeBytes=%d\n", pMac->dvt.txHeap.freeBytes);
                    dvtLog(pMac, LOGE, "txHeap: freeList.usedBytes=%d\n", pMac->dvt.txHeap.usedBytes);
                    dvtLog(pMac, LOGE, "txHeap: freeList.segPool.freeSegs=%d\n", pMac->dvt.txHeap.segPool.freeSegs);
                    dvtLog(pMac, LOGE, "txHeap: freeList.segPool.usedSegs=%d\n", pMac->dvt.txHeap.segPool.usedSegs);


                    if (payloadHead != NULL)
                    {
                        //some heap bytes already allocated for payload
                        PutHeapBytes(pMac, &pMac->dvt.txHeap, payloadHead);
                    }

                    return(DVT_STATUS_FAILURE);
                }
                else
                {
                    //copy appSeg content to newly allocated segment
                    assert(payloadSeg->length == appSeg->length);
                    memcpy(payloadSeg->hddContext, appSeg->hddContext, appSeg->length);

                    if (payloadHead == NULL)
                    {
                        //first segment of payload
                        payloadHead = payloadSeg;
                        payloadTail = payloadSeg;
                    }
                    else
                    {
                        assert(payloadTail->next == NULL);
                        payloadTail->next = payloadSeg;
                        payloadTail = payloadSeg;
                    }
                    segCount++;
                }
                appSeg = appSeg->next;
            }

            txFrame->bd = payloadHead;

            //reset variables to go through payload segments in appFrame
            appSeg = appFrame->segment;
            payloadHead = NULL;
            payloadTail = NULL;

            while(appSeg != NULL)
            {
                sHddMemSegment *payloadSeg;

                payloadSeg = GetHeapBytes(pMac, &pMac->dvt.txHeap, appSeg->length, DVT_HEAP_DWORD_ACCESS);

                if (payloadSeg == NULL)
                {
                    //failed, free what has been allocated thus far
                    eHalStatus retVal = palFreeMemory(pMac->hHdd, txFrame);
                    assert(retVal == eHAL_STATUS_SUCCESS);
                    assert(appFrame->bd != NULL);

                    PutHeapBytes(pMac, &pMac->dvt.txHeap, appFrame->bd);

                    dvtLog(pMac, LOGE, "ERROR: failed to allocate segment of length %d from txHeap\n", appSeg->length);

                    if (payloadHead != NULL)
                    {
                        //some heap bytes already allocated for payload
                        PutHeapBytes(pMac, &pMac->dvt.txHeap, payloadHead);
                    }

                    return(DVT_STATUS_FAILURE);
                }
                else
                {
                    //copy appSeg content to newly allocated segment
                    assert(payloadSeg->length == appSeg->length);
                    memcpy(payloadSeg->hddContext, appSeg->hddContext, appSeg->length);

                    if (payloadHead == NULL)
                    {
                        //first segment of payload
                        payloadHead = payloadSeg;
                        payloadTail = payloadSeg;
                    }
                    else
                    {
                        assert(payloadTail->next == NULL);
                        payloadTail->next = payloadSeg;
                        payloadTail = payloadSeg;
                    }
                    segCount++;
                }
                appSeg = appSeg->next;
            }

            txFrame->segment = payloadHead;

            assert(appFrame->descCount == segCount);
            txFrame->descCount = segCount;

            {
                //frame is ready - enqueue it according to pipe
                if (dvtTxEnqueueFrame(pMac, queue, txFrame) != DVT_STATUS_SUCCESS)
                {
                    //error
                    dvtLog(pMac, LOGE, "ERROR: Could not enqueue application frame\n");
                    return(DVT_STATUS_FAILURE);
                }
            }
        }
        else
        {
            //couldn't allocate memory for txFrame
            return(DVT_STATUS_FAILURE);
        }
    }

    return (retVal);
}
#else //don't USE_DVT_HEAP
eANI_DVT_STATUS dvtWritePipeFrameTransfer(tpAniSirGlobal pMac, tDvtSendPacket *pPacketArray, tANI_U32 countPackets)
{
    eANI_DVT_STATUS retVal = DVT_STATUS_SUCCESS;
    tANI_U32 i;
    sTxFrameTransfer *txFrame;
    sTxFrameTransfer *appFrame;
    eDvtTxQueue queue;
    tSmacBdHostTx *pBD;


    // Set the count before starting to pump the packets to PAL,
    // because , if we increment the packet counts one at a time
    // it may lead to spurious MSG responses being sent if the counter
    // goes down to zero, due to packet completions happening while this
    // routine is still pumping packets in the FOR loop.

    //need lock here
    pMac->dvt.cTxCompleteCount = countPackets;

    //translate packets to get txFrame structure for DXE
    for (i=0; i<countPackets; i++)
    {
        appFrame = &pPacketArray[i].txFrame;
        pBD = (tSmacBdHostTx *)(appFrame->bd->hddContext);

        assert(appFrame != NULL);
        assert(appFrame->next == NULL);

        //Right now, the queues are mapped directly to the pipes, so find out which queue to use for this frame
        if ((eDvtTxQueue)((tANI_U32)pPacketArray[i].pipe - 1) < NUM_DVT_TX_QUEUES)
        {
            queue = (eDvtTxQueue)((tANI_U32)pPacketArray[i].pipe - 1);

            //set the queue for application packets
            pMac->dvt.txQueues[queue].callback = dvtAppTxDequeueCallback;   //set callback for completion of application frames
            pMac->dvt.txQueues[queue].pipe = pPacketArray[i].pipe;
        }
        else
        {
            return (DVT_STATUS_FAILURE);
        }


        //add tx attemp count
#if defined( ANI_OS_TYPE_WINDOWS )
        NdisInterlockedIncrement(&pMac->dvt.dvtInfo.txInfo.cTxAttempCounts[pPacketArray[i].pipe - CSR_PIPE]);
        NdisInterlockedIncrement(&pMac->dvt.cTxCountPerRate[(eHalPhyRates)dvtGetHalPhyRate((tANI_U8)pBD->rateIndex)][pPacketArray[i].pipe - CSR_PIPE]);
        NdisInterlockedIncrement(&pMac->dvt.cTxCountperSta[pBD->tid % MAX_TIDS][pBD->taIndex % NUM_DEST_STATIONS]);
#else
        pMac->dvt.dvtInfo.txInfo.cTxAttempCounts[pPacketArray[i].pipe - CSR_PIPE] ++;
        pMac->dvt.cTxCountPerRate[(eHalPhyRates)dvtGetHalPhyRate((tANI_U8)pBD->rateIndex)][pPacketArray[i].pipe - CSR_PIPE] ++;
        pMac->dvt.cTxCountperSta[pBD->tid % MAX_TIDS][pBD->taIndex % NUM_DEST_STATIONS] ++;
#endif

        //frame is ready - enqueue it according to pipe
        if (dvtTxEnqueueFrame(pMac, queue, appFrame) != DVT_STATUS_SUCCESS)
        {
            //error
            dvtLog(pMac, LOGE, "ERROR: Could not enqueue application frame\n");
            return(DVT_STATUS_FAILURE);
        }
    }

    return (retVal);
}


#endif


static void dvtAppTxDequeueCallback(tHalHandle hHal, eDvtTxQueue queue)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hHal;
    ePipes pipe = pMac->dvt.txQueues[queue].pipe;

    switch (pipe)
    {
        case CSR_PIPE:
        case PIPE_TX_1:
        case PIPE_TX_2:
        case PIPE_TX_3:
        case PIPE_TX_4:
        case PIPE_TX_5:
        case PIPE_TX_6:
        case PIPE_TX_7:
#if defined( ANI_OS_TYPE_WINDOWS )
            NdisInterlockedIncrement(&pMac->dvt.dvtInfo.txInfo.cTxOKCounts[pipe - CSR_PIPE]);
#else
            pMac->dvt.dvtInfo.txInfo.cTxOKCounts[pipe - CSR_PIPE] ++;
#endif

            //TBD: how to get tx err count?
            //pMac->dvt.dvtInfo.txInfo.cTxErrCounts[pipe - CSR_PIPE] ++;

            //need lock here
#if defined( ANI_OS_TYPE_WINDOWS )
            if(NdisInterlockedDecrement(&pMac->dvt.cTxCompleteCount) == 0)
#else
            pMac->dvt.cTxCompleteCount --;
            if (pMac->dvt.cTxCompleteCount == 0) //reach to zero
#endif
            {
                //send message back to indicate protocol driver to unlock frame memory.
                tSirMsgQ *pMsg = &(pMac->dvt.dvtMsg);
                tDvtMsgbuffer *pDvtMsg = (tDvtMsgbuffer *)(pMsg->bodyptr);

                dvtSendMsgResponse(pMac, pDvtMsg);

                if (eHAL_STATUS_SUCCESS != palFreeMemory( pMac->hHdd, (tANI_U8*)pDvtMsg ) )
                {
                    assert(0);
                    break;
                }
            }
            break;
        default:
            //assert(0);
            break;
    }
}


eANI_DVT_STATUS dvtGetBmuInfo(tpAniSirGlobal pMac, sDvtBMUInfo *pBmuInfo)
{
    eANI_DVT_STATUS retVal = DVT_STATUS_SUCCESS;
    tANI_U32 i, availBdPdu, wqEnable, isDpuWq, bdPduThreshold, bdPduReserved;

    if ( palReadRegister( pMac->hHdd, BMU_CONTROL_REG, &(pBmuInfo->control) ) != eHAL_STATUS_SUCCESS)
        return (DVT_STATUS_FAILURE);

    if (bmuCommand_read_wq_nr(pMac, 0, &(pBmuInfo->freeBD)) != eHAL_STATUS_SUCCESS) //wq0: idle BDs
        return (DVT_STATUS_FAILURE);

    if (bmuCommand_read_wq_nr(pMac, 1, &(pBmuInfo->freePDU)) != eHAL_STATUS_SUCCESS) //wq1: idle PDUs
        return (DVT_STATUS_FAILURE);

    if ( palReadRegister( pMac->hHdd, BMU_ERR_INTR_STATUS_REG, &(pBmuInfo->errIntrStatus) ) != eHAL_STATUS_SUCCESS)
        return (DVT_STATUS_FAILURE);
        //masking the release_fifo_full_warning and not_enough_BD_PDU_warning bits
        pBmuInfo->errIntrStatus = pBmuInfo->errIntrStatus & 0xFFFFFDFE;

    if ( palReadRegister( pMac->hHdd, BMU_ERR_INTR_ENABLE_REG, &(pBmuInfo->errIntrEnable) ) != eHAL_STATUS_SUCCESS)
        return (DVT_STATUS_FAILURE);

    if ( palReadRegister( pMac->hHdd, BMU_ERR_INT_ADDR_REG, &(pBmuInfo->errIntrAddr) ) != eHAL_STATUS_SUCCESS)
        return (DVT_STATUS_FAILURE);

    if ( palReadRegister( pMac->hHdd, BMU_ERR_INT_WDATA_REG, &(pBmuInfo->errIntrWData) ) != eHAL_STATUS_SUCCESS)
        return (DVT_STATUS_FAILURE);

    for (i=0; i< SMAC_BMUWQ_NUM; i++)
    {
        if (bmuCommand_read_wq_head(pMac, i, &pBmuInfo->wqInfo[i].headBdIndex) != eHAL_STATUS_SUCCESS)
            return (DVT_STATUS_FAILURE);
        if (bmuCommand_read_wq_tail(pMac, i, &pBmuInfo->wqInfo[i].tailBdIndex) != eHAL_STATUS_SUCCESS)
            return (DVT_STATUS_FAILURE);

        if (bmuCommand_read_wq_nr(pMac, i, &pBmuInfo->wqInfo[i].availBDs) != eHAL_STATUS_SUCCESS)
            return (DVT_STATUS_FAILURE);

        if ( palReadRegister( pMac->hHdd, BMU_WQ_ENABLE_REG, &wqEnable ) != eHAL_STATUS_SUCCESS)
            return (DVT_STATUS_FAILURE);

        pBmuInfo->wqInfo[i].isEnabled = (wqEnable & (1<<i)) ? 1 : 0;

        if ( palReadRegister( pMac->hHdd, BMU_DPU_WQ_ASSIGNMENT_REG, &isDpuWq ) != eHAL_STATUS_SUCCESS)
            return (DVT_STATUS_FAILURE);

        pBmuInfo->wqInfo[i].isDpuWq = (isDpuWq & (1<<i)) ? 1 : 0;;
    }

    for (i=0; i<11; i++)
    {
        if ( palReadRegister( pMac->hHdd, (BMU_BD_PDU_THRESHOLD0_REG + (i*4)), &bdPduThreshold ) != eHAL_STATUS_SUCCESS)
            return (DVT_STATUS_FAILURE);

        pBmuInfo->masterInfo[i].bdThreshold = (bdPduThreshold & BMU_BD_PDU_THRESHOLD0_BD_THRESHOLD_0_MASK) >> BMU_BD_PDU_THRESHOLD0_BD_THRESHOLD_0_OFFSET;
        pBmuInfo->masterInfo[i].pduThreshold = (bdPduThreshold & BMU_BD_PDU_THRESHOLD0_PDU_THRESHOLD_0_MASK) >> BMU_BD_PDU_THRESHOLD0_PDU_THRESHOLD_0_OFFSET;

        if ( palReadRegister( pMac->hHdd, (BMU_BD_PDU_RESERVED0_REG + (i*4)), &bdPduReserved ) != eHAL_STATUS_SUCCESS)
            return (DVT_STATUS_FAILURE);

        pBmuInfo->masterInfo[i].bdReserved = (bdPduReserved & BMU_BD_PDU_RESERVED0_NUMBER_RESERVED_BD_MASK) >> BMU_BD_PDU_RESERVED0_NUMBER_RESERVED_BD_DEFAULT;
        pBmuInfo->masterInfo[i].pduReserved = (bdPduReserved & BMU_BD_PDU_RESERVED0_NUMBER_RESERVED_PDU_MASK) >> BMU_BD_PDU_RESERVED0_NUMBER_RESERVED_PDU_OFFSET;
    }

    if (palReadRegister(pMac->hHdd, MCU_BD_PDU_BASE_ADDR_REG, &pBmuInfo->bdPduBaseAddr) != eHAL_STATUS_SUCCESS)
        return (DVT_STATUS_FAILURE);

    return (retVal);
}

eANI_DVT_STATUS dvtGetDxeInfo(tpAniSirGlobal pMac, sDvtDxeInfo *pDxeInfo)
{
    eANI_DVT_STATUS retVal = DVT_STATUS_SUCCESS;
    tANI_U32 i, chReg;

    for (i=0; i<MAX_DXE_CHANNEL; i++)
    {
        chReg = DXE_0_CH_CTRL_REG + DXE_CH_REG_SIZE * i;
        if (palReadRegister(pMac->hHdd, chReg, &pDxeInfo->channelInfo[i].csr) != eHAL_STATUS_SUCCESS)
            return (DVT_STATUS_FAILURE);
        chReg = DXE_0_CH_SZ_REG + DXE_CH_REG_SIZE * i;
        if (palReadRegister(pMac->hHdd, chReg, &pDxeInfo->channelInfo[i].sz) != eHAL_STATUS_SUCCESS)
            return (DVT_STATUS_FAILURE);
        chReg = DXE_0_CH_SADRL_REG + DXE_CH_REG_SIZE * i;
        if (palReadRegister(pMac->hHdd, chReg, &pDxeInfo->channelInfo[i].saddr) != eHAL_STATUS_SUCCESS)
            return (DVT_STATUS_FAILURE);
        chReg = DXE_0_CH_DADRL_REG + DXE_CH_REG_SIZE * i;
        if (palReadRegister(pMac->hHdd, chReg, &pDxeInfo->channelInfo[i].daddr) != eHAL_STATUS_SUCCESS)
            return (DVT_STATUS_FAILURE);
        chReg = DXE_0_CH_DESCL_REG + DXE_CH_REG_SIZE * i;
        if (palReadRegister(pMac->hHdd, chReg, &pDxeInfo->channelInfo[i].desc) != eHAL_STATUS_SUCCESS)
            return (DVT_STATUS_FAILURE);
//        chReg = DXE_0_CH_LST_BD_REG + DXE_CH_REG_SIZE * i;
//        if (palReadRegister(pMac->hHdd, chReg, &pDxeInfo->channelInfo[i].lstDesc) != eHAL_STATUS_SUCCESS)
//            return (DVT_STATUS_FAILURE);
        chReg = DXE_0_CH_BD_REG + DXE_CH_REG_SIZE * i;
        if (palReadRegister(pMac->hHdd, chReg, &pDxeInfo->channelInfo[i].bd) != eHAL_STATUS_SUCCESS)
            return (DVT_STATUS_FAILURE);
        chReg = DXE_0_CH_HEAD_REG + DXE_CH_REG_SIZE * i;
        if (palReadRegister(pMac->hHdd, chReg, &pDxeInfo->channelInfo[i].head) != eHAL_STATUS_SUCCESS)
            return (DVT_STATUS_FAILURE);
        chReg = DXE_0_CH_TAIL_REG + DXE_CH_REG_SIZE * i;
        if (palReadRegister(pMac->hHdd, chReg, &pDxeInfo->channelInfo[i].tail) != eHAL_STATUS_SUCCESS)
            return (DVT_STATUS_FAILURE);
        chReg = DXE_0_CH_PDU_REG + DXE_CH_REG_SIZE * i;
        if (palReadRegister(pMac->hHdd, chReg, &pDxeInfo->channelInfo[i].pdu) != eHAL_STATUS_SUCCESS)
            return (DVT_STATUS_FAILURE);
//        chReg = DXE_0_CH_TSTMP_REG + DXE_CH_REG_SIZE * i;
//        if (palReadRegister(pMac->hHdd, chReg, &pDxeInfo->channelInfo[i].tstmp) != eHAL_STATUS_SUCCESS)
//            return (DVT_STATUS_FAILURE);
    }

    return (retVal);
}

eANI_DVT_STATUS dvtGetDpuDescInfo(tpAniSirGlobal pMac, sDvtDpuDescInfo *dpuDescInfo)
{
    eANI_DVT_STATUS retVal = DVT_STATUS_SUCCESS;
    tANI_U32 dpuBaseAddrReg = 0;
    tDpuDescriptor dpuDesc;

    if ( palReadRegister( pMac->hHdd, DPU_DPU_BASE_ADDR_REG, &dpuBaseAddrReg ) != eHAL_STATUS_SUCCESS)
        return (DVT_STATUS_FAILURE);
    if( palReadDeviceMemory( pMac->hHdd, ((dpuDescInfo->dpuDescIdx * sizeof(tDpuDescriptor)) + dpuBaseAddrReg), (tANI_U8 *)&(dpuDesc), sizeof(tDpuDescriptor) )  != eHAL_STATUS_SUCCESS)
        return (DVT_STATUS_FAILURE);

    dpuDescInfo->dpuDescStats.ppi = (tANI_U8)dpuDesc.ppi;
    dpuDescInfo->dpuDescStats.pli = (tANI_U8)dpuDesc.pli;
    dpuDescInfo->dpuDescStats.txFragThreshold4B = (tANI_U16)dpuDesc.txFragThreshold4B;
    dpuDescInfo->dpuDescStats.signature = (tANI_U8)dpuDesc.signature;
    dpuDescInfo->dpuDescStats.enablePerTidDecomp = (tANI_U16)dpuDesc.enablePerTidDecomp;
    dpuDescInfo->dpuDescStats.enablePerTidComp = (tANI_U16)dpuDesc.enablePerTidComp;
    dpuDescInfo->dpuDescStats.enableTxPerTidInsertConcatSeqNum = (tANI_U16)dpuDesc.enableTxPerTidInsertConcatSeqNum;
    dpuDescInfo->dpuDescStats.enableRxPerTidRemoveConcatSeqNum = (tANI_U16)dpuDesc.enableRxPerTidRemoveConcatSeqNum;
    dpuDescInfo->dpuDescStats.replayCountSet = (tANI_U8)dpuDesc.replayCountSet;
    dpuDescInfo->dpuDescStats.mickeyIndex = (tANI_U8)dpuDesc.mickeyIndex;
    dpuDescInfo->dpuDescStats.keyIndex = (tANI_U8)dpuDesc.keyIndex;
    dpuDescInfo->dpuDescStats.txKeyId = (tANI_U8)dpuDesc.txKeyId;
    dpuDescInfo->dpuDescStats.encryptMode = (tANI_U8)dpuDesc.encryptMode;
    palCopyMemory(pMac->hHdd, (tANI_U8 *)(dpuDescInfo->dpuDescStats.idxPerTidReplayCount), (tANI_U8 *)(dpuDesc.idxPerTidReplayCount), 4 * sizeof(tANI_U32));
    dpuDescInfo->dpuDescStats.txSentBlocks = dpuDesc.txSentBlocks;
    dpuDescInfo->dpuDescStats.rxRcvddBlocks = dpuDesc.rxRcvddBlocks;
    dpuDescInfo->dpuDescStats. wepRxKeyIdx0 = (tANI_U8)dpuDesc. wepRxKeyIdx0;
    dpuDescInfo->dpuDescStats. wepRxKeyIdx1 = (tANI_U8)dpuDesc. wepRxKeyIdx1;
    dpuDescInfo->dpuDescStats. wepRxKeyIdx2 = (tANI_U8)dpuDesc. wepRxKeyIdx2;
    dpuDescInfo->dpuDescStats. wepRxKeyIdx3 = (tANI_U8)dpuDesc. wepRxKeyIdx3;
    dpuDescInfo->dpuDescStats.micErrCount = (tANI_U8)dpuDesc.micErrCount;
    dpuDescInfo->dpuDescStats.excludedCount = dpuDesc.excludedCount;
    dpuDescInfo->dpuDescStats.formatErrorCount = (tANI_U16)dpuDesc.formatErrorCount;
    dpuDescInfo->dpuDescStats.undecryptableCount = (tANI_U16)dpuDesc.undecryptableCount;
    dpuDescInfo->dpuDescStats.decryptErrorCount = dpuDesc.decryptErrorCount;
    dpuDescInfo->dpuDescStats.decryptSuccessCount = dpuDesc.decryptSuccessCount;
    dpuDescInfo->dpuDescStats.keyIdErr = (tANI_U8)dpuDesc.keyIdErr;
    dpuDescInfo->dpuDescStats.extIVerror = (tANI_U8)dpuDesc.extIVerror;

    return (retVal);
}

eANI_DVT_STATUS dvtGetDpuInfo(tpAniSirGlobal pMac, sDvtDpuInfo *pDpuInfo)
{
    eANI_DVT_STATUS retVal = DVT_STATUS_SUCCESS;
    if ( palReadRegister( pMac->hHdd, DPU_DPU_TXPKTCOUNT_REG, &(pDpuInfo->txPktCount) ) != eHAL_STATUS_SUCCESS)
        return (DVT_STATUS_FAILURE);
    if ( palReadRegister( pMac->hHdd, DPU_DPU_RXPKTCOUNT_REG, &(pDpuInfo->rxPktCount) ) != eHAL_STATUS_SUCCESS)
        return (DVT_STATUS_FAILURE);
    if ( palReadRegister( pMac->hHdd, DPU_DPU_BD_CHECK_COUNT_REG, &(pDpuInfo->bDCheckCount) ) != eHAL_STATUS_SUCCESS)
        return (DVT_STATUS_FAILURE);
    if ( palReadRegister( pMac->hHdd, DPU_DPU_CONTROL_REG, &(pDpuInfo->control) ) != eHAL_STATUS_SUCCESS)
        return (DVT_STATUS_FAILURE);
    if ( palReadRegister( pMac->hHdd, DPU_DPU_INTERRUPT_MASK_REG, &(pDpuInfo->interruptMask) ) != eHAL_STATUS_SUCCESS)
        return (DVT_STATUS_FAILURE);
    if ( palReadRegister( pMac->hHdd, DPU_DPU_INTERRUPT_STATUS_REG, &(pDpuInfo->interruptStatus) ) != eHAL_STATUS_SUCCESS)
        return (DVT_STATUS_FAILURE);
//removed since 199B netlist
//    if ( palReadRegister( pMac->hHdd, DPU_DPU_EVENT_FIFO_REG, &(pDpuInfo->eventfifo) ) != eHAL_STATUS_SUCCESS)
//        return (DVT_STATUS_FAILURE);
    if ( palReadRegister( pMac->hHdd, DPU_DPU_REPLAY_CNT_THR_MSW_REG, &(pDpuInfo->replayCntThrMSW) ) != eHAL_STATUS_SUCCESS)
        return (DVT_STATUS_FAILURE);
    if ( palReadRegister( pMac->hHdd, DPU_DPU_REPLAY_CNT_THR_LSW_REG, &(pDpuInfo->rplayCntThrLSW) ) != eHAL_STATUS_SUCCESS)
        return (DVT_STATUS_FAILURE);
    if ( palReadRegister( pMac->hHdd, DPU_DPU_PPI_EXPAND_THRESHOLD_REG, &(pDpuInfo->pPIExpandThreshold) ) != eHAL_STATUS_SUCCESS)
        return (DVT_STATUS_FAILURE);
    if ( palReadRegister( pMac->hHdd, DPU_DPU_MAXIMUM_PKT_LEN_REG, &(pDpuInfo->maximumPktLen) ) != eHAL_STATUS_SUCCESS)
        return (DVT_STATUS_FAILURE);
    if ( palReadRegister( pMac->hHdd, DPU_DPU_COMPRESSION_TYPE_CNT_REG, &(pDpuInfo->compressionTypeCnt) ) != eHAL_STATUS_SUCCESS)
        return (DVT_STATUS_FAILURE);
    if ( palReadRegister( pMac->hHdd, DPU_DPU_COMPRESSION_NUM_PKTS_REG, &(pDpuInfo->compressionNumPkts) ) != eHAL_STATUS_SUCCESS)
        return (DVT_STATUS_FAILURE);
    if ( palReadRegister( pMac->hHdd, DPU_DPU_DECOMPRESSION_NUM_PKTS_REG, &(pDpuInfo->decompressionNumPkts) ) != eHAL_STATUS_SUCCESS)
        return (DVT_STATUS_FAILURE);
    if ( palReadRegister( pMac->hHdd, DPU_DPU_WATCHDOG_REG, &(pDpuInfo->watchdog) ) != eHAL_STATUS_SUCCESS)
        return (DVT_STATUS_FAILURE);
    if ( palReadRegister( pMac->hHdd, DPU_DPU_BD_IN_COUNTS_REG, &(pDpuInfo->bDInCounts) ) != eHAL_STATUS_SUCCESS)
        return (DVT_STATUS_FAILURE);
    if ( palReadRegister( pMac->hHdd, DPU_DPU_BD_OUT_COUNTS_REG, &(pDpuInfo->bDOutCounts) ) != eHAL_STATUS_SUCCESS)
        return (DVT_STATUS_FAILURE);

    return (retVal);
}

eANI_DVT_STATUS dvtGatherInfo(tpAniSirGlobal pMac, sDvtGatherInfo *dvtInfo, tANI_U32 dbgInfoMask)
{
    eANI_DVT_STATUS retVal = DVT_STATUS_SUCCESS;

    sDvtTxInfo  *pTxInfo;
    sDvtRxInfo  *pRxInfo;
    sDvtBMUInfo *pBmuInfo;
    sDvtDxeInfo *pDxeInfo;
    sDvtDpuInfo *pDpuInfo;
    sDvtDpuDescInfo *pDpuStats;

    pTxInfo = &(dvtInfo->txInfo);
    pRxInfo = &(dvtInfo->rxInfo);
    pBmuInfo = &(dvtInfo->bmuInfo);
    pDxeInfo = &(dvtInfo->dxeInfo);
    pDpuInfo = &(dvtInfo->dpuInfo);
    pDpuStats = &(dvtInfo->dpuDescInfo);

    if (dbgInfoMask & DVT_INFO_MASK_TX)
    {
        palCopyMemory(pMac->hHdd, (tANI_U8 *)pTxInfo, (tANI_U8 *)&(pMac->dvt.dvtInfo.txInfo), sizeof(sDvtTxInfo));
    }
    if (dbgInfoMask & DVT_INFO_MASK_RX)
    {
        palCopyMemory(pMac->hHdd, (tANI_U8 *)pRxInfo, (tANI_U8 *)&(pMac->dvt.dvtInfo.rxInfo), sizeof(sDvtRxInfo));
    }
    if (dbgInfoMask & DVT_INFO_MASK_BMU)
    {
        retVal = dvtGetBmuInfo(pMac, pBmuInfo);
    }
    if (dbgInfoMask & DVT_INFO_MASK_DXE)
    {
        retVal = dvtGetDxeInfo(pMac, pDxeInfo);
    }
    if (dbgInfoMask & DVT_INFO_MASK_DPU)
    {
        retVal = dvtGetDpuInfo(pMac, pDpuInfo);
    }
    if (dbgInfoMask & DVT_INFO_MASK_DPU_DESC)
    {
        retVal = dvtGetDpuDescInfo(pMac, pDpuStats);
    }

    return (retVal);

}

eANI_DVT_STATUS dvtResetCounters(tpAniSirGlobal pMac)
{
    palZeroMemory(pMac->hHdd, (void *)&(pMac->dvt.dvtInfo.txInfo), sizeof(sDvtTxInfo));
    palZeroMemory(pMac->hHdd, (void *)&(pMac->dvt.dvtInfo.rxInfo), sizeof(sDvtRxInfo));
    palZeroMemory(pMac->hHdd, (void *)&(pMac->dvt.cRxCountperSta), MAX_TIDS * NUM_DEST_STATIONS * sizeof(tANI_U32));
    palZeroMemory(pMac->hHdd, (void *)&(pMac->dvt.cTxCountperSta), MAX_TIDS * NUM_DEST_STATIONS * sizeof(tANI_U32));
    palZeroMemory(pMac->hHdd, (void *)&(pMac->dvt.cRxPMIRateIdxMisMatchCount), NUM_HAL_PHY_RATES * sizeof(tANI_U32));
    palZeroMemory(pMac->hHdd, (void *)&(pMac->dvt.cRxDataFrames), MAX_NUM_DATA_FRAMES * sizeof(tANI_U32));
    palZeroMemory(pMac->hHdd, (void *)&(pMac->dvt.cRxControlFrames), sizeof(tANI_U32));
    palZeroMemory(pMac->hHdd, (void *)&(pMac->dvt.cRxMgmtFrames), sizeof(tANI_U32));
    palZeroMemory(pMac->hHdd, (void *)&(pMac->dvt.cRxBeaconFrames), sizeof(tANI_U32));
    palZeroMemory(pMac->hHdd, (void *)&(pMac->dvt.cRxMulticastFrames), sizeof(tANI_U32));
    palZeroMemory(pMac->hHdd, (void *)&(pMac->dvt.cRxBroadcastFrames), sizeof(tANI_U32));
    palZeroMemory(pMac->hHdd, (void *)&(pMac->dvt.cRxUnicastFrames), sizeof(tANI_U32));

    return DVT_STATUS_SUCCESS;
}

eANI_DVT_STATUS dvtDumpBdPdu(tpAniSirGlobal pMac, tANI_U32 dumpMasknIndex, tANI_U8 *pBuf)
{
    eANI_DVT_STATUS retVal = DVT_STATUS_SUCCESS;
    tANI_U32 bdPduBaseAddr;
    tANI_U32 value, maxBdIndex, maxPduIndex;

    if (pBuf == 0)
        return (DVT_STATUS_FAILURE);

    if (palReadRegister(pMac->hHdd, MCU_BD_PDU_BASE_ADDR_REG, &bdPduBaseAddr) != eHAL_STATUS_SUCCESS)
        return (DVT_STATUS_FAILURE);

    if (palReadRegister(pMac->hHdd, BMU_CONTROL_REG, &value) != eHAL_STATUS_SUCCESS)
        return eHAL_STATUS_FAILURE;

    maxBdIndex = (value & BMU_CONTROL_MAX_BD_INDEX_NR_MASK) >> BMU_CONTROL_MAX_BD_INDEX_NR_OFFSET;
    maxPduIndex = (value & BMU_CONTROL_MAX_PDU_INDEX_NR_MASK) >> BMU_CONTROL_MAX_PDU_INDEX_NR_OFFSET;

    if ((dumpMasknIndex & DVT_DXE_MASK_DUMP_BD) == DVT_DXE_MASK_DUMP_BD)
    {
        //dump BD
        dumpMasknIndex &= ~DVT_DXE_MASK_DUMP_BD;
        if (dumpMasknIndex > maxBdIndex)
            retVal = DVT_STATUS_FAILURE;
    }
    else if ((dumpMasknIndex & DVT_DXE_MASK_DUMP_PDU) == DVT_DXE_MASK_DUMP_PDU)
    {
        //dump PDU
        dumpMasknIndex &= ~DVT_DXE_MASK_DUMP_PDU;
        if (dumpMasknIndex > maxPduIndex)
            retVal = DVT_STATUS_FAILURE;
    }

    if (palReadDeviceMemory(pMac->hHdd, bdPduBaseAddr+dumpMasknIndex*HAL_BD_SIZE, pBuf, HAL_BD_SIZE) != eHAL_STATUS_SUCCESS)
            return (DVT_STATUS_FAILURE);

    return (retVal);
}

eANI_DVT_STATUS dvtCrcCheckEnable(tpAniSirGlobal pMac, tANI_U32 crcEnable)
{
    eANI_DVT_STATUS retVal = DVT_STATUS_SUCCESS;

    pMac->dvt.crcEnable = crcEnable;
    if (pMac->dvt.rxCrcTableInit == 0)
    {
        rxCrcGen(pMac);
        pMac->dvt.rxCrcTableInit = 1;
    }

    return (retVal);
}

eANI_DVT_STATUS dvtDxeTimestampEnable(tpAniSirGlobal pMac, tANI_U32 dxeTimestampEnable)
{
    eANI_DVT_STATUS retVal = DVT_STATUS_SUCCESS;
    tANI_U32 regVal, i;

    pMac->dvt.dxeTimestampEnable = dxeTimestampEnable;
    if (pMac->dvt.dxeTimestampEnable)
    {
        // Reset DXE time histogram indices
        for(i=0; i < MAX_RX_PIPES; i++)
            pMac->dvt.dxePktHeadIndx[i] = pMac->dvt.dxePktTailIndx[i] = -1;

        // Turn on DXE timestamping
        palReadRegister(pMac->hHdd, DXE_0_DMA_CSR_REG, &regVal);
        regVal &= ~DXE_0_DMA_CSR_H2B_TSTMP_OFF_MASK;
        regVal |= DXE_0_DMA_CSR_H2B_TSTMP_OFF_MASK & 
                  ((offsetof(tSmacBdHostTx, dxeH2BStartTimestamp)/4) << 
                   DXE_0_DMA_CSR_H2B_TSTMP_OFF_OFFSET);
        regVal &= ~DXE_0_DMA_CSR_B2H_TSTMP_OFF_MASK;
        regVal |= DXE_0_DMA_CSR_B2H_TSTMP_OFF_MASK & 
                  ((offsetof(tSmacBdHostTx, dxeB2HStartTimestamp)/4) << 
                   DXE_0_DMA_CSR_B2H_TSTMP_OFF_OFFSET);
        regVal |= DXE_0_DMA_CSR_TSTMP_EN_MASK;
        palWriteRegister(pMac->hHdd, DXE_0_DMA_CSR_REG, regVal);
    }
    else
    {
        // Turn off DXE timestamping
        palReadRegister(pMac->hHdd, DXE_0_DMA_CSR_REG, &regVal);
        regVal &= ~(DXE_0_DMA_CSR_H2B_TSTMP_OFF_MASK |
                    DXE_0_DMA_CSR_B2H_TSTMP_OFF_MASK | 
                    DXE_0_DMA_CSR_TSTMP_EN_MASK);
        palWriteRegister(pMac->hHdd, DXE_0_DMA_CSR_REG, regVal);
    }

    return (retVal);
}



eANI_DVT_STATUS dvtNviWriteData(tpAniSirGlobal pMac, tANI_U32 eepromOffset, tANI_U8 pBuf[MAX_BUFSIZE], tANI_U32 nBytes)
{
    eANI_DVT_STATUS retVal = DVT_STATUS_SUCCESS;

    if (eHAL_STATUS_SUCCESS != asicNVIWriteData(pMac, eepromOffset, pBuf, nBytes))
        return DVT_STATUS_FAILURE;

    return (retVal);
}

eANI_DVT_STATUS dvtNviReadData(tpAniSirGlobal pMac, tANI_U32 eepromOffset, tANI_U8 pBuf[MAX_BUFSIZE], tANI_U32 nBytes)
{
    eANI_DVT_STATUS retVal = DVT_STATUS_SUCCESS;

    if (eHAL_STATUS_SUCCESS != asicNVIReadData(pMac, eepromOffset, pBuf, nBytes))
        return DVT_STATUS_FAILURE;

    return (retVal);
}

eANI_DVT_STATUS dvtNviWriteBurstData(tpAniSirGlobal pMac, tANI_U32 eepromOffset, tANI_U32 pBuf[MAX_BUFSIZE], tANI_U32 nDwords)
{
    eANI_DVT_STATUS retVal = DVT_STATUS_SUCCESS;

    if (eHAL_STATUS_SUCCESS != asicNVIWriteBurstData(pMac, eepromOffset, pBuf, nDwords))
        return DVT_STATUS_FAILURE;

    return (retVal);
}

eANI_DVT_STATUS dvtNviReadBurstData(tpAniSirGlobal pMac, tANI_U32 eepromOffset, tANI_U32 pBuf[MAX_BUFSIZE], tANI_U32 nDwords)
{
    eANI_DVT_STATUS retVal = DVT_STATUS_SUCCESS;

    if (eHAL_STATUS_SUCCESS != asicNVIReadBurstData(pMac, eepromOffset, pBuf, nDwords))
        return DVT_STATUS_FAILURE;

    return (retVal);
}

eANI_DVT_STATUS dvtGetEepromFieldSize(tpAniSirGlobal pMac, eEepromField field, tANI_U32 *fieldSize)
{
    eANI_DVT_STATUS retVal = DVT_STATUS_SUCCESS;

    retVal = halGetEepromFieldSize(pMac, field, fieldSize);

    return (retVal);
}

eANI_DVT_STATUS dvtGetEepromTableSize(tpAniSirGlobal pMac, eEepromTable table, tANI_U32 *tableSize)
{
    eANI_DVT_STATUS retVal = DVT_STATUS_SUCCESS;

    retVal = halGetEepromTableSize(pMac, table, tableSize);

    return (retVal);
}

eANI_DVT_STATUS dvtGetEepromTableDir(tpAniSirGlobal pMac, eEepromTable table, sEepromTableDir *dirEntry)
{
    eANI_DVT_STATUS retVal = DVT_STATUS_SUCCESS;


    return (retVal);
}

eANI_DVT_STATUS dvtReadEepromField(tpAniSirGlobal pMac, eEepromField field, uEepromFields *fieldData)
{
    eANI_DVT_STATUS retVal = DVT_STATUS_SUCCESS;

    retVal = halReadEepromField(pMac, field, fieldData);

    return (retVal);
}

eANI_DVT_STATUS dvtWriteEepromField(tpAniSirGlobal pMac, eEepromField field, uEepromFields *fieldData)
{
    eANI_DVT_STATUS retVal = DVT_STATUS_SUCCESS;

    retVal = halWriteEepromField(pMac, field, fieldData);

    return (retVal);
}

eANI_DVT_STATUS dvtReadEepromTable(tpAniSirGlobal pMac, eEepromTable eepromTable, uEepromTables *tableData, tANI_U32 *tableLen)
{
    eANI_DVT_STATUS retVal = DVT_STATUS_SUCCESS;

    retVal = halReadEepromTable(pMac, eepromTable, tableData);

    return (retVal);
}

eANI_DVT_STATUS dvtWriteEepromTable(tpAniSirGlobal pMac, eEepromTable eepromTable, uEepromTables *tableData)
{
    eANI_DVT_STATUS retVal = DVT_STATUS_SUCCESS;

    retVal = halWriteEepromTable(pMac, eepromTable, tableData);

    return (retVal);
}

eANI_DVT_STATUS dvtRemoveEepromTable(tpAniSirGlobal pMac, eEepromTable eepromTable)
{
    eANI_DVT_STATUS retVal = DVT_STATUS_SUCCESS;

    retVal = halRemoveEepromTable(pMac, eepromTable);

    return (retVal);
}


eANI_DVT_STATUS dvtSimpleMacConfig(tpAniSirGlobal pMac, sDvtSimpleMacConfig mac)
{
    eANI_DVT_STATUS retVal = DVT_STATUS_SUCCESS;
    tSirMsgQ halMsg;

    do
    {
        // Send out the global config request to SoftMAC
        halMsg.type =  SIR_HAL_INIT_WM_CFG_REQ;
        halMsg.reserved = 0;
        halMsg.bodyval = 0;
        halMsg.bodyptr = NULL;
        
        halPostMsgApi(pMac, &halMsg);

        if (cfgSetStr(pMac,
                      WNI_CFG_STA_ID,
                      &mac.macAddr[0], sizeof(tSirMacAddr)) != eSIR_SUCCESS)
        {
            dvtLog(pMac, LOGW, "dvtSimpleMacConfig(): Failed to set own MAC address to %02x-%02x-%02x-%02x-%02x-%02x\n",
                   mac.macAddr[0], mac.macAddr[1], mac.macAddr[2], mac.macAddr[3], mac.macAddr[4], mac.macAddr[5]);
            retVal = DVT_STATUS_FAILURE;
            break;
        }
        else
        {
            dvtLog(pMac, LOGW, "dvtSimpleMacConfig(): Added own MAC address to %02x-%02x-%02x-%02x-%02x-%02x\n",
                   mac.macAddr[0], mac.macAddr[1], mac.macAddr[2], mac.macAddr[3], mac.macAddr[4], mac.macAddr[5]);
        }

        memcpy(&pMac->dvt.mac, &mac, sizeof(sDvtSimpleMacConfig));


        pMac->dvt.mac.configured = eANI_BOOLEAN_TRUE;

        //DEBUG: temporary until we have an interface to set these
        pMac->dvt.mac.wlanCapabilities.edca = eANI_BOOLEAN_FALSE;
        pMac->dvt.mac.wlanCapabilities.hcca = eANI_BOOLEAN_FALSE;
        pMac->dvt.mac.wlanCapabilities.n = eANI_BOOLEAN_FALSE;
        pMac->dvt.mac.wlanCapabilities.greenfield = eANI_BOOLEAN_FALSE;
    } while(0);

    return (retVal);
}

eANI_DVT_STATUS dvtSimpleBssConfig(tpAniSirGlobal pMac, sDvtSimpleBssConfig bss, tANI_U32 *bssIndex)
{
    eANI_DVT_STATUS retVal = DVT_STATUS_SUCCESS;
    tpAddBssParams pHalBss;
    tpAddStaParams pHalSta;

    if (pMac->dvt.mac.configured == eANI_BOOLEAN_FALSE)
    {
        //mac must be configured first
        return (DVT_STATUS_FAILURE);
    }

    if (palAllocateMemory(pMac->hHdd, (void **)&pHalBss, sizeof(tAddBssParams)) != eHAL_STATUS_SUCCESS)
    {
        dvtLog(pMac, LOGE, FL("palAllocateMemory Failed\n"));
        return DVT_STATUS_FAILURE;
    }

    pHalSta = &pHalBss->staContext;
    
    //setting up station params to be used for this mac
    memcpy(pHalSta->bssId, bss.bssId, sizeof(tSirMacAddr));
    pHalSta->assocId = 1;     //no association for simple throughput testing
    pHalSta->staType = 0;     //Self?
    memcpy(pHalSta->staMac, pMac->dvt.mac.macAddr, sizeof(tSirMacAddr));
    pHalSta->listenInterval = 100;
    pHalSta->wmmEnabled = 0;//(tANI_U8)pMac->dvt.mac.wlanCapabilities.edca;
    pHalSta->uAPSD = 0;       //not used
    pHalSta->maxSPLen = 10;   //not used
    pHalSta->htCapable = 0;//(tANI_U8)pMac->dvt.mac.wlanCapabilities.n;
    pHalSta->greenFieldCapable = 0;//(tANI_U8)pMac->dvt.mac.wlanCapabilities.greenfield;
    pHalSta->txChannelWidthSet = 0;
    pHalSta->mimoPS = 0;
    pHalSta->rifsMode = 0;
    pHalSta->lsigTxopProtection = 0;
    pHalSta->delBASupport = 0;
    pHalSta->us32MaxAmpduDuration = 0;
    pHalSta->maxAmpduSize = 0;
    pHalSta->maxAmpduDensity = 0;
    pHalSta->maxAmsduSize = 0;
    memset(pHalSta->staTCParams, 0, sizeof(tTCParams) * 8);
    pHalSta->status = 0;
    pHalSta->staIdx = 0;


    memcpy(pHalBss->bssId, bss.bssId, sizeof(tSirMacAddr));



    if (pMac->dvt.mac.ap == eANI_BOOLEAN_TRUE)
    {
        //ap
        if (bss.bssType == DVT_BSS_INFRASTRUCTURE)
        {
            pHalBss->bssType = eSIR_INFRASTRUCTURE_MODE;
            pHalBss->operMode = 0;
        }
        else
        {
            palFreeMemory(pMac->hHdd, pHalBss);
            return (DVT_STATUS_FAILURE);    //for now if we are setting this as an AP, it should be an infrastructure BSS
        }
    }
    else
    {
        //station
        if (bss.bssType == DVT_BSS_INFRASTRUCTURE)
        {
            pHalBss->bssType = eSIR_INFRASTRUCTURE_MODE;
            pHalBss->operMode = 1;
        }
        else
        {
            pHalBss->bssType = eSIR_IBSS_MODE;
            pHalBss->operMode = 1;
        }
    }

    if (pMac->dvt.mac.wlanCapabilities.b == eANI_BOOLEAN_TRUE)
    {
        pHalBss->nwType = eSIR_11B_NW_TYPE;
    }
    else if (pMac->dvt.mac.wlanCapabilities.g == eANI_BOOLEAN_TRUE)
    {
        pHalBss->nwType = eSIR_11G_NW_TYPE;
    }
    else if (pMac->dvt.mac.wlanCapabilities.a == eANI_BOOLEAN_TRUE)
    {
        pHalBss->nwType = eSIR_11A_NW_TYPE;
    }


    pHalBss->beaconInterval = 100;
    pHalBss->dtimPeriod = 1;
    memset(&pHalBss->cfParamSet, 0, sizeof(tSirMacCfParamSet));

    memset(&pHalBss->rateSet, 0, sizeof(tSirMacRateSet));  //temporary

    if (pMac->dvt.mac.wlanCapabilities.n == eANI_BOOLEAN_TRUE)
    {
        pHalBss->htCapable = 1;
    }
    else
    {
        pHalBss->htCapable = 0;
    }


    pHalBss->htOperMode = 0;
    pHalBss->dualCTSProtection = 0;
    pHalBss->txChannelWidthSet =0;
    pHalBss->currentOperChannel = 1;
    pHalBss->currentExtChannel = 5;
    pHalBss->status = 0;
    pHalBss->bssIdx = 0;
    pHalBss->respReqd = 0;

    {
        tSirMsgQ halMsg;

        halMsg.type =  SIR_HAL_ADD_BSS_REQ;
        halMsg.reserved = 0;
        halMsg.bodyptr = pHalBss;
        halMsg.bodyval = 0;

        if (halPostMsgApi(pMac, &halMsg) == eHAL_STATUS_SUCCESS)
        {
            tANI_U16 bssIndex = pHalBss->bssIdx;

            assert(bssIndex == 0);  //just a single bss so far

            //store BSS to Dvt - for now just a single bss
            memcpy(&pMac->dvt.bss[bssIndex], &bss, sizeof(sDvtSimpleBssConfig));
            pMac->dvt.bss[bssIndex].configured = eANI_BOOLEAN_TRUE;
        }
        else
        {
            //could not add the bss
            palFreeMemory(pMac->hHdd, pHalBss);
            return (DVT_STATUS_FAILURE);
        }
    }

    return (retVal);
}

eANI_DVT_STATUS dvtAddSimpleStation(tpAniSirGlobal pMac, sDvtSimpleStationConfig sta, tANI_U32 *stationIndex)
{
    eANI_DVT_STATUS retVal;
    tpAddStaParams pHalSta;

    
    if (palAllocateMemory(pMac->hHdd, (void **)&pHalSta, sizeof(tAddStaParams)) != eHAL_STATUS_SUCCESS)
    {
        dvtLog(pMac, LOGE, FL("palAllocateMemory Failed\n"));
        return DVT_STATUS_FAILURE;
    }

    memcpy(pHalSta->bssId, pMac->dvt.bss[0].bssId, sizeof(tSirMacAddr));

    pHalSta->assocId = 1;     //no association for simple throughput testing
    memcpy(pHalSta->staMac, sta.macAddr, sizeof(tSirMacAddr));

    if(memcmp(pHalSta->staMac, pMac->dvt.mac.macAddr, 6))
    {
        // STA's address does not match own MAC address, so this
        // must be a peer STA. This logic needs to be revisited when
        // we add test cases for testing the Multiple BSSID feature.
        pHalSta->staType = STA_ENTRY_PEER;
    }
    else
    {
        // STA's address matches own MAC address, so this
        // must be self STA. This logic needs to be revisited when
        // we add test cases for testing the Multiple BSSID feature.
        pHalSta->staType = STA_ENTRY_SELF;
    }

    pHalSta->listenInterval = 100;
    pHalSta->wmmEnabled = 0;//(tANI_U8)pMac->dvt.mac.wlanCapabilities.edca;
    pHalSta->uAPSD = 0;       //not used
    pHalSta->maxSPLen = 10;   //not used
    pHalSta->htCapable = 0;//(tANI_U8)pMac->dvt.mac.wlanCapabilities.n;
    pHalSta->greenFieldCapable = 0;//(tANI_U8)pMac->dvt.mac.wlanCapabilities.greenfield;
    pHalSta->txChannelWidthSet = 0;
    pHalSta->mimoPS = 0;
    pHalSta->rifsMode = 0;
    pHalSta->lsigTxopProtection = 0;
    pHalSta->delBASupport = 0;
    //pHalSta->primaryRateIndex = 0;
    //pHalSta->secondaryRateIndex = 0;
    //pHalSta->tertiaryRateIndex = 0;
    pHalSta->us32MaxAmpduDuration = 0;
    pHalSta->maxAmpduSize = 0;
    pHalSta->maxAmpduDensity = 0;
    pHalSta->maxAmsduSize = 0;
    memset(pHalSta->staTCParams, 0, sizeof(tTCParams) * 8);
    pHalSta->status = 0;
    pHalSta->staIdx = 0;

    {
        tSirMsgQ halMsg;

        halMsg.type =  SIR_HAL_ADD_STA_REQ;
        halMsg.reserved = 0;
        halMsg.bodyptr = pHalSta;
        halMsg.bodyval = 0;

        if (halPostMsgApi(pMac, &halMsg) != eHAL_STATUS_SUCCESS)
        {
            palFreeMemory(pMac->hHdd, pHalSta);
            return DVT_STATUS_FAILURE;
        }
    }

    assert(pHalSta->staIdx < NUM_DEST_STATIONS);
    memcpy(pMac->dvt.simpleStaTable[pHalSta->staIdx].macAddr, sta.macAddr, sizeof(tSirMacAddr));
    pMac->dvt.simpleStaTable[pHalSta->staIdx].configured = eANI_BOOLEAN_TRUE;

    //pHalSta->should come back with the station index and the status filled in
    if (pHalSta->status == eHAL_STATUS_SUCCESS)
    {
        *stationIndex = pHalSta->staIdx;
        return (DVT_STATUS_SUCCESS);
    }
    else
    {
        *stationIndex = pHalSta->staIdx;
        return (DVT_STATUS_FAILURE);
    }
}

eANI_DVT_STATUS dvtDeleteStation(tpAniSirGlobal pMac, tANI_U32 stationIndex)
{
    eANI_DVT_STATUS retVal = DVT_STATUS_SUCCESS;
    tSirMsgQ halMsg;
    tpDeleteStaParams pParams;

    if (palAllocateMemory(pMac->hHdd, (void **)&pParams, sizeof(tDeleteStaParams)) != eHAL_STATUS_SUCCESS)
    {
        dvtLog(pMac, LOGE, FL("palAllocateMemory Failed\n"));
        return DVT_STATUS_FAILURE;
    }

    pParams->staIdx = (tANI_U16)stationIndex;
    pParams->status = DVT_STATUS_SUCCESS;
    pParams->respReqd = 1;

    halMsg.type = SIR_HAL_DELETE_STA_REQ;
    halMsg.reserved = 0;
    halMsg.bodyptr = pParams;
    halMsg.bodyval = 0;
    
    retVal = halPostMsgApi(pMac, &halMsg);
    if (retVal != eHAL_STATUS_SUCCESS)
    {
        palFreeMemory(pMac->hHdd, pParams);
    }

    return (retVal);
}

eANI_DVT_STATUS dvtGetStationTable(tpAniSirGlobal pMac, tDvtMsgGetStaTable *pTable)
{
    int i;

    // ToDo: I'd feel a lot better if we were checking the size of 'pTable' somehow...
    for (i = 0; i < NUM_DEST_STATIONS; ++i)
    {
        memcpy( &( pTable->entries[i].macAddr ), &( pMac->dvt.simpleStaTable[i].macAddr ), 6 );

        pTable->entries[i].configured = pMac->dvt.simpleStaTable[i].configured;
    }

    return (DVT_STATUS_SUCCESS);
}

eANI_DVT_STATUS dvtMemTest(tpAniSirGlobal pMac, tDvtMemTest memTest, tANI_U32 *failedAddr)
{
    tANI_U32 iterSize;
    tANI_U8 writeBuf[128];
    tANI_U8 readBuf[128];
    tANI_U8 byte;
    tANI_U32 addr = memTest.startAddr;

    switch (memTest.accessSize)
    {
        case DVT_MEM_TEST_8BIT_ACCESS:
            iterSize = 1;
            break;
        case DVT_MEM_TEST_16BIT_ACCESS:
            iterSize = 2;
            break;
        case DVT_MEM_TEST_32BIT_ACCESS:
            iterSize = 4;
            break;
        case DVT_MEM_TEST_128BYTE_ACCESS:
            iterSize = 128;
            break;

        default:
            return (DVT_STATUS_INVALID_PARM);
            break;
    }

    while (addr < memTest.startAddr + memTest.length)
    {
        switch (memTest.type)
        {
            case DVT_MEM_TEST_WRITE_READBACK_RANDOM:
                for (byte = 0; byte < iterSize; byte++)
                {
                    writeBuf[byte] = (tANI_U8)(rand() & 0xFF);
                }
                break;

            case DVT_MEM_TEST_WRITE_READBACK_VALUE:
                for (byte = 0; byte < iterSize; byte++)
                {
                    writeBuf[byte] = (tANI_U8)(memTest.value & 0xFF);
                }
                break;

            default:
                return (DVT_STATUS_INVALID_PARM);
                break;
        }

        if (palWriteDeviceMemory(pMac->hHdd, addr, writeBuf, iterSize) != eHAL_STATUS_SUCCESS)
        {
            return (DVT_STATUS_FAILURE);
        }
        else if (palReadDeviceMemory(pMac->hHdd, addr, readBuf, iterSize) != eHAL_STATUS_SUCCESS)
        {
            return (DVT_STATUS_FAILURE);
        }
        else
        {
            if (memcmp(writeBuf, readBuf, iterSize) != 0)
            {
                //write value doesn't match read value
                *failedAddr = addr;

                return (DVT_STATUS_FAILURE);
            }
        }

        addr += iterSize;
    }

    return (DVT_STATUS_SUCCESS);
}

eANI_DVT_STATUS dvtGetDpuSignature(tpAniSirGlobal pMac, tANI_U16 staIdx, tANI_U8 *dpuSignature)
{
    eANI_DVT_STATUS retVal = DVT_STATUS_SUCCESS;

    tSirMsgQ halMsg;
    tpGetDpuParams  pParam;

    if (palAllocateMemory(pMac->hHdd, (void **)&pParam, sizeof(tGetDpuParams)) != eHAL_STATUS_SUCCESS)
    {
        dvtLog(pMac, LOGE, FL("palAllocateMemory Failed\n"));
        return DVT_STATUS_FAILURE;
    }
    
    halMsg.type = SIR_HAL_GET_DPUINFO_REQ;
    halMsg.reserved = 0;
    pParam->staIdx = staIdx;
    pParam->status = DVT_STATUS_SUCCESS;
    pParam->dpuDescIndx = 0;
    pParam->dpuSignature = 0;
    halMsg.bodyptr = pParam;
    halMsg.bodyval = 0;

    retVal = halPostMsgApi(pMac, &halMsg);
    if (retVal != eHAL_STATUS_SUCCESS)
    {
        palFreeMemory(pMac->hHdd, pParam);
    }    

    return (retVal);
}

eANI_DVT_STATUS dvtSetStaKeyParams(tpAniSirGlobal pMac, tSetStaKeyParams keyParams)
{
    eANI_DVT_STATUS retVal = DVT_STATUS_SUCCESS;
    tpSetStaKeyParams pKeyParams;
    tSirMsgQ halMsg;

    if (palAllocateMemory(pMac->hHdd, (void **)&pKeyParams, sizeof(tSetStaKeyParams)) != eHAL_STATUS_SUCCESS)
    {
        dvtLog(pMac, LOGE, FL("palAllocateMemory Failed\n"));
        return DVT_STATUS_FAILURE;
    }

    palCopyMemory(pMac->hHdd, pKeyParams, &keyParams, sizeof(tSetStaKeyParams));
    
    halMsg.type = SIR_HAL_SET_STAKEY_REQ;
    halMsg.reserved = 0;
    halMsg.bodyptr = pKeyParams;
    halMsg.bodyval = 0;

    retVal = halPostMsgApi(pMac, &halMsg);
    if (retVal != eHAL_STATUS_SUCCESS)
    {
        palFreeMemory(pMac->hHdd, pKeyParams);
    }

    return (retVal);
}

eANI_DVT_STATUS dvtSetBssKeyParams(tpAniSirGlobal pMac, tSetBssKeyParams keyParams)
{
    eANI_DVT_STATUS retVal = DVT_STATUS_SUCCESS;
    tpSetBssKeyParams pKeyParams;
    tSirMsgQ halMsg;
    
    if (palAllocateMemory(pMac->hHdd, (void **)&pKeyParams, sizeof(tSetBssKeyParams)) != eHAL_STATUS_SUCCESS)
    {
        dvtLog(pMac, LOGE, FL("palAllocateMemory Failed\n"));
        return DVT_STATUS_FAILURE;
    }
    
    palCopyMemory(pMac->hHdd, pKeyParams, &keyParams, sizeof(tSetBssKeyParams));
    
    halMsg.type = SIR_HAL_SET_BSSKEY_REQ;
    halMsg.reserved = 0;
    halMsg.bodyptr = pKeyParams;
    halMsg.bodyval = 0;
    
    retVal = halPostMsgApi(pMac, &halMsg);
    if (retVal != eHAL_STATUS_SUCCESS)
    {
        palFreeMemory(pMac->hHdd, pKeyParams);
    }

    return (retVal);
}

eANI_DVT_STATUS dvtAddStation(tpAniSirGlobal pMac, tAddStaParams sta)
{
    eANI_DVT_STATUS retVal = DVT_STATUS_SUCCESS;
    tpAddStaParams pSta;
    tSirMsgQ halMsg;

    
    if (palAllocateMemory(pMac->hHdd, (void **)&pSta, sizeof(tAddStaParams)) != eHAL_STATUS_SUCCESS)
    {
        dvtLog(pMac, LOGE, FL("palAllocateMemory Failed\n"));
        return DVT_STATUS_FAILURE;
    }
    
    palCopyMemory(pMac->hHdd, pSta, &sta, sizeof(tAddStaParams));

    halMsg.type = SIR_HAL_ADD_STA_REQ;
    halMsg.reserved = 0;
    halMsg.bodyptr = pSta;
    halMsg.bodyval = 0;

    retVal = halPostMsgApi(pMac, &halMsg);
    if (retVal != eHAL_STATUS_SUCCESS)
    {
        palFreeMemory(pMac->hHdd, pSta);
    }

    return (retVal);
}

eANI_DVT_STATUS dvtBssConfig(tpAniSirGlobal pMac, tAddBssParams bss)
{
    eANI_DVT_STATUS retVal = DVT_STATUS_SUCCESS;
    tSirMsgQ halMsg;
    tpAddBssParams pBss;
    
    if (palAllocateMemory(pMac->hHdd, (void **)&pBss, sizeof(tAddBssParams)) != eHAL_STATUS_SUCCESS)
    {
        dvtLog(pMac, LOGE, FL("palAllocateMemory Failed\n"));
        return DVT_STATUS_FAILURE;
    }
    
    palCopyMemory(pMac->hHdd, pBss, &bss, sizeof(tAddBssParams));
    
    halMsg.type = SIR_HAL_ADD_BSS_REQ;
    halMsg.reserved = 0;
    halMsg.bodyptr = pBss;
    halMsg.bodyval = 0;

    retVal = halPostMsgApi(pMac, &halMsg);
    if (retVal != eHAL_STATUS_SUCCESS)
    {
        palFreeMemory(pMac->hHdd, pBss);
    }

    return (retVal);
}

eANI_DVT_STATUS dvtSetEepromBurstPageSize(tpAniSirGlobal pMac, tANI_U32 pageSize)
{
    eANI_DVT_STATUS retVal = DVT_STATUS_SUCCESS;

    retVal = asicNVISetBurstPageSize(pMac, EEPROM_CHIP_SELECT, 32);

    return (retVal);
}

eANI_DVT_STATUS dvtGetTxCountPerRate(tpAniSirGlobal pMac, tANI_U32 *cTxCountPerRate, tANI_U32 size)
{
    eANI_DVT_STATUS retVal = DVT_STATUS_SUCCESS;
    retVal = palCopyMemory(pMac->hHdd, (tANI_U8 *)cTxCountPerRate, (tANI_U8 *)(pMac->dvt.cTxCountPerRate), size);

    return (retVal);
}


eANI_DVT_STATUS dvtGetRxCountPerRate(tpAniSirGlobal pMac, tANI_U32 *cRxCountPerRate, tANI_U32 size)
{
    eANI_DVT_STATUS retVal = DVT_STATUS_SUCCESS;
    retVal = palCopyMemory(pMac->hHdd, (tANI_U8 *)cRxCountPerRate, (tANI_U8 *)(pMac->dvt.cRxCountPerRate), size);

    return (retVal);
}

eANI_DVT_STATUS dvtGetRxPMIRateIdxMisMatchCount(tpAniSirGlobal pMac, tANI_U32 *cRxPMIRateIdxMisMatchCount, tANI_U32 size)
{
    eANI_DVT_STATUS retVal = DVT_STATUS_SUCCESS;
    retVal = palCopyMemory(pMac->hHdd, (tANI_U8 *)cRxPMIRateIdxMisMatchCount, (tANI_U8 *)(pMac->dvt.cRxPMIRateIdxMisMatchCount), NUM_HAL_PHY_RATES * sizeof(tANI_U32));

    return (retVal);
}

eANI_DVT_STATUS dvtGetRxCounters(tpAniSirGlobal pMac, tDvtRxCounters *rxCounters, tANI_U8 size)
{
    eANI_DVT_STATUS retVal = DVT_STATUS_SUCCESS;
    tANI_U32 i, okCount = 0, errCount = 0;
    for(i = 0;i < MAX_RX_PIPES;i++)
    {
        okCount = okCount + pMac->dvt.dvtInfo.rxInfo.cRxOKCounts[i];
        errCount = errCount + pMac->dvt.dvtInfo.rxInfo.cRXErrCounts[i];
    }
    rxCounters->cRxOKCounts = okCount;
    rxCounters->cRXErrCounts = errCount;

    okCount = 0;
    errCount = 0;
    for(i = 0;i < MAX_TX_PIPES;i++)
    {
        okCount = okCount + pMac->dvt.dvtInfo.txInfo.cTxOKCounts[i];
        errCount = errCount + pMac->dvt.dvtInfo.txInfo.cTxAttempCounts[i];
    }
    rxCounters->cTxAttempCounts = errCount;
    rxCounters->cTxOKCounts = okCount;
    retVal = palCopyMemory(pMac->hHdd, (tANI_U8 *)(rxCounters->cRxCountPerTid), (tANI_U8 *)(pMac->dvt.dvtInfo.rxInfo.cRxCountPerTid), MAX_TIDS * sizeof(tANI_U32));
    retVal = palCopyMemory(pMac->hHdd, (tANI_U8 *)(rxCounters->cRxErrCountPerTid), (tANI_U8 *)(pMac->dvt.dvtInfo.rxInfo.cRxErrCountPerTid), MAX_TIDS * sizeof(tANI_U32));
    retVal = palCopyMemory(pMac->hHdd, (tANI_U8 *)(rxCounters->rxpFlags), (tANI_U8 *)(pMac->dvt.rxpFlags), MAX_RX_PIPES * sizeof(tDvtRxpFlags));
    retVal = palCopyMemory(pMac->hHdd, (tANI_U8 *)(rxCounters->cRxDataFrames), (tANI_U8 *)(pMac->dvt.cRxDataFrames), MAX_NUM_DATA_FRAMES * sizeof(tANI_U32));
    rxCounters->cRxControlFrames = pMac->dvt.cRxControlFrames;
    rxCounters->cRxMgmtFrames = pMac->dvt.cRxMgmtFrames;
    rxCounters->cRxBeaconFrames = pMac->dvt.cRxBeaconFrames;
    rxCounters->cRxBroadcastFrames = pMac->dvt.cRxBroadcastFrames;
    rxCounters->cRxMulticastFrames = pMac->dvt.cRxMulticastFrames;
    rxCounters->cRxUnicastFrames = pMac->dvt.cRxUnicastFrames;
    return (retVal);
}

eANI_DVT_STATUS dvtDumpRegs(tpAniSirGlobal pMac, tANI_U32 *regAddr, tANI_U32 *regVal, tANI_U32 size)
{
    eANI_DVT_STATUS retVal = DVT_STATUS_SUCCESS;
    tANI_U32 i;
    for(i = 0;i < size; i++)
    {
        if (palReadRegister(pMac->hHdd, regAddr[i], &regVal[i] ) != eHAL_STATUS_SUCCESS)
            return eHAL_STATUS_FAILURE;
    }
    return (retVal);
}

eANI_DVT_STATUS dvtReadRegs(tpAniSirGlobal pMac, sDvtReadRegs *regs, tANI_U8 size)
{
    eANI_DVT_STATUS retVal = DVT_STATUS_SUCCESS;

    if ( dvtDumpRegs(pMac, dpuRegs, regs->dpuRegVals, MAX_DPU_REGS) != eHAL_STATUS_SUCCESS)
        return eHAL_STATUS_FAILURE;
    if ( dvtDumpRegs(pMac, bmuRegs, regs->bmuRegVals, MAX_BMU_REGS) != eHAL_STATUS_SUCCESS)
        return eHAL_STATUS_FAILURE;
    if ( dvtDumpRegs(pMac, dxeRegs, regs->dxeRegVals, MAX_DXE_REGS) != eHAL_STATUS_SUCCESS)
        return eHAL_STATUS_FAILURE;
    if ( dvtDumpRegs(pMac, rxpRegs, regs->rxpRegVals, MAX_RXP_REGS) != eHAL_STATUS_SUCCESS)
        return eHAL_STATUS_FAILURE;
    if ( dvtDumpRegs(pMac, txpRegs, regs->txpRegVals, MAX_TXP_REGS) != eHAL_STATUS_SUCCESS)
        return eHAL_STATUS_FAILURE;
    if ( dvtDumpRegs(pMac, mtuRegs, regs->mtuRegVals, MAX_MTU_REGS) != eHAL_STATUS_SUCCESS)
        return eHAL_STATUS_FAILURE;

    return (retVal);
}

eANI_DVT_STATUS dvtGetCountPerSta(tpAniSirGlobal pMac, tANI_U32 *countPerSta, tANI_BOOLEAN isTx)
{
    eANI_DVT_STATUS retVal = DVT_STATUS_SUCCESS;
     if(isTx == eANI_BOOLEAN_TRUE) // get tx counts
        retVal = palCopyMemory(pMac->hHdd, (tANI_U8 *)countPerSta, (tANI_U8 *)(pMac->dvt.cTxCountperSta), MAX_TIDS * NUM_DEST_STATIONS * sizeof(tANI_U32));
     else
        retVal = palCopyMemory(pMac->hHdd, (tANI_U8 *)countPerSta, (tANI_U8 *)(pMac->dvt.cRxCountperSta), MAX_TIDS * NUM_DEST_STATIONS * sizeof(tANI_U32));

     return (retVal);
}

eANI_DVT_STATUS dvtWriteBeaconToMemory(tpAniSirGlobal pMac, tANI_U8 *beacon, tANI_U16 bssIndex, tANI_U32 length)
{
    eANI_DVT_STATUS retVal = DVT_STATUS_SUCCESS;
    retVal = halWriteBeaconToMemory(pMac, beacon, bssIndex, length);

    return (retVal);
}

eANI_DVT_STATUS dvtGetSmacRuntimeStats(tpAniSirGlobal pMac, tDvtSmacRuntimeStat *smacStats, tANI_U8 size)
{
    eANI_DVT_STATUS retVal = DVT_STATUS_SUCCESS;
    tSmacRuntimeStat smacRuntimeStat;

    if ( palReadDeviceMemory( pMac->hHdd, 0x00000000, (tANI_U8 *)&smacRuntimeStat, sizeof(tSmacRuntimeStat) ) != eHAL_STATUS_SUCCESS)
        retVal = DVT_STATUS_FAILURE;

    utilByteSwapU32((tANI_U32 *)&smacRuntimeStat, sizeof(tSmacRuntimeStat));

    smacStats->reserved1[0] = smacRuntimeStat.reserved1[0];
    smacStats->reserved1[1] = smacRuntimeStat.reserved1[1];
    smacStats->signature = smacRuntimeStat.signature;
    smacStats->arch = smacRuntimeStat.arch;
    smacStats->version = smacRuntimeStat.version;
    smacStats->caps = smacRuntimeStat.caps;
    smacStats->addInfo = smacRuntimeStat.addInfo;
    smacStats->heartbeat = smacRuntimeStat.heartbeat;
    smacStats->irqStatus = smacRuntimeStat.irqStatus;
    smacStats->smacStatus = smacRuntimeStat.smacStatus;
    smacStats->errorAddr = smacRuntimeStat.errorAddr;
    smacStats->errorInfo = smacRuntimeStat.errorInfo;
    smacStats->smacReg1 = smacRuntimeStat.smacReg1;
    smacStats->smacReg2 = smacRuntimeStat.smacReg2;
    smacStats->smacReg3 = smacRuntimeStat.smacReg3;
    smacStats->smacDiagInfoBlockOffset = smacRuntimeStat.smacDiagInfoBlockOffset;
    smacStats->sysErrorCount = smacRuntimeStat.sysErrorCount;
    smacStats->checkedErrorCount = smacRuntimeStat.checkedErrorCount;
    smacStats->mboxMsgTxCount = smacRuntimeStat.mboxMsgTxCount;
    smacStats->mboxMsgTxFailedCount = smacRuntimeStat.mboxMsgTxFailedCount;
    smacStats->mboxMsgTxLastErrno = smacRuntimeStat.mboxMsgTxLastErrno;
    smacStats->mboxMsgRxCount = smacRuntimeStat.mboxMsgRxCount;
    smacStats->mboxMsgRxIgnoredCount = smacRuntimeStat.mboxMsgRxIgnoredCount;
    smacStats->mboxMsgRxFailedCount = smacRuntimeStat.mboxMsgRxFailedCount;
    smacStats->beaconTxCount = smacRuntimeStat.beaconTxCount;
    smacStats->beaconRxCount = smacRuntimeStat.beaconRxCount;
    smacStats->txQueueCount = smacRuntimeStat.txQueueCount;
    smacStats->txQueueDropCount = smacRuntimeStat.txQueueDropCount;
    smacStats->txProgramCount = smacRuntimeStat.txProgramCount;
    smacStats->txSendCount = smacRuntimeStat.txSendCount;
    smacStats->txAckedCount = smacRuntimeStat.txAckedCount;
    smacStats->txSendFragCount  = smacRuntimeStat.txSendFragCount ;
    smacStats->txSendAmpduCount = smacRuntimeStat.txSendAmpduCount;
    smacStats->txSendGroupCount = smacRuntimeStat.txSendGroupCount;
    smacStats->txSendNoRespCount = smacRuntimeStat.txSendNoRespCount;
    smacStats->txSendCtrlCount = smacRuntimeStat.txSendCtrlCount;
    smacStats->txRxRespInvalidCount = smacRuntimeStat.txRxRespInvalidCount;
    smacStats->txRetryCount = smacRuntimeStat.txRetryCount;
    smacStats->txRxRespCount = smacRuntimeStat.txRxRespCount;
    smacStats->txTxpErrorCount = smacRuntimeStat.txTxpErrorCount;
    smacStats->txSendBurstCount = smacRuntimeStat.txSendBurstCount;
    smacStats->txDropCount = smacRuntimeStat.txDropCount;
    smacStats->txMissingRxCCAcount = smacRuntimeStat.txMissingRxCCAcount;
    smacStats->txMissingRxPktPushCount = smacRuntimeStat.txMissingRxPktPushCount;
    smacStats->txRxPktdetTimeout = smacRuntimeStat.txRxPktdetTimeout;
    smacStats->txErrorCount = smacRuntimeStat.txErrorCount;
    smacStats->txQueueRawCount = smacRuntimeStat.txQueueRawCount;
    smacStats->txSendConcatCount = 0;
    smacStats->txSendResv = smacRuntimeStat.txSendResv;
    smacStats->txBAretansmitFrameCount = smacRuntimeStat.txBAretansmitFrameCount;
    smacStats->txUnackBACount = smacRuntimeStat.txUnackBACount;
    smacStats->txRetransmitByUnexpectedResp = smacRuntimeStat.txRetransmitByUnexpectedResp;
    smacStats->txRetransmitByTxpReset = smacRuntimeStat.txRetransmitByTxpReset;
    smacStats->txBadStaSignature = smacRuntimeStat.txBadStaSignature;
    smacStats->rxRcvdCount = smacRuntimeStat.rxRcvdCount;
    smacStats->rxTxRespCount = smacRuntimeStat.rxTxRespCount;
    smacStats->rxFragCount = smacRuntimeStat.rxFragCount;
    smacStats->rxFragDropCount = smacRuntimeStat.rxFragDropCount;
    smacStats->rxAmpduCount = smacRuntimeStat.rxAmpduCount;
    smacStats->rxAmsduCount = smacRuntimeStat.rxAmsduCount;
    smacStats->rxAmsduSubFrames = smacRuntimeStat.rxAmsduSubFrames;
    smacStats->rxDroppedCount = smacRuntimeStat.rxDroppedCount;
    smacStats->rxFwdCount = smacRuntimeStat.rxFwdCount;
    smacStats->rxDropDupCount = smacRuntimeStat.rxDropDupCount;
    smacStats->rxMcastCount = smacRuntimeStat.rxMcastCount;
    smacStats->rxConcatCount = smacRuntimeStat.rxConcatCount;

    return (retVal);
}
#endif //ANI_DVT_DEBUG

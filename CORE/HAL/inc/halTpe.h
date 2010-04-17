/**
 *
 *  @file:         halTpe.h
 *
 *  @brief:        Header file for the TPE Hardware Block.
 *
 *  @author:       Madhava Reddy S
 *
 *  Copyright (C) 2002 - 2007, Qualcomm, Inc. All rights reserved.
 *
 *  Change History:
 * ---------------------------------------
 * 09/07/2007  File created.
 * 12/19/2007  Production driver related changes.
 */

#ifndef __HAL_TPE_H_
#define __HAL_TPE_H_
#include "halInterrupts.h"
#include "halFw.h" /* ALIGN4 is defined to be used both host and target buid environment */
#define SW_TEMPLATE_TRANSMISSION_ENABLE     3
#define TPE_SW_SOFTWARE_TX_CONTROL_REG1_SW_WAIT_COMMAND2_VALUE  0x19
#define TPE_SW_SOFTWARE_TX_CONTROL_REG1_SW_WAIT_COMMAND1_VALUE  0x30
#define BEACON_TEMPLATE_HEADER      0x10
#define BEACON_TEMPLATE_BODY            0x20
#define BEACON_TEMPLATE_DATA            0x7D0
#define BEACON_TEMPLATE_CRC         0x04
#define SW_TEMPLATE_CRC             0x04

#define RATE_INDEX                      0xB

#define TPE_STA_DESC_RESERVED_SPACE 0x32    // 50 dwords //0x39     /**< 57 dwords */

#define ONE_TU                      0x400

#define SW_TX_SIFS_A_MODE_CYCLES    0xB4
#define SW_TX_SIFS_B_MODE_CYCLES    0x5A

#define SW_TEMPLATE_DATA_OFFSET 0x10

#define SW_TEMPLATE_HEADER_LEN  0x10

//Converting txop from units of 32us to total us
#define CONVERT_TXOP_TO_US(txop) (txop << 5)

#define AMPDU_DENSITY_DELIMITER  4
#define DENSITY_CAL              (AMPDU_DENSITY_DELIMITER * 8 * 4)


enum {
    MC_STATS_QID_0_MASK = 0,
    BC_STATS_QID_0_MASK,
    MC_STATS_QID_1_MASK,
    BC_STATS_QID_1_MASK,
    MC_STATS_QID_2_MASK,
    BC_STATS_QID_2_MASK,
    MC_STATS_QID_3_MASK,
    BC_STATS_QID_3_MASK,
    MC_STATS_QID_4_MASK,
    BC_STATS_QID_4_MASK,
    MC_STATS_QID_5_MASK,
    BC_STATS_QID_5_MASK,
    MC_STATS_QID_6_MASK,
    BC_STATS_QID_6_MASK,
    MC_STATS_QID_7_MASK,
    BC_STATS_QID_7_MASK,
    MC_STATS_QID_8_MASK,
    BC_STATS_QID_8_MASK,
    MC_STATS_QID_9_MASK,
    BC_STATS_QID_9_MASK,
    MC_STATS_QID_10_MASK,
    BC_STATS_QID_10_MASK,
    MC_STATS_QID_11_MASK,
    BC_STATS_QID_11_MASK,
    MC_STATS_QID_12_MASK,
    BC_STATS_QID_12_MASK,
    MC_STATS_QID_13_MASK,
    BC_STATS_QID_13_MASK,
    MC_STATS_QID_14_MASK,
    BC_STATS_QID_14_MASK,
    MC_STATS_QID_15_MASK,
    BC_STATS_QID_15_MASK
};

/*  QID8 is for broadcast data which uses no ACK policy.
    For QID9 at self STA, we use it for unicast mgmt and set ACK policy to normal ACK.
    QID 10 at self STA, we use it for b/mcast mgmt and set ACK policy to NO ACK.
*/
#define SELF_STA_PER_QUEUE_ACK_POLICY_Q0_15     0x55525555
#define SELF_STA_PER_QUEUE_ACK_POLICY_Q16_19    0x15

#define PEER_STA_PER_QUEUE_ACK_POLICY_Q0_15     0x00000000
#define PEER_STA_PER_QUEUE_ACK_POLICY_Q16_19    0x00

typedef __ani_attr_pre_packed struct sTemplateHeader {
#ifndef ANI_BIG_BYTE_ENDIAN
    tANI_U32  protocol  : 2;
    tANI_U32  type      : 2;
    tANI_U32  subtype   : 4;
    tANI_U32  tods      : 1;
    tANI_U32  fromds    : 1;
    tANI_U32  morefrag  : 1;
    tANI_U32  retry     : 1;
    tANI_U32  pm        : 1;
    tANI_U32  moredata  : 1;
    tANI_U32  wep       : 1;
    tANI_U32  order     : 1;
    tANI_U32  duration  : 16;
#else
    tANI_U32  duration  : 16;
    tANI_U32  order     : 1;
    tANI_U32  wep       : 1;
    tANI_U32  moredata  : 1;
    tANI_U32  pm        : 1;
    tANI_U32  retry     : 1;
    tANI_U32  morefrag  : 1;
    tANI_U32  fromds    : 1;
    tANI_U32  tods      : 1;
    tANI_U32  subtype   : 4;
    tANI_U32  type      : 2;
    tANI_U32  protocol  : 2;
#endif
} __ani_attr_packed __ani_attr_aligned_4 tTemplateHeader;

typedef __ani_attr_pre_packed struct sHwACKTemplate {
    tTemplateHeader hdr;

    tANI_U32 raLo;

#ifndef ANI_BIG_BYTE_ENDIAN
    tANI_U32 raHi       : 16;
    tANI_U32 reserved1  : 16;
#else
    tANI_U32 reserved1  : 16;
    tANI_U32 raHi       : 16;
#endif
} __ani_attr_packed __ani_attr_aligned_4 tHwACKTemplate;

typedef __ani_attr_pre_packed struct sHwRTSTemplate {
    tTemplateHeader hdr;

    tANI_U32 raLo;

#ifndef ANI_BIG_BYTE_ENDIAN
    tANI_U32 raHi       : 16;
    tANI_U32 taLo       : 16;
#else
    tANI_U32 taLo       : 16;
    tANI_U32 raHi       : 16;
#endif

    tANI_U32 taHi;
} __ani_attr_packed __ani_attr_aligned_4 tHwRTSTemplate;

typedef __ani_attr_pre_packed struct sHwCTSTemplate {
    tTemplateHeader hdr;

    tANI_U32 raLo;

#ifndef ANI_BIG_BYTE_ENDIAN
    tANI_U32 raHi       : 16;
    tANI_U32 reserved1  : 16;
#else
    tANI_U32 reserved1  : 16;
    tANI_U32 raHi       : 16;
#endif
} __ani_attr_packed __ani_attr_aligned_4 tHwCTSTemplate;

typedef __ani_attr_pre_packed struct sHwBARTemplate {
    tTemplateHeader hdr;

    tANI_U32 raLo;

#ifndef ANI_BIG_BYTE_ENDIAN
    tANI_U32 raHi       : 16;
    tANI_U32 taLo       : 16;
#else
    tANI_U32 taLo       : 16;
    tANI_U32 raHi       : 16;
#endif

    tANI_U32 taHi;

#ifndef ANI_BIG_BYTE_ENDIAN
    tANI_U32 barControl     : 16;
    tANI_U32 backSSN        : 16;
#else
    tANI_U32 backSSN        : 16;
    tANI_U32 barControl     : 16;
#endif

} __ani_attr_packed __ani_attr_aligned_4 tHwBARTemplate;

typedef __ani_attr_pre_packed struct sHwBATemplate {
    tTemplateHeader hdr;

    tANI_U32 raLo;

#ifndef ANI_BIG_BYTE_ENDIAN
    tANI_U32 raHi       : 16;
    tANI_U32 taLo       : 16;
#else
    tANI_U32 taLo       : 16;
    tANI_U32 raHi       : 16;
#endif

    tANI_U32 taHi;

#ifndef ANI_BIG_BYTE_ENDIAN
    tANI_U32 baCntrl    : 16;
    tANI_U32 backSSN    : 16;
#else
    tANI_U32 backSSN    : 16;
    tANI_U32 baCntrl    : 16;
#endif
} __ani_attr_packed __ani_attr_aligned_4 tHwBATemplate;

typedef __ani_attr_pre_packed struct sHwPSPollTemplate {
#ifndef ANI_BIG_BYTE_ENDIAN
    tANI_U32  protocol  : 2;
    tANI_U32  type      : 2;
    tANI_U32  subtype   : 4;
    tANI_U32  tods      : 1;
    tANI_U32  fromds    : 1;
    tANI_U32  morefrag  : 1;
    tANI_U32  retry     : 1;
    tANI_U32  pm        : 1;
    tANI_U32  moredata  : 1;
    tANI_U32  wep       : 1;
    tANI_U32  order     : 1;
    tANI_U32  aid       : 16;
#else
    tANI_U32  aid       : 16;
    tANI_U32  order     : 1;
    tANI_U32  wep       : 1;
    tANI_U32  moredata  : 1;
    tANI_U32  pm        : 1;
    tANI_U32  retry     : 1;
    tANI_U32  morefrag  : 1;
    tANI_U32  fromds    : 1;
    tANI_U32  tods      : 1;
    tANI_U32  subtype   : 4;
    tANI_U32  type      : 2;
    tANI_U32  protocol  : 2;
#endif

    tANI_U32 bssidLo;

#ifndef ANI_BIG_BYTE_ENDIAN
    tANI_U32 bssidHi        : 16;
    tANI_U32 taLo           : 16;
#else
    tANI_U32 taLo           : 16;
    tANI_U32 bssidHi        : 16;
#endif

    tANI_U32 taHi;
} __ani_attr_packed __ani_attr_aligned_4 tHwPSPollTemplate;

typedef __ani_attr_pre_packed struct sSwPSPollTemplate {
    tTemplateHeader hdr;

    tANI_U32 bssidLo;

#ifndef ANI_BIG_BYTE_ENDIAN
    tANI_U32 bssidHi        : 16;
    tANI_U32 taLo           : 16;
#else
    tANI_U32 taLo           : 16;
    tANI_U32 bssidHi        : 16;
#endif

    tANI_U32 taHi;
} __ani_attr_packed __ani_attr_aligned_4 tSwPSPollTemplate, *tpSwPSPollTemplate;

typedef __ani_attr_pre_packed struct sHwQosNullTemplate {
    tTemplateHeader hdr;

    tANI_U32 raLo;

#ifndef ANI_BIG_BYTE_ENDIAN
    tANI_U32 raHi       : 16;
    tANI_U32 taLo       : 16;
#else
    tANI_U32 taLo       : 16;
    tANI_U32 raHi       : 16;
#endif

    tANI_U32 taHi;
    tANI_U32 bssIdLo;

#ifndef ANI_BIG_BYTE_ENDIAN
    tANI_U32 bssIdHi    : 16;
    tANI_U32 seqCtrl    : 16;
#else
    tANI_U32 seqCtrl    : 16;
    tANI_U32 bssIdHi    : 16;
#endif

#ifndef ANI_BIG_BYTE_ENDIAN
    tANI_U32 tid                : 4;
    tANI_U32 eosp               : 1;
    tANI_U32 ackpolicy          : 2;
    tANI_U32 reserved1          : 1;
    tANI_U32 appsbufferstate    : 8;
    tANI_U32 reserved2          : 16;

#else
    tANI_U32 reserved2          : 16;
    tANI_U32 appsbufferstate    : 8;
    tANI_U32 reserved1          : 1;
    tANI_U32 ackpolicy          : 2;
    tANI_U32 eosp               : 1;
    tANI_U32 tid                : 4;
#endif

} __ani_attr_packed __ani_attr_aligned_4 tHwQosNullTemplate;

typedef __ani_attr_pre_packed struct sHwCFEndTemplate {
    tTemplateHeader hdr;

    tANI_U32 raLo;

#ifndef ANI_BIG_BYTE_ENDIAN
    tANI_U32 raHi       : 16;
    tANI_U32 bssidLo    : 16;
#else
    tANI_U32 bssidLo    : 16;
    tANI_U32 raHi       : 16;
#endif

    tANI_U32 bssidHi;
} __ani_attr_packed __ani_attr_aligned_4 tHwCFEndTemplate;

typedef __ani_attr_pre_packed struct sHwDataNullTemplate {
    tTemplateHeader hdr;

    tANI_U32 raLo;

#ifndef ANI_BIG_BYTE_ENDIAN
    tANI_U32 raHi       : 16;
    tANI_U32 taLo       : 16;
#else
    tANI_U32 taLo       : 16;
    tANI_U32 raHi       : 16;
#endif

    tANI_U32 taHi;
    tANI_U32 bssIdLo;

#ifndef ANI_BIG_BYTE_ENDIAN
    tANI_U32 bssIdHi    : 16;
    tANI_U32 seqCtrl    : 16;
#else
    tANI_U32 seqCtrl    : 16;
    tANI_U32 bssIdHi    : 16;
#endif

} __ani_attr_packed __ani_attr_aligned_4 tHwDataNullTemplate;


typedef __ani_attr_pre_packed struct sSwDataNullTemplate {
    tTemplateHeader hdr;

    tANI_U32 raLo;

#ifndef ANI_BIG_BYTE_ENDIAN
    tANI_U32 raHi       : 16;
    tANI_U32 taLo       : 16;
#else
    tANI_U32 taLo       : 16;
    tANI_U32 raHi       : 16;
#endif

    tANI_U32 taHi;
    tANI_U32 bssIdLo;

#ifndef ANI_BIG_BYTE_ENDIAN
    tANI_U32 bssIdHi    : 16;
    tANI_U32 seqCtrl    : 16;
#else
    tANI_U32 seqCtrl    : 16;
    tANI_U32 bssIdHi    : 16;
#endif

} __ani_attr_packed __ani_attr_aligned_4 tSwDataNullTemplate, *tpSwDataNullTemplate;

typedef union {
    tHwACKTemplate      ackTemplate;
    tHwRTSTemplate      rtsTemplate;
    tHwCTSTemplate      ctsTemplate;
    tHwQosNullTemplate  qosNullTemplate;
    tHwDataNullTemplate dataNullTemplate;
    tHwBARTemplate      barTemplate;
    tHwBATemplate       baTemplate;
    tHwPSPollTemplate   psPollTemplate;
    tHwCFEndTemplate    cfEndTemplate;
} uHwTemplates;

typedef __ani_attr_pre_packed struct sTpeStaDesc {

    /** Byte 0 - 3 */
    tANI_U32 macAddr1Lo;

    /** Byte 4 - 7 */
#ifdef ANI_BIG_BYTE_ENDIAN
    tANI_U32 macAddr2Lo : 16;
    tANI_U32 macAddr1Hi : 16;
#else
    tANI_U32 macAddr1Hi : 16;
    tANI_U32 macAddr2Lo : 16;
#endif

    /** Byte 8 - 11 */
    tANI_U32 macAddr2Hi;

    /** Byte 12 - 15 */
#ifdef ANI_BIG_BYTE_ENDIAN
    tANI_U32 reserved1              : 6;
    tANI_U32 protection_type        : 2;
    tANI_U32 reserved2              : 12;
    tANI_U32 retry_threshold2       : 4;
    tANI_U32 retry_threshold1       : 4;
    tANI_U32 retry_threshold0       : 4;
#else
    tANI_U32 retry_threshold0       : 4;
    tANI_U32 retry_threshold1       : 4;
    tANI_U32 retry_threshold2       : 4;
    tANI_U32 reserved2              : 12;
    tANI_U32 protection_type        : 2;
    tANI_U32 reserved1              : 6;
#endif

    /** Byte 16-19 */
    tANI_U32 ack_policy_vectorLo;

    //Byte 20 - 23
#ifdef ANI_BIG_BYTE_ENDIAN
    tANI_U32 unused1                : 7;
    tANI_U32 ampdu_valid            : 19;
    tANI_U32 ack_policy_vectorHi    : 6;
#else
    tANI_U32 ack_policy_vectorHi    : 6;
    tANI_U32 ampdu_valid            : 19;
    tANI_U32 unused1                : 7;
#endif

    // Byte 24-59
    tTpeStaDescRateInfo rate_params_20Mhz[TPE_STA_MAX_RETRY_RATE];
    tTpeStaDescRateInfo rate_params_40Mhz[TPE_STA_MAX_RETRY_RATE];
    tTpeStaDescRateInfo bd_rate_params[TPE_STA_MAX_RETRY_RATE];
    //Byte 60 - 63
#ifdef ANI_BIG_BYTE_ENDIAN
    tANI_U32 unused2                : 24;
    tANI_U32 data_wt_cycles         : 8;
#else
    tANI_U32 data_wt_cycles         : 8;
    tANI_U32 unused2                : 24;
#endif

    //Byte 64 - 67
#ifdef ANI_BIG_BYTE_ENDIAN
    tANI_U32 reserved3              : 2;
    tANI_U32 max_bytes_in_ampdu     : 2;
    tANI_U32 bssid_of_sta           : 4;
    tANI_U32 tsfoffset_for_probe_resp_bd_rate_0_1       : 12;
    tANI_U32 tsfoffset_for_probe_resp_bd_rate_2_3       : 12;
#else
    tANI_U32 tsfoffset_for_probe_resp_bd_rate_2_3       : 12;
    tANI_U32 tsfoffset_for_probe_resp_bd_rate_0_1       : 12;
    tANI_U32 bssid_of_sta           : 4;
    tANI_U32 max_bytes_in_ampdu     : 2;
    tANI_U32 reserved3              : 2;
#endif

    //Byte 68 - 71
#ifdef ANI_BIG_BYTE_ENDIAN
    tANI_U32 reserved4              : 2;
    tANI_U32 ampdu_window_size_qid4 : 6;
    tANI_U32 ampdu_window_size_qid3 : 6;
    tANI_U32 ampdu_window_size_qid2 : 6;
    tANI_U32 ampdu_window_size_qid1 : 6;
    tANI_U32 ampdu_window_size_qid0 : 6;
#else
    tANI_U32 ampdu_window_size_qid0 : 6;
    tANI_U32 ampdu_window_size_qid1 : 6;
    tANI_U32 ampdu_window_size_qid2 : 6;
    tANI_U32 ampdu_window_size_qid3 : 6;
    tANI_U32 ampdu_window_size_qid4 : 6;
    tANI_U32 reserved4              : 2;
#endif

    //Byte 72 - 75
#ifdef ANI_BIG_BYTE_ENDIAN
    tANI_U32 reserved5              : 2;
    tANI_U32 ampdu_window_size_qid9 : 6;
    tANI_U32 ampdu_window_size_qid8 : 6;
    tANI_U32 ampdu_window_size_qid7 : 6;
    tANI_U32 ampdu_window_size_qid6 : 6;
    tANI_U32 ampdu_window_size_qid5 : 6;
#else
    tANI_U32 ampdu_window_size_qid5 : 6;
    tANI_U32 ampdu_window_size_qid6 : 6;
    tANI_U32 ampdu_window_size_qid7 : 6;
    tANI_U32 ampdu_window_size_qid8 : 6;
    tANI_U32 ampdu_window_size_qid9 : 6;
    tANI_U32 reserved5              : 2;
#endif

    //Byte 76 - 79
#ifdef ANI_BIG_BYTE_ENDIAN
    tANI_U32 reserved6                  : 2;
    tANI_U32 ampdu_window_size_qid14    : 6;
    tANI_U32 ampdu_window_size_qid13    : 6;
    tANI_U32 ampdu_window_size_qid12    : 6;
    tANI_U32 ampdu_window_size_qid11    : 6;
    tANI_U32 ampdu_window_size_qid10    : 6;
#else
    tANI_U32 ampdu_window_size_qid10    : 6;
    tANI_U32 ampdu_window_size_qid11    : 6;
    tANI_U32 ampdu_window_size_qid12    : 6;
    tANI_U32 ampdu_window_size_qid13    : 6;
    tANI_U32 ampdu_window_size_qid14    : 6;
    tANI_U32 reserved6                  : 2;
#endif

    //Byte 80 - 83
#ifdef ANI_BIG_BYTE_ENDIAN
    tANI_U32 reserved7                  : 2;
    tANI_U32 ampdu_window_size_qid19    : 6;
    tANI_U32 ampdu_window_size_qid18    : 6;
    tANI_U32 ampdu_window_size_qid17    : 6;
    tANI_U32 ampdu_window_size_qid16    : 6;
    tANI_U32 ampdu_window_size_qid15    : 6;
#else
    tANI_U32 ampdu_window_size_qid15    : 6;
    tANI_U32 ampdu_window_size_qid16    : 6;
    tANI_U32 ampdu_window_size_qid17    : 6;
    tANI_U32 ampdu_window_size_qid18    : 6;
    tANI_U32 ampdu_window_size_qid19    : 6;
    tANI_U32 reserved7                  : 2;
#endif

    //Byte 84 - 87
#ifdef ANI_BIG_BYTE_ENDIAN
    tANI_U32 reserved8                  : 12;
    tANI_U32 bd_raw_mode                : 20;
#else
    tANI_U32 bd_raw_mode                : 20;
    tANI_U32 reserved8                  : 12;
#endif

    //Byte 88 - 91
    tANI_U32 mcbcStatsQidMap;

    //FIXME_GEN6 - this data strucutture is changed
    //    down below. Need to be modified when AMSDU lifetime
    //    in QOS feature support is provided.
    //Reserving space for additional bytes in TPE sta descriptor
    tANI_U32 rsvd[TPE_STA_DESC_RESERVED_SPACE];

} __ani_attr_packed __ani_attr_aligned_4 tTpeStaDesc, *tpTpeStaDesc;

/**
 *  TPE Rate Adaptation Related Sta Stats
 */
typedef __ani_attr_pre_packed struct sTpeStaRateAdaptStats {

    // 20M Primary Rate stats
#ifdef ANI_BIG_BYTE_ENDIAN
    tANI_U32 tot20MTxPpduDataFrms1 : 16;
    tANI_U32 tot20MTxMpduDataFrms1 : 16;
#else
    tANI_U32 tot20MTxMpduDataFrms1 : 16;
    tANI_U32 tot20MTxPpduDataFrms1 : 16;
#endif

    // 20M Primary Rate stats
#ifdef ANI_BIG_BYTE_ENDIAN
    tANI_U32 tot20MMpduInAmPdu1 : 16;
    tANI_U32 tot20MPpduDataFrmsWithAckTo1 : 16;
#else
    tANI_U32 tot20MPpduDataFrmsWithAckTo1 : 16;
    tANI_U32 tot20MMpduInAmPdu1 : 16;
#endif
    // 20M Primary/Secondary Rate stats
#ifdef ANI_BIG_BYTE_ENDIAN
    tANI_U32 tot20MMpduDataFrmsAcked1 : 16;
    tANI_U32 tot20MTxPpduDataFrms2 : 16;
#else
    tANI_U32 tot20MTxPpduDataFrms2 : 16;
    tANI_U32 tot20MMpduDataFrmsAcked1 : 16;
#endif
    // 20M Secondary Rate stats
#ifdef ANI_BIG_BYTE_ENDIAN
    tANI_U32 tot20MTxMpduDataFrms2 : 16;
    tANI_U32 tot20MMpduInAmPdu2 : 16;
#else
    tANI_U32 tot20MMpduInAmPdu2 : 16;
    tANI_U32 tot20MTxMpduDataFrms2 : 16;
#endif
    // 20M Secondary Rate stats
#ifdef ANI_BIG_BYTE_ENDIAN
    tANI_U32 tot20MPpduDataFrmsWithAckTo2 : 16;
    tANI_U32 tot20MMpduDataFrmsAcked2 : 16;
#else
    tANI_U32 tot20MMpduDataFrmsAcked2 : 16;
    tANI_U32 tot20MPpduDataFrmsWithAckTo2 : 16;
#endif
    // 20M Tertiary Rate stats
#ifdef ANI_BIG_BYTE_ENDIAN
    tANI_U32 tot20MTxPpduDataFrms3 : 16;
    tANI_U32 tot20MTxMpduDataFrms3 : 16;
#else
    tANI_U32 tot20MTxMpduDataFrms3 : 16;
    tANI_U32 tot20MTxPpduDataFrms3 : 16;
#endif
    // 20M Tertiary Rate stats
#ifdef ANI_BIG_BYTE_ENDIAN
    tANI_U32 tot20MMpduInAmPdu3 : 16;
    tANI_U32 tot20MPpduDataFrmsWithAckTo3 : 16;
#else
    tANI_U32 tot20MPpduDataFrmsWithAckTo3 : 16;
    tANI_U32 tot20MMpduInAmPdu3 : 16;
#endif
    // 20M Tertiary Rate stats & 40M Primary Rate stats
#ifdef ANI_BIG_BYTE_ENDIAN
    tANI_U32 tot20MMpduDataFrmsAcked3 : 16;
    tANI_U32 tot40MTxPpduDataFrms1 : 16;
#else
    tANI_U32 tot40MTxPpduDataFrms1 : 16;
    tANI_U32 tot20MMpduDataFrmsAcked3 : 16;
#endif
    // 40M Primary Rate stats
#ifdef ANI_BIG_BYTE_ENDIAN
    tANI_U32 tot40MTxMpduDataFrms1 : 16;
    tANI_U32 tot40MMpduInAmPdu1 : 16;
#else
    tANI_U32 tot40MMpduInAmPdu1 : 16;
    tANI_U32 tot40MTxMpduDataFrms1 : 16;
#endif
    // 40M Primary Rate stats
#ifdef ANI_BIG_BYTE_ENDIAN
    tANI_U32 tot40MPpduDataFrmsWithAckTo1 : 16;
    tANI_U32 tot40MMpduDataFrmsAcked1 : 16;
#else
    tANI_U32 tot40MMpduDataFrmsAcked1 : 16;
    tANI_U32 tot40MPpduDataFrmsWithAckTo1 : 16;
#endif
    // 40M Secondary Rate stats
#ifdef ANI_BIG_BYTE_ENDIAN
    tANI_U32 tot40MTxPpduDataFrms2 : 16;
    tANI_U32 tot40MTxMpduDataFrms2 : 16;
#else
    tANI_U32 tot40MTxMpduDataFrms2 : 16;
    tANI_U32 tot40MTxPpduDataFrms2 : 16;
#endif
    // 40M Secondary Rate stats
#ifdef ANI_BIG_BYTE_ENDIAN
    tANI_U32 tot40MMpduInAmPdu2 : 16;
    tANI_U32 tot40MPpduDataFrmsWithAckTo2 : 16;
#else
    tANI_U32 tot40MPpduDataFrmsWithAckTo2 : 16;
    tANI_U32 tot40MMpduInAmPdu2 : 16;
#endif
    // 40M Secondary Rate stats & 40M Tertiary Rate stats
#ifdef ANI_BIG_BYTE_ENDIAN
    tANI_U32 tot40MMpduDataFrmsAcked2 : 16;
    tANI_U32 tot40MTxPpduDataFrms3 : 16;
#else
    tANI_U32 tot40MTxPpduDataFrms3 : 16;
    tANI_U32 tot40MMpduDataFrmsAcked2 : 16;
#endif
    // 40M Tertiary Rate stats
#ifdef ANI_BIG_BYTE_ENDIAN
    tANI_U32 tot40MTxMpduDataFrms3 : 16;
    tANI_U32 tot40MMpduInAmPdu3 : 16;
#else
    tANI_U32 tot40MMpduInAmPdu3 : 16;
    tANI_U32 tot40MTxMpduDataFrms3 : 16;
#endif
    // 40M Tertiary Rate stats
#ifdef ANI_BIG_BYTE_ENDIAN
    tANI_U32 tot40MPpduDataFrmsWithAckTo3 : 16;
    tANI_U32 tot40MMpduDataFrmsAcked3 : 16;
#else
    tANI_U32 tot40MMpduDataFrmsAcked3 : 16;
    tANI_U32 tot40MPpduDataFrmsWithAckTo3 : 16;
#endif

} __ani_attr_packed __ani_attr_aligned_4 tTpeStaRaStats, *tpTpeStaRaStats;

/**
 *  TPE Rate Adaptation Related Sta Stats
 */
typedef __ani_attr_pre_packed struct sTpeStaDot11Stats {

#ifdef ANI_BIG_BYTE_ENDIAN
        tANI_U32 txFragCnt : 16;
        tANI_U32 txFrmSuccCnt : 16;
#else
        tANI_U32 txFrmSuccCnt : 16;
        tANI_U32 txFragCnt : 16;
#endif

#ifdef ANI_BIG_BYTE_ENDIAN
        tANI_U32 txFrmFailCnt : 16;
        tANI_U32 txMcFrmCnt : 16;
#else
        tANI_U32 txMcFrmCnt : 16;
        tANI_U32 txFrmFailCnt : 16;
#endif

#ifdef ANI_BIG_BYTE_ENDIAN
        tANI_U32 txFrmRetryCnt : 16;
        tANI_U32 txFrmMultiRetryCnt : 16;
#else
        tANI_U32 txFrmMultiRetryCnt : 16;
        tANI_U32 txFrmRetryCnt : 16;
#endif

#ifdef ANI_BIG_BYTE_ENDIAN
        tANI_U32 rtsSuccCnt : 16;
        tANI_U32 rtsFailCnt : 16;
#else
        tANI_U32 rtsFailCnt : 16;
        tANI_U32 rtsSuccCnt : 16;
#endif

#ifdef ANI_BIG_BYTE_ENDIAN
        tANI_U32 ackSuccCnt : 16;
        tANI_U32 ackFailCnt : 16;
#else
        tANI_U32 ackFailCnt : 16;
        tANI_U32 ackSuccCnt : 16;
#endif
} __ani_attr_packed __ani_attr_aligned_4 tTpeStaDot11Stats, *tpeStaDot11Stats;

typedef __ani_attr_pre_packed struct sTpeStaStats {
     tTpeStaRaStats     raStats;
     tTpeStaDot11Stats  dot11Stats[MAX_NUM_OF_BACKOFFS];
} __ani_attr_packed __ani_attr_aligned_4 tTpeStaStats, *tpTpeStaStats;

/**
 *  STA desc and Stats
 */
typedef struct sTpeStaDescAndStats {
    tTpeStaDesc     tpeStaDesc;
    tTpeStaStats    tpeStaStats;
} __ani_attr_packed __ani_attr_aligned_4 tTpeStaDescAndStats, *tpTpeStaDescAndStats;

/**
 *  Sw Template
 */
typedef __ani_attr_pre_packed struct sSwTemplate {

#ifdef ANI_BIG_BYTE_ENDIAN
    tANI_U32 reserved1              : 6;
    tANI_U32 ignore_expected_resp   : 1;
    tANI_U32 template_len           : 12;
    tANI_U32 expected_resp_sub_type : 4;
    tANI_U32 expected_resp_type     : 2;
    tANI_U32 resp_is_expected       : 1;
    tANI_U32 template_sub_type      : 4;
    tANI_U32 template_type          : 2;
#else
    tANI_U32 template_type          : 2;
    tANI_U32 template_sub_type      : 4;
    tANI_U32 resp_is_expected       : 1;
    tANI_U32 expected_resp_type     : 2;
    tANI_U32 expected_resp_sub_type : 4;
    tANI_U32 template_len           : 12;
    tANI_U32 ignore_expected_resp   : 1;
    tANI_U32 reserved1              : 6;
#endif

#ifdef ANI_BIG_BYTE_ENDIAN
    tANI_U32 reserved2                  : 12;
    tANI_U32 tx_power                   : 5;
    tANI_U32 tx_antenna_enable          : 3;
    tANI_U32 reserved3                  : 1;
    tANI_U32 stbc                       : 2;
    tANI_U32 primary_data_rate_index    : 9;
#else
    tANI_U32 primary_data_rate_index    : 9;
    tANI_U32 stbc                       : 2;
    tANI_U32 reserved3                  : 1;
    tANI_U32 tx_antenna_enable          : 3;
    tANI_U32 tx_power                   : 5;
    tANI_U32 reserved2                  : 12;
#endif

#ifdef ANI_BIG_BYTE_ENDIAN
    tANI_U32 reserved4          : 16;
    tANI_U32 tsf_offset         : 16;
#else
    tANI_U32 tsf_offset         : 16;
    tANI_U32 reserved4          : 16;
#endif

    tANI_U32 reserved5;

//    tANI_U8 template_data[2032];

} __ani_attr_packed __ani_attr_aligned_4 tSwTemplate, *tpSwTemplate;

typedef __ani_attr_pre_packed struct sBeaconTemplateBody {
    tTemplateHeader hdr;
    tANI_U32 daLo;


#ifndef ANI_BIG_BYTE_ENDIAN
    tANI_U32 daHi       : 16;
    tANI_U32 saLo       : 16;
#else
    tANI_U32 saLo       : 16;
    tANI_U32 daHi       : 16;
#endif

    tANI_U32 saHi;

    tANI_U32 bssidLo;

#ifndef ANI_BIG_BYTE_ENDIAN
    tANI_U32 bssidHi    : 16;
    tANI_U32 seqCtrl    : 16;
#else
    tANI_U32 seqCtrl    : 16;
    tANI_U32 bssidHi    : 16;
#endif


    tANI_U32 timeStampLo;
    tANI_U32    timeStampHi;

} __ani_attr_packed __ani_attr_aligned_4 tBeaconTemplateBody, *tpBeaconTemplateBody;

typedef __ani_attr_pre_packed struct sBeaconTemplateHeader {
#ifdef ANI_BIG_BYTE_ENDIAN
    tANI_U32 reserved1              : 7;
    tANI_U32 template_len           : 12;
    tANI_U32 expected_resp_sub_type : 4;
    tANI_U32 expected_resp_type     : 2;
    tANI_U32 resp_is_expected       : 1;
    tANI_U32 template_sub_type      : 4;
    tANI_U32 template_type          : 2;
#else
    tANI_U32 template_type          : 2;
    tANI_U32 template_sub_type      : 4;
    tANI_U32 resp_is_expected       : 1;
    tANI_U32 expected_resp_type     : 2;
    tANI_U32 expected_resp_sub_type : 4;
    tANI_U32 template_len           : 12;
    tANI_U32 reserved1              : 7;
#endif

#ifdef ANI_BIG_BYTE_ENDIAN
    tANI_U32 reserved2          : 12;
    tANI_U32 tx_power           : 5;
    tANI_U32 tx_antenna_enable  : 3;
    tANI_U32 reserved3          : 1;
    tANI_U32 stbc               : 2;
    tANI_U32 primary_data_rate_index    : 9;
#else
    tANI_U32 primary_data_rate_index    : 9;
    tANI_U32 stbc               : 2;
    tANI_U32 reserved3          : 1;
    tANI_U32 tx_antenna_enable  : 3;
    tANI_U32 tx_power           : 5;
    tANI_U32 reserved2          : 12;
#endif

#ifdef ANI_BIG_BYTE_ENDIAN
    tANI_U32 reserved4          : 16;
    tANI_U32 tsf_offset         : 16;
#else
    tANI_U32 tsf_offset         : 16;
    tANI_U32 reserved4          : 16;
#endif

    tANI_U32 reserved5;
} __ani_attr_packed __ani_attr_aligned_4 tBeaconTemplateHeader, *tpBeaconTemplateHeader;

typedef __ani_attr_pre_packed struct sBeaconTemplate {
    tBeaconTemplateHeader   template_header;
    tBeaconTemplateBody     template_body;
    tANI_U8                 template_data[BEACON_TEMPLATE_DATA];

} __ani_attr_packed __ani_attr_aligned_4 tBeaconTemplate, *tpBeaconTemplate;

typedef struct sHwTemplateParams {
    tANI_U32 aid;
    tANI_U8 ta[6];
    tANI_U8 bssId[6];
    tANI_U8 pm;
} tHwTemplateParams;

eHalStatus halTpe_Start(tHalHandle hHal, void *arg);
eHalStatus halTpe_InitHwTemplates(tHalHandle hHal, void *arg);
eHalStatus halTpe_RateTableRead(tpAniSirGlobal pMac, tANI_U32 tableAddr,
                                    tANI_U32 *dWord1, tANI_U32 *dWord2);
eHalStatus halTpe_RateTableWrite(tpAniSirGlobal pMac, tANI_U32 tableAddr,
                                    tANI_U32 dWord1, tANI_U32 dWord2);
eHalStatus halTpe_SetStaDesc(tpAniSirGlobal pMac, tANI_U8 staId,
                                                tpTpeStaDesc pTpeStaDesc);
eHalStatus halTpe_GetStaDesc(tpAniSirGlobal pMac, tANI_U8 staId,
                                    tpTpeStaDesc pTpeStaDesc);
eHalStatus halTpe_GetStaStats(tpAniSirGlobal pMac, tANI_U8 staId,
                                    tpTpeStaStats pTpeStaStats);
eHalStatus halTpe_GetStaRaStats(tpAniSirGlobal pMac, tANI_U8 staId,
        tpTpeStaRaStats pTpeStaRaStats);
eHalStatus halTpe_ClearStaRaStats(tpAniSirGlobal pMac, tANI_U8 staId);
eHalStatus halTpe_SetSifsCycle(tpAniSirGlobal pMac, tANI_U32 cfgValue, tANI_U32 mask);
eHalStatus halTpe_SetPmBit(tpAniSirGlobal pMac, tHalBitVal mask);

eHalStatus halTpe_SetBeaconTemplate(tpAniSirGlobal pMac, tANI_U16 beaconIndex, tSirMacAddr  bssId);
eHalStatus halTpe_SaveStaConfig(tpAniSirGlobal pMac, tpTpeStaDesc tpeStaDescCfg, tANI_U8 staIdx);
eHalStatus halTpe_GetStaConfig(tpAniSirGlobal pMac, tpTpeStaDesc *tpeStaDescCfg, tANI_U8 staIdx);
eHalStatus halTpe_RestoreStaConfig(tpAniSirGlobal pMac, tpTpeStaDesc tpeStaDescCfg, tANI_U8 staIdx);
eHalStatus halTpe_SetProtectionThreshold(tpAniSirGlobal pMac, tANI_U32 threshold);
eHalStatus halTpe_UpdateStaDesc(tpAniSirGlobal pMac, tANI_U32 staIdx,
                                                    tpTpeStaDesc tpeStaDesc);
eHalStatus halTpe_SetStaDescRateInfo(tpAniSirGlobal pMac,
        tANI_U32 staIdx, tTpeRateType type, tTpeRetryRate retry,
        tpTpeStaDescRateInfo pTpeRateInfo);
eHalStatus halTpe_GetStaDescRateInfo(tpAniSirGlobal pMac,
        tANI_U32 staIdx, tTpeRateType type,
        tpTpeStaDescRateInfo *pTpeStaDescRateInfo);
eHalStatus halTpe_UpdateStaDescRateInfo(tpAniSirGlobal pMac, tANI_U32 staIdx,
                                tANI_U32 channel, tpTpeStaDescRateInfo ptpeStaDescRateInfo);
void halTpe_DumpEdcaTxOp(tpAniSirGlobal pMac);
eHalStatus halTpe_UpdateStaDescFields(tpAniSirGlobal pMac, tANI_U32 staIdx,
            tANI_U32 startOffset, tANI_U32 length, tpTpeStaDesc tpeStaDesc);
eHalStatus halTpe_EnableBeacon(tpAniSirGlobal pMac, tANI_U16 beaconIndex);
eHalStatus halTpe_DisableBeacon(tpAniSirGlobal pMac, tANI_U16 beaconIndex);
eHalStatus halTpe_ReEnableBeacon(tpAniSirGlobal pMac, tANI_U16 beaconIndex);
eHalStatus halTpe_UpdateEdcaTxOp(tpAniSirGlobal pMac, tANI_U16 *pTxOp);
eHalStatus halTpe_UpdateMtuMaxBssid(tpAniSirGlobal pMac);
eHalStatus halTpe_UpdateBeacon(tpAniSirGlobal pMac, tANI_U8 *beacon,
                                    tANI_U16 beaconIndex, tANI_U32 length);
void halTpe_UpdateMaxMpduInAmpdu(tpAniSirGlobal pMac, tANI_U32 mpdusInAmpdu);
void halTpe_TerminateAmpduAtRateChange(tpAniSirGlobal pMac, tANI_U8 enable);
void halTpe_SetAmpduTxTime(tpAniSirGlobal pMac, tANI_U32 maxAmpduTxTime);
eHalStatus halTpe_SetAcToBkofLookupVec(tpAniSirGlobal pMac);
eHalStatus halTpe_SetLsigTxopProtection(tpAniSirGlobal pMac, tANI_U8 lsigProtFlag, tANI_U8 lsigProtType);
void halTpe_SetPSPollTemplate(tpAniSirGlobal pMac, tHwPSPollTemplate *pPsPoll);
void halTpe_SetQosNullTemplate(tpAniSirGlobal pMac, tANI_U32 type,
                                tANI_U32 sub_type, tHwTemplateParams *hwTemplateParams);
void halTpe_SetDataNullTemplate(tpAniSirGlobal pMac, tANI_U32 type,
                                tANI_U32 sub_type, tHwTemplateParams *hwTemplateParams);
eHalStatus halTpe_CalculateAmpduDensity(tpAniSirGlobal pMac, tANI_U32 rateIdx, tANI_U32 *pAmpduDensity, tANI_U32 maxAmpduDensity);
eHalStatus halIntTpeMcuBdBasedTxInt1PHostHandler( tHalHandle hHalHandle, eHalIntSources intSource );
void halTpe_DumpMpiCmdTableEntry(tpAniSirGlobal pMac, tANI_U32 rateIndex);
void halTpe_PrintMpiCmdTable(tpAniSirGlobal pMac);
#ifdef CONFIGURE_SW_TEMPLATE
eHalStatus halTpe_InitSwTemplateBase(tpAniSirGlobal pMac,
                tANI_U32 swTemplate_offset);
#endif //CONFIGURE_SW_TEMPLATE
eHalStatus halTpe_SetSwTemplate(tpAniSirGlobal pMac, tSwTemplate *swTemplate);
eHalStatus halTpe_Set11gProtectionCntrlIndex(tpAniSirGlobal pMac, tANI_U8 set);
#endif /**< __HAL_TPE_H_ */

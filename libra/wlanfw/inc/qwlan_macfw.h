#ifndef __QWLAN_MACFW_H
#define __QWLAN_MACFW_H

/*===========================================================================

FILE:
  qwlan_macfw.h

BRIEF DESCRIPTION:
   Header file containing the definitions used both by Host and macFw

DESCRIPTION:
  qwlanfw.h contains:
  ** definitions of Ctrl Messages understood by Gen6 FW.
  ** May include HW related definitions.
  This file should be used for Host side development.

                Copyright (c) 2008 QUALCOMM Incorporated.
                All Right Reserved.
                Qualcomm Confidential and Proprietary
===========================================================================*/

/*===========================================================================

                      EDIT HISTORY FOR FILE

$Header$
$DateTime$

when       who            what, where, why
--------   ---          -----------------------------------------------------
08/18/08   chaitanya      Created
09/14/08   hoonki         Rate Adaptation in FW
10/19/04   hoonki         WAPI support for Libra
===========================================================================*/

#define PMU_RXP_SLPPULSE_MTO_WEIRD_SLEEP_WORKAROUND 1

/*===========================================================================
  MCU
 ==========================================================================*/

/*---------------------------------------------------------------------------
 * Mailbox
 *-------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
 * Mailbox assignment
 *-------------------------------------------------------------------------*/

enum {
   QWLAN_MCU_MAILBOX_H2F_CTRL = 0x0,
   QWLAN_MCU_MAILBOX_F2H_CTRL = 0x1,
   QWLAN_MCU_MAILBOX_H2F_DIAG = 0x2,
   QWLAN_MCU_MAILBOX_F2H_DIAG = 0x3,
};


#define MCU_MAILBOX_HOST2FW  QWLAN_MCU_MAILBOX_H2F_CTRL
#define MCU_MAILBOX_FW2HOST  QWLAN_MCU_MAILBOX_F2H_CTRL

#define QWLAN_PMIC_SLEEPCLK_PERIOD_NS 30518

/*===========================================================================
   HOST AND FIRMWARE MACROS
===========================================================================*/
#define QWLANFW_MAX_BCN_FILTER                                  8
#define QWLANFW_MAX_MATCH_PTRNS                                 8
#define QWLANFW_MAX_RSSI_REGS                                  20
#define QWLANFW_MBOX_MSG_VER                                    0
#define QWLANFW_MBOX_MSG_LENGTH                               256
#define QWLANFW_MAGIC_PCKT_PTTRN_ID                             8
#define QWLANFW_MAX_AC                                        0x4
/*===========================================================================
  FW MEMORY MAP
===========================================================================*/
#define QWLANFW_MEM_H2F_MSG_ADDR_OFFSET               0x1000
#define QWLANFW_MEM_F2H_MSG_ADDR_OFFSET               0x1200

#define QWLANFW_MEM_FW_LOG_ADDR_OFFSET                0x1400 /*1400 - 23ff*/
#define QWLANFW_MEM_ERR_STATS_ADDR_OFFSET             0x2400 /*2400 - 247f*/
#define QWLANFW_MEM_PHY_CAL_CORRECTIONS_ADDR_OFFSET   0x2480 /*2480 - 24ff*/
#define QWLANFW_MEM_HW_COUNTERS_ADDR_OFFSET           0x2500 /*2500 - 26ff*/
#define QWLANFW_MEM_SYS_CONFIG_ADDR_OFFSET            0x2700 /*2700 - 27ff*/

/* offsets define memory area sizes */
#define FEATURE_WLANFW_COREX_LOG_BUFFER_SIZE (QWLANFW_MEM_ERR_STATS_ADDR_OFFSET - QWLANFW_MEM_FW_LOG_ADDR_OFFSET)

/* HOST 2 FIRMWARE data structure conversion considering endian */
#ifdef ANI_BIG_BYTE_ENDIAN
#define HTOFS(n) (n)
#define HTOFL(n) (n)
#else /* ANI_BIG_BYTE_ENDIAN */
#define HTOFS(n) (((((unsigned short)(n) & 0xFF)) << 8) | (((unsigned short)(n) & 0xFF00) >> 8))
#define HTOFL(n) (((((unsigned long)(n) & 0xFF)) << 24) | \
                     ((((unsigned long)(n) & 0xFF00)) << 8) | \
                     ((((unsigned long)(n) & 0xFF0000)) >> 8) | \
                     ((((unsigned long)(n) & 0xFF000000)) >> 24))
#endif /* ANI_BIG_BYTE_ENDIAN */
/* align must be power of 2 */
/*  value   CEIL(4) ROUNDUP(4)
    0       0       4
    1       4       4
    2       4       4
    3       4       4
    4       4       8
    5       8       8
    6       8       8
    7       8       8
    8       8      12
*/


#ifdef WLAN_SOFTAP_FEATURE
 /* To  Enable the FW based Beacon Tx &processing */
 #define WLAN_SOFTAP_FW_BEACON_TRANSMIT       1
 /* To Get Host Print Logs */
 #define WLAN_SOFTAP_FW_BEACON_TX_PRNT_LOG    0

 /* To Enable Soft-Ap regulate Flow Logic  */
 #define WLAN_SOFTAP_REG_TRAFFIC_FLOW
 
 /* To enable Soft-AP keep alive feature  */
 #define WLAN_SOFTAP_FEATURE_KEEPALIVE
 /* To enable Soft-AP keep alive print log  */
 //#define WLAN_SOFTAP_FEATURE_KEEPALIVE_PRINT_LOG

 /* To enable Soft-AP RA link broken feature  */
 #define WLAN_SOFTAP_FEATURE_RALINK_BROKEN
 /* To enable Soft-AP RA link broken print log  */
 #define WLAN_SOFTAP_FEATURE_RALINK_BROKEN_PRINT_LOG
 
 /* To enable Soft-AP unknown addr2 handling feature */
 #define WLAN_SOFTAP_FEATURE_UNKNOWN_ADDR2_HANDLING
 /* To enable Soft-AP unknown addr2 handling print log */
 #define WLAN_SOFTAP_FEATURE_UNKNOWN_ADDR2_HANDLING_PRINT_LOG
 
 /* To Enable the FW processing Probe Requests Feature */
 #define WLAN_SOFTAP_FW_PROCESS_PROBE_REQ_FEATURE                      1
 /* To Trigger Dummy Probe Response Template update    */
 #define WLAN_SOFTAP_PROBE_RSP_TEMPLATE_LIM2HAL_MSG_ENA                1
  /* To Enable Host Probe Response Print Logs    */
 #define WLAN_SOFTAP_FW_PROCESS_PROBE_REQ_FEATURE_HOST_PRINT_LOG       0
 #define WLAN_SOFTAP_FEATURE_TIM_BASED_DIS_ASSOC

 #define WLAN_SOFTAP_FW_PROBE_RSP_CTRL_BLOCK_SIZE      4
 #define WLAN_SOFTAP_FW_PROBE_RSP_PAYLOAD_SIZE         476 
 #define WLAN_SOFTAP_FW_PROBE_REQ_BIT_MAP_LEN          32 //( 256 >> 3)
 #define WLAN_SOFTAP_FW_PROBE_RSP_BLOCK_SIZE           ( WLAN_SOFTAP_FW_PROBE_RSP_PAYLOAD_SIZE + WLAN_SOFTAP_FW_PROBE_REQ_BIT_MAP_LEN     )
 #define WLAN_SOFTAP_FW_PROBE_RSP_TEMPLATE_SIZE        ( WLAN_SOFTAP_FW_PROBE_RSP_BLOCK_SIZE   + WLAN_SOFTAP_FW_PROBE_RSP_CTRL_BLOCK_SIZE )
 #define WLAN_SOFTAP_FW_PROBE_REQ_BIT_MAP_LEN_IN_WORDS ( WLAN_SOFTAP_FW_PROBE_REQ_BIT_MAP_LEN  >> 2 )
 #define WLAN_SOFTAP_PROBE_RESP_PAYLOAD_SIZE_IN_WORDS  ( WLAN_SOFTAP_FW_PROBE_RSP_PAYLOAD_SIZE >> 2 )

#else
 #define WLAN_SOFTAP_FW_BEACON_TRANSMIT                                0
 #define WLAN_SOFTAP_FW_BEACON_TX_PRNT_LOG                             0
 #define WLAN_SOFTAP_FW_PROCESS_PROBE_REQ_FEATURE                      0
 #define WLAN_SOFTAP_PROBE_RSP_TEMPLATE_LIM2HAL_MSG_ENA                0
 #define WLAN_SOFTAP_FW_PROCESS_PROBE_REQ_FEATURE_HOST_PRINT_LOG       0
#endif


#define CEIL_ALIGN(value, align) \
   (((value) + (align) - 1) & ~((align) - 1))

#define ROUNDUP_ALIGN(value, align) \
   (((value) + (align)) & ~((align) - 1))

/* Shared Table for Rate Adaptation in FW */
/* Right immediately after RA shared table, It's better to place
FW starting address not to lose any single byte in SRAM.
Total shared table for RA depends on
RA_CB_ENABLED, HAL_NUM_BSSID, HAL_MAC_MAX_TX_RATES, and HAL_NUM_STA.
Currently, RA_CB_DISABLED, NUM_BSSID=2, MAX_TX_RATES=43, NUM_STA = 8
results in QWLANFW_MEMMAP_RA_SHARED_TABLE_ADDR = 0x3480.
See binary.lds.in in firmware source build tree */

#define QWLANFW_MEMMAP_RA_SHARED_TABLE_ADDR  0x2800
#define QWLANFW_MEMMAP_RA_BSS_INFO_TABLE     CEIL_ALIGN(QWLANFW_MEMMAP_RA_SHARED_TABLE_ADDR, 16)

#ifdef RA_CB_ENABLED
#define QWLANFW_MEMMAP_TPE_RATE_TABLE        CEIL_ALIGN(QWLANFW_MEMMAP_RA_BSS_INFO_TABLE+sizeof(tHalRaBssInfo)*HAL_NUM_BSSID, 4)
#define QWLANFW_MEMMAP_HAL_RATE_INFO         CEIL_ALIGN(QWLANFW_MEMMAP_TPE_RATE_TABLE+sizeof(tTpeRateTable)*HAL_MAC_MAX_TX_RATES, 4)
#else
#define QWLANFW_MEMMAP_HAL_RATE_INFO         CEIL_ALIGN(QWLANFW_MEMMAP_RA_BSS_INFO_TABLE+sizeof(tHalRaBssInfo)*HAL_NUM_BSSID, 4)
#endif
#define QWLANFW_MEMMAP_RATE_TXPWR_TABLE      CEIL_ALIGN(QWLANFW_MEMMAP_HAL_RATE_INFO+sizeof(tHalRateInfo)*HAL_MAC_MAX_TX_RATES, 4)
#define QWLANFW_MEMMAP_RA_GLOBAL_CONFIG      CEIL_ALIGN(QWLANFW_MEMMAP_RATE_TXPWR_TABLE+sizeof(tANI_U8)*HAL_MAC_MAX_TX_RATES, 4)
#define QWLANFW_MEMMAP_RA_STA_CONFIG         CEIL_ALIGN(QWLANFW_MEMMAP_RA_GLOBAL_CONFIG+sizeof(tHalRaGlobalInfo), 4)
#define QWLANFW_MEMMAP_RA_RATE_TX_COUNT      CEIL_ALIGN(QWLANFW_MEMMAP_RA_STA_CONFIG+sizeof(tHalRaInfo)*HAL_NUM_STA, 4)
#define QWLANFW_MEMMAP_RA_SHARED_TABLE_END_ADDR CEIL_ALIGN(QWLANFW_MEMMAP_RA_RATE_TX_COUNT+sizeof(tANI_U32)*HAL_MAC_MAX_TX_RATES, 4)

/*===========================================================================
  FW STATUS CODES
===========================================================================*/
#define QWLANFW_STATUS_SUCCESS                                 0x0
#define QWLANFW_STATUS_INITDONE                                0x1
#define QWLANFW_STATUS_NOT_ENOUGH_ADU_MEMORY                   0x2
#define QWLANFW_STATUS_BMPS_NO_REPLY_TO_NULLFRAME_FROM_AP      0x3
#define QWLANFW_STATUS_BMPS_NO_REPLY_TO_PSPOLLFRAME_FROM_AP    0x4
#define QWLANFW_STATUS_BMPS_RSSI_DROPPED_BELOW_THRESHOLD       0x5
#define QWLANFW_STATUS_IMPROPER_MSGLEN                         0x6
#define QWLANFW_STATUS_TOOMANY_FILTERS                         0x7
#define QWLANFW_STATUS_TOOMANY_PTRNS                           0x8
#define QWLANFW_STATUS_INCONSISTENT_FWSTATE                    0x9
#define QWLANFW_STATUS_FAILED_TOSEND_NULLDATA_FRM              0xA
#define QWLANFW_STATUS_INIT_FAILURE                            0xB
#define QWLANFW_STATUS_BMPS_ENTER_FAILED                       0xC
#define QWLANFW_STATUS_BMPS_MAX_MISSED_BEACONS                 0xD
#define QWLANFW_STATUS_REGULATE_TRAFFIC_NO_REPLY_TO_NULLFRAME  0xE
#define QWLANFW_STATUS_IMPROPER_REFERENCE_TBTT                 0xF
#define QWLANFW_STATUS_SOFTAP_REG_FLOW_CTRL_BD_UNDER_RUN      0x10


#define QWLANFW_STATUS_BMPS_BDPDU_NOT_IDLE_FAILURE          0x1001
#define QWLANFW_STATUS_BMPS_SIF_FREEZE_FAILURE              0x1002
#define QWLANFW_STATUS_BMPS_BTQM_QUEUES_NOT_EMPTY_FAILURE   0x1003
#define QWLANFW_STATUS_FW_VERSION                           0x1004

/*============================================================================
  FW LOG CODES
============================================================================*/

/* Log event type bitmasks (8 bits - 8 of these) */
#define QWLANFW_LOG_EVENT_TYPE_ALL               0xff
#define QWLANFW_LOG_EVENT_TYPE_MISC              (1<<0)
#define QWLANFW_LOG_EVENT_TYPE_MSG_HANDLING      (1<<1)
#define QWLANFW_LOG_EVENT_TYPE_INTR_HANDLING     (1<<2)
#define QWLANFW_LOG_EVENT_TYPE_TIMEOUT_HANDLING  (1<<3)
#define QWLANFW_LOG_EVENT_TYPE_JOB_HANDLING      (1<<4)
#define QWLANFW_LOG_EVENT_TYPE_INIT              (1<<5)

/* log NCODES (16 bits - 65536 of these) */

/* log codes for miscellaneous
   (QWLANFW_LOG_EVENT_TYPE_MISC)
*/
#define QWLANFW_LOG_CODE_MISC_BASE                            0x0

#define QWLANFW_LOG_CODE_NULL_DATA_FRM_SENT                   (QWLANFW_LOG_CODE_MISC_BASE + 0x1)
#define QWLANFW_LOG_CODE_PSPOLL_FRM_SENT                      (QWLANFW_LOG_CODE_MISC_BASE + 0x2)
#define QWLANFW_LOG_CODE_PM_DEFER_QUEUE_FULL                  (QWLANFW_LOG_CODE_MISC_BASE + 0x3)
#define QWLANFW_LOG_CODE_PM_FRAME_DEFERRED                    (QWLANFW_LOG_CODE_MISC_BASE + 0x4)

#define QWLANFW_LOG_CODE_PARSE_BEACON                         (QWLANFW_LOG_CODE_MISC_BASE + 0x10)
#define QWLANFW_LOG_CODE_PARSE_BEACON_INTERVAL_CHANGED        (QWLANFW_LOG_CODE_MISC_BASE + 0x11)
#define QWLANFW_LOG_CODE_PARSE_BEACON_CAP_INFO_CHANGED        (QWLANFW_LOG_CODE_MISC_BASE + 0x12)
#define QWLANFW_LOG_CODE_MATCH_IE_EID_CHANGED                 (QWLANFW_LOG_CODE_MISC_BASE + 0x13)
#define QWLANFW_LOG_CODE_MATCH_PTRN_ID                        (QWLANFW_LOG_CODE_MISC_BASE + 0x14)
#define QWLANFW_LOG_CODE_BACKUP_REGISTERS                     (QWLANFW_LOG_CODE_MISC_BASE + 0x15)
#define QWLANFW_LOG_CODE_TIME_TO_NEXT_TBTT_NEXT_GT_CUR        (QWLANFW_LOG_CODE_MISC_BASE + 0x16)
#define QWLANFW_LOG_CODE_TIME_TO_NEXT_TBTT_NEXT_LT_CUR        (QWLANFW_LOG_CODE_MISC_BASE + 0x17)
#define QWLANFW_LOG_CODE_TIME_TO_NEXT_TBTT_REM_GT_LI          (QWLANFW_LOG_CODE_MISC_BASE + 0x18)
#define QWLANFW_LOG_CODE_BEACON_HIST_PTBTTLO_FTBTTLO          (QWLANFW_LOG_CODE_MISC_BASE + 0x19)
#define QWLANFW_LOG_CODE_BEACON_HIST_PTBTT_HILO_FTBTT_HILO    (QWLANFW_LOG_CODE_MISC_BASE + 0x1a)
#define QWLANFW_LOG_CODE_BEACON_HIST_NUM_BEACONS_MISSED       (QWLANFW_LOG_CODE_MISC_BASE + 0x1b)
#define QWLANFW_LOG_CODE_REGULATE_TRAFFIC_IN_INVALID_STATE    (QWLANFW_LOG_CODE_MISC_BASE + 0x1c)

#define QWLANFW_LOG_CODE_SET_CHANNEL_BOND_STATE               (QWLANFW_LOG_CODE_MISC_BASE + 0x20)
#define QWLANFW_LOG_CODE_SET_CHANNEL_NUM                      (QWLANFW_LOG_CODE_MISC_BASE + 0x21)
#define QWLANFW_LOG_CODE_ASSESS_CAL_MEASURING_RADIO_TEMP      (QWLANFW_LOG_CODE_MISC_BASE + 0x22)
#define QWLANFW_LOG_CODE_ASSESS_CAL_SEND_PERIODIC_CAL_MSG     (QWLANFW_LOG_CODE_MISC_BASE + 0x23)
#define QWLANFW_LOG_CODE_PHY_ASIC_SET_FREQ                    (QWLANFW_LOG_CODE_MISC_BASE + 0x24)
#define QWLANFW_LOG_CODE_ASIC_AGC_CALC_GAIN_LUTS_FOR_CHAN     (QWLANFW_LOG_CODE_MISC_BASE + 0x25)
#define QWLANFW_LOG_CODE_ASIC_PERF_CAL_MEAS_DID_NOT_FINISH    (QWLANFW_LOG_CODE_MISC_BASE + 0x26)
#define QWLANFW_LOG_CODE_PHY_CARRIER_SUPPRESS_TX_CHAIN        (QWLANFW_LOG_CODE_MISC_BASE + 0x27)
#define QWLANFW_LOG_CODE_PHY_TX_IQ_CAL                        (QWLANFW_LOG_CODE_MISC_BASE + 0x28)
#define QWLANFW_LOG_CODE_PHY_RX_IQ_CAL_NEW_CORR               (QWLANFW_LOG_CODE_MISC_BASE + 0x29)
#define QWLANFW_LOG_CODE_PHY_RX_IQ_CAL_RX_IQ_CORR             (QWLANFW_LOG_CODE_MISC_BASE + 0x2a)
#define QWLANFW_LOG_CODE_PHY_DCO_CAL_RX_CHAIN                 (QWLANFW_LOG_CODE_MISC_BASE + 0x2b)
#define QWLANFW_LOG_CODE_PHY_DCO_CAL_FAILED_TO_CONVERGE       (QWLANFW_LOG_CODE_MISC_BASE + 0x2c)
#define QWLANFW_LOG_CODE_CHAN_TUNE_TIMER_HANDLER              (QWLANFW_LOG_CODE_MISC_BASE + 0x2d)
#define QWLANFW_LOG_CODE_PHY_CARRRIER_SUPPRESS_TX_CHAIN       (QWLANFW_LOG_CODE_MISC_BASE + 0x2e)
#define QWLANFW_LOG_CODE_ASIC_GET_CAL_ADC_SAMP_DID_NOT_FINISH (QWLANFW_LOG_CODE_MISC_BASE + 0x2f)

#define QWLANFW_LOG_CODE_PHY_BIN_DCO_CAL_RX_CHAIN_IQ_VALS     (QWLANFW_LOG_CODE_MISC_BASE + 0x30)
#define QWLANFW_LOG_CODE_DCO_CAL_FAILED_TO_CONVERGE           (QWLANFW_LOG_CODE_MISC_BASE + 0x31)
#define QWLANFW_LOG_CODE_DCO_CAL_LOADING_MIN_ERROR_VALUES     (QWLANFW_LOG_CODE_MISC_BASE + 0x32)
#define QWLANFW_LOG_CODE_RF_SET_CUR_CHAN_FIRST_CONF           (QWLANFW_LOG_CODE_MISC_BASE + 0x33)
#define QWLANFW_LOG_CODE_RF_SET_CUR_CHAN_WARMING_UP           (QWLANFW_LOG_CODE_MISC_BASE + 0x34)
#define QWLANFW_LOG_CODE_RF_SET_CUR_CHAN_APP_CACHED_VCOBANK   (QWLANFW_LOG_CODE_MISC_BASE + 0x35)
#define QWLANFW_LOG_CODE_RF_SET_CUR_CHAN_SAVE_VCOBANK         (QWLANFW_LOG_CODE_MISC_BASE + 0x36)
#define QWLANFW_LOG_CODE_RSSI_MONITORING                      (QWLANFW_LOG_CODE_MISC_BASE + 0x37)
#define QWLANFW_LOG_CODE_FAST_BPS                             (QWLANFW_LOG_CODE_MISC_BASE + 0x38)

/* log codes for message handling
   (QWLANFW_LOG_EVENT_TYPE_MSG_HANDLING)
*/
#define QWLANFW_LOG_CODE_MSG_BASE                           0x100

#define QWLANFW_LOG_CODE_MBOX1_FW_POSTED_MSG_TO_HOST        (QWLANFW_LOG_CODE_MSG_BASE + 0x1)
#define QWLANFW_LOG_CODE_MBOX0_HOST_POSTED_MSG_TO_FW        (QWLANFW_LOG_CODE_MSG_BASE + 0x2)

#define QWLANFW_LOG_CODE_ENTER_IMPS_MSG_HANDLER             (QWLANFW_LOG_CODE_MSG_BASE + 0x10)
#define QWLANFW_LOG_CODE_ENTER_BMPS_MSG_HANDLER             (QWLANFW_LOG_CODE_MSG_BASE + 0x11)
#define QWLANFW_LOG_CODE_ENTER_BMPS_MSG_TIMESTAMP           (QWLANFW_LOG_CODE_MSG_BASE + 0x12)
#define QWLANFW_LOG_CODE_EXIT_BMPS_MSG_HANDLER              (QWLANFW_LOG_CODE_MSG_BASE + 0x13)
#define QWLANFW_LOG_CODE_SUSPEND_BMPS_MSG_HANDLER           (QWLANFW_LOG_CODE_MSG_BASE + 0x14)
#define QWLANFW_LOG_CODE_RESUME_BMPS_MSG_HANDLER            (QWLANFW_LOG_CODE_MSG_BASE + 0x15)
#define QWLANFW_LOG_CODE_ENTER_UAPSD_MSG_HANDLER            (QWLANFW_LOG_CODE_MSG_BASE + 0x16)
#define QWLANFW_LOG_CODE_ENTER_UAPSD_MSG_IN_INVALID_STATE   (QWLANFW_LOG_CODE_MSG_BASE + 0x17)
#define QWLANFW_LOG_CODE_EXIT_UAPSD_MSG_HANDLER             (QWLANFW_LOG_CODE_MSG_BASE + 0x18)
#define QWLANFW_LOG_CODE_EXIT_UAPSD_MSG_IN_INVALID_STATE    (QWLANFW_LOG_CODE_MSG_BASE + 0x19)
#define QWLANFW_LOG_CODE_ADD_BCN_FILTER_MSG_HANDLER         (QWLANFW_LOG_CODE_MSG_BASE + 0x1a)
#define QWLANFW_LOG_CODE_ADD_MATCH_PTRN_MSG_HANDLER         (QWLANFW_LOG_CODE_MSG_BASE + 0x1b)
#define QWLANFW_LOG_CODE_REM_MATCH_PTRN_MSG_HANDLER         (QWLANFW_LOG_CODE_MSG_BASE + 0x1c)

#define QWLANFW_LOG_CODE_CAL_UPDATE_MSG_HANDLER             (QWLANFW_LOG_CODE_MSG_BASE + 0x20)
#define QWLANFW_LOG_CODE_SET_CHANNEL_MSG_HANDLER            (QWLANFW_LOG_CODE_MSG_BASE + 0x21)
#define QWLANFW_LOG_CODE_SET_CHAIN_SELECT_MSG_HANDLER       (QWLANFW_LOG_CODE_MSG_BASE + 0x22)

#define QWLANFW_LOG_CODE_BT_EVENT_HANDLER                   (QWLANFW_LOG_CODE_MSG_BASE + 0x30)
#define QWLANFW_LOG_CODE_BT_EVENT_HANDLER_NEW_MODE          (QWLANFW_LOG_CODE_MSG_BASE + 0x31)
#define QWLANFW_LOG_CODE_WLAN_EVENT_HANDLER                 (QWLANFW_LOG_CODE_MSG_BASE + 0x32)
#define QWLANFW_LOG_CODE_WLAN_EVENT_HANDLER_NEW_MODE        (QWLANFW_LOG_CODE_MSG_BASE + 0x33)
#define QWLANFW_LOG_CODE_RA_UPDATE_MSG_HANDLER              (QWLANFW_LOG_CODE_MSG_BASE + 0x34)

#define QWLANFW_LOG_CODE_BT_ACL_CREATE_CONN                 (QWLANFW_LOG_CODE_MSG_BASE + 0x40)
#define QWLANFW_LOG_CODE_BT_ACL_CONN_COMPLETE               (QWLANFW_LOG_CODE_MSG_BASE + 0x41)
#define QWLANFW_LOG_CODE_BT_SCO_CREATE_CONN                 (QWLANFW_LOG_CODE_MSG_BASE + 0x42)
#define QWLANFW_LOG_CODE_BT_SCO_CONN_COMPLETE               (QWLANFW_LOG_CODE_MSG_BASE + 0x43)
#define QWLANFW_LOG_CODE_BT_SCO_CONN_UPDATE                 (QWLANFW_LOG_CODE_MSG_BASE + 0x44)
#define QWLANFW_LOG_CODE_BT_DISCONNECT                      (QWLANFW_LOG_CODE_MSG_BASE + 0x45)
#define QWLANFW_LOG_CODE_BT_ACL_CONN_UPDATE                 (QWLANFW_LOG_CODE_MSG_BASE + 0x46)
#ifdef LIBRA_WAPI_SUPPORT
#define QWLANFW_LOG_CODE_WAPI_SET_KEY                       (QWLANFW_LOG_CODE_MSG_BASE + 0x50)
#define QWLANFW_LOG_CODE_WAPI_REMOVE_KEY                    (QWLANFW_LOG_CODE_MSG_BASE + 0x51)
#endif /* LIBRA_WAPI_SUPPORT */


/* log codes for interrupt handling
   (QWLANFW_LOG_EVENT_TYPE_INTR_HANDLING)
*/
#define QWLANFW_LOG_CODE_INTR_BASE                          0x200

#define QWLANFW_LOG_CODE_H2F_MBOX_INTR_MSG_AT_WRONG_ADDR    (QWLANFW_LOG_CODE_INTR_BASE + 0x1)
#define QWLANFW_LOG_CODE_H2F_MBOX_INTR_HOST_RX_ACK_FROM_FW  (QWLANFW_LOG_CODE_INTR_BASE + 0x2)
#define QWLANFW_LOG_CODE_F2H_MBOX_INTR_FW_RX_ACK_FROM_HOST  (QWLANFW_LOG_CODE_INTR_BASE + 0x3)
#define QWLANFW_LOG_CODE_F2H_MBOX_INTR_FAKE_HOST_PROCESSING (QWLANFW_LOG_CODE_INTR_BASE + 0x4)
#define QWLANFW_LOG_CODE_F2H_MBOX_INTR_MSG_AT_WRONG_ADDR    (QWLANFW_LOG_CODE_INTR_BASE + 0x5)
#define QWLANFW_LOG_CODE_F2H_MBOX_INTR_MSG_DUMP             (QWLANFW_LOG_CODE_INTR_BASE + 0x6)
#define QWLANFW_LOG_CODE_F2H_MBOX_INTR_MSG_DUMP_STATUS      (QWLANFW_LOG_CODE_INTR_BASE + 0x7)
#define QWLANFW_LOG_CODE_PMU_PWR_UP_INTR_HANDLER            (QWLANFW_LOG_CODE_INTR_BASE + 0x8)
#define QWLANFW_LOG_CODE_PMU_PWR_UP_INTR_IN_INVALID_STATE   (QWLANFW_LOG_CODE_INTR_BASE + 0x9)
#define QWLANFW_LOG_CODE_BPS_DISABLED_SIF_UNFREEZE          (QWLANFW_LOG_CODE_INTR_BASE + 0xa)
#define QWLANFW_LOG_CODE_BPS_DISABLED                       (QWLANFW_LOG_CODE_INTR_BASE + 0xb)
#define QWLANFW_LOG_CODE_BPS_DISABLED_PTSF                  (QWLANFW_LOG_CODE_INTR_BASE + 0xc)
#define QWLANFW_LOG_CODE_BPS_DISABLED_BL_NL                 (QWLANFW_LOG_CODE_INTR_BASE + 0xd)
#define QWLANFW_LOG_CODE_RXP_INTR_HANDLER                   (QWLANFW_LOG_CODE_INTR_BASE + 0xe)
#define QWLANFW_LOG_CODE_RXP_INTR_USTATUS_NOT_READSTATUS    (QWLANFW_LOG_CODE_INTR_BASE + 0xf)
#define QWLANFW_LOG_CODE_FW_RX_WQ_INTR_HANDLER              (QWLANFW_LOG_CODE_INTR_BASE + 0x10)
#define QWLANFW_LOG_CODE_FW_RX_WQ_INTR_IN_INVALID_STATE     (QWLANFW_LOG_CODE_INTR_BASE + 0x11)
#define QWLANFW_LOG_CODE_FW_RX_WQ_INTR_FRM_EXCEEDS_MAX_LEN  (QWLANFW_LOG_CODE_INTR_BASE + 0x12)
#define QWLANFW_LOG_CODE_FW_RX_WQ_INTR_UNKNOWN_TYPE_SUBTYPE (QWLANFW_LOG_CODE_INTR_BASE + 0x13)
#define QWLANFW_LOG_CODE_BMU_WQ3_INTR_HANDLER               (QWLANFW_LOG_CODE_INTR_BASE + 0x14)
#define QWLANFW_LOG_CODE_COMBINED_INTR_HANDLER              (QWLANFW_LOG_CODE_INTR_BASE + 0x15)
#define QWLANFW_LOG_CODE_COMBINED_INTR_SEND_NULL_FRM_FAIL   (QWLANFW_LOG_CODE_INTR_BASE + 0x16)
#define QWLANFW_LOG_CODE_COMBINED_INTR_FEEDBACK             (QWLANFW_LOG_CODE_INTR_BASE + 0x17)
#define QWLANFW_LOG_CODE_FW_TX_WQ_INTR_HANDLER              (QWLANFW_LOG_CODE_INTR_BASE + 0x18)
#define QWLANFW_LOG_CODE_BPS_DISABLED_SW_INTR               (QWLANFW_LOG_CODE_INTR_BASE + 0x19)
#define QWLANFW_LOG_CODE_BPS_DISABLED_BCN_MISS              (QWLANFW_LOG_CODE_INTR_BASE + 0x1a)
#define QWLANFW_LOG_CODE_TBTT_ADJUST_LEFT_ADJUST            (QWLANFW_LOG_CODE_INTR_BASE + 0x1b)
#define QWLANFW_LOG_CODE_TBTT_RIGHT_ADJUST                  (QWLANFW_LOG_CODE_INTR_BASE + 0x1c)
#define QWLANFW_LOG_CODE_TBTT_LEFT_ADJUST                   (QWLANFW_LOG_CODE_INTR_BASE + 0x1d)
#define QWLANFW_LOG_CODE_TBTT_ADJUST_ERROR                  (QWLANFW_LOG_CODE_INTR_BASE + 0x1e)

/* log codes for timeout handling
   (QWLANFW_LOG_EVENT_TYPE_TIMEOUT_HANDLING)
*/
#define QWLANFW_LOG_CODE_TIMEOUT_BASE                       0x300
#define QWLANFW_LOG_CODE_PMU_SLEEP_TIMEOUT_HANDLER          (QWLANFW_LOG_CODE_TIMEOUT_BASE + 0x1)
#define QWLANFW_LOG_CODE_BCAST_DATA_RECEP_TIMEOUT_HANDLER   (QWLANFW_LOG_CODE_TIMEOUT_BASE + 0x2)
#define QWLANFW_LOG_CODE_UCAST_DATA_RECEP_TIMEOUT_HANDLER   (QWLANFW_LOG_CODE_TIMEOUT_BASE + 0x3)
#define QWLANFW_LOG_CODE_SIFUNFREEZE_TIMEOUT_HANDLER        (QWLANFW_LOG_CODE_TIMEOUT_BASE + 0x4)
#define QWLANFW_LOG_CODE_BEACON_RX_TIMEOUT_HANDLER          (QWLANFW_LOG_CODE_TIMEOUT_BASE + 0x5)

/* log codes for job handling
   (QWLANFW_LOG_EVENT_TYPE_JOB_HANDLING)
*/
#define QWLANFW_LOG_CODE_JOB_BASE                           0x400

#define QWLANFW_LOG_CODE_PROCESS_JOB                        (QWLANFW_LOG_CODE_JOB_BASE + 0x1)
#define QWLANFW_LOG_CODE_PROCESS_JOB_ERROR                  (QWLANFW_LOG_CODE_JOB_BASE + 0x2)
#define QWLANFW_LOG_CODE_POST_MBOX_MSG_SEND_FAIL            (QWLANFW_LOG_CODE_JOB_BASE + 0x3)

#define QWLANFW_LOG_CODE_IMPS_PWR_DOWN_MBOX_MSG_SEND_FAIL   (QWLANFW_LOG_CODE_JOB_BASE + 0x10)
#define QWLANFW_LOG_CODE_BMPS_PWR_DOWN_HANDLER              (QWLANFW_LOG_CODE_JOB_BASE + 0x11)
#define QWLANFW_LOG_CODE_BMPS_PWR_DOWN_MBOX_MSG_SEND_FAIL   (QWLANFW_LOG_CODE_JOB_BASE + 0x12)
#define QWLANFW_LOG_CODE_BMPS_PWR_DOWN_RECEIVED_ACK_FROM_AP (QWLANFW_LOG_CODE_JOB_BASE + 0x13)
#define QWLANFW_LOG_CODE_BMPS_PWR_DOWN_LISTEN_INTERVAL      (QWLANFW_LOG_CODE_JOB_BASE + 0x14)
#define QWLANFW_LOG_CODE_BMPS_PWR_DOWN_CALIBRATION_RESULT   (QWLANFW_LOG_CODE_JOB_BASE + 0x15)
#define QWLANFW_LOG_CODE_BMPS_PWR_DOWN_NEXT_WAKEUP_COUNTER  (QWLANFW_LOG_CODE_JOB_BASE + 0x16)
#define QWLANFW_LOG_CODE_BMPS_PWR_DOWN_ALL_BD_NOT_IDLE      (QWLANFW_LOG_CODE_JOB_BASE + 0x17)
#define QWLANFW_LOG_CODE_BMPS_PWR_DOWN_HOST_INTERRUPTS      (QWLANFW_LOG_CODE_JOB_BASE + 0x18)
#define QWLANFW_LOG_CODE_BMPS_PWR_DOWN_SIF_STOP_FAILURE     (QWLANFW_LOG_CODE_JOB_BASE + 0x19)
#define QWLANFW_LOG_CODE_BMPS_PWR_DOWN_HOST_ACQUIRED_MUTEX  (QWLANFW_LOG_CODE_JOB_BASE + 0x1a)
#define QWLANFW_LOG_CODE_BMPS_PWR_DOWN_ENTER_BPS_SLEEP      (QWLANFW_LOG_CODE_JOB_BASE + 0x1b)
#define QWLANFW_LOG_CODE_BMPS_PWR_DOWN_TIME_ELASPED         (QWLANFW_LOG_CODE_JOB_BASE + 0x1c)
#define QWLANFW_LOG_CODE_BMPS_PWR_DOWN_ERR_SIF_FREEZE       (QWLANFW_LOG_CODE_JOB_BASE + 0x1d)
#define QWLANFW_LOG_CODE_BMPS_PWR_DOWN_ENTER_BMPS_FAILED    (QWLANFW_LOG_CODE_JOB_BASE + 0x1e)
#define QWLANFW_LOG_CODE_BMPS_PWR_DOWN_ENTER_BMPS_CANCELLED (QWLANFW_LOG_CODE_JOB_BASE + 0x1f)

#define QWLANFW_LOG_CODE_BMPS_PWR_DOWN_DL_DH                (QWLANFW_LOG_CODE_JOB_BASE + 0x20)
#define QWLANFW_LOG_CODE_BMPS_PWR_DOWN_TIMEOUT_VAL          (QWLANFW_LOG_CODE_JOB_BASE + 0x21)
#define QWLANFW_LOG_CODE_BMPS_PWR_DOWN_DPU_DXE_CNTRS        (QWLANFW_LOG_CODE_JOB_BASE + 0x22)
#define QWLANFW_LOG_CODE_BMPS_PWR_DOWN_PWR_DOWN_POSTPONED   (QWLANFW_LOG_CODE_JOB_BASE + 0x23)
#define QWLANFW_LOG_CODE_BMPS_PWR_DOWN_BMPS_CANCELLED       (QWLANFW_LOG_CODE_JOB_BASE + 0x24)
#define QWLANFW_LOG_CODE_ENTER_BMPS_STATE_CHANGE_TIMEOUT    (QWLANFW_LOG_CODE_JOB_BASE + 0x25)
#define QWLANFW_LOG_CODE_EXIT_BMPS_STATE_CHANGE_TIMEOUT     (QWLANFW_LOG_CODE_JOB_BASE + 0x26)
#define QWLANFW_LOG_CODE_SUSP_BMPS_STATE_CHANGE_TIMEOUT     (QWLANFW_LOG_CODE_JOB_BASE + 0x27)
#define QWLANFW_LOG_CODE_BMPS_PWR_DOWN_BCN_MISS             (QWLANFW_LOG_CODE_JOB_BASE + 0x28)
#define QWLANFW_LOG_CODE_BPS_MODE_TIMEOUT_ERR_DIFF_VAL      (QWLANFW_LOG_CODE_JOB_BASE + 0x29)
#define QWLANFW_LOG_CODE_ERR_RPE_FLUSH                      (QWLANFW_LOG_CODE_JOB_BASE + 0x2a)
#define QWLANFW_LOG_CODE_BMPS_PWR_DOWN_ERR_HOST_INTR        (QWLANFW_LOG_CODE_JOB_BASE + 0x2b)
#define QWLANFW_LOG_CODE_BTC_FAILED_PM_TRANSITION           (QWLANFW_LOG_CODE_JOB_BASE + 0x2c)

/* log codes for init
   (QWLANFW_LOG_EVENT_TYPE_INIT)
*/
#define QWLANFW_LOG_CODE_INIT_BASE                          0x500
#define QWLANFW_LOG_CODE_MACFW_VER_MAJOR_MINOR_PATCH_BUILD  (QWLANFW_LOG_CODE_INIT_BASE + 0x1)
#define QWLANFW_LOG_CODE_START_TSF_DEC_TIMER                (QWLANFW_LOG_CODE_INIT_BASE + 0x2)
#define QWLANFW_LOG_CODE_PHY_ASIC_INIT                      (QWLANFW_LOG_CODE_INIT_BASE + 0x3)
#define QWLANFW_LOG_CODE_RF_INIT                            (QWLANFW_LOG_CODE_INIT_BASE + 0x4)

/* log codes for RA (rate adaptation)
*/
#define QWLANFW_LOG_CODE_RA_BASE                            0x600
#define QWLANFW_LOG_CODE_RA_RECORD                          (QWLANFW_LOG_CODE_RA_BASE + 0x1)
#define QWLANFW_LOG_CODE_RA_ADD_BSS                         (QWLANFW_LOG_CODE_RA_BASE + 0x2)
#define QWLANFW_LOG_CODE_RA_DEL_BSS                         (QWLANFW_LOG_CODE_RA_BASE + 0x3)
#define QWLANFW_LOG_CODE_RA_ADD_STA                         (QWLANFW_LOG_CODE_RA_BASE + 0x4)
#define QWLANFW_LOG_CODE_RA_DEL_STA                         (QWLANFW_LOG_CODE_RA_BASE + 0x5)
#define QWLANFW_LOG_CODE_RA_STATE_TRANSIT                   (QWLANFW_LOG_CODE_RA_BASE + 0x6)
#define QWLANFW_LOG_CODE_RA_LOG_PER                         (QWLANFW_LOG_CODE_RA_BASE + 0x7)
#define QWLANFW_LOG_CODE_RA_ERROR_CODE                      (QWLANFW_LOG_CODE_RA_BASE + 0x8)
#define QWLANFW_LOG_CODE_RA_WARN_CODE                       (QWLANFW_LOG_CODE_RA_BASE + 0x9)

/*==========================================================================
  LOG RECORD DESC
==========================================================================*/

/* This needs to be larger than WLANFW_MODULE_NUM from wlanfwdefs.h

   It's nice for this to be larger than the actual number of modules
   so we can grow the number of modules without changing the layout
   of the log descriptor.  This is desirable because Quarky will
   decode the log descriptor and may not be built as often as the
   driver.
*/
#define COREX_LOG_NUM_FILTERS 8

typedef struct {
   tANI_U8 nLogLevel;
   tANI_U8 nEventTypeMask;
} CorexLog_EventFilterType;

/* overhead taken from log space for management structures includes:
   overhead from nHeadIndex
   overhead from nTailIndex
   overhead from Event Filter structure
*/
#define COREX_LOG_OVERHEAD \
  (sizeof(tANI_U32) \
   + sizeof(tANI_U32) \
   + sizeof(tANI_U32) \
   + (sizeof(CorexLog_EventFilterType) * COREX_LOG_NUM_FILTERS))

#define FEATURE_WLANFW_COREX_LOG_BUFFER_ENTRIES \
   ((FEATURE_WLANFW_COREX_LOG_BUFFER_SIZE - COREX_LOG_OVERHEAD) / sizeof(tANI_U32))

typedef struct {
   volatile tANI_U32 nHaltLogging;
   volatile tANI_U32 nHeadIndex;
   volatile tANI_U32 nTailIndex;
   CorexLog_EventFilterType sEventFilter[COREX_LOG_NUM_FILTERS];
   tANI_U32 aBuffer[FEATURE_WLANFW_COREX_LOG_BUFFER_ENTRIES];
} CorexLog_LogDescType;

typedef struct {
   tANI_U8  nNumOfWords;
   tANI_U8  nModuleIndex;
   tANI_U16 nCode;
   tANI_U32 nTimestamp;
} CorexLog_EntryType;

/* size of entries in Log Buffer in words */
/* divide by size of entries in CorexLog_LogDescType.aBuffer */
#define FEATURE_WLANFW_COREX_LOG_BUFFER_ENTRY_WORD_SIZE \
   (sizeof(CorexLog_EntryType)/sizeof(tANI_U32))

typedef PACKED_PRE struct PACKED_POST _UapsdAcParamType {
#ifdef ANI_BIG_BYTE_ENDIAN
   tANI_U32  usSrvIntrMs   : 16;
   tANI_U32  ucUp          : 8;
   tANI_U32  bReserved     : 8;
#else
   tANI_U32  bReserved     : 8;
   tANI_U32  ucUp          : 8;
   tANI_U32  usSrvIntrMs   : 16;
#endif

   tANI_U32  uSuspIntrMs;
   tANI_U32  uDelayIntrMs;

} UapsdAcParamType;

/*==========================================================================
  STATUS MESSAGE INFO STRUCT
==========================================================================*/
typedef struct {
  tANI_U32 uStatusCode;
  tANI_U32 aStatusInfo[8];
} StatusMsgInfo;

typedef struct {
  tANI_U32 uIdleBds;
} BdPduNotIdleInfo;

typedef struct {
  tANI_U32 uMacIntr;
  tANI_U32 uSifRxFifoData;
} SifFreezeInfo;

typedef struct {
  tANI_U32 uMj;
  tANI_U32 uMn;
  tANI_U32 uPatch;
  tANI_U32 uBuild;
} FwVersionInfo;
/*===========================================================================
  MAILBOX MESSAGE HEADER
===========================================================================*/
typedef  PACKED_PRE struct PACKED_POST _Qwlanfw_CtrlMsgStruct
{
   /*Version Information*/
#ifdef ANI_BIG_BYTE_ENDIAN
   tANI_U32  usVer : 6;
   tANI_U32  usMsgType : 10;
   tANI_U32  usMsgLen : 16;
#else
   tANI_U32  usMsgLen  : 16;
   tANI_U32  usMsgType : 10;
   tANI_U32  usVer     : 6;
#endif

#ifdef ANI_BIG_BYTE_ENDIAN
   tANI_U32  msgSerialNum :16;
   /* The below fields are not supported
    * and shall not be used in Host
    */
   tANI_U32  senderID : 15;
   tANI_U32  bRespNeeded : 1;
#else
   tANI_U32  bRespNeeded  : 1;
   tANI_U32  senderID     : 15;
   tANI_U32  msgSerialNum : 16;
#endif
   tANI_U32  callback0;
   tANI_U32  callback1;

} Qwlanfw_CtrlMsgType;

/*===========================================================================
  SYSTEM CONFIGURATION
===========================================================================*/
typedef  PACKED_PRE struct PACKED_POST _Qwlanfw_SysCfgStruct
{
   /*Registers Backup*/
   tANI_U32  uAduReinitAddress;
   tANI_U32  uRegWriteCount;

#ifdef ANI_BIG_BYTE_ENDIAN
   /*BMPS */
   tANI_U32   ucMaxPsPoll           : 8;
   tANI_U32   usPmuSleepTimeoutMsec : 16;
   tANI_U32   ucDtimPeriod          : 8;
#else
   tANI_U32   ucDtimPeriod          : 8;
   tANI_U32   usPmuSleepTimeoutMsec : 16;
   tANI_U32   ucMaxPsPoll           : 8;
#endif

#ifdef ANI_BIG_BYTE_ENDIAN
   tANI_U32   ucListenInterval     : 16;
   tANI_U32   ucMaxFrmRetries      : 8;
   /*Beacon Filtering*/
   tANI_U32   ucBeaconFilterPeriod : 8;
#else
   /*Beacon Filtering*/
   tANI_U32   ucBeaconFilterPeriod : 8;
   tANI_U32   ucMaxFrmRetries      : 8;
   tANI_U32   ucListenInterval     : 16;
#endif

#ifdef ANI_BIG_BYTE_ENDIAN
   tANI_U32   ucStaAid                : 8;
   /*Network Filtering*/
   tANI_U32   ucNetWakeupFilterPeriod : 8;
   /*Rssi Filtering*/
   tANI_U32   ucRssiFilterPeriod      : 8;
   tANI_U32   ucNumBeaconRssiAvg      : 8;
#else
   tANI_U32   ucNumBeaconRssiAvg      : 8;
   /*Rssi Filtering*/
   tANI_U32   ucRssiFilterPeriod      : 8;
   /*Network Filtering*/
   tANI_U32   ucNetWakeupFilterPeriod : 8;
   tANI_U32   ucStaAid                : 8;
#endif

#ifdef ANI_BIG_BYTE_ENDIAN
   /*HalPhy related*/
   tANI_U32   ucNumTxAntennas : 8;
   tANI_U32   ucNumRxAntennas : 8;
   tANI_U32   bPeriodicChanTune : 1;
   tANI_U32   bClosedLoop : 1;
   tANI_U32   ucOpenLoopTxGain : 8;
   tANI_U32   bReserved1        : 6;
#else
   tANI_U32   bReserved1        : 6;
   tANI_U32   ucOpenLoopTxGain : 8;
   tANI_U32   bClosedLoop : 1;
   tANI_U32   bPeriodicChanTune : 1;
   tANI_U32   ucNumRxAntennas : 8;
   tANI_U32   ucNumTxAntennas : 8;
#endif

   /*Mac Addresses*/
   tANI_U32   staMacAddrLo;
#ifdef ANI_BIG_BYTE_ENDIAN
   tANI_U32   staMacAddrHi : 16;
   tANI_U32   ucBeaconIntervalMsec : 16;
#else
   tANI_U32   ucBeaconIntervalMsec : 16;
   tANI_U32   staMacAddrHi : 16;
#endif

   tANI_U32   apMacAddrLo;
#ifdef ANI_BIG_BYTE_ENDIAN
   tANI_U32   apMacAddrHi : 16;
   tANI_U32   ucRfSupplySettlingTimeClk : 16;
#else
   tANI_U32   ucRfSupplySettlingTimeClk : 16;
   tANI_U32   apMacAddrHi : 16;
#endif

#ifdef ANI_BIG_BYTE_ENDIAN
   tANI_U32   staIdx          : 8;
   tANI_U32   ucMaxMissBeacon : 8;
   tANI_U32   ucMaxBcnWaitTU  : 8;
   tANI_U32   peerStaIdx      : 8;
#else
   tANI_U32   peerStaIdx      : 8;
   tANI_U32   ucMaxBcnWaitTU  : 8;
   tANI_U32   ucMaxMissBeacon : 8;
   tANI_U32   staIdx          : 8;
#endif

#ifdef ANI_BIG_BYTE_ENDIAN
   /*Feature En/Disable*/
   tANI_U32  bMagicPacketEnabled     : 1;
   tANI_U32  bUcastFrmFilterEnable   : 1;
   tANI_U32  bBeaconFilterEnable     : 1;
   tANI_U32  bNetWakeupFilterEnable  : 1;
   tANI_U32  bMagicPacketFilterEnable: 1;
   tANI_U32  bRssiFilterEnable       : 1;
   tANI_U32  bMinRssiAvgAlg          : 1;
   tANI_U32  bRegulateTraffic        : 1;
   tANI_U32  bRfXoOn                 : 1;
   tANI_U32  bMutexProtectionEnable  : 1;
   tANI_U32  bRefTbttAdjustment      : 1;
   tANI_U32  bBcnMissHandling        : 1;
   tANI_U32  bBcnMissFastPath        : 1;
   tANI_U32  bBcnMissMLC             : 1;
   tANI_U32  bTimBasedDisAssocEna    : 1;
   tANI_U32  bReserved5              : 17;
#else
   tANI_U32  bReserved5              : 17;
   tANI_U32  bTimBasedDisAssocEna    : 1;
   tANI_U32  bBcnMissMLC             : 1;
   tANI_U32  bBcnMissFastPath        : 1;
   tANI_U32  bBcnMissHandling        : 1;
   tANI_U32  bRefTbttAdjustment      : 1;
   tANI_U32  bMutexProtectionEnable  : 1;
   tANI_U32  bRfXoOn                 : 1;
   tANI_U32  bRegulateTraffic        : 1;
   tANI_U32  bMinRssiAvgAlg          : 1;
   tANI_U32  bRssiFilterEnable       : 1;
   tANI_U32  bMagicPacketFilterEnable: 1;
   tANI_U32  bNetWakeupFilterEnable  : 1;
   /*Feature En/Disable*/
   tANI_U32  bBeaconFilterEnable     : 1;
   tANI_U32  bUcastFrmFilterEnable   : 1;
   tANI_U32  bMagicPacketEnabled     : 1;
#endif

#ifdef ANI_BIG_BYTE_ENDIAN
   /* The following flags are used for
    * debugging purposes. All the debug
    * related flags are in this bitmap
    */
   tANI_U32  bNoPwrDown              : 1;
   tANI_U32  bReserved6              : 31;
#else
   tANI_U32  bReserved6              : 31;
   tANI_U32  bNoPwrDown              : 1;
#endif

   /*PS-Poll SW template parameters*/
#ifdef ANI_BIG_BYTE_ENDIAN
   tANI_U32   ucRateIndex           : 9;
   tANI_U32   ucTxPower             : 5;
   tANI_U32   uBcnMissGracePeriodUs : 16;
   tANI_U32   bReserved7            : 2;
#else
   tANI_U32   bReserved7            : 2;
   tANI_U32   uBcnMissGracePeriodUs : 16;
   tANI_U32   ucTxPower             : 5;
   tANI_U32   ucRateIndex           : 9;
#endif

#ifdef ANI_BIG_BYTE_ENDIAN
   tANI_U32   ucBcastDataRecepTimeoutMs      : 8;
   tANI_U32   ucUcastDataRecepTimeoutMs      : 8;
   tANI_U32   ucMaxSifUnfreezeTimeoutMs      : 8;
   tANI_U32   ucBtqmQueuesEmptyTimeoutMs     : 8;
#else
   tANI_U32   ucBtqmQueuesEmptyTimeoutMs     : 8;
   tANI_U32   ucMaxSifUnfreezeTimeoutMs      : 8;
   tANI_U32   ucUcastDataRecepTimeoutMs      : 8;
   tANI_U32   ucBcastDataRecepTimeoutMs      : 8;
#endif

#ifdef ANI_BIG_BYTE_ENDIAN
   tANI_U32   usBmpsModeEarlyTimeoutUs        : 16;
   tANI_U32   usBmpsMinSleepTimeUs            : 16;
#else
   tANI_U32   usBmpsMinSleepTimeUs            : 16;
   tANI_U32   usBmpsModeEarlyTimeoutUs        : 16;
#endif

#ifdef ANI_BIG_BYTE_ENDIAN
   tANI_U32   usBmpsSleepTimeOverheadsUs      : 16;
   tANI_U32   usBmpsForcedSleepTimeOverheadsUs: 16;
#else
   tANI_U32   usBmpsForcedSleepTimeOverheadsUs: 16;
   tANI_U32   usBmpsSleepTimeOverheadsUs      : 16;
#endif

#ifdef ANI_BIG_BYTE_ENDIAN
   tANI_U32   ucBmpsFirstBeaconTimeoutMs      : 8;
   tANI_U32   ucBdPduEmptyMonitorMs           : 8;
   tANI_U32   bReserved3                      : 8;
   tANI_U32   ucRfSupplySettlingTimeClk19_2   : 8;
#else
   tANI_U32   ucRfSupplySettlingTimeClk19_2   : 8;
   tANI_U32   bReserved3                      : 8;
   tANI_U32   ucBdPduEmptyMonitorMs           : 8;
   tANI_U32   ucBmpsFirstBeaconTimeoutMs      : 8;
#endif

#ifdef ANI_BIG_BYTE_ENDIAN
   tANI_U32   ucRegulateTrafficMonitorMsec  : 8;
   tANI_U32   usBdPduOffset                 : 16;
   tANI_U32   ucDpuRoutingWq                : 8;
#else
   tANI_U32   ucDpuRoutingWq                : 8;
   tANI_U32   usBdPduOffset                 : 16;
   tANI_U32   ucRegulateTrafficMonitorMsec  : 8;
#endif

#ifdef ANI_BIG_BYTE_ENDIAN
   tANI_S8   ucRssiThreshold1     : 8;
   tANI_S8   ucRssiThreshold2     : 8;
   tANI_S8   ucRssiThreshold3     : 8;
   tANI_U8   bRssiThres1PosNotify : 1;
   tANI_U8   bRssiThres1NegNotify : 1;
   tANI_U8   bRssiThres2PosNotify : 1;
   tANI_U8   bRssiThres2NegNotify : 1;
   tANI_U8   bRssiThres3PosNotify : 1;
   tANI_U8   bRssiThres3NegNotify : 1;
   tANI_U8   bReserved10          : 2;
#else
   tANI_U8   bReserved10          : 2;
   tANI_U8   bRssiThres3NegNotify : 1;
   tANI_U8   bRssiThres3PosNotify : 1;
   tANI_U8   bRssiThres2NegNotify : 1;
   tANI_U8   bRssiThres2PosNotify : 1;
   tANI_U8   bRssiThres1NegNotify : 1;
   tANI_U8   bRssiThres1PosNotify : 1;
   tANI_S8   ucRssiThreshold3     : 8;
   tANI_S8   ucRssiThreshold2     : 8;
   tANI_S8   ucRssiThreshold1     : 8;
#endif

   /* BTC */
   tANI_U32 btcBtIntervalMode1;
   tANI_U32 btcWlanIntervalMode1;
   tANI_U32 btcActionOnPmFailMode1;
   tANI_U32 btcExecutionMode;

   /* TPC */
   tANI_U32   uTpcGainLutAduReinitAddr;

#ifdef ANI_BIG_BYTE_ENDIAN
   tANI_U32   uAirTimeComp         : 16;
   tANI_U32   uInitTimeComp        : 16;
#else
   tANI_U32   uInitTimeComp        : 16;
   tANI_U32   uAirTimeComp         : 16;
#endif

   tANI_U32   uVolRegReinitAddress;
   tANI_U32   uNumVolRegCount;

#ifdef ANI_BIG_BYTE_ENDIAN
   tANI_U32   ucUapsdDataRecepTimeoutMs      : 8;
   tANI_U32   usBmpsSleepTimeOverheadsUs19_2 : 16;
   tANI_U32   ucNumConsBcnMiss               : 8; 
#else
   tANI_U32   ucNumConsBcnMiss               : 8; 
   tANI_U32   usBmpsSleepTimeOverheadsUs19_2 : 16;
   tANI_U32   ucUapsdDataRecepTimeoutMs      : 8;
#endif

   tANI_U32   uMgmtWoWLPassMask;

#ifdef WLAN_SOFTAP_FEATURE
   tANI_U32   uBssTableOffset;
   tANI_U32   uStaTableOffset;
   tANI_U32   beaconTemplate_offset;
#if WLAN_SOFTAP_FW_PROCESS_PROBE_REQ_FEATURE
   tANI_U32   probeRespTemplate_offset;
#endif

#ifdef ANI_BIG_BYTE_ENDIAN
   tANI_U32 fDisLinkMonitor:1;                 // when set link monitoring is disabled.
   tANI_U32 fEnableFwUnknownAddr2Handling:1;   // enables handling of unknown addr2 at FW.
   tANI_U32 bReserved11:30;
#else
   tANI_U32 bReserved11:30;
   tANI_U32 fEnableFwUnknownAddr2Handling:1;  // enables handling of unknown addr2 at FW.
   tANI_U32 fDisLinkMonitor:1;                // when set link monitoring is disabled.
#endif
   tANI_U32 ucApLinkMonitorMsec;              //link monitoring timer interval
   tANI_U32 ucUnknownAddr2CreditIntvMsec;     //interval at which unkown addr2 credit will be reset to max.
#endif

#ifdef ANI_BIG_BYTE_ENDIAN
   tANI_U32  ucMaxBss:8;
   tANI_U32  ucMaxSta:8;
   tANI_U32  ucDpuSig:8;
   tANI_U32  ucDpuIdx:8;
#else
   tANI_U32  ucDpuIdx:8;
   tANI_U32  ucDpuSig:8;
   tANI_U32  ucMaxSta:8;
   tANI_U32  ucMaxBss:8;
#endif

   UapsdAcParamType acParam[QWLANFW_MAX_AC];

} Qwlanfw_SysCfgType;

/*===========================================================================
  RA CONFIGURATION
===========================================================================*/
typedef enum {
    TPE_STA_PRIM_DATA_RATE = 0,
    TPE_STA_SECD_DATA_RATE,
    TPE_STA_TERT_DATA_RATE,
    TPE_STA_MAX_RETRY_RATE
} tTpeRetryRate;

typedef enum {
    TPE_STA_20MHZ_RATE = 0,
    TPE_STA_40MHZ_RATE,
    TPE_STA_BD_RATE,
    TPE_STA_RATE_TYPE_MAX
} tTpeRateType;

typedef enum
{
    TPE_RATE_PROTECTION_NONE,
    TPE_RATE_PROTECTION_CTS,
    TPE_RATE_PROTECTION_RTS,
    TPE_RATE_PROTECTION_DUAL_CTS, //not supported

    // RTS always is not one of the defined TPE desc
    // protection mode, this is to force using RTS/CTS
    // protection
    TPE_RATE_PROTECTION_RTS_ALWAYS,
    // Auto mode is not one of the defined TPE descriptor
    // protection mode, so keep this at the end.
    // Basically auto mode is to decide which mode will be
    // appropriate to choose by the host.
    TPE_RATE_PROTECTION_AUTO
}tTpeProtPolicy;

/**
 *  TPE STA Desc Rate Info
 */
typedef PACKED_PRE struct PACKED_POST sTpeStaDescRateInfo{

#ifdef ANI_BIG_BYTE_ENDIAN
    tANI_U32 reserved               : 4;
    tANI_U32 protection_mode        : 2;
    tANI_U32 ampdu_density          : 7;
    tANI_U32 tx_power               : 5;
    tANI_U32 tx_antenna_enable      : 3;
    tANI_U32 STBC_Valid             : 2;
    tANI_U32 rate_index             : 9;
#else
    tANI_U32 rate_index             : 9;
    tANI_U32 STBC_Valid             : 2;
    tANI_U32 tx_antenna_enable      : 3;
    tANI_U32 tx_power               : 5;
    tANI_U32 ampdu_density          : 7;
    tANI_U32 protection_mode        : 2;
    tANI_U32 reserved               : 4;
#endif
}ALIGN_4 tTpeStaDescRateInfo, *tpTpeStaDescRateInfo;

enum {
    PKT_TYPE_11a = 0,
    PKT_TYPE_11b,
    PKT_TYPE_11N_GREENFIELD,
    PKT_TYPE_11N_MIXEDMODE
};

// Difference in the indices between Mixed mode rates and the GF rates
#define HALRATE_MM_TO_GF_OFFSET         18

typedef enum
{
    // 802.11b Rates - Long Preamble
    TPE_RT_IDX_11B_RATE_LONG_PR_BASE_OFFSET = 0,
    TPE_RT_IDX_11B_LONG_1_MBPS = TPE_RT_IDX_11B_RATE_LONG_PR_BASE_OFFSET,
    TPE_RT_IDX_11B_LONG_2_MBPS,
    TPE_RT_IDX_11B_LONG_5_5_MBPS,
    TPE_RT_IDX_11B_LONG_11_MBPS,

    // 802.11b Rates - Short Preamble
    TPE_RT_IDX_11B_RATE_SHORT_PR_BASE_OFFSET = 4,
    TPE_RT_IDX_11B_LONG_1_MBPS_DUP = TPE_RT_IDX_11B_RATE_SHORT_PR_BASE_OFFSET,
    TPE_RT_IDX_11B_SHORT_2_MBPS,
    TPE_RT_IDX_11B_SHORT_5_5_MBPS,
    TPE_RT_IDX_11B_SHORT_11_MBPS,

    // Libra 11AG 20MHz Rates                   //rate codes
    TPE_RT_IDX_11A_6_MBPS = 8,
    TPE_RT_IDX_11A_9_MBPS,
    TPE_RT_IDX_11A_12_MBPS,
    TPE_RT_IDX_11A_18_MBPS,
    TPE_RT_IDX_11A_24_MBPS,
    TPE_RT_IDX_11A_36_MBPS,
    TPE_RT_IDX_11A_48_MBPS,
    TPE_RT_IDX_11A_54_MBPS,

    //MCS Index #0-15 (20MHz) Mixed Mode
    TPE_RT_IDX_MCS_1NSS_MM_6_5_MBPS = 16,
    TPE_RT_IDX_MCS_1NSS_MM_13_MBPS,
    TPE_RT_IDX_MCS_1NSS_MM_19_5_MBPS,
    TPE_RT_IDX_MCS_1NSS_MM_26_MBPS,
    TPE_RT_IDX_MCS_1NSS_MM_39_MBPS,
    TPE_RT_IDX_MCS_1NSS_MM_52_MBPS,
	TPE_RT_IDX_MCS_1NSS_MM_58_5_MBPS,
    TPE_RT_IDX_MCS_1NSS_MM_65_MBPS,
    TPE_RT_IDX_MCS_1NSS_MM_SG_7_2_MBPS,
    TPE_RT_IDX_MCS_1NSS_MM_SG_14_4_MBPS,
    TPE_RT_IDX_MCS_1NSS_MM_SG_21_7_MBPS,
    TPE_RT_IDX_MCS_1NSS_MM_SG_28_9_MBPS,
    TPE_RT_IDX_MCS_1NSS_MM_SG_43_3_MBPS,
    TPE_RT_IDX_MCS_1NSS_MM_SG_57_8_MBPS,
    TPE_RT_IDX_MCS_1NSS_MM_SG_65_MBPS,
    TPE_RT_IDX_MCS_1NSS_MM_SG_72_2_MBPS,

    //SLR Rates
    TPE_RT_IDX_SLR_0_25_MBPS = 32,
    TPE_RT_IDX_SLR_0_5_MBPS,

    //MCS Index #0-15 (20MHz) Greenfield Mode
    TPE_RT_IDX_MCS_1NSS_GF_6_5_MBPS = 34,
    TPE_RT_IDX_MCS_1NSS_GF_13_MBPS,
    TPE_RT_IDX_MCS_1NSS_GF_19_5_MBPS,
    TPE_RT_IDX_MCS_1NSS_GF_26_MBPS,
    TPE_RT_IDX_MCS_1NSS_GF_39_MBPS,
    TPE_RT_IDX_MCS_1NSS_GF_52_MBPS,
    TPE_RT_IDX_MCS_1NSS_GF_58_5_MBPS,
    TPE_RT_IDX_MCS_1NSS_GF_65_MBPS,

    //Qualcomm Proprietary Rates 20Mhz Greenfield Mode
    TPE_RT_IDX_ANI_GF_68_25_MBPS = 42,

    // Since STBC rates are not used for TX,
    // this is the max TX rates
    TPE_RT_IDX_MAX_TX_RATES = 43,

    // NOTE: As we have only one transmit chain, STBC rates
    // cannot be used in the transmit side.
    //MCS Index #0-7 (20MHz) STBC Rates Greenfield Mode
    TPE_RT_IDX_MCS_1NSS_STBC_GF_6_5_MBPS = TPE_RT_IDX_MAX_TX_RATES,
    TPE_RT_IDX_MCS_1NSS_STBC_GF_13_MBPS,
    TPE_RT_IDX_MCS_1NSS_STBC_GF_19_5_MBPS,
    TPE_RT_IDX_MCS_1NSS_STBC_GF_26_MBPS,
    TPE_RT_IDX_MCS_1NSS_STBC_GF_39_MBPS,
    TPE_RT_IDX_MCS_1NSS_STBC_GF_52_MBPS,
    TPE_RT_IDX_MCS_1NSS_STBC_GF_58_5_MBPS,
    TPE_RT_IDX_MCS_1NSS_STBC_GF_65_MBPS,

    //MCS Index #0-15 (20MHz) STBC Rates Mixed Mode
    TPE_RT_IDX_MCS_1NSS_STBC_MM_6_5_MBPS = 51,
    TPE_RT_IDX_MCS_1NSS_STBC_MM_SG_7_2_MBPS,
    TPE_RT_IDX_MCS_1NSS_STBC_MM_13_MBPS,
    TPE_RT_IDX_MCS_1NSS_STBC_MM_SG_14_4_MBPS,
    TPE_RT_IDX_MCS_1NSS_STBC_MM_19_5_MBPS,
    TPE_RT_IDX_MCS_1NSS_STBC_MM_SG_21_7_MBPS,
    TPE_RT_IDX_MCS_1NSS_STBC_MM_26_MBPS,
    TPE_RT_IDX_MCS_1NSS_STBC_MM_SG_28_9_MBPS,
    TPE_RT_IDX_MCS_1NSS_STBC_MM_39_MBPS,
    TPE_RT_IDX_MCS_1NSS_STBC_MM_SG_43_3_MBPS,
    TPE_RT_IDX_MCS_1NSS_STBC_MM_52_MBPS,
    TPE_RT_IDX_MCS_1NSS_STBC_MM_SG_57_8_MBPS,
    TPE_RT_IDX_MCS_1NSS_STBC_MM_58_5_MBPS,
    TPE_RT_IDX_MCS_1NSS_STBC_MM_SG_65_MBPS,
    TPE_RT_IDX_MCS_1NSS_STBC_MM_65_MBPS,
    TPE_RT_IDX_MCS_1NSS_STBC_MM_SG_72_2_MBPS,

    //Qualcomm Proprietary Rates 20Mhz Mixed Mode
    TPE_RT_IDX_ANI_STBC_GF_68_25_MBPS = 67,

    TPE_RT_IDX_MAX_RATES = 68,

    TPE_RT_IDX_INVALID =   TPE_RT_IDX_MAX_RATES

} tTpeRateIdx;

/* bitfields defined for rateProperty field in the tHalRateInfo structure*/
#define RA_CCKDSSS      0x1
#define RA_40MHZ        0x2
#define RA_DUPLICATE    0x4
#define RA_HT           0x8
#define RA_MIMO         0x10
#define RA_SGI          0x20
#define RA_SPREAMBLE    0x40
#define RA_GENERIC      0x80
#define RA_11AG_RATES   0x100
#define RA_STBC         0x200
#define RA_SLR          0x400
#define RA_MM           0x800
#define RA_QCOMM        0x1000
#define RA_DISABLED     0x2000

/* bit 14-15 unused */

/* bit 16-17 */
#define RA_MODULATION_SHIFT 16
#define RA_MODULATION_MASK  (3<<RA_MODULATION_SHIFT)
#define RA_BPSK             (0<<RA_MODULATION_SHIFT)
#define RA_QPSK             (1<<RA_MODULATION_SHIFT)
#define RA_16QAM            (2<<RA_MODULATION_SHIFT)
#define RA_64QAM            (3<<RA_MODULATION_SHIFT)
#define RA_TOTAL_MODULATION   (4)

/* bit 18-19 unused */

/* bit 20-22 */
#define RA_CODERATE_SHIFT   20
#define RA_CODERATE_MASK    (7 << RA_CODERATE_SHIFT)

#define RA_CODERATE_1_2     (0<<RA_CODERATE_SHIFT)
#define RA_CODERATE_2_3     (1<<RA_CODERATE_SHIFT)
#define RA_CODERATE_3_4     (2<<RA_CODERATE_SHIFT)
#define RA_CODERATE_5_6     (3<<RA_CODERATE_SHIFT)
#define RA_CODERATE_7_8     (4<<RA_CODERATE_SHIFT)
#define RA_TOTAL_CODERATE   (5)

/*bit 23-31 unused */

#define RA_11B_RATES            (RA_CCKDSSS)
#define RA_11B_SPREAMBLE        (RA_11B_RATES | RA_SPREAMBLE )
#define RA_11B_DUP              (RA_11B_RATES | RA_DUPLICATE | RA_40MHZ)
#define RA_11B_SPREAMBLE_DUP    (RA_11B_SPREAMBLE | RA_DUPLICATE | RA_40MHZ)
#define RA_11AG_DUP_RATES       (RA_11AG_RATES| RA_DUPLICATE | RA_40MHZ)

#define RA_HT_MIMO_MASK              (RA_HT|RA_MIMO)

#define RA_HT_20_SIMO           (RA_HT)
#define RA_HT_SGI               (RA_HT|RA_SGI)
#define RA_HT_20_SIMO_SGI       (RA_HT|RA_SGI)
#define RA_HT_20_MIMO           (RA_HT|RA_MIMO)
#define RA_HT_20_MIMO_SGI       (RA_HT|RA_MIMO|RA_SGI)

#define RA_HT_40_SIMO           (RA_HT|RA_40MHZ)
#define RA_HT_40_SIMO_SGI       (RA_HT|RA_40MHZ|RA_SGI)
#define RA_HT_40_MIMO           (RA_HT|RA_40MHZ|RA_MIMO)
#define RA_HT_40_MIMO_SGI       (RA_HT|RA_40MHZ|RA_MIMO|RA_SGI)

#define RA_HT_MCS32             (RA_HT|RA_40MHZ|RA_DUPLICATE)
#define RA_HT_MCS32_SGI         (RA_HT|RA_40MHZ|RA_DUPLICATE|RA_SGI)

#define RA_QCOMM_20_SIMO        (RA_HT|RA_QCOMM)

#define RA_HT_MCSOFFSET_MASK               (7)

/* HALRATE_TOTAL_SENSITIVITY_LEVELS are used between the host/firmware shared table */
#define HALRATE_MIN_SENSITIVITY    (-20)
#define HALRATE_MAX_SENSITIVITY    (320)

#define HALRATE_SENSITIVITY_UNIT    (10)

#define HALRATE_TOTAL_SENSITIVITY_LEVELS  ((HALRATE_MAX_SENSITIVITY-HALRATE_MIN_SENSITIVITY)/10)


typedef struct sTpeSramRateTable {
    tANI_U32    word0;
    tANI_U32    word1;
} tTpeSramRateTable, *tpTpeSramRateTable;


typedef struct sTpeRateTable {

#ifdef ANI_BIG_BYTE_ENDIAN
	/* word 0 */
    tANI_U32    cntrlRspTxPwr:5;
    tANI_U32    rsvd:1;
    tANI_U32    txAntEnable:3;
    tANI_U32    cntrlRateIdx:9;
    tANI_U32    shortGuard:1;
    tANI_U32    nssOr11bMode:2;
    tANI_U32    bwMode:2;
    tANI_U32    psduRate:7;
    tANI_U32    pktType:2;

	/* word 1 */
    tANI_U32    stbcValid:2;
    tANI_U32    protCntrlRateIdx:4;
    tANI_U32    neltfs:2;
    tANI_U32    ndltfs:2;
    tANI_U32    ndbpsOr4timesRate:12;
    tANI_U32    ampduValid:1;
    tANI_U32    rspRateIdx:9;
#else
	/* word 0 */
    tANI_U32    pktType:2;
    tANI_U32    psduRate:7;
    tANI_U32    bwMode:2;
    tANI_U32    nssOr11bMode:2;
    tANI_U32    shortGuard:1;
    tANI_U32    cntrlRateIdx:9;
    tANI_U32    txAntEnable:3;
    tANI_U32    rsvd:1;
    tANI_U32    cntrlRspTxPwr:5;

	/* word 1 */
    tANI_U32    rspRateIdx:9;
    tANI_U32    ampduValid:1;
    tANI_U32    ndbpsOr4timesRate:12;
    tANI_U32    ndltfs:2;
    tANI_U32    neltfs:2;
    tANI_U32    protCntrlRateIdx:4;
    tANI_U32    stbcValid:2;
#endif
} tTpeRateTable, *tpTpeRateTable;

typedef struct sHalRateInfo {
#ifdef ANI_BIG_BYTE_ENDIAN
       tANI_U16    actualTputKbps;     /* max MAC SAP throughput */
       tANI_U16    thruputKbps;        /* PHY rate */

       tANI_U16    ieRateMcsIdx;       /* non 11n rates: IE rate. 11n rates: MCS index */
       tANI_S16    sensitivity;        /* 1 unit= 0.1db */

       tANI_U16    sensThruputRank;    /* rank of rate when sorted by sensitivity (primary) and throughput */
       tANI_U16    thruputSensRank;    /* rank of rate when sorted by throughput (primary) and sensitivity */

       tANI_U32    rateProperty;       /* Properties of rate */
       tANI_U32    tpeRateIdx;         /* TPE Rate index */
#else
       tANI_U16    thruputKbps;        /* PHY rate */
       tANI_U16    actualTputKbps;     /* max MAC SAP throughput */
       tANI_S16    sensitivity;        /* 1 unit= 0.1db */
       tANI_U16    ieRateMcsIdx;       /* non 11n rates: IE rate. 11n rates: MCS index */
       tANI_U16    sensThruputRank;    /* rank of rate when sorted by sensitivity (primary) and throughput */
       tANI_U16    thruputSensRank;    /* rank of rate when sorted by throughput (primary) and sensitivity */
       tANI_U32    rateProperty;       /* Properties of rate */
       tTpeRateIdx tpeRateIdx;         /* TPE Rate index */
#endif
} tHalRateInfo, * tpHalRateInfo;


#define HAL_RA_MAX_RATES           3

/* NOTE: Libra supports only 20MHz mode and doesnot support the channel bonding
 * mode, the per station TX rate channel is 1 */
#define HAL_RA_TXRATE_CHANNEL_NUM  1

/*
 * Rate information per station
 */
/* -------------------------------------------------------------------------- */
/* enumerated mac supported rates
 * this includes generic user rates which map to a set of actual rates
 * as well as all specific mac rates actually supported on the device
 * NOTE: these enums are used to index into RA tables, so any changes here
 * must be reflected in the RA tables as well
 */
typedef enum eHalMacRate
{
    /* ----------------------------------------------------------
     * generic mac rates - represent a rate which can be achieved
     * by some combination of supported rate/mode
     */
    HALRATE_MODE_START = 0,

    /* 11b long preamble rates */
    HALRATE_11B_START = HALRATE_MODE_START,
    HALRATE_1 = HALRATE_11B_START,                 /* +0 */
    HALRATE_2,                                     /* +1 */
    HALRATE_55,                                    /* +2 */
    HALRATE_11,                                    /* +3 */
    HALRATE_11B_END = HALRATE_11,

    /* 11b short preamble rates. No 1Mbps short preamble rate*/
    HALRATE_SPREAM_11B_START,
    HALRATE_SPREAM_1 = HALRATE_SPREAM_11B_START,   /* +4 */
    HALRATE_SPREAM_2,                              /* +5 */
    HALRATE_SPREAM_55,                             /* +6 */
    HALRATE_SPREAM_11,                             /* +7 */
    HALRATE_SPREAM_11B_END = HALRATE_SPREAM_11,

    /* 11a/g rates RxP rate index 8-15*/
    HALRATE_11A_START,
    HALRATE_6 = HALRATE_11A_START,           /* +8  */
    HALRATE_9,                                 /* +9  */
    HALRATE_12,                                /* +10 */
    HALRATE_18,                                /* +11 */
    HALRATE_24,                                /* +12 */
    HALRATE_36,                                /* +13 */
    HALRATE_48,                                /* +14 */
    HALRATE_54,                                /* +15 */
    HALRATE_11A_END = HALRATE_54,

    /* HT simo rates MCS 0-7 */
    HALRATE_HT_SIMO_START,
    HALRATE_HT_START = HALRATE_HT_SIMO_START,
    HALRATE_HT_SIMO_0065 = HALRATE_HT_SIMO_START,     /* +16 */
    HALRATE_HT_SIMO_0130,                              /* +17 */
    HALRATE_HT_SIMO_0195,                              /* +18 */
    HALRATE_HT_SIMO_0260,                              /* +19 */
    HALRATE_HT_SIMO_0390,                              /* +20 */
    HALRATE_HT_SIMO_0520,                              /* +21 */
    HALRATE_HT_SIMO_0585,                              /* +22 */
    HALRATE_HT_SIMO_0650,                              /* +23 */
    HALRATE_HT_SIMO_END = HALRATE_HT_SIMO_0650,

    /* HT simo+SGI rates MCS 0-7 */
    HALRATE_HT_SIMO_SGI_START,
    HALRATE_HT_SIMO_SGI_0072 = HALRATE_HT_SIMO_SGI_START,  /* +24 */
    HALRATE_HT_SIMO_SGI_0144,                              /* +25 */
    HALRATE_HT_SIMO_SGI_0217,                              /* +26 */
    HALRATE_HT_SIMO_SGI_0289,                              /* +27 */
    HALRATE_HT_SIMO_SGI_0433,                              /* +28 */
    HALRATE_HT_SIMO_SGI_0578,                              /* +29 */
    HALRATE_HT_SIMO_SGI_0650,                              /* +30 */
    HALRATE_HT_SIMO_SGI_0722,                              /* +31 */
    HALRATE_HT_SIMO_SGI_END = HALRATE_HT_SIMO_SGI_0722,

    /* SLR rates */
    HALRATE_SLR_START,
    HALRATE_SLR_0025 = HALRATE_SLR_START,             /* +32 */
    HALRATE_SLR_0050,                              /* +33 */
    HALRATE_SLR_END = HALRATE_SLR_0050,

    /* Greenfield MCS 0-7 rates */
    HALRATE_GF_START,
    HALRATE_GF_SIMO_0065 = HALRATE_GF_START,          /* +34 */
    HALRATE_GF_SIMO_0130,                          /* +35 */
    HALRATE_GF_SIMO_0195,                          /* +36 */
    HALRATE_GF_SIMO_0026,                          /* +37 */
    HALRATE_GF_SIMO_0039,                          /* +38 */
    HALRATE_GF_SIMO_0052,                          /* +39 */
    HALRATE_GF_SIMO_0585,                          /* +40 */
    HALRATE_GF_SIMO_0650,                          /* +41 */
    HALRATE_GF_END = HALRATE_GF_SIMO_0650,

    /* Qualcomm proprietary rates */
    HALRATE_QUALCOMM_GF_START,
    HALRATE_QUALCOMM_GF_6825 = HALRATE_QUALCOMM_GF_START,   /* +42 */
    HALRATE_QUALCOMM_GF_END = HALRATE_QUALCOMM_GF_6825,

    // NOTE: As we have only one transmit chain, STBC rates
    // cannot be used in the transmit side.
    HALRATE_MODE_TX_END,         /* 43 */

    //MCS Index #0-7 (20MHz) STBC Rates Greenfield Mode
    HALRATE_STBC_START = HALRATE_MODE_TX_END,
    HALRATE_STBC_GF_START = HALRATE_STBC_START,
    HALRATE_STBC_GF_0065 = HALRATE_STBC_GF_START,        /* +43 */
    HALRATE_STBC_GF_0013,                          /* +44 */
    HALRATE_STBC_GF_0195,                          /* +45 */
    HALRATE_STBC_GF_0026,                          /* +46 */
    HALRATE_STBC_GF_0039,                          /* +47 */
    HALRATE_STBC_GF_0052,                          /* +48 */
    HALRATE_STBC_GF_0585,                          /* +49 */
    HALRATE_STBC_GF_0650,                                   /* +50 */
    HALRATE_STBC_GF_END = HALRATE_STBC_GF_0650,

    //MCS Index #0-15 (20MHz) STBC Rates Mixed Mode
    HALRATE_STBC_MM_START,
    HALRATE_STBC_MM_0065 = HALRATE_STBC_MM_START,        /* +51 */
    HALRATE_STBC_MM_SG_0072,                       /* +52 */
    HALRATE_STBC_MM_0013,                          /* +53 */
    HALRATE_STBC_MM_SG_0144,                       /* +54 */
    HALRATE_STBC_MM_0195,                          /* +55 */
    HALRATE_STBC_MM_SG_0217,                       /* +56 */
    HALRATE_STBC_MM_0026,                          /* +57 */
    HALRATE_STBC_MM_SG_0289,                       /* +58 */
    HALRATE_STBC_MM_0039,                          /* +59 */
    HALRATE_STBC_MM_SG_0433,                       /* +60 */
    HALRATE_STBC_MM_0052,                          /* +61 */
    HALRATE_STBC_MM_SG_0578,                       /* +62 */
    HALRATE_STBC_MM_0585,                          /* +63 */
    HALRATE_STBC_MM_SG_0650,                       /* +64 */
    HALRATE_STBC_MM_0650,                          /* +65 */
    HALRATE_STBC_MM_SG_0722,                       /* +66 */
    HALRATE_STBC_MM_END = HALRATE_STBC_MM_SG_0722,

    //Qualcomm Proprietary Rates 20Mhz Mixed Mode
    HALRATE_QUALCOMM_STBC_GF_6825_MBPS,                       /* +67 */
    HALRATE_STBC_END = HALRATE_QUALCOMM_STBC_GF_6825_MBPS,

    /* Invalid Rate */
    HALRATE_INVALID, /* 68 */

    HALRATE_MODE_END,

} tHalMacRate, *tpHalMacRate;

/* -------------------------------------------------------------------------- */
/* macros to get information about the rates */

/* number of all supported rates */
#define HAL_MAC_MAX_RATES         (HALRATE_MODE_END - HALRATE_MODE_START)

/* number of supported TX rates */
#define HAL_MAC_MAX_TX_RATES      (HALRATE_MODE_TX_END - HALRATE_MODE_START) // 43

/* number of 11b rates */
#define HAL_NUM_11B_RATES         (HALRATE_11B_END - HALRATE_11B_START + 1)
/* number of 11a rates */
#define HAL_NUM_11A_RATES         (HALRATE_11A_END - HALRATE_11A_START + 1)

#define HAL_NUM_MACRATE_2_IERATE_ENTRIES (HALRATE_MODE_TX_END - HALRATE_MODE_START)


/*-- flavours of supported rates
 * valid types are
    11B
    SPREAM_11B
    DUP_11B
    SPREAM_DUP_11B
    11A
    DUP_11A
    MIMO
    CB
    MIMO_CB
    HT_SIMO
    HT_SIMO_SGI
    HT_MIMO
    HT_MIMO_SGI
    HT_SIMO_CB
    HT_SIMO_CB_SGI
    HT_MIMO_CB
    HT_MIMO_CB_SGI
 * example use: HAL_RATE_IS_TYPE(MIMO_CB, halRate)
 */

/* how many tANI_U32's needed to hold a bitmap of nRates */
#define RA_CONVERT_2_U32_BITMAP(nRates) ((nRates + 31) >> 5)

/* #tANI_U32's needed for a bitmap representation for all rates */
#define HAL_NUM_U32_MAP_RATES    RA_CONVERT_2_U32_BITMAP(HAL_MAC_MAX_TX_RATES)   // 2

/*
 * number of rates that are potential candidates for the next step from a
 * given rate. This is the max number of rates that can be sampled from any
 * current rate. Among these RA_SAMPLING_RATES_MAX rates, index RA_SAMPLING_BASE_RATE
 * is the base rate and index RA_SAMPLING_JUMP_RATE is the jump rate.
 */
#define RA_SAMPLING_BASE_RATE  0
#define RA_SAMPLING_FIRST_RATE  1
#if 0 //  --> works, but let's wait until the feedback from the team
#define RA_SAMPLING_RATES_MAX  8
#else
#define RA_SAMPLING_RATES_MAX  12
#endif
/* ---------------------------------------------------------------------------
 * the sampling table is an array of rates that should be sampled to determine
 * the next rate to jump to. The RA algorithm will look at this set of rates
 * to determine which rate to jump to (subject to per restrictions)
 */
typedef PACKED_PRE struct PACKED_POST sRateAdaptSamplingTable {
    tANI_U8 sampleRate[RA_SAMPLING_RATES_MAX];
} tRateAdaptSamplingTable, *tpRateAdaptSamplingTable;

#define RA_GOODPERTHRESH_SENSITIVITY_TABLE_SIZE  10
/*
 * how to set the retry rates
 */
typedef enum eHalRetryMode {
    HAL_RETRY_USE_WHATEVER = 0, /* use whatever the algo thinks is best */
    HAL_RETRY_USE_MINRATE,
    HAL_RETRY_USE_PRIMARY,  /* use primary rate as retry */
    HAL_RETRY_USE_SPECIFIED, /* use specified retry rate */
    HAL_RETRY_USE_CLOSEST, /* use closes lower rate */
} tHalRetryMode, *tpHalRetryMode;

/* ----------------------------------------------------------------------------
 * types used by rate adaptation module
 */

/*
 * rate adapt mode : determines current operating mode of the RA algorithm
 */
typedef enum eRateAdaptMode {
    RATE_ADAPT_FIXED = 0,
    RATE_ADAPT_AUTO
} tRateAdaptMode, *tpRateAdaptMode;

/* PER selection algorithm */
#define RA_PER_SELECT_MPDU    0
#define RA_PER_SELECT_PPDU    1
#define RA_PER_SELECT_HYBRID  2

#define RA_STA_RATE_HISTORY_LEN 32  /* code assumes this number is power of 2 */

/*
*  Periodically. HAL uses these counter to switch
*  rate at proper time.
*/
typedef PACKED_PRE struct PACKED_POST sTxRateStat {
#ifdef ANI_BIG_BYTE_ENDIAN
    tANI_U32 totalTxPpdus:16;      /* Total PPDU transmitted */
    tANI_U32 totalTxMpdus:16;       /* Total MPDU transmitted in all AMPDU & MPDUs */
#else
    tANI_U32 totalTxMpdus:16;       /* Total MPDU transmitted in all AMPDU & MPDUs */
    tANI_U32 totalTxPpdus:16;      /* Total PPDU transmitted */
#endif

#ifdef ANI_BIG_BYTE_ENDIAN
    tANI_U32 totalAckTimeoutPpdus:16; /* # of BA/ACK timeouts */
    tANI_U32 totalHybridTx:16;      /* # of total transmission.
                                        If ACK timeout, only+1, if ACK comes back add # of MPDU transmitted.  */
#else
    tANI_U32 totalHybridTx:16;      /* # of total transmission.
                                        If ACK timeout, only+1, if ACK comes back add # of MPDU transmitted.  */
    tANI_U32 totalAckTimeoutPpdus:16; /* # of BA/ACK timeouts */
#endif

#ifdef ANI_BIG_BYTE_ENDIAN
    tANI_U32 totalRespMpdus:16;     /* including all MPDUs ACKed & NACKed in BA and ACK */
    tANI_U32 dataRateIndex:16;      /* data rate index */
#else
    tANI_U32 dataRateIndex:16;      /* data rate index */
    tANI_U32 totalRespMpdus:16;     /* including all MPDUs ACKed & NACKed in BA and ACK */
#endif

} tTxRateStat;

/*
*  For each STA, now HAL selects 6 rates and configures to TPE.
*  In TPE, for each STA, a rate stat block is maintained which
*  directly updated by the TPE on TX activities and is
*  periodically collected by HAL for rate adaptation decision.
*/
typedef PACKED_PRE struct PACKED_POST sRateAdaptTxStat {

    tTxRateStat  rastats[HAL_RA_MAX_RATES][HAL_RA_TXRATE_CHANNEL_NUM];

} tRateAdaptTxStat;


/* ----------------------------------------------------------------------------
 * per sta RA information
 */
typedef PACKED_PRE struct PACKED_POST sHalRaInfo {
#ifdef ANI_BIG_BYTE_ENDIAN
    tANI_U8         lowestRateByNwType;
    tANI_U8         opRateMode;
    tANI_U8         fixedRate;      /* if fixed rate is being used, this is it */
    tANI_U8         rateAdaptMode;
#else
    tANI_U8         rateAdaptMode;
    tANI_U8         fixedRate;      /* if fixed rate is being used, this is it */
    tANI_U8         opRateMode;
    tANI_U8         lowestRateByNwType;
#endif

    tANI_U32        supportedRates[HAL_NUM_U32_MAP_RATES];
    tANI_U32        validRates[HAL_NUM_U32_MAP_RATES];

#ifdef ANI_BIG_BYTE_ENDIAN
    tANI_U16        maxDataRate;    /* Max Data Rate as per 11n draft */
    tANI_U8         currentRate;    /* the current mac rate */
    tANI_U8         maxAmpduDensity;  /* 3 : 0~7 : 2^(11nAMPDUdensity -4) */
#else
    tANI_U8         maxAmpduDensity;  /* 3 : 0~7 : 2^(11nAMPDUdensity -4) */
    tANI_U8         currentRate;    /* the current mac rate */
    tANI_U16        maxDataRate;    /* Max Data Rate as per 11n draft */
#endif

#ifdef ANI_BIG_BYTE_ENDIAN
    tANI_U8         bssIdx;
    tANI_U8         minDataRateIdx;    /* Min Data Rate for this STA */
    tANI_U8         staType;
    tANI_U8         reserved2:1;
    tANI_U8         gfEnabled:1;
    tANI_U8         shortPreamble:1;
    tANI_U8         shortGI40:1;
    tANI_U8         shortGI20:1;
    tANI_U8         mimoMode:1;
    tANI_U8         cbMode:1;
    tANI_U8         valid:1;
#else /* ANI_BIG_BYTE_ENDIAN */
    tANI_U8         valid:1;
    tANI_U8         cbMode:1;
    tANI_U8         mimoMode:1;
    tANI_U8         shortGI20:1;
    tANI_U8         shortGI40:1;
    tANI_U8         shortPreamble:1;
    tANI_U8         gfEnabled:1;
    tANI_U8         reserved2:1;
    tANI_U8         staType;
    tANI_U8         minDataRateIdx;    /* Min Data Rate for this STA */
    tANI_U8         bssIdx;
#endif /* ANI_BIG_BYTE_ENDIAN */

    /* Previously reported Tx stats */
    tRateAdaptTxStat prevStatsCache;
    tANI_U8         sRateTable[CEIL_ALIGN(HAL_MAC_MAX_TX_RATES,4)];
    tANI_U8         tRateTable[CEIL_ALIGN(HAL_MAC_MAX_TX_RATES,4)];
} tHalRaInfo, *tpHalRaInfo;

typedef PACKED_PRE struct PACKED_POST sBssRaBit {
#ifdef ANI_BIG_BYTE_ENDIAN
    tANI_U32 selfStaIdx:8;
    tANI_U32 reserved1:15;
    tANI_U32 bssIdx:2;
    tANI_U32 fShortSlot:1;
    tANI_U32 rifsMode:1;
    tANI_U32 nonGfPresent:1;
    tANI_U32 ht20Coexist:1;
    tANI_U32 llbCoexist:1;
    tANI_U32 llgCoexist:1;
    tANI_U32 fShortPreamble:1;    // that are part of this BSS
#else
    tANI_U32 fShortPreamble:1;    // that are part of this BSS
    tANI_U32 llgCoexist:1;
    tANI_U32 llbCoexist:1;
    tANI_U32 ht20Coexist:1;
    tANI_U32 nonGfPresent:1;
    tANI_U32 rifsMode:1;
    tANI_U32 fShortSlot:1;
    tANI_U32 bssIdx:2;
    tANI_U32 reserved1:15;
    tANI_U32 selfStaIdx:8;
#endif
} bssRaBit;

typedef PACKED_PRE union PACKED_POST sBssRaParam {
   bssRaBit bit;
   tANI_U32  dword;
} bssRaParam;

typedef PACKED_PRE struct PACKED_POST sHalRaBssInfo {
    bssRaParam  u;
   tRateAdaptSamplingTable bssAutoSampleRateTable[HAL_MAC_MAX_TX_RATES];
} tHalRaBssInfo, *tpHalRaBssInfo;

typedef PACKED_PRE struct PACKED_POST sHalRaGlobalInfo
{
#ifdef ANI_BIG_BYTE_ENDIAN
   tANI_U8   raPerAlgoSelection; /* PER calculation algorithm, based on PPDU, MPDU, or HYBRID */
   tANI_U8   tRate;              /* preferred tertiary rate. used only when rMode=HAL_RETRY_USE_SPECIFIED, set to 0 for other modes */
   tANI_U8   sRate;              /* preferred secondary rate. used only when rMode=HAL_RETRY_USE_SPECIFIED, set to 0 for other modes */
   tANI_U8   rMode;              /* retry mode. see tHalRetryMode */
#else
   tANI_U8   rMode;              /* retry mode. see tHalRetryMode */
   tANI_U8   sRate;              /* preferred secondary rate. used only when rMode=HAL_RETRY_USE_SPECIFIED, set to 0 for other modes */
   tANI_U8   tRate;              /* preferred tertiary rate. used only when rMode=HAL_RETRY_USE_SPECIFIED, set to 0 for other modes */
   tANI_U8   raPerAlgoSelection; /* PER calculation algorithm, based on PPDU, MPDU, or HYBRID */
#endif
#ifdef ANI_BIG_BYTE_ENDIAN
   /* input from cfg */
   tANI_U8   txFailExceptionThreshold; /* when TxCount is frozen, only TX failures will cause exception */
   tANI_U8   protPolicy;  /* cfg.protPolicy update here */
   tANI_U16  rtsThreshold; /* cfg.rtsThreshold update here */
#else
   tANI_U16  rtsThreshold; /* cfg.rtsThreshold update here */
   tANI_U8   protPolicy;  /* cfg.protPolicy update here */
   tANI_U8   txFailExceptionThreshold;
#endif
    /* threshold setting */
#ifdef ANI_BIG_BYTE_ENDIAN
   tANI_U8   failThreshold;         /* Total number of failures to consider it a valid sample */
   tANI_U8   consecFailThreshold;   /* Number of consecutive failures to consider it a valid sample */
   tANI_U8   extraStayIncThreshold; /* In STAY state, increment maxExtraStayPeriods by this value */
   tANI_U8   perIgnoreThreshold;    /* PER below this is equivalent to zero This considered roughly to the expected PER due to collision */
#else
   tANI_U8   perIgnoreThreshold;
   tANI_U8   extraStayIncThreshold;
   tANI_U8   consecFailThreshold;
   tANI_U8   failThreshold;
#endif

#ifdef ANI_BIG_BYTE_ENDIAN
   tANI_U8   perGoodLinkJumpThreshold;   /* a link is "good" if the PER is better than this limit */
   tANI_U8   perGoodLinkSampleThreshold; /* PER drop on a bad link to cause resampling. In STAY state, try to seed up */
   tANI_U8   perBadLinkJumpThreshold;    /* In STAY state, if PER higher than this thresh, start a new sampling */
   tANI_U8   perBadLinkJumpRetryRateThreshold; /* If Primary rate's PER higher than this threshold, consider using its retry rate for sampling */
#else
   tANI_U8   perBadLinkJumpRetryRateThreshold;
   tANI_U8   perBadLinkJumpThreshold;
   tANI_U8   perGoodLinkSampleThreshold;
   tANI_U8   perGoodLinkJumpThreshold;
#endif

#ifdef ANI_BIG_BYTE_ENDIAN
   tANI_U16  badLinkPersistencyThresh;   /* Number of consecutive good samples before considering link is bad */
   tANI_U16  goodLinkPersistencyThresh;  /* Number of consecutive good samples before considering link is good */
#else
   tANI_U16  goodLinkPersistencyThresh;
   tANI_U16  badLinkPersistencyThresh;
#endif
    /* samples's validity */
#ifdef ANI_BIG_BYTE_ENDIAN
   tANI_U16  linkIdleSamples;     /* Number of rate adapt periods before we conclude link is idle (msec) */
   tANI_U8   minPeriodsPerSample; /* number of ra sampling periods before a sample is considered sufficient, count of raPeriod */
   tANI_U8   minTxPerSample;      /* Minimum number of packets before a sample is completed, count of packet */
#else
   tANI_U8   minTxPerSample;      /* Minimum number of packets before a sample is completed */
   tANI_U8   minPeriodsPerSample; /* number of ra sampling periods before a sample is considered sufficient, count of raPeriod */
   tANI_U16  linkIdleSamples;     /* Number of rate adapt periods before we conclude link is idle (msec) */
#endif
#ifdef ANI_BIG_BYTE_ENDIAN
   tANI_U16  quickSamplePeriod;   /* Number of rate adaptation periods before quick sampling */
   tANI_U16  sampleAllPeriod;     /* Number of rate adaptation periods between exhaustive sampling */
#else
   tANI_U16  sampleAllPeriod;
   tANI_U16  quickSamplePeriod;
#endif
#ifdef ANI_BIG_BYTE_ENDIAN
   tANI_U16  raPeriod;              /* The period of collecting the statistics for RA, in msec */
   tANI_U8   higherSensSampleRates; /* the number of sampling rates (in the table) for better rates */
   tANI_U8   lowerSensSampleRates;  /* the number of sampling rates (in the table) for lower rates */
#else
   tANI_U8   lowerSensSampleRates;
   tANI_U8   higherSensSampleRates;
   tANI_U16  raPeriod;
#endif
    /* sensitivity */
#ifdef ANI_BIG_BYTE_ENDIAN
   tANI_U16  retry2SensitivityDiff; /* min diff in sensitivity/10 of selected teritary rates */
   tANI_U16  retry1SensitivityDiff; /* min diff in sensitivity/10 of selected secondary rates */
#else
   tANI_U16  retry1SensitivityDiff;
   tANI_U16  retry2SensitivityDiff;
#endif

#ifdef ANI_BIG_BYTE_ENDIAN
   tANI_S16  lowerRateMinSensDiff;  /* Skip rates whose sensitivities are too close. Only select rates whose sensitivity is at least this value/10 dB.*/
   tANI_S16  betterRateMaxSensDiff; /* max diff in sensitivity/10 for next higher tput rate. Max sens jump happens at  */
#else
   tANI_S16  betterRateMaxSensDiff;
   tANI_S16  lowerRateMinSensDiff;
#endif

#ifdef ANI_BIG_BYTE_ENDIAN
   tANI_U8   min11nRateIdx;            /* Minimum 11N rate for RA to adapt to (in MBps) */
   tANI_U8   min11gRateIdx;            /* Minimum 11G rate for RA to adapt to (in MBps) */
   tANI_U8   min11bRateIdx;            /* Minimum 11B rate for RA to adapt to (in MBps) */
   tANI_U8   rsvd1;
#else
   tANI_U8   rsvd1;
   tANI_U8   min11bRateIdx;            /* Minimum 11B rate for RA to adapt to (in MBps) */
   tANI_U8   min11gRateIdx;
   tANI_U8   min11nRateIdx;
#endif
    /* if more elements are needed, insert here */
    tANI_U8   goodPerThreshBySensitivity[CEIL_ALIGN(RA_GOODPERTHRESH_SENSITIVITY_TABLE_SIZE,4)];
    /* don't append at tail */
} tHalRaGlobalInfo, *tpHalRaGlobalInfo;

/*************************** Soft AP **************************/
// BSS types
typedef enum {
    BSS_TYPE_AP_SELF,   // BSS is for self AP
    BSS_TYPE_AP_PEER,   // BSS is for peer AP
    BSS_TYPE_IBSS,      // BSS is for IBSS
    BSS_TYPE_BTAMP      // BSS is for BTAMP
} tBssType;

// STA types
typedef enum {
    STA_TYPE_STA_SELF,  // Self STA associated to the peer AP in Infra STA mode.
    STA_TYPE_AP_PEER,   // Peer AP to whom Self STA would be associated in Infra STA mode.
    STA_TYPE_AP_SELF,   // Self AP when in Infra AP mode
    STA_TYPE_STA_PEER   // Peer STA associated to AP (self) in Infra AP mode.
} tStaType;


/* Data Structure for storing BSS related parameters in FW sysConfig*/
typedef struct sBssInfo {
#ifdef ANI_BIG_BYTE_ENDIAN
   tANI_U32  valid:1;          // BSS valid bit
   tANI_U32  staId:3;          // Station ID associated with the BSS
   tANI_U32  bcastStaId:3;     // Broadcast station ID associated with the BSS
   tANI_U32  operMode:2;       // 11B, 11G or 11N or Mixed
   tANI_U32  bssType:2;        // Self BSS or Peer AP
   tANI_U32  qosAp:1;          // If QoS is enabled
   tANI_U32  bProxyProbeRespEn :1;        //To Enable Disable Proxy Probe Resp
   tANI_U32  bHiddenSSIDEn     :1;        //To Enable Hidden ssid.
   tANI_U32  ucMaxProbeRespRetryLimit :3; //probe Response Max retries   
   tANI_U32  regFlowCtrlCntr          :13;//To support RegulateFlowcontrol Bail out Logic
   tANI_U32  rsvd1:2;
#else
   tANI_U32  rsvd1:2;
   tANI_U32  regFlowCtrlCntr          :13;//To support RegulateFlowcontrol Bail out Logic
   tANI_U32  ucMaxProbeRespRetryLimit :3; //probe Response Max retries
   tANI_U32  bHiddenSSIDEn     :1;        //To Enable Hidden ssid.      
   tANI_U32  bProxyProbeRespEn :1;        //To Enable Disable Proxy Probe Resp
   tANI_U32  qosAp:1;          // If QoS is enabled
   tANI_U32  bssType:2;        // Self BSS or Peer AP
   tANI_U32  operMode:2;       // 11B, 11G or 11N or Mixed
   tANI_U32  bcastStaId:3;     // Broadcast station ID associated with the BSS
   tANI_U32  staId:3;          // Station ID associated with the BSS
   tANI_U32  valid:1;          // BSS valid bit
#endif

   tANI_U32 dtimIntervalTh;

#ifdef ANI_BIG_BYTE_ENDIAN
   tANI_U32  beaconIntv:16;     // Beacon interval
   tANI_U32  dtimPeriod:8;      // dtim Period
   tANI_U32  maxDtimFrameTx:8;  // Max allowed DTIM bcast frame Tx
#else
   tANI_U32  maxDtimFrameTx:8;  // Max allowed DTIM bcast frame Tx
   tANI_U32  dtimPeriod:8;      // dtim Period
   tANI_U32  beaconIntv:16;     // Beacon interval
#endif
   
    /* BSSID for the BSS */     
   tANI_U32   bssidLo;
#ifdef ANI_BIG_BYTE_ENDIAN     
   tANI_U32   bssidHi:16;      
   tANI_U32   rsvd3:16;
#else
   tANI_U32   rsvd3:16;
   tANI_U32   bssidHi:16;      
#endif


}tBssInfo, *tpBssInfo;

/* Data Structure for storing STA related parameters in sysConfig*/
typedef struct sStaInfo {
#ifdef ANI_BIG_BYTE_ENDIAN
   tANI_U32  valid:1;          // STA valid bit
   tANI_U32  staType:2;        // Station type
   tANI_U32  aid:4;            // association ID
   tANI_U32  isQosSta:1;       // If it is a QoS STA
   tANI_U32  uapsdMask:4;      // UAPSD mask set in the (Re) Assoc req
   tANI_U32  maxSPLen:2;       // Max SP len 
   tANI_U32  listenIntv:8;     // Listen interval
   tANI_U32  bssIdx:2;         // Bss Index that this STA belongs to
   tANI_U32  rsvd:8;
#else
   tANI_U32  rsvd:8;
   tANI_U32  bssIdx:2;         // Bss Index that this STA belongs to
   tANI_U32  listenIntv:8;     // Listen interval
   tANI_U32  maxSPLen:2;       // Max SP len 
   tANI_U32  uapsdMask:4;      // UAPSD mask set in the (Re) Assoc req
   tANI_U32  isQosSta:1;       // If it is a QoS STA
   tANI_U32  aid:4;            // association ID
   tANI_U32  staType:2;        // Station type
   tANI_U32  valid:1;          // STA valid bit
#endif

   /*Mac Addresses*/
   tANI_U32  staMacAddrLo;
#ifdef ANI_BIG_BYTE_ENDIAN
   tANI_U32  staMacAddrHi : 16;
   tANI_U32  bReserved2   : 16;
#else
   tANI_U32  bReserved2   : 16;
   tANI_U32  staMacAddrHi : 16;
#endif

}tStaInfo, *tpStaInfo;




/*===========================================================================
  HW COUNTERS
===========================================================================*/
typedef struct _Qwlanfw_HwCountersStruct {
  tANI_U32 uRxp_Phy_Mpdu_Cnt;
  tANI_U32 uRxp_Dma_Send_Cnt;
  tANI_U32 uRxp_Dma_Drop_Cnt;
  tANI_U32 uRxp_Fcs_Err_Cnt;
  tANI_U32 uRxp_Phy_Err_Drop_Cnt;
  tANI_U32 uDpu_Dpu_txPktCount;
  tANI_U32 uDpu_Dpu_rxPktCount;
  tANI_U32 uDpu_Dpu_Error_WQ_Count;
  tANI_U32 uDxe_Counter0;
  tANI_U32 uDxe_Counter1;
  tANI_U32 uTpe_Unicast_Bytes_Lower;
  tANI_U32 uTpe_Unicast_Bytes_Upper;
  tANI_U32 uTpe_Multicast_Bytes_Lower;
  tANI_U32 uTpe_Multicast_Bytes_Upper;
  tANI_U32 uTpe_Broadcast_Bytes_Lower;
  tANI_U32 uTpe_Broadcast_Bytes_Upper;
  tANI_U32 uTpe_Sw_Life_Time_Drop_Cnt;
  tANI_U32 uAdu_Uma_Tx_Counters1;
  tANI_U32 uAdu_Uma_Tx_Counters2;
  tANI_U32 uAdu_Uma_Llc_Counters;
  tANI_U32 uAdu_Uma_Tx_Counters3;
  tANI_U32 uAdu_Adu_Counters1;
  tANI_U32 uAdu_Adu_Counters2;
  tANI_U32 uAdu_Adu_Counters3;
  tANI_U32 uMtu_Cca_Counter0;
  tANI_U32 uSif_Tx_Rcvd_Frm_Count;
  tANI_U32 uSif_Rx_Frm_Rd_Count;
} Qwlanfw_HwCntrType;

/*===========================================================================
  Phy calibration values
===========================================================================*/
typedef struct _PhyCalCorrStruct {
#ifdef ANI_BIG_BYTE_ENDIAN
   tANI_U32             ucIDcoCorrChain0 : 8;
   tANI_U32             ucQDcoCorrChain0 : 8;
   tANI_U32             ucIDcoCorrChain1 : 8;
   tANI_U32             ucQDcoCorrChain1 : 8;
#else
   tANI_U32             ucQDcoCorrChain1 : 8;
   tANI_U32             ucIDcoCorrChain1 : 8;
   tANI_U32             ucQDcoCorrChain0 : 8;
   tANI_U32             ucIDcoCorrChain0 : 8;
#endif

#ifdef ANI_BIG_BYTE_ENDIAN
   tANI_S32             usITxLoCorrChain0 : 16;
   tANI_S32             usQTxLoCorrChain0 : 16;
#else
   tANI_S32             usQTxLoCorrChain0 : 16;
   tANI_S32             usITxLoCorrChain0 : 16;
#endif

#ifdef ANI_BIG_BYTE_ENDIAN
   tANI_S32             usCenterRxIQCorrChain0    : 16;
   tANI_S32             usOffCenterRxIQCorrChain0 : 16;
#else
   tANI_S32             usOffCenterRxIQCorrChain0 : 16;
   tANI_S32             usCenterRxIQCorrChain0    : 16;
#endif

#ifdef ANI_BIG_BYTE_ENDIAN
   tANI_S32             usImbalanceRxIQCorrChain0 : 16;
   tANI_S32             usCenterRxIQCorrChain1    : 16;
#else
   tANI_S32             usCenterRxIQCorrChain1    : 16;
   tANI_S32             usImbalanceRxIQCorrChain0 : 16;
#endif

#ifdef ANI_BIG_BYTE_ENDIAN
   tANI_S32             usOffCenterRxIQCorrChain1 : 16;
   tANI_S32             usImbalanceRxIQCorrChain1 : 16;
#else
   tANI_S32             usImbalanceRxIQCorrChain1 : 16;
   tANI_S32             usOffCenterRxIQCorrChain1 : 16;
#endif

#ifdef ANI_BIG_BYTE_ENDIAN
   tANI_S32             usCenterTxIQCorrChain0    : 16;
   tANI_S32             usOffCenterTxIQCorrChain0 : 16;
#else
   tANI_S32             usOffCenterTxIQCorrChain0 : 16;
   tANI_S32             usCenterTxIQCorrChain0    : 16;
#endif

#ifdef ANI_BIG_BYTE_ENDIAN
   tANI_S32             usImbalanceTxIQCorrChain0 : 16;
   tANI_S32             usCenterTxIQCorrChain1    : 16;
#else
   tANI_S32             usCenterTxIQCorrChain1    : 16;
   tANI_S32             usImbalanceTxIQCorrChain0 : 16;
#endif

#ifdef ANI_BIG_BYTE_ENDIAN
   tANI_S32             usOffCenterTxIQCorrChain1 : 16;
   tANI_S32             usImbalanceTxIQCorrChain1 : 16;
#else
   tANI_S32             usImbalanceTxIQCorrChain1 : 16;
   tANI_S32             usOffCenterTxIQCorrChain1 : 16;
#endif

#ifdef ANI_BIG_BYTE_ENDIAN
   tANI_U32             ucBw1Chain0 : 8;
   tANI_U32             ucBw2Chain0 : 8;
   tANI_U32             ucBw3Chain0 : 8;
   tANI_U32             ucBw4Chain0 : 8;
#else
   tANI_U32             ucBw4Chain0 : 8;
   tANI_U32             ucBw3Chain0 : 8;
   tANI_U32             ucBw2Chain0 : 8;
   tANI_U32             ucBw1Chain0 : 8;
#endif

#ifdef ANI_BIG_BYTE_ENDIAN
   tANI_U32             ucBw5Chain0 : 8;
   tANI_U32             ucBw6Chain0 : 8;
   tANI_U32             ucBw1Chain1 : 8;
   tANI_U32             ucBw2Chain1 : 8;
#else
   tANI_U32             ucBw2Chain1 : 8;
   tANI_U32             ucBw1Chain1 : 8;
   tANI_U32             ucBw6Chain0 : 8;
   tANI_U32             ucBw5Chain0 : 8;
#endif

#ifdef ANI_BIG_BYTE_ENDIAN
   tANI_U32             ucBw3Chain1 : 8;
   tANI_U32             ucBw4Chain1 : 8;
   tANI_U32             ucBw5Chain1 : 8;
   tANI_U32             ucBw6Chain1 : 8;
#else
   tANI_U32             ucBw6Chain1 : 8;
   tANI_U32             ucBw5Chain1 : 8;
   tANI_U32             ucBw4Chain1 : 8;
   tANI_U32             ucBw3Chain1 : 8;
#endif

} Qwlanfw_PhyCalCorrType;

/* Naming covention: All messages with trailer REQ will
 * have response and vice-versa
 */
/*==================================================================================
  HOST -> FW MESSAGE TYPES
==================================================================================*/
#define QWLANFW_HOST2FW_MSG_TYPES_BEGIN        0x0
/*IMPS*/
#define QWLANFW_HOST2FW_ENTER_IMPS_REQ         QWLANFW_HOST2FW_MSG_TYPES_BEGIN + 0x0
/*BMPS*/
#define QWLANFW_HOST2FW_ENTER_BMPS_REQ         QWLANFW_HOST2FW_MSG_TYPES_BEGIN + 0x1
#define QWLANFW_HOST2FW_EXIT_BMPS_REQ          QWLANFW_HOST2FW_MSG_TYPES_BEGIN + 0x2
#define QWLANFW_HOST2FW_SUSPEND_BMPS_REQ       QWLANFW_HOST2FW_MSG_TYPES_BEGIN + 0x3
#define QWLANFW_HOST2FW_RESUME_BMPS_REQ        QWLANFW_HOST2FW_MSG_TYPES_BEGIN + 0x4
#define QWLANFW_HOST2FW_ENTER_UAPSD_REQ        QWLANFW_HOST2FW_MSG_TYPES_BEGIN + 0x5
#define QWLANFW_HOST2FW_EXIT_UAPSD_REQ         QWLANFW_HOST2FW_MSG_TYPES_BEGIN + 0x6
/* Beacon filtering */
#define QWLANFW_HOST2FW_ADD_BEACON_FILTER      QWLANFW_HOST2FW_MSG_TYPES_BEGIN + 0x7
#define QWLANFW_HOST2FW_REM_BEACON_FILTER      QWLANFW_HOST2FW_MSG_TYPES_BEGIN + 0x8
/* Pattern Matching */
#define QWLANFW_HOST2FW_ADD_PTRN               QWLANFW_HOST2FW_MSG_TYPES_BEGIN + 0x9
#define QWLANFW_HOST2FW_REM_PTRN               QWLANFW_HOST2FW_MSG_TYPES_BEGIN + 0xA
/*Hal Phy*/
#define QWLANFW_HOST2FW_CAL_UPDATE_REQ         QWLANFW_HOST2FW_MSG_TYPES_BEGIN + 0xB
#define QWLANFW_HOST2FW_SET_CHANNEL_REQ        QWLANFW_HOST2FW_MSG_TYPES_BEGIN + 0xC
#define QWLANFW_HOST2FW_SET_CHAIN_SELECT_REQ   QWLANFW_HOST2FW_MSG_TYPES_BEGIN + 0xD
/* BTC */
#define QWLANFW_HOST2FW_BT_EVENT               QWLANFW_HOST2FW_MSG_TYPES_BEGIN + 0xE
/* SCAN */
#define QWLANFW_HOST2FW_SCAN_START             QWLANFW_HOST2FW_MSG_TYPES_BEGIN + 0xF
#define QWLANFW_HOST2FW_SCAN_END               QWLANFW_HOST2FW_MSG_TYPES_BEGIN + 0x10
/* CONNECTION SETUP */
#define QWLANFW_HOST2FW_CONNECTION_SETUP_START QWLANFW_HOST2FW_MSG_TYPES_BEGIN + 0x11
#define QWLANFW_HOST2FW_CONNECTION_SETUP_END   QWLANFW_HOST2FW_MSG_TYPES_BEGIN + 0x12
/* CONNECTION TERMINATED */
#define QWLANFW_HOST2FW_CONNECTION_NONE        QWLANFW_HOST2FW_MSG_TYPES_BEGIN + 0x13
/* RA */
#define QWLANFW_HOST2FW_RA_UPDATE              QWLANFW_HOST2FW_MSG_TYPES_BEGIN + 0x14

#ifdef WLAN_SOFTAP_FEATURE
/* SoftAP */
#define QWLANFW_HOST2FW_ADD_BSS                QWLANFW_HOST2FW_MSG_TYPES_BEGIN + 0x15
#define QWLANFW_HOST2FW_SEND_BEACON            QWLANFW_HOST2FW_MSG_TYPES_BEGIN + 0x16
#define QWLANFW_HOST2FW_ADD_STA                QWLANFW_HOST2FW_MSG_TYPES_BEGIN + 0x17
#define QWLANFW_HOST2FW_DEL_STA                QWLANFW_HOST2FW_MSG_TYPES_BEGIN + 0x18
#define QWLANFW_HOST2FW_DEL_BSS                QWLANFW_HOST2FW_MSG_TYPES_BEGIN + 0x19
#define QWLANFW_HOST2FW_SAP_UTIL_CMD           QWLANFW_HOST2FW_MSG_TYPES_BEGIN + 0x1A
#define QWLANFW_HOST2FW_UPDATE_PROBE_RESPONSE_TEMPLATE_REQ      QWLANFW_HOST2FW_MSG_TYPES_BEGIN + 0x1B
#define QWLANFW_HOST2FW_UPDATE_BA              QWLANFW_HOST2FW_MSG_TYPES_BEGIN + 0x1C

#ifdef LIBRA_WAPI_SUPPORT
/* WAPI */
#define QWLANFW_HOST2FW_WPI_KEY_SET            QWLANFW_HOST2FW_MSG_TYPES_BEGIN + 0x1D
#define QWLANFW_HOST2FW_WPI_KEY_REMOVED        QWLANFW_HOST2FW_MSG_TYPES_BEGIN + 0x1E
#define QWLANFW_HOST2FW_MSG_TYPES_END          QWLANFW_HOST2FW_WPI_KEY_REMOVED
#else
#define QWLANFW_HOST2FW_MSG_TYPES_END          QWLANFW_HOST2FW_UPDATE_BA
#endif

#else

#ifdef LIBRA_WAPI_SUPPORT
/* WAPI */
#define QWLANFW_HOST2FW_WPI_KEY_SET            QWLANFW_HOST2FW_MSG_TYPES_BEGIN + 0x15
#define QWLANFW_HOST2FW_WPI_KEY_REMOVED        QWLANFW_HOST2FW_MSG_TYPES_BEGIN + 0x16
#define QWLANFW_HOST2FW_MSG_TYPES_END          QWLANFW_HOST2FW_WPI_KEY_REMOVED
#else
#define QWLANFW_HOST2FW_MSG_TYPES_END          QWLANFW_HOST2FW_RA_UPDATE
#endif

#endif

/* Note: We can't define HOST2FW_MSG_TYPES_XXX beyond 0x80 (127) */
/*==================================================================================
  FW -> HOST MESSAGE TYPES
==================================================================================*/
/* CR#xxxxxx: 
   Single host driver with both SOFTAP and WAPI features wants to download either SOFTAP+WAPI firmware or only SOFTAP enabled firmware
   By making  FW2HOST_MSG_TYPES_BEGIN independent of HOST2FW_MSG_END, 
   we can always receives FW2HOST_MSG correctly (including FW2HOST_STATUS(INIT_DONE) message)
   no matter how many messages are defined in the HOST2FW for different feature set. */
//#define QWLANFW_FW2HOST_MSG_TYPES_BEGIN        QWLANFW_HOST2FW_MSG_TYPES_END   + 0x1 /* original before CR#xxxxx */
/* Initially, we decided to reserve half of message pool for HOST2FW message, and rest half for FW2HOST message. 
   Currently 10 bit is reserved for usMsgType (see Qwlanfw_CtrlMsgType)
*/   
//#define QWLANFW_FW2HOST_MSG_TYPES_BEGIN        0x200  /* 512 */
/* 512 is the half of the message pool for 10 bits, but in halFwApi.c, halFW_SendMsg(…, tANI_U8 msgType, …) has only one byte of msgType. 
I think this needs to be discussed in the team.
Either argument of halFW_SendMsg() should be uint16 to have HOST2FW_MSG up to 512, or 
we should redefine the Qwlanfw_CtrlMsgType->ubMsgType to uint8.
Because of above reason and for smallest change, I reserved 128 for HOST2FW, and 128 for FW2HOST for now */ 
#define QWLANFW_FW2HOST_MSG_TYPES_BEGIN        0x80   /* 128 */

/*Status*/
#define QWLANFW_FW2HOST_STATUS                 QWLANFW_FW2HOST_MSG_TYPES_BEGIN + 0x0
/*IMPS*/
#define QWLANFW_FW2HOST_ENTER_IMPS_RSP         QWLANFW_FW2HOST_MSG_TYPES_BEGIN + 0x1
#define QWLANFW_FW2HOST_IMPS_EXITED            QWLANFW_FW2HOST_MSG_TYPES_BEGIN + 0x2
/*BMPS*/
#define QWLANFW_FW2HOST_ENTER_BMPS_RSP         QWLANFW_FW2HOST_MSG_TYPES_BEGIN + 0x3
#define QWLANFW_FW2HOST_EXIT_BMPS_RSP          QWLANFW_FW2HOST_MSG_TYPES_BEGIN + 0x4
#define QWLANFW_FW2HOST_SUSPEND_BMPS_RSP       QWLANFW_FW2HOST_MSG_TYPES_BEGIN + 0x5
#define QWLANFW_FW2HOST_RESUME_BMPS_RSP        QWLANFW_FW2HOST_MSG_TYPES_BEGIN + 0x6
#define QWLANFW_FW2HOST_ENTER_UAPSD_RSP        QWLANFW_FW2HOST_MSG_TYPES_BEGIN + 0x7
#define QWLANFW_FW2HOST_EXIT_UAPSD_RSP         QWLANFW_FW2HOST_MSG_TYPES_BEGIN + 0x8
/*Hal Phy*/
#define QWLANFW_FW2HOST_CAL_UPDATE_RSP         QWLANFW_FW2HOST_MSG_TYPES_BEGIN + 0x9
#define QWLANFW_FW2HOST_PERFORM_PERIODIC_CAL   QWLANFW_FW2HOST_MSG_TYPES_BEGIN + 0xA
#define QWLANFW_FW2HOST_SET_CHANNEL_RSP        QWLANFW_FW2HOST_MSG_TYPES_BEGIN + 0xB
#define QWLANFW_FW2HOST_SET_CHAIN_SELECT_RSP   QWLANFW_FW2HOST_MSG_TYPES_BEGIN + 0xC
/*RSSI*/
#define QWLANFW_FW2HOST_RSSI_NOTIFICATION      QWLANFW_FW2HOST_MSG_TYPES_BEGIN + 0xD
/* RA */
#define QWLANFW_FW2HOST_RA_NOTIFICATOIN        QWLANFW_FW2HOST_MSG_TYPES_BEGIN + 0xE

#ifdef WLAN_SOFTAP_FEATURE
/* Keepalive / Unknown A2 / Tim-based dis */
#define QWLANFW_FW2HOST_DEL_STA_CONTEXT        QWLANFW_FW2HOST_MSG_TYPES_BEGIN + 0xF
#define QWLANFW_FW2HOST_MSG_TYPES_END          QWLANFW_FW2HOST_DEL_STA_CONTEXT
#else
#define QWLANFW_FW2HOST_MSG_TYPES_END          QWLANFW_FW2HOST_RA_NOTIFICATOIN
#endif

/*==================================================================================
  HOST -> FW MESSAGE ENUMS
==================================================================================*/

/* Enumeration of all the different kinds of BT events */
/* matched to tSmeBtEventType */
typedef enum
{
  QWLANFW_BT_EVENT_DEVICE_SWITCHED_ON = 0,
  QWLANFW_BT_EVENT_DEVICE_SWITCHED_OFF,
  QWLANFW_BT_EVENT_INQUIRY_STARTED,
  QWLANFW_BT_EVENT_INQUIRY_STOPPED,
  QWLANFW_BT_EVENT_INQUIRY_SCAN_STARTED,
  QWLANFW_BT_EVENT_INQUIRY_SCAN_STOPPED,
  QWLANFW_BT_EVENT_PAGE_STARTED,
  QWLANFW_BT_EVENT_PAGE_STOPPED,
  QWLANFW_BT_EVENT_PAGE_SCAN_STARTED,
  QWLANFW_BT_EVENT_PAGE_SCAN_STOPPED,
  QWLANFW_BT_EVENT_CREATE_ACL_CONNECTION,
  QWLANFW_BT_EVENT_ACL_CONNECTION_COMPLETE,
  QWLANFW_BT_EVENT_CREATE_SYNC_CONNECTION,
  QWLANFW_BT_EVENT_SYNC_CONNECTION_COMPLETE,
  QWLANFW_BT_EVENT_SYNC_CONNECTION_UPDATED,
  QWLANFW_BT_EVENT_DISCONNECTION_COMPLETE,
  QWLANFW_BT_EVENT_MODE_CHANGED,
  QWLANFW_BT_EVENT_A2DP_STREAM_START,
  QWLANFW_BT_EVENT_A2DP_STREAM_STOP,
  QWLANFW_BT_EVENT_TYPE_MAX    //This and beyond are invalid values
} tFwBtEventEnum;

/*==================================================================================
  HOST -> FW BTC DEFINES
  these need to match the corresponding defintions in SME (btcApi.h)
==================================================================================*/

#define QWLANFW_BT_INVALID_CONN_HANDLE   (0xFFFF)  /**< Invalid connection handle */

/** ACL and Sync connection attempt results */
#define QWLANFW_BT_CONN_STATUS_FAIL      (0)  /**< Connection failed */
#define QWLANFW_BT_CONN_STATUS_SUCCESS   (1)  /**< Connection successful */
#define	QWLANFW_BT_CONN_STATUS_MAX       (2)  /**< this and beyond are invalid values */

/** ACL and Sync link types
    These must match the Bluetooth Spec!
*/
#define QWLANFW_BT_SCO                   (0)   /**< SCO Link */
#define QWLANFW_BT_ACL                   (1)   /**< ACL Link */
#define QWLANFW_BT_eSCO                  (2)   /**< eSCO Link */
#define QWLANFW_BT_LINK_TYPE_MAX         (3)   /**< This value and higher are invalid */

/** ACL link modes
    These must match the Bluetooth Spec!
*/
#define QWLANFW_BT_ACL_ACTIVE            (0)   /**< Active mode */
#define QWLANFW_BT_ACL_HOLD              (1)   /**< Hold mode */
#define QWLANFW_BT_ACL_SNIFF             (2)   /**< Sniff mode */
#define QWLANFW_BT_ACL_PARK              (3)   /**< Park mode */
#define QWLANFW_BT_ACL_MODE_MAX          (4)   /**< This value and higher are invalid */

/** BTC Executions Modes allowed to be set by user
*/
#define QWLANFW_BTC_SMART_COEXISTENCE    (0)   /** BTC Mapping Layer decides whats best, balanced */
#define	QWLANFW_BTC_WLAN_ONLY            (1)   /** WLAN takes all mode */
#define	QWLANFW_BTC_PTA_ONLY             (2)   /** Allow only 3 wire protocol in H/W */
#define QWLANFW_BTC_SMART_MAX_WLAN       (3)   /** BTC Mapping Layer decides whats best, WLAN weighted */
#define QWLANFW_BTC_SMART_MAX_BT         (4)   /** BTC Mapping Layer decides whats best, BT weighted */
#define QWLANFW_BTC_SMART_BT_A2DP        (5)   /** BTC Mapping Layer decides whats best, balanced + BT A2DP weight */
#define	QWLANFW_BT_EXEC_MODE_MAX         (6)   /** This and beyond are invalid values */

/** Enumeration of different kinds actions that BTC Mapping Layer
    can do if PM indication (to AP) fails. */
#define QWLANFW_BTC_RESTART_CURRENT      (0) /** Restart the interval we just failed to leave */
#define QWLANFW_BTC_START_NEXT           (1) /** Start the next interval even though the */

/*==================================================================================
  HOST -> FW MESSAGE DEFINITIONS
==================================================================================*/
/**
   @brief
    QWLANFW_HOST2FW_ENTER_IMPS_REQ
 */
typedef  PACKED_PRE struct PACKED_POST _Qwlanfw_EnterImpsReqStruct
{
   Qwlanfw_CtrlMsgType  hdr;
   /* Currently nothing to pass to FW */
} Qwlanfw_EnterImpsReqType;

/**
   @brief
    QWLANFW_HOST2FW_ENTER_BMPS_REQ

 */
typedef  PACKED_PRE struct PACKED_POST _Qwlanfw_EnterBmpsReqStruct
{
   Qwlanfw_CtrlMsgType  hdr;
   tANI_U32 timestampLo; /* Timestamp in last beacon received from AP */
   tANI_U32 timestampHi;
   tANI_U32 hwTimestampLo; /* Timestamp in last beacon received from AP */
   tANI_U32 hwTimestampHi;
} Qwlanfw_EnterBmpsReqType;

/**
   @brief
    QWLANFW_HOST2FW_EXIT_BMPS_REQ
 */
typedef  PACKED_PRE struct PACKED_POST _Qwlanfw_ExitBmpsReqStruct
{
   Qwlanfw_CtrlMsgType  hdr;
   /** This flag indicates if FW has to send NullData
       frame (with PS=0) to exit BMPS mode.
    */
#ifdef ANI_BIG_BYTE_ENDIAN
   tANI_U32             bExitDot11PwrSave : 1;
   tANI_U32             bReserved : 31;
#else
   tANI_U32             bReserved         : 31;
   tANI_U32             bExitDot11PwrSave : 1;
#endif
} Qwlanfw_ExitBmpsReqType;

/**
   @brief
    QWLANFW_HOST2FW_SUSPEND_BMPS_REQ
 */
typedef  PACKED_PRE struct PACKED_POST _Qwlanfw_SuspendBmpsReqStruct
{
   Qwlanfw_CtrlMsgType  hdr;
   /*Currently nothing to pass to FW */
} Qwlanfw_SuspendBmpsReqType;

/**
   @brief
    QWLANFW_HOST2FW_RESUME_BMPS_REQ
 */
typedef  PACKED_PRE struct PACKED_POST _Qwlanfw_ResumeBmpsReqStruct
{
   Qwlanfw_CtrlMsgType  hdr;
   /*Currently nothing to pass to FW */
} Qwlanfw_ResumeBmpsReqType;

/**
   @brief
    QWLANFW_HOST2FW_ENTER_UAPSD
 */
typedef  PACKED_PRE struct PACKED_POST _Qwlanfw_EnterUapsdStruct
{
   Qwlanfw_CtrlMsgType  hdr;
#ifdef WLAN_SOFTAP_FEATURE   
   tANI_U32 staIdx;
   tANI_U32 maxSPLen;
#endif

#ifdef ANI_BIG_BYTE_ENDIAN
   tANI_U32             bBcAcDeliveryEnable : 1;
   tANI_U32             bBeAcDeliveryEnable : 1;
   tANI_U32             bViAcDeliveryEnable : 1;
   tANI_U32             bVoAcDeliveryEnable : 1;
   tANI_U32             bBcAcTriggerEnable : 1;
   tANI_U32             bBeAcTriggerEnable : 1;
   tANI_U32             bViAcTriggerEnable : 1;
   tANI_U32             bVoAcTriggerEnable : 1;
   tANI_U32             bReserved : 24;
#else
   tANI_U32             bReserved : 24;
   tANI_U32             bVoAcTriggerEnable : 1;
   tANI_U32             bViAcTriggerEnable : 1;
   tANI_U32             bBeAcTriggerEnable : 1;
   tANI_U32             bBcAcTriggerEnable : 1;
   tANI_U32             bVoAcDeliveryEnable : 1;
   tANI_U32             bViAcDeliveryEnable : 1;
   tANI_U32             bBeAcDeliveryEnable : 1;
   tANI_U32             bBcAcDeliveryEnable : 1;
#endif


} Qwlanfw_EnterUapsdType;

/**
   @brief
    QWLANFW_HOST2FW_ENTER_UAPSD
 */
typedef  PACKED_PRE struct PACKED_POST _Qwlanfw_ExitUapsdStruct
{
   Qwlanfw_CtrlMsgType  hdr;
#ifdef WLAN_SOFTAP_FEATURE
   tANI_U32 staIdx;
#endif

} Qwlanfw_ExitUpasdType;

/**
   @brief
    Beacon filter structure
 */
typedef  PACKED_PRE struct PACKED_POST _Qwlanfw_BcnIeChangeFilterStruct
{
#ifdef ANI_BIG_BYTE_ENDIAN
   tANI_U32   ucIeElementId  : 8;
   /* If usFilterLength=0 then indicate if IE is present*/
   tANI_U32   usFilterLength : 16; /* Needs to be multiple of 4 bytes */
   tANI_U32   uReserved      : 8;
#else
   tANI_U32   uReserved      : 8;
   tANI_U32   usFilterLength : 16; /* Needs to be multiple of 4 bytes */
   tANI_U32   ucIeElementId  : 8;
#endif
   /* This is followed by {ByteOffset, ByteValue, ByteMask, ByteRef}
    * for the bytes which require tracking in the IEs.
    * ByteOffset -> Offset of the byte from the start of IE
    * ByteValue  -> The value which is taken as reference
    * BitMask   -> Mask to say what bits to look into
    * ByteRef    -> ByteRef ? Take ByteValue as Ref : Take next value as Ref
    */

} Qwlanfw_BcnIeChangeFilterType;

typedef  PACKED_PRE struct PACKED_POST _Qwlanfw_BcnIeByteStruct
{
#ifdef ANI_BIG_BYTE_ENDIAN
   tANI_U32 ucByteOffset : 8;
   tANI_U32 ucByteValue  : 8;
   tANI_U32 ucBitMask    : 8;
   tANI_U32 ucByteRef    : 8;
#else
   tANI_U32 ucByteRef    : 8;
   tANI_U32 ucBitMask    : 8;
   tANI_U32 ucByteValue  : 8;
   tANI_U32 ucByteOffset : 8;
#endif
} Qwlanfw_BcnIeByteType;
/**
   @brief
    QWLANFW_HOST2FW_ADD_BEACON_FILTER
 */
typedef  PACKED_PRE struct PACKED_POST _Qwlanfw_AddBcnFilterMsgStruct
{
   Qwlanfw_CtrlMsgType  hdr;
#ifdef ANI_BIG_BYTE_ENDIAN
   tANI_U32             usCapInfoFieldValue : 16;
   tANI_U32             usCapInfoFieldMask  : 16;
#else
   tANI_U32             usCapInfoFieldMask  : 16;
   tANI_U32             usCapInfoFieldValue : 16;
#endif

#ifdef ANI_BIG_BYTE_ENDIAN
   tANI_U32             bcninterval : 16;
   tANI_U32             usReserved  : 16;
#else
   tANI_U32             usReserved  : 16;
   tANI_U32             bcninterval : 16;
#endif
   /* This is followed by a list of Qwlanfw_BeaconIeChangeFilterType
    * structures. If the length of the message is four bytes then no
    *  filters are sent.
    */
} Qwlanfw_AddBcnFilterMsgType;

/**
   @brief
    QWLANFW_HOST2FW_REM_BEACON_FILTER
 */
typedef  PACKED_PRE struct PACKED_POST _Qwlanfw_RemBcnFilterMsgStruct
{
   Qwlanfw_CtrlMsgType  hdr;
#ifdef ANI_BIG_BYTE_ENDIAN
   tANI_U32             ucNumIds : 8;
   tANI_U32             bReserved : 24;
#else
   tANI_U32             bReserved : 24;
   tANI_U32             ucNumIds  : 8;
#endif
   tANI_U8              ucRemIeId[QWLANFW_MAX_BCN_FILTER];
} Qwlanfw_RemBcnFilterMsgType;

/**
   @brief
    Pattern for Netwakeup feature
 */
typedef  PACKED_PRE struct PACKED_POST _Qwlanfw_MatchPtrnStruct
{
#ifdef ANI_BIG_BYTE_ENDIAN
   /*This is the ID for this pattern and this is used in Remove pattern mesg*/
   tANI_U32    usPtrnId            : 8;
   /* Specifies in bytes the offset from the beginning of the 802.11 packet to the
    * start of the wake-up pattern.
    */
   tANI_U32    usPatternByteOffset : 8;
   /* Specifies in bytes the size of pattern which immediately follows end of this
    * structure. Must be non-zero
    */
   tANI_U32    ucPatternSize       : 8;
   /* Specifies in bytes the size of pattern which immediately follows end of this
    * structure. Must be non-zero
    */
   tANI_U32    ucPatternMaskSize   : 8;
   /* This is followed by actual pattern and the mask for that pattern */
#else
   tANI_U32    ucPatternMaskSize   : 8;
   tANI_U32    ucPatternSize       : 8;
   tANI_U32    usPatternByteOffset : 8;
   tANI_U32    usPtrnId            : 8;
#endif
} Qwlanfw_MatchPtrnType;

/**
   @brief
    QWLANFW_HOST2FW_ADD_MATCH_PTRN
 */
typedef  PACKED_PRE struct PACKED_POST _Qwlanfw_AddMatchPtrnMsgStruct
{
   Qwlanfw_CtrlMsgType  hdr;
#ifdef ANI_BIG_BYTE_ENDIAN
   tANI_U32             ucNumPtrns : 8;
   tANI_U32             bReserved : 24;
#else
   tANI_U32             bReserved  : 24;
   tANI_U32             ucNumPtrns : 8;
#endif
   /* This is followed by the patterns defined in the above format*/
} Qwlanfw_AddMatchPtrnMsgType;

/**
   @brief
    QWLANFW_HOST2FW_REM_MATCH_PTRN
 */
typedef  PACKED_PRE struct PACKED_POST _Qwlanfw_RemMatchPtrnMsgStruct
{
   Qwlanfw_CtrlMsgType  hdr;
#ifdef ANI_BIG_BYTE_ENDIAN
   tANI_U32             ucNumPtrns : 8;
   tANI_U32             bReserved : 24;
#else
   tANI_U32             bReserved  : 24;
   tANI_U32             ucNumPtrns : 8;
#endif
   tANI_U8              aPtrnId[QWLANFW_MAX_MATCH_PTRNS];
   /* This is followed by the patterns defined in the above format*/
} Qwlanfw_RemMatchPtrnMsgType;

/**
   @brief
    QWLANFW_HOST2FW_CAL_UPDATE_REQ
 */
typedef  PACKED_PRE struct PACKED_POST _Qwlanfw_CalUpdateReqStruct
{
   Qwlanfw_CtrlMsgType  hdr;
   tANI_U32             usCalId;
   tANI_U32             usPeriodic;
} Qwlanfw_CalUpdateReqType;

/**
   @brief
    QWLANFW_HOST2FW_SET_CHANNEL_REQ
 */
typedef  PACKED_PRE struct PACKED_POST _Qwlanfw_SetChannelReqStruct
{
   Qwlanfw_CtrlMsgType  hdr;
   tANI_U32             usChanNum;          //channel number 1 to 14 for 2.4GHz
   tANI_U32             ucCbState;          // always 0 for 20MHz
   tANI_U32             ucRegDomain;        // enum according to eRegDomainId
   tANI_U32             ucCalRequired;      // flag to decide whether cal is needed or not.
} Qwlanfw_SetChannelReqType;

/**
@brief
QWLANFW_HOST2FW_SET_CHAIN_SELECT_REQ
*/
typedef  PACKED_PRE struct PACKED_POST _Qwlanfw_SetChainSelectReqStruct
{
   Qwlanfw_CtrlMsgType  hdr;
   tANI_U32             uPhyChainSelections;
} Qwlanfw_SetChainSelectReqType;


typedef  PACKED_PRE struct PACKED_POST _Qwlanfw_ConnectionSetupStartStruct
{
   Qwlanfw_CtrlMsgType  hdr;
   /* Currently nothing to pass to FW */
} Qwlanfw_ConnectionSetupStartType;

typedef  PACKED_PRE struct PACKED_POST _Qwlanfw_ConnectionSetupEndStruct
{
   Qwlanfw_CtrlMsgType  hdr;
   /* Currently nothing to pass to FW */
} Qwlanfw_ConnectionSetupEndType;

typedef  PACKED_PRE struct PACKED_POST _Qwlanfw_ScanStartStruct
{
   Qwlanfw_CtrlMsgType  hdr;
   /* Currently nothing to pass to FW */
} Qwlanfw_ScanStartType;

typedef  PACKED_PRE struct PACKED_POST _Qwlanfw_ScanEndStruct
{
   Qwlanfw_CtrlMsgType  hdr;
   /* Currently nothing to pass to FW */
} Qwlanfw_ScanEndType;

typedef  PACKED_PRE struct PACKED_POST _Qwlanfw_ConnectionTerminatedStruct
{
   Qwlanfw_CtrlMsgType  hdr;
   /* Currently nothing to pass to FW */
} Qwlanfw_ConnectionTerminatedType;


/**
   @brief
    Substructure for QWLANFW_HOST2FW_BT_EVENT message.

    Covers these event types:
    QWLANFW_BT_EVENT_CREATE_ACL_CONNECTION
    QWLANFW_BT_EVENT_ACL_CONNECTION_COMPLETE
 */
typedef PACKED_PRE struct PACKED_POST _Qwlanfw_BtAclConnectionParamStruct
{
#ifdef ANI_BIG_BYTE_ENDIAN
   tANI_U32 connectionHandle     : 16;
   tANI_U32 status               : 8;
   tANI_U32 reserved             : 8;
#else
   tANI_U32 reserved             : 8;
   tANI_U32 status               : 8;
   tANI_U32 connectionHandle     : 16;
#endif

#ifdef ANI_BIG_BYTE_ENDIAN
   tANI_U8 bdAddr0;
   tANI_U8 bdAddr1;
   tANI_U8 bdAddr2;
   tANI_U8 bdAddr3;
#else
   tANI_U8 bdAddr3;
   tANI_U8 bdAddr2;
   tANI_U8 bdAddr1;
   tANI_U8 bdAddr0;
#endif

#ifdef ANI_BIG_BYTE_ENDIAN
   tANI_U8 bdAddr4;
   tANI_U8 bdAddr5;
   tANI_U8 reserved2;
   tANI_U8 reserved3;
#else
   tANI_U8 reserved3;
   tANI_U8 reserved2;
   tANI_U8 bdAddr5;
   tANI_U8 bdAddr4;
#endif

} Qwlanfw_BtAclConnectionParamType;

/**
   @brief
    Substructure for QWLANFW_HOST2FW_BT_EVENT message.

    Covers these event types:
    QWLANFW_BT_EVENT_CREATE_SYNC_CONNECTION
    QWLANFW_BT_EVENT_SYNC_CONNECTION_COMPLETE
    QWLANFW_BT_EVENT_SYNC_CONNECTION_UPDATED
 */
typedef PACKED_PRE struct PACKED_POST _Qwlanfw_BtSyncConnectionParamStruct
{
#ifdef ANI_BIG_BYTE_ENDIAN
   tANI_U32 connectionHandle     : 16;
   tANI_U32 reserved             : 16;
#else
   tANI_U32 reserved             : 16;
   tANI_U32 connectionHandle     : 16;
#endif

   tANI_U32 linkType;

#ifdef ANI_BIG_BYTE_ENDIAN
   tANI_U32 status               : 8;
   tANI_U32 scoInterval          : 8;
   tANI_U32 scoWindow            : 8;
   tANI_U32 retransmisisonWindow : 8;
#else
   tANI_U32 retransmisisonWindow : 8;
   tANI_U32 scoWindow            : 8;
   tANI_U32 scoInterval          : 8;
   tANI_U32 status               : 8;
#endif

#ifdef ANI_BIG_BYTE_ENDIAN
   tANI_U8 bdAddr0;
   tANI_U8 bdAddr1;
   tANI_U8 bdAddr2;
   tANI_U8 bdAddr3;
#else
   tANI_U8 bdAddr3;
   tANI_U8 bdAddr2;
   tANI_U8 bdAddr1;
   tANI_U8 bdAddr0;
#endif

#ifdef ANI_BIG_BYTE_ENDIAN
   tANI_U8 bdAddr4;
   tANI_U8 bdAddr5;
   tANI_U8 reserved2;
   tANI_U8 reserved3;
#else
   tANI_U8 reserved3;
   tANI_U8 reserved2;
   tANI_U8 bdAddr5;
   tANI_U8 bdAddr4;
#endif

} Qwlanfw_BtSyncConnectionParamType;

/**
   @brief
    Substructure for QWLANFW_HOST2FW_BT_EVENT message.

    Covers these event types:
    QWLANFW_BT_EVENT_MODE_CHANGED
 */
typedef PACKED_PRE struct PACKED_POST _Qwlanfw_BtAclModeChangeParamStruct
{
#ifdef ANI_BIG_BYTE_ENDIAN
   tANI_U32 connectionHandle : 16;
   tANI_U32 reserved         : 16;
#else
   tANI_U32 reserved         : 16;
   tANI_U32 connectionHandle : 16;
#endif

   tANI_U32 mode;
} Qwlanfw_BtAclModeChangeParamType;

/**
   @brief
    Substructure for QWLANFW_HOST2FW_BT_EVENT message.

    Covers these event types:
    QWLANFW_BT_EVENT_DISCONNECTION_COMPLETE
 */
typedef PACKED_PRE struct PACKED_POST _Qwlanfw_BtDisconnectParamStruct
{
#ifdef ANI_BIG_BYTE_ENDIAN
   tANI_U32 connectionHandle : 16;
   tANI_U32 reserved         : 16;
#else
   tANI_U32 reserved         : 16;
   tANI_U32 connectionHandle : 16;
#endif

} Qwlanfw_BtDisconnectParamType;

/**
   @brief
    QWLANFW_HOST2FW_BT_EVENT
 */
typedef PACKED_PRE struct PACKED_POST _Qwlanfw_BtEventMsgStruct
{
   Qwlanfw_CtrlMsgType  hdr;
   tANI_U32 btEventType;

   /* BT event struct depending on btEventType goes here */
} Qwlanfw_BtEventMsgType;

/* Enumeration of all the different kinds of RA Update events */
typedef enum
{
  QWLANFW_RA_UPDATE_STATS_ADAPT = 0,
  QWLANFW_RA_UPDATE_ADD_BSS = 1,    /* RA_TODO: not supported yet. only one bss; to reduce the transaction, ADD_STA include bss information, too*/
  QWLANFW_RA_UPDATE_DEL_BSS,        /* RA_TODO: not supported yet. only one bss */
  QWLANFW_RA_UPDATE_ADD_STA,        /* currently this will trigger the timer in FW */
  QWLANFW_RA_UPDATE_DEL_STA,
  QWLANFW_RA_UPDATE_TIMER,
  QWLANFW_RA_UPDATE_PARAM,
  QWLANFW_RA_FORCE_STA_RATE,
  QWLANFW_RA_UPDATE_TYPE_MAX    //This and beyond are invalid values
} tFwRaUpdateEnum;

typedef PACKED_PRE struct PACKED_POST _Qwlanfw_RaAddBssStruct
{
//  tANI_U32  bssIdx;   /* valid range: 0~1 */
#ifdef ANI_BIG_BYTE_ENDIAN
   tANI_U32   bssIdx:8;
   tANI_U32   selfStaIdx:8;
   tANI_U32   reserved1:16;
#else
   tANI_U32   reserved1:16;
   tANI_U32   selfStaIdx:8;
   tANI_U32   bssIdx:8;
#endif
} Qwlanfw_RaAddBssMsgType;

typedef PACKED_PRE struct PACKED_POST _Qwlanfw_RaDelBssStruct
{
   tANI_U32  bssIdx;   /* valid range: 0~1 */
} Qwlanfw_RaDelBssMsgType;

typedef PACKED_PRE struct PACKED_POST _Qwlanfw_RaAddStaStruct
{
   tANI_U32   staIdx;   /* valid range: 0~7 */
/* use below in the future in case compaction is required
#ifdef ANI_BIG_BYTE_ENDIAN
   tANI_U32   staIdx:8;
   tANI_U32   reserved1:24;
#else
   tANI_U32   reserved1:24;
   tANI_U32   staIdx:8;
#endif
*/
   // add more, but be careful about endian
} Qwlanfw_RaAddStaMsgType;

typedef PACKED_PRE struct PACKED_POST _Qwlanfw_RaDelStaStruct
{
   tANI_U32   staIdx;   /* valid range: 0~7 */
/* use below in the future in case compaction is required
#ifdef ANI_BIG_BYTE_ENDIAN
   tANI_U32   staIdx:8;
   tANI_U32   reserved1:24;
#else
   tANI_U32   reserved1:24;
   tANI_U32   staIdx:8;
#endif
*/
   // add more, but be careful about endian
} Qwlanfw_RaDelStaMsgType;

#define RA_UPDATE_TIMER_START    1
#define RA_UPDATE_TIMER_STOP     2
#define RA_UPDATE_TIMER_PERIOD   3
typedef PACKED_PRE struct PACKED_POST _Qwlanfw_RaTimerStruct
{
// tANI_U32  raPeriod;  /* valid range: 20~60000 msec */
#ifdef ANI_BIG_BYTE_ENDIAN
   tANI_U32   raPeriod:16;
   tANI_U32   raTimerCntrl:16;
#else
   tANI_U32   raTimerCntrl:16;
   tANI_U32   raPeriod:16;
#endif
   // add more, but be careful about endian
} Qwlanfw_RaTimerMsgType;

/* bitmap for which parameter is updated */
#define RA_UPDATE_BSS_INFO      (1 << 0)
#define RA_UPDATE_TPE_INFO      (1 << 1)
#define RA_UPDATE_RATE_INFO     (1 << 2)
#define RA_UPDATE_TXPWR_INFO    (1 << 3)
#define RA_UPDATE_GLOBAL_INFO   (1 << 4)
#define RA_UPDATE_STA_INFO      (1 << 5)

typedef PACKED_PRE struct PACKED_POST _Qwlanfw_RaUpdateParamStruct
{
   tANI_U32   bmCode;           /* bitmap code to which paramter is modified */
   tANI_U32   paramSpecific;    /* parameter specific information */
   // add more, but be careful about endian
} Qwlanfw_RaUpdateParamMsgType;

typedef PACKED_PRE struct PACKED_POST _Qwlanfw_RaForceStaRateStruct
{
   tANI_U32   staId;    /* valid range: 0~7 */
   tANI_U32   rateType; /* valid range: 0-2 */
   tANI_U32   priRateIdx; /* valid range: 0-43 */
   tANI_U32   secRateIdx; /* valid range: 0-43 */
   tANI_U32   terRateIdx; /* valid range: 0-43 */
   // add more, but be careful about endian
} Qwlanfw_RaForceStaRateMsgType;


typedef PACKED_PRE struct PACKED_POST _Qwlanfw_RaUpdateStruct
{
   Qwlanfw_CtrlMsgType  hdr;
#ifdef ANI_BIG_BYTE_ENDIAN
   tANI_U16 raUpdateType;     /* note: currently, only max. 65536 different event are reserved */
   tANI_U16 raUpdateMsgLen;   /* note: currently, only max. max msg length = 65536 */
#else
   tANI_U16 raUpdateMsgLen;   /* note: currently, only max. max msg length = 65536 */
   tANI_U16 raUpdateType;     /* note: currently, only max. 65536 different event are reserved */
#endif
    PACKED_PRE union PACKED_POST {
    Qwlanfw_RaAddBssMsgType         raAddBssMsg;
    Qwlanfw_RaDelBssMsgType         raDelBssMsg;
    Qwlanfw_RaAddStaMsgType         raAddStaMsg;
    Qwlanfw_RaDelStaMsgType         raDelStaMsg;
    Qwlanfw_RaTimerMsgType          raTimerMsg;
    Qwlanfw_RaUpdateParamMsgType    raUpdateParam;
    Qwlanfw_RaForceStaRateMsgType   raForceStaRate;
    } u;
} Qwlanfw_RaUpdateMsgType;


#ifdef WLAN_SOFTAP_FEATURE
typedef  PACKED_PRE struct PACKED_POST _Qwlanfw_AddBssStruct
{
   Qwlanfw_CtrlMsgType  hdr;
   
   /* BSS index */
   tANI_U32 bssIdx;
} Qwlanfw_AddBssType;

typedef  PACKED_PRE struct PACKED_POST _Qwlanfw_DelBssStruct
{
   Qwlanfw_CtrlMsgType  hdr;
   
   /* BSS index */
   tANI_U32 bssIdx;
} Qwlanfw_DelBssType;

typedef  PACKED_PRE struct PACKED_POST _Qwlanfw_SendBeaconStruct
{
   Qwlanfw_CtrlMsgType  hdr;
  
#ifdef ANI_BIG_BYTE_ENDIAN
   tANI_U32   bssIdx                :8;

   tANI_U32   bcnTimIeOffset        :8;
   tANI_U32   bReserved             :16;
#else
   tANI_U32   bReserved             :16;
   tANI_U32   bcnTimIeOffset        :8;
   tANI_U32   bssIdx                :8;  /*  BSS Index   */
#endif
} Qwlanfw_SendBeaconType;

typedef  PACKED_PRE struct PACKED_POST _Qwlanfw_AddStaStruct
{
   Qwlanfw_CtrlMsgType  hdr;
   
   /* STA index */
   tANI_U32 staIdx;
} Qwlanfw_AddStaType;

typedef  PACKED_PRE struct PACKED_POST _Qwlanfw_DelStaStruct
{
   Qwlanfw_CtrlMsgType  hdr;
   
   /* STA index */
   tANI_U32 staIdx;
} Qwlanfw_DelStaType;

typedef  PACKED_PRE struct PACKED_POST _Qwlanfw_UpdateBaStruct
{
   Qwlanfw_CtrlMsgType  hdr;
   
#ifdef ANI_BIG_BYTE_ENDIAN
   tANI_U32 staIdx:4;       // Sta Index
   tANI_U32 queueId:5;      // Queue Id
   tANI_U32 code:1;         // AMPDU valid bit
   tANI_U32 resvd:22;       // Reserved
#else
   tANI_U32 resvd:22;       // Reserved
   tANI_U32 code:1;         // AMPDU valid bit
   tANI_U32 queueId:5;      // Queue ID
   tANI_U32 staIdx:4;       // Sta Index
#endif
} Qwlanfw_UpdateBaType;


#if WLAN_SOFTAP_FW_PROCESS_PROBE_REQ_FEATURE

typedef  PACKED_PRE struct PACKED_POST _Qwlanfw_UpdateProbeRespTemplateStaStruct
{
   Qwlanfw_CtrlMsgType  hdr;
   
#ifdef ANI_BIG_BYTE_ENDIAN
	  tANI_U32	 bssIdx 			   : 8;
   tANI_U32  enableFlag   : 1;
   tANI_U32  bReserved    :23;
#else
   tANI_U32  bReserved    :23;
   tANI_U32  enableFlag   : 1;
	  tANI_U32	 bssIdx 			   : 8;	
#endif

} Qwlanfw_UpdateProbeRespTemplateStaType;

#endif


#define SAP_UTIL_CMD_DEQUEUE_BD             0
#define SAP_UTIL_CMD_CHK_BD_LINK_INTEGRITY  1
#define SAP_UTIL_CMD_GET_QUEUE_INFO         2
#define SAP_UTIL_CMD_PULL_BTQM_FRAMES       3
#define SAP_UTIL_CMD_ENABLE_BTQM            4
#define SAP_UTIL_CMD_DISABLE_BTQM           5
#define SAP_UTIL_CMD_STA_ENTER_PS           6
#define SAP_UTIL_CMD_STA_EXIT_PS            7
#define SAP_UTIL_CMD_TRANSMIT_NULL_DATA     8
#define SAP_UTIL_CMD_GET_BD_COUNT           9
#define SAP_UTIL_CMD_TX_PRB_RSP_PKT        10
#define SAP_UTIL_CMD_PROCESS_PRB_REQ_CMD   11
#define SAP_UTIL_CMD_TX_CTS2SELF_CMD       12
#define SAP_UTIL_CMD_SEND_FC_INFO_FW2TL    13
#define SAP_UTIL_CMD_GET_STA_INFO          14
#define SAP_UTIL_CMD_GET_STA_PRIV_INFO     15 
#define SAP_UTIL_CMD_SEND_DEL_STA_INFO     16 

#define SAP_UTIL_CMD_TRANSMIT_NULL_DATA_QOS_MASK    0x1
#define SAP_UTIL_CMD_TRANSMIT_NULL_DATA_EOSP_MASK   0x2

typedef  PACKED_PRE struct PACKED_POST _Qwlanfw_SapUtilStruct
{
   Qwlanfw_CtrlMsgType  hdr;
   
#ifdef ANI_BIG_BYTE_ENDIAN
   /* STA index */
   tANI_U32 staIdx:3;
   tANI_U32 queueId:4;
   tANI_U32 cmd:5;
   tANI_U32 rsvd: 20;
#else
   tANI_U32 rsvd: 20;
   tANI_U32 cmd:5;
   tANI_U32 queueId:4;
   tANI_U32 staIdx:3;
#endif   

} Qwlanfw_SapUtilStruct;

/**
@brief
QWLANFW_FW2HOST_DEL_STA_CONTEXT
*/
typedef  PACKED_PRE struct PACKED_POST _Qwlanfw_DeleteStaContextStruct
{
   Qwlanfw_CtrlMsgType  hdr;
   
#ifdef ANI_BIG_BYTE_ENDIAN
   tANI_U32  assocId     : 8;
   tANI_U32  staIdx      : 8;
   tANI_U32  bssIdx      : 8;
   tANI_U32  uReasonCode : 8;
#else
   tANI_U32  uReasonCode : 8;
   tANI_U32  bssIdx      : 8;
   tANI_U32  staIdx      : 8;
   tANI_U32  assocId     : 8;
#endif

   tANI_U32  uStatus;

} tQwlanfw_DeleteStaContextType, * tpQwlanfw_DeleteStaContextType;

enum {
   QWLAN_DEL_STA_REASON_CODE_NONE       = 0x0,
   QWLAN_DEL_STA_REASON_CODE_KEEP_ALIVE = 0x1,
   QWLAN_DEL_STA_REASON_CODE_TIM_BASED  = 0x2,
   QWLAN_DEL_STA_REASON_CODE_RA_BASED   = 0x3,
   QWLAN_DEL_STA_REASON_CODE_UNKNOWN_A2 = 0x4
};


typedef struct fwProbeRespTemplateCtrlBlock_Type{
#ifdef ANI_BIG_BYTE_ENDIAN
      tANI_U32  bProbeRespBitMapAndTemplateEn     : 1;
      tANI_U32  BitMapLen       : 8;
      tANI_U32  TemplateLen     :16;
      tANI_U32  rsvd            : 7;
#else
      tANI_U32  rsvd            : 7;
      tANI_U32  TemplateLen     :16;
      tANI_U32  BitMapLen       : 8;
      tANI_U32  bProbeRespBitMapAndTemplateEn     : 1;
#endif
}tFwProbeRespTemplateCtrlBlock, * tpFwProbeRespTemplateCtrlBlock;

typedef struct fwProbeRespTemplate_Type{
      tFwProbeRespTemplateCtrlBlock      prbCtlBlock;
      tANI_U32 ucProxyProbeReqValidIEBmap[WLAN_SOFTAP_FW_PROBE_REQ_BIT_MAP_LEN_IN_WORDS];
      tANI_U32 payLoad[WLAN_SOFTAP_PROBE_RESP_PAYLOAD_SIZE_IN_WORDS];
}tFwProbeRespTemplate, * tpFwProbeRespTemplate;

#endif //WLAN_SOFTAP_FEATURE



/*==================================================================================
  FW -> HOST MESSAGE DEFINITIONS
==================================================================================*/
/**
   @brief
    QWLANFW_FW2HOST_STATUS
 */
typedef  PACKED_PRE struct PACKED_POST _Qwlanfw_StatusMsgStruct
{
   Qwlanfw_CtrlMsgType  hdr;
   /** See QWLANFW_STATUS_XXXX
    */
   tANI_U32             uStatus;
} Qwlanfw_StatusMsgType;

/**
   @brief
    QWLANFW_FW2HOST_ENTER_IMPS_RSP
 */
typedef  PACKED_PRE struct PACKED_POST _Qwlanfw_EnterImpsRspStruct
{
   Qwlanfw_CtrlMsgType  hdr;
   tANI_U32             uStatus;
} Qwlanfw_EnterImpsRspType;

/**
   @brief
    QWLANFW_FW2HOST_IMPS_EXITED
 */
typedef  PACKED_PRE struct PACKED_POST _Qwlanfw_ImpsExitedStruct
{
   Qwlanfw_CtrlMsgType  hdr;
   tANI_U32             uStatus;
} Qwlanfw_ImpsExitedType;

/**
   @brief
    QWLANFW_FW2HOST_ENTER_BMPS_RSP
 */
typedef  PACKED_PRE struct PACKED_POST _Qwlanfw_EnterBmpsRspStruct
{
   Qwlanfw_CtrlMsgType  hdr;
   tANI_U32             uStatus;
} Qwlanfw_EnterBmpsRspType;

/**
   @brief
    QWLANFW_FW2HOST_EXIT_BMPS_RSP
 */
typedef  PACKED_PRE struct PACKED_POST _Qwlanfw_ExitBmpsRspStruct
{
   Qwlanfw_CtrlMsgType  hdr;
   tANI_U32             uStatus;
} Qwlanfw_ExitBmpsRspType;

/**
   @brief
    QWLANFW_FW2HOST_SUSPEND_BMPS_RSP
 */
typedef  PACKED_PRE struct PACKED_POST _Qwlanfw_SuspendBmpsRspStruct
{
   Qwlanfw_CtrlMsgType  hdr;
   tANI_U32             uStatus;
} Qwlanfw_SuspendBmpsRspType;

/**
   @brief
    QWLANFW_FW2HOST_RESUME_BMPS_RSP
 */
typedef  PACKED_PRE struct PACKED_POST _Qwlanfw_ResumeBmpsRspStruct
{
   Qwlanfw_CtrlMsgType  hdr;
   tANI_U32             uStatus;
} Qwlanfw_ResumeBmpsRspType;

/**
   @brief
    QWLANFW_FW2HOST_ENTER_UAPSD_RSP
 */
typedef  PACKED_PRE struct PACKED_POST _Qwlanfw_EnterUapsdRspStruct
{
   Qwlanfw_CtrlMsgType  hdr;
   tANI_U32             uStatus;
} Qwlanfw_EnterUapsdRspType;

/**
   @brief
    QWLANFW_FW2HOST_EXIT_UAPSD_RSP
 */
typedef  PACKED_PRE struct PACKED_POST _Qwlanfw_ExitUapsdRspStruct
{
   Qwlanfw_CtrlMsgType  hdr;
   tANI_U32             uStatus;
} Qwlanfw_ExitUapsdRspType;

/**
   @brief
    QWLANFW_FW2HOST_RSSI_NOTIFICATION
 */
typedef  PACKED_PRE struct PACKED_POST _Qwlanfw_RssiNotificationStruct
{
   Qwlanfw_CtrlMsgType  hdr;
#ifdef ANI_BIG_BYTE_ENDIAN
   tANI_U32             bRssiThres1PosCross : 1;
   tANI_U32             bRssiThres1NegCross : 1;
   tANI_U32             bRssiThres2PosCross : 1;
   tANI_U32             bRssiThres2NegCross : 1;
   tANI_U32             bRssiThres3PosCross : 1;
   tANI_U32             bRssiThres3NegCross : 1;
   tANI_U32             bReserved           : 26;
#else
   tANI_U32             bReserved           : 26;
   tANI_U32             bRssiThres3NegCross : 1;
   tANI_U32             bRssiThres3PosCross : 1;
   tANI_U32             bRssiThres2NegCross : 1;
   tANI_U32             bRssiThres2PosCross : 1;
   tANI_U32             bRssiThres1NegCross : 1;
   tANI_U32             bRssiThres1PosCross : 1;
#endif

} Qwlanfw_RssiNotificationType;

/**
   @brief
    QWLANFW_FW2HOST_CAL_UPDATE_RSP
 */
typedef  PACKED_PRE struct PACKED_POST _Qwlanfw_CalUpdateRspStruct
{
   Qwlanfw_CtrlMsgType  hdr;
   tANI_U32             uStatus;
   /*TBD Other parameter are yet to be
    *identified.
    */
} Qwlanfw_CalUpdateRspType;

/**
   @brief
    QWLANFW_FW2HOST_PERFORM_PERIODIC_CAL
 */
typedef  PACKED_PRE struct PACKED_POST _Qwlanfw_PerformPeriodicCalStruct
{
   Qwlanfw_CtrlMsgType  hdr;
   tANI_U32             uStatus;
   /*TBD Other parameters are yet to be
    * identified
    */
} Qwlanfw_PerformPeriodicCalType;

/**
   @brief
    QWLANFW_FW2HOST_SET_CHANNEL_RSP
 */
typedef  PACKED_PRE struct PACKED_POST _Qwlanfw_SetChannelRspStruct
{
  Qwlanfw_CtrlMsgType  hdr;
#ifdef ANI_BIG_BYTE_ENDIAN
  tANI_U32             topGainDB       : 8;
  tANI_U32             bottomDB        : 8;
  tANI_U32             maxAgcGainIndex : 16;
#else
  tANI_U32             maxAgcGainIndex : 16;
  tANI_U32             bottomDB        : 8;
  tANI_U32             topGainDB       : 8;
#endif
  tANI_U32             uStatus;
} Qwlanfw_SetChannelRspType;

/**
@brief
QWLANFW_FW2HOST_SET_CHAIN_SELECT_RSP
*/
typedef  PACKED_PRE struct PACKED_POST _Qwlanfw_SetChainSelectRspStruct
{
   Qwlanfw_CtrlMsgType  hdr;
   tANI_U32             uStatus;
} Qwlanfw_SetChainSelectRspType;


#ifdef LIBRA_WAPI_SUPPORT
#define QWLANFW_KEY_TYPE_PTK    0
#define QWLANFW_KEY_TYPE_GTK    1
#define QWLANFW_KEY_TYPE_MAX    2

/**
   @brief
    QWLANFW_HOST2FW_WPI_KEY_SET / REMOVED
   
*/
typedef  PACKED_PRE struct PACKED_POST _Qwlanfw_AddRemoveKeyReqStruct
{
   Qwlanfw_CtrlMsgType  hdr;
#ifdef ANI_BIG_BYTE_ENDIAN
   tANI_U32  keyType:2;   //01: GTK   //00: PTK
   tANI_U32  keyIndex:6;
   tANI_U32  dpuIndex:8;
   tANI_U32  reserved0:16;
#else
   tANI_U32  reserved0:16;
   tANI_U32  dpuIndex:8;
   tANI_U32  keyIndex:6;
   tANI_U32  keyType:2;   //01: GTK  //00: PTK  
#endif
} Qwlanfw_AddRemoveKeyReqType;
#endif

#endif /*_QWLAN_MACFW_H*/

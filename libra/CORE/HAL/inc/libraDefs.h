/**
 *
 *  @file:         libraDefs.h
 *
 *  @brief:        This file libraDefs.h contains the Libra hardware related definitions.
 *
 *  @author:       David Liu
 *
 *  Copyright (C)  2008, Qualcomm, Inc. All rights reserved.
 *
 *  Change History:
 * ---------------------------------------
 * 05/09/2008  File created.
 */
#ifndef __LIBRA_DEFS_H
#define __LIBRA_DEFS_H

#include "libra.h"

#ifndef __ASSEMBLER__

/* --------------------------------------------------------------------
 * Support definitions for taurus
 * --------------------------------------------------------------------
 */

/*
 * Libra suppports 8 stations
 */
#define HAL_NUM_STA                 8
#define HAL_NUM_BSSID               3 /* For MultiSession */
#define HAL_NUM_UMA_DESC_ENTRIES    8

#define MAX_NUM_OF_BACKOFFS         8

/*
 * From NOVA Mac Arch document
 *  Encryp. mode    The encryption mode
 *  000: Encryption functionality is not enabled
 *  001: Encryption is set to WEP
 *  010: Encryption is set to WEP 104
 *  011: Encryption is set to TKIP
 *  100: Encryption is set to AES
 *  101 - 111: Reserved for future
 */

#define HAL_ENC_POLICY_NULL        0
#define HAL_ENC_POLICY_WEP40       1
#define HAL_ENC_POLICY_WEP104      2
#define HAL_ENC_POLICY_TKIP        3
#define HAL_ENC_POLICY_AES_CCM     4

/// General Virgo data structures related defines
#define HAL_BD_SIZE                128
#define HAL_PDU_SIZE               128
#define HAL_PDU_PLOAD_SIZE         124

/* --------------------------------------------------------------------- */
/* BMU */
/* --------------------------------------------------------------------- */
#define BMU_COMMAND_BASE_ADDRESS    0x0F000000  // got this from libra map: 0f00_0000
#define BMU_MEMORY_BASE_ADDRESS     0x10000         // from pci stand point, internal memory start at 0
#define BMU_BD_SIZE_BITS            7
#define BMU_BD_SIZE                 128         // bytes
#define BMU_MAX_NUM_BD              1023        // Max number of BDs BMU can support as per the Libra programmer's guide.
#define BMU_MAX_BD_INDEX_ALLOWED    1023        // We may change this value later if we need some for some other purpose such as tracing
#define HW_MAX_QUEUES               0x0b
#define BTQM_STA_QUEUE_ENTRY_SIZE    8

#define BMU_CMD_GET_BD_PDU_REQ_CODE1_SHIFT  16
#define BMU_CMD_GET_BD_PDU_REQ_CODE2_SHIFT  20

#define BMU_NUM_BD_GET_SHIFT        16
#define BMU_MASTER_ID_SHIFT          8
#define BMU_QUEUEID_SHIFT            8
/* -------------
 *  Memory Size
 * -------------
 */
#define BMU_INTERNAL_MEMORY_SIZE_128K    0x20000      // 128K
#define BMU_INTERNAL_MEMORY_SIZE_256K    0x40000      // 128K
#define BMU_INTERNAL_MEMORY_SIZE_512K    0x80000      // 512K
#define BMU_TOTAL_MEMORY_SIZE            0x40000      // 256K


// BMU enable/disable sta transmission commands
typedef enum eBmuStaTxCfgCmd {
    eBMU_ENB_TX_QUE_DONOT_ENB_TRANS       = 0x0,
    eBMU_ENB_TX_QUE_ENB_TRANS             = 0x1,
    eBMU_DIS_TX_QUE_DIS_TRANS             = 0x2,
    eBMU_DIS_TX_QUE_DIS_TRANS_CLEANUP_QUE = 0x3,
} tBmuStaTxCfgCmd;


/* ------------------
 *  Memory Addresses
 * ------------------
 */



/*
 * BMU master ID assignment
 *
 * BMU master ID is used to identify master sending commands to BMU.
 * Master index should be accompanied with BMU GET IDLE or RELEASE
 * commands. Each master ID is associated with BD_PDU threshold, and
 * BD_PDU reservation slot. And BD_PDU_AVAIL signal is controlled 
 * based on available BD/PDU count and BD/PDU threshold. That is 
 * threshold is set per master ID not per WQ index. This mechanism
 * guarantees high priority module can get BD or PDUs when the availale
 * BD/PDU counts low. 
 *
 * RXP and DPU have fixed IDs, and MCPU and DXE master IDs are
 * software programmable. Current assignment is as follows
 *
 * 0 : RXP (1)
 * 1 - 2 : DPU (2)
 * 3 - 9 : DXE (7)
 * 10 : mCPU (1)
 *
 */

enum {
    BMU_MASTERID_0,
    BMU_MASTERID_1,
    BMU_MASTERID_2,
    BMU_MASTERID_3,
    BMU_MASTERID_4,
    BMU_MASTERID_5,
    BMU_MASTERID_6,
    BMU_MASTERID_7,
    BMU_MASTERID_8,
    BMU_MASTERID_NUM,

    /* Hardware defined */
    BMU_MASTERID_RXP = BMU_MASTERID_0,
    BMU_MASTERID_DPU_TX = BMU_MASTERID_1,
    BMU_MASTERID_DPU_RX = BMU_MASTERID_2,

    /* Software defined - DXE */
    BMU_MASTERID_DXE_START = BMU_MASTERID_5,
    BMU_MASTERID_DXE_END = BMU_MASTERID_7,
    
    BMU_MASTERID_DXE_0 = BMU_MASTERID_5,  //TX	 
    BMU_MASTERID_DXE_1,                   //RX
    BMU_MASTERID_DXE_2,                   //FW
    BMU_MASTERID_DXE_RCV = BMU_MASTERID_DXE_1,
    BMU_MASTERID_SW = BMU_MASTERID_8,
};


/* ------------------
 *  BMU command Type Definition
 * ------------------
 */
typedef enum eBmuCmdType {
    PUSH_WQ_CMDTYPE                     = 0x0,
    POP_WQ_CMDTYPE                      = 0x4,
    WRITE_WQ_HEAD_CMDTYPE               = 0x8,
    READ_WQ_HEAD_CMDTYPE                = 0xc,
    WRITE_WQ_TAIL_CMDTYPE               = 0x10,
    READ_WQ_TAIL_CMDTYPE                = 0x14,
    WRITE_WQ_NR_CMDTYPE                 = 0x18,
    READ_WQ_NR_CMDTYPE                  = 0x1c,
    WRITE_BD_POINTER_CMDTYPE            = 0x20,
    READ_BD_POINTER_CMDTYPE             = 0x24,
    RESERVATION_REQUEST_BD_PDU_CMDTYPE  = 0x28,
    GET_BD_PDU_CMDTYPE                  = 0x2c,
    RELEASE_PDU_CMDTYPE                 = 0X30,
    RELEASE_BD_CMDTYPE                  = 0X34,
    RESERVE_ALL_BD_PDU_CMDTYPE          = 0x44,
} tBmuCmdType;


/* Request code for BMU command for getting BD/PDU */
typedef enum sBmuGetBdReqCode {
    REQ_CODE_NO_BD_PDU = 0, // No BD/PDU requested
    REQ_CODE_ONE_BD,        // One BD requested
    REQ_CODE_ONE_PDU        // ONE PDU requested
} tBmuGetBdReqCode;

#define BMU_CMD_POP_WQ(wq)                  (BMU_COMMAND_BASE_ADDRESS|((wq<<8)|POP_WQ_CMDTYPE)) 
#define BMU_CMD_PUSH_WQ(wq)                 (BMU_COMMAND_BASE_ADDRESS|((wq<<8)|PUSH_WQ_CMDTYPE)) 
#define BMU_CMD_WRITE_BD_PTR(bdIdx)         (BMU_COMMAND_BASE_ADDRESS|((bdIdx<<8)|WRITE_BD_POINTER_CMDTYPE))
#define BMU_CMD_READ_BD_PTR(bdIdx)          (BMU_COMMAND_BASE_ADDRESS|((bdIdx<<8)|READ_BD_POINTER_CMDTYPE))
#define BMU_CMD_READ_WQ_HEAD(wq)            (BMU_COMMAND_BASE_ADDRESS|((wq<<8)|READ_WQ_HEAD_CMDTYPE))
#define BMU_CMD_GET_BD_PDU(code1,code2,mod) (BMU_COMMAND_BASE_ADDRESS|  \
                                            ((code2<<BMU_CMD_GET_BD_PDU_REQ_CODE2_SHIFT)|\
                                             (code1<<BMU_CMD_GET_BD_PDU_REQ_CODE1_SHIFT)|\
                                             (mod<<BMU_MASTER_ID_SHIFT)|GET_BD_PDU_CMDTYPE))

// ----------------------------
// BD and PDU Threshold values
//reference section 4.7.1 in libra_wmac_sys_programmers_guide.doc
// ----------------------------
#define BMU_RXP_BD_THRESHOLD        0x35
#define BMU_DPUTX_BD_THRESHOLD      0x1
#define BMU_DPURX_BD_THRESHOLD      0x1
#define BMU_ADU_BD_THRESHOLD        0x1
#define BMU_RPE_BD_THRESHOLD        0x15
#define BMU_DXE_CH0_BD_THRESHOLD    0x40
#define BMU_DXE_CH1_BD_THRESHOLD    0
#define BMU_DXE_CH2_BD_THRESHOLD    0x35
#define BMU_MACSW_BD_THRESHOLD      0x35
#define BMU_RXP_PDU_THRESHOLD       0xa0
#define BMU_DPUTX_PDU_THRESHOLD     0x1
#define BMU_DPURX_PDU_THRESHOLD     0x1
#define BMU_ADU_PDU_THRESHOLD       0x1
#define BMU_RPE_PDU_THRESHOLD       0x60
#define BMU_DXE_CH0_PDU_THRESHOLD   0x100
#define BMU_DXE_CH1_PDU_THRESHOLD   0
#define BMU_DXE_CH2_PDU_THRESHOLD   0xa0
#define BMU_MACSW_PDU_THRESHOLD     0xa0


// thresholds to use for internal memory only case (interchangeable bd/pdu)
#define BMU_RXP_BD_THRESHOLD_INT        0x35
#define BMU_DPUTX_BD_THRESHOLD_INT      0x1
#define BMU_DPURX_BD_THRESHOLD_INT      0x1
#define BMU_ADU_BD_THRESHOLD_INT        0x1
#define BMU_RPE_BD_THRESHOLD_INT        0x15
#define BMU_DXE_CH0_BD_THRESHOLD_INT    0x40
#define BMU_DXE_CH1_BD_THRESHOLD_INT    0x0
#define BMU_DXE_CH2_BD_THRESHOLD_INT    0x35
#define BMU_MACSW_BD_THRESHOLD_INT      0x35
#define BMU_DPUTX_PDU_THRESHOLD_INT     0x1
#define BMU_DPURX_PDU_THRESHOLD_INT     0x1
#define BMU_ADU_PDU_THRESHOLD_INT       0x1
#define BMU_RPE_PDU_THRESHOLD_INT       0x60
#define BMU_DXE_CH1_PDU_THRESHOLD_INT   0
#define BMU_DXE_CH2_PDU_THRESHOLD_INT   0xa0
#define BMU_MACSW_PDU_THRESHOLD_INT     0xa0

/* If DAFCA trace debug is enabled, then only 128K
 * internal memory is allowed to be used by MAC. Hence
 * need to adjust BD/PDU thresholds accordingly.
 */
#ifdef DAFCA_TRACE_DEBUG_ENABLE
  #define BMU_RXP_PDU_THRESHOLD_INT     0x60
  #define BMU_DXE_CH0_PDU_THRESHOLD_INT 0x60
#else
  #define BMU_RXP_PDU_THRESHOLD_INT     0xa0
  #define BMU_DXE_CH0_PDU_THRESHOLD_INT 0x100
#endif

#define BMU_DXE_TX_BD_THRESHOLD     BMU_DXE_CH0_BD_THRESHOLD
#define BMU_DXE_RX_BD_THRESHOLD     BMU_DXE_CH1_BD_THRESHOLD
#define BMU_DXE_FW_BD_THRESHOLD     BMU_DXE_CH2_BD_THRESHOLD
#define BMU_DXE_TX_PDU_THRESHOLD    BMU_DXE_CH0_PDU_THRESHOLD
#define BMU_DXE_RX_PDU_THRESHOLD    BMU_DXE_CH1_PDU_THRESHOLD
#define BMU_DXE_FW_PDU_THRESHOLD    BMU_DXE_CH2_PDU_THRESHOLD

#define BMU_DXE_TX_BD_THRESHOLD_INT     BMU_DXE_CH0_BD_THRESHOLD_INT
#define BMU_DXE_RX_BD_THRESHOLD_INT     BMU_DXE_CH1_BD_THRESHOLD_INT
#define BMU_DXE_FW_BD_THRESHOLD_INT     BMU_DXE_CH2_BD_THRESHOLD_INT
#define BMU_DXE_TX_PDU_THRESHOLD_INT    BMU_DXE_CH0_PDU_THRESHOLD_INT
#define BMU_DXE_RX_PDU_THRESHOLD_INT    BMU_DXE_CH1_PDU_THRESHOLD_INT
#define BMU_DXE_FW_PDU_THRESHOLD_INT    BMU_DXE_CH2_PDU_THRESHOLD_INT


/* --------------------------------------------------------------------- */
/* MCU */
/* --------------------------------------------------------------------- */
#define  MCU_MAX_NUM_OF_MAILBOX             4
#define  MCU_MAX_NUM_OF_MUTEX               8
#define  MCU_MAILBOX_CONTROL_REG_ADDR(n)    (QWLAN_MCU_MB0_CONTROL_REG + (n * 8))
#define  MCU_MUTEX_REG_ADDR(n)              (QWLAN_MCU_MUTEX0_REG + (n * 4))

/* --------------------------------------------------------------------- */
/* RXP */
/* --------------------------------------------------------------------- */
/* Bit position in the RXP frame_filter_config register */
#define RXP_VERSION                   QWLAN_RXP_FRAME_FILTER_CONFIG_VERSION_CHECK_ENABLE_MASK
#define RXP_BLOCK                     QWLAN_RXP_FRAME_FILTER_CONFIG_BLOCK_RECEPTION_ENABLE_MASK
#define RXP_NAV_SET                   QWLAN_RXP_FRAME_FILTER_CONFIG_NAV_SET_ENABLE_MASK
#define RXP_NAV_CLEAR                 QWLAN_RXP_FRAME_FILTER_CONFIG_NAV_CLEAR_ENABLE_MASK
#define RXP_NAV_SET_ON_ABORT          QWLAN_RXP_FRAME_FILTER_CONFIG_NAV_SET_ENABLE_ON_ABORT_MASK
#define RXP_NAV_CLEAR_ON_ABORT        QWLAN_RXP_FRAME_FILTER_CONFIG_NAV_CLEAR_ENABLE_ON_ABORT_MASK
#define RXP_ADDR1_BLOCK_BROADCAST     QWLAN_RXP_FRAME_FILTER_CONFIG_ADDR1_BLOCK_BROADCAST_ENABLE_MASK
#define RXP_ADDR1_BLOCK_MULTICAST     QWLAN_RXP_FRAME_FILTER_CONFIG_ADDR1_BLOCK_MULTICAST_ENABLE_MASK
#define RXP_ADDR1_BLOCK_UNICAST       QWLAN_RXP_FRAME_FILTER_CONFIG_ADDR1_BLOCK_UNICAST_ENABLE_MASK
#define RXP_ADDR1_FILTER              QWLAN_RXP_FRAME_FILTER_CONFIG_ADDR1_BINARY_SEARCH_FILTER_ENABLE_MASK
#define RXP_ADDR1_ACCEPT_MULTICAST    QWLAN_RXP_FRAME_FILTER_CONFIG_ADDR1_ACCEPT_REMAINING_MULTICAST_ENABLE_MASK
#define RXP_ADDR1_ACCEPT_UNICAST      QWLAN_RXP_FRAME_FILTER_CONFIG_ADDR1_ACCEPT_REMAINING_UNICAST_ENABLE_MASK
#define RXP_ADDR2_FILTER              QWLAN_RXP_FRAME_FILTER_CONFIG_ADDR2_BINARY_SEARCH_FILTER_ENABLE_MASK
#define RXP_ADDR2_ACCEPT_REMAIN       QWLAN_RXP_FRAME_FILTER_CONFIG_ADDR2_ACCEPT_REMAINING_ENABLE_MASK
#define RXP_ADDR3_FILTER              QWLAN_RXP_FRAME_FILTER_CONFIG_ADDR3_BINARY_SEARCH_FILTER_ENABLE_MASK
#define RXP_ADDR3_ACCEPT_REMAIN       QWLAN_RXP_FRAME_FILTER_CONFIG_ADDR3_ACCEPT_REMAINING_ENABLE_MASK
#define RXP_FCS                       QWLAN_RXP_FRAME_FILTER_CONFIG_FCS_FILTER_ENABLE_MASK
#define RXP_PHY_RX_ABORT              QWLAN_RXP_FRAME_FILTER_CONFIG_PHY_RX_ABORT_ENABLE_MASK
#define RXP_FRAME_TRANSLATION         QWLAN_RXP_FRAME_FILTER_CONFIG_FRAME_TRANSLATION_REQ_EN_MASK
#define RXP_DROP_AT_DMA               QWLAN_RXP_FRAME_FILTER_CONFIG_G5_DROP_AT_DMA_MASK


// Derived from above
#define RXP_ACCEPT_ALL_ADDR1        (RXP_ADDR1_FILTER | RXP_ADDR1_ACCEPT_MULTICAST |RXP_ADDR1_ACCEPT_UNICAST)
#define RXP_ACCEPT_ALL_ADDR2        (RXP_ADDR2_FILTER | RXP_ADDR2_ACCEPT_REMAIN)
#define RXP_ACCEPT_ALL_ADDR3        (RXP_ADDR3_FILTER | RXP_ADDR3_ACCEPT_REMAIN)
#define RXP_ACCEPT_ALL_ADDRESS      (RXP_ACCEPT_ALL_ADDR1 | RXP_ACCEPT_ALL_ADDR2 | RXP_ACCEPT_ALL_ADDR3)

#define RXP_FRAME_TYPES_MAX             64

/* Binary Search Table */
#define  RXP_MAX_TABLE_ENTRY            HAL_NUM_STA + HAL_NUM_BSSID

/* set the default max PDU's to be reserved by RXP */
#define RXP_DMA_MAX_RSV_PDU             68

/* TSF Compensation values */
#define HAL_11B_1MBPS_TSF_COMPENSATION_VALUE 0x44
#define HAL_11B_2MBPS_TSF_COMPENSATION_VALUE 0x22
#define HAL_11B_5_5MBPS_TSF_COMPENSATION_VALUE 0x11
#define HAL_11B_11MBPS_TSF_COMPENSATION_VALUE 0x0C

#define HAL_11AG_6MBPS_TSF_COMPENSATION_VALUE 0x11
#define HAL_11AG_54MBPS_TSF_COMPENSATION_VALUE 0x05
#define HAL_11AG_OTHER_TSF_COMPENSATION_VALUE 0x11



/* --------------------------------------------------------------------- */
/* DPU */
/* --------------------------------------------------------------------- */
#define DPU_MAX_DESCRIPTOR_ENTRIES      255

/* default number of BD and PDU's to be reserved by DPU for internal memory */
#define DPU_TX_RSV_NUMBD_INT    0x10
#define DPU_TX_RSV_NUMPDU_INT   0x20

/* DPU feedback in BD */
#define DPU_FEEDBACK_WPI_UNPROTECTED    0x20    //Receive unprotected frame on WPI encrypted channel


/* TPE Sta Stats Info */
#define TPE_STATSPERBKOFF_SIZE      MAX_NUM_OF_BACKOFFS

#define TPE_BEACON_1MBPS_LONG_TSF_OFFSET    0x184

/* TPE TXCPLT related*/

/*
THIS IS THE COMMENT ABOUT TPE FEEDBACK register from e-mail
Basically whenever packet is successfully sent and dequeued from the system: the bit [0]= 1'b0 If there is an ack timie out: bit[0]=1 
        ----------> This doesn't mean that the packet is dropped. 
        ----------> The same will be retried till the retry count reaches the threshold

So If you guys are looking for a case where the packet has been dropped because of retries you need to chek for
        ------> (bit[0]=1 &(bit[3:1] == 3'b001 OR bit[3:1] == 3'b010))

        registerComment
                                    [0] is ack_to_valid
                                    [3:1] feedback taht is sent to bmu -- Definition is as follows.
                                        S/w Doesn't need this. Kept this for any debug puropse and if s/w comes
                                        up with new requirement in future, these can be used

                                    TPE_BMU_FB_RESET                        3'b000
                                    TPE_BMU_FB_DROP                         3'b001
                                    TPE_BMU_FB_DROP_AND_REARB               3'b010
                                    TPE_BMU_FB_RESET_AND_REARB              3'b011
                                    TPE_BMU_FB_RESET_AND_CHK_RETRY          3'b100
                                    TPE_BMU_FB_RESET_CHK_RTRY_AND_REARB     3'b101
                                    TPE_BMU_FB_RESET_AND_REARB_UPDATE_RETRY 3'b110
        endComment

*/
#define TPE_ACK_TO_MASK 0x1  //mask to check whether ACK is valid or not in the feedback register.
#define TPE_TX_COMPLETE_FEEDBACK_MASK 0xF //applies to both txComplete0 and txComplete1
#define IS_TPE_TX_COMPLETE_FEEDBACK_FRAME_DROPPED(x) (((x == 0x3) || (x == 0x5))?1:0)

#define TPE_FW_TX_COMPLETE_INT_EN_MASK        QWLAN_MCU_COMBINED_INT_EN_TPE_MCU_BD_BASED_TX_INT_0_P_EN_MASK
#define TPE_HOST_TX_COMPLETE_INT_EN_MASK      QWLAN_MCU_COMBINED_HOST_INT_EN_TPE_MCU_BD_BASED_TX_INT_1_P_HOSTINT_EN_MASK

#define TPE_FW_TX_COMPLETE_INT_STATUS_MASK    QWLAN_MCU_COMBINED_INT_MASKED_STATUS_TPE_MCU_BD_BASED_TX_INT_0_P_MASK_STATUS_MASK
#define TPE_HOST_TX_COMPLETE_INT_STATUS_MASK  QWLAN_MCU_COMBINED_HOST_INT_MASKED_STATUS_TPE_MCU_BD_BASED_TX_INT_1_P_HOSTINT_MASK_STATUS_MASK

#define TPE_FW_TPE_TX_COMPLETE_FEEDBACK_REG   QWLAN_TPE_TPE_MCU_BD_BASED_TX_INT_FEEDBACK_REG
#define TPE_HOST_TPE_TX_COMPLETE_FEEDBACK_REG QWLAN_TPE_TPE_MCU_BD_BASED_TX_INT_FEEDBACK_1_REG

/* --------------------------------------------------------------------- */
/* MTU */
/* --------------------------------------------------------------------- */
/* desired default for CCA_SEL register */
#define MTU_CCA_SEL_MODE_REG1_DEFAULT 0x36DB6DB6
#define MTU_CCA_SEL_MODE_REG2_DEFAULT 0x00000036


/* --------------------------------------------------------------------- */
/* SCU */
/* --------------------------------------------------------------------- */

#define SCU_SYS_FPGA_REVISION_TAURUS_A1   0x15e
#define SCU_SYS_FPGA_REVISION_TAURUS_A2   0x15e
#define SCU_SYS_FPGA_REVISION_TAURUS_A3   0x15f
#define SCU_SYS_FPGA_REVISION_TAURUS_A4   0x17f

/* --------------------------------------------------------------------- */
/* AGC */
/* --------------------------------------------------------------------- */

#define AGC_RDET_RESET_VALUE_1    0x1f
#define AGC_RDET_RESET_VALUE_2    0x0
#define AGC_RDET_FLAG_RESET_VALUE 0x1

/* --------------------------------------------------------------------- */
/* TPC */
/* --------------------------------------------------------------------- */
/* Back up 32 TPC gain lut entries in ADU reinit */
#define TPC_GAIN_LUT_ADU_REINIT_SIZE    (32 * 4 * 2)

/* --------------------------------------------------------------------- */
/* BD  type*/
/* --------------------------------------------------------------------- */
#define HWBD_TYPE_GENERIC                  0   /* generic BD format */
#define HWBD_TYPE_FRAG                     1   /* fragmentation BD format*/

/* TX BD structure */
#define SWBD_TX_MPDUHEADER_OFFSET          0x48    /* MPDU header offset */

/* TX MPDU data offset (FIXME) There's no fixed MPDU data offset */
#define SWBD_TX_MPDUDATA_OFFSET            128

/* RX BD structure */
#define SWBD_RX_MPDUHEADER_OFFSET          0x4c

/* RX station index */
#define HWBD_RX_UNKNOWN_UCAST              254
#define HWBD_RX_UNKNOWN_MCAST              255
#define HWBD_RX_UNFILTERED                 253
#define SWBD_TX_TID_MGMT_LOW               0   /* ProbeResponse */
#define SWBD_TX_TID_MGMT_HIGH              1   /* Other management frames */

/* ---------------------------------------------------------------------*/
/* DXE*/
/* --------------------------------------------------------------------- */
/* TO DO
 * The below defines have to be reworked in the HAL side.
 * eg, max dxe channel is initialized to 2 but actually it 
 * is 3
 */
typedef enum
{
    DMA_CHANNEL_0,
    DMA_CHANNEL_1,
    DMA_CHANNEL_2,
    // Set to upper bound, not a real channel
    DMA_CHANNEL_MAX,
    DMA_CHANNEL_NONE,

    DMA_CHANNEL_TX = DMA_CHANNEL_0,
    DMA_CHANNEL_RX = DMA_CHANNEL_1,
    DMA_CHANNEL_FW = DMA_CHANNEL_2, 
}eDMAChannel;

#define MAX_DXE_CHANNEL     (DMA_CHANNEL_MAX - DMA_CHANNEL_0)

// These are used by the SDIO DXE driver
#define SIF_TX_FIFO_BASE    0x0E024100
#define SIF_RX_FIFO_BASE    0x0E024200

enum {
    QUE_SIF_TX,
    QUE_SIF_RX = 26
};
/* --------------------------------------------------------------------- */
/* BMU WQ Sharing*/
/* --------------------------------------------------------------------- */
/* TO DO
 * This structure should be revised as per PG
 */
 
/*
 * BMU WQ assignment, as per Libra Programmer's Guide
 *
 */
 
typedef enum sBmuWqId {

    /* ====== In use WQs ====== */

    /* BMU */
    BMUWQ_BMU_IDLE_BD = 0,
    BMUWQ_BMU_IDLE_PDU = 1,

    /* RxP */
    BMUWQ_RXP_UNKNWON_ADDR = 2,  /* currently unhandled by HAL */

    /* DPU RX */
    BMUWQ_DPU_RX = 3,

    /* DPU TX */
    BMUWQ_DPU_TX = 6,

    /* Firmware */
    BMUWQ_FW_TRANSMIT = 12,  /* DPU Tx->FW Tx */
    BMUWQ_FW_RECV = 7,       /* DPU Rx->FW Rx */
    BMUWQ_FW_RPE_RECV = 4,   /* RXP/RPE Rx->FW Rx */
    FW_SCO_WQ = BMUWQ_FW_RPE_RECV,
    BMUWQ_FW_DPU_TX = 5,

    /* DPU Error */
    BMUWQ_DPU_ERROR_WQ = 8,  

    /* DXE RX */
    BMUWQ_DXE_RX = 11,

    /* ADU/UMA */
    BMUWQ_ADU_UMA_TX = 23,
    BMUWQ_ADU_UMA_RX = 24, 

    /* BMU BTQM */
    BMUWQ_BTQM = 25,

    /* Special WQ for BMU to dropping all frames coming to this WQ ID */
    BMUWQ_SINK = 255,

    /* Total BMU WQ count in Libra */
    BMUWQ_NUM = 27,

    /* Aliases */
    BMUWQ_BTQM_TX_MGMT = BMUWQ_BTQM,
    BMUWQ_BTQM_TX_DATA = BMUWQ_BTQM,
    BMUWQ_BMU_WQ2 = BMUWQ_RXP_UNKNWON_ADDR,


    /* ====== Unused/Reserved WQ ====== */

    /* ADU/UMA Error WQ */
    BMUWQ_ADU_UMA_TX_ERROR_WQ = 13, /* Not in use by HAL */
    BMUWQ_ADU_UMA_RX_ERROR_WQ = 10, /* Not in use by HAL */

    /* DPU Error WQ2 */
    BMUWQ_DPU_ERROR_WQ2 = 9, /* Not in use by HAL */

    /* FW WQs */
    BMUWQ_FW_MESG = 14,      /* DxE Tx->FW, Not in use by FW */
    //BMUWQ_FW_DXECH2_0 = 15,  /* BD/PDU<->MEM conversion using DxE CH2.  Not in use by FW */ 
	
    //WQ where all frames with unknown Addr2 filter exception cases frames will pushed if FW wants host to 
    //send deauth to the sender. 
    BMUWQ_HOST_RX_UNKNOWN_ADDR2_FRAMES = 15,        
	
    BMUWQ_FW_DXECH2_1 = 16,  /* BD/PDU<->MEM conversion using DxE CH2.  Not in use by FW */
	
    /* Unassigned WQs in programmer's guide */
    BMUWQ_BMU_WQ4 = 4,
    BMUWQ_BMU_WQ5 = 5,
    BMUWQ_BMU_WQ17 = 17,
    BMUWQ_BMU_WQ18 = 18,
    BMUWQ_BMU_WQ19 = 19,
    BMUWQ_BMU_WQ20 = 20,
    BMUWQ_BMU_WQ21 = 21,
    BMUWQ_BMU_WQ22 = 22,

#ifdef LIBRA_WAPI_SUPPORT    
    BMUWQ_WAPI_DPU_TX = 17,  /* DPU Tx->FW WAPI Tx */
    BMUWQ_WAPI_DXE_REQ = BMUWQ_BMU_WQ20,
    BMUWQ_WAPI_RX_DXE_DONE = BMUWQ_BMU_WQ21,
    BMUWQ_WAPI_TX_DXE_DONE = BMUWQ_BMU_WQ22,
#endif
    
} tBmuWqId;

typedef enum 
{
    BTQM_QID0 = 0,
    BTQM_QID1,
    BTQM_QID2,
    BTQM_QID3,
    BTQM_QID4,
    BTQM_QID5,
    BTQM_QID6,
    BTQM_QID7,
    BTQM_QID8,
    BTQM_QID9,
    BTQM_QID10,

    BTQM_QUEUE_TX_TID_0 = BTQM_QID0,
    BTQM_QUEUE_TX_TID_1,
    BTQM_QUEUE_TX_TID_2,
    BTQM_QUEUE_TX_TID_3,
    BTQM_QUEUE_TX_TID_4,
    BTQM_QUEUE_TX_TID_5,
    BTQM_QUEUE_TX_TID_6,
    BTQM_QUEUE_TX_TID_7,
    

    /* Queue Id Usage                       BO Index
        8       Non-QoS data frames         6
        9       Management (low priority)   3
        10      Management (high priority)  2
    */
    BTQM_QUEUE_TX_nQOS = BTQM_QID8, 
    BTQM_QUEUE_SELF_STA_UCAST_MGMT = BTQM_QID9,
    BTQM_QUEUE_SELF_STA_UCAST_DATA = BTQM_QID9,    
    BTQM_QUEUE_SELF_STA_BCAST_MGMT = BTQM_QID10,
    BTQM_QUEUE_TX_AC_BE = BTQM_QUEUE_TX_TID_0,
    BTQM_QUEUE_TX_AC_BK = BTQM_QUEUE_TX_TID_2,
    BTQM_QUEUE_TX_AC_VI = BTQM_QUEUE_TX_TID_4,
    BTQM_QUEUE_TX_AC_VO = BTQM_QUEUE_TX_TID_6
}tBtqmQId;

#define STACFG_MAX_TC   8

/*-------------------------------------------------------------------------*/
/* Timers*/
/*-------------------------------------------------------------------------*/
/* TO DO
 * This list of MTU timer should be complete
 */

/** Number of timers available in the system */
#define QWLAN_MTU_TIMER_NUM             8

#define QWLAN_MTU_TIMER_CONTINUOUS_CONTROL_REG \
   QWLAN_MTU_TIMER_CONTROL11TO8_REG
#define QWLAN_MTU_TIMER_CONTINUOUS_CONTROL_ENABLE_N(index) \
   (1 << ((index) + QWLAN_MTU_TIMER_CONTROL11TO8_SW_MTU_CONTINUOUS_VALID_OFFSET))

enum {
   /* Free run clock for firmware */
   QWLAN_MTU_TIMER_FW_CLOCK = 6,
   /* Timer for firmware timer module */
   QWLAN_MTU_TIMER_FW_TIMER = 7,
};


/*-------------------------------------------------------------------------*/
/* Mutex*/
/*-------------------------------------------------------------------------*/
/* TO DO
 * All the mutexes are to be listed here
 */

/** Number of mutexes available in the system */
#define QWLAN_MCU_MUTEX_NUM                 8
 
#define QWLAN_MCU_MUTEX_HOSTFW_SYNC_INDEX   0
#define QWLAN_MCU_MUTEX_HOSTFW_SYNC_ADDR    QWLAN_MCU_MUTEX0_REG
#define QWLAN_HOSTFW_SYNC_MUTEX_MAX_COUNT   7

/*-------------------------------------------------------------------------*/
/* ADU*/
/*-------------------------------------------------------------------------*/

#define HAL_REG_RSVD_BIT        0

#define HAL_REG_FW_FILLED       (0)   // FW to fill the value of the register
#define HAL_REG_HOST_FILLED     (1)   // Host to fill the value and FW not to touch


#define HAL_REG_HOST_FILLED_MASK   (HAL_REG_HOST_FILLED << 0)

// Definitions for wait command
#define HAL_REG_REINIT_WAIT_CMD             0x80000000
#define HAL_REG_REINIT_WAIT_CMD_MASK        0x80000000
#define HAL_REG_REINIT_WAIT_CYCLE_MASK      0x00FFFFFF

// End of re-init register table command
#define HAL_REG_REINIT_TABLE_END_CMD        0xFF000000
#define HAL_REG_REINIT_TABLE_END_CMD_MASK   0xFF000000

#define HAL_REG_REINIT_TABLE_REG_ADDR_MASK  0x7FFFFFFC

#define RF_GC_BUS_BUG_WORKAROUND   1

#ifdef RF_GC_BUS_BUG_WORKAROUND
#define HAL_REG_FW_EXTRA_FILLED 2   // FW to fill the value with special considerations
#define HAL_REG_FW_EXTRA_FILLED_MASK   (HAL_REG_FW_EXTRA_FILLED << 0)
#else
#define HAL_REG_FW_EXTRA_FILLED 0
#endif


/*------------------------------------------------------------------------*/
/* Shared Interrupts*/
/*------------------------------------------------------------------------*/

//TO DO
#define QWLANHOST_RXP_RXP_GROUPED_INTERRUPT_ENABLE_MASK     0
#define QWLANFW_RXP_RXP_GROUPED_INTERRUPT_ENABLE_MASK       ~(QWLANHOST_RXP_RXP_GROUPED_INTERRUPT_ENABLE_MASK)

//TO DO
#define QWLANHOST_MCU_COMBINED_HOST_INT_EN_MASK             0
#define QWLANFW_MCU_COMBINED_INT_EN_MASK                    ~(QWLANHOST_MCU_COMBINED_HOST_INT_EN_MASK)

#define QWLANHOST_PMU_BPS_MODE_MASK                         0
#define QWLANFW_PMU_BPS_MODE_MASK                           ~(QWLANHOST_PMU_BPS_MODE_MASK)

#define QWLANHOST_BMU_BTQM_INTERRUPT_ENABLE_MASK            0
#define QWLANFW_BMU_BTQM_INTERRUPT_ENABLE_MASK              ~(QWLANHOST_BMU_BTQM_INTERRUPT_ENABLE_MASK)


#define MAC_INTR_MASK(__module, __intr, __bit, __user)  \
            (QWLAN_##__module##_##__intr##_##__bit##_MASK) & (QWLAN##__user##_##__module##_##__intr##_MASK)


/*===========================================================
  RXP Filter Cfg for Different Frames
===========================================================*/
typedef enum eFrameType {
    eMGMT_ASSOC_REQ = 0,
    eMGMT_ASSOC_RSP,            // 1
    eMGMT_REASSOC_REQ,          // 2
    eMGMT_REASSOC_RSP,          // 3
    eMGMT_PROBE_REQ,            // 4
    eMGMT_PROBE_RSP,            // 5
    eMGMT_RSVD1,                // 6
    eMGMT_RSVD2,                // 7
    eMGMT_BEACON,               // 8
    eMGMT_ATIM,                 // 9
    eMGMT_DISASSOC,             // 10
    eMGMT_AUTH,                 // 11
    eMGMT_DEAUTH,               // 12
    eMGMT_ACTION,               // 13
    eMGMT_ACTION_NOACK,         // 14
    eMGMT_RSVD4,                // 15
    eCTRL_RSVD1,                // 16
    eCTRL_RSVD2,                // 17
    eCTRL_RSVD3,                // 18
    eCTRL_RSVD4,                // 19
    eCTRL_RSVD5,                // 20
    eCTRL_RSVD6,                // 21
    eCTRL_RSVD7,                // 22
    eCTRL_CONTROL_WRAPPER,      // 23
    eCTRL_BAR,                  // 24
    eCTRL_BA,                   // 25
    eCTRL_PSPOLL,               // 26
    eCTRL_RTS,                  // 27
    eCTRL_CTS,                  // 28
    eCTRL_ACK,                  // 29
    eCTRL_CFEND,                // 30
    eCTRL_CFEND_CFACK,          // 31
    eDATA_DATA,                 // 32
    eDATA_DATA_CFACK,           // 33
    eDATA_DATA_CFPOLL,          // 34
    eDATA_DATA_CFACK_CFPOLL,    // 35
    eDATA_NULL,                 // 36
    eDATA_CFACK,                // 37
    eDATA_CFPOLL,               // 38
    eDATA_CFACK_CFPOLL,         // 39
    eDATA_QOSDATA,              // 40
    eDATA_QOSDATA_CFACK,        // 41
    eDATA_QOSDATA_CFPOLL,       // 42
    eDATA_QOSDATA_CFACK_CFPOLL, // 43
    eDATA_QOSNULL,              // 44
    eDATA_RSVD1,                // 45
    eDATA_QOS_CFPOLL,           // 46
    eDATA_QOS_CFACK_CFPOLL,     // 47
    eRSVD_RSVD0,            // 48
    eRSVD_RSVD1,            // 49
    eRSVD_RSVD2,            // 50
    eRSVD_RSVD3,            // 51
    eRSVD_RSVD4,            // 52
    eRSVD_RSVD5,            // 53
    eRSVD_RSVD6,            // 54
    eRSVD_RSVD7,            // 55
    eRSVD_RSVD8,            // 56
    eRSVD_RSVD9,            // 57
    eRSVD_RSVD10,            // 58
    eRSVD_RSVD11,            // 59
    eRSVD_RSVD12,            // 60
    eRSVD_RSVD13,            // 61
    eRSVD_RSVD14,            // 62
    eRSVD_RSVD15,            // 63
} tFrameType;

#define RXP_TYPE_SUBTYPE_MASK(x) ((x >= eDATA_DATA) ? (1 << (x - eDATA_DATA)) : (1 << x))

#endif //__ASSEMBLER__
#endif // __LIBRA_DEFS_H


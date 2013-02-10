#ifndef QWLANHW_COMMON_DEFS_H
#define QWLANHW_COMMON_DEFS_H

/*===========================================================================

FILE: 
   qwlanhw_commondefs.h

BRIEF DESCRIPTION:
   Definitions for Qualcomm WLAN architecture.

DESCRIPTION:
   Qualcomm has several generations of WLAN chipsets. Each architecture 
   has common features with variations. This file defines common definitions
   among Qualcomm WLAN architectures. Definition file for each architecture
   defines architecture specific part. 

                Copyright (c) 2008 Qualcomm Technologies, Inc.
                All Right Reserved.
                Qualcomm Technologies Confidential and Proprietary
===========================================================================*/

/*===========================================================================

                      EDIT HISTORY FOR FILE

$Header$
$DateTime$

when       who            what, where, why
--------   ---          -----------------------------------------------------
02/29/08   holee        Created

===========================================================================*/

/*===========================================================================
   ADDRESS MAP
===========================================================================*/

/** Base address of BMU command address space. This space is 16 MB big. */
#define QWLAN_BASEADDR_SRAM             0x00000000

/** Base address of BMU command address space. This space is 16 MB big. */
#define QWLAN_BASEADDR_BMU_COMMANDS     0x0f000000
/** End of BMU command address space. This is last address + 1 */
#define QWLAN_BASEADDR_BMU_COMMANDS_END 0x10000000

/*===========================================================================
   BMU
===========================================================================*/

/*---------------------------------------------------------------------------
 * BD/PDU Size
 *-------------------------------------------------------------------------*/

/** Size of a BD. BD size is fixed by hardware. */
#define QWLAN_BMU_BD_SIZE               128
/** Number of bits required to represent byte offset in a BD. 
   BD_SIZE = 1 << BD_SIZE_BITS 
*/
#define QWLAN_BMU_BD_SIZE_BITS          7
/** Size of a PDU. PDU size is also fixed by hardware. */
#define QWLAN_BMU_PDU_SIZE              128
/** Number of bits required to represent byte offset in a PDU. 
   PDU_SIZE = 1 << PDU_SIZE_BITS 
*/
#define QWLAN_BMU_PDU_SIZE_BITS         7
/** Number of bytes reserved to store next PDU index in PDU. Hardware uses
   last two bytes in PDU for PDU index, however two bytes before the next
   PDU index are set to 0 when DXE constructs a PDU. 
*/
#define QWLAN_BMU_PDU_NEXT_INDEX_SIZE   4
/** Number of bytes storing payload in a PDU. */
#define QWLAN_BMU_PDU_PAYLOAD_SIZE      (QWLAN_BMU_PDU_SIZE - QWLAN_BMU_PDU_NEXT_INDEX_SIZE)

/*---------------------------------------------------------------------------
 * BD/PDU Index
 *-------------------------------------------------------------------------*/

/** Invalid BD index. BMU treats 0 as NULL (invalid) BD index. */
#define QWLAN_BMU_BD_INDEX_INVALID      0
/** Last BD index. */
#define QWLAN_BMU_BD_INDEX_LAST         (QWLAN_BMU_BD_MAX_NUM - 1)
/** Last PDU index. */
#define QWLAN_BMU_PDU_INDEX_LAST        (QWLAN_BMU_PDU_MAX_NUM - 1)

/*---------------------------------------------------------------------------
 * BMU Commands
 *-------------------------------------------------------------------------*/

#ifndef __ASSEMBLER__

enum {
   QWLAN_BMU_CMD_PUSH_WQ = 0x00,
   QWLAN_BMU_CMD_POP_WQ = 0x04,
   QWLAN_BMU_CMD_WRITE_WQ_HEAD = 0x08,
   QWLAN_BMU_CMD_READ_WQ_HEAD = 0x0c,
   QWLAN_BMU_CMD_WRITE_WQ_TAIL = 0x10,
   QWLAN_BMU_CMD_READ_WQ_TAIL = 0x14,
   QWLAN_BMU_CMD_WRITE_WQ_NR = 0x18,
   QWLAN_BMU_CMD_READ_WQ_NR = 0x1c,
   QWLAN_BMU_CMD_WRITE_BD_POINTER = 0x20,
   QWLAN_BMU_CMD_READ_BD_POINTER = 0x24,
   QWLAN_BMU_CMD_RESERVATION_REQUEST = 0x28,
   QWLAN_BMU_CMD_GET_BD_PDU = 0x2c,
   QWLAN_BMU_CMD_RELEASE_PDU = 0x30,
   QWLAN_BMU_CMD_RELEASE_BD = 0x34,
   QWLAN_BMU_CMD_RELEASE_BD_PDU = 0x38,
   QWLAN_BMU_CMD_EXPANDED_RESERVATION_REQUEST = 0x3c,
   QWLAN_BMU_CMD_SW_TRACE_REFERENCE = 0x40,
};

enum {
   QWLAN_BMU_CMD_GET_BD_PDU_NONE = 0,
   QWLAN_BMU_CMD_GET_BD_PDU_BD = 1,
   QWLAN_BMU_CMD_GET_BD_PDU_PDU = 2,
};

#endif /* __ASSEMBLER__ */

/*===========================================================================
   DPU
===========================================================================*/

/*===========================================================================
   DXE
===========================================================================*/

/*---------------------------------------------------------------------------
 * DXE channel registers
 *-------------------------------------------------------------------------*/

#define QWLAN_DXE_0_CH_CTRL_REG_N(index) \
   (QWLAN_DXE_0_CH_CTRL_REG + (index) * 0x40)
#define QWLAN_DXE_0_CH_STATUS_REG_N(index) \
   (QWLAN_DXE_0_CH_STATUS_REG + (index) * 0x40)
#define QWLAN_DXE_0_CH_SZ_REG_N(index) \
   (QWLAN_DXE_0_CH_SZ_REG + (index) * 0x40)
#define QWLAN_DXE_0_CH_SADRL_REG_N(index) \
   (QWLAN_DXE_0_CH_SADRL_REG + (index) * 0x40)
#define QWLAN_DXE_0_CH_SADRH_REG_N(index) \
   (QWLAN_DXE_0_CH_SADRH_REG + (index) * 0x40)
#define QWLAN_DXE_0_CH_DADRL_REG_N(index) \
   (QWLAN_DXE_0_CH_DADRL_REG + (index) * 0x40)
#define QWLAN_DXE_0_CH_DADRH_REG_N(index) \
   (QWLAN_DXE_0_CH_DADRH_REG + (index) * 0x40)
#define QWLAN_DXE_0_CH_DESCL_REG_N(index) \
   (QWLAN_DXE_0_CH_DESCL_REG + (index) * 0x40)
#define QWLAN_DXE_0_CH_DESCH_REG_N(index) \
   (QWLAN_DXE_0_CH_DESCH_REG + (index) * 0x40)
#define QWLAN_DXE_0_CH_LST_DESCL_REG_N(index) \
   (QWLAN_DXE_0_CH_LST_DESCL_REG + (index) * 0x40)
#define QWLAN_DXE_0_CH_LST_DESCH_REG_N(index) \
   (QWLAN_DXE_0_CH_LST_DESCH_REG + (index) * 0x40)
#define QWLAN_DXE_0_CH_BD_REG_N(index) \
   (QWLAN_DXE_0_CH_BD_REG + (index) * 0x40)
#define QWLAN_DXE_0_CH_HEAD_REG_N(index) \
   (QWLAN_DXE_0_CH_HEAD_REG + (index) * 0x40)
#define QWLAN_DXE_0_CH_TAIL_REG_N(index) \
   (QWLAN_DXE_0_CH_TAIL_REG + (index) * 0x40)
#define QWLAN_DXE_0_CH_PDU_REG_N(index) \
   (QWLAN_DXE_0_CH_PDU_REG + (index) * 0x40)
#define QWLAN_DXE_0_CH_TSTMP_REG_N(index) \
   (QWLAN_DXE_0_CH_TSTMP_REG + (index) * 0x40)

/*===========================================================================
   MCU
===========================================================================*/

/*---------------------------------------------------------------------------
 * Mailboxes
 *-------------------------------------------------------------------------*/

/* Mailbox reset */
#define QWLAN_MCU_MB_CONTROL_MB_RESET_MASK \
   QWLAN_MCU_MB0_CONTROL_MB_RESET_MASK

/* Mailbox interrupt direction */
#define QWLAN_MCU_MB_CONTROL_INT_DIRECTION_ECPU_TO_HOST \
   0 
#define QWLAN_MCU_MB_CONTROL_INT_DIRECTION_HOST_TO_ECPU \
   QWLAN_MCU_MB0_CONTROL_INT_DIRECTION_MASK

#define QWLAN_MCU_MB_CONTROL_REG_N(index) \
   (QWLAN_MCU_MB0_CONTROL_REG + (index) * 8)
#define QWLAN_MCU_MB_CONTROL_COUNTERS_REG_N(index) \
   (QWLAN_MCU_MB0_CONTROL_COUNTERS_REG + (index) * 8)

/*---------------------------------------------------------------------------
 * Mutexes
 *-------------------------------------------------------------------------*/

#define QWLAN_MCU_MUTEX_REG_N(index) \
   (QWLAN_MCU_MUTEX0_REG + (index) * 4)

#define QWLAN_MCU_MUTEX_RESET_MASK              QWLAN_MCU_MUTEX0_RESET_MASK
#define QWLAN_MCU_MUTEX_RESET_OFFSET            QWLAN_MCU_MUTEX0_RESET_OFFSET
#define QWLAN_MCU_MUTEX_MAXCOUNT_MASK           QWLAN_MCU_MUTEX0_MAXCOUNT_MASK
#define QWLAN_MCU_MUTEX_MAXCOUNT_OFFSET         QWLAN_MCU_MUTEX0_MAXCOUNT_OFFSET
#define QWLAN_MCU_MUTEX_CURRENTCOUNT_MASK       QWLAN_MCU_MUTEX0_CURRENTCOUNT_MASK
#define QWLAN_MCU_MUTEX_CURRENTCOUNT_OFFSET     QWLAN_MCU_MUTEX0_CURRENTCOUNT_OFFSET

/*===========================================================================
   MTU
===========================================================================*/

/*---------------------------------------------------------------------------
 * Timer
 *-------------------------------------------------------------------------*/

#define QWLAN_MTU_TIMER_REG_N(index) \
   (QWLAN_MTU_TIMER_0_REG + (index) * 4)

#define QWLAN_MTU_SW_MATCH_REGISTER_REG(index) \
   (QWLAN_MTU_SW_MATCH_REGISTER_0_REG + (index) * 4)

/* Timer control register */
#define QWLAN_MTU_TIMER_CONTROL_DIRECTION_DOWN  0
#define QWLAN_MTU_TIMER_CONTROL_DIRECTION_UP    1

#define QWLAN_MTU_TIMER_CONTROL_UNIT_STOP       0
#define QWLAN_MTU_TIMER_CONTROL_UNIT_CLK        1
#define QWLAN_MTU_TIMER_CONTROL_UNIT_USEC       2
#define QWLAN_MTU_TIMER_CONTROL_UNIT_USER       3

#define QWLAN_MTU_TIMER_CONTROL_UNIT_N(index, value) \
   ((value) << (QWLAN_MTU_TIMER_CONTROL_SW_MTU_BASIC_UNIT_SELECT_OFFSET + (index) * 2))
#define QWLAN_MTU_TIMER_CONTROL_DIRECTION_N(index, value) \
   ((value) << (QWLAN_MTU_TIMER_CONTROL_SW_MTU_TIMER_UP_DOWN_CNTRL_OFFSET + (index) * 1))
#define QWLAN_MTU_TIMER_CONTROL_MASK_N(index) \
   (QWLAN_MTU_TIMER_CONTROL_UNIT_N(index, 3) | QWLAN_MTU_TIMER_CONTROL_DIRECTION_N(index, 1))

/*---------------------------------------------------------------------------
 * Timer assignment
 *-------------------------------------------------------------------------*/

#ifndef __ASSEMBLER__

enum {
   /* More to come... */
   QWLAN_MTU_TIMER_
};

#endif /* __ASSEMBLER__ */

/*---------------------------------------------------------------------------
 * Backoff timer
 *-------------------------------------------------------------------------*/

#define QWLAN_MTU_BKOF_CNT_REG(index) \
   (QWLAN_MTU_BKOF_CNT_0_REG + (index) * 4)

/*===========================================================================
   RXP
===========================================================================*/

/*---------------------------------------------------------------------------
 * Filter table
 *-------------------------------------------------------------------------*/

#define QWLAN_RXP_FRAME_FILTER_CONFIG_REG_N(index) \
   (QWLAN_RXP_FRAME_FILTER_CONFIG_REG + (index) * 4)

/*===========================================================================
   SCU
===========================================================================*/

/*---------------------------------------------------------------------------
 * UART
 *-------------------------------------------------------------------------*/

/* IIR */
#define QWLAN_SCU_UART_IIR_ID_NO_INTERRUPT   (0x1 << QWLAN_SCU_UART_IIR_ID_OFFSET)
#define QWLAN_SCU_UART_IIR_ID_MSR_CHANGE     (0x0 << QWLAN_SCU_UART_IIR_ID_OFFSET)
#define QWLAN_SCU_UART_IIR_ID_THR_EMPTY      (0x2 << QWLAN_SCU_UART_IIR_ID_OFFSET)
#define QWLAN_SCU_UART_IIR_ID_RBR_AVAIL      (0x4 << QWLAN_SCU_UART_IIR_ID_OFFSET)
#define QWLAN_SCU_UART_IIR_ID_LSR_CHANGE     (0x6 << QWLAN_SCU_UART_IIR_ID_OFFSET)
#define QWLAN_SCU_UART_IIR_ID_RBR_TIMEOUT    (0xc << QWLAN_SCU_UART_IIR_ID_OFFSET)

/* LCR */
#define QWLAN_SCU_UART_LCR_DLS_DATA5    (0 << QWLAN_SCU_UART_LCR_DLS_OFFSET)
#define QWLAN_SCU_UART_LCR_DLS_DATA6    (1 << QWLAN_SCU_UART_LCR_DLS_OFFSET)
#define QWLAN_SCU_UART_LCR_DLS_DATA7    (2 << QWLAN_SCU_UART_LCR_DLS_OFFSET)
#define QWLAN_SCU_UART_LCR_DLS_DATA8    (3 << QWLAN_SCU_UART_LCR_DLS_OFFSET)

#define QWLAN_SCU_UART_LCR_STOP_1BIT    (0 << QWLAN_SCU_UART_LCR_STOP_OFFSET)
#define QWLAN_SCU_UART_LCR_STOP_2BITS   (1 << QWLAN_SCU_UART_LCR_STOP_OFFSET)

#define QWLAN_SCU_UART_LCR_PEN_NONE     (0 << QWLAN_SCU_UART_LCR_PEN_OFFSET)
#define QWLAN_SCU_UART_LCR_PEN_ODD      (1 << QWLAN_SCU_UART_LCR_PEN_OFFSET)
#define QWLAN_SCU_UART_LCR_PEN_EVEN     (3 << QWLAN_SCU_UART_LCR_PEN_OFFSET)

/*===========================================================================
   TPE
===========================================================================*/

/*---------------------------------------------------------------------------
 * QWLAN_TPE_TPE_MCU_BD_BASED_TX_INT_FEEDBACK_REG
 * QWLAN_TPE_TPE_MCU_BD_BASED_TX_INT_FEEDBACK_1_REG
 *-------------------------------------------------------------------------*/

/* [0] : ACK_TO_VALID */
#define QWLAN_TPE_TPE_MCU_BD_BASED_TX_INT_FEEDBACK_ACK_TO_VALID_OFFSET                       0
#define QWLAN_TPE_TPE_MCU_BD_BASED_TX_INT_FEEDBACK_ACK_TO_VALID_MASK                         0x01

/* [3:1] : TPE_BMU_FEEDBACK[2:0] */
#define QWLAN_TPE_TPE_MCU_BD_BASED_TX_INT_FEEDBACK_TPE_BMU_FB_OFFSET                         1
#define QWLAN_TPE_TPE_MCU_BD_BASED_TX_INT_FEEDBACK_TPE_BMU_FB_MASK                           (0x07 << 1)

#define QWLAN_TPE_TPE_MCU_BD_BASED_TX_INT_FEEDBACK_TPE_BMU_FB_RESET                          0
#define QWLAN_TPE_TPE_MCU_BD_BASED_TX_INT_FEEDBACK_TPE_BMU_FB_DROP                           1
#define QWLAN_TPE_TPE_MCU_BD_BASED_TX_INT_FEEDBACK_TPE_BMU_FB_DROP_AND_REARB                 2
#define QWLAN_TPE_TPE_MCU_BD_BASED_TX_INT_FEEDBACK_TPE_BMU_FB_RESET_AND_REARB                3
#define QWLAN_TPE_TPE_MCU_BD_BASED_TX_INT_FEEDBACK_TPE_BMU_FB_RESET_AND_CHK_RETRY            4
#define QWLAN_TPE_TPE_MCU_BD_BASED_TX_INT_FEEDBACK_TPE_BMU_FB_RESET_CHK_RTRY_AND_REARB       5
#define QWLAN_TPE_TPE_MCU_BD_BASED_TX_INT_FEEDBACK_TPE_BMU_FB_RESET_AND_REARB_UPDATE_RETRY   6

#endif /* QWLANHW_COMMON_DEFS_H */


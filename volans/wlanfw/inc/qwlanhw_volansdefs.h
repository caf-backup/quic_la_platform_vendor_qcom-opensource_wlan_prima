#ifndef QWLANHW_VOLANS_DEFS_H
#define QWLANHW_VOLANS_DEFS_H

/*===========================================================================

FILE: 
   qwlanhw_volansdefs.h

BRIEF DESCRIPTION:
   Definitions for Volans architecture.

DESCRIPTION:
   Register definitions do not tell everything about the architecture. 
   On top of register definitions, additional information about the 
   hardware architecture are described in this file. 

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
   Interrupts (aCPU)
===========================================================================*/

/* Top level interrupts */
#define QWLAN_IRQ_ECPU_MTU_WATCHDOG_WARNING       0
#define QWLAN_IRQ_ECPU_UART                       1
#define QWLAN_IRQ_ECPU_SW_0                       2
#define QWLAN_IRQ_ECPU_MTU                        3 /* combined */
#define QWLAN_IRQ_ECPU_MCU_MAILBOX_0              4
#define QWLAN_IRQ_ECPU_MCU_MAILBOX_1              5
#define QWLAN_IRQ_ECPU_MCU_MAILBOX_2              6
#define QWLAN_IRQ_ECPU_MCU_MAILBOX_3              7
#define QWLAN_IRQ_ECPU_MCU_MAILBOX_N(mboxIndex)   (QWLAN_IRQ_ECPU_MAILBOX_0 + (mboxIndex))
#define QWLAN_IRQ_ECPU_PMU_POWERED_UP             8
#define QWLAN_IRQ_ECPU_DXE_CHANNEL_0              9
#define QWLAN_IRQ_ECPU_DXE_CHANNEL_1              10
#define QWLAN_IRQ_ECPU_DXE_CHANNEL_2              11
#define QWLAN_IRQ_ECPU_DXE_CHANNEL_N(chIndex)     (QWLAN_IRQ_ECPU_DXE_CHANNEL_0 + (chIndex))
#define QWLAN_IRQ_ECPU_RXP                        12
#define QWLAN_IRQ_ECPU_COMBINED                   13 /* combined */
#define QWLAN_IRQ_ECPU_PMU_MCU_BPS_DISABLED       14
#define QWLAN_IRQ_ECPU_RPE                        15
#define QWLAN_IRQ_ECPU_MTU_TIMER_4                16
#define QWLAN_IRQ_ECPU_ADU                        17
#define QWLAN_IRQ_ECPU_MTU_TIMER_5                18
#define QWLAN_IRQ_ECPU_MIF                        21
#define QWLAN_IRQ_ECPU_BTC                        22
#define QWLAN_IRQ_ECPU_PHY_FIQ                    23
#define QWLAN_IRQ_ECPU_PHY_IRQ                    24
#define QWLAN_IRQ_ECPU_BMU_WQ_DATA_AVAILABLE      25 /* combined */
#define QWLAN_IRQ_ECPU_PHY_HIF                    26
#define QWLAN_IRQ_ECPU_MTU_TIMER_6                27
#define QWLAN_IRQ_ECPU_MTU_TIMER_7                28
#define QWLAN_IRQ_ECPU_MTU_TIMER_4TO7_N(index)    (((index) == 4) ? QWLAN_IRQ_ECPU_MTU_TIMER_4 : \
                                                  (((index) == 5) ? QWLAN_IRQ_ECPU_MTU_TIMER_5 : \
                                                  (((index) == 6) ? QWLAN_IRQ_ECPU_MTU_TIMER_6 : \
                                                  QWLAN_IRQ_ECPU_MTU_TIMER_7)))
#define QWLAN_IRQ_ECPU_SW_1                       31

#define QWLAN_IRQ_ECPU_TOP_NUM                    32

#define QWLAN_IRQ_ECPU_MAILBOX_0                  QWLAN_IRQ_ECPU_MCU_MAILBOX_0
#define QWLAN_IRQ_ECPU_MAILBOX_1                  QWLAN_IRQ_ECPU_MCU_MAILBOX_1
#define QWLAN_IRQ_ECPU_MAILBOX_2                  QWLAN_IRQ_ECPU_MCU_MAILBOX_2
#define QWLAN_IRQ_ECPU_MAILBOX_3                  QWLAN_IRQ_ECPU_MCU_MAILBOX_3
#define QWLAN_IRQ_ECPU_MAILBOX_N                  QWLAN_IRQ_ECPU_MCU_MAILBOX_N

/* MTU interrupts - virtual IRQ */
#define QWLAN_IRQ_ECPU_MTU_START                  QWLAN_IRQ_ECPU_TOP_NUM
#define QWLAN_IRQ_ECPU_MTU_TIMER_0                (QWLAN_IRQ_ECPU_MTU_START + 0)
#define QWLAN_IRQ_ECPU_MTU_TIMER_1                (QWLAN_IRQ_ECPU_MTU_START + 1)
#define QWLAN_IRQ_ECPU_MTU_TIMER_2                (QWLAN_IRQ_ECPU_MTU_START + 2)
#define QWLAN_IRQ_ECPU_MTU_TIMER_3                (QWLAN_IRQ_ECPU_MTU_START + 3)
#define QWLAN_IRQ_ECPU_MTU_TIMER_N(timerIndex)    (((timerIndex) < 4) ? \
                                                     (QWLAN_IRQ_ECPU_MTU_START + (timerIndex)) : \
                                                     QWLAN_IRQ_ECPU_MTU_TIMER_4TO7_N(timerIndex))
#define QWLAN_IRQ_ECPU_MTU_BACKOFF_0              (QWLAN_IRQ_ECPU_MTU_START + 4)
#define QWLAN_IRQ_ECPU_MTU_BACKOFF_1              (QWLAN_IRQ_ECPU_MTU_START + 5)
#define QWLAN_IRQ_ECPU_MTU_SIFS_TIMEOUT           (QWLAN_IRQ_ECPU_MTU_START + 6)
#define QWLAN_IRQ_ECPU_MTU_PIFS_TIMEOUT           (QWLAN_IRQ_ECPU_MTU_START + 7)
#define QWLAN_IRQ_ECPU_MTU_RESP_TIMEOUT_EARLY_PKTDET (QWLAN_IRQ_ECPU_MTU_START + 8)
#define QWLAN_IRQ_ECPU_MTU_RESP_TIMEOUT_PKTDET    (QWLAN_IRQ_ECPU_MTU_START + 9)
#define QWLAN_IRQ_ECPU_MTU_RESP_TIMEOUT_PKTPUSH   (QWLAN_IRQ_ECPU_MTU_START + 10)
#define QWLAN_IRQ_ECPU_MTU_WATCHDOG_PROTECTION_ERROR (QWLAN_IRQ_ECPU_MTU_START + 11)
#define QWLAN_IRQ_ECPU_MTU_WATCHDOG_ENABLE_DISABLE_ERROR (QWLAN_IRQ_ECPU_MTU_START + 12)
#define QWLAN_IRQ_ECPU_MTU_ERROR                  (QWLAN_IRQ_ECPU_MTU_START + 13)
#define QWLAN_IRQ_ECPU_MTU_MCU_TX_BOUNDARY_INT    (QWLAN_IRQ_ECPU_MTU_START + 14)

#define QWLAN_IRQ_ECPU_MTU_NUM                    15

/* BMU WQ interrupts - virtual IRQ */
#define QWLAN_IRQ_ECPU_BMU_WQ_START               (QWLAN_IRQ_ECPU_TOP_NUM + QWLAN_IRQ_ECPU_MTU_NUM)
#define QWLAN_IRQ_ECPU_BMU_WQ_N(wqIndex)          (QWLAN_IRQ_ECPU_BMU_WQ_START + (wqIndex))

#define QWLAN_IRQ_ECPU_BMU_WQ_2                   QWLAN_IRQ_ECPU_BMU_WQ_N(2)
#define QWLAN_IRQ_ECPU_BMU_WQ_3                   QWLAN_IRQ_ECPU_BMU_WQ_N(3)
#define QWLAN_IRQ_ECPU_BMU_WQ_4                   QWLAN_IRQ_ECPU_BMU_WQ_N(4)
#define QWLAN_IRQ_ECPU_BMU_WQ_5                   QWLAN_IRQ_ECPU_BMU_WQ_N(5)
#define QWLAN_IRQ_ECPU_BMU_WQ_6                   QWLAN_IRQ_ECPU_BMU_WQ_N(6)
#define QWLAN_IRQ_ECPU_BMU_WQ_7                   QWLAN_IRQ_ECPU_BMU_WQ_N(7)
#define QWLAN_IRQ_ECPU_BMU_WQ_8                   QWLAN_IRQ_ECPU_BMU_WQ_N(8)
#define QWLAN_IRQ_ECPU_BMU_WQ_9                   QWLAN_IRQ_ECPU_BMU_WQ_N(9)
#define QWLAN_IRQ_ECPU_BMU_WQ_10                  QWLAN_IRQ_ECPU_BMU_WQ_N(10)
#define QWLAN_IRQ_ECPU_BMU_WQ_11                  QWLAN_IRQ_ECPU_BMU_WQ_N(11)
#define QWLAN_IRQ_ECPU_BMU_WQ_12                  QWLAN_IRQ_ECPU_BMU_WQ_N(12)
#define QWLAN_IRQ_ECPU_BMU_WQ_13                  QWLAN_IRQ_ECPU_BMU_WQ_N(13)
#define QWLAN_IRQ_ECPU_BMU_WQ_14                  QWLAN_IRQ_ECPU_BMU_WQ_N(14)
#define QWLAN_IRQ_ECPU_BMU_WQ_15                  QWLAN_IRQ_ECPU_BMU_WQ_N(15)
#define QWLAN_IRQ_ECPU_BMU_WQ_16                  QWLAN_IRQ_ECPU_BMU_WQ_N(16)
#define QWLAN_IRQ_ECPU_BMU_WQ_17                  QWLAN_IRQ_ECPU_BMU_WQ_N(17)
#define QWLAN_IRQ_ECPU_BMU_WQ_18                  QWLAN_IRQ_ECPU_BMU_WQ_N(18)
#define QWLAN_IRQ_ECPU_BMU_WQ_19                  QWLAN_IRQ_ECPU_BMU_WQ_N(19)
#define QWLAN_IRQ_ECPU_BMU_WQ_20                  QWLAN_IRQ_ECPU_BMU_WQ_N(20)
#define QWLAN_IRQ_ECPU_BMU_WQ_21                  QWLAN_IRQ_ECPU_BMU_WQ_N(21)
#define QWLAN_IRQ_ECPU_BMU_WQ_22                  QWLAN_IRQ_ECPU_BMU_WQ_N(22)
#define QWLAN_IRQ_ECPU_BMU_WQ_23                  QWLAN_IRQ_ECPU_BMU_WQ_N(23)
#define QWLAN_IRQ_ECPU_BMU_WQ_24                  QWLAN_IRQ_ECPU_BMU_WQ_N(24)

#define QWLAN_IRQ_ECPU_BMU_WQ_NUM                 25

/* Number of interrupt sources */
#define QWLAN_IRQ_ECPU_LEVEL_NUM                  2
#define QWLAN_IRQ_ECPU_NUM                        (QWLAN_IRQ_ECPU_TOP_NUM + QWLAN_IRQ_ECPU_MTU_NUM + QWLAN_IRQ_ECPU_BMU_WQ_NUM)

/*===========================================================================
   BMU
===========================================================================*/

/*---------------------------------------------------------------------------
 * WQ
 *-------------------------------------------------------------------------*/

/** Number of BMU WQs */
#define QWLAN_BMU_WQ_NUM                27

/*---------------------------------------------------------------------------
 * WQ Assignment
 *-------------------------------------------------------------------------*/
#ifndef __ASSEMBLER__

enum {
   /* BMU, for maintaining idle BD/PDUs */
   QWLAN_BMU_WQ_BMU_IDLE_BD = 0,
   QWLAN_BMU_WQ_BMU_IDLE_PDU = 1,
   /*...*/
   QWLAN_BMU_WQ_ADU_TX = 23,
   QWLAN_BMU_WQ_ADU_RX = 24,
};

#endif /*__ASSEMBLER__*/
/*---------------------------------------------------------------------------
 * Module ID Assignment
 *-------------------------------------------------------------------------*/

#ifndef __ASSEMBLER__

enum {
   QWLAN_BMU_MODULE_ID_RXP = 0,
   QWLAN_BMU_MODULE_ID_DPU_TX = 1,
   QWLAN_BMU_MODULE_ID_DPU_RX = 2,
   QWLAN_BMU_MODULE_ID_ADU = 3,
   QWLAN_BMU_MODULE_ID_RPE = 4,
   QWLAN_BMU_MODULE_ID_DXE = 5,
   QWLAN_BMU_MODULE_ID_SW = 62,
   QWLAN_BMU_MODULE_ID_SINK = 63,
};

#endif /* __ASSEMBLER__ */

/*---------------------------------------------------------------------------
 * BD/PDU Number
 *-------------------------------------------------------------------------*/

/** Maximum number of BD supported by BMU. This includes a BD index 0. Though 
   it is treated as invalid BD index, it takes space in BD/PDU memory.
*/
#define QWLAN_BMU_BD_MAX_NUM            1024
/** Maximum number of PDU supported by BMU. This also include BDs because 
   in some configuration BDs are interchangeable with PDUs. 16 bits are used
   to describe PDU index, so possible number of PDUs is 2^16. 
*/
#define QWLAN_BMU_PDU_MAX_NUM           65536

/*===========================================================================
   DXE
===========================================================================*/

/*---------------------------------------------------------------------------
 * Channels
 *-------------------------------------------------------------------------*/

/** Number of channels available in the system */
#define QWLAN_DXE_CHANNEL_NUM           3


/*===========================================================================
   MCU
===========================================================================*/
    
/*---------------------------------------------------------------------------
 * Mailbox
 *-------------------------------------------------------------------------*/
        
/** Number of mailboxes available in the system */
#define QWLAN_MCU_MAILBOX_NUM           4

/*---------------------------------------------------------------------------
 * Interrupts
 *-------------------------------------------------------------------------*/

#define QWLAN_MCU_ACPU_IRQ_LEVELS           2

#define QWLAN_MCU_ACPU_INT_STATUS_REG_N(index) \
   (QWLAN_MCU_FIQ_STATUS_REG + (index) * 0x10)
#define QWLAN_MCU_ACPU_INT_ENABLE_REG_N(index) \
   (QWLAN_MCU_FIQ_EN_REG + (index) * 0x10)
#define QWLAN_MCU_ACPU_INT_MASKED_STATUS_REG_N(index) \
   (QWLAN_MCU_FIQ_MASKED_STATUS_REG + (index) * 0x10)
#define QWLAN_MCU_ACPU_INT_CLEAR_REG_N(index) \
   (QWLAN_MCU_FIQ_CLEAR_REG + (index) * 0x10)

/*---------------------------------------------------------------------------
 * Software interrupts
 *-------------------------------------------------------------------------*/

#define QWLAN_MCU_SW_INT_NUM              2

#define QWLAN_MCU_SW_INT_REG_N(index) \
   (QWLAN_MCU_SW_INT_0_REG + (index) * 0x04)

/*===========================================================================
   MTU
===========================================================================*/

/*---------------------------------------------------------------------------
 * Timer registers
 *-------------------------------------------------------------------------*/

#define QWLAN_MTU_TIMER_CONTINUOUS_CONTROL_REG \
   QWLAN_MTU_TIMER_CONTROL11TO8_REG
#define QWLAN_MTU_TIMER_CONTINUOUS_CONTROL_ENABLE_N(index) \
   (1 << ((index) + QWLAN_MTU_TIMER_CONTROL11TO8_SW_MTU_CONTINUOUS_VALID_OFFSET))

/*---------------------------------------------------------------------------
 * Timers
 *-------------------------------------------------------------------------*/

/** Number of timers available in the system */
#define QWLAN_MTU_TIMER_NUM             8

/*---------------------------------------------------------------------------
 * Architecture dependent timer assignment
 *-------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
 * Backoff timers
 *-------------------------------------------------------------------------*/

/** Number of backoff timers available in the system */
#define QWLAN_MTU_BACKOFF_NUM           8

/*---------------------------------------------------------------------------
 * Backoff timer assignment
 *-------------------------------------------------------------------------*/

#ifndef __ASSEMBLER__

enum {
   /* More to come... */
   QWLAN_MTU_BACKOFF_
};

/*---------------------------------------------------------------------------
 * Mutex
 *-------------------------------------------------------------------------*/

/** Number of mutexes available in the system */
#define QWLAN_MCU_MUTEX_NUM             4

#endif /* __ASSEMBLER__ */

#endif /* QWLANHW_VOLANS_DEFS_H */


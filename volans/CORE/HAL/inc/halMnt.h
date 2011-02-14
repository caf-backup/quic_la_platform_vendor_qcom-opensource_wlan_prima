
/*
 * Airgo Networks, Inc proprietary. All rights reserved.
 * halMnt.h:    HAL thread process header file.
 * Author:   V. K. Kandarpa
 * Date:     01/29/2002
 *
 * History:-
 * Date      Modified by            Modification Information
 * --------------------------------------------------------------------------
 *
 */

# ifndef __HALMNT_H
# define __HALMNT_H

# include "aniGlobal.h"
# include "sirCommon.h"
# include "sirDebug.h"

/* Defines */

#define HAL_STAT_TIMER_VAL_IN_SECONDS 10 // 10 seconds

# define HAL_STAT_TIMER_VALUE \
         (HAL_STAT_TIMER_VAL_IN_SECONDS * SYS_TICKS_PER_SECOND)
# define HAL_TEMP_MEAS_TIMER_VALUE   (300 * SYS_TICKS_PER_SECOND) // 5 min.

/// Initial timer duration for the temperature monitoring timer, used for the open TPC
/// functionality.
#define HAL_OPEN_TPC_TEMP_MEAS_TIMER_VALUE   (120 * SYS_TICKS_PER_SECOND) // 120 secs.

# define HAL_AGC_RESET_TIMER_DEFAULT (20 / SYS_TICK_DUR_MS)       // 20 ms

# define HAL_2_MINUTE_COUNT (120 / HAL_STAT_TIMER_VAL_IN_SECONDS)

/* Function */

tSirRetStatus halProcessStartEvent(tpAniSirGlobal);

// ------------------------------------------------------------
/// MMH - Mailbox Message Handler Interfaces
/* Defines */

# define HAL_MMH_MB_MSG_TYPE_MASK    0xFF00

// RxMB Functions

// TxMB Functions

tSirRetStatus halMmhForwardMBmsg(void*, tSirMbMsg*);

// ------------------------------------------------------------
/// statistics
void halMntProcessGetRadioStatsReq(tpAniSirGlobal pMac, tpSirMsgQ pMsg);
void halMntProcessGetPerStaStatsReq(tpAniSirGlobal pMac, tpSirMsgQ pMsg);
void halMntProcessGetAggregateStaStatsReq(tpAniSirGlobal pMac, tpSirMsgQ pMsg);
void halMntProcessGetClearStatsReq(tpAniSirGlobal pMac, tpSirMsgQ pMsg);
tSirRetStatus halMntGetPerStaStats(tpAniSirGlobal pMac, tANI_U16 staId);
void halMntGetPeriodicRadioStats(tpAniSirGlobal pMac);





# endif /* __HALMNT_H */

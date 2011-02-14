/*
 * Airgo Networks, Inc proprietary. All rights reserved.
 * This file halTimer.h contains the utility function HAL
 * uses for timer related functions. 
 * 
 * Author:        Susan Tsao
 * Date:          01/04/07
 * --------------------------------------------------------------------
 */

#ifndef _HALTIMER_H_
#define _HALTIMER_H

#include "aniGlobal.h"    // tpAniSirGlobal

//hal timers
enum
{
    //BA Activity check timer.
    eHAL_BA_ACT_CHK_TIMER,
    //PS Command Response timeout
    eHAL_PS_RSP_TIMEOUT_TIMER
};

void halDeactivateAndChangeTimer(tpAniSirGlobal pMac, tANI_U32 timerId);
tSirRetStatus halTimersCreate(tpAniSirGlobal pMac);
tSirRetStatus halTimersDestroy(tpAniSirGlobal pMac);

#endif /* _HALTIMER_H_ */


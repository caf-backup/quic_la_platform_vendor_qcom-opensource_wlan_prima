#ifndef HAL_TL_FLUSH_H__
#define HAL_TL_FLUSH_H__
/*
 * Qualcomm Inc proprietary. All rights reserved.
 *
 * This file contains Routines related hal TL Flush operation
 *
 * Date:        08/3/08
 * --------------------------------------------------------------------
 *
 */
#include <sirTypes.h>
#include <aniGlobal.h>
#include <ani_assert.h>
#include <halDebug.h>
#include <halInternal.h>

#include <wlan_qct_hal.h> //TL <-> HAL Structures and macros

/* ------------ Function Prototypes ----------------*/
extern tSirRetStatus halTLProcessFlushReq(tpAniSirGlobal pMac, tSirMsgQ *pMsg);

#endif /* HAL_TL_FLUSH_H__ */

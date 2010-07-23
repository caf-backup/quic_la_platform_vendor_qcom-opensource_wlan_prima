/*******************************************************************
 *
 *    DESCRIPTION: Provides an intermediate interface to Vos so
 *                  so that phy code can also be built on 
 *                  WinCE and other systems that have a different implementation for Vos functionality.
 *
 *    AUTHOR:   Mark Nelson
 *
 *******************************************************************/
#ifndef __HALPHYVOS_H
#define __HALPHYVOS_H

#include "vos_event.h"

#define HAL_PHY_SET_CHAN_EVENT_TYPE  vos_event_t

eHalStatus halPhy_VosEventInit(tHalHandle hHal);
eHalStatus halPhy_VosEventDestroy(tHalHandle hHal);
eHalStatus halPhy_VosEventWaitSetChannel(tHalHandle hHal);
eHalStatus halPhy_VosEventResetSetChannel(tHalHandle hHal);
eHalStatus halPhy_HandlerFwRspMsg(tHalHandle hHal, void* pFwMsg);
void halPhyWaitForInterrupt(tHalHandle hHal, tANI_U32 msgType);

#endif /*__HALPHYVOS_H*/


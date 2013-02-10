/** ------------------------------------------------------------------------- *


    \file halMacStats.h

    \brief Header file for hal Mac Stats.

    \author Kiran V

    Copyright (C) 2007 Qualcomm Technologies, Inc.

   ========================================================================= */

#ifndef _HAL_MAC_STATS_H_
#define _HAL_MAC_STATS_H_

#include "halTypes.h"    // eHalStatus tHalHandle

eHalStatus halMacStats_Open(tHalHandle hHal, void *arg);
eHalStatus halMacStats_Close(tHalHandle hHal, void *arg);

#endif /* _HAL_MAC_STATS_H_ */


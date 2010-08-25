/*
 * Airgo Networks, Inc proprietary. All rights reserved.
 * halPMU.h:  Provides all the APIs to the PMU Hardware Block.
 * Author:    Neelay Das
 * Date:      11/20/2006
 *
 * --------------------------------------------------------------------------
 */
#ifndef _HALPMU_H_
#define _HALPMU_H_

#include "halTypes.h"
#include "aniGlobal.h"

eHalStatus halPMU_Start(tHalHandle hHal, void *arg);

/*
 * Setting the address where register list is present in the ADU
 */
eHalStatus halPmu_SetAduReInitAddress(tHalHandle hHal, tANI_U32 address);

/*
 *  Enable/Disable the initialization of the ADU registers list
 */
eHalStatus halPmu_AduReinitEnableDisable(tHalHandle hHal, tANI_U8 enable);

/*
 * Clear the ADU_REINIT_DONE bit in the ADU_REINIT register
 */
void halPmu_ClearAduReinitDone(tpAniSirGlobal pMac);

#endif /* _HALPMU_H_ */




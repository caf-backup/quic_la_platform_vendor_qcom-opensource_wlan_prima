/*
 * Airgo Networks, Inc proprietary. All rights reserved.
 * halPMU.c:  Provides all the APIs to the PMU Hardware Block.
 * Author:    Neelay Das
 * Date:      11/20/2006
 *
 * --------------------------------------------------------------------------
 */

#include "halInternal.h"
#ifdef WLAN_HAL_VOLANS
#include "volansDefs.h"
#else
#include "libraDefs.h"
#endif
#include "palApi.h"
#if !defined ANI_OS_TYPE_OSX
#include "dvtDebug.h"
#endif
#include "ani_assert.h"
#include "halDebug.h"

// Routine to initialize the PMU
eHalStatus
halPMU_Start(
    tHalHandle   hHal,
    void        *arg)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hHal;

    // DXE clock gating has been disabled like Libra 
    halWriteRegister(pMac,QWLAN_SCU_SYS_DISABLE_CLK_GATING_REG, 
                     QWLAN_SCU_SYS_DISABLE_CLK_GATING_DXE_DISABLE_CLK_GATING_MASK);
    return eHAL_STATUS_SUCCESS;
}


/* 
 * Setting the address where register list is present in the ADU
 */
eHalStatus halPmu_SetAduReInitAddress(tHalHandle hHal, tANI_U32 address)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hHal;
    
    halWriteRegister(pMac, 
            QWLAN_PMU_ADU_REINIT_ADDRESS_REG, address);
    
    return eHAL_STATUS_SUCCESS;
}


/*
 *  Enable/Disable the initialization of the ADU registers list
 */
eHalStatus halPmu_AduReinitEnableDisable(tHalHandle hHal, tANI_U8 enable)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hHal;
    tANI_U32 regValue = 0;

    halReadRegister(pMac,
            QWLAN_PMU_ADU_REINIT_REG, &regValue);
    
    if(enable) {
        regValue |= QWLAN_PMU_ADU_REINIT_PMU_ADU_REINIT_ENABLE_MASK; 
    } else {
        regValue &= ~QWLAN_PMU_ADU_REINIT_PMU_ADU_REINIT_ENABLE_MASK;
    }
    
    halWriteRegister(pMac,
            QWLAN_PMU_ADU_REINIT_REG, regValue);

    return eHAL_STATUS_SUCCESS;
}


void halPmu_ClearAduReinitDone(tpAniSirGlobal pMac)
{
    tANI_U32 regValue = 0;

    // Read the ADU_REINIT register
    halReadRegister(pMac, QWLAN_PMU_ADU_REINIT_REG, &regValue);

    // Software must write 1 to ADU_PMU_REINIT_DONE bit to clear
    regValue |= QWLAN_PMU_ADU_REINIT_ADU_PMU_REINIT_DONE_MASK;
    halWriteRegister(pMac, QWLAN_PMU_ADU_REINIT_REG, regValue);
}

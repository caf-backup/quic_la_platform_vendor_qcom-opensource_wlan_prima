/*
 * Airgo Networks, Inc proprietary. All rights reserved.
 * halPMU.c:  Provides all the APIs to the PMU Hardware Block.
 * Author:    Neelay Das
 * Date:      11/20/2006
 *
 * --------------------------------------------------------------------------
 */

#include "halInternal.h"
#include "libraDefs.h"
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
    tANI_U32 regValue = 0;

    // Enable All clock gating except DXE, since there is a bug
    // in DXE clock gating.
    halWriteRegister(pMac,QWLAN_SCU_SYS_DISABLE_CLK_GATING_REG,
                     QWLAN_SCU_SYS_DISABLE_CLK_GATING_DXE_DISABLE_CLK_GATING_MASK);

    // enable pmu_ana_deep_sleep_en in ldo_ctrl_reg
    regValue  = QWLAN_PMU_LDO_CTRL_REG_PMU_ANA_DEEP_SLEEP_EN_MASK |
                QWLAN_PMU_LDO_CTRL_REG_PMU_ANA_1P23_LPM_AON_MASK_MASK |
                QWLAN_PMU_LDO_CTRL_REG_PMU_ANA_1P23_LPM_SW_MASK_MASK |
                QWLAN_PMU_LDO_CTRL_REG_PMU_ANA_2P3_LPM_MASK_MASK;
    halWriteRegister(pMac, QWLAN_PMU_LDO_CTRL_REG_REG, regValue);

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

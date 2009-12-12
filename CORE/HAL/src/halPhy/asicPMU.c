/**
 *
   Airgo Networks, Inc proprietary.
   All Rights Reserved, Copyright 2005
   This program is the confidential and proprietary product of Airgo Networks Inc.
   Any Unauthorized use, reproduction or transfer of this program is strictly prohibited.


   asicPMU.cc: Hardware block abstraction of Power Management block
   Author:  Mark Nelson
   Date:    3/10/05

   History -
   Date        Modified by              Modification Information
  --------------------------------------------------------------------------

 */
#include "ani_assert.h"
#include "sys_api.h"

void asicPMUInit(tpAniSirGlobal pMac)
{
    asicSetPMUManualEnMode(pMac, PMU_AUTO_EN);
}


void asicPMURadioControl(tpAniSirGlobal pMac, ePMURadioControl radio)
{
    assert(pMac != 0);

    switch (radio)
    {
        case PMU_RADIO_OFF:
            //halWriteRegister(pMac, PMU_RF_CTL_REG, PMU_RF_CTL_RADIO_OFF);
            break;

        case PMU_RADIO_2_4_GHZ:
            //halWriteRegister(pMac, PMU_RF_CTL_REG, PMU_RF_CTL_RADIO_2_4GHZ);
            break;

        case PMU_RADIO_5_GHZ:
            //halWriteRegister(pMac, PMU_RF_CTL_REG, PMU_RF_CTL_RADIO_5GHZ);
            break;

        case PMU_RADIO_5_GHZ_UNII:
            //halWriteRegister(pMac, PMU_RF_CTL_REG, (PMU_RF_CTL_RADIO_5GHZ | PMU_RF_CTL_UNII_BAND_SEL_MASK));
            break;
        default:
            assert(0);
            break;
    }
}

void asicSetPMUManualEnMode(tpAniSirGlobal pMac, ePMUEnableStatus pmuMode)
{
    assert(pMac != 0);
    assert(pmuMode >= PMU_AUTO_EN);
    assert(pmuMode < MAX_PMU_STATE);

    switch (pmuMode)
    {
        case PMU_CAL_DCO_EN:
            SET_PMU_CAL_DCO_EN_MODE;
            break;

        case PMU_CAL_IQ_EN:
            SET_PMU_CAL_IQ_EN_MODE;
            break;

        case PMU_CAL_TX_LO_EN:
            SET_PMU_CAL_TX_LO_EN_MODE;
            break;

        default:
        case PMU_AUTO_EN:
            SET_PMU_NO_MANUAL_ENABLES;
            break;
    }
}



//returns previous state of psDisable - !single threaded implementation
tANI_BOOLEAN asicSetPowerDisable(tpAniSirGlobal pMac, tANI_BOOLEAN onOff)
{
    tANI_BOOLEAN retVal = pMac->hphy.pmu.psDisabled;

    assert(pMac != 0);
    assert((onOff == eANI_BOOLEAN_ON) || (onOff == eANI_BOOLEAN_OFF));

    if (onOff == eANI_BOOLEAN_ON)
    {
       //disable power save
/*  TODO: Find appropriate control for Taurus
        halWriteRegister(pMac, TIT_PMU_PMU_CONTROL_REG,
                     (TIT_PMU_PMU_CONTROL_PS_DISABLE_MASK |
                      TIT_PMU_PMU_CONTROL_ROOTCLK_BPS_DISABLE_MASK
                     )
                 );
*/
       phyLog(pMac, LOG2, "power-save disabled\n");
       pMac->hphy.pmu.psDisabled = eANI_BOOLEAN_ON;
    }
    else
    {
       //always keep the clk80 and clk120 bits disabled,
       // because they are not needed and cause spurs in the RF which will affect transmit/receive
/*  TODO: Find appropriate control for Taurus
        halWriteRegister(pMac, TIT_PMU_PMU_CONTROL_REG,
                     (TIT_PMU_PMU_CONTROL_AGC_BEACONPS_EN_MASK |
                      TIT_PMU_PMU_CONTROL_BEACONPS_DAC_PWDN_MASK
                     )
                 );
*/
       phyLog(pMac, LOG2, "power-save enabled\n");
       pMac->hphy.pmu.psDisabled = eANI_BOOLEAN_OFF;
    }

    return (retVal);
}


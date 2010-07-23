/**
 *
   Airgo Networks, Inc proprietary.
   All Rights Reserved, Copyright 2005
   This program is the confidential and proprietary product of Airgo Networks Inc.
   Any Unauthorized use, reproduction or transfer of this program is strictly prohibited.


   asicAGC.cc: Abstracts the Phy AGC Engine
   Author:  Mark Nelson
   Date:    3/4/05

   History -
   Date        Modified by              Modification Information
  --------------------------------------------------------------------------

 */

#include "ani_assert.h"
#include "sys_api.h"


eHalStatus asicAGCReset(tpAniSirGlobal pMac)
{
    eHalStatus retVal;
    assert(pMac != 0);

    SET_PHY_REG(pMac->hHdd, AGC_RESET_REG, 1);
    SET_PHY_REG(pMac->hHdd, AGC_RESET_REG, 0);

    return (retVal);
}

eHalStatus asicSetAgcCCAMode(tpAniSirGlobal pMac, ePhyCCAMode primaryCcaMode, ePhyCCAMode secondaryCcaMode)
{
    eHalStatus retVal;
    assert(pMac != 0);
    assert((primaryCcaMode >= PHY_CCA_FORCED_ON) && (primaryCcaMode <= PHY_CCA_ED_OR_CD_AND_CS));
    assert((secondaryCcaMode >= PHY_CCA_FORCED_ON) && (secondaryCcaMode <= PHY_CCA_SEC_ED40_AND_NOR_PKTDET40_PKTDET20));

/*
        SET_PHY_REG(pMac->hHdd, AGC_CCA_MODE_REG,
                    (AGC_CCA_MODE_SEC_MASK & ((tANI_U32 )secondaryCcaMode << AGC_CCA_MODE_SEC_OFFSET)) |
                    (AGC_CCA_MODE_PRI_MASK & ((tANI_U32 )primaryCcaMode << AGC_CCA_MODE_PRI_OFFSET))
                 );
*/

        //TODO: CCA temporary fix
        //per James' email until they figure out the problem related to CCA and TXCTL_RAMP_DOWN_REG
        // problem is that without setting the TXCTL_RAMP_DOWN_REG value very high, AMPDUs do not work.
        // Apparently, there is some issue with the smaller RAMP_DOWN value affecting CCA
#ifdef CHANNEL_BONDED_CAPABLE        
        SET_PHY_REG(pMac->hHdd, QWLAN_AGC_CCA_MODE_REG, 
                    (QWLAN_AGC_CCA_MODE_SEC_MASK & ((tANI_U32 )PHY_CCA_CD << QWLAN_AGC_CCA_MODE_SEC_OFFSET)) |
                    (QWLAN_AGC_CCA_MODE_PRI_MASK & ((tANI_U32 )PHY_CCA_CD << QWLAN_AGC_CCA_MODE_PRI_OFFSET))
                   );
#else
    SET_PHY_REG(pMac->hHdd, QWLAN_AGC_CCA_MODE_REG,
                (QWLAN_AGC_CCA_MODE_PRI_MASK & ((tANI_U32 )primaryCcaMode << QWLAN_AGC_CCA_MODE_PRI_OFFSET))
             );
#endif             
        
#ifdef ANALOG_LINK
    SET_PHY_REG(pMac->hHdd, QWLAN_AGC_TH_CD_REG, 40);
#endif

    return (retVal);

}

eHalStatus asicSetDisabledRxPacketTypes(tpAniSirGlobal pMac, ePhyRxDisabledPktTypes modTypes)
{
    eHalStatus retVal;
    assert(pMac != 0);
    assert((modTypes >= PHY_RX_DISABLE_NONE) && (modTypes <= PHY_RX_DISABLE_ALL_TYPES));

    //for Gen6, make sure SLR mode packet detection is disabled always 
    modTypes |= PHY_RX_DISABLE_SLR;
    
    SET_PHY_REG(pMac->hHdd, QWLAN_AGC_DIS_MODE_REG, ((tANI_U32)modTypes));
    retVal = asicAGCReset(pMac);

    return (retVal);
}



eHalStatus asicAGCSetDensity(tpAniSirGlobal pMac, tANI_BOOLEAN densityOn, ePhyNwDensity density20MHz, ePhyNwDensity density40MHz)
{
    eHalStatus retVal;
    tANI_U8 dens20Setting = 0;
    tANI_U8 dens40Setting = 0;
    eRfSubBand  rfBand;

    rfBand = rfGetAGBand(pMac);

    if (rfBand == INVALID_RF_SUBBAND)
    {
        return eHAL_STATUS_FAILURE;
    }

    if (densityOn == eANI_BOOLEAN_TRUE)
    {
        //not close proximity - set AGC_TH_CD to appropriate density setting
        switch (density20MHz)
        {
            case PHY_NW_DENSITY_MED:
                switch (rfBand)
                {
                    case RF_BAND_2_4_GHZ:
                    default:
                        dens20Setting = AGC_MED_DENSITY_2_4_GHZ_20MHZ_SETTING;
                        break;
                }
                break;

            case PHY_NW_DENSITY_HIGH:
                switch (rfBand)
                {
                    case RF_BAND_2_4_GHZ:
                    default:
                        dens20Setting = AGC_HIGH_DENSITY_2_4_GHZ_20MHZ_SETTING;
                        break;
                }
                break;

            case PHY_NW_DENSITY_ADAPTIVE:
                switch (rfBand)
                {
                    case RF_BAND_2_4_GHZ:
                    default:
                        dens20Setting = AGC_ADAPTIVE_DENSITY_2_4_GHZ_20MHZ_SETTING;
                        break;
                }
                break;

            case PHY_NW_DENSITY_LOW:
            default:
                switch (rfBand)
                {
                    case RF_BAND_2_4_GHZ:
                    default:
                        dens20Setting = AGC_LOW_DENSITY_2_4_GHZ_20MHZ_SETTING;
                        break;
                }
                break;
        }

        phyLog(pMac, LOG1, "Network density set, On/Off = %d, density20 = %d, density40 = %d, 20MHz = %x, 40MHz = %x\n",
                                                    densityOn, density20MHz, density40MHz, dens20Setting, dens40Setting);
        SET_PHY_REG(pMac->hHdd, QWLAN_AGC_TH_CD_REG, (dens20Setting | (dens40Setting << QWLAN_AGC_TH_CD_TH40_OFFSET)));

        pMac->hphy.nwDensity20MHz = density20MHz;
        pMac->hphy.nwDensity40MHz = density40MHz;
    }
    else
    {
        //close proximity - ignore density param and set AGC_TH_CD to high value
        SET_PHY_REG(pMac->hHdd, QWLAN_AGC_TH_CD_REG, AGC_CLOSE_PROXIMITY_SETTING);
    }

    if (pMac->hphy.phy.test.testDisableRfAccess == eANI_BOOLEAN_TRUE)
    {
        //analog link - force this to a lower value
        SET_PHY_REG(pMac->hHdd, QWLAN_AGC_TH_CD_REG, 0x3c3c);
    }

    pMac->hphy.densityEnabled = densityOn;

    return (retVal);
}

#ifdef ANI_MANF_DIAG
eHalStatus asicOverrideAGCRxChainGain(tpAniSirGlobal pMac, ePhyRxChains rxChain, tANI_U8 gain)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
    
    assert(gain < RF_AGC_GAIN_LUT_DEPTH);

    SET_PHY_REG(pMac->hHdd, QWLAN_AGC_GAINSET_WRITE_REG, 0);

    switch (rxChain)
    {
        case PHY_RX_CHAIN_0:
        case PHY_ALL_RX_CHAINS:
        default:
            SET_PHY_REG(pMac->hHdd, QWLAN_AGC_GAINSET0_REG, ((tANI_U8 )gain | QWLAN_AGC_GAINSET0_OVERRIDE_MASK));
            break;
    }
    //propogate gain to RF
    SET_PHY_REG(pMac->hHdd, QWLAN_AGC_GAINSET_WRITE_REG, QWLAN_AGC_GAINSET_WRITE_OVERRIDE_EOVERRIDE);
    asicAGCReset(pMac);
    
    return (retVal);
}

eHalStatus asicCeaseOverrideAGCRxChainGain(tpAniSirGlobal pMac, ePhyRxChains rxChain)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;

    switch (rxChain)
    {
        case PHY_RX_CHAIN_0:
        case PHY_ALL_RX_CHAINS:
        default:
            SET_PHY_REG(pMac->hHdd, QWLAN_AGC_GAINSET0_REG, 0);
            break;
    }
    asicAGCReset(pMac);

    return (retVal);
}
#endif


/** ------------------------------------------------------------------------- * 
    ------------------------------------------------------------------------- *  

  
    \file asic.c
  
    \brief contains other phy layer asic module functions
  
    $Id$ 
  
    Copyright (C) 2006 Airgo Networks, Incorporated
  
  
   ========================================================================== */

#include "ani_assert.h"
#include "sys_api.h"

#ifdef FIXME_VOLANS
eHalStatus phyAsicInit(tpAniSirGlobal pMac)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
    tANI_U8 cardType = *((tANI_U8*)pMac->hphy.pEepromCache + offsetof(sHalEeprom, fields.cardType));
    
    assert(pMac != 0);

    if ((retVal = asicTxCtlInit(pMac)) == eHAL_STATUS_SUCCESS)
    {
        if ((retVal = asicTxFirInit(pMac)) == eHAL_STATUS_SUCCESS)
        {
            if ((retVal = asicAGCInit(pMac)) == eHAL_STATUS_SUCCESS)
            {
                if ((retVal = asicTPCInit(pMac)) == eHAL_STATUS_SUCCESS)
                {
#ifdef FIXME_GEN5                
                    if ((pMac->hphy.phy.test.testDisableRfAccess == eANI_BOOLEAN_TRUE) &&
                        (pMac->hphy.phy.test.testDisablePhyRegAccess == eANI_BOOLEAN_FALSE)
                       )
                    {
                        //only for analog links
                        SET_PHY_REG(pMac->hHdd, TBAPB_ANTENNA1_CTRL_REG, 0);   //antenna 1 enabled for 5.5 & 11Mbps
                    }
                    
                    SET_PHY_REG(pMac->hHdd, RBAPB_CONTROL_BARKER_TIMING_TRACKING_REG, 
                                (RBAPB_CONTROL_BARKER_TIMING_TRACKING_FEEDBACK_EENABLE << RBAPB_CONTROL_BARKER_TIMING_TRACKING_FEEDBACK_OFFSET) |
                                (2 << RBAPB_CONTROL_BARKER_TIMING_TRACKING_THRESHOLD_OFFSET)
                               );
                    SET_PHY_REG(pMac->hHdd, RACTL_STATISTICS_SELECT_REG,  RACTL_STATISTICS_SELECT_VALUE_EAGC_ONLY);
#endif                    
                    if (cardType == CARD_TYPE_USB)
                    {
                        //enable appropriate GPIO for receive LNA
                        if (!((eHAL_STATUS_SUCCESS == (retVal = rdModWrNovaField(pMac, GPIO_MODE_0_REG,   MSK_2 << 5, 5, 0x3))) &&
                              (eHAL_STATUS_SUCCESS == (retVal = rdModWrNovaField(pMac, GPIO_OUTPUT_0_REG, MSK_2 << 5, 5, 0x3)))
                             )
                           )
                        {
                            return (retVal);
                        }
                        
                    }
                    
                }
            }
        }
    }
    
    return(retVal);


}



eHalStatus phyAsicSetFreq(tpAniSirGlobal pMac, tANI_U16 freq)
{
    eHalStatus retVal = eHAL_STATUS_FAILURE;
    tANI_U8 padcGain = 0;

    assert(pMac != 0);
    assert(freq >= 2412 && freq <= 5825);


    SET_PHY_REG(pMac->hHdd, AGC_CHANNEL_FREQ_REG, freq);
#ifdef FIXME_GEN5
    if (freq > 4000)
    {
        SET_PHY_REG(pMac->hHdd, AGC_TH_D0_DBL_LOW_REG, 160);
        SET_PHY_REG(pMac->hHdd, AGC_TH_MAXCORDBLN_REG, 60);
        if( eHAL_STATUS_SUCCESS == halReadEepromField( pMac, EEPROM_COMMON_PDADC_GAIN_5_GHZ, (uEepromFields *) &padcGain ))
        {
            retVal = rdModWrNovaField(pMac, PMU_PDADC_RXADC_CONTROLS_REG, MSK_3 << 1, 1, (padcGain & MSK_3));
        }
    }
    else if (freq >= 2412)
    {
        SET_PHY_REG(pMac->hHdd, AGC_TH_D0_DBL_LOW_REG, 120);
        if( eHAL_STATUS_SUCCESS == halReadEepromField( pMac, EEPROM_COMMON_PDADC_GAIN_2_4_GHZ, (uEepromFields *) &padcGain ))
        {
            retVal = rdModWrNovaField(pMac, PMU_PDADC_RXADC_CONTROLS_REG, MSK_3 << 1, 1, (padcGain & MSK_3));
        }
    }
    else
    {
        assert(0);  //chan outside of enumerated values
    }
#endif
    SET_PHY_REG(pMac->hHdd, RACTL_PTC_CHAN_FREQ_REG, ((0x10000 * 20) / freq));
    
    if (freq <= 2484)
    {
        SET_PHY_REG(pMac->hHdd, GPIO_RFCONFIG_REG, GPIO_RFCONFIG_BAND_SEL_24G_ESEL24G);
    }
    else
    {
        SET_PHY_REG(pMac->hHdd, GPIO_RFCONFIG_REG, GPIO_RFCONFIG_BAND_SEL_24G_ESEL5G);

    }

    return(retVal);
}


eHalStatus phyAsicRfEnBand(tpAniSirGlobal pMac, eRfSubBand band)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
    tANI_U32 reg;

#ifdef FIXME_GEN5    
    //James says all cards will have the polarity bits set to 0
    // could possibly change to be card-type specific in the future - waiting for further instructions
    SET_PHY_REG(pMac->hHdd, GPIO_RFIF_POLARITY_REG, 0);
    
    GET_PHY_REG(pMac->hHdd, GPIO_RFIF_EN_REG, &reg);

    //GPIO_RFIF_EN_REG - keep chain selections, but change band if needed
    if (((reg & GPIO_RFIF_EN_RX_EN_24G_MASK) || (reg == 0)) && (band == RF_BAND_5_GHZ))
    {
        //uninitialized or change from 2.4 to 5 GHz
        reg = reg & (GPIO_RFIF_EN_PA_ENABLE_24G_1_MASK | GPIO_RFIF_EN_PA_ENABLE_24G_0_MASK);
        reg >>= GPIO_RFIF_EN_PA_ENABLE_24G_0_OFFSET;
        
        reg <<= GPIO_RFIF_EN_PA_ENABLE_5G_0_OFFSET;
        reg |= GPIO_RFIF_EN_RX_EN_5G_MASK;
        
        SET_PHY_REG(pMac->hHdd, GPIO_RFIF_EN_REG, reg);
        
        pMac->hphy.phy.phyRerunTxLoCal = eANI_BOOLEAN_TRUE; //band change requires carrier suppression to be run next time
        
    }
    else if (((reg & GPIO_RFIF_EN_RX_EN_5G_MASK) || (reg == 0)) && (band == RF_BAND_2_4_GHZ))
    {
        //uninitialized or change from 5 to 2.4 GHz
        reg = reg & (GPIO_RFIF_EN_PA_ENABLE_5G_1_MASK | GPIO_RFIF_EN_PA_ENABLE_5G_0_MASK);
        reg >>= GPIO_RFIF_EN_PA_ENABLE_5G_0_OFFSET;
        
        reg <<= GPIO_RFIF_EN_PA_ENABLE_24G_0_OFFSET;
        reg |= GPIO_RFIF_EN_RX_EN_24G_MASK;
        
        SET_PHY_REG(pMac->hHdd, GPIO_RFIF_EN_REG, reg);

        pMac->hphy.phy.phyRerunTxLoCal = eANI_BOOLEAN_TRUE; //band change requires carrier suppression to be run next time
    }    
#endif

    return (retVal);
}


eHalStatus phyAsicRfEnChains(tpAniSirGlobal pMac, ePhyChainSelect txChains)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
    tANI_U32 reg, curChains, newChains;

#ifdef FIXME_GEN5
    GET_PHY_REG(pMac->hHdd, GPIO_RFIF_EN_REG, &reg);
#endif
    //GPIO_RFIF_EN_REG - keep band, but change chain selections if needed
#if defined(ANI_PHY_DBG) && !defined(ANI_MANF_DIAG)
    if (pMac->hphy.phy.test.testDisablePhyRegAccess == eANI_BOOLEAN_FALSE)
    {
        if(!(reg != 0))
        {
            phyLog(pMac, LOGE, "ERROR: GPIO_RFIF_EN_REG value is zero\n");
            return eHAL_STATUS_FAILURE;
        }
    }
#endif


    switch (txChains)
    {
        case PHY_CHAIN_SEL_R0_ONLY:
        case PHY_CHAIN_SEL_R1_ONLY:
        case PHY_CHAIN_SEL_R2_ONLY:
        case PHY_CHAIN_SEL_R0R1_ONLY:
        case PHY_CHAIN_SEL_R0R1R2_ONLY:
            newChains = 0;
            break;
        case PHY_CHAIN_SEL_R0_T0_ON:
        case PHY_CHAIN_SEL_R0R1_T0_ON:
        case PHY_CHAIN_SEL_T0_ONLY:
            newChains = 1;
            break;
        case PHY_CHAIN_SEL_T1_ONLY:
            newChains = 2;
            break;
        case PHY_CHAIN_SEL_R0R1_T0T1_ON:
        case PHY_CHAIN_SEL_R0R1R2_T0T1_ON:
        case PHY_CHAIN_SEL_NO_RX_TX:
        case PHY_CHAIN_SEL_T0T1_ONLY:
            newChains = 3;
            break;
        default:
            assert(0);
            return (eHAL_STATUS_FAILURE);
            break;
    }
    
#ifdef FIXME_GEN5    
    if (reg & GPIO_RFIF_EN_RX_EN_24G_MASK)
    {
        curChains = (reg & (GPIO_RFIF_EN_PA_ENABLE_24G_1_MASK | GPIO_RFIF_EN_PA_ENABLE_24G_0_MASK)) >> GPIO_RFIF_EN_PA_ENABLE_24G_0_OFFSET;
        if (curChains != newChains)
        {
            SET_PHY_REG(pMac->hHdd, GPIO_RFIF_EN_REG, ((newChains << GPIO_RFIF_EN_PA_ENABLE_24G_0_OFFSET) | GPIO_RFIF_EN_RX_EN_24G_MASK));
        }
    }
    else if (reg & GPIO_RFIF_EN_RX_EN_5G_MASK)
    {
        curChains = (reg & (GPIO_RFIF_EN_PA_ENABLE_5G_1_MASK | GPIO_RFIF_EN_PA_ENABLE_5G_0_MASK)) >> GPIO_RFIF_EN_PA_ENABLE_5G_0_OFFSET;
        if (curChains != newChains)
        {
            SET_PHY_REG(pMac->hHdd, GPIO_RFIF_EN_REG, ((newChains << GPIO_RFIF_EN_PA_ENABLE_5G_0_OFFSET) | GPIO_RFIF_EN_RX_EN_5G_MASK));
        }
    }
    else if (pMac->hphy.phy.test.testDisablePhyRegAccess == eANI_BOOLEAN_FALSE)
    {
        //the following is a valid assertion, but because Betta is violating the process and messing up the EEPROMs, we need this to pass for now.
        //TODO: assert(0); //one of the preceding conditions must be true
    }
#endif

    return (retVal);
}
#endif /* def FIXME_VOLANS */

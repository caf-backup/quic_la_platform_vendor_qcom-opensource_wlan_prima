/**
 *
   Airgo Networks, Inc proprietary.
   All Rights Reserved, Copyright 2005, 2007
   This program is the confidential and proprietary product of Airgo Networks Inc.
   Any Unauthorized use, reproduction or transfer of this program is strictly prohibited.


   halPhy.c: Abstracts physical layer components
             This module provides top level data objects that are used across multiple hardware blocks

   Author:  Mark Nelson
   Date:    3/4/05

   History -
   Date        Modified by              Modification Information
  --------------------------------------------------------------------------

 */

#include "sys_api.h"
#include "halFw.h"
#include "halFwApi.h"
#ifdef ANI_MANF_DIAG
#include "pttModuleApi.h"
#endif
#include "halPhyVos.h"


//if the testChannelId is set to NORMAL_CHANNEL_SETTING, then we will allow all channel changes to take effect, not just those from a test
#define NORMAL_CHANNEL_SETTING     0xFF
#ifdef ANI_MANF_DIAG
#define SET_CHAIN_SELECT_WAIT       250000000
#define CAL_UPDATE_WAIT             500000000
#endif

//config called before init but after halNvOpen
eHalStatus halPhyOpen(tHalHandle hHal)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal) hHal;
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
    //right now hardcoding it to FCC. read this from NVI and invoke halPhySetRegDomain to set it.
    eRegDomainId curDomain = REG_DOMAIN_FCC;

    //hard coding the nTx and nRx. Need to fetch them from hal sys config structure
    tANI_U8 nTx = PHY_MAX_TX_CHAINS;
    tANI_U8 nRx = PHY_MAX_RX_CHAINS;

#ifdef ANI_MANF_DIAG
    //allocate ADC capture cache
    if(palAllocateMemory(pMac->hHdd, (void **)&pMac->ptt.pADCCaptureCache, PHY_MAX_RX_CHAINS * GRAB_RAM_DBLOCK_SIZE * sizeof(tANI_U32)) != eHAL_STATUS_SUCCESS)
    {
        phyLog(pMac, LOGE, "unable to allocate memory for ADC capture. \n");
        return eHAL_STATUS_FAILURE;
    }
#endif

    pMac->hphy.phy.cfgChains = halPhyGetChainSelect(pMac, nTx, nRx);
    pMac->hphy.phy.openLoopTxGain = OPEN_LOOP_TX_HIGH_GAIN_OVERRIDE;

#ifdef VOLANS_RF
    pMac->hphy.phy.test.testDisableRfAccess = eANI_BOOLEAN_FALSE;
    pMac->hphy.phy.test.testDisablePhyRegAccess = eANI_BOOLEAN_FALSE;
#else //no phy or RF support in netlist
    pMac->hphy.phy.test.testDisableRfAccess = eANI_BOOLEAN_TRUE;
    pMac->hphy.phy.test.testDisablePhyRegAccess = eANI_BOOLEAN_TRUE;
#endif

    //if ((retVal = ConfigureTpcFromNv(pMac)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    if ((retVal = halPhySetRegDomain(pMac, curDomain)) != eHAL_STATUS_SUCCESS) { return (retVal); }

    //intialize the setChan event for wait blocking around halPhySetChannel
    return (halPhy_VosEventInit(hHal));
}

eHalStatus halPhyClose(tHalHandle hHal)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal) hHal;
    eHalStatus retVal = eHAL_STATUS_SUCCESS;

    if ((retVal = phyTxPowerClose(pMac)) != eHAL_STATUS_SUCCESS) { return (retVal); }

#ifdef ANI_MANF_DIAG
    if(pMac->ptt.pADCCaptureCache)
    {
        palFreeMemory(pMac->hHdd, pMac->ptt.pADCCaptureCache);
    }
#endif

    //destroy the setChan event for wait blocking around halPhySetChannel
    return (halPhy_VosEventDestroy(hHal));
}


eHalStatus halPhyStart(tHalHandle hHal)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
    tpAniSirGlobal pMac = (tpAniSirGlobal) hHal;

    pMac->hphy.phy.chanBondState = PHY_SINGLE_CHANNEL_CENTERED;
    pMac->hphy.phy.activeChains = PHY_CHAIN_SEL_R0R1_T0_ON;
    pMac->hphy.phy.openLoopTxGain = OPEN_LOOP_TX_HIGH_GAIN_OVERRIDE;

    pMac->hphy.rf.curChannel = RF_CHAN_1;

    //do we need this?
    pMac->hphy.pwr = PHY_POWER_NORMAL;

#ifdef VOLANS_RF
    pMac->hphy.phy.test.testDisableRfAccess = eANI_BOOLEAN_FALSE;
    pMac->hphy.phy.test.testDisablePhyRegAccess = eANI_BOOLEAN_FALSE;
#else    //no phy or RF support in netlist
    pMac->hphy.phy.test.testDisableRfAccess = eANI_BOOLEAN_TRUE;
    pMac->hphy.phy.test.testDisablePhyRegAccess = eANI_BOOLEAN_TRUE;
#endif

#if defined(ANI_MANF_DIAG) && defined(VOLANS_RF)
    {
        pttModuleInit(pMac);
    }
#endif

    return (retVal);
}


eHalStatus halPhySetRxPktsDisabled(tHalHandle hHal, ePhyRxDisabledPktTypes rxDisabled)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal) hHal;
    eHalStatus retVal = eHAL_STATUS_SUCCESS;

    assert(pMac != 0);
    assert((rxDisabled >= PHY_RX_DISABLE_NONE) && (rxDisabled <= PHY_RX_DISABLE_ALL_TYPES));

    //save desired disabled setting - if we change channel bonded status, then we may need to re-enable 40MHz frames
    pMac->hphy.modTypes = rxDisabled;

#ifdef CHANNEL_BONDED_CAPABLE
    if (PHY_SINGLE_CHANNEL_CENTERED == pMac->hphy.phy.chanBondState)
    {
        //we need to prevent 40MHz packet types being received when only 20MHz is available
        rxDisabled |= (PHY_RX_DISABLE_11N40);
    }
#endif

    {
        //automatically enable 11b packets - agreed that the mac would never expect llb packets to be off
#ifndef ANI_MANF_DIAG //don't do this for manufacturing driver since it interfere's with external control
        rxDisabled &= ~PHY_RX_DISABLE_11B;
#endif
    }

    retVal = asicSetDisabledRxPacketTypes(pMac, rxDisabled);

    return (retVal);

}


eHalStatus halPhyDisableAllPackets(tHalHandle hHal)
{
    return asicSetDisabledRxPacketTypes((tpAniSirGlobal)hHal, PHY_RX_DISABLE_ALL_TYPES);
}


eHalStatus halPhySetAgcCCAMode(tHalHandle hHal, ePhyCCAMode primaryCcaMode, ePhyCCAMode secondaryCcaMode)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
    tpAniSirGlobal pMac = (tpAniSirGlobal) hHal;

    assert(pMac != 0);
    assert((primaryCcaMode >= PHY_CCA_FORCED_ON) && (primaryCcaMode <= PHY_CCA_ED_OR_CD_AND_CS));
    assert((secondaryCcaMode >= PHY_CCA_FORCED_ON) && (secondaryCcaMode <= PHY_CCA_SEC_ED40_AND_NOR_PKTDET40_PKTDET20));

    if (pMac->hphy.phy.test.testDisableRfAccess == eANI_BOOLEAN_FALSE)
    {
        retVal = asicSetAgcCCAMode(pMac, primaryCcaMode, secondaryCcaMode);
    }
    else
    {
        //leave it to defaults for AEVB link
        retVal = asicSetAgcCCAMode(pMac, PHY_CCA_ED_OR_CD_AND_CS, PHY_CCA_ED_AND_CD);
    }
    return (retVal);
}


ePhyChainSelect halPhyGetChainSelect(tHalHandle hHal, tANI_U8 numTxChains, tANI_U8 numRxChains)
{
    //assert(hHal != NULL);

    if (numTxChains == 1)
    {
        switch (numRxChains)
        {
            case 0:
                return (PHY_CHAIN_SEL_T0_ON);     //test mode
            case 1:
                return (PHY_CHAIN_SEL_R0_T0_ON);
            case 2:
                return (PHY_CHAIN_SEL_R0R1_T0_ON);
            default:
                return (INVALID_PHY_CHAIN_SEL);
        }
    }
    else if (numTxChains == 0)                      //all test modes here
    {
        switch (numRxChains)
        {
            case 0:
                return (PHY_CHAIN_SEL_NO_RX_TX);
            case 1:
                return (PHY_CHAIN_SEL_R0_ON);
            case 2:
                return (PHY_CHAIN_SEL_R0R1_ON);
            default:
                return (INVALID_PHY_CHAIN_SEL);
        }
    }
    return (INVALID_PHY_CHAIN_SEL);
}

tANI_U8 halPhyQueryNumRxChains(ePhyChainSelect phyRxTxAntennaMode)
{
    switch (phyRxTxAntennaMode)
    {
        case PHY_CHAIN_SEL_T0_ON:
        case PHY_CHAIN_SEL_NO_RX_TX:
            return (0);
        case PHY_CHAIN_SEL_R0_T0_ON:
        case PHY_CHAIN_SEL_R0_ON:
        case PHY_CHAIN_SEL_T0_R1_ON:
        case PHY_CHAIN_SEL_R1_ON:
            return (1);
        case PHY_CHAIN_SEL_R0R1_T0_ON:
        case PHY_CHAIN_SEL_R0R1_ON:
            return (2);
        default:
            //assert(0);  //no other valid modes
            return(0);
    }
}

tANI_U8 halPhyQueryNumTxChains(ePhyChainSelect phyRxTxAntennaMode)
{
    switch (phyRxTxAntennaMode)
    {
        case PHY_CHAIN_SEL_R0_ON:
        case PHY_CHAIN_SEL_R0R1_ON:
        case PHY_CHAIN_SEL_R1_ON:
        case PHY_CHAIN_SEL_NO_RX_TX:
            return (0);
        case PHY_CHAIN_SEL_R0_T0_ON:
        case PHY_CHAIN_SEL_R0R1_T0_ON:
        case PHY_CHAIN_SEL_T0_ON:
        case PHY_CHAIN_SEL_T0_R1_ON:
            return (1);
        default:
            //assert(0);  //no other valid modes
            return(0);
    }
}


ePhyChainSelect halPhyGetActiveChainSelect(tHalHandle hHal)
{
    //pMac->hphy.phy.activeChains should match the numTx and numRx chain selection from the sysconfig
    //has to be updated while handling fw_init_done mailbox msg from firmware
    //Also, it has to be updated in the mailbox response handler of halPhySetChainSelect
    tpAniSirGlobal pMac = (tpAniSirGlobal) hHal;
    return (pMac->hphy.phy.activeChains);
}


eHalStatus halPhySetChainSelect(tHalHandle hHal, ePhyChainSelect phyChainSelections)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
    tpAniSirGlobal pMac = (tpAniSirGlobal) hHal;
    tANI_U16 dialogToken = 0;

    //send a mailbox message to the firmware
    //in the response handler update pMac->hphy.phy.activeChains
    Qwlanfw_SetChainSelectReqType chainSelect;
    chainSelect.uPhyChainSelections = (tANI_U32)phyChainSelections;

    pMac->hphy.setPhyMsgEvent = eANI_BOOLEAN_TRUE;

    //send a mailbox messag to the firmware to set the channel
    retVal = halFW_SendMsg(pMac,
                           HAL_MODULE_ID_PHY,
                           QWLANFW_HOST2FW_SET_CHAIN_SELECT_REQ,
                           dialogToken,
                           sizeof(Qwlanfw_SetChainSelectReqType),
                           &chainSelect,
                           TRUE,
                           NULL
                          );
#ifdef ANI_MANF_DIAG
        halPhyWaitForInterrupt(hHal, QWLANFW_FW2HOST_SET_CHAIN_SELECT_RSP);
        pMac->hphy.phy.activeChains = phyChainSelections;
#endif
   
   pMac->hphy.phy.cfgChains = phyChainSelections;

   return (retVal);
}


//simply performs a periodic calibration,
//should be called after changing a channel as well
eHalStatus halPhyCalUpdate(tHalHandle hHal)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
    tpAniSirGlobal pMac = (tpAniSirGlobal) hHal;
    tANI_U16 dialogToken = 0;

    //send a mailbox message to the firmware
    Qwlanfw_CalUpdateReqType calUpdate;
    calUpdate.usPeriodic = 1;

#ifdef ANI_MANF_DIAG
    calUpdate.usCalId = pMac->hphy.phy.test.testCalMode;
#else
    calUpdate.usCalId = ALL_CALS;
#endif

    pMac->hphy.setPhyMsgEvent = eANI_BOOLEAN_TRUE;

    //send a mailbox message to the firmware to perform the calibration
    retVal = halFW_SendMsg(pMac,
                           HAL_MODULE_ID_PHY,
                           QWLANFW_HOST2FW_CAL_UPDATE_REQ,
                           dialogToken,
                           sizeof(Qwlanfw_CalUpdateReqType),
                           &calUpdate,
                           TRUE,
                           NULL
                          );
     
#ifdef ANI_MANF_DIAG
       halPhyWaitForInterrupt(hHal, QWLANFW_FW2HOST_CAL_UPDATE_RSP); 
       //vos_sleep(300); 
       //sirBusyWait(CAL_UPDATE_WAIT);
#endif
    
    return (retVal);
}

eHalStatus halPhySetChannel(tHalHandle hHal, tANI_U8 channelNumber, ePhyChanBondState cbState, tANI_U8 calRequired)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
    tpAniSirGlobal pMac = (tpAniSirGlobal) hHal;
    tANI_U16 dialogToken = 0;
    Qwlanfw_SetChannelReqType setChan;

   assert(cbState == PHY_SINGLE_CHANNEL_CENTERED);
   assert(channelNumber <= NUM_2_4GHZ_CHANNELS);

    setChan.usChanNum = channelNumber;
    setChan.ucCbState = (tANI_U8)cbState;
    //TODO: setChan.ucRegDomain = (tANI_U8)halPhyGetRegDomain(pMac);
    setChan.ucRegDomain = 0;
    setChan.ucCalRequired = calRequired;

#ifdef ANI_PHY_DEBUG
    //test interface is in control of channel setting
    if (pMac->hphy.phy.test.testChannelId != NORMAL_CHANNEL_SETTING)
    {
        //test command controls channel setting
        // testChannelId is set externally to match what is being passed in channelNumber
        // if these don't match, then we assume the channel change is not under our test control and thus we ignore it
        if ((pMac->hphy.phy.test.testChannelId != channelNumber) || (pMac->hphy.phy.test.testCbState != cbState))
        {
            return (eHAL_STATUS_SUCCESS); //return success to keep things running otherwise normally
        }
    }
    //else
    // when testChannelId == NORMAL_CHANNEL_SETTING
    // this means we are not under test control and we allow the parameters to take effect
#endif
    if(pMac->hal.halMac.isFwInitialized != eANI_BOOLEAN_TRUE)
        return eHAL_STATUS_FAILURE;

    pMac->hphy.setPhyMsgEvent = eANI_BOOLEAN_TRUE;

    //send a mailbox message to the firmware to set the channel
    retVal = halFW_SendMsg(pMac, HAL_MODULE_ID_PHY, QWLANFW_HOST2FW_SET_CHANNEL_REQ, dialogToken,
                                 sizeof(Qwlanfw_SetChannelReqType), &setChan, FALSE, NULL);
    if(retVal != eHAL_STATUS_SUCCESS) { return (retVal); }

#ifdef ANI_MANF_DIAG
       halPhyWaitForInterrupt(hHal, QWLANFW_FW2HOST_SET_CHANNEL_RSP); 
       //sirBusyWait(SET_CHANNEL_WAIT);
#endif

    {
        //Set the tx power. handle this in halSetChannel response handler
        eRfChannels chan = rfGetChannelIndex(channelNumber, PHY_SINGLE_CHANNEL_CENTERED/*cbState*/);
        tANI_U16 freq = rfChIdToFreqCoversion(rfGetChannelIdFromIndex(chan));
        eRfSubBand bandIndex = RF_SUBBAND_2_4_GHZ;

        pMac->hphy.phy.chanBondState = PHY_SINGLE_CHANNEL_CENTERED/*cbState*/;

        switch (bandIndex)
        {
            case RF_SUBBAND_2_4_GHZ:
                bandIndex = RF_BAND_2_4_GHZ;
                break;

            default:
                phyLog(pMac, LOGE, "ERROR: band not found\n");
                return (retVal);
        }

        //use calRequired var to determine whether it is operating channel or not.
        //load the power det values only if it is an operating channel for production driver.
        {
            if ((retVal = phySetTxPower(pMac, freq, bandIndex)) != eHAL_STATUS_SUCCESS)
            {
                return (retVal);
            }
        }

        pMac->hphy.rf.curChannel = chan;
        return eHAL_STATUS_SUCCESS;
    }
}


eHalStatus halPhyGetChannelListWithPower(tHalHandle hHal,
                                         tChannelListWithPower *channels20MHz /*[NUM_LEGIT_RF_CHANNELS] */,
                                         tANI_U8 *num20MHzChannelsFound,
                                         tChannelListWithPower *channels40MHz /*[NUM_CHAN_BOND_CHANNELS] */,
                                         tANI_U8 *num40MHzChannelsFound
                                        )
{
    tpAniSirGlobal pMac = (tpAniSirGlobal) hHal;
    sRegulatoryDomains  *pRegDomain;
    eHalStatus status = eHAL_STATUS_INVALID_PARAMETER;
    int i, count;

    //Ignore 40MHz channels for now
    if( channels20MHz && num20MHzChannelsFound && ( pMac->hphy.phy.curRegDomain < REGDOMAIN_COUNT ) )
    {
        status = halGetNvTableLoc(pMac, NV_TABLE_REGULATORY_DOMAINS, (uNvTables **)&pRegDomain);
        if( HAL_STATUS_SUCCESS(status) )
        {
            //set the pointer to the right index
            pRegDomain = &pRegDomain[pMac->hphy.phy.curRegDomain];
            count = 0;
            for( i = 0; i < NUM_RF_CHANNELS; i++ )
            {
                if( pRegDomain->channels[i].enabled )
                {
                    channels20MHz[count].chanId = rfGetChannelIdFromIndex( i );
                    channels20MHz[count++].pwr = pRegDomain->channels[i].pwrLimit;
                }
            }
            *num20MHzChannelsFound = (tANI_U8)count;
        }
    }

    return (status);
}


eHalStatus halPhySetNwDensity(tHalHandle hHal, tANI_BOOLEAN densityOn, ePhyNwDensity density20MHz, ePhyNwDensity density40MHz)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal) hHal;
    return (asicAGCSetDensity(pMac, densityOn, density20MHz, density40MHz));
}




//this function will display phy settings and attempt to diagnose any problems in the physical layer

eHalStatus halPhyDiagnose(tHalHandle hHal, char *callerStr)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;

    return (retVal);
}

eHalStatus halPhyGetRxGainRange(tHalHandle hHal, tANI_U8 *maxGainIndex, tANI_U8 *topGaindB, tANI_U8 *bottomGain)
{
    return (eHAL_STATUS_SUCCESS);
}

eHalStatus halPhyChIdToFreqConversion(tANI_U8 num, tANI_U16 *pfreq)
{
    tANI_U16 freq;

    if (!pfreq) return eHAL_STATUS_FAILURE;

    freq = rfChIdToFreqCoversion(num);
    if (!freq) return eHAL_STATUS_FAILURE;

    *pfreq = freq;

    return eHAL_STATUS_SUCCESS;

} // End halPhyChIdToFreqCoversion.


eHalStatus halPhyStop(tHalHandle hHal)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal) hHal;
    pMac->hphy.rf.curChannel = INVALID_RF_CHANNEL;

    return (eHAL_STATUS_SUCCESS);
}

eHalStatus halPhySetPowerSave(tHalHandle hHal, ePhyPowerSave pwr)
{
    return (eHAL_STATUS_SUCCESS);
}


/*
* Description:
* This API is provided to override
* the default behavior of the PHY
* to "not" drop packets received
* with the "sounding bit" set
*
* This lets us be bug compatible
* with MRVL chipsets that exhibit
* this behavior
*/
eHalStatus halPhyRxSoundingBitFrames( tHalHandle hHal, tANI_BOOLEAN enable )
{
    tpAniSirGlobal pMac = (tpAniSirGlobal) hHal;
    eHalStatus retVal = eHAL_STATUS_SUCCESS;

    retVal = rdModWrAsicField( pMac,
            QWLAN_RACTL_RACTL_TEST_REG,
            QWLAN_RACTL_RACTL_TEST_SOUNDING_BIT_CHK_EN_MASK,
            QWLAN_RACTL_RACTL_TEST_SOUNDING_BIT_CHK_EN_OFFSET,
            (eANI_BOOLEAN_TRUE == enable)? 0: 1 );

    return retVal;
}

eHalStatus halPhyConfigureTpc(tHalHandle hHal)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal) hHal;
    eHalStatus retVal = eHAL_STATUS_SUCCESS;

    retVal = ConfigureTpcFromNv(pMac);

    return retVal;

}


eHalStatus halPhyLoadTxPowerDetValues(tHalHandle hHal)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal) hHal;
    eHalStatus retVal = eHAL_STATUS_SUCCESS;

    //Set the tx power. handle this in halSetChannel response handler
    eRfChannels chan = rfGetChannelIndex(pMac->hal.currentChannel, PHY_SINGLE_CHANNEL_CENTERED/*pMac->hal.currentCBState*/);
    tANI_U16 freq = rfChIdToFreqCoversion(rfGetChannelIdFromIndex(chan));

    //use calRequired var to determine whether it is operating channel or not.
    //load the power det values only if it is an operating channel for production driver.

    retVal = phySetTxPower(pMac, freq, pMac->hal.currentRfBand);
    phyLog(pMac, LOGE, FL("phySetTxPower, freq : %d, band : %d \n"), freq, pMac->hal.currentRfBand);

    return retVal;
}

eHalStatus halPhyUpdateTxGainOverride(tHalHandle hHal, tANI_U8 txGain)
{
#ifdef VOLANS_FPGA
    return eHAL_STATUS_SUCCESS;
#else
    tpAniSirGlobal pMac = (tpAniSirGlobal) hHal;
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
    Qwlanfw_SysCfgType *pFwConfig = (Qwlanfw_SysCfgType *)pMac->hal.FwParam.pFwConfig;

    pMac->hphy.phy.openLoopTxGain = txGain;

    if ((retVal = asicTPCAutomatic(pMac)) != eHAL_STATUS_SUCCESS) { return (retVal); }

    pFwConfig->ucOpenLoopTxGain = pMac->hphy.phy.openLoopTxGain;
    retVal = halFW_UpdateSystemConfig(pMac, pMac->hal.FwParam.fwSysConfigAddr, (tANI_U8 *)pFwConfig, sizeof(Qwlanfw_SysCfgType));

    return retVal;

#endif //#ifdef VOLANS_FPGA
}

#ifndef VERIFY_HALPHY_SIMV_MODEL
inline void hv_printLog(const char *str, ...)
{
    //Only used when testing on SIMV Model
}
#endif

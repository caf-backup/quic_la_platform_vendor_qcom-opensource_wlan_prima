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
#include "pttModuleApi.h"
#include "halPhyVos.h"
#include "halTimer.h"


//if the testChannelId is set to NORMAL_CHANNEL_SETTING, then we will allow all channel changes to take effect, not just those from a test
#define NORMAL_CHANNEL_SETTING     0xFF

#define SET_CHAIN_SELECT_WAIT		250000000
#define CAL_UPDATE_WAIT				500000000
#define MAX_SLEEP                   10000

static void halPhyWaitForInterrupt(tHalHandle hHal, tANI_U32 msgType)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal) hHal;
    tANI_U8 sleepCnt = 0;

    //pMac->hphy.setPhyMsgEvent = eANI_BOOLEAN_TRUE;

    switch(msgType)
    {
        case QWLANFW_FW2HOST_CAL_UPDATE_RSP:
        case QWLANFW_FW2HOST_SET_CHANNEL_RSP:
        case QWLANFW_FW2HOST_SET_CHAIN_SELECT_RSP:

        // HAL_PHY_VOS_SCHEDULE_WAIT_EVENT

        {   //we should wait for setPhyMsgEvent to be set to false in the
            // halPhy_HandlerFwRspMsg() function when the response interrupt is processed.
            do
            {
                //sleep 100 microsec
                sirSleepWait(100);
                sleepCnt++;
            }while ((pMac->hphy.setPhyMsgEvent == eANI_BOOLEAN_TRUE) && (sleepCnt < MAX_SLEEP));

            break;
        }
        default:
            return;
    }

    if(sleepCnt == MAX_SLEEP)
    {
        phyLog(pMac, LOGE, "Did not receive Mail box interrupt from fw! \n");
    }

    return;
}
///////////////////////////////////////////
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

    pMac->hphy.phy.cfgChains = halPhyGetChainSelect(pMac, nTx, nRx);
    pMac->hphy.phy.openLoopTxGain = OPEN_LOOP_TX_HIGH_GAIN_OVERRIDE;

    //get/try_to_find this information from the config file
    pMac->hphy.phy.test.testDisableSpiAccess = eANI_BOOLEAN_FALSE;
    pMac->hphy.phy.test.testDisablePhyRegAccess = eANI_BOOLEAN_FALSE;

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

    //destroy the setChan event for wait blocking around halPhySetChannel
    return (halPhy_VosEventDestroy(hHal));
}

eHalStatus halPhyFwInitDone(tHalHandle hHal)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal) hHal;
    eHalStatus retVal = eHAL_STATUS_SUCCESS;

    if(pMac->gDriverType == eDRIVER_TYPE_MFG)
    {
        //for FTM driver leave init_gain at 74 as suggested by James
        SET_PHY_REG(pMac->hHdd, QWLAN_AGC_INIT_GAIN_REG, 74);
    }

    return retVal;

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

    //get/try_to_find this information from the config file
    pMac->hphy.phy.test.testDisableSpiAccess = eANI_BOOLEAN_FALSE;
    pMac->hphy.phy.test.testDisablePhyRegAccess = eANI_BOOLEAN_FALSE;

    if(pMac->gDriverType == eDRIVER_TYPE_MFG)
    {
        GET_PHY_REG(pMac->hHdd, QWLAN_RFAPB_REV_ID_REG, &pMac->hphy.rf.revId);
    }

#ifndef WLAN_FTM_STUB
    if(pMac->gDriverType == eDRIVER_TYPE_MFG)
    {
        //allocate ADC capture cache
        if(palAllocateMemory(pMac->hHdd, (void **)&pMac->ptt.pADCCaptureCache, PHY_MAX_RX_CHAINS * GRAB_RAM_DBLOCK_SIZE * sizeof(tANI_U32)) != eHAL_STATUS_SUCCESS)
        {
            phyLog(pMac, LOGE, "unable to allocate memory for ADC capture. \n");
            return eHAL_STATUS_FAILURE;
        }
    }
#endif

    if ((retVal = halQFuseRead(pMac)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    if ((retVal = ConfigureTpcFromNv(pMac)) != eHAL_STATUS_SUCCESS) { return (retVal); }

    asicSetupTestWaveform(pMac, pWave, sizeof(pWave)/sizeof(pWave[0]), eANI_BOOLEAN_OFF);


    if ((retVal = phyTxPowerInit(pMac)) == eHAL_STATUS_SUCCESS)

    {
        //this is needed so that the network density is enabled by default, which sets the densityEnabled flag
        //Only if densityEnabled == TRUE, will CLPC return the proper power settings
        if ((retVal = halPhySetNwDensity(pMac, eANI_BOOLEAN_TRUE, PHY_NW_DENSITY_LOW, PHY_NW_DENSITY_LOW)) != eHAL_STATUS_SUCCESS)
        {
            return (retVal);
        }

        //To start off, load the power det table for channel 1
        if ((retVal = phySetTxPower(pMac, START_TPC_CHANNEL, RF_BAND_2_4_GHZ)) != eHAL_STATUS_SUCCESS)
        {
            return (retVal);
        }
#ifndef WLAN_FTM_STUB
        if(pMac->gDriverType == eDRIVER_TYPE_MFG)
        {
            pttModuleInit(pMac);

            //start the ADC RSSI stats collection timer
            halTimersCreate(pMac);
        }
#endif
    }
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
        if(pMac->gDriverType == eDRIVER_TYPE_PRODUCTION) //don't do this for manufacturing driver since it interfere's with external control
        {
            rxDisabled &= ~PHY_RX_DISABLE_11B;
        }
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

    if (pMac->hphy.phy.test.testDisableSpiAccess == eANI_BOOLEAN_FALSE)
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

    if(pMac->gDriverType == eDRIVER_TYPE_MFG)
    {
        halPhyWaitForInterrupt(hHal, QWLANFW_FW2HOST_SET_CHAIN_SELECT_RSP);
        //sirBusyWait(SET_CHAIN_SELECT_WAIT);
        pMac->hphy.phy.activeChains = phyChainSelections;
    }

    pMac->hphy.phy.cfgChains = phyChainSelections;


    return (retVal);
}


/*
static eHalStatus halPhyLockChannel(tpAniSirGlobal pMac)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
    return (retVal);
}
*/
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

    if(pMac->gDriverType == eDRIVER_TYPE_MFG)
    {
        calUpdate.usCalId = pMac->hphy.phy.test.testCalMode;
    }
    else
    {
        calUpdate.usCalId = ALL_CALS;
    }

    if(pMac->gDriverType == eDRIVER_TYPE_MFG)
    {
        //calibrations in firmware require this specific waveform to be preloaded by the host
        // this saves us code space in firmware, which only stops and starts the waveform, and doesn't declare it.
        asicSetupTestWaveform(pMac, pWave, 184, eANI_BOOLEAN_OFF);

    }

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

    if(pMac->gDriverType == eDRIVER_TYPE_MFG)
    {
        halPhyWaitForInterrupt(hHal, QWLANFW_FW2HOST_CAL_UPDATE_RSP);
        //sirBusyWait(CAL_UPDATE_WAIT);
    }
    return (retVal);
}

eHalStatus halPhySetChannel(tHalHandle hHal, tANI_U8 channelNumber,
        ePhyChanBondState cbState, tANI_U8 calRequired)
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

    if(pMac->gDriverType == eDRIVER_TYPE_MFG)
    {
        //Reset the event after receiving the setchan rsp from fw
        if ((retVal = halPhy_VosEventResetSetChannel(hHal)) != eHAL_STATUS_SUCCESS) { return (retVal); }

        // fwSetChannelStatus is toggled ( to success/failure) in halPhySetChannel fw rsp msg handler
        // hard code it to always success
        pMac->hphy.fwSetChannelStatus = eHAL_STATUS_SUCCESS;

    }

    //send a mailbox messag to the firmware to set the channel
    retVal = halFW_SendMsg(pMac, HAL_MODULE_ID_PHY, QWLANFW_HOST2FW_SET_CHANNEL_REQ, dialogToken,
                                 sizeof(Qwlanfw_SetChannelReqType), &setChan, FALSE, NULL);
    if(retVal != eHAL_STATUS_SUCCESS) { return (retVal); }

    if(pMac->gDriverType == eDRIVER_TYPE_MFG)
    {
        //Wait till the host receives setChannel rsp from fw
        //if ((retVal = halPhy_VosEventWaitSetChannel(hHal)) != eHAL_STATUS_SUCCESS) { return (retVal); }

        //If fwSetChannelStatus successful, then proceed
        /*200 millsec*/
       //sirBusyWait(200*1000*1000);
       pMac->hphy.fwSetChannelStatus = eHAL_STATUS_SUCCESS;
        if (pMac->hphy.fwSetChannelStatus == eHAL_STATUS_SUCCESS) {
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

            if ((retVal = phySetTxPower(pMac, freq, bandIndex)) != eHAL_STATUS_SUCCESS)
            {
                    return (retVal);
                }
    #ifdef ANI_PHY_DEBUG
            // if ((retVal = phyCalFromBringupTables(pMac, freq)) != eHAL_STATUS_SUCCESS)
            // {
            //     return (retVal);
            // }
    #endif
            pMac->hphy.rf.curChannel = chan;

        } else {
            return eHAL_STATUS_FAILURE;
        }
    }

    return eHAL_STATUS_SUCCESS;
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
    eHalStatus retVal = eHAL_STATUS_SUCCESS;

    tpAniSirGlobal pMac = (tpAniSirGlobal) hHal;
    pMac->hphy.rf.curChannel = INVALID_RF_CHANNEL;

#ifndef WLAN_FTM_STUB
    if(pMac->gDriverType == eDRIVER_TYPE_MFG)
    {
        if(pMac->ptt.pADCCaptureCache)
        {
            palFreeMemory(pMac->hHdd, pMac->ptt.pADCCaptureCache);
        }
    }
#endif

    return (retVal);
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

#define CAL_GAIN_INDEX          78
#define CAL_BCK_UP_REG_SIZE     1000
//#define QWLAN_RFAPB_RX_DCOC_IQ_REG     QWLAN_RFAPB_BASE + 0x288

/* DESCRIPTION:
 *      Routine to write Phy/RF cal registers into ADU memory.
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *      pAddr:  Starting address in the ADU memory from where the
 *              register/value tuple will be written
 *
 * RETURN:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */
eHalStatus halPhyBckupCalRegisters(tHalHandle hHal, tANI_U32 *pMemAddr)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal) hHal;
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tANI_U32 dcoIndex, modeSel, rxChain, gainStep, count = 0;
    tANI_U32 *pCalBuf = NULL;
    tANI_U32 chainCorr[PHY_MAX_RX_CHAINS];
    tANI_U32 *aduMem = (tANI_U32 *)(*pMemAddr);
    Qwlanfw_PhyCalCorrType phyCalCorr;

    /** Read the Dco correction valuess from device memory.*/
    palReadDeviceMemory(pMac->hHdd, QWLANFW_MEM_PHY_CAL_CORRECTIONS_ADDR_OFFSET,
                (tANI_U8 *)&phyCalCorr, sizeof(Qwlanfw_PhyCalCorrType));

    chainCorr[PHY_RX_CHAIN_0] = (phyCalCorr.ucIDcoCorrChain0 << 8/*QWLAN_RFAPB_RX_DCOC_IQ_RX_DCOC_I_OFFSET*/) | phyCalCorr.ucQDcoCorrChain0;
    chainCorr[PHY_RX_CHAIN_1] = (phyCalCorr.ucIDcoCorrChain1 << 8/*QWLAN_RFAPB_RX_DCOC_IQ_RX_DCOC_I_OFFSET*/) | phyCalCorr.ucQDcoCorrChain1;

    status = palAllocateMemory(pMac->hHdd, (void **)&pCalBuf,
                               (sizeof(tANI_U32) * CAL_BCK_UP_REG_SIZE));
    if (status != eHAL_STATUS_SUCCESS){
        return status;
    }

    //decide to open/close the loop
    {
        pCalBuf[count++] = (QWLAN_TPC_TXPWR_ENABLE_REG | HAL_REG_HOST_FILLED_MASK);
        pCalBuf[count++] = 0x0;
        pCalBuf[count++] = (QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG | HAL_REG_HOST_FILLED_MASK);
        pCalBuf[count++] = 0x20;
        pCalBuf[count++] = (QWLAN_TPC_TXPWR_OVERRIDE0_REG | HAL_REG_FW_FILLED);
        pCalBuf[count++] = 0xaf;
        pCalBuf[count++] = (QWLAN_TPC_TXPWR_ENABLE_REG | HAL_REG_FW_FILLED);
        pCalBuf[count++] = 0x2;
        pCalBuf[count++] = (QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG | HAL_REG_HOST_FILLED_MASK);
        pCalBuf[count++] = 0x0;
        pCalBuf[count++] = (QWLAN_RFAPB_TX_GAIN_CONTROL_REG | HAL_REG_FW_FILLED | HAL_REG_FW_EXTRA_FILLED);
        pCalBuf[count++] = 0xa;
    }

    //update bwCal registers here
    {
        pCalBuf[count++] = ((QWLAN_RFAPB_MODE_SEL1_REG)|( HAL_REG_HOST_FILLED_MASK));
        pCalBuf[count++] = 0x0; //for rx chain0
        pCalBuf[count++] = ((QWLAN_RFAPB_BB_BW1_REG)|( HAL_REG_HOST_FILLED_MASK));
        pCalBuf[count++] = phyCalCorr.ucBw1Chain0;
        pCalBuf[count++] = ((QWLAN_RFAPB_BB_BW2_REG)|( HAL_REG_HOST_FILLED_MASK));
        pCalBuf[count++] = phyCalCorr.ucBw2Chain0;
        pCalBuf[count++] = ((QWLAN_RFAPB_BB_BW3_REG)|( HAL_REG_HOST_FILLED_MASK));
        pCalBuf[count++] = phyCalCorr.ucBw3Chain0;
        pCalBuf[count++] = ((QWLAN_RFAPB_BB_BW4_REG)|( HAL_REG_HOST_FILLED_MASK));
        pCalBuf[count++] = phyCalCorr.ucBw4Chain0;
        pCalBuf[count++] = ((QWLAN_RFAPB_BB_BW5_REG)|( HAL_REG_HOST_FILLED_MASK));
        pCalBuf[count++] = phyCalCorr.ucBw5Chain0;
        pCalBuf[count++] = ((QWLAN_RFAPB_BB_BW6_REG)|( HAL_REG_HOST_FILLED_MASK));
        pCalBuf[count++] = phyCalCorr.ucBw6Chain0;

        pCalBuf[count++] = ((QWLAN_RFAPB_MODE_SEL1_REG)|( HAL_REG_HOST_FILLED_MASK));
        pCalBuf[count++] = 0x100; //for rx chain1
        pCalBuf[count++] = ((QWLAN_RFAPB_BB_BW1_REG)|( HAL_REG_HOST_FILLED_MASK));
        pCalBuf[count++] = phyCalCorr.ucBw1Chain1;
        pCalBuf[count++] = ((QWLAN_RFAPB_BB_BW2_REG)|( HAL_REG_HOST_FILLED_MASK));
        pCalBuf[count++] = phyCalCorr.ucBw2Chain1;
        pCalBuf[count++] = ((QWLAN_RFAPB_BB_BW3_REG)|( HAL_REG_HOST_FILLED_MASK));
        pCalBuf[count++] = phyCalCorr.ucBw3Chain1;
        pCalBuf[count++] = ((QWLAN_RFAPB_BB_BW4_REG)|( HAL_REG_HOST_FILLED_MASK));
        pCalBuf[count++] = phyCalCorr.ucBw4Chain1;
        pCalBuf[count++] = ((QWLAN_RFAPB_BB_BW5_REG)|( HAL_REG_HOST_FILLED_MASK));
        pCalBuf[count++] = phyCalCorr.ucBw5Chain1;
        pCalBuf[count++] = ((QWLAN_RFAPB_BB_BW6_REG)|( HAL_REG_HOST_FILLED_MASK));
        pCalBuf[count++] = phyCalCorr.ucBw6Chain1;
    }

    //update txloCal values here
    pCalBuf[count++] = ((QWLAN_RFAPB_REV_ID_REG)|( HAL_REG_HOST_FILLED_MASK));
    pCalBuf[count++] = 0;

    pCalBuf[count++] = ((QWLAN_RFAPB_MODE_SEL1_REG)|( HAL_REG_HOST_FILLED_MASK));
    pCalBuf[count++] = 0;/*QWLAN_RFAPB_MODE_SEL1_PLLEN_FORCE_MASK |
                       QWLAN_RFAPB_MODE_SEL1_IQ_DIV_MODE_MASK |
                       QWLAN_RFAPB_MODE_SEL1_EN_TXLO_MODE_MASK |
                       QWLAN_RFAPB_MODE_SEL1_EN_RXLO_MODE_MASK;*/ //0x4000;

    //pCalBuf[count++] = (QWLAN_RFAPB_DA_GAIN_CTL_REG | HAL_REG_FW_FILLED | HAL_REG_FW_EXTRA_FILLED);
    //pCalBuf[count++] = 0;

    //pCalBuf[count++] = (QWLAN_RFAPB_PDET_OVRD_REG | HAL_REG_FW_FILLED | HAL_REG_FW_EXTRA_FILLED);
    //pCalBuf[count++] = 0;

    //pCalBuf[count++] = ((QWLAN_RFAPB_PDET_CTL_REG)|( HAL_REG_HOST_FILLED_MASK));
    //pCalBuf[count++] = 0xe0f0;

    pCalBuf[count++] = ((QWLAN_RFAPB_TX_DCOC_RANGE0_REG)|( HAL_REG_HOST_FILLED_MASK));
    pCalBuf[count++] = 0xaaaa;

    pCalBuf[count++] = ((QWLAN_RFAPB_TX_DCOC_RANGE1_REG)|( HAL_REG_HOST_FILLED_MASK));
    pCalBuf[count++] = 0xaaaa;

    pCalBuf[count++] = ((QWLAN_RFAPB_TX_DCOC_EN0_REG)|( HAL_REG_HOST_FILLED_MASK));
    pCalBuf[count++] = 0xffff;

    for(gainStep = 0; gainStep < NUM_TX_GAIN_STEPS; gainStep++)
    {
        pCalBuf[count++] = ((QWLAN_RFAPB_TX_GAIN_CONTROL_REG)|( HAL_REG_HOST_FILLED_MASK));
        pCalBuf[count++] = gainStep;

        pCalBuf[count++] = ((QWLAN_RFAPB_TX_DCOC_IQ_REG)|( HAL_REG_HOST_FILLED_MASK));
        pCalBuf[count++] = (phyCalCorr.usITxLoCorrChain0 << 8/*GEMINI_TX_DCOC_IQ_TX_DCOC_I_OFFSET*/) |
                           (phyCalCorr.usQTxLoCorrChain0);
    }

    //set the tx dacs //0x30d
    pCalBuf[count++] = ((QWLAN_TXCTL_DAC_CONTROL_REG) | (HAL_REG_HOST_FILLED_MASK));
    pCalBuf[count++] = QWLAN_TXCTL_DAC_CONTROL_TXEN_OVERRIDE_EN_MASK | QWLAN_TXCTL_DAC_CONTROL_DAC_OVERRIDE_EN_MASK |
                      QWLAN_TXCTL_DAC_CONTROL_CH3STDBY_OVERRIDE_VAL_MASK | QWLAN_TXCTL_DAC_CONTROL_CH2STDBY_OVERRIDE_VAL_MASK |
                      QWLAN_TXCTL_DAC_CONTROL_CH0STDBY_OVERRIDE_VAL_MASK;

    // disable all rx pkt types
    pCalBuf[count++] = ((QWLAN_AGC_DIS_MODE_REG) | (HAL_REG_HOST_FILLED_MASK));
    pCalBuf[count++] = 7/*PHY_RX_DISABLE_ALL_TYPES*/;

    //rx xbar - Always "straight" for cal
    pCalBuf[count++] = ((QWLAN_AGC_CONFIG_XBAR_REG)|( HAL_REG_HOST_FILLED_MASK));
    pCalBuf[count++] = 0;

    //force on all the RX enables on //0x333
    pCalBuf[count++] = ((QWLAN_AGC_RX_OVERRIDE_REG)| (HAL_REG_HOST_FILLED_MASK));
    pCalBuf[count++] = QWLAN_AGC_RX_OVERRIDE_OVERRIDE_EN_MASK | QWLAN_AGC_RX_OVERRIDE_ENRX_VAL_MASK |
                      QWLAN_AGC_RX_OVERRIDE_STBY_VAL_MASK;

    //reset AGC
    pCalBuf[count++] = ((QWLAN_AGC_AGC_RESET_REG)| (HAL_REG_HOST_FILLED_MASK));
    pCalBuf[count++] = 1;
    pCalBuf[count++] = ((QWLAN_AGC_AGC_RESET_REG)| (HAL_REG_HOST_FILLED_MASK));
    pCalBuf[count++] = 0;

    //set the calmode to dco
    pCalBuf[count++] = ((QWLAN_CAL_CALMODE_REG)| (HAL_REG_HOST_FILLED_MASK));
    pCalBuf[count++] = QWLAN_CAL_CALMODE_MODE_EINITDCCAL;

    //by pass dco correction //3
    pCalBuf[count++] = ((QWLAN_CAL_OVERRIDE_REG)| (HAL_REG_HOST_FILLED_MASK));
    pCalBuf[count++] = QWLAN_CAL_OVERRIDE_CHN1_DC_MASK | QWLAN_CAL_OVERRIDE_CHN0_DC_MASK;

    pCalBuf[count++] = ((QWLAN_AGC_AGC_RESET_REG)|( HAL_REG_HOST_FILLED_MASK));
    pCalBuf[count++] = 1;


    pCalBuf[count++] = ((QWLAN_RFIF_GC_CFG_REG)|( HAL_REG_HOST_FILLED_MASK));
    pCalBuf[count++] = 0;


    for(rxChain = 0; rxChain < PHY_MAX_RX_CHAINS; rxChain++)
    {
        //select the specific chain we are correcting
        modeSel = 0; /*QWLAN_RFAPB_MODE_SEL1_PLLEN_FORCE_MASK |
                  QWLAN_RFAPB_MODE_SEL1_IQ_DIV_MODE_MASK |
                  QWLAN_RFAPB_MODE_SEL1_EN_TXLO_MODE_MASK |
                  QWLAN_RFAPB_MODE_SEL1_EN_RXLO_MODE_MASK;*/ //0x400f;
        modeSel |= (rxChain << 0x8 /*QWLAN_RFAPB_MODE_SEL1_TXRX_REG_SEL_OFFSET*/);

        pCalBuf[count++] = ((QWLAN_RFAPB_REV_ID_REG)|( HAL_REG_HOST_FILLED_MASK));
        pCalBuf[count++] = 0;

        pCalBuf[count++] = ((QWLAN_RFAPB_MODE_SEL1_REG)|( HAL_REG_HOST_FILLED_MASK));
        pCalBuf[count++] = modeSel;

        pCalBuf[count++] = ((QWLAN_RFAPB_REV_ID_REG)|( HAL_REG_HOST_FILLED_MASK));
        pCalBuf[count++] = 0;

        for(dcoIndex = 0; dcoIndex < NUM_RF_DCO_VALUES; dcoIndex++)
        {
            pCalBuf[count++] = ((QWLAN_RFAPB_RX_GAIN_CONTROL_REG)|( HAL_REG_HOST_FILLED_MASK));
            pCalBuf[count++] = ((dcoIndex << QWLAN_RFAPB_RX_GAIN_CONTROL_RX_GC1_OFFSET) | dcoIndex);

            pCalBuf[count++] = (QWLAN_RFAPB_RX_DCOC_IQ_REG| HAL_REG_HOST_FILLED_MASK);
            pCalBuf[count++] = chainCorr[rxChain];

            //pCalBuf[count++] = ((HAL_REG_REINIT_WAIT_CMD) | (0x320));
        }
    }

    pCalBuf[count++] = ((QWLAN_RFAPB_REV_ID_REG)|( HAL_REG_HOST_FILLED_MASK));
    pCalBuf[count++] = 0;

    pCalBuf[count++] = ((QWLAN_RFIF_GC_CFG_REG)|( HAL_REG_HOST_FILLED_MASK));
    pCalBuf[count++] = QWLAN_RFIF_GC_CFG_TX_GAIN_EN_MASK | QWLAN_RFIF_GC_CFG_RX_GAIN_EN_MASK; //3;

    pCalBuf[count++] = ((QWLAN_AGC_AGC_RESET_REG)| (HAL_REG_HOST_FILLED_MASK));
    pCalBuf[count++] = 0;

    // restore dco correction is performed
    pCalBuf[count++] = ((QWLAN_CAL_OVERRIDE_REG)| (HAL_REG_HOST_FILLED_MASK));
    pCalBuf[count++] = 0;

    //normal mode
    pCalBuf[count++] = ((QWLAN_CAL_CALMODE_REG)| (HAL_REG_HOST_FILLED_MASK));
    pCalBuf[count++] = 0;

    //restore
    pCalBuf[count++] = ((QWLAN_AGC_CONFIG_XBAR_REG)| (HAL_REG_HOST_FILLED_MASK));
    pCalBuf[count++] = 0;
    pCalBuf[count++] = ((QWLAN_AGC_RX_OVERRIDE_REG)| (HAL_REG_HOST_FILLED_MASK));
    pCalBuf[count++] = QWLAN_AGC_RX_OVERRIDE_ENRX_VAL_MASK; //0x30;
    pCalBuf[count++] = ((QWLAN_AGC_AGC_RESET_REG)|( HAL_REG_HOST_FILLED_MASK));
    pCalBuf[count++] = 1;
    pCalBuf[count++] = ((QWLAN_AGC_AGC_RESET_REG)| (HAL_REG_HOST_FILLED_MASK));
    pCalBuf[count++] = 0;

    pCalBuf[count++] = ((QWLAN_AGC_DIS_MODE_REG)| (HAL_REG_HOST_FILLED_MASK));
    pCalBuf[count++] = PHY_RX_DISABLE_SLR;
    pCalBuf[count++] = ((QWLAN_TXCTL_DAC_CONTROL_REG) | (HAL_REG_HOST_FILLED_MASK));
    pCalBuf[count++] = QWLAN_TXCTL_DAC_CONTROL_CH3STDBY_OVERRIDE_VAL_MASK | QWLAN_TXCTL_DAC_CONTROL_CH2STDBY_OVERRIDE_VAL_MASK |
                      QWLAN_TXCTL_DAC_CONTROL_CH1STDBY_OVERRIDE_VAL_MASK | QWLAN_TXCTL_DAC_CONTROL_CH0STDBY_OVERRIDE_VAL_MASK;//0xf;
    //set_rx_gain(0xFF);

    pCalBuf[count++] = ((QWLAN_RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_AGC_REG)| (HAL_REG_HOST_FILLED_MASK));
    pCalBuf[count++] = QWLAN_RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_AGC_PWR_MASK | QWLAN_RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_AGC_ALWAYS_ON_MASK; //3;

    //t.csr.agc.gainset0(override=0,gain=0)
    pCalBuf[count++] = ((QWLAN_AGC_GAINSET0_REG) | (HAL_REG_HOST_FILLED_MASK));
    pCalBuf[count++] = 0;

    //t.csr.agc.gainset1(override=0,gain=0)
    pCalBuf[count++] = ((QWLAN_AGC_GAINSET1_REG) | (HAL_REG_HOST_FILLED_MASK));
    pCalBuf[count++] = 0;

    pCalBuf[count++] = ((QWLAN_AGC_AGC_RESET_REG) | (HAL_REG_HOST_FILLED_MASK));
    pCalBuf[count++] = 1;
    pCalBuf[count++] = ((QWLAN_AGC_AGC_RESET_REG) | (HAL_REG_HOST_FILLED_MASK));
    pCalBuf[count++] = 0;

    if(count >= CAL_BCK_UP_REG_SIZE )
    {
        phyLog(pMac, LOGE, "Backed up calibraton registers count exceeded: %d:\n", count);
        palFreeMemory(pMac->hHdd, pCalBuf);
        VOS_ASSERT(0);

        return eHAL_STATUS_FAILURE;
    }

    //TODO: put a check on count
    palWriteDeviceMemory(pMac->hHdd, (tANI_U32)aduMem, (tANI_U8 *)pCalBuf,
                        (sizeof(tANI_U32) * count));

    palFreeMemory(pMac->hHdd, pCalBuf);

    // Update the start address pointer pointing to the
    // register re-init table
    *pMemAddr = *pMemAddr + (sizeof(tANI_U32) * count);


    return status;
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
    tpAniSirGlobal pMac = (tpAniSirGlobal) hHal;
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
    Qwlanfw_SysCfgType *pFwConfig = (Qwlanfw_SysCfgType *)pMac->hal.FwParam.pFwConfig;

    pMac->hphy.phy.openLoopTxGain = txGain;

    if ((retVal = asicTPCAutomatic(pMac)) != eHAL_STATUS_SUCCESS) { return (retVal); }

    pFwConfig->ucOpenLoopTxGain = pMac->hphy.phy.openLoopTxGain;
    retVal = halFW_UpdateSystemConfig(pMac, pMac->hal.FwParam.fwSysConfigAddr, (tANI_U8 *)pFwConfig, sizeof(Qwlanfw_SysCfgType));

    return retVal;

}

void halPhyAdcRssiStatsCollection(tHalHandle hHal)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal) hHal;

    pttCollectAdcRssiStats(pMac);
}

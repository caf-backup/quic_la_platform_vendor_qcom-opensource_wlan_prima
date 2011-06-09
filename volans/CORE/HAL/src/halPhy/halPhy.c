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

#define SET_CHAIN_SELECT_WAIT       250000000
#define CAL_UPDATE_WAIT             500000000

extern const tANI_U8 plutCharacterized[MAX_TPC_CHANNELS][TPC_MEM_POWER_LUT_DEPTH];
extern const tANI_U8 plutCharacterizedVolans2[MAX_TPC_CHANNELS][TPC_MEM_POWER_LUT_DEPTH];
extern const tANI_U16 plutPdadcOffset[MAX_TPC_CHANNELS];
extern const tANI_U16 plutPdadcOffsetVolans2[MAX_TPC_CHANNELS];

//config called before init but after halNvOpen
eHalStatus halPhyOpen(tHalHandle hHal)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal) hHal;
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
    eRegDomainId curDomain = REG_DOMAIN_FCC;
    sDefaultCountry  *pCountryCode;

    //hard coding the nTx and nRx. Need to fetch them from hal sys config structure
    tANI_U8 nTx = PHY_MAX_TX_CHAINS;
    tANI_U8 nRx = PHY_MAX_RX_CHAINS;

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

    //read the regulatory domain idx from Nv and update the phy state
    if(halGetNvTableLoc(pMac, NV_TABLE_DEFAULT_COUNTRY, (uNvTables **)&pCountryCode) == eHAL_STATUS_SUCCESS)
    {
        curDomain = (eRegDomainId)(pCountryCode->regDomain);
    }
    if ((retVal = halPhySetRegDomain(pMac, curDomain)) != eHAL_STATUS_SUCCESS) { return (retVal); }

    pMac->hphy.phy.regDomainInfo = pMac->hphy.nvTables[NV_TABLE_REGULATORY_DOMAINS ];
    pMac->hphy.phy.pwrOptimal = pMac->hphy.nvTables[NV_TABLE_RATE_POWER_SETTINGS];
    pMac->hphy.phy.antennaPathLoss = pMac->hphy.nvTables[NV_TABLE_ANTENNA_PATH_LOSS];
    pMac->hphy.phy.pktTypePwrLimits = pMac->hphy.nvTables[NV_TABLE_PACKET_TYPE_POWER_LIMITS];

    //intialize the setChan event for wait blocking around halPhySetChannel
    return (halPhy_VosEventInit(hHal));
}

eHalStatus halPhyClose(tHalHandle hHal)
{
    //destroy the setChan event for wait blocking around halPhySetChannel
    return (halPhy_VosEventDestroy(hHal));
}


eHalStatus halPhyStart(tHalHandle hHal)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
    tpAniSirGlobal pMac = (tpAniSirGlobal) hHal;

    pMac->hphy.phy.chanBondState = PHY_SINGLE_CHANNEL_CENTERED;
    pMac->hphy.phy.activeChains = PHY_CHAIN_SEL_R0_T0_ON;
    pMac->hphy.phy.openLoopTxGain = OPEN_LOOP_TX_HIGH_GAIN_OVERRIDE;

    pMac->hphy.rf.curChannel = RF_CHAN_1;   //channel 1 automatically initialized in firmware

    // //do we need this?
    // pMac->hphy.pwr = PHY_POWER_NORMAL;

#ifdef VOLANS_RF
    pMac->hphy.phy.test.testDisableRfAccess = eANI_BOOLEAN_FALSE;
    pMac->hphy.phy.test.testDisablePhyRegAccess = eANI_BOOLEAN_FALSE;
#else    //no phy or RF support in netlist
    pMac->hphy.phy.test.testDisableRfAccess = eANI_BOOLEAN_TRUE;
    pMac->hphy.phy.test.testDisablePhyRegAccess = eANI_BOOLEAN_TRUE;
#endif

#ifndef WLAN_FTM_STUB
    if (pMac->gDriverType == eDRIVER_TYPE_MFG)
    {
        //allocate ADC capture cache
        if (palAllocateMemory(pMac->hHdd, (void **)&pMac->ptt.pADCCaptureCache, PHY_MAX_RX_CHAINS * GRAB_RAM_DBLOCK_SIZE * sizeof(tANI_U32)) != eHAL_STATUS_SUCCESS)
        {
            phyLog(pMac, LOGE, "unable to allocate memory for ADC capture. \n");
            return eHAL_STATUS_FAILURE;
        }
    }
#endif

#ifdef VOLANS_RF
    asicEnablePhyClocks(pMac);

    GET_PHY_REG(pMac->hHdd, QWLAN_RFAPB_REV_ID_REG, &(pMac->hphy.rf.revId));
#endif

#if !defined(WLAN_FTM_STUB) && defined(VOLANS_RF)
    if (pMac->gDriverType == eDRIVER_TYPE_MFG)
    {
        pttModuleInit(pMac);

        //start the ADC RSSI stats collection timer
        halTimersCreate(pMac);
    }
#endif

            return (retVal);
}


eHalStatus halPhyStop(tHalHandle hHal)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal) hHal;
    pMac->hphy.rf.curChannel = INVALID_RF_CHANNEL;

#ifndef WLAN_FTM_STUB
    if (pMac->gDriverType == eDRIVER_TYPE_MFG)
    {
        if(pMac->ptt.pADCCaptureCache)
        {
            palFreeMemory(pMac->hHdd, pMac->ptt.pADCCaptureCache);
        }
        }
#endif

    return (eHAL_STATUS_SUCCESS);
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
        case PHY_CHAIN_SEL_BT_R0_T0_ON:
            return (1);
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
        case PHY_CHAIN_SEL_NO_RX_TX:
            return (0);
        case PHY_CHAIN_SEL_R0_T0_ON:
        case PHY_CHAIN_SEL_T0_ON:
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
#ifndef WLAN_FTM_STUB
    if(pMac->gDriverType == eDRIVER_TYPE_MFG)
    {
        halPhyWaitForInterrupt(hHal, QWLANFW_FW2HOST_SET_CHAIN_SELECT_RSP);
        pMac->hphy.phy.activeChains = phyChainSelections;
    }
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

    if(pMac->gDriverType == eDRIVER_TYPE_MFG)
    {
        calUpdate.usCalId = pMac->hphy.phy.test.testCalMode;
    }
    else
    {
        calUpdate.usCalId = ALL_CALS;
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
    eRfChannels rfChannel;
    Qwlanfw_SetChannelReqType setChan;

   assert(cbState == PHY_SINGLE_CHANNEL_CENTERED);
   assert(channelNumber <= NUM_2_4GHZ_CHANNELS);

    setChan.usChanNum = channelNumber;
    setChan.ucCbState = (tANI_U8)cbState;
    setChan.ucRegDomain = (tANI_U8)halPhyGetRegDomain(pMac);
    setChan.ucCalRequired = (tANI_U32)NO_CALS;

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

    {
        rfChannel = rfGetChannelIndex(channelNumber, cbState);

		VOS_ASSERT(rfChannel != INVALID_RF_CHANNEL);
        /* Don't do copy in case of scanning, PLUTs are not used */
        if (calRequired)
        {
            tANI_U32 *tempPtr = (tANI_U32 *)&setChan.tpcPowerLUT[0];
            tANI_U32 *pTpcPwrLUT = NULL;
            tANI_U32 i = 0;

            if (RF_CHIP_VERSION(RF_CHIP_ID_VOLANS2))
            {
                tTpcPowerTable *plutCharacterizedVolans2 = (tTpcPowerTable *)(&pMac->hphy.nvCache.tables.plutCharacterized[0]);
                tANI_U16 *plutPdadcOffsetVolans2 = (tANI_U16 *)(&pMac->hphy.nvCache.tables.plutPdadcOffset[0]);

                pTpcPwrLUT = (tANI_U32 *)&(plutCharacterizedVolans2[rfChannel][0][0]);
                setChan.pdAdcOffset = plutPdadcOffsetVolans2[rfChannel];
            }
            else
            {
                pTpcPwrLUT = (tANI_U32 *)&plutCharacterized[rfChannel][0];
                setChan.pdAdcOffset = plutPdadcOffset[rfChannel];
            }

            for (;i < TPC_MEM_POWER_LUT_DEPTH; i += 4)
            {
                *tempPtr = sirSwapU32(*pTpcPwrLUT);
                tempPtr++;
                pTpcPwrLUT++;
            }

#ifndef WLAN_FTM_STUB
            if (pMac->gDriverType == eDRIVER_TYPE_MFG)
            {
                setChan.ucCalRequired = (tANI_U32)ALL_CALS;
            }
            else
#endif
            {
                setChan.ucCalRequired = (tANI_U32)RX_DCO_CAL_ONLY;
            }
        }
    }

    //send a mailbox message to the firmware to set the channel
    retVal = halFW_SendMsg(pMac, HAL_MODULE_ID_PHY, QWLANFW_HOST2FW_SET_CHANNEL_REQ, dialogToken,
                                 sizeof(Qwlanfw_SetChannelReqType), &setChan, FALSE, NULL);
    if(retVal != eHAL_STATUS_SUCCESS) { return (retVal); }

    if(pMac->gDriverType == eDRIVER_TYPE_MFG)
    {
       halPhyWaitForInterrupt(hHal, QWLANFW_FW2HOST_SET_CHANNEL_RSP);
       //sirBusyWait(SET_CHANNEL_WAIT);
            }

            {
        pMac->hphy.phy.chanBondState = PHY_SINGLE_CHANNEL_CENTERED/*cbState*/;

        pMac->hphy.rf.curChannel = rfChannel;
    }

    //Write AGC RSSI offset for 11b packets.
    {
        tANI_U32 value;
        sRssiChannelOffsets *rssiChanOffsets = (sRssiChannelOffsets *)(&pMac->hphy.nvCache.tables.rssiChanOffsets[0]);

        value = (AGC_RSSI_OFFSET_DEFAULT +
                    ((rssiChanOffsets[PHY_RX_CHAIN_0].bRssiOffset[rfChannel]) / 100));

        halWriteRegister(pMac, QWLAN_AGC_RSSI_OFFSET0_REG, value);
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



eHalStatus halPhyUpdateTxGainOverride(tHalHandle hHal, tANI_U8 txGain)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal) hHal;
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
    Qwlanfw_SysCfgType *pFwConfig = (Qwlanfw_SysCfgType *)pMac->hal.FwParam.pFwConfig;

    pFwConfig->ucOpenLoopTxGain = txGain;

    if ((retVal = asicTPCAutomatic(pMac)) != eHAL_STATUS_SUCCESS)
    {
        return (retVal);
        }

    pFwConfig->ucOpenLoopTxGain = pMac->hphy.phy.openLoopTxGain;
    retVal = halFW_UpdateSystemConfig(pMac, pMac->hal.FwParam.fwSysConfigAddr, (tANI_U8 *)pFwConfig, sizeof(Qwlanfw_SysCfgType));

    return retVal;
}

#ifdef HALPHY_CAL_DEBUG
extern void phyRxDcoCal(tpAniSirGlobal pMac, tANI_U8 loadDcoOnly);
extern void phyRxIm2Cal(tpAniSirGlobal pMac, tANI_U8 Im2CalOnly);
#endif

eHalStatus halPhyFwInitDone(tHalHandle hHal)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
    tpAniSirGlobal pMac = (tpAniSirGlobal) hHal;

    /** Update NV Cache Table */
    {
        uNvTables   nvTable;
        sCalStatus  fwCalStatus;

        if (halReadDeviceMemory(pMac, QWLANFW_MEM_PHY_CAL_STATUS_ADDR_OFFSET,
                    (void *)&fwCalStatus, sizeof(fwCalStatus)) != eHAL_STATUS_SUCCESS)
        {
            phyLog(pMac, LOGP, FL("Could not read CalStatus from device memory\n"));
            return eHAL_STATUS_FAILURE;
        }

        nvTable.rFCalValues.calStatus = 0;

        if (!fwCalStatus.process_monitor)
            nvTable.rFCalValues.calStatus |= eNV_CALID_PROCESS_MONITOR;

        if (!fwCalStatus.hdet_dco)
            nvTable.rFCalValues.calStatus |= eNV_CALID_HDET_CAL_CODE;

        if (!fwCalStatus.pllVcoLinearity)
            nvTable.rFCalValues.calStatus |= eNV_CALID_PLL_VCO_LINEARITY;

        if (!fwCalStatus.rtuner)
            nvTable.rFCalValues.calStatus |= eNV_CALID_RTUNER;

        if (!fwCalStatus.ctuner)
            nvTable.rFCalValues.calStatus |= eNV_CALID_CTUNER;

        if (!fwCalStatus.im2UsingToneGen || !fwCalStatus.im2UsingNoisePwr)
            nvTable.rFCalValues.calStatus |= eNV_CALID_RX_IM2;

        if (!fwCalStatus.temperature)
            nvTable.rFCalValues.calStatus |= eNV_CALID_TEMPERATURE;

        if (!fwCalStatus.lnaBias)
            nvTable.rFCalValues.calStatus |= eNV_CALID_LNA_BANDSETTING;

        if (!fwCalStatus.lnaBandTuning)
            nvTable.rFCalValues.calStatus |= eNV_CALID_LNA_BANDTUNING;

        if (!fwCalStatus.lnaGainAdjust)
            nvTable.rFCalValues.calStatus |= eNV_CALID_LNA_GAINADJUST;

        if (halReadDeviceMemory(pMac, QWLANFW_MEM_PHY_CAL_CORRECTIONS_ADDR_OFFSET,
                    (void *)&(nvTable.rFCalValues.calData),
                    sizeof(nvTable.rFCalValues.calData)) != eHAL_STATUS_SUCCESS)
        {
            phyLog(pMac, LOGP, FL("Could not read CalCorrections from device memory\n"));
            return eHAL_STATUS_FAILURE;
        }

        halWriteNvTable(pMac, NV_TABLE_RF_CAL_VALUES, (uNvTables *)&nvTable);
    }

    retVal = phyTxPowerInit(pMac);

#ifndef WLAN_FTM_STUB
    if (pMac->gDriverType == eDRIVER_TYPE_MFG)
    {
        halStoreTableToNv(pMac, NV_TABLE_RF_CAL_VALUES);
        //comment out the Noise Gain figure improvement changes as it is affecting max sensitivity results.
#if 0
        //t.csr.rfapb.rxfe_lna_highgain_bias_ctl(rxfe_lna_highgain_bias_ctl=0xff) -- Greg's email (11/8/2010 12:29PM)
        SET_PHY_REG(pMac->hHdd, QWLAN_RFAPB_RXFE_LNA_HIGHGAIN_BIAS_CTL_REG, 0xFF);

        //depot2/hardware/gen7/lab/python/pythonlab/bbic/volans.py#80

        //t.csr.rfapb.rxfe_lna_highgain_bias_ctl.rxfe_lna_highgain_bias_ctl(0xff)
        //t.csr.rfapb.bbf_aux2.tia_config(0)
        rdModWrAsicField(pMac, QWLAN_RFAPB_BBF_AUX2_REG, QWLAN_RFAPB_BBF_AUX2_TIA_CONFIG_MASK,
                            QWLAN_RFAPB_BBF_AUX2_TIA_CONFIG_OFFSET, 0);
        //t.csr.rfapb.rxfe_gm_3.rxfe_gm_ibias_core_ctrl(2)
        rdModWrAsicField(pMac, QWLAN_RFAPB_RXFE_GM_3_REG, QWLAN_RFAPB_RXFE_GM_3_RXFE_GM_IBIAS_CORE_CTRL_MASK,
                            QWLAN_RFAPB_RXFE_GM_3_RXFE_GM_IBIAS_CORE_CTRL_OFFSET, 2);
#endif
        //t.csr.rfapb.tx_delay6(pa_st3_start_prd=29, reserved=0, pa_st3_end_prd=0)
        SET_PHY_REG(pMac->hHdd, QWLAN_RFAPB_TX_DELAY6_REG,
                           ((29 << QWLAN_RFAPB_TX_DELAY6_PA_ST3_START_PRD_OFFSET) |
                            (0 << QWLAN_RFAPB_TX_DELAY6_PA_ST3_END_PRD_OFFSET)));

        //run the loopback CLPC evm cal to record the ofdm pwr offsets in NV
        //if(pMac->hphy.nvCache.tables.ofdmCmdPwrOffset.ofdmPwrOffset == 0)
        //{
        //    phyClpcLpbkCal(pMac);
        //}

        //force the ofdm offset to 0 till the CLPC characterization issues are resolved
        pMac->hphy.nvCache.tables.ofdmCmdPwrOffset.ofdmPwrOffset = 0;
    }
    else
#endif
    {
        /* Disable PHYDBG clocks in production driver as we are not disabling it in
         * asicStopWaveform in firmware during calibration.
         * In FTM driver this is needed for RxPktCounts in Rx only mode, so leave
         * it running.
         */
        rdModWrAsicField(pMac, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_REG,
                                QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_PHYDBG_MASK,
                                QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_PHYDBG_OFFSET, 0);
        rdModWrAsicField(pMac, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_REG,
                                QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_PHYDBG_APB_MASK,
                                QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_PHYDBG_APB_OFFSET, 0);
    }

#ifdef HALPHY_CAL_DEBUG
    phyRxIm2Cal(pMac, eANI_BOOLEAN_FALSE);
    phyRxDcoCal(pMac, eANI_BOOLEAN_FALSE);
#endif

    return retVal;
}
#ifndef WLAN_FTM_STUB
void halPhyAdcRssiStatsCollection(tHalHandle hHal)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal) hHal;

    pttCollectAdcRssiStats(pMac);
}
#endif

#ifdef WLAN_SOFTAP_FEATURE
#define RF_ANT_EN_MASK_FOR_1_RX      0x1
#define RF_ANT_EN_MASK_FOR_2_RX      0x3

// This function enables the AGC Listen Mode
eHalStatus halPhyAGCEnableListenMode(tHalHandle hHal, tANI_U8 EDETThreshold)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
#ifdef FIXME_VOLANS
    tANI_U16   rf_ant_en = 1, number = 0, cw_detect_dis = 1, val;
    tANI_U16   bbf_sat5_egy_thres_in, bbf_sat5_egy_thres_man = 0;
    tpAniSirGlobal pMac = (tpAniSirGlobal) hHal;

    assert(pMac != 0);

    // get the RX antenna info to determine value of rf_ant_en
    val = (tANI_U16) halPhyQueryNumRxChains(pMac->hphy.phy.cfgChains);
    switch(val)
    {
        case 1:
           rf_ant_en = RF_ANT_EN_MASK_FOR_1_RX;
           break;
        case 2:
           rf_ant_en = RF_ANT_EN_MASK_FOR_2_RX;
           break;

        default:
           return eHAL_STATUS_FAILURE;
    }

    // configure register AGC_N_LISTEN_REG
    val = ((rf_ant_en << QWLAN_AGC_N_LISTEN_RF_ANT_EN_OFFSET) | (number << QWLAN_AGC_N_LISTEN_NUMBER_OFFSET));
    SET_PHY_REG(pMac->hHdd, QWLAN_AGC_N_LISTEN_REG, val);

    // configure register AGC_N_CAPTURE_REG
    val = ((rf_ant_en << QWLAN_AGC_N_CAPTURE_RF_ANT_EN_OFFSET) | (number << QWLAN_AGC_N_CAPTURE_NUMBER_OFFSET));
    SET_PHY_REG(pMac->hHdd, QWLAN_AGC_N_CAPTURE_REG, val);

    // configure register AGC_CW_DETECT_REG
    retVal = rdModWrAsicField( pMac, QWLAN_AGC_CW_DETECT_REG, QWLAN_AGC_CW_DETECT_DIS_MASK,
                               QWLAN_AGC_CW_DETECT_DIS_OFFSET, cw_detect_dis);

    // configure register RFAPB_BBF_SAT5_REG
    bbf_sat5_egy_thres_in = (tANI_U16) EDETThreshold;
    val = ((bbf_sat5_egy_thres_man << QWLAN_RFAPB_BBF_SAT5_EGY_THRES_MAN_OFFSET) |
          (bbf_sat5_egy_thres_in  << QWLAN_RFAPB_BBF_SAT5_EGY_THRES_IN_OFFSET));
    SET_PHY_REG(pMac->hHdd, QWLAN_RFAPB_BBF_SAT5_REG, val);
#endif
    return (retVal);
}

// This function disables the AGC Listen Mode
eHalStatus halPhyAGCDisableListenMode(tHalHandle hHal)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
#ifdef FIXME_VOLANS
    tANI_U16   rf_ant_en = 0, number, cw_detect_dis = 0, val;
    tpAniSirGlobal pMac = (tpAniSirGlobal) hHal;

    assert(pMac != 0);

    // configure register AGC_N_LISTEN_REG
    number = (tANI_U16) halPhyQueryNumRxChains(pMac->hphy.phy.cfgChains);
    val = ((rf_ant_en << QWLAN_AGC_N_LISTEN_RF_ANT_EN_OFFSET) | (number << QWLAN_AGC_N_LISTEN_NUMBER_OFFSET));
    SET_PHY_REG(pMac->hHdd, QWLAN_AGC_N_LISTEN_REG, val);

    // configure register AGC_N_CAPTURE_REG
    val = ((rf_ant_en << QWLAN_AGC_N_CAPTURE_RF_ANT_EN_OFFSET) | (number << QWLAN_AGC_N_CAPTURE_NUMBER_OFFSET));
    SET_PHY_REG(pMac->hHdd, QWLAN_AGC_N_CAPTURE_REG, val);

    // configure register AGC_CW_DETECT_REG
    retVal = rdModWrAsicField( pMac, QWLAN_AGC_CW_DETECT_REG, QWLAN_AGC_CW_DETECT_DIS_MASK,
                               QWLAN_AGC_CW_DETECT_DIS_OFFSET, cw_detect_dis);

    // configure register RFAPB_BBF_SAT5_REG
    val = ((QWLAN_RFAPB_BBF_SAT5_EGY_THRES_MAN_DEFAULT << QWLAN_RFAPB_BBF_SAT5_EGY_THRES_MAN_OFFSET) |
          (QWLAN_RFAPB_BBF_SAT5_EGY_THRES_IN_DEFAULT  << QWLAN_RFAPB_BBF_SAT5_EGY_THRES_IN_OFFSET));
    SET_PHY_REG(pMac->hHdd, QWLAN_RFAPB_BBF_SAT5_REG, val);
#endif
    return (retVal);
}
#endif
#ifndef VERIFY_HALPHY_SIMV_MODEL
inline void hv_printLog(const char *str, ...)
{
    //Only used when testing on SIMV Model
}
#endif

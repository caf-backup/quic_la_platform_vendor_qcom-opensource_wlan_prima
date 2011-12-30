/**
 *

 * Copyright (c) 2011 Qualcomm Atheros, Inc. 
 * All Rights Reserved. 
 * Qualcomm Atheros Confidential and Proprietary. 
 * 
 * Copyright (C) 2006 Airgo Networks, Incorporated
 * 
 */

#include "sys_api.h"


#define SUCCESS     PTT_STATUS_SUCCESS
#define FAILURE     PTT_STATUS_FAILURE


#ifdef VERIFY_HALPHY_SIMV_MODEL
extern void verifyVcoLinearityCal(void);
#endif


#ifndef WLAN_FTM_STUB
#include "pttModuleApi.h"

//Device Register Access
eQWPttStatus pttDbgReadRegister(tpAniSirGlobal pMac, tANI_U32 regAddr, tANI_U32 *regValue)
{
    if (palReadRegister(pMac->hHdd, regAddr, regValue) != eHAL_STATUS_SUCCESS)
    {
        return (FAILURE);
    }

    return (SUCCESS);
}


eQWPttStatus pttDbgWriteRegister(tpAniSirGlobal pMac, tANI_U32 regAddr, tANI_U32 regValue)
{
    if (palWriteRegister(pMac->hHdd, regAddr, regValue) != eHAL_STATUS_SUCCESS)
    {
        return (FAILURE);
    }

    return (SUCCESS);
}

#include "pttModuleApi.h"



const tANI_U8 defaultSourceMacAddress[ANI_MAC_ADDR_SIZE] = { 0x22, 0x22, 0x44, 0x44, 0x33, 0x33 };  //STA
const tANI_U8 defaultDestMacAddress[ANI_MAC_ADDR_SIZE] =   { 0x22, 0x22, 0x11, 0x11, 0x33, 0x33 };  //AP
const tANI_U8 defaultBssIdMacAddress[ANI_MAC_ADDR_SIZE] =  { 0x22, 0x22, 0x11, 0x11, 0x33, 0x33 };



sAgcGainLut pttAgcGainLUT[PHY_MAX_RX_CHAINS];



tANI_U8 defaultAddr1[ANI_MAC_ADDR_SIZE] = { 0x00, 0x77, 0x55, 0x33, 0x11, 0x00 };   //dest
tANI_U8 defaultAddr3[ANI_MAC_ADDR_SIZE] = { 0x00, 0x77, 0x55, 0x33, 0x11, 0x00 };   //bssId

void pttModuleInit(tpAniSirGlobal pMac)
{
    uNvFields fields;
    pMac->hphy.phy.test.testChannelId = 1;
    pMac->hphy.phy.test.testCbState = PHY_SINGLE_CHANNEL_CENTERED;
    pMac->hphy.phy.test.testTpcClosedLoop = eANI_BOOLEAN_TRUE;
    pMac->hphy.phy.test.testTxGainIndexSource = RATE_POWER_NON_LIMITED;
    pMac->hphy.phy.test.testForcedTxPower = 0;
    pMac->hphy.phy.test.testLastPwrIndex = 0;
    pMac->hphy.phy.test.testForcedTxGainIndex = 0;
    pMac->hphy.phy.test.identicalPayloadEnabled = eANI_BOOLEAN_FALSE;

    pMac->ptt.forcedTxGain[PHY_TX_CHAIN_0].coarsePwr = TPC_COARSE_TXPWR_6;
    pMac->ptt.forcedTxGain[PHY_TX_CHAIN_0].finePwr = TPC_FINE_TXPWR_7;

    //initialize frame gen variables
    pMac->ptt.frameGenEnabled = eANI_BOOLEAN_FALSE;
    pMac->ptt.phyDbgFrameGen = eANI_BOOLEAN_TRUE;

    pMac->ptt.frameGenParams.numTestPackets = 0;   //Continuous
    pMac->ptt.frameGenParams.interFrameSpace = 10;
    pMac->ptt.frameGenParams.rate = HAL_PHY_RATE_11A_6_MBPS;
    pMac->ptt.frameGenParams.payloadContents = TEST_PAYLOAD_RANDOM;
    pMac->ptt.frameGenParams.payloadLength = MAX_PAYLOAD_SIZE;
    pMac->ptt.frameGenParams.payloadFillByte = 0xA5;
    pMac->ptt.frameGenParams.pktAutoSeqNum = eANI_BOOLEAN_FALSE;
    pMac->ptt.frameGenParams.pktScramblerSeed = 7;
    pMac->ptt.frameGenParams.crc = 0;
    pMac->ptt.frameGenParams.preamble = PHYDBG_PREAMBLE_OFDM;
    memcpy(&pMac->ptt.frameGenParams.addr1[0], defaultAddr1, ANI_MAC_ADDR_SIZE);
    if (eHAL_STATUS_SUCCESS == halReadNvField(pMac, NV_COMMON_MAC_ADDR, &fields))
    {
        memcpy(&pMac->ptt.frameGenParams.addr2[0], &fields, ANI_MAC_ADDR_SIZE);
    }
    else
    {
        memset(&pMac->ptt.frameGenParams.addr2[0], 0xFF, ANI_MAC_ADDR_SIZE);
    }
    memcpy(&pMac->ptt.frameGenParams.addr3[0], defaultAddr3, ANI_MAC_ADDR_SIZE);

    pMac->ptt.agcEnables.rx[PHY_RX_CHAIN_0] = eANI_BOOLEAN_TRUE;

    pMac->ptt.wfmEnabled = eANI_BOOLEAN_FALSE;
    pMac->ptt.wfmStored = eANI_BOOLEAN_FALSE;

    // store rx agc gain luts for restoration later
    {

        //!Loading the gain tables for Gen6 is not necessary as they are initialized using a const table from the RF code.
        //asicGetAgcGainLut(pMac, PHY_RX_CHAIN_0, pttAgcGainLUT[PHY_RX_CHAIN_0]);
    }

    /*! moved this from pttMsgInit, so we need to verify the init sequence with and without PTT GUI
        The idea is to have pttModuleInit initialize everything for manufacturing, and then pttMsgInit
        allows PTT GUI to get the initialized values.
        Anything that should be reinitialized or only done when PTT GUI is connecting can be done in pttMsgInit
    */

    pMac->hphy.phy.phyPeriodicCalEnable = eANI_BOOLEAN_FALSE; // ignore periodic calibrations.

#if 0
    //always leave phydbg clock on for ptt
    rdModWrAsicField(pMac, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_REG,
                           QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_PHYDBG_MASK,
                           QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_PHYDBG_OFFSET,
                           1
                    );
#endif

#if 0
    // Clear stats
    pttResetRxPacketStatistics(pMac);
#endif
}


// Msg Init - preparing for starting PTT and also providing default values back to GUI
eQWPttStatus pttMsgInit(tpAniSirGlobal pMac, tPttModuleVariables** ptt)
{
    phyLog(pMac, LOGE, "pttMsgInit: initialize PTT. \n");

    pttModuleInit(pMac);    //reinitialize when this message is received
    *ptt = &pMac->ptt;

    return SUCCESS;
}



//NV Service
eQWPttStatus pttGetNvTable(tpAniSirGlobal pMac, eNvTable nvTable, uNvTables *tableData)
{
    eHalStatus rc = eHAL_STATUS_FAILURE;

    rc = halReadNvTable(pMac, nvTable, tableData);

    if (eHAL_STATUS_SUCCESS == rc)
    {
        return (SUCCESS);
    }
    else
    {
        return (FAILURE);
    }
}



eQWPttStatus pttSetNvTable(tpAniSirGlobal pMac, eNvTable nvTable, uNvTables *tableData)
{

    if ((tANI_U32)nvTable >= NUM_NV_TABLE_IDS)
    {
        return (FAILURE);
    }
    else
    {
        /* We want this table to take effect immediately for mfg purposes. Otherwise, the adapter has to be
            reset for these values to take effect per initialization.
        */
        //Since all phy tables are operating on the cache,
        // just retune the channel and any tables
        switch (nvTable)
        {
            case NV_FIELDS_IMAGE:
            case NV_TABLE_RATE_POWER_SETTINGS:  //data received in t2Decimal format - see uAbsPwrPrecision
            case NV_TABLE_REGULATORY_DOMAINS:
            case NV_TABLE_DEFAULT_COUNTRY:
            case NV_TABLE_TPC_POWER_TABLE:
            case NV_TABLE_TPC_PDADC_OFFSETS:
            //case NV_TABLE_CAL_MEMORY:
            //case NV_TABLE_CAL_STATUS:
            case NV_TABLE_RSSI_CHANNEL_OFFSETS:
            case NV_TABLE_RF_CAL_VALUES:
            case NV_TABLE_ANTENNA_PATH_LOSS:
            case NV_TABLE_PACKET_TYPE_POWER_LIMITS:
            case NV_TABLE_OFDM_CMD_PWR_OFFSET:
            case NV_TABLE_TX_BB_FILTER_MODE:
            case NV_TABLE_FREQUENCY_FOR_1_3V_SUPPLY:

                if (eHAL_STATUS_FAILURE == halWriteNvTable(pMac, nvTable, tableData))
                {
                    phyLog(pMac, LOGE, "Unable to write table %d\n", (tANI_U32)nvTable);
                    return (FAILURE);
                }
                break;
            default:
                phyLog(pMac, LOGE, "ERROR: table %d not programmed\n", nvTable);
                break;
        }
    }

    return (SUCCESS);
}




eQWPttStatus pttDelNvTable(tpAniSirGlobal pMac, eNvTable nvTable)
{
    eHalStatus rc = eHAL_STATUS_FAILURE;
    rc = halRemoveNvTable(pMac, nvTable);

    if (eHAL_STATUS_SUCCESS == rc)
    {
        //as a precaution, since we've removed the nv table, copy the defaults back
        switch (nvTable)
        {
            case NV_TABLE_REGULATORY_DOMAINS:
                pMac->hphy.phy.regDomainInfo = NULL;
                break;
            case NV_TABLE_RATE_POWER_SETTINGS:
                pMac->hphy.phy.pwrOptimal = NULL;
                break;
            default:
                break;
        }
       return (SUCCESS);
    }
    else
    {
        return (FAILURE);
    }
}


eQWPttStatus pttBlankNv(tpAniSirGlobal pMac)
{
    eHalStatus rc = eHAL_STATUS_FAILURE;

    rc = halBlankNv(pMac);
    if (eHAL_STATUS_SUCCESS == rc)
    {
       return (SUCCESS);
    }
    else
    {
        return (FAILURE);
    }
}



eQWPttStatus pttGetNvField(tpAniSirGlobal pMac, eNvField nvField, uNvFields *fieldData)
{
    if (eHAL_STATUS_SUCCESS == halReadNvField(pMac, nvField, fieldData))
    {
       return (SUCCESS);
    }
    else
    {
        return (FAILURE);
    }
}

eQWPttStatus pttSetNvField(tpAniSirGlobal pMac, eNvField nvField, uNvFields *fieldData)
{
    if (eHAL_STATUS_SUCCESS == halWriteNvField(pMac, nvField, fieldData))
    {
       return (SUCCESS);
    }
    else
    {
        return (FAILURE);
    }
}

eQWPttStatus pttStoreNvTable(tpAniSirGlobal pMac, eNvTable nvTable)
{
    if (eHAL_STATUS_SUCCESS == halStoreTableToNv(pMac, nvTable))
    {
        return (SUCCESS);
    }
    else
    {
        return (FAILURE);
    }
}

eQWPttStatus pttSetRegDomain(tpAniSirGlobal pMac, eRegDomainId regDomainId)
{
    if (eHAL_STATUS_SUCCESS == halPhySetRegDomain(pMac, regDomainId))
    {
        return (SUCCESS);
    }
    else
    {
        return (FAILURE);
    }
}


eQWPttStatus pttDbgReadMemory(tpAniSirGlobal pMac, tANI_U32 memAddr, tANI_U32 nBytes, tANI_U32 *pMemBuf)
{
    //TODO: add bounds checking on pttDbgReadMemory params
    if ((tANI_U32)pMemBuf % 4 != 0)
    {
        return (FAILURE);
    }

    if (eHAL_STATUS_SUCCESS == palCopyMemory(pMac->hHdd, pMemBuf, (void *)memAddr, nBytes))
    {
        return (SUCCESS);
    }
    else
    {
        return (FAILURE);
    }
}


eQWPttStatus pttDbgWriteMemory(tpAniSirGlobal pMac, tANI_U32 memAddr, tANI_U32 nBytes, tANI_U32 *pMemBuf)
{

    if ((tANI_U32)pMemBuf % 4 != 0)
    {
        return (FAILURE);
    }

    if (eHAL_STATUS_SUCCESS == palCopyMemory(pMac->hHdd, (void *)memAddr, pMemBuf, nBytes))
    {
        return (SUCCESS);
    }
    else
    {
        return (FAILURE);
    }
}


//Device MAC Test Setup
eQWPttStatus pttSetChannel(tpAniSirGlobal pMac, tANI_U32 chId, ePhyChanBondState cbState)
{
    eQWPttStatus rc = SUCCESS;
    tANI_U8 oldChan = pMac->hphy.phy.test.testChannelId;
    ePhyChanBondState oldCbState = pMac->hphy.phy.test.testCbState;
    tANI_BOOLEAN frameGenEnabled = pMac->ptt.frameGenEnabled;

    if (frameGenEnabled && (pMac->ptt.phyDbgFrameGen))
    {
        //stop frame gen - restart after changing chain selections
        pttStartStopTxPacketGen(pMac, eANI_BOOLEAN_OFF);
    }


    pMac->hphy.phy.test.testChannelId = (tANI_U8)chId;
    pMac->hphy.phy.test.testCbState = cbState;

    if (!(cbState == PHY_SINGLE_CHANNEL_CENTERED
#ifdef CHANNEL_BONDED_CAPABLE
            || cbState == PHY_DOUBLE_CHANNEL_LOW_PRIMARY
            || cbState == PHY_DOUBLE_CHANNEL_HIGH_PRIMARY
#endif
         ) ||
         (halPhySetChannel(pMac, (tANI_U8)chId, (ePhyChanBondState)cbState, TRUE
#ifdef WLAN_AP_STA_CONCURRENCY
                           , FALSE
#endif
                           ) != eHAL_STATUS_SUCCESS)
       )
    {
        //failed channel setting - put back old settings
        pMac->hphy.phy.test.testChannelId = oldChan;
        pMac->hphy.phy.test.testCbState = oldCbState;
        phyLog(pMac, LOGE, "!Could not set channel %d for CB=%d\n", chId, cbState);
        rc = FAILURE;
    }
    else
    {
        rc = SUCCESS;

        //palWriteRegister(pMac->hHdd, QWLAN_RFAPB_SW_OVERRIDE_REG, 0x7e9);

        // store rx agc gain luts for restoration later - these are frequency specific
        {
            //!Loading the gain tables for Gen6 is not necessary as they are initialized using a const table from the RF code.
            //asicGetAgcGainLut(pMac, PHY_RX_CHAIN_0, pttAgcGainLUT[PHY_RX_CHAIN_0]);
        }

        if ((frameGenEnabled  == eANI_BOOLEAN_TRUE) && (pMac->ptt.phyDbgFrameGen == eANI_BOOLEAN_TRUE))
        {
            if ((oldCbState != PHY_SINGLE_CHANNEL_CENTERED) && (cbState == PHY_SINGLE_CHANNEL_CENTERED))
            {
                //going from 40 to 20MHz - if rate is not compatible then don't restart
#ifdef CHANNEL_BONDED_CAPABILITY
                if (TEST_PHY_RATE_IS_CB(pMac->ptt.frameGenParams.rate) == eANI_BOOLEAN_TRUE)
                {
                    //channel bonded rates not supported on 20MHz channel
                    return (rc);
                }
#endif
            }

            //restart frame gen
            if (pttStartStopTxPacketGen(pMac, eANI_BOOLEAN_ON) == FAILURE)
            {
                return (FAILURE);
            }
        }
    }

    return rc;
}



eQWPttStatus pttEnableChains(tpAniSirGlobal pMac, ePhyChainSelect chainSelection)
{
    tANI_BOOLEAN stopTx = eANI_BOOLEAN_FALSE;
    tANI_BOOLEAN frameGenEnabled = pMac->ptt.frameGenEnabled;

    if (frameGenEnabled && (pMac->ptt.phyDbgFrameGen))
    {
        //stop frame gen - restart after changing chain selections
        pttStartStopTxPacketGen(pMac, eANI_BOOLEAN_OFF);
    }

    if (halPhyQueryNumTxChains(chainSelection) == 0)
    {
        stopTx = eANI_BOOLEAN_TRUE;
    }

    if (eHAL_STATUS_SUCCESS == halPhySetChainSelect(pMac, chainSelection))
    {
        //chain selection set, but we need to make sure the correct receive chains are set for test modes

        //now that the receive chains are setup, check to see the transmit chains need to be overriden
        if (pMac->ptt.wfmEnabled)
        {
            //for waveforms, we need to make sure the transmitters are overriden correctly
            switch (chainSelection)
            {
                case PHY_CHAIN_SEL_R0_T0_ON:
                case PHY_CHAIN_SEL_T0_ON:
                    if (asicEnableTxDACs(pMac, PHY_TX_CHAIN_0, eANI_BOOLEAN_ON, pMac->ptt.wfmEnabled) != eHAL_STATUS_SUCCESS) { return (FAILURE); }
                    break;

                case PHY_CHAIN_SEL_R0_ON:
                case PHY_CHAIN_SEL_NO_RX_TX:
                    pttStopWaveform(pMac);
                    if (asicEnableTxDACs(pMac, PHY_NO_TX_CHAINS, eANI_BOOLEAN_ON, eANI_BOOLEAN_OFF) != eHAL_STATUS_SUCCESS) { return (FAILURE); }
                    break;

                default:
                    assert(0);
                    return (FAILURE);
                    break;
            }


            //set the waveform gain again after the change in enabled tx chains
            if (eHAL_STATUS_SUCCESS != asicTPCPowerOverride(pMac,  pMac->ptt.forcedTxGain[PHY_TX_CHAIN_0],
                                                                   pMac->ptt.forcedTxGain[PHY_TX_CHAIN_0],
                                                                   pMac->ptt.forcedTxGain[PHY_TX_CHAIN_0],
                                                                   pMac->ptt.forcedTxGain[PHY_TX_CHAIN_0]
                                                           )
               )
            {
                return (FAILURE);
            }
        }
        else if (stopTx == eANI_BOOLEAN_TRUE)
        {
            //not a waveform, leave the transmitters in automatic mode
            if ((pMac->ptt.frameGenEnabled) && (pMac->ptt.phyDbgFrameGen))
            {
                //stop/restart frame gen
                pttStartStopTxPacketGen(pMac, eANI_BOOLEAN_OFF);

                if (eHAL_STATUS_SUCCESS != asicAGCReset(pMac))
                {
                    return (FAILURE);
                }
            }
            pMac->ptt.frameGenEnabled = eANI_BOOLEAN_FALSE; //stop frame generation
        }
        else if (frameGenEnabled && (pMac->ptt.phyDbgFrameGen))
        {
            //stop frame gen - restart after changing chain selections
            pttStartStopTxPacketGen(pMac, eANI_BOOLEAN_ON);
        }
    }

    return (SUCCESS);   //could be an invalid chain selection
}



//Tx Waveform Gen Service
eQWPttStatus pttSetWaveform(tpAniSirGlobal pMac, tWaveformSample *waveform, tANI_U16 numSamples, tANI_BOOLEAN clk80)
{
    if ((numSamples <= 1) || (numSamples > MAX_TEST_WAVEFORM_SAMPLES))
    {
        return (FAILURE);
    }

    if (asicSetupTestWaveform(pMac, waveform, numSamples, (tANI_BOOLEAN)clk80) == eHAL_STATUS_SUCCESS)
    {
       pMac->ptt.wfmStored = eANI_BOOLEAN_TRUE;
       pMac->ptt.numWfmSamples = numSamples;
       return (SUCCESS);
    }
    else
    {
       pMac->ptt.wfmStored = eANI_BOOLEAN_FALSE;
       pMac->ptt.numWfmSamples = 0;
       return (FAILURE);
    }

}


eQWPttStatus pttSetRxWaveformGain(tpAniSirGlobal pMac, ePhyRxChains rxChain, tANI_U8 gain)
{
    asicOverrideAGCRxChainGain(pMac, rxChain, gain);

    return (SUCCESS);
}


eQWPttStatus pttSetTxWaveformGain(tpAniSirGlobal pMac, ePhyTxChains txChain, tANI_U8 gain)
{
    eTxCoarseGain coarse = (eTxCoarseGain)(READ_MASKED_BITS(gain, COARSE_GAIN_MASK, COARSE_GAIN_OFFSET));
    eTxFineGain fine = (eTxFineGain)(READ_MASKED_BITS(gain, FINE_GAIN_MASK, FINE_GAIN_OFFSET));

    if ((tANI_U32 )MAX_TPC_COARSE_TXPWR < coarse)
    {
        return (FAILURE);
    }

    if ((txChain == PHY_ALL_TX_CHAINS) || (txChain == PHY_MAX_TX_CHAINS))
    {
        pMac->ptt.forcedTxGain[PHY_TX_CHAIN_0].coarsePwr = coarse;
        pMac->ptt.forcedTxGain[PHY_TX_CHAIN_0].finePwr = fine;
    }
    else if ((txChain == PHY_TX_CHAIN_0))
    {
        pMac->ptt.forcedTxGain[txChain].coarsePwr = coarse;
        pMac->ptt.forcedTxGain[txChain].finePwr = fine;
    }
    else
    {
        return (FAILURE);
    }

    //waveform requires that the override gain register is used


    asicTPCPowerOverride(pMac,
                            pMac->ptt.forcedTxGain[PHY_TX_CHAIN_0],
                            pMac->ptt.forcedTxGain[PHY_TX_CHAIN_0],
                            pMac->ptt.forcedTxGain[PHY_TX_CHAIN_0],
                            pMac->ptt.forcedTxGain[PHY_TX_CHAIN_0]
                        );

    return (SUCCESS);
}



eQWPttStatus pttGetWaveformPowerAdc(tpAniSirGlobal pMac, sTxChainsPowerAdcReadings *txPowerAdc)
{
    tANI_U8 txPwr;

    if(asicGetTxPowerMeasurement(pMac, PHY_TX_CHAIN_0, &txPwr) != eHAL_STATUS_SUCCESS) { return FAILURE; }

    txPwr >>= 1;

    txPowerAdc->txPowerAdc[PHY_TX_CHAIN_0] = txPwr;

    return (SUCCESS);
}


eQWPttStatus pttStartWaveform(tpAniSirGlobal pMac)
{
#ifndef VERIFY_HALPHY_SIMV_MODEL
    if (pMac->ptt.frameGenEnabled  == eANI_BOOLEAN_TRUE)
    {
        //can't allow starting  waveform while packet generation is going - it will cause phyDbg to hang.
        return (FAILURE);
    }

    if (pMac->ptt.wfmStored == eANI_BOOLEAN_TRUE)
    {
        if (asicStartTestWaveform(pMac, WAVE_CONTINUOUS, 0, pMac->ptt.numWfmSamples - 1) == eHAL_STATUS_SUCCESS)
        {
            //pMac->ptt.wfmEnabled = eANI_BOOLEAN_TRUE;   //must preceed enabling the chains

            //make sure the chain selections are still accurate by resetting this again
            //return (pttEnableChains(pMac, pMac->hphy.phy.activeChains));
            //set the waveform gain again after the change in enabled tx chains
            if (eHAL_STATUS_SUCCESS != asicTPCPowerOverride(pMac,  pMac->ptt.forcedTxGain[PHY_TX_CHAIN_0],
                                                                   pMac->ptt.forcedTxGain[PHY_TX_CHAIN_0],
                                                                   pMac->ptt.forcedTxGain[PHY_TX_CHAIN_0],
                                                                   pMac->ptt.forcedTxGain[PHY_TX_CHAIN_0]
                                                           )
               )
            {
                return (FAILURE);
            }

            return (SUCCESS);
        }
        else
        {
            pMac->ptt.wfmEnabled = eANI_BOOLEAN_FALSE;

            return (FAILURE);
        }
    }
#endif

    return (FAILURE);
}


eQWPttStatus pttStartWaveformPlayback(tpAniSirGlobal pMac, tANI_U32 startIndex, tANI_U32 numSamples)
{
#ifdef VERIFY_HALPHY_SIMV_MODEL
     if (startIndex > 1023) startIndex = 1023;
     if ((numSamples + startIndex) > 1024) numSamples = 1024 - startIndex;

     asicStartTestWaveform(WAVE_CONTINUOUS, startIndex, startIndex + numSamples - 1);
     return SUCCESS;
#else
     return FAILURE;
#endif
}


eQWPttStatus pttStopWaveform(tpAniSirGlobal pMac)
{
#ifdef VERIFY_HALPHY_SIMV_MODEL
    asicStopTestWaveform();

    return (SUCCESS);
#else
    if (asicStopTestWaveform(pMac) == eHAL_STATUS_SUCCESS)
    {
        pMac->ptt.wfmEnabled = eANI_BOOLEAN_FALSE;   //must preceed enabling the chains

        //reset the chains so they are not setup for waveform
        return (pttEnableChains(pMac, pMac->hphy.phy.activeChains));
    }
    else
    {
        return (FAILURE);
    }
#endif
}




eQWPttStatus pttCloseTpcLoop(tpAniSirGlobal pMac, tANI_BOOLEAN tpcClose)
{
    eHalStatus retVal;

    char onOffStr[2][4] =
    {
        "OFF",
        "ON"
    };

    if (tpcClose > 1)
    {
        return(FAILURE);
    }

    phyLog(pMac, LOGE, "pttCloseTpcLoop(): %s", &onOffStr[tpcClose][0]);

    pMac->hphy.phy.test.testTpcClosedLoop = tpcClose;
    if (asicTPCAutomatic(pMac) == eHAL_STATUS_FAILURE)
    {
        return (FAILURE);
    }

    pMac->hphy.phy.test.testTxGainIndexSource = RATE_POWER_NON_LIMITED;
    {
        tANI_U32 reg;

        GET_PHY_REG(pMac->hHdd, QWLAN_TPC_TXPWR_ENABLE_REG, &reg);
    }
    return (SUCCESS);
}



    //open loop service
eQWPttStatus pttSetPacketTxGainTable(tpAniSirGlobal pMac, ePhyTxChains txChain, tANI_U8 minIndex, tANI_U8 maxIndex, tANI_U8 *gainTable)
{
    tANI_U8 i;

    if ((txChain != PHY_TX_CHAIN_0) ||
        (maxIndex >= TPC_MEM_GAIN_LUT_DEPTH)
       )
    {
        return(FAILURE);
    }

    for (i = minIndex; i <= maxIndex; i++)
    {
        eTxCoarseGain coarse = READ_MASKED_BITS(*gainTable, COARSE_GAIN_MASK, COARSE_GAIN_OFFSET);
        eTxFineGain fine = (eTxFineGain)READ_MASKED_BITS(*gainTable, FINE_GAIN_MASK, FINE_GAIN_OFFSET);

        if ((tANI_U32 )MAX_TPC_COARSE_TXPWR < coarse)
        {
            return (FAILURE);
        }

        pMac->ptt.tpcGainLut[txChain][i].coarsePwr = coarse;
        pMac->ptt.tpcGainLut[txChain][i].finePwr = fine;
        gainTable++;
    }


    asicLoadTPCGainLUT(pMac, txChain, &(pMac->ptt.tpcGainLut[txChain][0]));


    return (SUCCESS);
}

eQWPttStatus pttGetPacketTxGainTable(tpAniSirGlobal pMac, ePhyTxChains txChain, tANI_U8 *gainTable)
{
    tANI_U32 i;

    if (txChain > 1)
    {
        return(FAILURE);
    }

    for (i = 0; i < TPC_MEM_GAIN_LUT_DEPTH; i++)
    {
        if (asicGetTxGainAtIndex(pMac, txChain, (tANI_U8 )i, (tTxGainCombo *)gainTable) != eHAL_STATUS_SUCCESS)
        {
            return (FAILURE);
        }
        gainTable++;
    }

    return (SUCCESS);
}

eQWPttStatus pttSetPacketTxGainIndex(tpAniSirGlobal pMac, tANI_U8 index)
{
    tANI_BOOLEAN frameGenEnabled = pMac->ptt.frameGenEnabled;

    if (index >= TPC_MEM_GAIN_LUT_DEPTH)
    {
        return(FAILURE);
    }

    if (frameGenEnabled && (pMac->ptt.phyDbgFrameGen))
    {
        //stop frame gen - restart after changing chain selections
        pttStartStopTxPacketGen(pMac, eANI_BOOLEAN_OFF);
    }


    //this serves as either the open loop gain or the closed loop desired power Lut match
    pMac->hphy.phy.test.testForcedTxGainIndex = index;
    pMac->hphy.phy.test.testTxGainIndexSource = FORCE_POWER_TEMPLATE_INDEX;


    if ((frameGenEnabled  == eANI_BOOLEAN_TRUE) && (pMac->ptt.phyDbgFrameGen == eANI_BOOLEAN_TRUE))
    {
        //restart frame gen
        pttStartStopTxPacketGen(pMac, eANI_BOOLEAN_ON);
    }
    return (SUCCESS);
}


eQWPttStatus pttForcePacketTxGain(tpAniSirGlobal pMac, ePhyTxChains txChain, tANI_U8 gain)
{

    eTxCoarseGain coarse = READ_MASKED_BITS(gain, COARSE_GAIN_MASK, COARSE_GAIN_OFFSET);
    eTxFineGain fine = (eTxFineGain)READ_MASKED_BITS(gain, FINE_GAIN_MASK, FINE_GAIN_OFFSET);


    //for packet transmission, fill the tx gain tables with the force gain
    //this is independent of setting the gain index, which will become irrelevant for open-loop
    // For closed-loop, these values will be adjusted by the TPC hardware and so really doesn't make sense
    // except to initialize the gain tables to something close to the current calibration
    if ((txChain == PHY_TX_CHAIN_0))
    {
        tANI_U32 i;

        for (i = 0; i < TPC_MEM_GAIN_LUT_DEPTH; i++)
        {
            pMac->ptt.tpcGainLut[txChain][i].coarsePwr = coarse;
            pMac->ptt.tpcGainLut[txChain][i].finePwr = fine;
        }
        asicLoadTPCGainLUT(pMac, txChain, &(pMac->ptt.tpcGainLut[txChain][0]));
    }
    else
    {
        return (FAILURE);
    }

    //since we are forcing the txGain, make sure we choose the gainIdx as 0.
    pMac->hphy.phy.test.testForcedTxGainIndex = 0;
    pMac->hphy.phy.test.testTxGainIndexSource = FORCE_POWER_TEMPLATE_INDEX;

    return (SUCCESS);
}


eQWPttStatus pttUpdateTpcSplitLut(tpAniSirGlobal pMac, ePhyTxPwrRange pwrRange, tANI_U32 splitIdx)
{
    tANI_U8 tpcIdx = TPC_LUT_SPLIT_IDX;

    switch (pwrRange)
    {
        case PHY_TX_POWER_RANGE_R2P:
            {
                tRateGroupPwr *pwrOptimum;
                if(halGetNvTableLoc(pMac, NV_TABLE_RATE_POWER_SETTINGS, (uNvTables **)&pwrOptimum) == eHAL_STATUS_SUCCESS)
                {
                    t2Decimal _6Mbps_pwr = pwrOptimum[0][HAL_PHY_RATE_11A_6_MBPS].reported;
                    tpcIdx = (tANI_U8)((2 * _6Mbps_pwr)/100 - 17); //8.5dBm:idx0, 24dBm:idx31

                    if(tpcIdx > (TPC_MEM_GAIN_LUT_DEPTH - 1))
                    {
                        tpcIdx = TPC_LUT_SPLIT_IDX;
                    }
                }
            }
            break;

        case PHY_TX_POWER_RANGE_11B:
            tpcIdx = 0;
            break;

        case PHY_TX_POWER_RANGE_OFDM:
            tpcIdx = TPC_MEM_GAIN_LUT_DEPTH - 1;
            break;

        case PHY_TX_POWER_RANGE_FIXED_POINT:
            if(splitIdx < TPC_MEM_GAIN_LUT_DEPTH)
            {
                tpcIdx = (tANI_U8)splitIdx;
            }
            else
            {
                return (FAILURE);
            }
            break;

        default:
            return (FAILURE);
            break;
    }

    phyTxPowerSplitLutUpdate(pMac, tpcIdx);

    return (SUCCESS);
}

    //closed loop(CLPC) service
eQWPttStatus pttSetPwrIndexSource(tpAniSirGlobal pMac, ePowerTempIndexSource indexSource)
{
    eQWPttStatus retVal;
    tANI_BOOLEAN frameGenEnabled = pMac->ptt.frameGenEnabled;

    if ((indexSource < FORCE_POWER_TEMPLATE_INDEX) || (indexSource > RATE_POWER_NON_LIMITED))
    {
        return(FAILURE);
    }

    //txGainIndexSource is used in halPhyGetPowerForRate to decide what power template index to return
    if (pMac->hphy.phy.test.testTpcClosedLoop == eANI_BOOLEAN_TRUE)
    {
        switch (indexSource)
        {
            case FORCE_POWER_TEMPLATE_INDEX:
                pMac->hphy.phy.test.testTxGainIndexSource = indexSource;
                break;

            case FIXED_POWER_DBM:
                pMac->hphy.phy.test.testTxGainIndexSource = indexSource;
                break;

            case REGULATORY_POWER_LIMITS:
                pMac->hphy.phy.test.testTxGainIndexSource = indexSource;
                break;

            case RATE_POWER_NON_LIMITED:
                pMac->hphy.phy.test.testTxGainIndexSource = indexSource;
                break;

            default:
                return (FAILURE);
                break;
        }
    }

    //stop/restart frame gen
    retVal = pttStartStopTxPacketGen(pMac, eANI_BOOLEAN_OFF);
    if (retVal == SUCCESS)
    {
        if ((frameGenEnabled  == eANI_BOOLEAN_TRUE) && (pMac->ptt.phyDbgFrameGen == eANI_BOOLEAN_TRUE))
        {
            retVal = pttStartStopTxPacketGen(pMac, eANI_BOOLEAN_ON);
        }
    }

    return (SUCCESS);
}


eQWPttStatus pttSetTxPower(tpAniSirGlobal pMac, t2Decimal dbmPwr)
{
    eQWPttStatus retVal;
    tANI_BOOLEAN frameGenEnabled = pMac->ptt.frameGenEnabled;

    if ((dbmPwr > 5000) || (dbmPwr < -5000))
    {
        return (FAILURE);
    }

    phyLog(pMac, LOGE, "pttSetTxPower to %d.%d dBm\n", (dbmPwr / 100), (dbmPwr % 100));

    pMac->hphy.phy.test.testForcedTxPower = dbmPwr;
    pMac->hphy.phy.test.testTxGainIndexSource = FIXED_POWER_DBM;

    //stop/restart frame gen
    retVal = pttStartStopTxPacketGen(pMac, eANI_BOOLEAN_OFF);
    if (retVal == SUCCESS)
    {
        if ((frameGenEnabled  == eANI_BOOLEAN_TRUE) && (pMac->ptt.phyDbgFrameGen == eANI_BOOLEAN_TRUE))
        {
            retVal = pttStartStopTxPacketGen(pMac, eANI_BOOLEAN_ON);
        }
    }

    return (retVal);
}



eQWPttStatus pttGetTxPowerReport(tpAniSirGlobal pMac, tTxPowerReport *pwrReport)
{
    tANI_U32 adc0;
    //tPwrTemplateRange lutRangeIndexed;

    // if (pMac->ptt.wfmEnabled)
    // {
    //     //Waveform power is a direct measurement of the continuous waveform
    //     if(asicGetTxPowerMeasurement(pMac, PHY_TX_CHAIN_0, (tPowerDet *)&adc0) != eHAL_STATUS_SUCCESS) { return FAILURE; }
    // }
    // else
        if (((pMac->ptt.frameGenEnabled) && (pMac->ptt.phyDbgFrameGen)) || (pMac->ptt.wfmEnabled))
    {
        tANI_U32 sensed0;
        tANI_U32 pdAdcOffset;

#if 0//def ANI_PHY_DEBUG
        //this is to verify that the RF is setup to deliver correct power readings at SENSED_PWR0
        {
            tANI_U32 reg;

            palReadRegister(pMac->hHdd, QWLAN_RFAPB_MODE_SEL2_REG, &reg);
            assert(reg & QWLAN_RFAPB_MODE_SEL2_EN_RTX_BIAS_MASK);

            palReadRegister(pMac->hHdd, QWLAN_RFAPB_HDET_DCOC_REG, &reg);
            assert(reg == 0xC000);

            palReadRegister(pMac->hHdd, QWLAN_RFAPB_MODE_SEL3_REG, &reg);
            assert(reg == 0x0013);

            palReadRegister(pMac->hHdd, QWLAN_RFAPB_HDET_TEST_REG, &reg);
            assert((reg & QWLAN_RFAPB_HDET_TEST_HDET_GPO_SEL_1_0_MASK) == 1);
            assert(reg & QWLAN_RFAPB_HDET_TEST_HDET_GPO_EN_MASK);

            palReadRegister(pMac->hHdd, QWLAN_RFAPB_HDET_CTL_REG, &reg);
            assert(reg == 0x1567);

            palReadRegister(pMac->hHdd, QWLAN_RFAPB_HKADC_CFG_REG, &reg);
            assert((reg & QWLAN_RFAPB_HKADC_CFG_HKADC_INPUT_MASK) == 9);
        }
#endif
        //packet power is retrieved from the preamble sync'd sample in SENSED_POWER
        palReadRegister(pMac->hHdd, QWLAN_TPC_SENSED_PWR0_REG, &sensed0);
        phyLog(pMac, LOGE, "sensed_pwr0 = 0x%X\n", (unsigned int)sensed0);
        palReadRegister(pMac->hHdd, QWLAN_TPC_PDADC_OFFSET_REG, &pdAdcOffset);
        phyLog(pMac, LOGE, "pdAdc offset = 0x%X\n", (unsigned int)pdAdcOffset);


        adc0 = ((sensed0 & QWLAN_TPC_SENSED_PWR0_VALUE_MASK) >> QWLAN_TPC_SENSED_PWR0_VALUE_OFFSET);

        //return rawAdc offsetted by Hdet dco
        pwrReport->txChains[PHY_TX_CHAIN_0].rawAdc = (tPowerAdc)(adc0);// - pMac->hphy.hdetResidualDCO);

        adc0 -= ((pdAdcOffset & QWLAN_TPC_PDADC_OFFSET_VALUE_MASK) >> QWLAN_TPC_PDADC_OFFSET_VALUE_OFFSET);
    }
    else
    {
        phyLog(pMac, LOGE, "ERROR: Tx power report only available if frames or a waveform is transmitted\n");
        memset(pwrReport, 0, sizeof(tTxPowerReport));
        return FAILURE;
    }

    adc0 >>= 3;

    pwrReport->channelId = pMac->hphy.phy.test.testChannelId;
    pwrReport->cbState = pMac->hphy.phy.test.testCbState;
    pwrReport->rate = pMac->ptt.frameGenParams.rate;
    pwrReport->pwrTemplateIndex = pMac->hphy.phy.test.testLastPwrIndex;

    pwrReport->txChains[PHY_TX_CHAIN_0].adc = (tPowerDetect)adc0;
    asicGetTxGainAtIndex(pMac, PHY_TX_CHAIN_0, pwrReport->pwrTemplateIndex, &pwrReport->txChains[PHY_TX_CHAIN_0].gain);

/*
    if (pMac->hphy.phyTPC.pwrCfgPresent == eANI_BOOLEAN_TRUE)
    {
        lutRangeIndexed = phyGetTxPowerRangeForTempIndex(pMac, PHY_TX_CHAIN_0, pwrReport->pwrTemplateIndex);
        pwrReport->txChains[PHY_TX_CHAIN_0].indexMinMatch.lut = lutRangeIndexed.min;
        pwrReport->txChains[PHY_TX_CHAIN_0].indexMinMatch.abs.reported = phyGetAbsTxPowerForLutValue(pMac, PHY_TX_CHAIN_0, lutRangeIndexed.min);
        pwrReport->txChains[PHY_TX_CHAIN_0].indexMaxMatch.lut = lutRangeIndexed.max;
        pwrReport->txChains[PHY_TX_CHAIN_0].indexMaxMatch.abs.reported = phyGetAbsTxPowerForLutValue(pMac, PHY_TX_CHAIN_0, lutRangeIndexed.max);

        pwrReport->txChains[PHY_TX_CHAIN_0].output.lut = (tPowerDetect)pMac->hphy.phyTPC.curTpcPwrLUT[PHY_TX_CHAIN_0][adc0];
        pwrReport->txChains[PHY_TX_CHAIN_0].output.abs.reported = phyGetAbsTxPowerForLutValue(pMac, PHY_TX_CHAIN_0, pwrReport->txChains[PHY_TX_CHAIN_0].output.lut);
    }
    else
    {
        phyLog(pMac, LOGE, "NO TX POWER CONFIG - TxPowerReport inaccurate\n");
        pwrReport->txChains[PHY_TX_CHAIN_0].indexMinMatch.lut = 0;
        pwrReport->txChains[PHY_TX_CHAIN_0].indexMinMatch.abs.reported = 0;
        pwrReport->txChains[PHY_TX_CHAIN_0].indexMaxMatch.lut = 0;
        pwrReport->txChains[PHY_TX_CHAIN_0].indexMaxMatch.abs.reported = 0;
        pwrReport->txChains[PHY_TX_CHAIN_0].output.abs.reported = 0;
    }
*/
    return SUCCESS;
}





/*
#define MIN_POINT (0)
#define MAX_POINT (MAX_TPC_CAL_POINTS - 1)
static eQWPttStatus NormalizeTpcChainData(tpAniSirGlobal pMac, tTpcCalPoint *chainData, tANI_U16 numTpcCalPoints, tTpcCaldPowerPoint tpcChainCfg[MAX_TPC_CAL_POINTS])
{
    tANI_U16 calPoint;

    //requiring at least 3 cal points per chain
    if ((numTpcCalPoints != MAX_TPC_CAL_POINTS))
    {
        return(FAILURE);
    }

    //if numTpcCalPoints < MAX_TPC_CAL_POINTS, fill out rest of chainData with last point of data
    for (calPoint = numTpcCalPoints; calPoint < MAX_TPC_CAL_POINTS; calPoint++)
    {
        memcpy((void *)&(chainData[calPoint]), (void *)&(chainData[numTpcCalPoints - 1]), sizeof(tTpcCalPoint));
    }


    //now chainData should contain MAX_TPC_CAL_POINTS
    for (calPoint = 0; calPoint < MAX_TPC_CAL_POINTS; calPoint++)
    {
        tANI_S32 x = chainData[calPoint].absPowerMeasured.reported;

        if (x < (t2Decimal)(MIN_PWR_LUT_DBM_2DEC_PLACES))
        {
            phyLog(pMac, LOGE, "NormalizeTpcChainData(): reported pwr %d dBm < min pwr %d dbm, mapping pwr adc code %d to 0\n",
                                    x / 100, (MIN_PWR_LUT_DBM_2DEC_PLACES / 100), chainData[calPoint].pwrDetAdc );

            // The ADC reading for a power measurement less than MIN_PWR_LUT_DBM is mapped to zero
            //SET_FULL_PRECISION_TPC_LUT_VALUE(0, tpcChainCfg[calPoint]);
            tpcChainCfg[calPoint].adjustedPwrDet = 0;
        }
        else if (x > (t2Decimal)(MAX_PWR_LUT_DBM_2DEC_PLACES))
        {
            phyLog(pMac, LOGE, "NormalizeTpcChainData(): reported pwr %d dBm > max pwr %d dbm, mapping pwr adc code %d to %d\n",
                                    x / 100, MAX_PWR_LUT_DBM_2DEC_PLACES / 100, chainData[calPoint].pwrDetAdc ,
                                    ((TPC_MEM_POWER_LUT_DEPTH) -1));
            // The ADC reading for a power measurement greater than MAX_PWR_LUT_DBM is clipped
            //SET_FULL_PRECISION_TPC_LUT_VALUE(((TPC_MEM_POWER_LUT_DEPTH * EXTRA_TPC_LUT_MULT) -1), tpcChainCfg[calPoint]);
            tpcChainCfg[calPoint].adjustedPwrDet = TPC_MEM_POWER_LUT_DEPTH - 1;

        }
        else
        {
            tANI_S32 x1 = (t2Decimal)(MIN_PWR_LUT_DBM_2DEC_PLACES);
            tANI_S32 y1 = 0;
            tANI_S32 x2 = (t2Decimal)(MAX_PWR_LUT_DBM_2DEC_PLACES);
            tANI_S32 y2 = ((TPC_MEM_POWER_LUT_DEPTH) -1);
            tTpcLutValue lutValue;

            // this simply scales the power measurements to the necessary range
            // since x1 ( = 8(00) ) <= x <= x2 ( =24(00) ) and y1 = 0, y2 = 127
            lutValue = (tTpcLutValue)InterpolateBetweenPoints(x1, y1, x2, y2, x);
            //SET_FULL_PRECISION_TPC_LUT_VALUE(lutValue, tpcChainCfg[calPoint]);
            tpcChainCfg[calPoint].adjustedPwrDet = (tPowerDetect)lutValue;

            phyLog(pMac, LOGE, "NormalizeTpcChannelData(): measured pwr %d dbm, mapping pwr adc code %d to %d\n",
                                    (t2Decimal)(MIN_PWR_LUT_DBM_2DEC_PLACES), chainData[calPoint].pwrDetAdc,
                                    tpcChainCfg[calPoint].adjustedPwrDet );
        }

        //check for non-monotonic ADC or power
        if (calPoint > 0)
        {
            if ((chainData[calPoint].pwrDetAdc < chainData[calPoint - 1].pwrDetAdc) ||
                (x < chainData[calPoint - 1].absPowerMeasured.reported)
               )
            {
                //non-monotonic ADC reading
                return (FAILURE);
            }
        }

        //only monotonic power/ADC readings allowed
        // copy this straight across
        tpcChainCfg[calPoint].pwrDetAdc = chainData[calPoint].pwrDetAdc;
    }

    return (SUCCESS);
}



eQWPttStatus pttSaveTxPwrFreqTable(tpAniSirGlobal pMac, tANI_U8 numTpcCalFreqs, const tTpcFreqData *table)
{

    eQWPttStatus retVal = SUCCESS;   //init to fail
    tANI_U8 highestTxChain = PHY_MAX_TX_CHAINS; //(pMac->hphy.phy.cfgChains == PHY_CHAIN_SEL_R0R1_T0_ON ? 1 : 2);


    //if ((table[0].freq != START_TPC_CHANNEL) || (table[1].freq != END_TPC_CHANNEL) || (numTpcCalFreqs != MAX_TPC_CHANNELS))
    {
        phyLog(pMac, LOGE, "pttSaveTxPwrFreqTable called with wrong input params\n");
        return (FAILURE);
    }


    //TODO: Need to see if the proper tables are in NV memory ((pMac->hphy.phy.pwrOptimal != NULL) && (pMac->hphy.phy.regDomainInfo != NULL)); //should be initialized

    if ((numTpcCalFreqs > MAX_TPC_CHANNELS) || (table == NULL))
    {
        return (FAILURE);
    }


    if (numTpcCalFreqs == 2)
    {
        //Process input table into set of calibration points to store in NV
        //we assume that the given table is ordered in ascending frequencies and ascending calpoints
        //the table can contain 2.4GHz only, 5GHz only, or both together, we'll sort them out.
        tTpcConfig pwrSampled[MAX_TPC_CHANNELS];
        tANI_U8 chanIndex;

        memset(&pwrSampled, 0, sizeof(tTpcConfig) * MAX_TPC_CHANNELS);

        //this loop normalizes each channel's chain data and then puts into either aPwrSampled or bPwrSampled
        for (chanIndex = 0; chanIndex < numTpcCalFreqs; chanIndex++)
        {
            tTpcConfig tpcFreqCfg;
            tANI_U32 txChain;

            tpcFreqCfg.freq = table[chanIndex].freq;

            //all power data will be scaled to this range
            tpcFreqCfg.absPower.min = MIN_PWR_LUT_DBM_2DEC_PLACES;
            tpcFreqCfg.absPower.max = MAX_PWR_LUT_DBM_2DEC_PLACES;

            //interpolate the data for this frequency in order to produce the correct ADC vs Pwr Lut Values
            for (txChain = 0; txChain < highestTxChain; txChain++)
            {
                assert(table[chanIndex].empirical[txChain].numTpcCalPoints == MAX_TPC_CAL_POINTS);
                //This function will use the input date to interpolate MAX_TPC_CAL_POINTS points into tpcFreqCfg
                if (NormalizeTpcChainData(pMac,
                                          (tTpcCalPoint *)&(table[chanIndex].empirical[txChain].chain[0]),
                                          table[chanIndex].empirical[txChain].numTpcCalPoints,
                                          (tTpcCaldPowerPoint *)&(tpcFreqCfg.empirical[txChain][0])
                                         )
                      == FAILURE
                   )
                {
                    return (FAILURE);
                }
            }

            //add the interpolated points to the NV table to be stored
            memcpy(&(pwrSampled[chanIndex]), &tpcFreqCfg, sizeof(tTpcConfig));
        }

        {
            if (eHAL_STATUS_SUCCESS == halWriteNvTable(pMac, NV_TABLE_TPC_CONFIG, (uNvTables *)pwrSampled))
            {
                phyLog(pMac, LOGE, "Set NV_TABLE_TPC_CONFIG table ... done.\n\n");
            }
            else
            {
               phyLog(pMac, LOGE, "Set NV_TABLE_TPC_CONFIG table ... FAILED.\n\n");
               return (FAILURE);
            }
        }

        //now reconfigure TPC to make changes take effect immediately
        // if (ConfigureTpcFromNv(pMac) != eHAL_STATUS_SUCCESS)
        // {
        //     retVal = FAILURE;
        // }
    }


    return (retVal);

}
*/


eQWPttStatus pttSetPowerLut(tpAniSirGlobal pMac, ePhyTxChains txChain, tANI_U8 minIndex, tANI_U8 maxIndex, tANI_U8 *powerLut)
{
    tANI_U32 i;

    if ((txChain != PHY_TX_CHAIN_0) ||
        (maxIndex >= TPC_MEM_POWER_LUT_DEPTH)
       )
    {
        return(FAILURE);
    }

    for (i = minIndex; i <= maxIndex; i++)
    {
        pMac->ptt.tpcPowerLut[txChain][i] = *powerLut;
        powerLut++;
    }
    asicLoadTPCPowerLUT(pMac, txChain, &(pMac->ptt.tpcPowerLut[txChain][0]));

    return (SUCCESS);
}


eQWPttStatus pttGetPowerLut(tpAniSirGlobal pMac, ePhyTxChains txChain, tANI_U8 *powerLut)
{
    tANI_U32 i;

    if (txChain > 1)
    {
        return(FAILURE);
    }

    for (i = 0; i < TPC_MEM_POWER_LUT_DEPTH; i++)
    {
        if (asicGetTxPowerLutAtIndex(pMac, txChain, (tANI_U8 )i, (tPowerDetect *)powerLut) != eHAL_STATUS_SUCCESS)
        {
            return (FAILURE);
        }
        powerLut++;
    }

    return (SUCCESS);
}





//Rx Gain Service
eQWPttStatus pttDisableAgcTables(tpAniSirGlobal pMac, sRxChainsAgcDisable gains)
{

    if (gains.rx[PHY_RX_CHAIN_0] >= RF_AGC_GAIN_LUT_DEPTH)
    {
        return (FAILURE);
    }

    asicOverrideAGCRxChainGain(pMac, PHY_RX_CHAIN_0, gains.rx[PHY_RX_CHAIN_0]);

    pMac->ptt.agcEnables.rx[PHY_RX_CHAIN_0] = eANI_BOOLEAN_FALSE;

    return (SUCCESS);
}


eQWPttStatus pttEnableAgcTables(tpAniSirGlobal pMac, sRxChainsAgcEnable enables)
{
    if (enables.rx[PHY_RX_CHAIN_0] > 1)
    {
        return(FAILURE);
    }


    if (enables.rx[PHY_RX_CHAIN_0] == eANI_BOOLEAN_TRUE)
    {
        asicCeaseOverrideAGCRxChainGain(pMac, PHY_RX_CHAIN_0);

        //load current AGC Gain Tables
        //!Loading the gain tables for Gen6 is not necessary as they are initialized using a const table from the RF code.
        //asicSetAGCGainLut(pMac, PHY_RX_CHAIN_0, 0, (NUM_GAIN_LUT_SETTINGS - 1), &pttAgcGainLUT[PHY_RX_CHAIN_0][0]);
    }
    pMac->ptt.agcEnables.rx[PHY_RX_CHAIN_0] = enables.rx[PHY_RX_CHAIN_0];

    return (SUCCESS);
}

#define RSSI_TO_DBM_OFFSET     -100

void pttCollectAdcRssiStats(tpAniSirGlobal pMac)
{
    tANI_U32 rssi0, rssi0Stats = 0;
    tANI_U8 counter, validCounterRssi0 = 0;

    /*check the valid bit '8th bit' before reading values
     which should be set to 1 for valid RSSI reads*/

    for (counter = 0; counter < 10; counter++ )
    {

        palReadRegister(pMac->hHdd, QWLAN_PMI_LAST_STATS0_REG, &rssi0);

        /*reading rssi0 value*/
        if(rssi0)
        {
            rssi0Stats += (tANI_U16)(rssi0);
            validCounterRssi0++;
        }
        else
        {
            rssi0Stats += 0;
        }
     }

    /*average the value over valid reads*/
    rssi0 = ((rssi0Stats == 0)?0:(rssi0Stats/validCounterRssi0));

    /*assign rssiValues to response*/
    if(rssi0 > 0)
    {
        pMac->ptt.rssi.rx[PHY_RX_CHAIN_0] = (tANI_S8)rssi0;
    }
}

void pttGetRxRssi(tpAniSirGlobal pMac, sRxChainsRssi *rssi)
{
    eRfChannels curChan = rfGetChannelIndex(pMac->hphy.phy.test.testChannelId, pMac->hphy.phy.test.testCbState);
    t2Decimal rssiOffset0;

    if(curChan == INVALID_RF_CHANNEL)
    {
        //default it to channel 1
        curChan = RF_CHAN_1;
    }

    //use the bgnpower offsets for RSSI as well. make sure you strip off last two decimal places
    {
        tANI_U32 pktMode;
        sRssiChannelOffsets *rssiChanOffsets = (sRssiChannelOffsets *)(&pMac->hphy.nvCache.tables.rssiChanOffsets[0]);

        palReadRegister(pMac->hHdd, QWLAN_AGC_DIS_MODE_REG, &pktMode);

        if(pktMode & QWLAN_AGC_DIS_MODE_DISABLE_11AG_MASK)
        {
            rssiOffset0 = 0;
        }
        else
        {
            rssiOffset0 = (rssiChanOffsets[PHY_RX_CHAIN_0].gnRssiOffset[curChan] -
                            rssiChanOffsets[PHY_RX_CHAIN_0].bRssiOffset[curChan]) / 100;
        }
    }

    rssi->rx[PHY_RX_CHAIN_0] = (tANI_S8)(pMac->ptt.rssi.rx[PHY_RX_CHAIN_0] + RSSI_TO_DBM_OFFSET + rssiOffset0);
}


void pttGetUnicastMacPktRxRssi(tpAniSirGlobal pMac, sRxChainsRssi *rssi)
{
    eRfChannels curChan = rfGetChannelIndex(pMac->hphy.phy.test.testChannelId, pMac->hphy.phy.test.testCbState);
    t2Decimal rssiOffset0;
    Qwlanfw_PhyFtmInfoType ftmInfo;

    if(curChan == INVALID_RF_CHANNEL)
    {
        //default it to channel 1
        curChan = RF_CHAN_1;
    }

    //use the bgnpower offsets for RSSI as well. make sure you strip off last two decimal places
    {
        tANI_U32 pktMode;
        sRssiChannelOffsets *rssiChanOffsets = (sRssiChannelOffsets *)(&pMac->hphy.nvCache.tables.rssiChanOffsets[0]);

        palReadRegister(pMac->hHdd, QWLAN_AGC_DIS_MODE_REG, &pktMode);

        if(pktMode & QWLAN_AGC_DIS_MODE_DISABLE_11AG_MASK)
        {
            rssiOffset0 = 0;
        }
        else
        {
            rssiOffset0 = (rssiChanOffsets[PHY_RX_CHAIN_0].gnRssiOffset[curChan] -
                            rssiChanOffsets[PHY_RX_CHAIN_0].bRssiOffset[curChan]) / 100;
        }
    }

    palReadDeviceMemory(pMac->hHdd, QWLANFW_MEM_PHY_FTM_INFO_ADDR_OFFSET, (tANI_U8*)&ftmInfo, sizeof(Qwlanfw_PhyFtmInfoType));
    rssi->rx[PHY_RX_CHAIN_0] = (tANI_S8)(ftmInfo.uRxLastRssiVal + RSSI_TO_DBM_OFFSET + rssiOffset0);

}



//Rx Frame Catcher Service
eQWPttStatus pttSetRxDisableMode(tpAniSirGlobal pMac, sRxTypesDisabled disabled)
{
    ePhyRxDisabledPktTypes setVal;

    if ((disabled.agPktsDisabled > 1) ||
        (disabled.bPktsDisabled > 1) ||
        (disabled.slrPktsDisabled > 1)
       )
    {
        return(FAILURE);
    }

    setVal = (ePhyRxDisabledPktTypes)(((tANI_U8 )disabled.agPktsDisabled |
                                       ((tANI_U8 )disabled.bPktsDisabled << 1) |
                                       ((tANI_U8 )disabled.slrPktsDisabled << 2)
                                      )
                                     );

    if (halPhySetRxPktsDisabled(pMac, setVal) != eHAL_STATUS_SUCCESS)
    {
        return (FAILURE);
    }

    return (SUCCESS);
}


eQWPttStatus pttGetRxPktCounts(tpAniSirGlobal pMac, sRxFrameCounters *counters)
{
    Qwlanfw_PhyFtmInfoType ftmInfo;
    eHalStatus retVal;
    {
        tANI_U32 reg;

        GET_PHY_REG(pMac->hHdd, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_REG, &reg);
        if ((reg & QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_PHYDBG_MASK) == 0)
        {
            phyLog(pMac, LOGE, "PHYDBG clock not on for pttGetRxPktCounts\n");
            return (FAILURE);
        }
    }

    if (eHAL_STATUS_SUCCESS != palReadRegister(pMac->hHdd, QWLAN_PHYDBG_RXPKT_CNT_REG, &counters->totalRxPackets))
    {
        return (FAILURE);
    }

    if (eHAL_STATUS_SUCCESS != palReadRegister(pMac->hHdd, QWLAN_RXP_FCS_ERR_CNT_REG, &counters->totalMacFcsErrPackets))
    {
        return (FAILURE);
    }

    {
        palReadDeviceMemory(pMac->hHdd, QWLANFW_MEM_PHY_FTM_INFO_ADDR_OFFSET, (tANI_U8*)&ftmInfo, sizeof(Qwlanfw_PhyFtmInfoType));
        counters->totalMacRxPackets = ftmInfo.uRxDataFrameCount;

    }

    return (SUCCESS);
}



eQWPttStatus pttResetRxPacketStatistics(tpAniSirGlobal pMac)
{
    Qwlanfw_PhyFtmInfoType ftmInfo;
    eHalStatus retVal;
    {
        tANI_U32 reg;

        GET_PHY_REG(pMac->hHdd, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_REG, &reg);
        if ((reg & QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_PHYDBG_MASK) == 0)
        {
            phyLog(pMac, LOGE, "PHYDBG clock not on for pttResetRxPacketStatistics\n");
            return (eHAL_STATUS_FAILURE);
        }
    }

    if (eHAL_STATUS_SUCCESS != palWriteRegister(pMac->hHdd, QWLAN_PHYDBG_RXPKT_CNT_REG, 0))
    {
        return (FAILURE);
    }

    if (eHAL_STATUS_SUCCESS != palWriteRegister(pMac->hHdd, QWLAN_RXP_CLEAR_STATS_REG, QWLAN_RXP_CLEAR_STATS_CLEAR_STATS_MASK))
    {
        return (FAILURE);
    }

    palZeroMemory(pMac->hHdd, (void*)&ftmInfo, sizeof(Qwlanfw_PhyFtmInfoType));
    palWriteDeviceMemory(pMac->hHdd, QWLANFW_MEM_PHY_FTM_INFO_ADDR_OFFSET, (tANI_U8*)&ftmInfo, sizeof(Qwlanfw_PhyFtmInfoType));

    return (SUCCESS);
}






//Rx Symbol Service
eQWPttStatus pttGrabRam(tpAniSirGlobal pMac, tANI_U32 startSample, tANI_U32 numSamples,
                                            eGrabRamSampleType sampleType, tGrabRamSample *grabRam)
{
    if ((numSamples == 0) || (numSamples > MAX_REQUESTED_GRAB_RAM_SAMPLES) || (startSample > LAST_GRAB_RAM_SAMPLE_INDEX))
    {
        return (FAILURE);
    }

        if (eHAL_STATUS_SUCCESS == asicGrabAdcSamples(pMac, startSample, numSamples, sampleType, grabRam))
        {
            return (SUCCESS);
        }
        else
        {
            return (FAILURE);
        }
}


eQWPttStatus pttRxDcoCal(tpAniSirGlobal pMac, tRxChainsDcoCorrections *calValues, tANI_U8 gain)
{
    if ((pMac->ptt.wfmEnabled == eANI_BOOLEAN_FALSE) && (pMac->ptt.frameGenEnabled== eANI_BOOLEAN_FALSE))
    {
        pMac->hphy.phy.test.testCalMode = RX_DCO_CAL_ONLY;
        if (halPhyCalUpdate(pMac) == eHAL_STATUS_SUCCESS)
        {
            pttGetRxDcoCorrect(pMac, calValues, gain);
            return (SUCCESS);
        }
        else
        {
            return (FAILURE);   //can't do this while generating a waveform
        }
    }
    else
    {
        return (FAILURE);   //can't do this while generating a waveform
    }
}

eQWPttStatus pttRxIm2Cal(tpAniSirGlobal pMac, tRxChainsIm2Corrections *calValues, tANI_U8 im2CalOnly)
{
#ifdef VERIFY_HALPHY_SIMV_MODEL
    tANI_U8 dummy;

    if (im2CalOnly)
        pMac->hphy.phy.test.testCalMode = RX_IM2_CAL_ONLY;
    else
        pMac->hphy.phy.test.testCalMode = RX_DCO_IM2_CAL;

    if (halPhyCalUpdate(pMac) == eHAL_STATUS_SUCCESS)
    {
        pttGetRxIm2Correct(pMac, calValues, dummy);
        return (SUCCESS);
    }
#endif
    return (FAILURE);
}

//Phy Calibration Service
eQWPttStatus pttRxIqCal(tpAniSirGlobal pMac, sRxChainsIQCalValues *calValues, eGainSteps gain)
{
    if ((pMac->ptt.wfmEnabled == eANI_BOOLEAN_FALSE) && (pMac->ptt.frameGenEnabled == eANI_BOOLEAN_FALSE))
    {
        pMac->hphy.phy.test.testCalMode = RX_IQ_CAL_ONLY;
        if (halPhyCalUpdate(pMac) == eHAL_STATUS_SUCCESS)
        {
            pttGetRxIqCorrect(pMac, calValues, gain);
            return (SUCCESS);
        }
        else
        {
            return (FAILURE);   //can't do this while generating a waveform
        }
    }
    else
    {
        return (FAILURE);   //can't do this while generating a waveform
    }
}


eQWPttStatus pttTxIqCal(tpAniSirGlobal pMac, sTxChainsIQCalValues *calValues, eGainSteps gain)
{
    if ((pMac->ptt.wfmEnabled == eANI_BOOLEAN_FALSE) && (pMac->ptt.frameGenEnabled == eANI_BOOLEAN_FALSE))
    {
        pMac->hphy.phy.test.testCalMode = TX_IQ_CAL_ONLY;
        if (halPhyCalUpdate(pMac) == eHAL_STATUS_SUCCESS)
        {
            pttGetTxIqCorrect(pMac, calValues, gain);
            return (SUCCESS);
        }
        else
        {
            return (FAILURE);   //can't do this while generating a waveform
        }
    }
    else
    {
        return (FAILURE);   //can't do this while generating a waveform
    }
}


eQWPttStatus pttTxCarrierSuppressCal(tpAniSirGlobal pMac, sTxChainsLoCorrections *calValues, eGainSteps gain)
{
    if ((pMac->ptt.wfmEnabled == eANI_BOOLEAN_FALSE) && (pMac->ptt.frameGenEnabled == eANI_BOOLEAN_FALSE))
    {
        pMac->hphy.phy.test.testCalMode = TX_LO_CAL_ONLY;
        if (halPhyCalUpdate(pMac) == eHAL_STATUS_SUCCESS)
        {
            pttGetTxCarrierSuppressCorrect(pMac, calValues, gain);
            return (SUCCESS);
        }
        else
        {
            return (FAILURE);   //can't do this while generating a waveform
        }
    }
    else
    {
        return (FAILURE);   //can't do this while generating a waveform
    }
}


eQWPttStatus pttExecuteInitialCals(tpAniSirGlobal pMac)
{
    if ((pMac->ptt.wfmEnabled == eANI_BOOLEAN_FALSE) && (pMac->ptt.frameGenEnabled == eANI_BOOLEAN_FALSE))
    {
        pMac->hphy.phy.test.testCalMode = ALL_CALS;
        if (halPhyCalUpdate(pMac) == eHAL_STATUS_SUCCESS)
        {
            return (SUCCESS);
        }
        else
        {
            return (FAILURE);   //can't do this while generating a waveform
        }
    }
    else
    {
        return (FAILURE);   //can't do this while generating a waveform
    }
}

eQWPttStatus pttHdetCal(tpAniSirGlobal pMac, sRfHdetCalValues *hdetCalValues)
{
    sRFCalValues    *pRfCalValues;

    halGetNvTableLoc(pMac, NV_TABLE_RF_CAL_VALUES, (uNvTables **)&pRfCalValues);

    hdetCalValues->hdetDcocCode = pRfCalValues->calData.hdet_cal_code;
    hdetCalValues->hdetDcoOffset = pRfCalValues->calData.hdet_dco;

    pMac->hphy.hdetResidualDCO = pRfCalValues->calData.hdet_dco;
#if 0
    rfHdetDCOCal(pMac, &(hdetCalValues->hdetDcocCode));
    rfHdetDCOCal(pMac, &(hdetCalValues->hdetDcocCode));
    rfGetHdetDCOffset(pMac, &(hdetCalValues->hdetDcoOffset));

    pMac->hphy.hdetResidualDCO = hdetCalValues->hdetDcoOffset;
#endif

    return SUCCESS;
}

//Phy Calibration Override Service
eQWPttStatus pttSetTxCarrierSuppressCorrect(tpAniSirGlobal pMac, sTxChainsLoCorrections calValues, eGainSteps gain)
{
    if ((tANI_U32)gain > NUM_TX_GAIN_STEPS)
    {
        phyLog(pMac, LOGE, "Incorrect Gain %d\n", gain);
        return (FAILURE);
    }

#ifdef VERIFY_HALPHY_SIMV_MODEL
    if ((eHAL_STATUS_SUCCESS == rfSetTxLoCorrect(PHY_TX_CHAIN_0, gain, calValues.txLo[PHY_TX_CHAIN_0]))
       )
#else
    if ((eHAL_STATUS_SUCCESS == rfSetTxLoCorrect(pMac, PHY_TX_CHAIN_0, gain, calValues.txLo[PHY_TX_CHAIN_0]))
       )
#endif
    {
        return (SUCCESS);
    }
    else
    {
        return (FAILURE);
    }
}


eQWPttStatus pttGetTxCarrierSuppressCorrect(tpAniSirGlobal pMac, sTxChainsLoCorrections *calValues, eGainSteps gain)
{
    if ((tANI_U32)gain > NUM_TX_GAIN_STEPS)
    {
        phyLog(pMac, LOGE, "Incorrect Gain %d\n", gain);
        return (FAILURE);
    }

#ifdef VERIFY_HALPHY_SIMV_MODEL
    if ((eHAL_STATUS_SUCCESS == rfGetTxLoCorrect(PHY_TX_CHAIN_0, gain, &calValues->txLo[PHY_TX_CHAIN_0]))
       )
#else
    if ((eHAL_STATUS_SUCCESS == rfGetTxLoCorrect(pMac, PHY_TX_CHAIN_0, gain, &calValues->txLo[PHY_TX_CHAIN_0]))
       )
#endif
    {
        return (SUCCESS);
    }
    else
    {
        return (FAILURE);
    }
}


eQWPttStatus pttSetTxIqCorrect(tpAniSirGlobal pMac, sTxChainsIQCalValues calValues, eGainSteps gain)
{
    if ((tANI_U32)gain > NUM_TX_GAIN_STEPS)
    {
        phyLog(pMac, LOGE, "Incorrect Gain %d\n", gain);
        return (FAILURE);
    }

#ifdef VERIFY_HALPHY_SIMV_MODEL
    asicTxFirSetTxPhaseCorrection(gain, PHY_TX_CHAIN_0, calValues.iq[PHY_TX_CHAIN_0]);
    return (SUCCESS);
#else
    if (eHAL_STATUS_SUCCESS == asicTxFirSetTxPhaseCorrection(pMac, gain, PHY_TX_CHAIN_0, calValues.iq[PHY_TX_CHAIN_0]))
    {
        return (SUCCESS);
    }
    else
    {
        return (FAILURE);
    }
#endif
}

eQWPttStatus pttGetTxIqCorrect(tpAniSirGlobal pMac, sTxChainsIQCalValues *calValues, eGainSteps gain)
{
    if ((tANI_U32)gain > NUM_TX_GAIN_STEPS)
    {
        phyLog(pMac, LOGE, "Incorrect Gain %d\n", gain);
        return (FAILURE);
    }

#ifdef VERIFY_HALPHY_SIMV_MODEL
    asicTxFirGetTxPhaseCorrection(gain, PHY_TX_CHAIN_0, &calValues->iq[PHY_TX_CHAIN_0]);
    return (SUCCESS);
#else

    if (eHAL_STATUS_SUCCESS == asicTxFirGetTxPhaseCorrection(pMac, gain, PHY_TX_CHAIN_0, &calValues->iq[PHY_TX_CHAIN_0]))
    {
        return (SUCCESS);
    }
    else
    {
        return (FAILURE);
    }
#endif
}


eQWPttStatus pttSetRxIqCorrect(tpAniSirGlobal pMac, sRxChainsIQCalValues calValues, eGainSteps gain)
{
    if ((tANI_U32)gain > NUM_RX_GAIN_STEPS)
    {
        phyLog(pMac, LOGE, "Incorrect Gain %d\n", gain);
        return (FAILURE);
    }
/*
    for (rxChain = PHY_RX_CHAIN_0; rxChain < PHY_MAX_RX_CHAINS; rxChain)
    {
        sIQCalValues *correct = &calValues.iq[rxChain];
        if (eHAL_STATUS_SUCCESS != asicWriteRxPhaseCorrection(pMac, gain, rxChain, *correct))
        {
            return (FAILURE);
        }
    }
*/
#ifdef VERIFY_HALPHY_SIMV_MODEL
    asicWriteRxPhaseCorrection(gain, PHY_RX_CHAIN_0, calValues.iq[PHY_RX_CHAIN_0]);
#else
    asicWriteRxPhaseCorrection(pMac, gain, PHY_RX_CHAIN_0, calValues.iq[PHY_RX_CHAIN_0]);
#endif

    return (SUCCESS);
}


eQWPttStatus pttGetRxIqCorrect(tpAniSirGlobal pMac, sRxChainsIQCalValues *calValues, eGainSteps gain)
{
    sIQCalValues correct;

    if ((tANI_U32)gain > NUM_RX_GAIN_STEPS)
    {
        phyLog(pMac, LOGE, "Incorrect Gain %d\n", gain);
        return (FAILURE);
    }
/*
    for (rxChain = PHY_RX_CHAIN_0; rxChain < PHY_MAX_RX_CHAINS; rxChain)
    {
        sIQCalValues *correct = &calValues->iq[rxChain];
        if (eHAL_STATUS_SUCCESS != asicReadRxPhaseCorrection(pMac, gain, rxChain, correct))
        {
            return (FAILURE);
        }
    }
*/

#ifdef VERIFY_HALPHY_SIMV_MODEL
    asicReadRxPhaseCorrection(gain, PHY_RX_CHAIN_0, &correct);
#else
    asicReadRxPhaseCorrection(pMac, gain, PHY_RX_CHAIN_0, &correct);
#endif
    calValues->iq[PHY_RX_CHAIN_0].center = correct.center;
    calValues->iq[PHY_RX_CHAIN_0].offCenter = correct.offCenter;
    calValues->iq[PHY_RX_CHAIN_0].imbalance = correct.imbalance;

    return (SUCCESS);
}


#define MAX_DCO_LUT_INDEX   31
eQWPttStatus pttSetRxDcoCorrect(tpAniSirGlobal pMac, tRxChainsDcoCorrections calValues, tANI_U8 gain)
{
    tANI_U8 lutIdx = 0;

    if ((tANI_U32)gain > NUM_RF_RX_GAIN_STEPS)
    {
        phyLog(pMac, LOGE, "Incorrect Gain %d\n", gain);
        return (FAILURE);
    }

/*
    for (rxChain = 0; rxChain < PHY_MAX_RX_CHAINS; rxChain)
    {
        if (eHAL_STATUS_SUCCESS != rfSetDCOffset(pMac, rxChain, gain, calValues.dco[rxChain]))
        {
            return (FAILURE);
        }
    }
*/
    for (lutIdx = 0; lutIdx <= MAX_DCO_LUT_INDEX; lutIdx++)
    {
#ifdef VERIFY_HALPHY_SIMV_MODEL
        rfSetDCOffset(PHY_RX_CHAIN_0, lutIdx, calValues.dco[PHY_RX_CHAIN_0]);
#else
        rfSetDCOffset(pMac, PHY_RX_CHAIN_0, lutIdx, calValues.dco[PHY_RX_CHAIN_0]);
#endif
    }

    return (SUCCESS);
}

eQWPttStatus pttSetRxIm2Correct(tpAniSirGlobal pMac, tRxChainsIm2Corrections calValues, tANI_U8 dummy)
{
#ifdef VERIFY_HALPHY_SIMV_MODEL
    rfSetIm2Correct(calValues.dco[PHY_RX_CHAIN_0]);
#endif

    return (SUCCESS);
}


eQWPttStatus pttGetRxDcoCorrect(tpAniSirGlobal pMac, tRxChainsDcoCorrections *calValues, tANI_U8 gain)
{
    gain = 31;  //All lutIndices have same value

    if ((tANI_U32)gain > NUM_RF_RX_GAIN_STEPS)
    {
        phyLog(pMac, LOGE, "Incorrect Gain %d\n", gain);
        return (FAILURE);
    }
/*
    for (rxChain = 0; rxChain < PHY_MAX_RX_CHAINS; rxChain)
    {
        if (eHAL_STATUS_SUCCESS != rfGetDCOffset(pMac, rxChain, gain, &calValues->dco[rxChain]))
        {
            return (FAILURE);
        }
    }
*/
#ifdef VERIFY_HALPHY_SIMV_MODEL
    rfGetDCOffset(PHY_RX_CHAIN_0, gain, &calValues->dco[PHY_RX_CHAIN_0]);
#else
    rfGetDCOffset(pMac, PHY_RX_CHAIN_0, gain, &calValues->dco[PHY_RX_CHAIN_0]);
#endif

    return (SUCCESS);
}

eQWPttStatus pttGetRxIm2Correct(tpAniSirGlobal pMac, tRxChainsIm2Corrections *calValues, tANI_U8 dummy)
{
#ifdef VERIFY_HALPHY_SIMV_MODEL
    rfGetIm2Correct(&calValues->dco[PHY_RX_CHAIN_0]);
#endif

    return (SUCCESS);
}




//Rf Calibration Service
eQWPttStatus pttGetTempAdc(tpAniSirGlobal pMac, eRfTempSensor tempSensor, tTempADCVal *tempAdc)
{
    if (eHAL_STATUS_SUCCESS == rfTakeTemp(pMac, tempSensor, 1, (tTempADCVal *)tempAdc))
    {
       return (SUCCESS);
    }
    else
    {
       return (FAILURE);
    }
}

eQWPttStatus pttStartToneGen(tpAniSirGlobal pMac, tANI_U8 lutIdx, tANI_U8 band)
{
#ifdef VERIFY_HALPHY_SIMV_MODEL
    rfGenerateRFTone(lutIdx, band);
#endif
    return (SUCCESS);
}

eQWPttStatus pttStopToneGen(tpAniSirGlobal pMac, tANI_U32 option)
{
#ifdef VERIFY_HALPHY_SIMV_MODEL
    rfStopRFTone();
#endif
    return (SUCCESS);
}

eQWPttStatus pttReadRfField(tpAniSirGlobal pMac, tANI_U32 addr, tANI_U32 mask, tANI_U32 shift, tANI_U32 *value)
{
    if (eHAL_STATUS_SUCCESS == rfReadField(pMac, addr, mask, shift, value))
    {
        return (SUCCESS);
    }
    else
    {
        return (FAILURE);
    }
}


eQWPttStatus pttWriteRfField(tpAniSirGlobal pMac, tANI_U32 addr, tANI_U32 mask, tANI_U32 shift, tANI_U32 value)
{
    if (eHAL_STATUS_SUCCESS == rfWriteField(pMac, addr, mask, shift, value))
    {
        return (SUCCESS);
    }
    else
    {
        return (FAILURE);
    }
}

eQWPttStatus pttDeepSleep(tpAniSirGlobal pMac)
{
#ifdef FIXME_VOLANS
    tANI_U32 value;

    palReadBAR4Register(pMac->hHdd, QWLAN_SIF_BAR4_WLAN_CONTROL_REG_REG, &value);
    palWriteBAR4Register(pMac->hHdd, QWLAN_SIF_BAR4_WLAN_CONTROL_REG_REG,
                                        value | QWLAN_SIF_BAR4_WLAN_CONTROL_REG_TRSW_SUPPLY_CTRL_0_MASK);
#ifdef ANI_PHY_DEBUG
    {
        tANI_U32 dumpVal;
        palReadBAR4Register(pMac->hHdd, QWLAN_SIF_BAR4_WLAN_CONTROL_REG_REG, &dumpVal);
        phyLog(pMac, LOGE, "QWLAN_SIF_BAR4_WLAN_CONTROL_REG_REG 0x%x\n", dumpVal);
    }
#endif

    palReadBAR4Register(pMac->hHdd, QWLAN_SIF_BAR4_WLAN_PWR_SAVE_CONTROL_REG_REG, &value);
    palWriteBAR4Register(pMac->hHdd, QWLAN_SIF_BAR4_WLAN_PWR_SAVE_CONTROL_REG_REG,
                                        value & ~QWLAN_SIF_BAR4_WLAN_PWR_SAVE_CONTROL_REG_TRSW_SUPPLY_CTRL_1_MASK);
#ifdef ANI_PHY_DEBUG
    {
        tANI_U32 dumpVal;
        palReadBAR4Register(pMac->hHdd, QWLAN_SIF_BAR4_WLAN_PWR_SAVE_CONTROL_REG_REG, &dumpVal);
        phyLog(pMac, LOGE, "QWLAN_SIF_BAR4_WLAN_PWR_SAVE_CONTROL_REG_REG 0x%x\n", dumpVal);
    }
#endif

//#ifdef FIXME_VOLANS
    palReadRegister(pMac->hHdd, QWLAN_PMU_LDO_CTRL_REG_REG, &value);
    palWriteRegister(pMac->hHdd, QWLAN_PMU_LDO_CTRL_REG_REG,
                                    (value | QWLAN_PMU_LDO_CTRL_REG_PMU_ANA_DEEP_SLEEP_EN_MASK));
#ifdef ANI_PHY_DEBUG
    {
        tANI_U32 dumpVal;
        palReadRegister(pMac->hHdd, QWLAN_PMU_LDO_CTRL_REG_REG, &dumpVal);
        phyLog(pMac, LOGE, "QWLAN_PMU_LDO_CTRL_REG_REG 0x%x\n", dumpVal);
    }
#endif

    palReadRegister(pMac->hHdd, QWLAN_PMU_RING_OSC_CTRL_SEL_REG_REG, &value);
    palWriteRegister(pMac->hHdd, QWLAN_PMU_RING_OSC_CTRL_SEL_REG_REG,
                                    (value |
                                     QWLAN_PMU_RING_OSC_CTRL_SEL_REG_PMU_GCU_SDIO_AUX_CLK_GATE_SEL_REG_MASK |
                                     QWLAN_PMU_RING_OSC_CTRL_SEL_REG_PMU_GCU_ROSC_CLK_GATE_SEL_REG_MASK
#ifdef FIXME_VOLANS
                                     | QWLAN_PMU_RING_OSC_CTRL_SEL_REG_RING_OSC_PWR_EN_SEL_REG_MASK
#endif
                    ));
#ifdef ANI_PHY_DEBUG
    {
        tANI_U32 dumpVal;
        palReadRegister(pMac->hHdd, QWLAN_PMU_RING_OSC_CTRL_SEL_REG_REG, &dumpVal);
        phyLog(pMac, LOGE, "QWLAN_PMU_RING_OSC_CTRL_SEL_REG_REG 0x%x\n", dumpVal);
    }
#endif

    palReadBAR4Register(pMac->hHdd, QWLAN_SIF_BAR4_WLAN_PWR_SAVE_CONTROL_REG_REG, &value);
    palWriteBAR4Register(pMac->hHdd, QWLAN_SIF_BAR4_WLAN_PWR_SAVE_CONTROL_REG_REG,
                                        (value | QWLAN_SIF_BAR4_WLAN_PWR_SAVE_CONTROL_REG_SUSPEND_WLAN_MASK));
#ifdef ANI_PHY_DEBUG
    {
        tANI_U32 dumpVal;
        palReadBAR4Register(pMac->hHdd, QWLAN_SIF_BAR4_WLAN_PWR_SAVE_CONTROL_REG_REG, &dumpVal);
        phyLog(pMac, LOGE, "QWLAN_SIF_BAR4_WLAN_PWR_SAVE_CONTROL_REG_REG 0x%x\n", dumpVal);
    }
#endif

    sirBusyWait(50000000); //wait 50 ms

    palReadBAR4Register(pMac->hHdd, QWLAN_SIF_BAR4_WLAN_CONTROL_REG_REG, &value);
    palWriteBAR4Register(pMac->hHdd, QWLAN_SIF_BAR4_WLAN_CONTROL_REG_REG,
                                        (value & ~QWLAN_SIF_BAR4_WLAN_CONTROL_REG_PMU_GCU_CLK_ROSC_G_EN_MASK));

    sirBusyWait(50000000); //wait 50 ms
#ifdef ANI_PHY_DEBUG
    {
        tANI_U32 dumpVal;
        palReadBAR4Register(pMac->hHdd, QWLAN_SIF_BAR4_WLAN_CONTROL_REG_REG, &dumpVal);
        phyLog(pMac, LOGE, "QWLAN_SIF_BAR4_WLAN_CONTROL_REG_REG 0x%x\n", dumpVal);
    }
#endif

    palReadBAR4Register(pMac->hHdd, QWLAN_SIF_BAR4_WLAN_CONTROL_REG_REG, &value);
    palWriteBAR4Register(pMac->hHdd, QWLAN_SIF_BAR4_WLAN_CONTROL_REG_REG,
                                        (value & ~QWLAN_SIF_BAR4_WLAN_CONTROL_REG_PMU_ROSC_PWR_EN_MASK));

    sirBusyWait(50000000); //wait 50 ms

#ifdef ANI_PHY_DEBUG
    {
        tANI_U32 dumpVal;
        palReadBAR4Register(pMac->hHdd, QWLAN_SIF_BAR4_WLAN_CONTROL_REG_REG, &dumpVal);
        phyLog(pMac, LOGE, "QWLAN_SIF_BAR4_WLAN_CONTROL_REG_REG 0x%x\n", dumpVal);
    }
#endif
#endif //FIXME_VOLANS
    return (SUCCESS);
}

eQWPttStatus pttSystemReset(tpAniSirGlobal pMac)
{
    //TODO: determine what to do for reset halPerformSystemReset((tHalHandle)pMac);
    return (SUCCESS);
}

#endif //ifndef WLAN_FTM_STUB


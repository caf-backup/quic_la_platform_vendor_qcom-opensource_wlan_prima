/**
 *
   Airgo Networks, Inc proprietary.
   All Rights Reserved, Copyright 2006
   This program is the confidential and proprietary product of Airgo Networks Inc.
   Any Unauthorized use, reproduction or transfer of this program is strictly prohibited.

 */

#include "sys_api.h"


#define SUCCESS     PTT_STATUS_SUCCESS
#define FAILURE     PTT_STATUS_FAILURE

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

static eQWPttStatus NormalizeTpcChainData(tpAniSirGlobal pMac, tTpcCalPoint *chainData, tANI_U16 numTpcCalPointsPerFreq, tTpcCaldPowerPoint tpcChainCfg[MAX_TPC_CAL_POINTS]);



sAgcGainLut pttAgcGainLUT[PHY_MAX_RX_CHAINS];



tANI_U8 defaultAddr1[ANI_MAC_ADDR_SIZE] = { 0x00, 0x77, 0x55, 0x33, 0x11, 0x00 };   //dest
tANI_U8 defaultAddr3[ANI_MAC_ADDR_SIZE] = { 0x00, 0x77, 0x55, 0x33, 0x11, 0x00 };   //bssId

void pttModuleInit(tpAniSirGlobal pMac)
{
    uNvFields fields;
    pMac->hphy.phy.test.testChannelId = 1;
    pMac->hphy.phy.test.testCbState = PHY_SINGLE_CHANNEL_CENTERED;
    pMac->hphy.phy.test.testTpcClosedLoop = eANI_BOOLEAN_FALSE;
    pMac->hphy.phy.test.testTxGainIndexSource = RATE_POWER_NON_LIMITED;
    pMac->hphy.phy.test.testForcedTxPower = 0;
    pMac->hphy.phy.test.testLastPwrIndex = 0;
    pMac->hphy.phy.test.testForcedTxGainIndex = 0;

    pMac->ptt.forcedTxGain[PHY_TX_CHAIN_0].coarsePwr = TPC_COARSE_TXPWR_POS_20_DBM;
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
    pMac->ptt.agcEnables.rx[PHY_RX_CHAIN_1] = eANI_BOOLEAN_TRUE;

    pMac->ptt.wfmEnabled = eANI_BOOLEAN_FALSE;
    pMac->ptt.wfmStored = eANI_BOOLEAN_FALSE;

    // store rx agc gain luts for restoration later
    {

        //!Loading the gain tables for Gen6 is not necessary as they are initialized using a const table from the RF code.
        //asicGetAgcGainLut(pMac, PHY_RX_CHAIN_0, pttAgcGainLUT[PHY_RX_CHAIN_0]);
        //asicGetAgcGainLut(pMac, PHY_RX_CHAIN_1, pttAgcGainLUT[PHY_RX_CHAIN_1]);
    }

    /*! moved this from pttMsgInit, so we need to verify the init sequence with and without PTT GUI
        The idea is to have pttModuleInit initialize everything for manufacturing, and then pttMsgInit
        allows PTT GUI to get the initialized values.
        Anything that should be reinitialized or only done when PTT GUI is connecting can be done in pttMsgInit
    */

    pMac->hphy.phy.phyPeriodicCalEnable = eANI_BOOLEAN_FALSE; // ignore periodic calibrations.

    //always leave phydbg clock on for ptt
    rdModWrAsicField(pMac, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_REG,
                           QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_PHYDBG_MASK,
                           QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_PHYDBG_OFFSET,
                           1
                    );

    // Clear stats
    pttResetRxPacketStatistics(pMac);
}


// Msg Init - preparing for starting PTT and also providing default values back to GUI
eQWPttStatus pttMsgInit(tpAniSirGlobal pMac, tPttModuleVariables** ptt)
{
    phyLog(pMac, LOGE, "pttMsgInit: initialize PTT. \n");

    pttModuleInit(pMac);    //reinitialize when this message is received
    *ptt = &pMac->ptt;

    return SUCCESS;
}


/*
 * For Libra interpolating between 2dBm and 18dBm for TPC_MEM_POWER_LUT_DEPTH, the
 * slope value would be 8 and y-Intercept would be -16 and the straight line
 * looks like y = 8x - 16. where y being pAdc value and x being abs pwr measured
 */
static t2Decimal absReportedWithPrecision(tANI_U8 pdet)
{
    tANI_S32 slope = TPC_MEM_POWER_LUT_DEPTH/(MAX_PWR_LUT_DBM - MIN_PWR_LUT_DBM);
    tANI_S32 yIntercept = MIN_PWR_LUT_DBM * slope;  //actually it is -ve y-intercept
    tANI_S16 reported2Decimal = (tANI_S16)((pdet + yIntercept) * 100/slope);

    //If MTT commands 3.63dBm, the equivalent adjusted pwr would be 13.
    //But when you try to convret back 13 to floating point commanded pwr, it gives 3.62.
    //Again if MTT commands 3.62dBm, the equivalent adjusted pwr would be 12.
    //So two consecutive set and get TPC_CONFIG table result in different values.
    //To avoid this, add the precision. You are adding 1 to 362 to make it 363

    reported2Decimal += (((pdet * 100) % slope) > 0) ? 1 : 0;

    return (t2Decimal)reported2Decimal;
}

//NV Service
eQWPttStatus pttGetNvTable(tpAniSirGlobal pMac, eNvTable nvTable, uNvTables *tableData)
{
    eHalStatus rc = eHAL_STATUS_FAILURE;

    rc = halReadNvTable(pMac, nvTable, tableData);

    if (eHAL_STATUS_SUCCESS == rc)
    {
        if (nvTable == NV_TABLE_TPC_CONFIG)
        {
            //convert tTpcConfig back to the sTpcFreqCalTable structure, which is used by pttApi
            uNvTables tempTable;
            tANI_U8 chan;
            tANI_U8 txChain;
            tANI_U8 point;

            memcpy(&tempTable, tableData, sizeof(uNvTables));       //use temporary table to avoid accidental member overwriting
            memset(tableData, 0x0, sizeof(uNvTables));

            tableData->tpcFreqCal.numChannels = MAX_TPC_CHANNELS;

            //need to convert all float values to t2Decimal format
            for (chan = 0; chan < MAX_TPC_CHANNELS; chan++)
            {
                tableData->tpcFreqCal.calValues[chan].freq = tempTable.tpcConfig[chan].freq;

                for (txChain = 0; txChain < PHY_MAX_TX_CHAINS; txChain++)
                {
                    tableData->tpcFreqCal.calValues[chan].empirical[txChain].numTpcCalPoints = MAX_TPC_CAL_POINTS;

                    for (point = 0; point < MAX_TPC_CAL_POINTS; point++)
                    {
                        tableData->tpcFreqCal.calValues[chan].empirical[txChain].chain[point].txGain = 0;              //have nothing stored for this
                        tableData->tpcFreqCal.calValues[chan].empirical[txChain].chain[point].temperatureAdc = 0;      //have nothing stored for this
                        tableData->tpcFreqCal.calValues[chan].empirical[txChain].chain[point].pwrDetAdc = tempTable.tpcConfig[chan].empirical[txChain][point].pwrDetAdc;
                        tableData->tpcFreqCal.calValues[chan].empirical[txChain].chain[point].absPowerMeasured.reported =
                              absReportedWithPrecision(tempTable.tpcConfig[chan].empirical[txChain][point].adjustedPwrDet);

                        //tableData->tpcFreqCal.calValues[chan].empirical[txChain].chain[point].absPowerMeasured.reported =
                        //    phyGetAbsTxPowerForLutValue(pMac, txChain, tempTable.tpcConfig[chan].empirical[txChain][point].adjustedPwrDet);
                    }
                }
            }
        }

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
            case NV_TABLE_RF_CAL_VALUES:
            case NV_TABLE_RSSI_OFFSETS:
            case NV_TABLE_RSSI_CHANNEL_OFFSETS:
                if (eHAL_STATUS_FAILURE == halWriteNvTable(pMac, nvTable, tableData))
                {
                    phyLog(pMac, LOGE, "Unable to write table %d\n", (tANI_U32)nvTable);
                    return (FAILURE);
                }
                break;
            case NV_TABLE_TPC_CONFIG:
                //data received in sTpcFreqCalTable format, with power values in t2Decimal format
                // prescale TPC values in pttSaveTxPwrFreqTable(),
                //  which writes the results in tTpcConfig format to NV cache
                // The QFUSE implementation will pack/unpack between tTpcConfig and the QFuse bit layout.

                //TODO: add byte swapping for the sTpcFreqCalTable format before calling pttSaveTxPwrFreqTable
                return(pttSaveTxPwrFreqTable(pMac, tableData->tpcFreqCal.numChannels,
                                                   &tableData->tpcFreqCal.calValues[0]
                                            )
                      );

            case NV_TABLE_QFUSE:
                phyLog(pMac, LOGE, "ERROR: QFuse can not be directly written or read\n");
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
         (halPhySetChannel(pMac, (tANI_U8)chId, (ePhyChanBondState)cbState,FALSE) != eHAL_STATUS_SUCCESS)
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
            //asicGetAgcGainLut(pMac, PHY_RX_CHAIN_1, pttAgcGainLUT[PHY_RX_CHAIN_1]);
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
                case PHY_CHAIN_SEL_R0R1_T0_ON:
                case PHY_CHAIN_SEL_T0_R1_ON:
                case PHY_CHAIN_SEL_T0_ON:
                    if (asicEnableTxDACs(pMac, PHY_TX_CHAIN_0, eANI_BOOLEAN_ON, pMac->ptt.wfmEnabled) != eHAL_STATUS_SUCCESS) { return (FAILURE); }
                    break;

                case PHY_CHAIN_SEL_R1_ON:
                case PHY_CHAIN_SEL_R0_ON:
                case PHY_CHAIN_SEL_R0R1_ON:
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
    if (pMac->ptt.frameGenEnabled  == eANI_BOOLEAN_TRUE)
    {
        //can't allow starting  waveform while packet generation is going - it will cause phyDbg to hang.
        return (FAILURE);
    }

    if (pMac->ptt.wfmStored == eANI_BOOLEAN_TRUE)
    {
        if (asicStartTestWaveform(pMac, WAVE_CONTINUOUS, 0, pMac->ptt.numWfmSamples - 1) == eHAL_STATUS_SUCCESS)
        {
            pMac->ptt.wfmEnabled = eANI_BOOLEAN_TRUE;   //must preceed enabling the chains

            //make sure the chain selections are still accurate by resetting this again
            return (pttEnableChains(pMac, pMac->hphy.phy.activeChains));
        }
        else
        {
            pMac->ptt.wfmEnabled = eANI_BOOLEAN_FALSE;

            return (FAILURE);
        }
    }

    return (FAILURE);
}


eQWPttStatus pttStopWaveform(tpAniSirGlobal pMac)
{
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
}




eQWPttStatus pttCloseTpcLoop(tpAniSirGlobal pMac, tANI_BOOLEAN tpcClose)
{
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
    return (SUCCESS);
}



    //open loop service
eQWPttStatus pttSetPacketTxGainTable(tpAniSirGlobal pMac, ePhyTxChains txChain, tANI_U8 minIndex, tANI_U8 maxIndex, tANI_U8 *gainTable)
{
    tANI_U8 i;

    if ((txChain > 1) ||
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

        //pMac->hphy.phy.test.testTxGainIndexSource = FORCE_CLOSED_LOOP_GAIN;
    }
    else
    {
        return (FAILURE);
    }

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
    tPowerDetect adc0;
    tPwrTemplateRange lutRangeIndexed;
	//remove after test

    if (pMac->ptt.wfmEnabled)
    {
        //Waveform power is a direct measurement of the continuous waveform
        if(asicGetTxPowerMeasurement(pMac, PHY_TX_CHAIN_0, &adc0) != eHAL_STATUS_SUCCESS) { return FAILURE; }
    }
    else if ((pMac->ptt.frameGenEnabled) && (pMac->ptt.phyDbgFrameGen))
    {
        tANI_U32 sensed0;

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


        adc0 = (tPowerDetect)((sensed0 & QWLAN_TPC_SENSED_PWR0_VALUE_MASK) >> QWLAN_TPC_SENSED_PWR0_VALUE_OFFSET);
    }
    else
    {
        phyLog(pMac, LOGE, "ERROR: Tx power report only available if frames or a waveform is transmitted\n");
        memset(pwrReport, 0, sizeof(tTxPowerReport));
        return FAILURE;
    }



    adc0 >>= 1;

    pwrReport->channelId = pMac->hphy.phy.test.testChannelId;
    pwrReport->cbState = pMac->hphy.phy.test.testCbState;
    pwrReport->rate = pMac->ptt.frameGenParams.rate;
    pwrReport->pwrTemplateIndex = pMac->hphy.phy.test.testLastPwrIndex;

    pwrReport->txChains[PHY_TX_CHAIN_0].adc = adc0;
    asicGetTxGainAtIndex(pMac, PHY_TX_CHAIN_0, pwrReport->pwrTemplateIndex, &pwrReport->txChains[PHY_TX_CHAIN_0].gain);

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

    return SUCCESS;
}






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
                                    ((TPC_MEM_POWER_LUT_DEPTH/* * EXTRA_TPC_LUT_MULT*/) -1));
            // The ADC reading for a power measurement greater than MAX_PWR_LUT_DBM is clipped
            //SET_FULL_PRECISION_TPC_LUT_VALUE(((TPC_MEM_POWER_LUT_DEPTH * EXTRA_TPC_LUT_MULT) -1), tpcChainCfg[calPoint]);
            tpcChainCfg[calPoint].adjustedPwrDet = TPC_MEM_POWER_LUT_DEPTH - 1;

        }
        else
        {
            tANI_S32 x1 = (t2Decimal)(MIN_PWR_LUT_DBM_2DEC_PLACES);
            tANI_S32 y1 = 0;
            tANI_S32 x2 = (t2Decimal)(MAX_PWR_LUT_DBM_2DEC_PLACES);
            tANI_S32 y2 = ((TPC_MEM_POWER_LUT_DEPTH/* * EXTRA_TPC_LUT_MULT*/) -1);
            tTpcLutValue lutValue;

            // this simply scales the power measurements to the necessary range
            // since x1 ( = 8(00) ) <= x <= x2 ( =24(00) ) and y1 = 0, y2 = 127
            lutValue = (tTpcLutValue)InterpolateBetweenPoints(x1, y1, x2, y2, x);
            //SET_FULL_PRECISION_TPC_LUT_VALUE(lutValue, tpcChainCfg[calPoint]);
            tpcChainCfg[calPoint].adjustedPwrDet = (tPowerDetect)lutValue;

            phyLog(pMac, LOGE, "NormalizeTpcChannelData(): measured pwr %d dbm, mapping pwr adc code %d to %d\n",
                                    (t2Decimal)(MIN_PWR_LUT_DBM_2DEC_PLACES), chainData[calPoint].pwrDetAdc,
                                    tpcChainCfg[calPoint].adjustedPwrDet/*GET_FULL_PRECISION_TPC_LUT_VALUE(tpcChainCfg[calPoint].adjustedPwrDet, tpcChainCfg[calPoint].extraPrecision.hi8_adjustedPwrDet)*/ );
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


    if (/*(table[0].freq != START_TPC_CHANNEL) || (table[1].freq != END_TPC_CHANNEL) ||*/ (numTpcCalFreqs != MAX_TPC_CHANNELS))
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
        if (ConfigureTpcFromNv(pMac) != eHAL_STATUS_SUCCESS)
        {
            retVal = FAILURE;
        }
    }


    return (retVal);

}



eQWPttStatus pttSetPowerLut(tpAniSirGlobal pMac, ePhyTxChains txChain, tANI_U8 minIndex, tANI_U8 maxIndex, tANI_U8 *powerLut)
{
    tANI_U32 i;

    if ((txChain > 1) ||
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

    if( (gains.rx[PHY_RX_CHAIN_0] >= RF_AGC_GAIN_LUT_DEPTH) ||
        (gains.rx[PHY_RX_CHAIN_1] >= RF_AGC_GAIN_LUT_DEPTH)
      )
    {
        return (FAILURE);
    }

    asicOverrideAGCRxChainGain(pMac, PHY_RX_CHAIN_0, gains.rx[PHY_RX_CHAIN_0]);
    asicOverrideAGCRxChainGain(pMac, PHY_RX_CHAIN_1, gains.rx[PHY_RX_CHAIN_1]);

    pMac->ptt.agcEnables.rx[PHY_RX_CHAIN_0] = eANI_BOOLEAN_FALSE;
    pMac->ptt.agcEnables.rx[PHY_RX_CHAIN_1] = eANI_BOOLEAN_FALSE;

    return (SUCCESS);
}


eQWPttStatus pttEnableAgcTables(tpAniSirGlobal pMac, sRxChainsAgcEnable enables)
{
    if ((enables.rx[PHY_RX_CHAIN_0] > 1) ||
        (enables.rx[PHY_RX_CHAIN_1] > 1)
       )
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

    if (enables.rx[PHY_RX_CHAIN_1] == eANI_BOOLEAN_TRUE)
    {
        asicCeaseOverrideAGCRxChainGain(pMac, PHY_RX_CHAIN_1);

        //load current AGC Gain Tables
        //!Loading the gain tables for Gen6 is not necessary as they are initialized using a const table from the RF code.
        //asicSetAGCGainLut(pMac, PHY_RX_CHAIN_1, 0, (NUM_GAIN_LUT_SETTINGS - 1), &pttAgcGainLUT[PHY_RX_CHAIN_1][0]);
    }
    pMac->ptt.agcEnables.rx[PHY_RX_CHAIN_1] = enables.rx[PHY_RX_CHAIN_1];

    return (SUCCESS);
}

#define RSSI_TO_DBM_OFFSET     -105

void pttCollectAdcRssiStats(tpAniSirGlobal pMac)
{
    tANI_U32 rssi0, rssi1, rssi0Stats = 0, rssi1Stats = 0;
    tANI_U8 counter, validCounterRssi0 = 0, validCounterRssi1 = 0;

    /*check the valid bit '8th bit' before reading values
     which should be set to 1 for valid RSSI reads*/

    for (counter = 0; counter < 10; counter++ ) {

        palReadRegister(pMac->hHdd, QWLAN_AGC_ADC_RSSI0_REG, &rssi0);
        palReadRegister(pMac->hHdd, QWLAN_AGC_ADC_RSSI1_REG, &rssi1);

        /*reading rssi0 value*/
        if(rssi0 & QWLAN_AGC_ADC_RSSI0_INVALID_MASK)
        {
            rssi0Stats += (tANI_U8)(rssi0 & QWLAN_AGC_ADC_RSSI0_RSSI_MASK);
            validCounterRssi0++;
        }
        else
        {
            rssi0Stats += 0;
        }
        /*reading rssi1 value*/
        if(rssi1 & QWLAN_AGC_ADC_RSSI1_INVALID_MASK)
        {
            rssi1Stats += (tANI_U8)(rssi1 & QWLAN_AGC_ADC_RSSI1_RSSI_MASK);
            validCounterRssi1++;
        }
        else
        {
            rssi1Stats += 0;
        }
    }

    /*average the value over valid reads*/
    rssi0 = ((rssi0Stats == 0)?0:(rssi0Stats/validCounterRssi0));
    rssi1 = ((rssi1Stats == 0)?0:(rssi1Stats/validCounterRssi1));

    /*assign rssiValues to response*/
    if(rssi0 > 0)
    {
        pMac->ptt.rssi.rx[PHY_RX_CHAIN_0] = (tANI_S8)rssi0;
    }

    if(rssi1 >0)
    {
        pMac->ptt.rssi.rx[PHY_RX_CHAIN_1] = (tANI_S8)rssi1;
    }

}

void pttGetRxRssi(tpAniSirGlobal pMac, sRxChainsRssi *rssi)
{
    eRfChannels curChan = rfGetChannelIndex(pMac->hphy.phy.test.testChannelId, pMac->hphy.phy.test.testCbState);
    t2Decimal rssiOffset0, rssiOffset1;

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
            //rssiOffset = pMac->hphy.phy.regDomainInfo[pMac->hphy.phy.curRegDomain].bRatePowerOffset[curChan].reported / 100;
            rssiOffset0 = rssiChanOffsets[PHY_RX_CHAIN_0].bRssiOffset[curChan] / 100;
            rssiOffset1 = rssiChanOffsets[PHY_RX_CHAIN_1].bRssiOffset[curChan] / 100;
        }
        else
        {
            //rssiOffset = pMac->hphy.phy.regDomainInfo[pMac->hphy.phy.curRegDomain].gnRatePowerOffset[curChan].reported / 100;
            rssiOffset0 = rssiChanOffsets[PHY_RX_CHAIN_0].gnRssiOffset[curChan] / 100;
            rssiOffset1 = rssiChanOffsets[PHY_RX_CHAIN_1].gnRssiOffset[curChan] / 100;
        }
    }

    rssi->rx[PHY_RX_CHAIN_0] = pMac->ptt.rssi.rx[PHY_RX_CHAIN_0] + RSSI_TO_DBM_OFFSET + rssiOffset0;
    rssi->rx[PHY_RX_CHAIN_1] = pMac->ptt.rssi.rx[PHY_RX_CHAIN_1] + RSSI_TO_DBM_OFFSET + rssiOffset1;
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

    {
        tANI_U32 dmaDrop, typeSubTypeFilter, addr1Drop;
        palReadRegister(pMac->hHdd, QWLAN_RXP_DMA_DROP_CNT_REG, &dmaDrop);
        palReadRegister(pMac->hHdd, QWLAN_RXP_TYPE_SUBTYPE_FILTER_CNT_REG, &typeSubTypeFilter);
        palReadRegister(pMac->hHdd, QWLAN_RXP_ADDR1_DROP_CNT_REG, &addr1Drop);

        //QWLAN_RXP_DMA_DROP_CNT_REG - QWLAN_RXP_TYPE_SUBTYPE_FILTER_CNT_REG - QWLAN_RXP_ADDR1_DROP_CNT_REG
        counters->totalMacRxPackets = dmaDrop - typeSubTypeFilter - addr1Drop;
    }

    if (eHAL_STATUS_SUCCESS != palReadRegister(pMac->hHdd, QWLAN_RXP_FCS_ERR_CNT_REG, &counters->totalMacFcsErrPackets))
    {
        return (FAILURE);
    }

    return (SUCCESS);
}



eQWPttStatus pttResetRxPacketStatistics(tpAniSirGlobal pMac)
{
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

    return (SUCCESS);
}






//Rx Symbol Service
eQWPttStatus pttGrabRam(tpAniSirGlobal pMac, tANI_U32 startSample, tANI_U32 numSamples, tGrabRamSample *grabRam)
{
    if ((numSamples == 0) || (numSamples > MAX_REQUESTED_GRAB_RAM_SAMPLES) || (startSample > LAST_GRAB_RAM_SAMPLE_INDEX))
    {
        return (FAILURE);
    }

        if (eHAL_STATUS_SUCCESS == asicGrabAdcSamples(pMac, startSample, numSamples, grabRam))
        {
            return (SUCCESS);
        }
        else
        {
            return (FAILURE);
        }


    return (SUCCESS);
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

#define ABS(x)      ((x < 0) ? -x : x)
eQWPttStatus pttHdetCal(tpAniSirGlobal pMac, sRfNvCalValues *rfCalValues, tANI_BOOLEAN internal)//gs_low=0,gs_high=15,detector='external')
{
    //tANI_U32 gs_low=0, gs_high=15, detector='external';
    eHalStatus retVal;
    tANI_S32 padc, curr_min;// = N.zeros((128,1),int)
    tANI_S32 target_low = 10, target_high = 220;
    tANI_U8 ib_scale, ib_rcal, dcoc, opt_val_atten = 0, ext_atten;
    tANI_U32 opt_val, revId;

    pMac->hphy.phy.test.testInternalHdetCal = internal;

    if(pMac->hphy.phy.test.testInternalHdetCal == eANI_BOOLEAN_FALSE)
    //if(pMac->hphy.rf.revId < 0x200)
    {
        //setup tx
        SET_PHY_REG(pMac->hHdd, QWLAN_TPC_GAIN_LUT0_MREG, 0x5f);
        sirBusyWait(50000000);

        SET_PHY_REG(pMac->hHdd, QWLAN_RFIF_GC_CFG_REG, 0);

        //always start with default values
        SET_PHY_REG(pMac->hHdd, QWLAN_RFAPB_HDET_DCOC_REG, 0xc000);
#if 1//def LIBRA_1_1
        SET_PHY_REG(pMac->hHdd, QWLAN_RFAPB_HDET_CTL_REG, 0x3660);
#else
        SET_PHY_REG(pMac->hHdd, QWLAN_RFAPB_HDET_CTL_REG, 0x3667);
#endif
        rdModWrAsicField(pMac, QWLAN_RFAPB_HDET_CTL_REG, QWLAN_RFAPB_HDET_CTL_HDET_PATH_SEL_MASK, QWLAN_RFAPB_HDET_CTL_HDET_PATH_SEL_OFFSET, 0);

        GET_PHY_REG(pMac->hHdd, QWLAN_RFAPB_REV_ID_REG, &revId);
        SET_PHY_REG(pMac->hHdd, QWLAN_RFIF_GC_CFG_REG, (QWLAN_RFIF_GC_CFG_TX_GAIN_EN_MASK | QWLAN_RFIF_GC_CFG_RX_GAIN_EN_MASK));


        //set gain to 0, sweep the hdet_dcoc, and pick a value near 10 for sensed power
        curr_min = 1000;
        for(ib_scale = 0; ib_scale < 2; ib_scale++)
        {
            rdModWrAsicField(pMac, QWLAN_RFAPB_HDET_DCOC_REG, QWLAN_RFAPB_HDET_DCOC_IB_SCAL_EN_MASK, QWLAN_RFAPB_HDET_DCOC_IB_SCAL_EN_OFFSET, ib_scale);

            for(ib_rcal = 0; ib_rcal < 2; ib_rcal++)
            {
                rdModWrAsicField(pMac, QWLAN_RFAPB_HDET_DCOC_REG, QWLAN_RFAPB_HDET_DCOC_IB_RCAL_EN_MASK, QWLAN_RFAPB_HDET_DCOC_IB_RCAL_EN_OFFSET, ib_rcal);

                for(dcoc = 0; dcoc < 32; dcoc++)
                {
                    rdModWrAsicField(pMac, QWLAN_RFAPB_HDET_DCOC_REG, QWLAN_RFAPB_HDET_DCOC_DCOC_CODE_5_0_MASK, QWLAN_RFAPB_HDET_DCOC_DCOC_CODE_5_0_OFFSET, dcoc);
                    sirBusyWait(50000000);

                    GET_PHY_REG(pMac->hHdd, QWLAN_TPC_SENSED_PWR0_REG, &padc);
                    if(ABS(padc-target_low) < curr_min)
                    {
                        curr_min = ABS(padc-target_low);
                        GET_PHY_REG(pMac->hHdd, QWLAN_RFAPB_HDET_DCOC_REG, &opt_val);
                        phyLog(pMac, LOGE, "----- dcoc new opt -----\n");
                    }
                    phyLog(pMac, LOGE, "padc = %d, ib_scale = %d, ib_rcal = %d, dcoc = %d\n", padc, ib_scale, ib_rcal, dcoc);
                }
            }
        }

        SET_PHY_REG(pMac->hHdd, QWLAN_RFAPB_HDET_DCOC_REG, opt_val);

        //now, set gain to max, sweep ext_atten and pick a value near high_target

        SET_PHY_REG(pMac->hHdd, QWLAN_TPC_GAIN_LUT0_MREG, 0xff);
        sirBusyWait(50000000);

        SET_PHY_REG(pMac->hHdd, QWLAN_RFIF_GC_CFG_REG, 0);

        GET_PHY_REG(pMac->hHdd, QWLAN_RFAPB_REV_ID_REG, &revId);
        SET_PHY_REG(pMac->hHdd, QWLAN_RFIF_GC_CFG_REG, (QWLAN_RFIF_GC_CFG_TX_GAIN_EN_MASK | QWLAN_RFIF_GC_CFG_RX_GAIN_EN_MASK));


        curr_min = 1000;
#if 1//def LIBRA_1_1
        for(ext_atten = 0; ext_atten < 16; ext_atten++)
        {
            rdModWrAsicField(pMac, QWLAN_RFAPB_HDET_CTL_REG, QWLAN_RFAPB_HDET_CTL_EXT_ATTEN_3_0_MASK, QWLAN_RFAPB_HDET_CTL_EXT_ATTEN_3_0_OFFSET, ext_atten);
            sirBusyWait(50000000);

            GET_PHY_REG(pMac->hHdd, QWLAN_TPC_SENSED_PWR0_REG, &padc);
            if(ABS(padc-target_high)<curr_min)
            {
                curr_min = ABS(padc-target_high);
                opt_val_atten = ext_atten;
                phyLog(pMac, LOGE, "----- atten new opt -----\n");
            }
            phyLog(pMac, LOGE, "padc = %d, ext_atten = %d\n", padc, ext_atten);
        }
#else
        opt_val_atten = 0x3;
#endif
        rdModWrAsicField(pMac, QWLAN_RFAPB_HDET_CTL_REG, QWLAN_RFAPB_HDET_CTL_EXT_ATTEN_3_0_MASK, QWLAN_RFAPB_HDET_CTL_EXT_ATTEN_3_0_OFFSET, opt_val_atten);
        rdModWrAsicField(pMac, QWLAN_RFAPB_HDET_CTL_REG, QWLAN_RFAPB_HDET_CTL_INT_ATTEN_3_0_MASK, QWLAN_RFAPB_HDET_CTL_INT_ATTEN_3_0_OFFSET, opt_val_atten);

        SET_PHY_REG(pMac->hHdd, QWLAN_TPC_GAIN_LUT0_MREG, 0xf);

        //opt_val_atten, opt_val
        pMac->ptt.rfCalValues.hdet_ctl_ext_atten              = (tANI_U8)(opt_val_atten);
        pMac->ptt.rfCalValues.hdet_dcoc_code                  = (tANI_U8)((opt_val & QWLAN_RFAPB_HDET_DCOC_DCOC_CODE_5_0_MASK) >> QWLAN_RFAPB_HDET_DCOC_DCOC_CODE_5_0_OFFSET);
        pMac->ptt.rfCalValues.hdet_dcoc_ib_rcal_en            = (tANI_U8)((opt_val & QWLAN_RFAPB_HDET_DCOC_IB_RCAL_EN_MASK) >> QWLAN_RFAPB_HDET_DCOC_IB_RCAL_EN_OFFSET);
        pMac->ptt.rfCalValues.hdet_dcoc_ib_scal_en            = (tANI_U8)((opt_val & QWLAN_RFAPB_HDET_DCOC_IB_SCAL_EN_MASK) >> QWLAN_RFAPB_HDET_DCOC_IB_SCAL_EN_OFFSET);

        memcpy(rfCalValues, &pMac->ptt.rfCalValues, sizeof(sRfNvCalValues));
        memcpy(&pMac->hphy.nvCache.tables.rfCalValues, &pMac->ptt.rfCalValues, sizeof(sRfNvCalValues));
    }
    else
    {
        SET_PHY_REG(pMac->hHdd, QWLAN_RFIF_GC_CFG_REG, 0);

        //trigger the internal cal
        SET_PHY_REG(pMac->hHdd, QWLAN_RFAPB_HDET_DCOC_REG, 0x4000);
        {
            //eRfChannels tempChan = rfGetChannelIndex(pMac->hphy.phy.test.testChannelId, pMac->hphy.phy.test.testCbState);
            SET_PHY_REG(pMac->hHdd, QWLAN_RFAPB_HDET_CTL_REG, 0xb112/*hdetVals[tempChan].hdetCalVal*/);
        }

        GET_PHY_REG(pMac->hHdd, QWLAN_RFAPB_REV_ID_REG, &revId);
        SET_PHY_REG(pMac->hHdd, QWLAN_RFIF_GC_CFG_REG, (QWLAN_RFIF_GC_CFG_TX_GAIN_EN_MASK | QWLAN_RFIF_GC_CFG_RX_GAIN_EN_MASK));


    }
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

    if ((eHAL_STATUS_SUCCESS == rfSetTxLoCorrect(pMac, PHY_TX_CHAIN_0, gain, calValues.txLo[PHY_TX_CHAIN_0]))
       )
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


    if ((eHAL_STATUS_SUCCESS == rfGetTxLoCorrect(pMac, PHY_TX_CHAIN_0, gain, &calValues->txLo[PHY_TX_CHAIN_0]))
       )
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

    if (eHAL_STATUS_SUCCESS == asicTxFirSetTxPhaseCorrection(pMac, gain, PHY_TX_CHAIN_0, calValues.iq[PHY_TX_CHAIN_0]))
    {
        return (SUCCESS);
    }
    else
    {
        return (FAILURE);
    }
    return (FAILURE);
}

eQWPttStatus pttGetTxIqCorrect(tpAniSirGlobal pMac, sTxChainsIQCalValues *calValues, eGainSteps gain)
{
    if ((tANI_U32)gain > NUM_TX_GAIN_STEPS)
    {
        phyLog(pMac, LOGE, "Incorrect Gain %d\n", gain);
        return (FAILURE);
    }

    if (eHAL_STATUS_SUCCESS == asicTxFirGetTxPhaseCorrection(pMac, gain, PHY_TX_CHAIN_0, &calValues->iq[PHY_TX_CHAIN_0]))
    {
        return (SUCCESS);
    }
    else
    {
        return (FAILURE);
    }
    return (FAILURE);
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
    asicWriteRxPhaseCorrection(pMac, gain, PHY_RX_CHAIN_0, calValues.iq[PHY_RX_CHAIN_0]);
    asicWriteRxPhaseCorrection(pMac, gain, PHY_RX_CHAIN_1, calValues.iq[PHY_RX_CHAIN_1]);

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

    asicReadRxPhaseCorrection(pMac, gain, PHY_RX_CHAIN_0, &correct);
    calValues->iq[PHY_RX_CHAIN_0].center = correct.center;
    calValues->iq[PHY_RX_CHAIN_0].offCenter = correct.offCenter;
    calValues->iq[PHY_RX_CHAIN_0].imbalance = correct.imbalance;

    asicReadRxPhaseCorrection(pMac, gain, PHY_RX_CHAIN_1, &correct);
    calValues->iq[PHY_RX_CHAIN_1].center = correct.center;
    calValues->iq[PHY_RX_CHAIN_1].offCenter = correct.offCenter;
    calValues->iq[PHY_RX_CHAIN_1].imbalance = correct.imbalance;

    return (SUCCESS);
}


eQWPttStatus pttSetRxDcoCorrect(tpAniSirGlobal pMac, tRxChainsDcoCorrections calValues, tANI_U8 gain)
{
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
    rfSetDCOffset(pMac, PHY_RX_CHAIN_0, gain, calValues.dco[PHY_RX_CHAIN_0]);
    rfSetDCOffset(pMac, PHY_RX_CHAIN_1, gain, calValues.dco[PHY_RX_CHAIN_1]);

    return (SUCCESS);
}


eQWPttStatus pttGetRxDcoCorrect(tpAniSirGlobal pMac, tRxChainsDcoCorrections *calValues, tANI_U8 gain)
{
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
    rfGetDCOffset(pMac, PHY_RX_CHAIN_0, gain, &calValues->dco[PHY_RX_CHAIN_0]);
    rfGetDCOffset(pMac, PHY_RX_CHAIN_1, gain, &calValues->dco[PHY_RX_CHAIN_1]);

    return (SUCCESS);
}

eQWPttStatus pttGetHdetCorrect(tpAniSirGlobal pMac, sRfNvCalValues *rfCalValues)
{
    memcpy(rfCalValues, &pMac->ptt.rfCalValues, sizeof(sRfNvCalValues));

    return SUCCESS;
}



//Rf Calibration Service
eQWPttStatus pttGetTempAdc(tpAniSirGlobal pMac, tANI_U8 *tempAdc)
{
    if (eHAL_STATUS_SUCCESS == rfTakeTemp(pMac, 1, (tTempADCVal *)tempAdc))
    {
       return (SUCCESS);
    }
    else
    {
       return (FAILURE);
    }

    return (SUCCESS);
}

eQWPttStatus pttReadRfField(tpAniSirGlobal pMac, eRfFields rfFieldId, tANI_U32 *value)
{
    if (eHAL_STATUS_SUCCESS == rfReadField(pMac, rfFieldId, value, GEMINI_CHIP))
    {
        return (SUCCESS);
    }
    else
    {
        return (FAILURE);
    }
}


eQWPttStatus pttWriteRfField(tpAniSirGlobal pMac, eRfFields rfFieldId, tANI_U32 value)
{
    if (eHAL_STATUS_SUCCESS == rfWriteField(pMac, rfFieldId, value, GEMINI_CHIP))
    {
        return (SUCCESS);
    }
    else
    {
        return (FAILURE);
    }
}

//abr: commented as deepSleep support is not in FTM

/*eQWPttStatus pttDeepSleep(tpAniSirGlobal pMac)
{
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
                                     QWLAN_PMU_RING_OSC_CTRL_SEL_REG_PMU_GCU_ROSC_CLK_GATE_SEL_REG_MASK |
                                     QWLAN_PMU_RING_OSC_CTRL_SEL_REG_RING_OSC_PWR_EN_SEL_REG_MASK));
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

    return (SUCCESS);
}
*/

eQWPttStatus pttSystemReset(tpAniSirGlobal pMac)
{
    //TODO: determine what to do for reset halPerformSystemReset((tHalHandle)pMac);
    return (SUCCESS);
}

eQWPttStatus pttLogDump(tpAniSirGlobal pMac, tANI_U32 cmd, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4)
{

    tANI_U8 buf[3000];
    buf[0] = '\0';
    logRtaiDump(pMac, cmd, arg1, arg2, arg3, arg4, buf);
    buf[2999] = '\0';
    phyLog(pMac, LOGE, "logDump %d %x %x %x %x \n",  cmd, arg1, arg2, arg3, arg4);
    phyLog(pMac, LOGE, "logDump %s\n",  buf);

    return (SUCCESS);
}

#endif


/**
 *
   Airgo Networks, Inc proprietary.
   All Rights Reserved, Copyright 2005
   This program is the confidential and proprietary product of Airgo Networks Inc.
   Any Unauthorized use, reproduction or transfer of this program is strictly prohibited.


   phyCalService.cc: Calibration functionality extending from Baseband blocks to RF
   Author:  Mark Nelson
   Date:    3/10/05

   History -
   Date        Modified by              Modification Information
  --------------------------------------------------------------------------

 */
#include "ani_assert.h"
#include "sys_api.h"





eHalStatus phyInitialCalStart(tpAniSirGlobal pMac)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
    
    //if these bringup tables exist, then get the table locations for use when changing channels
    //don't bother checking the return values here,
    // the pointer will be NULL if it is not found and function returns eHAL_STATUS_FAILURE in this case
    halGetEepromTableLoc(pMac, EEPROM_TABLE_TX_IQ, (uEepromTables **)&pMac->hphy.phy.txIQTable);
    halGetEepromTableLoc(pMac, EEPROM_TABLE_RX_IQ, (uEepromTables **)&pMac->hphy.phy.rxIQTable);
    halGetEepromTableLoc(pMac, EEPROM_TABLE_TX_LO, (uEepromTables **)&pMac->hphy.phy.txLoCorrectTable);
    halGetEepromTableLoc(pMac, EEPROM_TABLE_RX_DCO, (uEepromTables **)&pMac->hphy.phy.rxDcoTable);
    halGetEepromTableLoc(pMac, EEPROM_TABLE_DEMO_CHANNEL_LIST, (uEepromTables **)&pMac->hphy.phy.demoFreqs);
    halGetEepromTableLoc(pMac, EEPROM_TABLE_LNA_SW_GAIN, (uEepromTables **)&pMac->hphy.phy.lnaSwGainTable);


    pMac->hphy.phy.phyUseBringupTables = eANI_BOOLEAN_FALSE;
    pMac->hphy.phy.phyPeriodicCalEnable = eANI_BOOLEAN_FALSE; // only enabled below where we have the capability and know the initial cal is working

    //only boards that are RF-capable can perform or use the phy calibrations
    if (pMac->hphy.phy.test.testDisableSpiAccess == eANI_BOOLEAN_FALSE)
    {

        SET_PHY_REG(pMac->hHdd, CAL_IQ_CORR_MODE_REG, 0);  //will always use values from correction memory

#ifdef ANI_MANF_DIAG
        {
            //For manf build, we will run this calibration every time on initialization, and store the result into EEPROM
            //Note: if this is being run on the Betta ATE setup, the REG_ACCESS field in EEPROM should be set so that this
            // initial cal does not actually access any registers. testDisableSpiAccess = TRUE in this case.


            if (rfGetCurChannel(pMac) != INVALID_RF_CHANNEL)
            {
                if (RunInitialCal(pMac, ALL_CALS) == eHAL_STATUS_SUCCESS)
                {
                    phyLog(pMac, LOGE, "Initial Cal Succeeded\n");
                }
                else
                {
                    phyLog(pMac, LOGE, "ERROR: Initial Cals Failed, periodic cal disbled.\n");
                    pMac->hphy.phy.phyPeriodicCalEnable = eANI_BOOLEAN_FALSE;
                }
            }
            else if (pMac->hphy.phy.txIQTable != NULL &&
                     pMac->hphy.phy.rxIQTable != NULL &&
                     pMac->hphy.phy.txLoCorrectTable != NULL &&
                     pMac->hphy.phy.rxDcoTable != NULL &&
                     pMac->hphy.phy.demoFreqs != NULL &&
                     pMac->hphy.phy.lnaSwGainTable != NULL
                    )
            {
                //all Betta tables are recorded
                phyLog(pMac, LOGW, "Loading bringup tables from EEPROM, periodic cal disabled\n");
                pMac->hphy.phy.phyUseBringupTables = eANI_BOOLEAN_TRUE; //enable cal from bringup ATE tables
                pMac->hphy.phy.phyPeriodicCalEnable = eANI_BOOLEAN_FALSE;
            }
        }
#else
        if (pMac->hphy.phy.txIQTable != NULL &&
                 pMac->hphy.phy.rxIQTable != NULL &&
                 pMac->hphy.phy.txLoCorrectTable != NULL &&
                 pMac->hphy.phy.rxDcoTable != NULL &&
                 pMac->hphy.phy.demoFreqs != NULL &&
                 pMac->hphy.phy.lnaSwGainTable != NULL
                )
        {
            //all Betta tables are recorded
            phyLog(pMac, LOGW, "Loading bringup tables from EEPROM, periodic cal disabled\n");
            pMac->hphy.phy.phyUseBringupTables = eANI_BOOLEAN_TRUE; //enable cal from bringup ATE tables
            pMac->hphy.phy.phyPeriodicCalEnable = eANI_BOOLEAN_FALSE;
        }
        else if (pMac->hphy.phy.test.testDisableSpiAccess == eANI_BOOLEAN_FALSE)
        {

            phyLog(pMac, LOGE, "ERROR: Initial calibration values not found\n");
            phyLog(pMac, LOGE, "ERROR: bringup tables not found\n");

            if (pMac->hphy.phy.lnaSwGainTable == NULL)
            {
                phyLog(pMac, LOGE, "ERROR: Missing EEPROM_TABLE_LNA_SW_GAIN. Cannot proceed without accurate measurements.\n");
                return (eHAL_STATUS_FAILURE);
            }
            else if (rfGetCurChannel(pMac) != INVALID_RF_CHANNEL)
            {                
                //LNA switch gains present, we should attempt to run the initial cals
                phyLog(pMac, LOGE, "Running Initial Cals on current channel...\n");
                if (RunInitialCal(pMac, ALL_CALS) == eHAL_STATUS_SUCCESS)
                {
                    phyLog(pMac, LOGE, "Initial Cal Succeeded\n");
                }
                else
                {
                    phyLog(pMac, LOGE, "ERROR: Initial Cals Failed, periodic cal disbled.\n");
                    pMac->hphy.phy.phyPeriodicCalEnable = eANI_BOOLEAN_FALSE;
                    return (eHAL_STATUS_FAILURE);
                }                    
            }
        }
#endif
    }
#ifndef ANI_MANF_DIAG
    else if (pMac->hphy.phy.txIQTable != NULL &&
             pMac->hphy.phy.rxIQTable != NULL &&
             pMac->hphy.phy.txLoCorrectTable != NULL &&
             pMac->hphy.phy.rxDcoTable != NULL &&
             pMac->hphy.phy.demoFreqs != NULL &&
             pMac->hphy.phy.lnaSwGainTable != NULL
            )
    {
        //all Betta tables are recorded
        pMac->hphy.phy.phyUseBringupTables = eANI_BOOLEAN_TRUE;
        //this may be a board fresh from Betta programming
        //we can convert the PHY_REG_ACCESS field to RF_ALL_ACCESS automatically in this case
        if (pMac->hphy.phy.test.testDisablePhyRegAccess == eANI_BOOLEAN_TRUE)
        {
            //phy access disabled means the REG_ACCESS field is not set to RF - change it
            uEepromFields fields;

            //must change SDRAM appropriately for RF card
            fields.sdramInfo = (tANI_U8)SDRAM_NOT_PRESENT;
            if (eHAL_STATUS_SUCCESS != halWriteEepromField(pMac, EEPROM_COMMON_SDRAM_INFO, &fields))
            {
                phyLog(pMac, LOGE, "Failed to write EEPROM\n");
                return (eHAL_STATUS_FAILURE);   //return failure here to force driver re-enable
            }

            fields.phyRegAccess = (tANI_U8)RF_ALL_ACCESS;

            if ((eHAL_STATUS_SUCCESS != halWriteEepromField(pMac, EEPROM_COMMON_REG_ACCESS, &fields)) ||
                (eHAL_STATUS_SUCCESS != halStoreTableToEeprom(pMac, EEPROM_FIELDS_IMAGE))
               )
            {
                phyLog(pMac, LOGE, "ERROR: Failed to convert this board to RF_ALL_ACCESS\n");
            }
            else
            {
                phyLog(pMac, LOGE, "Board converted to RF_ALL_ACCESS\n");
                phyLog(pMac, LOGE, "!If this is not an RF board, this could result in a system hang! Proceed to blank EEPROM.\n");
                phyLog(pMac, LOGE, "If this is an RF board, then re-enable the driver for this change to take effect");
                return (eHAL_STATUS_FAILURE);   //return failure here to force driver re-enable
            }
        }
    }
#endif

    return (retVal);
}



eHalStatus LoadRecentCalValues(tpAniSirGlobal pMac, eInitCals calId, eRfSubBand currBand)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
    tANI_U32 gain, chain;

    assert((currBand == RF_BAND_2_4_GHZ) || (currBand == RF_BAND_5_GHZ));
    assert(calId != ALL_CALS);
    assert(pMac->hphy.phy.calTable[currBand] != NULL);
    
    if (calId == TX_LO_CAL_ONLY)
    {
        phyLog(pMac, LOG2, "Loading TXLO Values from Table for %s band\n", (currBand == 0) ? "2.4GHz":"5GHz");

        for (chain = 0; chain < PHY_MAX_TX_CHAINS; chain++)
        {
            if ((retVal = rfSetTxLoCorrect(pMac, chain, pMac->hphy.phy.calTable[currBand]->txloCorrection[chain])) != eHAL_STATUS_SUCCESS) { return (retVal); }
            phyLog(pMac, LOG2, "Chain = %d, Correction values : I = %d, Q = %d\n", chain, 
                                        pMac->hphy.phy.calTable[currBand]->txloCorrection[chain].iLo, 
                                        pMac->hphy.phy.calTable[currBand]->txloCorrection[chain].qLo); 
        
        }
        phyLog(pMac, LOG3, "TxLO Cal Loaded, Band = %d, Channel = %d\n", currBand, rfGetCurChannel(pMac));
     }

    if (calId == RX_DCO_CAL_ONLY)
    {
        phyLog(pMac, LOG2, "Loading DCO Cal Values from Table for %s band\n", (currBand == 0) ? "2.4GHz":"5GHz");
        for (chain = 0; chain < PHY_MAX_RX_CHAINS; chain++)
        {
            for (gain = 0; gain < NUM_QUASAR_RX_GAIN_STEPS; gain++)
            {
                if ((retVal = rfSetDCOffset(pMac, chain, gain, pMac->hphy.phy.calTable[currBand]->dcoCorrection[chain])) != eHAL_STATUS_SUCCESS) { return (retVal); }
            }
        }
        phyLog(pMac, LOG3, "DCO Cal Loaded, Band = %d, Channel = %d\n", currBand, rfGetCurChannel(pMac));
    }

#ifdef ANI_PHY_DEBUG
//    retVal = dumpQuasarCorrectionValues(pMac, calId);
#endif

    return (retVal);
}


#include "testNovaRegs.h"
#include "rfApi.h"
eHalStatus RunInitialCal(tpAniSirGlobal pMac, eInitCals calId)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
    ePhyRxDisabledPktTypes agcDisModeReg;
    tANI_U32 bandwidthConfig;


#ifdef ANI_PHY_DEBUG
    //keep this here for debugging
/*
        testLogPhyWatchList(pMac);
        {
            tANI_U32 i;
            for (i = 0; i < NUM_QUASAR_FIELDS; i++)
            {
                tANI_U32 value;
        
                if (eHAL_STATUS_SUCCESS == rfReadQuasarField(pMac, i, &value))
                {
                    phyLog(pMac, LOGE, "%d:  %s = 0x%08X\n", i, &quasarFieldStr[i][0], value);
                }
                else
                {
                    phyLog(pMac, LOGE, "Failed to read %s\n", &quasarFieldStr[i][0]);
                }
            }
        }
*/
#endif
    
    //global setup for all cals
    SET_PHY_REG(pMac->hHdd, TPC_TXPWR_ENABLE_REG, 0);   //turn off TPC closed loop control for calibrations

    //save off bandwidth config reg for later restoration
    GET_PHY_REG(pMac->hHdd, AGC_BANDWIDTH_CONFIG_REG, &bandwidthConfig);
#ifdef FIXME_GEN5	
    SET_PHY_REG(pMac->hHdd, AGC_BANDWIDTH_CONFIG_REG, AGC_SINGLE_CHANNEL_CENTERED);
#endif
    if ((retVal = asicTxFirSetIqImbalanceBypass(pMac, eANI_BOOLEAN_FALSE)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    if ((retVal = asicSetAGCCrossbar(pMac, AGC_ALL_RX)) != eHAL_STATUS_SUCCESS) { return (retVal); }

    // Disable AGC packet types from detecting false triggers
    if ((retVal = asicGetDisabledRxPacketTypes(pMac, &agcDisModeReg)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    if ((retVal = asicSetDisabledRxPacketTypes(pMac, PHY_RX_DISABLE_ALL_TYPES)) != eHAL_STATUS_SUCCESS) { return (retVal); }

    if ((retVal = rdModWrNovaField(pMac, RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, 
                                   RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_CAL_MASK, 
                                   RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_CAL_OFFSET, 
                                   1
                                  )
        ) != eHAL_STATUS_SUCCESS
       )
    {
        return (retVal);
    }


    switch (calId)
    {
        case RX_DCO_CAL_ONLY:
            if ((retVal = phyInitRxDcoCal(pMac)) != eHAL_STATUS_SUCCESS) { return (retVal); }
            break;

        case TX_LO_CAL_ONLY:
            phyLog(pMac, LOG1, "Starting TxLo ...\n");
            if ((retVal = phyInitTxCarrierSuppression(pMac)) != eHAL_STATUS_SUCCESS) { return (retVal); }
            phyLog(pMac, LOG1, "Finished TxLo ...\n");
            break;

        case ALL_CALS:
            phyLog(pMac, LOG1, "Starting Rx Dco ...\n");
            if ((retVal = phyInitRxDcoCal(pMac)) != eHAL_STATUS_SUCCESS) { return (retVal); }
            phyLog(pMac, LOG1, "Finished Rx Dco ...\n");

            phyLog(pMac, LOG1, "Starting TxLo ...\n");
            if ((retVal = phyInitTxCarrierSuppression(pMac)) != eHAL_STATUS_SUCCESS) { return (retVal); }
            phyLog(pMac, LOG1, "Finished TxLo ...\n");
            break;

        default:
            phyLog(pMac, LOG1, "No Calibration performed\n");
            break;
    }

    if ((retVal = rdModWrNovaField(pMac, RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, 
                                   RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_CAL_MASK, 
                                   RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_CAL_OFFSET, 
                                   0
                                  )
        ) != eHAL_STATUS_SUCCESS
       )
    {
        return (retVal);
    }

    //global return to previous state from all cals
     //return to closed loop - this is gated for mfg by a test flag
    if ((retVal = asicTPCAutomatic(pMac)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    SET_PHY_REG(pMac->hHdd, AGC_BANDWIDTH_CONFIG_REG, bandwidthConfig);

    //restores all chain selection settings, including crossbar
    if ((retVal = asicTxFirSetIqImbalanceBypass(pMac, eANI_BOOLEAN_TRUE)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    if ((retVal = halPhySetChainSelect(pMac, halPhyGetActiveChainSelect(pMac))) != eHAL_STATUS_SUCCESS) { return (retVal); }
    if ((retVal = asicSetDisabledRxPacketTypes(pMac, agcDisModeReg)) != eHAL_STATUS_SUCCESS) { return (retVal); }

#ifdef ANI_PHY_DEBUG
    //keep this here for debugging
/*
        testLogPhyWatchList(pMac);
        {
            tANI_U32 i;
            for (i = 0; i < NUM_QUASAR_FIELDS; i++)
            {
                tANI_U32 value;
        
                if (eHAL_STATUS_SUCCESS == rfReadQuasarField(pMac, i, &value))
                {
                    phyLog(pMac, LOGE, "%d:  %s = 0x%08X\n", i, &quasarFieldStr[i][0], value);
                }
                else
                {
                    phyLog(pMac, LOGE, "Failed to read %s\n", &quasarFieldStr[i][0]);
                }
            }
        }
*/
#endif

    return (retVal);
}




/* During our initial card bringup, we have stored correction values as a precaution in case run-time calibrations don't work.
    This function interpolates these value for the desired frequency and loads them into the asic.
*/

eHalStatus phyCalFromBringupTables(tpAniSirGlobal pMac, tANI_U16 freq)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
    tANI_U8 loIndex;
    tANI_U8 hiIndex;
    tANI_S32 x1;
    tANI_S32 y1;
    tANI_S32 x2;
    tANI_S32 y2;
    tANI_S32 x;


    if (pMac->hphy.phy.phyUseBringupTables == eANI_BOOLEAN_FALSE)
    {
        return (eHAL_STATUS_SUCCESS);   //no cal from bringup tables
    }
    else if (/*(pMac->hphy.phy.test.testDisableSpiAccess == eANI_BOOLEAN_FALSE) &&*/    //only RF-capable boards can use the bringup tables
             pMac->hphy.eepromSize == HAL_EEPROM_SIZE_32K &&   
             pMac->hphy.phy.txIQTable != NULL &&
             pMac->hphy.phy.rxIQTable != NULL &&
             pMac->hphy.phy.txLoCorrectTable != NULL &&
             pMac->hphy.phy.rxDcoTable != NULL &&
             pMac->hphy.phy.demoFreqs != NULL &&
             pMac->hphy.phy.lnaSwGainTable != NULL
            )
    {
        //all the bringup tables are in EEPROM so use them

        if ((retVal = FindEncompassingFreqs(freq, (void *)pMac->hphy.phy.demoFreqs, 0, sizeof(tANI_U32), NUM_DEMO_CAL_CHANNELS, &loIndex, &hiIndex)) == eHAL_STATUS_SUCCESS)
        {
            //all interpolations below are between the lo and high frequencies
            x = freq;
            x1 = pMac->hphy.phy.demoFreqs[loIndex];
            x2 = pMac->hphy.phy.demoFreqs[hiIndex];

            //if these tables are configured, the respective pointers will be set prior to setting a channel
            if (pMac->hphy.phy.txIQTable)
            {
                sIQCalValues correct;
                tANI_U32 txChain;
                tANI_U32 gain;

                for (txChain = PHY_TX_CHAIN_0; txChain < PHY_MAX_TX_CHAINS; txChain++)
                {
                    for (gain = 0; gain < NUM_TX_GAIN_STEPS; gain++)
                    {
                        //interpolate between lo and high
                        y1 = (tANI_S32)pMac->hphy.phy.txIQTable[loIndex][txChain][gain].center;
                        y2 = (tANI_S32)pMac->hphy.phy.txIQTable[hiIndex][txChain][gain].center;
                        correct.center = (tANI_S9)InterpolateBetweenPoints(x1, y1, x2, y2, x);

                        y1 = (tANI_S32)pMac->hphy.phy.txIQTable[loIndex][txChain][gain].offCenter;
                        y2 = (tANI_S32)pMac->hphy.phy.txIQTable[hiIndex][txChain][gain].offCenter;
                        correct.offCenter = (tANI_S9)InterpolateBetweenPoints(x1, y1, x2, y2, x);

                        y1 = (tANI_S32)pMac->hphy.phy.txIQTable[loIndex][txChain][gain].imbalance;
                        y2 = (tANI_S32)pMac->hphy.phy.txIQTable[hiIndex][txChain][gain].imbalance;
                        correct.imbalance = (tANI_S9)InterpolateBetweenPoints(x1, y1, x2, y2, x);

                        //set interpolated value for this frequency, txchain, & gain
                        if ((retVal = asicTxFirSetTxPhaseCorrection(pMac, (eGainSteps)gain, (ePhyTxChains)txChain, correct)) != eHAL_STATUS_SUCCESS)
                        {
                            return (retVal);
                        }
                    }
                }
                //disable the TxFirIqImbBypass only for AEVB with bringup tables loaded in EEPROM
                if ((pMac->hphy.phy.phyUseBringupTables == eANI_BOOLEAN_TRUE) && (pMac->hphy.phy.test.testDisableSpiAccess == eANI_BOOLEAN_TRUE))
                {
                    if ((retVal = asicTxFirSetIqImbalanceBypass(pMac, eANI_BOOLEAN_FALSE)) != eHAL_STATUS_SUCCESS) { return (retVal); }
                }
            }

            if (pMac->hphy.phy.rxIQTable)
            {
                sIQCalValues correct;
                tANI_U32 rxChain;
                tANI_U32 gain;

                for (rxChain = PHY_RX_CHAIN_0; rxChain < PHY_MAX_RX_CHAINS; rxChain++)
                {
                    for (gain = 0; gain < NUM_RX_GAIN_STEPS; gain++)
                    {
                        //interpolate between lo and high
                        y1 = (tANI_S32)pMac->hphy.phy.rxIQTable[loIndex][rxChain][gain].center;
                        y2 = (tANI_S32)pMac->hphy.phy.rxIQTable[hiIndex][rxChain][gain].center;
                        correct.center = (tANI_S9)InterpolateBetweenPoints(x1, y1, x2, y2, x);

                        y1 = (tANI_S32)pMac->hphy.phy.rxIQTable[loIndex][rxChain][gain].offCenter;
                        y2 = (tANI_S32)pMac->hphy.phy.rxIQTable[hiIndex][rxChain][gain].offCenter;
                        correct.offCenter = (tANI_S9)InterpolateBetweenPoints(x1, y1, x2, y2, x);

                        y1 = (tANI_S32)pMac->hphy.phy.rxIQTable[loIndex][rxChain][gain].imbalance;
                        y2 = (tANI_S32)pMac->hphy.phy.rxIQTable[hiIndex][rxChain][gain].imbalance;
                        correct.imbalance = (tANI_S9)InterpolateBetweenPoints(x1, y1, x2, y2, x);

                        //set interpolated value for this frequency, rxchain, & gain
                        if ((retVal = asicWriteRxPhaseCorrection(pMac, (eGainSteps)gain, (ePhyRxChains)rxChain, correct)) != eHAL_STATUS_SUCCESS)
                        {
                            return (retVal);
                        }
                    }
                }
            }

            if (pMac->hphy.phy.txLoCorrectTable)
            {
                sTxFirLoCorrect correct;
                tANI_U32 txChain;
                tANI_U32 gain;

                for (txChain = PHY_TX_CHAIN_0; txChain < PHY_MAX_TX_CHAINS; txChain++)
                {
                    for (gain = 0; gain < NUM_TX_GAIN_STEPS; gain++)
                    {
                        //interpolate between lo and high
                        y1 = (tANI_S32)pMac->hphy.phy.txLoCorrectTable[loIndex][txChain][gain].iLo;
                        y2 = (tANI_S32)pMac->hphy.phy.txLoCorrectTable[hiIndex][txChain][gain].iLo;
                        correct.iLo = (tANI_S6)InterpolateBetweenPoints(x1, y1, x2, y2, x);

                        y1 = (tANI_S32)pMac->hphy.phy.txLoCorrectTable[loIndex][txChain][gain].qLo;
                        y2 = (tANI_S32)pMac->hphy.phy.txLoCorrectTable[hiIndex][txChain][gain].qLo;
                        correct.qLo = (tANI_S6)InterpolateBetweenPoints(x1, y1, x2, y2, x);

                        //set interpolated value for this frequency, txchain, & gain
                        if ((retVal = asicTxFirSetTxCarrierCorrection(pMac, (eGainSteps)gain, (ePhyTxChains)txChain, correct)) != eHAL_STATUS_SUCCESS)
                        {
                            return (retVal);
                        }
                    }
                }
            }

/*      Betta values aren't working because script is run without proper shielding!
                if (pMac->hphy.phy.rxDcoTable)
                {
                    tRxDcoCorrect correct;
                    tANI_U32 rxChain;
                    tANI_U32 gain;

                    correct.dcRange = 1;    //always 1 for 133us

                    for (rxChain = PHY_RX_CHAIN_0; rxChain < PHY_MAX_RX_CHAINS; rxChain++)
                    {
                        for (gain = 0; gain < NUM_QUASAR_RX_GAIN_STEPS; gain++)
                        {
                            //interpolate between lo and high
                            y1 = (tANI_S32)pMac->hphy.phy.rxDcoTable[loIndex][rxChain][gain].IDcoCorrect;
                            y2 = (tANI_S32)pMac->hphy.phy.rxDcoTable[hiIndex][rxChain][gain].IDcoCorrect;
                            correct.IDcoCorrect = (tANI_S6)InterpolateBetweenPoints(x1, y1, x2, y2, x);
    
                            y1 = (tANI_S32)pMac->hphy.phy.rxDcoTable[loIndex][rxChain][gain].QDcoCorrect;
                            y2 = (tANI_S32)pMac->hphy.phy.rxDcoTable[hiIndex][rxChain][gain].QDcoCorrect;
                            correct.QDcoCorrect = (tANI_S6)InterpolateBetweenPoints(x1, y1, x2, y2, x);
    
                            //set interpolated value for this frequency, txchain, & gain
                            if ((retVal = rfSetDCOffset(pMac, rxChain, gain, correct)) != eHAL_STATUS_SUCCESS)
                            {
                                return (retVal);
                            }
                        }
                    }
                }
*/

        }
    }

    return (retVal);
}



eHalStatus phyPeriodicCal(tpAniSirGlobal pMac, eInitCals calId)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
    ePhyRxDisabledPktTypes agcDisModeReg;
    tANI_U32 bandwidthConfig;
    tTxGain  txGain;

#ifdef ANI_PHY_DEBUG
    //keep this here for debugging
/*
        testLogPhyWatchList(pMac);
        {
            tANI_U32 i;
            for (i = 0; i < NUM_QUASAR_FIELDS; i++)
            {
                tANI_U32 value;
        
                if (eHAL_STATUS_SUCCESS == rfReadQuasarField(pMac, i, &value))
                {
                    phyLog(pMac, LOGE, "%d:  %s = 0x%08X\n", i, &quasarFieldStr[i][0], value);
                }
                else
                {
                    phyLog(pMac, LOGE, "Failed to read %s\n", &quasarFieldStr[i][0]);
                }
            }
        }
*/
#endif
    
    //global setup for all cals
    SET_PHY_REG(pMac->hHdd, TPC_TXPWR_ENABLE_REG, 0);   //turn off TPC closed loop control for calibrations

    //save off bandwidth config reg for later restoration
    GET_PHY_REG(pMac->hHdd, AGC_BANDWIDTH_CONFIG_REG, &bandwidthConfig);
#ifdef FIXME_GEN5	
    SET_PHY_REG(pMac->hHdd, AGC_BANDWIDTH_CONFIG_REG, AGC_SINGLE_CHANNEL_CENTERED);
#endif
    if ((retVal = asicTxFirSetIqImbalanceBypass(pMac, eANI_BOOLEAN_FALSE)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    if ((retVal = asicSetAGCCrossbar(pMac, AGC_ALL_RX)) != eHAL_STATUS_SUCCESS) { return (retVal); }

    // Disable AGC packet types from detecting false triggers
    if ((retVal = asicGetDisabledRxPacketTypes(pMac, &agcDisModeReg)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    if ((retVal = asicSetDisabledRxPacketTypes(pMac, PHY_RX_DISABLE_ALL_TYPES)) != eHAL_STATUS_SUCCESS) { return (retVal); }

    if ((retVal = rdModWrNovaField(pMac, RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, 
                                   RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_CAL_MASK, 
                                   RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_CAL_OFFSET, 
                                   1
                                  )
        ) != eHAL_STATUS_SUCCESS
       )
    {
        return (retVal);
    }


    switch (calId)
    {
        case RX_DCO_CAL_ONLY:
            
            //if ((retVal = phyInitRxDcoCal(pMac)) != eHAL_STATUS_SUCCESS) { return (retVal); }
            if ((retVal = phyPeriodicDcoCalRxChain(pMac, PHY_ALL_RX_CHAINS, QUASAR_GAIN_MAX_INDEX)) != eHAL_STATUS_SUCCESS) { return (retVal); }
            break;

        case TX_LO_CAL_ONLY:
            phyLog(pMac, LOG1, "Starting Periodic TxLo ...\n");
            txGain.coarsePwr = TX_GAIN_STEP_7;
            txGain.finePwr = 0;
            if ((retVal = phyPeriodicTxCarrierSuppression(pMac, PHY_ALL_TX_CHAINS, txGain)) != eHAL_STATUS_SUCCESS) { return (retVal); }
            phyLog(pMac, LOG1, "Finished Periodic TxLo ...\n");
            break;

        case ALL_CALS:
            phyLog(pMac, LOG1, "Starting Periodic Rx Dco ...\n");
            if ((retVal = phyPeriodicDcoCalRxChain(pMac, PHY_ALL_RX_CHAINS, QUASAR_GAIN_MAX_INDEX)) != eHAL_STATUS_SUCCESS) { return (retVal); }
            phyLog(pMac, LOG1, "Finished Periodic Rx Dco ...\n");

            phyLog(pMac, LOG1, "Starting Periodic TxLo ...\n");
            txGain.coarsePwr = TX_GAIN_STEP_7;
            txGain.finePwr = 0;
            if ((retVal = phyPeriodicTxCarrierSuppression(pMac, PHY_ALL_TX_CHAINS, txGain)) != eHAL_STATUS_SUCCESS) { return (retVal); }
            phyLog(pMac, LOG1, "Finished Periodic TxLo ...\n");
            break;
        default:
            phyLog(pMac, LOG1, "No Calibration performed\n");
            break;
    }

    if ((retVal = rdModWrNovaField(pMac, RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, 
                                   RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_CAL_MASK, 
                                   RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_CAL_OFFSET, 
                                   0
                                  )
        ) != eHAL_STATUS_SUCCESS
       )
    {
        return (retVal);
    }

    //global return to previous state from all cals
     //return to closed loop - this is gated for mfg by a test flag
    if ((retVal = asicTPCAutomatic(pMac)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    SET_PHY_REG(pMac->hHdd, AGC_BANDWIDTH_CONFIG_REG, bandwidthConfig);

    //restores all chain selection settings, including crossbar
    if ((retVal = asicTxFirSetIqImbalanceBypass(pMac, eANI_BOOLEAN_TRUE)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    if ((retVal = halPhySetChainSelect(pMac, halPhyGetActiveChainSelect(pMac))) != eHAL_STATUS_SUCCESS) { return (retVal); }
    if ((retVal = asicSetDisabledRxPacketTypes(pMac, agcDisModeReg)) != eHAL_STATUS_SUCCESS) { return (retVal); }

#ifdef ANI_PHY_DEBUG
    //keep this here for debugging
/*
        testLogPhyWatchList(pMac);
        {
            tANI_U32 i;
            for (i = 0; i < NUM_QUASAR_FIELDS; i++)
            {
                tANI_U32 value;
        
                if (eHAL_STATUS_SUCCESS == rfReadQuasarField(pMac, i, &value))
                {
                    phyLog(pMac, LOGE, "%d:  %s = 0x%08X\n", i, &quasarFieldStr[i][0], value);
                }
                else
                {
                    phyLog(pMac, LOGE, "Failed to read %s\n", &quasarFieldStr[i][0]);
                }
            }
        }
*/
#endif

    return (retVal);
}


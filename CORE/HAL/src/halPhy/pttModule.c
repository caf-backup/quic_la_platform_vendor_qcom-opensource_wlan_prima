/**
 *
   Airgo Networks, Inc proprietary.
   All Rights Reserved, Copyright 2006
   This program is the confidential and proprietary product of Airgo Networks Inc.
   Any Unauthorized use, reproduction or transfer of this program is strictly prohibited.

 */

#include <string.h>
#include "sys_api.h"


#define SUCCESS     PTT_STATUS_SUCCESS
#define FAILURE     PTT_STATUS_FAILURE

#define TAURUS_A3   //3/15/07


#if defined(ANI_PHY_DEBUG) || defined(ANI_MANF_DIAG)
#include "pttModuleApi.h"

//Device Register Access
ePttStatus pttDbgReadRegister(tpAniSirGlobal pMac, tANI_U32 regAddr, tANI_U32 *regValue)
{
    if (halReadRegister(pMac, regAddr, regValue) != eHAL_STATUS_SUCCESS)
    {
        return (FAILURE);
    }

    return (SUCCESS);
}


ePttStatus pttDbgWriteRegister(tpAniSirGlobal pMac, tANI_U32 regAddr, tANI_U32 regValue)
{
    if (halWriteRegister(pMac, regAddr, regValue) != eHAL_STATUS_SUCCESS)
    {
        return (FAILURE);
    }

    return (SUCCESS);
}

#endif



#ifdef ANI_MANF_DIAG
#include "pttModuleApi.h"



const tANI_U8 defaultSourceMacAddress[ANI_MAC_ADDR_SIZE] = { 0x22, 0x22, 0x44, 0x44, 0x33, 0x33 };  //STA
const tANI_U8 defaultDestMacAddress[ANI_MAC_ADDR_SIZE] =   { 0x22, 0x22, 0x11, 0x11, 0x33, 0x33 };  //AP
const tANI_U8 defaultBssIdMacAddress[ANI_MAC_ADDR_SIZE] =  { 0x22, 0x22, 0x11, 0x11, 0x33, 0x33 };

static ePttStatus NormalizeTpcChainData(tpAniSirGlobal pMac, tTpcCalPoint *chainData, tANI_U16 numTpcCalPointsPerFreq, tTpcCaldPowerPoint tpcChainCfg[MAX_TPC_CAL_POINTS]);
static eHalStatus VerifyLnaSwTableData(tpAniSirGlobal pMac, sLnaSwGainTable *tableData);



sAgcGainLut pttAgcGainLUT[PHY_MAX_RX_CHAINS];



tANI_U8 defaultAddr1[ANI_MAC_ADDR_SIZE] = { 0x00, 0x77, 0x55, 0x33, 0x11, 0x00 };   //dest
tANI_U8 defaultAddr3[ANI_MAC_ADDR_SIZE] = { 0x00, 0x77, 0x55, 0x33, 0x11, 0x00 };   //bssId

void pttModuleInit(tpAniSirGlobal pMac)
{
    uEepromFields fields;
    
    
    pMac->hphy.phy.test.testTpcClosedLoop = eANI_BOOLEAN_FALSE;
    pMac->hphy.phy.test.testTxGainIndexSource = RATE_POWER_NON_LIMITED;
    pMac->hphy.phy.test.testForcedTxPower = 0;
    pMac->hphy.phy.test.testLastPwrIndex = 0;
    pMac->hphy.phy.test.testForcedTxGainIndex = 0;

    pMac->ptt.forcedTxGain[PHY_TX_CHAIN_0].coarsePwr = TPC_COARSE_TXPWR_POS_20_DBM;
    pMac->ptt.forcedTxGain[PHY_TX_CHAIN_0].finePwr = TPC_FINE_TXPWR_7;
    pMac->ptt.forcedTxGain[PHY_TX_CHAIN_1].coarsePwr = TPC_COARSE_TXPWR_POS_20_DBM;
    pMac->ptt.forcedTxGain[PHY_TX_CHAIN_1].finePwr = TPC_FINE_TXPWR_7;

    //initialize frame gen variables
    pMac->ptt.frameGenEnabled = eANI_BOOLEAN_FALSE;
    pMac->ptt.phyDbgFrameGen = eANI_BOOLEAN_TRUE;

    pMac->ptt.frameGenParams.numTestPackets = 0;   //Continuous
    pMac->ptt.frameGenParams.interFrameSpace = 10;
    pMac->ptt.frameGenParams.rate = HAL_PHY_RATE_SSF_SIMO_6_MBPS;
    pMac->ptt.frameGenParams.payloadContents = TEST_PAYLOAD_RANDOM;
    pMac->ptt.frameGenParams.payloadLength = MAX_PAYLOAD_SIZE;
    pMac->ptt.frameGenParams.payloadFillByte = 0xA5;
    pMac->ptt.frameGenParams.pktAutoSeqNum = eANI_BOOLEAN_FALSE;
    pMac->ptt.frameGenParams.pktScramblerSeed = 7;
    pMac->ptt.frameGenParams.crc = 0;
    pMac->ptt.frameGenParams.preamble = PHYDBG_PREAMBLE_OFDM;
    memcpy(&pMac->ptt.frameGenParams.addr1[0], defaultAddr1, ANI_MAC_ADDR_SIZE);
    
    if (eHAL_STATUS_SUCCESS == halReadEepromField(pMac, EEPROM_COMMON_MAC_ADDR, &fields))
    {
        memcpy(&pMac->ptt.frameGenParams.addr2[0], &fields, ANI_MAC_ADDR_SIZE);
    }
    else
    {
        memset(&pMac->ptt.frameGenParams.addr2[0], 0xFF, ANI_MAC_ADDR_SIZE);
    }
    memcpy(&pMac->ptt.frameGenParams.addr3[0], defaultAddr3, ANI_MAC_ADDR_SIZE);


    pMac->ptt.rx0AgcEnabled = eANI_BOOLEAN_TRUE;
    pMac->ptt.rx1AgcEnabled = eANI_BOOLEAN_TRUE;
    pMac->ptt.rx2AgcEnabled = eANI_BOOLEAN_TRUE;
    pMac->ptt.wfmEnabled = eANI_BOOLEAN_FALSE;
    pMac->ptt.wfmStored = eANI_BOOLEAN_FALSE;

    // store rx agc gain luts for restoration later
    {

        asicGetAgcGainLut(pMac, PHY_RX_CHAIN_0, pttAgcGainLUT[PHY_RX_CHAIN_0]);
        asicGetAgcGainLut(pMac, PHY_RX_CHAIN_1, pttAgcGainLUT[PHY_RX_CHAIN_1]);
        asicGetAgcGainLut(pMac, PHY_RX_CHAIN_2, pttAgcGainLUT[PHY_RX_CHAIN_2]);
    }

    /*! moved this from pttMsgInit, so we need to verify the init sequence with and without PTT GUI
        The idea is to have pttModuleInit initialize everything for manufacturing, and then pttMsgInit
        allows PTT GUI to get the initialized values.
        Anything that should be reinitialized or only done when PTT GUI is connecting can be done in pttMsgInit
    */

    pMac->hphy.phy.phyPeriodicCalEnable = eANI_BOOLEAN_FALSE; // ignore periodic calibrations.

    //always leave phydbg clock on for ptt
    rdModWrNovaField(pMac, RXCLKCTRL_APB_BLOCK_CLK_EN_REG, 
                           RXCLKCTRL_APB_BLOCK_CLK_EN_PHYDBG_MASK, 
                           RXCLKCTRL_APB_BLOCK_CLK_EN_PHYDBG_OFFSET, 
                           1
                    );

    // Clear stats
    pttResetRxPacketStatistics(pMac);

}


// Msg Init - preparing for starting PTT and also providing default values back to GUI
ePttStatus pttMsgInit(tpAniSirGlobal pMac, tPttModuleVariables** ptt)
{
    phyLog(pMac, LOGE, "pttMsgInit: initialize PTT. \n");

    pttModuleInit(pMac);    //reinitialize when this message is received
    *ptt = &pMac->ptt;

    return SUCCESS;
}



//EEPROM Service
ePttStatus pttGetTpcCalState(tpAniSirGlobal pMac, eTpcCalState* calState)
{
    *calState = TPC_NOT_CALIBRATED;

    if ((halIsTableInEeprom(pMac, EEPROM_TABLE_TPC_PARAMS) == eHAL_STATUS_SUCCESS) &&
        (halIsTableInEeprom(pMac, EEPROM_TABLE_TPC_CONFIG) == eHAL_STATUS_SUCCESS)
       )
    {
        *calState = TPC_CALIBRATION_COMPLETE;
    }
    else
    {
        if ((halIsTableInEeprom(pMac, EEPROM_TABLE_2_4_TPC_PARAMS) == eHAL_STATUS_SUCCESS) &&
            (halIsTableInEeprom(pMac, EEPROM_TABLE_2_4_TPC_CONFIG) == eHAL_STATUS_SUCCESS)
           )
        {
            //2.4 GHZ Configured
            *calState = TPC_2_4GHZ_CALIBRATION_STORED;
        }

        if ((halIsTableInEeprom(pMac, EEPROM_TABLE_5_TPC_PARAMS) == eHAL_STATUS_SUCCESS) &&
            (halIsTableInEeprom(pMac, EEPROM_TABLE_5_TPC_CONFIG) == eHAL_STATUS_SUCCESS)
           )
        {
            //5 GHZ Configured
            if (*calState == TPC_2_4GHZ_CALIBRATION_STORED)
            {
                *calState = TPC_CALIBRATION_COMPLETE;
            }
            else
            {
                *calState = TPC_5GHZ_CALIBRATED_STORED;
            }
        }
    }
    
    return (SUCCESS);

}


ePttStatus pttResetTpcCalState(tpAniSirGlobal pMac, eTpcCalState *calState)
{

    //need to erase TPC tables and other calibrations
    switch (*calState)
    {
        case TPC_CALIBRATION_COMPLETE:
            halRemoveEepromTable(pMac, EEPROM_TABLE_2_4_TPC_PARAMS);
            halRemoveEepromTable(pMac, EEPROM_TABLE_2_4_TPC_CONFIG);
            halRemoveEepromTable(pMac, EEPROM_TABLE_5_TPC_PARAMS);
            halRemoveEepromTable(pMac, EEPROM_TABLE_5_TPC_CONFIG);
            halRemoveEepromTable(pMac, EEPROM_TABLE_TPC_PARAMS);
            halRemoveEepromTable(pMac, EEPROM_TABLE_TPC_CONFIG);
            break;

        case TPC_2_4GHZ_CALIBRATION_STORED:
            halRemoveEepromTable(pMac, EEPROM_TABLE_2_4_TPC_PARAMS);
            halRemoveEepromTable(pMac, EEPROM_TABLE_2_4_TPC_CONFIG);
            break;

        case TPC_5GHZ_CALIBRATED_STORED:
            halRemoveEepromTable(pMac, EEPROM_TABLE_5_TPC_PARAMS);
            halRemoveEepromTable(pMac, EEPROM_TABLE_5_TPC_CONFIG);
            break;

        case TPC_NOT_CALIBRATED:
            return SUCCESS;
            break;
    }

    //now read TPC calibration from EEPROM to make it take effect immediately
    if (eHAL_STATUS_SUCCESS != ConfigureTpcFromEeprom(pMac))
    {
        return (FAILURE);
    }

    return SUCCESS;
}


//this function calculates the designated checksum, stores it, and returns the value stored
ePttStatus pttSetEepromCksum(tpAniSirGlobal pMac, tANI_U32 *cksum, tANI_BOOLEAN isFixPart)
{
    eHalStatus rc = eHAL_STATUS_FAILURE;

    rc = halSetEepromCksum(pMac, cksum, isFixPart);

    if (eHAL_STATUS_SUCCESS == rc)
    {
       return (SUCCESS);
    }
    else
    {
        return (FAILURE);
    }

}


ePttStatus pttGetEepromCksum(tpAniSirGlobal pMac, tANI_U32 *cksum, tANI_U32 *computedCksum, tANI_BOOLEAN isFixPart)
{
    eHalStatus rc = eHAL_STATUS_FAILURE;

    rc = halGetEepromCksum(pMac, cksum, computedCksum, isFixPart);


    if (eHAL_STATUS_SUCCESS == rc)
    {
        return (SUCCESS);
    }
    else
    {
        return (FAILURE);
    }
}


ePttStatus pttGetEepromTable(tpAniSirGlobal pMac, eEepromTable eepromTable, uEepromTables *tableData)
{
    eHalStatus rc = eHAL_STATUS_FAILURE;

    rc = halReadEepromTable(pMac, eepromTable, tableData);
    if (eHAL_STATUS_SUCCESS == rc)
    {
        return (SUCCESS);
    }
    else
    {
        return (FAILURE);
    }
}



#define NORMAL_CHANNEL_SETTING     0xFF     //same as in halPhy.c

ePttStatus pttSetEepromTable(tpAniSirGlobal pMac, eEepromTable eepromTable, uEepromTables *tableData)
{

    if ((tANI_U32)eepromTable >= NUM_EEPROM_TABLE_IDS)
    {
        return (FAILURE);
    }


    {
        /* We want this table to take effect immediately for mfg purposes. Otherwise, the adapter has to be
            reset for these values to take effect per initialization.
        */
        //Since all phy tables are operating on the cache,
        // just retune the channel and any tables
        switch (eepromTable)
        {
            case EEPROM_TABLE_LNA_SW_GAIN:
                if ((pMac->hphy.phy.test.testChannelId == NORMAL_CHANNEL_SETTING) &&
                    (pMac->hphy.rf.curChannel != INVALID_RF_CHANNEL)
                   )
                {
                    pMac->hphy.phy.test.testChannelId = pMac->hphy.rf.curChannel;
                    pMac->hphy.phy.test.testCbState = PHY_SINGLE_CHANNEL_CENTERED;
                }

                if (pMac->hphy.phy.test.testChannelId == NORMAL_CHANNEL_SETTING)
                {
                    phyLog(pMac, LOGE, "ERROR: Current channel invalid!  EEPROM_TABLE_LNA_SW_GAIN not stored!\n", (tANI_U32)eepromTable);
                    return (FAILURE);
                }

#ifndef ANI_PHY_DEBUG
#error ANI_PHY_DEBUG not defined for ANI_MANF_DIAG build
#endif
                if (VerifyLnaSwTableData(pMac, (sLnaSwGainTable *)tableData) != eHAL_STATUS_SUCCESS)
                {
                    return (FAILURE);
                }

                if (eHAL_STATUS_FAILURE == halWriteEepromTable(pMac, eepromTable, tableData))
                {
                    phyLog(pMac, LOGE, "Unable to write table %d\n", (tANI_U32)eepromTable);
                    return (FAILURE);
                }
                
                if (pMac->hphy.phy.test.testDisableSpiAccess == eANI_BOOLEAN_FALSE)
                {
                }
                else
                {
                    //this board is not RF capable so we shouldn't run the initial calibration, or it is being programmed in the Betta ATE setup
                    //in either case, we will expect this and any other tables to be stored through the pttStoreEepromTable() function
                    phyLog(pMac, LOGW, "WARN: Board not configured for RF\n");
                    return (SUCCESS);
                }
                break;

            //tables that would normally be stored for production driver
            case EEPROM_FIELDS_IMAGE:
            case EEPROM_TABLE_REGULATORY_DOMAINS_V2:
            case EEPROM_TABLE_RATE_POWER_SETTINGS:
            case EEPROM_TABLE_RX_NOTCH_FILTER:
            case EEPROM_TABLE_MULT_BSSID:
            case EEPROM_TABLE_TPC_TEMP_COMP:
            case EEPROM_TABLE_QUASAR_FILTERS:
            case EEPROM_TABLE_CAL_TABLES:

            //tables associated with CLPC may be restored using PTT GUI, so allow these to be written and also ConfigureTpcFromEeprom immediately if possible
            //for these, a reboot will be required to make them take effect
            case EEPROM_TABLE_2_4_TPC_PARAMS:
            case EEPROM_TABLE_2_4_TPC_CONFIG:
            case EEPROM_TABLE_5_TPC_PARAMS:
            case EEPROM_TABLE_5_TPC_CONFIG:
            case EEPROM_TABLE_TPC_PARAMS:
            case EEPROM_TABLE_TPC_CONFIG:
                if (eHAL_STATUS_FAILURE == halWriteEepromTable(pMac, eepromTable, tableData))
                {
                    phyLog(pMac, LOGE, "Unable to write table %d\n", (tANI_U32)eepromTable);
                    return (FAILURE);
                }
                else
                {
                    //update table pointers used by phy so they are immediately accessible
                    halGetEepromTableLoc(pMac, EEPROM_TABLE_REGULATORY_DOMAINS_V2, (uEepromTables **)&pMac->hphy.phy.regDomainInfo);
                    halGetEepromTableLoc(pMac, EEPROM_TABLE_RATE_POWER_SETTINGS, (uEepromTables **)&pMac->hphy.phy.pwrOptimal);
                    halGetEepromTableLoc(pMac, EEPROM_TABLE_RX_NOTCH_FILTER, (uEepromTables **)&pMac->hphy.phy.rxNotch);
                    halGetEepromTableLoc(pMac, EEPROM_TABLE_TPC_TEMP_COMP, (uEepromTables **)&pMac->hphy.phyTPC.tpcTempCompTable);
                }
                break;
            

            //tables only stored for Betta brinup purposes
            case EEPROM_TABLE_TX_IQ:
            case EEPROM_TABLE_RX_IQ:
            case EEPROM_TABLE_TX_LO:
            case EEPROM_TABLE_QUASAR_REGS:
            case EEPROM_TABLE_DEMO_CHANNEL_LIST:
            case EEPROM_TABLE_RX_DCO:
                if (pMac->hphy.phy.test.testDisableSpiAccess == eANI_BOOLEAN_FALSE)
                {
                    phyLog(pMac, LOGW, "WARN: Bringup table being programmed while board is configured for full register access!\n");
                }
                
                if (eHAL_STATUS_FAILURE == halWriteEepromTable(pMac, eepromTable, tableData))
                {
                    phyLog(pMac, LOGE, "Unable to write table %d\n", (tANI_U32)eepromTable);
                    return (FAILURE);
                }
                break;
            
            default:
                phyLog(pMac, LOGW, "ERROR: table %d not programmed\n", eepromTable);
                break;
        }
    }

    return (SUCCESS);
}


static eHalStatus VerifyLnaSwTableData(tpAniSirGlobal pMac, sLnaSwGainTable *tableData)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
    tANI_U32 nFreqs;
    // tANI_U8 rxLnaMax = 0;
    // tANI_U8 rxLnaMin = 0xFF;
    // tANI_U8 altMax = 0;
    // ePhyRxChains altGainLimitChain = PHY_NO_RX_CHAINS;
    // ePhyRxChains rxLnaGainLimitChain = PHY_NO_RX_CHAINS;


    nFreqs = tableData->nFreqs;

    if (nFreqs < MAX_LNA_SW_FREQS)
    {
        tANI_U32 i;

        phyLog(pMac,LOGE, "LNA Sw nFreqs = %d\n", nFreqs);

        for (i = 0; i < nFreqs; i++)
        {
            phyLog(pMac,LOGE, "%04d MHz: \tRx0{R+LNA=%03d Alt=%03d} \tRx1{R+LNA=%03d Alt=%03d} \tRx2{R+LNA=%03d Alt=%03d}\n",
                        tableData->chan[i].freq,
                        tableData->chan[i].swGains[PHY_RX_CHAIN_0].rxLnaOnGain,
                        tableData->chan[i].swGains[PHY_RX_CHAIN_0].alternateGain,
                        tableData->chan[i].swGains[PHY_RX_CHAIN_1].rxLnaOnGain,
                        tableData->chan[i].swGains[PHY_RX_CHAIN_1].alternateGain,
                        tableData->chan[i].swGains[PHY_RX_CHAIN_2].rxLnaOnGain,
                        tableData->chan[i].swGains[PHY_RX_CHAIN_2].alternateGain
                  );
            if (INVALID_RF_CHANNEL == rfGetIndexFromFreq(tableData->chan[i].freq, PHY_SINGLE_CHANNEL_CENTERED))
            {
                phyLog(pMac, LOGE, "ERROR: freq %d not recognized as a supported 20MHz channel\n", tableData->chan[i].freq);
                return (eHAL_STATUS_FAILURE);
            }

            if ((i > 0) && (tableData->chan[i].freq <= tableData->chan[i-1].freq))
            {
                phyLog(pMac, LOGE, "ERROR: LNA Sw Gain table must be ordered in ascending frequencies\n", tableData->chan[i].freq);
                return (eHAL_STATUS_FAILURE);
            }

            if ((tableData->chan[i].swGains[PHY_RX_CHAIN_0].rxLnaOnGain <= tableData->chan[i].swGains[PHY_RX_CHAIN_0].alternateGain) ||
                (tableData->chan[i].swGains[PHY_RX_CHAIN_1].rxLnaOnGain <= tableData->chan[i].swGains[PHY_RX_CHAIN_1].alternateGain) 
#if !defined(ANI_BUS_TYPE_USB)                
                || (tableData->chan[i].swGains[PHY_RX_CHAIN_2].rxLnaOnGain <= tableData->chan[i].swGains[PHY_RX_CHAIN_2].alternateGain)
#endif                
               )
            {
                phyLog(pMac, LOGE, "ERROR: rxLnaOnGain must exceed alternateGain measurement\n");
                return (eHAL_STATUS_FAILURE);
            }
        }
    }
    else
    {
        phyLog(pMac,LOGE, "ERROR: LNA nFreqs = %d, Out of range\n", nFreqs);
        return (eHAL_STATUS_FAILURE);
    }

    return (retVal);
}


ePttStatus pttDelEepromTable(tpAniSirGlobal pMac, eEepromTable eepromTable)
{
    eHalStatus rc = eHAL_STATUS_FAILURE;

    rc = halRemoveEepromTable(pMac, eepromTable);
    

    if (eHAL_STATUS_SUCCESS == rc)
    {
        //as a precaution, since we've removed the eeprom table, make sure that any corresponding phy pointer is set back to NULL
        switch (eepromTable)
        {
            case EEPROM_TABLE_QUASAR_FILTERS:
                pMac->hphy.rf.chanFilterSettings = NULL;
                break;
            case EEPROM_TABLE_LNA_SW_GAIN:
                pMac->hphy.phy.lnaSwGainTable = NULL;
                break;
            case EEPROM_TABLE_QUASAR_REGS:
                pMac->hphy.rf.quasarRegsTable = NULL;
                break;
            case EEPROM_TABLE_REGULATORY_DOMAINS_V2:
                pMac->hphy.phy.regDomainInfo = NULL;
                break;
            case EEPROM_TABLE_RATE_POWER_SETTINGS:
                pMac->hphy.phy.pwrOptimal = NULL;
                break;
            case EEPROM_TABLE_RX_NOTCH_FILTER:
                pMac->hphy.phy.rxNotch = NULL;
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




//!this function is purely for bringup purposes - use the pttSetEepromTable and pttSetEepromField function for normal programming
ePttStatus pttSetEepromImage(tpAniSirGlobal pMac, tANI_U32 offset, tANI_U32 len, tANI_U8 *pBuf, tANI_BOOLEAN toCache)
{
    eHalStatus rc = eHAL_STATUS_SUCCESS;

    if(pMac->hphy.eepromSize < (offset + len))
    {
        return (FAILURE);
    }

    if (len % 4 != 0)
    {
        //only allow reading of EEPROM in dword quantities
        return (FAILURE);
    }

    if (toCache == eANI_BOOLEAN_TRUE)
    {
        //copy to cache
        rc = palCopyMemory(pMac->hHdd, (tANI_U8 *)pMac->hphy.pEepromCache + offset, pBuf, len);
    }
    else
    {
        // read binary image from eeprom
        rc = asicNVIWriteBurstData(pMac, offset, (tANI_U32 *)pBuf, len / 4);
    }

    if (eHAL_STATUS_SUCCESS == rc)
    {
       return (SUCCESS);
    }
    else
    {
        return (FAILURE);
    }
}

ePttStatus pttGetEepromImage(tpAniSirGlobal pMac, tANI_U32 offset, tANI_U32 len, tANI_U8 *pBuf, tANI_BOOLEAN fromCache)
{
    eHalStatus rc = eHAL_STATUS_SUCCESS;

    if(pMac->hphy.eepromSize < (offset + len))
    {
        return (FAILURE);
    }

    if (len % 4 != 0)
    {
        //only allow reading of EEPROM in dword quantities
        return (FAILURE);
    }

    if (fromCache == eANI_BOOLEAN_TRUE)
    {
        //copy from cache
        rc = palCopyMemory(pMac->hHdd, pBuf, (tANI_U8 *)pMac->hphy.pEepromCache + offset, len);
    }
    else
    {
        // read binary image from eeprom
        rc = asicNVIReadBurstData(pMac, offset, (tANI_U32 *)pBuf, len / 4);
    }

    if (eHAL_STATUS_SUCCESS == rc)
    {
       return (SUCCESS);
    }
    else
    {
        return (FAILURE);
    }
}

ePttStatus pttBlankEeprom(tpAniSirGlobal pMac)
{
    eHalStatus rc = eHAL_STATUS_FAILURE;

    rc = halBlankEeprom(pMac);
    if (eHAL_STATUS_SUCCESS == rc)
    {
       return (SUCCESS);
    }
    else
    {
        return (FAILURE);
    }
}



ePttStatus pttGetEepromField(tpAniSirGlobal pMac, eEepromField eepromField, uEepromFields *fieldData)
{
    if (eHAL_STATUS_SUCCESS == halReadEepromField(pMac, eepromField, fieldData))
    {
       return (SUCCESS);
    }
    else
    {
        return (FAILURE);
    }
}

ePttStatus pttSetEepromField(tpAniSirGlobal pMac, eEepromField eepromField, uEepromFields *fieldData)
{
    if (eHAL_STATUS_SUCCESS == halWriteEepromField(pMac, eepromField, fieldData))
    {
       return (SUCCESS);
    }
    else
    {
        return (FAILURE);
    }
}

ePttStatus pttStoreEepromTable(tpAniSirGlobal pMac, eEepromTable eepromTable)
{
    if (eHAL_STATUS_SUCCESS == halStoreTableToEeprom(pMac, eepromTable))
    {
        return (SUCCESS);
    }
    else
    {
        return (FAILURE);
    }
}

ePttStatus pttSetRegDomain(tpAniSirGlobal pMac, eRegDomainId regDomainId)
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


ePttStatus pttApiWriteRegister(tpAniSirGlobal pMac, eApiRegister regId, tANI_U32 regValue)
{
    tANI_U32 regAddr;

    switch (regId)
    {
        case eREG_RX_INIT_GAIN:
            regAddr = AGC_INIT_GAIN_REG;
            break;
        case eREG_RX_COARSE_STEP:
            regAddr = AGC_COARSE_STEP_REG;
            break;
        case eREG_TARGET_BO_ACTIVE:
            regAddr = AGC_TARGET_BO_ACTIVE_REG;
            break;
        case eREG_TH_CD:
            regAddr = AGC_TH_CD_REG;
            break;
        case eREG_CW_DETECT:
            regAddr = AGC_CW_DETECT_REG;
            break;
        default:
            return(FAILURE);
            break;
    }

    return (pttDbgWriteRegister(pMac, regAddr, regValue));
}

ePttStatus pttApiReadRegister(tpAniSirGlobal pMac, eApiRegister regId, tANI_U32 *regValue)
{
    tANI_U32 regAddr;

    switch (regId)
    {
        case eREG_RX_INIT_GAIN:
            regAddr = AGC_INIT_GAIN_REG;
            break;
        case eREG_RX_COARSE_STEP:
            regAddr = AGC_COARSE_STEP_REG;
            break;
        case eREG_TARGET_BO_ACTIVE:
            regAddr = AGC_TARGET_BO_ACTIVE_REG;
            break;
        case eREG_TH_CD:
            regAddr = AGC_TH_CD_REG;
            break;
        case eREG_CW_DETECT:
            regAddr = AGC_CW_DETECT_REG;
            break;
        default:
            return(FAILURE);
            break;
    }

    return (pttDbgReadRegister(pMac, regAddr, regValue));
}


ePttStatus pttDbgReadMemory(tpAniSirGlobal pMac, tANI_U32 memAddr, tANI_U32 nBytes, tANI_U32 *pMemBuf)
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


ePttStatus pttDbgWriteMemory(tpAniSirGlobal pMac, tANI_U32 memAddr, tANI_U32 nBytes, tANI_U32 *pMemBuf)
{
    //TODO: add bounds checking on pttDbgWriteMemory params
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
ePttStatus pttSetChannel(tpAniSirGlobal pMac, tANI_U32 chId, ePhyChanBondState cbState)
{
    ePttStatus rc = SUCCESS;
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

    if (!(cbState == PHY_SINGLE_CHANNEL_CENTERED ||
          cbState == PHY_DOUBLE_CHANNEL_LOW_PRIMARY ||
          cbState == PHY_DOUBLE_CHANNEL_HIGH_PRIMARY) ||
         (halPhySetChannel(pMac, (tANI_U8)chId, (ePhyChanBondState)cbState) != eHAL_STATUS_SUCCESS)
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
        
        // store rx agc gain luts for restoration later - these are frequency specific
        {
            asicGetAgcGainLut(pMac, PHY_RX_CHAIN_0, pttAgcGainLUT[PHY_RX_CHAIN_0]);
            asicGetAgcGainLut(pMac, PHY_RX_CHAIN_1, pttAgcGainLUT[PHY_RX_CHAIN_1]);
            asicGetAgcGainLut(pMac, PHY_RX_CHAIN_2, pttAgcGainLUT[PHY_RX_CHAIN_2]);
        }

        if ((frameGenEnabled  == eANI_BOOLEAN_TRUE) && (pMac->ptt.phyDbgFrameGen == eANI_BOOLEAN_TRUE))
        {
            if ((oldCbState != PHY_SINGLE_CHANNEL_CENTERED) && (cbState == PHY_SINGLE_CHANNEL_CENTERED))
            {
                //going from 40 to 20MHz - if rate is not compatible then don't restart
                if (TEST_PHY_RATE_IS_CB(pMac->ptt.frameGenParams.rate) == eANI_BOOLEAN_TRUE)
                {
                    //channel bonded rates not supported on 20MHz channel
                    return (rc);
                }
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


ePttStatus pttEnableChains(tpAniSirGlobal pMac, ePhyChainSelect chainSelection)
{
    tANI_BOOLEAN stopTx = eANI_BOOLEAN_FALSE;
    tANI_U8 rxMask = 0xFF;
    tANI_BOOLEAN frameGenEnabled = pMac->ptt.frameGenEnabled;

    if (frameGenEnabled && (pMac->ptt.phyDbgFrameGen))
    {
        //stop frame gen - restart after changing chain selections
        pttStartStopTxPacketGen(pMac, eANI_BOOLEAN_OFF);
    }

    if (eHAL_STATUS_SUCCESS == halPhySetChainSelect(pMac, chainSelection))
    {
        //chain selection set, but we need to make sure the correct receive chains are set for test modes
        switch (chainSelection)
        {
            //test modes - follow up halPhySetChainSelect with specific Rx/Tx overrides
            case PHY_CHAIN_SEL_R0_ONLY:
                rxMask = AGC_RX_0;
                stopTx = eANI_BOOLEAN_TRUE;
                break;

            case PHY_CHAIN_SEL_R1_ONLY:
                rxMask = AGC_RX_1;
                stopTx = eANI_BOOLEAN_TRUE;
                break;

            case PHY_CHAIN_SEL_R2_ONLY:
                rxMask = AGC_RX_2;
                stopTx = eANI_BOOLEAN_TRUE;
                break;

            case PHY_CHAIN_SEL_NO_RX_TX:
                stopTx = eANI_BOOLEAN_TRUE;
            case PHY_CHAIN_SEL_T0_ONLY:
            case PHY_CHAIN_SEL_T1_ONLY:
            case PHY_CHAIN_SEL_T0T1_ONLY:
                rxMask = AGC_NO_RX;
                break;

            case PHY_CHAIN_SEL_R0R1_ONLY:
                rxMask = AGC_RX_0_AND_1;
                stopTx = eANI_BOOLEAN_TRUE;
                break;

            case PHY_CHAIN_SEL_R0R1R2_ONLY:
                rxMask = AGC_ALL_RX;
                stopTx = eANI_BOOLEAN_TRUE;
                break;

            //production modes - receive and transmit already setup correctly if we're not transmitting a waveform
            case PHY_CHAIN_SEL_R0_T0_ON:
            case PHY_CHAIN_SEL_R0R1_T0_ON:
            case PHY_CHAIN_SEL_R0R1_T0T1_ON:
            case PHY_CHAIN_SEL_R0R1R2_T0T1_ON:
            default:
                break;
        }

        if (rxMask != 0xFF)
        {
            //we've entered a mode that requires some alteration to the receive chain enables and crossbar settings
            if (asicSetAGCCrossbar(pMac, rxMask) == eHAL_STATUS_SUCCESS)
            {
                if (eHAL_STATUS_SUCCESS != asicOverrideAGCRxEnable(pMac, rxMask, AGC_RX_CALIBRATING))
                {
                    return (FAILURE);
                }
            }
            else
            {
                return (FAILURE);
            }
        }

        asicAGCReset(pMac);

        //now that the receive chains are setup, check to see the transmit chains need to be overriden
        if (pMac->ptt.wfmEnabled)
        {
            //for waveforms, we need to make sure the transmitters are overriden correctly
            switch (chainSelection)
            {
                case PHY_CHAIN_SEL_R0_ONLY:
                case PHY_CHAIN_SEL_R1_ONLY:
                case PHY_CHAIN_SEL_R2_ONLY:
                case PHY_CHAIN_SEL_NO_RX_TX:
                case PHY_CHAIN_SEL_R0R1_ONLY:
                case PHY_CHAIN_SEL_R0R1R2_ONLY:
                    //stop waveform and override all tx to OFF
                    pttStopWaveform(pMac);
                    {
                        if (eHAL_STATUS_SUCCESS != asicSetTxDACs(pMac, PHY_NO_TX_CHAINS, eANI_BOOLEAN_ON, eANI_BOOLEAN_OFF))
                        {
                            return (FAILURE);
                        }
                    }

                    break;

                case PHY_CHAIN_SEL_T0_ONLY:
                case PHY_CHAIN_SEL_R0_T0_ON:
                case PHY_CHAIN_SEL_R0R1_T0_ON:
                    //override tx1 to OFF
                    if (eHAL_STATUS_SUCCESS != asicSetTxDACs(pMac, PHY_TX_CHAIN_1, eANI_BOOLEAN_ON, pMac->ptt.wfmEnabled))
                    {
                        return (FAILURE);
                    }
                    break;
                case PHY_CHAIN_SEL_T1_ONLY:
                    //override tx0 to OFF
                    if (eHAL_STATUS_SUCCESS != asicSetTxDACs(pMac, PHY_TX_CHAIN_0, eANI_BOOLEAN_ON, pMac->ptt.wfmEnabled))
                    {
                        return (FAILURE);
                    }
                    break;

                case PHY_CHAIN_SEL_T0T1_ONLY:
                case PHY_CHAIN_SEL_R0R1_T0T1_ON:
                case PHY_CHAIN_SEL_R0R1R2_T0T1_ON:
                    //override no chains to OFF
                    if (eHAL_STATUS_SUCCESS != asicSetTxDACs(pMac, PHY_NO_TX_CHAINS, eANI_BOOLEAN_ON, pMac->ptt.wfmEnabled))
                    {
                        return (FAILURE);
                    }
                    break;

                default:
                    assert(0);
                    return (FAILURE);
                    break;
            }

            //set the waveform gain again after the change in enabled tx chains
            if (eHAL_STATUS_SUCCESS != asicTPCPowerOverride(pMac,  pMac->ptt.forcedTxGain[PHY_TX_CHAIN_0], pMac->ptt.forcedTxGain[PHY_TX_CHAIN_1]))
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
ePttStatus pttSetWaveform(tpAniSirGlobal pMac, tWaveformSample *waveform, tANI_U16 numSamples, tANI_BOOLEAN clk80)
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


ePttStatus pttSetTxWaveformGain(tpAniSirGlobal pMac, ePhyTxChains txChain, tANI_U8 gain)
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

        pMac->ptt.forcedTxGain[PHY_TX_CHAIN_1].coarsePwr = coarse;
        pMac->ptt.forcedTxGain[PHY_TX_CHAIN_1].finePwr = fine;
    }
    else if ((txChain == PHY_TX_CHAIN_0) || (txChain == PHY_TX_CHAIN_1))
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
                            pMac->ptt.forcedTxGain[PHY_TX_CHAIN_1]);

    return (SUCCESS);
}


ePttStatus pttGetWaveformPowerAdc(tpAniSirGlobal pMac, tANI_U8 *tx0PowerAdc, tANI_U8 *tx1PowerAdc)
{
    if(asicGetTxPowerMeasurement(pMac, PHY_TX_CHAIN_0, tx0PowerAdc) != eHAL_STATUS_SUCCESS){ return FAILURE; }
    if(asicGetTxPowerMeasurement(pMac, PHY_TX_CHAIN_1, tx1PowerAdc) != eHAL_STATUS_SUCCESS){ return FAILURE; }
    (*tx0PowerAdc) >>= 1;
    (*tx1PowerAdc) >>= 1;
    
    return (SUCCESS);
}


ePttStatus pttStartWaveform(tpAniSirGlobal pMac)
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


ePttStatus pttStopWaveform(tpAniSirGlobal pMac)
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




ePttStatus pttCloseTpcLoop(tpAniSirGlobal pMac, tANI_BOOLEAN tpcClose)
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
ePttStatus pttSetPacketTxGainTable(tpAniSirGlobal pMac, ePhyTxChains txChain, tANI_U8 minIndex, tANI_U8 maxIndex, tANI_U8 *gainTable)
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

ePttStatus pttGetPacketTxGainTable(tpAniSirGlobal pMac, ePhyTxChains txChain, tANI_U8 *gainTable)
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

ePttStatus pttSetPacketTxGainIndex(tpAniSirGlobal pMac, tANI_U8 index)
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


ePttStatus pttForcePacketTxGain(tpAniSirGlobal pMac, ePhyTxChains txChain, tANI_U8 gain)
{

    eTxCoarseGain coarse = READ_MASKED_BITS(gain, COARSE_GAIN_MASK, COARSE_GAIN_OFFSET);
    eTxFineGain fine = (eTxFineGain)READ_MASKED_BITS(gain, FINE_GAIN_MASK, FINE_GAIN_OFFSET);


    //for packet transmission, fill the tx gain tables with the force gain
    //this is independent of setting the gain index, which will become irrelevant for open-loop
    // For closed-loop, these values will be adjusted by the TPC hardware and so really doesn't make sense
    // except to initialize the gain tables to something close to the current calibration
    if ((txChain == PHY_ALL_TX_CHAINS) || (txChain == PHY_MAX_TX_CHAINS))
    {
        tANI_U32 i;

        for (i = 0; i < TPC_MEM_GAIN_LUT_DEPTH; i++)
        {
            pMac->ptt.tpcGainLut[PHY_TX_CHAIN_0][i].coarsePwr = coarse;
            pMac->ptt.tpcGainLut[PHY_TX_CHAIN_0][i].finePwr = fine;
        }
        asicLoadTPCGainLUT(pMac, PHY_TX_CHAIN_0, &(pMac->ptt.tpcGainLut[PHY_TX_CHAIN_0][0]));

        for (i = 0; i < TPC_MEM_GAIN_LUT_DEPTH; i++)
        {
            pMac->ptt.tpcGainLut[PHY_TX_CHAIN_1][i].coarsePwr = coarse;
            pMac->ptt.tpcGainLut[PHY_TX_CHAIN_1][i].finePwr = fine;
        }
        asicLoadTPCGainLUT(pMac, PHY_TX_CHAIN_1, &(pMac->ptt.tpcGainLut[PHY_TX_CHAIN_1][0]));
    }
    else if ((txChain == PHY_TX_CHAIN_0) || (txChain == PHY_TX_CHAIN_1))
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

    return (SUCCESS);
}



    //closed loop(CLPC) service
ePttStatus pttSetPwrIndexSource(tpAniSirGlobal pMac, ePowerTempIndexSource indexSource)
{
    ePttStatus retVal;
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


ePttStatus pttSetTxPower(tpAniSirGlobal pMac, t2Decimal dbmPwr)
{
    ePttStatus retVal;
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



ePttStatus pttGetTxPowerReport(tpAniSirGlobal pMac, tTxPowerReport *pwrReport)
{
    tPowerDetect adc0, adc1;
    tPwrTemplateRange lutRangeIndexed;

    if (pMac->ptt.wfmEnabled)
    {
        //Waveform power is a direct measurement of the continuous waveform
        if(asicGetTxPowerMeasurement(pMac, PHY_TX_CHAIN_0, &adc0) != eHAL_STATUS_SUCCESS){ return FAILURE; }
        if(asicGetTxPowerMeasurement(pMac, PHY_TX_CHAIN_0, &adc1) != eHAL_STATUS_SUCCESS){ return FAILURE; }
    }
    else if ((pMac->ptt.frameGenEnabled) && (pMac->ptt.phyDbgFrameGen))
    {
        tANI_U32 sensed;
    
        //packet power is retrieved from the preamble sync'd sample in SENSED_POWER
#ifdef TAURUS_A3
        halReadRegister(pMac, TPC_SENSED_PWR_REG, &sensed);

#else
        do
        {
            // Due to some Taurus bugs we need to follow a procedure to
            // 1. Ensure phyDbg packet gen was enabled and momentarily stop it
            // 1. Reset TXP to enable one subsequent Power ADC reading
            // 2. Ensure that closed-loop power control is enabled while performing the measurement - restore appropriately afterwards
            // 3. Restart the phyDbg packet generation - the measurement is from the first frame sent
            // 4. Get TPC_SENSED_PWR_REG and return power report based on this reading
            pttStartStopTxPacketGen(pMac, eANI_BOOLEAN_OFF);    //stop
        
            halWriteRegister(pMac, TXP_TXP_CMDF_DIN_REG, 0x10101);
            halWriteRegister(pMac, TXP_TXP_CMDF_DIN_REG, 0);

    /*
                halWriteRegister(pMac, MCU_SOFT_RESET_REG, 
                                   (MCU_SOFT_RESET_TXP_PHYTX_ABORT_MASK |
                                    MCU_SOFT_RESET_PHY_AHB_2_APB_SOFT_RESET_MASK |
                                    MCU_SOFT_RESET_DPU_SOFT_RESET_MASK |
                                    MCU_SOFT_RESET_BMU_SOFT_RESET_MASK |
                                    MCU_SOFT_RESET_MTU_SOFT_RESET_MASK |
                                    MCU_SOFT_RESET_RXP_SOFT_RESET_MASK |
                                    MCU_SOFT_RESET_TXP_SOFT_RESET_MASK |
                                    MCU_SOFT_RESET_DBR_SOFT_RESET_MASK |
                                    MCU_SOFT_RESET_CBR_SOFT_RESET_MASK |
                                    MCU_SOFT_RESET_MLC_SOFT_RESET_MASK
                                   )
                                );
    */
            rdModWrNovaField(pMac, MCU_SOFT_RESET_REG, 
                             MCU_SOFT_RESET_TXP_SOFT_RESET_MASK, 
                             MCU_SOFT_RESET_TXP_SOFT_RESET_OFFSET, 
                             1
                            );
            rdModWrNovaField(pMac, MCU_SOFT_RESET_REG, 
                             MCU_SOFT_RESET_TXP_SOFT_RESET_MASK, 
                             MCU_SOFT_RESET_TXP_SOFT_RESET_OFFSET, 
                             0
                            );

            if (pMac->hphy.phy.test.testTpcClosedLoop == eANI_BOOLEAN_FALSE)
            {
                //temporarily close the loop while taking a measurement
                halWriteRegister(pMac, TPC_TXPWR_ENABLE_REG, TPC_TXPWR_ENABLE_EN_MASK);
            }
            pttStartStopTxPacketGen(pMac, eANI_BOOLEAN_ON);    //restart
        
            sirBusyWait(1000000);
            halReadRegister(pMac, TPC_SENSED_PWR_REG, &sensed);


            if (pMac->hphy.phy.test.testTpcClosedLoop == eANI_BOOLEAN_FALSE)
            {
                //temporarily close the loop while taking a measurement
                halWriteRegister(pMac, TPC_TXPWR_ENABLE_REG, 0);
            }
        
            //if the loop was closed, then iterate to ensure the power always converges
        }while ((i++ < 50) && (pMac->hphy.phy.test.testTpcClosedLoop == eANI_BOOLEAN_TRUE));
#endif

        adc0 = (tPowerDetect)((sensed & TPC_SENSED_PWR_SENSPWR0_MASK) >> TPC_SENSED_PWR_SENSPWR0_OFFSET);
        adc1 = (tPowerDetect)((sensed & TPC_SENSED_PWR_SENSPWR1_MASK) >> TPC_SENSED_PWR_SENSPWR1_OFFSET);
    }
    else
    {
        phyLog(pMac, LOGE, "ERROR: Tx power report only available if frames or a waveform is transmitted\n");
        memset(pwrReport, 0, sizeof(tTxPowerReport));
        return FAILURE;
    }

    
    
    adc0 >>= 1;
    adc1 >>= 1;

    pwrReport->channelId = pMac->hphy.phy.test.testChannelId;
    pwrReport->cbState = pMac->hphy.phy.test.testCbState;
    pwrReport->rate = pMac->ptt.frameGenParams.rate;
    pwrReport->pwrTemplateIndex = pMac->hphy.phy.test.testLastPwrIndex;
    pwrReport->tx0.adc = adc0;
    asicGetTxGainAtIndex(pMac, PHY_TX_CHAIN_0, pwrReport->pwrTemplateIndex, &pwrReport->tx0.gain);

    if (pMac->hphy.phyTPC.pwrCfgPresent == eANI_BOOLEAN_TRUE)
    {
        lutRangeIndexed = phyGetTxPowerRangeForTempIndex(pMac, PHY_TX_CHAIN_0, pwrReport->pwrTemplateIndex);
        pwrReport->tx0.indexMinMatch.lut = lutRangeIndexed.min;
        pwrReport->tx0.indexMinMatch.abs.reported = phyGetAbsTxPowerForLutValue(pMac, PHY_TX_CHAIN_0, lutRangeIndexed.min);
        pwrReport->tx0.indexMaxMatch.lut = lutRangeIndexed.max;
        pwrReport->tx0.indexMaxMatch.abs.reported = phyGetAbsTxPowerForLutValue(pMac, PHY_TX_CHAIN_0, lutRangeIndexed.max);

        pwrReport->tx0.output.lut = (tPowerDetect)pMac->hphy.phyTPC.curTpcPwrLUT[PHY_TX_CHAIN_0][adc0];
        pwrReport->tx0.output.abs.reported = phyGetAbsTxPowerForLutValue(pMac, PHY_TX_CHAIN_0, pwrReport->tx0.output.lut);
    }
    else
    {
        phyLog(pMac, LOGE, "NO TX POWER CONFIG - TxPowerReport inaccurate\n");
        pwrReport->tx0.indexMinMatch.lut = 0;
        pwrReport->tx0.indexMinMatch.abs.reported = 0;
        pwrReport->tx0.indexMaxMatch.lut = 0;
        pwrReport->tx0.indexMaxMatch.abs.reported = 0;
        pwrReport->tx0.output.abs.reported = 0;
    }

    pwrReport->tx1.adc = adc1;
    asicGetTxGainAtIndex(pMac, PHY_TX_CHAIN_1, pwrReport->pwrTemplateIndex, &pwrReport->tx1.gain);

    if (pMac->hphy.phyTPC.pwrCfgPresent == eANI_BOOLEAN_TRUE)
    {
        lutRangeIndexed = phyGetTxPowerRangeForTempIndex(pMac, PHY_TX_CHAIN_1, pwrReport->pwrTemplateIndex);
        pwrReport->tx1.indexMinMatch.lut = lutRangeIndexed.min;
        pwrReport->tx1.indexMinMatch.abs.reported = phyGetAbsTxPowerForLutValue(pMac, PHY_TX_CHAIN_1, lutRangeIndexed.min);
        pwrReport->tx1.indexMaxMatch.lut = lutRangeIndexed.max;
        pwrReport->tx1.indexMaxMatch.abs.reported = phyGetAbsTxPowerForLutValue(pMac, PHY_TX_CHAIN_1, lutRangeIndexed.max);

        pwrReport->tx1.output.lut = (tPowerDetect)pMac->hphy.phyTPC.curTpcPwrLUT[PHY_TX_CHAIN_1][adc1];
        pwrReport->tx1.output.abs.reported = phyGetAbsTxPowerForLutValue(pMac, PHY_TX_CHAIN_1, pwrReport->tx1.output.lut);
    }
    else
    {
        pwrReport->tx1.indexMinMatch.lut = 0;
        pwrReport->tx1.indexMinMatch.abs.reported = 0;
        pwrReport->tx1.indexMaxMatch.lut = 0;
        pwrReport->tx1.indexMaxMatch.abs.reported = 0;
        pwrReport->tx1.output.abs.reported = 0;
    }

    return SUCCESS;
}






#define MIN_POINT (0)
#define MAX_POINT (numTpcCalPointsPerFreq - 1)
static ePttStatus NormalizeTpcChainData(tpAniSirGlobal pMac, tTpcCalPoint *chainData, tANI_U16 numTpcCalPoints, tTpcCaldPowerPoint tpcChainCfg[MAX_TPC_CAL_POINTS])
{
    tANI_U16 calPoint;

    //requiring at least 3 cal points per chain
    if ((numTpcCalPoints > MAX_TPC_CAL_POINTS) || (numTpcCalPoints <= 2))
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
            SET_FULL_PRECISION_TPC_LUT_VALUE(0, tpcChainCfg[calPoint]);
        }
        else if (x > (t2Decimal)(MAX_PWR_LUT_DBM_2DEC_PLACES))
        {
            phyLog(pMac, LOGE, "NormalizeTpcChainData(): reported pwr %d dBm > max pwr %d dbm, mapping pwr adc code %d to %d\n",
                                    x / 100, MAX_PWR_LUT_DBM_2DEC_PLACES / 100, chainData[calPoint].pwrDetAdc ,
                                    ((TPC_MEM_POWER_LUT_DEPTH * EXTRA_TPC_LUT_MULT) -1));
            // The ADC reading for a power measurement greater than MAX_PWR_LUT_DBM is clipped
            SET_FULL_PRECISION_TPC_LUT_VALUE(((TPC_MEM_POWER_LUT_DEPTH * EXTRA_TPC_LUT_MULT) -1), tpcChainCfg[calPoint]);
            
        }
        else
        {
            tANI_S32 x1 = (t2Decimal)(MIN_PWR_LUT_DBM_2DEC_PLACES);
            tANI_S32 y1 = 0;
            tANI_S32 x2 = (t2Decimal)(MAX_PWR_LUT_DBM_2DEC_PLACES);
            tANI_S32 y2 = ((TPC_MEM_POWER_LUT_DEPTH * EXTRA_TPC_LUT_MULT) -1);
            tTpcLutValue lutValue;

            // this simply scales the power measurements to the necessary range
            // since x1 ( = 8(00) ) <= x <= x2 ( =24(00) ) and y1 = 0, y2 = 127
            lutValue = (tTpcLutValue)InterpolateBetweenPoints(x1, y1, x2, y2, x);
            SET_FULL_PRECISION_TPC_LUT_VALUE(lutValue, tpcChainCfg[calPoint]);

            phyLog(pMac, LOGE, "NormalizeTpcChannelData(): measured pwr %d dbm, mapping pwr adc code %d to %d\n",
                                    (t2Decimal)(MIN_PWR_LUT_DBM_2DEC_PLACES), chainData[calPoint].pwrDetAdc,
                                    GET_FULL_PRECISION_TPC_LUT_VALUE(tpcChainCfg[calPoint].adjustedPwrDet, tpcChainCfg[calPoint].extraPrecision.hi8_adjustedPwrDet) );
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


/*
    ePttStatus pttSaveTxPwrFreqTable(tpAniSirGlobal pMac, tANI_U8 numTpcCalFreqs, const tTpcFreqData *table)
    {
    
         ePttStatus retVal = FAILURE;   //init to fail
         tANI_U8 highestTxChain = (pMac->hphy.phy.cfgChains == PHY_CHAIN_SEL_R0R1_T0_ON ? 1 : 2);
    
    
         if ((numTpcCalFreqs > NUM_LEGIT_RF_CHANNELS) || (table == NULL))
         {
             return (FAILURE);
         }
    
         if ((pMac->hphy.phy.pwrOptimal == NULL) || (pMac->hphy.phy.regDomainInfo == NULL))
         {
             phyLog(pMac, LOGE, "Storing CLPC without appropriate EEPROM image not allowed\n");
             return (FAILURE);
         }
    
         {
             //Process input table into set of calibration points to store in EEPROM
             //we assume that the given table is ordered in ascending frequencies and ascending calpoints
             //the table can contain 2.4GHz only, 5GHz only, or both together, we'll sort them out.
             tTpcConfig aPwrSampled[NUM_TPC_5GHZ_CHANNELS];
             tTpcConfig bPwrSampled[NUM_TPC_2_4GHZ_CHANNELS];
             tANI_U8 chanIndex;
             tANI_U8 aIndex = 0;
             tANI_U8 bIndex = 0;
    
             memset(&aPwrSampled, 0, sizeof(tTpcConfig) * NUM_TPC_5GHZ_CHANNELS);
             memset(&bPwrSampled, 0, sizeof(tTpcConfig) * NUM_TPC_2_4GHZ_CHANNELS);
    
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
    
                 //add the interpolated points to the EEPROM table to be stored
                 if (tpcFreqCfg.freq >= 2412 && tpcFreqCfg.freq <= 2484)
                 {
                     bIndex++;
    
                     if (bIndex > NUM_TPC_2_4GHZ_CHANNELS)
                     {
                         return (FAILURE);
                     }
                     //copy one channel's worth of data to the 24GHz table that will be stored in EEPROM
                     memcpy(&(bPwrSampled[bIndex - 1]), &tpcFreqCfg, sizeof(tTpcConfig));
                 }
                 if (tpcFreqCfg.freq >= 4920 && tpcFreqCfg.freq <= 5805)
                 {
                     aIndex++;
                     if (aIndex > NUM_TPC_5GHZ_CHANNELS)
                     {
                         return (FAILURE);
                     }
                     //copy one channel's worth of data to the 24GHz table that will be stored in EEPROM
                     memcpy(&(aPwrSampled[aIndex - 1]), &tpcFreqCfg, sizeof(tTpcConfig));
                 }
             }
    
             if ((aIndex > 1) && (aIndex <= NUM_TPC_5GHZ_CHANNELS))
             {
                 //5GHz calibrated - store 5GHz EEPROM table
                 if (eHAL_STATUS_SUCCESS == halWriteEepromTable(pMac, EEPROM_TABLE_5_TPC_CONFIG, (uEepromTables *)aPwrSampled))
                 {
                     tTpcParams tpcParams;
                     phyLog(pMac, LOGE, "Set EEPROM_TABLE_5_TPC_CONFIG table ... done.\n\n");
    
                     tpcParams.numTpcCalFreqs = aIndex;
                     tpcParams.numTpcCalPointsPerFreq = MAX_TPC_CAL_POINTS;
    
                     if (eHAL_STATUS_SUCCESS == halWriteEepromTable(pMac, EEPROM_TABLE_5_TPC_PARAMS, (uEepromTables *)&tpcParams))
                     {
                         retVal = SUCCESS;
                         phyLog(pMac, LOGE, "Set EEPROM_TABLE_5_TPC_PARAMS table ... done.\n\n");
                         
                         if ((eHAL_STATUS_SUCCESS == halStoreTableToEeprom(pMac, EEPROM_TABLE_5_TPC_CONFIG)) &&
                             (eHAL_STATUS_SUCCESS == halStoreTableToEeprom(pMac, EEPROM_TABLE_5_TPC_PARAMS))
                            )
                         {
                            phyLog(pMac, LOGE, "EEPROM_TABLE_5_TPC_CONFIG and EEPROM_TABLE_5_TPC_PARAMS tables stored to EEPROM\n");
                         }
                     }
                     else
                     {
                         phyLog(pMac, LOGE, "Set EEPROM_TABLE_5_TPC_PARAMS table ... FAILED.\n\n");
                         return (FAILURE);
                     }
                 }
                 else
                 {
                    phyLog(pMac, LOGE, "Set EEPROM_TABLE_5_TPC_CONFIG table ... FAILED.\n\n");
                    return (FAILURE);
                 }
             }
    
             if ((bIndex > 1) && (bIndex <= NUM_TPC_2_4GHZ_CHANNELS))
             {
                 //2.4GHz calibrated - store 2.4GHz EEPROM table
                 if (eHAL_STATUS_SUCCESS == halWriteEepromTable(pMac, EEPROM_TABLE_2_4_TPC_CONFIG, (uEepromTables *)bPwrSampled))
                 {
                     tTpcParams tpcParams;
                     phyLog(pMac, LOGE, "Set EEPROM_TABLE_2_4_TPC_CONFIG table ... done.\n\n");
    
                     tpcParams.numTpcCalFreqs = bIndex;
                     tpcParams.numTpcCalPointsPerFreq = MAX_TPC_CAL_POINTS;
    
                     if (eHAL_STATUS_SUCCESS == halWriteEepromTable(pMac, EEPROM_TABLE_2_4_TPC_PARAMS, (uEepromTables *)&tpcParams))
                     {
                         retVal = SUCCESS;
                         phyLog(pMac, LOGE, "Set EEPROM_TABLE_2_4_TPC_PARAMS table ... done.\n\n");
    
                         if ((eHAL_STATUS_SUCCESS == halStoreTableToEeprom(pMac, EEPROM_TABLE_2_4_TPC_CONFIG)) &&
                             (eHAL_STATUS_SUCCESS == halStoreTableToEeprom(pMac, EEPROM_TABLE_2_4_TPC_PARAMS))
                            )
                         {
                            phyLog(pMac, LOGE, "EEPROM_TABLE_2_4_TPC_CONFIG and EEPROM_TABLE_2_4_TPC_PARAMS tables stored to EEPROM\n");
                         }
                     }
                     else
                     {
                         phyLog(pMac, LOGE, "Set EEPROM_TABLE_2_4_TPC_PARAMS table ... FAILED.\n\n");
                         return (FAILURE);
                     }
                 }
                 else
                 {
                    phyLog(pMac, LOGE, "Set EEPROM_TABLE_2_4_TPC_CONFIG table ... FAILED.\n\n");
                    return (FAILURE);
                 }
             }
         }
    
        //now reconfigure TPC to make changes take effect immediately
        if (ConfigureTpcFromEeprom(pMac) != eHAL_STATUS_SUCCESS)
        {
            retVal = FAILURE;
        }
    
        return (retVal);
    
    }
*/



/* new version based on combined EEPROM table */
ePttStatus pttSaveTxPwrFreqTable(tpAniSirGlobal pMac, tANI_U8 numTpcCalFreqs, const tTpcFreqData *table)
{

    ePttStatus retVal = FAILURE;   //init to fail
    tANI_U8 highestTxChain = (pMac->hphy.phy.cfgChains == PHY_CHAIN_SEL_R0R1_T0_ON ? 1 : 2);


    if ((numTpcCalFreqs > NUM_LEGIT_RF_CHANNELS) || (table == NULL))
    {
        return (FAILURE);
    }

    if ((pMac->hphy.phy.pwrOptimal == NULL) || (pMac->hphy.phy.regDomainInfo == NULL))
    {
        phyLog(pMac, LOGE, "Storing CLPC without appropriate EEPROM image not allowed\n");
        return (FAILURE);
    }

    if (numTpcCalFreqs >= 2)
    {
        //Process input table into set of calibration points to store in EEPROM
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

            //add the interpolated points to the EEPROM table to be stored
            memcpy(&(pwrSampled[chanIndex]), &tpcFreqCfg, sizeof(tTpcConfig));
        }

        {
            if (eHAL_STATUS_SUCCESS == halWriteEepromTable(pMac, EEPROM_TABLE_TPC_CONFIG, (uEepromTables *)pwrSampled))
            {
                tTpcParams tpcParams;
                phyLog(pMac, LOGE, "Set EEPROM_TABLE_TPC_CONFIG table ... done.\n\n");

                tpcParams.numTpcCalFreqs = numTpcCalFreqs;
                tpcParams.numTpcCalPointsPerFreq = MAX_TPC_CAL_POINTS;

                if (eHAL_STATUS_SUCCESS == halWriteEepromTable(pMac, EEPROM_TABLE_TPC_PARAMS, (uEepromTables *)&tpcParams))
                {
                    retVal = SUCCESS;
                    phyLog(pMac, LOGE, "Set EEPROM_TABLE_TPC_PARAMS table ... done.\n\n");
                    
                    if ((eHAL_STATUS_SUCCESS == halStoreTableToEeprom(pMac, EEPROM_TABLE_TPC_CONFIG)) &&
                        (eHAL_STATUS_SUCCESS == halStoreTableToEeprom(pMac, EEPROM_TABLE_TPC_PARAMS))
                       )
                    {
                       phyLog(pMac, LOGE, "EEPROM_TABLE_TPC_CONFIG and EEPROM_TABLE_TPC_PARAMS tables stored to EEPROM\n");
                    }
                }
                else
                {
                    phyLog(pMac, LOGE, "Set EEPROM_TABLE_TPC_PARAMS table ... FAILED.\n\n");
                    return (FAILURE);
                }
            }
            else
            {
               phyLog(pMac, LOGE, "Set EEPROM_TABLE_TPC_CONFIG table ... FAILED.\n\n");
               return (FAILURE);
            }
        }

        //now reconfigure TPC to make changes take effect immediately
        if (ConfigureTpcFromEeprom(pMac) != eHAL_STATUS_SUCCESS)
        {
            retVal = FAILURE;
        }
    }


    return (retVal);

}



ePttStatus pttSetPowerLut(tpAniSirGlobal pMac, ePhyTxChains txChain, tANI_U8 minIndex, tANI_U8 maxIndex, tANI_U8 *powerLut)
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


ePttStatus pttGetPowerLut(tpAniSirGlobal pMac, ePhyTxChains txChain, tANI_U8 *powerLut)
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
ePttStatus pttDisableAgcTables(tpAniSirGlobal pMac, tANI_U8 rx0GainIdx, tANI_U8 rx1GainIdx, tANI_U8 rx2GainIdx)
{

    if( (rx0GainIdx >= AGC_GAIN_LUT_DEPTH) || (rx1GainIdx >= AGC_GAIN_LUT_DEPTH) || (rx2GainIdx >= AGC_GAIN_LUT_DEPTH) )
    {
        return (FAILURE);
    }
    
    asicOverrideAGCRxChainGain(pMac, PHY_RX_CHAIN_0, rx0GainIdx);
    asicOverrideAGCRxChainGain(pMac, PHY_RX_CHAIN_1, rx1GainIdx);
    asicOverrideAGCRxChainGain(pMac, PHY_RX_CHAIN_2, rx2GainIdx);

    pMac->ptt.rx0AgcEnabled = eANI_BOOLEAN_FALSE;
    pMac->ptt.rx1AgcEnabled = eANI_BOOLEAN_FALSE;
    pMac->ptt.rx2AgcEnabled = eANI_BOOLEAN_FALSE;
    
    return (SUCCESS);
}


ePttStatus pttEnableAgcTables(tpAniSirGlobal pMac, tANI_BOOLEAN rx0, tANI_BOOLEAN rx1, tANI_BOOLEAN rx2)
{
    if ((rx0 > 1) || (rx1 > 1) || (rx2 > 1))
    {
        return(FAILURE);
    }


    if (rx0 == eANI_BOOLEAN_TRUE)
    {
        asicCeaseOverrideAGCRxChainGain(pMac, PHY_RX_CHAIN_0);

        //load current AGC Gain Tables
        asicSetAGCGainLut(pMac, PHY_RX_CHAIN_0, 0, (NUM_GAIN_LUT_SETTINGS - 1), &pttAgcGainLUT[PHY_RX_CHAIN_0][0]);
    }
    pMac->ptt.rx0AgcEnabled = rx0;

    if (rx1 == eANI_BOOLEAN_TRUE)
    {
        asicCeaseOverrideAGCRxChainGain(pMac, PHY_RX_CHAIN_1);

        //load current AGC Gain Tables
        asicSetAGCGainLut(pMac, PHY_RX_CHAIN_1, 0, (NUM_GAIN_LUT_SETTINGS - 1), &pttAgcGainLUT[PHY_RX_CHAIN_1][0]);
    }
    pMac->ptt.rx1AgcEnabled = rx1;

    if (rx2 == eANI_BOOLEAN_TRUE)
    {
        asicCeaseOverrideAGCRxChainGain(pMac, PHY_RX_CHAIN_2);

        //load current AGC Gain Tables
        asicSetAGCGainLut(pMac, PHY_RX_CHAIN_2, 0, (NUM_GAIN_LUT_SETTINGS - 1), &pttAgcGainLUT[PHY_RX_CHAIN_2][0]);
    }
    pMac->ptt.rx2AgcEnabled = rx2;

    return (SUCCESS);
}


ePttStatus pttSetAgcTables(tpAniSirGlobal pMac, ePhyRxChains rxChain, tANI_U8 minIndex, tANI_U8 maxIndex, const tRxGain *rxGainTable)
{

    tRxGain sRxGainTable;
    memcpy(&sRxGainTable, rxGainTable, sizeof(tRxGain));
#ifdef ANI_BUS_TYPE_USB
    sRxGainTable.swEn &= (~LNA_SW_EN_MSK); //R+LNA_OFF because there is an external LNA controlled independently through GPIO
#endif

    if ((minIndex >= AGC_GAIN_LUT_DEPTH) || (maxIndex >= AGC_GAIN_LUT_DEPTH) || (minIndex > maxIndex) )
    {
        return(FAILURE);
    }


    if (rxChain < PHY_MAX_RX_CHAINS)
    {
        asicSetAGCGainLut(pMac, rxChain, minIndex, maxIndex, &sRxGainTable);
    }
    else if (rxChain == PHY_ALL_RX_CHAINS)
    {
        tANI_U8 rx;

        for (rx = 0; rx < PHY_MAX_RX_CHAINS; rx++)
        {
            asicSetAGCGainLut(pMac, rx, minIndex, maxIndex, &sRxGainTable);
        }
    }
    else
    {
        return (FAILURE);
    }

    return (SUCCESS);
}


ePttStatus pttGetAgcTable(tpAniSirGlobal pMac, ePhyRxChains rxChain, tRxGain *agcTable /* NUM_AGC_GAINS */)
{
    if (rxChain > 2)
    {
        return(FAILURE);
    }

    if (eHAL_STATUS_SUCCESS == asicGetAgcGainLut(pMac, rxChain, agcTable))
    {
        return(SUCCESS);
    }
    else
    {
        return(FAILURE);
    }
}

void pttGetRxRssi(tpAniSirGlobal pMac, tANI_U8 *rx0Rssi, tANI_U8 *rx1Rssi, tANI_U8 *rx2Rssi)
{
    halReadRegister(pMac, AGC_RSSI0_REG, (tANI_U32 *)rx0Rssi);
    halReadRegister(pMac, AGC_RSSI1_REG, (tANI_U32 *)rx1Rssi);
    halReadRegister(pMac, AGC_RSSI2_REG, (tANI_U32 *)rx2Rssi);
}





//Rx Frame Catcher Service
ePttStatus pttSetRxDisableMode(tpAniSirGlobal pMac, tANI_BOOLEAN aPktsDisabled, tANI_BOOLEAN bPktsDisabled, tANI_BOOLEAN chanBondPktsDisabled)
{
    ePhyRxDisabledPktTypes setVal;

    if ((aPktsDisabled > 1) ||
        (bPktsDisabled > 1) ||
        (chanBondPktsDisabled > 1)
       )
    {
        return(FAILURE);
    }

    setVal = (ePhyRxDisabledPktTypes)(((tANI_U8 )aPktsDisabled |
                                       ((tANI_U8 )bPktsDisabled << 1) |
                                       ((tANI_U8 )chanBondPktsDisabled << 2) |
                                       ((tANI_U8 )chanBondPktsDisabled << 3)
                                      )
                                     );

    if (halPhySetRxPktsDisabled(pMac, setVal) != eHAL_STATUS_SUCCESS)
    {
        return (FAILURE);
    }

    return (SUCCESS);
}


ePttStatus pttGetRxPktCounts(tpAniSirGlobal pMac, tANI_U32 *numRxPackets)
{
    eHalStatus retVal;
    {
        tANI_U32 reg;
    
        GET_PHY_REG(pMac->hHdd, RXCLKCTRL_APB_BLOCK_CLK_EN_REG, &reg);
        if ((reg & RXCLKCTRL_APB_BLOCK_CLK_EN_PHYDBG_MASK) == 0)
        {
            phyLog(pMac, LOGE, "PHYDBG clock not on for pttGetRxPktCounts\n");
            return (FAILURE);
        }
    }
    
    if (eHAL_STATUS_SUCCESS != halReadRegister(pMac, PHYDBG_PKTCNT_REG, numRxPackets))
    {
        return (FAILURE);
    }
    
    return (SUCCESS);
}



ePttStatus pttResetRxPacketStatistics(tpAniSirGlobal pMac)
{
    eHalStatus retVal;
    {
        tANI_U32 reg;
    
        GET_PHY_REG(pMac->hHdd, RXCLKCTRL_APB_BLOCK_CLK_EN_REG, &reg);
        if ((reg & RXCLKCTRL_APB_BLOCK_CLK_EN_PHYDBG_MASK) == 0)
        {
            phyLog(pMac, LOGE, "PHYDBG clock not on for pttResetRxPacketStatistics\n");
            return (eHAL_STATUS_FAILURE);
        }
    }

    if (eHAL_STATUS_SUCCESS != halWriteRegister(pMac, PHYDBG_PKTCNT_REG, 0))
    {
        return (FAILURE);
    }

    return (SUCCESS);
}






//Rx Symbol Service
ePttStatus pttGrabRam(tpAniSirGlobal pMac, tANI_U32 startSample, tANI_U32 numSamples, tGrabRamSample *grabRam)
{
    if (eHAL_STATUS_SUCCESS == asicGrabAdcSamples(pMac, startSample, numSamples, grabRam))
    {    
        return (SUCCESS);
    }
    else
    {
        return (FAILURE);
    }
}


ePttStatus pttRxDcoCal(tpAniSirGlobal pMac, tRxDcoCorrect *rx0Dco, tRxDcoCorrect *rx1Dco, tRxDcoCorrect *rx2Dco, eGainSteps gain)
{
    if (pMac->ptt.wfmEnabled == eANI_BOOLEAN_FALSE)
    {
         RunInitialCal(pMac, RX_DCO_CAL_ONLY);   //temporary until we get DCO isolated for this

         pttGetRxDcoCorrect(pMac, rx0Dco, rx1Dco, rx2Dco, gain);

        
        return (SUCCESS);
    }
    else
    {
        return (FAILURE);   //can't do this while generating a waveform
    }
}



//Phy Calibration Service
ePttStatus pttRxIqCal(tpAniSirGlobal pMac, sIQCalValues* rx0, sIQCalValues* rx1, sIQCalValues* rx2, eGainSteps gain)
{
/*      TODO: Restore PTT Cal interface with new implementation
    if (pMac->ptt.wfmEnabled == eANI_BOOLEAN_FALSE)
    {
         phyIQCal(pMac);

         //restore settings, since this function may have altered these things
         pttEnableTxChains(pMac, pMac->ptt.tx0Enabled, pMac->ptt.tx1Enabled);
         pttEnableRxChains(pMac, pMac->ptt.rx0Enabled, pMac->ptt.rx1Enabled, pMac->ptt.rx2Enabled);
         pttSetChannel(pMac, pMac->hphy.phy.test.testChannelId, pMac->hphy.phy.test.testCbState);

         pttGetRxIqCorrect(pMac, PHY_RX_CHAIN_0, &(rx0->phase), &(rx0->amp));
         pttGetRxIqCorrect(pMac, PHY_RX_CHAIN_1, &(rx1->phase), &(rx1->amp));
         pttGetRxIqCorrect(pMac, PHY_RX_CHAIN_2, &(rx2->phase), &(rx2->amp));


        return (SUCCESS);
    }
    else
    {
        return (FAILURE);   //can't do this while generating a waveform
    }
*/
        return (SUCCESS);
}


ePttStatus pttTxIqCal(tpAniSirGlobal pMac, sIQCalValues* tx0, sIQCalValues* tx1, eGainSteps gain)
{

        return (SUCCESS);
}


ePttStatus pttTxCarrierSuppressCal(tpAniSirGlobal pMac, tTxLoCorrect* tx0, tTxLoCorrect *tx1, eGainSteps gain)
{
    if (pMac->ptt.wfmEnabled == eANI_BOOLEAN_FALSE)
    {
        RunInitialCal(pMac, TX_LO_CAL_ONLY);

        pttGetTxCarrierSuppressCorrect(pMac, tx0, tx1, gain);

        //restore settings, since this function may have altered these things
        // pttEnableTxChains(pMac, pMac->ptt.tx0Enabled, pMac->ptt.tx1Enabled);
        // pttEnableRxChains(pMac, pMac->ptt.rx0Enabled, pMac->ptt.rx1Enabled, pMac->ptt.rx2Enabled);


        return (SUCCESS);
    }
    else
    {
        return (FAILURE);   //can't do this while generating a waveform
    }
}


ePttStatus pttExecuteInitialCals(tpAniSirGlobal pMac)
{
    eHalStatus retVal = FAILURE;
    
    // We want to make sure that we only perform & store EEPROM_TABLE_QUASAR_FILTERS 
    //  if it appears that the board's EEPROM has been previously programmed with PTT GUI.
    // Check these two tables programmed in the default EEPROM image for an indication that this is the case
    
    if ((pMac->hphy.phy.regDomainInfo) && (pMac->hphy.phy.pwrOptimal))
    {
        tQuasarFilterSettings chanFilterSettings;
        
        if ((retVal = rfQuasarFilterCal(pMac, &chanFilterSettings)) == eHAL_STATUS_SUCCESS)
        {
            if ((retVal = halWriteEepromTable(pMac, EEPROM_TABLE_QUASAR_FILTERS, (uEepromTables *)&chanFilterSettings)) == eHAL_STATUS_SUCCESS)
            {
                if ((retVal = halGetEepromTableLoc(pMac, EEPROM_TABLE_QUASAR_FILTERS, (uEepromTables **)&pMac->hphy.rf.chanFilterSettings)) == eHAL_STATUS_SUCCESS)
                {
                    if ((retVal = halStoreTableToEeprom(pMac, EEPROM_TABLE_QUASAR_FILTERS)) == eHAL_STATUS_SUCCESS)
                    {
                        phyLog(pMac, LOGE, "Successfully calibrated RF filters and stored them in EEPROM\n");
                    }
                }
            }
        }
    }

    if (retVal == eHAL_STATUS_SUCCESS)
    {
        return (SUCCESS);
    }
    else
    {
        return (FAILURE);
    }
}


//Phy Calibration Override Service
ePttStatus pttSetTxCarrierSuppressCorrect(tpAniSirGlobal pMac, tTxLoCorrect tx0, tTxLoCorrect tx1, eGainSteps gain)
{
    if ((tANI_U32)gain > NUM_TX_GAIN_STEPS)
    {
        phyLog(pMac, LOGE, "Incorrect Gain %d\n", gain);
        return (FAILURE);
    }
        
    if (eHAL_STATUS_SUCCESS == rfSetTxLoCorrect(pMac, PHY_TX_CHAIN_0, tx0))
    {
        if (eHAL_STATUS_SUCCESS == rfSetTxLoCorrect(pMac, PHY_TX_CHAIN_1, tx1))
        {
            return (SUCCESS);
        }
        else
        {
            return (FAILURE);
        }
    }
    else
    {
        return (FAILURE);
    }
}


ePttStatus pttGetTxCarrierSuppressCorrect(tpAniSirGlobal pMac, tTxLoCorrect* tx0, tTxLoCorrect *tx1, eGainSteps gain)
{
    if ((tANI_U32)gain > NUM_TX_GAIN_STEPS)
    {
        phyLog(pMac, LOGE, "Incorrect Gain %d\n", gain);
        return (FAILURE);
    }
    
    
    if (eHAL_STATUS_SUCCESS == rfGetTxLoCorrect(pMac, PHY_TX_CHAIN_0, tx0))
    {
        if (eHAL_STATUS_SUCCESS == rfGetTxLoCorrect(pMac, PHY_TX_CHAIN_1, tx1))
        {
            return (SUCCESS);
        }
        else
        {
            return (FAILURE);
        }
    }
    else
    {
        return (FAILURE);
    }
}


ePttStatus pttSetTxIqCorrect(tpAniSirGlobal pMac, ePhyTxChains txChain, sIQCalValues correct, eGainSteps gain)
{
    if ((tANI_U32)gain > NUM_TX_GAIN_STEPS)
    {
        phyLog(pMac, LOGE, "Incorrect Gain %d\n", gain);
        return (FAILURE);
    }

    if (eHAL_STATUS_SUCCESS == asicTxFirSetTxPhaseCorrection(pMac, gain, txChain, correct))
    {
        return (SUCCESS);
    }
    else
    {
        return (FAILURE);
    }
}

ePttStatus pttGetTxIqCorrect(tpAniSirGlobal pMac, ePhyTxChains txChain, sIQCalValues *correct, eGainSteps gain)
{
    if ((tANI_U32)gain > NUM_TX_GAIN_STEPS)
    {
        phyLog(pMac, LOGE, "Incorrect Gain %d\n", gain);
        return (FAILURE);
    }

    if (eHAL_STATUS_SUCCESS == asicTxFirGetTxPhaseCorrection(pMac, gain, txChain, correct))
    {
        return (SUCCESS);
    }
    else
    {
        return (FAILURE);
    }
}



ePttStatus pttSetRxIqCorrect(tpAniSirGlobal pMac, ePhyRxChains rxChain, sIQCalValues correct, eGainSteps gain)
{
    if ((tANI_U32)gain > NUM_RX_GAIN_STEPS)
    {
        phyLog(pMac, LOGE, "Incorrect Gain %d\n", gain);
        return (FAILURE);
    }

    if (eHAL_STATUS_SUCCESS == asicWriteRxPhaseCorrection(pMac, gain, rxChain, correct))
    {
        return (SUCCESS);
    }
    else
    {
        return (FAILURE);
    }
}


ePttStatus pttGetRxIqCorrect(tpAniSirGlobal pMac, ePhyRxChains rxChain, sIQCalValues *correct, eGainSteps gain)
{
    if ((tANI_U32)gain > NUM_RX_GAIN_STEPS)
    {
        phyLog(pMac, LOGE, "Incorrect Gain %d\n", gain);
        return (FAILURE);
    }

    if (eHAL_STATUS_SUCCESS == asicReadRxPhaseCorrection(pMac, gain, rxChain, correct))
    {
        return (SUCCESS);
    }
    else
    {
        return (FAILURE);
    }
}


ePttStatus pttSetRxDcoCorrect(tpAniSirGlobal pMac, tRxDcoCorrect rx0Dco, tRxDcoCorrect rx1Dco, tRxDcoCorrect rx2Dco, eGainSteps gain)
{
    if ((tANI_U32)gain > NUM_QUASAR_RX_GAIN_STEPS)
    {
        phyLog(pMac, LOGE, "Incorrect Gain %d\n", gain);
        return (FAILURE);
    }

    rfSetDCOffset(pMac, PHY_RX_CHAIN_0, gain, rx0Dco);
    rfSetDCOffset(pMac, PHY_RX_CHAIN_1, gain, rx1Dco);
    rfSetDCOffset(pMac, PHY_RX_CHAIN_2, gain, rx2Dco);

    return (SUCCESS);
}


ePttStatus pttGetRxDcoCorrect(tpAniSirGlobal pMac, tRxDcoCorrect* rx0Dco, tRxDcoCorrect *rx1Dco, tRxDcoCorrect *rx2Dco, eGainSteps gain)
{
    if ((tANI_U32)gain > NUM_QUASAR_RX_GAIN_STEPS)
    {
        phyLog(pMac, LOGE, "Incorrect Gain %d\n", gain);
        return (FAILURE);
    }

    rfGetDCOffset(pMac, PHY_RX_CHAIN_0, gain, rx0Dco);
    rfGetDCOffset(pMac, PHY_RX_CHAIN_1, gain, rx1Dco);
    rfGetDCOffset(pMac, PHY_RX_CHAIN_2, gain, rx2Dco);

    return (SUCCESS);
}





//Rf Calibration Service
ePttStatus pttGetTempAdc(tpAniSirGlobal pMac, ePhyTxChains txChain, tANI_U8 *tempAdc)
{
    switch (txChain)
    {
        case PHY_TX_CHAIN_0:
        case PHY_TX_CHAIN_1:
             if (eHAL_STATUS_SUCCESS == rfTakeTemp(pMac, txChain, 1, (tTempADCVal *)tempAdc))
             {
                return (SUCCESS);
             }
             else
             {
                return (FAILURE);
             }
            break;
        default:
            return(FAILURE);
            break;
    }

    return (SUCCESS);
}

ePttStatus pttReadQuasarField(tpAniSirGlobal pMac, eQuasarFields quasarField, tANI_U32 *value)
{
    if (eHAL_STATUS_SUCCESS == rfReadQuasarField(pMac, quasarField, value))
    {
        return (SUCCESS);
    }
    else
    {
        return (FAILURE);
    }
}


ePttStatus pttWriteQuasarField(tpAniSirGlobal pMac, eQuasarFields quasarField, tANI_U32 value)
{
    if (eHAL_STATUS_SUCCESS == rfWriteQuasarField(pMac, quasarField, value))
    {
        return (SUCCESS);
    }
    else
    {
        return (FAILURE);
    }
}


ePttStatus pttSystemReset(tpAniSirGlobal pMac)
{
    //TODO: determine what to do for reset halPerformSystemReset((tHalHandle)pMac);
    return (SUCCESS);
}

ePttStatus pttLogDump(tpAniSirGlobal pMac, tANI_U32 cmd, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4)
{

    tANI_U8 buf[3000];
    buf[0] = '\0';
#ifdef FIXME_GEN6
    logRtaiDump(pMac, cmd, arg1, arg2, arg3, arg4, buf);
#endif
    buf[2999] = '\0';
    phyLog(pMac, LOGE, "logDump %d %x %x %x %x \n",  cmd, arg1, arg2, arg3, arg4);
    phyLog(pMac, LOGE, "logDump %s\n",  buf);

    return (SUCCESS);
}

#endif // ANI_MANF_DIAG


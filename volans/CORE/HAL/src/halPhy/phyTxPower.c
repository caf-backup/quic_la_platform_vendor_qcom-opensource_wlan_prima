/**
 *
   Airgo Networks, Inc proprietary.
   All Rights Reserved, Copyright 2008 2009
   This program is the confidential and proprietary product of Airgo Networks Inc.
   Any Unauthorized use, reproduction or transfer of this program is strictly prohibited.


   phyTxPower.cc: Tx Power Control functionality
   Author:  Mark Nelson
   Date:    12/9/2008
  --------------------------------------------------------------------------
 *
 */

#include "sys_api.h"


#ifdef ANI_PHY_DEBUG
extern const char rateStr[NUM_HAL_PHY_RATES][50];
#endif

extern const tANI_U8 plutCharacterized[MAX_TPC_CHANNELS][TPC_MEM_POWER_LUT_DEPTH];
extern const tANI_U8 plutCharacterizedVolans2[MAX_TPC_CHANNELS][TPC_MEM_POWER_LUT_DEPTH];

tTxGain tpcGainLut[PHY_MAX_TX_CHAINS][TPC_MEM_GAIN_LUT_DEPTH] =
{
    {   //tx0 gain LUT
        { 0x1, 0xf },
        { 0x0, 0xf },
        { 0x2, 0xf },
        { 0x3, 0x9 },

        { 0x3, 0xb },
        { 0x3, 0xd },
        { 0x3, 0xf },
        { 0x4, 0x9 },

        { 0x4, 0xb },
        { 0x4, 0xd },
        { 0x4, 0xf },
        { 0x5, 0x9 },

        { 0x5, 0xb },
        { 0x5, 0xd },
        { 0x5, 0xf },
        { 0x6, 0x9 },

        { 0x6, 0xb },
        { 0x6, 0xd },
        { 0x6, 0xf },
        { 0x7, 0x9 },

        { 0x7, 0xb }, //19.5 dBm
        { 0x7, 0xd }, //20 dBm
        { 0x7, 0xf },
        { 0xe, 0xb },

        { 0xe, 0xd },
        { 0xe, 0xf },
        { 0xf, 0x9 },
        { 0xf, 0xb },

        { 0xf, 0xd },
        { 0xf, 0xf },
        { 0xf, 0xf },
        { 0xf, 0xf }
    },
};


// eHalStatus ConfigureTpcFromNv(tpAniSirGlobal pMac)
// {
//     eHalStatus retVal = eHAL_STATUS_SUCCESS;
//     tTpcParams *tpcParams = NULL;
//     tTpcConfig *tpcConfig = NULL;
//     tTpcParams myTpcParams;
//
//     tpcParams = &myTpcParams;
//
//     //For Gen6, we are fixed at 2 frequencies and 4 cal points per freq.
//     myTpcParams.numTpcCalPointsPerFreq = 4;
//     myTpcParams.numTpcCalFreqs = 2;
//
//     //Note that this may be a reconfiguration after MTT removes TPC data and we need to correctly eliminate old TPC data
//     if (pMac->hphy.phyTPC.pwrCfgPresent == eANI_BOOLEAN_TRUE)
//     {
//         //previous configuration
//
//         // if (pMac->hphy.phyTPC.combinedBands.pwrInterp != NULL)
//         // {
//         //     if ((retVal = palFreeMemory(pMac->hHdd, pMac->hphy.phyTPC.combinedBands.pwrInterp)) != eHAL_STATUS_SUCCESS)
//         //     {
//         //         assert(0);
//         //         return (retVal);
//         //     }
//         // }
//         pMac->hphy.phyTPC.pwrCfgPresent = eANI_BOOLEAN_FALSE;
//     }
//
//     // The result of having the TPC configuration is that we will use it and set pwrCfgPresent to TRUE.
//     // If these tables don't exist, the driver will still continue, but with pwrCfgPresent set to FALSE.
//     // This logic is necessary to use the manfDiag driver and PTT GUI with a device where these have not calibrated yet.
//     if (//Keep for future (halGetNvTableLoc(pMac, NV_TABLE_TPC_PARAMS, (uNvTables **)&tpcParams) == eHAL_STATUS_SUCCESS) &&
//         (halGetNvTableLoc(pMac, NV_TABLE_TPC_CONFIG, (uNvTables **)&tpcConfig) == eHAL_STATUS_SUCCESS)
//        )
//     {
//         assert(tpcParams != NULL);
//         assert(tpcConfig != NULL);
//
//         if ((retVal = phyTxPowerConfig(pMac, tpcConfig, tpcParams->numTpcCalFreqs, tpcParams->numTpcCalPointsPerFreq, RF_BAND_2_4_GHZ)) == eHAL_STATUS_SUCCESS)
//         {
//             pMac->hphy.phyTPC.pwrCfgPresent = eANI_BOOLEAN_TRUE;
//         }
//         else
//         {
//             return (retVal);
//         }
//     }
//
//     {
//         tANI_U16 freq = rfChIdToFreqCoversion(rfGetChannelIdFromIndex(rfGetCurChannel(pMac)));
//
//         if ((retVal = phySetTxPower(pMac, freq, RF_SUBBAND_2_4_GHZ)) != eHAL_STATUS_SUCCESS)
//         {
//             return (retVal);
//         }
//     }
//
//     if (((retVal = halGetNvTableLoc(pMac, NV_TABLE_REGULATORY_DOMAINS, (uNvTables **)&pMac->hphy.phy.regDomainInfo)) != eHAL_STATUS_SUCCESS) ||
//         ((retVal = halGetNvTableLoc(pMac, NV_TABLE_RATE_POWER_SETTINGS, (uNvTables **)&pMac->hphy.phy.pwrOptimal)) != eHAL_STATUS_SUCCESS)
//          )
//     {
//           //these tables should have been stored before the power configuration, which is done in manufacturing
//           phyLog(pMac, LOGE, "ERROR: Could not find necessary transmit power information\n");
//
//           return (eHAL_STATUS_SUCCESS);   //return success anyway so driver loads - otherwise we won't be able to write these tables
//     }
//
//     return (retVal);
// }


eHalStatus phyTxPowerInit(tpAniSirGlobal pMac)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
    tANI_U8 *pPlut = NULL;

    if (RF_CHIP_VERSION(RF_CHIP_ID_VOLANS2))
    {
        tTpcPowerTable *plutCharacterizedVolans2 = (tTpcPowerTable *)(&pMac->hphy.nvCache.tables.plutCharacterized[0]);
        pPlut = (tANI_U8 *)&(plutCharacterizedVolans2[pMac->hphy.rf.curChannel][0][0]);
    }
    else
    {
        pPlut = (tANI_U8 *)&plutCharacterized[pMac->hphy.rf.curChannel];
    }

    if ((retVal = asicLoadTPCPowerLUT(pMac, PHY_TX_CHAIN_0,
                                            pPlut)) != eHAL_STATUS_SUCCESS)
    {
        return (retVal);
    }

    {
        sRFCalValues    *pRfCalValues;

        halGetNvTableLoc(pMac, NV_TABLE_RF_CAL_VALUES, (uNvTables **)&pRfCalValues);

        SET_PHY_REG(pMac->hHdd, QWLAN_TPC_PDADC_OFFSET_REG, (pRfCalValues->calData.hdet_dco + pMac->hphy.nvCache.tables.plutPdadcOffset[pMac->hphy.rf.curChannel]));
    }

    //tx power configuration should have preceded this
    SET_PHY_REG(pMac->hHdd, QWLAN_TPC_PDADC_SCALE_REG, 0);
    retVal = asicTPCAutomatic(pMac);
    if (retVal != eHAL_STATUS_SUCCESS)
    {
        return (retVal);
    }

    //load gain LUTs
    if (
        ((retVal = asicLoadTPCGainLUT(pMac, PHY_TX_CHAIN_0, &(tpcGainLut[PHY_TX_CHAIN_0][0]))) != eHAL_STATUS_SUCCESS)
       )
    {
        return (retVal);
    }

    if (pMac->gDriverType == eDRIVER_TYPE_PRODUCTION)
    {
        //halPhyGetPowerForRate relies on this to use the limits, even for the production build.
        pMac->hphy.phy.test.testTxGainIndexSource = REGULATORY_POWER_LIMITS;
    }

    return (retVal);
}




//#define INTERNAL_PATH_SEL   1
// eHalStatus phyTxPowerConfig(tpAniSirGlobal pMac, tTpcConfig *tpcConfig, tANI_U8 numFreqs, tANI_U8 numCalPointsPerFreq, eRfSubBand bandCfg)
// {
//     eHalStatus retVal = eHAL_STATUS_SUCCESS;
//     tANI_U32 channel, revId;
//     tPhyTxPowerBand *band;
//
//     if (!(numFreqs <= NUM_20MHZ_RF_CHANNELS) )
//     {
//         phyLog(pMac, LOGE, "ERROR: Incorrect Num freqs in Transmit Power Configuration\n");
//         return (eHAL_STATUS_FAILURE);
//     }
//     if (!(numCalPointsPerFreq == MAX_TPC_CAL_POINTS) )
//     {
//         phyLog(pMac, LOGE, "ERROR: Incorrect Num cal points in Transmit Power Configuration\n");
//         return (eHAL_STATUS_FAILURE);
//     }
//     if (!(bandCfg == RF_BAND_2_4_GHZ))
//     {
//         phyLog(pMac, LOGE, "ERROR: Incorrect band in Transmit Power Configuration\n");
//         return (eHAL_STATUS_FAILURE);
//     }
//
//     band = &pMac->hphy.phyTPC.combinedBands;
//
//     assert(numFreqs == 2);
//     // FOR GEN6, We are fixed at 2 frequencies, but keep this code for future
//     // if (numFreqs == 2)
//     // {
//     //     band->numTpcCalFreqs = numFreqs;
//     //     band->numTpcCalPointsPerFreq = numCalPointsPerFreq;
//     //     band->pwrSampled = tpcConfig;
//     // }
//     // else
//     // {
//     //     band->numTpcCalFreqs = 0;
//     //     band->numTpcCalPointsPerFreq = 0;
//     //     band->pwrSampled = NULL;
//     //     band->pwrInterp = NULL;
//     //     phyLog(pMac, LOGE, "Warning: !No Transmit Power Configuration\n");
//     //     assert(0);
//     //     return(eHAL_STATUS_FAILURE);
//     // }
//     //
//     //if ( (retVal = palAllocateMemory(pMac->hHdd, (void **)&band->pwrInterp, numFreqs * sizeof(tTpcPowerTable))) == eHAL_STATUS_SUCCESS)
//
//     //for GEN6, remove dynamic allocation of pwr data - use allocations in the same structure as the pointers.
//     band->pwrInterp = band->pwrInterpAllocation;
//     band->pwrSampled = band->pwrSampledAllocation;
//
//     //add protection to avoid corrupting various RFAPB registers due to a bug with the way the GC bus is handled
//     SET_PHY_REG(pMac->hHdd, QWLAN_RFIF_GC_CFG_REG, 0);
//
// #ifndef WLAN_HAL_VOLANS //FIXME_VOLANS
// #ifndef INTERNAL_PATH_SEL
//     rdModWrAsicField(pMac, QWLAN_RFAPB_HDET_CTL_REG,  QWLAN_RFAPB_HDET_CTL_HDET_PATH_SEL_MASK,  QWLAN_RFAPB_HDET_CTL_HDET_PATH_SEL_OFFSET,  QWLAN_RFAPB_HDET_CTL_HDET_PATH_SEL_EHDET_PATH_SEL_0 );
// #endif
//     rdModWrAsicField(pMac, QWLAN_RFAPB_HDET_CTL_REG,  QWLAN_RFAPB_HDET_CTL_EXT_ATTEN_3_0_MASK,  QWLAN_RFAPB_HDET_CTL_EXT_ATTEN_3_0_OFFSET,  pMac->hphy.nvCache.tables.rfCalValues.hdet_ctl_ext_atten  );
//     rdModWrAsicField(pMac, QWLAN_RFAPB_HDET_DCOC_REG, QWLAN_RFAPB_HDET_DCOC_DCOC_CODE_5_0_MASK, QWLAN_RFAPB_HDET_DCOC_DCOC_CODE_5_0_OFFSET, pMac->hphy.nvCache.tables.rfCalValues.hdet_dcoc_code      );
// #endif //WLAN_HAL_VOLANS
//     rdModWrAsicField(pMac, QWLAN_RFAPB_HDET_DCOC_REG, QWLAN_RFAPB_HDET_DCOC_IB_RCAL_EN_MASK,    QWLAN_RFAPB_HDET_DCOC_IB_RCAL_EN_OFFSET,    pMac->hphy.nvCache.tables.rfCalValues.hdet_dcoc_ib_rcal_en);
//     rdModWrAsicField(pMac, QWLAN_RFAPB_HDET_DCOC_REG, QWLAN_RFAPB_HDET_DCOC_IB_SCAL_EN_MASK,    QWLAN_RFAPB_HDET_DCOC_IB_SCAL_EN_OFFSET,    pMac->hphy.nvCache.tables.rfCalValues.hdet_dcoc_ib_scal_en);
//     rdModWrAsicField(pMac, QWLAN_RFAPB_HDET_DCOC_REG, QWLAN_RFAPB_HDET_DCOC_HDET_ENABLE_MASK,   QWLAN_RFAPB_HDET_DCOC_HDET_ENABLE_OFFSET,   1);
//     rdModWrAsicField(pMac, QWLAN_RFAPB_HDET_DCOC_REG, QWLAN_RFAPB_HDET_DCOC_OVRD_HDET_MASK,     QWLAN_RFAPB_HDET_DCOC_OVRD_HDET_OFFSET,     1);
// #ifdef INTERNAL_PATH_SEL
//     rdModWrAsicField(pMac, QWLAN_RFAPB_HDET_CTL_REG,  QWLAN_RFAPB_HDET_CTL_INT_ATTEN_3_0_MASK,  QWLAN_RFAPB_HDET_CTL_INT_ATTEN_3_0_OFFSET,  pMac->hphy.nvCache.tables.rfCalValues.hdet_ctl_ext_atten  );
// #endif
//
//     GET_PHY_REG(pMac->hHdd, QWLAN_RFAPB_REV_ID_REG, &revId);
//     SET_PHY_REG(pMac->hHdd, QWLAN_RFIF_GC_CFG_REG, (QWLAN_RFIF_GC_CFG_TX_GAIN_EN_MASK | QWLAN_RFIF_GC_CFG_RX_GAIN_EN_MASK));
//
//     {
//         //memory allocated to hold interpolated data for each sampled channel
//         for (channel = 0; channel < numFreqs; channel++)
//         {
//             memcpy(&(band->pwrSampled[channel]), &(tpcConfig[channel]), sizeof(tTpcConfig));
//
//             InterpolateCalPowerPoints(pMac, channel, bandCfg);
//         }
//
//         {
//             pMac->hphy.phy.test.testTxGainIndexSource = RATE_POWER_NON_LIMITED;
//
//             //find out if TPC cal tables are in NV
//             if (!(
//                   //(eHAL_STATUS_SUCCESS == halIsTableInNv(pMac, NV_TABLE_TPC_PARAMS)) &&
//                   (eHAL_STATUS_SUCCESS == halIsTableInNv(pMac, NV_TABLE_TPC_CONFIG))
//                   )
//                )
//             {
//                 //we don't have a closed-loop configuration in NV - new device - keep power control as open loop
//                 pMac->hphy.phy.test.testTpcClosedLoop = eANI_BOOLEAN_FALSE;
//
// #if !defined(ANI_MANF_DIAG)
//                 phyLog(pMac, LOGE, "ERROR: No CLPC Calibration in NV\n");
// #endif
//             }
//             else
//             {
//                 //we have a closed-loop configuration in NV - close the power control loop
// #if !defined(ANI_MANF_DIAG)
//                 pMac->hphy.phy.test.testTpcClosedLoop = eANI_BOOLEAN_TRUE;
// #endif
//             }
//         }
//     }
//     return(retVal);
// }

// eHalStatus phyTxPowerClose(tpAniSirGlobal pMac)
// {
//     eHalStatus retVal = eHAL_STATUS_SUCCESS;
//
//     if (pMac->hphy.phyTPC.pwrCfgPresent == eANI_BOOLEAN_TRUE)
//     {
//         //previous configuration - free pwrInterp memory and it will be reallocated accordingly
//
//         // if (pMac->hphy.phyTPC.combinedBands.pwrInterp != NULL)
//         // {
//         //     if ( (retVal = palFreeMemory(pMac->hHdd, pMac->hphy.phyTPC.combinedBands.pwrInterp)) != eHAL_STATUS_SUCCESS)
//         //     {
//         //         assert(0);
//         //         return(retVal);
//         //     }
//         // }
//
//         pMac->hphy.phyTPC.pwrCfgPresent = eANI_BOOLEAN_FALSE;
//     }
//
//     return(retVal);
// }

// static void InterpolateCalPowerPoints(tpAniSirGlobal pMac, tANI_U32 tpcChannel, eRfSubBand bandCfg)
// {
//     tANI_U8 calPoint;
//     tANI_U32 chain;
//     tTpcCaldPowerPoint *lastPoint;
//     tPhyTxPowerBand *band;
//     tANI_U8 highestTxChain =  PHY_MAX_TX_CHAINS; //halPhyQueryNumTxChains(pMac->hphy.phy.cfgChains);
//
//     band = &pMac->hphy.phyTPC.combinedBands;
//
//     assert(band->pwrSampled != NULL);
//     assert(band->pwrInterp != NULL);    //should have been previously allocated
//
//
//     for (chain = PHY_TX_CHAIN_0; chain < highestTxChain; chain++)
//     {
//         tANI_U8 point;
//         tPowerDetect x1, x2;
//         tTpcLutValue y1, y2;
//
//         {
//             //fill the power LUT values
//             for (point = 0; point <= band->pwrSampled[tpcChannel].empirical[chain][0].pwrDetAdc; point++)
//             {
//                 //fill all values preceding the first cal point with the adjusted value from the first point
//                 band->pwrInterp[tpcChannel][chain][point] = band->pwrSampled[tpcChannel].empirical[chain][0].adjustedPwrDet;
//                 //GET_FULL_PRECISION_TPC_LUT_VALUE(band->pwrSampled[tpcChannel].empirical[chain][0].adjustedPwrDet,
//                 //                                 band->pwrSampled[tpcChannel].empirical[chain][0].extraPrecision.hi8_adjustedPwrDet
//                 //                                );
//             }
//
//             //for (calPoint = 0; calPoint < (band->numTpcCalPointsPerFreq - 1); calPoint++)
//             for (calPoint = 0; calPoint < (MAX_TPC_CAL_POINTS - 1); calPoint++)
//             {
//                 tPowerDetect interpPoint;
//
//                 x1 = band->pwrSampled[tpcChannel].empirical[chain][calPoint].pwrDetAdc;
//                 x2 = band->pwrSampled[tpcChannel].empirical[chain][calPoint + 1].pwrDetAdc;
//
//                 //tTpcLutValue y1 = GET_FULL_PRECISION_TPC_LUT_VALUE(band->pwrSampled[tpcChannel].empirical[chain][calPoint].adjustedPwrDet,
//                 //                                                   band->pwrSampled[tpcChannel].empirical[chain][calPoint].extraPrecision.hi8_adjustedPwrDet
//                 //                                                  );
//                 y1 = band->pwrSampled[tpcChannel].empirical[chain][calPoint].adjustedPwrDet;
//                 //tTpcLutValue y2 = GET_FULL_PRECISION_TPC_LUT_VALUE(band->pwrSampled[tpcChannel].empirical[chain][calPoint + 1].adjustedPwrDet,
//                 //                                                   band->pwrSampled[tpcChannel].empirical[chain][calPoint + 1].extraPrecision.hi8_adjustedPwrDet
//                 //                                                  );
//                 y2 = band->pwrSampled[tpcChannel].empirical[chain][calPoint + 1].adjustedPwrDet;
//
//                 assert(x2 >= x1);
//
//                 band->pwrInterp[tpcChannel][chain][x1] = y1;
//                 band->pwrInterp[tpcChannel][chain][x2] = y2;
//
//                 if (x1 == 0xff) //the last point, stop interpolation
//                     break;
//
//                 for (interpPoint = x1 + 1; interpPoint < x2; interpPoint++)
//                 {
//                     //fill in interpolated adjustedPwrDet values between calibrated points
//                     band->pwrInterp[tpcChannel][chain][interpPoint] =
//                     (tTpcLutValue)InterpolateBetweenPoints(x1, y1, x2, y2, interpPoint);
//                 }
//             }
//
//             lastPoint =
//             &(band->pwrSampled[tpcChannel].empirical[chain][MAX_TPC_CAL_POINTS - 1]);
//             //&(band->pwrSampled[tpcChannel].empirical[chain][band->numTpcCalPointsPerFreq - 1]);
//
//             for (point = lastPoint->pwrDetAdc; point < TPC_MEM_POWER_LUT_DEPTH; point++)
//             {
//                 //extrapolate all values following the last cal point with the same slope of the last two cal points
//                 band->pwrInterp[tpcChannel][chain][point] = (tTpcLutValue)InterpolateBetweenPoints(x1, y1, x2, y2, point);
//                 if(band->pwrInterp[tpcChannel][chain][point] > (TPC_MEM_POWER_LUT_DEPTH - 1))
//                 {
//                     band->pwrInterp[tpcChannel][chain][point] = TPC_MEM_POWER_LUT_DEPTH - 1;
//                 }
//
//                 //GET_FULL_PRECISION_TPC_LUT_VALUE(lastPoint->adjustedPwrDet, lastPoint->extraPrecision.hi8_adjustedPwrDet);
//             }
//         }
//     }
// }

// eHalStatus phySetTxPower(tpAniSirGlobal pMac, tANI_U32 freq, eRfSubBand bandCfg)
// {
//     eHalStatus retVal = eHAL_STATUS_SUCCESS;
// #ifndef VOLANS_RF
//     tANI_U32 channel;
//     tANI_U32 lowFreq;
//     tANI_U32 lowestFreq;
//     tANI_U32 highFreq;
//     tANI_U32 highestFreq;
//     tPhyTxPowerBand *band;
//
//     band = &pMac->hphy.phyTPC.combinedBands;
//
//     assert(band != NULL);
//
//     if (pMac->hphy.phyTPC.pwrCfgPresent == eANI_BOOLEAN_FALSE)
// #if !defined(ANI_MANF_DIAG) || defined(ANI_DVT_DEBUG)
//     {
//         //no tx power configuration - see corresponding logic in halPhyGetPowerForRate
//         if (pMac->hphy.phy.test.testDisableRfAccess == eANI_BOOLEAN_FALSE)
//         {
//             return(eHAL_STATUS_FAILURE);   //for non-mfg drivers, we want to fail here because a TPC config is needed
//         }
//         else
//         {
//             return(eHAL_STATUS_SUCCESS);   // non-RF capable boards can be allowed to continue
//         }
//     }
// #else
//     {
//         return(eHAL_STATUS_SUCCESS);   //for the ANI_MANF_DIAG driver, we want to continue even if the TPC cal is not configured
//     }
// #endif
//     else if ( //(band->numTpcCalFreqs < 2) ||
//               (band->pwrInterp == NULL) ||
//               (band->pwrSampled == NULL)
//             )
//     {
//         phyLog(pMac, LOGE, "ERROR: Band not configured for transmit power - Requires manufacturing calibration\n");
//         return(eHAL_STATUS_SUCCESS);   //return success anyway
//     }
//
//     //lowestFreq =   rfChIdToFreqCoversion(
//     //               rfGetChannelIdFromIndex(
//     //               rfGetIndexFromFreq((tANI_U16)band->pwrSampled[0].freq, PHY_SINGLE_CHANNEL_CENTERED)));
//     lowestFreq = band->pwrSampled[0].freq;
//
//     if (freq < lowestFreq)
//     {
//         freq = lowestFreq;  //this will force the lowest freq's cal data to be used
//     }
//
//     //highestFreq =  rfChIdToFreqCoversion(
//     //               rfGetChannelIdFromIndex(
//     //               rfGetIndexFromFreq(
//     //                (tANI_U16)band->pwrSampled[((tANI_U32)band->numTpcCalFreqs - 1)].freq, PHY_SINGLE_CHANNEL_CENTERED)));
//     highestFreq = band->pwrSampled[MAX_TPC_CHANNELS - 1].freq;
//
//     if (freq > highestFreq)
//     {
//         freq = highestFreq;  //this will force the highest freq's cal data to be used
//     }
//
//     //first, find the two cal channels that encompass the requested frequency
//     //for (channel = 0; channel < ((tANI_U32)band->numTpcCalFreqs - 1); channel++)
//     for (channel = 0; channel < ((tANI_U32)MAX_TPC_CHANNELS - 1); channel++)
//     {
//         // lowFreq =   rfChIdToFreqCoversion(
//         //                 rfGetChannelIdFromIndex(
//         //                     rfGetIndexFromFreq((tANI_U16)band->pwrSampled[channel].freq, PHY_SINGLE_CHANNEL_CENTERED)));
//         //
//         // highFreq =  rfChIdToFreqCoversion(
//         //                 rfGetChannelIdFromIndex(
//         //                     rfGetIndexFromFreq((tANI_U16)band->pwrSampled[channel + 1].freq, PHY_SINGLE_CHANNEL_CENTERED)));
//         lowFreq = band->pwrSampled[channel].freq;
//         highFreq = band->pwrSampled[channel + 1].freq;
//
//         if ( (freq >= lowFreq) && (freq <= highFreq) )
//         {
//             //found encompassing calibrated channels
//             if (freq == lowFreq)
//             {
//                 //use lowFreq interpolated power table
//                 memcpy(&(pMac->hphy.phyTPC.curTpcPwrLUT[PHY_TX_CHAIN_0][0]),
//                        &(band->pwrInterp[channel][PHY_TX_CHAIN_0][0]),
//                        TPC_MEM_POWER_LUT_DEPTH * sizeof(tTpcLutValue));
//             }
//             else if (freq == highFreq)
//             {
//                 //use highFreq interpolated power table
//                 memcpy(&(pMac->hphy.phyTPC.curTpcPwrLUT[PHY_TX_CHAIN_0][0]),
//                        &(band->pwrInterp[channel + 1][PHY_TX_CHAIN_0][0]),
//                        TPC_MEM_POWER_LUT_DEPTH * sizeof(tTpcLutValue));
//
//             }
//             else
//             {
//                 //interpolate between frequencies
//                 InterpolateCalChannels(pMac, channel, channel + 1, freq, bandCfg);
//             }
//             break;  //freq found and curTpcPwrLUT filled, break out of loop
//         }
//     }
//
//     {
//         //Here's where we divide out the extra precision bits before loading them into the LUTs
//         tANI_U32 i;
//         tPowerDetect lutValues[PHY_MAX_TX_CHAINS][TPC_MEM_POWER_LUT_DEPTH];
//
//         for (i = 0; i < TPC_MEM_POWER_LUT_DEPTH; i++)
//         {
//             lutValues[PHY_TX_CHAIN_0][i] = (tPowerDetect)(pMac->hphy.phyTPC.curTpcPwrLUT[PHY_TX_CHAIN_0][i]);// / EXTRA_TPC_LUT_MULT);
//
//             assert(lutValues[PHY_TX_CHAIN_0][i] < TPC_MEM_POWER_LUT_DEPTH);
//         }
//
//
//         if (
//            ((retVal = asicLoadTPCPowerLUT(pMac, PHY_TX_CHAIN_0, &(lutValues[PHY_TX_CHAIN_0][0]))) != eHAL_STATUS_SUCCESS)
//            )
//         {
//             return(retVal);
//         }
//
//
//     }
// #endif
//     return(retVal);
// }

// #ifndef VOLANS_RF
// static void InterpolateCalChannels(tpAniSirGlobal pMac, tANI_U32 lowTpcChan, tANI_U32 highTpcChan, tANI_U32 interpFreq, eRfSubBand bandCfg)
// {
//     tANI_U8 point;
//     tANI_U32 chain;
//
//     tANI_U8 highestTxChain = PHY_MAX_TX_CHAINS; //halPhyQueryNumTxChains(pMac->hphy.phy.cfgChains);
//     tPhyTxPowerBand *band;
//
//     band = &pMac->hphy.phyTPC.combinedBands;
//
//     assert(band != NULL);
//
//     assert(band->pwrInterp != NULL);
//     assert(band->pwrSampled != NULL);
//
//     for (chain = PHY_TX_CHAIN_0; chain < highestTxChain; chain++)
//     {
//         for (point = 0; point < TPC_MEM_POWER_LUT_DEPTH; point++)
//         {
//             //interpolate power LUT values for the freq
//             tANI_U32 x1 = band->pwrSampled[lowTpcChan].freq;
//             tANI_U32 x2 = band->pwrSampled[highTpcChan].freq;
//             tTpcLutValue y1 = band->pwrInterp[lowTpcChan][chain][point];
//             tTpcLutValue y2 = band->pwrInterp[highTpcChan][chain][point];
//
//             pMac->hphy.phyTPC.curTpcPwrLUT[chain][point] =
//             (tTpcLutValue)InterpolateBetweenPoints(x1, (tANI_S32)y1, x2, (tANI_S32)y2, (tANI_S32)interpFreq);
//         }
//     }
// }
// #endif



//-0.25dBm adjustment to be made to compensate b rates, because we cal with OFDM rates
#define B_RATE_CAL_ADJUSTMENT               -150
#define B_RATE_CAL_ADJUSTMENT_VOLANS2       -100
#define GN_RATE_BANDEDGE_ADJUSTMENT         -100
#define GN_RATE_BANDEDGE_ADJUSTMENT_VOLANS2 0

#define TPC_INDEX_WIFI_DIRECT   0
#ifdef VOLANS_1_0_CLPC_WORKAROUND
#define TPC_INDEX_LOW_POWER     2
#else
#define TPC_INDEX_LOW_POWER     1
#endif

#define PACKET_TYPE_POWER_LIMIT(ptr, mode, chan)  (*(((t2Decimal *)(ptr)) + ((mode) * (NUM_2_4GHZ_CHANNELS)) + (chan)))

eHalStatus halPhyGetPowerForRate(tHalHandle hHal, eHalPhyRates rate, ePowerMode pwrMode, tPowerdBm absPwrLimit, tPwrTemplateIndex *retTemplateIndex)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal) hHal;
    eRfSubBand rfSubband;
    t2Decimal absPwrLimit_2dec;
    t2Decimal pktTypePwrLimit = 0;

    ePhyChanBondState cbState;

    eRfChannels curChan = rfGetCurChannel(pMac);

    cbState = pMac->hphy.phy.chanBondState;

    assert(curChan != INVALID_RF_CHANNEL);
    assert(pwrMode < POWER_MODE_INVALID);

    switch (pwrMode)
    {
        case POWER_MODE_LOW_POWER:
            *retTemplateIndex = TPC_INDEX_LOW_POWER;
            return(eHAL_STATUS_SUCCESS);
        case POWER_MODE_WIFI_DIRECT:
            *retTemplateIndex = TPC_INDEX_WIFI_DIRECT;
            return(eHAL_STATUS_SUCCESS);
        default:
            break;
    }

#ifndef WLAN_FTM_STUB
    if (pMac->gDriverType == eDRIVER_TYPE_MFG)
    {
        if (pMac->hphy.phy.test.testTxGainIndexSource == FORCE_POWER_TEMPLATE_INDEX)
        {
            pMac->hphy.phy.test.testLastPwrIndex = pMac->hphy.phy.test.testForcedTxGainIndex;
            *retTemplateIndex = pMac->hphy.phy.test.testLastPwrIndex;
            return(eHAL_STATUS_SUCCESS);
        }
    }
/*
    else if (pMac->hphy.phyTPC.pwrCfgPresent == eANI_BOOLEAN_FALSE)
    {
        //no CLPC data - must return
        *retTemplateIndex = 0;
        phyLog(pMac, LOGE, "ERROR: No Tx Power Config available to calculate power settings from!");
        return(eHAL_STATUS_SUCCESS);   // non-RF capable boards can be allowed to continue
    }
*/

#else
/*
    if (pMac->hphy.phyTPC.pwrCfgPresent == eANI_BOOLEAN_FALSE)
    {
        *retTemplateIndex = 0;
        phyLog(pMac, LOGE, "ERROR: No Tx Power Config available to calculate power settings from!");

        if (pMac->hphy.phy.test.testDisableRfAccess == eANI_BOOLEAN_FALSE)
        {
            return(eHAL_STATUS_FAILURE);   // The RF board requires a Tx Power config
        }
        else
        {
            return(eHAL_STATUS_SUCCESS);   // non-RF capable boards can be allowed to continue
        }
    }
*/
#endif

    if (pMac->hphy.phy.pwrOptimal == NULL)
    {
        phyLog(pMac, LOGE, "ERROR: NV_TABLE_RATE_POWER_SETTINGS not found");
        return(eHAL_STATUS_FAILURE);
    }
    if (pMac->hphy.phy.regDomainInfo == NULL)
    {
        phyLog(pMac, LOGE, "ERROR: NV_TABLE_REGULATORY_DOMAINS not found");
        return(eHAL_STATUS_FAILURE);
    }

    switch (curChan)
    {
        //2.4GHz Band
        case RF_CHAN_1:
        case RF_CHAN_2:
        case RF_CHAN_3:
        case RF_CHAN_4:
        case RF_CHAN_5:
        case RF_CHAN_6:
        case RF_CHAN_7:
        case RF_CHAN_8:
        case RF_CHAN_9:
        case RF_CHAN_10:
        case RF_CHAN_11:
        case RF_CHAN_12:
        case RF_CHAN_13:
        case RF_CHAN_14:
            assert(cbState == PHY_SINGLE_CHANNEL_CENTERED);
            rfSubband = RF_SUBBAND_2_4_GHZ;
            break;

        default:
            assert(0);
            return(eHAL_STATUS_FAILURE);
            break;
    }

    absPwrLimit_2dec = CONVERT_TO_2DECIMAL_PLACES(absPwrLimit);

    {
        //now determine power template value to use for the requested rate on the current channel
        tANI_U8 numTxAntennas;
        t2Decimal absPwr;
        tPowerDetect relPwr;

        //desired power for a single antenna
        {
            t2Decimal desiredPower;

            desiredPower = pMac->hphy.phy.pwrOptimal[rfSubband][rate].reported;
            absPwr = desiredPower;
        }


        numTxAntennas = 1;
        //KEEP THIS CODE FOR FUTURE
        // numTxAntennas = halPhyQueryNumTxChains(pMac->hphy.phy.activeChains);
        //
        // //we must account for antenna gains here because this is outside of the calibrated power table
        // //first account for the number of antennas transmitting at the same conducted power
        // switch (numTxAntennas)
        // {
        //     case 0:
        //         *retTemplateIndex = 0;
        //         return (eHAL_STATUS_SUCCESS);
        //     case 1:
        //         //no adjustment for 1 chain
        //         break;
        //     case 2:
        //         absPwrLimit_2dec -= 301; //-3dBm to account for 2 antennas instead of 1
        //         break;
        //     case 3:
        //         absPwrLimit_2dec -= 477; //-4.77dBm to account for 3 antennas instead of 1
        //         break;
        //     case 4:
        //         absPwrLimit_2dec -= 602; //-6.02dBm to account for 4 antennas instead of 1
        //         break;
        //     default:
        //         assert(0);
        //         return (eHAL_STATUS_FAILURE);
        // }

        //account for average array gain
        //absPwrLimit_2dec -= pMac->hphy.phy.regDomainInfo[pMac->hphy.phy.curRegDomain].antennaGain[rfSubband].reported;
        //account for antenna path loss per channel
        absPwrLimit_2dec += pMac->hphy.phy.antennaPathLoss[curChan];

        /*
         * For the proximity set to ON for all rates the power is 0
         */
        // if (pMac->hphy.densityEnabled == eANI_BOOLEAN_FALSE)
        // {
        //     printk(KERN_ERR "densityEnabled = eANI_BOOLEAN_FALSE\n");
        //     absPwr = 0;
        // }
        // else

        //now we have absolute powers comparable for a single tx chain
        if (absPwr > absPwrLimit_2dec)
        {
            //limit power to regulatory domain
            absPwr = absPwrLimit_2dec;
        }

#ifndef WLAN_FTM_STUB
        if (pMac->gDriverType == eDRIVER_TYPE_MFG)
        {
            if (pMac->hphy.phy.test.testTxGainIndexSource == FIXED_POWER_DBM)
            {
                absPwr = pMac->hphy.phy.test.testForcedTxPower;   //override power per rate setting
                //absPwr += pMac->hphy.phy.antennaPathLoss[curChan];
                //for fixed power selection, we do not want regulatory limits to apply
               //they are mutually exclusive
            }
        }
#endif

        if (rate > NUM_HAL_PHY_RATES)
        {
            assert(0);
            return(eHAL_STATUS_FAILURE);
        }
        else
        {
            switch (rate)
            {
                // 11B
                case HAL_PHY_RATE_11B_LONG_1_MBPS:
                case HAL_PHY_RATE_11B_LONG_2_MBPS:
                case HAL_PHY_RATE_11B_LONG_5_5_MBPS:
                case HAL_PHY_RATE_11B_LONG_11_MBPS:
                case HAL_PHY_RATE_11B_SHORT_2_MBPS:
                case HAL_PHY_RATE_11B_SHORT_5_5_MBPS:
                case HAL_PHY_RATE_11B_SHORT_11_MBPS:
                //  SLR
                case HAL_PHY_RATE_SLR_0_25_MBPS:
                case HAL_PHY_RATE_SLR_0_5_MBPS:

                    if (RF_CHIP_VERSION(RF_CHIP_ID_VOLANS2))
                    {
                        if (pMac->gDriverType == eDRIVER_TYPE_PRODUCTION)
                        {
                            pktTypePwrLimit = PACKET_TYPE_POWER_LIMIT(pMac->hphy.phy.pktTypePwrLimits, MODE_802_11B, curChan);

                            if (absPwr > pktTypePwrLimit)
                            {
                                absPwr = pktTypePwrLimit;
                            }
                        }

                        absPwr += B_RATE_CAL_ADJUSTMENT_VOLANS2;
                    }
                    else
                    {
                        absPwr += B_RATE_CAL_ADJUSTMENT;
                    }
                    break;

                //Spica_Virgo 11A 20MHz Rates
                case HAL_PHY_RATE_11A_6_MBPS:
                case HAL_PHY_RATE_11A_9_MBPS:
                case HAL_PHY_RATE_11A_12_MBPS:
                case HAL_PHY_RATE_11A_18_MBPS:
                case HAL_PHY_RATE_11A_24_MBPS:
                case HAL_PHY_RATE_11A_36_MBPS:
                case HAL_PHY_RATE_11A_48_MBPS:
                case HAL_PHY_RATE_11A_54_MBPS:
                    if (RF_CHIP_VERSION(RF_CHIP_ID_VOLANS2))
                    {
                        if (pMac->gDriverType == eDRIVER_TYPE_PRODUCTION)
                        {
                            pktTypePwrLimit = PACKET_TYPE_POWER_LIMIT(pMac->hphy.phy.pktTypePwrLimits, MODE_802_11AG, curChan);

                            if (absPwr > pktTypePwrLimit)
                            {
                                absPwr = pktTypePwrLimit;
                            }
                        }
                    }

                    if ((curChan == RF_CHAN_1) || (curChan == RF_CHAN_11))
                    {
                        if (RF_CHIP_VERSION(RF_CHIP_ID_VOLANS2))
                        {
                            absPwr += GN_RATE_BANDEDGE_ADJUSTMENT_VOLANS2;
                        }
                        else
                        {
                            absPwr += GN_RATE_BANDEDGE_ADJUSTMENT;
                        }
                    }

                    //if (pMac->gDriverType == eDRIVER_TYPE_PRODUCTION)
                    {
                        absPwr -= pMac->hphy.nvCache.tables.ofdmCmdPwrOffset.ofdmPwrOffset;
                    }
                    break;

                //MCS Index #0-15 (20MHz)
                case HAL_PHY_RATE_MCS_1NSS_6_5_MBPS:
                case HAL_PHY_RATE_MCS_1NSS_13_MBPS:
                case HAL_PHY_RATE_MCS_1NSS_19_5_MBPS:
                case HAL_PHY_RATE_MCS_1NSS_26_MBPS:
                case HAL_PHY_RATE_MCS_1NSS_39_MBPS:
                case HAL_PHY_RATE_MCS_1NSS_52_MBPS:
                case HAL_PHY_RATE_MCS_1NSS_58_5_MBPS:
                case HAL_PHY_RATE_MCS_1NSS_65_MBPS:
                case HAL_PHY_RATE_MCS_1NSS_MM_SG_7_2_MBPS:
                case HAL_PHY_RATE_MCS_1NSS_MM_SG_14_4_MBPS:
                case HAL_PHY_RATE_MCS_1NSS_MM_SG_21_7_MBPS:
                case HAL_PHY_RATE_MCS_1NSS_MM_SG_28_9_MBPS:
                case HAL_PHY_RATE_MCS_1NSS_MM_SG_43_3_MBPS:
                case HAL_PHY_RATE_MCS_1NSS_MM_SG_57_8_MBPS:
                case HAL_PHY_RATE_MCS_1NSS_MM_SG_65_MBPS:
                case HAL_PHY_RATE_MCS_1NSS_MM_SG_72_2_MBPS:
                    if (RF_CHIP_VERSION(RF_CHIP_ID_VOLANS2))
                    {
                        if (pMac->gDriverType == eDRIVER_TYPE_PRODUCTION)
                        {
                            pktTypePwrLimit = PACKET_TYPE_POWER_LIMIT(pMac->hphy.phy.pktTypePwrLimits, MODE_802_11N, curChan);

                            if (absPwr > pktTypePwrLimit)
                            {
                                absPwr = pktTypePwrLimit;
                            }
                        }
                    }

                    if ((curChan == RF_CHAN_1) || (curChan == RF_CHAN_11))
                    {
                        if (RF_CHIP_VERSION(RF_CHIP_ID_VOLANS2))
                        {
                            absPwr += GN_RATE_BANDEDGE_ADJUSTMENT_VOLANS2;
                        }
                        else
                        {
                            absPwr += GN_RATE_BANDEDGE_ADJUSTMENT;
                        }
                    }

                    //if (pMac->gDriverType == eDRIVER_TYPE_PRODUCTION)
                    {
                        absPwr -= pMac->hphy.nvCache.tables.ofdmCmdPwrOffset.ofdmPwrOffset;
                    }
                    break;

                default:
                    break;
            }
        }

        {
            //y = mx + c where y is the TPC Gain LUT Index, m is the slope of the
            //Power-GainLUT curve, c is the the minimum power, x is the power in dBm.
            //m = (y2 - y1)/(x2 - x1) = (31 - 1)/(24 - 9) = 2
            //c = y1 - mx1 = 1 - 2*9 = -17
            tANI_S32 slope = TPC_GAIN_LUT_PWR_SLOPE;                                //m
            tANI_S32 yIntercept = (MIN_TPC_GAIN_INDEX * 100) - (slope * MIN_TPC_GAIN_LUT_DBM_2DEC_PLACES); //c * 100

            if (absPwr < MIN_TPC_GAIN_LUT_DBM_2DEC_PLACES)
            {
                absPwr = MIN_TPC_GAIN_LUT_DBM_2DEC_PLACES;
            }
            else if (absPwr > MAX_TPC_GAIN_LUT_DBM_2DEC_PLACES)
            {
                absPwr = MAX_TPC_GAIN_LUT_DBM_2DEC_PLACES;
            }

            //relPwr is y and absPwr is x
            relPwr =(tPowerDetect)(((slope * absPwr) + yIntercept)/100);
            phyLog(pMac, LOG1, "relPwr = %d\n", relPwr);

#ifdef ANI_PHY_DEBUG
            phyLog(pMac, LOG2, "%s  Pwrlimit: %d.%d  absPwr %d.%d  template:%d\n",
                   &rateStr[rate][0],
                   absPwrLimit_2dec / 100, absPwrLimit_2dec % 100,
                   (absPwr) / 100, (absPwr) % 100,
                   relPwr
                  );
#endif

#ifndef WLAN_FTM_STUB
            if (pMac->gDriverType == eDRIVER_TYPE_MFG)
                pMac->hphy.phy.test.testLastPwrIndex = relPwr;
#endif

            *retTemplateIndex = relPwr;
            return(eHAL_STATUS_SUCCESS);
        }
    }
}


const t_mW lookup_mWatts[TPC_MEM_GAIN_LUT_DEPTH] =
{
    //mW    //dBm
    6,      // 8
    7,      // 8.5
    8,      // 9
    9,      // 9.5
    10,     // 10
    11,     // 10.5
    13,     // 11
    14,     // 11.5
    16,     // 12
    18,     // 12.5
    20,     // 13
    22,     // 13.5
    25,     // 14
    28,     // 14.5
    32,     // 15
    35,     // 15.5
    40,     // 16
    45,     // 16.5
    50,     // 17
    56,     // 17.5
    63,     // 18
    71,     // 18.5
    79,     // 19
    89,     // 19.5
    100,    // 20
    112,    // 20.5
    126,    // 21
    141,    // 21.5
    158,    // 22
    178,    // 22.5
    200,    // 23
    224     // 23.5
};

#define PRESERVED_BITS (128)


#define NUM_GAIN_MULT_INDICES 41


const tANI_U16 lookup_mW_Multiplier[NUM_GAIN_MULT_INDICES] =
{
    //these are scaled externally by PRESERVED_BITS
    12,     // -10
    14,     // -9.5
    16,     // -9
    17,     // -8.5
    20,     // -8
    23,     // -7.5
    25,     // -7
    28,     // -6.5
    32,     // -6
    35,     // -5.5
    40,     // -5
    44,     // -4.5
    51,     // -4
    57,     // -3.5
    64,     // -3
    71,     // -2.5
    80,     // -2
    90,     // -1.5
    101,    // -1
    113,    // -0.5
    128,    // 0
    143,    // 0.5
    161,    // 1
    180,    // 1.5
    202,    // 2
    227,    // 2.5
    256,    // 3
    286,    // 3.5
    321,    // 4
    360,    // 4.5
    404,    // 5
    454,    // 5.5
    509,    // 6
    572,    // 6.5
    641,    // 7
    719,    // 7.5
    807,    // 8
    906,    // 8.5
    1016,   // 9
    1140,   // 9.5
    1280    // 10
};

//db passed as t2Decimal value
#define GET_MW_MULT_INDEX(db) (db < -1000 ? 0 :                                            \
                               (db > 1000 ? (NUM_GAIN_MULT_INDICES - 1) :                  \
                                (((NUM_GAIN_MULT_INDICES - 1) >> 1) + ((db/100) * 2))      \
                               )                                                           \
                              )

eHalStatus halPhySetTxMilliWatts(tHalHandle hHal, t_mW mWatts, tPwrTemplateIndex *retTemplateIndex)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal) hHal;
    tANI_U32 i;
#ifdef MULTIPLE_TRANSMIT_CHAINS
    tANI_U8 numTxAntennas;
#endif
    t2Decimal arrayGain = 0;
    tANI_U8 m;
    eRfSubBand rfSubband;
    eRfChannels curChan = rfGetCurChannel(pMac);

    if (curChan != INVALID_RF_CHANNEL)
    {
        rfSubband = rfGetBand(pMac, curChan);
    }
    else
    {
        return(eHAL_STATUS_FAILURE);
    }

#ifdef MULTIPLE_TRANSMIT_CHAINS
    //account for number of antennas (if 2, then divide mW by 2, which is equivalent to -3dB)
    // and average antenna array gain
    numTxAntennas = halPhyQueryNumTxChains(pMac->hphy.phy.activeChains);
#endif
    arrayGain = pMac->hphy.phy.regDomainInfo[pMac->hphy.phy.curRegDomain].antennaGain[rfSubband].reported;

    //calc index into lookup_mW_Multiplier for array gain multiplier
    //we want the inverse of arrayGain.
    //i.e. if arrayGain is 3dB, then we want to pick an index 3dB less
    m = (tANI_U8)GET_MW_MULT_INDEX(-arrayGain);
    mWatts = (mWatts * lookup_mW_Multiplier[m]) / PRESERVED_BITS;

#ifdef MULTIPLE_TRANSMIT_CHAINS
    if (numTxAntennas == 2)
    {
        mWatts = mWatts / 2; //equivalent to -3dBm for both antennas being on
    }
#endif

    if (mWatts <= 6)    //6 mW corresponds to 8dBm, our minimum power
    {
        *retTemplateIndex = 0;
        return(eHAL_STATUS_SUCCESS);
    }

    //linear search for closest mW match
    for (i = TPC_MEM_GAIN_LUT_DEPTH; i > 0; i--)
    {
        if (mWatts >= lookup_mWatts[i-1] )
        {
            break;  //i-1 is the closest lower match than the power we are looking for
        }
    }

    //decide if lower or upper index is closest match
    if ( (i < TPC_MEM_GAIN_LUT_DEPTH) && ((mWatts - lookup_mWatts[i-1]) > (lookup_mWatts[i] - mWatts)) )
    {
        //use upper bounding index
        *retTemplateIndex = (tPwrTemplateIndex)i;
    }
    else
    {
        //use lower bounding index
        *retTemplateIndex = (tPwrTemplateIndex)(i - 1);
    }


    return(eHAL_STATUS_SUCCESS);
}


eHalStatus halPhyGetTxMilliWatts(tHalHandle hHal, tPwrTemplateIndex pwrTemplateIndex, t_mW *ret_mWatts)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal) hHal;
    t_mW mW;
    tANI_U8 numTxAntennas;
    t2Decimal arrayGain = 0;
    tANI_U8 m;
    eRfSubBand rfSubband;
    eRfChannels curChan = rfGetCurChannel(pMac);

    if (curChan != INVALID_RF_CHANNEL)
    {
        rfSubband = rfGetBand(pMac, curChan);
    }
    else
    {
        return(eHAL_STATUS_FAILURE);
    }


    //account for number of antennas (if 2, then divide mW by 2, which is equivalent to -3dB)
    // and average antenna array gain
    numTxAntennas = halPhyQueryNumTxChains(pMac->hphy.phy.activeChains);

    arrayGain = pMac->hphy.phy.regDomainInfo[pMac->hphy.phy.curRegDomain].antennaGain[rfSubband].reported;

    assert(pwrTemplateIndex < TPC_MEM_GAIN_LUT_DEPTH);

    //look up per chain non-radiated output
    mW = lookup_mWatts[pwrTemplateIndex];

    //calc total output power based on antennas in use
    //calc index into lookup_mW_Multiplier for array gain multiplier
    m = (tANI_U8)GET_MW_MULT_INDEX(arrayGain);
    mW = (mW * lookup_mW_Multiplier[m]) / PRESERVED_BITS;

#ifdef MULTIPLE_TRANSMIT_CHAINS
    if (numTxAntennas == 2)
    {
        mW = mW * 2; //equivalent to +3dBm for both antennas being on
    }
#endif

    *ret_mWatts = mW;

    return(eHAL_STATUS_SUCCESS);
}


/*
    tPowerdBmRange InterpolateAbsPowerPerFreq(tpAniSirGlobal pMac, tANI_U16 freq)
    {
        //tANI_U32 channel;
        tANI_U8 lowChan;
        tANI_U8 highChan;
        tPowerdBmRange retVal = {0, 0};
        tPhyTxPowerBand *band;

        band = &pMac->hphy.phyTPC.combinedBands;
        assert(band != NULL);
        //assert(band->numTpcCalFreqs >= 2);




        //  CHANNEL BONDING NOTE:
        //  Note that if we are in channel-bonded mode, we still will interpolate the frequency among the
        //  non-CB channels, because #1 -that is what is calibrated, and #2 any CB freq falls between non-CB freqs
        //

        //make sure freq is within calibrated freq range, if not correct to range before we interpolate
        //if (FindEncompassingFreqs(freq, &band->pwrSampled[0], offsetof(tTpcConfig, freq), sizeof(tTpcConfig), band->numTpcCalFreqs, &lowChan, &highChan) == eHAL_STATUS_SUCCESS)
        if (FindEncompassingFreqs(freq, &band->pwrSampled[0], offsetof(tTpcConfig, freq), sizeof(tTpcConfig), MAX_TPC_CHANNELS, &lowChan, &highChan) == eHAL_STATUS_SUCCESS)
        {
            if (lowChan != highChan)
            {
                retVal.min = (t2Decimal)InterpolateBetweenPoints((tANI_S32)band->pwrSampled[lowChan].freq,
                                                                 band->pwrSampled[lowChan].absPower.min,
                                                                 (tANI_S32)band->pwrSampled[highChan].freq,
                                                                 band->pwrSampled[highChan].absPower.min,
                                                                 (tANI_S32)freq
                                                                );

                retVal.max = (t2Decimal)InterpolateBetweenPoints((tANI_S32)band->pwrSampled[lowChan].freq,
                                                                 band->pwrSampled[lowChan].absPower.max,
                                                                 (tANI_S32)band->pwrSampled[highChan].freq,
                                                                 band->pwrSampled[highChan].absPower.max,
                                                                 (tANI_S32)freq
                                                                );
            }
            else
            {
                return(band->pwrSampled[lowChan].absPower);
            }
            return(retVal);
        }

        assert(0);
        return(retVal);
    }
*/

tPowerdBm halPhyGetRegDomainLimit(tHalHandle hHal, eHalPhyRates rate)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal) hHal;
    eRfChannels chanIndex = rfGetCurChannel(pMac);

    if (pMac->hphy.phy.regDomainInfo != NULL)
    {
        return (tPowerdBm)(pMac->hphy.phy.regDomainInfo[pMac->hphy.phy.curRegDomain].channels[chanIndex].pwrLimit);
    }
    else
    {
        return (tPowerdBm)OPT_PWRLIMIT_DBM;
    }

    //preserve the following snippet for next gen
#ifdef CHANNEL_BONDED_CAPABLE
    tpAniSirGlobal pMac = (tpAniSirGlobal) hHal;
    tChannelPwrLimit chanLimit = 30;
    eRfChannels chanIndex = rfGetCurChannel(pMac);
    ePhyChanBondState cbState = halPhyGetChannelBondState(pMac);

    assert(chanIndex < NUM_RF_CHANNELS);

    if (pMac->hphy.phy.regDomainInfo == NULL)
    {
        phyLog(pMac, LOGE, "ERROR: Regulatory domain table not present\n");
        return(0);
    }

    if (rate>NUM_HAL_PHY_RATES)
    {
        assert(0);
        return((tPowerdBm)chanLimit);
    }

    if (TEST_PHY_RATE_IS_NON_CB(rate) )
    {
        switch (cbState)
        {
            case PHY_SINGLE_CHANNEL_CENTERED:
                //chanIndex is correct power index
                chanLimit = pMac->hphy.phy.regDomainInfo[pMac->hphy.phy.curRegDomain].channels[chanIndex].pwrLimit;
                break;
            case PHY_DOUBLE_CHANNEL_LOW_PRIMARY:
                {
                    //chanIndex needs to be adjusted to the low primary limit
                    tANI_U8 chId = rfGetChannelIdFromIndex(chanIndex) - 2;
                    chanIndex = rfGetChannelIndex(chId, PHY_SINGLE_CHANNEL_CENTERED);
                    assert(chanIndex != INVALID_RF_CHANNEL);
                    chanLimit = pMac->hphy.phy.regDomainInfo[pMac->hphy.phy.curRegDomain].channels[chanIndex].pwrLimit;
                    break;
                }
            case PHY_DOUBLE_CHANNEL_HIGH_PRIMARY:
                {
                    //chanIndex needs to be adjusted to the high primary limit
                    tANI_U8 chId = rfGetChannelIdFromIndex(chanIndex) + 2;
                    chanIndex = rfGetChannelIndex(chId, PHY_SINGLE_CHANNEL_CENTERED);
                    assert(chanIndex != INVALID_RF_CHANNEL);
                    chanLimit = pMac->hphy.phy.regDomainInfo[pMac->hphy.phy.curRegDomain].channels[chanIndex].pwrLimit;
                    break;
                }
            default:
                assert(0);
                break;
        }
    }
    else
    {

        switch (cbState)
        {
            // for the moment, setup to use the corresponding non-CB limit for the CB centered channel
            case PHY_DOUBLE_CHANNEL_LOW_PRIMARY:
            case PHY_DOUBLE_CHANNEL_HIGH_PRIMARY:
                {
                    //chanIndex needs to be adjusted to the low primary limit
                    assert(chanIndex != INVALID_RF_CHANNEL);
                    chanLimit = pMac->hphy.phy.regDomainInfo[pMac->hphy.phy.curRegDomain].channels[chanIndex].pwrLimit;
                    break;
                }
            default:
                //assert(0); //channel bonded rates can only be evaluated while in channel-bonded mode
                phyLog(pMac, LOG1, "ERROR: halPhyGetRegDomainLimit(CB rate=%d) while in 20MHz mode\n", rate);
                break;
        }
    }
    return((tPowerdBm)chanLimit);
#endif
}

#ifndef WLAN_FTM_STUB
tPowerDetect phyGetTxPowerLutValForAbsPower(tpAniSirGlobal pMac, ePhyTxChains txChain, t2Decimal absPwr)
{
    //return power template index corresponding to the forced dBm value
    //first we've got to get the interpolated absolute power range for the current frequency
    //second, interpolate across the LUT values vs. interpolated absolute power range
    //tANI_U16 freq;
    tPowerdBmRange dBmRange;
    tPowerDetect absLut;
    tANI_S32 x1;
    tANI_S32 y1;
    tANI_S32 x2;
    tANI_S32 y2;
    tANI_S32 x;

    //freq = rfChIdToFreqCoversion(rfGetChannelIdFromIndex(rfGetCurChannel(pMac)));
    dBmRange.min = MIN_PWR_LUT_DBM_2DEC_PLACES;
    dBmRange.max = MAX_PWR_LUT_DBM_2DEC_PLACES;//InterpolateAbsPowerPerFreq(pMac, freq);

    x1 = dBmRange.min;
    y1 = 0; //pMac->hphy.phyTPC.curTpcPwrLUT[txChain][0];  //minimum value in LUT
    x2 = dBmRange.max;
    y2 = TPC_MEM_POWER_LUT_DEPTH - 1; //pMac->hphy.phyTPC.curTpcPwrLUT[txChain][TPC_MEM_POWER_LUT_DEPTH - 1];  //maximum value in LUT
    x = absPwr;

    //guard against interpolating for something out of cald range
    if (x < x1)
    {
        x = x1;
    }
    else if (x > x2)
    {
        x = x2;
    }

    absLut = (tPowerDetect)InterpolateBetweenPoints(x1, y1, x2, y2, x);

    phyLog(pMac, LOG1, "phyGetTxPowerLutValForAbsPower(): min = %d, max = %d, lut min = %d, lut max= %d, arg = %d, absLut = %d\n",
           x1, x2, y1, y2, x, absLut);

    return(absLut);
}

t2Decimal phyGetAbsTxPowerForLutValue(tpAniSirGlobal pMac, ePhyTxChains txChain, tPowerDetect lutValue)
{
    tPowerdBmRange dBmRange;
    t2Decimal absPwr;
    tANI_S32 x1;
    tANI_S32 y1;
    tANI_S32 x2;
    tANI_S32 y2;
    tANI_S32 x;

    //freq = rfChIdToFreqCoversion(rfGetChannelIdFromIndex(rfGetCurChannel(pMac)));
    dBmRange.min = MIN_PWR_LUT_DBM_2DEC_PLACES;
    dBmRange.max = MAX_PWR_LUT_DBM_2DEC_PLACES;//InterpolateAbsPowerPerFreq(pMac, freq);

    y1 = dBmRange.min;
    x1 = 0;
    y2 = dBmRange.max;
    x2 = TPC_MEM_POWER_LUT_DEPTH - 1;
    x = lutValue;

    absPwr = (t2Decimal)InterpolateBetweenPoints(x1, y1, x2, y2, x);

    phyLog(pMac, LOG1, "phyGetAbsTxPowerForLutValue(): min = %d, max = %d, lut min = %d, lut max= %d, arg = %d, absPwr = %d\n",
           y1, y2, x1, x2, x, absPwr);

    return(absPwr);
}
#endif

eHalStatus halPhySetRegDomain(tHalHandle hHal, eRegDomainId regDomain)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal) hHal;

    if ( (tANI_U32)regDomain < NUM_REG_DOMAINS)
    {
        pMac->hphy.phy.curRegDomain = regDomain;

        return(eHAL_STATUS_SUCCESS);
    }
    else
    {
        return(eHAL_STATUS_FAILURE);
    }
}

eRegDomainId halPhyGetRegDomain(tHalHandle hHal)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal) hHal;

    return(pMac->hphy.phy.curRegDomain);
}

/* This interface returns power in dBm for the given rate based on rate to power table contents. Please
 * note that this will return pwrIndBm * 100. So, the caller should do divide by 100 on the value obtained */
eHalStatus halPhyGetPwrFromRate2PwrTable(tHalHandle hHal, eHalPhyRates rate, t2Decimal *pwr2dec)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal) hHal;
    eRfSubBand  rfSubBand;
    eRfChannels curChan = rfGetCurChannel(pMac);

    if (NULL == pwr2dec)
    {
        phyLog(pMac, LOGE, FL("Null pointer issued to store pwr for rate"));
        VOS_ASSERT(0);
        return eHAL_STATUS_FAILURE;
    }

    switch (curChan)
    {
        //2.4GHz Band
        case RF_CHAN_1:
        case RF_CHAN_2:
        case RF_CHAN_3:
        case RF_CHAN_4:
        case RF_CHAN_5:
        case RF_CHAN_6:
        case RF_CHAN_7:
        case RF_CHAN_8:
        case RF_CHAN_9:
        case RF_CHAN_10:
        case RF_CHAN_11:
        case RF_CHAN_12:
        case RF_CHAN_13:
        case RF_CHAN_14:
            rfSubBand = RF_SUBBAND_2_4_GHZ;
            break;

        default:
            phyLog(pMac, LOGE, FL("Current Channel %d is not valid\n"), curChan);
            VOS_ASSERT(0);
            return(eHAL_STATUS_FAILURE);
    }

    *pwr2dec = pMac->hphy.phy.pwrOptimal[rfSubBand][rate].reported;
    phyLog(pMac, LOG1, FL("Power for rate %d is %d\n"), rate, *pwr2dec);
    return eHAL_STATUS_SUCCESS;
}

/* Return the maxPwrIndex that can be used for a given absolute power limit in dBm */
inline eHalStatus halPhyGetMaxTxPowerIndex(tHalHandle hHal, tPowerdBm absPwrLimit, tPwrTemplateIndex *retTemplateIndex)
{
    VOS_ASSERT(NULL != retTemplateIndex);

    /* Get the max power index by getting the power index for 1Mbps 11b rate as this rate can
     * have the maxpower compared to other rates. In case if systems team changes this in future to use max TX
     * power for some other rate(s), only this function should be modified and the interface should remain the same.
     */

    if (halPhyGetPowerForRate(hHal, HAL_PHY_RATE_11B_LONG_1_MBPS, POWER_MODE_HIGH_POWER,
                            absPwrLimit, retTemplateIndex) != eHAL_STATUS_SUCCESS)
    {
        phyLog(hHal, LOGP, FL("Failed\n"));
        return eHAL_STATUS_FAILURE;
    }

    phyLog(hHal, LOG1, FL("MAX Pwr Idx to be used for abs power of %d is %d\n"), absPwrLimit, *retTemplateIndex);
    return eHAL_STATUS_SUCCESS;
}

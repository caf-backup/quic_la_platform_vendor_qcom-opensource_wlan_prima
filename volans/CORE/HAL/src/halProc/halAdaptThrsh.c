/**
 * Copyright (c) 2011 Qualcomm Atheros, Inc. 
 * All Rights Reserved. 
 * Qualcomm Atheros Confidential and Proprietary. 
 * 
 * Copyright (C) 2006 Airgo Networks, Incorporated
 * 
 * This file halThrshProcess.c contains the functions used to
 * implement the adaptive threshold adjustment algorithm
 *
 *
 * Author:      Viji Alagarsamy
 * Date:        08/16/2005
 * History:-
 * Date            Modified by    Modification Information
 * 2/27/06         Satish G       Moved to Taurus HAL (functions stub'ed out)
 * --------------------------------------------------------------------
 */

//#if (WNI_POLARIS_FW_PRODUCT == STA)
#include "palTypes.h"
#include "sirCommon.h"
#include "aniGlobal.h"
#include "halAdaptThrsh.h"
#include "halDebug.h"
#include "halUtils.h"
#include "halMTU.h"
#include "cfgApi.h"

//Sample every 125ms and adjust every 3 samples
#define  HAL_ADAPT_THRESH_SAMPLE_INTERVAL_IN_MICROSEC      250000
#define  HAL_ADAPT_THRESH_ADJUSTMENT_INTERVAL              3

// Default thresholds above which the AGC 
// decides a packet is received
#define  HAL_ADAPT_THRESH_CD_DEFAULT           0      //sugggested value 0x30 does not work
#define  HAL_ADAPT_THRESH_D0_11A_DEFAULT       0x64   //100
#define  HAL_ADAPT_THRESH_D0_11B_DEFAULT       0x28   //40
#define  HAL_ADAPT_THRESH_D0_11N_DEFAULT       0x46   //70
#define  HAL_ADAPT_THRESH_D0_DBL_DEFAULT       0x6e   //110

/* ----------------------------
 * Carrier Detection Threshold
 * ---------------------------- */
// 12/22/05 - Changed the min value to 48 (0x30) per James G, bug 12716
// 09/03/05 - Changed the max value to 80 (0x50) as suggested by James G.
#define  HAL_ADAPT_THRESH_CD_MIN_THRESHOLD   0
#define  HAL_ADAPT_THRESH_CD_MAX_THRESHOLD   0x50
#define  HAL_ADAPT_THRESH_CD_STEP_THRESHOLD  0x1

/* --------------------------------
 * Correlation Detection Threshold
 * -------------------------------- */
// 11a packets (th_d0_11a)
#define  HAL_ADAPT_THRESH_D0_11A_MIN_THRESHOLD   0x64   
#define  HAL_ADAPT_THRESH_D0_11A_MAX_THRESHOLD   0xC8   
#define  HAL_ADAPT_THRESH_D0_11A_STEP_THRESHOLD  0x5

// 11b packets (th_d0_11b)
#define  HAL_ADAPT_THRESH_D0_11B_MIN_THRESHOLD   0x28
#define  HAL_ADAPT_THRESH_D0_11B_MAX_THRESHOLD   0x96
#define  HAL_ADAPT_THRESH_D0_11B_STEP_THRESHOLD  0x5

// 11n packets (th_d0_11n)
#define  HAL_ADAPT_THRESH_D0_11N_MIN_THRESHOLD   0x46
#define  HAL_ADAPT_THRESH_D0_11N_MAX_THRESHOLD   0xc8
#define  HAL_ADAPT_THRESH_D0_11N_STEP_THRESHOLD  0x5

// Channel bonded (double bandwidth) 11a and MIMO packets (th_do_dbl)
#define  HAL_ADAPT_THRESH_D0_DBL_MIN_THRESHOLD   0x6e  
#define  HAL_ADAPT_THRESH_D0_DBL_MAX_THRESHOLD   0xc8
#define  HAL_ADAPT_THRESH_D0_DBL_STEP_THRESHOLD  0x5
 
// set FDR min/max thresholds
#define  HAL_ADAPT_THRESH_HIGH_FALSE_DETECT      256
#define  HAL_ADAPT_THRESH_LOW_FALSE_DETECT       32

// --------------------------------------------------------------------
static inline void
halATH_storeAGCReg(tpAniSirGlobal pMac)
{
#ifdef FIXME_GEN5_PHY
    (void) halReadRegister(pMac, AGC_TH_CD_REG, &(pMac->hal.halAdaptThresh.currentTH_CD));
    (void) halReadRegister(pMac, AGC_TH_D0_A_REG, &(pMac->hal.halAdaptThresh.currentTH_D0_A));
    (void) halReadRegister(pMac, AGC_TH_D0_B_REG, &(pMac->hal.halAdaptThresh.currentTH_D0_B));
    (void) halReadRegister(pMac, AGC_TH_D0_11N_REG, &(pMac->hal.halAdaptThresh.currentTH_D0_N));
    (void) halReadRegister(pMac, AGC_TH_D0_DBL_REG, &(pMac->hal.halAdaptThresh.currentTH_D0_DBL));
#endif
}    

static inline void
halATH_restoreAGCReg(tpAniSirGlobal pMac)
{
#ifdef FIXME_GEN5_PHY
    halWriteRegister(pMac, AGC_TH_CD_REG,     pMac->hal.halAdaptThresh.currentTH_CD);
    halWriteRegister(pMac, AGC_TH_D0_A_REG,   pMac->hal.halAdaptThresh.currentTH_D0_A);
    halWriteRegister(pMac, AGC_TH_D0_B_REG,   pMac->hal.halAdaptThresh.currentTH_D0_B);
    halWriteRegister(pMac, AGC_TH_D0_11N_REG, pMac->hal.halAdaptThresh.currentTH_D0_N);
    halWriteRegister(pMac, AGC_TH_D0_DBL_REG, pMac->hal.halAdaptThresh.currentTH_D0_DBL);  
#endif
}


/** ----------------------------------------------------------------------
\fn      halATH_setAlgorithm
\brief   This function enables/disables the adaptive threshold, 
\        and sets the algorithm type based on the WNI_CFG_NETWORK_DENSITY 
\        and WNI_CFG_ADAPTIVE_THRESHOLD_ALGORITHM settings.
\param   tpAniSirGlobal pMac
\return  none
  ------------------------------------------------------------------------*/
void halATH_setAlgorithm(tpAniSirGlobal pMac)
{
    tANI_U32  nwDensity;
    tANI_U32  algorithm;

    if (wlan_cfgGetInt(pMac, WNI_CFG_NETWORK_DENSITY, (tANI_U32 *) &nwDensity) != eSIR_SUCCESS)
    {
        HALLOGP( halLog(pMac, LOGP, FL("cfgGet(WNI_CFG_NETWORK_DENSITY) failed \n")));
        return;
    }

    if (nwDensity == WNI_CFG_NETWORK_DENSITY_ADAPTIVE)
    {
        if (wlan_cfgGetInt(pMac, WNI_CFG_ADAPTIVE_THRESHOLD_ALGORITHM, (tANI_U32 *) &algorithm) != eSIR_SUCCESS)
        {
            HALLOGP( halLog(pMac, LOGP, FL("cfgGet(WNI_CFG_ADAPTIVE_THRESHOLD_ALGORITHM) failed \n")));
            return;
        }

        HALLOGW( halLog(pMac, LOGW, FL("Network Density is AUTO, so enable ADAPTIVE THRESHOLD ALGORITHM \n")));
        halATH_enableAlgorithm(pMac, TRUE);

        if (algorithm == WNI_CFG_ADAPTIVE_THRESHOLD_ALGORITHM_CORRELATION)
        {
            HALLOGW( halLog(pMac, LOGW, FL("CORRELATION DETECTION ALGORITHM selected \n")));
            halATH_selectAlgorithm(pMac, ANI_SCH_ADAPTIVE_THRESHOLD_TH_D0);
        }
        else
        {
            HALLOGW( halLog(pMac, LOGW, FL("CARRIER DETECTION ALGORITHM selected \n")));
            halATH_selectAlgorithm(pMac, ANI_SCH_ADAPTIVE_THRESHOLD_TH_CD);
        }
    } 
    else
    {
        HALLOGW( halLog(pMac, LOGW, FL("NETWORK_DENSITY is not AUTO, so disable ADAPTIVE THRESHOLD ALGORITHM \n")));
        halATH_enableAlgorithm(pMac, FALSE);
    } 

    return;
}


/*
 * FUNCTION:  halATH_initAGCCounts()
 * 
 * NOTE: 
 *   Enable transition counter 5. This counter will be triggered
 *   on transition between AGC state 9 to AGC state 1.
 * 
 *   Enable transition counter 6. This counter will be triggered
 *   on transition between AGC state 8 to AGC state 1.
 *
 *   Enable transition counter 7. This counter will be triggered
 *   on transition between AGC state 7 to AGC state 1.
 *
 */     
static void
halATH_initAGCCounts(tpAniSirGlobal pMac)
{
#ifdef FIXME_GEN5_PHY
    halWriteRegister(pMac, AGC_TRANSITION_COUNT5_EN_REG, 0x119); // 90f0
    halWriteRegister(pMac, AGC_TRANSITION_COUNT6_EN_REG, 0x118); // 90f4
    halWriteRegister(pMac, AGC_TRANSITION_COUNT7_EN_REG, 0x117); // 90f8

    (void) halReadRegister(pMac, AGC_TRANSITION_COUNT5_REG, &(pMac->hal.halAdaptThresh.agc9To1Transitions));
    (void) halReadRegister(pMac, AGC_TRANSITION_COUNT6_REG, &(pMac->hal.halAdaptThresh.agc8To1Transitions));
    (void) halReadRegister(pMac, AGC_TRANSITION_COUNT7_REG, &(pMac->hal.halAdaptThresh.agc7To1Transitions));
    (void) halReadRegister(pMac, RACTL_SIGNAL_INVALID_COUNT_REG, &(pMac->hal.halAdaptThresh.signalInvalidCount));
#endif   
    return;
}

void
halATH_setDefaultThresholdValues(tpAniSirGlobal pMac)
{
#ifdef FIXME_GEN5_PHY
    halWriteRegister(pMac, AGC_TH_CD_REG,   HAL_ADAPT_THRESH_CD_DEFAULT);
    halWriteRegister(pMac, AGC_TH_D0_A_REG, HAL_ADAPT_THRESH_D0_11A_DEFAULT);
    halWriteRegister(pMac, AGC_TH_D0_B_REG, HAL_ADAPT_THRESH_D0_11B_DEFAULT);
    halWriteRegister(pMac, AGC_TH_D0_DBL_REG, HAL_ADAPT_THRESH_D0_DBL_DEFAULT);
    halWriteRegister(pMac, AGC_TH_D0_11N_REG, HAL_ADAPT_THRESH_D0_11N_DEFAULT);
#endif
    
    return;
}

void
halATH_handleScanStart(tpAniSirGlobal pMac)
{
    // store current settings of thresholds
    if( pMac->hal.halAdaptThresh.fThresholdAdaptionEnabled)
    {
        pMac->hal.halAdaptThresh.fScanInProgress = true;
        halATH_storeAGCReg(pMac);

        // !!LAC - we are seeing that the thresh is causing us to not see a lot of scan
        // results on some machines.   Try setting the default thresholds during scan.
        // This does correct the erratic behavior of scan result reception that we've
        // been seeing.
        halATH_setDefaultThresholdValues( pMac );
    }
    return;
}

void
halATH_handleScanStop(tpAniSirGlobal pMac)
{
    // re-store old settings of thresholds
    if( pMac->hal.halAdaptThresh.fThresholdAdaptionEnabled && pMac->hal.halAdaptThresh.fScanInProgress )
    {
        halATH_restoreAGCReg(pMac);
        pMac->hal.halAdaptThresh.fScanInProgress = false;
    }
    return;
}


void
halATH_handlePSStart(tpAniSirGlobal pMac)
{
    // store current settings of thresholds
    if( pMac->hal.halAdaptThresh.fThresholdAdaptionEnabled)
    {
        halATH_storeAGCReg(pMac);
        // !!LAC - we are seeing that the thresh is causing us to not see a lot of scan
        // results on some machines.   Try setting the default thresholds during scan.
        // This does correct the erratic behavior of scan result reception that we've
        // been seeing.
        halATH_setDefaultThresholdValues( pMac );
    }
    return;
}

void
halATH_handlePSStop(tpAniSirGlobal pMac)
{
    // re-store old settings of thresholds
    if( pMac->hal.halAdaptThresh.fThresholdAdaptionEnabled && pMac->hal.halAdaptThresh.fScanInProgress )
        halATH_restoreAGCReg(pMac);
    return;
}

void
halATH_enableAlgorithm(tpAniSirGlobal pMac, tANI_U8 fEnable)
{
    pMac->hal.halAdaptThresh.fThresholdAdaptionEnabled = (fEnable == 0) ? false : true;
    return;
}

void
halATH_selectAlgorithm(tpAniSirGlobal pMac, tANI_U32 Algo)
{
    pMac->hal.halAdaptThresh.algoSelection = Algo;
    return;
}

void
halATH_selectBand(tpAniSirGlobal pMac, tANI_U32 Band)
{
    pMac->hal.halAdaptThresh.bandSelection = Band;
    return;
}

void
halATH_selectAdjustmentIntervals(tpAniSirGlobal pMac, tANI_U32 AdjustmentInterval)
{
    pMac->hal.halAdaptThresh.adjustmentInterval = AdjustmentInterval;
    pMac->hal.halAdaptThresh.adjustmentIntervalCnt = pMac->hal.halAdaptThresh.adjustmentInterval;
    return;
}

void
halATH_selectSampleIntervals(tpAniSirGlobal pMac, tANI_U32 SampleInterval)
{
    // sample interval in terms of us
    pMac->hal.halAdaptThresh.sampleInterval = SampleInterval;
    return;
}

void halATH_setLinkState(tpAniSirGlobal pMac, tSirLinkState linkState)
{
    pMac->hal.halAdaptThresh.currentLinkState = linkState;
        
    if( pMac->hal.halAdaptThresh.fThresholdAdaptionEnabled)
    {
        if (linkState == eSIR_LINK_PREASSOC_STATE)
        {
            halATH_setDefaultThresholdValues(pMac);
        }
    }
}

void
halATH_setThCd(tpAniSirGlobal pMac, tANI_U32 min, tANI_U32 max, tANI_U32 step )
{
    pMac->hal.halAdaptThresh.minTH_CD = min;
    pMac->hal.halAdaptThresh.maxTH_CD = max;
    pMac->hal.halAdaptThresh.stepTH_CD = step;
    return;
}

void
halATH_setThD011a(tpAniSirGlobal pMac, tANI_U32 min, tANI_U32 max, tANI_U32 step )
{
    pMac->hal.halAdaptThresh.minTH_D0_A = min;
    pMac->hal.halAdaptThresh.maxTH_D0_A = max;
    pMac->hal.halAdaptThresh.stepTH_D0_A = step;
    return;
}

void
halATH_setThD011b(tpAniSirGlobal pMac, tANI_U32 min, tANI_U32 max, tANI_U32 step )
{
    pMac->hal.halAdaptThresh.minTH_D0_B = min;
    pMac->hal.halAdaptThresh.maxTH_D0_B = max;
    pMac->hal.halAdaptThresh.stepTH_D0_B = step;
    return;
}

void
halATH_setThD011n(tpAniSirGlobal pMac, tANI_U32 min, tANI_U32 max, tANI_U32 step )
{
    pMac->hal.halAdaptThresh.minTH_D0_N = min;
    pMac->hal.halAdaptThresh.maxTH_D0_N = max;
    pMac->hal.halAdaptThresh.stepTH_D0_N = step;
    return;
}

void
halATH_setThD011dbl(tpAniSirGlobal pMac, tANI_U32 min, tANI_U32 max, tANI_U32 step )
{
    pMac->hal.halAdaptThresh.minTH_D0_DBL = min;
    pMac->hal.halAdaptThresh.maxTH_D0_DBL = max;
    pMac->hal.halAdaptThresh.stepTH_D0_DBL = step;
    return;
}


void
halATH_setFDRThresholds(tpAniSirGlobal pMac, tANI_U32 high, tANI_U32 low)
{
    pMac->hal.halAdaptThresh.highFalseDetect = high;
    pMac->hal.halAdaptThresh.lowFalseDetect = low;
    return;
}

void
halATH_resetStats(tpAniSirGlobal pMac)
{
    pMac->hal.halAdaptThresh.numTH_CDAdjustedUp = 0;
    pMac->hal.halAdaptThresh.numTH_CDAdjustedDown = 0;
    pMac->hal.halAdaptThresh.numTH_D0_A_AdjustedUp = 0;
    pMac->hal.halAdaptThresh.numTH_D0_A_AdjustedDown = 0;
    pMac->hal.halAdaptThresh.numTH_D0_B_AdjustedUp = 0;
    pMac->hal.halAdaptThresh.numTH_D0_B_AdjustedDown = 0;
    pMac->hal.halAdaptThresh.numTH_D0_N_AdjustedUp = 0;
    pMac->hal.halAdaptThresh.numTH_D0_N_AdjustedUp = 0;
    pMac->hal.halAdaptThresh.numTH_D0_DBL_AdjustedDown = 0;
    pMac->hal.halAdaptThresh.numTH_D0_DBL_AdjustedDown = 0;
    pMac->hal.halAdaptThresh.numFDRRangeViolated = 0;
  
    return;
}


/** -------------------------------------------------------------
\fn      halATH_initialize
\brief   This function initializes the global settings for
\        adaptive threshold
\param   tpAniSirGlobal pMac
\return  none
  -------------------------------------------------------------*/
void halATH_initialize(tpAniSirGlobal pMac)
{
    halATH_resetStats(pMac);
    halATH_initAGCCounts(pMac);
    halATH_setFDRThresholds( pMac, HAL_ADAPT_THRESH_HIGH_FALSE_DETECT, HAL_ADAPT_THRESH_LOW_FALSE_DETECT );

    pMac->hal.halAdaptThresh.fScanInProgress = false;
    pMac->hal.halAdaptThresh.fEnableSamplesLogging = false;
 
    //TODO: sample every 125ms and adjust every 3 samples
    halATH_selectSampleIntervals(pMac, HAL_ADAPT_THRESH_SAMPLE_INTERVAL_IN_MICROSEC);
    halATH_selectAdjustmentIntervals(pMac, HAL_ADAPT_THRESH_ADJUSTMENT_INTERVAL);

    //TODO: enable the algorithm for 2.4GHz band only ??
    halATH_selectBand(pMac, ANI_SCH_ADAPTIVE_ALGO_BAND_ALL);

 
    halATH_setThCd( pMac, 
                              HAL_ADAPT_THRESH_CD_MIN_THRESHOLD, 
                              HAL_ADAPT_THRESH_CD_MAX_THRESHOLD, 
                              HAL_ADAPT_THRESH_CD_STEP_THRESHOLD );

    halATH_setThD011a( pMac, 
                                  HAL_ADAPT_THRESH_D0_11A_MIN_THRESHOLD, 
                                  HAL_ADAPT_THRESH_D0_11A_MAX_THRESHOLD, 
                                  HAL_ADAPT_THRESH_D0_11A_STEP_THRESHOLD );

    halATH_setThD011b( pMac, 
                                  HAL_ADAPT_THRESH_D0_11B_MIN_THRESHOLD, 
                                  HAL_ADAPT_THRESH_D0_11B_MAX_THRESHOLD, 
                                  HAL_ADAPT_THRESH_D0_11B_STEP_THRESHOLD );

    halATH_setThD011n( pMac, 
                                  HAL_ADAPT_THRESH_D0_11N_MIN_THRESHOLD,
                                  HAL_ADAPT_THRESH_D0_11N_MAX_THRESHOLD,
                                  HAL_ADAPT_THRESH_D0_11N_STEP_THRESHOLD );

    halATH_setThD011dbl( pMac, 
                                    HAL_ADAPT_THRESH_D0_DBL_MIN_THRESHOLD,
                                    HAL_ADAPT_THRESH_D0_DBL_MAX_THRESHOLD,
                                    HAL_ADAPT_THRESH_D0_DBL_STEP_THRESHOLD );
#ifdef FIXME_GEN5_PHY

    halWriteRegister(pMac, AGC_TH_D0_A_REG, HAL_ADAPT_THRESH_D0_11A_DEFAULT);
    halWriteRegister(pMac, AGC_TH_D0_B_REG, HAL_ADAPT_THRESH_D0_11B_DEFAULT);
    halWriteRegister(pMac, AGC_TH_D0_DBL_REG, HAL_ADAPT_THRESH_D0_DBL_DEFAULT);
    halWriteRegister(pMac, AGC_TH_D0_11N_REG, HAL_ADAPT_THRESH_D0_11N_DEFAULT);
    
    (void) halReadRegister(pMac, AGC_TH_CD_REG,     &(pMac->hal.halAdaptThresh.currentTH_CD));
    (void) halReadRegister(pMac, AGC_TH_D0_A_REG,   &(pMac->hal.halAdaptThresh.currentTH_D0_A));
    (void) halReadRegister(pMac, AGC_TH_D0_B_REG,   &(pMac->hal.halAdaptThresh.currentTH_D0_B));
    (void) halReadRegister(pMac, AGC_TH_D0_11N_REG, &(pMac->hal.halAdaptThresh.currentTH_D0_N));
    (void) halReadRegister(pMac, AGC_TH_D0_DBL_REG, &(pMac->hal.halAdaptThresh.currentTH_D0_DBL));

    if (pMac->hal.halAdaptThresh.fEnableSamplesLogging == true)
    {
        halLog(pMac, LOGW, "halATH_initialize() ");
        HALLOGW( halLog( pMac, LOGW, FL("AGC_TH_CD_REG =   0x%x (%d)   "),  pMac->hal.halAdaptThresh.currentTH_CD,   pMac->hal.halAdaptThresh.currentTH_CD ));
        HALLOGW( halLog( pMac, LOGW, FL("AGC_TH_D0_A_REG = 0x%x (%d)   "),  pMac->hal.halAdaptThresh.currentTH_D0_A, pMac->hal.halAdaptThresh.currentTH_D0_A ));
        HALLOGW( halLog( pMac, LOGW, FL("AGC_TH_D0_B_REG = 0x%x (%d)   "),  pMac->hal.halAdaptThresh.currentTH_D0_B, pMac->hal.halAdaptThresh.currentTH_D0_B ));
        HALLOGW( halLog( pMac, LOGW, FL("AGC_TH_D0_11N_REG = 0x%x (%d) "),  pMac->hal.halAdaptThresh.currentTH_D0_N, pMac->hal.halAdaptThresh.currentTH_D0_N ));
        HALLOGW( halLog( pMac, LOGW, FL("AGC_TH_D0_DBL_REG = 0x%x (%d) "),  pMac->hal.halAdaptThresh.currentTH_D0_DBL, pMac->hal.halAdaptThresh.currentTH_D0_DBL ));
    }
#endif

    return;
}


tANI_U8 halATH_isBandSelected(tpAniSirGlobal pMac)
{
    tANI_U8   fBandSelected = true;
#ifdef FIXME_GEN5_PHY    
    tANI_U32  readValue;

    do
    {
        // check if both bands are enabled
        if( ANI_SCH_ADAPTIVE_ALGO_BAND_ALL == (pMac->hal.halAdaptThresh.bandSelection & ANI_SCH_ADAPTIVE_ALGO_BAND_ALL) ) break;

        (void) halReadRegister(pMac, AGC_DIS_MODE_REG, &readValue); 
        if( readValue & AGC_DIS_MODE_DISABLE_11B_MASK )
        {
            // we must be in 5GHz band, check if 5GHz band is enabled
            if( pMac->hal.halAdaptThresh.bandSelection & ANI_SCH_ADAPTIVE_ALGO_BAND_5GHZ ) break;
        }
        else
        {
            // we must be in 2.4GHz band, check if 2.4GHz band is enabled
            if( pMac->hal.halAdaptThresh.bandSelection & ANI_SCH_ADAPTIVE_ALGO_BAND_2GHZ ) break;
        }

        fBandSelected = false;
    }
    while( 0 );
#endif
    return fBandSelected;
}


tANI_U8 halATH_isAdjustmentRequired( tpAniSirGlobal pMac, tANI_U32 aModeFDR, tANI_U32 bModeFDR, tANI_U32 highFalseDetectThreshold, tANI_U32 lowFalseDetectThreshold )
{
    tANI_U8   fAdjustmentRequired = false;
#ifdef FIXME_GEN5_PHY
    do
    {
        // Carrier Detection
        if( pMac->hal.halAdaptThresh.algoSelection & ANI_SCH_ADAPTIVE_THRESHOLD_TH_CD )
        {  
            if( ((aModeFDR + bModeFDR) > highFalseDetectThreshold) || ((aModeFDR + bModeFDR) < lowFalseDetectThreshold)  )
            {
                fAdjustmentRequired = true;
                break;
            }
        }
        // Correlation Detection
        if( pMac->hal.halAdaptThresh.algoSelection & ANI_SCH_ADAPTIVE_THRESHOLD_TH_D0 )
        {
           if ( 
               // Too many false detect, probably because current threshold is set too low. Adjust up if possible. 
               ((aModeFDR > highFalseDetectThreshold) && (pMac->hal.halAdaptThresh.currentTH_D0_A   < pMac->hal.halAdaptThresh.maxTH_D0_A))   ||
               ((aModeFDR > highFalseDetectThreshold) && (pMac->hal.halAdaptThresh.currentTH_D0_N   < HAL_ADAPT_THRESH_D0_11N_MAX_THRESHOLD)) ||
               ((aModeFDR > highFalseDetectThreshold) && (pMac->hal.halAdaptThresh.currentTH_D0_DBL < HAL_ADAPT_THRESH_D0_DBL_MAX_THRESHOLD)) ||  
               ((bModeFDR > highFalseDetectThreshold) && (pMac->hal.halAdaptThresh.currentTH_D0_B   < pMac->hal.halAdaptThresh.maxTH_D0_B))   || 
               // Too few false detect, probably because current threshold is set too high. Adjust down if possible. 
               ((aModeFDR < lowFalseDetectThreshold) && (pMac->hal.halAdaptThresh.currentTH_D0_A   > pMac->hal.halAdaptThresh.minTH_D0_A))    ||
               ((aModeFDR < lowFalseDetectThreshold) && (pMac->hal.halAdaptThresh.currentTH_D0_N   > pMac->hal.halAdaptThresh.minTH_D0_N))    ||
               ((aModeFDR < lowFalseDetectThreshold) && (pMac->hal.halAdaptThresh.currentTH_D0_DBL > pMac->hal.halAdaptThresh.minTH_D0_DBL))  ||
               ((bModeFDR < lowFalseDetectThreshold) && (pMac->hal.halAdaptThresh.currentTH_D0_B   > pMac->hal.halAdaptThresh.minTH_D0_B)) )
           {

                 if( pMac->hal.halAdaptThresh.fEnableSamplesLogging )
                 {
                     HALLOGW( halLog(pMac, LOGW, FL("\t !!!!!!!!!!! Adjustment Needed !!!!!!!!! \n"))));

                 }    
                fAdjustmentRequired = true;
                break;
            }   
        }
    }
    while( 0 );
#endif    
    return fAdjustmentRequired;
}


void
halATH_adjustThCD( tpAniSirGlobal pMac, tANI_U32 aModeFDR, tANI_U32 bModeFDR, tANI_U32 highFalseDetectThreshold, tANI_U32 lowFalseDetectThreshold )
{
#ifdef FIXME_GEN5_PHY
    if(  ((aModeFDR + bModeFDR) > highFalseDetectThreshold) || ((aModeFDR + bModeFDR) < lowFalseDetectThreshold) )
    {
        // adjust the thresholds
        halReadRegister(pMac, AGC_TH_CD_REG, &(pMac->hal.halAdaptThresh.currentTH_CD));

        if( pMac->hal.halAdaptThresh.fEnableSamplesLogging )
            HALLOGW( halLog(pMac, LOGW, FL("Current TH_CD: %x \n"), pMac->hal.halAdaptThresh.currentTH_CD));

        if( ((aModeFDR + bModeFDR) > highFalseDetectThreshold) &&
            (pMac->hal.halAdaptThresh.currentTH_CD < pMac->hal.halAdaptThresh.maxTH_CD) )
        {
            (pMac->hal.halAdaptThresh.numTH_CDAdjustedUp)++;

            if( pMac->hal.halAdaptThresh.fEnableSamplesLogging )
                HALLOGW( halLog(pMac, LOGW, FL("TH_CD: Adjusting UP\n")));

            pMac->hal.halAdaptThresh.currentTH_CD += pMac->hal.halAdaptThresh.stepTH_CD;

            if( pMac->hal.halAdaptThresh.currentTH_CD > pMac->hal.halAdaptThresh.maxTH_CD )
                pMac->hal.halAdaptThresh.currentTH_CD = pMac->hal.halAdaptThresh.maxTH_CD;
        }
        else if( ((aModeFDR + bModeFDR) < lowFalseDetectThreshold) &&
                    (pMac->hal.halAdaptThresh.currentTH_CD > pMac->hal.halAdaptThresh.minTH_CD) )
        {
            (pMac->hal.halAdaptThresh.numTH_CDAdjustedDown)++;

            if( pMac->hal.halAdaptThresh.fEnableSamplesLogging )
                HALLOGW( halLog(pMac, LOGW, FL("TH_CD: Adjusting DOWN\n")));

            pMac->hal.halAdaptThresh.currentTH_CD -= pMac->hal.halAdaptThresh.stepTH_CD;

            if( pMac->hal.halAdaptThresh.currentTH_CD < pMac->hal.halAdaptThresh.minTH_CD )
                pMac->hal.halAdaptThresh.currentTH_CD = pMac->hal.halAdaptThresh.minTH_CD;
        }

        if( pMac->hal.halAdaptThresh.fEnableSamplesLogging )
            HALLOGW( halLog(pMac, LOGW, FL("New TH_CD: %x\n"), pMac->hal.halAdaptThresh.currentTH_CD));

        // write the new threshold
        halWriteRegister(pMac, AGC_TH_CD_REG, pMac->hal.halAdaptThresh.currentTH_CD);
    }
#endif    
    return;
}


/** -------------------------------------------------------------
\fn      halATH_adjustThD0
\brief   This function adjusts the AGC correlation threshold 
\        registers. 
\param   tpAniSirGlobal pMac
\param   tANI_U32       aModeFDR
\param   tANI_U32       bModeFDR
\return  none
  -------------------------------------------------------------*/
void
halATH_adjustThD0( tpAniSirGlobal pMac, tANI_U32 aModeFDR, tANI_U32 bModeFDR )
{
#ifdef FIXME_GEN5_PHY
    tANI_U32  readValue;
    
    // adjust the thresholds
    (void) halReadRegister(pMac, AGC_TH_D0_A_REG,   &(pMac->hal.halAdaptThresh.currentTH_D0_A));
    (void) halReadRegister(pMac, AGC_TH_D0_B_REG,   &(pMac->hal.halAdaptThresh.currentTH_D0_B));
    (void) halReadRegister(pMac, AGC_TH_D0_11N_REG, &(pMac->hal.halAdaptThresh.currentTH_D0_N));
    (void) halReadRegister(pMac, AGC_TH_D0_DBL_REG, &(pMac->hal.halAdaptThresh.currentTH_D0_DBL));
    (void) halReadRegister(pMac, AGC_DIS_MODE_REG, &readValue);
    if( !(readValue & AGC_DIS_MODE_DISABLE_11AG_MASK) )
    {
        if ( aModeFDR > pMac->hal.halAdaptThresh.highFalseDetect )
        {
            /* The number of false detect observed exceeds the max allowed.
             * Hence, we should increase the AGC threshold if the threshold
             * has not already reached the maxTH_DO.
             */
            if (pMac->hal.halAdaptThresh.currentTH_D0_A < pMac->hal.halAdaptThresh.maxTH_D0_A) 
            {
                if( pMac->hal.halAdaptThresh.fEnableSamplesLogging )
                    HALLOGW( halLog(pMac, LOGW, FL("TH_D0_A: Adjusting UP\n")));

                (pMac->hal.halAdaptThresh.numTH_D0_A_AdjustedUp)++;
                pMac->hal.halAdaptThresh.currentTH_D0_A += pMac->hal.halAdaptThresh.stepTH_D0_A;

                if( pMac->hal.halAdaptThresh.currentTH_D0_A > pMac->hal.halAdaptThresh.maxTH_D0_A )
                    pMac->hal.halAdaptThresh.currentTH_D0_A = pMac->hal.halAdaptThresh.maxTH_D0_A;
            }

            if (pMac->hal.halAdaptThresh.currentTH_D0_N < pMac->hal.halAdaptThresh.maxTH_D0_N) 
            {
                if( pMac->hal.halAdaptThresh.fEnableSamplesLogging )
                    HALLOGW( halLog(pMac, LOGW, FL("TH_D0_N: Adjusting UP\n")));

                (pMac->hal.halAdaptThresh.numTH_D0_N_AdjustedUp)++;
                pMac->hal.halAdaptThresh.currentTH_D0_N += pMac->hal.halAdaptThresh.stepTH_D0_N;

                if( pMac->hal.halAdaptThresh.currentTH_D0_N > pMac->hal.halAdaptThresh.maxTH_D0_N )
                    pMac->hal.halAdaptThresh.currentTH_D0_N = pMac->hal.halAdaptThresh.maxTH_D0_N;
            }

            if (pMac->hal.halAdaptThresh.currentTH_D0_DBL < pMac->hal.halAdaptThresh.maxTH_D0_DBL) 
            {
                if( pMac->hal.halAdaptThresh.fEnableSamplesLogging )
                    HALLOGW( halLog(pMac, LOGW, FL("TH_D0_DBL: Adjusting UP\n")));

                (pMac->hal.halAdaptThresh.numTH_D0_DBL_AdjustedUp)++;
                pMac->hal.halAdaptThresh.currentTH_D0_DBL += pMac->hal.halAdaptThresh.stepTH_D0_DBL;

                if( pMac->hal.halAdaptThresh.currentTH_D0_DBL > pMac->hal.halAdaptThresh.maxTH_D0_DBL )
                    pMac->hal.halAdaptThresh.currentTH_D0_DBL = pMac->hal.halAdaptThresh.maxTH_D0_DBL;
            }
        }
        else if (aModeFDR < pMac->hal.halAdaptThresh.lowFalseDetect)
        {
            if (pMac->hal.halAdaptThresh.currentTH_D0_A > pMac->hal.halAdaptThresh.minTH_D0_A) 
            {
                if( pMac->hal.halAdaptThresh.fEnableSamplesLogging )
                    HALLOGW( halLog(pMac, LOGW, FL("TH_D0_A: Adjusting DOWN\n")));

                (pMac->hal.halAdaptThresh.numTH_D0_A_AdjustedDown)++;

                pMac->hal.halAdaptThresh.currentTH_D0_A -= pMac->hal.halAdaptThresh.stepTH_D0_A;

                if( pMac->hal.halAdaptThresh.currentTH_D0_A < pMac->hal.halAdaptThresh.minTH_D0_A )
                    pMac->hal.halAdaptThresh.currentTH_D0_A = pMac->hal.halAdaptThresh.minTH_D0_A;
            }

            if (pMac->hal.halAdaptThresh.currentTH_D0_N > pMac->hal.halAdaptThresh.minTH_D0_N) 
            {
                if( pMac->hal.halAdaptThresh.fEnableSamplesLogging )
                    HALLOGW( halLog(pMac, LOGW, FL("TH_D0_N: Adjusting DOWN\n")));

                (pMac->hal.halAdaptThresh.numTH_D0_N_AdjustedDown)++;

                pMac->hal.halAdaptThresh.currentTH_D0_N -= pMac->hal.halAdaptThresh.stepTH_D0_N;

                if( pMac->hal.halAdaptThresh.currentTH_D0_N < pMac->hal.halAdaptThresh.minTH_D0_N )
                    pMac->hal.halAdaptThresh.currentTH_D0_N = pMac->hal.halAdaptThresh.minTH_D0_N;
            }

            if (pMac->hal.halAdaptThresh.currentTH_D0_DBL > pMac->hal.halAdaptThresh.minTH_D0_DBL) 
            {
                if( pMac->hal.halAdaptThresh.fEnableSamplesLogging )
                    HALLOGW( halLog(pMac, LOGW, FL("TH_D0_DBL: Adjusting DOWN\n")));

                (pMac->hal.halAdaptThresh.numTH_D0_DBL_AdjustedDown)++;

                pMac->hal.halAdaptThresh.currentTH_D0_DBL -= pMac->hal.halAdaptThresh.stepTH_D0_DBL;

                if( pMac->hal.halAdaptThresh.currentTH_D0_DBL < pMac->hal.halAdaptThresh.minTH_D0_DBL )
                    pMac->hal.halAdaptThresh.currentTH_D0_DBL = pMac->hal.halAdaptThresh.minTH_D0_DBL;
            }
        }
    }
    
    (void) halReadRegister(pMac, AGC_DIS_MODE_REG, &readValue);
    if( !(readValue & AGC_DIS_MODE_DISABLE_11B_MASK) )
    {
        // 11b decoder is enabled

        if( (bModeFDR > pMac->hal.halAdaptThresh.highFalseDetect) &&
            (pMac->hal.halAdaptThresh.currentTH_D0_B < pMac->hal.halAdaptThresh.maxTH_D0_B) )
        {
            (pMac->hal.halAdaptThresh.numTH_D0_B_AdjustedUp)++;

            if( pMac->hal.halAdaptThresh.fEnableSamplesLogging )
                HALLOGW( halLog(pMac, LOGW, FL("TH_D0_B: Adjusting UP\n")));

            pMac->hal.halAdaptThresh.currentTH_D0_B += pMac->hal.halAdaptThresh.stepTH_D0_B;

            if( pMac->hal.halAdaptThresh.currentTH_D0_B > pMac->hal.halAdaptThresh.maxTH_D0_B )
                pMac->hal.halAdaptThresh.currentTH_D0_B = pMac->hal.halAdaptThresh.maxTH_D0_B;
        }
        else if( (bModeFDR < pMac->hal.halAdaptThresh.lowFalseDetect) &&
                 (pMac->hal.halAdaptThresh.currentTH_D0_B > pMac->hal.halAdaptThresh.minTH_D0_B) )

        {
            (pMac->hal.halAdaptThresh.numTH_D0_B_AdjustedDown)++;

            if( pMac->hal.halAdaptThresh.fEnableSamplesLogging )
                HALLOGW( halLog(pMac, LOGW, FL("TH_D0_B: Adjusting DOWN\n")));

            pMac->hal.halAdaptThresh.currentTH_D0_B -= pMac->hal.halAdaptThresh.stepTH_D0_B;

            if( pMac->hal.halAdaptThresh.currentTH_D0_B < pMac->hal.halAdaptThresh.minTH_D0_B )
                pMac->hal.halAdaptThresh.currentTH_D0_B = pMac->hal.halAdaptThresh.minTH_D0_B;
        }
    }

    if( pMac->hal.halAdaptThresh.fEnableSamplesLogging )
    {
        HALLOGW( halLog(pMac, LOGW, FL("New TH_D0_A:   0x%x (%d)\n"), pMac->hal.halAdaptThresh.currentTH_D0_A, pMac->hal.halAdaptThresh.currentTH_D0_A));
        HALLOGW( halLog(pMac, LOGW, FL("New TH_D0_B:   0x%x (%d)\n"), pMac->hal.halAdaptThresh.currentTH_D0_B, pMac->hal.halAdaptThresh.currentTH_D0_B));
        HALLOGW( halLog(pMac, LOGW, FL("New TH_D0_N:   0x%x (%d)\n"), pMac->hal.halAdaptThresh.currentTH_D0_B, pMac->hal.halAdaptThresh.currentTH_D0_N));
        HALLOGW( halLog(pMac, LOGW, FL("New TH_D0_DBL: 0x%x (%d)\n"), pMac->hal.halAdaptThresh.currentTH_D0_B, pMac->hal.halAdaptThresh.currentTH_D0_DBL));
    }

    // write the new threshold
    halWriteRegister(pMac, AGC_TH_D0_A_REG,   pMac->hal.halAdaptThresh.currentTH_D0_A);
    halWriteRegister(pMac, AGC_TH_D0_B_REG,   pMac->hal.halAdaptThresh.currentTH_D0_B);
    halWriteRegister(pMac, AGC_TH_D0_11N_REG, pMac->hal.halAdaptThresh.currentTH_D0_N);
    halWriteRegister(pMac, AGC_TH_D0_DBL_REG, pMac->hal.halAdaptThresh.currentTH_D0_DBL);   
#endif
    return;
}

void halATH_adjustAdaptiveThreshold(tpAniSirGlobal pMac)
{
#ifdef FIXME_GEN5_PHY
    tANI_U32 currentAgc8To1Transitions, currentAgc7To1Transitions, currentAgc9To1Transitions, currentSignalInvalidCount;
    tANI_U32 diffAgc8To1Transitions, diffAgc7To1Transitions, diffAgc9To1Transitions, diffSignalInvalidCount;
    tANI_U32 wrapAroundOffset;
    tANI_U32 aModeFDR, bModeFDR;

    if (pMac->hal.halAdaptThresh.sampleInterval && pMac->hal.halAdaptThresh.fThresholdAdaptionEnabled
                         && pMac->hal.halAdaptThresh.currentLinkState == eSIR_LINK_POSTASSOC_STATE)
    {
        do
        {
            // if mechanism not enabled, then don't do anything
            if( !(pMac->hal.halAdaptThresh.fThresholdAdaptionEnabled) ) 
                break;
            
            // if we're in power save, then don't adapt
            if( halUtil_CurrentlyInPowerSave(pMac) )  break;

            // if the station is not associated, don't adapt
            if (pMac->hal.halAdaptThresh.currentLinkState != eSIR_LINK_POSTASSOC_STATE)
              break;
            
            // if scan is in progress don't adjust thresholds
            if( pMac->hal.halAdaptThresh.fScanInProgress ) break;

            // check if we need to adapt threshold in this band
            if( !halATH_isBandSelected( pMac ) ) break;

            // if sample interval is not set, then don't adjust
            if ( !(pMac->hal.halAdaptThresh.sampleInterval) )  break;

            // get current transition count
            (void) halReadRegister(pMac, AGC_TRANSITION_COUNT5_REG, &currentAgc9To1Transitions); 
            (void) halReadRegister(pMac, AGC_TRANSITION_COUNT6_REG, &currentAgc8To1Transitions);
            (void) halReadRegister(pMac, AGC_TRANSITION_COUNT7_REG, &currentAgc7To1Transitions); 
            (void) halReadRegister(pMac, RACTL_SIGNAL_INVALID_COUNT_REG, &currentSignalInvalidCount);

            wrapAroundOffset = 0;
            if( pMac->hal.halAdaptThresh.agc9To1Transitions > currentAgc9To1Transitions )
                wrapAroundOffset = 65536;   // wrap around case
            diffAgc9To1Transitions = currentAgc9To1Transitions + wrapAroundOffset - pMac->hal.halAdaptThresh.agc9To1Transitions;

            wrapAroundOffset = 0;
            if( pMac->hal.halAdaptThresh.agc8To1Transitions > currentAgc8To1Transitions )
                wrapAroundOffset = 65536;   // wrap around case
            diffAgc8To1Transitions = currentAgc8To1Transitions + wrapAroundOffset - pMac->hal.halAdaptThresh.agc8To1Transitions;

            wrapAroundOffset = 0;
            if( pMac->hal.halAdaptThresh.agc7To1Transitions > currentAgc7To1Transitions )
                wrapAroundOffset = 65536;   // wrap around case
            diffAgc7To1Transitions = currentAgc7To1Transitions + wrapAroundOffset - pMac->hal.halAdaptThresh.agc7To1Transitions;

            wrapAroundOffset = 0;
            if( pMac->hal.halAdaptThresh.signalInvalidCount > currentSignalInvalidCount )
                wrapAroundOffset = 65536;   // wrap around case
            diffSignalInvalidCount = currentSignalInvalidCount + wrapAroundOffset - pMac->hal.halAdaptThresh.signalInvalidCount;

            if ( pMac->hal.halAdaptThresh.fEnableSamplesLogging )
            {
                HALLOGW( halLog(pMac, LOGW, FL("**********  Adaptive Threshold:  Sampling every %d us ************ \n"), pMac->hal.halAdaptThresh.sampleInterval)));

                HALLOGW( halLog(pMac, LOGW, FL("                         Previous  Current   Difference \n"))));

                HALLOGW( halLog(pMac, LOGW, FL("aModeFDR: 8To1:          %5d       %5d       %5d     \n"), 
                       pMac->hal.halAdaptThresh.agc8To1Transitions, currentAgc8To1Transitions, diffAgc8To1Transitions));

                HALLOGW( halLog(pMac, LOGW, FL("aModeFDR: signalInvalid: %5d       %5d       %5d     \n"), 
                       pMac->hal.halAdaptThresh.signalInvalidCount, currentSignalInvalidCount, diffSignalInvalidCount));     
                       
                HALLOGW( halLog(pMac, LOGW, FL("bModeFDR: 7to1:          %5d       %5d       %5d     \n"), 
                                   pMac->hal.halAdaptThresh.agc7To1Transitions, currentAgc7To1Transitions, diffAgc7To1Transitions));

                HALLOGW( halLog(pMac, LOGW, FL("bModeFDR: 9to1:          %5d       %5d       %5d     \n"), 
                                   pMac->hal.halAdaptThresh.agc9To1Transitions, currentAgc9To1Transitions, diffAgc9To1Transitions));
            }

            // store the new values
            pMac->hal.halAdaptThresh.agc9To1Transitions = currentAgc9To1Transitions;
            pMac->hal.halAdaptThresh.agc8To1Transitions = currentAgc8To1Transitions;
            pMac->hal.halAdaptThresh.agc7To1Transitions = currentAgc7To1Transitions;
            pMac->hal.halAdaptThresh.signalInvalidCount = currentSignalInvalidCount;

            // calculate FDR
            aModeFDR = diffAgc8To1Transitions + diffSignalInvalidCount;
            bModeFDR = diffAgc7To1Transitions + diffAgc9To1Transitions;

            // calculate FDR/second = FDR 
            aModeFDR = ((aModeFDR * 1000000) / (pMac->hal.halAdaptThresh.sampleInterval) );
            bModeFDR = ((bModeFDR * 1000000) / (pMac->hal.halAdaptThresh.sampleInterval) );


            if( pMac->hal.halAdaptThresh.fEnableSamplesLogging )
            {
                HALLOGW( halLog(pMac, LOGW, FL("Total aModeFDR: %d \n"), aModeFDR));

                HALLOGW( halLog(pMac, LOGW, FL("Total bModeFDR: %d \n"), bModeFDR));

                HALLOGW( halLog(pMac, LOGW, FL("False Detect Threshold:  low %d,  high %d \n"), 
                       pMac->hal.halAdaptThresh.lowFalseDetect, pMac->hal.halAdaptThresh.highFalseDetect);

                HALLOGW( halLog(pMac, LOGW, FL("TH_CD:    current hal:  %4d  (min %4d, max %4d) \n"),
                       pMac->hal.halAdaptThresh.currentTH_CD,  pMac->hal.halAdaptThresh.minTH_CD, pMac->hal.halAdaptThresh.maxTH_CD);
     
                HALLOGW( halLog(pMac, LOGW, FL("TH_D0_A:  current hal:  %4d  (min %4d, max %4d) \n"),
                       pMac->hal.halAdaptThresh.currentTH_D0_A, pMac->hal.halAdaptThresh.minTH_D0_A, pMac->hal.halAdaptThresh.maxTH_D0_A);
                             
                HALLOGW( halLog(pMac, LOGW, FL("TH_D0_B:   current hal: %4d  (min %4d, max %4d) \n"),
                       pMac->hal.halAdaptThresh.currentTH_D0_B, pMac->hal.halAdaptThresh.minTH_D0_B, pMac->hal.halAdaptThresh.maxTH_D0_B);                        
     
                HALLOGW( halLog(pMac, LOGW, FL("TH_D0_N:   current hal: %4d  (min %4d, max %4d) \n"),
                       pMac->hal.halAdaptThresh.currentTH_D0_N, pMac->hal.halAdaptThresh.minTH_D0_N, pMac->hal.halAdaptThresh.maxTH_D0_N);                        

                HALLOGW( halLog(pMac, LOGW, FL("TH_D0_DBL: current hal: %4d  (min %4d, max %4d) \n"),
                       pMac->hal.halAdaptThresh.currentTH_D0_DBL, pMac->hal.halAdaptThresh.minTH_D0_DBL, pMac->hal.halAdaptThresh.maxTH_D0_DBL);                 
            }

            if( halATH_isAdjustmentRequired( pMac, aModeFDR, bModeFDR, pMac->hal.halAdaptThresh.highFalseDetect, pMac->hal.halAdaptThresh.lowFalseDetect ) )
            {
                // FDR out of range detected
                (pMac->hal.halAdaptThresh.numFDRRangeViolated)++;

                // TODO:  Why are we using adjustmentIntervalcnt??
                //pMac->hal.halAdaptThresh.adjustmentIntervalCnt--;
                //if( pMac->hal.halAdaptThresh.adjustmentIntervalCnt ) break;

                // REVIEW!! - Does the order in which these thresholds are set matters ??
                // If both Algorithms are enabled then we will set TH_CD first and TH_D0 later
                if( pMac->hal.halAdaptThresh.algoSelection & ANI_SCH_ADAPTIVE_THRESHOLD_TH_CD )
                {
                    halATH_adjustThCD( pMac, aModeFDR, bModeFDR, pMac->hal.halAdaptThresh.highFalseDetect, pMac->hal.halAdaptThresh.lowFalseDetect );
                }

                if( pMac->hal.halAdaptThresh.algoSelection & ANI_SCH_ADAPTIVE_THRESHOLD_TH_D0 )
                {
                    halATH_adjustThD0( pMac, aModeFDR, bModeFDR );
                }
            }
     
            // reload the count
            pMac->hal.halAdaptThresh.adjustmentIntervalCnt = pMac->hal.halAdaptThresh.adjustmentInterval;
        }
        while( 0 );
    }
    
    halMTU_setAdaptThreshTimer(pMac);
#endif
    return;
}

/** ------------------------------------------------------
\fn      halATH_exitPsMode
\brief   This function takes care of restarting the Adaptive
\        Threshold timer.
\param   tpAniSirGlobal  pMac
\return  none
\ -------------------------------------------------------- */
void halATH_exitPsMode(tpAniSirGlobal pMac)
{
#ifdef FIXME_GEN5_PHY
#if defined(ANI_PRODUCT_TYPE_CLIENT)
    if (halMTU_setAdaptThreshTimer(pMac) != eHAL_STATUS_SUCCESS )
        HALLOGE( halLog(pMac, LOGE, FL("Fail to start Adaptive Thresh Timer \n")));
#endif
#endif
    return;
}

//#endif

/*
 * Airgo Networks, Inc proprietary. All rights reserved.
 *
 * Author:      Viji Alagarsamy
 * Date:        08/16/05
 * History:-
 * Date            Modified by    Modification Information
 * --------------------------------------------------------------------
 *
 */

#ifndef __HAL_ADAPT_THRSH_H__
#define __HAL_ADAPT_THRSH_H__


//#if ( WNI_POLARIS_FW_PRODUCT == STA )

/// Global Hal Adaptive Threshold structure
typedef struct sAniHalAdaptThresh
{
    // enables threshold adaption algorithm
    tANI_U8 fThresholdAdaptionEnabled;

    // indicates if scan is in progress.
    // during scan Algorithm behaves differently
    tANI_U8 fScanInProgress;

    // holds the current link state of the system
    // AT should be performed only if the link state is POST ASSOC
    tSirLinkState currentLinkState;
    
    // selects if TH_CD or TH_D0 or both need to be adjusted
    tANI_U32 algoSelection;

    // specifies in mSec the sample interval
    tANI_U32 sampleInterval;

    // specifies in units of sample interval, when to trigger adjustment
    tANI_U32 adjustmentInterval;

    // specifies if the algo needs to be implemented
    // in 2.4GHz or 5GHz or both
    tANI_U32 bandSelection;

    tANI_U32 highFalseDetect;        // upper bound of FDR which triggers threshold adjustment
    tANI_U32 lowFalseDetect;         // lower bound of FDR which triggers threshold adjustment

    // TH_CD configurable parameters
    tANI_U32 minTH_CD;               // minimum value for TH_CD
    tANI_U32 maxTH_CD;               // maximum value for TH_CD
    tANI_U32 stepTH_CD;              // step in which TH_CD can change

    // TH_D0 configurable parameters
    tANI_U32 minTH_D0_A;             // minimum value for TH_D0_A
    tANI_U32 maxTH_D0_A;             // maximum value for TH_D0_A
    tANI_U32 stepTH_D0_A;            // step in which TH_D0_A can change

    tANI_U32 minTH_D0_B;             // minimum value for TH_D0_B
    tANI_U32 maxTH_D0_B;             // maximum value for TH_D0_B
    tANI_U32 stepTH_D0_B;            // step in which TH_D0_B can change

    tANI_U32 minTH_D0_N;             // minimum value for TH_D0_N
    tANI_U32 maxTH_D0_N;             // maximum value for TH_D0_N
    tANI_U32 stepTH_D0_N;            // step in which TH_D0_N can change

    tANI_U32 minTH_D0_DBL;           // minimum value for TH_D0_DBL
    tANI_U32 maxTH_D0_DBL;           // maximum value for TH_D0_DBL
    tANI_U32 stepTH_D0_DBL;          // step in which TH_D0_DBL can change

    // internal parameters
    tANI_U32 currentTH_CD;           // current value of TH_CD
    tANI_U32 currentTH_D0_A;         // current value of TH_D0_A
    tANI_U32 currentTH_D0_B;         // current value of TH_D0_B
    tANI_U32 currentTH_D0_N;         // current value of TH_D0_11n
    tANI_U32 currentTH_D0_DBL;       // current value of TH_D0_titan40
    tANI_U32 adjustmentIntervalCnt;  // adjustment is triggered when this count goes to zero
    tANI_U32 agc9To1Transitions;
    tANI_U32 agc8To1Transitions;
    tANI_U32 signalInvalidCount;
    tANI_U32 serviceInvalidCount;
    tANI_U32 agc7To1Transitions;
    tANI_U32 currentCCAMode;
    tANI_U32 currentTH_EDET;
    tANI_U32 currentTH_CS;

    // stats
    tANI_U32 numTH_CDAdjustedUp;        // number of times threshold was adjusted with + delta
    tANI_U32 numTH_CDAdjustedDown;      // number of times threshold was adjusted with - delta
    tANI_U32 numTH_D0_A_AdjustedUp;     // number of times threshold was adjusted with + delta
    tANI_U32 numTH_D0_A_AdjustedDown;   // number of times threshold was adjusted with - delta
    tANI_U32 numTH_D0_B_AdjustedUp;     // number of times threshold was adjusted with + delta
    tANI_U32 numTH_D0_B_AdjustedDown;   // number of times threshold was adjusted with - delta
    tANI_U32 numTH_D0_N_AdjustedUp;     // number of times threshold was adjusted with + delta
    tANI_U32 numTH_D0_N_AdjustedDown;   // number of times threshold was adjusted with - delta
    tANI_U32 numTH_D0_DBL_AdjustedUp;   // number of times threshold was adjusted with + delta
    tANI_U32 numTH_D0_DBL_AdjustedDown; // number of times threshold was adjusted with - delta
    tANI_U32 numFDRRangeViolated;       // number of times FDR rate was out of specified range

    tANI_U8 fEnableSamplesLogging;
} tAniHalAdaptThresh, *tpAniHalAdaptThresh;

void halATH_initialize(tpAniSirGlobal pMac);

void halATH_enableAlgorithm(tpAniSirGlobal pMac, tANI_U8 fEnable);

void halATH_selectAlgorithm(tpAniSirGlobal pMac, tANI_U32 Algo);

void halATH_selectBand(tpAniSirGlobal pMac, tANI_U32 Band);

void halATH_selectAdjustmentIntervals(tpAniSirGlobal pMac, tANI_U32 AdjustmentInterval);

void halATH_selectSampleIntervals(tpAniSirGlobal pMac, tANI_U32 SampleInterval);

void halATH_setThCd(tpAniSirGlobal pMac, tANI_U32 min, tANI_U32 max, tANI_U32 step );

void halATH_setThD011a(tpAniSirGlobal pMac, tANI_U32 min, tANI_U32 max, tANI_U32 step );

void halATH_setThD011b(tpAniSirGlobal pMac, tANI_U32 min, tANI_U32 max, tANI_U32 step );

void halATH_setThD011n(tpAniSirGlobal pMac, tANI_U32 min, tANI_U32 max, tANI_U32 step );

void halATH_setThD011dbl(tpAniSirGlobal pMac, tANI_U32 min, tANI_U32 max, tANI_U32 step );

void halATH_setFDRThresholds(tpAniSirGlobal pMac, tANI_U32 high, tANI_U32 low);

void halSetDefaultThresholdValues(tpAniSirGlobal pMac);

void halATH_handleScanStart(tpAniSirGlobal pMac);

void halATH_handleScanStop(tpAniSirGlobal pMac);

void halHandlePS_Start(tpAniSirGlobal pMac);

void halHandlePS_Stop(tpAniSirGlobal pMac);

void halATH_handlePSStart(tpAniSirGlobal pMac);

void halATH_handlePSStop(tpAniSirGlobal pMac);

void halATH_adjustAdaptiveThreshold(tpAniSirGlobal pMac);

void halATH_adjustThCD( tpAniSirGlobal pMac, tANI_U32 aModeFDR, tANI_U32 bModeFDR, 
                     tANI_U32 highFalseDetectThreshold, tANI_U32 lowFalseDetectThreshold);

void halATH_exitPsMode(tpAniSirGlobal pMac);

void halATH_setLinkState(tpAniSirGlobal pMac, tSirLinkState linkState);

void halATH_setAlgorithm(tpAniSirGlobal pMac);

//#endif
#endif

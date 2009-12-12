/******************************************************************************
*
* Name:  pmcApi.h
*
* Description: Power Management Control (PMC) API definitions.
*
* Copyright 2008 (c) Qualcomm, Incorporated.  
* All Rights Reserved.
* Qualcomm Confidential and Proprietary.
*
******************************************************************************/

#ifndef __PMC_API_H__
#define __PMC_API_H__

#define BMPS_TRAFFIC_TIMER_DEFAULT 5000  //unit = ms

/* Power events that are signaled to PMC. */
typedef enum ePmcPowerEvent
{
    ePMC_SYSTEM_HIBERNATE,  /* host is entering hibernation */
    ePMC_SYSTEM_RESUME,  /* host is resuming after hibernation */
    ePMC_HW_WLAN_SWITCH_OFF,  /* Hardware WLAN Switch has been turned off */
    ePMC_HW_WLAN_SWITCH_ON,  /* Hardware WLAN Switch has been turned on */
    ePMC_SW_WLAN_SWITCH_OFF,  /* Software WLAN Switch has been turned off */
    ePMC_SW_WLAN_SWITCH_ON,  /* Software WLAN Switch has been turned on */
    ePMC_BATTERY_OPERATION,  /* host is now operating on battery power */
    ePMC_AC_OPERATION  /* host is now operating on AC power */
} tPmcPowerEvent;


/* Power saving modes. */
typedef enum ePmcPowerSavingMode
{
    ePMC_IDLE_MODE_POWER_SAVE,  /* Idle Mode Power Save (IMPS) */
    ePMC_BEACON_MODE_POWER_SAVE,  /* Beacon Mode Power Save (BMPS) */
    ePMC_SPATIAL_MULTIPLEX_POWER_SAVE,  /* Spatial Multiplexing Power Save (SMPS) */
    ePMC_UAPSD_MODE_POWER_SAVE,  /* Unscheduled Automatic Power Save Delivery Mode */
    ePMC_STANDBY_MODE_POWER_SAVE,  /* Standby Power Save Mode */
    ePMC_WOWL_MODE_POWER_SAVE  /* Wake-on-Wireless LAN Power Save Mode */
} tPmcPowerSavingMode;


/* Switch states. */
typedef enum ePmcSwitchState
{
    ePMC_SWITCH_OFF,  /* switch off */
    ePMC_SWITCH_ON  /* switch on */
} tPmcSwitchState;


/* Device power states. */
typedef enum ePmcPowerState
{
    ePMC_FULL_POWER,  /* full power */
    ePMC_LOW_POWER,  /* low power */
} tPmcPowerState;
 
/* PMC states. */
typedef enum ePmcState
{
    STOPPED, /* PMC is stopped */
    FULL_POWER, /* full power */
    LOW_POWER, /* low power */
    REQUEST_IMPS,  /* requesting IMPS */
    IMPS,  /* in IMPS */
    REQUEST_BMPS,  /* requesting BMPS */
    BMPS,  /* in BMPS */
    REQUEST_FULL_POWER,  /* requesting full power */
    REQUEST_START_UAPSD,  /* requesting Start UAPSD */
    REQUEST_STOP_UAPSD,  /* requesting Stop UAPSD */
    UAPSD,           /* in UAPSD */
    REQUEST_STANDBY,  /* requesting standby mode */
    STANDBY,  /* in standby mode */
    REQUEST_ENTER_WOWL, /* requesting enter WOWL */
    REQUEST_EXIT_WOWL,  /* requesting exit WOWL */
    WOWL                /* Chip in WOWL mode */
} tPmcState;

/* Which beacons should be forwarded to the host. */
typedef enum ePmcBeaconsToForward
{
    ePMC_NO_BEACONS,  /* none */
    ePMC_BEACONS_WITH_TIM_SET,  /* with TIM set */
    ePMC_BEACONS_WITH_DTIM_SET,  /* with DTIM set */
    ePMC_NTH_BEACON,  /* every Nth beacon */
    ePMC_ALL_BEACONS  /* all beacons */
} tPmcBeaconsToForward;


/* The Spatial Mulitplexing Power Save modes. */
typedef enum ePmcSmpsMode
{
    ePMC_DYNAMIC_SMPS,  /* dynamic SMPS */
    ePMC_STATIC_SMPS  /* static SMPS */
} tPmcSmpsMode;


/* Configuration parameters for Idle Mode Power Save (IMPS). */
typedef struct sPmcImpsConfigParams
{
    tANI_BOOLEAN enterOnAc;  /* FALSE if device should enter IMPS only when host operating
                                on battery power, TRUE if device should enter always */
} tPmcImpsConfigParams, *tpPmcImpsConfigParams;


/* Configuration parameters for Beacon Mode Power Save (BMPS). */
typedef struct sPmcBmpsConfigParams
{
    tANI_BOOLEAN enterOnAc;  /* FALSE if device should enter BMPS only when host operating on
                                battery power, TRUE if device should enter always */
    tANI_U32 txThreshold;  /* transmit rate under which BMPS should be entered (frames / traffic measurement period) */
    tANI_U32 rxThreshold;  /* receive rate under which BMPS should be entered (frames / traffic measurement period) */
    tANI_U32 trafficMeasurePeriod; /* period for BMPS traffic measurement (milliseconds) */
    tANI_U32 bmpsPeriod;  /* amount of time in low power (beacon intervals) */
    tPmcBeaconsToForward forwardBeacons;  /* which beacons should be forwarded to the host */
    tANI_U32 valueOfN;  /* the value of N when forwardBeacons is set to ePMC_NTH_BEACON */
    tANI_BOOLEAN usePsPoll;  /* TRUE if PS-POLL should be used to retrieve frames from AP, FALSE if a
                                null data frame with the PM bit reset should be used */
    tANI_BOOLEAN setPmOnLastFrame; /* TRUE to keep device in BMPS as much as possible, FALSE otherwise, TRUE means:
                                      1) PM bit should be set on last pending transmit data frame
                                      2) null frame with PM bit set should be transmitted after last pending receive
                                         frame has been processed */
} tPmcBmpsConfigParams, *tpPmcBmpsConfigParams;


/* Configuration parameters for Spatial Mulitplexing Power Save (SMPS). */
typedef struct sPmcSmpsConfigParams
{
    tPmcSmpsMode mode;  /* mode to use */
    tANI_BOOLEAN enterOnAc;  /* FALSE if device should enter SMPS only when host operating on
                                battery power, TRUE if device should enter always */
} tPmcSmpsConfigParams, *tpPmcSmpsConfigParams;


/* Routine definitions. */
extern eHalStatus pmcOpen (tHalHandle hHal);
extern eHalStatus pmcStart (tHalHandle hHal);
extern eHalStatus pmcStop (tHalHandle hHal);
extern eHalStatus pmcClose (tHalHandle hHal );
extern eHalStatus pmcSignalPowerEvent (tHalHandle hHal, tPmcPowerEvent event);
extern eHalStatus pmcSetConfigPowerSave (tHalHandle hHal, tPmcPowerSavingMode psMode, void *pConfigParams);
extern eHalStatus pmcGetConfigPowerSave (tHalHandle hHal, tPmcPowerSavingMode psMode, void *pConfigParams);
extern eHalStatus pmcEnablePowerSave (tHalHandle hHal, tPmcPowerSavingMode psMode);
extern eHalStatus pmcStartAutoBmpsTimer (tHalHandle hHal);
extern eHalStatus pmcStopAutoBmpsTimer (tHalHandle hHal);
extern eHalStatus pmcDisablePowerSave (tHalHandle hHal, tPmcPowerSavingMode psMode);
extern eHalStatus pmcQueryPowerState (tHalHandle hHal, tPmcPowerState *pPowerState, tPmcSwitchState *pHwWlanSwitchState,
                                      tPmcSwitchState *pSwWlanSwitchState);
extern tANI_BOOLEAN pmcIsPowerSaveEnabled (tHalHandle hHal, tPmcPowerSavingMode psMode);
extern eHalStatus pmcRequestFullPower (tHalHandle hHal, void (*callbackRoutine) (void *callbackContext, eHalStatus status),
                                       void *callbackContext, tRequestFullPowerReason fullPowerReason);
extern eHalStatus pmcRequestImps (tHalHandle hHal, tANI_U32 impsPeriod,
                                  void (*callbackRoutine) (void *callbackContext, eHalStatus status),
                                  void *callbackContext);
extern eHalStatus pmcRegisterPowerSaveCheck (tHalHandle hHal, tANI_BOOLEAN (*checkRoutine) (void *checkContext),
                                             void *checkContext);
extern eHalStatus pmcDeregisterPowerSaveCheck (tHalHandle hHal, tANI_BOOLEAN (*checkRoutine) (void *checkContext));
extern void pmcMessageProcessor (tHalHandle hHal, tSirSmeRsp *pMsg);

extern eHalStatus pmcRequestBmps (
   tHalHandle hHal,
   void (*callbackRoutine) (void *callbackContext, eHalStatus status),
   void *callbackContext);

extern eHalStatus pmcStartUapsd (
   tHalHandle hHal,
   void (*callbackRoutine) (void *callbackContext, eHalStatus status),
   void *callbackContext);

extern eHalStatus pmcStopUapsd (tHalHandle hHal);

extern eHalStatus pmcRequestStandby (
   tHalHandle hHal,
   void (*callbackRoutine) (void *callbackContext, eHalStatus status),
   void *callbackContext);

extern eHalStatus pmcRegisterDeviceStateUpdateInd (tHalHandle hHal, 
   void (*callbackRoutine) (void *callbackContext, tPmcState pmcState),
   void *callbackContext);

extern eHalStatus pmcDeregisterDeviceStateUpdateInd (tHalHandle hHal, 
   void (*callbackRoutine) (void *callbackContext, tPmcState pmcState));

extern eHalStatus pmcReady(tHalHandle hHal);

void pmcDumpInit(tHalHandle hHal);

extern eHalStatus pmcWowlAddBcastPattern (
   tHalHandle hHal, 
   tpSirWowlAddBcastPtrn pattern);

extern eHalStatus pmcWowlDelBcastPattern (
   tHalHandle hHal, 
   tpSirWowlDelBcastPtrn pattern);

extern eHalStatus pmcEnterWowl ( 
    tHalHandle hHal, 
    void (*callbackRoutine) (void *callbackContext, eHalStatus status),   
    void *callbackContext, tpSirSmeWowlEnterParams wowlEnterParams);

extern eHalStatus pmcExitWowl (tHalHandle hHal);

#endif

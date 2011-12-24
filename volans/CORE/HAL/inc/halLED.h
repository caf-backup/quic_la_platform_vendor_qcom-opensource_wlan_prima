/*
 * Copyright (c) 2011 Qualcomm Atheros, Inc. 
 * All Rights Reserved. 
 * Qualcomm Atheros Confidential and Proprietary. 
 * 
 * Copyright (C) 2006 Airgo Networks, Incorporated
 * 
 * halLED.h:  Handles LED feature.
 * Author:    Naveen G
 * Date:      06/27/2007
 *
 * --------------------------------------------------------------------------
 */
#ifndef __HALLED_H_
#define __HALLED_H_

/** Enable LED functionality by default for station.*/
#if 0 //FIXME_NO_VIRGO
#if defined(ANI_PRODUCT_TYPE_CLIENT)
#define ANI_LED_ENABLE
#endif
#endif

#include  "aniGlobal.h"
#include  "palTypes.h"
#include  "halTypes.h"
#include  "sirParams.h"     // tSirMsgQ

typedef enum {
    eHAL_LINK_LED,
    eHAL_TRAFFIC_LED,
    eHAL_POWER_LED,
    eHAL_SCAN_LED
}tHalLedType;

typedef enum {
    eHAL_LED_ON,
    eHAL_LED_OFF,
    eHAL_LED_BLINK
}tHalLedState;

/** Structure to store LED related information.*/
typedef struct sAniHalLedConfig
{
    tANI_U16       sid;
    tANI_U16       svid;
    tANI_U32       cardType;
    tANI_U8        idleOnTh;
    tANI_U8        idleOffTh;
    tANI_U8        scanOnTh;
    tANI_U8        scanOffTh;
    tANI_U8        trafficOnTh;
    tANI_U8        trafficOffTh;

    tANI_BOOLEAN   singleLed;
    tHalLedState   powerInd;
    tHalLedState   trafficInd;
    tHalLedState   scanInd;
    tANI_BOOLEAN   bEnable;
    tANI_BOOLEAN   bInvert;
}tHalLedConfig, *tpHalLedConfig;

typedef struct sAniHalLedParam
{
    tANI_U8        OnCnt;             /**< Number of interval for which the LED is on.*/
    tANI_U8        OffCnt;            /**< Number of interval for which the LED is off.*/
    tANI_U8        trafficOnCnt;      /**< Number of interval for which the LED is on.*/
    tANI_U8        trafficOffCnt;     /**< Number of interval for which the LED is off.*/
    tANI_BOOLEAN   linkLedOn;        /**< Indicates the current associated state of Station.*/
    tANI_U32       prevTxNum;        /**< Cached value of Tx Packet Count.*/
    tANI_U32       prevRxNum;        /**< Cached value of Rx Packet Count.*/
    tANI_U32       ledGPIOReg;       /**< Cached value GPIO Reg to avoid Register Reads.*/
    tANI_BOOLEAN   scanIndTriggered;

    tHalLedConfig  config;
}tHalLedParam, *tpHalLedParam;

#if 0 //FIXME_NO_VIRGO
void halLedHandler(tpAniSirGlobal pMac);
eHalStatus halInitLed(tpAniSirGlobal pMac);
eHalStatus halCloseLed(tpAniSirGlobal pMac);
void halSetLed(tpAniSirGlobal pMac, tHalLedType ledType, tHalLedState ledState);
#endif

#endif //__HALLED_H_

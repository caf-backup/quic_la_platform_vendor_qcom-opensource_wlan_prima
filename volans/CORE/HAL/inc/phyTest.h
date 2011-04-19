/** ------------------------------------------------------------------------- *
    ------------------------------------------------------------------------- *


    \file phyTest.h

    \brief test definitions that affect physical layer operations

    $Id$

    Copyright (C) 2006 Airgo Networks, Incorporated


   ========================================================================== */

#ifndef PHYTEST_H
#define PHYTEST_H

#include "halPhy.h"
#include <phyTxPower.h>

//supports testing of closed-loop power control
typedef enum
{
    FORCE_CLOSED_LOOP_GAIN     = 0,  //phyDbg pkt gen only uses gain index 0 when we are taking measurements with the closed-loop gain
    FORCE_POWER_TEMPLATE_INDEX = 1,  //only use forced power template index
    FIXED_POWER_DBM            = 2,  //only use to specify fixed power, ignoring rate/channel/reg limits
    REGULATORY_POWER_LIMITS    = 3,  //use production power Lut settings limited by power limit table per channel
    RATE_POWER_NON_LIMITED     = 4,   //use power specified per rate and channel group, but don't limit power by channel
    POWER_INDX_SRC_MAX_VAL      = 0xFFFFFFFF, //dummy val to set enum to 4 bytes
}ePowerTempIndexSource;


typedef struct
{
    //test variables used for MFG_DIAG or other testing
    //these are here because they affect the operation of the physical layer and must be shared with PTT for control
    tANI_BOOLEAN testDisableRfAccess;
    tANI_BOOLEAN testDisablePhyRegAccess;
    tANI_BOOLEAN testTpcClosedLoop;
    tANI_U8 testChannelId;
    ePhyChanBondState testCbState;
    ePowerTempIndexSource testTxGainIndexSource;
    t2Decimal testForcedTxPower;                           //force this power output based on calibration
    tPwrTemplateIndex testLastPwrIndex;
    tPwrTemplateIndex testForcedTxGainIndex;               //force this gain index for all rates/channels
    tANI_BOOLEAN testLogPhyRegisters;
    tANI_BOOLEAN testLogSpiRegs;
    tANI_BOOLEAN testDisableFilterCalOptimization;
    tANI_U8 testDcoCalGainIndex;
    eCalSelection testCalMode;
    tANI_BOOLEAN testInternalHdetCal;
    tANI_BOOLEAN identicalPayloadEnabled;
    tANI_U8 reserved[2];
}tPhyTest;

#endif


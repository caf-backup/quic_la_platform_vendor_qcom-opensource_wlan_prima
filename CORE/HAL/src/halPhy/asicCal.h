/**
 *
   Airgo Networks, Inc proprietary.
   All Rights Reserved, Copyright 2005
   This program is the confidential and proprietary product of Airgo Networks Inc.
   Any Unauthorized use, reproduction or transfer of this program is strictly prohibited.


   asicCal.h: Abstraction of Phy Calibration Engine
   Author:  Mark Nelson
   Date:    3/4/05

   History -
   Date        Modified by              Modification Information
  --------------------------------------------------------------------------

 */

#ifndef ASICCAL_H
#define ASICCAL_H

#define CAL_STATUS_MASK                 QWLAN_CAL_STATUS_STATUS_MASK
#define CAL_STATUS_OFFSET               QWLAN_CAL_STATUS_STATUS_OFFSET

#define CAL_MODE_REG                    QWLAN_CAL_CALMODE_REG
#define CAL_MODE_INTERPOLATE_MASK       QWLAN_CAL_CALMODE_INTERPOLATE_MASK
#define CAL_MODE_INTERPOLATE_OFFSET     QWLAN_CAL_CALMODE_INTERPOLATE_OFFSET
#define CAL_MODE_MASK                   QWLAN_CAL_CALMODE_MODE_MASK
#define CAL_MODE_OFFSET                 QWLAN_CAL_CALMODE_MODE_OFFSET

#define CAL_MODE_ENORMAL                QWLAN_CAL_CALMODE_MODE_ENORMAL
#define CAL_MODE_EINITDCCAL             QWLAN_CAL_CALMODE_MODE_EINITDCCAL
#define CAL_MODE_ERESDCCAL              QWLAN_CAL_CALMODE_MODE_ERESDCCAL
#define CAL_MODE_EIQCAL                 QWLAN_CAL_CALMODE_MODE_EIQCAL
#define CAL_MODE_ETXLOCAL               QWLAN_CAL_CALMODE_MODE_ETXLOCAL


#define CAL_MEM                         QWLAN_CAL_IQ_CORR_COEFF_MEM_MREG
#define CAL_MEM_PHASE_VAL_WIDTH         (9)                             //each value is 9 bits wide
#define CAL_MEM_CHAIN_MULT              (3 * QWLAN_CAL_MEM_PHASE_VAL_WIDTH)   //each chain takes 27 bits
#define CAL_MEM_GAIN_MULT               (32)                            //gain indexes are on 32 byte boundaries


#define DCO_TIMER_ON_40MHZ_VALUE    11
#define DCO_TIMER_OFF_40MHZ_VALUE   10
#define DCO_TIMER_ON_80MHZ_VALUE    22
#define DCO_TIMER_OFF_80MHZ_VALUE   20

typedef enum
{
    PHY_CAL_MODE_NORMAL     = CAL_MODE_ENORMAL,
    PHY_CAL_MODE_INITDCCAL  = CAL_MODE_EINITDCCAL,
    PHY_CAL_MODE_RESDCCAL   = CAL_MODE_ERESDCCAL,
    PHY_CAL_MODE_IQCAL      = CAL_MODE_EIQCAL,
    PHY_CAL_MODE_TXLOCAL    = CAL_MODE_ETXLOCAL
}eCalMode;


typedef enum
{
    PHY_CAL_IDLE,
    PHY_CAL_DONE,
    PHY_CAL_BUSY,
    PHY_CAL_ERROR,
    PHY_CAL_UNFINISHED
}ePhyCalState;

#define DCO_ERROR_TOLERANCE (50)    // for quasar, original 70.

typedef struct
{
    tANI_U32 rxIqCache[NUM_RX_GAIN_STEPS][8];    //use this to keep a record of what is written to the baseband asic so we don't have to read it back to modify it.
}tAsicCal;

#endif /* ASICCAL_H */

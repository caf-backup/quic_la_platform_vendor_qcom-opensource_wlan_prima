/**
 *
   Airgo Networks, Inc proprietary.
   All Rights Reserved, Copyright 2005
   This program is the confidential and proprietary product of Airgo Networks Inc.
   Any Unauthorized use, reproduction or transfer of this program is strictly prohibited.


   asicPMU.h:
   Author:  Mark Nelson
   Date:    3/10/05

   History -
   Date        Modified by              Modification Information
  --------------------------------------------------------------------------

 */

#ifndef ASICPMU_H
#define ASICPMU_H


//define PMU mode settings for PMU_CAL_DCO_EN


#define SET_PMU_CAL_DCO_EN_MODE


#define SET_PMU_CAL_IQ_EN_MODE

#define SET_PMU_NO_MANUAL_ENABLES

#define SET_PMU_CAL_TX_LO_EN_MODE


//TODO: Do we have this for Taurus?
typedef enum
{
    PMU_RADIO_OFF,
    PMU_RADIO_2_4_GHZ,
    PMU_RADIO_5_GHZ,
    PMU_RADIO_5_GHZ_UNII
}ePMURadioControl;


//this enumeration defines a set of known states that indicate how to set the PMU registers
typedef enum
{
    PMU_AUTO_EN,
    PMU_CAL_DCO_EN,
    PMU_CAL_TX_LO_EN,
    PMU_CAL_IQ_EN,
    MAX_PMU_STATE
}ePMUEnableStatus;


typedef struct
{
    tANI_BOOLEAN psDisabled;
}tAsicPmu;


#endif /* ASICPMU_H */

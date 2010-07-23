/**
 *
   Airgo Networks, Inc proprietary.
   All Rights Reserved, Copyright 2005
   This program is the confidential and proprietary product of Airgo Networks Inc.
   Any Unauthorized use, reproduction or transfer of this program is strictly prohibited.


   asicTXCTL.h:
   Author:  Mark Nelson
   Date:    3/22/05

   History -
   Date        Modified by              Modification Information
  --------------------------------------------------------------------------

 */

#ifndef ASICTXCTL_H
#define ASICTXCTL_H



#define TXCTL_FIR_TX_MODE_REG                               QWLAN_TXCTL_FIR_MODE_REG
#define TXCTL_FIR_TX_MODE_SELECT_MASK                       QWLAN_TXCTL_FIR_MODE_SELECT_FIRMODE_MASK


//#define TXCTL_DAC_CONTROL_REG           TXCTL_DAC_CONTROL_REG
#define TXCTL_DAC_RF_EN_MASK            (QWLAN_TXCTL_DAC_CONTROL_TXEN0_OVERRIDE_VAL_MASK)
#define TXCTL_DAC_RF_EN_OFFSET          (4)

#define TXCTL_DAC_STBY_MASK             (QWLAN_TXCTL_DAC_CONTROL_CH0STDBY_OVERRIDE_VAL_MASK)
#define TXCTL_DAC_STBY_OFFSET           (0)

#define TXCTL_DAC_OVERRIDE_EN_MASK      QWLAN_TXCTL_DAC_CONTROL_DAC_OVERRIDE_EN_MASK
#define TXCTL_RF_OVERRIDE_EN_MASK       QWLAN_TXCTL_DAC_CONTROL_TXEN_OVERRIDE_EN_MASK
#define TXCTL_DAC_MODE_MASK             QWLAN_TXCTL_DAC_CONTROL_DAC_ENABLE_MODE_MASK
#define DAC_WARMUP                      QWLAN_TXCTL_DAC_CONTROL_DAC_ENABLE_MODE_EWARMUP
#define DAC_RAMPUP                      QWLAN_TXCTL_DAC_CONTROL_DAC_ENABLE_MODE_ERAMPUP
#define DAC_PREAMB                      QWLAN_TXCTL_DAC_CONTROL_DAC_ENABLE_MODE_EPREAMB


#define TXCTL_DAC_TX0   BIT_0


#endif /* ASICTXCTL_H */

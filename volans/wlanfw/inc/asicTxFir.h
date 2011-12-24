/**
 *
   Copyright (c) 2011 Qualcomm Atheros, Inc. 
   All Rights Reserved. 
   Qualcomm Atheros Confidential and Proprietary. 
  
   Copyright (C) 2006 Airgo Networks, Incorporated
  
   asicTxFir.h: 
   Author:  Mark Nelson
   Date:    3/25/05

   History - 
   Date        Modified by              Modification Information
  --------------------------------------------------------------------------

 */

#ifndef ASICTXFIR_H
#define ASICTXFIR_H



#define TXFIR_MEM                           QWLAN_TXFIR_TXCAL_MEM0_MREG




#define TXFIR_MEM_GAIN_MULT                 (16)    //bytes per gain
#define TXFIR_MEM_PER_CHAIN                 (16 * TXFIR_MEM_GAIN_MULT)    //# of gains per chain * bytes per gain



typedef tIQAdc tTxCarrierError;

typedef struct
{
    tANI_U32 txIqLoCache[PHY_MAX_TX_CHAINS][NUM_TX_GAIN_STEPS][4];
    tANI_U32 spatialRotation;
}tAsicTxFir;


void asicTxFirApplyOfdmCoef(eRfChannels channel);

#endif /* ASICTXFIR_H */

#ifndef RFQUASARFILTERCAL_H
#define RFQUASARFILTERCAL_H

typedef struct
{
    tANI_U16 numSamplesTx;                  //the number of grab ram samples needed to get good numbers for transmit cal
    tANI_U16 numSamplesRx;                  //the number of grab ram samples needed to get good numbers for receive cal
    
    tANI_BOOLEAN freqList[NUM_RF_CHANNELS]; // TRUE means cal it
    
    tANI_U8 rxGainVal[NUM_RF_SUBBANDS];     // = { 113, 121, 121, 121, 121 }
    tANI_U8 txGainVal[NUM_RF_SUBBANDS];     // = { 2, 2, 2, 2, 2 }
    tANI_U8 tx_gain;                        // = 8
    tANI_U8 rx_gain;                        // = 19
    
    tANI_U8 cal_gain_index;                 //=40
}rfQuasarFilterCalParams;


#endif /* RFQUASARFILTERCAL_H */

#ifndef _RFMIDAS_H_
#define _RFMMIDAS_H_

#include <wlan_bit.h>

typedef struct
{
    tANI_U16 ver;
    tANI_U16 unused;
}tRfChipVer;


typedef enum
{
    RF_CAL_NORMAL,
    RF_CAL_TX_LO_START, //setup necessary registers to start Tx carrier suppression cal
    RF_CAL_TX_LO_END,   //restore necessary registers after Tx carrier suppression cal
    RF_CAL_TX_IQ_START,
    RF_CAL_TX_IQ_END,
    RF_CAL_RX_IQ_START,
    RF_CAL_RX_IQ_END,
    RF_CAL_RX_DPD_START,
    RF_CAL_RX_DPD_END,

    MAX_RF_CAL_MODE
}eRfCalMode;





#endif /* _RFMIDAS_H_ */

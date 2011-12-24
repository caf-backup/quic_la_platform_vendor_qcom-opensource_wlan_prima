/**
 *

   Copyright (c) 2011 Qualcomm Atheros, Inc. 
   All Rights Reserved. 
   Qualcomm Atheros Confidential and Proprietary. 
  
   Copyright (C) 2006 Airgo Networks, Incorporated

   drvWaveGen.h: Interface to setup, start, and stop a test waveform in Polaris or Titan
   Author:  Mark Nelson
   Date:    2/15/05

   History -
   Date        Modified by              Modification Information
  --------------------------------------------------------------------------

 */

#ifndef ASICWFM_H
#define ASICWFM_H

#include <wlan_bit.h>
#include <asicTypes.h>

#ifdef __cplusplus
extern "C"
{
#endif


#define WFM_CLK_80  BIT_3
#define WFM_START   BIT_0
#define WFM_STOP    BIT_1

#define WFM_MEM_I_DATA_MASK         (0x7FF)
#define WFM_MEM_Q_DATA_OFFSET       (0xB)
#define WFM_MEM_Q_DATA_MASK         (0x3FF800)

typedef enum
{
    WAVE_SINGLE_SHOT = 0,
    WAVE_CONTINUOUS = BIT_2
}eWaveMode;


#define MAX_TONE_AMPLITUDE  (2^11)  // peak to peak

#define MAX_TEST_WAVEFORM_SAMPLES    500

typedef tIQDac tWaveformSample;     //11-bit signed values

#define NUM_RX_IMB_CAL_TONES    4

#define CAL_WFM_TX_TONE_8_START_IDX         0
#define CAL_WFM_TX_TONE_8_STOP_IDX          255
#define CAL_WFM_TX_TONE_MINUS_8_START_IDX   256
#define CAL_WFM_TX_TONE_MINUS_8_STOP_IDX    511
#define CAL_WFM_RX_TONE_START_IDX           512
#define CAL_WFM_RX_TONE_STOP_IDX            767

extern const tWaveformSample tone8[256];


#ifdef __cplusplus
}
#endif


#endif /* ASICWFM_H */

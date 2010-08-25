/**
 *
   Airgo Networks, Inc proprietary.
   All Rights Reserved, Copyright 2005
   This program is the confidential and proprietary product of Airgo Networks Inc.
   Any Unauthorized use, reproduction or transfer of this program is strictly prohibited.


   drvWaveGen.h: Interface to setup, start, and stop a test waveform in Polaris or Titan
   Author:  Mark Nelson
   Date:    2/15/05

   History -
   Date        Modified by              Modification Information
  --------------------------------------------------------------------------

 */

#ifndef ASICWFM_H
#define ASICWFM_H

#include "wlan_bit.h"
#include "asicTypes.h"

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

#define MAX_TEST_WAVEFORM_SAMPLES 320

typedef tIQDac tWaveformSample;     //11-bit signed values

#define NUM_TEST_WAVE_1_SAMPLES 128
extern const tWaveformSample testWave1[NUM_TEST_WAVE_1_SAMPLES];

#define NUM_PREAMBLE_A_SAMPLES MAX_TEST_WAVEFORM_SAMPLES
extern const tWaveformSample preambleAWaveform[NUM_PREAMBLE_A_SAMPLES];

typedef enum
{
    CAL_WFM_TX_AMP_A,
    CAL_WFM_TX_AMP_B,
    CAL_WFM_TX_PHASE_TONE_A2,
    CAL_WFM_TX_PHASE_TONE_A4,
    CAL_WFM_TX_PHASE_TONE_A8,
    CAL_WFM_TX_PHASE_TONE_A16,
    CAL_WFM_TX_PHASE_TONE_A32,
    CAL_WFM_TX_PHASE_TONE_B2,
    CAL_WFM_TX_PHASE_TONE_B4,
    CAL_WFM_TX_PHASE_TONE_B8,
    CAL_WFM_TX_PHASE_TONE_B16,
    CAL_WFM_TX_PHASE_TONE_B32,
    CAL_WFM_RX_TONE_2,
    CAL_WFM_RX_TONE_4,
    CAL_WFM_RX_TONE_8,
    CAL_WFM_RX_TONE_16,
    CAL_WFM_RX_TONE_32,
    
    NUM_CAL_WFM_SECTIONS
}eCalWaveform;


typedef struct
{
    tANI_U32 start;
    tANI_U32 end;
}sCalWfmIndices;


#define NUM_CAL_WFM_SAMPLES 250
#define NUM_RX_IMB_CAL_TONES    4

#define CAL_WFM_TX_TONE_8_START_IDX         48
#define CAL_WFM_TX_TONE_8_STOP_IDX          55
#define CAL_WFM_TX_TONE_MINUS_8_START_IDX   108
#define CAL_WFM_TX_TONE_MINUS_8_STOP_IDX    115
#define CAL_WFM_RX_TONE_START_IDX           120
#define CAL_WFM_RX_TONE_STOP_IDX            183

extern const tWaveformSample calWaveform[NUM_CAL_WFM_SAMPLES];

extern const sCalWfmIndices calWfmIndices[NUM_CAL_WFM_SECTIONS];

extern const tWaveformSample tone8[256];

extern const tWaveformSample pWave[184];

#ifdef __cplusplus
}
#endif


#endif /* ASICWFM_H */

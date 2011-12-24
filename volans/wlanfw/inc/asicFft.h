/** ------------------------------------------------------------------------- * 
    ------------------------------------------------------------------------- *  

  
    \file asicFft.h
  
    \brief FFT block definitions
  
    $Id$ 
  
  
    Copyright (c) 2011 Qualcomm Atheros, Inc. 
    All Rights Reserved. 
    Qualcomm Atheros Confidential and Proprietary. 
  
    Copyright (C) 2006 Airgo Networks, Incorporated
  
   ========================================================================== */

#ifndef ASICFFT_H
#define ASICFFT_H

#define FFT_RAM                     QWLAN_FFT_FFT_RAM_MREG
#define FFT_TONE_INDEX_MULT         (0x10)

#define MAX_TONES_DATA_COLLECTION   8

typedef struct
{
    tANI_S12 real;
    tANI_S12 imag;
}sFftToneData;


typedef struct
{
    sFftToneData hiTone;
    sFftToneData loTone;
}sTwoToneData;

typedef sFftToneData sToneDataCollection[MAX_TONES_DATA_COLLECTION];

#endif


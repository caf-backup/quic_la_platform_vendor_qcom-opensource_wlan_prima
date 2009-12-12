/** ------------------------------------------------------------------------- * 
    ------------------------------------------------------------------------- *  

  
    \file asicFft.h
  
    \brief FFT block definitions
  
    $Id$ 
  
    Copyright (C) 2006 Airgo Networks, Incorporated
  
  
   ========================================================================== */

#ifndef ASICFFT_H
#define ASICFFT_H

#define FFT_RAM                     FFT_FFT_RAM_MREG
#define FFT_TONE_INDEX_MULT         (0x20)



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


#endif


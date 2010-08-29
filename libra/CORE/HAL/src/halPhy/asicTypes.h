/** ------------------------------------------------------------------------- * 
    ------------------------------------------------------------------------- *  

  
    \file asicTypes.h
  
    \brief Holds core types needed in asic headers.
  
    $Id$ 
  
    Copyright (C) 2006 Airgo Networks, Incorporated
  
  
   ========================================================================== */

#ifndef ASICTYPES_H
#define ASICTYPES_H




typedef struct
{
    tANI_S16 I;   //ADC sample of PHY_I_RAIL
    tANI_S16 Q;   //ADC sample of PHY_Q_RAIL
}tIQSamples;

typedef tIQSamples tIQAdc;
typedef tIQSamples tIQDac;

#endif

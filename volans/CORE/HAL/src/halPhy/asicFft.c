/** ------------------------------------------------------------------------- * 
    ------------------------------------------------------------------------- *  

  
    \file asicFft.c
  
    \brief Interfaces to the FFT block
  
    $Id$ 
  
    Copyright (C) 2006 Airgo Networks, Incorporated
  
  
   ========================================================================== */

#include "sys_api.h"



#ifdef FIXME_VOLANS

eHalStatus asicFftGetToneData(tpAniSirGlobal pMac, ePhyRxChains rxChain, tANI_U8 toneIndex, sTwoToneData *toneData)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
    tANI_U32 loToneIndex;
    tANI_U32 hiToneIndex;
    tANI_U32 loToneLoc;
    tANI_U32 hiToneLoc;
    
    assert(toneIndex < 64);
    assert(toneData != NULL);

    loToneIndex = EndianFlip( ((toneIndex + 32) % 64), 6);
    hiToneIndex = EndianFlip( (((64 - toneIndex) + 32) % 64), 6);

    loToneLoc = FFT_RAM + (loToneIndex * FFT_TONE_INDEX_MULT);
    hiToneLoc = FFT_RAM + (hiToneIndex * FFT_TONE_INDEX_MULT);
    
    
    SET_PHY_REG(pMac->hHdd, FFT_APB_ACCESS_REG, FFT_APB_ACCESS_APB_MODE_MASK);

    switch (rxChain)
    {
        case PHY_RX_CHAIN_0:
            {
                tANI_U32 toneIndexData[2];
                GET_PHY_MEMORY(pMac->hHdd, loToneLoc, (tANI_U8 *)toneIndexData, 2);
                toneData->loTone.real = (tANI_S12)(toneIndexData[0] & 0xFFF);
                toneData->loTone.imag = (tANI_S12)(((toneIndexData[0] & 0xF000) >> 12) | ((toneIndexData[1] & 0xFF) << 4));

                GET_PHY_MEMORY(pMac->hHdd, hiToneLoc, (tANI_U8 *)toneIndexData, 2);
                toneData->hiTone.real = (tANI_S12)(toneIndexData[0] & 0xFFF);
                toneData->hiTone.imag = (tANI_S12)(((toneIndexData[0] & 0xF000) >> 12) | ((toneIndexData[1] & 0xFF) << 4));
            }
            break;
        case PHY_RX_CHAIN_1:
            {
                tANI_U32 toneIndexData[2];
                GET_PHY_MEMORY(pMac->hHdd, loToneLoc + 4, (tANI_U8 *)toneIndexData, 2);
                toneData->loTone.real = (tANI_S12)(((toneIndexData[0] & 0xFF00) >> 8) | ((toneIndexData[1] & 0xF) << 8));
                toneData->loTone.imag = (tANI_S12)((toneIndexData[1] & 0xFFF0) >> 4);

                GET_PHY_MEMORY(pMac->hHdd, hiToneLoc + 4, (tANI_U8 *)toneIndexData, 2);
                toneData->hiTone.real = (tANI_S12)(((toneIndexData[0] & 0xFF00) >> 8) | ((toneIndexData[1] & 0xF) << 8));
                toneData->hiTone.imag = (tANI_S12)((toneIndexData[1] & 0xFFF0) >> 4);
            }
            break;
        case PHY_RX_CHAIN_2:
            {
                tANI_U32 toneIndexData[2];
                GET_PHY_MEMORY(pMac->hHdd, loToneLoc + 12, (tANI_U8 *)toneIndexData, 2);
                toneData->loTone.real = (tANI_S12)(toneIndexData[0] & 0xFFF);
                toneData->loTone.imag = (tANI_S12)(((toneIndexData[0] & 0xF000) >> 12) | ((toneIndexData[1] & 0xFF) << 4));

                GET_PHY_MEMORY(pMac->hHdd, hiToneLoc + 12, (tANI_U8 *)toneIndexData, 2);
                toneData->hiTone.real = (tANI_S12)(toneIndexData[0] & 0xFFF);
                toneData->hiTone.imag = (tANI_S12)(((toneIndexData[0] & 0xF000) >> 12) | ((toneIndexData[1] & 0xFF) << 4));
            }
            break;
        default:
            assert(0);
            break;
    }
    

    SET_PHY_REG(pMac->hHdd, FFT_APB_ACCESS_REG, 0);
    
    return (retVal);

}

#endif /* FIXME_VOLANS */

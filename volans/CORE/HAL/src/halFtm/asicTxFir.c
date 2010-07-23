/**
 *
   Airgo Networks, Inc proprietary.
   All Rights Reserved, Copyright 2005
   This program is the confidential and proprietary product of Airgo Networks Inc.
   Any Unauthorized use, reproduction or transfer of this program is strictly prohibited.


   asicTxFir.cc:
   Author:  Mark Nelson
   Date:    3/25/05

   History -
   Date        Modified by              Modification Information
  --------------------------------------------------------------------------

 */
#if defined(ANI_MANF_DIAG) && !defined(VERIFY_HALPHY_SIMV_MODEL)

#include <ani_assert.h>
#include <wlan_bit.h>
#include <sys_api.h>


eHalStatus asicTxFirSetTxCarrierCorrection(tpAniSirGlobal pMac, eGainSteps gain, ePhyTxChains txChain, sTxFirLoCorrect correct)
{
    eHalStatus retVal;
    tANI_U32 startWord = TXFIR_MEM + ((tANI_U32)txChain * TXFIR_MEM_PER_CHAIN) + ((tANI_U32)gain * TXFIR_MEM_GAIN_MULT);
    tANI_U32 loWord;
    tANI_U32 txIqLoCache[3];


    assert((tANI_U32)txChain < PHY_MAX_TX_CHAINS);
    assert(((tANI_U32)correct.iLo & 0xFFC0) == 0);  //expecting this value to be properly masked when it's passed
    assert(((tANI_U32)correct.qLo & 0xFFC0) == 0);  //expecting this value to be properly masked when it's passed
    assert((tANI_U32)gain < NUM_TX_GAIN_STEPS);

    loWord = ((tANI_U32)(correct.iLo & 0x3F) | ((tANI_U32)((correct.qLo & 0x3F) << 6)));
    GET_PHY_MEMORY(pMac->hHdd, startWord, (tANI_U32 *)txIqLoCache, 3);

    /* Right now, this interface is not used by any other part of the code. In case, if these needs to be used in future,
       the SET_PHY_MEMORY() should be done for the entire Txfir memory for that particular gain. If not, the write of the
       updated correction values will fail and reading back results in zero */
    switch (txChain)
    {
        case PHY_TX_CHAIN_0:
            {
                txIqLoCache[1] &= 0x000007FF;
                txIqLoCache[1] |= ((loWord & 0x0000001F) << 11);   //first 5 bits
                txIqLoCache[2] = (loWord >> 5) & 0x0000007F;      //last 7 bits

                // {
                //     tANI_U32 reg[4];
                //     GET_PHY_MEMORY(pMac->hHdd, startWord, (tANI_U8 *)&reg[0], 4);
                //     phyLog(LOGE, "TX%d gain %d BEFORE: [0]=0x%8.8X  [1]=0x%8.8X  [2]=0x%8.8X  [3]=0x%8.8X\n", txChain, gain,
                //                     reg[0], reg[1], reg[2], reg[3]
                //            );
                //
                //     phyLog(LOGE, "SET                : [0]=0x%8.8X  [1]=0x%8.8X  [2]=0x%8.8X  [3]=0x%8.8X\n",
                //                     txIqLoCache[txChain][gain][0],
                //                     txIqLoCache[txChain][gain][1],
                //                     txIqLoCache[txChain][gain][2],
                //                     txIqLoCache[txChain][gain][3]
                //            );
                // }

                SET_PHY_MEMORY(pMac->hHdd, startWord + (4 * 1), (tANI_U32 *)txIqLoCache, 2);

                // {
                //     tANI_U32 reg[4];
                //     GET_PHY_MEMORY(pMac->hHdd, startWord, (tANI_U8 *)&reg[0], 4);
                //     phyLog(LOGE, "TX%d gain %d AFTER: [0]=0x%8.8X  [1]=0x%8.8X  [2]=0x%8.8X  [3]=0x%8.8X\n", txChain, gain,
                //                     reg[0], reg[1], reg[2], reg[3]
                //            );
                // }
                //phyLog(LOGE, "asicTxFirSetTxCarrierCorrection(TX%d Gain %02d) @ %08X\n", txChain, gain, startWord + (4 * 1));
            }
            break;
        default:
            //TODO: phyLog(LOGE, "ERROR: Incorrect Tx chain");
            assert(0);
            return (eHAL_STATUS_FAILURE);
    }
    return (eHAL_STATUS_SUCCESS);
}



eHalStatus asicTxFirGetTxCarrierCorrection(tpAniSirGlobal pMac, eGainSteps gain, ePhyTxChains txChain, sTxFirLoCorrect *correct)
{
    eHalStatus retVal;
    tANI_U32 startWord = TXFIR_MEM + ((tANI_U32)txChain * TXFIR_MEM_PER_CHAIN) + ((tANI_U32)gain * TXFIR_MEM_GAIN_MULT);
    tANI_U32 txIqLoCache[3];

    assert(correct != NULL);
    assert((tANI_U32)txChain < PHY_MAX_TX_CHAINS);
    assert((tANI_U32)gain < NUM_TX_GAIN_STEPS);

    switch (txChain)
    {
        case PHY_TX_CHAIN_0:
            {
                //phyLog(LOGE, "asicTxFirGetTxCarrierCorrection(TX%d Gain %02d) @ %08X\n", txChain, gain, startWord + (4 * 1));
                GET_PHY_MEMORY(pMac->hHdd, startWord, (tANI_U8 *)txIqLoCache, 3);
                correct->iLo = (tANI_S6)(((txIqLoCache[1] & 0x0000F800) >> 11) |
                                         ((txIqLoCache[2] & 0x00000001) << 5)
                                        );
                correct->qLo = (tANI_S6)((txIqLoCache[2] & 0x0000007E) >> 1);
            }
            break;
        default:
            assert(0);
            break;
    }
    return (eHAL_STATUS_SUCCESS);
}


#define GET_CLIPPED(val) ( (val < -255) ? -255 : ( (val > 255) ? 255 : val ) )
eHalStatus asicTxFirSetTxPhaseCorrection(tpAniSirGlobal pMac, eGainSteps gain, ePhyTxChains txChain, sIQCalValues correct)
{
    eHalStatus retVal;
    tANI_U32 startWord = TXFIR_MEM + ((tANI_U32)txChain * TXFIR_MEM_PER_CHAIN) + ((tANI_U32)gain * TXFIR_MEM_GAIN_MULT);
    tANI_U32 iqWord;
    tANI_U32 txIqLoCache[3];
    
    correct.offCenter = GET_CLIPPED(correct.offCenter); 
    correct.center = GET_CLIPPED(correct.center); 
    correct.imbalance = GET_CLIPPED(correct.imbalance); 

    iqWord = ((0x1FF & correct.offCenter) | ((0x1FF & correct.center) << 9) | ((0x1FF & correct.imbalance) << 18));
    
    assert((tANI_U32)txChain < PHY_MAX_TX_CHAINS);
    assert(((tANI_U32)correct.center & 0xFE00) == 0);
    assert(((tANI_U32)correct.offCenter & 0xFE00) == 0);
    assert(((tANI_U32)correct.imbalance & 0xFE00) == 0);
    assert((tANI_U32)gain < NUM_TX_GAIN_STEPS);

    //GET_PHY_MEMORY(startWord, (tANI_U32 *)txIqLoCache, 3);
    GET_PHY_REG(pMac->hHdd, startWord, (tANI_U32 *)&txIqLoCache[0]);
    GET_PHY_REG(pMac->hHdd, startWord+4, (tANI_U32 *)&txIqLoCache[1]);
    GET_PHY_REG(pMac->hHdd, startWord+8, (tANI_U32 *)&txIqLoCache[2]);
    //TODO: phyLog(LOG3, "Chain = %d, Gain = %d, Amplitude Correction = %d, Mid Phase Correction = %d, "
    //                    "Outer Phase Correction = %d\n", txChain, gain, correct.imbalance, correct.center, correct.offCenter);

    switch (txChain)
    {
        case PHY_TX_CHAIN_0:
            {
                txIqLoCache[0] = iqWord & 0x0000FFFF;
                txIqLoCache[1] &= 0x0000F800;
                txIqLoCache[1] |= ((iqWord & 0x07FF0000) >> 16);
            }
            break;
        default:
            assert(0);
            break;
    }
    //phyLog(LOGE, "asicTxFirSetTxPhaseCorrection(TX%d Gain %02d) @ %08X\n", txChain, gain, startWord);
    
    //!!!BUG WORKAROUND: must write the last word for the previous two words to take effect
    //SET_PHY_MEMORY(startWord, (tANI_U32 *)txIqLoCache, 3);
    SET_PHY_REG(pMac->hHdd, startWord, txIqLoCache[0]);
    SET_PHY_REG(pMac->hHdd, startWord+4, txIqLoCache[1]);
    SET_PHY_REG(pMac->hHdd, startWord+8, txIqLoCache[2]);
	
    return (eHAL_STATUS_SUCCESS);
}

eHalStatus asicTxFirGetTxPhaseCorrection(tpAniSirGlobal pMac, eGainSteps gain, ePhyTxChains txChain, sIQCalValues *correct)
{
    eHalStatus retVal;
    tANI_U32 startWord = TXFIR_MEM + ((tANI_U32)txChain * TXFIR_MEM_PER_CHAIN) + ((tANI_U32)gain * TXFIR_MEM_GAIN_MULT);
    tANI_U32 txIqLoCache[2];
    
    
    assert(correct != NULL);
    assert((tANI_U32)txChain < PHY_MAX_TX_CHAINS);
    assert((tANI_U32)gain < NUM_TX_GAIN_STEPS);

    switch (txChain)
    {
        case PHY_TX_CHAIN_0:
            {
                GET_PHY_REG(pMac->hHdd, startWord, (tANI_U32 *)&txIqLoCache[0]);
                GET_PHY_REG(pMac->hHdd, startWord+4, (tANI_U32 *)&txIqLoCache[1]);

                correct->offCenter = (tANI_S9)(txIqLoCache[0] & 0x000001FF);
                correct->center =    (tANI_S9)(((txIqLoCache[0] & 0x0000FE00) >> 9) |        //7 bits
                                               ((txIqLoCache[1] & 0x00000003) << 7)          //2 bits
                                              );
                correct->imbalance = (tANI_S9)((txIqLoCache[1] & 0x000007FC) >> 2);          //9 bits

                correct->offCenter = ((tANI_S16)(correct->offCenter << 7)) >> 7;
                correct->center = ((tANI_S16)(correct->center << 7)) >> 7;
                correct->imbalance = ((tANI_S16)(correct->imbalance << 7)) >> 7;
            }
            break;
        default:
            assert(0);
            break;
    }    
    return (eHAL_STATUS_SUCCESS);
}
#endif


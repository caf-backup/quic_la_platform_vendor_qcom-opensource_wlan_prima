/**
 *
   Airgo Networks, Inc proprietary.
   All Rights Reserved, Copyright 2005
   This program is the confidential and proprietary product of Airgo Networks Inc.
   Any Unauthorized use, reproduction or transfer of this program is strictly prohibited.


   asicCal.cc: Abstracts Phy Calibration Engine
   Author:  Mark Nelson
   Date:    3/4/05

   History -
   Date        Modified by              Modification Information
  --------------------------------------------------------------------------

 */
#include <sys_api.h>


#define GET_CLIPPED(val) ( (val < -255) ? -255 : ( (val > 255) ? 255 : val ) )
eHalStatus asicWriteRxPhaseCorrection(tpAniSirGlobal pMac, eGainSteps gain, ePhyRxChains rxChain, sIQCalValues iqCorrect)
{
    eHalStatus retVal;
    tANI_U32 startWord = CAL_MEM + ((tANI_U32)gain * CAL_MEM_GAIN_MULT);
    tANI_U32 rxIqCache[4];
    tANI_U32 iqWord;


    iqCorrect.offCenter = GET_CLIPPED(iqCorrect.offCenter); 
    iqCorrect.center = GET_CLIPPED(iqCorrect.center); 
    iqCorrect.imbalance = GET_CLIPPED(iqCorrect.imbalance); 
    
    iqWord = ((0x1FF & iqCorrect.offCenter) | ((0x1FF & iqCorrect.center) << 9) | ((0x1FF & iqCorrect.imbalance) << 18));
    
    
    assert((tANI_U32)rxChain < PHY_MAX_RX_CHAINS);
    assert(((tANI_U32)iqCorrect.center & 0xFE00) == 0);
    assert(((tANI_U32)iqCorrect.offCenter & 0xFE00) == 0);
    assert(((tANI_U32)iqCorrect.imbalance & 0xFE00) == 0);
    assert(!(iqWord & 0xF8000000));
    assert(gain < NUM_RX_GAIN_STEPS);

    //GET_PHY_MEMORY(startWord, (tANI_U8 *)&rxIqCache[0], 4);
    GET_PHY_REG(pMac->hHdd, startWord, (tANI_U32 *)&rxIqCache[0]);
    GET_PHY_REG(pMac->hHdd, startWord+4, (tANI_U32 *)&rxIqCache[1]);
    GET_PHY_REG(pMac->hHdd, startWord+8, (tANI_U32 *)&rxIqCache[2]);
    GET_PHY_REG(pMac->hHdd, startWord+12, (tANI_U32 *)&rxIqCache[3]);
    //TODO: phyLog(LOG3, "Chain = %d, Gain = %d, Amplitude Correction = 0x%X, Mid Phase Correction = 0x%X, " 
    //                 "Outer Phase Correction = 0x%X\n", rxChain, gain, iqCorrect.imbalance, iqCorrect.center, iqCorrect.offCenter);

    switch (rxChain)
    {
        case PHY_RX_CHAIN_0:
            {
                rxIqCache[0] = iqWord & 0x0000FFFF;
                rxIqCache[1] &= 0x0000F800;
                rxIqCache[1] |= (iqWord >> 16);
            }
            break;
        case PHY_RX_CHAIN_1:
            {
                rxIqCache[1] &= 0x000007FF;
                rxIqCache[1] |= ((iqWord & 0x0000001F) << 11);  //first 5 bits
                rxIqCache[2] = ((iqWord >> 5) & 0x0000FFFF);    //next 16 bits
                rxIqCache[3] &= 0x0000FFC0;
                rxIqCache[3] |= ((iqWord >> (5 + 16)) & 0x0000003F);    //last 6 bits
            }
            break;
        default:
            assert(0);
            break;
    }

    
    //SET_PHY_MEMORY(pMac->hHdd, startWord, (tANI_U8 *)&rxIqCache[0], 4);
    SET_PHY_REG(pMac->hHdd, startWord, rxIqCache[0]);
    SET_PHY_REG(pMac->hHdd, startWord+4, rxIqCache[1]);
    SET_PHY_REG(pMac->hHdd, startWord+8, rxIqCache[2]);
    SET_PHY_REG(pMac->hHdd, startWord+12, rxIqCache[3]);
    
    //TODO: phyLog(LOG4, "[0] = 0x%X, [1] = 0x%X, [2] = 0x%X, [3] = 0x%X, [4] = 0x%X, [5] = 0x%X, [6] = 0x%X\n", 
    //                     rxIqCache[gain][0],
    //                     rxIqCache[gain][1],
    //                     rxIqCache[gain][2],
    //                     rxIqCache[gain][3],
    //                     rxIqCache[gain][4],
    //                     rxIqCache[gain][5],
    //                     rxIqCache[gain][6]
    //       );

    return (eHAL_STATUS_SUCCESS);
}

eHalStatus asicReadRxPhaseCorrection(tpAniSirGlobal pMac, eGainSteps gain, ePhyRxChains rxChain, sIQCalValues *iqCorrect)
{
    eHalStatus retVal;
    tANI_U32 startWord = CAL_MEM + ((tANI_U32)gain * CAL_MEM_GAIN_MULT);
    tANI_U32 dummy_value;
    tANI_U32 rxIqCache[4];
    
    
    assert((tANI_U32)rxChain < PHY_MAX_RX_CHAINS);
    assert(gain < NUM_RX_GAIN_STEPS);
    

    /* There is a bug in Taurus asic which make the first read for a particular gain in rx IQ coefficient memory to be 
    very slow and the value read is zero. To avoid that, do a dummy read first and then access the required memory 
    location so that the dummy read gets zero and further memory accesses are results in the proper values */

    //GET_PHY_MEMORY(startWord, &dummy_value, 1);
    GET_PHY_REG(pMac->hHdd, startWord, &dummy_value);
    
    //GET_PHY_MEMORY(startWord, (tANI_U8 *)&rxIqCache[0], 4);
    
    GET_PHY_REG(pMac->hHdd, startWord, (tANI_U32 *)&rxIqCache[0]);
    GET_PHY_REG(pMac->hHdd, startWord+4, (tANI_U32 *)&rxIqCache[1]);
    GET_PHY_REG(pMac->hHdd, startWord+8, (tANI_U32 *)&rxIqCache[2]);
    GET_PHY_REG(pMac->hHdd, startWord+12, (tANI_U32 *)&rxIqCache[3]);
    
    switch (rxChain)
    {
        case PHY_RX_CHAIN_0:
            {
                iqCorrect->offCenter = (tANI_S9)(rxIqCache[0] & MSK_9);
                iqCorrect->center    = (tANI_S9)(((rxIqCache[0] & (MSK_7 << 9)) >> 9) |
                                                  ((rxIqCache[1] & MSK_2) << 7)
                                                );
                iqCorrect->imbalance = (tANI_S9)((rxIqCache[1] & (MSK_9 << 2)) >> 2);
            }
            break;
        case PHY_RX_CHAIN_1:
            {
                iqCorrect->offCenter = (tANI_S9)(((rxIqCache[1] & (MSK_5 << 11)) >> 11) |
                                                  ((rxIqCache[2] & MSK_4) << 5)
                                                );
                iqCorrect->center    = (tANI_S9)((rxIqCache[2] & (MSK_9 << 4)) >> 4);
                iqCorrect->imbalance = (tANI_S9)(((rxIqCache[2] & (MSK_3 << 13)) >> 13) |
                                                  ((rxIqCache[3] & MSK_6) << 3)
                                                );
            }
            break;
        default:
            //TODO: phyLog(LOGE, "ERROR: Incorrect Rx chain");
            assert(0);
            return (eHAL_STATUS_FAILURE);
    }
    iqCorrect->offCenter = ((tANI_S16)(iqCorrect->offCenter << 7)) >> 7;
    iqCorrect->center = ((tANI_S16)(iqCorrect->center << 7)) >> 7;
    iqCorrect->imbalance = ((tANI_S16)(iqCorrect->imbalance << 7)) >> 7;

    
    //TODO: phyLog(LOG4, "[0] = 0x%X, [1] = 0x%X, [2] = 0x%X, [3] = 0x%X, [4] = 0x%X, [5] = 0x%X, [6] = 0x%X\n", 
    //                     rxIqCache[gain][0],
    //                     rxIqCache[gain][1],
    //                     rxIqCache[gain][2],
    //                     rxIqCache[gain][3],
    //                     rxIqCache[gain][4],
    //                     rxIqCache[gain][5],
    //                     rxIqCache[gain][6]
    //       );
    
    //TODO: phyLog(LOG3, "Chain = %d, Gain = %d, Amplitude Correction = 0x%X, Mid Phase Correction = 0x%X, " 
    //                "Outer Phase Correction = 0x%X\n", rxChain, gain, iqCorrect->imbalance, iqCorrect->center, iqCorrect->offCenter);
    
    return (eHAL_STATUS_SUCCESS);
}






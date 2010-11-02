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
#ifndef VERIFY_HALPHY_SIMV_MODEL

#ifndef WLAN_FTM_STUB

#define GET_CLIPPED(val) ( (val < -255) ? -255 : ( (val > 255) ? 255 : val ) )
eHalStatus asicWriteRxPhaseCorrection(tpAniSirGlobal pMac, eGainSteps gain, ePhyRxChains rxChain, sIQCalValues iqCorrect)
{
#ifdef VOLANS_RF
	tANI_U32 startWord = CAL_MEM + ((tANI_U32)gain * CAL_MEM_GAIN_MULT);
	tANI_U32 rxIqCache[2];
	tANI_U32 iqWord;
    eHalStatus retVal;

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
	//TODO: phyLog(LOG3, "Chain = %d, Gain = %d, Amplitude Correction = 0x%X, Mid Phase Correction = 0x%X, " 
	//				   "Outer Phase Correction = 0x%X\n", rxChain, gain, iqCorrect.imbalance, iqCorrect.center, iqCorrect.offCenter);

	switch (rxChain)
	{
		case PHY_RX_CHAIN_0:
		default:
			{
				rxIqCache[0] = iqWord & 0x0000FFFF;
				rxIqCache[1] &= 0x0000F800;
				rxIqCache[1] |= (iqWord >> 16);
			}
			break;
	}

	
	//SET_PHY_MEMORY(startWord, (tANI_U8 *)&rxIqCache[0], 4);
	SET_PHY_REG(pMac->hHdd, startWord, rxIqCache[0]);
	SET_PHY_REG(pMac->hHdd, startWord+4, rxIqCache[1]);
#endif
    return (eHAL_STATUS_SUCCESS);
}

eHalStatus asicReadRxPhaseCorrection(tpAniSirGlobal pMac, eGainSteps gain, ePhyRxChains rxChain, sIQCalValues *iqCorrect)
{
#ifdef VOLANS_RF
	 tANI_U32 startWord = CAL_MEM + ((tANI_U32)gain * CAL_MEM_GAIN_MULT);
	 tANI_U32 rxIqCache[2];
     eHalStatus retVal;
	 
	 assert((tANI_U32)rxChain < PHY_MAX_RX_CHAINS);
	 assert(gain < NUM_RX_GAIN_STEPS);
 
	 GET_PHY_REG(pMac->hHdd, startWord, (tANI_U32 *)&rxIqCache[0]);
	 GET_PHY_REG(pMac->hHdd, startWord+4, (tANI_U32 *)&rxIqCache[1]);
	 
	 switch (rxChain)
	 {
		 case PHY_RX_CHAIN_0:
		 default:
			 {
				 iqCorrect->offCenter = (tANI_S9)(rxIqCache[0] & MSK_9);
				 iqCorrect->center	  = (tANI_S9)(((rxIqCache[0] & (MSK_7 << 9)) >> 9) |
												   ((rxIqCache[1] & MSK_2) << 7)
												 );
				 iqCorrect->imbalance = (tANI_S9)((rxIqCache[1] & (MSK_9 << 2)) >> 2);
			 }
			 break;
	 }
	 iqCorrect->offCenter = ((tANI_S16)(iqCorrect->offCenter << 7)) >> 7;
	 iqCorrect->center = ((tANI_S16)(iqCorrect->center << 7)) >> 7;
	 iqCorrect->imbalance = ((tANI_S16)(iqCorrect->imbalance << 7)) >> 7;
#endif
    return (eHAL_STATUS_SUCCESS);	
}
#endif //ifndef WLAN_FTM_STUB
#endif //ifndef VERIFY_HALPHY_SIMV_MODEL




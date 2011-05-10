/** ------------------------------------------------------------------------- *
    ------------------------------------------------------------------------- *


    \file halQFuse.c

    \brief qfuse facilities

    $Id$

    Copyright (C) 2008 Qualcomm, Incorporated


   ========================================================================== */

#include "aniGlobal.h"
#include "ani_assert.h"
#include "halDebug.h"
#include "halRfTypes.h"

extern const sHalNv nvDefaults;


enum
{
    QFUSE_NOT_BLOWN = 0,
    QFUSE_BLOWN = 1
};

eHalStatus palWaitRegVal( tHddHandle hHdd, tANI_U32 reg, tANI_U32 mask,
                             tANI_U32 waitRegVal, tANI_U32 perIterWaitInNanoSec,
                             tANI_U32 numIter, tANI_U32 *pReadRegVal )
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hHdd;
    eHalStatus  nStatus = eHAL_STATUS_SUCCESS;
    //tANI_U32    waitTime = (perIterWaitInNanoSec + 999) / 1000;

    do
    {
        nStatus = palReadRegister(pMac->hHdd, reg, pReadRegVal);
        if (nStatus != eHAL_STATUS_SUCCESS)
        {
            break;
        }

        if ((*pReadRegVal & mask) == waitRegVal)
        {
            break;
        }

        if (--numIter)
        {
            sirBusyWait(1000);  //wait 1 microsecond
        }
        else
        {
            nStatus = eHAL_STATUS_FAILURE;
            break;
        }

    } while (1);

    return nStatus;
}

void halQFusePackBits(tHalHandle hMac)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hMac;

    assert(pMac->hphy.nvCache.tables.tpcConfig[0].freq == START_TPC_CHANNEL);
    assert(pMac->hphy.nvCache.tables.tpcConfig[1].freq == END_TPC_CHANNEL);

    pMac->hphy.nvCache.tables.qFuseData.dword0_chan1_calpoint_0_pwrDetAdc       = pMac->hphy.nvCache.tables.tpcConfig[0].empirical[0][0].pwrDetAdc;
    pMac->hphy.nvCache.tables.qFuseData.dword0_chan1_calpoint_0_adjustedPwrDet  = pMac->hphy.nvCache.tables.tpcConfig[0].empirical[0][0].adjustedPwrDet;
    pMac->hphy.nvCache.tables.qFuseData.dword0_chan1_calpoint_1_pwrDetAdc       = pMac->hphy.nvCache.tables.tpcConfig[0].empirical[0][1].pwrDetAdc;
    pMac->hphy.nvCache.tables.qFuseData.dword0_chan1_calpoint_1_adjustedPwrDet  = pMac->hphy.nvCache.tables.tpcConfig[0].empirical[0][1].adjustedPwrDet;
    pMac->hphy.nvCache.tables.qFuseData.dword0_chan1_calpoint_2_adjustedPwrDet  = pMac->hphy.nvCache.tables.tpcConfig[0].empirical[0][2].adjustedPwrDet & 0xF;

    pMac->hphy.nvCache.tables.qFuseData.dword1_chan1_calpoint_2_pwrDetAdc       = pMac->hphy.nvCache.tables.tpcConfig[0].empirical[0][2].pwrDetAdc;
    pMac->hphy.nvCache.tables.qFuseData.dword1_chan1_calpoint_2_adjustedPwrDet  = pMac->hphy.nvCache.tables.tpcConfig[0].empirical[0][2].adjustedPwrDet >> 4;
    pMac->hphy.nvCache.tables.qFuseData.dword1_chan1_calpoint_3_pwrDetAdc       = pMac->hphy.nvCache.tables.tpcConfig[0].empirical[0][3].pwrDetAdc;
    pMac->hphy.nvCache.tables.qFuseData.dword1_chan1_calpoint_3_adjustedPwrDet  = pMac->hphy.nvCache.tables.tpcConfig[0].empirical[0][3].adjustedPwrDet;
    pMac->hphy.nvCache.tables.qFuseData.dword1_chan13_calpoint_0_pwrDetAdc      = pMac->hphy.nvCache.tables.tpcConfig[1].empirical[0][0].pwrDetAdc & 0x1;
    pMac->hphy.nvCache.tables.qFuseData.dword1_chan13_calpoint_0_adjustedPwrDet = pMac->hphy.nvCache.tables.tpcConfig[1].empirical[0][0].adjustedPwrDet;

    pMac->hphy.nvCache.tables.qFuseData.dword2_chan13_calpoint_0_pwrDetAdc      = pMac->hphy.nvCache.tables.tpcConfig[1].empirical[0][0].pwrDetAdc >> 1;
    pMac->hphy.nvCache.tables.qFuseData.dword2_chan13_calpoint_1_pwrDetAdc      = pMac->hphy.nvCache.tables.tpcConfig[1].empirical[0][1].pwrDetAdc;
    pMac->hphy.nvCache.tables.qFuseData.dword2_chan13_calpoint_1_adjustedPwrDet = pMac->hphy.nvCache.tables.tpcConfig[1].empirical[0][1].adjustedPwrDet;
    pMac->hphy.nvCache.tables.qFuseData.dword2_chan13_calpoint_2_pwrDetAdc      = pMac->hphy.nvCache.tables.tpcConfig[1].empirical[0][2].pwrDetAdc & 0x1F;
    pMac->hphy.nvCache.tables.qFuseData.dword2_chan13_calpoint_2_adjustedPwrDet = pMac->hphy.nvCache.tables.tpcConfig[1].empirical[0][2].adjustedPwrDet;
    pMac->hphy.nvCache.tables.qFuseData.dword3_chan13_calpoint_2_pwrDetAdc      = pMac->hphy.nvCache.tables.tpcConfig[1].empirical[0][2].pwrDetAdc >> 5;

    pMac->hphy.nvCache.tables.qFuseData.dword3_chan13_calpoint_3_pwrDetAdc      = pMac->hphy.nvCache.tables.tpcConfig[1].empirical[0][3].pwrDetAdc;
    pMac->hphy.nvCache.tables.qFuseData.dword3_chan13_calpoint_3_adjustedPwrDet = pMac->hphy.nvCache.tables.tpcConfig[1].empirical[0][3].adjustedPwrDet;

    pMac->hphy.nvCache.tables.qFuseData.dword3_rf_hdet_ctl_ext_atten            = pMac->hphy.nvCache.tables.rfCalValues.hdet_ctl_ext_atten;
    pMac->hphy.nvCache.tables.qFuseData.dword3_rf_hdet_dcoc_code                = pMac->hphy.nvCache.tables.rfCalValues.hdet_dcoc_code;
    pMac->hphy.nvCache.tables.qFuseData.dword3_rf_hdet_dcoc_ib_rcal_en          = pMac->hphy.nvCache.tables.rfCalValues.hdet_dcoc_ib_rcal_en;
    pMac->hphy.nvCache.tables.qFuseData.dword3_rf_hdet_dcoc_ib_scal_en          = pMac->hphy.nvCache.tables.rfCalValues.hdet_dcoc_ib_scal_en;
}

/*
    If the part has a clipped, PADC value for channel 9, we use information from other frequency channels to extrapolate a
    corrected value for the clipped location.
    i.e., normalize the slope between the calpoints 3 and 4 for channel 9 based on the channel 3's information.

    Target = 127
    slope_3 = (padc_channel3_13p5dBm- padc_channel3_10dBm) / (pout_channel3_13p5dBm -pout_channel3_10dBm)
    slope_9 = (padc_channel9_13p5dBm- padc_channel9_10dBm) / (pout_channel9_13p5dBm -pout_channel9_10dBm)
    slope_3_top = (padc_channel3_16dBm- padc_channel3_13p5dBm) / (pout_channel3_16dBm- pout_channel3_13p5dBm)
    slope_9_top = (padc_channel9_16dBm- padc_channel9_13p5dBm) / (pout_channel9_16dBm- pout_channel9_13p5dBm)
    new_padc = (slope_3_top.*slope_9./slope_3).*( pout_channel9_16dBm - pout_channel9_13p5dBm)+ padc_channel9_13p5dBm;
    new_slope_9 = (pout_channel9_16dm-pout_channel9_13p5dBm)./(new_padc_9 – padc_channel9_13p5dBm);
    pout_channel9_16dBm = pout_channel9_16dBm - new_slope_9.*(new_padc_9-target_point); %replacing old pout value
    padc_channel9_16dBm = Target; %replacing old padc value

*/
static void halQFuseCorrectPadcClippedValue(tHalHandle hMac)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hMac;

    tANI_U8 target = 127;
    tANI_U8 padc00, pout00, padc01, pout01, padc02, pout02, padc03, pout03, padc10, pout10, padc11, pout11, padc12, pout12, padc13, pout13;

    padc00 = pMac->hphy.nvCache.tables.tpcConfig[0].empirical[0][0].pwrDetAdc      ;
    pout00 = pMac->hphy.nvCache.tables.tpcConfig[0].empirical[0][0].adjustedPwrDet ;
    padc01 = pMac->hphy.nvCache.tables.tpcConfig[0].empirical[0][1].pwrDetAdc      ;
    pout01 = pMac->hphy.nvCache.tables.tpcConfig[0].empirical[0][1].adjustedPwrDet ;
    padc02 = pMac->hphy.nvCache.tables.tpcConfig[0].empirical[0][2].pwrDetAdc      ;
    pout02 = pMac->hphy.nvCache.tables.tpcConfig[0].empirical[0][2].adjustedPwrDet ;
    padc03 = pMac->hphy.nvCache.tables.tpcConfig[0].empirical[0][3].pwrDetAdc      ;
    pout03 = pMac->hphy.nvCache.tables.tpcConfig[0].empirical[0][3].adjustedPwrDet ;
    padc10 = pMac->hphy.nvCache.tables.tpcConfig[1].empirical[0][0].pwrDetAdc      ;
    pout10 = pMac->hphy.nvCache.tables.tpcConfig[1].empirical[0][0].adjustedPwrDet ;
    padc11 = pMac->hphy.nvCache.tables.tpcConfig[1].empirical[0][1].pwrDetAdc      ;
    pout11 = pMac->hphy.nvCache.tables.tpcConfig[1].empirical[0][1].adjustedPwrDet ;
    padc12 = pMac->hphy.nvCache.tables.tpcConfig[1].empirical[0][2].pwrDetAdc      ;
    pout12 = pMac->hphy.nvCache.tables.tpcConfig[1].empirical[0][2].adjustedPwrDet ;
    padc13 = pMac->hphy.nvCache.tables.tpcConfig[1].empirical[0][3].pwrDetAdc      ;
    pout13 = pMac->hphy.nvCache.tables.tpcConfig[1].empirical[0][3].adjustedPwrDet ;

    //slope_3 = (padc02- padc01) / (pout02 -pout01);
    //slope_9 = (padc12- padc11) / (pout12 -pout11);
    //slope_3_top = (padc03- padc02) / (pout03- pout02);
    //slope_9_top = (padc13- padc12) / (pout13- pout12);

    //check whether the part has a clipped padc value or not.
	if (((padc03 == 127) && (padc13 == 127)) || ((padc03 <= padc02) && (padc13 <= padc12)))
	{
		//fail. Monotonicity breach. operate in open loop
		pMac->hphy.phy.test.sysInOpenLoopMode = eANI_BOOLEAN_TRUE;

        //Operate with open loop gain of 0xaf
        palWriteRegister(pMac->hHdd, QWLAN_TPC_GAIN_LUT0_MREG, 0xaf);
	}
	else if ((padc03 == 127 ) || (padc03 <= padc02))
	{
		//use the top slope from channel 9 on channel 3
		//newSlope = slope_9_top * slope_3/slope_9;
		pMac->hphy.nvCache.tables.tpcConfig[0].empirical[0][3].adjustedPwrDet =
						pout02 + (((target - padc02) * (pout13- pout12) * (pout02 -pout01) * (padc12- padc11))/
							((padc13- padc12) * (padc02- padc01) * (pout12 -pout11)));

		pMac->hphy.nvCache.tables.tpcConfig[0].empirical[0][3].pwrDetAdc = target; //%replacing old padc value

	}
	else if ((padc13 == 127 ) || ( padc13 <= padc12 ))
	{
		//use the top slope from channel 3 on channel 9
		//newSlope = slope_3_top * slope_9/slope_3;
        pMac->hphy.nvCache.tables.tpcConfig[1].empirical[0][3].adjustedPwrDet =
						pout12 + (((target - padc12) * (pout03- pout02) * (pout12 -pout11) * (padc02- padc01)) /
							((padc03- padc02) * (padc12- padc11) * (pout02 -pout01)));

        pMac->hphy.nvCache.tables.tpcConfig[1].empirical[0][3].pwrDetAdc = target; //%replacing old padc value
    }
}

static void halQFuseParseBits(tHalHandle hMac)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hMac;

    if (halIsQFuseBlown(hMac) == eHAL_STATUS_SUCCESS)
    {
        pMac->hphy.nvCache.tables.tpcConfig[0].freq = START_TPC_CHANNEL;
        //pMac->hphy.nvCache.tables.tpcConfig[0].absPower.min;
        //pMac->hphy.nvCache.tables.tpcConfig[0].absPower.max;
        pMac->hphy.nvCache.tables.tpcConfig[0].empirical[0][0].pwrDetAdc      = (tPowerDetect)pMac->hphy.nvCache.tables.qFuseData.dword0_chan1_calpoint_0_pwrDetAdc;
        pMac->hphy.nvCache.tables.tpcConfig[0].empirical[0][0].adjustedPwrDet = (tPowerDetect)pMac->hphy.nvCache.tables.qFuseData.dword0_chan1_calpoint_0_adjustedPwrDet;
        pMac->hphy.nvCache.tables.tpcConfig[0].empirical[0][1].pwrDetAdc      = (tPowerDetect)pMac->hphy.nvCache.tables.qFuseData.dword0_chan1_calpoint_1_pwrDetAdc;
        pMac->hphy.nvCache.tables.tpcConfig[0].empirical[0][1].adjustedPwrDet = (tPowerDetect)pMac->hphy.nvCache.tables.qFuseData.dword0_chan1_calpoint_1_adjustedPwrDet;
        pMac->hphy.nvCache.tables.tpcConfig[0].empirical[0][2].pwrDetAdc      = (tPowerDetect)pMac->hphy.nvCache.tables.qFuseData.dword1_chan1_calpoint_2_pwrDetAdc;
        pMac->hphy.nvCache.tables.tpcConfig[0].empirical[0][2].adjustedPwrDet = (tPowerDetect)((pMac->hphy.nvCache.tables.qFuseData.dword1_chan1_calpoint_2_adjustedPwrDet << 4)
                                                                                                  | pMac->hphy.nvCache.tables.qFuseData.dword0_chan1_calpoint_2_adjustedPwrDet);
        pMac->hphy.nvCache.tables.tpcConfig[0].empirical[0][3].pwrDetAdc      = (tPowerDetect)pMac->hphy.nvCache.tables.qFuseData.dword1_chan1_calpoint_3_pwrDetAdc;
        pMac->hphy.nvCache.tables.tpcConfig[0].empirical[0][3].adjustedPwrDet = (tPowerDetect)pMac->hphy.nvCache.tables.qFuseData.dword1_chan1_calpoint_3_adjustedPwrDet;

        pMac->hphy.nvCache.tables.tpcConfig[1].freq = END_TPC_CHANNEL;
        pMac->hphy.nvCache.tables.tpcConfig[1].empirical[0][0].pwrDetAdc      = (tPowerDetect)((pMac->hphy.nvCache.tables.qFuseData.dword2_chan13_calpoint_0_pwrDetAdc << 1)
                                                                                                  | pMac->hphy.nvCache.tables.qFuseData.dword1_chan13_calpoint_0_pwrDetAdc);
        pMac->hphy.nvCache.tables.tpcConfig[1].empirical[0][0].adjustedPwrDet = (tPowerDetect)pMac->hphy.nvCache.tables.qFuseData.dword1_chan13_calpoint_0_adjustedPwrDet;
        pMac->hphy.nvCache.tables.tpcConfig[1].empirical[0][1].pwrDetAdc      = (tPowerDetect)pMac->hphy.nvCache.tables.qFuseData.dword2_chan13_calpoint_1_pwrDetAdc;
        pMac->hphy.nvCache.tables.tpcConfig[1].empirical[0][1].adjustedPwrDet = (tPowerDetect)pMac->hphy.nvCache.tables.qFuseData.dword2_chan13_calpoint_1_adjustedPwrDet;
        pMac->hphy.nvCache.tables.tpcConfig[1].empirical[0][2].pwrDetAdc      = (tPowerDetect)((pMac->hphy.nvCache.tables.qFuseData.dword3_chan13_calpoint_2_pwrDetAdc << 5)
                                                                                                  | pMac->hphy.nvCache.tables.qFuseData.dword2_chan13_calpoint_2_pwrDetAdc);
        pMac->hphy.nvCache.tables.tpcConfig[1].empirical[0][2].adjustedPwrDet = (tPowerDetect)pMac->hphy.nvCache.tables.qFuseData.dword2_chan13_calpoint_2_adjustedPwrDet;
        pMac->hphy.nvCache.tables.tpcConfig[1].empirical[0][3].pwrDetAdc      = (tPowerDetect)pMac->hphy.nvCache.tables.qFuseData.dword3_chan13_calpoint_3_pwrDetAdc;
        pMac->hphy.nvCache.tables.tpcConfig[1].empirical[0][3].adjustedPwrDet = (tPowerDetect)pMac->hphy.nvCache.tables.qFuseData.dword3_chan13_calpoint_3_adjustedPwrDet;

        //pMac->hphy.pwrParams.numTpcCalPointsPerFreq = MAX_TPC_CAL_POINTS;
        //pMac->hphy.pwrParams.numTpcCalFreqs = MAX_TPC_CHANNELS;

        pMac->hphy.nvCache.tables.rfCalValues.hdet_ctl_ext_atten              = (tANI_U8)pMac->hphy.nvCache.tables.qFuseData.dword3_rf_hdet_ctl_ext_atten  ;
        pMac->hphy.nvCache.tables.rfCalValues.hdet_dcoc_code                  = (tANI_U8)pMac->hphy.nvCache.tables.qFuseData.dword3_rf_hdet_dcoc_code      ;
        pMac->hphy.nvCache.tables.rfCalValues.hdet_dcoc_ib_rcal_en            = (tANI_U8)pMac->hphy.nvCache.tables.qFuseData.dword3_rf_hdet_dcoc_ib_rcal_en;
        pMac->hphy.nvCache.tables.rfCalValues.hdet_dcoc_ib_scal_en            = (tANI_U8)pMac->hphy.nvCache.tables.qFuseData.dword3_rf_hdet_dcoc_ib_scal_en;

        /*
          There was an issue with the calibration procedure on the ATE. Due to this problem,
          parts can be released with clipped PADC values in Qfuse. If the part has a clipped
          PADC value, we use information from other frequency channels to extrapolate a corrected
          value for the clipped location.
        */
        halQFuseCorrectPadcClippedValue(hMac);
    }
    else
    {
        //QFuse data doesn't exist - populate with default data
        memcpy(&pMac->hphy.nvCache.tables.tpcConfig[0], &nvDefaults.tables.tpcConfig[0], sizeof(tTpcConfig) * MAX_TPC_CHANNELS);
        memcpy(&pMac->hphy.nvCache.tables.rfCalValues, &nvDefaults.tables.rfCalValues, sizeof(sRfNvCalValues));
    }
}

static eHalStatus halQFuseSwBlow(tHalHandle hMac, tANI_U8 bit, tANI_U8 shiftPosition)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hMac;
    //eHalStatus retVal = eHAL_STATUS_FAILURE;
    tANI_U32 regVal;

    if(bit)
    {
        //ensures that the chain is all-zero prior to shift in the bit pattern to program
        palWriteRegister(pMac->hHdd, QWLAN_SCU_QFUSE_EF_SHIFT_VALUE_REG,SCU_QFUSE_EF_SHIFT_VALUE_CLEAR);
        palWriteRegister(pMac->hHdd, QWLAN_SCU_QFUSE_EF_BLOW_VALUE_REG, SCU_QFUSE_EF_BLOW_VALUE_CLEAR);
        palWriteRegister(pMac->hHdd, QWLAN_SCU_QFUSE_EF_PWR_CTL_REG, 1);
        palWriteRegister(pMac->hHdd, QWLAN_SCU_QFUSE_EF_STATUS_REG, SCU_QFUSE_EF_STATUS);

        if(palWaitRegVal(pMac->hHdd, QWLAN_SCU_QFUSE_EF_STATUS_REG, (QWLAN_SCU_QFUSE_EF_STATUS_STATUS_START_MASK |
                                                                QWLAN_SCU_QFUSE_EF_STATUS_STATUS_DONE_MASK),
                                                                3, 1000, 5000, &regVal) == eHAL_STATUS_SUCCESS)
        {
            palWriteRegister(pMac->hHdd, QWLAN_SCU_QFUSE_EF_STATUS_REG, 0);
            palWriteRegister(pMac->hHdd, QWLAN_SCU_QFUSE_EF_PWR_CTL_REG, 1);
        }
        else
        {
            return eHAL_STATUS_FAILURE;
        }

        //actual programming
        palWriteRegister(pMac->hHdd, QWLAN_SCU_QFUSE_EF_SHIFT_VALUE_REG, shiftPosition);
        palWriteRegister(pMac->hHdd, QWLAN_SCU_QFUSE_EF_BLOW_VALUE_REG, SCU_QFUSE_EF_BLOW_VALUE);
        palWriteRegister(pMac->hHdd, QWLAN_SCU_QFUSE_EF_PWR_CTL_REG, 0);
        palWriteRegister(pMac->hHdd, QWLAN_SCU_QFUSE_EF_STATUS_REG, SCU_QFUSE_EF_STATUS);

        if(palWaitRegVal(pMac->hHdd, QWLAN_SCU_QFUSE_EF_STATUS_REG, (QWLAN_SCU_QFUSE_EF_STATUS_STATUS_START_MASK |
                                                                QWLAN_SCU_QFUSE_EF_STATUS_STATUS_DONE_MASK),
                                                                3, 1000, 5000, &regVal) == eHAL_STATUS_SUCCESS)
        {
            palWriteRegister(pMac->hHdd, QWLAN_SCU_QFUSE_EF_STATUS_REG, 0);
            palWriteRegister(pMac->hHdd, QWLAN_SCU_QFUSE_EF_PWR_CTL_REG, 1);
        }
        else
        {
            return eHAL_STATUS_FAILURE;
        }
    }

    return eHAL_STATUS_SUCCESS;
}

eHalStatus halQFuseWrite(tHalHandle hMac)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hMac;
    tANI_U32 *pattern;
    tANI_U32 wordCount, bitCount;

    if (memcmp(&pMac->hphy.nvCache.tables.tpcConfig[0], &nvDefaults.tables.tpcConfig[0], sizeof(tTpcConfig) * MAX_TPC_CHANNELS) == 0)
    {
        //don't burn default data
        HALLOGE(halLog(pMac, LOGE, "ERROR: Can't burn default data to qfuse as this would be irreversible"));
        return (eHAL_STATUS_FAILURE);
    }

    halQFusePackBits(hMac);

    pMac->hphy.nvCache.tables.qFuseData.dword3_sw_fuse_pgm_dsbl = QFUSE_BLOWN;    //this is cleared if we successfully burn the qfuse

    pattern  = (tANI_U32 *)&pMac->hphy.nvCache.tables.qFuseData;

    /* init config*/
    palWriteRegister(pMac->hHdd, QWLAN_SCU_QFUSE_EF_PWR_CTL_REG, 1);
    palWriteRegister(pMac->hHdd, QWLAN_SCU_QFUSE_EF_CHAIN_SEL_REG, QWLAN_SCU_QFUSE_EF_CHAIN_SEL_CONFIG_CHAIN_SEL_MASK);
    palWriteRegister(pMac->hHdd, QWLAN_SCU_QFUSE_EF_BLOW_TIMER_REG, SCU_QFUSE_EF_BLOW_TIMER);

    for(wordCount = 0; wordCount < (sizeof(sQFuseConfig) >> 2); wordCount++)
    {
        for(bitCount = 0; bitCount < NUM_DWORD_BINARY_BITS; bitCount++)
        {
            tANI_U8 bit, shiftPosition;
            bit = ((pattern[wordCount] >> bitCount) & 1) ? 1 : 0;
            //this writes the the word in this way: wordLoc in qFuse is [wordIndex in qFuse Data]
            //shiftPosition = (tANI_U8)((wordCount * NUM_DWORD_BINARY_BITS) + (NUM_DWORD_BINARY_BITS - bitCount));

            //this writes the the word in this way: wordLoc in qFuse is [maxAddrLoc - wordIndex in qFuse Data]
            //we want word0 to be written at [127:96] and word3 at [31:0]
            shiftPosition = (tANI_U8)((wordCount * NUM_DWORD_BINARY_BITS) + (NUM_DWORD_BINARY_BITS - bitCount));
            if(halQFuseSwBlow(hMac, bit, shiftPosition) != eHAL_STATUS_SUCCESS)
            {
                return eHAL_STATUS_FAILURE;
            }
        }
    }

    return eHAL_STATUS_SUCCESS;
}

eHalStatus halQFuseRead(tHalHandle hMac)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tpAniSirGlobal pMac = (tpAniSirGlobal)hMac;
    tANI_U32 *dWord = (tANI_U32 *)&pMac->hphy.nvCache.tables.qFuseData;

    palReadRegister(pMac->hHdd, QWLAN_SCU_QFUSE_EF_CONFIG_BYTE_3_REG, &dWord[0]);
    palReadRegister(pMac->hHdd, QWLAN_SCU_QFUSE_EF_CONFIG_BYTE_2_REG, &dWord[1]);
    palReadRegister(pMac->hHdd, QWLAN_SCU_QFUSE_EF_CONFIG_BYTE_1_REG, &dWord[2]);
    palReadRegister(pMac->hHdd, QWLAN_SCU_QFUSE_EF_CONFIG_BYTE_0_REG, &dWord[3]);

    halQFuseParseBits(hMac);

    return status;
}

eHalStatus halIsQFuseBlown(tHalHandle hMac)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hMac;
    tANI_U32 dWord[4];

    palReadRegister(pMac->hHdd, QWLAN_SCU_QFUSE_EF_CONFIG_BYTE_3_REG, &dWord[0]);
    palReadRegister(pMac->hHdd, QWLAN_SCU_QFUSE_EF_CONFIG_BYTE_2_REG, &dWord[1]);
    palReadRegister(pMac->hHdd, QWLAN_SCU_QFUSE_EF_CONFIG_BYTE_1_REG, &dWord[2]);
    palReadRegister(pMac->hHdd, QWLAN_SCU_QFUSE_EF_CONFIG_BYTE_0_REG, &dWord[3]);

    if((dWord[0] == 0) && (dWord[1] == 0) && (dWord[2] == 0) && (dWord[3] == 0))
    {
        return (eHAL_STATUS_FAILURE);     //no, unblown
    }
    else
    {
        return (eHAL_STATUS_SUCCESS);     //yes, its blown
    }
}

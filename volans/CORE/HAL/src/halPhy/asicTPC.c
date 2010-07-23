/**
 *
   Airgo Networks, Inc proprietary.
   All Rights Reserved, Copyright 2005
   This program is the confidential and proprietary product of Airgo Networks Inc.
   Any Unauthorized use, reproduction or transfer of this program is strictly prohibited.


   asicTPC.cc: Abstracts Transmit Power Control registers
   Author:  Mark Nelson
   Date:    3/25/05

   History -
   Date        Modified by              Modification Information
  --------------------------------------------------------------------------

 */
#include "ani_assert.h"
#include "sys_api.h"

eHalStatus asicTPCPowerOverride(tpAniSirGlobal pMac, tTxGain tx0, tTxGain tx1, tTxGain tx2, tTxGain tx3)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
#if 0
    tANI_U32 bkup0, bkup1, bkup2;

    // Debugged tx gain override -
    //     Prior to overriding the gains through the baseband registers:
    //     Need to have the TPC, TxCtl, and TxFir dynamic clock gating disabled
    //     Need to manually force tx RF enables
    //     Need to have the ant_en and select_firmode bits set in the txctl.fir_mode register


    //backup registers to restore after overriding the gain

    GET_PHY_REG(pMac->hHdd, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, &bkup0);
    GET_PHY_REG(pMac->hHdd, QWLAN_TXCTL_DAC_CONTROL_REG, &bkup1);
    GET_PHY_REG(pMac->hHdd, TXCTL_FIR_TX_MODE_REG, &bkup2);

    SET_PHY_REG(pMac->hHdd, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG,
                                bkup0 |
                                QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TXCTL_MASK |
                                QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TXFIR_MASK |
                                QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TPC_MASK
               );
    SET_PHY_REG(pMac->hHdd, QWLAN_TXCTL_DAC_CONTROL_REG,
                                TXCTL_DAC_OVERRIDE_EN_MASK |
                                TXCTL_RF_OVERRIDE_EN_MASK |
                                (TXCTL_DAC_TX0 | TXCTL_DAC_TX1 | TXCTL_DAC_TX2 | TXCTL_DAC_TX3) |
                                TXCTL_DAC_RF_EN_MASK
               );
    SET_PHY_REG(pMac->hHdd, TXCTL_FIR_TX_MODE_REG,
                                (TXCTL_FIR_TX_MODE_SELECT_MASK |
                                 ((TXCTL_DAC_TX0 | TXCTL_DAC_TX1 | TXCTL_DAC_TX2 | TXCTL_DAC_TX3) << QWLAN_TXCTL_FIR_MODE_ANT_EN_OFFSET)
                                )
               );



    SET_PHY_REG(pMac->hHdd, QWLAN_TPC_TXPWR_ENABLE_REG, 0);   //turn off TPC closed loop control

#ifdef ANI_PHY_DEBUG
    {
        //since we are overriding the gain, enable cannot be true
        tANI_U32 val;
        GET_PHY_REG(pMac->hHdd, QWLAN_TPC_TXPWR_ENABLE_REG, &val);
        assert((val & TPC_TXPWR_ENABLE_MASK) == 0);
    }
#endif

    //first, write the gains that we want to send to the RF and digital portion
    SET_PHY_REG(pMac->hHdd, QWLAN_TPC_TXPWR_OVERRIDE0_REG,
                    (QWLAN_TPC_TXPWR_OVERRIDE0_RF_POWER_MASK & (tx0.coarsePwr << QWLAN_TPC_TXPWR_OVERRIDE0_RF_POWER_OFFSET)) |
                    (QWLAN_TPC_TXPWR_OVERRIDE0_FINE_POWER_MASK & (tx0.finePwr << QWLAN_TPC_TXPWR_OVERRIDE0_FINE_POWER_OFFSET))
               );


    SET_PHY_REG(pMac->hHdd, QWLAN_TPC_TXPWR_ENABLE_REG, QWLAN_TPC_TXPWR_ENABLE_OVERRIDE_MASK);

    //debug registers
    // DUMP_PHY_REG(TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG);
    // DUMP_PHY_REG(TXCTL_DAC_CONTROL_REG);
    // DUMP_PHY_REG(TXCTL_FIR_TX_MODE_REG);
    // DUMP_PHY_REG(TPC_TXPWR_OVERRIDE0_REG);
    // DUMP_PHY_REG(TPC_TXPWR_OVERRIDE1_REG);
    // DUMP_PHY_REG(TPC_TXPWR_OVERRIDE2_REG);
    // DUMP_PHY_REG(TPC_TXPWR_OVERRIDE3_REG);
    // DUMP_PHY_REG(TPC_TXPWR_ENABLE_REG);
    //
    // DUMP_RF_FIELD(QUASAR_FIELD_TX_GAIN_0);
    // DUMP_RF_FIELD(QUASAR_FIELD_TX_GAIN_1);

    //restore other registers
    SET_PHY_REG(pMac->hHdd, QWLAN_TXCTL_DAC_CONTROL_REG, bkup1);
    SET_PHY_REG(pMac->hHdd, TXCTL_FIR_TX_MODE_REG, bkup2);
    SET_PHY_REG(pMac->hHdd, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, bkup0);   //restore this last

    //asicAGCReset(pMac);
#endif
    tANI_U32 bkup0, bkup1;

    //override = 1, en = 0
    SET_PHY_REG(pMac->hHdd, QWLAN_TPC_TXPWR_ENABLE_REG, QWLAN_TPC_TXPWR_ENABLE_OVERRIDE_MASK);

    GET_PHY_REG(pMac->hHdd, QWLAN_RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, &bkup0);
    GET_PHY_REG(pMac->hHdd, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, &bkup1);


    rdModWrAsicField(pMac, QWLAN_RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG,
                    QWLAN_RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_XBAR_MASK,
                    QWLAN_RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_XBAR_OFFSET, 1);


    rdModWrAsicField(pMac, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG,
                    QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TPC_MASK,
                    QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TPC_OFFSET, 1);
    rdModWrAsicField(pMac, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG,
                    QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TXFIR_APB_MASK,
                    QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TXFIR_APB_OFFSET, 1);


    SET_PHY_REG(pMac->hHdd, QWLAN_TPC_TXPWR_OVERRIDE0_REG,
                    (QWLAN_TPC_TXPWR_OVERRIDE0_RF_POWER_MASK & (tx0.coarsePwr << QWLAN_TPC_TXPWR_OVERRIDE0_RF_POWER_OFFSET)) |
                    (QWLAN_TPC_TXPWR_OVERRIDE0_FINE_POWER_MASK & (tx0.finePwr << QWLAN_TPC_TXPWR_OVERRIDE0_FINE_POWER_OFFSET))
               );

    SET_PHY_REG(pMac->hHdd, QWLAN_TPC_MAN_TXPWR_REG, QWLAN_TPC_MAN_TXPWR_STRB_MASK);


    SET_PHY_REG(pMac->hHdd, QWLAN_RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, bkup0);
    SET_PHY_REG(pMac->hHdd, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, bkup1);

    return (retVal);
}

eHalStatus asicTPCAutomatic(tpAniSirGlobal pMac)
{
    eHalStatus retVal;
    assert(pMac != 0);


    SET_PHY_REG(pMac->hHdd, QWLAN_TPC_TXPWR_ENABLE_REG, 0);   //turn off TPC closed loop control

#if defined(ANI_MANF_DIAG)
    SET_PHY_REG(pMac->hHdd, QWLAN_TPC_TXPWR_ENABLE_REG, pMac->hphy.phy.test.testTpcClosedLoop);
#else
    if(halIsQFuseBlown(pMac) == eHAL_STATUS_SUCCESS) //assuming valid pwr table data in qFuse
    {
        SET_PHY_REG(pMac->hHdd, QWLAN_TPC_TXPWR_ENABLE_REG, TPC_TXPWR_ENABLE_MASK);   //turn on TPC closed loop control
    }
    else
    {
        //Open the loop
        tTxGain fineGain;
        fineGain.coarsePwr = (eTxCoarseGain)(pMac->hphy.phy.openLoopTxGain);
        fineGain.finePwr = (eTxFineGain)(0xF);

        if ((retVal = asicTPCPowerOverride(pMac, fineGain, fineGain, fineGain, fineGain)) != eHAL_STATUS_SUCCESS) { return (retVal); }
        pMac->hphy.phy.test.testTpcClosedLoop = eANI_BOOLEAN_FALSE;
    }
#endif



    return (retVal);

}


eHalStatus asicLoadTPCPowerLUT(tpAniSirGlobal pMac, ePhyTxChains txChain, tANI_U8 *tpcPowerLUT)
{
    eHalStatus retVal;
    tANI_U32 enable;
    tANI_U32 point;
#ifdef ANI_PHY_DEBUG    
    tANI_U32 tpcLutCache[TPC_MEM_POWER_LUT_DEPTH];
#endif
    tANI_U32 pwrLutOffset;

    assert(pMac != 0);


    switch (txChain)
    {
        case PHY_TX_CHAIN_0:
            pwrLutOffset = QWLAN_TPC_POWERDET0_RAM_MREG;
            break;
        default:
            assert(0);
            return (eHAL_STATUS_FAILURE);
    }


    GET_PHY_REG(pMac->hHdd, QWLAN_TPC_TXPWR_ENABLE_REG, &enable);

    if ((enable & TPC_TXPWR_ENABLE_MASK) == 1)
    {
        SET_PHY_REG(pMac->hHdd, QWLAN_TPC_TXPWR_ENABLE_REG, 0);
    }

    SET_PHY_REG(pMac->hHdd, QWLAN_TPC_APBACCESS_REG, QWLAN_TPC_APBACCESS_SELECT_MASK);
    
    for (point = 0; point < TPC_MEM_POWER_LUT_DEPTH; point++)
    {
#ifdef ANI_PHY_DEBUG        
        tpcLutCache[point] = tpcPowerLUT[point];
#endif        
        SET_PHY_REG(pMac->hHdd, pwrLutOffset + (4 * point), tpcPowerLUT[point]);
    }

    //SET_PHY_REG(pMac->hHdd, QWLAN_TPC_APBACCESS_REG, QWLAN_TPC_APBACCESS_SELECT_MASK);

    //SET_PHY_MEMORY(pMac->hHdd, pwrLutOffset, (tANI_U8 *)tpcLutCache, TPC_MEM_POWER_LUT_DEPTH);


#ifdef ANI_PHY_DEBUG
    {
        tANI_U32 confirmTpcLutCache[TPC_MEM_POWER_LUT_DEPTH];

        GET_PHY_MEMORY(pMac->hHdd, pwrLutOffset, (tANI_U8 *)confirmTpcLutCache, TPC_MEM_POWER_LUT_DEPTH);

        if (memcmp(confirmTpcLutCache, tpcLutCache, sizeof(tANI_U32) * TPC_MEM_POWER_LUT_DEPTH) != 0)
        {
            phyLog(pMac, LOG2, "ERROR: TPC PWR LUT programming failed verification\n");
        }

        for (point = 0; point < TPC_MEM_POWER_LUT_DEPTH; point++)
        {
            phyLog(pMac, LOG2, "Writing TX%d TPC PWR LUT index %03d = %03d, Reading %03d\n",
                   (tANI_U8 )txChain, point, tpcPowerLUT[point], confirmTpcLutCache[point]
                  );
        }

    }
#endif
    SET_PHY_REG(pMac->hHdd, QWLAN_TPC_APBACCESS_REG, 0);

    if ((enable & TPC_TXPWR_ENABLE_MASK) == 1)
    {
        SET_PHY_REG(pMac->hHdd, QWLAN_TPC_TXPWR_ENABLE_REG, TPC_TXPWR_ENABLE_MASK)
    }


    return (retVal);
}


eHalStatus asicLoadTPCGainLUT(tpAniSirGlobal pMac, ePhyTxChains txChain, tTxGain *tpcGainLut)
{
    eHalStatus retVal;
    tANI_U32 enable;
    tANI_U32 point;
    tANI_U32 gainLutOffset;

    assert(pMac != 0);

    switch (txChain)
    {
        case PHY_TX_CHAIN_0:
            gainLutOffset = QWLAN_TPC_GAIN_LUT0_MREG;
            break;
        default:
            assert(0);
            return (eHAL_STATUS_FAILURE);
    }

    GET_PHY_REG(pMac->hHdd, QWLAN_TPC_TXPWR_ENABLE_REG, &enable);



    if ((enable & TPC_TXPWR_ENABLE_MASK) == 1)
    {
        SET_PHY_REG(pMac->hHdd, QWLAN_TPC_TXPWR_ENABLE_REG, 0);
    }

    SET_PHY_REG(pMac->hHdd, QWLAN_TPC_APBACCESS_REG, QWLAN_TPC_APBACCESS_SELECT_MASK);

    for (point = 0; point < TPC_MEM_GAIN_LUT_DEPTH; point++)
    {
        tANI_U32 addr = (gainLutOffset + (point * 4));
        tANI_U16 gain;
        gain = ((tpcGainLut[point].coarsePwr << TPC_GAIN_RF_OFFSET) + tpcGainLut[point].finePwr);

        SET_PHY_REG(pMac->hHdd, addr, gain);

#ifdef ANI_PHY_DEBUG
        {
            tANI_U32 val;

            GET_PHY_REG(pMac->hHdd, addr, &val);

            phyLog(pMac, LOG3, "Writing TX%d TPC Gain LUT(%X) index %d = %d, Reading %d\n",
                   (tANI_U8 )txChain, addr, point, gain, val);
        }
#endif
    }

    SET_PHY_REG(pMac->hHdd, QWLAN_TPC_APBACCESS_REG, 0);

    if ((enable & TPC_TXPWR_ENABLE_MASK) == 1)
    {
        SET_PHY_REG(pMac->hHdd, QWLAN_TPC_TXPWR_ENABLE_REG, TPC_TXPWR_ENABLE_MASK);
    }


    return (retVal);

}

#ifdef ANI_MANF_DIAG
eHalStatus asicGetTxGainAtIndex(tpAniSirGlobal pMac, ePhyTxChains txChain, tPwrTemplateIndex index, tTxGainCombo *retGain)
{
    eHalStatus retVal;
    tANI_U32 gain = 0;
    tANI_U32 enable;
    tANI_U32 gainLutOffset;

    assert(pMac != 0);

    switch (txChain)
    {
        case PHY_TX_CHAIN_0:
            gainLutOffset = QWLAN_TPC_GAIN_LUT0_MREG;
            break;
        default:
            assert(0);
            return (eHAL_STATUS_FAILURE);
    }

    GET_PHY_REG(pMac->hHdd, QWLAN_TPC_TXPWR_ENABLE_REG, &enable);

    if ((enable & TPC_TXPWR_ENABLE_MASK) == 1)
    {
        SET_PHY_REG(pMac->hHdd, QWLAN_TPC_TXPWR_ENABLE_REG, 0);
    }

    GET_PHY_REG(pMac->hHdd, (gainLutOffset + (index * 4)), &gain);

    if ((enable & TPC_TXPWR_ENABLE_MASK) == 1)
    {
        SET_PHY_REG(pMac->hHdd, QWLAN_TPC_TXPWR_ENABLE_REG, TPC_TXPWR_ENABLE_MASK);
    }

    *retGain = (tTxGainCombo)gain;
    return (retVal);
}


eHalStatus asicGetTxPowerLutAtIndex(tpAniSirGlobal pMac, ePhyTxChains txChain, tPowerDetect adcIndex, tPowerDetect *retPwr)
{
    eHalStatus retVal;
    tANI_U32 pwr = 0;
    tANI_U32 enable;
    tANI_U32 pwrLutOffset;

    assert(pMac != 0);

    switch (txChain)
    {
        case PHY_TX_CHAIN_0:
            pwrLutOffset = QWLAN_TPC_POWERDET0_RAM_MREG;
            break;
        default:
            assert(0);
            return (eHAL_STATUS_FAILURE);
    }

    GET_PHY_REG(pMac->hHdd, QWLAN_TPC_TXPWR_ENABLE_REG, &enable);

    if ((enable & TPC_TXPWR_ENABLE_MASK) == 1)
    {
        SET_PHY_REG(pMac->hHdd, QWLAN_TPC_TXPWR_ENABLE_REG, 0);
    }

    GET_PHY_REG(pMac->hHdd, (pwrLutOffset + (adcIndex * 4)), &pwr);

    if ((enable & TPC_TXPWR_ENABLE_MASK) == 1)
    {
        SET_PHY_REG(pMac->hHdd, QWLAN_TPC_TXPWR_ENABLE_REG, TPC_TXPWR_ENABLE_MASK);
    }

    *retPwr = (tPowerDetect)pwr;

    return (retVal);
}

#define MAX_PWR_MEAS_ITER   50
eHalStatus asicGetTxPowerMeasurement(tpAniSirGlobal pMac, ePhyTxChains txChain, tANI_U8 *retAdc)
{
    eHalStatus retVal;
    tANI_U32 status;
    tANI_U32 adc = 0;
    tANI_U8 iter = 0;

    assert(pMac != 0);
    phyLog(pMac, LOG1, "asicGetTxPowerMeasurement TX%d\n", txChain);



    do
    {
        SET_PHY_REG(pMac->hHdd, TPC_ADC_CTRL_REG, TPC_ADC_GET_MASK);

        //TODO:revisit

        retVal = palWaitRegVal(pMac->hHdd, QWLAN_TPC_ADC_STATUS_REG, (TPC_ADC_BUSY_P_MASK | TPC_ADC_BUSY_T_MASK), 0, 1000, 10000, &status);

        if (retVal != eHAL_STATUS_SUCCESS)
        {
            assert(0);
            break;
        }
    }while (status & TPC_ADC_FAILED_MASK && (iter++ <= MAX_PWR_MEAS_ITER));

    if(!((status & TPC_ADC_FAILED_MASK) == 0))
    {
        phyLog(pMac, LOGE, "ERROR: ADC status busy went low following a power-adc response timeout\n");
        return eHAL_STATUS_FAILURE;
    }

    adc = 0;
    *retAdc = (tANI_U8)adc;



    return (retVal);
}

eHalStatus asicTPCGetADCReading(tpAniSirGlobal pMac, tANI_U8 *pADC)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
    
    SET_PHY_REG(pMac->hHdd, QWLAN_TPC_ADC_CTRL_GET_ADC_REG, QWLAN_TPC_ADC_CTRL_GET_ADC_GET_ADC_MASK);
    {
        tANI_U32 i = 1000;  //number of iterations to wait for reading to complete
        tANI_U32 status = 0;

        do
        {
            GET_PHY_REG(pMac->hHdd, QWLAN_TPC_ADC_STATUS_REG, &status);
        }while(i-- > 0 && (status & QWLAN_TPC_ADC_STATUS_BUSY_P_MASK));
    
        if (status & QWLAN_TPC_ADC_STATUS_FAILED_MASK)
        {
            assert(0);
            *pADC = 0;
            return (eHAL_STATUS_FAILURE);
        }
    }
    
    {
        tANI_U32 adc;
        GET_PHY_REG(pMac->hHdd, QWLAN_TPC_SENSED_PWR0_REG, &adc);
        
        *pADC = (tANI_U8)adc;
    }

    return (retVal);
}


#endif

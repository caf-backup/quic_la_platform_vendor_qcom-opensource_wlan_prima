

#include "sys_api.h"


static eHalStatus rfWriteDataField(tpAniSirGlobal pMac, tANI_U32 regNum, tANI_U32 dataMask, tANI_U32 dataShift, tANI_U32 data);
static eHalStatus rfReadDataField(tpAniSirGlobal pMac, tANI_U32 regAddr, tANI_U32 dataMask, tANI_U32 dataShift, tANI_U32 *pData);

#define PWR_MEAS_DELAY          80
#define PWR_MEAS_WAIT_TIME      100
#define SETTLING_TIME           200

#define TX_DCO_RANGE_SETTING    1
#define RX_DCO_RANGE_SETTING    1

#define SET_RF_CHIP_REG(regAddr, regVal)                                                            \
{                                                                                                   \
    if (palWriteRegister(pMac->hHdd, regAddr, regVal) == eHAL_STATUS_FAILURE)                          \
    {                                                                                               \
        return (eHAL_STATUS_FAILURE);                                                               \
    };                                                                                              \
}

#define GET_RF_CHIP_REG(regAddr, pVal)                                                              \
{                                                                                                   \
    if (palReadRegister(pMac->hHdd, regAddr, pVal) == eHAL_STATUS_FAILURE)                             \
    {                                                                                               \
        return (eHAL_STATUS_FAILURE);                                                               \
    };                                                                                              \
}


#define SET_RF_FIELD(regAddr, mask, shift, regVal)                                                  \
{                                                                                                   \
    rfWriteDataField(pMac, regAddr, mask, shift, regVal);                                    \
}

#define GET_RF_FIELD(regAddr, mask, shift, pRegVal)                                                  \
{                                                                                                   \
    rfReadDataField(pMac, regAddr, mask, shift, pRegVal);                                    \
}

#define MIDAS_REG_STEP  4

eHalStatus rfWriteReg(tpAniSirGlobal pMac, tANI_U32 addr, tANI_U32 value)
{
    SET_RF_CHIP_REG(addr, value);

    return (eHAL_STATUS_SUCCESS);
}

eHalStatus rfReadReg(tpAniSirGlobal pMac, tANI_U32 addr, tANI_U32 *value)
{
    GET_RF_CHIP_REG(addr, value);

    return (eHAL_STATUS_SUCCESS);
}


static eHalStatus rfWriteDataField(tpAniSirGlobal pMac, tANI_U32 regAddr, tANI_U32 dataMask, tANI_U32 dataShift, tANI_U32 data)
{
    tANI_U32 regData;


    GET_RF_CHIP_REG(regAddr, &regData);

    regData &= ~(dataMask);
    regData |= ((data << dataShift) & dataMask);

    SET_RF_CHIP_REG(regAddr, (tANI_U16)regData);

    return (eHAL_STATUS_SUCCESS);
}

static eHalStatus rfReadDataField(tpAniSirGlobal pMac, tANI_U32 regAddr, tANI_U32 dataMask, tANI_U32 dataShift, tANI_U32 *pData)
{


    GET_RF_CHIP_REG(regAddr, pData);

    *pData &= dataMask;
    *pData >>= dataShift;

    return (eHAL_STATUS_SUCCESS);
}


eHalStatus rfWriteField(tpAniSirGlobal pMac, tANI_U32 regAddr, tANI_U32 dataMask, tANI_U32 dataShift, tANI_U32 data)
{


    SET_RF_FIELD(regAddr, dataMask, dataShift, data);

    return (eHAL_STATUS_SUCCESS);
}

eHalStatus rfReadField(tpAniSirGlobal pMac, tANI_U32 regAddr, tANI_U32 dataMask, tANI_U32 dataShift, tANI_U32 *pData)
{


    GET_RF_FIELD(regAddr, dataMask, dataShift, pData);

    return (eHAL_STATUS_SUCCESS);
}

#ifndef VERIFY_HALPHY_SIMV_MODEL
//calibration support functions

eHalStatus rfTakeTemp(tpAniSirGlobal pMac, eRfTempSensor setup, tANI_U8 nSamples, tTempADCVal *retTemp)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
    tANI_U32 avg = 0;
    tANI_U32 i;
    tANI_U32 bkup0, /*bkup1,*/ bkup2, bkup3;
    tANI_U16 adc = 0;

    assert (nSamples > 0);
    //assumes that we are not transmitting

    GET_PHY_REG(pMac->hHdd, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, &bkup0);
    GET_PHY_REG(pMac->hHdd, QWLAN_TPC_RC_DELAY_REG, &bkup2);       //when sampling hdet directly, only sample for 80 clocks = 1 microsecond
    GET_PHY_REG(pMac->hHdd, QWLAN_TPC_TXPWR_ENABLE_REG, &bkup3);


    SET_PHY_REG(pMac->hHdd, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG,
                                bkup0 |
                                QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TXCTL_MASK |
                                QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TXFIR_MASK |
                                QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TXFIR_APB_MASK |
                                QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TPC_MASK
               );

    SET_RF_CHIP_REG(QWLAN_RFAPB_TX_RF_1_EN_REG, 0xfc0f);
    SET_RF_CHIP_REG(QWLAN_RFAPB_TX_RF_2_EN_REG, QWLAN_RFAPB_TX_RF_2_EN_RF_EN_2_OVERWRITE_MASK |
                                                        QWLAN_RFAPB_TX_RF_2_EN_HDET_EN_OVRWRT_MASK);
    SET_PHY_REG(pMac->hHdd, QWLAN_TPC_TXPWR_ENABLE_REG, 0);   //turn off TPC closed loop control
    SET_PHY_REG(pMac->hHdd, QWLAN_TPC_RC_DELAY_REG, 80);       //when sampling hdet directly, only sample for 80 clocks = 1 microsecond

    switch (setup)
    {
        case TEMP_SENSOR_PA:
             //select Vtemp_pa
            SET_RF_FIELD(QWLAN_RFAPB_HDET_CTL_REG, QWLAN_RFAPB_HDET_CTL_HDET_OUT_SEL_MASK, QWLAN_RFAPB_HDET_CTL_HDET_OUT_SEL_OFFSET, 8);

            //enable pa temp sensor
            SET_RF_FIELD(QWLAN_RFAPB_HDET_TEST_REG, QWLAN_RFAPB_HDET_TEST_PA_TEMP_SEN_EN_MASK, QWLAN_RFAPB_HDET_TEST_PA_TEMP_SEN_EN_OFFSET, 1);
            break;
        case TEMP_SENSOR_RX:
            break;

        default:
            break;
    }

    for (i = 0; i < nSamples; i++)
    {
        asicTPCGetADCReading(pMac, &adc);

        avg += adc;
    }

    avg = avg / nSamples;

    //disable pa temp sensor
    SET_RF_FIELD(QWLAN_RFAPB_HDET_TEST_REG, QWLAN_RFAPB_HDET_TEST_PA_TEMP_SEN_EN_MASK, QWLAN_RFAPB_HDET_TEST_PA_TEMP_SEN_EN_OFFSET, 0);

    //select hdet_in
    SET_RF_FIELD(QWLAN_RFAPB_HDET_CTL_REG, QWLAN_RFAPB_HDET_CTL_HDET_OUT_SEL_MASK, QWLAN_RFAPB_HDET_CTL_HDET_OUT_SEL_OFFSET, 1);

    SET_PHY_REG(pMac->hHdd, QWLAN_TPC_RC_DELAY_REG, bkup2);
    SET_PHY_REG(pMac->hHdd, QWLAN_TPC_TXPWR_ENABLE_REG, bkup3);

    SET_RF_CHIP_REG(QWLAN_RFAPB_TX_RF_1_EN_REG, 0);
    SET_RF_CHIP_REG(QWLAN_RFAPB_TX_RF_2_EN_REG, 0);
    
    SET_PHY_REG(pMac->hHdd, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, bkup0);

    *retTemp = (tTempADCVal)avg;
    return retVal;
}


eHalStatus rfSetDCOffset(tpAniSirGlobal pMac, ePhyRxChains rxChain, tANI_U8 dcoIndex, tRxDcoCorrect offset)
{
#ifdef VOLANS_RF
        assert (dcoIndex < NUM_RF_DCO_VALUES);
        assert(rxChain < PHY_MAX_RX_CHAINS);

        SET_RF_CHIP_REG(QWLAN_RFAPB_RX_DCOC_IQ_0_REG + MIDAS_REG_STEP * dcoIndex,
                        (offset.IDcoCorrect << QWLAN_RFAPB_RX_DCOC_IQ_0_RX_DCOC_I_0_OFFSET) |
                        (offset.QDcoCorrect << QWLAN_RFAPB_RX_DCOC_IQ_0_RX_DCOC_Q_0_OFFSET)
                       );
#endif

    return eHAL_STATUS_SUCCESS;
}


eHalStatus rfGetDCOffset(tpAniSirGlobal pMac, ePhyRxChains rxChain, tANI_U8 dcoIndex, tRxDcoCorrect *offset)
{
#ifdef VOLANS_RF
        tANI_U32 value = 0;

        assert (dcoIndex < NUM_RF_DCO_VALUES);

        GET_RF_CHIP_REG(QWLAN_RFAPB_RX_DCOC_IQ_0_REG + MIDAS_REG_STEP * dcoIndex, &value);

        offset->IDcoCorrect = (tDcoCorrect)((value & QWLAN_RFAPB_RX_DCOC_IQ_0_RX_DCOC_I_0_MASK)
                                                            >> QWLAN_RFAPB_RX_DCOC_IQ_0_RX_DCOC_I_0_OFFSET);
        offset->QDcoCorrect = (tDcoCorrect)((value & QWLAN_RFAPB_RX_DCOC_IQ_0_RX_DCOC_Q_0_MASK)
                                                            >> QWLAN_RFAPB_RX_DCOC_IQ_0_RX_DCOC_Q_0_OFFSET);
#endif
    return eHAL_STATUS_SUCCESS;
}


eHalStatus rfGetTxLoCorrect(tpAniSirGlobal pMac, ePhyTxChains txChain, eGainSteps txGain, tTxLoCorrect *corr)
{
#ifdef VOLANS_RF
        tANI_U32 value = 0;

        assert (corr != NULL);

        GET_RF_CHIP_REG(QWLAN_RFAPB_TX_BBF_OFFSET_0_REG + MIDAS_REG_STEP * txGain, &value);

        corr->IDcoCorrect = (tDcoCorrect)((value & QWLAN_RFAPB_TX_BBF_OFFSET_0_TX_OFFS_DAC_I_0_MASK)
                                                            >> QWLAN_RFAPB_TX_BBF_OFFSET_0_TX_OFFS_DAC_I_0_OFFSET);
        corr->QDcoCorrect = (tDcoCorrect)((value & QWLAN_RFAPB_TX_BBF_OFFSET_0_TX_OFFS_DAC_Q_0_MASK)
                                                            >> QWLAN_RFAPB_TX_BBF_OFFSET_0_TX_OFFS_DAC_Q_0_OFFSET);
#endif

    return eHAL_STATUS_SUCCESS;
}


eHalStatus rfSetTxLoCorrect(tpAniSirGlobal pMac, ePhyTxChains txChain, eGainSteps txGain, tTxLoCorrect corr)
{
#ifdef VOLANS_RF
    SET_RF_CHIP_REG(QWLAN_RFAPB_TX_BBF_OFFSET_0_REG + MIDAS_REG_STEP * txGain,
                    (corr.IDcoCorrect << QWLAN_RFAPB_TX_BBF_OFFSET_0_TX_OFFS_DAC_I_0_OFFSET) |
                    (corr.QDcoCorrect << QWLAN_RFAPB_TX_BBF_OFFSET_0_TX_OFFS_DAC_Q_0_OFFSET)
                   );
#endif
    return eHAL_STATUS_SUCCESS;
}

eHalStatus rfHdetDCOCal(tpAniSirGlobal pMac, tANI_U16 *hdetDcocCode)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
#ifdef VOLANS_RF
    tANI_U32 regVal;
    //tANI_U32 timestamp = 0;
    tANI_U8 hdetCalCode;

    GET_RF_CHIP_REG(QWLAN_RFAPB_TX_RF_1_EN_REG, &regVal);
    SET_RF_CHIP_REG(QWLAN_RFAPB_TX_RF_1_EN_REG, (regVal |
                    QWLAN_RFAPB_TX_RF_1_EN_RF_EN_1_OVERWRITE_MASK |
                    QWLAN_RFAPB_TX_RF_1_EN_TX_BIAS_EN_OVRWRT_MASK));

    GET_RF_CHIP_REG(QWLAN_RFAPB_TX_RF_2_EN_REG, &regVal);
    SET_RF_CHIP_REG(QWLAN_RFAPB_TX_RF_2_EN_REG, (regVal |
                    QWLAN_RFAPB_TX_RF_2_EN_RF_EN_2_OVERWRITE_MASK |
                    QWLAN_RFAPB_TX_RF_2_EN_HDET_EN_OVRWRT_MASK));

    rdModWrAsicField(pMac, QWLAN_RFAPB_HDET_CTL_REG,
                    QWLAN_RFAPB_HDET_CTL_HDET_CSTART_MASK,
                    QWLAN_RFAPB_HDET_CTL_HDET_CSTART_OFFSET, 1);

    //GET_TIMESTAMP(&timestamp);
    //do
    //{
    //    GET_RF_CHIP_REG(QWLAN_RFAPB_HDET_CTL_REG, &regVal);
    //
    //    regVal &= QWLAN_RFAPB_HDET_CTL_HDET_CSTART_MASK;
    //} while (regVal && (ELAPSED_MICROSECONDS(timestamp) < 40));
    DELAY_MICROSECONDS(40);


    GET_RF_CHIP_REG(QWLAN_RFAPB_CAL_DATA_REG, &regVal);

    hdetCalCode = ((regVal & QWLAN_RFAPB_CAL_DATA_HDET_CAL_CODE_MASK)
                            >> QWLAN_RFAPB_CAL_DATA_HDET_CAL_CODE_OFFSET);

    SET_RF_FIELD(QWLAN_RFAPB_HDET_DCOC_REG,
            QWLAN_RFAPB_HDET_DCOC_DCOC_CODE_MASK,
            QWLAN_RFAPB_HDET_DCOC_DCOC_CODE_OFFSET,
            hdetCalCode);
    SET_RF_FIELD(QWLAN_RFAPB_HDET_DCOC_REG,
            QWLAN_RFAPB_HDET_DCOC_OVRD_HDET_MASK,
            QWLAN_RFAPB_HDET_DCOC_OVRD_HDET_OFFSET, 1);

    SET_RF_CHIP_REG(QWLAN_RFAPB_TX_RF_1_EN_REG, 0);
    SET_RF_CHIP_REG(QWLAN_RFAPB_TX_RF_2_EN_REG, 0);

#endif
    *hdetDcocCode = hdetCalCode;

    return retVal;
}

eHalStatus rfGetHdetDCOffset(tpAniSirGlobal pMac, tANI_U16 *hdetDcoOffset)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
#ifdef VOLANS_RF
    tANI_U32 regVal;
    tANI_U32 bkup1, bkup2;
    tANI_U32 rcDelay;

    GET_RF_CHIP_REG(QWLAN_RFAPB_TX_RF_1_EN_REG, &regVal);
    SET_RF_CHIP_REG(QWLAN_RFAPB_TX_RF_1_EN_REG, (regVal |
                                    QWLAN_RFAPB_TX_RF_1_EN_RF_EN_1_OVERWRITE_MASK |
                                    QWLAN_RFAPB_TX_RF_1_EN_TX_BIAS_EN_OVRWRT_MASK));

    GET_RF_CHIP_REG(QWLAN_RFAPB_TX_RF_2_EN_REG, &regVal);
    SET_RF_CHIP_REG(QWLAN_RFAPB_TX_RF_2_EN_REG, (regVal |
                                    QWLAN_RFAPB_TX_RF_2_EN_RF_EN_2_OVERWRITE_MASK |
                                    QWLAN_RFAPB_TX_RF_2_EN_HDET_EN_OVRWRT_MASK));

    GET_PHY_REG(pMac->hHdd, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, &bkup1);
    SET_PHY_REG(pMac->hHdd, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, (bkup1 |
                    QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TPC_MASK |
                    QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TXFIR_MASK |
                    QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TXFIR_APB_MASK |
                    QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TXCTL_MASK));

    GET_PHY_REG(pMac->hHdd, QWLAN_TXCTL_DAC_CONTROL_REG, &regVal);
    bkup2 = regVal;
    regVal |= QWLAN_TXCTL_DAC_CONTROL_TXEN_OVERRIDE_EN_MASK;
    regVal &= ~QWLAN_TXCTL_DAC_CONTROL_TXEN0_OVERRIDE_VAL_MASK;
    SET_PHY_REG(pMac->hHdd, QWLAN_TXCTL_DAC_CONTROL_REG, regVal);
    DELAY_MICROSECONDS(SETTLING_TIME);

    GET_PHY_REG(pMac->hHdd, QWLAN_TPC_RC_DELAY_REG, &rcDelay);
    SET_PHY_REG(pMac->hHdd, QWLAN_TPC_RC_DELAY_REG, PWR_MEAS_DELAY);

    SET_PHY_REG(pMac->hHdd, QWLAN_TPC_ADC_CTRL_GET_ADC_REG, QWLAN_TPC_ADC_CTRL_GET_ADC_GET_ADC_MASK);
    DELAY_MICROSECONDS(PWR_MEAS_WAIT_TIME);

    GET_PHY_REG(pMac->hHdd, QWLAN_TPC_SENSED_PWR0_REG, &regVal);
    SET_PHY_REG(pMac->hHdd, QWLAN_TPC_RC_DELAY_REG, rcDelay);

    SET_PHY_REG(pMac->hHdd, QWLAN_TXCTL_DAC_CONTROL_REG, bkup2);
    SET_PHY_REG(pMac->hHdd, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, bkup1);
    SET_RF_CHIP_REG(QWLAN_RFAPB_TX_RF_1_EN_REG, 0);
    SET_RF_CHIP_REG(QWLAN_RFAPB_TX_RF_2_EN_REG, 0);

    *hdetDcoOffset = (tANI_U16)regVal;
#endif
    return retVal;
}

#endif /* #ifndef VERIFY_HALPHY */

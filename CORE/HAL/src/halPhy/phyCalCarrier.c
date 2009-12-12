/**
 *
   Airgo Networks, Inc proprietary.
   All Rights Reserved, Copyright 2005
   This program is the confidential and proprietary product of Airgo Networks Inc.
   Any Unauthorized use, reproduction or transfer of this program is strictly prohibited.


   phyCalCarrier.cc: physical layer calibration of carrier suppression
   Author:  Mark Nelson
   Date:    3/12/05

   History -
   Date        Modified by              Modification Information
  --------------------------------------------------------------------------

 */

#include "string.h"
#include "sys_api.h"



#define OPTIMAL_TX_GAIN  7
#define OPTIMAL_RX_GAIN  30

#define SQUARE(x)   (x * x)
#define ABS(x)      ((x < 0) ? -x : x)
#define SIGN(x)     ((x != 0) ? ((x) / ABS(x)):0)

#define CAL_GAIN_INDEX  QUASAR_GAIN_MAX_INDEX

typedef enum
{
    INCREMENT_CORRECTION = 1,
    DECREMENT_CORRECTION,
    CORRECTION_REACHED
} tDirection;

static eHalStatus setRxGain(tpAniSirGlobal pMac,tANI_U8 rxChainGain1, tANI_U8 rxChainGain2, tANI_U8 rxChainGain3)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
#ifdef ANI_PHY_DEBUG
    tANI_U32 val;
#endif

    if ((retVal = asicAGCReset(pMac)) != eHAL_STATUS_SUCCESS) {  return retVal; }

    if (pMac->hphy.phy.calInfo.currentCalibration == CAL_MODE_ETXLOCAL)
    {
        // set quasar car gc2 to set gc_select=rx
        if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_GC_SELECT, 0)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    }

    if ((retVal = asicOverrideAGCRxChainGain(pMac, PHY_RX_CHAIN_0, rxChainGain1)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    if ((retVal = asicOverrideAGCRxChainGain(pMac, PHY_RX_CHAIN_1, rxChainGain2)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    if ((retVal = asicOverrideAGCRxChainGain(pMac, PHY_RX_CHAIN_2, rxChainGain3)) != eHAL_STATUS_SUCCESS) { return (retVal); }

#ifdef ANI_PHY_DEBUG
    if (pMac->hphy.phy.phyDebugLogLevel >= LOG4)
    {
        rfReadQuasarField(pMac, QUASAR_FIELD_RXG_0, &val);
        phyLog(pMac, LOG4, "In function setRxGain RxGain0 = %d", val);

        rfReadQuasarField(pMac, QUASAR_FIELD_RXG_1, &val);
        phyLog(pMac, LOG4, "RxGain1 = %d", val);

        rfReadQuasarField(pMac, QUASAR_FIELD_RXG_2, &val);
        phyLog(pMac, LOG4, "RxGain2 = %d", val);
    }
#endif

    return retVal;
}

eHalStatus setTxGain(tpAniSirGlobal pMac, tTxGain tx0, tTxGain tx1)
{
#ifdef ANI_PHY_DEBUG
    tANI_U32 val = 0, value;
#endif
    eHalStatus retVal = eHAL_STATUS_SUCCESS;

    if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_GC_SELECT, 1)) != eHAL_STATUS_SUCCESS) { return (retVal); }

    /* Enable TPC, TXCTL, TXFIR in TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE Register */
    if ((retVal = rdModWrNovaField(pMac, TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG,
                        TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TPC_MASK,
                        TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TPC_OFFSET, 1)) != eHAL_STATUS_SUCCESS)
    {  return retVal; }
    if ((retVal = rdModWrNovaField(pMac, TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG,
                        TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TXCTL_MASK,
                        TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TXCTL_OFFSET, 1)) != eHAL_STATUS_SUCCESS)
    {  return retVal; }
    if ((retVal = rdModWrNovaField(pMac, TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG,
                        TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TXFIR_MASK,
                        TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TXFIR_OFFSET, 1)) != eHAL_STATUS_SUCCESS)
    {  return retVal; }


    if ((retVal = rdModWrNovaField(pMac, RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG,
                        RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_XBAR_MASK,
                        RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_XBAR_OFFSET, 1)) != eHAL_STATUS_SUCCESS)
    {  return retVal; }

#ifdef FIXME_GEN5
    SET_PHY_REG(pMac->hHdd, TXCTL_FIR_MODE_REG,
                       (TXCTL_FIR_MODE_ANT_EN_EBOTH << 1) | TXCTL_FIR_MODE_SELECT_FIRMODE_EGENERATOR);
#endif
    phyLog(pMac, LOG4, "Tx Gain 0 Coarse = %d, Fine = %d", tx0.coarsePwr, tx0.finePwr);
    phyLog(pMac, LOG4, "Tx Gain 1 Coarse = %d, Fine = %d", tx1.coarsePwr, tx1.finePwr);

    retVal = asicTPCPowerOverride(pMac, tx0, tx1);
    if (retVal != eHAL_STATUS_SUCCESS)
    {
        return (retVal);
    }

#ifdef ANI_PHY_DEBUG
    if (pMac->hphy.phy.phyDebugLogLevel >= LOG4)
    {
        rfReadQuasarField(pMac, QUASAR_FIELD_RX1_ST_EN, &value);
        phyLog(pMac, LOG4, "Quasar Rx1 State Enable = %d", value);
        rfReadQuasarField(pMac, QUASAR_FIELD_RX_ALL_ST_EN, &value);
        phyLog(pMac, LOG4, "Quasar All State Enable = %d", value);
        rfReadQuasarField(pMac, QUASAR_FIELD_TX_ST_EN, &value);
        phyLog(pMac, LOG4, "Quasar Tx state enable = %d", value);

        rfReadQuasarField(pMac, QUASAR_FIELD_RX_IQ_TEST_0, &value);
        phyLog(pMac, LOG4, "Quasar IQ test 0 = %d", value);
        rfReadQuasarField(pMac, QUASAR_FIELD_RX_IQ_TEST_1, &value);
        phyLog(pMac, LOG4, "Quasar IQ test 1 = %d", value);


        GET_PHY_REG(pMac->hHdd, AGC_RX_OVERRIDE_REG, &value);
        phyLog(pMac, LOG4, "AGC Rx Override Register = %d", value);
        GET_PHY_REG(pMac->hHdd, TXCTL_DAC_CONTROL_REG, &value);
        phyLog(pMac, LOG4, "TXCTL DAC Control Register = %d", value);

        rfReadQuasarField(pMac, QUASAR_FIELD_GC_SELECT, &val);
        phyLog(pMac, LOG4, "SetTxGain GC Select = %d", val);

        rfReadQuasarField(pMac, QUASAR_FIELD_TX_GAIN_0, &val);
        phyLog(pMac, LOG4, "SetTxGain after TPC Power override TxGain0 = %d", val);

        rfReadQuasarField(pMac, QUASAR_FIELD_TX_GAIN_1, &val);
        phyLog(pMac, LOG4, "TxGain1 = %d", val);

        rfReadQuasarField(pMac, QUASAR_FIELD_TX_GAIN_0, &val);
        phyLog(pMac, LOG4, "SetTxGain end TxGain0 = %d", val);

        rfReadQuasarField(pMac, QUASAR_FIELD_TX_GAIN_1, &val);
        phyLog(pMac, LOG4, "TxGain1 = %d", val);

        rfReadQuasarField(pMac, QUASAR_FIELD_RXG_0, &val);
        phyLog(pMac, LOG4, "RxGain0 = %d", val);

        rfReadQuasarField(pMac, QUASAR_FIELD_RXG_1, &val);
        phyLog(pMac, LOG4, "RxGain1 = %d", val);

        rfReadQuasarField(pMac, QUASAR_FIELD_RXG_2, &val);
        phyLog(pMac, LOG4, "RxGain2 = %d", val);
    }
#endif

    return retVal;
}

static eHalStatus setupTxCarrierSuppression(tpAniSirGlobal pMac, tANI_U32 txChain, tANI_U32 rxChain)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
    tANI_U32 value;

    if ((retVal = asicTxFirSetLoLeakageBypass(pMac,TRUE)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    if ((retVal = asicSetAGCCrossbar(pMac, AGC_RX_0_AND_1)) != eHAL_STATUS_SUCCESS) { return (retVal); }

    if ((retVal = asicSetDisabledRxPacketTypes(pMac, PHY_RX_DISABLE_ALL_TYPES)) != eHAL_STATUS_SUCCESS) { return (retVal); }

    GET_PHY_REG(pMac->hHdd, AGC_RX_OVERRIDE_REG, &value);
    SET_PHY_REG(pMac->hHdd, AGC_RX_OVERRIDE_REG, value | (AGC_RX_OVERRIDE_OVERRIDE_EN_MASK |
                                                            AGC_RX_OVERRIDE_ENRX_VAL_MASK));

    if (txChain == PHY_TX_CHAIN_0)
    {
        retVal = setRxGain(pMac, OPTIMAL_TX_GAIN, OPTIMAL_RX_GAIN, 0);
        if (retVal != eHAL_STATUS_SUCCESS)
        {
            return (retVal);
        }
    }
    else if (txChain == PHY_TX_CHAIN_1)
    {
        retVal = setRxGain(pMac, OPTIMAL_RX_GAIN, OPTIMAL_TX_GAIN, 0);
        if (retVal != eHAL_STATUS_SUCCESS)
        {
            return (retVal);
        }
    }

    if (txChain == PHY_TX_CHAIN_0)
    {
        if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_TX0_LO, TRUE)) != eHAL_STATUS_SUCCESS) {  return retVal; }
        if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_TX1_LO, FALSE)) != eHAL_STATUS_SUCCESS) {  return retVal; }
        if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_RX0_LO, FALSE)) != eHAL_STATUS_SUCCESS) {  return retVal; }
        if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_RX1_LO, TRUE)) != eHAL_STATUS_SUCCESS) {  return retVal; }
        if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_RX2_LO, FALSE)) != eHAL_STATUS_SUCCESS) {  return retVal; }

        if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_RX1_ST_EN, 2)) != eHAL_STATUS_SUCCESS) {  return retVal; }
        if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_RX_ALL_ST_EN, 2)) != eHAL_STATUS_SUCCESS) {  return retVal; }
        if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_TX_ST_EN, 1)) != eHAL_STATUS_SUCCESS) {  return retVal; }

        if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_TX_CAL_EN_0, 0)) != eHAL_STATUS_SUCCESS) {  return retVal; }
        if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_TX_CAL_EN_1, 0)) != eHAL_STATUS_SUCCESS) {  return retVal; }
        if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_RX_IQ_TEST_0, 0)) != eHAL_STATUS_SUCCESS) {  return retVal; }
        if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_RX_IQ_TEST_1, 1)) != eHAL_STATUS_SUCCESS) {  return retVal; }
        if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_RX_IQ_TEST_2_0, 0)) != eHAL_STATUS_SUCCESS) {  return retVal; }
        if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_RX_IQ_TEST_2_1, 0)) != eHAL_STATUS_SUCCESS) {  return retVal; }
        if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_RX_LB_GAIN, 0)) != eHAL_STATUS_SUCCESS) {  return retVal; }
        if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_TX_LB_GAIN, 1)) != eHAL_STATUS_SUCCESS) {  return retVal; }
    }
    else if (txChain == PHY_TX_CHAIN_1)
    {
        if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_TX0_LO, FALSE)) != eHAL_STATUS_SUCCESS) {  return retVal; }
        if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_TX1_LO, TRUE)) != eHAL_STATUS_SUCCESS) {  return retVal; }
        if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_RX0_LO, TRUE)) != eHAL_STATUS_SUCCESS) {  return retVal; }
        if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_RX1_LO, FALSE)) != eHAL_STATUS_SUCCESS) {  return retVal; }
        if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_RX2_LO, FALSE)) != eHAL_STATUS_SUCCESS) {  return retVal; }

        if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_RX1_ST_EN, 1)) != eHAL_STATUS_SUCCESS) {  return retVal; }
        if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_RX_ALL_ST_EN, 1)) != eHAL_STATUS_SUCCESS) {  return retVal; }
        if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_TX_ST_EN, 2)) != eHAL_STATUS_SUCCESS) {  return retVal; }

        if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_TX_CAL_EN_0, 0)) != eHAL_STATUS_SUCCESS) {  return retVal; }
        if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_TX_CAL_EN_1, 0)) != eHAL_STATUS_SUCCESS) {  return retVal; }
        if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_RX_IQ_TEST_0, 1)) != eHAL_STATUS_SUCCESS) {  return retVal; }
        if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_RX_IQ_TEST_1, 0)) != eHAL_STATUS_SUCCESS) {  return retVal; }
        if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_RX_IQ_TEST_2_0, 0)) != eHAL_STATUS_SUCCESS) {  return retVal; }
        if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_RX_IQ_TEST_2_1, 0)) != eHAL_STATUS_SUCCESS) {  return retVal; }
        if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_RX_LB_GAIN, 0)) != eHAL_STATUS_SUCCESS) {  return retVal; }
        if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_TX_LB_GAIN, 1)) != eHAL_STATUS_SUCCESS) {  return retVal; }
    }

#ifdef FIXME_GEN5
    if (txChain == PHY_TX_CHAIN_0)
    {
        tANI_U32 override;

        override = AGC_RX_OVERRIDE_OVERRIDE_EN_MASK | AGC_RX_OVERRIDE_ENRX_VAL_DEFAULT |
                                                     AGC_RX_OVERRIDE_STBY_VAL_EADC_STBY1_VAL1;

        SET_PHY_REG(pMac->hHdd, AGC_RX_OVERRIDE_REG, override);
    }
    else if (txChain == PHY_TX_CHAIN_1)
    {
        tANI_U32 override;

        override = AGC_RX_OVERRIDE_OVERRIDE_EN_MASK | AGC_RX_OVERRIDE_ENRX_VAL_DEFAULT |
                                                     AGC_RX_OVERRIDE_STBY_VAL_EADC_STBY0_VAL1;

        SET_PHY_REG(pMac->hHdd, AGC_RX_OVERRIDE_REG, override);
    }
#endif
    /* Set DAC Control Reg as follows: txen_override_en = 1, txen0_override_val = 0, txen1_override_val = 0,
       dac_override_en = 1, ch0stdby_override_val = 1, ch1stdby_override_val = 1,txen1_override_val = 1 */

    SET_PHY_REG(pMac->hHdd, TXCTL_DAC_CONTROL_REG, TXCTL_DAC_CONTROL_TXEN_OVERRIDE_EN_MASK
                                                  | TXCTL_DAC_CONTROL_DAC_OVERRIDE_EN_MASK
                                                  | TXCTL_DAC_CONTROL_CH1STDBY_OVERRIDE_VAL_MASK
                                                  | TXCTL_DAC_CONTROL_CH0STDBY_OVERRIDE_VAL_MASK);


    SET_PHY_REG(pMac->hHdd, CAL_CHAIN_SEL_REG, (((tANI_U32 )PHY_I_RAIL << CAL_CHAIN_SEL_IQSEL_OFFSET) | (tANI_U32 )rxChain));

    retVal = asicSetPhyCalMode(pMac, PHY_CAL_MODE_TXLOCAL);
    if (retVal != eHAL_STATUS_SUCCESS)
    {
        return (retVal);
    }

    retVal = asicSetPhyCalLength(pMac, 1023);
    if (retVal != eHAL_STATUS_SUCCESS)
    {
        return (retVal);
    }

    /* Set CAL Override Reg to bypass chn0_dc, chn1_dc, chn2_dc */
    asicZeroFineDCOCorrection(pMac, PHY_ALL_RX_CHAINS);

    if ((retVal = rdModWrNovaField(pMac, RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG,
                                    RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_CAL_MASK,
                                    RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_CAL_OFFSET, 1)) != eHAL_STATUS_SUCCESS)
    {
        return retVal;
    }

    /* Set the Clip High Threshold to 1536 and Clip Low Threshold to  512 */
    SET_PHY_REG(pMac->hHdd, AGC_TH_CLIP_HIGH_REG, (1024 + 512));
    SET_PHY_REG(pMac->hHdd, AGC_TH_CLIP_LOW_REG, (1024 - 512));

    return retVal;
}

static eHalStatus readDCOLut(tpAniSirGlobal pMac, tANI_U32 rxChain, tANI_U8 gain, tIQAdc *rail)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
    tANI_U32 value = 0;
    tRxDcoCorrect offset;

    GET_PHY_REG(pMac->hHdd, AGC_RX_OVERRIDE_REG, &value);
    SET_PHY_REG(pMac->hHdd, AGC_RX_OVERRIDE_REG, value | AGC_RX_OVERRIDE_ENRX_VAL_MASK);

    if (rxChain == PHY_RX_CHAIN_0)
    {
        if ((retVal = setRxGain(pMac, gain, 0, 0)) != eHAL_STATUS_SUCCESS) {  return retVal; }
    }
    else if (rxChain == PHY_RX_CHAIN_1)
    {
        if ((retVal = setRxGain(pMac, 0, gain, 0)) != eHAL_STATUS_SUCCESS) {  return retVal; }
    }
    else
    {
        if ((retVal = setRxGain(pMac, 0, 0, gain)) != eHAL_STATUS_SUCCESS) {  return retVal; }
    }

    SET_PHY_REG(pMac->hHdd, AGC_RX_OVERRIDE_REG, value);

    retVal = rfGetDCOffset(pMac, rxChain, gain, &offset);

    if (retVal != eHAL_STATUS_SUCCESS)
    {
        return retVal;
    }

    rail->I = (tANI_S16) offset.IDcoCorrect;
    rail->Q = (tANI_S16) offset.QDcoCorrect;

    return retVal;
}

static eHalStatus writeDCOLut (tpAniSirGlobal pMac, tANI_U32 rxChain,tANI_S16 irail, tANI_S16 qrail, tANI_U8 gain)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS, value = 0;
    //tRxGain  rxGain;
    tRxDcoCorrect offset;

    GET_PHY_REG(pMac->hHdd, AGC_RX_OVERRIDE_REG, &value);
    SET_PHY_REG(pMac->hHdd, AGC_RX_OVERRIDE_REG, value | AGC_RX_OVERRIDE_ENRX_VAL_MASK);

    if (rxChain == PHY_RX_CHAIN_0)
    {
        if ((retVal = setRxGain(pMac, gain, 0, 0)) != eHAL_STATUS_SUCCESS) {  return retVal; }
    }
    else if (rxChain == PHY_RX_CHAIN_1)
    {
        if ((retVal = setRxGain(pMac, 0, gain, 0)) != eHAL_STATUS_SUCCESS) {  return retVal; }
    }
    else
    {
        if ((retVal = setRxGain(pMac, 0, 0, gain)) != eHAL_STATUS_SUCCESS) {  return retVal; }
    }

    SET_PHY_REG(pMac->hHdd, AGC_RX_OVERRIDE_REG, value);

    offset.dcRange = 1;
    offset.IDcoCorrect = (tDcoCorrect) irail;
    offset.QDcoCorrect = (tDcoCorrect) qrail;

    return rfSetDCOffset(pMac, rxChain, gain, offset);
}

static eHalStatus checkOverflow(tpAniSirGlobal pMac, tANI_U32 rxChain, tIQAdc *overflow)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
    tANI_U32    value[2];

    if (rxChain == PHY_RX_CHAIN_0)
    {
        GET_PHY_MEMORY(pMac->hHdd, AGC_SAMPLE_ADC_I0_REG, value, 2);
        overflow->I = (tANI_U16)(value[0]  & AGC_SAMPLE_ADC_I0_OVFL_MASK) >> AGC_SAMPLE_ADC_I0_OVFL_OFFSET;
        overflow->Q = (tANI_U16)(value[1] & AGC_SAMPLE_ADC_Q0_OVFL_MASK) >> AGC_SAMPLE_ADC_Q0_OVFL_OFFSET;
    }
    else if (rxChain == PHY_RX_CHAIN_1)
    {
        GET_PHY_MEMORY(pMac->hHdd, AGC_SAMPLE_ADC_I1_REG, value, 2);
        overflow->I = (tANI_U16)(value[0] & AGC_SAMPLE_ADC_I1_OVFL_MASK) >> AGC_SAMPLE_ADC_I1_OVFL_OFFSET;
        overflow->Q = (tANI_U16)(value[1] & AGC_SAMPLE_ADC_Q1_OVFL_MASK) >> AGC_SAMPLE_ADC_Q1_OVFL_OFFSET;
    }
    else if (rxChain == PHY_RX_CHAIN_2)
    {
        GET_PHY_MEMORY(pMac->hHdd, AGC_SAMPLE_ADC_I2_REG, value, 2);
        overflow->I = (tANI_U16)(value[0] & AGC_SAMPLE_ADC_I2_OVFL_MASK) >> AGC_SAMPLE_ADC_I2_OVFL_OFFSET;
        overflow->Q = (tANI_U16)(value[1] & AGC_SAMPLE_ADC_Q2_OVFL_MASK) >> AGC_SAMPLE_ADC_Q2_OVFL_OFFSET;
    }

    phyLog(pMac, LOG3, "Overflow I = %d, Q = %d\n", overflow->I, overflow->Q);
    return retVal;
}

static tANI_U16 inline power(tANI_U16 x, tANI_U16 y)
{
    tANI_U16 i = 0, result = 1;

    for (i = 0; i < y; i++)
    {
        result *= x;
    }

    return result;
}

static tANI_S16 twoscomplement(tANI_U16 val, tANI_U16 nbits)
{
    tANI_S16 result = (tANI_S16)val;

    if (val > power(2, nbits - 1))
    {
        result = val - power(2, nbits);
    }

    return result;
}

extern eHalStatus asicGetCalStatus(tpAniSirGlobal pMac, ePhyCalState *status);

static eHalStatus measureTxLoDCO(tpAniSirGlobal pMac, tANI_U32 rxChain, tIQAdc *dco)
{
    tANI_U32 toCnt = 10;
    tANI_U32 Cnt = 0;
    tANI_U32 status = 0;
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
    tANI_U32 value;
    tIQAdc  overflow;
    tIQAdc  temp = {0, 0};
    tANI_U32    calVal[2];

    if(pMac->hphy.phy.calInfo.currentCalibration == CAL_MODE_EINITDCCAL)
    {
        asicSetPhyCalMode(pMac, CAL_MODE_EINITDCCAL);
    }

    //Convert the below invidual register writes to a single memory write to speed up the writes.
    //SET_PHY_REG(pMac->hHdd, CAL_CHAIN_SEL_REG, (((tANI_U32) PHY_I_RAIL << CAL_CHAIN_SEL_IQSEL_OFFSET) | rxChain));
    //SET_PHY_REG(pMac->hHdd, CAL_MEASURE_REG, CAL_MEASURE_CMD_EMEASURE);
    calVal[0] = CAL_MEASURE_CMD_EMEASURE;
    calVal[1] = (((tANI_U32) PHY_I_RAIL << CAL_CHAIN_SEL_IQSEL_OFFSET) | rxChain);

    SET_PHY_MEMORY(pMac->hHdd, CAL_MEASURE_REG, calVal, 2);

    if (pMac->hphy.phy.calInfo.currentCalibration == CAL_MODE_EINITDCCAL)
        toCnt = 1000;

    while ((Cnt < toCnt) && (status != 1))
    {
        if (pMac->hphy.phy.calInfo.currentCalibration == CAL_MODE_ETXLOCAL)
        {
            sirBusyWait(1000);
        }
        retVal = asicGetCalStatus(pMac, (ePhyCalState *)&status);

        if (retVal != eHAL_STATUS_SUCCESS)
        {
            return (retVal);
        }
        Cnt += 1;
        if (pMac->hphy.phy.calInfo.currentCalibration == CAL_MODE_ETXLOCAL)
        {
            status = twoscomplement((tANI_U16)status, 2);
        }
    }
    if (Cnt == toCnt)
    {
        phyLog(pMac, LOGE, "Cal Measure did not return from Busy\n");
    }

    SET_PHY_REG(pMac->hHdd, CAL_CHAIN_SEL_REG, (((tANI_U32) PHY_I_RAIL << CAL_CHAIN_SEL_IQSEL_OFFSET) | rxChain));
    GET_PHY_REG(pMac->hHdd, CAL_DCOFFSET_REG, &value);
    dco->I = (((tANI_S16)value & CAL_DCOFFSET_DCOFFSET_MASK));
    dco->I = (tANI_S16)twoscomplement(dco->I, 10);

    SET_PHY_REG(pMac->hHdd, CAL_CHAIN_SEL_REG, (((tANI_U32) PHY_Q_RAIL << CAL_CHAIN_SEL_IQSEL_OFFSET) | rxChain));
    GET_PHY_REG(pMac->hHdd, CAL_DCOFFSET_REG, &value);
    dco->Q = (((tANI_S16)value & CAL_DCOFFSET_DCOFFSET_MASK));
    dco->Q = (tANI_S16)twoscomplement(dco->Q, 10);

    toCnt = 1;
    
    if(pMac->hphy.phy.calInfo.currentCalibration == CAL_MODE_EINITDCCAL)
    {
        asicSetPhyCalMode(pMac, CAL_MODE_ENORMAL);
        toCnt = 5;
    }

    for (Cnt = 0; Cnt < toCnt; Cnt++)
    {
        retVal = checkOverflow(pMac, rxChain, &overflow);
        if (retVal != eHAL_STATUS_SUCCESS)
        {

            return (retVal);
        }
        if (overflow.I)
            temp.I++;
        if (overflow.Q)
            temp.Q++;
    }

    if (temp.I)
        overflow.I = 1;
    else
        overflow.I = 0;

    if (temp.Q)
        overflow.Q = 1;
    else
        overflow.Q = 0;

    phyLog(pMac, LOG3, "Overflow , I = %d, Q = %d\n", overflow.I, overflow.Q);

    if (overflow.I == 1) {
        if (dco->I > 0)
        {
            dco->I -= 1023;
        }
        else
        {
            dco->I += 1023;
        }
    }

    if (overflow.Q == 1) {
        if (dco->Q > 0)
        {
            dco->Q -= 1023;
        }
        else
        {
            dco->Q += 1023;
        }
    }

    return retVal;
}

static eHalStatus calibrateLoopbackDCO(tpAniSirGlobal pMac, tANI_U32 rxChain, tANI_U8 rxGain, tIQAdc *dcoMinVal)
{
    tIQAdc          dcoOptVal = {0, 0};
    tIQAdc          currDco, dirDco, dco;
    tANI_U8         loopI = 0, loopQ = 0, loop;
    eHalStatus      retVal = eHAL_STATUS_SUCCESS;
    tANI_U32        value = 0;
    tRxDcoCorrect   offset;
    eRfSubBand      currBand;
    tDirection      IDir, QDir;

    currBand = rfGetAGBand(pMac);

    dcoMinVal->I = dcoMinVal->Q = 10000;

    GET_PHY_REG(pMac->hHdd, AGC_RX_OVERRIDE_REG, &value);
    SET_PHY_REG(pMac->hHdd, AGC_RX_OVERRIDE_REG, value | AGC_RX_OVERRIDE_ENRX_VAL_MASK);

    if (rxChain == PHY_RX_CHAIN_0)
    {
        if ((retVal = setRxGain(pMac, rxGain, 0, 0)) != eHAL_STATUS_SUCCESS) {  return retVal; }
    }
    else if (rxChain == PHY_RX_CHAIN_1)
    {
        if ((retVal = setRxGain(pMac, 0, rxGain, 0)) != eHAL_STATUS_SUCCESS) {  return retVal; }
    }
    else
    {
        if ((retVal = setRxGain(pMac, 0, 0, rxGain)) != eHAL_STATUS_SUCCESS) {  return retVal; }
    }

    SET_PHY_REG(pMac->hHdd, AGC_RX_OVERRIDE_REG, value);

    //Measure the error with the current correction values
    offset.dcRange = 1;
    offset.IDcoCorrect = (tDcoCorrect) pMac->hphy.phy.calTable[currBand]->txloDcoCorrect[rxChain].IDcoCorrect;
    offset.QDcoCorrect = (tDcoCorrect) pMac->hphy.phy.calTable[currBand]->txloDcoCorrect[rxChain].QDcoCorrect;

    retVal = rfSetDCOffset(pMac, rxChain, rxGain, offset);
    if (retVal != eHAL_STATUS_SUCCESS)
    {
        return (retVal);
    }

    retVal = measureTxLoDCO(pMac, rxChain, &currDco);
    if (retVal != eHAL_STATUS_SUCCESS)
    {
        return (retVal);
    }

    phyLog(pMac, LOG2, "With Previous correction: I = %u, Q = %u; CurrError: I = %d, Q = %d", offset.IDcoCorrect, offset.QDcoCorrect,
                                                                            currDco.I, currDco.Q);
    //Find the directions(positive or negative) for I and Q. Increment both I and Q and measure the error. 
    //If both the I and Q error measured is less than the error measured above, then the direction is +ve 
    //for both I and Q. If if both errors are high compared to error measured above, the direction is -ve
    //for I and Q. If I error is less and Q error is more, then I direction is +ve and Q direction is -ve.
    //If I error is more and Q error is less, then I direction is -ve and Q direction is +ve.
    offset.IDcoCorrect += 1;
    offset.QDcoCorrect += 1;

    offset.IDcoCorrect &= 0x7F;
    offset.QDcoCorrect &= 0x7F;
    
    retVal = rfSetDCOffset(pMac, rxChain, rxGain, offset);
    if (retVal != eHAL_STATUS_SUCCESS)
    {
        return (retVal);
    }

    retVal = measureTxLoDCO(pMac, rxChain, &dirDco);
    if (retVal != eHAL_STATUS_SUCCESS)
    {
        return (retVal);
    }

    phyLog(pMac, LOG2, "With incremented correction: I = %u, Q = %u; CurrError: I = %d, Q = %d", offset.IDcoCorrect, offset.QDcoCorrect,
                                                                            dirDco.I, dirDco.Q);
    
    if (ABS(currDco.I) >= ABS(dirDco.I))
    {
        IDir = INCREMENT_CORRECTION;
        phyLog(pMac, LOG2, "IDIR Positive");
    }
    else
    {
        IDir = DECREMENT_CORRECTION;
        phyLog(pMac, LOG2, "IDIR Negative");
    }

    if (ABS(currDco.Q) >= ABS(dirDco.Q))
    {
        QDir = INCREMENT_CORRECTION;
        phyLog(pMac, LOG2, "QDIR Positive");
    }
    else
    {
        QDir = DECREMENT_CORRECTION;
        phyLog(pMac, LOG2, "QDIR Negative");
    }

  
    //Set the corrections same as that of the previous I and Q values
    offset.IDcoCorrect -= 1;
    offset.QDcoCorrect -= 1;

    offset.IDcoCorrect &= 0x7F;
    offset.QDcoCorrect &= 0x7F;
    
    for (loop = 0; loop < 128; loop++)
    {
        loopI = offset.IDcoCorrect;
        loopQ = offset.QDcoCorrect;

        dco.I = currDco.I;
        dco.Q = currDco.Q;
            
        //the correction values are signed 7-bit, so we need to shift them up one so the compiler performs the adjustment correctly, then shift them back down
        if (IDir == INCREMENT_CORRECTION)
        {
            //positive error = positive adjustment
            offset.IDcoCorrect += 1;
        }
        else if (IDir == DECREMENT_CORRECTION)
        {
            //negative error = negative adjustment
            offset.IDcoCorrect -= 1;
        }
        offset.IDcoCorrect &= 0x7F;

        if (QDir == INCREMENT_CORRECTION)
        {
            //positive error = positive adjustment
            offset.QDcoCorrect += 1;
        }
        else if (QDir == DECREMENT_CORRECTION)
        {
            //negative error = negative adjustment
            offset.QDcoCorrect -= 1;
        }
        offset.QDcoCorrect &= 0x7F;

        retVal = rfSetDCOffset(pMac, rxChain, rxGain, offset);
        if (retVal != eHAL_STATUS_SUCCESS)
        {
            return (retVal);
        }

        retVal = measureTxLoDCO(pMac, rxChain, &currDco);
        if (retVal != eHAL_STATUS_SUCCESS)
        {
            return (retVal);
        }
        
        phyLog(pMac, LOG3, "Prev DCO:I = %d, Q = %d, loop = %d, Correction: I = %d, Q = %d\n", dco.I, dco.Q, loop, loopI, loopQ);
        phyLog(pMac, LOG3, "Curr DCO, I = %d, Q = %d, currCorr: I = %d, Q = %d\n", currDco.I, currDco.Q, offset.IDcoCorrect, offset.QDcoCorrect);

        if (ABS(dco.I) < ABS(currDco.I))
        {
            IDir = CORRECTION_REACHED;
            offset.IDcoCorrect = (tDcoCorrect)loopI;
            dcoMinVal->I = dco.I;
        }

        
        if (ABS(dco.Q) < ABS(currDco.Q))
        {
            QDir = CORRECTION_REACHED;
            offset.QDcoCorrect = (tDcoCorrect)loopQ;
            dcoMinVal->Q = dco.Q;
        }

        if (IDir == CORRECTION_REACHED && QDir == CORRECTION_REACHED)
        {
            break;
        }
    }

    if (loop == 128)
    {
        phyLog(pMac, LOGE, "Number of Iterations = %d, Taking the corrections from the last loop",loop);
        loopI = offset.IDcoCorrect;
        loopQ = offset.QDcoCorrect;
    }
    else
    {
        phyLog(pMac, LOG1, "Number of Iterations = %d",loop);
    }

    
    dcoOptVal.I = (tANI_S16)loopI;
    dcoOptVal.Q = (tANI_S16)loopQ;
    
    phyLog(pMac, LOG1, "Opt Val, I = %d, Q = %d; Min Error: I = %d, Q = %d\n", dcoOptVal.I, dcoOptVal.Q, dcoMinVal->I, dcoMinVal->Q);
    
    offset.dcRange = 1;
    pMac->hphy.phy.calTable[currBand]->txloDcoCorrect[rxChain].IDcoCorrect = offset.IDcoCorrect = (tDcoCorrect) dcoOptVal.I;
    pMac->hphy.phy.calTable[currBand]->txloDcoCorrect[rxChain].QDcoCorrect = offset.QDcoCorrect = (tDcoCorrect) dcoOptVal.Q;
    
    retVal = rfSetDCOffset(pMac, rxChain, rxGain, offset);
    if (retVal != eHAL_STATUS_SUCCESS)
    {
        return (retVal);
    }

    return retVal;
}

tANI_S16 convert_sign_mag(tANI_S16 value, tANI_U16 nbits)
{
    tANI_U32 magMask = (1 << (nbits - 1)) - 1;
    tANI_S32 sgnBit = ((value < 0) << (nbits - 1));
    return ((value > 0) ? (value & ((1 << nbits) - 1)) : (tANI_S16)(sgnBit | (ABS(value) & magMask)));
}

static eHalStatus restore(tpAniSirGlobal pMac)
{
    eHalStatus retVal;
#if 0
    tANI_U32 value;
#endif

    if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_RX_IQ_TEST_0, 0)) != eHAL_STATUS_SUCCESS) {  return retVal; }
    if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_RX_IQ_TEST_1, 0)) != eHAL_STATUS_SUCCESS) {  return retVal; }

#if 0
    if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_RX1_ST_EN, 0x7)) != eHAL_STATUS_SUCCESS) {  return retVal; }
    if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_RX_ALL_ST_EN, 0x7)) != eHAL_STATUS_SUCCESS) {  return retVal; }
    if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_TX_ST_EN, 0x3)) != eHAL_STATUS_SUCCESS) {  return retVal; }
#endif

    if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_TX0_LO, 0x1)) != eHAL_STATUS_SUCCESS) {  return retVal; }
    if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_TX1_LO, 0x1)) != eHAL_STATUS_SUCCESS) {  return retVal; }
    if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_RX0_LO, 0x1)) != eHAL_STATUS_SUCCESS) {  return retVal; }
    if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_RX1_LO, 0x1)) != eHAL_STATUS_SUCCESS) {  return retVal; }
    if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_RX2_LO, 0x1)) != eHAL_STATUS_SUCCESS) {  return retVal; }

    //Select Rx
    if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_GC_SELECT, 0x0)) != eHAL_STATUS_SUCCESS) {  return retVal; }
    if ((retVal = rfWriteQuasarField( pMac, QUASAR_FIELD_SPI_LUT_EN, 0)) != eHAL_STATUS_SUCCESS) {  return retVal; }

#if 0
    GET_PHY_REG(pMac->hHdd, TPC_TXPWR_ENABLE_REG, &value);
    SET_PHY_REG(pMac->hHdd, TPC_TXPWR_ENABLE_REG, value | 0x1);
#endif

#ifdef FIXME_GEN5
    SET_PHY_REG(pMac->hHdd, TXCTL_FIR_MODE_REG,
         (TXCTL_FIR_MODE_ANT_EN_EBOTH << 1) | TXCTL_FIR_MODE_SELECT_FIRMODE_ENORMAL);
#endif

    asicSetPhyCalMode(pMac, CAL_MODE_ENORMAL);

    if ((retVal = rdModWrNovaField(pMac, AGC_GAINSET0_REG, AGC_GAINSET0_OVERRIDE_MASK,
                AGC_GAINSET0_OVERRIDE_OFFSET, 0)) != eHAL_STATUS_SUCCESS)
    {  return retVal; }
    if ((retVal = rdModWrNovaField(pMac, AGC_GAINSET1_REG, AGC_GAINSET1_OVERRIDE_MASK,
                AGC_GAINSET1_OVERRIDE_OFFSET, 0)) != eHAL_STATUS_SUCCESS)
    {  return retVal; }
    if ((retVal = rdModWrNovaField(pMac, AGC_GAINSET2_REG, AGC_GAINSET2_OVERRIDE_MASK,
                AGC_GAINSET2_OVERRIDE_OFFSET, 0)) != eHAL_STATUS_SUCCESS)
    {  return retVal; }
    if ((retVal = rdModWrNovaField(pMac, AGC_RX_OVERRIDE_REG, AGC_RX_OVERRIDE_OVERRIDE_EN_MASK,
                AGC_RX_OVERRIDE_OVERRIDE_EN_OFFSET, 0)) != eHAL_STATUS_SUCCESS)
    {  return retVal; }

#if 0
    if ((retVal = rdModWrNovaField(pMac, TXCTL_DAC_CONTROL_REG, TXCTL_DAC_CONTROL_TXEN_OVERRIDE_EN_MASK,
                 TXCTL_DAC_CONTROL_TXEN_OVERRIDE_EN_OFFSET, 0)) != eHAL_STATUS_SUCCESS)
    {  return retVal; }

    if ((retVal = rdModWrNovaField(pMac, TXCTL_DAC_CONTROL_REG, TXCTL_DAC_CONTROL_DAC_OVERRIDE_EN_MASK,
                                        TXCTL_DAC_CONTROL_DAC_OVERRIDE_EN_OFFSET, 0)) != eHAL_STATUS_SUCCESS)
    {  return retVal; }

    if ((retVal = rdModWrNovaField(pMac, TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG,
                                    TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TPC_MASK,
                                    TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TPC_OFFSET, 0)) != eHAL_STATUS_SUCCESS)
    {  return retVal; }
    if ((retVal = rdModWrNovaField(pMac, TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG,
                                     TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TXCTL_MASK,
                                     TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TXCTL_OFFSET, 0)) != eHAL_STATUS_SUCCESS)
    {  return retVal; }
    if ((retVal = rdModWrNovaField(pMac, TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG,
                                     TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TXFIR_MASK,
                                     TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TXFIR_OFFSET, 0)) != eHAL_STATUS_SUCCESS)
    {  return retVal; }
    
    if ((retVal = rdModWrNovaField(pMac, RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG,
                                     RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_XBAR_MASK,
                                     RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_XBAR_OFFSET, 0)) != eHAL_STATUS_SUCCESS)
    {  return retVal; }
    if ((retVal = rdModWrNovaField(pMac, RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG,
                                     RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_CAL_MASK,
                                     RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_CAL_OFFSET, 0)) != eHAL_STATUS_SUCCESS)
    {  return retVal; }

#endif

    SET_PHY_REG(pMac->hHdd, AGC_TH_CLIP_HIGH_REG, 1824);

    SET_PHY_REG(pMac->hHdd, AGC_TH_CLIP_LOW_REG, 224);


    //Found these additional registers not restored
    SET_PHY_REG(pMac->hHdd, CAL_OVERRIDE_REG, 0);
    SET_PHY_REG(pMac->hHdd, CAL_LENGTH_REG, 0x1F);

    //asicSetTxDACs(pMac, PHY_NO_TX_CHAINS, eANI_BOOLEAN_OFF, eANI_BOOLEAN_OFF);

    return retVal;
}

tANI_U32 calculateError(tIQAdc complex1, tIQAdc complex2)
{
    tIQAdc result;
    tANI_U32 error;

    result.I = complex1.I - complex2.I;
    result.Q = complex1.Q - complex2.Q;

    error = (result.I * result.I);
    error = error + (result.Q * result.Q);

    return (error);
}


#define MAX_TX_LO_CAL_ITER  (40)
#define TX_LO_CAL_TOLERANCE (2)

eHalStatus phyCarrierSuppressTxChain(tpAniSirGlobal pMac, ePhyTxChains txChain, tTxGain gain)
{
    eHalStatus      retVal;
    tANI_U32        iter = 0, val;
    tANI_U32        curError, minError;
    tANI_U32        deltaError;
    tANI_S32        deltaRawI, deltaRawQ, diffICorr, diffQCorr;
    tIQAdc          deltaDco, curCorr = {0, 0}, minCorr = {0, 0}, tempCorr;
    tIQAdc          resDco, savedDco;
    tIQAdc          currDco, currStep;
    eRfSubBand      currBand;
    tTxLoCorrect    txloCorrection;
    ePhyTxChains    rxChain = PHY_RX_CHAIN_0;
    tTxLoCorrect    txloPrevCorrection[MAX_TX_LO_CAL_ITER];
    tTxGain         txGain0, txGain1;
    
    pMac->hphy.phy.calInfo.currentCalibration = CAL_MODE_ETXLOCAL;

    currBand = rfGetAGBand(pMac);

    if (txChain == PHY_TX_CHAIN_0)
    {
        rxChain = PHY_RX_CHAIN_1;
    }
    else
    {
        rxChain = PHY_RX_CHAIN_0;
    }

    if ((retVal = setupTxCarrierSuppression(pMac, txChain, rxChain)) != eHAL_STATUS_SUCCESS)
    {
        pMac->hphy.phy.calInfo.currentCalibration = CAL_MODE_ENORMAL;
        return retVal;
    }

#ifdef ANI_PHY_DEBUG
    if (pMac->hphy.phy.phyDebugLogLevel >= LOG4)
    {
        rfReadQuasarField(pMac, QUASAR_FIELD_RXG_0, &val);
        phyLog(pMac, LOG4, "After Set up RxGain0 = %d", val);

        rfReadQuasarField(pMac, QUASAR_FIELD_RXG_1, &val);
        phyLog(pMac, LOG4, "RxGain1 = %d", val);

        rfReadQuasarField(pMac, QUASAR_FIELD_RXG_2, &val);
        phyLog(pMac, LOG4, "RxGain2 = %d", val);
    }
#endif

    if (txChain == PHY_TX_CHAIN_0)
    {
        retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_TX0_LO, 0);
        if (retVal != eHAL_STATUS_SUCCESS)
        {
            pMac->hphy.phy.calInfo.currentCalibration = CAL_MODE_ENORMAL;
            return (retVal);
        }
    }
    else if (txChain == PHY_TX_CHAIN_1)
    {
        retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_TX1_LO, 0);
        if (retVal != eHAL_STATUS_SUCCESS)
        {
            pMac->hphy.phy.calInfo.currentCalibration = CAL_MODE_ENORMAL;
            return (retVal);
        }
    }

#ifdef ANI_PHY_DEBUG
    if (pMac->hphy.phy.phyDebugLogLevel >= LOG4)
    {
        GET_PHY_REG(pMac->hHdd, TXFIR_CFG_REG, &val);
        phyLog(pMac, LOG4, "TXFIR_CFG_REG Reg = 0x%x\n", val);
        GET_PHY_REG(pMac->hHdd, AGC_CONFIG_XBAR_REG, &val);
        phyLog(pMac, LOG4, "AGC_CONFIG_XBAR_REG Reg = 0x%x\n", val);
        GET_PHY_REG(pMac->hHdd, AGC_DIS_MODE_REG, &val);
        phyLog(pMac, LOG4, "AGC_DIS_MODE_REG Reg = 0x%x\n", val);
        GET_PHY_REG(pMac->hHdd, AGC_RX_OVERRIDE_REG, &val);
        phyLog(pMac, LOG4, "AGC_RX_OVERRIDE_REG Reg = 0x%x\n", val);
        GET_PHY_REG(pMac->hHdd, TXCTL_DAC_CONTROL_REG, &val);
        phyLog(pMac, LOG4, "TXCTL_DAC_CONTROL_REG Reg = 0x%x\n", val);
        GET_PHY_REG(pMac->hHdd, CAL_CHAIN_SEL_REG, &val);
        phyLog(pMac, LOG4, "CAL_CHAIN_SEL_REG Reg = 0x%x\n", val);
        GET_PHY_REG(pMac->hHdd, CAL_LENGTH_REG, &val);
        phyLog(pMac, LOG4, "CAL_LENGTH_REG Reg = 0x%x\n", val);
        GET_PHY_REG(pMac->hHdd, CAL_CALMODE_REG, &val);
        phyLog(pMac, LOG4, "CAL_CALMODE_REG Reg = 0x%x\n", val);
        GET_PHY_REG(pMac->hHdd, CAL_OVERRIDE_REG, &val);
        phyLog(pMac, LOG4, "CAL_OVERRIDE_REG Reg = 0x%x\n", val);
        GET_PHY_REG(pMac->hHdd, RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, &val);
        phyLog(pMac, LOG4, "RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG Reg = 0x%x\n", val);
    }
#endif

    retVal = readDCOLut(pMac, rxChain, OPTIMAL_RX_GAIN, &savedDco);
    if (retVal != eHAL_STATUS_SUCCESS)
    {
        return (retVal);
    }
    phyLog(pMac, LOG1, "Saved DCO : I = %d, Q = %d\n", savedDco.I, savedDco.Q);

    retVal = calibrateLoopbackDCO(pMac, rxChain, OPTIMAL_RX_GAIN, &resDco);
    if (retVal != eHAL_STATUS_SUCCESS)
    {
        return (retVal);
    }

    rfReadQuasarField(pMac, QUASAR_FIELD_RXG_0, &val);
    phyLog(pMac, LOG4, "After ResDCO Calibration RxGain0 = %d", val);

#ifdef ANI_PHY_DEBUG
    if (pMac->hphy.phy.phyDebugLogLevel >= LOG4)
    {
        rfReadQuasarField(pMac, QUASAR_FIELD_RXG_1, &val);
        phyLog(pMac, LOG4, "RxGain1 = %d", val);

        rfReadQuasarField(pMac, QUASAR_FIELD_RXG_2, &val);
        phyLog(pMac, LOG4, "RxGain2 = %d", val);
    }
#endif

    phyLog(pMac, LOG1, "Residual DCO : I = %d, Q = %d\n", resDco.I, resDco.Q);

    if (txChain == PHY_TX_CHAIN_0)
    {
        retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_TX0_LO, 1);
        if (retVal != eHAL_STATUS_SUCCESS)
        {
            pMac->hphy.phy.calInfo.currentCalibration = CAL_MODE_ENORMAL;
            return (retVal);
        }

        txGain0 = gain;
        txGain1.coarsePwr = txGain1.finePwr = 0;

        retVal = setTxGain(pMac, txGain0, txGain1);
        if (retVal != eHAL_STATUS_SUCCESS)
        {
            pMac->hphy.phy.calInfo.currentCalibration = CAL_MODE_ENORMAL;
            return (retVal);
        }
    }
    else if (txChain == PHY_TX_CHAIN_1)
    {
        retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_TX1_LO, 1);
        if (retVal != eHAL_STATUS_SUCCESS)
        {
            pMac->hphy.phy.calInfo.currentCalibration = CAL_MODE_ENORMAL;
            return (retVal);
        }

        txGain1 = gain;
        txGain0.coarsePwr = txGain0.finePwr = 0;

        retVal = setTxGain(pMac, txGain0, txGain1);
        if (retVal != eHAL_STATUS_SUCCESS)
        {
            pMac->hphy.phy.calInfo.currentCalibration = CAL_MODE_ENORMAL;
            return (retVal);
        }
    }

    txloPrevCorrection[0].iLo = txloCorrection.iLo = pMac->hphy.phy.calTable[currBand]->txloCorrection[txChain].iLo;
    txloPrevCorrection[0].qLo = txloCorrection.qLo = pMac->hphy.phy.calTable[currBand]->txloCorrection[txChain].qLo;
    
    retVal = rfSetTxLoCorrect(pMac, txChain, txloCorrection);
    if (retVal != eHAL_STATUS_SUCCESS)
    {
        pMac->hphy.phy.calInfo.currentCalibration = CAL_MODE_ENORMAL;
        return (retVal);
    }

    retVal = measureTxLoDCO(pMac, rxChain,&currDco);
    if (retVal != eHAL_STATUS_SUCCESS)
    {
        pMac->hphy.phy.calInfo.currentCalibration = CAL_MODE_ENORMAL;
        return (retVal);
    }

    curError = calculateError(currDco, resDco);
    minError = curError;
    minCorr.I = curCorr.I = txloCorrection.iLo;
    minCorr.Q = curCorr.Q = txloCorrection.qLo;
    
#ifdef ANI_PHY_DEBUG
    if (pMac->hphy.phy.phyDebugLogLevel >= LOG4)
    {
        rfReadQuasarField(pMac, QUASAR_FIELD_RXG_0, &val);
        phyLog(pMac, LOG4, "Before Correction algorithm RxGain0 = %d", val);

        rfReadQuasarField(pMac, QUASAR_FIELD_RXG_1, &val);
        phyLog(pMac, LOG4, "RxGain1 = %d", val);

        rfReadQuasarField(pMac, QUASAR_FIELD_RXG_2, &val);
        phyLog(pMac, LOG4, "RxGain2 = %d", val);
    }
#endif

    phyLog(pMac, LOG2, "Current Error = %u", curError);
    phyLog(pMac, LOG2, "loop = %d, Current Correction: I = %d, Q = %d", iter, curCorr.I, curCorr.Q);

    while (iter < MAX_TX_LO_CAL_ITER)
    {
        iter++;

        /* Delta Error Measurement in I Direction */
        tempCorr.I = curCorr.I + 1;
        tempCorr.Q = curCorr.Q;

        txloCorrection.iLo = (tANI_OB5)convert_sign_mag(tempCorr.I, 5);
        txloCorrection.qLo = (tANI_OB5)convert_sign_mag(tempCorr.Q, 5);

        retVal = rfSetTxLoCorrect(pMac, txChain, txloCorrection);
        if (retVal != eHAL_STATUS_SUCCESS)
        {
            pMac->hphy.phy.calInfo.currentCalibration = CAL_MODE_ENORMAL;
            return (retVal);
        }

        retVal = measureTxLoDCO(pMac, rxChain, &deltaDco);
        if (retVal != eHAL_STATUS_SUCCESS)
        {
            pMac->hphy.phy.calInfo.currentCalibration = CAL_MODE_ENORMAL;
            return (retVal);
        }

        deltaError = calculateError(deltaDco, resDco);
        deltaRawI = (deltaError - curError);

        phyLog(pMac, LOG3, "Delta I Error = %d", deltaError);
        phyLog(pMac, LOG3, "Delta DCO After I Correction : I = %d, Q = %d\n", deltaDco.I, deltaDco.Q);

        /* Delta Error Measurement in Q Direction */
        tempCorr.I = curCorr.I;
        tempCorr.Q = curCorr.Q + 1;

        txloCorrection.iLo = (tANI_OB5)convert_sign_mag(tempCorr.I, 5);
        txloCorrection.qLo = (tANI_OB5)convert_sign_mag(tempCorr.Q, 5);

        retVal = rfSetTxLoCorrect(pMac, txChain, txloCorrection);
        if (retVal != eHAL_STATUS_SUCCESS)
        {
            pMac->hphy.phy.calInfo.currentCalibration = CAL_MODE_ENORMAL;
            return (retVal);
        }

        retVal = measureTxLoDCO(pMac, rxChain, &deltaDco);
        if (retVal != eHAL_STATUS_SUCCESS)
        {
            pMac->hphy.phy.calInfo.currentCalibration = CAL_MODE_ENORMAL;
            return (retVal);
        }

        deltaError = calculateError(deltaDco, resDco);
        deltaRawQ = (deltaError - curError);

        phyLog(pMac, LOG3, "Delta Q Error = %d", deltaError);
        phyLog(pMac, LOG3, "Delta DCO After Q Correction : I = %d, Q = %d\n", deltaDco.I, deltaDco.Q);

        if ((ABS(deltaRawI)) < (ABS(deltaRawQ)))
        {
            deltaRawI = 0;
        }
        else
        {
            deltaRawQ = 0;
        }

        if (SIGN(deltaRawI) > 0)
        {
            currStep.I = -1;
        }
        else if (SIGN(deltaRawI) < 0)
        {
            currStep.I = 1;
        }
        else
        {
            currStep.I = 0;
        }

        if (SIGN(deltaRawQ) > 0)
        {
            currStep.Q = -1;
        }
        else if (SIGN(deltaRawQ) < 0)
        {
            currStep.Q = 1;
        }
        else
        {
            currStep.Q = 0;
        }

        phyLog(pMac, LOG3, "Current Step: I = %d, Q = %d\n", currStep.I, currStep.Q);
        curCorr.I = curCorr.I + currStep.I;
        curCorr.Q = curCorr.Q + currStep.Q;

        txloCorrection.iLo = (tANI_OB5)convert_sign_mag(curCorr.I, 5);
        txloCorrection.qLo = (tANI_OB5)convert_sign_mag(curCorr.Q, 5);

        retVal = rfSetTxLoCorrect(pMac, txChain, txloCorrection);
        if (retVal != eHAL_STATUS_SUCCESS)
        {
            pMac->hphy.phy.calInfo.currentCalibration = CAL_MODE_ENORMAL;
            return (retVal);
        }

        retVal = measureTxLoDCO(pMac, rxChain,&currDco);
        if (retVal != eHAL_STATUS_SUCCESS)
        {
            pMac->hphy.phy.calInfo.currentCalibration = CAL_MODE_ENORMAL;
            return (retVal);
        }
        curError = calculateError(currDco, resDco);
        phyLog(pMac, LOG3, "Result, I = %d, Q = %d\n", (currDco.I - resDco.I), (currDco.Q - resDco.Q));
        phyLog(pMac, LOG2, "Current Error = %u", curError);
        phyLog(pMac, LOG2, "loop = %d, Current Correction: I = %d, Q = %d", iter, curCorr.I, curCorr.Q);

        phyLog(pMac, LOG2, "Min Error = %d\n", minError);
        phyLog(pMac, LOG2, "Min Correction: I = %d, Q = %d\n", minCorr.I, minCorr.Q);

        if (curError < minError) 
        {
            minError = curError;
            minCorr.I = curCorr.I;
            minCorr.Q = curCorr.Q;
        }

        txloPrevCorrection[iter].iLo = txloCorrection.iLo;
        txloPrevCorrection[iter].qLo = txloCorrection.qLo;

        if (iter > 5)
        {
            diffICorr = txloPrevCorrection[iter].iLo - txloPrevCorrection[iter-6].iLo;
            diffQCorr = txloPrevCorrection[iter].qLo - txloPrevCorrection[iter-6].qLo;
            if ((ABS(diffICorr) + ABS(diffQCorr)) <= 1)
            {
                break;
            }
        }
    }

    phyLog(pMac, LOG1, "Iter = %d, Min Error = %d\n", iter, minError);
    phyLog(pMac, LOG1, "Min Correction: I = %d, Q = %d\n", minCorr.I, minCorr.Q);

    txloCorrection.iLo = (tANI_OB5)convert_sign_mag(minCorr.I, 5);
    txloCorrection.qLo = (tANI_OB5)convert_sign_mag(minCorr.Q, 5);

    retVal = rfSetTxLoCorrect(pMac, txChain, txloCorrection);
    if (retVal != eHAL_STATUS_SUCCESS)
    {
        pMac->hphy.phy.calInfo.currentCalibration = CAL_MODE_ENORMAL;
        return (retVal);
    }

    pMac->hphy.phy.calTable[currBand]->txloCorrection[txChain].iLo = txloCorrection.iLo;
    pMac->hphy.phy.calTable[currBand]->txloCorrection[txChain].qLo = txloCorrection.qLo;
    pMac->hphy.phy.calTable[currBand]->useTxLoCorrection = eANI_BOOLEAN_FALSE;

    phyLog(pMac, LOGE, "Chain = %d, Correction Table: I = %d, Q = %d\n", txChain, 
                                        pMac->hphy.phy.calTable[currBand]->txloCorrection[txChain].iLo, 
                                        pMac->hphy.phy.calTable[currBand]->txloCorrection[txChain].qLo);

    
    retVal = writeDCOLut(pMac, rxChain, savedDco.I, savedDco.Q, OPTIMAL_RX_GAIN);
    if (retVal != eHAL_STATUS_SUCCESS)
    {
        pMac->hphy.phy.calInfo.currentCalibration = CAL_MODE_ENORMAL;
        return (retVal);
    }

    phyLog(pMac, LOG1, "For TxChain %d, Quasar Correction made is : I = %d, Q = %d\n",
                                                         txChain, minCorr.I, minCorr.Q);
    pMac->hphy.phy.calInfo.currentCalibration = CAL_MODE_ENORMAL;
    return (retVal);
}

#if 0
#ifdef ANI_MANF_DIAG
eHalStatus phyDcoCalRxChain(tpAniSirGlobal pMac, ePhyRxChains rxChain, eGainSteps gain)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
    tANI_U32 iteration = 0, value = 0;
    tIQAdc dcoError = {1000, 1000};
    tIQAdc currDco;
    tIQAdc   dcoOptVal = {0, 0};
    eRfSubBand currBand;

    currBand = rfGetAGBand(pMac);

    pMac->hphy.phy.calInfo.currentCalibration = CAL_MODE_EINITDCCAL;
    GET_PHY_REG(pMac->hHdd, AGC_RX_OVERRIDE_REG, &value);
    SET_PHY_REG(pMac->hHdd, AGC_RX_OVERRIDE_REG, value | AGC_RX_OVERRIDE_ENRX_VAL_MASK);

    retVal = setRxGain(pMac, gain, gain, gain);
    if (retVal != eHAL_STATUS_SUCCESS)
    {
        return (retVal);
    }
    SET_PHY_REG(pMac->hHdd, AGC_RX_OVERRIDE_REG, value);

#ifdef ANI_PHY_DEBUG
    if (pMac->hphy.phy.phyDebugLogLevel >= LOG4)
    {
        rfReadQuasarField(pMac, QUASAR_FIELD_RXG_0, &value);
        phyLog(pMac, LOG4, "After Set up RxGain0 = %d", value);

        rfReadQuasarField(pMac, QUASAR_FIELD_RXG_1, &value);
        phyLog(pMac, LOG4, "RxGain1 = %d", value);

        rfReadQuasarField(pMac, QUASAR_FIELD_RXG_2, &value);
        phyLog(pMac, LOG4, "RxGain2 = %d", value);

        rfReadQuasarField(pMac, QUASAR_FIELD_RX_GAIN_0, &value);
        phyLog(pMac, LOG4, "GC4 RxGain0 = %d", value);

        rfReadQuasarField(pMac, QUASAR_FIELD_RX_GAIN_1, &value);
        phyLog(pMac, LOG4, "GC3 RxGain1 = %d", value);

        rfReadQuasarField(pMac, QUASAR_FIELD_RX_GAIN_2, &value);
        phyLog(pMac, LOG4, "GC3 RxGain2 = %d", value);

        rfReadQuasarField(pMac, QUASAR_FIELD_PLL_TUNE_ERR, &value);
        phyLog(pMac, LOG4, "PLL Tune Error Reg = 0x%x", value);
    }
#endif

    //Calibrate DCO for I & Q rails
    while (iteration < DCO_CORRECTION_RANGE)
    {
        retVal = writeDCOLut(pMac, (tANI_U32)rxChain, (tANI_S16)iteration, (tANI_S16)iteration, (tANI_U8)gain);
        if (retVal != eHAL_STATUS_SUCCESS)
        {
            pMac->hphy.phy.calInfo.currentCalibration = CAL_MODE_ENORMAL;
            return (retVal);
        }

#ifdef ANI_PHY_DEBUG
        if (pMac->hphy.phy.phyDebugLogLevel >= LOG4)
        {
            GET_PHY_REG(pMac->hHdd, AGC_RX_OVERRIDE_REG, &value);
            phyLog(pMac, LOG4, "AGC_RX_OVERRIDE_REG Reg = 0x%x\n", value);
            GET_PHY_REG(pMac->hHdd, TXCTL_DAC_CONTROL_REG, &value);
            phyLog(pMac, LOG4, "TXCTL_DAC_CONTROL_REG Reg = 0x%x\n", value);

            GET_PHY_REG(pMac->hHdd, RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, &value);
            phyLog(pMac, LOG4, "RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG Reg = 0x%x\n", value);


            rfReadQuasarField(pMac, QUASAR_FIELD_RX1_ST_EN, &value);
            phyLog(pMac, LOG4, "Quasar Rx1 State Enable = %d", value);
            rfReadQuasarField(pMac, QUASAR_FIELD_RX_ALL_ST_EN, &value);
            phyLog(pMac, LOG4, "Quasar All State Enable = %d", value);
            rfReadQuasarField(pMac, QUASAR_FIELD_TX_ST_EN, &value);
            phyLog(pMac, LOG4, "Quasar Tx state enable = %d", value);

            retVal = rfReadQuasarField(pMac, QUASAR_FIELD_TX0_LO, &value);
            phyLog(pMac, LOG4, "TX0 Lo  = %d", value);
            retVal = rfReadQuasarField(pMac, QUASAR_FIELD_TX1_LO, &value);
            phyLog(pMac, LOG4, "TX1 Lo = %d", value);
            retVal = rfReadQuasarField(pMac, QUASAR_FIELD_RX0_LO, &value);
            phyLog(pMac, LOG4, "RX0 Lo = %d", value);
            retVal = rfReadQuasarField(pMac, QUASAR_FIELD_RX1_LO, &value);
            phyLog(pMac, LOG4, "RX1 Lo = %d", value);
            retVal = rfReadQuasarField(pMac, QUASAR_FIELD_RX2_LO, &value);
            phyLog(pMac, LOG4, "RX2 Lo = %d", value);

            {
                tANI_U32 agc_sample_adc_chain = AGC_SAMPLE_ADC_I0_REG + ((tANI_U32)rxChain * 8);
                tANI_U32 sampleReg, i = 0;

                //read AGC_SAMPLE_ADC_I0_REG


                for (i = 0; i < 5; i++)
                {
                    GET_PHY_REG(pMac->hHdd, agc_sample_adc_chain, &sampleReg);
                    phyLog(pMac, LOG4, "Chain = %d, I Sample = 0x%x\n", rxChain, sampleReg);
                    GET_PHY_REG(pMac->hHdd, agc_sample_adc_chain + 4, &sampleReg); 
                    phyLog(pMac, LOG4, "Chain = %d, Q Sample = 0x%x\n", rxChain, sampleReg);
                }
            }
        }
#endif
        retVal = measureTxLoDCO(pMac, rxChain, &currDco);

        if (retVal != eHAL_STATUS_SUCCESS)
        {
            pMac->hphy.phy.calInfo.currentCalibration = CAL_MODE_ENORMAL;
            return (retVal);
        }

        if (ABS(currDco.I) < ABS(dcoError.I))
        {
            dcoOptVal.I = (tANI_U16)iteration;
            dcoError.I = currDco.I;
            phyLog(pMac, LOG4, "min real at Corval = %d\n",iteration);
        }
        if (ABS(currDco.Q) < ABS(dcoError.Q))
        {
            dcoOptVal.Q = (tANI_U16)iteration;
            dcoError.Q = currDco.Q;
            phyLog(pMac, LOG4, "min img at Corval = %d\n",iteration);
        }
        phyLog(pMac, LOG3, "DCO Error : I = %d, Q = %d\n", dcoError.I, dcoError.Q);
        phyLog(pMac, LOG3, "Chain = %d Gain = %d CorVal = %d dco = %d +j%d min_val = %d + j%d\n", rxChain, gain, iteration, currDco.I, currDco.Q, dcoError.I, dcoError.Q);
        iteration++;
    }

    phyLog(pMac, LOGE, "Chain = %d, DCO Correction Values : I = %d, Q = %d, Iterations = %d\n", rxChain, dcoOptVal.I, dcoOptVal.Q, iteration);
    writeDCOLut(pMac, rxChain, dcoOptVal.I, dcoOptVal.Q, gain);
    
    pMac->hphy.phy.calTable[currBand]->dcoCorrection[rxChain].IDcoCorrect = (tDcoCorrect)dcoOptVal.I;
    pMac->hphy.phy.calTable[currBand]->dcoCorrection[rxChain].QDcoCorrect = (tDcoCorrect)dcoOptVal.Q;
    pMac->hphy.phy.calTable[currBand]->dcoCorrection[rxChain].dcRange = 1;

#if 0
    asicSetAGCGainLut(pMac, rxChain, (tANI_U8)gain, (tANI_U8)gain, (const tRxGain *)&curVal);
    SET_PHY_REG(pMac->hHdd, GPIO_RFIF_EN_REG, lnaEnables);
#endif
    pMac->hphy.phy.calInfo.currentCalibration = CAL_MODE_ENORMAL;
    return retVal;
}
#endif  //  ANI_MANF_DIAG
#endif  // Comment ends here

eHalStatus phyPeriodicTxCarrierSuppression(tpAniSirGlobal pMac, ePhyTxChains txChain, tTxGain txGain)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
    eRfSubBand currBand;

    currBand = rfGetAGBand(pMac);
    pMac->hphy.phy.calTable[currBand]->useTxLoCorrection = eANI_BOOLEAN_FALSE;

    if (txChain == PHY_MAX_TX_CHAINS || txChain == PHY_ALL_TX_CHAINS) 
    {
        for (txChain = 0; txChain < PHY_MAX_TX_CHAINS; txChain++)
        {
            phyLog(pMac, LOG1, "Calibrating TxLo for Tx Chain %d and Rx Chain %d\n", txChain, (txChain)? 0:1);
            if ((retVal = phyCarrierSuppressTxChain(pMac, txChain, txGain)) != eHAL_STATUS_SUCCESS)
            { return (retVal); }
        }
    }
    else if (txChain == PHY_TX_CHAIN_1 || txChain == PHY_TX_CHAIN_0)
    {
        phyLog(pMac, LOG1, "Calibrating TxLo for Tx Chain %d and Rx Chain %d\n", txChain, (txChain)? 0:1);
        if ((retVal = phyCarrierSuppressTxChain(pMac, txChain, txGain)) != eHAL_STATUS_SUCCESS)
        { return (retVal); }
    }
    else
    {
        phyLog(pMac, LOGE, "ERROR: Tx Chain greater than 2, TxLo is not calibrated\n");
    }

    restore(pMac);
    pMac->hphy.phy.calTable[currBand]->useTxLoCorrection = eANI_BOOLEAN_TRUE;

    return (retVal);
}


eHalStatus phyInitTxCarrierSuppression(tpAniSirGlobal pMac)
{
    eHalStatus  retVal = eHAL_STATUS_SUCCESS;
    ePhyTxChains curr_tx_chain;
    ePhyRxChains curr_rx_chain;
    eRfSubBand currBand;
    tTxGain     txGain;

    currBand = rfGetAGBand(pMac);
    pMac->hphy.phy.calTable[currBand]->useTxLoCorrection = eANI_BOOLEAN_FALSE;
    txGain.coarsePwr = OPTIMAL_TX_GAIN;
    txGain.finePwr = 0;

    for (curr_tx_chain = 0; curr_tx_chain < PHY_MAX_TX_CHAINS; curr_tx_chain++)
    {
        if (curr_tx_chain == PHY_TX_CHAIN_0)
        {
            curr_rx_chain = PHY_RX_CHAIN_1;
            phyLog(pMac, LOG1, "Calibrating TxLo for Tx Chain 0 and Rx Chain 1\n");
        }
        else if (curr_tx_chain == PHY_TX_CHAIN_1)
        {
            curr_rx_chain = PHY_RX_CHAIN_0;
            phyLog(pMac, LOG1, "Calibrating TxLo for Tx Chain 1 and Rx Chain 0\n");
        }
        else
        {
            phyLog(pMac, LOGE, "ERROR: Tx Chain greater than 2, TxLo is not calibrated\n");
            break;
        }

        pMac->hphy.phy.calTable[currBand]->txloCorrection[curr_tx_chain].iLo = 0;
        pMac->hphy.phy.calTable[currBand]->txloCorrection[curr_tx_chain].qLo = 0;
        pMac->hphy.phy.calTable[currBand]->txloDcoCorrect[curr_rx_chain].IDcoCorrect = 0;
        pMac->hphy.phy.calTable[currBand]->txloDcoCorrect[curr_rx_chain].QDcoCorrect = 0;

        if ((retVal = phyCarrierSuppressTxChain(pMac, curr_tx_chain, txGain)) != eHAL_STATUS_SUCCESS)
        { return (retVal); }
    }

    restore(pMac);
    pMac->hphy.phy.calTable[currBand]->useTxLoCorrection = eANI_BOOLEAN_TRUE;

    return (retVal);
}

#if 0
#ifdef ANI_MANF_DIAG
eHalStatus phyInitRxDcoCal(tpAniSirGlobal pMac)
{
    eHalStatus retVal;
    ePhyRxChains rxChain;
    tANI_U32 gain, val;
    tANI_U8 calGainIndex = CAL_GAIN_INDEX;
    eRfSubBand  currBand;
    tANI_U32    clipThresholds[2];

    currBand = rfGetAGBand(pMac);

    pMac->hphy.phy.calTable[currBand]->useDcoCorrection = eANI_BOOLEAN_FALSE;

    retVal = rdModWrNovaField(pMac, AGC_BANDWIDTH_CONFIG_REG, AGC_BANDWIDTH_CONFIG_CB_ENABLE_MASK, 
                                                            AGC_BANDWIDTH_CONFIG_CB_ENABLE_OFFSET, 0);
    if (retVal != eHAL_STATUS_SUCCESS)
    {
        return retVal;
    }

    SET_PHY_REG(pMac->hHdd, TXCTL_DAC_CONTROL_REG, 0x33);

    //asicSetAGCCrossbar(pMac, AGC_ALL_RX);
    GET_PHY_REG(pMac->hHdd, AGC_CONFIG_XBAR_REG, &val);
    SET_PHY_REG(pMac->hHdd, AGC_CONFIG_XBAR_REG, (val & 0xFF00) | 0x10);
    if ((retVal = asicAGCReset(pMac)) != eHAL_STATUS_SUCCESS) {  return retVal; }

    if ((retVal = asicSetDisabledRxPacketTypes(pMac, PHY_RX_DISABLE_ALL_TYPES)) != eHAL_STATUS_SUCCESS){ return retVal; }
    SET_PHY_REG(pMac->hHdd, AGC_RX_OVERRIDE_REG, 0x0777);

    SET_PHY_REG(pMac->hHdd, CAL_OVERRIDE_REG, 0x07);
    //done in phyDcoCalRxChain
    //if ((retVal = asicOverrideAGCRxChainGain(pMac, PHY_ALL_RX_CHAINS, calGainIndex)) != eHAL_STATUS_SUCCESS) { return (retVal); }

    clipThresholds[0] = (1024 + 512);
    clipThresholds[1] = (1024 - 512);
    SET_PHY_MEMORY(pMac->hHdd, AGC_TH_CLIP_HIGH_REG, clipThresholds, 2);

#ifdef ANI_PHY_DEBUG
    if (pMac->hphy.phy.phyDebugLogLevel >= LOG4)
    {
        GET_PHY_REG(pMac->hHdd, AGC_BANDWIDTH_CONFIG_REG, &val);
        phyLog(pMac, LOG4, "AGC_BANDWIDTH_CONFIG_REG Reg = 0x%x\n", val);
        GET_PHY_REG(pMac->hHdd, AGC_CONFIG_XBAR_REG, &val);
        phyLog(pMac, LOG4, "AGC_CONFIG_XBAR_REG Reg = 0x%x\n", val);
        GET_PHY_REG(pMac->hHdd, AGC_DIS_MODE_REG, &val);
        phyLog(pMac, LOG4, "AGC_DIS_MODE_REG Reg = 0x%x\n", val);
        GET_PHY_REG(pMac->hHdd, AGC_RX_OVERRIDE_REG, &val);
        phyLog(pMac, LOG4, "AGC_RX_OVERRIDE_REG Reg = 0x%x\n", val);
        GET_PHY_REG(pMac->hHdd, TXCTL_DAC_CONTROL_REG, &val);
        phyLog(pMac, LOG4, "TXCTL_DAC_CONTROL_REG Reg = 0x%x\n", val);
        GET_PHY_REG(pMac->hHdd, CAL_OVERRIDE_REG, &val);
        phyLog(pMac, LOG4, "CAL_OVERRIDE_REG Reg = 0x%x\n", val);
    }
#endif

    for (rxChain = 0; rxChain < PHY_MAX_RX_CHAINS; rxChain++)
    {
        phyLog(pMac, LOG1, "Performing Initial DCO cal on RX%d\n", rxChain);

        if ((retVal = phyDcoCalRxChain(pMac, rxChain, calGainIndex)) != eHAL_STATUS_SUCCESS) { return (retVal); }

        for (gain = 0; gain < NUM_QUASAR_RX_GAIN_STEPS; gain++)
        {
            if ((retVal = writeDCOLut(pMac, rxChain, pMac->hphy.phy.calTable[currBand]->dcoCorrection[rxChain].IDcoCorrect, 
                pMac->hphy.phy.calTable[currBand]->dcoCorrection[rxChain].QDcoCorrect, (tANI_U8)gain)) != eHAL_STATUS_SUCCESS) { return (retVal); }
        }
        phyLog(pMac, LOG4, "DCO LUT loop Correction Values : I = %d, Q = %d\n", 
                            pMac->hphy.phy.calTable[currBand]->dcoCorrection[rxChain].IDcoCorrect,
                            pMac->hphy.phy.calTable[currBand]->dcoCorrection[rxChain].QDcoCorrect);
    }
    if ((retVal = asicZeroFineDCOCorrection(pMac, PHY_NO_RX_CHAINS)) != eHAL_STATUS_SUCCESS) { return (retVal); }

    clipThresholds[0] = AGC_TH_CLIP_HIGH_THRESHOLD_DEFAULT;
    clipThresholds[1] = AGC_TH_CLIP_LOW_THRESHOLD_DEFAULT;
    SET_PHY_MEMORY(pMac->hHdd, AGC_TH_CLIP_HIGH_REG, clipThresholds, 2);

    pMac->hphy.phy.calTable[currBand]->useDcoCorrection = eANI_BOOLEAN_TRUE;
    GET_PHY_REG(pMac->hHdd, AGC_RX_OVERRIDE_REG, &val);
    SET_PHY_REG(pMac->hHdd, AGC_RX_OVERRIDE_REG, val | AGC_RX_OVERRIDE_ENRX_VAL_MASK);
    setRxGain(pMac, 0, 0, 0);
    SET_PHY_REG(pMac->hHdd, AGC_RX_OVERRIDE_REG, val);
    if ((retVal = asicCeaseOverrideAGCRxChainGain(pMac, PHY_ALL_RX_CHAINS)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    return (retVal);
}
#endif  //ANI_MANF_DIAG
#endif  // Comment ends here

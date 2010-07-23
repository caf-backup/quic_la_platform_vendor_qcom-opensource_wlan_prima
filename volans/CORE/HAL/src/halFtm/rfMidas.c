

#include "sys_api.h"


static eHalStatus rfWriteDataField(tpAniSirGlobal pMac, eRfChipSelect chipSel, tANI_U32 regNum, tANI_U32 dataMask, tANI_U32 dataShift, tANI_U32 data);
static eHalStatus rfReadDataField(tpAniSirGlobal pMac, eRfChipSelect chipSel, tANI_U32 regAddr, tANI_U32 dataMask, tANI_U32 dataShift, tANI_U32 *pData);

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
    rfWriteDataField(pMac, MIDAS_CHIP, regAddr, mask, shift, regVal);                                    \
}

#define GET_RF_FIELD(regAddr, mask, shift, pRegVal)                                                  \
{                                                                                                   \
    rfReadDataField(pMac, MIDAS_CHIP, regAddr, mask, shift, pRegVal);                                    \
}

#define MIDAS_REG_STEP	4

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


static eHalStatus rfWriteDataField(tpAniSirGlobal pMac, eRfChipSelect chipSel, tANI_U32 regAddr, tANI_U32 dataMask, tANI_U32 dataShift, tANI_U32 data)
{
    tANI_U32 regData;
    (void) chipSel;

    GET_RF_CHIP_REG(regAddr, &regData);

    regData &= ~(dataMask);
    regData |= ((data << dataShift) & dataMask);

    SET_RF_CHIP_REG(regAddr, (tANI_U16)regData);

    return (eHAL_STATUS_SUCCESS);
}

static eHalStatus rfReadDataField(tpAniSirGlobal pMac, eRfChipSelect chipSel, tANI_U32 regAddr, tANI_U32 dataMask, tANI_U32 dataShift, tANI_U32 *pData)
{
    (void) chipSel;

    GET_RF_CHIP_REG(regAddr, pData);

    *pData &= dataMask;
    *pData >>= dataShift;

    return (eHAL_STATUS_SUCCESS);
}


eHalStatus rfWriteField(tpAniSirGlobal pMac, eRfChipSelect chipSel, tANI_U32 regAddr, tANI_U32 dataMask, tANI_U32 dataShift, tANI_U32 data)
{
    (void) chipSel; //chipSel is not needed - only one possible RF Chip

    SET_RF_FIELD(regAddr, dataMask, dataShift, data);

    return (eHAL_STATUS_SUCCESS);
}

eHalStatus rfReadField(tpAniSirGlobal pMac, eRfChipSelect chipSel, tANI_U32 regAddr, tANI_U32 dataMask, tANI_U32 dataShift, tANI_U32 *pData)
{
    (void) chipSel; //chipSel is not needed - only one possible RF Chip

    GET_RF_FIELD(regAddr, dataMask, dataShift, pData);

    return (eHAL_STATUS_SUCCESS);
}

#ifndef VERIFY_HALPHY_SIMV_MODEL
//calibration support functions
eHalStatus rfTakeTemp(tpAniSirGlobal pMac, ePhyTxChains txChain, tANI_U8 nSamples, tTempADCVal *retTemperature)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;

    SET_PHY_REG(pMac->hHdd, QWLAN_TPC_ADC_CTRL_GET_ADC_REG, QWLAN_TPC_ADC_CTRL_GET_ADC_GET_ADC_MASK);
    {
        tANI_U32 i = 1000;  //number of iterations to wait for reading to complete
        tANI_U32 status = 0;

        do
        {
            GET_PHY_REG(pMac->hHdd, QWLAN_TPC_ADC_STATUS_REG, &status);
        }while(i-- > 0 && (status & QWLAN_TPC_ADC_STATUS_BUSY_T_MASK));
    
        if (status & QWLAN_TPC_ADC_STATUS_FAILED_MASK)
        {
            assert(0);
            *retTemperature = 0;
            return eHAL_STATUS_FAILURE;
        }
    }
    
    {
        tANI_U32 adc;
        GET_PHY_REG(pMac->hHdd, QWLAN_TPC_SENSED_PWR0_REG, &adc);
        
        *retTemperature = (tANI_U16)adc;
    }

    return eHAL_STATUS_SUCCESS;
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
#endif /* #ifndef VERIFY_HALPHY */

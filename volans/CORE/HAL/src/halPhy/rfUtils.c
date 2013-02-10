/** ------------------------------------------------------------------------- *
    ------------------------------------------------------------------------- *


    \file rfUtils.c

    \brief Gemini functionality

    $Id$

    Copyright (C) 2008 Qualcomm Technologies, Inc.


   ========================================================================== */


#include "sys_api.h"

#ifdef VERIFY_HALPHY_SIMV_MODEL //rfChannels in fw code
extern const tRfChannelProps rfChannels[NUM_RF_CHANNELS];
#else
const tRfChannelProps rfChannels[NUM_RF_CHANNELS] =
{
    //RF_SUBBAND_2_4_GHZ
    //freq, chan#, band
    { 2412, 1  , RF_SUBBAND_2_4_GHZ},        //RF_CHAN_1,
    { 2417, 2  , RF_SUBBAND_2_4_GHZ},        //RF_CHAN_2,
    { 2422, 3  , RF_SUBBAND_2_4_GHZ},        //RF_CHAN_3,
    { 2427, 4  , RF_SUBBAND_2_4_GHZ},        //RF_CHAN_4,
    { 2432, 5  , RF_SUBBAND_2_4_GHZ},        //RF_CHAN_5,
    { 2437, 6  , RF_SUBBAND_2_4_GHZ},        //RF_CHAN_6,
    { 2442, 7  , RF_SUBBAND_2_4_GHZ},        //RF_CHAN_7,
    { 2447, 8  , RF_SUBBAND_2_4_GHZ},        //RF_CHAN_8,
    { 2452, 9  , RF_SUBBAND_2_4_GHZ},        //RF_CHAN_9,
    { 2457, 10 , RF_SUBBAND_2_4_GHZ},        //RF_CHAN_10,
    { 2462, 11 , RF_SUBBAND_2_4_GHZ},        //RF_CHAN_11,
    { 2467, 12 , RF_SUBBAND_2_4_GHZ},        //RF_CHAN_12,
    { 2472, 13 , RF_SUBBAND_2_4_GHZ},        //RF_CHAN_13,
    { 2484, 14 , RF_SUBBAND_2_4_GHZ},        //RF_CHAN_14,
};
#endif


//channel/band/freq functions
eRfSubBand rfGetBand(tpAniSirGlobal pMac, eRfChannels chan)
{
    assert(chan < NUM_RF_CHANNELS);
    //return(rfChannels[chan].band);        restore later for 5GHz product
    return (RF_SUBBAND_2_4_GHZ);
}


eRfSubBand rfGetAGBand(tpAniSirGlobal pMac)
{
/*  //restore later for 5GHz product
        eRfSubBand  bandIndex;
    
        bandIndex = rfGetBand(pMac, rfGetCurChannel(pMac));
    
        switch (bandIndex)
        {
            case RF_SUBBAND_2_4_GHZ:
                bandIndex = RF_BAND_2_4_GHZ;
                break;
    
            default:
                phyLog(pMac, LOGE, "ERROR: band not found\n");
                return (INVALID_RF_SUBBAND);
        }
*/
    return(RF_BAND_2_4_GHZ);
}


eRfChannels rfGetCurChannel(tpAniSirGlobal pMac)
{
    return(pMac->hphy.rf.curChannel);
}

tANI_U16 rfChIdToFreqCoversion(tANI_U8 chanNum)
{
    int i;

    for (i = 0; i < NUM_RF_CHANNELS; i++)
    {
        if (rfChannels[i].channelNum == chanNum)
        {
            return rfChannels[i].targetFreq;
        }
    }

    return (0);
}

eRfChannels rfGetChannelIndex(tANI_U8 chanNum, ePhyChanBondState cbState)
{
    int i = MIN_20MHZ_RF_CHANNEL;
    int max = MAX_20MHZ_RF_CHANNEL;
    
#ifndef VERIFY_HALPHY_SIMV_MODEL
    assert(cbState == PHY_SINGLE_CHANNEL_CENTERED);
#endif
    if (cbState == PHY_SINGLE_CHANNEL_CENTERED)
    {
        i = MIN_20MHZ_RF_CHANNEL;
        max = MAX_20MHZ_RF_CHANNEL;
    }
    
    //linear search through the valid channels
    for (; (i <= max); i++)
    {
        if (rfChannels[i].channelNum == chanNum)
        {
            return ((eRfChannels)i);
        }
    }

    return INVALID_RF_CHANNEL;
}

tANI_U8 rfGetChannelIdFromIndex(eRfChannels chIndex)
{
    //assert(chIndex < NUM_RF_CHANNELS);

    return (rfChannels[chIndex].channelNum);
}


//The get the supported channel list.
//As input, pNum20MhzChannels is the size of the array of p20MhzChannels.
//Upon return, pNum20MhzChannels has the number of supported channels.
//When successfully return, p20MhzChannels contains the channel ID.
eHalStatus halPhyGetSupportedChannels( tHalHandle hHal, tANI_U8 *p20MhzChannels, int *pNum20MhzChannels,
                                       tANI_U8 *p40MhzChannels, int *pNum40MhzChannels)
{
    //tpAniSirGlobal pMac = PMAC_STRUCT( hHal );
    eHalStatus status = eHAL_STATUS_INVALID_PARAMETER;

    if( p20MhzChannels && pNum20MhzChannels )
    {
        if( *pNum20MhzChannels >= NUM_RF_CHANNELS )
        {
            int i;

            for( i = 0; i < NUM_RF_CHANNELS; i++ )
            {
                p20MhzChannels[i] = rfGetChannelIdFromIndex( i );
            }
            status = eHAL_STATUS_SUCCESS;
        }
        *pNum20MhzChannels = NUM_RF_CHANNELS;
    }

    //Ignore 40Mhz channels for now.

    return (status);
}

#ifdef HALPHY_CAL_DEBUG
//#if 1
//#define phylog(x, ...)
/** RxDco Calibration ******************************************************/

#include "pttModuleApi.h"

tANI_BOOLEAN log_regaccess = eANI_BOOLEAN_FALSE;

#define FEATURE_WLANFW_RF_ACCESS
#define MIDAS_REG_STEP      4
//#define DELAY_MICROSECONDS(x)   sirBusyWait((x) * 1000)
#define EXTEND_SIGNED_VALUE_TO_S16(value, input_sign_bit) (((tANI_S16)((value) << (15 - (input_sign_bit)))) >> (15 - (input_sign_bit)))
#define ABS(x)      (((tANI_S32)(x) < 0) ? -(x) : (x))
#define CorexLog_Print(x, ...)
#define CorexLog_Log(x, ...)

eHalStatus DEBUG_GET_PHY_REG(tpAniSirGlobal pMac, tANI_U32 addr, tANI_U32 *value)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;

    GET_PHY_REG(pMac->hHdd, addr, value);
    if (log_regaccess)
    {
        phylog(LOGE, "0x%x: 0x%x\n", addr, *value);
    }

    return retVal;
}

eHalStatus DEBUG_SET_PHY_REG(tpAniSirGlobal pMac, tANI_U32 addr, tANI_U32 value)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;

    SET_PHY_REG(pMac->hHdd, addr, value);
    DEBUG_GET_PHY_REG(pMac, addr, &value);

    return retVal;
}

eHalStatus DEBUG_GET_RF_CHIP_REG(tpAniSirGlobal pMac, tANI_U32 addr, tANI_U32 *value)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;

    GET_PHY_REG(pMac->hHdd, addr, value);
    if (log_regaccess)
    {
        phylog(LOGE, "0x%x: 0x%x\n", addr, *value);
    }

    return retVal;
}

eHalStatus DEBUG_SET_RF_CHIP_REG(tpAniSirGlobal pMac, tANI_U32 addr, tANI_U32 value)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;

    SET_PHY_REG(pMac->hHdd, addr, value);
    DEBUG_GET_PHY_REG(pMac, addr, &value);

    return retVal;
}

eHalStatus debug_rdModWriteAsicField(tpAniSirGlobal pMac, tANI_U32 addr, tANI_U32 mask, tANI_U32 offset, tANI_U32 value)
{
    rdModWrAsicField(pMac, addr, mask, offset, value);
    DEBUG_GET_PHY_REG(pMac, addr, &value);

    return eHAL_STATUS_SUCCESS;
}

void _rfSetDCOffset(tpAniSirGlobal pMac, ePhyRxChains rxChain, tANI_U8 dcoIndex, tRxDcoCorrect offset)
{
    DEBUG_SET_RF_CHIP_REG(pMac, QWLAN_RFAPB_RX_DCOC_IQ_0_REG + MIDAS_REG_STEP * dcoIndex,
                        (offset.IDcoCorrect << QWLAN_RFAPB_RX_DCOC_IQ_0_RX_DCOC_I_0_OFFSET) |
                        (offset.QDcoCorrect << QWLAN_RFAPB_RX_DCOC_IQ_0_RX_DCOC_Q_0_OFFSET)
                       );
}


void _rfGetDCOffset(tpAniSirGlobal pMac, ePhyRxChains rxChain, tANI_U8 dcoIndex, tRxDcoCorrect *offset)
{
    tANI_U32 value = 0;
    
    DEBUG_GET_RF_CHIP_REG(pMac, QWLAN_RFAPB_RX_DCOC_IQ_0_REG + MIDAS_REG_STEP * dcoIndex, &value);

    offset->IDcoCorrect = (tDcoCorrect)((value & QWLAN_RFAPB_RX_DCOC_IQ_0_RX_DCOC_I_0_MASK)
                                                            >> QWLAN_RFAPB_RX_DCOC_IQ_0_RX_DCOC_I_0_OFFSET);
    offset->QDcoCorrect = (tDcoCorrect)((value & QWLAN_RFAPB_RX_DCOC_IQ_0_RX_DCOC_Q_0_MASK)
                                                            >> QWLAN_RFAPB_RX_DCOC_IQ_0_RX_DCOC_Q_0_OFFSET);
}

#if 0
eHalStatus asicAGCReset(tpAniSirGlobal pMac)
{
    DEBUG_SET_PHY_REG(pMac, QWLAN_AGC_AGC_RESET_REG, 1);
    DEBUG_SET_PHY_REG(pMac, QWLAN_AGC_AGC_RESET_REG, 0);

    return eHAL_STATUS_SUCCESS;
}


void asicSetDisabledRxPacketTypes(tpAniSirGlobal pMac, ePhyRxDisabledPktTypes modTypes)
{
    DEBUG_SET_PHY_REG(pMac, QWLAN_AGC_DIS_MODE_REG, ((tANI_U32)modTypes));
    asicAGCReset(pMac);
}
#endif

void asicSetPhyCalMode(tpAniSirGlobal pMac, eCalMode mode)
{
#ifdef FEATURE_WLANFW_RF_ACCESS
    DEBUG_SET_PHY_REG(pMac, QWLAN_CAL_CALMODE_REG, mode);
#endif
}

void asicSetPhyCalLength(tpAniSirGlobal pMac, tANI_U16 numSamples)
{
#ifdef FEATURE_WLANFW_RF_ACCESS
    DEBUG_SET_PHY_REG(pMac, QWLAN_CAL_LENGTH_REG, numSamples);
#endif
}


void asicGetCalStatus(tpAniSirGlobal pMac, ePhyCalState *status)
{
#ifdef FEATURE_WLANFW_RF_ACCESS
    tANI_U32 reg;

    DEBUG_GET_PHY_REG(pMac, QWLAN_CAL_STATUS_REG, &reg);
    reg = (reg & CAL_STATUS_MASK) >> CAL_STATUS_OFFSET;

    *status = (ePhyCalState)reg;
#endif
}


#define DCO_NEW_CORRECTION_RANGE        255
#define DCO_BINARY_CORRECTION_RANGE     8
#define DCO_NEW_ERROR_TOLERANCE         (20)

#define DCO_GAIN_LUT_IDX            126
#define RX_DCOC_RANGE               0x5555
#define RXDCO_DCOC_CLIP_LEVEL       1022

typedef struct
{
    tANI_U8 IDcoCorrect;
    tANI_U8 QDcoCorrect;
}tRxNewDcoCorrect;

typedef struct
{
    tIQAdc error;
    tRxNewDcoCorrect dcoOffset;
}tDcoErrorCorr;

#ifdef FEATURE_WLANFW_RF_ACCESS
static void phyRxDcoSetup(tpAniSirGlobal pMac)
{
    tANI_U32 gain = 0;
    tANI_U32 regVal = 0;

    //t.csr.rfapb.rx_dcoc_en0(0xffff)
    DEBUG_SET_PHY_REG(pMac, QWLAN_RFAPB_RX_DCOC_EN0_REG, 0xffff);
    //t.csr.rfapb.rx_dcoc_en1(0xffff)
    DEBUG_SET_PHY_REG(pMac, QWLAN_RFAPB_RX_DCOC_EN1_REG, 0xffff);

    //t.csr.rfapb.rx_dcoc_ctl(rx_dcoc_idx_sel=0)
    debug_rdModWriteAsicField(pMac, QWLAN_RFAPB_RX_DCOC_CTL_REG, QWLAN_RFAPB_RX_DCOC_CTL_RX_DCOC_IDX_SEL_MASK,
            QWLAN_RFAPB_RX_DCOC_CTL_RX_DCOC_IDX_SEL_OFFSET, 0);

    //t.csr.rfapb.rx_dcoc_range0(0xffff)
    DEBUG_SET_PHY_REG(pMac, QWLAN_RFAPB_RX_DCOC_RANGE0_REG, RX_DCOC_RANGE);
    //t.csr.rfapb.rx_dcoc_range1(0xffff)
    DEBUG_SET_PHY_REG(pMac, QWLAN_RFAPB_RX_DCOC_RANGE1_REG, RX_DCOC_RANGE);
    //t.csr.rfapb.rx_dcoc_range2(0xffff)
    DEBUG_SET_PHY_REG(pMac, QWLAN_RFAPB_RX_DCOC_RANGE2_REG, RX_DCOC_RANGE);
    //t.csr.rfapb.rx_dcoc_range3(0xffff)
    DEBUG_SET_PHY_REG(pMac, QWLAN_RFAPB_RX_DCOC_RANGE3_REG, RX_DCOC_RANGE);

    //t.csr.rfapb.bbf_sar_dcoc_ctl_1(bbf_adcoc_ext_reg_over_en_i=1,
    //                                   bbf_adcoc_ext_reg_over_en_q=1,
    //                                   bbf_start_adcoc_en_i = 1,
    //                                   bbf_start_adcoc_en_q = 1)
    DEBUG_GET_PHY_REG(pMac, QWLAN_RFAPB_BBF_SAR_DCOC_CTL_1_REG, &regVal);
    regVal |= ( QWLAN_RFAPB_BBF_SAR_DCOC_CTL_1_BBF_ADCOC_EXT_REG_OVER_EN_Q_MASK |
                    QWLAN_RFAPB_BBF_SAR_DCOC_CTL_1_BBF_ADCOC_EXT_REG_OVER_EN_I_MASK |
                    QWLAN_RFAPB_BBF_SAR_DCOC_CTL_1_BBF_START_ADCOC_EN_I_MASK |
                    QWLAN_RFAPB_BBF_SAR_DCOC_CTL_1_BBF_START_ADCOC_EN_Q_MASK);
    DEBUG_SET_PHY_REG(pMac, QWLAN_RFAPB_BBF_SAR_DCOC_CTL_1_REG, regVal);

    //t.csr.rfif.gc_cfg.rx_gain_en(0)
    debug_rdModWriteAsicField(pMac, QWLAN_RFIF_GC_CFG_REG, QWLAN_RFIF_GC_CFG_RX_GAIN_EN_MASK,
                    QWLAN_RFIF_GC_CFG_RX_GAIN_EN_OFFSET, 0);

    //t.csr.agc.gainset0(override=1,gain=t.csr.agc.init_gain.gain())
    DEBUG_GET_PHY_REG(pMac, QWLAN_AGC_INIT_GAIN_REG, &gain);
    DEBUG_SET_PHY_REG(pMac, QWLAN_AGC_GAINSET0_REG, gain | QWLAN_AGC_GAINSET0_OVERRIDE_MASK); 

    //t.csr.agc.sat_step(ignore_adc_clip=1,sat_step_en=0)
    debug_rdModWriteAsicField(pMac, QWLAN_AGC_SAT_STEP_REG, QWLAN_AGC_SAT_STEP_IGNORE_ADC_CLIP_MASK,
                    QWLAN_AGC_SAT_STEP_IGNORE_ADC_CLIP_OFFSET, 1);
    debug_rdModWriteAsicField(pMac, QWLAN_AGC_SAT_STEP_REG, QWLAN_AGC_SAT_STEP_SAT_STEP_EN_MASK,
                    QWLAN_AGC_SAT_STEP_SAT_STEP_EN_OFFSET, 0);

    //t.csr.agc.rx_override(override_en=1,enrx_val=1,stby_val=1)
    DEBUG_SET_PHY_REG(pMac, QWLAN_AGC_RX_OVERRIDE_REG, QWLAN_AGC_RX_OVERRIDE_ENRX_VAL_MASK |
                                            QWLAN_AGC_RX_OVERRIDE_STBY_VAL_MASK |
                                            QWLAN_AGC_RX_OVERRIDE_OVERRIDE_EN_MASK);

    //t.csr.rfapb.rx_gain_control(rx_gc=gc_lut_idx) //127
    DEBUG_SET_PHY_REG(pMac, QWLAN_RFAPB_RX_GAIN_CONTROL_REG, DCO_GAIN_LUT_IDX);
}
#endif


#ifdef FEATURE_WLANFW_RF_ACCESS
static void phyRxDcoRestore(tpAniSirGlobal pMac)
{
    //t.csr.agc.rx_override(override_en=0,enrx_val=1,stby_val=1)
    DEBUG_SET_PHY_REG(pMac, QWLAN_AGC_RX_OVERRIDE_REG, (QWLAN_AGC_RX_OVERRIDE_ENRX_VAL_MASK |
                                            QWLAN_AGC_RX_OVERRIDE_STBY_VAL_MASK));

    //t.csr.agc.gainset0.override(0)
    DEBUG_SET_PHY_REG(pMac, QWLAN_AGC_GAINSET0_REG, 0);

    //t.csr.agc.sat_step(ignore_adc_clip=0,sat_step_en=1)
    debug_rdModWriteAsicField(pMac, QWLAN_AGC_SAT_STEP_REG, QWLAN_AGC_SAT_STEP_IGNORE_ADC_CLIP_MASK,
                    QWLAN_AGC_SAT_STEP_IGNORE_ADC_CLIP_OFFSET, 0);
    debug_rdModWriteAsicField(pMac, QWLAN_AGC_SAT_STEP_REG, QWLAN_AGC_SAT_STEP_SAT_STEP_EN_MASK,
                    QWLAN_AGC_SAT_STEP_SAT_STEP_EN_OFFSET, 1);
    //t.csr.rfif.gc_cfg.rx_gain_en(1)
    debug_rdModWriteAsicField(pMac, QWLAN_RFIF_GC_CFG_REG, QWLAN_RFIF_GC_CFG_RX_GAIN_EN_MASK,
                    QWLAN_RFIF_GC_CFG_RX_GAIN_EN_OFFSET, 1);
}
#endif

#define MAX_WAIT_TO_COUNT   1000
#ifdef FEATURE_WLANFW_RF_ACCESS
static void measureNoShuntDco(tpAniSirGlobal pMac, tANI_S16 *dco_real, tANI_S16 *dco_imag)
{
    tANI_U32 to_cnt = 0, curr_cnt = 0 ,new_cnt = 0;
    tANI_U32 temp = 0;

    DEBUG_GET_PHY_REG(pMac, QWLAN_AGC_COARSE_DCO_UPDATE_COUNTER_REG, &curr_cnt);

    do {

        DELAY_MICROSECONDS(10);
        DEBUG_GET_PHY_REG(pMac, QWLAN_AGC_COARSE_DCO_UPDATE_COUNTER_REG, &new_cnt);
    } while ((curr_cnt == new_cnt) && (++to_cnt < MAX_WAIT_TO_COUNT));

    if (to_cnt == MAX_WAIT_TO_COUNT)
        hv_printLog("to_cnt == WAIT COUNT, cur_cnt = %d, new_cnt = %d\n", curr_cnt, new_cnt);
        
    hv_printLog("to_cnt == WAIT COUNT, cur_cnt = %d, new_cnt = %d\n", curr_cnt, new_cnt);

    DEBUG_GET_PHY_REG(pMac, QWLAN_AGC_COARSE_DCO_CHAIN0_REAL_REG, &temp);
    hv_printLog("1. temp = %d\n", temp);
    hv_printLog("twocomp(temp) = %d\n", EXTEND_SIGNED_VALUE_TO_S16(temp, 9));

    *dco_real = (EXTEND_SIGNED_VALUE_TO_S16(temp, 9) << 1); //2's compliment * 2
    hv_printLog("*dco_real = %d\n", *dco_real);

    DEBUG_GET_PHY_REG(pMac, QWLAN_AGC_COARSE_DCO_CHAIN0_IMAG_REG, &temp);
    hv_printLog("2.temp = %d\n", temp);

    *dco_imag = (EXTEND_SIGNED_VALUE_TO_S16(temp, 9) << 1); //2's compliment * 2
    hv_printLog("*dco_imag = %d\n", *dco_imag);
}
#endif

#define NUM_CORR_BITS   8
tANI_U8 limitDcoCorr(tANI_S16 dcVal)
{
    if (dcVal < 0)
        return 0;

    if (dcVal > ((1 << NUM_CORR_BITS) - 1))
        return (tANI_U8)((1 << NUM_CORR_BITS) - 1);

    return (tANI_U8)dcVal;
}

#ifdef FEATURE_WLANFW_RF_ACCESS
static void writeDcoLut(tpAniSirGlobal pMac, tANI_U32 dcoLutIdx, tANI_S16 dcReal, tANI_S16 dcImag)
{
    tRxDcoCorrect dcoCorrect;

    dcoCorrect.IDcoCorrect = limitDcoCorr(dcReal);
    dcoCorrect.QDcoCorrect = limitDcoCorr(dcImag); 

    //Chain 0
    _rfSetDCOffset(pMac, PHY_RX_CHAIN_0, dcoLutIdx, dcoCorrect);
}

static void readDcoLut(tpAniSirGlobal pMac, tANI_U32 dcoLutIdx, tANI_U8 *IDcoCorrect, tANI_U8 *QDcoCorrect)
{
    tRxDcoCorrect dcoCorrect;

    _rfGetDCOffset(pMac, PHY_RX_CHAIN_0, dcoLutIdx, &dcoCorrect);

    *IDcoCorrect = dcoCorrect.IDcoCorrect;
    *QDcoCorrect = dcoCorrect.QDcoCorrect;
}

static void writeDcocAndMeasure(tpAniSirGlobal pMac, tANI_U8 dcoLutIdx, tDcoErrorCorr *dcoErrCorr)
{
    writeDcoLut(pMac, dcoLutIdx, dcoErrCorr->dcoOffset.IDcoCorrect, dcoErrCorr->dcoOffset.QDcoCorrect);

    DELAY_MICROSECONDS(40);  /* 40us */ 

    measureNoShuntDco(pMac, &dcoErrCorr->error.I, &dcoErrCorr->error.Q);
}

static void overrideRffeGain(tpAniSirGlobal pMac, tANI_U32 dcoLutIdx)
{
    tANI_U8 max_bq_pga_gain = 0xcf;

    DEBUG_SET_PHY_REG(pMac, QWLAN_RFAPB_RX_GC_0_REG + DCO_GAIN_LUT_IDX * 4, max_bq_pga_gain | (dcoLutIdx << 8));
}

static void updateError(tDcoErrorCorr *errorCorr, tDcoErrorCorr *negErrorCorr, tDcoErrorCorr *posErrorCorr)
{
    if(errorCorr->error.I < 0)
    {
        negErrorCorr->error.I = errorCorr->error.I;
        negErrorCorr->dcoOffset.IDcoCorrect = errorCorr->dcoOffset.IDcoCorrect;
    }
    else
    {
        posErrorCorr->error.I = errorCorr->error.I;
        posErrorCorr->dcoOffset.IDcoCorrect = errorCorr->dcoOffset.IDcoCorrect;
    }

    if(errorCorr->error.Q < 0)
    {
        negErrorCorr->error.Q = errorCorr->error.Q;
        negErrorCorr->dcoOffset.QDcoCorrect = errorCorr->dcoOffset.QDcoCorrect;
    }
    else
    {
        posErrorCorr->error.Q = errorCorr->error.Q;
        posErrorCorr->dcoOffset.QDcoCorrect = errorCorr->dcoOffset.QDcoCorrect;
    }
}


static void searchBinary(tpAniSirGlobal pMac, tANI_U8 dcoLutIdx, tANI_S16 *corrI, tANI_S16 *corrQ)
{
    tANI_U32 iteration = 0;
    tDcoErrorCorr dcoCorrErr, negErrorCorr, posErrorCorr;
    tIQAdc minError;  //init to max error values
    tANI_U8 minICorrect = 0;
    tANI_U8 minQCorrect = 0;

    minError.I = MSK_10;
    minError.Q = MSK_10;

    //identify +ve error and -ve error
    {
        dcoCorrErr.dcoOffset.IDcoCorrect = (tANI_U8)0;
        dcoCorrErr.dcoOffset.QDcoCorrect = (tANI_U8)0;

        writeDcocAndMeasure(pMac, dcoLutIdx, &dcoCorrErr);

        updateError(&dcoCorrErr, &negErrorCorr, &posErrorCorr);

        dcoCorrErr.dcoOffset.IDcoCorrect = (tANI_U8)DCO_NEW_CORRECTION_RANGE;
        dcoCorrErr.dcoOffset.QDcoCorrect = (tANI_U8)DCO_NEW_CORRECTION_RANGE;

        writeDcocAndMeasure(pMac, dcoLutIdx, &dcoCorrErr);

        updateError(&dcoCorrErr, &negErrorCorr, &posErrorCorr);
    }

    //Calibrate DCO for I & Q rails
    while (iteration++ < DCO_BINARY_CORRECTION_RANGE)
    {
        dcoCorrErr.dcoOffset.IDcoCorrect = (negErrorCorr.dcoOffset.IDcoCorrect + posErrorCorr.dcoOffset.IDcoCorrect)/2;
        dcoCorrErr.dcoOffset.QDcoCorrect = (negErrorCorr.dcoOffset.QDcoCorrect + posErrorCorr.dcoOffset.QDcoCorrect)/2;

        writeDcocAndMeasure(pMac, dcoLutIdx, &dcoCorrErr);

#ifndef QWLANFW_SUPPRESS_PRINT_PHY  // DEBUG
        CorexLog_Print(WLANFW_MODULE_MAC, WLANFW_LOG_3, "iter=%d phy RX%d CorrI=%d CorrQ=%d DCO_ERR:I=%d Q=%d\r\n", iteration, 0,
                dcoCorrErr.dcoOffset.IDcoCorrect, dcoCorrErr.dcoOffset.QDcoCorrect, dcoCorrErr.error.I, dcoCorrErr.error.Q);
#else
        CorexLog_Log(WLANFW_MODULE_PHY,
                     WLANFW_LOG_3,
                     QWLANFW_LOG_EVENT_TYPE_MISC,
                     QWLANFW_LOG_CODE_PHY_BIN_DCO_CAL_RX_CHAIN_IQ_VALS,
                     6,
                     iteration,
                     0,
                     dcoCorrErr.dcoOffset.IDcoCorrect,
                     dcoCorrErr.dcoOffset.QDcoCorrect,
                     dcoCorrErr.error.I,
                     dcoCorrErr.error.Q);
#endif

        if (ABS(dcoCorrErr.error.I) < ABS(minError.I))
        {
            minError.I = dcoCorrErr.error.I;
            minICorrect = (tANI_U8)dcoCorrErr.dcoOffset.IDcoCorrect;
        }

        if (ABS(dcoCorrErr.error.Q) < ABS(minError.Q))
        {
            minError.Q = dcoCorrErr.error.Q;
            minQCorrect = (tANI_U8)dcoCorrErr.dcoOffset.QDcoCorrect;
        }

//        if ((GET_MAG(minError.I) <= DCO_NEW_ERROR_TOLERANCE) &&
//            (GET_MAG(minError.Q) <= DCO_NEW_ERROR_TOLERANCE)) break;

        updateError(&dcoCorrErr, &negErrorCorr, &posErrorCorr);
    }

    if (iteration == DCO_NEW_CORRECTION_RANGE)
    {
#ifndef QWLANFW_SUPPRESS_PRINT_PHY  // DEBUG
        CorexLog_Print(WLANFW_MODULE_MAC, WLANFW_LOG_ALWAYS,"DCO CAL FAILED TO CONVERGE\r\n");
#else
        CorexLog_Log(WLANFW_MODULE_PHY,
                     WLANFW_LOG_ALWAYS,
                     QWLANFW_LOG_EVENT_TYPE_MISC,
                     QWLANFW_LOG_CODE_DCO_CAL_FAILED_TO_CONVERGE,
                     0);
#endif
    }

    {
        tRxNewDcoCorrect setOffset;

        setOffset.IDcoCorrect = minICorrect;
        setOffset.QDcoCorrect = minQCorrect;

#ifndef QWLANFW_SUPPRESS_PRINT_PHY  // DEBUG
        CorexLog_Print(WLANFW_MODULE_MAC, WLANFW_LOG_DEBUG, "Loading minError values: I=%d at error=%d  Q=%d at error %d\r\n",
                            minICorrect, minError.I, minQCorrect, minError.Q);
#else
        CorexLog_Log(WLANFW_MODULE_PHY,
                     WLANFW_LOG_DEBUG,
                     QWLANFW_LOG_EVENT_TYPE_MISC,
                     QWLANFW_LOG_CODE_DCO_CAL_LOADING_MIN_ERROR_VALUES,
                     4,
                     minICorrect,
                     minError.I,
                     minQCorrect,
                     minError.Q);
#endif
        *corrI = setOffset.IDcoCorrect;
        *corrQ = setOffset.QDcoCorrect;

        writeDcoLut(pMac, dcoLutIdx, *corrI, *corrQ);
    }
}
#endif


#define MAX_DCO_LUT_RANGE       32
#define NUM_DCO_LUT_INDICES     32
void phyRxDcoCal(tpAniSirGlobal pMac, tANI_U8 loadDcoOnly)
{
#ifdef FEATURE_WLANFW_RF_ACCESS
    tANI_U32 thSignalLow = 0;
    tANI_S16 corrI = 0, corrQ = 0;
    tANI_U8 idx, maxIdx = MAX_DCO_LUT_RANGE - 1;

    phyRxDcoSetup(pMac);

    // max_idx = max(dco_lut_range)
    
    // th_signal_low = t.csr.agc.th_signal_low.threshold()
    DEBUG_GET_PHY_REG(pMac, QWLAN_AGC_TH_SIGNAL_LOW_REG, &thSignalLow);
    // t.csr.agc.th_signal_low(0)
    DEBUG_SET_PHY_REG(pMac, QWLAN_AGC_TH_SIGNAL_LOW_REG, 0);

    overrideRffeGain(pMac, maxIdx);
    searchBinary(pMac, maxIdx, &corrI, &corrQ);

    for (idx = 0; idx < maxIdx; idx++)
    {
        writeDcoLut(pMac, idx, corrI, corrQ);
    }

    // t.csr.agc.th_signal_low.threshold(th_signal_low)
    DEBUG_SET_PHY_REG(pMac, QWLAN_AGC_TH_SIGNAL_LOW_REG, thSignalLow);

    phyRxDcoRestore(pMac);
#endif
}


#if 1
/**************** IM2 Calibration *********************************/

typedef struct
{
    tANI_BOOLEAN I;
    tANI_BOOLEAN Q;
} tBoolIQ;

typedef struct
{
    tDcoErrorCorr errorCorrLo;
    tDcoErrorCorr errorCorrHi;
    tBoolIQ       on2sides_prv;
    tBoolIQ       done;
} tUpdateIQ;


#ifdef FEATURE_WLANFW_RF_ACCESS
static void asicWaitCalDone(tpAniSirGlobal pMac, ePhyCalState *status)
{
    int i = 0;
    int x = 0;

    hv_printLog("Wait for the measurement\n");
    do
    {
        DELAY_MICROSECONDS(100); // TODO: tune this later for optimization

        asicGetCalStatus(pMac, status);
        
        
        if (*status == PHY_CAL_DONE)
        {
            hv_printLog("Measurement done\n");
            return;
        }
        else
        {            
            if (*status == PHY_CAL_BUSY)
            {
                i--;    //let it continue one more time
                //TODO: phyLog(LOGE, "WARN: PHY_CAL_BUSY - not enough time to complete measurement\n");
            }
            else if (*status == PHY_CAL_IDLE)
            {
                //note that this should never happen, but it is occasionally, so 
                tANI_U32 reg;

                hv_printLog("Is cal idle????\n");
                DEBUG_GET_PHY_REG(pMac, CAL_MODE_REG, &reg);
                if (reg < PHY_CAL_UNFINISHED)
                {
                    //TODO: phyLog(LOGE, "ERROR: PHY_CAL_IDLE when mode=%s\n", &modeStr[reg][0]);
                }

                if (x < 5)
                {
                    DEBUG_SET_PHY_REG(pMac, QWLAN_CAL_MEASURE_REG, QWLAN_CAL_MEASURE_CMD_EMEASURE); //force another measurement
                    i = 0;
                    x++;
                }
                else
                {
                    return;
                }
            }
            else if (*status == PHY_CAL_ERROR)
            {
                //TODO: phyLog(LOGE, "ERROR: asicWaitCalDone: Status %X\n", status);
            }
        }

        i++;
    }
    while (i < 10); // 500 us total wait loop.

    *status = PHY_CAL_UNFINISHED;
}
#endif 

void asicPerformCalMeasurement(tpAniSirGlobal pMac)
{
#ifdef FEATURE_WLANFW_RF_ACCESS
    tANI_U32 reg;
    ePhyCalState status;


    DEBUG_GET_PHY_REG(pMac, CAL_MODE_REG, &reg);
    //DEBUG_SET_PHY_REG(pMac, CAL_MODE_REG, PHY_CAL_MODE_INITDCCAL);    //just for test
    
    reg &= CAL_MODE_MASK;
    assert(reg != 0);

    DEBUG_SET_PHY_REG(pMac, QWLAN_CAL_MEASURE_REG, QWLAN_CAL_MEASURE_CMD_EMEASURE);

    // DUMP_PHY_REG(CAL_MODE_REG);
    // DUMP_PHY_REG(CAL_LENGTH_REG);
    // DUMP_PHY_REG(CAL_MEASURE_REG);
    // DUMP_PHY_REG(CAL_CHAIN_SEL_REG);
    // DUMP_PHY_REG(RXACLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG);
    // DUMP_PHY_REG(RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG);
    // DUMP_PHY_REG(TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG);
    // DUMP_PHY_REG(AGC_CONFIG_XBAR_REG);
    // DUMP_PHY_REG(AGC_RX_OVERRIDE_REG);
    // DUMP_PHY_REG(TXCTL_DAC_CONTROL_REG);
    
    //TODO: phyLog(LOG2, "Starting Cal Measurement: %s ", &(modeStr[reg][0]));

    
    asicWaitCalDone(pMac, &status);
    {
        //TODO: phyLog(LOG2, "Meas Status = %d\n", status);

        if (
            (status == PHY_CAL_UNFINISHED) ||
            (status == PHY_CAL_IDLE) ||
            (status == PHY_CAL_BUSY) ||
            (status == PHY_CAL_ERROR)
           )
        {
            hv_printLog("Measurement was not completed\n");
#ifndef QWLANFW_SUPPRESS_PRINT
            CorexLog_Print(WLANFW_MODULE_PHY, WLANFW_LOG_ALWAYS, "<asicWaitCalDone>: Measurement did not finish after several\n" 
                                                                 "repeated attempts. status: %d\r\n", status);
#else
            //TODO: phyLog(LOGW, "WARN: Measurement did not finish after several repeated attempts!\n");
            CorexLog_Log(WLANFW_MODULE_PHY, 
                         WLANFW_LOG_ALWAYS, 
                         QWLANFW_LOG_EVENT_TYPE_MISC, 
                         QWLANFW_LOG_CODE_ASIC_PERF_CAL_MEAS_DID_NOT_FINISH,
                         1,
                         status);
#endif
        }
    }
    
    //DUMP_PHY_REG(CAL_STATUS_REG);
#endif    
}

#define PHY_I_RAIL  0
#define PHY_Q_RAIL  1

void asicGetCalADCSamples(tpAniSirGlobal pMac, tIQAdc *dco)
{
#ifdef FEATURE_WLANFW_RF_ACCESS
    tANI_U32 regVal;

    DEBUG_SET_PHY_REG(pMac, QWLAN_CAL_CHAIN_SEL_REG, ((tANI_U32 )PHY_I_RAIL << QWLAN_CAL_CHAIN_SEL_IQSEL_OFFSET));
    asicPerformCalMeasurement(pMac);

    DEBUG_GET_PHY_REG(pMac, QWLAN_CAL_DCOFFSET_REG, &regVal);
    dco->I = (tANI_S16)EXTEND_SIGNED_VALUE_TO_S16(regVal, 10);

    DEBUG_SET_PHY_REG(pMac, QWLAN_CAL_CHAIN_SEL_REG, ((tANI_U32 )PHY_Q_RAIL << QWLAN_CAL_CHAIN_SEL_IQSEL_OFFSET));
    DEBUG_GET_PHY_REG(pMac, QWLAN_CAL_DCOFFSET_REG, &regVal);
    dco->Q = (tANI_S16)EXTEND_SIGNED_VALUE_TO_S16(regVal, 10);
    
    //DEBUG_SET_PHY_REG(pMac, CAL_MODE_REG, CAL_MODE_ENORMAL);
#endif
}

static void enableIm2ToneGen(tpAniSirGlobal pMac, tANI_BOOLEAN inBand)
{
    log_regaccess = eANI_BOOLEAN_TRUE;
    if (inBand)
    {
        DEBUG_SET_RF_CHIP_REG(pMac, QWLAN_RFAPB_RXFE_TONEGEN_0_REG, 126);
        DEBUG_SET_RF_CHIP_REG(pMac, QWLAN_RFAPB_RXFE_TONEGEN_11_REG, 72);
    }
    else
    {
        DEBUG_SET_RF_CHIP_REG(pMac, QWLAN_RFAPB_RXFE_TONEGEN_0_REG, 99);
        debug_rdModWriteAsicField(pMac, QWLAN_RFAPB_RXFE_TONEGEN_5_REG,
                                        QWLAN_RFAPB_RXFE_TONEGEN_5_TG_VCO_BAND_MASK,
                                        QWLAN_RFAPB_RXFE_TONEGEN_5_TG_VCO_BAND_OFFSET, 0);
        DEBUG_SET_RF_CHIP_REG(pMac, QWLAN_RFAPB_RXFE_TONEGEN_11_REG, 73);
    }
        
    DEBUG_SET_RF_CHIP_REG(pMac, QWLAN_RFAPB_RXFE_TONEGEN_9_REG, 7);
    debug_rdModWriteAsicField(pMac, QWLAN_RFAPB_RXFE_TONEGEN_5_REG,
                                    QWLAN_RFAPB_RXFE_TONEGEN_5_TG_IVCOBK_MASK,
                                    QWLAN_RFAPB_RXFE_TONEGEN_5_TG_IVCOBK_OFFSET, 64);
    debug_rdModWriteAsicField(pMac, QWLAN_RFAPB_RXFE_TONEGEN_12_REG,
                                    QWLAN_RFAPB_RXFE_TONEGEN_12_TG_COMP_EN_MASK,
                                    QWLAN_RFAPB_RXFE_TONEGEN_12_TG_COMP_EN_OFFSET, 1);
    debug_rdModWriteAsicField(pMac, QWLAN_RFAPB_RXFE_TONEGEN_12_REG,
                                    QWLAN_RFAPB_RXFE_TONEGEN_12_TG_ATTEN_TWK_MASK,
                                    QWLAN_RFAPB_RXFE_TONEGEN_12_TG_ATTEN_TWK_OFFSET, 1);
    debug_rdModWriteAsicField(pMac, QWLAN_RFAPB_RXFE_TONEGEN_10_REG,
                                    QWLAN_RFAPB_RXFE_TONEGEN_10_TG_CP_GAIN_MASK,
                                    QWLAN_RFAPB_RXFE_TONEGEN_10_TG_CP_GAIN_OFFSET, 15);
    debug_rdModWriteAsicField(pMac, QWLAN_RFAPB_RXFE_TONEGEN_10_REG,
                                    QWLAN_RFAPB_RXFE_TONEGEN_10_TG_KVCO_CTRL_MASK,
                                    QWLAN_RFAPB_RXFE_TONEGEN_10_TG_KVCO_CTRL_OFFSET, 3);
    DEBUG_SET_RF_CHIP_REG(pMac, QWLAN_RFAPB_RXFE_TONEGEN_13_REG, 0);
    debug_rdModWriteAsicField(pMac, QWLAN_RFAPB_RXFE_TONEGEN_1_REG,
                                    QWLAN_RFAPB_RXFE_TONEGEN_1_TG_EN_MASK,
                                    QWLAN_RFAPB_RXFE_TONEGEN_1_TG_EN_OFFSET, 1);
    debug_rdModWriteAsicField(pMac, QWLAN_RFAPB_RXFE_LNA_BLANK_BIST_CTL_REG,
                                    QWLAN_RFAPB_RXFE_LNA_BLANK_BIST_CTL_RXFE_LNA_BLANK_BIST_OVRRD_MASK,
                                    QWLAN_RFAPB_RXFE_LNA_BLANK_BIST_CTL_RXFE_LNA_BLANK_BIST_OVRRD_OFFSET, 1);
    debug_rdModWriteAsicField(pMac, QWLAN_RFAPB_RXFE_LNA_BLANK_BIST_CTL_REG,
                                    QWLAN_RFAPB_RXFE_LNA_BLANK_BIST_CTL_RXFE_LNA_DUMMY_RFBIST_EN_OVRD_MASK,
                                    QWLAN_RFAPB_RXFE_LNA_BLANK_BIST_CTL_RXFE_LNA_DUMMY_RFBIST_EN_OVRD_OFFSET, 1);
    //log_regaccess = eANI_BOOLEAN_FALSE;
}

static void disableIm2ToneGen(tpAniSirGlobal pMac, tANI_U8 inBand)
{
    log_regaccess = eANI_BOOLEAN_TRUE;
    debug_rdModWriteAsicField(pMac, QWLAN_RFAPB_RXFE_TONEGEN_1_REG,
                                    QWLAN_RFAPB_RXFE_TONEGEN_1_TG_EN_MASK,
                                    QWLAN_RFAPB_RXFE_TONEGEN_1_TG_EN_OFFSET, 0);
    debug_rdModWriteAsicField(pMac, QWLAN_RFAPB_RXFE_LNA_BLANK_BIST_CTL_REG,
                                    QWLAN_RFAPB_RXFE_LNA_BLANK_BIST_CTL_RXFE_LNA_BLANK_BIST_OVRRD_MASK,
                                    QWLAN_RFAPB_RXFE_LNA_BLANK_BIST_CTL_RXFE_LNA_BLANK_BIST_OVRRD_OFFSET, 0);
    debug_rdModWriteAsicField(pMac, QWLAN_RFAPB_RXFE_LNA_BLANK_BIST_CTL_REG,
                                    QWLAN_RFAPB_RXFE_LNA_BLANK_BIST_CTL_RXFE_LNA_DUMMY_RFBIST_EN_OVRD_MASK,
                                    QWLAN_RFAPB_RXFE_LNA_BLANK_BIST_CTL_RXFE_LNA_DUMMY_RFBIST_EN_OVRD_OFFSET, 0);
    //log_regaccess = eANI_BOOLEAN_FALSE;
}

//Modified
static void phyRxIm2Setup(tpAniSirGlobal pMac)
{
    //t.csr.rfapb.rx_dcoc_en0(0xffff)
    DEBUG_SET_PHY_REG(pMac, QWLAN_RFAPB_RX_DCOC_EN0_REG, 0xffff);
    //t.csr.rfapb.rx_dcoc_en1(0xffff)
    DEBUG_SET_PHY_REG(pMac, QWLAN_RFAPB_RX_DCOC_EN1_REG, 0xffff);

    //t.csr.rfapb.rx_dcoc_ctl(rx_dcoc_idx_sel=0)
    debug_rdModWriteAsicField(pMac, QWLAN_RFAPB_RX_DCOC_CTL_REG, QWLAN_RFAPB_RX_DCOC_CTL_RX_DCOC_IDX_SEL_MASK,
            QWLAN_RFAPB_RX_DCOC_CTL_RX_DCOC_IDX_SEL_OFFSET, 0);

    //t.csr.rfapb.rx_dcoc_range0(0x5555)
    DEBUG_SET_PHY_REG(pMac, QWLAN_RFAPB_RX_DCOC_RANGE0_REG, 0x5555);
    //t.csr.rfapb.rx_dcoc_range1(0x5555)
    DEBUG_SET_PHY_REG(pMac, QWLAN_RFAPB_RX_DCOC_RANGE1_REG, 0x5555);
    //t.csr.rfapb.rx_dcoc_range2(0x5555)
    DEBUG_SET_PHY_REG(pMac, QWLAN_RFAPB_RX_DCOC_RANGE2_REG, 0x5555);
    //t.csr.rfapb.rx_dcoc_range3(0x5555)
    DEBUG_SET_PHY_REG(pMac, QWLAN_RFAPB_RX_DCOC_RANGE3_REG, 0x5555);

    //t.csr.rfapb.bbf_sar_dcoc_ctl_1(bbf_adcoc_ext_reg_over_en_i=1,
    //                                   bbf_adcoc_ext_reg_over_en_q=1,
    //                                   bbf_start_adcoc_en_i = 1,
    //                                   bbf_start_adcoc_en_q = 1)
    debug_rdModWriteAsicField(pMac, QWLAN_RFAPB_BBF_SAR_DCOC_CTL_1_REG, QWLAN_RFAPB_BBF_SAR_DCOC_CTL_1_BBF_ADCOC_EXT_REG_OVER_EN_I_MASK,
                    QWLAN_RFAPB_BBF_SAR_DCOC_CTL_1_BBF_ADCOC_EXT_REG_OVER_EN_I_OFFSET, 1);
    debug_rdModWriteAsicField(pMac,  QWLAN_RFAPB_BBF_SAR_DCOC_CTL_1_REG, QWLAN_RFAPB_BBF_SAR_DCOC_CTL_1_BBF_ADCOC_EXT_REG_OVER_EN_Q_MASK,
                    QWLAN_RFAPB_BBF_SAR_DCOC_CTL_1_BBF_ADCOC_EXT_REG_OVER_EN_Q_OFFSET, 1);
    debug_rdModWriteAsicField(pMac, QWLAN_RFAPB_BBF_SAR_DCOC_CTL_1_REG, QWLAN_RFAPB_BBF_SAR_DCOC_CTL_1_BBF_START_ADCOC_EN_I_MASK,
                    QWLAN_RFAPB_BBF_SAR_DCOC_CTL_1_BBF_START_ADCOC_EN_I_OFFSET, 1);
    debug_rdModWriteAsicField(pMac, QWLAN_RFAPB_BBF_SAR_DCOC_CTL_1_REG, QWLAN_RFAPB_BBF_SAR_DCOC_CTL_1_BBF_START_ADCOC_EN_Q_MASK,
                    QWLAN_RFAPB_BBF_SAR_DCOC_CTL_1_BBF_START_ADCOC_EN_Q_OFFSET, 1);

    //t.csr.rfif.gc_cfg.rx_gain_en(0)
    debug_rdModWriteAsicField(pMac, QWLAN_RFIF_GC_CFG_REG, QWLAN_RFIF_GC_CFG_RX_GAIN_EN_MASK,
                    QWLAN_RFIF_GC_CFG_RX_GAIN_EN_OFFSET, 0);

    //asicSetDisabledRxPacketTypes(PHY_RX_DISABLE_ALL_TYPES);

    //t.csr.agc.rx_override(override_en=1,enrx_val=1,stby_val=1)
    DEBUG_SET_PHY_REG(pMac, QWLAN_AGC_RX_OVERRIDE_REG, QWLAN_AGC_RX_OVERRIDE_ENRX_VAL_MASK |
                                            QWLAN_AGC_RX_OVERRIDE_STBY_VAL_MASK |
                                            QWLAN_AGC_RX_OVERRIDE_OVERRIDE_EN_MASK);

    DEBUG_SET_PHY_REG(pMac, QWLAN_CAL_CHN0_DCO_OVR_REG, 0);
    DEBUG_SET_PHY_REG(pMac, QWLAN_CAL_OVERRIDE_REG, 1);

    //updte the mixer gate bias setting from Roger
    //im2 dac resistor and voltage settings
    debug_rdModWriteAsicField(pMac, QWLAN_RFAPB_RX_IM2_VCM1_REG, QWLAN_RFAPB_RX_IM2_VCM1_IM2_VGF_MASK,
                    QWLAN_RFAPB_RX_IM2_VCM1_IM2_VGF_OFFSET, 3);
    DEBUG_SET_PHY_REG(pMac, QWLAN_RFAPB_RX_IM2_I_CFG1_REG, 5);
    DEBUG_SET_PHY_REG(pMac, QWLAN_RFAPB_RX_IM2_Q_CFG1_REG, 5);
    //im2 dac mode and vref settings
    debug_rdModWriteAsicField(pMac, QWLAN_RFAPB_RX_IM2_EN_CTL_REG, QWLAN_RFAPB_RX_IM2_EN_CTL_IM2_TEST_MASK,
                    QWLAN_RFAPB_RX_IM2_EN_CTL_IM2_TEST_OFFSET, 0);

    //added by Oliver
    //config AGC and cal mode before hand
    //asicSetDisabledRxPacketTypes(pMac, PHY_RX_DISABLE_ALL_TYPES);
    DEBUG_SET_PHY_REG(pMac, QWLAN_AGC_DIS_MODE_REG, 3);
    asicSetPhyCalMode(pMac, PHY_CAL_MODE_INITDCCAL);
    asicSetPhyCalLength(pMac, 1023);
}

//Modified
static void phyRxIm2Restore(tpAniSirGlobal pMac)
{
    //t.csr.rfif.gc_cfg.rx_gain_en(1)
    debug_rdModWriteAsicField(pMac, QWLAN_RFIF_GC_CFG_REG, QWLAN_RFIF_GC_CFG_RX_GAIN_EN_MASK,
                    QWLAN_RFIF_GC_CFG_RX_GAIN_EN_OFFSET, 1);
    //t.csr.agc.rx_override(override_en=0,enrx_val=1,stby_val=1)
    DEBUG_SET_PHY_REG(pMac, QWLAN_AGC_RX_OVERRIDE_REG, (QWLAN_AGC_RX_OVERRIDE_ENRX_VAL_MASK |
                                            QWLAN_AGC_RX_OVERRIDE_STBY_VAL_MASK));
    DEBUG_SET_PHY_REG(pMac, QWLAN_RFAPB_RX_DCOC_RANGE0_REG, RX_DCOC_RANGE);
    DEBUG_SET_PHY_REG(pMac, QWLAN_RFAPB_RX_DCOC_RANGE1_REG, RX_DCOC_RANGE);
    DEBUG_SET_PHY_REG(pMac, QWLAN_RFAPB_RX_DCOC_RANGE2_REG, RX_DCOC_RANGE);
    DEBUG_SET_PHY_REG(pMac, QWLAN_RFAPB_RX_DCOC_RANGE3_REG, RX_DCOC_RANGE);

    if (RF_CHIP_VERSION(RF_CHIP_ID_VOLANS2))
    {
        asicSetPhyCalLength(pMac, 31);
    }
    else
    {
        asicSetPhyCalLength(pMac, 63);
    }
    asicSetPhyCalMode(pMac, PHY_CAL_MODE_NORMAL);

    DEBUG_SET_PHY_REG(pMac, QWLAN_CAL_OVERRIDE_REG, 0);

    //asicSetDisabledRxPacketTypes(pMac, PHY_RX_DISABLE_NONE);
    DEBUG_SET_PHY_REG(pMac, QWLAN_AGC_DIS_MODE_REG, 0);
}

static void measureDco(tpAniSirGlobal pMac, tANI_S16 *meas_i, tANI_S16 *meas_q)
{
    tIQAdc dco;

    asicGetCalADCSamples(pMac, &dco);

    *meas_i = dco.I;
    *meas_q = dco.Q;
}

//Modified
void rfSetIm2Correct(tpAniSirGlobal pMac, tRxIm2Correct im2Corr)
{
#ifdef FEATURE_WLANFW_RF_ACCESS
    //assert(im2Corr.ICorrect < 128);
    //assert(im2Corr.QCorrect < 128);

    //im2Corr.ICorrect = (tIm2Correct)(signMagnitude(im2Corr.ICorrect, 6) << 1);
    //im2Corr.QCorrect = (tIm2Correct)(signMagnitude(im2Corr.QCorrect, 6) << 1);

    //SET_RF_FIELD(QWLAN_RFAPB_RX_IM2_I_CFG0_REG,
    //                QWLAN_RFAPB_RX_IM2_I_CFG0_IM2_VCAL_IDAC_I_MASK,
    //                QWLAN_RFAPB_RX_IM2_I_CFG0_IM2_VCAL_IDAC_I_OFFSET,
    //                im2Corr.ICorrect);
    //SET_RF_FIELD(QWLAN_RFAPB_RX_IM2_Q_CFG0_REG,
    //                QWLAN_RFAPB_RX_IM2_Q_CFG0_IM2_VCAL_IDAC_Q_MASK,
    //                QWLAN_RFAPB_RX_IM2_Q_CFG0_IM2_VCAL_IDAC_Q_OFFSET,
    //                im2Corr.QCorrect);
    log_regaccess = eANI_BOOLEAN_TRUE;
    DEBUG_SET_RF_CHIP_REG(pMac, QWLAN_RFAPB_RX_IM2_SPARE0_REG, im2Corr.ICorrect);
    DEBUG_SET_RF_CHIP_REG(pMac, QWLAN_RFAPB_RX_IM2_SPARE1_REG, im2Corr.QCorrect);
    //log_regaccess = eANI_BOOLEAN_FALSE;
#endif
}

//Modified
void rfGetIm2Correct(tpAniSirGlobal pMac, tRxIm2Correct *im2Corr)
{
#ifdef FEATURE_WLANFW_RF_ACCESS
    tANI_U32 value = 0;

    //DEBUG_GET_RF_CHIP_REG(pMac, QWLAN_RFAPB_RX_IM2_I_CFG0_REG, &value);
    //im2Corr->ICorrect = (tIm2Correct) ((value & QWLAN_RFAPB_RX_IM2_I_CFG0_IM2_VCAL_IDAC_I_MASK)
    //                                                    >> QWLAN_RFAPB_RX_IM2_I_CFG0_IM2_VCAL_IDAC_I_OFFSET);

    //DEBUG_GET_RF_CHIP_REG(pMac, QWLAN_RFAPB_RX_IM2_Q_CFG0_REG, &value);
    //im2Corr->ICorrect = (tIm2Correct) ((value & QWLAN_RFAPB_RX_IM2_Q_CFG0_IM2_VCAL_IDAC_Q_MASK)
    //                                                    >> QWLAN_RFAPB_RX_IM2_Q_CFG0_IM2_VCAL_IDAC_Q_OFFSET);
    DEBUG_GET_RF_CHIP_REG(pMac, QWLAN_RFAPB_RX_IM2_SPARE0_REG, &value);
    im2Corr->ICorrect = value;
    DEBUG_GET_RF_CHIP_REG(pMac, QWLAN_RFAPB_RX_IM2_SPARE1_REG, &value);
    im2Corr->QCorrect = value;
#endif
}

static void writeIm2Corr(tpAniSirGlobal pMac, tANI_S16 corr_i, tANI_S16 corr_q)
{
    tRxIm2Correct im2Corr;

    hv_printLog("corr_i = 0x%x(%d), corr_q = 0x%x(%d)\n", corr_i, corr_i, corr_q, corr_q);
    im2Corr.ICorrect = (tIm2Correct)corr_i;
    im2Corr.QCorrect = (tIm2Correct)corr_q;
    hv_printLog("ICorrect = 0x%x(%d), QCorrect = 0x%x(%d)\n",
                   im2Corr.ICorrect, im2Corr.ICorrect, im2Corr.QCorrect, im2Corr.QCorrect);

    rfSetIm2Correct(pMac, im2Corr);
}

#define IM2_BS_MAX_STEPS    8

void im2UpdateErrCorr(tpAniSirGlobal pMac, tANI_BOOLEAN update_i, tUpdateIQ *pErrCorrIQ)
{
    tANI_U8 corr_lo_2update, corr_hi_2update, corr_mean;
    tANI_S16 meas_lo_2update, meas_hi_2update;
    tANI_S16 meas_mean_0, meas_mean_1, meas_mean;
    tANI_BOOLEAN on2sides_prv_2update, done_2update;
    tANI_BOOLEAN lo_moved = eANI_BOOLEAN_FALSE, on2sides_crt;
    /* Modified binary search to minimize dco residual for non-linear
     * curve of dco vs. corr dac setting
     * 1) Close-to-zero if the curve crosses zero
     * 2) minimum dco absolute if the curve never crosses zero
     */
    //update either I or Q using identical but independent process
    if (update_i)
    {
        corr_lo_2update = pErrCorrIQ->errorCorrLo.dcoOffset.IDcoCorrect;
        corr_hi_2update = pErrCorrIQ->errorCorrHi.dcoOffset.IDcoCorrect;
        meas_lo_2update = pErrCorrIQ->errorCorrLo.error.I;
        meas_hi_2update = pErrCorrIQ->errorCorrHi.error.I;
        on2sides_prv_2update = pErrCorrIQ->on2sides_prv.I;
        done_2update = pErrCorrIQ->done.I;
    }
    else
    {
        corr_lo_2update = pErrCorrIQ->errorCorrLo.dcoOffset.QDcoCorrect;
        corr_hi_2update = pErrCorrIQ->errorCorrHi.dcoOffset.QDcoCorrect;
        meas_lo_2update = pErrCorrIQ->errorCorrLo.error.Q;
        meas_hi_2update = pErrCorrIQ->errorCorrHi.error.Q;
        on2sides_prv_2update = pErrCorrIQ->on2sides_prv.Q;
        done_2update = pErrCorrIQ->done.Q;
    }

    //mid_point
    corr_mean = (1+(corr_lo_2update + corr_hi_2update)) >> 1;
    //measure dco with mid-point
    if (update_i)
    {
        writeIm2Corr(pMac, corr_mean, pErrCorrIQ->errorCorrLo.dcoOffset.QDcoCorrect);
        measureDco(pMac, &meas_mean_0, &pErrCorrIQ->errorCorrLo.error.Q);
        writeIm2Corr(pMac, corr_mean, pErrCorrIQ->errorCorrHi.dcoOffset.QDcoCorrect);
        measureDco(pMac, &meas_mean_1, &pErrCorrIQ->errorCorrHi.error.Q);
    }
    else
    {
        writeIm2Corr(pMac, pErrCorrIQ->errorCorrLo.dcoOffset.IDcoCorrect, corr_mean);
        measureDco(pMac, &pErrCorrIQ->errorCorrLo.error.I, &meas_mean_0);
        writeIm2Corr(pMac, pErrCorrIQ->errorCorrHi.dcoOffset.IDcoCorrect, corr_mean);
        measureDco(pMac, &pErrCorrIQ->errorCorrHi.error.I, &meas_mean_1);
    }

    meas_mean = (meas_mean_0 + meas_mean_1)/2;

    //initial state
    done_2update = eANI_BOOLEAN_FALSE;
    on2sides_crt = on2sides_prv_2update;
    //check if it needs update or not. If yes, which one(lo or hi) to be updated
    if ((ABS(meas_lo_2update) < 5) || (ABS(meas_hi_2update) < 5))
    {
        //when dco residual is low enough, no need to update anymore
        done_2update = eANI_BOOLEAN_TRUE;
    }
    else
    {
        if (!on2sides_prv_2update)
        {
            //meas_lo and meas_hi on the same side of x-axis
            if (((meas_mean > 0) && (meas_lo_2update > 0)) || ((meas_mean < 0) && (meas_lo_2update < 0)))
            {
                //meas_mean is on the same side as meas_lo and meas_hi
                if ((ABS(meas_mean) > ABS(meas_lo_2update)) && (ABS(meas_mean) > ABS(meas_hi_2update)))
                {
                    //mid-point has a larger dco, no need to update anymore
                    done_2update = eANI_BOOLEAN_TRUE;
                }
                else
                {
                    //whichever farther away from zero, get updated to mid-point
                    if (ABS(meas_lo_2update) > ABS(meas_hi_2update))
                    {
                        lo_moved = eANI_BOOLEAN_TRUE;
                    }
                    else
                    {
                        lo_moved = eANI_BOOLEAN_FALSE;
                    }
                    on2sides_crt = eANI_BOOLEAN_FALSE;
                }
            }
            else
            {
                //meas_mean is on the different side from meas_lo and meas_hi
                //whichever farther away from zero, get updated to mid-point
                if (ABS(meas_lo_2update) > ABS(meas_hi_2update))
                {
                    lo_moved = eANI_BOOLEAN_TRUE;
                }
                else
                {
                    lo_moved = eANI_BOOLEAN_FALSE;
                }
                on2sides_crt = eANI_BOOLEAN_TRUE;
            }
        }
        else
        {
            //meas_lo and meas_hi are on the different side of x-axis
            //whichever on the same side with meas_mean will be updated
            if (((meas_mean > 0) && (meas_lo_2update > 0)) || ((meas_mean < 0) && (meas_lo_2update < 0)))
            {
                lo_moved = eANI_BOOLEAN_TRUE;
            }
            else
            {
                lo_moved = eANI_BOOLEAN_FALSE;
            }
            on2sides_crt = eANI_BOOLEAN_TRUE;
        }
    }

    //implement update
    if (!done_2update)
    {
        //update corr_i/q
        if (lo_moved)
        {
            corr_lo_2update = corr_mean;
            meas_lo_2update = meas_mean;
        }
        else
        {
            corr_hi_2update = corr_mean;
            meas_hi_2update = meas_mean;
        }
        //update state
        on2sides_prv_2update = on2sides_crt;
        //early exit
        if (abs(meas_mean) < 5)
        {
            done_2update = eANI_BOOLEAN_TRUE;
        }
    }

    //return updated value for i and q
    if (update_i)
    {
        pErrCorrIQ->errorCorrLo.dcoOffset.IDcoCorrect = corr_lo_2update;
        pErrCorrIQ->errorCorrHi.dcoOffset.IDcoCorrect = corr_hi_2update;
        pErrCorrIQ->errorCorrLo.error.I = meas_lo_2update;
        pErrCorrIQ->errorCorrHi.error.I = meas_hi_2update;
        pErrCorrIQ->on2sides_prv.I = on2sides_prv_2update;
        pErrCorrIQ->done.I = done_2update;
    }
    else
    {
        pErrCorrIQ->errorCorrLo.dcoOffset.QDcoCorrect = corr_lo_2update;
        pErrCorrIQ->errorCorrHi.dcoOffset.QDcoCorrect = corr_hi_2update;
        pErrCorrIQ->errorCorrLo.error.Q = meas_lo_2update;
        pErrCorrIQ->errorCorrHi.error.Q = meas_hi_2update;
        pErrCorrIQ->on2sides_prv.Q = on2sides_prv_2update;
        pErrCorrIQ->done.Q = done_2update;
    }

    if (update_i)
    {
        phylog(LOGE, "im2_iter: corr_i_lo = %d, meas_i_lo = %d, corr_i_hi = %d, meas_i_hi = %d, on2sides_i_prv = %d, done_i = %d\n",
            pErrCorrIQ->errorCorrLo.dcoOffset.IDcoCorrect,
            pErrCorrIQ->errorCorrLo.error.I,
            pErrCorrIQ->errorCorrHi.dcoOffset.IDcoCorrect,
            pErrCorrIQ->errorCorrHi.error.I,
            pErrCorrIQ->on2sides_prv.I,
            pErrCorrIQ->done.I);
    }
    else
    {
        phylog(LOGE, "im2_iter: corr_q_lo = %d, meas_q_lo = %d, corr_q_hi = %d, meas_q_hi = %d, on2sides_q_prv = %d, done_q = %d\n",
            pErrCorrIQ->errorCorrLo.dcoOffset.QDcoCorrect,
            pErrCorrIQ->errorCorrLo.error.Q,
            pErrCorrIQ->errorCorrHi.dcoOffset.QDcoCorrect,
            pErrCorrIQ->errorCorrHi.error.Q,
            pErrCorrIQ->on2sides_prv.Q,
            pErrCorrIQ->done.Q);
    }
}

//Modified
void im2BinarySearch(tpAniSirGlobal pMac, tANI_S16 *meas_i, tANI_S16 *meas_q)
{
    tUpdateIQ curCorrErr;
    tANI_BOOLEAN done = eANI_BOOLEAN_FALSE;
    tANI_S16 corr_i_final, corr_q_final;
    tANI_U32 cnt = 0;

    curCorrErr.errorCorrLo.dcoOffset.IDcoCorrect = 0;
    curCorrErr.errorCorrLo.dcoOffset.QDcoCorrect = 0;
    curCorrErr.errorCorrLo.error.I = 0;
    curCorrErr.errorCorrLo.error.Q = 0;

    curCorrErr.errorCorrHi.dcoOffset.IDcoCorrect = 255;
    curCorrErr.errorCorrHi.dcoOffset.QDcoCorrect = 255;
    curCorrErr.errorCorrHi.error.I = 0;
    curCorrErr.errorCorrHi.error.Q = 0;

    curCorrErr.on2sides_prv.I = eANI_BOOLEAN_TRUE;
    curCorrErr.on2sides_prv.Q = eANI_BOOLEAN_TRUE;

    curCorrErr.done.I = eANI_BOOLEAN_FALSE;
    curCorrErr.done.Q = eANI_BOOLEAN_FALSE;

    writeIm2Corr(pMac, curCorrErr.errorCorrLo.dcoOffset.IDcoCorrect,
                        curCorrErr.errorCorrLo.dcoOffset.QDcoCorrect);
    measureDco(pMac, &curCorrErr.errorCorrLo.error.I,
                        &curCorrErr.errorCorrLo.error.Q);

    writeIm2Corr(pMac, curCorrErr.errorCorrHi.dcoOffset.IDcoCorrect,
                        curCorrErr.errorCorrHi.dcoOffset.QDcoCorrect);
    measureDco(pMac, &curCorrErr.errorCorrHi.error.I,
                        &curCorrErr.errorCorrHi.error.Q);

    //Check if meas_i/q_lo and meas_i/q_hi are on one side of origin
    if ((curCorrErr.errorCorrLo.error.I < 0 && curCorrErr.errorCorrHi.error.I < 0)||
            (curCorrErr.errorCorrLo.error.I > 0 && curCorrErr.errorCorrHi.error.I > 0))
    {
        curCorrErr.on2sides_prv.I = eANI_BOOLEAN_FALSE;
    }

    if ((curCorrErr.errorCorrLo.error.Q < 0 && curCorrErr.errorCorrHi.error.Q < 0) ||
            (curCorrErr.errorCorrLo.error.Q > 0 && curCorrErr.errorCorrHi.error.Q > 0))
    {
        curCorrErr.on2sides_prv.Q = eANI_BOOLEAN_FALSE;
    }

    phylog(LOGE, "im2_init_meas: corr_i_lo = %d, meas_i_lo = %d, corr_i_hi = %d, meas_i_hi = %d, on2sides_i_prv = %d, done_i = %d\n",
            curCorrErr.errorCorrLo.dcoOffset.IDcoCorrect,
            curCorrErr.errorCorrLo.error.I,
            curCorrErr.errorCorrHi.dcoOffset.IDcoCorrect,
            curCorrErr.errorCorrHi.error.I,
            curCorrErr.on2sides_prv.I,
            curCorrErr.done.I);

    while (!done)
    {
        //cal i
        if (!curCorrErr.done.I)
        {
            im2UpdateErrCorr(pMac, eANI_BOOLEAN_TRUE, &curCorrErr);
        }

        if (!curCorrErr.done.Q)
        {
            im2UpdateErrCorr(pMac, eANI_BOOLEAN_FALSE, &curCorrErr);
        }

        cnt++;

        done = (cnt == IM2_BS_MAX_STEPS);
    }

    //after zero-search
    //pick the corr setting with the smallest residual dco
    if (ABS(curCorrErr.errorCorrLo.error.I) <
            ABS(curCorrErr.errorCorrHi.error.I))
    {
        corr_i_final = curCorrErr.errorCorrLo.dcoOffset.IDcoCorrect;
        *meas_i = curCorrErr.errorCorrLo.error.I;
    }
    else
    {
        corr_i_final = curCorrErr.errorCorrHi.dcoOffset.IDcoCorrect;
        *meas_i = curCorrErr.errorCorrHi.error.I;
    }

    if (ABS(curCorrErr.errorCorrLo.error.Q) <
            ABS(curCorrErr.errorCorrHi.error.Q))
    {
        corr_q_final = curCorrErr.errorCorrLo.dcoOffset.QDcoCorrect;
        *meas_q = curCorrErr.errorCorrLo.error.Q;
    }
    else
    {
        corr_q_final = curCorrErr.errorCorrHi.dcoOffset.QDcoCorrect;
        *meas_q = curCorrErr.errorCorrHi.error.Q;
    }

    writeIm2Corr(pMac, corr_i_final, corr_q_final);

    phylog(LOGE, "corr_i_final = %d, meas_i_final = %d, corr_q_final = %d, meas_q_final = %d\n",
                    corr_i_final, *meas_i, corr_q_final, *meas_q);
}
#endif

#define IM2_MAX_ITER    5
#define DCO_LUT_IDX     31
/* If Im2CalOnly is true, we don't do RxDcoCal before IM2 Cal */
void phyRxIm2Cal(tpAniSirGlobal pMac, tANI_U8 Im2CalOnly)
{
#ifdef FEATURE_WLANFW_RF_ACCESS
    tANI_U8 save_corr_i, save_corr_q;
    tANI_S16 meas_i, meas_q;
    tANI_S16 corr_i, corr_q;
    tANI_U32 cnt = 0;
    tANI_U32 highgain_bias_ctl, lna_load_ctune;
    tANI_U32 th_signal_low;

    readDcoLut(pMac, DCO_LUT_IDX, &save_corr_i, &save_corr_q);

    log_regaccess = eANI_BOOLEAN_TRUE;
    phyRxIm2Setup(pMac);

    DEBUG_GET_PHY_REG(pMac, QWLAN_AGC_TH_SIGNAL_LOW_REG, &th_signal_low);
    DEBUG_SET_PHY_REG(pMac, QWLAN_AGC_TH_SIGNAL_LOW_REG, 0);

    //Config LNA settings beforehand
    DEBUG_GET_PHY_REG(pMac, QWLAN_RFAPB_RXFE_LNA_HIGHGAIN_BIAS_CTL_REG, &highgain_bias_ctl);
    DEBUG_SET_PHY_REG(pMac, QWLAN_RFAPB_RXFE_LNA_HIGHGAIN_BIAS_CTL_REG, 0xFF);
    DEBUG_GET_PHY_REG(pMac, QWLAN_RFAPB_RXFE_LNA_LOAD_CTL_REG_REG, &lna_load_ctune);
    debug_rdModWriteAsicField(pMac, QWLAN_RFAPB_RXFE_LNA_LOAD_CTL_REG_REG,
                    QWLAN_RFAPB_RXFE_LNA_LOAD_CTL_REG_RXFE_LNA_LOAD_CTUNE_MASK,
                    QWLAN_RFAPB_RXFE_LNA_LOAD_CTL_REG_RXFE_LNA_LOAD_CTUNE_OFFSET, 63);


    //log_regaccess = eANI_BOOLEAN_FALSE;

    writeIm2Corr(pMac, 127, 127);

    while (cnt++ < IM2_MAX_ITER)
    {
        if (!Im2CalOnly)
        {
            disableIm2ToneGen(pMac, eANI_BOOLEAN_FALSE);

            //do RxDco cal with ToneGen OFF
            searchBinary(pMac, DCO_LUT_IDX, &corr_i, &corr_q);

            measureDco(pMac, &meas_i, &meas_q);
            //phylog(LOGE, "meas_i = %d, meas_q = %d\n", meas_i, meas_q);
        }

        enableIm2ToneGen(pMac, eANI_BOOLEAN_FALSE);

        im2BinarySearch(pMac, &meas_i, &meas_q);

        measureDco(pMac, &meas_i, &meas_q);
        //phylog(LOGE, "meas_i = %d, meas_q = %d\n", meas_i, meas_q);
    }

    disableIm2ToneGen(pMac, eANI_BOOLEAN_FALSE);

    log_regaccess = eANI_BOOLEAN_TRUE;
    phyRxIm2Restore(pMac);

    DEBUG_SET_PHY_REG(pMac, QWLAN_AGC_TH_SIGNAL_LOW_REG, th_signal_low);

    //Config LNA settings beforehand
    DEBUG_SET_PHY_REG(pMac, QWLAN_RFAPB_RXFE_LNA_HIGHGAIN_BIAS_CTL_REG, highgain_bias_ctl);
    DEBUG_SET_PHY_REG(pMac, QWLAN_RFAPB_RXFE_LNA_LOAD_CTL_REG_REG, lna_load_ctune);

    //log_regaccess = eANI_BOOLEAN_FALSE;

    writeDcoLut(pMac, DCO_LUT_IDX, (tANI_S16)save_corr_i, (tANI_S16)save_corr_q);
#endif
}

#undef FEATURE_WLANFW_RF_ACCESS
#endif /* HALPHY_CAL_DEBUG */

/*
 * File:        halRFBringup.c
 * Description: This file contains all the interface functions to
 *              interact with the firmware
 *
 * Copyright (c) 2008 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Confidential and Proprietary
 *
 *
 * History:
 *
 * When       Who         What/Where/Why
 * -------------------------------------------------------------------
 * 01/23/2009 bharathp      Functions to support Rx DCO cal for the 
 *                          bringup RF AEVB
 *
 *
 */

#include "palApi.h"
#ifdef LIBRA_RF
#include "halDebug.h"


#define DCO_NEW_CORRECTION_RANGE        255
#define DCO_NEW_ERROR_TOLERANCE (40)

typedef struct
{
    tANI_U8 IDcoCorrect;
    tANI_U8 QDcoCorrect;
    tANI_U8     dcRange;
}tRxNewDcoCorrect;

typedef struct
{
    tIQAdc error;
    tRxNewDcoCorrect dcoOffset;
}tDcoErrorCorr;


static void set_rx_gain(tpAniSirGlobal pMac, tANI_U8 rxgain)
{
    tANI_U32 value;
    
    halReadRegister(pMac, QWLAN_RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_AGC_REG, &value);
    value |= QWLAN_RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_AGC_PWR_MASK;
    halWriteRegister(pMac, QWLAN_RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_AGC_REG, value);
    
    //t.csr.rxclkctrl.apb_block_dyn_clkg_disable_agc(pwr=1)
    
    if(rxgain != 0xFF)
    {
        //t.csr.agc.gainset_write(0)
        halWriteRegister(pMac, QWLAN_AGC_GAINSET_WRITE_REG, 0);
        
        //t.csr.agc.gainset0(override=1,gain=rxgain)
        halWriteRegister(pMac, QWLAN_AGC_GAINSET0_REG, (QWLAN_AGC_GAINSET0_OVERRIDE_MASK|rxgain));
        
        //t.csr.agc.gainset1(override=1,gain=rxgain)
        halWriteRegister(pMac, QWLAN_AGC_GAINSET1_REG, (QWLAN_AGC_GAINSET1_OVERRIDE_MASK|rxgain));
        
        //t.csr.agc.gainset_write(1)
        halWriteRegister(pMac, QWLAN_AGC_GAINSET_WRITE_REG, 1);
    }
    else
    {
        //t.csr.agc.gainset0(override=0,gain=0)
        halWriteRegister(pMac, QWLAN_AGC_GAINSET0_REG, 0);
        
        //t.csr.agc.gainset1(override=0,gain=0)
        halWriteRegister(pMac, QWLAN_AGC_GAINSET1_REG, 0);
        
        //t.csr.agc.agc_reset(1)
        //t.csr.agc.agc_reset(0)
        halWriteRegister(pMac, QWLAN_AGC_AGC_RESET_REG, 1);        
        halWriteRegister(pMac, QWLAN_AGC_AGC_RESET_REG, 0);                
    }
}

static void asicGetCalADCSamples(tpAniSirGlobal pMac, ePhyRxChains chain, tIQAdc *dcoError)
{
    tANI_U32 shunt, status, cnt, dcOffset;
    //t.csr.cal.calmode(1)
    halWriteRegister(pMac, QWLAN_CAL_CALMODE_REG, 1);
    //t.csr.cal.length(1023)
    halWriteRegister(pMac, QWLAN_CAL_LENGTH_REG, 1023);
    //t.csr.cal.chain_sel(chain)
    halWriteRegister(pMac, QWLAN_CAL_CHAIN_SEL_REG, chain);

    //if not shunt:
    //    t.csr.cal.debug_output_en_mask.dc_est_en_coarse(0)
    halReadRegister(pMac, QWLAN_CAL_DEBUG_OUTPUT_EN_MASK_REG, &shunt);
    shunt &= ~QWLAN_CAL_DEBUG_OUTPUT_EN_MASK_DC_EST_EN_COARSE_MASK;
    halWriteRegister(pMac, QWLAN_CAL_DEBUG_OUTPUT_EN_MASK_REG, shunt);

    //#Trigger measurement and wait until it's done
    //t.csr.cal.measure(1)
    halWriteRegister(pMac, QWLAN_CAL_MEASURE_REG, 1);
    
    status = 0;
    cnt = 0;
    while ((status != 1) & (cnt<1000))
    {
        //status = t.csr.cal.status.status()
		sirBusyWait(100000);
        halReadRegister(pMac, QWLAN_CAL_STATUS_REG, &status);
        status = (status & QWLAN_CAL_STATUS_STATUS_MASK) >> QWLAN_CAL_STATUS_STATUS_OFFSET;
        cnt = cnt+1;
        //if Verbose==1: print "cnt=%d status=%d" % (cnt,status)
    }
    
    if(cnt == 1000)
    {
        HALLOGE(halLog(pMac, LOGE, FL("ERROR: Measurement did not finish after several repeated attempts!\n")));
    }
    //t.csr.cal.chain_sel.iqsel(0)
    //halWriteRegister(pMac, QWLAN_CAL_CHAIN_SEL_REG, chain);
    //dcoi = t.csr.cal.dcoffset.dcoffset.TwosComp()
    halWriteRegister(pMac, QWLAN_CAL_CHAIN_SEL_REG, (0|chain));
	halReadRegister(pMac, QWLAN_CAL_DCOFFSET_REG, &dcOffset);
	dcOffset = (dcOffset & QWLAN_CAL_DCOFFSET_DCOFFSET_MASK) >> QWLAN_CAL_DCOFFSET_DCOFFSET_OFFSET;
    dcoError->I = ((tANI_S16)(dcOffset << 5) >> 5);
    
    //t.csr.cal.chain_sel.iqsel(1)
    halWriteRegister(pMac, QWLAN_CAL_CHAIN_SEL_REG, (QWLAN_CAL_CHAIN_SEL_IQSEL_MASK|chain));
    //dcoq = t.csr.cal.dcoffset.dcoffset.TwosComp()
    halReadRegister(pMac, QWLAN_CAL_DCOFFSET_REG, &dcOffset);
	dcOffset = (dcOffset & QWLAN_CAL_DCOFFSET_DCOFFSET_MASK) >> QWLAN_CAL_DCOFFSET_DCOFFSET_OFFSET;
    dcoError->Q = ((tANI_S16)(dcOffset << 5) >> 5);
    
    //dco = dcoi + dcoq*1j
    
    //t.csr.cal.calmode(0)
    halWriteRegister(pMac, QWLAN_CAL_CALMODE_REG, 0);
    //t.csr.cal.length(31)
    halWriteRegister(pMac, QWLAN_CAL_LENGTH_REG, 31);
}

static void rfGetDCOffset(tpAniSirGlobal pMac, ePhyRxChains rxChain, tANI_U8 dcoIndex, tRxNewDcoCorrect *offset)
{
    tANI_U32 modeSel, value = 0;

    //t.csr.rfapb.rx_gain_control(rx_gc0=gain,rx_gc1=gain)
    halWriteRegister(pMac, QWLAN_RFAPB_RX_GAIN_CONTROL_REG, ((dcoIndex << QWLAN_RFAPB_RX_GAIN_CONTROL_RX_GC1_OFFSET) | dcoIndex));

    //select the specific chain we are correcting
    halReadRegister(pMac, QWLAN_RFAPB_MODE_SEL1_REG, &modeSel);
    modeSel &= ~QWLAN_RFAPB_MODE_SEL1_TXRX_WRITE_ALL_MASK;
    modeSel &= ~QWLAN_RFAPB_MODE_SEL1_TXRX_REG_SEL_MASK;
    modeSel |= (rxChain << QWLAN_RFAPB_MODE_SEL1_TXRX_REG_SEL_OFFSET);
    halWriteRegister(pMac, QWLAN_RFAPB_MODE_SEL1_REG, modeSel);
    
    halReadRegister(pMac, QWLAN_RFAPB_RX_DCOC_IQ_REG, &value);    

    offset->IDcoCorrect = (tANI_U8)((value & QWLAN_RFAPB_RX_DCOC_IQ_RX_DCOC_I_MASK) >> QWLAN_RFAPB_RX_DCOC_IQ_RX_DCOC_I_OFFSET);
    offset->QDcoCorrect = (tANI_U8)((value & QWLAN_RFAPB_RX_DCOC_IQ_RX_DCOC_Q_MASK) >> QWLAN_RFAPB_RX_DCOC_IQ_RX_DCOC_Q_OFFSET);
    offset->dcRange = 1;
}

static void rfSetDCOffset(tpAniSirGlobal pMac, ePhyRxChains rxChain, tANI_U8 dcoIndex, tRxNewDcoCorrect offset)
{
    tANI_U32 reset, modeSel;
    
    halReadRegister(pMac, QWLAN_AGC_AGC_RESET_REG, &reset);        
    halWriteRegister(pMac, QWLAN_AGC_AGC_RESET_REG, 1); 
    
    //t.csr.rfapb.rx_gain_control(rx_gc0=gain,rx_gc1=gain)
    halWriteRegister(pMac, QWLAN_RFAPB_RX_GAIN_CONTROL_REG, ((dcoIndex << QWLAN_RFAPB_RX_GAIN_CONTROL_RX_GC1_OFFSET) | dcoIndex));
    
    //select the specific chain we are correcting
    halReadRegister(pMac, QWLAN_RFAPB_MODE_SEL1_REG, &modeSel);
	modeSel &= ~QWLAN_RFAPB_MODE_SEL1_TXRX_WRITE_ALL_MASK;
    modeSel &= ~QWLAN_RFAPB_MODE_SEL1_TXRX_REG_SEL_MASK;
    modeSel |= (rxChain << QWLAN_RFAPB_MODE_SEL1_TXRX_REG_SEL_OFFSET);
	halWriteRegister(pMac, QWLAN_RFAPB_MODE_SEL1_REG, modeSel);
    
    halWriteRegister(pMac, QWLAN_RFAPB_RX_DCOC_IQ_REG, (offset.IDcoCorrect << QWLAN_RFAPB_RX_DCOC_IQ_RX_DCOC_I_OFFSET) |
                                                       (offset.QDcoCorrect << QWLAN_RFAPB_RX_DCOC_IQ_RX_DCOC_Q_OFFSET)
                                                       );
    sirBusyWait(10000);  //wait 1 us
    halWriteRegister(pMac, QWLAN_AGC_AGC_RESET_REG, reset);
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

static void phyDcoCalRxChain(tpAniSirGlobal pMac, ePhyRxChains rxChain, tANI_U8 gain)
{
    tANI_U32 iteration = 0;
    tDcoErrorCorr dcoCorrErr, negErrorCorr, posErrorCorr;
        
    //set gain for this calibration
    set_rx_gain(pMac, gain);
    
    //identify +ve error and -ve error
    {
        dcoCorrErr.dcoOffset.IDcoCorrect = (tANI_U8)0;
        dcoCorrErr.dcoOffset.QDcoCorrect = (tANI_U8)0;
        dcoCorrErr.dcoOffset.dcRange = 1;
        rfSetDCOffset(pMac, rxChain, (tANI_U8)gain, dcoCorrErr.dcoOffset);
        asicGetCalADCSamples(pMac, rxChain, &(dcoCorrErr.error));
        updateError(&dcoCorrErr, &negErrorCorr, &posErrorCorr);
        dcoCorrErr.dcoOffset.IDcoCorrect = (tANI_U8)255;
        dcoCorrErr.dcoOffset.QDcoCorrect = (tANI_U8)255;
        rfSetDCOffset(pMac, rxChain, (tANI_U8)gain, dcoCorrErr.dcoOffset);
        asicGetCalADCSamples(pMac, rxChain, &(dcoCorrErr.error));
        updateError(&dcoCorrErr, &negErrorCorr, &posErrorCorr);
    }

    //Calibrate DCO for I & Q rails
    while (iteration++ < DCO_NEW_CORRECTION_RANGE)
    {
        dcoCorrErr.dcoOffset.IDcoCorrect = (negErrorCorr.dcoOffset.IDcoCorrect + posErrorCorr.dcoOffset.IDcoCorrect)/2;
        dcoCorrErr.dcoOffset.QDcoCorrect = (negErrorCorr.dcoOffset.QDcoCorrect + posErrorCorr.dcoOffset.QDcoCorrect)/2;
        dcoCorrErr.dcoOffset.dcRange = 1;
        rfSetDCOffset(pMac, rxChain, (tANI_U8)gain, dcoCorrErr.dcoOffset);
        asicGetCalADCSamples(pMac, rxChain, &(dcoCorrErr.error));

        HALLOGE(halLog(pMac, LOGE, FL("iter=%d phy RX%d CorrI=%d CorrQ=%d DCO_ERR:I=%d Q=%d\n"), iteration, rxChain, 
                dcoCorrErr.dcoOffset.IDcoCorrect, dcoCorrErr.dcoOffset.QDcoCorrect, dcoCorrErr.error.I, dcoCorrErr.error.Q));

        if ((GET_MAG(dcoCorrErr.error.I) <= DCO_NEW_ERROR_TOLERANCE) &&
            (GET_MAG(dcoCorrErr.error.Q) <= DCO_NEW_ERROR_TOLERANCE)
           )
        {
            break;  //tolerances met, exit loop
        }
        updateError(&dcoCorrErr, &negErrorCorr, &posErrorCorr);
    }

    if (iteration == DCO_NEW_CORRECTION_RANGE)
    {
        HALLOGE(halLog(pMac, LOGE, FL("DCO CAL FAILED TO CONVERGE")));
    }
}

void halRF_InitRxDcoCal(tpAniSirGlobal pMac)
{
    ePhyRxChains rxChain;
    tANI_U32 gain, ant_en, value, agcDisModeReg, xBar, n_active, rxOverride, calOverRide, dacControl, n_active_bits;
    tANI_U8 calGainIndex = 80;
       
    //set the tx dacs
    halReadRegister(pMac, QWLAN_TXCTL_FIR_MODE_REG, &ant_en);
    ant_en = (ant_en & QWLAN_TXCTL_FIR_MODE_ANT_EN_MASK) >> QWLAN_TXCTL_FIR_MODE_ANT_EN_OFFSET;
    halReadRegister(pMac, QWLAN_TXCTL_DAC_CONTROL_REG, &value);
    dacControl = value;
    value |= QWLAN_TXCTL_DAC_CONTROL_TXEN_OVERRIDE_EN_MASK | QWLAN_TXCTL_DAC_CONTROL_DAC_OVERRIDE_EN_MASK;
    value = (value & ~QWLAN_TXCTL_DAC_CONTROL_CH0STDBY_OVERRIDE_VAL_MASK) | ((ant_en & 0x1) << QWLAN_TXCTL_DAC_CONTROL_CH0STDBY_OVERRIDE_VAL_OFFSET);
    value = (value & ~QWLAN_TXCTL_DAC_CONTROL_CH1STDBY_OVERRIDE_VAL_MASK) | (((ant_en & 0x2)>>1) << QWLAN_TXCTL_DAC_CONTROL_CH1STDBY_OVERRIDE_VAL_OFFSET);
    halWriteRegister(pMac, QWLAN_TXCTL_DAC_CONTROL_REG, value);        
    
    // disable all rx pkt types
    halReadRegister(pMac, QWLAN_AGC_DIS_MODE_REG, &agcDisModeReg);
    halWriteRegister(pMac, QWLAN_AGC_DIS_MODE_REG, PHY_RX_DISABLE_ALL_TYPES);        

    //rx xbar - Always "straight" for cal
    halReadRegister(pMac, QWLAN_AGC_CONFIG_XBAR_REG, &value);
    xBar = value;
    value = (value & ~QWLAN_AGC_CONFIG_XBAR_CHAIN0XBAR_MASK);
    halWriteRegister(pMac, QWLAN_AGC_CONFIG_XBAR_REG, value);
    
    //force on all the RX enables on (may not be needed with Quasar)
    halReadRegister(pMac, QWLAN_AGC_N_ACTIVE_REG, &n_active);
    if(n_active == 1)
        n_active_bits = 0x111;
    else
        n_active_bits = 0x333;

    halReadRegister(pMac, QWLAN_AGC_RX_OVERRIDE_REG, &rxOverride);
    halWriteRegister(pMac, QWLAN_AGC_RX_OVERRIDE_REG, n_active_bits);        

    //reset AGC
    halWriteRegister(pMac, QWLAN_AGC_AGC_RESET_REG, 1);        
    halWriteRegister(pMac, QWLAN_AGC_AGC_RESET_REG, 0);        

    //set the calmode to dco
    halWriteRegister(pMac, QWLAN_CAL_CALMODE_REG, 1);
    
    //by pass dco correction
    halReadRegister(pMac, QWLAN_CAL_OVERRIDE_REG, &calOverRide);
    halWriteRegister(pMac, QWLAN_CAL_OVERRIDE_REG, 3);        

    
    for (rxChain = 0; rxChain < 2/*PHY_MAX_RX_CHAINS*/; rxChain++)
    {
        HALLOG1(halLog(pMac, LOG1, FL("Performing Initial DCO cal on RX%d\n"), rxChain));
        
        {
            tRxNewDcoCorrect dco;
            phyDcoCalRxChain(pMac, rxChain, calGainIndex);

            //now plug resultant dco at max gain into all other dco gain settings
            rfGetDCOffset(pMac, rxChain, calGainIndex, &dco);
            
            for (gain = 0; gain < NUM_RF_DCO_VALUES; gain++)
            {
                rfSetDCOffset(pMac, rxChain, (tANI_U8)gain, dco);
            }
        }
    }

    // restore dco correction is performed
    halWriteRegister(pMac, QWLAN_CAL_OVERRIDE_REG, 0);
    
    //normal mode
    halWriteRegister(pMac, QWLAN_CAL_CALMODE_REG, 0);

    //restore
    halWriteRegister(pMac, QWLAN_AGC_CONFIG_XBAR_REG, xBar);
    halWriteRegister(pMac, QWLAN_AGC_RX_OVERRIDE_REG, rxOverride);
    halWriteRegister(pMac, QWLAN_AGC_AGC_RESET_REG, 1);        
    halWriteRegister(pMac, QWLAN_AGC_AGC_RESET_REG, 0);        

    halWriteRegister(pMac, QWLAN_AGC_DIS_MODE_REG, agcDisModeReg);
    halWriteRegister(pMac, QWLAN_TXCTL_DAC_CONTROL_REG, dacControl);
    set_rx_gain(pMac, 0xFF);    
}


typedef struct
{
    tANI_U8 channelNum;            //channel number
    tANI_U16 targetFreq;           //number in MHz
    tANI_U32 pllReg0;              //PLL reg0
    tANI_U32 pllReg1;              //PLL reg0
    tANI_U32 pllReg2;              //PLL reg0
    tANI_U32 pllReg4;              //PLL reg0
}tRfFreqFracNPLLVals;

//The following Fractional-N PLL values are retrieved from 
//python script that does the computation. For more info
//refer //depot2/hardware/gen5_6/lab/python/gen5lab/digic/libra.py
const tRfFreqFracNPLLVals pllVals[14] =
{    
    //chan, freq, pllFracNNF1, pllFracNNF2, pllFracNNF3, pllFracNNAB
    { 1  ,  2412,    0x0   ,      0x0   ,        0x28  ,    0x3e},  
    { 2  ,  2417,    0xab  ,      0xaa  ,        0x38  ,    0x3e},  
    { 3  ,  2422,    0x55  ,      0x55  ,        0x49  ,    0x3e},
    { 4  ,  2427,    0x0   ,      0x0   ,        0x5a  ,    0x3e},  
    { 5  ,  2432,    0xab  ,      0xaa  ,        0x6a  ,    0x3e},
    { 6  ,  2437,    0x55  ,      0x55  ,        0x7b  ,    0x3e},  
    { 7  ,  2442,    0x0   ,      0x0   ,        0xc   ,    0x3f},  
    { 8  ,  2447,    0xab  ,      0xaa  ,        0x1c  ,    0x3f},          
    { 9  ,  2452,    0x55  ,      0x55  ,        0x2d  ,    0x3f},  
    { 10 ,  2457,    0x0   ,      0x0   ,        0x3e  ,    0x3f},  
    { 11 ,  2462,    0xab  ,      0xaa  ,        0xce  ,    0x40},  
    { 12 ,  2467,    0x55  ,      0x55  ,        0xdf  ,    0x40},  
    { 13 ,  2472,    0x0   ,      0x0   ,        0xf0  ,    0x40},         
    { 14 ,  2484,    0x0   ,      0x0   ,        0x18  ,    0x40}     
};

void halRF_SetChannel(tpAniSirGlobal pMac, tANI_U8 chan)
{    
    tANI_U32 i;
    if((chan > 14) || (chan < 1))
    {
        HALLOGE(halLog(pMac, LOGE, FL("ERROR: Invalid channel!\n")));
        return;
    }
    
    for(i = 0; i < 2; i++)
    {
        halWriteRegister(pMac, 0xe02e004, 0x1);

        halWriteRegister(pMac, 0xe02f820, 0x601);
        halWriteRegister(pMac, 0xe02f984, 0xff71);
        halWriteRegister(pMac, 0xe02f968, 0xa500);                  
        halWriteRegister(pMac, 0xe02f8e8, 0);      
                
        halWriteRegister(pMac, 0xe02f8c0, pllVals[chan-1].pllReg0);        
        halWriteRegister(pMac, 0xe02f8c4, pllVals[chan-1].pllReg1);        
        halWriteRegister(pMac, 0xe02f8c8, pllVals[chan-1].pllReg2);                  
        halWriteRegister(pMac, 0xe02f8d0, pllVals[chan-1].pllReg4);      
        
        sirBusyWait(1000000); //wait 100us
        halWriteRegister(pMac, 0xe02f820, 0x601);
        {
            //self._digic.csr.rfapb.mode_sel1.pllen_force(1)
            tANI_U32 value;
            halReadRegister(pMac, QWLAN_RFAPB_MODE_SEL1_REG, &value);
            value = (value & ~QWLAN_RFAPB_MODE_SEL1_PLLEN_FORCE_MASK) | (1 << QWLAN_RFAPB_MODE_SEL1_PLLEN_FORCE_OFFSET);
            halWriteRegister(pMac, QWLAN_RFAPB_MODE_SEL1_REG, value);
        }
        halWriteRegister(pMac, 0xe02f820, 0x600);
        
        //make sure we can turn on tcxo and pll from pmu
        {
            tANI_U32 value;
            halReadRegister(pMac, QWLAN_PMU_RF_PA_TRSW_CTRL_REG_REG, &value);
            value = (value & ~QWLAN_PMU_RF_PA_TRSW_CTRL_REG_PMU_RFA_PLL_EN_MASK_MASK);
            value = (value & ~QWLAN_PMU_RF_PA_TRSW_CTRL_REG_PMU_RFA_TCXO_BUF_EN_MASK_MASK);
            halWriteRegister(pMac, QWLAN_PMU_RF_PA_TRSW_CTRL_REG_REG, value);
        }
        halWriteRegister(pMac, QWLAN_MCU_MCU_PMU_INFO_REG, 1);
        halWriteRegister(pMac, 0xe02e004, 0x0);
    }    
}
#endif //LIBRA_RF

#include "ani_assert.h"
#include "sys_api.h"
#include "rfQuasarFilterCal2.h"

#ifdef ANI_MANF_DIAG
const tWaveformSample wfmTxCal[] =
{
    { 254, 0      },
    { 99, 0       },
    { -254, 359   },
    { -282, 0     },
    { 254, -508   },
    { 422, 0      },
    { -254, 359   },
    { -498, 0     },
    { 254, 0      },
    { 498, 0      },
    { -253, -359  },
    { -422, 0     },
    { 254, 508    },
    { 282, 0      },
    { -253, -359  },
    { -99, 0      },
    { 254, 0      },
    { -99, 0      },
    { -254, 359   },
    { 282, 0      },
    { 254, -508   },
    { -422, 0     },
    { -254, 359   },
    { 498, 0      },
    { 254, 0      },
    { -498, 0     },
    { -253, -359  },
    { 422, 0      },
    { 254, 508    },
    { -282, 0     },
    { -253, -359  },
    { 99, 0       },
    { 254, 0      },
    { 99, 0       },
    { -254, 359   },
    { -282, 0     },
    { 254, -508   },
    { 422, 0      },
    { -254, 359   },
    { -498, 0     },
    { 254, 0      },
    { 498, 0      },
    { -253, -359  },
    { -422, 0     },
    { 254, 508    },
    { 282, 0      },
    { -253, -359  },
    { -99, 0      },
    { 254, 0      },
    { -99, 0      },
    { -254, 359   },
    { 282, 0      },
    { 254, -508   },
    { -422, 0     },
    { -254, 359   },
    { 498, 0      },
    { 254, 0      },
    { -498, 0     },
    { -253, -359  },
    { 422, 0      },
    { 254, 508    },
    { -282, 0     },
    { -253, -359  },
    { 99, 0       },
    { 254, 0      },
    { 99, 0       },
    { -254, 359   },
    { -282, 0     },
    { 254, -508   },
    { 422, 0      },
    { -254, 359   },
    { -498, 0     },
    { 254, 0      },
    { 498, 0      },
    { -253, -359  },
    { -422, 0     },
    { 254, 508    },
    { 282, 0      },
    { -253, -359  },
    { -99, 0      },
    { 254, 0      },
    { -99, 0      },
    { -254, 359   },
    { 282, 0      },
    { 254, -508   },
    { -422, 0     },
    { -254, 359   },
    { 498, 0      },
    { 254, 0      },
    { -498, 0     },
    { -253, -359  },
    { 422, 0      },
    { 254, 508    },
    { -282, 0     },
    { -253, -359  },
    { 99, 0       },
    { 254, 0      },
    { 99, 0       },
    { -254, 359   },
    { -282, 0     },
    { 254, -508   },
    { 422, 0      },
    { -254, 359   },
    { -498, 0     },
    { 254, 0      },
    { 498, 0      },
    { -253, -359  },
    { -422, 0     },
    { 254, 508    },
    { 282, 0      },
    { -253, -359  },
    { -99, 0      },
    { 254, 0      },
    { -99, 0      },
    { -254, 359   },
    { 282, 0      },
    { 254, -508   },
    { -422, 0     },
    { -254, 359   },
    { 498, 0      },
    { 254, 0      },
    { -498, 0     },
    { -253, -359  },
    { 422, 0      },
    { 254, 508    },
    { -282, 0     },
    { -253, -359  },
    { 99, 0       },
    { 254, 0      },
    { 99, 0       },
    { -254, 359   },
    { -282, 0     },
    { 254, -508   },
    { 422, 0      },
    { -254, 359   },
    { -498, 0     },
    { 254, 0      },
    { 498, 0      },
    { -253, -359  },
    { -422, 0     },
    { 254, 508    },
    { 282, 0      },
    { -253, -359  },
    { -99, 0      },
    { 254, 0      },
    { -99, 0      },
    { -254, 359   },
    { 282, 0      },
    { 254, -508   },
    { -422, 0     },
    { -254, 359   },
    { 498, 0      },
    { 254, 0      },
    { -498, 0     },
    { -253, -359  },
    { 422, 0      },
    { 254, 508    },
    { -282, 0     },
    { -253, -359  },
    { 99, 0       },
    { 254, 0      },
    { 99, 0       },
    { -254, 359   },
    { -282, 0     },
    { 254, -508   },
    { 422, 0      },
    { -254, 359   },
    { -498, 0     },
    { 254, 0      },
    { 498, 0      },
    { -253, -359  },
    { -422, 0     },
    { 254, 508    },
    { 282, 0      },
    { -253, -359  },
    { -99, 0      },
    { 254, 0      },
    { -99, 0      },
    { -254, 359   },
    { 282, 0      },
    { 254, -508   },
    { -422, 0     },
    { -254, 359   },
    { 498, 0      },
    { 254, 0      },
    { -498, 0     },
    { -253, -359  },
    { 422, 0      },
    { 254, 508    },
    { -282, 0     },
    { -253, -359  },
    { 99, 0       },
    { 254, 0      },
    { 99, 0       },
    { -254, 359   },
    { -282, 0     },
    { 254, -508   },
    { 422, 0      },
    { -254, 359   },
    { -498, 0     },
    { 254, 0      },
    { 498, 0      },
    { -253, -359  },
    { -422, 0     },
    { 254, 508    },
    { 282, 0      },
    { -253, -359  },
    { -99, 0      },
    { 254, 0      },
    { -99, 0      },
    { -254, 359   },
    { 282, 0      },
    { 254, -508   },
    { -422, 0     },
    { -254, 359   },
    { 498, 0      },
    { 254, 0      },
    { -498, 0     },
    { -253, -359  },
    { 422, 0      },
    { 254, 508    },
    { -282, 0     },
    { -253, -359  },
    { 99, 0       },
    { 254, 0      },
    { 99, 0       },
    { -254, 359   },
    { -282, 0     },
    { 254, -508   },
    { 422, 0      },
    { -254, 359   },
    { -498, 0     },
    { 254, 0      },
    { 498, 0      },
    { -253, -359  },
    { -422, 0     },
    { 254, 508    },
    { 282, 0      },
    { -253, -359  },
    { -99, 0      },
    { 254, 0      },
    { -99, 0      },
    { -254, 359   },
    { 282, 0      },
    { 254, -508   },
    { -422, 0     },
    { -254, 359   },
    { 498, 0      },
    { 254, 0      },
    { -498, 0     },
    { -253, -359  },
    { 422, 0      },
    { 254, 508    },
    { -282, 0     },
    { -253, -359  },
    { 99, 0       }
};


#define MAX_NUM_SAMPLES   2048
#define TX_IF 0
#define TX_RF 1
#define RX_IF 2

static eHalStatus MeasureRxLoopbackPower(tpAniSirGlobal pMac, ePhyRxChains rxChain, tANI_U32 *pwr, tGrabRamSample *sampleBuffer, tANI_U16 numSamples);
static eHalStatus calibrateLoopbackDCO(tpAniSirGlobal pMac, tANI_U32 rxChain, tANI_U8 rxGain);
static eHalStatus writeDCOLut (tpAniSirGlobal pMac, tANI_U32 rxChain,tANI_S16 irail, tANI_S16 qrail, tANI_U8 gain);
static eHalStatus searchTxCal(tpAniSirGlobal pMac, tANI_U32 chan, tANI_U32 bound1, tANI_U32 bound2);
static eHalStatus searchRxCal(tpAniSirGlobal pMac, tANI_U32 chan, tANI_U32 bound1, tANI_U32 bound2);

static eHalStatus rfQuasarCalInit(tpAniSirGlobal pMac)
{
    eHalStatus  retVal;
        
    if ((retVal = asicGetDisabledRxPacketTypes(pMac, &(pMac->hphy.rf.filterCalRegContext.PktTypes))) != eHAL_STATUS_SUCCESS) { return (retVal); }   
    GET_PHY_REG(pMac->hHdd, AGC_CONFIG_XBAR_REG, &(pMac->hphy.rf.filterCalRegContext.configXbar));
    GET_PHY_REG(pMac->hHdd, AGC_RX_OVERRIDE_REG, &(pMac->hphy.rf.filterCalRegContext.agcRxOverride));    
    GET_PHY_REG(pMac->hHdd, RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, &(pMac->hphy.rf.filterCalRegContext.rxClkCtrl));
    GET_PHY_REG(pMac->hHdd, TXCTL_DAC_CONTROL_REG, &(pMac->hphy.rf.filterCalRegContext.dacControl));
    GET_PHY_REG(pMac->hHdd, TXCTL_FIR_MODE_REG, &(pMac->hphy.rf.filterCalRegContext.firTxMode));
    GET_PHY_REG(pMac->hHdd, AGC_GAINSET0_REG, &(pMac->hphy.rf.filterCalRegContext.gainSet0));
    GET_PHY_REG(pMac->hHdd, AGC_GAINSET1_REG, &(pMac->hphy.rf.filterCalRegContext.gainSet1));
    GET_PHY_REG(pMac->hHdd, AGC_GAINSET2_REG, &(pMac->hphy.rf.filterCalRegContext.gainSet2));
    GET_PHY_REG(pMac->hHdd, TPC_TXPWR_OVERRIDE_REG, &(pMac->hphy.rf.filterCalRegContext.txPwrOverride));
    GET_PHY_REG(pMac->hHdd, TPC_TXPWR_ENABLE_REG, &(pMac->hphy.rf.filterCalRegContext.txPwrEnable));
    GET_PHY_REG(pMac->hHdd, CAL_CHAIN_SEL_REG, &(pMac->hphy.rf.filterCalRegContext.chainSel));
    GET_PHY_REG(pMac->hHdd, AGC_BANDWIDTH_CONFIG_REG, &(pMac->hphy.rf.filterCalRegContext.bdwidthCfg));

/*
    // Commenting this code because the following register reads are not needed as all the below fields are read only fields 
    if ((retVal = rfReadQuasarField(pMac, QUASAR_FIELD_TX_GAIN_0, &(pMac->hphy.rf.filterCalRegContext.txGain0))) != eHAL_STATUS_SUCCESS) { return (retVal); }
    if ((retVal = rfReadQuasarField(pMac, QUASAR_FIELD_TX_GAIN_1, &(pMac->hphy.rf.filterCalRegContext.txGain1))) != eHAL_STATUS_SUCCESS) { return (retVal); }

    if ((retVal = rfReadQuasarField(pMac, QUASAR_FIELD_RXG_2, &(pMac->hphy.rf.filterCalRegContext.rxg2))) != eHAL_STATUS_SUCCESS) { return (retVal); }
    if ((retVal = rfReadQuasarField(pMac, QUASAR_FIELD_RXG_1, &(pMac->hphy.rf.filterCalRegContext.rxg1))) != eHAL_STATUS_SUCCESS) { return (retVal); }
    if ((retVal = rfReadQuasarField(pMac, QUASAR_FIELD_RXG_0, &(pMac->hphy.rf.filterCalRegContext.rxg0))) != eHAL_STATUS_SUCCESS) { return (retVal); }
*/
    if ((retVal = rfReadQuasarField(pMac, QUASAR_FIELD_RX_GAIN_1, &(pMac->hphy.rf.filterCalRegContext.rxGain))) != eHAL_STATUS_SUCCESS) { return (retVal); }
    if ((retVal = rfReadQuasarField(pMac, QUASAR_FIELD_SPI_LUT_INDEX, &(pMac->hphy.rf.filterCalRegContext.spiLut))) != eHAL_STATUS_SUCCESS) { return (retVal); }

    SET_PHY_REG(pMac->hHdd, AGC_TH_CLIP_HIGH_REG, (1024 + 512));
    SET_PHY_REG(pMac->hHdd, AGC_TH_CLIP_LOW_REG, (1024 - 512));

    return (retVal);
}

static eHalStatus rfQuasarCalRestore(tpAniSirGlobal pMac)
{
    eHalStatus  retVal;
    tANI_U32    value;

    retVal = rfReadDataField(pMac, QUASAR_TEST1_REG, MSK_32, 0, &value);
    if (retVal != eHAL_STATUS_SUCCESS)
    {
        return retVal;
    }
//    value &= 0x27031;

    value &= (QUASAR_TEST1_NDIV_OUT_MASK | QUASAR_TEST1_TEST_LB_RF_MASK | QUASAR_TEST1_DET_OFFSET_MASK | QUASAR_TEST1_RDIV_TEST_MASK);

    retVal = rfWriteDataField(pMac, QUASAR_TEST1_REG, MSK_32, 0, value);
    if (retVal != eHAL_STATUS_SUCCESS)
    {
        return retVal;
    }
    
/* 
    // The below register writes are done in the optimized way above. Retaining this legacy code for reference 
    if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_RX_IQ_TEST_1, 0)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_RX_IQ_TEST_0, 0)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_TX_CAL_EN_0, 0)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_TX_CAL_EN_1, 0)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_RX_LB_GAIN, 0)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_TX_LB_GAIN, 0)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    //if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_DET_GAIN, 0)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    //if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_SPI_LUT_INDEX, 0)) != eHAL_STATUS_SUCCESS) { return (retVal); }

    if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_SPI_LUT_EN, 0)) != eHAL_STATUS_SUCCESS) { return (retVal); }

    if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_SPI_LUT_INDEX, pMac->hphy.rf.filterCalRegContext.spiLut)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_TX_GAIN_0, pMac->hphy.rf.filterCalRegContext.txGain0)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_TX_GAIN_1, pMac->hphy.rf.filterCalRegContext.txGain1)) != eHAL_STATUS_SUCCESS) { return (retVal); }

    if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_RXG_2, pMac->hphy.rf.filterCalRegContext.rxg2)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_RXG_1, pMac->hphy.rf.filterCalRegContext.rxg1)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_RXG_0, pMac->hphy.rf.filterCalRegContext.rxg0)) != eHAL_STATUS_SUCCESS) { return (retVal); }
*/

    if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_SPI_LUT_EN, 0)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_RX_GAIN_1, pMac->hphy.rf.filterCalRegContext.rxGain)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    
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
    
    if ((retVal = asicSetDisabledRxPacketTypes(pMac, pMac->hphy.rf.filterCalRegContext.PktTypes)) != eHAL_STATUS_SUCCESS) { return (retVal); }   
    SET_PHY_REG(pMac->hHdd, AGC_CONFIG_XBAR_REG, pMac->hphy.rf.filterCalRegContext.configXbar);
    SET_PHY_REG(pMac->hHdd, AGC_RX_OVERRIDE_REG, pMac->hphy.rf.filterCalRegContext.agcRxOverride);   
    SET_PHY_REG(pMac->hHdd, RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, pMac->hphy.rf.filterCalRegContext.rxClkCtrl);
    SET_PHY_REG(pMac->hHdd, TXCTL_DAC_CONTROL_REG, pMac->hphy.rf.filterCalRegContext.dacControl);
    SET_PHY_REG(pMac->hHdd, TXCTL_FIR_MODE_REG, pMac->hphy.rf.filterCalRegContext.firTxMode);
    SET_PHY_REG(pMac->hHdd, AGC_GAINSET0_REG, pMac->hphy.rf.filterCalRegContext.gainSet0);
    SET_PHY_REG(pMac->hHdd, AGC_GAINSET1_REG, pMac->hphy.rf.filterCalRegContext.gainSet1);
    SET_PHY_REG(pMac->hHdd, AGC_GAINSET2_REG, pMac->hphy.rf.filterCalRegContext.gainSet2);
    SET_PHY_REG(pMac->hHdd, TPC_TXPWR_OVERRIDE_REG, pMac->hphy.rf.filterCalRegContext.txPwrOverride);
    SET_PHY_REG(pMac->hHdd, TPC_TXPWR_ENABLE_REG, pMac->hphy.rf.filterCalRegContext.txPwrEnable);
    SET_PHY_REG(pMac->hHdd, CAL_CHAIN_SEL_REG, pMac->hphy.rf.filterCalRegContext.chainSel);
    SET_PHY_REG(pMac->hHdd, AGC_BANDWIDTH_CONFIG_REG, pMac->hphy.rf.filterCalRegContext.bdwidthCfg);
    
    SET_PHY_REG(pMac->hHdd, AGC_TH_CLIP_HIGH_REG, 1824);
    SET_PHY_REG(pMac->hHdd, AGC_TH_CLIP_LOW_REG, 224);

    SET_PHY_REG(pMac->hHdd, CAL_OVERRIDE_REG, 0);
    SET_PHY_REG(pMac->hHdd, CAL_LENGTH_REG, 0x1F);
    return (retVal);
}


static tANI_U8 getTuneVal(tpAniSirGlobal pMac, tANI_U32 chan, tANI_U32 mode, tANI_U32 bound1, tANI_U32 bound2)
{
    if(mode == TX_IF)
    {
        if(pMac->hphy.rf.filterCalRegContext.chanFilterSettings[chan].txIf != 0xFF)
        {
            return pMac->hphy.rf.filterCalRegContext.chanFilterSettings[chan].txIf;
        }
        else
        {
            if (searchTxCal(pMac, chan, bound1, bound2) != eHAL_STATUS_SUCCESS) { return 0x80; }
            return pMac->hphy.rf.filterCalRegContext.chanFilterSettings[chan].txIf;
        }
    }
    else if(mode == TX_RF)
    {
        if(pMac->hphy.rf.filterCalRegContext.chanFilterSettings[chan].txRf != 0xFF)
        {
            return pMac->hphy.rf.filterCalRegContext.chanFilterSettings[chan].txRf;
        }
        else
        {
            if (searchTxCal(pMac, chan, bound1, bound2) != eHAL_STATUS_SUCCESS) { return 0x80; }
            return pMac->hphy.rf.filterCalRegContext.chanFilterSettings[chan].txRf;
        }        
    }
    else if(mode == RX_IF)
    {
        if(pMac->hphy.rf.filterCalRegContext.chanFilterSettings[chan].rxIf != 0xFF)
        {
            return pMac->hphy.rf.filterCalRegContext.chanFilterSettings[chan].rxIf;
        }
        else
        {
            if (searchRxCal(pMac, chan, bound1, bound2) != eHAL_STATUS_SUCCESS) { return 0x80; }
            return pMac->hphy.rf.filterCalRegContext.chanFilterSettings[chan].rxIf;
        }        
    }
    
    return 0xFF;
}
             
static tANI_U8 transitPoints[MAX_LEGIT_RF_CHANNEL+1];
static tGrabRamSample *sampleBuffer;
static rfQuasarFilterCalParams *calParams;

static void binaryFilterCal(tpAniSirGlobal pMac, tANI_U32 mode, tANI_U32 first, tANI_U32 last, tANI_U32 bound1, tANI_U32 bound2)
{
    bound1 = first;
    bound2 = last;
    if(getTuneVal(pMac, last, mode, bound1, bound2) != getTuneVal(pMac, first, mode, bound1, bound2))
    {
        tANI_U32 temp = (last+first)/2;
        if(temp == first)
        {
            transitPoints[first] = 1; transitPoints[last] = 1;
        }            

        else
        {
            if(getTuneVal(pMac, temp, mode, bound1, bound2) != getTuneVal(pMac, last, mode, bound1, bound2))
            {
                binaryFilterCal(pMac, mode, temp, last, bound1, bound2);
            }

            if(getTuneVal(pMac, temp, mode, bound1, bound2) != getTuneVal(pMac, first, mode, bound1, bound2))
            {
                binaryFilterCal(pMac, mode, first, temp, bound1, bound2);
            }
        }
    }
}
    
static void update(tpAniSirGlobal pMac, tANI_U32 start, tANI_U32 stop, tANI_U32 mode)
{
    tANI_U32 i, j, firstIdx = start, nextIdx = 0;
    tANI_U32 length = (stop) + 1;
    for(i=start;i<length;i++)
    {
        while((i < length) && (transitPoints[i] == 0))
        {
            i++;
        }
        if(i == length)
            nextIdx = i - 1;
        else                
            nextIdx = i;
        if((nextIdx - firstIdx) == 1)
        {
            firstIdx = nextIdx;
            continue;
        }
        
        for(j=firstIdx; j<=nextIdx; j++)
        {
            if(mode == TX_IF)
            {
                pMac->hphy.rf.filterCalRegContext.chanFilterSettings[j].txIf = pMac->hphy.rf.filterCalRegContext.chanFilterSettings[firstIdx].txIf;
            }
            else if(mode == TX_RF)
            {
                pMac->hphy.rf.filterCalRegContext.chanFilterSettings[j].txRf = pMac->hphy.rf.filterCalRegContext.chanFilterSettings[firstIdx].txRf;
            }
            else if(mode == RX_IF)
            {
                pMac->hphy.rf.filterCalRegContext.chanFilterSettings[j].rxIf = pMac->hphy.rf.filterCalRegContext.chanFilterSettings[firstIdx].rxIf;
            }            
        }
        firstIdx = nextIdx;            
    }    
}

eHalStatus rfQuasarFilterCal(tpAniSirGlobal pMac, tQuasarFilterSettings *pChanFilterSettings)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
    eRfChannels curChan = rfGetCurChannel(pMac);
    tGrabRamSample *pSampleBuffer;
    tANI_U8 *pFFs;
    tANI_U32 i, chan, chanMinIndex, chanMaxIndex, chanMinCBIndex, chanMaxCBIndex;
    tANI_U8 productBands;
    tRxDcoCorrect dcoSaved0, dcoSaved1;
    //tQuasarFilterSettings chanFilterSettings;
    tANI_U8 rxGainVals[NUM_RF_SUBBANDS] = { 113, 121, 121, 121, 121 };
    tANI_U8 txGainVals[NUM_RF_SUBBANDS] = { 2, 2, 2, 2, 2 };
    rfQuasarFilterCalParams sCalParams;
    {
        palCopyMemory( pMac->hHdd, &(sCalParams.rxGainVal[0]), &(rxGainVals[0]), NUM_RF_SUBBANDS );
        palCopyMemory( pMac->hHdd, &(sCalParams.txGainVal[0]), &(txGainVals[0]), NUM_RF_SUBBANDS );
        sCalParams.tx_gain = 8;
        sCalParams.rx_gain = 19;
        sCalParams.cal_gain_index = 40;  
    }    
    calParams = &sCalParams;
    
    assert(pChanFilterSettings != NULL);
    
    palFillMemory( pMac->hHdd, &(pMac->hphy.rf.filterCalRegContext.chanFilterSettings), sizeof(tQuasarFilterSettings), 0xFF );
    
    if ((retVal = palAllocateMemory(pMac->hHdd, (void **)&pSampleBuffer, (MAX_NUM_SAMPLES * sizeof(tGrabRamSample)))) != eHAL_STATUS_SUCCESS)
    {
        phyLog(pMac, LOGE, "ERROR: Unable to allocate memory for rxIf filter tuning\n");
        return retVal; 
    }    
    sampleBuffer = pSampleBuffer;

    if ((retVal = rfGetDCOffset(pMac, PHY_RX_CHAIN_1, calParams->cal_gain_index, &dcoSaved1)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    if ((retVal = rfGetDCOffset(pMac, PHY_RX_CHAIN_0, calParams->cal_gain_index, &dcoSaved0)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    
    if ((retVal = halReadEepromField(pMac, EEPROM_COMMON_PRODUCT_BANDS, (uEepromFields *)&productBands)) != eHAL_STATUS_SUCCESS) { goto done; }

    if(productBands >= NUM_PRODUCT_BANDS)
    {
        phyLog(pMac, LOGE, "ERROR: Invalid EEPROM_COMMON_PRODUCT_BANDS\n");
        return eHAL_STATUS_FAILURE; 
    }
    
    if(productBands == PRODUCT_BAND_11_B_G)
    {
        chanMinIndex = MIN_2_4GHZ_CHANNEL;
        chanMaxIndex = MAX_2_4GHZ_CHANNEL;
        chanMinCBIndex = MIN_CB_2_4GHZ_CHANNEL;
        chanMaxCBIndex = MAX_CB_2_4GHZ_CHANNEL;
    }
    else if(productBands == PRODUCT_BAND_11_A_B_G)
    {
        chanMinIndex = MIN_2_4GHZ_CHANNEL;
        chanMaxIndex = MAX_5GHZ_CHANNEL;
        chanMinCBIndex = MIN_CHAN_BOND_CHANNEL;
        chanMaxCBIndex = MAX_CHAN_BOND_CHANNEL;
    }
    else
    {
        chanMinIndex = MIN_5GHZ_CHANNEL;
        chanMaxIndex = MAX_5GHZ_CHANNEL;
        chanMinCBIndex = MIN_CB_5GHZ_CHANNEL;
        chanMaxCBIndex = MAX_CB_5GHZ_CHANNEL;
    }
    
    if(pMac->hphy.phy.test.testDisableFilterCalOptimization == eANI_BOOLEAN_FALSE)
    {
        //need to do some work here, generalize based on the channel indices above.
        if((productBands == PRODUCT_BAND_11_A_B_G) || (productBands == PRODUCT_BAND_11_A))
        {
            //txRf ==> 5GHz
            palFillMemory( pMac->hHdd, transitPoints, (MAX_LEGIT_RF_CHANNEL+1), 0 );
            binaryFilterCal(pMac, TX_RF, RF_CHAN_240, RF_CHAN_165, RF_CHAN_240, RF_CHAN_165);
            update(pMac, RF_CHAN_240, RF_CHAN_165, TX_RF);

            //txIf ==> 5GHz
            palFillMemory( pMac->hHdd, transitPoints, (MAX_LEGIT_RF_CHANNEL+1), 0 );
            binaryFilterCal(pMac, TX_IF, RF_CHAN_240, RF_CHAN_165, RF_CHAN_240, RF_CHAN_165);
            update(pMac, RF_CHAN_240, RF_CHAN_165, TX_IF);    

            //rxIf ==> 5GHz
            palFillMemory( pMac->hHdd, transitPoints, (MAX_LEGIT_RF_CHANNEL+1), 0 );
            binaryFilterCal(pMac, RX_IF, RF_CHAN_240, RF_CHAN_165, RF_CHAN_240, RF_CHAN_165);
            update(pMac, RF_CHAN_240, RF_CHAN_165, RX_IF);        
        }

        if((productBands == PRODUCT_BAND_11_B_G) || (productBands == PRODUCT_BAND_11_A_B_G))
        {
            //txRf ==> 2.4GHz
            palFillMemory( pMac->hHdd, transitPoints, (MAX_LEGIT_RF_CHANNEL+1), 0 );
            binaryFilterCal(pMac, TX_RF, RF_CHAN_1, RF_CHAN_14, RF_CHAN_1, RF_CHAN_14);
            update(pMac, RF_CHAN_1, RF_CHAN_14, TX_RF);

            //txIf ==> 2.4GHz
            palFillMemory( pMac->hHdd, transitPoints, (MAX_LEGIT_RF_CHANNEL+1), 0 );
            binaryFilterCal(pMac, TX_IF, RF_CHAN_1, RF_CHAN_14, RF_CHAN_1, RF_CHAN_14);
            update(pMac, RF_CHAN_1, RF_CHAN_14, TX_IF);    

            //rxIf ==> 2.4GHz
            palFillMemory( pMac->hHdd, transitPoints, (MAX_LEGIT_RF_CHANNEL+1), 0 );
            binaryFilterCal(pMac, RX_IF, RF_CHAN_1, RF_CHAN_14, RF_CHAN_1, RF_CHAN_14);
            update(pMac, RF_CHAN_1, RF_CHAN_14, RX_IF);            
        }
    }
    
    for (chan = 0; chan < NUM_RF_CHANNELS; chan++)
    {
        if( !(((chan >= chanMinIndex) && (chan <= chanMaxIndex)) || ((chan >= chanMinCBIndex) && (chan <= chanMaxCBIndex))) )
        {
            continue;
        }
        switch (chan)
        {
            case RF_CHAN_BOND_3:
            case RF_CHAN_BOND_4:
            case RF_CHAN_BOND_5:
            case RF_CHAN_BOND_6:
            case RF_CHAN_BOND_7:
            case RF_CHAN_BOND_8:
            case RF_CHAN_BOND_9:
            case RF_CHAN_BOND_10:
            case RF_CHAN_BOND_11:
                pMac->hphy.rf.filterCalRegContext.chanFilterSettings[chan].txIf = pMac->hphy.rf.filterCalRegContext.chanFilterSettings[chan - (RF_CHAN_BOND_3 - RF_CHAN_3)].txIf;
                pMac->hphy.rf.filterCalRegContext.chanFilterSettings[chan].txRf = pMac->hphy.rf.filterCalRegContext.chanFilterSettings[chan - (RF_CHAN_BOND_3 - RF_CHAN_3)].txRf;
                pMac->hphy.rf.filterCalRegContext.chanFilterSettings[chan].rxIf = pMac->hphy.rf.filterCalRegContext.chanFilterSettings[chan - (RF_CHAN_BOND_3 - RF_CHAN_3)].rxIf;
                continue;

            case RF_CHAN_BOND_242:
            case RF_CHAN_BOND_246:
            case RF_CHAN_BOND_250:                
                pMac->hphy.rf.filterCalRegContext.chanFilterSettings[chan].txIf = pMac->hphy.rf.filterCalRegContext.chanFilterSettings[chan - (RF_CHAN_BOND_242 - RF_CHAN_240)].txIf;
                pMac->hphy.rf.filterCalRegContext.chanFilterSettings[chan].txRf = pMac->hphy.rf.filterCalRegContext.chanFilterSettings[chan - (RF_CHAN_BOND_242 - RF_CHAN_240)].txRf;
                pMac->hphy.rf.filterCalRegContext.chanFilterSettings[chan].rxIf = pMac->hphy.rf.filterCalRegContext.chanFilterSettings[chan - (RF_CHAN_BOND_242 - RF_CHAN_240)].rxIf;
                continue;
        
            case RF_CHAN_BOND_210:
            case RF_CHAN_BOND_214:
                pMac->hphy.rf.filterCalRegContext.chanFilterSettings[chan].txIf = pMac->hphy.rf.filterCalRegContext.chanFilterSettings[chan - (RF_CHAN_BOND_210 - RF_CHAN_208)].txIf;
                pMac->hphy.rf.filterCalRegContext.chanFilterSettings[chan].txRf = pMac->hphy.rf.filterCalRegContext.chanFilterSettings[chan - (RF_CHAN_BOND_210 - RF_CHAN_208)].txRf;
                pMac->hphy.rf.filterCalRegContext.chanFilterSettings[chan].rxIf = pMac->hphy.rf.filterCalRegContext.chanFilterSettings[chan - (RF_CHAN_BOND_210 - RF_CHAN_208)].rxIf;
                continue;
        
            case RF_CHAN_BOND_38:
            case RF_CHAN_BOND_42:
            case RF_CHAN_BOND_46:
            case RF_CHAN_BOND_50:
            case RF_CHAN_BOND_54:
            case RF_CHAN_BOND_58:
            case RF_CHAN_BOND_62:
                pMac->hphy.rf.filterCalRegContext.chanFilterSettings[chan].txIf = pMac->hphy.rf.filterCalRegContext.chanFilterSettings[chan - (RF_CHAN_BOND_38 - RF_CHAN_36)].txIf;
                pMac->hphy.rf.filterCalRegContext.chanFilterSettings[chan].txRf = pMac->hphy.rf.filterCalRegContext.chanFilterSettings[chan - (RF_CHAN_BOND_38 - RF_CHAN_36)].txRf;
                pMac->hphy.rf.filterCalRegContext.chanFilterSettings[chan].rxIf = pMac->hphy.rf.filterCalRegContext.chanFilterSettings[chan - (RF_CHAN_BOND_38 - RF_CHAN_36)].rxIf;
                continue;

            case RF_CHAN_BOND_102:
            case RF_CHAN_BOND_106:
            case RF_CHAN_BOND_110:
            case RF_CHAN_BOND_114:
            case RF_CHAN_BOND_118:
            case RF_CHAN_BOND_122:
            case RF_CHAN_BOND_126:
            case RF_CHAN_BOND_130:
            case RF_CHAN_BOND_134:
            case RF_CHAN_BOND_138:
                pMac->hphy.rf.filterCalRegContext.chanFilterSettings[chan].txIf = pMac->hphy.rf.filterCalRegContext.chanFilterSettings[chan - (RF_CHAN_BOND_102 - RF_CHAN_100)].txIf;
                pMac->hphy.rf.filterCalRegContext.chanFilterSettings[chan].txRf = pMac->hphy.rf.filterCalRegContext.chanFilterSettings[chan - (RF_CHAN_BOND_102 - RF_CHAN_100)].txRf;
                pMac->hphy.rf.filterCalRegContext.chanFilterSettings[chan].rxIf = pMac->hphy.rf.filterCalRegContext.chanFilterSettings[chan - (RF_CHAN_BOND_102 - RF_CHAN_100)].rxIf;
                continue;

            case RF_CHAN_BOND_151:
            case RF_CHAN_BOND_155:
            case RF_CHAN_BOND_159:
            case RF_CHAN_BOND_163:
                pMac->hphy.rf.filterCalRegContext.chanFilterSettings[chan].txIf = pMac->hphy.rf.filterCalRegContext.chanFilterSettings[chan - (RF_CHAN_BOND_151 - RF_CHAN_149)].txIf;
                pMac->hphy.rf.filterCalRegContext.chanFilterSettings[chan].txRf = pMac->hphy.rf.filterCalRegContext.chanFilterSettings[chan - (RF_CHAN_BOND_151 - RF_CHAN_149)].txRf;
                pMac->hphy.rf.filterCalRegContext.chanFilterSettings[chan].rxIf = pMac->hphy.rf.filterCalRegContext.chanFilterSettings[chan - (RF_CHAN_BOND_151 - RF_CHAN_149)].rxIf;
                continue;

            default:
            break;
        }
        
        if(pMac->hphy.phy.test.testDisableFilterCalOptimization == eANI_BOOLEAN_TRUE)
        {
            if ((retVal = searchTxCal(pMac, chan, chan, chan)) != eHAL_STATUS_SUCCESS) { goto done; }
            if ((retVal = searchRxCal(pMac, chan, chan, chan)) != eHAL_STATUS_SUCCESS) { goto done; }
        }
    }

done:
    asicCeaseOverrideAGCRxChainGain(pMac, PHY_ALL_RX_CHAINS );
    if ((retVal = rfSetDCOffset(pMac, PHY_RX_CHAIN_1, calParams->cal_gain_index, dcoSaved1)) != eHAL_STATUS_SUCCESS) { return (retVal); }    
    if ((retVal = rfSetDCOffset(pMac, PHY_RX_CHAIN_0, calParams->cal_gain_index, dcoSaved0)) != eHAL_STATUS_SUCCESS) { return (retVal); }    
    if ((retVal = rfSetCurChannel(pMac, curChan)) != eHAL_STATUS_SUCCESS) { return (retVal); }

    calParams = NULL;
    sampleBuffer = NULL;
    palFreeMemory(pMac->hHdd, pSampleBuffer);
    //write back all the FF's to 0
    pFFs = (tANI_U8 *)&(pMac->hphy.rf.filterCalRegContext.chanFilterSettings);
    for(i=0; i<sizeof(tQuasarFilterSettings);i++)
    {
        if(pFFs[i] == 0xFF)
        {
            pFFs[i] = 0;
        }
        
    }
    palCopyMemory( pMac->hHdd, pChanFilterSettings, &(pMac->hphy.rf.filterCalRegContext.chanFilterSettings), sizeof(tQuasarFilterSettings) );
        
    return retVal;
}

static eHalStatus searchTxCal(tpAniSirGlobal pMac, tANI_U32 chan, tANI_U32 bound1, tANI_U32 bound2)
{
    //search_tx_cal
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
    tANI_U8 txChain, startIfIdx = 0, startRfIdx = 0;
    tANI_U8 rxChain;
    tANI_U8 confidence[PHY_MAX_TX_CHAINS];
    tANI_U8 maxPwrIfIndex[PHY_MAX_TX_CHAINS];
    tANI_U8 maxPwrRfIndex[PHY_MAX_TX_CHAINS];
    tANI_U32 val, numIfVals, numRfVals, regVal, cfg5Val;
    eRfSubBand curSubband;

    assert(chan <= MAX_CHAN_BOND_CHANNEL);
    assert(bound1 <= MAX_CHAN_BOND_CHANNEL);
    assert(bound2 <= MAX_CHAN_BOND_CHANNEL);    
    
    if ((retVal = rfSetCurChannel(pMac, chan)) == eHAL_STATUS_SUCCESS) 
    {
/*
        // The below register writes are done in the optimized way below. Retaining this legacy code for reference
        if ((retVal = rfSetChanBondMode(pMac, eANI_BOOLEAN_TRUE)) != eHAL_STATUS_SUCCESS) { return retVal; }
        rfWriteQuasarField(pMac, QUASAR_FIELD_BBF_RX_TUNE, 1);
        rfWriteQuasarField(pMac, QUASAR_FIELD_BBF_TX_TUNE, 1);
*/

        retVal = rfReadDataField(pMac, QUASAR_REG_CFG_2, MSK_32, 0, &regVal);
        if (retVal != eHAL_STATUS_SUCCESS)
        {
            return (retVal);
        }

        regVal &= ~((QUASAR_BBF_RX_TUNE_MASK << QUASAR_BBF_RX_TUNE_OFFSET) 
                    | (QUASAR_BBF_TX_TUNE_MASK << QUASAR_BBF_TX_TUNE_OFFSET));
        regVal |= (1 & QUASAR_BBF_RX_TUNE_MASK) << QUASAR_BBF_RX_TUNE_OFFSET;
        regVal |= (1 & QUASAR_BBF_TX_TUNE_MASK) << QUASAR_BBF_TX_TUNE_OFFSET;

        retVal = rfWriteDataField(pMac, QUASAR_REG_CFG_2, MSK_32, 0, regVal);
        if (retVal != eHAL_STATUS_SUCCESS)
        {
            return (retVal);
        }

    }
    
    curSubband = rfGetBand(pMac, chan);
    
    for (txChain = 0; txChain < PHY_MAX_TX_CHAINS; txChain++)
    {
        phyLog(pMac, LOG2, "\nTX%d:\n", txChain);
        
        rfQuasarCalInit(pMac);

        retVal = rfReadDataField(pMac, QUASAR_REG_TEST_1, MSK_32, 0, &regVal);
        if (retVal != eHAL_STATUS_SUCCESS)
        {
            return (retVal);
        }

//        regVal &= 0x27331;
        
        regVal &= (QUASAR_TEST1_NDIV_OUT_MASK | QUASAR_TEST1_TEST_LB_RF_MASK | QUASAR_TEST1_RX_IQ_TEST_2_0_MASK 
                        | QUASAR_TEST1_RX_IQ_TEST_2_0_MASK | QUASAR_TEST1_DET_OFFSET_MASK | QUASAR_TEST1_RDIV_TEST_MASK);

        if (txChain == 0)
        {
            rxChain = 1;
            regVal |= QUASAR_TEST1_RX_IQ_TEST_1_MASK;
//            if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_RX_IQ_TEST_1, 1)) != eHAL_STATUS_SUCCESS) { return retVal; }
//            if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_RX_IQ_TEST_0, 0)) != eHAL_STATUS_SUCCESS) { return retVal; }
        }
        else //TODO: if we have a tx1
        {
            rxChain = 0;
            regVal |= QUASAR_TEST1_RX_IQ_TEST_0_MASK;
//            if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_RX_IQ_TEST_1, 0)) != eHAL_STATUS_SUCCESS) { return retVal; }
//            if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_RX_IQ_TEST_0, 1)) != eHAL_STATUS_SUCCESS) { return retVal; }
        }

        {
            //setup_tx
            tANI_U8 rx_gain_val;

            if ((retVal = asicSetAGCCrossbar(pMac, AGC_ALL_RX)) != eHAL_STATUS_SUCCESS) { return retVal; }
            if ((retVal = asicSetDisabledRxPacketTypes(pMac, PHY_RX_DISABLE_ALL_TYPES)) != eHAL_STATUS_SUCCESS) { return retVal; }
/*
            // The below register writes are done in the optimized way below. Retaining this legacy code for reference

            if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_TX_CAL_EN_0, 0)) != eHAL_STATUS_SUCCESS) { return retVal; }
            if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_TX_CAL_EN_1, 0)) != eHAL_STATUS_SUCCESS) { return retVal; }
            if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_RX_LB_GAIN, 1)) != eHAL_STATUS_SUCCESS) { return retVal; }
            if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_TX_LB_GAIN, 1)) != eHAL_STATUS_SUCCESS) { return retVal; }
            if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_DET_GAIN, 1)) != eHAL_STATUS_SUCCESS) { return retVal; }
*/

            regVal |= (QUASAR_TEST1_RX_LB_GAIN_MASK | QUASAR_TEST1_TX_LB_GAIN_MASK | 
                                ((1 & QUASAR_DET_GAIN_MASK) << QUASAR_DET_GAIN_OFFSET));

            retVal = rfWriteDataField(pMac, QUASAR_REG_TEST_1, MSK_32, 0, regVal);
            if (retVal != eHAL_STATUS_SUCCESS)
            {
                return (retVal);
            }

            rx_gain_val = calParams->rxGainVal[curSubband];

            if (txChain == 0)
            {
                SET_PHY_REG(pMac->hHdd, AGC_RX_OVERRIDE_REG, 0x702);
                if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_SPI_LUT_INDEX, calParams->cal_gain_index)) != eHAL_STATUS_SUCCESS) { return retVal; }
                if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_SPI_LUT_EN, 1)) != eHAL_STATUS_SUCCESS) { return retVal; }
                if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_RX_GAIN_1, rx_gain_val)) != eHAL_STATUS_SUCCESS) { return retVal; }
            }
            else
            {
                SET_PHY_REG(pMac->hHdd, AGC_RX_OVERRIDE_REG, 0x701);
                if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_SPI_LUT_INDEX, calParams->cal_gain_index)) != eHAL_STATUS_SUCCESS) { return retVal; }
                if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_SPI_LUT_EN, 1)) != eHAL_STATUS_SUCCESS) { return retVal; }
                if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_RX_GAIN_0, rx_gain_val)) != eHAL_STATUS_SUCCESS) { return retVal; }
            }

            GET_PHY_REG(pMac->hHdd, RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, &val);
            val = val | RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_XBAR_MASK|RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_CAL_MASK|RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_RXFIR_MASK;
            SET_PHY_REG(pMac->hHdd, RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, val);
        }

        {
            if ((retVal = asicSetTxDACs(pMac, PHY_ALL_TX_CHAINS, eANI_BOOLEAN_TRUE, eANI_BOOLEAN_FALSE)) != eHAL_STATUS_SUCCESS) { return (retVal); }
            if ((retVal = calibrateLoopbackDCO(pMac, rxChain, calParams->cal_gain_index)) != eHAL_STATUS_SUCCESS) { return retVal; }
        
        }

        {
            //need to select appropriate list of rf & if values to try
            struct listVals
            {
                tANI_U32 numVals;
                tANI_U8 list[8];
            };
        
            struct listVals ifVals2G = { 3, { 0, 1, 3 }};
            struct listVals rfVals2G = { 3, { 0, 1, 3 }};
            struct listVals ifVals5G = { 4, { 0, 1, 3, 7 }};
            struct listVals rfVals5G = { 8, { 0, 1, 2, 3, 4, 5, 6, 7 }};
            struct listVals *pIfVals;
            struct listVals *pRfVals;

            tANI_U8 if_rf_indices[8] = {0, 1, 0xFF, 2, 0xFF, 0xFF, 0xFF, 3};
            //start_waveform(tx_chain)
            if ((retVal = asicSetTxDACs(pMac, !txChain, eANI_BOOLEAN_TRUE, eANI_BOOLEAN_TRUE)) != eHAL_STATUS_SUCCESS) { return (retVal); }
            if ((retVal = asicSetupTestWaveform(pMac, wfmTxCal, sizeof(wfmTxCal) / sizeof(tWaveformSample), eANI_BOOLEAN_TRUE)) != eHAL_STATUS_SUCCESS) { return retVal; }
            if ((retVal = asicStartTestWaveform(pMac, WAVE_CONTINUOUS, 0, ((sizeof(wfmTxCal) / sizeof(tWaveformSample)) - 1))) != eHAL_STATUS_SUCCESS) { return retVal; }
            if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_GC_SELECT, 1)) != eHAL_STATUS_SUCCESS) { return retVal; } //'tx'

            //if (((chan >= MIN_2_4GHZ_CHANNEL) && (chan <= MAX_2_4GHZ_CHANNEL)) || ((chan >= RF_CHAN_BOND_3) && (chan <= RF_CHAN_BOND_11)) )
            if(curSubband == RF_SUBBAND_2_4_GHZ)
            {
                //2.4GHz channel
                pIfVals = &ifVals2G;
                pRfVals = &rfVals2G;
                numIfVals = 3;
                numRfVals = 3;
                if( (pMac->hphy.rf.filterCalRegContext.chanFilterSettings[bound1].txRf != pMac->hphy.rf.filterCalRegContext.chanFilterSettings[bound2].txRf) && (pMac->hphy.rf.filterCalRegContext.chanFilterSettings[bound1].txRf != 0xFF) && (pMac->hphy.rf.filterCalRegContext.chanFilterSettings[bound2].txRf != 0xFF) )
                {
                    startRfIdx = if_rf_indices[pMac->hphy.rf.filterCalRegContext.chanFilterSettings[bound2].txRf];
                    pRfVals->numVals = (if_rf_indices[pMac->hphy.rf.filterCalRegContext.chanFilterSettings[bound1].txRf]) + 1;                
                }
            }
            else
            {
                //5GHz channel
                pIfVals = &ifVals5G;
                pRfVals = &rfVals5G;
                numIfVals = 4;
                numRfVals = 8;
                if( (pMac->hphy.rf.filterCalRegContext.chanFilterSettings[bound1].txRf != pMac->hphy.rf.filterCalRegContext.chanFilterSettings[bound2].txRf) && (pMac->hphy.rf.filterCalRegContext.chanFilterSettings[bound1].txRf != 0xFF) && (pMac->hphy.rf.filterCalRegContext.chanFilterSettings[bound2].txRf != 0xFF) )
                {
                    startRfIdx = pMac->hphy.rf.filterCalRegContext.chanFilterSettings[bound2].txRf;
                    pRfVals->numVals = (pMac->hphy.rf.filterCalRegContext.chanFilterSettings[bound1].txRf) + 1;                
                }
            }
            
            if( (pMac->hphy.rf.filterCalRegContext.chanFilterSettings[bound1].txIf != pMac->hphy.rf.filterCalRegContext.chanFilterSettings[bound2].txIf) && (pMac->hphy.rf.filterCalRegContext.chanFilterSettings[bound1].txIf != 0xFF) && (pMac->hphy.rf.filterCalRegContext.chanFilterSettings[bound2].txIf != 0xFF) )
            {
                startIfIdx = if_rf_indices[pMac->hphy.rf.filterCalRegContext.chanFilterSettings[bound2].txIf];
                pIfVals->numVals = (if_rf_indices[pMac->hphy.rf.filterCalRegContext.chanFilterSettings[bound1].txIf]) + 1;                
            }
            
            {
                tTxGain txGain = { calParams->txGainVal[curSubband], 0 };
                tTxGain otherGain = { 0, 0 };
                tANI_U8 ifIndex, rfIndex;
                tANI_U32 pwr = 0;
                tANI_U32 maxPwr = 0;
                tANI_U32 nextHighestPwr = 0;
                tANI_U32 totalPwr = 0;

                if ((retVal = asicTPCPowerOverride(pMac, txGain, otherGain)) != eHAL_STATUS_SUCCESS) { return retVal; }
                //if ((retVal = asicTxGainOverride(pMac, txChain, (tANI_U8)txGain.coarsePwr, 0)) != eHAL_STATUS_SUCCESS) { return retVal; } 
                
                if ((retVal = rdModWrNovaField(pMac, AGC_BANDWIDTH_CONFIG_REG, AGC_BANDWIDTH_CONFIG_CB_ENABLE_MASK,
                             AGC_BANDWIDTH_CONFIG_CB_ENABLE_OFFSET, 1)) != eHAL_STATUS_SUCCESS)
                {  return retVal; }
        
                retVal = rfReadDataField(pMac, QUASAR_REG_CFG_5, MSK_32, 0, &cfg5Val);
                if (retVal != eHAL_STATUS_SUCCESS)
                {
                    return (retVal);
                }
                for (ifIndex = startIfIdx; ifIndex < pIfVals->numVals; ifIndex++)
                {
                    for (rfIndex = startRfIdx; rfIndex < pRfVals->numVals; rfIndex++)
                    {
                        // The below register writes are done in the optimized way below. Retaining this legacy code for reference    
                        //if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_TX_IF_TUN, pIfVals->list[ifIndex])) != eHAL_STATUS_SUCCESS) { return retVal; }
                        //if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_TX_RF_TUN, pRfVals->list[rfIndex])) != eHAL_STATUS_SUCCESS) { return retVal; }
                        cfg5Val &= ~((QUASAR_TX_RF_TUN_MASK << QUASAR_TX_RF_TUN_OFFSET) | 
                                    (QUASAR_TX_IF_TUN_MASK << QUASAR_TX_IF_TUN_OFFSET));
                        
                        cfg5Val |= (((pIfVals->list[ifIndex] & QUASAR_TX_IF_TUN_MASK) << QUASAR_TX_IF_TUN_OFFSET)
                                    | ((pRfVals->list[rfIndex] & QUASAR_TX_RF_TUN_MASK) << QUASAR_TX_RF_TUN_OFFSET));

                        retVal = rfWriteDataField(pMac, QUASAR_REG_CFG_5, MSK_32, 0, cfg5Val);
                        if (retVal != eHAL_STATUS_SUCCESS)
                        {
                            return (retVal);
                        }
                        if ((retVal = MeasureRxLoopbackPower(pMac, rxChain, &pwr, sampleBuffer, 1024)) != eHAL_STATUS_SUCCESS) { return retVal; }
                        phyLog(pMac, LOG4, "\tifIndex = %d, rfIndex = %d  pwr = %d:\n", ifIndex, rfIndex, pwr);

                        if (pwr > maxPwr)
                        {
                            maxPwrIfIndex[txChain] = pIfVals->list[ifIndex];//ifIndex;
                            maxPwrRfIndex[txChain] = pRfVals->list[rfIndex];//rfIndex;
                            nextHighestPwr = maxPwr;
                            maxPwr = pwr;
                            phyLog(pMac, LOG3, "\t\tBetter-> ifIndex = %d, rfIndex = %d  pwrSample = %d:\n", ifIndex, rfIndex, pwr);
                        }
                        totalPwr += pwr;
                    }
                }
                
                if ((totalPwr == 0) || ((pIfVals->numVals - startIfIdx) <= 0) || ((pRfVals->numVals - startRfIdx) <= 0))
                {
                    phyLog(pMac, LOGE, "Error computing the confidence values\n");
                    phyLog(pMac, LOGE, "Terminating the filter cal to avoid divide by zero exception\n");
                    return eHAL_STATUS_FAILURE;
                }
                else
                {
                    {
                        tANI_U32 meanNextPwr = ((totalPwr/* - nextHighestPwr*/) / ((pIfVals->numVals - startIfIdx) * (pRfVals->numVals - startRfIdx)));
                        confidence[txChain] = (tANI_U8) ((maxPwr - nextHighestPwr) / (meanNextPwr * 144)); //per Kapil's email July 13, 2007
                    }

                    phyLog(pMac, LOG3, "\t\t\tmaxPwr = %d, nextPwr = %d, conf = %d\n", maxPwr, nextHighestPwr, confidence);
                    phyLog(pMac, LOG2, "\t\t\tRecorded-> ifIndex = %d, rfIndex = %d\n", maxPwrIfIndex, maxPwrRfIndex);                    
                }
                
            
                if ((retVal = asicStopTestWaveform(pMac)) != eHAL_STATUS_SUCCESS) { return retVal; }
            
                {
                    //unsetup_tx
                    rfQuasarCalRestore(pMac);
                    //Commenting the following register writes, since this is already done in restore function
                    //if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_RX_IQ_TEST_1, 0)) != eHAL_STATUS_SUCCESS) { return retVal; }
                    //if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_RX_IQ_TEST_0, 0)) != eHAL_STATUS_SUCCESS) { return retVal; }
                    //if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_DET_GAIN, 0)) != eHAL_STATUS_SUCCESS) { return (retVal); }
                }
            }
        }
    }

    //now figure out which txChain has the best settings and set them accordingly
    if (confidence[PHY_TX_CHAIN_0] > confidence[PHY_TX_CHAIN_1])
    {
        pMac->hphy.rf.filterCalRegContext.chanFilterSettings[chan].txIf = maxPwrIfIndex[PHY_TX_CHAIN_0];
        pMac->hphy.rf.filterCalRegContext.chanFilterSettings[chan].txRf = maxPwrRfIndex[PHY_TX_CHAIN_0];
    }
    else
    {
        pMac->hphy.rf.filterCalRegContext.chanFilterSettings[chan].txIf = maxPwrIfIndex[PHY_TX_CHAIN_1];
        pMac->hphy.rf.filterCalRegContext.chanFilterSettings[chan].txRf = maxPwrRfIndex[PHY_TX_CHAIN_1];
    }

    // The below register writes are done in the optimized way below. Retaining this legacy code for reference        
    //if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_TX_IF_TUN, pMac->hphy.rf.filterCalRegContext.chanFilterSettings[chan].txIf/*;txFilterSettings->txIf*/)) != eHAL_STATUS_SUCCESS) { return retVal; }
    //if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_TX_RF_TUN, pMac->hphy.rf.filterCalRegContext.chanFilterSettings[chan].txRf/*;txFilterSettings->txRf*/)) != eHAL_STATUS_SUCCESS) { return retVal; }
    cfg5Val &= ~((QUASAR_RX_IF_TUN_MASK << QUASAR_RX_IF_TUN_OFFSET) | 
                (QUASAR_TX_IF_TUN_MASK << QUASAR_TX_IF_TUN_OFFSET));
    
    cfg5Val |= (((pMac->hphy.rf.filterCalRegContext.chanFilterSettings[chan].txIf & QUASAR_TX_IF_TUN_MASK) << QUASAR_TX_IF_TUN_OFFSET)
                | ((pMac->hphy.rf.filterCalRegContext.chanFilterSettings[chan].txRf & QUASAR_TX_RF_TUN_MASK) << QUASAR_TX_RF_TUN_OFFSET));
    
    retVal = rfWriteDataField(pMac, QUASAR_REG_CFG_5, MSK_32, 0, cfg5Val);
    if (retVal != eHAL_STATUS_SUCCESS)
    {
        return (retVal);
    }
    
    
    return retVal;
}

static eHalStatus searchRxCal(tpAniSirGlobal pMac, tANI_U32 chan, tANI_U32 bound1, tANI_U32 bound2)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
    tANI_U8 txChain = 0, startIdx = 0;
    tANI_U8 rxChain = 1;
    tANI_U8 numTuneVals, range, rx_gain_val;
    tANI_U32 val = 0, regVal;
    eRfSubBand curSubband;
 
    assert(chan <= MAX_CHAN_BOND_CHANNEL);
    assert(bound1 <= MAX_CHAN_BOND_CHANNEL);
    assert(bound2 <= MAX_CHAN_BOND_CHANNEL);    
    
    if ((retVal = rfSetCurChannel(pMac, chan)) == eHAL_STATUS_SUCCESS) 
    {
/*
        // The below register writes are done in the optimized way below. Retaining this legacy code for reference    
        if ((retVal = rfSetChanBondMode(pMac, eANI_BOOLEAN_TRUE)) != eHAL_STATUS_SUCCESS) { return retVal; }
        rfWriteQuasarField(pMac, QUASAR_FIELD_BBF_RX_TUNE, 1);
        rfWriteQuasarField(pMac, QUASAR_FIELD_BBF_TX_TUNE, 1);
*/

        retVal = rfReadDataField(pMac, QUASAR_REG_CFG_2, MSK_32, 0, &regVal);
        if (retVal != eHAL_STATUS_SUCCESS)
        {
            return (retVal);
        }
        
        regVal &= ~((QUASAR_BBF_RX_TUNE_MASK << QUASAR_BBF_RX_TUNE_OFFSET) 
                    | (QUASAR_BBF_TX_TUNE_MASK << QUASAR_BBF_TX_TUNE_OFFSET));
        regVal |= (1 & QUASAR_BBF_RX_TUNE_MASK) << QUASAR_BBF_RX_TUNE_OFFSET;
        regVal |= (1 & QUASAR_BBF_TX_TUNE_MASK) << QUASAR_BBF_TX_TUNE_OFFSET;
        
        retVal = rfWriteDataField(pMac, QUASAR_REG_CFG_2, MSK_32, 0, regVal);
        if (retVal != eHAL_STATUS_SUCCESS)
        {
            return (retVal);
        }
    }
    
    curSubband = rfGetBand(pMac, chan);
    rx_gain_val = calParams->rxGainVal[curSubband];
    
    rfQuasarCalInit(pMac);

    if(curSubband == RF_SUBBAND_2_4_GHZ)
    {
        numTuneVals = 8;
    }
    else
    {
        numTuneVals = 16;
    }
    
    //judge based on the values here
    if( (pMac->hphy.rf.filterCalRegContext.chanFilterSettings[bound1].rxIf != pMac->hphy.rf.filterCalRegContext.chanFilterSettings[bound2].rxIf) && (pMac->hphy.rf.filterCalRegContext.chanFilterSettings[bound1].rxIf != 0xFF) && (pMac->hphy.rf.filterCalRegContext.chanFilterSettings[bound2].rxIf != 0xFF) )
    {
        startIdx = pMac->hphy.rf.filterCalRegContext.chanFilterSettings[bound1].rxIf;
        numTuneVals = pMac->hphy.rf.filterCalRegContext.chanFilterSettings[bound2].rxIf + 1;
    }

    retVal = rfReadDataField(pMac, QUASAR_REG_TEST_1, MSK_32, 0, &regVal);
    if (retVal != eHAL_STATUS_SUCCESS)
    {
        return (retVal);
    }
    
//    regVal &= 0x2733F;

    regVal &= (QUASAR_TEST1_NDIV_OUT_MASK | QUASAR_TEST1_TEST_LB_RF_MASK | QUASAR_TEST1_RX_IQ_TEST_2_0_MASK 
                    | QUASAR_TEST1_RX_IQ_TEST_2_0_MASK | QUASAR_TEST1_DET_OFFSET_MASK 
                    | QUASAR_TEST1_DET_GAIN_MASK | QUASAR_TEST1_RDIV_TEST_MASK);

    if (rxChain == 1)
    {
        regVal |= QUASAR_TEST1_RX_IQ_TEST_1_MASK;
//        if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_RX_IQ_TEST_1, 1)) != eHAL_STATUS_SUCCESS) { return retVal; }
//        if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_RX_IQ_TEST_0, 0)) != eHAL_STATUS_SUCCESS) { return retVal; }
    }
    else
    {
        regVal |= QUASAR_TEST1_RX_IQ_TEST_0_MASK;
//        if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_RX_IQ_TEST_1, 0)) != eHAL_STATUS_SUCCESS) { return retVal; }
//        if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_RX_IQ_TEST_0, 1)) != eHAL_STATUS_SUCCESS) { return retVal; }
    }

    if ((retVal = asicSetAGCCrossbar(pMac, AGC_ALL_RX)) != eHAL_STATUS_SUCCESS) { return retVal; }
    if ((retVal = asicSetDisabledRxPacketTypes(pMac, PHY_RX_DISABLE_ALL_TYPES)) != eHAL_STATUS_SUCCESS) { return retVal; }
/* 
    // The below register writes are done in the optimized way below. Retaining this legacy code for reference    
    if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_TX_CAL_EN_0, 0)) != eHAL_STATUS_SUCCESS) { return retVal; }
    if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_TX_CAL_EN_1, 0)) != eHAL_STATUS_SUCCESS) { return retVal; }
    if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_RX_LB_GAIN, 1)) != eHAL_STATUS_SUCCESS) { return retVal; }
    if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_TX_LB_GAIN, 1)) != eHAL_STATUS_SUCCESS) { return retVal; }
*/

    regVal |= (QUASAR_TEST1_RX_LB_GAIN_MASK | QUASAR_TEST1_TX_LB_GAIN_MASK);
    
    retVal = rfWriteDataField(pMac, QUASAR_REG_TEST_1, MSK_32, 0, regVal);
    if (retVal != eHAL_STATUS_SUCCESS)
    {
        return (retVal);
    }
    if (txChain == 0)
    {
        SET_PHY_REG(pMac->hHdd, AGC_RX_OVERRIDE_REG, 0x702);
        if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_SPI_LUT_INDEX, calParams->cal_gain_index)) != eHAL_STATUS_SUCCESS) { return retVal; }
        if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_SPI_LUT_EN, 1)) != eHAL_STATUS_SUCCESS) { return retVal; }
        if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_RX_GAIN_1, rx_gain_val)) != eHAL_STATUS_SUCCESS) { return retVal; }
    }
    else
    {
        SET_PHY_REG(pMac->hHdd, AGC_RX_OVERRIDE_REG, 0x701);
        if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_SPI_LUT_INDEX, calParams->cal_gain_index)) != eHAL_STATUS_SUCCESS) { return retVal; }
        if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_SPI_LUT_EN, 1)) != eHAL_STATUS_SUCCESS) { return retVal; }
        if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_RX_GAIN_0, rx_gain_val)) != eHAL_STATUS_SUCCESS) { return retVal; }
    }

    //# disable dynamic clocks
    GET_PHY_REG(pMac->hHdd, RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, &val);
    val = val | RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_XBAR_MASK|RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_CAL_MASK|RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_RXFIR_MASK;
    SET_PHY_REG(pMac->hHdd, RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, val);
    
    if ((retVal = asicSetTxDACs(pMac, PHY_ALL_TX_CHAINS, eANI_BOOLEAN_TRUE, eANI_BOOLEAN_FALSE)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    if ((retVal = calibrateLoopbackDCO(pMac, rxChain, calParams->cal_gain_index)) != eHAL_STATUS_SUCCESS) { return retVal; }

    //start_waveform(tx_chain)
    if ((retVal = asicSetTxDACs(pMac, PHY_TX_CHAIN_1, eANI_BOOLEAN_TRUE, eANI_BOOLEAN_TRUE)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    if ((retVal = asicSetupTestWaveform(pMac, wfmTxCal, sizeof(wfmTxCal) / sizeof(tWaveformSample), eANI_BOOLEAN_TRUE)) != eHAL_STATUS_SUCCESS) { return retVal; }
    if ((retVal = asicStartTestWaveform(pMac, WAVE_CONTINUOUS, 0, ((sizeof(wfmTxCal) / sizeof(tWaveformSample)) - 1))) != eHAL_STATUS_SUCCESS) { return retVal; }
    
    if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_GC_SELECT, 1)) != eHAL_STATUS_SUCCESS) { return retVal; } //'tx'

    {
        tTxGain txGain = { calParams->txGainVal[curSubband], 0 };
        tTxGain otherGain = { 0, 0 };
        tANI_U32 pwr = 0;
        tANI_U32 optPwr = 0;
        tANI_U8 optRange = -1;

        if ((retVal = asicTPCPowerOverride(pMac, txGain, otherGain)) != eHAL_STATUS_SUCCESS) { return retVal; }
        //if ((retVal = asicTxGainOverride(pMac, txChain, (tANI_U8)txGain.coarsePwr, 0)) != eHAL_STATUS_SUCCESS) { return retVal; } 
        

        if ((retVal = rdModWrNovaField(pMac, AGC_BANDWIDTH_CONFIG_REG, AGC_BANDWIDTH_CONFIG_CB_ENABLE_MASK,
                     AGC_BANDWIDTH_CONFIG_CB_ENABLE_OFFSET, 1)) != eHAL_STATUS_SUCCESS)
        {  return retVal; }
        
        for (range = startIdx; range < numTuneVals; range++)
        {
            if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_RX_IF_TUN, range)) != eHAL_STATUS_SUCCESS) { return retVal; }

            if ((retVal = MeasureRxLoopbackPower(pMac, rxChain, &pwr, sampleBuffer, 1024)) != eHAL_STATUS_SUCCESS) { return retVal; }
            phyLog(pMac, LOG4, "channel:%d\trx_tune_range = %d, pwr = %d:\n", chan, range, pwr);

            if (optPwr < pwr)
            {
                optPwr = pwr;
                optRange = range;
                phyLog(pMac, LOG3, "channel:%d\t\tBetter-> tune_range = %d, pwrSample = %d:\n", chan, range, pwr);
            }
        }
        
        if(startIdx > numTuneVals)
        {
            phyLog(pMac, LOGE, "Error running searchRxCal in QuasarFilterCal\n");
            return eHAL_STATUS_FAILURE;
        }
        
        if ((retVal = asicStopTestWaveform(pMac)) != eHAL_STATUS_SUCCESS) { return retVal; }

        {
            //unsetup_rx
            rfQuasarCalRestore(pMac);
            //Commenting the following register writes, since this is already done in restore function
            //if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_RX_IQ_TEST_1, 0)) != eHAL_STATUS_SUCCESS) { return retVal; }
            //if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_RX_IQ_TEST_0, 0)) != eHAL_STATUS_SUCCESS) { return retVal; }
        }
        pMac->hphy.rf.filterCalRegContext.chanFilterSettings[chan].rxIf = optRange;
    }                        
    if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_RX_IF_TUN, pMac->hphy.rf.filterCalRegContext.chanFilterSettings[chan].rxIf)) != eHAL_STATUS_SUCCESS) { return retVal; }//t.quasar.csr.cfg5.rx_if_tun(int(x))
    //write_dco_lut
    if ((retVal = writeDCOLut(pMac, rxChain, (tANI_S16)(pMac->hphy.rf.filterCalRegContext.currentVal[rxChain].I), (tANI_S16)(pMac->hphy.rf.filterCalRegContext.currentVal[rxChain].Q), calParams->cal_gain_index)) != eHAL_STATUS_SUCCESS) { return retVal; }
        
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

    if (val > (power(2, nbits - 1) - 1))
    {
        result = val - power(2, nbits);
    }

    return result;
}


static eHalStatus setRxGain(tpAniSirGlobal pMac,tANI_U8 rxChainGain1, tANI_U8 rxChainGain2, tANI_U8 rxChainGain3)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;

    if ((retVal = asicAGCReset(pMac)) != eHAL_STATUS_SUCCESS) {  return retVal; }

    // set quasar car gc2 to set gc_select=rx
    if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_GC_SELECT, 0)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    if ((retVal = asicOverrideAGCRxChainGain(pMac, PHY_RX_CHAIN_0, rxChainGain1)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    if ((retVal = asicOverrideAGCRxChainGain(pMac, PHY_RX_CHAIN_1, rxChainGain2)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    if ((retVal = asicOverrideAGCRxChainGain(pMac, PHY_RX_CHAIN_2, rxChainGain3)) != eHAL_STATUS_SUCCESS) { return (retVal); }

    return retVal;
}

#if 0
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
#endif

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
    tANI_U32    value, high, low;
    
    GET_PHY_REG(pMac->hHdd, AGC_TH_CLIP_HIGH_REG, &high);
    GET_PHY_REG(pMac->hHdd, AGC_TH_CLIP_LOW_REG, &low);

    SET_PHY_REG(pMac->hHdd, AGC_TH_CLIP_HIGH_REG, (1024 + 512));
    SET_PHY_REG(pMac->hHdd, AGC_TH_CLIP_LOW_REG, (1024 - 512));
    
    if (rxChain == PHY_RX_CHAIN_0)
    {
        GET_PHY_REG(pMac->hHdd, AGC_SAMPLE_ADC_I0_REG, &value);
        overflow->I = (tANI_U16)(value  & AGC_SAMPLE_ADC_I0_OVFL_MASK) >> AGC_SAMPLE_ADC_I0_OVFL_OFFSET;
        GET_PHY_REG(pMac->hHdd, AGC_SAMPLE_ADC_Q0_REG, &value);
        overflow->Q = (tANI_U16)(value & AGC_SAMPLE_ADC_Q0_OVFL_MASK) >> AGC_SAMPLE_ADC_Q0_OVFL_OFFSET;
    }
    else if (rxChain == PHY_RX_CHAIN_1)
    {
        GET_PHY_REG(pMac->hHdd, AGC_SAMPLE_ADC_I1_REG, &value);
        overflow->I = (tANI_U16)(value & AGC_SAMPLE_ADC_I1_OVFL_MASK) >> AGC_SAMPLE_ADC_I1_OVFL_OFFSET;
        GET_PHY_REG(pMac->hHdd, AGC_SAMPLE_ADC_Q1_REG, &value);
        overflow->Q = (tANI_U16)(value & AGC_SAMPLE_ADC_Q1_OVFL_MASK) >> AGC_SAMPLE_ADC_Q1_OVFL_OFFSET;
    }
    else if (rxChain == PHY_RX_CHAIN_2)
    {
        GET_PHY_REG(pMac->hHdd, AGC_SAMPLE_ADC_I2_REG, &value);
        overflow->I = (tANI_U16)(value & AGC_SAMPLE_ADC_I2_OVFL_MASK) >> AGC_SAMPLE_ADC_I2_OVFL_OFFSET;
        GET_PHY_REG(pMac->hHdd, AGC_SAMPLE_ADC_Q2_REG, &value);
        overflow->Q = (tANI_U16)(value & AGC_SAMPLE_ADC_Q2_OVFL_MASK) >> AGC_SAMPLE_ADC_Q2_OVFL_OFFSET;
    }

    SET_PHY_REG(pMac->hHdd, AGC_TH_CLIP_HIGH_REG, high);
    SET_PHY_REG(pMac->hHdd, AGC_TH_CLIP_LOW_REG, low);
    
    phyLog(pMac, LOG3, "Overflow I = %d, Q = %d\n", overflow->I, overflow->Q);
    return retVal;
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

    asicSetPhyCalMode(pMac, CAL_MODE_ETXLOCAL);
    
    SET_PHY_REG(pMac->hHdd, CAL_OVERRIDE_REG, 0x07);
    SET_PHY_REG(pMac->hHdd, CAL_LENGTH_REG, 1023);
    

    if ((retVal = rdModWrNovaField(pMac, CAL_CHAIN_SEL_REG, CAL_CHAIN_SEL_CHAIN_MASK,
        CAL_CHAIN_SEL_CHAIN_OFFSET, rxChain)) != eHAL_STATUS_SUCCESS)
    {  return retVal; }

    SET_PHY_REG(pMac->hHdd, CAL_MEASURE_REG, CAL_MEASURE_CMD_EMEASURE);

    toCnt = 1000;

    while ((Cnt < toCnt) && (status != 1))
    {
        {
            sirBusyWait(1000);
        }
        retVal = asicGetCalStatus(pMac, (ePhyCalState *)&status);

        if (retVal != eHAL_STATUS_SUCCESS)
        {
            return (retVal);
        }
        Cnt += 1;
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

    {
        asicSetPhyCalMode(pMac, CAL_MODE_ENORMAL);
    }

    for (Cnt = 0; Cnt < 5; Cnt++)
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

#if 0
static eHalStatus calibrateLoopbackDCO(tpAniSirGlobal pMac, tANI_U32 rxChain, tANI_U8 rxGain)
{
    tIQAdc dcoOptVal = {0, 0};
    tIQAdc dco;
    tANI_S16 loop = 0;
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
    tIQAdc dcoMinVal = {1000, 1000};
    tANI_U32 val = 0;

    GET_PHY_REG(pMac->hHdd, TXCTL_DAC_CONTROL_REG, &val);
    val = val | TXCTL_DAC_CONTROL_DAC_OVERRIDE_EN_MASK | TXCTL_DAC_CONTROL_CH1STDBY_OVERRIDE_VAL_MASK | TXCTL_DAC_CONTROL_CH0STDBY_OVERRIDE_VAL_MASK;
    SET_PHY_REG(pMac->hHdd, TXCTL_DAC_CONTROL_REG, val);

    GET_PHY_REG(pMac->hHdd, RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, &val);
    val = val | RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_CAL_MASK;
    SET_PHY_REG(pMac->hHdd, RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, val);

    if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_GC_SELECT, 0)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    
    if ((retVal = readDCOLut(pMac, rxChain, rxGain, &(pMac->hphy.rf.filterCalRegContext.currentVal[rxChain]))) != eHAL_STATUS_SUCCESS) { return (retVal); }
    
    for (loop = 0; loop < 128; loop++)
    {
        retVal = writeDCOLut(pMac, rxChain, loop, loop, rxGain);
        if (retVal != eHAL_STATUS_SUCCESS)
        {
            return (retVal);
        }

        retVal = measureTxLoDCO(pMac, rxChain, &dco);
        if (retVal != eHAL_STATUS_SUCCESS)
        {
            return (retVal);
        }

        if (GET_MAG(dco.I) < GET_MAG(dcoMinVal.I))
        {
            dcoOptVal.I = loop;
            dcoMinVal.I = dco.I;
        }

        if (GET_MAG(dco.Q) < GET_MAG(dcoMinVal.Q))
        {
            dcoOptVal.Q = loop;
            dcoMinVal.Q = dco.Q;
        }
#ifdef ANI_PHY_DEBUG
        phyLog(pMac, LOG4, "DCO , I = %d, Q = %d, loop = %d\n", dco.I, dco.Q, loop);
        phyLog(pMac, LOG4, "DCO Min, I = %d, Q = %d, loop = %d\n", dcoMinVal.I, dcoMinVal.Q, loop);
#endif
    }

    phyLog(pMac, LOG3, "For rxChain: %d, Opt Val, I = %d, Q = %d\n", rxChain, dcoOptVal.I, dcoOptVal.Q);
    retVal = writeDCOLut(pMac, rxChain, dcoOptVal.I, dcoOptVal.Q, rxGain);
    if (retVal != eHAL_STATUS_SUCCESS)
    {
        return (retVal);
    }
    
    SET_PHY_REG(pMac->hHdd, CAL_OVERRIDE_REG, 0);
    return retVal;
}
#endif

#define ABS(x)      ((x < 0) ? -x : x)
typedef enum
{
    INCREMENT_CORRECTION = 1,
    DECREMENT_CORRECTION,
    CORRECTION_REACHED
} tDirection;

static eHalStatus calibrateLoopbackDCO(tpAniSirGlobal pMac, tANI_U32 rxChain, tANI_U8 rxGain)
{
    tIQAdc          currDco, dirDco, dco;
    tANI_U8         loopI = 0, loopQ = 0, loop;
    eHalStatus      retVal = eHAL_STATUS_SUCCESS;
    tANI_U32        value = 0;
    tRxDcoCorrect   offset;
    eRfSubBand      currBand;
    tDirection      IDir, QDir;

    currBand = rfGetAGBand(pMac);

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

    phyLog(pMac, LOG2, "Previous correction: I = %u, Q = %u; CurrError: I = %d, Q = %d", offset.IDcoCorrect, offset.QDcoCorrect,
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
        }

        
        if (ABS(dco.Q) < ABS(currDco.Q))
        {
            QDir = CORRECTION_REACHED;
            offset.QDcoCorrect = (tDcoCorrect)loopQ;
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

    retVal = rfSetDCOffset(pMac, rxChain, rxGain, offset);
    if (retVal != eHAL_STATUS_SUCCESS)
    {
        return (retVal);
    }

    return retVal;
}

static eHalStatus MeasureRxLoopbackPower(tpAniSirGlobal pMac, ePhyRxChains rxChain, tANI_U32 *pwr, tGrabRamSample *sampleBuffer, tANI_U16 numSamples)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
    tANI_U32 i = 0;
    tANI_U16 nSamp, tempSamp = numSamples;
    int iMean = 0, qMean = 0;

    if (numSamples > MAX_NUM_SAMPLES)
    {
        phyLog(pMac, LOGE, "ERROR: Attempted to get more samples than we have allocated space for\n");
        assert(0);
        return (eHAL_STATUS_FAILURE);
    }

    while (numSamples > 0)
    {
        if (numSamples >= MAX_REQUESTED_GRAB_RAM_SAMPLES)
        {
            nSamp = MAX_REQUESTED_GRAB_RAM_SAMPLES;
        }
        else
        {
            nSamp = numSamples;
        }
        
        //the starting sample, i, represents an absolute offset into the sampleBuffer where samples will be stored
        if ((retVal = asicGrabAdcSamples(pMac, i, nSamp, sampleBuffer)) != eHAL_STATUS_SUCCESS) { return (retVal); }
        i += nSamp;
        numSamples -= nSamp;
    }

    *pwr = 0;
#if 1
    //power average samples   
    if (rxChain == PHY_RX_CHAIN_0)
    {
        for (i = 0; i < tempSamp; i++)
        {
            iMean = iMean + sampleBuffer[i].rx0.I;
            qMean = qMean + sampleBuffer[i].rx0.Q;            
        }
        iMean = iMean/tempSamp;
        qMean = qMean/tempSamp;
        for (i = 0; i < tempSamp; i++)
        {
            tANI_U32 newSamp = ((sampleBuffer[i].rx0.I - iMean) * (sampleBuffer[i].rx0.I - iMean)) + ((sampleBuffer[i].rx0.Q - qMean) * (sampleBuffer[i].rx0.Q - qMean));
            
            if (newSamp + *pwr > (0xFFFFFFFFL - BIT_22))
            {
                //need to catch this condition before any potential overflow
                phyLog(pMac, LOGE, "ERROR: Sum of power samples can't exceed 32bits\n");
                return (eHAL_STATUS_FAILURE);
            }
            else
            {
                *pwr += newSamp;
            }            
        }        
    }
    else if (rxChain == PHY_RX_CHAIN_1)
    {
        for (i = 0; i < tempSamp; i++)
        {
            iMean = iMean + sampleBuffer[i].rx1.I;
            qMean = qMean + sampleBuffer[i].rx1.Q;    
        }
        iMean = iMean/tempSamp;
        qMean = qMean/tempSamp;
        for (i = 0; i < tempSamp; i++)
        {
            tANI_U32 newSamp = ((sampleBuffer[i].rx1.I - iMean) * (sampleBuffer[i].rx1.I - iMean)) + ((sampleBuffer[i].rx1.Q - qMean) * (sampleBuffer[i].rx1.Q - qMean));
            
            if (newSamp + *pwr > (0xFFFFFFFFL - BIT_22))
            {
                //need to catch this condition before any potential overflow
                phyLog(pMac, LOGE, "ERROR: Sum of power samples can't exceed 32bits\n");
                return (eHAL_STATUS_FAILURE);
            }
            else
            {
                *pwr += newSamp;
            }            
        }
    }
#endif
#if 0
    
    if (rxChain == PHY_RX_CHAIN_0)
    {
        for (i = 0; i < tempSamp; i++)
        {
            tANI_U32 newSamp = 
                    (sampleBuffer[i].rx0.I * sampleBuffer[i].rx0.I) +
                    (sampleBuffer[i].rx0.Q * sampleBuffer[i].rx0.Q);
            
            if (newSamp + *pwr > (0xFFFFFFFFL - BIT_22))
            {
                //need to catch this condition before any potential overflow
                phyLog(pMac, LOGE, "ERROR: Sum of power samples can't exceed 32bits\n");
                return (eHAL_STATUS_FAILURE);
            }
            else
            {
                *pwr += newSamp;
            }
        }
    }
    else if (rxChain == PHY_RX_CHAIN_1)
    {
        for (i = 0; i < tempSamp; i++)
        {
            tANI_U32 newSamp = 
                (sampleBuffer[i].rx1.I * sampleBuffer[i].rx1.I) +
                (sampleBuffer[i].rx1.Q * sampleBuffer[i].rx1.Q);
            
            if (newSamp + *pwr > (0xFFFFFFFFL - BIT_22))
            {
                //need to catch this condition before any potential overflow
                phyLog(pMac, LOGE, "ERROR: Sum of power samples can't exceed 32bits\n");
                return (eHAL_STATUS_FAILURE);
            }
            else
            {
                *pwr += newSamp;
            }
        }
    }
#endif    
    else
    {
        assert(0);
    }
    
    *pwr = *pwr/tempSamp;
    
    return (retVal);
}

#endif

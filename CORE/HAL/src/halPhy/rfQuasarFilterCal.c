

#ifdef ANI_MANF_DIAG

#include <string.h>
#include "ani_assert.h"
#include "sys_api.h"


//waveform samples from Greg Steele - basically sets up tones to span the width of the channel
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


const tWaveformSample wfmRxCal[] =
{
    { 0, 0      },
    { 0, -161   },
    { 0, 249    },
    { 0, -224   },
    { 0, 97     },
    { 0, 73     },
    { 0, -211   },
    { 0, 252    },
    { 0, -179   },
    { 0, 24     },
    { 0, 141    },
    { 0, -243   },
    { 0, 234    },
    { 0, -119   },
    { 0, -49    },
    { 0, 196    },
    { 0, -254   },
    { 0, 196    },
    { 0, -49    },
    { 0, -119   },
    { 0, 234    },
    { 0, -243   },
    { 0, 141    },
    { 0, 24     },
    { 0, -179   },
    { 0, 252    },
    { 0, -211   },
    { 0, 73     },
    { 0, 97     },
    { 0, -224   },
    { 0, 249    },
    { 0, -161   },
    { 0, 0      },
    { 0, 161    },
    { 0, -249   },
    { 0, 224    },
    { 0, -97    },
    { 0, -73    },
    { 0, 211    },
    { 0, -252   },
    { 0, 179    },
    { 0, -24    },
    { 0, -141   },
    { 0, 243    },
    { 0, -234   },
    { 0, 119    },
    { 0, 49     },
    { 0, -196   },
    { 0, 254    },
    { 0, -196   },
    { 0, 49     },
    { 0, 119    },
    { 0, -234   },
    { 0, 243    },
    { 0, -141   },
    { 0, -24    },
    { 0, 179    },
    { 0, -252   },
    { 0, 211    },
    { 0, -73    },
    { 0, -97    },
    { 0, 224    },
    { 0, -249   },
    { 0, 161    },
    { 0, 0      },
    { 0, -161   },
    { 0, 249    },
    { 0, -224   },
    { 0, 97     },
    { 0, 73     },
    { 0, -211   },
    { 0, 252    },
    { 0, -179   },
    { 0, 24     },
    { 0, 141    },
    { 0, -243   },
    { 0, 234    },
    { 0, -119   },
    { 0, -49    },
    { 0, 196    },
    { 0, -254   },
    { 0, 196    },
    { 0, -49    },
    { 0, -119   },
    { 0, 234    },
    { 0, -243   },
    { 0, 141    },
    { 0, 24     },
    { 0, -179   },
    { 0, 252    },
    { 0, -211   },
    { 0, 73     },
    { 0, 97     },
    { 0, -224   },
    { 0, 249    },
    { 0, -161   },
    { 0, 0      },
    { 0, 161    },
    { 0, -249   },
    { 0, 224    },
    { 0, -97    },
    { 0, -73    },
    { 0, 211    },
    { 0, -252   },
    { 0, 179    },
    { 0, -24    },
    { 0, -141   },
    { 0, 243    },
    { 0, -234   },
    { 0, 119    },
    { 0, 49     },
    { 0, -196   },
    { 0, 254    },
    { 0, -196   },
    { 0, 49     },
    { 0, 119    },
    { 0, -234   },
    { 0, 243    },
    { 0, -141   },
    { 0, -24    },
    { 0, 179    },
    { 0, -252   },
    { 0, 211    },
    { 0, -73    },
    { 0, -97    },
    { 0, 224    },
    { 0, -249   },
    { 0, 161    },
    { 0, 0      },
    { 0, -161   },
    { 0, 249    },
    { 0, -224   },
    { 0, 97     },
    { 0, 73     },
    { 0, -211   },
    { 0, 252    },
    { 0, -179   },
    { 0, 24     },
    { 0, 141    },
    { 0, -243   },
    { 0, 234    },
    { 0, -119   },
    { 0, -49    },
    { 0, 196    },
    { 0, -254   },
    { 0, 196    },
    { 0, -49    },
    { 0, -119   },
    { 0, 234    },
    { 0, -243   },
    { 0, 141    },
    { 0, 24     },
    { 0, -179   },
    { 0, 252    },
    { 0, -211   },
    { 0, 73     },
    { 0, 97     },
    { 0, -224   },
    { 0, 249    },
    { 0, -161   },
    { 0, 0      },
    { 0, 161    },
    { 0, -249   },
    { 0, 224    },
    { 0, -97    },
    { 0, -73    },
    { 0, 211    },
    { 0, -252   },
    { 0, 179    },
    { 0, -24    },
    { 0, -141   },
    { 0, 243    },
    { 0, -234   },
    { 0, 119    },
    { 0, 49     },
    { 0, -196   },
    { 0, 254    },
    { 0, -196   },
    { 0, 49     },
    { 0, 119    },
    { 0, -234   },
    { 0, 243    },
    { 0, -141   },
    { 0, -24    },
    { 0, 179    },
    { 0, -252   },
    { 0, 211    },
    { 0, -73    },
    { 0, -97    },
    { 0, 224    },
    { 0, -249   },
    { 0, 161    },
    { 0, 0      },
    { 0, -161   },
    { 0, 249    },
    { 0, -224   },
    { 0, 97     },
    { 0, 73     },
    { 0, -211   },
    { 0, 252    },
    { 0, -179   },
    { 0, 24     },
    { 0, 141    },
    { 0, -243   },
    { 0, 234    },
    { 0, -119   },
    { 0, -49    },
    { 0, 196    },
    { 0, -254   },
    { 0, 196    },
    { 0, -49    },
    { 0, -119   },
    { 0, 234    },
    { 0, -243   },
    { 0, 141    },
    { 0, 24     },
    { 0, -179   },
    { 0, 252    },
    { 0, -211   },
    { 0, 73     },
    { 0, 97     },
    { 0, -224   },
    { 0, 249    },
    { 0, -161   },
    { 0, 0      },
    { 0, 161    },
    { 0, -249   },
    { 0, 224    },
    { 0, -97    },
    { 0, -73    },
    { 0, 211    },
    { 0, -252   },
    { 0, 179    },
    { 0, -24    },
    { 0, -141   },
    { 0, 243    },
    { 0, -234   },
    { 0, 119    },
    { 0, 49     },
    { 0, -196   },
    { 0, 254    },
    { 0, -196   },
    { 0, 49     },
    { 0, 119    },
    { 0, -234   },
    { 0, 243    },
    { 0, -141   },
    { 0, -24    },
    { 0, 179    },
    { 0, -252   },
    { 0, 211    },
    { 0, -73    },
    { 0, -97    },
    { 0, 224    },
    { 0, -249   },
    { 0, 161    }
};

#define NUM_24_TX_IF_TUNE_VALS    3
#define NUM_24_TX_RF_TUNE_VALS    3

#define NUM_5_TX_IF_TUNE_VALS    4
#define NUM_5_TX_RF_TUNE_VALS    8

#define NUM_24_RX_IF_TUNE_VALS   8

#define NUM_5_RX_IF_TUNE_VALS   16

#define NUM_RX_IF_SAMPLES   2048
static eHalStatus MeasureRx1LoopbackPower(tpAniSirGlobal pMac, tANI_U32 *pwr, tGrabRamSample *sampleBuffer);
static eHalStatus calibrateLoopbackDCO(tpAniSirGlobal pMac, tANI_U32 rxChain, tANI_U8 rxGain);

// for debug extern eHalStatus log_grab_ram(tpAniSirGlobal pMac, tANI_U32 startSample, tANI_U32 numSamples);


extern const tANI_U8 agcQuasarGains[NUM_QUASAR_RX_GAIN_STEPS];


//#define NUM_RF_CHANNELS 1

eHalStatus rfQuasarFilterCal(tpAniSirGlobal pMac, tQuasarFilterSettings *pChanFilterSettings)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
    eRfChannels chan;
    tGrabRamSample *sampleBuffer;
    eRfChannels curChan = rfGetCurChannel(pMac);
    ePhyChainSelect chains = halPhyGetActiveChainSelect(pMac);
    ePhyRxDisabledPktTypes disabledTypes;
    tTxGain txGain = { 8, 0 };
    tTxGain otherGain = { 0, 0 };
    tQuasarFilterSettings chanFilterSettings;



    assert(pChanFilterSettings != NULL);


    memset(&chanFilterSettings, 0, sizeof(tQuasarFilterSettings));
#if defined(ANI_BUS_TYPE_USB)
    if ((retVal = halPhySetChainSelect(pMac, PHY_CHAIN_SEL_R0R1_T0T1_ON)) != eHAL_STATUS_SUCCESS) { return (retVal); }
#else
    if ((retVal = halPhySetChainSelect(pMac, PHY_CHAIN_SEL_R0R1R2_T0T1_ON)) != eHAL_STATUS_SUCCESS) { return (retVal); }
#endif
    if ((retVal = asicGetDisabledRxPacketTypes(pMac, &disabledTypes)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    if ((retVal = asicSetDisabledRxPacketTypes(pMac, PHY_RX_DISABLE_ALL_TYPES)) != eHAL_STATUS_SUCCESS) { return (retVal); }


    if ((retVal = rdModWrNovaField(pMac, RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG,
                    RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_CAL_MASK,
                    RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_CAL_OFFSET, 1)) != eHAL_STATUS_SUCCESS)
    {
        return (retVal);
    }

    if ((retVal = rdModWrNovaField(pMac, RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG,
                    RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_RXFIR_MASK,
                    RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_RXFIR_OFFSET, 1)) != eHAL_STATUS_SUCCESS)
    {
        return (retVal);
    }


    //txIf & txRf cal
    {
        //search_tx_cal
        {
            //setup_tx - use txChain0, rxChain1 for loopback
            if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_TX_CAL_EN_0, 1)) != eHAL_STATUS_SUCCESS) { return (retVal); }
            if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_TX_CAL_EN_1, 0)) != eHAL_STATUS_SUCCESS) { return (retVal); }
            if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_DET_GAIN, 1)) != eHAL_STATUS_SUCCESS) { return (retVal); }

            SET_PHY_REG(pMac->hHdd, AGC_RX_OVERRIDE_REG, 0x702);
            //asicOverrideAGCRxEnable(pMac, AGC_RX_1, AGC_RX_CALIBRATING);
            //if ((retVal = asicOverrideAGCRxChainGain(pMac, PHY_RX_CHAIN_0, txGain.coarsePwr)) != eHAL_STATUS_SUCCESS) { return (retVal); }     //precaution in case this accidentally gets sent as a tx gain
            if ((retVal = asicAGCReset(pMac)) != eHAL_STATUS_SUCCESS) { return (retVal); }

            if ((retVal = asicSetTxDACs(pMac, PHY_TX_CHAIN_1, eANI_BOOLEAN_TRUE, eANI_BOOLEAN_TRUE)) != eHAL_STATUS_SUCCESS) { return (retVal); }
            if ((retVal = asicSetupTestWaveform(pMac, wfmTxCal, sizeof(wfmTxCal) / sizeof(tWaveformSample), eANI_BOOLEAN_TRUE)) != eHAL_STATUS_SUCCESS) { return (retVal); }
            if ((retVal = asicStartTestWaveform(pMac, WAVE_CONTINUOUS, 0, ((sizeof(wfmTxCal) / sizeof(tWaveformSample)) - 1))) != eHAL_STATUS_SUCCESS) { return (retVal); }
            if ((retVal = asicTPCPowerOverride(pMac, txGain, otherGain)) != eHAL_STATUS_SUCCESS) { return (retVal); }
            if ((retVal = asicZeroFineDCOCorrection(pMac, PHY_ALL_RX_CHAINS)) != eHAL_STATUS_SUCCESS) { return (retVal); }
            if ((retVal = asicSetPhyCalLength(pMac, 0x3FF)) != eHAL_STATUS_SUCCESS) { return (retVal); }
            if ((retVal = asicSetPhyCalMode(pMac, PHY_CAL_MODE_INITDCCAL)) != eHAL_STATUS_SUCCESS) { return (retVal); }
        }

        {
            tIQAdc pwrSample;
            tANI_U32 maxPwrIfIndex = 0, maxPwrRfIndex = 0;


            for (chan = 0; chan < NUM_RF_CHANNELS; chan++)
            {
                tANI_U8 ifIndex;
                tANI_U8 rfIndex;
                tIQAdc maxPwr = { 0x8000, 0 };  //initialize to very lowest negative number

                //channel bonded channels can use the low 20MHz measurement
                //this works because we cal the 20MHz channels first
                switch (chan)
                {
                    // donot run the calibration. use the txIf, txRf and rxIf of RF_CHAN_3 for these two channels
                    case RF_CHAN_1:
                    case RF_CHAN_2:
                        continue;

                    case RF_CHAN_BOND_3:
                    case RF_CHAN_BOND_4:
                    case RF_CHAN_BOND_5:
                    case RF_CHAN_BOND_6:
                    case RF_CHAN_BOND_7:
                    case RF_CHAN_BOND_8:
                    case RF_CHAN_BOND_9:
                    case RF_CHAN_BOND_10:
                    case RF_CHAN_BOND_11:
                        chanFilterSettings[chan].txIf = chanFilterSettings[chan - (RF_CHAN_BOND_3 - RF_CHAN_3)].txIf;
                        chanFilterSettings[chan].txRf = chanFilterSettings[chan - (RF_CHAN_BOND_3 - RF_CHAN_3)].txRf;
                        continue;
                    
                    case RF_CHAN_BOND_242:
                    case RF_CHAN_BOND_246:
                    case RF_CHAN_BOND_250:
                    case RF_CHAN_BOND_210:
                    case RF_CHAN_BOND_214:
                        chanFilterSettings[chan].txIf = chanFilterSettings[chan - (RF_CHAN_BOND_242 - RF_CHAN_240)].txIf;
                        chanFilterSettings[chan].txRf = chanFilterSettings[chan - (RF_CHAN_BOND_242 - RF_CHAN_240)].txRf;
                        continue;
                    
                    case RF_CHAN_BOND_38:
                    case RF_CHAN_BOND_42:
                    case RF_CHAN_BOND_46:
                    case RF_CHAN_BOND_50:
                    case RF_CHAN_BOND_54:
                    case RF_CHAN_BOND_58:
                    case RF_CHAN_BOND_62:
                        chanFilterSettings[chan].txIf = chanFilterSettings[chan - (RF_CHAN_BOND_38 - RF_CHAN_36)].txIf;
                        chanFilterSettings[chan].txRf = chanFilterSettings[chan - (RF_CHAN_BOND_38 - RF_CHAN_36)].txRf;
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
                        chanFilterSettings[chan].txIf = chanFilterSettings[chan - (RF_CHAN_BOND_102 - RF_CHAN_100)].txIf;
                        chanFilterSettings[chan].txRf = chanFilterSettings[chan - (RF_CHAN_BOND_102 - RF_CHAN_100)].txRf;
                        continue;
                    
                    case RF_CHAN_BOND_151:
                    case RF_CHAN_BOND_155:
                    case RF_CHAN_BOND_159:
                    case RF_CHAN_BOND_163:
                        chanFilterSettings[chan].txIf = chanFilterSettings[chan - (RF_CHAN_BOND_151 - RF_CHAN_149)].txIf;
                        chanFilterSettings[chan].txRf = chanFilterSettings[chan - (RF_CHAN_BOND_151 - RF_CHAN_149)].txRf;
                        continue;
                    
                    default:
                        break;
                }


                if ((retVal = rfSetCurChannel(pMac, chan)) != eHAL_STATUS_SUCCESS) { return (retVal); }
                if ((retVal = rfSetChanBondMode(pMac, eANI_BOOLEAN_TRUE)) != eHAL_STATUS_SUCCESS) { return (retVal); }

                phyLog(pMac, LOG1, "Tx Filter cal for Channel index %d:\n", chan);

                if (((chan >= MIN_2_4GHZ_CHANNEL) && (chan <= MAX_2_4GHZ_CHANNEL)) ||
                    ((chan >= MIN_CB_2_4GHZ_CHANNEL) && (chan <= MAX_CB_2_4GHZ_CHANNEL))
                   )
                {
                    //2.4GHz channel
                    tANI_U8 ifTuneVals[NUM_24_TX_IF_TUNE_VALS] = { 0, 1, 3 };
                    tANI_U8 rfTuneVals[NUM_24_TX_RF_TUNE_VALS] = { 0, 1, 3 };

                    for (ifIndex = 0; ifIndex < NUM_24_TX_IF_TUNE_VALS; ifIndex++)
                    {
                        for (rfIndex = 0; rfIndex < NUM_24_TX_RF_TUNE_VALS; rfIndex++)
                        {
                            if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_TX_IF_TUN, ifTuneVals[ifIndex])) != eHAL_STATUS_SUCCESS) { return (retVal); }
                            if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_TX_RF_TUN, rfTuneVals[rfIndex])) != eHAL_STATUS_SUCCESS) { return (retVal); }

                            if ((retVal = asicGetCalADCSamples(pMac, PHY_RX_CHAIN_1, &pwrSample)) != eHAL_STATUS_SUCCESS) { return (retVal); }

                            phyLog(pMac, LOG4, "\tifIndex = %d, rfIndex = %d  pwrSample = %d:\n", ifIndex, rfIndex, (tANI_S16)pwrSample.I);
/*
    #ifdef ANI_PHY_DEBUG
                                log_grab_ram(pMac, 0, 1024); //temporary debug
    #endif
*/

                            if (pwrSample.I > maxPwr.I)
                            {
                                maxPwrIfIndex = ifIndex;
                                maxPwrRfIndex = rfIndex;
                                maxPwr.I = pwrSample.I;
                                phyLog(pMac, LOG3, "\t\tBetter-> ifIndex = %d, rfIndex = %d  pwrSample = %d:\n", ifIndex, rfIndex, (tANI_S16)pwrSample.I);
                            }
                        }
                    }
                    //loop exits with best settings captured
                    chanFilterSettings[chan].txIf = ifTuneVals[maxPwrIfIndex];
                    chanFilterSettings[chan].txRf = rfTuneVals[maxPwrRfIndex];

                    if(chan == RF_CHAN_3)
                    {
                        chanFilterSettings[RF_CHAN_1].txIf = chanFilterSettings[RF_CHAN_2].txIf = ifTuneVals[maxPwrIfIndex];
                        chanFilterSettings[RF_CHAN_1].txRf = chanFilterSettings[RF_CHAN_2].txRf = rfTuneVals[maxPwrRfIndex];
                    }
                    phyLog(pMac, LOG2, "\t\t\tRecorded-> ifIndex = %d, rfIndex = %d\n", maxPwrIfIndex, maxPwrRfIndex);
                }
                else
                {
                    //2.4GHz channel
                    tANI_U8 ifTuneVals[NUM_5_TX_IF_TUNE_VALS] = { 0, 1, 3, 7 };
                    tANI_U8 rfTuneVals[NUM_5_TX_RF_TUNE_VALS] = { 0, 1, 2, 3, 4, 5, 6, 7 };

                    for (ifIndex = 0; ifIndex < NUM_5_TX_IF_TUNE_VALS; ifIndex++)
                    {
                        for (rfIndex = 0; rfIndex < NUM_5_TX_RF_TUNE_VALS; rfIndex++)
                        {
                            if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_TX_IF_TUN, ifTuneVals[ifIndex])) != eHAL_STATUS_SUCCESS) { return (retVal); }
                            if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_TX_RF_TUN, rfTuneVals[rfIndex])) != eHAL_STATUS_SUCCESS) { return (retVal); }

                            if ((retVal = asicGetCalADCSamples(pMac, PHY_RX_CHAIN_1, &pwrSample)) != eHAL_STATUS_SUCCESS) { return (retVal); }

                            phyLog(pMac, LOG4, "\tifIndex = %d, rfIndex = %d  pwrSample = %d:\n", ifIndex, rfIndex, (tANI_S16)pwrSample.I);

                            if (pwrSample.I > maxPwr.I)
                            {
                                maxPwrIfIndex = ifIndex;
                                maxPwrRfIndex = rfIndex;
                                maxPwr.I = pwrSample.I;
                                phyLog(pMac, LOG3, "\t\tBetter-> ifIndex = %d, rfIndex = %d  pwrSample = %d:\n", ifIndex, rfIndex, (tANI_S16)pwrSample.I);
                            }
                        }
                    }
                    //loop exits with best settings captured
                    chanFilterSettings[chan].txIf = ifTuneVals[maxPwrIfIndex];
                    chanFilterSettings[chan].txRf = rfTuneVals[maxPwrRfIndex];

                    if(chan == RF_CHAN_3)
                    {
                        chanFilterSettings[RF_CHAN_1].txIf = chanFilterSettings[RF_CHAN_2].txIf = ifTuneVals[maxPwrIfIndex];
                        chanFilterSettings[RF_CHAN_1].txRf = chanFilterSettings[RF_CHAN_2].txRf = rfTuneVals[maxPwrRfIndex];
                    }
                    phyLog(pMac, LOG2, "\t\t\tRecorded-> ifIndex = %d, rfIndex = %d\n", maxPwrIfIndex, maxPwrRfIndex);
                }

            }
        }

        {
            //unsetup_tx
            if ((retVal = asicStopTestWaveform(pMac)) != eHAL_STATUS_SUCCESS) { return (retVal); }
            if ((retVal = asicZeroFineDCOCorrection(pMac, PHY_NO_RX_CHAINS)) != eHAL_STATUS_SUCCESS) { return (retVal); }
            if ((retVal = asicSetPhyCalMode(pMac, PHY_CAL_MODE_NORMAL)) != eHAL_STATUS_SUCCESS) { return (retVal); }
            if ((retVal = asicTPCAutomatic(pMac)) != eHAL_STATUS_SUCCESS) { return (retVal); }

            if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_TX_CAL_EN_0, 0)) != eHAL_STATUS_SUCCESS) { return (retVal); }
            if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_TX_CAL_EN_1, 0)) != eHAL_STATUS_SUCCESS) { return (retVal); }
            if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_DET_GAIN, 0)) != eHAL_STATUS_SUCCESS) { return (retVal); }
            if ((retVal = asicCeaseOverrideAGCRxChainGain(pMac, PHY_RX_CHAIN_1)) != eHAL_STATUS_SUCCESS) { return (retVal); }
        }
    }



    if ((retVal = palAllocateMemory(pMac->hHdd, (void **)&sampleBuffer, (NUM_RX_IF_SAMPLES * sizeof(tGrabRamSample)))) == eHAL_STATUS_SUCCESS)
    {
        const tANI_U8 rxGain = 32;  //32 from Greg's script
        tRxGain curVal;
        tRxGain tempGainVal = { agcQuasarGains[MAX_QUASAR_RX_GAIN_STEP], T_SW_EN_MSK };
        tANI_U32 lnaEnables;
        tANI_U32 bandwidthConfig;
        tRxDcoCorrect dcoSaved;

        if ((retVal = rfGetDCOffset(pMac, PHY_RX_CHAIN_1, rxGain, &dcoSaved)) != eHAL_STATUS_SUCCESS) { return (retVal); }


        //search_rx_cal - use txChain0, rxChain1 for loopback

        {
            //setup_rx
            if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_RX_IQ_TEST_0, 0)) != eHAL_STATUS_SUCCESS) { return (retVal); }
            if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_RX_IQ_TEST_1, 1)) != eHAL_STATUS_SUCCESS) { return (retVal); }
            if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_RX_LB_GAIN, 1)) != eHAL_STATUS_SUCCESS) { return (retVal); }
            if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_TX_LB_GAIN, 1)) != eHAL_STATUS_SUCCESS) { return (retVal); }

            GET_PHY_REG(pMac->hHdd, AGC_BANDWIDTH_CONFIG_REG, &bandwidthConfig);

            {
                //force gain value so that it doesn't matter if this is run before or after the receive gains are calibrated
                //temporarily flip TR switch to T for this chain & gain indes - provides better front end isolation
                if ((retVal = asicGetAgcGainIndex(pMac, PHY_RX_CHAIN_1, rxGain, &curVal)) != eHAL_STATUS_SUCCESS) { return (retVal); }
                if ((retVal = asicSetAGCGainLut(pMac, PHY_RX_CHAIN_1, rxGain, rxGain, (const tRxGain *)&tempGainVal)) != eHAL_STATUS_SUCCESS) { return (retVal); }

                //save off register for restoration after
                GET_PHY_REG(pMac->hHdd, GPIO_RFIF_EN_REG, &lnaEnables);

                {
                    tANI_U32 dacControl;
                    tANI_U32 firTxMode;
                    tANI_U32 rxOverrideReg;
                    tANI_U32 agcReset;

                    GET_PHY_REG(pMac->hHdd, AGC_RESET_REG, &agcReset);
                    assert(agcReset == 0);

                    GET_PHY_REG(pMac->hHdd, TXCTL_FIR_MODE_REG, &firTxMode);
                    GET_PHY_REG(pMac->hHdd, TXCTL_DAC_CONTROL_REG, &dacControl);
                    GET_PHY_REG(pMac->hHdd, AGC_RX_OVERRIDE_REG, &rxOverrideReg);

                    //in order to force the rx gain, force tx chains off, and rx chains on
                    if ((retVal = asicSetTxDACs(pMac, PHY_ALL_TX_CHAINS, eANI_BOOLEAN_TRUE, eANI_BOOLEAN_FALSE)) != eHAL_STATUS_SUCCESS) { return (retVal); }
                    if ((retVal = asicOverrideAGCRxEnable(pMac, AGC_ALL_RX, AGC_RX_CALIBRATING)) != eHAL_STATUS_SUCCESS) { return (retVal); }

                    //now that the gain at the rxGain index has been overwritten, override the AGC to use this gain
                    if ((retVal = asicOverrideAGCRxChainGain(pMac, PHY_RX_CHAIN_1, rxGain)) != eHAL_STATUS_SUCCESS) { return (retVal); }

                    SET_PHY_REG(pMac->hHdd, TXCTL_FIR_MODE_REG, firTxMode);
                    SET_PHY_REG(pMac->hHdd, TXCTL_DAC_CONTROL_REG, dacControl);
                    SET_PHY_REG(pMac->hHdd, AGC_RX_OVERRIDE_REG, rxOverrideReg);
                }
            }

            SET_PHY_REG(pMac->hHdd, AGC_RX_OVERRIDE_REG, 0x702);

            {
                //from observation, there can be a large DC component associated with the combination of the Tx carrier, the loopback path, and the rx path.
                //It seems the loopback path is quite large, so we must cal out the DC when in this mode.
                //Here we want the Tx Lo's on, the DACs turned off, and then calibrate PHY_RX_CHAIN_1 for the gain index we are using, without any waveform playing
                if ((retVal = asicSetTxDACs(pMac, PHY_ALL_TX_CHAINS, eANI_BOOLEAN_TRUE, eANI_BOOLEAN_FALSE)) != eHAL_STATUS_SUCCESS) { return (retVal); }
                if ((retVal = calibrateLoopbackDCO(pMac, PHY_RX_CHAIN_1, rxGain)) != eHAL_STATUS_SUCCESS) { return (retVal); }
                //should leave this with the DCO offset values set
            }

            if ((retVal = asicSetTxDACs(pMac, PHY_TX_CHAIN_1, eANI_BOOLEAN_TRUE, eANI_BOOLEAN_TRUE)) != eHAL_STATUS_SUCCESS) { return (retVal); }
            if ((retVal = asicSetupTestWaveform(pMac, wfmRxCal, sizeof(wfmRxCal) / sizeof(tWaveformSample), eANI_BOOLEAN_TRUE)) != eHAL_STATUS_SUCCESS) { return (retVal); }
            if ((retVal = asicStartTestWaveform(pMac, WAVE_CONTINUOUS, 0, ((sizeof(wfmRxCal) / sizeof(tWaveformSample)) - 1))) != eHAL_STATUS_SUCCESS) { return (retVal); }
            if ((retVal = asicTPCPowerOverride(pMac, txGain, otherGain)) != eHAL_STATUS_SUCCESS) { return (retVal); }
        }


        {
            tANI_U8 ifTuneVals[NUM_5_RX_IF_TUNE_VALS] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };   //these values contain both 2.4 & 5GHz subsets
            tANI_U32 maxPwrIfIndex = 0;

            for (chan = 0; chan < NUM_RF_CHANNELS; chan++)
            {
                tANI_U8 ifIndex;
                tANI_U32 pwr = 0;
                tANI_U32 maxPwr = 0;
                tANI_U32 nVals;

                //channel bonded channels can use the low 20MHz measurement
                //this works because we cal the 20MHz channels first
                switch (chan)
                {
                    // donot run the calibration. use the txIf, txRf and rxIf of RF_CHAN_3 for these two channels
                    case RF_CHAN_1:
                    case RF_CHAN_2:
                        continue;

                    case RF_CHAN_BOND_3:
                    case RF_CHAN_BOND_4:
                    case RF_CHAN_BOND_5:
                    case RF_CHAN_BOND_6:
                    case RF_CHAN_BOND_7:
                    case RF_CHAN_BOND_8:
                    case RF_CHAN_BOND_9:
                    case RF_CHAN_BOND_10:
                    case RF_CHAN_BOND_11:
                        chanFilterSettings[chan].rxIf = chanFilterSettings[chan - (RF_CHAN_BOND_3 - RF_CHAN_3)].rxIf;
                        continue;
                    
                    case RF_CHAN_BOND_242:
                    case RF_CHAN_BOND_246:
                    case RF_CHAN_BOND_250:
                    case RF_CHAN_BOND_210:
                    case RF_CHAN_BOND_214:
                        chanFilterSettings[chan].rxIf = chanFilterSettings[chan - (RF_CHAN_BOND_242 - RF_CHAN_240)].rxIf;
                        continue;
                    
                    case RF_CHAN_BOND_38:
                    case RF_CHAN_BOND_42:
                    case RF_CHAN_BOND_46:
                    case RF_CHAN_BOND_50:
                    case RF_CHAN_BOND_54:
                    case RF_CHAN_BOND_58:
                    case RF_CHAN_BOND_62:
                        chanFilterSettings[chan].rxIf = chanFilterSettings[chan - (RF_CHAN_BOND_38 - RF_CHAN_36)].rxIf;
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
                        chanFilterSettings[chan].rxIf = chanFilterSettings[chan - (RF_CHAN_BOND_102 - RF_CHAN_100)].rxIf;
                        continue;
                    
                    case RF_CHAN_BOND_151:
                    case RF_CHAN_BOND_155:
                    case RF_CHAN_BOND_159:
                    case RF_CHAN_BOND_163:
                        chanFilterSettings[chan].rxIf = chanFilterSettings[chan - (RF_CHAN_BOND_151 - RF_CHAN_149)].rxIf;
                        continue;
                    
                    default:
                        break;
                }
                
                
                if ((retVal = rfSetCurChannel(pMac, chan)) != eHAL_STATUS_SUCCESS) { return (retVal); }
                if ((retVal = rfSetChanBondMode(pMac, eANI_BOOLEAN_TRUE)) != eHAL_STATUS_SUCCESS) { return (retVal); }
                SET_PHY_REG(pMac->hHdd, AGC_BANDWIDTH_CONFIG_REG, AGC_DOUBLE_CHANNEL_LOW_PRIMARY);

                //apply tx filter coefficients for accurate rx filter tuning
                if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_TX_IF_TUN, chanFilterSettings[chan].txIf)) != eHAL_STATUS_SUCCESS) { return (retVal); }
                if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_TX_RF_TUN, chanFilterSettings[chan].txRf)) != eHAL_STATUS_SUCCESS) { return (retVal); }

                phyLog(pMac, LOG1, "Rx Filter cal for Channel index %d:\n", chan);

                if (((chan >= MIN_2_4GHZ_CHANNEL) && (chan <= MAX_2_4GHZ_CHANNEL)) ||
                    ((chan >= MIN_CB_2_4GHZ_CHANNEL) && (chan <= MAX_CB_2_4GHZ_CHANNEL))
                   )
                {
                    nVals = NUM_24_RX_IF_TUNE_VALS;

                    //turn off lna enables, but leave PA enables on - both bands since we are looping all channels
                    SET_PHY_REG(pMac->hHdd, GPIO_RFIF_EN_REG, GPIO_RFIF_EN_PA_ENABLE_24G_0_MASK);
                }
                else
                {
                    nVals = NUM_5_RX_IF_TUNE_VALS;

                    //turn off lna enables, but leave PA enables on - both bands since we are looping all channels
                    SET_PHY_REG(pMac->hHdd, GPIO_RFIF_EN_REG, GPIO_RFIF_EN_PA_ENABLE_5G_0_MASK);
                }

                for (ifIndex = 0; ifIndex < nVals; ifIndex++)
                {
                    if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_RX_IF_TUN, ifTuneVals[ifIndex])) != eHAL_STATUS_SUCCESS) { return (retVal); }

                    if ((retVal = MeasureRx1LoopbackPower(pMac, &pwr, sampleBuffer)) != eHAL_STATUS_SUCCESS) { return (retVal); }
                    phyLog(pMac, LOG4, "\tifIndex = %d  pwr = %d:\n", ifIndex, pwr);

                    if (pwr > maxPwr)
                    {
                        maxPwrIfIndex = ifIndex;
                        maxPwr = pwr;
                        phyLog(pMac, LOG3, "\t\tBetter-> ifIndex = %d  pwr = %d:\n", ifIndex, pwr);
                    }
                }

                //loop exits with best setting captured
                chanFilterSettings[chan].rxIf = ifTuneVals[maxPwrIfIndex];
                if(chan == RF_CHAN_3)
                {
                    chanFilterSettings[RF_CHAN_1].rxIf = chanFilterSettings[RF_CHAN_2].rxIf = ifTuneVals[maxPwrIfIndex];
                }
                phyLog(pMac, LOG2, "\t\tRecorded-> ifIndex = %d\n", maxPwrIfIndex);
            }
        }

        {
            //now restore prior chain gain and lnas
            if ((retVal = asicSetAGCGainLut(pMac, PHY_RX_CHAIN_1, rxGain, rxGain, (const tRxGain *)&curVal)) != eHAL_STATUS_SUCCESS) { return (retVal); }
            SET_PHY_REG(pMac->hHdd, GPIO_RFIF_EN_REG, lnaEnables);

        }

        {
            //unsetup_rx
            if ((retVal = asicStopTestWaveform(pMac)) != eHAL_STATUS_SUCCESS) { return (retVal); }
            if ((retVal = asicTPCAutomatic(pMac)) != eHAL_STATUS_SUCCESS) { return (retVal); }

            if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_RX_IQ_TEST_1, 0)) != eHAL_STATUS_SUCCESS) { return (retVal); }
            if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_RX_LB_GAIN, 0)) != eHAL_STATUS_SUCCESS) { return (retVal); }
            if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_TX_LB_GAIN, 0)) != eHAL_STATUS_SUCCESS) { return (retVal); }

            if ((retVal = asicCeaseOverrideAGCRxChainGain(pMac, PHY_RX_CHAIN_1)) != eHAL_STATUS_SUCCESS) { return (retVal); }
            SET_PHY_REG(pMac->hHdd, AGC_BANDWIDTH_CONFIG_REG, bandwidthConfig);
        }

        if ((retVal = rfSetDCOffset(pMac, PHY_RX_CHAIN_1, rxGain, dcoSaved)) != eHAL_STATUS_SUCCESS) { return (retVal); }

        if ((retVal = palFreeMemory(pMac->hHdd, sampleBuffer)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    }
    else
    {
        phyLog(pMac, LOGE, "ERROR: Unable to allocate memory for rxIf filter tuning\n");
        return (eHAL_STATUS_FAILURE);
    }

    if ((retVal = asicSetPhyCalLength(pMac, 0xF)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    if ((retVal = asicSetDisabledRxPacketTypes(pMac, disabledTypes)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    if ((retVal = rfSetCurChannel(pMac, curChan)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    if ((retVal = halPhySetChainSelect(pMac, chains)) != eHAL_STATUS_SUCCESS) { return (retVal); }

    if ((retVal = rdModWrNovaField(pMac, RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG,
                    RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_CAL_MASK,
                    RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_CAL_OFFSET, 0)) != eHAL_STATUS_SUCCESS)
    {
        return (retVal);
    }

    if ((retVal = rdModWrNovaField(pMac, RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG,
                    RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_RXFIR_MASK,
                    RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_RXFIR_OFFSET, 0)) != eHAL_STATUS_SUCCESS)
    {
        return (retVal);
    }

    memcpy(pChanFilterSettings, &chanFilterSettings, sizeof(tQuasarFilterSettings));
    return (retVal);
}



static eHalStatus MeasureRx1LoopbackPower(tpAniSirGlobal pMac, tANI_U32 *pwr, tGrabRamSample *sampleBuffer)
{
    eHalStatus retVal;
    tANI_U32 i;


    if ((retVal = asicGrabAdcSamples(pMac, 0, MAX_REQUESTED_GRAB_RAM_SAMPLES, sampleBuffer)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    if ((retVal = asicGrabAdcSamples(pMac, MAX_REQUESTED_GRAB_RAM_SAMPLES, MAX_REQUESTED_GRAB_RAM_SAMPLES, sampleBuffer)) != eHAL_STATUS_SUCCESS) { return (retVal); }

    *pwr = 0;

    //power average samples
    for (i = 0; i < NUM_RX_IF_SAMPLES; i++)
    {
        *pwr += (sampleBuffer[i].rx1.I * sampleBuffer[i].rx1.I) +
                (sampleBuffer[i].rx1.Q * sampleBuffer[i].rx1.Q);
    }

    return (retVal);
}





//copied and modified from Tx Carrier suppression - brute force method that needs optimization
static eHalStatus calibrateLoopbackDCO(tpAniSirGlobal pMac, tANI_U32 rxChain, tANI_U8 rxGain)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
    tRxDcoCorrect dcoCorrect = {0, 0, 1};
    tRxDcoCorrect dcoOptVal = {0, 0, 1};
    tIQAdc dcoMinVal = { 10000, 10000 };
    tIQAdc dco;
    tANI_U32 loop = 0;

    if ((retVal = asicSetPhyCalMode(pMac, PHY_CAL_MODE_INITDCCAL)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    if ((retVal = asicSetPhyCalLength(pMac, 1023)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    if ((retVal = asicZeroFineDCOCorrection(pMac, PHY_ALL_RX_CHAINS)) != eHAL_STATUS_SUCCESS) { return (retVal); }

    for (loop = 0; loop < 128; loop++)
    {
        //retVal = writeDCOLut(pMac, rxChain, loop, loop, rxGain);
        dcoCorrect.IDcoCorrect = (tANI_S8)loop;
        dcoCorrect.QDcoCorrect = (tANI_S8)loop;
        if ((retVal = rfSetDCOffset(pMac, rxChain, rxGain, dcoCorrect)) != eHAL_STATUS_SUCCESS) { return (retVal); }

        //retVal = measureTxLoDCO(pMac, rxChain, &dco);
        if ((retVal = asicGetCalADCSamples(pMac, rxChain, &dco)) != eHAL_STATUS_SUCCESS) { return (retVal); }

        if (GET_MAG(dco.I) < GET_MAG(dcoMinVal.I))
        {
            dcoOptVal.IDcoCorrect = (tANI_S8)loop;
            dcoMinVal.I = dco.I;
        }

        if (GET_MAG(dco.Q) < GET_MAG(dcoMinVal.Q))
        {
            dcoOptVal.QDcoCorrect = (tANI_S8)loop;
            dcoMinVal.Q = dco.Q;
        }

#ifdef ANI_PHY_DEBUG
        phyLog(pMac, LOG4, "DCO , I = %d, Q = %d, loop = %d\n", dco.I, dco.Q, loop);
        phyLog(pMac, LOG4, "DCO Min, I = %d, Q = %d, loop = %d\n", dcoMinVal.I, dcoMinVal.Q, loop);
#endif
    }

    phyLog(pMac, LOG3, "Opt Loopback DCO Val, I = %d, Q = %d\n", dcoOptVal.IDcoCorrect, dcoOptVal.QDcoCorrect);
    if ((retVal = rfSetDCOffset(pMac, rxChain, rxGain, dcoOptVal)) != eHAL_STATUS_SUCCESS) { return (retVal); }

    if ((retVal = asicSetPhyCalMode(pMac, PHY_CAL_MODE_NORMAL)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    if ((retVal = asicZeroFineDCOCorrection(pMac, PHY_NO_RX_CHAINS)) != eHAL_STATUS_SUCCESS) { return (retVal); }

    return retVal;
}

#endif

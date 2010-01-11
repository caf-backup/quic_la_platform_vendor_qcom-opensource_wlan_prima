/**
 *
   Airgo Networks, Inc proprietary.
   All Rights Reserved, Copyright 2005
   This program is the confidential and proprietary product of Airgo Networks Inc.
   Any Unauthorized use, reproduction or transfer of this program is strictly prohibited.


   asicWfm.cc: Encapsulates functionality to setup, start and stop test waveforms
   Author:  Mark Nelson
   Date:    2/15/05

   History -
   Date        Modified by              Modification Information
  --------------------------------------------------------------------------

 */

#include <ani_assert.h>
#include <sys_api.h>
/*
static const tANI_U32 wfmNewMem[] =
{
    256,
     8,
     26825,
     9,
     28811,
     10,
     6215,
     11,
     20480,
     11,
     8121,
     11,
     30581,
     10,
     28471,
     9,
     1792,
     8,
     20179,
     6,
     24242,
     4,
     16029,
     2,
     1686,
     0,
     52893,
     61,
     44722,
     59,
     48851,
     57,
     1792,
     56,
     40759,
     54,
     38773,
     53,
     61369,
     52,
     45056,
     52,
     59463,
     52,
     37003,
     53,
     39113,
     54,
     256,
     56,
     47405,
     57,
     43342,
     59,
     51555,
     61,
     362,
     0,
     14691,
     2,
     22862,
     4,
     18733,
     6,
     256,
     8,
     28811,
     10,
     20480,
     11,
     30581,
     10,
     1792,
     8,
     24242,
     4,
     1686,
     0,
     44722,
     59,
     1792,
     56,
     38773,
     53,
     45056,
     52,
     37003,
     53,
     256,
     56,
     43342,
     59,
     362,
     0,
     22862,
     4,
     256,
     8,
     20480,
     11,
     1792,
     8,
     1686,
     0,
     1792,
     56,
     45056,
     52,
     256,
     56,
     362,
     0,
     256,
     8,
     1792,
     8,
     1792,
     56,
     256,
     56,
     256,
     8,
     18733,
     6,
     22862,
     4,
     14691,
     2,
     362,
     0,
     51555,
     61,
     43342,
     59,
     47405,
     57,
     256,
     56,
     39113,
     54,
     37003,
     53,
     59463,
     52,
     45056,
     52,
     61369,
     52,
     38773,
     53,
     40759,
     54,
     1792,
     56,
     48851,
     57,
     44722,
     59,
     52893,
     61,
     1686,
     0,
     16029,
     2,
     24242,
     4,
     20179,
     6,
     1792,
     8,
     28471,
     9,
     30581,
     10,
     8121,
     11,
     20480,
     11,
     6215,
     11,
     28811,
     10,
     26825,
     9,
     256,
     8,
     22862,
     4,
     362,
     0,
     43342,
     59,
     256,
     56,
     37003,
     53,
     45056,
     52,
     38773,
     53,
     1792,
     56,
     44722,
     59,
     1686,
     0,
     24242,
     4,
     1792,
     8,
     30581,
     10,
     20480,
     11,
     28811,
     10,
     256,
     8,
     362,
     0,
     256,
     56,
     45056,
     52,
     1792,
     56,
     1686,
     0,
     1792,
     8,
     20480,
     11,
     256,
     8,
     256,
     56,
     1792,
     56,
     1792,
     8,
     256,
     8,
     47196,
     1,
     40811,
     62,
     65506,
     63,
     12261,
     0,
     46854,
     3,
     22399,
     3,
     10379,
     58,
     10331,
     57,
     12287,
     0,
     61433,
     61,
     65490,
     59,
     55399,
     4,
     22787,
     7,
     28698,
     0,
     57194,
     62,
     0,
     0,
     38841,
     0,
     28408,
     4,
     57279,
     1,
     51355,
     56,
     36926,
     58,
     43006,
     0,
     51185,
     60,
     12251,
     61,
     28825,
     6,
     14578,
     6,
     30683,
     63,
     12155,
     63,
     63506,
     63,
     12166,
     1,
     57086,
     4,
     0,
     0,
     61595,
     55,
     12325,
     60,
     38911,
     0,
     57317,
     59,
     57326,
     62,
     37063,
     7,
     51406,
     4,
     57253,
     62,
     36760,
     63,
     61460,
     63,
     63314,
     1,
     57113,
     4,
     63547,
     61,
     49293,
     55,
     51218,
     61,
     0,
     0,
     22491,
     59,
     53262,
     0,
     6379,
     8,
     14491,
     3,
     38783,
     62,
     53181,
     63,
     63493,
     63,
     57125,
     2,
     24389,
     4,
     63594,
     59,
     12406,
     56,
     10245,
     63,
     6143,
     63,
     24531,
     59,
     57399,
     2
};
*/

const tWaveformSample pWave[184] =
{
    {   256,    256     },
    {   201,    301     },
    {   139,    334     },
    {   71,     355     },
    {   0,      362     },
    {   -71,    355     },
    {   -139,   334     },
    {   -201,   301     },
    {   -256,   256     },
    {   -301,   201     },
    {   -334,   139     },
    {   -355,   71      },
    {   -362,   0       },
    {   -355,   -71     },
    {   -334,   -139    },
    {   -301,   -201    },
    {   -256,   -256    },
    {   -201,   -301    },
    {   -139,   -334    },
    {   -71,    -355    },
    {   0,      -362    },
    {   71,     -355    },
    {   139,    -334    },
    {   201,    -301    },
    {   256,    -256    },
    {   301,    -201    },
    {   334,    -139    },
    {   355,    -71     },
    {   362,    0       },
    {   355,    71      },
    {   334,    139     },
    {   301,    201     },
    {   256,    256     },
    {   139,    334     },
    {   0,      362     },
    {   -139,   334     },
    {   -256,   256     },
    {   -334,   139     },
    {   -362,   0       },
    {   -334,   -139    },
    {   -256,   -256    },
    {   -139,   -334    },
    {   0,      -362    },
    {   139,    -334    },
    {   256,    -256    },
    {   334,    -139    },
    {   362,    0       },
    {   334,    139     },
    {   256,    256     },
    {   0,      362     },
    {   -256,   256     },
    {   -362,   0       },
    {   -256,   -256    },
    {   0,      -362    },
    {   256,    -256    },
    {   362,    0       },
    {   256,    256     },
    {   -256,   256     },
    {   -256,   -256    },
    {   256,    -256    },
    {   256,    256     },
    {   301,    201     },
    {   334,    139     },
    {   355,    71      },
    {   362,    0       },
    {   355,    -71     },
    {   334,    -139    },
    {   301,    -201    },
    {   256,    -256    },
    {   201,    -301    },
    {   139,    -334    },
    {   71,     -355    },
    {   0,      -362    },
    {   -71,    -355    },
    {   -139,   -334    },
    {   -201,   -301    },
    {   -256,   -256    },
    {   -301,   -201    },
    {   -334,   -139    },
    {   -355,   -71     },
    {   -362,   0       },
    {   -355,   71      },
    {   -334,   139     },
    {   -301,   201     },
    {   -256,   256     },
    {   -201,   301     },
    {   -139,   334     },
    {   -71,    355     },
    {   0,      362     },
    {   71,     355     },
    {   139,    334     },
    {   201,    301     },
    {   256,    256     },
    {   334,    139     },
    {   362,    0       },
    {   334,    -139    },
    {   256,    -256    },
    {   139,    -334    },
    {   0,      -362    },
    {   -139,   -334    },
    {   -256,   -256    },
    {   -334,   -139    },
    {   -362,   0       },
    {   -334,   139     },
    {   -256,   256     },
    {   -139,   334     },
    {   0,      362     },
    {   139,    334     },
    {   256,    256     },
    {   362,    0       },
    {   256,    -256    },
    {   0,      -362    },
    {   -256,   -256    },
    {   -362,   0       },
    {   -256,   256     },
    {   0,      362     },
    {   256,    256     },
    {   256,    -256    },
    {   -256,   -256    },
    {   -256,   256     },
    {   256,    256     },
    {   92,     55      },
    {   -149,   -45     },
    {   -30,    -1      },
    {   -27,    5       },
    {   -250,   118     },
    {   -129,   106     },
    {   139,    -187    },
    {   91,     -219    },
    {   -1,     5       },
    {   -7,     -67     },
    {   -46,    -129    },
    {   103,    155     },
    {   259,    235     },
    {   26,     14      },
    {   -150,   -37     },
    {   0,      0       },
    {   -71,    18      },
    {   -264,   141     },
    {   -65,    59      },
    {   155,    -231    },
    {   62,     -174    },
    {   -2,     20      },
    {   -15,    -104    },
    {   -37,    -91     },
    {   153,    206     },
    {   242,    199     },
    {   -37,    -18     },
    {   -133,   -27     },
    {   18,     -1      },
    {   -122,   37      },
    {   -258,   155     },
    {   0,      0       },
    {   155,    -258    },
    {   37,     -122    },
    {   -1,     18      },
    {   -27,    -133    },
    {   -18,    -37     },
    {   199,    242     },
    {   206,    153     },
    {   -91,    -37     },
    {   -104,   -15     },
    {   20,     -2      },
    {   -174,   62      },
    {   -231,   155     },
    {   59,     -65     },
    {   141,    -264    },
    {   18,     -71     },
    {   0,      0       },
    {   -37,    -150    },
    {   14,     26      },
    {   235,    259     },
    {   155,    103     },
    {   -129,   -46     },
    {   -67,    -7      },
    {   5,      -1      },
    {   -219,   91      },
    {   -187,   139     },
    {   106,    -129    },
    {   118,    -250    },
    {   5,      -27     },
    {   -1,     -30     },
    {   -45,    -149    },
    {   55,     92      }
};

#ifdef ANI_MANF_DIAG
eHalStatus asicTxFirSetChainBypass(tpAniSirGlobal pMac, ePhyTxChains txChain, tANI_BOOLEAN chainBypassEnable)
{
    switch (txChain)
    {
        case PHY_TX_CHAIN_0:
            rdModWrAsicField(pMac, QWLAN_TXFIR_CFG_REG, QWLAN_TXFIR_CFG_CHAIN0FIRBYPASS_MASK, QWLAN_TXFIR_CFG_CHAIN0FIRBYPASS_OFFSET, (tANI_U32)chainBypassEnable);
            return (eHAL_STATUS_SUCCESS);
        default:
            //TODO: phyLog(LOGE, "ERROR: Incorrect Tx chain");
            assert(0);
            return (eHAL_STATUS_FAILURE);;
    }
}

eHalStatus asicTxFirSetPaOverride(tpAniSirGlobal pMac, tANI_BOOLEAN overrideEnable, ePhyTxChains chainPaEnables)
{

    if (overrideEnable == eANI_BOOLEAN_TRUE)
    {
        switch (chainPaEnables)
        {
            //Modify the pa_override and pa_override_value fields together
            case PHY_TX_CHAIN_0:
                rdModWrAsicField(pMac, QWLAN_TXFIR_CFG_REG,
                                 (QWLAN_TXFIR_CFG_PA_OVERRIDE_VALUE_MASK | QWLAN_TXFIR_CFG_PA_OVERRIDE_MASK),
                                 QWLAN_TXFIR_CFG_PA_OVERRIDE_OFFSET,
                                 3
                                );
                return (eHAL_STATUS_SUCCESS);
            case PHY_NO_TX_CHAINS:
                rdModWrAsicField(pMac, QWLAN_TXFIR_CFG_REG,
                                 (QWLAN_TXFIR_CFG_PA_OVERRIDE_VALUE_MASK | QWLAN_TXFIR_CFG_PA_OVERRIDE_MASK),
                                 QWLAN_TXFIR_CFG_PA_OVERRIDE_OFFSET,
                                 1
                                );
                return (eHAL_STATUS_SUCCESS);
            default:
                //TODO: phyLog(LOGE, "ERROR: Incorrect Tx chain");
                assert(0);
            return (eHAL_STATUS_FAILURE);;
        }
    }
    else
    {
        rdModWrAsicField(pMac, QWLAN_TXFIR_CFG_REG, QWLAN_TXFIR_CFG_PA_OVERRIDE_MASK, QWLAN_TXFIR_CFG_PA_OVERRIDE_OFFSET, 0);
        return (eHAL_STATUS_SUCCESS);
    }
}
#endif

/*
//load the raw memory for the time being. loading the I and Q samples seem to be a problem.
eHalStatus asicSetupTestWaveform(tpAniSirGlobal pMac, const tWaveformSample *pWave, tANI_U16 numSamples, tANI_BOOLEAN clk80)
{
    rdModWrAsicField(pMac, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_REG, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_PHYDBG_MASK, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_PHYDBG_OFFSET, 1);
    rdModWrAsicField(pMac, QWLAN_TXCLKCTRL_APB_BLOCK_CLK_EN_REG, QWLAN_TXCLKCTRL_APB_BLOCK_CLK_EN_WFM_MASK, QWLAN_TXCLKCTRL_APB_BLOCK_CLK_EN_WFM_OFFSET, 1);
    {
        tANI_U32 i = 0;
        for(i = 0; i < sizeof(wfmNewMem)/4; i++)
        {
            halWriteRegister(pMac, (QWLAN_PHYDBG_DBGMEM_MREG + (4*i)), wfmNewMem[i]);
        }
    }
    rdModWrAsicField(pMac, QWLAN_TXCLKCTRL_APB_BLOCK_CLK_EN_REG, QWLAN_TXCLKCTRL_APB_BLOCK_CLK_EN_WFM_MASK, QWLAN_TXCLKCTRL_APB_BLOCK_CLK_EN_WFM_OFFSET, 0);

    return eHAL_STATUS_SUCCESS;
}
*/

eHalStatus asicSetupTestWaveform(tpAniSirGlobal pMac, const tWaveformSample *pWave, tANI_U16 numSamples, tANI_BOOLEAN clk80)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;

    pMac->hphy.wfm_clk80 = clk80;                   //records what clock sample rate 1 = 80, 0 = 20 MHz


    rdModWrAsicField(pMac, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_REG, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_PHYDBG_MASK, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_PHYDBG_OFFSET, 1);
    rdModWrAsicField(pMac, QWLAN_TXCLKCTRL_APB_BLOCK_CLK_EN_REG, QWLAN_TXCLKCTRL_APB_BLOCK_CLK_EN_WFM_MASK, QWLAN_TXCLKCTRL_APB_BLOCK_CLK_EN_WFM_OFFSET, 1);
    {
        tANI_U16 sample = 0;
        tANI_U32 Samples[MAX_TEST_WAVEFORM_SAMPLES * 2];

        //lower 16 bits goes into even numbered U32 words, and the higher 16 bits goes into the odd numbered U32 words
        for (sample = 0; (sample < numSamples); sample++)
        {
            Samples[sample * 2] = ((tANI_U32)pWave[sample].I & 0x7FF) | (((tANI_U32)pWave[sample].Q & 0x1F) << 11);  //11 bits for I, 5 LSBs of Q
            Samples[(sample * 2) + 1] = (((tANI_U32)pWave[sample].Q & 0x7FF) >> 5);

            SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_DBGMEM_MREG + (sample * 8), Samples[sample * 2]);
            SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_DBGMEM_MREG + ((sample * 8) + 4), Samples[(sample * 2) + 1]);
            //phyLog(LOGE, "I=%d        Q=%d\n", pWave[sample].I, pWave[sample].Q);
        }

        //SET_PHY_MEMORY(pMac->hHdd, QWLAN_PHYDBG_DBGMEM_MREG, Samples, numSamples * 2);
    }

    return (retVal);
}

#ifdef ANI_MANF_DIAG
eHalStatus asicStartTestWaveform(tpAniSirGlobal pMac, eWaveMode playback, tANI_U32 startIndex, tANI_U32 endIndex)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;

    //first stop any previous waveform gen operation
    asicStopTestWaveform(pMac);
    {
        // Explicitely enable clcok to PHYDBG module
        rdModWrAsicField(pMac, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_REG, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_PHYDBG_MASK, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_PHYDBG_OFFSET, 1);

        // enable wfm clock in txclk control. It will be set also after asicStopTestWaveform is called.
        rdModWrAsicField(pMac, QWLAN_TXCLKCTRL_APB_BLOCK_CLK_EN_REG, QWLAN_TXCLKCTRL_APB_BLOCK_CLK_EN_WFM_MASK, QWLAN_TXCLKCTRL_APB_BLOCK_CLK_EN_WFM_OFFSET, 1);

        // disable clock-gating for TXFIR and TXCTL
        rdModWrAsicField(pMac, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TXFIR_MASK, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TXFIR_OFFSET, 1);
        rdModWrAsicField(pMac, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TXCTL_MASK, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TXCTL_OFFSET, 1);

        // Experimenting with clocks to get PA Override to work
        // rdModWrAsicField(pMac, QWLAN_RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG,
        //                             QWLAN_RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_CAL_MASK,
        //                             QWLAN_RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_CAL_OFFSET, 1);
        //
        // rdModWrAsicField(pMac, QWLAN_RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG,
        //                     QWLAN_RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_RXFIR_MASK,
        //                     QWLAN_RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_RXFIR_OFFSET, 1);
        //
        // rdModWrAsicField(pMac, QWLAN_RXACLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG,
        //                                 QWLAN_RXACLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_FFT_MASK,
        //                                 QWLAN_RXACLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_FFT_OFFSET, 1);
        //
        // rdModWrAsicField(pMac, QWLAN_RXACLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG,
        //                                 QWLAN_RXACLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TDC_MASK,
        //                                 QWLAN_RXACLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TDC_OFFSET, 1);
        //
        // rdModWrAsicField(pMac, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG,
        //                     QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TPC_MASK,
        //                     QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TPC_OFFSET, 1);
        //
        // rdModWrAsicField(pMac, QWLAN_RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG,
        //                     QWLAN_RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_XBAR_MASK,
        //                     QWLAN_RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_XBAR_OFFSET, 1);
        //end experiment


        // spatial rotation will alter the waveform and should be off
        //SET_PHY_REG(pMac->hHdd, TXFIR_SPATIAL_ROTATION_REG, 0);

        asicTxFirSetPaOverride(pMac, eANI_BOOLEAN_TRUE, PHY_TX_CHAIN_0);

        // need explanation, enable clock to TXCTL (according to python script)
        // not critical, still functional without it.
        // BW 062008: Sometimes the TXCTL clock gating needs to be temporarily disabled to do certain things.  I know that’s vague, but that’s the best answer I can give right now without having to investigate why.

        rdModWrAsicField(pMac, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TXCTL_MASK, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TXCTL_OFFSET, 0);

        // set conf to to use debug SRAM and put in continous mode unconditionally.
        if (playback == WAVE_CONTINUOUS)
        {
            SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_CFGMODE_REG,QWLAN_PHYDBG_CFGMODE_DBGMEM_SEL1_MASK | QWLAN_PHYDBG_CFGMODE_CONT1_MASK);
        }
        else
        {
            SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_CFGMODE_REG,QWLAN_PHYDBG_CFGMODE_DBGMEM_SEL1_MASK);
        }

        // set up start and stop addresses
        SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_START_ADDR1_REG, startIndex);
        SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_MAX_ADDR1_REG, endIndex);

        // configure PHY CONF Reg and Chain Bypass according to 80MHz (bypass TxFir) or 20MHz
        // routed through TxFir, to DAC.
        if (pMac->hphy.wfm_clk80 == eANI_BOOLEAN_ON)
        {
            //set chain0, 1, 2, 3 bypass bits again since it was reset by asicStopTestWaveform previously.
            //bypass all chains and let the DAC_CONTROL manages which chain actually are transmitted
            asicTxFirSetChainBypass(pMac, PHY_TX_CHAIN_0, eANI_BOOLEAN_ON);
            //80 MHz sampling rate, samples input to TXFIR. dup_ch0 is not set in this case. Also, it is not
            //set in other parts of the code.
            // bandwidth_mode = 80MHz, dup_ch0 = 1, mif_txtest=0, taif_dbsel = 0, txpb_mode = 0x1, txfir_dbgsel = 1
            // i.e. chain 0 ==> 0x0206);
            // txpb_mode field is critical.....
            SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_PLYBCK_CFG_REG,
                        (QWLAN_PHYDBG_PLYBCK_CFG_PLYBK_RATE_EPLYBK_80M << QWLAN_PHYDBG_PLYBCK_CFG_PLYBK_RATE_OFFSET) |
                        (QWLAN_PHYDBG_PLYBCK_CFG_TXPB_MODE_ETXPB32_TXF80_CH0 << QWLAN_PHYDBG_PLYBCK_CFG_TXPB_MODE_OFFSET) |
                        QWLAN_PHYDBG_PLYBCK_CFG_TXFIR_DBGSEL_MASK |
                        QWLAN_PHYDBG_PLYBCK_CFG_DUP_CH0_MASK);
        }
        else
        {
            // 20 MHz sampling rate, samples input to TXFIR
            // bandwidth_mode = 20MHz, dup_ch0 = 1, mif_txtest=0, taif_dbsel = 0, txpb_mode = 0x1, txfir_dbgsel = 1
            // i.e. chain 0 ==> 0x8206);
            // txpb_mode field is critical.....
            SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_PLYBCK_CFG_REG,
                        (QWLAN_PHYDBG_PLYBCK_CFG_PLYBK_RATE_EPLYBK_20M << QWLAN_PHYDBG_PLYBCK_CFG_PLYBK_RATE_OFFSET) |
                        (QWLAN_PHYDBG_PLYBCK_CFG_TXPB_MODE_ETXPB32_TXF80_CH0 << QWLAN_PHYDBG_PLYBCK_CFG_TXPB_MODE_OFFSET) |
                        QWLAN_PHYDBG_PLYBCK_CFG_TXFIR_DBGSEL_MASK |
                        QWLAN_PHYDBG_PLYBCK_CFG_DUP_CH0_MASK);
        };

        //Gen start pluse to start waveform generation to TXFIR
        SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_START1_REG, QWLAN_PHYDBG_START1_START_MASK);
    }

    // remaining setting done by upper layer after return of this function call.
    // (a) turn on DAC
    // SET_PHY_REG(pMac->hHdd, TXCTL_DAC_CONTROL_REG, determined by upper function);
    // (b) config TxTCL for FIR Mode operation
    // SET_PHY_REG(pMac->hHdd, TXCTL_FIR_MODE_REG, determined by upper function);

    return (retVal);
}

#define STOP_ITER_LIMIT 10000

eHalStatus asicStopTestWaveform(tpAniSirGlobal pMac)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;

    SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_CFGMODE_REG, QWLAN_PHYDBG_CFGMODE_STOP1_MASK);
    // set back to use SRAM for playback
    rdModWrAsicField(pMac, QWLAN_PHYDBG_CFGMODE_REG,
                     QWLAN_PHYDBG_CFGMODE_DBGMEM_SEL1_MASK,
                     QWLAN_PHYDBG_CFGMODE_DBGMEM_SEL1_OFFSET,
                     0
                    );

    {
        tANI_U32 i = 0;
        tANI_U32 reg;

        do
        {
            GET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_STATUS_REG, &reg);
        }
        while ((i++ < STOP_ITER_LIMIT) && (reg & QWLAN_PHYDBG_STATUS_TXSTATE_MASK));

        if (i >= STOP_ITER_LIMIT)
        {
            assert(0);
            return (eHAL_STATUS_FAILURE);
        }
    }

    //SET_PHY_REG(pMac->hHdd, TXFIR_SPATIAL_ROTATION_REG, hphy.txfir.spatialRotation); //return spatial rotation to default value

    //Must turn chain0 & chain 1 & chain 2 & chain 3 bypass bits in TxFir back to normal
    //disable bypass for all chains
    asicTxFirSetChainBypass(pMac, PHY_TX_CHAIN_0, eANI_BOOLEAN_OFF);

    SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_RST1_REG, 1);

    rdModWrAsicField(pMac, QWLAN_MIF_MIF_MEM_CFG_REG,
                     QWLAN_MIF_MIF_MEM_CFG_MIF_MEM_CFG_MASK,
                     QWLAN_MIF_MIF_MEM_CFG_MIF_MEM_CFG_OFFSET,
                     0    //set back to normal for host access
                    );

    // Explicitely disable clcok to PHYDBG module
    //rdModWrAsicField(pMac, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_REG, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_PHYDBG_MASK, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_PHYDBG_OFFSET, 0);

    // disenable wfm clock in txclk control. It will be set also after asicStopTestWaveform is called.
    rdModWrAsicField(pMac, QWLAN_TXCLKCTRL_APB_BLOCK_CLK_EN_REG, QWLAN_TXCLKCTRL_APB_BLOCK_CLK_EN_WFM_MASK, QWLAN_TXCLKCTRL_APB_BLOCK_CLK_EN_WFM_OFFSET, 0);

    // enable clock gating for TXFIR and TXCTL
    rdModWrAsicField(pMac, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TXFIR_MASK, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TXFIR_OFFSET, 0);
    rdModWrAsicField(pMac, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TXCTL_MASK, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TXCTL_OFFSET, 0);

    // specific for Virgo: disable pa_override and provide override value

    asicTxFirSetPaOverride(pMac, eANI_BOOLEAN_FALSE, PHY_ALL_TX_CHAINS);
    return retVal;
}
#endif


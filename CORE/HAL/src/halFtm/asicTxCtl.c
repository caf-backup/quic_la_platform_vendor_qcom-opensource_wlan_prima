/**
 *
   Airgo Networks, Inc proprietary.
   All Rights Reserved, Copyright 2005
   This program is the confidential and proprietary product of Airgo Networks Inc.
   Any Unauthorized use, reproduction or transfer of this program is strictly prohibited.


   asicTXCTL.cc:
   Author:  Mark Nelson
   Date:    3/22/05

   History -
   Date        Modified by              Modification Information
  --------------------------------------------------------------------------

 */

#include <sys_api.h>


#ifndef WLAN_FTM_STUB

eHalStatus asicEnableTxDACs(tpAniSirGlobal pMac, ePhyTxChains txChainsOn, tANI_BOOLEAN override, tANI_BOOLEAN wfm)
{
    eHalStatus retVal;
    tANI_U32 val = 0;
    tANI_U32 txen;



    // if override = true, then any chain that is not enabled, will have the RF enable(txen) disabled
    val = TXCTL_DAC_OVERRIDE_EN_MASK | TXCTL_RF_OVERRIDE_EN_MASK;

    switch (txChainsOn)
    {
        case PHY_ALL_TX_CHAINS:
            //enable DACS and power up RF for all Tx chain
            txen = (TXCTL_DAC_TX0);
            val |=  TXCTL_DAC_RF_EN_MASK;
            break;

        case PHY_TX_CHAIN_0:
            //disable other chains DAC and RF Tx Power
            //enable  PHY_TX_CHAIN_0 DAC and RF Tx Power
            txen = TXCTL_DAC_TX0;
            val |=  ((TXCTL_DAC_RF_EN_MASK    & (txen << TXCTL_DAC_RF_EN_OFFSET)) |
                     (TXCTL_DAC_STBY_MASK     & ((TXCTL_DAC_TX1 | TXCTL_DAC_TX2 | TXCTL_DAC_TX3) << TXCTL_DAC_STBY_OFFSET))
                    );
            break;

        case PHY_NO_TX_CHAINS:
            //disable DACS and power down RF for all Tx chains
            txen = 0;
            val |=  TXCTL_DAC_STBY_MASK;
            break;
        default:
            //TODO: phyLog(LOGE, "ERROR: Incorrect Tx chain");
            assert(0);
            return (eHAL_STATUS_FAILURE);
    }

    if (override == eANI_BOOLEAN_ON)
    {
        rdModWrAsicField(pMac, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TXCTL_MASK, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TXCTL_OFFSET, 1);
        rdModWrAsicField(pMac, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TXFIR_MASK, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TXFIR_OFFSET, 1);
        SET_PHY_REG(pMac->hHdd, QWLAN_TXCTL_DAC_CONTROL_REG, val);

        {
            volatile tANI_U32 verifyReg;
            GET_PHY_REG(pMac->hHdd, QWLAN_TXCTL_DAC_CONTROL_REG, &verifyReg);
            verifyReg = verifyReg;
        }

        if (wfm == eANI_BOOLEAN_TRUE)
        {
            //set both chains for waveform output
            SET_PHY_REG(pMac->hHdd, QWLAN_TXCTL_FIR_MODE_REG,
                          (TXCTL_FIR_TX_MODE_SELECT_MASK | (txen << QWLAN_TXCTL_FIR_MODE_ANT_EN_OFFSET) |
                           QWLAN_TXCTL_FIR_MODE_DIS_11MBPS_MASK | QWLAN_TXCTL_FIR_MODE_DIS_5MBPS_MASK
                          )
                       );
            {
                volatile tANI_U32 verifyReg;
                GET_PHY_REG(pMac->hHdd, QWLAN_TXCTL_FIR_MODE_REG, &verifyReg);
                verifyReg = verifyReg;
            }
        }

        rdModWrAsicField(pMac, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TXCTL_MASK, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TXCTL_OFFSET, 0);
        //don't disable dynamic txfir clocks here - let this be done in asicStopTestWaveform
        //otherwise, we only see a DC tone
    }
    else
    {
        rdModWrAsicField(pMac, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TXCTL_MASK, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TXCTL_OFFSET, 1);
        rdModWrAsicField(pMac, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TXFIR_MASK, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TXFIR_OFFSET, 1);
        //when switching off override, first override to disable the RF chains
        val = (TXCTL_DAC_OVERRIDE_EN_MASK | TXCTL_RF_OVERRIDE_EN_MASK | TXCTL_DAC_STBY_MASK);
        SET_PHY_REG(pMac->hHdd, QWLAN_TXCTL_DAC_CONTROL_REG, val);

        SET_PHY_REG(pMac->hHdd, QWLAN_TXCTL_DAC_CONTROL_REG, TXCTL_DAC_STBY_MASK);

        SET_PHY_REG(pMac->hHdd, QWLAN_TXCTL_FIR_MODE_REG, (txen << QWLAN_TXCTL_FIR_MODE_ANT_EN_OFFSET) |
                                               QWLAN_TXCTL_FIR_MODE_DIS_11MBPS_MASK | QWLAN_TXCTL_FIR_MODE_DIS_5MBPS_MASK
                   );

        rdModWrAsicField(pMac, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TXCTL_MASK, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TXCTL_OFFSET, 0);
        rdModWrAsicField(pMac, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TXFIR_MASK, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TXFIR_OFFSET, 0);
    }

    return (eHAL_STATUS_SUCCESS);

}
#endif

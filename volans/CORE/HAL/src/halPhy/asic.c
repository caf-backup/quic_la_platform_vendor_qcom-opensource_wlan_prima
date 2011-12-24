/** ------------------------------------------------------------------------- * 
    ------------------------------------------------------------------------- *  

  
    \file asic.c
  
    \brief contains other phy layer asic module functions
  
    $Id$ 
  
  
    Copyright (c) 2011 Qualcomm Atheros, Inc. 
    All Rights Reserved. 
    Qualcomm Atheros Confidential and Proprietary. 
  
    Copyright (C) 2006 Airgo Networks, Incorporated
  
   ========================================================================== */

#include "ani_assert.h"
#include "sys_api.h"

eHalStatus asicWaitRegVal( tHddHandle hHdd, tANI_U32 reg, tANI_U32 mask,
                             tANI_U32 waitRegVal, tANI_U32 perIterWaitInNanoSec,
                             tANI_U32 numIter, tANI_U32 *pReadRegVal )
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hHdd;
    eHalStatus  nStatus = eHAL_STATUS_SUCCESS;
    //tANI_U32    waitTime = (perIterWaitInNanoSec + 999) / 1000;

    do
    {
        nStatus = palReadRegister(pMac->hHdd, reg, pReadRegVal);
        if (nStatus != eHAL_STATUS_SUCCESS)
        {
            break;
        }

        if ((*pReadRegVal & mask) == waitRegVal)
        {
            break;
        }

        if (--numIter)
        {
            sirBusyWait(1000);  //wait 1 microsecond
        }
        else
        {
            nStatus = eHAL_STATUS_FAILURE;
            break;
        }

    } while (1);

    return nStatus;
}

eHalStatus asicEnablePhyClocks(tpAniSirGlobal pMac)
{
    tANI_U32 regVal = 0;
    eHalStatus retVal = eHAL_STATUS_SUCCESS;

    //Enable all the clocks
    SET_PHY_REG(pMac->hHdd, QWLAN_RXCLKCTRL_ROOT_CLK_EN_REG, 
                                    (QWLAN_RXCLKCTRL_ROOT_CLK_EN_CLK20_MASK |
                                    QWLAN_RXCLKCTRL_ROOT_CLK_EN_CLK40_MASK | 
                                    QWLAN_RXCLKCTRL_ROOT_CLK_EN_CLK80_40D_MASK |
                                    QWLAN_RXCLKCTRL_ROOT_CLK_EN_CLK80_40S_MASK | 
                                    QWLAN_RXCLKCTRL_ROOT_CLK_EN_CLK160_MASK |
                                    QWLAN_RXCLKCTRL_ROOT_CLK_EN_CLK_MIF_PHYDBG_INTF_MASK | 
                                    QWLAN_RXCLKCTRL_ROOT_CLK_EN_CLK80_MASK));

    SET_PHY_REG(pMac->hHdd, QWLAN_RXACLKCTRL_ROOT_CLK_EN_REG, 
                                    (QWLAN_RXACLKCTRL_ROOT_CLK_EN_CLK80_40D_MASK |
                                    QWLAN_RXACLKCTRL_ROOT_CLK_EN_CLK160_MASK |
                                    QWLAN_RXACLKCTRL_ROOT_CLK_EN_CLK80_MASK));

    SET_PHY_REG(pMac->hHdd, QWLAN_TXCLKCTRL_ROOT_CLK_EN_REG,
                                    QWLAN_TXCLKCTRL_ROOT_CLK_EN_CLK160_MASK |
                                    QWLAN_TXCLKCTRL_ROOT_CLK_EN_CLK80_MASK);

    SET_PHY_REG(pMac->hHdd, QWLAN_RXCLKCTRL_APB_SYNC_RESET_REG, 
                                    QWLAN_RXCLKCTRL_APB_SYNC_RESET_VALUE_MASK);

    SET_PHY_REG(pMac->hHdd, QWLAN_RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_AGC_REG, 0);

    GET_PHY_REG(pMac->hHdd, QWLAN_RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_AGC_REG, &regVal);

    regVal |= QWLAN_RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_AGC_WLAN_POS_MASK;
    SET_PHY_REG(pMac->hHdd, QWLAN_RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_AGC_REG, regVal);

    SET_PHY_REG(pMac->hHdd, QWLAN_RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, 0);

    GET_PHY_REG(pMac->hHdd, QWLAN_RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, &regVal);

    regVal |= QWLAN_RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_XBAR_MASK;
    SET_PHY_REG(pMac->hHdd, QWLAN_RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, regVal);

    GET_PHY_REG(pMac->hHdd, QWLAN_RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, &regVal);

    regVal |= QWLAN_RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_PMICCA_MASK;
    SET_PHY_REG(pMac->hHdd, QWLAN_RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, regVal);

    GET_PHY_REG(pMac->hHdd, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_REG, &regVal);

    regVal &= ~QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_PHYDBG_APB_MASK;
    SET_PHY_REG(pMac->hHdd, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_REG, regVal);

    GET_PHY_REG(pMac->hHdd, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_REG, &regVal);

    regVal |= QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_RFIF_MASK;
    SET_PHY_REG(pMac->hHdd, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_REG, regVal);

    GET_PHY_REG(pMac->hHdd, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_REG, &regVal);

    regVal |= QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_PMI_MASK;
    SET_PHY_REG(pMac->hHdd, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_REG, regVal);

    GET_PHY_REG(pMac->hHdd, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_REG, &regVal);

    regVal |= QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_AGC_MASK;
    SET_PHY_REG(pMac->hHdd, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_REG, regVal);

    GET_PHY_REG(pMac->hHdd, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_REG, &regVal);

    regVal &= ~QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_PHYINT_MASK;
    SET_PHY_REG(pMac->hHdd, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_REG, regVal);

    GET_PHY_REG(pMac->hHdd, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_REG, &regVal);

    regVal &= ~QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_TMUX_MASK;
    SET_PHY_REG(pMac->hHdd, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_REG, regVal);

    SET_PHY_REG(pMac->hHdd, QWLAN_RXACLKCTRL_APB_SYNC_RESET_REG,
                                    QWLAN_RXACLKCTRL_APB_SYNC_RESET_VALUE_MASK);

    SET_PHY_REG(pMac->hHdd, QWLAN_RXACLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, 0);

    SET_PHY_REG(pMac->hHdd, QWLAN_TXCLKCTRL_APB_SYNC_RESET_REG,
                                    QWLAN_TXCLKCTRL_APB_SYNC_RESET_VALUE_MASK);

    SET_PHY_REG(pMac->hHdd, QWLAN_TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, 0);

    return retVal;
}

//file can be deleted for Volans

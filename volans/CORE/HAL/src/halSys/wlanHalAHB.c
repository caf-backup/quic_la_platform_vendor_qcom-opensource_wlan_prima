/**
 *
 *  @file:      halAHB.c
 *
 *  @brief:     Provides all the APIs to set AHB.
 *
 *  Copyright (C) 2008, Qualcomm, Inc. All rights reserved.
 *
 *  Change History:
 * ---------------------------------------------------------------
 * 03/13/2009  Dinesh Upadhyay			File created.
 */

#include "halAHB.h" 

//priority values that are different than default.
tAhbPri ahbPriList[] = {
    {QWLAN_CAHB_CAHB_RXP_PL_REG, QWLAN_CAHB_CAHB_RXP_PL_PRIORITY_OFFSET, QWLAN_CAHB_CAHB_RXP_PL_PRIORITY_MASK, 5},
    {QWLAN_CAHB_CAHB_DBR_PL_REG, QWLAN_CAHB_CAHB_DBR_PL_PRIORITY_OFFSET, QWLAN_CAHB_CAHB_DBR_PL_PRIORITY_MASK, 3},        
    {QWLAN_DAHB_DAHB_RXP_PL_REG, QWLAN_DAHB_DAHB_RXP_PL_PRIORITY_OFFSET, QWLAN_DAHB_DAHB_RXP_PL_PRIORITY_MASK, 5},
    {QWLAN_DAHB_DAHB_TXP_PL_REG, QWLAN_DAHB_DAHB_TXP_PL_PRIORITY_OFFSET, QWLAN_DAHB_DAHB_TXP_PL_PRIORITY_MASK, 5},
    {QWLAN_DAHB_DAHB_BMUW_PL_REG, QWLAN_DAHB_DAHB_BMUW_PL_PRIORITY_OFFSET, QWLAN_DAHB_DAHB_BMUW_PL_PRIORITY_MASK, 3},
    {QWLAN_DAHB_DAHB_TPE_PL_REG, QWLAN_DAHB_DAHB_TPE_PL_PRIORITY_OFFSET, QWLAN_DAHB_DAHB_TPE_PL_PRIORITY_MASK, 4},
    {QWLAN_DAHB_DAHB_RPE_PL_REG, QWLAN_DAHB_DAHB_RPE_PL_PRIORITY_OFFSET, QWLAN_DAHB_DAHB_RPE_PL_PRIORITY_MASK, 4},
    {QWLAN_DAHB_DAHB_ADU_PL_REG, QWLAN_DAHB_DAHB_ADU_PL_PRIORITY_OFFSET, QWLAN_DAHB_DAHB_ADU_PL_PRIORITY_MASK, 6},
    {QWLAN_DAHB_DAHB_SIF_PL_REG, QWLAN_DAHB_DAHB_SIF_PL_PRIORITY_OFFSET, QWLAN_DAHB_DAHB_SIF_PL_PRIORITY_MASK, 3}            
 };


eHalStatus halAhb_Start(tHalHandle hHal, void *arg)
{
    tANI_U32 regVal = 0;
    tANI_U8 loopCnt = 0;

	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

    for(loopCnt = 0; loopCnt < (sizeof(ahbPriList)/sizeof(ahbPriList[0])); loopCnt++)
    {
        halReadRegister(pMac, ahbPriList[loopCnt].regAddr, &regVal);
        regVal &= ~ahbPriList[loopCnt].priMask;
        regVal |= (ahbPriList[loopCnt].pri << ahbPriList[loopCnt].priOffset); 
        halWriteRegister(pMac, ahbPriList[loopCnt].regAddr, regVal);
    }
    return eHAL_STATUS_SUCCESS;
}


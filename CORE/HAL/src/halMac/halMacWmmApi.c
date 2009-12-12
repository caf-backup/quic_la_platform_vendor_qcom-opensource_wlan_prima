/*
 * Airgo Networks, Inc proprietary. All rights reserved.
 * halMacWmmApi.c: Provides all the HAL WMM APIs in this file.
 * Author:    Neelay Das
 * Date:      10/22/2006
 * History:-
 * Date        Modified by            Modification Information
 *
 * --------------------------------------------------------------------------
 *
 */

/* Application Specific include files */
#include "halTypes.h"
#include "halCommonApi.h"

#include "sirApi.h"
#include "sirParams.h"
#include "halDebug.h"
#include "cfgApi.h"
#include "halMTU.h"

void halWmmDumpACRegs(tpAniSirGlobal pMac)
{
    tANI_U32 regVal = 0;
    do
    {
    
        halReadRegister(pMac, QWLAN_TPE_SW_MEDIUM_TIME_THR_AC0_AC1_REG, &regVal);
        HALLOGW( halLog(pMac, LOGW, FL("QWLAN_TPE_SW_MEDIUM_TIME_THR_AC0_AC1_REG = %x\n"),  regVal ));
        halReadRegister(pMac, QWLAN_TPE_SW_MEDIUM_TIME_THR_AC2_AC3_REG, &regVal);
        HALLOGW( halLog(pMac, LOGW, FL("QWLAN_TPE_SW_MEDIUM_TIME_THR_AC2_AC3_REG = %x\n"),  regVal ));            
        halReadRegister(pMac, QWLAN_TPE_SW_ADMISSION_CONTROL_CONTROL_REG_REG, &regVal);
        HALLOGW( halLog(pMac, LOGW, FL("QWLAN_TPE_SW_ADMISSION_CONTROL_CONTROL_REG_REG = %x\n"),  regVal ));                        

    }while(0);
}

static eHalStatus halWmmUpdateTpeMediumTime(tpAniSirGlobal pMac, tANI_U8 aci, tANI_U16 mediumTime)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tANI_U32 regVal = 0;
    tANI_U32 acMask = 0;
    tANI_U32 acOffset = 0;
    tANI_U32 regAddr = 0;
    do
    {
        switch(aci)
        {
            case EDCA_AC_BE:
                acMask = QWLAN_TPE_SW_MEDIUM_TIME_THR_AC0_AC1_SW_MEDIUM_TIME_THR_AC0_MASK;
                acOffset = QWLAN_TPE_SW_MEDIUM_TIME_THR_AC0_AC1_SW_MEDIUM_TIME_THR_AC0_OFFSET;
                regAddr = QWLAN_TPE_SW_MEDIUM_TIME_THR_AC0_AC1_REG;
                break;
            case EDCA_AC_BK:
                acMask = QWLAN_TPE_SW_MEDIUM_TIME_THR_AC0_AC1_SW_MEDIUM_TIME_THR_AC1_MASK;
                acOffset = QWLAN_TPE_SW_MEDIUM_TIME_THR_AC0_AC1_SW_MEDIUM_TIME_THR_AC1_OFFSET;
                regAddr = QWLAN_TPE_SW_MEDIUM_TIME_THR_AC0_AC1_REG;
                break;

            case EDCA_AC_VI:
                acMask = QWLAN_TPE_SW_MEDIUM_TIME_THR_AC2_AC3_SW_MEDIUM_TIME_THR_AC2_MASK;
                acOffset = QWLAN_TPE_SW_MEDIUM_TIME_THR_AC2_AC3_SW_MEDIUM_TIME_THR_AC2_OFFSET;
                regAddr = QWLAN_TPE_SW_MEDIUM_TIME_THR_AC2_AC3_REG;
                break;
            case EDCA_AC_VO:
                acMask = QWLAN_TPE_SW_MEDIUM_TIME_THR_AC2_AC3_SW_MEDIUM_TIME_THR_AC3_MASK;
                acOffset = QWLAN_TPE_SW_MEDIUM_TIME_THR_AC2_AC3_SW_MEDIUM_TIME_THR_AC3_OFFSET;
                regAddr = QWLAN_TPE_SW_MEDIUM_TIME_THR_AC2_AC3_REG;
                break;
            default:
                HALLOGE( halLog(pMac, LOGE, FL("aci = %d is invalid \n"),  aci ));
                status = eHAL_STATUS_FAILURE;
                break;
        }

        if(eHAL_STATUS_SUCCESS != status)
            break;
        halReadRegister(pMac, regAddr, &regVal);
        regVal &= ~acMask;
        regVal |= mediumTime << acOffset;
        
        halWriteRegister(pMac, regAddr, regVal);
    }while(0);
    return status;
}

static eHalStatus halWmmEnableACBkOff(tpAniSirGlobal pMac, tANI_U32 bkOff, tAniBool enable)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tANI_U32 regVal = 0;
    do
    {
        //Ac to backoff mapping for admission control is already done during TPE start.
        //change only the bit for corresponding backoff.        
        halReadRegister(pMac, QWLAN_TPE_SW_ADMISSION_CONTROL_CONTROL_REG_REG, &regVal);
        if(enable)
        {
            regVal |= (1 << (QWLAN_TPE_SW_ADMISSION_CONTROL_CONTROL_REG_SW_AC_VALID_FOR_BKOF_VECTOR_OFFSET + bkOff));        
        }
        else
        {
            regVal &= ~(1 << (QWLAN_TPE_SW_ADMISSION_CONTROL_CONTROL_REG_SW_AC_VALID_FOR_BKOF_VECTOR_OFFSET + bkOff));
        }

        halWriteRegister(pMac, QWLAN_TPE_SW_ADMISSION_CONTROL_CONTROL_REG_REG, regVal);
    }while(0);

    return status;
}

tSirRetStatus
halWmmAddTspec(
    tpAniSirGlobal  pMac,
    tAddTsParams *pTSParams)
{
    tANI_U8 aci; // AC index for the AC to be used for this TSPEC
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tHalCfgSta *pStaEntry;
    tANI_U32 bkOffIdx = 0;
    tTspecTblEntry  *pTspecInfo = &pMac->hal.halMac.tspecInfo[pTSParams->tspecIdx];

    do
    {
        // validate the staid
        if (pTSParams->staIdx >= pMac->hal.halMac.maxSta)
        {
            HALLOGE( halLog(pMac, LOGE, FL("Invalid staid 0x%x\n"),  pTSParams->staIdx ));
            status = eHAL_STATUS_FAILURE;
            break;
        }
    
        // update the tspec info
        pTspecInfo->tsinfo = pTSParams->tspec.tsinfo;
        pTspecInfo->staIdx = pTSParams->staIdx;
    
        // Validate the TSPEC here so that if there is a failure
        // from this point onwards the halWmmDelTspec() routine can cleanup appropriately
        if(!pTspecInfo->valid)
            pTspecInfo->valid = eANI_BOOLEAN_TRUE;
    
        // Update the TC parameters
    
         if(eHAL_STATUS_SUCCESS != halTable_GetStaConfig(pMac, &pStaEntry, (tANI_U8)pTspecInfo->staIdx))
         {
            HALLOGE( halLog(pMac, LOGE, FL("Failed to retrieve SoftMAC config for staid 0x%x\n"),  pTSParams->staIdx ));
            status = eHAL_STATUS_FAILURE;
            break;
         }
    
        aci = HAL_WMM_TID_TO_AC(HAL_WMM_DEFAULT_TID_AC_MAP, pTspecInfo->tsinfo.traffic.userPrio);
    
        HALLOGW( halLog(pMac, LOGW, FL("==Admission Control Registers before adding TSPEC=====\n")));
        halWmmDumpACRegs(pMac);
        //update TPE medium time for the AC.
        if(eHAL_STATUS_SUCCESS != halWmmUpdateTpeMediumTime(pMac, aci, pTSParams->tspec.mediumTime))
        {
            status = eHAL_STATUS_FAILURE;
            break;
        }

        //Now enable the backoff for admission control. 
        bkOffIdx = __halMTU_ac2BkoffIndex(pMac, aci);
        if(eHAL_STATUS_SUCCESS != halWmmEnableACBkOff(pMac, bkOffIdx, eSIR_TRUE))
        {
            status = eHAL_STATUS_FAILURE;
            break;
        }
        HALLOGW( halLog(pMac, LOGW, FL("==Admission Control Registers after adding TSPEC=====\n")));
        halWmmDumpACRegs(pMac);

    } while(0);

    pTSParams->status = status;
    halMsg_GenerateRsp(pMac, SIR_HAL_ADD_TS_RSP, 0, (void *) pTSParams, 0);
    if(status == eHAL_STATUS_SUCCESS)
        return eSIR_SUCCESS;
    else
        return eSIR_FAILURE;
}

tSirRetStatus
halWmmDelTspec(
    tpAniSirGlobal  pMac,
    tDelTsParams *pTSParams)
{
    tANI_U8 aci; // AC index for the AC to be used for this TSPEC
    tTspecTblEntry  *pTspecInfo = &pMac->hal.halMac.tspecInfo[pTSParams->tspecIdx];
    tANI_U32 bkOffIdx;
    if(!pTspecInfo->valid)
    {
        HALLOGE( halLog(pMac, LOGE, FL("TSPEC not valid for handle %d\n"),
                   pTSParams->tspecIdx));
        return eSIR_HAL_TSPEC_INVALID;
    }

    // validate the staid
    if (pTSParams->staIdx >= pMac->hal.halMac.maxSta)
    {
        HALLOGE( halLog(pMac, LOGE, FL("Invalid staid 0x%x\n"),  pTSParams->staIdx ));
        return eSIR_HAL_STAID_INVALID;
    }

    aci = HAL_WMM_TID_TO_AC(HAL_WMM_DEFAULT_TID_AC_MAP, pTspecInfo->tsinfo.traffic.userPrio);

    // Invalidate the TSPEC
    pTspecInfo->valid = eANI_BOOLEAN_FALSE;

    HALLOGW( halLog(pMac, LOGW, FL("==Admission Control Registers before deleting TSPEC=====\n")));
    halWmmDumpACRegs(pMac);

    //Now disable the backoff for admission control. 
    bkOffIdx = __halMTU_ac2BkoffIndex(pMac, aci);    
    if(eHAL_STATUS_SUCCESS != halWmmEnableACBkOff(pMac, bkOffIdx, eSIR_FALSE))
    {
        return eSIR_FAILURE;
    }
    HALLOGW( halLog(pMac, LOGW, FL("==Admission Control Registers after adding TSPEC=====\n")));
    halWmmDumpACRegs(pMac);

    return eSIR_SUCCESS;
}


/*
 * Qualcomm Inc proprietary. All rights reserved.
 * This file contains Routines related with RF Kill feature.
 *
 * Author:      Dinesh Upadhyay 
 * Date:        08/3/07
 * History:-
 * Date            Modified by    Modification Information
 * --------------------------------------------------------------------
 *
 */

#include "halInterrupts.h"
#include "aniGlobal.h"
#include "halDebug.h"

/** -------------------------------------------------------------
\fn halIntEnableRadioOn
\brief This API is used to enable radio_on interrupt.
\param   tHalHandle hHalHandle
\return eHalStatus - status
  -------------------------------------------------------------*/

eHalStatus
halIntEnableRadioOn(tHalHandle hHalHandle)
{
    eHalStatus retStatus = eHAL_STATUS_SUCCESS;
    retStatus = halIntEnable(hHalHandle, eHAL_INT_MCU_SYS_GROUPED_1_RADIO_ON);
    if(HAL_STATUS_SUCCESS( retStatus))
    {
#if defined ANI_BUS_TYPE_PCI
        retStatus = halIntEnable(hHalHandle, eHAL_INT_PIF_PCI_MCU_SYS_GROUP_1);
#endif
    }
    if(HAL_STATUS_SUCCESS(retStatus))
        retStatus = halIntChipEnable(hHalHandle);
    return retStatus;
}

/** -------------------------------------------------------------
\fn halReadRadioSwitchOn
\brief read radio on/off register.
\param   tpAniSirGlobal pMac
\return  true -> radio switch is on; false ->radio switch is off
  -------------------------------------------------------------*/
static tANI_BOOLEAN halIsRadioSwitchOn(tpAniSirGlobal pMac)
{
    tANI_U32    regValue;
    eHalStatus  status = eHAL_STATUS_SUCCESS;
    tANI_BOOLEAN switchOn = eANI_BOOLEAN_FALSE;

    do
    {
        status = halReadRegister(pMac, MCU_RF_ON_OFF_CONTROL_REG, (tANI_U32 *)&regValue);
        if (status != eHAL_STATUS_SUCCESS)
        {
            break;
        }

        if ( (MCU_RF_ON_OFF_CONTROL_RD_ON_OFF_HWPIN_STAT_MASK & regValue) )
        {
            switchOn = eANI_BOOLEAN_TRUE;
        }

    } while (0);

    return( switchOn );
}

/** -------------------------------------------------------------
\fn halIntRadioSwitchHandler
\brief radio_on interrupt handler for radio_on at Init time.
\param   tHalHandle hHalHandle
\param   eHalIntSources timerIntr
\return eHalStatus - status
  -------------------------------------------------------------*/
static eHalStatus halIntRadioSwitchHandler( tHalHandle hHal, tHalIntRadioOnOff event )
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hHal;
    tSirMsgQ msg;    
    tSirMbMsg* mb = NULL;
    eHalStatus retCode = eHAL_STATUS_SUCCESS;

    halLog(pMac, LOGW, FL("called \n"));  

    do
    {
        if(HAL_INT_RADIO_ON == event)
        {
            retCode = halIntDisable(hHal, eHAL_INT_MCU_SYS_GROUPED_1_RADIO_ON);        
            if(eHAL_STATUS_SUCCESS != retCode)
            {
                  halLog(pMac, LOGP, FL("Could not disable interrupt for radio on\n"));            
                  break;
            }
            //radio on / off interrupts are level triggered and need to be disabled before we clear there status. 
            //otherwise pif_pci status does not get cleared and we keep looping for ever in halIntHandler.
            //halIntDisable/Enable routine sets the cache appropriately but it does not write to the register during interrupt
            //processing. So we need to call halIntWriteToEnableReg after any disable of these interrupts.
            retCode = halIntWriteCacheToEnableReg(hHal, eHAL_INT_MCU_SYS_GROUPED_1_RADIO_ON);            
            if(eHAL_STATUS_SUCCESS != retCode)
            {
                halLog(pMac, LOGP, FL("Could not write cache to the interrupt enable reg for radio on\n"));                  
                break;
            }
        }
        else
        {
            retCode = halIntDisable(hHal, eHAL_INT_MCU_SYS_GROUPED_1_RADIO_OFF);
            if(eHAL_STATUS_SUCCESS != retCode)
            {
                halLog(pMac, LOGP, FL("Could not disable interrupt for radio off\n"));
                break;
            }
            //radio on / off interrupts are level triggered and need to be disabled before we clear there status. 
            //otherwise pif_pci status does not get cleared and we keep looping for ever in halIntHandler.
            //halIntDisable/Enable routine sets the cache appropriately but it does not write to the register during interrupt
            //processing. So we need to call halIntWriteToEnableReg after any disable of these interrupts.
            retCode = halIntWriteCacheToEnableReg(hHal, eHAL_INT_MCU_SYS_GROUPED_1_RADIO_OFF);
            if(eHAL_STATUS_SUCCESS != retCode)
            {
                  halLog(pMac, LOGP, FL("Could not write cache to the interrupt enable reg for radio off\n"));                              
                  break;
            }
        }

        retCode = palAllocateMemory( pMac->hHdd, (void **)&mb, sizeof(tSirMbMsg));
        if( eHAL_STATUS_SUCCESS != retCode)
        {
            halLog(pMac, LOGP, FL("palAllocateMemory Failed!\n"));
            break;
        }
        mb->type = SIR_HAL_RADIO_ON_OFF_IND;
        mb->msgLen  = sizeof(tSirMbMsg);    // len in bytes
        mb->data[0] = event;
        msg.bodyptr = mb;
        msg.bodyval = 0;
        retCode = halMmhPostMsgApi(pMac, &msg, eHI_PRI);
        if(eSIR_SUCCESS != retCode)
        {
            palFreeMemory(pMac->hHdd, (void*)mb);
            halLog(pMac, LOGE, FL("halMmhPostMsgApiFailed!\n"));            
            break;
        }
        //we return from here in good case
        return retCode;
    } while(0);

    //we return from here only for failure case.
    return retCode;
}

 
/** -------------------------------------------------------------
\fn halIntMcuSysGroup1RadioOffHandler
\brief radio_off interrupt handler.
\param   tHalHandle hHalHandle
\param   eHalIntSources timerIntr
\return eHalStatus - status
  -------------------------------------------------------------*/
static eHalStatus halIntMcuSysGroup1RadioOffHandler( tHalHandle hHal, eHalIntSources timerIntr )
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hHal;
    //process the radio off interrupt when radio switch is off
    //otherwise just ignore and interrupt status will be cleared by the
    //generic interrupt handler routine.
    if(!(halIsRadioSwitchOn(pMac)))
        return halIntRadioSwitchHandler(hHal, HAL_INT_RADIO_OFF);
    else
        return eHAL_STATUS_SUCCESS;
}

/** -------------------------------------------------------------
\fn halIntMcuSysGroup1RadioOnHandler
\brief radio_on interrupt handler for radio_on
\param   tHalHandle hHalHandle
\param   eHalIntSources timerIntr
\return eHalStatus - status
  -------------------------------------------------------------*/
static eHalStatus halIntMcuSysGroup1RadioOnHandler( tHalHandle hHal, eHalIntSources timerIntr )
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hHal;
    //process the radio on interrupt when radio switch is on
    //otherwise just ignore and interrupt status will be cleared by the
    //generic interrupt handler routine.
    if(halIsRadioSwitchOn(pMac))
        return halIntRadioSwitchHandler(hHal, HAL_INT_RADIO_ON);
    else
        return eHAL_STATUS_SUCCESS;
}

/** -------------------------------------------------------------
\fn halIntRadioSwitchRegister
\brief enrolement of handler for radio_on, radio_off interrupts.
\param   tHalHandle hHal
\return eHalStatus - status
  -------------------------------------------------------------*/
eHalStatus halIntRadioSwitchRegister( tHalHandle hHal)
{
  tpAniSirGlobal pMac = (tpAniSirGlobal) hHal;

  /** Enroll Radio_off INTR handler
   */
  if( eHAL_STATUS_SUCCESS !=
      halIntEnrollHandler(eHAL_INT_MCU_SYS_GROUPED_1_RADIO_OFF, &halIntMcuSysGroup1RadioOffHandler))
      halLog( pMac, LOGW,
              FL("Unable to enroll an Radio_off INTR handler \n"));

  /** Enroll Radio_on INTR handlers
   */
  if( eHAL_STATUS_SUCCESS !=
      halIntEnrollHandler(eHAL_INT_MCU_SYS_GROUPED_1_RADIO_ON, &halIntMcuSysGroup1RadioOnHandler))
      halLog( pMac, LOGW,
              FL("Unable to enroll an INTR handler \n"));

  /** Enable radio_off interrupt at MCU level.*/
  if( eHAL_STATUS_SUCCESS != halIntEnable( hHal, eHAL_INT_MCU_SYS_GROUPED_1_RADIO_OFF ))
      halLog( pMac, LOGW,
              FL("Failed to ENABLE the Radio_off INTR!\n"));

  /** Enable radio_on interrupt at MCU level.*/
  if( eHAL_STATUS_SUCCESS != halIntEnable( hHal, eHAL_INT_MCU_SYS_GROUPED_1_RADIO_ON ))
      halLog( pMac, LOGW,
              FL("Failed to ENABLE the Radio_on INTR!\n"));

  /** Enable PIF top-level INTR... */
  if( eHAL_STATUS_SUCCESS != halIntEnable( hHal, eHAL_INT_PIF_PCI_MCU_SYS_GROUP_1))
      halLog( pMac, LOGW,
              FL("Failed to ENABLE the radio_on radio_off Group INTR!\n"));

  return eHAL_STATUS_SUCCESS;
}
  

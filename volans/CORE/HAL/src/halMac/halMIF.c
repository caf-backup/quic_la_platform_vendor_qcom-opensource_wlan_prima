/*
 * Copyright (c) 2011 Qualcomm Atheros, Inc. 
 * All Rights Reserved. 
 * Qualcomm Atheros Confidential and Proprietary. 
 * 
 * Copyright (C) 2006 Airgo Networks, Incorporated
 * 
 * halMIF.c:  Provides all the MAC APIs to the BMU Hardware Block.
 * Author:    Susan Tsao
 * Date:      03/13/2006
 *
 * --------------------------------------------------------------------------
 */

#include "halTypes.h"
#include "palTypes.h"
#ifdef WLAN_HAL_VOLANS
#include "volansDefs.h"
#else
#include "libraDefs.h"
#endif
#include "halMIF.h"
#include "aniGlobal.h"
#include "halDebug.h"


/*-----------------------------------------------------------------
 * FUNCTION: halEnableMifClkSustanance
 * 
 * The function configures MIF parameters for it to operate always
 * at 80 MHz clock
 * 
 *-----------------------------------------------------------------
 */
static
eHalStatus halEnableMIFClkSustanance(tHalHandle hHal)
{
    tANI_U32   uRegVal = 0;
	eHalStatus status;
#ifdef WLAN_DEBUG
    tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
#endif /* WLAN_DEBUG */
	   
    status = halReadRegister(hHal, QWLAN_MIF_MIF_ACC_REG, &uRegVal); 
   	if(eHAL_STATUS_SUCCESS != status)
   	{
        HALLOGE(halLog(pMac, LOGE, FL("Unable to read MIF ACC Register!\n")));
      	return (status);
   	}

   	/* No bits defined, hence using the raw bits */	
   	uRegVal |= 0x1E;

   	status = halWriteRegister(hHal, QWLAN_MIF_MIF_ACC_REG, uRegVal);
   	if(eHAL_STATUS_SUCCESS != status)
   	{
        HALLOGE(halLog(pMac, LOGE, FL("Unable to write MIF ACC Register!\n")));
    	return (status);
   	}

    /* 1 ms must be sufficient enough for the clock to stabilize, however, giving one
     * more milli second of grace time
     */
    vos_sleep(2);
  
   	return (VOS_STATUS_SUCCESS);
}

/* ---------------------------------
 * FUNCTION:  halMif_Init()
 *
 * ---------------------------------
 */
eHalStatus halMif_Start(tHalHandle hHal, void *arg)
{
    /**
     * FIXME:
     * Following change is needed in MIF_ACC to ensure that MIF always operates
     * at 80 MHz clock. It is anticipated in Volans that in some cases MIF may not
     * operate at 80 MHz and may cause read/write failures. Hence as a precautionary
     * measure the MIF_ACC register is being configured below. May have to be 
     * removed later is not required or the issue is fixed in later revisions of VOLANS
     *
     * This is one time change only since, MIF is in 1.3V AON domain. The configuration
     * is sustaining across power-save cycles.
     *
     * This particular location is chosen for configuration intentionally. This MIF
     * register has to be written before first access to MIF   
     **/
    return halEnableMIFClkSustanance(hHal);
}


/**
 * @brief  : This Routine is a Interrupt handler for MIF error Interrupts
 *           This issues a mac Reset if the error is FATAL.
 * @param  : hHalHandle - Mac Global Handle
 * @param  : intSource - Source for the paticular Interrupt.
 * @return : eHAL_STATUS_SUCCESS on Success and appropriate error sattus on error.
 * @note   : All MIF Error Interrupts are FATAL.
 */

eHalStatus halIntMIFErrorHandler(tHalHandle hHalHandle, eHalIntSources intSource)
{
    eHalStatus status;
    tANI_U32 intRegMask;
    tANI_U32 intRegStatus;
#ifdef WLAN_HAL_VOLANS
    tANI_U32 acpuInvalidAddr = 0, ahbInvalidAddr = 0;
#endif
    tpAniSirGlobal pMac = PMAC_STRUCT(hHalHandle);
    tANI_U32 uCodes[3];

    /** Read Interrupt Status.*/
    status = halIntGetErrorStatus(hHalHandle, intSource, &intRegStatus, &intRegMask);
    if (status != eHAL_STATUS_SUCCESS) {
        HALLOGE(halLog(pMac, LOGE, FL("Unable to read MIF Int Register Status!\n")));
        macSysResetReq(pMac, eSIR_MIF_EXCEPTION);                        
        return status;
    }

    /** Display Error Status Information.*/
    VOS_TRACE (VOS_MODULE_ID_HAL, VOS_TRACE_LEVEL_FATAL, "MIF Error Interrupt Status  : %x\nMIF Error Interrupt Mask/Enable  : %x\n",  intRegStatus, intRegMask );

#ifdef WLAN_HAL_VOLANS
    halReadRegister(hHalHandle, QWLAN_MIF_MIF_ACPU_INVALID_ADDR_REG, &acpuInvalidAddr);
    halReadRegister(hHalHandle, QWLAN_MIF_MIF_AHB_INVALID_ADDR_REG, &ahbInvalidAddr);    
    VOS_TRACE(VOS_MODULE_ID_HAL, VOS_TRACE_LEVEL_FATAL, "AHB invalid addr" 
                    ": %x, ACPU invalid addr = %x\n", ahbInvalidAddr, acpuInvalidAddr);
    halReadDeviceMemory(hHalHandle, QWLANFW_MEMMAP_UCODES_ADDR, &uCodes, 3*sizeof(tANI_U32));
    VOS_TRACE(VOS_MODULE_ID_HAL, VOS_TRACE_LEVEL_FATAL, "uCodes[0]: %x uCodes[1]: %x "
                    "uCodes[2]: %x\n", uCodes[0], uCodes[1], uCodes[2]);    
#endif

    /** Fatal Issue mac Reset.*/
    macSysResetReq(pMac, eSIR_MIF_EXCEPTION);                          

    return (eHAL_STATUS_SUCCESS);
}




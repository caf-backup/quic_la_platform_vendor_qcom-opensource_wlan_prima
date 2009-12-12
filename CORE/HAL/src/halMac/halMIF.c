/*
 * Airgo Networks, Inc proprietary. All rights reserved.
 * halMIF.c:  Provides all the MAC APIs to the BMU Hardware Block.
 * Author:    Susan Tsao
 * Date:      03/13/2006
 *
 * --------------------------------------------------------------------------
 */

#include "halTypes.h"
#include "palTypes.h"
#include "libraDefs.h"
#include "halMIF.h"
#include "aniGlobal.h"
#include "halDebug.h"


/* ---------------------------------
 * FUNCTION:  halMif_Init()
 *
 * ---------------------------------
 */
eHalStatus halMif_Start(tHalHandle hHal, void *arg)
{
    return eHAL_STATUS_SUCCESS;
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
    tpAniSirGlobal pMac = PMAC_STRUCT(hHalHandle);

    /** Read Interrupt Status.*/
    status = halIntGetErrorStatus(hHalHandle, intSource, &intRegStatus, &intRegMask);
    if (status != eHAL_STATUS_SUCCESS) {
        HALLOGE(halLog(pMac, LOGE, FL("Unable to read MIF Int Register Status!\n")));
        macSysResetReq(pMac, eSIR_MIF_EXCEPTION);                        
        return status;
    }

    /** Display Error Status Information.*/
    HALLOGE( halLog( pMac, LOGE, FL("MIF Error Interrupt Status  : %x\n"),  intRegStatus ));
    HALLOGE( halLog( pMac, LOGE, FL("MIF Error Interrupt Mask/Enable  : %x\n"),  intRegMask ));

    /** Fatal Issue mac Reset.*/
    macSysResetReq(pMac, eSIR_MIF_EXCEPTION);                          

    return (eHAL_STATUS_SUCCESS);
}




/*
 * Copyright (c) 2011 Qualcomm Atheros, Inc. 
 * All Rights Reserved. 
 * Qualcomm Atheros Confidential and Proprietary. 
 * 
 * Copyright (C) 2006 Airgo Networks, Incorporated
 * 
 * halMCPU.c:  Provides all the MAC APIs to the MCPU Block.
 * Author:    Naveen G
 * Date:      04/26/2006
 *
 * --------------------------------------------------------------------------
 */

#include "palTypes.h"
#include "halMCPU.h"
#include "halDebug.h"

eHalStatus halIntMCPUErrorHandler(tHalHandle hHalHandle, eHalIntSources intSource)
{
    tANI_U32 intRegMask;
    tANI_U32 intRegStatus;
    tpAniSirGlobal pMac = PMAC_STRUCT(hHalHandle);

    /** Read Interrupt Status.*/
    halIntGetErrorStatus(hHalHandle, intSource, &intRegStatus, &intRegMask);

    /** Display Error Status Information.*/
    HALLOGE( halLog( pMac, LOGE, FL("MCPU Error Interrupt Status      : %x\n"),  intRegStatus ));
    HALLOGE( halLog( pMac, LOGE, FL("MCPU Error Interrupt Mask/Enable : %x\n"),  intRegMask ));

    /** Fatal Issue mac reset.*/
    HALLOGE(halLog(pMac, LOGE, FL("MCPU Fatal Error!\n")));
    macSysResetReq(pMac, eSIR_MCPU_EXCEPTION);            
    return (eHAL_STATUS_SUCCESS);
}

/** ------------------------------------------------------------------------- * 
    ------------------------------------------------------------------------- *  

  
    \file asic.c
  
    \brief contains other phy layer asic module functions
  
    $Id$ 
  
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

//file can be deleted for Volans

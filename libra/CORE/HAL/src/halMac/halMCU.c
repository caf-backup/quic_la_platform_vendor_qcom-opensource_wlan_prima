/*
 * Airgo Networks, Inc proprietary. All rights reserved.
 * halMCU.c:  Provides all the MAC APIs to the MCU Hardware Block.
 * Author:    Satish Gune
 * Date:      02/09/2006
 *
 * --------------------------------------------------------------------------
 */

#include "halTypes.h"
#include "palTypes.h"
#include "aniGlobal.h"
#include "libraDefs.h"
#include "halMCU.h"
#include "halDebug.h"
#include "halAdu.h"

#define  MCU_MAX_NUM_OF_MAILBOX             4
#define  MCU_MAX_NUM_OF_MUTEX               8
#define  MCU_MAILBOX_CONTROL_REG_ADDR(n)    (QWLAN_MCU_MB0_CONTROL_REG + (n * 8))
#define  MCU_MUTEX_REG_ADDR(n)              (QWLAN_MCU_MUTEX0_REG + (n * 4))


/* -------------------------------------------------
 * FUNCTION:  halMcu_Start()
 *
 * NOTE:
 *  -Initialization of BD/PDU base address will be set
 *   as part of the BMU initialization.
 *  -Reset all 16 mailboxes
 *  -Reset all 8 mutexes
 * --------------------------------------------------
 */
eHalStatus halMcu_Start(tHalHandle hHal, void *arg)
{
    tANI_U8    i;
    tpAniSirGlobal pMac = (tpAniSirGlobal)hHal;

    // By default, MLC loopback is disabled.
    // For Loopback testing, enable MLC loopback and set
    // byte_forwarding_interval to 0x33 as Guido suggested
    //if (halWriteRegister(pMac, MCU_MLC_CONTROL_REG, 0x80330000) != eHAL_STATUS_SUCCESS)
    //    return eHAL_STATUS_FAILURE;

    (void) arg;

    /* Reset all mailbox by setting the reset bit to 1.
     * This parameters, counters related to this Mailbox will be cleared.
     */
    for (i = 0; i < MCU_MAX_NUM_OF_MAILBOX; i++)
    {
        halWriteRegister(pMac,
                             MCU_MAILBOX_CONTROL_REG_ADDR(i),
                             QWLAN_MCU_MB0_CONTROL_MB_RESET_MASK) ;
    }

    /* Reset all mutex */
    for (i = 0; i < MCU_MAX_NUM_OF_MUTEX; i++)
    {
        halWriteRegister(pMac,
                         MCU_MUTEX_REG_ADDR(i),
                         QWLAN_MCU_MUTEX0_RESET_MASK);
    }

    // Temporary workaround for the PCI hang issue
    // if (halWriteRegister(pMac, 0x0e028ccc, 0x00FF00FF) != eHAL_STATUS_SUCCESS)
    //    return eHAL_STATUS_FAILURE;

    return eHAL_STATUS_SUCCESS;
}

/* -------------------------------------------------
 * FUNCTION:  halMcu_ResetMutexCount()
 *
 * NOTE:
 *  -This function resets the HW mutex and initializes the count. 
 * args
 *      pMac    : Mac parameter structure pointer.
 *      mutexId : Which HW mutex to use.
 *      count   : Count to initialize the mutex for (couting sempahore)
 * --------------------------------------------------
 */
void halMcu_ResetMutexCount(tpAniSirGlobal pMac, tANI_U8 mutexIdx, tANI_U8 count)
{
    tANI_U32 regValue = 0;

    regValue = ((1 << QWLAN_MCU_MUTEX0_RESET_OFFSET) | (count << QWLAN_MCU_MUTEX0_MAXCOUNT_OFFSET));
    halWriteRegister(pMac, MCU_MUTEX_REG_ADDR(mutexIdx), regValue);

    return;
}

/* -------------------------------------------------
 * FUNCTION:  halMcu_ResetModules()
 *
 * NOTE:
 *  -This function resets one or more MCU modules. 
 * args
 *      pMac  : Mac parameter structure pointer.
 *      bits    : this is bitmask for the module/modules to be reset derived from the bit assignment
 *                  in taurus.h. 
 * --------------------------------------------------
 */

eHalStatus halMcu_ResetModules(tpAniSirGlobal pMac, tANI_U32 bits)
{
    tANI_U32 regValOld = 0, regValNew = 0;
    //reading the old register value and saving it for restoration at the end.
    halReadRegister(pMac,
          QWLAN_MCU_SOFT_RESET_REG, 
          &regValOld);        

    //modifying only the bits that we are interested in.
    halWriteRegister(pMac,  QWLAN_MCU_SOFT_RESET_REG,  (bits | regValOld) );

    //reading the register to add some delay to make sure reset is over before we restore the register back.
    halReadRegister(pMac, QWLAN_MCU_SOFT_RESET_REG, &regValNew);
    //restore the register value back.
    halWriteRegister(pMac,  QWLAN_MCU_SOFT_RESET_REG,  regValOld );

    return eHAL_STATUS_SUCCESS;
}

eHalStatus halMcu_CBRErrorInterruptHandler(tHalHandle hHalHandle, eHalIntSources intSource)

{
    eHalStatus status;
    tANI_U32 intRegMask;
    tANI_U32 intRegStatus;
    tpAniSirGlobal pMac = PMAC_STRUCT(hHalHandle);
    
    /** Read Interrupt Status.*/
    status = halIntGetErrorStatus(hHalHandle, intSource, &intRegStatus, &intRegMask);
    if (status != eHAL_STATUS_SUCCESS) {
        return status;
    }

    /** Display Error Status Information.*/
    HALLOGE( halLog( pMac, LOGE, FL("CBR/DBR Error Interrupt Status      : %x\n"),  intRegStatus ));
    HALLOGE( halLog( pMac, LOGE, FL("CBR/DBR Error Interrupt Mask/Enable : %x\n"),  intRegMask ));

    /** Fatal Issue mac reset.*/
    macSysResetReq(pMac, eSIR_MCU_EXCEPTION);            

    return (eHAL_STATUS_SUCCESS);
}


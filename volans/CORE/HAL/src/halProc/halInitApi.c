/**
 *
 *  @file:         halInitApi.c
 *
 *  @brief:        Provide HAL APIs.
 *
 *  @author:       Susan Tsao
 *
 *  Copyright (C) 2008, Qualcomm, Inc. All rights reserved.
 *
 *  Change History:
 * ---------------------------------------
 * 02/06/2006  File created.
 */

#include "halInternal.h"
#include "halHddApis.h"
#include "halDebug.h"
#include "halMTU.h"
#include "halRxp.h"
#include "halRpe.h"
#include "halTpe.h"
#include "halTxp.h"
#include "halAdu.h"
#include "halPhyApi.h"
#include "halRFKill.h"
#include "halMacStats.h"
#include "halCommonApi.h"   // halCleanup
#include "cfgApi.h"         // cfgCleanup
#include "halTLApi.h"   //halTLApInit() and halTLApiExit()
#include "halAHB.h"
#include "vos_power.h"
#include "halFtmRx.h"


#ifdef ANI_OS_TYPE_LINUX
#include "ccmApi.h"
#endif // ANI_OS_TYPE_LINUX

#include "macInitApi.h"
#include "halLED.h"
#ifdef ANI_LOGDUMP
#include "halLogDump.h"
#endif

#include "halPwrSave.h"
#include "halFwApi.h"
#include "halRegBckup.h"
#include "vos_api.h"

/* --------------------------------------------------------------------------
 * local types and defs
 */
typedef eHalStatus (*pFptr)(void *, void *);

#define NUM_FUNCS 4
#define OPEN_IDX  0     // fptr index for open
#define START_IDX 1     // fptr index for start
#define STOP_IDX  2     // fptr index for stop
#define CLOSE_IDX 3     // fptr index for close

#define PMU_PWR_UP_WAIT_PER_ITERATION    500000 // 500 uS
#define PMU_PWR_UP_LOOP_MAX    200    // Amounts to a total wait for 20 ms

#define MCPU_RST_WAIT_PER_ITERATION    500000 // 500 uS
#define PCIE_SOFT_RST_WAIT    500000 // 500 uS
#define USB_SOFT_RST_WAIT    1000 // 1 uS

/* generic entry for each module
 * each hal module supports four functions: open, start, stop and close
 */
typedef struct sFuncEntry {
    pFptr       pFunc[NUM_FUNCS];
    char        name[20];
} tFuncEntry, *tpFuncEntry;

#ifdef WLAN_DEBUG
static char *opName[NUM_FUNCS] = {
    "open",
    "start",
    "stop",
    "close"
};
#endif


/* --------------------------------------------------------------------------
 * local protos
 */
static eHalStatus nvOpen(tHalHandle hHal, void *arg);
static eHalStatus nvClose(tHalHandle hHal, void *arg);
static eHalStatus phyOpen(tHalHandle hHal, void *arg);
static eHalStatus phyStart(tHalHandle hHal, void *arg);
static eHalStatus phyStop(tHalHandle hHal, void *arg);
static eHalStatus phyClose(tHalHandle hHal, void *arg);
#if defined(ANI_BUS_TYPE_PCI)
static eHalStatus hal_pci_chip_pre_init(tHalHandle hHal, void *arg);
static eHalStatus halPciResetChip(tHalHandle hHal, void *arg);
#define halResetChip    halPciResetChip
#elif defined(ANI_BUS_TYPE_USB)
static eHalStatus hal_usb_chip_pre_init(tHalHandle hHal, void *arg);
static eHalStatus halUsbResetChip(tHalHandle hHal, void *arg);
#define halResetChip    halUsbResetChip
#elif defined(ANI_BUS_TYPE_SDIO)
static eHalStatus halSdioResetChip(tHalHandle hHal, void *arg);
#define halResetChip    halSdioResetChip
#endif
static eHalStatus halSaveDeviceInfo(tHalHandle hHal, void *arg);
static void halOpenInit(tpAniSirGlobal pMac);
static void halCloseExit(tpAniSirGlobal pMac);
/* --------------------------------------------------------------------------
 * FIXME/TODO
 */
// FIXME  - These should come from header files.
extern tSirRetStatus sysInitGlobals(tpAniSirGlobal pMac);
extern tSirRetStatus logInit (tpAniSirGlobal);
////

/* --------------------------------------------------------------------------
 * Local (RO) vars
 */
/* table of function pointers for each module. Each of these will be called
 * during the corresponding halOpen/Start/Stop/Close functions.
 * If a module does not implement the corresponding function, use NULL
 */
static tFuncEntry funcTable[] = {
    // func pointer(open)  start                        stop                    close                    name (<20chars)
    { {halResetChip,     halResetChip,                  halResetChip,           halResetChip},           "ResetChip"        },
    { {halMemoryMap_Open,halMemoryMap_Start,            NULL,                   NULL},                   "MemMap"           },
    { {NULL,             halSaveDeviceInfo,             NULL,                   NULL},                   "FillChipInfo"     },
    { {halRegBckup_Open, halRegBckup_Start,             NULL,                   NULL},                   "RegisterBackup"   },
    { {NULL,             halPMU_Start,                  NULL,                   NULL},                   "PMU"              },
    { {NULL,             halMif_Start,                  NULL,                   NULL},                   "MIF"              },
    { {nvOpen,           NULL,                          NULL,                   nvClose},                "halNv"            },
    { {phyOpen,          phyStart,                      phyStop,                phyClose},               "halPhy"           },
    { {NULL,             halMcu_Start,                  NULL,                   NULL},                   "MCU"              },
    { {NULL,             halMTU_Start,                  NULL,                   NULL},                   "MTU"              },
    { {halMbox_Open,     halMbox_Start,                 halMbox_Stop,           halMbox_Close},          "Mailbox"          },
    { {NULL,             halFW_Init,                    halFW_Exit,             NULL},                   "Firmware"         },
    { {NULL,             halAhb_Start,                  NULL,                   NULL},                   "AHB"              },
    { {NULL,             halBmu_Start,                  NULL,                   NULL},                   "BMU"              },
    { {NULL,             halRate_Start,                 NULL,                   NULL},                   "RateTable Init"   },
    { {NULL,             halTpe_Start,                  NULL,                   NULL},                   "TPE"              },
    { {NULL,             halTpe_InitHwTemplates,        NULL,                   NULL},                   "TPETemplates"     },
    { {NULL,             halRpe_Start,                  NULL,                   NULL},                   "RPE"              },
    { {NULL,             halAdu_Start,                  NULL,                   NULL},                   "ADU"              },
    { {halDpu_Open,      halDpu_Start,                  halDpu_Stop,            halDpu_Close},           "DPU"              },
    { {halRxp_Open,      halRxp_Start,                  halRxp_Stop,            halRxp_Close},           "RXP"              },
    { {NULL,             halTxp_Start,                  NULL,                   NULL},                   "TXP"              },
    { {halTable_Open,    halTable_Start,                halTable_Stop,          halTable_Close},         "Tables"           },
#if defined(ANI_OS_TYPE_LINUX)
    { {halTable_StaCacheOpen, halTable_StaCacheStart,   halTable_StaCacheStop,  halTable_StaCacheClose}, "StaCache Tables"  },
#endif
    { {NULL,             halPS_Init,                    halPS_Exit,             NULL},                   "PowerSaveModule"  },
    { {NULL,             halMacRaStart,                 halMacRaStop,           NULL},                   "Rate-Adaptation"  },
    { {NULL,             halFW_CheckInitComplete,       NULL,                   NULL},                   "Check FW Init"    },
    { {halDXE_Open,      halDXE_Start,                  halDXE_Stop,            halDXE_Close},           "halDxe"           },
    { {NULL,             (pFptr)halTLApiInit,           (pFptr)halTLApiExit,           NULL},                   "HAL TL API"       },
    { {halMacStats_Open, NULL,                          NULL,                   halMacStats_Close},      "Mac Stats"        },
};


//eDRIVER_TYPE_MFG
#ifndef WLAN_FTM_STUB
//This alternate table is to
static tFuncEntry funcFtmTable[] = {
    // func pointer(open)  start                        stop                    close                    name (<20chars)
    { {halResetChip,      halResetChip,                  halResetChip,           halResetChip},           "ResetChip"},
    { {NULL,              halSaveDeviceInfo,             NULL,                   NULL},                   "FillChipInfo"},
    { {NULL,              halPMU_Start,                  NULL,                   NULL},                   "PMU"              },
    { {nvOpen,            NULL,                          NULL,                   nvClose},                "halNv"           },
    { {phyOpen,           phyStart,                      phyStop,                phyClose},               "halPhy"          },
    { {NULL,              halMif_Start,                  NULL,                   NULL},                   "MIF"     },
    { {NULL,              halMcu_Start,                  NULL,                   NULL},                   "MCU"             },
    //{ {NULL,              enAllInts,                     NULL,                   NULL},                   "EnAllInts"     },
    { {halMbox_Open,      halMbox_Start,                 halMbox_Stop,           halMbox_Close},          "Mailbox"             },
    { {NULL,              NULL,                          NULL,                   nvClose},                "halNv"           },
    { {NULL,              halFW_Init,                    halFW_Exit,             NULL},                   "Firmware"            },
    { {halRxp_Open,       halRxp_Start,                  halRxp_Stop,            halRxp_Close},           "RXP"             },
    { {NULL,              halFtmRx_Start,                NULL,                   NULL},                   "Add1_filter"     },
    { {NULL,              halFW_CheckInitComplete,       NULL,                   NULL},                   "Check FW Init"   },

};
#endif

static eHalStatus halSdioResetChip(tHalHandle hHal, void *arg)
{
#if 0
    tpAniSirGlobal  pMac = (tpAniSirGlobal) hHal;

      tANI_U32        value1;
      tANI_U32        value, sifResetLoopCount = 0;
      (void)          arg;

      // Reset the WLAN CSR access engine in SIF
      if ( halReadRegister(pMac, QWLAN_SIF_SIF_TOP_CONTROL_REG, &value) != eHAL_STATUS_SUCCESS)
                return eHAL_STATUS_FAILURE;

      value |= QWLAN_SIF_SIF_TOP_CONTROL_CSR_ACC_FIFO_MGNT_RESET_MASK;

      if ( halWriteRegister(pMac, QWLAN_SIF_SIF_TOP_CONTROL_REG, value) != eHAL_STATUS_SUCCESS)
                return eHAL_STATUS_FAILURE;

      HALLOGW( halLog(pMac, LOGW, FL("%s: waiting for SIF CSR engine reset.\n"),  TEXT(__FUNCTION__) ));

      while(sifResetLoopCount++ < PMU_PWR_UP_LOOP_MAX)
      {
          if ( halReadRegister(pMac, QWLAN_SIF_SIF_TOP_CONTROL_REG, &value) != eHAL_STATUS_SUCCESS)
                return eHAL_STATUS_FAILURE;

          if(!(value & QWLAN_SIF_SIF_TOP_CONTROL_CSR_ACC_FIFO_MGNT_RESET_MASK))
          {
              break;
          }
          else
          {
              //wait 500us
              sirBusyWait(PMU_PWR_UP_WAIT_PER_ITERATION);
          }
      }

      HALLOGW( halLog(pMac, LOGW, FL("%s: Done waiting for SIF CSR engine reset.\n"),  TEXT(__FUNCTION__) ));

#ifndef ANI_SYS_ONLY_FPGA
      //Delete all RXP BST entries before system reset.
      halRxp_DelAllEntries(pMac);
#endif // #ifndef ANI_SYS_ONLY_FPGA

      if ( halReadRegister(pMac, QWLAN_SIF_SIF_TOP_CONTROL_REG, &value) != eHAL_STATUS_SUCCESS)
          return eHAL_STATUS_FAILURE;

      value |= (QWLAN_SIF_SIF_TOP_CONTROL_RX_FIFO_MGNT_SOFT_RESET_MASK |
                QWLAN_SIF_SIF_TOP_CONTROL_TX_FIFO_MGNT_SOFT_RESET_MASK);

      if ( halWriteRegister(pMac, QWLAN_SIF_SIF_TOP_CONTROL_REG, value) != eHAL_STATUS_SUCCESS)
              return eHAL_STATUS_FAILURE;

      HALLOGW( halLog(pMac, LOGW, FL("%s: waiting for SIF Tx/Rx engine reset.\n"),  TEXT(__FUNCTION__) ));

      while(sifResetLoopCount++ < PMU_PWR_UP_LOOP_MAX)
      {
          if ( halReadRegister(pMac, QWLAN_SIF_SIF_TOP_CONTROL_REG, &value) != eHAL_STATUS_SUCCESS)
              return eHAL_STATUS_FAILURE;

          if(!(value & (QWLAN_SIF_SIF_TOP_CONTROL_RX_FIFO_MGNT_SOFT_RESET_MASK |
                        QWLAN_SIF_SIF_TOP_CONTROL_TX_FIFO_MGNT_SOFT_RESET_MASK)))
          {
              break;
          }
          else
          {
              //wait 500us
              sirBusyWait(PMU_PWR_UP_WAIT_PER_ITERATION);
          }
      }

      HALLOGW( halLog(pMac, LOGW, FL("%s: Done waiting for SIF Tx/Rx engine reset.\n"),  TEXT(__FUNCTION__) ));

#ifdef FIXME_GEN6
    if ( palSdioIOReset(pMac->hHdd) != eHAL_STATUS_SUCCESS)
      {
              return eHAL_STATUS_FAILURE;
      }
#endif
#endif

        return eHAL_STATUS_SUCCESS;
}

static eHalStatus
runModuleFunc (
    tHalHandle  hHal,
    void       *pArg1,
    tANI_U8     fIndex)
{
    tpFuncEntry     pEntry;
    eHalStatus      status = eHAL_STATUS_SUCCESS;
    tpAniSirGlobal  pMac = (tpAniSirGlobal) hHal;
    tANI_U8 i;
    tANI_U8 numEntries;

    if (fIndex >= NUM_FUNCS)
        return eHAL_STATUS_INVALID_PARAMETER;

#ifndef WLAN_FTM_STUB
    if(pMac->gDriverType == eDRIVER_TYPE_MFG)
    {
        pEntry = &funcFtmTable[0];
        numEntries = sizeof(funcFtmTable)/sizeof(funcFtmTable[0]);
    }
    else
#endif
    {
    pEntry = &funcTable[0];
        numEntries = sizeof(funcTable)/sizeof(funcTable[0]);
    }

    for (i = 0; i < numEntries; i++, pEntry++)
    {
        if (pEntry->pFunc[fIndex] == NULL) continue;

        status = (* ((pFptr) pEntry->pFunc[fIndex]))(pMac, pArg1);

        if (status != eHAL_STATUS_SUCCESS)
        {
            HALLOGE( halLog(pMac, LOGE, FL("%s %s failed (status 0x%x)\n"),
                   pEntry->name, opName[fIndex], status));
            //We cannot quit when stopping or closing because all routines need to run
            if((fIndex != STOP_IDX) && (fIndex != CLOSE_IDX))
            {
                break;
            }
        }
        else
            HALLOGW( halLog(pMac, LOGW, FL("%s %s  OK\n"), pEntry->name, opName[fIndex]));
    }

    return status;
}

/** -------------------------------------------------------------
\fn halSaveDeviceInfo
\brief to save device related information at HAL.
\       right now it reads hw register to get chip revision and
\       sets HAL global.
\param   tHalHandle hHal
\param   void *arg
\return  status code
  -------------------------------------------------------------*/
static
eHalStatus halSaveDeviceInfo(tHalHandle hHal, void *arg)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal) hHal;
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tANI_U32 revNum = 0;

    halReadRegister(pMac, QWLAN_SIF_SIF_CHIP_REV_ID_REG_REG, &revNum);
    revNum = ((revNum & QWLAN_SIF_SIF_CHIP_REV_ID_REG_CHIP_VERSION_MASK) >> QWLAN_SIF_SIF_CHIP_REV_ID_REG_CHIP_VERSION_OFFSET);

    //storing the rev num in HAL global field.
        pMac->hal.chipRevNum = revNum;

    halReadRegister(pMac, QWLAN_RFAPB_REV_ID_REG, &revNum);

    //storing the rf rev id.
    pMac->hphy.rf.revId = revNum;

    //FIXME need to fill in right card type when that gets defined afer removing the eeprom files.
    //          Also need to remove the eeprom code below once we decide how we define the same for Libra.
    pMac->hal.cardType = 0;

    return status;
}

/** -------------------------------------------------------------
\fn halOpen
\brief this function is called from macOpen in the init process. This function will open
\       all the HAL modules.
\param   tpAniSirGlobal pMac
\param   tHalHandle* pHalHandle
\param   tHddHandle hHdd
\param   tHalOpenParameters pHalOpenParms
\return   eHalStatus
  -------------------------------------------------------------*/

eHalStatus halOpen(tpAniSirGlobal pMac, tHalHandle *pHalHandle, tHddHandle hHdd, tMacOpenParameters *pMacOpenParms)
{
    eHalStatus halStatus = eHAL_STATUS_SUCCESS;

    // Initialize HAL before performing open on all the HAL modules
    halOpenInit(pMac);

    /* Perform open on all HAL modules */
    halStatus = runModuleFunc(*pHalHandle, (void *) pMacOpenParms, OPEN_IDX);
    if (halStatus != eHAL_STATUS_SUCCESS)
        return halStatus;

   /* set the state */
    // FIXME
    //halStateSet(pMac, eHAL_INIT);

    return halStatus;
}

/**
 * \brief Close the HAL & free allocated resources
 *
 * \sa halOpen
 *
 *
 * \param hHal Handle returned from halOpen
 *
 * \return A member of the eHalStatus enumeration
 *
 *
 * This method \em should be safe to call even if halOpen failed and
 * we're partially or completely un-initialized.
 *
 */

eHalStatus halClose( tHalHandle hHal )
{
    tpAniSirGlobal pMac = ( tpAniSirGlobal ) hHal;
    eHalStatus     status, nReturn = eHAL_STATUS_SUCCESS;

    // Make sure that this instance of the adapter was actually opened
    // earlier:
    if ( NULL == pMac ) return eHAL_STATUS_NOT_OPEN;

    // Call the close routines for all modules (LIM, DPU, &c):
    status = runModuleFunc( hHal, NULL, CLOSE_IDX );
    if ( ! HAL_STATUS_SUCCESS( status ) ) nReturn = status;

    // Final exit operation on HAL
    halCloseExit(pMac);

    return nReturn;

} // End halClose.

/** -------------------------------------------------------------
\fn halStart
\brief this function is called from macStart in the start sequence. There will not be
\        any memory allocation with global context here.
\param   tHalHandle hHal
\param   tHalMacStartParameters* pHalMacStartParms
\return    eHalStatus
  -------------------------------------------------------------*/

eHalStatus
halStart(
    tHalHandle              hHal,
    tHalMacStartParameters *pHalMacStartParms)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal) hHal;
    eHalStatus status;

    /* Get the device Card ID here. Required to perform device specific configurations */
    WLANBAL_GetSDIOCardIdentifier(pMac->pAdapter, &pMac->hal.deviceCardId);

    pMac->hal.halMac.fShortSlot = 1; //initializing as short slot enabled.
    status = runModuleFunc(hHal, (void *) pHalMacStartParms, START_IDX);
    if (status != eHAL_STATUS_SUCCESS)
        return status;

    /** Enable all interrupt services.*/
    halIntDefaultRegServicesEnable(hHal, eANI_BOOLEAN_TRUE);

    //halPhyStart leaves all receive packet types disabled
    // now enable all receive packet types
    status = halPhySetRxPktsDisabled(pMac, PHY_RX_DISABLE_NONE);
    if (status != eHAL_STATUS_SUCCESS)
    {
        return (status);
    }
#ifdef ANI_LOGDUMP
    halDumpInit(pMac);
#endif

    // Set the HAL state to INIT
    halStateSet(pMac, eHAL_INIT);

#ifndef WLAN_FTM_STUB
    if(pMac->gDriverType == eDRIVER_TYPE_MFG)
    {
        if(eHAL_STATUS_SUCCESS != halIntChipEnable((tHalHandle)pMac))
        {
            HALLOGP( halLog(pMac, LOGP, FL("halIntChipEnable failed\n")));
        }
        else
        {
            //set the halState to ready
            halStateSet(pMac, eHAL_SYS_READY);
        }
    }
#endif

     return eHAL_STATUS_SUCCESS;
}

/** -------------------------------------------------------------
\fn halStop
\brief this function is called from macStop in the stop context. This is reverse of
\       of halStart. this function will bring the hal modules to the same state after halOpen
\param   tHalHandle hHal
\return    eHalStatus
  -------------------------------------------------------------*/

eHalStatus halStop( tHalHandle hHal , tHalStopType stopType )
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hHal;
    eHalStatus     status = eHAL_STATUS_SUCCESS, nReturn = eHAL_STATUS_SUCCESS;

    if ( NULL == hHal ) return eHAL_STATUS_NOT_OPEN;

    // Disable interrupts from SIF
    if (!vos_is_logp_in_progress(VOS_MODULE_ID_HAL, NULL)) 
    {
        status = halIntChipDisable( hHal );

        if ( ! HAL_STATUS_SUCCESS( status ) )
            nReturn = status;
    }

    /** Disable all default interrupt services.*/
    halIntDefaultRegServicesEnable(hHal, eANI_BOOLEAN_FALSE);

    status = halIntReset( hHal );
    if ( ! HAL_STATUS_SUCCESS( status ) ) nReturn = status;

    // Note: We CANNOT quit cleanup even if there are error returns for
    // earlier functions
    status = runModuleFunc( hHal, NULL, STOP_IDX );
    if ( ! HAL_STATUS_SUCCESS( status ) ) nReturn = status;

#if defined(ANI_LED_ENABLE)
    halCloseLed(pMac);
#endif

    if (!vos_is_logp_in_progress(VOS_MODULE_ID_HAL, NULL)) 
    {
        halPS_ExecuteStandbyProcedure(pMac);
    }

    halCleanup( pMac );
    return nReturn;
}

/** -------------------------------------------------------------
\fn halReset
\brief this function is called from macReset. this function sends message to HDD
\       to start reset.
\param   tpAniSirGlobal pMac
\param   tANI_U32 rc
\return    eHalStatus
  -------------------------------------------------------------*/

eHalStatus halReset(tHalHandle hHal, tANI_U32 rc)
{
   tANI_U32 resetReason;

   switch(rc)
   {
      case eSIR_MIF_EXCEPTION:
         resetReason = VOS_CHIP_RESET_MIF_EXCEPTION;
         break;
      case eSIR_FW_EXCEPTION:
         resetReason = VOS_CHIP_RESET_FW_EXCEPTION;
         break;
      case eSIR_PS_MUTEX_READ_EXCEPTION:
         resetReason = VOS_CHIP_RESET_MUTEX_READ_FAILURE;
         break;
      default:
         resetReason =  VOS_CHIP_RESET_UNKNOWN_EXCEPTION;
         break;
   }

   vos_chipReset(NULL, VOS_FALSE, NULL, NULL, resetReason);

   return eHAL_STATUS_SUCCESS;
}

/*
 *
 */
tHddHandle halHandle2HddHandle(tHalHandle hHal)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal) hHal;

    if (!pMac) {
        return 0;
    }
    return pMac->hHdd;
}

/*
 * Hal register read function.
 */
eHalStatus halReadRegister(tHalHandle hHal, tANI_U32 regOffset, tANI_U32 *pRegValue)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hHal;
    return pMac->hal.funcReadReg(hHal, regOffset, pRegValue);
}


/*
 * DESCRIPTION:
 *      API Function to write into the HW register. This function calls uses the
 *      function pointer to write into the register
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *      regAddr: Address of the register
 *      regValue: Value to be written into the register
 *      pRecordAddr: Pointer to the location where these Reg addr& value has to saved
 *
 * RETURN:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */
eHalStatus halWriteRegister(tHalHandle hHal, tANI_U32 regAddr, tANI_U32 regValue)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hHal;
    return pMac->hal.funcWriteReg(hHal, regAddr, regValue);
}

eHalStatus halWriteDeviceMemory( tHalHandle hHal, tANI_U32 dstOffset, void *pSrcBuffer, tANI_U32 numBytes )
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hHal;
    return pMac->hal.funcWriteMem(hHal, dstOffset, (tANI_U8 *)pSrcBuffer, numBytes);
}

eHalStatus halReadDeviceMemory( tHalHandle hHal, tANI_U32 srcOffset, void *pBuffer, tANI_U32 numBytes )
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hHal;
    return pMac->hal.funcReadMem(hHal, srcOffset, pBuffer, numBytes);
}

eHalStatus halFillDeviceMemory( tHalHandle hHal, tANI_U32 memOffset, tANI_U32 numBytes, tANI_BYTE fillValue )
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hHal;
    return (palFillDeviceMemory( pMac->hHdd, memOffset, numBytes, fillValue ));
}

eHalStatus halZeroDeviceMemory( tHalHandle hHal, tANI_U32 memOffset, tANI_U32 numBytes )
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hHal;
    return (palFillDeviceMemory( pMac->hHdd, memOffset, numBytes, 0 ));
}

eHalStatus halNormalWriteRegister( tHalHandle hHal, tANI_U32 regAddr, tANI_U32 regValue)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hHal;
    return( palWriteRegister( pMac->hHdd, regAddr, regValue ) );
}

eHalStatus halNormalReadRegister( tHalHandle hHal, tANI_U32 regAddr, tANI_U32 *pRegValue)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hHal;
    return( palReadRegister( pMac->hHdd, regAddr, pRegValue ) );
}

eHalStatus halNormalWriteMemory(tHalHandle hHal, tANI_U32 dstOffset, void *pSrcBuffer, tANI_U32 numBytes)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hHal;
    return( palWriteDeviceMemory( pMac->hHdd, dstOffset, (tANI_U8 *)pSrcBuffer, numBytes ) );
}

eHalStatus halNormalReadMemory(tHalHandle hHal, tANI_U32 srcOffset, void *pBuffer, tANI_U32 numBytes)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hHal;
    return( palReadDeviceMemory( pMac->hHdd, srcOffset, pBuffer, numBytes ) );
}

// Initialize the function pointers
static void halOpenInit(tpAniSirGlobal pMac)
{
    // Initialize the write register function pointer
    pMac->hal.funcWriteReg = halNormalWriteRegister;

    // Initialize the read register function pointer
    pMac->hal.funcReadReg = halNormalReadRegister;

    // Initialize the write register function pointer
    pMac->hal.funcWriteMem = halNormalWriteMemory;

    // Initialize the read register function pointer
    pMac->hal.funcReadMem = halNormalReadMemory;
}

static void halCloseExit(tpAniSirGlobal pMac)
{
    // Initialize the read/write register function pointer to NULL
    pMac->hal.funcWriteReg = NULL;
    pMac->hal.funcReadReg  = NULL;
    pMac->hal.funcWriteMem = NULL;
    pMac->hal.funcReadMem  = NULL;
}

/** -------------------------------------------------------------
\fn halFreeMsg
\brief Called by VOS scheduler (function vos_sched_flush_mc_mqs)
\      to free a given HAL message on the TX and MC thread.
\      This happens when there are messages pending in the HAL
\      queue when system is being stopped and reset.
\param   tpAniSirGlobal pMac
\param   tSirMsgQ       pMsg
\return  none
-----------------------------------------------------------------*/
void halFreeMsg( tpAniSirGlobal pMac, tSirMsgQ* pMsg)
{
    if(pMsg != NULL) {
        if (pMsg->bodyptr) {
            vos_mem_free((v_VOID_t*)pMsg->bodyptr);
        }

        pMsg->bodyptr = NULL;
        pMsg->bodyval = 0;
        pMsg->type = 0;
    }

    return;
}


#ifdef ANI_SNIFFER
eHalStatus halSetPromiscuousMode( tHalHandle hHal, tANI_BOOLEAN fMinimalCfg)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hHal;
    eHalStatus    status;
    tANI_U32    value;

    if(!fMinimalCfg)
    {
        status = halMsg_setPromiscMode(hHal);

        if (status != eHAL_STATUS_SUCCESS)
            return status;

        halRxp_disable(pMac);

        // Configure the Rxp sequence FIFO to work in promiscuous mode.
        halReadRegister(pMac, RXP_CFG_FLT_NAV_CONTROL_RESPONSE_REQ_REG, &value);
        value &= ~(  RXP_CFG_FLT_NAV_CONTROL_RESPONSE_REQ_CFG_FLT_SEQUENCE_NR_PREFETCHING_ENABLED_MASK|
                    RXP_CFG_FLT_NAV_CONTROL_RESPONSE_REQ_CFG_FLT_SEQUENCE_NR_FIFO_CLEAR_DISABLE_MASK |
                    RXP_CFG_FLT_NAV_CONTROL_RESPONSE_REQ_CFG_FLT_GENERATE_REPONSE_INT_ENABLED_MASK|
                    RXP_CFG_FLT_NAV_CONTROL_RESPONSE_REQ_CFG_FLT_RESPONSE_REQ_INT_BASED_WARMUP_MASK);
        halWriteRegister(pMac, RXP_CFG_FLT_NAV_CONTROL_RESPONSE_REQ_REG, value);

        halRxp_enable(pMac);
    }

    // Enable RXP to pass through FCS corrupted frames so that they can be displayed
    // in the sniffer
    status = halReadRegister(pMac, RXP_PKT_FILTER_MASK_REG, &value);

    if (status != eHAL_STATUS_SUCCESS) {
        return status;
    }

    value |= RXP_PKT_FILTER_MASK_MASK_FCS_ERR_MASK;
    return (halWriteRegister(pMac, RXP_PKT_FILTER_MASK_REG, value));
}

eHalStatus halResetPromiscuousMode( tHalHandle hHal)
{
    tANI_U32 value;
    tpAniSirGlobal pMac = (tpAniSirGlobal)hHal;
    eHalStatus    status;

    // Re-enable FCS error checks in RXP
    status = halReadRegister(pMac, RXP_PKT_FILTER_MASK_REG, &value);

    if (status != eHAL_STATUS_SUCCESS) {
        return status;
    }

    value &= ~RXP_PKT_FILTER_MASK_MASK_FCS_ERR_MASK;
    return (halWriteRegister(pMac, RXP_PKT_FILTER_MASK_REG, value));
}
#endif

/*
 * Wrapper function for halNvOpen
 */
static eHalStatus nvOpen(tHalHandle hHal, void *arg)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hHal;
    (void) arg;

    return( halNvOpen(pMac) );
}

/*
 * Wrapper function for halNvClose
 */
static eHalStatus nvClose(tHalHandle hHal, void *arg)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hHal;
    (void) arg;

    return( halNvClose(pMac) );
}


/*
 * Wrapper function for halPhyOpen
 */
static eHalStatus phyOpen(tHalHandle hHal, void *arg)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hHal;
    (void) arg;

    return( halPhyOpen(pMac) );
}


/*
 * Wrapper function for halPhyStart
 */
static eHalStatus phyStart(tHalHandle hHal, void *arg)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hHal;
    (void) arg;
    return( halPhyStart(pMac) );
}

/*
 * Wrapper function for halPhyStop
 */
static eHalStatus phyStop(tHalHandle hHal, void *arg)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hHal;
    (void) arg;
    return( halPhyStop(pMac) );
}


/*
 * Wrapper function for halPhyClose
 */
static eHalStatus phyClose(tHalHandle hHal, void *arg)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hHal;
    (void) arg;

    return( halPhyClose(pMac) );
}


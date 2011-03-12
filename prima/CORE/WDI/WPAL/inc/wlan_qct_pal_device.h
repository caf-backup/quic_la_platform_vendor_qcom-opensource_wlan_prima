#ifndef WLAN_QCT_PAL_DEVICE_H
#define WLAN_QCT_PAL_DEVICE_H
/* ====================================================================================================================

   @file   wlan_qct_pal_device.h

   @brief
    This file contains the external API exposed by WLAN PAL Device specific functionalities
    Copyright (c) 2011 Qualcomm Incorporated. All Rights Reserved
    Qualcomm Confidential and Properietary

 * ==================================================================================================================*/

/* ====================================================================================================================
                  EDIT HISTORY FOR FILE

   This section contains comments describing changes made to the module.
   Notice that changes are listed in reverse chronological order

   When             Who                 What, Where, Why
   ---------        --------            -------------------------------------------------------------------------------
   FEB/07/11        sch                 Create module
 * ==================================================================================================================*/

/* ====================================================================================================================
                  INCLUDE FILES FOR MODULES
 * ==================================================================================================================*/
#include "wlan_qct_pal_type.h"
#include "wlan_qct_pal_status.h"
#include "wlan_qct_pal_trace.h"

/* ====================================================================================================================
                  PREPROCESSORS AND DEFINITIONS
 * ==================================================================================================================*/
#define     DXE_INTERRUPT_TX_COMPLE      0x02
#define     DXE_INTERRUPT_RX_READY       0x04
#define     WPAL_ISR_CLIENT_MAX          0x08

/* ====================================================================================================================
  @  Function Name 
      wpalIsrType

  @  Description 
      DXE ISR functio prototype
      DXE should register ISR function into platform

  @  Parameters
      pVoid                       pDXEContext : DXE module control block

  @  Return
      NONE
 * ==================================================================================================================*/
typedef void (* wpalIsrType)(void *usrCtxt);

/* ====================================================================================================================
                  GLOBAL FUNCTIONS
 * ==================================================================================================================*/
/* ====================================================================================================================
   @ Function Name

   @ Description

   @ Arguments

   @ Return value

   @ Note

 * ==================================================================================================================*/
wpt_status wpalDeviceInit
(
   void                 *deviceCB
);

/* ====================================================================================================================
   @ Function Name

   @ Description

   @ Arguments

   @ Return value

   @ Note

 * ==================================================================================================================*/
wpt_status wpalDeviceClose
(
   void                 *deviceC
);

/* ====================================================================================================================
                  CLIENT SERVICE EXPOSE FUNCTIONS GENERIC
 * ==================================================================================================================*/
/* ====================================================================================================================
  @  Function Name 
      wpalRegisterInterrupt

  @  Description 
      Interrupt registration function
      Client(DXE) register interrupt into platform with callback function pointer and
      client context pointer
      DXE expectes 2 HW interrupts, 
      so DXE will call same registration functions 2 times with different call back function

  @  Parameters
      WLANDXE_InterruptType    intType : interrupt type, TX or RX
      WLANDXE_IsrType          callbackFunction : ISR function pointer
      void                    *usrCtxt : DXE control block

  @  Return
      wpt_status
 * ==================================================================================================================*/
wpt_status wpalRegisterInterrupt
(
   wpt_uint32                           intType,
   wpalIsrType                          callbackFunction,
   void                                *usrCtxt
);

/* ====================================================================================================================
  @  Function Name 
      wpalEnableInterrupt

  @  Description 
      Enable platform side interrupt interface
      If platform side interrupt is not enabled, even RIVA sends interrupt
      it will not be recognized

  @  Parameters
      WLANDXE_InterruptType    intType : interrupt type, TX or RX

  @  Return
      wpt_status
 * ==================================================================================================================*/
wpt_status wpalEnableInterrupt
(
   wpt_uint32                          intType
);

/* ====================================================================================================================
  @  Function Name 
      wpalDisableInterrupt

  @  Description 
      Disable platform side interrupt interface
      If platform side interrupt is not enabled, even RIVA sends interrupt
      it will not be recognized

  @  Parameters
      WLANDXE_InterruptType    intType : interrupt type, TX or RX

  @  Return
      wpt_status
 * ==================================================================================================================*/
wpt_status wpalDisableInterrupt
(
   wpt_uint32                           intType
);

/* ====================================================================================================================
   @ Function Name

   @ Description

   @ Arguments

   @ Return value

   @ Note

 * ==================================================================================================================*/
wpt_status wpalReadRegister
(
   wpt_uint32                           address,
   wpt_uint32                          *data
);

/* ====================================================================================================================
   @ Function Name

   @ Description

   @ Arguments

   @ Return value

   @ Note

 * ==================================================================================================================*/
wpt_status wpalWriteRegister
(
   wpt_uint32                           address,
   wpt_uint32                           data
);

/* ====================================================================================================================
   @ Function Name

   @ Description

   @ Arguments

   @ Return value

   @ Note

 * ==================================================================================================================*/
wpt_status wpalReadDeviceMemory
(
   wpt_uint32                            address,
   wpt_uint8                            *DestBuffer,
   wpt_uint32                            len
);

/* ====================================================================================================================
   @ Function Name

   @ Description

   @ Arguments

   @ Return value

   @ Note

 * ==================================================================================================================*/
wpt_status wpalWriteDeviceMemory
(
   wpt_uint32                            address,
   wpt_uint8                            *srcBuffer,
   wpt_uint32                            len
);


#endif /* WLAN_QCT_PAL_DEVICE_H*/

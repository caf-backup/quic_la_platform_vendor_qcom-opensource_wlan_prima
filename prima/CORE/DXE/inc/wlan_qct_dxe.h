#ifndef WLAN_QCT_DXE_H
#define WLAN_QCT_DXE_H

/**=========================================================================
  
  @file  wlan_qct_dxe.h
  
  @brief 
               
   This file contains the external API exposed by the wlan data transfer abstraction layer module.
   Copyright (c) 2008 QUALCOMM Incorporated. All Rights Reserved.
   Qualcomm Confidential and Proprietary
========================================================================*/

/*===========================================================================

                      EDIT HISTORY FOR FILE


  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.


  $Header:$ $DateTime: $ $Author: $


when           who        what, where, why
--------    ---         ----------------------------------------------------------
08/03/10    schang      Created module.

===========================================================================*/

/*===========================================================================

                          INCLUDE FILES FOR MODULE

===========================================================================*/

/*----------------------------------------------------------------------------
 * Include Files
 * -------------------------------------------------------------------------*/
#include "wlan_qct_pal_api.h"
#include "wlan_qct_pal_packet.h"
#include "wlan_qct_pal_status.h"
#include "wlan_qct_pal_type.h"
#include "wlan_qct_pal_msg.h"
#include "wlan_qct_pal_sync.h"
#include "wlan_qct_wdi_dts.h"

/*----------------------------------------------------------------------------
 *  Type Declarations
 * -------------------------------------------------------------------------*/
/* DXE Descriptor contents SWAP option flag */
#ifdef WINDOWS_DT
#ifndef FEATURE_R33D
#define FEATURE_R33D
#endif /* FEATURE_R33D */
#endif /* WINDOWS_DT */

//#define WLANDXE_ENDIAN_SWAP_ENABLE

/* Default RX OS frame buffer size
 * Size must be same with Vos Packet Size */
#define WLANDXE_DEFAULT_RX_OS_BUFFER_SIZE  1792

typedef enum
{
   WLANDXE_POWER_STATE_FULL,
   WLANDXE_POWER_STATE_BMPS,
   WLANDXE_POWER_STATE_MAX
} WLANDXE_PowerStateType;

/*==========================================================================
  @  Type Name
      WLANDXE_RxFrameReadyCbType 

  @  Description 
       RX Frame Ready indication CB

  @  Parameters
         pVoid         pAdaptor : Driver global control block pointer
         palPacket     pRXFramePtr : Received Frame Pointer
         pVoid         userCtxt : DTS user contect pointer

  @  Return
        wpt_status
===========================================================================*/
typedef WDTS_RxFrameReadyCbType WLANDXE_RxFrameReadyCbType;

/*==========================================================================
  @  Type Name
       WLANDXE_TxCompleteCbType 

  @  Description 
      TX complete indication CB

  @  Parameters
         pVoid      pAdaptor : Driver global control block pointer
         void       pTXFramePtr : Completed TX Frame Pointer
         pVoid      userCtxt : DTS user contect pointer

  @  Return
       wpt_status
===========================================================================*/
typedef WDTS_TxCompleteCbType WLANDXE_TxCompleteCbType;

/*==========================================================================
  @  Type Name
      WLANDXE_LowResourceCbType 

  @  Description 
       DXE Low resource indication CB

  @  Parameters
      pVoid      pAdaptor : Driver global control block pointer
      BOOL      lowResourceCondition : DXE low resource or not
      pVoid      userCtxt : DTS user contect pointer

  @  Return
      wpt_status
===========================================================================*/
typedef WDTS_LowResourceCbType WLANDXE_LowResourceCbType;

/*-------------------------------------------------------------------------
 *Function declarations and documenation
 *-------------------------------------------------------------------------*/
/*==========================================================================
  @  Function Name 
      WLANDXE_Open

  @  Description 
      Open host DXE driver, allocate DXE resources
      Allocate, DXE local control block, DXE descriptor pool, DXE descriptor control block pool

  @  Parameters
      pVoid      pAdaptor : Driver global control block pointer

  @  Return
      pVoid DXE local module control block pointer
===========================================================================*/
void *WLANDXE_Open
(
   void
);

/*==========================================================================
  @  Function Name 
      WLANDXE_ClientRegistration

  @  Description 
      Make callback functions registration into DXE driver from DXE driver client

  @  Parameters
      pVoid                       pDXEContext : DXE module control block
      WDTS_RxFrameReadyCbType     rxFrameReadyCB : RX Frame ready CB function pointer
      WDTS_TxCompleteCbType       txCompleteCB : TX complete CB function pointer
      WDTS_LowResourceCbType      lowResourceCB : Low DXE resource notification CB function pointer
      void                       *userContext : DXE Cliennt control block

  @  Return
      wpt_status
===========================================================================*/
wpt_status WLANDXE_ClientRegistration
(
   void                       *pDXEContext,
   WDTS_RxFrameReadyCbType     rxFrameReadyCB,
   WDTS_TxCompleteCbType       txCompleteCB,
   WDTS_LowResourceCbType      lowResourceCB,
   void                       *userContext
);

/*==========================================================================
  @  Function Name 
      WLANDXE_Start

  @  Description 
      Start Host DXE driver
      Initialize DXE channels and start channel

  @  Parameters
      pVoid                       pDXEContext : DXE module control block

  @  Return
      wpt_status
===========================================================================*/
wpt_status WLANDXE_Start
(
   void  *pDXEContext
);

/*==========================================================================
  @  Function Name 
      WLANDXE_TXFrame

  @  Description 
      Trigger frame transmit from host to RIVA

  @  Parameters
      pVoid            pDXEContext : DXE Control Block
      wpt_packet       pPacket : transmit packet structure
      WDTS_ChannelType channel : TX channel

  @  Return
      wpt_status
===========================================================================*/
wpt_status WLANDXE_TxFrame
(
   void                 *pDXEContext,
   wpt_packet           *pPacket,
   WDTS_ChannelType      channel
);

/*==========================================================================
  @  Function Name 
      WLANDXE_Stop

  @  Description 
      Stop DXE channels and DXE engine operations

  @  Parameters
      pVoid            pDXEContext : DXE Control Block

  @  Return
      wpt_status
===========================================================================*/
wpt_status WLANDXE_Stop
(
   void *pDXEContext
);

/*==========================================================================
  @  Function Name 
      WLANDXE_Close

  @  Description 
      Close DXE channels
      Free DXE related resources
      DXE descriptor free
      Descriptor control block free
      Pre allocated RX buffer free

  @  Parameters
      pVoid            pDXEContext : DXE Control Block

  @  Return
      wpt_status
===========================================================================*/
wpt_status WLANDXE_Close
(
   void *pDXEContext
);

/*==========================================================================
  @  Function Name 
      WLANDXE_TriggerTX

  @  Description 
      TBD

  @  Parameters
      pVoid            pDXEContext : DXE Control Block

  @  Return
      wpt_status
===========================================================================*/
wpt_status WLANDXE_TriggerTX
(
   void *pDXEContext
);

/*==========================================================================
  @  Function Name 
      WLANDXE_SetPowerStats

  @  Description 
      From Client let DXE knows what is the WLAN HW(RIVA) power state

  @  Parameters
      pVoid                    pDXEContext : DXE Control Block
      WLANDXE_PowerStateType   powerState

  @  Return
      wpt_status
===========================================================================*/
wpt_status WLANDXE_SetPowerStats
(
   void                    *pDXEContext,
   WLANDXE_PowerStateType   powerState
);

#ifdef WLANDXE_TEST_CHANNEL_ENABLE
/*==========================================================================
  @  Function Name 
      WLANDXE_UnitTest

  @  Description 
      Temporary for the DXE module test

  @  Parameters
      NONE

  @  Return
      NONE

===========================================================================*/
void WLANDXE_UnitTestStartDXE
(
   void
);

/*==========================================================================
  @  Function Name 

  @  Description 

  @  Parameters

  @  Return

===========================================================================*/
void WLANDXE_UnitTestDataTransfer
(
   void
);

/*==========================================================================
  @  Function Name 

  @  Description 

  @  Parameters

  @  Return

===========================================================================*/
void WLANDXE_UnitTestEventHandle
(
   void     *dxeCB
);
#endif /* WLANDXE_TEST_CHANNEL_ENABLE */
#endif /* WLAN_QCT_DXE_H */

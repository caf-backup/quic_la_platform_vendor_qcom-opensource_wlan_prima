#ifndef WLAN_QCT_BAL_H
#define WLAN_QCT_BAL_H

/*=========================================================================
  
  @file  wlan_qct_bal.h
  
  @brief WLAN BUS ABSTRACTION LAYER EXTERNAL API
               
   This file contains the external API exposed by the wlan bus abstraction layer module.
  
   Copyright 2008 (c) Qualcomm Technologies, Inc.  All Rights Reserved.
   
   Qualcomm Technologies Confidential and Proprietary.
  
  ========================================================================*/

/*===========================================================================

                      EDIT HISTORY FOR FILE


  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.


  $Header:$ $DateTime: $ $Author: $


when           who        what, where, why
--------    ---         ----------------------------------------------------------
05/21/08    schang      Created module.

===========================================================================*/

/* $Header$ */

/*===========================================================================

                          INCLUDE FILES FOR MODULE

===========================================================================*/

/*----------------------------------------------------------------------------
 * Include Files
 * -------------------------------------------------------------------------*/
#include "vos_api.h"
#include "wlan_qct_sal.h"

#ifdef __cplusplus
extern "C" {
#endif
/*----------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * -------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
 *  Type Declarations
 * -------------------------------------------------------------------------*/
/* HAL callback functions typedef */
/*----------------------------------------------------------------------------

  @brief If ASIC interrupt is happen this callback will be invoked
        Interrupt will be routed to HAL

  @param pAdater - Global adapter handle
      
  @return General status code
        VOS_STATUS_SUCCESS      Success
        VOS_STATUS_E_FAILURE    HAL Cback function pointer is not valid
        
----------------------------------------------------------------------------*/
typedef VOS_STATUS
(*WLANBAL_ASICInterruptCBType)(v_PVOID_t pAdapter);

/*----------------------------------------------------------------------------

  @brief If fatal error is happen at the lower layer this callback will be
        invoked. Error will be routed to HAL

  @param v_U32_t errorCode
        ERROR_UNKNOWN_REASON      Not defined yet
        ERROR_UNSUPPORTED_REASON  Unsupported operation
        ERROR_MAX_REASON          MAX

  @param v_PVOID_t pAdater
        Global adapter handle

  @param NONE
      
  @return General status code
        VOS_STATUS_SUCCESS      Success
        VOS_STATUS_E_FAILURE    HAL Cback function pointer is not valid
        
----------------------------------------------------------------------------*/
typedef VOS_STATUS
(*WLANBAL_FatalErrorCBType)(v_PVOID_t pAdapter,
                            v_U32_t   errorCode);

#ifdef WLAN_FEATURE_PROTECT_TXRX_REG_ACCESS
/*----------------------------------------------------------------------------

  @brief Whenever BAL wants to make sure hardware should be awake during the 
        register access, this callback should be called before accessing the 
        register that is intended to read

  @param pMacContext - Userdata passed during registration 
      
  @return General status code
        VOS_STATUS_SUCCESS      Success
        VOS_STATUS_E_FAILURE    if unable to acquire resource
        
----------------------------------------------------------------------------*/
typedef VOS_STATUS
    (*WLANBAL_AcquireResourceCBType)(v_PVOID_t pMacContext);

/*----------------------------------------------------------------------------

  @brief After completing the necessary operations with hardware, BAL should 
        call this callback to release the previously acquired resource.

  @param pMacContext - Userdata passed during registration 
      
  @return General status code
        VOS_STATUS_SUCCESS      Success
        VOS_STATUS_E_FAILURE    if unable to acquire resource
----------------------------------------------------------------------------*/

typedef VOS_STATUS
    (*WLANBAL_ReleaseResourceCBType)(v_PVOID_t pMacContext);
#endif

/*----------------------------------------------------------------------------

  @brief BAL calls this callback to get the DXE channel status when SSC
         requires 

  @param pMacContext - Userdata passed during registration 
      
  @return General status code
        VOS_STATUS_SUCCESS      Success
        VOS_STATUS_E_FAILURE    if unable to acquire resource
----------------------------------------------------------------------------*/

typedef VOS_STATUS
    (*WLANBAL_DxEChannelStatusCBType)(v_U32_t *pStatus, v_PVOID_t pMacContext);
/*
 * Elements HAL have to register to BAL
 */
typedef struct
{
   /* ASIC interrupt callback Function */
   WLANBAL_ASICInterruptCBType   asicInterruptCB;
   /* Fatal error callback Function */
   WLANBAL_FatalErrorCBType      fatalErrorCB;

#ifdef WLAN_FEATURE_PROTECT_TXRX_REG_ACCESS
   /* Callback to acquire mutex */
   WLANBAL_AcquireResourceCBType acquireResourceCB;
   /* Callback to release mutex */
   WLANBAL_ReleaseResourceCBType releaseResourceCB;
#endif /* WLAN_FEATURE_PROTECT_TXRX_REG_ACCESS */

   /* Callback to get dxe channel status */
   WLANBAL_DxEChannelStatusCBType dxeChannelStatusCB;

   /* HAL internal handle */
   v_PVOID_t                     halUsrData;
} WLANBAL_HalRegType;

/*
 * SDIO DXE Header config information per channel
 */
typedef struct

{
   v_BOOL_t     shortDescriptor;
   v_U32_t      sourceAddress;
   v_U32_t      destinationAddress;
   v_U32_t      nextAddress;
   v_U32_t      descriptorControl;
} WLANBAL_SDIODXEChannelConfigType;

typedef struct
{
   WLANBAL_SDIODXEChannelConfigType TXChannel;
   WLANBAL_SDIODXEChannelConfigType RXChannel;
#ifdef ANI_CHIPSET_VOLANS
   WLANBAL_SDIODXEChannelConfigType RXHiChannel;
#endif
} WLANBAL_SDIODXEHeaderConfigType;

/*
 * DXE Header type
 * DXE Short Header size is 20 bytes
 */
typedef struct
{
   v_U32_t    descriptorControl;
   v_U32_t    size;
   v_U32_t    srcAddress;
   v_U32_t    destAddress;
   v_U32_t    nextAddress;
} WLANBAL_sDXEHeaderType;

#define WLAN_BAL_DXE_HEADER_SIZE   sizeof(WLANBAL_sDXEHeaderType)

/* TL callback functions typedef */
/*----------------------------------------------------------------------------

  @brief Notify frame received

  @param voss_pkt_t     vossPacketPtr
        vOSS frame pointer

  @param v_PVOID_t pAdapter
        Global adapter handle

  @param NONE
      
  @return General status code
        VOS_STATUS_SUCCESS      Success
        VOS_STATUS_E_INVAL      Invalid argument
        VOS_STATUS_E_FAILURE    TL Cback function pointer is not valid
        
----------------------------------------------------------------------------*/
typedef VOS_STATUS
(*WLANBAL_ReceiveFrameCBType)(v_PVOID_t      pAdapter,
                              vos_pkt_t     *vossPacketPtr);
                              
/*----------------------------------------------------------------------------

  @brief Notify send out frames are done to TL

  @param voss_pkt_t          vossPacketPtr
        vOSS frame pointer

  @param WLAN_TxStatusType   txStatus
        Frame send out status

  @param v_PVOID_t pAdapter
        Global adapter handle

  @return General status code
        VOS_STATUS_SUCCESS      Success
        VOS_STATUS_E_INVAL      Invalid argument
        VOS_STATUS_E_FAILURE    TL Cback function pointer is not valid
        
----------------------------------------------------------------------------*/
typedef VOS_STATUS
(*WLANBAL_TXCompleteCBType)(v_PVOID_t           pAdapter,
                            vos_pkt_t         * vossPacketPtr,
                            VOS_STATUS          txStatus);

/*----------------------------------------------------------------------------

  @brief Get TX frames from TL

  @param v_U16_t      maxPacket
        Max packet counts allowed by SSC

  @param voss_pkt_t   vossPacketPtr
        Packet pointer have to be sent out

  @param v_PVOID_t pAdapter
        Global adapter handle
      
  @return General status code
        VOS_STATUS_SUCCESS      Success
        VOS_STATUS_E_INVAL      Invalid argument
        VOS_STATUS_E_FAILURE    TL Cback function pointer is not valid
        
----------------------------------------------------------------------------*/
typedef v_BOOL_t
(*WLANBAL_GetTXFramesCBType)(v_PVOID_t    pAdapter,
                             vos_pkt_t  **vossPacketPtr,
                             v_U32_t      maxPacket,
                             v_BOOL_t*    pbUrgent );

/*----------------------------------------------------------------------------

  @brief Send TX buffer resources to TL asynchlonously

  @param v_U32_t     txResourcesAvailable,
        WLAN HW buffer availble
        
  @param v_PVOID_t pAdapter
        Global adapter handle

  @return General status code
        VOS_STATUS_SUCCESS      Success
        VOS_STATUS_E_FAILURE    HAL Cback function pointer is not valid
        
----------------------------------------------------------------------------*/
typedef VOS_STATUS
(*WLANBAL_TXResourcesCBType)(v_PVOID_t   pAdapter,
                             v_U32_t     txResourcesAvailable);

/* Elements have to be registered to BAL by TL */
typedef struct
{
   WLANBAL_ReceiveFrameCBType    receiveFrameCB;
   WLANBAL_GetTXFramesCBType     getTXFrameCB;
   WLANBAL_TXCompleteCBType      txCompleteCB;
   WLANBAL_TXResourcesCBType     txResourceCB;
   v_U32_t                       txResourceThreashold;
   v_PVOID_t                     tlUsrData;
} WLANBAL_TlRegType;


/*End This is place holder for compile */
/*-------------------------------------------------------------------------
 *Function declarations and documenation
 *-------------------------------------------------------------------------*/

/*=========================================================================
 * Interactions with vOSS
 *=========================================================================*/ 
/*----------------------------------------------------------------------------

  @brief During open process, resources are needed have to be allocated and 
        internal structure have to be initialized. And have to open trigger
        SAL and SSC module

  @param v_PVOID_t pAdapter
        Global adapter handle

  @return General status code
        VOS_STATUS_SUCCESS      Open Success
        VOS_STATUS_E_NOMEM      Open Fail, Resource alloc fail
        VOS_STATUS_E_FAILURE    Open Fail, Unknown reason        
      
----------------------------------------------------------------------------*/
VOS_STATUS WLANBAL_Open
(
   v_PVOID_t pAdapter
);

/*----------------------------------------------------------------------------

  @brief Start BAL module and start trigger SAL and SSC.
        If BAL is not open return fail

  @param v_PVOID_t pAdapter
        Global adapter handle

  @return General status code
        VOS_STATUS_SUCCESS      Start Success
        VOS_STATUS_E_FAILURE    Start Fail, BAL Not open yet

----------------------------------------------------------------------------*/
VOS_STATUS WLANBAL_Start
(
   v_PVOID_t pAdapter
);


/*----------------------------------------------------------------------------

  @brief Stop BAL module. Initialize BAL internal structure.
        Trigger to stop SAL and SSC

  @param  v_PVOID_t pAdapter
        Global adapter handle

  @return General status code
        VOS_STATUS_SUCCESS      Stop Success
        VOS_STATUS_E_FAILURE    Stop Fail, BAL not started     
        
----------------------------------------------------------------------------*/
VOS_STATUS WLANBAL_Stop
(
   v_PVOID_t pAdapter
);


/*----------------------------------------------------------------------------

  @brief Close BAL module. Free internal resources.
        Trigger to close SAL and SSC

  @param v_PVOID_t pAdapter
        Global adapter handle

  @return General status code
        VOS_STATUS_SUCCESS      Close Success
        VOS_STATUS_E_FAILURE    Close Fail, BAL not open 
      
----------------------------------------------------------------------------*/
VOS_STATUS WLANBAL_Close
(
   v_PVOID_t pAdapter
);


/*----------------------------------------------------------------------------

  @brief TBD

  @param v_PVOID_t pAdapter
        Global adapter handle

  @return General status code
      
----------------------------------------------------------------------------*/
VOS_STATUS WLANBAL_Reset
(
   v_PVOID_t pAdapter
);


/*=========================================================================
 * END Interactions with vOSS
 *=========================================================================*/ 

/*=========================================================================
 * Interactions with HAL
 *=========================================================================*/ 
 
/*----------------------------------------------------------------------------

  @brief Register HAL Callback functions to BAL.
        Registration functions are Interrupt happen notification function
        and fatal error happen notification function

  @param v_PVOID_t pAdapter
        Global adapter handle
      
  @param WLANBAL_HalRegType
        HAL Registration elements.
        ASIC interrupt callback function, Fatal error notification function
        and HAL internal data have to be registered.

  @return General status code
        VOS_STATUS_SUCCESS       Registration success
        VOS_STATUS_E_RESOURCES   BAL resources are not ready
        VOS_STATUS_E_INVAL       Invalid argument
      
----------------------------------------------------------------------------*/
VOS_STATUS WLANBAL_RegHalCBFunctions
(
   v_PVOID_t           pAdapter,
   WLANBAL_HalRegType *halReg
);

/*----------------------------------------------------------------------------

  @brief Read register value from WLAN hardware
        Every register size is 4 byte

  @param v_PVOID_t pAdapter
        Global adapter handle
      
  @param v_U32_t regAddress
        Register address have to be read

  @param v_U32_t *bufferPtr
        Buffer pointer will store data
      
  @return General status code
        VOS_STATUS_SUCCESS       Read success
        VOS_STATUS_E_INVAL       bufferPtr is not valid
        VOS_STATUS_E_FAILURE     BAL is not ready
        
----------------------------------------------------------------------------*/
VOS_STATUS WLANBAL_ReadRegister
(
   v_PVOID_t  pAdapter,
   v_U32_t    regAddress,
   v_U32_t   *bufferPtr
);

/*----------------------------------------------------------------------------

  @brief Read Multiple registers value from WLAN hardware
         HAL Has to give enough buffer space and
         Registers addresses have to be successful
         Every register size is 4 byte

  @param v_PVOID_t pAdapter
        Global adapter handle
      
  @param v_U32_t regAddress
        Register address have to be read

  @param v_U32_t *bufferPtr
        Buffer pointer will store data

  @param v_U32_t numRegisters
        Count of Registers have to be read
      
  @return General status code
        VOS_STATUS_SUCCESS       Read success
        VOS_STATUS_E_INVAL       bufferPtr is not valid
        VOS_STATUS_E_FAILURE     BAL is not ready
        
----------------------------------------------------------------------------*/
VOS_STATUS WLANBAL_ReadMultipleRegisters
(
   v_PVOID_t  pAdapter,
   v_U32_t    regAddress,
   v_U32_t   *bufferPtr,
   v_U32_t    numRegisters
);

/*----------------------------------------------------------------------------

  @brief Write register value from WLAN hardware
        Every register size is 4 byte

  @param v_PVOID_t pAdapter
        Global adapter handle
      
  @param v_U32_t regAddress
        Register address has to be write
        
  @param v_U32_t regData
        Data value will be written

  @return General status code
        VOS_STATUS_SUCCESS       Write success
        VOS_STATUS_E_FAILURE     BAL is not ready
      
----------------------------------------------------------------------------*/
VOS_STATUS WLANBAL_WriteRegister
(
   v_PVOID_t  pAdapter,
   v_U32_t    regAddress,
   v_U32_t    regData
);

/*----------------------------------------------------------------------------

  @brief Read memory from WLAN hardware

  @param v_PVOID_t pAdapter
        Global adapter handle
      
  @param v_U32_t memAddress
        v_U8_t length

  @param v_PVOID_t *bufferPtr
      
  @return General status code
      
----------------------------------------------------------------------------*/
VOS_STATUS WLANBAL_ReadMemory
(
   v_PVOID_t  pAdapter,
   v_U32_t    memAddress,
   v_PVOID_t  bufferPtr,
   v_U32_t    length
);

/*----------------------------------------------------------------------------

  @brief Write memory from WLAN hardware

  @param v_U32_t   memAddress
  @param v_PVOID_t *bufferPtr
  @param v_U8_t    length

  @return General status code
      
----------------------------------------------------------------------------*/
VOS_STATUS WLANBAL_WriteMemory
(
   v_PVOID_t  pAdapter,
   v_U32_t    memAddress,
   v_PVOID_t  bufferPtr,
   v_U32_t    length
);

/*----------------------------------------------------------------------------

  @brief Enable WLAN ASIC interrupt

  @param v_PVOID_t pAdapter
        Global adapter handle

  @return General status code
      
----------------------------------------------------------------------------*/
VOS_STATUS WLANBAL_EnableASICInterrupt
(
   v_PVOID_t pAdapter
);

#ifdef WLAN_PERF 
VOS_STATUS WLANBAL_EnableASICInterruptEx
(
   v_PVOID_t pAdapter,
   v_U32_t   uIntMask
);
#endif /* WLAN_PERF */

/*----------------------------------------------------------------------------

  @brief Disable WLAN ASIC interrupt

  @param v_PVOID_t pAdapter
        Global adapter handle

  @return General status code
      
----------------------------------------------------------------------------*/
VOS_STATUS WLANBAL_DisableASICInterrupt
(
   v_PVOID_t pAdapter
);

#ifdef WLAN_PERF 
VOS_STATUS WLANBAL_DisableASICInterruptEx
(
   v_PVOID_t pAdapter,
   v_U32_t   uIntMask
   
);
#endif /* WLAN_PERF  */
/*----------------------------------------------------------------------------

  @brief Config DXE Channel information

  @param v_PVOID_t pAdapter Global adapter handle
  @param WLANBAL_SDIODXEHeaderConfigType    ConfigInfo Default channel config infromation

  @return General status code
      
----------------------------------------------------------------------------*/
VOS_STATUS WLANBAL_DXEHeaderConfig
(
   v_PVOID_t                          pAdapter,
   WLANBAL_SDIODXEHeaderConfigType   *ConfigInfo
);

/*----------------------------------------------------------------------------

  @brief Suspend BAL, Trigger SSC Suspend

  @param v_PVOID_t pAdapter
        Global adapter handle

  @return General status code

----------------------------------------------------------------------------*/
VOS_STATUS WLANBAL_Suspend
(
   v_PVOID_t pAdapter
);

/*----------------------------------------------------------------------------

  @brief Resume BAL from Suspend, Trigger SSC Resume

  @param v_PVOID_t pAdapter
        Global adapter handle

  @return General status code

----------------------------------------------------------------------------*/
VOS_STATUS WLANBAL_Resume
(
   v_PVOID_t pAdapter
);

/*----------------------------------------------------------------------------

  @brief Suspend Entire chip, Trigger SSC Suspend Chip

  @param v_PVOID_t pAdapter
        Global adapter handle

  @return General status code
        VOS_STATUS_SUCCESS       Suspend Chip success
        VOS_STATUS_E_INVAL       Invalid Parameters

----------------------------------------------------------------------------*/
VOS_STATUS WLANBAL_SuspendChip
(
   v_PVOID_t pAdapter
);


/*----------------------------------------------------------------------------
  @brief Suspend Entire chip, Trigger SSC Suspend Chip

  @param v_PVOID_t pAdapter
        Global adapter handle

  @return General status code
        VOS_STATUS_SUCCESS       Suspend Chip success
        VOS_STATUS_E_INVAL       Invalid Parameters

----------------------------------------------------------------------------*/
VOS_STATUS WLANBAL_SuspendChip_NoLock
(
   v_PVOID_t pAdapter
);

/*----------------------------------------------------------------------------

  @brief Resume entire chip from Suspend, Trigger SSC Resume chip

  @param v_PVOID_t pAdapter
        Global adapter handle

  @return General status code
        VOS_STATUS_SUCCESS       Suspend Chip success
        VOS_STATUS_E_INVAL       Invalid Parameters

----------------------------------------------------------------------------*/
VOS_STATUS WLANBAL_ResumeChip
(
   v_PVOID_t pAdapter
);


/*=========================================================================
 * END Interactions with HAL
 *=========================================================================*/ 

/*=========================================================================
 * Interactions with TL
 *=========================================================================*/ 

/*----------------------------------------------------------------------------

  @brief Register TL Callback functions to BAL.
        Registration functions are Receive Frames notification function,
        Get send frame request function, send complete notification functions
        and TX resource availability notification function

  @param v_PVOID_t pAdapter
        Global adapter handle
      
  @param WLANBAL_TlRegType *tlReg
        TL registration element type.
        Receive frame CB, TX complete CB, get TX Frame CB, TX resources CB
        and TL internal hanlde are elements of this structure

  @return General status code
        VOS_STATUS_SUCCESS       Registration success
        VOS_STATUS_E_INVAL       Invalid argument
        VOS_STATUS_E_FAILURE     BAL is not ready
      
----------------------------------------------------------------------------*/
VOS_STATUS WLANBAL_RegTlCbFunctions
(
   v_PVOID_t          pAdapter,
   WLANBAL_TlRegType *tlReg
);

/*----------------------------------------------------------------------------

  @brief Get available TX buffer size within WLAN ASIC.
        If TX buffer size is larger then preset threshold value,
        just pass buffer size. Otherwise, pass TX buffer size and start timer
        to wait HW handle TX buffer and make it free. After timer expired,
        read TX buffer size again. During this timer running, TL has to wait

  @param v_PVOID_t pAdapter
        Global adapter handle

  @param v_U32_t *availableTxBuffer
        Buffer pointer have to be stored TX resource size
      
  @return General status code
        VOS_STATUS_SUCCESS      Notify success
        VOS_STATUS_E_INVAL      Invalid argument
        VOS_STATUS_E_FAILURE    BAL is not ready
      
----------------------------------------------------------------------------*/
VOS_STATUS WLANBAL_GetTxResources
(
   v_PVOID_t  pAdapter,
   v_U32_t   *availableTxBuffer
);

/*----------------------------------------------------------------------------

  @brief TL Notify there are TX pending frames to SSC

  @param v_PVOID_t pAdapter
        Global adapter handle

  @return General status code
        VOS_STATUS_SUCCESS      Notify success
        VOS_STATUS_E_FAILURE    BAL is not ready
      
----------------------------------------------------------------------------*/
VOS_STATUS WLANBAL_StartXmit
(
   v_PVOID_t pAdapter
);

/*----------------------------------------------------------------------------

  @brief Routine to get vendor specific SDIO card ID

  @param v_PVOID_t pAdapter
         Global adapter handle
         
         v_U16_t   *pCard_Id
         ASIC vendor specific id

  @return General status code
        VOS_STATUS_SUCCESS      Notify success
        VOS_STATUS_E_FAILURE    BAL is not ready
      
----------------------------------------------------------------------------*/
VOS_STATUS WLANBAL_GetSDIOCardIdentifier
(
   v_PVOID_t pAdapter,
   v_U16_t   *pCardId
);

/*=========================================================================
 * END Interactions with TL
 *=========================================================================*/ 

/*=========================================================================
 * Interactions with SSC
 *=========================================================================*/ 

/*=========================================================================
 * END Interactions with SSC
 *=========================================================================*/ 
#ifdef __cplusplus
}
#endif
#endif /* WLAN_QCT_BAL_H */

#ifndef WLAN_QCT_SSC_H
#define WLAN_QCT_SSC_H

/*===========================================================================
  @file wlan_qct_ssc.h

  @brief This header file defines the API provided by the WLAN SIF (SDIO 
  interface) Software Controller (SSC) to its clients in order to access 
  the SDIO bus.
  The APIs provided include primarily data packet tx/rx functionality as well
  as handling of the SIF hardware block interrupts. Register/memory access is
  to be provided directly by VOS.

  The SIF module is a hardware component running on a companion ASIC with
  WLAN functionality (such as Libra), and the SSC can be treated as a host
  software driver running on the MSM to transfer requests for SIF access
  over the SDIO bus.
  Please see SSC HLD and LLD documents in order to understand the callflows
  and behavior expected of SSC clients.
  
  Copyright (c) 2008 QUALCOMM Incorporated.
  All Rights Reserved.
  Qualcomm Confidential and Proprietary
===========================================================================*/

/*=========================================================================== 
    
                       EDIT HISTORY FOR FILE 
   
   
  This section contains comments describing changes made to the module. 
  Notice that changes are listed in reverse chronological order. 
   
   
  $Header:$ $DateTime: $ $Author: $ 
   
   
  when        who    what, where, why 
  --------    ---    --------------------------------------------------------
  05/05/08    lyr     Created module. 
     
===========================================================================*/ 

/*---------------------------------------------------------------------------
 * Include Files
 * ------------------------------------------------------------------------*/

#include "vos_types.h"
#include "vos_status.h"
#include "vos_packet.h"
#include "vos_mq.h"

/*---------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * ------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
   WLANSSC_RESET_HANDLE

   Clients of SSC should not be operating on the handle directly - they
   should use this MACRO to reset to an invalid value.
---------------------------------------------------------------------------*/
#define WLANSSC_RESET_HANDLE(Handle_)  ((Handle_) = NULL)


/*---------------------------------------------------------------------------
   WLANSSC_HANDLE_IS_VALID

   Clients of SSC should not be operating on the handle directly - they
   should use this MACRO to check if the handle is valid
---------------------------------------------------------------------------*/
#define WLANSSC_HANDLE_IS_VALID(Handle_)  ( (NULL == (Handle_)) ? VOS_FALSE : VOS_TRUE )


/*---------------------------------------------------------------------------
 * Type Declarations
 * ------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------
   WLANSSC_HandleType

   This defines the type of an SSC handle
---------------------------------------------------------------------------*/
typedef v_PVOID_t WLANSSC_HandleType;


/*---------------------------------------------------------------------------
   WLANSSC_ReasonCodeType

   This gives the possible reason codes in APIs offered by SSC to its clients
   for certain notifications and results
---------------------------------------------------------------------------*/
typedef enum
{
  WLANSSC_UNKNOWN_REASON      = 0,
  WLANSSC_UNSUPPORTED_REASON  = 1,

  WLANSSC_MAX_REASON

} WLANSSC_ReasonCodeType;


/*---------------------------------------------------------------------------
   WLANSSC_FlowType

   This gives the possible flows within SSC that can be used to suspend
   or resume

   For now, interrupts cannot be disabled; however, if need be, this can be
   added here.
---------------------------------------------------------------------------*/
typedef enum
{
  WLANSSC_TX_FLOW             = 0, /* Flow in the transmit direction only  */
  WLANSSC_RX_FLOW             = 1, /* Flow in the receive direction only   */
  WLANSSC_ALL_FLOW            = 2, /* All flows (both tx/rx for now)       */

  WLANSSC_MAX_FLOW

} WLANSSC_FlowType;


/*---------------------------------------------------------------------------
   WLANSSC_GetSSCContextCbackType

   This callback is invoked by SSC to obtain a pointer to the SSC context
   from the entity invoking the SSC_Open() API.

   The size specifies the memory required to be present in the SSC context
   for use by SSC for internal storage.

   Return value: Status of function call
---------------------------------------------------------------------------*/
typedef VOS_STATUS (* WLANSSC_GetSSCContextCbackType)
(
  /* Size that SSC intends to use within the context pointer               */
  v_U32_t        uRequestedContextSize,

  /* OUT value that provides the SSC context pointer                       */
  v_PVOID_t     *pSSCContext,

  /* user data passed with Open                                            */
  v_PVOID_t      UserData
);


/*---------------------------------------------------------------------------
   WLANSSC_GetMultipleTxPacketCbackType

   This callback is invoked by SSC to retrieve a packet chain from the entity
   which registered it
   The number of bytes specifies the MAX bytes allowed to be passed in the
   packet chain
   TxCompleteUserData is stored by SSC and passed back in the corresponding
   TxCompleteCback invocation
   PktChainPtr is the packet chain containing data; if no data, this should
   be set to NULL.

   Return value: Is more data available at client?
                 A return of TRUE could trigger further requests from SSC
---------------------------------------------------------------------------*/
typedef v_BOOL_t (* WLANSSC_GetMultipleTxPacketCbackType)
(
  /* Maximum allowed number of bytes in the packet chain returned          */
  v_U32_t        uMaxAllowedBytes,

  /* Pointer to the packet chain                                           */
  vos_pkt_t    **pPktChain,

  /* user data passed with Start                                           */
  v_PVOID_t      UserData,

  /* user data to be passed with TxComplete                                */
  v_PVOID_t      pTxCompleteUserData,

  /*Compensation delay is not allowed if pkt is urgent*/
  v_BOOL_t*       pbUrgent
);


/*---------------------------------------------------------------------------
   WLANSSC_TxCompleteCbackType

   This callback is invoked by SSC to notify the client that a transmission
   has been completed
   PktChainPtr is the previous packet chain passed via a call to 
   GetMultipleTxPacketCback() and this data has been processed completely
   by SSC
   Note that the status code defines whether it succeeded
---------------------------------------------------------------------------*/
typedef v_VOID_t (* WLANSSC_TxCompleteCbackType)
(
  /* Pointer to the packet chain                                           */
  vos_pkt_t          *pPktChain,

  /* Status of the transmission attempt                                    */
  VOS_STATUS          eStatus,

  /* user data passed with Start                                           */
  v_PVOID_t           UserData,

  /* user data passed with the tx data                                     */
  v_PVOID_t           TxCompleteUserData
);


/*---------------------------------------------------------------------------
   WLANSSC_RxPacketHandlerCbackType

   This callback is invoked by SSC to provide a packet chain to the entity
   which registered it, containing complete packets received over the SDIO
   bus
---------------------------------------------------------------------------*/
typedef v_VOID_t (* WLANSSC_RxPacketHandlerCbackType)
(
  /* Pointer to the packet chain                                           */
  vos_pkt_t    *pPktChain,

  /* user data passed with Start                                           */
  v_PVOID_t     UserData
);


/*---------------------------------------------------------------------------
   WLANSSC_FatalErrorIndicationCbackType

   This callback is invoked by SSC to provide an indication that a fatal
   error has occurred in the SSC that could not be recovered from internally.
   A reset of the chip and software stack is likely the recovery process.
---------------------------------------------------------------------------*/
typedef v_VOID_t (* WLANSSC_FatalErrorIndicationCbackType)
(
  /* Reason code for the error                                             */
  WLANSSC_ReasonCodeType  eReason,

  /* user data passed with Start                                           */
  v_PVOID_t               UserData
);


/*---------------------------------------------------------------------------
   WLANSSC_ASICInterruptIndicationCbackType

   This callback is invoked by SSC to provide an indication that an ASIC
   interrupt was received at the SSC level and further processing is required
   by the SSC client
---------------------------------------------------------------------------*/
typedef v_VOID_t (* WLANSSC_ASICInterruptIndicationCbackType)
(
  /* user data passed with Start                                           */
  v_PVOID_t         UserData
);


/*---------------------------------------------------------------------------
   WLANSSC_StartParamsType

   This is the set of parameters SSC requires as part of the WLANSSC_Start 
   API.
   The members defined in this structure are all mandatory and if not filled,
   the call will fail.
---------------------------------------------------------------------------*/
typedef struct
{
  WLANSSC_GetMultipleTxPacketCbackType      pfnGetMultipleTxPacketCback;
  WLANSSC_TxCompleteCbackType               pfnTxCompleteCback;
  WLANSSC_RxPacketHandlerCbackType          pfnRxPacketHandlerCback;
  WLANSSC_ASICInterruptIndicationCbackType  pfnASICInterruptIndicationCback;
  WLANSSC_FatalErrorIndicationCbackType     pfnFatalErrorIndicationCback;
} WLANSSC_StartParamsType;

   

/*---------------------------------------------------------------------------
 * Function Declarations and Documentation
 * ------------------------------------------------------------------------*/

/**
 @brief WLANSSC_Open is used to perform power-up initialization tasks 
 including initialization of SSC internal data structures.

 Must be called only once before WLANSSC_Close

 @param Handle: OUT parameter that provides the caller a handle created
                This handle must then be passed on every subsequent call to 
                SSC

 @param SalHandle: Handle to the SAL instance that SSC should use for its
                   operations over the bus. This cannot be NULL.

 @param GetSSCContextCback: Callback to be used by SSC to retrieve the SSC
                            context in which it should operate: caller is
                            expected to provide the memory for this context.

 @param UserData: Any handle that client expects to be passed back with the
                  GetSSCContextCback

 @see WLANSSC_Close

 @return Result of the function call
*/
VOS_STATUS WLANSSC_Open
(
  v_PVOID_t                          SalHandle,
  WLANSSC_HandleType                *pHandle,
  WLANSSC_GetSSCContextCbackType     pfnGetSSCContextCback,
  v_PVOID_t                          UserData
);


/**
 @brief WLANSSC_Close is used to shutdown SSC. Upon return from this call,
 all resources used by SSC will be freed.

 WLANSSC_Open must have been called.

 Also, any SSC context allocated during the SSC_Open() call may be freed only
 after this function returns.

 @param Handle: SSC handle to operate on (returned in WLANSSC_Open)

 @see WLANSSC_Open
 @return Result of the function call
*/
VOS_STATUS WLANSSC_Close
(
  WLANSSC_HandleType Handle
);


/**
 @brief WLANSSC_Start is used to prepare the SSC for upcoming SDIO 
 transactions.
 At the end of this call, SSC is ready to accept any requests for data tx/rx
 as well as for handling interrupts from the chip.

 The target device SIF block is initialized (resetting the hardware, 
 configuring various thresholds and other values).
 At the end of this call, communication with the hardware over SDIO is 
 possible.
 At this point, the Libra interrupts are also enabled.

 This is the point at which the client is required to register all callbacks
 for notification.

 @param Handle: SSC handle to operate on (returned in WLANSSC_Open)

 @param StartParams: Set of parameters required for WLANSSC_Start call

 @param UserData   : Any handle that the client expects to be passed back
                     at the time of invoking callbacks registered in Start()
 
 @see WLANSSC_Stop

 @return Result of the function call
*/
VOS_STATUS WLANSSC_Start
(
  WLANSSC_HandleType           Handle,
  WLANSSC_StartParamsType     *pStartParams,
  v_PVOID_t                    UserData  
);


/**
 @brief WLANSSC_Stop is used to return the SSC to a state wherein no more
 transactions are possible and interrupts are disabled.
 This is the state in which the SSC was prior to WLANSSC_Start being invoked:
 i.e. internal resources are available but the Libra chip has not been 
 enabled.

 @param Handle: SSC handle to operate on (returned in WLANSSC_Open)

 @see WLANSSC_Start
 @return Result of the function call
*/
VOS_STATUS WLANSSC_Stop
(
  WLANSSC_HandleType Handle
);


/**
 @brief WLANSSC_Reset is used to return the SSC to a state wherein no more
 transactions are possible and interrupts are disabled.
 This is the state in which the SSC was prior to WLANSSC_Start being invoked:

 It is the equivalent of WLANSSC_Close() followed by WLANSSC_Open()

 @param Handle: SSC handle to operate on (returned in WLANSSC_Open)

 @see WLANSSC_Stop

 @return Result of the function call
*/
VOS_STATUS WLANSSC_Reset
(
  WLANSSC_HandleType Handle
);


/**
 @brief WLANSSC_StartTransmit is used to notify SSC that there is data
 available for transmission.
 SSC will, when possible, initiate retrieval of data from client using
 the callback registered during WLANSSC_Start()

 @param Handle: SSC handle to operate on (returned in WLANSSC_Open)

 @see WLANSSC_Start

 @return Result of the function call
*/
VOS_STATUS WLANSSC_StartTransmit
(
  WLANSSC_HandleType Handle
);


/**
 @brief WLANSSC_Suspend is used to temporarily disable operations in either
 tx, rx or both directions as specified in the call.

 However interrupts unrelated to tx and rx will be unaffected.

 e.g. if tx is disabled, all tx operations will be terminated and no more
 transmission will be serviced by SSC. However, SSC will continue to receive
 data over the bus and handle the other interrupts unrelated to tx.

 Note that a pending request at the time of suspend might still elicit a 
 response which will be propagated.

 @param Handle: SSC handle to operate on (returned in WLANSSC_Open)
 @param SuspendedFlow: flow to suspend (ALL suspends both tx/rx); bits set
 here will be suspended. Bits not set will be ignored.
 
 @see WLANSSC_Resume

 @return Result of the function call
*/
VOS_STATUS WLANSSC_Suspend
(
  WLANSSC_HandleType       Handle,
  WLANSSC_FlowType         eSuspendedFlow
);


/**
 @brief WLANSSC_Resume is used to resume certain flows in the SSC that have been
 previously suspended.

 If an attempt is made to resume a flow which is already enabled, this will
 return SUCCESS.

 @param Handle: SSC handle to operate on (returned in WLANSSC_Open)

 @param ResumedFlow: flow to resume (ALL resumes both tx/rx); bits set here
 will be resumed. Bits not set here will be ignored.

 @see WLANSSC_Suspend

 @return Result of the function call
*/
VOS_STATUS WLANSSC_Resume
(
  WLANSSC_HandleType       Handle,
  WLANSSC_FlowType         eResumedFlow
);


/**
 @brief WLANSSC_EnableASICInterrupt is used to enable the ASIC interrupt on
 the Libra device.

 Before invoking this function, SSC_Start() must already have been called.

 @param Handle: SSC handle to operate on (returned in WLANSSC_Open)

 @see WLANSSC_DisableASICInterrupt

 @return Result of the function call
*/
VOS_STATUS WLANSSC_EnableASICInterrupt
(
  WLANSSC_HandleType Handle
);

/**
 @brief WLANSSC_EnableASICInterruptEx is used to enable specified ASIC-related interrupts on 
 the Libra device.
 e.g. this may be used to enable both ASIC_INTR and MAC_HOST_INTR
 This will *not* automatically disable previously enabled ASIC interrupts; any disabling
 will only happen via an explicit call to WLANSSC_DisableASICInterruptEx

 The interrupt handler will be invoked only *once* per interrupt status read,
 regardless of how many client-specified interrupt bits are set (e.g whether only
 ASIC_INTR is set, MAC_INTR is set or both are set, the callback will be called
 only once)

 Client is expected to handle clearing the interrupt source directly; SSC
 *will not* clear the interrupts

 Before invoking this function, SSC_Start() must already have been called.

 @param Handle: SSC handle to operate on (returned in WLANSSC_Open)

 @param uInterruptMask: bitmask of interrupts to enable (MUST be per libra.h)

 @see WLANSSC_DisableASICInterruptEx

 @return Result of the function call
*/
VOS_STATUS WLANSSC_EnableASICInterruptEx
(
  WLANSSC_HandleType Handle,
  v_U32_t            uInterruptMask
);

/**
 @brief WLANSSC_DisableASICInterruptEx is used to disable specified ASIC-related interrupts on
 the Libra device.
 e.g. this may be used to disable both ASIC_INTR and MAC_HOST_INTR

 Only the specified interrupts will be disabled; others will remain unchanged

 Before invoking this function, SSC_Start() must already have been called.

 @param Handle: SSC handle to operate on (returned in WLANSSC_Open)

 @param uInterruptMask: bitmask of interrupts to disable (MUST be per libra.h)
                        if a bit is set (i.e. 1) in this field, it will be disabled
                        if a bit is not set (i.e. 0) in this field, it will be ignored

 @see WLANSSC_EnableASICInterruptEx

 @return Result of the function call
*/
VOS_STATUS WLANSSC_DisableASICInterruptEx
(
  WLANSSC_HandleType Handle,
  v_U32_t            uInterruptMask
);

/**
 @brief WLANSSC_DisableASICInterrupt is used to disable the ASIC interrupt on
 the Libra device.

 Before invoking this function, SSC_Start() must already have been called.

 @param Handle: SSC handle to operate on (returned in WLANSSC_Open)

 @see WLANSSC_EnableASICInterrupt

 @return Result of the function call
*/
VOS_STATUS WLANSSC_DisableASICInterrupt
(
  WLANSSC_HandleType Handle
);


/**
 @brief WLANSSC_ProcessMsg is used to receive any messages serialized via
 the VOS services

 NOTE: SHOULD ONLY BE CALLED BY VOS!!

 Before invoking this function, SSC_Start() must already have been called.

 @param pMsg: pointer to the message being serialized via VOS

 @param pContext: the context provided by the VOS services

 @see vos_tx_mq_serialize

 @return Result of the function call
*/
VOS_STATUS WLANSSC_ProcessMsg
(
  v_PVOID_t  pContext,
  vos_msg_t *pMsg
);


/**
 @brief WLANSSC_FreeMsg is used to free any messages pending with
 the VOS services

 NOTE: SHOULD ONLY BE CALLED BY VOS!!

 Before invoking this function, SSC_Start() must already have been called.

 @param pMsg: pointer to the message being serialized via VOS

 @param pContext: the context provided by the VOS services

 @see vos_tx_mq_serialize

 @return Result of the function call
*/
VOS_STATUS WLANSSC_FreeMsg
(
  v_PVOID_t  pContext,
  vos_msg_t *pMsg
);


/**
 @brief WLANSSC_SuspendChip is used to put the Libra chip in standby mode

  After this API is invoked the chip cannot be accessed except after
  ResumeChip

 @param Handle: SSC handle to operate on (returned in WLANSSC_Open)
 
 @see WLANSSC_ResumeChip

 @return Result of the function call
*/
VOS_STATUS WLANSSC_SuspendChip
(
  WLANSSC_HandleType       Handle
);

/**
 @brief WLANSSC_SuspendChip_NoLock is used to put the Libra chip in standby mode
 without acquring SSC lock.
  After this API is invoked the chip cannot be accessed except after
  ResumeChip

 @param Handle: SSC handle to operate on (returned in WLANSSC_Open)
 
 @see WLANSSC_ResumeChip

 @return Result of the function call
*/
VOS_STATUS WLANSSC_SuspendChip_NoLock
(
  WLANSSC_HandleType       Handle
);

/**
 @brief WLANSSC_ResumeChip is used to bring a previously suspended Libra
 chip out of the standby mode and ready for activity.

 @param Handle: SSC handle to operate on (returned in WLANSSC_Open)

 @see WLANSSC_SuspendChip

 @return Result of the function call
*/
VOS_STATUS WLANSSC_ResumeChip
(
  WLANSSC_HandleType       Handle
);


/**
 @brief WLANSSC_DisableGlobalClkGating is used to disable the clock gating
 globally. The feature is expected to help debugging any clock gating related
 issues within the chip

 @param Handle: SSC handle to operate on (returned in WLANSSC_Open)

 @see NONE

 @return Result of the function call
*/
VOS_STATUS WLANSSC_DisableGlobalClkGating
(
  WLANSSC_HandleType       Handle      
);

/**
 @brief WLANSSC_EnableGlobalClkGating is used to enable global clock 
 gating within the chip
 
 @param Handle: SSC control block to operate on

 @see NONE

 @return Result of the function call
*/
VOS_STATUS WLANSSC_EnableGlobalClkGating
(
    WLANSSC_HandleType       Handle
);

#endif /* WLAN_QCT_SSC_H */

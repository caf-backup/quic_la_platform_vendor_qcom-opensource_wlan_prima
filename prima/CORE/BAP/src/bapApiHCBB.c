/*===========================================================================

                      b a p A p i H C B B . C
                                               
  OVERVIEW:
  
  This software unit holds the implementation of the WLAN BAP modules
  Host Controller and Baseband functions.
  
  The functions externalized by this module are to be called ONLY by other 
  WLAN modules (HDD) that properly register with the BAP Layer initially.

  DEPENDENCIES: 

  Are listed for each API below. 
  
  
  Copyright (c) 2008 QUALCOMM Incorporated.
  All Rights Reserved.
  Qualcomm Confidential and Proprietary
===========================================================================*/

/*===========================================================================

                      EDIT HISTORY FOR FILE


  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.


   $Header: /home/labuser/btamp-linux/CORE/BAP/src/bapApiHCBB.c,v 1.2 2010/05/20 19:05:44 labuser Exp labuser $$DateTime$$Author: labuser $


  when        who     what, where, why
----------    ---    --------------------------------------------------------
2008-09-15    jez     Created module

===========================================================================*/

/*----------------------------------------------------------------------------
 * Include Files
 * -------------------------------------------------------------------------*/
#include "vos_trace.h"

/* BT-AMP PAL API header file */ 
#include "bapApi.h" 
#include "bapInternal.h" 

//#define BAP_DEBUG
/*----------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * -------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
 * Type Declarations
 * -------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * Global Data Definitions
 * -------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * Static Variable Definitions
 * -------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * Static Function Declarations and Definitions
 * -------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * Externalized Function Definitions
* -------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * Function Declarations and Documentation
 * -------------------------------------------------------------------------*/


/* Host Controller and Baseband Commands */

/*----------------------------------------------------------------------------

  FUNCTION    WLAN_BAPReset()

  DESCRIPTION 
    Implements the actual HCI Reset command.
    Produces an asynchronous command complete event. Through the 
    command complete callback.  (I.E., (*tpWLAN_BAPEventCB).)

  DEPENDENCIES 
    NA. 

  PARAMETERS 

    IN
    btampHandle: pointer to the BAP handle.  Returned from WLANBAP_GetNewHndl.
   
  RETURN VALUE
    The result code associated with performing the operation  

    VOS_STATUS_SUCCESS:  Success
  
  SIDE EFFECTS 
  
----------------------------------------------------------------------------*/
VOS_STATUS  
WLAN_BAPReset
( 
  ptBtampHandle btampHandle
)
{
    VOS_STATUS  vosStatus;
    tBtampHCI_Event bapHCIEvent; /* This now encodes ALL event types */
    ptBtampContext btampContext = (ptBtampContext) btampHandle;
    /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

    VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "%s: btampHandle value: %x", __FUNCTION__,  btampHandle); 

    /* Validate params */ 
    if (btampHandle == NULL) {
      return VOS_STATUS_E_FAULT;
    }

    /* Form and immediately return the command complete event... */ 
    bapHCIEvent.bapHCIEventCode = BTAMP_TLV_HCI_COMMAND_COMPLETE_EVENT;
    bapHCIEvent.u.btampCommandCompleteEvent.present = 1;
    bapHCIEvent.u.btampCommandCompleteEvent.num_hci_command_packets = 1;
    bapHCIEvent.u.btampCommandCompleteEvent.command_opcode 
        = BTAMP_TLV_HCI_RESET_CMD;
    bapHCIEvent.u.btampCommandCompleteEvent.cc_event.Reset.status 
        = WLANBAP_STATUS_SUCCESS;

    vosStatus = (*btampContext->pBapHCIEventCB) 
        (  
         //btampContext->pHddHdl,   /* this refers to the BSL per connection context */
         btampContext->pAppHdl,   /* this refers the BSL per application context */
         &bapHCIEvent, /* This now encodes ALL event types */
         VOS_FALSE /* Flag to indicate assoc-specific event */ 
        );

    return vosStatus;
} /* WLAN_BAPReset */

/*----------------------------------------------------------------------------

  FUNCTION    WLAN_BAPSetEventMask()

  DESCRIPTION 
    Implements the actual HCI Set Event Mask command.  There is no need for 
    a callback because when this call returns the action has been completed.

  DEPENDENCIES 
    NA. 

  PARAMETERS 

    IN
    btampHandle: pointer to the BAP handle.  Returned from WLANBAP_GetNewHndl.
    pBapHCISetEventMask:  pointer to the "HCI Set Event Mask" Structure.
   
    IN/OUT
    pBapHCIEvent:  Return event value for the command complete event. 
                (The caller of this routine is responsible for sending 
                the Command Complete event up the HCI interface.)
   
  RETURN VALUE
    The result code associated with performing the operation  

    VOS_STATUS_E_FAULT:  pointer to pBapHCISetEventMask is NULL 
    VOS_STATUS_SUCCESS:  Success
  
  SIDE EFFECTS 
  
----------------------------------------------------------------------------*/
VOS_STATUS  
WLAN_BAPSetEventMask
( 
  ptBtampHandle btampHandle,
  tBtampTLVHCI_Set_Event_Mask_Cmd   *pBapHCISetEventMask,
  tpBtampHCI_Event pBapHCIEvent /* This now encodes ALL event types */
                                /* Including Command Complete and Command Status*/
)
{

    return VOS_STATUS_SUCCESS;
} /* WLAN_BAPSetEventMask */

/*----------------------------------------------------------------------------

  FUNCTION    WLAN_BAPFlush()

  DESCRIPTION 
    Implements the actual HCI Flush command
    Produces an asynchronous command complete event. Through the 
    event callback. And an asynchronous Flush occurred event. Also through the 
    event callback.

  DEPENDENCIES 
    NA. 

  PARAMETERS 

    IN
    btampHandle: pointer to the BAP handle.  Returned from WLANBAP_GetNewHndl.
    pBapHCIFlush:  pointer to the "HCI Flush" Structure.
   
  RETURN VALUE
    The result code associated with performing the operation  

    VOS_STATUS_E_FAULT:  pointer to pBapHCIFlush is NULL 
    VOS_STATUS_SUCCESS:  Success
  
  SIDE EFFECTS 
  
----------------------------------------------------------------------------*/
VOS_STATUS  
WLAN_BAPFlush
( 
  ptBtampHandle btampHandle,
  tBtampTLVHCI_Flush_Cmd     *pBapHCIFlush
)
{

    return VOS_STATUS_SUCCESS;
} /* WLAN_BAPFlush */

/*----------------------------------------------------------------------------

  FUNCTION    WLAN_BAPReadConnectionAcceptTimeout()

  DESCRIPTION 
    Implements the actual HCI Read Connection Accept Timeout command.  There 
    is no need for a callback because when this call returns the action 
    has been completed.

  DEPENDENCIES 
    NA. 

  PARAMETERS 

    IN
    btampHandle: pointer to the BAP handle.  Returned from WLANBAP_GetNewHndl.
   
    IN/OUT
    pBapHCIEvent:  Return event value for the command complete event. 
                (The caller of this routine is responsible for sending 
                the Command Complete event up the HCI interface.)
   
  RETURN VALUE
    The result code associated with performing the operation  

    VOS_STATUS_E_FAULT:  pointer to pBapHCIReadConnectionAcceptTimeout is NULL 
    VOS_STATUS_SUCCESS:  Success
  
  SIDE EFFECTS 
  
----------------------------------------------------------------------------*/
VOS_STATUS  
WLAN_BAPReadConnectionAcceptTimeout
( 
  ptBtampHandle btampHandle,
  tpBtampHCI_Event pBapHCIEvent /* This now encodes ALL event types */
                                /* Including "Read" Command Complete */
)
{
    ptBtampContext btampContext = (ptBtampContext) btampHandle;
    /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

    VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH,
               "%s: btampHandle value: %x", __FUNCTION__,  btampHandle);

    /* Validate params */ 
    if ((NULL == btampHandle) || (NULL == pBapHCIEvent))
    {
        VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH,
                   "Invalid input parameters in %s", __FUNCTION__);
        return VOS_STATUS_E_FAULT;
    }

    /* Fill in the parameters for command complete event... */ 
    pBapHCIEvent->bapHCIEventCode = BTAMP_TLV_HCI_COMMAND_COMPLETE_EVENT;
    pBapHCIEvent->u.btampCommandCompleteEvent.present = TRUE;
    pBapHCIEvent->u.btampCommandCompleteEvent.num_hci_command_packets = 1;
    pBapHCIEvent->u.btampCommandCompleteEvent.command_opcode 
        = BTAMP_TLV_HCI_READ_CONNECTION_ACCEPT_TIMEOUT_CMD;
    pBapHCIEvent->u.btampCommandCompleteEvent.cc_event.Read_Connection_Accept_TO.status
        = WLANBAP_STATUS_SUCCESS;
    pBapHCIEvent->u.btampCommandCompleteEvent.cc_event.Read_Connection_Accept_TO.connection_accept_timeout
        = btampContext->bapConnectionAcceptTimerInterval;

    return VOS_STATUS_SUCCESS;
} /* WLAN_BAPReadConnectionAcceptTimeout */

/*----------------------------------------------------------------------------

  FUNCTION    WLAN_BAPWriteConnectionAcceptTimeout()

  DESCRIPTION 
    Implements the actual HCI Write Connection Accept Timeout command.  There 
    is no need for a callback because when this call returns the action 
    has been completed.

  DEPENDENCIES 
    NA. 

  PARAMETERS 

    IN
    btampHandle: pointer to the BAP handle.  Returned from WLANBAP_GetNewHndl.
    pBapHCIWriteConnectionAcceptTimeout:  pointer to the "HCI Connection Accept Timeout" Structure.
   
    IN/OUT
    pBapHCIEvent:  Return event value for the command complete event. 
                (The caller of this routine is responsible for sending 
                the Command Complete event up the HCI interface.)
   
  RETURN VALUE
    The result code associated with performing the operation  

    VOS_STATUS_E_FAULT:  pointer to pBapHCIWriteConnectionAcceptTimeout is NULL 
    VOS_STATUS_SUCCESS:  Success
  
  SIDE EFFECTS 
  
----------------------------------------------------------------------------*/
VOS_STATUS  
WLAN_BAPWriteConnectionAcceptTimeout
( 
  ptBtampHandle btampHandle,
  tBtampTLVHCI_Write_Connection_Accept_Timeout_Cmd   *pBapHCIWriteConnectionAcceptTimeout,
  tpBtampHCI_Event pBapHCIEvent /* This now encodes ALL event types */
                                /* Including Command Complete and Command Status*/
)
{
    ptBtampContext btampContext = (ptBtampContext) btampHandle;
    /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

    VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH,
               "%s: btampHandle value: %x", __FUNCTION__,  btampHandle);

    /* Validate params */ 
    if ((NULL == btampHandle) || (NULL == pBapHCIWriteConnectionAcceptTimeout)
        || (NULL == pBapHCIEvent))
    {
        VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH,
                   "Invalid input parameters in %s", __FUNCTION__);
        return VOS_STATUS_E_FAULT;
    }

    /* Validate the allowed timeout interval range */
    if ((pBapHCIWriteConnectionAcceptTimeout->connection_accept_timeout >
         WLANBAP_CON_ACCEPT_TIMEOUT_MAX_RANGE) || 
        (pBapHCIWriteConnectionAcceptTimeout->connection_accept_timeout <
         WLANBAP_CON_ACCEPT_TIMEOUT_MIN_RANGE))
    {
        VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH,
                   "Out of range for connection accept timeout parameters in %s",
                   __FUNCTION__);
        pBapHCIEvent->u.btampCommandCompleteEvent.cc_event.Write_Connection_Accept_TO.status
            = WLANBAP_ERROR_INVALID_HCI_CMND_PARAM;
    }
    else
    {
        /* Save the Physical link connection accept timeout value */
        btampContext->bapConnectionAcceptTimerInterval = 
            pBapHCIWriteConnectionAcceptTimeout->connection_accept_timeout;

        /* Return status for command complete event */
        pBapHCIEvent->u.btampCommandCompleteEvent.cc_event.Write_Connection_Accept_TO.status
            = WLANBAP_STATUS_SUCCESS;
    }

    /* Fill in the parameters for command complete event... */ 
    pBapHCIEvent->bapHCIEventCode = BTAMP_TLV_HCI_COMMAND_COMPLETE_EVENT;
    pBapHCIEvent->u.btampCommandCompleteEvent.present = TRUE;
    pBapHCIEvent->u.btampCommandCompleteEvent.num_hci_command_packets = 1;
    pBapHCIEvent->u.btampCommandCompleteEvent.command_opcode 
        = BTAMP_TLV_HCI_WRITE_CONNECTION_ACCEPT_TIMEOUT_CMD;

    return VOS_STATUS_SUCCESS;
} /* WLAN_BAPWriteConnectionAcceptTimeout */


/*----------------------------------------------------------------------------

  FUNCTION    WLAN_BAPReadLinkSupervisionTimeout()

  DESCRIPTION 
    Implements the actual HCI Read Link Supervision Timeout command.  There 
    is no need for a callback because when this call returns the action 
    has been completed.

  DEPENDENCIES 
    NA. 

  PARAMETERS 

    IN
    btampHandle: pointer to the BAP handle.  Returned from WLANBAP_GetNewHndl.
   
    IN/OUT
    pBapHCIEvent:  Return event value for the command complete event. 
                (The caller of this routine is responsible for sending 
                the Command Complete event up the HCI interface.)
   
  RETURN VALUE
    The result code associated with performing the operation  

    VOS_STATUS_E_FAULT:  pointer to pBapHCIReadLinkSupervisionTimeout is NULL 
    VOS_STATUS_SUCCESS:  Success
  
  SIDE EFFECTS 
  
----------------------------------------------------------------------------*/
VOS_STATUS  
WLAN_BAPReadLinkSupervisionTimeout
( 
  ptBtampHandle btampHandle,
  /* Only 8 bits (phy_link_handle) of this log_link_handle are valid. */
  tBtampTLVHCI_Read_Link_Supervision_Timeout_Cmd *pBapHCIReadLinkSupervisionTimeout,
  tpBtampHCI_Event pBapHCIEvent /* This now encodes ALL event types */
                                /* Including "Read" Command Complete*/
)
{
    ptBtampContext btampContext = (ptBtampContext) btampHandle;
    v_U8_t         phyLinkHandle;
    /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

    VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH,
               "%s: btampHandle value: %x", __FUNCTION__,  btampHandle);

    /* Validate params */ 
    if ((NULL == btampHandle) || (NULL == pBapHCIReadLinkSupervisionTimeout) ||
        (NULL == pBapHCIEvent))
    {
        VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH,
                   "Invalid input parameters in %s", __FUNCTION__);
        return VOS_STATUS_E_FAULT;
    }

    /* Validate the phyiscal link handle extracted from
       logical link handle (lower byte valid) */
    phyLinkHandle = (v_U8_t) pBapHCIReadLinkSupervisionTimeout->log_link_handle;

    if (phyLinkHandle != btampContext->phy_link_handle)
    {
        VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH,
                   "Invalid Physical link handle in %s", __FUNCTION__);
        pBapHCIEvent->u.btampCommandCompleteEvent.cc_event.Read_Link_Supervision_TO.link_supervision_timeout
            = 0x00; /* Invalid value */
        pBapHCIEvent->u.btampCommandCompleteEvent.cc_event.Read_Link_Supervision_TO.log_link_handle
            = pBapHCIReadLinkSupervisionTimeout->log_link_handle;
        pBapHCIEvent->u.btampCommandCompleteEvent.cc_event.Read_Link_Supervision_TO.status
            = WLANBAP_ERROR_INVALID_HCI_CMND_PARAM;
    }
    else
    {
        pBapHCIEvent->u.btampCommandCompleteEvent.cc_event.Read_Link_Supervision_TO.link_supervision_timeout
            = btampContext->bapLinkSupervisionTimerInterval;
        pBapHCIEvent->u.btampCommandCompleteEvent.cc_event.Read_Link_Supervision_TO.log_link_handle
            = pBapHCIReadLinkSupervisionTimeout->log_link_handle;
        pBapHCIEvent->u.btampCommandCompleteEvent.cc_event.Read_Link_Supervision_TO.status
            = WLANBAP_STATUS_SUCCESS;
    }

    /* Fill in the parameters for command complete event... */ 
    pBapHCIEvent->bapHCIEventCode = BTAMP_TLV_HCI_COMMAND_COMPLETE_EVENT;
    pBapHCIEvent->u.btampCommandCompleteEvent.present = TRUE;
    pBapHCIEvent->u.btampCommandCompleteEvent.num_hci_command_packets = 1;
    pBapHCIEvent->u.btampCommandCompleteEvent.command_opcode 
        = BTAMP_TLV_HCI_READ_LINK_SUPERVISION_TIMEOUT_CMD;

    return VOS_STATUS_SUCCESS;
} /* WLAN_BAPReadLinkSupervisionTimeout */

/*----------------------------------------------------------------------------

  FUNCTION    WLAN_BAPWriteLinkSupervisionTimeout()

  DESCRIPTION 
    Implements the actual HCI Write Link Supervision Timeout command.  There 
    is no need for a callback because when this call returns the action 
    has been completed.

  DEPENDENCIES 
    NA. 

  PARAMETERS 

    IN
    btampHandle: pointer to the BAP handle.  Returned from WLANBAP_GetNewHndl.
    pBapHCIWriteLinkSupervisionTimeout:  pointer to the "HCI Link Supervision Timeout" Structure.
   
    IN/OUT
    pBapHCIEvent:  Return event value for the command complete event. 
                (The caller of this routine is responsible for sending 
                the Command Complete event up the HCI interface.)
   
  RETURN VALUE
    The result code associated with performing the operation  

    VOS_STATUS_E_FAULT:  pointer to pBapHCIWriteLinkSupervisionTimeout is NULL 
    VOS_STATUS_SUCCESS:  Success
  
  SIDE EFFECTS 
  
----------------------------------------------------------------------------*/
VOS_STATUS  
WLAN_BAPWriteLinkSupervisionTimeout
( 
  ptBtampHandle btampHandle,
  tBtampTLVHCI_Write_Link_Supervision_Timeout_Cmd   *pBapHCIWriteLinkSupervisionTimeout,
  tpBtampHCI_Event pBapHCIEvent /* This now encodes ALL event types */
                                /* Including Command Complete and Command Status*/
)
{
    ptBtampContext btampContext = (ptBtampContext) btampHandle;
    v_U8_t         phyLinkHandle;
    /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

    VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH,
               "%s: btampHandle value: %x", __FUNCTION__,  btampHandle);

    /* Validate params */ 
    if ((NULL == btampHandle) || (NULL == pBapHCIWriteLinkSupervisionTimeout) ||
        (NULL == pBapHCIEvent))
    {
        VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH,
                   "Invalid input parameters in %s", __FUNCTION__);
        return VOS_STATUS_E_FAULT;
    }

    /* Validate the phyiscal link handle extracted from
       logical link handle (lower byte valid) */
    phyLinkHandle = (v_U8_t) pBapHCIWriteLinkSupervisionTimeout->log_link_handle;

    if (phyLinkHandle != btampContext->phy_link_handle)
    {
        VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH,
                   "Invalid Physical link handle in %s", __FUNCTION__);
        pBapHCIEvent->u.btampCommandCompleteEvent.cc_event.Write_Link_Supervision_TO.log_link_handle
            = pBapHCIWriteLinkSupervisionTimeout->log_link_handle;
        pBapHCIEvent->u.btampCommandCompleteEvent.cc_event.Write_Link_Supervision_TO.status
            = WLANBAP_ERROR_INVALID_HCI_CMND_PARAM;
    }
    else
    {
        /* Save the LS timeout interval */
        btampContext->bapLinkSupervisionTimerInterval =
            pBapHCIWriteLinkSupervisionTimeout->link_supervision_timeout;

        pBapHCIEvent->u.btampCommandCompleteEvent.cc_event.Write_Link_Supervision_TO.log_link_handle
            = pBapHCIWriteLinkSupervisionTimeout->log_link_handle;
        pBapHCIEvent->u.btampCommandCompleteEvent.cc_event.Write_Link_Supervision_TO.status
            = WLANBAP_STATUS_SUCCESS;
    }

    /* Fill in the parameters for command complete event... */ 
    pBapHCIEvent->bapHCIEventCode = BTAMP_TLV_HCI_COMMAND_COMPLETE_EVENT;
    pBapHCIEvent->u.btampCommandCompleteEvent.present = TRUE;
    pBapHCIEvent->u.btampCommandCompleteEvent.num_hci_command_packets = 1;
    pBapHCIEvent->u.btampCommandCompleteEvent.command_opcode 
        = BTAMP_TLV_HCI_WRITE_LINK_SUPERVISION_TIMEOUT_CMD;

    return VOS_STATUS_SUCCESS;
} /* WLAN_BAPWriteLinkSupervisionTimeout */

/* v3.0 Host Controller and Baseband Commands */


/*----------------------------------------------------------------------------

  FUNCTION    WLAN_BAPReadLogicalLinkAcceptTimeout()

  DESCRIPTION 
    Implements the actual HCI Read Logical Link Accept Timeout command.  There 
    is no need for a callback because when this call returns the action 
    has been completed.

  DEPENDENCIES 
    NA. 

  PARAMETERS 

    IN
    btampHandle: pointer to the BAP handle.  Returned from WLANBAP_GetNewHndl.
   
    IN/OUT
    pBapHCIEvent:  Return event value for the command complete event. 
                (The caller of this routine is responsible for sending 
                the Command Complete event up the HCI interface.)
   
  RETURN VALUE
    The result code associated with performing the operation  

    VOS_STATUS_E_FAULT:  pointer to pBapHCIReadLogicalLinkAcceptTimeout is NULL 
    VOS_STATUS_SUCCESS:  Success
  
  SIDE EFFECTS 
  
----------------------------------------------------------------------------*/
VOS_STATUS  
WLAN_BAPReadLogicalLinkAcceptTimeout
( 
  ptBtampHandle btampHandle,
  tpBtampHCI_Event pBapHCIEvent /* This now encodes ALL event types */
                                /* Including "Read" Command Complete*/
)
{
    ptBtampContext btampContext = (ptBtampContext) btampHandle;
    /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

    VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH,
               "%s: btampHandle value: %x", __FUNCTION__,  btampHandle);

    /* Validate params */ 
    if ((NULL == btampHandle) || (NULL == pBapHCIEvent))
    {
        VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH,
                   "Invalid input parameters in %s", __FUNCTION__);
        return VOS_STATUS_E_FAULT;
    }

    /* Fill in the parameters for command complete event... */ 
    pBapHCIEvent->bapHCIEventCode = BTAMP_TLV_HCI_COMMAND_COMPLETE_EVENT;
    pBapHCIEvent->u.btampCommandCompleteEvent.present = TRUE;
    pBapHCIEvent->u.btampCommandCompleteEvent.num_hci_command_packets = 1;
    pBapHCIEvent->u.btampCommandCompleteEvent.command_opcode 
        = BTAMP_TLV_HCI_READ_LOGICAL_LINK_ACCEPT_TIMEOUT_CMD;
    pBapHCIEvent->u.btampCommandCompleteEvent.cc_event.Read_Logical_Link_Accept_TO.status
        = WLANBAP_STATUS_SUCCESS;
    pBapHCIEvent->u.btampCommandCompleteEvent.cc_event.Read_Logical_Link_Accept_TO.logical_link_accept_timeout
        = btampContext->bapLogicalLinkAcceptTimerInterval;

    return VOS_STATUS_SUCCESS;
} /* WLAN_BAPReadLogicalLinkAcceptTimeout */

/*----------------------------------------------------------------------------

  FUNCTION    WLAN_BAPWriteLogicalLinkAcceptTimeout()

  DESCRIPTION 
    Implements the actual HCI Write Logical Link Accept Timeout command.  There 
    is no need for a callback because when this call returns the action 
    has been completed.

  DEPENDENCIES 
    NA. 

  PARAMETERS 

    IN
    btampHandle: pointer to the BAP handle.  Returned from WLANBAP_GetNewHndl.
    pBapHCIWriteLogicalLinkAcceptTimeout:  pointer to the "HCI Logical Link Accept Timeout" Structure.
   
    IN/OUT
    pBapHCIEvent:  Return event value for the command complete event. 
                (The caller of this routine is responsible for sending 
                the Command Complete event up the HCI interface.)
   
  RETURN VALUE
    The result code associated with performing the operation  

    VOS_STATUS_E_FAULT:  pointer to pBapHCIWriteLogicalLinkAcceptTimeout is NULL 
    VOS_STATUS_SUCCESS:  Success
  
  SIDE EFFECTS 
  
----------------------------------------------------------------------------*/
VOS_STATUS  
WLAN_BAPWriteLogicalLinkAcceptTimeout
( 
  ptBtampHandle btampHandle,
  tBtampTLVHCI_Write_Logical_Link_Accept_Timeout_Cmd   *pBapHCIWriteLogicalLinkAcceptTimeout,
  tpBtampHCI_Event pBapHCIEvent /* This now encodes ALL event types */
                                /* Including Command Complete and Command Status*/
)
{
    ptBtampContext btampContext = (ptBtampContext) btampHandle;
    /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

    VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH,
               "%s: btampHandle value: %x", __FUNCTION__,  btampHandle);

    /* Validate params */ 
    if ((NULL == btampHandle) || (NULL == pBapHCIWriteLogicalLinkAcceptTimeout)
        || (NULL == pBapHCIEvent))
    {
        VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH,
                   "Invalid input parameters in %s", __FUNCTION__);
        return VOS_STATUS_E_FAULT;
    }

    /* Validate the allowed timeout interval range */
    if ((pBapHCIWriteLogicalLinkAcceptTimeout->logical_link_accept_timeout >
         WLANBAP_CON_ACCEPT_TIMEOUT_MAX_RANGE) || 
        (pBapHCIWriteLogicalLinkAcceptTimeout->logical_link_accept_timeout <
         WLANBAP_CON_ACCEPT_TIMEOUT_MIN_RANGE))
    {
        VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH,
                   "Out of range for logocal connection accept timeout parameters in %s",
                   __FUNCTION__);
        pBapHCIEvent->u.btampCommandCompleteEvent.cc_event.Write_Logical_Link_Accept_TO.status
            = WLANBAP_ERROR_INVALID_HCI_CMND_PARAM;
    }
    else
    {
        /* Save the Physical link connection accept timeout value */
        btampContext->bapLogicalLinkAcceptTimerInterval = 
            pBapHCIWriteLogicalLinkAcceptTimeout->logical_link_accept_timeout;

        /* Return status for command complete event */
        pBapHCIEvent->u.btampCommandCompleteEvent.cc_event.Write_Logical_Link_Accept_TO.status
            = WLANBAP_STATUS_SUCCESS;
    }

    /* Fill in the parameters for command complete event... */ 
    pBapHCIEvent->bapHCIEventCode = BTAMP_TLV_HCI_COMMAND_COMPLETE_EVENT;
    pBapHCIEvent->u.btampCommandCompleteEvent.present = TRUE;
    pBapHCIEvent->u.btampCommandCompleteEvent.num_hci_command_packets = 1;
    pBapHCIEvent->u.btampCommandCompleteEvent.command_opcode 
        = BTAMP_TLV_HCI_WRITE_LOGICAL_LINK_ACCEPT_TIMEOUT_CMD;

    return VOS_STATUS_SUCCESS;
} /* WLAN_BAPWriteLogicalLinkAcceptTimeout */

/*----------------------------------------------------------------------------

  FUNCTION    WLAN_BAPSetEventMaskPage2()

  DESCRIPTION 
    Implements the actual HCI Set Event Mask Page 2 command.  There is no need for 
    a callback because when this call returns the action has been completed.

  DEPENDENCIES 
    NA. 

  PARAMETERS 

    IN
    btampHandle: pointer to the BAP handle.  Returned from WLANBAP_GetNewHndl.
    pBapHCISetEventMaskPage2:  pointer to the "HCI Set Event Mask Page 2" Structure.
   
    IN/OUT
    pBapHCIEvent:  Return event value for the command complete event. 
                (The caller of this routine is responsible for sending 
                the Command Complete event up the HCI interface.)
   
  RETURN VALUE
    The result code associated with performing the operation  

    VOS_STATUS_E_FAULT:  pointer to pBapHCISetEventMaskPage2 is NULL 
    VOS_STATUS_SUCCESS:  Success
  
  SIDE EFFECTS 
  
----------------------------------------------------------------------------*/
VOS_STATUS  
WLAN_BAPSetEventMaskPage2
( 
  ptBtampHandle btampHandle,
  tBtampTLVHCI_Set_Event_Mask_Page_2_Cmd   *pBapHCISetEventMaskPage2,
  tpBtampHCI_Event pBapHCIEvent /* This now encodes ALL event types */
                                /* Including Command Complete and Command Status*/
)
{
    ptBtampContext btampContext = (ptBtampContext) btampHandle;
    /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

    VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH,
               "%s: btampHandle value: %x", __FUNCTION__,  btampHandle);

    /* Validate params */ 
    if ((NULL == btampHandle) || (NULL == pBapHCISetEventMaskPage2)
        || (NULL == pBapHCIEvent))
    {
        VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH,
                   "Invalid input parameters in %s", __FUNCTION__);
        return VOS_STATUS_E_FAULT;
    }


    /* Save away the event mask */
    vos_mem_copy(  
            btampContext->event_mask_page_2, 
            pBapHCISetEventMaskPage2->event_mask_page_2, 
            8 );

    /* Return status for command complete event */
    pBapHCIEvent->u.btampCommandCompleteEvent.cc_event.Set_Event_Mask_Page_2.status
        = WLANBAP_STATUS_SUCCESS;

    /* Fill in the parameters for command complete event... */ 
    pBapHCIEvent->bapHCIEventCode = BTAMP_TLV_HCI_COMMAND_COMPLETE_EVENT;
    pBapHCIEvent->u.btampCommandCompleteEvent.present = TRUE;
    pBapHCIEvent->u.btampCommandCompleteEvent.num_hci_command_packets = 1;
    pBapHCIEvent->u.btampCommandCompleteEvent.command_opcode 
        = BTAMP_TLV_HCI_SET_EVENT_MASK_PAGE_2_CMD;

    return VOS_STATUS_SUCCESS;
} /* WLAN_BAPSetEventMaskPage2 */

/*----------------------------------------------------------------------------

  FUNCTION    WLAN_BAPReadLocationData()

  DESCRIPTION 
    Implements the actual HCI Read Location Data command.  There 
    is no need for a callback because when this call returns the action 
    has been completed.

  DEPENDENCIES 
    NA. 

  PARAMETERS 

    IN
    btampHandle: pointer to the BAP handle.  Returned from WLANBAP_GetNewHndl.
   
    IN/OUT
    pBapHCIEvent:  Return event value for the command complete event. 
                (The caller of this routine is responsible for sending 
                the Command Complete event up the HCI interface.)
   
  RETURN VALUE
    The result code associated with performing the operation  

    VOS_STATUS_E_FAULT:  pointer to pBapHCIReadLocationData is NULL 
    VOS_STATUS_SUCCESS:  Success
  
  SIDE EFFECTS 
  
----------------------------------------------------------------------------*/
VOS_STATUS  
WLAN_BAPReadLocationData
( 
  ptBtampHandle btampHandle,
  tpBtampHCI_Event pBapHCIEvent /* This now encodes ALL event types */
                                /* Including "Read" Command Complete*/
)
{

    return VOS_STATUS_SUCCESS;
} /* WLAN_BAPReadLocationData */

/*----------------------------------------------------------------------------

  FUNCTION    WLAN_BAPWriteLocationData()

  DESCRIPTION 
    Implements the actual HCI Write Location Data command.  There 
    is no need for a callback because when this call returns the action 
    has been completed.

  DEPENDENCIES 
    NA. 

  PARAMETERS 

    IN
    btampHandle: pointer to the BAP handle.  Returned from WLANBAP_GetNewHndl.
    pBapHCIWriteLocationData:  pointer to the "HCI Write Location Data" Structure.
   
    IN/OUT
    pBapHCIEvent:  Return event value for the command complete event. 
                (The caller of this routine is responsible for sending 
                the Command Complete event up the HCI interface.)
   
  RETURN VALUE
    The result code associated with performing the operation  

    VOS_STATUS_E_FAULT:  pointer to pBapHCIWriteLocationData is NULL 
    VOS_STATUS_SUCCESS:  Success
  
  SIDE EFFECTS 
  
----------------------------------------------------------------------------*/
VOS_STATUS  
WLAN_BAPWriteLocationData
( 
  ptBtampHandle btampHandle,
  tBtampTLVHCI_Write_Location_Data_Cmd   *pBapHCIWriteLocationData,
  tpBtampHCI_Event pBapHCIEvent /* This now encodes ALL event types */
                                /* Including Command Complete and Command Status*/
)
{
    /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

    VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH,
               "%s: btampHandle value: %x", __FUNCTION__,  btampHandle);

    /* Validate params */ 
    if ((NULL == btampHandle) || (NULL == pBapHCIWriteLocationData)
        || (NULL == pBapHCIEvent))
    {
        VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH,
                   "Invalid input parameters in %s", __FUNCTION__);
        return VOS_STATUS_E_FAULT;
    }

    /* TODO: save away the location data
    btampContext->bapLogicalLinkAcceptTimerInterval = 
        pBapHCIWriteLogicalLinkAcceptTimeout->logical_link_accept_timeout;
    */

    /* Return status for command complete event */
    pBapHCIEvent->u.btampCommandCompleteEvent.cc_event.Write_Location_Data.status
        = WLANBAP_STATUS_SUCCESS;

    /* Fill in the parameters for command complete event... */ 
    pBapHCIEvent->bapHCIEventCode = BTAMP_TLV_HCI_COMMAND_COMPLETE_EVENT;
    pBapHCIEvent->u.btampCommandCompleteEvent.present = TRUE;
    pBapHCIEvent->u.btampCommandCompleteEvent.num_hci_command_packets = 1;
    pBapHCIEvent->u.btampCommandCompleteEvent.command_opcode 
        = BTAMP_TLV_HCI_WRITE_LOCATION_DATA_CMD;

    return VOS_STATUS_SUCCESS;
} /* WLAN_BAPWriteLocationData */


/*----------------------------------------------------------------------------

  FUNCTION    WLAN_BAPReadFlowControlMode()

  DESCRIPTION 
    Implements the actual HCI Read Flow Control Mode command.  There 
    is no need for a callback because when this call returns the action 
    has been completed.

  DEPENDENCIES 
    NA. 

  PARAMETERS 

    IN
    btampHandle: pointer to the BAP handle.  Returned from WLANBAP_GetNewHndl.
   
    IN/OUT
    pBapHCIEvent:  Return event value for the command complete event. 
                (The caller of this routine is responsible for sending 
                the Command Complete event up the HCI interface.)
   
  RETURN VALUE
    The result code associated with performing the operation  

    VOS_STATUS_E_FAULT:  pointer to pBapHCIReadFlowControlMode is NULL 
    VOS_STATUS_SUCCESS:  Success
  
  SIDE EFFECTS 
  
----------------------------------------------------------------------------*/
VOS_STATUS  
WLAN_BAPReadFlowControlMode
( 
  ptBtampHandle btampHandle,
  tpBtampHCI_Event pBapHCIEvent /* This now encodes ALL event types */
                                /* Including "Read" Command Complete*/
)
{
    /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

    VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH,
               "%s: btampHandle value: %x", __FUNCTION__,  btampHandle);

    /* Validate params */ 
    if ((NULL == btampHandle) || (NULL == pBapHCIEvent))
    {
        VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH,
                   "Invalid input parameters in %s", __FUNCTION__);
        return VOS_STATUS_E_FAULT;
    }

    /* Fill in the parameters for command complete event... */ 
    pBapHCIEvent->bapHCIEventCode = BTAMP_TLV_HCI_COMMAND_COMPLETE_EVENT;
    pBapHCIEvent->u.btampCommandCompleteEvent.present = TRUE;
    pBapHCIEvent->u.btampCommandCompleteEvent.num_hci_command_packets = 1;
    pBapHCIEvent->u.btampCommandCompleteEvent.command_opcode 
        = BTAMP_TLV_HCI_READ_FLOW_CONTROL_MODE_CMD;
    pBapHCIEvent->u.btampCommandCompleteEvent.cc_event.Read_Flow_Control_Mode.status
        = WLANBAP_STATUS_SUCCESS;
    pBapHCIEvent->u.btampCommandCompleteEvent.cc_event.Read_Flow_Control_Mode.flow_control_mode
        = 0x00; // WLANBAP_FLOW_CONTROL_MODE_PACKET_BASED;

    return VOS_STATUS_SUCCESS;
} /* WLAN_BAPReadFlowControlMode */

/*----------------------------------------------------------------------------

  FUNCTION    WLAN_BAPWriteFlowControlMode()

  DESCRIPTION 
    Implements the actual HCI Write Flow Control Mode command.  There 
    is no need for a callback because when this call returns the action 
    has been completed.

  DEPENDENCIES 
    NA. 

  PARAMETERS 

    IN
    btampHandle: pointer to the BAP handle.  Returned from WLANBAP_GetNewHndl.
    pBapHCIWriteFlowControlMode:  pointer to the "HCI Write Flow Control Mode" Structure.
   
    IN/OUT
    pBapHCIEvent:  Return event value for the command complete event. 
                (The caller of this routine is responsible for sending 
                the Command Complete event up the HCI interface.)
   
  RETURN VALUE
    The result code associated with performing the operation  

    VOS_STATUS_E_FAULT:  pointer to pBapHCIWriteFlowControlMode is NULL 
    VOS_STATUS_SUCCESS:  Success
  
  SIDE EFFECTS 
  
----------------------------------------------------------------------------*/
VOS_STATUS  
WLAN_BAPWriteFlowControlMode
( 
  ptBtampHandle btampHandle,
  tBtampTLVHCI_Write_Flow_Control_Mode_Cmd   *pBapHCIWriteFlowControlMode,
  tpBtampHCI_Event pBapHCIEvent /* This now encodes ALL event types */
                                /* Including Command Complete and Command Status*/
)
{

    return VOS_STATUS_SUCCESS;
} /* WLAN_BAPWriteFlowControlMode */

/*----------------------------------------------------------------------------

  FUNCTION    WLAN_BAPReadBestEffortFlushTimeout()

  DESCRIPTION 
    Implements the actual HCI Read Best Effort Flush Timeout command.  There 
    is no need for a callback because when this call returns the action 
    has been completed.

  DEPENDENCIES 
    NA. 

  PARAMETERS 

    IN
    btampHandle: pointer to the BAP handle.  Returned from WLANBAP_GetNewHndl.
   
    IN/OUT
    pBapHCIEvent:  Return event value for the command complete event. 
                (The caller of this routine is responsible for sending 
                the Command Complete event up the HCI interface.)
   
  RETURN VALUE
    The result code associated with performing the operation  

    VOS_STATUS_E_FAULT:  pointer to pBapHCIReadBEFlushTO is NULL 
    VOS_STATUS_SUCCESS:  Success
  
  SIDE EFFECTS 
  
----------------------------------------------------------------------------*/
VOS_STATUS  
WLAN_BAPReadBestEffortFlushTimeout
( 
  ptBtampHandle btampHandle,
  /* The log_link_hanlde identifies which logical link's BE TO*/
  tBtampTLVHCI_Read_Best_Effort_Flush_Timeout_Cmd   *pBapHCIReadBEFlushTO,
  tpBtampHCI_Event pBapHCIEvent /* This now encodes ALL event types */
                                /* Including "Read" Command Complete*/
)
{

    return VOS_STATUS_SUCCESS;
} /* WLAN_BAPReadBestEffortFlushTimeout */

/*----------------------------------------------------------------------------

  FUNCTION    WLAN_BAPWriteBestEffortFlushTimeout()

  DESCRIPTION 
    Implements the actual HCI Write Best Effort Flush TO command.  There 
    is no need for a callback because when this call returns the action 
    has been completed.

  DEPENDENCIES 
    NA. 

  PARAMETERS 

    IN
    btampHandle: pointer to the BAP handle.  Returned from WLANBAP_GetNewHndl.
    pBapHCIWriteBEFlushTO:  pointer to the "HCI Write BE Flush TO" Structure.
   
    IN/OUT
    pBapHCIEvent:  Return event value for the command complete event. 
                (The caller of this routine is responsible for sending 
                the Command Complete event up the HCI interface.)
   
  RETURN VALUE
    The result code associated with performing the operation  

    VOS_STATUS_E_FAULT:  pointer to pBapHCIWriteBEFlushTO is NULL 
    VOS_STATUS_SUCCESS:  Success
  
  SIDE EFFECTS 
  
----------------------------------------------------------------------------*/
VOS_STATUS  
WLAN_BAPWriteBestEffortFlushTimeout
( 
  ptBtampHandle btampHandle,
  tBtampTLVHCI_Write_Best_Effort_Flush_Timeout_Cmd   *pBapHCIWriteBEFlushTO,
  tpBtampHCI_Event pBapHCIEvent /* This now encodes ALL event types */
                                /* Including Command Complete and Command Status*/
)
{

    return VOS_STATUS_SUCCESS;
} /* WLAN_BAPWriteBestEffortFlushTimeout */


/*----------------------------------------------------------------------------

  FUNCTION    WLAN_BAPSetShortRangeMode()

  DESCRIPTION 
    Implements the actual HCI Set Short Range Mode command.  There is no need for 
    a callback because when this call returns the action has been completed.

  DEPENDENCIES 
    NA. 

  PARAMETERS 

    IN
    btampHandle: pointer to the BAP handle.  Returned from WLANBAP_GetNewHndl.
    pBapHCIShortRangeMode:  pointer to the "HCI Set Short Range Mode" Structure.
   
    IN/OUT
    pBapHCIEvent:  Return event value for the command complete event. 
                (The caller of this routine is responsible for sending 
                the Command Complete event up the HCI interface.)
   
  RETURN VALUE
    The result code associated with performing the operation  

    VOS_STATUS_E_FAULT:  pointer to pBapHCIShortRangeMode is NULL 
    VOS_STATUS_SUCCESS:  Success
  
  SIDE EFFECTS 
  
----------------------------------------------------------------------------*/
VOS_STATUS  
WLAN_BAPSetShortRangeMode
( 
  ptBtampHandle btampHandle,
  tBtampTLVHCI_Set_Short_Range_Mode_Cmd   *pBapHCIShortRangeMode,
  tpBtampHCI_Event pBapHCIEvent /* This now encodes ALL event types */
                                /* Including Command Complete and Command Status*/
)
{

    return VOS_STATUS_SUCCESS;
} /* WLAN_BAPSetShortRangeMode */

/*----------------------------------------------------------------------------

  DESCRIPTION   
    Callback registered with TL for BAP, this is required in order for
    TL to inform BAP, that the flush operation requested has been completed. 
    
    The registered reception callback is being triggered by TL whenever a 
    frame SIR_TL_HAL_FLUSH_AC_RSP is received by TL from HAL.

  PARAMETERS 

    IN
    pvosGCtx:       pointer to the global vos context; a handle to TL's 
                    or SME's control block can be extracted from its context 
    ucStaId:        station identifier for the requested value
    ucTid:          identifier of the tspec 
    status:         status of the Flush operation
  
  RETURN VALUE 
    The result code associated with performing the operation

----------------------------------------------------------------------------*/
VOS_STATUS WLANBAP_TLFlushCompCallback
( 
  v_PVOID_t     pvosGCtx,
  v_U8_t        ucStaId, 
  v_U8_t        ucTID, 
  v_U8_t        status
)
{

    return VOS_STATUS_SUCCESS;
} // WLANBAP_TLFlushCompCallback


/* End of v3.0 Host Controller and Baseband Commands */


 



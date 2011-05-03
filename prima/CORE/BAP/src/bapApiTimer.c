/*===========================================================================

                      b a p A p i T i m e r . C
                                               
  OVERVIEW:
  
  This software unit holds the implementation of the timer routines
  required by the WLAN BAP module.  
  
  The functions provide by this module are called by the rest of 
  the BT-AMP PAL module.

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


   $Header: /cygdrive/e/Builds/M7201JSDCAAPAD52240B/WM/platform/msm7200/Src/Drivers/SD/ClientDrivers/WLAN/QCT/CORE/BAP/src/bapApiTimer.c,v 1.1 2008/11/21 20:30:04 jzmuda Exp jzmuda $$DateTime$$Author: jzmuda $


  when        who     what, where, why
----------    ---    --------------------------------------------------------
2008-10-23    jez     Created module

===========================================================================*/

/*----------------------------------------------------------------------------
 * Include Files
 * -------------------------------------------------------------------------*/
// I think this pulls in everything
#include "vos_types.h"
#include "bapApiTimer.h"

//#define BAP_DEBUG

/*----------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * -------------------------------------------------------------------------*/
#define WLAN_BAP_TX_PKT_MONITOR_TIME 100

/*----------------------------------------------------------------------------
 * Type Declarations
 * -------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * Global Data Definitions
 * -------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 *  External declarations for global context 
 * -------------------------------------------------------------------------*/
#if 1
//*BT-AMP packet LLC OUI value*/
static const v_U8_t WLANBAP_BT_AMP_OUI[] =  {0x00, 0x19, 0x58 };

#endif

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

/*----------------------------------------------------------------------------
 * Utility Function implementations 
 * -------------------------------------------------------------------------*/

/*==========================================================================

  FUNCTION    WLANBAP_InitConnectionAcceptTimer

  DESCRIPTION 
    Initialize the Connection Accept Timer.
    
  DEPENDENCIES 
    
  PARAMETERS 

    IN
    pBtampCtx:   pointer to the BAP control block
   
  RETURN VALUE
    The result code associated with performing the operation  

    VOS_STATUS_E_FAULT:  access would cause a page fault  
    VOS_STATUS_SUCCESS:  Everything is good :) 

  SIDE EFFECTS 
  
============================================================================*/
VOS_STATUS 
WLANBAP_InitConnectionAcceptTimer
( 
  ptBtampContext  pBtampCtx
)
{
  VOS_STATUS  vosStatus = VOS_STATUS_SUCCESS;

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/


  /*------------------------------------------------------------------------
    Sanity check BAP control block 
   ------------------------------------------------------------------------*/

  if ( NULL == pBtampCtx ) 
  {
    VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_ERROR,
                 "Invalid BAP pointer in %s", __FILE__);
    return VOS_STATUS_E_FAULT;
  }

  /*Initialize the timer */
  vosStatus = vos_timer_init( 
          &pBtampCtx->bapConnectionAcceptTimer,
          VOS_TIMER_TYPE_SW, /* use this type */
          WLANBAP_ConnectionAcceptTimerHandler,
          pBtampCtx);
   
  return VOS_STATUS_SUCCESS;
}/* WLANBAP_InitConnectionAcceptTimer */

/*==========================================================================

  FUNCTION    WLANBAP_DeinitConnectionAcceptTimer

  DESCRIPTION 
    Destroy the Connection Accept Timer.
    
  DEPENDENCIES 
    
  PARAMETERS 

    IN
    pBtampCtx:   pointer to the BAP control block
   
  RETURN VALUE
    The result code associated with performing the operation  

    VOS_STATUS_E_FAULT:  access would cause a page fault  
    VOS_STATUS_SUCCESS:  Everything is good :) 

  SIDE EFFECTS 
  
============================================================================*/
VOS_STATUS 
WLANBAP_DeinitConnectionAcceptTimer
( 
  ptBtampContext  pBtampCtx
)
{
  VOS_STATUS  vosStatus = VOS_STATUS_SUCCESS;

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/


  /*------------------------------------------------------------------------
    Sanity check BAP control block 
   ------------------------------------------------------------------------*/

  if ( NULL == pBtampCtx ) 
  {
    VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_ERROR,
                 "Invalid BAP pointer in %s", __FILE__);
    return VOS_STATUS_E_FAULT;
  }

  /*Initialize and then Start the timer */
  vosStatus = vos_timer_destroy ( 
          &pBtampCtx->bapConnectionAcceptTimer );
   
  return VOS_STATUS_SUCCESS;
}/* WLANBAP_DeinitConnectionAcceptTimer */

/*==========================================================================

  FUNCTION    WLANBAP_StartConnectionAcceptTimer

  DESCRIPTION 
    Start the Connection Accept Timer.
    
  DEPENDENCIES 
    
  PARAMETERS 

    IN
    pBtampCtx:   pointer to the BAP control block
    interval:    time interval.
   
  RETURN VALUE
    The result code associated with performing the operation  

    VOS_STATUS_E_FAULT:  access would cause a page fault  
    VOS_STATUS_SUCCESS:  Everything is good :) 

  SIDE EFFECTS 
  
============================================================================*/
VOS_STATUS 
WLANBAP_StartConnectionAcceptTimer
( 
  ptBtampContext  pBtampCtx,
  v_U32_t interval
)
{
  /*------------------------------------------------------------------------
    Sanity check BAP control block 
   ------------------------------------------------------------------------*/

  if ( NULL == pBtampCtx ) 
  {
    VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_ERROR,
                 "Invalid BAP pointer in %s", __FILE__);
    return VOS_STATUS_E_FAULT;
  }

  /*Start the connection accept timer*/
  vos_timer_start( 
          &pBtampCtx->bapConnectionAcceptTimer,
          interval);

  return VOS_STATUS_SUCCESS;
}/* WLANBAP_StartConnectionAcceptTimer */


/*==========================================================================

  FUNCTION    WLANBAP_StopConnectionAcceptTimer 

  DESCRIPTION 
    Stop the Connection Accept Timer.
    
  DEPENDENCIES 
    
  PARAMETERS 

    IN
    pBtampCtx:   pointer to the BAP control block
   
  RETURN VALUE
    The result code associated with performing the operation  

    VOS_STATUS_E_FAULT:  access would cause a page fault  
    VOS_STATUS_SUCCESS:  Everything is good :) 

  SIDE EFFECTS 
  
============================================================================*/
VOS_STATUS 
WLANBAP_StopConnectionAcceptTimer 
( 
  ptBtampContext  pBtampCtx
)
{
  VOS_STATUS  vosStatus = VOS_STATUS_SUCCESS;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/


  /*------------------------------------------------------------------------
    Sanity check BAP control block 
   ------------------------------------------------------------------------*/

  if ( NULL == pBtampCtx ) 
  {
    VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_ERROR,
                 "Invalid BAP pointer in %s", __FILE__);
    return VOS_STATUS_E_FAULT;
  }

  /*Stop the timer */
  vosStatus =  vos_timer_stop( 
           &pBtampCtx->bapConnectionAcceptTimer);
 
   
  return VOS_STATUS_SUCCESS;
}/* WLANBAP_StopConnectionAcceptTimer */ 



/*==========================================================================

  FUNCTION    WLANBAP_ConnectionAcceptTimerHandler

  DESCRIPTION 
    Callback function registered with vos timer for the Connection
    Accept timer 
    
  DEPENDENCIES 
    
  PARAMETERS 

    IN
    userData:      pointer can be used to retrive the BT-AMP context 
   
  RETURN VALUE
    None
         
  SIDE EFFECTS 
  
============================================================================*/
v_VOID_t 
WLANBAP_ConnectionAcceptTimerHandler
( 
  v_PVOID_t userData 
)
{
  ptBtampContext  pBtampCtx = (ptBtampContext)userData;
  tWLAN_BAPEvent bapEvent; /* State machine event */
  VOS_STATUS  vosStatus = VOS_STATUS_SUCCESS;
  v_U8_t status;    /* return the BT-AMP status here */
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  /*-----------------------------------------------------------------------
    Sanity check 
   -----------------------------------------------------------------------*/
  if ( NULL == pBtampCtx )
  {
     VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_ERROR,
                "WLAN BAP: Fatal error in %s", __FILE__ );
     VOS_ASSERT(0);
     return; 
  }

 /*---------------------------------------------------------------------
    Feed this timeout to the BTAMP FSM 
   ---------------------------------------------------------------------*/
  /* Fill in the event structure */ 
  bapEvent.event = eWLAN_BAP_TIMER_CONNECT_ACCEPT_TIMEOUT;
  bapEvent.params = NULL;

  /* Handle event */ 
  vosStatus = btampFsm(pBtampCtx, &bapEvent, &status);

  /* Now transition to fully disconnected and notify phy link disconnect*/ 
  bapEvent.event =  eWLAN_BAP_MAC_READY_FOR_CONNECTIONS;
  bapEvent.params = NULL;

  /* Handle event */ 
  vosStatus = btampFsm(pBtampCtx, &bapEvent, &status);


}/*WLANBAP_ConnectionAcceptTimerHandler*/

/*==========================================================================

  FUNCTION    WLANBAP_InitLinkSupervisionTimer

  DESCRIPTION 
    Initialize the Link Supervision Timer.
    
  DEPENDENCIES 
    
  PARAMETERS 

    IN
    pBtampCtx:   pointer to the BAP control block
   
  RETURN VALUE
    The result code associated with performing the operation  

    VOS_STATUS_E_FAULT:  access would cause a page fault  
    VOS_STATUS_SUCCESS:  Everything is good :) 

  SIDE EFFECTS 
  
============================================================================*/
VOS_STATUS 
WLANBAP_InitLinkSupervisionTimer
( 
  ptBtampContext  pBtampCtx
)
{
  VOS_STATUS  vosStatus = VOS_STATUS_SUCCESS;

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/


  /*------------------------------------------------------------------------
    Sanity check BAP control block 
   ------------------------------------------------------------------------*/

  if ( NULL == pBtampCtx ) 
  {
    VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_ERROR,
                 "Invalid BAP pointer in %s", __FILE__);
    return VOS_STATUS_E_FAULT;
  }

  /*Initialize the timer */
  vosStatus = vos_timer_init( 
          &pBtampCtx->bapLinkSupervisionTimer,
          VOS_TIMER_TYPE_SW, /* use this type */
          WLANBAP_LinkSupervisionTimerHandler,
          pBtampCtx);
   
  return VOS_STATUS_SUCCESS;
}/* WLANBAP_InitLinkSupervisionTimer */

/*==========================================================================

  FUNCTION    WLANBAP_DeinitLinkSupervisionTimer

  DESCRIPTION 
    Destroy the Link Supervision Timer.
    
  DEPENDENCIES 
    
  PARAMETERS 

    IN
    pBtampCtx:   pointer to the BAP control block
   
  RETURN VALUE
    The result code associated with performing the operation  

    VOS_STATUS_E_FAULT:  access would cause a page fault  
    VOS_STATUS_SUCCESS:  Everything is good :) 

  SIDE EFFECTS 
  
============================================================================*/
VOS_STATUS 
WLANBAP_DeinitLinkSupervisionTimer
( 
  ptBtampContext  pBtampCtx
)
{
  VOS_STATUS  vosStatus = VOS_STATUS_SUCCESS;

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/


  /*------------------------------------------------------------------------
    Sanity check BAP control block 
   ------------------------------------------------------------------------*/

  if ( NULL == pBtampCtx ) 
  {
    VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_ERROR,
                 "Invalid BAP pointer in %s", __FILE__);
    return VOS_STATUS_E_FAULT;
  }

  /*Initialize and then Start the timer */
  vosStatus = vos_timer_destroy ( 
          &pBtampCtx->bapLinkSupervisionTimer );
   
  return VOS_STATUS_SUCCESS;
}/* WLANBAP_DeinitLinkSupervisionTimer */

/*==========================================================================

  FUNCTION    WLANBAP_StartLinkSupervisionTimer

  DESCRIPTION 
    Start the LinkSupervisionTimer Timer.
    
  DEPENDENCIES 
    
  PARAMETERS 

    IN
    pBtampCtx:   pointer to the BAP control block
    interval:    time interval.
   
  RETURN VALUE
    The result code associated with performing the operation  

    VOS_STATUS_E_FAULT:  access would cause a page fault  
    VOS_STATUS_SUCCESS:  Everything is good :) 

  SIDE EFFECTS 
  
============================================================================*/
VOS_STATUS 
WLANBAP_StartLinkSupervisionTimer
( 
  ptBtampContext  pBtampCtx,
  v_U32_t interval
)
{
  /*------------------------------------------------------------------------
    Sanity check BAP control block 
   ------------------------------------------------------------------------*/

  if ( NULL == pBtampCtx ) 
  {
    VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_ERROR,
                 "Invalid BAP pointer in %s", __FILE__);
    return VOS_STATUS_E_FAULT;
  }

  vos_timer_start( 
          &pBtampCtx->bapLinkSupervisionTimer,
          interval);

  return VOS_STATUS_SUCCESS;
}/* WLANBAP_StartLinkSupervisionTimer */

/*==========================================================================

  FUNCTION    WLANBAP_StopLinkSupervisionTimer 

  DESCRIPTION 
    Stop the LinkSupervision Timer.
    
  DEPENDENCIES 
    
  PARAMETERS 

    IN
    pBtampCtx:   pointer to the BAP control block
   
  RETURN VALUE
    The result code associated with performing the operation  

    VOS_STATUS_E_FAULT:  access would cause a page fault  
    VOS_STATUS_SUCCESS:  Everything is good :) 

  SIDE EFFECTS 
  
============================================================================*/
VOS_STATUS 
WLANBAP_StopLinkSupervisionTimer 
( 
  ptBtampContext  pBtampCtx
)
{
  VOS_STATUS  vosStatus = VOS_STATUS_SUCCESS;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/


  /*------------------------------------------------------------------------
    Sanity check BAP control block 
   ------------------------------------------------------------------------*/

  if ( NULL == pBtampCtx ) 
  {
    VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_ERROR,
                 "Invalid BAP pointer in %s", __FILE__);
    return VOS_STATUS_E_FAULT;
  }

  /*Stop the timer */
  vosStatus =  vos_timer_stop( 
           &pBtampCtx->bapLinkSupervisionTimer);
 
   
  return VOS_STATUS_SUCCESS;
}/* WLANBAP_StopLinkSupervisionTimer */ 


/*==========================================================================

  FUNCTION    WLANBAP_LinkSupervisionTimerHandler

  DESCRIPTION 
    Callback function registered with vos timer for the LinkSupervision timer 
    
  DEPENDENCIES 
    
  PARAMETERS 

    IN
    userData:      pointer can be used to retrive the BT-AMP context 
   
  RETURN VALUE
    None
         
  SIDE EFFECTS 
  
============================================================================*/
v_VOID_t 
WLANBAP_LinkSupervisionTimerHandler
( 
  v_PVOID_t userData 
)
{
    ptBtampContext           pBtampCtx =      (ptBtampContext)userData;
    VOS_STATUS               vosStatus =      VOS_STATUS_SUCCESS;
    ptBtampHandle            btampHandle =    (ptBtampHandle)userData;
    tWLAN_BAPEvent           bapEvent; /* State machine event */
    v_U8_t                   phy_link_handle = pBtampCtx->phy_link_handle;

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  /*-----------------------------------------------------------------------
    Sanity check 
   -----------------------------------------------------------------------*/
    if ( NULL == pBtampCtx )
    {
       VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_ERROR,
                "WLAN BAP: Fatal error in %s", __FILE__ );
       VOS_ASSERT(0);
       return; 
    }

     VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_ERROR,
                "WLAN BAP:In LinkSupervision Timer handler %s", __FILE__ );

    if(pBtampCtx->dataPktPending == VOS_TRUE)
    {
        pBtampCtx->dataPktPending = VOS_FALSE;
        pBtampCtx->lsReqPktPending = VOS_FALSE;
	      pBtampCtx->retries = 0;
        vosStatus = WLANBAP_StopLinkSupervisionTimer(pBtampCtx);
        vosStatus = WLANBAP_StartLinkSupervisionTimer (pBtampCtx,
                    pBtampCtx->bapLinkSupervisionTimerInterval * WLANBAP_BREDR_BASEBAND_SLOT_TIME);

        //Data is seen. or our previous packet is not yet fetched by TL.Don't do any thing.Just return;
        return;
    }
    else if((pBtampCtx->lsReqPktPending == VOS_TRUE ) && (pBtampCtx->retries == 5))
    {
        VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_ERROR,
                    "#########WLAN BAP: LinkSupervision Timed OUT######## %s", __FUNCTION__ );
	
     /*---------------------------------------------------------------------
    Feed this timeout to the BTAMP FSM 
   ---------------------------------------------------------------------*/
        /* Fill in the event structure */ 
        bapEvent.event =eWLAN_BAP_HCI_PHYSICAL_LINK_DISCONNECT;
        bapEvent.params = NULL;

        /* Handle event */ 
        vosStatus = btampFsm(pBtampCtx, &bapEvent, (v_U8_t *)&vosStatus);
    }
    else
    {    
        /* If we have transmit pkt pending and the time out occured,resend the ls packet */
        WLANBAP_StopLinkSupervisionTimer(pBtampCtx);
	    pBtampCtx->pPacket = pBtampCtx->lsReqPacket;
        vosStatus = WLANBAP_TxLinkSupervision( btampHandle, 
                                               phy_link_handle, 
                                               pBtampCtx->pPacket ,
                                               WLANTL_BT_AMP_TYPE_LS_REQ);
    }
    
}/*WLANBAP_LinkSupervisionTimerHandler*/

/*==========================================================================

  FUNCTION    WLANBAP_StartTxPacketMonitorTimer

  DESCRIPTION 
    Start the Tx Packet Monitor Timer.
    
  DEPENDENCIES 
    
  PARAMETERS 

    IN
    pBtampCtx:   pointer to the BAP control block
    interval:    time interval.
   
  RETURN VALUE
    The result code associated with performing the operation  

    VOS_STATUS_E_FAULT:  access would cause a page fault  
    VOS_STATUS_SUCCESS:  Everything is good :) 

  SIDE EFFECTS 
  
============================================================================*/
VOS_STATUS 
WLANBAP_StartTxPacketMonitorTimer
( 
  ptBtampContext  pBtampCtx
)
{
  VOS_STATUS  vosStatus = VOS_STATUS_SUCCESS;
  v_U32_t     uInterval = WLAN_BAP_TX_PKT_MONITOR_TIME; 
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/


  /*------------------------------------------------------------------------
    Sanity check BAP control block 
   ------------------------------------------------------------------------*/
  if ( NULL == pBtampCtx ) 
  {
    VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_ERROR,
                 "Invalid BAP pointer in %s", __FILE__);
    return VOS_STATUS_E_FAULT;
  }

  /*Start the timer */
  vosStatus = vos_timer_start( &pBtampCtx->bapTxPktMonitorTimer,
                                uInterval);

  return vosStatus;
}/* WLANBAP_StartTxPacketMonitorTimer */


/*==========================================================================

  FUNCTION    WLANBAP_StopTxPacketMonitorTimer 

  DESCRIPTION 
    Stop the Connection Accept Timer.
    
  DEPENDENCIES 
    
  PARAMETERS 

    IN
    pBtampCtx:   pointer to the BAP control block
   
  RETURN VALUE
    The result code associated with performing the operation  

    VOS_STATUS_E_FAULT:  access would cause a page fault  
    VOS_STATUS_SUCCESS:  Everything is good :) 

  SIDE EFFECTS 
  
============================================================================*/
VOS_STATUS 
WLANBAP_StopTxPacketMonitorTimer 
( 
  ptBtampContext  pBtampCtx
)
{
  VOS_STATUS  vosStatus = VOS_STATUS_SUCCESS;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/


  /*------------------------------------------------------------------------
    Sanity check BAP control block 
   ------------------------------------------------------------------------*/
  if ( NULL == pBtampCtx ) 
  {
    VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_ERROR,
                 "Invalid BAP pointer in %s", __FILE__);
    return VOS_STATUS_E_FAULT;
  }

  /*Stop the timer */
  vosStatus =  vos_timer_stop( &pBtampCtx->bapTxPktMonitorTimer);
 
   
  return vosStatus;
}/* WLANBAP_StopTxPacketMonitorTimer */ 



/*==========================================================================

  FUNCTION    WLANBAP_TxPacketMonitorHandler

  DESCRIPTION 
    Callback function registered with vos timer for the Connection
    Accept timer 
    
  DEPENDENCIES 
    
  PARAMETERS 

    IN
    userData:      pointer can be used to retrive the BT-AMP context 
   
  RETURN VALUE
    None
         
  SIDE EFFECTS 
  
============================================================================*/
v_VOID_t 
WLANBAP_TxPacketMonitorHandler
( 
  v_PVOID_t userData 
)
{
  ptBtampContext     pBtampCtx       = (ptBtampContext)userData;
  v_U32_t            uTxCompleted    = 0; 
  tpBtampLogLinkCtx  pLogLinkContext = NULL;
  v_U8_t             i, j;
  tBtampHCI_Event    bapHCIEvent; /* This now encodes ALL event types */
  BTAMPFSM_INSTANCEDATA_T *instanceVar = &pBtampCtx->bapPhysLinkMachine;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  /*-----------------------------------------------------------------------
    Sanity check 
   -----------------------------------------------------------------------*/
  if ( NULL == pBtampCtx )
  {
     VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_ERROR,
                "WLAN BAP: Fatal error in %s", __FILE__ );
     VOS_ASSERT(0);
     return; 
  }

#ifdef BAP_DEBUG
  /* Trace the tBtampCtx being passed in. */
  VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH,
            "WLAN BAP Context Monitor: pBtampCtx value = %x in %s:%d", pBtampCtx, __FUNCTION__, __LINE__ );
#endif //BAP_DEBUG

  /* Format the Number of completed packets event */ 
  bapHCIEvent.bapHCIEventCode = BTAMP_TLV_HCI_NUM_OF_COMPLETED_PKTS_EVENT;
  bapHCIEvent.u.btampNumOfCompletedPktsEvent.num_handles = 0;

 /*---------------------------------------------------------------------
    Check if LL still exists, if TRUE generate num_pkt_event and
    restart the timer 
   ---------------------------------------------------------------------*/
  for (i = 0, j = 0; i < WLANBAP_MAX_LOG_LINKS ; i++) 
  {
     pLogLinkContext = &pBtampCtx->btampLogLinkCtx[i];
     if ( pLogLinkContext->present ) 
     {
       uTxCompleted = pLogLinkContext->uTxPktCompleted;
       bapHCIEvent.u.btampNumOfCompletedPktsEvent.conn_handles[j] =
           pLogLinkContext->log_link_handle;
       bapHCIEvent.u.btampNumOfCompletedPktsEvent.num_completed_pkts[j] =
           uTxCompleted;

       j++;

       vos_atomic_decrement_U32_by_value((v_U32_t *) &pLogLinkContext->uTxPktCompleted,
                                         (v_U32_t) uTxCompleted);

      VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_ERROR,
                "wlan bap: %s Log Link handle - %d No Of Pkts - %d", __FILE__, 
	      pLogLinkContext->log_link_handle, uTxCompleted);  
     }
  }

  /* Indicate only if at least one logical link is present and number of
     completed packets is non zero */
  if (j && uTxCompleted)
  {
      VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_ERROR,
                "WLAN BAP: Indicating Num Completed packets Event");

      /*issue num_pkt_event for uTxCompleted*/
      bapHCIEvent.u.btampNumOfCompletedPktsEvent.num_handles = j;
      (*pBtampCtx->pBapHCIEventCB)
      (
           pBtampCtx->pHddHdl,   /* this refers the BSL per application context */
           &bapHCIEvent, /* This now encodes ALL event types */
           VOS_TRUE /* Flag to indicate assoc-specific event */ 
      );
  }

  /* Restart the Packet monitoring timer if still Physical link
   * is present. 
   * It is possible that when the physical link is tear down, 
   * timer start request is in Q and could start again. 
   */
  if (CONNECTED == instanceVar->stateVar)
  {
    WLANBAP_StartTxPacketMonitorTimer(pBtampCtx);
  }
}/*WLANBAP_TxPacketMonitorHandler*/


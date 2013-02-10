/*
* Copyright (c) 2012 Qualcomm Atheros, Inc.
* All Rights Reserved.
* Qualcomm Atheros Confidential and Proprietary.
*/

/*===========================================================================

                      s a p M o d u l e . C
                                               
  OVERVIEW:
  
  This software unit holds the implementation of the WLAN SAP modules
  functions providing EXTERNAL APIs. It is also where the global SAP module
  context gets initialised
  
  DEPENDENCIES: 

  Are listed for each API below.
  
  Copyright (c) 2010 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Qualcomm Technologies Confidential and Proprietary
===========================================================================*/

/*===========================================================================

                      EDIT HISTORY FOR FILE


  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.



  when               who                 what, where, why
----------       ---                --------------------------------------------------------
03/15/10     SOFTAP team            Created module
06/03/10     js                     Added support to hostapd driven 
 *                                  deauth/disassoc/mic failure

===========================================================================*/

/* $Header$ */

/*----------------------------------------------------------------------------
 * Include Files
 * -------------------------------------------------------------------------*/
#include "wlan_qct_tl.h"
#include "vos_trace.h"

// Pick up the sme callback registration API
#include "sme_Api.h"

// SAP API header file

#include "sapInternal.h"
#include "halInternal.h"
#include "smeInside.h"

/*----------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * -------------------------------------------------------------------------*/
#define SAP_DEBUG

/*----------------------------------------------------------------------------
 * Type Declarations
 * -------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
 * Global Data Definitions
 * -------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 *  External declarations for global context 
 * -------------------------------------------------------------------------*/
//  No!  Get this from VOS.
//  The main per-Physical Link (per WLAN association) context.
ptSapContext  gpSapCtx = NULL;

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

/*==========================================================================
  FUNCTION    WLANSAP_Open

  DESCRIPTION 
    Called at driver initialization (vos_open). SAP will initialize 
    all its internal resources and will wait for the call to start to 
    register with the other modules. 
    
  DEPENDENCIES 
    
  PARAMETERS 

    IN
    pvosGCtx    : Pointer to the global vos context; a handle to SAP's 
                  control block can be extracted from its context 
   
  RETURN VALUE
    The result code associated with performing the operation  

    VOS_STATUS_E_FAULT: Pointer to SAP cb is NULL ; access would cause a page 
                         fault  
    VOS_STATUS_SUCCESS: Success

  SIDE EFFECTS   
============================================================================*/
VOS_STATUS 
WLANSAP_Open
(
    v_PVOID_t pvosGCtx
)
{

    ptSapContext  pSapCtx = NULL;
    /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
    VOS_ASSERT(pvosGCtx);
    /*------------------------------------------------------------------------
    Allocate (and sanity check?!) SAP control block 
    ------------------------------------------------------------------------*/
    vos_alloc_context(pvosGCtx, VOS_MODULE_ID_SAP, (v_VOID_t **)&pSapCtx, sizeof(tSapContext));

    if (NULL == pSapCtx)
    {
        VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_ERROR,
                 "Invalid SAP pointer from pvosGCtx on WLANSAP_Open");
        return VOS_STATUS_E_FAULT;
    }

    /*------------------------------------------------------------------------
        Clean up SAP control block, initialize all values
    ------------------------------------------------------------------------*/
    VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_INFO_HIGH, "WLANSAP_Open");

    WLANSAP_CleanCB(pSapCtx, 0 /*do not empty*/);

    // Setup the "link back" to the VOSS context
    pSapCtx->pvosGCtx = pvosGCtx;

    // Store a pointer to the SAP context provided by VOSS
    gpSapCtx = pSapCtx;
   
    /*------------------------------------------------------------------------
        Allocate internal resources
       ------------------------------------------------------------------------*/

    return VOS_STATUS_SUCCESS;
}// WLANSAP_Open

/*==========================================================================
  FUNCTION    WLANSAP_Start

  DESCRIPTION 
    Called as part of the overall start procedure (vos_start). SAP will 
    use this call to register with TL as the SAP entity for 
    SAP RSN frames. 

  DEPENDENCIES 
    
  PARAMETERS 

    IN
    pvosGCtx    : Pointer to the global vos context; a handle to SAP's 
                  control block can be extracted from its context 

  RETURN VALUE
    The result code associated with performing the operation  

    VOS_STATUS_E_FAULT: Pointer to SAP cb is NULL ; access would cause a page 
                         fault  
    VOS_STATUS_SUCCESS: Success

  SIDE EFFECTS
============================================================================*/

VOS_STATUS 
WLANSAP_Start
( 
    v_PVOID_t  pvosGCtx 
)
{
#ifdef WLAN_SOFTAP_FEATURE
    ptSapContext  pSapCtx = NULL;

    /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

    VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_INFO_HIGH,
                 "WLANSAP_Start invoked successfully\n");
    /*------------------------------------------------------------------------
        Sanity check
        Extract SAP control block 
    ------------------------------------------------------------------------*/
    pSapCtx = VOS_GET_SAP_CB(pvosGCtx);
    if ( NULL == pSapCtx ) 
    {
        VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_ERROR,
                "Invalid SAP pointer from pvosGCtx on WLANSAP_Start");
        return VOS_STATUS_E_FAULT;
    }

    /*------------------------------------------------------------------------
        For now, presume security is not enabled.
    -----------------------------------------------------------------------*/
    pSapCtx->ucSecEnabled = WLANSAP_SECURITY_ENABLED_STATE;


    /*------------------------------------------------------------------------
        Now configure the roaming profile links. To SSID and bssid.
    ------------------------------------------------------------------------*/
    // We have room for two SSIDs.  
    pSapCtx->csrRoamProfile.SSIDs.numOfSSIDs = 1; // This is true for now.  
    pSapCtx->csrRoamProfile.SSIDs.SSIDList = pSapCtx->SSIDList;  //Array of two  
    pSapCtx->csrRoamProfile.SSIDs.SSIDList[0].SSID.length = 0;
    pSapCtx->csrRoamProfile.SSIDs.SSIDList[0].handoffPermitted = VOS_FALSE;
    pSapCtx->csrRoamProfile.SSIDs.SSIDList[0].ssidHidden = pSapCtx->SSIDList[0].ssidHidden;

    pSapCtx->csrRoamProfile.BSSIDs.numOfBSSIDs = 1; // This is true for now.  
    pSapCtx->csrRoamProfile.BSSIDs.bssid = &pSapCtx->bssid;  

    // Now configure the auth type in the roaming profile. To open.
    pSapCtx->csrRoamProfile.negotiatedAuthType = eCSR_AUTH_TYPE_OPEN_SYSTEM; // open is the default
    
    if( !VOS_IS_STATUS_SUCCESS( vos_lock_init( &pSapCtx->SapGlobalLock)))
    {
        VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_ERROR,
                 "WLANSAP_Start failed init lock\n");        
        return VOS_STATUS_E_FAULT;
    }


#endif 

    return VOS_STATUS_SUCCESS;
}/* WLANSAP_Start */

/*==========================================================================

  FUNCTION    WLANSAP_Stop

  DESCRIPTION 
    Called by vos_stop to stop operation in SAP, before close. SAP will suspend all 
    BT-AMP Protocol Adaption Layer operation and will wait for the close 
    request to clean up its resources. 

  DEPENDENCIES 
    
  PARAMETERS 

    IN
    pvosGCtx    : Pointer to the global vos context; a handle to SAP's 
                  control block can be extracted from its context 

  RETURN VALUE
    The result code associated with performing the operation  

    VOS_STATUS_E_FAULT: Pointer to SAP cb is NULL ; access would cause a page 
                         fault  
    VOS_STATUS_SUCCESS: Success

  SIDE EFFECTS
============================================================================*/
VOS_STATUS 
WLANSAP_Stop
( 
    v_PVOID_t  pvosGCtx 
)
{

#ifdef WLAN_SOFTAP_FEATURE
    ptSapContext  pSapCtx = NULL;

    /*------------------------------------------------------------------------
        Sanity check
        Extract SAP control block 
    ------------------------------------------------------------------------*/
    VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_INFO_HIGH,
                "WLANSAP_Stop invokedsuccessfully ");

    pSapCtx = VOS_GET_SAP_CB(pvosGCtx);
    if (NULL == pSapCtx)
    {
        VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_ERROR,
                 "Invalid SAP pointer from pvosGCtx on WLANSAP_Stop");
        return VOS_STATUS_E_FAULT;
    }
    
    if( !VOS_IS_STATUS_SUCCESS( vos_lock_destroy( &pSapCtx->SapGlobalLock ) ) )
    {
        VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_ERROR,
                 "WLANSAP_Stop failed destroy lock\n");        
        return VOS_STATUS_E_FAULT;
    }
    /*------------------------------------------------------------------------
        Stop SAP (de-register RSN handler!?)  
    ------------------------------------------------------------------------*/
#endif

    return VOS_STATUS_SUCCESS;
}/* WLANSAP_Stop */

/*==========================================================================
  FUNCTION    WLANSAP_Close

  DESCRIPTION 
    Called by vos_close during general driver close procedure. SAP will clean up 
    all the internal resources. 

  DEPENDENCIES 

  PARAMETERS 

    IN
    pvosGCtx    : Pointer to the global vos context; a handle to SAP's 
                  control block can be extracted from its context 

  RETURN VALUE
    The result code associated with performing the operation  

    VOS_STATUS_E_FAULT: Pointer to SAP cb is NULL ; access would cause a page 
                         fault  
    VOS_STATUS_SUCCESS: Success

  SIDE EFFECTS
============================================================================*/
VOS_STATUS 
WLANSAP_Close
( 
    v_PVOID_t  pvosGCtx 
)
{
#ifdef WLAN_SOFTAP_FEATURE
    ptSapContext  pSapCtx = NULL;
    /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

    /*------------------------------------------------------------------------
        Sanity check
        Extract SAP control block 
    ------------------------------------------------------------------------*/
    VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_INFO_HIGH,
                 "WLANSAP_Close invoked");

    pSapCtx = VOS_GET_SAP_CB(pvosGCtx);
    if (NULL == pSapCtx)
    {
        VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_ERROR,
                 "Invalid SAP pointer from pvosGCtx on WLANSAP_Close");
        return VOS_STATUS_E_FAULT;
    }

    /*------------------------------------------------------------------------
        Cleanup SAP control block. 
    ------------------------------------------------------------------------*/
    VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_INFO_HIGH, "WLANSAP_Close");
    WLANSAP_CleanCB(pSapCtx, VOS_TRUE /* empty queues/lists/pkts if any*/);

    /*------------------------------------------------------------------------
        Free SAP context from VOSS global 
    ------------------------------------------------------------------------*/
    vos_free_context(pvosGCtx, VOS_MODULE_ID_SAP, pSapCtx);

#endif
    return VOS_STATUS_SUCCESS;
}/* WLANSAP_Close */

/*----------------------------------------------------------------------------
 * Utility Function implementations 
 * -------------------------------------------------------------------------*/

/*==========================================================================

  FUNCTION    WLANSAP_CleanCB

  DESCRIPTION 
    Clear out all fields in the SAP context.

  DEPENDENCIES 

  PARAMETERS 

    IN
    pvosGCtx    : Pointer to the global vos context; a handle to SAP's 
                  control block can be extracted from its context 

  RETURN VALUE
    The result code associated with performing the operation  

    VOS_STATUS_E_FAULT: Pointer to SAP cb is NULL ; access would cause a page 
                         fault  
    VOS_STATUS_SUCCESS: Success

  SIDE EFFECTS
============================================================================*/
VOS_STATUS 
WLANSAP_CleanCB
( 
    ptSapContext  pSapCtx,
    v_U32_t freeFlag // 0 /*do not empty*/);
)
{
#ifdef WLAN_SOFTAP_FEATURE
    /*------------------------------------------------------------------------
        Sanity check SAP control block 
    ------------------------------------------------------------------------*/

    if (NULL == pSapCtx)
    {
        VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_ERROR,
                 "Invalid SAP pointer in WLANSAP_CleanCB");
        return VOS_STATUS_E_FAULT;
    }

    /*------------------------------------------------------------------------
        Clean up SAP control block, initialize all values
    ------------------------------------------------------------------------*/
    VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_INFO_HIGH, "WLANSAP_CleanCB");

    vos_mem_zero( pSapCtx, sizeof(tSapContext));

    pSapCtx->pvosGCtx = NULL;

    pSapCtx->sapsMachine= eSAP_DISCONNECTED;

    VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_INFO_HIGH, "%s: Initializing State: %d, sapContext value = %x",
            __FUNCTION__, pSapCtx->sapsMachine, pSapCtx);   
    pSapCtx->sessionId = 0;
    pSapCtx->channel = 0;  

#endif
    return VOS_STATUS_SUCCESS;
}// WLANSAP_CleanCB

/*==========================================================================
  FUNCTION    WLANSAP_pmcFullPwrReqCB

  DESCRIPTION 
    Callback provide to PMC in the pmcRequestFullPower API. 

  DEPENDENCIES 

  PARAMETERS 

    IN
    callbackContext:  The user passed in a context to identify 
    status:           The halStatus 

  RETURN VALUE
    None

  SIDE EFFECTS
============================================================================*/
void
WLANSAP_pmcFullPwrReqCB
(
    void *callbackContext,
    eHalStatus status
)
{
    if(HAL_STATUS_SUCCESS(status))
    {
         //If success what else to be handled???
    }
    else
    {
        VOS_TRACE(VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_ERROR, 
               "WLANSAP_pmcFullPwrReqCB: PMC failed to put the chip in Full power\n");

        //ASSERT
        VOS_ASSERT(0);
    }

}// WLANSAP_pmcFullPwrReqCB
/*==========================================================================
  FUNCTION    WLANSAP_getState

  DESCRIPTION 
    This api returns the current SAP state to the caller.

  DEPENDENCIES 

  PARAMETERS 

    IN
    pContext            : Pointer to Sap Context structure

  RETURN VALUE
    Returns the SAP FSM state.  
============================================================================*/

v_U8_t WLANSAP_getState 
(
    v_PVOID_t  pvosGCtx
)
{
    ptSapContext  pSapCtx = NULL;

    pSapCtx = VOS_GET_SAP_CB(pvosGCtx);

    if ( NULL == pSapCtx )
    {
        VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_INFO_HIGH,
           "Invalid SAP pointer from pvosGCtx on WLANSAP_Start");
        return VOS_STATUS_E_FAULT;
    }
    return pSapCtx->sapsMachine;
}

/*==========================================================================
  FUNCTION    WLANSAP_StartBss

  DESCRIPTION 
    This api function provides SAP FSM event eWLAN_SAP_PHYSICAL_LINK_CREATE for
    starting AP BSS

  DEPENDENCIES 

  PARAMETERS 

    IN
    pContext            : Pointer to Sap Context structure
    pQctCommitConfig    : Pointer to configuration structure passed down from HDD(HostApd for Android)
    hdd_SapEventCallback: Callback function in HDD called by SAP to inform HDD about SAP results
    pUsrContext         : Parameter that will be passed back in all the SAP callback events.

  RETURN VALUE
    The result code associated with performing the operation  

    VOS_STATUS_E_FAULT: Pointer to SAP cb is NULL ; access would cause a page 
                         fault  
    VOS_STATUS_SUCCESS: Success

  SIDE EFFECTS
============================================================================*/
VOS_STATUS 
WLANSAP_StartBss
(
    v_PVOID_t  pvosGCtx,//pwextCtx
    tpWLAN_SAPEventCB pSapEventCallback,
    tsap_Config_t *pConfig,
    v_PVOID_t  pUsrContext
)
{
    tWLAN_SAPEvent sapEvent;    /* State machine event*/
    VOS_STATUS vosStatus = VOS_STATUS_SUCCESS;
    ptSapContext  pSapCtx = NULL;
    tANI_BOOLEAN restartNeeded;
    tHalHandle hHal;

    /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

    /*------------------------------------------------------------------------
        Sanity check
        Extract SAP control block 
    ------------------------------------------------------------------------*/
    VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_INFO_HIGH,
                 "WLANSAP_StartBss");

    if (VOS_STA_SAP_MODE == vos_get_conparam ())
    {
        pSapCtx = VOS_GET_SAP_CB(pvosGCtx);
        if ( NULL == pSapCtx )
        {
            VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_INFO_HIGH,
                    "Invalid SAP pointer from pvosGCtx on WLANSAP_Start");
            return VOS_STATUS_E_FAULT;
        }
        pSapCtx->sapsMachine = eSAP_DISCONNECTED;

        /* Channel selection is auto or configured */
        pSapCtx->channel = pConfig->channel;
        pSapCtx->pUsrContext = pUsrContext;

        //Set the BSSID to your "self MAC Addr" read the mac address from Configuation ITEM received from HDD
        pSapCtx->csrRoamProfile.BSSIDs.numOfBSSIDs = 1;
        vos_mem_copy(pSapCtx->csrRoamProfile.BSSIDs.bssid, 
                     pSapCtx->self_mac_addr,
                     sizeof( tCsrBssid ) ); 

        //Save a copy to SAP context
        vos_mem_copy(pSapCtx->csrRoamProfile.BSSIDs.bssid, 
                    pConfig->self_macaddr.bytes, sizeof(v_MACADDR_t));
        vos_mem_copy(pSapCtx->self_mac_addr,
                    pConfig->self_macaddr.bytes, sizeof(v_MACADDR_t));

        //copy the configuration items to csrProfile
        sapconvertToCsrProfile( pConfig, eCSR_BSS_TYPE_INFRA_AP, &pSapCtx->csrRoamProfile);

        hHal = VOS_GET_HAL_CB(pvosGCtx);
        if (NULL == hHal)
        {
            VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_INFO_HIGH,
                       "%s: Invalid MAC context from pvosGCtx", __FUNCTION__);
        }
        else
        {
            sme_setRegInfo(hHal, pConfig->countryCode);
            sme_ResetCountryCodeInformation(hHal, &restartNeeded);
        }

        // Copy MAC filtering settings to sap context
        pSapCtx->eSapMacAddrAclMode = pConfig->SapMacaddr_acl;
        vos_mem_copy(pSapCtx->acceptMacList, pConfig->accept_mac, sizeof(pConfig->accept_mac));
        pSapCtx->nAcceptMac = pConfig->num_accept_mac;
        sapSortMacList(pSapCtx->acceptMacList, pSapCtx->nAcceptMac);
        vos_mem_copy(pSapCtx->denyMacList, pConfig->deny_mac, sizeof(pConfig->deny_mac));
        pSapCtx->nDenyMac = pConfig->num_deny_mac;
        sapSortMacList(pSapCtx->denyMacList, pSapCtx->nDenyMac);

        /* Fill in the event structure for FSM */
        sapEvent.event = eSAP_HDD_START_INFRA_BSS;
        sapEvent.params = 0;//pSapPhysLinkCreate

        /* Store the HDD callback in SAP context */
        pSapCtx->pfnSapEventCallback = pSapEventCallback;

        /* Handle event*/
        vosStatus = sapFsm(pSapCtx, &sapEvent);
     }
     else
     {
        VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_ERROR,
                "SoftAp role has not been enabled");
     }

    return vosStatus;
}// WLANSAP_StartBss

/*==========================================================================
  FUNCTION    WLANSAP_StopBss

  DESCRIPTION 
    This api function provides SAP FSM event eSAP_HDD_STOP_INFRA_BSS for
    stopping AP BSS

  DEPENDENCIES 

  PARAMETERS 

    IN
    pvosGCtx    : Pointer to the global vos context; a handle to SAP's 
                  control block can be extracted from its contexe

  RETURN VALUE
    The result code associated with performing the operation  

    VOS_STATUS_E_FAULT: Pointer to VOSS GC is NULL ; access would cause a page 
                         fault  
    VOS_STATUS_SUCCESS: Success

  SIDE EFFECTS
============================================================================*/
VOS_STATUS 
WLANSAP_StopBss
(
 v_PVOID_t  pvosGCtx
)
{
    tWLAN_SAPEvent sapEvent;    /* State machine event*/
    VOS_STATUS vosStatus = VOS_STATUS_SUCCESS;
    ptSapContext  pSapCtx = NULL;
    /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

    /*------------------------------------------------------------------------
        Sanity check
        Extract SAP control block 
    ------------------------------------------------------------------------*/
    VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_INFO_HIGH,
                 "WLANSAP_StopBss");

    if ( NULL == pvosGCtx ) 
    {
        VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_ERROR,
                "Invalid Global VOSS handle");
        return VOS_STATUS_E_FAULT;
    }

    pSapCtx = VOS_GET_SAP_CB(pvosGCtx);

    if (NULL == pSapCtx )
    {
        VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_ERROR,
                "Invalid SAP pointer from pvosGCtx on WLANSAP_Stop");
        return VOS_STATUS_E_FAULT;
    }

    /* Fill in the event structure for FSM */
    sapEvent.event = eSAP_HDD_STOP_INFRA_BSS;
    sapEvent.params = 0;

    /* Handle event*/
    vosStatus = sapFsm(pSapCtx, &sapEvent);

    return vosStatus;
}

/*==========================================================================
  FUNCTION    WLANSAP_GetAssocStations

  DESCRIPTION 
    This api function is used to probe the list of associated stations from various modules of CORE stack

  DEPENDENCIES 
    NA. 

  PARAMETERS

    IN
    pvosGCtx        : Pointer to vos global context structure
    modId           : Module from whom list of associtated stations  is supposed to be probed. If an invalid module is passed
                        then by default VOS_MODULE_ID_PE will be probed
    IN/OUT
    pAssocStas      : Pointer to list of associated stations that are known to the module specified in mod parameter

  NOTE: The memory for this list will be allocated by the caller of this API

  RETURN VALUE
    The VOS_STATUS code associated with performing the operation

    VOS_STATUS_SUCCESS:  Success

  SIDE EFFECTS
============================================================================*/
VOS_STATUS
WLANSAP_GetAssocStations
(
    v_PVOID_t pvosGCtx,
    VOS_MODULE_ID modId,
    tpSap_AssocMacAddr pAssocStas
)
{
    ptSapContext  pSapCtx = VOS_GET_SAP_CB(pvosGCtx);

    /*------------------------------------------------------------------------
      Sanity check
      Extract SAP control block 
      ------------------------------------------------------------------------*/
    if (NULL == pSapCtx)
    {
      VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_ERROR,
                    "Invalid SAP pointer from pvosGCtx on WLANSAP_GetAssocStations");
      return VOS_STATUS_E_FAULT;
    }

    sme_RoamGetAssociatedStas( VOS_GET_HAL_CB(pSapCtx->pvosGCtx), pSapCtx->sessionId,
                                modId,
                                pSapCtx->pUsrContext,
                                (v_PVOID_t *)pSapCtx->pfnSapEventCallback,
                                (v_U8_t *)pAssocStas );

    return VOS_STATUS_SUCCESS;
}


/*==========================================================================
  FUNCTION    WLANSAP_RemoveWpsSessionOverlap

  DESCRIPTION 
    This api function provides for Ap App/HDD to remove an entry from session session overlap info.

  DEPENDENCIES 
    NA. 

  PARAMETERS

    IN
    pvosGCtx: Pointer to vos global context structure
    pRemoveMac: pointer to v_MACADDR_t for session MAC address
   
  RETURN VALUE
    The VOS_STATUS code associated with performing the operation  

    VOS_STATUS_SUCCESS:  Success
    VOS_STATUS_E_FAULT:  Session is not dectected. The parameter is function not valid.
  
  SIDE EFFECTS   
============================================================================*/
VOS_STATUS
WLANSAP_RemoveWpsSessionOverlap

(
    v_PVOID_t pvosGCtx,
    v_MACADDR_t pRemoveMac
)
{
  ptSapContext  pSapCtx = VOS_GET_SAP_CB(pvosGCtx);

  /*------------------------------------------------------------------------
    Sanity check
    Extract SAP control block 
  ------------------------------------------------------------------------*/
  if (NULL == pSapCtx)
  {
    VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_ERROR,
        "Invalid SAP pointer from pvosGCtx on WLANSAP_RemoveWpsSessionOverlap");
    return VOS_STATUS_E_FAULT;
  }

  sme_RoamGetWpsSessionOverlap( VOS_GET_HAL_CB(pSapCtx->pvosGCtx), pSapCtx->sessionId,
                                pSapCtx->pUsrContext,
                                (v_PVOID_t *)pSapCtx->pfnSapEventCallback,
                                pRemoveMac);

  return VOS_STATUS_SUCCESS;
}

/*==========================================================================
  FUNCTION    WLANSAP_getWpsSessionOverlap

  DESCRIPTION 
    This api function provides for Ap App/HDD to get WPS session overlap info.

  DEPENDENCIES 
    NA. 

  PARAMETERS

    IN
    pvosGCtx: Pointer to vos global context structure
   
  RETURN VALUE
    The VOS_STATUS code associated with performing the operation  

    VOS_STATUS_SUCCESS:  Success
  
  SIDE EFFECTS   
============================================================================*/
VOS_STATUS
WLANSAP_getWpsSessionOverlap
(
 v_PVOID_t pvosGCtx  
)
{
    v_MACADDR_t pRemoveMac = VOS_MAC_ADDR_ZERO_INITIALIZER; 

    ptSapContext  pSapCtx = VOS_GET_SAP_CB(pvosGCtx);

    /*------------------------------------------------------------------------
      Sanity check
      Extract SAP control block 
      ------------------------------------------------------------------------*/
    if (NULL == pSapCtx)
    {
      VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_ERROR,
                    "Invalid SAP pointer from pvosGCtx on WLANSAP_getWpsSessionOverlap");
      return VOS_STATUS_E_FAULT;
    }
    
    sme_RoamGetWpsSessionOverlap( VOS_GET_HAL_CB(pSapCtx->pvosGCtx), pSapCtx->sessionId,
                                pSapCtx->pUsrContext,
                                (v_PVOID_t *)pSapCtx->pfnSapEventCallback,
                                pRemoveMac);

    return VOS_STATUS_SUCCESS;
}

/*==========================================================================
  FUNCTION    WLANSAP_DisassocSta

  DESCRIPTION 
    This api function provides for Ap App/HDD initiated disassociation of station

  DEPENDENCIES 
    NA. 

  PARAMETERS

    IN
    pvosGCtx            : Pointer to vos global context structure
    pPeerStaMac         : Mac address of the station to disassociate
   
  RETURN VALUE
    The VOS_STATUS code associated with performing the operation  

    VOS_STATUS_SUCCESS:  Success
  
  SIDE EFFECTS   
============================================================================*/
VOS_STATUS 
WLANSAP_DisassocSta
(
    v_PVOID_t  pvosGCtx,
    v_U8_t *pPeerStaMac
)
{
    ptSapContext  pSapCtx = VOS_GET_SAP_CB(pvosGCtx);

    /*------------------------------------------------------------------------
      Sanity check
      Extract SAP control block 
      ------------------------------------------------------------------------*/    
    if (NULL == pSapCtx)
    {
        VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_ERROR,
                    "Invalid SAP pointer from pvosGCtx on WLANSAP_GetAssocStations");
        return VOS_STATUS_E_FAULT;
    }

    sme_RoamDisconnectSta(VOS_GET_HAL_CB(pSapCtx->pvosGCtx), pSapCtx->sessionId,
                            pPeerStaMac);

    return VOS_STATUS_SUCCESS;
}

#ifdef WLAN_SOFTAP_FEATURE
/*==========================================================================
  FUNCTION    WLANSAP_DeauthSta

  DESCRIPTION 
    This api function provides for Ap App/HDD initiated deauthentication of station

  DEPENDENCIES 
    NA. 

  PARAMETERS

    IN
    pvosGCtx            : Pointer to vos global context structure
    pPeerStaMac         : Mac address of the station to deauthenticate
   
  RETURN VALUE
    The VOS_STATUS code associated with performing the operation  

    VOS_STATUS_SUCCESS:  Success
  
  SIDE EFFECTS   
============================================================================*/
VOS_STATUS 
WLANSAP_DeauthSta
(
    v_PVOID_t  pvosGCtx,
    v_U8_t *pPeerStaMac
)
{
    ptSapContext  pSapCtx = VOS_GET_SAP_CB(pvosGCtx);

    /*------------------------------------------------------------------------
      Sanity check
      Extract SAP control block 
      ------------------------------------------------------------------------*/    
    if (NULL == pSapCtx)
    {
        VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_ERROR,
                    "Invalid SAP pointer from pvosGCtx on WLANSAP_DeauthSta");
        return VOS_STATUS_E_FAULT;
    }

    sme_RoamDeauthSta(VOS_GET_HAL_CB(pSapCtx->pvosGCtx), pSapCtx->sessionId,
                            pPeerStaMac);

    return VOS_STATUS_SUCCESS;
}
#endif

/*==========================================================================
  FUNCTION    WLANSAP_SetCounterMeasure

  DESCRIPTION 
    This api function is used to disassociate all the stations and prevent 
    association for any other station.Whenever Authenticator receives 2 mic failures 
    within 60 seconds, Authenticator will enable counter measure at SAP Layer. 
    Authenticator will start the 60 seconds timer. Core stack will not allow any 
    STA to associate till HDD disables counter meassure. Core stack shall kick out all the 
    STA which are currently associated and DIASSOC Event will be propogated to HDD for 
    each STA to clean up the HDD STA table.Once the 60 seconds timer expires, Authenticator 
    will disable the counter meassure at core stack. Now core stack can allow STAs to associate.

  DEPENDENCIES 
    NA. 

  PARAMETERS

    IN
pvosGCtx: Pointer to vos global context structure
bEnable: If TRUE than all stations will be disassociated and no more will be allowed to associate. If FALSE than CORE
will come out of this state.
   
  RETURN VALUE
    The VOS_STATUS code associated with performing the operation  

    VOS_STATUS_SUCCESS:  Success
  
  SIDE EFFECTS   
============================================================================*/
VOS_STATUS
WLANSAP_SetCounterMeasure
(
    v_PVOID_t pvosGCtx,
    v_BOOL_t bEnable
)
{
    ptSapContext  pSapCtx = VOS_GET_SAP_CB(pvosGCtx);

    /*------------------------------------------------------------------------
      Sanity check
      Extract SAP control block 
      ------------------------------------------------------------------------*/
    if (NULL == pSapCtx)
    {
        VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_ERROR,
                    "Invalid SAP pointer from pvosGCtx on WLANSAP_GetAssocStations");
        return VOS_STATUS_E_FAULT;
    }

    sme_RoamTKIPCounterMeasures(VOS_GET_HAL_CB(pSapCtx->pvosGCtx), pSapCtx->sessionId, bEnable);

    return VOS_STATUS_SUCCESS;
}

/*==========================================================================

  FUNCTION    WLANSAP_SetKeysSta

  DESCRIPTION 
    This api function provides for Ap App/HDD to set key for a station.

  DEPENDENCIES 
    NA. 

  PARAMETERS

    IN
pvosGCtx: Pointer to vos global context structure
pSetKeyInfo: tCsrRoamSetKey structure for the station
   
  RETURN VALUE
    The VOS_STATUS code associated with performing the operation  

    VOS_STATUS_SUCCESS:  Success
  
  SIDE EFFECTS   
============================================================================*/
VOS_STATUS
WLANSAP_SetKeySta
(
    v_PVOID_t pvosGCtx, tCsrRoamSetKey *pSetKeyInfo
)
{
    VOS_STATUS vosStatus = VOS_STATUS_SUCCESS;
    ptSapContext  pSapCtx = NULL;
    v_PVOID_t hHal = NULL;
        eHalStatus halStatus = eHAL_STATUS_FAILURE;
        v_U32_t roamId=0xFF;

    if (VOS_STA_SAP_MODE == vos_get_conparam ( ))
    {
        pSapCtx = VOS_GET_SAP_CB(pvosGCtx);
        if (NULL == pSapCtx)
        {
            VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_ERROR,
                "Invalid SAP pointer from pvosGCtx on WLANSAP_SetKeySra");
            return VOS_STATUS_E_FAULT;
        }
        hHal = VOS_GET_HAL_CB(pSapCtx->pvosGCtx);
        if (NULL == hHal)
        {
            VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_ERROR,
                   "Invalid HAL pointer from pvosGCtx on WLANSAP_GetAssocStations");
            return VOS_STATUS_E_FAULT;
        }
        halStatus = sme_RoamSetKey(hHal, pSapCtx->sessionId, pSetKeyInfo, &roamId);

        if (halStatus == eHAL_STATUS_SUCCESS)
        {
            vosStatus = VOS_STATUS_SUCCESS;
        } else
        {
            vosStatus = VOS_STATUS_E_FAULT;
        }
    }
    else
        vosStatus = VOS_STATUS_E_FAULT;

    return vosStatus;
}

/*==========================================================================
  FUNCTION    WLANSAP_DelKeySta

  DESCRIPTION 
    This api function provides for Ap App/HDD to delete key for a station.

  DEPENDENCIES 
    NA. 

  PARAMETERS

    IN
pvosGCtx: Pointer to vos global context structure
pSetKeyInfo: tCsrRoamRemoveKey structure for the station
   
  RETURN VALUE
    The VOS_STATUS code associated with performing the operation  

    VOS_STATUS_SUCCESS:  Success
  
  SIDE EFFECTS   
============================================================================*/
VOS_STATUS
WLANSAP_DelKeySta
(
     v_PVOID_t pvosGCtx, 
    tCsrRoamRemoveKey *pRemoveKeyInfo
)
{
    VOS_STATUS vosStatus = VOS_STATUS_SUCCESS;
    ptSapContext  pSapCtx = NULL;
    v_PVOID_t hHal = NULL;
    eHalStatus halStatus = eHAL_STATUS_FAILURE;
    v_U32_t roamId=0xFF;
    tCsrRoamSetKey SetKeyInfo;
    tCsrRoamSetKey *pSetKeyInfo;

    pSetKeyInfo = &SetKeyInfo;

    if (VOS_STA_SAP_MODE == vos_get_conparam ( ))
    {
        pSapCtx = VOS_GET_SAP_CB(pvosGCtx);
        if (NULL == pSapCtx)
        { 
            VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_ERROR,
                "Invalid SAP pointer from pvosGCtx on WLANSAP_SetKeySra");
            return VOS_STATUS_E_FAULT;
        }

        hHal = VOS_GET_HAL_CB(pSapCtx->pvosGCtx);
        if (NULL == hHal)
        {
            VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_ERROR,
                   "Invalid HAL pointer from pvosGCtx on WLANSAP_GetAssocStations");
            return VOS_STATUS_E_FAULT;
        }

        pSetKeyInfo->encType = pRemoveKeyInfo->encType;
        vos_mem_copy(pSetKeyInfo->peerMac, pRemoveKeyInfo->peerMac, WNI_CFG_BSSID_LEN); 
        pSetKeyInfo->keyId = pRemoveKeyInfo->keyId;
        pSetKeyInfo->keyLength = 0;

        sme_RoamSetKey(hHal, pSapCtx->sessionId, pSetKeyInfo, &roamId);

        if (halStatus == eHAL_STATUS_SUCCESS)
        {
            vosStatus = VOS_STATUS_SUCCESS;
        }
        else
        {
            vosStatus = VOS_STATUS_E_FAULT;
        }
    }
    else
        vosStatus = VOS_STATUS_E_FAULT;

    return vosStatus;
}

VOS_STATUS
WLANSap_getstationIE_information(v_PVOID_t pvosGCtx, 
                                 v_U32_t   *pLen,
                                 v_U8_t    *pBuf)
{
    VOS_STATUS vosStatus = VOS_STATUS_E_FAILURE;
    ptSapContext  pSapCtx = NULL;
    v_U32_t len = 0;

    if (VOS_STA_SAP_MODE == vos_get_conparam ( )){
        pSapCtx = VOS_GET_SAP_CB(pvosGCtx);
        if (NULL == pSapCtx)
        { 
            VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_ERROR,
        "Invalid SAP pointer from pvosGCtx on WLANSAP_SetKeySra");
            return VOS_STATUS_E_FAULT;
        }
        if (pLen)
        {
            len = *pLen;
            *pLen = pSapCtx->nStaWPARSnReqIeLength;
            VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_ERROR,
                    "WPAIE len : %x\n", *pLen);
            if(pBuf)
            {
                if(len >= pSapCtx->nStaWPARSnReqIeLength)
                {
                    vos_mem_copy( pBuf, pSapCtx->pStaWpaRsnReqIE, pSapCtx->nStaWPARSnReqIeLength);
                    VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_ERROR,
                        "WPAIE: %2x:%2x:%2x:%2x:%2x:%2x\n", pBuf[0], pBuf[1],pBuf[2],pBuf[3],pBuf[4],pBuf[5]);
                    vosStatus = VOS_STATUS_SUCCESS;
                }
            }
        }
    }
   
    if( VOS_STATUS_E_FAILURE == vosStatus)
        VOS_TRACE(VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_ERROR, "Error Failed unable to populate the RSNWPAIE\n"); 

    return vosStatus;

}

/*==========================================================================
  FUNCTION    WLANSAP_Set_WpsIe

  DESCRIPTION 
    This api function provides for Ap App/HDD to set WPS IE.

  DEPENDENCIES 
    NA. 

  PARAMETERS

    IN
pvosGCtx: Pointer to vos global context structure
pWPSIE:  tSap_WPSIE structure that include WPS IEs
   
  RETURN VALUE
    The VOS_STATUS code associated with performing the operation  

    VOS_STATUS_SUCCESS:  Success
  
  SIDE EFFECTS   
============================================================================*/
VOS_STATUS
WLANSAP_Set_WpsIe
(
 v_PVOID_t pvosGCtx, tSap_WPSIE *pSap_WPSIe
)
{
    ptSapContext  pSapCtx = NULL;
    v_PVOID_t hHal = NULL;

    VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_ERROR,
            "%s, %d", __FUNCTION__, __LINE__);    
            
    if(VOS_STA_SAP_MODE == vos_get_conparam ( )) {
        pSapCtx = VOS_GET_SAP_CB(pvosGCtx);
        if ( NULL == pSapCtx )
        { 
            VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_ERROR,
                "Invalid SAP pointer from pvosGCtx on WLANSAP_Set_WpsIe");
            return VOS_STATUS_E_FAULT;
        }
        
        hHal = VOS_GET_HAL_CB(pSapCtx->pvosGCtx);
        if ( NULL == hHal ){
            VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_ERROR,
                "Invalid HAL pointer from pvosGCtx on WLANSAP_Set_WpsIe");
            return VOS_STATUS_E_FAULT;
        }
        
        if ( sap_AcquireGlobalLock( pSapCtx ) == VOS_STATUS_SUCCESS )
        {
            if (pSap_WPSIe->sapWPSIECode == eSAP_WPS_BEACON_IE)
            {
                vos_mem_copy(&pSapCtx->APWPSIEs.SirWPSBeaconIE, &pSap_WPSIe->sapwpsie.sapWPSBeaconIE, sizeof(tSap_WPSBeaconIE));    
            }
            else if (pSap_WPSIe->sapWPSIECode == eSAP_WPS_PROBE_RSP_IE) 
            {
                vos_mem_copy(&pSapCtx->APWPSIEs.SirWPSProbeRspIE, &pSap_WPSIe->sapwpsie.sapWPSProbeRspIE, sizeof(tSap_WPSProbeRspIE));
            }
            else
            {
                sap_ReleaseGlobalLock( pSapCtx );
                return VOS_STATUS_E_FAULT;
            }
            sap_ReleaseGlobalLock( pSapCtx );
            return VOS_STATUS_SUCCESS;
        }
        else
            return VOS_STATUS_E_FAULT;
    }
    else
        return VOS_STATUS_E_FAULT;
}

/*==========================================================================
  FUNCTION   WLANSAP_Update_WpsIe

  DESCRIPTION 
    This api function provides for Ap App/HDD to update WPS IEs.

  DEPENDENCIES
    NA. 

  PARAMETERS

    IN
pvosGCtx: Pointer to vos global context structure
   
  RETURN VALUE
    The VOS_STATUS code associated with performing the operation  

    VOS_STATUS_SUCCESS:  Success
  
  SIDE EFFECTS   
============================================================================*/
VOS_STATUS
WLANSAP_Update_WpsIe
(
 v_PVOID_t pvosGCtx
)
{
    VOS_STATUS vosStatus = VOS_STATUS_E_FAULT;
    ptSapContext  pSapCtx = NULL;
    eHalStatus halStatus = eHAL_STATUS_FAILURE;
    v_PVOID_t hHal = NULL;

    VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_ERROR,
            "%s, %d", __FUNCTION__, __LINE__);    
    
    if(VOS_STA_SAP_MODE == vos_get_conparam ( )){
        pSapCtx = VOS_GET_SAP_CB(pvosGCtx);
        if ( NULL == pSapCtx )
        { 
            VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_ERROR,
                "Invalid SAP pointer from pvosGCtx on WLANSAP_Set_WpsIe");
            return VOS_STATUS_E_FAULT;
        }

        hHal = VOS_GET_HAL_CB(pSapCtx->pvosGCtx);
        if ( NULL == hHal ){
            VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_ERROR,
                   "Invalid HAL pointer from pvosGCtx on WLANSAP_GetAssocStations");
            return VOS_STATUS_E_FAULT;
        }

        halStatus = sme_RoamUpdateAPWPSIE( hHal, pSapCtx->sessionId, &pSapCtx->APWPSIEs);

        if(halStatus == eHAL_STATUS_SUCCESS) {
            vosStatus = VOS_STATUS_SUCCESS;
        } else
        {
            vosStatus = VOS_STATUS_E_FAULT;
        }

    }

    return vosStatus;
}

/*==========================================================================
  FUNCTION    WLANSAP_Get_WPS_State

  DESCRIPTION 
    This api function provides for Ap App/HDD to check if WPS session in process.

  DEPENDENCIES
    NA. 

  PARAMETERS

    IN
pvosGCtx: Pointer to vos global context structure

    OUT
pbWPSState: Pointer to variable to indicate if it is in WPS Registration state
 
  RETURN VALUE
    The VOS_STATUS code associated with performing the operation  

    VOS_STATUS_SUCCESS:  Success
  
  SIDE EFFECTS   
============================================================================*/
VOS_STATUS
WLANSAP_Get_WPS_State
(
 v_PVOID_t pvosGCtx, v_BOOL_t *bWPSState
)
{
    ptSapContext  pSapCtx = NULL;
    v_PVOID_t hHal = NULL;

    VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_ERROR,
        "%s, %d", __FUNCTION__, __LINE__);    
          
    if(VOS_STA_SAP_MODE == vos_get_conparam ( )){
    
        pSapCtx = VOS_GET_SAP_CB(pvosGCtx);
        if ( NULL == pSapCtx )
        { 
            VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_ERROR,
                "Invalid SAP pointer from pvosGCtx on WLANSAP_Get_WPS_State");
             return VOS_STATUS_E_FAULT;
        }

        hHal = VOS_GET_HAL_CB(pSapCtx->pvosGCtx);
        if ( NULL == hHal ){
            VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_ERROR,
                "Invalid HAL pointer from pvosGCtx on WLANSAP_Get_WPS_State");
            return VOS_STATUS_E_FAULT;
        }
    
        if ( sap_AcquireGlobalLock(pSapCtx ) == VOS_STATUS_SUCCESS )
        {
            if(pSapCtx->APWPSIEs.SirWPSProbeRspIE.FieldPresent & SIR_WPS_PROBRSP_SELECTEDREGISTRA_PRESENT)
                *bWPSState = eANI_BOOLEAN_TRUE;
            else
                *bWPSState = eANI_BOOLEAN_FALSE;
            
            sap_ReleaseGlobalLock( pSapCtx  );
            
            return VOS_STATUS_SUCCESS;
        }
        else 
            return VOS_STATUS_E_FAULT;
    }
    else
        return VOS_STATUS_E_FAULT;

}

VOS_STATUS
sap_AcquireGlobalLock
(
    ptSapContext  pSapCtx 
)
{
    VOS_STATUS vosStatus = VOS_STATUS_E_FAULT;

    if( VOS_IS_STATUS_SUCCESS( vos_lock_acquire( &pSapCtx->SapGlobalLock) ) )
    {
            vosStatus = VOS_STATUS_SUCCESS;
    }

    return (vosStatus);
}

VOS_STATUS
sap_ReleaseGlobalLock
(
    ptSapContext  pSapCtx 
)
{
    VOS_STATUS vosStatus = VOS_STATUS_E_FAULT;

    if( VOS_IS_STATUS_SUCCESS( vos_lock_release( &pSapCtx->SapGlobalLock) ) )
    {
        vosStatus = VOS_STATUS_SUCCESS;
    }

    return (vosStatus);
}

/*==========================================================================
  FUNCTION    WLANSAP_Set_WPARSNIes

  DESCRIPTION 
    This api function provides for Ap App/HDD to set AP WPA and RSN IE in its beacon and probe response.

  DEPENDENCIES 
    NA. 

  PARAMETERS

    IN
        pvosGCtx: Pointer to vos global context structure
        pWPARSNIEs: buffer to the WPA/RSN IEs 
        WPARSNIEsLen: length of WPA/RSN IEs
   
  RETURN VALUE
    The VOS_STATUS code associated with performing the operation  

    VOS_STATUS_SUCCESS:  Success
  
  SIDE EFFECTS   
============================================================================*/
VOS_STATUS WLANSAP_Set_WPARSNIes(v_PVOID_t pvosGCtx, v_U8_t *pWPARSNIEs, v_U32_t WPARSNIEsLen)
{
 
    ptSapContext  pSapCtx = NULL;
    eHalStatus halStatus = eHAL_STATUS_FAILURE;
    v_PVOID_t hHal = NULL;

    if(VOS_STA_SAP_MODE == vos_get_conparam ( )){
        pSapCtx = VOS_GET_SAP_CB(pvosGCtx);
        if ( NULL == pSapCtx )
        { 
            VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_ERROR,
                "Invalid SAP pointer from pvosGCtx on WLANSAP_Set_WPARSNIes");
            return VOS_STATUS_E_FAULT;
        }

        hHal = VOS_GET_HAL_CB(pSapCtx->pvosGCtx);
        if ( NULL == hHal ){
            VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_ERROR,
                   "Invalid HAL pointer from pvosGCtx on WLANSAP_Set_WPARSNIes");
            return VOS_STATUS_E_FAULT;
        }
        
        pSapCtx->APWPARSNIEs.length = (tANI_U16)WPARSNIEsLen;
        vos_mem_copy(pSapCtx->APWPARSNIEs.rsnIEdata, pWPARSNIEs, WPARSNIEsLen);
    
        halStatus = sme_RoamUpdateAPWPARSNIEs( hHal, pSapCtx->sessionId, &pSapCtx->APWPARSNIEs);

        if(halStatus == eHAL_STATUS_SUCCESS) {
            return VOS_STATUS_SUCCESS;
        } else
        {
            return VOS_STATUS_E_FAULT;
        }
    }

    return VOS_STATUS_E_FAULT;    
}

VOS_STATUS WLANSAP_GetStatistics(v_PVOID_t pvosGCtx, tSap_SoftapStats *statBuf, v_BOOL_t bReset)
{
    if (NULL == pvosGCtx)
    {
        return VOS_STATUS_E_FAULT;
    }

    return (WLANTL_GetSoftAPStatistics(pvosGCtx, statBuf, bReset));
}

#ifdef WLAN_FEATURE_P2P
/*==========================================================================

  FUNCTION    WLANSAP_SendAction

  DESCRIPTION 
    This api function provides to send action frame sent by upper layer.

  DEPENDENCIES 
    NA. 

  PARAMETERS

  IN
    pvosGCtx: Pointer to vos global context structure
    pBuf: Pointer of the action frame to be transmitted
    len: Length of the action frame
   
  RETURN VALUE
    The VOS_STATUS code associated with performing the operation  

    VOS_STATUS_SUCCESS:  Success
  
  SIDE EFFECTS   
============================================================================*/
VOS_STATUS WLANSAP_SendAction( v_PVOID_t pvosGCtx, const tANI_U8 *pBuf,
                               tANI_U32 len, tANI_U16 wait )
{
    ptSapContext  pSapCtx = NULL;
    v_PVOID_t hHal = NULL;
    eHalStatus halStatus = eHAL_STATUS_FAILURE;

    if( VOS_STA_SAP_MODE == vos_get_conparam ( ) )
    {
        pSapCtx = VOS_GET_SAP_CB( pvosGCtx );
        if (NULL == pSapCtx)
        {
            VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_ERROR,
                "Invalid SAP pointer from pvosGCtx on WLANSAP_SendAction");
            return VOS_STATUS_E_FAULT;
        }
        hHal = VOS_GET_HAL_CB(pSapCtx->pvosGCtx);
        if( ( NULL == hHal ) || ( eSAP_TRUE != pSapCtx->isSapSessionOpen ) )
        {
            VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_ERROR,
              "HAL pointer (%p) NULL OR SME session is not open (%d)",
              hHal, pSapCtx->isSapSessionOpen );
            return VOS_STATUS_E_FAULT;
        }

        halStatus = sme_sendAction( hHal, pSapCtx->sessionId, pBuf, len, 0, 0 );

        if ( eHAL_STATUS_SUCCESS == halStatus )
        {
            return VOS_STATUS_SUCCESS;
        }
    }

    VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_ERROR,
               "Failed to Send Action Frame");

    return VOS_STATUS_E_FAULT;
}

/*==========================================================================

  FUNCTION    WLANSAP_RemainOnChannel

  DESCRIPTION 
    This api function provides to set Remain On channel on specified channel
    for specified duration.

  DEPENDENCIES 
    NA. 

  PARAMETERS

  IN
    pvosGCtx: Pointer to vos global context structure
    channel: Channel on which driver has to listen 
    duration: Duration for which driver has to listen on specified channel
    callback: Callback function to be called once Listen is done.
    pContext: Context needs to be called in callback function. 
   
  RETURN VALUE
    The VOS_STATUS code associated with performing the operation  

    VOS_STATUS_SUCCESS:  Success
  
  SIDE EFFECTS   
============================================================================*/
VOS_STATUS WLANSAP_RemainOnChannel( v_PVOID_t pvosGCtx,
                                    tANI_U8 channel, tANI_U32 duration,
                                    remainOnChanCallback callback,
                                    void *pContext )
{
    ptSapContext  pSapCtx = NULL;
    v_PVOID_t hHal = NULL;
    eHalStatus halStatus = eHAL_STATUS_FAILURE;

    if( VOS_STA_SAP_MODE == vos_get_conparam ( ) )
    {
        pSapCtx = VOS_GET_SAP_CB( pvosGCtx );
        if (NULL == pSapCtx)
        {
            VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_ERROR,
                "Invalid SAP pointer from pvosGCtx on WLANSAP_SendAction");
            return VOS_STATUS_E_FAULT;
        }
        hHal = VOS_GET_HAL_CB(pSapCtx->pvosGCtx);
        if( ( NULL == hHal ) || ( eSAP_TRUE != pSapCtx->isSapSessionOpen ) )
        {
            VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_ERROR,
              "HAL pointer (%p) NULL OR SME session is not open (%d)",
              hHal, pSapCtx->isSapSessionOpen );
            return VOS_STATUS_E_FAULT;
        }

        halStatus = sme_RemainOnChannel( hHal, pSapCtx->sessionId,
                          channel, duration, callback, pContext );

        if( eHAL_STATUS_SUCCESS == halStatus )
        {
            return VOS_STATUS_SUCCESS;
        }
    }

    VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_ERROR,
               "Failed to Set Remain on Channel");

    return VOS_STATUS_E_FAULT;
}

/*==========================================================================

  FUNCTION    WLANSAP_CancelRemainOnChannel

  DESCRIPTION 
    This api cancel previous remain on channel request.

  DEPENDENCIES 
    NA. 

  PARAMETERS

  IN
    pvosGCtx: Pointer to vos global context structure
   
  RETURN VALUE
    The VOS_STATUS code associated with performing the operation  

    VOS_STATUS_SUCCESS:  Success
  
  SIDE EFFECTS   
============================================================================*/
VOS_STATUS WLANSAP_CancelRemainOnChannel( v_PVOID_t pvosGCtx )
{
    ptSapContext  pSapCtx = NULL;
    v_PVOID_t hHal = NULL;
    eHalStatus halStatus = eHAL_STATUS_FAILURE;

    if( VOS_STA_SAP_MODE == vos_get_conparam ( ) )
    {
        pSapCtx = VOS_GET_SAP_CB( pvosGCtx );
        if (NULL == pSapCtx)
        {
            VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_ERROR,
                "Invalid SAP pointer from pvosGCtx on WLANSAP_SendAction");
            return VOS_STATUS_E_FAULT;
        }
        hHal = VOS_GET_HAL_CB(pSapCtx->pvosGCtx);
        if( ( NULL == hHal ) || ( eSAP_TRUE != pSapCtx->isSapSessionOpen ) )
        {
            VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_ERROR,
              "HAL pointer (%p) NULL OR SME session is not open (%d)",
              hHal, pSapCtx->isSapSessionOpen );
            return VOS_STATUS_E_FAULT;
        }

        halStatus = sme_CancelRemainOnChannel( hHal, pSapCtx->sessionId );

        if( eHAL_STATUS_SUCCESS == halStatus )
        {
            return VOS_STATUS_SUCCESS;
        }
    }

    VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_ERROR,
                    "Failed to Cancel Remain on Channel");

    return VOS_STATUS_E_FAULT;
}

/*==========================================================================

  FUNCTION    WLANSAP_RegisterMgmtFrame

  DESCRIPTION 
    HDD use this API to register specified type of frame with CORE stack.
    On receiving such kind of frame CORE stack should pass this frame to HDD

  DEPENDENCIES 
    NA. 

  PARAMETERS

  IN
    pvosGCtx: Pointer to vos global context structure
    frameType: frameType that needs to be registered with PE.
    matchData: Data pointer which should be matched after frame type is matched.
    matchLen: Length of the matchData
   
  RETURN VALUE
    The VOS_STATUS code associated with performing the operation  

    VOS_STATUS_SUCCESS:  Success
  
  SIDE EFFECTS   
============================================================================*/
VOS_STATUS WLANSAP_RegisterMgmtFrame( v_PVOID_t pvosGCtx, tANI_U16 frameType,
                                      tANI_U8* matchData, tANI_U16 matchLen )
{
    ptSapContext  pSapCtx = NULL;
    v_PVOID_t hHal = NULL;
    eHalStatus halStatus = eHAL_STATUS_FAILURE;

    if( VOS_STA_SAP_MODE == vos_get_conparam ( ) )
    {
        pSapCtx = VOS_GET_SAP_CB( pvosGCtx );
        if (NULL == pSapCtx)
        {
            VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_ERROR,
                "Invalid SAP pointer from pvosGCtx on WLANSAP_SendAction");
            return VOS_STATUS_E_FAULT;
        }
        hHal = VOS_GET_HAL_CB(pSapCtx->pvosGCtx);
        if( ( NULL == hHal ) || ( eSAP_TRUE != pSapCtx->isSapSessionOpen ) )
        {
            VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_ERROR,
              "HAL pointer (%p) NULL OR SME session is not open (%d)",
              hHal, pSapCtx->isSapSessionOpen );
            return VOS_STATUS_E_FAULT;
        }

        halStatus = sme_RegisterMgmtFrame(hHal, pSapCtx->sessionId,
                          frameType, matchData, matchLen);

        if( eHAL_STATUS_SUCCESS == halStatus )
        {
            return VOS_STATUS_SUCCESS;
        }
    }

    VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_ERROR,
                    "Failed to Register MGMT frame");

    return VOS_STATUS_E_FAULT;
}

/*==========================================================================

  FUNCTION    WLANSAP_DeRegisterMgmtFrame

  DESCRIPTION 
   This API is used to deregister previously registered frame. 

  DEPENDENCIES 
    NA. 

  PARAMETERS

  IN
    pvosGCtx: Pointer to vos global context structure
    frameType: frameType that needs to be De-registered with PE.
    matchData: Data pointer which should be matched after frame type is matched.
    matchLen: Length of the matchData
   
  RETURN VALUE
    The VOS_STATUS code associated with performing the operation  

    VOS_STATUS_SUCCESS:  Success
  
  SIDE EFFECTS   
============================================================================*/
VOS_STATUS WLANSAP_DeRegisterMgmtFrame( v_PVOID_t pvosGCtx, tANI_U16 frameType,
                                      tANI_U8* matchData, tANI_U16 matchLen )
{
    ptSapContext  pSapCtx = NULL;
    v_PVOID_t hHal = NULL;
    eHalStatus halStatus = eHAL_STATUS_FAILURE;

    if( VOS_STA_SAP_MODE == vos_get_conparam ( ) )
    {
        pSapCtx = VOS_GET_SAP_CB( pvosGCtx );
        if (NULL == pSapCtx)
        {
            VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_ERROR,
                "Invalid SAP pointer from pvosGCtx on WLANSAP_SendAction");
            return VOS_STATUS_E_FAULT;
        }
        hHal = VOS_GET_HAL_CB(pSapCtx->pvosGCtx);
        if( ( NULL == hHal ) || ( eSAP_TRUE != pSapCtx->isSapSessionOpen ) )
        {
            VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_ERROR,
              "HAL pointer (%p) NULL OR SME session is not open (%d)",
              hHal, pSapCtx->isSapSessionOpen );
            return VOS_STATUS_E_FAULT;
        }

        halStatus = sme_DeregisterMgmtFrame( hHal, pSapCtx->sessionId,
                          frameType, matchData, matchLen );

        if( eHAL_STATUS_SUCCESS == halStatus )
        {
            return VOS_STATUS_SUCCESS;
        }
    }

    VOS_TRACE( VOS_MODULE_ID_SAP, VOS_TRACE_LEVEL_ERROR,
                    "Failed to Deregister MGMT frame");

    return VOS_STATUS_E_FAULT;
}
#endif // WLAN_FEATURE_P2P

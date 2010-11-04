#ifdef WLAN_FEATURE_VOWIFI_11R
/**=========================================================================

  \brief Definitions for SME FT APIs

   Copyright 2008 (c) Qualcomm, Incorporated.  All Rights Reserved.

   Qualcomm Confidential and Proprietary.

  ========================================================================*/

/*--------------------------------------------------------------------------
  Include Files
  ------------------------------------------------------------------------*/
#include <halInternal.h>
#include <smsDebug.h>
#include <csrInsideApi.h>
#include <csrNeighborRoam.h>

/*--------------------------------------------------------------------------
  Initialize the FT context. 
  ------------------------------------------------------------------------*/
void sme_FTOpen(tHalHandle hHal)
{
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );
    eHalStatus     status = eHAL_STATUS_SUCCESS;

    pMac->ft.ftSmeContext.auth_ft_ies = NULL;                        
    pMac->ft.ftSmeContext.auth_ft_ies_length = 0;                        

    pMac->ft.ftSmeContext.reassoc_ft_ies = NULL;                        
    pMac->ft.ftSmeContext.reassoc_ft_ies_length = 0;       

    status = palTimerAlloc(pMac->hHdd, &pMac->ft.ftSmeContext.preAuthReassocIntvlTimer, 
                            sme_PreauthReassocIntvlTimerCallback, (void *)pMac);

    if (eHAL_STATUS_SUCCESS != status)
    {
        smsLog(pMac, LOGE, FL("Preauth Reassoc interval Timer allocation failed"));
        return;
    }                 

    pMac->ft.ftSmeContext.FTState = eFT_START_READY;
}

/*--------------------------------------------------------------------------
  Cleanup the SME FT Global context. 
  ------------------------------------------------------------------------*/
void sme_FTClose(tHalHandle hHal)
{
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

    if (pMac->ft.ftSmeContext.auth_ft_ies != NULL)
    {
#if defined WLAN_FEATURE_VOWIFI_11R_DEBUG
        smsLog( pMac, LOGE, "%s: Freeing %p and setting to NULL\n", 
            __func__, pMac->ft.ftSmeContext.auth_ft_ies);
#endif
        vos_mem_free(pMac->ft.ftSmeContext.auth_ft_ies);
        pMac->ft.ftSmeContext.auth_ft_ies = NULL;
    }
    pMac->ft.ftSmeContext.auth_ft_ies_length = 0;                        

    if (pMac->ft.ftSmeContext.reassoc_ft_ies == NULL)
    {
#if defined WLAN_FEATURE_VOWIFI_11R_DEBUG
        smsLog( pMac, LOGE, "%s: Freeing %p and setting to NULL\n", 
            __func__, pMac->ft.ftSmeContext.reassoc_ft_ies);
#endif
        vos_mem_free(pMac->ft.ftSmeContext.reassoc_ft_ies);
        pMac->ft.ftSmeContext.reassoc_ft_ies = NULL;                        
    }
    pMac->ft.ftSmeContext.reassoc_ft_ies_length = 0;                        

    pMac->ft.ftSmeContext.FTState = eFT_START_READY;
    vos_mem_zero(pMac->ft.ftSmeContext.preAuthbssId, ANI_MAC_ADDR_SIZE);

    if (pMac->ft.ftSmeContext.psavedFTPreAuthRsp == NULL)
    {
#if defined WLAN_FEATURE_VOWIFI_11R_DEBUG
        smsLog( pMac, LOGE, "%s: Freeing %p and setting to NULL\n", 
            __func__, pMac->ft.ftSmeContext.psavedFTPreAuthRsp);
#endif
        vos_mem_free(pMac->ft.ftSmeContext.psavedFTPreAuthRsp);
        pMac->ft.ftSmeContext.psavedFTPreAuthRsp = NULL;                        
    }
}



/*--------------------------------------------------------------------------
  Find if the MIC control is non-zero, when we get Reassoc Req. 
  ------------------------------------------------------------------------*/
static inline int isMICcontrolNonZero(tANI_U8 *ft_ies)
{
    return TRUE;
}

/*--------------------------------------------------------------------------
  Each time the supplicant sends down the FT IEs to the driver.
  This function is called in SME. This fucntion packages and sends
  the FT IEs to PE.
  ------------------------------------------------------------------------*/
void sme_SetFTIEs( tHalHandle hHal, tANI_U8 sessionId, tANI_U8 *ft_ies, 
        tANI_U32 ft_ies_length )
{
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );
    eHalStatus status = eHAL_STATUS_FAILURE;

    status = sme_AcquireGlobalLock( &pMac->sme );
    if (!( HAL_STATUS_SUCCESS( status ))) return;

    if (ft_ies == NULL) 
    {
        smsLog( pMac, LOGE, "%s: ft ies is NULL\n", __func__);
        sme_ReleaseGlobalLock( &pMac->sme );
        return; 
    }

#if defined WLAN_FEATURE_VOWIFI_11R_DEBUG
    smsLog( pMac, LOGE, "FT IEs Req is received in state %d\n", 
        pMac->ft.ftSmeContext.FTState);
#endif

    // Global Station FT State
    switch(pMac->ft.ftSmeContext.FTState)
    {
        case eFT_START_READY:
        case eFT_AUTH_REQ_READY:
            if ((pMac->ft.ftSmeContext.auth_ft_ies) && 
                    (pMac->ft.ftSmeContext.auth_ft_ies_length))
            {
                // Free the one we received last from the supplicant
                vos_mem_free(pMac->ft.ftSmeContext.auth_ft_ies);
                pMac->ft.ftSmeContext.auth_ft_ies_length = 0; 
            }

            // Save the FT IEs
            pMac->ft.ftSmeContext.auth_ft_ies = vos_mem_malloc(ft_ies_length);
            pMac->ft.ftSmeContext.auth_ft_ies_length = ft_ies_length;
            vos_mem_copy((tANI_U8 *)pMac->ft.ftSmeContext.auth_ft_ies, ft_ies,
                ft_ies_length);
                
            pMac->ft.ftSmeContext.FTState = eFT_AUTH_REQ_READY;

#if defined WLAN_FEATURE_VOWIFI_11R_DEBUG
            smsLog( pMac, LOGE, "ft_ies_length=%d\n", ft_ies_length);
            smsLog( pMac, LOGE, "%d: New Auth ft_ies_length=%02x%02x%02x\n", 
                current->pid, pMac->ft.ftSmeContext.auth_ft_ies[0],
                pMac->ft.ftSmeContext.auth_ft_ies[1],
                pMac->ft.ftSmeContext.auth_ft_ies[2]);
#endif
            break;

        case eFT_AUTH_COMPLETE:
            // We will need to re-start preauth. If we received FT IEs in
            // eFT_PRE_AUTH_DONE state, it implies there was a rekey in 
            // our pre-auth state. Hence this implies we need Pre-auth again.

            // OK now inform SME we have no pre-auth list.
            // Delete the pre-auth node locally. Set your self back to restart pre-auth
            // TBD
#if defined WLAN_FEATURE_VOWIFI_11R_DEBUG
            smsLog( pMac, LOGE, 
                "Pre-auth done and now receiving---> AUTH REQ <---- in state %d\n", 
                pMac->ft.ftSmeContext.FTState);
            smsLog( pMac, LOGE, "Unhandled reception of FT IES in state %d\n", 
                pMac->ft.ftSmeContext.FTState);
#endif
            break;

        case eFT_REASSOC_REQ_WAIT:
            // We are done with pre-auth, hence now waiting for
            // reassoc req. This is the new FT Roaming in place

            // At this juncture we are ready to start sending Re-Assoc Req.
#if defined WLAN_FEATURE_VOWIFI_11R_DEBUG
            smsLog( pMac, LOGE, "New Reassoc Req=%p in state %d\n", 
                ft_ies, pMac->ft.ftSmeContext.FTState);
#endif
            if ((pMac->ft.ftSmeContext.reassoc_ft_ies) && 
                (pMac->ft.ftSmeContext.reassoc_ft_ies_length))
            {
                // Free the one we received last from the supplicant
                vos_mem_free(pMac->ft.ftSmeContext.reassoc_ft_ies);
                pMac->ft.ftSmeContext.reassoc_ft_ies_length = 0; 
            }
            if (isMICcontrolNonZero(ft_ies) != TRUE) 
            {
                smsLog( pMac, LOGE, 
                    "%s: MIC is 0, while waiting for reassoc req ft_ies_length=%d\n", 
                    __func__, ft_ies_length);
            }

            // Save the FT IEs
            pMac->ft.ftSmeContext.reassoc_ft_ies = vos_mem_malloc(ft_ies_length);
            pMac->ft.ftSmeContext.reassoc_ft_ies_length = ft_ies_length;
            vos_mem_copy((tANI_U8 *)pMac->ft.ftSmeContext.reassoc_ft_ies, ft_ies,
                ft_ies_length);
                
            pMac->ft.ftSmeContext.FTState = eFT_START_READY;
#if defined WLAN_FEATURE_VOWIFI_11R_DEBUG
            smsLog( pMac, LOGE, "ft_ies_length=%d state=%d\n", ft_ies_length,
                pMac->ft.ftSmeContext.FTState);
            smsLog( pMac, LOGE, "%d: New Auth ft_ies_length=%02x%02x%02x\n", 
                current->pid, pMac->ft.ftSmeContext.reassoc_ft_ies[0],
                pMac->ft.ftSmeContext.reassoc_ft_ies[1],
                pMac->ft.ftSmeContext.reassoc_ft_ies[2]);
#endif
            
            break;

        default:
            smsLog( pMac, LOGE, "%s: Unhandled state=%d\n", __func__,
                pMac->ft.ftSmeContext.FTState);
            break;
    }
    sme_ReleaseGlobalLock( &pMac->sme );
}
            
/*--------------------------------------------------------------------------
 *
 * HDD Interface to SME. SME now sends the Auth 2 and RIC IEs up to the supplicant.
 * The supplicant will then proceed to send down the
 * Reassoc Req.
 *
 *------------------------------------------------------------------------*/
void sme_GetFTPreAuthResponse( tHalHandle hHal, tANI_U8 *ft_ies, tANI_U32 *ft_ies_length )
{
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );
    eHalStatus status = eHAL_STATUS_FAILURE;

    status = sme_AcquireGlobalLock( &pMac->sme );
    if (!( HAL_STATUS_SUCCESS( status ))) return;

    // hdd needs to pack the bssid also along with the 
    // auth response to supplicant
    vos_mem_copy(ft_ies, pMac->ft.ftSmeContext.preAuthbssId, ANI_MAC_ADDR_SIZE);

    // Copy the auth resp FTIEs
    *ft_ies_length = 0;
    *ft_ies_length = pMac->ft.ftSmeContext.psavedFTPreAuthRsp->ft_ies_length;
    if (*ft_ies_length > (MAX_FTIE_SIZE-ANI_MAC_ADDR_SIZE)) 
    {
        smsLog( pMac, LOGE, "%s: FTIES length bigger than 256 = %d\n", __func__, *ft_ies_length);
        smsLog( pMac, LOGE, "%s: Unable to get Reassoc Req from supplicant\n", __func__, *ft_ies_length);
        sme_ReleaseGlobalLock( &pMac->sme );
        return;
    }

    if (*ft_ies_length) 
    {
        vos_mem_copy(&(ft_ies[ANI_MAC_ADDR_SIZE]), 
            pMac->ft.ftSmeContext.psavedFTPreAuthRsp->ft_ies, 
            pMac->ft.ftSmeContext.psavedFTPreAuthRsp->ft_ies_length);

        // Wait for the reassoc req. We now have passed auth resp 2 as follows
        // SME->HDD->Supplicant and waiting for the reassoc req.
        pMac->ft.ftSmeContext.FTState = eFT_REASSOC_REQ_WAIT;
    }

#ifdef WLAN_FEATURE_VOWIFI_11R_DEBUG
    smsLog( pMac, LOGE, "%s: Filled auth resp = %d\n", __func__, *ft_ies_length);
#endif

    sme_ReleaseGlobalLock( &pMac->sme );
    return;
}

/*--------------------------------------------------------------------------
 *
 * SME now sends the RIC IEs up to the supplicant.
 * The supplicant will then proceed to send down the
 * Reassoc Req.
 *
 *------------------------------------------------------------------------*/
void sme_GetRICIEs( tHalHandle hHal, tANI_U8 *ric_ies, tANI_U32 *ric_ies_length )
{
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );
    eHalStatus status = eHAL_STATUS_FAILURE;

    status = sme_AcquireGlobalLock( &pMac->sme );
    if (!( HAL_STATUS_SUCCESS( status ))) return;

    // Copy the ric ies 
    *ric_ies_length = 0;

    if (pMac->ft.ftSmeContext.psavedFTPreAuthRsp->ric_ies_length) 
    {
        *ric_ies_length = pMac->ft.ftSmeContext.psavedFTPreAuthRsp->ric_ies_length;
        vos_mem_copy(ric_ies, pMac->ft.ftSmeContext.psavedFTPreAuthRsp->ric_ies, 
            pMac->ft.ftSmeContext.psavedFTPreAuthRsp->ric_ies_length);
    }

#ifdef WLAN_FEATURE_VOWIFI_11R_DEBUG
    smsLog( pMac, LOGE, "%s: Filled ric ies = %d\n", __func__, *ric_ies_length);
#endif

    sme_ReleaseGlobalLock( &pMac->sme );
    return;
}

/*--------------------------------------------------------------------------
 *
 * Timer callback for the timer that is started between the preauth completion and 
 * reassoc request to the PE. In this interval, it is expected that the pre-auth response 
 * and RIC IEs are passed up to the WPA supplicant and received back the necessary FTIEs 
 * required to be sent in the reassoc request
 *
 *------------------------------------------------------------------------*/
void sme_PreauthReassocIntvlTimerCallback(void *context)
{
    tpAniSirGlobal pMac = PMAC_STRUCT(context);

#ifdef WLAN_FEATURE_NEIGHBOR_ROAMING
    csrNeighborRoamRequestHandoff(pMac);
#endif
    return;
}
/* End of File */
#endif /* WLAN_FEATURE_VOWIFI_11R */

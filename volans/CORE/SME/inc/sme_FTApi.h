#if !defined( __SME_FTAPI_H )
#define __SME_FTAPI_H

#include <limFTDefs.h>
#include <palTimer.h>

/**=========================================================================
  
  \brief macros and prototype for SME APIs
  
   Copyright 2008 (c) Qualcomm Technologies, Inc.  All Rights Reserved.
   
   Qualcomm Technologies Confidential and Proprietary.
  
  ========================================================================*/
typedef enum eFTIEState
{
    eFT_START_READY,                // Start before and after 11r assoc
    eFT_AUTH_REQ_READY,             // When we have recvd the 1st or nth auth req
    eFT_WAIT_AUTH2,                 // Sent auth1 and waiting auth2
    eFT_AUTH_COMPLETE,              // We are now ready for FT phase, send auth1, recd auth2
    eFT_REASSOC_REQ_WAIT,           // Now we have sent Auth Rsp to the supplicant and waiting
                                    // Reassoc Req from the supplicant.
} tFTIEStates;


typedef struct sFTSMEContext
{
    tANI_U8           *auth_ft_ies;
    tANI_U32          auth_ft_ies_length;

    tANI_U8           *reassoc_ft_ies;
    tANI_U16          reassoc_ft_ies_length;

    // Pre-Auth info
    tFTIEStates       FTState;               // The state of FT in the current 11rAssoc
    tSirMacAddr       preAuthbssId;          // BSSID to preauth to
    tANI_U32          smeSessionId;    

    // Saved pFTPreAuthRsp
    tpSirFTPreAuthRsp psavedFTPreAuthRsp;

    // Time to trigger reassoc once pre-auth is successful
    tPalTimerHandle   preAuthReassocIntvlTimer;

} tftSMEContext, *tpftSMEContext;

/*--------------------------------------------------------------------------
  Prototype functions
  ------------------------------------------------------------------------*/
void sme_FTOpen(tHalHandle hHal);
void sme_FTClose(tHalHandle hHal);
void sme_SetFTIEs( tHalHandle hHal, tANI_U8 sessionId, tANI_U8 *ft_ies, tANI_U16 ft_ies_length );
void csrFTPreAuthRspProcessor( tHalHandle hHal, tpSirFTPreAuthRsp pFTPreAuthRsp );
void sme_GetFTPreAuthResponse( tHalHandle hHal, tANI_U8 *ft_ies, tANI_U32 ft_ies_ip_len, tANI_U16 *ft_ies_length );
void sme_GetRICIEs( tHalHandle hHal, tANI_U8 *ric_ies, tANI_U32 ric_ies_ip_len, tANI_U32 *ric_ies_length );
void sme_PreauthReassocIntvlTimerCallback(void *context);


#endif //#if !defined( __SME_FTAPI_H )

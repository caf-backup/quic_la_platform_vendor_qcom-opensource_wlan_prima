
#if!defined( __LIM_SESSION_UTILS_H )
#define __LIM_SESSION_UTILS_H


/**=========================================================================
  
  \file  limSessionUtils.h
  
  \brief prototype for lim Session Utility related APIs

  \author Sunit Bhatia
  
   Copyright 2008 (c) Qualcomm, Incorporated.  All Rights Reserved.
   
   Qualcomm Confidential and Proprietary.
  
  ========================================================================*/


/*--------------------------------------------------------------------------
  Include Files
  ------------------------------------------------------------------------*/



/*-------------------------------------------------------------------------- 
  Preprocessor definitions and constants
  ------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------- 
  Type declarations
  ------------------------------------------------------------------------*/


/*------------------------------------------------------------------------- 
  Function declarations and documenation
  ------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------
  \brief peValidateJoinReq() - validates the Join request .

  This function is called to validate the Join Request for a BT-AMP station. If start BSS session is present
  this function returns TRUE else returns FALSE.
    
  \param pMac                   - pointer to global adapter context
  \return                           - return TRUE if start BSS session is present else return FALSE.
  
  \sa
  --------------------------------------------------------------------------*/
tANI_U8 peValidateBtJoinRequest(tpAniSirGlobal pMac);

/* --------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------
  \brief peGetValidPowerSaveSession() - Fetches the valid session for powersave .

  This function is called to check the valid session for power save, if more than one session is active , this function 
  it returns NULL.
  if there is only one valid "infrastructure" session present in "linkestablished" state this function returns sessionentry.
  For all other cases it returns NULL.
    
  \param pMac                   - pointer to global adapter context
  \return                            - return session is address if valid session is  present else return NULL.
  
  \sa
  --------------------------------------------------------------------------*/


tpPESession peGetValidPowerSaveSession(tpAniSirGlobal pMac);

/* --------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------
  \brief peIsAnySessionActive() - checks for the active session presence .

  This function returns TRUE if atleast one valid session is present else it returns FALSE
      
  \param pMac                   - pointer to global adapter context
  \return                            - return TRUE if atleast one session is active else return FALSE.
  
  \sa
  --------------------------------------------------------------------------*/

tANI_U8 peIsAnySessionActive(tpAniSirGlobal pMac);
/* --------------------------------------------------------------------------*/



/*--------------------------------------------------------------------------
  \brief isLimSessionOffChannel() - Determines if the session is 
                                        off channel.

  This function returns TRUE if the session Id passed needs to be on a different
  channel than atleast one session already active.
    
  \param pMac                   - pointer to global adapter context
  \param sessionId              - session ID of the session to be verified.  
  
  \return tANI_U8               - Boolean value for off-channel operation.
  
  \sa
  --------------------------------------------------------------------------*/
tANI_U8
isLimSessionOffChannel(tpAniSirGlobal pMac, tANI_U8 sessionId);
/* --------------------------------------------------------------------------*/

tANI_U8
peGetActiveSessionChannel( tpAniSirGlobal pMac );
#endif //#if !defined( __LIM_SESSION_UTILS_H )


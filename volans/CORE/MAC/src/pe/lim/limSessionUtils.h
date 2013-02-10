
#if!defined( __LIM_SESSION_UTILS_H )
#define __LIM_SESSION_UTILS_H


/**=========================================================================
  
  \file  limSessionUtils.h
  
  \brief prototype for lim Session Utility related APIs

  \author Sunit Bhatia
  
   Copyright 2008 (c) Qualcomm Technologies, Inc.  All Rights Reserved.
   
   Qualcomm Technologies Confidential and Proprietary.
  
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
  
  \brief peGetCurrentChannel() - Returns the  channel number for scanning, from a valid session.

  This function itrates the session Table and returns the channel number from first valid session
   if no sessions are valid it returns 0
    
  \param pMac                   - pointer to global adapter context
  \return                            - channel to scan from valid session else zero.
  
  \sa
  
  --------------------------------------------------------------------------*/
tANI_U8 peGetCurrentChannel(tpAniSirGlobal pMac);


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



#endif //#if !defined( __LIM_SESSION_UTILS_H )


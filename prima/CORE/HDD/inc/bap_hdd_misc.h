#if !defined( BAP_HDD_MISC_H )
#define BAP_HDD_MISC_H

/**===========================================================================
  
  \file  BAP_HDD_MISC.h
  
  \brief Linux HDD Adapter Type
         Copyright 2008 (c) Qualcomm, Incorporated.
         All Rights Reserved.
         Qualcomm Confidential and Proprietary.
  
  ==========================================================================*/
  
/*--------------------------------------------------------------------------- 
  Include files
  -------------------------------------------------------------------------*/ 
  
#include <bapApi.h>
#include <vos_types.h>
/*--------------------------------------------------------------------------- 
  Function declarations and documenation
  -------------------------------------------------------------------------*/ 

/**---------------------------------------------------------------------------
  
  \brief WLANBAP_SetConfig() - To updates some configuration for BAP module in
  SME
  
  This should be called after WLANBAP_Start().
  
  \param  - NA
  
  \return -
      The result code associated with performing the operation  

    VOS_STATUS_E_FAILURE:  failed to set the config in SME BAP 
    VOS_STATUS_SUCCESS:  Success

              
  --------------------------------------------------------------------------*/
VOS_STATUS WLANBAP_SetConfig
(
    WLANBAP_ConfigType *pConfig
);


#endif    // end #if !defined( BAP_HDD_MISC_H )

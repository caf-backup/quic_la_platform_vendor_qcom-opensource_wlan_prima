#if !defined( __VOS_WM_TYPES_H )
#define __VOS_WM_TYPES_H


/**=========================================================================
  
  \file  vos_wm_types
  
  \brief virtual Operating System Servies (vOSS) types mapping on WinMobile
               
   Definitions for vOSS types on Windows Mobile...
  
   Copyright 2008 (c) Qualcomm, Incorporated.  All Rights Reserved.
   
   Qualcomm Confidential and Proprietary.
  
  ========================================================================*/

/* $Header$ */

/*--------------------------------------------------------------------------
  Include Files
  ------------------------------------------------------------------------*/
#include <ndis.h>
#include <windows.h>
#include <winbase.h>
#include <vos_types.h>
#include <vos_list.h>
#include <vos_packet.h>
#include <vos_timer.h>
#include <vos_wm_list_types.h>

/*-------------------------------------------------------------------------- 
  Preprocessor definitions and constants
  ------------------------------------------------------------------------*/



/*-------------------------------------------------------------------------- 
  Type declarations
  ------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


struct vos_wm_event_s
{
   HANDLE   evt;
   v_U32_t  cookie;
  
};

struct vos_lock_s
{
   CRITICAL_SECTION criticalsection;
   v_U32_t cookie;
   DWORD threadID;
   v_U32_t state;
};

struct vos_timer_s
{
   MMRESULT timerID;
   vos_timer_callback_t callback;
   v_PVOID_t userData;
   VOS_TIMER_TYPE type;
   DWORD threadID;
   v_U32_t cookie;
   v_U32_t state;	

};

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // #if !defined __VOSS_WM_TYPES_H

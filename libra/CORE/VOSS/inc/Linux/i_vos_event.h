#if !defined( __I_VOS_EVENT_H )
#define __I_VOS_EVENT_H

/**=========================================================================
  
  \file  i_vos_event.h
  
  \brief Linux-specific definitions for vOSS Events
  
   Copyright 2008 (c) Qualcomm Technologies, Inc.  All Rights Reserved.
   
   Qualcomm Technologies Confidential and Proprietary.
  
  ========================================================================*/

/* $Header$ */

/*--------------------------------------------------------------------------
  Include Files
  ------------------------------------------------------------------------*/
#include <vos_types.h>
#include <linux/sched.h>
#include <linux/string.h>

/*-------------------------------------------------------------------------- 
  Preprocessor definitions and constants
  ------------------------------------------------------------------------*/
#define VOS_EVENT_SET    (1<<0)
#define LINUX_EVENT_COOKIE 0x12341234

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/*-------------------------------------------------------------------------- 
  Type declarations
  ------------------------------------------------------------------------*/

typedef struct evt
{
   volatile unsigned long event_flags;
   struct task_struct * process;
   v_U32_t  cookie;
} vos_event_t;

/*------------------------------------------------------------------------- 
  Function declarations and documenation
  ------------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // __I_VOS_EVENT_H

/*
 * Copyright (c) 2012-2013 Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Confidential and Proprietary.
 */
/**=========================================================================
  
  \file  wlan_qct_pal_timer.c
  
  \brief Implementation trace/logging APIs PAL exports. wpt = (Wlan Pal Type) wpal = (Wlan PAL)
               
   Definitions for platform Windows.
  
   Copyright 2010-2011 (c) Qualcomm, Incorporated.  All Rights Reserved.
   
   Qualcomm Confidential and Proprietary.
  
  ========================================================================*/

#include "wlan_qct_pal_timer.h"
#include "wlan_qct_pal_trace.h"
#include "wlan_qct_os_status.h"
#include "vos_threads.h"

#include <linux/delay.h>

/*---------------------------------------------------------------------------
 \brief wpalTimerCback - VOS timer callback function

 \param pUserdata - A cookie to data passed back in the callback function
---------------------------------------------------------------------------*/
static void wpalTimerCback( void * userData )
{
   wpt_timer *pTimer = (wpt_timer *)userData;

   if(NULL != pTimer->callback)
   {
      pTimer->callback(pTimer->pUserData);
   }
   else
   {
      WPAL_TRACE( eWLAN_MODULE_PAL, eWLAN_PAL_TRACE_LEVEL_WARN,
                  " %s pTimer(%p) callback after deleted",
                  __func__, pTimer );
   }
}/*wpalTimerCback*/

/*---------------------------------------------------------------------------
 \brief wpalTimerInit - initialize a wpt_timer object

 \param pTimer - a pointer to caller allocated wpt_timer object
 \param callback - A callback function
 \param pUserData - A cookie to data passed back in the callback function

 \return wpt_status eWLAN_PAL_STATUS_SUCCESS - success. Fail otherwise.
---------------------------------------------------------------------------*/
wpt_status wpalTimerInit(wpt_timer * pTimer, wpal_timer_callback callback, void *pUserData)
{
   /* Sanity Checks */
   if( pTimer == NULL || callback == NULL )
   {
      WPAL_TRACE( eWLAN_MODULE_PAL, eWLAN_PAL_TRACE_LEVEL_ERROR,
                  " %s Wrong param pTimer(%p) callback(%p)",
                  __func__, pTimer, callback );
      return eWLAN_PAL_STATUS_E_INVAL;
   }

   if ( vos_timer_init( &pTimer->timer.timerObj, VOS_TIMER_TYPE_SW, 
                        wpalTimerCback, (void*)pTimer ) == VOS_STATUS_SUCCESS )
   {
      pTimer->callback = callback;
      pTimer->pUserData = pUserData;
      return eWLAN_PAL_STATUS_SUCCESS;
   }

   return eWLAN_PAL_STATUS_E_FAILURE;
}/*wpalTimerInit*/


/*---------------------------------------------------------------------------
    \brief wpalTimerDelete - invalidate a wpt_timer object

    \param pTimer a pointer to caller allocated wpt_timer object

    \return eWLAN_PAL_STATUS_SUCCESS ?? success. Fail otherwise.
---------------------------------------------------------------------------*/
wpt_status wpalTimerDelete(wpt_timer *pTimer)
{
   wpt_status status;

   /* Sanity Checks */
   if( pTimer == NULL )
   {
      WPAL_TRACE( eWLAN_MODULE_PAL, eWLAN_PAL_TRACE_LEVEL_ERROR,
                  " %s Wrong param pTimer(%p)",
                  __func__, pTimer );
      return eWLAN_PAL_STATUS_E_INVAL;
   }

   status = WPAL_VOS_TO_WPAL_STATUS( vos_timer_destroy(&pTimer->timer.timerObj));

   if( status == eWLAN_PAL_STATUS_SUCCESS )
   {
      pTimer->callback = NULL;
      pTimer->pUserData = NULL;
   }

   return status;
}/*wpalTimerDelete*/


/*---------------------------------------------------------------------------
    wpalTimerStart - start a wpt_timer object with a timeout value

    \param pTimer - a pointer to caller allocated wpt_timer object
    \param timeout - timeout value of the timer. In unit of milli-seconds

    \return
        eWLAN_PAL_STATUS_SUCCESS - success. Fail otherwise.
---------------------------------------------------------------------------*/
wpt_status wpalTimerStart(wpt_timer * pTimer, wpt_uint32 timeout)
{
   /* Sanity Checks */
   if( pTimer == NULL )
   {
      WPAL_TRACE( eWLAN_MODULE_PAL, eWLAN_PAL_TRACE_LEVEL_ERROR,
                  " %s Wrong param pTimer(%p)",
                  __func__, pTimer );
      return eWLAN_PAL_STATUS_E_INVAL;
   }
   return ( WPAL_VOS_TO_WPAL_STATUS( vos_timer_start( &pTimer->timer.timerObj,
                                                     timeout ) ) );
}/*wpalTimerStart*/


/*---------------------------------------------------------------------------
    \brief wpalTimerStop - stop a wpt_timer object. Stop doesn't guarantee the
            timer handler is not called if it is already timeout.

    \param pTimer - a pointer to caller allocated wpt_timer object

    \return
        eWLAN_PAL_STATUS_SUCCESS - success. Fail otherwise.
---------------------------------------------------------------------------*/
wpt_status wpalTimerStop(wpt_timer * pTimer)
{
   /* Sanity Checks */
   if( pTimer == NULL )
   {
      WPAL_TRACE( eWLAN_MODULE_PAL, eWLAN_PAL_TRACE_LEVEL_ERROR,
                  " %s Wrong param pTimer(%p)",
                  __func__, pTimer );
      return eWLAN_PAL_STATUS_E_INVAL;
   }
   return (WPAL_VOS_TO_WPAL_STATUS( vos_timer_stop( &pTimer->timer.timerObj )));
}/*wpalTimerStop*/

/*---------------------------------------------------------------------------
    \brief wpalTimerGetCurStatus - Get the current status of timer

    \param pTimer - a pointer to caller allocated wpt_timer object

    \return
        VOS_TIMER_STATE
---------------------------------------------------------------------------*/
WPAL_TIMER_STATE wpalTimerGetCurStatus(wpt_timer * pTimer)
{
   /* Sanity Checks */
   if( pTimer == NULL )
   {
      WPAL_TRACE( eWLAN_MODULE_PAL, eWLAN_PAL_TRACE_LEVEL_ERROR,
                  " %s Wrong param pTimer(%p)",
                  __func__, pTimer );
      return eWLAN_PAL_STATUS_E_INVAL;
   }
   return vos_timer_getCurrentState( &pTimer->timer.timerObj );
}/*wpalTimerGetCurStatus*/

/*---------------------------------------------------------------------------
    \brief wpalGetSystemTime - Get the system time in milliseconds

    \return
        current time in milliseconds
---------------------------------------------------------------------------*/
wpt_uint32 wpalGetSystemTime(void)
{
   return vos_timer_get_system_time();
}/*wpalGetSystemTime*/

/*---------------------------------------------------------------------------
    wpalSleep - sleep for a specified interval
    Param:
        timeout - amount of time to sleep. In unit of milli-seconds.
    Return:
        eWLAN_PAL_STATUS_SUCCESS - success. Fail otherwise.
---------------------------------------------------------------------------*/
wpt_status wpalSleep(wpt_uint32 timeout)
{
   vos_sleep( timeout );
   return eWLAN_PAL_STATUS_SUCCESS;
}

/*---------------------------------------------------------------------------
    wpalBusyWait - Thread busy wait with specified usec
    Param:
        usecDelay - amount of time to wait. In unit of micro-seconds.
    Return:
        NONE
---------------------------------------------------------------------------*/
void wpalBusyWait(wpt_uint32 usecDelay)
{
   vos_busy_wait(usecDelay);
   return;
}

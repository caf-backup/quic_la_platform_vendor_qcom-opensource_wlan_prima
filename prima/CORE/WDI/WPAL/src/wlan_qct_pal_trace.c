

/**=========================================================================
  
  \file  wlan_qct_pal_trace.c
  
  \brief Implementation trace/logging APIs PAL exports. wpt = (Wlan Pal Type) wpal = (Wlan PAL)
               
   Definitions for platform Windows.
  
   Copyright 2010-2011 (c) Qualcomm, Incorporated.  All Rights Reserved.
   
   Qualcomm Confidential and Proprietary.
  
  ========================================================================*/

#include "wlan_qct_pal_trace.h"
#include "i_vos_types.h"

#ifdef WLAN_DEBUG


/*--------------------------------------------------------------------------
  Preprocessor definitions and constants
  ------------------------------------------------------------------------*/

#define WPAL_TRACE_BUFFER_SIZE ( 512 )

// macro to map vos trace levels into the bitmask
#define WPAL_TRACE_LEVEL_TO_MODULE_BITMASK( _level ) ( ( 1 << (_level) ) )

typedef struct
{
   // Trace level for a module, as a bitmask.  The bits in this mask
   // are ordered by wpt_tracelevel.  For example, each bit represents
   // one of the bits in wpt_tracelevel that may be turned on to have
   // traces at that level logged, i.e. if eWLAN_PAL_TRACE_LEVEL_ERROR is
   // == 2, then if bit 2 (low order) is turned ON, then ERROR traces
   // will be printed to the trace log.
   //
   // Note that all bits turned OFF means no traces.
   wpt_uint16 moduleTraceLevel;

   // 3 character string name for the module
   wpt_uint8 moduleNameStr[ 4 ];   // 3 chars plus the NULL

} moduleTraceInfo;


// Array of static data that contains all of the per module trace
// information.  This includes the trace level for the module and
// the 3 character 'name' of the module for marking the trace logs.
moduleTraceInfo gTraceInfo[ eWLAN_MODULE_COUNT ] =
{
   { (1<<eWLAN_PAL_TRACE_LEVEL_FATAL)|(1<<eWLAN_PAL_TRACE_LEVEL_ERROR), "DAL" }, 
   { (1<<eWLAN_PAL_TRACE_LEVEL_FATAL)|(1<<eWLAN_PAL_TRACE_LEVEL_ERROR), "CTL" },
   { (1<<eWLAN_PAL_TRACE_LEVEL_FATAL)|(1<<eWLAN_PAL_TRACE_LEVEL_ERROR), "DAT" }, 
   { (1<<eWLAN_PAL_TRACE_LEVEL_FATAL)|(1<<eWLAN_PAL_TRACE_LEVEL_ERROR), "PAL" }, 
};


/*-------------------------------------------------------------------------
  Functions
  ------------------------------------------------------------------------*/
void wpalTraceSetLevel( wpt_moduleid module, wpt_tracelevel level )
{
   // Make sure the caller is passing in a valid LEVEL and MODULE.
   if( (eWLAN_PAL_TRACE_LEVEL_COUNT >= level) && (eWLAN_MODULE_COUNT >= module) )
   {

      // Treat 'none' differently.  NONE means we have to run off all
      // the bits in the bit mask so none of the traces appear.  Anything other
      // than 'none' means we need to turn ON a bit in the bitmask.
      if ( eWLAN_PAL_TRACE_LEVEL_NONE == level )
      {
	      gTraceInfo[ module ].moduleTraceLevel = eWLAN_PAL_TRACE_LEVEL_NONE;
      }
      else
      {
         // Set the desired bit in the bit mask for the module trace level.
	      gTraceInfo[ module ].moduleTraceLevel |= WPAL_TRACE_LEVEL_TO_MODULE_BITMASK( level );
      }
   }
}

wpt_boolean wpalTraceCheckLevel( wpt_moduleid module, wpt_tracelevel level )
{
   wpt_boolean traceOn = eWLAN_PAL_FALSE;

   if ( ( eWLAN_PAL_TRACE_LEVEL_NONE == level ) ||
        ( level >= eWLAN_PAL_TRACE_LEVEL_COUNT ))
   {
      traceOn = eWLAN_PAL_FALSE;
   }
   else if( eWLAN_PAL_TRACE_LEVEL_ALL == level )
   {
      traceOn = eWLAN_PAL_TRUE;
   }
   else
   {
      traceOn = ( level & gTraceInfo[ module ].moduleTraceLevel ) ? eWLAN_PAL_TRUE : eWLAN_PAL_FALSE;
   }

   return( traceOn );
}



/*----------------------------------------------------------------------------

  \brief wpalTrace() - Externally called trace function

  Checks the level of severity and accordingly prints the trace messages

  \param module - module identifier.   A member of the wpt_moduleid
         enumeration that identifies the module issuing the trace message.

  \param level - trace level.   A member of the wpt_tracelevel
         enumeration indicating the severity of the condition causing the
         trace message to be issued.   More severe conditions are more
         likely to be logged.

  \param strFormat - format string.  The message to be logged.  This format
         string contains printf-like replacement parameters, which follow
         this parameter in the variable argument list.

  \return  nothing

  \sa

  --------------------------------------------------------------------------*/
void wpalTrace( wpt_moduleid module, wpt_tracelevel level, char *strFormat, ... )
{
   wpt_uint8 strBuffer[ WPAL_TRACE_BUFFER_SIZE ];
   int n;

   // Print the trace message when the desired level bit is set in the module
   // tracel level mask.
   if ( gTraceInfo[ module ].moduleTraceLevel & WPAL_TRACE_LEVEL_TO_MODULE_BITMASK( level ) )
   {
   // the trace level strings in an array.  these are ordered in the same order
      // as the trace levels are defined in the enum (see VOS_TRACE_LEVEL) so we
      // can index into this array with the level and get the right string.  The
      // vos trace levels are...
      // none, Fata, Error, Warning, Info, InfoHigh, InfoMed, InfoLow
      static const char * TRACE_LEVEL_STR[] = { "   ", "F ", "E ", "W ", "I ", "IH", "IM", "IL" };
      va_list val;
      va_start(val, strFormat);

      // print the prefix string into the string buffer...
      n = snprintf(strBuffer, WPAL_TRACE_BUFFER_SIZE, "[%d:%d:%2s:%3s] ",
                   smp_processor_id(),
                   in_interrupt() ? 0 : current->pid,
                   (char *) TRACE_LEVEL_STR[ level ],
                   (char *) gTraceInfo[ module ].moduleNameStr);


      // print the formatted log message after the prefix string.
      // note we reserve space for the terminating newline & NUL
      vsnprintf(strBuffer + n, WPAL_TRACE_BUFFER_SIZE - n - 2, strFormat, val);
      strcat(strBuffer, "\n");

      switch(level)
      {
         default:
            printk(KERN_CRIT "%s: Unknown trace level passed in!\n", __FUNCTION__); 
            // fall thru and use FATAL

         case eWLAN_PAL_TRACE_LEVEL_FATAL:
            printk(KERN_CRIT "%s", strBuffer);
            break;

         case eWLAN_PAL_TRACE_LEVEL_ERROR:
            printk(KERN_ERR "%s", strBuffer);
            break;

         case eWLAN_PAL_TRACE_LEVEL_WARN:
            printk(KERN_WARNING "%s", strBuffer);
            break;

         case eWLAN_PAL_TRACE_LEVEL_INFO:
            printk(KERN_INFO "%s", strBuffer);
            break;

         case eWLAN_PAL_TRACE_LEVEL_INFO_HIGH:
            printk(KERN_NOTICE "%s", strBuffer);
            break;

         case eWLAN_PAL_TRACE_LEVEL_INFO_MED:
            printk(KERN_NOTICE "%s", strBuffer);
            break;

         case eWLAN_PAL_TRACE_LEVEL_INFO_LOW:
            printk(KERN_INFO "%s", strBuffer);
            break;
      }
      va_end( val);
   }
}


#endif //WLAN_DEBUG








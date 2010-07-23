/**=========================================================================

  \file  vos_trace.c

  \brief virtual Operating System Servies (vOS)

   Trace, logging, and debugging definitions and APIs

   Copyright 2008 (c) Qualcomm, Incorporated.  All Rights Reserved.

   Qualcomm Confidential and Proprietary.

  ========================================================================*/

/*===========================================================================

                       EDIT HISTORY FOR FILE


  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.


  $Header:$ $DateTime: $ $Author: $


  when        who    what, where, why
  --------    ---    --------------------------------------------------------
  09/16/08    hvm    Adding ability to set multiple trace levels per component
  09/11/08    lac    Added trace levels per component.  Cleanup from review.
  08/14/08    vpai   Particular modules and desired level can be selected
  06/20/08    vpai   Created Module
===========================================================================*/

/*--------------------------------------------------------------------------
  Include Files
  ------------------------------------------------------------------------*/
#include <vos_trace.h>

/*--------------------------------------------------------------------------
  Preprocessor definitions and constants
  ------------------------------------------------------------------------*/

#define VOS_TRACE_BUFFER_SIZE ( 1024 )

// macro to map vos trace levels into the bitmask
#define VOS_TRACE_LEVEL_TO_MODULE_BITMASK( _level ) ( ( 1 << (_level) ) )

typedef struct
{
   // Trace level for a module, as a bitmask.  The bits in this mask
   // are ordered by VOS_TRACE_LEVEL.  For example, each bit represents
   // one of the bits in VOS_TRACE_LEVEL that may be turned on to have
   // traces at that level logged, i.e. if VOS_TRACE_LEVEL_ERROR is
   // == 2, then if bit 2 (low order) is turned ON, then ERROR traces
   // will be printed to the trace log.
   //
   // Note that all bits turned OFF means no traces.
   v_U16_t moduleTraceLevel;

   // 3 character string name for the module
   unsigned char moduleNameStr[ 4 ];   // 3 chars plus the NULL

} moduleTraceInfo;


// Array of static data that contains all of the per module trace
// information.  This includes the trace level for the module and
// the 3 character 'name' of the module for marking the trace logs.
moduleTraceInfo gVosTraceInfo[ VOS_MODULE_ID_MAX ] =
{
   { (1<<VOS_TRACE_LEVEL_FATAL), "TL " }, 
   { (1<<VOS_TRACE_LEVEL_FATAL), "BAL" }, 
   { (1<<VOS_TRACE_LEVEL_FATAL), "SAL" }, 
   { (1<<VOS_TRACE_LEVEL_FATAL), "SSC" },
   // Made HDD with info high as we need the assoc completion events
   // to determine if we are associated, disassociated etc.
   // important control path events.
   { (1<<VOS_TRACE_LEVEL_FATAL) | (1<<VOS_TRACE_LEVEL_INFO_HIGH), "HDD" }, 
   { (1<<VOS_TRACE_LEVEL_FATAL), "SME" }, 
   { (1<<VOS_TRACE_LEVEL_FATAL), "PE " }, 
   { (1<<VOS_TRACE_LEVEL_FATAL), "HAL" }, 
   { (1<<VOS_TRACE_LEVEL_FATAL), "SYS" }, 
   { (1<<VOS_TRACE_LEVEL_FATAL), "VOS" }
};


/*-------------------------------------------------------------------------
  Functions
  ------------------------------------------------------------------------*/
void vos_trace_setLevel( VOS_MODULE_ID module, VOS_TRACE_LEVEL level )
{
   // Make sure the caller is passing in a valid LEVEL.
   if ( level >= VOS_TRACE_LEVEL_MAX )
   {
      printk(KERN_CRIT "%s: Invalid trace level %d passed in!\n",__FUNCTION__, level);
      return;
   }

   // Treat 'none' differently.  NONE means we have to run off all
   // the bits in the bit mask so none of the traces appear.  Anything other
   // than 'none' means we need to turn ON a bit in the bitmask.
   if ( VOS_TRACE_LEVEL_NONE == level )
   {
	   gVosTraceInfo[ module ].moduleTraceLevel = VOS_TRACE_LEVEL_NONE;
   }
   else
   {
      // Set the desired bit in the bit mask for the module trace level.
	   gVosTraceInfo[ module ].moduleTraceLevel |= VOS_TRACE_LEVEL_TO_MODULE_BITMASK( level );
   }
}

void vos_trace_setValue( VOS_MODULE_ID module, VOS_TRACE_LEVEL level, v_U8_t on)
{
   // Make sure the caller is passing in a valid LEVEL.
   if ( level < 0  || level >= VOS_TRACE_LEVEL_MAX )
   {
      printk(KERN_CRIT "%s: Invalid trace level %d passed in!\n",__FUNCTION__, level);
      return;
   }
	
   // Make sure the caller is passing in a valid module.
   if ( module < 0 || module >= VOS_MODULE_ID_MAX )
   {
      printk(KERN_CRIT "%s: Invalid module id %d passed in!\n",__FUNCTION__, module);
      return;
   }

   // Treat 'none' differently.  NONE means we have to turn off all
   // the bits in the bit mask so none of the traces appear.
   if ( VOS_TRACE_LEVEL_NONE == level )
   {
	   gVosTraceInfo[ module ].moduleTraceLevel = VOS_TRACE_LEVEL_NONE;
   }
   // Treat 'All' differently.  All means we have to turn on all
   // the bits in the bit mask so all of the traces appear.
   else if ( VOS_TRACE_LEVEL_ALL == level )
   {
	   gVosTraceInfo[ module ].moduleTraceLevel = 0xFFFF;
   }
	
   else
   {
      if(on)
          // Set the desired bit in the bit mask for the module trace level.
	       gVosTraceInfo[ module ].moduleTraceLevel |= VOS_TRACE_LEVEL_TO_MODULE_BITMASK( level );
		else
			// Clear the desired bit in the bit mask for the module trace level.
			gVosTraceInfo[ module ].moduleTraceLevel &= ~(VOS_TRACE_LEVEL_TO_MODULE_BITMASK( level ));
   }
}


v_BOOL_t vos_trace_getLevel( VOS_MODULE_ID module, VOS_TRACE_LEVEL level )
{
   v_BOOL_t traceOn = VOS_FALSE;

   if ( ( VOS_TRACE_LEVEL_NONE == level ) ||
        ( VOS_TRACE_LEVEL_ALL  == level ) ||
        ( level >= VOS_TRACE_LEVEL_MAX  )    )
   {
      traceOn = VOS_FALSE;
   }
   else
   {
      traceOn = ( level & gVosTraceInfo[ module ].moduleTraceLevel ) ? VOS_TRUE : VOS_FALSE;
   }

   return( traceOn );
}


#ifdef VOS_ENABLE_TRACING

/*----------------------------------------------------------------------------

  \brief vos_trace_msg() - Externally called trace function

  Checks the level of severity and accordingly prints the trace messages

  \param module - module identifier.   A member of the VOS_MODULE_ID
         enumeration that identifies the module issuing the trace message.

  \param level - trace level.   A member of the VOS_TRACE_LEVEL
         enumeration indicating the severity of the condition causing the
         trace message to be issued.   More severe conditions are more
         likely to be logged.

  \param strFormat - format string.  The message to be logged.  This format
         string contains printf-like replacement parameters, which follow
         this parameter in the variable argument list.

  \return  nothing

  \sa

  --------------------------------------------------------------------------*/
void vos_trace_msg( VOS_MODULE_ID module, VOS_TRACE_LEVEL level, char *strFormat, ... )
{
   char strBuffer[VOS_TRACE_BUFFER_SIZE];

   // Print the trace message when the desired level bit is set in the module
   // tracel level mask.
   if ( gVosTraceInfo[ module ].moduleTraceLevel & VOS_TRACE_LEVEL_TO_MODULE_BITMASK( level ) )
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
      snprintf( strBuffer, VOS_TRACE_BUFFER_SIZE, "[%2s:%3s] ",
                 (char *) TRACE_LEVEL_STR[ level ],
                 (char *) gVosTraceInfo[ module ].moduleNameStr );

      // then move the format string with var replacements into the string buffer
      // AFTER the prefix string.  Note the prefix string is 9 characters long and
      // looks something like this.... [I :HDD] with a space after the last bracket.
      // Hence the hardcoded 9 below where we put the formaatted string following
      // the prefix. We also need to reserve additional 2 spaces for the terminating
      // new line and null character
      vsnprintf(strBuffer + 9,VOS_TRACE_BUFFER_SIZE - 11, strFormat, val );
      strcat(strBuffer, "\n");

      switch(level)
      {
         default:
            printk(KERN_CRIT "%s: Unknown trace level passed in!\n", __FUNCTION__); 
            break;

         case VOS_TRACE_LEVEL_FATAL:
            printk(KERN_CRIT "%s", strBuffer);
            break;

         case VOS_TRACE_LEVEL_ERROR:
            printk(KERN_CRIT "%s", strBuffer);
            break;

         case VOS_TRACE_LEVEL_WARN:
            printk(KERN_CRIT "%s", strBuffer);
            break;

         case VOS_TRACE_LEVEL_INFO:
            printk(KERN_CRIT "%s", strBuffer);
            break;

         case VOS_TRACE_LEVEL_INFO_HIGH:
            printk(KERN_CRIT "%s", strBuffer);
            break;

         case VOS_TRACE_LEVEL_INFO_MED:
            printk(KERN_CRIT "%s", strBuffer);
            break;

         case VOS_TRACE_LEVEL_INFO_LOW:
            printk(KERN_CRIT "%s", strBuffer);
            break;
      }
      va_end( val);
   }
}

void vos_trace_display(void)
{
     printk(KERN_CRIT "     1)FATAL  2)ERROR  3)WARN  4)INFO  5)INFO_H  6)INFO_M  7)INFO_L\n"); 
     printk(KERN_CRIT "0)TL     %s        %s        %s      %s       %s         %s         %s\n",
         (gVosTraceInfo[0].moduleTraceLevel & (1 << VOS_TRACE_LEVEL_FATAL)) ? "X":"",
         (gVosTraceInfo[0].moduleTraceLevel & (1 << VOS_TRACE_LEVEL_ERROR)) ? "X":"",
         (gVosTraceInfo[0].moduleTraceLevel & (1 << VOS_TRACE_LEVEL_WARN)) ? "X":"",
         (gVosTraceInfo[0].moduleTraceLevel & (1 << VOS_TRACE_LEVEL_INFO)) ? "X":"",
         (gVosTraceInfo[0].moduleTraceLevel & (1 << VOS_TRACE_LEVEL_INFO_HIGH)) ? "X":"",
         (gVosTraceInfo[0].moduleTraceLevel & (1 << VOS_TRACE_LEVEL_INFO_MED)) ? "X":"",
         (gVosTraceInfo[0].moduleTraceLevel & (1 << VOS_TRACE_LEVEL_INFO_LOW)) ? "X":""
         );
     printk(KERN_CRIT "1)BAL    %s        %s       %s       %s        %s         %s         %s\n",
         (gVosTraceInfo[1].moduleTraceLevel & (1 << VOS_TRACE_LEVEL_FATAL)) ? "X":"",
         (gVosTraceInfo[1].moduleTraceLevel & (1 << VOS_TRACE_LEVEL_ERROR)) ? "X":"",
         (gVosTraceInfo[1].moduleTraceLevel & (1 << VOS_TRACE_LEVEL_WARN)) ? "X":"",
         (gVosTraceInfo[1].moduleTraceLevel & (1 << VOS_TRACE_LEVEL_INFO)) ? "X":"",
         (gVosTraceInfo[1].moduleTraceLevel & (1 << VOS_TRACE_LEVEL_INFO_HIGH)) ? "X":"",
         (gVosTraceInfo[1].moduleTraceLevel & (1 << VOS_TRACE_LEVEL_INFO_MED)) ? "X":"",
         (gVosTraceInfo[1].moduleTraceLevel & (1 << VOS_TRACE_LEVEL_INFO_LOW)) ? "X":""
         );
     printk(KERN_CRIT "2)SAL    %s        %s       %s       %s        %s         %s         %s\n",
         (gVosTraceInfo[2].moduleTraceLevel & (1 << VOS_TRACE_LEVEL_FATAL)) ? "X":"",
         (gVosTraceInfo[2].moduleTraceLevel & (1 << VOS_TRACE_LEVEL_ERROR)) ? "X":"",
         (gVosTraceInfo[2].moduleTraceLevel & (1 << VOS_TRACE_LEVEL_WARN)) ? "X":"",
         (gVosTraceInfo[2].moduleTraceLevel & (1 << VOS_TRACE_LEVEL_INFO)) ? "X":"",
         (gVosTraceInfo[2].moduleTraceLevel & (1 << VOS_TRACE_LEVEL_INFO_HIGH)) ? "X":"",
         (gVosTraceInfo[2].moduleTraceLevel & (1 << VOS_TRACE_LEVEL_INFO_MED)) ? "X":"",
         (gVosTraceInfo[2].moduleTraceLevel & (1 << VOS_TRACE_LEVEL_INFO_LOW)) ? "X":""
         );
     printk(KERN_CRIT "3)SSC    %s        %s       %s       %s        %s         %s         %s\n",
         (gVosTraceInfo[3].moduleTraceLevel & (1 << VOS_TRACE_LEVEL_FATAL)) ? "X":"",
         (gVosTraceInfo[3].moduleTraceLevel & (1 << VOS_TRACE_LEVEL_ERROR)) ? "X":"",
         (gVosTraceInfo[3].moduleTraceLevel & (1 << VOS_TRACE_LEVEL_WARN)) ? "X":"",
         (gVosTraceInfo[3].moduleTraceLevel & (1 << VOS_TRACE_LEVEL_INFO)) ? "X":"",
         (gVosTraceInfo[3].moduleTraceLevel & (1 << VOS_TRACE_LEVEL_INFO_HIGH)) ? "X":"",
         (gVosTraceInfo[3].moduleTraceLevel & (1 << VOS_TRACE_LEVEL_INFO_MED)) ? "X":"",
         (gVosTraceInfo[3].moduleTraceLevel & (1 << VOS_TRACE_LEVEL_INFO_LOW)) ? "X":""
         );
     printk(KERN_CRIT "4)HDD    %s        %s       %s       %s        %s         %s         %s\n",
         (gVosTraceInfo[4].moduleTraceLevel & (1 << VOS_TRACE_LEVEL_FATAL)) ? "X":"",
         (gVosTraceInfo[4].moduleTraceLevel & (1 << VOS_TRACE_LEVEL_ERROR)) ? "X":"",
         (gVosTraceInfo[4].moduleTraceLevel & (1 << VOS_TRACE_LEVEL_WARN)) ? "X":"",
         (gVosTraceInfo[4].moduleTraceLevel & (1 << VOS_TRACE_LEVEL_INFO)) ? "X":"",
         (gVosTraceInfo[4].moduleTraceLevel & (1 << VOS_TRACE_LEVEL_INFO_HIGH)) ? "X":"",
         (gVosTraceInfo[4].moduleTraceLevel & (1 << VOS_TRACE_LEVEL_INFO_MED)) ? "X":"",
         (gVosTraceInfo[4].moduleTraceLevel & (1 << VOS_TRACE_LEVEL_INFO_LOW)) ? "X":""
         );
     printk(KERN_CRIT "5)SME    %s        %s       %s       %s        %s         %s         %s\n",
         (gVosTraceInfo[5].moduleTraceLevel & (1 << VOS_TRACE_LEVEL_FATAL)) ? "X":"",
         (gVosTraceInfo[5].moduleTraceLevel & (1 << VOS_TRACE_LEVEL_ERROR)) ? "X":"",
         (gVosTraceInfo[5].moduleTraceLevel & (1 << VOS_TRACE_LEVEL_WARN)) ? "X":"",
         (gVosTraceInfo[5].moduleTraceLevel & (1 << VOS_TRACE_LEVEL_INFO)) ? "X":"",
         (gVosTraceInfo[5].moduleTraceLevel & (1 << VOS_TRACE_LEVEL_INFO_HIGH)) ? "X":"",
         (gVosTraceInfo[5].moduleTraceLevel & (1 << VOS_TRACE_LEVEL_INFO_MED)) ? "X":"",
         (gVosTraceInfo[5].moduleTraceLevel & (1 << VOS_TRACE_LEVEL_INFO_LOW)) ? "X":""
         );
     printk(KERN_CRIT "6)PE     %s        %s       %s       %s        %s         %s         %s\n",
         (gVosTraceInfo[6].moduleTraceLevel & (1 << VOS_TRACE_LEVEL_FATAL)) ? "X":"",
         (gVosTraceInfo[6].moduleTraceLevel & (1 << VOS_TRACE_LEVEL_ERROR)) ? "X":"",
         (gVosTraceInfo[6].moduleTraceLevel & (1 << VOS_TRACE_LEVEL_WARN)) ? "X":"",
         (gVosTraceInfo[6].moduleTraceLevel & (1 << VOS_TRACE_LEVEL_INFO)) ? "X":"",
         (gVosTraceInfo[6].moduleTraceLevel & (1 << VOS_TRACE_LEVEL_INFO_HIGH)) ? "X":"",
         (gVosTraceInfo[6].moduleTraceLevel & (1 << VOS_TRACE_LEVEL_INFO_MED)) ? "X":"",
         (gVosTraceInfo[6].moduleTraceLevel & (1 << VOS_TRACE_LEVEL_INFO_LOW)) ? "X":""
         );
     printk(KERN_CRIT "7)HAL    %s        %s       %s       %s        %s         %s         %s\n",
         (gVosTraceInfo[7].moduleTraceLevel & (1 << VOS_TRACE_LEVEL_FATAL)) ? "X":"",
         (gVosTraceInfo[7].moduleTraceLevel & (1 << VOS_TRACE_LEVEL_ERROR)) ? "X":"",
         (gVosTraceInfo[7].moduleTraceLevel & (1 << VOS_TRACE_LEVEL_WARN)) ? "X":"",
         (gVosTraceInfo[7].moduleTraceLevel & (1 << VOS_TRACE_LEVEL_INFO)) ? "X":"",
         (gVosTraceInfo[7].moduleTraceLevel & (1 << VOS_TRACE_LEVEL_INFO_HIGH)) ? "X":"",
         (gVosTraceInfo[7].moduleTraceLevel & (1 << VOS_TRACE_LEVEL_INFO_MED)) ? "X":"",
         (gVosTraceInfo[7].moduleTraceLevel & (1 << VOS_TRACE_LEVEL_INFO_LOW)) ? "X":""
         );
     printk(KERN_CRIT "8)SYS    %s        %s       %s       %s        %s         %s         %s\n",
         (gVosTraceInfo[8].moduleTraceLevel & (1 << VOS_TRACE_LEVEL_FATAL)) ? "X":"",
         (gVosTraceInfo[8].moduleTraceLevel & (1 << VOS_TRACE_LEVEL_ERROR)) ? "X":"",
         (gVosTraceInfo[8].moduleTraceLevel & (1 << VOS_TRACE_LEVEL_WARN)) ? "X":"",
         (gVosTraceInfo[8].moduleTraceLevel & (1 << VOS_TRACE_LEVEL_INFO)) ? "X":"",
         (gVosTraceInfo[8].moduleTraceLevel & (1 << VOS_TRACE_LEVEL_INFO_HIGH)) ? "X":"",
         (gVosTraceInfo[8].moduleTraceLevel & (1 << VOS_TRACE_LEVEL_INFO_MED)) ? "X":"",
         (gVosTraceInfo[8].moduleTraceLevel & (1 << VOS_TRACE_LEVEL_INFO_LOW)) ? "X":""
         );
     printk(KERN_CRIT "9)VOS    %s        %s       %s       %s        %s         %s         %s\n",
         (gVosTraceInfo[9].moduleTraceLevel & (1 << VOS_TRACE_LEVEL_FATAL)) ? "X":"",
         (gVosTraceInfo[9].moduleTraceLevel & (1 << VOS_TRACE_LEVEL_ERROR)) ? "X":"",
         (gVosTraceInfo[9].moduleTraceLevel & (1 << VOS_TRACE_LEVEL_WARN)) ? "X":"",
         (gVosTraceInfo[9].moduleTraceLevel & (1 << VOS_TRACE_LEVEL_INFO)) ? "X":"",
         (gVosTraceInfo[9].moduleTraceLevel & (1 << VOS_TRACE_LEVEL_INFO_HIGH)) ? "X":"",
         (gVosTraceInfo[9].moduleTraceLevel & (1 << VOS_TRACE_LEVEL_INFO_MED)) ? "X":"",
         (gVosTraceInfo[9].moduleTraceLevel & (1 << VOS_TRACE_LEVEL_INFO_LOW)) ? "X":""
         );
}


#endif

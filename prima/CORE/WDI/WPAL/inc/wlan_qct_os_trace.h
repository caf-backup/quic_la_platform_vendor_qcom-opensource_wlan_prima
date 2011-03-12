#if !defined( __WLAN_QCT_OS_TRACE_H )
#define __WLAN_QCT_OS_TRACE_H


#ifdef WLAN_DEBUG

/**----------------------------------------------------------------------------
  
 \brief WPAL_TRACE() / wpalTrace() - Trace / logging API
   
 Users wishing to add tracing information to their code should use 
 WPAL_TRACE.  WPAL_TRACE() will compile into a call to wpalTrace() when
 tracing is enabled.
  
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
    
  --------------------------------------------------------------------------*/
void wpalTrace( wpt_moduleid module, wpt_tracelevel level, char *strFormat, ... );

#define WPAL_TRACE wpalTrace

#define WPAL_ASSERT( _condition )                          \
if ( ! ( _condition ) )                                   \
{                                                         \
   printk(KERN_CRIT "VOS ASSERT in %s Line %d\n", __FUNCTION__, __LINE__); \
   WARN_ON(1); \
}

#else //WLAN_DEBUG

#define WPAL_TRACE
#define WPAL_ASSERT

#endif //WLAN_DEBUG

#endif // __WLAN_QCT_OS_TRACE_H

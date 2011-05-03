/*
 * Airgo Networks, Inc proprietary. All rights reserved.
 * logApi.cc - Handles log messages for all the modules.
 * Author:        Kevin Nguyen    
 * Date:          02/27/02
 * History:-
 * 02/11/02       Created.
 * 03/12/02       Rearrange logDebug parameter list and add more params.
 * --------------------------------------------------------------------
 * 
 */
#define WNI_PRINT_DEBUG

#include <sirCommon.h>
#include <sirDebug.h>
#include <utilsApi.h>
#if defined(FEATURE_WLAN_NON_INTEGRATED_SOC)
#include <halCommonApi.h>
#endif
#include <wlan_qct_wda.h>

#include <stdarg.h>
#include "sirWrapper.h"
#include "utilsGlobal.h"
#include "macInitApi.h"
#include "palApi.h"

#include "vos_trace.h"

#ifdef ANI_OS_TYPE_ANDROID
#include <linux/kernel.h>
#endif


//This is not right here. Need to find a better place. 
//_vsnprintf is a function in Windows
//Temporary workaround.
#ifndef ANI_OS_TYPE_WINDOWS
#ifndef _vsnprintf
#define _vsnprintf vsnprintf
#endif
#endif // not Windows

#define dbgTraceInfo(_Mask, _InParams)                 \
  {                                                      \
    DbgPrint _InParams ;                                 \
  }

#define utilLogLogDebugMessage(HddAdapter, _LogBuffer)   \
  {                                                      \
    VOS_TRACE(VOS_MODULE_ID_SYS, VOS_TRACE_LEVEL_INFO,   \
              _LogBuffer);                               \
  }
  

// ---------------------------------------------------------------------
/**
 * logInit() 
 *
 * FUNCTION:
 * This function is called to prepare the logging utility.
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 * None.
 *
 * NOTE:
 *
 * @param tpAniSirGlobal Sirius software parameter strucutre pointer
 * @return None
 */
tSirRetStatus 
logInit(tpAniSirGlobal pMac)
{
    tANI_U32    i;

    if (palAllocateMemory(pMac->hHdd, ((void **)&pMac->gLogBuffer), MAX_LOG_SIZE) != eHAL_STATUS_SUCCESS)
        return eSIR_FAILURE;

    /* Initialize the pMac structure */
    palZeroMemory(pMac->hHdd, pMac->gLogBuffer, MAX_LOG_SIZE);
    
    // Add code to initialize debug level from CFG module
    // For now, enable all logging
    for (i = 0; i < LOG_ENTRY_NUM; i++)
    {
#ifdef SIR_DEBUG
        pMac->utils.gLogEvtLevel[i] = pMac->utils.gLogDbgLevel[i] = LOG1;
#else
#ifdef LX5280
        pMac->utils.gLogEvtLevel[i] = pMac->utils.gLogDbgLevel[i] = LOGE;
#else
        pMac->utils.gLogEvtLevel[i] = pMac->utils.gLogDbgLevel[i] = LOGW;
#endif
#endif
    }
    return eSIR_SUCCESS;

} /*** logInit() ***/

void
logDeinit(tpAniSirGlobal pMac)
{
    palFreeMemory(pMac->hHdd, pMac->gLogBuffer);
    return;
}

/**
 * logDbg() 
 *
 *FUNCTION:
 * This function is called to log a debug message.
 *
 *PARAMS:
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 * None.
 *
 *NOTE:
 *
 * @param tpAniSirGlobal Sirius software parameter strucutre pointer
 * @param ModId        8-bit modID
 * @param debugLevel   debugging level for this message
 * @param pStr         string parameter pointer
 * @return None
 */

#if defined(ANI_OS_TYPE_OSX)
#if defined ANI_FIREWIRE_LOG
#include <IOKit/firewire/FireLog.h>
#define printk          FireLog
#else
#define printk			printf
#endif
#define	tx_time_get()	(0)
#endif

void logDbg(tpAniSirGlobal pMac, tANI_U8 modId, tANI_U32 debugLevel, const char *pStr,...)
{
#ifdef WLAN_DEBUG
    if ( debugLevel > pMac->utils.gLogDbgLevel[LOG_INDEX_FOR_MODULE( modId )] )
        return;
    else
    {
        va_list marker;

        va_start( marker, pStr );     /* Initialize variable arguments. */

        logDebug(pMac, modId, debugLevel, pStr, marker);
        
        va_end( marker );              /* Reset variable arguments.      */
    }      
#endif
}

#ifdef VOSS_ENABLED
static inline VOS_TRACE_LEVEL getVosDebugLevel(tANI_U32 debugLevel)
{   
    switch(debugLevel)
    {
        case LOGP:
            return VOS_TRACE_LEVEL_FATAL;
        case LOGE:
            return VOS_TRACE_LEVEL_ERROR;
        case LOGW:
            return VOS_TRACE_LEVEL_WARN;
        case LOG4:
            return VOS_TRACE_LEVEL_INFO;
        case LOG3:
            return VOS_TRACE_LEVEL_INFO_HIGH;
        case LOG2:
            return VOS_TRACE_LEVEL_INFO_MED;
        case LOG1:
            return VOS_TRACE_LEVEL_INFO_LOW;
        default:
            return VOS_TRACE_LEVEL_INFO_LOW;
	}
}

static inline VOS_MODULE_ID getVosModuleId(tANI_U8 modId)
{
    switch(modId)
    {
        case SIR_HAL_MODULE_ID:
        case SIR_PHY_MODULE_ID:
            return VOS_MODULE_ID_WDA;

        case SIR_LIM_MODULE_ID:
        case SIR_SCH_MODULE_ID:
        case SIR_PMM_MODULE_ID:
        case SIR_CFG_MODULE_ID:
        case SIR_MNT_MODULE_ID:
        case SIR_DPH_MODULE_ID:
        case SIR_DBG_MODULE_ID:
            return VOS_MODULE_ID_PE;

        case SIR_SYS_MODULE_ID:
            return VOS_MODULE_ID_SYS;
    
        case SIR_SMS_MODULE_ID:
            return VOS_MODULE_ID_SME;

        default:
            return VOS_MODULE_ID_SYS;
    }
}
#endif // VOSS_ENABLED

void logDebug(tpAniSirGlobal pMac, tANI_U8 modId, tANI_U32 debugLevel, const char *pStr, va_list marker)
{
#ifdef VOSS_ENABLED

    VOS_TRACE_LEVEL  vosDebugLevel;
    VOS_MODULE_ID    vosModuleId;

    vosDebugLevel = getVosDebugLevel(debugLevel);
    vosModuleId = getVosModuleId(modId);

#ifdef ANI_OS_TYPE_ANDROID
    vsnprintf(pMac->gLogBuffer, MAX_LOG_SIZE-1, (char *)pStr, marker);
#else

#ifdef WINDOWS_DT
    RtlStringCbVPrintfA( &pMac->gLogBuffer[ 0 ], MAX_LOG_SIZE - 1, pStr, marker );
#else
    _vsnprintf(pMac->gLogBuffer, MAX_LOG_SIZE-1, (char *)pStr, marker);
#endif

#endif
    VOS_TRACE(vosModuleId, vosDebugLevel, pMac->gLogBuffer);

    // The caller must check loglevel
    VOS_ASSERT( ( debugLevel <= pMac->utils.gLogDbgLevel[LOG_INDEX_FOR_MODULE( modId )] ) && ( LOGP != debugLevel ) );
#else
    tANI_U32          index;
#if defined(ANI_OS_TYPE_LINUX) || defined(ANI_OS_TYPE_OSX)
    int len;
#endif

    index = modId - LOG_FIRST_MODULE_ID;

#if defined(ANI_OS_TYPE_WINDOWS)
    _vsnprintf(pMac->gLogBuffer, MAX_LOG_SIZE-1, (char *)pStr, marker);
    // Log the message in the Windows Event Log, if this is a LOGE or
    // LOGP message
    if ((LOGE == debugLevel) || (LOGP == debugLevel))
        utilLogLogDebugMessage(pMac->pAdapter, pMac->gLogBuffer);
#endif

    // Check for panic level
    if (debugLevel == LOGP)
    {
#if defined(ANI_OS_TYPE_LINUX) || defined(ANI_OS_TYPE_OSX)
        vsnprintf(pMac->gLogBuffer, MAX_LOG_SIZE-1, (char *)pStr, marker);
        printk("************* MODULE 0x%x radioId 0x%x **************\n",
               modId, (int) pMac->sys.gSirRadioId);
        printk(pMac->gLogBuffer);
        printk("***************************************\n");
#else // WINDOWS
        _vsnprintf(pMac->gLogBuffer, MAX_LOG_SIZE-1, (char *)pStr, marker);
        dbgTraceInfo(DBG_MASK_LOG_MSGS, ("[%0u] ", tx_time_get()));
        dbgTraceInfo(DBG_MASK_LOG_MSGS, (pMac->gLogBuffer));
#endif

        // Action taken
        macSysResetReq(pMac, eSIR_LOGP_EXCEPTION);
        return;
    }

    // Verify against current log level
    if (debugLevel > pMac->utils.gLogDbgLevel[index])
        return;

#ifdef WNI_PRINT_DEBUG
#if defined(ANI_OS_TYPE_LINUX) || defined(ANI_OS_TYPE_OSX)
    len = vsnprintf(pMac->gLogBuffer, MAX_LOG_SIZE-1, (char *)pStr, marker);
    printk("[%d][%2x][%d][%0u] ",
           (int) pMac->sys.gSirRadioId, modId, (int) debugLevel, (int) tx_time_get());
    printk(pMac->gLogBuffer);
    if (pMac->gLogBuffer[len-1] != '\n')
		printk("\n");
		
    return;
#elif defined(ANI_OS_TYPE_WINDOWS)
    _vsnprintf(pMac->gLogBuffer, MAX_LOG_SIZE-1, (char *)pStr, marker);
    //dbgTraceInfo(DBG_MASK_LOG_MSGS, (pMac->gLogBuffer));
    vprintf(pStr, marker);
#endif // ANI_OS_TYPE
#endif // WNI_PRINT_DEBUG
#endif // VOSS_ENABLED
} /*** end logDebug() ***/

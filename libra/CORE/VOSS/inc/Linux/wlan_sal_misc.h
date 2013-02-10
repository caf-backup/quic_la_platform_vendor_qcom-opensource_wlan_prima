#ifndef WLAN_SAL_MISC_H
#define WLAN_SAL_MISC_H

/**=========================================================================
  
  @file  wlan_sal_misc.h
  
  @brief WLAN SDIO ABSTRACTION LAYER MISC MACROs and DEFINES file 
               
   This file contains the external API exposed by the wlan SDIO abstraction layer module.
   Copyright (c) 2008 Qualcomm Technologies, Inc. All Rights Reserved.
   Qualcomm Technologies Confidential and Proprietary
========================================================================*/

/*===========================================================================

                      EDIT HISTORY FOR FILE


  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.


  $Header:$ $DateTime: $ $Author: $


when           who        what, where, why
--------    ---         ----------------------------------------------------------

===========================================================================*/

/*===========================================================================

                          INCLUDE FILES FOR MODULE

===========================================================================*/

/*----------------------------------------------------------------------------
 * Include Files
 * -------------------------------------------------------------------------*/
#include <linux/mutex.h>
#include <linux/mmc/card.h>
#include <linux/mmc/host.h>
#include <linux/mmc/sdio_func.h>
#include <libra_sdioif.h>

#ifdef __cplusplus
extern "C" {
#endif

/*----------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * -------------------------------------------------------------------------*/
/* Test arguments valid or not */
#define IS_VALID_1_ARG(a)              (NULL != a) ? 1 : 0
#define IS_VALID_2_ARG(a, b)           ((NULL != a) && (NULL != b)) ? 1 : 0

#define SAL_DEBUG

#ifdef SAL_DEBUG
#define SASSERT(a)                     BUG_ON(!(a)) 
#define SENTER()                       
#define SEXIT()                       
#define SLINE()                        

#else

#define SENTER()                       
#define SEXIT()                       
#define SASSERT(a)                     VOS_ASSERT(a) 
#define SLINE()                        

#endif

#define SMSGFATAL(a, b, c, d)          VOS_TRACE(VOS_MODULE_ID_SAL, VOS_TRACE_LEVEL_FATAL, a, b, c, d)
#define SMSGERROR(a, b, c, d)          VOS_TRACE(VOS_MODULE_ID_SAL, VOS_TRACE_LEVEL_ERROR, a, b, c, d)
#define SMSGWARN(a, b, c, d)           VOS_TRACE(VOS_MODULE_ID_SAL, VOS_TRACE_LEVEL_WARN, a, b, c, d)
#define SMSGINFO(a, b, c, d)           VOS_TRACE(VOS_MODULE_ID_SAL, VOS_TRACE_LEVEL_INFO_LOW, a, b, c, d)

#define SGETLOCK(a, b)                                        \
        do                                                    \
        {                                                     \
           mutex_lock(b);                                     \
           SMSGINFO("%s Lock success\n", a, 0, 0);            \
        }while(0)

#define SRELEASELOCK(a, b)                                    \
        do                                                    \
        {                                                     \
              mutex_unlock(b);                                \
              SMSGINFO("%s Release Lock success\n", a, 0, 0);\
        }while(0)


/*----------------------------------------------------------------------------
 *  Libra manufactures id based on which the probe function gets called.
 *  Libra function enable timeout
 *  Max block size. 
 * -------------------------------------------------------------------------*/
#define LIBRA_MAN_ID              0x70
#define LIBRA_FUNC_ENABLE_TIMEOUT 5000 // 5 sec
#define WLANSAL_MAX_BLOCK_SIZE    128

/*----------------------------------------------------------------------------
 *  Type Declarations
 * -------------------------------------------------------------------------*/

typedef struct 
{
   v_PVOID_t                         pAdapter;         // WLAN Adapter
   WLANSAL_SscRegType                sscCBs;           // SSC Call Backs
   WLANSAL_CardInfoType              cardInfo;         // Card capability
   struct mutex                      lock;             // Native lock for the SDIO i/f
   spinlock_t                        spinlock;         // Release of sdio device is controlled by this
   v_BOOL_t                          isINTEnabled;     // INT enabled flag
   struct sdio_func                  *sdio_func_dev;   // The handle to the sd/mmc sdio device
} salHandleType;

/*-------------------------------------------------------------------------
 *Function prototypes and documenation
 *-------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------

   @brief Claim SDIO device lock. Unified scheme used in SAL, BAL. 

   @param SDIO function device. 

   @return void
      
----------------------------------------------------------------------------*/
static inline void sd_claim_host(struct sdio_func *sdio_func_dev)
{
   hdd_adapter_t *pAdapter;

   pAdapter =  sdio_get_drvdata(sdio_func_dev);

   libra_claim_host(sdio_func_dev, &(pAdapter->pid_sdio_claimed), current->pid, &(pAdapter->sdio_claim_count));

}

/*----------------------------------------------------------------------------

   @brief Release SDIO device lock. Unified scheme used in SAL, BAL. 

   @param SDIO function device. 

   @return void
      
----------------------------------------------------------------------------*/
static inline void sd_release_host(struct sdio_func *sdio_func_dev)
{
   hdd_adapter_t *pAdapter;

   pAdapter =  sdio_get_drvdata(sdio_func_dev);

   libra_release_host(sdio_func_dev, &(pAdapter->pid_sdio_claimed), current->pid, &(pAdapter->sdio_claim_count));
}

#ifdef __cplusplus
}
#endif

#endif /* WLAN_SAL_MISC_H */


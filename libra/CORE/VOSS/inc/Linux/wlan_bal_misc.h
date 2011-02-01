#ifndef WLAN_BAL_MISC_H
#define WLAN_BAL_MISC_H

/*===========================================================================

               WLAN  BUS ABSTRACTION LAYER WINMOBILE - MISC MACROS FILE
                
                   
DESCRIPTION
  This file contains the external API exposed by the wlan SDIO abstraction layer module.
  
      
  Copyright (c) 2008 QUALCOMM Incorporated. All Rights Reserved.
  Qualcomm Confidential and Proprietary
===========================================================================*/

/*===========================================================================

                      EDIT HISTORY FOR FILE


  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.


  $Header:$ $DateTime: $ $Author: $


when           who        what, where, why
--------    ---         ----------------------------------------------------------
12/08/08    schang      AMSS porting
05/21/08    schang      Created module.
05/05/09                Linux porting

===========================================================================*/

/*===========================================================================

                          INCLUDE FILES FOR MODULE

===========================================================================*/

/*----------------------------------------------------------------------------
 * Include Files
 * -------------------------------------------------------------------------*/
#include <wlan_qct_ssc.h>
#include <wlan_hdd_includes.h>
#include <linux/mutex.h>
#include <linux/kernel.h>
#include <linux/mmc/card.h>
#include <linux/mmc/host.h>
#include <linux/mmc/sdio_func.h>

#ifdef __cplusplus
extern "C" {
#endif

/*----------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * -------------------------------------------------------------------------*/
/* Error MSG for BAL module */

//#define BAL_DEBUG
#ifdef BAL_DEBUG

#define BENTER()  VOS_TRACE( VOS_MODULE_ID_BAL, VOS_TRACE_LEVEL_INFO, "%s: Enter %d", __func__, __LINE__)                     
#define BEXIT()   VOS_TRACE( VOS_MODULE_ID_BAL, VOS_TRACE_LEVEL_INFO, "%s: Exit %d", __func__, __LINE__)                    
#define BLINE()   VOS_TRACE( VOS_MODULE_ID_BAL, VOS_TRACE_LEVEL_INFO, "%s: - %d - ", __func__, __LINE__)                     

#else

#define BENTER()  
#define BEXIT()  
#define BLINE() 

#endif

#define BMSGERROR(a, b, c, d)          VOS_TRACE(VOS_MODULE_ID_BAL, VOS_TRACE_LEVEL_ERROR, a, b, c, d)
#define BMSGWARN(a, b, c, d)           VOS_TRACE(VOS_MODULE_ID_BAL, VOS_TRACE_LEVEL_WARN, a, b, c, d)
#define BMSGINFO(a, b, c, d)           VOS_TRACE(VOS_MODULE_ID_BAL, VOS_TRACE_LEVEL_INFO_LOW, a, b, c, d)

#define BGETLOCK(message, lock)                               \
        do                                                    \
        {                                                     \
           mutex_lock(lock);                                  \
           BMSGINFO("%s Lock success", message, 0, 0);        \
        }while(0)

#define BRELEASELOCK(message, lock)                                 \
        do                                                          \
        {                                                           \
              mutex_unlock(lock);                                   \
              BMSGINFO("%s Release Lock success\n", message, 0, 0); \
        }while(0)



/* NON SIF Default Register size is fixed 4 bytes */
#define WLANBAL_ASIC_REGISTER_SIZE      4

#define VOS_GET_SSC_CTXT(a)            vos_get_context(VOS_MODULE_ID_SSC, a)

#define WLANBAL_DMA_MAX_BUFFER_SIZE     0x1000
/* UINT32 type endian swap */
#define SWAP_ENDIAN_UINT32(a)          (a = (a >> 0x18 ) |((a & 0xFF0000) >> 0x08) | \
                                            ((a & 0xFF00) << 0x08)  | ((a & 0xFF) << 0x18))



/* DXE Header config default values */
#define WLANBAL_DXE_TX_DESC_CTRL        0x000007ED
#define WLANBAL_DXE_TX_SRC_ADDRESS      0x0E024100
#define WLANBAL_DXE_TX_DEST_ADDRESS     23
#define WLANBAL_DXE_TX_NEXT_ADDRESS     0x0E024100

#define WLANBAL_DXE_RX_DESC_CTRL        0X0040076F
#define WLANBAL_DXE_RX_SRC_ADDRESS      11
#define WLANBAL_DXE_RX_DEST_ADDRESS     0x0E02421A
#define WLANBAL_DXE_RX_NEXT_ADDRESS     0x00

/* Address Translation */
#define WLANBAL_ADDRESS_BASE_MASK      0xFFFF0000
#define WLANBAL_ADDRESS_NON_SIF_MASK   0x00010000
#define WLANBAL_ADDRESS_TARGET_MASK    0x0000FFFF
#define WLANBAL_ADDRESS_BIT_SHIFT      16

#define WLANBAL_64K_BOUNDARY           0x10000

/* BD/PDU Resource masks */
#define WLANBAL_BD_PDU_16BIT_MASK       0x0000FFFF
#define WLANBAL_PDU_BIT_SHIFT           16

/* If TX resource is not enough, wait till this timeout period and try get again */
#define WLANBAL_TX_RESOURCE_TIMEOUT     10

/* Maximum TX BACKOFF TIMER Counter.Max TX resource timer value =
WLANBAL_MAX_TX_BACKOFF_COUNTER WLANBAL_TX_RESOURCE_TIMEOUT */
#define WLANBAL_MAX_TX_BACKOFF_COUNTER   10

/* Test arguments valid or not */
#define IS_VALID_1_ARG(a)              (NULL != a) ? 1 : 0
#define IS_VALID_2_ARG(a, b)           ((NULL != a) && (NULL != b)) ? 1 : 0


#define KTHREAD_COMM_INFO 'k'

//#define BAL_CLAIM_HOST_DEBUG

#ifdef BAL_CLAIM_HOST_DEBUG
#define BC_MSG(a, b, c, d)           VOS_TRACE(VOS_MODULE_ID_BAL, VOS_TRACE_LEVEL_INFO_LOW, a, b, c, d)
#else
#define BC_MSG(a, b, c, d)
#endif

/*----------------------------------------------------------------------------
 *  Type Declarations
 * -------------------------------------------------------------------------*/
typedef struct
{
   v_PVOID_t                       pAdapter;
   spinlock_t                      spinlock;
   vos_timer_t                     timer;
   WLANBAL_TlRegType               tlReg;
   WLANBAL_HalRegType              halCBacks;
   v_U32_t                         currentBaseAddress;
   WLANBAL_SDIODXEHeaderConfigType sdioDXEConfig;
   v_U8_t                          *dmaBuffer;
   struct sdio_func                *sdio_func_dev;
} balHandleType;



#ifdef __cplusplus
}
#endif

#endif 

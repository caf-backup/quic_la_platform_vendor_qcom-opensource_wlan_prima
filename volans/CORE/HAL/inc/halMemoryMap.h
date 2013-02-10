/**
 *
 *  @file:         halMemoryMap.h
 *
 *  @brief:        Header file for the MTU Hardware Block.
 *
 *  @author:       Susan Tsao
 *
 *  Copyright (C) 2008, Qualcomm Technologies, Inc. All rights reserved.
 *
 *  Change History:
 * ---------------------------------------
 * 02/06/2006  File created.
 * 01/16/2008  Virgo related changes.
 */
#ifndef _HALMEMORYMAP_H_
#define _HALMEMORYMAP_H_

#include "halTypes.h"

/* ---------------------------------------------------------
 *  CFG values - to be removed once cfg download is working
 * ----------------------------------------------------------
 */
#define MAX_NUM_OF_TIDS                16
#define MAX_TXFRAMES                   512
#define MAX_RATE_ENTRIES               224
#define MAX_RC_ENTRIES_PER_SET         256

#define MCPU2HOST_MSG_SIZE             0x100    // 256 bytes (0x100), should match QWLANFW_MBOX_MSG_LENGTH in qwlan_macfw.h
#define HOST2MCPU_MSG_SIZE             0x100    // 256 bytes (0x100), should match QWLANFW_MBOX_MSG_LENGTH in qwlan_macfw.h

#define DXE_RXDESC_INFO_SIZE           0x80                          // 128 bytes
#define BEACON_TEMPLATE_SIZE           0x180
#define RA_STA_INFO_SIZE               sizeof(tHalRaInfo)
#define RA_BSS_INFO_SIZE               sizeof(tHalRaBssInfo)
#ifdef WLAN_SOFTAP_FEATURE
#define PROBE_RSP_TEMPLATE_MAX_SIZE    0x180 //unit byte.
#define STA_INFO_SIZE                  sizeof(tStaInfo)
#define BSS_INFO_SIZE                  sizeof(tBssInfo)
#endif


#define MAX_STA_ENTRIES_PER_RC_SET MAX_RC_ENTRIES_PER_SET / MAX_NUM_OF_TIDS

#ifdef WLAN_HAL_VOLANS
#define MCPU2HOST_MSG_OFFSET           0x600    // 0x1200, should match QWLANFW_MEM_F2H_MSG_ADDR_OFFSET in qwlan_macfw.h
#define HOST2MCPU_MSG_OFFSET           0x500    // 0x1000, should match QWLANFW_MEM_H2F_MSG_ADDR_OFFSET in qwlan_macfw.h
#else
#define MCPU2HOST_MSG_OFFSET           0x1200    // 0x1200, should match QWLANFW_MEM_F2H_MSG_ADDR_OFFSET in qwlan_macfw.h
#define HOST2MCPU_MSG_OFFSET           0x1000    // 0x1000, should match QWLANFW_MEM_H2F_MSG_ADDR_OFFSET in qwlan_macfw.h
#endif

#define DPU_DESCRIPTOR_OFFSET          BMU_MEMORY_BASE_ADDRESS                            // 0x0

#define FW_IMAGE_MEMORY_BASE_ADDRESS    0x00

#define HAL_MEM_BOUNDARY_ALIGN          128


#define TPE_PER_STA_STATS_SIZE          sizeof(tTpeStaStats)    /**<    0x240 */
#define TPE_STA_DESC_ENTRY_SIZE         sizeof(tTpeStaDesc)
#define RPE_STA_DESC_ENTRY_SIZE         sizeof(tRpeStaDesc)
#define TPE_STA_DESC_AND_STATS_SIZE     (TPE_STA_DESC_ENTRY_SIZE + TPE_PER_STA_STATS_SIZE)
#define RPE_PARTIAL_BITMAP_SIZE         sizeof(tRpePartialBAInfo)

#define ADU_UMA_STA_DESC_ENTRY_SIZE     sizeof(tAduUmaStaDesc)

#ifdef FEATURE_ON_CHIP_REORDERING
#define MAX_NUM_OF_ONCHIP_REORDER_SESSIONS   2
#define NO_OF_RPE_REORDER_STA_DS_ENTRIES     (BA_DEFAULT_RX_BUFFER_SIZE/2)
#define RPE_REORDER_STA_DS_SIZE         sizeof(sRpeReorderStaData)
#endif

#ifdef VOLANS_PHY_TX_OPT_ENABLED
#define ADU_REG_RECONFIG_TABLE_SIZE     0xAC8   /* 0x2800 - (size occupied by Phy TX registers). This can be still reduced */
#define ADU_PHY_TX_REG_RECONFIG_TABLE_SIZE     0x1D38   /* Approx 850 Phy registers. With 10% extra space */
#else
#define ADU_REG_RECONFIG_TABLE_SIZE     0x2800   /* 14K for the register re-initialization list */
#endif /* VOLANS_PHY_TX_OPT_ENABLED */

#define ADU_MIMO_PS_PRG_SIZE            0x200    /* 256bytes to start with... may need to be increased */

#define HW_TEMPLATE_BASE                0x00
#define TEMPLATE_ACK_OFFSET             (HW_TEMPLATE_BASE + 0x00)
#define TEMPLATE_RTS_OFFSET             (HW_TEMPLATE_BASE + 0x20)
#define TEMPLATE_CTS_OFFSET             (HW_TEMPLATE_BASE + 0x40)
#define TEMPLATE_BAR_OFFSET             (HW_TEMPLATE_BASE + 0x60)
#define TEMPLATE_BA_OFFSET              (HW_TEMPLATE_BASE + 0x80)
#define TEMPLATE_PS_POLL_OFFSET         (HW_TEMPLATE_BASE + 0xA0)
#define TEMPLATE_QOS_NULL_OFFSET        (HW_TEMPLATE_BASE + 0xC0)
#define TEMPLATE_CF_END_OFFSET          (HW_TEMPLATE_BASE + 0xE0)
#define TEMPLATE_DATA_NULL_OFFSET       (HW_TEMPLATE_BASE + 0x100)
#define HW_TEMPLATE_SIZE                (HW_TEMPLATE_BASE + 0x120)

#define RPE_STA_DESC_QUEUE_SIZE         sizeof(tRpeStaQueueInfo)
#define TPE_PER_STA_STATS_START_OFFSET  TPE_STA_DESC_ENTRY_SIZE

#define SW_TEMPLATE_SIZE                0x100

eHalStatus halMemoryMap_Open(tHalHandle hHal, void *arg);
eHalStatus halMemoryMap_Start(tHalHandle hHal, void *arg);


#endif /* _HALMEMORYMAP_H_ */




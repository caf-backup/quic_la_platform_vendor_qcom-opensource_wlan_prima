#if !defined( __LEGACYPALTYPES_H__ )
#define __LEGACYPALTYPES_H__

/*==========================================================================
 *
 *  @file:     halLegacyPalTypes.h
 *
 *  @brief:    Exports and types for the Platform Abstraction Layer typedefs.
 *
 *  @author:   Kumar Anand
 *
 *             Copyright (C) 2010, Qualcomm, Inc. 
 *             All rights reserved.
 *
 *=========================================================================*/

#include "wlan_defs.h"

/************************************************************************
 * List of defines to enable/disable HAL Features.
 * These should go to a Makefie eventually.
 ************************************************************************/

#ifndef ANI_PRODUCT_TYPE_CLIENT
#define ANI_PRODUCT_TYPE_CLIENT
#endif

#ifndef ANI_OS_TYPE_REX
#define ANI_OS_TYPE_REX
#endif

#ifndef ANI_COMPILER_TYPE_RVCT
#define ANI_COMPILER_TYPE_RVCT
#endif

#ifndef ANI_LITTLE_BYTE_ENDIAN
#define ANI_LITTLE_BYTE_ENDIAN
#endif

#ifndef WLAN_DEBUG
#define WLAN_DEBUG
#endif

//#ifndef PRIMA_PHY_TX_OPT_ENABLED
//#define PRIMA_PHY_TX_OPT_ENABLED
//#endif

//#ifndef ADU_MEM_OPT_ENABLED
//#define ADU_MEM_OPT_ENABLED
//#endif

#ifndef WLAN_HAL_PRIMA
#define WLAN_HAL_PRIMA
#endif

#ifndef FW_PRESENT
#define FW_PRESENT
#endif

#ifndef HAL_SELF_STA_PER_BSS
#define HAL_SELF_STA_PER_BSS        1
#endif

#ifndef HAL_BCAST_STA_PER_BSS
#define HAL_BCAST_STA_PER_BSS       1
#endif

#ifndef HAL_INT_DEBUG
#define HAL_INT_DEBUG
#endif

#ifndef PRIMA_FPGA
#define PRIMA_FPGA
#endif

//#ifndef FEATURE_WLAN_WAPI
//#define FEATURE_WLAN_WAPI
//#endif

//#ifndef __HAL_UNIT_TEST__
//#define __HAL_UNIT_TEST__
//#endif

/* Common type definitions */
typedef uint8     tANI_U8;
typedef int8      tANI_S8;
typedef uint16    tANI_U16;
typedef int16     tANI_S16;
typedef uint32    tANI_U32;
typedef int32     tANI_S32;
typedef uint64    tANI_U64;
typedef byte      tANI_BYTE;
typedef boolean   tANI_BOOLEAN;
typedef uint32    tANI_TIMESTAMP;

#endif /*__LEGACYPALTYPES_H__*/

/*
* Copyright (c) 2012-2013 Qualcomm Atheros, Inc.
* All Rights Reserved.
* Qualcomm Atheros Confidential and Proprietary.
*/

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
 *             Copyright (C) 2010, Qualcomm Technologies, Inc.
 *             All rights reserved.
 *
 *=========================================================================*/


/* Common type definitions */
typedef unsigned char   tANI_U8;
typedef signed char     tANI_S8;
typedef unsigned short  tANI_U16;
typedef signed short    tANI_S16;
typedef unsigned long   tANI_U32;
typedef signed long     tANI_S32;

typedef tANI_U8        	uint8;
typedef tANI_S8        	int8;
typedef tANI_U16       	uint16;
typedef tANI_S16      	int16;
typedef tANI_U32       	uint32;
typedef tANI_S32       	int32;
#ifndef BUILD_QWPTTSTATIC
typedef long long       tANI_U64;
typedef tANI_U64       	uint64;
#endif

typedef unsigned char   tANI_BYTE;
typedef unsigned char   tANI_BOOLEAN;
typedef unsigned long   tANI_TIMESTAMP;

typedef tANI_BYTE      	tANI_U8;
typedef tANI_BOOLEAN   	boolean;
typedef tANI_TIMESTAMP 	uint32;
#endif /*__LEGACYPALTYPES_H__*/

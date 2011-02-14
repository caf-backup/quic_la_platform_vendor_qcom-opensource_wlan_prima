/*
 * File:        halFw.h
 * Description: This file contains the FW file to be used int the host software
 *
 * Copyright (c) 2008 QUALCOMM Incorporated.
 * All Rights Reserved.
 * Qualcomm Confidential and Proprietary
 *
 *
 * History:
 *
 * When       Who         What/Where/Why
 * -------------------------------------------------------------------
 * 08/22/2008 lawrie      Created
 *
 *
 */

#ifndef _HALFW_H_
#define _HALFW_H_

#include "palTypes.h"

#ifndef PACKED_PRE
#define PACKED_PRE   __ani_attr_pre_packed
#endif

#ifndef PACKED_POST
#define PACKED_POST   __ani_attr_packed
#endif

#ifndef ALIGN_4
#define ALIGN_4    __ani_attr_aligned_4
#endif

#if defined(__ANI_COMPILER_PRAGMA_PACK_STACK)
#pragma pack(push, 1)
#elif defined(__ANI_COMPILER_PRAGMA_PACK)
#pragma pack(1)
#else
#endif

#include "qwlan_macfw.h"

#if defined(__ANI_COMPILER_PRAGMA_PACK_STACK)
#pragma pack(pop)
#elif defined(__ANI_COMPILER_PRAGMA_PACK)
#else
#endif


#endif //_HALFW_H_

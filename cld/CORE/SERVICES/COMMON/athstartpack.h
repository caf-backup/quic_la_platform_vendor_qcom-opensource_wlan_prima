/*
 * Copyright (c) 2013 Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Confidential and Proprietary.
 */


//------------------------------------------------------------------------------
// <copyright file="athstartpack.h" company="Atheros">
//    Copyright (c) 2004-2010 Atheros Corporation.  All rights reserved.
// $ATH_LICENSE_HOSTSDK0_C$
//------------------------------------------------------------------------------
//==============================================================================
// start compiler-specific structure packing
//
// Author(s): ="Atheros"
//==============================================================================
#ifndef _ATHSTARTPACK_H
#define _ATHSTARTPACK_H

#if defined(LINUX) || defined(__linux__)
#include "osapi_linux.h"
#endif /* LINUX */

#ifdef QNX
#endif /* QNX */

#if __LONG_MAX__ == __INT_MAX__
/* 32-bit compilation */
#define PREPACK64
#define POSTPACK64
#else
/* 64-bit compilation */
#define PREPACK64 PREPACK
#define POSTPACK64 POSTPACK
#endif

#endif /* _ATHSTARTPACK_H */

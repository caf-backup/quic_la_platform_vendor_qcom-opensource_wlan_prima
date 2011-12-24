/*
 * Copyright (c) 2011 Qualcomm Atheros, Inc. 
 * All Rights Reserved. 
 * Qualcomm Atheros Confidential and Proprietary. 
 *
 * Copyright (C) 2006 Airgo Networks, Incorporated
 *
 * This file halHddApi.h contains the prototypes for DVT Apis
 * that are used by Hdd
 *
 * Author:      Sam Hsieh
 * Date:        03/10/2006
 * History:-
 * Date         Modified by    Modification Information
 * --------------------------------------------------------------------
 */

#ifndef _DVTHDDAPI_H_
#define _DVTHDDAPI_H_

#if (WNI_POLARIS_FW_OS == SIR_WINDOWS)
#include <ndis.h>
#endif

typedef struct sAniSirGlobal *tpAniSirGlobal;

#include "sirParams.h"
#include "dvtMsgApi.h"

eANI_DVT_STATUS dvtProcessMsg(tpAniSirGlobal pMac, tDvtMsgbuffer *pDvtMsg);
tSirRetStatus dvtMmhForwardMBmsg(void* pSirGlobal, tSirMbMsg* pMb);

#endif // _DVTHDDAPI_H_

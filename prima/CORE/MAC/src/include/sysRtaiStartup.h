/* 
 * Copyright (c) 2011-2012 Qualcomm Atheros, Inc. 
 * All Rights Reserved. 
 * Qualcomm Atheros Confidential and Proprietary. 
 *
 * Airgo Networks, Inc proprietary. All rights reserved.
 * sysStartup.h: System startup header file.
 *
 * Author:      V. K. Kandarpa
 * Date:        07/26/2002
 *
 * History:-
 * Date         Modified by            Modification Information
 * --------------------------------------------------------------------------
 *
 */

# ifndef __SYSRTAISTARTUP_H
# define __SYSRTAISTARTUP_H

#include <sirCommon.h>
#include <aniParam.h>
#include "halTypes.h"
extern void sysRecvPacket(tHalHandle hHal, void* pPacket);
extern void sysRtaiStartup(tAniMacParam*);
extern void sysRtaiCleanup(tHalHandle hHal);
extern void sysMacModInit(tAniMacParam * pParam, struct rtLibApp * rt);
extern void sysMacModExit(tHalHandle hHal);
extern eHalStatus sysMailboxRead(tHalHandle hHal, void * message);
extern eHalStatus sysMailboxWrite(tHalHandle hHal, void * message);
extern int mac_mod_init(tAniMacParam * pParam);
extern void mac_mod_exit(tAniMacParam * pParam);

# endif /* __SYSSTARTUP_H */

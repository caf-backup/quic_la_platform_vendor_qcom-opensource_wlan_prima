/**
 *
   Airgo Networks, Inc proprietary.
   All Rights Reserved, Copyright 2005 2006
   This program is the confidential and proprietary product of Airgo Networks Inc.
   Any Unauthorized use, reproduction or transfer of this program is strictly prohibited.


   pttMsgApi.h: Handles kernal mode messages sent from socket or pttApi.dll
   Author:  Mark Nelson
   Date:    6/22/05

   History -
   Date        Modified by              Modification Information
  --------------------------------------------------------------------------

 */

#include "ani_assert.h"
#include "string.h"
#include "pttModuleApi.h"

#if defined(ANI_MANF_DIAG) || defined(ANI_PHY_DEBUG)




void pttProcessMsg(tpAniSirGlobal pMac, tPttMsgbuffer *pttMsg)
{
}

#endif  //defined(ANI_MANF_DIAG) || defined(ANI_PHY_DEBUG)


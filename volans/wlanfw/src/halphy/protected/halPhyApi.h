/**
 *
   Airgo Networks, Inc proprietary.
   All Rights Reserved, Copyright 2005, 2007
   This program is the confidential and proprietary product of Airgo Networks Inc.
   Any Unauthorized use, reproduction or transfer of this program is strictly prohibited.


   halPhyApi.h: halPhy interface
   Author:  Mark Nelson
   Date:    4/9/05

   History -
   Date        Modified by              Modification Information
  --------------------------------------------------------------------------

 */

#ifndef HALPHYAPI_H
#define HALPHYAPI_H

#include <halPhy.h>


void halPhyInitCal(void);

tANI_U8 halPhyQueryNumRxChains(ePhyChainSelect phyRxTxAntennaMode);
tANI_U8 halPhyQueryNumTxChains(ePhyChainSelect phyRxTxAntennaMode);
ePhyChainSelect halPhyGetActiveChainSelect();
void halPhySetChainSelect(ePhyChainSelect phyRxTxAntennaMode);

void halPhySetChannel(tANI_U8 channelNumber, ePhyChanBondState cbState);

#ifdef CHANNEL_BONDED_CAPABLE
void halPhySetChannelBondState(ePhyChanBondState chBondState);
#endif

void halPhySetRxPktsDisabled(ePhyRxDisabledPktTypes rxDisabled);

#endif /* HALPHYAPI_H */

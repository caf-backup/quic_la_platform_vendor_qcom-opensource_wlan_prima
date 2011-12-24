/**
 *

   Copyright (c) 2011 Qualcomm Atheros, Inc. 
   All Rights Reserved. 
   Qualcomm Atheros Confidential and Proprietary. 
  
   Copyright (C) 2006 Airgo Networks, Incorporated

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

void halPhySetChannel(tANI_U8 channelNumber, ePhyChanBondState cbState
#ifdef WLAN_AP_STA_CONCURRENCY
          , tANI_U8 bSendCts
#endif
          );

#ifdef CHANNEL_BONDED_CAPABLE
void halPhySetChannelBondState(ePhyChanBondState chBondState);
#endif

void halPhySetRxPktsDisabled(ePhyRxDisabledPktTypes rxDisabled);

#endif /* HALPHYAPI_H */

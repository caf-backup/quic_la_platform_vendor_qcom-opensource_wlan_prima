/**
 *
   Airgo Networks, Inc proprietary.
   All Rights Reserved, Copyright 2005
   This program is the confidential and proprietary product of Airgo Networks Inc.
   Any Unauthorized use, reproduction or transfer of this program is strictly prohibited.


   phyGlobal.h: Holds all globals for the phy, rf, and asic layers in hal
   Author:  Mark Nelson
   Date:    4/9/05

   History -
   Date        Modified by              Modification Information
  --------------------------------------------------------------------------

 */

#ifndef PHYGLOBAL_H
#define PHYGLOBAL_H


#include <rf.h>
#include <phy.h>
#include <asic.h>



typedef struct
{
    //physical layer data - corresponds to individual modules
    tPhy        phy;
    tPhyTxPower phyTPC;
    tRF         rf;
    tAsicAgc    agc;

    tANI_BOOLEAN wfm_clk80; //=ON if 20MHZ clock samples, =OFF for 80MHZ clock samples
    ePhyRxDisabledPktTypes modTypes;  //current disabled packet types
}tPhyGlobal;


#endif /* PHYGLOBAL_H */

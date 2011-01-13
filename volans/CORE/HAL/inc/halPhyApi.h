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

#include "halPhy.h"
#include <halPhyRates.h>

#ifdef VERIFY_HALPHY_SIMV_MODEL //To get rid of multiple definition error, in eazy way.
#define halPhySetRxPktsDisabled     host_halPhySetRxPktsDisabled
#define halPhyQueryNumRxChains      host_halPhyQueryNumRxChains
#define halPhyQueryNumTxChains      host_halPhyQueryNumTxChains
#define halPhyGetActiveChainSelect  host_halPhyGetActiveChainSelect
#define halPhySetChainSelect        host_halPhySetChainSelect
#define halPhySetChannel            host_halPhySetChannel
#endif


//Allocates memory and configures the physical layer with information from EEPROM before other chip initializations
eHalStatus halPhyOpen(tHalHandle hHal);

//Frees physical layer memory
eHalStatus halPhyClose(tHalHandle hHal);

//After other chip initializations, initializes the physical layer to ready-to-run state
// on first channel in regulatory domain with configured number of tx/rx
eHalStatus halPhyStart(tHalHandle hHal);

//Resets the physical layer globals
eHalStatus halPhyStop(tHalHandle hHal);

/***********************************************************/
/* All functions below can be invoked after initialization */
/***********************************************************/

//disable/enable specific received packet types
eHalStatus halPhySetRxPktsDisabled(tHalHandle hHal, ePhyRxDisabledPktTypes rxDisabled);
eHalStatus halPhyDisableAllPackets(tHalHandle hHal);


//Alter the AGC CCA mode in use per sub-channel
eHalStatus halPhySetAgcCCAMode(tHalHandle hHal, ePhyCCAMode primaryCcaMode, ePhyCCAMode secondaryCcaMode);

//Functions to determine or set the chain selection mode
ePhyChainSelect halPhyGetChainSelect(tHalHandle hHal, tANI_U8 numTxChains, tANI_U8 numRxChains);
tANI_U8 halPhyQueryNumRxChains(ePhyChainSelect phyRxTxAntennaMode);
tANI_U8 halPhyQueryNumTxChains(ePhyChainSelect phyRxTxAntennaMode);
ePhyChainSelect halPhyGetActiveChainSelect(tHalHandle hHal);
eHalStatus halPhySetChainSelect(tHalHandle hHal, ePhyChainSelect phyRxTxAntennaMode);


//to determine if it is time to perform a calibration
eHalStatus halPhyAssessCal(tHalHandle hHal, tANI_BOOLEAN *performCal);

//may be invoked at any convenient time to calibrate the physical layer
// traffic must be stopped at the time.
eHalStatus halPhyCalUpdate(tHalHandle hHal);

//to set the primary channel and which channel-bonded state this is
eHalStatus halPhySetChannel(tHalHandle hHal, tANI_U8 channelNumber, ePhyChanBondState cbState, tANI_U8 calRequired);

//to get the current channel-bonded state
ePhyChanBondState halPhyGetChannelBondState(tHalHandle hHal);

//to set the current regulatory domain
eHalStatus halPhySetRegDomain(tHalHandle hHal, eRegDomainId regDomain);

//To get the current regulatory domain
eRegDomainId halPhyGetRegDomain(tHalHandle hHal);

//to get the lists of channels found enabled in the current regulatory domain
eHalStatus halPhyGetChannelListWithPower(tHalHandle hHal,
                                                tChannelListWithPower *channels20MHz /*[NUM_LEGIT_RF_CHANNELS] */, tANI_U8 *num20MHzChannelsFound,
                                                tChannelListWithPower *channels40MHz /*[NUM_CHAN_BOND_CHANNELS] */, tANI_U8 *num40MHzChannelsFound
                                        );


//sets the density to on or off, and the setting if it's on
//if density is off, then close proximity is set - primarily used for cage testing at Airgo
eHalStatus halPhySetNwDensity(tHalHandle hHal, tANI_BOOLEAN densityOn, ePhyNwDensity density20MHz, ePhyNwDensity density40MHz);


//gets the Transmit power template index to use for the specified rate, for the current channel and regulatory domain limits
//OUTPUT: retTemplateIndex
eHalStatus halPhyGetPowerForRate(tHalHandle hHal, eHalPhyRates rate, ePowerMode pwrMode,
                                    tPowerdBm absPwrLimit, tPwrTemplateIndex *retTemplateIndex);


tPowerdBm halPhyGetRegDomainLimit(tHalHandle hHal, eHalPhyRates rate);

//dumps phy registers and will attempt to diagnose transmit and receive from the bottom up
//the callerStr is required to annotate the location that this was called from.
//Note this can be used internally at key points or also externally through a test command.
eHalStatus halPhyDiagnose(tHalHandle hHal, char *callerStr);


//returns the power template index corresponding to the desired total output mWatts
// this takes into account the number antennas and antenna gains, but is not capped by channel power limits.
eHalStatus halPhySetTxMilliWatts(tHalHandle hHal, t_mW mWatts, tPwrTemplateIndex *retTemplateIndex);


//returns an integer number of milli-watts based on the pwrTemplateIndex
eHalStatus halPhyGetTxMilliWatts(tHalHandle hHal, tPwrTemplateIndex pwrTemplateIndex, t_mW *ret_mWatts);


//The get the supported channel list.
//As input, pNum20MhzChannels is the size of the array of p20MhzChannels.
//Upon return, pNum20MhzChannels has the number of supported channels.
//When successfully return, p20MhzChannels contains the channel ID.
eHalStatus halPhyGetSupportedChannels( tHalHandle hHal, tANI_U8 *p20MhzChannels, int *pNum20MhzChannels,
                                       tANI_U8 *p40MhzChannels, int *pNum40MhzChannels);

// Routine to update tpc tx gain override in open loop mode.
eHalStatus halPhyUpdateTxGainOverride(tHalHandle hHal, tANI_U8 txGain);


// Routine to collect the adc rssi stats
void halPhyAdcRssiStatsCollection(tHalHandle hHal);

// Routine to configure AGC listen mode
eHalStatus halPhyAGCEnableListenMode(tHalHandle hHal, tANI_U8 EDETThreshold);
eHalStatus halPhyAGCDisableListenMode(tHalHandle hHal);
// Routine to do PHY settings post fw init
eHalStatus halPhyFwInitDone(tHalHandle hHal);

/* Routine to return the power in Dbm for the given rate from Rate 2 power table */
eHalStatus halPhyGetPwrFromRate2PwrTable(tHalHandle hHal, eHalPhyRates rate, t2Decimal *pwr2dec);
/* Routine to return the maxPwrIndex that can be used for the given absolute power limit in dBm */
inline eHalStatus halPhyGetMaxTxPowerIndex(tHalHandle hHal, tPowerdBm absPwrLimit, tPwrTemplateIndex *retTemplateIndex);
#endif /* HALPHYAPI_H */

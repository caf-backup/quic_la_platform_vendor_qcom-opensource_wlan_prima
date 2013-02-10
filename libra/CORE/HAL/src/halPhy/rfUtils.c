/** ------------------------------------------------------------------------- *
    ------------------------------------------------------------------------- *


    \file rfUtils.c

    \brief Gemini functionality

    $Id$

    Copyright (C) 2008 Qualcomm Technologies, Inc.


   ========================================================================== */


#include "sys_api.h"


const tRfChannelProps rfChannels[NUM_RF_CHANNELS] =
{
    //RF_SUBBAND_2_4_GHZ
    //freq, chan#, band
    { 2412, 1  , RF_SUBBAND_2_4_GHZ},        //RF_CHAN_1,
    { 2417, 2  , RF_SUBBAND_2_4_GHZ},        //RF_CHAN_2,
    { 2422, 3  , RF_SUBBAND_2_4_GHZ},        //RF_CHAN_3,
    { 2427, 4  , RF_SUBBAND_2_4_GHZ},        //RF_CHAN_4,
    { 2432, 5  , RF_SUBBAND_2_4_GHZ},        //RF_CHAN_5,
    { 2437, 6  , RF_SUBBAND_2_4_GHZ},        //RF_CHAN_6,
    { 2442, 7  , RF_SUBBAND_2_4_GHZ},        //RF_CHAN_7,
    { 2447, 8  , RF_SUBBAND_2_4_GHZ},        //RF_CHAN_8,
    { 2452, 9  , RF_SUBBAND_2_4_GHZ},        //RF_CHAN_9,
    { 2457, 10 , RF_SUBBAND_2_4_GHZ},        //RF_CHAN_10,
    { 2462, 11 , RF_SUBBAND_2_4_GHZ},        //RF_CHAN_11,
    { 2467, 12 , RF_SUBBAND_2_4_GHZ},        //RF_CHAN_12,
    { 2472, 13 , RF_SUBBAND_2_4_GHZ},        //RF_CHAN_13,
    { 2484, 14 , RF_SUBBAND_2_4_GHZ},        //RF_CHAN_14,
};


//channel/band/freq functions
eRfSubBand rfGetBand(tpAniSirGlobal pMac, eRfChannels chan)
{
    assert(chan < NUM_RF_CHANNELS);
    //return(rfChannels[chan].band);        restore later for 5GHz product
    return (RF_SUBBAND_2_4_GHZ);
}


eRfSubBand rfGetAGBand(tpAniSirGlobal pMac)
{
/*  //restore later for 5GHz product
        eRfSubBand  bandIndex;
    
        bandIndex = rfGetBand(pMac, rfGetCurChannel(pMac));
    
        switch (bandIndex)
        {
            case RF_SUBBAND_2_4_GHZ:
                bandIndex = RF_BAND_2_4_GHZ;
                break;
    
            default:
                phyLog(pMac, LOGE, "ERROR: band not found\n");
                return (INVALID_RF_SUBBAND);
        }
*/
    return(RF_BAND_2_4_GHZ);
}


eRfChannels rfGetCurChannel(tpAniSirGlobal pMac)
{
    return(pMac->hphy.rf.curChannel);
}

tANI_U16 rfChIdToFreqCoversion(tANI_U8 chanNum)
{
    int i;

    for (i = 0; i < NUM_RF_CHANNELS; i++)
    {
        if (rfChannels[i].channelNum == chanNum)
        {
            return rfChannels[i].targetFreq;
        }
    }

    return (0);
}

eRfChannels rfGetChannelIndex(tANI_U8 chanNum, ePhyChanBondState cbState)
{
    int i = MIN_20MHZ_RF_CHANNEL;
    int max = MAX_20MHZ_RF_CHANNEL;
    
    assert(cbState == PHY_SINGLE_CHANNEL_CENTERED);
    if (cbState == PHY_SINGLE_CHANNEL_CENTERED)
    {
        i = MIN_20MHZ_RF_CHANNEL;
        max = MAX_20MHZ_RF_CHANNEL;
    }
    
    //linear search through the valid channels
    for (; (i <= max); i++)
    {
        if (rfChannels[i].channelNum == chanNum)
        {
            return ((eRfChannels)i);
        }
    }

    return INVALID_RF_CHANNEL;
}

tANI_U8 rfGetChannelIdFromIndex(eRfChannels chIndex)
{
    //assert(chIndex < NUM_RF_CHANNELS);

    return (rfChannels[chIndex].channelNum);
}


//The get the supported channel list.
//As input, pNum20MhzChannels is the size of the array of p20MhzChannels.
//Upon return, pNum20MhzChannels has the number of supported channels.
//When successfully return, p20MhzChannels contains the channel ID.
eHalStatus halPhyGetSupportedChannels( tHalHandle hHal, tANI_U8 *p20MhzChannels, int *pNum20MhzChannels,
                                       tANI_U8 *p40MhzChannels, int *pNum40MhzChannels)
{
    //tpAniSirGlobal pMac = PMAC_STRUCT( hHal );
    eHalStatus status = eHAL_STATUS_INVALID_PARAMETER;

    if( p20MhzChannels && pNum20MhzChannels )
    {
        if( *pNum20MhzChannels >= NUM_RF_CHANNELS )
        {
            int i;

            for( i = 0; i < NUM_RF_CHANNELS; i++ )
            {
                p20MhzChannels[i] = rfGetChannelIdFromIndex( i );
            }
            status = eHAL_STATUS_SUCCESS;
        }
        *pNum20MhzChannels = NUM_RF_CHANNELS;
    }

    //Ignore 40Mhz channels for now.

    return (status);
}



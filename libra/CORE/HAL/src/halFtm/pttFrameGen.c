/** ------------------------------------------------------------------------- *
    ------------------------------------------------------------------------- *


    \file pttFrameGen.c

    \brief PTT Module interfaces for packet generation

    $Id$

    Copyright (C) 2006 Airgo Networks, Incorporated


   ========================================================================== */
#include "sys_api.h"
#include "pttModuleApi.h"



#define SUCCESS     PTT_STATUS_SUCCESS
#define FAILURE     PTT_STATUS_FAILURE

#ifndef WLAN_FTM_STUB

//Tx Frame Gen Service
eQWPttStatus pttConfigTxPacketGen(tpAniSirGlobal pMac, sPttFrameGenParams frameParams)
{
    if ((frameParams.numTestPackets > QWLAN_PHYDBG_TXPKT_CNT_CNT_MASK) ||
        (frameParams.interFrameSpace > (MSK_24 / ONE_MICROSECOND)) ||
        (frameParams.rate > NUM_HAL_PHY_RATES) ||
        (frameParams.payloadContents > TEST_PAYLOAD_TEMPLATE) ||
        (frameParams.payloadLength > MAX_PAYLOAD_SIZE) ||
        (frameParams.pktAutoSeqNum > eANI_BOOLEAN_TRUE) ||
        (frameParams.pktScramblerSeed > MSK_7)
       )
    {
        return (FAILURE);
    }
#ifdef CHANNEL_BONDED_CAPABLE
    if ((pMac->hphy.phy.test.testCbState == PHY_SINGLE_CHANNEL_CENTERED) &&
        (TEST_PHY_RATE_IS_CB(frameParams.rate) == eANI_BOOLEAN_TRUE)
       )
    {
        //can't allow 40MHz rate on a 20MHz channel - it will hang PhyDbg!
        return (FAILURE);
    }
#endif
    pMac->ptt.frameGenParams.numTestPackets =   frameParams.numTestPackets;
    pMac->ptt.frameGenParams.interFrameSpace =  frameParams.interFrameSpace;
    pMac->ptt.frameGenParams.rate =             frameParams.rate;
    pMac->ptt.frameGenParams.payloadContents =  frameParams.payloadContents;
    pMac->ptt.frameGenParams.payloadLength =    frameParams.payloadLength;
    pMac->ptt.frameGenParams.payloadFillByte =  frameParams.payloadFillByte;
    pMac->ptt.frameGenParams.pktAutoSeqNum =    frameParams.pktAutoSeqNum;
    pMac->ptt.frameGenParams.pktScramblerSeed = frameParams.pktScramblerSeed;
    pMac->ptt.frameGenParams.preamble =         frameParams.preamble;
    memcpy(&pMac->ptt.frameGenParams.addr1[0], &frameParams.addr1[0], ANI_MAC_ADDR_SIZE);
    memcpy(&pMac->ptt.frameGenParams.addr2[0], &frameParams.addr2[0], ANI_MAC_ADDR_SIZE);
    memcpy(&pMac->ptt.frameGenParams.addr3[0], &frameParams.addr3[0], ANI_MAC_ADDR_SIZE);
    pMac->ptt.frameGenParams.crc =              frameParams.crc;

    //stop/restart frame gen
    if (pttStartStopTxPacketGen(pMac, eANI_BOOLEAN_OFF) == SUCCESS)
    {
        if ((pMac->ptt.frameGenEnabled  == eANI_BOOLEAN_TRUE) && (pMac->ptt.phyDbgFrameGen == eANI_BOOLEAN_TRUE))
        {
            if (pttStartStopTxPacketGen(pMac, eANI_BOOLEAN_ON) == FAILURE)
            {
                return (FAILURE);
            }
        }
    }
    else
    {
        return (FAILURE);
    }

    return (SUCCESS);
}


eQWPttStatus pttStartStopTxPacketGen(tpAniSirGlobal pMac, tANI_BOOLEAN startStop)
{
    if ((startStop > 1) || (pMac->ptt.wfmEnabled == eANI_BOOLEAN_TRUE))
    {
        return(FAILURE);
    }

    if (startStop == eANI_BOOLEAN_TRUE)
    {
        if (halPhyQueryNumTxChains(halPhyGetActiveChainSelect(pMac)) == 0)
        {
            // we cannot allow frame generation to start if no transmitters are enabled
            //   we discovered that with Titan, if frame generation is started with both transmitters overriden disabled
            //   it will loopback the frames in the MAC
            if ((pMac->ptt.frameGenEnabled) && (pMac->ptt.phyDbgFrameGen))
            {
                if (asicPhyDbgStopFrameGen(pMac) != eHAL_STATUS_SUCCESS)
                {
                   return (FAILURE);
                }

                //now allow receive again
                if (asicAGCReset(pMac) != eHAL_STATUS_SUCCESS)
                {
                   return (FAILURE);
                }
            }
            pMac->ptt.frameGenEnabled = FALSE;

            return (FAILURE);
        }

        if (pMac->ptt.frameGenEnabled == eANI_BOOLEAN_TRUE)
        {
            phyLog(pMac, LOGW, "pttStartStopTxPacketGen: packet gen has been started.\n");
            return (FAILURE);   //don't start packet gen again while it is running.
        }

        //start transmitting packets using current parameters:
        //payloadLength per payloadLengthType
        //payload needs to be loaded per payloadContents, per payloadFillByte if applicable
        pMac->ptt.frameGenEnabled = eANI_BOOLEAN_TRUE;

        //start pkt gen
        if (pMac->ptt.phyDbgFrameGen)
        {
            //setup and start the phyDbg module for continuous packets
            if (eHAL_STATUS_SUCCESS !=
                asicPhyDbgStartFrameGen(pMac,
                                           pMac->ptt.frameGenParams.rate,
                                           pMac->ptt.frameGenParams.payloadLength,
                                           pMac->ptt.frameGenParams.payloadContents,
                                           pMac->ptt.frameGenParams.payloadFillByte,
                                           &pMac->ptt.payload[0],
                                           pMac->ptt.frameGenParams.numTestPackets,
                                           pMac->ptt.frameGenParams.interFrameSpace,
                                           pMac->ptt.frameGenParams.pktAutoSeqNum,
                                           pMac->ptt.frameGenParams.pktScramblerSeed,
                                           pMac->ptt.frameGenParams.preamble,
                                           &pMac->ptt.frameGenParams.addr1[0],
                                           &pMac->ptt.frameGenParams.addr2[0],
                                           &pMac->ptt.frameGenParams.addr3[0],
                                           pMac->ptt.frameGenParams.crc
                                       )
               )
           {
               return (FAILURE);
           }
        }
        else
        {
            phyLog(pMac, LOGW, "pttStartStopTxPacketGen: Only PhyDbg is available to transmit packets\n");
            return (FAILURE);
        }
    }
    else
    {
        //stop transmitting
        if ((pMac->ptt.frameGenEnabled) && (pMac->ptt.phyDbgFrameGen))
        {
            //stop the PhyDbg module
            if (asicPhyDbgStopFrameGen(pMac) != eHAL_STATUS_SUCCESS)
            {
               return (FAILURE);
            }

            //now allow receive again
            if (asicAGCReset(pMac) != eHAL_STATUS_SUCCESS)
            {
               return (FAILURE);
            }
        }
        pMac->ptt.frameGenEnabled = FALSE;
    }

    return (SUCCESS);
}



eQWPttStatus pttQueryTxStatus(tpAniSirGlobal pMac, sTxFrameCounters *numFrames, tANI_BOOLEAN *busy)
{
    ePhyDbgTxStatus status;

    if (eHAL_STATUS_SUCCESS == asicPhyDbgQueryStatus(pMac, numFrames, &status))
    {
        if (status == PHYDBG_TX_IDLE)
        {
            *busy = eANI_BOOLEAN_FALSE;
        }
        else
        {
            *busy = eANI_BOOLEAN_TRUE;
        }
        return (SUCCESS);
    }
    else
    {
        return (FAILURE);
    }
}

#endif

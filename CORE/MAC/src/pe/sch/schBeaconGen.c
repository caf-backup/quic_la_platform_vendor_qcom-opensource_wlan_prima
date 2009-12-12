/*
 * Airgo Networks, Inc proprietary. All rights reserved.
 * This file schBeaconGen.cc contains beacon generation related
 * functions
 *
 * Author:      Sandesh Goel
 * Date:        02/25/02
 * History:-
 * Date            Modified by    Modification Information
 * --------------------------------------------------------------------
 *
 */
 
#include "palTypes.h"
#include "wniCfgAp.h"
#include "halDataStruct.h"
#include "aniGlobal.h"
#include "sirMacProtDef.h"

#include "limUtils.h"
#include "limApi.h"
#include "halCommonApi.h"
#include "halMsgApi.h"
#include "cfgApi.h"
#include "pmmApi.h"
#include "schApi.h"

#include "parserApi.h"

#include "schDebug.h"

//
// March 15, 2006
// Temporarily (maybe for all of Alpha-1), assuming TIM = 0
//

#ifdef ANI_PRODUCT_TYPE_AP

static void
specialBeaconProcessing(tpAniSirGlobal pMac, tANI_U32 beaconSize);
#endif

tSirRetStatus schAppendAddnIE(tpAniSirGlobal pMac, tANI_U8 *pFrame, tANI_U32 maxBeaconSize, tANI_U32 *nBytes)
{
    tSirRetStatus status = eSIR_FAILURE;
    tANI_U32 present, len;
    
     if((status = wlan_cfgGetInt(pMac, WNI_CFG_PROBE_RSP_BCN_ADDNIE_FLAG, &present)) != eSIR_SUCCESS)
    {
        limLog(pMac, LOGP, FL("Unable to get WNI_CFG_PROBE_RSP_BCN_ADDNIE_FLAG"));
        return status;
    }
    if(present)
    {
          if((status = wlan_cfgGetStrLen(pMac, WNI_CFG_PROBE_RSP_BCN_ADDNIE_DATA, &len)) != eSIR_SUCCESS)
          {
                limLog(pMac, LOGP, FL("Unable to get WNI_CFG_PROBE_RSP_BCN_ADDNIE_DATA length"));
                return status;
          }
          
          if(len <= WNI_CFG_PROBE_RSP_BCN_ADDNIE_DATA_LEN && len && ((len + *nBytes) <= maxBeaconSize))
          { 
                if((status = wlan_cfgGetStr(pMac, WNI_CFG_PROBE_RSP_BCN_ADDNIE_DATA, pFrame, &len)) == eSIR_SUCCESS)
                    *nBytes = *nBytes + len;
           }
          
       }
     
    return status;
}

// --------------------------------------------------------------------
/**
 * schSetFixedBeaconFields
 *
 * FUNCTION:
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param None
 * @return None
 */

tSirRetStatus schSetFixedBeaconFields(tpAniSirGlobal pMac)
{
    tpAniBeaconStruct pBeacon = (tpAniBeaconStruct)
                                   pMac->sch.schObject.gSchBeaconFrameBegin;
    tpSirMacMgmtHdr mac;
    tANI_U16        offset;
    tANI_U8        *ptr;
    tDot11fBeacon1  bcn1;
    tDot11fBeacon2  bcn2;
    tANI_U32        i, nStatus, nBytes;
    tANI_U32        wpsApEnable=0, tmp;

    PELOG1(schLog(pMac, LOG1, FL("Setting fixed beacon fields\n"));)

    /*
     * First set the fixed fields
     */

    // set the TFP headers

    // set the mac header
    palZeroMemory( pMac->hHdd, ( tANI_U8*) &pBeacon->macHdr, sizeof( tSirMacMgmtHdr ) );
    mac = (tpSirMacMgmtHdr) &pBeacon->macHdr;
    mac->fc.type = SIR_MAC_MGMT_FRAME;
    mac->fc.subType = SIR_MAC_MGMT_BEACON;

    for (i=0; i<6; i++)
        mac->da[i] = 0xff;
    limGetMyMacAddr(pMac, mac->sa);
    limGetBssid(pMac, mac->bssId);

    mac->fc.fromDS = 0;
    mac->fc.toDS = 0;

    /*
     * Now set the beacon body
     */

    palZeroMemory( pMac->hHdd, ( tANI_U8*) &bcn1, sizeof( bcn1 ) );

    // Skip over the timestamp (it'll be updated later).

    bcn1.BeaconInterval.interval = pMac->sch.schObject.gSchBeaconInterval;
    PopulateDot11fCapabilities( pMac, &bcn1.Capabilities );
    PopulateDot11fSSID2( pMac, &bcn1.SSID );
    PopulateDot11fSuppRates( pMac, POPULATE_DOT11F_RATES_OPERATIONAL, &bcn1.SuppRates );
    PopulateDot11fDSParams( pMac, &bcn1.DSParams );
    PopulateDot11fIBSSParams( pMac, &bcn1.IBSSParams );

    offset = sizeof( tAniBeaconStruct );
    ptr    = pMac->sch.schObject.gSchBeaconFrameBegin + offset;

    nStatus = dot11fPackBeacon1( pMac, &bcn1, ptr,
                                 SCH_MAX_BEACON_SIZE - offset,
                                 &nBytes );
    if ( DOT11F_FAILED( nStatus ) )
    {
      schLog( pMac, LOGE, FL("Failed to packed a tDot11fBeacon1 (0x%0"
                             "8x.).\n"), nStatus );
      return eSIR_FAILURE;
    }
    else if ( DOT11F_WARNED( nStatus ) )
    {
      schLog( pMac, LOGE, FL("There were warnings while packing a tDo"
                             "t11fBeacon1 (0x%08x.).\n"), nStatus );
    }
    /*changed  to correct beacon corruption */
    palZeroMemory( pMac->hHdd, ( tANI_U8*) &bcn2, sizeof( bcn2 ) );
    pMac->sch.schObject.gSchBeaconOffsetBegin = offset + ( tANI_U16 )nBytes;
    schLog( pMac, LOG1, FL("Initialized beacon begin, offset %d\n"), offset );

    /*
     * Initialize the 'new' fields at the end of the beacon
     */

    
    PopulateDot11fCountry( pMac, &bcn2.Country );
    if(bcn1.Capabilities.qos)
    {
        PopulateDot11fEDCAParamSet( pMac, &bcn2.EDCAParamSet );
    }

    if(pMac->lim.gLim11hEnable)
    {
      PopulateDot11fPowerConstraints( pMac, &bcn2.PowerConstraints );
      PopulateDot11fTPCReport( pMac, &bcn2.TPCReport );
    }

#ifdef ANI_PRODUCT_TYPE_AP
    if( pMac->lim.gLim11hEnable && (eLIM_QUIET_RUNNING == pMac->lim.gLimSpecMgmt.quietState))
    {
      PopulateDot11fQuiet( pMac, &bcn2.Quiet );
    }

    /* If 11h is enabled, and AP is in the state of changing either the
     * primary channel, or both primary & secondary channel, and the
     * channel switch count is still being decremented, then AP shall
     * populate the 802.11h channel switch IE in its Beacons and Probe
     * Responses.
     */
    if ( (pMac->lim.gLim11hEnable) &&
         (pMac->lim.gLimChannelSwitch.switchCount != 0) &&
         (pMac->lim.gLimSpecMgmt.dot11hChanSwState == eLIM_11H_CHANSW_RUNNING))

    {
      PopulateDot11fChanSwitchAnn( pMac, &bcn2.ChanSwitchAnn );
      PopulateDot11fExtChanSwitchAnn(pMac, &bcn2.ExtChanSwitchAnn);
    }
#endif

    PopulateDot11fERPInfo( pMac, &bcn2.ERPInfo );

    if(pMac->lim.htCapability)
    {
        PopulateDot11fHTCaps( pMac, &bcn2.HTCaps );
        PopulateDot11fHTInfo( pMac, &bcn2.HTInfo );
    }

    PopulateDot11fExtSuppRates( pMac, &bcn2.ExtSuppRates );
    
    PopulateDot11fWPA( pMac, &pMac->lim.gpLimStartBssReq->rsnIE,
                       &bcn2.WPA );
    PopulateDot11fRSN( pMac, &pMac->lim.gpLimStartBssReq->rsnIE,
                       &bcn2.RSN );
    if(pMac->lim.gLimWmeEnabled)
    {
        PopulateDot11fWMM( pMac, &bcn2.WMMInfoAp, &bcn2.WMMParams, &bcn2.WMMCaps );
    }

    if (wlan_cfgGetInt(pMac, (tANI_U16) WNI_CFG_WPS_ENABLE, &tmp) != eSIR_SUCCESS)
        limLog(pMac, LOGP,"Failed to cfg get id %d\n", WNI_CFG_WPS_ENABLE );
    
    wpsApEnable = tmp & WNI_CFG_WPS_ENABLE_AP;
    
    if (wpsApEnable)
    {
        PopulateDot11fWsc(pMac, &bcn2.WscBeacon);
    }

    if (pMac->lim.wscIeInfo.wscEnrollmentState == eLIM_WSC_ENROLL_BEGIN)
    {
        PopulateDot11fWscRegistrarInfo(pMac, &bcn2.WscBeacon);
        pMac->lim.wscIeInfo.wscEnrollmentState = eLIM_WSC_ENROLL_IN_PROGRESS;
    }

    if (pMac->lim.wscIeInfo.wscEnrollmentState == eLIM_WSC_ENROLL_END)
    {
        DePopulateDot11fWscRegistrarInfo(pMac, &bcn2.WscBeacon);
        pMac->lim.wscIeInfo.wscEnrollmentState = eLIM_WSC_ENROLL_NOOP;
    }


    nStatus = dot11fPackBeacon2( pMac, &bcn2,
                                 pMac->sch.schObject.gSchBeaconFrameEnd,
                                 SCH_MAX_BEACON_SIZE, &nBytes );
    if ( DOT11F_FAILED( nStatus ) )
    {
      schLog( pMac, LOGE, FL("Failed to packed a tDot11fBeacon2 (0x%0"
                             "8x.).\n"), nStatus );
      return eSIR_FAILURE;
    }
    else if ( DOT11F_WARNED( nStatus ) )
    {
      schLog( pMac, LOGE, FL("There were warnings while packing a tDo"
                             "t11fBeacon2 (0x%08x.).\n"), nStatus );
    }

    //TODO: Append additional IE here.
    schAppendAddnIE(pMac, pMac->sch.schObject.gSchBeaconFrameEnd + nBytes, SCH_MAX_BEACON_SIZE, &nBytes);

    pMac->sch.schObject.gSchBeaconOffsetEnd = ( tANI_U16 )nBytes;

    schLog( pMac, LOG1, FL("Initialized beacon end, offset %d\n"),
            pMac->sch.schObject.gSchBeaconOffsetEnd );

    pMac->sch.schObject.fBeaconChanged = 1;

    return eSIR_SUCCESS;
}

#ifdef ANI_PRODUCT_TYPE_AP

//----------------------------
/**
  *     @function : schUpdateCfpParam
  *     @brief      : Generate the CFP Parameter Set
  *     
  *     @param    :  pMac - tpAniSirGlobal
  *                       ptr    - Pointer to the BeaconFrame
  *                       pbeaconSize - The beaconSize
  *
  *     @return    :  Return the Updated Ptrlocation
  */
static tANI_U8 *
__schUpdateCfpParam(tpAniSirGlobal pMac, tANI_U8 *ptr, tANI_U32 *pbeaconSize)
{  
    tANI_U32 val;

    *ptr++ = SIR_MAC_CF_PARAM_SET_EID;
    *ptr++ = SIR_MAC_CF_PARAM_SET_EID_MIN;

    wlan_cfgGetInt(pMac, WNI_CFG_CFP_PERIOD, &val);
    if (++pMac->sch.schObject.gSchCFPCount == val)
        pMac->sch.schObject.gSchCFPCount = 0;

    *ptr++ = pMac->sch.schObject.gSchCFPCount;
    *ptr++ = (tANI_U8)val;

    wlan_cfgGetInt(pMac, WNI_CFG_CFP_MAX_DURATION, &val);
    pMac->sch.schObject.gSchCFPMaxDuration = (tANI_U8)val;

    sirStoreU16(ptr, (tANI_U16)val);
    ptr += 2;

    if (pMac->sch.schObject.gSchCFPCount == 0)
        pMac->sch.schObject.gSchCFPDurRemaining = pMac->sch.schObject.gSchCFPMaxDuration;
    else if (pMac->sch.schObject.gSchCFPDurRemaining > pMac->sch.schObject.gSchBeaconInterval)
        pMac->sch.schObject.gSchCFPDurRemaining -= pMac->sch.schObject.gSchBeaconInterval;
    else
        pMac->sch.schObject.gSchCFPDurRemaining = 0;

    sirStoreU16(ptr, pMac->sch.schObject.gSchCFPDurRemaining);
    ptr += 2;

    (*pbeaconSize) += 2 + SIR_MAC_CF_PARAM_SET_EID_MIN;

    return ptr;
}

#endif

// --------------------------------------------------------------------
/**
 * writeBeaconToMemory
 *
 * FUNCTION:
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param None
 * @param size    Size of the beacon to write to memory
 * @param length Length field of the beacon to write to memory
 * @return None
 */

void writeBeaconToMemory(tpAniSirGlobal pMac, tANI_U16 size, tANI_U16 length)
{
    tANI_U16          i;
    tpAniBeaconStruct pBeacon;

    // copy end of beacon only if length > 0
    if (length > 0)
    {
        for (i=0; i < pMac->sch.schObject.gSchBeaconOffsetEnd; i++)
            pMac->sch.schObject.gSchBeaconFrameBegin[size++] = pMac->sch.schObject.gSchBeaconFrameEnd[i];
    }
    
    // Update the beacon length
    pBeacon = (tpAniBeaconStruct) pMac->sch.schObject.gSchBeaconFrameBegin;
    // Do not include the beaconLength indicator itself
    if (length == 0)
    {
        pBeacon->beaconLength = 0;
        // Dont copy entire beacon, Copy length field alone
        size = 4;
    }
    else
        pBeacon->beaconLength = (tANI_U32) size - sizeof( tANI_U32 );

    // write size bytes from gSchBeaconFrameBegin
    PELOG2(schLog(pMac, LOG2, FL("Beacon size - %d bytes\n"), size);)
    PELOG2(sirDumpBuf(pMac, SIR_SCH_MODULE_ID, LOG2, pMac->sch.schObject.gSchBeaconFrameBegin, size);)

    if (! pMac->sch.schObject.fBeaconChanged)
        return;

    pMac->sch.gSchGenBeacon = 1;
    if (pMac->sch.gSchGenBeacon)
    {
        pMac->sch.gSchBeaconsSent++;

        //
        // Copy beacon data to SoftMAC shared memory...
        // Do this by sending a message to HAL
        //

        size = (size + 3) & (~3);
        if( eSIR_SUCCESS != schSendBeaconReq( pMac, pMac->sch.schObject.gSchBeaconFrameBegin, size ))
            PELOGE(schLog(pMac, LOGE, FL("schSendBeaconReq() returned an error (zsize %d)\n"), size);)
        else
        {
            pMac->sch.gSchBeaconsWritten++;
        }
    }
    pMac->sch.schObject.fBeaconChanged = 0;
}

// --------------------------------------------------------------------
/**
 * @function: SchProcessPreBeaconInd
 *
 * @brief : Process the PreBeacon Indication from the Lim
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param : pMac - tpAniSirGlobal
 *
 * @return None
 */

void
schProcessPreBeaconInd(tpAniSirGlobal pMac)
{
    tANI_U32 beaconSize = pMac->sch.schObject.gSchBeaconOffsetBegin;

    // If SME is not in normal mode, no need to generate beacon
    if (pMac->lim.gLimSmeState != eLIM_SME_NORMAL_STATE)
        return;


    switch(pMac->lim.gLimSystemRole){

    case eLIM_STA_IN_IBSS_ROLE:
        // generate IBSS parameter set
        writeBeaconToMemory(pMac, (tANI_U16) beaconSize, (tANI_U16)beaconSize);
        break;

#ifdef ANI_PRODUCT_TYPE_AP
    case eLIM_AP_ROLE: {
        tANI_U8 *ptr = &pMac->sch.schObject.gSchBeaconFrameBegin[pMac->sch.schObject.gSchBeaconOffsetBegin];
        tANI_U16 timLength = 0;

        if (pMac->sch.schObject.gSchCFPEnabled)
            ptr = __schUpdateCfpParam( pMac, ptr, &beaconSize);

        // generate TIM
        pmmGenerateTIM(pMac, &ptr, &timLength);
        beaconSize += 2 + timLength;

        /**
        * Safe to call this each time.
        * Based on the requirement for updating the
        * fixed beacon fields, this routine will
        * appropriately update the fixed fields
        */
        specialBeaconProcessing(pMac, beaconSize);
        writeBeaconToMemory(pMac, beaconSize, beaconSize);
        
        pmmHandleTimBasedDisassociation( pMac);
    }break;
#endif

    default:
        limLog(pMac, LOGE, FL("Error-PE has Receive PreBeconGenIndication when System is in %d role"),
               pMac->lim.gLimSystemRole);
    }
}

/**-------------------------------------------------------------
 \fn       specialBeaconProcessing
 \brief   To add/update channel switch IE/ Quiet IE in beacons.
          And also to resume transmission and measurement after
          switching the channel.
      
 \param   pMac
 \param   beaconSize  Size of the beacon
 \return   NONE
 --------------------------------------------------------------*/
#ifdef ANI_PRODUCT_TYPE_AP

static void
specialBeaconProcessing( tpAniSirGlobal pMac, tANI_U32 beaconSize)
{
    tANI_BOOLEAN fBeaconChanged = eANI_BOOLEAN_FALSE;

    fBeaconChanged = limUpdateQuietIEInBeacons( pMac );

    if((pMac->lim.wscIeInfo.wscEnrollmentState == eLIM_WSC_ENROLL_BEGIN) ||
       (pMac->lim.wscIeInfo.wscEnrollmentState == eLIM_WSC_ENROLL_END))
    {
        fBeaconChanged = eANI_BOOLEAN_TRUE;
    }


    /*******************************
     * Processing Channel Switch IE
     *******************************/
    if (pMac->lim.gLimSpecMgmt.dot11hChanSwState == eLIM_11H_CHANSW_RUNNING)
    {
        fBeaconChanged = eANI_BOOLEAN_TRUE;

#if 0
        // If the station doesn't support 11h or have link monitoring enabled,
        // AP has to send disassoc frame to indicate station before going
        // to new channel. Otherwise station wont connect to AP in new channel.
        if (pMac->lim.gLimChannelSwitch.switchCount == 1)
        {
            if((pMac->lim.gLimChannelSwitch.state
                == eLIM_CHANNEL_SWITCH_PRIMARY_ONLY) ||
                (pMac->lim.gLimChannelSwitch.state
                == eLIM_CHANNEL_SWITCH_PRIMARY_AND_SECONDARY))
            {
                tSirMacAddr   bcAddr = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

                limSendDisassocMgmtFrame(pMac,
                                   eSIR_MAC_DISASSOC_LEAVING_BSS_REASON,
                                   bcAddr);
            }
        }
#endif
        if (pMac->lim.gLimChannelSwitch.switchCount == 0)
        {
            
            /* length is set to 0, so that no beacon is transmitted without channel switch IE
                       * before switching  to new channel */
            pMac->sch.schObject.fBeaconChanged = 1;
            writeBeaconToMemory(pMac, beaconSize, 0);
            schSetFixedBeaconFields(pMac);

           PELOG3(limLog(pMac, LOG3, FL("Channel switch state = %d\n"), pMac->lim.gLimChannelSwitch.state);)
            switch(pMac->lim.gLimChannelSwitch.state)
            {
                case eLIM_CHANNEL_SWITCH_PRIMARY_ONLY:
                    limSwitchPrimaryChannel(pMac, pMac->lim.gLimChannelSwitch.primaryChannel);
                    break;
                case eLIM_CHANNEL_SWITCH_SECONDARY_ONLY:
                    limSwitchPrimarySecondaryChannel(pMac,
                                             pMac->lim.gLimCurrentChannelId,
                                             pMac->lim.gLimChannelSwitch.secondarySubBand);
                    break;
                case eLIM_CHANNEL_SWITCH_PRIMARY_AND_SECONDARY:
                    limSwitchPrimarySecondaryChannel(pMac,
                                             pMac->lim.gLimChannelSwitch.primaryChannel,
                                             pMac->lim.gLimChannelSwitch.secondarySubBand);
                    break;
                case eLIM_CHANNEL_SWITCH_IDLE:
                    PELOGE(schLog(pMac, LOGE, FL("incorrect state - CHANNEL_SWITCH_IDLE\n"));)
                    break;

                default:
                    break;
            }
            pMac->lim.gLimChannelSwitch.state = eLIM_CHANNEL_SWITCH_IDLE;

            limSendSmeRsp(pMac, eWNI_SME_SWITCH_CHL_RSP, eSIR_SME_SUCCESS);

            limFrameTransmissionControl(pMac, eLIM_TX_BSS_BUT_BEACON, eLIM_RESUME_TX);
            /* Flag to indicate 11h channel switch is done. */
            pMac->lim.gLimSpecMgmt.dot11hChanSwState = eLIM_11H_CHANSW_INIT;
            pMac->lim.gLimSpecMgmt.quietState = eLIM_QUIET_INIT;
            LIM_SET_RADAR_DETECTED(pMac, eANI_BOOLEAN_FALSE);
            
            if (pMac->lim.gpLimMeasReq)
                limReEnableLearnMode(pMac);

            return;
        }
    }

    if (fBeaconChanged)
    {
        schSetFixedBeaconFields(pMac);

        if (pMac->lim.gLimChannelSwitch.switchCount > 0)
            pMac->lim.gLimChannelSwitch.switchCount--;
    }
}
#endif



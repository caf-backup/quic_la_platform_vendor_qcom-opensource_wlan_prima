/*============================================================================
limLogDump.c

Implements the dump commands specific to the lim module. 

Copyright (c) 2007 QUALCOMM Incorporated.
All Rights Reserved.
Qualcomm Confidential and Proprietary
 ============================================================================*/

#include "limApi.h"

#if defined(ANI_LOGDUMP)


#include "limUtils.h"
#include "limSecurityUtils.h"
#include "schApi.h"
#include "limSerDesUtils.h"
#include "limAssocUtils.h"
#include "limSendMessages.h"
#include "logDump.h"
#include "limTrace.h"

extern eHalStatus csrSendSmeReassocReqMsg( tpAniSirGlobal pMac, tSirBssDescription *pBssDescription, 
													 tCsrRoamProfile *pProfile );

#ifdef WLAN_DEBUG  
static char *getRole( tLimSystemRole role )
{
  switch (role)
  {
    case eLIM_AP_ROLE:
        return "AP";
    case eLIM_STA_IN_IBSS_ROLE:
        return "IBSS";
    case eLIM_STA_ROLE:
        return "STA/BP";
    default:
        return "UNKNOWN";
  }
}
#endif

#if (defined(ANI_PRODUCT_TYPE_AP) || defined(ANI_PRODUCT_TYPE_AP_SDK))
static char *
dumpMacAddr(tpAniSirGlobal pMac, char *p, tANI_U8 *addr)
{
    p += log_sprintf( pMac,p, "%2x:%2x:%2x:%2x:%2x:%2x",
                    addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
    return p;
}
#endif


char *dumpLim( tpAniSirGlobal pMac, char *p )
{
#ifdef WLAN_DEBUG    

  tANI_U16 i, j;


  p += log_sprintf( pMac,p, "\n ----- LIM Debug Information ----- \n");
  p += log_sprintf( pMac,p, "LIM Role  = (%d) %s\n",
                  pMac->lim.gLimSystemRole, getRole(pMac->lim.gLimSystemRole));
  p += log_sprintf( pMac,p, "SME State = (%d) %s",
                  pMac->lim.gLimSmeState, limSmeStateStr(pMac->lim.gLimSmeState));
  p += log_sprintf( pMac,p, "MLM State = (%d) %s",
                  pMac->lim.gLimMlmState, limMlmStateStr(pMac->lim.gLimMlmState));

  p += log_sprintf( pMac,p, "CHANNEL BONDING Mode (%1d) and State (X|X|X|AU|CS|U/D|O|A) (0x%1x)\n",
                  pMac->lim.gCbMode, pMac->lim.gCbState);
  p += log_sprintf( pMac,p, "802.11n HT Capability: %s\n",
                  (pMac->lim.htCapability == 1) ? "Enabled" : "Disabled");
  p += log_sprintf( pMac,p, "gLimProcessDefdMsgs: %s\n",
                  (pMac->lim.gLimProcessDefdMsgs == 1) ? "Enabled" : "Disabled");

  if (pMac->lim.gLimSystemRole == eLIM_STA_ROLE)
  {
      p += log_sprintf( pMac,p, "AID = %X\t\t\n", pMac->lim.gLimAID);
      p += log_sprintf( pMac,p, "SSID mismatch in Beacon Count              = %d\n",
                      pMac->lim.gLimBcnSSIDMismatchCnt);
      p += log_sprintf( pMac,p, "Number of link establishments               = %d\n",
                      pMac->lim.gLimNumLinkEsts);
  }
  else if (pMac->lim.gLimSystemRole == eLIM_AP_ROLE)
  {
      p += log_sprintf( pMac,p, "Num of STAs associated                     = %d\n",
                      pMac->lim.gLimNumOfCurrentSTAs);

      p += log_sprintf( pMac,p, "Num of Pre-auth contexts                   = %d\n",
                      pMac->lim.gLimNumPreAuthContexts);

      p += log_sprintf( pMac,p, "Num of AssocReq dropped in invalid State   = %d\n",
                      pMac->lim.gLimNumAssocReqDropInvldState);

      p += log_sprintf( pMac,p, "Num of ReassocReq dropped in invalid State = %d\n",
                      pMac->lim.gLimNumReassocReqDropInvldState);

      p += log_sprintf( pMac,p, "Num of Hash Miss Event ignored             = %d\n",
                      pMac->lim.gLimNumHashMissIgnored);




  }

  p += log_sprintf( pMac,p, "Num of RxCleanup Count                     = %d\n",
                  pMac->lim.gLimNumRxCleanup);
  p += log_sprintf( pMac,p, "Unexpected Beacon Count                    = %d\n",
                  pMac->lim.gLimUnexpBcnCnt);
  p += log_sprintf( pMac,p, "Number of Re/Assoc rejects of 11b STAs     = %d\n",
                  pMac->lim.gLim11bStaAssocRejectCount);
  p += log_sprintf( pMac,p, "No. of HeartBeat Failures in LinkEst State = %d\n",
                  pMac->lim.gLimHBfailureCntInLinkEstState);
  p += log_sprintf( pMac,p, "No. of Probe Failures after HB failed      = %d\n",
                  pMac->lim.gLimProbeFailureAfterHBfailedCnt);
  p += log_sprintf( pMac,p, "No. of HeartBeat Failures in Other States  = %d\n",
                  pMac->lim.gLimHBfailureCntInOtherStates);
  p += log_sprintf( pMac,p, "No. of Beacons Rxed During HB Interval     = %d\n",
                  pMac->lim.gLimRxedBeaconCntDuringHB);
  p += log_sprintf( pMac,p, "Self Operating Mode                              = %s\n", limDot11ModeStr(pMac, (tANI_U8)pMac->lim.gLimDot11Mode));





  p += log_sprintf( pMac,p, "\n");

  if (pMac->lim.gLimSystemRole == eLIM_AP_ROLE)
      i = 2;
  else
      i = 1;
  for (; i< pMac->lim.maxStation; i++)
  {
      tpDphHashNode pSta = dphGetHashEntry(pMac, (unsigned short)i);
      if (pSta && pSta->added)
      {
          p += log_sprintf( pMac,p, "\nSTA AID: %d  STA ID: %d Valid: %d AuthType: %d MLM State: %s",
                          i, pSta->staIndex, pSta->valid,
                          pSta->mlmStaContext.authType,
                          limMlmStateStr(pSta->mlmStaContext.mlmState));

          p += log_sprintf( pMac,p, "\tAID:%-2d  OpRateMode:%s  ShPrmbl:%d  HT:%d  GF:%d  TxChWidth:%d  MimoPS:%d  LsigProt:%d\n",
                            pSta->assocId, limStaOpRateModeStr(pSta->supportedRates.opRateMode),
                            pSta->shortPreambleEnabled, pSta->mlmStaContext.htCapability,
                            pSta->htGreenfield, pSta->htSupportedChannelWidthSet,
                            pSta->htMIMOPSState, pSta->htLsigTXOPProtection);

          p += log_sprintf( pMac,p, "\tAMPDU [MaxSz(Factor):%d, Dens: %d]  AMSDU-MaxLen: %d\n",
                          pSta->htMaxRxAMpduFactor, pSta->htAMpduDensity,pSta->htMaxAmsduLength);
          p += log_sprintf( pMac,p, "\tDSSCCkMode40Mhz: %d, SGI20: %d, SGI40: %d\n",
                          pSta->htDsssCckRate40MHzSupport, pSta->htShortGI20Mhz,
                          pSta->htShortGI40Mhz);

          p += log_sprintf( pMac,p, "\t11b Rates: ");
          for(j=0; j<SIR_NUM_11B_RATES; j++)
              if(pSta->supportedRates.llbRates[j] > 0)
                  p += log_sprintf( pMac,p, "%d ", pSta->supportedRates.llbRates[j]);

          p += log_sprintf( pMac,p, "\n\t11a Rates: ");
          for(j=0; j<SIR_NUM_11A_RATES; j++)
              if(pSta->supportedRates.llaRates[j] > 0)
                  p += log_sprintf( pMac,p, "%d ", pSta->supportedRates.llaRates[j]);

          p += log_sprintf( pMac,p, "\n\tPolaris Rates: ");
          for(j=0; j<SIR_NUM_POLARIS_RATES; j++)
              if(pSta->supportedRates.aniLegacyRates[j] > 0)
                  p += log_sprintf( pMac,p, "%d ", pSta->supportedRates.aniLegacyRates[j]);

          p += log_sprintf( pMac,p, "\n\tTitan and Taurus Proprietary Rate Bitmap: %08x\n",
                          pSta->supportedRates.aniEnhancedRateBitmap);
          p += log_sprintf( pMac,p, "\tMCS Rate Set Bitmap: ");
          for(j=0; j<SIR_MAC_MAX_SUPPORTED_MCS_SET; j++)
              p += log_sprintf( pMac,p, "%x ", pSta->supportedRates.supportedMCSSet[j]);

      }
  }
  p += log_sprintf( pMac,p, "\nProbe response disable          = %d\n",
                  pMac->lim.gLimProbeRespDisableFlag);

#if (WNI_POLARIS_FW_PRODUCT == WLAN_STA)
  p += log_sprintf( pMac,p, "Scan mode enable                = %d\n",
                  pMac->sys.gSysEnableScanMode);
  p += log_sprintf( pMac,p, "BackgroundScanDisable           = %d\n",
                  pMac->lim.gLimBackgroundScanDisable);
  p += log_sprintf( pMac,p, "ForceBackgroundScanDisable      = %d\n",
                  pMac->lim.gLimForceBackgroundScanDisable);
  p += log_sprintf( pMac,p, "LinkMonitor mode enable         = %d\n",
                  pMac->sys.gSysEnableLinkMonitorMode);
  p += log_sprintf( pMac,p, "Qos Capable                     = %d\n",
                  SIR_MAC_GET_QOS(pMac->lim.gLimCurrentBssCaps));
  p += log_sprintf( pMac,p, "Wme Capable                     = %d\n",
                  LIM_BSS_CAPS_GET(WME, pMac->lim.gLimCurrentBssQosCaps));
  p += log_sprintf( pMac,p, "Wsm Capable                     = %d\n",
                  LIM_BSS_CAPS_GET(WSM, pMac->lim.gLimCurrentBssQosCaps));
  if (pMac->lim.gLimSystemRole == eLIM_STA_IN_IBSS_ROLE)
  {
      p += log_sprintf( pMac,p, "Number of peers in IBSS         = %d\n",
                      pMac->lim.gLimNumIbssPeers);
      if (pMac->lim.gLimNumIbssPeers)
      {
          tLimIbssPeerNode *pTemp;
          pTemp = pMac->lim.gLimIbssPeerList;
          p += log_sprintf( pMac,p, "MAC-Addr           Ani Edca WmeInfo HT  Caps  #S,#E(Rates)\n");
          while (pTemp != NULL)
          {
              p += log_sprintf( pMac,p, "%02X:%02X:%02X:%02X:%02X:%02X ",
                              pTemp->peerMacAddr[0],
                              pTemp->peerMacAddr[1],
                              pTemp->peerMacAddr[2],
                              pTemp->peerMacAddr[3],
                              pTemp->peerMacAddr[4],
                              pTemp->peerMacAddr[5]);
              p += log_sprintf( pMac,p, " %d   %d,%d        %d  %d  %04X  %d,%d\n",
                              pTemp->aniIndicator,
                              pTemp->edcaPresent, pTemp->wmeEdcaPresent,
                              pTemp->wmeInfoPresent,
                              pTemp->htCapable,
                              pTemp->capabilityInfo,
                              pTemp->supportedRates.numRates,
                              pTemp->extendedRates.numRates);
              pTemp = pTemp->next;
          }
      }
  }
#else
  p += log_sprintf( pMac,p, "Measurements enabled            = %d\n",
                  pMac->sys.gSysEnableLearnMode);
  p += log_sprintf( pMac,p, "Scan Mode for Learn Mode enable = %d\n",
                  pMac->lim.gLimUseScanModeForLearnMode);
#endif
  p += log_sprintf( pMac,p, "System Scan/Learn Mode bit      = %d\n",
                  pMac->lim.gLimSystemInScanLearnMode);
  p += log_sprintf( pMac,p, "Scan override                   = %d\n",
                  pMac->lim.gLimScanOverride);
  p += log_sprintf( pMac,p, "CB State protection             = %d\n",
                  pMac->lim.gLimCBStateProtection);
  p += log_sprintf( pMac,p, "Count of Titan STA's            = %d\n",
                  pMac->lim.gLimTitanStaCount);

  //current BSS capability
  p += log_sprintf( pMac,p, "**********Current BSS Capability********\n");
  p += log_sprintf( pMac,p, "Ess = %d, ", SIR_MAC_GET_ESS(pMac->lim.gLimCurrentBssCaps));
  p += log_sprintf( pMac,p, "Privacy = %d, ", SIR_MAC_GET_PRIVACY(pMac->lim.gLimCurrentBssCaps));
  p += log_sprintf( pMac,p, "Short Preamble = %d, ", SIR_MAC_GET_SHORT_PREAMBLE(pMac->lim.gLimCurrentBssCaps));
  p += log_sprintf( pMac,p, "Short Slot = %d, ", SIR_MAC_GET_SHORT_SLOT_TIME(pMac->lim.gLimCurrentBssCaps));
  p += log_sprintf( pMac,p, "Qos = %d\n", SIR_MAC_GET_QOS(pMac->lim.gLimCurrentBssCaps));

  //Protection related information
  p += log_sprintf( pMac,p, "*****Protection related information******\n");
  p += log_sprintf( pMac,p, "Protection %s\n", pMac->lim.gLimProtectionControl ? "Enabled" : "Disabled");

  p += log_sprintf( pMac,p, "OBSS MODE = %d\n", pMac->lim.gHTObssMode);
#if (defined(ANI_PRODUCT_TYPE_AP) || defined(ANI_PRODUCT_TYPE_AP_SDK))

    p += log_sprintf( pMac,p, "\nNumber of active OLBC detected = %d\n",
                    pMac->lim.gLimOlbcParams.numSta);
    if (pMac->lim.gLimOlbcParams.protectionEnabled)
        p += log_sprintf( pMac,p, "Protection due to OLBC is ON\n");
    else
        p += log_sprintf( pMac,p, "Protection due to OLBC is OFF\n");

    p += log_sprintf( pMac,p, "Content of OLBC cache: \n");
    for (i=0; i<LIM_PROT_STA_OVERLAP_CACHE_SIZE; i++)
    {
        if (pMac->lim.protStaOverlapCache[i].active)
        {
            p = dumpMacAddr(pMac, p, pMac->lim.protStaOverlapCache[i].addr);
            p += log_sprintf( pMac,p, "\n");
        }
    }

    p += log_sprintf( pMac,p, "Content of Protection cache: \n");
    for (i=0; i<LIM_PROT_STA_CACHE_SIZE; i++)
    {
        if (pMac->lim.protStaCache[i].active)
        {
            p = dumpMacAddr(pMac, p, pMac->lim.protStaCache[i].addr);
            if(pMac->lim.protStaCache[i].protStaCacheType == eLIM_PROT_STA_CACHE_TYPE_HT20)
                p += log_sprintf( pMac,p, " Type: HT20\n");
            else if(pMac->lim.protStaCache[i].protStaCacheType == eLIM_PROT_STA_CACHE_TYPE_llB)
                p += log_sprintf( pMac,p, " Type: 11B\n");
             else if(pMac->lim.protStaCache[i].protStaCacheType == eLIM_PROT_STA_CACHE_TYPE_llG)
                p += log_sprintf( pMac,p, " Type: 11G\n");

            p += log_sprintf( pMac,p, "\n");
        }
    }
    p += log_sprintf( pMac,p, "Count of different type sta associated\n");
    p += log_sprintf( pMac,p, "11B         = %d, Protection: %d\n", pMac->lim.gLim11bParams.numSta, pMac->lim.gLim11bParams.protectionEnabled);
    p += log_sprintf( pMac,p, "11G         = %d, Protection: %d\n", pMac->lim.gLim11gParams.numSta, pMac->lim.gLim11gParams.protectionEnabled);
    p += log_sprintf( pMac,p, "nonGF      = %d, Protection: %d\n", pMac->lim.gLimNonGfParams.numSta, pMac->lim.gLimNonGfParams.protectionEnabled);
    p += log_sprintf( pMac,p, "!LsigTxop = %d, Protection: %d\n", pMac->lim.gLimLsigTxopParams.numSta, pMac->lim.gLimLsigTxopParams.protectionEnabled);
    p += log_sprintf( pMac,p, "ht20         = %d Protection:  %d\n", pMac->lim.gLimHt20Params.numSta, pMac->lim.gLimHt20Params.protectionEnabled);

    p += log_sprintf( pMac,p, "\nNumber of STA do not support short preamble = %d\n",
                  pMac->lim.gLimNoShortParams.numNonShortPreambleSta);
    for (i=0; i<LIM_PROT_STA_CACHE_SIZE; i++)
    {
        if (pMac->lim.gLimNoShortParams.staNoShortCache[i].active)
        {
            p = dumpMacAddr(pMac, p, pMac->lim.gLimNoShortParams.staNoShortCache[i].addr);
            p += log_sprintf( pMac,p, "\n");
        }
    }

    p += log_sprintf( pMac,p, "\nNumber of STA do not support short slot time = %d\n",
                    pMac->lim.gLimNoShortSlotParams.numNonShortSlotSta);
    for (i=0; i<LIM_PROT_STA_CACHE_SIZE; i++)
    {
        if (pMac->lim.gLimNoShortSlotParams.staNoShortSlotCache[i].active)
        {
            p = dumpMacAddr(pMac, p, pMac->lim.gLimNoShortSlotParams.staNoShortSlotCache[i].addr);
            p += log_sprintf( pMac,p, "\n");
        }
    }


#endif
    p += log_sprintf( pMac, p, "HT operating Mode = %d, llbCoexist = %d, llgCoexist = %d, ht20Coexist = %d, nonGfPresent = %d, RifsMode = %d, lsigTxop = %d\n",
                      pMac->lim.gHTOperMode, pMac->lim.llbCoexist, pMac->lim.llgCoexist,
                      pMac->lim.ht20MhzCoexist, pMac->lim.gHTNonGFDevicesPresent,
                      pMac->lim.gHTRifsMode, pMac->lim.gHTLSigTXOPFullSupport);

    p += log_sprintf(pMac, p, "2nd Channel offset = %d\n",
                  pMac->lim.gHTSecondaryChannelOffset);
#endif    
    return p;
}

/*******************************************
 * FUNCTION: triggerBeaconGen()
 *
 * This logdump sends SIR_SCH_BEACON_GEN_IND to SCH.
 * SCH then proceeds to generate a beacon template
 * and copy it to the Host/SoftMAC shared memory
 *
 * TODO - This routine can safely be deleted once
 * beacon generation is working
 ******************************************/
char *triggerBeaconGen( tpAniSirGlobal pMac, char *p )
{
    tSirMsgQ mesg = { (tANI_U16) SIR_LIM_BEACON_GEN_IND, (tANI_U16) 0, (tANI_U32) 0 };
    
    pMac->lim.gLimSmeState = eLIM_SME_NORMAL_STATE;
    MTRACE(macTrace(pMac, TRACE_CODE_SME_STATE, 0, pMac->lim.gLimSmeState));
    pMac->lim.gLimSystemRole = eLIM_AP_ROLE;
    
    p += log_sprintf( pMac, p,
          "Posted SIR_LIM_BEACON_GEN_IND with result = %s\n",
          (eSIR_SUCCESS == limPostMsgApi( pMac, &mesg ))?
            "Success": "Failure" );
    
    return p;
}


/*******************************************
 * FUNCTION: testLimSendProbeRsp()
 *
 * This logdump sends SIR_MAC_MGMT_PROBE_RSP
 *
 * TODO - This routine can safely be deleted once
 * the MGMT frame transmission is working
 ******************************************/
char *testLimSendProbeRsp( tpAniSirGlobal pMac, char *p )
{
    tSirMacAddr peerMacAddr = { 0, 1, 2, 3, 4, 5 };
    tAniSSID ssId;
    tANI_U32 len = SIR_MAC_MAX_SSID_LENGTH;


    if( eSIR_SUCCESS != wlan_cfgGetStr( pMac,
        WNI_CFG_SSID,
        (tANI_U8 *) &ssId.ssId,
        (tANI_U32 *) &len ))
    {
        // Could not get SSID from CFG. Log error.
        p += log_sprintf( pMac, p, "Unable to retrieve SSID\n" );
        return p;
    }
    else
        ssId.length = (tANI_U8) len;

    p += log_sprintf( pMac, p, "Calling limSendProbeRspMgmtFrame...\n" );
    limSendProbeRspMgmtFrame( pMac, peerMacAddr, &ssId, -1, 1 );

    return p;
}


static char *sendSmeScanReq(tpAniSirGlobal pMac, char *p)
{
    tSirMsgQ         msg;
    tSirSmeScanReq   scanReq, *pScanReq;

    p += log_sprintf( pMac,p, "sendSmeScanReq: Preparing eWNI_SME_SCAN_REQ message\n");

    pScanReq = (tSirSmeScanReq *) &scanReq;

    if (palAllocateMemory(pMac->hHdd, (void **)&pScanReq, sizeof(tSirSmeScanReq)) != eHAL_STATUS_SUCCESS)
    {
        p += log_sprintf( pMac,p,"sendSmeScanReq: palAllocateMemory() failed \n");
        return p;
    }

    pScanReq->messageType = eWNI_SME_SCAN_REQ;
    pScanReq->minChannelTime = 30;
    pScanReq->maxChannelTime = 130;
    pScanReq->bssType = eSIR_INFRASTRUCTURE_MODE;
    limGetBssid(pMac, pScanReq->bssId);
    palCopyMemory(pMac->hHdd, (void *) &pScanReq->ssId.ssId, (void *)"Ivan", 4);
    pScanReq->ssId.length = 4;
    pScanReq->scanType = eSIR_ACTIVE_SCAN;
    pScanReq->returnAfterFirstMatch = 0;
    pScanReq->returnUniqueResults = 0;
    pScanReq->returnFreshResults = SIR_BG_SCAN_PURGE_RESUTLS|SIR_BG_SCAN_RETURN_FRESH_RESULTS;
    pScanReq->channelList.numChannels = 1;
    pScanReq->channelList.channelNumber[0] = 6;

    msg.type = eWNI_SME_SCAN_REQ;
    msg.bodyptr = pScanReq;
    msg.bodyval = 0;
    p += log_sprintf( pMac,p, "sendSmeScanReq: limPostMsgApi(eWNI_SME_SCAN_REQ) \n");
    limPostMsgApi(pMac, &msg);

    return p;
}


void
limSetEdcaBcastACMFlag(tpAniSirGlobal pMac, tANI_U32 ac, tANI_U32 acmFlag)
{
    pMac->sch.schObject.gSchEdcaParamsBC[ac].aci.acm = (tANI_U8)acmFlag;
    pMac->sch.schObject.gSchEdcaParamSetCount++;
    schSetFixedBeaconFields(pMac);
}

static char *
limDumpEdcaParams(tpAniSirGlobal pMac, char *p)
{
    tANI_U8 i = 0;
    p += log_sprintf( pMac,p, "EDCA parameter set count = %d\n",  pMac->sch.schObject.gSchEdcaParamSetCount);
    p += log_sprintf( pMac,p, "Broadcast parameters\n");
    p += log_sprintf( pMac,p, "AC\tACI\tACM\tAIFSN\tCWMax\tCWMin\tTxopLimit\t\n");
    for(i = 0; i < MAX_NUM_AC; i++)
    {
        //right now I am just interested in ACM bit. this can be extended for all other EDCA paramters.
        p += log_sprintf( pMac,p, "%d\t%d\t%d\t%d\t%d\t%d\t%d\n",  i,
          pMac->sch.schObject.gSchEdcaParamsBC[i].aci.aci, pMac->sch.schObject.gSchEdcaParamsBC[i].aci.acm,
          pMac->sch.schObject.gSchEdcaParamsBC[i].aci.aifsn, pMac->sch.schObject.gSchEdcaParamsBC[i].cw.max,
          pMac->sch.schObject.gSchEdcaParamsBC[i].cw.min, pMac->sch.schObject.gSchEdcaParamsBC[i].txoplimit);
    }

    p += log_sprintf( pMac,p, "\nLocal parameters\n");
    p += log_sprintf( pMac,p, "AC\tACI\tACM\tAIFSN\tCWMax\tCWMin\tTxopLimit\t\n");
    for(i = 0; i < MAX_NUM_AC; i++)
    {
        //right now I am just interested in ACM bit. this can be extended for all other EDCA paramters.
        p += log_sprintf( pMac,p, "%d\t%d\t%d\t%d\t%d\t%d\t%d\n",  i,
              pMac->sch.schObject.gSchEdcaParams[i].aci.aci, pMac->sch.schObject.gSchEdcaParams[i].aci.acm,
              pMac->sch.schObject.gSchEdcaParams[i].aci.aifsn, pMac->sch.schObject.gSchEdcaParams[i].cw.max,
              pMac->sch.schObject.gSchEdcaParams[i].cw.min, pMac->sch.schObject.gSchEdcaParams[i].txoplimit);
    }

    return p;
}


static char* limDumpTspecEntry(tpAniSirGlobal pMac, char *p, tANI_U32 tspecEntryNo)
{
    tpLimTspecInfo pTspecList;
    if(tspecEntryNo >= LIM_NUM_TSPEC_MAX)
    {
        p += log_sprintf( pMac,p, "Tspec Entry no. %d is out of allowed range(0 .. %d)\n",
                        tspecEntryNo,  (LIM_NUM_TSPEC_MAX - 1));
        return p;
    }
    pTspecList = &pMac->lim.tspecInfo[tspecEntryNo];
    if (pTspecList->inuse)
        p += log_sprintf( pMac,p, "Entry %d is VALID\n", tspecEntryNo);
    else
    {
        p += log_sprintf( pMac,p, "Entry %d is UNUSED\n", tspecEntryNo);
        return p;
    }
    p += log_sprintf( pMac,p, "\tSta %0x:%0x:%0x:%0x:%0x:%0x, AID %d, Index %d\n",
                            pTspecList->staAddr[0], pTspecList->staAddr[1],
                            pTspecList->staAddr[2], pTspecList->staAddr[3],
                            pTspecList->staAddr[4], pTspecList->staAddr[5],
                            pTspecList->assocId,  pTspecList->idx);
    p += log_sprintf( pMac,p, "\tType %d, Length %d, ackPolicy %d, userPrio %d, accessPolicy = %d, Dir %d, tsid %d\n",
                            pTspecList->tspec.type, pTspecList->tspec.length,
                            pTspecList->tspec.tsinfo.traffic.ackPolicy, pTspecList->tspec.tsinfo.traffic.userPrio,
                            pTspecList->tspec.tsinfo.traffic.accessPolicy, pTspecList->tspec.tsinfo.traffic.direction,
                            pTspecList->tspec.tsinfo.traffic.tsid);
    p += log_sprintf( pMac,p, "\tPsb %d, Agg %d, TrafficType %d, schedule %d; msduSz: nom %d, max %d\n",
                            pTspecList->tspec.tsinfo.traffic.psb, pTspecList->tspec.tsinfo.traffic.aggregation,
                            pTspecList->tspec.tsinfo.traffic.trafficType, pTspecList->tspec.tsinfo.schedule.schedule,
                            pTspecList->tspec.nomMsduSz,  pTspecList->tspec.maxMsduSz);
    p += log_sprintf( pMac,p, "\tSvcInt: Min %d, Max %d; dataRate: Min %d, mean %d, peak %d\n",
                            pTspecList->tspec.minSvcInterval,  pTspecList->tspec.maxSvcInterval,
                            pTspecList->tspec.minDataRate,  pTspecList->tspec.meanDataRate,
                            pTspecList->tspec.peakDataRate);
    p += log_sprintf( pMac,p, "\tmaxBurstSz %d, delayBound %d, minPhyRate %d, surplusBw %d, mediumTime %d\n",
                            pTspecList->tspec.maxBurstSz, pTspecList->tspec.delayBound,
                            pTspecList->tspec.minPhyRate, pTspecList->tspec.surplusBw,
                            pTspecList->tspec.mediumTime);

    return p;
}

static char* dumpTspecTableSummary(tpAniSirGlobal pMac, tpLimTspecInfo pTspecList, char *p, int ctspec)
{
  p += log_sprintf( pMac, p, "%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",
            ctspec, pTspecList->idx, pTspecList->assocId,
            pTspecList->tspec.tsinfo.traffic.ackPolicy, pTspecList->tspec.tsinfo.traffic.userPrio,
            pTspecList->tspec.tsinfo.traffic.psb, pTspecList->tspec.tsinfo.traffic.aggregation,
            pTspecList->tspec.tsinfo.traffic.accessPolicy, pTspecList->tspec.tsinfo.traffic.direction,
            pTspecList->tspec.tsinfo.traffic.tsid, pTspecList->tspec.tsinfo.traffic.trafficType);

  return p;
}

static char* limDumpDphTableSummary(tpAniSirGlobal pMac, char *p)
{
  tANI_U8 i;
  p += log_sprintf( pMac,p, "DPH Table dump\n");
  p += log_sprintf( pMac,p, "aid staId bssid encPol qosMode wme 11e wsm staaddr\n");

  for(i = 0; i < pMac->lim.maxStation; i++)
  {
    if (pMac->dph.dphHashTable.pDphNodeArray[i].added)
    {
      p += log_sprintf( pMac,p, "%d  %d  %d      %d         %d   %d %d   %d  %x:%x:%x:%x:%x:%x\n",
                            pMac->dph.dphHashTable.pDphNodeArray[i].assocId,
                            pMac->dph.dphHashTable.pDphNodeArray[i].staIndex,
                            pMac->dph.dphHashTable.pDphNodeArray[i].bssId,
                            pMac->dph.dphHashTable.pDphNodeArray[i].encPolicy,
                            pMac->dph.dphHashTable.pDphNodeArray[i].qosMode,
                            pMac->dph.dphHashTable.pDphNodeArray[i].wmeEnabled,
                            pMac->dph.dphHashTable.pDphNodeArray[i].lleEnabled,
                            pMac->dph.dphHashTable.pDphNodeArray[i].wsmEnabled,
                            pMac->dph.dphHashTable.pDphNodeArray[i].staAuthenticated,
                            pMac->dph.dphHashTable.pDphNodeArray[i].staAddr[0],
                            pMac->dph.dphHashTable.pDphNodeArray[i].staAddr[1],
                            pMac->dph.dphHashTable.pDphNodeArray[i].staAddr[2],
                            pMac->dph.dphHashTable.pDphNodeArray[i].staAddr[3],
                            pMac->dph.dphHashTable.pDphNodeArray[i].staAddr[4],
                            pMac->dph.dphHashTable.pDphNodeArray[i].staAddr[5]);
    }
  }
  return p;
}

// add the specified tspec to the tspec list
static char* limDumpTsecTable( tpAniSirGlobal pMac, char* p)
{
    int ctspec;
    tpLimTspecInfo  pTspecList = &pMac->lim.tspecInfo[0];

    p += log_sprintf( pMac,p, "=======LIM TSPEC TABLE DUMP\n");
    p += log_sprintf( pMac,p, "Num\tIdx\tAID\tAckPol\tUP\tPSB\tAgg\tAccessPol\tDir\tTSID\ttraffic\n");

    for (ctspec = 0; ctspec < LIM_NUM_TSPEC_MAX; ctspec++, pTspecList++)
    {
        if (pTspecList->inuse)
            p = dumpTspecTableSummary(pMac, pTspecList, p, ctspec);
    }
    return p;
}

static char *
dump_lim_tspec_table( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    (void) arg1; (void) arg2; (void) arg3; (void) arg4;
    p = limDumpTsecTable(pMac, p);
    return p;
}

static char *
dump_lim_tspec_entry( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    (void) arg2; (void) arg3; (void) arg4;
    p = limDumpTspecEntry(pMac, p, arg1);
    return p;
}

static char *
dump_lim_dph_table_summary( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    (void) arg2; (void) arg3; (void) arg4;
    p = limDumpDphTableSummary(pMac, p);
    return p;
}


static char *
dump_lim_link_monitor_stats( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    tANI_U32 ind, val;

    (void) arg2; (void) arg3; (void) arg4;
    p += log_sprintf( pMac,p, "\n ----- LIM Heart Beat Stats ----- \n");
    p += log_sprintf( pMac,p, "No. of HeartBeat Failures in LinkEst State = %d\n",
                    pMac->lim.gLimHBfailureCntInLinkEstState);
    p += log_sprintf( pMac,p, "No. of Probe Failures after HB failed      = %d\n",
                    pMac->lim.gLimProbeFailureAfterHBfailedCnt);
    p += log_sprintf( pMac,p, "No. of HeartBeat Failures in Other States = %d\n",
                    pMac->lim.gLimHBfailureCntInOtherStates);

    if (wlan_cfgGetInt(pMac, WNI_CFG_HEART_BEAT_THRESHOLD, &val) == eSIR_SUCCESS)
        p += log_sprintf( pMac,p, "Cfg HeartBeat Threshold = %d\n", val);

    p += log_sprintf( pMac,p, "# Beacons Rcvd in HB interval    # of times\n");

    for (ind = 1; ind < MAX_NO_BEACONS_PER_HEART_BEAT_INTERVAL; ind++)
    {
         p += log_sprintf( pMac,p, "\t\t\t\t\t\t\t\t%2d\t\t\t\t\t\t\t\t\t\t\t%8d\n", ind,
                        pMac->lim.gLimHeartBeatBeaconStats[ind]);
    }
    p += log_sprintf( pMac,p, "\t\t\t\t\t\t\t\t%2d>\t\t\t\t\t\t\t\t\t\t%8d\n",
                    MAX_NO_BEACONS_PER_HEART_BEAT_INTERVAL-1,
                    pMac->lim.gLimHeartBeatBeaconStats[0]);

    if (arg1 != 0)
    {
        for (ind = 0; ind < MAX_NO_BEACONS_PER_HEART_BEAT_INTERVAL; ind++)
           pMac->lim.gLimHeartBeatBeaconStats[ind] = 0;

        pMac->lim.gLimHBfailureCntInLinkEstState   = 0;
        pMac->lim.gLimProbeFailureAfterHBfailedCnt = 0;
        pMac->lim.gLimHBfailureCntInOtherStates    = 0;

        p += log_sprintf( pMac,p, "\nReset HeartBeat Statistics\n");
    }
    return p;
}

static char *
dump_lim_edca_params( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    (void) arg1; (void) arg2; (void) arg3; (void) arg4;
    p = limDumpEdcaParams(pMac, p);
    return p;
}

static char *
dump_lim_acm_set( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    (void) arg3; (void) arg4;
    limSetEdcaBcastACMFlag(pMac, arg1 /*ac(0..3)*/, arg2 /*(acmFlag = 1 to set ACM*/);
    return p;
}

static char *
dump_lim_bgscan_toggle( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    (void) arg2; (void) arg3; (void) arg4;
    pMac->lim.gLimForceBackgroundScanDisable = (arg1 == 0) ? 1 : 0;
    p += log_sprintf( pMac,p, "Bgnd scan is now %s\n",
        (pMac->lim.gLimForceBackgroundScanDisable) ? "Disabled" : "On");
    return p;
}

static char *
dump_lim_linkmonitor_toggle( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    (void) arg2; (void) arg3; (void) arg4;
    pMac->sys.gSysEnableLinkMonitorMode = (arg1 == 0) ? 0 : 1;
    p += log_sprintf( pMac,p, "LinkMonitor mode enable = %s\n",
        (pMac->sys.gSysEnableLinkMonitorMode) ? "On" : "Off");
    return p;
}

static char *
dump_lim_proberesp_toggle( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    (void) arg2; (void) arg3; (void) arg4;
    pMac->lim.gLimProbeRespDisableFlag = (arg1 == 0) ? 0 : 1;
    p += log_sprintf( pMac,p, "ProbeResponse mode disable = %s\n",
        (pMac->lim.gLimProbeRespDisableFlag) ? "On" : "Off");
    return p;
}

static char *
dump_lim_add_sta( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    tpDphHashNode pStaDs;
    tSirMacAddr staMac = {0};
    tANI_U16 aid;
    if(arg2 > 5)
      goto addStaFail;
    aid = limAssignAID(pMac);
    pStaDs = dphGetHashEntry(pMac, aid);
    if(NULL == pStaDs)
    {
        staMac[5] = (tANI_U8) arg1;
        pStaDs = dphAddHashEntry(pMac, staMac, aid);
        if(NULL == pStaDs)
          goto addStaFail;

        pStaDs->staType = STA_ENTRY_PEER;
        switch(arg2)
        {
            //11b station
            case 0:
                        {
                            pStaDs->mlmStaContext.htCapability = 0;
                            pStaDs->erpEnabled = 0;
                            p += log_sprintf( pMac,p, "11b");
                        }
                        break;
            //11g station
            case 1:
                        {
                            pStaDs->mlmStaContext.htCapability = 0;
                            pStaDs->erpEnabled = 1;
                            p += log_sprintf( pMac,p, "11g");
                        }
                        break;
            //ht20 station non-GF
            case 2:
                        {
                            pStaDs->mlmStaContext.htCapability = 1;
                            pStaDs->erpEnabled = 1;
                            pStaDs->htSupportedChannelWidthSet = 0;
                            pStaDs->htGreenfield = 0;
                            p += log_sprintf( pMac,p, "HT20 non-GF");
                        }
                        break;
            //ht20 station GF
            case 3:
                        {
                            pStaDs->mlmStaContext.htCapability = 1;
                            pStaDs->erpEnabled = 1;
                            pStaDs->htSupportedChannelWidthSet = 0;
                            pStaDs->htGreenfield = 1;
                            p += log_sprintf( pMac,p, "HT20 GF");
                        }
                        break;
            //ht40 station non-GF
            case 4:
                        {
                            pStaDs->mlmStaContext.htCapability = 1;
                            pStaDs->erpEnabled = 1;
                            pStaDs->htSupportedChannelWidthSet = 1;
                            pStaDs->htGreenfield = 0;
                            p += log_sprintf( pMac,p, "HT40 non-GF");
                        }
                        break;
            //ht40 station GF
            case 5:
                        {
                            pStaDs->mlmStaContext.htCapability = 1;
                            pStaDs->erpEnabled = 1;
                            pStaDs->htSupportedChannelWidthSet = 1;
                            pStaDs->htGreenfield = 1;
                            p += log_sprintf( pMac,p, "HT40 GF");
                        }
                        break;
            default:
                        {
                          p += log_sprintf( pMac,p, "arg2 not in range [0..3]. Station not added.\n");
                          goto addStaFail;
                        }
                        break;
        }

        pStaDs->added = 1;
        p += log_sprintf( pMac,p, " station with mac address 00:00:00:00:00:%x added.\n", (tANI_U8)arg1);
        limAddSta(pMac, pStaDs);
    }
    else
    {
addStaFail:
        p += log_sprintf( pMac,p, "Could not add station\n");
        p += log_sprintf( pMac,p, "arg1: 6th byte of the station MAC address\n");
        p += log_sprintf( pMac,p, "arg2[0..5] : station type as described below\n");
        p += log_sprintf( pMac,p, "\t 0: 11b, 1: 11g, 2: HT20 non-GF, 3: HT20 GF, 4: HT40 non-GF, 5: HT40 GF\n");
    }
    return p;
}

static char *
dump_lim_del_sta( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    tpDphHashNode pStaDs;
    tLimMlmDisassocInd mlmDisassocInd;

    tANI_U8 reasonCode = eSIR_MAC_DISASSOC_DUE_TO_INACTIVITY_REASON;

    pStaDs = dphGetHashEntry(pMac, (tANI_U16) arg1);

    if(NULL == pStaDs)
    {
            p += log_sprintf( pMac,p, "Could not find station with assocId = %d\n", arg1);
            return p;
    }

    if (pStaDs->mlmStaContext.mlmState != eLIM_MLM_LINK_ESTABLISHED_STATE)
    {
        p += log_sprintf( pMac,p, "received Disassoc frame from peer that is in state %X \n", pStaDs->mlmStaContext.mlmState);
        return p;
    }

    pStaDs->mlmStaContext.cleanupTrigger = eLIM_PEER_ENTITY_DISASSOC;
    pStaDs->mlmStaContext.disassocReason = (tSirMacReasonCodes) reasonCode;

    // Issue Disassoc Indication to SME.
    palCopyMemory( pMac->hHdd, (tANI_U8 *) &mlmDisassocInd.peerMacAddr,
                                (tANI_U8 *) pStaDs->staAddr, sizeof(tSirMacAddr));
    mlmDisassocInd.reasonCode = reasonCode;
#if (WNI_POLARIS_FW_PRODUCT == AP)
    mlmDisassocInd.aid        = pStaDs->assocId;
#endif
    mlmDisassocInd.disassocTrigger = eLIM_PEER_ENTITY_DISASSOC;

    limPostSmeMessage(pMac,  LIM_MLM_DISASSOC_IND,  (tANI_U32 *) &mlmDisassocInd);
    // Receive path cleanup
    limCleanupRxPath(pMac, pStaDs);
    return p;
}


static char *
set_lim_prot_cfg( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{

/**********************************
* Protection Enable
*
*LOWER byte for associated stations
*UPPER byte for overlapping stations.
*11g ==> protection from 11g
*11b ==> protection from 11b
*each byte will have the following info
*bit7     bit6     bit5   bit4 bit3   bit2  bit1 bit0
*reserved reserved RIFS Lsig n-GF ht20 11g 11b
**********************************
WNI_CFG_PROTECTION_ENABLED    I    4    9
V    RW    NP  RESTART
LIM
0    0xff    0xff
V    RW    NP  RESTART
LIM
0    0xffff    0xffff

#ENUM FROM_llB 0
#ENUM FROM_llG 1
#ENUM HT_20 2
#ENUM NON_GF 3
#ENUM LSIG_TXOP 4
#ENUM RIFS 5
#ENUM OLBC_FROM_llB 8
#ENUM OLBC_FROM_llG 9
#ENUM OLBC_HT20 10
#ENUM OLBC_NON_GF 11
#ENUM OLBC_LSIG_TXOP 12
#ENUM OLBC_RIFS 13
******************************************/
    if(1 == arg1)
        dump_cfg_set(pMac, WNI_CFG_PROTECTION_ENABLED, 0xff, arg3, arg4, p);
    else if(2 == arg1)
        dump_cfg_set(pMac, WNI_CFG_PROTECTION_ENABLED, arg2 & 0xff, arg3, arg4, p);
    else
    {
        p += log_sprintf( pMac,p, "To set protection config:\n");
        p += log_sprintf( pMac,p, "arg1: operation type(1 -> set to Default 0xff, 2-> set to a arg2, else print help)\n");
    }
    return p;
}


static char *
dump_lim_set_protection_control( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    dump_cfg_set(pMac, WNI_CFG_FORCE_POLICY_PROTECTION, arg1, arg2, arg3, p);
    limSetCfgProtection(pMac);
    return p;
}


static char *
dump_lim_send_SM_Power_Mode( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    tSirMsgQ    msg;
    tpSirMbMsg  pMBMsg;
        tSirMacHTMIMOPowerSaveState state;

        p += log_sprintf( pMac,p, "%s: Verifying the Arguements\n", __FUNCTION__);
    if ((arg1 > 3) || (arg1 == 2))
    {
                p += log_sprintf( pMac,p, "Invalid Arguement , enter one of the valid states\n");
                return p;
        }

        state = (tSirMacHTMIMOPowerSaveState) arg1;

    palAllocateMemory(pMac->hHdd, (void **)&pMBMsg, WNI_CFG_MB_HDR_LEN + sizeof(tSirMacHTMIMOPowerSaveState));
    pMBMsg->type = eWNI_PMC_SMPS_STATE_IND;
    pMBMsg->msgLen = (tANI_U16)(WNI_CFG_MB_HDR_LEN + sizeof(tSirMacHTMIMOPowerSaveState));
    palCopyMemory(pMac->hHdd, pMBMsg->data, &state, sizeof(tSirMacHTMIMOPowerSaveState));

    msg.type = eWNI_PMC_SMPS_STATE_IND;
    msg.bodyptr = pMBMsg;
    msg.bodyval = 0;

    if (limPostMsgApi(pMac, &msg) != TX_SUCCESS)
    {
            p += log_sprintf( pMac,p, "Updating the SMPower Request has failed \n");
        palFreeMemory(pMac->hHdd, pMBMsg);
    }
    else
    {
        p += log_sprintf( pMac,p, "Updating the SMPower Request is Done \n");
    }

        return p;
}




static char *
dump_lim_addba_req( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
tSirRetStatus status;
tpDphHashNode pSta;

  (void) arg4;

  // Get DPH Sta entry for this ASSOC ID
  pSta = dphGetHashEntry( pMac, (tANI_U16) arg1 );
  if( NULL == pSta )
  {
    p += log_sprintf( pMac, p,
        "\n%s: Could not find entry in DPH table for assocId = %d\n",
        __FUNCTION__,
        arg1 );
  }
  else
  {
    status = limPostMlmAddBAReq( pMac, pSta, (tANI_U8) arg2, (tANI_U16) arg3 );
    p += log_sprintf( pMac, p,
        "\n%s: Attempted to send an ADDBA Req to STA Index %d, for TID %d. Send Status = %s\n",
        __FUNCTION__,
        pSta->staIndex,
        arg2,
        limResultCodeStr( status ));
  }

  return p;
}

static char *
dump_lim_delba_req( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
tSirRetStatus status;
tpDphHashNode pSta;

  // Get DPH Sta entry for this ASSOC ID
  pSta = dphGetHashEntry( pMac, (tANI_U16) arg1 );
  if( NULL == pSta )
  {
    p += log_sprintf( pMac, p,
        "\n%s: Could not find entry in DPH table for assocId = %d\n",
        __FUNCTION__,
        arg1 );
  }
  else
  {
    status = limPostMlmDelBAReq( pMac, pSta, (tANI_U8) arg2, (tANI_U8) arg3, (tANI_U16) arg4 );
    p += log_sprintf( pMac, p,
        "\n%s: Attempted to send a DELBA Ind to STA Index %d, "
        "as the BA \"%s\" for TID %d, with Reason code %d. "
        "Send Status = %s\n",
        __FUNCTION__,
        pSta->staIndex,
        (arg2 == 1)? "Initiator": "Recipient",
        arg3, // TID
        arg4, // Reason Code
        limResultCodeStr( status ));
  }

  return p;
}

static char *
dump_lim_ba_timeout( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
  // Call HAL API to trigger deletion of BA due to timeout
  (void)halMsg_PostBADeleteInd( pMac, (tANI_U16) arg1, (tANI_U8) arg2, (tANI_U8) arg3,
         HAL_BA_ERR_TIMEOUT );
  p += log_sprintf( pMac, p,
      "\n%s: Attempted to trigger a BA Timeout Ind to STA Index %d, for TID %d, Direction %d\n",
      __FUNCTION__,
      arg1, // STA index
      arg2, // TID
      arg3 ); // BA Direction

  return p;
}

static char *
dump_lim_list_active_ba( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
tANI_U32 i;
tpDphHashNode pSta;

  (void) arg2; (void) arg3; (void) arg4;

  // Get DPH Sta entry for this ASSOC ID
  pSta = dphGetHashEntry( pMac, (tANI_U16) arg1 );
  if( NULL == pSta )
  {
    p += log_sprintf( pMac, p,
        "\n%s: Could not find entry in DPH table for assocId = %d\n",
        __FUNCTION__,
        arg1 );
  }
  else
  {
    p += log_sprintf( pMac, p,
        "\nList of Active BA sessions for STA Index %d with Assoc ID %d\n",
        pSta->staIndex,
        arg1 );

    p += log_sprintf( pMac, p, "TID\tRxBA\tTxBA\tRxBufferSize\tTxBufferSize\tRxBATimeout\tTxBATimeout\n");
    for( i = 0; i < STACFG_MAX_TC; i ++ )
      p += log_sprintf( pMac, p,
          "%d\t%d\t%d\t%d\t%d\t%d\t%d\n",
          i, // TID
          pSta->tcCfg[i].fUseBARx,
          pSta->tcCfg[i].fUseBATx,
          pSta->tcCfg[i].rxBufSize,
          pSta->tcCfg[i].txBufSize,
          pSta->tcCfg[i].tuRxBAWaitTimeout,
          pSta->tcCfg[i].tuTxBAWaitTimeout );
  }

  return p;
}


static char *
dump_lim_AddBA_DeclineStat( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{

    int Tid, Enable=(arg1 & 0x1);
    tANI_U8 val;

    if (arg1 > 1) {
        log_sprintf( pMac,p, "%s:Invalid Value is entered for Enable/Disable \n", __FUNCTION__ );
        arg1 &= 1;
    }       
    
    val = pMac->lim.gAddBA_Declined;
    
    if (arg2 > 7) {
        log_sprintf( pMac,p, "%s:Invalid Value is entered for Tid \n", __FUNCTION__ );
        Tid = arg2 & 0x7;
    } else
        Tid = arg2;
    
    
    if ( Enable)
        val  |= Enable << Tid;
    else
        val &=  ~(0x1 << Tid);

    if (cfgSetInt(pMac, (tANI_U16)WNI_CFG_ADDBA_REQ_DECLINE, (tANI_U32) val) != eSIR_SUCCESS)
             log_sprintf( pMac,p, "%s:Config Set for ADDBA REQ Decline has failed \n", __FUNCTION__ );

     log_sprintf( pMac,p, "%s:Decline value %d is being set for TID %d ,\n \tAddBA_Decline Cfg value is %d \n", __FUNCTION__ , arg1, Tid, (int) val);

     return p;
}
static char *
dump_lim_set_dot11_mode( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{

    dump_cfg_set(pMac, WNI_CFG_DOT11_MODE, arg1, arg2, arg3, p);
    if ( (limGetSystemRole(pMac) == eLIM_AP_ROLE) ||
          (limGetSystemRole(pMac) == eLIM_STA_IN_IBSS_ROLE))
        schSetFixedBeaconFields(pMac);
    p += log_sprintf( pMac,p, "The Dot11 Mode is set to %s", limDot11ModeStr(pMac, (tANI_U8)pMac->lim.gLimDot11Mode));
    return p;
}


static char* dump_lim_update_cb_Mode(tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    tANI_U32 localPwrConstraint;
    
    if ( !pMac->lim.htCapability )
    {
        p += log_sprintf( pMac,p, "Error: Dot11 mode is non-HT, can not change the CB mode.\n");
        return p;
    }
    
    pMac->lim.gHTSecondaryChannelOffset = arg1;
    setupCBState(pMac,  limGetAniCBState(pMac->lim.gHTSecondaryChannelOffset));

    if(eSIR_SUCCESS != cfgSetInt(pMac, WNI_CFG_CHANNEL_BONDING_MODE,  
                                    arg1 ? WNI_CFG_CHANNEL_BONDING_MODE_ENABLE : WNI_CFG_CHANNEL_BONDING_MODE_DISABLE))
        p += log_sprintf(pMac,p, "cfgSetInt failed for WNI_CFG_CHANNEL_BONDING_MODE\n");
    
    wlan_cfgGetInt(pMac, WNI_CFG_LOCAL_POWER_CONSTRAINT, &localPwrConstraint);
        
    limSendSwitchChnlParams(pMac, pMac->lim.gLimCurrentChannelId, pMac->lim.gHTSecondaryChannelOffset,
                                                                  (tPowerdBm) localPwrConstraint);
    if ( (limGetSystemRole(pMac) == eLIM_AP_ROLE) ||
          (limGetSystemRole(pMac) == eLIM_STA_IN_IBSS_ROLE))
           schSetFixedBeaconFields(pMac);
    return p;
    
}

static char* dump_lim_abort_scan(tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
 (void) arg1; (void) arg2; (void) arg3; (void) arg4;
 //csrScanAbortMacScan(pMac);
    return p;
    
}

static char* dump_lim_start_stop_bg_scan(tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
 (void) arg2; (void) arg3; (void) arg4;

 if (TX_TIMER_VALID(pMac->lim.limTimers.gLimBackgroundScanTimer))
 {
     limDeactivateAndChangeTimer(pMac, eLIM_BACKGROUND_SCAN_TIMER);
 }

 if(arg1 == 1)
 {
     if (tx_timer_activate(
                         &pMac->lim.limTimers.gLimBackgroundScanTimer) != TX_SUCCESS)
     {
         pMac->lim.gLimBackgroundScanTerminate = TRUE;
     }
     else
     {
         pMac->lim.gLimBackgroundScanTerminate = FALSE;
         pMac->lim.gLimBackgroundScanDisable = false;
         pMac->lim.gLimForceBackgroundScanDisable = false;
     }
 }
 else
 {
     pMac->lim.gLimBackgroundScanTerminate = TRUE;
     pMac->lim.gLimBackgroundScanChannelId = 0;
     pMac->lim.gLimBackgroundScanDisable = true;
     pMac->lim.gLimForceBackgroundScanDisable = true;
 }
    return p;
    
}

static char* 
dump_lim_get_pe_statistics(tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tpAniGetPEStatsReq pReq;
    tANI_U32 statsMask;

    (void) arg2; (void) arg3; (void) arg4;

    
    switch(arg1)
    {        
        case 1:
            statsMask = PE_SUMMARY_STATS_INFO;
            break;
        case 2:
            statsMask = PE_GLOBAL_CLASS_A_STATS_INFO;
            break;
        case 3:
            statsMask = PE_GLOBAL_CLASS_B_STATS_INFO;
            break;
        case 4:
            statsMask = PE_GLOBAL_CLASS_C_STATS_INFO;
            break;
        case 5:
            statsMask = PE_PER_STA_STATS_INFO;
            break;
        default:
            return p;
    }
    

    if( eHAL_STATUS_SUCCESS != (status = palAllocateMemory (pMac->hHdd, (void**) &pReq, sizeof(tAniGetPEStatsReq))))
    {
        p += log_sprintf( pMac,p, "Error: Unable to allocate memory.\n");
        return p;
    }

    palZeroMemory( pMac, pReq, sizeof(*pReq));
    
    pReq->msgType = eWNI_SME_GET_STATISTICS_REQ;
    pReq->statsMask = statsMask;
    pReq->staId = (tANI_U16)arg2;

    pMac->lim.gLimRspReqd = eANI_BOOLEAN_TRUE;
    limPostSmeMessage(pMac, eWNI_SME_GET_STATISTICS_REQ, (tANI_U32 *) pReq);
    
    return p;
    
}

extern char* setLOGLevel( tpAniSirGlobal pMac, char *p, tANI_U32 module, tANI_U32 level );
static char *
dump_lim_set_log_level( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
	p = setLOGLevel(pMac, p, arg1, arg2);
	return p;
}

static char *
dump_lim_update_log_level( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    vos_trace_setLevel( arg1, arg2 );
    return p;
}


static char *
dump_lim_scan_req_send( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    (void) arg1; (void) arg2; (void) arg3; (void) arg4;
    p = sendSmeScanReq(pMac, p);
    return p;
}

static char *
dump_lim_sme_ReAssocReq( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
#if (WNI_POLARIS_FW_PRODUCT == AP)
    p += log_sprintf( pMac,p, "ReAssocReq not supported in AP\n");
#else
    tCsrRoamProfile profile;
    tSirBssDescription bssDesc = { 0x96, 
                                                   {0x00, 0x0A, 0xF5, 0xBE, 0xFC, 0xF3},   // Mac Address of the new AP change this
                                                   {0xAA8E17D3, 1}, 
                                                   0x64, 
                                                   0x421, 
                                                   2, 
                                                   1, 
                                                   0xE6, 
                                                   0, 
                                                   0x6, 
                                                   0x6, 
                                                   {0, 0, 0}, 
                                                   0, 
                                                   0x807b794F, 
                                                   {0x6E410600} 
                                                };
    tSirMacSSid     ssid = {6, 
                                       "AniNet" 
                                      };

    (void) arg1; (void) arg2; (void) arg3; (void) arg4;
    palZeroMemory(pMac->hHdd, (void **)&profile, sizeof(profile));

    profile.SSIDs.numOfSSIDs = 1;
    profile.SSIDs.SSIDList->SSID = ssid;

    profile.phyMode = eCSR_DOT11_MODE_AUTO;
    profile.negotiatedAuthType = eCSR_AUTH_TYPE_OPEN_SYSTEM;
    {
        struct tLimPreAuthNode *pAuthNode;

        pAuthNode = limAcquireFreePreAuthNode(pMac, &pMac->lim.gLimPreAuthTimerTable);

        if (pAuthNode)
        {
            palCopyMemory( pMac->hHdd, (tANI_U8 *) pAuthNode->peerMacAddr,
                          bssDesc.bssId,
                          sizeof(tSirMacAddr));
            pAuthNode->fTimerStarted = 0;
            pAuthNode->mlmState = eLIM_MLM_AUTHENTICATED_STATE;
            pAuthNode->authType = (tAniAuthType) eSIR_OPEN_SYSTEM;
            limAddPreAuthNode(pMac, pAuthNode);
        }
    }        
    csrSendSmeReassocReqMsg(pMac, &bssDesc, &profile);
    p += log_sprintf( pMac,p, "ReAssocReq is issued to CSR\n");
#endif
    return p;
}

static char *
dump_lim_dot11h_stats( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    unsigned int i;
    (void) arg1; (void) arg2; (void) arg3; (void) arg4;

    p += log_sprintf(pMac, p, "11h Enabled = %s\n", pMac->lim.gLim11hEnable? "TRUE": "FALSE");

#if (WNI_POLARIS_FW_PACKAGE == ADVANCED) && defined(ANI_PRODUCT_TYPE_AP)
    p += log_sprintf(pMac, p, "Measurement request issued by WSM = %s\n",
                              (pMac->lim.gpLimMeasReq != NULL)? "ISSUED": "NOT ISSUED");
    p += log_sprintf(pMac, p, "Measurement request details...\n");
    if (pMac->lim.gpLimMeasReq != NULL)
    {
        p += log_sprintf(pMac, p, "numChannels = %d, periodicMeasEnabled = %d, measIndPeriod = %d,"
            "shortTermPeriod = %d, averagingPeriod = %d, shortChannelScanDuration = %d,"
            "longChannelScanDuration = %d, SYS_TICK_DUR_MS = %d\n",
            pMac->lim.gpLimMeasReq->channelList.numChannels, pMac->lim.gpLimMeasReq->measControl.periodicMeasEnabled,
            pMac->lim.gpLimMeasReq->measIndPeriod, pMac->lim.gpLimMeasReq->measDuration.shortTermPeriod,
            pMac->lim.gpLimMeasReq->measDuration.averagingPeriod,
            pMac->lim.gpLimMeasReq->measDuration.shortChannelScanDuration,
            pMac->lim.gpLimMeasReq->measDuration.longChannelScanDuration, SYS_TICK_DUR_MS );

        p += log_sprintf(pMac, p, "Measurement channels...\n");
        for (i = 0; i < pMac->lim.gpLimMeasReq->channelList.numChannels; i++)
        {
            p += log_sprintf(pMac, p, "%d ", pMac->lim.gpLimMeasReq->channelList.channelNumber[i]);
        }
        p += log_sprintf(pMac, p, "\n");
        p += log_sprintf(pMac, p, "Total Number of BSS learned = %d\n", pMac->lim.gpLimMeasData->numBssWds);
        p += log_sprintf(pMac, p, "Total Number of Channels learned = %d\n", pMac->lim.gpLimMeasData->numMatrixNodes);
        p += log_sprintf(pMac, p, "Duration of learning = %d\n", pMac->lim.gpLimMeasData->duration);
        p += log_sprintf(pMac, p, "Is measurement Indication timer active = %s\n",
                                  (pMac->lim.gLimMeasParams.isMeasIndTimerActive)?"YES": "NO");
        p += log_sprintf(pMac, p, "Next learn channel Id = %d\n", pMac->lim.gLimMeasParams.nextLearnChannelId);
    }
    p += log_sprintf(pMac, p, "Measurement running = %s\n",
                              pMac->sys.gSysEnableLearnMode?"TRUE": "FALSE");
#endif
    p += log_sprintf(pMac, p, "Is system in learn mode = %s\n",
                              pMac->lim.gLimSystemInScanLearnMode?"YES": "NO");
    
    p += log_sprintf(pMac, p, "Quiet Enabled = %s\n", (pMac->lim.gLimSpecMgmt.fQuietEnabled)?"YES": "NO");
    p += log_sprintf(pMac, p, "Quiet state = %d\n", pMac->lim.gLimSpecMgmt.quietState);
    p += log_sprintf(pMac, p, "Quiet Count = %d\n", pMac->lim.gLimSpecMgmt.quietCount);
    p += log_sprintf(pMac, p, "Quiet Duration in ticks = %d\n", pMac->lim.gLimSpecMgmt.quietDuration);
    p += log_sprintf(pMac, p, "Quiet Duration in TU = %d\n", pMac->lim.gLimSpecMgmt.quietDuration_TU);
    
    p += log_sprintf(pMac, p, "Channel switch state = %d\n", pMac->lim.gLimSpecMgmt.dot11hChanSwState);
    p += log_sprintf(pMac, p, "Channel switch mode = %s\n",
            (pMac->lim.gLimChannelSwitch.switchMode == eSIR_CHANSW_MODE_SILENT)?"SILENT": "NORMAL");
    p += log_sprintf(pMac, p, "Channel switch primary channel = %d\n",
                              pMac->lim.gLimChannelSwitch.primaryChannel);
    p += log_sprintf(pMac, p, "Channel switch secondary sub band = %d\n",
                              pMac->lim.gLimChannelSwitch.secondarySubBand);
    p += log_sprintf(pMac, p, "Channel switch switch count = %d\n",
                              pMac->lim.gLimChannelSwitch.switchCount);
    p += log_sprintf(pMac, p, "Channel switch switch timeout value = %d\n",
                              pMac->lim.gLimChannelSwitch.switchTimeoutValue);

    p += log_sprintf(pMac, p, "Radar interrupt configured = %s\n",
                              pMac->lim.gLimSpecMgmt.fRadarIntrConfigured?"YES": "NO");
    p += log_sprintf(pMac, p, "Radar detected in current operating channel = %s\n",
                              pMac->lim.gLimSpecMgmt.fRadarDetCurOperChan?"YES": "NO");
    p += log_sprintf(pMac, p, "Radar detected channels...\n");
    for (i = 0; i < pMac->sys.radarDetectCount; i++)
    {
        p += log_sprintf(pMac, p, "%d ", pMac->sys.detRadarChIds[i]);
    }
    p += log_sprintf(pMac, p, "\n");
    
    return p;
}

static char *
dump_lim_enable_measurement( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    (void) arg2; (void) arg3; (void) arg4;

    if (arg1)
    {
        pMac->sys.gSysEnableLearnMode = eANI_BOOLEAN_TRUE;
        p += log_sprintf(pMac, p, "Measurement enabled\n");
    }
    else
    {
        pMac->sys.gSysEnableLearnMode = eANI_BOOLEAN_FALSE;
        p += log_sprintf(pMac, p, "Measurement disabled\n");
    }

    return p;
}

static char *
dump_lim_enable_quietIE( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    (void) arg2; (void) arg3; (void) arg4;

    if (arg1)
    {
        pMac->lim.gLimSpecMgmt.fQuietEnabled = eANI_BOOLEAN_TRUE;
        p += log_sprintf(pMac, p, "QuietIE enabled\n");
    }
    else
    {
        pMac->lim.gLimSpecMgmt.fQuietEnabled = eANI_BOOLEAN_FALSE;
        p += log_sprintf(pMac, p, "QuietIE disabled\n");
    }

    return p;
}

static char *
dump_lim_disable_enable_scan( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    (void) arg2; (void) arg3; (void) arg4;

    if (arg1)
    {
        pMac->lim.fScanDisabled = 1;
        p += log_sprintf(pMac, p, "Scan disabled\n");
    }
    else
    {
        pMac->lim.fScanDisabled = 0;
        p += log_sprintf(pMac, p, "scan enabled\n");
    }

    return p;
}

static char *finishScan(tpAniSirGlobal pMac, char *p)
{
    tSirMsgQ         msg;

    p += log_sprintf( pMac,p, "logDump finishScan \n");

    msg.type = SIR_LIM_MIN_CHANNEL_TIMEOUT;
    msg.bodyval = 0;
    msg.bodyptr = NULL;
    
    limPostMsgApi(pMac, &msg);
    return p;
}


static char *
dump_lim_info( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    (void) arg1; (void) arg2; (void) arg3; (void) arg4;
    p = dumpLim( pMac, p );
    return p;
}

static char *
dump_lim_finishscan_send( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    (void) arg1; (void) arg2; (void) arg3; (void) arg4;
    p = finishScan(pMac, p);
    return p;
}

static char *
dump_lim_prb_rsp_send( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    (void) arg1; (void) arg2; (void) arg3; (void) arg4;
    p = testLimSendProbeRsp( pMac, p );
    return p;
}

static char *
dump_sch_beacon_trigger( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    (void) arg1; (void) arg2; (void) arg3; (void) arg4;
    p = triggerBeaconGen(pMac, p);
    return p;
}


static char* dump_lim_trace_cfg(tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    MTRACE(macTraceCfg(pMac, arg1, arg2, arg3, arg4);)
    return p;
}

static char* dump_lim_trace_dump(tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    MTRACE(macTraceDumpAll(pMac, (tANI_U8)arg1, (tANI_U8)arg2, arg3);)
    return p;
}

static char* dump_lim_set_scan_in_powersave( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
	p += log_sprintf( pMac,p, "logDump set scan in powersave to %d \n", arg1);
    dump_cfg_set(pMac, WNI_CFG_SCAN_IN_POWERSAVE, arg1, arg2, arg3, p);
    return p;
}

static tDumpFuncEntry limMenuDumpTable[] = {
	{0,     "PE (300-499)",                                          NULL},
	{300,    "LIM: Dump state(s)/statistics",                          dump_lim_info},
    {301,   "PE.LIM: dump TSPEC Table",                              dump_lim_tspec_table},
    {302,   "PE.LIM: dump specified TSPEC entry (id)",               dump_lim_tspec_entry},
    {303,   "PE.LIM: dump EDCA params",                              dump_lim_edca_params},
    {304,   "PE.LIM: dump DPH table summary",                        dump_lim_dph_table_summary},
    {305,   "PE.LIM: dump link monitor stats",                       dump_lim_link_monitor_stats},
    {306,   "PE.LIM:dump Set the BAR Decline stat(arg1= 1/0 (enable/disable) arg2 =TID",          dump_lim_AddBA_DeclineStat},
    {307,   "PE: LIM: dump CSR Send ReAssocReq",                     dump_lim_sme_ReAssocReq},
    {308,   "PE:LIM: dump all 11H related data",                     dump_lim_dot11h_stats},
    {309,   "PE:LIM: dump to enable Measurement on AP",              dump_lim_enable_measurement},
    {310,   "PE:LIM: dump to enable QuietIE on AP",                  dump_lim_enable_quietIE},
    {311,   "PE:LIM: disable/enable scan 1(disable)",                dump_lim_disable_enable_scan},    
    {320,   "PE.LIM: send sme scan request",                         dump_lim_scan_req_send},


    /*FIXME_GEN6*/
    /* This dump command is more of generic dump cmd and hence it should 
     * be moved to logDump.c
     */
    {321,   "PE:LIM: Set Log Level <VOS Module> <VOS Log Level>",    dump_lim_update_log_level},
    {322,   "PE.LIM: Enable/Disable PE Tracing",                     dump_lim_trace_cfg},
    {323,   "PE.LIM: Trace Dump if enabled",                           dump_lim_trace_dump},
    {331,   "PE.LIM: Send finish scan to LIM",                       dump_lim_finishscan_send},
    {332,   "PE.LIM: force probe rsp send from LIM",                 dump_lim_prb_rsp_send},
    {333,   "PE.SCH: Trigger to generate a beacon",                  dump_sch_beacon_trigger},
    {335,   "PE.LIM: set ACM flag (0..3)",                           dump_lim_acm_set},
    {336,   "PE.LIM: Send an ADDBA Req to peer MAC arg1=aid,arg2=tid, arg3=ssn",   dump_lim_addba_req},
    {337,   "PE.LIM: Send a DELBA Ind to peer MAC arg1=aid,arg2=recipient(0)/initiator(1),arg3=tid,arg4=reasonCode",    dump_lim_delba_req},
    {338,   "PE.LIM: Trigger a BA timeout for STA index",            dump_lim_ba_timeout},
    {339,   "PE.LIM: List active BA session(s) for AssocID",         dump_lim_list_active_ba},
    {340,   "PE.LIM: Set background scan flag (0-disable, 1-enable)",dump_lim_bgscan_toggle},
    {341,   "PE.LIM: Set link monitoring mode",                      dump_lim_linkmonitor_toggle},
    {342,   "PE.LIM: AddSta <6th byte of station Mac>",              dump_lim_add_sta},
    {343,   "PE.LIM: DelSta <aid>",                                  dump_lim_del_sta},
    {344,   "PE.LIM: Set probe respond flag",                        dump_lim_proberesp_toggle},
    {345,   "PE.LIM: set protection config bitmap",                  set_lim_prot_cfg},
    {346,   "PE:LIM: Set the Dot11 Mode",                            dump_lim_set_dot11_mode},
    {347,   "PE:Enable or Disable Protection",                       dump_lim_set_protection_control},
    {348,   "PE:LIM: Send SM Power Mode Action frame",               dump_lim_send_SM_Power_Mode},
    {349,   "PE: LIM: Change CB Mode",                               dump_lim_update_cb_Mode},
    {350,   "PE: LIM: abort scan",                                   dump_lim_abort_scan},
    {351,   "PE: LIM: Start stop BG scan",                           dump_lim_start_stop_bg_scan},
    {352,   "PE: LIM: PE statistics <scanmask>",                     dump_lim_get_pe_statistics},
    {353,   "PE: LIM: Set MAC log level <Mac Module ID> <Log Level>", dump_lim_set_log_level},
    {354,   "PE: LIM: Set Scan in Power Save <0-disable, 1-enable>",  dump_lim_set_scan_in_powersave},
};
	

void limDumpInit(tpAniSirGlobal pMac)
{
	logDumpRegisterTable( pMac, &limMenuDumpTable[0], 
						  sizeof(limMenuDumpTable)/sizeof(limMenuDumpTable[0]) );
}


#endif //#if defined(ANI_LOGDUMP)


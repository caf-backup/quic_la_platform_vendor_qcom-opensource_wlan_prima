#if !defined( __LIM_SESSION_H )
#define __LIM_SESSION_H


/**=========================================================================
  
  \file  limSession.h
  
  \brief prototype for lim Session related APIs

  \author Sunit Bhatia
  
   Copyright 2008 (c) Qualcomm, Incorporated.  All Rights Reserved.
   
   Qualcomm Confidential and Proprietary.
  
  ========================================================================*/


/*--------------------------------------------------------------------------
  Include Files
  ------------------------------------------------------------------------*/



/*-------------------------------------------------------------------------- 
  Preprocessor definitions and constants
  ------------------------------------------------------------------------*/
#ifdef WLAN_SOFTAP_FEATURE
#define NUM_WEP_KEYS 4
#endif


/*-------------------------------------------------------------------------- 
  Type declarations
  ------------------------------------------------------------------------*/

typedef struct sPESession           // Added to Support BT-AMP
{
    /* To check session table is in use or free*/
    tANI_U8                 available;
    tANI_U8                 peSessionId;
    tANI_U8                 smeSessionId;
    tANI_U16                transactionId;

    //In AP role: BSSID and selfMacAddr will be the same.
    //In STA role: they will be different
    tSirMacAddr             bssId;
    tSirMacAddr             selfMacAddr;
    tSirMacSSid             ssId;
    tANI_U8                 bssIdx;
    tANI_U8                 valid;
    tLimMlmStates           limMlmState;            //MLM State
    tLimMlmStates           limPrevMlmState;        //Previous MLM State
    tLimSmeStates           limSmeState;            //SME State
    tLimSmeStates           limPrevSmeState;        //Previous SME State
    tLimSystemRole          limSystemRole;
    tSirBssType             bssType;
    tANI_U8                 operMode;               // AP - 0; STA - 1 ; 
    tSirNwType              nwType;
    tpSirSmeStartBssReq     pLimStartBssReq;        //handle to smestart bss req
    tpSirSmeJoinReq         pLimJoinReq;            // handle to sme join req
    tpSirSmeReassocReq      pLimReAssocReq;         //handle to sme reassoc req
    tpLimMlmJoinReq         pLimMlmJoinReq;         //handle to MLM join Req
    void                    *pLimMlmReassocReq;      //handle to MLM reassoc Req
    tANI_U16                channelChangeReasonCode;
    tANI_U8                 dot11mode;
    tANI_U8                 htCapabality;
    tSirRFBand              limRFBand;
    tANI_U8                 limIbssActive;          //TO SUPPORT CONCURRENCY

    /* These global varibales moved to session Table to support BT-AMP : Oct 9th review */
    tAniAuthType            limCurrentAuthType;
    tANI_U16                limCurrentBssCaps;
    tANI_U8                 limCurrentBssQosCaps;
    tANI_U16                limCurrentBssPropCap;
    tANI_U8                 limSentCapsChangeNtf;
    tANI_U32                limCurrentTitanHtCaps;
    tANI_U16                limAID;

	/* Parameters  For Reassociation */
    tSirMacAddr             limReAssocbssId;
	tSirMacChanNum          limReassocChannelId;
    tSirMacSSid             limReassocSSID;
    tANI_U16                limReassocBssCaps;
    tANI_U8                 limReassocBssQosCaps;
    tANI_U16                limReassocBssPropCap;
    tANI_U32                limReassocTitanHtCaps;

    // Assoc or ReAssoc Response Data/Frame
    void                   *limAssocResponseData;
    


    /** BSS Table parameters **/


    /*
    * staId:  Start BSS: 	this is the  Sta Id for the BSS.
                 Join: 		this is the selfStaId
      In both cases above, the peer STA ID wll be stored in dph hash table.
    */
    tANI_U16                staId;	        
    tANI_U16                statypeForBss;          //to know session is for PEER or SELF
    tANI_U8                 shortSlotTimeSupported;
    tANI_U8                 fShortPreamble;   
    tANI_U8                 llaCoexist;    
    tANI_U8                 llbCoexist;
    tANI_U8                 llgCoexist;
    tANI_U8                 ht20Coexist;
    tANI_U8                 llnNonGFCoexist;
    tANI_U8                 fLsigTXOPProtectionFullSupport;
    tANI_U8                 fRIFSMode;
    tSirMacBeaconInterval   beaconInterval;
    tANI_U8                 dtimPeriod;
    tSirMacRateSet       rateSet;
    tSirMacRateSet       extRateSet;
    tSirMacHTOperatingMode  htOperMode;
    tANI_U8                 currentOperChannel;
    tANI_U8                 LimRxedBeaconCntDuringHB;
    
    //Time stamp of the last beacon received from the BSS to which STA is connected.
    tANI_U64                lastBeaconTimeStamp;
    //RX Beacon count for the current BSS to which STA is connected.
    tANI_U32                currentBssBeaconCnt;
    tANI_U8                 lastBeaconDtimCount;
    tANI_U8                 lastBeaconDtimPeriod;

    tANI_U32                bcnLen;
    tANI_U8                 *beacon;                //Used to store last beacon / probe response before assoc.

    tANI_U32                assocReqLen;
    tANI_U8                 *assocReq;              //Used to store association request frame sent out while associating.

    tANI_U32                assocRspLen;
    tANI_U8                 *assocRsp;              //Used to store association response recieved while associating
    tAniSirDph              dph;
    void *                  *parsedAssocReq;        // Used to store parsed assoc req from various requesting station
    
    tANI_U32           	    encryptType;

#ifdef WLAN_SOFTAP_FEATURE
    tANI_BOOLEAN            bTkipCntrMeasActive;    // Used to keep record of TKIP counter measures start/stop

    tANI_U8                 gLimProtectionControl;  //used for 11n protection

    // OBss Mode . set when we have Non HT STA is associated or with in overlap bss
    tANI_U8                 gHTObssMode; 

    tANI_U8                 gHTNonGFDevicesPresent;	

    //protection related config cache
    tCfgProtection          cfgProtection;
	
    // Number of legacy STAs associated
    tLimProtStaParams          gLim11bParams;

    // Number of 11A STAs associated
    tLimProtStaParams          gLim11aParams;

    // Number of non-ht non-legacy STAs associated
    tLimProtStaParams          gLim11gParams;

    //Number of nonGf STA associated
    tLimProtStaParams       gLimNonGfParams;

    //Number of HT 20 STAs associated
    tLimProtStaParams          gLimHt20Params;

    //Number of Lsig Txop not supported STAs associated
    tLimProtStaParams          gLimLsigTxopParams;

    // Number of STAs that do not support short preamble
    tLimNoShortParams         gLimNoShortParams;

    // Number of STAs that do not support short slot time
    tLimNoShortSlotParams   gLimNoShortSlotParams;


    // OLBC parameters
    tLimProtStaParams  gLimOlbcParams;

    // OLBC parameters
    tLimProtStaParams  gLimOverlap11gParams;

    tLimProtStaParams  gLimOverlap11aParams;
    tLimProtStaParams gLimOverlapHt20Params;
    tLimProtStaParams gLimOverlapNonGfParams;

    //cache for each overlap
    tCacheParams     protStaCache[LIM_PROT_STA_CACHE_SIZE];

    tANI_U8                 privacy;
    tAniAuthType            authType;
    tSirKeyMaterial         WEPKeyMaterial[NUM_WEP_KEYS];

    tDot11fIERSN            gStartBssRSNIe;
    tDot11fIEWPA            gStartBssWPAIe;
    tSirAPWPSIEs            APWPSIEs;
    tANI_U8                 apUapsdEnable;
    tSirWPSPBCSession       *pAPWPSPBCSession;
    tANI_U32                DefProbeRspIeBitmap[8];
    tANI_U32                proxyProbeRspEn;
    tDot11fProbeResponse    probeRespFrame;
    tANI_U8                 ssidHidden;
    tANI_BOOLEAN            fwdWPSPBCProbeReq;
    tANI_U8                 wps_state;
#endif
    tPowerdBm  maxTxPower;   //MIN (Regulatory and local power constraint)
#if defined WLAN_FEATURE_VOWIFI
    tPowerdBm  txMgmtPower;
#endif
}tPESession, *tpPESession;

#define LIM_MAX_ACTIVE_SESSIONS 4


/*------------------------------------------------------------------------- 
  Function declarations and documenation
  ------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------
  
  \brief peCreateSession() - creates a new PE session given the BSSID

  This function returns the session context and the session ID if the session 
  corresponding to the passed BSSID is found in the PE session table.
    
  \param pMac                   - pointer to global adapter context
  \param bssid                   - BSSID of the new session
  \param sessionId             -session ID is returned here, if session is created.
  
  \return tpPESession          - pointer to the session context or NULL if session can not be created.
  
  \sa
  
  --------------------------------------------------------------------------*/
tpPESession peCreateSession(tpAniSirGlobal pMac, tANI_U8 *bssid , tANI_U8* sessionId, tANI_U16 numSta);


/*--------------------------------------------------------------------------
  \brief peFindSessionByBssid() - looks up the PE session given the BSSID.

  This function returns the session context and the session ID if the session 
  corresponding to the given BSSID is found in the PE session table.
    
  \param pMac                   - pointer to global adapter context
  \param bssid                   - BSSID of the session
  \param sessionId             -session ID is returned here, if session is found. 
  
  \return tpPESession          - pointer to the session context or NULL if session is not found.
  
  \sa
  --------------------------------------------------------------------------*/
tpPESession peFindSessionByBssid(tpAniSirGlobal pMac,  tANI_U8*  bssid,    tANI_U8* sessionId);



/*--------------------------------------------------------------------------
  \brief peFindSessionByPeerSta() - looks up the PE session given the Peer Station Address.

  This function returns the session context and the session ID if the session 
  corresponding to the given destination address is found in the PE session table.
    
  \param pMac                   - pointer to global adapter context
  \param sa                   - Peer STA Address of the session
  \param sessionId             -session ID is returned here, if session is found. 
  
  \return tpPESession          - pointer to the session context or NULL if session is not found.
  
  \sa
  --------------------------------------------------------------------------*/
tpPESession peFindSessionByPeerSta(tpAniSirGlobal pMac, tANI_U8*  sa, tANI_U8* sessionId);

/*--------------------------------------------------------------------------
  \brief peFindSessionBySessionId() - looks up the PE session given the session ID.

  This function returns the session context  if the session 
  corresponding to the given session ID is found in the PE session table.
    
  \param pMac                   - pointer to global adapter context
  \param sessionId             -session ID for which session context needs to be looked up.
  
  \return tpPESession          - pointer to the session context or NULL if session is not found.
  
  \sa
  --------------------------------------------------------------------------*/
 tpPESession peFindSessionBySessionId(tpAniSirGlobal pMac , tANI_U8 sessionId);




/*--------------------------------------------------------------------------
  \brief peDeleteSession() - deletes the PE session given the session ID.

    
  \param pMac                   - pointer to global adapter context
  \param sessionId             -session ID of the session which needs to be deleted.
    
  \sa
  --------------------------------------------------------------------------*/
void peDeleteSession(tpAniSirGlobal pMac, tpPESession psessionEntry);


/*--------------------------------------------------------------------------
  \brief peDeleteSession() - Returns the SME session ID and Transaction ID .

    
  \param pMac                   - pointer to global adapter context
  \param sessionId             -session ID of the session which needs to be deleted.
    
  \sa
  --------------------------------------------------------------------------*/


#endif //#if !defined( __LIM_SESSION_H )






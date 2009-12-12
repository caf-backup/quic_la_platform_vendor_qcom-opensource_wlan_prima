/*
 * Airgo Networks, Inc proprietary. All rights reserved.
 * This file contains the source code for CFG API functions.
 *
 * Author:      Kevin Nguyen
 * Date:        04/09/02
 * History:-
 * 04/09/02        Created.
 * --------------------------------------------------------------------
 */

#include "palTypes.h"
#include "cfgPriv.h"
#include "cfgDebug.h"

static void Notify(tpAniSirGlobal, tANI_U16, tANI_U32);


// ---------------------------------------------------------------------
tANI_U32 cfgNeedRestart(tpAniSirGlobal pMac, tANI_U16 cfgId)
{
    return !!(pMac->cfg.gCfgEntry[cfgId].control & CFG_CTL_RESTART) ;
}

// ---------------------------------------------------------------------
tANI_U32 cfgNeedReload(tpAniSirGlobal pMac, tANI_U16 cfgId)
{
    return !!(pMac->cfg.gCfgEntry[cfgId].control & CFG_CTL_RELOAD) ;
}

// ---------------------------------------------------------------------
/**
 * wlan_cfgInit()
 *
 * FUNCTION:
 * CFG initialization function.
 *
 * LOGIC:
 * Please see Configuration & Statistic Collection Micro-Architecture
 * specification for the pseudocode.
 *
 * ASSUMPTIONS:
 * None.
 *
 * NOTE:
 * This function must be called during system initialization.
 *
 * @param None
 * @return None.
 */

void
wlan_cfgInit(tpAniSirGlobal pMac)
{
    // Set status to not-ready
    pMac->cfg.gCfgStatus = CFG_INCOMPLETE;

    // Send CFG_DNLD_REQ to host
    PELOGW(cfgLog(pMac, LOGW, FL("Sending CFG_DNLD_REQ\n"));)
    cfgSendHostMsg(pMac, WNI_CFG_DNLD_REQ, WNI_CFG_DNLD_REQ_LEN,
                   WNI_CFG_DNLD_REQ_NUM, 0, 0, 0);

} /*** end wlan_cfgInit() ***/

// ---------------------------------------------------------------------
/**
 * cfgSetInt()
 *
 * FUNCTION:
 * This function is called to update an integer parameter.
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 * - Range checking is performed by the calling function.  In case this
 *   function call is being triggered by a request from host, then host
 *   is responsible for performing range checking before sending the
 *   request.
 *
 * - Host RW permission checking should already be done prior to calling
 *   this function by the message processing function.
 *
 * NOTE:
 *
 * @param cfgId:     16-bit CFG parameter ID
 * @param value:     32-bit unsigned value
 *
 * @return eSIR_SUCCESS       :  request completed successfully \n
 * @return eSIR_CFG_INVALID_ID:  invalid CFG parameter ID \n
 */

tSirRetStatus
cfgSetInt(tpAniSirGlobal pMac, tANI_U16 cfgId, tANI_U32 value)
{
    tANI_U32      index;
    tANI_U32      control, mask;
    tSirRetStatus  retVal;

    if (cfgId >= CFG_PARAM_MAX_NUM)
    {
        PELOGE(cfgLog(pMac, LOGE, FL("Invalid cfg id %d\n"), cfgId);)
        return eSIR_CFG_INVALID_ID;
    }

    control  = pMac->cfg.gCfgEntry[cfgId].control;
    index    = control & CFG_BUF_INDX_MASK;
    retVal   = eSIR_SUCCESS;

    // Check if parameter is valid
    if ((control & CFG_CTL_VALID) == 0)
    {
        PELOGE(cfgLog(pMac, LOGE, FL("Not valid cfg id %d\n"), cfgId);)
        retVal = eSIR_CFG_INVALID_ID;
    }
#if 0
    else if (((control & CFG_CTL_RESTART) && !Restarting(pMac)) ||
             ((control & CFG_CTL_RELOAD) && !Reloading(pMac)))
    {
        cfgLog(pMac, LOGE, FL("Change requires a restart/reload cfg id %d state %d\n"),
               cfgId, pMac->lim.gLimSmeState);
        retVal = eSIR_CFG_INVALID_ID;
    }
#endif
    else if ((pMac->cfg.gCfgIBufMin[index] > value) ||
             (pMac->cfg.gCfgIBufMax[index] < value))
    {
        PELOGE(cfgLog(pMac, LOGE, FL("Value %d out of range [%d,%d] cfg id %d\n"),
               value, pMac->cfg.gCfgIBufMin[index],
               pMac->cfg.gCfgIBufMax[index], cfgId);)
        retVal = eSIR_CFG_INVALID_ID;
    }
    else
    {
            // Write integer value
            pMac->cfg.gCfgIBuf[index] = value;

            // Update hardware if necessary
            mask = control & CFG_CTL_NTF_MASK;
            if ((mask & CFG_CTL_NTF_HW) != 0)
                PELOGE(cfgLog(pMac, LOGE, FL("CFG Notify HW not supported!!!\n"));)

            // Notify other modules if necessary
            if ((mask & CFG_CTL_NTF_MASK) != 0)
                Notify(pMac, cfgId, mask);

    }

    return (retVal);

} /*** end cfgSetInt ***/

// ---------------------------------------------------------------------
/**
 * cfgCheckValid()
 *
 * FUNCTION:
 * This function is called to check if a parameter is valid
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param cfgId:  16-bit CFG parameter ID
 *
 * @return eSIR_SUCCESS:         request completed successfully
 * @return eSIR_CFG_INVALID_ID:  invalid CFG parameter ID
 */

tSirRetStatus
cfgCheckValid(tpAniSirGlobal pMac, tANI_U16 cfgId)
{
    tANI_U32      control;

    if (cfgId >= CFG_PARAM_MAX_NUM)
    {
        PELOG3(cfgLog(pMac, LOG3, FL("Invalid cfg id %d\n"), cfgId);)
        return(eSIR_CFG_INVALID_ID);
    }

    control = pMac->cfg.gCfgEntry[cfgId].control;

    // Check if parameter is valid
    if ((control & CFG_CTL_VALID) == 0)
    {
        PELOG3(cfgLog(pMac, LOG3, FL("Not valid cfg id %d\n"), cfgId);)
        return(eSIR_CFG_INVALID_ID);
    }
    else
        return(eSIR_SUCCESS);

} /*** end cfgCheckValid() ***/

// ---------------------------------------------------------------------
/**
 * wlan_cfgGetInt()
 *
 * FUNCTION:
 * This function is called to read an integer parameter.
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param cfgId:  16-bit CFG parameter ID
 * @param pVal:   address where parameter value will be written
 *
 * @return eSIR_SUCCESS:         request completed successfully
 * @return eSIR_CFG_INVALID_ID:  invalid CFG parameter ID
 */

tSirRetStatus
wlan_cfgGetInt(tpAniSirGlobal pMac, tANI_U16 cfgId, tANI_U32 *pValue)
{
    tANI_U32      index;
    tANI_U32      control;
    tSirRetStatus  retVal;

    if (cfgId >= CFG_PARAM_MAX_NUM)
    {
        PELOGE(cfgLog(pMac, LOGE, FL("Invalid cfg id %d\n"), cfgId);)
        retVal = eSIR_CFG_INVALID_ID;
        return retVal;
    }

    control = pMac->cfg.gCfgEntry[cfgId].control;
    index   = control & CFG_BUF_INDX_MASK;
    retVal  = eSIR_SUCCESS;

    // Check if parameter is valid
    if ((control & CFG_CTL_VALID) == 0)
    {
        PELOGE(cfgLog(pMac, LOGE, FL("Not valid cfg id %d\n"), cfgId);)
        retVal = eSIR_CFG_INVALID_ID;
    }
    else {
        // Get integer value
        if(index < CFG_AP_IBUF_MAX_SIZE)
            *pValue = pMac->cfg.gCfgIBuf[index];
    }

    return (retVal);

} /*** end wlan_cfgGetInt() ***/

#ifdef NOT_CURRENTLY_USED
// ---------------------------------------------------------------------
/**
 * cfgIncrementInt()
 *
 * FUNCTION:
 * This function is called to increment an integer parameter by n.
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 * - No range checking will be performed.
 * - Host RW permission should be checked prior to calling this
 *   function.
 *
 * NOTE:
 *
 * @param cfgId:     16-bit CFG parameter ID
 * @param value:     increment value
 *
 * @return eSIR_SUCCESS:         request completed successfully
 * @return eSIR_CFG_INVALID_ID:  invalid CFG parameter ID
 */

tSirRetStatus
cfgIncrementInt(tpAniSirGlobal pMac, tANI_U16 cfgId, tANI_U32 value)
{
    tANI_U32      index;
    tANI_U32      control;
    tSirRetStatus  retVal;

    if (cfgId >= CFG_PARAM_MAX_NUM)
    {
        PELOGE(cfgLog(pMac, LOGE, FL("Invalid cfg id %d\n"), cfgId);)
        retVal = eSIR_CFG_INVALID_ID;
    }

    control  = pMac->cfg.gCfgEntry[cfgId].control;
    index    = control & CFG_BUF_INDX_MASK;
    retVal   = eSIR_SUCCESS;

    // Check if parameter is valid
    if ((control & CFG_CTL_VALID) == 0)
    {
        PELOGE(cfgLog(pMac, LOGE, FL("Not valid cfg id %d\n"), cfgId);)
        retVal = eSIR_CFG_INVALID_ID;
    }
    else
    {
            // Increment integer value
            pMac->cfg.gCfgIBuf[index] += value;

    }

    return (retVal);
}
#endif // NOT_CURRENTLY_USED

// ---------------------------------------------------------------------
/**
 * cfgSetStr()
 *
 * FUNCTION:
 * This function is called to set a string parameter.
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 * - No length checking will be performed.  Should be done by calling
 *   module.
 * - Host RW permission should be checked prior to calling this
 *   function.
 *
 * NOTE:
 *
 * @param cfgId:     16-bit CFG parameter ID
 * @param pStr:      address of string data
 * @param len:       string length
 *
 * @return eSIR_SUCCESS:         request completed successfully
 * @return eSIR_CFG_INVALID_ID:  invalid CFG parameter ID
 * @return eSIR_CFG_INVALID_LEN: invalid parameter length
 *
 */

tSirRetStatus
cfgSetStr(tpAniSirGlobal pMac, tANI_U16 cfgId, tANI_U8 *pStr, tANI_U32 length)
{
    tANI_U8       *pDst, *pDstEnd;
    tANI_U32      index, paramLen, control, mask;
    tSirRetStatus  retVal;

    if (cfgId >= CFG_PARAM_MAX_NUM)
    {
        PELOGE(cfgLog(pMac, LOGE, FL("Invalid cfg id %d\n"), cfgId);)
        return eSIR_CFG_INVALID_ID;
    }

    control  = pMac->cfg.gCfgEntry[cfgId].control;
    index    = control & CFG_BUF_INDX_MASK;
    retVal   = eSIR_SUCCESS;

    // Check if parameter is valid
    if ((control & CFG_CTL_VALID) == 0)
    {
        PELOGE(cfgLog(pMac, LOGE, FL("Invalid cfg id %d\n"), cfgId);)
        retVal = eSIR_CFG_INVALID_ID;
    }
    else if (index >= (sizeof(pMac->cfg.gCfgSBuf)/sizeof(pMac->cfg.gCfgSBuf[0])))
    {
        PELOGE(cfgLog(pMac, LOGE, FL("Invalid Sbuf index %d (max %d)\n"),
               index, (sizeof(pMac->cfg.gCfgSBuf)/sizeof(pMac->cfg.gCfgSBuf[0])));)
        retVal = eSIR_CFG_INVALID_ID;
    }
    else
    {
            pDst = &pMac->cfg.gCfgSBuf[index];
            paramLen = *pDst++;
            if (length > paramLen)
            {
                PELOGE(cfgLog(pMac, LOGE, FL("Invalid length %d (>%d) cfg id %d\n"),
                       length, paramLen, cfgId);)
                retVal = eSIR_CFG_INVALID_LEN;
            }
            else
            {
                *pDst++ = (tANI_U8)length;
                pDstEnd = pDst + length;
                while (pDst < pDstEnd)
                    *pDst++ = *pStr++;

                // Update hardware if necessary
                mask = control & CFG_CTL_NTF_MASK;
                if ((mask & CFG_CTL_NTF_HW) != 0)
                    PELOGE(cfgLog(pMac, LOGE, FL("CFG Notify HW not supported!!!\n"));)

                // Notify other modules if necessary
                if ((mask & CFG_CTL_NTF_MASK) != 0)
                    Notify(pMac, cfgId, mask);
            }

        }

    return (retVal);

} /*** end cfgSetStr() ***/

// ---------------------------------------------------------------------
/**
 * wlan_cfgGetStr()
 *
 * FUNCTION:
 * This function is called to get a string parameter.
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 * - Host RW permission should be checked prior to calling this
 *   function.
 *
 * NOTE:
 *
 * @param cfgId:     16-bit CFG parameter ID
 * @param pBuf:      address of string buffer
 * @param pLen:      address of max buffer length
 *                   actual length will be returned at this address
 *
 * @return eSIR_SUCCESS:         request completed successfully
 * @return eSIR_CFG_INVALID_ID:  invalid CFG parameter ID
 * @return eSIR_CFG_INVALID_LEN: invalid parameter length
 *
 */

tSirRetStatus
wlan_cfgGetStr(tpAniSirGlobal pMac, tANI_U16 cfgId, tANI_U8 *pBuf, tANI_U32 *pLength)
{
    tANI_U8             *pSrc, *pSrcEnd;
    tANI_U32            index, control;
    tSirRetStatus  retVal;

    if (cfgId >= CFG_PARAM_MAX_NUM)
    {
        PELOGE(cfgLog(pMac, LOGE, FL("Invalid cfg id %d\n"), cfgId);)
        retVal = eSIR_CFG_INVALID_ID;
        return retVal;
    }

    control  = pMac->cfg.gCfgEntry[cfgId].control;
    index    = control & CFG_BUF_INDX_MASK;
    retVal   = eSIR_SUCCESS;

    // Check if parameter is valid
    if ((control & CFG_CTL_VALID) == 0)
    {
        PELOGE(cfgLog(pMac, LOGE, FL("Not valid cfg id %d\n"), cfgId);)
        retVal = eSIR_CFG_INVALID_ID;
    }
    else
    {
        // Get string
        pSrc  = &pMac->cfg.gCfgSBuf[index];
        pSrc++;                               // skip over max length
        if (*pLength < *pSrc)
        {
            PELOGE(cfgLog(pMac, LOGE, FL("Invalid length %d (<%d) cfg id %d\n"),
                   *pLength, *pSrc, cfgId);)
            retVal = eSIR_CFG_INVALID_LEN;
        }
        else
        {
            *pLength = *pSrc++;               // save parameter length
            pSrcEnd = pSrc + *pLength;
            while (pSrc < pSrcEnd)
                *pBuf++ = *pSrc++;
        }
    }

    return (retVal);

} /*** end wlan_cfgGetStr() ***/

// ---------------------------------------------------------------------
/**
 * wlan_cfgGetStrMaxLen()
 *
 * FUNCTION:
 * This function is called to get a string maximum length.
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 * - Host RW permission should be checked prior to calling this
 *   function.
 *
 * NOTE:
 *
 * @param cfgId:     16-bit CFG parameter ID
 * @param pLen:      maximum length will be returned at this address
 *
 * @return eSIR_SUCCESS:         request completed successfully
 * @return eSIR_CFG_INVALID_ID:  invalid CFG parameter ID
 *
 */

tSirRetStatus
wlan_cfgGetStrMaxLen(tpAniSirGlobal pMac, tANI_U16 cfgId, tANI_U32 *pLength)
{
    tANI_U32            index, control;
    tSirRetStatus  retVal;

    if (cfgId >= CFG_PARAM_MAX_NUM)
    {
        PELOGE(cfgLog(pMac, LOGE, FL("Invalid cfg id %d\n"), cfgId);)
        retVal = eSIR_CFG_INVALID_ID;
    }

    control  = pMac->cfg.gCfgEntry[cfgId].control;
    index    = control & CFG_BUF_INDX_MASK;
    retVal   = eSIR_SUCCESS;

    // Check if parameter is valid
    if ((control & CFG_CTL_VALID) == 0)
    {
        PELOGE(cfgLog(pMac, LOGE, FL("Not valid cfg id %d\n"), cfgId);)
        retVal = eSIR_CFG_INVALID_ID;
    }
    else
    {
        *pLength = pMac->cfg.gCfgSBuf[index];
    }

    return (retVal);

} /*** end wlan_cfgGetStrMaxLen() ***/

// ---------------------------------------------------------------------
/**
 * wlan_cfgGetStrLen()
 *
 * FUNCTION:
 * This function is called to get a string length.
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 * - Host RW permission should be checked prior to calling this
 *   function.
 *
 * NOTE:
 *
 * @param cfgId:     16-bit CFG parameter ID
 * @param pLen:      current length will be returned at this address
 *
 * @return eSIR_SUCCESS:         request completed successfully
 * @return eSIR_CFG_INVALID_ID:  invalid CFG parameter ID
 *
 */

tSirRetStatus
wlan_cfgGetStrLen(tpAniSirGlobal pMac, tANI_U16 cfgId, tANI_U32 *pLength)
{
    tANI_U32            index, control;
    tSirRetStatus  retVal;

    if (cfgId >= CFG_PARAM_MAX_NUM)
    {
        PELOGE(cfgLog(pMac, LOGE, FL("Invalid cfg id %d\n"), cfgId);)
        retVal = eSIR_CFG_INVALID_ID;
    }

    control  = pMac->cfg.gCfgEntry[cfgId].control;
    index    = control & CFG_BUF_INDX_MASK;
    retVal   = eSIR_SUCCESS;

    // Check if parameter is valid
    if ((control & CFG_CTL_VALID) == 0)
    {
        PELOGE(cfgLog(pMac, LOGE, FL("Not valid cfg id %d\n"), cfgId);)
        retVal = eSIR_CFG_INVALID_ID;
    }
    else
    {
        *pLength = pMac->cfg.gCfgSBuf[index+1];
    }

    return (retVal);

} /*** end wlan_cfgGetStrLen() ***/



/*-------------------------------------------------------------
\fn     cfgGetDot11dTransmitPower
\brief  This function returns the regulatory max transmit power
\param  pMac
\return tPowerdBm - Power
\-------------------------------------------------------------*/
static tPowerdBm
cfgGetDot11dTransmitPower(tpAniSirGlobal pMac, tANI_U16   cfgId,
                                        tANI_U32 cfgLength, tANI_U8 channel)
{
    tANI_U8    *pCountryInfo = NULL;
    tANI_U8    count = 0;
    tPowerdBm  maxTxPwr = HAL_MAX_TXPOWER_INVALID;
    eHalStatus    status;
    
    /* At least one element is present */
    if(cfgLength < sizeof(tSirMacChanInfo))
    {
        PELOGE(cfgLog(pMac, LOGE, FL("Invalid CFGLENGTH %d while getting 11d txpower"), cfgLength);)
        goto error;
    }

    status = palAllocateMemory(pMac->hHdd, (void **)&pCountryInfo, cfgLength);
    if (status != eHAL_STATUS_SUCCESS)
    {
        cfgLog(pMac, LOGP, FL(" palAllocateMemory() failed, status = %d"), status);
        goto error;
    }
    /* The CSR will always update this CFG. The contents will be from country IE if regulatory domain
     * is enabled on AP else will contain EEPROM contents
     */
    if (wlan_cfgGetStr(pMac, cfgId, pCountryInfo, &cfgLength) != eSIR_SUCCESS)
    {
        palFreeMemory(pMac->hHdd, pCountryInfo);
        pCountryInfo = NULL;
            
        cfgLog(pMac, LOGP, FL("Failed to retrieve 11d configuration parameters while retrieving 11d tuples"));
        goto error;
    }
    /* Identify the channel and maxtxpower */
    while(count <= (cfgLength - (sizeof(tSirMacChanInfo))))
    {
        tANI_U8    firstChannel, maxChannels;

        firstChannel = pCountryInfo[count++];
        maxChannels = pCountryInfo[count++];
        maxTxPwr = pCountryInfo[count++];

        if((channel >= firstChannel) && 
            (channel < (firstChannel + maxChannels)))
        {
            break;
        }
    }

error:
    if(NULL != pCountryInfo)
        palFreeMemory(pMac->hHdd, pCountryInfo);
       
    return maxTxPwr;
}


/**----------------------------------------------------------------------
\fn     cfgGetRegulatoryMaxTransmitPower

\brief  Gets regulatory tx power on the current channel.

\param  pMac
\param  channel
\param  rfBand
 -----------------------------------------------------------------------*/
tPowerdBm cfgGetRegulatoryMaxTransmitPower(tpAniSirGlobal pMac, tANI_U8 channel)
{
    tANI_U32    cfgLength = 0;
    tANI_U16    cfgId = 0;
    tPowerdBm  maxTxPwr;
    eRfBandMode rfBand = eRF_BAND_UNKNOWN;

    if ((channel >= SIR_11A_CHANNEL_BEGIN) &&
        (channel <= SIR_11A_CHANNEL_END))
        rfBand = eRF_BAND_5_GHZ;
    else
        rfBand = eRF_BAND_2_4_GHZ;

    
    /* Get the max transmit power for current channel for the current regulatory domain */
    switch (rfBand)
    {
        case eRF_BAND_2_4_GHZ:
            cfgId = WNI_CFG_MAX_TX_POWER_2_4;
            cfgLength = WNI_CFG_MAX_TX_POWER_2_4_LEN;
            PELOG2(cfgLog(pMac, LOG2, FL("HAL: Reading CFG for 2.4 GHz channels to get regulatory max tx power"));)
            break;

        case eRF_BAND_5_GHZ:
            cfgId = WNI_CFG_MAX_TX_POWER_5;
            cfgLength = WNI_CFG_MAX_TX_POWER_5_LEN;
            PELOG2(cfgLog(pMac, LOG2, FL("HAL: Reading CFG for 5.0 GHz channels to get regulatory max tx power"));)
            break;

        case eRF_BAND_UNKNOWN:
        default:
            PELOG2(cfgLog(pMac, LOG2, FL("HAL: Invalid current working band for the device"));)
            return HAL_MAX_TXPOWER_INVALID; //Its return, not break.
    }

    maxTxPwr = cfgGetDot11dTransmitPower(pMac, cfgId, cfgLength, channel);

    return (maxTxPwr);
}

// ---------------------------------------------------------------------
/**
 * cfgGetCapabilityInfo
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

tSirRetStatus
cfgGetCapabilityInfo(tpAniSirGlobal pMac, tANI_U16 *pCap)
{
    tANI_U32 val = 0;
    tpSirMacCapabilityInfo pCapInfo;

    *pCap = 0;
    pCapInfo = (tpSirMacCapabilityInfo) pCap;

    if (limGetSystemRole(pMac) == eLIM_STA_IN_IBSS_ROLE)
        pCapInfo->ibss = 1; // IBSS bit
    else if (limGetSystemRole(pMac) == eLIM_AP_ROLE ||
             limGetSystemRole(pMac) == eLIM_STA_ROLE)
        pCapInfo->ess = 1; // ESS bit
    else
        cfgLog(pMac, LOGP, FL("can't get capability, role is UNKNOWN!!\n"));

#if (WNI_POLARIS_FW_PRODUCT == AP)
    if (limGetSystemRole(pMac) == eLIM_AP_ROLE)
    {
        // CF-pollable bit
        if (wlan_cfgGetInt(pMac, WNI_CFG_CF_POLLABLE, &val) != eSIR_SUCCESS)
        {
            cfgLog(pMac, LOGP, FL("cfg get WNI_CFG_CF_POLLABLE failed\n"));
            return eSIR_FAILURE;
        }
        if (val)
            pCapInfo->cfPollable = 1;

        // CF-poll request bit
        if (wlan_cfgGetInt(pMac, WNI_CFG_CF_POLL_REQUEST, &val) != eSIR_SUCCESS)
        {
            cfgLog(pMac, LOGP, FL("cfg get WNI_CFG_CF_POLL_REQUEST failed\n"));
            return eSIR_FAILURE;
        }
        if (val)
            pCapInfo->cfPollReq = 1;
    }
#endif

    // PRIVACY bit
    if (wlan_cfgGetInt(pMac, WNI_CFG_PRIVACY_ENABLED, &val) != eSIR_SUCCESS)
    {
        cfgLog(pMac, LOGP, FL("cfg get WNI_CFG_PRIVACY_ENABLED failed\n"));
        return eSIR_FAILURE;
    }
    if (val)
        pCapInfo->privacy = 1;

    // Short preamble bit
    if (wlan_cfgGetInt(pMac, WNI_CFG_SHORT_PREAMBLE, &val) != eSIR_SUCCESS)
    {
        cfgLog(pMac, LOGP, FL("cfg get WNI_CFG_SHORT_PREAMBLE failed\n"));
        return eSIR_FAILURE;
    }
    if (val)
        pCapInfo->shortPreamble = 1;


    // PBCC bit
    pCapInfo->pbcc = 0;

    // Channel agility bit
    pCapInfo->channelAgility = 0;
    //If STA/AP operating in 11B mode, don't set rest of the capability info bits.
    if(pMac->lim.gLimDot11Mode == WNI_CFG_DOT11_MODE_11B)
        return eSIR_SUCCESS;


    
    // Short slot time bit
    if (pMac->lim.gLimSystemRole == eLIM_AP_ROLE)
    {
        if (wlan_cfgGetInt(pMac, WNI_CFG_SHORT_SLOT_TIME, &val)
                       != eSIR_SUCCESS)
        {
            cfgLog(pMac, LOGP,
                   FL("cfg get WNI_CFG_SHORT_SLOT_TIME failed\n"));
            return eSIR_FAILURE;
        }
        if (val)
            pCapInfo->shortSlotTime = 1;
    }
    else
    {
        if (wlan_cfgGetInt(pMac, WNI_CFG_11G_SHORT_SLOT_TIME_ENABLED, &val)
                       != eSIR_SUCCESS)
        {
            cfgLog(pMac, LOGP,
                   FL("cfg get WNI_CFG_11G_SHORT_SLOT_TIME failed\n"));
            return eSIR_FAILURE;
        }
        if (val)
            pCapInfo->shortSlotTime = 1;
    }


    // Spectrum Management bit
    if((eLIM_STA_IN_IBSS_ROLE != pMac->lim.gLimSystemRole) &&
            pMac->lim.gLim11hEnable )
    {
      if (wlan_cfgGetInt(pMac, WNI_CFG_11H_ENABLED, &val) != eSIR_SUCCESS)
      {
          cfgLog(pMac, LOGP, FL("cfg get WNI_CFG_11H_ENABLED failed\n"));
          return eSIR_FAILURE;
      }
      if (val)
          pCapInfo->spectrumMgt = 1;
    }

    // QoS bit
    if (wlan_cfgGetInt(pMac, WNI_CFG_QOS_ENABLED, &val) != eSIR_SUCCESS)
    {
        cfgLog(pMac, LOGP, FL("cfg get WNI_CFG_QOS_ENABLED failed\n"));
        return eSIR_FAILURE;
    }
    if (val)
        pCapInfo->qos = 1;

    // APSD bit
    if (wlan_cfgGetInt(pMac, WNI_CFG_APSD_ENABLED, &val) != eSIR_SUCCESS)
    {
        cfgLog(pMac, LOGP, FL("cfg get WNI_CFG_APSD_ENABLED failed\n"));
        return eSIR_FAILURE;
    }
    if (val)
        pCapInfo->apsd = 1;

    //DSSS-OFDM
    //FIXME : no config defined yet. 
    
    // Block ack bit
    if (wlan_cfgGetInt(pMac, WNI_CFG_BLOCK_ACK_ENABLED, &val) != eSIR_SUCCESS)
    {
        cfgLog(pMac, LOGP, FL("cfg get WNI_CFG_BLOCK_ACK_ENABLED failed\n"));
        return eSIR_FAILURE;
    }
    pCapInfo->delayedBA = (tANI_U16)((val >> WNI_CFG_BLOCK_ACK_ENABLED_DELAYED) & 1);
    pCapInfo->immediateBA = (tANI_U16)((val >> WNI_CFG_BLOCK_ACK_ENABLED_IMMEDIATE) & 1);

    return eSIR_SUCCESS;
}

// --------------------------------------------------------------------
/**
 * cfgSetCapabilityInfo
 *
 * FUNCTION:
 * This function is called on BP based on the capabilities
 * received in SME_JOIN/REASSOC_REQ message.
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE: 1. ESS/IBSS capabilities are based on system role.
 *       2. Since PBCC, Channel agility and Extended capabilities
 *          are not supported, they're not set at CFG
 *
 * @param  pMac   Pointer to global MAC structure
 * @param  caps   16-bit Capability Info field
 * @return None
 */

void
cfgSetCapabilityInfo(tpAniSirGlobal pMac, tANI_U16 caps)
{
#if (defined(ANI_PRODUCT_TYPE_AP))

    if (cfgSetInt(pMac, WNI_CFG_PRIVACY_ENABLED,
                  SIR_MAC_GET_PRIVACY(caps)) != eSIR_SUCCESS)
        cfgLog(pMac, LOGP, FL("could not set privacy at CFG\n"));

    if (cfgSetInt(pMac, WNI_CFG_SHORT_PREAMBLE,
                  SIR_MAC_GET_SHORT_PREAMBLE(caps)) != eSIR_SUCCESS)
        cfgLog(pMac, LOGP, FL("could not set short preamble at CFG\n"));

    if (cfgSetInt(pMac, WNI_CFG_QOS_ENABLED,
                  SIR_MAC_GET_QOS(caps)) != eSIR_SUCCESS)
        cfgLog(pMac, LOGP, FL("could not set qos at CFG\n"));

    if (cfgSetInt(pMac, WNI_CFG_11G_SHORT_SLOT_TIME_ENABLED,
                  SIR_MAC_GET_SHORT_SLOT_TIME(caps)) != eSIR_SUCCESS)
        cfgLog(pMac, LOGP, FL("could not set short slot time at CFG\n"));

    if (cfgSetInt(pMac, WNI_CFG_APSD_ENABLED,
                  SIR_MAC_GET_APSD(caps)) != eSIR_SUCCESS)
        cfgLog(pMac, LOGP, FL("could not set apsd at CFG\n"));

    if (cfgSetInt(pMac, WNI_CFG_BLOCK_ACK_ENABLED,
                  SIR_MAC_GET_BLOCK_ACK(caps)) != eSIR_SUCCESS)
        cfgLog(pMac, LOGP, FL("could not set BlockAck at CFG\n"));
#endif
}


// ---------------------------------------------------------------------
/**
 * cfgCleanup()
 *
 * FUNCTION:
 * CFG cleanup function.
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 * None.
 *
 * NOTE:
 * This function must be called during system shutdown.
 *
 * @param None
 *
 * @return None.
 *
 */

void
cfgCleanup(tpAniSirGlobal pMac)
{
    // Set status to not-ready
    pMac->cfg.gCfgStatus = CFG_INCOMPLETE;

} /*** end CfgCleanup() ***/

// ---------------------------------------------------------------------
/**
 * Notify()
 *
 * FUNCTION:
 * This function is called to notify other modules of parameter update.
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param cfgId:    configuration parameter ID
 * @param mask:     notification mask
 *
 * @return None.
 *
 */

static void
Notify(tpAniSirGlobal pMac, tANI_U16 cfgId, tANI_U32 ntfMask)
{

    tSirMsgQ    mmhMsg;

    mmhMsg.type = SIR_CFG_PARAM_UPDATE_IND;
    mmhMsg.bodyval = (tANI_U32)cfgId;
    mmhMsg.bodyptr = NULL;

    MTRACE(macTraceMsgTx(pMac, 0, mmhMsg.type));

    if ((ntfMask & CFG_CTL_NTF_SCH) != 0)
        schPostMessage(pMac, &mmhMsg);

    if ((ntfMask & CFG_CTL_NTF_LIM) != 0)
        limPostMsgApi(pMac, &mmhMsg);

    if ((ntfMask & CFG_CTL_NTF_HAL) != 0)
        halPostMsgApi(pMac, &mmhMsg);

    // Notify ARQ

} /*** end Notify() ***/

// ---------------------------------------------------------------------








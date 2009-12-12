/*******************************************************************
 *
 *    DESCRIPTION:  This module encapsulates necessary Vos interactions
 *                  so that they are not directly present in source
 *                  that is also built for the WinCE SDIO driver.
 *                  In the WinCE SDIO driver, these will have different implemenation.
 *
 *    AUTHOR:
 *
 *    HISTORY:
 *
 *******************************************************************/

#include "sys_api.h"
#include "halFw.h"
#include "halPhyVos.h"

eHalStatus halPhy_VosEventInit(tHalHandle hHal)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal) hHal;

    //intialize the setChan event for wait blocking around halPhySetChannel
    if (!VOS_IS_STATUS_SUCCESS( vos_event_init(&pMac->hphy.setChanEvent) ))
    {
        phyLog(pMac, LOGE, "ERROR: setChan vos event init failed!!\n");
        return eHAL_STATUS_FAILURE;
    }
    else
    {
        return (eHAL_STATUS_SUCCESS);
    }
}

eHalStatus halPhy_VosEventDestroy(tHalHandle hHal)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal) hHal;

    //destroy the setChan event for wait blocking around halPhySetChannel
    if (!VOS_IS_STATUS_SUCCESS( vos_event_destroy(&pMac->hphy.setChanEvent) ))
    {
        phyLog(pMac, LOGE, "ERROR: setChan vos event destroy failed!!\n");
        return eHAL_STATUS_FAILURE;
    }
    else
    {
        return (eHAL_STATUS_SUCCESS);
    }
}

#define VOS_EVENT_SET_CHAN_TIME_OUT 2500
eHalStatus halPhy_VosEventWaitSetChannel(tHalHandle hHal)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal) hHal;
    tANI_U8 eventIdx = 0;

    //Wait till the host receives setChannel rsp from fw
    if (!VOS_IS_STATUS_SUCCESS( vos_wait_events(&pMac->hphy.setChanEvent, 1,
                                VOS_EVENT_SET_CHAN_TIME_OUT, &eventIdx) ))
    {
        phyLog(pMac, LOGE, "ERROR: setChan vos events wait failed!!\n");
        return eHAL_STATUS_FAILURE;
    }
    else
    {
        return (eHAL_STATUS_SUCCESS);
    }

}

eHalStatus halPhy_VosEventResetSetChannel(tHalHandle hHal)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal) hHal;

    //Reset the set channel event
    if (!VOS_IS_STATUS_SUCCESS( vos_event_reset(&pMac->hphy.setChanEvent) ))
    {
        phyLog(pMac, LOGE, "ERROR: setChan vos_event_reset failed!!\n");
        return eHAL_STATUS_FAILURE;
    }
    else
    {
        return (eHAL_STATUS_SUCCESS);
    }

}


eHalStatus halPhy_HandlerFwRspMsg(tHalHandle hHal, void* pFwMsg)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
    tpAniSirGlobal pMac = (tpAniSirGlobal) hHal;
    tMBoxMsgHdr *pMsgHdr = (tMBoxMsgHdr*)pFwMsg;

    // Handle the type of FW message received
    switch(pMsgHdr->MsgType) {

        case QWLANFW_FW2HOST_CAL_UPDATE_RSP:
        {
            Qwlanfw_CalUpdateRspType *calUpdateRsp = (Qwlanfw_CalUpdateRspType *)pFwMsg;
            if(calUpdateRsp->uStatus != eHAL_STATUS_SUCCESS)
            {
                phyLog(pMac, LOGE, "ERROR: Update calibration Failed in firmware!!\n");
            }
            break;
        }

        case QWLANFW_FW2HOST_SET_CHANNEL_RSP:
        {
#ifdef  ANI_MANF_DIAG
            Qwlanfw_SetChannelRspType *setChanRsp = (Qwlanfw_SetChannelRspType *)pFwMsg;
            if(setChanRsp->uStatus == eHAL_STATUS_SUCCESS)
            {
                pMac->hphy.fwSetChannelStatus = eHAL_STATUS_SUCCESS;
            }
            //set the event waiting on  pMac->hphy.setChanEvent
            if (!VOS_IS_STATUS_SUCCESS( vos_event_set(&pMac->hphy.setChanEvent) ))
            {
                phyLog(pMac, LOGE, "ERROR: setChan vos events set failed!!\n");
                pMac->hphy.fwSetChannelStatus = eHAL_STATUS_FAILURE;
                retVal = eHAL_STATUS_FAILURE;
            }            
#else
            halPhy_HandleSetChannelRsp(hHal, pFwMsg);
#endif

            break;
        }

        case QWLANFW_FW2HOST_SET_CHAIN_SELECT_RSP:
        {
            Qwlanfw_SetChainSelectRspType *chainSelectRsp = (Qwlanfw_SetChainSelectRspType *)pFwMsg;
            if(chainSelectRsp->uStatus == eHAL_STATUS_SUCCESS)
            {
                pMac->hphy.phy.activeChains = pMac->hphy.phy.cfgChains;
            }
            else
            {
                //try to set it again?
                phyLog(pMac, LOGE, "ERROR: halPhySetChainSelect Failed in firmware!!\n");
            }
            break;
        }

        default:
            retVal = eHAL_STATUS_FW_MSG_INVALID;
            break;
    }
    return (retVal);
}


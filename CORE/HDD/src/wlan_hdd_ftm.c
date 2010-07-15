/**========================================================================

  \file  wlan_hdd_ftm.c

  \brief This file contains the WLAN factory test mode implementation

   Copyright 2008 (c) Qualcomm, Incorporated.  All Rights Reserved.

   Qualcomm Confidential and Proprietary.

  ========================================================================*/

/**=========================================================================

                       EDIT HISTORY FOR FILE


  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.


  $Header:$   $DateTime: $ $Author: $


  when        who    what, where, why
  --------    ---    --------------------------------------------------------
  04/5/09     Shailender     Created module.

  ==========================================================================*/




#include <vos_mq.h>
#include "vos_sched.h"
#include <vos_api.h>
#include "sirTypes.h"
#include "halTypes.h"
#include "sirApi.h"
#include "sirMacProtDef.h"
#include "sme_Api.h"
#include "macInitApi.h"
#include "wlan_qct_sal.h"
#include "wlan_qct_bal.h"
#include "wlan_qct_sys.h"
#include "wlan_qct_tl.h"
#include "wlan_hdd_misc.h"
#include "i_vos_packet.h"
#include "vos_nvitem.h"
#include "wlan_hdd_main.h"
#include "vos_power.h"
#include "ani_assert.h"
#include "sys_api.h"
#include "pttModuleApi.h"
#include "qwlan_version.h"

#ifdef ANI_MANF_DIAG
#define RXMODE_DISABLE_ALL 0
#define RXMODE_ENABLE_ALL  1
#define RXMODE_ENABLE_11GN 2
#define RXMODE_ENABLE_11B  3

#define FTM_CHAIN_SEL_NO_RX_TX      0
#define FTM_CHAIN_SEL_R0R1_ON       1
#define FTM_CHAIN_SEL_R0_ON         2
#define FTM_CHAIN_SEL_R1_ON         3
#define FTM_CHAIN_SEL_T0_ON         4
#define FTM_CHAIN_SEL_MAX           4
#define FTM_CHAIN_SEL_R0R1_T0_ON    5


extern const sHalNv nvDefaults;

static void wlan_ftm_register_wext(hdd_adapter_t *pAdapter);

static const freq_chan_t  freq_chan_tbl[] = { 
     {2412, 1}, {2417, 2},{2422, 3}, {2427, 4}, {2432, 5}, {2437, 6}, {2442, 7}, 
     {2447, 8}, {2452, 9},{2457, 10},{2462, 11},{2467 ,12},{2472, 13},{2484, 14}
};

static rateStr2rateIndex_t rateName_rateIndex_tbl[] = 
{
   { HAL_PHY_RATE_11B_LONG_1_MBPS,       "11B_LONG_1_MBPS"},
   { HAL_PHY_RATE_11B_LONG_2_MBPS,       "11B_LONG_2_MBPS"},
   { HAL_PHY_RATE_11B_LONG_5_5_MBPS,     "11B_LONG_5_5_MBPS"},
   { HAL_PHY_RATE_11B_LONG_11_MBPS,      "11B_LONG_11_MBPS"},
   { HAL_PHY_RATE_11B_SHORT_2_MBPS,      "11B_SHORT_2_MBPS"},
   { HAL_PHY_RATE_11B_SHORT_5_5_MBPS,    "11B_SHORT_5_5_MBPS"},
   { HAL_PHY_RATE_11B_SHORT_11_MBPS,     "11B_SHORT_11_MBPS"},
   //Spica_Virgo 11A 20MHz Rates
   { HAL_PHY_RATE_11A_6_MBPS,            "11A_6_MBPS"},
   { HAL_PHY_RATE_11A_9_MBPS,            "11A_9_MBPS"},
   { HAL_PHY_RATE_11A_12_MBPS,           "11A_12_MBPS"},
   { HAL_PHY_RATE_11A_18_MBPS,           "11A_18_MBPS"},
   { HAL_PHY_RATE_11A_24_MBPS,           "11A_24_MBPS"},
   { HAL_PHY_RATE_11A_36_MBPS,           "11A_36_MBPS"},
   { HAL_PHY_RATE_11A_48_MBPS,           "11A_48_MBPS"},
   { HAL_PHY_RATE_11A_54_MBPS,           "11A_54_MBPS"},

  //MCS Index #0-15 (20MHz)
   { HAL_PHY_RATE_MCS_1NSS_6_5_MBPS,   "MCS_6_5_MBPS"},
   { HAL_PHY_RATE_MCS_1NSS_13_MBPS,    "MCS_13_MBPS"},
   { HAL_PHY_RATE_MCS_1NSS_19_5_MBPS,  "MCS_19_5_MBPS"},
   { HAL_PHY_RATE_MCS_1NSS_26_MBPS,    "MCS_26_MBPS"},
   { HAL_PHY_RATE_MCS_1NSS_39_MBPS,    "MCS_39_MBPS"},
   { HAL_PHY_RATE_MCS_1NSS_52_MBPS,    "MCS_52_MBPS"},
   { HAL_PHY_RATE_MCS_1NSS_58_5_MBPS,  "MCS_58_5_MBPS"}, 
   { HAL_PHY_RATE_MCS_1NSS_65_MBPS,    "MCS_65_MBPS"},
   { HAL_PHY_RATE_MCS_1NSS_MM_SG_72_2_MBPS, "MCS_72_2_MBPS"}
};

static rateIndex2Preamble_t rate_index_2_preamble_table[] = 
{ 

   { HAL_PHY_RATE_11B_LONG_1_MBPS,       PHYDBG_PREAMBLE_LONGB},
   { HAL_PHY_RATE_11B_LONG_2_MBPS,       PHYDBG_PREAMBLE_LONGB},
   { HAL_PHY_RATE_11B_LONG_5_5_MBPS,     PHYDBG_PREAMBLE_LONGB},
   { HAL_PHY_RATE_11B_LONG_11_MBPS,      PHYDBG_PREAMBLE_LONGB},
   { HAL_PHY_RATE_11B_SHORT_2_MBPS,      PHYDBG_PREAMBLE_SHORTB},
   { HAL_PHY_RATE_11B_SHORT_5_5_MBPS,    PHYDBG_PREAMBLE_SHORTB},
   { HAL_PHY_RATE_11B_SHORT_11_MBPS,     PHYDBG_PREAMBLE_SHORTB},

    //SLR Rates
   { HAL_PHY_RATE_SLR_0_25_MBPS,        PHYDBG_PREAMBLE_NOT_SUPPORTED},
   { HAL_PHY_RATE_SLR_0_5_MBPS,         PHYDBG_PREAMBLE_NOT_SUPPORTED},

   //Spica_Virgo 11A 20MHz Rates
   { HAL_PHY_RATE_11A_6_MBPS,           PHYDBG_PREAMBLE_OFDM},
   { HAL_PHY_RATE_11A_9_MBPS,           PHYDBG_PREAMBLE_OFDM},
   { HAL_PHY_RATE_11A_12_MBPS,          PHYDBG_PREAMBLE_OFDM},
   { HAL_PHY_RATE_11A_18_MBPS,          PHYDBG_PREAMBLE_OFDM},
   { HAL_PHY_RATE_11A_24_MBPS,          PHYDBG_PREAMBLE_OFDM},
   { HAL_PHY_RATE_11A_36_MBPS,          PHYDBG_PREAMBLE_OFDM},
   { HAL_PHY_RATE_11A_48_MBPS,          PHYDBG_PREAMBLE_OFDM},
   { HAL_PHY_RATE_11A_54_MBPS,          PHYDBG_PREAMBLE_OFDM},

  //MCS Index #0-15 (20MHz)
   { HAL_PHY_RATE_MCS_1NSS_6_5_MBPS,   PHYDBG_PREAMBLE_MIXED},
   { HAL_PHY_RATE_MCS_1NSS_13_MBPS,    PHYDBG_PREAMBLE_MIXED},
   { HAL_PHY_RATE_MCS_1NSS_19_5_MBPS,  PHYDBG_PREAMBLE_MIXED},
   { HAL_PHY_RATE_MCS_1NSS_26_MBPS,    PHYDBG_PREAMBLE_MIXED},
   { HAL_PHY_RATE_MCS_1NSS_39_MBPS,    PHYDBG_PREAMBLE_MIXED},
   { HAL_PHY_RATE_MCS_1NSS_52_MBPS,    PHYDBG_PREAMBLE_MIXED},
   { HAL_PHY_RATE_MCS_1NSS_58_5_MBPS,  PHYDBG_PREAMBLE_MIXED}, 
   { HAL_PHY_RATE_MCS_1NSS_65_MBPS,    PHYDBG_PREAMBLE_MIXED},
   { HAL_PHY_RATE_MCS_1NSS_MM_SG_7_2_MBPS, PHYDBG_PREAMBLE_NOT_SUPPORTED},
   { HAL_PHY_RATE_MCS_1NSS_MM_SG_14_4_MBPS,PHYDBG_PREAMBLE_NOT_SUPPORTED},
   { HAL_PHY_RATE_MCS_1NSS_MM_SG_21_7_MBPS,PHYDBG_PREAMBLE_NOT_SUPPORTED},
   { HAL_PHY_RATE_MCS_1NSS_MM_SG_28_9_MBPS,PHYDBG_PREAMBLE_NOT_SUPPORTED},
   { HAL_PHY_RATE_MCS_1NSS_MM_SG_43_3_MBPS,PHYDBG_PREAMBLE_NOT_SUPPORTED},
   { HAL_PHY_RATE_MCS_1NSS_MM_SG_57_8_MBPS,PHYDBG_PREAMBLE_NOT_SUPPORTED},
   { HAL_PHY_RATE_MCS_1NSS_MM_SG_65_MBPS, PHYDBG_PREAMBLE_NOT_SUPPORTED},
   { HAL_PHY_RATE_MCS_1NSS_MM_SG_72_2_MBPS, PHYDBG_PREAMBLE_MIXED},
};

typedef struct
{
    tANI_BOOLEAN frameGenEnabled;
    tANI_BOOLEAN wfmEnabled;
    sPttFrameGenParams frameParams;
    v_U16_t txpower;
    v_U16_t rxmode;
    v_U16_t chainSelect;

} FTM_STATUS ;
static FTM_STATUS ftm_status;

//tpAniSirGlobal pMac;

static void _ftm_status_init(void)
{
    tANI_U8 addr1[ANI_MAC_ADDR_SIZE] = { 0x00, 0x11, 0x11, 0x11, 0x11, 0x11 };   //dest
    tANI_U8 addr2[ANI_MAC_ADDR_SIZE] = { 0x00, 0x22, 0x22, 0x22, 0x22, 0x22 };   //sour
    tANI_U8 addr3[ANI_MAC_ADDR_SIZE] = { 0x00, 0x33, 0x33, 0x33, 0x33, 0x33 };   //bssId

    ftm_status.wfmEnabled = eANI_BOOLEAN_FALSE;
    ftm_status.frameGenEnabled = eANI_BOOLEAN_FALSE;
    ftm_status.frameParams.numTestPackets = 0;   //Continuous
    ftm_status.frameParams.interFrameSpace = 10;
    ftm_status.frameParams.rate = HAL_PHY_RATE_11A_6_MBPS;
    ftm_status.frameParams.payloadContents = TEST_PAYLOAD_RANDOM;
    ftm_status.frameParams.payloadLength = 2000;
    ftm_status.frameParams.payloadFillByte = 0xA5;
    ftm_status.frameParams.pktAutoSeqNum = eANI_BOOLEAN_FALSE;
    ftm_status.frameParams.pktScramblerSeed = 7;
    ftm_status.frameParams.crc = 0;
    ftm_status.frameParams.preamble = PHYDBG_PREAMBLE_OFDM;
    memcpy(&ftm_status.frameParams.addr1[0], addr1, ANI_MAC_ADDR_SIZE);
    memcpy(&ftm_status.frameParams.addr2[0], addr2, ANI_MAC_ADDR_SIZE);
    memcpy(&ftm_status.frameParams.addr3[0], addr3, ANI_MAC_ADDR_SIZE);
    ftm_status.txpower = 2 ;
    ftm_status.rxmode = RXMODE_ENABLE_ALL; /* macStart() enables all receive pkt types */
    ftm_status.chainSelect = FTM_CHAIN_SEL_R0R1_T0_ON;

    return;
}

/**---------------------------------------------------------------------------

  \brief wlan_ftm_postmsg() -

   The function used for sending the command to the halphy.

  \param  - cmd_ptr - Pointer command buffer.

  \param  - cmd_len - Command length.

  \return - 0 for success, non zero for failure

  --------------------------------------------------------------------------*/

static v_U32_t wlan_ftm_postmsg(v_U8_t *cmd_ptr, v_U16_t cmd_len)
{
    vos_msg_t   *ftmReqMsg;
    vos_msg_t    ftmMsg;
    ENTER();

    ftmReqMsg = (vos_msg_t *) cmd_ptr;

    ftmMsg.type = ftmReqMsg->type;
    ftmMsg.reserved = 0;
    ftmMsg.bodyptr = (v_U8_t*)cmd_ptr;
    ftmMsg.bodyval = 0;

    /* Use Vos messaging mechanism to send the command to halPhy */

    if (VOS_STATUS_SUCCESS != vos_mq_post_message(VOS_MODULE_ID_HAL,
                                    (vos_msg_t *)&ftmMsg)) {
        hddLog(VOS_TRACE_LEVEL_ERROR,"%s: : Failed to post Msg to HAL\n",__func__);

        return VOS_STATUS_E_FAILURE;
    }

    EXIT();
    return VOS_STATUS_SUCCESS;
}

/**---------------------------------------------------------------------------

  \brief wlan_ftm_priv_set_txifs() -

   This function is used for 

  \param  - pAdapter - Pointer HDD Context.
          - ifs

  \return - 0 for success, non zero for failure

  --------------------------------------------------------------------------*/



static VOS_STATUS wlan_ftm_priv_set_txifs(hdd_adapter_t *pAdapter,v_U32_t ifs)
{
    if(pAdapter->ftm.ftm_state != WLAN_FTM_STARTED)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:Ftm has not started.Please start the ftm. ",__func__);
        return VOS_STATUS_E_FAILURE;
    }

    /* do not allow to change setting when tx pktgen is enabled */
    if (ftm_status.frameGenEnabled)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:cannot set txifs when pktgen is enabled.",__func__);
        return VOS_STATUS_E_FAILURE;
    }

    if (ifs > 100000) //max = (MSK_24 / ONE_MICROSECOND)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:ifs value is invalid ",__func__);
        return VOS_STATUS_E_FAILURE;
    }

    ftm_status.frameParams.interFrameSpace = ifs;

    return VOS_STATUS_SUCCESS;
}

/**---------------------------------------------------------------------------

  \brief wlan_ftm_priv_set_txpktcnt() -

   This function is used for 

  \param  - pAdapter - Pointer HDD Context.
          - ifs

  \return - 0 for success, non zero for failure

  --------------------------------------------------------------------------*/

static VOS_STATUS wlan_ftm_priv_set_txpktcnt(hdd_adapter_t *pAdapter,v_U32_t cnt)
{
    if(pAdapter->ftm.ftm_state != WLAN_FTM_STARTED)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:Ftm has not started.Please start the ftm. ",__func__);
        return VOS_STATUS_E_FAILURE;
    }

    /* do not allow to change setting when tx pktgen is enabled */
    if (ftm_status.frameGenEnabled)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:cannot set txpktcnt when pktgen is enabled.",__func__);
        return VOS_STATUS_E_FAILURE;
    }

    if (cnt > QWLAN_PHYDBG_TXPKT_CNT_CNT_MASK) //0xFFFF
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:pktcnt value is invalid",__func__);
        return VOS_STATUS_E_FAILURE;
    }

    ftm_status.frameParams.numTestPackets = cnt;

    return VOS_STATUS_SUCCESS;
}

static VOS_STATUS wlan_ftm_priv_set_txpktlen(hdd_adapter_t *pAdapter,v_U32_t len)
{
    if(pAdapter->ftm.ftm_state != WLAN_FTM_STARTED)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:Ftm has not started.Please start the ftm. ",__func__);
        return VOS_STATUS_E_FAILURE;
    }

    /* do not allow to change setting when tx pktgen is enabled */
    if (ftm_status.frameGenEnabled)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:cannot set txpktcnt when pktgen is enabled.",__func__);
        return VOS_STATUS_E_FAILURE;
    }

    if (len > 4095) //4096
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:payload len is invalid",__func__);
        return VOS_STATUS_E_FAILURE;
    }

    ftm_status.frameParams.payloadLength = (tANI_U16)len;

    return VOS_STATUS_SUCCESS;
}

/**---------------------------------------------------------------------------
  --------------------------------------------------------------------------*/

static VOS_STATUS wlan_ftm_priv_enable_chain(hdd_adapter_t *pAdapter,v_U16_t chainSelect)
{
    tPttMsgbuffer *pMsgBuf;
    uPttMsgs *pMsgBody;
    VOS_STATUS status;
    v_U16_t chainSelect_save = chainSelect;

    if(pAdapter->ftm.ftm_state != WLAN_FTM_STARTED)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:Ftm has not started.Please start the ftm. ",__func__);
        return VOS_STATUS_E_FAILURE;
    }

    if (chainSelect > FTM_CHAIN_SEL_MAX)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:Invalid chain",__func__);
        return VOS_STATUS_E_FAILURE;
    }

    /* do not allow to change setting when tx pktgen is enabled */
    if (ftm_status.frameGenEnabled)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:cannot select chain when pktgen is enabled.",__func__);
        return VOS_STATUS_E_FAILURE;
    }

    switch (chainSelect)
    {
        case FTM_CHAIN_SEL_NO_RX_TX:
            chainSelect = PHY_CHAIN_SEL_NO_RX_TX;
            break;

        case FTM_CHAIN_SEL_R0R1_ON:
            chainSelect = PHY_CHAIN_SEL_R0R1_ON;
            break;

        case FTM_CHAIN_SEL_R0_ON:
            chainSelect = PHY_CHAIN_SEL_R0_ON;
            break;

        case FTM_CHAIN_SEL_R1_ON:
            chainSelect = PHY_CHAIN_SEL_R1_ON;
            break;

        case FTM_CHAIN_SEL_T0_ON:
            chainSelect = PHY_CHAIN_SEL_T0_ON;
            break;
    }

    pMsgBuf = (tPttMsgbuffer *)vos_mem_malloc(sizeof(tPttMsgbuffer));

    init_completion(&pAdapter->ftm.ftm_comp_var);
    pMsgBuf->msgId = PTT_MSG_ENABLE_CHAINS;
    pMsgBuf->msgBodyLength = sizeof(tMsgPttEnableChains) + PTT_HEADER_LENGTH;

    pMsgBody = &pMsgBuf->msgBody;
    pMsgBody->EnableChains.chainSelect = chainSelect;

    status = wlan_ftm_postmsg((v_U8_t*)pMsgBuf,pMsgBuf->msgBodyLength);

    if(status != VOS_STATUS_SUCCESS)
    {        
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:wlan_ftm_postmsg failed",__func__);
        status = VOS_STATUS_E_FAILURE;
        goto done;
    }
    wait_for_completion_interruptible_timeout(&pAdapter->ftm.ftm_comp_var, msecs_to_jiffies(WLAN_FTM_COMMAND_TIME_OUT));

    if(pMsgBuf->msgResponse != PTT_STATUS_SUCCESS)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:Ptt response status failed",__func__);
        status = VOS_STATUS_E_FAILURE;
        goto done;
    }
    ftm_status.chainSelect = chainSelect_save;
done:
    vos_mem_free((v_VOID_t * )pMsgBuf);

    return status;
}

/**---------------------------------------------------------------------------
  --------------------------------------------------------------------------*/
static VOS_STATUS wlan_ftm_priv_get_status(hdd_adapter_t *pAdapter,char *buf)
{
    int ii;
    char *chain[] = {
        "None",
        "R0,R1",
        "R0",
        "R1",
        "T0",
        "R0,R1,T0"
    };
    char *rx[] = {
        "disable",
        "11g/n",
        "11b"
    };
    char *tx[] = {
        "stopped",
        "started",
    };
    if(pAdapter->ftm.ftm_state != WLAN_FTM_STARTED)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:Ftm has not started.Please start the ftm. ",__func__);
        return VOS_STATUS_E_FAILURE;
    }

    buf += sprintf(buf, "\n");
    buf += sprintf(buf, " chainSelect: %s\n", chain[ftm_status.chainSelect]);
    buf += sprintf(buf, " rxmode: %s\n", rx[ftm_status.rxmode]);
    buf += sprintf(buf, " txpktgen: %s\n", tx[ftm_status.frameGenEnabled]);
    buf += sprintf(buf, "  txifs: %ld\n", ftm_status.frameParams.interFrameSpace);
    buf += sprintf(buf, "  txrate: ");
    for(ii = 0; ii < SIZE_OF_TABLE(rateName_rateIndex_tbl); ii++)
    {
        if (rateName_rateIndex_tbl[ii].rate_index == ftm_status.frameParams.rate)
          break;
    }
    strcpy(buf, rateName_rateIndex_tbl[ii].rate_str);
    buf += strlen(buf) ;
    buf += sprintf(buf, "\n");
    buf += sprintf(buf, "  txpower: %d\n", ftm_status.txpower);
    buf += sprintf(buf, "  txpktcnt: %ld\n", ftm_status.frameParams.numTestPackets);
    buf += sprintf(buf, "  txpktlen: %d\n", ftm_status.frameParams.payloadLength);
    *buf = 0 ;
    return VOS_STATUS_SUCCESS;
}

v_VOID_t
static ftm_vos_sys_probe_thread_cback
(
  v_VOID_t *pUserData
)
{
    pVosContextType pVosContext= (pVosContextType)pUserData;
    if (vos_event_set(&pVosContext->ProbeEvent)!= VOS_STATUS_SUCCESS)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
         "%s: vos_event_set failed", __FUNCTION__);
        return;
    }
} /* vos_sys_probe_thread_cback() */


void HEXDUMP(char *s0, char *s1, int len)
{
    int tmp;
    printk(KERN_EMERG "%s\n :", s0);

    for (tmp = 0; tmp< len; tmp++) {
        printk(KERN_EMERG "%02x ", *s1++);
    }
    printk("\n");
}

/**---------------------------------------------------------------------------

  \brief wlan_hdd_ftm_open() -

   The function hdd_wlan_sdio_probe calls this function to initialize the FTM specific modules.

  \param  - pAdapter - Pointer HDD Context.

  \return - 0 for success, non zero for failure

  --------------------------------------------------------------------------*/

int wlan_hdd_ftm_open(hdd_adapter_t *pAdapter)
{
    VOS_STATUS vStatus       = VOS_STATUS_SUCCESS;
    v_U8_t iter               = 0;
    tSirRetStatus sirStatus = eSIR_SUCCESS;
    tMacOpenParameters macOpenParms;

    pVosContextType pVosContext= NULL;

    VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO_HIGH,
               "%s: Opening VOSS", __func__);

    pVosContext = vos_get_global_context(VOS_MODULE_ID_SYS, NULL);

    if (NULL == pVosContext)
    {
        VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                    "%s: Trying to open VOSS without a PreOpen",__func__);
        VOS_ASSERT(0);
        goto err_vos_status_failure;
    }

    /* Initialize the probe event */
    if (vos_event_init(&pVosContext->ProbeEvent) != VOS_STATUS_SUCCESS)
    {
        VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                    "%s: Unable to init probeEvent",__func__);
        VOS_ASSERT(0);
        goto err_vos_status_failure;
    }

    /* Initialize the free message queue */
    vStatus = vos_mq_init(&pVosContext->freeVosMq);
    if (! VOS_IS_STATUS_SUCCESS(vStatus))
    {

        /* Critical Error ...  Cannot proceed further */
        VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                "%s: Failed to initialize VOS free message queue",__func__);
        VOS_ASSERT(0);
        goto err_probe_event;
    }

    for (iter =0; iter < VOS_CORE_MAX_MESSAGES; iter++)
    {
        (pVosContext->aMsgWrappers[iter]).pVosMsg =
         &(pVosContext->aMsgBuffers[iter]);
        INIT_LIST_HEAD(&pVosContext->aMsgWrappers[iter].msgNode);
        vos_mq_put(&pVosContext->freeVosMq, &(pVosContext->aMsgWrappers[iter]));
    }

    /* Now Open the VOS Scheduler */
    vStatus= vos_sched_open(pVosContext, &pVosContext->vosSched,
                           sizeof(VosSchedContext));

    if (!VOS_IS_STATUS_SUCCESS(vStatus))
    {
       /* Critical Error ...  Cannot proceed further */
       VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                "%s: Failed to open VOS SCheduler", __func__);
       VOS_ASSERT(0);
       goto err_msg_queue;
    }

    /* Probe the MC thread */
    sysMcThreadProbe(pVosContext,
                    &ftm_vos_sys_probe_thread_cback,
                    pVosContext);

    if (vos_wait_single_event(&pVosContext->ProbeEvent, 0)!= VOS_STATUS_SUCCESS)
    {
        VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                "%s: Failed to probe MC Thread", __func__);
        VOS_ASSERT(0);
        goto err_sched_close;
    }

    /* initialize the NV module */
    vStatus = vos_nv_open();

    if (!VOS_IS_STATUS_SUCCESS(vStatus))
    {
        // NV module cannot be initialized, however the driver is allowed
        // to proceed
         VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
          "%s: Failed to initialize the NV module", __func__);
         goto err_sched_close;
    }
    /* If we arrive here, both threads dispacthing messages correctly */

    /* Now proceed to open the MAC */

    /* UMA is supported in hardware for performing the
      frame translation 802.11 <-> 802.3 */
    macOpenParms.frameTransRequired = 1;
    sirStatus = macOpen(&(pVosContext->pMACContext), pVosContext->pHDDContext,
                         &macOpenParms);

    if (eSIR_SUCCESS != sirStatus)
    {
        /* Critical Error ...    Cannot proceed further */
        VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                  "%s: Failed to open MAC", __func__);
        VOS_ASSERT(0);
        goto err_nv_close;
    }

    vStatus = WLANBAL_Open(pVosContext);

    if(!VOS_IS_STATUS_SUCCESS(vStatus))
    {
        VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
        "%s: Failed to open BAL",__func__);
        goto err_mac_close;
    }

    VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO_HIGH,
               "%s: VOSS successfully Opened",__func__);

       /* Save the hal context in Adapter */
    pAdapter->hHal = (tHalHandle)vos_get_context( VOS_MODULE_ID_HAL, pVosContext );

    if ( NULL == pAdapter->hHal )
    {
       hddLog(VOS_TRACE_LEVEL_ERROR,"%s: HAL context is null",__func__);
       goto err_mac_close;
    }

    wlan_ftm_register_wext(pAdapter); 
       //Initialize the nlink service
    if(nl_srv_init() != 0)
    {
       hddLog(VOS_TRACE_LEVEL_ERROR,"%S: nl_srv_init failed",__func__);
       goto err_mac_close;
    }

#ifdef PTT_SOCK_SVC_ENABLE
    //Initialize the PTT service
    if(ptt_sock_activate_svc(pAdapter) != 0)
    {
       hddLog(VOS_TRACE_LEVEL_ERROR,"%s: ptt_sock_activate_svc failed",__func__);
       goto err_nl_srv_init;
    }
#endif
       //Turn off carrier state
    netif_carrier_off(pAdapter->dev);

    //Stop the Interface TX queue. Just being safe
    netif_tx_disable(pAdapter->dev);

    //wtan: initialize ftm_status structure
    _ftm_status_init();

    pAdapter->ftm.ftm_state = WLAN_FTM_INITIALIZED;

    return VOS_STATUS_SUCCESS;

err_nl_srv_init:
nl_srv_exit();

err_mac_close:
macClose(pVosContext->pMACContext);

err_nv_close:
vos_nv_close();

err_sched_close:
vos_sched_close(pVosContext);

err_msg_queue:
vos_mq_deinit(&pVosContext->freeVosMq);

err_probe_event:
vos_event_destroy(&pVosContext->ProbeEvent);

err_vos_status_failure:

    return VOS_STATUS_E_FAILURE;
}



int wlan_hdd_ftm_close(hdd_adapter_t *pAdapter)
{
    VOS_STATUS vosStatus;
    v_CONTEXT_t vosContext = pAdapter->pvosContext;

    ENTER();
    vosStatus = WLANBAL_Close(vosContext);
    if (!VOS_IS_STATUS_SUCCESS(vosStatus))
    {
       VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
           "%s: Failed to close BAL",__func__);
       VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
    }
    vosStatus = macClose( ((pVosContextType)vosContext)->pMACContext);
    if (!VOS_IS_STATUS_SUCCESS(vosStatus))
    {
       VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
           "%s: Failed to close MAC",__func__);
       VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
    }
  
    ((pVosContextType)vosContext)->pMACContext = NULL;
  
    vosStatus = vos_nv_close();
    if (!VOS_IS_STATUS_SUCCESS(vosStatus))
    {
       VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
           "%s: Failed to close NV",__func__);
       VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
    }

  
    vosStatus = sysClose( vosContext );
    if (!VOS_IS_STATUS_SUCCESS(vosStatus))
    {
       VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
           "%s: Failed to close SYS",__func__);
       VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
    }

    nl_srv_exit();

    //Deregister the device with the kernel
    hdd_UnregisterWext(pAdapter->dev);

    if(test_bit(NET_DEVICE_REGISTERED, &pAdapter->event_flags)) 
    {
        unregister_netdev(pAdapter->dev);
        clear_bit(NET_DEVICE_REGISTERED, &pAdapter->event_flags);
    }
    return 0;
}

/**---------------------------------------------------------------------------

  \brief wlan_ftm_send_response() -

   The function sends the response to the ptt socket application running in user space.

  \param  - pAdapter - Pointer HDD Context.

  \return - 0 for success, non zero for failure

  --------------------------------------------------------------------------*/

static VOS_STATUS wlan_ftm_send_response(hdd_adapter_t *pAdapter){

   if( ptt_sock_send_msg_to_app(&pAdapter->ftm.wnl->wmsg, 0, ANI_NL_MSG_PUMAC, pAdapter->ftm.wnl->nlh.nlmsg_pid) < 0) {

       VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR, ("Ptt Socket error sending message to the app!!\n"));
       return VOS_STATUS_E_FAILURE;
   }
   return VOS_STATUS_SUCCESS;
}

/**---------------------------------------------------------------------------

  \brief wlan_hdd_ftm_start() -

   This function gets called when the FTM start commands received from the ptt socket application and
   it starts the following modules.
   1) SAL Start.
   2) BAL Start.
   3) MAC Start to download the firmware.


  \param  - pAdapter - Pointer HDD Context.

  \return - 0 for success, non zero for failure

  --------------------------------------------------------------------------*/

static int wlan_hdd_ftm_start(hdd_adapter_t *pAdapter)
{
    VOS_STATUS vStatus          = VOS_STATUS_SUCCESS;
    tSirRetStatus sirStatus      = eSIR_SUCCESS;
    pVosContextType pVosContext = (pVosContextType)pAdapter->pvosContext;
    tHalMacStartParameters halStartParams;

    VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
            "%s: Starting Libra SW", __func__);

    /* We support only one instance for now ...*/
    if (pVosContext == NULL)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
           "%s: mismatch in context",__FUNCTION__);
        goto err_status_failure;
    }

    if ((pVosContext->pBALContext == NULL) || ( pVosContext->pMACContext == NULL))
    {
        if (pVosContext->pBALContext == NULL)
           VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
               "%s: BAL NULL context",__FUNCTION__);
        else if (pVosContext->pMACContext == NULL)
           VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
               "%s: MAC NULL context",__FUNCTION__);
        else
           VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
               "%s: TL NULL context",__FUNCTION__);

        goto err_status_failure;
    }

    /* Start SAL now */
    vStatus = WLANSAL_Start(pVosContext);
    if (!VOS_IS_STATUS_SUCCESS(vStatus))
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
              "%s: Failed to start SAL",__func__);
        goto err_status_failure;
    }

    VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
           "%s: SAL correctly started", __func__);

    /* Start BAL */
    vStatus = WLANBAL_Start(pVosContext);

    if (!VOS_IS_STATUS_SUCCESS(vStatus))
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
              "%s: Failed to start BAL",__func__);
        goto err_sal_stop;
    }

    VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
             "%s: BAL correctly started",__func__);

    /* Start the MAC */
    vos_mem_zero((v_PVOID_t)&halStartParams, sizeof(tHalMacStartParameters));

    /* Attempt to get the firmware binary through VOS.  We need to pass this
           to the MAC when starting. */
    vStatus = hdd_request_firmware(LIBRA_FW_FILE,pAdapter,
                               (v_VOID_t **)&halStartParams.FW.pImage,
                               (v_SIZE_t *)&halStartParams.FW.cbImage);

    if ( !VOS_IS_STATUS_SUCCESS( vStatus ) )
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
             "%s: Failed to get firmware binary",__func__);
        printk(KERN_EMERG "***Failed to get firmware binary***\n");
        goto err_bal_stop;
    }

    VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
             "%s: Firmware binary file found",__func__);

    halStartParams.driverType = eDRIVER_TYPE_MFG;

    /* Start the MAC */
    sirStatus = macStart(pVosContext->pMACContext,(v_PVOID_t)&halStartParams);

    /* Free uo the FW image no matter what */
    if( NULL != halStartParams.FW.pImage )
    {
        hdd_release_firmware(LIBRA_FW_FILE,pVosContext->pHDDContext);
        halStartParams.FW.pImage = NULL;
        halStartParams.FW.cbImage = 0;
    }

    if (eSIR_SUCCESS != sirStatus)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
              "%s: Failed to start MAC", __func__);

        printk(KERN_EMERG "***Failed to start MAC****\n");
        goto err_bal_stop;
    }

    VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
            "%s: MAC correctly started",__func__);


    printk(KERN_EMERG "*** FTM Start Successful****\n");


    /* START SYS. This will trigger the CFG download */
    sysMcStart(pVosContext, ftm_vos_sys_probe_thread_cback, pVosContext);

    /* Initialize the ftm vos event */
    if (vos_event_init(& pAdapter->ftm.ftm_vos_event) != VOS_STATUS_SUCCESS)
    {
        VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                    "%s: Unable to init probeEvent",__func__);
        VOS_ASSERT(0);
        goto err_mac_stop;
    }

    pAdapter->ftm.ftm_state = WLAN_FTM_STARTED;

    return VOS_STATUS_SUCCESS;

err_mac_stop:
macStop(pVosContext->pMACContext, HAL_STOP_TYPE_SYS_RESET);

err_bal_stop:
WLANBAL_Stop(pVosContext);

err_sal_stop:
WLANSAL_Stop(pVosContext);

err_status_failure:

    return VOS_STATUS_E_FAILURE;

}


static int wlan_ftm_stop(hdd_adapter_t *pAdapter)
{
   VOS_STATUS vosStatus;   
   vos_call_status_type callType;

   if(pAdapter->ftm.ftm_state != WLAN_FTM_STARTED)
{
       VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:Ftm has not started.Please start the ftm. ",__func__);
       return VOS_STATUS_E_FAILURE;
   }

   if(pAdapter->ftm.cmd_iwpriv == TRUE)
   {
       /* SYS STOP will stop SME and MAC */
       vosStatus = sysStop( pAdapter->pvosContext);
       if (!VOS_IS_STATUS_SUCCESS(vosStatus))
       {
           VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
             "%s: Failed to stop SYS",__func__);
           VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
       }
       vosStatus = WLANBAL_Stop( pAdapter->pvosContext );
       if (!VOS_IS_STATUS_SUCCESS(vosStatus))
       {
           VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
             "%s: Failed to stop BAL",__func__);
           VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
       }

       vosStatus = WLANBAL_SuspendChip( pAdapter->pvosContext );
       VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );

       vosStatus = WLANSAL_Stop(pAdapter->pvosContext);
       VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );

       vosStatus = vos_chipAssertDeepSleep( &callType, NULL, NULL );
       VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );

       vosStatus = vos_chipVoteOffBBAnalogSupply(&callType, NULL, NULL);
       VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );

       vosStatus = vos_chipVoteOffRFSupply(&callType, NULL, NULL);
       VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
    }
   return WLAN_FTM_SUCCESS;
}



/**---------------------------------------------------------------------------

  \brief wlan_hdd_process_ftm_cmd() -

   This function process the commands received from the ptt socket application.

  \param  - pAdapter - Pointer HDD Context.

  \param  - wnl - Pointer to the ANI netlink header.

  \return - none

  --------------------------------------------------------------------------*/

void wlan_hdd_process_ftm_cmd
(
    hdd_adapter_t *pAdapter,
    tAniNlHdr *wnl
)
{
    wlan_hdd_ftm_request_t  *pRequestBuf = (wlan_hdd_ftm_request_t*)(((v_U8_t*)(&wnl->wmsg))+sizeof(tAniHdr)) ;
    v_U16_t   cmd_len;
    v_U8_t *pftm_data;

    ENTER();

    if (!pRequestBuf) {

        hddLog(VOS_TRACE_LEVEL_ERROR,"%s: request buffer is null\n",__func__);
        return ;
    }
    /*Save the received request*/
    pAdapter->ftm.pRequestBuf = pRequestBuf;

    pAdapter->ftm.pResponseBuf = (wlan_hdd_ftm_response_t*)pRequestBuf;

     /*Save the received request netlink header used for sending the response*/
    pAdapter->ftm.wnl = wnl;

    if (pRequestBuf->module_type != QUALCOMM_MODULE_TYPE) {

        hddLog(VOS_TRACE_LEVEL_ERROR,"%s: Invalid Module Type =%d\n",__func__,pRequestBuf->module_type);

        pAdapter->ftm.pResponseBuf->ftm_err_code = WLAN_FTM_FAILURE;
        wlan_ftm_send_response(pAdapter);
        return ;
    }

    switch (pRequestBuf->ftmpkt.ftm_cmd_type)
    {
    case WLAN_FTM_START:
        if (pAdapter->ftm.ftm_state == WLAN_FTM_STARTED) {

            hddLog(VOS_TRACE_LEVEL_ERROR,"%s: FTM has already started =%d\n",__func__,pRequestBuf->ftmpkt.ftm_cmd_type);
            pAdapter->ftm.pResponseBuf->ftm_hdr.data_len -= 1;
            pAdapter->ftm.pResponseBuf->ftm_err_code = WLAN_FTM_FAILURE;
            wlan_ftm_send_response(pAdapter);
            return;
        }
        if (wlan_hdd_ftm_start(pAdapter) != VOS_STATUS_SUCCESS) {
            hddLog(VOS_TRACE_LEVEL_ERROR,"%s: : Failed to start WLAN FTM\n",__func__);
            return;
        }
        /* Ptt application running on the host PC expects the length to be one byte less that what we have received*/
        pAdapter->ftm.pResponseBuf->ftm_hdr.data_len -= 1;
        pAdapter->ftm.pResponseBuf->ftm_err_code = WLAN_FTM_SUCCESS;
        pAdapter->ftm.pResponseBuf->ftmpkt.ftm_cmd_type = 0;


        wlan_ftm_send_response(pAdapter);

        break;

    case WLAN_FTM_STOP:

        if (pAdapter->ftm.ftm_state != WLAN_FTM_STARTED) {

            hddLog(VOS_TRACE_LEVEL_ERROR,"%s:: FTM has not started\n",__func__);
            return;
        }

        if (VOS_STATUS_SUCCESS != wlan_ftm_stop(pAdapter)) {

            pAdapter->ftm.pResponseBuf->ftm_err_code = WLAN_FTM_FAILURE;
            wlan_ftm_send_response(pAdapter);
            return;
        }

        /* This would send back the Command Success Status */
        pAdapter->ftm.pResponseBuf->ftm_err_code = WLAN_FTM_SUCCESS;

        wlan_ftm_send_response(pAdapter);

        break;

    case WLAN_FTM_CMD:

        /* if it is regular FTM command, pass it to HAL PHY */
        if(pAdapter->ftm.IsCmdPending == TRUE) {
            hddLog(VOS_TRACE_LEVEL_ERROR,"%s:: FTM command pending for process\n",__func__);
            return;
        }

        if (pAdapter->ftm.ftm_state != WLAN_FTM_STARTED) {

            hddLog(VOS_TRACE_LEVEL_ERROR,"%s:: FTM has not started\n",__func__);

            pAdapter->ftm.pResponseBuf->ftm_err_code = WLAN_FTM_FAILURE;
            wlan_ftm_send_response(pAdapter);
            return;

        }
        vos_event_reset(&pAdapter->ftm.ftm_vos_event);

        cmd_len = pRequestBuf->ftm_hdr.data_len;

        cmd_len -= (sizeof(wlan_hdd_ftm_request_t)- sizeof(pRequestBuf->ftmpkt.ftm_cmd_type));
        pftm_data = pRequestBuf->ftmpkt.pFtmCmd;

        //HEXDUMP("Request:",(char*)pftm_data,cmd_len);

        pAdapter->ftm.IsCmdPending = TRUE;

        /*Post the command to the HAL*/
        if (wlan_ftm_postmsg(pftm_data, cmd_len) != VOS_STATUS_SUCCESS) {

            hddLog(VOS_TRACE_LEVEL_ERROR,"%s:: FTM command failed\n",__func__);
            return;

        }
        /*Wait here until you get the response from HAL*/
        if (vos_wait_single_event(&pAdapter->ftm.ftm_vos_event, 0)!= VOS_STATUS_SUCCESS)
        {
            hddLog(VOS_TRACE_LEVEL_ERROR,
               "%s: vos_wait_single_event failed",__func__);
            return;
        }

        cmd_len = be16_to_cpu(pAdapter->ftm.wnl->wmsg.length);

        //HEXDUMP("Response to QXDM:", (char *)&pAdapter->ftm.wnl->wmsg, cmd_len);

        wlan_ftm_send_response(pAdapter);
        pAdapter->ftm.IsCmdPending = FALSE;
        break;

    default:

        hddLog(VOS_TRACE_LEVEL_ERROR,"%s:: Command not supported \n",__func__);
        return;
    }

    EXIT();
    return;
} /* wlan_adp_ftm_cmd() */

/**---------------------------------------------------------------------------

  \brief WLANFTM_McProcessMsg() -

   Called by VOSS when a message was serialized for FTM through the
   main thread/task.

  \param  -  message:        type and content of the message.

  \return - 0 for success, non zero for failure

  --------------------------------------------------------------------------*/


/**---------------------------------------------------------------------------

  \brief wlan_ftm_priv_start_stop_ftm() -

   This function is used for start/stop the ftm driver. 

  \param  - pAdapter - Pointer HDD Context.
              - start - 1/0 to start/stop ftm driver.

  \return - 0 for success, non zero for failure

  --------------------------------------------------------------------------*/

static VOS_STATUS wlan_ftm_priv_start_stop_ftm(hdd_adapter_t *pAdapter,v_U16_t start)
{
    VOS_STATUS status;

    if(start) 
    {
        pAdapter->ftm.cmd_iwpriv = TRUE;
        status = wlan_hdd_ftm_start(pAdapter);

        if(status != VOS_STATUS_SUCCESS) {
            
            VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "FTM Start Failed");
            return VOS_STATUS_E_FAILURE; 
        }
    } 
    else 
    {
        status = wlan_ftm_stop(pAdapter);
        
        if(status != VOS_STATUS_SUCCESS) {
            
            VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "FTM Stop Failed");
            return VOS_STATUS_E_FAILURE; 
        }
    }
    return VOS_STATUS_SUCCESS; 
}

/**---------------------------------------------------------------------------

  \brief wlan_ftm_priv_set_channel() -

   This function is used for setting the channel to the halphy ptt module. 

  \param  - pAdapter - Pointer HDD Context.
              - channel   -  Channel Number 1-14.

  \return - 0 for success, non zero for failure

  --------------------------------------------------------------------------*/

static VOS_STATUS wlan_ftm_priv_set_channel(hdd_adapter_t *pAdapter,v_U16_t channel)
{
    tPttMsgbuffer *pMsgBuf;
    uPttMsgs *pMsgBody;
    VOS_STATUS status;

    if(pAdapter->ftm.ftm_state != WLAN_FTM_STARTED)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:Ftm has not started.Please start the ftm. ",__func__);
        return VOS_STATUS_E_FAILURE;
    }

    if(!(channel >= 1 && channel <= 14)) 
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:Invalid Channel Number. ",__func__);
        return VOS_STATUS_E_FAILURE;
    }

    pMsgBuf = (tPttMsgbuffer *)vos_mem_malloc(sizeof(tPttMsgbuffer));

    init_completion(&pAdapter->ftm.ftm_comp_var);
    pMsgBuf->msgId = PTT_MSG_SET_CHANNEL;
    pMsgBuf->msgBodyLength = sizeof(tMsgPttSetChannel) + PTT_HEADER_LENGTH;

    pMsgBody = &pMsgBuf->msgBody;

    pMsgBody->SetChannel.chId = channel;

    VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO_HIGH, "Channel =%d\n",pMsgBody->SetChannel.chId);
    pMsgBody->SetChannel.cbState = PHY_SINGLE_CHANNEL_CENTERED;

    status = wlan_ftm_postmsg((v_U8_t*)pMsgBuf,pMsgBuf->msgBodyLength);

    if(status != VOS_STATUS_SUCCESS)
    {        
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:wlan_ftm_postmsg failed",__func__);
        status = VOS_STATUS_E_FAILURE;
        goto done;

    }
    wait_for_completion_interruptible_timeout(&pAdapter->ftm.ftm_comp_var, msecs_to_jiffies(WLAN_FTM_COMMAND_TIME_OUT));

    if(pMsgBuf->msgResponse != PTT_STATUS_SUCCESS)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:Ptt response status failed",__func__);
        status = VOS_STATUS_E_FAILURE;
        goto done;

    }
done:
    vos_mem_free((v_VOID_t * )pMsgBuf);

    return status;
}

/**---------------------------------------------------------------------------

  \brief wlan_ftm_priv_set_txpower() -

   This function is used for setting the txpower to the halphy ptt module. 

  \param  - pAdapter - Pointer HDD Context.
              - txpower   -  txpower Number 1-18.

  \return - 0 for success, non zero for failure

  --------------------------------------------------------------------------*/

static VOS_STATUS wlan_ftm_priv_set_txpower(hdd_adapter_t *pAdapter,v_U16_t txpower)
{
    tPttMsgbuffer *pMsgBuf;
    uPttMsgs *pMsgBody;
    VOS_STATUS status;

    if(pAdapter->ftm.ftm_state != WLAN_FTM_STARTED)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:Ftm has not started.Please start the ftm. ",__func__);
        return VOS_STATUS_E_FAILURE;
    }

    /* do not allow to change setting when tx pktgen is enabled, although halphy does allow changing tx power
     * when tx pktgen is enabled
     */
    if (ftm_status.frameGenEnabled)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:cannot set txpower when pktgen is enabled.",__func__);
        return VOS_STATUS_E_FAILURE;
    }

    if(!(txpower >= 2 && txpower <= 16)) 
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:Invalid tx power. ",__func__);
        return VOS_STATUS_E_FAILURE;
    }
    pMsgBuf = (tPttMsgbuffer *)vos_mem_malloc(sizeof(tPttMsgbuffer));

    init_completion(&pAdapter->ftm.ftm_comp_var);
    pMsgBuf->msgId = PTT_MSG_CLOSE_TPC_LOOP;
    pMsgBuf->msgBodyLength = sizeof(tMsgPttCloseTpcLoop) + PTT_HEADER_LENGTH;
    
    pMsgBody = &pMsgBuf->msgBody;
    pMsgBody->CloseTpcLoop.tpcClose = TRUE;

    status = wlan_ftm_postmsg((v_U8_t*)pMsgBuf,pMsgBuf->msgBodyLength);

    if(status != VOS_STATUS_SUCCESS)
    {        
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:wlan_ftm_postmsg failed",__func__);
        status = VOS_STATUS_E_FAILURE;
        goto done;
    }
    wait_for_completion_interruptible_timeout(&pAdapter->ftm.ftm_comp_var, msecs_to_jiffies(WLAN_FTM_COMMAND_TIME_OUT));

    if(pMsgBuf->msgResponse != PTT_STATUS_SUCCESS)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:Ptt response status failed",__func__);
        status = VOS_STATUS_E_FAILURE;
        goto done;
    }

    init_completion(&pAdapter->ftm.ftm_comp_var);
    pMsgBuf->msgId = PTT_MSG_SET_TX_POWER;
    pMsgBuf->msgBodyLength = sizeof(tMsgPttSetTxPower) + PTT_HEADER_LENGTH;

    pMsgBody->SetTxPower.dbmPwr = txpower*100;

    status = wlan_ftm_postmsg((v_U8_t*)pMsgBuf,pMsgBuf->msgBodyLength);

    if(status != VOS_STATUS_SUCCESS)
    {        
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:wlan_ftm_postmsg failed",__func__);
        status = VOS_STATUS_E_FAILURE;
        goto done;
    }
    wait_for_completion_interruptible_timeout(&pAdapter->ftm.ftm_comp_var, msecs_to_jiffies(WLAN_FTM_COMMAND_TIME_OUT));

    if(pMsgBuf->msgResponse != PTT_STATUS_SUCCESS)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:Ptt response status failed",__func__);
        status = VOS_STATUS_E_FAILURE;
        goto done;
    }

    ftm_status.txpower = txpower ;
 done:   
    vos_mem_free((v_VOID_t * )pMsgBuf);

    return status;

}
/**---------------------------------------------------------------------------

  \brief wlan_ftm_priv_set_txrate() -

   This function is used for setting the txrate to the halphy ptt module. 
   It converts the user input string for txrate to the tx rate index.

  \param  - pAdapter - Pointer HDD Context.
              - txrate   -  Pointer to the tx rate string.

  \return - 0 for success, non zero for failure

  --------------------------------------------------------------------------*/

static VOS_STATUS wlan_ftm_priv_set_txrate(hdd_adapter_t *pAdapter,char *txrate)
{
    int ii;

    if(pAdapter->ftm.ftm_state != WLAN_FTM_STARTED)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:Ftm has not started.Please start the ftm.",__func__);
        return VOS_STATUS_E_FAILURE;
    }

    /* do not allow to change setting when tx pktgen is enabled */
    if (ftm_status.frameGenEnabled)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:cannot set txrate when pktgen is enabled.",__func__);
        return VOS_STATUS_E_FAILURE;
    }

    for(ii = 0; ii < SIZE_OF_TABLE(rateName_rateIndex_tbl); ii++)
    {
        if(!strcmp(rateName_rateIndex_tbl[ii].rate_str,txrate))
           break;
    }
    if(ii >= SIZE_OF_TABLE(rateName_rateIndex_tbl))
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:Invalid Rate String\n",__func__);
        return VOS_STATUS_E_FAILURE;
    }

    ftm_status.frameParams.rate = rateName_rateIndex_tbl[ii].rate_index;
    ftm_status.frameParams.preamble = rate_index_2_preamble_table[rateName_rateIndex_tbl[ii].rate_index].Preamble;

    return VOS_STATUS_SUCCESS;
}

/**---------------------------------------------------------------------------

  \brief wlan_ftm_priv_start_stop_tx_pktgen() -

   This function is used for start/stop the tx packet generation. 

  \param  - pAdapter - Pointer HDD Context.
              - startStop   -  Value( 1/0) start/stop the tx packet generation.

  \return - 0 for success, non zero for failure

  --------------------------------------------------------------------------*/

static VOS_STATUS wlan_ftm_priv_start_stop_tx_pktgen(hdd_adapter_t *pAdapter,v_U16_t startStop)
{
    tPttMsgbuffer *pMsgBuf;
    uPttMsgs *pMsgBody;
    VOS_STATUS status;

    if(pAdapter->ftm.ftm_state != WLAN_FTM_STARTED)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:Ftm has not started.Please start the ftm. ",__func__);
        return VOS_STATUS_E_FAILURE;
    }

    if(startStop != 1 && startStop != 0)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:Tx value is invalid ",__func__);
        return VOS_STATUS_E_FAILURE;
    }

    if ((ftm_status.frameGenEnabled && startStop == 1) ||
        (!ftm_status.frameGenEnabled && startStop == 0))
    {
        return VOS_STATUS_SUCCESS ;
    }

    pMsgBuf = (tPttMsgbuffer *)vos_mem_malloc(sizeof(tPttMsgbuffer));

    if (startStop == 1)
    {
        init_completion(&pAdapter->ftm.ftm_comp_var);
        pMsgBuf->msgId = PTT_MSG_CONFIG_TX_PACKET_GEN;
        pMsgBuf->msgBodyLength = sizeof(tMsgPttConfigTxPacketGen) + PTT_HEADER_LENGTH;
        pMsgBody = &pMsgBuf->msgBody;
        pMsgBody->ConfigTxPacketGen.frameParams = ftm_status.frameParams ; 

        status = wlan_ftm_postmsg((v_U8_t*)pMsgBuf,pMsgBuf->msgBodyLength);
        if(status != VOS_STATUS_SUCCESS)
        {        
            VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:posting PTT_MSG_CONFIG_TX_PACKET_GEN failed",__func__);
            status = VOS_STATUS_E_FAILURE;
            goto done;
        }

        wait_for_completion_interruptible_timeout(&pAdapter->ftm.ftm_comp_var, msecs_to_jiffies(WLAN_FTM_COMMAND_TIME_OUT));
        if(pMsgBuf->msgResponse != PTT_STATUS_SUCCESS)
        {
            VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s: PTT_MSG_CONFIG_TX_PACKET_GEN failed",__func__);
            status = VOS_STATUS_E_FAILURE;
            goto done;
        }
    }

    init_completion(&pAdapter->ftm.ftm_comp_var);
    pMsgBuf->msgId = PTT_MSG_START_STOP_TX_PACKET_GEN;
    pMsgBuf->msgBodyLength = sizeof(tMsgPttStartStopTxPacketGen) + PTT_HEADER_LENGTH;
    pMsgBody = &pMsgBuf->msgBody;
    pMsgBody->StartStopTxPacketGen.startStop = startStop;

    status = wlan_ftm_postmsg((v_U8_t*)pMsgBuf,pMsgBuf->msgBodyLength);
    if(status != VOS_STATUS_SUCCESS)
    {        
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:wlan_ftm_postmsg failed",__func__);
        status = VOS_STATUS_E_FAILURE;
        goto done;
    }

    wait_for_completion_interruptible_timeout(&pAdapter->ftm.ftm_comp_var, msecs_to_jiffies(WLAN_FTM_COMMAND_TIME_OUT));
    if(pMsgBuf->msgResponse != PTT_STATUS_SUCCESS)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:Ptt response status failed",__func__);
        status = VOS_STATUS_E_FAILURE;
        goto done;
    }

done:
    vos_mem_free((v_VOID_t * )pMsgBuf);

    if (status == VOS_STATUS_SUCCESS)
    {
        if (startStop == 1)
        {
            ftm_status.frameGenEnabled = eANI_BOOLEAN_TRUE ;
        }
        else
        {
            ftm_status.frameGenEnabled = eANI_BOOLEAN_FALSE ;
        }
    }

    return status;
}


static VOS_STATUS wlan_ftm_priv_set_rssi_offset(hdd_adapter_t *pAdapter,v_S15_t *phyRxChains)
{
    tPttMsgbuffer *pMsgBuf;
    uPttMsgs *pMsgBody;
    VOS_STATUS status;

    if(pAdapter->ftm.ftm_state != WLAN_FTM_STARTED)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:Ftm has not started.Please start the ftm. ",__func__);
        return VOS_STATUS_E_FAILURE;
    }

    pMsgBuf = (tPttMsgbuffer *)vos_mem_malloc(sizeof(tPttMsgbuffer));

    init_completion(&pAdapter->ftm.ftm_comp_var);
    pMsgBuf->msgId = PTT_MSG_SET_NV_TABLE;
    pMsgBuf->msgBodyLength = sizeof(tMsgPttSetNvTable) + PTT_HEADER_LENGTH;
    pMsgBody = &pMsgBuf->msgBody;
    pMsgBody->SetNvTable.nvTable= NV_TABLE_RSSI_OFFSETS;
    memcpy(&pMsgBody->SetNvTable.tableData.rssiOffset[0], phyRxChains,sizeof(tANI_S16) * PHY_MAX_RX_CHAINS); 
   
    VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO_HIGH, "rssiOffset[0]= %d rssiOffset[1]= %d\n",
                                                            pMsgBody->SetNvTable.tableData.rssiOffset[0],
                                                            pMsgBody->SetNvTable.tableData.rssiOffset[1]);

    status = wlan_ftm_postmsg((v_U8_t*)pMsgBuf,pMsgBuf->msgBodyLength);
    if(status != VOS_STATUS_SUCCESS)
    {        
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:wlan_ftm_postmsg failed",__func__);
        status = VOS_STATUS_E_FAILURE;
        goto done;
    }

    wait_for_completion_interruptible_timeout(&pAdapter->ftm.ftm_comp_var, msecs_to_jiffies(WLAN_FTM_COMMAND_TIME_OUT));
    if(pMsgBuf->msgResponse != PTT_STATUS_SUCCESS)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:Ptt response status failed",__func__);
        status = VOS_STATUS_E_FAILURE;
        goto done;
    }

    init_completion(&pAdapter->ftm.ftm_comp_var);
    pMsgBuf->msgId = PTT_MSG_STORE_NV_TABLE;
    pMsgBuf->msgBodyLength = sizeof(tMsgPttStoreNvTable) + PTT_HEADER_LENGTH;
    pMsgBody = &pMsgBuf->msgBody;
    pMsgBody->StoreNvTable.nvTable= NV_TABLE_RSSI_OFFSETS;

    status = wlan_ftm_postmsg((v_U8_t*)pMsgBuf,pMsgBuf->msgBodyLength);
    if(status != VOS_STATUS_SUCCESS)
    {        
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:wlan_ftm_postmsg failed",__func__);
        status = VOS_STATUS_E_FAILURE;
        goto done;
    }

    wait_for_completion_interruptible_timeout(&pAdapter->ftm.ftm_comp_var, msecs_to_jiffies(WLAN_FTM_COMMAND_TIME_OUT));
    if(pMsgBuf->msgResponse != PTT_STATUS_SUCCESS)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:Ptt response status failed",__func__);
        status = VOS_STATUS_E_FAILURE;
        goto done;
    }
done:
    vos_mem_free((v_VOID_t * )pMsgBuf);

    return status;
}


/**---------------------------------------------------------------------------

  \brief wlan_ftm_rx_mode() -

   This function is used for start/stop the rx packet generation. 

  \param  - pAdapter - Pointer HDD Context.
              - rxmode   -  0-disable RX.
                               -  1-rx ALL frames
                               -  2-rx 11 g/n frames
                               -  3-rx 11b frames
                               
  \return - 0 for success, non zero for failure

  --------------------------------------------------------------------------*/

static VOS_STATUS wlan_ftm_priv_rx_mode(hdd_adapter_t *pAdapter,v_U16_t rxmode)
{
    tPttMsgbuffer *pMsgBuf;
    uPttMsgs *pMsgBody;
    VOS_STATUS status;

    if(pAdapter->ftm.ftm_state != WLAN_FTM_STARTED)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:Ftm has not started.Please start the ftm. ",__func__);
        return VOS_STATUS_E_FAILURE;
    }

    if(!(rxmode >= 0 && rxmode <=3))
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:Tx mode value is invalid ",__func__);
        return VOS_STATUS_E_FAILURE;
    }

    pMsgBuf = (tPttMsgbuffer *)vos_mem_malloc(sizeof(tPttMsgbuffer));

    init_completion(&pAdapter->ftm.ftm_comp_var);
    
    pMsgBuf->msgId = PTT_MSG_SET_RX_DISABLE_MODE;
    pMsgBuf->msgBodyLength = sizeof(tMsgPttSetRxDisableMode) + PTT_HEADER_LENGTH;

    pMsgBody = &pMsgBuf->msgBody;

    switch(rxmode)
    {
        case RXMODE_DISABLE_ALL: 
          pMsgBody->SetRxDisableMode.disabled.agPktsDisabled = VOS_TRUE;
          pMsgBody->SetRxDisableMode.disabled.bPktsDisabled  = VOS_TRUE;
          pMsgBody->SetRxDisableMode.disabled.slrPktsDisabled= VOS_TRUE;
          break;

        case RXMODE_ENABLE_ALL:          
          pMsgBody->SetRxDisableMode.disabled.agPktsDisabled = VOS_FALSE;
          pMsgBody->SetRxDisableMode.disabled.bPktsDisabled  = VOS_FALSE;
          pMsgBody->SetRxDisableMode.disabled.slrPktsDisabled= VOS_FALSE;
          break;

        case RXMODE_ENABLE_11GN:          
          pMsgBody->SetRxDisableMode.disabled.agPktsDisabled = VOS_FALSE;
          pMsgBody->SetRxDisableMode.disabled.bPktsDisabled  = VOS_TRUE;
          pMsgBody->SetRxDisableMode.disabled.slrPktsDisabled= VOS_TRUE;
          break;

        case RXMODE_ENABLE_11B:          
          pMsgBody->SetRxDisableMode.disabled.agPktsDisabled = VOS_TRUE;
          pMsgBody->SetRxDisableMode.disabled.bPktsDisabled  = VOS_FALSE;
          pMsgBody->SetRxDisableMode.disabled.slrPktsDisabled= VOS_TRUE;
          break;
           
    }

    status = wlan_ftm_postmsg((v_U8_t*)pMsgBuf,pMsgBuf->msgBodyLength);

    if(status != VOS_STATUS_SUCCESS)
    {        
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:wlan_ftm_postmsg failed",__func__);
        status = VOS_STATUS_E_FAILURE;
        goto done;
    }
    wait_for_completion_interruptible_timeout(&pAdapter->ftm.ftm_comp_var, msecs_to_jiffies(WLAN_FTM_COMMAND_TIME_OUT));

    if(pMsgBuf->msgResponse != PTT_STATUS_SUCCESS)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:Ptt response status failed",__func__);
        status = VOS_STATUS_E_FAILURE;
        goto done;
    }
    ftm_status.rxmode = rxmode ;
done:
    vos_mem_free((v_VOID_t * )pMsgBuf);

    return status;
}

/**---------------------------------------------------------------------------

  \brief wlan_ftm_priv_rx_pkt_clear() -

   This function sets the rx pkt count to zero. 

  \param  - pAdapter - Pointer HDD Context.
              - rx_pkt_clear   -  rx_pkt_clear value.

  \return - 0 for success, non zero for failure

  --------------------------------------------------------------------------*/

static VOS_STATUS wlan_ftm_priv_rx_pkt_clear(hdd_adapter_t *pAdapter,v_U16_t rx_pkt_clear)
{
    tPttMsgbuffer *pMsgBuf;
    uPttMsgs *pMsgBody;
    VOS_STATUS status;

    if(pAdapter->ftm.ftm_state != WLAN_FTM_STARTED)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:Ftm has not started.Please start the ftm. ",__func__);
        return VOS_STATUS_E_FAILURE;
    }

    if(rx_pkt_clear != 1)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:Invalid rx_pkt_clear value ",__func__);
        return VOS_STATUS_E_FAILURE;
    }

    pMsgBuf = (tPttMsgbuffer *)vos_mem_malloc(sizeof(tPttMsgbuffer));

    init_completion(&pAdapter->ftm.ftm_comp_var);
    pMsgBuf->msgId = PTT_MSG_RESET_RX_PACKET_STATISTICS;
    pMsgBuf->msgBodyLength = sizeof(tMsgPttResetRxPacketStatistics) + PTT_HEADER_LENGTH;

    pMsgBody = &pMsgBuf->msgBody;
    pMsgBody->ResetRxPacketStatistics.notUsed= rx_pkt_clear;

    status = wlan_ftm_postmsg((v_U8_t*)pMsgBuf,pMsgBuf->msgBodyLength);

    if(status != VOS_STATUS_SUCCESS)
    {        
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:wlan_ftm_postmsg failed",__func__);
        status = VOS_STATUS_E_FAILURE;
        goto done;
    }
    wait_for_completion_interruptible_timeout(&pAdapter->ftm.ftm_comp_var, msecs_to_jiffies(WLAN_FTM_COMMAND_TIME_OUT));

    if(pMsgBuf->msgResponse != PTT_STATUS_SUCCESS)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:Ptt response status failed",__func__);
        status = VOS_STATUS_E_FAILURE;
        goto done;
    }
done:
    vos_mem_free((v_VOID_t * )pMsgBuf);

    return status;
}

/**---------------------------------------------------------------------------

  \brief wlan_ftm_priv_get_channel() -

   This function gets the channel number from the halphy ptt module and 
   returns the channel number to the application.

  \param  - pAdapter - Pointer HDD Context.
              - pChannel   -  Poniter to get the Channel number.

  \return - 0 for success, non zero for failure

  --------------------------------------------------------------------------*/

static VOS_STATUS wlan_ftm_priv_get_channel(hdd_adapter_t *pAdapter,v_U16_t *pChannel)
{
    tPttMsgbuffer *pMsgBuf;
    uPttMsgs *pMsgBody;
    VOS_STATUS status;
    v_U16_t  freq;
    v_U8_t indx=0;

    if(pAdapter->ftm.ftm_state != WLAN_FTM_STARTED)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:Ftm has not started.Please start the ftm. ",__func__);
        return VOS_STATUS_E_FAILURE;
    }
    pMsgBuf = (tPttMsgbuffer *)vos_mem_malloc(sizeof(tPttMsgbuffer));

    init_completion(&pAdapter->ftm.ftm_comp_var);
    pMsgBuf->msgId = PTT_MSG_DBG_READ_REGISTER;
    pMsgBuf->msgBodyLength = sizeof(tMsgPttDbgReadRegister) + PTT_HEADER_LENGTH;

    pMsgBody = &pMsgBuf->msgBody;
    pMsgBody->DbgReadRegister.regAddr = QWLAN_AGC_CHANNEL_FREQ_REG;

    status = wlan_ftm_postmsg((v_U8_t*)pMsgBuf,pMsgBuf->msgBodyLength);

    if(status != VOS_STATUS_SUCCESS)
    {        
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:wlan_ftm_postmsg failed",__func__);
        status = VOS_STATUS_E_FAILURE;
        goto done;

    }
    wait_for_completion_interruptible_timeout(&pAdapter->ftm.ftm_comp_var, msecs_to_jiffies(WLAN_FTM_COMMAND_TIME_OUT));

    if(pMsgBuf->msgResponse != PTT_STATUS_SUCCESS)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:Ptt response status failed",__func__);
        status = VOS_STATUS_E_FAILURE;
        goto done;
    }
    
    freq = ((v_U16_t)pMsgBody->DbgReadRegister.regValue & QWLAN_AGC_CHANNEL_FREQ_FREQ_MASK);

    while ((indx <  SIZE_OF_TABLE(freq_chan_tbl)) && (freq != freq_chan_tbl[indx].freq))
            indx++; 
    if (indx >= SIZE_OF_TABLE(freq_chan_tbl))
    {        
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:Invalid Frequency!!!",__func__);
        status = VOS_STATUS_E_FAILURE;
        goto done;
    }

    *pChannel = freq_chan_tbl[indx].chan;
    
     VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO_HIGH, "Channel = %d  freq = %d\n",*pChannel, freq);
 done:    
     vos_mem_free((v_VOID_t * )pMsgBuf);

     return status;
}

/**---------------------------------------------------------------------------

  \brief wlan_ftm_priv_get_txpower() -

   This function gets the TX power from the halphy ptt module and 
   returns the TX power to the application.

  \param  - pAdapter - Pointer HDD Context.
              - pTxPwr   -  Poniter to get the Tx power.

  \return - 0 for success, non zero for failure

  --------------------------------------------------------------------------*/

static VOS_STATUS wlan_ftm_priv_get_txpower(hdd_adapter_t *pAdapter,v_U16_t *pTxPwr)
{
    tPttMsgbuffer *pMsgBuf;
    uPttMsgs *pMsgBody;
    VOS_STATUS status;

    if(pAdapter->ftm.ftm_state != WLAN_FTM_STARTED)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:Ftm has not started.Please start the ftm. ",__func__);
        return VOS_STATUS_E_FAILURE;
    }
    pMsgBuf = (tPttMsgbuffer *)vos_mem_malloc(sizeof(tPttMsgbuffer));

    init_completion(&pAdapter->ftm.ftm_comp_var);
    pMsgBuf->msgId = PTT_MSG_GET_TX_POWER_REPORT;
    pMsgBuf->msgBodyLength = sizeof(tMsgPttGetTxPowerReport) + PTT_HEADER_LENGTH;

    pMsgBody = &pMsgBuf->msgBody;

    status = wlan_ftm_postmsg((v_U8_t*)pMsgBuf,pMsgBuf->msgBodyLength);

    if(status != VOS_STATUS_SUCCESS)
    {        
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:wlan_ftm_postmsg failed",__func__);
        status = VOS_STATUS_E_FAILURE;
        goto done;
    }
    wait_for_completion_interruptible_timeout(&pAdapter->ftm.ftm_comp_var, msecs_to_jiffies(WLAN_FTM_COMMAND_TIME_OUT));

    if(pMsgBuf->msgResponse != PTT_STATUS_SUCCESS)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s: PTT_MSG_GET_TX_POWER_REPORT failed",__func__);
        status = VOS_STATUS_E_FAILURE;
        goto done;
    }
    *pTxPwr = ((((pMsgBody->GetTxPowerReport.pwrTemplateIndex & 0x1F) + 4)*50)/100);

 done: 
     vos_mem_free((v_VOID_t * )pMsgBuf);

     return status;
}

/**---------------------------------------------------------------------------

  \brief wlan_ftm_priv_get_ftm_version() -

   This function gets ftm driver and firmware version.

  \param  - pAdapter - Pointer HDD Context.
              - pTxRate   -  Poniter to get the Tx rate.

  \return - 0 for success, non zero for failure

  --------------------------------------------------------------------------*/

VOS_STATUS wlan_ftm_priv_get_ftm_version(hdd_adapter_t *pAdapter,char *pftmVer)
{
    tPttMsgbuffer *pMsgBuf;
    uPttMsgs *pMsgBody;
    VOS_STATUS status;
    v_U32_t reg_val;
    char *buf = pftmVer;
    FwVersionInfo *pFwVersion;

    if(pAdapter->ftm.ftm_state != WLAN_FTM_STARTED)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:Ftm has not started.Please start the ftm. ",__func__);
        return VOS_STATUS_E_FAILURE;
    }

    pMsgBuf = (tPttMsgbuffer *)vos_mem_malloc(sizeof(tPttMsgbuffer));

    init_completion(&pAdapter->ftm.ftm_comp_var);
    pMsgBuf->msgId = PTT_MSG_DBG_READ_REGISTER;
    pMsgBuf->msgBodyLength = sizeof(tMsgPttDbgReadRegister) + PTT_HEADER_LENGTH;

    pMsgBody = &pMsgBuf->msgBody;
    pMsgBody->DbgReadRegister.regAddr = QWLAN_RFAPB_REV_ID_REG;

    status = wlan_ftm_postmsg((v_U8_t*)pMsgBuf,pMsgBuf->msgBodyLength);

    if(status != VOS_STATUS_SUCCESS)
    {        
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:wlan_ftm_postmsg failed",__func__);
        status = VOS_STATUS_E_FAILURE;
        goto done;

    }
    wait_for_completion_interruptible_timeout(&pAdapter->ftm.ftm_comp_var, msecs_to_jiffies(WLAN_FTM_COMMAND_TIME_OUT));

    if(pMsgBuf->msgResponse != PTT_STATUS_SUCCESS)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:Ptt response status failed",__func__);
        status = VOS_STATUS_E_FAILURE;
        goto done;
    }
    
    reg_val = (v_U16_t)pMsgBody->DbgReadRegister.regValue;

    init_completion(&pAdapter->ftm.ftm_comp_var);

    pMsgBuf->msgId = PTT_MSG_GET_BUILD_RELEASE_NUMBER;
    pMsgBuf->msgBodyLength = sizeof(tMsgPttGetBuildReleaseNumber) + PTT_HEADER_LENGTH;

    pMsgBody = &pMsgBuf->msgBody;

    status = wlan_ftm_postmsg((v_U8_t*)pMsgBuf,pMsgBuf->msgBodyLength);

    if(status != VOS_STATUS_SUCCESS)
    {        
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:wlan_ftm_postmsg failed",__func__);
        status = VOS_STATUS_E_FAILURE;
        goto done;
    }
    wait_for_completion_interruptible_timeout(&pAdapter->ftm.ftm_comp_var, msecs_to_jiffies(WLAN_FTM_COMMAND_TIME_OUT));

    
    buf += sprintf(buf,"%s_",WLAN_CHIP_VERSION);
    /*Read the RevID*/
    buf += sprintf(buf,"%x.%x-",(v_U8_t)(reg_val >> 8), (v_U8_t)(reg_val & 0x000000FF));
    
    buf += sprintf(buf,"%s-", QWLAN_VERSIONSTR);
    pFwVersion = &pMsgBody->GetBuildReleaseNumber.relParams.fwVer;
    buf += sprintf(buf,"%ld.%ld.%ld.%ld",pFwVersion->uMj,pFwVersion->uMn,pFwVersion->uPatch,pFwVersion->uBuild);            

done:
    vos_mem_free((v_VOID_t * )pMsgBuf);

    return status;

}

/**---------------------------------------------------------------------------

  \brief wlan_ftm_priv_get_txrate() -

   This function gets the TX rate from the halphy ptt module and 
   returns the TX rate to the application.

  \param  - pAdapter - Pointer HDD Context.
              - pTxRate   -  Poniter to get the Tx rate.

  \return - 0 for success, non zero for failure

  --------------------------------------------------------------------------*/

static VOS_STATUS wlan_ftm_priv_get_txrate(hdd_adapter_t *pAdapter,char *pTxRate)
{
    tPttMsgbuffer *pMsgBuf;
    uPttMsgs *pMsgBody;
    VOS_STATUS status;
    v_U16_t rate_index,ii;

    if(pAdapter->ftm.ftm_state != WLAN_FTM_STARTED)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:Ftm has not started.Please start the ftm. ",__func__);
        return VOS_STATUS_E_FAILURE;
    }

    pMsgBuf = (tPttMsgbuffer *)vos_mem_malloc(sizeof(tPttMsgbuffer));

    init_completion(&pAdapter->ftm.ftm_comp_var);
    pMsgBuf->msgId = PTT_MSG_GET_TX_POWER_REPORT;
    pMsgBuf->msgBodyLength = sizeof(tMsgPttGetTxPowerReport) + PTT_HEADER_LENGTH;

    pMsgBody = &pMsgBuf->msgBody;

    status = wlan_ftm_postmsg((v_U8_t*)pMsgBuf,pMsgBuf->msgBodyLength);

    if(status != VOS_STATUS_SUCCESS)
    {        
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:wlan_ftm_postmsg failed",__func__);
        status = VOS_STATUS_E_FAILURE;
        goto done;
    }
    wait_for_completion_interruptible_timeout(&pAdapter->ftm.ftm_comp_var, msecs_to_jiffies(WLAN_FTM_COMMAND_TIME_OUT));

    if(pMsgBuf->msgResponse == PTT_STATUS_SUCCESS) {
      
       rate_index = pMsgBody->GetTxPowerReport.rate;
    }
    else {
       /*Return the default rate*/
       //rate_index = HAL_PHY_RATE_11A_6_MBPS;
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s: PTT_MSG_GET_TX_POWER_REPORT failed",__func__);
        status = VOS_STATUS_E_FAILURE;
        goto done;
    }

    for(ii = 0; ii < SIZE_OF_TABLE(rateName_rateIndex_tbl); ii++) {
        if(rateName_rateIndex_tbl[ii].rate_index == rate_index)
          break;
    }
    if(ii >= SIZE_OF_TABLE(rateName_rateIndex_tbl))
    {        
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:Invalid Rate Index\n",__func__);
        status = VOS_STATUS_E_FAILURE;
        goto done;
    }
    strcpy(pTxRate,rateName_rateIndex_tbl[ii].rate_str);
done:
    vos_mem_free((v_VOID_t * )pMsgBuf);

    return status;

}

/**---------------------------------------------------------------------------

  \brief wlan_ftm_priv_get_rx_pkt_count() -

   This function gets the rx pkt count from the halphy ptt module and 
   returns the rx pkt count  to the application.

  \param  - pAdapter - Pointer HDD Context.
              - pRxPktCnt   -  Poniter to get the rx pkt count.

  \return - 0 for success, non zero for failure

  --------------------------------------------------------------------------*/

static VOS_STATUS wlan_ftm_priv_get_rx_pkt_count(hdd_adapter_t *pAdapter,v_U16_t *pRxPktCnt)
{
    tPttMsgbuffer *pMsgBuf;
    uPttMsgs *pMsgBody;
    VOS_STATUS status;

    if(pAdapter->ftm.ftm_state != WLAN_FTM_STARTED)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:Ftm has not started.Please start the ftm. ",__func__);
        return VOS_STATUS_E_FAILURE;
    }
    pMsgBuf = (tPttMsgbuffer *)vos_mem_malloc(sizeof(tPttMsgbuffer));

    init_completion(&pAdapter->ftm.ftm_comp_var);
    pMsgBuf->msgId = PTT_MSG_GET_RX_PKT_COUNTS;
    pMsgBuf->msgBodyLength = sizeof(tMsgPttGetRxPktCounts) + PTT_HEADER_LENGTH;

    pMsgBody = &pMsgBuf->msgBody;

    status = wlan_ftm_postmsg((v_U8_t*)pMsgBuf,pMsgBuf->msgBodyLength);

    if(status != VOS_STATUS_SUCCESS)
    {        
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:wlan_ftm_postmsg failed",__func__);
        status = VOS_STATUS_E_FAILURE;
        goto done;
    }
    wait_for_completion_interruptible_timeout(&pAdapter->ftm.ftm_comp_var, msecs_to_jiffies(WLAN_FTM_COMMAND_TIME_OUT));

    if(pMsgBuf->msgResponse != PTT_STATUS_SUCCESS)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:Ptt response status failed",__func__);
        status = VOS_STATUS_E_FAILURE;
        goto done;
    } 
    *pRxPktCnt = pMsgBody->GetRxPktCounts.counters.totalRxPackets;
done:
    vos_mem_free((v_VOID_t * )pMsgBuf);
     
    return status;
}

/**---------------------------------------------------------------------------

  \brief wlan_ftm_priv_get_mac_address() -

   This function gets the mac address from the halphy ptt module and 
   returns the mac address  to the application.

  \param  - pAdapter - Pointer HDD Context.
              - buf   -  Poniter to get the mac address.

  \return - 0 for success, non zero for failure

  --------------------------------------------------------------------------*/

static VOS_STATUS wlan_ftm_priv_get_mac_address(hdd_adapter_t *pAdapter,char *buf)
{
    v_BOOL_t itemIsValid = VOS_FALSE;
    v_U8_t macAddr[VOS_MAC_ADDRESS_LEN] = {0, 0x0a, 0xf5, 4,5, 6};
    int ret;

    if(pAdapter->ftm.ftm_state != WLAN_FTM_STARTED)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:Ftm has not started.Please start the ftm. ",__func__);
        return VOS_STATUS_E_FAILURE;
    }
    /*Check the NV FIELD is valid or not*/
    if (vos_nv_getValidity(VNV_FIELD_IMAGE, &itemIsValid) == VOS_STATUS_SUCCESS)
    {
        if (itemIsValid == VOS_TRUE) {

            vos_nv_readMacAddress(macAddr); 
            ret = sprintf(buf, "%02x:%02x:%02x:%02x:%02x:%02x", MAC_ADDR_ARRAY(macAddr));
        }
    }
    else {
         /*Return Hard coded mac address*/
         ret = sprintf(buf, "%02x:%02x:%02x:%02x:%02x:%02x", MAC_ADDR_ARRAY(macAddr));
    }
    return VOS_STATUS_SUCCESS;
}

/**---------------------------------------------------------------------------

  \brief wlan_ftm_priv_set_mac_address() -

   This function sets the mac address to the halphy ptt module and 
   sends the netlink message to the ptt socket application which writes
   the macaddress to the qcom_wlan_nv.bin file 

  \param  - pAdapter - Pointer HDD Context.
              - buf   -  Poniter to the macaddress.

  \return - 0 for success, non zero for failure

  --------------------------------------------------------------------------*/

static VOS_STATUS wlan_ftm_priv_set_mac_address(hdd_adapter_t *pAdapter,char *buf)
{
    tPttMsgbuffer *pMsgBuf;
    uPttMsgs *pMsgBody;
    VOS_STATUS status;
    int macAddr[VOS_MAC_ADDRESS_LEN];
    v_U8_t *pMacAddress;
    v_U8_t  ii;

    if(pAdapter->ftm.ftm_state != WLAN_FTM_STARTED)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:Ftm has not started.Please start the ftm. ",__func__);
        return VOS_STATUS_E_FAILURE;
    }
    pMsgBuf = (tPttMsgbuffer *)vos_mem_malloc(sizeof(tPttMsgbuffer));
   
    init_completion(&pAdapter->ftm.ftm_comp_var);
    pMsgBuf->msgId = PTT_MSG_SET_NV_FIELD;
    pMsgBuf->msgBodyLength = sizeof(tMsgPttSetNvField) + PTT_HEADER_LENGTH;
    
    pMsgBody = &pMsgBuf->msgBody;
    pMsgBody->SetNvField.nvField = NV_COMMON_MAC_ADDR;
 
    /*We get the mac address in string format "XX:XX:XX:XX:XX:XX" convert to hex*/
    sscanf(buf,"%02x:%02x:%02x:%02x:%02x:%02x",&macAddr[0],(int*)&macAddr[1],(int*)&macAddr[2],(int*)&macAddr[3],(int*)&macAddr[4],(int*)&macAddr[5]);
 
    VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO_HIGH, "MacAddress = %02x:%02x:%02x:%02x:%02x:%02x",MAC_ADDR_ARRAY(macAddr));

    
    pMacAddress = &pMsgBody->SetNvField.fieldData.macAddr[0];

    for(ii = 0; ii < VOS_MAC_ADDRESS_LEN; ii++)
       pMacAddress[ii] = (v_U8_t)macAddr[ii];

    
    VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO_HIGH, "pMacAddress = %02x:%02x:%02x:%02x:%02x:%02x",MAC_ADDR_ARRAY(pMacAddress));
    status = wlan_ftm_postmsg((v_U8_t*)pMsgBuf,pMsgBuf->msgBodyLength);

    if(status != VOS_STATUS_SUCCESS)
    {        
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:wlan_ftm_postmsg failed!!",__func__);
        status = VOS_STATUS_E_FAILURE;
        goto done;
    }
    wait_for_completion_interruptible_timeout(&pAdapter->ftm.ftm_comp_var, msecs_to_jiffies(WLAN_FTM_COMMAND_TIME_OUT));
 
    if(pMsgBuf->msgResponse != PTT_STATUS_SUCCESS)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:Ptt response status failed",__func__);
        status = VOS_STATUS_E_FAILURE;
        goto done;
    }

    VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO_HIGH, "NV_COMMON_MAC_ADDR Success!!!\n");

    init_completion(&pAdapter->ftm.ftm_comp_var);
    memset( pMsgBuf,0,sizeof(tPttMsgbuffer)); 
 
    pMsgBuf->msgId = PTT_MSG_STORE_NV_TABLE;
    pMsgBuf->msgBodyLength = sizeof(tMsgPttStoreNvTable) + PTT_HEADER_LENGTH;
    
    pMsgBody = &pMsgBuf->msgBody;
 
    pMsgBody->StoreNvTable.nvTable = NV_FIELDS_IMAGE;
    
    status = wlan_ftm_postmsg((v_U8_t*)pMsgBuf,pMsgBuf->msgBodyLength);

    if(status != VOS_STATUS_SUCCESS)
    {        
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:wlan_ftm_postmsg failed!!!!",__func__);
        status = VOS_STATUS_E_FAILURE;
        goto done;
    }
    
    wait_for_completion_interruptible_timeout(&pAdapter->ftm.ftm_comp_var, msecs_to_jiffies(WLAN_FTM_COMMAND_TIME_OUT));
done:
    vos_mem_free((v_VOID_t * )pMsgBuf);

    return VOS_STATUS_SUCCESS;
}

/* set param sub-ioctls */
static int iw_ftm_setchar_getnone(struct net_device *dev, struct iw_request_info *info,
                       union iwreq_data *wrqu, char *extra)
{
    int sub_cmd = wrqu->data.flags;
    int ret = 0; /* sucess */
    VOS_STATUS status; 
    hdd_adapter_t *pAdapter = (netdev_priv(dev));

    VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO, "%s: Received length %d", __FUNCTION__, wrqu->data.length);
    VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO, "%s: Received data %s", __FUNCTION__, (char*)wrqu->data.pointer);
    
    switch(sub_cmd) 
    {
       case WE_SET_MAC_ADDRESS:
       {

          VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO, "SET MAC ADDRESS\n");

          status  = wlan_ftm_priv_set_mac_address(pAdapter,(char*)wrqu->data.pointer);

          if(status != VOS_STATUS_SUCCESS) 
          {
             hddLog(VOS_TRACE_LEVEL_FATAL,"wlan_ftm_priv_set_mac_address Failed =%d\n",status);
             ret = -EINVAL;
          }
          
       }
       break;
       case WE_SET_TX_RATE:
       { 
            status  = wlan_ftm_priv_set_txrate(pAdapter,(char*)wrqu->data.pointer);

            if(status != VOS_STATUS_SUCCESS) 
            {
               hddLog(VOS_TRACE_LEVEL_FATAL,"wlan_ftm_priv_set_txrate Failed =%d\n",status);
               ret = -EINVAL;
            }
            
            break;
        }
       default:  
       {
           hddLog(LOGE, "%s: Invalid sub command %d\n",__FUNCTION__, sub_cmd);
           ret = -EINVAL;
           break;
       }
    }
    return ret;
}

static int iw_ftm_setint_getnone(struct net_device *dev, struct iw_request_info *info,
                       union iwreq_data *wrqu, char *extra)
{
    hdd_adapter_t *pAdapter = (netdev_priv(dev));
    int *value = (int *)extra;
    int sub_cmd = value[0];
    int set_value = value[1];
    int ret = 0; /* success */
    VOS_STATUS status;
    
    switch(sub_cmd)
    {
        case WE_FTM_ON_OFF:
        { 
            status  = wlan_ftm_priv_start_stop_ftm(pAdapter,set_value);

            if(status != VOS_STATUS_SUCCESS) 
            {
               hddLog(VOS_TRACE_LEVEL_FATAL,"%s Failed =%d\n",__FUNCTION__, status);
               ret = -EINVAL;
            }

            break;
        }

        case WE_TX_PKT_GEN:
            status  = wlan_ftm_priv_start_stop_tx_pktgen(pAdapter,set_value);

            if(status != VOS_STATUS_SUCCESS) 
            {
               hddLog(VOS_TRACE_LEVEL_FATAL,"wlan_ftm_priv_start_stop_tx_pktgen Failed =%d\n",status);
               ret = -EINVAL;
            }
            break;

        case WE_SET_TX_IFS:
            status  = wlan_ftm_priv_set_txifs(pAdapter,set_value);

            if(status != VOS_STATUS_SUCCESS) 
            {
               hddLog(VOS_TRACE_LEVEL_FATAL,"wlan_ftm_priv_set_txifs Failed =%d\n",status);
               ret = -EINVAL;
            }
            break;

        case WE_SET_TX_PKT_CNT:
            status  = wlan_ftm_priv_set_txpktcnt(pAdapter,set_value);

            if(status != VOS_STATUS_SUCCESS) 
            {
               hddLog(VOS_TRACE_LEVEL_FATAL,"wlan_ftm_priv_set_txpktcnt Failed =%d\n",status);
               ret = -EINVAL;
            }
            break;

        case WE_SET_TX_PKT_LEN:
            status  = wlan_ftm_priv_set_txpktlen(pAdapter,set_value);

            if(status != VOS_STATUS_SUCCESS) 
            {
               hddLog(VOS_TRACE_LEVEL_FATAL,"wlan_ftm_priv_set_txpktlen Failed =%d\n",status);
               ret = -EINVAL;
            }
            break;

        case WE_SET_CHANNEL:
        { 
            status  = wlan_ftm_priv_set_channel(pAdapter,set_value);

            if(status != VOS_STATUS_SUCCESS) 
            {
               hddLog(VOS_TRACE_LEVEL_FATAL,"wlan_ftm_priv_set_channel Failed =%d\n",status);
               ret = -EINVAL;
            }
            break;
        }
        case WE_SET_TX_POWER:
        { 
            status  = wlan_ftm_priv_set_txpower(pAdapter,set_value);
            
            if(status != VOS_STATUS_SUCCESS) 
            {
               hddLog(VOS_TRACE_LEVEL_FATAL,"wlan_ftm_priv_set_txpower Failed =%d\n",status);
               ret = -EINVAL;
            }
            break;
        }
        case WE_CLEAR_RX_PKT_CNT:
        { 
            status  = wlan_ftm_priv_rx_pkt_clear(pAdapter,set_value);

            if(status != VOS_STATUS_SUCCESS) 
            {
               hddLog(VOS_TRACE_LEVEL_FATAL,"wlan_ftm_priv_rx_pkt_clear Failed =%d\n",status);
               ret = -EINVAL;
            }
            break;
        }           
        case WE_RX:
        { 
            status  = wlan_ftm_priv_rx_mode(pAdapter,set_value);

            if(status != VOS_STATUS_SUCCESS) 
            {
               hddLog(VOS_TRACE_LEVEL_FATAL,"wlan_ftm_priv_rx_mode Failed =%d\n",status);
               ret = -EINVAL;
            }
            break;
        }
        case WE_ENABLE_CHAIN:
        { 
            status  = wlan_ftm_priv_enable_chain(pAdapter,set_value);

            if(status != VOS_STATUS_SUCCESS) 
            {
               hddLog(VOS_TRACE_LEVEL_FATAL,"wlan_ftm_priv_enable_chain Failed =%d\n",status);
               ret = -EINVAL;
            }
            break;
        }

        default:  
        {
            hddLog(LOGE, "Invalid IOCTL setvalue command %d value %d \n",
                sub_cmd, set_value);
            break;
        }
    }

    return ret;
}

/* get param sub-ioctls */
static int iw_ftm_setnone_getint(struct net_device *dev, struct iw_request_info *info,
                       union iwreq_data *wrqu, char *extra)
{  
    hdd_adapter_t *pAdapter = (netdev_priv(dev));   
    int *value = (int *)extra;
    int ret = 0; /* sucess */
    VOS_STATUS status;

    switch (value[0])
    {
        case WE_GET_CHANNEL:
        {
           status = wlan_ftm_priv_get_channel(pAdapter,(v_U16_t*)value);

           if(status != VOS_STATUS_SUCCESS) 
           {
              hddLog(VOS_TRACE_LEVEL_FATAL,"wlan_ftm_priv_get_channel Failed =%d\n",status);
              ret = -EINVAL;
           }
           break;            
        }
        case WE_GET_TX_POWER:
        {
           status = wlan_ftm_priv_get_txpower(pAdapter,(v_U16_t*)value);

           if(status != VOS_STATUS_SUCCESS) 
           {
              hddLog(VOS_TRACE_LEVEL_FATAL,"wlan_ftm_priv_get_txpower Failed =%d\n",status);
              ret = -EINVAL;
           }
           break;            
        }
        case WE_GET_RX_PKT_CNT:
        {
           status = wlan_ftm_priv_get_rx_pkt_count(pAdapter,(v_U16_t*)value);

           if(status != VOS_STATUS_SUCCESS) 
           {
              hddLog(VOS_TRACE_LEVEL_FATAL,"wlan_ftm_priv_get_rx_pkt_count Failed =%d\n",status);
              ret = -EINVAL;
           }
           break;            
        }
        default:
        {
            hddLog(LOGE, "Invalid IOCTL get_value command %d ",value[0]);
            break;
        }
    }

    return ret;
}

static int iw_ftm_get_char_setnone(struct net_device *dev, struct iw_request_info *info,
                       union iwreq_data *wrqu, char *extra)
{    
    int sub_cmd = wrqu->data.flags;
    VOS_STATUS status;
    hdd_adapter_t *pAdapter = (netdev_priv(dev));

    switch(sub_cmd)
    {
        case WE_GET_MAC_ADDRESS:
        {
            status = wlan_ftm_priv_get_mac_address(pAdapter, extra);

            if(status != VOS_STATUS_SUCCESS) 
            {
                hddLog(VOS_TRACE_LEVEL_FATAL, "wlan_ftm_priv_get_mac_address failed =%d\n",status);
                return -EINVAL;
            }
            wrqu->data.length = strlen(extra)+1;
            break;
        }
        case WE_GET_TX_RATE:
        {
            status = wlan_ftm_priv_get_txrate(pAdapter, extra);

            if(status != VOS_STATUS_SUCCESS) 
            {
                hddLog(VOS_TRACE_LEVEL_FATAL, "wlan_ftm_priv_get_txrate failed =%d\n",status);
                return -EINVAL;
            }

            wrqu->data.length = strlen(extra)+1;
            break;
        }
        case WE_GET_FTM_VERSION:
        {
            status = wlan_ftm_priv_get_ftm_version(pAdapter, extra);

            if(status != VOS_STATUS_SUCCESS) 
            {
                hddLog(VOS_TRACE_LEVEL_FATAL, "wlan_ftm_priv_get_mac_address failed =%d\n",status);
                return -EINVAL;
            }
            wrqu->data.length = strlen(extra)+1;
            break;
        }
        case WE_GET_FTM_STATUS:
        {
            status = wlan_ftm_priv_get_status(pAdapter, extra);

            if(status != VOS_STATUS_SUCCESS) 
            {
                hddLog(VOS_TRACE_LEVEL_FATAL, "wlan_ftm_priv_get_status failed =%d\n",status);
                return -EINVAL;
            }

            wrqu->data.length = strlen(extra)+1;
            break;
        }
        default:  
        {
            hddLog(LOGE, "Invalid IOCTL command %d  \n",  sub_cmd );
            break;
        }
    }

    return 0;
}
#endif//ANI_MANF_DIAG

VOS_STATUS wlan_write_to_efs (v_U8_t *pData, v_U16_t data_len)
{
#ifdef ANI_MANF_DIAG
    tAniHdr *wmsg = NULL;
    v_U8_t *pBuf;
    hdd_adapter_t *pAdapter;
    v_CONTEXT_t pVosContext= NULL;

    pBuf =  (v_U8_t*)vos_mem_malloc(sizeof(tAniHdr) + sizeof(v_U32_t)+ data_len);

    wmsg = (tAniHdr*)pBuf;
    wmsg->type = PTT_MSG_FTM_CMDS_TYPE;
    wmsg->length = data_len + sizeof(tAniHdr)+ sizeof(v_U32_t);
    wmsg->length = FTM_SWAP16(wmsg->length);
    pBuf += sizeof(tAniHdr);

     /*Get the global context */
    pVosContext = vos_get_global_context(VOS_MODULE_ID_SYS, NULL);

     /*Get the Hdd Context */
    pAdapter = ((VosContextType*)(pVosContext))->pHDDContext;

    /* EfS command Code */
    *(v_U32_t*)pBuf = 0x000000EF;

    pBuf += sizeof(v_U32_t);

    memcpy(pBuf, pData,data_len);

   if(pAdapter->ftm.cmd_iwpriv == TRUE) {
       if( ptt_sock_send_msg_to_app(wmsg, 0, ANI_NL_MSG_PUMAC, pAdapter->ptt_pid) < 0) {
   
           VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR, ("Ptt Socket error sending message to the app!!\n"));
           return VOS_STATUS_E_FAILURE;
       }
   }
   else {
    if( ptt_sock_send_msg_to_app(wmsg, 0, ANI_NL_MSG_PUMAC, pAdapter->ftm.wnl->nlh.nlmsg_pid) < 0) {

        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR, ("Ptt Socket error sending message to the app!!\n"));
        return VOS_STATUS_E_FAILURE;
    }
   }

    vos_mem_free((v_VOID_t*)wmsg);
#endif

    return VOS_STATUS_SUCCESS;
}

#ifdef ANI_MANF_DIAG
/*  action sub-ioctls */
static int iw_ftm_setnone_getnone(struct net_device *dev, struct iw_request_info *info,
                       union iwreq_data *wrqu, char *extra)
{  
    int sub_cmd = wrqu->data.flags;
    int ret = 0; /* sucess */
    
    switch (sub_cmd)
    {
        case WE_SET_NV_DEFAULTS:
        {
            v_U8_t *pu8buf,*pTempBuf;
            v_U16_t size;
            size = sizeof(v_U32_t) + sizeof(sHalNv);
            hddLog(VOS_TRACE_LEVEL_INFO_HIGH,"HAL NV Size =%d\n",size);
            pu8buf = vos_mem_malloc(size);
            memset(pu8buf,0,size);
            pTempBuf = pu8buf;
            pTempBuf += sizeof(v_U32_t);
            memcpy(pTempBuf,&nvDefaults,sizeof(sHalNv));
         
            wlan_write_to_efs(pu8buf,size);
            vos_mem_free(pu8buf);
        }

        default:
        {
            VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,"%s: unknown ioctl %d", __FUNCTION__, sub_cmd);
            hddLog(LOGE, "Invalid IOCTL action command %d ", sub_cmd);
            break;
        }
    }

    return ret;
}

static int iw_ftm_set_var_ints_getnone(struct net_device *dev, struct iw_request_info *info,
        union iwreq_data *wrqu, char *extra)
{   

    hdd_adapter_t *pAdapter = (netdev_priv(dev));
    int sub_cmd = wrqu->data.flags;
    int *value = (int*)wrqu->data.pointer;
    v_S15_t phyRxChains[MAX_FTM_VAR_ARGS] = {0};

    if(wrqu->data.length != 2)
    {
        hddLog(LOGE, "Invalid number of Arguments  %d  \n",  wrqu->data.length);
        return -EINVAL;
    }
    switch (sub_cmd)
    {
        case WE_SET_RSSI_OFFSET:
        {
            phyRxChains[0] = *(v_S15_t*) value++;
            phyRxChains[1] = *(v_S15_t*) value;
            VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO_HIGH,"PhyRxchain0=%d PhyRxChain1=%d\n",phyRxChains[0],phyRxChains[1]);
            wlan_ftm_priv_set_rssi_offset(pAdapter, &phyRxChains[0]);
        }
        break;

        default:  
        {
            hddLog(LOGE, "Invalid IOCTL command %d  \n",  sub_cmd );
            break;
        }
    }

    return 0;
}



static const iw_handler we_ftm_private[] = {
   
   [WLAN_FTM_PRIV_SET_INT_GET_NONE      - SIOCIWFIRSTPRIV]   = iw_ftm_setint_getnone,  //set priv ioctl
   [WLAN_FTM_PRIV_SET_NONE_GET_INT      - SIOCIWFIRSTPRIV]   = iw_ftm_setnone_getint,  //get priv ioctl   
   [WLAN_FTM_PRIV_SET_CHAR_GET_NONE     - SIOCIWFIRSTPRIV]   = iw_ftm_setchar_getnone, //get priv ioctl   
   [WLAN_FTM_PRIV_GET_CHAR_SET_NONE     - SIOCIWFIRSTPRIV]   = iw_ftm_get_char_setnone,   
   [WLAN_FTM_PRIV_SET_NONE_GET_NONE     - SIOCIWFIRSTPRIV]   = iw_ftm_setnone_getnone, //action priv ioctl	 
   [WLAN_FTM_PRIV_SET_VAR_INT_GET_NONE  - SIOCIWFIRSTPRIV]   = iw_ftm_set_var_ints_getnone,   
};

/*Maximum command length can be only 15 */
static const struct iw_priv_args we_ftm_private_args[] = {

    /* handlers for main ioctl */
    {   WLAN_FTM_PRIV_SET_INT_GET_NONE,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
        0, 
        "" },

    {   WE_FTM_ON_OFF,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
        0, 
        "ftm" },        

    {   WE_TX_PKT_GEN,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
        0, 
        "tx" },

    {   WE_SET_TX_IFS,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
        0, 
        "set_txifs" },

    {   WE_SET_TX_PKT_CNT,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
        0, 
        "set_txpktcnt" },

    {   WE_SET_TX_PKT_LEN,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
        0, 
        "set_txpktlen" },

    {   WE_SET_CHANNEL,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
        0, 
        "set_channel" },        
        
    {   WE_SET_TX_POWER,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
        0, 
        "set_txpower" },
        
    {   WE_CLEAR_RX_PKT_CNT,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
        0, 
        "clr_rxpktcnt" },    
        
    {   WE_RX,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
        0, 
        "rx" },    

    {   WE_ENABLE_CHAIN,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
        0, 
        "ena_chain" },    

    /* handlers for main ioctl */
    {   WLAN_FTM_PRIV_SET_NONE_GET_INT,
        0, 
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
        "" },

    {   WE_GET_CHANNEL,
        0,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
        "get_channel" },    
        
    {   WE_GET_TX_POWER,
        0,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
        "get_txpower" },
               
    {   WE_GET_RX_PKT_CNT,
        0,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
        "get_rxpktcnt" },

    /* handlers for main ioctl */
    {   WLAN_FTM_PRIV_SET_CHAR_GET_NONE,
        IW_PRIV_TYPE_CHAR| 512,
        0, 
        "" },

    {   WE_SET_MAC_ADDRESS,
        IW_PRIV_TYPE_CHAR| 512,
        0, 
        "set_mac_address" },
        
    {   WE_SET_TX_RATE,
        IW_PRIV_TYPE_CHAR | 512,
        0, 
        "set_txrate" },

    /* handlers for main ioctl */
    {   WLAN_FTM_PRIV_GET_CHAR_SET_NONE,
        0,
        IW_PRIV_TYPE_CHAR| WE_FTM_MAX_STR_LEN,
        "" },
        
    {   WE_GET_MAC_ADDRESS,
        0,
        IW_PRIV_TYPE_CHAR| WE_FTM_MAX_STR_LEN,
        "get_mac_address" },
        
    {   WE_GET_FTM_VERSION,
        0,
        IW_PRIV_TYPE_CHAR| WE_FTM_MAX_STR_LEN,
        "ftm_version" },        
        
    {   WE_GET_TX_RATE,
        0,
        IW_PRIV_TYPE_CHAR| WE_FTM_MAX_STR_LEN,
        "get_txrate" },    

    {   WE_GET_FTM_STATUS,
        0,
        IW_PRIV_TYPE_CHAR| WE_FTM_MAX_STR_LEN,
        "get_status" },    

    {   WLAN_FTM_PRIV_SET_VAR_INT_GET_NONE,
        IW_PRIV_TYPE_INT | MAX_FTM_VAR_ARGS,
        0, 
        "" },
        
    {   WE_SET_RSSI_OFFSET,
        IW_PRIV_TYPE_INT| MAX_FTM_VAR_ARGS,
        0,
        "set_rssi_offset" }, 
        
    /* handlers for main ioctl */
    {   WLAN_FTM_PRIV_SET_NONE_GET_NONE,
        0,
        0, 
        "" },

    /* handlers for sub-ioctl */
    {   WE_SET_NV_DEFAULTS,
        0,
        0, 
        "set_nv_defaults" },

};

const struct iw_handler_def we_ftm_handler_def = {
   .num_standard     = 0,
   .num_private      = sizeof(we_ftm_private) / sizeof(we_ftm_private[0]),
   .num_private_args = sizeof(we_ftm_private_args) / sizeof(we_ftm_private_args[0]),

   .standard         = (iw_handler *)NULL,
   .private          = (iw_handler *)we_ftm_private,
   .private_args     = we_ftm_private_args,
   .get_wireless_stats = NULL,
};

static void wlan_ftm_register_wext(hdd_adapter_t *pAdapter)
{
    
    hdd_wext_state_t *pwextBuf;
    pwextBuf = kmalloc(sizeof(hdd_wext_state_t), GFP_KERNEL);

    if( !pwextBuf ) {
        hddLog( LOG1,"VOS unable to allocate memory\n");
        return;
    }
    // Zero the memory.  This zeros the profile structure.
    memset(pwextBuf, 0,sizeof(hdd_wext_state_t));
   
    // Set up the pointer to the Wireless Extensions state structure
    pAdapter->pWextState = pwextBuf;


    pAdapter->dev->wireless_handlers = (struct iw_handler_def *)&we_ftm_handler_def;

    return;
}

#endif //ANI_MANF_DIAG

VOS_STATUS WLANFTM_McProcessMsg (v_VOID_t *message)
{
#ifdef ANI_MANF_DIAG
    ftm_rsp_msg_t   *pFtmMsgRsp;

    VOS_STATUS vos_status = VOS_STATUS_SUCCESS;
    hdd_adapter_t *pAdapter;
    v_CONTEXT_t pVosContext= NULL;

    ENTER();

    pFtmMsgRsp = (ftm_rsp_msg_t *)message;

    if (!message )
    {
        VOS_TRACE( VOS_MODULE_ID_SYS, VOS_TRACE_LEVEL_ERROR,
                "WLAN FTM:Invalid parameter sent on WLANFTM_ProcessMainMessage");
        return VOS_STATUS_E_INVAL;
    }
    /*Get the global context */
    pVosContext = vos_get_global_context(VOS_MODULE_ID_SYS, NULL);

     /*Get the Hdd Context */
    pAdapter = ((VosContextType*)(pVosContext))->pHDDContext;

    if (pAdapter->ftm.cmd_iwpriv == TRUE) {
		
        complete(&pAdapter->ftm.ftm_comp_var);
    }
    else {
    /*Response length to Ptt App*/
    pAdapter->ftm.wnl->wmsg.length = sizeof(tAniHdr)+ SIZE_OF_FTM_DIAG_HEADER_LEN + pFtmMsgRsp->msgBodyLength;

     /*Ptt App expects the response length in LE */
    pAdapter->ftm.wnl->wmsg.length = FTM_SWAP16(pAdapter->ftm.wnl->wmsg.length);

    /*Response expects the length to be in */
    pAdapter->ftm.pResponseBuf->ftm_hdr.data_len = pAdapter->ftm.pRequestBuf->ftm_hdr.data_len - 1;

    /*Copy the message*/
    memcpy((char*)&pAdapter->ftm.pResponseBuf->ftmpkt,(char*)message,pFtmMsgRsp->msgBodyLength);

    /*Update the error code*/
    pAdapter->ftm.pResponseBuf->ftm_err_code = WLAN_FTM_SUCCESS;

    vos_status = vos_event_set(&pAdapter->ftm.ftm_vos_event);

    if (!VOS_IS_STATUS_SUCCESS(vos_status))
    {
       VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR, ("ERROR: HDD vos_event_set failed!!\n"));
       return VOS_STATUS_E_FAILURE;
    }
    }
    EXIT();
#endif
    return VOS_STATUS_SUCCESS;

}


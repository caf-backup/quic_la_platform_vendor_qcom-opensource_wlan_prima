/*
 * Copyright (c) 2011-2012 Qualcomm Atheros, Inc. 
 * All Rights Reserved. 
 * Qualcomm Atheros Confidential and Proprietary. 
 *
 * Airgo Networks, Inc proprietary. All rights reserved.
 * This file halHddApi.h contains the prototypes for HAL Apis
 * that are used by Hdd
 *
 * Author:      Viji Alagarsamy
 * Date:        07/07/2005
 * History:-
 * Date         Modified by    Modification Information
 * --------------------------------------------------------------------
 */

#ifndef _HALHDDAPI_H_
#define _HALHDDAPI_H_

#if (WNI_POLARIS_FW_OS == SIR_WINDOWS)
#include <ndis.h>
#endif

#include "sirParams.h"


#if (WNI_POLARIS_FW_OS == SIR_WINDOWS)

#define HAL_HDD_RX_MB_DATA_READY(pAdapter) (((hwReadRegister((pAdapter), TIT_BMU_RXMB_STATUS_REG) & \
                                              TIT_BMU_RXMB_STATUS_RX_MB_DONE_MASK) == \
                                              TIT_BMU_RXMB_STATUS_RX_MB_DONE_EBUSY) ? TRUE:FALSE)

#define HAL_HDD_TX_MB_READY(pAdapter) (((hwReadRegister((pAdapter), TIT_BMU_TXMB_STATUS_REG) & \
                                         TIT_BMU_TXMB_STATUS_TX_MB_DONE_MASK) == \
                                         TIT_BMU_TXMB_STATUS_TX_MB_DONE_EEMPTY) ? TRUE:FALSE)

#define HAL_HDD_DISABLE_RX(pAdapter) (halSetReceiverWinEnable(pAdapter->pMacGlobals, 0))

#define HAL_HDD_ENABLE_RX(pAdapter)  (halSetReceiverWinEnable(pAdapter->pMacGlobals, 1))

extern void halSetReceiverWinEnable(void *pPtr, tANI_U8 enable);

extern void macSysResetReqFromHDD(void *pMac, tANI_U32 rc);


extern void halPerformSystemReset(tHalHandle hMac);

#endif // (WNI_POLARIS_FW_OS == SIR_WINDOWS)


// Board capabilities fields
extern tANI_U8 halIsIntAntennaPopulated(void *pMacGlobal);
extern tANI_U8 halIsSdramPopulated(void *pMacGlobal);
extern tANI_U8 halIsExtPAPopulated(void *pMacGlobal);
extern tANI_U8 halGetNumTransmittersPopulated(void *pMacGlobal);
extern tANI_U8 halGetNumReceiversPopulated(void *pMacGlobal);
extern tANI_U8 halIsRadioSwitchOn(void *pMacGlobal);
extern tANI_U8 halBBchipVersion(void *pMacGlobal);
extern tANI_U8 halIsBcapBgOnly(void *pMacGlobal);
extern tANI_U8 halIsCardMpciFamily(void *pMacGlobal);
extern tANI_U8 halIsCardCbFamily(void *pMacGlobal);
extern tANI_U8 halIsCardApFamily(void *pMacGlobal);
extern void halEnableAdaptiveThresholdAlgo(void *pvMac, tANI_U8 fEnable);
extern void halSetAdaptiveThresholdAlgo(void *pvMac, tANI_U32 Algo);
void halInitAdaptiveThreshold(void *pvMac, tANI_U32 algorithm);

// TxMB Functions
extern tSirRetStatus halMmhForwardMBmsg(void* pSirGlobal, tSirMbMsg* pMb);

/// TM Ring cleanup routine
extern void halTxRingCleanup(void*);

//getting stats
extern tANI_U8 halGetRxTxStats(void *pvMac, void *pvStats);

#endif // _HALHDDAPI_H_

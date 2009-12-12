/*
 * Airgo Networks, Inc proprietary. All rights reserved.
 * This file halMacSecurityApi.h contains the Encryption APIs
 * Author:      Viji Alagarsamy
 * Date:        03/01/2005
 * History:-
 * Date         Modified by    Modification Information
 * --------------------------------------------------------------------
 */

#ifndef __HAL_MAC_SECURITY_API_H
#define __HAL_MAC_SECURITY_API_H

#include "aniGlobal.h"
//#include "halMacDef.h"


// Security Related definitions
#define HAL_MAX_NUM_WEP_KEYID 4 
#define HAL_MAX_NUM_AES_KEYID 1
#define HAL_INVALID_KEYID_INDEX 0xff

#define HAL_WEP40_LENGTH        5
#define HAL_WEP104_LENGTH       13
#define HAL_AES_LENGTH          16
#define HAL_TKIP_KEY_LENGTH     16
#define HAL_TKIP_MICKEY_LENGTH  16
#define HAL_TKIP_MICKEY_SIZE    8

#define HAL_DPU_DEFAULT_RCE_ON  0xFF
#define HAL_DPU_DEFAULT_RCE_OFF 0x00
#define HAL_DPU_DEFAULT_WCE_ON  0xFF
#define HAL_DPU_DEFAULT_WCE_OFF 0x00

#define HAL_DPU_DEFAULT_TX_RC_LOW   0
#define HAL_DPU_DEFAULT_TX_RC_HIGH  0

eHalStatus halGetWepKeysFromCfg(tpAniSirGlobal pMac, tANI_U8 *pDefaultKeyId, tANI_U8 *pNumKeys, tSirKeys *pSirKeys);

eHalStatus halSetBssWepKey(tpAniSirGlobal pMac, tANI_U8 bssIdx, tAniEdType encType, tANI_U8 keyId, tANI_U8 *pKey);

eHalStatus halSetStaWepKey(tpAniSirGlobal pMac, tANI_U8 staIdx, tAniEdType encType, tANI_U8 wepIdx);

// Set Key in the station DPU descriptor
eHalStatus halSetPerStaKey(tpAniSirGlobal pMac, tANI_U8 dpuIndex, tANI_U8 staId, tAniEdType encType, tANI_U16 bRCE, 
        tANI_U16 bWCE, tANI_U8 *winChkSize, tANI_U8 singleTidRc, tANI_U8 *pKey, tANI_U8 paeRole, tANI_U8 keyId);

eHalStatus halInvalidateBssWepKey(tpAniSirGlobal pMac, tANI_U8 bssIdx, tANI_U8 encType, tANI_U8 keyId);
eHalStatus halInvalidateStaKey(tpAniSirGlobal pMac,tANI_U8 keyId,tANI_U8 encMode);

#endif // __HAL_MAC_SECURITY_API_H


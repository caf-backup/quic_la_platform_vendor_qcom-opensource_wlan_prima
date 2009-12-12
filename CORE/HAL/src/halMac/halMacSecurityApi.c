/*
 * Airgo Networks, Inc proprietary. All rights reserved.
 * halMacSecurityApi.c: Provides all the MAC Encryption APIs in this file.
 * Author:    Viji Alagarsamy
 * Date:      03/07/2005
 * History:-
 * Date        Modified by            Modification Information
 *
 * --------------------------------------------------------------------------
 *
 */

/* Application Specific include files */
#include "halTypes.h"
#include "halDPU.h"
#include "halMacSecurityApi.h"

#include "sirApi.h"
#include "sirParams.h"
#include "halDebug.h"
#include "cfgApi.h"

// ---------------------------------------------------------------------------
/**
 * halGetWepKeysFromCfg
 *
 * FUNCTION:
 * Extract the WEP KEY(s) from the CFG. Recall that the
 * WEP Keys were handed down to us by the SME (WSM or WinHDD)
 * during WNI_CFG_SET_REQ, as MacConfig!!
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 * WEP Keys were handed down to MAC S/W by the SME
 *
 * NOTE:
 *
 * @param pMac an instance of MAC parameters
 * @param numKeys - num of keys(should always <=4)
 * @param pKey -   Pointer to the actual key
 * @return Success or Failure (Invalid Key length, Invalid Key ID)
 */
eHalStatus halGetWepKeysFromCfg( tpAniSirGlobal  pMac,
    tANI_U8 *pDefaultKeyId,
    tANI_U8 *pNumKeys,
    tSirKeys *pSirKeys )
{
tANI_U32 i, j, defKeyId = 0;
tANI_U32 val = SIR_MAC_KEY_LENGTH;
eHalStatus status = eHAL_STATUS_SUCCESS;

  if( eSIR_SUCCESS != wlan_cfgGetInt( pMac,
        WNI_CFG_WEP_DEFAULT_KEYID,
        &defKeyId ))
        HALLOGE(halLog( pMac, LOGE,
            FL("Unable to retrieve defaultKeyId from CFG. Defaulting to 0...\n")));

  *pDefaultKeyId = (tANI_U8)defKeyId;

  // Need to extract ALL of the configured WEP Keys
  for( i = 0, j = 0; i < SIR_MAC_MAX_NUM_OF_DEFAULT_KEYS; i++ )
  {
  	val = SIR_MAC_KEY_LENGTH;
    if( eSIR_SUCCESS != wlan_cfgGetStr( pMac,
          (tANI_U16) (WNI_CFG_WEP_DEFAULT_KEY_1 + i),
          pSirKeys[j].key,
          &val )){
      HALLOGW( halLog( pMac, LOGW, 
          FL("WEP Key index [%d] may not configured in CFG\n"),
          i ));
    }else
    {
      pSirKeys[j].keyId = (tANI_U8) i;
      // Actually, a DC (Don't Care) because
      // this is determined (and set) by PE/MLME
      pSirKeys[j].unicast = 0;
      // Another DC (Don't Care)
      pSirKeys[j].keyDirection = eSIR_TX_RX;
      // Another DC (Don't Care). Unused for WEP
      pSirKeys[j].paeRole = 0;
      // Determined from wlan_cfgGetStr() above...
      pSirKeys[j].keyLength = (tANI_U16) val;

      j++;
      *pNumKeys = (tANI_U8) j;
    }
  }

  return status;
}

// ---------------------------------------------------------------------------
/**
 * halSetBssWepKey
 *
 * FUNCTION:
 *    Sets WEP-40 and WEP-104 keys
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param pMac an instance of MAC parameters
 * @param bssIdx -  BSS IDX that the keys belongs to.
 * @param numKeys - num of keys(should always <=4)
 * @param pKey -   Pointer to the actual key
 * @return Success or Failure (Invalid Key length, Invalid Key ID)
 */
eHalStatus
halSetBssWepKey(
    tpAniSirGlobal  pMac,
    tANI_U8         bssIdx,
    tAniEdType      encType,
    tANI_U8         keyId,
    tANI_U8         *pKey)
{
    tpBssStruct bssTable = (tpBssStruct) pMac->hal.halMac.bssTable;
    tANI_U8     dpuKeyId;
    eHalStatus  status;

    if(keyId >= HAL_MAX_NUM_WEP_KEYID)
        return eHAL_STATUS_INVALID_PARAMETER;

    dpuKeyId = bssTable[bssIdx].wepKeyIds[keyId];
    /* We need to release the old keys from DPU Key table */
    if(dpuKeyId != HAL_INVALID_KEYID_INDEX)
        halDpu_ReleaseKeyId(pMac, dpuKeyId);

    bssTable[bssIdx].encryptMode = encType;

    if ( (encType != eSIR_ED_WEP40) && (encType != eSIR_ED_WEP104) )
    {
        bssTable[bssIdx].wepKeyIds[keyId] = HAL_INVALID_KEYID_INDEX;
        return eHAL_STATUS_SUCCESS;
    }

    status = halDpu_AllocKeyId(pMac, & dpuKeyId);
    if(status != eHAL_STATUS_SUCCESS)
    {
        bssTable[bssIdx].wepKeyIds[keyId] = HAL_INVALID_KEYID_INDEX;
        return status;
    }
    else
        bssTable[bssIdx].wepKeyIds[keyId] = dpuKeyId;

    status = halDpu_SetKeyDescriptor(pMac, dpuKeyId, encType, pKey);
    if(status != eHAL_STATUS_SUCCESS)
    {
        /* since it fails, we need to recover */
        halDpu_ReleaseKeyId(pMac,dpuKeyId);
        bssTable[bssIdx].wepKeyIds[keyId] = HAL_INVALID_KEYID_INDEX;
    }

    return status;
} // halSetWepKey


// ---------------------------------------------------------------------------
/**
 * halInvalidateBssWepKey
 *
 * FUNCTION:
 *    Invalidate WEP-40 and WEP-104 keys
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param pMac an instance of MAC parameters
 * @param bssIdx -  BSS IDX that the keys belongs to.
 * @param keyId - keyId to be invalidated.
 * @return Success or Failure (Invalid Key length, Invalid Key ID)
 */
eHalStatus
halInvalidateBssWepKey(
    tpAniSirGlobal  pMac,
    tANI_U8         bssIdx,
    tANI_U8         encType,
    tANI_U8         keyId)
{
    tpBssStruct bssTable = (tpBssStruct) pMac->hal.halMac.bssTable;
    tANI_U8     dpuKeyId;
    eHalStatus  status;
    tANI_U8 pKey[SIR_MAC_MAX_KEY_LENGTH];
    

    if(keyId >= HAL_MAX_NUM_WEP_KEYID)
        return eHAL_STATUS_INVALID_PARAMETER;

    if ( (encType != eSIR_ED_WEP40) && (encType != eSIR_ED_WEP104) )
    {
          return eHAL_STATUS_SUCCESS;
    }

    dpuKeyId = bssTable[bssIdx].wepKeyIds[keyId];

    palZeroMemory(pMac->hHdd, pKey, SIR_MAC_MAX_KEY_LENGTH); //Invalidate wep key to zero by writing all zeroes into key.
    status = halDpu_SetKeyDescriptor(pMac, dpuKeyId, (tAniEdType)encType, pKey);
    if(status != (eHalStatus)eHAL_STATUS_SUCCESS)
    {
        HALLOGE( halLog(pMac, LOGE, FL("Unable to invalidate buffer\n")));
    }

    return status;
} // halInvalidateBssWepKey

// ---------------------------------------------------------------------------
/**
 * halInvalidateStaKey
 *
 * FUNCTION:
 *    Invalidate station keys
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param pMac an instance of MAC parameters
 * @param keyId - keyId to be invalidated.
 * @param encMode - Encryption mode of the key invalidated.
 * @return Success or Failure (Invalid Key length, Invalid Key ID)
 */

eHalStatus
halInvalidateStaKey(
                        tpAniSirGlobal pMac,
                        tANI_U8 keyId,
                        tANI_U8 encMode)
{
    eHalStatus  status;
    tANI_U8 pKey[SIR_MAC_MAX_KEY_LENGTH];
    

    palZeroMemory(pMac->hHdd, pKey, SIR_MAC_MAX_KEY_LENGTH); //Invalidate key by writing all zeroes into key.
    status = halDpu_SetKeyDescriptor(pMac, keyId, (tAniEdType)encMode, pKey);
    if(status != (eHalStatus)eHAL_STATUS_SUCCESS)
    {
        HALLOGE( halLog(pMac, LOGE, FL("Unable to invalidate buffer\n")));
    }

    return status;
}

// ---------------------------------------------------------------------------
/**
 * halSetStaWepKey
 *
 * FUNCTION:
 *    Copys the static WEP keys from BSS and
 *    set the default transmit WEP key for a station
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param pMac an instance of MAC parameters
 * @param staIdx -
 * @param defWepIdx -
 * @return Success or Failure (Invalid station ID or WEP key Idx )
 */
eHalStatus
halSetStaWepKey(
    tpAniSirGlobal  pMac,
    tANI_U8         staIdx,
    tAniEdType      encType,
    tANI_U8         wepIdx)
{
    tpBssStruct bssTable    = (tpBssStruct) pMac->hal.halMac.bssTable;
    eHalStatus  status      = eHAL_STATUS_SUCCESS;
    tANI_U8     bssIdx, dpuIdx, dpuKeyIdx;

    if (wepIdx >= HAL_MAX_NUM_WEP_KEYID)
        return eHAL_STATUS_INVALID_KEYID;

    /* Get the bssIdx */
    status = halTable_GetBssIndexForSta(pMac, & bssIdx, staIdx);

    if (status != (eHalStatus)eSIR_SUCCESS)
        return status;

    /* Find the keyIdx for default wep key */
    dpuKeyIdx = bssTable[bssIdx].wepKeyIds[wepIdx];

    if (dpuKeyIdx == HAL_INVALID_KEYID_INDEX)
        return eHAL_STATUS_INVALID_KEYID;

    /* Get the dpuIdx */
    status = halTable_GetStaDpuIdx(pMac, staIdx, & dpuIdx );

    if (status != (eHalStatus)eSIR_SUCCESS)
        return status;

    //
    // Now, update this STA's DPU Descriptor wrt
    // a) TX WEP Key index
    // b) RX WEP Keys, which is inherited from the BSS
    // Basically, ensuring that the STA is referring to
    // the same WEP keys across the entire BSS
    //
    return halDpu_SetWepKeys(pMac, dpuIdx, encType, wepIdx,
                                  bssTable[bssIdx].wepKeyIds[0],
                                  bssTable[bssIdx].wepKeyIds[1],
                                  bssTable[bssIdx].wepKeyIds[2],
                                  bssTable[bssIdx].wepKeyIds[3] );

}


/*
 * halDpu_GenerateDerivedKeys
 *
 * FUNCTION:
 *  This function derives the K1 keys from the K keys
 *
 *
 * @param: pMac - an instance of MAC parameters
 * @param: pKey - pointer to the 'K' key
 * @param: pDerivedKey - pointer to the buffer in which the derived keys will be stored
 *
 * @return: Success or failure.
 */
eHalStatus 
halDpu_GenerateDerivedKeys(tpAniSirGlobal pMac, tANI_U8 *pKey, tANI_U8 *pDerivedKey)
{
    // TODO: The derived key should be generated here from the pKey
    // The algorithm for deriving the K1 keys for IGTK should be placed here

    // NOTE: for now simply copying key into derived key
    palCopyMemory(pMac->hHdd, pDerivedKey, pKey, SIR_MAC_MAX_KEY_LENGTH);

    return eHAL_STATUS_SUCCESS;
}


// ---------------------------------------------------------------------------
/**
 * halSetPerStaKey
 *
 * FUNCTION:
 *  This interface is used to write the key to the specified location.
 *  sessionKeyId indicates either TX or RX
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *   This interface shall be used to write to either TX or RX key in STA
 *   descriptor.
 *
 * @param pMac an instance of MAC parameters
 * @param dpuIndex - DPU Descriptor index
 * @param staId - Station ID
 * @param pKey - A pointer to the key
 * @param singleTidRc - Control for Replay Count, 1= Single TID based replay count on Tx
 *        0 = Per TID based replay count on TX       
 * @return Success or failure (Invalid key length)
 */
eHalStatus
halSetPerStaKey(
    tpAniSirGlobal  pMac,
    tANI_U8         dpuIndex,
    tANI_U8         staId,
    tAniEdType      encType,
    tANI_U16        bRCE,
    tANI_U16        bWCE,
    tANI_U8         *winChkSize,
    tANI_U8         singleTidRc,
    tANI_U8         *pKey,
    tANI_U8         paeRole,
    tANI_U8         defKeyId )
{
    eHalStatus status ;
    tANI_U8 dpuIdx , keyIdx , derivedKeyIdx , micKeyIdx , rcIdx, newDpuIdx ;
    tANI_U8 tmpIdx;
    tANI_U8 derivedKey[SIR_MAC_MAX_KEY_LENGTH];

    newDpuIdx = keyIdx = derivedKeyIdx = micKeyIdx = rcIdx = HAL_INVALID_KEYID_INDEX;

    //
    // If a DPU Descriptor Index is directly provided,
    // then use it. This is the case when the SME is
    // trying to configure the GTK (Group Transient Key)
    //
    // For the PTK (Pairwise Transient Key), we need
    // to extract the DPU Descriptor index wrt the given
    // STA index
    //
    if( HAL_INVALID_KEYID_INDEX != dpuIndex )
      dpuIdx = dpuIndex;
    else
    {
      if( eHAL_STATUS_SUCCESS !=
          (status = halTable_GetStaDpuIdx( pMac,
                                           staId,
                                           &dpuIdx )))
        return status;
    }

    if(dpuIdx == HAL_INVALID_KEYID_INDEX)
    {
        status = halDpu_AllocId(pMac, & dpuIdx);  //FIXME: why should index be allocated here?
        newDpuIdx = dpuIdx;                                                 //Moreover, This index is not going anywhere. 
        if(status != eHAL_STATUS_SUCCESS)
            goto failed ;
    }

    /* Release any existing key descriptors, reallocate only for encryption policy 
       being set to security other than none */
    
    status = halDpu_GetRCId(pMac, dpuIdx, &tmpIdx);
    if(eHAL_STATUS_SUCCESS == status)
        halDpu_ReleaseRCId(pMac, tmpIdx);
    
    status = halDpu_GetKeyId(pMac, dpuIdx, &tmpIdx);
    if(eHAL_STATUS_SUCCESS == status)
        halDpu_ReleaseKeyId(pMac, tmpIdx);
    
    status = halDpu_GetMicKeyId(pMac, dpuIdx, &tmpIdx);
    if(eHAL_STATUS_SUCCESS == status)
        halDpu_ReleaseMicKeyId(pMac, tmpIdx);


    if(eSIR_ED_NONE != encType)
    {
        /* Alloc a new key descriptor for this sta key */
        status = halDpu_AllocKeyId(pMac, & keyIdx);
        if(status != eHAL_STATUS_SUCCESS)
            goto failed;

        status = halDpu_SetKeyDescriptor(pMac, keyIdx, encType, pKey);
        if(status != eHAL_STATUS_SUCCESS)
            goto failed;
    }

    if(eSIR_ED_AES_128_CMAC == encType)
    {
        /* Alloc a new key descriptor for the derived keys used for BIP */
        status = halDpu_AllocKeyId(pMac, & derivedKeyIdx);
        if(status != eHAL_STATUS_SUCCESS)
            goto failed;

        status = halDpu_GenerateDerivedKeys(pMac, pKey, derivedKey);
        if(status != eHAL_STATUS_SUCCESS)
            goto failed;

        status = halDpu_SetKeyDescriptor(pMac, derivedKeyIdx, encType, derivedKey);
        if(status != eHAL_STATUS_SUCCESS)
            goto failed;
    }


    if(encType == eSIR_ED_TKIP)
    {
        /* Alloc a new Mic Key dexriptor */
        status = halDpu_AllocMicKeyId(pMac, & micKeyIdx, keyIdx);

        if(status != eHAL_STATUS_SUCCESS)
            goto failed;

        status = halDpu_SetMicKeyDescriptor(pMac, micKeyIdx, pKey, paeRole );
        if(status != eHAL_STATUS_SUCCESS)
            goto failed;

    }

    if(eSIR_ED_NONE != encType)
    {
        //
        // Determine if the RC Descriptor is already
        // allocated for this DPU descriptor index.
        // If no, only then allocate new RC's
        // Else, use the existing RC index
        //
        // An RC Descriptor could have been setup for
        // this DPU descriptor, when CONCATENATION is
        // turned ON during a prior ADD_STA
        //
        if( eHAL_STATUS_FAILURE == halDpu_GetRCId( pMac,
              dpuIdx,
              &rcIdx ))
        {
          status = halDpu_AllocRCId( pMac, &rcIdx );
          if( eHAL_STATUS_SUCCESS != status )
            goto failed;
        }

        status = halDpu_SetRCDescriptor( pMac, rcIdx, bRCE, bWCE, winChkSize );
        if(status != eHAL_STATUS_SUCCESS)
            goto failed;
    }

    HALLOGW( halLog( pMac, LOGW,
        FL("Trying to set DPU descriptor for STA Id %d: \n"
        "DPU - %d, ENC Type - %d, PAE Role - %d, \n"
        "KEY Id - %d, MICKEY Id - %d, RC Id - %d, \n"
        "RCE - %d, WCE - %d\n"),
        staId,
        dpuIdx, encType, paeRole,
        keyIdx, micKeyIdx, rcIdx,
        bRCE, bWCE ));

    status = halDpu_SetDescriptorAttributes(pMac, dpuIdx, encType,
            keyIdx, derivedKeyIdx, micKeyIdx, rcIdx, singleTidRc, defKeyId );
    if(status == eHAL_STATUS_SUCCESS)
        return status;


failed:

    if(newDpuIdx != HAL_INVALID_KEYID_INDEX )
        halDpu_ReleaseId(pMac, keyIdx);

    if(keyIdx != HAL_INVALID_KEYID_INDEX )
        halDpu_ReleaseKeyId(pMac, keyIdx);

    if(micKeyIdx != HAL_INVALID_KEYID_INDEX )
        halDpu_ReleaseMicKeyId(pMac, micKeyIdx);

    if(rcIdx != HAL_INVALID_KEYID_INDEX )
        halDpu_ReleaseRCId(pMac, rcIdx);

    return status;
}


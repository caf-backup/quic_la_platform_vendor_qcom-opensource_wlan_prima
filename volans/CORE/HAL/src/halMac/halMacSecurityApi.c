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

#include "halFw.h"

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

#if defined(FEATURE_WLAN_WAPI) && !defined(LIBRA_WAPI_SUPPORT)
/*
 * halDPU_SetWAPISTAxKeyIndexes
 *
 * FUNCTION:
 *  This function programs WAPI_STAX_key_indexes register
 *
 *
 * @param: wapiStaID - Gives the index of the register to be programmed (0-7)
 * @param: defKeyId - Key ID given by the supplicant
 * @param: fGTK - Indicates if the defKeyId is GTK/PTK
 *
 * @return: Success or failure.
 */
eHalStatus
halDPU_SetWAPISTAxKeyIndexes(tpAniSirGlobal  pMac, tANI_U8 wapiStaID, tANI_U8 defKeyId, tANI_BOOLEAN fGTK)
{
  tANI_U32   value = 0;
  tANI_U32   regAddress = QWLAN_DPU_WAPI_STA0_KEY_INDEXES_REG;
  tANI_U8   offset;

  if(wapiStaID < WAPI_STA_LOW_INDEX || wapiStaID > WAPI_STA_HIGH_INDEX)
  {
    HALLOGE( halLog(pMac, LOGE, FL("Invalid wapiStaID:%d\n"), wapiStaID));
    return eHAL_STATUS_FAILURE;
  }

  regAddress += wapiStaID * 4;

  /* Read QWLAN_DPU_WAPI_STAx_KEY_INDEXES_REG followed by read of QWLAN_DPU_WAPI_STA_KEY_INDEX_VALUES_REG
    * Value of QWLAN_DPU_WAPI_STAx_KEY_INDEXES_REG after read will be reflected in QWLAN_DPU_WAPI_STA_KEY_INDEX_VALUES_REG
    */
  halReadRegister(pMac, regAddress, &value);
  halReadRegister(pMac, QWLAN_DPU_WAPI_STA_KEY_INDEX_VALUES_REG, &value);

  offset = (fGTK == 0) ? QWLAN_DPU_WAPI_STA0_KEY_INDEXES_KEY_INDEX1_OFFSET: QWLAN_DPU_WAPI_STA0_KEY_INDEXES_KEY_INDEX0_OFFSET;

  if(fGTK == 0)
  {
    value = value & (~QWLAN_DPU_WAPI_STA0_KEY_INDEXES_KEY_INDEX1_MASK);
  }
  else
  {
    //This is a hack for always set to 0 because it works. Need to double check with Rama about this
    defKeyId = 0;
    value = value & (~QWLAN_DPU_WAPI_STA0_KEY_INDEXES_KEY_INDEX0_MASK);
  }

  value |= (defKeyId << offset);

  halWriteRegister(pMac, regAddress, value);

  return eHAL_STATUS_SUCCESS;
}
#endif

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
    tANI_U8         defKeyId,
    tANI_U8         *keyRsc)
{
    eHalStatus status ;
    tANI_U8 dpuIdx , keyIdx , derivedKeyIdx , micKeyIdx , rcIdx, newDpuIdx ;
    tANI_U8 tmpIdx;
    tANI_U8 derivedKey[SIR_MAC_MAX_KEY_LENGTH];
    tANI_BOOLEAN fGTK = eANI_BOOLEAN_FALSE;
#if defined(FEATURE_WLAN_WAPI) && !defined(LIBRA_WAPI_SUPPORT)
    tANI_U8     wapiStaID = 0;
#endif
    tpDpuInfo pDpu = (tpDpuInfo) pMac->hal.halMac.dpuInfo;

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
    {
      //Base on the assumption that caller only sets the index for GTK
        //This is the case as of now.(10/21/2009)
      fGTK = eANI_BOOLEAN_TRUE;
      dpuIdx = dpuIndex;
    }
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
    {
        halDpu_ReleaseRCId(pMac, dpuIdx, tmpIdx);
        pDpu->descTable[dpuIdx].rcIdx = HAL_INVALID_KEYID_INDEX;
    }
    
    status = halDpu_GetKeyId(pMac, dpuIdx, &tmpIdx);
    if(eHAL_STATUS_SUCCESS == status)
    {
#if defined(FEATURE_WLAN_WAPI)
        //To avoid the new packet number being overwritten by decoding packet using the old
        //GTK/PTK (and old PN) during rekeying. Invalidate the old key so we won't receive 
        //packet encrypted by old key
        if(eSIR_ED_WPI == encType)
        {
           halInvalidateStaKey(pMac, tmpIdx, eSIR_ED_WPI); 
        }
#endif
        halDpu_ReleaseKeyId(pMac, tmpIdx);
        pDpu->descTable[dpuIdx].keyIdx = HAL_INVALID_KEYID_INDEX;
    }
    
    status = halDpu_GetMicKeyId(pMac, dpuIdx, &tmpIdx);
    if(eHAL_STATUS_SUCCESS == status)
    {
        halDpu_ReleaseMicKeyId(pMac, tmpIdx);
        pDpu->descTable[dpuIdx].micKeyIdx = HAL_INVALID_KEYID_INDEX;
    }

    if(eSIR_ED_NONE != encType)
    {
        /* Alloc a new key descriptor for this sta key */
        status = halDpu_AllocKeyId(pMac, & keyIdx);
        if(status != eHAL_STATUS_SUCCESS)
            goto failed;

#if defined(FEATURE_WLAN_WAPI)
        if( eSIR_ED_WPI == encType )
        {
            //For volans 1.0, WAPI key id in DPU desc must match the physical index in key desc
            //This code just to provide a warning in case WAPI is not working due to this reason
            //This portion can be removed when volans 2.0 is available.
            if( (keyIdx != defKeyId) && !fGTK )
            {
                HALLOGE(halLog(pMac, LOGE, "Volans 1.0 WAPI keyid(%d) must match keyIdx(%d)\n", defKeyId, keyIdx);)
            }
        }
#endif //WAPI
        if((eSIR_ED_WEP40 != encType) && (eSIR_ED_WEP104 != encType))
        {
            HALLOGE(halLog(pMac, LOGE, "  HAL Set keyIdx (%d) encType(%d) key = %02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X\n",
                keyIdx, encType, pKey[0], pKey[1], pKey[2], pKey[3],pKey[4], pKey[5],
                pKey[6], pKey[7], pKey[8],
                pKey[9], pKey[10], pKey[11], pKey[12], pKey[13], pKey[14], pKey[15]));
        }
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

#if defined(FEATURE_WLAN_WAPI)
    if( eSIR_ED_WPI == encType )
    {
        /* Alloc a new Mic Key dexriptor */
        status = halDpu_AllocMicKeyId(pMac, & micKeyIdx, keyIdx);

        if(status != eHAL_STATUS_SUCCESS)
        {
            HALLOGE(halLog(pMac, LOGE, " failed to allocate WPI MIC key descriptor\n"));
            goto failed;
        }
        HALLOGE(halLog(pMac, LOGE, "  HAL Set WPI MickeyIdx (%d) encType(%d) key = %02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X\n",
                micKeyIdx, encType, pKey[0+16], pKey[1+16], pKey[2+16], pKey[3+16], pKey[4+16], pKey[5+16], 
                pKey[6+16], pKey[7+16], pKey[8+16],
                pKey[9+16], pKey[10+16], pKey[11+16], pKey[12+16], pKey[13+16], pKey[14+16], pKey[15+16]));
        status = halDpu_SetWPIMicKeyDescriptor(pMac, micKeyIdx, &pKey[HAL_WPI_KEY_LENGTH], paeRole );
        if(status != eHAL_STATUS_SUCCESS)
            goto failed;
    }

#endif // defined(FEATURE_WLAN_WAPI)

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
          status = halDpu_AllocRCId( pMac, encType, &rcIdx );
          if( eHAL_STATUS_SUCCESS != status )
            goto failed;
        }

#if defined(FEATURE_WLAN_WAPI)
        if(eSIR_ED_WPI == encType)
        {
#ifdef LIBRA_WAPI_SUPPORT
          tANI_U8 txPN[WLAN_WAPI_KEY_RSC_LEN] = {0x5C, 0x36, 0x5C, 0x36,
                                                 0x5C, 0x36, 0x5C, 0x36,
                                                 0x5C, 0x36, 0x5C, 0x36,
                                                 0x5C, 0x36, 0x5C, 0x36};
          tANI_U8 rxPN[WLAN_WAPI_KEY_RSC_LEN] = {0x5C, 0x36, 0x5C, 0x36,
                                                 0x5C, 0x36, 0x5C, 0x36,
                                                 0x5C, 0x36, 0x5C, 0x36,
                                                 0x5C, 0x36, 0x5C, 0x37};
#else
          tANI_U8 txPN[WLAN_WAPI_KEY_RSC_LEN] = {0x36, 0x5C, 0x36, 0x5C,
                                                 0x36, 0x5C, 0x36, 0x5C,
                                                 0x36, 0x5C, 0x36, 0x5C,
                                                 0x36, 0x5C, 0x36, 0x5C};
          tANI_U8 rxPN[WLAN_WAPI_KEY_RSC_LEN] = {0x37, 0x5C, 0x36, 0x5C,
                                                 0x36, 0x5C, 0x36, 0x5C,
                                                 0x36, 0x5C, 0x36, 0x5C,
                                                 0x36, 0x5C, 0x36, 0x5C};
#endif

            //For supplicant, the packet number(PN) starts at 0x5C365C365C365C365C365C365C365C36
            //For authenticator, the packet number(PN) starts at 0x5C365C365C365C365C365C365C365C37
            //NOTE: Assuming PN is reset after rekey
            if( fGTK )
            {
                //For multicast, PN starts at 0x36 as the lowest byte
                rxPN[WLAN_WAPI_KEY_RSC_LEN - 1] = 0x36;
            }
            else if( 0 != paeRole )
            {
                //we are the authenticator
                txPN[WLAN_WAPI_KEY_RSC_LEN - 1] = 0x37;
                rxPN[WLAN_WAPI_KEY_RSC_LEN - 1] = 0x36;
            }
            if( NULL != keyRsc )
            {
                palCopyMemory(pMac->hHdd, rxPN, keyRsc, WLAN_WAPI_KEY_RSC_LEN);
            }

            HALLOGE(halLog(pMac, LOGE, "  HAL Set WAPI RCIdx (%d) encType(%d)" 
                "TX = %02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X"
                "RX = %02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X\n",
                rcIdx, encType, txPN[0], txPN[1], txPN[2], txPN[3], txPN[4], txPN[5], txPN[6],
                txPN[7], txPN[8],
                txPN[9], txPN[10], txPN[11], txPN[12], txPN[13], txPN[14], txPN[15],
                rxPN[0], rxPN[1], rxPN[2], rxPN[3], rxPN[4], rxPN[5], rxPN[6],
                rxPN[7], rxPN[8],
                rxPN[9], rxPN[10], rxPN[11], rxPN[12], rxPN[13], rxPN[14], rxPN[15]));
            status = halDpu_SetWAPIRCDescriptor( pMac, rcIdx, txPN, rxPN );
        }
        else
#endif
        {
            status = halDpu_SetRCDescriptor( pMac, rcIdx, bRCE, bWCE, winChkSize );
        }
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

    /* DPU HW treats encryption mode 4 plus RMF bit set as BIP thus for BIP
       eSIR_ED_AES_128_CMAC should be set as eSIR_ED_CCMP in DPU Desc*/
    if(eSIR_ED_AES_128_CMAC == encType)
    {
       encType = eSIR_ED_CCMP;
    }

#if defined(FEATURE_WLAN_WAPI) && !defined(LIBRA_WAPI_SUPPORT)
    if(eSIR_ED_WPI == encType)
    {
      /* Programming wapiStaID changes for WAPI in case of IBSS*/
      /* Currently WAPI is supported only in Infrastructure mode */
      wapiStaID = 0;
      status = halDPU_SetWAPISTAxKeyIndexes(pMac, wapiStaID, defKeyId, fGTK);
      if(status != eHAL_STATUS_SUCCESS)
        goto failed;
    }
#endif

    status = halDpu_SetDescriptorAttributes(pMac, dpuIdx, encType,
            keyIdx, derivedKeyIdx, micKeyIdx, rcIdx, singleTidRc, defKeyId
#if defined(FEATURE_WLAN_WAPI) && !defined(LIBRA_WAPI_SUPPORT)
            , wapiStaID
            , fGTK
#endif
    );

    if(status == eHAL_STATUS_SUCCESS)
    {
#if defined(LIBRA_WAPI_SUPPORT)
        if(eSIR_ED_WPI == encType)
        {
            Qwlanfw_AddRemoveKeyReqType addKey;
            tANI_U8 uFwMesgType = QWLANFW_HOST2FW_WPI_KEY_SET;

            if(fGTK)
            {
                addKey.keyType = QWLANFW_KEY_TYPE_GTK;
            }
            else
            {
                addKey.keyType = QWLANFW_KEY_TYPE_PTK;
            }
            
            addKey.keyIndex = defKeyId;//In the case of WAPI, setting this to the Key Id obtained OTA
            addKey.dpuIndex = dpuIdx;
            addKey.reserved0 = 0;
            status = halFW_SendMsg(pMac, HAL_MODULE_ID_WAPI, uFwMesgType, 0, 
                                         sizeof(Qwlanfw_AddRemoveKeyReqType), (void *)&addKey, FALSE, NULL);
            if(!HAL_STATUS_SUCCESS(status))
            {
                if(pMac->hal.halMac.isFwInitialized)
                {
                    //if FW already initialized, should never fail. Assert here.
                    VOS_ASSERT(0);
                }
                goto failed;
                    
            }
        } //if(eSIR_ED_WPI == encType)
#endif
        return status;
    }


failed:

    if(newDpuIdx != HAL_INVALID_KEYID_INDEX )
        halDpu_ReleaseId(pMac, keyIdx);

    if(keyIdx != HAL_INVALID_KEYID_INDEX )
        halDpu_ReleaseKeyId(pMac, keyIdx);

    if(micKeyIdx != HAL_INVALID_KEYID_INDEX )
        halDpu_ReleaseMicKeyId(pMac, micKeyIdx);

    if(rcIdx != HAL_INVALID_KEYID_INDEX )
        halDpu_ReleaseRCId(pMac, dpuIdx, rcIdx);

    return status;
}


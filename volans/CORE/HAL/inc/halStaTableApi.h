#ifndef _HALSTA_TABLE_API_H_
#define _HALSTA_TABLE_API_H_

#include "palTypes.h"
#include "halTypes.h"    // eHalStatus
#include "aniGlobal.h"   // tpAniSirGlobal
#include "sirTypes.h"    // tSirMacAddr
#include "halBDApi.h"    // eFrameTxDir
#include "halMsgApi.h"   // tStaRateMode
#include "halFw.h"       // bssRaBit bssRaParam

#define HAL_STA_SIG_AUTH_MASK	0x4

eHalStatus halTable_Open(tHalHandle hHal, void *arg);
eHalStatus halTable_Start(tHalHandle hHal, void *arg);
eHalStatus halTable_Stop(tHalHandle hHal, void *arg);
eHalStatus halTable_Close(tHalHandle hHal, void *arg);

eHalStatus halTable_GetStaId(tpAniSirGlobal pMac, tANI_U8 type, tSirMacAddr bssId, tSirMacAddr staAddr, tANI_U8 *id);
eHalStatus halTable_GetBssIndex(tpAniSirGlobal pMac, tSirMacAddr bssId, tANI_U8 *id);
eHalStatus halTable_SetBssDpuIdx(tpAniSirGlobal pMac, tANI_U8 bssIdx, tANI_U8 dpuIdx);
eHalStatus halTable_GetBssDpuIdx(tpAniSirGlobal pMac, tANI_U8 bssIdx, tANI_U8 *pDpuIdx);
eHalStatus halTable_SetBssBcastMgmtDpuIdx(tpAniSirGlobal pMac, tANI_U8 bssIdx, tANI_U8 mgmtDpuIdx);
eHalStatus halTable_GetBssBcastMgmtDpuIdx(tpAniSirGlobal pMac, tANI_U8 bssIdx, tANI_U8 *mgmtDpuIdx);
eHalStatus halTable_BssAddSta(tpAniSirGlobal pMac, tANI_U8 bssIdx, tANI_U8 staIdx);
eHalStatus halTable_BssDelSta(tpAniSirGlobal pMac, tANI_U8 bssIdx, tANI_U8 staIdx);


eHalStatus halTable_ClearSta(tpAniSirGlobal pMac, tANI_U8 staIndex);
eHalStatus halTable_ClearBss(tpAniSirGlobal pMac, tANI_U8 bssIndex);

/* search STA by bssid,associd and sta addr */
eHalStatus halTable_FindStaidByAddrAndBssid(tpAniSirGlobal pMac, tSirMacAddr bssId, tSirMacAddr staAddr, tANI_U8 *id);
eHalStatus halTable_FindStaidByAddr(tHalHandle hHalHandle, tSirMacAddr staAddr, tANI_U8 *id);
eHalStatus halTable_FindAddrByStaid(tpAniSirGlobal pMac, tANI_U8 id, tSirMacAddr staAddr);
eHalStatus halTable_FindAddrByBssid( tpAniSirGlobal pMac, tANI_U8 bssIdx, tSirMacAddr bssId );
eHalStatus halTable_FindBssidByAddr(tpAniSirGlobal pMac, tSirMacAddr bssId, tANI_U8 *id);
eHalStatus halTable_FindStaidByAddrAndType(tpAniSirGlobal pMac, tSirMacAddr staAddr, tANI_U8 *id, tANI_U8 type);
eHalStatus halTable_FindStaIdByAssocId(tpAniSirGlobal pMac, tANI_U16 assocId, tANI_U8 *staId);

eHalStatus halTable_SaveStaConfig(tpAniSirGlobal pMac, tHalCfgSta *staEntry, tANI_U8 staId);
eHalStatus halTable_SaveBssConfig(tpAniSirGlobal pMac, 
         tANI_U8 rfBand, bssRaParam config, tANI_U8 bssIdx);
eHalStatus halTable_SaveEncMode(tpAniSirGlobal pMac, tANI_U8 staIdx, tAniEdType encMode);
eHalStatus halTable_RestoreStaConfig(tpAniSirGlobal pMac, tHalCfgSta *staEntry, tANI_U8 staId);
eHalStatus halTable_GetStaConfig(tpAniSirGlobal pMac, tHalCfgSta **staEntry, tANI_U8 staIdx);
eHalStatus halTable_GetBssType(tpAniSirGlobal pMac, tANI_U8 bssId, tANI_U8 *bssType);
eHalStatus halTable_GetStaIndexForBss(tpAniSirGlobal pMac, tANI_U8 bssIndex, tANI_U8 *staIndex);
eHalStatus halTable_GetBssIndexForSta(tpAniSirGlobal pMac, tANI_U8 *bssIndex, tANI_U8 staIndex);
eHalStatus halTable_ValidateStaIndex(tpAniSirGlobal pMac, tANI_U8 staId);
eHalStatus halTable_ValidateBssIndex(tpAniSirGlobal pMac, tANI_U8 bssIdx);
eHalStatus halTable_UpdateBssACM(tpAniSirGlobal pMac, tEdcaParams *pEdcaParams, tANI_U8 *pPrevAcmMap, tANI_U8 *pNewAcmMap);
eHalStatus halTable_GetSelfMacAddr(tpAniSirGlobal pMac, tSirMacAddr selfMacAddr);
eHalStatus halStaTableGetBssIdx(tpAniSirGlobal pMac, tANI_U8 staId, tANI_U8 *pBssIdx);
eHalStatus halTable_SetStaType(tpAniSirGlobal pMac, tANI_U8 staIdx, tANI_U8 staType);
eHalStatus halTable_SetStaAddr(tpAniSirGlobal pMac, tANI_U8 staIdx, tSirMacAddr staAddr);
eHalStatus halTable_GetStaAddr(tpAniSirGlobal pMac, tANI_U8 staIdx, tANI_U8 **pStaAddr);
eHalStatus halTable_GetStaType(tpAniSirGlobal pMac, tANI_U8 staIdx, tANI_U8 *pStaType);
eHalStatus halTable_SetStaDpuIdx(tpAniSirGlobal pMac, tANI_U8 staIdx, tANI_U8 dpuIdx);
eHalStatus halTable_GetStaDpuIdx(tpAniSirGlobal pMac, tANI_U8 staIdx, tANI_U8 *pDpuIdx);
eHalStatus halTable_SetStaUMAIdx(tpAniSirGlobal pMac, tANI_U8 staIdx, tANI_U8 umaIdx);
eHalStatus halTable_GetStaUMAIdx(tpAniSirGlobal pMac, tANI_U8 staIdx, tANI_U8 *pUMAIdx);
eHalStatus halTable_SetStaUMABcastIdx(tpAniSirGlobal pMac, tANI_U8 staIdx, tANI_U8 umaBcastIdx);
eHalStatus halTable_SetStaopRateMode(tpAniSirGlobal pMac, tANI_U8 staIdx, tStaRateMode  opRateMode);
eHalStatus halTable_GetStaopRateMode(tpAniSirGlobal pMac, tANI_U8 staIdx, tStaRateMode *popRateMode);
eHalStatus halTable_SetStaBcastDpuIdx(tpAniSirGlobal pMac,tANI_U8 staIdx,tANI_U8 dpuIdx);
eHalStatus halTable_GetStaBcastDpuIdx(tpAniSirGlobal pMac,tANI_U8 staIdx,tANI_U8 * pDpuIdx);
eHalStatus halTable_GetStaUMABcastIdx(tpAniSirGlobal pMac, tANI_U8 staIdx, tANI_U8 *pUMABcastIdx);
eHalStatus halTable_SetStaBcastMgmtDpuIdx(tpAniSirGlobal pMac, tANI_U8 staIdx, tANI_U8 dpuIdx);
eHalStatus halTable_GetStaBcastMgmtDpuIdx(tpAniSirGlobal pMac, tANI_U8 staIdx, tANI_U8 *pDpuIdx);
eHalStatus halTable_SetStaAssocId(tpAniSirGlobal pMac, tANI_U8 staIdx, tANI_U16 assocId);
eHalStatus halTable_GetStaAssocId(tpAniSirGlobal pMac, tANI_U8 staIdx, tANI_U16 * pAssocId);
eHalStatus halTable_SetStaSignature(tpAniSirGlobal pMac, tANI_U8 staIdx, tANI_U8 staSignature);
eHalStatus halTable_GetStaSignature(tpAniSirGlobal pMac, tANI_U8 staIdx, tANI_U8 *pStaSignature);
eHalStatus halTable_SetStaAuthState(tpAniSirGlobal pMac, tANI_U8 staIdx, tANI_BOOLEAN authenticated);
eHalStatus halTable_SetStaIdxForBss(tpAniSirGlobal pMac, tANI_U8 bssIdx, tANI_U8 staIdx);
eHalStatus halTable_SetStaQosEnabled(tpAniSirGlobal pMac, tANI_U8 staIdx, tANI_U8 qosEnabled);
eHalStatus halTable_GetStaQosEnabled(tpAniSirGlobal pMac, tANI_U8 staIdx, tANI_U8 *qosEnabled);
eHalStatus halTable_SetStaHtEnabled(tpAniSirGlobal pMac, tANI_U8 staIdx, tANI_U8 htEnabled, tANI_U8 gfEnabled);
eHalStatus halTable_SetBeaconIntervalForBss(tpAniSirGlobal pMac, tANI_U8 bssIdx, tANI_U16 bcnInterval);
eHalStatus halTable_GetBeaconIntervalForBss(tpAniSirGlobal pMac, tANI_U8 bssIdx, tANI_U16 *pbcnInterval);

eHalStatus halIsBssTableEmpty(tpAniSirGlobal pMac);

eHalStatus halTable_FindStaInBss( tpAniSirGlobal pMac, tANI_U8 bssIdx, tANI_U8 *pStaIdx);

eHalStatus halSta_getDefaultRCDescriptorFields( tpAniSirGlobal pMac,
    tHalCfgSta staEntry,
    tANI_U16 *rce,
    tANI_U16 *wce,
    tANI_U8  *winChkSize );

eHalStatus halTable_SetStaBASessionID( tpAniSirGlobal pMac,
    tANI_U8 staIdx,
    tANI_U8 baTID,
    tANI_U16 baSessionID );
eHalStatus halTable_GetStaBASessionID( tpAniSirGlobal pMac,
    tANI_U8 staIdx,
    tANI_U8 baTID,
    tANI_U16 *baSessionID );

// Get/Set the Max Ampdu density for the STA
eHalStatus halTable_SetStaMaxAmpduDensity(tpAniSirGlobal pMac, tANI_U8 staIdx, tANI_U8 density);
eHalStatus halTable_GetStaMaxAmpduDensity(tpAniSirGlobal pMac, tANI_U8 staIdx, tANI_U8 *pDensity);

eHalStatus halTable_SetStaAddBAReqParams(tpAniSirGlobal pMac, tANI_U16 staIdx,
    tANI_U8 tid, tSavedAddBAReqParamsStruct addBAReqParamsStruct);
eHalStatus halTable_GetStaAddBAParams(tpAniSirGlobal pMac,
    tSavedAddBAReqParamsStruct* pStaAddBAParams);

/* Set/Get the stations BTQM TX config */
eHalStatus halTable_SetStaTxConfig( tpAniSirGlobal pMac, 
        tANI_U32 staIdx, tANI_U32 txConfig);
eHalStatus halTable_GetStaTxConfig( tpAniSirGlobal pMac, 
        tANI_U32 staIdx, tANI_U32 *pTxConfig);

/* debug/dump support */
void halTableDbg_dumpStaTable(tpAniSirGlobal pMac);
void halTableDbg_dumpBssTable(tpAniSirGlobal pMac);

/* mac hash table support */

eHalStatus halTable_SetDtimPeriod( tpAniSirGlobal pMac, tANI_U8 bssIdx, tANI_U8 dtim);
eHalStatus halTable_GetDtimPeriod( tpAniSirGlobal pMac, tANI_U8 bssIdx, tANI_U8 *pDtim);

eHalStatus halTable_AddToStaCache(tpAniSirGlobal pMac, tSirMacAddr staAddr, tANI_U8 staId);
void halTable_RemoveFromStaCache(tpAniSirGlobal pMac,tSirMacAddr     staAddr);
eHalStatus  halTable_StaIdCacheFind(tpAniSirGlobal pMac, tSirMacAddr mac, tANI_U8 *staID);
eHalStatus halTable_StaCacheOpen(tHalHandle hHal, void *arg);
eHalStatus halTable_StaCacheStart(tHalHandle hHal, void *arg);
eHalStatus halTable_StaCacheStop(tHalHandle hHal, void *arg);
eHalStatus halTable_StaCacheClose(tHalHandle hHal, void *arg);
#define STA_ADDR_HASH(staAddr) (staAddr[5])
#endif /* _HALSTA_TABLE_API_H_ */


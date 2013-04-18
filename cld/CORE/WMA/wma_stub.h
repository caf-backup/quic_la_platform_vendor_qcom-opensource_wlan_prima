#ifndef WMA_STUB
#define WMA_STUB
#include "ol_txrx_htt_api.h"
#include "ol_txrx_ctrl_api.h"
#include "ol_txrx_osif_api.h"

#include "adf_nbuf.h"
#include "vos_api.h"
#include "vos_packet.h"
#include "vos_types.h"
#include "vos_api.h"
#include "vos_packet.h"
#include "vos_types.h"
#include "adf_nbuf.h"
#include "vos_api.h"
#include "vos_types.h"

static inline VOS_STATUS wma_shutdown(v_PVOID_t pVosContext, v_BOOL_t closeTransport)
{
	return VOS_STATUS_SUCCESS;
}

static inline void WMA_TimerTrafficStatsInd(void *pWMA) {
	return;
}

static inline VOS_STATUS WMA_GetWcnssHardwareVersion(v_PVOID_t pvosGCtx,
		tANI_U8 *pVersion,
		tANI_U32 versionBufferSize)
{
	return VOS_STATUS_SUCCESS;
}

static inline VOS_STATUS WMA_GetWcnssWlanCompiledVersion(v_PVOID_t pvosGCtx,
		tSirVersionType *pVersion)
{
	return VOS_STATUS_SUCCESS;
}

static inline tANI_U8 WMA_getFwWlanFeatCaps(tANI_U8 featEnumValue)
{
	return featEnumValue;
}

static inline void WMA_disableCapablityFeature(tANI_U8 feature_index) {
	return;
}

static inline VOS_STATUS
WMA_DS_PeekRxPacketInfo
(
 vos_pkt_t *vosDataBuff,
 v_PVOID_t *ppRxHeader,
 v_BOOL_t  bSwap
 ){
	return VOS_STATUS_SUCCESS;
}

static inline VOS_STATUS WMA_GetWcnssSoftwareVersion(v_PVOID_t pvosGCtx,
		tANI_U8 *pVersion,
		tANI_U32 versionBufferSize){
	return VOS_STATUS_SUCCESS;
}

static inline VOS_STATUS WMA_HALDumpCmdReq(tpAniSirGlobal   pMac, tANI_U32  cmd,
		tANI_U32   arg1, tANI_U32   arg2, tANI_U32   arg3,
		tANI_U32   arg4, tANI_U8   *pBuffer)	{
	return VOS_STATUS_SUCCESS;
}

static inline void WMA_TrafficStatsTimerActivate(v_BOOL_t activate) 
{
	return;
}

static inline VOS_STATUS WMA_GetWcnssWlanReportedVersion(v_PVOID_t pvosGCtx,
		tSirVersionType *pVersion) 
{
	return VOS_STATUS_SUCCESS;
}

static inline void WMA_featureCapsExchange(v_PVOID_t pVosContext) {
	return;
}
#endif


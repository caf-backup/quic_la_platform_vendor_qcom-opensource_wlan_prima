/**
 *
 *  @file:         halRpe.c
 *
 *  @brief:       Provides all the MAC APIs to the RPE Hardware Block.
 *
 *  @author:    Madhava Reddy S
 *
 *  Copyright (C) 2002 - 2007, Qualcomm, Inc. All rights reserved.
 *
 *  Change History:
 * ---------------------------------------
 * 06/07/2007  File created.
 * 01/30/2008  Moved from bringup to virgo production.
 */

#include "halTypes.h"
#include "palTypes.h"
#include "aniGlobal.h"
#ifdef WLAN_HAL_VOLANS
#include "volansDefs.h"
#else
#include "libraDefs.h"
#endif
#include "halRpe.h"
#include "sirMacProtDef.h"
#include "halDebug.h"
#include "halAdu.h"
#include "limApi.h"

eHalStatus rpe_init_error_interrupt(tpAniSirGlobal pMac);

/**
 * \brief Intialize the RPE error interrupts
 *
 * \sa rpe_init_error_interrupt
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \return eHalStatus Interrupt initialization status
 */
eHalStatus rpe_init_error_interrupt(tpAniSirGlobal pMac)
{
	tANI_U32 value;

	value = QWLAN_RPE_ERR_INT_ENABLE_BU_SM_STUCK_ERR_INT_EN_MASK |
            	QWLAN_RPE_ERR_INT_ENABLE_DD_SM_STUCK_ERR_INT_EN_MASK |
            	QWLAN_RPE_ERR_INT_ENABLE_BMU_BD_AVAIL_ERR_INT_ENABLE_MASK |
            	QWLAN_RPE_ERR_INT_ENABLE_GBI_ERR_INT_EN_MASK |
            	QWLAN_RPE_ERR_INT_ENABLE_GAM_ERR_INT_EN_MASK;

	if (halRpeinit_error_interrupt(pMac, value) != eHAL_STATUS_SUCCESS)
    	return eHAL_STATUS_FAILURE;

	return eHAL_STATUS_SUCCESS;
}


/**
 * \brief Configure the RPE routing flag
 *
 * \sa halRpe_CfgRoutingFlag
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param drop_pkts Queue to route drop packets
 *
 * \param good_pkts Queue to route good packets
 *
 * \return eHalStatus Routing flag configuration status
 */
eHalStatus halRpe_CfgRoutingFlag(tpAniSirGlobal pMac, tANI_U32 drop_pkts,
                                        	tANI_U32 good_pkts)
{

	halWriteRegister(pMac, QWLAN_RPE_ROUTING_FLAG_REG,
                    (drop_pkts << QWLAN_RPE_ROUTING_FLAG_ROUTING_DROP_PACKET_OFFSET)| good_pkts);

	return eHAL_STATUS_SUCCESS;
}


/**
 * \brief Configure max Stations and Queues
 *
 * \sa halRpe_program_staid_qid
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param maxRpeStations Maximum RPE Stations supported
 *
 * \param maxRpeQueues Maximum RPE Queues supported
 *
 * \return eHalStatus Station Queue configuration status
 */
eHalStatus halRpe_program_staid_qid(tpAniSirGlobal pMac,
                    	tANI_U32 maxRpeStations, tANI_U32 maxRpeQueues)
{

	halWriteRegister(pMac, QWLAN_RPE_FULLSTATE_STATIONS_QUEUES_REG,
                    (maxRpeQueues << QWLAN_BMU_TX_QUEUE_STAID_QUEUEID_CONFIG_MAX_VALID_QUEUEID_OFFSET)|
                	maxRpeStations);

	return eHAL_STATUS_SUCCESS;
}

/**
 * \brief Configure RPE Station Descriptor Base
 *
 * \sa halRpe_sta_desc_base
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param rpeStaDesc_offset Station Descriptor Base adress
 *
 * \return eHalStatus Station Descriptor Base configuration status
 */
eHalStatus halRpe_sta_desc_base(tpAniSirGlobal pMac, tANI_U32 rpeStaDesc_offset)
{
    /** Zero out the RPE STA descriptor */
	halZeroDeviceMemory(pMac, pMac->hal.memMap.rpeStaDesc_offset,
                                    	pMac->hal.memMap.rpeStaDesc_size);

	halWriteRegister(pMac, QWLAN_RPE_MEM_BASE_ADDR_REG,
                	rpeStaDesc_offset);

	return eHAL_STATUS_SUCCESS;
}


/**
 * \brief Initialize the RPE module
 *
 * \sa halRpe_Start
 *
 * \param hHal The global tHalHandle object
 *
 * \param arg Pointer to arguments
 *
 * \return eHalStatus RPE module initialization status
 */
eHalStatus halRpe_Start(tHalHandle hHal, void *arg)
{
	tpAniSirGlobal 	pMac = (tpAniSirGlobal)hHal;
	tANI_U32		drop_pkts, good_pkts;

#ifdef FEATURE_ON_CHIP_REORDERING
    tANI_U32 value;
#endif

    /** Initialize the number of Queues and Stations */
	if (halRpe_program_staid_qid(pMac, pMac->hal.memMap.maxStations - 1,
                	pMac->hal.memMap.maxHwQueues) != eHAL_STATUS_SUCCESS)
    	return eHAL_STATUS_FAILURE;

    /** Program the RPE STA data structure Base */
	if (halRpe_sta_desc_base(pMac, pMac->hal.memMap.rpeStaDesc_offset)
                                                != eHAL_STATUS_SUCCESS)
    	return eHAL_STATUS_FAILURE;

    /** Configure the routing flag */
	drop_pkts = 0xFF;                    /**< Default value */
	good_pkts = QWLAN_RPE_ROUTING_FLAG_ROUTING_GOOD_PACKET_DEFAULT; /**< Default value */
	if (halRpe_CfgRoutingFlag(pMac, drop_pkts, good_pkts)
                                                != eHAL_STATUS_SUCCESS)
    	return eHAL_STATUS_FAILURE;

    /** Enable error interrupts */
    if (rpe_init_error_interrupt(pMac) != eHAL_STATUS_SUCCESS)
        return eHAL_STATUS_FAILURE;

#ifdef FEATURE_ON_CHIP_REORDERING
    /* Forward out of order packets packets arriving in reorder */
    halWriteRegister(pMac, QWLAN_RPE_RPE_CONFIG_REG,
         QWLAN_RPE_RPE_CONFIG_DEFAULT | (1 << QWLAN_RPE_RPE_CONFIG_OUOFORD_PKT_FWD_OFFSET));

    halWriteRegister(pMac, QWLAN_RPE_MEMORY_THRESHOLD_FOR_RPE_REG,
                    QWLAN_RPE_MEMORY_THRESHOLD_FOR_RPE_MEMORY_UNITS_DEFAULT | 
                    QWLAN_RPE_MEMORY_THRESHOLD_FOR_RPE_MEMORY_THRESHOLD_DEFAULT);

    /* Program qid_to_acx_mapping registers where x:0-3*/
    value = QWLAN_RPE_QID_TO_AC0_MAPPING_QID3_AC0_MAPPING_DEFAULT |
            QWLAN_RPE_QID_TO_AC0_MAPPING_QID2_AC0_MAPPING_DEFAULT;
    value |= (BTQM_QUEUE_TX_TID_3 << QWLAN_RPE_QID_TO_AC0_MAPPING_QID1_AC0_MAPPING_OFFSET |
             BTQM_QUEUE_TX_TID_0 << QWLAN_RPE_QID_TO_AC0_MAPPING_QID0_AC0_MAPPING_OFFSET);

    halWriteRegister(pMac, QWLAN_RPE_QID_TO_AC0_MAPPING_REG, value);

    value = QWLAN_RPE_QID_TO_AC1_MAPPING_QID3_AC1_MAPPING_DEFAULT |
            QWLAN_RPE_QID_TO_AC1_MAPPING_QID2_AC1_MAPPING_DEFAULT;
    value |= (BTQM_QUEUE_TX_TID_2 << QWLAN_RPE_QID_TO_AC1_MAPPING_QID1_AC1_MAPPING_OFFSET |
            BTQM_QUEUE_TX_TID_1 << QWLAN_RPE_QID_TO_AC1_MAPPING_QID0_AC1_MAPPING_OFFSET);

    halWriteRegister(pMac, QWLAN_RPE_QID_TO_AC1_MAPPING_REG, value);

    value = QWLAN_RPE_QID_TO_AC2_MAPPING_QID3_AC2_MAPPING_DEFAULT |
            QWLAN_RPE_QID_TO_AC2_MAPPING_QID2_AC2_MAPPING_DEFAULT;
    value |= (BTQM_QUEUE_TX_TID_4 << QWLAN_RPE_QID_TO_AC2_MAPPING_QID1_AC2_MAPPING_OFFSET |
            BTQM_QUEUE_TX_TID_5 << QWLAN_RPE_QID_TO_AC2_MAPPING_QID0_AC2_MAPPING_OFFSET);

    halWriteRegister(pMac, QWLAN_RPE_QID_TO_AC2_MAPPING_REG, value);

    value = QWLAN_RPE_QID_TO_AC3_MAPPING_QID3_AC3_MAPPING_DEFAULT |
            QWLAN_RPE_QID_TO_AC3_MAPPING_QID2_AC3_MAPPING_DEFAULT;
    value |= (BTQM_QUEUE_TX_TID_6 << QWLAN_RPE_QID_TO_AC3_MAPPING_QID1_AC3_MAPPING_OFFSET |
            BTQM_QUEUE_TX_TID_7 << QWLAN_RPE_QID_TO_AC3_MAPPING_QID0_AC3_MAPPING_OFFSET);

    halWriteRegister(pMac, QWLAN_RPE_QID_TO_AC3_MAPPING_REG, value);

    pMac->hal.halMac.numOfOnChipReorderSessions = 0;
    pMac->hal.halMac.maxNumOfOnChipReorderSessions = 0;

    /* Zero out the RPE STA descriptor */
    halZeroDeviceMemory(pMac, pMac->hal.memMap.rpeReOrderSTADataStructure_offset,
                                    pMac->hal.memMap.rpeReOrderSTADataStructure_size);

#endif /* FEATURE_ON_CHIP_REORDERING */

    /** Zero out the RPE Partial Bit Map */
	halZeroDeviceMemory(pMac, pMac->hal.memMap.rpePartialBitmap_offset,
        	pMac->hal.memMap.rpePartialBitmap_size) ;

	return eHAL_STATUS_SUCCESS;
}


/**
 * \brief Initialize RPE error interrupts
 *
 * \sa halRpeinit_error_interrupt
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param error_mask Error interrupt mask
 *
 * \return eHalStatus Error interrupts initialization status
 */
eHalStatus halRpeinit_error_interrupt(tpAniSirGlobal pMac, tANI_U32 error_mask)
{
	halWriteRegister(pMac, QWLAN_RPE_ERR_INT_ENABLE_REG,
                	error_mask);

	return eHAL_STATUS_SUCCESS;
}

/**
 * \brief Retreive RPE error interrupt status
 *
 * \sa halRpe_interrupt_status
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param intr_status Pointer to store error interrupt status
 *
 * \return eHalStatus Error interrupt reterival status
 */
eHalStatus halRpe_interrupt_status(tpAniSirGlobal pMac, tANI_U32 *intr_status)
{
	halReadRegister(pMac, QWLAN_RPE_ERR_INT_STATUS_REG,
                	intr_status) ;

	return eHAL_STATUS_SUCCESS;
}

/**
 * \brief Configure the RPE Station Descriptor
 *
 * \sa halRpe_cfgStaDesc
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param staId Station Id to set the descriptor
 *
 * \param rpeStaDesc Pointer to rpe station descriptor information
 *
 * \param rpePartialBAInfo Pointer to rpe partial block ack information
 *
 * \return eHalStatus Station descriptor configuration status
 */
 eHalStatus halRpe_cfgStaDesc(tpAniSirGlobal pMac, tANI_U32 staIdx,
                                                	tpRpeStaDesc pRpeStaDesc)
{
	tANI_U32	    	address;

	address = pMac->hal.memMap.rpeStaDesc_offset + (staIdx * RPE_STA_DESC_ENTRY_SIZE);

#if 0
    /** Zero out the RPE STA descriptor */
	if (halZeroDeviceMemory(pMac, address,
                            	sizeof(rpeStaDesc)) != eHAL_STATUS_SUCCESS)
    	return eHAL_STATUS_FAILURE;
#endif
    /** Save the STA config */
	if (halRpe_SaveStaConfig(pMac, pRpeStaDesc, (tANI_U8) staIdx) != eHAL_STATUS_SUCCESS)
    	return eHAL_STATUS_FAILURE;

    /** Write to RPE STA Desc memory */
	halWriteDeviceMemory(pMac, address, (tANI_U8 *)pRpeStaDesc,
                            	RPE_STA_DESC_ENTRY_SIZE);

	return eHAL_STATUS_SUCCESS;
}


/**
 * \brief Get RPE Station Descriptor
 *
 * \sa halRpe_GetStaDesc
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param staId Station Id to get the descriptor
 *
 * \param rpeStaDesc Pointer to sotre descriptor information
 *
 * \return eHalStatus Station descriptor retreiving status
 */
eHalStatus halRpe_GetStaDesc(tpAniSirGlobal pMac, tANI_U8 staId,
                                        	tpRpeStaDesc rpeStaDesc)
{
	tANI_U32	address;

	palFillMemory(pMac->hHdd, (void *)rpeStaDesc, RPE_STA_DESC_ENTRY_SIZE, 0);

	address = pMac->hal.memMap.rpeStaDesc_offset + (staId * RPE_STA_DESC_ENTRY_SIZE);
	halReadDeviceMemory(pMac, address, (tANI_U8 *)rpeStaDesc,
                    	RPE_STA_DESC_ENTRY_SIZE);

	return eHAL_STATUS_SUCCESS;
}


/**
 * \brief Save the Sta Descriptor Configuration
 *
 * \sa halRpe_SaveStaConfig
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param tpRpeStaDesc Pointer to rpe sta desc configuration
 *
 * \param staIdx Station index
 *
 * \return eHalStatus save sta desc config status
 */
eHalStatus halRpe_SaveStaConfig(tpAniSirGlobal pMac, tpRpeStaDesc pRpeStaDesc, tANI_U8 staIdx)
{
    eHalStatus status = eHAL_STATUS_INVALID_STAIDX;
    tpStaStruct t = (tpStaStruct) pMac->hal.halMac.staTable;

    if (staIdx < pMac->hal.halMac.maxSta)
    {
        palCopyMemory(pMac->hHdd, (tANI_U8 *) &t[staIdx].rpeStaDesc, (tANI_U8 *)pRpeStaDesc, RPE_STA_DESC_ENTRY_SIZE);
        status = eHAL_STATUS_SUCCESS;
    }

    return status;
}

eHalStatus halRpe_SaveStaQueueConfig(tpAniSirGlobal pMac, tANI_U8 staIdx, tANI_U32 queueId,
                                                        	tpRpeStaQueueInfo rpeStaQueueInfo)
{
    eHalStatus status = eHAL_STATUS_INVALID_STAIDX;
    tpStaStruct t = (tpStaStruct) pMac->hal.halMac.staTable;

    if (staIdx < pMac->hal.halMac.maxSta)
    {
        palCopyMemory(pMac->hHdd, (tANI_U8 *) &t[staIdx].rpeStaDesc.rpeStaQueueInfo[queueId],
                                        (tANI_U8 *)rpeStaQueueInfo, RPE_STA_DESC_QUEUE_SIZE);
        status = eHAL_STATUS_SUCCESS;
    }

    return status;
}

eHalStatus halRpe_RestoreStaQueueConfig(tpAniSirGlobal pMac, tANI_U8 staIdx, tANI_U32 queueId,
                                                        	tpRpeStaQueueInfo rpeStaQueueInfo)
{
    eHalStatus status = eHAL_STATUS_INVALID_STAIDX;
    tpStaStruct t = (tpStaStruct) pMac->hal.halMac.staTable;

    if (staIdx < pMac->hal.halMac.maxSta)
    {
        palCopyMemory(pMac->hHdd, (tANI_U8 *)rpeStaQueueInfo, (tANI_U8 *) &t[staIdx].rpeStaDesc.rpeStaQueueInfo[queueId],
                                                                                    	RPE_STA_DESC_ENTRY_SIZE);
        status = eHAL_STATUS_SUCCESS;
    }

    return status;
}


/**
 * \brief Restore the Sta Descriptor Configuration
 *
 * \sa halRpe_RestoreStaConfig
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param pRpeStaDesc Pointer to resotore rpe sta desc configuration
 *
 * \param staIdx Station index
 *
 * \return eHalStatus Restore sta desc config status
 */
eHalStatus halRpe_RestoreStaConfig(tpAniSirGlobal pMac, tpRpeStaDesc pRpeStaDesc, tANI_U8 staIdx)
{
    eHalStatus status = eHAL_STATUS_INVALID_STAIDX;
    tpStaStruct t = (tpStaStruct) pMac->hal.halMac.staTable;

    if (staIdx < pMac->hal.halMac.maxSta)
    {
        palCopyMemory(pMac->hHdd, (tANI_U8 *)pRpeStaDesc, (tANI_U8 *) &t[staIdx].rpeStaDesc, RPE_STA_DESC_ENTRY_SIZE);
        status = eHAL_STATUS_SUCCESS;
    }

    return status;
}

/**
 * \brief Configure the RPE Station Descriptor
 *
 * \sa halRpe_cfgStaDesc
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param staId Station Id to set the descriptor
 *
 * \param rpeStaDesc Pointer to rpe station descriptor information
 *
 * \param rpePartialBAInfo Pointer to rpe partial block ack information
 *
 * \return eHalStatus Station descriptor configuration status
 */
 eHalStatus halRpe_UpdateStaDesc(tpAniSirGlobal pMac, tANI_U32 staIdx,
                                                	tpRpeStaDesc rpeStaDesc)
{
	tANI_U32	    	address;

	address = pMac->hal.memMap.rpeStaDesc_offset + (staIdx * RPE_STA_DESC_ENTRY_SIZE);

    /** Write to RPE STA Desc memory */
	halWriteDeviceMemory(pMac, address, (tANI_U8 *)rpeStaDesc,
                            	RPE_STA_DESC_ENTRY_SIZE);

	return eHAL_STATUS_SUCCESS;
}


eHalStatus halRpe_UpdateStaDescQueueInfo(tpAniSirGlobal pMac, tANI_U32 staIdx,
                                        	tANI_U32 queueId, tpRpeStaQueueInfo rpeStaQueueInfo)
{

	tANI_U32	    	address;

	address = pMac->hal.memMap.rpeStaDesc_offset + (staIdx * RPE_STA_DESC_ENTRY_SIZE) +
                                (queueId * RPE_STA_DESC_QUEUE_SIZE);

    /** Write to RPE STA Desc memory */
	halWriteDeviceMemory(pMac, address, (tANI_U8 *)rpeStaQueueInfo,
                            	RPE_STA_DESC_QUEUE_SIZE) ;

	return eHAL_STATUS_SUCCESS;
}


eHalStatus halRpe_GetStaDescQueueInfo(tpAniSirGlobal pMac, tANI_U32 staIdx,
                                        	tANI_U32 queueId, tpRpeStaQueueInfo rpeStaQueueInfo)
{

	tANI_U32	    	address;

	palFillMemory(pMac->hHdd, (void *)rpeStaQueueInfo, RPE_STA_DESC_QUEUE_SIZE, 0);

	address = pMac->hal.memMap.rpeStaDesc_offset + (staIdx * RPE_STA_DESC_ENTRY_SIZE) +
                                (queueId * RPE_STA_DESC_QUEUE_SIZE);

    /** Write to RPE STA Desc memory */
	halReadDeviceMemory(pMac, address, (tANI_U8 *)rpeStaQueueInfo,
                            	RPE_STA_DESC_QUEUE_SIZE);

	return eHAL_STATUS_SUCCESS;
}

eHalStatus halRpe_BlockAndFlushFrames(tpAniSirGlobal pMac, tANI_U8 staIdx, tANI_U8 queueId,
                                            	tRpeSwBlockReq rpeSwBlockReq)
{

    /** Lock the RPE STA desc  */
	if (halRpe_UpdateSwBlockReq(pMac, staIdx, queueId, eRPE_SW_ENABLE_DROP) != eHAL_STATUS_SUCCESS)
    	return eHAL_STATUS_FAILURE;

    /** Flush the Queues */
	if (halRpe_FlushBitMapCache(pMac) != eHAL_STATUS_SUCCESS)
    	return eHAL_STATUS_FAILURE;

#ifdef FEATURE_ON_CHIP_REORDERING
    if (halRpe_FlushReorderPacketMemory(pMac, staIdx, queueId) != eHAL_STATUS_SUCCESS)
    	return eHAL_STATUS_FAILURE;
#endif

	if (halRpe_FlushrsrcEntry(pMac, staIdx, queueId) != eHAL_STATUS_SUCCESS)
    	return eHAL_STATUS_FAILURE;

	return eHAL_STATUS_SUCCESS;
}

eHalStatus halRpe_UpdateSwBlockReq(tpAniSirGlobal pMac, tANI_U8 staIdx, tANI_U8 queueId,
                                            	tRpeSwBlockReq rpeSwBlockReq)
{
	tANI_U32 value;

	value = (staIdx << QWLAN_RPE_RPE_SW_BLOCK_PKTS_SW_BLOCK_STAID_OFFSET) |
            (queueId << QWLAN_RPE_RPE_SW_BLOCK_PKTS_SW_BLOCK_QID_OFFSET) |
            	rpeSwBlockReq;

	halWriteRegister(pMac, QWLAN_RPE_RPE_SW_BLOCK_PKTS_REG, value);

	halReadRegister(pMac, QWLAN_RPE_RPE_SW_BLOCK_PKTS_REG, &value);

	return eHAL_STATUS_SUCCESS;
}

eHalStatus halRpe_FlushBitMapCache(tpAniSirGlobal pMac)
{

#ifdef WLAN_HAL_VOLANS
	//tANI_U32 value;
#else
	tANI_U32 value;
#endif

	halWriteRegister(pMac, QWLAN_RPE_SW_FLUSH_BITMAP_CACHE_REG,
        	QWLAN_RPE_SW_FLUSH_BITMAP_CACHE_CFG_FLUSH_PARTIALSTATE_CACHE_MASK |
        	QWLAN_RPE_SW_FLUSH_BITMAP_CACHE_CFG_FLUSH_FULLSTATE_CACHE_MASK);

#ifndef WLAN_HAL_VOLANS //FIXME_VOLANS
    /** Poll on both bits to clear */
	do {

    	halReadRegister(pMac, QWLAN_RPE_SW_FLUSH_BITMAP_CACHE_REG, &value) ;

    } while ((value & QWLAN_RPE_SW_FLUSH_BITMAP_CACHE_CFG_FLUSH_PARTIALSTATE_CACHE_MASK));

	do {

    	halReadRegister(pMac, QWLAN_RPE_SW_FLUSH_BITMAP_CACHE_REG, &value);

    } while ((value & QWLAN_RPE_SW_FLUSH_BITMAP_CACHE_CFG_FLUSH_FULLSTATE_CACHE_MASK));
#endif
	return eHAL_STATUS_SUCCESS;
}


eHalStatus halRpe_FlushrsrcEntry(tpAniSirGlobal pMac, tANI_U8 staIdx, tANI_U8 queueId)
{
	tANI_U32 value;

	value = (staIdx << QWLAN_RPE_RPE_SW_FLUSH_RSRC_ENTRY_SW_FLUSH_RSRC_STAID_OFFSET) |
            (queueId << QWLAN_RPE_RPE_SW_FLUSH_RSRC_ENTRY_SW_FLUSH_RSRC_ENTRY_QUEUE_OFFSET) |
        	QWLAN_RPE_RPE_SW_FLUSH_RSRC_ENTRY_SW_FLUSH_RSRC_ENTRY_MASK;

	halWriteRegister(pMac, QWLAN_RPE_RPE_SW_FLUSH_RSRC_ENTRY_REG,
                                        	value) ;


	do {

    	halReadRegister(pMac, QWLAN_RPE_RPE_SW_FLUSH_RSRC_ENTRY_REG,
                                                &value) ;
    } while ( (value & QWLAN_RPE_RPE_SW_FLUSH_RSRC_ENTRY_SW_FLUSH_RSRC_ENTRY_STATUS_MASK));

	return eHAL_STATUS_SUCCESS;

}

eHalStatus halRpe_ErrIntHandler(tHalHandle hHalHandle, eHalIntSources intSource){
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tANI_U32 intRegStatus;
    tpAniSirGlobal pMac = PMAC_STRUCT(hHalHandle);
	tANI_U8 tID;
	tANI_U8 staIdx;
	tANI_U32 Reason_Code;
	tANI_U32 rpeIntrStatus;
    tANI_U32 index, intHandled[]={
        QWLAN_RPE_ERR_INT_STATUS_2K_JUMP_SN_IN_BASESSION_INT_STATUS_MASK,
    	QWLAN_RPE_ERR_INT_STATUS_2K_JUMP_SSN_IN_BAR_INT_STATUS_MASK,
    	QWLAN_RPE_ERR_INT_STATUS_BAR_IN_NON_BASESSION_INT_STATUS_MASK,
    	QWLAN_RPE_ERR_INT_STATUS_FRAGPKT_IN_BASESSION_INT_STATUS_MASK
    };        
#define RPE_INT_MASK ((1<<QWLAN_RPE_ERR_INT_STATUS_2K_JUMP_SN_IN_BASESSION_INT_STATUS_OFFSET)-1)

	HALLOG1( halLog( pMac, LOG1, FL("halRpe_ErrIntHandler entered\n")));

    /** Read Interrupt Status.*/
	halRpe_interrupt_status(pMac, &rpeIntrStatus);
    intRegStatus = rpeIntrStatus;

	HALLOGE( halLog( pMac, LOGE, FL("RPE Err Intr status %x\n"),intRegStatus));
    for (index=0; index < sizeof(intHandled)/sizeof(tANI_U32); index++){
        if(rpeIntrStatus & intHandled[index]){
			
        	staIdx = (tANI_U8)((rpeIntrStatus & QWLAN_RPE_ERR_INT_STATUS_INT_STAID_MASK) 
				>> QWLAN_RPE_ERR_INT_STATUS_INT_STAID_OFFSET) ;
        	tID =  (tANI_U8)((rpeIntrStatus & QWLAN_RPE_ERR_INT_STATUS_INT_QID_MASK ) 
				>> QWLAN_RPE_ERR_INT_STATUS_INT_QID_OFFSET);
        	Reason_Code = (rpeIntrStatus & QWLAN_RPE_ERR_INT_STATUS_REASON_CODE_MASK) 
				>> QWLAN_RPE_ERR_INT_STATUS_REASON_CODE_OFFSET;

			HALLOGW( halLog( pMac, LOGW, FL("staIdx %d tID %d RC %d\n"),
				staIdx,tID,Reason_Code));
            // Post the message to indicate deletion of BA
            (void)halMsg_PostBADeleteInd( pMac, staIdx, tID, eBA_RECIPIENT, Reason_Code);
        }
    }            

    //clear all interrupts already checked
    for (index=0; index < sizeof(intHandled)/sizeof(tANI_U32); index++)
      rpeIntrStatus &= ~intHandled[index];

    if(rpeIntrStatus & RPE_INT_MASK){
        HALLOGE( halLog(pMac, LOGE, FL("RPE FATAL Error Interrupt source %d, Status 0x%x\n"),
                intSource, intRegStatus));
        macSysResetReq(pMac, eSIR_RPE_EXCEPTION);
        return status;
    }

    return (eHAL_STATUS_SUCCESS);
}

#ifdef FEATURE_ON_CHIP_REORDERING
eHalStatus halRpe_FlushReorderPacketMemory(tpAniSirGlobal pMac, tANI_U8 staIdx, tANI_U8 queueId)
{
  tANI_U32 value;
  tANI_U32 error_status;

  /* Flush Reorder Packet Memory for this STA and QID */
  do {
    value = (staIdx << QWLAN_RPE_SW_FLUSH_BITMAP_CACHE_CFG_FWD_TO_DPU_STAID_OFFSET) |
              (queueId << QWLAN_RPE_SW_FLUSH_BITMAP_CACHE_CFG_FWD_TO_DPU_QID_OFFSET) |
              QWLAN_RPE_SW_FLUSH_BITMAP_CACHE_CFG_FWD_TO_DPU_MASK;

    halWriteRegister(pMac, QWLAN_RPE_SW_FLUSH_BITMAP_CACHE_REG, value);

    halReadRegister(pMac, QWLAN_RPE_SW_FLUSH_BITMAP_CACHE_REG, &value);

    /* Check fwd_to_dpu_fail_status and issue cfg_fwd_to_dpu again in case if the status is failed */
    error_status = value & QWLAN_RPE_SW_FLUSH_BITMAP_CACHE_FWD_TO_DPU_FAIL_STATUS_MASK;
  } while(error_status);

  /* fwd_to_dpu_done_status is set in case if the packets from RPE Reorder Packet Memory are forwarded to DPU */
  /* Reset fwd_to_dpu_done_status */
  value = QWLAN_RPE_SW_FLUSH_BITMAP_CACHE_FWD_TO_DPU_DONE_STATUS_DEFAULT << 
    QWLAN_RPE_SW_FLUSH_BITMAP_CACHE_FWD_TO_DPU_DONE_STATUS_OFFSET;

  halWriteRegister(pMac, QWLAN_RPE_SW_FLUSH_BITMAP_CACHE_REG, value);

  return eHAL_STATUS_SUCCESS;
}

eHalStatus halRPE_UpdateOnChipReorderThreshold(tpAniSirGlobal pMac, tANI_U32 threshold, tANI_U32 value)
{
  switch(threshold)
  {
    case WNI_CFG_RPE_POLLING_THRESHOLD:
      halWriteRegister(pMac, QWLAN_RPE_POLLING_THRESHOLD_FOR_RPE_REG, value*1000);
      break;

    case WNI_CFG_RPE_AGING_THRESHOLD_FOR_AC0_REG:
      halWriteRegister(pMac, QWLAN_RPE_AGING_THRESHOLD_FOR_AC0_REG, value*1000);
      break;

    case WNI_CFG_RPE_AGING_THRESHOLD_FOR_AC1_REG:
      halWriteRegister(pMac, QWLAN_RPE_AGING_THRESHOLD_FOR_AC1_REG, value*1000);
      break;

    case WNI_CFG_RPE_AGING_THRESHOLD_FOR_AC2_REG:
      halWriteRegister(pMac, QWLAN_RPE_AGING_THRESHOLD_FOR_AC2_REG, value*1000);
      break;

    case WNI_CFG_RPE_AGING_THRESHOLD_FOR_AC3_REG:
      halWriteRegister(pMac, QWLAN_RPE_AGING_THRESHOLD_FOR_AC3_REG, value*1000);
      break;

    default:
        HALLOG1( halLog(pMac, LOG1, FL("This case should never hit\n")));
        break;
  }

  return eHAL_STATUS_SUCCESS;
}

#endif /* FEATURE_ON_CHIP_REORDERING */


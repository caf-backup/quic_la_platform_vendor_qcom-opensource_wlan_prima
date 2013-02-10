/**
 *
 *  @file:         halRpe.c
 *
 *  @brief:       Provides all the MAC APIs to the RPE Hardware Block.
 *
 *  @author:    Madhava Reddy S
 *
 *  Copyright (C) 2002 - 2007, Qualcomm Technologies, Inc. All rights reserved.
 *
 *  Change History:
 * ---------------------------------------
 * 06/07/2007  File created.
 * 01/30/2008  Moved from bringup to virgo production.
 */

#include "halTypes.h"
#include "palTypes.h"
#include "aniGlobal.h"
#include "libraDefs.h"
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

    /** Zero out the RPE Partial Bit Map */
	halZeroDeviceMemory(pMac, pMac->hal.memMap.rpePartialBitmap_offset,
        	pMac->hal.memMap.rpePartialBitmap_size) ;

	pMac->hal.halMac.rpeErrIntrStatusVal = 0;
	pMac->hal.halMac.rpeErrCntThreshold = 2;
	pMac->hal.halMac.rpeErrCnt = 0;

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

	tANI_U32 value;

	halWriteRegister(pMac, QWLAN_RPE_SW_FLUSH_BITMAP_CACHE_REG,
        	QWLAN_RPE_SW_FLUSH_BITMAP_CACHE_CFG_FLUSH_PARTIALSTATE_CACHE_MASK |
        	QWLAN_RPE_SW_FLUSH_BITMAP_CACHE_CFG_FLUSH_FULLSTATE_CACHE_MASK);

    /** Poll on both bits to clear */
	do {

    	halReadRegister(pMac, QWLAN_RPE_SW_FLUSH_BITMAP_CACHE_REG, &value) ;

    } while ((value & QWLAN_RPE_SW_FLUSH_BITMAP_CACHE_CFG_FLUSH_PARTIALSTATE_CACHE_MASK));

	do {

    	halReadRegister(pMac, QWLAN_RPE_SW_FLUSH_BITMAP_CACHE_REG, &value);

    } while ((value & QWLAN_RPE_SW_FLUSH_BITMAP_CACHE_CFG_FLUSH_FULLSTATE_CACHE_MASK));

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

    /** Read Interrupt Status.*/
	halRpe_interrupt_status(pMac, &rpeIntrStatus);
    intRegStatus = rpeIntrStatus;

    for (index=0; index < sizeof(intHandled)/sizeof(tANI_U32); index++){
        if(rpeIntrStatus & intHandled[index]){
        	HALLOGE( halLog( pMac, LOGE,
              	FL("halRpe_ErrIntHandler : \n")));

            if(pMac->hal.halMac.rpeErrIntrStatusVal != intHandled[index])
            {
                pMac->hal.halMac.rpeErrIntrStatusVal = intHandled[index];
                pMac->hal.halMac.rpeErrCnt++;
                continue;
            }
            else
            {
                pMac->hal.halMac.rpeErrCnt++;
                if(pMac->hal.halMac.rpeErrCnt < pMac->hal.halMac.rpeErrCntThreshold)
                   continue;
            }            

        	staIdx = (tANI_U8)(rpeIntrStatus & QWLAN_RPE_ERR_INT_STATUS_STATION_ID_MASK) >> QWLAN_RPE_ERR_INT_STATUS_STATION_ID_OFFSET ;
        	tID =  (tANI_U8)(rpeIntrStatus & QWLAN_RPE_ERR_INT_STATUS_QUEUE_ID_MASK ) >> QWLAN_RPE_ERR_INT_STATUS_QUEUE_ID_OFFSET;
        	Reason_Code = (rpeIntrStatus & QWLAN_RPE_ERR_INT_STATUS_REASON_CODE_MASK) >> QWLAN_RPE_ERR_INT_STATUS_REASON_CODE_OFFSET;

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

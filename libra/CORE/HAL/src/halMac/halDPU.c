/*
 * Airgo Networks, Inc proprietary. All rights reserved.
 * halDPU.c:  Provides all the MAC APIs to the DPU Hardware Block.
 * Author:    Satish Gune
 * Date:      02/09/2006
 *
 * --------------------------------------------------------------------------
 */
#include "halTypes.h"
#include "palApi.h"
#include "libraDefs.h"
#include "halDPU.h"
#include "halDebug.h"
#include "cfgApi.h"             //wlan_cfgGetInt
#include "halMacSecurityApi.h"
#include "halUtils.h"

/* --------------------------------------------------------------------------
 * local defines
 */

//this is the offset for one writable unit (in bytes) in a RC Desc. which 
//holds WCE and window checkSize from the beginning 
//of the RC Desc.
#define HAL_DPU_RC_WCE_OFFSET 12 
//Writable unit in bytes for RC Descriptor.
#define HAL_DPU_RC_WRITABLE_UNIT 4
#define DPU_DESCRIPTOR_TIDOFFSET        (offsetof(tDpuDescriptor,sequenceField))

#define DPU_CP_INTENABLE_MASK \
    (DPU_CP_INTENABLE_EGR_RELNULL_ERROR_ENABLE_MASK | \
     DPU_CP_INTENABLE_RELNULL_ERROR_ENABLE_MASK | \
     DPU_CP_INTENABLE_TXOFF_ERROR_ENABLE_MASK    | \
     DPU_CP_INTENABLE_MICLEN_ERROR_ENABLE_MASK   | \
     DPU_CP_INTENABLE_CPRQ_RX_ERROR_ENABLE_MASK  | \
     DPU_CP_INTENABLE_CPRD_RX_ERROR_ENABLE_MASK  | \
     DPU_CP_INTENABLE_RELPDU_ERROR_ENABLE_MASK   | \
     DPU_CP_INTENABLE_CPRD_TX_ERROR_ENABLE_MASK  | \
     DPU_CP_INTENABLE_MAX_SIZE_ERROR_ENABLE_MASK      | \
     DPU_CP_INTENABLE_END_MARKER_ERROR_ENABLE_MASK    | \
     DPU_CP_INTENABLE_PAYLOAD_LEN_ERROR_ENABLE_MASK   | \
     DPU_CP_INTENABLE_PDU_CNT_ERROR_ENABLE_MASK       | \
     DPU_CP_INTENABLE_WDMA_CP_HANG_ENABLE_MASK        | \
     DPU_CP_INTENABLE_RDMA_CP_HANG_ENABLE_MASK)

/*  TO set int warning counter,
    Log every 4k occurance
*/
#define SET_WARNING_COUNTER(counter, warningType, intRegStatus, errWqCnt)\
        counter++; \
        if(!(counter & 0xfff))\
            HALLOGE( halLog(pMac, LOGE, FL("%s Int Received %d times, intRegStatus = %d, errWqCnt= %d !!\n"), \
                warningType, counter, intRegStatus, errWqCnt));



/* --------------------------------------------------------------------------
 * local funcs
 */
static void halDpu_FreePendingErrorPackets(tpAniSirGlobal pMac);
/* -------------------------------------------------------------
 * FUNCTION:  dpu_hw_init()
 *
 * NOTE:
 *   Initialize base address of the DPU data structure.
 *   Initialize base address of the key data structure.
 *   Initialize base address of the TKIP MIC key data structure.
 *   Initialize base address of the replay counter structure.
 * -------------------------------------------------------------
 */
static void
dpu_hw_init(
    tpAniSirGlobal pMac)
{
    tANI_U32   value = 0;


    /*  WEP Clock gating is disabled */
    /* TX/Rx Tag Check Enabled */
   
    halWriteRegister(pMac, QWLAN_DPU_DPU_CONTROL_REG,
         (( QWLAN_DPU_DPU_CONTROL_WEP_CLKGATE_DISABLE_MASK |
         QWLAN_DPU_DPU_CONTROL_TX_TAG_CHK_EN_MASK | 
         QWLAN_DPU_DPU_CONTROL_RX_TAG_CHK_EN_MASK)));

    /* DPU Soft Reset: This is to FIX DPU hang Issue*/    
    halWriteRegister(pMac, QWLAN_MCU_SOFT_RESET_REG, (QWLAN_MCU_SOFT_RESET_DPU_SOFT_RESET_MASK));
    halWriteRegister(pMac, QWLAN_MCU_SOFT_RESET_REG, (QWLAN_MCU_SOFT_RESET_DPU_SOFT_RESET_MASK
                                                & ~QWLAN_MCU_SOFT_RESET_DPU_SOFT_RESET_MASK));

    /* Init DPU Descriptor data structure */
    halZeroDeviceMemory(pMac, pMac->hal.memMap.dpuDescriptor_offset, pMac->hal.memMap.dpuDescriptor_size);
    halWriteRegister(pMac, QWLAN_DPU_DPU_BASE_ADDR_REG, pMac->hal.memMap.dpuDescriptor_offset);

    /* Init Key Descriptor data structure */
    halZeroDeviceMemory(pMac, pMac->hal.memMap.keyDescriptor_offset, pMac->hal.memMap.keyDescriptor_size);
    halWriteRegister(pMac, QWLAN_DPU_DPU_KEYBASE_ADDR_REG, pMac->hal.memMap.keyDescriptor_offset);

    /* Init TKIP MIC Key data structure */
    halZeroDeviceMemory(pMac, pMac->hal.memMap.micKey_offset, pMac->hal.memMap.micKey_size);
    halWriteRegister(pMac, QWLAN_DPU_DPU_MICKEYBASE_ADDR_REG, pMac->hal.memMap.micKey_offset);

    /* Init Replay Counter data structure */
    halZeroDeviceMemory(pMac, pMac->hal.memMap.replayCounter_offset, pMac->hal.memMap.replayCounter_size);
    halWriteRegister(pMac, QWLAN_DPU_DPU_RCBASE_ADDR_REG, pMac->hal.memMap.replayCounter_offset);

    value = (BMUWQ_SINK << QWLAN_DPU_DPU_ERROR_WQ_ERR_WQ_B_OFFSET) |
            (BMUWQ_DPU_ERROR_WQ << QWLAN_DPU_DPU_ERROR_WQ_ERR_WQ_A_OFFSET);

    halWriteRegister(pMac, QWLAN_DPU_DPU_ERROR_WQ_REG, value);

    //David: For now, route all DPU errors to Error WQ for debugging.
    // later we should set the select register properly to only route
    // softmac interested errors to softmac and leave the rest to junk error wq
    value = 0xffffffff;

    halWriteRegister(pMac, QWLAN_DPU_DPU_ERROR_WQ_SELECT_REG, value);
 
 
#ifdef FIXME_GEN6
    /* enable the use of reservation for DPU tx (but don't do this for rx) */
    halReadRegister(pMac, QWLAN_DPU_DPU_CONTROL_REG, &value);

    value |= QWLAN_DPU_DPU_CONTROL_ROUTING_FLAG_EN_MASK | QWLAN_DPU_DPU_CONTROL_WQ_TX_RSV_EN_MASK;
    halWriteRegister(pMac, QWLAN_DPU_DPU_CONTROL_REG, value);
    
    halWriteRegister(pMac, QWLAN_DPU_DPU_ROUTING_FLAG_REG, BMUWQ_ADU_UMA_RX);
#endif
#ifdef FIXME_GEN5
    value |= QWLAN_DPU_DPU_CONTROL_WQ_TX_RSV_EN_MASK | QWLAN_DPU_DPU_CONTROL_PASS_ZERO_LEN_MASK;
    halWriteRegister(pMac, QWLAN_DPU_DPU_CONTROL_REG, value);
#endif

    /* The following two registers need to be set correctly. Otherwise it'll cause the MIC failure in 
       WPA-TKIP mode with Ninento-PS2. CR 266792 */

    /* Set Priority bit in dpu_mask_reg */
    halReadRegister(pMac, QWLAN_DPU_DPU_TKIP_MASK_REG, &value);
    value |= QWLAN_DPU_DPU_TKIP_MASK_PRIORITY_MASK_7_MASK;
    halWriteRegister(pMac, QWLAN_DPU_DPU_TKIP_MASK_REG, value);

    /* Set priority bit in dpu_bug_fix */
    halReadRegister(pMac, QWLAN_DPU_DPU_BUG_FIX_REG, &value);
    value |= QWLAN_DPU_DPU_BUG_FIX_TKIP_PRIORITY_SEL_MASK;
    halWriteRegister(pMac, QWLAN_DPU_DPU_BUG_FIX_REG, value);

}



/* ------------------------------------------------------------
 * FUNCTION:  dpu_hw_init_wq()
 *
 * NOTE:
 *   Initialize DPU_WQ_assignment register as following:
 *   - Assign WQ3-WQ4 to DPU by setting the corresponding
 *     assignment_enable bits (bit0 - bit7) to 1.
 *   - Also, set the following:
 *       WQ3:  Assigned to DPU RX (set bit 8 to 0)
 *       WQ4:  Assigned to DPU TX (set bit 9 to 1)
 * -------------------------------------------------------------
 */
static void
dpu_hw_init_wq(
    tpAniSirGlobal pMac)
{
    tANI_U32   value = 0;
    /* Only assign WQ3, 6 for DPU */

    /* Assign WQ3, 6 to DPU */
    value |= (QWLAN_BMU_DPU_WQ_ASSIGNMENT_WQ3_DPU_ASSIGNMENT_ENABLE_MASK |
              QWLAN_BMU_DPU_WQ_ASSIGNMENT_WQ6_DPU_ASSIGNMENT_ENABLE_MASK
    );

    /* Assign WQ6 for DPU TX direction.  */
    value |= QWLAN_BMU_DPU_WQ_ASSIGNMENT_WQ6_DPU_ASSIGNMENT_DIRECTION_MASK;

    /* Assign WQ3 for DPU RX direction */
    value &= ~QWLAN_BMU_DPU_WQ_ASSIGNMENT_WQ3_DPU_ASSIGNMENT_DIRECTION_MASK;

    halWriteRegister(pMac, QWLAN_BMU_DPU_WQ_ASSIGNMENT_REG, value);

    
}


/* --------------------------------------------------
 * FUNCTION:  dpu_hw_init_descriptors()
 *
 * NOTE:
 *   Set "Tx Fragmentation Threshold" value to 0xFFF
 *   in all DPU descriptor.
 * --------------------------------------------------
 */
static void
dpu_hw_init_descriptors(
    tpAniSirGlobal pMac)
{
    tDpuDescriptor    dpu, *pDpu;
    tANI_U32              i, address;
    tANI_U32              maxDpuEntries;

    maxDpuEntries = pMac->hal.memMap.maxBssids + pMac->hal.memMap.maxStations;

    pDpu = (tDpuDescriptor *)&dpu;
    palFillMemory(pMac->hHdd, (void *)pDpu, sizeof(tDpuDescriptor), 0);

    pDpu->txFragThreshold4B = 0;
    for (i=0; i < maxDpuEntries; i++)
    {
        address = pMac->hal.memMap.dpuDescriptor_offset + (i * sizeof(tDpuDescriptor));
        halWriteDeviceMemory(pMac, address, (tANI_U8 *)pDpu,
                                 sizeof(tDpuDescriptor));
    }
}


/* --------------------------------------------------
 * FUNCTION:  dpu_set_descriptor()
 *
 * NOTE:
 *   set the DPU Descriptor into device memory
 * --------------------------------------------------
 */
static eHalStatus
dpu_set_descriptor(
    tpAniSirGlobal      pMac,
    tANI_U8             idx,
    tANI_U32            offset,
    tANI_U32            size,
    tDpuDescriptor *pDpu)
{
    tANI_U32              address;

    address = pMac->hal.memMap.dpuDescriptor_offset + (idx * sizeof(tDpuDescriptor));
    halWriteDeviceMemory(pMac, address, (tANI_U8 *)pDpu + offset,
                             size);
    return eHAL_STATUS_SUCCESS;
}

/* --------------------------------------------------
 * FUNCTION:  dpu_set_descriptor_with_noseq()
 *
 * NOTE:
 *   set the DPU Descriptor into device memory
 * --------------------------------------------------
 */
static eHalStatus
dpu_set_descriptor_with_noseq(
    tpAniSirGlobal      pMac,
    tANI_U8             idx,
    tANI_U32            offset,
    tANI_U32            size,
    tDpuDescriptor *pDpu)
{
    
    size -= (sizeof(tDpuAutoSeqNumRecord) * 
                (MAX_NUM_OF_TIDS/DPU_AUTOSEQ_FIELD_LEN));

    return dpu_set_descriptor( pMac,
        idx, offset, size, pDpu);

}

/* --------------------------------------------------
 * FUNCTION:  dpu_get_descriptor()
 *
 * NOTE:
 *   set the DPU Descriptor into device memory
 * --------------------------------------------------
 */
static eHalStatus
dpu_get_descriptor(
    tpAniSirGlobal      pMac,
    tANI_U8             idx,
    tDpuDescriptor *pDpu)
{
    tANI_U32              address;

    palFillMemory(pMac->hHdd, (void *)pDpu, sizeof(tDpuDescriptor), 0);

    address = pMac->hal.memMap.dpuDescriptor_offset + (idx * sizeof(tDpuDescriptor));
    halReadDeviceMemory(pMac, address, (tANI_U8 *)pDpu,
                             sizeof(tDpuDescriptor));
    return eHAL_STATUS_SUCCESS;
}

/* --------------------------------------------------
 * FUNCTION:  dpu_set_key_descriptor()
 *
 * NOTE:
 *   set the DPU Key Descriptor into device memory
 * --------------------------------------------------
 */
static eHalStatus
dpu_set_key_descriptor(
    tpAniSirGlobal          pMac,
    tANI_U8                 idx,
    tDpuKeyDescriptor  *pKey)
{
    tANI_U32 address;

    address = pMac->hal.memMap.keyDescriptor_offset + (idx * sizeof(tDpuKeyDescriptor));

    halWriteDeviceMemory(pMac, address, (tANI_U8 *)pKey,
                             sizeof(tDpuKeyDescriptor));
    return eHAL_STATUS_SUCCESS;
}
/** -------------------------------------------------------------
\fn halDpu_GetRCDescAddr
\brief  gets RC descriptor address
\param  tpAniSirGlobal pMac
\param  tANI_U8 rcBaseIdx, 
\param  tANI_U8 rcIdx
\return eHalStatus - status
  -------------------------------------------------------------*/
static
tANI_U32 halDpu_GetRCDescAddr(tpAniSirGlobal pMac, tANI_U8 rcBaseIdx, 
                              tANI_U8 rcIdx)
{
    tANI_U32 address;
    address = pMac->hal.memMap.replayCounter_offset +
        (rcBaseIdx * MAX_RC_ENTRIES_PER_SET * sizeof( tDpuReplayCounterDescriptor )) +
        (rcIdx * sizeof( tDpuReplayCounterDescriptor ));
    return address;
}

/* --------------------------------------------------
 * FUNCTION:  dpu_set_rc_descriptor()
 *
 * NOTE:
 *   set the DPU Mic Key into device memory
 * --------------------------------------------------
 */
static eHalStatus
dpu_set_rc_descriptor( tpAniSirGlobal pMac,
    tANI_U8 rcBaseIdx,
    tANI_U8 rcIdx,
    tANI_U8 *rcDesc,
    tANI_U32 len)
{
  tANI_U32 address = halDpu_GetRCDescAddr(pMac, rcBaseIdx, rcIdx);
  halWriteDeviceMemory(pMac, address, rcDesc, len);

  return eHAL_STATUS_SUCCESS;
}


/* --------------------------------------------------
 * FUNCTION:  dpu_set_mic_key_descriptor()
 *
 * NOTE:
 *   set the DPU Mic Key into device memory
 * --------------------------------------------------
 */
static eHalStatus
dpu_set_mic_key_descriptor(
    tpAniSirGlobal            pMac,
    tANI_U8                   idx,
    tANI_U8 *pMic,
    tANI_U32 size)
{
    tANI_U32 address;

    address = pMac->hal.memMap.micKey_offset + (idx * size);

    halWriteDeviceMemory(pMac, address, pMic, size);
    return eHAL_STATUS_SUCCESS;
}

/* --------------------------------------------------
 * FUNCTION:  dpu_init_tables()
 *
 * NOTE:
 * Initialize the DPU descriptor, Key, MIC and RC tables
 * --------------------------------------------------
 */
static void
dpu_init_tables(
    tpAniSirGlobal pMac)
{
    tANI_U8 i;
    tpDpuInfo      pDpu = (tpDpuInfo) pMac->hal.halMac.dpuInfo;

    for(i = 0; i < pDpu->maxEntries; i++)
    {
        /* To make our life easier, let hw use the same idx as the hal copy */
        pDpu->descTable[i].hwIndex      = i;
        pDpu->descTable[i].used         = 0;

        pDpu->keyTable[i].hwIndex       = i;
        pDpu->keyTable[i].used          = 0;

        pDpu->micKeyTable[i].hwIndex    = i;
        pDpu->micKeyTable[i].used       = 0;

        pDpu->rcDescTable[i].hwIndex    = i;
        // RC Base Index can range from 0..15
        pDpu->rcDescTable[i].hwRCBaseIndex = i / MAX_STA_ENTRIES_PER_RC_SET;
        pDpu->rcDescTable[i].used       = 0;
    }
}

/* --------------------------------------------------
 * FUNCTION:  dpu_init_error_wq()
 *
 * NOTE:
 * Initialize the DPU Error WQ and Select Registers to
 * Capture MIC error.
 * --------------------------------------------------
 */

static void dpu_init_error_wq(tpAniSirGlobal pMac)
{
    tANI_U32 value;

    /** Initialize the DPU error WQ A to BMUWQ_DPU_ERROR_WQ.*/
    value = QWLAN_DPU_DPU_ERROR_WQ_ERR_WQ_B_MASK & QWLAN_DPU_DPU_ERROR_WQ_ERR_WQ_B_DEFAULT;
    value |= QWLAN_DPU_DPU_ERROR_WQ_ERR_WQ_A_MASK & BMUWQ_DPU_ERROR_WQ;
     
    halWriteRegister(pMac, QWLAN_DPU_DPU_ERROR_WQ_REG,     value);

    /** Set the DPU Select reg to capture MIC error and send to WQ A -> BMUWQ_DPU_ERROR_WQ.*/
    value = ~(1 << SWBD_DPUERR_BAD_TKIP_MIC); 
    halWriteRegister(pMac, QWLAN_DPU_DPU_ERROR_WQ_SELECT_REG, value);
}

// -----------------------------------------------------------------------------
// API functions
// -----------------------------------------------------------------------------

/* --------------------------
 * FUNCTION:  halDPU_Start()
 * --------------------------
 */
eHalStatus
halDpu_Start(
    tHalHandle   hHal,
    void        *arg)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hHal;
    tANI_U8        dpuIdx = 0;
    (void) arg;

    dpu_hw_init(pMac);

    //David: from 190D, DPU no longer supports cp_disable, sp_disable in
    //       control register. So remove this code
    //if (dpu_hw_set_bypass_mode(pMac) != eHAL_STATUS_SUCCESS)
    //    return eHAL_STATUS_FAILURE;

    dpu_hw_init_wq(pMac);

    dpu_hw_init_descriptors(pMac);

    dpu_init_tables(pMac);

    dpu_init_error_wq(pMac) ;

//    if (dpu_alloc_mgmt_dpu_desc(pMac) != eHAL_STATUS_SUCCESS)
//        return eHAL_STATUS_FAILURE;

    // From DPU pass it on to ADU which is WQ 24.
    // Let UMA get the frames on the Rx too.
    halDpu_BdRoutingFlagOverride(pMac, 1, BMUWQ_ADU_UMA_RX);

    // Allocate DPU descriptor for self STA entry
    if(eHAL_STATUS_SUCCESS != halDpu_AllocId ( pMac, &dpuIdx )){
        HALLOGE( halLog( pMac, LOGE, FL("Unable to Allocate DPU index\n")));
        return eHAL_STATUS_FAILURE;
    }
    //dpu index for self entry is expected to be HAL_DPU_SELF_STA_DEFAULT_IDX only. and this entry is
    //reserved for self station.
    if(dpuIdx != HAL_DPU_SELF_STA_DEFAULT_IDX)
    {
        HALLOGE( halLog(pMac, LOGE, FL("dpuIdx for self entry is %u (expected to be %u)\n"), dpuIdx, HAL_DPU_SELF_STA_DEFAULT_IDX));
        return eHAL_STATUS_FAILURE;
    }


    return eHAL_STATUS_SUCCESS;
}

/* ------------------------------------------------------------
 * FUNCTION:  halDpu_Open
 *
 * NOTE:
 *   Initialize the DPU descriptor, Key, MicKey and Replay Counter tables.
 *   Allocate the memory
 *
 * ------------------------------------------------------------
 */
eHalStatus
halDpu_Open(
    tHalHandle   hHal,
    void        *arg)
{
    eHalStatus     status;
    tANI_U8        max_entries;
    tpAniSirGlobal pMac = (tpAniSirGlobal)hHal;
    tpDpuInfo      pDpu = NULL;

    (void) arg;

    max_entries  = (tANI_U8) ( pMac->hal.memMap.maxStations +
        pMac->hal.memMap.maxBssids );
    if (max_entries >= DPU_MAX_DESCRIPTOR_ENTRIES)
    {
        HALLOGE( halLog(pMac, LOGE, FL("Exceeded max entries\n")));
        return eHAL_STATUS_FAILURE;
    }
    pMac->hal.halMac.dpuInfo = NULL;
    status = palAllocateMemory(pMac->hHdd, &pMac->hal.halMac.dpuInfo,
                               sizeof(tDpuInfo));
    if (status != eHAL_STATUS_SUCCESS){
        goto out;
    }
    palZeroMemory( pMac->hHdd, (void *) pMac->hal.halMac.dpuInfo, sizeof(tDpuInfo) );


    pDpu = (tpDpuInfo) pMac->hal.halMac.dpuInfo;
    pDpu->descTable = NULL;
    // Allocate the memory for these tables
    status = palAllocateMemory(pMac->hHdd, (void **) & pDpu->descTable,
                               max_entries * sizeof(tHalDpuDescEntry));
    if (status != eHAL_STATUS_SUCCESS){
        goto out;
    }
    palZeroMemory( pMac->hHdd, (void *) pDpu->descTable,
                   max_entries * sizeof( tHalDpuDescEntry ));
    pDpu->keyTable = NULL;
    status = palAllocateMemory(pMac->hHdd, (void **) & pDpu->keyTable,
                               max_entries * sizeof(tHalDpuKeyEntry));
    if (status != eHAL_STATUS_SUCCESS){
        goto out;
    }
    palZeroMemory( pMac->hHdd, (void *) pDpu->keyTable,
                   max_entries * sizeof( tHalDpuKeyEntry ));
    pDpu->micKeyTable = NULL;
    status = palAllocateMemory(pMac->hHdd, (void **) & pDpu->micKeyTable,
                               max_entries * sizeof(tHalDpuMicKeyEntry));
    if (status != eHAL_STATUS_SUCCESS){
        goto out;
    }
    palZeroMemory( pMac->hHdd, (void *) pDpu->micKeyTable,
                   max_entries * sizeof( tHalDpuMicKeyEntry ));
    pDpu->rcDescTable = NULL;
    status = palAllocateMemory(pMac->hHdd, (void **) & pDpu->rcDescTable,
                               max_entries * sizeof(tHalDpuRCEntry));
    if (status != eHAL_STATUS_SUCCESS)
        goto out;
    palZeroMemory( pMac->hHdd, (void *) pDpu->rcDescTable,
                   max_entries * sizeof( tHalDpuRCEntry ));

    pDpu->maxEntries = max_entries;
    HALLOGW( halLog( pMac, LOGW, FL("Allocate DPU tables, size %u\n"),   max_entries ));
    return eHAL_STATUS_SUCCESS;
out:    
    HALLOGE( halLog(pMac, LOGE, FL("DPU open failed \n")));
    if(pMac->hal.halMac.dpuInfo){
        if(pDpu->rcDescTable)
            palFreeMemory(pMac->hHdd,pDpu->rcDescTable);
        if(pDpu->micKeyTable)
            palFreeMemory(pMac->hHdd,pDpu->micKeyTable);
        if(pDpu->keyTable)
            palFreeMemory(pMac->hHdd,pDpu->keyTable);
        if(pDpu->descTable)
            palFreeMemory(pMac->hHdd,pDpu->descTable);
        palFreeMemory(pMac->hHdd,pMac->hal.halMac.dpuInfo);

    }    
    return status;    
}

/* ------------------------------------------------------------
 * FUNCTION:  halDpu_Stop
 *
 * NOTE:
 *   Zero out the DPU descriptor, Key, MIC and RC tables
 * ------------------------------------------------------------
 */
eHalStatus
halDpu_Stop(
    tHalHandle   hHal,
    void        *arg)
{
    tANI_U8        max_entries;
    tpAniSirGlobal pMac = (tpAniSirGlobal)hHal;
    tpDpuInfo      pDpu = (tpDpuInfo) pMac->hal.halMac.dpuInfo;

    (void) arg;

    if (pDpu == NULL)
        return eHAL_STATUS_SUCCESS;

    max_entries  = (tANI_U8) ( pMac->hal.memMap.maxStations +
        pMac->hal.memMap.maxBssids );
    if (max_entries >= DPU_MAX_DESCRIPTOR_ENTRIES)
    {
        HALLOGE( halLog(pMac, LOGE, FL("Exceeded max entries\n")));
        return eHAL_STATUS_FAILURE;
    }

    if (pDpu->descTable != NULL)
        palZeroMemory( pMac->hHdd, (void *) pDpu->descTable,
                       max_entries * sizeof( tHalDpuDescEntry ));

    if (pDpu->keyTable != NULL)
        palZeroMemory( pMac->hHdd, (void *) pDpu->keyTable,
                       max_entries * sizeof( tHalDpuKeyEntry ));

    if (pDpu->micKeyTable != NULL)
        palZeroMemory( pMac->hHdd, (void *) pDpu->micKeyTable,
                       max_entries * sizeof( tHalDpuMicKeyEntry ));

    if (pDpu->rcDescTable != NULL)
        palZeroMemory( pMac->hHdd, (void *) pDpu->rcDescTable,
                       max_entries * sizeof( tHalDpuRCEntry ));

    pDpu->maxEntries = max_entries;

    return eHAL_STATUS_SUCCESS;
}


/* ------------------------------------------------------------
 * FUNCTION:  halDpu_Close
 *
 * NOTE:
 *   Close the DPU descriptor, Key, MIC and RC tables
 *
 * ------------------------------------------------------------
 */
eHalStatus
halDpu_Close(
    tHalHandle   hHal,
    void        *arg)
{
    eHalStatus status;
    tpAniSirGlobal pMac = (tpAniSirGlobal)hHal;
    tpDpuInfo      pDpu = (tpDpuInfo) pMac->hal.halMac.dpuInfo;

    (void) arg;

    if (pDpu == NULL)
        return eHAL_STATUS_SUCCESS;

    if (pDpu->descTable != NULL)
        status = palFreeMemory(pMac->hHdd, pDpu->descTable);

    if (pDpu->keyTable != NULL)
        status = palFreeMemory(pMac->hHdd, pDpu->keyTable);

    if (pDpu->micKeyTable != NULL)
        status = palFreeMemory(pMac->hHdd, pDpu->micKeyTable);

    if (pDpu->rcDescTable != NULL)
        status = palFreeMemory(pMac->hHdd, pDpu->rcDescTable);

    status = palFreeMemory(pMac->hHdd, pDpu);
    pMac->hal.halMac.dpuInfo = NULL;

    return status;
}

/* ------------------------------------------------------------
 * FUNCTION:  halDpu_AllocId
 *
 * NOTE:
 *   Allocate a new DPU descriptor index.
 *
 * ------------------------------------------------------------
 */
eHalStatus
halDpu_AllocId(
    tpAniSirGlobal  pMac,
    tANI_U8         *id)
{
    tANI_U8             i;
    tANI_U8             found = 0;
    tHalDpuDescEntry   *pDpuDesc;
    tpDpuInfo pDpu = (tpDpuInfo) pMac->hal.halMac.dpuInfo;

    for (i=0; i < pDpu->maxEntries; i++ )
    {
        if(pDpu->descTable[i].used == 0)
        {
            found = 1;
            break;
        }
    }

    if(found)
    {
        HALLOG1( halLog(pMac, LOG1, FL("Got DPU descriptor index %d \n"), i));
        *id = i;
        pDpu->descTable[i].used = 1;
    }
    else
    {
        HALLOGE( halLog(pMac, LOGE, FL("DPU descriptor table full")));
        return eHAL_STATUS_DPU_DESCRIPTOR_TABLE_FULL;
    }

    pDpuDesc = & pDpu->descTable[i];
    /* Initial the encType to None */
    pDpuDesc->wepType   = eSIR_WEP_DYNAMIC;
    pDpuDesc->keyIdx    = HAL_INVALID_KEYID_INDEX;
    pDpuDesc->micKeyIdx = HAL_INVALID_KEYID_INDEX;
    pDpuDesc->rcIdx     = HAL_INVALID_KEYID_INDEX;
    pDpuDesc->halDpuDescriptor.encryptMode = eSIR_ED_NONE;
    pDpuDesc->halDpuDescriptor.txFragThreshold4B = 0;

    return eHAL_STATUS_SUCCESS;
}

/* ------------------------------------------------------------
 * FUNCTION:  halTable_SetDpuKeyId
 *
 * NOTE:
 *   configure a keyIdx into DPU descriptor
 *
 * ------------------------------------------------------------
 */
eHalStatus
halDpu_SetWepKeys(
    tpAniSirGlobal  pMac,
    tANI_U8         dpuId,
    tAniEdType      encType,
    tANI_U8         defWepId,
    tANI_U8         keyId0,
    tANI_U8         keyId1,
    tANI_U8         keyId2,
    tANI_U8         keyId3)
{
    eHalStatus status ;
    tANI_U8 rcIdx, tid;
    tHalDpuDescEntry  *pDpuDesc;
    tpDpuInfo pDpu = (tpDpuInfo) pMac->hal.halMac.dpuInfo;

    if(dpuId >= pDpu->maxEntries)
    {
        HALLOGE( halLog(pMac, LOGE, FL("Got invalid index %d \n"), dpuId));
        return eHAL_STATUS_INVALID_PARAMETER;
    }

    if(defWepId >= HAL_MAX_NUM_WEP_KEYID)  return eHAL_STATUS_INVALID_KEYID;

    // Get the DPU Descriptor wrt this DPU Index
    pDpuDesc = &pDpu->descTable[dpuId];

    // Update the DPU Descriptor wrt the Rx Decrypt WEP Keys
    if( keyId0 < HAL_MAX_NUM_WEP_KEYID )
    {
      if( pDpu->keyTable[keyId0].used != 0 )
        pDpuDesc->halDpuDescriptor.keyIndex0 = pDpu->keyTable[keyId0].hwIndex;
    }
    if( keyId1 < HAL_MAX_NUM_WEP_KEYID )
    {
      if( pDpu->keyTable[keyId1].used != 0 )
        pDpuDesc->halDpuDescriptor.keyIndex1 = pDpu->keyTable[keyId1].hwIndex;
    }
    if( keyId2 < HAL_MAX_NUM_WEP_KEYID )
    {
      if( pDpu->keyTable[keyId2].used != 0 )
        pDpuDesc->halDpuDescriptor.keyIndex2 = pDpu->keyTable[keyId2].hwIndex;
    }
    if( keyId3 < HAL_MAX_NUM_WEP_KEYID )
    {
      if( pDpu->keyTable[keyId3].used != 0 )
        pDpuDesc->halDpuDescriptor.keyIndex3 = pDpu->keyTable[keyId3].hwIndex;
    }

    /* We need to release the old keys if there is any */
    //
    // TODO - Need to understand why?
#if 0
    if(pDpuDesc->wepType == eSIR_WEP_DYNAMIC)
    {
        if(pDpuDesc->keyIdx != HAL_INVALID_KEYID_INDEX)
            halDpu_ReleaseKeyId(pMac, pDpuDesc->keyIdx);
        if(pDpuDesc->rcIdx != HAL_INVALID_KEYID_INDEX)
            halDpu_ReleaseRCId(pMac, pDpuDesc->rcIdx);
        if(pDpuDesc->micKeyIdx != HAL_INVALID_KEYID_INDEX)
            halDpu_ReleaseMicKeyId(pMac, pDpuDesc->micKeyIdx);

        pDpuDesc->keyIdx    = HAL_INVALID_KEYID_INDEX;
        pDpuDesc->micKeyIdx = HAL_INVALID_KEYID_INDEX;
        pDpuDesc->rcIdx     = HAL_INVALID_KEYID_INDEX;
    }
#endif

    // FIXME - Why is this??
    pDpuDesc->wepType = eSIR_WEP_STATIC;

    // Update the Tx Key index to one of the four WEP Keys
    switch( defWepId )
    {
        case 0: pDpuDesc->keyIdx = keyId0; break;
        case 1: pDpuDesc->keyIdx = keyId1; break;
        case 2: pDpuDesc->keyIdx = keyId2; break;
        case 3: pDpuDesc->keyIdx = keyId3; break;
        default: break;
    }

    // set the default transmit key Id for this sta
    pDpuDesc->halDpuDescriptor.txKeyId = defWepId;


#if 0
    if ( pDpuDesc->halDpuDescriptor.txKeyId == HAL_INVALID_KEYID_INDEX)
    {
        HALLOGE( halLog(pMac, LOGE, FL("The default WEP key is not configured properly for Dpu %d, WEP (%d) \n"),
               dpuId, defWepId);
        return eHAL_STATUS_INVALID_KEYID;
    }
#endif

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
          dpuId,
          &rcIdx ))
    {
      status = halDpu_AllocRCId( pMac, encType, &rcIdx );
      if( eHAL_STATUS_SUCCESS != status )
        goto failed;
    }

    {
    tANI_U8 winChkSize[MAX_NUM_OF_TIDS];

      palZeroMemory( pMac->hHdd, winChkSize, sizeof( winChkSize ));
      status = halDpu_SetRCDescriptor( pMac,
          rcIdx,
          HAL_DPU_DEFAULT_RCE_OFF, // RCE
          HAL_DPU_DEFAULT_WCE_OFF, // WCE
          winChkSize );
      if(status != eHAL_STATUS_SUCCESS)
          goto failed;
    }

    // configure the actual descriptor
    pDpuDesc->halDpuDescriptor.encryptMode = (tANI_U8) encType;

    // Do we require this in Gen6, now that we have different entries for keyidx?
//    pDpuDesc->halDpuDescriptor.keyIndex = pDpu->keyTable[defWepId].hwIndex;

    pDpuDesc->halDpuDescriptor.txKeyId = defWepId;
    pDpuDesc->rcIdx = rcIdx;

    pDpuDesc->halDpuDescriptor.replayCountSet =
      (tANI_U32) (pDpu->rcDescTable[rcIdx].hwRCBaseIndex & 0xf);

    for( tid = 0; tid < MAX_NUM_OF_TIDS; tid++ )
    {
      // For RC Id0..Id15 indices
      HAL_SET_DPU_RCIDX( &pDpuDesc->halDpuDescriptor,
          (tANI_U8) tid,
          (tANI_U8) (pDpu->rcDescTable[rcIdx].hwIndex + tid));
    }

#if 0
    /* Update the signature, so any frames remaining in the queue
     * will be dropped by DPU
     */
    pDpuDesc->halDpuDescriptor.signature ++ ;
#endif
    HALLOGW( halLog(pMac, LOGW, FL("Configure WEP Key for Dpu %d\n"),
           dpuId ));


    status = dpu_set_descriptor_with_noseq(pMac,
        pDpuDesc->hwIndex,
        0,
        sizeof(tDpuDescriptor),
        &pDpuDesc->halDpuDescriptor );

failed:

    return status;
}

/* ------------------------------------------------------------
 * FUNCTION:  halDpu_GetStatus
 *
 * NOTE:
 *   get stats
 *
 * ------------------------------------------------------------
 */
eHalStatus
halDpu_GetStatus(
    tpAniSirGlobal      pMac,
    tANI_U8             dpuIdx,
    tpDpuStatsParams    pDpuStatus )
{
    eHalStatus status;
    tDpuDescriptor  msg, *pMsg;

    pMsg = (tDpuDescriptor *)&msg;

    status = dpu_get_descriptor(pMac, dpuIdx, pMsg);
    if(status != eHAL_STATUS_SUCCESS)
        return status;
 
    pDpuStatus->encMode             = (tANI_U8)pMsg->encryptMode;
    pDpuStatus->sendBlocks          = pMsg->txSentBlocks;
    pDpuStatus->recvBlocks          = pMsg->rxRcvddBlocks;
    pDpuStatus->replays             = pMsg->replayCountSet;
    pDpuStatus->micErrorCnt         = (tANI_U8) pMsg->micErrCount;
    pDpuStatus->protExclCnt         = pMsg->excludedCount;
    pDpuStatus->formatErrCnt        = (tANI_U16) pMsg->formatErrorCount;
    pDpuStatus->unDecryptableCnt    = (tANI_U16) pMsg->undecryptableCount;
    pDpuStatus->decryptErrCnt       = pMsg->decryptErrorCount;
    pDpuStatus->decryptOkCnt        = pMsg->decryptSuccessCount;

    return status;

}


/* ------------------------------------------------------------
 * FUNCTION:  halTable_SetDpuDesciptor
 * NOTE:
 *   configure the dpu descriptor
 *
 * ------------------------------------------------------------
 */
eHalStatus
halDpu_SetDescriptorAttributes(
    tpAniSirGlobal  pMac,
    tANI_U8         dpuIdx,
    tAniEdType      encType,
    tANI_U8         keyIdx,
    tANI_U8         derivedKeyIdx,
    tANI_U8         micKeyIdx,
    tANI_U8         rcIdx,
    tANI_U8         singleTidRc,
    tANI_U8         defKeyId)
{
    tHalDpuDescEntry   *pDpuDesc;
    int                 tid;
    tANI_U8 rcIdxBase;
    tpDpuInfo pDpu = (tpDpuInfo) pMac->hal.halMac.dpuInfo;

    if(dpuIdx >= pDpu->maxEntries)
    {
        HALLOGW( halLog(pMac, LOGW, FL("Got invalid index %d \n"), dpuIdx));
        return eHAL_STATUS_INVALID_PARAMETER;
    }

    pDpuDesc = & pDpu->descTable[dpuIdx];

    if(pDpuDesc->used == 0)
        return eHAL_STATUS_INVALID_KEYID;


    /* We need to release the old keys if there is any */
#if 0
    if(pDpuDesc->wepType == eSIR_WEP_DYNAMIC)
    {
        if(pDpuDesc->keyIdx != HAL_INVALID_KEYID_INDEX)
            halDpu_ReleaseKeyId(pMac, pDpuDesc->keyIdx);
        if(pDpuDesc->rcIdx != HAL_INVALID_KEYID_INDEX)
            halDpu_ReleaseRCId(pMac, pDpuDesc->rcIdx);
        if(pDpuDesc->micKeyIdx != HAL_INVALID_KEYID_INDEX)
            halDpu_ReleaseMicKeyId(pMac, pDpuDesc->micKeyIdx);
        pDpuDesc->keyIdx    = HAL_INVALID_KEYID_INDEX;
        pDpuDesc->micKeyIdx = HAL_INVALID_KEYID_INDEX;
        pDpuDesc->rcIdx     = HAL_INVALID_KEYID_INDEX;
    }
#endif


    // Hack: Any per sta key will use eSIR_WEP_DYNAMIC
    pDpuDesc->wepType   = eSIR_WEP_DYNAMIC;

    pDpuDesc->keyIdx    = keyIdx;
    pDpuDesc->derivedKeyIdx = derivedKeyIdx;
    pDpuDesc->micKeyIdx = micKeyIdx;
    pDpuDesc->rcIdx     = rcIdx;

    // configure the actual descriptor
    pDpuDesc->halDpuDescriptor.encryptMode  = (tANI_U8) encType;
    pDpuDesc->halDpuDescriptor.singleTidRc  = singleTidRc;

    if(keyIdx != HAL_INVALID_KEYID_INDEX)
    {
        if(defKeyId == 0) {
            pDpuDesc->halDpuDescriptor.keyIndex0 = pDpu->keyTable[keyIdx].hwIndex;
        } else if(defKeyId == 1) {
            pDpuDesc->halDpuDescriptor.keyIndex1 = pDpu->keyTable[keyIdx].hwIndex;
        } else if(defKeyId == 2) {
            pDpuDesc->halDpuDescriptor.keyIndex2 = pDpu->keyTable[keyIdx].hwIndex;
        } else if(defKeyId == 3){
            pDpuDesc->halDpuDescriptor.keyIndex3 = pDpu->keyTable[keyIdx].hwIndex;
        } else {
            pDpuDesc->halDpuDescriptor.keyIndex0 = pDpu->keyTable[keyIdx].hwIndex;
            HALLOGW( halLog(pMac, LOGW, FL("Got invalid KeyIndex %d \n"), defKeyId));           
        }
    }
    else
    {
        pDpuDesc->halDpuDescriptor.keyIndex0 = 0;
    }

    // For AES-128-CMAC for broadcast/multicast Management frames, set the derived 
    // key index along with the default key index
    if(encType == eSIR_ED_AES_128_CMAC) 
    {
        if(defKeyId == HAL_DPU_KEY_ID_4) 
        {
            if(derivedKeyIdx != HAL_INVALID_KEYID_INDEX) 
            {
                pDpuDesc->halDpuDescriptor.keyIndex4d =
                    pDpu->keyTable[derivedKeyIdx].hwIndex;
            } 
            else 
            {
                pDpuDesc->halDpuDescriptor.keyIndex4d = 0;
            }
        } 
        else if(defKeyId == HAL_DPU_KEY_ID_5) 
        {
            if(derivedKeyIdx != HAL_INVALID_KEYID_INDEX) 
            {
                pDpuDesc->halDpuDescriptor.keyIndex5d = 
                    pDpu->keyTable[derivedKeyIdx].hwIndex;
            } 
            else 
            {
                pDpuDesc->halDpuDescriptor.keyIndex5d = 0;
            }
        }
    }
    else if(encType == eSIR_ED_WEP40 || encType == eSIR_ED_WEP104)
    {
        /* For dynamic WEP, all the 4 rx keys will be the same */
        pDpuDesc->halDpuDescriptor.keyIndex0 = pDpuDesc->halDpuDescriptor.keyIndex0;
        pDpuDesc->halDpuDescriptor.keyIndex1 = pDpuDesc->halDpuDescriptor.keyIndex0;
        pDpuDesc->halDpuDescriptor.keyIndex2 = pDpuDesc->halDpuDescriptor.keyIndex0;
        pDpuDesc->halDpuDescriptor.keyIndex3 = pDpuDesc->halDpuDescriptor.keyIndex0;
    }
#if defined(LIBRA_WAPI_SUPPORT)
    else if(eSIR_ED_WPI == encType)
    {
        //For Libra supprt WAPI, the DPU desc's encMode is no-encryption
        //And set a custom bit to indication whether it is for a WAPI station
        pDpuDesc->halDpuDescriptor.encryptMode = eSIR_ED_NONE;
        pDpuDesc->halDpuDescriptor.wapi = 1;
	    if(defKeyId == 0)
		    pDpuDesc->halDpuDescriptor.keyIndex0 = keyIdx;
        else if(defKeyId == 1)
		    pDpuDesc->halDpuDescriptor.keyIndex1 = keyIdx;
    }
    else
    {
        pDpuDesc->halDpuDescriptor.wapi = 0;
    }
#endif

    if(rcIdx != HAL_INVALID_KEYID_INDEX)
    {
        /*
         * The replay counter index will be beyond the 8 bits space in the Dpu
         * descriptor(256(sta) * 16(tc)). The DPU will use two seperate fields to
         * address RC index. replayCountSet(4 bits) is the base; there are 16
         * offset field to represent 16 tcs.
         */
    
        pDpuDesc->halDpuDescriptor.replayCountSet =
          (tANI_U32) (pDpu->rcDescTable[rcIdx].hwRCBaseIndex & 0xf);

        rcIdxBase = (tANI_U8)pDpu->rcDescTable[rcIdx].hwIndex;
    }
    else
    {
        pDpuDesc->halDpuDescriptor.replayCountSet = 0;
        rcIdxBase = 0;
    }

    pDpuDesc->halDpuDescriptor.txKeyId = defKeyId;

    for( tid = 0; tid < MAX_NUM_OF_TIDS; tid++ )
    {
      // For RC Id0..Id15 indices
      HAL_SET_DPU_RCIDX( &pDpuDesc->halDpuDescriptor,
          (tANI_U8) tid,
          (tANI_U8) (rcIdxBase + tid));
    }
    /* Retrive the fragmentation threshold value from DPU and Use*/
    pDpuDesc->halDpuDescriptor.txFragThreshold4B = 
	    pDpu->descTable[dpuIdx].halDpuDescriptor.txFragThreshold4B;
#if 0
    /* Update the signature, so any frames remaining in the queue
     * will be dropped by DPU
     */
    pDpuDesc->halDpuDescriptor.signature ++ ;
#endif

    HALLOGW( halLog( pMac, LOGW,
        FL("Configuring the DPU Descriptor per STA Key (%d) for Dpu %d \n"
        "txFragThreshold4B: %d, signature %d \n"
        "Key Index 0: %d, Key Index 1: %d, Key Index 2: %d, Key Index 3: %d\n"
        "Key Index 4: %d, Key Index 5: %d, Key Index 4d: %d, Key Index 5d: %d\n"
        "Key Base: %d, replayCountSet: %d, Single TID RC: %d\n"
        "xKeyId: %d, encryptMode: %d, idxPerTidReplayCount: %d \n"),
        keyIdx, dpuIdx,
        pDpuDesc->halDpuDescriptor.txFragThreshold4B,
        pDpuDesc->halDpuDescriptor.signature,
        pDpuDesc->halDpuDescriptor.keyIndex0, pDpuDesc->halDpuDescriptor.keyIndex1,
        pDpuDesc->halDpuDescriptor.keyIndex2, pDpuDesc->halDpuDescriptor.keyIndex3,
        pDpuDesc->halDpuDescriptor.keyIndex4, pDpuDesc->halDpuDescriptor.keyIndex5,
        pDpuDesc->halDpuDescriptor.keyIndex4d, pDpuDesc->halDpuDescriptor.keyIndex5d,
        pDpuDesc->halDpuDescriptor.keyBase, pDpuDesc->halDpuDescriptor.replayCountSet,
        pDpuDesc->halDpuDescriptor.singleTidRc, pDpuDesc->halDpuDescriptor.txKeyId,
        (pDpuDesc->halDpuDescriptor.encryptMode & 0x7),
        pDpuDesc->halDpuDescriptor.idxPerTidReplayCount ));

    return (dpu_set_descriptor_with_noseq(pMac, pDpuDesc->hwIndex, 0,
                               sizeof(tDpuDescriptor), &pDpuDesc->halDpuDescriptor));

}


/* ------------------------------------------------
 * FUNCTION:  halTable_setFragThreshold()
 *
 * NOTE:
 *   Set DPU descriptor's fragment threshold value
 *   for the given station.
 * ------------------------------------------------
 */
eHalStatus
halDpu_SetFragThreshold(
    tpAniSirGlobal  pMac,
    tANI_U8         dpuIdx,
    tANI_U16        fragSize)
{
    tpDpuInfo pDpu = (tpDpuInfo) pMac->hal.halMac.dpuInfo;
    tDpuDescriptor dpu;

    if(dpuIdx >= pDpu->maxEntries)
    {
        HALLOGW( halLog(pMac, LOGW, FL("Got invalid index %d \n"), dpuIdx));
        return eHAL_STATUS_INVALID_PARAMETER;
    }

    HALLOG1( halLog(pMac, LOG1, FL("round DPU [%d] fragment threshold %d byte to %d DWORD \n"),
           dpuIdx, fragSize, fragSize>>2));

    // The Fragmentation Threshold should account for the MPDU header
    switch( pDpu->descTable[dpuIdx].halDpuDescriptor.encryptMode )
    {
      case eSIR_ED_WEP40:
      case eSIR_ED_WEP104:
        if(fragSize >= 36)
            fragSize -= 36;
        break;

      case eSIR_ED_CCMP:
        if(fragSize >= 44)
            fragSize -= 44;
        break;

      case eSIR_ED_TKIP:
        if(fragSize >= 48)
            fragSize -= 48;
        break;

      case eSIR_ED_NONE:
      default:
        if(fragSize >= 28)
            fragSize -= 28;
        break;
    }

    pDpu->descTable[dpuIdx].halDpuDescriptor.txFragThreshold4B = fragSize>>2;

    //
    // Update the DPU descriptor with respect to the
    // "modified" parameter only and not the remainder
    // of the DPU descriptor fields
    //
    if( eHAL_STATUS_SUCCESS == dpu_get_descriptor( pMac,
          dpuIdx,
          &dpu ))
    {
      // Update the Fragmentation Threshold
      dpu.txFragThreshold4B = fragSize >> 2;

      // Update the DPU Descriptor in memory
      return dpu_set_descriptor( pMac,
          pDpu->descTable[dpuIdx].hwIndex,
          0,
          4,    // Just update the first WORD of the DPU descriptor where the
                // fragmentation threshold field resides.
          &dpu );
    }
    else
      return eHAL_STATUS_FAILURE;

}

/* ------------------------------------------------------------
 * FUNCTION:  halDpu_GetSignature
 *
 * NOTE:
 *    Get the signature stored in DPU table
 *
 * ------------------------------------------------------------
 */
eHalStatus
halDpu_GetSignature(
    tpAniSirGlobal  pMac,
    tANI_U8         dpuId,
    tANI_U8        *sig )
{
    tpDpuInfo pDpu = (tpDpuInfo) pMac->hal.halMac.dpuInfo;
    if(dpuId >= pDpu->maxEntries)
    {
        HALLOGW( halLog(pMac, LOGW, FL("Got invalid index %d \n"), dpuId));
        return eHAL_STATUS_INVALID_PARAMETER;
    }

    if(pDpu->descTable[dpuId].used == 0)
        return eHAL_STATUS_INVALID_KEYID;

    *sig = (tANI_U8)pDpu->descTable[dpuId].halDpuDescriptor.signature;

    return eHAL_STATUS_SUCCESS;


}

/* ------------------------------------------------------------
 * FUNCTION:  halDpu_ReleaseId
 *
 * NOTE:
 *    Release the Dpu ID
 *
 * ------------------------------------------------------------
 */
eHalStatus
halDpu_ReleaseId(
    tpAniSirGlobal  pMac,
    tANI_U8         id)
{
    tpDpuInfo pDpu = (tpDpuInfo) pMac->hal.halMac.dpuInfo;

    if (id >= pDpu->maxEntries)
        return eHAL_STATUS_INVALID_PARAMETER;

    if (pDpu->descTable[id].used == 0)
        return eHAL_STATUS_SUCCESS;

    /* Update the signature, so any frames remaining in the queue
     * will be dropped by DPU
     */
    pDpu->descTable[id].halDpuDescriptor.signature ++ ;

    pDpu->descTable[id].used = 0;
    
    return (dpu_set_descriptor(pMac, pDpu->descTable[id].hwIndex,
                               0,
                               4,    // Just update the first WORD of the DPU descriptor where the
                                    // signature field resides. 
                               &pDpu->descTable[id].halDpuDescriptor));
}

/* ------------------------------------------------------------
 * FUNCTION:  halDpu_ReleaseDescriptor
 *
 * NOTE:
 *    Release the Dpu entry and any other descriptors whenever possible
 *
 * ------------------------------------------------------------
 */
eHalStatus
halDpu_ReleaseDescriptor(
    tpAniSirGlobal  pMac,
    tANI_U8         id)
{
    tANI_U8 keyIdx;
    tpDpuInfo pDpu = (tpDpuInfo) pMac->hal.halMac.dpuInfo;
    tANI_U32              address;

    if(id >= pDpu->maxEntries)
        return eHAL_STATUS_INVALID_PARAMETER;

    if(pDpu->descTable[id].used == 0)
        return eHAL_STATUS_SUCCESS;

    // release these Key, MicKey and RC field
    if(pDpu->descTable[id].wepType != eSIR_WEP_STATIC)
    {
        keyIdx = pDpu->descTable[id].keyIdx ;
        if(keyIdx != HAL_INVALID_KEYID_INDEX)
            halDpu_ReleaseKeyId(pMac, keyIdx);

        keyIdx = pDpu->descTable[id].micKeyIdx ;
        if(keyIdx != HAL_INVALID_KEYID_INDEX)
            halDpu_ReleaseMicKeyId(pMac, keyIdx);
    }
    
    keyIdx = pDpu->descTable[id].rcIdx ;
    if(keyIdx != HAL_INVALID_KEYID_INDEX)
        halDpu_ReleaseRCId(pMac, id, keyIdx);

    address = pMac->hal.memMap.dpuDescriptor_offset + (id * sizeof(tDpuDescriptor));
    if(halZeroDeviceMemory(pMac, address, sizeof(tDpuDescriptor)))
    {
        HALLOGW( halLog(pMac, LOGW, FL("halZeroDeviceMemory() failed \n")));
        return eHAL_STATUS_FAILURE;
    }

    halMacClearDpuStats(pMac, id);
    
    return halDpu_ReleaseId(pMac, id);
}



/* ------------------------------------------------------------
 * FUNCTION:  halDpu_AllocKeyId
 *
 * NOTE:
 *   Allocate a new DPU Key descriptor index.
 *
 * ------------------------------------------------------------
 */
eHalStatus
halDpu_AllocKeyId(
    tpAniSirGlobal  pMac,
    tANI_U8         *id)
{
    tANI_U8       i;
    tANI_U8       found = 0;
    tpDpuInfo pDpu = (tpDpuInfo) pMac->hal.halMac.dpuInfo;

    for (i=0; i < pDpu->maxEntries; i++ )
    {
        if(pDpu->keyTable[i].used == 0)
        {
            found = 1;
            break;
        }
    }

    if (found)
    {
        HALLOGW( halLog(pMac, LOGW, FL("Got DPU Key index %d \n"), i));
        *id = i;
        pDpu->keyTable[i].used = 1;
    }
    else
    {
        HALLOGW( halLog(pMac, LOGW, FL("DPU Key descriptor table full")));
        return eHAL_STATUS_STA_TABLE_FULL; // todo: change ret code
    }

    return eHAL_STATUS_SUCCESS;
}

/* ------------------------------------------------------------
 * FUNCTION:  halDpu_ReleaseKeyId
 * NOTE:
 *   Release the Dpu Key ID
 *
 * ------------------------------------------------------------
 */
eHalStatus
halDpu_ReleaseKeyId(
    tpAniSirGlobal  pMac,
    tANI_U8         id)
{
    tpDpuInfo pDpu = (tpDpuInfo) pMac->hal.halMac.dpuInfo;
    if (id >= pDpu->maxEntries)
    {
        HALLOGW( halLog(pMac, LOGW, FL("Got invalid index %d \n"), id));
        return eHAL_STATUS_INVALID_PARAMETER;
    }

    pDpu->keyTable[id].used = 0;
    HALLOGW( halLog(pMac, LOGW, FL("Released DPU Key index %d \n"), id));

    return eHAL_STATUS_SUCCESS;
}

/* ------------------------------------------------------------
 * FUNCTION:  halDpu_SetKeyDescriptor
 * NOTE:
 *   Set the Dpu Key descriptor
 *
 * ------------------------------------------------------------
 */
eHalStatus
halDpu_SetKeyDescriptor(
    tpAniSirGlobal  pMac,
    tANI_U8         id,
    tAniEdType      encryptMode,
    tANI_U8         *pKey)
{
    tHalDpuKeyEntry        *pKeyDesc;
    tANI_U32                keyLen, byteIndx;
    tANI_U8                  keyDescSize = sizeof(tDpuKeyDescriptor);
    tANI_U8                  i = 0;
    tpDpuInfo pDpu = (tpDpuInfo) pMac->hal.halMac.dpuInfo;

    if (id >= pDpu->maxEntries)
    {
        HALLOGE( halLog(pMac, LOGE, FL("Got invalid index %d \n"), id));
        return eHAL_STATUS_INVALID_PARAMETER;
    }

    pKeyDesc = & pDpu->keyTable[id];

    switch (encryptMode)
    {
        case eSIR_ED_WEP40:     keyLen = HAL_WEP40_LENGTH;  break;
        case eSIR_ED_WEP104:    keyLen = HAL_WEP104_LENGTH; break;
        case eSIR_ED_CCMP:      keyLen = HAL_AES_LENGTH;    break;
        case eSIR_ED_TKIP:      keyLen = HAL_TKIP_KEY_LENGTH;   break;
#ifdef FEATURE_WLAN_WAPI
        case eSIR_ED_WPI:       keyLen = HAL_WPI_KEY_LENGTH;    break;
#endif
        default: return eHAL_STATUS_INVALID_PARAMETER;      
    }

/*
  As per mac software architecure document
  The key programming should be such that
  0x0 = B15, B14, B13, B12
  0x4 = B11, B10, B09, B08
  0x8 = B07, B06, B05, B04
  0xC = B03, B02, B01, B00
  Also, the WEP-40 Keys are from B04 : B00
  The WEP-104 Keys are B12 : B00
*/

    palZeroMemory( pMac->hHdd,
        (void *) pKeyDesc->halDpuKey.key128bit,
        sizeof( tDpuKeyDescriptor ));

    for(i = 0; i < keyLen; i++)
    {
      byteIndx = keyDescSize - keyLen + i;
      pKeyDesc->halDpuKey.key128bit[byteIndx >> 2] |= *(pKey + i) << ((3 - (byteIndx%4))*8);
    }

    return (dpu_set_key_descriptor(pMac, pKeyDesc->hwIndex,
                                 & pKeyDesc->halDpuKey));
}


/* ------------------------------------------------------------
 * FUNCTION:  halDpu_AllocMicKeyId
 *
 * NOTE:
 *   Allocate a new DPU Key descriptor index.
 *
 * ------------------------------------------------------------
 */
eHalStatus
halDpu_AllocMicKeyId(
    tpAniSirGlobal  pMac,
    tANI_U8         *id,
    tANI_U8         keyId)
{
    tpDpuInfo pDpu = (tpDpuInfo) pMac->hal.halMac.dpuInfo;

    // Mickey ID is implicity same as the keyId, so check for the
    // same index in the MIC table for a free entry
    if(pDpu->micKeyTable[keyId].used == 0) {
        HALLOGW( halLog(pMac, LOGW, FL("Got DPU MicKey index %d \n"), keyId));
        *id = keyId;
        pDpu->micKeyTable[keyId].used = 1;
        return eHAL_STATUS_SUCCESS;
    }

    HALLOGE( halLog(pMac, LOGE, FL("DPU MicKey descriptor table full")));
    return eHAL_STATUS_DPU_MICKEY_TABLE_FULL;
}

/* ------------------------------------------------------------
 * FUNCTION:  halDpu_ReleaseMicKeyId
 *
 * NOTE:
 *   Release the Dpu MicKey ID
 *
 * ------------------------------------------------------------
 */
eHalStatus
halDpu_ReleaseMicKeyId(
    tpAniSirGlobal  pMac,
    tANI_U8         id)
{
    tpDpuInfo pDpu = (tpDpuInfo) pMac->hal.halMac.dpuInfo;

    if(id >= pDpu->maxEntries)
    {
        HALLOGW( halLog(pMac, LOGW, FL("Got invalid index %d \n"), id));
        return eHAL_STATUS_INVALID_PARAMETER;
    }
    pDpu->micKeyTable[id].used = 0;
    return eHAL_STATUS_SUCCESS;
}

/* ------------------------------------------------------------ 
 * FUNCTION:  halDpu_GetMicKey
 *
 * NOTE:
 *   Get the DPU Mic Key from the Key Buffer.
 *     The Keys are stored in the order B0-B4
 *     This reverses it and returns the byte stream.
 * ------------------------------------------------------------
 */
tANI_U32
halDpu_GetMicKey(
        tANI_U8        *pKey,
        tANI_U8        index)
{
    tANI_U8 i, buf[4];
    
    for (i=0; i<4; i++)
          buf[i] = pKey[index - i];
    
    return sirReadU32N(buf);
}


/* ------------------------------------------------------------
 * FUNCTION:  halDpu_SetMicKeyDescriptor
 *
 * NOTE:
 *   Set the DPU Mic Key descriptor
 *
 * ------------------------------------------------------------
 */
eHalStatus
halDpu_SetMicKeyDescriptor(
    tpAniSirGlobal  pMac,
    tANI_U8         id,
    tANI_U8         *pKey,
    tANI_U8         paeRole )
{
    tANI_U8 *pMicKey;
    tHalDpuMicKeyEntry     *pDpuMicDesc;
    tpDpuInfo pDpu = (tpDpuInfo) pMac->hal.halMac.dpuInfo;

    if(id >= pDpu->maxEntries)
    {
        HALLOGW( halLog(pMac, LOGW, FL("Got invalid index %d \n"), id));
        return eHAL_STATUS_INVALID_PARAMETER;
    }

    pDpuMicDesc = & pDpu->micKeyTable[id];

    pMicKey = (pKey + HAL_TKIP_KEY_LENGTH);

    //
    // Key size -> B0..B31
    // B00..B15 -> TKIP Encryption Key
    // B16..B23 -> Tx/Authenticator MIC Key
    // B24..B31 -> Rx/Supplicant MIC Key
    //
    // paeRole( 1 ) -> Authenticator
    // paeRole( 0 ) -> Supplicant
    //
    if( paeRole )
    {
      // b128..b191 - Authenticator's STA to the Supplicant's STA (or Tx MIC)
      // K0,K1
      pDpuMicDesc->halDpuMicKey.txMicKey64bit[0] = halDpu_GetMicKey( pMicKey, 7 );
      pDpuMicDesc->halDpuMicKey.txMicKey64bit[1] = halDpu_GetMicKey( pMicKey , 3 );

      // b192..b254 - Supplicant's STA to the Authenticator's STA (or Rx MIC)
      // K0,K1
      pDpuMicDesc->halDpuMicKey.rxMicKey64bit[0] = halDpu_GetMicKey( pMicKey, 15 );
      pDpuMicDesc->halDpuMicKey.rxMicKey64bit[1] = halDpu_GetMicKey( pMicKey, 11 );
    }
    else
    {
      // b128..b191 - Supplicant's STA to the Authenticator's STA (or Tx MIC)
      // K0,K1
      pDpuMicDesc->halDpuMicKey.rxMicKey64bit[0] = halDpu_GetMicKey(pMicKey, 7);
      pDpuMicDesc->halDpuMicKey.rxMicKey64bit[1] = halDpu_GetMicKey(pMicKey, 3);

      // b192..b255 - Authenticator's STA to the Supplicant's STA (or Rx MIC)
      // K0,K1
      pDpuMicDesc->halDpuMicKey.txMicKey64bit[0] = halDpu_GetMicKey(pMicKey, 15);
      pDpuMicDesc->halDpuMicKey.txMicKey64bit[1] = halDpu_GetMicKey(pMicKey, 11);
    }
    
    return (dpu_set_mic_key_descriptor(pMac, pDpuMicDesc->hwIndex,
                                          (tANI_U8 *)&pDpuMicDesc->halDpuMicKey, sizeof(tDpuMicKeyDescriptor)));
}


#if defined(FEATURE_WLAN_WAPI)

eHalStatus halDpu_SetWPIMicKeyDescriptor(tpAniSirGlobal pMac, tANI_U8 id,
                        tANI_U8 *pKey, tANI_U8 paeRole )
{
    tHalDpuMicKeyEntry *pDpuMicDesc;
    tDpuWpiMicKeyDescriptor *pDpuWpiMicDesc;
    tpDpuInfo pDpu = (tpDpuInfo) pMac->hal.halMac.dpuInfo;
    tANI_U8 i;

    if(id >= pDpu->maxEntries)
    {
        HALLOGW( halLog(pMac, LOGW, FL("Got invalid index %d \n"), id));
        return eHAL_STATUS_INVALID_PARAMETER;
    }

    pDpuMicDesc = & pDpu->micKeyTable[id];
    //The cast should be fine because they have the same size
    pDpuWpiMicDesc = (tDpuWpiMicKeyDescriptor *)&pDpuMicDesc->halDpuMicKey;

    //For WPI, the whole 16-byte MIC key is for both TX and RX
    //MIC key follows encryption key
    VOS_ASSERT(HAL_WPI_MICKEY_LENGTH == sizeof(tDpuWpiMicKeyDescriptor));
	palZeroMemory(pMac->hHdd, (void *)pDpuWpiMicDesc->wpiMicKey, HAL_WPI_MICKEY_LENGTH);

    for(i = 0; i < HAL_WPI_MICKEY_LENGTH; i++)
    {
        pDpuWpiMicDesc->wpiMicKey[i >> 2] |= *(pKey + i) << ((3 - (i%4))*8);
    }

    return (dpu_set_mic_key_descriptor(pMac, pDpuMicDesc->hwIndex,
                                          (tANI_U8 *)&pDpuWpiMicDesc->wpiMicKey, HAL_WPI_MICKEY_LENGTH));
}

#endif // defined(FEATURE_WLAN_WAPI)

/*****************************************************
 *
 * FUNCTION:  halDpu_GetRCId
 *
 * LOGIC:
 * This API is used to determine if an RC Descriptor
 * has already been allocated for a given DPU index.
 * If YES,
 *   Use the existing RC index
 * Else,
 *   It's the responsibility of the caller to allocate
 *   a new RC descriptor
 *
 *****************************************************/
eHalStatus halDpu_GetRCId( tpAniSirGlobal pMac,
    tANI_U8 dpuIndex,
    tANI_U8 *rcIndex )
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tHalDpuDescEntry *pDpuDesc;
    tpDpuInfo pDpu = (tpDpuInfo) pMac->hal.halMac.dpuInfo;

    pDpuDesc = &pDpu->descTable[dpuIndex];
    if( HAL_INVALID_KEYID_INDEX == pDpuDesc->rcIdx )
        status = eHAL_STATUS_FAILURE;
    else
        *rcIndex = pDpuDesc->rcIdx;

    return status;
}

/*****************************************************
  * FUNCTION:  halDpu_EnableRCWinChk
  *
  * LOGIC:
  * This API is used to enable the replay counter
  * window check.
  *
  *****************************************************/
eHalStatus halDpu_EnableRCWinChk( tpAniSirGlobal pMac,
          tANI_U8 dpuIndex,
        tANI_U32 queueId )
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tANI_U8 rcId;
    tpDpuInfo pDpu = (tpDpuInfo) pMac->hal.halMac.dpuInfo;

    if ((status = halDpu_GetRCId(
        pMac, dpuIndex, &rcId )) != eHAL_STATUS_SUCCESS )
        return status;

    pDpu->rcDescTable[rcId].u.halDpuRC[queueId].winChkEnabled = 1;
    if ((status = dpu_set_rc_descriptor(
        pMac, pDpu->rcDescTable[rcId].hwRCBaseIndex, 
        (tANI_U8) (pDpu->rcDescTable[rcId].hwIndex + queueId), 
        (tANI_U8 *)&pDpu->rcDescTable[rcId].u.halDpuRC[queueId],
        sizeof( tDpuReplayCounterDescriptor ))) != eHAL_STATUS_SUCCESS)
        return status;

    return status;
}
 
/*****************************************************
  *
  * FUNCTION:  halDpu_DisableRCWinChk
 *
 * LOGIC:
 * This API is used to disable the replay counter
 * window check.
 *
 *****************************************************/
eHalStatus halDpu_DisableRCWinChk( tpAniSirGlobal pMac,
        tANI_U8 dpuIndex,
        tANI_U32 queueId )
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tANI_U8 rcId;
    tpDpuInfo pDpu = (tpDpuInfo) pMac->hal.halMac.dpuInfo;

    if ((status = halDpu_GetRCId(
        pMac, dpuIndex, &rcId )) != eHAL_STATUS_SUCCESS )
        return status;
 
     pDpu->rcDescTable[rcId].u.halDpuRC[queueId].winChkEnabled = 0;
    if ((status = dpu_set_rc_descriptor(
        pMac, pDpu->rcDescTable[rcId].hwRCBaseIndex, 
        (tANI_U8) (pDpu->rcDescTable[rcId].hwIndex + queueId), 
        (tANI_U8 *)&pDpu->rcDescTable[rcId].u.halDpuRC[queueId],
        sizeof( tDpuReplayCounterDescriptor ))) != eHAL_STATUS_SUCCESS)
        return status;

    return status;
}

/*****************************************************
 *
 *
 * FUNCTION:  halDpu_GetKeyId
 *
 * LOGIC:
 * This API is used to determine if a Key Descriptor
 * has already been allocated for a given DPU index.
 * If YES,
 *   It returns the existing Key index
 * Else,
 *   It returns failure.
 *
 *****************************************************/
eHalStatus halDpu_GetKeyId( tpAniSirGlobal pMac,
    tANI_U8 dpuIndex,
    tANI_U8 *keyIndex )
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tHalDpuDescEntry *pDpuDesc;
    tpDpuInfo pDpu = (tpDpuInfo) pMac->hal.halMac.dpuInfo;

    pDpuDesc = &pDpu->descTable[dpuIndex];
    if( HAL_INVALID_KEYID_INDEX == pDpuDesc->keyIdx )
        status = eHAL_STATUS_FAILURE;
    else
        *keyIndex = pDpuDesc->keyIdx;

    return status;
}

/*****************************************************
 *
 * FUNCTION:  halDpu_GetMicKeyId
 *
 * LOGIC:
 * This API is used to determine if a MIC-Key Descriptor
 * has already been allocated for a given DPU index.
 * If YES,
 *   It returns the existing MIC-Key index
 * Else,
 *   It returns failure.
 *
 *****************************************************/
eHalStatus halDpu_GetMicKeyId( tpAniSirGlobal pMac,
    tANI_U8 dpuIndex,
    tANI_U8 *micKeyIndex )
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tHalDpuDescEntry *pDpuDesc;
    tpDpuInfo pDpu = (tpDpuInfo) pMac->hal.halMac.dpuInfo;

    pDpuDesc = &pDpu->descTable[dpuIndex];
    if( HAL_INVALID_KEYID_INDEX == pDpuDesc->micKeyIdx )
        status = eHAL_STATUS_FAILURE;
    else
        *micKeyIndex = pDpuDesc->micKeyIdx;

    return status;
}


/* ------------------------------------------------------------
 * FUNCTION:  halDpu_AllocRCId
 *
 * NOTE:
 *   Allocate a new DPU Key descriptor index.
 *   To support WAPI, it is required that the RC descriptors are
 *   pre-allocated on continuous addressing memory because multiple
 *   continuous units are needed.
 *
 * ------------------------------------------------------------
 */
eHalStatus
halDpu_AllocRCId(
    tpAniSirGlobal  pMac,
    tAniEdType      encType,
    tANI_U8         *id)
{
    tANI_U8       i;
    tANI_U8       found = 0;
    tpDpuInfo pDpu = (tpDpuInfo) pMac->hal.halMac.dpuInfo;

    for (i=0; i < pDpu->maxEntries; i++ )
    {
        if(pDpu->rcDescTable[i].used == 0)
        {
#ifdef FEATURE_WLAN_WAPI
            if( eSIR_ED_WPI == encType )
            {
                //need two RC descriptors
                if( i == pDpu->maxEntries - 2 )
                {
                    //No more, 
                    HALLOGE( halLog( pMac, LOGE, FL(" runs out of RC desc for WAPI\n") ) );
                    break;
                }
                //NOTE: Since we need one more for Volans, may as well allocate that for Libra
                if( (pDpu->rcDescTable[i+1].used != 0 ) || (pDpu->rcDescTable[i+2].used != 0 ) )
                {
                    //doesn't fit, try again
                    continue;
                }
                //We allocate two RC descriptors for each RC because WAPI needs 32 bytes
                pDpu->rcDescTable[i+1].used = 1;
                pDpu->rcDescTable[i+2].used = 1;
            }
#endif
            found = 1;
            break;
        }
    }

    if(found)
    {
        HALLOGW( halLog(pMac, LOGW, FL("Got DPU Replay Counter index %d \n"), i));
        *id = i;
        pDpu->rcDescTable[i].used = 1;
        pDpu->rcDescTable[i].hwIndex = i * MAX_NUM_OF_TIDS;
    }
    else
    {
        HALLOGW( halLog(pMac, LOGW, FL("DPU Replay Coutner table full")));
        return eHAL_STATUS_STA_TABLE_FULL; // todo: change ret code
    }

    return eHAL_STATUS_SUCCESS;
}


/* ------------------------------------------------------------
 * FUNCTION:  halDpu_SetRCDescriptor
 *
 * NOTE:
 *   Set the DPU Replay Counter descriptor
 *
 * ------------------------------------------------------------
 */
eHalStatus
halDpu_SetRCDescriptor( tpAniSirGlobal  pMac,
    tANI_U8 id,
    tANI_U16 bRCE,
    tANI_U16 bWCE,
    tANI_U8 *winChkSize )
{
    int i;
    eHalStatus status;
    tpDpuInfo pDpu = (tpDpuInfo) pMac->hal.halMac.dpuInfo;

    if( id >= pDpu->maxEntries )
    {
        HALLOGW( halLog( pMac, LOGW,
                FL("Got invalid index %d \n"), id ));
        return eHAL_STATUS_INVALID_PARAMETER;
    }

    /* To assign RC per TID */
    for( i = 0; i < MAX_NUM_OF_TIDS; i++ )
    {
        //pDpu->rcDescTable[id].hwRCIndex = i; // RC Id0..Id15 Indices
        pDpu->rcDescTable[id].u.halDpuRC[i].txReplayCount31to0 = HAL_DPU_DEFAULT_TX_RC_LOW; // TX RC
        pDpu->rcDescTable[id].u.halDpuRC[i].txReplayCount47to32 = HAL_DPU_DEFAULT_TX_RC_HIGH; // TX RC

        pDpu->rcDescTable[id].u.halDpuRC[i].replayChkEnabled = bRCE;// RCE
        pDpu->rcDescTable[id].u.halDpuRC[i].winChkEnabled = 0;// always disable WCE due to DPU RC check HW bug.

        // Window Size
        pDpu->rcDescTable[id].u.halDpuRC[i].winChkSize = (tANI_U32) winChkSize[i];

        status = dpu_set_rc_descriptor( pMac,
                pDpu->rcDescTable[id].hwRCBaseIndex,
                (tANI_U8) (pDpu->rcDescTable[id].hwIndex + i), // For RC Id0..Id15
                (tANI_U8 *)&pDpu->rcDescTable[id].u.halDpuRC[i],
                sizeof( tDpuReplayCounterDescriptor ) );

        if(status != eHAL_STATUS_SUCCESS)
            return status;
    }

    return eHAL_STATUS_SUCCESS;
}



#if defined(FEATURE_WLAN_WAPI)
//For Chips have other than Libra, this needs to be redefine
eHalStatus halDpu_SetWAPIRCDescriptor( tpAniSirGlobal pMac, tANI_U8 id, tANI_U8 *pTxRC, tANI_U8 *pRxRC )
{
    int i;
    eHalStatus status;
    tpDpuInfo pDpu = (tpDpuInfo) pMac->hal.halMac.dpuInfo;

    if( id >= pDpu->maxEntries )
    {
        HALLOGW( halLog( pMac, LOGW,
                FL("Got invalid index %d \n"), id ));
        return eHAL_STATUS_INVALID_PARAMETER;
    }

    /* To assign RC per TID */
    for( i = 0; i < MAX_NUM_OF_TIDS; i++ )
    {
        palCopyMemory(pMac->hHdd, (void *)pDpu->rcDescTable[id].u.halWpiDpuRC[i].txReplayCount, pTxRC, 16);
        palCopyMemory(pMac->hHdd, (void *)pDpu->rcDescTable[id].u.halWpiDpuRC[i].rxReplayCount, pRxRC, 16);

        status = dpu_set_rc_descriptor( pMac,
                pDpu->rcDescTable[id].hwRCBaseIndex,
                (tANI_U8) (pDpu->rcDescTable[id].hwIndex + i * HAL_WAPI_RC_DESCRIPTOR_COUNT), // For RC Id0..Id15
                (tANI_U8 *)&pDpu->rcDescTable[id].u.halWpiDpuRC[i],
                sizeof( tDpuWpiReplayCounterDescriptor ) );

        if(status != eHAL_STATUS_SUCCESS)
            return status;
    }

    return eHAL_STATUS_SUCCESS;
}

#endif


/* ------------------------------------------------------------
 * FUNCTION:  halDpu_ReleaseRCId
 *
 * NOTE:
 *   Release the Dpu Replay Counter ID
 *
 * ------------------------------------------------------------
 */
eHalStatus
halDpu_ReleaseRCId(
    tpAniSirGlobal  pMac,
    tANI_U8         dpuIdx,
    tANI_U8         id)
{
    tpDpuInfo pDpu = (tpDpuInfo) pMac->hal.halMac.dpuInfo;
    if(id >= pDpu->maxEntries)
        {
            HALLOGW( halLog(pMac, LOGW, FL("Got invalid index %d \n"), id));
            return eHAL_STATUS_INVALID_PARAMETER;
        }
    pDpu->rcDescTable[id].used = 0;
#ifdef FEATURE_WLAN_WAPI
    if( pDpu->descTable[dpuIdx].halDpuDescriptor.wapi )
    {
        VOS_ASSERT( (id < pDpu->maxEntries - 2) && (pDpu->rcDescTable[id+1].used) && (pDpu->rcDescTable[id+2].used));
        pDpu->rcDescTable[id+1].used = 0;
        pDpu->rcDescTable[id+2].used = 0;
    }
#endif
    return eHAL_STATUS_SUCCESS;
}

/* ------------------------------------------------------------
 * FUNCTION:  halDpu_BdRoutingFlagOverride
 *
 * NOTE:
 *   Override the DPU routing flag in BD
 *
 * ------------------------------------------------------------
 */
eHalStatus
halDpu_BdRoutingFlagOverride(
    tpAniSirGlobal  pMac,
    tANI_U8         enable, tANI_U32 wqIdx)
{
    tANI_U32 value;
    halReadRegister(pMac, QWLAN_DPU_DPU_CONTROL_REG, &value);

    if(enable == 0){
        wqIdx = BMUWQ_SINK;
        value &= ~QWLAN_DPU_DPU_CONTROL_ROUTING_FLAG_EN_MASK;
    }else
        value |= QWLAN_DPU_DPU_CONTROL_ROUTING_FLAG_EN_MASK;
    
    halWriteRegister(pMac, QWLAN_DPU_DPU_ROUTING_FLAG_REG, wqIdx);
    halWriteRegister(pMac, QWLAN_DPU_DPU_CONTROL_REG, value);

    return eHAL_STATUS_SUCCESS;
}



/* ------------------------------------------------------------
 * FUNCTION:  halDpu_SetTxReservedBdPdu
 *
 * NOTE:
 *   Set up the default values for DPU reservations
 *   This is intended for use with internal memory only when the
 *   system defaults are too large for teh amount of available memory
 *
 * ------------------------------------------------------------
 */
void
halDpu_SetTxReservedBdPdu(
    tpAniSirGlobal pMac)
{
    tANI_U32  addr[] = {
        QWLAN_DPU_DPU_WQ_3_RESERVE_REG,
        QWLAN_DPU_DPU_WQ_4_RESERVE_REG,
        QWLAN_DPU_DPU_WQ_5_RESERVE_REG,
        QWLAN_DPU_DPU_WQ_6_RESERVE_REG,
        QWLAN_DPU_DPU_WQ_7_RESERVE_REG,
        QWLAN_DPU_DPU_WQ_8_RESERVE_REG,
        QWLAN_DPU_DPU_WQ_9_RESERVE_REG,
        QWLAN_DPU_DPU_WQ_10_RESERVE_REG};
    tANI_U32 value, i;

    value  = ((DPU_TX_RSV_NUMBD_INT << QWLAN_DPU_DPU_WQ_3_RESERVE_BD_CNT_OFFSET)
              & QWLAN_DPU_DPU_WQ_3_RESERVE_BD_CNT_MASK);
    value |= ((DPU_TX_RSV_NUMPDU_INT << QWLAN_DPU_DPU_WQ_3_RESERVE_PDU_CNT_OFFSET)
              & QWLAN_DPU_DPU_WQ_3_RESERVE_PDU_CNT_MASK);

    for (i=0; i<sizeof(addr)/sizeof(addr[0]); i++)
//        (void) halWriteRegister(pMac, addr[i], value);
        (void) halWriteRegister(pMac, addr[i], value);

}

/**
 * @brief  : This Routine is a Interrupt handler for DPU error Interrupts
 *           This issues a mac reset if the error is FATAL, otherwise
 *           Displays the warning and maintains the stats for the error.
 *
 * @note   : The DPU MIC error and DPU Replay Threshold should be handled in LIM.
 * @todo   : For the above mentioned two errors should be handled in LIM so should
 *           be replaced with Posting Message to LIM.
 *
 * @param  : hHalHandle - Mac Global Handle
 * @param  : intSource - Source for the paticular Interrupt.
 * @return : eHAL_STATUS_SUCCESS on Success and appropriate error sattus on error.
 */

eHalStatus halIntDPUErrorHandler(tHalHandle hHalHandle, eHalIntSources intSource)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tANI_U32 intRegMask;
    tANI_U32 intRegStatus;
    tpAniSirGlobal pMac = PMAC_STRUCT(hHalHandle);
    tANI_U32 errWqCnt;

#if 0 //In gen6, we handle the MIC error in this function so comment out this
    /** Do not handle DPU/MIC Error as it is handled at GROUP0 toplevel.*/
    if (intSource == eHAL_INT_DPU_MIC_ERR)
        return eHAL_STATUS_SUCCESS;
#endif

    /** Read Interrupt Status.*/
    status = halIntGetErrorStatus(hHalHandle, intSource, &intRegStatus, &intRegMask);
    if (status != eHAL_STATUS_SUCCESS) {
        HALLOGE( halLog(pMac, LOGE, FL("Unable to read DPU Error Int Register Status!\n")));
        macSysResetReq(pMac, eSIR_DPU_EXCEPTION);
        return status;
    }

    //Read error count reg
    halReadRegister(pMac, QWLAN_DPU_DPU_ERROR_WQ_COUNT_REG, &errWqCnt);

    intRegStatus &= intRegMask;
    
    if (intRegStatus & QWLAN_DPU_DPU_INTERRUPT_STATUS_MIC_ERR_MASK)
    {
        SET_WARNING_COUNTER(pMac->hal.halIntErrStats.halIntDpuMicErr, "dpuMicErr",
            intRegStatus, errWqCnt);
        halDpu_HandleMICErrorInterrupt( pMac );
        return eHAL_STATUS_SUCCESS;
    }

    if (intRegStatus & QWLAN_DPU_DPU_INTERRUPT_STATUS_PKT_ERR_MASK)
    {
        SET_WARNING_COUNTER(pMac->hal.halIntErrStats.halIntDpuPktErr, "dpuPktErr",
            intRegStatus, errWqCnt);
        return eHAL_STATUS_SUCCESS;
    }

    if(intRegStatus & QWLAN_DPU_DPU_INTERRUPT_STATUS_REPLAY_THR_MASK){
        SET_WARNING_COUNTER(pMac->hal.halIntErrStats.halIntDpuReplayTh, "dpuReplayTh",
            intRegStatus, errWqCnt);
    }

    if(intRegStatus & QWLAN_DPU_DPU_INTERRUPT_STATUS_WATCHDOG_TIMER_MASK){
        SET_WARNING_COUNTER(pMac->hal.halIntErrStats.halIntDpuWatchDogErr, "dpuWatchDogErr",
            intRegStatus, errWqCnt);
        //Need to take action if this happens frequenlty.          
    }


    intRegStatus &= ~(QWLAN_DPU_DPU_INTERRUPT_STATUS_PKT_ERR_MASK|
                      QWLAN_DPU_DPU_INTERRUPT_STATUS_MIC_ERR_MASK|
                      QWLAN_DPU_DPU_INTERRUPT_STATUS_REPLAY_THR_MASK|
                      QWLAN_DPU_DPU_INTERRUPT_STATUS_WATCHDOG_TIMER_MASK);

    if(intRegStatus){
        /** Display Error Status Information.*/
        HALLOGE( halLog(pMac, LOGE, FL("DPU FATAL Error Interrupt source %d, Status 0x%x, Enable 0x%x, errWqCnt 0x%x\n"),
                intSource, intRegStatus, intRegMask, errWqCnt));
        macSysResetReq(pMac, eSIR_DPU_EXCEPTION);            
        return status;
    } 

    return (eHAL_STATUS_SUCCESS);
}
/**
    @brief : DPU MIC interrupt handler, post message to HAL.
    @param : pMac MAC global variable
    @return : NONE
*/
void halDpu_HandleMICErrorInterrupt(tpAniSirGlobal pMac)
{
    tSirMsgQ     msg;

    msg.type = SIR_HAL_DPU_MIC_ERROR;
    msg.reserved = 0;
    msg.bodyval = 0;
    msg.bodyptr = NULL;

    if (halPostMsgApi( pMac, &msg) != eSIR_SUCCESS)
    {
        HALLOGE( halLog(pMac, LOGE, FL("Post DPU MIC ERROR from HAL failed\n")));
    }

    return;
}

/** WEP IV + TKIP Extended IV + MIC */
#define MPDU_DATA_IV_LEN           16


static eHalStatus halDpu_GetBDFromDpuErrorWQ(tpAniSirGlobal pMac, void *pBDBuffer, tANI_U32 bdIndex)
{
    tANI_U32        bdPduAddr;
    tHalRxBd     *pDpuErrorBD;

    pDpuErrorBD = (tHalRxBd *) pBDBuffer;

    bdPduAddr = BMU_CONV_BD_PDU_IDX_TO_ADDR(bdIndex);

    HALLOGE( halLog( pMac, LOGE, FL("DPU MIC ERROR BD INDEX : %d BD ADDR: 0x%x\n"), bdIndex, bdPduAddr));

    halReadDeviceMemory(pMac, bdPduAddr, (tANI_U8 *)pDpuErrorBD, HAL_BD_SIZE);

    if ((pDpuErrorBD->mpduDataOffset + MPDU_DATA_IV_LEN) > HAL_BD_SIZE)
    {
        HALLOGE( halLog( pMac, LOGE, FL("DPU MIC ERROR MPDU OFFSET : %d is greater than %d So reading nex PDU Index :%d\n"), 
                pDpuErrorBD->mpduDataOffset, HAL_BD_SIZE, pDpuErrorBD->headPduIdx));
        if (pDpuErrorBD->pduCount)
            return eHAL_STATUS_FAILURE;

        bdPduAddr = BMU_CONV_BD_PDU_IDX_TO_ADDR(pDpuErrorBD->headPduIdx);

        halReadDeviceMemory(pMac, bdPduAddr, (tANI_U8 *)pDpuErrorBD + HAL_BD_SIZE, HAL_BD_SIZE);
    }

    return eHAL_STATUS_SUCCESS;
}


static eHalStatus halDpu_GetDpuMICErrorInfoFromBD(tpAniSirGlobal pMac, tpHalRxBd pBD, tpSirMicFailureInfo pMicInfo)
{
    tANI_U8               *pPayload;
    tANI_U8               *da,*sa,*ta;
    tANI_U16              TSClo;
    tANI_U32              TSChi;
    tpSirMacDataHdr4a     pHdr;

    pHdr = SIR_MAC_BD_TO_MPDUHEADER4A(pBD);

    halUtil_EndianConvert(pMac, (tANI_U32 *)pHdr, sizeof(tSirMacDataHdr4a)+MPDU_DATA_IV_LEN);

    if (pHdr->fc.fromDS)
    {
        if (pHdr->fc.toDS)
        {
            ta = pHdr->addr2;
            da = pHdr->addr3;
            sa = pHdr->addr4;
        }
        else
        {
            ta = pHdr->addr2;
            da = pHdr->addr1;
            sa = pHdr->addr3;
        }
    }
    else
    {
        if (pHdr->fc.toDS)
        {
            ta = pHdr->addr2;
            da = pHdr->addr3;
            sa = pHdr->addr2;
        }
        else
        {
            ta = pHdr->addr2;
            da = pHdr->addr1;
            sa = pHdr->addr2;
        }
    }
    HALLOG1( halLog( pMac, LOG1, FL("DPU MIC ERROR Dpu feedback : %d\n"), pBD->dpuFeedback));

    pPayload = SIR_MAC_BD_TO_MPDUDATA(pBD);
    TSChi    = sirReadU32(&pPayload[4]);
    TSClo    = (pPayload[0] * 256) + pPayload[2];

    palCopyMemory(pMac->hHdd, pMicInfo->srcMacAddr, sa, sizeof(tSirMacAddr));
    palCopyMemory(pMac->hHdd, pMicInfo->taMacAddr, ta, sizeof(tSirMacAddr));
    palCopyMemory(pMac->hHdd, pMicInfo->dstMacAddr, da, sizeof(tSirMacAddr));
    palCopyMemory(pMac->hHdd, pMicInfo->rxMacAddr, pHdr->addr1, sizeof(tSirMacAddr));
    
    HALLOG1( halLog( pMac, LOG1, FL("DPU MIC ERROR SRC MAC ADDR [%x]:[%x]:[%x]:[%x]:[%x]:[%x]\n"), sa[5], sa[4], sa[3], sa[2], sa[1], sa[0]));
    HALLOG1( halLog( pMac, LOG1, FL("DPU MIC ERROR TRANSMIT MAC ADDR [%x]:[%x]:[%x]:[%x]:[%x]:[%x]\n"), ta[5], ta[4], ta[3], ta[2], ta[1], ta[0]));
    HALLOG1( halLog( pMac, LOG1, FL("DPU MIC ERROR DST MAC ADDR [%x]:[%x]:[%x]:[%x]:[%x]:[%x]\n"), da[5], da[4], da[3], da[2], da[1], da[0]));

    if ((da[0] & 0x01))
    {
        pMicInfo->multicast = eSIR_TRUE;
        HALLOG1( halLog( pMac, LOG1, FL("DPU MIC ERROR FOR MULTICAST\n")));
    }
    else
    {
        pMicInfo->multicast = eSIR_FALSE;
        HALLOG1( halLog( pMac, LOG1, FL("DPU MIC ERROR FOR UNICAST\n")));
    }

    pMicInfo->IV1 = pPayload[1];
    pMicInfo->keyId = (pPayload[3] >> 6);
    sirStoreU32N(&pMicInfo->TSC[0],TSChi);
    sirStoreU16N(&pMicInfo->TSC[4],TSClo);

    return eHAL_STATUS_SUCCESS;    
}

/**
    @brief: This handles the DPU MIC Error message in HAL.
          - Pops the BD out of DPU MIC Error WQ(17)
          - Reads the BD.
          - Push the to Junk WQ.
          - Indicate DPU MIC error to HDD with info got from BD.
          - Continue this until there is no packets in the
          DPU error WQ.

    @param : pMac MAC global variable.
    @return : NONE.
*/
void halDpu_MICErrorIndication(tpAniSirGlobal pMac)
{
    tANI_U32   bdIndex;
    tSirMsgQ   msg;
    eHalStatus status;
    tHalRxBd   *pDpuErrorBD;
    tpSirSmeMicFailureInd pMicInd;
    tBssSystemRole BssSystemRole = eSYSTEM_UNKNOWN_ROLE;

    /** Allocating for two BD/PDU just in case if the MPDU Header+Data dosent fit in BD.*/
    status = palAllocateMemory( pMac->hHdd, (void **) &pDpuErrorBD, HAL_BD_SIZE * 2);
    if (status != eHAL_STATUS_SUCCESS)
    {
        HALLOGE( halLog( pMac, LOGE, FL("Error Allocating Memory for BD Failed\n")));
        return;
    }

    do
    {
        /** Pop BD from DPU Error WQ.*/
        bmuCommand_pop_wq(pMac, BMUWQ_DPU_ERROR_WQ, &bdIndex);

        /** If WQ is empty return.*/
        if (bdIndex == 0)
            break;

        status = halDpu_GetBDFromDpuErrorWQ(pMac, pDpuErrorBD, bdIndex);
        if (status != eHAL_STATUS_SUCCESS)
        {
            HALLOGE( halLog( pMac, LOGE, FL("Unable to Get BD\n")));
            bmuCommand_push_wq(pMac, BMUWQ_SINK, bdIndex);
            goto error;
        }

        /** Free the BD PDU Link by pushing to Junk WQ.*/
        bmuCommand_push_wq(pMac, BMUWQ_SINK, bdIndex);

        status = palAllocateMemory( pMac->hHdd, (void **) &pMicInd, sizeof(tSirSmeMicFailureInd));
        if (status != eHAL_STATUS_SUCCESS)
        {
            HALLOGE( halLog( pMac, LOGE, FL("Error Allocating Memory for Mic Failure Indication\n")));
            goto error;
        }

        palZeroMemory( pMac->hHdd, pMicInd, sizeof(tSirSmeMicFailureInd));

        status = halDpu_GetDpuMICErrorInfoFromBD(pMac, pDpuErrorBD, &pMicInd->info);
        if (status != eHAL_STATUS_SUCCESS)
        {
            HALLOGE( halLog( pMac, LOGE, FL("Unable to Get Dpu MIC Error info from BD\n")));
            palFreeMemory(pMac->hHdd, (tANI_U8 *) pMicInd);
            goto error;
        }

        msg.type = eWNI_SME_MIC_FAILURE_IND;
        msg.reserved = 0;
        msg.bodyval = 0;
        msg.bodyptr = pMicInd;

#if defined(ANI_OS_TYPE_LINUX) && defined(ANI_LITTLE_BYTE_ENDIAN)
        sirStoreU16N((tANI_U8 *) &pMicInd->messageType, eWNI_SME_MIC_FAILURE_IND);
        sirStoreU16N((tANI_U8 *) &pMicInd->length, sizeof(tSirSmeMicFailureInd));
#else
        pMicInd->messageType = eWNI_SME_MIC_FAILURE_IND;
        pMicInd->length = sizeof(tSirSmeMicFailureInd);    // len in bytes
#endif

        BssSystemRole = halGetBssSystemRoleFromStaIdx(pMac, pDpuErrorBD->addr2Index);

        if(BssSystemRole == eSYSTEM_AP_ROLE)
        {
            palCopyMemory(pMac->hHdd, pMicInd->bssId, pMicInd->info.rxMacAddr,sizeof(tSirMacAddr));
        } else
        {   
            palCopyMemory(pMac->hHdd, pMicInd->bssId,pMicInd->info.taMacAddr,sizeof(tSirMacAddr));            
        }
        
        HALLOGE( halLog( pMac, LOGE, FL("Posting DPU MIC Error to HDD!!\n")));
        if (halMmhPostMsgApi(pMac, &msg, eHI_PRI) != eSIR_SUCCESS)
        {
            palFreeMemory(pMac->hHdd, (tANI_U8 *) pMicInd);
            HALLOGE( halLog( pMac, LOGE, FL("Error Posting Mic Failure Indication to HDD\n")));
        }

    } while(TRUE);

error:
    halDpu_FreePendingErrorPackets(pMac);
    palFreeMemory(pMac->hHdd, (tANI_U8 *) pDpuErrorBD);
    return;
}

static void halDpu_FreePendingErrorPackets(tpAniSirGlobal pMac)
{
    tANI_U32   bdIndex;

    for(;;)
    {
        /** Pop BD from DPU Error WQ.*/
        bmuCommand_pop_wq(pMac, BMUWQ_DPU_ERROR_WQ, &bdIndex);

        /** If WQ is empty return.*/
        if (bdIndex == 0)
            break;

        /** Free the BD PDU Link by pushing to Junk WQ.*/
        bmuCommand_push_wq(pMac, BMUWQ_SINK, bdIndex);
    }

    return;
}

/*
   Function Name     : halDpu_GetSequence
   Input Arguments   : Pointer ti tpAniSirGlobal Structure
                         DUP Index
   Output Values       : Sequence Num
   Return Values       : Status

   This function retrieves the sequence number of the last packet transmitted
*/

eHalStatus halDpu_GetSequence(tpAniSirGlobal pMac, tANI_U8 dpuIdx, tANI_U8 tId, tANI_U16 *sequenceNum)
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    tDpuDescriptor    dpuDesc;


    status = dpu_get_descriptor(pMac, dpuIdx, &dpuDesc);
    if(status != eHAL_STATUS_SUCCESS)
        return status;

    if (tId > MAX_NUM_OF_TIDS)
        return status;
    HALLOG1( halLog( pMac, LOG1, FL("DPU seqNum %08x %08x %08x %08x \n"),dpuDesc.sequenceField[0],
        dpuDesc.sequenceField[1],dpuDesc.sequenceField[2],dpuDesc.sequenceField[3]));

    if(tId & DPU_AUTOSEQ_FIELD_MASK)
       *sequenceNum = (tANI_U16) dpuDesc.sequenceField
                                        [tId/DPU_AUTOSEQ_FIELD_LEN].tid1;
    else
       *sequenceNum = (tANI_U16) dpuDesc.sequenceField
                                        [tId/DPU_AUTOSEQ_FIELD_LEN].tid0;

    HALLOG1( halLog( pMac, LOG1, FL("DPU %d tid %d cur seqNum is %d\n"), dpuIdx, tId, *sequenceNum));
    return eHAL_STATUS_SUCCESS;
}

/*
   Function Name     : halDpu_ResetEncryMode
   Input Arguments   : Pointer ti tpAniSirGlobal Structure
                         DPU Index
   Return Values       : Status
   This function retrieves the DPU control parameters and resets the encryption mode and sets it back in DPU Descriptor
*/

eHalStatus halDpu_ResetEncryMode(tpAniSirGlobal pMac, tANI_U8 dpuIdx)
{
    tpDpuInfo pDpu = (tpDpuInfo) pMac->hal.halMac.dpuInfo;
    tDpuDescriptor dpu;

    if( eHAL_STATUS_SUCCESS == dpu_get_descriptor( pMac,
          dpuIdx,
          &dpu ))
    {
	if (dpu.encryptMode) {

            /* Reset Encryption Mode Only */
            dpu.encryptMode = 0;
	    pDpu->descTable[dpuIdx].halDpuDescriptor.encryptMode = 0;
					
            return dpu_set_descriptor( pMac,
                    pDpu->descTable[dpuIdx].hwIndex,
                    0,
                    sizeof(tDpuDescriptor),    
                    &dpu );
         }
         return eHAL_STATUS_SUCCESS;
    }
    return eHAL_STATUS_FAILURE;
}


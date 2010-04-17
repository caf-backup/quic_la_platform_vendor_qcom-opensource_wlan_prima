/*
 * Qualcomm Inc proprietary. All rights reserved.
 * halBmu.c:  Provides all the MAC APIs to the BMU Hardware Block.
 * Author:    Susan Tsao
 * Date:      02/06/2006
 *
 * --------------------------------------------------------------------------
 */

#include "halTypes.h"
#include "aniGlobal.h"
#include "libraDefs.h"
#include "halDebug.h"
#include "halAdu.h"
#include "halMTU.h"
#include "halMacWmmApi.h"
#include "cfgApi.h"             //wlan_cfgGetInt
#include "halRegBckup.h"


#define DYNAMIC_RECORD    1

/* --------------------------------------------------------------------------
 * Local definitions
 */
#define  BD_PDU_INTERCHANGEABLE        1
#define  BD_PDU_NOT_INTERCHANGEABLE    0

#define ENABLE_TRACE   1
#define DISABLE_TRACE  0

#define  PACKET_MEMORY_256K    0x40000      // 256K byte
#define  PACKET_MEMORY_512K    0x80000      // 512K byte
#define  PACKET_MEMORY_1MB     0x100000     // 1M   byte
#define  PACKET_MEMORY_2MB     0x200000     // 2M   byte
#define  PACKET_MEMORY_3MB     0x300000     // 3M   byte

#define  RECOMMENDED_NUMOF_BD_FOR_1MB      1024
#define  RECOMMENDED_NUMOF_BD_FOR_2MB      1535
#define  RECOMMENDED_NUMOF_BD_FOR_3MB      2047

#define BMU_REG_MAX_POLLING  500
#define BMU_REG_POLLING_WARNING  50
// If we're using external DRAM, the lower 128K is not used
#define  EXTERNAL_DRAM_UNUSABLE  0x20000    // 128K byte

/* --------------------------
 *  BMU Tracing Commands API
 * --------------------------
 */
typedef enum eBmuTraceCmd {
    eBMU_TRACE_POP_CMD             = 0x0,
    eBMU_TRACE_PUSH_CMD            = 0X1,
    eBMU_TRACE_RELEASE_PDU_CMD     = 0X2,
    eBMU_TRACE_RELEASE_BD_CMD      = 0X3,
    eBMU_TRACE_RELEASE_BD_PDU_CMD  = 0X4,
    eBMU_TRACE_RSV_CMD             = 0X5,
    eBMU_TRACE_GET_CMD             = 0X6,
    eBMU_TRACE_GET_CMD_SECOND_PART = 0x7
} tBmuTraceCmd;

typedef struct sBdPduThr {
    int masterIndex;
    int thrRegAddr;
    int bdThrIntMem;
    int bdThrExtMem;
    int pduThrIntMem;
    int pduThrExtMem;
} tBdPduThr, *tpBdPduThr;

typedef struct sBdPduDynThr {
    int bdThrIntMem;
    int bdThrExtMem;
    int pduThrIntMem;
    int pduThrExtMem;
} tBdPduDynthr, *tpBdPduDynthr;

typedef struct sBtqmQIdMapping {
    tMtuBkId bkId;
    tANI_U8  tid;
}tBtqmQIdMapping;


/* list of BD and PDU thresholds corresponding to each module index */
//in accordance with libra_wmac_sys_programmers_guide.doc section 4.7.1
static tBdPduThr thrInfo[] = {
    // moduleIndex          Register address                 BD threshold (intMem)           BD threshold (extMem)      PduThr (InternalMem)           PduThr (ExternalMem)
    {BMU_MASTERID_0,   QWLAN_BMU_BD_PDU_THRESHOLD0_REG,  BMU_RXP_BD_THRESHOLD_INT,       BMU_RXP_BD_THRESHOLD,      BMU_RXP_PDU_THRESHOLD_INT,       BMU_RXP_PDU_THRESHOLD},
    {BMU_MASTERID_1,   QWLAN_BMU_BD_PDU_THRESHOLD1_REG,  BMU_DPUTX_BD_THRESHOLD_INT,     BMU_DPUTX_BD_THRESHOLD,    BMU_DPUTX_PDU_THRESHOLD_INT,     BMU_DPUTX_PDU_THRESHOLD},
    {BMU_MASTERID_2,   QWLAN_BMU_BD_PDU_THRESHOLD2_REG,  BMU_DPURX_BD_THRESHOLD_INT,     BMU_DPURX_BD_THRESHOLD,    BMU_DPURX_PDU_THRESHOLD_INT,     BMU_DPURX_PDU_THRESHOLD},
    {BMU_MASTERID_3,   QWLAN_BMU_BD_PDU_THRESHOLD3_REG,  BMU_ADU_BD_THRESHOLD_INT,       BMU_ADU_BD_THRESHOLD,      BMU_ADU_PDU_THRESHOLD_INT,       BMU_ADU_PDU_THRESHOLD},
    {BMU_MASTERID_4,   QWLAN_BMU_BD_PDU_THRESHOLD4_REG,  BMU_RPE_BD_THRESHOLD_INT,       BMU_RPE_BD_THRESHOLD,      BMU_RPE_PDU_THRESHOLD_INT,       BMU_RPE_PDU_THRESHOLD},
    {BMU_MASTERID_5,   QWLAN_BMU_BD_PDU_THRESHOLD5_REG,  BMU_DXE_TX_BD_THRESHOLD_INT,       BMU_DXE_TX_BD_THRESHOLD,   BMU_DXE_TX_PDU_THRESHOLD_INT,       BMU_DXE_TX_PDU_THRESHOLD},
    {BMU_MASTERID_6,   QWLAN_BMU_BD_PDU_THRESHOLD6_REG,  BMU_DXE_RX_BD_THRESHOLD_INT,    BMU_DXE_RX_BD_THRESHOLD,   BMU_DXE_RX_PDU_THRESHOLD_INT,    BMU_DXE_RX_PDU_THRESHOLD},
    {BMU_MASTERID_7,   QWLAN_BMU_BD_PDU_THRESHOLD7_REG,  BMU_DXE_FW_BD_THRESHOLD_INT,    BMU_DXE_FW_BD_THRESHOLD,   BMU_DXE_FW_PDU_THRESHOLD_INT,    BMU_DXE_FW_PDU_THRESHOLD}
};

static tBtqmQIdMapping btqmQIdMapping[] = {
    // Back off ID        TID
    {MTU_BKID_AC_BE, BTQM_QUEUE_TX_TID_0},
    {MTU_BKID_AC_BK, BTQM_QUEUE_TX_TID_1},
    {MTU_BKID_AC_BK, BTQM_QUEUE_TX_TID_2},
    {MTU_BKID_AC_BE, BTQM_QUEUE_TX_TID_3},
    {MTU_BKID_AC_VI, BTQM_QUEUE_TX_TID_4},
    {MTU_BKID_AC_VI, BTQM_QUEUE_TX_TID_5},
    {MTU_BKID_AC_VO, BTQM_QUEUE_TX_TID_6},
    {MTU_BKID_AC_VO, BTQM_QUEUE_TX_TID_7},
    {MTU_BKID_nQoS,  0},
    {MTU_BKID_MGMT,  0},
    {MTU_BKID_0,     0}
};

/* TID->QueueID mapping*/
static tANI_U8 btqmQosTid2QidMapping[] = { 
    BTQM_QID0, BTQM_QID1, BTQM_QID2, BTQM_QID3, BTQM_QID4, BTQM_QID5, BTQM_QID6, BTQM_QID7 
};

/* --------------------------------------------------------------------------
 * Local functions
 */
static void bmu_init_bd_pdu_pointer(
    tpAniSirGlobal  pMac,
    tANI_U32       *pBdStart,
    tANI_U32       *pBdEnd,
    tANI_U32       *pPduStart,
    tANI_U32       *pPduEnd,
    tANI_U32       *bdPduExchangFlag);

static void  bmu_init_control_register(
    tpAniSirGlobal pMac,
    tANI_U32       maxBdIndex,
    tANI_U32       pduTailIndex);

static eHalStatus bmu_init_wq2_to_wq26(tpAniSirGlobal pMac);
static eHalStatus bmu_init_error_interrupt(tpAniSirGlobal pMac);
static void bmu_init_bd_pdu_threshold(tpAniSirGlobal pMac, tANI_U32 intMem);
static void bmu_set_wq_head_tail_nr(tpAniSirGlobal pMac, tANI_U32 wqIndex, tANI_U32 head, tANI_U32 tail, tANI_U32 nr);
static eHalStatus halEEPROM_getMemoryInfo(tpAniSirGlobal pMac, tANI_U32 *internal, tANI_U32 *external);
static eHalStatus halEEPROM_getNumOfBdPDu(tpAniSirGlobal pMac, tANI_U32 *bd, tANI_U32 *pdu);
static eHalStatus bmu_init_queueid_bo_mapping(tpAniSirGlobal pMac);
static eHalStatus bmu_fast_init_wq0_bd_idle_list(tpAniSirGlobal pMac, tANI_U32 bdStart,
                                                 tANI_U32 bdEnd, tANI_U32 startIndex, tANI_U32 *tailBdIndex);
eHalStatus halBmu_InitStaMemory(tpAniSirGlobal pMac);
static eHalStatus halBmu_InitControl2Reg(tpAniSirGlobal pMac, tANI_U32 mask);
static eHalStatus halBmu_tx_queue_staid_qid_config(tpAniSirGlobal pMac,  tANI_U32 maxStaid, tANI_U32 maxQid, tANI_U32 maxBssStaIds);
static eHalStatus halBmu_btqm_tx_wq_base_addr(tpAniSirGlobal pMac,  tANI_U32 tx_wq_addr);
static eHalStatus halBmu_queueid_qos_map(tpAniSirGlobal pMac);
static eHalStatus halBmu_btqm_init_control1_register(tpAniSirGlobal pMac, tANI_U32 ctrlCfg);
static void halBmu_btqm_init_control2_register(tpAniSirGlobal pMac, tANI_U32 txQueueIdsMask);
static eHalStatus halBmu_InitIntHanlder(tpAniSirGlobal pMac);


/* -------------------------------------------------------------
 * FUNCTION:  halBmu_Start()
 *
 * NOTE:
 *   BMU initialization procedures.
 *
 * TODO IN FUTURE:
 *  - Program the correct BD PDU Threshold for RXP, DPU, DXE, etc
 * ---------------------------------------------------------------
 */
eHalStatus
halBmu_Start(
    tHalHandle hHal,
    void      *arg)
{
    tANI_U32 bdTailIndex;

    tANI_U32 bdStart=0, bdEnd=0, pduStart=0,pduEnd=0;
    tANI_U32 bdPduExchangeable = BD_PDU_NOT_INTERCHANGEABLE;

    tpAniSirGlobal pMac = (tpAniSirGlobal)hHal;
    tANI_U32 value, status;
    (void) arg;


    HALLOGW( halLog( pMac, LOGW, FL("%s: ***** BMU INITIALIZATION ***** \n"),  __FUNCTION__ ));

#ifdef DYNAMIC_RECORD
    // Start recording register writes
    halRegBckup_StartRecord(pMac, (HAL_REG_RSVD_BIT|HAL_REG_HOST_FILLED));
#endif

    // Initialize the BD/PDU base address
    if (halBmu_InitBdPduBaseAddress(pMac) != eHAL_STATUS_SUCCESS) {
        HALLOGE( halLog (pMac, LOGE, FL("Init BD/PDU base addr failed")));
        return eHAL_STATUS_FAILURE;
    }

    /** Set the Start Address of TxWQ */
    if (halBmu_btqm_tx_wq_base_addr(pMac, pMac->hal.memMap.btqmTxQueue_offset) != eHAL_STATUS_SUCCESS)
        return eHAL_STATUS_FAILURE;
    bmu_init_bd_pdu_pointer(pMac, &bdStart, &bdEnd, &pduStart, &pduEnd, &bdPduExchangeable);
    pMac->hal.halMac.bdPduExchangeable = bdPduExchangeable;

    /** Initialize the BD using fast bd setup control */
    //BD 0 is reserved. so startIndex is 1.
    if (bmu_fast_init_wq0_bd_idle_list(pMac, bdStart, bdEnd, 1, &bdTailIndex) != eHAL_STATUS_SUCCESS)
        return eHAL_STATUS_FAILURE;

    /* For Gen6, all BD/PDUs are interchangable */
    if (bdPduExchangeable != BD_PDU_INTERCHANGEABLE)
        return eHAL_STATUS_FAILURE;

    bmu_set_wq_head_tail_nr(pMac, BMUWQ_BMU_IDLE_PDU, 0, 0, 0);

    halDpu_SetTxReservedBdPdu(pMac);

    bmu_init_control_register(pMac, bdTailIndex, bdTailIndex);

    /** Configure the control2 register to enable AMSDU release support */
    if (halBmu_InitControl2Reg(pMac, QWLAN_BMU_CONTROL2_AMSDU_RELEASE_SUPPORT_ENABLE_MASK) != eHAL_STATUS_SUCCESS)
        return eHAL_STATUS_FAILURE;

    /** Program the STA ids and QIDs */
    if (halBmu_tx_queue_staid_qid_config(pMac, pMac->hal.memMap.maxStations - 1,
                    pMac->hal.memMap.maxHwQueues - 1, 1) != eHAL_STATUS_SUCCESS)
        return eHAL_STATUS_FAILURE;

    /** Mapping of Qos frames to Qid */
    if(halBmu_queueid_qos_map(pMac) != eHAL_STATUS_SUCCESS)
        return eHAL_STATUS_FAILURE;

    /** Assign Backoff Engines to Qid*/
    if (bmu_init_queueid_bo_mapping(pMac) != eHAL_STATUS_SUCCESS)
        return eHAL_STATUS_FAILURE;

    /** Initialize the BTQM STA memory */
    if (halBmu_InitStaMemory(pMac) != eHAL_STATUS_SUCCESS)
        return eHAL_STATUS_FAILURE;

    /** Decide which QueueId are for transmission */
    halBmu_btqm_init_control2_register(pMac, TX_QID_ENABLE);

    halReadRegister(pMac, QWLAN_BMU_BTQM_STATUS_REG, &status);
    HALLOGW( halLog( pMac, LOGW, FL("HAL BTQM Status 0x%x\n"),  status ));


    value = QWLAN_BMU_BTQM_CONTROL1_BTQM_QUEUEING_CTRL_HP_ENABLE_MASK |
                        QWLAN_BMU_BTQM_CONTROL1_BTQM_QUEUEING_CTRL_LP_ENABLE_MASK |
                        QWLAN_BMU_BTQM_CONTROL1_BTQM_TPE_INTERFACE_ENABLE_MASK |
                        QWLAN_BMU_BTQM_CONTROL1_BTQM_RXP_BA_INTERFACE_ENABLE_MASK;

    if (halBmu_btqm_init_control1_register(pMac, value) != eHAL_STATUS_SUCCESS)
        return eHAL_STATUS_FAILURE;

#ifdef DYNAMIC_RECORD
    // Issue a wait command into the ADU register list
    halRegBckup_WriteWaitCmd(pMac, 
            &pMac->hal.RegBckupParam.regListCurrAddr, 
            HAL_REG_REINIT_BMU_CONTROL1_WAIT_CYCLES );
#endif

    if (halBmu_enableWq(pMac, BMUWQ_BMU_IDLE_BD, BMUWQ_BMU_IDLE_PDU) != eHAL_STATUS_SUCCESS)
        return eHAL_STATUS_FAILURE;

    if (bmu_init_wq2_to_wq26(pMac) != eHAL_STATUS_SUCCESS)
        return eHAL_STATUS_FAILURE;

    // Enable WQ2 - WQ26
    if (halBmu_enableWq(pMac, BMUWQ_BMU_WQ2, BMUWQ_NUM - 1) != eHAL_STATUS_SUCCESS)
        return eHAL_STATUS_FAILURE;

    if (bmu_init_error_interrupt(pMac) != eHAL_STATUS_SUCCESS)
        return eHAL_STATUS_FAILURE;

    bmu_init_bd_pdu_threshold(pMac, bdPduExchangeable) ;

    halBmu_InitIntHanlder(pMac);

    /** Initialize the BMU BD/PDU Dynamic Threshold Change Flag.*/
    pMac->hal.halMac.halDynamicBdPduEnabled = eANI_BOOLEAN_TRUE;

    /* If we need HW Trace, we can enable this macro */
    // This is mainly required when in Power Save. This register
    // will be restored when the chip is powered back up.
    // We will not need a dump command.
#ifdef BMU_HW_TRACING
    {
        tANI_U32    temp_regValue=0; 

        // Alter this to the bits that we need to enable
        halWriteRegister(pMac, QWLAN_BMU_ENHANCED_TRACING_CONTROL1_REG, 0x144f) ;

        // This is to include HW integrity checks in BMU.
        halReadRegister(pMac, QWLAN_BMU_CONTROL2_REG, &temp_regValue);
        temp_regValue |= QWLAN_BMU_CONTROL2_BLOCK_RXP_PUSH_INTEGRITY_CHECK_MASK;
        temp_regValue |= QWLAN_BMU_CONTROL2_FREEZE_ONE_ERROR_ENABLE_MASK;
        temp_regValue |= QWLAN_BMU_CONTROL2_RELEASE_INTEGRITY_CHECK_MASK;
        temp_regValue |= QWLAN_BMU_CONTROL2_PUSH_DATA_INTEGRITY_CHECK_MASK;

        halWriteRegister(pMac, QWLAN_BMU_CONTROL2_REG, temp_regValue);
    }
#endif

#ifdef DYNAMIC_RECORD
    // Stop recording the register writes
    halRegBckup_StopRecord(pMac);
#endif

    return eHAL_STATUS_SUCCESS;
}


/* ----------------------------------------
 * FUNCTION: halEEPROM_getMemoryInfo()
 *
 * NOTE:
 *   Get internal memory size and external
 *   memory size from EEPROM.
 * ----------------------------------------
 */
static eHalStatus
halEEPROM_getMemoryInfo(tpAniSirGlobal pMac, tANI_U32 *internal, tANI_U32 *external)
{
    *external = 0;  //assume no external SDRAM
    *internal = getInternalMemory(pMac) - pMac->hal.memMap.packetMemory_offset;

    return eHAL_STATUS_SUCCESS;
}

/* ------------------------------------
 * FUNCTION: halEEPROM_getNumOfBdPDu()
 * ------------------------------------
 */
static eHalStatus
halEEPROM_getNumOfBdPDu(tpAniSirGlobal pMac, tANI_U32 *bd, tANI_U32 *pdu)
{
    *bd = 0;
    *pdu = 0;

    return eHAL_STATUS_SUCCESS;
}

/* ----------------------------------------------------
 * FUNCTION: halBMU_getPacketMemorySize()
 *
 * NOTE:
 *   Compute the actual size of the Packet memory,
 *   Take into account the softmac un-cached fragments.
 * -----------------------------------------------------
 */
static eHalStatus
halBMU_getPacketMemorySize(tpAniSirGlobal pMac, tANI_U32 *pktMemory)
{
    tANI_U32   internal, external;
    tANI_U32   pMemStart, pMemEnd;
    tANI_U32   chunkSize[16];
    tANI_U32   chunk = 0;
#if defined(ANI_BUS_TYPE_USB)
    tANI_U32   pUsbStagingBufStart = USB_STAGING_BUF_OFFSET, pUsbStagingBufEnd = BMU_PACKET_MEMORY_OFFSET;
#endif

    halEEPROM_getMemoryInfo(pMac, &internal, &external);

    // The lower 128K in the external DRAM is not accessible
    if (external >= EXTERNAL_DRAM_UNUSABLE)
        external -= EXTERNAL_DRAM_UNUSABLE;


    *pktMemory = 0;
    pMemStart = pMac->hal.memMap.packetMemory_offset;

    pMemEnd = pMac->hal.memMap.packetMemory_offset + internal + external;

    if (pMemEnd > pMemStart)
    {
        chunkSize[chunk] = pMemEnd - pMemStart;
        *pktMemory += chunkSize[chunk];
    }

    return eHAL_STATUS_SUCCESS;
}


/* ----------------------------------------------------
 * FUNCTION: halBMU_getNumOfBdPdu()
 *
 * NOTE:
 *   For a given packet memory size configuration,
 *   calculate the number of "128-byte" BD that can
 *   fit into a given memory configuration. Then
 *   compute the number BD and PDU it can be supported.
 *
 *   The recommended setting for "number of BD"
 *   are of following:
 *
 *      Packet Memory        Number of BD
 *      -------------        ------------
 *         1Mbyte                1024
 *         2Mbyte                1535
 *         3Mbyte or more        2047
 * ----------------------------------------------------
 */
void
halBMU_getNumOfBdPdu(tpAniSirGlobal pMac, tANI_U32 *numOfBD, tANI_U32 *numOfPDU)
{
    tANI_U32   numOfSegment, pktMemSize;

    halBMU_getPacketMemorySize(pMac, &pktMemSize);

    numOfSegment = (pktMemSize / HAL_BD_SIZE);

    if (pktMemSize < PACKET_MEMORY_512K)
    {
        *numOfBD = numOfSegment;
        *numOfPDU = *numOfBD;
    }
    else if ( (pktMemSize >= PACKET_MEMORY_512K) && (pktMemSize < PACKET_MEMORY_2MB) )
    {
        *numOfBD = RECOMMENDED_NUMOF_BD_FOR_1MB;
        *numOfPDU = numOfSegment - *numOfBD;
    }
    else if ( (pktMemSize >= PACKET_MEMORY_2MB) && (pktMemSize < PACKET_MEMORY_3MB) )
    {
        *numOfBD = RECOMMENDED_NUMOF_BD_FOR_2MB;
        *numOfPDU = numOfSegment - *numOfBD;
    }
    else if (pktMemSize >= PACKET_MEMORY_3MB)
    {
        *numOfBD = RECOMMENDED_NUMOF_BD_FOR_3MB;
        *numOfPDU = numOfSegment - *numOfBD;
    }

    /** Initialize the BMU BD-PDU Dynamic Threshold Change Flag.*/
    pMac->hal.halMac.halMaxBdPduAvail = numOfSegment;

    HALLOGW( halLog(pMac, LOGW, FL(" BMU Calculates: %d BD,  %d PDU, Total = %d \n"),
                       *numOfBD, *numOfPDU, numOfSegment));

    return;
}


/* ----------------------------------------------------------------------
 * FUNCTION: bmu_init_bd_pdu_pointer()
 *
 * NOTE:
 *   - If the number of BD and PDU are the same, then BMU is set
 *     in a special mode where each BD can be used as a PDU,
 *     and vice versa.
 *   - If the number of BD and PDU are not equal, then BD index
 *     range from 1 to X, while PDU index range from X+1, to N.
 *
 * PARAM:
 *   pBdStart:               pointer to the start of BD 1
 *   pBdEnd:                 pointer to the start of BD X
 *   pPduStart:              pointer to the start of BD X+1
 *   pPduEnd:                pointer to the start of BD N
 *   numOfBD, numOfPDU:      Compute based on internal/external memory
 *   eeprom_bd, eeprom_pdu:  BD, PDU read from EEPROM
 *
 *   FOr a given eeprom configuration, BD and PDU shall be assigned as following:
 *     eeprom_bd   eeprom_pdu
 *    ------------------------
 *        0           0       BD=numOfBD, PDU=numOfPDU
 *        X           Y       BD = min{numOfBD, eeprom_bd}
 *                            PDU = min{numOfPDU, eeprom_pdu}
 *        X           0       BD = numOfBD and (BD should be less than 2047).
 *                            BD & PDU used interchangeably.
 *        0           Y       BD=numOfBD,  PDU = min[numOfPDU, eeprom_pdu)
 * --------------------------------------------------------------------
 */
static void
bmu_init_bd_pdu_pointer(
    tpAniSirGlobal pMac,
    tANI_U32      *pBdStart,
    tANI_U32      *pBdEnd,
    tANI_U32      *pPduStart,
    tANI_U32      *pPduEnd,
    tANI_U32      *bdPduExchangeFlag)
{

    tANI_U32    numOfBD, numOfPDU;
    tANI_U32    eeprom_bd, eeprom_pdu;
    tANI_U32    i, addr;
    tANI_U32    availBD;     // available BD (including BD 0)
    tANI_U32    availPDU;
    tANI_U32    internal, external;


    halEEPROM_getMemoryInfo(pMac, &internal, &external);
    halBMU_getNumOfBdPdu(pMac, &numOfBD, &numOfPDU);
    halEEPROM_getNumOfBdPDu(pMac, &eeprom_bd, &eeprom_pdu);

    if ( (eeprom_bd != 0) && (eeprom_bd < numOfBD) )
        numOfBD = eeprom_bd;

    if ( (eeprom_pdu != 0) && (eeprom_pdu < numOfPDU) )
        numOfPDU = eeprom_pdu;


    /* We're only going to use up to BD index 1535. The remaining
     * BDs will be reserved for tracing purpose.
     *
     * So with the "numOfBD" allowed, and max BD index 1535,
     * calculate how many BD can actually be used within
     * the free packet memory space.
     */
    *pBdStart = pMac->hal.memMap.packetMemory_offset + BMU_BD_SIZE;    // ptr to BD 1
    availBD = 1;
    for (i=1, addr=*pBdStart; i<=BMU_MAX_BD_INDEX_ALLOWED; i++, addr+=BMU_BD_SIZE)
    {
            *pBdEnd = addr;
            availBD++;
            //halLog(pMac, LOGW, "%s: addr 0x%x free. availBD %d \n", __FUNCTION__, *pBdEnd, availBD);
        if (availBD >= numOfBD)
            break;
    }

    /* Set BD & PDU idle list separately */
    if ( ((eeprom_bd == 0) && (eeprom_pdu == 0) && (numOfBD == numOfPDU)) ||
         ((eeprom_bd != 0) && (eeprom_pdu == 0) && (numOfBD < BMU_MAX_BD_INDEX_ALLOWED)) )
    {
        *pPduStart = *pBdStart;
        *pPduEnd = *pBdEnd;
        *bdPduExchangeFlag = BD_PDU_INTERCHANGEABLE;
        availBD = availPDU = numOfBD;
    }
    else
    {
        availPDU = 0;
        *pPduStart = *pBdEnd + BMU_BD_SIZE;             // ptr to PDU 0
        for (addr=*pPduStart; addr<=(internal+ external); addr+=BMU_BD_SIZE)
        {
                *pPduEnd = addr;
                availPDU++;
                //halLog(pMac, LOGW, "%s: addr 0x%x free. availPDU %d \n", __FUNCTION__, *pPduEnd, availPDU);
            if (availPDU >= (numOfBD + numOfPDU - availBD))
                break;
        }
    }

    HALLOGW( halLog(pMac, LOGW, FL("%s: Actual Available BD: %d,  pBdStart-->0x%x,  pBdEnd-->0x%x \n"),
           __FUNCTION__, availBD, *pBdStart, *pBdEnd));

    HALLOGW( halLog(pMac, LOGW, FL("%s: Actual Available PDU: %d, pPduStart-->0x%x, pPduEnd-->0x%x \n"),
           __FUNCTION__, availPDU, *pPduStart, *pPduEnd));

    HALLOGW( halLog(pMac, LOGW, FL("%s: BD and PDU %s used interchangeably \n"),
           __FUNCTION__, ((*bdPduExchangeFlag) ? "is" : "are NOT") ));

}


/* -----------------------------------------------------
 * FUNCTION: bmu_init_control_register()
 *
 * NOTE:
 *   Set the  max_bd_index and max_pdu_index value into
 *   the BMU control register.
 *------------------------------------------------------
 */
static void
bmu_init_control_register(
    tpAniSirGlobal pMac,
    tANI_U32       maxBdIndex,
    tANI_U32       pduTailIndex)
{
    tANI_U32   value;

    HALLOGW( halLog( pMac, LOGW, FL("%s: maxBdIndex = %d,  pduTailIndex = %d \n"),  __FUNCTION__, maxBdIndex, pduTailIndex ));

    value = (pduTailIndex << QWLAN_BMU_CONTROL_MAX_PDU_INDEX_NR_OFFSET) |
            (maxBdIndex << QWLAN_BMU_CONTROL_MAX_BD_INDEX_NR_OFFSET);

    halWriteRegister(pMac, QWLAN_BMU_CONTROL_REG, value) ;

 }


/* --------------------------------------------------------------
 * FUNCTION:  bmu_init_wq2_to_wq24()
 *
 * NOTE:
 *   Initialize WQ2 to WQ24's Head/Tail/NR to 0.
 *
 *   Writting head/tail/nr command will fail if the WQ is not
 *   enabled. This is reported in  bug13009. Basically, HW checks
 *   if the WQ to which a command is destined has been enabled.
 *   If WQ is not enabled, then BMU will generate an error
 *   interrupt status of 0x80000004 (ie. Invalid WQ error).
 * --------------------------------------------------------------
 */
static eHalStatus
bmu_init_wq2_to_wq26(
    tpAniSirGlobal pMac)
{
    tANI_U32  wqIndex;

    if (halBmu_enableWq(pMac, 2, 26) != eHAL_STATUS_SUCCESS)
        return eHAL_STATUS_FAILURE;

    for(wqIndex=2; wqIndex <= 26; wqIndex++)
    {
        bmuCommand_write_wq_head(pMac, wqIndex, 0);

        bmuCommand_write_wq_tail(pMac, wqIndex, 0);

        bmuCommand_write_wq_nr(pMac, wqIndex, 0);
    }

    if (halBmu_disableWq(pMac, 2, 26) != eHAL_STATUS_SUCCESS)
        return eHAL_STATUS_FAILURE;

    return eHAL_STATUS_SUCCESS;
}


/* -------------------------------------------------------
 * FUNCTION: bmu_init_error_interrupt()
 *
 * NOTE:
 *    - Check the interrupt status register before declaring
 *      that initialization is successful. If the register
 *      returns a nonzero value, that means something went
 *      wrong during the BMU initialization, and therefore,
 *      we should no proceed any further.
 *    - Clear all pending inetrrupt in the BMU "Error Interrupt
 *      Status" register. (According to register spec, writting
 *      1'b1 to the bits clears the error status bits)
 *    - Enable all error interrupt bits.
 * -------------------------------------------------------
 */
static eHalStatus
bmu_init_error_interrupt(
    tpAniSirGlobal pMac)
{
    tANI_U32    readValue, value;

    readValue=0; // to fix linux compile error, remove when below code enabled


    halReadRegister(pMac, QWLAN_BMU_ERR_INTR_STATUS_REG, &readValue);


    if (readValue != 0)
    {
        HALLOGE( halLog( pMac, LOGE, FL("bmu_init_error_interrupt(): error status 0x%x "),  readValue ));
        return eHAL_STATUS_FAILURE;
    }

    value = QWLAN_BMU_ERR_INTR_EN_DEFAULT_MASK;


    // At this point, we're ready to handle BMU interrupts. Enable all.
    halWriteRegister(pMac, QWLAN_BMU_ERR_INTR_ENABLE_REG, value);

    return eHAL_STATUS_SUCCESS;
}


/* -----------------------------------------------------------
 * FUNCTION:  bmu_init_bd_pdu_threshold()
 *
 * NOTE:
 *   BD and PDU thresholds can be programmed on a per master
 *   and/or DXE channel basis.  The threshold value controls
 *   the bd_pdu_available[n] signal the corresponding module.
 *   If the available free BDs or PDUs are less then the
 *   threshold value, the bd_pdu_available[n] signal to its
 *   module will not be set, causing that master to stall.
 *   Setting threshold value to 0 means don't care.
 *
 *   Following threshold are taken from the Programer's Guide:
 *   RXP Threshold(0):    BD=20, PDU=20
 *   DPU Threshold(1):    BD=0,  PDU=0
 *   DXE Threshold(2-10): BD=40, PDU=60
 * -----------------------------------------------------------
 */
static void
bmu_init_bd_pdu_threshold(
    tpAniSirGlobal  pMac,
    tANI_U32        intMem)
{
    tANI_U32   i;
    tpBdPduThr tInfo = &thrInfo[0];
    // fill all the threshold registers
    for (i=0; i < (sizeof(thrInfo)/sizeof(thrInfo[0])); i++, tInfo++)
    {
        tANI_U32 value, bdthr;
        value   = (intMem == BD_PDU_INTERCHANGEABLE) ? tInfo->pduThrIntMem : tInfo->pduThrExtMem;
        bdthr   = (intMem == BD_PDU_INTERCHANGEABLE) ? tInfo->bdThrIntMem : tInfo->bdThrExtMem;
        value <<= QWLAN_BMU_BD_PDU_THRESHOLD0_PDU_THRESHOLD_0_OFFSET;
        value  |= bdthr;
        halWriteRegister(pMac, tInfo->thrRegAddr, value);

    }
}

/* --------------------------------------------------------
 * FUNCTION:  halBmu_enableWq()
 *
 * NOTE:
 *   Enable the workqueues by setting 1 to the corresponding
 *   bits in the BMU WQ_ENABLE register.
 *      WQ 0:     Assigned for Idle BD.
 *      WQ 1:     Assigned for Idle PDU
 *      WQ 2:     Assigned for mCPU RX
 *      WQ 3:     Can be assigned to DPU, DXE or mCPU
 *      WQ 4-10:  Can be assigned to DPU, DXE or mCPU
 *      WQ 11-17: Can be assigned to DXE or mCPU
 *      WQ 18-24: Can be assigned to mCPU
 * ---------------------------------------------------------
 */
eHalStatus halBmu_enableWq(tpAniSirGlobal pMac, tANI_U32 startWqIndex,  tANI_U32 lastWqIndex)
{
    tANI_U32   value, mask;

    if ((startWqIndex >= BMUWQ_NUM) || (lastWqIndex >= BMUWQ_NUM))
        return eHAL_STATUS_FAILURE;

    halReadRegister(pMac, QWLAN_BMU_WQ_ENABLE_REG, &value);

    mask = ((1 << (lastWqIndex - startWqIndex + 1)) - 1) << startWqIndex;

    halWriteRegister(pMac, QWLAN_BMU_WQ_ENABLE_REG, (value | mask));

    return eHAL_STATUS_SUCCESS;
}


/* ---------------------------------------
 * FUNCTION:  halBmu_disableWq()
 *
 * NOTE:
 *   Disable the workqueues by setting 0
 *   to the corresponding bits in the
 *   BMU WQ_ENABLE register.
 * ---------------------------------------
 */
eHalStatus halBmu_disableWq(tpAniSirGlobal pMac, tANI_U32 startWqIndex,  tANI_U32 lastWqIndex)
{
    tANI_U32   value, mask;

    if ((startWqIndex >= BMUWQ_NUM) || (lastWqIndex >= BMUWQ_NUM))
        return eHAL_STATUS_FAILURE;

    halReadRegister(pMac, QWLAN_BMU_WQ_ENABLE_REG, &value);

    mask = ((1 << (lastWqIndex - startWqIndex + 1)) - 1) << startWqIndex;

    halWriteRegister(pMac, QWLAN_BMU_WQ_ENABLE_REG, (value & ~mask));

    return eHAL_STATUS_SUCCESS;
}


/* ---------------------------------------------------
 * FUNCTION:  halBmu_InitBdPduBaseAddress()
 *
 * NOTE:
 *   Program the starting memory address of the BD and PDU
 *   buffers. This is essentially the starting address of the
 *   packet memory.
 *   Note: The lowest 8 bit must be zero. Hence, need
 *   to mask with 0xFFFFFF80
 * ---------------------------------------------------
 */
eHalStatus halBmu_InitBdPduBaseAddress(tpAniSirGlobal pMac)
{
    tANI_U32   address = pMac->hal.memMap.packetMemory_offset;

    address &= QWLAN_MCU_BD_PDU_BASE_ADDR_BD_PDU_BASE_ADDRESS_MASK;
    halWriteRegister(pMac, QWLAN_MCU_BD_PDU_BASE_ADDR_REG, address);

    return eHAL_STATUS_SUCCESS;
}


static eHalStatus bmu_fast_init_wq0_bd_idle_list(tpAniSirGlobal pMac,
                                                tANI_U32        start,          // start addr of BD 1
                                                tANI_U32        end,            // start addr of last BD
                                                tANI_U32 startIndex, tANI_U32 *tailBdIndex)
{
    tANI_U32 addr, prevBdIndex, curBdIndex;
    tANI_U32    availBDs = 0, availBDs1 = 0, loop = 0, fastSetStatus;

    *tailBdIndex = 0;
    prevBdIndex = 0;
    curBdIndex = startIndex;

    for (addr = start; (addr <= end); addr+=BMU_BD_SIZE)
    {
            prevBdIndex = curBdIndex;
                    availBDs++;

            curBdIndex ++;

            /**
                BMU on chip sram can support only up to max BD index 1023.
            If we have reached the max BD index, then we're done
            initializing BD idle list.
        */
        if (curBdIndex >= BMU_MAX_BD_INDEX_ALLOWED) {
            break;
        }
    }
    *tailBdIndex = prevBdIndex; // curBdIndex now points to the first PDU

    /** Setup the BD list */
    halWriteRegister(pMac, QWLAN_BMU_FAST_BD_LINK_SETUP_CONTROL_REG,
                                startIndex | (*tailBdIndex << QWLAN_BMU_FAST_BD_LINK_SETUP_CONTROL_BD_END_INDEX_OFFSET) |
                                QWLAN_BMU_FAST_BD_LINK_SETUP_CONTROL_CREATE_BD_LINKED_LIST_MASK);
    //check maximum BMU_REG_MAX_POLLING times whether the status bit for fast setup has been cleared or not.
    do
    {
        halReadRegister(pMac, QWLAN_BMU_FAST_BD_LINK_SETUP_CONTROL_REG, &fastSetStatus);
    }while((loop++ <= BMU_REG_MAX_POLLING) && (fastSetStatus & QWLAN_BMU_FAST_BD_LINK_SETUP_CONTROL_BD_LINK_LIST_CREATION_STATUS_MASK));

#ifdef DYNAMIC_RECORD
    // Issue a wait command into the ADU register list
    halRegBckup_WriteWaitCmd(pMac, 
            &pMac->hal.RegBckupParam.regListCurrAddr, 
            HAL_REG_REINIT_BMU_WAIT_CYCLES );
#endif

    /** Set the last BD index point to NULL */
    bmuCommand_write_bd_pointer(pMac, *tailBdIndex, 0);

    HALLOGW( halLog( pMac, LOGW, FL("%s: BMU WQ0 fast setup polled %d time, total %d BDs configured.\n"),  __FUNCTION__, loop, *tailBdIndex ));

    /** Get the total available BDs */
    halReadRegister(pMac, QWLAN_BMU_AVAILABLE_BD_PDU_AFTER_RSV_REG, &availBDs1); 

    availBDs1 &= 0x0000FFFF;

    HALLOGW( halLog( pMac, LOGW, FL(": Register BMU_AVAILABLE_BD_PDU_AFTER_RSV_REG Read Available BDs = %d \n"),  availBDs1 ));

    /** Updating the BMU WQ Head/Tail/NR commands */
    bmu_set_wq_head_tail_nr(pMac, BMUWQ_BMU_IDLE_BD, startIndex,
                                *tailBdIndex, availBDs);


    return eHAL_STATUS_SUCCESS;

}


/* -------------------------------------------------------
 * BMU COMMAND:  POP/PUSH WQ
 *
 *   The "PUSH WQ" command adds a BD to a specified WQ.
 *   It is a write command. We shall never write a BD
 *   index of 0 in the push wq command.
 *
 *   The "POP WQ" command removes a BD from a specified WQ.
 *   It is a read command. BMU will return the index of the
 *   BD that was removed from a WQ. If BMU returns a BD
 *   index of 0, that means the WQ is empty.
 * -------------------------------------------------------
 */
void bmuCommand_push_wq(tpAniSirGlobal pMac, tANI_U32 wqIndex, tANI_U32 value)
{
    halWriteRegister(pMac, (BMU_COMMAND_BASE_ADDRESS | (wqIndex << 8) | PUSH_WQ_CMDTYPE),  value);
}

void bmuCommand_pop_wq(tpAniSirGlobal pMac, tANI_U32 wqIndex, tANI_U32 *readValue)
{
    halReadRegister(pMac, (BMU_COMMAND_BASE_ADDRESS | (wqIndex << 8) | POP_WQ_CMDTYPE), readValue) ;

}

/* -------------------------------------------------------
 * BMU COMMAND:  WQ HEAD/Tail/NR
 *
 *   The address format is:
 *       31     23       15        7         0
 *       ------------------------------------
 *      | Rsvd  |Rsvd =0 |WQ index |CMD Type |
 *       ------------------------------------
 *
 *   The read/write data structure format is:
 *       31              15        7         0
 *       -----------------------------------------
 *      |  Reserved = 0  | BD index or NR entries |
 *       -----------------------------------------
 * -------------------------------------------------------
 */
void bmuCommand_write_wq_head(tpAniSirGlobal pMac, tANI_U32 wqIndex, tANI_U32 value)
{
    halWriteRegister(pMac,
                         (BMU_COMMAND_BASE_ADDRESS | (wqIndex << 8) | WRITE_WQ_HEAD_CMDTYPE),
                         value) ;

}

void bmuCommand_read_wq_head(tpAniSirGlobal pMac, tANI_U32 wqIndex, tANI_U32 *readValue)
{
    halReadRegister(pMac,
                        (BMU_COMMAND_BASE_ADDRESS | (wqIndex << 8) | READ_WQ_HEAD_CMDTYPE),
                        readValue) ;
}

void bmuCommand_write_wq_tail(tpAniSirGlobal pMac, tANI_U32 wqIndex, tANI_U32 value)
{
    halWriteRegister(pMac,
                         (BMU_COMMAND_BASE_ADDRESS | (wqIndex << 8) | WRITE_WQ_TAIL_CMDTYPE),
                         value) ;
}

void bmuCommand_read_wq_tail(tpAniSirGlobal pMac, tANI_U32 wqIndex, tANI_U32 *readValue)
{
    halReadRegister(pMac,
                        (BMU_COMMAND_BASE_ADDRESS | (wqIndex << 8) | READ_WQ_TAIL_CMDTYPE),
                        readValue);
}
void bmuCommand_write_wq_nr(tpAniSirGlobal pMac, tANI_U32 wqIndex, tANI_U32 value)
{
    halWriteRegister(pMac,
                         (BMU_COMMAND_BASE_ADDRESS | (wqIndex << 8) | WRITE_WQ_NR_CMDTYPE),
                         value) ;

}

void bmuCommand_read_wq_nr(tpAniSirGlobal pMac, tANI_U32 wqIndex, tANI_U32 *readValue)
{
    halReadRegister(pMac,
                        (BMU_COMMAND_BASE_ADDRESS | (wqIndex << 8) | READ_WQ_NR_CMDTYPE),
                        readValue);

}

/* -------------------------------------------------------
 * BMU COMMAND:  READ/WRITE BD POINTER
 *
 *   The command address format is:
 *       31        23        7         0
 *       -------------------------------
 *      |   Rsvd   |BD index |CMD Type |
 *       -------------------------------
 *
 *   The read/write data structure are as following:
 *       31              15          0
 *       -----------------------------
 *      |   Reserved     | BD pointer |
 *       -----------------------------
 *
 *   BD pointer: The BD index of the BD to which the
 *               BD (as specified in the command address
 *               field) is linked.
 * ----------------------------------------------------
 */
void bmuCommand_write_bd_pointer(tpAniSirGlobal pMac, tANI_U32 bdIndex, tANI_U32 value)
{
    halWriteRegister(pMac,
                         (BMU_COMMAND_BASE_ADDRESS | (bdIndex << 8) | WRITE_BD_POINTER_CMDTYPE),
                         value);


}

void bmuCommand_read_bd_pointer(tpAniSirGlobal pMac, tANI_U32 bdIndex, tANI_U32 *pValue)
{
    halReadRegister(pMac,
                        (BMU_COMMAND_BASE_ADDRESS | (bdIndex << 8) | READ_BD_POINTER_CMDTYPE),
                        pValue);

}

/* -------------------------------------------------------
 * BMU COMMAND:  BD/PDU RESERVATION REQUEST
 *
 *   A module can request the BMU to reserve a number of BDs
 *   and PDUs for that module.
 *
 *   It is a read command, and user must specify:
 *     pdu:  Number of PDUs the module is requesting to reserve
 *     bd:   Number of BDs the module is requesting to reserve
 *     module: The index of hte module reserving the BD and/or PDUs
 *   BMU will return either:
 *     0: command successful
 *     1: The requested number of BD/PDU are not available.
 *
 *   If the module ends up not using all of those reserved
 *   BDs and PDUs, it may wish to release the remainder.
 *   This can be done reading this command again, but this
 *   time, set the "number of PDU" and "number of BD" requested
 *   to 0.
 * -------------------------------------------------------
 */
eHalStatus bmuCommand_request_bd_pdu(tpAniSirGlobal pMac, tANI_U32 pdu, tANI_U32 bd, tANI_U32 module, tANI_U32 *readValue)
{
    tANI_U32   address;

    address = BMU_COMMAND_BASE_ADDRESS|(pdu << 16)|(bd << 14)|(module << 8)|RESERVATION_REQUEST_BD_PDU_CMDTYPE;

    halReadRegister(pMac, address, readValue);


    return eHAL_STATUS_SUCCESS;
}

/* -------------------------------------------------------
 * BMU COMMAND:  GET IDLE BD/PDU
 *
 *   The command address format is:
 *    31     23    19     15   13      7        0
 *    -------------------------------------------
 *   | Rsvd |REQ 2| REQ 1|RSVD| MODULE| CMD TYPE |
 *    -------------------------------------------
 *
 *   The read/write data structure are as following:
 *    31                     15                      0
 *    ------------------------------------------------
 *   |BD/PDU Index for REQ 2 | BD/PDU Index for REQ 1 |
 *    ------------------------------------------------
 *
 *   It is a read command, and user must specify:
 *   1) Module Index: The index of the module getting the
 *      idle BDs and/or PDUs. This index allows the BMU
 *      to track how many BDs and PDUs have been given
 *      to that module in case that module had reserved
 *      BDs and PDUs. This command can also be used without
 *      reserving any BDs/PDUs in advance, but then its
 *      possible that the BD/PDUs are not going to be
 *      available.
 *   2) The encoding for REQ code is:
 *        0:  No BD/PDU requested
 *        1:  One BD requested
 *        2:  One PDU requested
 *        3-15: Reserved
 * -------------------------------------------------------
 */
eHalStatus bmuCommand_get_bd_pdu(tpAniSirGlobal pMac, tANI_U32 req1, tANI_U32 req2, tANI_U32 module, tANI_U32 *readValue)
{
    tANI_U32   address = BMU_COMMAND_BASE_ADDRESS|(req2<<20)|(req1<<16)|(module << 8)|GET_BD_PDU_CMDTYPE;

    halReadRegister(pMac, address, readValue);

    return eHAL_STATUS_SUCCESS;
}

/* -------------------------------------------------------
 * BMU COMMAND:  RELEASE BD
 *
 *   This is used to release BDs to their respective
 *   idle lists. It is a write command, and the command
 *   address format is:
 *    31     23        15   13      7        0
 *    ----------------------------------------
 *   | Rsvd |NUM of BD|RSVD| MODULE| CMD TYPE |
 *    ----------------------------------------
 *
 *   The write data structure are as following:
 *    31              15           0
 *    --------------------------------
 *   | Tail BD Index | Head BD Index  |
 *    --------------------------------
 *
 *   When the "number of BDs" is 1, the "tail bd index"
 *   field should contain the same index as the
 *   "head bd index"
 * -------------------------------------------------------
 */
eHalStatus bmuCommand_release_bd(tpAniSirGlobal pMac, tANI_U32 bd, tANI_U32 module, tANI_U32 head, tANI_U32 tail)
{
    tANI_U32   address = BMU_COMMAND_BASE_ADDRESS|(bd<<16)|(module << 8)|RELEASE_BD_CMDTYPE;

    if ((bd == 1) && (head != tail))
        return eHAL_STATUS_FAILURE;

    halWriteRegister(pMac, address, ((tail << 16) | head));

    return eHAL_STATUS_SUCCESS;
}

/* ---------------------------------------
 * BMU COMMAND:  RELEASE PDU
 *
 *  Similar to release pdu command above.
 * ---------------------------------------
 */
eHalStatus bmuCommand_release_pdu(tpAniSirGlobal pMac, tANI_U32 pdu, tANI_U32 module, tANI_U32 head, tANI_U32 tail)
{
    tANI_U32   address = (BMU_COMMAND_BASE_ADDRESS|(pdu<<16)|(module << 8)|RELEASE_PDU_CMDTYPE);

    if ((pdu == 1) && (head != tail))
        return eHAL_STATUS_FAILURE;

    halWriteRegister(pMac, address, ((tail << 16) | head));

    return eHAL_STATUS_SUCCESS;
}



/* ------------------------------------------------
 * FUNCTION: bmu_set_bdpdu_threshold()
 *
 * NOTE:
 *   Sets the BD and PDU threshold value in the
 *   BD_PDU_threshold[x] registers.
 * ------------------------------------------------
 */
eHalStatus bmuReg_set_bdpdu_threshold(tpAniSirGlobal pMac, tANI_U32 index, tANI_U32 bdThresh, tANI_U32 pduThresh)
{
    tANI_U32  address = QWLAN_BMU_BD_PDU_THRESHOLD0_REG + (index * 4);
    tANI_U32  value = bdThresh | (pduThresh << QWLAN_BMU_BD_PDU_THRESHOLD1_PDU_THRESHOLD_1_OFFSET);

    if (index > 10)
        return eHAL_STATUS_FAILURE;

    halWriteRegister(pMac, address, value);

    return eHAL_STATUS_SUCCESS;
}


/* ----------------------------------------------------
 * FUNCTION: halBmuTrace_setMaxIndex()
 *
 * NOTE:
 *   When the internal BMU memory tracing functionality
 *   is enabled, one can specify the max memory index
 *   that can be used to store trace information.
 * ----------------------------------------------------
 */
eHalStatus halBmuTrace_setMaxIndex(tpAniSirGlobal pMac, tANI_U32 index)
{
    tANI_U32    value; //, maxIndex;

    halReadRegister(pMac, QWLAN_BMU_CONTROL2_REG, &value);

#if 0 //FIXME_NO_VIRGO
    //FIXME: Libra doesn't have these register control bits. Need to figure out how to set the
    //       BMU trace indices.
    maxIndex = ( (index << BMU_CONTROL2_BMU_MEM_TRACE_MAX_INDEX_OFFSET) &
                 BMU_CONTROL2_BMU_MEM_TRACE_MAX_INDEX_MASK);
    value = (value & ~BMU_CONTROL2_BMU_MEM_TRACE_MAX_INDEX_MASK) | maxIndex |  QWLAN_BMU_CONTROL2_AMSDU_RELEASE_SUPPORT_ENABLE_MASK;

    if (halWriteRegister(pMac, BMU_CONTROL2_REG, value) != eHAL_STATUS_SUCCESS)
        return eHAL_STATUS_FAILURE;
#endif
    return eHAL_STATUS_FAILURE;
}


/* --------------------------------------
 * FUNCTION: halBmuTrace_printConfig()
 *
 * NOTE:
 *   Print trace configuration settings.
 * --------------------------------------
 */
eHalStatus halBmuTrace_printConfig(tpAniSirGlobal pMac)
{
#if 0 //FIXME_NO_VIRGO
    //FIXME: Libra doesn't have these register control bits. Need to figure out how to set the
    //       BMU trace indices.

    tANI_U32    readValue, value;

    halReadRegister(pMac, BMU_CONTROL2_REG, &readValue);


    halLog(pMac, LOGW, "-----------------\n");
    halLog(pMac, LOGW, "    BMU TRACE    \n");
    halLog(pMac, LOGW, "-----------------\n");

    value = (readValue & BMU_CONTROL2_BMU_MEM_TRACE_MAX_INDEX_MASK) >>
            BMU_CONTROL2_BMU_MEM_TRACE_MAX_INDEX_OFFSET;
    halLog(pMac, LOGW, "BMU TRACE: max memory index %d \n", value);

    value = readValue & BMU_CONTROL2_BMU_MEM_RELEASE_TRACE_ENABLE_MASK;
    halLog(pMac, LOGW, "BMU TRACE: Release commands - %s \n",( (value) ? "enabled" : "disabled") );

    value = readValue & BMU_CONTROL2_BMU_MEM_RES_GET_TRACE_ENABLE_MASK;
    halLog(pMac, LOGW, "BMU TRACE: Reservation & Get commands - %s \n",( (value) ? "enabled" : "disabled") );

    value = readValue & BMU_CONTROL2_BMU_MEM_POP_PUSH_TRACE_ENABLE_MASK;
    halLog(pMac, LOGW, "BMU TRACE: Pop and Push commands - %s \n",( (value) ? "enabled" : "disabled") );
#endif
    return eHAL_STATUS_FAILURE;
}


/* ---------------------------------------
 * FUNCTION: bmu_set_wq_head_tail_nr()
 *
 * NOTE:
 *   Print trace configuration settings.
 * ---------------------------------------
 */
void 
bmu_set_wq_head_tail_nr(tpAniSirGlobal pMac, tANI_U32 wqIndex, tANI_U32 head, tANI_U32 tail, tANI_U32 nr)
{
    bmuCommand_write_wq_head(pMac, wqIndex, head);

    bmuCommand_write_wq_tail(pMac, wqIndex, tail);

    bmuCommand_write_wq_nr(pMac, wqIndex, nr) ;

    HALLOGW( halLog(pMac, LOGW, FL(" %s Head: %d, Tail: %d, NR: %d \n"),
            ((wqIndex) ? "PDU" : "BD"), head, tail, nr));

}


static eHalStatus halBmu_tx_queue_staid_qid_config(tpAniSirGlobal pMac,  tANI_U32 maxStaid, tANI_U32 maxQid, tANI_U32 maxBssStaIds)
{
    tSystemRole    system_role;
    tANI_U32    value = 0;

    system_role = halGetSystemRole(pMac);

    if (eSYSTEM_STA_ROLE == system_role) {

        if (halWriteRegister(pMac, QWLAN_BMU_TX_QUEUE_STAID_QUEUEID_CONFIG_REG,
                (maxStaid << QWLAN_BMU_TX_QUEUE_STAID_QUEUEID_CONFIG_MAX_VALID_STAID_OFFSET) |
                (maxQid << QWLAN_BMU_TX_QUEUE_STAID_QUEUEID_CONFIG_MAX_VALID_QUEUEID_OFFSET))
                != eHAL_STATUS_SUCCESS)
            return eHAL_STATUS_FAILURE;
    } else if (eSYSTEM_AP_ROLE | eSYSTEM_STA_IN_IBSS_ROLE) {

        if (halWriteRegister(pMac, QWLAN_BMU_TX_QUEUE_STAID_QUEUEID_CONFIG_REG,
                (maxStaid << QWLAN_BMU_TX_QUEUE_STAID_QUEUEID_CONFIG_MAX_VALID_STAID_OFFSET) |
                (maxQid << QWLAN_BMU_TX_QUEUE_STAID_QUEUEID_CONFIG_MAX_VALID_QUEUEID_OFFSET)|
                (maxBssStaIds << QWLAN_BMU_TX_QUEUE_STAID_QUEUEID_CONFIG_NR_OF_BEACON_STAIDS_OFFSET))
                != eHAL_STATUS_SUCCESS)
            return eHAL_STATUS_FAILURE;
    }

    halReadRegister(pMac, QWLAN_BMU_TX_QUEUE_STAID_QUEUEID_CONFIG_REG, &value);

    HALLOGW( halLog( pMac, LOGW, FL("HAL Tx Qid STAId Config 0x%x\n"),  value ));

    return eHAL_STATUS_SUCCESS;

}

/**
 *    Set Tx WQ Base address
 */
static eHalStatus halBmu_btqm_tx_wq_base_addr(tpAniSirGlobal pMac,  tANI_U32 tx_wq_addr)
{
    HALLOGW( tANI_U32 value = 0);

    /** Zero out the BTQM_TX_WQ descriptor */
    halZeroDeviceMemory(pMac, pMac->hal.memMap.btqmTxQueue_offset,
                                pMac->hal.memMap.btqmTxQueue_size) ;

    HALLOGW( halLog( pMac, LOGW, FL("HAL Tx tx_wq_addr 0x%x\n"),  tx_wq_addr ));

    halWriteRegister(pMac, QWLAN_BMU_BTQM_QUEUE_INFO_BASE_ADDR_REG,
            pMac->hal.memMap.btqmTxQueue_offset );

    HALLOGW(halReadRegister(pMac, QWLAN_BMU_BTQM_QUEUE_INFO_BASE_ADDR_REG, &value));

    HALLOGW( halLog( pMac, LOGW, FL("HAL Tx Wq Base Addr 0x%x\n"),  value ));

    return eHAL_STATUS_SUCCESS;

}

/**
 *    Assign Backoff to Queue IDs
 */
static eHalStatus bmu_init_queueid_bo_mapping(tpAniSirGlobal pMac)
{
    tANI_U32 queueId_0to7 = 0, queueId_8to10 = 0, qidIndex, bitOffset;

    for(qidIndex = BTQM_QID0, bitOffset = QWLAN_BMU_QUEUEID_BO_MAPPING1_BO_FOR_QUEUEID0_OFFSET;
        bitOffset <= QWLAN_BMU_QUEUEID_BO_MAPPING1_BO_FOR_QUEUEID7_OFFSET;
        bitOffset += QWLAN_BMU_QUEUEID_BO_MAPPING1_BO_FOR_QUEUEID1_OFFSET, qidIndex ++){
        queueId_0to7 |= (btqmQIdMapping[qidIndex].bkId << bitOffset);
     }

    for(bitOffset = QWLAN_BMU_QUEUEID_BO_MAPPING2_BO_FOR_QUEUEID8_OFFSET;
        bitOffset <= QWLAN_BMU_QUEUEID_BO_MAPPING2_BO_FOR_QUEUEID10_OFFSET;
        bitOffset += QWLAN_BMU_QUEUEID_BO_MAPPING1_BO_FOR_QUEUEID1_OFFSET, qidIndex ++){
        queueId_8to10 |= (btqmQIdMapping[qidIndex].bkId << bitOffset);
     }

    halWriteRegister(pMac, QWLAN_BMU_QUEUEID_BO_MAPPING1_REG, queueId_0to7);
    halWriteRegister(pMac, QWLAN_BMU_QUEUEID_BO_MAPPING2_REG, queueId_8to10);

    return eHAL_STATUS_SUCCESS;
}

/* ------------------------------------------------
 * FUNCTION: halBmu_queueid_qos_map()
 *
 * NOTE:
 *   Initializes which queues contain Qos frames
 *   Initialize the TID values for the appropriate QID
 *   Only Qos enabled queues requires a TID setting.
 * ------------------------------------------------
 */
static eHalStatus halBmu_queueid_qos_map(tpAniSirGlobal pMac)
{
    tANI_U32 qosQueueBitMask = 0;
    tANI_U32 queueId_0to7 = 0;
    tANI_U32 qidIndex, bitOffset;

    for(qidIndex = BTQM_QID0, bitOffset = QWLAN_BMU_QOS_QUEUEID_MAPPING2_TID_FOR_QUEUEID0_OFFSET;
        bitOffset <= QWLAN_BMU_QOS_QUEUEID_MAPPING2_TID_FOR_QUEUEID7_OFFSET;
        bitOffset += QWLAN_BMU_QOS_QUEUEID_MAPPING2_TID_FOR_QUEUEID1_OFFSET, qidIndex ++){
        qosQueueBitMask |= (1<< btqmQIdMapping[qidIndex].tid);
        queueId_0to7 |= (btqmQIdMapping[qidIndex].tid << bitOffset);
     }

    halWriteRegister(pMac, QWLAN_BMU_QOS_QUEUEID_MAPPING1_REG, qosQueueBitMask);
    halWriteRegister(pMac, QWLAN_BMU_QOS_QUEUEID_MAPPING2_REG, queueId_0to7);
    return eHAL_STATUS_SUCCESS;
}

eHalStatus halBmu_get_qid_for_qos_tid(tpAniSirGlobal pMac, tANI_U8 tid, tANI_U8 *qid)
{
    if (tid > BTQM_QUEUE_TX_TID_7 )
        return eHAL_STATUS_FAILURE;
        
    *qid = btqmQosTid2QidMapping[tid];
    return eHAL_STATUS_SUCCESS;
}


eHalStatus halBmu_InitStaMemory(tpAniSirGlobal pMac)
{
    tANI_U32 value;

    value = QWLAN_BMU_BTQM_CONTROL1_FAST_STA_MEM_INIT_MASK;
    halWriteRegister(pMac, QWLAN_BMU_BTQM_CONTROL1_REG,
                value );
#ifdef DYNAMIC_RECORD
    // Issue a wait command into the ADU register list
    halRegBckup_WriteWaitCmd(pMac, 
            &pMac->hal.RegBckupParam.regListCurrAddr, 
            HAL_REG_REINIT_BMU_CONTROL1_WAIT_CYCLES );
#endif


    HALLOGW( halReadRegister(pMac, QWLAN_BMU_BTQM_CONTROL1_REG, &value));

    HALLOGW( halLog( pMac, LOGW, FL("HAL BTQM Control1 0x%x\n"),  value ));

    value = 0;

    do {
       halReadRegister(pMac, QWLAN_BMU_BTQM_CONTROL1_REG, &value);
        HALLOGW( halLog( pMac, LOGW, FL("HAL BTQM Control1 in loop 0x%x\n"),  value ));

    } while ((value & QWLAN_BMU_BTQM_CONTROL1_FAST_STAMEM_INIT_STATUS_MASK));


    HALLOGW( halLog( pMac, LOGW, FL("HAL BTQM Control1 0x%x\n"),  value ));

    value &= ~QWLAN_BMU_BTQM_CONTROL1_FAST_STA_MEM_INIT_MASK;


    halWriteRegister(pMac, QWLAN_BMU_BTQM_CONTROL1_REG,
                value) ;
#ifdef DYNAMIC_RECORD

    // Issue a wait command into the ADU register list
    halRegBckup_WriteWaitCmd(pMac, 
            &pMac->hal.RegBckupParam.regListCurrAddr, 
            HAL_REG_REINIT_BMU_CONTROL1_WAIT_CYCLES );
#endif


    HALLOGW( halReadRegister(pMac, QWLAN_BMU_BTQM_CONTROL1_REG, &value));

    HALLOGW( halLog( pMac, LOGW, FL("HAL BTQM Control1 0x%x\n"),  value ));

    return eHAL_STATUS_SUCCESS;
}

/* ------------------------------------------------
 * FUNCTION: halBmu_btqm_init_control2_register()
 *
 * NOTE:
 *   Indicates which queues are enabled for transmission
 *
 * ------------------------------------------------
 */
static void halBmu_btqm_init_control2_register(tpAniSirGlobal pMac, tANI_U32 txQueueIdsMask)
{
    tANI_U32 value;

    halReadRegister(pMac, QWLAN_BMU_BTQM_CONTROL2_REG, &value);

	// LIBRA2.0 HW bug fix: CR-0000143098 & CR-0000142931
	// BMU: Dropping of frames with seq. number lower then BA SSN
	// BMU: misformed “null Block-Ack” handling
    if (halGetChipRevNum(pMac) == LIBRA_CHIP_REV_ID_2_0) {
		value |= QWLAN_BMU_BTQM_CONTROL2_CFG_BA_PARTIAL_BITMAP_UPDATE_MASK;
	}

    value |= (QWLAN_BMU_BTQM_CONTROL2_BTQM_QUEUEID_QUEUE_ENABLE_MASK & txQueueIdsMask);

    halWriteRegister(pMac, QWLAN_BMU_BTQM_CONTROL2_REG,
                value );

    HALLOGW(halReadRegister(pMac, QWLAN_BMU_BTQM_CONTROL2_REG, &value));

    HALLOGW( halLog( pMac, LOGW, FL("HAL BTQM Control2 0x%x\n"),  value ));

}

/* ------------------------------------------------
 * FUNCTION: halBmu_btqm_init_control1_register()
 *
 * NOTE:
 *   Enable/Diable BTQM Tx Queue features
 *
 * ------------------------------------------------
 */
static eHalStatus halBmu_btqm_init_control1_register(tpAniSirGlobal pMac, tANI_U32 ctrlCfg)
{
    tANI_U32 value;

    halReadRegister(pMac, QWLAN_BMU_BTQM_CONTROL1_REG, &value);


    value |= ctrlCfg;

    halWriteRegister(pMac, QWLAN_BMU_BTQM_CONTROL1_REG,
                value );

    HALLOGW( halReadRegister(pMac, QWLAN_BMU_BTQM_CONTROL1_REG, &value));

    HALLOGW( halLog( pMac, LOGW, FL("HAL BTQM Control1 0x%x\n"),  value ));

    return eHAL_STATUS_SUCCESS;

}

/* ------------------------------------------------
 * FUNCTION: halBmu_sta_enable_disable_control()
 *
 * NOTE:
 *   Enable/Diable STA control params
 *
 *    staId - STA id
 *    staCfgCmd    00 - Enable Tx Queue, Disable Transmission
 *                01 - Enable Tx Queue, Enable Transmission
 *                10 - Disable Tx Queue, Disable Transmission
 *                11 - Disable Tx Queue, Disable Transmission and cleanup any data left in Tx Queues
 * ------------------------------------------------
 */
eHalStatus halBmu_sta_enable_disable_control(tpAniSirGlobal pMac, tANI_U32 staId, tANI_U32 staCfgCmd)
{
    tANI_U32 value;


    /** Set the staId and configuration command */
    value = (staCfgCmd << QWLAN_BMU_BTQM_STA_ENABLE_DISABLE_CONTROL_STA_CONFIGURATION_COMMAND_OFFSET) |
        (staId << QWLAN_BMU_BTQM_STA_ENABLE_DISABLE_CONTROL_STAID_OFFSET);

    halWriteRegister(pMac, QWLAN_BMU_BTQM_STA_ENABLE_DISABLE_CONTROL_REG,
            value );

    /** Poll on bit 31 is clear */
    do {

        halReadRegister(pMac, QWLAN_BMU_BTQM_STA_ENABLE_DISABLE_CONTROL_REG, &value);

    } while ((value & QWLAN_BMU_BTQM_STA_ENABLE_DISABLE_CONTROL_STA_UPDATE_STATUS_MASK));

    (void)halTable_SetStaTxConfig(pMac, staId, staCfgCmd);

    return eHAL_STATUS_SUCCESS;
}

/**
 * @brief  : This Routine is a Interrupt handler for BMU error Interrupts
 *           This issues a mac reset if the error is FATAL, otherwise
 *           Displays the warning and maintains the stats for the error.
 *
 * @param  : hHalHandle - Mac Global Handle
 * @param  : intSource - Source for the paticular Interrupt.
 * @return : eHAL_STATUS_SUCCESS on Success and appropriate error sattus on error.
 */
eHalStatus halIntBMUErrorHandler(tHalHandle hHalHandle, eHalIntSources intSource)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tANI_U32 intRegMask;
    tANI_U32 intRegStatus;
    tANI_U32 intBMUErrAddr;
    tANI_U32 intBMUErrWData;
    tpAniSirGlobal pMac = PMAC_STRUCT(hHalHandle);

#ifdef BMU_ERR_DEBUG
    tANI_U8 buffer[128];
    tANI_U16 i = 0;
    tANI_U32 value = 0, address = 0, bmuCommand = 0;
    tANI_U32 regValue = 0;
#endif

    /** Read Interrupt Status.*/
    status = halIntGetErrorStatus(hHalHandle, intSource, &intRegStatus, &intRegMask);
    if (status != eHAL_STATUS_SUCCESS) {
        return status;
    }

    /** Read Error Interrupt Address.*/
    (void) halReadRegister(pMac, QWLAN_BMU_ERR_INT_ADDR_REG, &intBMUErrAddr);
    /** Read Error Interrupt WDATA.*/
    halReadRegister(pMac, QWLAN_BMU_ERR_INT_WDATA_REG, &intBMUErrWData);

    intRegStatus &= intRegMask;

   if(intRegStatus & QWLAN_BMU_ERR_INTR_STATUS_RELEASE_FIFO_FULL_WARNING_MASK){
        ++pMac->hal.halIntErrStats.halIntBmuFifoFull;
        HALLOG1( halLog( pMac, LOG1, FL("BMU Release FIFO full Warning Received %d times!!\n"),  pMac->hal.halIntErrStats.halIntBmuFifoFull ));
    }

    if(intRegStatus & QWLAN_BMU_ERR_INTR_STATUS_NOT_ENOUGH_BD_PDU_WARNING_MASK){
        ++pMac->hal.halIntErrStats.halIntBmuNoBdPdu;
        HALLOG1( halLog( pMac, LOG1, FL("BMU Not Enough PDU/BD Warning Received %d times!!\n"),  pMac->hal.halIntErrStats.halIntBmuNoBdPdu ));
    }

    intRegStatus &= ~(QWLAN_BMU_ERR_INTR_STATUS_RELEASE_FIFO_FULL_WARNING_MASK|
                      QWLAN_BMU_ERR_INTR_STATUS_NOT_ENOUGH_BD_PDU_WARNING_MASK);

    if(intRegStatus){
        /** Display Read Error Information.*/
        HALLOGE( halLog(pMac, LOGE, FL("BMU FATAL Error Interrupt Status %x, enable %x, \
                Address %x, WData %x\n"), intRegStatus, intRegMask,
                intBMUErrAddr, intBMUErrWData));

#ifdef BMU_ERR_DEBUG
        halReadRegister(pMac, QWLAN_BMU_BTQM_ERR_STATUS_REG, &value);
        HALLOGE(halLog(pMac, LOGE, FL("BTQM Err = 0x%08x"), value));

        // Dump the BD that caused the BMU error
            halReadRegister(pMac, QWLAN_MCU_BD_PDU_BASE_ADDR_REG, &value);
            address = value + (intBMUErrWData * 128);
            palZeroMemory(pMac->hHdd, (void*)&buffer, 128);
            halReadDeviceMemory(pMac, address, &buffer[0], 128);

        halReadRegister(pMac, 0x0f00001c, &regValue);
        HALLOGE( halLog(pMac, LOGE, FL("Num of BDs in WQ0 = %x"), regValue));

        // Print the number of BDs in all the WQs
        for(i=0; i<=26; i++) {
            bmuCommand = 0x0f00001c;
            bmuCommand |= i << 8;
            halReadRegister(pMac, bmuCommand, &regValue);
            HALLOGE( halLog(pMac, LOGE, FL("Num of BDs in WQ%d = %x (Cmd = 0x%08x)"), i, regValue, bmuCommand));
        }

        HALLOGE( halLog(pMac, LOGE, FL("Address = 0x%08x, BD = %d, Loc = 0x%08x"), value, intBMUErrWData, address));
        for(i=0; i<128; i+=16) {
                tANI_U32 *pU32 = (tANI_U32 *) &(buffer[i]);
            HALLOGE( halLog(pMac, LOGE, FL("%3d(%2x) %08x %08x %08x %08x\n"), pU32[0], pU32[1], pU32[2], pU32[3])); 
        }
#endif
        macSysResetReq(pMac, eSIR_BMU_EXCEPTION);
        return status;
    }


    return (eHAL_STATUS_SUCCESS);
}

static eHalStatus halBmu_InitControl2Reg(tpAniSirGlobal pMac, tANI_U32 mask)
{
    tANI_U32 value;

    halReadRegister(pMac, QWLAN_BMU_CONTROL2_REG, &value);


    /** Clear the mask */
    value &= ~mask;

    /** Set the mask */
    value |= mask;

    if (halWriteRegister(pMac, QWLAN_BMU_CONTROL2_REG, value) != eHAL_STATUS_SUCCESS)
        return eHAL_STATUS_FAILURE;

    return eHAL_STATUS_SUCCESS;
}

eHalStatus halBmu_GetStaWqStatus(tpAniSirGlobal pMac, tpBmuStaQueueData pStaQueueData)
{
    tpStaStruct pSta = (tpStaStruct) pMac->hal.halMac.staTable;
    tANI_U8 staIdx;
    tANI_U32 bmuBtqmStaQueues;
    tANI_U8 bssIdx;
    tANI_U16 dtimCnt;

    palZeroMemory(pMac->hHdd, pStaQueueData, sizeof(tBmuStaQueueData));

    /** Get the WQ Info for all stations */
    for (staIdx = 0; staIdx < pMac->hal.memMap.maxStations; staIdx++) {

        if (halBmu_btqmStaTxWqStatus(pMac, staIdx, &bmuBtqmStaQueues) != eHAL_STATUS_SUCCESS)
            return eHAL_STATUS_FAILURE;

        /** If status not equal to zero there is data for that sta */
        if (bmuBtqmStaQueues != 0) {

            /** Get dtim count */
            halMTU_GetDtimCount(pMac, &dtimCnt);

                        /** Broadcast/Multicast */
            if(staIdx == 0) {
                if(dtimCnt == 0)
                    pStaQueueData->fBroadcastTrafficPending = 1;
            }
            /** Unicast */
            else {
                pStaQueueData->assocId[staIdx] = (tANI_U16)pSta->assocId;
            }

            /** Get the BSSID for the STA */
            if (halTable_GetBssIndexForSta(pMac, &bssIdx, staIdx) != eHAL_STATUS_SUCCESS)
                return eHAL_STATUS_FAILURE;

            pStaQueueData->dtimCount = (tANI_U8) dtimCnt;
            pStaQueueData->bssIdx = bssIdx;
        }

        /** Increment PSta Pointer */
        pSta++;

    }

    return eHAL_STATUS_SUCCESS;
}

eHalStatus halBmu_btqmStaTxWqStatus(tpAniSirGlobal pMac, tANI_U8 staIdx, tANI_U32 *pbmuBtqmStaQueues)
{
    tANI_U32     value = 0;


    value = staIdx & ~QWLAN_BMU_STA_CONFIG_STATUS1_STA_CONFIG_UPDATE_MASK;

    /** Write the staidx and update mask values */
    halWriteRegister(pMac, QWLAN_BMU_STA_CONFIG_STATUS1_REG, value);

    /** Poll on bit 31 is clear */
    do {

        halReadRegister(pMac, QWLAN_BMU_STA_CONFIG_STATUS1_REG, &value);


    } while ((value & QWLAN_BMU_STA_CONFIG_STATUS1_READ_WRITE_STATUS_MASK));

    /** Read the status 4 register to know which queues have data */
    halReadRegister(pMac, QWLAN_BMU_STA_CONFIG_STATUS4_REG, &value);

    *pbmuBtqmStaQueues = value & QWLAN_BMU_STA_CONFIG_STATUS4_DATA_AVAILABLE_MASK;

    return eHAL_STATUS_SUCCESS;
}


eHalStatus halBmu_StaCfgForPM(tpAniSirGlobal pMac, tANI_U16 staIdx, tANI_U32 mask, tANI_U16 delEnbQIdMask, tANI_U16 trigEnbQIdMask)
{
    tANI_U32     value = 0;

    value = staIdx & ~QWLAN_BMU_STA_CONFIG_STATUS1_STA_CONFIG_UPDATE_MASK;

    /** Write the staidx and update mask values */
    halWriteRegister(pMac, QWLAN_BMU_STA_CONFIG_STATUS1_REG, value);

    
    /** Poll on bit 31 is clear */
    do {

        halReadRegister(pMac, QWLAN_BMU_STA_CONFIG_STATUS1_REG, &value);

    } while ((value & QWLAN_BMU_STA_CONFIG_STATUS1_READ_WRITE_STATUS_MASK));

    /** Read the Config2 register */
    halReadRegister(pMac, QWLAN_BMU_STA_CONFIG_STATUS2_REG, &value);

    /** Clear the mask */
    value &= ~mask;

    /** Set the mask */
    value |= mask;

    halWriteRegister(pMac, QWLAN_BMU_STA_CONFIG_STATUS2_REG, value);

#if 0 //FIXME_NO_VIRGO

    // Update delivery and trigger enabled queids
    value = delEnbQIdMask << QWLAN_BMU_STA_CONFIG_STATUS3_DELIVERY_ENABLED_TCID_QUEUES_OFFSET | trigEnbQIdMask;

    halWriteRegister(pMac, QWLAN_BMU_STA_CONFIG_STATUS3_REG, value);
#endif

    /** Write back the values */
    value = staIdx | QWLAN_BMU_STA_CONFIG_STATUS1_STA_CONFIG_UPDATE_MASK;

    /** Write the staidx and update mask values */
    halWriteRegister(pMac, QWLAN_BMU_STA_CONFIG_STATUS1_REG, value);

    /** Poll on bit 31 is clear */
    do {

        halReadRegister(pMac, QWLAN_BMU_STA_CONFIG_STATUS1_REG, &value);

    } while ((value & QWLAN_BMU_STA_CONFIG_STATUS1_READ_WRITE_STATUS_MASK));

    return eHAL_STATUS_SUCCESS;
}


tANI_U16 halBmu_getQidMask(tANI_U8 acAPSDflag)
{
    tANI_U16 mask = 0;

    if (acAPSDflag & HAL_APSD_AC_BE_MASK)
    {
        // AC_BE is mapped to backoff engine 7 which is mapped to Qid 0 & 3
        mask |= (1 << BTQM_QUEUE_TX_TID_0) | (1 << BTQM_QUEUE_TX_TID_3);
    }

    if (acAPSDflag & HAL_APSD_AC_BK_MASK)
    {
        // AC_BK is mapped to backoff engine 6 which is mapped to Qid 1 & 2
        mask |= (1 << BTQM_QUEUE_TX_TID_1) | (1 << BTQM_QUEUE_TX_TID_2);
    }

    if (acAPSDflag & HAL_APSD_AC_VI_MASK)
    {
        // AC_VI is mapped to backoff engine 5 which is mapped to Qid 8, 4 & 5
        mask |= (1 << BTQM_QUEUE_TX_TID_4) | (1 << BTQM_QUEUE_TX_TID_5);
    }

    if (acAPSDflag & HAL_APSD_AC_VO_MASK)
    {
        // AC_VO is mapped to backoff engine 4 which is mapped to Qid 9, 6 & 7
        mask |= (1 << BTQM_QUEUE_TX_TID_6) | (1 << BTQM_QUEUE_TX_TID_7);
    }

    return mask;
}

eHalStatus halBmu_ConfigureToSendBAR(tpAniSirGlobal pMac, tANI_U8 staIdx, tANI_U8 queueId)
{
    tANI_U32 value;

    value = (staIdx << QWLAN_BMU_TRANSMIT_QUEUE_ACCESS_CONTROL1_STAID_OFFSET) |
            (queueId << QWLAN_BMU_TRANSMIT_QUEUE_ACCESS_CONTROL1_QUEUEID_OFFSET);

    /** Write the staidx and queueId */
    halWriteRegister(pMac, QWLAN_BMU_TRANSMIT_QUEUE_ACCESS_CONTROL1_REG, value);

    halReadRegister(pMac, QWLAN_BMU_TRANSMIT_QUEUE_ACCESS_CONTROL4_REG, &value);

#ifdef CONFIGURE_SW_TEMPLATE
    /** Enable the write option and set queue bar indication bit */
    value |= ~QWLAN_BMU_TRANSMIT_QUEUE_ACCESS_CONTROL4_READ_WRITE_CONTROL_MASK |
            QWLAN_BMU_TRANSMIT_QUEUE_ACCESS_CONTROL4_SET_QUEUE_BAR_INDICATION_MASK;
#endif //CONFIGURE_SW_TEMPLATE

    /* Enable the read operation to avoid writing any unexpected values 
	   into the BTQM queue information (head/tail/nrframes)*/
	/* And also set queue bar indication bit for BMU to transmit the BAR */    
	value |= (QWLAN_BMU_TRANSMIT_QUEUE_ACCESS_CONTROL4_SET_QUEUE_BAR_INDICATION_MASK | 
		      QWLAN_BMU_TRANSMIT_QUEUE_ACCESS_CONTROL4_READ_WRITE_CONTROL_MASK);

    /** Write the staidx and queueId */
    halWriteRegister(pMac, QWLAN_BMU_TRANSMIT_QUEUE_ACCESS_CONTROL4_REG, value);

    /** Poll on write access to complete */
    do {
        halReadRegister(pMac, QWLAN_BMU_TRANSMIT_QUEUE_ACCESS_CONTROL4_REG, 
                                                    &value);

    } while (!(value & QWLAN_BMU_TRANSMIT_QUEUE_ACCESS_CONTROL4_ACCESS_DONE_MASK));

    return eHAL_STATUS_SUCCESS;
}

eHalStatus halBmu_ReadBtqmQFrmInfo(tpAniSirGlobal pMac, tANI_U8 staIdx, tANI_U8 queueId, tANI_U32 *pNumFrames, 
                                tANI_U32 *pHeadBdIndex, tANI_U32 *pTailBdIndex )
{
    tANI_U32 value, count = 0;
#ifdef BMU_FATAL_ERROR
    tANI_U32 bmuBtqmStaQueues = 0;
#endif
    eHalStatus status = eHAL_STATUS_FAILURE;
    tANI_U32 regValue = 0;
    tANI_U8  dataBkoffStalled = FALSE;

#ifdef BMU_FATAL_ERROR
    if (halBmu_getBtqmQueueStatus(pMac, staIdx, &bmuBtqmStaQueues) != eHAL_STATUS_SUCCESS)
        return eHAL_STATUS_FAILURE;

    /** If status not equal to zero there is data for that sta */
    if (bmuBtqmStaQueues & QWLAN_BMU_STA_CONFIG_STATUS2_STA_TX_ENABLE_MASK) {
        if(eHAL_STATUS_SUCCESS != (status = halBmu_sta_enable_disable_control(pMac, staIdx, eBMU_ENB_TX_QUE_DONOT_ENB_TRANS)))
           return status;
    }
#else
    // Disable Data Backoffs.
    halReadRegister(pMac, QWLAN_MTU_BKOF_CONTROL_REG, &regValue);
    if ((regValue & SW_MTU_STALL_DATA_BKOF_MASK) != SW_MTU_STALL_DATA_BKOF_MASK) {
        halMTU_stallBackoffs(pMac, SW_MTU_STALL_DATA_BKOF_MASK);
        dataBkoffStalled = TRUE;
    }
    else {
        HALLOG1( halLog( pMac, LOG1, FL("Data Backoff stalled at call itself hence not stalled here\n")));
    }
#endif

    value = (staIdx << QWLAN_BMU_TRANSMIT_QUEUE_ACCESS_CONTROL1_STAID_OFFSET) |
            (queueId << QWLAN_BMU_TRANSMIT_QUEUE_ACCESS_CONTROL1_QUEUEID_OFFSET);

    /** Write the staidx and queueId */
    halWriteRegister(pMac, QWLAN_BMU_TRANSMIT_QUEUE_ACCESS_CONTROL1_REG, value);

    halWriteRegister(pMac, QWLAN_BMU_TRANSMIT_QUEUE_ACCESS_CONTROL4_REG, 
                         QWLAN_BMU_TRANSMIT_QUEUE_ACCESS_CONTROL4_READ_WRITE_CONTROL_MASK);

    /** Poll on write access to complete */
    do {
        halReadRegister(pMac, QWLAN_BMU_TRANSMIT_QUEUE_ACCESS_CONTROL4_REG, &value);
        count++;
    } while (!(value & QWLAN_BMU_TRANSMIT_QUEUE_ACCESS_CONTROL4_ACCESS_DONE_MASK)&& (count < BMU_REG_MAX_POLLING));

	if(count > BMU_REG_POLLING_WARNING){
		HALLOGE( halLog(pMac, LOGE, FL("Polled BTQM Q register for %d times %d!!!\n"), staIdx, count));
	}
    if(count >= BMU_REG_MAX_POLLING)
         return status;

    halReadRegister(pMac, QWLAN_BMU_TRANSMIT_QUEUE_ACCESS_CONTROL5_REG, &value);


    if(pNumFrames)
       *pNumFrames = value & QWLAN_BMU_TRANSMIT_QUEUE_ACCESS_CONTROL5_NR_OF_FRAMES_IN_QUEUE_MASK;

    halReadRegister(pMac, QWLAN_BMU_TRANSMIT_QUEUE_ACCESS_CONTROL6_REG, &value);


    if(pHeadBdIndex)
       *pHeadBdIndex = value >> QWLAN_BMU_TRANSMIT_QUEUE_ACCESS_CONTROL6_QUEUE_HEAD_BD_INDEX_OFFSET;

    if(pTailBdIndex)
        *pTailBdIndex = value &  QWLAN_BMU_TRANSMIT_QUEUE_ACCESS_CONTROL6_QUEUE_TAIL_BD_INDEX_MASK; 

    status = eHAL_STATUS_SUCCESS;

#ifdef BMU_FATAL_ERROR
    if (bmuBtqmStaQueues & QWLAN_BMU_STA_CONFIG_STATUS2_STA_TX_ENABLE_MASK) {
        if(eHAL_STATUS_SUCCESS != (status = halBmu_sta_enable_disable_control(pMac, staIdx, eBMU_ENB_TX_QUE_ENB_TRANS))){
             HALLOGE( halLog(pMac, LOGE, FL("Can't reenable BTQM Tx for STA %d\n"), staIdx));
             return eHAL_STATUS_FAILURE;
        }
     }
#else
    // Enable Data Backoffs.
    if (dataBkoffStalled == TRUE)
        halMTU_startBackoffs(pMac, SW_MTU_STALL_DATA_BKOF_MASK);
    else {
        HALLOG1( halLog( pMac, LOG1, FL("Data Backoff stalled at call itself hence not starting here\n")));
    }
#endif
    
    return status;
}

eHalStatus halBmu_ReadBdInfo(tpAniSirGlobal pMac, tANI_U32 bdIdx, tpBmuBtqmBdInfo pBdInfo, tANI_U8 fDetailed){
    tANI_U32 value, value2, count = 0;

    if(!pBdInfo || !bdIdx)
        return eHAL_STATUS_FAILURE;

    halWriteRegister(pMac, QWLAN_BMU_BD_INDEX_INFO_ACCESS_CONTROL1_REG, bdIdx);
                                                
    /** Poll on write access to complete */
    do {
        halReadRegister(pMac, QWLAN_BMU_BD_INDEX_INFO_ACCESS_CONTROL1_REG, &value);
        count++;
    } while ((value & QWLAN_BMU_BD_INDEX_INFO_ACCESS_CONTROL1_BD_INDEX_INFO_ACCESS_STATUS_MASK)&& (count < BMU_REG_MAX_POLLING));

	if(count > BMU_REG_POLLING_WARNING){
		HALLOGE( halLog(pMac, LOGE, FL("!!!Polled BMU BD index %d info register for %d times %d!!!\n"), bdIdx, count));
	}
    if(count >= BMU_REG_MAX_POLLING)
         return eHAL_STATUS_FAILURE;

    halReadRegister(pMac, QWLAN_BMU_BD_INDEX_INFO_ACCESS_CONTROL6_REG,&value2);
    halReadRegister(pMac, QWLAN_BMU_BD_INDEX_INFO_ACCESS_CONTROL5_REG, &value);

   pBdInfo->seqNum = (value2 & QWLAN_BMU_BD_INDEX_INFO1_SEQ_NUM_COUNT_MASK) >> QWLAN_BMU_BD_INDEX_INFO1_SEQ_NUM_COUNT_OFFSET;
   pBdInfo->frameCtrl = (value2 & QWLAN_BMU_BD_INDEX_INFO1_FRAME_CTRL_MASK) >> QWLAN_BMU_BD_INDEX_INFO1_FRAME_CTRL_OFFSET;
   pBdInfo->frameSize = (value & QWLAN_BMU_BD_INDEX_INFO0_FRAME_SIZE_MASK) >> QWLAN_BMU_BD_INDEX_INFO0_FRAME_SIZE_OFFSET;

   if(fDetailed){

       pBdInfo->fragNum = (value & QWLAN_BMU_BD_INDEX_INFO0_FRAG_NUM_COUNT_LO_MASK) >> QWLAN_BMU_BD_INDEX_INFO0_FRAG_NUM_COUNT_LO_OFFSET;    
       pBdInfo->retry = (value & QWLAN_BMU_BD_INDEX_INFO0_RETRY_COUNT_MASK) >> QWLAN_BMU_BD_INDEX_INFO0_RETRY_COUNT_OFFSET;   
       pBdInfo->nxtBdIndex = (value & QWLAN_BMU_BD_INDEX_INFO0_NEXT_BD_INDEX_MASK) >> QWLAN_BMU_BD_INDEX_INFO0_NEXT_BD_INDEX_OFFSET;   

       halReadRegister(pMac, QWLAN_BMU_BD_INDEX_INFO_ACCESS_CONTROL7_REG, &value);
       
       pBdInfo->bdRateOffset = (value & QWLAN_BMU_BD_INDEX_INFO2_BD_RATE_MASK) >> QWLAN_BMU_BD_INDEX_INFO2_BD_RATE_OFFSET;
       pBdInfo->bdIntrMask = (value & QWLAN_BMU_BD_INDEX_INFO2_BD_INTR_MASK) >> QWLAN_BMU_BD_INDEX_INFO2_BD_INTR_OFFSET;
       pBdInfo->fragNum |= ((value2 & QWLAN_BMU_BD_INDEX_INFO1_FRAG_NUM_COUNT_HI_MASK) >> QWLAN_BMU_BD_INDEX_INFO1_FRAG_NUM_COUNT_HI_OFFSET)<<QWLAN_BMU_BD_INDEX_INFO0_FRAG_NUM_COUNT_LO_BITS;
   }

   HALLOGW( halLog(pMac, LOGW, FL("BD %d: SeqNum %d(0x%x) Size %d FC 0x%04x\n"),
            bdIdx, pBdInfo->seqNum, pBdInfo->seqNum, pBdInfo->frameSize)
   );

    return eHAL_STATUS_SUCCESS;
}


static eHalStatus halBmu_InitIntHanlder(tpAniSirGlobal pMac)
{
    tHalHandle hHal = (tHalHandle)pMac;
    tANI_U32 value;

    halReadRegister(pMac, QWLAN_BMU_ERR_INTR_ENABLE_REG,
                    &value);

    value |= QWLAN_BMU_ERR_INTR_ENABLE_DOUBLE_RELEASE_BD_ERR_ENABLE_MASK;

    halWriteRegister(pMac, QWLAN_BMU_ERR_INTR_ENABLE_REG,
                    value);

    /** Enable the Interrupts */
    halIntEnable(hHal, eHAL_INT_MCU_COMBINED_INT_BMU_ERROR);

    /** Enable the Error Interrupt */
    halIntEnable(hHal, eHAL_INT_BMU_ERROR_DEFAULT);

    /** Enable the WQ Interrupt */
    halIntEnable(hHal, eHAL_INT_MCU_BMU_WQ_INT_WQ_DEFAULT);

    return eHAL_STATUS_SUCCESS;
}

/**
 * @brief  : This Routine is the default BMU WQ Interrupt handler for all WQs interested
 *
 * @param  : hHalHandle - Mac Global Handle
 * @param  : intSource - Source for the paticular Interrupt.
 * @return : eHAL_STATUS_SUCCESS on Success and appropriate error sattus on error.
 */
eHalStatus halIntBmuWqHandler(tHalHandle hHalHandle, eHalIntSources intSource){
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tANI_U32 intRegMask;
    tANI_U32 intRegStatus;
    HALLOGE( tpAniSirGlobal pMac = PMAC_STRUCT(hHalHandle));
    /** Read Interrupt Status.*/
    status = halIntGetErrorStatus(hHalHandle, intSource, &intRegStatus, &intRegMask);
    if (status != eHAL_STATUS_SUCCESS) {
        return status;
    }

    intRegStatus &= intRegMask;

    if(intRegStatus){
        /** Display Error Information.*/
        HALLOGE( halLog( pMac, LOGE, FL("BMU WQ Interrupt Status %x, enable %x\n"),  intRegStatus, intRegMask ));
        return status;
    }
    return (eHAL_STATUS_SUCCESS);
}


/*
 * DESCRIPTION:
 *      Routine to backup the BTQM station config parameters.
 *      Each stations txConfig in the BTQM is written into the
 *      device memory, with the repition of address of the BTQM 
 *      register, the value specific to each station and the
 *      wait command.
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *      pAddr:  Starting address in the ADU memory from where the
 *              register/value tuple will be written
 *
 * RETURN:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */
eHalStatus halBmu_BckupBtqmStaConfig(tpAniSirGlobal pMac, tANI_U32 *pAddr)
{
    tpStaStruct pSta = (tpStaStruct) pMac->hal.halMac.staTable;
    tANI_U8  staId = 0;
    tANI_U32 value = 0, address = 0;
    tANI_U32 startAddr = *pAddr;
    tANI_U32 *pMemAddr = (tANI_U32*)startAddr;

    for (staId = 0; staId < pMac->hal.halMac.maxSta; staId++) {

        if(!pSta[staId].valid) {
            continue;
        }

        // Write the register address
        value = QWLAN_BMU_BTQM_STA_ENABLE_DISABLE_CONTROL_REG | HAL_REG_RSVD_BIT | HAL_REG_HOST_FILLED;
        halWriteDeviceMemory(pMac, 
                    (tANI_U32)pMemAddr++,
                    (tANI_U8*)&value, 
                    sizeof(tANI_U32));

        // Write the value 
        value = (pSta[staId].txConfig << QWLAN_BMU_BTQM_STA_ENABLE_DISABLE_CONTROL_STA_CONFIGURATION_COMMAND_OFFSET) |
            (staId << QWLAN_BMU_BTQM_STA_ENABLE_DISABLE_CONTROL_STAID_OFFSET);
        halWriteDeviceMemory(pMac, 
                    (tANI_U32)pMemAddr++,
                    (tANI_U8*)&value, 
                    sizeof(tANI_U32));

        // Issue a wait command
        address = (tANI_U32)pMemAddr;
        halRegBckup_WriteWaitCmd(pMac, &address, HAL_REG_REINIT_BTQM_STA_EN_DIS_WAIT_CYCLES);
        pMemAddr = (tANI_U32*)address;

    }

    // Update the start address pointer pointing to the
    // register re-init table
    *pAddr = (tANI_U32)pMemAddr;

    return eHAL_STATUS_SUCCESS;
}

/*
 * DESCRIPTION:
 *      Routine to get the status of BTQM queues for particular STA ID
 
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *      staIdx: Sta Index for which informatoin of Queues need to be retrieved.
 *      pbmuBtqmStatus: Status of BTQM queue.
 *
 * RETURN:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */

eHalStatus halBmu_getBtqmQueueStatus(tpAniSirGlobal pMac, tANI_U8 staIdx, tANI_U32 *pbmuBtqmStatus)
{
    tANI_U32     value = 0;
    tANI_U32     count = 0;

    value = staIdx & ~QWLAN_BMU_STA_CONFIG_STATUS1_STA_CONFIG_UPDATE_MASK;
	
    /** Write the staidx and update mask values */
    halWriteRegister(pMac, QWLAN_BMU_STA_CONFIG_STATUS1_REG, value);

    /** Poll on bit 31 is clear */
    do {

        count++;
        halReadRegister(pMac, QWLAN_BMU_STA_CONFIG_STATUS1_REG, &value);
        if (count >= BMU_REG_MAX_POLLING) {
            HALLOGE( halLog(pMac, LOGE, FL("Polled BTQM STA CONFIG register for %d!!!\n"), count));
            count = 0;
        }

    } while ((value & QWLAN_BMU_STA_CONFIG_STATUS1_READ_WRITE_STATUS_MASK));

    /** Read the status 2 register to know which queues have data */
    halReadRegister(pMac, QWLAN_BMU_STA_CONFIG_STATUS2_REG, &value);

    *pbmuBtqmStatus =  value & QWLAN_BMU_STA_CONFIG_STATUS2_STA_TX_ENABLE_MASK; 

     return eHAL_STATUS_SUCCESS;

}

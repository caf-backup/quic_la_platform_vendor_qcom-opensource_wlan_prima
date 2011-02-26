/**
 *
 *  @file:         halTpe.c
 *
 *  @brief:        Provides all the MAC APIs to the MTU Hardware Block.
 *
 *  @author:       Madhava Reddy S
 *
 *  Copyright (C) 2002 - 2007, Qualcomm, Inc. All rights reserved.
 *
 *  Change History:
 * ---------------------------------------
 * 09/07/2007  File created.
 * 12/19/2007  Virgo related changes.
 */

#include "halTypes.h"
#include "palTypes.h"
#include "aniGlobal.h"
#ifdef WLAN_HAL_VOLANS
#include "volansDefs.h"
#else
#include "libraDefs.h"
#endif
#include "halTpe.h"
#include "sirMacProtDef.h"
#include "halDebug.h"
#include "cfgApi.h"             //wlan_cfgGetInt
#include "halMTU.h"
#include "halAdu.h"
#include "halRxp.h"
#include "halTLApi.h"
#include "halFw.h"

/**
 * \brief Writes Hw template to device memory.
 *
 * \fn __halTpe_WriteHwTemplate
 *
 * \param hHal The global tHalHandle object
 *
 * \param tANI_U32 memOffset
 *
 * \param tANI_U8 *pBuffer
 *
 * \param tANI_U32 numBytes
 *
 * \return void
 */
static void __halTpe_WriteHwTemplate(tpAniSirGlobal pMac, tANI_U32 memOffset, tANI_U8 *pBuffer, tANI_U32 numBytes)
{
    tANI_U32 *pU32 = (tANI_U32 *)pBuffer;    
    tANI_U32 i, j;

    halZeroDeviceMemory(pMac, memOffset, numBytes);

    //When writing to HW template to device memory, the buffer prepared at
    //Host side is already in LITTLE endian format, but BAL will have SIF swap 
    //each DWORD to BIG endian. To avoid the swapping to corrupt ACK/BA/... templates,
    //swap the buffer here.
    for (i= numBytes, j = 0; i; i-= sizeof(tANI_U32), j++)
        pU32[ j ] = sirSwapU32( pU32[j] );

    halWriteDeviceMemory(pMac, memOffset, pBuffer, numBytes);

}

/**
 * \brief Initialize the Hw template.
 *
 * \fn __halTpe_InitializeHwTemplate
 *
 * \param hHal The global tHalHandle object
 *
 * \param tANI_U32 type
 *
 * \param tANI_U32 sub_type
 *
 * \param void *hwTemplate
 *
 * \param tANI_U32 numBytes
 *
 * \return eHalStatus
 */
static void __halTpe_InitializeHwTemplate(tpAniSirGlobal pMac, tANI_U32 type,
                                               tANI_U32 sub_type, void *hwTemplate, tANI_U32 numBytes)
{
    tTemplateHeader *pHeader = (tTemplateHeader *)hwTemplate;

    palZeroMemory(pMac->hHdd, hwTemplate, numBytes);

    pHeader->type = type;
    pHeader->subtype = sub_type;

}

/**
 * \fn __halTpe_InitStaDescBase
 *
 * \brief Configure TPE Station Descriptor Base
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param tpeStaDesc_offset Station Descriptor Base adress
 *
 * \return eHalStatus Station Descriptor Base configuration status
 */
static eHalStatus __halTpe_InitStaDescBase(tpAniSirGlobal pMac, tANI_U32 tpeStaDesc_offset)
{

    HALLOGW( halLog(pMac, LOGW, FL("HAL tpeStaDesc_offset Offset 0x%x\n"),
                                    tpeStaDesc_offset));
    /** Zero out the TPE STA descriptor */
    halZeroDeviceMemory(pMac, pMac->hal.memMap.tpeStaDesc_offset,
            pMac->hal.memMap.tpeStaDesc_size);

    halWriteRegister(pMac, QWLAN_TPE_SW_STA_DESCR_ADDR_REG,
                    tpeStaDesc_offset) ;

    return eHAL_STATUS_SUCCESS;
}

/**
 * \fn __halTpe_InitHwTemplateBase
 *
 * \brief Configure TPE HW Template Base
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param hwTemplate_offset HW Template Base adress
 *
 * \return eHalStatus HW Template Base configuration status
 */
static eHalStatus __halTpe_InitHwTemplateBase(tpAniSirGlobal pMac,
                                    tANI_U32 hwTemplate_offset)
{
    /** Zero out the HW Template Descriptor */
    halZeroDeviceMemory(pMac, pMac->hal.memMap.hwTemplate_offset,
                                HW_TEMPLATE_SIZE);

    halWriteRegister(pMac, QWLAN_TPE_SW_TEMPLATE_BASE_ADDR_REG,
                    hwTemplate_offset);

    return eHAL_STATUS_SUCCESS;
}

/**
 * \fn __halTpe_InitBeaconTemplateBase
 *
 * \brief Configure TPE Beacon Template Base
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param tpe_beacon_template_offset Beacon Template Base adress
 *
 * \return eHalStatus Beacon Template Base configuration status
 */
static void __halTpe_InitBeaconTemplateBase(tpAniSirGlobal pMac)
{
    /** Zero out the TPE Beacon template address */
    halZeroDeviceMemory(pMac, pMac->hal.memMap.beaconTemplate_offset,
            pMac->hal.memMap.beaconTemplate_size);

    halWriteRegister(pMac, QWLAN_TPE_SW_BEACON_BASE_ADDR_REG,
                pMac->hal.memMap.beaconTemplate_offset) ;

}

/**
 * \fn __halTpe_SetACKTemplate
 *
 * \brief Configure the ACK Template
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param type Type value of the template
 *
 * \param sub_type Sub Type value of the template
 *
 * \param hwTemplateParams Pointer to SW configurable parameters for the Templates
 *
 * \return eHalStatus ACK Template Configuration status
 */
static void __halTpe_SetACKTemplate(tpAniSirGlobal pMac, tANI_U32 type,
                                tANI_U32 sub_type, tHwTemplateParams *hwTemplateParams)
{

    tHwACKTemplate    ackTemplate;

    // Zero out the structure to take care of some uninitialized fields that are supposed
    // to be filled in by SW.
    
    palZeroMemory(pMac->hHdd, &ackTemplate, sizeof(ackTemplate));

    __halTpe_InitializeHwTemplate(pMac, type, sub_type, &ackTemplate, sizeof(ackTemplate));

     __halTpe_WriteHwTemplate(pMac, pMac->hal.memMap.hwTemplate_offset +
                        TEMPLATE_ACK_OFFSET, (tANI_U8 *)&ackTemplate,
                        sizeof(ackTemplate));

}

/**
 * \fn __halTpe_SetRTSTemplate
 *
 * \brief Configure the RTS Template
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param type Type value of the template
 *
 * \param sub_type Sub Type value of the template
 *
 * \param hwTemplateParams Pointer to SW configurable parameters for the Templates
 *
 * \return eHalStatus RTS Template Configuration status
 */
static void __halTpe_SetRTSTemplate(tpAniSirGlobal pMac, tANI_U32 type,
                                tANI_U32 sub_type, tHwTemplateParams *hwTemplateParams)
{
    tHwRTSTemplate rtsTemplate;


    // Zero out the structure to take care of some uninitialized fields that are supposed
    // to be filled in by SW.
    
    palZeroMemory(pMac->hHdd, &rtsTemplate, sizeof(rtsTemplate));

    __halTpe_InitializeHwTemplate(pMac, type, sub_type, &rtsTemplate, sizeof(rtsTemplate));

    __halTpe_WriteHwTemplate(pMac, pMac->hal.memMap.hwTemplate_offset +
                        TEMPLATE_RTS_OFFSET, (tANI_U8 *)&rtsTemplate,
                        sizeof(rtsTemplate));

    
}

/**
 * \fn __halTpe_SetCTSTemplate
 *
 * \brief Configure the CTS Template
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param type Type value of the template
 *
 * \param sub_type Sub Type value of the template
 *
 * \param hwTemplateParams Pointer to SW configurable parameters for the Templates
 *
 * \return eHalStatus CTS Template Configuration status
 */
static void halTpe_SetCTSTemplate(tpAniSirGlobal pMac, tANI_U32 type,
                                tANI_U32 sub_type, tHwTemplateParams *hwTemplateParams)
{
    tHwCTSTemplate ctsTemplate;


    // Zero out the structure to take care of some uninitialized fields that are supposed
    // to be filled in by SW.
    
    palZeroMemory(pMac->hHdd, &ctsTemplate, sizeof(ctsTemplate));

    __halTpe_InitializeHwTemplate(pMac, type, sub_type, &ctsTemplate, sizeof(ctsTemplate));

    __halTpe_WriteHwTemplate(pMac, pMac->hal.memMap.hwTemplate_offset +
                        TEMPLATE_CTS_OFFSET, (tANI_U8 *)&ctsTemplate,
                        sizeof(ctsTemplate));

    
}

/**
 * \fn __halTpe_SetBARTemplate
 *
 * \brief Configure the BAR Template
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param type Type value of the template
 *
 * \param sub_type Sub Type value of the template
 *
 * \param hwTemplateParams Pointer to SW configurable parameters for the Templates
 *
 * \return eHalStatus BAR Template Configuration status
 */
static void __halTpe_SetBARTemplate(tpAniSirGlobal pMac, tANI_U32 type,
                                tANI_U32 sub_type, tHwTemplateParams *hwTemplateParams)
{
    tHwBARTemplate barTemplate;


    // Zero out the structure to take care of some uninitialized fields that are supposed
    // to be filled in by SW.
    
    palZeroMemory(pMac->hHdd, &barTemplate, sizeof(barTemplate));

    __halTpe_InitializeHwTemplate(pMac, type, sub_type, &barTemplate, sizeof(barTemplate));

    __halTpe_WriteHwTemplate(pMac, pMac->hal.memMap.hwTemplate_offset +
                        TEMPLATE_BAR_OFFSET, (tANI_U8 *)&barTemplate,
                        sizeof(barTemplate));

    
}

/**
 * \fn __halTpe_SetBATemplate
 *
 * \brief Configure the BA Template
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param type Type value of the template
 *
 * \param sub_type Sub Type value of the template
 *
 * \param hwTemplateParams Pointer to SW configurable parameters for the Templates
 *
 * \return eHalStatus BA Template Configuration status
 */
static void __halTpe_SetBATemplate(tpAniSirGlobal pMac, tANI_U32 type,
                                tANI_U32 sub_type, tHwTemplateParams *hwTemplateParams)
{
    tHwBATemplate baTemplate;


    // Zero out the structure to take care of some uninitialized fields that are supposed
    // to be filled in by SW.
    
    palZeroMemory(pMac->hHdd, &baTemplate, sizeof(baTemplate));

    __halTpe_InitializeHwTemplate(pMac, type, sub_type, &baTemplate, sizeof(baTemplate));

     __halTpe_WriteHwTemplate(pMac, pMac->hal.memMap.hwTemplate_offset +
                        TEMPLATE_BA_OFFSET, (tANI_U8 *)&baTemplate,
                        sizeof(baTemplate));


}

/**
 * \fn __halTpe_SetCFEndTemplate
 *
 * \brief Configure the CF-END Template
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param type Type value of the template
 *
 * \param sub_type Sub Type value of the template
 *
 * \param hwTemplateParams Pointer to SW configurable parameters for the Templates
 *
 * \return eHalStatus CF-END Template Configuration status
 */
static void __halTpe_SetCFEndTemplate(tpAniSirGlobal pMac, tANI_U32 type,
                                tANI_U32 sub_type, tHwTemplateParams *hwTemplateParams)
{
    tHwCFEndTemplate cfEndTemplate;
    

    // Zero out the structure to take care of some uninitialized fields that are supposed
    // to be filled in by SW.
    
    palZeroMemory(pMac->hHdd, &cfEndTemplate, sizeof(cfEndTemplate));

    __halTpe_InitializeHwTemplate(pMac, type, sub_type, &cfEndTemplate, sizeof(cfEndTemplate));

     __halTpe_WriteHwTemplate(pMac, pMac->hal.memMap.hwTemplate_offset +
                        TEMPLATE_CF_END_OFFSET, (tANI_U8 *)&cfEndTemplate,
                        sizeof(cfEndTemplate));

    
}

/**
 * \fn  halTpe_InitHwTemplates
 *
 * \brief Initialize the TPE HW Templates
 *
 * \param hHal The global tHalHandle object
 *
 * \param arg Pointer to arguments
 *
 * \return eHalStatus TPE HW Templates initialization status
 */
eHalStatus halTpe_InitHwTemplates(tHalHandle hHal, void *arg)
{
    tpAniSirGlobal     pMac = (tpAniSirGlobal)hHal;
    tHwTemplateParams    hwTemplateParams;

    HALLOGW( halLog(pMac, LOGW, FL("HAL TPE Init HW Templates \n")));

    palZeroMemory(pMac->hHdd, &hwTemplateParams, sizeof(hwTemplateParams));

    /** Initialize tha ACK template */
    __halTpe_SetACKTemplate(pMac, SIR_MAC_CTRL_FRAME, SIR_MAC_CTRL_ACK, &hwTemplateParams);

    /** Initialize tha RTS template */
    __halTpe_SetRTSTemplate(pMac, SIR_MAC_CTRL_FRAME, SIR_MAC_CTRL_RTS, &hwTemplateParams);

    /** Initialize tha CTS template */
    halTpe_SetCTSTemplate(pMac, SIR_MAC_CTRL_FRAME, SIR_MAC_CTRL_CTS, &hwTemplateParams);

    /** Initialize tha BAR template */
    __halTpe_SetBARTemplate(pMac, SIR_MAC_CTRL_FRAME, SIR_MAC_CTRL_BAR, &hwTemplateParams);

    /** Initialize tha BA template */
    __halTpe_SetBATemplate(pMac, SIR_MAC_CTRL_FRAME, SIR_MAC_CTRL_BA, &hwTemplateParams);

    /** Initialize tha CF-END template */
    __halTpe_SetCFEndTemplate(pMac, SIR_MAC_CTRL_FRAME, SIR_MAC_CTRL_CF_END, &hwTemplateParams);

    /** Initialize tha Qos Null template */
    halTpe_SetQosNullTemplate(pMac, SIR_MAC_DATA_FRAME, SIR_MAC_DATA_QOS_NULL, &hwTemplateParams);

    /** Initialize tha Data Null template */
    halTpe_SetDataNullTemplate(pMac, SIR_MAC_DATA_FRAME, SIR_MAC_DATA_NULL, &hwTemplateParams);

    return eHAL_STATUS_SUCCESS;
}

/**
 * \fn halTpe_SetConsiderSecondPacket
 *
 * \brief To set the consider second packet bit in TPE SW PM register. 
 *        This setting will give correct duration/id in the fragmented frames.
 *        The TPE will calculate the duration including the next fragement also. 
 *        This is required when a fragmentation chain has to be transmitted with each fragment 
 *        having Duration field considering the next fragment. This is applicable for all the 
 *        fragments except the last one.
 *
 * \param pMac The global tpAniSirGlobal object 
 *
 * \return void
 */
static void halTpe_SetConsiderSecondPacket(tpAniSirGlobal pMac)
{
    tANI_U32 value;

    /** Read the control register */
    halReadRegister(pMac, QWLAN_TPE_SW_PM_REG, &value);

    value |= QWLAN_TPE_SW_PM_SW_CONSIDER_SECOND_PKT_READ_MASK;

    halWriteRegister(pMac, QWLAN_TPE_SW_PM_REG, value);
}

/**
 * \fn halTpe_Start
 *
 * \brief Initialize the TPE module
 *
 * \param hHal The global tHalHandle object
 *
 * \param arg Pointer to arguments
 *
 * \return eHalStatus TPE module initialization status
 */
eHalStatus halTpe_Start(tHalHandle hHal, void *arg)
{
    tpAniSirGlobal     pMac = (tpAniSirGlobal)hHal;
    tANI_U32     value, mask;

    /** Initialize TPE STA desc base */
    if (__halTpe_InitStaDescBase(pMac, pMac->hal.memMap.tpeStaDesc_offset)
                                                != eHAL_STATUS_SUCCESS)
        return eHAL_STATUS_FAILURE;

    /** Initialize HW Template base */
    if (__halTpe_InitHwTemplateBase(pMac, pMac->hal.memMap.hwTemplate_offset)
                                                != eHAL_STATUS_SUCCESS)
        return eHAL_STATUS_FAILURE;

    /** Initialize the Beacon Template base */
    __halTpe_InitBeaconTemplateBase(pMac);

    /** Set the Tx SIFS cycles */
    value = (tANI_U32)(SW_TX_SIFS_A_MODE_CYCLES << QWLAN_TPE_SW_PM_SW_TX_SIFS_A_MODE_CYCLES_OFFSET) | 
           (tANI_U32)(SW_TX_SIFS_B_MODE_CYCLES << QWLAN_TPE_SW_PM_SW_TX_SIFS_B_MODE_CYCLES_OFFSET);

    mask = QWLAN_TPE_SW_PM_SW_TX_SIFS_A_MODE_CYCLES_MASK | QWLAN_TPE_SW_PM_SW_TX_SIFS_B_MODE_CYCLES_MASK;

    if (halTpe_SetSifsCycle(pMac, value, mask) != eHAL_STATUS_SUCCESS)
        return eHAL_STATUS_FAILURE;

    /** Set AC to Back-off lookup vector */
    if (halTpe_SetAcToBkofLookupVec(pMac) != eHAL_STATUS_SUCCESS)
        return eHAL_STATUS_FAILURE;

    /** For getting correct duration/id in the frames, setting consider second packet bit **/
    halTpe_SetConsiderSecondPacket(pMac);

    /** Enable TPE.statistic_counters_cntl.copy_vista_counter to fetch the stats */
    {
        if(halReadRegister(pMac, QWLAN_TPE_STATISTIC_COUNTERS_CNTL_REG, 
                                       &value) != eHAL_STATUS_SUCCESS)
            return eHAL_STATUS_FAILURE;
        value |= QWLAN_TPE_STATISTIC_COUNTERS_CNTL_COPY_VISTA_COUNTERS_MASK;
        
        if(halWriteRegister(pMac, QWLAN_TPE_STATISTIC_COUNTERS_CNTL_REG, 
                                        value) != eHAL_STATUS_SUCCESS)
            return eHAL_STATUS_FAILURE;
    }

    return eHAL_STATUS_SUCCESS;
}

/**
 * \fn halTpe_SetProtectionThreshold
 *
 * \brief Set TPE RTS Protection threshold
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \return eHalStatus Rate table reterival status
 */
eHalStatus halTpe_SetProtectionThreshold(tpAniSirGlobal pMac, tANI_U32 threshold)
{
    tANI_U32 value;

    /** Read the control register */
    halReadRegister(pMac, QWLAN_TPE_SW_MAX_MPDUS_IN_AMPDU_AND_MISC_REG, &value);

    value &= ~(QWLAN_TPE_SW_MAX_MPDUS_IN_AMPDU_AND_MISC_SW_PROTECTION_THR_MASK);
    value |= (threshold << QWLAN_TPE_SW_MAX_MPDUS_IN_AMPDU_AND_MISC_SW_PROTECTION_THR_OFFSET);

    halWriteRegister(pMac, QWLAN_TPE_SW_MAX_MPDUS_IN_AMPDU_AND_MISC_REG, value);

    return eHAL_STATUS_SUCCESS;
}

/**
 * \fn halTpe_RateTableRead
 *
 * \brief Reterive TPE Rate table contents
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param tableAddr Index of the rate table
 *
 * \param data    Pointer to strore data
 *
 *
 * \return eHalStatus Rate table reterival status
 */
#ifdef WLAN_HAL_VOLANS
eHalStatus halTpe_RateTableRead(tpAniSirGlobal pMac, tANI_U32 tableAddr,
                        tANI_U32 *data)
#else
eHalStatus halTpe_RateTableRead(tpAniSirGlobal pMac, tANI_U32 tableAddr,
                        tANI_U32 *dWord0, tANI_U32 *dWord1)
#endif
{
    tANI_U32    value;

    /** Read the control register */
    halReadRegister(pMac, QWLAN_TPE_RATE_TABLE_SRAM_CONTROL_REG,
                        &value) ;

    /** Check if bit 31 is set */
    if (value & QWLAN_TPE_RATE_TABLE_SRAM_CONTROL_RATE_SRAM_XACT_DONE_MASK) {

        /** Start the read transaction by writing 0 to bit 31 */
        value = (tableAddr << QWLAN_TPE_RATE_TABLE_SRAM_CONTROL_RATE_SRAM_XACT_ADDR_OFFSET) |
                (QWLAN_TPE_RATE_TABLE_SRAM_CONTROL_RATE_SRAM_XACT_START_MASK &
                ~QWLAN_TPE_RATE_TABLE_SRAM_CONTROL_RATE_SRAM_XACT_DONE_MASK &
                ~QWLAN_TPE_RATE_TABLE_SRAM_CONTROL_RATE_SRAM_XACT_WRITE_READZ_MASK);

        halWriteRegister(pMac, QWLAN_TPE_RATE_TABLE_SRAM_CONTROL_REG,
                            value);

        value = 0;

        /** Poll on 31 bit to set */
        do {
            halReadRegister(pMac ,QWLAN_TPE_RATE_TABLE_SRAM_CONTROL_REG, &value);

        } while (!(value & QWLAN_TPE_RATE_TABLE_SRAM_CONTROL_RATE_SRAM_XACT_DONE_MASK));

        /** Read the table data register */
#ifdef WLAN_HAL_VOLANS
        halReadRegister(pMac, QWLAN_TPE_RATE_TABLE_SRAM_CONTROL_REG,
                                    data) ;
        *data = (*data & QWLAN_TPE_RATE_TABLE_SRAM_CONTROL_RATE_SRAM_XACT_DATA_MASK) >> 
                QWLAN_TPE_RATE_TABLE_SRAM_CONTROL_RATE_SRAM_XACT_DATA_OFFSET;
#else
        halReadRegister(pMac, QWLAN_TPE_RATE_TABLE_SRAM_DATA1_REG,
                                    dWord1) ;

        /** Read the table data0 register */
        halReadRegister(pMac, QWLAN_TPE_RATE_TABLE_SRAM_DATA0_REG,
                                    dWord0);
#endif

    }

    return eHAL_STATUS_SUCCESS;
}

/**
 * \fn halTpe_RateTableWrite
 *
 * \brief Configure TPE Rate table contents
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param tableAddr Index of the rate table
 *
 * \param data    data to write to rate table
 *
 * \return eHalStatus Rate table configuration status
 */
#ifdef WLAN_HAL_VOLANS
eHalStatus halTpe_RateTableWrite(tpAniSirGlobal pMac, tANI_U32 tableAddr,
                                                tANI_U32 data)
#else
eHalStatus halTpe_RateTableWrite(tpAniSirGlobal pMac, tANI_U32 tableAddr,
                                                tANI_U32 dWord0, tANI_U32 dWord1)
#endif
{
    tANI_U32    value;
    tANI_U32    retry_limit=10;

    /** Read the control register */
    halReadRegister(pMac, QWLAN_TPE_RATE_TABLE_SRAM_CONTROL_REG,
                                    &value) ;

    /** Check if bit 31 is set */
    if (value & QWLAN_TPE_RATE_TABLE_SRAM_CONTROL_RATE_SRAM_XACT_DONE_MASK) {

#ifdef WLAN_HAL_VOLANS
        /** Start the Write transaction by writing 0 to bit 31 */
        value = ((tableAddr << QWLAN_TPE_RATE_TABLE_SRAM_CONTROL_RATE_SRAM_XACT_ADDR_OFFSET) |
                 (data << QWLAN_TPE_RATE_TABLE_SRAM_CONTROL_RATE_SRAM_XACT_DATA_OFFSET) |
                QWLAN_TPE_RATE_TABLE_SRAM_CONTROL_RATE_SRAM_XACT_START_MASK |
                QWLAN_TPE_RATE_TABLE_SRAM_CONTROL_RATE_SRAM_XACT_WRITE_READZ_MASK) &
                ~QWLAN_TPE_RATE_TABLE_SRAM_CONTROL_RATE_SRAM_XACT_DONE_MASK;
#else
        /** Write to table data1 register */
        halWriteRegister(pMac, QWLAN_TPE_RATE_TABLE_SRAM_DATA1_REG,
                                    dWord1);

        /** Write table data0 register */
        halWriteRegister(pMac, QWLAN_TPE_RATE_TABLE_SRAM_DATA0_REG,
                                    dWord0);

        /** Start the Write transaction by writing 0 to bit 31 */
        value = ((tableAddr << QWLAN_TPE_RATE_TABLE_SRAM_CONTROL_RATE_SRAM_XACT_ADDR_OFFSET) |
                QWLAN_TPE_RATE_TABLE_SRAM_CONTROL_RATE_SRAM_XACT_START_MASK |
                QWLAN_TPE_RATE_TABLE_SRAM_CONTROL_RATE_SRAM_XACT_WRITE_READZ_MASK) &
                ~QWLAN_TPE_RATE_TABLE_SRAM_CONTROL_RATE_SRAM_XACT_DONE_MASK;

#endif
        halWriteRegister(pMac, QWLAN_TPE_RATE_TABLE_SRAM_CONTROL_REG,
                                    value) ;

        value = 0;

        /** Poll on 31 bit to set to complete the write */
        do {
            halReadRegister(pMac ,QWLAN_TPE_RATE_TABLE_SRAM_CONTROL_REG, &value);
            retry_limit--;
        } while (!(value & QWLAN_TPE_RATE_TABLE_SRAM_CONTROL_RATE_SRAM_XACT_DONE_MASK) && (retry_limit > 0));
        if(0 == retry_limit)
		    HALLOGE( halLog( pMac, LOGE, FL("HAL TPE RATE TABLE write Failed\n")));
    }

    return eHAL_STATUS_SUCCESS;
}

/**
 * \fn halTpe_SetStaDesc
 *
 * \brief Configure the TPE Station Descriptor
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param staId Station Id to set the descriptor
 *
 * \param pTpeStaDesc Pointer to tpe station descriptor information
 *
 * \return eHalStatus Station descriptor configuration status
 */
eHalStatus halTpe_SetStaDesc(tpAniSirGlobal pMac, tANI_U8 staId,
                                                    tpTpeStaDesc pTpeStaDesc)
{
    tANI_U32    address;

    address = pMac->hal.memMap.tpeStaDesc_offset + (staId * TPE_STA_DESC_AND_STATS_SIZE);

    /** Save the STA config */
    if (halTpe_SaveStaConfig(pMac, pTpeStaDesc, staId) != eHAL_STATUS_SUCCESS)
        return eHAL_STATUS_FAILURE;

    HALLOGW( halLog( pMac, LOGW, FL("HAL TPE STA Desc address 0x%x\n"),  address ));
    HALLOGW( halLog( pMac, LOGW, FL("HAL TPE STA Desc size 0x%x\n"),  TPE_STA_DESC_ENTRY_SIZE ));

    halWriteDeviceMemory(pMac, address, (tANI_U8 *)pTpeStaDesc,
                        TPE_STA_DESC_ENTRY_SIZE);
    return eHAL_STATUS_SUCCESS;
}


/**
 * \fn halTpe_GetStaDesc
 *
 * \brief Get TPE Station Descriptor
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param staId Station Id to get the descriptor
 *
 * \param pTpeStaDesc Pointer to sotre descriptor information
 *
 * \return eHalStatus Station descriptor retreiving status
 */
eHalStatus halTpe_GetStaDesc(tpAniSirGlobal pMac, tANI_U8 staId,
                                                    tpTpeStaDesc pTpeStaDesc)
{
    tANI_U32    address;

    palFillMemory(pMac->hHdd, (void *)pTpeStaDesc, TPE_STA_DESC_ENTRY_SIZE, 0);

    address = pMac->hal.memMap.tpeStaDesc_offset + (staId * TPE_STA_DESC_AND_STATS_SIZE);

    halReadDeviceMemory(pMac, 
            address, (tANI_U8 *)pTpeStaDesc, TPE_STA_DESC_ENTRY_SIZE);

    return eHAL_STATUS_SUCCESS;
}


/**
 * \fn halTpe_GetStaStats
 *
 * \brief Get TPE Stats
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param staId Station Id to get the descriptor
 *
 * \param pTpeStats Pointer to sotre stats information
 *
 * \return eHalStatus Stats retreiving status
 */
eHalStatus halTpe_GetStaStats(tpAniSirGlobal pMac, tANI_U8 staId,
                                            tpTpeStaStats pTpeStaStats)
{
    tANI_U32    address;

    palFillMemory(pMac->hHdd, (void *)pTpeStaStats, sizeof(tTpeStaStats), 0);

    address = pMac->hal.memMap.tpeStaDesc_offset + (staId * TPE_STA_DESC_AND_STATS_SIZE) + TPE_PER_STA_STATS_START_OFFSET;

    halReadDeviceMemory(pMac, address, (tANI_U8 *)pTpeStaStats, sizeof(tTpeStaStats));

    return eHAL_STATUS_SUCCESS;
}


/* Function to retrieve the Rate adaptation related statistics/counters
 * for the given station ID in the TPE station descriptor */
eHalStatus halTpe_GetStaRaStats(tpAniSirGlobal pMac, tANI_U8 staId, 
        tpTpeStaRaStats pTpeStaRaStats)
{
    tANI_U32    address;

    palFillMemory(pMac->hHdd, (void *)pTpeStaRaStats, sizeof(tTpeStaRaStats), 0);

    // Get the address of the stations RA stats in the TPE
    // descriptor
    address = pMac->hal.memMap.tpeStaDesc_offset + 
        (staId * TPE_STA_DESC_AND_STATS_SIZE) + 
        TPE_PER_STA_STATS_START_OFFSET;

    halReadDeviceMemory(pMac, address, (tANI_U8 *)pTpeStaRaStats,
            sizeof(tTpeStaRaStats));

    return eHAL_STATUS_SUCCESS;
    }

/* Function to clear the Rate adaptation related statistics/counters
 * for the given station ID in the TPE station descriptor */
eHalStatus halTpe_ClearStaRaStats(tpAniSirGlobal pMac, tANI_U8 staId)
{
    tANI_U32    address;

    // Get the address of the stations RA stats in the TPE
    // descriptor
    address = pMac->hal.memMap.tpeStaDesc_offset + 
        (staId * TPE_STA_DESC_AND_STATS_SIZE) + 
        TPE_PER_STA_STATS_START_OFFSET;

    // Clear the stats by filling it with zero
    halFillDeviceMemory(pMac, address, sizeof(tTpeStaRaStats), 0);

    return eHAL_STATUS_SUCCESS;
}


/**
 * \fn halTpe_SetSifsCycle
 *
 * \brief Set Sifs cycle for A and G mode
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param cfgValue Configuration value
 *
 * \param mask cfg value mask
 *
 * \return eHalStatus return status
 */
eHalStatus halTpe_SetSifsCycle(tpAniSirGlobal pMac, tANI_U32 cfgValue, tANI_U32 mask)
{
    tANI_U32 value;

    halReadRegister(pMac, QWLAN_TPE_SW_PM_REG, &value);

    /** Clear the mask */
    value &= ~mask;

    /** Set the value */
    value |= cfgValue;

    halWriteRegister(pMac,  QWLAN_TPE_SW_PM_REG,  value );

    return eHAL_STATUS_SUCCESS;
}

/**
 * \fn halTpe_SetPmBit
 *
 * \brief SW Configurable PM
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param mask Set/Reset mask
 *
 * \return eHalStatus SW configurable PM status
 */
eHalStatus halTpe_SetPmBit(tpAniSirGlobal pMac, tHalBitVal mask)
{
    tANI_U32 value;

    halReadRegister(pMac, QWLAN_TPE_SW_PM_REG, &value);

    if (mask == eHAL_SET)
        value |= QWLAN_TPE_SW_PM_SW_PM_MASK; /** Set the value */
    else
        value &= ~QWLAN_TPE_SW_PM_SW_PM_MASK; /** Clear the mask */

    halWriteRegister(pMac, QWLAN_TPE_SW_PM_REG, value);

    return eHAL_STATUS_SUCCESS;
}

/**
 * \fn halTpe_SetPSPollTemplate
 *
 * \brief Configure the PS-POLL Template
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param type Type value of the template
 *
 * \param sub_type Sub Type value of the template
 *
 * \param hwTemplateParams Pointer to SW configurable parameters for the Templates
 *
 * \return eHalStatus PS-POLL Template Configuration status
 */
void halTpe_SetPSPollTemplate(tpAniSirGlobal pMac, tHwPSPollTemplate *pPsPoll)
{


    __halTpe_WriteHwTemplate(pMac, pMac->hal.memMap.hwTemplate_offset +
                        TEMPLATE_PS_POLL_OFFSET, (tANI_U8 *)pPsPoll,
                        sizeof(tHwPSPollTemplate));

}

/**
 * \fn halTpe_SetQosNullTemplate
 *
 * \brief Configure the QoS NULL Template
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param type Type value of the template
 *
 * \param sub_type Sub Type value of the template
 *
 * \param hwTemplateParams Pointer to SW configurable parameters for the Templates
 *
 * \return eHalStatus QoS NULL Template Configuration status
 */
void halTpe_SetQosNullTemplate(tpAniSirGlobal pMac, tANI_U32 type,
                                tANI_U32 sub_type, tHwTemplateParams *hwTemplateParams)
{
    tHwQosNullTemplate qosNullTemplate;

    // Zero out the structure to take care of some uninitialized fields that are supposed
    // to be filled in by SW.
    
    palZeroMemory(pMac->hHdd, &qosNullTemplate, sizeof(qosNullTemplate));

    __halTpe_InitializeHwTemplate(pMac, type, sub_type, &qosNullTemplate, sizeof(qosNullTemplate));

     __halTpe_WriteHwTemplate(pMac, pMac->hal.memMap.hwTemplate_offset +
                        TEMPLATE_QOS_NULL_OFFSET, (tANI_U8 *)&qosNullTemplate,
                        sizeof(qosNullTemplate));

    
}

/**
 * \brief Configure the Data NULL Template
 *
 * \fn halTpe_SetDataNullTemplate
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param type Type value of the template
 *
 * \param sub_type Sub Type value of the template
 *
 * \param hwTemplateParams Pointer to SW configurable parameters for the Templates
 *
 * \return eHalStatus Data NULL Template Configuration status
 */
void halTpe_SetDataNullTemplate(tpAniSirGlobal pMac, tANI_U32 type,
                                      tANI_U32 sub_type, tHwTemplateParams *hwTemplateParams)
{
    tHwDataNullTemplate dataNullTemplate;

    // Zero out the structure to take care of some uninitialized fields that are supposed
    // to be filled in by SW.
    
    palZeroMemory(pMac->hHdd, &dataNullTemplate, sizeof(dataNullTemplate));

    __halTpe_InitializeHwTemplate(pMac, type, sub_type, &dataNullTemplate, sizeof(dataNullTemplate));

    dataNullTemplate.hdr.pm = hwTemplateParams->pm;

    __halTpe_WriteHwTemplate(pMac, pMac->hal.memMap.hwTemplate_offset +
                        TEMPLATE_DATA_NULL_OFFSET, (tANI_U8 *)&dataNullTemplate,
                        sizeof(dataNullTemplate));


}

#ifdef CONFIGURE_SW_TEMPLATE
/**
 * \brief Configure the SW Template programming
 *
 * \fn halTpe_TriggerSwTemplate
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \return eHalStatus SW Template programming status
 */
eHalStatus halTpe_TriggerSwTemplate(tpAniSirGlobal pMac)
{
    tANI_U32   value;

    halReadRegister(pMac, QWLAN_TPE_SW_SOFTWARE_TX_CONTROL_REG1_REG, &value);

    value &= ~QWLAN_TPE_SW_SOFTWARE_TX_CONTROL_REG1_SW_TX_START_VALID_MASK;
    halWriteRegister(pMac, QWLAN_TPE_SW_SOFTWARE_TX_CONTROL_REG1_REG, value);

    halReadRegister(pMac, QWLAN_TPE_SW_SOFTWARE_TX_CONTROL_REG1_REG, &value);

    value |= QWLAN_TPE_SW_SOFTWARE_TX_CONTROL_REG1_SW_TX_START_VALID_MASK |
            QWLAN_TPE_SW_SOFTWARE_TX_CONTROL_REG1_SW_TX_RAW_PKT_VALID_MASK |
            (SW_TEMPLATE_TRANSMISSION_ENABLE <<
            QWLAN_TPE_SW_SOFTWARE_TX_CONTROL_REG1_SW_TX_SESSION_CONTROL_OFFSET) |
            (TPE_SW_SOFTWARE_TX_CONTROL_REG1_SW_WAIT_COMMAND2_VALUE <<
            QWLAN_TPE_SW_SOFTWARE_TX_CONTROL_REG1_SW_WAIT_COMMAND2_OFFSET ) |
            (TPE_SW_SOFTWARE_TX_CONTROL_REG1_SW_WAIT_COMMAND1_VALUE <<
            QWLAN_TPE_SW_SOFTWARE_TX_CONTROL_REG1_SW_WAIT_COMMAND1_OFFSET);


    halWriteRegister(pMac, QWLAN_TPE_SW_SOFTWARE_TX_CONTROL_REG1_REG, value);

    return eHAL_STATUS_SUCCESS;
}
#endif  //CONFIGURE_SW_TEMPLATE

/**
 * \fn halTpe_SetBeaconTemplate
 *
 * \brief Configure the Beacon Template programming
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param beaconTemplate Pointer to Beacon template
 *
 * \return eHalStatus Beacon Template programming status
 */
eHalStatus halTpe_SetBeaconTemplate(tpAniSirGlobal pMac, tANI_U16 beaconIndex, tSirMacAddr  bssId)
{
    tBeaconTemplate beaconTemplate;
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tANI_U32 beaconOffset;
    tTpeRateIdx rateIndex;
    tPwrTemplateIndex txPower = 0;

    beaconTemplate.template_header.template_type = SIR_MAC_MGMT_FRAME;
    beaconTemplate.template_header.template_sub_type = SIR_MAC_MGMT_BEACON;
    beaconTemplate.template_header.resp_is_expected = 0;
    beaconTemplate.template_header.expected_resp_type = 0;
    beaconTemplate.template_header.expected_resp_sub_type = 0;
    beaconTemplate.template_header.template_len = 0;
    beaconTemplate.template_header.reserved1 = 0;
    halGetBcnRateIdx(pMac, &rateIndex);
    halRate_getPowerIndex(pMac, rateIndex, &txPower);
    beaconTemplate.template_header.primary_data_rate_index = rateIndex;
    beaconTemplate.template_header.stbc = 0;
    beaconTemplate.template_header.reserved2 = 0;
    beaconTemplate.template_header.reserved3 = 0;
    beaconTemplate.template_header.tx_power = txPower;
    beaconTemplate.template_header.tx_antenna_enable = 0;
    beaconTemplate.template_header.tsf_offset = TPE_BEACON_1MBPS_LONG_TSF_OFFSET;
    beaconTemplate.template_header.reserved4 = 0;
    beaconTemplate.template_header.reserved5 = 0;

    /** TODO: Get txPower and txAntenna enable */

    beaconOffset = pMac->hal.memMap.beaconTemplate_offset + (beaconIndex * BEACON_TEMPLATE_SIZE);


    /** Zero out the SW Template memory */
    halZeroDeviceMemory(pMac, beaconOffset, BEACON_TEMPLATE_SIZE);

    /** Write the beacon header */
     halWriteDeviceMemory(pMac, beaconOffset ,
                            (tANI_U8 *)&beaconTemplate.template_header, BEACON_TEMPLATE_HEADER);

    halLog(pMac, LOGE, FL("halTpe_SetBeaconTemplate resets memory at address %u length %u \n"), beaconOffset, BEACON_TEMPLATE_HEADER);
    return status;
}

eHalStatus halTpe_EnableBeacon(tpAniSirGlobal pMac, 
    tSirMacBeaconInterval beaconInterval)
{
    tANI_U32 mbssidInterval;
    tANI_U8 activeBssCnt = 0;
    
    
    /** For further reference */
    pMac->hal.halMac.beaconInterval = beaconInterval;
    /** For further reference */
    pMac->hal.halMac.beaconInterval = beaconInterval;
    HALLOG1( halLog(pMac, LOG1, FL("BTAMP BRANCH===> beacon interval %x\n"),
            beaconInterval));

    /** Uncomment this once the cfg is added */
#ifdef FIXME_GEN5
    /** Program the Beacon interval */
    if ( wlan_cfgGetInt(pMac, WNI_CFG_MTU_BSSID_INTERVAL, &bssidInterval) != eSIR_SUCCESS)
        halLog(pMac, LOGE, FL("cfgGet WNI_CFG_MTU_BSSID_INTERVAL Failed\n"));
#else
    mbssidInterval = 16;    /** Uncomment after adding cfg */
#endif

    /** Get the Active Bss count */
    halMTU_GetActiveBss(pMac, &activeBssCnt);

    /** Store the Active Bss count for future use */
    pMac->hal.halMac.activeBss = activeBssCnt;

    /* When starting an IBSS, we must first reset TSF, and then
     * TBTT. Reversing the order(i.e first reset TBTT and then TSF)
     * will delay beacon transmission. As Ram explains the issue 
     * with resetting TBTT before TSF is:
     * "TBTT may get reset at a point when TSF is a very high 
     * value. Althought TBTT becomes zero for a short period, TBTT
     * tracks the TSF timer. For ex. let TSF = 80sec. So TBTT will 
     * be =80sec+100ms. Now despite the fact that TSF is reset, 
     * the TBTT timer will still be tracking the (80sec + 100ms) value.
     */
	/* This also applies in BT-AMP case */
#ifndef WLAN_SOFTAP_FEATURE    
    if (activeBssCnt <= 1) {
        halMTU_SetTsfTimer(pMac, 0, 0);
        halMTU_SetTbttTimer(pMac, 0, 0);
    } /** FIXME TODO Should take care of Multiple BSSID case.*/
#endif
    /** \fixme \todo  Do we need to set the mtbtt register.
     *  As per ram it is an internal register.
     */
    if (activeBssCnt > 1)
    {
#ifndef WLAN_SOFTAP_FEATURE    
        if (activeBssCnt == 2)
        {
            // When we start an IBSS set the TSF timer to be 0.
            // Also set tbtt to 0.
            halMTU_SetTsfTimer(pMac, 0, 0);
            halMTU_SetTbttTimer(pMac, 0, 0);
        }
#endif
        halMTU_UpdateMbssInterval(pMac, mbssidInterval);
    }

    halMTU_UpdateNumBSS(pMac, activeBssCnt);

    halMTU_UpdateBeaconInterval(pMac, beaconInterval);
    // If we truely want multiple BSS with multiple beacon transmission
    // we need to support the new mtbtt timer code here.
    // Since we have only 1 BSS sending beacon at any point in
    // time now, code here only sets TBTT timer.


    return eHAL_STATUS_SUCCESS;
}

/**
 * \brief Save the Sta Descriptor Configuration
 *
 * \fn halTpe_SaveStaConfig
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param tpeStaDescCfg Pointer to tpe sta desc configuration
 *
 * \param staIdx Station index
 *
 * \return eHalStatus save sta desc config status
 */
eHalStatus halTpe_SaveStaConfig(tpAniSirGlobal pMac, tpTpeStaDesc pTpeStaDesc, tANI_U8 staIdx)
{
    eHalStatus status = eHAL_STATUS_INVALID_STAIDX;
    tpStaStruct t = (tpStaStruct) pMac->hal.halMac.staTable;

    if (staIdx < pMac->hal.halMac.maxSta)
    {
        status = palCopyMemory(pMac->hHdd, (tANI_U8 *) &t[staIdx].tpeStaDesc,
            (tANI_U8 *)pTpeStaDesc, TPE_STA_DESC_ENTRY_SIZE);
    }

    return status;
}

/**
 * \brief Get the Sta Descriptor Configuration
 *
 * \fn halTpe_GetStaConfig
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param tpeStaDescCfg Pointer to store tpe sta desc configuration
 *
 * \param staIdx Station index
 *
 * \return eHalStatus Get sta desc config status
 */
eHalStatus halTpe_GetStaConfig(tpAniSirGlobal pMac, tpTpeStaDesc *ppTpeStaDesc, tANI_U8 staIdx)
{
    eHalStatus status = eHAL_STATUS_INVALID_STAIDX;
    tpStaStruct t = (tpStaStruct) pMac->hal.halMac.staTable;

    if (staIdx < pMac->hal.halMac.maxSta)
    {
        *ppTpeStaDesc = &t[staIdx].tpeStaDesc;
        status = eHAL_STATUS_SUCCESS;
    }

    return status;
}

/**
 * \brief Restore the Sta Descriptor Configuration
 *
 * \fn halTpe_RestoreStaConfig
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param tpeStaDescCfg Pointer to resotore tpe sta desc configuration
 *
 * \param staIdx Station index
 *
 * \return eHalStatus Restore sta desc config status
 */
eHalStatus halTpe_RestoreStaConfig(tpAniSirGlobal pMac, tpTpeStaDesc pTpeStaDesc, tANI_U8 staIdx)
{
    eHalStatus status = eHAL_STATUS_INVALID_STAIDX;
    tpStaStruct t = (tpStaStruct) pMac->hal.halMac.staTable;

    if (staIdx < pMac->hal.halMac.maxSta)
    {
        status =  palCopyMemory(pMac->hHdd, (tANI_U8 *)pTpeStaDesc, 
                (tANI_U8 *) &t[staIdx].tpeStaDesc, TPE_STA_DESC_ENTRY_SIZE);
    }

    return status;
}

/**
 * \fn halTpe_UpdateStaDesc
 *
 * \brief Configure the TPE Station Descriptor
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
 eHalStatus halTpe_UpdateStaDesc(tpAniSirGlobal pMac, tANI_U32 staIdx,
                                                    tpTpeStaDesc tpeStaDesc)
{
    tANI_U32            address;

    address = pMac->hal.memMap.tpeStaDesc_offset + (staIdx * TPE_STA_DESC_AND_STATS_SIZE);

    /** Write to TPE STA Desc memory */
    halWriteDeviceMemory(pMac, address, (tANI_U8 *)tpeStaDesc,
                                TPE_STA_DESC_ENTRY_SIZE);

    return eHAL_STATUS_SUCCESS;
}

/**
 * \fn halTpe_UpdateStaDescFields
 *
 * \brief Configure the TPE Station Descriptor
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param staId Station Id to set the descriptor
 *
 * \param tANI_U32 startOffset
 *
 * \param length
 *
 * \param tpeStaDesc Pointer to tpe station descriptor information
 *
 * \return eHalStatus Station descriptor configuration status
 */
eHalStatus halTpe_UpdateStaDescFields(tpAniSirGlobal pMac, tANI_U32 staIdx,
            tANI_U32 startOffset, tANI_U32 length, tpTpeStaDesc tpeStaDesc)
{
    tANI_U32            address;

    address = pMac->hal.memMap.tpeStaDesc_offset + (staIdx * TPE_STA_DESC_AND_STATS_SIZE) + startOffset;

    /** Write to TPE STA Desc memory */
    halWriteDeviceMemory(pMac, address, (tANI_U8 *)tpeStaDesc + startOffset, length);

    return eHAL_STATUS_SUCCESS;
}


/*
 * Function to return the pointer to the TPE sta descriptor's rate info 
 */
eHalStatus halTpe_GetStaDescRateInfo(tpAniSirGlobal pMac, 
        tANI_U32 staIdx, tTpeRateType type, 
        tpTpeStaDescRateInfo *pTpeStaDescRateInfo)
{
    tpStaStruct t = (tpStaStruct) pMac->hal.halMac.staTable;
    tANI_U32 address, offset;

    if (staIdx > pMac->hal.halMac.maxSta) {
        return eHAL_STATUS_INVALID_STAIDX;
    }

    // Calculate the start address of the TPE descriptor for this STA
    address = pMac->hal.memMap.tpeStaDesc_offset + 
        (staIdx * TPE_STA_DESC_AND_STATS_SIZE);

    offset = offsetof(tTpeStaDesc, rate_params_20Mhz);

    // Read the rate info from the TPE descriptor into memory
    halReadDeviceMemory(pMac, (address + offset),
            (tANI_U8 *)&t[staIdx].tpeStaDesc.rate_params_20Mhz[0], sizeof(tTpeStaDescRateInfo)*TPE_STA_MAX_RETRY_RATE*TPE_STA_RATE_TYPE_MAX);

    switch(type) {
        case TPE_STA_20MHZ_RATE:
            *pTpeStaDescRateInfo = &t[staIdx].tpeStaDesc.rate_params_20Mhz[0];
            break;
        case TPE_STA_40MHZ_RATE:
            *pTpeStaDescRateInfo = &t[staIdx].tpeStaDesc.rate_params_40Mhz[0];
            break;
        case TPE_STA_BD_RATE:
            *pTpeStaDescRateInfo = &t[staIdx].tpeStaDesc.bd_rate_params[0];
            break;
        default:
            break;
    }

    return eHAL_STATUS_SUCCESS;
}



/**
 * \fn halTpe_UpdateStaDescRateInfo
 *
 * \brief Update the TPE station Rate Info
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param staId Station Id to set the descriptor
 *
 * \param tANI_U32 channel  Channel
 *
 * \param tpTpeStaDescRateInfo ptpeStaDescRateInfo
 *
 * \return eHalStatus Station Rate Info configuration status
 */
eHalStatus halTpe_UpdateStaDescRateInfo(tpAniSirGlobal pMac, tANI_U32 staIdx,
                                tANI_U32 channel, tpTpeStaDescRateInfo ptpeStaDescRateInfo)
{
    tANI_U32            address;

    address = pMac->hal.memMap.tpeStaDesc_offset + (staIdx * TPE_STA_DESC_AND_STATS_SIZE);

    switch(channel) {

        case TPE_STA_20MHZ_RATE:
            /* Write to TPE STA Desc memory */
            halWriteDeviceMemory(pMac, 
                    address + offsetof(tTpeStaDesc, rate_params_20Mhz),
                                        (tANI_U8 *)ptpeStaDescRateInfo,
                    sizeof(tTpeStaDescRateInfo) * TPE_STA_MAX_RETRY_RATE); //Need to update PRI/SEC/TERTIARY rates. So multiply by 3
            break;

        case TPE_STA_40MHZ_RATE:
            /* Write to TPE STA Desc memory */
            halWriteDeviceMemory(pMac, 
                    address + offsetof(tTpeStaDesc, rate_params_40Mhz),
                                        (tANI_U8 *)ptpeStaDescRateInfo,
                    sizeof(tTpeStaDescRateInfo) * TPE_STA_MAX_RETRY_RATE); //Need to update PRI/SEC/TERTIARY rates. So multiply by 3
            break;

        case TPE_STA_BD_RATE:
            /* Write to TPE STA Desc memory */
            halWriteDeviceMemory(pMac, 
                    address + offsetof(tTpeStaDesc, bd_rate_params),
                                        (tANI_U8 *)ptpeStaDescRateInfo,
                    sizeof(tTpeStaDescRateInfo) * TPE_STA_MAX_RETRY_RATE); //Need to update PRI/SEC/TERTIARY rates. So multiply by 3
            break;
        default:
            break;
    }

    return eHAL_STATUS_SUCCESS;
}

/**
 * \fn halTpe_UpdateEdcaTxOp
 *
 * \brief Update the TPE EDCA TXOP
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param pTxOp  txop for Back Off 0 - 7
 *
 * \return eHalStatus Station Rate Info configuration status
 */
eHalStatus halTpe_UpdateEdcaTxOp(tpAniSirGlobal pMac, tANI_U16 *pTxOp)
{

    halWriteRegister(pMac, QWLAN_TPE_EDCF_TXOP_0_1_REG,
            pTxOp[MTU_BKID_0]|(pTxOp[MTU_BKID_1]<<QWLAN_TPE_EDCF_TXOP_0_1_EDCF_TXOP_1_OFFSET));

    halWriteRegister(pMac, QWLAN_TPE_EDCF_TXOP_2_3_REG,
            pTxOp[MTU_BKID_2]|(pTxOp[MTU_BKID_3]<<QWLAN_TPE_EDCF_TXOP_2_3_EDCF_TXOP_3_OFFSET));

    halWriteRegister(pMac, QWLAN_TPE_EDCF_TXOP_4_5_REG,
            pTxOp[MTU_BKID_4]|(pTxOp[MTU_BKID_5]<<QWLAN_TPE_EDCF_TXOP_4_5_EDCF_TXOP_5_OFFSET));

    halWriteRegister(pMac, QWLAN_TPE_EDCF_TXOP_6_7_REG,
            pTxOp[MTU_BKID_6]|(pTxOp[MTU_BKID_7]<<QWLAN_TPE_EDCF_TXOP_6_7_EDCF_TXOP_7_OFFSET));

    return eHAL_STATUS_SUCCESS;
}

/**
 * \fn halTpe_DumpEdcaTxOp
 *
 * \brief dumps the TPE EDCA TXOP
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param pTxOp  txop for Back Off 0 - 7
 *
 * \return none
 */

void halTpe_DumpEdcaTxOp(tpAniSirGlobal pMac)
{
    tANI_U32 regVal = 0;

    halReadRegister(pMac, QWLAN_TPE_EDCF_TXOP_0_1_REG, &regVal);
    HALLOGW( halLog(pMac, LOGW, FL("QWLAN_TPE_EDCF_TXOP_0_1_REG = %x\n"), regVal));

    halReadRegister(pMac, QWLAN_TPE_EDCF_TXOP_2_3_REG, &regVal);
    HALLOGW(halLog(pMac, LOGW, FL("QWLAN_TPE_EDCF_TXOP_2_3_REG = %x\n"), regVal));        

    halReadRegister(pMac, QWLAN_TPE_EDCF_TXOP_4_5_REG, &regVal);
    HALLOGW(halLog(pMac, LOGW, FL("QWLAN_TPE_EDCF_TXOP_4_5_REG = %x\n"), regVal));        

    halReadRegister(pMac, QWLAN_TPE_EDCF_TXOP_6_7_REG, &regVal);
    HALLOGW(halLog(pMac, LOGW, FL("QWLAN_TPE_EDCF_TXOP_6_7_REG = %x\n"), regVal));        

    return;
}

/**
 * \fn     :   halTpe_UpdateMtuMaxBssid
 *
 * \brief  :   Update the Max Bssid Based on current MTU configuration.
 *
 * \param  :   tpAniSirGlobal pMac     : Handle to Mac Structure.
 *
 * \return :   eHalStatus
 */

void halTpe_UpdateMtuMaxBssid(tpAniSirGlobal pMac)
{
    tANI_U8 activeBssCnt;

    /** Get the Active Bss count */
    /** \bug MTU number of active BSS could be misleading as the HAL number of active Bss
     *  needs to be used where the MTU may not reflect the number of BSS
     *  as could be cleared to disable beacon going out of a paticular BSS.
     */
    halMTU_GetActiveBss(pMac, &activeBssCnt);

    halMTU_UpdateNumBSS(pMac, activeBssCnt);
}

/**
 * \fn halTpe_DisableBeacon
 *
 * \brief Disable the Beacon transmission
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param beaconIndex Beacon Index
 *
 * \return eHalStatus Beacon disable status
 */
eHalStatus halTpe_DisableBeacon(tpAniSirGlobal pMac, tANI_U16 beaconIndex)
{
    eHalStatus status;

    /** \todo Need to verify if this is going to work even for single/multiple BSS*/
    status = halMTU_UpdateValidBssid(pMac, beaconIndex, eHAL_CLEAR);

    return status;
}

/**
 * \fn halTpe_ReEnableBeacon
 *
 * \brief Disable the Beacon transmission
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param beaconIndex Beacon Index
 *
 * \return eHalStatus Beacon disable status
 */
eHalStatus halTpe_ReEnableBeacon(tpAniSirGlobal pMac, tANI_U16 beaconIndex)
{
    eHalStatus status;

    /** \todo Need to verify if this is going to work even for single/multiple BSS*/
    status = halMTU_UpdateValidBssid(pMac, beaconIndex, eHAL_SET);

    return status;
}

#ifndef WLAN_SOFTAP_FEATURE
/**
 * \fn halTpe_UpdateBeaconMemory
 *
 * \brief Function to Update the beacon memory
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param beacon Pointer to beacon memory to update
 *
 * \param beaconIndex Beacon Index to update
 *
 * \param length Length of the beacon body
 *
 * \return eHalStatus Beacon memory update status
 */
void halTpe_UpdateBeaconMemory(tpAniSirGlobal pMac, tANI_U8 *beacon,
                                    tANI_U16 beaconIndex, tANI_U32 length)
{
    tBeaconTemplate beaconTemplate;
    tTpeRateIdx rateIndex;
    tANI_U32 beaconOffset;
    tANI_U32 alignedLen;

    beaconTemplate.template_header.template_type = SIR_MAC_MGMT_FRAME;
    beaconTemplate.template_header.template_sub_type = SIR_MAC_MGMT_BEACON;
    beaconTemplate.template_header.resp_is_expected = 0;
    beaconTemplate.template_header.expected_resp_type = 0;
    beaconTemplate.template_header.expected_resp_sub_type = 0;
    beaconTemplate.template_header.reserved1 = 0;
    beaconTemplate.template_header.stbc = 0;
    beaconTemplate.template_header.reserved2 = 0;
    beaconTemplate.template_header.reserved3 = 0;
    beaconTemplate.template_header.tx_power = 0;
    beaconTemplate.template_header.tx_antenna_enable = 0;
    beaconTemplate.template_header.tsf_offset = TPE_BEACON_1MBPS_LONG_TSF_OFFSET;
    beaconTemplate.template_header.reserved4 = 0;
    beaconTemplate.template_header.reserved5 = 0;
    
    halGetBcnRateIdx(pMac, &rateIndex);
    beaconTemplate.template_header.primary_data_rate_index = rateIndex;


    beaconTemplate.template_header.template_len = length + BEACON_TEMPLATE_CRC;

    beaconOffset = pMac->hal.memMap.beaconTemplate_offset + (beaconIndex * BEACON_TEMPLATE_SIZE);


    halWriteDeviceMemory(pMac, beaconOffset ,
                            (tANI_U8 *)&beaconTemplate.template_header, BEACON_TEMPLATE_HEADER);

    //FIXME: halWriteDevicememory requires lenght to be mulltiple of four and aligned to 4 byte boundry.
    alignedLen = ( beaconTemplate.template_header.template_len + 3 ) & ~3 ;

    // beacon body need to be swapped sicne there is another swap occurs while BAL writes
    // the beacon to Libra.
    sirSwapU32BufIfNeeded((tANI_U32*)beacon, alignedLen>>2);

     halWriteDeviceMemory(pMac, beaconOffset + BEACON_TEMPLATE_HEADER,
                            (tANI_U8 *)beacon, alignedLen );

}

#else
/**
 * \fn halTpe_UpdateBeaconMemory
 *
 * \brief Function to Update the beacon memory
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param beacon Pointer to beacon memory to update
 *
 * \param beaconIndex Beacon Index to update
 *
 * \param length Length of the beacon body
 *
 * \return eHalStatus Beacon memory update status
 */
void halTpe_UpdateBeaconMemory(tpAniSirGlobal pMac, tANI_U8 *beacon,
                                    tANI_U16 beaconIndex, tANI_U32 length)
{
    tBeaconTemplateHeader beaconTemplateHeader;
    tPwrTemplateIndex txPower = 0;    
    tTpeRateIdx rateIndex;
    tANI_U32 beaconOffset;
    tANI_U32 alignedLen;

    beaconTemplateHeader.template_type = SIR_MAC_MGMT_FRAME;
    beaconTemplateHeader.template_sub_type = SIR_MAC_MGMT_BEACON;
    beaconTemplateHeader.resp_is_expected = 0;
    beaconTemplateHeader.expected_resp_type = 0;
    beaconTemplateHeader.expected_resp_sub_type = 0;
    beaconTemplateHeader.reserved1 = 0;
    beaconTemplateHeader.stbc = 0;
    beaconTemplateHeader.reserved2 = 0;
    beaconTemplateHeader.reserved3 = 0;
    beaconTemplateHeader.tx_antenna_enable = 0;
    beaconTemplateHeader.tsf_offset = TPE_BEACON_1MBPS_LONG_TSF_OFFSET;
    beaconTemplateHeader.reserved4 = 0;
    beaconTemplateHeader.reserved5 = 0;
    
    halGetBcnRateIdx(pMac, &rateIndex);
    halRate_getPowerIndex(pMac, rateIndex, &txPower);
    beaconTemplateHeader.primary_data_rate_index = rateIndex;
    beaconTemplateHeader.tx_power = txPower;

    beaconTemplateHeader.template_len = length + BEACON_TEMPLATE_CRC;

    beaconOffset = pMac->hal.memMap.beaconTemplate_offset + (beaconIndex * BEACON_TEMPLATE_SIZE);

    halWriteDeviceMemory(pMac, beaconOffset ,
                            (tANI_U8 *)&beaconTemplateHeader, BEACON_TEMPLATE_HEADER);

    //FIXME: halWriteDevicememory requires lenght to be mulltiple of four and aligned to 4 byte boundry.
    alignedLen = (( beaconTemplateHeader.template_len + 3 ) & (~3)) ;

    // beacon body need to be swapped sicne there is another swap occurs while BAL writes
    // the beacon to Libra.
    sirSwapU32BufIfNeeded((tANI_U32*)beacon, alignedLen>>2);

    halWriteDeviceMemory(pMac, beaconOffset + BEACON_TEMPLATE_HEADER,
                            (tANI_U8 *)beacon, alignedLen );

    halLog(pMac, LOGW, FL("halTpe_UpdateBeaconMemory beacon template \n"));
    sirDumpBuf(pMac, SIR_HAL_MODULE_ID, LOGW, (tANI_U8*)&beaconTemplateHeader, alignedLen);    
    sirDumpBuf(pMac, SIR_HAL_MODULE_ID, LOGW, beacon, alignedLen);
}

#endif
/**
 * \fn halTpe_UpdateBeacon
 *
 * \brief Function to Update the beacon params
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param beacon Pointer to beacon memory to update
 *
 * \param beaconIndex Beacon Index to update
 *
 * \param length Length of the beacon body
 *
 * \return eHalStatus Beacon update status
 */
 #ifdef WLAN_SOFTAP_FEATURE
eHalStatus halTpe_UpdateBeacon(tpAniSirGlobal pMac, tANI_U8 *beacon,
                                    tANI_U16 bssIndex, tANI_U32 length, tANI_U16 timIeOffset)
{
    tANI_U32 value;
    tANI_U32 loop = 0;

    /** Update beacon request */
    halReadRegister(pMac, QWLAN_TPE_SW_BEACON_MAX_SIZE_MISC_REG, &value);

    /** Initially clear the sw update beacon request */
    /* also clear the beacon template size */
    value &= ~(QWLAN_TPE_SW_BEACON_MAX_SIZE_MISC_SW_UPDATE_BCN_REQ_MASK | QWLAN_TPE_SW_BEACON_MAX_SIZE_MISC_SW_BEACON_MAX_SIZE_MASK);

    halWriteRegister(pMac,  QWLAN_TPE_SW_BEACON_MAX_SIZE_MISC_REG, value);

    /** Set the sw update beacon request */
    value |= QWLAN_TPE_SW_BEACON_MAX_SIZE_MISC_SW_UPDATE_BCN_REQ_MASK |
         (BEACON_TEMPLATE_SIZE << QWLAN_TPE_SW_BEACON_MAX_SIZE_MISC_SW_BEACON_MAX_SIZE_OFFSET) | 
         (bssIndex << QWLAN_TPE_SW_BEACON_MAX_SIZE_MISC_SW_UPDATE_BCN_INDEX_OFFSET);

    halWriteRegister(pMac,  QWLAN_TPE_SW_BEACON_MAX_SIZE_MISC_REG, value);

    /** Poll on sw update beacon request bit to set to grant the update request */
    do {
        halReadRegister(pMac , QWLAN_TPE_SW_BEACON_MAX_SIZE_MISC_REG, &value);
        loop++;

        if (loop > 1000) {
            HALLOGW(halLog(pMac, LOGW, FL("HAL TPE sw update beacon request Interrupt not generated \n")));
            break;
        }

    } while (!(value & QWLAN_TPE_SW_BEACON_MAX_SIZE_MISC_MAIN_SW_UPDATE_BCN_GNT_BY_HW_MASK));

    /** Update Beacon Memory */
    halTpe_UpdateBeaconMemory(pMac, beacon, bssIndex, length);

    /** Unlock the beacon ownership */
    halReadRegister(pMac, QWLAN_TPE_SW_BEACON_MAX_SIZE_MISC_REG, &value);

    value |= QWLAN_TPE_SW_BEACON_MAX_SIZE_MISC_SW_BCN_UN_LOCK_BY_SW_MASK;

    halWriteRegister(pMac, QWLAN_TPE_SW_BEACON_MAX_SIZE_MISC_REG, value);

    if(halFW_UpdateBeaconReq(pMac, (tANI_U8) bssIndex, (tANI_U16)timIeOffset) != eHAL_STATUS_SUCCESS)
    {
        HALLOGE(halLog(pMac, LOGE, FL("halFW_UpdateBeaconReq failed")));
        return eHAL_STATUS_FAILURE;
    }
    
    return eHAL_STATUS_SUCCESS;
}

#else
eHalStatus halTpe_UpdateBeacon(tpAniSirGlobal pMac, tANI_U8 *beacon,
                                    tANI_U16 beaconIndex, tANI_U32 length)
{
    tANI_U32 value;
    tANI_U32 loop = 0;
    
    /** Update beacon request */
    halReadRegister(pMac, QWLAN_TPE_SW_BEACON_MAX_SIZE_MISC_REG, &value);

    /** Initially clear the sw update beacon request */
    value &= ~QWLAN_TPE_SW_BEACON_MAX_SIZE_MISC_SW_UPDATE_BCN_REQ_MASK;

    halWriteRegister(pMac,  QWLAN_TPE_SW_BEACON_MAX_SIZE_MISC_REG, value);

    /** Set the sw update beacon request */
    value |= QWLAN_TPE_SW_BEACON_MAX_SIZE_MISC_SW_UPDATE_BCN_REQ_MASK |
            BEACON_TEMPLATE_SIZE | (beaconIndex << QWLAN_TPE_SW_BEACON_MAX_SIZE_MISC_SW_UPDATE_BCN_INDEX_OFFSET);
    halWriteRegister(pMac,  QWLAN_TPE_SW_BEACON_MAX_SIZE_MISC_REG, value);

    /** Poll on sw update beacon request bit to set to grant the update request */
    do {
        halReadRegister(pMac , QWLAN_TPE_SW_BEACON_MAX_SIZE_MISC_REG, &value);
        loop++;

        if (loop > 1000) {
            HALLOGW(halLog(pMac, LOGW, FL("HAL TPE sw update beacon request Interrupt not generated \n")));
            break;
        }

    } while (!(value & QWLAN_TPE_SW_BEACON_MAX_SIZE_MISC_MAIN_SW_UPDATE_BCN_GNT_BY_HW_MASK));

    /** Update Beacon Memory */
    halTpe_UpdateBeaconMemory(pMac, beacon, beaconIndex, length);

    /** Unlock the beacon ownership */
    halReadRegister(pMac, QWLAN_TPE_SW_BEACON_MAX_SIZE_MISC_REG, &value);

    value |= QWLAN_TPE_SW_BEACON_MAX_SIZE_MISC_SW_BCN_UN_LOCK_BY_SW_MASK;

    halWriteRegister(pMac, QWLAN_TPE_SW_BEACON_MAX_SIZE_MISC_REG, value);

    return eHAL_STATUS_SUCCESS;
}

#endif
/**
 * \fn halTpe_UpdateMaxMpduInAmpdu
 *
 * \brief Function to Set max MPDU in AMPDU
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param tANI_U32 cfgValue
 *
 * \return None
 */
void halTpe_UpdateMaxMpduInAmpdu(tpAniSirGlobal pMac, tANI_U32 mpduInAmpdu)
{
    tANI_U32 value = 0;
    halReadRegister(pMac, QWLAN_TPE_SW_MAX_MPDUS_IN_AMPDU_AND_MISC_REG, &value);
    value &= ~QWLAN_TPE_SW_MAX_MPDUS_IN_AMPDU_AND_MISC_SW_MAX_MPDUS_IN_AMPDU_MASK;
    value |= mpduInAmpdu;
    halWriteRegister(pMac, QWLAN_TPE_SW_MAX_MPDUS_IN_AMPDU_AND_MISC_REG, value);
}

/**
 * \fn halTpe_TerminateAmpduAtRateChange
 *
 * \brief Function to set/unset bit to terminate AMPDU transmission on rate change
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param tANI_U8 enable/disable
 *
 * \return None
 */
void halTpe_TerminateAmpduAtRateChange(tpAniSirGlobal pMac, tANI_U8 enable)
{
    tANI_U32 value = 0;
    palReadRegister(pMac->hHdd, QWLAN_TPE_SW_MAX_MPDUS_IN_AMPDU_AND_MISC_REG, &value);
    if(enable) {
        value |= QWLAN_TPE_SW_MAX_MPDUS_IN_AMPDU_AND_MISC_SW_TX_AMPDU_TERMINATE_AT_RATE_CHANGE_MASK;
    } else {
        value &= ~QWLAN_TPE_SW_MAX_MPDUS_IN_AMPDU_AND_MISC_SW_TX_AMPDU_TERMINATE_AT_RATE_CHANGE_MASK;
    }
    palWriteRegister(pMac->hHdd, QWLAN_TPE_SW_MAX_MPDUS_IN_AMPDU_AND_MISC_REG, value);
}


/**
 * \fn halTpe_SetAmpduTxTime
 *
 * \brief Function to Set AMPDU Tx time for both GF and Mixed mode
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param tANI_U32 maxAmpduTxTime : TX time to be set for both GF and Mixed mode.
 *
 * \return None
 */
void halTpe_SetAmpduTxTime(tpAniSirGlobal pMac, tANI_U32 maxAmpduTxTime)
{
    tANI_U32 value = 0;
    value = (maxAmpduTxTime << QWLAN_TPE_SW_MAX_BYTES_AND_TX_TIME_IN_AMPDU_SW_MAX_AMPDU_TX_TIME_OFFSET) |
            (maxAmpduTxTime << QWLAN_TPE_SW_MAX_BYTES_AND_TX_TIME_IN_AMPDU_SW_MAX_AMPDU_TX_TIME_FOR_MIXED_MODE_OFFSET);
    halWriteRegister(pMac, QWLAN_TPE_SW_MAX_BYTES_AND_TX_TIME_IN_AMPDU_REG, value);
}

/**
 * \fn halTpe_SetAcToBkofLookupVec
 *
 * \brief Function to Set AC to Back-Off lookup vector
 *
 * \param pMac The global tpAniSirGlobal object
 *

 * \return eHalStatus
 */

eHalStatus halTpe_SetAcToBkofLookupVec(tpAniSirGlobal pMac)
{
    tANI_U32 value = 0;
    tANI_U32 ac = 0;
    tANI_U16 acBkOffMap = 0;

    for(ac = 0; ac < MAX_NUM_AC; ac++)
    {
        acBkOffMap |=  (ac & 0x3) << (__halMTU_ac2BkoffIndex(pMac, ac) * 2);
    }

    halReadRegister(pMac, QWLAN_TPE_SW_ADMISSION_CONTROL_CONTROL_REG_REG, &value);

    value &= ~QWLAN_TPE_SW_ADMISSION_CONTROL_CONTROL_REG_SW_AC_TO_BKOF_LOOKUP_VECTOR_MASK;

    value |= ((acBkOffMap << QWLAN_TPE_SW_ADMISSION_CONTROL_CONTROL_REG_SW_AC_TO_BKOF_LOOKUP_VECTOR_OFFSET)
        & QWLAN_TPE_SW_ADMISSION_CONTROL_CONTROL_REG_SW_AC_TO_BKOF_LOOKUP_VECTOR_MASK);

    halWriteRegister(pMac, QWLAN_TPE_SW_ADMISSION_CONTROL_CONTROL_REG_REG, value);

    return eHAL_STATUS_SUCCESS;
}

/**
 * \fn halTpe_SetLsigTxopProtection
 *
 * \brief Function to Set LSIG Txop Protection
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param tANI_U8 lsigProtFlag
 *
 * \param tANI_U8 lsigProtType
 *
 * \return eHalStatus
 */
eHalStatus halTpe_SetLsigTxopProtection(tpAniSirGlobal pMac, tANI_U8 lsigProtFlag, tANI_U8 lsigProtType)
{
    tANI_U32 value;

    halReadRegister(pMac, QWLAN_TPE_SW_PM_REG, &value);

    if (lsigProtFlag)
        value |= QWLAN_TPE_SW_PM_SW_LSIG_PROTECTION_VALID_MASK;
    else
        value &= ~QWLAN_TPE_SW_PM_SW_LSIG_PROTECTION_VALID_MASK;

    if (lsigProtType)
        value |= QWLAN_TPE_SW_PM_SW_LSIG_PROTECTION_TYPE_MASK;
    else
        value &= ~QWLAN_TPE_SW_PM_SW_LSIG_PROTECTION_TYPE_MASK;

    halWriteRegister(pMac, QWLAN_TPE_SW_PM_REG, value);

    return eHAL_STATUS_SUCCESS;
}

/**
 * \fn halTpe_CalculateAmpduDensity
 *
 * \brief Function to calculate ampdu density
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param tANI_U32 rateIdx
 *
 * \param tANI_U32 *pAmpduDensity
 *
 * \param tANI_U32 maxAmpduDensity
 *
 * \return eHalStatus
 */
eHalStatus halTpe_CalculateAmpduDensity(tpAniSirGlobal pMac, tANI_U32 rateIdx, tANI_U32 *pAmpduDensity, tANI_U32 maxAmpduDensity)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tANI_U32 mpduSpace;

    /* Get MPDU space bytes */
    mpduSpace = halRate_GetMpduSpaceByTpeRate(rateIdx, maxAmpduDensity);

    /* Convert MPDU space bytes to number of delimiters */
    *pAmpduDensity = (mpduSpace + 3) / 4;

    return status;
}

eHalStatus halIntTpeMcuBdBasedTxInt1PHostHandler( tHalHandle hHalHandle, eHalIntSources intSource )
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tANI_U32 intRegMask;
    tANI_U32 intRegStatus;
    tpAniSirGlobal pMac = PMAC_STRUCT(hHalHandle);
    tANI_U32 regVal = 0;

    /** Read Interrupt Status.*/
    status = halIntGetErrorStatus(hHalHandle, intSource, &intRegStatus, &intRegMask);
    if (status != eHAL_STATUS_SUCCESS) {
        return status;
    }

    intRegStatus &= intRegMask;   

    if(intRegStatus){
        tSirMsgQ        msg;
        tANI_U32        txCompleteSuccess = 0;
        tANI_U32        txCompFeedback = 0;

        halReadRegister(pMac, TPE_HOST_TPE_TX_COMPLETE_FEEDBACK_REG, &regVal);
        txCompFeedback = regVal & TPE_TX_COMPLETE_FEEDBACK_MASK;
        txCompleteSuccess = (!(txCompFeedback & TPE_ACK_TO_MASK)); //if the ack is valid
        if(txCompleteSuccess || IS_TPE_TX_COMPLETE_FEEDBACK_FRAME_DROPPED(txCompFeedback))
        {
            //Let the frame transmitter know whether ack from peer was received or it was dropped after reaching max retry limit.
            //ignore when the interrupts are received because of HW retries.
            msg.type        = SIR_HAL_TX_COMPLETE_IND;
            msg.bodyval     = txCompleteSuccess;
            msg.bodyptr     = NULL;
            halPostMsgApi(pMac, &msg);
        }
    } 
    return status;
}

/**
 * \fn halTpe_DumpMpiCmdTableEntry
 *
 * \brief Function to dump the TPE rate entry in the MPI cmd table
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param tANI_U32 rateIdx
 *
 * \return void
 */
#if 0
void halTpe_DumpMpiCmdTableEntry(tpAniSirGlobal pMac, tANI_U32 rateIndex)
{
    tANI_U32 word0 = 0;
    tANI_U32 word1 = 0;

    halTpe_RateTableRead(pMac, rateIndex, &word0, &word1);


    HALLOGW( halLog(pMac, LOGW, FL("Word0        = 0x%08x, Word1 = 0x%08x\n"), word0, word1));
    HALLOGW( halLog(pMac, LOGW, FL("PktType      = (0x%x)"), (word0 & 0x3), (word0 & 0x3))); //PKTTYPE(word0), PKTTYPE(word0)));
    HALLOGW( halLog(pMac, LOGW, FL("PsduRate     = (0x%x)"), (word0 & (((1<<9)-1)<<2))>>2));//PSDURATE(word0), PSDURATE(word0)));
    HALLOGW( halLog(pMac, LOGW, FL("BandWidth    = (0x%x)"), (word0 & (((1<<2)-1)<<9))>>9));//BANDWIDTH(word0), BANDWIDTH(word0)));
    HALLOGW( halLog(pMac, LOGW, FL("NS11b        = (0x%x)"), (word0 & (((1<<2)-1)<<11))>>11));//NS11b(word0), NS11b(word0)));
    HALLOGW( halLog(pMac, LOGW, FL("Short Guard  = (0x%x)"), (word0 & (((1)<<13))>>13)));//SHORTGUARD(word0), SHORTGUARD(word0)));
    HALLOGW( halLog(pMac, LOGW, FL("Ctrl Rate    = (0x%x)"), (word0 & (((1<<9)-1)<<14))>>14));//CTRLRATEIDX(word0), CTRLRATEIDX(word0)));
    HALLOGW( halLog(pMac, LOGW, FL("Tx Antenna   = (0x%x)"), (word0 & (((1<<3)-1)<<23))>>23));//TXANTENNA(word0), TXANTENNA(word0)));
    HALLOGW( halLog(pMac, LOGW, FL("NES          = (0x%x)"), (word0 & (((1))<<26))>>26));//NES(word0), NES(word0)));
    HALLOGW( halLog(pMac, LOGW, FL("TxPower      = (0x%x)"), (word0 & (((1<<5)-1)<<27))>>27));//TXPOWER(word0), TXPOWER(word0)));
    HALLOGW( halLog(pMac, LOGW, FL("Rsp Rate     = (0x%x)"), (word1 & (((1<<9)-1)))));//RSPRATEIDX(word1), RSPRATEIDX(word1)));
    HALLOGW( halLog(pMac, LOGW, FL("AMPDU        = (0x%x)"), (word1 & (((1))<<9))>>9));//AMPDU(word1), AMPDU(word1)));
    HALLOGW( halLog(pMac, LOGW, FL("NDBPS4TRATE  = (0x%x)"), (word1 & (((1<<12)-1)<<10))>>10));//NDBPS4TRATE(word1), NDBPS4TRATE(word1)));
    HALLOGW( halLog(pMac, LOGW, FL("NDLTFS       = (0x%x)"), (word1 & (((1<<2)-1)<<22))>>22));//NDLTFS(word1), NDLTFS(word1)));
    HALLOGW( halLog(pMac, LOGW, FL("NELTFS       = (0x%x)"), (word1 & (((1<<2)-1)<<24))>>24));//NELTFS(word1), NELTFS(word1)));
    HALLOGW( halLog(pMac, LOGW, FL("PCTRLRATEIDX = (0x%x)"), (word1 & (((1<<4)-1)<<26))>>26));//PCTRLRATEIDX(word1), PCTRLRATEIDX(word1)));
    HALLOGW( halLog(pMac, LOGW, FL("STBC         = (0x%x)"), (word1 & (((1<<2)-1)<<30))>>30));//STBC(word1), STBC(word1)));

}
#endif 

void halTpe_DumpMpiCmdTableEntry(tpAniSirGlobal pMac, tANI_U32 rateIndex)
{
#ifdef WLAN_HAL_VOLANS
    tANI_U32 data = 0;

    halTpe_RateTableRead(pMac, rateIndex, &data);

//  halLog(pMac, LOGE, "data = 0x%08x, data);
    HALLOGE(halLog(pMac, LOGE, FL("%d: %02d, %02d, %02d, %02d, %02d, %02d, %02d, %02d, %02d"),
        rateIndex, (data & 0x3), ((data & (((1<<9)-1)<<2))>>2),
        ((data & (((1<<2)-1)<<9))>>9), ((data & (((1<<2)-1)<<11))>>11),
        ((data & (((1)<<13))>>13)), ((data & (((1<<9)-1)<<14))>>14),
        ((data & (((1<<3)-1)<<23))>>23), ((data & (((1))<<26))>>26),
        ((data & (((1ul<<5)-1)<<27))>>27)));
#else
    tANI_U32 word0 = 0;
    tANI_U32 word1 = 0;

    halTpe_RateTableRead(pMac, rateIndex, &word0, &word1);

//  halLog(pMac, LOGE, "Word0 = 0x%08x, Word1 = 0x%08x\n", word0, word1);
    HALLOGE(halLog(pMac, LOGE, FL("%d: %02d, %02d, %02d, %02d, %02d, %02d, %02d, %02d, %02d, %02d, %02d, %02d, %02d, %02d, %02d, %02d"),
        rateIndex, (word0 & 0x3), ((word0 & (((1<<9)-1)<<2))>>2),
        ((word0 & (((1<<2)-1)<<9))>>9), ((word0 & (((1<<2)-1)<<11))>>11),
        ((word0 & (((1)<<13))>>13)), ((word0 & (((1<<9)-1)<<14))>>14),
        ((word0 & (((1<<3)-1)<<23))>>23), ((word0 & (((1))<<26))>>26),
        ((word0 & (((1ul<<5)-1)<<27))>>27), ((word1 & (((1<<9)-1)))),
        ((word1 & (((1))<<9))>>9), ((word1 & (((1<<12)-1)<<10))>>10),
        ((word1 & (((1<<2)-1)<<22))>>22), ((word1 & (((1<<2)-1)<<24))>>24),
        ((word1 & (((1<<4)-1)<<26))>>26), ((word1 & (((1ul<<2)-1)<<30))>>30)));
#endif
}

void halTpe_PrintMpiCmdTable(tpAniSirGlobal pMac)
{
    tANI_U8 rateIndex = 0;

    HALLOGE(halLog(pMac, LOGE, FL("Rate, PktType, PsduRate, BandWidth, NS11b, Short Guard, Ctrl Rate, Tx Antenna, NES, TxPower, Rsp Rate, AMPDU, NDBPS4TRATE, NDLTFS, NELTFS, PCTRLRATEIDX, STBC")));
    for( rateIndex=0; rateIndex<68; rateIndex++) {
        halTpe_DumpMpiCmdTableEntry(pMac, rateIndex);
    }
}

#ifdef CONFIGURE_SW_TEMPLATE
/**
 * \brief Configure TPE SW Template Base
 *
 * \sa halTpe_sw_template_base
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param swTemplate_offset SW Template Base adress
 *
 * \return eHalStatus SW Template Base configuration status
 */
eHalStatus halTpe_InitSwTemplateBase(tpAniSirGlobal pMac, 
                tANI_U32 swTemplate_offset)
{
    /** Zero out the TPE SW Template address */
    halZeroDeviceMemory(pMac,    pMac->hal.memMap.swTemplate_offset, 
        pMac->hal.memMap.swTemplate_size);
        
    
    halWriteRegister(pMac, QWLAN_TPE_SW_SOFTWARE_TX_ADDRESS_REG, 
        swTemplate_offset);


    return eHAL_STATUS_SUCCESS;
}
#endif //CONFIGURE_SW_TEMPLATE

/*
 * Function to set the sw_11g_ctrl_index_offset_valid bit when 11G protection is enabled.
 * This makes sure that the control frames go out at the 11b rates instead of 11G rates.
 */
eHalStatus halTpe_Set11gProtectionCntrlIndex(tpAniSirGlobal pMac, tANI_U8 set)
{
    tANI_U32 regValue = 0;

    // Read the value of the 11G control frame index register
    halReadRegister(pMac, QWLAN_TPE_SW_11G_CTRL_INDEX_OFFSET_REG, &regValue);

    if(set) {
        regValue |= QWLAN_TPE_SW_11G_CTRL_INDEX_OFFSET_SW_11G_CTRL_INDEX_OFFSET_VALID_MASK;
    } else {
        regValue &= ~QWLAN_TPE_SW_11G_CTRL_INDEX_OFFSET_SW_11G_CTRL_INDEX_OFFSET_VALID_MASK;
    }

    halWriteRegister(pMac, QWLAN_TPE_SW_11G_CTRL_INDEX_OFFSET_REG, regValue);

    return eHAL_STATUS_SUCCESS;
}

#ifdef WLAN_FEATURE_VOWIFI

/* 
* \brief Updates the TX power for rates in TPE station descriptor for the given staIdx
*
*
* \param pMac The global tpAniSirGlobal object
*
* \param staIdx Index of station descriptor that needs to be updated
*
*\param type - Type of the rate to be updated. 20MHz/40MHz/BD rates
*
*\param maxPwrIndex - Max power that is allowed for transmission
*
* \return eHalStatus SW Template Base configuration status
*/

eHalStatus halTpe_UpdateDescPowerParams(tpAniSirGlobal pMac, tANI_U8 staIdx, 
                            tTpeRateType type, tPwrTemplateIndex maxPwrIndex)
{
    tpTpeStaDescRateInfo    tpeRateInfo;
    tANI_U8                 index;
    tANI_BOOLEAN            pwrChanged = FALSE;
    eHalStatus              status = eHAL_STATUS_FAILURE;

    /* Get the current rate info in the TPE descriptor for the given staIdx. This would 
           also give the current TX power index used for all the rates. This function returns the 
           pointer to the rateDesc from the staTable. So, any update here will automatically 
           update the staTable */
    status = halTpe_GetStaDescRateInfo(pMac, staIdx, type, &tpeRateInfo);
    if (eHAL_STATUS_SUCCESS == status)
    {
        for (index = 0; index < TPE_STA_MAX_RETRY_RATE; index++)
        {
            /* Check the current power in TPE sta desc is above the max pwr allowed 
                        and update it correspondingly for all the rates. (PRI/SEC/TER)*/
            if (maxPwrIndex < tpeRateInfo[index].tx_power)
            {
                tpeRateInfo[index].tx_power = maxPwrIndex;
                pwrChanged = TRUE;
            }
        }
        /* If anything got changed, update the TPE station descriptor in the hardware */
        if (TRUE == pwrChanged)
        {
            status = halTpe_UpdateStaDescRateInfo(pMac, staIdx, type, tpeRateInfo);
        }
    }
    return status;
}
#endif /* WLAN_FEATURE_VOWIFI */


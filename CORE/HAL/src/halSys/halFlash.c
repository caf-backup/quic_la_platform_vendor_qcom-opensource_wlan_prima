/**
 *
 *  @file:         halFlash.c
 *
 *  @brief:        Implementation Flash related functions.
 *
 *  @author:       Sanoop Kottontavida
 *
 *  Copyright (C) 2002 - 2007, Qualcomm, Inc. All rights reserved.
 *
 *  Change History:
 * ---------------------------------------
 */

#include "aniGlobal.h"

eHalStatus halEepromOpen(tHalHandle hMac)
{
    return eHAL_STATUS_FAILURE;
}

eHalStatus halEepromClose(tHalHandle hMac)
{
    return eHAL_STATUS_FAILURE;
}

eHalStatus halGetEepromFieldSize(tHalHandle hMac, eEepromField field, tANI_U32 *fieldSize)
{
    *fieldSize = 0;
    
    return eHAL_STATUS_FAILURE;
}

eHalStatus halReadEepromField(tHalHandle hMac, eEepromField field, uEepromFields *fieldData)
{
    return eHAL_STATUS_FAILURE;
}

eHalStatus halWriteEepromField(tHalHandle hMac, eEepromField field, uEepromFields *fieldData)
{
    return eHAL_STATUS_FAILURE;
}

eHalStatus halGetEepromCksum(tHalHandle hMac, tANI_U32 *cksum, tANI_U32 *computedCksum, tANI_BOOLEAN isFixPart)
{
    return eHAL_STATUS_SUCCESS;
}

eHalStatus halSetEepromCksum(tHalHandle hMac, tANI_U32 *cksum, tANI_BOOLEAN isFixPart)
{
    return eHAL_STATUS_SUCCESS;
}

eHalStatus halStoreTableToEeprom(tHalHandle hMac, eEepromTable tableID)
{
    return eHAL_STATUS_SUCCESS;
}

eHalStatus halGetEepromTableSize(tHalHandle hMac, eEepromTable table, tANI_U32 *tableSize)
{
    *tableSize = 0;

    return eHAL_STATUS_SUCCESS;
}

eHalStatus halReadEepromTable(tHalHandle hMac, eEepromTable eepromTable, uEepromTables *tableData)
{
    return eHAL_STATUS_FAILURE;
}

eHalStatus halWriteEepromTable(tHalHandle hMac, eEepromTable eepromTable, uEepromTables *tableData)
{
    return eHAL_STATUS_FAILURE;
}

eHalStatus halGetEepromTableLoc(tHalHandle hMac, eEepromTable eepromTable, uEepromTables **tableData)
{
    return eHAL_STATUS_SUCCESS;
}

eHalStatus halIsTableInEeprom(tHalHandle hMac, eEepromTable tableID)
{
    return eHAL_STATUS_SUCCESS;
}

eHalStatus halRemoveEepromTable(tHalHandle hMac, eEepromTable eepromTable)
{
    return eHAL_STATUS_SUCCESS;
}

eHalStatus halBlankEeprom(tHalHandle hMac)
{
    return eHAL_STATUS_SUCCESS;
}

void halByteSwapEepromTable(tHalHandle hMac, eEepromTable tableID, uEepromTables *tableData)
{
}

void halEepromLog(tHalHandle hMac, eEepromTable tableID, tANI_BOOLEAN fromCache, tANI_U16 offset, tANI_U16 numDwords)
{
}

void halEepromTestHostIfCfg(tHalHandle hMac, eHostIFCfgType configType)
{
}

void halEepromTestWriteTable(tHalHandle hMac, eEepromTable eepromTable)
{
}

void halEepromWriteDefaultTable(tHalHandle hMac, eEepromTable eepromTable)
{
}

void halEepromConvertRegDomainTable(tHalHandle hMac)
{
}

void halGetSetDwordEeprom(tHalHandle hMac, tANI_BOOLEAN set, tANI_U32 offset, tANI_U32 val)
{
}

void halTurnOffOnL1Eeprom(tHalHandle hMac, tANI_BOOLEAN on)
{
}


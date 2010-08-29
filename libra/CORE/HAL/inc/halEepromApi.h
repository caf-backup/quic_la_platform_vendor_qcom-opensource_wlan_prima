/** ------------------------------------------------------------------------- *
    ------------------------------------------------------------------------- *


    \file

    \brief Function interfaces to access EEPROM

    $Id$

    Copyright (C) 2006 Airgo Networks, Incorporated


   ========================================================================== */

#ifndef HALEEPROMAPI_H
#define HALEEPROMAPI_H

#include "halEeprom.h"

eHalStatus halEepromOpen(tHalHandle hMac, sHalEeprom *pEepromEntries);
eHalStatus halEepromClose(tHalHandle hMac);
eHalStatus halGetEepromFieldSize(tHalHandle hMac, eEepromField field, tANI_U32 *fieldSize);
eHalStatus halReadEepromField(tHalHandle hMac, eEepromField field, uEepromFields *fieldData);
eHalStatus halWriteEepromField(tHalHandle hMac, eEepromField field, uEepromFields *fieldData);
eHalStatus halStoreTableToEeprom(tHalHandle hMac, eEepromTable tableID);
eHalStatus halGetEepromTableLoc(tHalHandle hMac, eEepromTable eepromTable, uEepromTables **tableData);

#ifdef ANI_PHY_DEBUG

void DisplayLnaSwGainTable(tHalHandle hMac, sLnaSwGainTable *tableData);
void DisplayCalTable(tHalHandle hMac, sInitCalValues *tableData);

#endif

#endif


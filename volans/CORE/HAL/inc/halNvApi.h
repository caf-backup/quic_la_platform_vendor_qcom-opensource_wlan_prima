/** ------------------------------------------------------------------------- *
    ------------------------------------------------------------------------- *


    \file

    \brief Function interfaces to access NV

    $Id$


    Copyright (c) 2011 Qualcomm Atheros, Inc. 
    All Rights Reserved. 
    Qualcomm Atheros Confidential and Proprietary. 
  
    Copyright (C) 2006 Airgo Networks, Incorporated

   ========================================================================== */

#ifndef HALNVAPI_H
#define HALNVAPI_H

#include "halNv.h"

eHalStatus halNvOpen(tHalHandle hMac);
eHalStatus halNvClose(tHalHandle hMac);
eHalStatus halGetNvFieldSize(tHalHandle hMac, eNvField field, tANI_U32 *fieldSize);
eHalStatus halReadNvField(tHalHandle hMac, eNvField field, uNvFields *fieldData);
eHalStatus halWriteNvField(tHalHandle hMac, eNvField field, uNvFields *fieldData);
eHalStatus halStoreTableToNv(tHalHandle hMac, eNvTable tableID);
eHalStatus halGetNvTableLoc(tHalHandle hMac, eNvTable nvTable, uNvTables **tableLoc);
eHalStatus halIsTableInNv(tHalHandle hMac, eNvTable nvTable);
eHalStatus halReadNvTable(tHalHandle hMac, eNvTable nvTable, uNvTables *tableData);
eHalStatus halWriteNvTable(tHalHandle hMac, eNvTable nvTable, uNvTables *tableData);
eHalStatus halRemoveNvTable(tHalHandle hMac, eNvTable nvTable);
eHalStatus halBlankNv(tHalHandle hMac);
void halByteSwapNvTable(tHalHandle hMac, eNvTable tableID, uNvTables *tableData);

#endif


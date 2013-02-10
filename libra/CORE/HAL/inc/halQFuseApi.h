/** ------------------------------------------------------------------------- *
    ------------------------------------------------------------------------- *


    \file

    \brief Function interfaces to access QFUSE

    $Id$

    Copyright (C) 2008 Qualcomm Technologies, Inc.


   ========================================================================== */

#ifndef HALQFUSEAPI_H
#define HALQFUSEAPI_H

void halQFusePackBits(tHalHandle hMac);
eHalStatus halQFuseWrite(tHalHandle hMac);
eHalStatus halQFuseRead(tHalHandle hMac);
eHalStatus halIsQFuseBlown(tHalHandle hMac);


#endif


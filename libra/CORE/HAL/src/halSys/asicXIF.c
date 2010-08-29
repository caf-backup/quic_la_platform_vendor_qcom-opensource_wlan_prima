/** ------------------------------------------------------------------------- *
    ------------------------------------------------------------------------- *


    \file asicXIF.c

    \brief XIF specific APIs

    $Id$

    Copyright (C) 2006 Airgo Networks, Incorporated


   ========================================================================== */

#include "halInternal.h"
#include "libraDefs.h"
#include "palApi.h"
#include "halDebug.h"

eHalStatus asicXIFReadPhyReg(tHalHandle hMac, tANI_U32 addr, tANI_U32 *pData)
{
    tpAniSirGlobal pMac = PMAC_STRUCT(hMac);
    eHalStatus status = eHAL_STATUS_FAILURE;
    tANI_U32 tmp;

    do
    {
        // Set the address
        status = halWriteRegister(pMac, XIF_PHY_WRDATA_ADDR_REG, addr);

        if(!HAL_STATUS_SUCCESS(status)) break;

        // Issue the GO_WRITE command. Actually, the functionality of the GO_WRITE and GO_READ commands is swapped in
        // the ASIC logic. So, use the GO_WRITE command to read data.
        status = halWriteRegister(pMac, XIF_PHY_CMD_REG, XIF_XIF_PHY_CMD_REG_PHY_GOWRITE_MASK);

        if(!HAL_STATUS_SUCCESS(status)) break;

        //Now, wait for the command to be completed
        do
        {
            status = halReadRegister(pMac, XIF_PHY_CMD_REG, &tmp);

            if(!HAL_STATUS_SUCCESS(status)) break;
        }
        while(0 != tmp);

        if(!HAL_STATUS_SUCCESS(status)) break;

        // Now, grab the read data
        status = halReadRegister(pMac, XIF_PHY_RDDATA_REG, pData);
    }
    while(0);

    return status;
}

eHalStatus asicXIFWritePhyReg(tHalHandle hMac, tANI_U32 addr, tANI_U32 data)
{
    tpAniSirGlobal pMac = PMAC_STRUCT(hMac);
    eHalStatus status = eHAL_STATUS_FAILURE;
    tANI_U32 tmp;

    do
    {
        // Set the address and write data
        status = halWriteRegister(
                                 pMac, 
                                 XIF_PHY_WRDATA_ADDR_REG, 
                                 (addr & XIF_XIF_PHY_WRDATA_ADDR_REG_WR_RD_ADDRESS_MASK) |
                                 ((data << XIF_XIF_PHY_WRDATA_ADDR_REG_WRITE_DATA_OFFSET) & 
                                  XIF_XIF_PHY_WRDATA_ADDR_REG_WRITE_DATA_MASK)
                                 );

        if(!HAL_STATUS_SUCCESS(status)) break;

        // Issue the GO_READ command. Actually, the functionality of the GO_WRITE and GO_READ commands is swapped in
        // the ASIC logic. So, use the GO_READ command to write data.
        status = halWriteRegister(pMac, XIF_PHY_CMD_REG, XIF_XIF_PHY_CMD_REG_PHY_GOREAD_MASK);

        if(!HAL_STATUS_SUCCESS(status)) break;

        //Now, wait for the command to be completed
        do
        {
            status = halReadRegister(pMac, XIF_PHY_CMD_REG, &tmp);

            if(!HAL_STATUS_SUCCESS(status)) break;
        }
        while(0 != tmp);

    }
    while(0);

    return status;
}


/** ------------------------------------------------------------------------- * 
    ------------------------------------------------------------------------- *  

  
    \file halHddApi.h
  
    \brief Exports and types for the Hardware Abstraction Layer interfaces.
  
    $Id$ 
  
    Copyright (C) 2006 Airgo Networks, Incorporated
  
    This file contains all the interfaces for thge Hardware Abstration Layer
    functions.  It is intended to be included in all modules that are using 
    the external HAL interfaces.
  
   ========================================================================== */
#ifndef HALHDD_H__
#define HALHDD_H__

// halApi needs to know about the HDD/PAL handle. 
// is there another way to do this to prevent the nested includes ?
#include "halTypes.h"
#include "halPhy.h"

#include "halTxRx.h"

#include "halTx.h"
#include "halRx.h"
#include "halInterrupts.h"

#include "wlan_nv.h"


#include "palApi.h"
#include "wniCfgAp.h"
#include "sirMacProtDef.h"

/** ------------------------------------------------------------------------- * 

    \typedef tDriverType
    
    \brief   Indicate the driver type to the mac, and based on this do
             appropriate initialization.
    
    -------------------------------------------------------------------------- */

typedef enum
{
    eDRIVER_TYPE_PRODUCTION  = 0,
    eDRIVER_TYPE_MFG         = 1,
    eDRIVER_TYPE_DVT         = 2
} tDriverType;

/** ------------------------------------------------------------------------- * 

    \typedef tMacOpenParameters
    
    \brief Parameters needed for Enumeration of all status codes returned by the higher level 
    interface functions.
    
    -------------------------------------------------------------------------- */

typedef struct sMacOpenParameters
{
    tANI_U16 maxStation;
    tANI_U16 maxBssId;
    tANI_U32 frameTransRequired;
    tDriverType  driverType;
} tMacOpenParameters;



/** ------------------------------------------------------------------------- * 

    \typedef tHalFirmwareParameters 
    
    \brief Parameters for the Firmware image that needs to be passed
    into the HAL.
    
    The buffer for this image is managed (allocated and freed) by the caller.
    
    -------------------------------------------------------------------------- */

typedef struct sHalFirmwareParameters
{
    // size of the Firmware image
    unsigned long cbImage;

    // pointer to the Firmware image.
    void *pImage;
    

} tHalFirmwareParameters;


/** ------------------------------------------------------------------------- * 

    \typedef tHalMacStartParameters 
    
    \brief Parameters for the HAL to start the MAC.
    
    -------------------------------------------------------------------------- */

typedef struct sHalMacStartParameters
{
    // parametes for the Firmware
    tHalFirmwareParameters FW;    
    tDriverType  driverType;

} tHalMacStartParameters;

typedef enum
{
    HAL_STOP_TYPE_SYS_RESET,
    HAL_STOP_TYPE_SYS_DEEP_SLEEP,
    HAL_STOP_TYPE_RF_KILL   
}tHalStopType;

typedef enum
{
    HAL_INT_RADIO_ON,
    HAL_INT_RADIO_OFF
}tHalIntRadioOnOff;

/** ------------------------------------------------------------------------- * 
    ------------------------------------------------------------------------- *  

    \func halHandle2HddHandle

    \brief - Returns the HDD Handle for a given HAL instance

    \param tHalHandle - handle to an open HAL instance.
    
    \return tHddHandle - handle to the associated HDD instance
   
    -------------------------------------------------------------------------- */
tHddHandle halHandle2HddHandle( tHalHandle hHal );


/** ------------------------------------------------------------------------- * 
    ------------------------------------------------------------------------- *  

    \func halReadRegister

    \brief - Read a register value from the hardware.  

    \param tHalHandle - handle to an open HAL instance.
    
    \param regOffset - Offset in register space...
    
    \param unsigned long *pRegValue -
    
    \return eHalStatus
        eHAL_STATUS_SUCCESS - 
        
        eHAL_FAILURE - general failure not covered by the following failing
        return codes.
        
        eHAL_STATUS_NOT_OPEN - HAL has not been opened.
    
        eHAL_STATUS_CARD_NOT_PRESENT - the radio card has been removed.
        
    Note:  This function CAN fail, in particular if the card is removed.
   
    -------------------------------------------------------------------------- */
eHalStatus halReadRegister( tHalHandle hHal, tANI_U32 regOffset, tANI_U32 *pRegValue );

eHalStatus halNormalReadRegister( tHalHandle hHal, tANI_U32 regAddr, tANI_U32* pRegValue);

/** ------------------------------------------------------------------------- * 
    ------------------------------------------------------------------------- *  

    \func halWriteRegister

    \brief - Write a value to a register value on the hardware.  

    \param tHalHandle - handle to an open HAL instance.
    
    \param regOffset - Offset in register space...
    
    \param unsigned long regValue - the value to be written to the register.
    
    \return eHalStatus
        eHAL_STATUS_SUCCESS -
        
        eHAL_FAILURE - general failure not covered by the following failing
        return codes.
        
        eHAL_STATUS_NOT_OPEN - HAL has not been opened.
    
        eHAL_STATUS_CARD_NOT_PRESENT - the radio card has been removed.
        
    Note:  This function CAN fail, in particular if the card is removed.
   
    -------------------------------------------------------------------------- */
eHalStatus halWriteRegister( tHalHandle hHal, tANI_U32 regOffset, tANI_U32 regValue );

eHalStatus halNormalWriteRegister( tHalHandle hHal, tANI_U32 regAddr, tANI_U32 regValue);

/** ------------------------------------------------------------------------- * 
    ------------------------------------------------------------------------- *  

    \func halReadMemory

    \brief - Read data froma memory location on the hardware.  

    \param tHalHandle - handle to an open HAL instance.
    
    \param srcOffset - Offset in memory space on the device where the
    data is being read from.
    
    \param pBuffer - pointer to the buffer in host memory where the data 
    read from the hardware will be placed.
    
    \param numBytes - count of bytes in the buffer.
    
    \return eHalStatus
        eHAL_STATUS_SUCCESS - 
        
        eHAL_FAILURE - general failure not covered by the following failing
        return codes.
        
        eHAL_STATUS_NOT_OPEN - HAL has not been opened.
    
        eHAL_STATUS_CARD_NOT_PRESENT - the radio card has been removed.
        
    Note:  This function CAN fail, in particular if the card is removed.
   
    -------------------------------------------------------------------------- */
eHalStatus halReadDeviceMemory( tHalHandle hHal, tANI_U32 srcOffset, void *pBuffer, tANI_U32 numBytes );
eHalStatus halNormalReadMemory( tHalHandle hHal, tANI_U32 srcOffset, void *pBuffer, tANI_U32 numBytes );



/** ------------------------------------------------------------------------- * 
    ------------------------------------------------------------------------- *  

    \func halWriteMemory

    \brief - Write data to a memory location on the hardware.  

    \param tHalHandle - handle to an open HAL instance.
    
    \param dstOffset - Destination offset in memory space on the device where
    memory is being written.
    
    \param pBupSrcBufferffer - pointer to the buffer in host memory where the data is  
    written from.
    
    \param numBytes - count of bytes in the buffer.
    
    \return eHalStatus
        eHAL_STATUS_SUCCESS - 
        
        eHAL_FAILURE - general failure not covered by the following failing
        return codes.
        
        eHAL_STATUS_NOT_OPEN - HAL has not been opened.
    
        eHAL_STATUS_CARD_NOT_PRESENT - the radio card has been removed.
        
    Note:  This function CAN fail, in particular if the card is removed.
   
    -------------------------------------------------------------------------- */
eHalStatus halWriteDeviceMemory( tHalHandle hHal, tANI_U32 dstOffset, void *pSrcBuffer, tANI_U32 numBytes );  
eHalStatus halNormalWriteMemory( tHalHandle hHal, tANI_U32 dstOffset, void *pSrcBuffer, tANI_U32 numBytes );

/** ------------------------------------------------------------------------- * 
    ------------------------------------------------------------------------- *  

    \func halFillDeviceMemory

    \brief - Fill the hardware memory will a constant value.

    \param tHalHandle - handle to an open HAL instance.
    
    \param memOffset - Offset in memory space on the device to clear clear.
        
    \param numBytes - number of bytes in the memory space to clear.

    \param fillValue - value to be filled.
    
    \return eHalStatus
        eHAL_STATUS_SUCCESS - 
        
        eHAL_FAILURE - general failure not covered by the following failing
        return codes.
        
        eHAL_STATUS_NOT_OPEN - HAL has not been opened.
    
        eHAL_STATUS_CARD_NOT_PRESENT - the radio card has been removed.
        
    Note:  This function CAN fail, in particular if the card is removed.
   
    -------------------------------------------------------------------------- */
eHalStatus halFillDeviceMemory( tHalHandle hHal, tANI_U32 memOffset, tANI_U32 numBytes, tANI_BYTE fillValue );

/** ------------------------------------------------------------------------- * 
    ------------------------------------------------------------------------- *  

    \func halZeroMemory

    \brief - Zero out (clear) memory on the hardware.  

    \param tHalHandle - handle to an open HAL instance.
    
    \param memOffset - Offset in memory space on the device to clear clear.
        
    \param numBytes - number of bytes in the memory space to clear.
    
    \return eHalStatus
        eHAL_STATUS_SUCCESS - 
        
        eHAL_FAILURE - general failure not covered by the following failing
        return codes.
        
        eHAL_STATUS_NOT_OPEN - HAL has not been opened.
    
        eHAL_STATUS_CARD_NOT_PRESENT - the radio card has been removed.
        
    Note:  This function CAN fail, in particular if the card is removed.
   
    -------------------------------------------------------------------------- */
eHalStatus halZeroDeviceMemory( tHalHandle hHal, tANI_U32 memOffset, tANI_U32 numBytes );

#ifdef ANI_SNIFFER
/** ------------------------------------------------------------------------- * 
    ------------------------------------------------------------------------- *  

    \func halSetPromiscousMode

    \brief - Sets the station to the promiscuous mode.  

    \param tHalHandle - handle to an open HAL instance.
    
    \return eHalStatus
        eHAL_STATUS_SUCCESS - if mode is set succesfully.
        
        eHAL_STATUS_FAILURE - if mode cannot be set.
        
    -------------------------------------------------------------------------- */
eHalStatus halSetPromiscuousMode(tHalHandle hHal, tANI_BOOLEAN fMinimalCfg);
eHalStatus halResetPromiscuousMode( tHalHandle hHal);
#endif

/** ------------------------------------------------------------------------- * 
    ------------------------------------------------------------------------- *  

    \func haMsg_

    \brief - Return the STAID for an STA entry given its MAC address.  

    \param tHalHandle - handle to an open HAL instance.
    
    \param staAddr - Specifies the MAC address corresponding to the STA entry.
 
	\param id - Pointer to a variable where the STAID is returned.
 
    \return eHalStatus
        eHAL_STATUS_SUCCESS - returned when the STA entry is found.
        
        eHAL_STATUS_INVALID_STAIDX - returned when the STA entry is not found in the STA table.
        
    -------------------------------------------------------------------------- */

eHalStatus halTable_FindStaidByAddr(tHalHandle hHalHandle, tSirMacAddr staAddr, tANI_U8 *id);


/** ------------------------------------------------------------------------- * 
    ------------------------------------------------------------------------- *  

    \func halTable_IsStaAuthenticated

    \brief - Returns a BOOLEAN value indicating whether a particular STA is in the authenticated
			 state or not.  

    \param tHalHandle - handle to an open HAL instance.
    
    \param staIdx - index to the STA entry.
 
    \return tANI_BOOLEAN
        1 - returned when the STA entry shows that the STA has already been authenticated.
        
        0 - returned when the STA entry is either not found or if it indicates that the STA
			has not been authenticated yet.
        
    -------------------------------------------------------------------------- */
tANI_BOOLEAN halTable_IsStaAuthenticated(tHalHandle hHalHandle, tANI_U8 staIdx);


tSirRetStatus halReadWscMiscCfg(tHalHandle hMac,
                                tANI_U32 *wscApConfigMethod,
                                tANI_U8 *manufacturer,
                                tANI_U8 *modelName,
                                tANI_U8 *modelNumber,
                                tANI_U8 *serialNumber,
                                tANI_U8 *devicename);
#endif



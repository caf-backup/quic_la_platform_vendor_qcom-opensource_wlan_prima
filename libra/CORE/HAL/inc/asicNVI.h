/** ------------------------------------------------------------------------- * 
    ------------------------------------------------------------------------- *  

  
    \file asicNVI.h
  
    \brief Interfaces for NVI module access
  
    $Id$ 
  
    Copyright (C) 2006 Airgo Networks, Incorporated
  
  
   ========================================================================== */

#ifndef ASICNVI_H
#define ASICNVI_H

#include "halTypes.h"

// chip select for EEPROM
#define EEPROM_CHIP_SELECT      3

#define EEPROM_BASE_ADDRESS     0x0DFF0000


//for 'custom_write' register
#define NVI_CUSTOM_COMMAND_VALUE_WRITE   0x02
#define NVI_CUSTOM_COMMAND_VALUE_READ    0x03

#define NVI_CUSTOM_WRITE_CMD_OFFSET    24
#define NVI_CUSTOM_WRITE_ADDR_OFFSET    8
#define NVI_CUSTOM_WRITE_DATA_OFFSET    0

//for 'custom_contorl' register
#define NVI_CUSTOM_CONTROL_CHIP_SELECT_EEPROM       0x8     //cs_3
#define NVI_CUSTOM_CONTROL_EEPROM_FREQUENCY_5MHZ    20//0x3c    USE THIS FOR FPGA


#define NVI_MAX_DWORD_SIZE  (16 * 1024)   //64KB

typedef enum
{
    EEPROM_STORED_LITTLE_ENDIAN,
    EEPROM_STORED_BIG_ENDIAN
}eEepromEndianess;


eHalStatus asicNVITestEndianness(tHalHandle hMac, eEepromEndianess *endianness);
eHalStatus asicNVIBlankEeprom(tHalHandle hMac, tANI_U16 eepromSize);
eHalStatus asicNVIWriteData(tHalHandle hMac, tANI_U32 addr, tANI_U8 *pBuf, tANI_U32 nBytes);
eHalStatus asicNVIReadData(tHalHandle hMac, tANI_U32 addr, tANI_U8 *pBuf, tANI_U32 nBytes);
eHalStatus asicNVIBurstConfig(tHalHandle hMac, tANI_U8 nviMHz);
eHalStatus asicNVIWriteBurstData(tHalHandle hMac, tANI_U32 addr, tANI_U32 *pBuf, tANI_U32 nDwords);
eHalStatus asicNVIReadBurstData(tHalHandle hMac, tANI_U32 addr, tANI_U32 *pBuf, tANI_U32 nDwords);
eHalStatus asicNVISetBurstPageSize(tHalHandle hMac, tANI_U32 cs, tANI_U32 nPageSize);

#endif


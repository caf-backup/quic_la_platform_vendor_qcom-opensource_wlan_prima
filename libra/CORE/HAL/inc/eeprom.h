/** ------------------------------------------------------------------------- *
    ------------------------------------------------------------------------- *


    \file eeprom.h

    \brief Types for EEPROM implementation
            Anything that needs to be publicly available should be in halEeprom.h or halEepromTables.h

    $Id$

    Copyright (C) 2006 Airgo Networks, Incorporated


   ========================================================================== */

#ifndef EEPROM_H
#define EEPROM_H

#include "halEeprom.h"


/// EEPROM Definitions
/// Specifies the size of the currently used EEPROM devices
#define HAL_EEPROM_SIZE_1K		0x0400
#define HAL_EEPROM_SIZE_8K      0x2000
#define HAL_EEPROM_SIZE_16K     0x4000
#define HAL_EEPROM_SIZE_32K     0x8000
#define HAL_EEPROM_SIZE         HAL_EEPROM_SIZE_1K


#define HAL_EEPROM_EMPTY_PATTERN  0xFF

/* It is important to note that these structures are stored in little endian format.
   All the host interfaces access this information in little endian format because
   that is the mode that the chip starts up with. 
   It is only after this has happened, that our driver would change the endian setting.
   
   So when these host interface structures are accessed in cache, wherever the fields are declared as 16 or 32 bits,
   they will be in the endianess as build for the platform. 
   In some cases, we have 16 or 32 bit fields that overlap the word boundaries, and we need to 
   declare these as byte arrays and fill them in little endian order.
   When they are burst accessed from EEPROM, we will always transfer them as little endian, 
   and then set the platform endianness back as appropriate.
*/


/* Tables in EEPROM are handled through a directory space.
    They can be added/replaced or removed, but at present the implementation requires
    the size to not change once it is written.

    When a table is removed, the contents still remain, but we set the number of entries to 0.

    The table directory and the start of tables should always be 32-bit aligned in the
    EEPROM space for burst accesses.
*/

//use these to check the EEPROM structure at init.
#define EEPROM_TABLE_DIR_START_OFFSET 608     //sEepromTableDir tableDirEntry[MAX_EEPROM_TABLE]; should start here


extern const sTxIQChannel        txIQTable[NUM_DEMO_CAL_CHANNELS];
extern const sRxIQChannel        rxIQTable[NUM_DEMO_CAL_CHANNELS];
extern const sLnaSwGainTable     lnaSwGainTable;
extern const sTxLoCorrectChannel txLoCorrectTable[NUM_DEMO_CAL_CHANNELS];
extern const sQuasarChannelRegs  quasarRegsTable[NUM_RF_CHANNELS];
extern const sRegulatoryDomains  regDomains[NUM_REG_DOMAINS];
extern const tTpcParams          pwrBgParams;
extern const tTpcConfig          pwrBgBand[MAX_TPC_CHANNELS];
extern const tRateGroupPwr       pwrOptimum[NUM_RF_SUBBANDS];
extern const tRxDcoCorrect       rxDco[PHY_MAX_RX_CHAINS];
extern const sInitCalValues      initCalValueTable;



#endif
	

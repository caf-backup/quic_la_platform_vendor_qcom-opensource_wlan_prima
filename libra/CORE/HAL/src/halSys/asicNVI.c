/** ------------------------------------------------------------------------- *
    ------------------------------------------------------------------------- *


    \file asicNVI.c

    \brief NVI primitive functions

    $Id$

    Copyright (C) 2006 Airgo Networks, Incorporated


   ========================================================================== */

#include "ani_assert.h"
#include "halDebug.h"
#include "halInternal.h"

#define NVI_CHIP_SEL_EEPROM        0x80
#define NVI_BUSY_WAIT_TIME         10000000   //100us


static eHalStatus customSpiWriteOneByte(tpAniSirGlobal pMac, tANI_U32 addr, tANI_U8 byteData);
static eHalStatus customSpiReadOneByte(tpAniSirGlobal pMac, tANI_U32 addr, tANI_U8 *pByteData);

//static eHalStatus asicNVIReadStatus(tHalHandle hMac, tANI_U8 *pStatus);




/**
    \fn customSpiWriteOneByte

    \brief Provides basic SPI implementation to write a single byte to EEPROM

    \param  addr is the raw EEPROM address without respect to any Taurus memory map or endianness
    \param  byteData is a single byte of data to write to the address

    \return eHAL_STATUS_SUCCESS or eHAL_STATUS_FAILURE

*/
static eHalStatus customSpiWriteOneByte(tpAniSirGlobal pMac, tANI_U32 addr, tANI_U8 byteData)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
    tANI_U32 csXControl;
    tANI_U32 customWrite = 0;
    tANI_U32 customControl;
    

#if defined(ANI_BIG_BYTE_ENDIAN)
    //need to byte swap on 32-bit boundaries so that it matches the burst.
    switch (addr % 4)
    {
        case 0: addr += 3;  break;
        case 1: addr += 1;  break;
        case 2: addr -= 1;  break;
        case 3: addr -= 3;  break;
        default: assert(0); return eHAL_STATUS_FAILURE;
    }
#endif

    //enable write-enable for EEPROM
    if ((retVal = halReadRegister(pMac, QWLAN_NVI_CS_X_WREN_REG, &csXControl)) == eHAL_STATUS_SUCCESS)
    {
        csXControl |= ( (NVI_CUSTOM_CONTROL_CHIP_SELECT_EEPROM << QWLAN_NVI_CS_X_WREN_CS_OFFSET) | (1 << QWLAN_NVI_CS_X_WREN_BUSY_OFFSET) );

        if ((retVal = halWriteRegister(pMac, QWLAN_NVI_CS_X_WREN_REG, csXControl)) == eHAL_STATUS_SUCCESS)
        {
#if 0
            while (csXControl & QWLAN_NVI_CS_X_WREN_BUSY_MASK)
            {
                if ((retVal = halReadRegister(pMac, QWLAN_NVI_CS_X_WREN_REG, &csXControl)) != eHAL_STATUS_SUCCESS)
                {
                    return (retVal);
                }
            }
#endif      
            //The above commented code can be optimally done using the below interface. Clean up the above code
            //when the below interface is tested regressively
            retVal = palWaitRegVal(pMac->hHdd, QWLAN_NVI_CS_X_WREN_REG, QWLAN_NVI_CS_X_WREN_BUSY_MASK, 0,
                                                                                1000, 5000, &csXControl);
            if (retVal != eHAL_STATUS_SUCCESS)
            {
                return retVal;
            }
            sirBusyWait(NVI_BUSY_WAIT_TIME);

            //custom write command/addr/data
            customWrite = (NVI_CUSTOM_COMMAND_VALUE_WRITE << NVI_CUSTOM_WRITE_CMD_OFFSET) |
                          (addr << NVI_CUSTOM_WRITE_ADDR_OFFSET) | (byteData << NVI_CUSTOM_WRITE_DATA_OFFSET);

            if ((retVal = halWriteRegister(pMac, QWLAN_NVI_CUSTOM_WRITE_REG, customWrite)) == eHAL_STATUS_SUCCESS)
            {
                //sirBusyWait(NVI_BUSY_WAIT_TIME);

                if ((retVal = halReadRegister(pMac, QWLAN_NVI_CUSTOM_CONTROL_REG, &customControl)) == eHAL_STATUS_SUCCESS)
                {
                    customControl &= ~(QWLAN_NVI_CUSTOM_CONTROL_WRLEN_MASK |
                                       QWLAN_NVI_CUSTOM_CONTROL_RDLEN_MASK |
                                       QWLAN_NVI_CUSTOM_CONTROL_FREQUENCY_MASK |
                                       QWLAN_NVI_CUSTOM_CONTROL_CS_MASK |
                                       QWLAN_NVI_CUSTOM_CONTROL_EEPROM_MASK
                                      );

                    customControl |= ((1 << QWLAN_NVI_CUSTOM_CONTROL_EEPROM_OFFSET) |
                                      (NVI_CUSTOM_CONTROL_CHIP_SELECT_EEPROM << QWLAN_NVI_CUSTOM_CONTROL_CS_OFFSET) |
                                      (0x20 << QWLAN_NVI_CUSTOM_CONTROL_WRLEN_OFFSET) |  //32 bits from custom_write
                                      //(8 << NVI_CUSTOM_CONTROL_RDLEN_OFFSET) |
                                      (NVI_CUSTOM_CONTROL_EEPROM_FREQUENCY_5MHZ << QWLAN_NVI_CUSTOM_CONTROL_FREQUENCY_OFFSET) |
                                      (1 << QWLAN_NVI_CUSTOM_CONTROL_BUSY_OFFSET)
                                     );

                    if ((retVal = halWriteRegister(pMac, QWLAN_NVI_CUSTOM_CONTROL_REG, customControl)) == eHAL_STATUS_SUCCESS)
                    {
                        //check completion
                        if ((retVal = halReadRegister(pMac, QWLAN_NVI_CUSTOM_CONTROL_REG, &customControl)) == eHAL_STATUS_SUCCESS)
                        {
#if 0
                            //loop until busy mask clears
                            while (customControl & NVI_CUSTOM_CONTROL_BUSY_MASK)
                            {
                                if ((retVal = halReadRegister(pMac, NVI_CUSTOM_CONTROL_REG, &customControl)) != eHAL_STATUS_SUCCESS)
                                {
                                    return (retVal);
                                }
                            }
#endif                      
                            //The above commented code can be optimally done using the below interface. Clean up the above code
                            //when the below interface is tested regressively
                            retVal = palWaitRegVal(pMac->hHdd, QWLAN_NVI_CUSTOM_CONTROL_REG, QWLAN_NVI_CUSTOM_CONTROL_BUSY_MASK, 0,
                                                                                                1000, 5000, &customControl);
                            if (retVal != eHAL_STATUS_SUCCESS)
                            {
                                return retVal;
                            }
                            sirBusyWait(NVI_BUSY_WAIT_TIME);
                        }
                    }
                }
            }
        }
    }

    return (retVal);
}


/**
    \fn customSpiReadOneByte

    \brief Provides basic SPI implementation to read a single byte from EEPROM

    \param  addr is the raw EEPROM address without respect to any Taurus memory map or endianness
    \param  pByteData is the location of a single byte, where the data will be placed

    \return eHAL_STATUS_SUCCESS or eHAL_STATUS_FAILURE

*/
static eHalStatus customSpiReadOneByte(tpAniSirGlobal pMac, tANI_U32 addr, tANI_U8 *pByteData)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
    tANI_U32 regVal;
    tANI_U32 customWrite = 0;
    tANI_U32 customControl;

#if defined(ANI_BIG_BYTE_ENDIAN)
    //need to byte swap on 32-bit boundaries so that it matches the burst access.
    switch (addr % 4)
    {
        case 0: addr += 3;  break;
        case 1: addr += 1;  break;
        case 2: addr -= 1;  break;
        case 3: addr -= 3;  break;
        default: assert(0); return eHAL_STATUS_FAILURE;
    }
#endif
    
    //custom write command/addr/data
    customWrite = (NVI_CUSTOM_COMMAND_VALUE_READ << NVI_CUSTOM_WRITE_CMD_OFFSET) |
                  (addr << NVI_CUSTOM_WRITE_ADDR_OFFSET);

    if ((retVal = halWriteRegister(pMac, QWLAN_NVI_CUSTOM_WRITE_REG, customWrite)) == eHAL_STATUS_SUCCESS)
    {
        //sirBusyWait(NVI_BUSY_WAIT_TIME);

        if ((retVal = halReadRegister(pMac, QWLAN_NVI_CUSTOM_CONTROL_REG, &customControl)) == eHAL_STATUS_SUCCESS)
        {
            //Clear
            customControl &= ~(QWLAN_NVI_CUSTOM_CONTROL_WRLEN_MASK |
                               QWLAN_NVI_CUSTOM_CONTROL_RDLEN_MASK |
                               QWLAN_NVI_CUSTOM_CONTROL_FREQUENCY_MASK |
                               QWLAN_NVI_CUSTOM_CONTROL_CS_MASK |
                               QWLAN_NVI_CUSTOM_CONTROL_EEPROM_MASK
                              );

            customControl |= ((1 << QWLAN_NVI_CUSTOM_CONTROL_EEPROM_OFFSET) |
                              (NVI_CUSTOM_CONTROL_CHIP_SELECT_EEPROM << QWLAN_NVI_CUSTOM_CONTROL_CS_OFFSET) |
                              (0x18 << QWLAN_NVI_CUSTOM_CONTROL_WRLEN_OFFSET) | //For EEProm, write top 24 bits from custom_write
                              (8 << QWLAN_NVI_CUSTOM_CONTROL_RDLEN_OFFSET) |
                              (NVI_CUSTOM_CONTROL_EEPROM_FREQUENCY_5MHZ << QWLAN_NVI_CUSTOM_CONTROL_FREQUENCY_OFFSET) |
                              (1 << QWLAN_NVI_CUSTOM_CONTROL_BUSY_OFFSET)
                             );


            if ((retVal = halWriteRegister(pMac, QWLAN_NVI_CUSTOM_CONTROL_REG, customControl)) == eHAL_STATUS_SUCCESS)
            {
                //check completion
                if ((retVal = halReadRegister(pMac, QWLAN_NVI_CUSTOM_CONTROL_REG, &customControl)) == eHAL_STATUS_SUCCESS)
                {

#if 0                
                    while (customControl & NVI_CUSTOM_CONTROL_BUSY_MASK)
                    {
                        if ((retVal = halReadRegister(pMac, NVI_CUSTOM_CONTROL_REG, &customControl)) != eHAL_STATUS_SUCCESS)
                        {
                            return (retVal);
                        }
                    }
#endif
                    //The above commented code can be optimally done using the below interface. Clean up the above code
                    //when the below interface is tested regressively
                    retVal = palWaitRegVal(pMac->hHdd, QWLAN_NVI_CUSTOM_CONTROL_REG, QWLAN_NVI_CUSTOM_CONTROL_BUSY_MASK, 0,
                                                                                                1000, 5000, &customControl);
                    if (retVal != eHAL_STATUS_SUCCESS)
                    {
                        return retVal;
                    }
                    sirBusyWait(NVI_BUSY_WAIT_TIME);

                    //read data
                    if ((retVal = halReadRegister(pMac, QWLAN_NVI_CUSTOM_READ_REG, &regVal)) == eHAL_STATUS_SUCCESS)
                    {
                        *pByteData = (tANI_U8)((regVal >> 24) & 0xff);
                    }
                }
            }
        }
    }

    return (retVal);
}



#define TEST_BYTE_0 0x12
#define TEST_BYTE_1 0x34
#define TEST_BYTE_2 0x56
#define TEST_BYTE_3 0x78
#define TEST_WORD   0x87654321L
/**
    \fn asicNVITestEndianness

    \brief Simple test on a DWORD to see if we are appropriately configured for platform endianness

    \param endianness - OUTPUT the endianness detected. -

    \return eHAL_STATUS_SUCCESS(endianness is correct or LITTLE) or eHAL_STATUS_FAILURE(endianness is not correct or BIG)

*/
eHalStatus asicNVITestEndianness(tHalHandle hMac, eEepromEndianess *endianness)
{
    tpAniSirGlobal pMac = PMAC_STRUCT(hMac);
    eHalStatus retVal;
    tANI_U32 testAddr;
    tANI_U8 testData[4] =
    {
        TEST_BYTE_0,
        TEST_BYTE_1,
        TEST_BYTE_2,
        TEST_BYTE_3
    };
    tANI_U32 testWord;

    //use the last DWORD in reserved field space, just before cksumPreceding
    //it is doubtful that this will ever get used for something else
    testAddr = (offsetof(sHalEeprom, fields.cksumPreceding) - 4);

    //Write the testData by the SPI bus and read back by the AHB bus
    if ((retVal = asicNVIWriteData(hMac, testAddr, testData, 4)) == eHAL_STATUS_SUCCESS)
    {
        //test data written
        if ((retVal = asicNVIReadBurstData(hMac, testAddr, &testWord, 1)) == eHAL_STATUS_SUCCESS)
        {
            if (((tANI_U8)testWord != TEST_BYTE_0) ||
                ((tANI_U8)(testWord >> 8) != TEST_BYTE_1) ||
                ((tANI_U8)(testWord >> 16) != TEST_BYTE_2) ||
                ((tANI_U8)(testWord >> 24) != TEST_BYTE_3)
               )
            {
                //SPI access does not match burst access - endianess is misaligned
                halLog(pMac, LOGE, "\nERROR: NVI Spi access does not match burst access\n");
                halLog(pMac, LOGE, "Spi write byte0=%X, byte1=%X, byte2=%X, byte3=%X to %8X\n",
                                    TEST_BYTE_0,
                                    TEST_BYTE_1,
                                    TEST_BYTE_2,
                                    TEST_BYTE_3,
                                    testAddr
                      );
                halLog(pMac, LOGE, "Burst read =%8X from %8X\n",
                                    testWord,
                                    testAddr
                      );
            }
            else
            {
                halLog(pMac, LOG2, "\nNVI Spi access matches burst access - endianess matches\n");
            }
        }
    }

    return (retVal);
}

#define NUM_BLANK_WORDS     128


/* asicNVIBlankEeprom is almost a duplicate of asicNVIWriteBurstData, but it only writes 0xFF to EEPROM.
    Doing it this way eliminates the need to allocate an EEPROM sized piece of memory to perform the blanking.
*/
eHalStatus asicNVIBlankEeprom(tHalHandle hMac, tANI_U16 eepromSize)
{
    tpAniSirGlobal pMac = PMAC_STRUCT(hMac);
    eHalStatus retVal = eHAL_STATUS_SUCCESS;

    //burst-mode should be pre-configured
    {
        //write blank pattern to EEPROM
        tANI_U32 dwAddr;
        tANI_U32 blankPattern[NUM_BLANK_WORDS] =
        {
            0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
            0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
            0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
            0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
            0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
            0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
            0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
            0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
            0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
            0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
            0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
            0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
            0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
            0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
            0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
            0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF
        };

        for (dwAddr = 0; dwAddr < eepromSize; dwAddr += NUM_BLANK_WORDS << 2)
        {
#ifdef FIXME_GEN6        
            if ((retVal = palWriteEepromMemory(pMac->hHdd, dwAddr,
                                               (tANI_U8 *)blankPattern, NUM_BLANK_WORDS << 2)
                                              )
                != eHAL_STATUS_SUCCESS
               )
#endif                
            {
                return(retVal);
            }
        }
    }

    return (retVal);
}






#if defined(ANI_MANF_DIAG) && (WNI_POLARIS_FW_OS == SIR_RTAI)
void    pal_rt_reset_watchdog(void);
#endif


/**
    \fn asicNVIWriteData

    \brief Provides write access to EEPROM via the SPI bus directly, rather than AHB bus.
            It is necessary that these writes are aligned as they would be from AHB, so endianess must be taken into account.
            We will byte swap all data based on 32-bit boundaries.
            The addr does not need to be 32-bit word aligned.

    \param  addr is the EEPROM address without respect to any Taurus memory map.
    \param  pBuf holds the nBytes of data to write to the addr parameter

    \return eHAL_STATUS_SUCCESS or eHAL_STATUS_FAILURE
*/
eHalStatus asicNVIWriteData(tHalHandle hMac, tANI_U32 addr, tANI_U8 *pBuf, tANI_U32 nBytes)
{
    tpAniSirGlobal pMac = PMAC_STRUCT(hMac);
    eHalStatus retVal;
    tANI_U32 i, regVal;

    if(((retVal = halReadRegister(pMac, QWLAN_NVI_GENERAL_REG, &regVal)) == eHAL_STATUS_SUCCESS) &&
       ((retVal = halWriteRegister(pMac, QWLAN_NVI_GENERAL_REG, (regVal | QWLAN_NVI_GENERAL_ARB_SEL_MASK))) == eHAL_STATUS_SUCCESS)
      )
    {
        //NVI set for SPI access
        for (i=0; i < nBytes; i++)
        {
#if defined(ANI_MANF_DIAG) && (WNI_POLARIS_FW_OS == SIR_RTAI)
            //kick the watchdog timer during this long operation
            if(i%500 == 0)
                pal_rt_reset_watchdog();
#endif
            if ((retVal = customSpiWriteOneByte(pMac, addr + i, pBuf[i])) != eHAL_STATUS_SUCCESS)
            {
                break;
            }
        }
    }

    return (retVal);
}




eHalStatus asicNVIReadData(tHalHandle hMac, tANI_U32 addr, tANI_U8 *pBuf, tANI_U32 nBytes)
{
    tpAniSirGlobal pMac = PMAC_STRUCT(hMac);
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
    tANI_U32 i;
    tANI_U32 regVal;

    if (((retVal = halReadRegister(pMac, QWLAN_NVI_GENERAL_REG, &regVal)) == eHAL_STATUS_SUCCESS) &&
        ((retVal = halWriteRegister(pMac, QWLAN_NVI_GENERAL_REG, (regVal | QWLAN_NVI_GENERAL_ARB_SEL_MASK))) == eHAL_STATUS_SUCCESS)
       )
    {

        for (i = 0; i < nBytes; i++)
        {
            if ((retVal = customSpiReadOneByte(pMac, addr+i, &pBuf[i])) != eHAL_STATUS_SUCCESS)
            {
                break;
            }
        }
    }

    return (retVal);
}

#define TARGET_SPI_CLK_MHZ      2               //always <=5
#define TARGET_WR_BURST_DELAY   5               //milliseconds


//example: mult is 1000000 for 6 decimal places of precision
#define ROUNDUP(num, den, mult)  ( (((num * mult) / den) % mult) > 0  ?                                              \
                                   ( ((num * mult) / den) + (mult - (((num * mult) / den) % mult)) ) / mult :        \
                                   (num / den)                                                                       \
                                 )

#if ROUNDUP(25, 10, 1000000) != 3
#error ROUNDUP wrong
#endif

eHalStatus asicNVIBurstConfig(tHalHandle hMac, tANI_U8 nviMHz /* see CS_3_CONFIG */)
{
    tpAniSirGlobal pMac = PMAC_STRUCT(hMac);
    eHalStatus retVal;
    tANI_U32 regVal;
    tANI_U32 freq;       //8 bits
    tANI_U32 delay;     //10 bits
    assert(nviMHz <= 240);

    //use nviMHz to calculate frequency and delay settings
    freq = ROUNDUP(nviMHz, (TARGET_SPI_CLK_MHZ * 2), 1000000) + 1;
    delay = ROUNDUP(TARGET_WR_BURST_DELAY * (nviMHz * 1000), 1024, 100);

    //enable burst-mode
    if (((retVal = halReadRegister(pMac, QWLAN_NVI_GENERAL_REG, &regVal)) == eHAL_STATUS_SUCCESS) &&
        ((retVal = halWriteRegister(pMac, QWLAN_NVI_GENERAL_REG, (regVal | QWLAN_NVI_GENERAL_ARB_SEL_MASK))) == eHAL_STATUS_SUCCESS)
       )
    {

#if ANI_EEPROM_WRITABLE
        regVal =  (QWLAN_NVI_CS_3_CONFIG_WRITE_EN_MASK | QWLAN_NVI_CS_3_CONFIG_EEPROM_MASK |
                   (delay << QWLAN_NVI_CS_3_CONFIG_DELAY_OFFSET) |
                   (freq << QWLAN_NVI_CS_3_CONFIG_FREQUENCY_OFFSET)
                  );
#else   //don't allow writes to EEPROM - leave write enable off
        regVal =  (QWLAN_NVI_CS_3_CONFIG_EEPROM_MASK |
                   (delay << QWLAN_NVI_CS_3_CONFIG_DELAY_OFFSET) |
                   (freq << QWLAN_NVI_CS_3_CONFIG_FREQUENCY_OFFSET)
                  );
#endif
        if ((retVal = halWriteRegister(pMac, QWLAN_NVI_CS_3_CONFIG_REG, regVal)) == eHAL_STATUS_SUCCESS)
        {
            halLog(pMac, LOGW, "NVI Burst Config: frequ = %d  delay = %d", freq, delay);
        }
    }

    return(retVal);
}

/*
#define BUSY_CHECK_LIMIT    10000

static eHalStatus asicNVIReadStatus(tHalHandle hMac, tANI_U8 *pStatus)
{
    tpAniSirGlobal pMac = PMAC_STRUCT(hMac);
    eHalStatus retVal;
    tANI_U32 readVal;
    tANI_S32 i = BUSY_CHECK_LIMIT;

    retVal = halWriteRegister(pMac, NVI_CS_X_RDSR_REG,  
                                     (NVI_CUSTOM_CONTROL_CHIP_SELECT_EEPROM << NVI_CS_X_RDSR_CS_OFFSET) | //chip select 3 for EEPROM
                                     NVI_CS_X_RDSR_BUSY_MASK
                                 );
    if (retVal == eHAL_STATUS_SUCCESS)
    {
        do
        {
            if ((retVal = halReadRegister(pMac, NVI_CS_X_RDSR_REG, &readVal)) != eHAL_STATUS_SUCCESS)
            {
                return (retVal);
            }
        }while ((readVal & NVI_CS_X_RDSR_BUSY_MASK) && i--);
    }

    *pStatus = (tANI_U8) ((readVal >> NVI_CS_X_RDSR_STATUS_OFFSET) & 0xff);

    return (retVal);

}
*/

#define RETRY_BURST_LIMIT 3

eHalStatus asicNVIWriteBurstData(tHalHandle hMac, tANI_U32 addr, tANI_U32 *pBuf, tANI_U32 nDwords)
{
    tpAniSirGlobal pMac = PMAC_STRUCT(hMac);
    eHalStatus retVal;
    tANI_U32 dwAddr = addr;
    tANI_U32 status;
//     tANI_U8 status;
//     tANI_S32 i = RETRY_BURST_LIMIT;

    //ensure that the address is dword aligned for the burst
    if (addr % 4 != 0)
    {
        return (eHAL_STATUS_FAILURE);
    }
#if 0    
    do
    {
        if ((retVal = halReadRegister(pMac, NVI_GENERAL_REG, &status)) != eHAL_STATUS_SUCCESS) //NVI_GENERAL_REG
        {
            return (retVal);
        }                
    }while (status & NVI_GENERAL_BUSY_MASK);
#endif
    //The above commented code can be optimally done using the below interface. Clean up the above code
    //when the below interface is tested regressively
    retVal = palWaitRegVal(pMac->hHdd, QWLAN_NVI_GENERAL_REG, QWLAN_NVI_GENERAL_BUSY_MASK, 0,
                                                           1000, 5000, &status);
    if (retVal != eHAL_STATUS_SUCCESS)
    {
        return retVal;
    }
    //sirBusyWait(5*1000*1000);   //wait 5 ms before writes

/*
        do
        {
            if ((retVal = asicNVIReadStatus (hMac, &status)) != eHAL_STATUS_SUCCESS)
            {
                return (retVal);
            }
        }while ((status & 1) && i--);
        
        if (!i)
        {
            halLog(pMac, LOGE, "Unable to burst write, because EEPROM read status not complete after %d tries!\n", RETRY_BURST_LIMIT);
            halLog(pMac, LOGE, "EEPROM status = %0xX\n", status);
            return (eHAL_STATUS_FAILURE);
        }
*/
#ifdef FIXME_GEN6

    retVal = palWriteEepromMemory(pMac->hHdd, dwAddr, (tANI_U8 *)pBuf, nDwords << 2);
#endif 
    return (retVal);
}


eHalStatus asicNVIReadBurstData(tHalHandle hMac, tANI_U32 addr, tANI_U32 *pBuf, tANI_U32 nDwords)
{
    tpAniSirGlobal pMac = PMAC_STRUCT(hMac);
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
    tANI_U32 dwAddr = addr;
//     tANI_U8 status;
//     tANI_S32 i = RETRY_BURST_LIMIT;

    if (addr % 4 != 0)
    {
        assert(0);
        return (eHAL_STATUS_FAILURE);
    }

/*
        do
        {
            if ((retVal = asicNVIReadStatus (hMac, &status)) != eHAL_STATUS_SUCCESS)
            {
                return (retVal);
            }
        }while ((status & 1) && i--);
        
        if (!i)
        {
            halLog(pMac, LOGE, "Unable to burst read, because EEPROM read status not complete after %d tries!\n", RETRY_BURST_LIMIT);
            halLog(pMac, LOGE, "EEPROM status = %0xX\n", status);
            return (eHAL_STATUS_FAILURE);
        }
        
*/

    //sirBusyWait(5*1000*1000);   //wait 5 ms before reads

#ifdef FIXME_GEN6
    retVal = palReadEepromMemory( pMac->hHdd, dwAddr, (tANI_U8 *)pBuf, nDwords << 2);
#endif 

    return (retVal);
}


//This function set the burst page size for a particular chip
//
eHalStatus asicNVISetBurstPageSize(tHalHandle hMac, tANI_U32 cs, tANI_U32 nPageSize)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tpAniSirGlobal pMac = PMAC_STRUCT(hMac);
    tANI_U32 nRegAddr = 0;
    tANI_U32 nRegVal = 0;
    tANI_U32 nMask = 0;
    tANI_U32 nSetVal = 0;

    if(nPageSize > 256 || (nPageSize % 4))
    {
        status = eHAL_STATUS_INVALID_PARAMETER;
    }
    else
    {
        nPageSize--; //get the biggest address whitin a page
        nPageSize >>= 2;
        switch (cs)
        {
            case 3:
                nRegAddr = QWLAN_NVI_CS_3_BURST_REG;
                nMask = ~(QWLAN_NVI_CS_3_BURST_BURST_MASK_MASK | QWLAN_NVI_CS_3_BURST_BURST_MASK);
                nSetVal = ((nPageSize << QWLAN_NVI_CS_3_BURST_BURST_MASK_OFFSET) | (nPageSize << QWLAN_NVI_CS_3_BURST_BURST_OFFSET));
                break;
            case 0:
                nRegAddr = QWLAN_NVI_CS_0_BURST_REG;
                nMask = ~(QWLAN_NVI_CS_0_BURST_BURST_MASK_MASK | QWLAN_NVI_CS_0_BURST_BURST_MASK);
                nSetVal = ((nPageSize << QWLAN_NVI_CS_0_BURST_BURST_MASK_OFFSET) | (nPageSize << QWLAN_NVI_CS_0_BURST_BURST_OFFSET));
                break;
            case 1:
                nRegAddr = QWLAN_NVI_CS_1_BURST_REG;
                nMask = ~(QWLAN_NVI_CS_1_BURST_BURST_MASK_MASK | QWLAN_NVI_CS_1_BURST_BURST_MASK);
                nSetVal = ((nPageSize << QWLAN_NVI_CS_1_BURST_BURST_MASK_OFFSET) | (nPageSize << QWLAN_NVI_CS_1_BURST_BURST_OFFSET));
                break;
            case 2:
                nRegAddr = QWLAN_NVI_CS_2_BURST_REG;
                nMask = ~(QWLAN_NVI_CS_2_BURST_BURST_MASK_MASK | QWLAN_NVI_CS_2_BURST_BURST_MASK);
                nSetVal = ((nPageSize << QWLAN_NVI_CS_2_BURST_BURST_MASK_OFFSET) | (nPageSize << QWLAN_NVI_CS_2_BURST_BURST_OFFSET));
                break;
            default:
                status = eHAL_STATUS_INVALID_PARAMETER;
                break;
        }
    }

    if (status == eHAL_STATUS_SUCCESS)
    {
        if ((status = halReadRegister(pMac, nRegAddr, &nRegVal)) == eHAL_STATUS_SUCCESS)
        {
            nRegVal &= nMask;
            status = halWriteRegister(pMac, nRegAddr, nRegVal | nSetVal);
        }
    }

    return status;
}



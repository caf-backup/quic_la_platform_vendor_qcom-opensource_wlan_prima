/*
 * Airgo Networks, Inc proprietary. All rights reserved.
 * This file drvApi.h contains the driver APIs for Polaris and Cygnus.
 * Author:      Mark Nelson
 * Date:        01/29/2005
 * History:-
 * Date         Modified by    Modification Information
 * --------------------------------------------------------------------
 */
#ifndef DRVSPIBUS_H
#define DRVSPIBUS_H

#include "wlan_bit.h"


#ifdef __cplusplus
extern "C"
{
#endif

#ifdef FIXME_GEN5

#define SPI_CONFIG_REG                      RFSPI_CONFIG_REG
#define SPI_CONTROL_REG                     RFSPI_CONTROL_REG
#define SPI_CONTROL_BUSY_MASK               RFSPI_CONTROL_BUSY_MASK
#define SPI_WRITE_LSW_REG                   RFSPI_WRITE_LSW_REG
#define SPI_WRITE_MSW_REG                   RFSPI_WRITE_MSW_REG
#define SPI_READ_LSW_REG                    RFSPI_READ_LSW_REG
#define SPI_READ_MSW_REG                    RFSPI_READ_MSW_REG

#define SPI_CONFIG_LSB_FIRST_MASK           RFSPI_CONFIG_LSB_FIRST_MASK
#define SPI_CONFIG_CLOCK_DIVIDE_MASK        RFSPI_CONFIG_CLOCK_DIVIDE_MASK

#define SPI_CONFIG_CLOCK_DIVIDE_E25_NS      RFSPI_CONFIG_CLOCK_DIVIDE_E25_NS
#define SPI_CONFIG_CLOCK_DIVIDE_E50_NS      RFSPI_CONFIG_CLOCK_DIVIDE_E50_NS
#define SPI_CONFIG_CLOCK_DIVIDE_E100_NS     RFSPI_CONFIG_CLOCK_DIVIDE_E100_NS
#define SPI_CONFIG_CLOCK_DIVIDE_E200_NS     RFSPI_CONFIG_CLOCK_DIVIDE_E200_NS
#endif




//defines for SPI:Control register
#define SPI_WRITE_SIZE_MASK         MSK_5
#define SPI_READ_SIZE_MASK          MSK_5
#define SPI_READ_SIZE_OFFSET        BIT_5
#define SPI_READ_SIZE_SHIFT         5
#define SPI_RF_CHIP_0_CONTROL       BIT_10
#define SPI_RF_CHIP_1_CONTROL       BIT_11
#define SPI_RF_CHIP_2_CONTROL       BIT_12
#define SPI_SYNTH_CHIP_CONTROL      BIT_13
#define SPI_READ_CONTROL            BIT_14  //0=write, 1=read
#define SPI_BUSY_CONTROL            BIT_15  //write 1 to start, poll till 0 to finish
#define SPI_WRITE_START             (SPI_BUSY_CONTROL)
#define SPI_READ_START              (SPI_BUSY_CONTROL | SPI_READ_CONTROL)

#define SPI_QUASAR_WRITE_SIZE       23  //= 24 - 1, because register doc says so
#define SPI_QUASAR_WRITE_ADDR_SIZE  5   //5 bits for address, 1 bit for SPI_RW_CONTROL - 1, because register doc says so
#define SPI_QUASAR_READ_SIZE        17  //= 18 - 1, because register doc says so

//defined pertaining to the data written or read across the SPI bus
#define SPI_DATA_SHIFT              0   //bits 0 to 18
#define SPI_DATA_MASK               MSK_18
#define SPI_REG_ADDR_MASK           MSK_5

#define WR_SPI_RW_CONTROL           BIT_18   //0=Write, 1=Read
#define WR_SPI_REG_ADDR_OFFSET      19

#define RD_SPI_RW_CONTROL           BIT_0   //0=Write, 1=Read
#define RD_SPI_REG_ADDR_OFFSET      1



#ifdef __cplusplus
}
#endif

#endif //DRVSPIBUS_H


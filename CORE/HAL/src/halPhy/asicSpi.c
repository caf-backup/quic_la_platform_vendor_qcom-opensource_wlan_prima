/*
 * Airgo Networks, Inc proprietary. All rights reserved.
 * drvCfg.cc: Provides all the Cygnus driver APIs including SPI in this file.
 *            Also provides interfaces to set synthesizer to a selected
 *            channel. Also contains the board level settings incase of
 *            band changes.
 * Author:    Mark Nelson
 * Date:      01/28/2005
 * History:-
 * Date       Modified by            Modification Information
 * --------------------------------------------------------------------------
 *
 */
#include "ani_assert.h"
#include "sys_api.h"


typedef tANI_U16 tRfChipSelect;
const tRfChipSelect chipSel[MAX_RF_CHIPS] =
{
    SPI_RF_CHIP_0_CONTROL  //QUASAR_CHIP
};

//static tANI_U8 TestSpiStatus(tpAniSirGlobal pMac);
static eHalStatus GetSpiReg(tpAniSirGlobal pMac, tANI_U16 regNum, tANI_U16 control, tANI_U32 * pVal);
static eHalStatus SetSpiReg(tpAniSirGlobal pMac, tANI_U16 wMsb, tANI_U16 wLsb, tANI_U16 control);

eHalStatus asicSpiInit(tpAniSirGlobal pMac)
{
    eHalStatus retVal;

    assert(pMac != 0);

    if (pMac->hphy.phy.test.testDisableSpiAccess)
        return (eHAL_STATUS_SUCCESS);
#ifdef FIXME_GEN5
    SET_PHY_REG(pMac->hHdd, SPI_CONFIG_REG, RFSPI_CONFIG_CLOCK_DIVIDE_E200_NS);
#endif
    return (retVal);
}





/* asicSpiReadDataRegister sets up correct info to make Spi read
    pMac is needed by the underlying functions - not good coupling
    rfChip is the combination of which SPI_RF_CHIP_0_CONTROL bits to set
    rfReg is the number of the RF chip's register to read
    pReadLoc is a pointer to the tANI_U32 variable to stuff the read data into
*/
eHalStatus asicSpiReadDataRegister(tpAniSirGlobal pMac, eRfChipSelect rfChip, tANI_U16 rfReg, tANI_U32 *pReadLoc)
{
    eHalStatus retVal = eHAL_STATUS_FAILURE;
    
    assert(pMac != 0);

    if (pMac->hphy.phy.test.testDisableSpiAccess)
        return (eHAL_STATUS_SUCCESS);

    if (rfChip == QUASAR_CHIP)
    {
        retVal = GetSpiReg(pMac, rfReg,
                            (tANI_U16)(SPI_READ_START |
                                   chipSel[rfChip] |
                                   ((SPI_READ_SIZE_MASK & SPI_QUASAR_READ_SIZE) << SPI_READ_SIZE_SHIFT) |
                                    (SPI_WRITE_SIZE_MASK & SPI_QUASAR_WRITE_ADDR_SIZE)
                                  ),
                           pReadLoc
                          );
    }

    return (retVal);
}


/* asicSpiWriteDataRegister sets up correct info to make Spi write
    pMac is needed by the underlying functions - not good coupling
    rfReg is the number of the RF chip's register to write
    wData is the data, it will be shifted and OR'd with the reg address
*/
eHalStatus asicSpiWriteDataRegister(tpAniSirGlobal pMac, eRfChipSelect rfChip, tANI_U16 rfReg, tANI_U32 wData)
{
    eHalStatus retVal = eHAL_STATUS_FAILURE;

    assert(pMac != 0);
    assert((rfChip == QUASAR_CHIP));
    assert(rfReg < QUASAR_NUM_REGS);

    if (pMac->hphy.phy.test.testDisableSpiAccess)
        return (eHAL_STATUS_SUCCESS);   //pretend it succeeded to stub out routine

    if (rfChip == QUASAR_CHIP)
    {
        //shadows the register
        pMac->hphy.rf.quasarRegCache[rfReg] = wData;

        wData = (wData << SPI_DATA_SHIFT) | ((tANI_U32 )rfReg << WR_SPI_REG_ADDR_OFFSET);
        retVal = SetSpiReg(pMac,
                           (tANI_U16)(( wData & (MSK_16 << 16)) >> 16),
                           (tANI_U16)(wData & MSK_16),
                           (tANI_U16)(SPI_WRITE_START |
                                  chipSel[rfChip] |
                                  (SPI_WRITE_SIZE_MASK & SPI_QUASAR_WRITE_SIZE)
                                 )
                          );
    }

    return (retVal);
}

// ---------------------------------------------------------------------------
/**
 * asicWaitForSpiBusyClear
 *
 * FUNCTION:
 *  Wait function for the SPI busy clear bit.
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param pMac Global MAC structure
 * @return status of the busy bit.
 */
static 
eHalStatus
asicWaitForSpiBusyClear(tpAniSirGlobal pMac, tANI_U32 *regVal)
{
    eHalStatus  retVal = eHAL_STATUS_SUCCESS;
#ifdef FIXME_GEN5    
    WAIT_PHY_REG_VAL(pMac, SPI_CONTROL_REG, SPI_BUSY_CONTROL, 
                         0, 1000, 50, regVal);
#endif
    return retVal;
}


/******************************************************************************

   GetSpiReg

   FUNCTION DESCRIPTION:
        Gets contents of a register accessed through the Spi bus

   LOGIC:

   ASSUMPTIONS:

   NOTE:

   PARAMETERS:
        tpAniSirGlobal pMac - needed to get register access across the HIF
        tANI_U16 regNum - number of register in RF Chip
        pRfChip - designates which chip to talk to
        tANI_U32 *pReadVal - location to read the register into

   RETURNS:
        eHalStatus

******************************************************************************/
static eHalStatus GetSpiReg(tpAniSirGlobal pMac, tANI_U16 regNum, tANI_U16 control, tANI_U32 * pVal)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
    tANI_U32 reg;
#ifndef ANI_MANF_DIAG
    tANI_U32 rd_on_off_hwpin_stat;
#endif
    tANI_U32 spiAccessBuffer[4];

    assert(pMac != 0);

    if (pMac->hphy.phy.test.testDisableSpiAccess)
        return retVal;

/*      for debug to ensure the SPI clocks are enabled
        {
            tANI_U32 reg;
            
            GET_PHY_REG(pMac->hHdd, RXCLKCTRL_APB_BLOCK_CLK_EN_REG, (&reg));
            if ((reg & RXCLKCTRL_APB_BLOCK_CLK_EN_RFSPI_MASK) == 0)
            {
                if ((retVal = rdModWrNovaField(pMac, RXCLKCTRL_APB_BLOCK_CLK_EN_REG, 
                                               RXCLKCTRL_APB_BLOCK_CLK_EN_RFSPI_MASK, 
                                               RXCLKCTRL_APB_BLOCK_CLK_EN_RFSPI_OFFSET, 
                                               1
                                              )
                    ) != eHAL_STATUS_SUCCESS
                   )
                {
                    return (retVal);
                }
            }
        }
*/
#ifdef FIXME_GEN5

    spiAccessBuffer[0] = RFSPI_CONFIG_CLOCK_DIVIDE_E200_NS;
    spiAccessBuffer[1] = 0;
    spiAccessBuffer[2] = ((tANI_U32)(regNum & SPI_REG_ADDR_MASK) << RD_SPI_REG_ADDR_OFFSET) | RD_SPI_RW_CONTROL;
    spiAccessBuffer[3] = control;
    // Convert all the individual register writes to a single memory write since the registers are contiguous. 
    // This gives good peroformance optimization in USB interface.
/*    
    SET_PHY_REG(pMac->hHdd, SPI_CONFIG_REG, RFSPI_CONFIG_CLOCK_DIVIDE_E200_NS);

    SET_PHY_REG(pMac->hHdd, SPI_WRITE_LSW_REG, ((tANI_U32)(regNum & SPI_REG_ADDR_MASK) << RD_SPI_REG_ADDR_OFFSET) | RD_SPI_RW_CONTROL);
    SET_PHY_REG(pMac->hHdd, SPI_WRITE_MSW_REG, 0);
    SET_PHY_REG(pMac->hHdd, SPI_CONTROL_REG, control);
*/
    SET_PHY_MEMORY(pMac->hHdd, SPI_CONFIG_REG, spiAccessBuffer, 4);
#endif
    {
//        tANI_U32 i = 0;

        retVal = asicWaitForSpiBusyClear(pMac, &reg);

        if (retVal != eHAL_STATUS_SUCCESS)
    {
            return retVal;
        }

/*
        do
        {
            sirBusyWait(1000);  //wait microsecond
            GET_PHY_REG(pMac->hHdd, SPI_CONTROL_REG, &reg);
            reg &= SPI_BUSY_CONTROL;
            
        }while (reg && (i++ < 50));
  */
        if (reg & SPI_BUSY_CONTROL)
        {
            phyLog(pMac, LOGE, "ERROR: GetSpiReg: SPI_BUSY_CONTROL is not cleared\n");
            retVal = eHAL_STATUS_FAILURE;
        }


        {
            //GET_PHY_REG(pMac->hHdd, SPI_READ_LSW_REG, &regL);
            //GET_PHY_REG(pMac->hHdd, SPI_READ_MSW_REG, &regH);
            //*pVal = (tANI_U16)regL | ((tANI_U16)regH << 16);
#ifdef FIXME_GEN5            
            GET_PHY_MEMORY(pMac->hHdd, SPI_READ_MSW_REG, &spiAccessBuffer[0], 2);
#endif
            *pVal = (tANI_U16)spiAccessBuffer[1] | ((tANI_U16)spiAccessBuffer[0] << 16);
        }
    }

#ifndef ANI_MANF_DIAG
    //check for the RF kill switch to be turned on
    if (pMac->hphy.phy.checkRfKill == eANI_BOOLEAN_TRUE)
    {
        GET_PHY_REG(pMac->hHdd, MCU_RF_ON_OFF_CONTROL_REG, &rd_on_off_hwpin_stat);
        if ( !(MCU_RF_ON_OFF_CONTROL_RD_ON_OFF_HWPIN_STAT_MASK & rd_on_off_hwpin_stat) )
        {
            pMac->hphy.phy.test.testDisableSpiAccess = eANI_BOOLEAN_TRUE;
            pMac->hphy.phy.test.testDisablePhyRegAccess = eANI_BOOLEAN_TRUE;
            return eHAL_STATUS_SUCCESS;
        }
    }
#endif

    return (retVal);
}

static eHalStatus SetSpiReg(tpAniSirGlobal pMac, tANI_U16 wMsb, tANI_U16 wLsb, tANI_U16 control)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;    
    tANI_U32    spiAccessBuffer[4];

#ifndef ANI_MANF_DIAG
    tANI_U32 rd_on_off_hwpin_stat;
    //check for the RF kill switch to be turned on
    if (pMac->hphy.phy.checkRfKill == eANI_BOOLEAN_TRUE)
    {
        GET_PHY_REG(pMac->hHdd, MCU_RF_ON_OFF_CONTROL_REG, &rd_on_off_hwpin_stat);
        if ( !(MCU_RF_ON_OFF_CONTROL_RD_ON_OFF_HWPIN_STAT_MASK & rd_on_off_hwpin_stat) )
        {
            pMac->hphy.phy.test.testDisableSpiAccess = eANI_BOOLEAN_TRUE;
            pMac->hphy.phy.test.testDisablePhyRegAccess = eANI_BOOLEAN_TRUE;
            return eHAL_STATUS_SUCCESS;
        }
    }
#endif

    if (pMac->hphy.phy.test.testDisableSpiAccess)
    {
        return retVal;
    }
    
/*      for debug to ensure the SPI clocks are enabled
        {
            tANI_U32 reg;
            
            GET_PHY_REG(pMac->hHdd, RXCLKCTRL_APB_BLOCK_CLK_EN_REG, (&reg));
            if ((reg & RXCLKCTRL_APB_BLOCK_CLK_EN_RFSPI_MASK) == 0)
            {
                if ((retVal = rdModWrNovaField(pMac, RXCLKCTRL_APB_BLOCK_CLK_EN_REG, 
                                               RXCLKCTRL_APB_BLOCK_CLK_EN_RFSPI_MASK, 
                                               RXCLKCTRL_APB_BLOCK_CLK_EN_RFSPI_OFFSET, 
                                               1
                                              )
                    ) != eHAL_STATUS_SUCCESS
                   )
                {
                    return (retVal);
                }
            }
        }
*/

    //Convert all the invidual register write to a single memory write since the registers are contiguous. 
    //This gives the more performance optimization for USB interface,
#if 0
    SET_PHY_REG(pMac->hHdd, SPI_WRITE_MSW_REG, (tANI_U32) wMsb);
    SET_PHY_REG(pMac->hHdd, SPI_WRITE_LSW_REG, (tANI_U32) wLsb);
    SET_PHY_REG(pMac->hHdd, SPI_CONTROL_REG, (tANI_U32) control);
#endif
    spiAccessBuffer[0] = (tANI_U32)wMsb;
    spiAccessBuffer[1] = (tANI_U32)wLsb;
    spiAccessBuffer[2] = (tANI_U32)control;
#ifdef FIXME_GEN5	
    SET_PHY_MEMORY(pMac->hHdd, SPI_WRITE_MSW_REG, spiAccessBuffer, 3);
#endif
    {
        tANI_U32 reg;
//        tANI_U32 i = 0;

        retVal = asicWaitForSpiBusyClear(pMac, &reg);
        
        if (retVal != eHAL_STATUS_SUCCESS)
        {
            return retVal;
        }

/*

        do
        {
            sirBusyWait(1000);  //wait microsecond
            GET_PHY_REG(pMac->hHdd, SPI_CONTROL_REG, &reg);
            reg &= SPI_BUSY_CONTROL;
            
        }while (reg && (i++ < 50));
*/

        if (reg & SPI_BUSY_CONTROL)
        {
            phyLog(pMac, LOGE, "ERROR: SetSpiReg: SPI_BUSY_CONTROL is not cleared\n");
            retVal = eHAL_STATUS_FAILURE;
        }
    }



    return retVal;
}


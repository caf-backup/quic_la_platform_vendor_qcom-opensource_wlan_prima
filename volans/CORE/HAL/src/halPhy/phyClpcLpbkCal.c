/**
 *
   Qualcomm, Inc proprietary.
   All Rights Reserved, Copyright 2011
   This program is the confidential and proprietary product of Airgo Networks Inc.
   Any Unauthorized use, reproduction or transfer of this program is strictly prohibited.


   phyClpcLpbkCal.c:
   Author:  Bharath Pandravada
   Date:    3/31/11

   History -
   Date        Modified by              Modification Information
  --------------------------------------------------------------------------

 */
#include <ani_assert.h>
#include <wlan_bit.h>
#include <sys_api.h>

/*
static void SetFrameParameters(sPttFrameGenParams *frameParams)
{
    // Set user contents:
    frameParams->numTestPackets = 256;
    frameParams->interFrameSpace = 16;
    frameParams->rate = HAL_PHY_RATE_11A_12_MBPS;
    frameParams->payloadLength = 696;//1000;
    frameParams->payloadContents = TEST_PAYLOAD_RANDOM;
    frameParams->payloadFillByte = 0;
    frameParams->pktAutoSeqNum = 0;
    frameParams->pktScramblerSeed = 0;
    frameParams->crc = 1;
    frameParams->preamble = PHYDBG_PREAMBLE_OFDM;

   return;
}

static void SetOneFrameParameters(sPttFrameGenParams *frameParams)
{
    // Set user contents:
    frameParams->numTestPackets = 1;
    frameParams->interFrameSpace = 16;
    frameParams->rate = HAL_PHY_RATE_11A_12_MBPS;
    frameParams->payloadLength = 696;//1000;
    frameParams->payloadContents = TEST_PAYLOAD_RANDOM;
    frameParams->payloadFillByte = 0;
    frameParams->pktAutoSeqNum = 0;
    frameParams->pktScramblerSeed = 0;
    frameParams->crc = 1;
    frameParams->preamble = PHYDBG_PREAMBLE_OFDM;

   return;
}
*/
static eHalStatus setup(tpAniSirGlobal pMac)
{
    eHalStatus retVal;
    SET_PHY_REG(pMac->hHdd, QWLAN_RFAPB_MC_OUT_OVRD_1_REG, 0x4);

    SET_PHY_REG(pMac->hHdd, QWLAN_RFAPB_MC_OUT_OVRD_0_REG, 0x7de6);

    rdModWrAsicField(pMac, QWLAN_RFAPB_MODE_SEL1_REG, QWLAN_RFAPB_MODE_SEL1_TXRX_CAL_MODE_MASK,
            QWLAN_RFAPB_MODE_SEL1_TXRX_CAL_MODE_OFFSET, 6);

    SET_PHY_REG(pMac->hHdd, QWLAN_AGC_DIS_MODE_REG, 2); //disable 11b pkt
    SET_PHY_REG(pMac->hHdd, QWLAN_TPC_TXPWR_ENABLE_REG, 0);//open the clpc loop

    rdModWrAsicField(pMac, QWLAN_AGC_PHY_LOOPBACK_REG, QWLAN_AGC_PHY_LOOPBACK_MODE_MASK,
        QWLAN_AGC_PHY_LOOPBACK_MODE_OFFSET, 1);

    SET_PHY_REG(pMac->hHdd, QWLAN_AGC_INIT_GAIN_REG, 8);
    rdModWrAsicField(pMac, QWLAN_AGC_COARSE_STEP_REG, QWLAN_AGC_COARSE_STEP_COARSE_STEP_MASK,
        QWLAN_AGC_COARSE_STEP_COARSE_STEP_OFFSET, 8);
    rdModWrAsicField(pMac, QWLAN_AGC_SAT_STEP_REG, QWLAN_AGC_SAT_STEP_SAT_STEP_EN_MASK,
    QWLAN_AGC_SAT_STEP_SAT_STEP_EN_OFFSET, 0);
    SET_PHY_REG(pMac->hHdd, QWLAN_AGC_AGC_RESET_REG, 0);
    SET_PHY_REG(pMac->hHdd, QWLAN_AGC_TH_CD_REG, 50);

    return (eHAL_STATUS_SUCCESS);

}

static eHalStatus restore(tpAniSirGlobal pMac)
{
    eHalStatus retVal;
    rdModWrAsicField(pMac, QWLAN_RFAPB_MODE_SEL1_REG, QWLAN_RFAPB_MODE_SEL1_TXRX_CAL_MODE_MASK,
            QWLAN_RFAPB_MODE_SEL1_TXRX_CAL_MODE_OFFSET, 0);

    rdModWrAsicField(pMac, QWLAN_AGC_PHY_LOOPBACK_REG, QWLAN_AGC_PHY_LOOPBACK_MODE_MASK,
        QWLAN_AGC_PHY_LOOPBACK_MODE_OFFSET, 0);

    SET_PHY_REG(pMac->hHdd, QWLAN_TPC_TXPWR_ENABLE_REG, 1); //close the loop

    SET_PHY_REG(pMac->hHdd, QWLAN_AGC_INIT_GAIN_REG, 71);

    rdModWrAsicField(pMac, QWLAN_AGC_COARSE_STEP_REG, QWLAN_AGC_COARSE_STEP_COARSE_STEP_MASK,
        QWLAN_AGC_COARSE_STEP_COARSE_STEP_OFFSET, 26);

    rdModWrAsicField(pMac, QWLAN_AGC_SAT_STEP_REG, QWLAN_AGC_SAT_STEP_SAT_STEP_EN_MASK,
        QWLAN_AGC_SAT_STEP_SAT_STEP_EN_OFFSET, 1);

    SET_PHY_REG(pMac->hHdd, QWLAN_AGC_DIS_MODE_REG, 0);
    SET_PHY_REG(pMac->hHdd, QWLAN_AGC_AGC_RESET_REG, 0);
    SET_PHY_REG(pMac->hHdd, QWLAN_AGC_TH_CD_REG, 0);
    SET_PHY_REG(pMac->hHdd, QWLAN_TACTL_SCR_CONFIG_REG, QWLAN_TACTL_SCR_CONFIG_DEFAULT);

    return (eHAL_STATUS_SUCCESS);
}

static eHalStatus lpbkCalPktGen(tpAniSirGlobal pMac, tANI_U32 txPktCnt, tANI_U8 tpcLutIdx)
{
    eHalStatus retVal;

    SET_PHY_REG(pMac->hHdd, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_REG, 0xff9L);
    SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_WARMUP_DLY_REG, 0xa8);
    SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_CFGMODE_REG, 0x3100L);
    SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_MAX_ADDR1_REG, 0xc);
    SET_PHY_REG(pMac->hHdd, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_REG, 0xff9L);
    SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_DBGMEM_MREG + 0 , 0xd797);
    SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_DBGMEM_MREG + 4 , 0x1);
    SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_DBGMEM_MREG + 8 , 0x2d0);
    SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_DBGMEM_MREG + 12, 0x0);
    SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_DBGMEM_MREG + 16, 0xb);
    SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_DBGMEM_MREG + 20, 0x0);
    SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_DBGMEM_MREG + 24, 0x0);
    SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_DBGMEM_MREG + 28, 0x0);
    SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_DBGMEM_MREG + 32, 0x0);
    SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_DBGMEM_MREG + 36, 0x0);
    SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_DBGMEM_MREG + 40, tpcLutIdx);
    SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_DBGMEM_MREG + 44, 0x0);
    SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_DBGMEM_MREG + 48, 0x0);
    SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_DBGMEM_MREG + 52, 0x0);
    SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_DBGMEM_MREG + 56, 0xd4);
    SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_DBGMEM_MREG + 60, 0x0);
    SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_DBGMEM_MREG + 64, 0x2);
    SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_DBGMEM_MREG + 68, 0x0);
    SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_DBGMEM_MREG + 72, 0xa);
    SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_DBGMEM_MREG + 76, 0x0);
    SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_DBGMEM_MREG + 80, 0xd4);
    SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_DBGMEM_MREG + 84, 0x0);
    SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_DBGMEM_MREG + 88, 0x2);
    SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_DBGMEM_MREG + 92, 0x0);
    SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_DBGMEM_MREG + 96, 0xa);
    SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_DBGMEM_MREG + 100, 0x0);
    SET_PHY_REG(pMac->hHdd, QWLAN_MPI_MPI_ENABLE_REG, 0x1);
    SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_RST1_REG, 0x0);
    SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_CFGMODE_REG, 0x2100L);
    SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_PLYBCK_CFG_REG, 0x7L);
    SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_START_ADDR1_REG, 0x0);
    SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_PLYBCK_CFG2_REG, 0x0L);
    SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_PRBS_REG, 0x804bL);
    SET_PHY_REG(pMac->hHdd, QWLAN_TACTL_SCR_CONFIG_REG, 0x17fL);
    SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_PRBS_REG, 0x804bL);
    SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_PRBS_MS_REG, 0x0);
    SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_PRBS_LOAD_REG, 0x1);
    SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_TXPKT_CNT_REG, txPktCnt);
    SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_PLYBKCNT_REG, 0x0);
    SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_START1_REG, 0x1);

    if(txPktCnt == 1)
    {
        vos_sleep_us(10000);
    }
    else
    {
        vos_sleep_us(50000);
    }

    SET_PHY_REG(pMac->hHdd, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_REG, 0xff9L);
    SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_CFGMODE_REG, 0x3100L);
    vos_sleep_us(100);
    SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_PLYBCK_CFG_REG, 0x6L);

    return (eHAL_STATUS_SUCCESS);
}

static eHalStatus setupRxaEvm(tpAniSirGlobal pMac, tANI_U8 tpcLutIdx)
{
    eHalStatus retVal;

    SET_PHY_REG(pMac->hHdd, QWLAN_RACTL_RACTL_SNRSTATSCLR_REG, 1);
    SET_PHY_REG(pMac->hHdd, QWLAN_RACTL_RACTL_SNRSTATSCFG_REG, 2); //t.csr.ractl.ractl_snrstatscfg(collect_always=1,lock=0)

    lpbkCalPktGen(pMac, 1, tpcLutIdx);

    SET_PHY_REG(pMac->hHdd, QWLAN_RACTL_RACTL_SNRSTATSCFG_REG, 3);//t.csr.ractl.ractl_snrstatscfg(lock=1)

    return (eHAL_STATUS_SUCCESS);
}

#define VALID_EVM_MIN_N_SYMS   246
static eHalStatus measureRxaEvm(tpAniSirGlobal pMac, tANI_S32 *lpbkEvm, tANI_U8 tpcLutIdx)
{
    eHalStatus retVal;
    tANI_S32 snrAccum = 0, snrTmp = 0;
    tANI_U32 stats_symcnt;
    tANI_U8 nRepeat  = 10, repeatCnt;

    setupRxaEvm(pMac, tpcLutIdx);

    for (repeatCnt = 0; repeatCnt < nRepeat; repeatCnt++)
    {
        tANI_U8 done = 0;
        tANI_U8 cnt = 0;
        tANI_U8 cntTh = 10;
        while( (!done) && (cnt < cntTh) )
        {
            SET_PHY_REG(pMac->hHdd, QWLAN_RACTL_RACTL_SNRSTATSCLR_REG, 1);
            SET_PHY_REG(pMac->hHdd, QWLAN_RACTL_RACTL_SNRSTATSCFG_REG, 2); //t.csr.ractl.ractl_snrstatscfg(collect_always=1,lock=0)

/*
            {
                tANI_BOOLEAN start = 1;
                sPttFrameGenParams frameParams;
                SetOneFrameParameters(&frameParams);
                pttConfigTxPacketGen(pMac, frameParams);
                pttStartStopTxPacketGen(pMac, start);
            }
            vos_sleep_us(100);
*/
            lpbkCalPktGen(pMac, 1, tpcLutIdx);

            SET_PHY_REG(pMac->hHdd, QWLAN_RACTL_RACTL_SNRSTATSCFG_REG, 3);//t.csr.ractl.ractl_snrstatscfg(lock=1)

            cnt = cnt + 1;

            GET_PHY_REG(pMac->hHdd, QWLAN_RACTL_RACTL_SNRSTATS_SYMCNT_REG, &stats_symcnt);
            //printk("measureRxaEvm: stats_symcnt =%d\n", (unsigned int)stats_symcnt);

            done = (stats_symcnt == VALID_EVM_MIN_N_SYMS) ? 1 : 0;

            //this will restore the ptt state machine for phyDbg frame gen
            //pttStartStopTxPacketGen(pMac, 0);
        }
        if (done)
        {
            tANI_U32 stats;

            GET_PHY_REG(pMac->hHdd, QWLAN_RACTL_RACTL_SNRSTATS_REG, &stats);
            GET_PHY_REG(pMac->hHdd, QWLAN_RACTL_RACTL_SNRSTATS_REG, &stats);
            GET_PHY_REG(pMac->hHdd, QWLAN_RACTL_RACTL_SNRSTATS_REG, &stats);
            snrAccum += (tANI_S16)stats;
            //printk("cnt = %d\n", cnt);
            //printk("snr_stats = %d, symcnt = %d, snrAccum = %d\n",(tANI_S16)stats ,(unsigned int)stats_symcnt, (int)snrAccum);

        }
        else
        {
            //printk("Error in measureRxaEvm. Did not detect correct number of symbols after %d packets\n", cntTh);
            *lpbkEvm = -20;
            return (eHAL_STATUS_SUCCESS);
        }

    }
    //snrTmp = (snrAccum * 256) / (nRepeat * stats_symcnt);
    //snrTmp = (snrAccum * 256.0) / (nRepeat * t.csr.ractl.ractl_snrstats_symcnt.stats_symcnt())
    //return (22*2) + (snrTmp / 256.0);
    if(snrAccum < 0)
    {
        snrTmp = (-snrAccum * 10) / (nRepeat * stats_symcnt);
        snrTmp = -snrTmp;
    }
    else
    {
        snrTmp = (snrAccum * 10) / (nRepeat * stats_symcnt);
    }

    *lpbkEvm = ((22*2*10) + (snrTmp));
    //printk( "nRepeat = %d, stats_symcnt = %d, snrTmp =%d\n", nRepeat, (unsigned int)stats_symcnt, (int)snrTmp);
    //printk( "returned lpbkEvm =%d\n", (int)(*lpbkEvm));

    return (eHAL_STATUS_SUCCESS);
}

static tANI_U8 getTpcIdx(t2Decimal dbmPwr)
{
    return  (((2 * dbmPwr)/100) - 17);
}

#define OFDM_MAX_RF_GAIN        8
#define OFDM_MAX_CMD_PWR_OFFSET 150

eHalStatus phyClpcLpbkCal(tpAniSirGlobal pMac)
{
    eHalStatus retVal;
    //t.board.rf_eval.ofdm_cmd_pwr_offset = 0.0
    tANI_U8 tpcLutIdx = 22;//(2*19.5 - 17)t.board.rf_eval.get_tpc_gain_lut_idx(tx_pwr,'11a')
    tANI_U8 done = 0;
    tANI_S32 lpbkEvm = 0, evmThr = -225;
    tANI_U32 tpcLutTxPwr;
    eRfChannels curChan = rfGetCurChannel(pMac);
    t2Decimal boardLoss = pMac->hphy.phy.antennaPathLoss[curChan];
    t2Decimal lpBktxPwr = OFDM_MAX_CHIP_OUTPUT_PWR;
    t2Decimal deltaOffsetdBm = 0;
    t2Decimal cmdDbmPwr = lpBktxPwr - boardLoss;

    pMac->hphy.phy.test.identicalPayloadEnabled = eANI_BOOLEAN_TRUE;

    //close the loop
    SET_PHY_REG(pMac->hHdd, QWLAN_TPC_TXPWR_ENABLE_REG, 1);
/*
    {
        sPttFrameGenParams frameParams;
        SetFrameParameters(&frameParams);
        pttConfigTxPacketGen(pMac, frameParams);
    }
*/
    while((!done) && (deltaOffsetdBm < OFDM_MAX_CMD_PWR_OFFSET))
    {


        tpcLutIdx = getTpcIdx(cmdDbmPwr - deltaOffsetdBm);
        // save of contents of tpc gain lut
        GET_PHY_REG(pMac->hHdd, QWLAN_TPC_GAIN_LUT0_MREG + (4 * tpcLutIdx), &tpcLutTxPwr);
        SET_PHY_REG(pMac->hHdd, QWLAN_TPC_GAIN_LUT0_MREG + (4 * tpcLutIdx), 0x7F);
        //t.csr.tpc.gain_lut0[tpcLutIdx](rf_gain=7,dig_gain=15)

        // let clpc converge
        //tx_packet(t,tx_pwr=tpcLutIdx,n_packets=256,short_pkts=True)

        {
            //pttSetTxPower(pMac, cmdDbmPwr - deltaOffsetdBm);
            //pttStartStopTxPacketGen(pMac, 1);
            tANI_U32 tpcLutConvergedTxPwr;
            tANI_U8 rf_gain, dig_gain;

            lpbkCalPktGen(pMac, 256, tpcLutIdx);
            //vos_sleep_us(50000);

            //this will restore the ptt state machine for phyDbg frame gen
            //pttStartStopTxPacketGen(pMac, 0);


            //monitor the rf and dig gain
            GET_PHY_REG(pMac->hHdd, QWLAN_TPC_GAIN_LUT0_MREG + (4 * tpcLutIdx), &tpcLutConvergedTxPwr);

            rf_gain = (tpcLutConvergedTxPwr >> 4) & 0xf;
            dig_gain = (tpcLutConvergedTxPwr) & 0xf;

            if( (rf_gain > OFDM_MAX_RF_GAIN) || ( (rf_gain == OFDM_MAX_RF_GAIN) && (dig_gain > 13) ) )
            {
                //t.board.rf_eval.ofdm_cmd_pwr_offset += 0.5
                deltaOffsetdBm += 50;
                //if(deltaOffsetdBm != 150)
                //{
                //    SET_PHY_REG(pMac->hHdd, QWLAN_TPC_GAIN_LUT0_MREG + (4 * tpcLutIdx), tpcLutTxPwr);
                //}
            }
            else
            {
                done = 1;
            }

        }
    }

    setup(pMac);

    measureRxaEvm(pMac, &lpbkEvm, tpcLutIdx);

    SET_PHY_REG(pMac->hHdd, QWLAN_TPC_GAIN_LUT0_MREG + (4 * tpcLutIdx), tpcLutTxPwr);

    restore(pMac);

    pMac->hphy.phy.test.identicalPayloadEnabled = eANI_BOOLEAN_FALSE;

    if(lpbkEvm == 440)
    {
        return (eHAL_STATUS_SUCCESS);
    }

    lpbkEvm = -lpbkEvm/2 + (3 * 10);

    //printk( "lpbkEvm =%d\n", (int)lpbkEvm);

    if (lpbkEvm > evmThr)
    {
        tANI_S32 deltaEvm = ((lpbkEvm - evmThr)/10) * 10;
        tANI_S32 offset = ((1 * 10) + deltaEvm) >> 1;

        if (offset > 15)
        {
            offset = 150;
        }
        else
        {
            offset = offset * 10;
        }

        //printk("deltaEvm =%d\n", (int)deltaEvm);
        //printk("offset =%d\n", (int)offset);
        //printk("deltaOffsetdBm =%d\n", (int)deltaOffsetdBm);
        offset += deltaOffsetdBm;
        if(offset > OFDM_MAX_CMD_PWR_OFFSET)
        {
            offset = OFDM_MAX_CMD_PWR_OFFSET;
        }
        {
            halWriteNvTable(pMac, NV_TABLE_OFDM_CMD_PWR_OFFSET, (uNvTables *)(&offset));
#ifndef WLAN_FTM_STUB
            if (pMac->gDriverType == eDRIVER_TYPE_MFG)
            {
                halStoreTableToNv(pMac, NV_TABLE_OFDM_CMD_PWR_OFFSET);
            }
#endif
        }

        //t.board.rf_eval.ofdm_cmd_pwr_offset = offset
    }
    return (eHAL_STATUS_SUCCESS);
}



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
#include <pttModuleApi.h>

#ifndef WLAN_FTM_STUB

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

    return (eHAL_STATUS_SUCCESS);
}

#define VALID_EVM_MIN_N_SYMS   246
static eHalStatus measureRxaEvm(tpAniSirGlobal pMac, tANI_S32 *lpbkEvm)
{
    eHalStatus retVal;
    tANI_S32 snrAccum = 0, snrTmp = 0;
    tANI_U32 stats_symcnt;
    tANI_U8 nRepeat  = 10, repeatCnt;
    for (repeatCnt = 0; repeatCnt < nRepeat; repeatCnt++)
    {
        tANI_U8 done = 0;
        tANI_U8 cnt = 0;
        tANI_U8 cntTh = 10;
        while( (!done) && (cnt < cntTh) )
        {
            SET_PHY_REG(pMac->hHdd, QWLAN_RACTL_RACTL_SNRSTATSCLR_REG, 1);
            SET_PHY_REG(pMac->hHdd, QWLAN_RACTL_RACTL_SNRSTATSCFG_REG, 2); //t.csr.ractl.ractl_snrstatscfg(collect_always=1,lock=0)

            {
                tANI_BOOLEAN start = 1;
                sPttFrameGenParams frameParams;
                SetOneFrameParameters(&frameParams);
                pttConfigTxPacketGen(pMac, frameParams);
                pttStartStopTxPacketGen(pMac, start);
            }
            vos_sleep_us(100);

            SET_PHY_REG(pMac->hHdd, QWLAN_RACTL_RACTL_SNRSTATSCFG_REG, 3);//t.csr.ractl.ractl_snrstatscfg(lock=1)

            cnt = cnt + 1;

            GET_PHY_REG(pMac->hHdd, QWLAN_RACTL_RACTL_SNRSTATS_SYMCNT_REG, &stats_symcnt);
            //printk("measureRxaEvm: stats_symcnt =%d\n", (unsigned int)stats_symcnt);

            done = (stats_symcnt == VALID_EVM_MIN_N_SYMS) ? 1 : 0;

            //this will restore the ptt state machine for phyDbg frame gen
            pttStartStopTxPacketGen(pMac, 0);
        }
        if (done)
        {
            tANI_U32 stats;

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

eHalStatus phyClpcLpbkCal(tpAniSirGlobal pMac)
{
    eHalStatus retVal;
    //t.board.rf_eval.ofdm_cmd_pwr_offset = 0.0
    tANI_U8 tpcLutIdx = 22;//(2*19.5 - 17)t.board.rf_eval.get_tpc_gain_lut_idx(tx_pwr,'11a')
    tANI_S32 lpbkEvm = 0, evmThr = -225;
    tANI_U32 tpcLutTxPwr;

    pMac->hphy.phy.test.identicalPayloadEnabled = eANI_BOOLEAN_TRUE;
    // save of contents of tpc gain lut
    GET_PHY_REG(pMac->hHdd, QWLAN_TPC_GAIN_LUT0_MREG + (4 * tpcLutIdx), &tpcLutTxPwr);
    SET_PHY_REG(pMac->hHdd, QWLAN_TPC_GAIN_LUT0_MREG + (4 * tpcLutIdx), 0x7F);
    //t.csr.tpc.gain_lut0[tpcLutIdx](rf_gain=7,dig_gain=15)

    // let clpc converge
    //tx_packet(t,tx_pwr=tpcLutIdx,n_packets=256,short_pkts=True)

    SET_PHY_REG(pMac->hHdd, QWLAN_TPC_TXPWR_ENABLE_REG, 1);

    {
        tANI_BOOLEAN start = 1;
        t2Decimal dbmPwr = 1950;
        sPttFrameGenParams frameParams;
        SetFrameParameters(&frameParams);
        pttConfigTxPacketGen(pMac, frameParams);
        pttSetTxPower(pMac, dbmPwr);
        pttStartStopTxPacketGen(pMac, start);

        vos_sleep_us(50000);

        //this will restore the ptt state machine for phyDbg frame gen
        pttStartStopTxPacketGen(pMac, 0);
    }

    setup(pMac);

    measureRxaEvm(pMac, &lpbkEvm);
    lpbkEvm = -lpbkEvm/2 + (3 * 10);

    //printk( "lpbkEvm =%d\n", (int)lpbkEvm);

    SET_PHY_REG(pMac->hHdd, QWLAN_TPC_GAIN_LUT0_MREG + (4 * tpcLutIdx), tpcLutTxPwr);

    restore(pMac);

    pMac->hphy.phy.test.identicalPayloadEnabled = eANI_BOOLEAN_FALSE;

    if (lpbkEvm > evmThr)
    {
        tANI_S32 deltaEvm = (lpbkEvm - evmThr);
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
        {
            halWriteNvTable(pMac, NV_TABLE_OFDM_CMD_PWR_OFFSET, (uNvTables *)(&offset));
            halStoreTableToNv(pMac, NV_TABLE_OFDM_CMD_PWR_OFFSET);
        }

        //t.board.rf_eval.ofdm_cmd_pwr_offset = offset
    }
    return (eHAL_STATUS_SUCCESS);
}
#endif


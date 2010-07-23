/** ------------------------------------------------------------------------- *
    ------------------------------------------------------------------------- *


    \file halQFuse.c

    \brief qfuse facilities

    $Id$

    Copyright (C) 2008 Qualcomm, Incorporated


   ========================================================================== */

#include "aniGlobal.h"
#include "ani_assert.h"
#include "halDebug.h"
#include "halRfTypes.h"

extern const sHalNv nvDefaults;


enum
{
    QFUSE_NOT_BLOWN = 0,
    QFUSE_BLOWN = 1
};

eHalStatus palWaitRegVal( tHddHandle hHdd, tANI_U32 reg, tANI_U32 mask,
                             tANI_U32 waitRegVal, tANI_U32 perIterWaitInNanoSec,
                             tANI_U32 numIter, tANI_U32 *pReadRegVal )
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hHdd;
    eHalStatus  nStatus = eHAL_STATUS_SUCCESS;
    //tANI_U32    waitTime = (perIterWaitInNanoSec + 999) / 1000;

    do
    {
        nStatus = palReadRegister(pMac->hHdd, reg, pReadRegVal);
        if (nStatus != eHAL_STATUS_SUCCESS)
        {
            break;
        }

        if ((*pReadRegVal & mask) == waitRegVal)
        {
            break;
        }

        if (--numIter)
        {
            sirBusyWait(1000);  //wait 1 microsecond
        }
        else
        {
            nStatus = eHAL_STATUS_FAILURE;
            break;
        }

    } while (1);

    return nStatus;
}

void halQFusePackBits(tHalHandle hMac)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hMac;

    assert(pMac->hphy.nvCache.tables.tpcConfig[0].freq == START_TPC_CHANNEL);
    assert(pMac->hphy.nvCache.tables.tpcConfig[1].freq == END_TPC_CHANNEL);

    pMac->hphy.nvCache.tables.qFuseData.dword0_chan1_calpoint_0_pwrDetAdc       = pMac->hphy.nvCache.tables.tpcConfig[0].empirical[0][0].pwrDetAdc;
    pMac->hphy.nvCache.tables.qFuseData.dword0_chan1_calpoint_0_adjustedPwrDet  = pMac->hphy.nvCache.tables.tpcConfig[0].empirical[0][0].adjustedPwrDet;
    pMac->hphy.nvCache.tables.qFuseData.dword0_chan1_calpoint_1_pwrDetAdc       = pMac->hphy.nvCache.tables.tpcConfig[0].empirical[0][1].pwrDetAdc;
    pMac->hphy.nvCache.tables.qFuseData.dword0_chan1_calpoint_1_adjustedPwrDet  = pMac->hphy.nvCache.tables.tpcConfig[0].empirical[0][1].adjustedPwrDet;
    pMac->hphy.nvCache.tables.qFuseData.dword0_chan1_calpoint_2_adjustedPwrDet  = pMac->hphy.nvCache.tables.tpcConfig[0].empirical[0][2].adjustedPwrDet & 0xF;

    pMac->hphy.nvCache.tables.qFuseData.dword1_chan1_calpoint_2_pwrDetAdc       = pMac->hphy.nvCache.tables.tpcConfig[0].empirical[0][2].pwrDetAdc;
    pMac->hphy.nvCache.tables.qFuseData.dword1_chan1_calpoint_2_adjustedPwrDet  = pMac->hphy.nvCache.tables.tpcConfig[0].empirical[0][2].adjustedPwrDet >> 4;
    pMac->hphy.nvCache.tables.qFuseData.dword1_chan1_calpoint_3_pwrDetAdc       = pMac->hphy.nvCache.tables.tpcConfig[0].empirical[0][3].pwrDetAdc;
    pMac->hphy.nvCache.tables.qFuseData.dword1_chan1_calpoint_3_adjustedPwrDet  = pMac->hphy.nvCache.tables.tpcConfig[0].empirical[0][3].adjustedPwrDet;
    pMac->hphy.nvCache.tables.qFuseData.dword1_chan13_calpoint_0_pwrDetAdc      = pMac->hphy.nvCache.tables.tpcConfig[1].empirical[0][0].pwrDetAdc & 0x1;
    pMac->hphy.nvCache.tables.qFuseData.dword1_chan13_calpoint_0_adjustedPwrDet = pMac->hphy.nvCache.tables.tpcConfig[1].empirical[0][0].adjustedPwrDet;

    pMac->hphy.nvCache.tables.qFuseData.dword2_chan13_calpoint_0_pwrDetAdc      = pMac->hphy.nvCache.tables.tpcConfig[1].empirical[0][0].pwrDetAdc >> 1;
    pMac->hphy.nvCache.tables.qFuseData.dword2_chan13_calpoint_1_pwrDetAdc      = pMac->hphy.nvCache.tables.tpcConfig[1].empirical[0][1].pwrDetAdc;
    pMac->hphy.nvCache.tables.qFuseData.dword2_chan13_calpoint_1_adjustedPwrDet = pMac->hphy.nvCache.tables.tpcConfig[1].empirical[0][1].adjustedPwrDet;
    pMac->hphy.nvCache.tables.qFuseData.dword2_chan13_calpoint_2_pwrDetAdc      = pMac->hphy.nvCache.tables.tpcConfig[1].empirical[0][2].pwrDetAdc & 0x1F;
    pMac->hphy.nvCache.tables.qFuseData.dword2_chan13_calpoint_2_adjustedPwrDet = pMac->hphy.nvCache.tables.tpcConfig[1].empirical[0][2].adjustedPwrDet;
    pMac->hphy.nvCache.tables.qFuseData.dword3_chan13_calpoint_2_pwrDetAdc      = pMac->hphy.nvCache.tables.tpcConfig[1].empirical[0][2].pwrDetAdc >> 5;

    pMac->hphy.nvCache.tables.qFuseData.dword3_chan13_calpoint_3_pwrDetAdc      = pMac->hphy.nvCache.tables.tpcConfig[1].empirical[0][3].pwrDetAdc;
    pMac->hphy.nvCache.tables.qFuseData.dword3_chan13_calpoint_3_adjustedPwrDet = pMac->hphy.nvCache.tables.tpcConfig[1].empirical[0][3].adjustedPwrDet;

    pMac->hphy.nvCache.tables.qFuseData.dword3_rf_hdet_ctl_ext_atten            = pMac->hphy.nvCache.tables.rfCalValues.hdet_ctl_ext_atten;
    pMac->hphy.nvCache.tables.qFuseData.dword3_rf_hdet_dcoc_code                = pMac->hphy.nvCache.tables.rfCalValues.hdet_dcoc_code;
    pMac->hphy.nvCache.tables.qFuseData.dword3_rf_hdet_dcoc_ib_rcal_en          = pMac->hphy.nvCache.tables.rfCalValues.hdet_dcoc_ib_rcal_en;
    pMac->hphy.nvCache.tables.qFuseData.dword3_rf_hdet_dcoc_ib_scal_en          = pMac->hphy.nvCache.tables.rfCalValues.hdet_dcoc_ib_scal_en;
}

#if 0
static void halQFuseParseBits(tHalHandle hMac)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hMac;

    if (halIsQFuseBlown(hMac) == eHAL_STATUS_SUCCESS)
    {
        pMac->hphy.nvCache.tables.tpcConfig[0].freq = START_TPC_CHANNEL;
        //pMac->hphy.nvCache.tables.tpcConfig[0].absPower.min;
        //pMac->hphy.nvCache.tables.tpcConfig[0].absPower.max;
        pMac->hphy.nvCache.tables.tpcConfig[0].empirical[0][0].pwrDetAdc      = (tPowerDetect)pMac->hphy.nvCache.tables.qFuseData.dword0_chan1_calpoint_0_pwrDetAdc;
        pMac->hphy.nvCache.tables.tpcConfig[0].empirical[0][0].adjustedPwrDet = (tPowerDetect)pMac->hphy.nvCache.tables.qFuseData.dword0_chan1_calpoint_0_adjustedPwrDet;
        pMac->hphy.nvCache.tables.tpcConfig[0].empirical[0][1].pwrDetAdc      = (tPowerDetect)pMac->hphy.nvCache.tables.qFuseData.dword0_chan1_calpoint_1_pwrDetAdc;
        pMac->hphy.nvCache.tables.tpcConfig[0].empirical[0][1].adjustedPwrDet = (tPowerDetect)pMac->hphy.nvCache.tables.qFuseData.dword0_chan1_calpoint_1_adjustedPwrDet;
        pMac->hphy.nvCache.tables.tpcConfig[0].empirical[0][2].pwrDetAdc      = (tPowerDetect)pMac->hphy.nvCache.tables.qFuseData.dword1_chan1_calpoint_2_pwrDetAdc;
        pMac->hphy.nvCache.tables.tpcConfig[0].empirical[0][2].adjustedPwrDet = (tPowerDetect)((pMac->hphy.nvCache.tables.qFuseData.dword1_chan1_calpoint_2_adjustedPwrDet << 4)
                                                                                                  | pMac->hphy.nvCache.tables.qFuseData.dword0_chan1_calpoint_2_adjustedPwrDet);
        pMac->hphy.nvCache.tables.tpcConfig[0].empirical[0][3].pwrDetAdc      = (tPowerDetect)pMac->hphy.nvCache.tables.qFuseData.dword1_chan1_calpoint_3_pwrDetAdc;
        pMac->hphy.nvCache.tables.tpcConfig[0].empirical[0][3].adjustedPwrDet = (tPowerDetect)pMac->hphy.nvCache.tables.qFuseData.dword1_chan1_calpoint_3_adjustedPwrDet;

        pMac->hphy.nvCache.tables.tpcConfig[1].freq = END_TPC_CHANNEL;
        pMac->hphy.nvCache.tables.tpcConfig[1].empirical[0][0].pwrDetAdc      = (tPowerDetect)((pMac->hphy.nvCache.tables.qFuseData.dword2_chan13_calpoint_0_pwrDetAdc << 1)
                                                                                                  | pMac->hphy.nvCache.tables.qFuseData.dword1_chan13_calpoint_0_pwrDetAdc);
        pMac->hphy.nvCache.tables.tpcConfig[1].empirical[0][0].adjustedPwrDet = (tPowerDetect)pMac->hphy.nvCache.tables.qFuseData.dword1_chan13_calpoint_0_adjustedPwrDet;
        pMac->hphy.nvCache.tables.tpcConfig[1].empirical[0][1].pwrDetAdc      = (tPowerDetect)pMac->hphy.nvCache.tables.qFuseData.dword2_chan13_calpoint_1_pwrDetAdc;
        pMac->hphy.nvCache.tables.tpcConfig[1].empirical[0][1].adjustedPwrDet = (tPowerDetect)pMac->hphy.nvCache.tables.qFuseData.dword2_chan13_calpoint_1_adjustedPwrDet;
        pMac->hphy.nvCache.tables.tpcConfig[1].empirical[0][2].pwrDetAdc      = (tPowerDetect)((pMac->hphy.nvCache.tables.qFuseData.dword3_chan13_calpoint_2_pwrDetAdc << 5)
                                                                                                  | pMac->hphy.nvCache.tables.qFuseData.dword2_chan13_calpoint_2_pwrDetAdc);
        pMac->hphy.nvCache.tables.tpcConfig[1].empirical[0][2].adjustedPwrDet = (tPowerDetect)pMac->hphy.nvCache.tables.qFuseData.dword2_chan13_calpoint_2_adjustedPwrDet;
        pMac->hphy.nvCache.tables.tpcConfig[1].empirical[0][3].pwrDetAdc      = (tPowerDetect)pMac->hphy.nvCache.tables.qFuseData.dword3_chan13_calpoint_3_pwrDetAdc;
        pMac->hphy.nvCache.tables.tpcConfig[1].empirical[0][3].adjustedPwrDet = (tPowerDetect)pMac->hphy.nvCache.tables.qFuseData.dword3_chan13_calpoint_3_adjustedPwrDet;

        //pMac->hphy.pwrParams.numTpcCalPointsPerFreq = MAX_TPC_CAL_POINTS;
        //pMac->hphy.pwrParams.numTpcCalFreqs = MAX_TPC_CHANNELS;

        pMac->hphy.nvCache.tables.rfCalValues.hdet_ctl_ext_atten              = (tANI_U8)pMac->hphy.nvCache.tables.qFuseData.dword3_rf_hdet_ctl_ext_atten  ;
        pMac->hphy.nvCache.tables.rfCalValues.hdet_dcoc_code                  = (tANI_U8)pMac->hphy.nvCache.tables.qFuseData.dword3_rf_hdet_dcoc_code      ;
        pMac->hphy.nvCache.tables.rfCalValues.hdet_dcoc_ib_rcal_en            = (tANI_U8)pMac->hphy.nvCache.tables.qFuseData.dword3_rf_hdet_dcoc_ib_rcal_en;
        pMac->hphy.nvCache.tables.rfCalValues.hdet_dcoc_ib_scal_en            = (tANI_U8)pMac->hphy.nvCache.tables.qFuseData.dword3_rf_hdet_dcoc_ib_scal_en;
    }
    else
    {
        //QFuse data doesn't exist - populate with default data
        memcpy(&pMac->hphy.nvCache.tables.tpcConfig[0], &nvDefaults.tables.tpcConfig[0], sizeof(tTpcConfig) * MAX_TPC_CHANNELS);
        memcpy(&pMac->hphy.nvCache.tables.rfCalValues, &nvDefaults.tables.rfCalValues, sizeof(sRfNvCalValues));
    }
}
#endif //#if 0

eHalStatus halQFuseWrite(tHalHandle hMac)
{
    return eHAL_STATUS_SUCCESS;
}

eHalStatus halQFuseRead(tHalHandle hMac)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tpAniSirGlobal pMac = (tpAniSirGlobal)hMac;

    //Dont take QFuse values instead use defaults
    pMac->hphy.nvCache.tables.qFuseData.dword3_sw_fuse_pgm_dsbl = QFUSE_NOT_BLOWN;

    return status;
}

eHalStatus halIsQFuseBlown(tHalHandle hMac)
{
	return (eHAL_STATUS_FAILURE);     //no, unblown
}


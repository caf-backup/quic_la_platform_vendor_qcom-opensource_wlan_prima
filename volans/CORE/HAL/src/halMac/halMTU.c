/**
 *
 *  @file:         haMtu.c
 *
 *  @brief:        Provides all the MAC APIs to the MTU Hardware Block.
 *
 *  @author:       Susan Tsao
 *
 *  Copyright (C) 2002 - 2007, Qualcomm, Inc. All rights reserved.
 *
 *  Change History:
 * ---------------------------------------
 * 09/09/2007  File created.
 * 11/27/2007  Virgo related changes.
 */
#include "palTypes.h"
#include "halMTU.h"
#include "halDebug.h"
#include "halMCU.h"
#include "halUtils.h"
#include "halTpe.h"
#include "halAdu.h"

#if defined(ANI_OS_TYPE_LINUX)
#include "rtaiWrapper.h"
    void rt_timer_tick(struct rtLibApp * rt);
#endif

#define MTU_LONG_RETRY_OFFSET     4
#define MTU_BKID_0_RETRY_OFFSET   0
#define MTU_BKID_1_RETRY_OFFSET   8
#define MTU_BKID_2_RETRY_OFFSET   16
#define MTU_BKID_3_RETRY_OFFSET   24
#define MTU_BKID_4_RETRY_OFFSET   0
#define MTU_BKID_5_RETRY_OFFSET   8
#define MTU_BKID_6_RETRY_OFFSET   16
#define MTU_BKID_7_RETRY_OFFSET   24

#ifdef WLAN_HAL_VOLANS

#ifdef VOLANS_FPGA // With Volans, not sure if on FPGA link we need to make different value other than 39 to talk with Virgo AP. TBD.
#define LIBRA_MTU_CLOCK_PER_USEC      39
#else
#define LIBRA_MTU_CLOCK_PER_USEC      39
#endif

#else

#ifdef LIBRA_FPGA // On FPGA set the clk/usec to 0x4A, on real chip, set to 79
#define LIBRA_MTU_CLOCK_PER_USEC      0x4a
#else
#define LIBRA_MTU_CLOCK_PER_USEC      79
#endif

#endif

#ifdef WLAN_HAL_VOLANS
#undef LIBRA_MTU_TIMING_WORKAROUND
#else
#define LIBRA_MTU_TIMING_WORKAROUND
#endif
// In Libra and Virgo Hard MAC implementation, OFDM signal extension got extended 5 more usec, which 
// makes the pktdet_n signal delayed by 11usec. In order to compensate the delay, SIFS has to change so
// ACK/BA preamble could go over the air exactly at SIFS boundary. 
// For BA response, hard MAC has a separate control when to send the BA bitmap to TPE and the default was 14usec. 
// But due to the (incorrect) signal extension, 11+14=25usec and that always triggers ACK timour in short slot 
// time case. Now reduce the value from 14->1usec.
// - HW team doesn't plan to fix this in Libra chip. Add a new #define in case in Gen6.2 we could remove this 
// workaround

#ifdef LIBRA_MTU_TIMING_WORKAROUND
#define LIBRA_MTU_2_4G_WRKARND_SIFS_USEC 4
#define LIBRA_MTU_2_4G_PKTDET_TO_BABITMAP_UPDATE_USEC   1
/*
  In our test, it shows that the turnaround time from ACK to Libra until Libra sends out
  the next frame, there is a small delays. The delay varies (~13us at 6mbps, ~4us at 36mbps).
  At this moment HW team does not understand the exact reason for this. Guido suggested to 
  reduce the DIFS by 13us as workaround. It fails some WIFI WMM test, hence reduce by 4us.
*/
#define LIBRA_MTU_2_4G_WRKARND_TOTAL_DIFS_REDUCTION (4)
#else
  #define LIBRA_MTU_2_4G_PKTDET_TO_BABITMAP_UPDATE_USEC   14
  #define LIBRA_MTU_2_4G_WRKARND_TOTAL_DIFS_REDUCTION 0
#endif
#define LIBRA_MTU_2_4G_SIFS_USEC   10


/**
 *  Following values from WMM spec:
 *          STA-CWmin       STA-CWmax          STA-AIFSN   AP-AIFSN   AP-CWmin        AP-CWmax
 *  =================================================================================================
 *  AC_BK   aCWmin          aCWmax             7            7         aCWmin          aCWmax
 *  AC_BE   aCWmin          aCWmax             3            3         aCWmin          4*(aCWmin+1)-1
 *  AC_VI   (aCWmin+1)/2-1  aCWmax             2            1         (aCWmin+1)/2-1  aCWmax
 *  AC_VO   (aCWmin+1)/4-1  (aCWmax+1)/2-1     2            1         (aCWmin+1)/4-1  (aCWmax+1)/2-1
 *
 *  11b CWmin,max = 31 ... 1023
 *  ERP             15/31 ... 1023
 *
 *  AIFS[AC] = AIFSN[AC]x slotTime + SIFS
 */

/**
 * 
 */

//Dinesh : need to change the backoff parametes of backoff 3 which is mapped for probeRsp.
static tMtuParams sta_params[MAX_NUMBER_OF_MODES] =
{
  /** backoff id 0 -> 7 */
  { {43, 43, 34, 34, 34, 34, 79, 43},        /** 11a DIFS  */
    {4, 4, 4,  4,  2,  3,  4, 4},            /** 11a cwMin */
    {10, 10, 10, 10, 3, 4, 10, 10},            /** 11a cwMax */
    90,                                      /** 11a EIFS  */
    25,                                      /** 11a PIFS  */
    9,                                       /** 11a SLOT  */
    32,                                      /** 11a cca_miss_limit FIXME : should be 25*/
    16,                                      /** 11a SIFS  */
    LIBRA_MTU_CLOCK_PER_USEC,                /** 11a one_usec_limit */
    9,                                       /** 11a bcn_slot_limit */
    2                                        /** 11a RIFS  */
  },


  /** backoff id 0 -> 7 */
  { {70, 70, 50, 50, 50, 50, 150, 70},       /** 11b DIFS  */
    {5, 5, 5,  5,  3,  4, 5, 5},             /** 11b cwMin */
    {10, 10, 10, 10, 4, 5, 10, 10},            /** 11b cwMax */
    364,                                     /** 11b EIFS  */
    30,                                      /** 11b PIFS  */
    20,                                      /** 11b SLOT  */
    32,                                      /** 11b cca_miss_limit FIXME : should be 30*/
    LIBRA_MTU_2_4G_SIFS_USEC,                /** 11b SIFS  */
    LIBRA_MTU_CLOCK_PER_USEC,                /** 11b one_usec_limit */
    20,                                      /** 11b bcn_slot_limit */
    0                                        /** 11b N/A  */
  },

  /** backoff id 0 -> 7 */
  { {70, 70, 50, 50, 50, 50, 150, 70},       /** 11g Mixed Mode DIFS  */
    {5, 5, 4,  4,  3,  4, 5, 5},             /** 11g Mixed Mode cwMin */
    {10, 10, 10, 10, 4, 5, 10, 10},            /** 11g Mixed Mode cwMax */
    90,                                      /** 11g Mixed Mode EIFS  */
    30,                                      /** 11g Mixed Mode PIFS  */
    20,                                      /** 11g Mixed Mode SLOT  */
    32,                                      /** 11g Mixed Mode cca_miss_limit FIXME : should be 30*/
    LIBRA_MTU_2_4G_SIFS_USEC,                /** 11g Mixed Mode SIFS  */
    LIBRA_MTU_CLOCK_PER_USEC,                /** 11g Mixed Mode one_usec_limit */
    20,                                      /** 11g Mixed Mode bcn_slot_limit */
    2                                        /** 11g Mixed Mode RIFS  */
  },

  /** backoff id 0 -> 7 */
  { {37, 37, 28, 28, 28, 28, 73, 37},        /** 11g Pure Mode DIFS  */
    {4, 4, 4,  4,  2,  3,  4, 4},            /** 11g Pure Mode cwMin */
    {10, 10, 10, 10, 3, 4, 10, 10},          /** 11g Pure Mode cwMax */
    90,                                      /** 11g Pure Mode EIFS  */
    19,                                      /** 11g Pure Mode PIFS  */
    9,                                       /** 11g Pure Mode SLOT  */
    32,                                      /** 11g Pure Mode cca_miss_limit FIXME : should be 19*/
    LIBRA_MTU_2_4G_SIFS_USEC,                /** 11g Pure Mode SIFS  */
    LIBRA_MTU_CLOCK_PER_USEC,                /** 11g Pure Mode one_usec_limit */
    20,                                      /** 11g Pure Mode bcn_slot_limit */
    2                                        /** 11g Pure Mode RIFS  */
  }
};

#ifdef ANI_PRODUCT_TYPE_AP

/** @todo TODO: Fix these AP parameters based on AIFS values in the standard */
static tMtuParams ap_params[MAX_NUMBER_OF_MODES] =
{
  /** BCN RSVD MHI MLO VO VI  BK  BE */
  { {34, 34, 25, 25, 25, 25, 79, 43},        /** 11a DIFS  */
    {4, 4, 2,  2,  2,  3,  4, 4},            /** 11a cwMin */
    {10, 10, 3, 3, 3, 4, 10, 6},             /** 11a cwMax */
    90,                                      /** 11a EIFS  */
    25,                                      /** 11a PIFS  */
    9,                                       /** 11a SLOT  */
    32,                                      /** 11a cca_miss_limit FIXME : should be 25*/
    16,                                      /** 11a SIFS  */
    LIBRA_MTU_CLOCK_PER_USEC,                /** 11a one_usec_limit */
    9,                                       /** 11a bcn_slot_limit */
    2                                        /** 11a RIFS  */
  },


  /** BCN RSVD MHI MLO VO VI  BK  BE */
  { {50, 50, 30, 30, 30, 30, 150, 70},       /** 11b DIFS  */
    {5, 5, 3,  3,  3,  4, 5, 5},             /** 11b cwMin */
    {10, 10, 4, 4, 4, 5, 10, 7},             /** 11b cwMax */
    364,                                     /** 11b EIFS  */
    30,                                      /** 11b PIFS  */
    20,                                      /** 11b SLOT  */
    32,                                      /** 11b cca_miss_limit FIXME : should be 30*/
    LIBRA_MTU_2_4G_SIFS_USEC,                                      /** 11b SIFS  */
    LIBRA_MTU_CLOCK_PER_USEC,                /** 11b one_usec_limit */
    20,                                      /** 11b bcn_slot_limit */
    0                                        /** 11b N/A  */
  },

  /** BCN RSVD MHI MLO VO VI  BK  BE */
  { {50, 50, 30, 30, 30, 30, 150, 70},       /** 11g Mixed Mode DIFS  */
    {5, 5, 3,  3,  3,  4, 5, 5},             /** 11g Mixed Mode cwMin */
    {10, 10, 4, 4, 4, 5, 10, 7},             /** 11g Mixed Mode cwMax */
    90,                                      /** 11g Mixed Mode EIFS  */
    30,                                      /** 11g Mixed Mode PIFS  */
    20,                                      /** 11g Mixed Mode SLOT  */
    32,                                      /** 11g Mixed Mode cca_miss_limit FIXME : should be 30*/
    LIBRA_MTU_2_4G_SIFS_USEC,                                      /** 11g Mixed Mode SIFS  */
    LIBRA_MTU_CLOCK_PER_USEC,                /** 11g Mixed Mode one_usec_limit */
    20,                                      /** 11g Mixed Mode bcn_slot_limit */
    2                                        /** 11g Mixed Mode RIFS  */
  },

  /** BCN RSVD MHI MLO VO VI  BK  BE */
  { {28, 28, 19, 19, 19, 19, 73, 37},        /** 11g Pure Mode DIFS  */
    {4, 4, 2,  2,  2,  3,  4, 4},            /** 11g Pure Mode cwMin */
    {10, 10, 3, 3, 3, 4, 10, 6},             /** 11g Pure Mode cwMax */
    90,                                      /** 11g Pure Mode EIFS  */
    19,                                      /** 11g Pure Mode PIFS  */
    9,                                       /** 11g Pure Mode SLOT  */
    32,                                      /** 11g Pure Mode cca_miss_limit FIXME : should be 19*/
    LIBRA_MTU_2_4G_SIFS_USEC,                                      /** 11g Pure Mode SIFS  */
    LIBRA_MTU_CLOCK_PER_USEC,                /** 11g Pure Mode one_usec_limit */
    20,                                      /** 11g Pure Mode bcn_slot_limit */
    2                                        /** 11g Pure Mode RIFS  */
  }
};

#endif

typedef struct sAcToBkoffRegIndex {
    tANI_U32 ac;
    tMtuBkId regIndex;
} tAcToBkoffRegIndex;

/** AC to MTU backoff engine index @fixme : check MTU index*/
static tAcToBkoffRegIndex regIndex[] = {
    {EDCA_AC_BE, MTU_BKID_AC_BE},
    {EDCA_AC_BK, MTU_BKID_AC_BK},
    {EDCA_AC_VI, MTU_BKID_AC_VI},
    {EDCA_AC_VO, MTU_BKID_AC_VO}
};


/** determine the backoff regiser index based on the supplied AC */
tANI_U32 __halMTU_ac2BkoffIndex(tpAniSirGlobal pMac, tANI_U32 ac)
{
    tANI_U32           i;
    tAcToBkoffRegIndex *p = &regIndex[0];
    for (i = 0; i < sizeof(regIndex)/sizeof(regIndex[0]); i++, p++)
        if (p->ac == ac)
            return p->regIndex;
    HALLOGW( halLog(pMac, LOGW, FL("Invalid ac (%d)\n"), ac));
    return ((tANI_U32) MTU_BKID_AC_VO);
}

/**
 * \fn:   __halMTU_Reset
 *
 * \brief:  Resets MTU.
 *
 * \fixme:   Do we need this function in GEN5.
 *
 * \param:  tpAniSirGlobal pMac : Handle to Mac Structure.
 *
 * \return: eHalStatus
 */
static eHalStatus __halMTU_Reset(tpAniSirGlobal pMac)
{
    if( eHAL_STATUS_SUCCESS != halMcu_ResetModules(pMac, QWLAN_MCU_SOFT_RESET_MTU_SOFT_RESET_MASK))
        return eHAL_STATUS_FAILURE;
    return eHAL_STATUS_SUCCESS;
}




#if defined(ANI_PRODUCT_TYPE_CLIENT)
#ifdef FIXME_GEN5
/**
 *  \fn    : __halMTU_HandleATHInterrupt
 *
 *  \brief : This function - handles timer interrupt which is propgrammed for SYS_TICK_TO_MICRO_SECOND
 *
 *  \param : pMac      hal handle and interrupt source.
 *
 *  \return: eHalStatus
 */
static eHalStatus __halMTU_HandleATHInterrupt( tHalHandle hHal, eHalIntSources timerIntr )
{
    tpAniSirGlobal  pMac = PMAC_STRUCT( hHal );
    tSirMsgQ        msgQ;
    tSirRetStatus   status;

    msgQ.type = SIR_HAL_TIMER_ADJUST_ADAPTIVE_THRESHOLD_IND;
    msgQ.reserved = 0;
    msgQ.bodyval = 0;
    msgQ.bodyptr = NULL;

    status = halPostMsgApi( pMac, &msgQ );
    if( status != eSIR_SUCCESS )
        HALLOGE( halLog(pMac, LOGE, FL("Post SIR_HAL_ADJUST_ADAPTIVE_THRESHOLD_IND failed \n")));

    return status;
}

/**
 *  \fn     : __halMTU_RegisterATHInterrupt
 *
 *  \brief  : This function - registers for Adaptive Threshold interrupt which is used in Station.
 *
 *  \param  : pMac      hal Handle and interrupt source.
 *
 *  \return : eHalStatus
 */
static eHalStatus __halMTU_RegisterATHInterrupt( tHalHandle hHal , eHalIntSources timerIntr)
{
    tANI_U32 regValOld = 0;
    tpAniSirGlobal pMac = (tpAniSirGlobal) hHal;
    
    /** Enroll Adaptive Threshold INTR handlers. */
    if( eHAL_STATUS_SUCCESS !=
      halIntEnrollHandler( ADAPTIVE_THRESHOLD_TIMER_MCU_SYS_GROUPED, &__halMTU_HandleATHInterrupt))
        HALLOGW( halLog( pMac, LOGW,
              FL("Unable to enroll an INTR handler \n")));

    /** Enable Adaptive Threshold timer group at MCU level. */
    if( eHAL_STATUS_SUCCESS != halIntEnable( hHal, ADAPTIVE_THRESHOLD_TIMER_MCU_SYS_GROUPED ))
        HALLOGW( halLog( pMac, LOGW,
              FL("Failed to ENABLE the  INTR!\n")));

    /** Enable PIF top-level INTR... */
    if( eHAL_STATUS_SUCCESS != halIntEnable( hHal, ADAPTIVE_THRESHOLD_TIMER_PIF_GROUP ))
        HALLOGW( halLog( pMac, LOGW,
              FL("Failed to ENABLE the Adaptive Threshold Group INTR!\n")));

    /** reading the old register value*/
    halReadRegister(pMac,
        QWLAN_MTU_TIMER_CONTROL_REG,
        &regValOld );

    /** Modifying only the bits that we are interested in.
     *  first the two corresponding two bits are being set
     *  to zero and then they are being set to 0x10.
     */
    halWriteRegister(pMac,
      QWLAN_MTU_TIMER_CONTROL_REG,
      ((regValOld & (~(MTU_TIMER_CONTROL_UNIT_MASK(MTUTIMER_ATH) |
      MTU_TIMER_CONTROL_UP_N(MTUTIMER_ATH)))) |
      MTU_TIMER_CONTROL_UNIT_USEC(MTUTIMER_ATH)));
    
    /** Update the desired TIMER tick value for the next tick */
    halWriteRegister(pMac,
      ADAPTIVE_THRESHOLD_MTU_TIMER_REG,
      SYS_TICK_TO_MICRO_SECOND);
    
    return eHAL_STATUS_SUCCESS;
}
#endif
/**
 *  \fn     : halMTU_setAdaptThreshTimer
 *
 *  \brief  : This function updates the desired TIMER tick value for Adaptive Threshold
 *
 *  \param  : pMac      hal handle and interrupt source.
 *
 *  \return : eHalStatus
 */
eHalStatus halMTU_setAdaptThreshTimer( tpAniSirGlobal pMac )
{
#ifdef FIXME_GEN5
    tANI_U32        nextTimerTick;

    if ( !(pMac->hal.halAdaptThresh.sampleInterval) )
        nextTimerTick = SYS_TICK_TO_MICRO_SECOND;
    else
        nextTimerTick = pMac->hal.halAdaptThresh.sampleInterval;

    /** Update the desired TIMER tick value for the next tick */
    halWriteRegister(pMac, ADAPTIVE_THRESHOLD_MTU_TIMER_REG, nextTimerTick);
#endif    
    return eHAL_STATUS_SUCCESS;
}
#endif  // ANI_PRODUCT_TYPE_CLIENT

void halMtu_setBackOffControl(tpAniSirGlobal pMac)
{
    tANI_U32 value;

    halReadRegister(pMac, QWLAN_MTU_BKOF_CONTROL_REG, &value);

    value |= QWLAN_MTU_BKOF_CONTROL_SW_MTU_SUPPRESS_COLLISION_INT_MASK;
    
    /** Program backOff control register to suppress collission interrupts */
    halWriteRegister(pMac, QWLAN_MTU_BKOF_CONTROL_REG, 
                           value);

}


/* MTU stall backoff based on the mask */
void halMTU_stallBackoffs(tpAniSirGlobal pMac, tANI_U32 mask)
{
    tANI_U32 regValue = 0, count = 0;
    eHalStatus status = eHAL_STATUS_SUCCESS;

    // Stall data backoffs
    halReadRegister(pMac, QWLAN_MTU_BKOF_CONTROL_REG, &regValue);
    regValue = regValue | (mask  << QWLAN_MTU_BKOF_CONTROL_SW_MTU_STALL_BKOF_OFFSET);
    halWriteRegister(pMac, QWLAN_MTU_BKOF_CONTROL_REG, regValue);

    // Poll for TPE to be idle
    regValue = 0;
    do {
        status = halReadRegister(pMac, QWLAN_MCU_MCU_WMAC_STATUS_REG, &regValue);
        count++;
        if (count > MCU_REG_POLLING_WARNING || eHAL_STATUS_SUCCESS != status) {
             HALLOGP( halLog(pMac, LOGP, FL("Polled MCU_WMAC_STATUS_REG register for %d times !!!\n"), count));
             break;			 
        }
    } while ((regValue & QWLAN_MCU_MCU_WMAC_STATUS_TPE_MCU_STATUS_MASK));
}


/* MTU start backoff based on the mask */
void halMTU_startBackoffs(tpAniSirGlobal pMac, tANI_U32 mask)
{
    tANI_U32 regValue = 0;
    halReadRegister(pMac, QWLAN_MTU_BKOF_CONTROL_REG, &regValue);
    regValue = regValue & (~(mask  << QWLAN_MTU_BKOF_CONTROL_SW_MTU_STALL_BKOF_OFFSET));
    halWriteRegister(pMac, QWLAN_MTU_BKOF_CONTROL_REG, regValue);
}

/**
 * \fn:   halMTU_Start
 *
 * \brief:  Resets MTU and initializes the MTU timing params.
 *
 * \param:  tHalHandle hHal : Handle to Mac Structure.
 *
 * \param:  void *arg : Mac Start parameter.
 *
 * \return: eHalStarus
 */
eHalStatus halMTU_Start(tHalHandle hHal, void *arg)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal) hHal;
    tANI_U32 value = 0;
    (void) arg;

    /**
     *  Reset MTU, via the MCU REG
     *  Refer to Bugzilla 14078 for more details
     *  Changelist 189904 precedes this change
     */
    if(__halMTU_Reset(pMac)!= eHAL_STATUS_SUCCESS)
        return eHAL_STATUS_FAILURE;

    halMTU_initTimingParams(pMac, MODE_11G_PURE);
    
    /** Program the BackOff Controls */
    halMtu_setBackOffControl(pMac);

    // SW response to consider vector = 0x7 to avoid a possible hang in TPE
    // when trying to stall the MTU backoff engines during scan.
    halReadRegister(pMac, QWLAN_MTU_MTU_FOR_HMAC_CONTROLS_REG, &value);
    value |= QWLAN_MTU_MTU_FOR_HMAC_CONTROLS_SW_RSP_TO_CONSIDER_VECTOR_MASK;
    halWriteRegister(pMac, QWLAN_MTU_MTU_FOR_HMAC_CONTROLS_REG, value);

    /** Register interrupt for adaptive threshold timer.*/
#if defined(ANI_PRODUCT_TYPE_CLIENT)
#ifdef FIXME_GEN5
    if(__halMTU_RegisterATHInterrupt(hHal, ADAPTIVE_THRESHOLD_TIMER_MCU_SYS_GROUPED) != eHAL_STATUS_SUCCESS)
        return eHAL_STATUS_FAILURE;
#endif
#endif    

    return eHAL_STATUS_SUCCESS;
}


/**
 * \fn     :   halMTU_setDifsRegisters
 *
 * \brief  :   Update the AIFS/DIFS in MTU hardware.
 *
 * \param  :   tpAniSirGlobal pMac : Handle to Mac Structure.
 *
 * \param  :   tMtuParams        :  mode parameters.
 *
 * \return :   void
 *
 * \note   :   AIFS = sifs + (aifsn * slot)
 */
void halMTU_setDifsRegisters(tpAniSirGlobal pMac, tMtuParams modeParams)
{
    tANI_U32   value;


    /** Setting DIFS which is nothing but AIFS*/
    value = (((modeParams.difs[0] - 
                LIBRA_MTU_2_4G_WRKARND_TOTAL_DIFS_REDUCTION ) << QWLAN_MTU_DIFS_LIMIT_0TO3_SW_MTU_DIFS_LIMIT_0_OFFSET) |
             ((modeParams.difs[1] - 
                LIBRA_MTU_2_4G_WRKARND_TOTAL_DIFS_REDUCTION ) << QWLAN_MTU_DIFS_LIMIT_0TO3_SW_MTU_DIFS_LIMIT_1_OFFSET) |
             ((modeParams.difs[2] - 
                LIBRA_MTU_2_4G_WRKARND_TOTAL_DIFS_REDUCTION ) << QWLAN_MTU_DIFS_LIMIT_0TO3_SW_MTU_DIFS_LIMIT_2_OFFSET) |
             ((modeParams.difs[3] - 
                LIBRA_MTU_2_4G_WRKARND_TOTAL_DIFS_REDUCTION ) << QWLAN_MTU_DIFS_LIMIT_0TO3_SW_MTU_DIFS_LIMIT_3_OFFSET));
    halWriteRegister(pMac, QWLAN_MTU_DIFS_LIMIT_0TO3_REG, value);

    HALLOGW(halLog(pMac, LOGW, FL("QWLAN_MTU_DIFS_LIMIT_0TO3_REG reg has new value = %x\n"), value));

    value = (((modeParams.difs[4] - 
                LIBRA_MTU_2_4G_WRKARND_TOTAL_DIFS_REDUCTION ) << QWLAN_MTU_DIFS_LIMIT_4TO7_SW_MTU_DIFS_LIMIT_4_OFFSET) |
             ((modeParams.difs[5] - 
                LIBRA_MTU_2_4G_WRKARND_TOTAL_DIFS_REDUCTION ) << QWLAN_MTU_DIFS_LIMIT_4TO7_SW_MTU_DIFS_LIMIT_5_OFFSET) |
             ((modeParams.difs[6] - 
                LIBRA_MTU_2_4G_WRKARND_TOTAL_DIFS_REDUCTION ) << QWLAN_MTU_DIFS_LIMIT_4TO7_SW_MTU_DIFS_LIMIT_6_OFFSET) |
             ((modeParams.difs[7] - 
                LIBRA_MTU_2_4G_WRKARND_TOTAL_DIFS_REDUCTION ) << QWLAN_MTU_DIFS_LIMIT_4TO7_SW_MTU_DIFS_LIMIT_7_OFFSET));
    halWriteRegister(pMac, QWLAN_MTU_DIFS_LIMIT_4TO7_REG, value) ;
    HALLOGW( halLog(pMac, LOGW, FL("QWLAN_MTU_DIFS_LIMIT_4TO7_REG reg has new value = %x\n"), value));

}


/**
 * \fn     :   halMTU_updateTimingParams
 *
 * \brief  :   Update the MTU timing parameter which are dependent on 802.11 MTU Mode.
 *               This function is called when we are changing from one MTU mode to another ignoring
 *               the current parameters.
 *
 * \param  :   tpAniSirGlobal pMac : Handle to Mac Structure.
 *
 * \param  :   tMtuMode mode: 802.11 Modes, 11a,b,mixed and pureg
 *
 * \return :   void
 */
void halMTU_updateTimingParams(tpAniSirGlobal pMac, tMtuMode mode)
{
    tANI_U32  value, idx;
    tMtuParams *modeParams;
#ifdef WLAN_DEBUG
#ifndef WLAN_MDM_CODE_REDUCTION_OPT
    tANI_U8  modeStr[][10]={ "11a", "11b", "mixed11g", "pure11g" };
#endif
#endif
    tANI_U8 sifs;

#ifdef ANI_PRODUCT_TYPE_AP
    if(pMac->hal.halSystemRole == eSYSTEM_AP_ROLE )
        modeParams = &ap_params[mode];
    else
#endif
        modeParams = &sta_params[mode];
        /** Setting DIFS */
        halMTU_setDifsRegisters(pMac, *modeParams);
        /** Setting EIFS, PIFS, SLOT */
        value = ( ((modeParams->eifs - 
                    LIBRA_MTU_2_4G_WRKARND_TOTAL_DIFS_REDUCTION ) << QWLAN_MTU_EIFS_PIFS_SLOT_LIMIT_SW_MTU_EIFS_LIMIT_OFFSET) |
                  ((modeParams->pifs - 
                    LIBRA_MTU_2_4G_WRKARND_TOTAL_DIFS_REDUCTION ) << QWLAN_MTU_EIFS_PIFS_SLOT_LIMIT_SW_PIFS_LIMIT_OFFSET) |
                  (modeParams->slot << QWLAN_MTU_EIFS_PIFS_SLOT_LIMIT_SW_MTU_SLOT_LIMIT_OFFSET) );
        halWriteRegister(pMac, QWLAN_MTU_EIFS_PIFS_SLOT_LIMIT_REG, value);

#ifdef LIBRA_MTU_TIMING_WORKAROUND
        if((mode == MODE_11B) || (mode == MODE_11G_MIXED) || (mode == MODE_11G_PURE))
        {
            sifs = LIBRA_MTU_2_4G_WRKARND_SIFS_USEC;
        }
        else
            sifs = modeParams->sifs;
        
#else    
        sifs = modeParams->sifs;
#endif
    
    
        /** Setting CCA_MISS, SIFS, ONE_USEC, BCN_SLOT */
        value = ( (modeParams->cca_miss_limit << QWLAN_MTU_SW_MTU_BCN_SLOT_USEC_SIFS_LIMIT_SW_MTU_EARLY_PKT_DET_MISS_LIMIT_OFFSET) |
#ifndef WLAN_HAL_VOLANS  //after Volans netlist 62, SIFS got moved to another register.
                  (sifs << QWLAN_MTU_SW_MTU_BCN_SLOT_USEC_SIFS_LIMIT_SW_MTU_SIFS_LIMIT_OFFSET) |
#endif
                  (modeParams->one_usec_limit << QWLAN_MTU_SW_MTU_BCN_SLOT_USEC_SIFS_LIMIT_SW_MTU_ONE_USEC_LIMIT_OFFSET) |
                   modeParams->bcn_slot_limit);
        halWriteRegister(pMac, QWLAN_MTU_SW_MTU_BCN_SLOT_USEC_SIFS_LIMIT_REG, value);
    
        /** Setting CWMin, CWMax */
    
        for(idx=0; idx < 8; idx++)
        {
            tANI_U16 cwMin, cwMax;
            cwMin = (1 << modeParams->cwMin[idx]) - 1;
            cwMax = (1 << modeParams->cwMax[idx]) - 1;
    
            value = ((cwMax << QWLAN_MTU_SW_CW_MIN_CW_MAX_0_SW_CW_MAX_0_OFFSET) | cwMin);
            halWriteRegister(pMac, QWLAN_MTU_SW_CW_MIN_CW_MAX_0_REG + sizeof(tANI_U32)*idx, value);
        }
    
        pMac->hal.halMac.lastMtuMode = mode;
        HALLOG1( halLog(pMac, LOG1, FL("MTU timing param switched from Mode %s to %s\n"), modeStr[pMac->hal.halMac.lastMtuMode], modeStr[mode]));
}

/**
 * \fn     :   halMTU_update11gSlotTimingParams
 *
 * \brief  :   Update the MTU timing parameter which are dependent on Slot time.
 *               This routine preserves the current EDCA parameters and modifies the new mode parameters
 *               with the current value taking into account of the new slot time.
 *               This function will be called when there is a slot time change in connected state.
 *
 * \param  :   tpAniSirGlobal pMac : Handle to Mac Structure.
 *
 * \param  :   tMtuMode mode: 802.11 Modes, 11a,b,mixed and pureg
 *
 * \return :   void
 */
void halMTU_update11gSlotTimingParams(tpAniSirGlobal pMac, tMtuMode mode)
{
    tMtuParams *modeParams;
    tANI_U8 ac;

#ifdef ANI_PRODUCT_TYPE_AP
    if(pMac->hal.halSystemRole == eSYSTEM_AP_ROLE )
       modeParams = &ap_params[mode];
    else
#endif
       modeParams = &sta_params[mode];
    for (ac=0;  ac < MAX_NUM_AC; ac++)
    {
        tANI_U32 bkoffId = __halMTU_ac2BkoffIndex(pMac, ac);
        
        /** Updating local host cwMin, cwMax.*/
        modeParams->cwMin[bkoffId] = pMac->hal.edcaParam[ac].cw.min;
        modeParams->cwMax[bkoffId] = pMac->hal.edcaParam[ac].cw.max;
        modeParams->difs[bkoffId] = modeParams->sifs + (pMac->hal.edcaParam[ac].aci.aifsn * modeParams->slot);
    }
    halMTU_updateTimingParams(pMac, mode);
}

/**
 * \fn     :   halMTU_initTimingParams
 *
 * \brief  :   Initialialize the MTU timing parameter for SIFS, DIFS, SLOT time.
 *
 * \param  :   tpAniSirGlobal pMac : Handle to Mac Structure.
 *
 * \param  :   tMtuMode mode: 802.11 Modes, 11a,b,mixed and pureg
 *
 * \return :   eHalStatus
 */
void halMTU_initTimingParams(tpAniSirGlobal pMac, tMtuMode mode)
{
    tANI_U32  value;
    halMTU_updateTimingParams(pMac, mode);

    // Set IBSS BCN CW limit
    halReadRegister(pMac, QWLAN_MTU_VALID_BSSID_BITMAP_REG, &value);
    value = (value & 0xFFFF) | (HAL_IBSS_CW_LIMIT << QWLAN_MTU_VALID_BSSID_BITMAP_IBSS_BCN_CW_LIMIT_OFFSET);
    halWriteRegister(pMac, QWLAN_MTU_VALID_BSSID_BITMAP_REG, value);

    //set MTU early interrupt limit clk count    
    halReadRegister(pMac, QWLAN_MTU_EARLY_INTERRUPT_LIMITS_REG, &value);
    value &= ~QWLAN_MTU_EARLY_INTERRUPT_LIMITS_SW_MTU_EARLY_SW_INT_LIMIT_CLKS_MASK;
    value |= ((LIBRA_MTU_CLOCK_PER_USEC -1) << QWLAN_MTU_EARLY_INTERRUPT_LIMITS_SW_MTU_EARLY_SW_INT_LIMIT_CLKS_OFFSET );
    halWriteRegister(pMac, QWLAN_MTU_EARLY_INTERRUPT_LIMITS_REG, value);

#ifdef LIBRA_MTU_TIMING_WORKAROUND
    //For BA response, in 2.4G, pktdet_n got extended by RxP for 11usec (not 6usec), therefore
    //in order to meet SIFS timing, this value has to be changed. HW suggest the value be 1usec.
    halWriteRegister(pMac, QWLAN_MTU_BKOF_CONTROL2_REG, 
        LIBRA_MTU_2_4G_PKTDET_TO_BABITMAP_UPDATE_USEC << QWLAN_MTU_BKOF_CONTROL2_SW_MTU_PKT_DET_TO_CNT_THR_OFFSET);
#endif

}

/**
 * \fn     :   halMTU_updateCW
 *
 * \brief  :   Update the MTU CW parameter.
 *
 * \param  :   tpAniSirGlobal pMac : Handle to Mac Structure.
 *
 * \param  :   tANI_U8 ac: Access Category
 *
 * \param  :   tANI_U16 ecwMin: Min CW Parameter.
 *
 * \param  :   tANI_U16 ecwMax: Max CW Parameter.
 *
 * \return :   eHalStatus
 */
void halMTU_updateCW(tpAniSirGlobal pMac, tANI_U8 ac, tANI_U16 ecwMin, tANI_U16 ecwMax)
{
    tANI_U32 value, address;
    tANI_U16 cwMin, cwMax;
    tANI_U32 bkoffId;
    tMtuParams *modeParams;
    tMtuMode mode = halMTU_getMode(pMac);

    modeParams = &sta_params[mode];

    bkoffId = __halMTU_ac2BkoffIndex(pMac, ac);

    /** Updating local host cwMin, cwMax.*/
    modeParams->cwMin[bkoffId] = ecwMin;
    modeParams->cwMax[bkoffId] = ecwMax;
    cwMin = (1 << ecwMin) - 1;
    cwMax = (1 << ecwMax) - 1;

    /** Updating MTU Register Values.*/
    address = QWLAN_MTU_SW_CW_MIN_CW_MAX_0_REG + (bkoffId * 4);
    value = ((cwMax << QWLAN_MTU_SW_CW_MIN_CW_MAX_0_SW_CW_MAX_0_OFFSET) | cwMin);
    HALLOGW( halLog(pMac, LOGW, FL("CWMin[%d]=%d,  CWMax[%d]=%d \n"), bkoffId, cwMin, bkoffId, cwMax));

    halWriteRegister(pMac, address, value);
    HALLOGW(halLog(pMac, LOGW, FL("QWLAN_MTU_SW_CW_MIN_CW_MAX_0_REG for backoff %d reg has new value = %x\n"), bkoffId, value));

}

/**
 * \fn     :   halMTU_updateIbssCW
 *
 * \param  :   tpAniSirGlobal pMac : Handle to Mac Structure.
 * \cwValue:   cw value
 *
 */
void halMTU_updateIbssCW(tpAniSirGlobal pMac, tANI_U32 cwValue)
{
    tANI_U32 value=0;

    halReadRegister(pMac, QWLAN_MTU_VALID_BSSID_BITMAP_REG, 
        &value);
    value = 
        (value & QWLAN_MTU_VALID_BSSID_BITMAP_SW_MTU_VALID_BSSID_BITMAP_MASK) |
        (cwValue << QWLAN_MTU_VALID_BSSID_BITMAP_IBSS_BCN_CW_LIMIT_OFFSET);
    halWriteRegister(pMac, QWLAN_MTU_VALID_BSSID_BITMAP_REG, value);
}

/**
 * \fn     :   halMTU_updateAIFS
 *
 * \brief  :   Update the modeparameters in software and then set them at HW.
 *
 * \param  :   tpAniSirGlobal pMac : Handle to Mac Structure.
 *
 * \param  :   tSirMacEdcaParamRecord*        : pointer to edca parameters.
 *
 * \return :   void
 *
 * \note   :   AIFS = sifs + (aifsn * slot)
 */
void halMTU_updateAIFS(tpAniSirGlobal pMac, tSirMacEdcaParamRecord* pEdcaParam)
{
    tANI_U32   bkOffIdx, ac;
    tMtuParams *modeParams;
    tMtuMode mode = halMTU_getMode(pMac);

    modeParams = &sta_params[mode];


    for(ac = 0; ac < MAX_NUM_AC; ac++)
    {
        bkOffIdx = __halMTU_ac2BkoffIndex(pMac, ac);

        HALLOGW( halLog(pMac, LOGW, FL("AIFS[%d] = %d , sifs = %d, slot = %d\n"), bkOffIdx, pEdcaParam[ac].aci.aifsn, modeParams->sifs, modeParams->slot));
        HALLOGW( halLog(pMac, LOGW, FL("difs = %d\n"), modeParams->difs[bkOffIdx]));

        modeParams->difs[bkOffIdx] = modeParams->sifs + (pEdcaParam[ac].aci.aifsn * modeParams->slot);
    }

    halMTU_setDifsRegisters(pMac, *modeParams);
}

/**
 * \fn     :   halMTU_printMTUParams
 *
 * \brief  :   Print the MTU timing parameters.
 *
 * \param  :   tpAniSirGlobal pMac : Handle to Mac Structure.
 *
 * \param  :   tMtuMode mode: 802.11 Modes, 11a,b,mixed and pureg
 *
 * \return :   eHalStatus
 */
eHalStatus halMTU_printMTUParams(tpAniSirGlobal pMac, tMtuMode  mode)
{
    tMtuParams *pMtu;
#ifdef ANI_PRODUCT_TYPE_AP
    if(pMac->hal.halSystemRole == eSYSTEM_AP_ROLE )
       pMtu = &ap_params[mode];
    else
#endif
       pMtu = &sta_params[mode];


    HALLOGW( halLog(pMac, LOGW, FL("**** %s: Local Parameters **** \n"), (mode==MODE_11A) ? "11A" :
           (mode==MODE_11B) ? "11B" : (mode==MODE_11G_MIXED) ? "11G Mixed" : "11G PURE"));

    HALLOGW( halLog(pMac, LOGW, FL("DIFS:  %4d  %4d  %4d  %4d  %4d  %4d  %4d  %4d \n"),
           pMtu->difs[0], pMtu->difs[1], pMtu->difs[2], pMtu->difs[3],
           pMtu->difs[4], pMtu->difs[5], pMtu->difs[6], pMtu->difs[7]));

    HALLOGW( halLog(pMac, LOGW, FL("cwMin: %4d  %4d  %4d  %4d  %4d  %4d  %4d  %4d \n"),
           pMtu->cwMin[0], pMtu->cwMin[1],pMtu->cwMin[2],pMtu->cwMin[3],
           pMtu->cwMin[4], pMtu->cwMin[5],pMtu->cwMin[6],pMtu->cwMin[7]));

    HALLOGW( halLog(pMac, LOGW, FL("cwMax: %4d  %4d  %4d  %4d  %4d  %4d  %4d  %4d \n"),
           pMtu->cwMax[0], pMtu->cwMax[1],pMtu->cwMax[2],pMtu->cwMax[3],
           pMtu->cwMax[4], pMtu->cwMax[5],pMtu->cwMax[6],pMtu->cwMax[7]));

    HALLOGW( halLog(pMac, LOGW, FL("EIFS: %d,   PIFS: %d,   SLOT: %d \n"),
           pMtu->eifs, pMtu->pifs, pMtu->slot));

    HALLOGW( halLog(pMac, LOGW, FL("CCA miss limit: %d  SIFS: %d  1us_limit: %d  Beacon Slot Limit %d \n"),
           pMtu->cca_miss_limit, pMtu->sifs, pMtu->one_usec_limit, pMtu->bcn_slot_limit));

    return eHAL_STATUS_SUCCESS;
}


/**
 * \fn     :   halMTU_update11gSlotTime
 *
 * \brief  :   Update short slot time.
 *
 * \param  :   tpAniSirGlobal pMac   : Handle to Mac Structure.
 *
 * \param  :   tANI_U8 fShortSlotTime.
 *
 * \return :   eHalStatus
 */
void halMTU_update11gSlotTime(tpAniSirGlobal pMac, tANI_U8 fShortSlotTime)
{
    if(fShortSlotTime)
    {
        halMTU_update11gSlotTimingParams(pMac, MODE_11G_PURE);
    }
    else
    {
        halMTU_update11gSlotTimingParams(pMac, MODE_11G_MIXED);
    }
}

/**
 * \fn     :   halMTU_getMode
 *
 * \brief  :   This function - returns the MTU operating mode.
 *
 * \param  :   tpAniSirGlobal pMac : Handle to Mac Structure.
 *
 * \return :   eHalStatus
 */
tMtuMode halMTU_getMode(tpAniSirGlobal pMac)
{
    if(pMac->hal.currentRfBand == eRF_BAND_5_GHZ)
        return MODE_11A;
    else if(pMac->hal.nwType == eSIR_11B_NW_TYPE)
        return MODE_11B;
    else
    { //11G Mixed/Pure
        return pMac->hal.halMac.fShortSlot ? MODE_11G_PURE : MODE_11G_MIXED;
    }

}

/**
 * \fn     :   halMTU_getMode
 *
 * \brief  :   This function - returns the MTU operating mode.
 *
 * \param :   tpAniSirGlobal pMac : Handle to Mac Structure.
 *
 * \param :   tANI_U8 shortRetry
 * 
 * \param :   tANI_U8 longRetry
 *
 * \return :   eHalStatus
 */
void halMTU_updateRetryLimit(tpAniSirGlobal pMac, tANI_U8 shortRetry, tANI_U8 longRetry)
{
    tANI_U8  retry;
    tANI_U32 value;
    retry = (longRetry<<MTU_LONG_RETRY_OFFSET)|shortRetry;
    value = (retry<<MTU_BKID_0_RETRY_OFFSET) | (retry<<MTU_BKID_1_RETRY_OFFSET) |
            (retry<<MTU_BKID_2_RETRY_OFFSET) | (retry<<MTU_BKID_3_RETRY_OFFSET);

    halWriteRegister(pMac, QWLAN_MTU_LONG_SHORT_XMIT_LIMIT_BKOF_3_TO_0_REG,
                                 value);

    value = (retry<<MTU_BKID_4_RETRY_OFFSET) | (retry<<MTU_BKID_5_RETRY_OFFSET) |
            (retry<<MTU_BKID_6_RETRY_OFFSET) | (retry<<MTU_BKID_7_RETRY_OFFSET);

    halWriteRegister(pMac, QWLAN_MTU_LONG_SHORT_XMIT_LIMIT_BKOF_7_TO_4_REG,
                                 value);

}
/**
 * \fn     :   halMTU_getRIFS
 *
 * \brief  :   Get the current RIFS.
 *
 * \param  :   tpAniSirGlobal pMac : Handle to Mac Structure.
 *
 * \param  :   tMtuMode mode       : 802.11 Modes, 11a,b,mixed and pureg
 *
 * \param  :   tANI_U8 *rifs       : Pointer to fill in the RIFS value.
 *
 * \return :   eHalStatus
 */
eHalStatus halMTU_getRIFS(tpAniSirGlobal pMac, tMtuMode  mode, tANI_U8 *rifs)
{
    if((mode < MAX_NUMBER_OF_MODES) && rifs)
    {
        *rifs = sta_params[mode].rifs;
        return eHAL_STATUS_SUCCESS;
    }
    else
        return eHAL_STATUS_FAILURE;
}

/**
 * \fn     :   halMTU_UpdateValidBssid
 *
 * \brief  :   Update the Valid BSSID Bit.
 *
 * \param  :   tpAniSirGlobal pMac : Handle to Mac Structure.
 *
 * \param  :   tMtuMode mode       : 802.11 Modes, 11a,b,mixed and pureg
 *
 * \param  :   tHalBitVal bitOp    : Specifies SET or CLEAR operation.
 *
 * \return :   eHalStatus
 */
eHalStatus halMTU_UpdateValidBssid(tpAniSirGlobal pMac, tANI_U16 bssIdx, tHalBitVal bitOp)
{
    tANI_U32 value;

    if ((bssIdx > pMac->hal.memMap.maxBssids))
    {
        HALLOGW( halLog(pMac, LOGW, FL("Invalid beaconIndex specified\n")));
        return eHAL_STATUS_FAILURE;
    }

    halReadRegister(pMac, QWLAN_MTU_VALID_BSSID_BITMAP_REG,
                                    &value) ;

    if (bitOp == eHAL_SET)
    {
        value |= (1 << bssIdx);
    }
    else /** Clear the bit.*/
    {
        value &= ~(1 << bssIdx);
    }

    halWriteRegister(pMac, QWLAN_MTU_VALID_BSSID_BITMAP_REG,
                                value) ;
    return eHAL_STATUS_SUCCESS;

}

/**
 * \fn     :   halMTU_GetTsfTimer
 *
 * \brief  :   Get the current TSF timer.
 *
 * \param  :   tpAniSirGlobal pMac : Handle to Mac Structure.
 *
 * \param  :   tANI_U32 *tsfTimerLo: Pointer to fill in tsf low.
 *
 * \param  :   tANI_U32 *tsfTimerHi: Pointer to fill in tsf High.
 *
 * \return :   eHalStatus
 */
void halMTU_GetTsfTimer(tpAniSirGlobal pMac, tANI_U32 *tsfTimerLo, tANI_U32 *tsfTimerHi)
{

    /** Read the tsf timer value */
    halReadRegister(pMac, QWLAN_MTU_TSF_TIMER_LO_REG,
                                    tsfTimerLo);


    halReadRegister(pMac, QWLAN_MTU_TSF_TIMER_HI_REG,
                                    tsfTimerHi);

}

/**
 * \fn     :   halMTU_SetTsfTimer
 *
 * \brief  :   Code to write to the TSF timer lo and high 
 *
 * \param  :   tpAniSirGlobal pMac : Handle to Mac Structure.
 *
 * \return :   void
 */
void halMTU_SetTsfTimer(tpAniSirGlobal pMac, tANI_U32 tsfTimerLo, tANI_U32 tsfTimerHi)
{

    halWriteRegister(pMac, QWLAN_MTU_TSF_TIMER_LO_REG,
                                    tsfTimerLo);

    halWriteRegister(pMac, QWLAN_MTU_TSF_TIMER_HI_REG,
                                    tsfTimerHi);
    return;
}

/**
 * \fn     :   halMTU_SetTbttTimer
 *
 * \brief  :   Update the MTU TBTT HI and LOW timer value.
 *
 * \param  :   tpAniSirGlobal pMac : Handle to Mac Structure.
 *
 * \param  :   tANI_U32 tbttLo     : TBTT low value.
 *
 * \param  :   tANI_U32 tbttHi     : TBTT High value.
 *
 * \return :   VOID
 */
void halMTU_SetTbttTimer(tpAniSirGlobal pMac, tANI_U32 tbttLo, tANI_U32 tbttHi)
{
    halWriteRegister(pMac, QWLAN_MTU_TBTT_L_REG,
                                    tbttLo) ;

    halWriteRegister(pMac, QWLAN_MTU_TBTT_H_REG,
                                    tbttHi) ;

}

/**
 * \fn     :   halMTU_GetTbttTimer
 *
 * \brief  :   Get the TBTT timer value from MTU.
 *
 * \param  :   tpAniSirGlobal pMac : Handle to Mac Structure.
 *
 * \param  :   tANI_U32 *tbttLo    : Pointer to fill in TBTT low.
 *
 * \param  :   tANI_U32 *tbttHi    : Pointer to fill in TBTT High.
 *
 * \return :   VOID
 */
void halMTU_GetTbttTimer(tpAniSirGlobal pMac, tANI_U32 *tbttLo, tANI_U32 *tbttHi)
{

    halReadRegister(pMac, QWLAN_MTU_TBTT_L_REG,
                                    tbttLo);

    halReadRegister(pMac, QWLAN_MTU_TBTT_H_REG,
                                    tbttHi) ;
}

/**
 * \todo   :   Do we need to update this? this might be a internal register FIXME.
 *
 * \fn     :   halMTU_SetMtbttTimer
 *
 * \brief  :   Update the MTU M_TBTT timer value.
 *
 * \param  :   tpAniSirGlobal pMac : Handle to Mac Structure.
 *
 * \param  :   tANI_U32 mtbttLo    : M TBTT Low value.
 *
 * \param  :   tANI_U32 mtbttHi    : M TBTT High value.
 *
 * \return :   VOID
 */
void halMTU_SetMtbttTimer(tpAniSirGlobal pMac, tANI_U32 mtbttLo, tANI_U32 mtbttHi)
{
    halWriteRegister(pMac, QWLAN_MTU_M_TBTT_L_REG,
                                    mtbttLo) ;

    halWriteRegister(pMac, QWLAN_MTU_M_TBTT_H_REG,
                                    mtbttHi) ;

}

/**
 * \fn     :   halMTU_GetActiveBss
 *
 * \brief  :   Retreive the total number of Active BSS established.
 *
 * \param  :   tpAniSirGlobal pMac   : Handle to Mac Structure.
 *
 * \param  :   tANI_U8 *activeBssCnt : Total number of active
 *                                     BSS established.
 *
 * \return :   eHalStatus
 */
void halMTU_GetActiveBss(tpAniSirGlobal pMac, tANI_U8 *activeBssCnt)
{
    tANI_U32 value;
    tANI_U32 i = 0;
    tANI_U8     bssCnt = 0;

    /** Read the MTU valid bitmap and calculate the active BSS count */
    halReadRegister(pMac, QWLAN_MTU_VALID_BSSID_BITMAP_REG,
                    &value);

    while (i < 16)
    {
        if (value  & 1)
            bssCnt++;

        i++;
        value >>= 1;
    }

    *activeBssCnt = bssCnt;

}

/**
 * \fn     :   halMTU_SetIbssValid_And_BTAMPMode
 *
 * \brief  :   To enable IBSS mode or btAmp mode based on the btamp_flag.
 *
 * \param  :   tpAniSirGlobal pMac : Handle to Mac Structure.
 *
 * \return :   VOID
 *
 * For the BTAMP mode
 * We will need to set the BTAMP and the IBSS bit in MTU as per the HW programing instructions.
 * In the BTAMP mode the regular TSF timer will be used.
 *
 */
void halMTU_SetIbssValid_And_BTAMPMode(tpAniSirGlobal pMac, 
        tANI_U8 btamp_flag)
{
    tANI_U32 value;

    /** Read the MTU hmac control */
    halReadRegister(pMac, QWLAN_MTU_MTU_FOR_HMAC_CONTROLS_REG,
                    &value);

    value |= QWLAN_MTU_MTU_FOR_HMAC_CONTROLS_SW_MTU_IBSS_VALID_MASK;

    if (btamp_flag) 
        value |= QWLAN_MTU_MTU_FOR_HMAC_CONTROLS_SW_MTU_BTAMP_MODE_MASK;

    /** Set the ibss valid bit */
    halWriteRegister(pMac, QWLAN_MTU_MTU_FOR_HMAC_CONTROLS_REG,
                    value);

}


/**
 * \fn     :   halMTU_SetDtim
 *
 * \brief  :   Update the DTIM related settings and enables DTIM
 *
 * \param  :   tpAniSirGlobal pMac   : Handle to Mac Structure.
 *
 * \param  :   v_U32_t  dtimPeriod   : DTIM Period that needs to be updated.
 *
 * \param  :   v_U32_t  dtimThreshLimit : DTIM Threshold Limit that needs to be updated.
 *
 * \return :   None
 */
void halMTU_SetDtim(tpAniSirGlobal pMac, v_U32_t dtimPeriod,
                                              v_U32_t dtimThreshLimit)
{
    v_U32_t value = 0;

    //set DTIM period and threshold;
    halWriteRegister(pMac, QWLAN_MTU_MTU_DTIM_CNT_AND_PERIOD_REG, dtimPeriod << QWLAN_MTU_MTU_DTIM_CNT_AND_PERIOD_DTIM_PERIOD_OFFSET);

    halWriteRegister(pMac, QWLAN_MTU_DTIM_THRSH_CNT_AND_LIMIT_REG,
                         dtimThreshLimit << QWLAN_MTU_DTIM_THRSH_CNT_AND_LIMIT_DTIM_THRSH_LIMIT_OFFSET);

    /** Enable DTIM counter */
    halReadRegister(pMac, QWLAN_MTU_MTU_FOR_HMAC_CONTROLS_REG, &value);
    value |= QWLAN_MTU_MTU_FOR_HMAC_CONTROLS_SW_MTU_DTIM_CNT_EN_MASK;
    halWriteRegister(pMac, QWLAN_MTU_MTU_FOR_HMAC_CONTROLS_REG, value);
}


/**
 * \fn     :   halMTU_GetDtimCount
 *
 * \brief  :   Retrieve the current DTIM count.
 *
 * \param  :   tpAniSirGlobal pMac : Handle to Mac Structure.
 *
 * \param  :   tANI_U16 *dtimCnt   : Pointer to retrieve DTIM count.
 *
 * \return :   void
 */
void halMTU_GetDtimCount(tpAniSirGlobal pMac, tANI_U16 *dtimCnt)
{
    tANI_U32 value;

    /** Set the DTIM period */
    halReadRegister(pMac, QWLAN_MTU_MTU_DTIM_CNT_AND_PERIOD_REG,
                            &value);

    *dtimCnt = (tANI_U16)(value >> QWLAN_MTU_MTU_DTIM_CNT_AND_PERIOD_DTIM_CNT_OFFSET);

}

/**
 * \fn     :   halMTU_UpdateBeaconInterval
 *
 * \brief  :   Update the MTU Beacon Interval.
 *
 * \param  :   tpAniSirGlobal pMac     : Handle to Mac Structure.
 *
 * \param  :   tANI_U32 beaconInterval : Beacon Interval
 *
 * \return :   eHalStatus
 */
void halMTU_UpdateBeaconInterval(tpAniSirGlobal pMac, tANI_U32 beaconInterval)
{
    tANI_U32 value;

    halReadRegister(pMac, QWLAN_MTU_BCN_BSSID_INTV_REG,
                                    &value) ;

    // Clear the  Beacon Interval  before write new value.
    value =  value & (~(QWLAN_MTU_BCN_BSSID_INTV_SW_MTU_BEACON_INTV_MASK)); 


    /** Configure the beacon bssid register */
    value |= (QWLAN_MTU_BCN_BSSID_INTV_SW_MTU_BEACON_INTV_MASK & 
                  (beaconInterval << QWLAN_MTU_BCN_BSSID_INTV_SW_MTU_BEACON_INTV_OFFSET));

    halWriteRegister(pMac, QWLAN_MTU_BCN_BSSID_INTV_REG,
                                    value);

}

/**
 * \fn     :   halMTU_DisableBeaconTransmission
 *
 * \brief  :   Stop Beacon Transmission 
 *
 * \param  :   tpAniSirGlobal pMac     : Handle to Mac Structure.
 *
 * \return :   eHalStatus
 */
void halMTU_DisableBeaconTransmission(tpAniSirGlobal pMac)
{
    tANI_U32 value;

    palReadRegister(pMac->hHdd, QWLAN_MTU_BCN_BSSID_INTV_REG, &value);

    // Disable beacon transmission, is controlled by resetting this flag.
    value &= (~QWLAN_MTU_BCN_BSSID_INTV_SW_MTU_TBTT_ENABLE_MASK);

    palWriteRegister(pMac->hHdd, QWLAN_MTU_BCN_BSSID_INTV_REG, value);

    HALLOG1(halLog(pMac, LOG1, FL("\nMTU Beacon Disable\n")));

    return; 
}


// Set the bit in MTU sw_mtu_tbtt_enable to start beacon transmission
// This called when the beacon is ready from PE to be send out.
void halMTU_EnableDisableBssidTBTTBeaconTransmission(tpAniSirGlobal pMac, 
        tANI_U32 beaconInterval, tANI_U8 enable_flag)
{
    tANI_U32 value;

    palReadRegister(pMac->hHdd, QWLAN_MTU_BCN_BSSID_INTV_REG,
                                    &value);

    /** Configure the beacon bssid register */
    if (enable_flag)
    {
        value |= (QWLAN_MTU_BCN_BSSID_INTV_SW_MTU_TBTT_ENABLE_MASK |
                (1<<QWLAN_MTU_BCN_BSSID_INTV_SW_MTU_MAX_BSSIDS_OFFSET));
    }
    else 
    {
        /** Configure the beacon bssid register */
        value &= (~(QWLAN_MTU_BCN_BSSID_INTV_SW_MTU_TBTT_ENABLE_MASK));
    }

    palWriteRegister(pMac->hHdd, QWLAN_MTU_BCN_BSSID_INTV_REG,
                                    value);

}


/**
 * \fn     :   halMTU_UpdateNumBSS
 *
 * \brief  :   Update the number of BSS in MTU.
 *
 * \param  :   tpAniSirGlobal pMac     : Handle to Mac Structure.
 *
 * \param  :   tANI_U8 numBSS          : number of active Bssid
 *
 * \return :   void
 */
void halMTU_UpdateNumBSS(tpAniSirGlobal pMac, tANI_U8 numBSS)
{
    tANI_U32 value;

    halReadRegister(pMac, QWLAN_MTU_BCN_BSSID_INTV_REG,
                                    &value);

    // Clear the field 1st and then write the new value
    value &= (~QWLAN_MTU_BCN_BSSID_INTV_SW_MTU_MAX_BSSIDS_MASK);

    value |= (QWLAN_MTU_BCN_BSSID_INTV_SW_MTU_MAX_BSSIDS_MASK & (numBSS << QWLAN_MTU_BCN_BSSID_INTV_SW_MTU_MAX_BSSIDS_OFFSET));

    /** Configure mtu M BSS*/
    if (numBSS > 1)
         value |= QWLAN_MTU_BCN_BSSID_INTV_SW_MTU_MBSSID_ENABLE_MASK;
    else
        value &= ~QWLAN_MTU_BCN_BSSID_INTV_SW_MTU_MBSSID_ENABLE_MASK;

    halWriteRegister(pMac, QWLAN_MTU_BCN_BSSID_INTV_REG,
                                    value) ;


}

/**
 * \fn     :   halMTU_UpdateMbssInterval
 *
 * \brief  :   Update the MTU Multiple BSS Beacon interval.
 *
 * \param  :   tpAniSirGlobal pMac     : Handle to Mac Structure.
 *
 * \param  :   tANI_U8 numBSS          : number of active Bssid
 *
 * \return :   void
 */
void halMTU_UpdateMbssInterval(tpAniSirGlobal pMac, tANI_U32 mbssInterval)
{
    tANI_U32 value;

    halReadRegister(pMac, QWLAN_MTU_BCN_BSSID_INTV_REG,
                                    &value) ;

    /** Configure mtu bssid interval */
    value |= (QWLAN_MTU_BCN_BSSID_INTV_SW_MTU_BSSID_INTV_MASK & (mbssInterval << QWLAN_MTU_BCN_BSSID_INTV_SW_MTU_BSSID_INTV_OFFSET));

    halWriteRegister(pMac, QWLAN_MTU_BCN_BSSID_INTV_REG,
                                    value) ;


}

/**
 * \fn     :  halMTU_DeactivateTimer
 *
 * \brief  :   Deactivates timer.
 *
 * \param  :   tpAniSirGlobal pMac 
 *
 * \return :   eHalStatus
 */
eHalStatus halMTU_DeactivateTimer(tpAniSirGlobal pMac, tMtuTimer timer)
{
    tANI_U32 regVal;
    
    halReadRegister(pMac, QWLAN_MTU_TIMER_CONTROL_REG, &regVal );
    regVal &= ~(MTU_TIMER_CONTROL_UP_N(timer) | MTU_TIMER_CONTROL_UNIT_MASK(timer));
    halWriteRegister(pMac, QWLAN_MTU_TIMER_CONTROL_REG, regVal) ;
    // Causes a deadlock in Tx thread. This is cause the hal interrupt
    // cache is shared between interrupt context and main thread.
    // We get into an infinite ASIC interrupt sometimes with 
    // the timer 5 never being serviced.
    // The main thread disables the bit for timer 5 interrupt mask and the 
    // inerrupt context in the handler reads the updated mask to find
    // that we no longer service timer_5. In this state we get into a dead lock.
    // Tx hangs. Only the main and interrupt contexts get to run.
    //halIntDisable((tHalHandle)pMac, eHAL_INT_MCU_HOST_INT_MTU_TIMER_5);

    return eHAL_STATUS_SUCCESS;
}

#ifndef WLAN_SOFTAP_FEATURE
/**
 * \fn     :   __halMtu_UpdatePreBeaconTimer
 *
 * \brief  :   Update the MTU Pre Beacon timer.
 *
 * \param  :   tpAniSirGlobal pMac : Handle to Mac Structure.
 *
 * \return :   eHalStatus
 */
static eHalStatus __halMtu_UpdatePreBeaconTimer(tpAniSirGlobal pMac)

{
    tANI_U32 timerIntv;
    tANI_U32 tsfLo, tsfHi;
    tANI_U32 tbttLo, tbttHi;

    halMTU_GetTbttTimer(pMac, &tbttLo, &tbttHi);

    halMTU_GetTsfTimer(pMac, &tsfLo, &tsfHi);

    if ((tbttLo - tsfLo) < (PRE_BEACON_INTERVAL * ONE_TU))
        timerIntv = tbttLo + (pMac->hal.halMac.beaconInterval * ONE_TU) - (PRE_BEACON_INTERVAL * ONE_TU);
    else
        timerIntv = tbttLo - (PRE_BEACON_INTERVAL * ONE_TU);

    halWriteRegister(pMac, QWLAN_MTU_SW_MATCH_REGISTER_5_REG, timerIntv);

    return eHAL_STATUS_SUCCESS;
}

/**
 * \fn     :   halInitPreBeaconTmr
 *
 * \brief  :   Enables MTU Pre Beacon timer TIMER_5.
 *
 * \param  :   tpAniSirGlobal pMac : Handle to Mac Structure.
 *
 * \return :   eHalStatus
 */
eHalStatus
halInitPreBeaconTmr( tpAniSirGlobal pMac )
{
    tANI_U32   regVal;
    eHalStatus status =  eHAL_STATUS_SUCCESS;

    // Setup timer in up counter mode
    halReadRegister(pMac, QWLAN_MTU_TIMER_CONTROL_REG, &regVal );
    // clear timer_5 configurations
    regVal &= ~(MTU_TIMER_CONTROL_UP_N(MTUTIMER_BEACON_PRE) |
                MTU_TIMER_CONTROL_UNIT_MASK(MTUTIMER_BEACON_PRE));
    halWriteRegister(pMac, QWLAN_MTU_TIMER_CONTROL_REG, regVal);

    // set TIMER_5 for up count mode
    regVal |= (MTU_TIMER_CONTROL_UP_N(MTUTIMER_BEACON_PRE) |
               MTU_TIMER_CONTROL_UNIT_CLK(MTUTIMER_BEACON_PRE));
    halWriteRegister(pMac, QWLAN_MTU_TIMER_CONTROL_REG, regVal);

    // Load SW_MATCH_REGISTER_5
    if ( (status = __halMtu_UpdatePreBeaconTimer(pMac)) != eHAL_STATUS_SUCCESS)
    {
          HALLOGE( halLog(pMac, LOGE, FL("\n__halMtu_UpdatePreBeaconTimer returned error!\n")));
          return status;
    }

    // Enable TIMER_5 interrupt
    halIntEnable((tHalHandle)pMac, eHAL_INT_MCU_HOST_INT_MTU_TIMER_5); // this always returns success.
    return eHAL_STATUS_SUCCESS;
    
}

/**
 * \fn     :   halIntMtuHandlePreBeaconTmr
 *
 * \brief  :   Handles MTU Pre Beacon timer.
 *
 * \param  :   tpAniSirGlobal pMac : Handle to Mac Structure.
 *
 * \return :   eHalStatus
 */
eHalStatus halIntMtuHandlePreBeaconTmr( tHalHandle hHal, eHalIntSources tsfTimerIntr )
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hHal;
    tANI_U16       dialog_token = 0;
    tSirMsgQ       msg;

    msg.type        = SIR_HAL_BEACON_PRE_IND;
    msg.reserved    = dialog_token;
    msg.bodyval     = 0;
    msg.bodyptr     = NULL;

    HALLOG1( halLog(pMac, LOG1, FL("PreBeacon Intr detected \n")));

    /** Send Msg to PE to update the Beacon parameters */
    halPostMsgApi(pMac, &msg);

    if (__halMtu_UpdatePreBeaconTimer(pMac) != eHAL_STATUS_SUCCESS)
        return eHAL_STATUS_FAILURE;

    return eHAL_STATUS_SUCCESS;
}
#endif


/**
 * \fn     :   halMTU_ErrorHandler
 *
 * \brief  :   MTU Hardware module error interrupt handler.
 *
 * \param  :   tpAniSirGlobal pMac     : Handle to Mac Structure.
 *
 * \param  :   eHalIntSources intSource: Interrupt source
 *
 * \return :   eHalStatus
 */
eHalStatus halMTU_ErrorHandler(tHalHandle hHalHandle, eHalIntSources intSource)
{
    tANI_U32 intRegMask;
    tANI_U32 intRegStatus;
    HALLOGE( tpAniSirGlobal pMac = PMAC_STRUCT(hHalHandle));

    /** Read Interrupt Status.*/
    halIntGetErrorStatus(hHalHandle, intSource, &intRegStatus, &intRegMask);

    /** Display Error Status Information.*/
    HALLOGE( halLog( pMac, LOGE, FL("MTU Error Interrupt Status  : %x\n"),  intRegStatus ));
    HALLOGE( halLog( pMac, LOGE, FL("MTU Error Interrupt Mask/Enable  : %x\n"),  intRegMask ));

    HALLOGE( halLog( pMac, LOGE, FL("MTU Error Interrupt Received %d Times!!\n"), ++pMac->hal.halIntErrStats.halIntMtuErr ));

    return (eHAL_STATUS_SUCCESS);
}

eHalStatus halMTU_TimerInterrupt( tHalHandle hHalHandle, eHalIntSources intSource )
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tANI_U32 intRegMask;
    tANI_U32 intRegStatus;
    HALLOG1( tpAniSirGlobal pMac = PMAC_STRUCT(hHalHandle));

    /** Read Interrupt Status.*/
    status = halIntGetErrorStatus(hHalHandle, intSource, &intRegStatus, &intRegMask);
    if (status != eHAL_STATUS_SUCCESS) {
        return status;
    }

    intRegStatus &= intRegMask;   

    if(intRegStatus){
        /** Display Error Information.*/
        HALLOG1( halLog( pMac, LOG1, FL("MTU timer Interrupt Status %x, enable %x\n"),  intRegStatus, intRegMask ));
    } 
    return status;    
}

eHalStatus halMTU_DefInterruptHandler( tHalHandle hHalHandle, eHalIntSources intSource )
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tANI_U32 intRegMask;
    tANI_U32 intRegStatus;
    HALLOGE( tpAniSirGlobal pMac = PMAC_STRUCT(hHalHandle));

    /** Read Interrupt Status.*/
    status = halIntGetErrorStatus(hHalHandle, intSource, &intRegStatus, &intRegMask);
    if (status != eHAL_STATUS_SUCCESS) {
        return status;
    }

    intRegStatus &= intRegMask;   

    if(intRegStatus){
        /** Display Error Information.*/
        HALLOGE( halLog( pMac, LOGE, FL("MTU Interrupt Status %x, enable %x\n"),  intRegStatus, intRegMask ));
    } 
    return status;
    
}

void halGetTxTSFtimer(tpAniSirGlobal pMac, tSirMacTimeStamp *pTime)
{
    tANI_U32 low = 0;
    tANI_U32 high = 0;
    (void) halMTU_GetTsfTimer(pMac, &low, &high);
    *((tANI_U32 *)pTime) = low;
    *(((tANI_U32 *)pTime) + 1) = high;
}

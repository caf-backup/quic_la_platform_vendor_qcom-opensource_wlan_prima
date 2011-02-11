/** ------------------------------------------------------------------------- *
    ------------------------------------------------------------------------- *


    \file halQFuse.h

    \brief halQFuse services

    $Id$

    Copyright (C) 2008 Qualcomm, Incorporated


   ========================================================================== */

#ifndef HALQFUSE_H
#define HALQFUSE_H

#define NUM_DWORD 4

#define NUM_DWORD_BINARY_BITS           32
#define SCU_QFUSE_EF_BLOW_VALUE                   1
#define SCU_QFUSE_EF_STATUS                       1
#define SCU_QFUSE_EF_SHIFT_VALUE_CLEAR            0x80
#define SCU_QFUSE_EF_BLOW_VALUE_CLEAR             0
#define SCU_QFUSE_EF_BLOW_TIMER                   0xF

// -------------------------------------------------------------------
#define DWORD0_SW_FUSE_PGM_DSBL_MASK                     0x1
#define DWORD0_SW_FUSE_PGM_DSBL_OFFSET                   0x1F

#define DWORD0_RESERVED_MASK                             0x7FF
#define DWORD0_RESERVED_OFFSET                           0x14

#define DWORD0_GAINSHIFT_RELATIVE_MASK                   0xF
#define DWORD0_GAINSHIFT_RELATIVE_OFFSET                 0x10

#define DWORD0_CHAN13_CALPOINT_3_PWRDETADC_MASK          0x7F
#define DWORD0_CHAN13_CALPOINT_3_PWRDETADC_OFFSET        0x9

#define DWORD0_CHAN13_CALPOINT_3_ADJUSTEDPWRDET_MASK     0x7F
#define DWORD0_CHAN13_CALPOINT_3_ADJUSTEDPWRDET_OFFSET   0x2

#define DWORD0_CHAN13_CALPOINT_2_PWRDETADC_MASK          0x3
#define DWORD0_CHAN13_CALPOINT_2_PWRDETADC_OFFSET        0x0

// -------------------------------------------------------------------
#define DWORD1_CHAN13_CALPOINT_2_PWRDETADC_MASK          0x1F
#define DWORD1_CHAN13_CALPOINT_2_PWRDETADC_OFFSET        0x1B

#define DWORD1_CHAN13_CALPOINT_2_ADJUSTEDPWRDET_MASK     0x7F
#define DWORD1_CHAN13_CALPOINT_2_ADJUSTEDPWRDET_OFFSET   0x14

#define DWORD1_CHAN13_CALPOINT_1_PWRDETADC_MASK          0x7F
#define DWORD1_CHAN13_CALPOINT_1_PWRDETADC_OFFSET        0xD

#define DWORD1_CHAN13_CALPOINT_1_ADJUSTEDPWRDET_MASK     0x7F
#define DWORD1_CHAN13_CALPOINT_1_ADJUSTEDPWRDET_OFFSET   0x6

#define DWORD1_CHAN13_CALPOINT_0_PWRDETADC_MASK          0x3F
#define DWORD1_CHAN13_CALPOINT_0_PWRDETADC_OFFSET        0x0

// -------------------------------------------------------------------
#define DWORD2_CHAN13_CALPOINT_0_PWRDETADC_MASK          0x1
#define DWORD2_CHAN13_CALPOINT_0_PWRDETADC_OFFSET        0x1F

#define DWORD2_CHAN13_CALPOINT_0_ADJUSTEDPWRDET_MASK     0x7F
#define DWORD2_CHAN13_CALPOINT_0_ADJUSTEDPWRDET_OFFSET   0x18

#define DWORD2_CHAN1_CALPOINT_3_PWRDETADC_MASK           0x7F
#define DWORD2_CHAN1_CALPOINT_3_PWRDETADC_OFFSET         0x11

#define DWORD2_CHAN1_CALPOINT_3_ADJUSTEDPWRDET_MASK      0x7F
#define DWORD2_CHAN1_CALPOINT_3_ADJUSTEDPWRDET_OFFSET    0xA

#define DWORD2_CHAN1_CALPOINT_2_PWRDETADC_MASK           0x7F
#define DWORD2_CHAN1_CALPOINT_2_PWRDETADC_OFFSET         0x3

#define DWORD2_CHAN1_CALPOINT_2_ADJUSTEDPWRDET_MASK      0x7
#define DWORD2_CHAN1_CALPOINT_2_ADJUSTEDPWRDET_OFFSET    0x0

// -------------------------------------------------------------------

#define DWORD3_CHAN1_CALPOINT_2_ADJUSTEDPWRDET_MASK      0xF
#define DWORD3_CHAN1_CALPOINT_2_ADJUSTEDPWRDET_OFFSET    0x1C

#define DWORD3_CHAN1_CALPOINT_1_PWRDETADC_MASK           0x7F
#define DWORD3_CHAN1_CALPOINT_1_PWRDETADC_OFFSET         0x15

#define DWORD3_CHAN1_CALPOINT_1_ADJUSTEDPWRDET_MASK      0x7F
#define DWORD3_CHAN1_CALPOINT_1_ADJUSTEDPWRDET_OFFSET    0xE

#define DWORD3_CHAN1_CALPOINT_0_PWRDETADC_MASK           0x7F
#define DWORD3_CHAN1_CALPOINT_0_PWRDETADC_OFFSET         0x7

#define DWORD3_CHAN1_CALPOINT_0_ADJUSTEDPWRDET_MASK      0x7F
#define DWORD3_CHAN1_CALPOINT_0_ADJUSTEDPWRDET_OFFSET    0x0

// -------------------------------------------------------------------


///////////////////////////
typedef struct
{
    /*MSB [127:96]*/
#ifdef ANI_BIG_BYTE_ENDIAN

    /*software bit to tell whether the
    qfuse is programmed or not*/
    tANI_U32 dword3_sw_fuse_pgm_dsbl                       :1;

    tANI_U32 dword3_reserved                               :3;

    //RF settings that affect CLPC measurements
    tANI_U32 dword3_rf_hdet_ctl_ext_atten                  :4;
    tANI_U32 dword3_rf_hdet_dcoc_code                      :6;
    tANI_U32 dword3_rf_hdet_dcoc_ib_rcal_en                :1;
    tANI_U32 dword3_rf_hdet_dcoc_ib_scal_en                :1;

    /*channel 13: for cal point3,
    7bit PwrDetADC lut index and 7bit value*/
    tANI_U32 dword3_chan13_calpoint_3_pwrDetAdc            :7;
    tANI_U32 dword3_chan13_calpoint_3_adjustedPwrDet       :7;

    /*channel 13: for cal point2,
    2bit PwrDetADC lut index*/
    tANI_U32 dword3_chan13_calpoint_2_pwrDetAdc            :2;

#else
    tANI_U32 dword3_chan13_calpoint_2_pwrDetAdc            :2;
    tANI_U32 dword3_chan13_calpoint_3_adjustedPwrDet       :7;
    tANI_U32 dword3_chan13_calpoint_3_pwrDetAdc            :7;

    //RF settings that affect CLPC measurements
    tANI_U32 dword3_rf_hdet_dcoc_ib_scal_en                :1;
    tANI_U32 dword3_rf_hdet_dcoc_ib_rcal_en                :1;
    tANI_U32 dword3_rf_hdet_dcoc_code                      :6;
    tANI_U32 dword3_rf_hdet_ctl_ext_atten                  :4;

    tANI_U32 dword3_reserved                               :3;
    tANI_U32 dword3_sw_fuse_pgm_dsbl                       :1;
#endif

    /*[95:64]*/
#ifdef ANI_BIG_BYTE_ENDIAN

    /*channel 13: for cal point2,
    5bit PwrDetADC lut index and 7bit value*/
    tANI_U32 dword2_chan13_calpoint_2_pwrDetAdc            :5;
    tANI_U32 dword2_chan13_calpoint_2_adjustedPwrDet       :7;

    /*channel 13: for cal point1,
    7bit PwrDetADC lut index and 7bit value*/
    tANI_U32 dword2_chan13_calpoint_1_pwrDetAdc            :7;
    tANI_U32 dword2_chan13_calpoint_1_adjustedPwrDet       :7;

    /*channel 13: for cal point0,
    6bit PwrDetADC lut index*/
    tANI_U32 dword2_chan13_calpoint_0_pwrDetAdc            :6;

#else
    tANI_U32 dword2_chan13_calpoint_0_pwrDetAdc            :6;
    tANI_U32 dword2_chan13_calpoint_1_adjustedPwrDet       :7;
    tANI_U32 dword2_chan13_calpoint_1_pwrDetAdc            :7;
    tANI_U32 dword2_chan13_calpoint_2_adjustedPwrDet       :7;
    tANI_U32 dword2_chan13_calpoint_2_pwrDetAdc            :5;
#endif

    /*[63:32]*/
#ifdef ANI_BIG_BYTE_ENDIAN

    /*channel 13: for cal point0,
    1bit PwrDetADC lut index and 7bit value*/
    tANI_U32 dword1_chan13_calpoint_0_pwrDetAdc            :1;
    tANI_U32 dword1_chan13_calpoint_0_adjustedPwrDet       :7;

    /*channel 1:  for cal point3,
    7bit PwrDetADC lut index and 7bit value*/
    tANI_U32 dword1_chan1_calpoint_3_pwrDetAdc             :7;
    tANI_U32 dword1_chan1_calpoint_3_adjustedPwrDet        :7;

    /*channel 1:  for cal point2,
    7bit PwrDetADC lut index and 3bit value*/
    tANI_U32 dword1_chan1_calpoint_2_pwrDetAdc             :7;
    tANI_U32 dword1_chan1_calpoint_2_adjustedPwrDet        :3;

#else
    tANI_U32 dword1_chan1_calpoint_2_adjustedPwrDet        :3;
    tANI_U32 dword1_chan1_calpoint_2_pwrDetAdc             :7;
    tANI_U32 dword1_chan1_calpoint_3_adjustedPwrDet        :7;
    tANI_U32 dword1_chan1_calpoint_3_pwrDetAdc             :7;
    tANI_U32 dword1_chan13_calpoint_0_adjustedPwrDet       :7;
    tANI_U32 dword1_chan13_calpoint_0_pwrDetAdc            :1;
#endif

    /*LSB [31:0]*/
#ifdef ANI_BIG_BYTE_ENDIAN

    /*channel 1:  for cal point2,
    4bit lut value*/
    tANI_U32 dword0_chan1_calpoint_2_adjustedPwrDet        :4;

    /*channel 1:  for cal point1,
    7bit PwrDetADC lut index and 7bit value*/
    tANI_U32 dword0_chan1_calpoint_1_pwrDetAdc             :7;
    tANI_U32 dword0_chan1_calpoint_1_adjustedPwrDet        :7;

    /*channel 1:  for cal point0,
    7bit PwrDetADC lut index and 7bit value*/
    tANI_U32 dword0_chan1_calpoint_0_pwrDetAdc             :7;
    tANI_U32 dword0_chan1_calpoint_0_adjustedPwrDet        :7;

#else
    tANI_U32 dword0_chan1_calpoint_0_adjustedPwrDet        :7;
    tANI_U32 dword0_chan1_calpoint_0_pwrDetAdc             :7;
    tANI_U32 dword0_chan1_calpoint_1_adjustedPwrDet        :7;
    tANI_U32 dword0_chan1_calpoint_1_pwrDetAdc             :7;
    tANI_U32 dword0_chan1_calpoint_2_adjustedPwrDet        :4;
#endif

}sQFuseConfig;

///////////////////////////
#endif


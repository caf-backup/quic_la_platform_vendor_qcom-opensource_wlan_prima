
#ifndef PHYCALMEMORY_H
#define PHYCALMEMORY_H

#include <halPhyCfg.h>

typedef tTxLoCorrect tTxLoCorrections[NUM_RF_CHANNELS][NUM_TX_GAIN_STEPS];
typedef sIQCalValues sTxIQChannel[NUM_RF_CHANNELS][NUM_TX_GAIN_STEPS];
typedef sIQCalValues sRxIQChannel[NUM_RF_CHANNELS][NUM_TX_GAIN_STEPS];


typedef tANI_U8  tRuntime_pa_ctune_reg;
typedef tANI_U16 tRuntime_tx_bbf_gain_cnt            [NUM_TEMPERATURE_BINS][NUM_TX_GAIN_STEPS];
typedef tANI_U16 tRuntime_tx_bbf_lin_adj             [NUM_TEMPERATURE_BINS][NUM_TX_GAIN_STEPS];
typedef tANI_U16 tRuntime_lo_mix_da_gain_cntl        [NUM_TEMPERATURE_BINS][NUM_TX_GAIN_STEPS];
typedef tANI_U16 tRuntime_pa_gain_cntl               [NUM_TEMPERATURE_BINS][NUM_TX_GAIN_STEPS];
typedef tANI_U16 tRuntime_da_pa_bias_1_cnt           [NUM_TEMPERATURE_BINS][NUM_TX_GAIN_STEPS];
typedef tANI_U16 tRuntime_da_pa_bias_2_cntl          [NUM_TEMPERATURE_BINS][NUM_TX_GAIN_STEPS];
typedef tANI_U8  tRuntime_tcxo_reg_prg               [NUM_TEMPERATURE_BINS];
typedef tANI_U8  tRuntime_cp_gain                    [NUM_TEMPERATURE_BINS];
typedef tANI_U8  tRuntime_rxfe_lna_highgain_bias_ctl [NUM_TEMPERATURE_BINS];
typedef tANI_U8  tRuntime_rxfe_gm_ibias_core_ctrl    [NUM_TEMPERATURE_BINS];
typedef tANI_U8  tRuntime_rxfe_gm_bias_ctrl          [NUM_TEMPERATURE_BINS];
typedef tANI_U8  tRuntime_tia_ibias                  [NUM_TEMPERATURE_BINS];
typedef tANI_U8  tRuntime_im2_vcm_bq                 [NUM_TEMPERATURE_BINS];
typedef tANI_U8  tRuntime_im2_vgf                    [NUM_TEMPERATURE_BINS];
typedef tANI_U16 tRuntime_hdet_bias                  [NUM_TEMPERATURE_BINS];
typedef tANI_U16 tRuntime_hdet_test                  [NUM_TEMPERATURE_BINS];
typedef tANI_U16 tRuntime_hdet_ctl                   [NUM_TEMPERATURE_BINS];
typedef tANI_U16 tRuntime_pll_ac_reg5                [NUM_TEMPERATURE_BINS];
typedef tANI_U16 tRuntime_pll_ac_reg1                [NUM_TEMPERATURE_BINS];
typedef tANI_U16 tRuntime_pll_pc_reg3                [NUM_TEMPERATURE_BINS];
typedef tANI_U16 tRuntime_tx_lo_mix_da_adj           [NUM_TEMPERATURE_BINS];
typedef tANI_U16 tRuntime_pll_pc_reg2                [NUM_TEMPERATURE_BINS];
typedef tANI_U16 tRuntime_pll_reg6                   [NUM_TEMPERATURE_BINS];
typedef tANI_U16 tRuntime_pll_reg4                   [NUM_TEMPERATURE_BINS];
typedef tANI_U16 tRuntime_pll_vb_reg0                [NUM_TEMPERATURE_BINS];
typedef tANI_U16 tRuntime_pll_ac_reg0                [NUM_TEMPERATURE_BINS];
typedef tANI_U8  tRuntime_bbf_ctl3                   [NUM_TEMPERATURE_BINS];
typedef tANI_U8  tRuntime_bbf_ctl4                   [NUM_TEMPERATURE_BINS];
typedef tANI_U16 tRuntime_pll_ac_reg10               [NUM_TEMPERATURE_BINS];


typedef struct
{
    tRuntime_pa_ctune_reg               pa_ctune_reg;
    tANI_U8 reserved[3];    //padding
    tRuntime_tx_bbf_gain_cnt            tx_bbf_gain_cnt;
    tRuntime_tx_bbf_lin_adj             tx_bbf_lin_adj;
    tRuntime_lo_mix_da_gain_cntl        lo_mix_da_gain_cntl;
    tRuntime_pa_gain_cntl               pa_gain_cntl;
    tRuntime_da_pa_bias_1_cnt           da_pa_bias_1_cnt;
    tRuntime_da_pa_bias_2_cntl          da_pa_bias_2_cntl;
    tRuntime_tcxo_reg_prg               tcxo_reg_prg;
    tRuntime_cp_gain                    cp_gain;
    tRuntime_rxfe_lna_highgain_bias_ctl rxfe_lna_highgain_bias_ctl;
    tRuntime_rxfe_gm_ibias_core_ctrl    rxfe_gm_ibias_core_ctrl;
    tRuntime_rxfe_gm_bias_ctrl          rxfe_gm_bias_ctrl;
    tRuntime_tia_ibias                  tia_ibias;
    tRuntime_im2_vcm_bq                 im2_vcm_bq;
    tRuntime_im2_vgf                    im2_vgf;
    tRuntime_hdet_bias                  hdet_bias;
    tRuntime_hdet_test                  hdet_test;
    tRuntime_hdet_ctl                   hdet_ctl;
    tRuntime_pll_ac_reg5                pll_ac_reg5;
    tRuntime_pll_ac_reg1                pll_ac_reg1;
    tRuntime_pll_pc_reg3                pll_pc_reg3;
    tRuntime_tx_lo_mix_da_adj           tx_lo_mix_da_adj;
    tRuntime_pll_pc_reg2                pll_pc_reg2;
    tRuntime_pll_reg6                   pll_reg6;
    tRuntime_pll_reg4                   pll_reg4;
    tRuntime_pll_vb_reg0                pll_vb_reg0;
    tRuntime_pll_ac_reg0                pll_ac_reg0;
    tRuntime_bbf_ctl3                   bbf_ctl3;
    tRuntime_bbf_ctl4                   bbf_ctl4;
    tRuntime_pll_ac_reg10               pll_ac_reg10;
}sRuntimePMValues;

#define NUM_PROCESS_MONITOR_BINS    5
typedef enum
{
    CTUNE_BIN_0,          //
    CTUNE_BIN_1,          //
    CTUNE_BIN_2,          //
    CTUNE_BIN_3,          //
    CTUNE_BIN_4,          //
    NUM_CTUNE_BINS
}eCTuneBins;

typedef enum
{
    PM_BIN_0,          //
    PM_BIN_1,          //
    PM_BIN_2,          //
    PM_BIN_3,          //
    PM_BIN_4,          //
    NUM_PM_BINS
}ePMBins;

typedef tRuntime_pa_ctune_reg                tFull_pa_ctune_reg                  [NUM_PROCESS_MONITOR_BINS][32];
typedef tRuntime_tx_bbf_gain_cnt             tFull_tx_bbf_gain_cnt               [NUM_PROCESS_MONITOR_BINS];
typedef tRuntime_tx_bbf_lin_adj              tFull_tx_bbf_lin_adj;
typedef tRuntime_lo_mix_da_gain_cntl         tFull_lo_mix_da_gain_cntl;
typedef tRuntime_pa_gain_cntl                tFull_pa_gain_cntl                  [NUM_PROCESS_MONITOR_BINS];
typedef tRuntime_da_pa_bias_1_cnt            tFull_da_pa_bias_1_cnt              [NUM_PROCESS_MONITOR_BINS];
typedef tRuntime_da_pa_bias_2_cntl           tFull_da_pa_bias_2_cntl             [NUM_PROCESS_MONITOR_BINS];
typedef tRuntime_tcxo_reg_prg                tFull_tcxo_reg_prg                  [NUM_PROCESS_MONITOR_BINS];
typedef tRuntime_cp_gain                     tFull_cp_gain                       [NUM_PROCESS_MONITOR_BINS];
typedef tRuntime_rxfe_lna_highgain_bias_ctl  tFull_rxfe_lna_highgain_bias_ctl    [NUM_PROCESS_MONITOR_BINS];
typedef tRuntime_rxfe_gm_ibias_core_ctrl     tFull_rxfe_gm_ibias_core_ctrl       [NUM_PROCESS_MONITOR_BINS];
typedef tRuntime_rxfe_gm_bias_ctrl           tFull_rxfe_gm_bias_ctrl             [NUM_PROCESS_MONITOR_BINS];
typedef tRuntime_tia_ibias                   tFull_tia_ibias                     [NUM_PROCESS_MONITOR_BINS];
typedef tRuntime_im2_vcm_bq                  tFull_im2_vcm_bq                    [NUM_PROCESS_MONITOR_BINS];
typedef tRuntime_im2_vgf                     tFull_im2_vgf                       [NUM_PROCESS_MONITOR_BINS];
typedef tRuntime_hdet_bias                   tFull_hdet_bias                     [NUM_PROCESS_MONITOR_BINS];
typedef tRuntime_hdet_test                   tFull_hdet_test                     [NUM_PROCESS_MONITOR_BINS];
typedef tRuntime_hdet_ctl                    tFull_hdet_ctl                      [NUM_PROCESS_MONITOR_BINS];
typedef tRuntime_pll_ac_reg5                 tFull_pll_ac_reg5                   [NUM_PROCESS_MONITOR_BINS];
typedef tRuntime_pll_ac_reg1                 tFull_pll_ac_reg1                   [NUM_PROCESS_MONITOR_BINS];
typedef tRuntime_pll_pc_reg3                 tFull_pll_pc_reg3                   [NUM_PROCESS_MONITOR_BINS];
typedef tRuntime_tx_lo_mix_da_adj            tFull_tx_lo_mix_da_adj              [NUM_PROCESS_MONITOR_BINS];
typedef tRuntime_pll_pc_reg2                 tFull_pll_pc_reg2                   [NUM_PROCESS_MONITOR_BINS];
typedef tRuntime_pll_reg6                    tFull_pll_reg6                      [NUM_PROCESS_MONITOR_BINS];
typedef tRuntime_pll_reg4                    tFull_pll_reg4                      [NUM_PROCESS_MONITOR_BINS];
typedef tRuntime_pll_vb_reg0                 tFull_pll_vb_reg0                   [NUM_PROCESS_MONITOR_BINS];
typedef tRuntime_pll_ac_reg0                 tFull_pll_ac_reg0                   [NUM_PROCESS_MONITOR_BINS];
typedef tRuntime_bbf_ctl3                    tFull_bbf_ctl3                      [NUM_PROCESS_MONITOR_BINS];
typedef tRuntime_bbf_ctl4                    tFull_bbf_ctl4                      [NUM_PROCESS_MONITOR_BINS];
typedef tRuntime_pll_ac_reg10                tFull_pll_ac_reg10                  [NUM_PROCESS_MONITOR_BINS];

typedef struct
{
    tFull_pa_ctune_reg               pa_ctune_reg;
    tFull_tx_bbf_gain_cnt            tx_bbf_gain_cnt;
    tFull_tx_bbf_lin_adj             tx_bbf_lin_adj;
    tFull_lo_mix_da_gain_cntl        lo_mix_da_gain_cntl;
    tFull_pa_gain_cntl               pa_gain_cntl;
    tFull_da_pa_bias_1_cnt           da_pa_bias_1_cnt;
    tFull_da_pa_bias_2_cntl          da_pa_bias_2_cntl;
    tFull_tcxo_reg_prg               tcxo_reg_prg;
    tFull_cp_gain                    cp_gain;
    tFull_rxfe_lna_highgain_bias_ctl rxfe_lna_highgain_bias_ctl;
    tFull_rxfe_gm_ibias_core_ctrl    rxfe_gm_ibias_core_ctrl;
    tFull_rxfe_gm_bias_ctrl          rxfe_gm_bias_ctrl;
    tFull_tia_ibias                  tia_ibias;
    tFull_im2_vcm_bq                 im2_vcm_bq;
    tFull_im2_vgf                    im2_vgf;
    tFull_hdet_bias                  hdet_bias;
    tFull_hdet_test                  hdet_test;
    tFull_hdet_ctl                   hdet_ctl;
    tFull_pll_ac_reg5                pll_ac_reg5;
    tFull_pll_ac_reg1                pll_ac_reg1;
    tFull_pll_pc_reg3                pll_pc_reg3;
    tFull_tx_lo_mix_da_adj           tx_lo_mix_da_adj;
    tFull_pll_pc_reg2                pll_pc_reg2;
    tFull_pll_reg6                   pll_reg6;
    tFull_pll_reg4                   pll_reg4;
    tFull_pll_vb_reg0                pll_vb_reg0;
    tFull_pll_ac_reg0                pll_ac_reg0;
    tFull_bbf_ctl3                   bbf_ctl3;
    tFull_bbf_ctl4                   bbf_ctl4;
}sFullPMValues;

typedef struct
{
    tFull_pa_ctune_reg               pa_ctune_reg;
    tFull_tx_bbf_gain_cnt            tx_bbf_gain_cnt;
    tFull_tx_bbf_lin_adj             tx_bbf_lin_adj;
    tFull_lo_mix_da_gain_cntl        lo_mix_da_gain_cntl;
    tFull_pa_gain_cntl               pa_gain_cntl;
    tFull_da_pa_bias_1_cnt           da_pa_bias_1_cnt;
    tFull_da_pa_bias_2_cntl          da_pa_bias_2_cntl;
    tFull_rxfe_lna_highgain_bias_ctl rxfe_lna_highgain_bias_ctl;
    tFull_rxfe_gm_ibias_core_ctrl    rxfe_gm_ibias_core_ctrl;
    tFull_rxfe_gm_bias_ctrl          rxfe_gm_bias_ctrl;
    tFull_tia_ibias                  tia_ibias;
    tFull_hdet_bias                  hdet_bias;
    tFull_hdet_test                  hdet_test;
    tFull_hdet_ctl                   hdet_ctl;
    tFull_pll_ac_reg5                pll_ac_reg5;
    tFull_pll_ac_reg1                pll_ac_reg1;
    tFull_pll_pc_reg3                pll_pc_reg3;
    tFull_tx_lo_mix_da_adj           tx_lo_mix_da_adj;
    tFull_pll_pc_reg2                pll_pc_reg2;
    tFull_pll_reg6                   pll_reg6;
    tFull_pll_reg4                   pll_reg4;
    tFull_pll_vb_reg0                pll_vb_reg0;
    tFull_pll_ac_reg0                pll_ac_reg0;
    tFull_pll_ac_reg10               pll_ac_reg10;
    tFull_bbf_ctl3                   bbf_ctl3;
    tFull_bbf_ctl4                   bbf_ctl4;
}sFullPMValuesVolans2;


#define CAL_STATUS_OVERALL_SUCCESS 0
#define CAL_STATUS_OVERALL_NOT_EXECUTED 0xFF
#define CAL_STATUS_OVERALL_FROM_CAL_MEMORY 0x01 //cal memory used to prepopulate all cal correction values

#define CAL_STATUS_FWINIT_SUCCESS 0
#define CAL_STATUS_FWINIT_NOT_EXECUTED 0xFF

#define CAL_STATUS_HDET_DCO_SUCCESS 0
#define CAL_STATUS_HDET_DCO_NOT_EXECUTED 0xFF

#define CAL_STATUS_RTUNER_SUCCESS 0
#define CAL_STATUS_RTUNER_STAGE_1_TIMED_OUT                 1
#define CAL_STATUS_RTUNER_STAGE_2_TIMED_OUT                 2
#define CAL_STATUS_RTUNER_NOT_EXECUTED 0xFF

#define CAL_STATUS_CTUNER_SUCCESS 0
#define CAL_STATUS_CTUNER_TIA_OUT_OF_BOUNDS_LIMITATION_255  1
#define CAL_STATUS_CTUNER_TIA_OUT_OF_BOUNDS_LIMITATION_0    2
#define CAL_STATUS_CTUNER_RCMEAS_EQUALS_0                   3
#define CAL_STATUS_CTUNER_INCOMPLETE_RCMEAS                 4
#define CAL_STATUS_CTUNER_NOT_EXECUTED 0xFF

#define CAL_STATUS_INSITU_SUCCESS 0
#define CAL_STATUS_INSITU_NOT_EXECUTED 0xFF

#define CAL_STATUS_PROCESS_MONITOR_SUCCESS 0
#define CAL_STATUS_PROCESS_MONITOR_NOT_EXECUTED 0xFF

#define CAL_STATUS_PLLVCOLINEARITY_SUCCESS 0
#define CAL_STATUS_PLLVCOLINEARITY_NOT_EXECUTED 0xFF

#define CAL_STATUS_TXIQ_SUCCESS 0
#define CAL_STATUS_TXIQ_NOT_EXECUTED 0xFF

#define CAL_STATUS_RXIQ_SUCCESS 0
#define CAL_STATUS_RXIQ_NOT_EXECUTED 0xFF

#define CAL_STATUS_RXDCO_SUCCESS 0
#define CAL_STATUS_RXDCO_NOT_EXECUTED 0xFF
#define CAL_STATUS_RXDCO_MEAS_ERROR 1

#define CAL_STATUS_TXLO_SUCCESS 0
#define CAL_STATUS_TXLO_NOT_EXECUTED 0xFF

#define CAL_STATUS_LNABIAS_SUCCESS 0
#define CAL_STATUS_LNABIAS_NOT_EXECUTED 0xFF

#define CAL_STATUS_LNABANDTUNING_SUCCESS 0
#define CAL_STATUS_LNABANDTUNING_NOT_EXECUTED 0xFF

#define CAL_STATUS_LNAGAINADJUST_SUCCESS 0
#define CAL_STATUS_LNAGAINADJUST_NOT_EXECUTED 0xFF

#define CAL_STATUS_IM2USINGNOISEPWR_SUCCESS 0
#define CAL_STATUS_IM2USINGNOISEPWR_NOT_EXECUTED 0xFF

#define CAL_STATUS_TEMPERATURE_SUCCESS 0
#define CAL_STATUS_TEMPERATURE_NOT_EXECUTED 0xFF

#define CAL_STATUS_CLPC_SUCCESS 0
#define CAL_STATUS_CLPC_NOT_EXECUTED 0xFF

#define CAL_STATUS_CLPC_TEMP_ADJUST_SUCCESS 0
#define CAL_STATUS_CLPC_TEMP_ADJUST_NOT_EXECUTED 0xFF

#define CAL_STATUS_TXDPD_SUCCESS 0
#define CAL_STATUS_TXDPD_NOT_EXECUTED 0xFF

#define CAL_STATUS_CHANNELTUNE_SUCCESS 0
#define CAL_STATUS_CHANNELTUNE_NOT_EXECUTED 0xFF

#define CAL_STATUS_RXGMSTAGELINEARITY_SUCCESS 0
#define CAL_STATUS_RXGMSTAGELINEARITY_NOT_EXECUTED 0xFF

#define CAL_STATUS_IM2USINGTONEGEN_SUCCESS 0
#define CAL_STATUS_IM2USINGTONEGEN_NOT_EXECUTED 0xFF


typedef struct
{
#ifdef ANI_BIG_BYTE_ENDIAN
    tANI_U8     overall;
    tANI_U8     fwInit;
    tANI_U8     hdet_dco;
    tANI_U8     rtuner;

    tANI_U8     ctuner;
    tANI_U8     insitu;
    tANI_U8     process_monitor;
    tANI_U8     pllVcoLinearity;

    tANI_U8     txIQ;
    tANI_U8     rxIQ;
    tANI_U8     rxDco;
    tANI_U8     txLo;

    tANI_U8     lnaBias;
    tANI_U8     lnaBandTuning;
    tANI_U8     lnaGainAdjust;
    tANI_U8     im2UsingNoisePwr;

    tANI_U8     temperature;
    tANI_U8     clpc;
    tANI_U8     clpc_temp_adjust;
    tANI_U8     txDpd;

    tANI_U8     channelTune;
    tANI_U8     rxGmStageLinearity;
    tANI_U8     im2UsingToneGen;
    tANI_U8     unused1;
#else
    tANI_U8     rtuner;
    tANI_U8     hdet_dco;
    tANI_U8     fwInit;
    tANI_U8     overall;

    tANI_U8     pllVcoLinearity;
    tANI_U8     process_monitor;
    tANI_U8     insitu;
    tANI_U8     ctuner;

    tANI_U8     txLo;
    tANI_U8     rxDco;
    tANI_U8     rxIQ;
    tANI_U8     txIQ;

    tANI_U8     im2UsingNoisePwr;
    tANI_U8     lnaGainAdjust;
    tANI_U8     lnaBandTuning;
    tANI_U8     lnaBias;

    tANI_U8     txDpd;
    tANI_U8     clpc_temp_adjust;
    tANI_U8     clpc;
    tANI_U8     temperature;

    tANI_U8     unused1;
    tANI_U8     im2UsingToneGen;
    tANI_U8     rxGmStageLinearity;
    tANI_U8     channelTune;
#endif

    tANI_U8     unused2[8];
}sCalStatus;

typedef struct
{
#ifdef ANI_BIG_BYTE_ENDIAN
    tANI_U16    process_monitor;
    tANI_U8     hdet_cal_code;
    tANI_U8     rxfe_gm_2;

    tANI_U8     tx_bbf_rtune;
    tANI_U8     pa_rtune_reg;
    tANI_U8     rt_code;
    tANI_U8     bias_rtune;

    tANI_U8     bb_bw1;
    tANI_U8     bb_bw2;
    tANI_U8     pa_ctune_reg;
    tANI_U8     reserved1;

    tANI_U8     bb_bw3;
    tANI_U8     bb_bw4;
    tANI_U8     bb_bw5;
    tANI_U8     bb_bw6;

    tANI_U16    rcMeasured;
    tANI_U8     tx_bbf_ct;
    tANI_U8     tx_bbf_ctr;

    tANI_U8     csh_maxgain_reg;
    tANI_U8     csh_0db_reg;
    tANI_U8     csh_m3db_reg;
    tANI_U8     csh_m6db_reg;

    tANI_U8     cff_0db_reg;
    tANI_U8     cff_m3db_reg;
    tANI_U8     cff_m6db_reg;
    tANI_U8     rxfe_gpio_ctl_1;

    tANI_U8     mix_bal_cnt_2;
    tANI_S8     rxfe_lna_highgain_bias_ctl_delta;
    tANI_U8     rxfe_lna_load_ctune;
    tANI_U8     rxfe_lna_ngm_rtune;

    tANI_U8     rx_im2_spare0;
    tANI_U8     rx_im2_spare1;
    tANI_U16    hdet_dco;

    tANI_U8     pll_vfc_reg3_b0;
    tANI_U8     pll_vfc_reg3_b1;
    tANI_U8     pll_vfc_reg3_b2;
    tANI_U8     pll_vfc_reg3_b3;

    tANI_U16    tempStart;
    tANI_U16    roomTemp;

    tANI_S16    ambientCalTemp;
    tANI_U8     ambientCalTempValid;
    tANI_U8     reserved2;
#else //ANI_BIG_BYTE_ENDIAN
    tANI_U8     rxfe_gm_2;
    tANI_U8     hdet_cal_code;
    tANI_U16    process_monitor;

    tANI_U8     bias_rtune;
    tANI_U8     rt_code;
    tANI_U8     pa_rtune_reg;
    tANI_U8     tx_bbf_rtune;

    tANI_U8     reserved1;
    tANI_U8     pa_ctune_reg;
    tANI_U8     bb_bw2;
    tANI_U8     bb_bw1;

    tANI_U8     bb_bw6;
    tANI_U8     bb_bw5;
    tANI_U8     bb_bw4;
    tANI_U8     bb_bw3;

    tANI_U8     tx_bbf_ctr;
    tANI_U8     tx_bbf_ct;
    tANI_U16    rcMeasured;

    tANI_U8     csh_m6db_reg;
    tANI_U8     csh_m3db_reg;
    tANI_U8     csh_0db_reg;
    tANI_U8     csh_maxgain_reg;

    tANI_U8     rxfe_gpio_ctl_1;
    tANI_U8     cff_m6db_reg;
    tANI_U8     cff_m3db_reg;
    tANI_U8     cff_0db_reg;

    tANI_U8     rxfe_lna_ngm_rtune;
    tANI_U8     rxfe_lna_load_ctune;
    tANI_S8     rxfe_lna_highgain_bias_ctl_delta;
    tANI_U8     mix_bal_cnt_2;

    tANI_U16    hdet_dco;
    tANI_U8     rx_im2_spare1;
    tANI_U8     rx_im2_spare0;

    tANI_U8     pll_vfc_reg3_b3;
    tANI_U8     pll_vfc_reg3_b2;
    tANI_U8     pll_vfc_reg3_b1;
    tANI_U8     pll_vfc_reg3_b0;

    tANI_U16    roomTemp;
    tANI_U16    tempStart;

    tANI_U8     reserved2;
    tANI_U8     ambientCalTempValid;
    tANI_S16    ambientCalTemp;
#endif

}sCalData;

typedef struct
{
    sCalData            nvCalData;
    tTxLoCorrections    txLoValues;
    sTxIQChannel        txIqValues;
    sRxIQChannel        rxIqValues;
    tTpcConfig          clpcData[MAX_TPC_CHANNELS];
    tTpcParams          clpcParams[MAX_TPC_CHANNELS];

}sCalFlashMemory;


typedef struct
{
    sCalStatus              calStatus;
    sCalFlashMemory         calFlash;
    sRuntimePMValues        tempGainDependentRF;
}sCalMemory;

typedef enum
{
    eNV_CALID_PROCESS_MONITOR   = 0x1,
    eNV_CALID_HDET_CAL_CODE     = 0x2,
    eNV_CALID_PLL_VCO_LINEARITY = 0x4,
    eNV_CALID_RTUNER            = 0x8,
    eNV_CALID_CTUNER            = 0x10,
    eNV_CALID_RX_IM2            = 0x20,
    eNV_CALID_TEMPERATURE       = 0x40,
    eNV_CALID_RXFE_GM2          = 0x80,
    eNV_CALID_LNA_BANDSETTING   = 0x100,
    eNV_CALID_LNA_BANDTUNING    = 0x200,
    eNV_CALID_LNA_GAINADJUST    = 0x400
}eNvCalID;


typedef struct
{
    tANI_U32 calStatus;  //use eNvCalID
    sCalData calData;
}sRFCalValues;

extern const sFullPMValues PMTable; //large table exists in 80KB segment of memory only at initialization
extern const sFullPMValuesVolans2 PMTableVolans2; //large table exists in 80KB segment of memory only at initialization
extern const sFullPMValuesVolans2 PMTableVolans3; //large table exists in 80KB segment of memory only at initialization
extern sCalMemory *calMemory;  //smaller table resides in 128KB segment of memory for runtime use

#endif

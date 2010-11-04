
#ifndef PHYCALMEMORY_H
#define PHYCALMEMORY_H

#include <halPhyCfg.h>

typedef tTxLoCorrect tTxLoCorrections[NUM_RF_CHANNELS][NUM_TX_GAIN_STEPS];
typedef sIQCalValues sTxIQChannel[NUM_RF_CHANNELS][NUM_TX_GAIN_STEPS];
typedef sIQCalValues sRxIQChannel[NUM_RF_CHANNELS][NUM_TX_GAIN_STEPS];




typedef tANI_U8 tRuntime_pa_ctune_reg               [NUM_TEMPERATURE_BINS];
typedef tANI_U8 tRuntime_pa_Istg1_tune              [NUM_TEMPERATURE_BINS][NUM_TX_GAIN_STEPS];
typedef tANI_U8 tRuntime_pa_Istg2_tune              [NUM_TEMPERATURE_BINS][NUM_TX_GAIN_STEPS];
typedef tANI_U8 tRuntime_pa_Ibn_stg3_tune           [NUM_TEMPERATURE_BINS][NUM_TX_GAIN_STEPS];
typedef tANI_U8 tRuntime_pa_Icas_stg3_tune          [NUM_TEMPERATURE_BINS][NUM_TX_GAIN_STEPS];
typedef tANI_U8 tRuntime_pa_gain1                   [NUM_TEMPERATURE_BINS][NUM_TX_GAIN_STEPS];
typedef tANI_U8 tRuntime_pa_gain2                   [NUM_TEMPERATURE_BINS][NUM_TX_GAIN_STEPS];
typedef tANI_U8 tRuntime_tcxo_reg_prg               [NUM_TEMPERATURE_BINS];
typedef tANI_U8 tRuntime_sel_vco_Rreg_3_0           [NUM_TEMPERATURE_BINS];
typedef tANI_U8 tRuntime_lo_knob_1_0                [NUM_TEMPERATURE_BINS];
typedef tANI_U8 tRuntime_cp_gain                    [NUM_TEMPERATURE_BINS];
typedef tANI_U8 tRuntime_rxfe_lna_highgain_bias_ctl [NUM_TEMPERATURE_BINS];
typedef tANI_U8 tRuntime_rxfe_gm_ibias_core_ctrl    [NUM_TEMPERATURE_BINS];
typedef tANI_U8 tRuntime_rxfe_gm_bias_ctrl          [NUM_TEMPERATURE_BINS];
typedef tANI_U8 tRuntime_tia_icq                    [NUM_TEMPERATURE_BINS];
typedef tANI_U8 tRuntime_bq_icq                     [NUM_TEMPERATURE_BINS];
typedef tANI_U8 tRuntime_bq_ibias                   [NUM_TEMPERATURE_BINS];
typedef tANI_U8 tRuntime_pga_icq                    [NUM_TEMPERATURE_BINS];
typedef tANI_U8 tRuntime_pga_ibias_rx               [NUM_TEMPERATURE_BINS];
typedef tANI_U8 tRuntime_tia_ibias                  [NUM_TEMPERATURE_BINS];

typedef tANI_U8 tRuntime_hdet_bias                  [NUM_TEMPERATURE_BINS];
typedef tANI_U8 tRuntime_hdet_test                  [NUM_TEMPERATURE_BINS];

typedef struct
{
    tRuntime_pa_ctune_reg               pa_ctune_reg;
    tRuntime_pa_Istg1_tune              pa_Istg1_tune;
    tRuntime_pa_Istg2_tune              pa_Istg2_tune;
    tRuntime_pa_Ibn_stg3_tune           pa_Ibn_stg3_tune;
    tRuntime_pa_Icas_stg3_tune          pa_Icas_stg3_tune;
    tRuntime_pa_gain1                   pa_gain1;
    tRuntime_pa_gain2                   pa_gain2;
    tRuntime_tcxo_reg_prg               tcxo_reg_prg;
    tRuntime_sel_vco_Rreg_3_0           sel_vco_Rreg_3_0;
    tRuntime_lo_knob_1_0                lo_knob_1_0;
    tRuntime_cp_gain                    cp_gain;
    tRuntime_rxfe_lna_highgain_bias_ctl rxfe_lna_highgain_bias_ctl;
    tRuntime_rxfe_gm_ibias_core_ctrl    rxfe_gm_ibias_core_ctrl;
    tRuntime_rxfe_gm_bias_ctrl          rxfe_gm_bias_ctrl;
    tRuntime_tia_icq                    tia_icq;
    tRuntime_bq_icq                     bq_icq;
    tRuntime_bq_ibias                   bq_ibias;
    tRuntime_pga_icq                    pga_icq;
    tRuntime_pga_ibias_rx               pga_ibias_rx;
    tRuntime_tia_ibias                  tia_ibias;
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

typedef tRuntime_pa_ctune_reg               tFull_pa_ctune_reg                  [NUM_PROCESS_MONITOR_BINS][NUM_CTUNE_BINS];
typedef tRuntime_pa_Istg1_tune              tFull_pa_Istg1_tune                 [NUM_PROCESS_MONITOR_BINS][NUM_CTUNE_BINS];
typedef tRuntime_pa_Istg2_tune              tFull_pa_Istg2_tune                 [NUM_PROCESS_MONITOR_BINS][NUM_CTUNE_BINS];
typedef tRuntime_pa_Ibn_stg3_tune           tFull_pa_Ibn_stg3_tune              [NUM_PROCESS_MONITOR_BINS][NUM_CTUNE_BINS];
typedef tRuntime_pa_Icas_stg3_tune          tFull_pa_Icas_stg3_tune             [NUM_PROCESS_MONITOR_BINS][NUM_CTUNE_BINS];
typedef tRuntime_pa_gain1                   tFull_pa_gain1                      [NUM_PROCESS_MONITOR_BINS][NUM_CTUNE_BINS];
typedef tRuntime_pa_gain2                   tFull_pa_gain2                      [NUM_PROCESS_MONITOR_BINS][NUM_CTUNE_BINS];
typedef tRuntime_tcxo_reg_prg               tFull_tcxo_reg_prg                  [NUM_PROCESS_MONITOR_BINS];
typedef tRuntime_sel_vco_Rreg_3_0           tFull_sel_vco_Rreg_3_0              [NUM_PROCESS_MONITOR_BINS];
typedef tRuntime_lo_knob_1_0                tFull_lo_knob_1_0                   [NUM_PROCESS_MONITOR_BINS];
typedef tRuntime_cp_gain                    tFull_cp_gain                       [NUM_PROCESS_MONITOR_BINS];
typedef tRuntime_rxfe_lna_highgain_bias_ctl tFull_rxfe_lna_highgain_bias_ctl    [NUM_PROCESS_MONITOR_BINS];
typedef tRuntime_rxfe_gm_ibias_core_ctrl    tFull_rxfe_gm_ibias_core_ctrl       [NUM_PROCESS_MONITOR_BINS];
typedef tRuntime_rxfe_gm_bias_ctrl          tFull_rxfe_gm_bias_ctrl             [NUM_PROCESS_MONITOR_BINS];
typedef tRuntime_tia_icq                    tFull_tia_icq                       [NUM_PROCESS_MONITOR_BINS];
typedef tRuntime_bq_icq                     tFull_bq_icq                        [NUM_PROCESS_MONITOR_BINS];
typedef tRuntime_bq_ibias                   tFull_bq_ibias                      [NUM_PROCESS_MONITOR_BINS];
typedef tRuntime_pga_icq                    tFull_pga_icq                       [NUM_PROCESS_MONITOR_BINS];
typedef tRuntime_pga_ibias_rx               tFull_pga_ibias_rx                  [NUM_PROCESS_MONITOR_BINS];
typedef tRuntime_tia_ibias                  tFull_tia_ibias                     [NUM_PROCESS_MONITOR_BINS];


typedef struct
{
    tFull_pa_ctune_reg               pa_ctune_reg;
    tFull_pa_Istg1_tune              pa_Istg1_tune;
    tFull_pa_Istg2_tune              pa_Istg2_tune;
    tFull_pa_Ibn_stg3_tune           pa_Ibn_stg3_tune;
    tFull_pa_Icas_stg3_tune          pa_Icas_stg3_tune;
    tFull_pa_gain1                   pa_gain1;
    tFull_pa_gain2                   pa_gain2;
    tFull_tcxo_reg_prg               tcxo_reg_prg;
    tFull_sel_vco_Rreg_3_0           sel_vco_Rreg_3_0;
    tFull_lo_knob_1_0                lo_knob_1_0;
    tFull_cp_gain                    cp_gain;
    tFull_rxfe_lna_highgain_bias_ctl rxfe_lna_highgain_bias_ctl;
    tFull_rxfe_gm_ibias_core_ctrl    rxfe_gm_ibias_core_ctrl;
    tFull_rxfe_gm_bias_ctrl          rxfe_gm_bias_ctrl;
    tFull_tia_icq                    tia_icq;
    tFull_bq_icq                     bq_icq;
    tFull_bq_ibias                   bq_ibias;
    tFull_pga_icq                    pga_icq;
    tFull_pga_ibias_rx               pga_ibias_rx;
    tFull_tia_ibias                  tia_ibias;
}sFullPMValues;


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
    tANI_U8     unused[9];
}sCalStatus;

typedef struct
{
    tANI_U16    process_monitor;
    tANI_U8     hdet_cal_code;
    tANI_U8     rxfe_gm_2;

    tANI_U8     tx_bbf_rtune;
    tANI_U8     pa_rtune_reg;
    tANI_U8     rt_code;
    tANI_U8     bias_rtune;

    tANI_U8     bb_bw1;
    tANI_U8     bb_bw2;
    tANI_U8     reserved[2];

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

    tANI_U8     rx_im2_i_cfg0;
    tANI_U8     rx_im2_i_cfg1;
    tANI_U8     rx_im2_q_cfg0;
    tANI_U8     rx_im2_q_cfg1;

    tANI_U8     pll_vfc_reg3_b0;
    tANI_U8     pll_vfc_reg3_b1;
    tANI_U8     pll_vfc_reg3_b2;
    tANI_U8     pll_vfc_reg3_b3;

    tANI_U16    tempStart;
    tANI_U16    tempFinish;

    tTxLoCorrections    txLoValues;
    sTxIQChannel        txIqValues;
    sRxIQChannel        rxIqValues;
    tTpcConfig          clpcData[MAX_TPC_CHANNELS];
    tTpcParams          clpcParams[MAX_TPC_CHANNELS];

}sCalFlashMemory;


typedef struct
{
    sCalStatus          calStatus;
    sCalFlashMemory     calFlash;
    sRuntimePMValues    tempGainDependentRF;
}sCalMemory;

extern const sFullPMValues PMTable; //large table exists in 80KB segment of memory only at initialization
extern sCalMemory *calMemory;  //smaller table resides in 128KB segment of memory for runtime use

#endif

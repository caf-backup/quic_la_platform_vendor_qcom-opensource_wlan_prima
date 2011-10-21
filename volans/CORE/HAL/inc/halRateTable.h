/*
 * Qualcomm Inc proprietary. All rights reserved.
 * halRateTable.h:  Provides APIs for rate table setup and access
 * Author:    Arul Vasantharaj
 * Date:      05/14/2008
 *
 * --------------------------------------------------------------------------
 */
#ifndef _HALRATETABLE_H_
#define _HALRATETABLE_H_

#include "halFw.h"

//Libra Rate Table definitions
//Spica_Virgo 11A 20MHz Rates                   //psdu rate codes
#define HAL_PSDU_RATE_11A_48_MBPS                 8
#define HAL_PSDU_RATE_11A_24_MBPS                 9
#define HAL_PSDU_RATE_11A_12_MBPS                 10
#define HAL_PSDU_RATE_11A_6_MBPS                  11
#define HAL_PSDU_RATE_11A_54_MBPS                 12
#define HAL_PSDU_RATE_11A_36_MBPS                 13
#define HAL_PSDU_RATE_11A_18_MBPS                 14
#define HAL_PSDU_RATE_11A_9_MBPS                  15

//Spica_Virgo 11A 40MHz Duplicate Rates
#define HAL_PSDU_RATE_11A_DUP_48_MBPS             8
#define HAL_PSDU_RATE_11A_DUP_24_MBPS             9
#define HAL_PSDU_RATE_11A_DUP_12_MBPS             10
#define HAL_PSDU_RATE_11A_DUP_6_MBPS              11
#define HAL_PSDU_RATE_11A_DUP_54_MBPS             12
#define HAL_PSDU_RATE_11A_DUP_36_MBPS             13
#define HAL_PSDU_RATE_11A_DUP_18_MBPS             14
#define HAL_PSDU_RATE_11A_DUP_9_MBPS              15

//MCS Index #0-15 (20MHz) Greenfield Mode
#define HAL_PSDU_RATE_MCS_1NSS_GF_6_5_MBPS            0
#define HAL_PSDU_RATE_MCS_1NSS_GF_SG_7_2_MBPS         0
#define HAL_PSDU_RATE_MCS_1NSS_GF_13_MBPS             1
#define HAL_PSDU_RATE_MCS_1NSS_GF_SG_14_4_MBPS        1
#define HAL_PSDU_RATE_MCS_1NSS_GF_19_5_MBPS           2
#define HAL_PSDU_RATE_MCS_1NSS_GF_SG_21_7_MBPS        2
#define HAL_PSDU_RATE_MCS_1NSS_GF_26_MBPS             3
#define HAL_PSDU_RATE_MCS_1NSS_GF_SG_28_9_MBPS        3
#define HAL_PSDU_RATE_MCS_1NSS_GF_39_MBPS             4
#define HAL_PSDU_RATE_MCS_1NSS_GF_SG_43_3_MBPS        4
#define HAL_PSDU_RATE_MCS_1NSS_GF_52_MBPS             5
#define HAL_PSDU_RATE_MCS_1NSS_GF_SG_57_8_MBPS        5
#define HAL_PSDU_RATE_MCS_1NSS_GF_58_5_MBPS           6
#define HAL_PSDU_RATE_MCS_1NSS_GF_SG_65_MBPS          6
#define HAL_PSDU_RATE_MCS_1NSS_GF_65_MBPS             7
#define HAL_PSDU_RATE_MCS_1NSS_GF_SG_72_2_MBPS        7
#define HAL_PSDU_RATE_MCS_2NSS_GF_13_MBPS             8
#define HAL_PSDU_RATE_MCS_2NSS_GF_SG_14_444_MBPS      8
#define HAL_PSDU_RATE_MCS_2NSS_GF_26_MBPS             9
#define HAL_PSDU_RATE_MCS_2NSS_GF_SG_28_889_MBPS      9
#define HAL_PSDU_RATE_MCS_2NSS_GF_39_MBPS             10
#define HAL_PSDU_RATE_MCS_2NSS_GF_SG_43_333_MBPS      10
#define HAL_PSDU_RATE_MCS_2NSS_GF_52_MBPS             11
#define HAL_PSDU_RATE_MCS_2NSS_GF_SG_57_778_MBPS      11
#define HAL_PSDU_RATE_MCS_2NSS_GF_78_MBPS             12
#define HAL_PSDU_RATE_MCS_2NSS_GF_SG_86_667_MBPS      12
#define HAL_PSDU_RATE_MCS_2NSS_GF_104_MBPS            13
#define HAL_PSDU_RATE_MCS_2NSS_GF_SG_115_556_MBPS     13
#define HAL_PSDU_RATE_MCS_2NSS_GF_117_MBPS            14
#define HAL_PSDU_RATE_MCS_2NSS_GF_SG_130_MBPS         14
#define HAL_PSDU_RATE_MCS_2NSS_GF_130_MBPS            15
#define HAL_PSDU_RATE_MCS_2NSS_GF_SG_144_444_MBPS     15

//MCS Index #0-15 (40MHz) Greenfield Mode
#define HAL_PSDU_RATE_MCS_1NSS_GF_13_5_MBPS           0
#define HAL_PSDU_RATE_MCS_1NSS_GF_SG_15_MBPS          0
#define HAL_PSDU_RATE_MCS_1NSS_GF_27_MBPS             1
#define HAL_PSDU_RATE_MCS_1NSS_GF_SG_30_MBPS          1
#define HAL_PSDU_RATE_MCS_1NSS_GF_40_5_MBPS           2
#define HAL_PSDU_RATE_MCS_1NSS_GF_SG_45_MBPS          2
#define HAL_PSDU_RATE_MCS_1NSS_GF_54_MBPS             3
#define HAL_PSDU_RATE_MCS_1NSS_GF_SG_60_MBPS          3
#define HAL_PSDU_RATE_MCS_1NSS_GF_81_MBPS             4
#define HAL_PSDU_RATE_MCS_1NSS_GF_SG_90_MBPS          4
#define HAL_PSDU_RATE_MCS_1NSS_GF_108_MBPS            5
#define HAL_PSDU_RATE_MCS_1NSS_GF_SG_120_MBPS         5
#define HAL_PSDU_RATE_MCS_1NSS_GF_121_5_MBPS          6
#define HAL_PSDU_RATE_MCS_1NSS_GF_SG_135_MBPS         6
#define HAL_PSDU_RATE_MCS_1NSS_GF_135_MBPS            7
#define HAL_PSDU_RATE_MCS_1NSS_GF_SG_150_MBPS         7
#define HAL_PSDU_RATE_MCS_2NSS_GF_27_MBPS             8
#define HAL_PSDU_RATE_MCS_2NSS_GF_SG_30_MBPS          8
#define HAL_PSDU_RATE_MCS_2NSS_GF_54_MBPS             9
#define HAL_PSDU_RATE_MCS_2NSS_GF_SG_60_MBPS          9
#define HAL_PSDU_RATE_MCS_2NSS_GF_81_MBPS             10
#define HAL_PSDU_RATE_MCS_2NSS_GF_SG_90_MBPS          10
#define HAL_PSDU_RATE_MCS_2NSS_GF_108_MBPS            11
#define HAL_PSDU_RATE_MCS_2NSS_GF_SG_120_MBPS         11
#define HAL_PSDU_RATE_MCS_2NSS_GF_162_MBPS            12
#define HAL_PSDU_RATE_MCS_2NSS_GF_SG_180_MBPS         12
#define HAL_PSDU_RATE_MCS_2NSS_GF_216_MBPS            13
#define HAL_PSDU_RATE_MCS_2NSS_GF_SG_240_MBPS         13
#define HAL_PSDU_RATE_MCS_2NSS_GF_243_MBPS            14
#define HAL_PSDU_RATE_MCS_2NSS_GF_SG_270_MBPS         14
#define HAL_PSDU_RATE_MCS_2NSS_GF_270_MBPS            15
#define HAL_PSDU_RATE_MCS_2NSS_GF_SG_300_MBPS         15

//MCS Index #0-15 (20MHz) Mixed Mode
#define HAL_PSDU_RATE_MCS_1NSS_MM_6_5_MBPS            0
#define HAL_PSDU_RATE_MCS_1NSS_MM_SG_7_2_MBPS         0
#define HAL_PSDU_RATE_MCS_1NSS_MM_13_MBPS             1
#define HAL_PSDU_RATE_MCS_1NSS_MM_SG_14_4_MBPS        1
#define HAL_PSDU_RATE_MCS_1NSS_MM_19_5_MBPS           2
#define HAL_PSDU_RATE_MCS_1NSS_MM_SG_21_7_MBPS        2
#define HAL_PSDU_RATE_MCS_1NSS_MM_26_MBPS             3
#define HAL_PSDU_RATE_MCS_1NSS_MM_SG_28_9_MBPS        3
#define HAL_PSDU_RATE_MCS_1NSS_MM_39_MBPS             4
#define HAL_PSDU_RATE_MCS_1NSS_MM_SG_43_3_MBPS        4
#define HAL_PSDU_RATE_MCS_1NSS_MM_52_MBPS             5
#define HAL_PSDU_RATE_MCS_1NSS_MM_SG_57_8_MBPS        5
#define HAL_PSDU_RATE_MCS_1NSS_MM_58_5_MBPS           6
#define HAL_PSDU_RATE_MCS_1NSS_MM_SG_65_MBPS          6
#define HAL_PSDU_RATE_MCS_1NSS_MM_65_MBPS             7
#define HAL_PSDU_RATE_MCS_1NSS_MM_SG_72_2_MBPS        7
#define HAL_PSDU_RATE_MCS_2NSS_MM_13_MBPS             8
#define HAL_PSDU_RATE_MCS_2NSS_MM_SG_14_444_MBPS      8
#define HAL_PSDU_RATE_MCS_2NSS_MM_26_MBPS             9
#define HAL_PSDU_RATE_MCS_2NSS_MM_SG_28_889_MBPS      9
#define HAL_PSDU_RATE_MCS_2NSS_MM_39_MBPS             10
#define HAL_PSDU_RATE_MCS_2NSS_MM_SG_43_333_MBPS      10
#define HAL_PSDU_RATE_MCS_2NSS_MM_52_MBPS             11
#define HAL_PSDU_RATE_MCS_2NSS_MM_SG_57_778_MBPS      11
#define HAL_PSDU_RATE_MCS_2NSS_MM_78_MBPS             12
#define HAL_PSDU_RATE_MCS_2NSS_MM_SG_86_667_MBPS      12
#define HAL_PSDU_RATE_MCS_2NSS_MM_104_MBPS            13
#define HAL_PSDU_RATE_MCS_2NSS_MM_SG_115_556_MBPS     13
#define HAL_PSDU_RATE_MCS_2NSS_MM_117_MBPS            14
#define HAL_PSDU_RATE_MCS_2NSS_MM_SG_130_MBPS         14
#define HAL_PSDU_RATE_MCS_2NSS_MM_130_MBPS            15
#define HAL_PSDU_RATE_MCS_2NSS_MM_SG_144_444_MBPS     15

//MCS Index #0-15 (40MHz) Mixed Mode
#define HAL_PSDU_RATE_MCS_1NSS_MM_13_5_MBPS           0
#define HAL_PSDU_RATE_MCS_1NSS_MM_SG_15_MBPS          0
#define HAL_PSDU_RATE_MCS_1NSS_MM_27_MBPS             1
#define HAL_PSDU_RATE_MCS_1NSS_MM_SG_30_MBPS          1
#define HAL_PSDU_RATE_MCS_1NSS_MM_40_5_MBPS           2
#define HAL_PSDU_RATE_MCS_1NSS_MM_SG_45_MBPS          2
#define HAL_PSDU_RATE_MCS_1NSS_MM_54_MBPS             3
#define HAL_PSDU_RATE_MCS_1NSS_MM_SG_60_MBPS          3
#define HAL_PSDU_RATE_MCS_1NSS_MM_81_MBPS             4
#define HAL_PSDU_RATE_MCS_1NSS_MM_SG_90_MBPS          4
#define HAL_PSDU_RATE_MCS_1NSS_MM_108_MBPS            5
#define HAL_PSDU_RATE_MCS_1NSS_MM_SG_120_MBPS         5
#define HAL_PSDU_RATE_MCS_1NSS_MM_121_5_MBPS          6
#define HAL_PSDU_RATE_MCS_1NSS_MM_SG_135_MBPS         6
#define HAL_PSDU_RATE_MCS_1NSS_MM_135_MBPS            7
#define HAL_PSDU_RATE_MCS_1NSS_MM_SG_150_MBPS         7
#define HAL_PSDU_RATE_MCS_2NSS_MM_27_MBPS             8
#define HAL_PSDU_RATE_MCS_2NSS_MM_SG_30_MBPS          8
#define HAL_PSDU_RATE_MCS_2NSS_MM_54_MBPS             9
#define HAL_PSDU_RATE_MCS_2NSS_MM_SG_60_MBPS          9
#define HAL_PSDU_RATE_MCS_2NSS_MM_81_MBPS             10
#define HAL_PSDU_RATE_MCS_2NSS_MM_SG_90_MBPS          10
#define HAL_PSDU_RATE_MCS_2NSS_MM_108_MBPS            11
#define HAL_PSDU_RATE_MCS_2NSS_MM_SG_120_MBPS         11
#define HAL_PSDU_RATE_MCS_2NSS_MM_162_MBPS            12
#define HAL_PSDU_RATE_MCS_2NSS_MM_SG_180_MBPS         12
#define HAL_PSDU_RATE_MCS_2NSS_MM_216_MBPS            13
#define HAL_PSDU_RATE_MCS_2NSS_MM_SG_240_MBPS         13
#define HAL_PSDU_RATE_MCS_2NSS_MM_243_MBPS            14
#define HAL_PSDU_RATE_MCS_2NSS_MM_SG_270_MBPS         14
#define HAL_PSDU_RATE_MCS_2NSS_MM_270_MBPS            15
#define HAL_PSDU_RATE_MCS_2NSS_MM_SG_300_MBPS         15

//MCS Index #16-31 (20MHz) Greenfield Mode
#define HAL_PSDU_RATE_MCS_3NSS_GF_19_5_MBPS           16
#define HAL_PSDU_RATE_MCS_3NSS_GF_SG_21_7_MBPS        16
#define HAL_PSDU_RATE_MCS_3NSS_GF_39_MBPS             17
#define HAL_PSDU_RATE_MCS_3NSS_GF_SG_43_3_MBPS        17
#define HAL_PSDU_RATE_MCS_3NSS_GF_58_5_MBPS           18
#define HAL_PSDU_RATE_MCS_3NSS_GF_SG_65_MBPS          18
#define HAL_PSDU_RATE_MCS_3NSS_GF_78_MBPS             19
#define HAL_PSDU_RATE_MCS_3NSS_GF_SG_86_7_MBPS        19
#define HAL_PSDU_RATE_MCS_3NSS_GF_117_MBPS            20
#define HAL_PSDU_RATE_MCS_3NSS_GF_SG_130_MBPS         20
#define HAL_PSDU_RATE_MCS_3NSS_GF_156_MBPS            21
#define HAL_PSDU_RATE_MCS_3NSS_GF_SG_173_3_MBPS       21
#define HAL_PSDU_RATE_MCS_3NSS_GF_175_5_MBPS          22
#define HAL_PSDU_RATE_MCS_3NSS_GF_SG_195_MBPS         22
#define HAL_PSDU_RATE_MCS_3NSS_GF_195_MBPS            23
#define HAL_PSDU_RATE_MCS_3NSS_GF_SG_216_7_MBPS       23
#define HAL_PSDU_RATE_MCS_4NSS_GF_26_MBPS             24
#define HAL_PSDU_RATE_MCS_4NSS_GF_SG_28_9_MBPS        24
#define HAL_PSDU_RATE_MCS_4NSS_GF_52_MBPS             25
#define HAL_PSDU_RATE_MCS_4NSS_GF_SG_57_8_MBPS        25
#define HAL_PSDU_RATE_MCS_4NSS_GF_78_MBPS             26
#define HAL_PSDU_RATE_MCS_4NSS_GF_SG_86_7_MBPS        26
#define HAL_PSDU_RATE_MCS_4NSS_GF_104_MBPS            27
#define HAL_PSDU_RATE_MCS_4NSS_GF_SG_115_6_MBPS       27
#define HAL_PSDU_RATE_MCS_4NSS_GF_156_MBPS            28
#define HAL_PSDU_RATE_MCS_4NSS_GF_SG_173_3_MBPS       28
#define HAL_PSDU_RATE_MCS_4NSS_GF_208_MBPS            29
#define HAL_PSDU_RATE_MCS_4NSS_GF_SG_231_1_MBPS       29
#define HAL_PSDU_RATE_MCS_4NSS_GF_234_MBPS            30
#define HAL_PSDU_RATE_MCS_4NSS_GF_SG_260_MBPS         30
#define HAL_PSDU_RATE_MCS_4NSS_GF_260_MBPS            31
#define HAL_PSDU_RATE_MCS_4NSS_GF_SG_288_9_MBPS       31

//MCS Index #16-31 (40MHz) Greenfield Mode
#define HAL_PSDU_RATE_MCS_3NSS_GF_40_5_MBPS           16
#define HAL_PSDU_RATE_MCS_3NSS_GF_SG_45_MBPS          16
#define HAL_PSDU_RATE_MCS_3NSS_GF_81_MBPS             17
#define HAL_PSDU_RATE_MCS_3NSS_GF_SG_90_MBPS          17
#define HAL_PSDU_RATE_MCS_3NSS_GF_121_5_MBPS          18
#define HAL_PSDU_RATE_MCS_3NSS_GF_SG_135_MBPS         18
#define HAL_PSDU_RATE_MCS_3NSS_GF_162_MBPS            19
#define HAL_PSDU_RATE_MCS_3NSS_GF_SG_180_MBPS         19
#define HAL_PSDU_RATE_MCS_3NSS_GF_243_MBPS            20
#define HAL_PSDU_RATE_MCS_3NSS_GF_SG_270_MBPS         20
#define HAL_PSDU_RATE_MCS_3NSS_GF_324_MBPS            21
#define HAL_PSDU_RATE_MCS_3NSS_GF_SG_360_MBPS         21
#define HAL_PSDU_RATE_MCS_3NSS_GF_364_5_MBPS          22
#define HAL_PSDU_RATE_MCS_3NSS_GF_SG_405_MBPS         22
#define HAL_PSDU_RATE_MCS_3NSS_GF_405_MBPS            23
#define HAL_PSDU_RATE_MCS_3NSS_GF_SG_450_MBPS         23
#define HAL_PSDU_RATE_MCS_4NSS_GF_54_MBPS             24
#define HAL_PSDU_RATE_MCS_4NSS_GF_SG_60_MBPS          24
#define HAL_PSDU_RATE_MCS_4NSS_GF_108_MBPS            25
#define HAL_PSDU_RATE_MCS_4NSS_GF_SG_120_MBPS         25
#define HAL_PSDU_RATE_MCS_4NSS_GF_162_MBPS            26
#define HAL_PSDU_RATE_MCS_4NSS_GF_SG_180_MBPS         26
#define HAL_PSDU_RATE_MCS_4NSS_GF_216_MBPS            27
#define HAL_PSDU_RATE_MCS_4NSS_GF_SG_240_MBPS         27
#define HAL_PSDU_RATE_MCS_4NSS_GF_324_MBPS            28
#define HAL_PSDU_RATE_MCS_4NSS_GF_SG_360_MBPS         28
#define HAL_PSDU_RATE_MCS_4NSS_GF_432_MBPS            29
#define HAL_PSDU_RATE_MCS_4NSS_GF_SG_480_MBPS         29
#define HAL_PSDU_RATE_MCS_4NSS_GF_486_MBPS            30
#define HAL_PSDU_RATE_MCS_4NSS_GF_SG_540_MBPS         30
#define HAL_PSDU_RATE_MCS_4NSS_GF_540_MBPS            31
#define HAL_PSDU_RATE_MCS_4NSS_GF_SG_600_MBPS         31

//MCS Index #16-31 (20MHz) Mixed Mode
#define HAL_PSDU_RATE_MCS_3NSS_MM_19_5_MBPS           16
#define HAL_PSDU_RATE_MCS_3NSS_MM_SG_21_7_MBPS        16
#define HAL_PSDU_RATE_MCS_3NSS_MM_39_MBPS             17
#define HAL_PSDU_RATE_MCS_3NSS_MM_SG_43_3_MBPS        17
#define HAL_PSDU_RATE_MCS_3NSS_MM_58_5_MBPS           18
#define HAL_PSDU_RATE_MCS_3NSS_MM_SG_65_MBPS          18
#define HAL_PSDU_RATE_MCS_3NSS_MM_78_MBPS             19
#define HAL_PSDU_RATE_MCS_3NSS_MM_SG_86_7_MBPS        19
#define HAL_PSDU_RATE_MCS_3NSS_MM_117_MBPS            20
#define HAL_PSDU_RATE_MCS_3NSS_MM_SG_130_MBPS         20
#define HAL_PSDU_RATE_MCS_3NSS_MM_156_MBPS            21
#define HAL_PSDU_RATE_MCS_3NSS_MM_SG_173_3_MBPS       21
#define HAL_PSDU_RATE_MCS_3NSS_MM_175_5_MBPS          22
#define HAL_PSDU_RATE_MCS_3NSS_MM_SG_195_MBPS         22
#define HAL_PSDU_RATE_MCS_3NSS_MM_195_MBPS            23
#define HAL_PSDU_RATE_MCS_3NSS_MM_SG_216_7_MBPS       23
#define HAL_PSDU_RATE_MCS_4NSS_MM_26_MBPS             24
#define HAL_PSDU_RATE_MCS_4NSS_MM_SG_28_9_MBPS        24
#define HAL_PSDU_RATE_MCS_4NSS_MM_52_MBPS             25
#define HAL_PSDU_RATE_MCS_4NSS_MM_SG_57_8_MBPS        25
#define HAL_PSDU_RATE_MCS_4NSS_MM_78_MBPS             26
#define HAL_PSDU_RATE_MCS_4NSS_MM_SG_86_7_MBPS        26
#define HAL_PSDU_RATE_MCS_4NSS_MM_104_MBPS            27
#define HAL_PSDU_RATE_MCS_4NSS_MM_SG_115_6_MBPS       27
#define HAL_PSDU_RATE_MCS_4NSS_MM_156_MBPS            28
#define HAL_PSDU_RATE_MCS_4NSS_MM_SG_173_3_MBPS       28
#define HAL_PSDU_RATE_MCS_4NSS_MM_208_MBPS            29
#define HAL_PSDU_RATE_MCS_4NSS_MM_SG_231_1_MBPS       29
#define HAL_PSDU_RATE_MCS_4NSS_MM_234_MBPS            30
#define HAL_PSDU_RATE_MCS_4NSS_MM_SG_260_MBPS         30
#define HAL_PSDU_RATE_MCS_4NSS_MM_260_MBPS            31
#define HAL_PSDU_RATE_MCS_4NSS_MM_SG_288_9_MBPS       31

//MCS Index #16-31 (40MHz) Mixed Mode
#define HAL_PSDU_RATE_MCS_3NSS_MM_40_5_MBPS           16
#define HAL_PSDU_RATE_MCS_3NSS_MM_SG_45_MBPS          16
#define HAL_PSDU_RATE_MCS_3NSS_MM_81_MBPS             17
#define HAL_PSDU_RATE_MCS_3NSS_MM_SG_90_MBPS          17
#define HAL_PSDU_RATE_MCS_3NSS_MM_121_5_MBPS          18
#define HAL_PSDU_RATE_MCS_3NSS_MM_SG_135_MBPS         18
#define HAL_PSDU_RATE_MCS_3NSS_MM_162_MBPS            19
#define HAL_PSDU_RATE_MCS_3NSS_MM_SG_180_MBPS         19
#define HAL_PSDU_RATE_MCS_3NSS_MM_243_MBPS            20
#define HAL_PSDU_RATE_MCS_3NSS_MM_SG_270_MBPS         20
#define HAL_PSDU_RATE_MCS_3NSS_MM_324_MBPS            21
#define HAL_PSDU_RATE_MCS_3NSS_MM_SG_360_MBPS         21
#define HAL_PSDU_RATE_MCS_3NSS_MM_364_5_MBPS          22
#define HAL_PSDU_RATE_MCS_3NSS_MM_SG_405_MBPS         22
#define HAL_PSDU_RATE_MCS_3NSS_MM_405_MBPS            23
#define HAL_PSDU_RATE_MCS_3NSS_MM_SG_450_MBPS         23
#define HAL_PSDU_RATE_MCS_4NSS_MM_54_MBPS             24
#define HAL_PSDU_RATE_MCS_4NSS_MM_SG_60_MBPS          24
#define HAL_PSDU_RATE_MCS_4NSS_MM_108_MBPS            25
#define HAL_PSDU_RATE_MCS_4NSS_MM_SG_120_MBPS         25
#define HAL_PSDU_RATE_MCS_4NSS_MM_162_MBPS            26
#define HAL_PSDU_RATE_MCS_4NSS_MM_SG_180_MBPS         26
#define HAL_PSDU_RATE_MCS_4NSS_MM_216_MBPS            27
#define HAL_PSDU_RATE_MCS_4NSS_MM_SG_240_MBPS         27
#define HAL_PSDU_RATE_MCS_4NSS_MM_324_MBPS            28
#define HAL_PSDU_RATE_MCS_4NSS_MM_SG_360_MBPS         28
#define HAL_PSDU_RATE_MCS_4NSS_MM_432_MBPS            29
#define HAL_PSDU_RATE_MCS_4NSS_MM_SG_480_MBPS         29
#define HAL_PSDU_RATE_MCS_4NSS_MM_486_MBPS            30
#define HAL_PSDU_RATE_MCS_4NSS_MM_SG_540_MBPS         30
#define HAL_PSDU_RATE_MCS_4NSS_MM_540_MBPS            31
#define HAL_PSDU_RATE_MCS_4NSS_MM_SG_600_MBPS         31


//MCS Index #32  (40MHz duplicate mode, Nss           = 1)
#define HAL_PSDU_RATE_MCS_DUP_1NSS_6_MBPS             32
#define HAL_PSDU_RATE_MCS_DUP_1NSS_SG_6_7_MBPS        32

//MCS Index #0-15 STBC Rates (20MHz) Greenfield Mode
#define HAL_PSDU_RATE_MCS_1NSS_STBC_GF_6_5_MBPS            0
#define HAL_PSDU_RATE_MCS_1NSS_STBC_GF_SG_7_2_MBPS         0
#define HAL_PSDU_RATE_MCS_1NSS_STBC_GF_13_MBPS             1
#define HAL_PSDU_RATE_MCS_1NSS_STBC_GF_SG_14_4_MBPS        1
#define HAL_PSDU_RATE_MCS_1NSS_STBC_GF_19_5_MBPS           2
#define HAL_PSDU_RATE_MCS_1NSS_STBC_GF_SG_21_7_MBPS        2
#define HAL_PSDU_RATE_MCS_1NSS_STBC_GF_26_MBPS             3
#define HAL_PSDU_RATE_MCS_1NSS_STBC_GF_SG_28_9_MBPS        3
#define HAL_PSDU_RATE_MCS_1NSS_STBC_GF_39_MBPS             4
#define HAL_PSDU_RATE_MCS_1NSS_STBC_GF_SG_43_3_MBPS        4
#define HAL_PSDU_RATE_MCS_1NSS_STBC_GF_52_MBPS             5
#define HAL_PSDU_RATE_MCS_1NSS_STBC_GF_SG_57_8_MBPS        5
#define HAL_PSDU_RATE_MCS_1NSS_STBC_GF_58_5_MBPS           6
#define HAL_PSDU_RATE_MCS_1NSS_STBC_GF_SG_65_MBPS          6
#define HAL_PSDU_RATE_MCS_1NSS_STBC_GF_65_MBPS             7
#define HAL_PSDU_RATE_MCS_1NSS_STBC_GF_SG_72_2_MBPS        7

//MCS Index #0-15 STBC Rates (20MHz) Mixed Mode
#define HAL_PSDU_RATE_MCS_1NSS_STBC_MM_6_5_MBPS            0
#define HAL_PSDU_RATE_MCS_1NSS_STBC_MM_SG_7_2_MBPS         0
#define HAL_PSDU_RATE_MCS_1NSS_STBC_MM_13_MBPS             1
#define HAL_PSDU_RATE_MCS_1NSS_STBC_MM_SG_14_4_MBPS        1
#define HAL_PSDU_RATE_MCS_1NSS_STBC_MM_19_5_MBPS           2
#define HAL_PSDU_RATE_MCS_1NSS_STBC_MM_SG_21_7_MBPS        2
#define HAL_PSDU_RATE_MCS_1NSS_STBC_MM_26_MBPS             3
#define HAL_PSDU_RATE_MCS_1NSS_STBC_MM_SG_28_9_MBPS        3
#define HAL_PSDU_RATE_MCS_1NSS_STBC_MM_39_MBPS             4
#define HAL_PSDU_RATE_MCS_1NSS_STBC_MM_SG_43_3_MBPS        4
#define HAL_PSDU_RATE_MCS_1NSS_STBC_MM_52_MBPS             5
#define HAL_PSDU_RATE_MCS_1NSS_STBC_MM_SG_57_8_MBPS        5
#define HAL_PSDU_RATE_MCS_1NSS_STBC_MM_58_5_MBPS           6
#define HAL_PSDU_RATE_MCS_1NSS_STBC_MM_SG_65_MBPS          6
#define HAL_PSDU_RATE_MCS_1NSS_STBC_MM_65_MBPS             7
#define HAL_PSDU_RATE_MCS_1NSS_STBC_MM_SG_72_2_MBPS        7

//MCS Index #0-15 STBC Rates (40MHz) Greenfield Mode
#define HAL_PSDU_RATE_MCS_1NSS_STBC_GF_13_5_MBPS           0
#define HAL_PSDU_RATE_MCS_1NSS_STBC_GF_SG_15_MBPS          0
#define HAL_PSDU_RATE_MCS_1NSS_STBC_GF_27_MBPS             1
#define HAL_PSDU_RATE_MCS_1NSS_STBC_GF_SG_30_MBPS          1
#define HAL_PSDU_RATE_MCS_1NSS_STBC_GF_40_5_MBPS           2
#define HAL_PSDU_RATE_MCS_1NSS_STBC_GF_SG_45_MBPS          2
#define HAL_PSDU_RATE_MCS_1NSS_STBC_GF_54_MBPS             3
#define HAL_PSDU_RATE_MCS_1NSS_STBC_GF_SG_60_MBPS          3
#define HAL_PSDU_RATE_MCS_1NSS_STBC_GF_81_MBPS             4
#define HAL_PSDU_RATE_MCS_1NSS_STBC_GF_SG_90_MBPS          4
#define HAL_PSDU_RATE_MCS_1NSS_STBC_GF_108_MBPS            5
#define HAL_PSDU_RATE_MCS_1NSS_STBC_GF_SG_120_MBPS         5
#define HAL_PSDU_RATE_MCS_1NSS_STBC_GF_121_5_MBPS          6
#define HAL_PSDU_RATE_MCS_1NSS_STBC_GF_SG_135_MBPS         6
#define HAL_PSDU_RATE_MCS_1NSS_STBC_GF_135_MBPS            7
#define HAL_PSDU_RATE_MCS_1NSS_STBC_GF_SG_150_MBPS         7

//MCS Index #0-15 STBC Rates (40MHz) Mixed Mode
#define HAL_PSDU_RATE_MCS_1NSS_STBC_MM_13_5_MBPS           0
#define HAL_PSDU_RATE_MCS_1NSS_STBC_MM_SG_15_MBPS          0
#define HAL_PSDU_RATE_MCS_1NSS_STBC_MM_27_MBPS             1
#define HAL_PSDU_RATE_MCS_1NSS_STBC_MM_SG_30_MBPS          1
#define HAL_PSDU_RATE_MCS_1NSS_STBC_MM_40_5_MBPS           2
#define HAL_PSDU_RATE_MCS_1NSS_STBC_MM_SG_45_MBPS          2
#define HAL_PSDU_RATE_MCS_1NSS_STBC_MM_54_MBPS             3
#define HAL_PSDU_RATE_MCS_1NSS_STBC_MM_SG_60_MBPS          3
#define HAL_PSDU_RATE_MCS_1NSS_STBC_MM_81_MBPS             4
#define HAL_PSDU_RATE_MCS_1NSS_STBC_MM_SG_90_MBPS          4
#define HAL_PSDU_RATE_MCS_1NSS_STBC_MM_108_MBPS            5
#define HAL_PSDU_RATE_MCS_1NSS_STBC_MM_SG_120_MBPS         5
#define HAL_PSDU_RATE_MCS_1NSS_STBC_MM_121_5_MBPS          6
#define HAL_PSDU_RATE_MCS_1NSS_STBC_MM_SG_135_MBPS         6
#define HAL_PSDU_RATE_MCS_1NSS_STBC_MM_135_MBPS            7
#define HAL_PSDU_RATE_MCS_1NSS_STBC_MM_SG_150_MBPS         7

//Airgo Proprietary Rates 20Mhz Greenfield Mode
#define HAL_PSDU_RATE_ANI_GF_68_25_MBPS               35
#define HAL_PSDU_RATE_ANI_GF_SG_75_83_MBPS            35
#define HAL_PSDU_RATE_ANI_GF_136_5_MBPS               50
#define HAL_PSDU_RATE_ANI_GF_SG_151_7_MBPS            50
#define HAL_PSDU_RATE_ANI_GF_204_75_MBPS              74
#define HAL_PSDU_RATE_ANI_GF_SG_227_5_MBPS            74
#define HAL_PSDU_RATE_ANI_GF_273_MBPS                 67
#define HAL_PSDU_RATE_ANI_GF_SG_303_33_MBPS           67

//Airgo Proprietary STBC Rates 20Mhz Greenfield Mode
#define HAL_PSDU_RATE_ANI_STBC_GF_68_25_MBPS          35

//Airgo Proprietary Rates 40Mhz Greenfield Mode
#define HAL_PSDU_RATE_ANI_GF_DUP_141_75_MBPS          35
#define HAL_PSDU_RATE_ANI_GF_DUP_SG_157_5_MBPS        35
#define HAL_PSDU_RATE_ANI_GF_DUP_283_5_MBPS           50
#define HAL_PSDU_RATE_ANI_GF_DUP_SG_315_MBPS          50
#define HAL_PSDU_RATE_ANI_GF_DUP_425_25_MBPS          74
#define HAL_PSDU_RATE_ANI_GF_DUP_SG_472_5_MBPS        74
#define HAL_PSDU_RATE_ANI_GF_DUP_567_MBPS             67
#define HAL_PSDU_RATE_ANI_GF_DUP_SG_630_MBPS          67

//Airgo Proprietary STBC Rates 40Mhz Greenfield Mode
#define HAL_PSDU_RATE_ANI_STBC_GF_DUP_141_75_MBPS     35

//802.11b Rates
#define HAL_PSDU_RATE_11B_LONG_1_MBPS                 0
#define HAL_PSDU_RATE_11B_LONG_2_MBPS                 1
#define HAL_PSDU_RATE_11B_SHORT_2_MBPS                1
#define HAL_PSDU_RATE_11B_LONG_5_5_MBPS               2
#define HAL_PSDU_RATE_11B_SHORT_5_5_MBPS              2
#define HAL_PSDU_RATE_11B_LONG_11_MBPS                3
#define HAL_PSDU_RATE_11B_SHORT_11_MBPS               3

//SLR Rates
#define HAL_PSDU_RATE_SLR_0_25_MBPS                   0
#define HAL_PSDU_RATE_SLR_0_5_MBPS                    1

/**************************************************************************
* NDBPS, 4Times Rate definitions
***************************************************************************/

//Spica_Virgo 11A Rates                               //4 Times rate
#define HAL_PSDU_RATE_11A_4T_48_MBPS                  192
#define HAL_PSDU_RATE_11A_4T_24_MBPS                  96
#define HAL_PSDU_RATE_11A_4T_12_MBPS                  48
#define HAL_PSDU_RATE_11A_4T_6_MBPS                   24
#define HAL_PSDU_RATE_11A_4T_54_MBPS                  216
#define HAL_PSDU_RATE_11A_4T_36_MBPS                  144
#define HAL_PSDU_RATE_11A_4T_18_MBPS                  72
#define HAL_PSDU_RATE_11A_4T_9_MBPS                   36

//MCS Index #0-31 (20MHz)                             //NDBPS
#define HAL_PSDU_RATE_20MHZ_MCS_IND_0_NDBPS           26
#define HAL_PSDU_RATE_20MHZ_MCS_IND_1_NDBPS           52
#define HAL_PSDU_RATE_20MHZ_MCS_IND_2_NDBPS           78
#define HAL_PSDU_RATE_20MHZ_MCS_IND_3_NDBPS           104
#define HAL_PSDU_RATE_20MHZ_MCS_IND_4_NDBPS           156
#define HAL_PSDU_RATE_20MHZ_MCS_IND_5_NDBPS           208
#define HAL_PSDU_RATE_20MHZ_MCS_IND_6_NDBPS           234
#define HAL_PSDU_RATE_20MHZ_MCS_IND_7_NDBPS           260
#define HAL_PSDU_RATE_20MHZ_MCS_IND_8_NDBPS           52
#define HAL_PSDU_RATE_20MHZ_MCS_IND_9_NDBPS           104
#define HAL_PSDU_RATE_20MHZ_MCS_IND_10_NDBPS          156
#define HAL_PSDU_RATE_20MHZ_MCS_IND_11_NDBPS          208
#define HAL_PSDU_RATE_20MHZ_MCS_IND_12_NDBPS          312
#define HAL_PSDU_RATE_20MHZ_MCS_IND_13_NDBPS          416
#define HAL_PSDU_RATE_20MHZ_MCS_IND_14_NDBPS          468
#define HAL_PSDU_RATE_20MHZ_MCS_IND_15_NDBPS          520
#define HAL_PSDU_RATE_20MHZ_MCS_IND_16_NDBPS          78
#define HAL_PSDU_RATE_20MHZ_MCS_IND_17_NDBPS          156
#define HAL_PSDU_RATE_20MHZ_MCS_IND_18_NDBPS          234
#define HAL_PSDU_RATE_20MHZ_MCS_IND_19_NDBPS          312
#define HAL_PSDU_RATE_20MHZ_MCS_IND_20_NDBPS          468
#define HAL_PSDU_RATE_20MHZ_MCS_IND_21_NDBPS          624
#define HAL_PSDU_RATE_20MHZ_MCS_IND_22_NDBPS          702
#define HAL_PSDU_RATE_20MHZ_MCS_IND_23_NDBPS          780
#define HAL_PSDU_RATE_20MHZ_MCS_IND_24_NDBPS          104
#define HAL_PSDU_RATE_20MHZ_MCS_IND_25_NDBPS          208
#define HAL_PSDU_RATE_20MHZ_MCS_IND_26_NDBPS          312
#define HAL_PSDU_RATE_20MHZ_MCS_IND_27_NDBPS          416
#define HAL_PSDU_RATE_20MHZ_MCS_IND_28_NDBPS          624
#define HAL_PSDU_RATE_20MHZ_MCS_IND_29_NDBPS          832
#define HAL_PSDU_RATE_20MHZ_MCS_IND_30_NDBPS          936
#define HAL_PSDU_RATE_20MHZ_MCS_IND_31_NDBPS          1040

//MCS Index #0-31 (40MHz)                             /NDBPS
#define HAL_PSDU_RATE_40MHZ_MCS_IND_0_NDBPS           54
#define HAL_PSDU_RATE_40MHZ_MCS_IND_1_NDBPS           108
#define HAL_PSDU_RATE_40MHZ_MCS_IND_2_NDBPS           162
#define HAL_PSDU_RATE_40MHZ_MCS_IND_3_NDBPS           216
#define HAL_PSDU_RATE_40MHZ_MCS_IND_4_NDBPS           324
#define HAL_PSDU_RATE_40MHZ_MCS_IND_5_NDBPS           432
#define HAL_PSDU_RATE_40MHZ_MCS_IND_6_NDBPS           486
#define HAL_PSDU_RATE_40MHZ_MCS_IND_7_NDBPS           540
#define HAL_PSDU_RATE_40MHZ_MCS_IND_8_NDBPS           108
#define HAL_PSDU_RATE_40MHZ_MCS_IND_9_NDBPS           216
#define HAL_PSDU_RATE_40MHZ_MCS_IND_10_NDBPS          324
#define HAL_PSDU_RATE_40MHZ_MCS_IND_11_NDBPS          432
#define HAL_PSDU_RATE_40MHZ_MCS_IND_12_NDBPS          648
#define HAL_PSDU_RATE_40MHZ_MCS_IND_13_NDBPS          864
#define HAL_PSDU_RATE_40MHZ_MCS_IND_14_NDBPS          972
#define HAL_PSDU_RATE_40MHZ_MCS_IND_15_NDBPS          1080
#define HAL_PSDU_RATE_40MHZ_MCS_IND_16_NDBPS          162
#define HAL_PSDU_RATE_40MHZ_MCS_IND_17_NDBPS          324
#define HAL_PSDU_RATE_40MHZ_MCS_IND_18_NDBPS          486
#define HAL_PSDU_RATE_40MHZ_MCS_IND_19_NDBPS          648
#define HAL_PSDU_RATE_40MHZ_MCS_IND_20_NDBPS          972
#define HAL_PSDU_RATE_40MHZ_MCS_IND_21_NDBPS          1296
#define HAL_PSDU_RATE_40MHZ_MCS_IND_22_NDBPS          1458
#define HAL_PSDU_RATE_40MHZ_MCS_IND_23_NDBPS          1620
#define HAL_PSDU_RATE_40MHZ_MCS_IND_24_NDBPS          216
#define HAL_PSDU_RATE_40MHZ_MCS_IND_25_NDBPS          432
#define HAL_PSDU_RATE_40MHZ_MCS_IND_26_NDBPS          648
#define HAL_PSDU_RATE_40MHZ_MCS_IND_27_NDBPS          864
#define HAL_PSDU_RATE_40MHZ_MCS_IND_28_NDBPS          1296
#define HAL_PSDU_RATE_40MHZ_MCS_IND_29_NDBPS          1728
#define HAL_PSDU_RATE_40MHZ_MCS_IND_30_NDBPS          1944
#define HAL_PSDU_RATE_40MHZ_MCS_IND_31_NDBPS          2160

//MCS Index #32  (40MHz duplicate mode, Nss           = 1)
#define HAL_PSDU_RATE_40MHZ_MCS_IND_32_NDBPS            24

//Airgo Proprietary Rates 20Mhz                     //4 Times rate
#define HAL_PSDU_RATE_ANI_4T_68_25_MBPS              273
#define HAL_PSDU_RATE_ANI_4T_136_5_MBPS              546
#define HAL_PSDU_RATE_ANI_4T_204_75_MBPS             819
#define HAL_PSDU_RATE_ANI_4T_273_MBPS                1092

//Airgo Proprietary Rates 40Mhz
#define HAL_PSDU_RATE_ANI_4T_141_75_MBPS             567
#define HAL_PSDU_RATE_ANI_4T_283_5_MBPS              1134
#define HAL_PSDU_RATE_ANI_4T_425_25_MBPS             1701
#define HAL_PSDU_RATE_ANI_4T_567_MBPS                2268

//802.11b Rates
#define HAL_PSDU_RATE_11B_4T_1_MBPS                  4
#define HAL_PSDU_RATE_11B_4T_2_MBPS                  8
#define HAL_PSDU_RATE_11B_4T_5_5_MBPS                22
#define HAL_PSDU_RATE_11B_4T_11_MBPS                 44

//SLR Rates
#define HAL_PSDU_RATE_SLR_4T_0_25_MBPS               1
#define HAL_PSDU_RATE_SLR_4T_0_5_MBPS                2


#define MIN_LIBRA_RATE_NUM 0
#define MAX_LIBRA_TX_RATE_NUM     TPE_RT_IDX_MAX_TX_RATES 
#define MAX_LIBRA_RATE_NUM        TPE_RT_IDX_MAX_RATES

#define HAL_RATETABLE_RATEINDEX_BEGIN   0
#define HAL_RATETABLE_RATEINDEX_END     (MAX_LIBRA_RATE_NUM - 1)

// index must always be an unsigned number
#define HAL_RATETABLE_IS_RATEINDEX_VALID(index)    ((index) <= HAL_RATETABLE_RATEINDEX_END)

// Max RTS threshold
#define HAL_RTS_THRESHOLD_MAX       WNI_CFG_RTS_THRESHOLD_STAMAX

// BYTE 0-3
#define  PKTTYPE(x)      	(x & 0x3)            // bit[0:1]
#define  PSDURATE(x)     	((x & 0x7F) << 2)    // bit[2:8]
#define  BANDWIDTH(x)    	((x & 0x3) << 9)     // bit[9:10]
#define  NS11b(x)        	((x & 0x3) << 11)    // bit[11:12]
#define  SHORTGUARD(x)    	(x << 13)            // bit[13]
#define  CTRLRATEIDX(x)  	((x & 0x1FF) << 14)  // bit[14:22]
#define  TXANTENNA(x)    	((x & 0x7) << 23)    // bit[23:25]
#define  NES(x)        	    (x << 26)            // bit[26]
#define  TXPOWER(x)     	((x & 0x1F) << 27)   // bit[27:31]

// BYTE 4-7
#define  RSPRATEIDX(x)   	(x & 0x1FF)          // bit[0:8]
#define  AMPDU(x)        	(x << 9)             // bit[9]
#define  NDBPS4TRATE(x)  	((x & 0xFFF) << 10)  // bit[10:21]
#define  NDLTFS(x)  	    ((x & 0x3) << 22)    // bit[22:23]
#define  NELTFS(x)  	    ((x & 0x3) << 24)    // bit[24:25]
#define  PCTRLRATEIDX(x)    ((x & 0xF) << 26)    // bit[26:29]
#define  STBC(x)            ((x & 0x3) << 30)    // bit[30:31]

#define HAL_RA_SENSITIVITY_GET(halRate) gHalRateInfo[(halRate)].sensitivity
#define HAL_RA_THRUPUT_GET(halRate)     gHalRateInfo[(halRate)].thruputKbps
#define HAL_RA_ACTUALTPUT_GET(halRate)  gHalRateInfo[(halRate)].actualTputKbps
#define HAL_RA_IERATEMCSIDX_GET(halRate)gHalRateInfo[(halRate)].ieRateMcsIdx
#define HAL_RA_TPERATEIDX_GET(halRate) gHalRateInfo[(halRate)].tpeRateIdx

#define HALRATE_IS_BELOW_MAX(halRate, maxRate) ((maxRate>0)?((HAL_RA_THRUPUT_GET(halRate)<=(maxRate*10))?1:0):1)

#define HALRATE_IS_DISABLED(halRate)  ((gHalRateInfo[(halRate)].rateProperty & RA_DISABLED))
#define HALRATE_IS_MIMO(halRate)      ((gHalRateInfo[(halRate)].rateProperty & RA_MIMO)==(RA_MIMO))
#define HALRATE_IS_HT_MIMO(halRate)   ((gHalRateInfo[(halRate)].rateProperty & (RA_MIMO|RA_HT))==(RA_MIMO|RA_HT))
#define HALRATE_IS_HT(halRate)        ((gHalRateInfo[(halRate)].rateProperty & (RA_HT))==(RA_HT))
#define HALRATE_IS_CB(halRate)        ((gHalRateInfo[(halRate)].rateProperty & (RA_40MHZ))==(RA_40MHZ))
#define HALRATE_IS_HT_LGI(halRate)    ((gHalRateInfo[(halRate)].rateProperty & (RA_SGI|RA_HT))==(RA_HT))
#define HALRATE_IS_HT_SGI(halRate)    ((gHalRateInfo[(halRate)].rateProperty & (RA_SGI|RA_HT))==(RA_SGI|RA_HT))
#define HALRATE_IS_11B(halRate)       ((gHalRateInfo[(halRate)].rateProperty & (RA_CCKDSSS))==(RA_CCKDSSS))    
#define HALRATE_IS_11B_SHORT_PREAM(halRate)   ((gHalRateInfo[(halRate)].rateProperty & (RA_CCKDSSS|RA_SPREAMBLE))==(RA_CCKDSSS|RA_SPREAMBLE))
#define HALRATE_IS_DUP(halRate)       ((gHalRateInfo[(halRate)].rateProperty & (RA_DUPLICATE))==(RA_DUPLICATE))
#define HALRATE_IS_11BG(halRate)      ((gHalRateInfo[(halRate)].rateProperty & (RA_11AG_RATES|RA_CCKDSSS)) !=0 )
#define HALRATE_IS_11AG(halRate)      (gHalRateInfo[(halRate)].rateProperty & (RA_11AG_RATES))
#define HALRATE_IS_SLR(halRate)		  ((gHalRateInfo[(halRate)].rateProperty & RA_SLR))


/* Qualcomm proprietary */
#define WLAN_DUR_TITAN_ESCORT               /* (TODO) Duration of escort packet */
#define WLAN_DUR_TITAN20_STF            8
#define WLAN_DUR_TITAN20_LTF            8
#define WLAN_DUR_TITAN20_SIGNAL         4
#define WLAN_DUR_TITAN20_ESF            4   /* ESF for concatenation */

#define WLAN_DUR_TITAN40_STF            6   /* Actually 5.2 us. ceil(5.2 us) = 6 us */
#define WLAN_DUR_TITAN40_LTF            4
#define WLAN_DUR_TITAN40_SIGNAL         2
#define WLAN_DUR_TITAN40_ESF            2   /* ESF for concatenation */

/* Airgo proprietary */
#define WLAN_PHYHEADER_TITAN20          (WLAN_DUR_TITAN20_STF + WLAN_DUR_TITAN20_LTF + WLAN_DUR_TITAN20_SIGNAL)
#define WLAN_PHYHEADER_TITAN40          (WLAN_DUR_TITAN40_STF + WLAN_DUR_TITAN40_LTF + WLAN_DUR_TITAN40_SIGNAL)


/* 11n */
#define WLAN_DUR_11N_LEGACY_STF         8   /* L-STF */
#define WLAN_DUR_11N_LEGACY_LTF         8   /* L-LTF */
#define WLAN_DUR_11N_LEGACY_SIGNAL      4   /* L-SIG */

#define WLAN_DUR_11N_HT_STF             4   /* HT-STF */
#define WLAN_DUR_11N_HT_LTF             4   /* HT-LTF */
#define WLAN_DUR_11N_HT_SIGNAL          8   /* HT-SIG */
#define WLAN_DUR_11N_HT_LTF1_GREENFIELD 8   /* HT-LTF1 */


#define WLAN_PHYHEADER_11N_MIXEDMODE(spatial_stream) ( \
    WLAN_DUR_11N_LEGACY_STF + WLAN_DUR_11N_LEGACY_LTF + WLAN_DUR_11N_LEGACY_SIGNAL + \
    WLAN_DUR_11N_HT_SIGNAL +  WLAN_DUR_11N_HT_STF + WLAN_DUR_11N_HT_LTF * (spatial_stream))

#define WLAN_PHYHEADER_11N_GREENFIELD(spatial_stream) ( \
    WLAN_DUR_11N_LEGACY_STF + \
    WLAN_DUR_11N_HT_LTF1_GREENFIELD + WLAN_DUR_11N_HT_SIGNAL + WLAN_DUR_11N_HT_LTF * ((spatial_stream) - 1))

#define HAL_PHYHEADER_11b_LONG_DURATION     192
#define HAL_PHYHEADER_11b_SHORT_DURATION    96


/* ------------------------------------
 * Response Rate Index Byte Definition
 * ------------------------------------ 
 */
#define  BAINDEX(x)     (x & 0xFF) << 24   // bit[31:24]
#define  ACKINDEX(x)    (x & 0xFF) << 16   // bit[23:16]
#define  CTSINDEX(x)    (x & 0xFF) << 8    // bit[15:8]
#define  RSVDINDEX(x)   (x & 0xFF)         // bit[7:0]

#define  GETBAINDEX(x)     ((x) >> 24)   
#define  GETACKINDEX(x)    (((x) >> 16) & 0xff )
#define  GETCTSINDEX(x)    (((x) >> 8) & 0xff )



#define RESPONSE_RATE_INDEX(ba,ack,cts,rsv)    \
           (BAINDEX(ba) | ACKINDEX(ack) | CTSINDEX(cts) | RSVDINDEX(rsv))


#define MAX_FIXED_RATE_NUM     TPE_RT_IDX_MAX_RATES

// index must always be an unsigned number
#define HAL_RATETABLE_IS_RATEINDEX_VALID(index)    ((index) <= HAL_RATETABLE_RATEINDEX_END)


#define HAL_PROTINFO_NOCHG       -1

#define HAL_RATE_TABLE_RETRY_CNT   5
#define HAL_SEND_MSG_SLEEP_COUNT   2000000


#define TPE_RT_IDX_11B_LONG_1_MBPS_OFFSET   (TPE_RT_IDX_11B_LONG_1_MBPS-TPE_RT_IDX_11B_RATE_LONG_PR_BASE_OFFSET)
#define TPE_RT_IDX_11B_LONG_2_MBPS_OFFSET   (TPE_RT_IDX_11B_LONG_2_MBPS-TPE_RT_IDX_11B_RATE_LONG_PR_BASE_OFFSET)
#define TPE_RT_IDX_11B_LONG_5_5_MBPS_OFFSET (TPE_RT_IDX_11B_LONG_5_5_MBPS-TPE_RT_IDX_11B_RATE_LONG_PR_BASE_OFFSET)
#define TPE_RT_IDX_11B_LONG_11_MBPS_OFFSET  (TPE_RT_IDX_11B_LONG_11_MBPS-TPE_RT_IDX_11B_RATE_LONG_PR_BASE_OFFSET)

#define TPE_RT_IDX_11B_SHORT_2_MBPS_OFFSET   (TPE_RT_IDX_11B_SHORT_2_MBPS-TPE_RT_IDX_11B_RATE_SHORT_PR_BASE_OFFSET)
#define TPE_RT_IDX_11B_SHORT_5_5_MBPS_OFFSET (TPE_RT_IDX_11B_SHORT_5_5_MBPS-TPE_RT_IDX_11B_RATE_SHORT_PR_BASE_OFFSET)
#define TPE_RT_IDX_11B_SHORT_11_MBPS_OFFSET  (TPE_RT_IDX_11B_SHORT_11_MBPS-TPE_RT_IDX_11B_RATE_SHORT_PR_BASE_OFFSET)

enum {
    PHY_BANDWIDTH_20MHZ = 0,
    PHY_BANDWIDTH_40MHZ,
    PHY_BANDWIDTH_40MHZ_DUPLICATE,
    PHY_BANDWIDTH_RSVD
};


enum {
    PHY_NSS_1SPATIAL = 0,
    PHY_NSS_2SPATIAL,
    PHY_NSS_3SPATIAL,
    PHY_NSS_4SPATIAL
};

enum {
	PR_SHORT = 0,
	PR_LONG,
	SLR,
	PR_RSV
};

enum {
	SG_DISABLED = 0,
	SG_ENABLED
};

enum {
	AMPDU_INV = 0,
	AMPDU_VALID
};

enum {
	STBC_INV = 0,
	STBC_VALID
};

enum {
	NDLTF_ZERO = 0,
	NDLTF_ONE  = 1,
	NDLTF_TWO  = 2,
	NDLTF_FOUR = 3
};

enum {
	NELTF_ZERO = 0,
	NELTF_ONE  = 1,
	NELTF_TWO  = 2,
	NELTF_FOUR = 3
};
#ifdef WLAN_HAL_VOLANS
//TPE SRAM rate table programmable data. Just to keep the size.
typedef struct sTpeSramRateTableDynamic {
#ifdef ANI_BIG_BYTE_ENDIAN    
    tANI_U32    rsvd:15;
    tANI_U32    data:17;
#else
    tANI_U32    data:17;
    tANI_U32    rsvd:15;
#endif
} tTpeSramRateTableDynamic, *tpTpeSramRateTableDynamic;


//Tpe SRAM rate table programmable data fields.
typedef struct sTpeRateTableDynamicData {
#ifdef ANI_BIG_BYTE_ENDIAN
        tANI_U32    rsvd:15;
        tANI_U32    protCntrlRateIdx:4;
        tANI_U32    rspRateIdx:4;
        tANI_U32    cntrlRspTxPwr:5;
        tANI_U32    cntrlRateIdx:4;        
#else
        tANI_U32    cntrlRateIdx:4;        
        tANI_U32    cntrlRspTxPwr:5;
        tANI_U32    rspRateIdx:4;
        tANI_U32    protCntrlRateIdx:4;
        tANI_U32    rsvd:15;
#endif
} tTpeRateTableDynamicData, *tpTpeRateTableDynamicData;
#endif

//TPE SRAM rate table complete. includes programmanle as well fixed parts. just to keep the size.

#ifdef WLAN_HAL_VOLANS
typedef struct sHalRateTable {
    tTpeRateTableDynamicData tpeRateTableDynamic; //This data structure keeps copy of the fields from rate table that are modifiable.
                                       //This should always be in sync with tpeRateTable data structure.
    tANI_U8         validEntry;
    tANI_U8    		stbcValid;
    tANI_U8    		txAntEnable;
#ifndef FEATURE_TX_PWR_CONTROL	
    tANI_U8    		txPwr;
#endif	
} tHalRateTable, *tpHalRateTable;

#else

typedef struct sHalRateTable {
    tTpeRateTable	tpeRateTable;
    tANI_U8         validEntry;
    tANI_U8    		stbcValid;
    tANI_U8    		txAntEnable;
    tANI_U8    		txPwr;
} tHalRateTable, *tpHalRateTable;
#endif


extern tHalRateInfo  gHalRateInfo[HAL_MAC_MAX_RATES];

#define HAL_MACRATE_2_MACRATEINFO(halRate)  ((tpHalRateInfo)  &gHalRateInfo[ (halRate) ])

eHalStatus halRate_Start(tHalHandle hHal, void *arg);
eHalStatus halRate_UpdateRateTablePower(tpAniSirGlobal pMac, tTpeRateIdx startRateIdx, tTpeRateIdx endRateIdx, tANI_U8 updateTpeHw);
eHalStatus halRate_getTpeRateCmd(tpAniSirGlobal pMac, tANI_U8 *mpiCmd, tANI_U32 rateIndex);
eHalStatus halRate_sendResponseRateTable(tpAniSirGlobal pMac);
eHalStatus halRate_UpdateTpeRateTable(tpAniSirGlobal pMac, tANI_U16 startRate, tANI_U16 endRate);
eHalStatus halRate_UpdateTpeTxPowerRateEntry(tpAniSirGlobal pMac, tANI_U32 rateIdx);
tHalMacRate halRate_tpeRate2HalRate(tANI_U32 tpeRate);
eHalStatus halRate_updateResponseRateTable(tpAniSirGlobal pMac);
eHalStatus halRate_getRateProperty(tpAniSirGlobal pMac, tHalMacRate halRate, tANI_U32 *rateProperty);
eHalStatus halRate_getPowerIndex(tpAniSirGlobal pMac, tANI_U32 rateIndex, tPwrTemplateIndex *pwrIndex);
eHalStatus halRate_getPacketTypeFromHalRate( tHalMacRate halRate, tANI_U8 *pPacketType );
void halRate_UpdateRateTxPower(tpAniSirGlobal pMac, tTpeRateIdx rateIndex, tPwrTemplateIndex power);
void halRate_SetTpeTxPower(tpAniSirGlobal pMac, tTpeRateIdx rateIndex, tPwrTemplateIndex power);
tTpeRateIdx halRate_halRate2TpeRate(tHalMacRate halRate);
eHalStatus halRate_updateResponseRateTableByBssBasicRate(tpAniSirGlobal pMac, tpSirSupportedRates pSuppRates);
eHalStatus halRate_enableTpeRate(tpAniSirGlobal hHal,  tANI_U32 enable, tANI_U32 tpeRateIdx);
eHalStatus halRate_changeTpeRateMaxActualTput(tpAniSirGlobal hHal, tANI_U32 maxTput, tANI_U32 tpeRateIdx);
tTpeRateIdx halRate_cfgFixedRate2TpeRate(tANI_U32 userCfgFixedrate);
void  halRateDbg_change11nRatePktType(tpAniSirGlobal pMac,tANI_U32 mixedMode, tANI_U32 startRate,  tANI_U32 endRateIdx);
eHalStatus halRate_BckupTpeRateTable(tpAniSirGlobal pMac, tANI_U32 *memAddr);
tANI_U32 halRate_GetMpduSpaceByTpeRate(tANI_U32 tpeRateIndex, tANI_U32 minimumMdpuStartSpacingParam);

/* Function to get the CFG set protection mode and if AUTO, then decide
 * which protection mode is suitable */
void halRate_getProtectionInfo(tpAniSirGlobal pMac, tANI_U32 staId,
        tANI_U32 type, tANI_U32 retry, tANI_U32 halDataRateIdx, 
        tANI_U32 *pProtPolicy);
void halRate_UpdateCtrlRspTxPower(tpAniSirGlobal pMac, tTpeRateIdx rateIdx, 
        tPwrTemplateIndex txPower, tANI_U8 updateTpeHw);
void halRate_GetTxPwrForRate(tpAniSirGlobal pMac, tTpeRateIdx rateIdx,
        tANI_S8 regPowerLimit, tPwrTemplateIndex  *pTxPower);
void halRate_DumpRateTxPower(tpAniSirGlobal pMac);
eHalStatus halRate_halRateInfoTableToFW(tpAniSirGlobal pMac, int startIndex, int endIndex);
eHalStatus halRate_TxPwrIndexToFW(tpAniSirGlobal pMac, int startIndex, int endIndex);
eHalPhyRates halRate_MacRateIdxtoPhyRateIdx(tpAniSirGlobal pMac, tTpeRateIdx tpeRateIdx);
#endif /* _HALRATETABLE_H_ */








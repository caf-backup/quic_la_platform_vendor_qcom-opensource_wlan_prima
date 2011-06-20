/** ------------------------------------------------------------------------- *
    ------------------------------------------------------------------------- *


    \file asicPhyDbg.c

   \brief phyDbg module to generate transmit frames,

    $Id$

    Copyright (C) 2006 Airgo Networks, Incorporated


   ========================================================================== */

/** baseline code is from NOVA branch, Debug Header changes,
 *  MPI header changes and rf warmup time is in PHYDBG Reg
 *  instead of the header. */

#include "ani_assert.h"
#include "sys_api.h"
#include "asicPhyDbg.h"
#include "mpdu.h"
#include <halPhyUtil.h>


#ifndef WLAN_FTM_STUB

typedef enum
{
    MPI_PKT_TYPE_11A = 0,               // 2b00 = 11a
    MPI_PKT_TYPE_11B = 1,               // 2b01 = 11b
    MPI_PKT_TYPE_GREENFIELD = 2,        // 2b10 = GreenField
    MPI_PKT_TYPE_MIXED_11A = 3,          // 2b11 = Mixed Mode
    MPI_PKT_TYPE_MAX_VAL    = 0xFFFFFFFF, //dummy val to set enum to 4 bytes
}ePhyDbgMpiPacketType;

typedef enum
{
    MPI_BANDWIDTH_20 = 0,               // 2b00 = 20 MHz mode
    MPI_BANDWIDTH_40 = 1,               // 2b01 = 40 MHz channel-bonding mode
    MPI_BANDWIDTH_40_DUP = 2,            // 2b10 = 40 MHz duplicate mode - this refers to legacy duplicate if
    MPI_BANDWIDTH_MAX_VAL   = 0xFFFFFFFF, //dummy val to set enum to 4 bytes

}ePhyDbgMpiBandwidth;                   //          packet_type is 11a; or HT-duplicate if packet_type is 11n.
                                        //          The psdu_rate must be set to 7'h20 for HT-duplicate packets
                                        // 2b11 = reserve

typedef enum
{                                       // actual field name is NSS/11b_mode, shared between 11a and 11b packets
    MPI_NSS_1 = 0,                      // 11n packets                      11b packets
    MPI_NSS_2 = 1,                      // 2b00 = 1 spatial stream          2b00 = 11b short preamble
    MPI_NSS_3 = 2,
    MPI_NSS_4 = 3,                      //
    MPI_11B_SHORT = 0,                  // 2b01 = 2 spatial stream          2b01 = 11b long preamble
    MPI_11B_LONG = 1,                   // 2b10 = 3 spatial stream          2b10 = SLR mode
    MPI_SLR_MODE = 2,                    // 2b11 = 4 spatial stream          2b11 = reserved
    MPI_NSS_MAX_VAL     = 0xFFFFFFFF, //dummy val to set enum to 4 bytes
}ePhyDbgMpiNss;

typedef enum
{
    MPI_11B_RATE_1_MBPS = 0,             // 2b00 = 1Mbps
    MPI_11B_RATE_2_MBPS = 1,             // 2b01 = 2Mbps
    MPI_11B_RATE_5_5_MBPS = 2,           // 2b10 = 5.5Mbps
    MPI_11B_RATE_11_MBPS = 3,            // 2b11 = 11Mbps
    MPI_11B_RATE_0_25_MBPS = 0,
    MPI_11B_RATE_0_5_MBPS = 1,          //
    MPI_11B_DONT_CARE = 0,               // 2b00 = don't care if packet_type is not 11b
    MPI_11B_MAX_VAL     = 0xFFFFFFFF, //dummy val to set enum to 4 bytes
}ePhyDbgMpi11bRate;

typedef enum
{
    MPI_NORMAL_800 = 0,
    MPI_SHORT_400 = 1,
    MPI_GUARD_DONT_CARE = 0,
    MPI_GUARD_MAX_VAL   = 0xFFFFFFFF, //dummy val to set enum to 4 bytes
}ePhyDbgMpiShortGuard;


typedef enum
{
    MPI_SUBBAND_MODE_PRIMARY = 0,
    MPI_SUBBAND_MODE_SECONDARY = 1,
    MPI_SUBBAND_MAX_VAL     = 0xFFFFFFFF, //dummy val to set enum to 4 bytes
}ePhyDbgMpiSubBand;

typedef enum
{
    MPI_PLCP_OVERRIDE_MASK_SMOOTHING = 0,
    MPI_PLCP_OVERRIDE_MASK_NOSOUNDING = 1,
    MPI_PLCP_OVERRIDE_MASK_RESERVED_BIT = 2,
    MPI_PLCP_OVERRIDE_MASK_NESS = 3,
    MPI_PLCP_OVERRIDE_MASK_MAX_VAL      = 0xFFFFFFFF, //dummy val to set enum to 4 bytes
}ePhyDbgMpiPlcpOverrideMask;

#ifdef FIXME_RE_DEFINITION
typedef enum
{
    MPI_PLCP_STBC_NONE = 0,
    MPI_PLCP_STBC_SIMO = 1,
    MPI_PLCP_STBC_MAX_VAL   = 0xFFFFFFFF, //dummy val to set enum to 4 bytes
}ePhyDbgMpiPlcpOverrideMask;
#endif

typedef enum
{
    MPI_PLCP_PPDU_RATE_CODE_48MBPS = 0x8,
    MPI_PLCP_PPDU_RATE_CODE_24BPS =  0x9,
    MPI_PLCP_PPDU_RATE_CODE_12MBPS = 0xA,
    MPI_PLCP_PPDU_RATE_CODE_6MBPS =  0xB,
    MPI_PLCP_PPDU_RATE_CODE_54MBPS = 0xC,
    MPI_PLCP_PPDU_RATE_CODE_36MBPS = 0xD,
    MPI_PLCP_PPDU_RATE_CODE_18MBPS = 0xE,
    MPI_PLCP_PPDU_RATE_CODE_9MBPS =  0xF,
    MPI_PLCP_PPDU_RATE_CODE_DONT_CARE = 0xB,
    MPI_PLCP_PDPU_RATE_CODE_MAX_VAL     = 0xFFFFFFFF, //dummy val to set enum to 4 bytes
}ePhyDbgMpiPpduRateCode;

typedef struct
{
    ePhyDbgMpiBandwidth bandwidth;
    ePhyDbgMpiNss nss11b;
    ePhyDbgMpi11bRate bRateSelect;
    ePhyDbgMpiShortGuard guard;
    tANI_U8 signalCode;
}sPhyDbgMpiRateSettings;

const sPhyDbgMpiRateSettings rateSettings[NUM_HAL_PHY_RATES] =
{
    //  ePhyDbgMpiBandwidth(1bit), ePhyDbgMpiNss(2bits), ePhyDbgMpi11bRate (2bit), ePhyDbgMpiShortGuard(1bit), signalCode(4bits)
    //802.11b Rates                                                                         //802.11b Rates
    {MPI_BANDWIDTH_20, MPI_11B_LONG, MPI_11B_RATE_1_MBPS,   MPI_GUARD_DONT_CARE, 0X0},      // HAL_PHY_RATE_11B_LONG_1_MBPS,
    {MPI_BANDWIDTH_20, MPI_11B_LONG, MPI_11B_RATE_2_MBPS,   MPI_GUARD_DONT_CARE, 0X1},      // HAL_PHY_RATE_11B_LONG_2_MBPS,
    {MPI_BANDWIDTH_20, MPI_11B_LONG, MPI_11B_RATE_5_5_MBPS, MPI_GUARD_DONT_CARE, 0X2},      // HAL_PHY_RATE_11B_LONG_5_5_MBPS,
    {MPI_BANDWIDTH_20, MPI_11B_LONG, MPI_11B_RATE_11_MBPS,  MPI_GUARD_DONT_CARE, 0X3},      // HAL_PHY_RATE_11B_LONG_11_MBPS,
    {MPI_BANDWIDTH_20, MPI_11B_SHORT, MPI_11B_RATE_2_MBPS,   MPI_GUARD_DONT_CARE, 0X1},     // HAL_PHY_RATE_11B_SHORT_2_MBPS,
    {MPI_BANDWIDTH_20, MPI_11B_SHORT, MPI_11B_RATE_5_5_MBPS,  MPI_GUARD_DONT_CARE, 0X2},    // HAL_PHY_RATE_11B_SHORT_5_5_MBPS,
    {MPI_BANDWIDTH_20, MPI_11B_SHORT, MPI_11B_RATE_11_MBPS,  MPI_GUARD_DONT_CARE, 0X3},     // HAL_PHY_RATE_11B_SHORT_11_MBPS,
    //SLR Rates                                                                             // SLR Rates
    {MPI_BANDWIDTH_20, MPI_SLR_MODE, MPI_11B_RATE_0_25_MBPS, MPI_GUARD_DONT_CARE, 0X0},     // HAL_PHY_RATE_SLR_0_25_MBPS,
    {MPI_BANDWIDTH_20, MPI_SLR_MODE, MPI_11B_RATE_0_5_MBPS,  MPI_GUARD_DONT_CARE, 0X1},     // HAL_PHY_RATE_SLR_0_5_MBPS,
    //Spica_Virgo 11A 20MHz Rates                                                           // Spica_Virgo 11A 20MHz Rates
    {MPI_BANDWIDTH_20, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0XB},                  // HAL_PHY_RATE_11A_6_MBPS,
    {MPI_BANDWIDTH_20, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0XF},                  // HAL_PHY_RATE_11A_9_MBPS,
    {MPI_BANDWIDTH_20, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0XA},                  // HAL_PHY_RATE_11A_12_MBPS,
    {MPI_BANDWIDTH_20, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0XE},                  // HAL_PHY_RATE_11A_18_MBPS,
    {MPI_BANDWIDTH_20, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0X9},                  // HAL_PHY_RATE_11A_24_MBPS,
    {MPI_BANDWIDTH_20, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0XD},                  // HAL_PHY_RATE_11A_36_MBPS,
    {MPI_BANDWIDTH_20, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0X8},                  // HAL_PHY_RATE_11A_48_MBPS,
    {MPI_BANDWIDTH_20, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0XC},                  // HAL_PHY_RATE_11A_54_MBPS,
    //MCS Index #0-15 (20MHz) Mixed Mode                                                    // MCS Index #0-15 (20MHz)
    {MPI_BANDWIDTH_20, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0X0},                  // HAL_PHY_RATE_MCS_1NSS_6_5_MBPS,
    {MPI_BANDWIDTH_20, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0X1},                  // HAL_PHY_RATE_MCS_1NSS_13_MBPS,
    {MPI_BANDWIDTH_20, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0X2},                  // HAL_PHY_RATE_MCS_1NSS_19_5_MBPS,
    {MPI_BANDWIDTH_20, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0X3},                  // HAL_PHY_RATE_MCS_1NSS_26_MBPS,
    {MPI_BANDWIDTH_20, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0X4},                  // HAL_PHY_RATE_MCS_1NSS_39_MBPS,
    {MPI_BANDWIDTH_20, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0X5},                  // HAL_PHY_RATE_MCS_1NSS_52_MBPS,
    {MPI_BANDWIDTH_20, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0X6},                  // HAL_PHY_RATE_MCS_1NSS_58_5_MBPS,
    {MPI_BANDWIDTH_20, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0X7},                  // HAL_PHY_RATE_MCS_1NSS_65_MBPS,
    {MPI_BANDWIDTH_20, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_SHORT_400, 0X0},                   // HAL_PHY_RATE_MCS_1NSS_MM_SG_7_2_MBPS,
    {MPI_BANDWIDTH_20, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_SHORT_400, 0X1},                   // HAL_PHY_RATE_MCS_1NSS_MM_SG_14_4_MBPS,
    {MPI_BANDWIDTH_20, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_SHORT_400, 0X2},                   // HAL_PHY_RATE_MCS_1NSS_MM_SG_21_7_MBPS,
    {MPI_BANDWIDTH_20, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_SHORT_400, 0X3},                   // HAL_PHY_RATE_MCS_1NSS_MM_SG_28_9_MBPS,
    {MPI_BANDWIDTH_20, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_SHORT_400, 0X4},                   // HAL_PHY_RATE_MCS_1NSS_MM_SG_43_3_MBPS,
    {MPI_BANDWIDTH_20, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_SHORT_400, 0X5},                   // HAL_PHY_RATE_MCS_1NSS_MM_SG_57_8_MBPS,
    {MPI_BANDWIDTH_20, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_SHORT_400, 0X6},                   // HAL_PHY_RATE_MCS_1NSS_MM_SG_65_MBPS,
    {MPI_BANDWIDTH_20, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_SHORT_400, 0X7}                    // HAL_PHY_RATE_MCS_1NSS_MM_SG_72_2_MBPS,
};


typedef struct
{
#ifdef ANI_LITTLE_BIT_ENDIAN
    tANI_U32 crc                    :1;     //always 1 = calculate CRC
    tANI_U32 cmd_len                :6;     //6 for 11b & 11a mixed mode, 11 for greenfield rates
    tANI_U32 ipg                    :24;    //interframe space in 160MHz clocks
    tANI_U32 cat                    :1;     //always 0 = normal packet
                                            //
    tANI_U32 pyldr_len              :16;    //number of random payload bytes to be generated
    tANI_U32 pyldf_len              :16;    //number of MPDU header bytes (+ number of pregenerated fixed payload bytes)
#else

    tANI_U32 cat                    :1;     //always 0 = normal packet
    tANI_U32 ipg                    :24;    //interframe space in 160MHz clocks
    tANI_U32 cmd_len                :6;     //6 for 11b & 11a mixed mode, 11 for greenfield rates
    tANI_U32 crc                    :1;     //always 1 = calculate CRC

    tANI_U32 pyldf_len              :16;    //number of MPDU header bytes (+ number of pregenerated fixed payload bytes)
    tANI_U32 pyldr_len              :16;    //number of random payload bytes to be generated
#endif
}sPhyDbgHdr;



typedef struct
{
#ifdef ANI_LITTLE_BIT_ENDIAN
    //byte 0
    tANI_U32 command_length         :8;     //8 for 11b & 11a mixed mode, 11 for greenfield rates

    //byte 1
    tANI_U32 packet_type            :2;
    tANI_U32 reserved_1             :1;
    tANI_U32 subband_mode           :1;
    tANI_U32 reserved_2             :1;
    tANI_U32 bandwidth_mode         :2;
    tANI_U32 reserved_3             :1;

    //byte 2
    tANI_U32 tx_antenna_enable      :3;
    tANI_U32 reserved_4             :1;
    tANI_U32 nss_11b_mode           :2;
    tANI_U32 b_rate                 :2; //seems to have vanished from the latest mpi document, but Brian is setting this

    //byte 3
    tANI_U32 tx_demanded_power      :5;
    tANI_U32 STBC                   :2;
    tANI_U32 reserved_5             :1;

    //byte 4
    tANI_U32 reserved_6             :3;
    tANI_U32 plcp_override          :1;
    tANI_U32 short_guard_interval   :1;
    tANI_U32 reserved_7             :3;

    //byte 5 & 6
    tANI_U32 psdu_length            :16;

    //byte 7
    tANI_U32 psdu_rate              :7;
    tANI_U32 a_mpdu_flag            :1;

    //byte 8 & 9
    tANI_U32 ppdu_length            :12;
    tANI_U32 reserved_8             :4;

    //byte 10
    tANI_U32 ppdu_rate              :4;
    tANI_U32 plcp_override_mask     :4;

    //byte 11
    tANI_U32 reserved_9             :8;    //there is an unused byte here -
                                           // don't include it when writing the memory
#else


    //byte 3
    tANI_U32 reserved_5             :1;
    tANI_U32 STBC                   :2;
    tANI_U32 tx_demanded_power      :5;
    //byte 2
    tANI_U32 b_rate                 :2; //seems to have vanished from the latest mpi document, but Brian is setting this
    tANI_U32 nss_11b_mode           :2;
    tANI_U32 reserved_4             :1;
    tANI_U32 tx_antenna_enable      :3;
    //byte 1
    tANI_U32 reserved_3             :1;
    tANI_U32 bandwidth_mode         :2;
    tANI_U32 reserved_2             :1;
    tANI_U32 subband_mode           :1;
    tANI_U32 reserved_1             :1;
    tANI_U32 packet_type            :2;
    //byte 0
    tANI_U32 command_length         :8;     //8 for 11b & 11a mixed mode, 11 for greenfield rates
    //byte 7
    tANI_U32 a_mpdu_flag            :1;
    tANI_U32 psdu_rate              :7;
    //byte 5 & 6
    tANI_U32 psdu_length            :16;
    //byte 4
    tANI_U32 reserved_7             :3;
    tANI_U32 short_guard_interval   :1;
    tANI_U32 plcp_override          :1;
    tANI_U32 reserved_6             :3;

    //byte 11
    tANI_U32 reserved_9             :8;
    //byte 10
    tANI_U32 plcp_override_mask     :4;
    tANI_U32 ppdu_rate              :4;
    //byte 8 & 9
    tANI_U32 reserved_8             :4;
    tANI_U32 ppdu_length            :12;

#endif
}sMpiHdr;

typedef struct
{
    sPhyDbgHdr  phyDbgHdr;
    sMpiHdr     mpiHdr;
    sMPDUHeader mpduHdr;
    //payload goes here
}tPhyDbgFrame;


//setup default frame for 1000 random bytes at 6Mbps

const tPhyDbgFrame defaultFrame =
{
#ifdef ANI_LITTLE_BIT_ENDIAN
    {
        0,                                      // crc
        11,                                     // cmd_len
        DEFAULT_INTERFRAME_SPACE,               // ipg
        0,                                      // cat
        1000,                                   // pyldr_len
        24                                      // pyldf_len
    },
    {
        11,                                     // command_length
        0,                                      // packet_type          {"11a":0, "11b":1, "11n_gf":2, "11n_mm":3, "slr": 1}
        0,                                      // reserved 1
        0,                                      // subband_mode         {"primary":0, "secondary":1}
        0,                                      // reserved_2
        0,                                      // bandwidth_mode       {"20":0, "cb":1, "dup":2}
        0,                                      // reserved_3
        1,                                      // tx_antenna_enable
        0,                                      // reserved_4
        0,                                      // nss_11b_mode         1 for 1Mbps, else 0
        0,                                      // b_rate
        16,                                     // tx_demanded_power    from halPhyGetRateForPower()
        0,                                      // STBC
        0,                                      // reserved_5
        0,                                      // reserved_6
        0,                                      // plcp_override            always 0
        0,                                      // short_guard_interval     based on rate
        0,                                      // reserved_7
        (sizeof(sMPDUHeader) + 1000 + 4),       // psdu_length, if we let HW to generate the 1000 MPDU payload, pyldf_len=sizeof(sMPDUHeader), 24 bytes,
                                                // and pyldr_leng=1000,
        0xB,                                    // psdu_rate  ===> signalCode from new rate table ==> fix it later
        0,                                      // a_mpdu_flag
        0,                                      // ppdu_length
        0,                                      // reserved_8
        0,                                      // ppdu_rate
        0,                                      // plcp_override_mask
        0                                       // reserved_9
    },
    {
        DATA_FRAME_TYPE,                        // tMacFrameType frameType;
        0,                                      // tMacFrameCtrl frameCtrl;
        _HTONS(1371),                           // tANI_U16 duration;
        { 0x00, 0xDE, 0xAD, 0xBE, 0xEF, 0x00},  // tANI_U8  MACAddress1[6];
        { 0x00, 0xCA, 0xFE, 0xBA, 0xBE, 0x00},  // tANI_U8  MACAddress2[6];
        { 0x00, 0x11, 0x11, 0x11, 0x11, 0x00},  // tANI_U8  MACAddress3[6];
        _HTONS(1)                               // tANI_U16 seqNum;
    }
#else
    {
        0,                                      // cat
        DEFAULT_INTERFRAME_SPACE,               // ipg
        11,                                     // cmd_len
        0,                                      // crc

        24,                                     // pyldf_len
        1000                                    // pyldr_len
    },
    {
        0,                                      // reserved_5
        0,                                      // STBC
        16,                                     // tx_demanded_power
        0,                                      // b_rate
        0,                                      // nss_11b_mode
        0,                                      // reserved_4
        1,                                      // tx_antenna_enable


        0,                                      // reserved_3
        0,                                      // bandwidth_mode
        0,                                      // reserved_2
        0,                                      // subband_mode
        0,                                      // reserved_1
        0,                                      // packet_type

        11,                                     // command_length

        0,                                      // a_mpdu_flag
        0xB,                                    // psdu_rate      ==> need new rate table ==> fix it later RY
        (sizeof(sMPDUHeader) + 1000 + 4),       // psdu_length
        0,                                      // reserved_7
        0,                                      // short_guard_interval
        0,                                      // plcp_override
        0,                                      // reserved_6

        0,                                      //reserved_9
        0,                                      // plcp_override_mask
        0,                                      //ppdu_rate
        0,                                      //reserved_8
        0                                       //ppdu_length
    },
    {
        DATA_FRAME_TYPE,                        // tMacFrameType frameType;
        0,                                      // tMacFrameCtrl frameCtrl;
        _HTONS(1371),                           // tANI_U16 duration;
        { 0x00, 0xDE, 0xAD, 0xBE, 0xEF, 0x00},  // tANI_U8  MACAddress1[6];
        { 0x00, 0xCA, 0xFE, 0xBA, 0xBE, 0x00},  // tANI_U8  MACAddress2[6];
        { 0x00, 0x11, 0x11, 0x11, 0x11, 0x00},  // tANI_U8  MACAddress3[6];
        _HTONS(1)                               // tANI_U16 seqNum;
    }


#endif
};


#define MIF_LSRAM_START     0x00000000L
#ifdef SPICA_FPGA
// Spica has D = 32 kB
#define MIF_LSRAM_END       0x00020000L
#else
// Virgo has 4D SRAM, each D is 32k words == 128 kB
#define MIF_LSRAM_END       0x00080000L
#endif

#define RAMP_UP_11B_PACKETS     0x100
#define RC_DELAY_11B_PACKETS    0x7ff
#define STOP_ITER_LIMIT         10000

extern eHalStatus halPhyGetPowerForRate(tHalHandle hHal, eHalPhyRates rate, ePowerMode pwrMode, tPowerdBm absPwrLimit, tPwrTemplateIndex *retTemplateIndex);
void printFrameFields(tpAniSirGlobal pMac, tPhyDbgFrame *frame, eHalPhyRates rate);
static eHalStatus CalcInterframeSpaceSetting(tpAniSirGlobal pMac, tANI_U32 numTestPackets, int interFrameSpace, int r_up, int r_down, int warmup_delay, int *ifsSetting);

eHalStatus asicPhyDbgStartFrameGen(tpAniSirGlobal pMac,
                                   eHalPhyRates rate,
                                   tANI_U16 payloadLength,
                                   ePayloadContents payloadContents,
                                   tANI_U8 payloadFillByte,
                                   tANI_U8 *payload,
                                   tANI_U32 numTestPackets,
                                   tANI_U32 interFrameSpace,
                                   tANI_BOOLEAN pktAutoSeqNum,
                                   tANI_U8 pktScramblerSeed,
                                   ePhyDbgPreamble preamble,
                                   tANI_U8 *addr1, tANI_U8 *addr2, tANI_U8 *addr3, tANI_BOOLEAN crc
                                  )
{
    eHalStatus retVal;
    //tANI_U32 sramLoc = QWLAN_PHYDBG_DBGMEM_MREG;
    //tANI_U32 numFrames;
    tPhyDbgFrame frame;

    //frameBuf sized to hold complete phydbg frame as it is written to memory
    //sPhyDbgHdr is written in 32-bit pieces,
    // whereas all the remaining data is parsed into one byte per 32-bit word
    //
    // !Important, when writing to phyDbg memory over the 16-bit AHB bus, we need
    // to chop the 32-bit words into two subsequent writes on subsequent 32-bit boundaries.
    tANI_U32 pktWords = (sizeof(sPhyDbgHdr) / 4) + sizeof(sMpiHdr) + sizeof(sMPDUHeader);
    tANI_U32 *bufWord;
    tANI_U32 frameBuf[((sizeof(sPhyDbgHdr) / 4) + sizeof(sMpiHdr) + sizeof(sMPDUHeader)) * 2];


    assert(rate < NUM_HAL_PHY_RATES);
    //assert(interFrameSpace < (MSK_24 / ONE_MICROSECOND));

    memcpy(&frame, &defaultFrame, sizeof(tPhyDbgFrame));

    if (pMac->hphy.phy.test.testDisablePhyRegAccess == eANI_BOOLEAN_TRUE)
    {
        return(eHAL_STATUS_FAILURE);
    }

    // Explicitely enable clcok to PHYDBG module
    if ((retVal = rdModWrAsicField(pMac, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_REG, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_PHYDBG_MASK, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_PHYDBG_OFFSET, 1)) != eHAL_STATUS_SUCCESS)
    {
        return(retVal);
    }
    if ((retVal = rdModWrAsicField(pMac, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_REG, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_PHYDBG_APB_MASK, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_PHYDBG_APB_OFFSET, 1)) != eHAL_STATUS_SUCCESS)
    {
        return(retVal);
    }
    {
        tANI_U32 reg;

        //SET_PHY_REG(pMac->hHdd, QWLAN_AGC_RX_OVERRIDE_REG, 0x300);

        GET_PHY_REG(pMac->hHdd, QWLAN_AGC_RX_OVERRIDE_REG, &reg);
        if (((reg & QWLAN_AGC_RX_OVERRIDE_ENRX_VAL_MASK) >> QWLAN_AGC_RX_OVERRIDE_ENRX_VAL_OFFSET) &
            ((reg & QWLAN_AGC_RX_OVERRIDE_OVERRIDE_EN_MASK) >> QWLAN_AGC_RX_OVERRIDE_OVERRIDE_EN_OFFSET)
           )
        {
            phyLog(pMac, LOGE, "frame generation won't work if any receivers are overriden enabled\n");
            return(eHAL_STATUS_FAILURE);
        }
    }


    asicPhyDbgStopFrameGen(pMac);

    // Explicitely enable clcok to PHYDBG module
    if ((retVal = rdModWrAsicField(pMac, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_REG, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_PHYDBG_MASK, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_PHYDBG_OFFSET, 1)) != eHAL_STATUS_SUCCESS)
    {
        return(retVal);
    }
    if ((retVal = rdModWrAsicField(pMac, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_REG, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_PHYDBG_APB_MASK, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_PHYDBG_APB_OFFSET, 1)) != eHAL_STATUS_SUCCESS)
    {
        return(retVal);
    }

    if ((preamble == PHYDBG_PREAMBLE_SHORTB) || (preamble == PHYDBG_PREAMBLE_LONGB))
    {
        //For "11b sparking" issue, use a different ramp up time for sending 11b packets
        SET_PHY_REG(pMac->hHdd, QWLAN_TXCTL_RAMP_UP_REG, RAMP_UP_11B_PACKETS);
        SET_PHY_REG(pMac->hHdd, QWLAN_TPC_RC_DELAY_REG, RC_DELAY_11B_PACKETS);
    }

    {   //set interframe spacing and warmup_delay
        int ifsSetting = 10;
        tANI_U32 warmup_delay = 2;  //value of 2 from Send() in send_pkt.py
        tANI_U32 ramp_up, ramp_down;

        GET_PHY_REG(pMac->hHdd, QWLAN_TXCTL_RAMP_UP_REG, &ramp_up);
        GET_PHY_REG(pMac->hHdd, QWLAN_TXCTL_RAMP_DOWN_REG, &ramp_down);

        CalcInterframeSpaceSetting(pMac, numTestPackets, (int)interFrameSpace, (int)ramp_up, (int)ramp_down, (int)warmup_delay, &ifsSetting);

        frame.phyDbgHdr.ipg = ifsSetting & MSK_24;

    }


    SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_RST1_REG, 0);

    //Format frame from PTT params
    {
        tPwrTemplateIndex powerIndex;
        tANI_U8 *byte;
        tANI_U32 *word;

        if (crc == eANI_BOOLEAN_TRUE)
        {
            frame.phyDbgHdr.crc = 1;
        }
        else
        {
            frame.phyDbgHdr.crc = 0;
        }

        frame.mpiHdr.bandwidth_mode = ((tANI_U8)rateSettings[rate].bandwidth & MSK_2);
        frame.mpiHdr.nss_11b_mode = ((tANI_U8)rateSettings[rate].nss11b & MSK_2);
        frame.mpiHdr.b_rate = ((tANI_U8)rateSettings[rate].bRateSelect & MSK_2);
        frame.mpiHdr.short_guard_interval = ((tANI_U8)rateSettings[rate].guard & MSK_1);

        {
            switch (preamble)
            {
                case PHYDBG_PREAMBLE_OFDM:
                    frame.mpiHdr.packet_type = MPI_PKT_TYPE_11A;
                    frame.mpiHdr.psdu_rate = ((tANI_U8)rateSettings[rate].signalCode & MSK_7);
                    //frame.mpiHdr.ppdu_rate = ((tANI_U8)MPI_PLCP_PPDU_RATE_CODE_6MBPS & MSK_4);
                    frame.mpiHdr.ppdu_rate = frame.mpiHdr.psdu_rate;
                    break;
                case PHYDBG_PREAMBLE_SHORTB:
                    frame.mpiHdr.packet_type = MPI_PKT_TYPE_11B;
                    break;
                case PHYDBG_PREAMBLE_LONGB:
                    frame.mpiHdr.packet_type = MPI_PKT_TYPE_11B;
                    break;
                case PHYDBG_PREAMBLE_GREENFIELD:
                    frame.mpiHdr.packet_type = MPI_PKT_TYPE_GREENFIELD;
                    frame.mpiHdr.psdu_rate = ((tANI_U8)rateSettings[rate].signalCode & MSK_7);
                    //frame.mpiHdr.ppdu_rate = ((tANI_U8)MPI_PLCP_PPDU_RATE_CODE_DONT_CARE & MSK_4);
                    frame.mpiHdr.ppdu_rate = frame.mpiHdr.psdu_rate;
                    break;
                case PHYDBG_PREAMBLE_MIXED:
                    frame.mpiHdr.packet_type = MPI_PKT_TYPE_MIXED_11A;
                    frame.mpiHdr.psdu_rate = ((tANI_U8)rateSettings[rate].signalCode & MSK_7);
                    frame.mpiHdr.ppdu_rate = ((tANI_U8)MPI_PLCP_PPDU_RATE_CODE_6MBPS & MSK_4);
                    break;
                default:
                    phyLog(pMac, LOGE, "ERROR: Bad preamble type %d\n", preamble);
                    return(eHAL_STATUS_FAILURE);
            }
        }

        frame.mpiHdr.tx_antenna_enable = 0;     //setting this to 0 requires the tx_fir_mode register to have the appropriate number of antennas

        if (//TEST ONLY (halIsQFuseBlown(pMac) == eHAL_STATUS_SUCCESS) &&   //if QFuse not blown, then pwrCfgPresent is only default data
            //(pMac->hphy.phyTPC.pwrCfgPresent == eANI_BOOLEAN_TRUE) &&    //must have CLPC data
            (pMac->hphy.phy.regDomainInfo != NULL) &&                    //must have reg domain info
            (pMac->hphy.phy.test.testTpcClosedLoop == eANI_BOOLEAN_TRUE) //loop must be closed, otherwise we want the index = 0
           )
        {
            eRfChannels rfChannel = rfGetCurChannel(pMac);

            assert(rfChannel != INVALID_RF_CHANNEL);


            if (pMac->hphy.phy.test.testTxGainIndexSource == REGULATORY_POWER_LIMITS)
            {
                if ((retVal =
                     halPhyGetPowerForRate(pMac,
                                           rate,
                                           POWER_MODE_HIGH_POWER,
                                           //the following param only used if the phy layer is set to regard this regulatory limit through pMac->hphy.phy.test.testTxGainIndexSource
                                           pMac->hphy.phy.regDomainInfo[pMac->hphy.phy.curRegDomain].channels[rfChannel].pwrLimit,
                                           &powerIndex)
                    ) != eHAL_STATUS_SUCCESS
                   )
                {
                    return(retVal);
                }
            }
            else if ((pMac->hphy.phy.test.testTxGainIndexSource == RATE_POWER_NON_LIMITED) ||
                     (pMac->hphy.phy.test.testTxGainIndexSource == FIXED_POWER_DBM) ||
                     (pMac->hphy.phy.test.testTxGainIndexSource == FORCE_POWER_TEMPLATE_INDEX)
                    )
            {
                if ((retVal =
                     halPhyGetPowerForRate(pMac,
                                           rate,
                                           POWER_MODE_HIGH_POWER,
                                           //the following param only used if the phy layer is set to regard this regulatory limit through pMac->hphy.phy.test.testTxGainIndexSource
                                           30,           //set power limitation to high value so as to be non-limiting, 0 to 31 valid range for set point
                                           &powerIndex)
                    ) != eHAL_STATUS_SUCCESS
                   )
                {
                    return(retVal);
                }
            }
            else
            {
                return(eHAL_STATUS_FAILURE);   //no other test modes are for closed-loop control
            }


            assert(powerIndex < 32);

            frame.mpiHdr.tx_demanded_power = powerIndex;    // use powerIndex returned from halPhyGetPowerForRate
        }
        else
        {
            frame.mpiHdr.tx_demanded_power = 0; //TPC does not automatically update gain index 0
        }

        frame.mpiHdr.psdu_length = sizeof(sMPDUHeader) + payloadLength;
        frame.mpiHdr.ppdu_length = sizeof(sMPDUHeader) + payloadLength;
        //frame.mpiHdr.psdu_length = payloadLength;
        //frame.mpiHdr.ppdu_length = payloadLength;

        if (frame.phyDbgHdr.crc == 1)
        {
            frame.mpiHdr.psdu_length += 4;
            frame.mpiHdr.ppdu_length += 4;
        }

        memcpy(&frame.mpduHdr.MACAddress1[0], addr1, ANI_MAC_ADDR_SIZE);         // &pMac->ptt.destMacAddress[0],
        memcpy(&frame.mpduHdr.MACAddress2[0], addr2, ANI_MAC_ADDR_SIZE);         // &pMac->ptt.sourceMacAddress[0]
        memcpy(&frame.mpduHdr.MACAddress3[0], addr3, ANI_MAC_ADDR_SIZE);         // &pMac->ptt.bssIdMacAddress[0],

         frame.mpduHdr.duration = FDUR_DURATION_MASK;    //set for max duration

         frame.mpduHdr.seqNum = 7;

        frame.phyDbgHdr.pyldf_len = sizeof(sMPDUHeader);    //initialize the fixed length to the fixed size mpdu header
                                                            //frame.phyDbgHdr.pyldf_len = size of the MPDU header being used
                                                            //in this frame formatting;

        //now fill in the payload
        switch (payloadContents)
        {
            case TEST_PAYLOAD_RAMP: //implement this later if needed
            case TEST_PAYLOAD_FILL_BYTE:
                if ((retVal = palFillMemory(pMac->hHdd, payload, payloadLength, payloadFillByte)
                    ) != eHAL_STATUS_SUCCESS
                   )
                {
                    return(retVal);
                }
                frame.phyDbgHdr.pyldf_len += payloadLength;
                frame.phyDbgHdr.pyldr_len = 0;                  // HW is not required to generate any byte
                break;
            case TEST_PAYLOAD_RANDOM:
                frame.phyDbgHdr.pyldr_len = payloadLength;      // HW is required to generated the requested num of bytes
                break;
            case TEST_PAYLOAD_NONE: //zero length
                frame.phyDbgHdr.pyldr_len = 0;                  // HW is not required to generate any byte, and the test
                                                                // rrame is with mpdu header being used in this mode only.
                break;
            default:
                assert(0);
                break;
        }

        {
            //everything about the frame is already setup and since not even the sequence number is changing,
            // just setup phydbg to loop back on the single frame
            //write header portion one byte at a time
#ifdef ANI_PHY_DEBUG
            printFrameFields(pMac, &frame, rate);
#endif

            //write first three 32-bit words with phyDbgHdr
            bufWord = &frameBuf[0];
            for (word = (tANI_U32 *)&frame.phyDbgHdr; word < (tANI_U32 *)&frame.mpiHdr; word++)
            {
                *bufWord = (*word & 0x0000FFFF);
                bufWord ++;
                *bufWord = ((*word & 0xFFFF0000) >> 16);
                bufWord ++;
            }

            sirBusyWait(5000);
            //byteswap the duration and seqNum
            // for (word = (tANI_U32 *)&frame.mpiHdr; word < (tANI_U32 *)&frame.mpduHdr; word++)
            // {
            //     HTONL(*word);
            // }
            // HTONS(frame.mpduHdr.duration);
            // HTONS(frame.mpduHdr.seqNum);

            //write remaining mpiHdr and mpduHdr one byte at a time to each 32-bit word
            {
                volatile tANI_U32 i = 0;

                //for (byte = (tANI_U8 *)&frame.mpiHdr; byte < ((tANI_U8 *)&frame + sizeof(tPhyDbgFrame)); byte++, i++)
                for (byte = (tANI_U8 *)&frame.mpiHdr; byte < ((tANI_U8 *)&frame + sizeof(sPhyDbgHdr) + sizeof(sMpiHdr)  + sizeof(sMPDUHeader)); byte++, i++)
                {
                    tANI_U32 intByte = *byte;

                    //skipping byte 11 in mpi header
                    if (byte == ((tANI_U8 *)&frame.mpiHdr + sizeof(frame.mpiHdr) -1))
                    {
                        continue;
                    }
                    //phyLog(pMac, LOG3, "Writing 0x%08X to addr 0x%08X\n", intByte, sramLoc);
                    assert((intByte & 0xFFFFFF00) == 0);

                    *bufWord = intByte & 0xFF;
                    bufWord ++;
                    *bufWord = (0);
                    bufWord ++;
                }
                i = i; //only for debug purposes to verify how many bytes are counted
            }
            //palWriteRegMemory takes the number of bytes, so multiply the number of words by 4
            retVal = palWriteRegMemory(pMac->hHdd, QWLAN_PHYDBG_DBGMEM_MREG, (tANI_U8 *)&frameBuf[0], ((pktWords * 2) * 4));
            if (retVal != eHAL_STATUS_SUCCESS)
            {
                return(retVal);
            }
        }
    }

    if (numTestPackets > QWLAN_PHYDBG_TXPKT_CNT_CNT_MASK)
    {
        SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_TXPKT_CNT_REG, QWLAN_PHYDBG_TXPKT_CNT_CNT_MASK);
    }
    else if (numTestPackets == 0)
    {
        //continuous packets - TODO: how do we set this up and stop it?
        SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_TXPKT_CNT_REG, 0);
    }
    else
    {
        SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_TXPKT_CNT_REG, numTestPackets);
    }

    SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_CFGMODE_REG,
                    (QWLAN_PHYDBG_CFGMODE_AUTO_TX_TRIG_SEL1_MASK |
                     QWLAN_PHYDBG_CFGMODE_DBGMEM_SEL1_MASK |
                     QWLAN_PHYDBG_CFGMODE_CONT1_MASK)
               );

    SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_PLYBCK_CFG_REG,
                    (QWLAN_PHYDBG_PLYBCK_CFG_MPI_TXTEST_MASK |
                     QWLAN_PHYDBG_PLYBCK_CFG_DUP_CH0_MASK |
                     QWLAN_PHYDBG_PLYBCK_CFG_TXFIR_DBGSEL_MASK
                    )
               );

    if(pMac->hphy.phy.test.identicalPayloadEnabled)
    {
        SET_PHY_REG(pMac->hHdd, QWLAN_TACTL_SCR_CONFIG_REG, ((1 << QWLAN_TACTL_SCR_CONFIG_TASCR_LOAD_OFFSET) |
                                                                (0x7f << QWLAN_TACTL_SCR_CONFIG_TASCR_SEED_OFFSET)));
        SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_PRBS_REG, ((1 << QWLAN_PHYDBG_PRBS_AUTO_RELOAD_OFFSET) |
                                                            (75 << QWLAN_PHYDBG_PRBS_SEED_LSB_OFFSET)));
        SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_PRBS_MS_REG, 0x0);//(0x340c98);
    }
    else
    {
        SET_PHY_REG(pMac->hHdd, QWLAN_TACTL_SCR_CONFIG_REG, QWLAN_TACTL_SCR_CONFIG_DEFAULT);
        SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_PRBS_REG, (0xff << QWLAN_PHYDBG_PRBS_SEED_LSB_OFFSET));//(0x10);                               //seed value
        SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_PRBS_MS_REG, 0x8a);//(0x5432);
    }

    SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_PRBS_LOAD_REG, QWLAN_PHYDBG_PRBS_LOAD_LOAD_MASK);


    //now setup phyDbg registers to loop this frame according to pMac->ptt.numTestPackets
    {

        SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_START_ADDR1_REG, 0/*QWLAN_PHYDBG_DBGMEM_MREG*/);
        //SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_PLYBCK_CFG2_REG, 0);

        //exclude sMpiHdr: byte 11 from word count when setting MAX_ADDR1
        SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_MAX_ADDR1_REG, (pktWords) - 1 - 1);
    }

    //start generating frames
    SET_PHY_REG(pMac->hHdd, QWLAN_MPI_MPI_ENABLE_REG, 1);
    SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_START1_REG, QWLAN_PHYDBG_START1_START_MASK);

    if(numTestPackets != 0)
    {
        //this is not continuous. stop the framegen once the pkts are transmitted
        tANI_U32 i = 0;
        tANI_U32 reg;

        do
        {
            GET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_STATUS_REG, &reg);
        }
        while ((i++ < STOP_ITER_LIMIT) && (reg & QWLAN_PHYDBG_STATUS_TXSTATE_MASK));

        if (i >= STOP_ITER_LIMIT)
        {
            phyLog(pMac, LOGE, "ERROR: PhyDbg txstate=%d, Not Idle when stopped!\n",
                   (reg & QWLAN_PHYDBG_STATUS_TXSTATE_MASK) >> QWLAN_PHYDBG_STATUS_TXSTATE_OFFSET
                  );
        }
        asicPhyDbgStopFrameGen(pMac);
    }

    return(retVal);
}

#ifdef ANI_PHY_DEBUG
extern const char rateStr[NUM_HAL_PHY_RATES][50];
void printFrameFields(tpAniSirGlobal pMac, tPhyDbgFrame *frame, eHalPhyRates rate)
{

    phyLog(pMac, LOGE, "%s = %d ", &rateStr[rate][0], rate);

    phyLog(pMac, LOGE, "phyDbgHdr.crc               = %d ", frame->phyDbgHdr.crc              );
    phyLog(pMac, LOGE, "phyDbgHdr.cmd_len           = %d ", frame->phyDbgHdr.cmd_len          );
    phyLog(pMac, LOGE, "phyDbgHdr.ipg               = %d ", frame->phyDbgHdr.ipg              );
    phyLog(pMac, LOGE, "phyDbgHdr.cat               = %d ", frame->phyDbgHdr.cat              );
    phyLog(pMac, LOGE, "phyDbgHdr.pyldf_len         = %d ", frame->phyDbgHdr.pyldf_len        );
    phyLog(pMac, LOGE, "phyDbgHdr.pyldr_len         = %d ", frame->phyDbgHdr.pyldr_len        );
    phyLog(pMac, LOGE, "mpiHdr.command_length       = %d ", frame->mpiHdr.command_length      );
    phyLog(pMac, LOGE, "mpiHdr.packet_type          = %d ", frame->mpiHdr.packet_type         );
    phyLog(pMac, LOGE, "mpiHdr.reserved_1           = %d ", frame->mpiHdr.reserved_1          );
    phyLog(pMac, LOGE, "mpiHdr.subband_mode         = %d ", frame->mpiHdr.subband_mode        );
    phyLog(pMac, LOGE, "mpiHdr.reserved_2           = %d ", frame->mpiHdr.reserved_2          );
    phyLog(pMac, LOGE, "mpiHdr.bandwidth_mode       = %d ", frame->mpiHdr.bandwidth_mode      );
    phyLog(pMac, LOGE, "mpiHdr.reserved_3           = %d ", frame->mpiHdr.reserved_3          );
    phyLog(pMac, LOGE, "mpiHdr.tx_antenna_enable    = %d ", frame->mpiHdr.tx_antenna_enable   );
    phyLog(pMac, LOGE, "mpiHdr.reserved_4           = %d ", frame->mpiHdr.reserved_4          );
    phyLog(pMac, LOGE, "mpiHdr.nss_11b_mode         = %d ", frame->mpiHdr.nss_11b_mode        );
    phyLog(pMac, LOGE, "mpiHdr.b_rate               = %d ", frame->mpiHdr.b_rate              );
    phyLog(pMac, LOGE, "mpiHdr.tx_demanded_power    = %d ", frame->mpiHdr.tx_demanded_power   );
    phyLog(pMac, LOGE, "mpiHdr.STBC_                = %d ", frame->mpiHdr.STBC                );
    phyLog(pMac, LOGE, "mpiHdr.reserved_5           = %d ", frame->mpiHdr.reserved_5          );
    phyLog(pMac, LOGE, "mpiHdr.reserved_6           = %d ", frame->mpiHdr.reserved_6          );
    phyLog(pMac, LOGE, "mpiHdr.plcp_override        = %d ", frame->mpiHdr.plcp_override       );
    phyLog(pMac, LOGE, "mpiHdr.short_guard_interval = %d ", frame->mpiHdr.short_guard_interval);
    phyLog(pMac, LOGE, "mpiHdr.reserved_7           = %d ", frame->mpiHdr.reserved_7          );
    phyLog(pMac, LOGE, "mpiHdr.psdu_length          = %d ", frame->mpiHdr.psdu_length         );
    phyLog(pMac, LOGE, "mpiHdr.psdu_rate            = %d ", frame->mpiHdr.psdu_rate           );
    phyLog(pMac, LOGE, "mpiHdr.a_mpdu_flag          = %d ", frame->mpiHdr.a_mpdu_flag         );
    phyLog(pMac, LOGE, "mpiHdr.ppdu_length          = %d ", frame->mpiHdr.ppdu_length         );
    phyLog(pMac, LOGE, "mpiHdr.reserved_8           = %d ", frame->mpiHdr.reserved_8          );
    phyLog(pMac, LOGE, "mpiHdr.ppdu_rate            = %d ", frame->mpiHdr.ppdu_rate           );
    phyLog(pMac, LOGE, "mpiHdr.plcp_override_mask   = %d ", frame->mpiHdr.plcp_override_mask  );
    phyLog(pMac, LOGE, "mpiHdr.reserved_9           = %d ", frame->mpiHdr.reserved_9          );

}
#endif

#define MAX_VAL(a, b) (a > b ? a : b)


//following extracted from Brian's sendpacket.py script
#define OVERHEAD1    (52)
#define OVERHEAD2    (29)
#define OVERHEAD3    (28)
#define OVERHEAD4    (44)
#define OVERHEAD5    (106)

static eHalStatus CalcInterframeSpaceSetting(tpAniSirGlobal pMac, tANI_U32 numTestPackets, int interFrameSpace, int r_up, int r_down, int warmup_delay, int *ifsSetting)
{
    eHalStatus retVal;
    int ramp_up = r_up;
    int ramp_down = r_down;
    int warmup = warmup_delay;
    int derived_ifs;
    int paGuard;
    int nClocksPerUs = 80;
    int ifs_requested = (interFrameSpace * nClocksPerUs);

    GET_PHY_REG(pMac->hHdd, QWLAN_TXCTL_PA_GUARD_REG, &paGuard);

    warmup = MAX_VAL(1, (280/*warmup * nClocksPerUs*/) - (paGuard + ramp_up + OVERHEAD1));


    derived_ifs = ifs_requested - warmup - paGuard - ramp_up - ramp_down - OVERHEAD2;

    if (derived_ifs < 1)
    {
        warmup = 0;

        derived_ifs = ifs_requested - paGuard - ramp_up - ramp_down - OVERHEAD3;

        if (derived_ifs < (ramp_down - OVERHEAD4))
        {
            derived_ifs = ifs_requested - ramp_up - OVERHEAD5;
            if (derived_ifs > (ramp_down - OVERHEAD4 - 4))
            {
                phyLog(pMac, LOGE, "Warning:  The IFS specified cannot be obtained precisely with the current ramp_up/ramp_down/pa_guard times.\n");
            }
            if (derived_ifs < 1)
            {
                phyLog(pMac, LOGE, "Warning:  Min IFS obtainable with current ramp_up setting is %d us.\n", ((OVERHEAD5 + ramp_up + 1)/nClocksPerUs));
            }
        }
/*
        if (derived_ifs < 0)
        {
            warmup = ifs_requested - ramp_up - OVERHEAD2;

            if (warmup < 0)
            {
                warmup = 0;
            }
            derived_ifs = 1;    //limit to min of 1 because 0 causes phydbg to hang
        }
*/
    }

    //SET_PHY_REG(pMac->hHdd, QWLAN_TXCTL_RAMP_UP_REG, ramp_up);
    //SET_PHY_REG(pMac->hHdd, QWLAN_TXCTL_RAMP_DOWN_REG, ramp_down);
    SET_PHY_REG(pMac->hHdd,QWLAN_PHYDBG_WARMUP_DLY_REG, (warmup & MSK_16));

    *ifsSetting = MAX_VAL(derived_ifs, 1); //limit to min of 1 because 0 causes phydbg to hang

    return(eHAL_STATUS_SUCCESS);

}

eHalStatus asicPhyDbgStopFrameGen(tpAniSirGlobal pMac)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;

    SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_CFGMODE_REG, QWLAN_PHYDBG_CFGMODE_STOP1_MASK);

    {
        tANI_U32 i = 0;
        tANI_U32 reg;

        do
        {
            GET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_STATUS_REG, &reg);
        }
        while ((i++ < STOP_ITER_LIMIT) && (reg & QWLAN_PHYDBG_STATUS_TXSTATE_MASK));

        if (i >= STOP_ITER_LIMIT)
        {
            phyLog(pMac, LOGE, "ERROR: PhyDbg txstate=%d, Not Idle when stopped!\n",
                   (reg & QWLAN_PHYDBG_STATUS_TXSTATE_MASK) >> QWLAN_PHYDBG_STATUS_TXSTATE_OFFSET
                  );
        }
    }

    //For "11b sparking" issue, use a different ramp up time for sending 11b packets alone
    SET_PHY_REG(pMac->hHdd, QWLAN_TPC_RC_DELAY_REG, QWLAN_TPC_RC_DELAY_DEFAULT);
    SET_PHY_REG(pMac->hHdd, QWLAN_TXCTL_RAMP_UP_REG, QWLAN_TXCTL_RAMP_UP_DEFAULT);
    // SET_PHY_REG(pMac->hHdd, QWLAN_TXCTL_RAMP_DOWN_REG, 51);

    SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_RST1_REG, 0);        // may write 1 as well, this write access will reset the playback
                                                        // FSM. But not affect configuration and status of PHYDBG module.

    // if ((retVal = rdModWrAsicField(pMac, QWLAN_MIF_MIF_MEM_CFG_REG,
    //                                QWLAN_MIF_MIF_MEM_CFG_MIF_MEM_CFG_MASK,
    //                                QWLAN_MIF_MIF_MEM_CFG_MIF_MEM_CFG_OFFSET,
    //                                0                    //set back to normal (conf #0) for host access
    //                               )
    //     ) != eHAL_STATUS_SUCCESS
    //    )
    // {
    //     return(retVal);
    // }

    // Explicitely disable clcok to PHYDBG module
	/* These register bits are required for Volans to receive incoming frames
     * hence, commenting out
     */
#if 0
	if ((retVal = rdModWrAsicField(pMac, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_REG, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_PHYDBG_MASK, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_PHYDBG_OFFSET, 0)) != eHAL_STATUS_SUCCESS)
    {
        return(retVal);
    }
    if ((retVal = rdModWrAsicField(pMac, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_REG, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_PHYDBG_APB_MASK, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_PHYDBG_APB_OFFSET, 0)) != eHAL_STATUS_SUCCESS)
    {
        return(retVal);
    }
#endif

    if ((retVal = rdModWrAsicField(pMac, QWLAN_PHYDBG_PLYBCK_CFG_REG, QWLAN_PHYDBG_PLYBCK_CFG_MPI_TXTEST_MASK, QWLAN_PHYDBG_PLYBCK_CFG_MPI_TXTEST_OFFSET, 0)) != eHAL_STATUS_SUCCESS)
    {
        return(retVal);
    }

    SET_PHY_REG(pMac->hHdd, QWLAN_MPI_MPI_ENABLE_REG, 0x1);

    return(retVal);
}



eHalStatus asicPhyDbgQueryStatus(tpAniSirGlobal pMac, sTxFrameCounters *numFrames, ePhyDbgTxStatus *status)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
    tANI_U32 value;



    GET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_STATUS_REG, &value);
    value = ((value & QWLAN_PHYDBG_STATUS_TXSTATE_MASK) >> QWLAN_PHYDBG_STATUS_TXSTATE_OFFSET);

    *status = value;

    GET_PHY_REG(pMac->hHdd, QWLAN_TXCTL_LEGACY_PKTS_REG,     &numFrames->legacy);
    GET_PHY_REG(pMac->hHdd, QWLAN_TXCTL_GF_SIMO_20_PKTS_REG, &numFrames->gfSimo20);
    GET_PHY_REG(pMac->hHdd, QWLAN_TXCTL_GF_MIMO_20_PKTS_REG, &numFrames->gfMimo20);
    GET_PHY_REG(pMac->hHdd, QWLAN_TXCTL_MM_SIMO_20_PKTS_REG, &numFrames->mmSimo20);
    GET_PHY_REG(pMac->hHdd, QWLAN_TXCTL_MM_MIMO_20_PKTS_REG, &numFrames->mmMimo20);
    GET_PHY_REG(pMac->hHdd, QWLAN_TXCTL_TXB_SHORT_PKTS_REG,  &numFrames->txbShort);
    GET_PHY_REG(pMac->hHdd, QWLAN_TXCTL_TXB_LONG_PKTS_REG,   &numFrames->txbLong);
#ifdef WLAN_HAL_LIBRA
    GET_PHY_REG(pMac->hHdd, QWLAN_TXCTL_TXB_SLR_PKTS_REG,    &numFrames->txbSlr);
#endif

    SET_PHY_REG(pMac->hHdd, QWLAN_TXCTL_LEGACY_PKTS_REG,     0);
    SET_PHY_REG(pMac->hHdd, QWLAN_TXCTL_GF_SIMO_20_PKTS_REG, 0);
    SET_PHY_REG(pMac->hHdd, QWLAN_TXCTL_GF_MIMO_20_PKTS_REG, 0);
    SET_PHY_REG(pMac->hHdd, QWLAN_TXCTL_MM_SIMO_20_PKTS_REG, 0);
    SET_PHY_REG(pMac->hHdd, QWLAN_TXCTL_MM_MIMO_20_PKTS_REG, 0);
    SET_PHY_REG(pMac->hHdd, QWLAN_TXCTL_TXB_SHORT_PKTS_REG,  0);
    SET_PHY_REG(pMac->hHdd, QWLAN_TXCTL_TXB_LONG_PKTS_REG,   0);
    SET_PHY_REG(pMac->hHdd, QWLAN_TXCTL_TXB_SLR_PKTS_REG,    0);

    numFrames->total = numFrames->legacy      +
                       numFrames->gfSimo20    +
                       numFrames->gfMimo20    +
                       numFrames->mmSimo20    +
                       numFrames->mmMimo20    +
                       numFrames->txbShort    +
                       numFrames->txbLong
#ifdef WLAN_HAL_LIBRA
                       + numFrames->txbSlr
#endif
                       ;

    return(retVal);
}

//#include "stdlib.h" //for random numbers

//static eHalStatus configPktScramblerSeed(tpAniSirGlobal pMac, tANI_U8 seed)
//{
//    eHalStatus retVal = eHAL_STATUS_SUCCESS;
/*
        tANI_U8 txctrl_seed;

        srand((unsigned int)seed);

        txctrl_seed = (rand() & MSK_7);
        SET_PHY_REG(pMac->hHdd, TACTL_SCR_CONFIG_REG, (TACTL_SCR_CONFIG_TASCR_LOAD_MASK | txctrl_seed));    //keep TACTL_SCR_CONFIG_TASCR_LOAD_MASK high so that all frames use this

*/
//    return (retVal);
//}



static eHalStatus DoGrabRamCapture(tpAniSirGlobal pMac, tANI_U32 rxChain, eGrabRamSampleType sampleType);
/* not used
#define MEM_DBLOCK1_ADDR    0                                           //bits 0 to 31 in sample format
#define MEM_DBLOCK2_ADDR    GRAB_RAM_DBLOCK_SIZE                         //bits 32 to 63 in sample format
#define MEM_DBLOCK3_ADDR    MEM_DBLOCK2_ADDR + GRAB_RAM_DBLOCK_SIZE      //bits 64 to 95 in sample format
*/

#define INTERLEAVE_OFFSET_0  (160 * 1024)
#ifdef SPICA_FPGA
#define INTERLEAVE_OFFSET_1  (16 * 1024 * 4)
#else
#define INTERLEAVE_OFFSET_1  (64 * 1024 * 4)
#endif

/*
    Each sample is 96 bits in this format, but divided among three separate 8K memory blocks

    rx0_I       bits 0 to 10
    rx0_Q       bits 11 to 21
    rx1_I       bits 22 to 32
    rx1_Q       bits 33 to 43
    rx2_I       bits 44 to 54
    rx2_Q       bits 55 to 65

    unused      bits 66 to 69
    rx0_gain    bits 70 to 75
    rx1_gain    bits 76 to 81
    rx2_gain    bits 82 to 87

    rfsat       bits 88 to 90

    unused      bit 91
    agc         bits 92 to 95

*/
#define SAMPLE_SIGN_SHIFT  5

#define S16_SAMPLE_CONVERT(x)   ((tANI_S16)((tANI_U32)x << SAMPLE_SIGN_SHIFT) >> SAMPLE_SIGN_SHIFT)

#define READ_MEM_SIZE   128 //samples
#define ONE_K_MEM_SIZE  1024

/*
    asicGrabAdcSamples for Libra is designed to capture the samples at bottom half of the SRAM internal memory.
    The upper half of SRAM is consumed by firmware src code. Also we can capture only one chain at a time.
    So the proceedure is to capture the samples for chain 0 first and save it to cache before proceeding to rx chain 1
    Moving forward i.e., for Volans, we do not have the previlege of using MIF/SRAM, since the total internal memory size
    would be 128k only. So we should use phyDbg mem. Limitations of using phyDbgMem: we can only capture 1k continous ADC samples.
    startSample is the index of the starting sample, 0 if starting from the beginning of memory
    sampleBuffer contains enough space to retrieve all samples requested
    numSamples is input as the number requested, and is output as the number retrieved
*/
eHalStatus asicGrabAdcSamples(tpAniSirGlobal pMac, tANI_U32 startSample, tANI_U32 numSamples,
                                    eGrabRamSampleType sampleType, tGrabRamSample *sampleBuffer)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;

#ifndef WLAN_FTM_STUB
    tANI_U32 w, rxChain;
    tANI_U32 *pAdcSamples0;
    tANI_U16 data;

    if (((startSample + numSamples) > GRAB_RAM_DBLOCK_SIZE) || (numSamples > MAX_REQUESTED_GRAB_RAM_SAMPLES))
    {
        phyLog(pMac, LOGE, "Too many grab ram samples requested: start=%d num=%d\n", startSample, numSamples);
        return(eHAL_STATUS_FAILURE);
    }

    if (startSample == 0)
    {
        for(rxChain = 0; rxChain < PHY_MAX_RX_CHAINS; rxChain++)
        {
            if (DoGrabRamCapture(pMac, rxChain, sampleType) != eHAL_STATUS_SUCCESS)
        {
            return(eHAL_STATUS_FAILURE);
        }
    }
    }

    if(startSample % 4)
    {
        phyLog(pMac, LOGE, "startSample should be a multiple of 4\n");
        return(eHAL_STATUS_FAILURE);
    }

    pAdcSamples0 = pMac->ptt.pADCCaptureCache; //for rxChain 0

    //phyLog(pMac, LOGE, "start=%d  numSamples=%d \n", startSample, numSamples);
    for(w = startSample; w < (numSamples + startSample); w++)
	{
        if (sampleType == GRABRAM_POSTIQ)
        {
            data = (tANI_S16)((pAdcSamples0[w] >> 0) & MSK_11);
            sampleBuffer[w - startSample].rx0.I = data;
            if (data > 1023)
                sampleBuffer[w - startSample].rx0.I = data - 2048;

            data = (tANI_S16)((pAdcSamples0[w] >> 11) & MSK_11);
            sampleBuffer[w - startSample].rx0.Q = data;
            if (data > 1023)
                sampleBuffer[w - startSample].rx0.Q = data - 2048;
        }
        else
        {
            sampleBuffer[w - startSample].rx0.I = (tANI_S16)(((pAdcSamples0[w] >> 0 ) & MSK_11) - 1024);
            sampleBuffer[w - startSample].rx0.Q = (tANI_S16)(((pAdcSamples0[w] >> 11) & MSK_11) - 1024);
	    }
	}

#else
    phyLog(pMac, LOGE, "Grab Ram capture only available in FTM builds\n");
#endif

    return(retVal);
}

static eHalStatus DoGrabRamCapture(tpAniSirGlobal pMac, tANI_U32 rxChain, eGrabRamSampleType sampleType)
{
    eHalStatus retVal;
    tANI_U32 capCnt, rxClkCtrlApbBlockClkBckup, mifMemCfg, agcTestBus, i;
    tANI_U32 xBarSel[PHY_MAX_RX_CHAINS] = {QWLAN_PHYDBG_CAPT_CFG_XBARSEL_EC32_ADC_CH0};
    tANI_U8 *startPtr;

    //back up registers
    GET_PHY_REG(pMac->hHdd, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_REG, &rxClkCtrlApbBlockClkBckup);
    GET_PHY_REG(pMac->hHdd, QWLAN_MIF_MIF_MEM_CFG_REG, &mifMemCfg);
    GET_PHY_REG(pMac->hHdd, QWLAN_AGC_TESTBUS_REG, &agcTestBus);


    // Explicitely enable clcok to PHYDBG module
        if ((retVal = rdModWrAsicField(pMac, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_REG, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_PHYDBG_MASK, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_PHYDBG_OFFSET, 1)) != eHAL_STATUS_SUCCESS)
        {
            return(retVal);
        }
        if ((retVal = rdModWrAsicField(pMac, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_REG, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_TMUX_MASK, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_TMUX_OFFSET, 1)) != eHAL_STATUS_SUCCESS)
        {
            return(retVal);
        }
    if ((retVal = rdModWrAsicField(pMac, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_REG, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_PHYDBG_APB_MASK, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_PHYDBG_APB_OFFSET, 1)) != eHAL_STATUS_SUCCESS)
    {
        return(retVal);
    }
    if ((retVal = rdModWrAsicField(pMac, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_REG, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_PHYINT_MASK, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_PHYINT_OFFSET, 1)) != eHAL_STATUS_SUCCESS)
        {
        return(retVal);
    }

    if ((retVal = rdModWrAsicField(pMac, QWLAN_MIF_MIF_MEM_CFG_REG,
                                   QWLAN_MIF_MIF_MEM_CFG_MIF_MEM_CFG_MASK,
                                   QWLAN_MIF_MIF_MEM_CFG_MIF_MEM_CFG_OFFSET,
                                   0x9
                                  )
        ) != eHAL_STATUS_SUCCESS
       )
    {
        return(retVal);
    }

    //needed for RF gain/sat data
    if ((retVal = rdModWrAsicField(pMac, QWLAN_AGC_TESTBUS_REG, QWLAN_AGC_TESTBUS_AGC_RXTMUX_TMUXSEL_MASK, QWLAN_AGC_TESTBUS_AGC_RXTMUX_TMUXSEL_OFFSET, QWLAN_AGC_TESTBUS_AGC_RXTMUX_TMUXSEL_EAGC_80_40_2)) != eHAL_STATUS_SUCCESS) { return(retVal);}

    if ((retVal = rdModWrAsicField(pMac, QWLAN_PHYDBG_CFGMODE_REG, QWLAN_PHYDBG_CFGMODE_AUTO_TX_TRIG_SEL1_MASK,
                                    QWLAN_PHYDBG_CFGMODE_AUTO_TX_TRIG_SEL1_OFFSET, 1)) != eHAL_STATUS_SUCCESS) {
        return(retVal);
    }
    if ((retVal = rdModWrAsicField(pMac, QWLAN_PHYDBG_CFGMODE_REG, QWLAN_PHYDBG_CFGMODE_DBGMEM_SEL1_MASK,
                                    QWLAN_PHYDBG_CFGMODE_DBGMEM_SEL1_OFFSET, 1)) != eHAL_STATUS_SUCCESS) {
        return(retVal);
    }
    if ((retVal = rdModWrAsicField(pMac, QWLAN_PHYDBG_CFGMODE_REG, QWLAN_PHYDBG_CFGMODE_CONT1_MASK,
                                    QWLAN_PHYDBG_CFGMODE_CONT1_OFFSET, 1)) != eHAL_STATUS_SUCCESS) {
        return(retVal);
    }
#if 0
    SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_CFGMODE_REG, QWLAN_PHYDBG_CFGMODE_AUTO_TX_TRIG_SEL1_MASK |
                                                            QWLAN_PHYDBG_CFGMODE_DBGMEM_SEL1_MASK |
                                                            QWLAN_PHYDBG_CFGMODE_CONT1_MASK);
#endif

    SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_CAPT_CFG_REG, QWLAN_PHYDBG_CAPT_CFG_RFSAT_GAIN_SEL_MASK | (xBarSel[rxChain] << QWLAN_PHYDBG_CAPT_CFG_XBARSEL_OFFSET));

    if (sampleType == GRABRAM_POSTIQ)
    {
        //capture the I/Q samples post Rx FIR correction
        SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_CAPT_CFG2_REG, QWLAN_PHYDBG_CAPT_CFG2_XBAR_PIC_DATASEL_MASK);
    }
    else
    {
        SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_CAPT_CFG2_REG, 0);
    }

    SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_MAX_ADDR0_REG, (GRAB_RAM_DBLOCK_SIZE - 1));

    SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_CAPTCNT_REG, 0);    //reset capture counter before we start the new capture

    SET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_START0_REG, 1);

    //loop until all words captured
    do
    {
        GET_PHY_REG(pMac->hHdd, QWLAN_PHYDBG_CAPTCNT_REG, &capCnt);
    }while (capCnt < (GRAB_RAM_DBLOCK_SIZE));


    //restore registers
    SET_PHY_REG(pMac->hHdd, QWLAN_MIF_MIF_MEM_CFG_REG, mifMemCfg);
    SET_PHY_REG(pMac->hHdd, QWLAN_RXCLKCTRL_APB_BLOCK_CLK_EN_REG, rxClkCtrlApbBlockClkBckup);
    SET_PHY_REG(pMac->hHdd, QWLAN_AGC_TESTBUS_REG, agcTestBus);

    sirBusyWait(50000000); //wait 50 ms

    startPtr = (tANI_U8 *)(pMac->ptt.pADCCaptureCache) + (rxChain * sizeof(tANI_U32) * GRAB_RAM_DBLOCK_SIZE);
    //try to read a max of 1k samples each time i.e 4k mem size
    for(i = 0; i < GRAB_RAM_DBLOCK_SIZE; i+= ONE_K_MEM_SIZE)
    {
        if ((palReadDeviceMemory(pMac->hHdd, INTERLEAVE_OFFSET_0 + (i * sizeof(tANI_U32)), (startPtr + (i * sizeof(tANI_U32))), ONE_K_MEM_SIZE * sizeof(tANI_U32))) != eHAL_STATUS_SUCCESS)
        {
            phyLog(pMac, LOGE, "Unable to read memory\n");
            return(eHAL_STATUS_FAILURE);
        }
    }

    return(retVal);
}


//keep this here for internal debugging
eHalStatus log_grab_ram(tpAniSirGlobal pMac, tANI_U32 startSample, tANI_U32 numSamples)
{
    if (((startSample + numSamples) <= GRAB_RAM_DBLOCK_SIZE) && (numSamples <= MAX_REQUESTED_GRAB_RAM_SAMPLES))
    {
        tGrabRamSample *buffer;
        phyLog(pMac, LOGE, "dump_grab_ram: start=%d  num=%d\n", startSample, numSamples);

        if (eHAL_STATUS_SUCCESS == palAllocateMemory(pMac->hHdd, (void **)&buffer, numSamples * sizeof(tGrabRamSample)))
        {
            tANI_U32 i;

            asicGrabAdcSamples(pMac, startSample, numSamples, GRABRAM_RAWADC, buffer);

            phyLog(pMac, LOGE, "idx: \tRX0_I \tRX0_Q \tRX1_I \tRX1_Q \tRX2_I \tRX2_Q\n");

            for (i = startSample; i < (startSample + numSamples); i++)
            {
                phyLog(pMac, LOGE, "%04d: \t%-05d \t%-05d\n",
                       i,
                       buffer[i - startSample].rx0.I,
                       buffer[i - startSample].rx0.Q
                      );
            }

            if (eHAL_STATUS_SUCCESS != palFreeMemory(pMac->hHdd, buffer))
            {
                phyLog(pMac, LOGE, "Failed to free buffer from grab ram dump\n");
            }
        }
        else
        {
            phyLog(pMac, LOGE, "Failed to allocate buffer for grab ram dump\n");
        }
    }
    else
    {
        phyLog(pMac, LOGE, "Bad Param: start=%d num=%d\n", startSample, numSamples);
    }

    return(eHAL_STATUS_SUCCESS);
}


#endif /* #ifndef WLAN_FTM_STUB */






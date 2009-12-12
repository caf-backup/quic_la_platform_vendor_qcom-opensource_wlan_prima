/** ------------------------------------------------------------------------- *
    ------------------------------------------------------------------------- *


    \file asicPhyDbg.c

    \brief phyDbg module to generate transmit frames

    $Id$

    Copyright (C) 2006 Airgo Networks, Incorporated


   ========================================================================== */
#include "ani_assert.h"
#include "string.h"
#include "sys_api.h"
#include "asicPhyDbg.h"
#include "mpdu.h"




#ifdef ANI_MANF_DIAG

#if defined(ANI_LITTLE_BYTE_ENDIAN) && !defined(ANI_BIG_BYTE_ENDIAN)
#define _HTONS(A) (A)
#define _HTONL(A) (A)

#define HTONS(A)  (A)
#define HTONL(A)  (A)
#define NTOHS(A)  (A)
#define NTOHL(A)  (A)


#elif defined(ANI_BIG_BYTE_ENDIAN) && !defined(ANI_LITTLE_BYTE_ENDIAN)

#define _HTONS(A)  ((((tANI_U16)(A) & 0xff00) >> 8) | \
                    (((tANI_U16)(A) & 0x00ff) << 8)   \
                   )
#define _HTONL(A)  ((((tANI_U32)(A) & 0xff000000) >> 24) | \
                    (((tANI_U32)(A) & 0x00ff0000) >> 8)  | \
                    (((tANI_U32)(A) & 0x0000ff00) << 8)  | \
                    (((tANI_U32)(A) & 0x000000ff) << 24)   \
                   )

#define HTONS(A) { A = _HTONS(A); }
#define HTONL(A) { A = _HTONL(A); }

#define NTOHS(A)     HTONS(A)
#define NTOHL(A)     HTONL(A)

#else

#error "Either ANI_BIG_BYTE_ENDIAN or ANI_LITTLE_BYTE_ENDIAN must be #defined, but not both."

#endif


typedef enum
{
    MPI_PKT_TYPE_11A = 0,
    MPI_PKT_TYPE_11B = 1,
    MPI_PKT_TYPE_GREENFIELD = 2,
    MPI_PKT_TYPE_MIXED_11A = 3
}ePhyDbgMpiPacketType;



typedef enum
{
    MPI_BANDWIDTH_20 = 0,
    MPI_BANDWIDTH_40 = 1,
    MPI_BANDWIDTH_40_DUP = 2
}ePhyDbgMpiBandwidth;


typedef enum
{
    MPI_NSS_1 = 0,
    MPI_NSS_2 = 1,
    MPI_11B_SHORT = 0,
    MPI_11B_LONG = 1
}ePhyDbgMpiNss;

typedef enum
{
    MPI_11B_RATE_1MBPS = 0,
    MPI_11B_RATE_2MBPS = 1,
    MPI_11B_RATE_5_5MBPS = 2,
    MPI_11B_RATE_11MBPS = 3,
    MPI_11B_DONT_CARE = 0
}ePhyDbgMpi11bRate;

typedef enum
{
    MPI_NORMAL_800 = 0,
    MPI_SHORT_400 = 1,
    MPI_GUARD_DONT_CARE = 0
}ePhyDbgMpiShortGuard;


typedef struct
{
    ePhyDbgMpiBandwidth bandwidth;
    ePhyDbgMpiNss nss11b;
    ePhyDbgMpi11bRate bRateSelect;
    ePhyDbgMpiShortGuard guard;
    tANI_U8 signalCode;
    tANI_BOOLEAN airgoRate;
}sPhyDbgMpiRateSettings;

const sPhyDbgMpiRateSettings rateSettings[NUM_HAL_PHY_RATES] =
{
    //  ePhyDbgMpiBandwidth, ePhyDbgMpiNss, ePhyDbgMpi11bRate, ePhyDbgMpiShortGuard, signalCode, airgoRate
    //802.11b Rates
    {   MPI_BANDWIDTH_20, MPI_11B_LONG,  MPI_11B_RATE_1MBPS,   MPI_GUARD_DONT_CARE, 0x0, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_11B_LONG_1_MBPS,
    {   MPI_BANDWIDTH_20, MPI_11B_LONG,  MPI_11B_RATE_2MBPS,   MPI_GUARD_DONT_CARE, 0x1, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_11B_LONG_2_MBPS,
    {   MPI_BANDWIDTH_20, MPI_11B_LONG,  MPI_11B_RATE_5_5MBPS, MPI_GUARD_DONT_CARE, 0x2, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_11B_LONG_5_5_MBPS,
    {   MPI_BANDWIDTH_20, MPI_11B_LONG,  MPI_11B_RATE_11MBPS,  MPI_GUARD_DONT_CARE, 0x3, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_11B_LONG_11_MBPS,
    {   MPI_BANDWIDTH_20, MPI_11B_SHORT, MPI_11B_RATE_2MBPS,   MPI_GUARD_DONT_CARE, 0x1, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_11B_SHORT_2_MBPS,
    {   MPI_BANDWIDTH_20, MPI_11B_SHORT, MPI_11B_RATE_5_5MBPS, MPI_GUARD_DONT_CARE, 0x2, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_11B_SHORT_5_5_MBPS,
    {   MPI_BANDWIDTH_20, MPI_11B_SHORT, MPI_11B_RATE_11MBPS,  MPI_GUARD_DONT_CARE, 0x3, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_11B_SHORT_11_MBPS,

    //Titan SSF-SIMO Rates
    {   MPI_BANDWIDTH_20, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0xB, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_SSF_SIMO_6_MBPS,
    {   MPI_BANDWIDTH_20, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0xF, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_SSF_SIMO_9_MBPS,
    {   MPI_BANDWIDTH_20, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0xA, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_SSF_SIMO_12_MBPS,
    {   MPI_BANDWIDTH_20, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0xE, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_SSF_SIMO_18_MBPS,
    {   MPI_BANDWIDTH_20, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0x9, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_SSF_SIMO_24_MBPS,
    {   MPI_BANDWIDTH_20, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0xD, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_SSF_SIMO_36_MBPS,
    {   MPI_BANDWIDTH_20, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0x8, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_SSF_SIMO_48_MBPS,
    {   MPI_BANDWIDTH_20, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0xC, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_SSF_SIMO_54_MBPS,

    //Titan SSF-MIMO Rates
    {   MPI_BANDWIDTH_20, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0xD, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_TITAN_SSF_MIMO_72_MBPS,
    {   MPI_BANDWIDTH_20, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0x8, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_TITAN_SSF_MIMO_96_MBPS,
    {   MPI_BANDWIDTH_20, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0xC, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_TITAN_SSF_MIMO_108_MBPS,

    //Titan SSF-SIMO+CB Rates
    {   MPI_BANDWIDTH_40, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0xB, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_TITAN_SSF_SIMO_CB_12_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0xF, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_TITAN_SSF_SIMO_CB_18_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0x3, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_TITAN_SSF_SIMO_CB_20_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0xA, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_TITAN_SSF_SIMO_CB_24_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0xE, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_TITAN_SSF_SIMO_CB_36_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0x2, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_TITAN_SSF_SIMO_CB_40_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0x6, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_TITAN_SSF_SIMO_CB_42_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0x9, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_TITAN_SSF_SIMO_CB_48_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0xD, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_TITAN_SSF_SIMO_CB_72_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0x1, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_TITAN_SSF_SIMO_CB_80_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0x5, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_TITAN_SSF_SIMO_CB_84_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0x8, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_TITAN_SSF_SIMO_CB_96_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0xC, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_TITAN_SSF_SIMO_CB_108_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0x0, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_TITAN_SSF_SIMO_CB_120_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0x4, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_TITAN_SSF_SIMO_CB_126_MBPS,

    //Titan SSF-MIMO+CB Rates
    {   MPI_BANDWIDTH_40, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0xA, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_TITAN_SSF_MIMO_CB_48_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0xE, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_TITAN_SSF_MIMO_CB_72_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0x2, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_TITAN_SSF_MIMO_CB_80_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0x6, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_TITAN_SSF_MIMO_CB_84_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0x9, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_TITAN_SSF_MIMO_CB_96_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0xD, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_TITAN_SSF_MIMO_CB_144_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0x1, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_TITAN_SSF_MIMO_CB_160_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0x5, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_TITAN_SSF_MIMO_CB_168_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0x8, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_TITAN_SSF_MIMO_CB_192_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0xC, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_TITAN_SSF_MIMO_CB_216_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0x0, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_TITAN_SSF_MIMO_CB_240_MBPS,

    //Titan ESF-MIMO Rates
    {   MPI_BANDWIDTH_20, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0xA, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_TITAN_ESF_MIMO_24_MBPS,
    {   MPI_BANDWIDTH_20, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0xE, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_TITAN_ESF_MIMO_36_MBPS,
    {   MPI_BANDWIDTH_20, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0x2, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_TITAN_ESF_MIMO_40_MBPS,
    {   MPI_BANDWIDTH_20, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0x6, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_TITAN_ESF_MIMO_42_MBPS,
    {   MPI_BANDWIDTH_20, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0x9, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_TITAN_ESF_MIMO_48_MBPS,
    {   MPI_BANDWIDTH_20, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0xD, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_TITAN_ESF_MIMO_72_MBPS,
    {   MPI_BANDWIDTH_20, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0x1, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_TITAN_ESF_MIMO_80_MBPS,
    {   MPI_BANDWIDTH_20, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0x5, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_TITAN_ESF_MIMO_84_MBPS,
    {   MPI_BANDWIDTH_20, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0x8, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_TITAN_ESF_MIMO_96_MBPS,
    {   MPI_BANDWIDTH_20, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0xC, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_TITAN_ESF_MIMO_108_MBPS,
    {   MPI_BANDWIDTH_20, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0x0, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_TITAN_ESF_MIMO_120_MBPS,
    {   MPI_BANDWIDTH_20, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0x4, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_TITAN_ESF_MIMO_126_MBPS,

    //Titan ESF-SIMO+CB Rates
    {   MPI_BANDWIDTH_40, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0xA, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_TITAN_ESF_SIMO_CB_24_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0xE, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_TITAN_ESF_SIMO_CB_36_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0x2, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_TITAN_ESF_SIMO_CB_40_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0x6, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_TITAN_ESF_SIMO_CB_42_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0x9, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_TITAN_ESF_SIMO_CB_48_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0xD, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_TITAN_ESF_SIMO_CB_72_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0x1, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_TITAN_ESF_SIMO_CB_80_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0x5, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_TITAN_ESF_SIMO_CB_84_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0x8, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_TITAN_ESF_SIMO_CB_96_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0xC, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_TITAN_ESF_SIMO_CB_108_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0x0, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_TITAN_ESF_SIMO_CB_120_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0x4, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_TITAN_ESF_SIMO_CB_126_MBPS,

    //Titan ESF-MIMO+CB Rates
    {   MPI_BANDWIDTH_40, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0xA, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_TITAN_ESF_MIMO_CB_48_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0xE, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_TITAN_ESF_MIMO_CB_72_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0x2, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_TITAN_ESF_MIMO_CB_80_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0x6, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_TITAN_ESF_MIMO_CB_84_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0x9, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_TITAN_ESF_MIMO_CB_96_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0xD, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_TITAN_ESF_MIMO_CB_144_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0x1, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_TITAN_ESF_MIMO_CB_160_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0x5, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_TITAN_ESF_MIMO_CB_168_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0x8, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_TITAN_ESF_MIMO_CB_192_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0xC, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_TITAN_ESF_MIMO_CB_216_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0x0, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_TITAN_ESF_MIMO_CB_240_MBPS,

    //Titan Legacy Duplicate Modes (40MHz)
    {   MPI_BANDWIDTH_40_DUP, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0xB, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_DUP_SSF_SIMO_6_MBPS,
    {   MPI_BANDWIDTH_40_DUP, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0xF, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_DUP_SSF_SIMO_9_MBPS,
    {   MPI_BANDWIDTH_40_DUP, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0xA, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_DUP_SSF_SIMO_12_MBPS,
    {   MPI_BANDWIDTH_40_DUP, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0xE, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_DUP_SSF_SIMO_18_MBPS,
    {   MPI_BANDWIDTH_40_DUP, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0x9, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_DUP_SSF_SIMO_24_MBPS,
    {   MPI_BANDWIDTH_40_DUP, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0xD, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_DUP_SSF_SIMO_36_MBPS,
    {   MPI_BANDWIDTH_40_DUP, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0x8, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_DUP_SSF_SIMO_48_MBPS,
    {   MPI_BANDWIDTH_40_DUP, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0xC, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_DUP_SSF_SIMO_54_MBPS,

    //MCS Index #0-15 (20MHz)
    {   MPI_BANDWIDTH_20, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0x0, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_MCS_1NSS_6_5_MBPS,
    {   MPI_BANDWIDTH_20, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0x1, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_MCS_1NSS_13_MBPS,
    {   MPI_BANDWIDTH_20, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0x2, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_MCS_1NSS_19_5_MBPS,
    {   MPI_BANDWIDTH_20, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0x3, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_MCS_1NSS_26_MBPS,
    {   MPI_BANDWIDTH_20, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0x4, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_MCS_1NSS_39_MBPS,
    {   MPI_BANDWIDTH_20, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0x5, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_MCS_1NSS_52_MBPS,
    {   MPI_BANDWIDTH_20, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0x6, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_MCS_1NSS_58_5_MBPS,
    {   MPI_BANDWIDTH_20, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0x7, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_MCS_1NSS_65_MBPS,

    {   MPI_BANDWIDTH_20, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_SHORT_400,  0x0, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_MCS_1NSS_SG_7_2_MBPS,
    {   MPI_BANDWIDTH_20, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_SHORT_400,  0x1, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_MCS_1NSS_SG_14_4_MBPS,
    {   MPI_BANDWIDTH_20, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_SHORT_400,  0x2, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_MCS_1NSS_SG_21_7_MBPS,
    {   MPI_BANDWIDTH_20, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_SHORT_400,  0x3, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_MCS_1NSS_SG_28_9_MBPS,
    {   MPI_BANDWIDTH_20, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_SHORT_400,  0x4, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_MCS_1NSS_SG_43_3_MBPS,
    {   MPI_BANDWIDTH_20, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_SHORT_400,  0x5, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_MCS_1NSS_SG_57_8_MBPS,
    {   MPI_BANDWIDTH_20, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_SHORT_400,  0x6, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_MCS_1NSS_SG_65_MBPS,
    {   MPI_BANDWIDTH_20, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_SHORT_400,  0x7, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_MCS_1NSS_SG_72_2_MBPS,

    {   MPI_BANDWIDTH_20, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0x8, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_MCS_2NSS_13_MBPS,
    {   MPI_BANDWIDTH_20, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0x9, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_MCS_2NSS_26_MBPS,
    {   MPI_BANDWIDTH_20, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0xA, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_MCS_2NSS_39_MBPS,
    {   MPI_BANDWIDTH_20, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0xB, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_MCS_2NSS_52_MBPS,
    {   MPI_BANDWIDTH_20, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0xC, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_MCS_2NSS_78_MBPS,
    {   MPI_BANDWIDTH_20, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0xD, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_MCS_2NSS_104_MBPS,
    {   MPI_BANDWIDTH_20, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0xE, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_MCS_2NSS_117_MBPS,
    {   MPI_BANDWIDTH_20, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0xF, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_MCS_2NSS_130_MBPS,

    {   MPI_BANDWIDTH_20, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_SHORT_400,  0x8, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_MCS_2NSS_SG_14_444_MBPS,
    {   MPI_BANDWIDTH_20, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_SHORT_400,  0x9, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_MCS_2NSS_SG_28_889_MBPS,
    {   MPI_BANDWIDTH_20, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_SHORT_400,  0xA, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_MCS_2NSS_SG_43_333_MBPS,
    {   MPI_BANDWIDTH_20, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_SHORT_400,  0xB, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_MCS_2NSS_SG_57_778_MBPS,
    {   MPI_BANDWIDTH_20, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_SHORT_400,  0xC, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_MCS_2NSS_SG_86_667_MBPS,
    {   MPI_BANDWIDTH_20, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_SHORT_400,  0xD, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_MCS_2NSS_SG_115_556_MBPS,
    {   MPI_BANDWIDTH_20, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_SHORT_400,  0xE, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_MCS_2NSS_SG_130_MBPS,
    {   MPI_BANDWIDTH_20, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_SHORT_400,  0xF, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_MCS_2NSS_SG_144_444_MBPS,

    //MCS Index #0-15 (40MHz)
    {   MPI_BANDWIDTH_40, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0x0, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_MCS_1NSS_CB_13_5_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0x1, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_MCS_1NSS_CB_27_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0x2, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_MCS_1NSS_CB_40_5_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0x3, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_MCS_1NSS_CB_54_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0x4, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_MCS_1NSS_CB_81_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0x5, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_MCS_1NSS_CB_108_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0x6, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_MCS_1NSS_CB_121_5_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0x7, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_MCS_1NSS_CB_135_MBPS,

    {   MPI_BANDWIDTH_40, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_SHORT_400,  0x0, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_MCS_1NSS_SG_CB_15_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_SHORT_400,  0x1, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_MCS_1NSS_SG_CB_30_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_SHORT_400,  0x2, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_MCS_1NSS_SG_CB_45_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_SHORT_400,  0x3, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_MCS_1NSS_SG_CB_60_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_SHORT_400,  0x4, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_MCS_1NSS_SG_CB_90_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_SHORT_400,  0x5, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_MCS_1NSS_SG_CB_120_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_SHORT_400,  0x6, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_MCS_1NSS_SG_CB_135_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_SHORT_400,  0x7, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_MCS_1NSS_SG_CB_150_MBPS,

    {   MPI_BANDWIDTH_40, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0x8, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_MCS_2NSS_CB_27_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0x9, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_MCS_2NSS_CB_54_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0xA, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_MCS_2NSS_CB_81_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0xB, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_MCS_2NSS_CB_108_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0xC, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_MCS_2NSS_CB_162_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0xD, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_MCS_2NSS_CB_216_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0xE, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_MCS_2NSS_CB_243_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0xF, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_MCS_2NSS_CB_270_MBPS,

    {   MPI_BANDWIDTH_40, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_SHORT_400,  0x8, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_MCS_2NSS_SG_CB_30_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_SHORT_400,  0x9, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_MCS_2NSS_SG_CB_60_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_SHORT_400,  0xA, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_MCS_2NSS_SG_CB_90_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_SHORT_400,  0xB, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_MCS_2NSS_SG_CB_120_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_SHORT_400,  0xC, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_MCS_2NSS_SG_CB_180_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_SHORT_400,  0xD, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_MCS_2NSS_SG_CB_240_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_SHORT_400,  0xE, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_MCS_2NSS_SG_CB_270_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_SHORT_400,  0xF, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_MCS_2NSS_SG_CB_300_MBPS,

    //MCS Index #32  (40MHz duplicate mode, Nss = 1)
    {   MPI_BANDWIDTH_40_DUP, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0x0, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_MCS_1NSS_6_MBPS,
    {   MPI_BANDWIDTH_40_DUP, MPI_NSS_1, MPI_11B_DONT_CARE, MPI_SHORT_400,  0x0, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_MCS_1NSS_SG_6_7_MBPS,

    //Airgo Proprietary Rates
    {   MPI_BANDWIDTH_20, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0x4, eANI_BOOLEAN_TRUE },    // HAL_PHY_RATE_136_5_MBPS,
    {   MPI_BANDWIDTH_20, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_SHORT_400,  0x4, eANI_BOOLEAN_TRUE },    // HAL_PHY_RATE_151_7_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_NORMAL_800, 0x4, eANI_BOOLEAN_TRUE },    // HAL_PHY_RATE_283_5_MBPS,
    {   MPI_BANDWIDTH_40, MPI_NSS_2, MPI_11B_DONT_CARE, MPI_SHORT_400,  0x4, eANI_BOOLEAN_TRUE },    // HAL_PHY_RATE_315_MBPS,

    //802.11b Duplicate Rates
    {   MPI_BANDWIDTH_40_DUP, MPI_11B_LONG,  MPI_11B_RATE_1MBPS,   MPI_GUARD_DONT_CARE, 0x0, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_DUP_11B_LONG_1_MBPS,
    {   MPI_BANDWIDTH_40_DUP, MPI_11B_LONG,  MPI_11B_RATE_2MBPS,   MPI_GUARD_DONT_CARE, 0x1, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_DUP_11B_LONG_2_MBPS,
    {   MPI_BANDWIDTH_40_DUP, MPI_11B_LONG,  MPI_11B_RATE_5_5MBPS, MPI_GUARD_DONT_CARE, 0x2, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_DUP_11B_LONG_5_5_MBPS,
    {   MPI_BANDWIDTH_40_DUP, MPI_11B_LONG,  MPI_11B_RATE_11MBPS,  MPI_GUARD_DONT_CARE, 0x3, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_DUP_11B_LONG_11_MBPS,
    {   MPI_BANDWIDTH_40_DUP, MPI_11B_SHORT, MPI_11B_RATE_2MBPS,   MPI_GUARD_DONT_CARE, 0x1, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_DUP_11B_SHORT_2_MBPS,
    {   MPI_BANDWIDTH_40_DUP, MPI_11B_SHORT, MPI_11B_RATE_5_5MBPS, MPI_GUARD_DONT_CARE, 0x2, eANI_BOOLEAN_FALSE },    // HAL_PHY_RATE_DUP_11B_SHORT_5_5_MBPS
    {   MPI_BANDWIDTH_40_DUP, MPI_11B_SHORT, MPI_11B_RATE_11MBPS,  MPI_GUARD_DONT_CARE, 0x3, eANI_BOOLEAN_FALSE }     // HAL_PHY_RATE_DUP_11B_SHORT_11_MBPS,

};


typedef struct
{
#ifdef ANI_LITTLE_BIT_ENDIAN
    tANI_U32 ipg                    :30;    //interframe space in 160MHz clocks
    tANI_U32 crc                    :1;     //always 1 = calculate CRC
    tANI_U32 cat                    :1;     //always 0 = normal packet

    tANI_U32 wud                    :16;    //warmup delay
    tANI_U32 cmd_len                :8;     //8 for 11b & 11a mixed mode, 11 for greenfield rates
    tANI_U32 svc_len                :5;     //always 0
    tANI_U32 pkt_type               :3;     //ignored

    tANI_U32 pyldr_len              :16;    //number of random payload bytes to be generated
    tANI_U32 pyldf_len              :16;    //number of MPDU header bytes (+ number of pregenerated fixed payload bytes)
#else

    tANI_U32 cat                    :1;     //always 0 = normal packet
    tANI_U32 crc                    :1;     //always 1 = calculate CRC
    tANI_U32 ipg                    :30;    //interframe space in 160MHz clocks

    tANI_U32 pkt_type               :3;     //ignored
    tANI_U32 svc_len                :5;     //always 0
    tANI_U32 cmd_len                :8;     //8 for 11b & 11a mixed mode, 11 for greenfield rates
    tANI_U32 wud                    :16;    //warmup delay

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
    tANI_U32 packet_type            :3;
    tANI_U32 subband_mode           :1;
    tANI_U32 reserved_1             :1;
    tANI_U32 bandwidth_mode         :2;
    tANI_U32 beamform_enable        :1;

    //byte 2
    tANI_U32 tx_antenna_enable      :3;
    tANI_U32 reserved_2             :1;
    tANI_U32 nss_11b_mode           :2;
    tANI_U32 b_rate                 :2; //seems to have vanished from the latest mpi document, but Brian is setting this

    //byte 3
    tANI_U32 tx_demanded_power      :5;
    tANI_U32 reserved_3             :3;

    //byte 4
    tANI_U32 escort_packet          :1;
    tANI_U32 reserved_4             :1;
    tANI_U32 plcp_override          :2;
    tANI_U32 short_guard_interval   :1;
    tANI_U32 concat_packet          :1;
    tANI_U32 reserved_5             :2;

    //byte 5 & 6
    tANI_U32 psdu_length            :16;

    //byte 7
    tANI_U32 psdu_rate              :4;
    tANI_U32 airgo_11n_rates        :1;
    tANI_U32 last_psdu_flag         :1;
    tANI_U32 a_mpdu_flag            :1;
    tANI_U32 reserved_6             :1;
    
    //byte 8 & 9
    tANI_U32 ppdu_length            :11;
    tANI_U32 reserved_7             :5;

    //byte 10
    tANI_U32 ppdu_rate              :4;
    tANI_U32 reserved_8             :4;
    
    //byte 11
    tANI_U32 reserved_9             :8;
    
    //there is an unused byte here - don't forget to account for it when doing sizeof(tPhyDbgFrame)
#else

    tANI_U32 reserved_3             :3;
    tANI_U32 tx_demanded_power      :5;

    tANI_U32 b_rate                 :2;
    tANI_U32 nss_11b_mode           :2;
    tANI_U32 reserved_2             :1;
    tANI_U32 tx_antenna_enable      :3;

    tANI_U32 beamform_enable        :1;
    tANI_U32 bandwidth_mode         :2;
    tANI_U32 reserved_1             :1;
    tANI_U32 subband_mode           :1;
    tANI_U32 packet_type            :3;

    tANI_U32 command_length         :8;     //8 for 11b & 11a mixed mode, 11 for greenfield rates

    tANI_U32 reserved_6             :1;
    tANI_U32 a_mpdu_flag            :1;
    tANI_U32 last_psdu_flag         :1;
    tANI_U32 airgo_11n_rates        :1;
    tANI_U32 psdu_rate              :4;

    tANI_U32 psdu_length            :16;

    tANI_U32 reserved_5             :2;
    tANI_U32 concat_packet          :1;
    tANI_U32 short_guard_interval   :1;
    tANI_U32 plcp_override          :1;
    tANI_U32 reserved_4             :2;
    tANI_U32 escort_packet          :1;
    
    tANI_U32 reserved_9             :8;
    
    tANI_U32 reserved_8             :4;
    tANI_U32 ppdu_rate              :4;    

    tANI_U32 reserved_7             :5;
    tANI_U32 ppdu_length            :11;    

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
        DEFAULT_INTERFRAME_SPACE,   // ipg
        0,                          // crc
        0,                          // cat
        (160 * 3),                  // wud
        11,                         // cmd_len
        0,                          // svc_len
        0,                          // pkt_type
        1000,                       // pyldr_len
        24                          // pyldf_len
    },
    {
        11,                                 // command_length
        0,                                  // packet_type          {"11a":0, "11b":1, "11n_gf":2, "11n_mm":3}
        0,                                  // subband_mode         {"primary":0, "secondary":1}
        0,                                  // reserved_1
        0,                                  // bandwidth_mode       {"20":0, "cb":1, "dup":2}
        0,                                  // beamform_enable      always 0
        1,                                  // tx_antenna_enable    
        0,                                  // reserved_2
        0,                                  // nss_11b_mode         1 for 1Mbps, else 0
        0,                                  // b_rate
        16,                                 // tx_demanded_power    from halPhyGetRateForPower()
        0,                                  // reserved_3
        0,                                  // escort_packet        0 always
        0,                                  // reserved_4
        0,                                  // plcp_override            ?
        0,                                  // short_guard_interval     based on rate
        0,                                  // concat_packet            always 0
        0,                                  // reserved_5
        (sizeof(sMPDUHeader) + 1000 + 4),   // psdu_length
        0xB,                                // psdu_rate
        0,                                  // airgo_11n_rates
        1,                                  // last_psdu_flag
        0,                                  // a_mpdu_flag
        0,                                  // reserved_6
        0,                                  // ppdu_length
        0,                                  // reserved_7 
        0,                                  // ppdu_rate 
        0                                   // reserved_8
    },
    {
        DATA_FRAME_TYPE,                        // tMacFrameType frameType;
        0,                                      // tMacFrameCtrl frameCtrl;
        _HTONS(1371),                           // tANI_U16 duration;
        { 0x00, 0xDE, 0xAD, 0xBE, 0xEF, 0x00 }, // tANI_U8  MACAddress1[6];
        { 0x00, 0xCA, 0xFE, 0xBA, 0xBE, 0x00 }, // tANI_U8  MACAddress2[6];
        { 0x00, 0x11, 0x11, 0x11, 0x11, 0x00 }, // tANI_U8  MACAddress3[6];
        _HTONS(1)                               // tANI_U16 seqNum;
    }
#else
    {
        0,                          // cat
        1,                          // crc
        DEFAULT_INTERFRAME_SPACE,   // ipg
        0,                          // pkt_type
        0,                          // svc_len
        11,                         // cmd_len
        (160 * 3),                  // wud
        24,                         // pyldf_len
        1000                        // pyldr_len
    },
    {
        0,                                  // reserved_3
        16,                                 // tx_demanded_power
        0,                                  // b_rate
        0,                                  // nss_11b_mode
        0,                                  // reserved_2
        1,                                  // tx_antenna_enable
        0,                                  // beamform_enable
        0,                                  // bandwidth_mode
        0,                                  // reserved_1
        0,                                  // subband_mode
        0,                                  // packet_type
        11,                                 // command_length
        0,                                  // reserved_6
        0,                                  // a_mpdu_flag
        1,                                  // last_psdu_flag
        0,                                  // airgo_11n_rates
        0xB,                                // psdu_rate
        (sizeof(sMPDUHeader) + 1000 + 4),   // psdu_length
        0,                                  // reserved_5
        0,                                  // concat_packet
        0,                                  // short_guard_interval
        0,                                  // plcp_override
        0,                                  // reserved_4
        0,                                  // escort_packet
        0,                                  //reserved_8
        0,                                  //ppdu_rate
        0,                                  //reserved_7
        0                                   //ppdu_length 
    },
    {
        DATA_FRAME_TYPE,                        // tMacFrameType frameType;
        0,                                      // tMacFrameCtrl frameCtrl;
        _HTONS(1371),                           // tANI_U16 duration;
        { 0x00, 0xDE, 0xAD, 0xBE, 0xEF, 0x00 }, // tANI_U8  MACAddress1[6];
        { 0x00, 0xCA, 0xFE, 0xBA, 0xBE, 0x00 }, // tANI_U8  MACAddress2[6];
        { 0x00, 0x11, 0x11, 0x11, 0x11, 0x00 }, // tANI_U8  MACAddress3[6];
        _HTONS(1)                               // tANI_U16 seqNum;
    }

#endif
};

#define MIF_LSRAM_START     0x00000000L
#define MIF_LSRAM_END       0x00020000L

/*
    const tANI_U32 pythonDump[14 * 2] = 
    {
        0x00000000,     //rate 94
        0x40000000,
        0x00000000,
        0x000B1EA9,
        0x00000000,
        0x00000064,
        0x00000000,
        0x0000000B,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000068,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000027,
        0x00000000,
        0x00000015,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000007
    };
*/


/*#$-NOTE:PhyDbg script for reference: //depot/lab/nova/python/asic/taurus/phydbg.py-$#*/

extern eHalStatus halPhyGetPowerForRate(tHalHandle hHal, eHalPhyRates rate, tPowerdBm absPwrLimit, tPwrTemplateIndex *retTemplateIndex);
void printFrameFields(tpAniSirGlobal pMac, tPhyDbgFrame *frame, eHalPhyRates rate);
//static eHalStatus configPktScramblerSeed(tpAniSirGlobal pMac, tANI_U8 seed);
static eHalStatus CalcInterframeSpaceSetting(tpAniSirGlobal pMac, tANI_U32 numTestPackets, tANI_U32 interFrameSpace, tANI_U32 r_up, tANI_U32 r_down, tANI_U32 *warmup_delay, int *ifsSetting);

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
    tANI_U32 sramLoc = MIF_LSRAM_START;
    tANI_U32 numFrames;
    tPhyDbgFrame frame;

    
    assert(rate < NUM_HAL_PHY_RATES);
    //assert(interFrameSpace < (MSK_24 / ONE_MICROSECOND));
    
    memcpy(&frame, &defaultFrame, sizeof(tPhyDbgFrame));


    if (pMac->hphy.phy.test.testDisablePhyRegAccess == eANI_BOOLEAN_TRUE)
    {
        return (eHAL_STATUS_FAILURE);
    }

    {
        tANI_U32 reg;

        GET_PHY_REG(pMac->hHdd, RXCLKCTRL_APB_BLOCK_CLK_EN_REG, &reg);
        if ((reg & RXCLKCTRL_APB_BLOCK_CLK_EN_PHYDBG_MASK) == 0)
        {
            phyLog(pMac, LOGE, "PHYDBG clock not on for asicPhyDbgStartFrameGen\n");
            return (eHAL_STATUS_FAILURE);
        }
    }

    {
        tANI_U32 reg;

        GET_PHY_REG(pMac->hHdd, AGC_RX_OVERRIDE_REG, &reg);
        if (((reg & AGC_RX_OVERRIDE_ENRX_VAL_MASK) >> AGC_RX_OVERRIDE_ENRX_VAL_OFFSET) &
            ((reg & AGC_RX_OVERRIDE_OVERRIDE_EN_MASK) >> AGC_RX_OVERRIDE_OVERRIDE_EN_OFFSET)
           )
        {
            phyLog(pMac, LOGE, "frame generation won't work if any receivers are overriden enabled\n");
            return (eHAL_STATUS_FAILURE);
        }
    }


    asicPhyDbgStopFrameGen(pMac);
    
    {   //set interframe spacing and warmup_delay
        int ifsSetting;
        tANI_U32 warmup_delay;
        tANI_U32 ramp_up, ramp_down;

        GET_PHY_REG(pMac->hHdd, TXCTL_RAMP_UP_REG, &ramp_up);
        GET_PHY_REG(pMac->hHdd, TXCTL_RAMP_DOWN_REG, &ramp_down);
        
        CalcInterframeSpaceSetting(pMac, numTestPackets, interFrameSpace, ramp_up, ramp_down, &warmup_delay, &ifsSetting);
        
        frame.phyDbgHdr.ipg = ifsSetting & MSK_24;
        frame.phyDbgHdr.wud = warmup_delay & MSK_16;
    }


    SET_PHY_REG(pMac->hHdd, TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG,
                    (TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TPC_MASK |
                     TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TXFIR_MASK |
                     TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TXB_MASK |
                     TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TXA_MASK |
                     TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_TXCTL_MASK |
                     TXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_MPI_MASK
                    )
               );

    if ((retVal = rdModWrNovaField(pMac, RXCLKCTRL_APB_BLOCK_CLK_EN_REG,
                                   RXCLKCTRL_APB_BLOCK_CLK_EN_TMUX_MASK,
                                   RXCLKCTRL_APB_BLOCK_CLK_EN_TMUX_OFFSET,
                                   1
                                  )
        ) != eHAL_STATUS_SUCCESS
       )
    {
        return (retVal);
    }

    SET_PHY_REG(pMac->hHdd, TXCLKCTRL_APB_BLOCK_CLK_EN_REG,
                    (TXCLKCTRL_APB_BLOCK_CLK_EN_TPC_MASK |
                     TXCLKCTRL_APB_BLOCK_CLK_EN_MPI_MASK |
                     TXCLKCTRL_APB_BLOCK_CLK_EN_TXB_MASK |
                     TXCLKCTRL_APB_BLOCK_CLK_EN_TXA_MASK |
                     TXCLKCTRL_APB_BLOCK_CLK_EN_TXFIR_MASK |
                     TXCLKCTRL_APB_BLOCK_CLK_EN_TXCTL_MASK |
                     TXCLKCTRL_APB_BLOCK_CLK_EN_TATMUX_MASK
                    )
               );

    SET_PHY_REG(pMac->hHdd, PHYDBG_RST1_REG, 0);

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
        if (rateSettings[rate].airgoRate == eANI_BOOLEAN_TRUE)
        {
            frame.mpiHdr.airgo_11n_rates = 1;
        }

        {
            switch (preamble)
            {
                case PHYDBG_PREAMBLE_OFDM:
                    frame.mpiHdr.packet_type = 0;
                    frame.mpiHdr.psdu_rate = ((tANI_U8)rateSettings[rate].signalCode & MSK_4);
                    if( ((rate >= HAL_PHY_RATE_TITAN_SSF_SIMO_CB_12_MBPS) && (rate <= HAL_PHY_RATE_TITAN_SSF_MIMO_CB_240_MBPS)) || 
                        ((rate >= HAL_PHY_RATE_DUP_SSF_SIMO_6_MBPS) && (rate <= HAL_PHY_RATE_DUP_SSF_SIMO_54_MBPS)) 
                      )
                    {
                        frame.mpiHdr.ppdu_rate = 11;
                    }
                    else
                    {
                        frame.mpiHdr.ppdu_rate = ((tANI_U8)rateSettings[rate].signalCode & MSK_4);
                    }
                    break;
                case PHYDBG_PREAMBLE_SHORTB:
                    frame.mpiHdr.packet_type = 1;
                    break;
                case PHYDBG_PREAMBLE_LONGB:
                    frame.mpiHdr.packet_type = 1;
                    break;
                case PHYDBG_PREAMBLE_GREENFIELD:
                    frame.mpiHdr.packet_type = 2;
                    frame.mpiHdr.psdu_rate = ((tANI_U8)rateSettings[rate].signalCode & MSK_4);
                    frame.mpiHdr.ppdu_rate = ((tANI_U8)rateSettings[rate].signalCode & MSK_4);
                    break;
                case PHYDBG_PREAMBLE_MIXED:
                    frame.mpiHdr.packet_type = 3;
                    frame.mpiHdr.psdu_rate = ((tANI_U8)rateSettings[rate].signalCode & MSK_4);
                    frame.mpiHdr.ppdu_rate = 11;    //6Mbps per 11n standard for mixed mode
                    break;
                default:
                    phyLog(pMac, LOGE, "ERROR: Bad preamble type %d\n", preamble);
                    return (eHAL_STATUS_FAILURE);
            }
            frame.phyDbgHdr.pkt_type = frame.mpiHdr.packet_type;
        }

        frame.mpiHdr.tx_antenna_enable = 0;     //setting this to 0 requires the tx_fir_mode register to have the appropriate number of antennas

        if ((pMac->hphy.phyTPC.pwrCfgPresent == eANI_BOOLEAN_TRUE) &&    //must have CLPC data
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
                                              //the following param only used if the phy layer is set to regard this regulatory limit through pMac->hphy.phy.test.testTxGainIndexSource
                                              pMac->hphy.phy.regDomainInfo[pMac->hphy.phy.curRegDomain].channels[rfChannel].pwrLimit,
                                              &powerIndex)
                    ) != eHAL_STATUS_SUCCESS
                   )
                {
                    return (retVal);
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
                                              //the following param only used if the phy layer is set to regard this regulatory limit through pMac->hphy.phy.test.testTxGainIndexSource
                                              30,           //set power limitation to high value so as to be non-limiting
                                              &powerIndex)
                    ) != eHAL_STATUS_SUCCESS
                   )
                {
                    return (retVal);
                }
            }
            else
            {
                return (eHAL_STATUS_FAILURE);   //no other test modes are for closed-loop control
            }
            

            assert(powerIndex < 32);
            if (pMac->hphy.phy.test.testTpcClosedLoop == eANI_BOOLEAN_TRUE)
            {
                frame.mpiHdr.tx_demanded_power = powerIndex;
            }
            else
            {
                frame.mpiHdr.tx_demanded_power = 0;
            }
        }
        else
        {
            frame.mpiHdr.tx_demanded_power = 0;
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
        //frame.phyDbgHdr.pyldf_len = 0;

        //now fill in the payload
        switch (payloadContents)
        {
            case TEST_PAYLOAD_RAMP: //implement this later if needed
            case TEST_PAYLOAD_FILL_BYTE:
                if ((retVal = palFillMemory(pMac->hHdd, payload, payloadLength, payloadFillByte)
                    ) != eHAL_STATUS_SUCCESS
                   )
                {
                    return (retVal);
                }
                frame.phyDbgHdr.pyldf_len += payloadLength;
                frame.phyDbgHdr.pyldr_len = 0;
                break;

            case TEST_PAYLOAD_RANDOM:
                frame.phyDbgHdr.pyldr_len = payloadLength;
                break;

            case TEST_PAYLOAD_NONE: //zero length
                frame.phyDbgHdr.pyldr_len = 0;
                break;

            default:
                assert(0);
                break;
        }

        //now replicate this frame in the device internal SRAM
        //copy each byte of the frame into it's own 32-bit word in SRAM

        if (/* (pktAutoSeqNum == eANI_BOOLEAN_TRUE) && */ (numTestPackets != 0))
        {
            //changing the sequence number for each frame implies that these have to be handled individually
            tANI_U32 frameSize = sizeof(tPhyDbgFrame) + (frame.phyDbgHdr.pyldf_len - sizeof(sMPDUHeader));
            if (frame.phyDbgHdr.crc == 1)
            {
                //CRC adds 4 bytes to each frame
                frameSize += 4;
            }

            printFrameFields(pMac, &frame, rate);
            
            sramLoc = 4;    //initialize to first SRAM0 location

            //now fill memory with frames up to the number requested or until memory is reached
            for (numFrames = 0;
                 ( ((sramLoc / 2) + (frameSize * 4) < GRAB_RAM_DBLOCK_SIZE) &&
                   (numFrames < numTestPackets)
                 );
                 numFrames++
                )
            {
                if (numFrames < numTestPackets + 1)
                {
                    //not the last frame
                    frame.mpiHdr.last_psdu_flag = 0;
                }
                else
                {
                    frame.mpiHdr.last_psdu_flag = 1;
                }
                
                
                //frame.mpduHdr.seqNum++;
                //write first three 32-bit words with phyDbgHdr
                for (word = (tANI_U32 *)&frame.phyDbgHdr; word < (tANI_U32 *)&frame.mpiHdr; word++)
                {
                    tANI_U32 intByte = *word;

                    phyLog(pMac, LOG3, "Writing 0x%08X to addr 0x%08X\n", intByte, sramLoc);

                    retVal = halWriteDeviceMemory(pMac, sramLoc, (tANI_U8 *)&intByte, sizeof(tANI_U32));
                    if (retVal != eHAL_STATUS_SUCCESS)
                    {
                        return (retVal);
                    }
                    sramLoc += sizeof(tANI_U32) * 2;   //write 4 bytes, every other 32-bit word - see mif_mem_cfg = 0
                }
                
                //byteswap the duration and seqNum
                for (word = (tANI_U32 *)&frame.mpiHdr; word < (tANI_U32 *)&frame.mpduHdr; word++)
                {
                    HTONL(*word);
                }
                HTONS(frame.mpduHdr.duration);
                HTONS(frame.mpduHdr.seqNum);

                //write remaining mpiHdr and mpduHdr one byte at a time to each 32-bit word
                for (byte = (tANI_U8 *)&frame.mpiHdr; byte < ((tANI_U8 *)&frame + sizeof(tPhyDbgFrame) - 1); byte++)
                {
                    tANI_U32 intByte = *byte;

                    phyLog(pMac, LOG3, "Writing 0x%08X to addr 0x%08X\n", intByte, sramLoc);

                    retVal = halWriteDeviceMemory(pMac, sramLoc, (tANI_U8 *)&intByte, sizeof(tANI_U32));
                    if (retVal != eHAL_STATUS_SUCCESS)
                    {
                        return (retVal);
                    }
                    sramLoc += sizeof(tANI_U32) * 2;   //write 4 bytes, every other 32-bit word - see mif_mem_cfg = 0
                }

/*
                    //write payload portion one byte at a time
                    for (byte = payload; byte < payload + (frame.phyDbgHdr.pyldf_len - sizeof(sMPDUHeader)); byte++)
                    {
                        tANI_U32 intByte = *byte;
                        HTONL(intByte);
                        retVal = halWriteDeviceMemory(pMac, sramLoc, (tANI_U8 *)&intByte, sizeof(tANI_U32));
                        if (retVal != eHAL_STATUS_SUCCESS)
                        {
                            return (retVal);
                        }
                        sramLoc += 4;
                    }
*/
            }
        }
        else
        {
            //everything about the frame is already setup and since not even the sequence number is changing,
            // just setup phydbg to loop back on the single frame
            //write header portion one byte at a time
            printFrameFields(pMac, &frame, rate);

            //halWriteDeviceMemory(pMac, 0, (tANI_U8 *)&pythonDump, sizeof(pythonDump));

            //SRAM 0 & 1 are interlaced together to form 64-bit words: SRAM 1=bits 0 to 31 SRAM 0=bits 32 to 63
            //SRAM 2 & 3 are interlaced together to form 64-bit words: SRAM 3=bits 0 to 31 SRAM 2=bits 32 to 63
            // for phyDbg, we want to view SRAM 0, 1, 2, and 3 in order, so de-interlace the 32 bits words
            

            sramLoc = 4;    //initialize to first SRAM0 location

            //write first three 32-bit words with phyDbgHdr
            for (word = (tANI_U32 *)&frame.phyDbgHdr; word < (tANI_U32 *)&frame.mpiHdr; word++)
            {
                tANI_U32 intByte = *word;

                phyLog(pMac, LOG3, "Writing 0x%08X to addr 0x%08X\n", intByte, sramLoc);

                retVal = halWriteDeviceMemory(pMac, sramLoc, (tANI_U8 *)&intByte, sizeof(tANI_U32));
                if (retVal != eHAL_STATUS_SUCCESS)
                {
                    return (retVal);
                }
                sramLoc += sizeof(tANI_U32) * 2;   //write 4 bytes, every other 32-bit word - see mif_mem_cfg = 0
            }
            
            //byteswap the duration and seqNum
            for (word = (tANI_U32 *)&frame.mpiHdr; word < (tANI_U32 *)&frame.mpduHdr; word++)
            {
                HTONL(*word);
            }
            HTONS(frame.mpduHdr.duration);
            HTONS(frame.mpduHdr.seqNum);
            
            //write remaining mpiHdr and mpduHdr one byte at a time to each 32-bit word
            for (byte = (tANI_U8 *)&frame.mpiHdr; byte < ((tANI_U8 *)&frame + sizeof(tPhyDbgFrame) - 1); byte++)
            {
                tANI_U32 intByte = *byte;

                phyLog(pMac, LOG3, "Writing 0x%08X to addr 0x%08X\n", intByte, sramLoc);

                retVal = halWriteDeviceMemory(pMac, sramLoc, (tANI_U8 *)&intByte, sizeof(tANI_U32));
                if (retVal != eHAL_STATUS_SUCCESS)
                {
                    return (retVal);
                }
                sramLoc += sizeof(tANI_U32) * 2;   //write 4 bytes, every other 32-bit word - see mif_mem_cfg = 0
            }


            //write payload portion one byte at a time
/*
                for (byte = payload; byte < payload + (frame.phyDbgHdr.pyldf_len - sizeof(sMPDUHeader)); byte++)
                {
                    tANI_U32 intByte = *byte;
                    HTONL(intByte);
                    retVal = halWriteDeviceMemory(pMac, sramLoc, (tANI_U8 *)&intByte, sizeof(tANI_U32));
                    if (retVal != eHAL_STATUS_SUCCESS)
                    {
                        return (retVal);
                    }
                    sramLoc += 4;
                }
*/

            //for these frames that we don't want to increment the sequence number on, the scrambler seed will be fixed
            //This satisfies a mfg requirement for Litepoint
/*
                if ((retVal = configPktScramblerSeed(pMac, pktScramblerSeed)) != eHAL_STATUS_SUCCESS)
                {
                    return (retVal);
                }
*/
        }
    }

    //previous memory write must occur with mif_mem_cfg set to 0
    //now change it to 9 for phyDbg packet generation
    if ((retVal = rdModWrNovaField(pMac, MIF_MIF_MEM_CFG_REG,
                                   MIF_MIF_MEM_CFG_MIF_MEM_CFG_MASK,
                                   MIF_MIF_MEM_CFG_MIF_MEM_CFG_OFFSET,
                                   9    //4D x 32bits for dbg_B I/F
                                  )
        ) != eHAL_STATUS_SUCCESS
       )
    {
        return (retVal);
    }


    if (numTestPackets > PHYDBG_MAXPKTCNT_TX_MASK)
    {
        SET_PHY_REG(pMac->hHdd, PHYDBG_MAXPKTCNT_REG, PHYDBG_MAXPKTCNT_TX_MASK);
    }
    else if (numTestPackets == 0)
    {
        //continuous packets - TODO: how do we set this up and stop it?
        SET_PHY_REG(pMac->hHdd, PHYDBG_MAXPKTCNT_REG, 0);
        SET_PHY_REG(pMac->hHdd, PHYDBG_CFGMODE_REG, PHYDBG_CFGMODE_CPM1_MASK);    //continuous mode flag
    }
    else
    {
        SET_PHY_REG(pMac->hHdd, PHYDBG_MAXPKTCNT_REG, numTestPackets);
    }

    SET_PHY_REG(pMac->hHdd, PHYDBG_PLYBCK_CFG_REG,
                    (PHYDBG_PLYBCK_CFG_MPI_TXTEST_MASK | (PHYDBG_PLYBCK_CFG_TXPB_MODE_ETXPB3 << PHYDBG_PLYBCK_CFG_TXPB_MODE_OFFSET))
               );    //setup for phy to take data from phydbg instead of TXP

    SET_PHY_REG(pMac->hHdd, PHYDBG_PRBS_REG, (PHYDBG_PRBS_EN_MASK | 0x10));               //seed value
    SET_PHY_REG(pMac->hHdd, PHYDBG_PRBS_MS_REG, 0x5432);
    SET_PHY_REG(pMac->hHdd, PHYDBG_PRBS_LOAD_REG, PHYDBG_PRBS_LOAD_LOAD_MASK);

    //now setup phyDbg registers to loop this frame according to pMac->ptt.numTestPackets
    SET_PHY_REG(pMac->hHdd, PHYDBG_MAXADDR1_REG, (((sramLoc / 2) - MIF_LSRAM_START) / 4) - 1);   //set to last used dword index in sram
    //SET_PHY_REG(pMac->hHdd, PHYDBG_MAXADDR1_REG, 0xD);   //set to last used dword index in sram
    
    
    //start generating frames
    SET_PHY_REG(pMac->hHdd, PHYDBG_START1_REG, PHYDBG_START1_START_MASK);

    return (retVal);
}

void printFrameFields(tpAniSirGlobal pMac, tPhyDbgFrame *frame, eHalPhyRates rate)
{
    
    phyLog(pMac, LOGE, "%s = %d ", &rateStr[rate][0], rate);
    
    phyLog(pMac, LOGE, "phyDbgHdr.ipg               = %d ", frame->phyDbgHdr.ipg              );
    phyLog(pMac, LOGE, "phyDbgHdr.crc               = %d ", frame->phyDbgHdr.crc              );
    phyLog(pMac, LOGE, "phyDbgHdr.cat               = %d ", frame->phyDbgHdr.cat              );
    phyLog(pMac, LOGE, "phyDbgHdr.wud               = %d ", frame->phyDbgHdr.wud              );
    phyLog(pMac, LOGE, "phyDbgHdr.cmd_len           = %d ", frame->phyDbgHdr.cmd_len          );
    phyLog(pMac, LOGE, "phyDbgHdr.svc_len           = %d ", frame->phyDbgHdr.svc_len          );
    phyLog(pMac, LOGE, "phyDbgHdr.pkt_type          = %d ", frame->phyDbgHdr.pkt_type         );
    phyLog(pMac, LOGE, "phyDbgHdr.pyldf_len         = %d ", frame->phyDbgHdr.pyldf_len        );
    phyLog(pMac, LOGE, "phyDbgHdr.pyldr_len         = %d ", frame->phyDbgHdr.pyldr_len        );
    phyLog(pMac, LOGE, "mpiHdr.command_length       = %d ", frame->mpiHdr.command_length      );
    phyLog(pMac, LOGE, "mpiHdr.packet_type          = %d ", frame->mpiHdr.packet_type         );
    phyLog(pMac, LOGE, "mpiHdr.subband_mode         = %d ", frame->mpiHdr.subband_mode        );
    phyLog(pMac, LOGE, "mpiHdr.reserved_1           = %d ", frame->mpiHdr.reserved_1          );
    phyLog(pMac, LOGE, "mpiHdr.bandwidth_mode       = %d ", frame->mpiHdr.bandwidth_mode      );
    phyLog(pMac, LOGE, "mpiHdr.beamform_enable      = %d ", frame->mpiHdr.beamform_enable     );
    phyLog(pMac, LOGE, "mpiHdr.tx_antenna_enable    = %d ", frame->mpiHdr.tx_antenna_enable   );
    phyLog(pMac, LOGE, "mpiHdr.reserved_2           = %d ", frame->mpiHdr.reserved_2          );
    phyLog(pMac, LOGE, "mpiHdr.nss_11b_mode         = %d ", frame->mpiHdr.nss_11b_mode        );
    phyLog(pMac, LOGE, "mpiHdr.b_rate               = %d ", frame->mpiHdr.b_rate              );
    phyLog(pMac, LOGE, "mpiHdr.tx_demanded_power    = %d ", frame->mpiHdr.tx_demanded_power   );
    phyLog(pMac, LOGE, "mpiHdr.reserved_3           = %d ", frame->mpiHdr.reserved_3          );
    phyLog(pMac, LOGE, "mpiHdr.escort_packet        = %d ", frame->mpiHdr.escort_packet       );
    phyLog(pMac, LOGE, "mpiHdr.reserved_4           = %d ", frame->mpiHdr.reserved_4          );
    phyLog(pMac, LOGE, "mpiHdr.plcp_override        = %d ", frame->mpiHdr.plcp_override       );
    phyLog(pMac, LOGE, "mpiHdr.short_guard_interval = %d ", frame->mpiHdr.short_guard_interval);
    phyLog(pMac, LOGE, "mpiHdr.concat_packet        = %d ", frame->mpiHdr.concat_packet       );
    phyLog(pMac, LOGE, "mpiHdr.reserved_5           = %d ", frame->mpiHdr.reserved_5          );
    phyLog(pMac, LOGE, "mpiHdr.psdu_length          = %d ", frame->mpiHdr.psdu_length         );
    phyLog(pMac, LOGE, "mpiHdr.psdu_rate            = %d ", frame->mpiHdr.psdu_rate           );
    phyLog(pMac, LOGE, "mpiHdr.airgo_11n_rates      = %d ", frame->mpiHdr.airgo_11n_rates     );
    phyLog(pMac, LOGE, "mpiHdr.last_psdu_flag       = %d ", frame->mpiHdr.last_psdu_flag      );
    phyLog(pMac, LOGE, "mpiHdr.a_mpdu_flag          = %d ", frame->mpiHdr.a_mpdu_flag         );
    phyLog(pMac, LOGE, "mpiHdr.reserved_6           = %d ", frame->mpiHdr.reserved_6          );
    phyLog(pMac, LOGE, "mpiHdr.ppdu_length          = %d ", frame->mpiHdr.ppdu_length         );
    phyLog(pMac, LOGE, "mpiHdr.reserved_7           = %d ", frame->mpiHdr.reserved_7          );
    phyLog(pMac, LOGE, "mpiHdr.ppdu_rate            = %d ", frame->mpiHdr.ppdu_rate           );
    phyLog(pMac, LOGE, "mpiHdr.reserved_8           = %d ", frame->mpiHdr.reserved_8          );
    
}


#define MAX_VAL(a, b) (a > b ? a : b)


//following extracted from Brian's sendpacket.py script
#define OVERHEAD    (151)

static eHalStatus CalcInterframeSpaceSetting(tpAniSirGlobal pMac, tANI_U32 numTestPackets, tANI_U32 interFrameSpace, tANI_U32 r_up, tANI_U32 r_down, tANI_U32 *warmup_delay, int *ifsSetting)
{
    eHalStatus retVal;
    int ramp_up = r_up;
    int ramp_down = r_down;
    
    if (interFrameSpace == 0)
    {
        return (0);
    }
    
    if (numTestPackets == 0)    //sent_cont -> _send_list(ifs_alg=1)
    {
        tANI_U32 ifs_requested = ((interFrameSpace * 1000) * 4) / 25;
        int derived_ifs;
        
        derived_ifs = ifs_requested - ramp_up - ramp_down - OVERHEAD;
        
        if (derived_ifs < 0)
        {
            if (ramp_down > 0)
            {
                ramp_down += derived_ifs;
                ramp_down = MAX_VAL(0, ramp_down);
                derived_ifs = ifs_requested - ramp_up - ramp_down - OVERHEAD;
            }
            
            if ((derived_ifs < 0) && (ramp_up > 0))
            {
                ramp_up += derived_ifs;
                ramp_up = MAX_VAL(0, ramp_up);
                derived_ifs = ifs_requested - ramp_up - ramp_down - OVERHEAD;
            }
            
            if (derived_ifs < 0)
            {
                derived_ifs = 0;   // Override for the INFO field
            }
            
            SET_PHY_REG(pMac->hHdd, TXCTL_RAMP_UP_REG, ramp_up);
            SET_PHY_REG(pMac->hHdd, TXCTL_RAMP_DOWN_REG, ramp_down);
            
        }
        else
        {
            if (derived_ifs > 0xFFFF)
            {
                derived_ifs = 0xFFFF;
            }
        }
        
        *warmup_delay = derived_ifs;
        *ifsSetting = 0;
        return (eHAL_STATUS_SUCCESS);
    }
    else    //not continuous
    {
        tANI_U32 ifs_requested = ((interFrameSpace * 1000) * 4) / 25;
        int derived_ifs;
        
        derived_ifs = ifs_requested - ramp_up - ramp_down - OVERHEAD;
        
        if (derived_ifs < 0)
        {
            if (ramp_down > 0)
            {
                ramp_down += derived_ifs;
                ramp_down = MAX_VAL(0, ramp_down);
                derived_ifs = ifs_requested - ramp_up - ramp_down - OVERHEAD;
            }
            
            if ((derived_ifs < 0) && (ramp_up > 0))
            {
                ramp_up += derived_ifs;
                ramp_up = MAX_VAL(0, ramp_up);
                derived_ifs = ifs_requested - ramp_up - ramp_down - OVERHEAD;
            }
            
            if (derived_ifs < 0)
            {
                derived_ifs = 0;   // Override for the INFO field
            }
            
            SET_PHY_REG(pMac->hHdd, TXCTL_RAMP_UP_REG, ramp_up);
            SET_PHY_REG(pMac->hHdd, TXCTL_RAMP_DOWN_REG, ramp_down);
            
        }
        else
        {
            if (derived_ifs > 0xFFFFFF)
            {
                derived_ifs = 0xFFFFFF;
            }
        }
        
        *warmup_delay = 1;
        *ifsSetting = derived_ifs;
        return (eHAL_STATUS_SUCCESS);
    }
}

#if 0
    static tANI_U32 ppdu_length_calc(ePhyDbgMpiBandwidth bw_type, eHalPhyRates rate, n_ss, short_gi, n_psdu_bytes)
    {
        tANI_U32 n_sd;
        tANI_U32 n_dbpsc /* = rate in Mbps? */;
        
        if (bw_type == MPI_BANDWIDTH_40)
        {
            n_sd = 108;
        }
        else if (bw_type == MPI_BANDWIDTH_40_DUP)
        {
            n_sd = 48;
        }
        else
        {
            n_sd = 52;
        }
        
        n_dbpsc = rate;
        n_dbps = int(n_dbpsc / 12.0 * n_sd)
        n_data_symbols = int(ceil((8*n_psdu_bytes + 16.0 + 6.0) / (n_ss * n_dbps)))
        if short_gi:
            n_data_symbols = 0.9 * n_data_symbols
        ppdu_length = int(3*(ceil(n_data_symbols)+n_ss+3)-3)
        return ppdu_length
    }
#endif

#define STOP_ITER_LIMIT 10000

eHalStatus asicPhyDbgStopFrameGen(tpAniSirGlobal pMac)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
    

    SET_PHY_REG(pMac->hHdd, PHYDBG_CFGMODE_REG, PHYDBG_CFGMODE_STOPCPM1_MASK);

    {
        tANI_U32 i = 0;
        tANI_U32 reg;

        do
        {
            GET_PHY_REG(pMac->hHdd, PHYDBG_STATUS_REG, &reg);
        }
        while ((i++ < STOP_ITER_LIMIT) && (reg & PHYDBG_STATUS_TXSTATE_MASK));
    
        if (i >= STOP_ITER_LIMIT)
        {
            phyLog(pMac, LOGE, "ERROR: PhyDbg txstate=%d, Not Idle when stopped!\n", 
                        (reg & PHYDBG_STATUS_TXSTATE_MASK) >> PHYDBG_STATUS_TXSTATE_OFFSET
                  );
        }
    }

    SET_PHY_REG(pMac->hHdd, TXCTL_RAMP_UP_REG, 0);
    SET_PHY_REG(pMac->hHdd, TXCTL_RAMP_DOWN_REG, 51);

    //SET_PHY_REG(pMac->hHdd, PHYDBG_RST1_REG, 1);
    SET_PHY_REG(pMac->hHdd, PHYDBG_RST1_REG, 0);

    if ((retVal = rdModWrNovaField(pMac, MIF_MIF_MEM_CFG_REG,
                                   MIF_MIF_MEM_CFG_MIF_MEM_CFG_MASK,
                                   MIF_MIF_MEM_CFG_MIF_MEM_CFG_OFFSET,
                                   0    //set back to normal for host access
                                  )
        ) != eHAL_STATUS_SUCCESS
       )
    {
        return (retVal);
    }


    return (retVal);
}



eHalStatus asicPhyDbgQueryStatus(tpAniSirGlobal pMac, sTxFrameCounters *numFrames, ePhyDbgTxStatus *status)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
    tANI_U32 value;
    


    GET_PHY_REG(pMac->hHdd, PHYDBG_STATUS_REG, &value);
    value = ((value & PHYDBG_STATUS_TXSTATE_MASK) >> PHYDBG_STATUS_TXSTATE_OFFSET);

    *status = value;
    
    GET_PHY_REG(pMac->hHdd, TXCTL_LEGACY_PKTS_REG,         &numFrames->legacy     );
    GET_PHY_REG(pMac->hHdd, TXCTL_AIRGO_SIMO_40_PKTS_REG,  &numFrames->airgoSimo40);
    GET_PHY_REG(pMac->hHdd, TXCTL_AIRGO_MIMO_20_PKTS_REG,  &numFrames->airgoMimo20);
    GET_PHY_REG(pMac->hHdd, TXCTL_AIRGO_MIMO_40_PKTS_REG,  &numFrames->airgoMimo40);
    GET_PHY_REG(pMac->hHdd, TXCTL_EWC_GF_SIMO_20_PKTS_REG, &numFrames->ewcGfSimo20);
    GET_PHY_REG(pMac->hHdd, TXCTL_EWC_GF_MIMO_20_PKTS_REG, &numFrames->ewcGfMimo20);
    GET_PHY_REG(pMac->hHdd, TXCTL_EWC_GF_SIMO_40_PKTS_REG, &numFrames->ewcGfSimo40);
    GET_PHY_REG(pMac->hHdd, TXCTL_EWC_GF_MIMO_40_PKTS_REG, &numFrames->ewcGfMimo40);
    GET_PHY_REG(pMac->hHdd, TXCTL_EWC_MM_SIMO_20_PKTS_REG, &numFrames->ewcMmSimo20);
    GET_PHY_REG(pMac->hHdd, TXCTL_EWC_MM_MIMO_20_PKTS_REG, &numFrames->ewcMmMimo20);
    GET_PHY_REG(pMac->hHdd, TXCTL_EWC_MM_SIMO_40_PKTS_REG, &numFrames->ewcMmSimo40);
    GET_PHY_REG(pMac->hHdd, TXCTL_EWC_MM_MIMO_40_PKTS_REG, &numFrames->ewcMmMimo40);
    GET_PHY_REG(pMac->hHdd, TXCTL_TXB_SHORT_PKTS_REG,      &numFrames->txbShort   );
    GET_PHY_REG(pMac->hHdd, TXCTL_TXB_LONG_PKTS_REG,       &numFrames->txbLong    );

    SET_PHY_REG(pMac->hHdd, TXCTL_LEGACY_PKTS_REG,         0);
    SET_PHY_REG(pMac->hHdd, TXCTL_AIRGO_SIMO_40_PKTS_REG,  0);
    SET_PHY_REG(pMac->hHdd, TXCTL_AIRGO_MIMO_20_PKTS_REG,  0);
    SET_PHY_REG(pMac->hHdd, TXCTL_AIRGO_MIMO_40_PKTS_REG,  0);
    SET_PHY_REG(pMac->hHdd, TXCTL_EWC_GF_SIMO_20_PKTS_REG, 0);
    SET_PHY_REG(pMac->hHdd, TXCTL_EWC_GF_MIMO_20_PKTS_REG, 0);
    SET_PHY_REG(pMac->hHdd, TXCTL_EWC_GF_SIMO_40_PKTS_REG, 0);
    SET_PHY_REG(pMac->hHdd, TXCTL_EWC_GF_MIMO_40_PKTS_REG, 0);
    SET_PHY_REG(pMac->hHdd, TXCTL_EWC_MM_SIMO_20_PKTS_REG, 0);
    SET_PHY_REG(pMac->hHdd, TXCTL_EWC_MM_MIMO_20_PKTS_REG, 0);
    SET_PHY_REG(pMac->hHdd, TXCTL_EWC_MM_SIMO_40_PKTS_REG, 0);
    SET_PHY_REG(pMac->hHdd, TXCTL_EWC_MM_MIMO_40_PKTS_REG, 0);
    SET_PHY_REG(pMac->hHdd, TXCTL_TXB_SHORT_PKTS_REG,      0);
    SET_PHY_REG(pMac->hHdd, TXCTL_TXB_LONG_PKTS_REG,       0);
    
    numFrames->total = numFrames->legacy      +
                       numFrames->airgoSimo40 +
                       numFrames->airgoMimo20 +
                       numFrames->airgoMimo40 +
                       numFrames->ewcGfSimo20 +
                       numFrames->ewcGfMimo20 +
                       numFrames->ewcGfSimo40 +
                       numFrames->ewcGfMimo40 +
                       numFrames->ewcMmSimo20 +
                       numFrames->ewcMmMimo20 +
                       numFrames->ewcMmSimo40 +
                       numFrames->ewcMmMimo40 +
                       numFrames->txbShort    +
                       numFrames->txbLong;

    return (retVal);
}

#include "stdlib.h" //for random numbers

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



static eHalStatus DoGrabRamCapture(tpAniSirGlobal pMac);

#define MEM_DBLOCK1_ADDR    0                                           //bits 0 to 31 in sample format
#define MEM_DBLOCK2_ADDR    GRAB_RAM_DBLOCK_SIZE                         //bits 32 to 63 in sample format
#define MEM_DBLOCK3_ADDR    MEM_DBLOCK2_ADDR + GRAB_RAM_DBLOCK_SIZE      //bits 64 to 95 in sample format


#define INTERLEAVE_OFFSET_0  0
#define INTERLEAVE_OFFSET_1  (16 * 1024 * 4)

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

#if defined(ANI_MANF_DIAG) && (WNI_POLARIS_FW_OS == SIR_RTAI)
void    pal_rt_reset_watchdog(void);
#endif


/*
    asicGrabAdcSamples is designed to allow capture of any segment of the available samples of all three chains

    startSample is the index of the starting 96-bit sample, 0 if starting from the beginning of memory
    sampleBuffer contains enough space to retrieve all samples requested
    numSamples is input as the number requested, and is output as the number retrieved
*/
eHalStatus asicGrabAdcSamples(tpAniSirGlobal pMac, tANI_U32 startSample, tANI_U32 numSamples, tGrabRamSample *sampleBuffer)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
    tANI_U32 i, w;
    tANI_U32 interleavedMem[2][2 * READ_MEM_SIZE];  //provide a buffer to read READ_MEM_SIZE samples at a time

#ifdef ANI_MANF_DIAG
    if (((startSample + numSamples) > GRAB_RAM_DBLOCK_SIZE) || (numSamples > MAX_REQUESTED_GRAB_RAM_SAMPLES))
    {
        phyLog(pMac, LOGE, "Too many grab ram samples requested: start=%d num=%d\n", startSample, numSamples);
        return (eHAL_STATUS_FAILURE);
    }

    if (startSample == 0)
    {
        if (DoGrabRamCapture(pMac) != eHAL_STATUS_SUCCESS)
        {
            return (eHAL_STATUS_FAILURE);
        }
    }

    //phyLog(pMac, LOGE, "start=%d  numSamples=%d \n", startSample, numSamples);
    for (w = startSample; w < (numSamples + startSample); w += READ_MEM_SIZE)
    {
        //phyLog(pMac, LOGE, "w=%d\n", w);

        if (((retVal = halReadDeviceMemory(pMac, INTERLEAVE_OFFSET_0 + ((w * 2) * 4), (tANI_U8 *)&interleavedMem[0][0], READ_MEM_SIZE * 2 * sizeof(tANI_U32))) == eHAL_STATUS_SUCCESS) &&
            ((retVal = halReadDeviceMemory(pMac, INTERLEAVE_OFFSET_1 + ((w * 2) * 4), (tANI_U8 *)&interleavedMem[1][0], READ_MEM_SIZE * 2 * sizeof(tANI_U32))) == eHAL_STATUS_SUCCESS)
           )
        {
            tANI_U32 end = READ_MEM_SIZE;
#if (WNI_POLARIS_FW_OS == SIR_RTAI)
            //kick the watchdog timer during this long operation
            pal_rt_reset_watchdog();
#endif            

            if ((w + READ_MEM_SIZE) > (numSamples + startSample))
            {
                end = (numSamples % READ_MEM_SIZE);
            }


            for (i = 0; i < end; i++)
            {
                //interleaved memory contains blocks 0 & 1 samples interleaved in the first 16Kwords, and blocks 2 & 3 interleaved in the second 16K words
                //values read are offset-binary, thus the - 1024. Cast them to tANI_S16
                tANI_U32 x = (w + i) - startSample;

                //phyLog(pMac, LOGE, "i=%d    x=%d\n", i, x);

                sampleBuffer[x].rx0.I = (tANI_S16)(((interleavedMem[0][(i * 2) + 1] >> 0 ) & MSK_11) - 1024);
                sampleBuffer[x].rx0.Q = (tANI_S16)(((interleavedMem[0][(i * 2) + 1] >> 11) & MSK_11) - 1024);
                sampleBuffer[x].rx1.I = (tANI_S16)((((interleavedMem[0][(i * 2) + 1] >> 22) & MSK_10) + (((interleavedMem[0][(i * 2)] >> 0 ) & MSK_1) << 10)) - 1024);
                sampleBuffer[x].rx1.Q = (tANI_S16)(((interleavedMem[0][(i * 2)] >> 1) & MSK_11) - 1024);
                sampleBuffer[x].rx2.I = (tANI_S16)(((interleavedMem[0][(i * 2)] >> 12) & MSK_11) - 1024);
                sampleBuffer[x].rx2.Q = (tANI_S16)((((interleavedMem[0][(i * 2)] >> 23) & MSK_9) + (((interleavedMem[1][(i * 2) + 1] >> 0 ) & MSK_2) << 9)) - 1024);
            }
        }
        else
        {
            phyLog(pMac, LOGE, "Unable to read memory\n");
            return (retVal);
        }
    }
#else
    phyLog(pMac, LOGE, "Grab Ram capture only available in ANI_MANF_DIAG builds\n");
#endif

    return (retVal);
}

static eHalStatus DoGrabRamCapture(tpAniSirGlobal pMac)
{
    eHalStatus retVal;
    tANI_U32 capCnt;
    tANI_U32 bandwidthConfig;


    GET_PHY_REG(pMac->hHdd, AGC_BANDWIDTH_CONFIG_REG, &bandwidthConfig);
    

    //this can only be used in 40MHz mode
    SET_PHY_REG(pMac->hHdd, AGC_BANDWIDTH_CONFIG_REG, AGC_DOUBLE_CHANNEL_LOW_PRIMARY);


    {
        tANI_U32 reg;

        GET_PHY_REG(pMac->hHdd, RXCLKCTRL_APB_BLOCK_CLK_EN_REG, &reg);
        if ((reg & RXCLKCTRL_APB_BLOCK_CLK_EN_PHYDBG_MASK) == 0)
        {
            phyLog(pMac, LOGE, "PHYDBG clock not on for grab ram capture\n");
            return (eHAL_STATUS_FAILURE);
        }
    }

    if ((retVal = rdModWrNovaField(pMac, MIF_MIF_MEM_CFG_REG,
                                   MIF_MIF_MEM_CFG_MIF_MEM_CFG_MASK,
                                   MIF_MIF_MEM_CFG_MIF_MEM_CFG_OFFSET,
                                   14
                                  )
        ) != eHAL_STATUS_SUCCESS
       )
    {
        return (retVal);
    }

    if ((retVal = rdModWrNovaField(pMac, RXCLKCTRL_APB_BLOCK_CLK_EN_REG,
                                   RXCLKCTRL_APB_BLOCK_CLK_EN_TMUX_MASK,
                                   RXCLKCTRL_APB_BLOCK_CLK_EN_TMUX_OFFSET,
                                   1
                                  )
        ) != eHAL_STATUS_SUCCESS
       )
    {
        return (retVal);
    }

    SET_PHY_REG(pMac->hHdd, AGC_TESTBUS_REG, AGC_TESTBUS_AGC_RXTMUX_TMUXSEL_EAGC_80_40_2);
    SET_PHY_REG(pMac->hHdd, PHYDBG_CFGMODE_REG, 0);
    SET_PHY_REG(pMac->hHdd, PHYDBG_CAPT_CFG_REG, PHYDBG_CAPT_CFG_ADCRXFSEL_EC96_ADC_CH210 << PHYDBG_CAPT_CFG_ADCRXFSEL_OFFSET);
    SET_PHY_REG(pMac->hHdd, PHYDBG_MAXADDR0_REG, (GRAB_RAM_DBLOCK_SIZE - 1));

    SET_PHY_REG(pMac->hHdd, PHYDBG_CAPTCNT_REG, 0);    //reset capture counter before we start the new capture
    SET_PHY_REG(pMac->hHdd, PHYDBG_START0_REG, 1);     //self-clearing?

    //loop until all words captured
    do
    {
        GET_PHY_REG(pMac->hHdd, PHYDBG_CAPTCNT_REG, &capCnt);
    }while (capCnt < (GRAB_RAM_DBLOCK_SIZE));


    if ((retVal = rdModWrNovaField(pMac, MIF_MIF_MEM_CFG_REG,
                                   MIF_MIF_MEM_CFG_MIF_MEM_CFG_MASK,
                                   MIF_MIF_MEM_CFG_MIF_MEM_CFG_OFFSET,
                                   0
                                  )
        ) != eHAL_STATUS_SUCCESS
       )
    {
        return (retVal);
    }

    if ((retVal = rdModWrNovaField(pMac, RXCLKCTRL_APB_BLOCK_CLK_EN_REG,
                                   RXCLKCTRL_APB_BLOCK_CLK_EN_TMUX_MASK,
                                   RXCLKCTRL_APB_BLOCK_CLK_EN_TMUX_OFFSET,
                                   0
                                  )
        ) != eHAL_STATUS_SUCCESS
       )
    {
        return (retVal);
    }

    SET_PHY_REG(pMac->hHdd, AGC_BANDWIDTH_CONFIG_REG, bandwidthConfig);
    return (retVal);
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
            
            asicGrabAdcSamples(pMac, startSample, numSamples, buffer);
            
            phyLog(pMac, LOGE, "idx: \tRX0_I \tRX0_Q \tRX1_I \tRX1_Q \tRX2_I \tRX2_Q\n");
            
            for (i = startSample; i < (startSample + numSamples); i++)
            {
                phyLog(pMac, LOGE, "%04d: \t%-05d \t%-05d \t%-05d \t%-05d \t%-05d \t%-05d\n",
                                    i,
                                    buffer[i - startSample].rx0.I,
                                    buffer[i - startSample].rx0.Q,
                                    buffer[i - startSample].rx1.I,
                                    buffer[i - startSample].rx1.Q,
                                    buffer[i - startSample].rx2.I,
                                    buffer[i - startSample].rx2.Q
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

    return (eHAL_STATUS_SUCCESS);
}



#endif

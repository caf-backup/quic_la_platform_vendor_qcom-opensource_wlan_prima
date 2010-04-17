/*
 * Airgo Networks, Inc proprietary. All rights reserved.
 * halRateAdaptation.h: Provides all the MAC driver APIs in this file.
 * Author:    Viji Alagarsamy
 * Date:      03/07/2005
 * History:-
 * Date        Modified by            Modification Information
 *
 * --------------------------------------------------------------------------
 *
 */

#ifndef __HALRATE_ADAPTATION_H
#define __HALRATE_ADAPTATION_H


#include "aniGlobal.h"
#include "halRateAdaptApi.h"


#define RA_11B_ENABLED
#define RA_11A_ENABLED
#define RA_SPREAM_ENABLED
#define RA_HT_ENABLED
#define RA_SIMO_ENABLED
#define RA_SGI_ENABLED

/* given a bitindex, get the bitvalue from an array */
#define RA_BITVAL_GET(index, pU32) ((pU32[(index)>>5] >> ((index) & 31)) & 0x1)
#define RA_BITVAL_SET(index, pU32) (pU32[(index)>>5] |= (1 << ((index) & 31)))
#define RA_BITVAL_CLR(index, pU32) (pU32[(index)>>5] &= (~ (1 << ((index) & 31))))

/* given a macrate, get the corresponding bit in the bitarray */
#define RA_RATE_SUPPORTED(halRate, pU32) (RA_BITVAL_GET((halRate), pU32))

#define GET_RA_MIMO_MODE()    (pRaInfo->mimoMode)

/*
 * macros to make filling of a bit mask based on enum'd rates a little easier
 */
// these macros break when mask length> 32 bits (ie. more than 32 bits)

#define CREATE_MASK(width)    ((width) == 32 ? 0xfffffffful : ((1ul << (width)) - 1))

#define SET_RATE_INDEX(pU32, startBit, endBit) {\
   pU32[(startBit)/32] |= (CREATE_MASK((endBit) - (startBit) + 1) << ((startBit) & 31)); \
   if((startBit)/32 != ((endBit)/32))\
      pU32[(endBit)/32]   |= (CREATE_MASK((endBit) - (startBit) + 1) >> ((32 - ((startBit) & 31))&31));\
}

#define CLEAR_RATE_INDEX(pU32, startBit, endBit) {\
   pU32[(startBit)/32] &= ~(CREATE_MASK((endBit) - (startBit) + 1) << ((startBit) & 31)); \
   if((startBit)/32 != ((endBit)/32))\
      pU32[(endBit)/32]   &= ~(CREATE_MASK((endBit) - (startBit) + 1) >> ((32 - ((startBit) & 31))&31));\
}


// 11 A/G rates
#ifdef RA_11A_ENABLED

#define SET_11A_RATES(pU32) \
   HALRATE_SETBIT(pU32, HALRATE_11A_START,HALRATE_11A_END)
#define CLEAR_11A_RATES(pU32) \
   HALRATE_CLEARBIT(pU32, HALRATE_11A_START,HALRATE_11A_END)

// 11A Dup rates
#ifdef RA_DUP_ENABLED
#define SET_DUP_11A_RATES(pU32) \
   HALRATE_SETBIT(pU32, HALRATE_DUP_11A_START,HALRATE_DUP_11A_END)
#define CLEAR_DUP_11A_RATES(pU32) \
   HALRATE_CLEARBIT(pU32, HALRATE_DUP_11A_START,HALRATE_DUP_11A_END)
#else // 11A Dup
#define SET_DUP_11A_RATES(pU32)
#define CLEAR_DUP_11A_RATES(pU32)
#endif

#else // 11A
#define SET_11A_RATES(pU32)
#define CLEAR_11A_RATES(pU32)
// 11A Dup
#define SET_DUP_11A_RATES(pU32)
#define CLEAR_DUP_11A_RATES(pU32)
#endif

// ANI Legacy rates
#ifdef RA_ANILEGACY_ENABLED
#define SET_ANILEGACY_RATES(pU32) \
   HALRATE_SETBIT(pU32, HALRATE_MIMO_START,HALRATE_MIMO_END)

#define SET_ANIENHANCED_RATES(pU32) \
   HALRATE_SETBIT(pU32, HALRATE_CB_START,HALRATE_MIMO_CB_END)
#else 
#define SET_ANILEGACY_RATES(pU32)
#define SET_ANIENHANCED_RATES(pU32)
#endif

// Taurus rates
#ifdef RA_TAURUS_ENABLED
#define SET_TAURUS_RATES(pU32)\
   HALRATE_SETBIT(pU32, HALRATE_TAURUS_MIMO_START, HALRATE_TAURUS_MIMO_CB_END)     \

#define CLEAR_TAURUS_SGI20_RATES(pU32)do{\
   HALRATE_SETBIT(pU32, HALRATE_TAURUS_MIMO_SGI_1517, HALRATE_TAURUS_MIMO_SGI_1517)     \
}while(0)

#define CLEAR_TAURUS_SGI40_RATES(pU32)do{\
   HALRATE_SETBIT(pU32, HALRATE_TAURUS_MIMO_CB_SGI_3150, HALRATE_TAURUS_MIMO_CB_SGI_3150)     \
}while(0)

#define CLEAR_TAURUS_CB_RATES(pU32)do{\
   HALRATE_SETBIT(pU32, HALRATE_TAURUS_SIMO_CB_2835,    HALRATE_TAURUS_SIMO_CB_2835)     \
   HALRATE_SETBIT(pU32, HALRATE_TAURUS_MIMO_CB_SGI_3150,HALRATE_TAURUS_MIMO_CB_SGI_3150)     \
}while(0)
#else 
#define SET_TAURUS_RATES(pU32)
#define CLEAR_TAURUS_SGI20_RATES(pU32)
#define CLEAR_TAURUS_SGI40_RATES(pU32)
#define CLEAR_TAURUS_CB_RATES(pU32)
#endif

// 11B rates
#ifdef RA_11B_ENABLED

#define SET_11B_RATES(pU32) \
   HALRATE_SETBIT(pU32, HALRATE_11B_START,HALRATE_11B_END)
#define CLEAR_11B_RATES(pU32) \
   HALRATE_CLEARBIT(pU32, HALRATE_11B_START,HALRATE_11B_END)

// 11B Duplicates rates
#ifdef RA_DUP_ENABLED
#define SET_DUP_11B_RATES(pU32) do{\
   HALRATE_SETBIT(pU32, HALRATE_DUP_11B_START,HALRATE_DUP_11B_END)}while(0)
#define CLEAR_DUP_11B_RATES(pU32) do{\
   HALRATE_CLEARBIT(pU32, HALRATE_DUP_11B_START,HALRATE_DUP_11B_END)}while(0)
#else // 11B Duplicates rates
#define SET_DUP_11B_RATES(pU32)
#define CLEAR_DUP_11B_RATES(pU32)
#endif

// 11B short preamble
#ifdef RA_SPREAM_ENABLED
#define SET_SPREAM_11B_NONDUP_RATES(pU32) do{\
   HALRATE_SETBIT(pU32, HALRATE_SPREAM_11B_START,HALRATE_SPREAM_11B_END)}while(0)
#define CLEAR_SPREAM_11B_NONDUP_RATES(pU32)do{ \
   HALRATE_CLEARBIT(pU32, HALRATE_SPREAM_11B_START,HALRATE_SPREAM_11B_END)}while(0)

// 11B short preamble DUP rates
#ifdef RA_DUP_ENABLED
#define SET_SPREAM_DUP_11B_RATES(pU32) do{\
   HALRATE_SETBIT(pU32, HALRATE_SPREAM_DUP_11B_START,HALRATE_SPREAM_DUP_11B_END)}while(0)
#define CLEAR_SPREAM_DUP_11B_RATES(pU32) do{\
   HALRATE_CLEARBIT(pU32, HALRATE_SPREAM_DUP_11B_START,HALRATE_SPREAM_DUP_11B_END)}while(0)
#else // 11B short preamble DUP
#define SET_SPREAM_DUP_11B_RATES(pU32)
#define CLEAR_SPREAM_DUP_11B_RATES(pU32)
#endif

#else // 11B short preamble
#define SET_SPREAM_11B_NONDUP_RATES(pU32)
#define CLEAR_SPREAM_11B_NONDUP_RATES(pU32)
#endif

#else 
// 11B
#define SET_11B_RATES(pU32)
#define CLEAR_11B_RATES(pU32)
// 11B Duplicates rates
#define SET_DUP_11B_RATES(pU32)
#define CLEAR_DUP_11B_RATES(pU32)
// 11B short preamble
#define SET_SPREAM_11B_NONDUP_RATES(pU32)
#define CLEAR_SPREAM_11B_NONDUP_RATES(pU32)
// 11B short preamble DUP
#define SET_SPREAM_DUP_11B_RATES(pU32)
#define CLEAR_SPREAM_DUP_11B_RATES(pU32)
#endif

// All 11B short preamble rates
#define SET_SPREAM_11B_RATES(pU32) do{\
   SET_SPREAM_11B_NONDUP_RATES(pU32) \
   SET_SPREAM_DUP_11B_RATES(pU32);}while(0)

#define CLEAR_SPREAM_11B_RATES(pU32)do{ \
   CLEAR_SPREAM_11B_NONDUP_RATES(pU32) \
   CLEAR_SPREAM_DUP_11B_RATES(pU32);}while(0)

// 11B Duplicates rates
#define SET_DUP_ALL_11B_RATES(pU32) do{\
   SET_DUP_11B_RATES(pU32) \
   SET_SPREAM_DUP_11B_RATES(pU32)}while(0)

#define CLEAR_DUP_ALL_11B_RATES(pU32) do{\
   CLEAR_DUP_11B_RATES(pU32) \
   CLEAR_SPREAM_DUP_11B_RATES(pU32)}while(0)

// 11BG rates
#define SET_11BG_RATES(pU32) do{\
   SET_11B_RATES(pU32) \
   SET_11A_RATES(pU32)}while(0)

#define CLEAR_11BG_RATES(pU32) do{\
   CLEAR_11B_RATES(pU32) \
   CLEAR_11A_RATES(pU32)}while(0)


// *** MIMO rates ***
#ifdef RA_MIMO_ENABLED

#define SET_MIMO_NONCB_RATES(pU32)    do{\
   HALRATE_SETBIT(pU32, HALRATE_MIMO_START, HALRATE_MIMO_END) }while(0)
#define CLEAR_MIMO_NONCB_RATES(pU32)  do{\
   HALRATE_CLEARBIT(pU32, HALRATE_MIMO_START, HALRATE_MIMO_END)}while(0)

// MIMO CB rates
#ifdef RA_CB_ENABLED
#define SET_MIMO_CB_RATES(pU32)     do{\
   HALRATE_SETBIT(pU32, HALRATE_MIMO_CB_START,     HALRATE_MIMO_CB_END)}while(0)
#define CLEAR_MIMO_CB_RATES(pU32)     do{\
   HALRATE_CLEARBIT(pU32, HALRATE_MIMO_CB_START,     HALRATE_MIMO_CB_END)}while(0)
#else // MIMO CB
#define SET_MIMO_CB_RATES(pU32)
#define CLEAR_MIMO_CB_RATES(pU32)
#endif

// MIMO HT rates
#ifdef RA_HT_ENABLED
// MIMO HT Non-CB rates
#define SET_HT_MIMO_NONCB_RATES(pU32) do{\
   HALRATE_SETBIT(pU32, HALRATE_HT_MIMO_START, HALRATE_HT_MIMO_END)}while(0)
#define CLEAR_HT_MIMO_NONCB_RATES(pU32) do{\
   HALRATE_CLEARBIT(pU32, HALRATE_HT_MIMO_START, HALRATE_HT_MIMO_END)}while(0)

// MIMO HT SGI rates
#ifdef RA_SGI_ENABLED
#define SET_HT_MIMO_SGI_RATES(pU32) do{\
   HALRATE_SETBIT(pU32, HALRATE_HT_MIMO_SGI_START, HALRATE_HT_MIMO_SGI_END)}while(0)
#define CLEAR_HT_MIMO_SGI_RATES(pU32) do{\
   HALRATE_CLEARBIT(pU32, HALRATE_HT_MIMO_SGI_START, HALRATE_HT_MIMO_SGI_END)}while(0)
#else // MIMO HT SGI rates
#define SET_HT_MIMO_SGI_RATES(pU32)
#define CLEAR_HT_MIMO_SGI_RATES(pU32)
#endif

// MIMO HT CB rates
#ifdef RA_CB_ENABLED
#define SET_HT_MIMO_CB_RATES(pU32) do{\
   HALRATE_SETBIT(pU32, HALRATE_HT_MIMO_CB_START,  HALRATE_HT_MIMO_CB_END)}while(0)
#define CLEAR_HT_MIMO_CB_RATES(pU32) do{\
   HALRATE_CLEARBIT(pU32, HALRATE_HT_MIMO_CB_START,  HALRATE_HT_MIMO_CB_END)}while(0)

// MIMO HT CB SGI rates
#ifdef RA_SGI_ENABLED
#define SET_HT_MIMO_CB_SGI_RATES(pU32) do{\
   HALRATE_SETBIT(pU32, HALRATE_HT_MIMO_CB_SGI_START, HALRATE_HT_MIMO_CB_SGI_END)}while(0)
#define CLEAR_HT_MIMO_CB_SGI_RATES(pU32) do{\
   HALRATE_CLEARBIT(pU32, HALRATE_HT_MIMO_CB_SGI_START, HALRATE_HT_MIMO_CB_SGI_END)}while(0)
#else // MIMO HT CB SGI
#define SET_HT_MIMO_CB_SGI_RATES(pU32)
#define CLEAR_HT_MIMO_CB_SGI_RATES(pU32)
#endif

#else // MIMO HT CB
#define SET_HT_MIMO_CB_RATES(pU32)
#define CLEAR_HT_MIMO_CB_RATES(pU32)
#endif

#else // MIMO HT
#define SET_HT_MIMO_NONCB_RATES(pU32)
#define CLEAR_HT_MIMO_NONCB_RATES(pU32)
#endif

#else // MIMO
#define SET_MIMO_NONCB_RATES(pU32)
#define CLEAR_MIMO_NONCB_RATES(pU32)
// MIMO CB
#define SET_MIMO_CB_RATES(pU32)
#define CLEAR_MIMO_CB_RATES(pU32)
// MIMO HT
#define SET_HT_MIMO_NONCB_RATES(pU32)
#define CLEAR_HT_MIMO_NONCB_RATES(pU32)
// MIMO HT SGI rates
#define SET_HT_MIMO_SGI_RATES(pU32)
#define CLEAR_HT_MIMO_SGI_RATES(pU32)
// MIMO HT CB
#define SET_HT_MIMO_CB_RATES(pU32)
#define CLEAR_HT_MIMO_CB_RATES(pU32)
// MIMO HT CB SGI
#define SET_HT_MIMO_CB_SGI_RATES(pU32)
#define CLEAR_HT_MIMO_CB_SGI_RATES(pU32)
#endif


// *** SIMO rates ***
#ifdef RA_SIMO_ENABLED

// SIMO CB rates
#ifdef RA_CB_ENABLED
#define SET_SIMO_CB_RATES(pU32)     do{\
   HALRATE_SETBIT(pU32, HALRATE_CB_START, HALRATE_CB_END)}while(0)
#define CLEAR_SIMO_CB_RATES(pU32)     do{\
   HALRATE_CLEARBIT(pU32, HALRATE_CB_START, HALRATE_CB_END)}while(0)
#else // SIMO CB
#define SET_SIMO_CB_RATES(pU32)
#define CLEAR_SIMO_CB_RATES(pU32)
#endif

// SIMO HT rates
#ifdef RA_HT_ENABLED

// SIMO HT Non-CB rates
#define SET_HT_SIMO_NONCB_RATES(pU32) do{\
   HALRATE_SETBIT(pU32, HALRATE_HT_SIMO_START, HALRATE_HT_SIMO_END)}while(0)
#define CLEAR_HT_SIMO_NONCB_RATES(pU32) do{\
   HALRATE_CLEARBIT(pU32, HALRATE_HT_SIMO_START, HALRATE_HT_SIMO_END)}while(0)

// SIMO HT SGI rates
#ifdef RA_SGI_ENABLED
#define SET_HT_SIMO_SGI_RATES(pU32) do{\
   HALRATE_SETBIT(pU32, HALRATE_HT_SIMO_SGI_START, HALRATE_HT_SIMO_SGI_END)}while(0)
#define CLEAR_HT_SIMO_SGI_RATES(pU32) do{\
   HALRATE_CLEARBIT(pU32, HALRATE_HT_SIMO_SGI_START, HALRATE_HT_SIMO_SGI_END)}while(0)
#else // SIMO HT SGI
#define SET_HT_SIMO_SGI_RATES(pU32)
#define CLEAR_HT_SIMO_SGI_RATES(pU32)
#endif

// SIMO HT CB rates
#ifdef RA_CB_ENABLED
#define SET_HT_SIMO_CB_RATES(pU32) do{\
   HALRATE_SETBIT(pU32, HALRATE_HT_SIMO_CB_START,  HALRATE_HT_SIMO_CB_END)}while(0)
#define CLEAR_HT_SIMO_CB_RATES(pU32) do{\
   HALRATE_CLEARBIT(pU32, HALRATE_HT_SIMO_CB_START,  HALRATE_HT_SIMO_CB_END)}while(0)

// SIMO HT CB SGI rates
#ifdef RA_SGI_ENABLED
#define SET_HT_SIMO_CB_SGI_RATES(pU32) do{\
   HALRATE_SETBIT(pU32, HALRATE_HT_SIMO_CB_SGI_START, HALRATE_HT_SIMO_CB_SGI_END)}while(0)
#define CLEAR_HT_SIMO_CB_SGI_RATES(pU32) do{\
   HALRATE_CLEARBIT(pU32, HALRATE_HT_SIMO_CB_SGI_START, HALRATE_HT_SIMO_CB_SGI_END)}while(0)
#else // SIMO HT CB SGI
#define SET_HT_SIMO_CB_SGI_RATES(pU32)
#define CLEAR_HT_SIMO_CB_SGI_RATES(pU32)
#endif

// SIMO HT CB DUP rates
#ifdef RA_DUP_ENABLED
#define SET_HT_SIMO_CB_DUP_RATES(pU32) do{\
   HALRATE_SETBIT(pU32, HALRATE_HT_SIMO_CB_DUP_START, HALRATE_HT_SIMO_CB_DUP_END)}while(0)
#define CLEAR_HT_SIMO_DUP_RATES(pU32) do{\
   HALRATE_CLEARBIT(pU32, HALRATE_HT_SIMO_CB_DUP_START, HALRATE_HT_SIMO_CB_DUP_END)}while(0)

// SIMO HT CB DUP SGI rates
#ifdef RA_SGI_ENABLED
#define SET_HT_SIMO_CB_DUP_SGI_RATES(pU32) do{\
   HALRATE_CLEARBIT(pU32, HALRATE_HT_SIMO_CB_DUP_SGI_START, HALRATE_HT_SIMO_CB_DUP_SGI_END)}while(0)
#define CLEAR_HT_SIMO_CB_DUP_SGI_RATES(pU32) do{\
   HALRATE_CLEARBIT(pU32, HALRATE_HT_SIMO_CB_DUP_SGI_START, HALRATE_HT_SIMO_CB_DUP_SGI_END)}while(0)
#else // SIMO HT CB DUP SGI 
#define SET_HT_SIMO_CB_DUP_SGI_RATES(pU32)
#define CLEAR_HT_SIMO_CB_DUP_SGI_RATES(pU32)
#endif

#else // SIMO HT CB DUP
#define SET_HT_SIMO_CB_DUP_RATES(pU32)
#define CLEAR_HT_SIMO_DUP_RATES(pU32)
#endif

#else // SIMO HT CB
#define SET_HT_SIMO_CB_RATES(pU32)
#define CLEAR_HT_SIMO_CB_RATES(pU32)
// SIMO HT CB SGI
#define SET_HT_SIMO_CB_SGI_RATES(pU32)
#define CLEAR_HT_SIMO_CB_SGI_RATES(pU32)
// SIMO HT CB DUP
#define SET_HT_SIMO_CB_DUP_RATES(pU32)
#define CLEAR_HT_SIMO_DUP_RATES(pU32)
// SIMO HT CB DUP SGI 
#define SET_HT_SIMO_CB_DUP_SGI_RATES(pU32)
#define CLEAR_HT_SIMO_CB_DUP_SGI_RATES(pU32)
#endif

#else // SIMO HT
#define SET_HT_SIMO_NONCB_RATES(pU32)
#define CLEAR_HT_SIMO_NONCB_RATES(pU32)
#endif

#else 
// SIMO CB
#define SET_SIMO_CB_RATES(pU32)
#define CLEAR_SIMO_CB_RATES(pU32)
// SIMO HT
#define SET_HT_SIMO_NONCB_RATES(pU32)
#define CLEAR_HT_SIMO_NONCB_RATES(pU32)
// SIMO HT SGI
#define SET_HT_SIMO_SGI_RATES(pU32)
#define CLEAR_HT_SIMO_SGI_RATES(pU32)
// SIMO HT CB
#define SET_HT_SIMO_CB_RATES(pU32)
#define CLEAR_HT_SIMO_CB_RATES(pU32)
// SIMO HT CB SGI
#define SET_HT_SIMO_CB_SGI_RATES(pU32)
#define CLEAR_HT_SIMO_CB_SGI_RATES(pU32)
// SIMO HT CB DUP
#define SET_HT_SIMO_CB_DUP_RATES(pU32)
#define CLEAR_HT_SIMO_DUP_RATES(pU32)
// SIMO HT CB DUP SGI 
#define SET_HT_SIMO_CB_DUP_SGI_RATES(pU32)
#define CLEAR_HT_SIMO_CB_DUP_SGI_RATES(pU32)
#endif // SIMO



#define HALRATE_SETBIT(pU32, startRate, endRate) \
   SET_RATE_INDEX(pU32, (startRate), (endRate))

#define HALRATE_CLEARBIT(pU32, startRate, endRate) \
   CLEAR_RATE_INDEX(pU32, (startRate), (endRate))

#define SET_CB_RATES(pU32)do{ \
   SET_SIMO_CB_RATES(pU32)   \
   SET_MIMO_CB_RATES(pU32)}while(0)

#define CLEAR_CB_RATES(pU32)do{ \
   CLEAR_SIMO_CB_RATES(pU32)   \
   CLEAR_MIMO_CB_RATES(pU32)}while(0)

#define SET_MIMO_RATES(pU32) do{\
   SET_MIMO_NONCB_RATES(pU32)        \
   SET_MIMO_CB_RATES(pU32)}while(0)

#define CLEAR_MIMO_RATES(pU32) do{\
   CLEAR_MIMO_NONCB_RATES(pU32)        \
   CLEAR_MIMO_CB_RATES(pU32)}while(0)

#define SET_HT_SIMO_RATES(pU32) do{\
   SET_HT_SIMO_NONCB_RATES(pU32) \
   SET_HT_SIMO_CB_RATES(pU32)}while(0)

#define CLEAR_HT_SIMO_RATES(pU32) do{\
   CLEAR_HT_SIMO_NONCB_RATES(pU32) \
   CLEAR_HT_SIMO_CB_RATES(pU32)}while(0)

#define SET_HT_MIMO_RATES(pU32) do{\
   SET_HT_MIMO_NONCB_RATES(pU32) \
   SET_HT_MIMO_CB_RATES(pU32)}while(0)

#define CLEAR_HT_MIMO_RATES(pU32) do{\
   CLEAR_HT_MIMO_NONCB_RATES(pU32) \
   CLEAR_HT_MIMO_CB_RATES(pU32)}while(0)

#define SET_HT_CB_RATES(pU32) do{\
   SET_HT_SIMO_CB_RATES(pU32)       \
   SET_HT_MIMO_CB_RATES(pU32)}while(0)

#define CLEAR_HT_CB_RATES(pU32) do{\
   CLEAR_HT_SIMO_CB_RATES(pU32)       \
   CLEAR_HT_MIMO_CB_RATES(pU32)}while(0)

#define SET_HT_ALL_CB_RATES(pU32) do{\
   SET_HT_SIMO_CB_SGI_RATES(pU32)   \
   SET_HT_SIMO_CB_RATES(pU32)       \
   SET_HT_MIMO_CB_RATES(pU32)       \
   SET_HT_MIMO_CB_SGI_RATES(pU32)}while(0)

#define CLEAR_HT_ALL_CB_RATES(pU32) do{\
   CLEAR_HT_SIMO_CB_SGI_RATES(pU32)   \
   CLEAR_HT_SIMO_CB_RATES(pU32)       \
   CLEAR_HT_MIMO_CB_RATES(pU32)       \
   CLEAR_HT_MIMO_CB_SGI_RATES(pU32)}while(0)

#define SET_HT_ALL_SHORTGI20_RATES(pU32) do{\
   SET_HT_MIMO_SGI_RATES(pU32) \
   SET_HT_SIMO_SGI_RATES(pU32) }while(0)

#define CLEAR_HT_ALL_SHORTGI20_RATES(pU32) do{\
   CLEAR_HT_MIMO_SGI_RATES(pU32) \
   CLEAR_HT_SIMO_SGI_RATES(pU32);}while(0)

#define SET_HT_ALL_SHORTGI40_RATES(pU32) do{\
   SET_HT_MIMO_CB_SGI_RATES(pU32)  \
   SET_HT_SIMO_CB_SGI_RATES(pU32)}while(0)

#define CLEAR_HT_ALL_SHORTGI40_RATES(pU32) do{\
   CLEAR_HT_MIMO_CB_SGI_RATES(pU32)  \
   CLEAR_HT_SIMO_CB_SGI_RATES(pU32)}while(0)

#define SET_HT_MIMO_ALLGI_RATES(pU32) do{\
   SET_HT_MIMO_NONCB_RATES(pU32)   \
   SET_HT_MIMO_CB_RATES(pU32)      \
   SET_HT_MIMO_SGI_RATES(pU32)     \
   SET_HT_MIMO_CB_SGI_RATES(pU32)}while(0)

#define CLEAR_HT_MIMO_ALLGI_RATES(pU32) do{\
   CLEAR_HT_MIMO_NONCB_RATES(pU32) \
   CLEAR_HT_MIMO_CB_RATES(pU32)    \
   CLEAR_HT_MIMO_SGI_RATES(pU32)   \
   CLEAR_HT_MIMO_CB_SGI_RATES(pU32)}while(0)

#define SET_HT_SIMO_ALLGI_RATES(pU32) do{\
   SET_HT_SIMO_NONCB_RATES(pU32);  \
   SET_HT_SIMO_CB_RATES(pU32);     \
   SET_HT_SIMO_SGI_RATES(pU32);    \
   SET_HT_SIMO_CB_SGI_RATES(pU32);}while(0)

#define CLEAR_HT_SIMO_ALLGI_RATES(pU32) do{\
   CLEAR_HT_SIMO_NONCB_RATES(pU32);\
   CLEAR_HT_SIMO_CB_RATES(pU32);   \
   CLEAR_HT_SIMO_SGI_RATES(pU32);  \
   CLEAR_HT_SIMO_CB_SGI_RATES(pU32);}while(0)

#define SET_HT_CB_ALLGI_RATES(pU32) do{\
   SET_HT_SIMO_CB_RATES(pU32)      \
   SET_HT_MIMO_CB_RATES(pU32)      \
   SET_HT_SIMO_CB_SGI_RATES(pU32) \
   SET_HT_MIMO_CB_SGI_RATES(pU32)}while(0)

#define CLEAR_HT_CB_ALLGI_RATES(pU32) do{\
   CLEAR_HT_SIMO_CB_RATES(pU32)    \
   CLEAR_HT_MIMO_CB_RATES(pU32)    \
   CLEAR_HT_SIMO_CB_SGI_RATES(pU32) \
   CLEAR_HT_MIMO_CB_SGI_RATES(pU32)}while(0)

#define SET_HT_MCS0TO7_SIMO_SGI_RATES(pU32) do{\
   SET_HT_SIMO_SGI_RATES(pU32) \
   SET_HT_SIMO_CB_SGI_RATES(pU32)}while(0)

#define CLEAR_HT_MCS0TO7_SIMO_SGI_RATES(pU32) do{\
   CLEAR_HT_SIMO_SGI_RATES(pU32) \
   CLEAR_HT_SIMO_CB_SGI_RATES(pU32)}while(0)

#define SET_HT_MCS32_ALLGI_RATES(pU32) do{\
   SET_HT_SIMO_CB_DUP_RATES(pU32);}while(0)

#define CLEAR_HT_MCS32_SGI_RATES(pU32) do{\
   CLEAR_HT_SIMO_CB_DUP_SGI_RATES(pU32);}while(0)

// convert a given SMAC rate to LGI (caller should make sure it is an SGI rate) */
#define RA_SGI_TO_LGI(smacSgiRate)  ((smacSgiRate) & (~(1)))

/*
 * this one creates a smaller bit mask of rates of a certain type from a specified
 * start rate to the end, for eg, create a mask of rates from CB_48 to CB_END
 * by using this macro as (pU32, CB, 26)
 */
#define SET_MASK_RATES(pU32, type, sRate) \
   HALRATE_SETBIT(pU32, HALRATE_ ## type ## _ ## sRate, HALRATE_ ## type ## _END)


/* macros to check whether a Sta node pointer is a valid sta or not */
#define RA_STA_IS_NOT_VALID(pSta)   ((pSta == NULL) || (pSta->valid == 0))
#define RA_STA_IS_VALID(pSta)       (! RA_STA_IS_NOT_VALID(pSta))



/// Periodicity of rate adaptation (in ms)
#define RA_RATE_ADAPTATION_PERIOD       50

/* ------------------- Sampling parameters -------------------------- */

/* Minimum number of packets before a sample is completed */
#define RA_SAMPLE_MIN_PKTS             10

/* Number of ra sampling periods before a sample is considered sufficient */
#define RA_SAMPLE_PERIODS_MIN          50

/* Number of consecutive failures to consider it a valid sample */
#define RA_SAMPLE_FAILURES_THRESHOLD   3

/* Total number of failures to consider it a valid sample */
#define RA_TOTAL_FAILURES_THRESHOLD    7

/* Number of consecutive good samples before considering link is good */
#define RA_GOODLINK_PERSISTENCY_THRESHOLD  2

/* Number of consecutive good samples before  considering link is bad */
#define RA_BADLINK_PERSISTENCY_THRESHOLD   2

/* a link is "good" if the PER is better than this limit */
#define RA_PER_GOODLINK_SAMPLE_THRESHOLD   10

/* PER drop on a bad link to cause resampling */
#define RA_GOODLINK_JUMP_THRESHOLD         5

/* PER increase on a good link to cause resampling */
#define RA_BADLINK_JUMP_THRESHOLD          25

/* PER threshold of primary rate before checking retry rates' PER & throughput  */
#define RA_PER_BADLINK_RETRYRATE_SAMPLE_THRESHOLD    80

/*
 * Number of rate adaptation periods between exhaustive sampling
 * Since exhaustive sampling is now limited to a small set of rates
 * it is alright to make it fairly short. Making it large results
 * in the link taking a long time to recover to high rates once it
 * settles on a lower rate
 */
#define RA_SAMPLE_ALL_PERIOD           40

/* Number of extra periods to stay at a good link before a new sampling */
#define RA_GOODLINK_PERSIST_MAX    RA_SAMPLE_ALL_PERIOD

/* Number of rate adaptation periods before quick sampling */
#define RA_QUICK_SAMPLE_PERIOD         5

/* Number of rate adapt periods before we conclude link is idle */
#define RA_LINK_IDLE_LIMIT             3000

/* min diff in  sensitivity/10 of selected secondary rates */
#define RA_RETRY1_SENSITIVITY_DIFF      20

/* min diff in sensitivity/10 of selected teritary rates */
#define RA_RETRY2_SENSITIVITY_DIFF      70

/* max diff in sensitivity/10 for next higher tput rate */
/* max sens jump happens at 16QAM->64QAM which is 6.5db jump so make the sens diff here be 6.5*10=65 +5 => 70 for some tolerance */
#define RA_NEXTRATE_MAX_SENSITIVITY_DIFF  70

/* # of lower sampling rates */
#define RA_LOWERSENSITIVITY_SAMPLE_RATES  14

/* # of higher sampling rates */
#define RA_HIGHERTHRUPUT_SAMPLE_RATES     9

/* # of higher sampling rates */
#define RA_LOWERTHRUPUT_SENSITIVITY_DIFF  15

/* any per below this is equivalent to zero for tput comparison purposes
 * this corresponds roughly to the expected per due to collisions on the link
 */
#define RA_PER_IGNORE_THRESHOLD            7

/* when TX count is frozen, consecutive of TX failure count (in mib) cuase exeption
 * to RA restart from scratch
 */
#define RA_TX_FAIL_EXCEPTION_THRESHOLD     3

/* ----------------------------------------------------------------------------
 * local types
 */

#define RALOG_STATS     (LOGW)
#define RALOG_DBG       (LOG1)
#define RALOG_WARNING   (LOGW)
#define RALOG_ERROR     (LOGE)
#define RALOG_INFO      (LOG2)
#define RALOG_VERBOSE   (LOG3)
#define RALOG_CLI       (LOGE)

typedef enum eHalIeRateType{
   IERATE_11B  = 0,
   IERATE_11A,
   IERATE_POLARIS,
} tHalIeRateType;

/* Note: this may include legacy configuration, some of them are deprecated 
 Just keep the sequences and values, as these are already documented and published.
 Keep in mind that some of them are not available now. 
 You can always rely on tHalRaGlobalInfo structre for the current */
typedef enum eHalRaGlobalCfg {
   RA_GLOBCFG_NONE =0,
   RA_GLOBCFG_PER_BADLINK_JMPTHRESH,
   RA_GLOBCFG_PER_GOODLINK_JMPTHRESH,
   RA_GLOBCFG_PER_BAD_RETRY_RATE_THRESHOLD ,  /* new */
   RA_GLOBCFG_PER_GOODLINK_SAMPLETHRESH   ,
   RA_GLOBCFG_BADLINK_PERSIST_THRESH   ,
   RA_GLOBCFG_GOODLINK_PERSIST_THRESH   ,
   RA_GLOBCFG_GOODLINK_PERTHRESH_BY_SENSITIVITY_DIFF,
   RA_GLOBCFG_RA_PERIOD   ,
   RA_PER_SELECTION,
   RA_GLOBCFG_TOTAL_FAILURES_THRESHOLD   ,
   RA_GLOBCFG_SAMPLE_FAILURES_THRESHOLD,
   RA_GLOBCFG_SAMPLE_ALL_PERIOD   ,
   RA_GLOBCFG_TRIGGER_DELETE   ,
   RA_GLOBCFG_QUICK_SAMPLE_PERIOD   ,
   RA_GLOBCFG_RALOG_LEVEL   ,
   RA_GLOBCFG_RALOG_MODE   ,
   RA_GLOBCFG_SAMPLE_MIN_PKTS   ,
   RA_GLOBCFG_SAMPLE_PERIODS_MIN  ,
   RA_GLOBCFG_RETRY1_SENSITIVITY_DIFF   ,
   RA_GLOBCFG_RETRY2_SENSITIVITY_DIFF   ,
   RA_GLOBCFG_LOWER_SENSITIVITY_SAMPLE_RATES   ,
   RA_GLOBCFG_HIGHER_SENSITIVITY_SAMPLE_RATES,
   RA_GLOBCFG_BETTER_RATE_MAX_SENSITIVITY_DIFF,
   RA_GLOBCFG_LOWER_RATE_MIN_SENSITIVITY_DIFF,
   RA_GLOBCFG_BADLINK_RETRYJMP_THRESH,
   RA_GLOBCFG_LINK_IDLE_SAMPLES,
   RA_GLOBCFG_GOOD_SAMPLE_EXTRA_STAY_INC_THRESH,
   RA_GLOBCFG_PER_LOW_IGNORE_THRESH,
   RA_GLOBCFG_TX_FAIL_EXCEPTION_THRESH,
} tHalRaGlobalCfg;


#endif /* __HALRATE_ADAPTATION_H */

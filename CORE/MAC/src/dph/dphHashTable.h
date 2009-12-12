/*
 * Airgo Networks, Inc proprietary. All rights reserved.
 * This file dphHashTable.h contains the definition of the scheduler class.
 *
 * Author:      Sandesh Goel
 * Date:        02/25/02
 * History:-
 * Date            Modified by    Modification Information
 * --------------------------------------------------------------------
 *
 */

#ifndef __DPH_HASH_TABLE_H__
#define __DPH_HASH_TABLE_H__

#include "aniGlobal.h"

extern tpDphHashNode dphGetHashEntry(tpAniSirGlobal pMac, tANI_U16 staId);
/// Initialize STA state
extern tpDphHashNode dphInitStaState(tpAniSirGlobal pMac, tSirMacAddr staAddr,
        tANI_U16 staId, tANI_U8 validStaIdx);


/// check if sta is valid
static inline tANI_U8
dphStaIdValid(tpAniSirGlobal pMac, tANI_U16 staId)
{
   tpDphHashNode pSta = dphGetHashEntry(pMac, staId);

   if (pSta && pSta->valid)
       return true;
   else
       return false;
}

/// Compare MAC addresses, return true if same
static inline tANI_U8
dphCompareMacAddr(tANI_U8 addr1[], tANI_U8 addr2[])
{
#if ((defined(ANI_PPC)) && defined(ANI_OS_TYPE_RTAI_LINUX))
    /*
     * Optimized comparison to take advantage of unaligned memory accesses
     * supported by the CPU.
    * This needs to be reviewed if the CPU changes.
     */

    return (((*((tANI_U32 *) addr1) - *((tANI_U32 *) addr2)) |
         (*((tANI_U16 *) &(addr1[4])) - *((tANI_U16 *) &(addr2[4])))) == 0);
#else
    return((addr1[0] == addr2[0]) &&
       (addr1[1] == addr2[1]) &&
       (addr1[2] == addr2[2]) &&
       (addr1[3] == addr2[3]) &&
       (addr1[4] == addr2[4]) &&
       (addr1[5] == addr2[5]));
#endif
}

/// Hash table class
typedef struct
{

  /// The hash table itself
  tpDphHashNode *pHashTable;

  /// The state array
  tDphHashNode  *pDphNodeArray;
} dphHashTableClass;

/// The hash table object
extern dphHashTableClass dphHashTable;

/// Print MAC addresse
extern void dphPrintMacAddr(struct sAniSirGlobal *pMac, tANI_U8 addr[], tANI_U32);

tpDphHashNode dphLookupHashEntry(tpAniSirGlobal pMac, tANI_U8 staAddr[], tANI_U16 *pStaId);
tpDphHashNode dphLookupAssocId(tpAniSirGlobal pMac,  tANI_U16 staIdx, tANI_U16* assocId);


/// Get a pointer to the hash node
extern tpDphHashNode dphGetHashEntry(tpAniSirGlobal pMac, tANI_U16 staId);

/// Add an entry to the hash table
extern tpDphHashNode dphAddHashEntry(tpAniSirGlobal pMac, tSirMacAddr staAddr, tANI_U16 staId);

#if 1 //def PLM_WDS
extern tpDphHashNode dphAddHashEntryWds(tpAniSirGlobal pMac, tSirMacAddr staAddr, tANI_U16 staId);
#endif

/// Delete an entry from the hash table
extern tSirRetStatus dphDeleteHashEntry(tpAniSirGlobal pMac, tSirMacAddr staAddr, tANI_U16 staId);

void dphHashTableClassInit(tpAniSirGlobal pMac);

#endif

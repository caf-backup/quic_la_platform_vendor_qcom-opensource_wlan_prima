
#if !defined( __SMEINTERNAL_H )
#define __SMEINTERNAL_H


/**=========================================================================
  
  \file  smeInternal.h
  
  \brief prototype for SME internal structures and APIs used for SME and MAC
  
   Copyright 2008 (c) Qualcomm, Incorporated.  All Rights Reserved.
   
   Qualcomm Confidential and Proprietary.
  
  ========================================================================*/

/* $Header$ */

/*--------------------------------------------------------------------------
  Include Files
  ------------------------------------------------------------------------*/
#include "vos_status.h"
#include "vos_lock.h"
#include "vos_trace.h"
#include "vos_memory.h"
#include "vos_types.h"
#include "csrLinkList.h"

/*-------------------------------------------------------------------------- 
  Type declarations
  ------------------------------------------------------------------------*/

typedef enum eSmeCommandType 
{
    eSmeNoCommand = 0, 
    eSmeDropCommand,
    //CSR
    eSmeCsrCommandMask = 0x10000,   //this is not a command, it is to identify this is a CSR command
    eSmeCommandScan,
    eSmeCommandRoam, 
    eSmeCommandWmStatusChange, 
    eSmeCommandSetKey,
    eSmeCommandRemoveKey,
    //PMC
    eSmePmcCommandMask = 0x20000, //To identify PMC commands
    eSmeCommandEnterImps,
    eSmeCommandExitImps,
    eSmeCommandEnterBmps,
    eSmeCommandExitBmps,
    eSmeCommandEnterUapsd,
    eSmeCommandExitUapsd,
    eSmeCommandEnterWowl,
    eSmeCommandExitWowl,
    eSmeCommandEnterStandby,
    //QOS
    eSmeQosCommandMask = 0x40000,  //To identify Qos commands
    eSmeCommandAddTs,
    eSmeCommandDelTs,
#ifdef FEATURE_INNAV_SUPPORT
	eSmeCommandMeas = 0x50000, //To identify the InNav meas commands
#endif
} eSmeCommandType;


typedef enum eSmeState
{
    SME_STATE_STOP,
    SME_STATE_START,
    SME_STATE_READY,
} eSmeState;


#define SME_IS_START(pMac)  (SME_STATE_STOP != (pMac)->sme.state)
#define SME_IS_READY(pMac)  (SME_STATE_READY == (pMac)->sme.state)


typedef struct tagSmeStruct
{
    eSmeState state;
    vos_lock_t lkSmeGlobalLock;
    tANI_U32 totalSmeCmd;
    void *pSmeCmdBufAddr;
    tDblLinkList smeCmdActiveList;
    tDblLinkList smeCmdPendingList;
    tDblLinkList smeCmdFreeList;   //preallocated roam cmd list
} tSmeStruct, *tpSmeStruct;


#endif //#if !defined( __SMEINTERNAL_H )

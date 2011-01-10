/*===========================================================================
  @file wlan_qct_ssc.c

  @brief This source file is the implementation of the SIF Software 
  Controller (SSC) module and provides services to communicate over SDIO
  with the SIF hardware block.

  The SIF module is a hardware component running on a companion ASIC with
  WLAN functionality (such as Libra), and the SSC can be treated as a host
  software driver running on the MSM to transfer requests for SIF access
  over the SDIO bus.
  Please see SSC HLD and LLD documents in order to understand the callflows
  and behavior expected of SSC clients.

  The SSC implementation is divided into external APIs and the core state
  machine (per LLD).
  The events to the state machine represent only those which impact the state
  machine and does not include all external APIs and callbacks.


  Key coding conventions:

  - QCT coding standard - 80-V1716-1 C

  - Other conventions (where standard allows user discretion)

    a) WLANSSC is module prefix
    b) eVariableName means VariableName is of an enumeration type
    c) stVariableName means VariableName is of a structure type
    d) hObjectHandle means ObjectHandle is a handle to object



  
  Copyright (c) 2008 QUALCOMM Incorporated.
  All Rights Reserved.
  Qualcomm Confidential and Proprietary
===========================================================================*/

/*=========================================================================== 
    
                       EDIT HISTORY FOR FILE 
   
   
  This section contains comments describing changes made to the module. 
  Notice that changes are listed in reverse chronological order. 
   
   
  $Header:$ $DateTime: $ $Author: $ 
   
   
  when        who    what, where, why 
  --------    ---    --------------------------------------------------------
  05/19/08    lyr     Created module. 
     
===========================================================================*/ 

/*---------------------------------------------------------------------------
 * Include Files
 * ------------------------------------------------------------------------*/

/* SSC related includes                                                    */
#include "wlan_qct_ssc.h"
#include "wlan_qct_ssc_defs.h"

/* Operating system constructs                                             */
#include "vos_types.h"
#include "vos_status.h"
#include "vos_packet.h"
#include "vos_lock.h"
#include "vos_mq.h"
#include "vos_memory.h"
#include "vos_threads.h"
#include "sscDebug.h"

/* SDIO services                                                           */
#include "wlan_qct_sal.h"

/*---------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * ------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------
   WLANSSC_COOKIE

   Used as sanity check to make sure the instance we are operating on is
   legitimate.

   Should be set when SSC instance is created, and zeroed out when the
   instance is deleted.
---------------------------------------------------------------------------*/
#define WLANSSC_COOKIE 0x6C797972

/*---------------------------------------------------------------------------
   WLANSSC_ASSERT

   This is a placeholder for actual VOS MACRO to force an assert

   test is the check to be performed for the assert (standard usage)
---------------------------------------------------------------------------*/
#define WLANSSC_ASSERT(test_)                                               \
do                                                                          \
{                                                                           \
  VOS_ASSERT((test_));                                                      \
} while ( 0 )

/*---------------------------------------------------------------------------
   WLANSSC_LOCKTX

   This macro is used to acquire the SSC tx lock for the SSC controlblock
---------------------------------------------------------------------------*/
#define WLANSSC_LOCKTX(pControlBlock_)                                       \
do                                                                          \
{                                                                           \
  if( VOS_STATUS_SUCCESS != vos_lock_acquire( &((pControlBlock_)->stSSCTxLock) ) ) \
  {                                                                         \
    WLANSSC_ASSERT( 0 );                                                    \
    SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "Error acquiring SSC lock"));                     \
  }                                                                         \
} while ( 0 )


/*---------------------------------------------------------------------------
   WLANSSC_UNLOCKTX

   This macro is used to release the SSC tx lock for the SSC controlblock
---------------------------------------------------------------------------*/
#define WLANSSC_UNLOCKTX(pControlBlock_)                                    \
do                                                                          \
{                                                                           \
  if( VOS_STATUS_SUCCESS != vos_lock_release( &((pControlBlock_)->stSSCTxLock) ) ) \
  {                                                                         \
    WLANSSC_ASSERT( 0 );                                                    \
    SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "Error releasing SSC lock"));                     \
  }                                                                         \
} while ( 0 )


/*---------------------------------------------------------------------------
   WLANSSC_LOCKRX

   This macro is used to acquire the SSC rx lock for the SSC controlblock
---------------------------------------------------------------------------*/
#define WLANSSC_LOCKRX(pControlBlock_)                                       \
do                                                                          \
{                                                                           \
  if( VOS_STATUS_SUCCESS != vos_lock_acquire( &((pControlBlock_)->stSSCRxLock) ) ) \
  {                                                                         \
    WLANSSC_ASSERT( 0 );                                                    \
    SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "Error acquiring SSC lock"));                     \
  }                                                                         \
} while ( 0 )


/*---------------------------------------------------------------------------
   WLANSSC_UNLOCKRX

   This macro is used to release the SSC rx lock for the SSC controlblock
---------------------------------------------------------------------------*/
#define WLANSSC_UNLOCKRX(pControlBlock_)                                    \
do                                                                          \
{                                                                           \
  if( VOS_STATUS_SUCCESS != vos_lock_release( &((pControlBlock_)->stSSCRxLock) ) ) \
  {                                                                         \
    WLANSSC_ASSERT( 0 );                                                    \
    SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "Error releasing SSC lock"));                     \
  }                                                                         \
} while ( 0 )


/*---------------------------------------------------------------------------
   WLANSSC_LOCKTXRX

   This macro is used to acquire both the SSC tx and rx locks for the SSC
   controlblock
---------------------------------------------------------------------------*/
#define WLANSSC_LOCKTXRX(pControlBlock_)                                       \
do                                                                          \
{                                                                           \
  if( VOS_STATUS_SUCCESS != vos_lock_acquire( &((pControlBlock_)->stSSCTxLock) ) ) \
  {                                                                         \
    WLANSSC_ASSERT( 0 );                                                    \
    SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "Error acquiring SSC lock"));                     \
  }                                                                         \
  if( VOS_STATUS_SUCCESS != vos_lock_acquire( &((pControlBlock_)->stSSCRxLock) ) ) \
  {                                                                         \
    WLANSSC_ASSERT( 0 );                                                    \
    SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "Error acquiring SSC lock"));                     \
  }                                                                         \
} while ( 0 )


/*---------------------------------------------------------------------------
   WLANSSC_UNLOCKTXRX

   This macro is used to release both the SSC tx and rx locks for the SSC
   controlblock
---------------------------------------------------------------------------*/
#define WLANSSC_UNLOCKTXRX(pControlBlock_)                                  \
do                                                                          \
{                                                                           \
  if( VOS_STATUS_SUCCESS != vos_lock_release( &((pControlBlock_)->stSSCRxLock) ) ) \
  {                                                                         \
    WLANSSC_ASSERT( 0 );                                                    \
    SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "Error releasing SSC lock"));                     \
  }                                                                         \
  if( VOS_STATUS_SUCCESS != vos_lock_release( &((pControlBlock_)->stSSCTxLock) ) ) \
  {                                                                         \
    WLANSSC_ASSERT( 0 );                                                    \
    SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "Error releasing SSC lock"));                     \
  }                                                                         \
} while ( 0 )


/*---------------------------------------------------------------------------
   WLANSSC_ISCONTEXTVALID

   This checks whether a particular SSC context has been properly allocated
---------------------------------------------------------------------------*/
#define WLANSSC_ISCONTEXTVALID(pContext_)                                    \
        ( ( (NULL != (WLANSSC_ControlBlockType *)(pContext_) ) &&            \
            (VOS_FALSE != (((WLANSSC_ControlBlockType *)(pContext_))->bInitialized)) && \
            (WLANSSC_COOKIE == (((WLANSSC_ControlBlockType *)(pContext_))->nCookie)) )  \
            ? VOS_TRUE : VOS_FALSE )


/*---------------------------------------------------------------------------
   WLANSSC_MAXPKTSIZE

   Size of the largest possible data packet that we can receive over the bus
   (this is the size of the largest expected *single* packet by upper layer)
---------------------------------------------------------------------------*/
#define WLANSSC_MAXPKTSIZE 0x700

/*---------------------------------------------------------------------------
   WLANSSC_MAXTXBUFSIZE

   Size of the largest aggregated frame we can transmit over the bus
---------------------------------------------------------------------------*/
#define WLANSSC_MAXTXBUFSIZE 0x4000

/*---------------------------------------------------------------------------
   WLANSSC_MAXRXBUFSIZE

   Size of the largest aggregated frame we can receive over the bus
---------------------------------------------------------------------------*/
#define WLANSSC_MAXRXBUFSIZE 0x4000

/*---------------------------------------------------------------------------
   WLANSSC_MINPKTSIZE

   Just the size of headers in a packet, in case data component is 0
   (used to calculate the worst possible alignment guard space needed
---------------------------------------------------------------------------*/
#define WLANSSC_MINPKTSIZE  60

/*---------------------------------------------------------------------------
   WLANSSC_MAXTXDXEALIGNMENT

   This is the maximum possible alignment between packets in an aggregated
   tx buffer
   Since DXE requires 4 byte alignment, this is 3 bytes
---------------------------------------------------------------------------*/
#define WLANSSC_MAXTXDXEALIGNMENT 3


/*---------------------------------------------------------------------------
   WLANSSC_TX_DELAY_THRESHOLD

   There is a possible delay before sending a buffer smaller than this threshold
   The reason is to increase the chance of aggregation on TX side.
---------------------------------------------------------------------------*/
#define WLANSSC_TX_DELAY_THRESHOLD   0x800

/*---------------------------------------------------------------------------
   WLANSSC_flowSUSPENDEDMASK

   Bitmask to represent suspended flows.
   Flow may be tx, rx or all.

   Tx implies that transmit flow has been suspended
   Rx implies that a receive flow has been suspended
   All implies that SSC has been suspended completely - move to SUSPENDED_S

---------------------------------------------------------------------------*/
#define WLANSSC_TXSUSPENDEDMASK   0x0001
#define WLANSSC_RXSUSPENDEDMASK   0x0002
#define WLANSSC_ALLSUSPENDEDMASK  0x0003


/*---------------------------------------------------------------------------
   WLANSSC_SDIOFUNCTIONn

  Define to specify function 0 and function 1
---------------------------------------------------------------------------*/
#define WLANSSC_SDIOFUNCTION0  WLANSAL_FUNCTION_ZERO
#define WLANSSC_SDIOFUNCTION1  WLANSAL_FUNCTION_ONE

/*---------------------------------------------------------------------------
   WLANSSC_REGISTERSIZE

  Register size on the Libra - it is 32-bits
---------------------------------------------------------------------------*/
#define WLANSSC_REGISTERSIZE  4

/*---------------------------------------------------------------------------
   WLANSSC_INVALIDINTERRUPTSNAPSHOT

  Represents an invalid snapshot : this means this bitmask should be ignored.
---------------------------------------------------------------------------*/
#define WLANSSC_INVALIDINTERRUPTSNAPSHOT   0xFFFFFFFF


/*---------------------------------------------------------------------------
   WLANSSC_EDINTERRUPTMASK

  Represents the set of bits in ED that hold the interrupts
---------------------------------------------------------------------------*/
#define WLANSSC_EDINTERRUPTMASK   0xFFFF


/*---------------------------------------------------------------------------
   WLANSSC_NONASICINTERRUPTMASK

  Represents the set of interrupt bits that SSC is interested in; all other
  bits represent those that a higher level entity might be interested in

  This holds the set of interrupts that are not allowed to be touched by the
  ASICInterrupt APIs - and needs to evolve as more SSC interrupts are added
---------------------------------------------------------------------------*/
#define WLANSSC_NONASICINTERRUPTMASK   0xC001FFFE


/*---------------------------------------------------------------------------
   WLANSSC_RESETLOOPCOUNT

  Maximum number of times to loop for the reset
---------------------------------------------------------------------------*/
#define WLANSSC_RESETLOOPCOUNT  200

/*---------------------------------------------------------------------------
   WLANSSC_RESETLOOPWAIT

  Time to wait for each count of the reset loop in msec
---------------------------------------------------------------------------*/
#define WLANSSC_RESETLOOPWAIT  500


/*---------------------------------------------------------------------------
   WLANSSC_SUSPENDWLANWAIT

  Time to wait after setting the SuspendWLAN bit in SIF_BAR4
---------------------------------------------------------------------------*/
/* This large wait time appears 
 * to be required for 
 * standby mode testing in FPGA.
 */
#ifdef LIBRA_FPGA 
#define WLANSSC_SUSPENDWLANWAIT  10000
#else
#define WLANSSC_SUSPENDWLANWAIT  10
#endif

/*---------------------------------------------------------------------------
   WLANSSC_RESUMEWLANWAIT

  Time to wait after setting the SuspendWLAN bit in SIF_BAR4
---------------------------------------------------------------------------*/
/* This large wait time appears 
 * to be required for 
 * standby mode testing in FPGA.
 */
#ifdef LIBRA_FPGA 
#define WLANSSC_RESUMEWLANWAIT   10000
#else
#define WLANSSC_RESUMEWLANWAIT   10
#endif

/*---------------------------------------------------------------------------
   WLANSSC_RESUMELOOPCOUNT

  Number of times to loop for PMU Blocked bit to be cleared in SIF_BAR4. 
  Each loop waits for a delay of WLANSSC_RESUMEWLANWAIT ms.
---------------------------------------------------------------------------*/
#define WLANSSC_RESUMELOOPCOUNT  20

/*---------------------------------------------------------------------------
   WLANSSC_TXFIFOFULLDURATIONTIMEOUT

  TX FIFO FULL DURATION TIMEOUT
---------------------------------------------------------------------------*/
#define WLANSSC_TXFIFOFULLDURATIONTIMEOUT  1

/*---------------------------------------------------------------------------
   WLANSSC_CLOCKFREQ

  App domain clock frequency
---------------------------------------------------------------------------*/
#define WLANSSC_APPDOMAINCLOCKFREQ  20000000

/*---------------------------------------------------------------------------
   WLANSSC_BLOCKSIZE

  Block size configured for the SDIO transfers
---------------------------------------------------------------------------*/
#define WLANSSC_BLOCKSIZE  128

/*---------------------------------------------------------------------------
   WLANSSC_SDIOCLOCKRATE

  Clock rate configured for the SDIO transfers
---------------------------------------------------------------------------*/
#define WLANSSC_SDIOCLOCKRATE  WLANSAL_SDIO_CSPEED_HIGH_MODE

/*---------------------------------------------------------------------------
   WLANSSC_RXYIELDTOTXTHRESHOLD

  Number of rx buffers to process (from ProcessRxData) before yielding to tx
---------------------------------------------------------------------------*/
#define WLANSSC_RXYIELDTOTXTHRESHOLD  3 


/*---------------------------------------------------------------------------
 * Type Declarations
 * ------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
   WLANSSC_MessageType

   This lists the messages that the SSC can post to itself
---------------------------------------------------------------------------*/
typedef enum
{
  WLANSSC_INTPENDING_MESSAGE     = 0,
  WLANSSC_TXPENDING_MESSAGE      = 1,
  WLANSSC_RXPENDING_MESSAGE      = 2,
  WLANSSC_MEMAVAIL_MESSAGE       = 3,

  WLANSSC_MAX_MESSAGE

} WLANSSC_MessageType;


/*---------------------------------------------------------------------------
   WLANSSC_RegBufferType

   This specifies the type of reg buffer to use: tx thread or interrupt thread
   Please note that this is required since register accesses can happen in
   any thread context and since a dma buffer ptr is required, two different
   threads (tx/int) require two different buffers.
   It is left to caller to distinguish which one to use

   Note: since external APIs are protected, they are free to use either
---------------------------------------------------------------------------*/
typedef enum
{
  WLANSSC_TX_REGBUFFER     = 0,
  WLANSSC_INT_REGBUFFER    = 1,

  WLANSSC_MAX_REGBUFFER

} WLANSSC_RegBufferType;


/*---------------------------------------------------------------------------
   WLANSSC_StateType

   This lists the states that the SSC State Machine can operate in
---------------------------------------------------------------------------*/
typedef enum
{
  WLANSSC_CLOSED_STATE          = 0,
  WLANSSC_OPEN_STATE            = 1,
  WLANSSC_READY_STATE           = 2,
  WLANSSC_SUSPENDED_STATE       = 3,

  WLANSSC_MAX_STATE

} WLANSSC_StateType;


 /*---------------------------------------------------------------------------
   WLANSSC_EventType

   This lists the events that the SSC State Machine can receive
---------------------------------------------------------------------------*/
typedef enum
{
  /* External API generated events                                         */
  WLANSSC_OPEN_EVENT                 =  0,
  WLANSSC_CLOSE_EVENT                =  1,
  WLANSSC_START_EVENT                =  2,
  WLANSSC_STOP_EVENT                 =  3,
  WLANSSC_SUSPEND_EVENT              =  4,
  WLANSSC_RESUME_EVENT               =  5,
  WLANSSC_RESET_EVENT                =  6,

  /* Scheduler related events                                              */
  WLANSSC_TRANSMIT_EVENT             =  7,
  WLANSSC_RECEIVE_EVENT              =  8,
  WLANSSC_SDIOINTERRUPT_EVENT        =  9,

  /* Asynchronous indications                                              */
  WLANSSC_FATALERROR_EVENT           = 10,

  WLANSSC_MAX_EVENT

} WLANSSC_EventType;


/*---------------------------------------------------------------------------
   WLANSSC_BusAccessType

   This lists the access types for the bus.
---------------------------------------------------------------------------*/
typedef enum
{
  WLANSSC_SYNC_BUSACCESS        = 0,
  WLANSSC_ASYNC_BUSACCESS       = 1,

  WLANSSC_MAX_BUSACCESS

} WLANSSC_BusAccessType;

/*---------------------------------------------------------------------------
   WLANSSC_ClientCallBacksType

   This is the set of callbacks that SSC requires to interact with its
   client.
   UserData is the UserData that is associated with the callbacks and is
   required to be passed back to the client.
---------------------------------------------------------------------------*/
typedef struct
{
  v_PVOID_t                                 UserData;
  WLANSSC_GetMultipleTxPacketCbackType      pfnGetMultipleTxPacketCback;
  WLANSSC_TxCompleteCbackType               pfnTxCompleteCback;
  WLANSSC_RxPacketHandlerCbackType          pfnRxPacketHandlerCback;
  WLANSSC_ASICInterruptIndicationCbackType  pfnASICInterruptIndicationCback;
  WLANSSC_FatalErrorIndicationCbackType     pfnFatalErrorIndicationCback;
} WLANSSC_ClientCallBacksType;


/*---------------------------------------------------------------------------
   WLANSSC_ControlBlockType

   This defines the type of the SSC control block.

   Normally the memory for this should be allocated by whoever is using the
   SSC.
---------------------------------------------------------------------------*/
typedef struct
{
  v_BOOL_t               bInitialized;  /* Is control block initialized?   */
  WLANSSC_StateType      eState;        /* Current SSC state               */

  v_U32_t                uSuspendedFlowMask;    /* Set of flows suspended  */
  v_U32_t                uInterruptEnableMask;  /* Permanently enabled ints  */
  v_U32_t                uASICInterruptMask;    /* ASIC ints enabled       */

  v_PVOID_t              hSALHandle;            /* SAL handle              */

  WLANSSC_ClientCallBacksType  stClientCbacks; /* Set of callbacks for clnt*/

  vos_lock_t                   stSSCTxLock; /* Lock to access SSC tx path  */
  vos_lock_t                   stSSCRxLock; /* Lock to access SSC rx path  */

  /* Used to store the pending tx data chain ptr                           */
  vos_pkt_t                   *pTxChain;
  /* UserData to be passed back along with pkt chain during tx complete    */
  v_PVOID_t                    TxCompleteUserData;

  /* Most recent snapshot of interrupts on the target                      */
  v_U32_t                      uInterruptSnapshot;

  /* Used to extract client data in the tx path                            */
  v_U8_t                      *pTxBuffer;

  /* Used to read in rx data from SAL                                      */
  v_U8_t                      *pRxBuffer;

  /* Used to read/write registers in tx/int thread                         */
  v_U32_t                     *pRegBuffer[WLANSSC_MAX_REGBUFFER];

  /* Used as scratch buffer for vos_push_tail() workaround                 */
  v_U8_t                       aRxScratchBuffer[WLANSSC_MAXPKTSIZE];

  /* Data used to determine scheduling between tx and rx                   */
  struct
  {
    /* Is a tx transaction pending?                                        */
    v_BOOL_t                      bTxPending;

    /* Number of Rx processed without yielding to tx                       */
    v_U32_t                      uRxProcessedCnt;    

  } sSchedulerInfo;

  struct
  {
    /* Position at which rx data processing last stopped                   */
    v_U32_t                      uCurrentRxDataPosition;

    /* Length of data in rx buffer: 0 indicates no data                    */
    v_U32_t                      uCurrentRxDataSize;

    /* Data size pending on the target (as retrieved from ED)              */
    v_U32_t                      uPendingTargetData;    

  } sRxBufferInfo;

  /* Used to hold an incomplete frame received in rx data from SAL         */
  vos_pkt_t                     *pTempRxFrame;

  /* Used to store an available new packet from VOS (from the callback)    */
  vos_pkt_t                     *pMemAvailFrame;

  /* The following struct maintains SSC statistics                         */
  struct
  {
    /* Counter for frames that got pulled from upper layers                */
    v_U32_t                      uNumTxFrames;

    /* Counter for data transmissions over bus                             */
    v_U32_t                      uNumTxCmd53;

    /* Counter for data frames that were pushed to upper layers            */
    v_U32_t                      uNumRxFrames;

    /* Counter for data receptions over bus                                */
    v_U32_t                      uNumRxCmd53;

    /* Counter for padding frames received                                 */
    v_U32_t                      uNumRxPadding;

    /* Counter for incomplete frames received                              */
    v_U32_t                      uNumRxPartial;

    /* Counter for remainder frames received                               */
    v_U32_t                      uNumRxRemainder;

    /* Counter for interrupts received                                     */
    v_U32_t                      uNumInterrupts;

  } sStatsInfo;

  v_BOOL_t               bChipSuspended; /* see if chip is suspended       */

  v_U32_t                nCookie;       /* Sanity check                    */

} WLANSSC_ControlBlockType;


/*---------------------------------------------------------------------------
   WLANSSC_EventHandlerType

   This is the prototype for the state machine event handlers. Each of these
   functions *must* be reentrant!
   Also the event handlers offer no protection to caller: this is to avoid
   making the state machine aware of whether it is operating in a single
   thread, or using locks.
   Each event handler assumes that caller provides sufficient `locks' for a
   given SSC instance.

   Returns the status of the request
---------------------------------------------------------------------------*/
typedef VOS_STATUS (* WLANSSC_EventHandlerType)(WLANSSC_ControlBlockType *pControlBlock);


/*---------------------------------------------------------------------------
   WLANSSC_StateEntryType

   This is the set of event handlers for each state in the SSC state machine
---------------------------------------------------------------------------*/
typedef WLANSSC_EventHandlerType WLANSSC_StateEntryType[WLANSSC_MAX_EVENT];


/*---------------------------------------------------------------------------
   WLANSSC_StateTableType

   This is the SSC state machine structure - this is organized as a function
   table with event handlers for each state.
---------------------------------------------------------------------------*/
typedef WLANSSC_StateEntryType WLANSSC_StateTableType[WLANSSC_MAX_STATE];


/*---------------------------------------------------------------------------
   WLANSSC_InterruptType

   This lists the interrupts that the SSC is interested in.

   Should these be listed in priority order?
---------------------------------------------------------------------------*/
typedef enum
{
  WLANSSC_MIN_INTERRUPT                              =  0,

  WLANSSC_TX_FRM_DXE_ERR_INTERRUPT                   =  WLANSSC_MIN_INTERRUPT,
  WLANSSC_RX_FRM_DXE_ERR_INTERRUPT                   =  1,
  WLANSSC_TX_PKT_OUT_OF_SYNC_INTERRUPT               =  2,
  WLANSSC_CSR_RD_ACC_ERR_INTERRUPT                   =  3,
  WLANSSC_CSR_WR_ACC_ERR_INTERRUPT                   =  4,
  WLANSSC_TX_FRM_CRC_ERR_INTERRUPT                   =  5,
  WLANSSC_TX_FIFO_FULL_TIMEOUT_INTERRUPT             =  6,
  WLANSSC_HOST_ABORTED_TX_FRAME_INTERRUPT            =  7,
  WLANSSC_TX_CMD53_XACTION_LT_PKT_LEN_ERR_INTERRUPT  =  8,
  WLANSSC_RX_UNDERFLOW_FRM_ERR_INTERRUPT             =  9,
  WLANSSC_PMU_WAKEUP_INTERRUPT                       = 10,
  WLANSSC_RX_FIFO_FULL_INTERRUPT                     = 11,
  WLANSSC_RX_FIFO_DATA_AVAIL_INTERRUPT               = 12,
  WLANSSC_RX_FIFO_EMPTY_INTERRUPT                    = 13,
  WLANSSC_RX_PKT_XFER_UNDERFLOW_INTERRUPT            = 14,

  WLANSSC_MAX_INTERRUPT

} WLANSSC_InterruptType;


/*---------------------------------------------------------------------------
   WLANSSC_InterruptHandlerCbackType

   This is the prototype for interrupt handlers that SSC uses

   Return value: Status of the function call
---------------------------------------------------------------------------*/
typedef VOS_STATUS (* WLANSSC_InterruptHandlerCbackType)
(
  WLANSSC_ControlBlockType *pControlBlock
);


/*---------------------------------------------------------------------------
   WLANSSC_InterruptHandlerEntryType

   This is the type of entry for the interrupt handler table
---------------------------------------------------------------------------*/
typedef struct
{
  v_U32_t                                 uInterruptMask;
  WLANSSC_InterruptHandlerCbackType       pfnInterruptHandler;
} WLANSSC_InterruptHandlerEntryType;


/*---------------------------------------------------------------------------
   WLANSSC_InterruptHandlerTableType

   This is the type of table for interrupt handlers
---------------------------------------------------------------------------*/
typedef WLANSSC_InterruptHandlerEntryType WLANSSC_InterruptHandlerTableType[WLANSSC_MAX_INTERRUPT];


/*---------------------------------------------------------------------------
 * Forward declarations
 * ------------------------------------------------------------------------*/
static VOS_STATUS WLANSSC_ExecuteEvent
(
  WLANSSC_ControlBlockType *pControlBlock,
  WLANSSC_EventType         eEvent
);

static v_VOID_t WLANSSC_TransitionState
(
  WLANSSC_ControlBlockType *pControlBlock,
  WLANSSC_StateType         eState
);

static v_VOID_t WLANSSC_ResetStats
(
  WLANSSC_ControlBlockType *pControlBlock
);

static VOS_STATUS WLANSSC_InitContext
(
  WLANSSC_ControlBlockType *pControlBlock
);

static VOS_STATUS WLANSSC_InitControlBlock
(
  WLANSSC_ControlBlockType *pControlBlock
);

static VOS_STATUS WLANSSC_DestroyContext
(
  WLANSSC_ControlBlockType *pControlBlock
);

static VOS_STATUS WLANSSC_DestroyControlBlock
(
  WLANSSC_ControlBlockType *pControlBlock
);

static VOS_STATUS WLANSSC_EnableInterrupt
(
  WLANSSC_ControlBlockType *pControlBlock,
  v_U32_t                   uInterruptMask
);

static VOS_STATUS WLANSSC_DisableInterrupt
(
  WLANSSC_ControlBlockType *pControlBlock,
  v_U32_t                   uInterruptMask
);

static VOS_STATUS WLANSSC_ClearAllInterrupts
(
  WLANSSC_ControlBlockType *pControlBlock
);

static VOS_STATUS WLANSSC_ReadRegister
(
  WLANSSC_ControlBlockType *pControlBlock,
  v_U32_t                   uRegister,
  v_U32_t                  *pValue,
  WLANSSC_RegBufferType     eRegType
);

static VOS_STATUS WLANSSC_WriteRegister
(
  WLANSSC_ControlBlockType *pControlBlock,
  v_U32_t                   uRegister,
  v_U32_t                  *pValue,
  WLANSSC_RegBufferType     eRegType
);

static VOS_STATUS WLANSSC_ReadRegisterFuncZero
(
  WLANSSC_ControlBlockType *pControlBlock,
  v_U32_t                   uRegister,
  v_U8_t                   *pValue,
  WLANSSC_RegBufferType     eRegType
);

static VOS_STATUS WLANSSC_WriteRegisterFuncZero
(
  WLANSSC_ControlBlockType *pControlBlock,
  v_U32_t                   uRegister,
  v_U8_t                   *pValue,
  WLANSSC_RegBufferType     eRegType
);

static VOS_STATUS WLANSSC_ResetChip
(
  WLANSSC_ControlBlockType *pControlBlock
);

static VOS_STATUS WLANSSC_EnableTx
(
  WLANSSC_ControlBlockType *pControlBlock
);

static VOS_STATUS WLANSSC_DisableTx
(
  WLANSSC_ControlBlockType *pControlBlock
);

static VOS_STATUS WLANSSC_EnableRx
(
  WLANSSC_ControlBlockType *pControlBlock
);

static VOS_STATUS WLANSSC_DisableRx
(
  WLANSSC_ControlBlockType *pControlBlock
);

static VOS_STATUS WLANSSC_SendData
(
  WLANSSC_ControlBlockType *pControlBlock,
  v_U8_t                   *pBuffer,
  v_U32_t                   uDataSize
);

static VOS_STATUS WLANSSC_Transmit
(
  WLANSSC_ControlBlockType *pControlBlock,
  v_U32_t                   uMaxAllowedPktSize
);

static VOS_STATUS WLANSSC_Receive
(
  WLANSSC_ControlBlockType *pControlBlock,
  v_U32_t                   uMaxAllowedPktSize
);

static VOS_STATUS WLANSSC_ProcessRxData
(
  WLANSSC_ControlBlockType *pControlBlock
);

static VOS_STATUS WLANSSC_ReceiveData
(
  WLANSSC_ControlBlockType *pControlBlock,
  v_U8_t                   *pBuffer,
  v_U32_t                   uDataSize,
  WLANSSC_BusAccessType     eBusAccess
);

static VOS_STATUS WLANSSC_NotifyRxPkt
(
  WLANSSC_ControlBlockType *pControlBlock
);

static VOS_STATUS WLANSSC_PrepareRxPkt
(
  WLANSSC_ControlBlockType *pControlBlock,
  v_U8_t                   *pBuffer,
  v_U32_t                   uCurrentDataSize,
  v_U32_t                   uCompletePacketSize
);

static VOS_STATUS WLANSSC_HandleInterrupt
(
  WLANSSC_ControlBlockType *pControlBlock
);


#ifdef FEATURE_WLAN_UNSUPPORTED
static v_U32_t WLANSSC_GetRxPktInterruptStatus
(
  WLANSSC_ControlBlockType *pControlBlock
);
#endif /* FEATURE_WLAN_UNSUPPORTED */


static v_U32_t WLANSSC_GetNextRxEDFromBuffer
(
  v_U8_t   *pu8RxBuffer
);


#ifdef FEATURE_WLAN_UNSUPPORTED
static v_U32_t WLANSSC_GetFrameLengthFromSD
(
  WLANSSC_RxStartDescriptorType   *pRxSD
);
#endif /* FEATURE_WLAN_UNSUPPORTED */


static VOS_STATUS WLANSSC_ProcessInterrupt
(
  WLANSSC_ControlBlockType *pControlBlock
);

static VOS_STATUS WLANSSC_MemoryAvailableCallback
(
  vos_pkt_t       *pPacket,
  v_PVOID_t        UserData
);

static VOS_STATUS WLANSSC_InterruptHandlerCallback
(
  v_PVOID_t   pUnusedBySSC,
  v_PVOID_t   pSSCHandle
);

static VOS_STATUS WLANSSC_TxCompleteCallback
(
  v_PVOID_t   pUnusedBySSC,
  VOS_STATUS  RequestStatus,
  v_PVOID_t   pSSCHandle
);

static VOS_STATUS WLANSSC_RxCompleteCallback
(
  v_PVOID_t   pUnusedBySSC,
  VOS_STATUS  RequestStatus,
  v_PVOID_t   pSSCHandle
);

static VOS_STATUS WLANSSC_OpenEventHandler
(
  WLANSSC_ControlBlockType *pControlBlock
);

static VOS_STATUS WLANSSC_CloseEventHandler
(
  WLANSSC_ControlBlockType *pControlBlock
);

static VOS_STATUS WLANSSC_FatalErrorEventHandler
(
  WLANSSC_ControlBlockType *pControlBlock
);

static VOS_STATUS WLANSSC_StartEventHandler
(
  WLANSSC_ControlBlockType *pControlBlock
);

static VOS_STATUS WLANSSC_StopEventHandler
(
  WLANSSC_ControlBlockType *pControlBlock
);

static VOS_STATUS WLANSSC_ResetEventHandler
(
  WLANSSC_ControlBlockType *pControlBlock
);

static VOS_STATUS WLANSSC_R_CloseEventHandler
(
  WLANSSC_ControlBlockType *pControlBlock
);

static VOS_STATUS WLANSSC_R_SuspendEventHandler
(
  WLANSSC_ControlBlockType *pControlBlock
);

static VOS_STATUS WLANSSC_R_ResumeEventHandler
(
  WLANSSC_ControlBlockType *pControlBlock
);

static VOS_STATUS WLANSSC_R_TransmitEventHandler
(
  WLANSSC_ControlBlockType *pControlBlock
);

static VOS_STATUS WLANSSC_R_InterruptEventHandler
(
  WLANSSC_ControlBlockType *pControlBlock
);

static VOS_STATUS WLANSSC_R_ReceiveEventHandler
(
  WLANSSC_ControlBlockType *pControlBlock
);

static VOS_STATUS WLANSSC_SD_CloseEventHandler
(
  WLANSSC_ControlBlockType *pControlBlock
);

static VOS_STATUS WLANSSC_SD_StopEventHandler
(
  WLANSSC_ControlBlockType *pControlBlock
);

static VOS_STATUS WLANSSC_SD_ResumeEventHandler
(
  WLANSSC_ControlBlockType *pControlBlock
);

static VOS_STATUS WLANSSC_FatalInterruptHandler
(
  WLANSSC_ControlBlockType *pControlBlock
);

static VOS_STATUS WLANSSC_UnexpectedInterruptHandler
(
  WLANSSC_ControlBlockType *pControlBlock
);

static VOS_STATUS WLANSSC_TxFrmCRCErrInterruptHandler
(
  WLANSSC_ControlBlockType *pControlBlock
);

static VOS_STATUS WLANSSC_PMUWakeupInterruptHandler
(
  WLANSSC_ControlBlockType *pControlBlock
);

static VOS_STATUS WLANSSC_RxFIFOFullHandler
(
  WLANSSC_ControlBlockType *pControlBlock
);

static VOS_STATUS WLANSSC_RxFIFODataAvailInterruptHandler
(
  WLANSSC_ControlBlockType *pControlBlock
);

static VOS_STATUS WLANSSC_RxFIFOEmptyInterruptHandler
(
  WLANSSC_ControlBlockType *pControlBlock
);


#ifdef FEATURE_WLAN_UNSUPPORTED
static VOS_STATUS WLANSSC_ASICInterruptHandler
(
  WLANSSC_ControlBlockType *pControlBlock
);
#endif  /* FEATURE_WLAN_UNSUPPORTED */


static VOS_STATUS WLANSSC_RxPktXferUnderflowInterruptHandler
(
  WLANSSC_ControlBlockType *pControlBlock
);


/*---------------------------------------------------------------------------
 * Data definitions
 * ------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
   WLANSSC_StateTable

   This is the SSC state table.

   Currently we do not have a separate state table for each SSC context since
   it is expected that actions for a given state will be the same for all.
   So adding more memory requirements for the SSC context is avoided by
   using a static table for now.

   For an eventname, an EventHandler which is common to multiple states is
   named:
   WLANSSC_eventnameEventHandler
   An EventHandler which is specific to state Xyz is named:
   WLANSSC_X_eventnameEventHandler
   If there are multiple states such as Wxy and Wxz, eventhandlers for these
   states will be named:
   WLANSSC_WY_eventnameEventHandler and WLANSSC_WZ_eventnameEventHandler
   respectively
---------------------------------------------------------------------------*/
static WLANSSC_StateTableType WLANSSC_StateTable =
{
  /* CLOSED_STATE                                                          */
  {
    WLANSSC_OpenEventHandler,
    NULL, /* CloseEvent                                                    */
    NULL, /* StartEvent                                                    */
    NULL, /* StopEvent                                                     */
    NULL, /* SuspendEvent                                                  */
    NULL, /* ResumeEvent                                                   */
    NULL, /* ResetEvent                                                    */
    NULL, /* TransmitEvent                                                 */
    NULL, /* ReceiveEvent                                                  */
    NULL, /* SDIOInterruptEvent                                            */
    NULL, /* FatalErrorEvent                                               */
  },

  /* OPEN_STATE                                                            */
  {
    NULL, /* OpenEvent                                                     */
    WLANSSC_CloseEventHandler,
    WLANSSC_StartEventHandler,
    NULL, /* StopEvent                                                     */
    NULL, /* SuspendEvent                                                  */
    NULL, /* ResumeEvent                                                   */
    WLANSSC_ResetEventHandler,
    NULL, /* TransmitEvent                                                 */
    NULL, /* ReceiveEvent                                                  */
    NULL, /* SDIOInterruptEvent                                            */
    NULL, /* FatalErrorEvent                                               */
  },

  /* READY_STATE                                                           */
  {
    NULL, /* OpenEvent                                                     */
    WLANSSC_R_CloseEventHandler,
    NULL, /* StartEvent                                                    */
    WLANSSC_StopEventHandler,
    WLANSSC_R_SuspendEventHandler,
    WLANSSC_R_ResumeEventHandler,
    WLANSSC_ResetEventHandler,
    WLANSSC_R_TransmitEventHandler,
    WLANSSC_R_ReceiveEventHandler,
    WLANSSC_R_InterruptEventHandler,
    WLANSSC_FatalErrorEventHandler,
  },

  /* SUSPENDED_STATE                                                       */
  {
    NULL, /* OpenEvent                                                     */
    WLANSSC_SD_CloseEventHandler,
    NULL, /* StartEvent                                                    */
    WLANSSC_SD_StopEventHandler,
    NULL, /* SuspendEvent                                                  */
    WLANSSC_SD_ResumeEventHandler,
    WLANSSC_ResetEventHandler,
    NULL, /* TransmitEvent                                                 */
    NULL, /* ReceiveEvent                                                  */
    NULL, /* InterruptEvent                                                */
    WLANSSC_FatalErrorEventHandler,
  }
};


/*---------------------------------------------------------------------------
   WLANSSC_TxHeader

   This is the static part of the Tx header to avoid generating it for every
   packet.
   Ths structure comes from the SSC defs - based on the SIF format.
---------------------------------------------------------------------------*/
static const WLANSSC_TxStartDescriptorType gWLANSSC_TxStartDescriptor =
{
  WLANSSC_SYNC_SEQ_DWORD_0,
  WLANSSC_SYNC_SEQ_DWORD_1,
  {0}   /* Length is initialized to 0                                        */
};


/*---------------------------------------------------------------------------
   WLANSSC_InterruptHandlerTable

   This is the static interrupt handler table
---------------------------------------------------------------------------*/
static const WLANSSC_InterruptHandlerTableType gWLANSSC_InterruptHandlerTable =
{
  /* WLANSSC_TX_FRM_DXE_ERR_INTERRUPT                                      */
  {
    QWLAN_SIF_SIF_INT_STATUS_TX_FRM_DXE_ERR_INTR_MASK,
    WLANSSC_FatalInterruptHandler,
  },
  /* WLANSSC_RX_FRM_DXE_ERR_INTERRUPT                                      */
  {
    QWLAN_SIF_SIF_INT_STATUS_RX_FRM_DXE_ERR_INTR_MASK,
    WLANSSC_FatalInterruptHandler,
  },
  /* WLANSSC_TX_PKT_OUT_OF_SYNC_INTERRUPT                                  */
  {
    QWLAN_SIF_SIF_INT_STATUS_TX_PKT_OUT_OF_SYNC_INTR_MASK,
    WLANSSC_UnexpectedInterruptHandler,
  },
  /* WLANSSC_CSR_RD_ACC_ERR_INTERRUPT                                      */
  {
    QWLAN_SIF_SIF_INT_STATUS_CSR_RD_ACC_ERR_INTR_MASK,
    WLANSSC_FatalInterruptHandler,
  },
  /* WLANSSC_CSR_WR_ACC_ERR_INTERRUPT                                      */
  {
    QWLAN_SIF_SIF_INT_STATUS_CSR_WR_ACC_ERR_INTR_MASK,
    WLANSSC_FatalInterruptHandler,
  },
  /* WLANSSC_TX_FRM_CRC_ERR_INTERRUPT                                      */
  {
    QWLAN_SIF_SIF_INT_STATUS_TX_FRM_CRC_ERR_INTR_MASK,
    WLANSSC_TxFrmCRCErrInterruptHandler,
  },
  /* WLANSSC_TX_FIFO_FULL_TIMEOUT_INTERRUPT                                */
  {
    QWLAN_SIF_SIF_INT_STATUS_TX_FIFO_FULL_TIMEOUT_INTR_MASK,
    WLANSSC_FatalInterruptHandler,
  },
  /* WLANSSC_HOST_ABORTED_TX_FRAME_INTERRUPT                               */
  {
    QWLAN_SIF_SIF_INT_STATUS_HOST_ABORTED_TX_FRAME_INTR_MASK,
    WLANSSC_UnexpectedInterruptHandler,
  },
  /* WLANSSC_TX_CMD53_XACTION_LT_PKT_LEN_ERR_INTERRUPT                     */
  {
    QWLAN_SIF_SIF_INT_STATUS_TX_CMD53_XACTION_LT_PKT_LEN_ERR_INTR_MASK,
    WLANSSC_UnexpectedInterruptHandler,
  },
  /* WLANSSC_RX_UNDERFLOW_FRM_ERR_INTERRUPT                                */
  {
    QWLAN_SIF_SIF_INT_STATUS_RX_UNDERFLOW_FRM_ERR_INT_MASK,
    WLANSSC_UnexpectedInterruptHandler,
  },
  /* WLANSSC_PMU_WAKEUP_INTERRUPT                                          */
  {
    QWLAN_SIF_SIF_INT_STATUS_PMU_WAKEUP_INTR_MASK,
    WLANSSC_PMUWakeupInterruptHandler,
  },
  /* WLANSSC_RX_FIFO_FULL_INTERRUPT                                        */
  {
    QWLAN_SIF_SIF_INT_STATUS_RX_FIFO_FULL_INTR_MASK,
    WLANSSC_RxFIFOFullHandler,
  },
  /* WLANSSC_RX_FIFO_DATA_AVAIL_INTERRUPT                                  */
  {
    QWLAN_SIF_SIF_INT_STATUS_RX_FIFO_DATA_AVAIL_INTR_MASK,
    WLANSSC_RxFIFODataAvailInterruptHandler,
  },
  /* WLANSSC_RX_FIFO_EMPTY_INTERRUPT                                       */
  {
    QWLAN_SIF_SIF_INT_STATUS_RX_FIFO_EMPTY_INTR_MASK,
    WLANSSC_RxFIFOEmptyInterruptHandler,
  },
  /* WLANSSC_RX_PKT_XFER_UNDERFLOW_INTERRUPT                               */
  {
    QWLAN_SIF_SIF_INT_STATUS_RX_PKT_XFER_UNDERFLOW_INTR_MASK,
    WLANSSC_RxPktXferUnderflowInterruptHandler,    
  }
};


/*---------------------------------------------------------------------------
   gWLANSSC_TxMsgCnt

   This is a count to force the tx msg count not to ever exceed an arbitrary
   value - this is a temporary workaround for TL posting multiple requests to
   SSC for a potentially single fetch operation.
---------------------------------------------------------------------------*/
static v_U32_t gWLANSSC_TxMsgCnt = 0;


/*---------------------------------------------------------------------------
 * External Function implementation
 * ------------------------------------------------------------------------*/

/**
 @brief WLANSSC_Open is used to perform power-up initialization tasks 
 including initialization of SSC internal data structures.

 Must be called only once before WLANSSC_Close

 @param Handle: OUT parameter that provides the caller a handle created
                This handle must then be passed on every subsequent call to 
                SSC

 @param SalHandle: Handle to the SAL instance that SSC should use for its
                   operations over the bus. This cannot be NULL.

 @param GetSSCContextCback: Callback to be used by SSC to retrieve the SSC
                            context in which it should operate: caller is
                            expected to provide the memory for this context.

 @param UserData: Any handle that client expects to be passed back with the
                  GetSSCContextCback

 @see WLANSSC_Close

 @return Result of the function call
*/
VOS_STATUS WLANSSC_Open
(
  v_PVOID_t                          SalHandle,
  WLANSSC_HandleType                *pHandle,
  WLANSSC_GetSSCContextCbackType     pfnGetSSCContextCback,
  v_PVOID_t                          UserData
)
{
  WLANSSC_ControlBlockType    *pControlBlock;
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /* Sanity check                                                          */
  WLANSSC_ASSERT( NULL != SalHandle );
  WLANSSC_ASSERT( NULL != pHandle );
  WLANSSC_ASSERT( NULL != pfnGetSSCContextCback );

  /* Validate SSC Context (make sure it is invalid)                        */
  if( NULL != *pHandle )
  {
    /* Just make sure this previous instance was properly freed            */
    WLANSSC_ASSERT( WLANSSC_COOKIE !=
                    ((WLANSSC_ControlBlockType *)pHandle)->nCookie );
    *pHandle = NULL;
  }
  else
  {
    SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO, "SSC Handle is NULL as expected"));
  }

  /* Create SSC Context                                                    */
  if( VOS_STATUS_SUCCESS != pfnGetSSCContextCback( sizeof(WLANSSC_ControlBlockType),
                                                   (v_PVOID_t)&pControlBlock,
                                                   UserData ) )
  {
    SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "Error creating SSC context"));
    return VOS_STATUS_E_RESOURCES;
  }

  /* Initialize SSC Context                                                */
  if( VOS_STATUS_SUCCESS != WLANSSC_InitContext(pControlBlock) )
  {
    /* Should never happen!                                                */
    WLANSSC_ASSERT( 0 );

    SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "Error initializing SSC context"));
    return VOS_STATUS_E_FAILURE;
  }

  /* Initialize control block                                              */
  if( VOS_STATUS_SUCCESS != WLANSSC_InitControlBlock(pControlBlock) )
  {
    SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "Error initializing SSC control block"));
    return VOS_STATUS_E_RESOURCES;
  }

  /* Acquire Lock                                                          */
  WLANSSC_LOCKTXRX( pControlBlock );

  /* Store the SAL Handle for future use                                   */
  pControlBlock->hSALHandle = SalHandle;

  if( VOS_STATUS_SUCCESS != WLANSSC_ExecuteEvent( pControlBlock, 
                                                  WLANSSC_OPEN_EVENT ) )
  {
    /* Should never happen!                                                */
    WLANSSC_ASSERT( 0 );

    SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "Error executing Open event"));

    WLANSSC_UNLOCKTXRX( pControlBlock );
    (v_VOID_t) WLANSSC_DestroyControlBlock( pControlBlock );
    (v_VOID_t) WLANSSC_DestroyContext( pControlBlock );
    return VOS_STATUS_E_FAILURE;
  }

  /* Set Handle for return to caller                                       */
  *pHandle = pControlBlock;

  WLANSSC_UNLOCKTXRX( pControlBlock );

  return VOS_STATUS_SUCCESS;
  
} /* WLANSSC_Open() */


/**
 @brief WLANSSC_Close is used to shutdown SSC. Upon return from this call,
 all resources used by SSC will be freed.

 WLANSSC_Open must have been called.

 Also, any SSC context allocated during the SSC_Open() call may be freed only
 after this function returns.

 @param Handle: SSC handle to operate on (returned in WLANSSC_Open)

 @see WLANSSC_Open
 @return Result of the function call
*/
VOS_STATUS WLANSSC_Close
(
  WLANSSC_HandleType Handle
)
{
  WLANSSC_ControlBlockType    *pControlBlock;
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  WLANSSC_ASSERT( NULL != Handle );

  pControlBlock = (WLANSSC_ControlBlockType *) Handle;

  WLANSSC_ASSERT( VOS_TRUE == WLANSSC_ISCONTEXTVALID( pControlBlock ) );

  /* Acquire Lock                                                          */
  WLANSSC_LOCKTXRX( pControlBlock );

  if( VOS_STATUS_SUCCESS != WLANSSC_ExecuteEvent( pControlBlock, 
                                                  WLANSSC_CLOSE_EVENT ) )
  {
    /* Should never happen!                                                */
    WLANSSC_ASSERT( 0 );

    SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "Error executing Close event"));

    WLANSSC_UNLOCKTXRX( pControlBlock );
    return VOS_STATUS_E_FAILURE;
  }

  /* Release Lock                                                          */
  WLANSSC_UNLOCKTXRX( pControlBlock );

  /* Destroy all the resources                                             */
  (v_VOID_t) WLANSSC_DestroyControlBlock( pControlBlock );
  (v_VOID_t) WLANSSC_DestroyContext( pControlBlock );

  return VOS_STATUS_SUCCESS;

} /* WLANSSC_Close() */



/**
 @brief WLANSSC_Start is used to prepare the SSC for upcoming SDIO 
 transactions.
 At the end of this call, SSC is ready to accept any requests for data tx/rx
 as well as for handling interrupts from the chip.

 The target device SIF block is initialized (resetting the hardware, 
 configuring various thresholds and other values).
 At the end of this call, communication with the hardware over SDIO is 
 possible.
 At this point, the Libra interrupts are also enabled.

 This is the point at which the client is required to register all callbacks
 for notification.

 @param Handle: SSC handle to operate on (returned in WLANSSC_Open)

 @param StartParams: Set of parameters required for WLANSSC_Start call

 @param UserData   : Any handle that the client expects to be passed back
                     at the time of invoking callbacks registered in Start()
 
 @see WLANSSC_Stop

 @return Result of the function call
*/
VOS_STATUS WLANSSC_Start
(
  WLANSSC_HandleType           Handle,
  WLANSSC_StartParamsType     *pStartParams,
  v_PVOID_t                    UserData  
)
{
  WLANSSC_ControlBlockType    *pControlBlock;
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  WLANSSC_ASSERT( NULL != Handle );

  pControlBlock = (WLANSSC_ControlBlockType *) Handle;

  WLANSSC_ASSERT( VOS_TRUE == WLANSSC_ISCONTEXTVALID( pControlBlock ) );

  /* Acquire Lock                                                          */
  WLANSSC_LOCKTXRX( pControlBlock );

  /* Before executing the event, we need to store the values from caller   */
  pControlBlock->stClientCbacks.UserData = UserData;

  pControlBlock->stClientCbacks.pfnASICInterruptIndicationCback = 
    pStartParams->pfnASICInterruptIndicationCback;

  pControlBlock->stClientCbacks.pfnFatalErrorIndicationCback = 
    pStartParams->pfnFatalErrorIndicationCback;

  pControlBlock->stClientCbacks.pfnGetMultipleTxPacketCback = 
    pStartParams->pfnGetMultipleTxPacketCback;

  pControlBlock->stClientCbacks.pfnRxPacketHandlerCback = 
    pStartParams->pfnRxPacketHandlerCback;

  pControlBlock->stClientCbacks.pfnTxCompleteCback = 
    pStartParams->pfnTxCompleteCback;
  
  gWLANSSC_TxMsgCnt = 0;

  /* Execute the actual event which triggers the start procedure           */
  if( VOS_STATUS_SUCCESS != WLANSSC_ExecuteEvent( pControlBlock, 
                                                  WLANSSC_START_EVENT ) )
  {
    /* Should never happen!                                                */
    WLANSSC_ASSERT( 0 );

    SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "Error executing Start event"));

    WLANSSC_UNLOCKTXRX( pControlBlock );
    return VOS_STATUS_E_FAILURE;
  }

  /* Release Lock                                                          */
  WLANSSC_UNLOCKTXRX( pControlBlock );

  return VOS_STATUS_SUCCESS;

} /* WLANSSC_Start() */


/**
 @brief WLANSSC_Stop is used to return the SSC to a state wherein no more
 transactions are possible and interrupts are disabled.
 This is the state in which the SSC was prior to WLANSSC_Start being invoked:
 i.e. internal resources are available but the Libra chip has not been 
 enabled.

 @param Handle: SSC handle to operate on (returned in WLANSSC_Open)

 @see WLANSSC_Start
 @return Result of the function call
*/
VOS_STATUS WLANSSC_Stop
(
  WLANSSC_HandleType Handle
)
{
  WLANSSC_ControlBlockType    *pControlBlock;
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  WLANSSC_ASSERT( NULL != Handle );

  pControlBlock = (WLANSSC_ControlBlockType *) Handle;

  WLANSSC_ASSERT( VOS_TRUE == WLANSSC_ISCONTEXTVALID( pControlBlock ) );

  /* Acquire Lock                                                          */
  WLANSSC_LOCKTXRX( pControlBlock );

  if( VOS_STATUS_SUCCESS != WLANSSC_ExecuteEvent( pControlBlock, 
                                                  WLANSSC_STOP_EVENT ) )
  {
    /* Should never happen!                                                */
    WLANSSC_ASSERT( 0 );

    SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "Error executing Stop event"));

    WLANSSC_UNLOCKTXRX( pControlBlock );
    return VOS_STATUS_E_FAILURE;
  }

  /* Release Lock                                                          */
  WLANSSC_UNLOCKTXRX( pControlBlock );

  return VOS_STATUS_SUCCESS;

} /* WLANSSC_Stop() */


/**
 @brief WLANSSC_Reset is used to return the SSC to a state wherein no more
 transactions are possible and interrupts are disabled.
 This is the state in which the SSC was prior to WLANSSC_Start being invoked:

 It is the equivalent of WLANSSC_Close() followed by WLANSSC_Open()

 @param Handle: SSC handle to operate on (returned in WLANSSC_Open)

 @see WLANSSC_Stop

 @return Result of the function call
*/
VOS_STATUS WLANSSC_Reset
(
  WLANSSC_HandleType Handle
)
{
  WLANSSC_ControlBlockType    *pControlBlock;
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  WLANSSC_ASSERT( NULL != Handle );

  pControlBlock = (WLANSSC_ControlBlockType *) Handle;

  WLANSSC_ASSERT( VOS_TRUE == WLANSSC_ISCONTEXTVALID( pControlBlock ) );

  /* Acquire Lock                                                          */
  WLANSSC_LOCKTXRX( pControlBlock );

  if( VOS_STATUS_SUCCESS != WLANSSC_ExecuteEvent( pControlBlock, 
                                                  WLANSSC_RESET_EVENT ) )
  {
    /* Should never happen!                                                */
    WLANSSC_ASSERT( 0 );

    SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "Error executing Reset event"));

    WLANSSC_UNLOCKTXRX( pControlBlock );
    return VOS_STATUS_E_FAILURE;
  }

  /* Release Lock                                                          */
  WLANSSC_UNLOCKTXRX( pControlBlock );

  return VOS_STATUS_SUCCESS;

} /* WLANSSC_Reset() */



/**
 @brief WLANSSC_StartTransmit is used to notify SSC that there is data
 available for transmission.
 SSC will, when possible, initiate retrieval of data from client using
 the callback registered during WLANSSC_Start()

 @param Handle: SSC handle to operate on (returned in WLANSSC_Open)

 @see WLANSSC_Start

 @return Result of the function call
*/
VOS_STATUS WLANSSC_StartTransmit
(
  WLANSSC_HandleType Handle
)
{
  vos_msg_t                    sMessage;
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  if(gWLANSSC_TxMsgCnt)
  {
    SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO, "Avoiding serializing SSC Start Xmit event %x", 
                 Handle));
    return VOS_STATUS_SUCCESS;
  }

  gWLANSSC_TxMsgCnt++;

  /* Signal the OS to serialize our event                                  */
  SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO, "Serializing SSC Start Xmit event for control block %x", 
               Handle));

  vos_mem_zero( &sMessage, sizeof(vos_msg_t) );

  sMessage.bodyptr = (v_PVOID_t)Handle;
  sMessage.type = WLANSSC_TXPENDING_MESSAGE;

  return vos_tx_mq_serialize(VOS_MQ_ID_SSC, &sMessage);

} /* WLANSSC_StartTransmit() */


/**
 @brief WLANSSC_Suspend is used to temporarily disable operations in either
 tx, rx or both directions as specified in the call.

 However interrupts unrelated to tx and rx will be unaffected.

 e.g. if tx is disabled, all tx operations will be terminated and no more
 transmission will be serviced by SSC. However, SSC will continue to receive
 data over the bus and handle the other interrupts unrelated to tx.

 @param Handle: SSC handle to operate on (returned in WLANSSC_Open)

 @param SuspendedFlow: flow to suspend (ALL suspends both tx/rx); bits set
 here will be suspended. Bits not set will be ignored.
 
 @see WLANSSC_Resume

 @return Result of the function call
*/
VOS_STATUS WLANSSC_Suspend
(
  WLANSSC_HandleType       Handle,
  WLANSSC_FlowType         eSuspendedFlow
)
{
  WLANSSC_ControlBlockType    *pControlBlock;
  v_U32_t                      uNewSuspendedMask;
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  WLANSSC_ASSERT( NULL != Handle );

  pControlBlock = (WLANSSC_ControlBlockType *) Handle;

  WLANSSC_ASSERT( VOS_TRUE == WLANSSC_ISCONTEXTVALID( pControlBlock ) );

  /* Acquire Lock                                                          */
  WLANSSC_LOCKTXRX( pControlBlock );

  switch( eSuspendedFlow )
  {
    case WLANSSC_TX_FLOW:
      uNewSuspendedMask = WLANSSC_TXSUSPENDEDMASK;
      break;

    case WLANSSC_RX_FLOW:
      uNewSuspendedMask = WLANSSC_RXSUSPENDEDMASK;
      break;

    case WLANSSC_ALL_FLOW:
      uNewSuspendedMask = WLANSSC_ALLSUSPENDEDMASK;
      break;

    default:
      WLANSSC_ASSERT( 0 );
      /* Default to all                                                    */
      uNewSuspendedMask = WLANSSC_ALLSUSPENDEDMASK;
  } /* SuspendedFlow */

  /* Trigger an event only if there is a change in suspend mask            */
  if( !(pControlBlock->uSuspendedFlowMask == uNewSuspendedMask) )
  {
    pControlBlock->uSuspendedFlowMask |= uNewSuspendedMask;

    if( VOS_STATUS_SUCCESS != WLANSSC_ExecuteEvent( pControlBlock, 
                                                    WLANSSC_SUSPEND_EVENT ) )
    {
      /* If someone misuses SSC, be kind to them and don't assert!         */
      //WLANSSC_ASSERT( 0 );
      
      SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "Error executing Suspend event"));
      
      WLANSSC_UNLOCKTXRX( pControlBlock );
      return VOS_STATUS_E_FAILURE;
    }
  }

  /* Release Lock                                                          */
  WLANSSC_UNLOCKTXRX( pControlBlock );

  return VOS_STATUS_SUCCESS;

} /* WLANSSC_Suspend() */



/**
 @brief WLANSSC_Resume is used to resume certain flows in the SSC that have 
 been previously suspended.

 If an attempt is made to resume a flow which is already enabled, this will
 return SUCCESS.

 @param Handle: SSC handle to operate on (returned in WLANSSC_Open)

 @param ResumedFlow: flow to resume (ALL resumes both tx/rx); bits set here
 will be resumed. Bits not set here will be ignored.

 @see WLANSSC_Suspend

 @return Result of the function call
*/
VOS_STATUS WLANSSC_Resume
(
  WLANSSC_HandleType       Handle,
  WLANSSC_FlowType         eResumedFlow
)
{
  WLANSSC_ControlBlockType    *pControlBlock;
  VOS_STATUS                   eStatus = VOS_STATUS_SUCCESS;
  vos_msg_t                    sMessage;
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  WLANSSC_ASSERT( NULL != Handle );

  pControlBlock = (WLANSSC_ControlBlockType *) Handle;

  WLANSSC_ASSERT( VOS_TRUE == WLANSSC_ISCONTEXTVALID( pControlBlock ) );

  /* Acquire Lock                                                          */
  WLANSSC_LOCKTXRX( pControlBlock );

  switch( eResumedFlow )
  {
    case WLANSSC_TX_FLOW:

      pControlBlock->uSuspendedFlowMask &= ~WLANSSC_TXSUSPENDEDMASK;

      if( VOS_STATUS_SUCCESS != WLANSSC_EnableTx( pControlBlock ) )
      {
        /* Should never happen!                                            */
        WLANSSC_ASSERT( 0 );
        eStatus = VOS_STATUS_E_FAILURE;
      }
      break;

    case WLANSSC_RX_FLOW:

      pControlBlock->uSuspendedFlowMask &= ~WLANSSC_RXSUSPENDEDMASK;
      
      vos_mem_zero( &sMessage, sizeof(vos_msg_t) );

      sMessage.bodyptr = (v_PVOID_t)pControlBlock;
      sMessage.type = WLANSSC_RXPENDING_MESSAGE;

      if( VOS_STATUS_SUCCESS != vos_tx_mq_serialize(VOS_MQ_ID_SSC, &sMessage) )
      {
        /* Should never happen!                                            */
        WLANSSC_ASSERT( 0 );
        eStatus = VOS_STATUS_E_FAILURE;
      }
      break;

    case WLANSSC_ALL_FLOW:

      pControlBlock->uSuspendedFlowMask &= ~WLANSSC_ALLSUSPENDEDMASK;

      if( VOS_STATUS_SUCCESS != WLANSSC_EnableTx( pControlBlock ) )
      {
        /* Should never happen!                                            */
        WLANSSC_ASSERT( 0 );
        eStatus = VOS_STATUS_E_FAILURE;
      }

      vos_mem_zero( &sMessage, sizeof(vos_msg_t) );

      sMessage.bodyptr = (v_PVOID_t)pControlBlock;
      sMessage.type = WLANSSC_RXPENDING_MESSAGE;

      if( VOS_STATUS_SUCCESS != vos_tx_mq_serialize(VOS_MQ_ID_SSC, &sMessage) )
      {
        /* Should never happen!                                            */
        WLANSSC_ASSERT( 0 );
        eStatus = VOS_STATUS_E_FAILURE;
      }
      break;

    default:
      /* Default to no change                                              */
      SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR,"No flow specified to resume!"));
  } /* ResumedFlow */

  if( VOS_STATUS_SUCCESS != WLANSSC_ExecuteEvent( pControlBlock, 
                                                  WLANSSC_RESUME_EVENT ) )
  {     
    SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "Error executing Resume event"));
    
    eStatus = VOS_STATUS_E_FAILURE;
  }

  /* Release Lock                                                          */
  WLANSSC_UNLOCKTXRX( pControlBlock );

  return eStatus;

} /* WLANSSC_Resume() */



/**
 @brief WLANSSC_SuspendChip is used to put the Libra chip in standby mode

  After this API is invoked the chip cannot be accessed except after
  ResumeChip

 @param Handle: SSC handle to operate on (returned in WLANSSC_Open)
 
 @see WLANSSC_ResumeChip

 @return Result of the function call
*/
VOS_STATUS WLANSSC_SuspendChip
(
  WLANSSC_HandleType       Handle
)
{
  WLANSSC_ControlBlockType    *pControlBlock;
  v_U8_t                       uRegValue = 0;
  v_U32_t                      uFuncOneRegValue = 0;
  v_U32_t                      uResetCount = 0;
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  WLANSSC_ASSERT( NULL != Handle );

  pControlBlock = (WLANSSC_ControlBlockType *) Handle;

  WLANSSC_ASSERT( VOS_TRUE == WLANSSC_ISCONTEXTVALID( pControlBlock ) );

  /* Acquire Lock                                                          */
  WLANSSC_LOCKTXRX( pControlBlock );

  if (!vos_is_logp_in_progress(VOS_MODULE_ID_SSC, NULL)) {
  /* Sanity check to make sure flows have previously been suspended        */
  // WLANSSC_ASSERT( WLANSSC_SUSPENDED_STATE == pControlBlock->eState );

  /* Rx fifo reset to avoid leaving data on chip                           */
  if( VOS_STATUS_SUCCESS != WLANSSC_ReadRegister(pControlBlock,
                                                 QWLAN_SIF_SIF_TOP_CONTROL_REG,
                                                 &uFuncOneRegValue,
                                                 WLANSSC_TX_REGBUFFER) )
  {
    WLANSSC_UNLOCKTXRX( pControlBlock );
    return VOS_STATUS_E_FAILURE;
  }

  uFuncOneRegValue |= (QWLAN_SIF_SIF_TOP_CONTROL_RX_FIFO_MGNT_SOFT_RESET_MASK);

  if( VOS_STATUS_SUCCESS != WLANSSC_WriteRegister(pControlBlock,
                                                  QWLAN_SIF_SIF_TOP_CONTROL_REG,
                                                  &uFuncOneRegValue,
                                                  WLANSSC_TX_REGBUFFER) )
  {
    WLANSSC_UNLOCKTXRX( pControlBlock );
    return VOS_STATUS_E_FAILURE;
  }

  SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO, "Waiting for SIF rx engine reset"));

  while( WLANSSC_RESETLOOPCOUNT > uResetCount++ )
  {
    if( VOS_STATUS_SUCCESS != WLANSSC_ReadRegister(pControlBlock,
                                                   QWLAN_SIF_SIF_TOP_CONTROL_REG,
                                                   &uFuncOneRegValue,
                                                   WLANSSC_TX_REGBUFFER) )
    {
      WLANSSC_UNLOCKTXRX( pControlBlock );
      return VOS_STATUS_E_FAILURE;
    }

    if( !( (QWLAN_SIF_SIF_TOP_CONTROL_RX_FIFO_MGNT_SOFT_RESET_MASK) & uFuncOneRegValue) )
    {
      /* Reset complete                                                    */
      break;
    }
    else
    {
      /* Need to wait and keep polling                                     */
      vos_sleep(WLANSSC_RESETLOOPWAIT);
    }
  }

  SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO, "SIF rx fifo reset complete"));
  }

  /* Flush out SSC buffers and completely reset `host rx state'            */
  if( NULL != pControlBlock->pMemAvailFrame )
  {
    SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO, "Returning pending VOS packet during suspend chip"));
    (v_VOID_t) vos_pkt_return_packet( pControlBlock->pMemAvailFrame );
  }

  pControlBlock->sRxBufferInfo.uCurrentRxDataPosition = 0;
  pControlBlock->sRxBufferInfo.uCurrentRxDataSize = 0;
  pControlBlock->sRxBufferInfo.uPendingTargetData = 0;


  /* Write to all the relevant registers here to complete suspend operation*/

  /* Begin T/R change */
  /* TRSW_SUPPLY_CTRL_0 - Read the current value just in case                 */
  if( VOS_STATUS_SUCCESS != WLANSSC_ReadRegisterFuncZero(pControlBlock,
                                                         QWLAN_SIF_BAR4_WLAN_CONTROL_REG_REG,
                                                         &uRegValue,
                                                         WLANSSC_TX_REGBUFFER) )
  {
    WLANSSC_ASSERT( 0 );

    SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "Error suspending chip"));
    WLANSSC_UNLOCKTXRX( pControlBlock );
    return VOS_STATUS_E_FAILURE;
  }

  uRegValue |= QWLAN_SIF_BAR4_WLAN_CONTROL_REG_DEFAULT;
  uRegValue &= ~(QWLAN_SIF_BAR4_WLAN_CONTROL_REG_SDIOC_CMD_ACTIVE_CHECK_DISABLE_MASK);	

  /* TRSW Bit 1 should be 0 before entering standby OR deepsleep*/
  uRegValue &= ~(QWLAN_SIF_BAR4_WLAN_PWR_SAVE_CONTROL_REG_TRSW_SUPPLY_CTRL_1_MASK);

  if( VOS_STATUS_SUCCESS != WLANSSC_WriteRegisterFuncZero(pControlBlock,
                                                          QWLAN_SIF_BAR4_WLAN_CONTROL_REG_REG,
                                                          &uRegValue,
                                                          WLANSSC_TX_REGBUFFER) )
  {
    WLANSSC_ASSERT( 0 );

    SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "Error suspending chip"));
    WLANSSC_UNLOCKTXRX( pControlBlock );
    return VOS_STATUS_E_FAILURE;
  }

  /* TRSW_SUPPLY_CTRL_1 - Read the current value just in case                 */
  if( VOS_STATUS_SUCCESS != WLANSSC_ReadRegisterFuncZero(pControlBlock,
                                                         QWLAN_SIF_BAR4_WLAN_PWR_SAVE_CONTROL_REG_REG,
                                                         &uRegValue,
                                                         WLANSSC_TX_REGBUFFER) )
  {
    WLANSSC_ASSERT( 0 );

    SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "Error suspending chip"));
    WLANSSC_UNLOCKTXRX( pControlBlock );
    return VOS_STATUS_E_FAILURE;
  }

  /* Add 0xFF (per instructions) to the register status and write back        */
  uRegValue |= QWLAN_SIF_BAR4_WLAN_PWR_SAVE_CONTROL_REG_DEFAULT;

  if( VOS_STATUS_SUCCESS != WLANSSC_WriteRegisterFuncZero(pControlBlock,
                                                          QWLAN_SIF_BAR4_WLAN_PWR_SAVE_CONTROL_REG_REG,
                                                          &uRegValue,
                                                          WLANSSC_TX_REGBUFFER) )
  {
    WLANSSC_ASSERT( 0 );

    SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "Error suspending chip"));
    WLANSSC_UNLOCKTXRX( pControlBlock );
    return VOS_STATUS_E_FAILURE;
  }

  /* End T/R change */

  /* Read the current value just in case                                   */
  if( VOS_STATUS_SUCCESS != WLANSSC_ReadRegisterFuncZero(pControlBlock,
                                                         QWLAN_SIF_BAR4_WLAN_PWR_SAVE_CONTROL_REG_REG,
                                                         &uRegValue,
                                                         WLANSSC_TX_REGBUFFER) )
  {
    WLANSSC_ASSERT( 0 );

    SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "Error suspending chip"));
    WLANSSC_UNLOCKTXRX( pControlBlock );
    return VOS_STATUS_E_FAILURE;
  }

  /* Add the suspend bit to the register status and write back             */
  uRegValue |= QWLAN_SIF_BAR4_WLAN_PWR_SAVE_CONTROL_REG_SUSPEND_WLAN_MASK;

  if( VOS_STATUS_SUCCESS != WLANSSC_WriteRegisterFuncZero(pControlBlock,
                                                          QWLAN_SIF_BAR4_WLAN_PWR_SAVE_CONTROL_REG_REG,
                                                          &uRegValue,
                                                          WLANSSC_TX_REGBUFFER) )
  {
    WLANSSC_ASSERT( 0 );

    SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "Error suspending chip"));
    WLANSSC_UNLOCKTXRX( pControlBlock );
    return VOS_STATUS_E_FAILURE;
  }

  /* Need to wait per programmer's guide (minimum allowed by VOS is 1 ms)  */
  vos_sleep(WLANSSC_SUSPENDWLANWAIT);


  /* Read the current value just in case                                   */
  if( VOS_STATUS_SUCCESS != WLANSSC_ReadRegisterFuncZero(pControlBlock,
                                                         QWLAN_SIF_BAR4_WLAN_CONTROL_REG_REG,
                                                         &uRegValue,
                                                         WLANSSC_TX_REGBUFFER) )
  {
    WLANSSC_ASSERT( 0 );

    SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "Error suspending chip"));
    WLANSSC_UNLOCKTXRX( pControlBlock );
    return VOS_STATUS_E_FAILURE;
  }

  /* Turn off PMU GCU clk in the register status and write back            */
  uRegValue &= ~QWLAN_SIF_BAR4_WLAN_CONTROL_REG_PMU_GCU_CLK_ROSC_G_EN_MASK;

  if( VOS_STATUS_SUCCESS != WLANSSC_WriteRegisterFuncZero(pControlBlock,
                                                          QWLAN_SIF_BAR4_WLAN_CONTROL_REG_REG,
                                                          &uRegValue,
                                                          WLANSSC_TX_REGBUFFER) )
  {
    WLANSSC_ASSERT( 0 );

    SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "Error suspending chip"));
    WLANSSC_UNLOCKTXRX( pControlBlock );
    return VOS_STATUS_E_FAILURE;
  }

  /* Need to wait per programmer's guide (minimum allowed by VOS is 1 ms)  */
  vos_sleep(WLANSSC_SUSPENDWLANWAIT);

  /* Read the current value just in case                                   */
  if( VOS_STATUS_SUCCESS != WLANSSC_ReadRegisterFuncZero(pControlBlock,
                                                         QWLAN_SIF_BAR4_WLAN_CONTROL_REG_REG,
                                                         &uRegValue,
                                                         WLANSSC_TX_REGBUFFER) )
  {
    WLANSSC_ASSERT( 0 );

    SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "Error suspending chip"));
    WLANSSC_UNLOCKTXRX( pControlBlock );
    return VOS_STATUS_E_FAILURE;
  }

  /* Turn off PMU ROSC pwr enable in the register status and write back    */
  uRegValue &= ~QWLAN_SIF_BAR4_WLAN_CONTROL_REG_PMU_ROSC_PWR_EN_MASK;

  if( VOS_STATUS_SUCCESS != WLANSSC_WriteRegisterFuncZero(pControlBlock,
                                                          QWLAN_SIF_BAR4_WLAN_CONTROL_REG_REG,
                                                          &uRegValue,
                                                          WLANSSC_TX_REGBUFFER) )
  {
    WLANSSC_ASSERT( 0 );

    SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "Error suspending chip"));
    WLANSSC_UNLOCKTXRX( pControlBlock );
    return VOS_STATUS_E_FAILURE;
  }


  if( VOS_STATUS_SUCCESS != WLANSSC_ReadRegisterFuncZero(pControlBlock,
                                                         QWLAN_SIF_BAR4_WLAN_STATUS_REG_REG,
                                                         &uRegValue,
                                                         WLANSSC_TX_REGBUFFER) )
  {
    WLANSSC_ASSERT( 0 );

    SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "Error suspending chip"));
    WLANSSC_UNLOCKTXRX( pControlBlock );
    return VOS_STATUS_E_FAILURE;
  }

  pControlBlock->bChipSuspended = VOS_TRUE;
  SSCLOG2(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO_HIGH, "Suspend Chip status %x", uRegValue));

  /* Release Lock                                                          */
  WLANSSC_UNLOCKTXRX( pControlBlock );

  return VOS_STATUS_SUCCESS;

} /* WLANSSC_SuspendChip() */


/**
 @brief WLANSSC_ResumeChip is used to bring a previously suspended Libra
 chip out of the standby mode and ready for activity.

 @param Handle: SSC handle to operate on (returned in WLANSSC_Open)

 @see WLANSSC_SuspendChip

 @return Result of the function call
*/
VOS_STATUS WLANSSC_ResumeChip
(
  WLANSSC_HandleType       Handle
)
{
  WLANSSC_ControlBlockType    *pControlBlock;
  v_U8_t                       uRegValue = 0;
  v_U8_t                       uResumeLoopcount = 0;
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  WLANSSC_ASSERT( NULL != Handle );

  pControlBlock = (WLANSSC_ControlBlockType *) Handle;

  WLANSSC_ASSERT( VOS_TRUE == WLANSSC_ISCONTEXTVALID( pControlBlock ) );

  /* Acquire Lock                                                          */
  WLANSSC_LOCKTXRX( pControlBlock );

  /* Sanity check to make sure flows have previously been suspended        */
  // WLANSSC_ASSERT( WLANSSC_SUSPENDED_STATE == pControlBlock->eState );


  /* Write to all the relevant registers here to complete resume operation */

  /* Read the current value just in case                                   */
  if( VOS_STATUS_SUCCESS != WLANSSC_ReadRegisterFuncZero(pControlBlock,
                                                         QWLAN_SIF_BAR4_WLAN_CONTROL_REG_REG,
                                                         &uRegValue,
                                                         WLANSSC_TX_REGBUFFER) )
  {
    WLANSSC_ASSERT( 0 );

    SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "Error resuming chip"));
    WLANSSC_UNLOCKTXRX( pControlBlock );
    return VOS_STATUS_E_FAILURE;
  }

  /* Turn on PMU ROSC pwr enable in the register status and write back     */
  uRegValue |= QWLAN_SIF_BAR4_WLAN_CONTROL_REG_PMU_ROSC_PWR_EN_MASK;
  uRegValue &= ~(QWLAN_SIF_BAR4_WLAN_CONTROL_REG_SDIOC_CMD_ACTIVE_CHECK_DISABLE_MASK);	

  if( VOS_STATUS_SUCCESS != WLANSSC_WriteRegisterFuncZero(pControlBlock,
                                                          QWLAN_SIF_BAR4_WLAN_CONTROL_REG_REG,
                                                          &uRegValue,
                                                          WLANSSC_TX_REGBUFFER) )
  {
    WLANSSC_ASSERT( 0 );

    SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "Error resuming chip"));
    WLANSSC_UNLOCKTXRX( pControlBlock );
    return VOS_STATUS_E_FAILURE;
  }

  /* Need to wait per programmer's guide (minimum allowed by VOS is 1 ms)  */
  vos_sleep(WLANSSC_RESUMEWLANWAIT);

  /* Read the current value just in case                                   */
  if( VOS_STATUS_SUCCESS != WLANSSC_ReadRegisterFuncZero(pControlBlock,
                                                         QWLAN_SIF_BAR4_WLAN_CONTROL_REG_REG,
                                                         &uRegValue,
                                                         WLANSSC_TX_REGBUFFER) )
  {
    WLANSSC_ASSERT( 0 );

    SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "Error resuming chip"));
    WLANSSC_UNLOCKTXRX( pControlBlock );
    return VOS_STATUS_E_FAILURE;
  }

  /* Turn on PMU GCU clk in the register status and write back             */
  uRegValue |= QWLAN_SIF_BAR4_WLAN_CONTROL_REG_PMU_GCU_CLK_ROSC_G_EN_MASK;

  if( VOS_STATUS_SUCCESS != WLANSSC_WriteRegisterFuncZero(pControlBlock,
                                                          QWLAN_SIF_BAR4_WLAN_CONTROL_REG_REG,
                                                          &uRegValue,
                                                          WLANSSC_TX_REGBUFFER) )
  {
    WLANSSC_ASSERT( 0 );

    SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "Error resuming chip"));
    WLANSSC_UNLOCKTXRX( pControlBlock );
    return VOS_STATUS_E_FAILURE;
  }

  /* Need to wait per programmer's guide (minimum allowed by VOS is 1 ms)  */
  vos_sleep(WLANSSC_RESUMEWLANWAIT);

  /* Read the current value just in case                                   */
  if( VOS_STATUS_SUCCESS != WLANSSC_ReadRegisterFuncZero(pControlBlock,
                                                         QWLAN_SIF_BAR4_WLAN_PWR_SAVE_CONTROL_REG_REG,
                                                         &uRegValue,
                                                         WLANSSC_TX_REGBUFFER) )
  {
    WLANSSC_ASSERT( 0 );

    SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "Error resuming chip"));
    WLANSSC_UNLOCKTXRX( pControlBlock );
    return VOS_STATUS_E_FAILURE;
  }

  /* Add the suspend bit to the register status and write back             */
  uRegValue &= ~QWLAN_SIF_BAR4_WLAN_PWR_SAVE_CONTROL_REG_SUSPEND_WLAN_MASK;

  if( VOS_STATUS_SUCCESS != WLANSSC_WriteRegisterFuncZero(pControlBlock,
                                                          QWLAN_SIF_BAR4_WLAN_PWR_SAVE_CONTROL_REG_REG,
                                                          &uRegValue,
                                                          WLANSSC_TX_REGBUFFER) )
  {
    WLANSSC_ASSERT( 0 );

    SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "Error resuming chip"));
    WLANSSC_UNLOCKTXRX( pControlBlock );
    return VOS_STATUS_E_FAILURE;
  }

  do 
  {
     vos_sleep(WLANSSC_RESUMEWLANWAIT);

    if( VOS_STATUS_SUCCESS != WLANSSC_ReadRegisterFuncZero(pControlBlock,
                                                           QWLAN_SIF_BAR4_WLAN_STATUS_REG_REG,
                                                           &uRegValue,
                                                           WLANSSC_TX_REGBUFFER) )
    {
      WLANSSC_ASSERT( 0 );

      SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "Error resuming chip"));
      WLANSSC_UNLOCKTXRX( pControlBlock );
      return VOS_STATUS_E_FAILURE;
    }
  } while ( (uRegValue & QWLAN_SIF_BAR4_WLAN_STATUS_REG_PMU_BLOCKED_BIT_MASK) && 
            (WLANSSC_RESUMELOOPCOUNT > ++uResumeLoopcount) ); 

   SSCLOG2(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO_HIGH, "Resume chip Status %x", uRegValue));

  /* Begin T/R change */
  /* TRSW_SUPPLY_CTRL_0 - Read the current value just in case                 */
  if( VOS_STATUS_SUCCESS != WLANSSC_ReadRegisterFuncZero(pControlBlock,
                                                         QWLAN_SIF_BAR4_WLAN_CONTROL_REG_REG,
                                                         &uRegValue,
                                                         WLANSSC_TX_REGBUFFER) )
  {
    WLANSSC_ASSERT( 0 );

    SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "Error Resuming chip"));
    WLANSSC_UNLOCKTXRX( pControlBlock );
    return VOS_STATUS_E_FAILURE;
  }

  uRegValue &= ~(QWLAN_SIF_BAR4_WLAN_CONTROL_REG_TRSW_SUPPLY_CTRL_0_MASK);

  if( VOS_STATUS_SUCCESS != WLANSSC_WriteRegisterFuncZero(pControlBlock,
                                                          QWLAN_SIF_BAR4_WLAN_CONTROL_REG_REG,
                                                          &uRegValue,
                                                          WLANSSC_TX_REGBUFFER) )
  {
    WLANSSC_ASSERT( 0 );

    SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "Error Resuming chip"));
    WLANSSC_UNLOCKTXRX( pControlBlock );
    return VOS_STATUS_E_FAILURE;
  }

  /* TRSW_SUPPLY_CTRL_1 - Read the current value just in case                 */
  if( VOS_STATUS_SUCCESS != WLANSSC_ReadRegisterFuncZero(pControlBlock,
                                                         QWLAN_SIF_BAR4_WLAN_PWR_SAVE_CONTROL_REG_REG,
                                                         &uRegValue,
                                                         WLANSSC_TX_REGBUFFER) )
  {
    WLANSSC_ASSERT( 0 );

    SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "Error Resuming chip"));
    WLANSSC_UNLOCKTXRX( pControlBlock );
    return VOS_STATUS_E_FAILURE;
  }


  /* TRSW Bit 1 should be 1 before entering standby OR deepsleep*/
  uRegValue |= (QWLAN_SIF_BAR4_WLAN_PWR_SAVE_CONTROL_REG_TRSW_SUPPLY_CTRL_1_MASK);

  if( VOS_STATUS_SUCCESS != WLANSSC_WriteRegisterFuncZero(pControlBlock,
                                                          QWLAN_SIF_BAR4_WLAN_PWR_SAVE_CONTROL_REG_REG,
                                                          &uRegValue,
                                                          WLANSSC_TX_REGBUFFER) )
  {
    WLANSSC_ASSERT( 0 );

    SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "Error resuming chip"));
    WLANSSC_UNLOCKTXRX( pControlBlock );
    return VOS_STATUS_E_FAILURE;
  }

  /* End T/R change */

   pControlBlock->bChipSuspended = VOS_FALSE;

   /* Enable rx data interrupt if the data path is not currently suspended */
   if( WLANSSC_SUSPENDED_STATE != pControlBlock->eState )
   {
     v_U32_t  uInterruptsEnabled;

     /* The best way to see interrupts enabled is to check the chip           */
     if( VOS_STATUS_SUCCESS != WLANSSC_ReadRegister(pControlBlock,
                                                    QWLAN_SIF_SIF_INT_EN_REG,
                                                    &(uInterruptsEnabled),
                                                    WLANSSC_TX_REGBUFFER) )
     {
       WLANSSC_ASSERT(0);
       WLANSSC_UNLOCKTXRX( pControlBlock );
       return VOS_STATUS_E_FAILURE;
     }

     /* Update the interrupt enable mask - this might cause a register write  */
     if( VOS_STATUS_SUCCESS != WLANSSC_EnableInterrupt( pControlBlock,
                                                        (uInterruptsEnabled |
                                                         QWLAN_SIF_SIF_INT_EN_RX_FIFO_DATA_AVAIL_INTR_EN_MASK) ) )
     {
       SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "Error enabling interrupt %x", uInterruptsEnabled));
       WLANSSC_UNLOCKTXRX( pControlBlock );
       return VOS_STATUS_E_FAILURE;
     }
     
   }

  /* Release Lock                                                          */
   WLANSSC_UNLOCKTXRX( pControlBlock );

  return VOS_STATUS_SUCCESS;

} /* WLANSSC_ResumeChip() */


/**
 @brief WLANSSC_EnableASICInterrupt is used to enable the ASIC interrupt on
 the Libra device.

 Before invoking this function, SSC_Start() must already have been called.

 @param Handle: SSC handle to operate on (returned in WLANSSC_Open)

 @see WLANSSC_DisableASICInterrupt

 @return Result of the function call
*/
VOS_STATUS WLANSSC_EnableASICInterrupt
(
  WLANSSC_HandleType Handle
)
{
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "Deprecated API - Please use WLANSSC_EnableASICInterruptEx()"));

  /* For now, this will fallback to enabling all ASIC interrupts           */
  return WLANSSC_EnableASICInterruptEx( Handle, ~WLANSSC_NONASICINTERRUPTMASK);

} /* WLANSSC_EnableASICInterrupt() */


/**
 @brief WLANSSC_DisableASICInterrupt is used to disable the ASIC interrupt on
 the Libra device.

 Before invoking this function, SSC_Start() must already have been called.

 @param Handle: SSC handle to operate on (returned in WLANSSC_Open)

 @see WLANSSC_EnableASICInterrupt

 @return Result of the function call
*/
VOS_STATUS WLANSSC_DisableASICInterrupt
(
  WLANSSC_HandleType Handle
)
{
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "Deprecated API - Please use WLANSSC_DisableASICInterruptEx()"));

  /* For now, this will fallback to enabling all ASIC interrupts           */
  return WLANSSC_DisableASICInterruptEx( Handle, ~WLANSSC_NONASICINTERRUPTMASK);

} /* WLANSSC_DisableASICInterrupt() */


/**
 @brief WLANSSC_EnableASICInterruptEx is used to enable specified ASIC-related interrupts on 
 the Libra device.
 e.g. this may be used to enable both ASIC_INTR and MAC_HOST_INTR
 This will *not* automatically disable previously enabled ASIC interrupts; any disabling
 will only happen via an explicit call to WLANSSC_DisableASICInterruptEx

 The interrupt handler will be invoked only *once* per interrupt status read,
 regardless of how many client-specified interrupt bits are set (e.g whether only
 ASIC_INTR is set, MAC_INTR is set or both are set, the callback will be called
 only once)

 Client is expected to handle clearing the interrupt source directly; SSC
 *will not* clear the interrupts

 Before invoking this function, SSC_Start() must already have been called.

 @param Handle: SSC handle to operate on (returned in WLANSSC_Open)

 @param uInterruptMask: bitmask of interrupts to enable (MUST be per libra.h)

 @see WLANSSC_DisableASICInterruptEx

 @return Result of the function call
*/
VOS_STATUS WLANSSC_EnableASICInterruptEx
(
  WLANSSC_HandleType Handle,
  v_U32_t            uInterruptMask
)
{
  WLANSSC_ControlBlockType    *pControlBlock;
  v_U32_t                      uInterruptsEnabled = 0;
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  WLANSSC_ASSERT( NULL != Handle );

  pControlBlock = (WLANSSC_ControlBlockType *) Handle;

  WLANSSC_ASSERT( VOS_TRUE == WLANSSC_ISCONTEXTVALID( pControlBlock ) );

  /* Acquire Lock                                                          */
  WLANSSC_LOCKTXRX( pControlBlock );

  /* The best way to see interrupts enabled is to check the chip           */
  if( VOS_STATUS_SUCCESS != WLANSSC_ReadRegister(pControlBlock,
                                                 QWLAN_SIF_SIF_INT_EN_REG,
                                                 &(uInterruptsEnabled),
                                                 WLANSSC_TX_REGBUFFER) )
  {
    WLANSSC_UNLOCKTXRX( pControlBlock );
    return VOS_STATUS_E_FAILURE;
  }

  /* Store the interrupt mask within the SSC control block                 */
  pControlBlock->uASICInterruptMask = pControlBlock->uASICInterruptMask | 
    (~WLANSSC_NONASICINTERRUPTMASK & uInterruptMask);

  // FIXME: Hardcode RX_FIFO_DATA_AVAIL since HAL is overwriting this on chip!
  /* Update the interrupt enable mask - this might cause a register write  */
  if( VOS_STATUS_SUCCESS != WLANSSC_EnableInterrupt( pControlBlock,
                                                     pControlBlock->uASICInterruptMask | uInterruptsEnabled |
                                   QWLAN_SIF_SIF_INT_EN_RX_FIFO_DATA_AVAIL_INTR_EN_MASK ) )
  {
    SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "Error enabling ASIC interrupt %x", uInterruptMask));
    WLANSSC_UNLOCKTXRX( pControlBlock );
    return VOS_STATUS_E_FAILURE;
  }

  /* Release Lock                                                          */
  WLANSSC_UNLOCKTXRX( pControlBlock );

  return VOS_STATUS_SUCCESS;

} /* WLANSSC_EnableASICInterruptEx() */


/**
 @brief WLANSSC_DisableASICInterruptEx is used to disable specified ASIC-related interrupts on
 the Libra device.
 e.g. this may be used to disable both ASIC_INTR and MAC_HOST_INTR

 Only the specified interrupts will be disabled; others will remain unchanged

 Before invoking this function, SSC_Start() must already have been called.

 @param Handle: SSC handle to operate on (returned in WLANSSC_Open)

 @param uInterruptMask: bitmask of interrupts to disable (MUST be per libra.h)
                        if a bit is set (i.e. 1) in this field, it will be disabled
                        if a bit is not set (i.e. 0) in this field, it will be ignored

 @see WLANSSC_EnableASICInterruptEx

 @return Result of the function call
*/
VOS_STATUS WLANSSC_DisableASICInterruptEx
(
  WLANSSC_HandleType Handle,
  v_U32_t            uInterruptMask
)
{
  WLANSSC_ControlBlockType    *pControlBlock;
  v_U32_t                      uInterruptsEnabled = 0;
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  WLANSSC_ASSERT( NULL != Handle );

  pControlBlock = (WLANSSC_ControlBlockType *) Handle;

  WLANSSC_ASSERT( VOS_TRUE == WLANSSC_ISCONTEXTVALID( pControlBlock ) );

  /* Acquire Lock                                                          */
  WLANSSC_LOCKTXRX( pControlBlock );

  /* The best way to see interrupts enabled is to check the chip           */
  if( VOS_STATUS_SUCCESS != WLANSSC_ReadRegister(pControlBlock,
                                                 QWLAN_SIF_SIF_INT_EN_REG,
                                                 &(uInterruptsEnabled),
                                                 WLANSSC_TX_REGBUFFER) )
  {
    WLANSSC_UNLOCKTXRX( pControlBlock );
    return VOS_STATUS_E_FAILURE;
  }

  /* Update the interrupt mask within the SSC control block                */
  pControlBlock->uASICInterruptMask = pControlBlock->uASICInterruptMask & 
    ~(~WLANSSC_NONASICINTERRUPTMASK & uInterruptMask);

  /* Update the interrupt enable mask - this might cause a register write  */
  if( VOS_STATUS_SUCCESS != WLANSSC_DisableInterrupt( pControlBlock,
                                                      uInterruptMask ) )
  {
    SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "Error disabling ASIC interrupt %x", uInterruptMask));
    WLANSSC_UNLOCKTXRX( pControlBlock );
    return VOS_STATUS_E_FAILURE;
  }

  uInterruptsEnabled = uInterruptsEnabled & ~uInterruptMask;

  // TODO: optimize disable interrupt to leave existing rx data int on
  /* Reenable any interrupts that we disabled (limitation of SSC_DisableInt)  */
  /* The best way to see if rx data is enabled is to check the chip       */
  if( VOS_STATUS_SUCCESS != WLANSSC_WriteRegister(pControlBlock,
                                                  QWLAN_SIF_SIF_INT_EN_REG,
                                                  &uInterruptsEnabled,
                                                  WLANSSC_TX_REGBUFFER) )
  {
    WLANSSC_UNLOCKTXRX( pControlBlock );
    return VOS_STATUS_E_FAILURE;
  }

  /* Release Lock                                                          */
  WLANSSC_UNLOCKTXRX( pControlBlock );

  return VOS_STATUS_SUCCESS;

} /* WLANSSC_DisableASICInterruptEx() */


/**
 @brief WLANSSC_ProcessMsg is used to receive the SSC event once it
 has been serialized by VOS

 @param pMsg: pointer to the message from VOS

 @param UserData: any userdata that was enqueued during serialization

 @see

 @return Result of the function call
*/
VOS_STATUS WLANSSC_ProcessMsg
(
  v_PVOID_t  pContext,
  vos_msg_t *pMsg
)
{
  WLANSSC_ControlBlockType    *pControlBlock;
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  WLANSSC_ASSERT( NULL != pMsg );

  pControlBlock = (WLANSSC_ControlBlockType *) pMsg->bodyptr;

  WLANSSC_ASSERT( VOS_TRUE == WLANSSC_ISCONTEXTVALID( pControlBlock ) );

  SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO, "Processing SSC message: %x for context: %x", pMsg->type, pContext ));

  /* Acquire Lock to tx only!                                              */
  WLANSSC_LOCKTX( pControlBlock );

  if( VOS_TRUE == pControlBlock->bChipSuspended )
  {
    /* Do not process any message other than for freeing mem avail         */

    if( WLANSSC_MEMAVAIL_MESSAGE == pMsg->type )
    {
      SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR,"Returning mem avail vos pkt : %x", pMsg->bodyval));
      (v_VOID_t) vos_pkt_return_packet( (vos_pkt_t *)pMsg->bodyval );
    }

    SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR,"Cannot process tx msg while chip suspended"));

    WLANSSC_UNLOCKTX( pControlBlock );
    return VOS_STATUS_E_FAILURE;
  }

  /* Update the action based on the message type                           */
  switch( pMsg->type )
  {
    case WLANSSC_TXPENDING_MESSAGE:

      /** Critical to decrement count before handling xmit
          to reduce race conditions - this guarantees we
          process at least one message after StartXmit
          might have bailed out
          Need the extra check to avoid counting below
          zero: note that we can only increment in other
          thread so this should be safe on race conditions
       */
      if( gWLANSSC_TxMsgCnt )
      {
        gWLANSSC_TxMsgCnt--;   
      }

      /* Execute the transmit event : this is the only event from tx thread*/
      if( VOS_STATUS_SUCCESS != WLANSSC_ExecuteEvent( pControlBlock, 
                                                      WLANSSC_TRANSMIT_EVENT ) )
      {
        /* Should never happen!                                            */
        SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR,"Cannot transmit in state: %d", pControlBlock->eState));

        WLANSSC_UNLOCKTX( pControlBlock );
        return VOS_STATUS_E_FAILURE;
      }

      break;

    case WLANSSC_MEMAVAIL_MESSAGE:
      /* Store the memory available in the controlblock                    */
      if( NULL == pControlBlock->pMemAvailFrame )
      {
        pControlBlock->pMemAvailFrame = (vos_pkt_t *)pMsg->bodyval;
      }
      else
      {
        SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO,"Existing frame: %x; Extra frame received: %x freeing new and continuing", 
                    pControlBlock->pMemAvailFrame,
                    pMsg->bodyval ));

       /* We could actually return here, but to be safe, we read more     */
        (v_VOID_t) vos_pkt_return_packet( (vos_pkt_t *)pMsg->bodyval );
      }

      /* Fall through to process rx again                                  */

    case WLANSSC_RXPENDING_MESSAGE:

      /* Execute the transmit event : this is the only event from tx thread*/
      if( VOS_STATUS_SUCCESS != WLANSSC_ExecuteEvent( pControlBlock, 
                                                      WLANSSC_RECEIVE_EVENT ) )
      {
        /* Should never happen!                                            */
        SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR,"Cannot receive in state: %d", pControlBlock->eState));

        WLANSSC_UNLOCKTX( pControlBlock );
        return VOS_STATUS_E_FAILURE;
      }

      break;

    default:
      WLANSSC_ASSERT( 0 );
      WLANSSC_UNLOCKTX( pControlBlock );
      return VOS_STATUS_E_FAILURE;
  }

  WLANSSC_UNLOCKTX( pControlBlock );

  return VOS_STATUS_SUCCESS;

} /* WLANSSC_ProcessMsg() */


/**
 @brief WLANSSC_FreeMsg is used to free any messages pending with
 the VOS services

 Before invoking this function, SSC_Start() must already have been called.

 @param pMsg: pointer to the message being serialized via VOS

 @param pContext: the context provided by the VOS services

 @see vos_tx_mq_serialize

 @return Result of the function call
*/
VOS_STATUS WLANSSC_FreeMsg
(
  v_PVOID_t  pContext,
  vos_msg_t *pMsg
)
{
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  WLANSSC_ASSERT( NULL != pMsg );

  SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO, "Freeing message: %x for context: %x", pMsg->type, pContext ));

  pMsg->bodyptr = 0;
  pMsg->bodyval = 0;
  pMsg->type = 0;

  return VOS_STATUS_SUCCESS;

} /* WLANSSC_FreeMsg() */



/*---------------------------------------------------------------------------
 * Internal Function implementation
 * ------------------------------------------------------------------------*/

/**
 @brief WLANSSC_ExecuteEvent is used to trigger an event handler for
 a specific event on the given SSC context.
 If the handler is not registered for the current state and event, the call
 will return failure.

 This function is likely to result in a state change
  
 @param Handle: SSC control block to operate on

 @param Event: Event to trigger handler for

 @see

 @return Result of the EventHandler
*/
static VOS_STATUS WLANSSC_ExecuteEvent
(
  WLANSSC_ControlBlockType *pControlBlock,
  WLANSSC_EventType         eEvent
)
{
  VOS_STATUS       eStatus;
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  WLANSSC_ASSERT( NULL != pControlBlock );

  SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO, "Event: %d in state : %d", 
               eEvent,
               pControlBlock->eState));

  WLANSSC_ASSERT( VOS_TRUE == WLANSSC_ISCONTEXTVALID( pControlBlock ) );

  if( NULL != WLANSSC_StateTable[pControlBlock->eState][eEvent] )
  {
    SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO, "Executing event"));

    eStatus = WLANSSC_StateTable[pControlBlock->eState][eEvent]( pControlBlock );
  }
  else
  {
    SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "Invalid event in current state"));
    eStatus = VOS_STATUS_E_PERM;
  }

  return eStatus;

} /* WLANSSC_ExecuteEvent() */


/**
 @brief WLANSSC_TransitionState is used to change the state of the control
 block to the required value.

 Any actions relevant to this transition might be performed here

 @param Handle: SSC control block to operate on

 @param State: New state to set

 @see

 @return
*/
static v_VOID_t WLANSSC_TransitionState
(
  WLANSSC_ControlBlockType *pControlBlock,
  WLANSSC_StateType         eState
)
{

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  WLANSSC_ASSERT( NULL != pControlBlock );

  SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO, "Changing state from %d to %d", 
               pControlBlock->eState, 
               eState));

  pControlBlock->eState = eState;

} /* WLANSSC_TransitionState() */


/**
 @brief WLANSSC_ResetStats is used to reset all the statistics in the SSC
 control block

 @param pControlBlock: SSC control block to operate on

 @see

 @return
*/
static v_VOID_t WLANSSC_ResetStats
(
  WLANSSC_ControlBlockType *pControlBlock
)
{

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  WLANSSC_ASSERT( NULL != pControlBlock );

  pControlBlock->sStatsInfo.uNumInterrupts = 0;
  pControlBlock->sStatsInfo.uNumRxCmd53 = 0;
  pControlBlock->sStatsInfo.uNumRxFrames = 0;
  pControlBlock->sStatsInfo.uNumRxPadding = 0;
  pControlBlock->sStatsInfo.uNumRxPartial = 0;
  pControlBlock->sStatsInfo.uNumRxRemainder = 0;
  pControlBlock->sStatsInfo.uNumTxCmd53 = 0;
  pControlBlock->sStatsInfo.uNumTxFrames = 0;

} /* WLANSSC_ResetStats() */


/**
 @brief WLANSSC_InitContext is used to initialize the key elements in the
 SSC control block to ensure validity.

 This is for init the context only! This does *not* init the ControlBlock
 itself!

 @param Handle: SSC control block to operate on

 @see WLANSSC_InitControlBlock, WLANSSC_DestroyContext

 @return Result of the function call
*/
static VOS_STATUS WLANSSC_InitContext
(
  WLANSSC_ControlBlockType *pControlBlock
)
{

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  WLANSSC_ASSERT( NULL != pControlBlock );

  SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO, "Initing SSC context : %x", 
               pControlBlock));

  pControlBlock->bInitialized = VOS_TRUE;
  pControlBlock->nCookie = WLANSSC_COOKIE;

  return VOS_STATUS_SUCCESS;

} /* WLANSSC_InitContext() */


/**
 @brief WLANSSC_InitControlBlock is used to initialize the contents of the
 control block.

 @param Handle: SSC control block to operate on

 @see WLANSSC_InitContext, WLANSSC_DestroyControlBlock

 @return Result of the function call
*/
static VOS_STATUS WLANSSC_InitControlBlock
(
  WLANSSC_ControlBlockType *pControlBlock
)
{
  v_U32_t   uRegBufferCnt;
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  WLANSSC_ASSERT( NULL != pControlBlock );

  SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO, "Initing SSC control block : %x", 
               pControlBlock));

  WLANSSC_ASSERT( VOS_TRUE == WLANSSC_ISCONTEXTVALID( pControlBlock ) );

  /* Tx buffer prep                                                        */
  pControlBlock->pTxBuffer = (v_U8_t *) vos_mem_dma_malloc( WLANSSC_MAXTXBUFSIZE );

  if( NULL == pControlBlock->pTxBuffer )
  {
    WLANSSC_ASSERT( 0 );

    SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "Failed to allocate tx buffer"));
    return VOS_STATUS_E_RESOURCES;
  }

  vos_mem_zero( pControlBlock->pTxBuffer, WLANSSC_MAXTXBUFSIZE );

  /* Copy over the start descriptor this once since the header is static   */
  vos_mem_copy( &pControlBlock->pTxBuffer[0], 
                &gWLANSSC_TxStartDescriptor, 
                sizeof(WLANSSC_TxStartDescriptorType) );

  /* Rx buffer prep                                                        */
  pControlBlock->pRxBuffer = (v_U8_t *) vos_mem_dma_malloc( WLANSSC_MAXRXBUFSIZE );

  if( NULL == pControlBlock->pRxBuffer )
  {
    WLANSSC_ASSERT( 0 );

    SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "Failed to allocate rx buffer"));
    return VOS_STATUS_E_RESOURCES;
  }

  vos_mem_zero( pControlBlock->pRxBuffer, WLANSSC_MAXRXBUFSIZE );

  /* Reg buffer prep (array of memory spaces for each reg buffer type)        */
  for( uRegBufferCnt = 0; uRegBufferCnt < WLANSSC_MAX_REGBUFFER; uRegBufferCnt++ )
  {
    pControlBlock->pRegBuffer[uRegBufferCnt] = (v_U32_t *) vos_mem_dma_malloc( WLANSSC_REGISTERSIZE );

    if( NULL == pControlBlock->pRegBuffer[uRegBufferCnt] )
    {
      WLANSSC_ASSERT( 0 );

      SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "Failed to allocate reg buffer: %d", uRegBufferCnt ));
      return VOS_STATUS_E_RESOURCES;
    }

    vos_mem_zero( pControlBlock->pRegBuffer[uRegBufferCnt], WLANSSC_REGISTERSIZE );
  }

  /* Other data structures init                                            */
  pControlBlock->eState = WLANSSC_CLOSED_STATE;

  vos_mem_zero( &(pControlBlock->stClientCbacks),
                sizeof(WLANSSC_ClientCallBacksType) );

  pControlBlock->uSuspendedFlowMask = 0;

  pControlBlock->uInterruptEnableMask = 0;
  pControlBlock->uASICInterruptMask = 0;

  pControlBlock->uInterruptSnapshot = WLANSSC_INVALIDINTERRUPTSNAPSHOT;

  pControlBlock->hSALHandle = NULL;

  pControlBlock->pTxChain = NULL;

  pControlBlock->pTempRxFrame = NULL;

  pControlBlock->pMemAvailFrame = NULL;

  pControlBlock->sSchedulerInfo.bTxPending = VOS_FALSE;

  pControlBlock->sSchedulerInfo.uRxProcessedCnt = 0;

  pControlBlock->sRxBufferInfo.uCurrentRxDataPosition = 0;

  pControlBlock->sRxBufferInfo.uCurrentRxDataSize = 0;

  pControlBlock->sRxBufferInfo.uPendingTargetData = 0;

  pControlBlock->bChipSuspended = VOS_FALSE;

  WLANSSC_ResetStats( pControlBlock );

  if( VOS_STATUS_SUCCESS != vos_lock_init( &(pControlBlock->stSSCTxLock) ) )
  {
    SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO, "Initing SSC Tx lock failed"));
    return VOS_STATUS_E_RESOURCES;
  }

  if( VOS_STATUS_SUCCESS != vos_lock_init( &(pControlBlock->stSSCRxLock) ) )
  {
    SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO, "Initing SSC Rx lock failed"));
    return VOS_STATUS_E_RESOURCES;
  }

  return VOS_STATUS_SUCCESS;

} /* WLANSSC_InitControlBlock() */


/**
 @brief WLANSSC_DestroyContext is used to invalidate the key elements in the
 SSC control block.

 This is for the context only! This does *not* free the ControlBlock
 itself!

 @param Handle: SSC control block to operate on

 @see WLANSSC_DestroyControlBlock, WLANSSC_InitContext

 @return Result of the function call
*/
static VOS_STATUS WLANSSC_DestroyContext
(
  WLANSSC_ControlBlockType *pControlBlock
)
{

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  WLANSSC_ASSERT( NULL != pControlBlock );

  SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO, "Destroying SSC context : %x", 
               pControlBlock));

  pControlBlock->bInitialized = VOS_FALSE;
  pControlBlock->nCookie = 0;

  return VOS_STATUS_SUCCESS;

} /* WLANSSC_DestroyContext() */


/**
 @brief WLANSSC_DestroyControlBlock is used to free the contents of the
 control block.

 @param Handle: SSC control block to operate on

 @see WLANSSC_DestroyContext, WLANSSC_InitControlBlock

 @return Result of the function call
*/
static VOS_STATUS WLANSSC_DestroyControlBlock
(
  WLANSSC_ControlBlockType *pControlBlock
)
{
  v_U32_t   uRegBufferCnt;
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  WLANSSC_ASSERT( NULL != pControlBlock );

  SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO, "Destroying SSC control block : %x", 
               pControlBlock));

  WLANSSC_ASSERT( VOS_TRUE == WLANSSC_ISCONTEXTVALID( pControlBlock ) );

  vos_mem_dma_free( pControlBlock->pTxBuffer );

  vos_mem_dma_free( pControlBlock->pRxBuffer );

  for( uRegBufferCnt = 0; uRegBufferCnt < WLANSSC_MAX_REGBUFFER; uRegBufferCnt++ )
  {
    vos_mem_dma_free( pControlBlock->pRegBuffer[uRegBufferCnt] );
  }

  pControlBlock->eState = WLANSSC_CLOSED_STATE;

  vos_mem_zero( &(pControlBlock->stClientCbacks),
                sizeof(WLANSSC_ClientCallBacksType) );

  pControlBlock->uSuspendedFlowMask = 0;

  pControlBlock->uInterruptEnableMask = 0;
  pControlBlock->uASICInterruptMask = 0;

  pControlBlock->uInterruptSnapshot = WLANSSC_INVALIDINTERRUPTSNAPSHOT;

  pControlBlock->hSALHandle = NULL;

  pControlBlock->pTxChain = NULL;

  /* Free the temp frame first                                             */
  vos_pkt_return_packet( pControlBlock->pTempRxFrame );
  pControlBlock->pTempRxFrame = NULL;

  /* Free the temp frame first                                             */
  vos_pkt_return_packet( pControlBlock->pMemAvailFrame );
  pControlBlock->pMemAvailFrame = NULL;

  pControlBlock->sSchedulerInfo.bTxPending = VOS_FALSE;

  pControlBlock->sSchedulerInfo.uRxProcessedCnt = 0;

  pControlBlock->sRxBufferInfo.uCurrentRxDataPosition = 0;

  pControlBlock->sRxBufferInfo.uCurrentRxDataSize = 0;

  pControlBlock->sRxBufferInfo.uPendingTargetData = 0;

  pControlBlock->bChipSuspended = VOS_FALSE;

  if( VOS_STATUS_SUCCESS != vos_lock_destroy( &(pControlBlock->stSSCTxLock) ) )
  {
    SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO, "Destroying SSC Tx lock failed"));
    return VOS_STATUS_E_RESOURCES;
  }

  if( VOS_STATUS_SUCCESS != vos_lock_destroy( &(pControlBlock->stSSCRxLock) ) )
  {
    SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO, "Destroying SSC Rx lock failed"));
    return VOS_STATUS_E_RESOURCES;
  }

  return VOS_STATUS_SUCCESS;

} /* WLANSSC_DestroyControlBlock() */


/**
 @brief WLANSSC_EnableInterrupt is used to enable one or more interrupts
 in a given SSC instance

 This not only updates the cache within the SSC but also writes to the 
 interrupt enable register on the target as required.

 Caller must ensure proper serialization to access the control block

 @param Handle: SSC control block to operate on

 @param InterruptMask: Mask of interrupts to enable (per sif.h)

 @see WLANSSC_DisableInterrupt

 @return Result of the function call
*/
static VOS_STATUS WLANSSC_EnableInterrupt
(
  WLANSSC_ControlBlockType *pControlBlock,
  v_U32_t                   uInterruptMask
)
{
  v_U32_t uMaskToSet;
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  WLANSSC_ASSERT( NULL != pControlBlock );

  WLANSSC_ASSERT( VOS_TRUE == WLANSSC_ISCONTEXTVALID( pControlBlock ) );

  uMaskToSet = uInterruptMask | pControlBlock->uInterruptEnableMask |
    pControlBlock->uASICInterruptMask;

  SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO, "New interrupts: %x for ctrl blk : %x", 
               uMaskToSet,
               pControlBlock));

  /* Update register on the Libra device                                 */
  if( VOS_STATUS_SUCCESS != WLANSSC_WriteRegister( pControlBlock,
                                                   QWLAN_SIF_SIF_INT_EN_REG, 
                                                   &uMaskToSet,
                                                   WLANSSC_TX_REGBUFFER ) )
  {
    return VOS_STATUS_E_FAILURE;
  }

  return VOS_STATUS_SUCCESS;

} /* WLANSSC_EnableInterrupt() */


/**
 @brief WLANSSC_DisableInterrupt is used to disable one or more interrupts
 in a given SSC instance

 This not only updates the cache within the SSC but also writes to the 
 interrupt enable register on the target as required.

 Caller must ensure proper serialization to access the control block

 @param Handle: SSC control block to operate on

 @param InterruptMask: Mask of interrupts to disable (per sif.h)

 @see WLANSSC_EnableInterrupt

 @return Result of the function call
*/
static VOS_STATUS WLANSSC_DisableInterrupt
(
  WLANSSC_ControlBlockType *pControlBlock,
  v_U32_t                   uInterruptMask
)
{
  v_U32_t  uMaskToSet;
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  WLANSSC_ASSERT( NULL != pControlBlock );

  WLANSSC_ASSERT( VOS_TRUE == WLANSSC_ISCONTEXTVALID( pControlBlock ) );

  uMaskToSet = ~uInterruptMask & 
    ( pControlBlock->uInterruptEnableMask | pControlBlock->uASICInterruptMask );

  SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO, "Disabled interrupts: %x for ctrl blk : %x", 
               uMaskToSet,
               pControlBlock));

  /* Update register on the Libra device                                 */
  if( VOS_STATUS_SUCCESS != WLANSSC_WriteRegister( pControlBlock,
                                                   QWLAN_SIF_SIF_INT_EN_REG, 
                                                   &(uMaskToSet),
                                                   WLANSSC_INT_REGBUFFER ) )
  {
    return VOS_STATUS_E_FAILURE;
  }

  return VOS_STATUS_SUCCESS;

} /* WLANSSC_DisableInterrupt() */


/**
 @brief WLANSSC_ClearAllInterrupts is used to clear all pending interrupts
 in a given SSC instance

  This results in an update on the target.

 Caller must ensure proper serialization to access the control block

 @param Handle: SSC control block to operate on

 @param InterruptMask: Mask of interrupts to disable (per sif.h)

 @see WLANSSC_EnableInterrupt

 @return Result of the function call
*/
static VOS_STATUS WLANSSC_ClearAllInterrupts
(
  WLANSSC_ControlBlockType *pControlBlock
)
{
  v_U32_t  uInterruptMaskToBeCleared = 0;
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  WLANSSC_ASSERT( NULL != pControlBlock );

  /* Do not clear out the ASIC interrupts or Rx data avail from Libra      */
  uInterruptMaskToBeCleared =   pControlBlock->uInterruptSnapshot & 
    ~(pControlBlock->uASICInterruptMask | QWLAN_SIF_SIF_INT_EN_RX_FIFO_DATA_AVAIL_INTR_EN_MASK);

  /* Optimization to avoid unnecessary bus accesses                        */
  if( 0 == uInterruptMaskToBeCleared )
  {
    SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO, "SSC ignoring zero clear for interrupts"));
    return VOS_STATUS_SUCCESS;
  }

  SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO, "Clearing all interrupts for ctrl blk : %x; %x", 
               pControlBlock, uInterruptMaskToBeCleared));

  /* Update register on the Libra device                                   */
  if( VOS_STATUS_SUCCESS != WLANSSC_WriteRegister( pControlBlock,
                                                   QWLAN_SIF_SIF_INT_CLR_REG,
                                                   &uInterruptMaskToBeCleared,
                                                   WLANSSC_INT_REGBUFFER ) )
  {
    return VOS_STATUS_E_FAILURE;
  }

  return VOS_STATUS_SUCCESS;

} /* WLANSSC_ClearAllInterrupts() */


/**
 @brief WLANSSC_ReadRegister is used to read a register value for a given
 SSC instance

 NOTE: This will ALWAYS read 32 bits since all SIF registers are 32-bit

 @param Handle: SSC control block to operate on

 @param uRegister: Register address to read from

 @param pValue: location to read the value into (must have at least 32-bits)

 @see WLANSSC_WriteRegister

 @return Result of the function call
*/
static VOS_STATUS WLANSSC_ReadRegister
(
  WLANSSC_ControlBlockType *pControlBlock,
  v_U32_t                   uRegister,
  v_U32_t                  *pValue,
  WLANSSC_RegBufferType     eRegType
)
{
  WLANSAL_Cmd53ReqType   sCmd53Request;
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  WLANSSC_ASSERT( NULL != pControlBlock );

  WLANSSC_ASSERT( NULL != pValue );

  if (vos_is_logp_in_progress(VOS_MODULE_ID_SSC, NULL)) {
     printk("%s: LOGP in progress, aborting this command \n", __FUNCTION__);
     return VOS_STATUS_SUCCESS;
  }

  vos_mem_zero( &sCmd53Request, sizeof(WLANSAL_Cmd53ReqType) );

  sCmd53Request.address = uRegister;

  /* Non-FIFO addresses are all incremental reads                          */
  sCmd53Request.addressHandle = WLANSAL_ADDRESS_INCREMENT;

  sCmd53Request.busCompleteCB = NULL;

  sCmd53Request.busDirection = WLANSAL_DIRECTION_READ;

  /* For now, always synchronous                                           */
  sCmd53Request.callSync = WLANSAL_CALL_SYNC;

  /* For a register, byte mode is sufficient                               */
  sCmd53Request.mode = WLANSAL_MODE_BYTE;
  sCmd53Request.dataSize = WLANSSC_REGISTERSIZE;

  WLANSSC_ASSERT( WLANSSC_MAX_REGBUFFER > eRegType );

  sCmd53Request.dataPtr = (v_U8_t *) pControlBlock->pRegBuffer[eRegType];

  sCmd53Request.function = WLANSSC_SDIOFUNCTION1;

  if( VOS_STATUS_SUCCESS != WLANSAL_Cmd53( pControlBlock->hSALHandle,
                                           &sCmd53Request ) )
  {
    return VOS_STATUS_E_FAILURE;
  }

  /* Copy over the value from the dma buffer                               */
  *pValue = *(pControlBlock->pRegBuffer[eRegType]);

  SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO, "Read register ptr: %x for ctrl blk : %x; register: %x", 
               pValue,
               pControlBlock,
               uRegister ));

  return VOS_STATUS_SUCCESS;

} /* WLANSSC_ReadRegister() */


/**
 @brief WLANSSC_WriteRegister is used to write a register value to the Libra
 device

 NOTE: This will *always write 32-bits since SIF registers are all 32-bit
 wide.

 @param Handle: SSC control block to operate on

 @param uRegister: Register address to write to

 @param pValue: location of the write data (will take 32-bits from here)

 @see WLANSSC_ReadRegister

 @return Result of the function call
*/
static VOS_STATUS WLANSSC_WriteRegister
(
  WLANSSC_ControlBlockType *pControlBlock,
  v_U32_t                   uRegister,
  v_U32_t                  *pValue,
  WLANSSC_RegBufferType     eRegType
)
{
  WLANSAL_Cmd53ReqType   sCmd53Request;
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  WLANSSC_ASSERT( NULL != pControlBlock );

  WLANSSC_ASSERT( NULL != pValue );

  WLANSSC_ASSERT( WLANSSC_MAX_REGBUFFER > eRegType );

  if (vos_is_logp_in_progress(VOS_MODULE_ID_SSC, NULL)) {
     printk("%s: LOGP in progress, aborting this command \n", __FUNCTION__);
     return VOS_STATUS_SUCCESS;
  }

  /* Copy over the value into the dma buffer                               */
  *(pControlBlock->pRegBuffer[eRegType]) = *pValue;

  vos_mem_zero( &sCmd53Request, sizeof(WLANSAL_Cmd53ReqType) );

  sCmd53Request.address = uRegister;

  /* Non-FIFO addresses are all incremental reads                          */
  sCmd53Request.addressHandle = WLANSAL_ADDRESS_INCREMENT;

  sCmd53Request.busCompleteCB = NULL;

  sCmd53Request.busDirection = WLANSAL_DIRECTION_WRITE;

  /* For now, always synchronous                                           */
  sCmd53Request.callSync = WLANSAL_CALL_SYNC;

  /* For a register, byte mode is sufficient                               */
  sCmd53Request.mode = WLANSAL_MODE_BYTE;
  sCmd53Request.dataSize = WLANSSC_REGISTERSIZE;

  sCmd53Request.dataPtr = (v_U8_t *) pControlBlock->pRegBuffer[eRegType];

  sCmd53Request.function = WLANSSC_SDIOFUNCTION1;

  if( VOS_STATUS_SUCCESS != WLANSAL_Cmd53( pControlBlock->hSALHandle,
                                           &sCmd53Request ) )
  {
    return VOS_STATUS_E_FAILURE;
  }

  SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO, "Write register ptr: %x for ctrl blk : %x; register: %x", 
               pValue,
               pControlBlock,
               uRegister ));

  return VOS_STATUS_SUCCESS;

} /* WLANSSC_WriteRegister() */


/**
 @brief WLANSSC_ReadRegisterFuncZero is used to read a register value for a 
 given SSC instance using function 0

 NOTE: This will ALWAYS read 8 bits since all func0 SIF registers are 8-bit

 @param Handle: SSC control block to operate on

 @param uRegister: Register address to read from

 @param pValue: location to read the value into (must have at least 8-bits)

 @see WLANSSC_WriteRegisterFuncZero

 @return Result of the function call
*/
static VOS_STATUS WLANSSC_ReadRegisterFuncZero
(
  WLANSSC_ControlBlockType *pControlBlock,
  v_U32_t                   uRegister,
  v_U8_t                   *pValue,
  WLANSSC_RegBufferType     eRegType
)
{
  WLANSAL_Cmd52ReqType   sCmd52Request;
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  WLANSSC_ASSERT( NULL != pControlBlock );

  WLANSSC_ASSERT( NULL != pValue );

  WLANSSC_ASSERT( WLANSSC_MAX_REGBUFFER > eRegType );

  vos_mem_zero( &sCmd52Request, sizeof(WLANSAL_Cmd52ReqType) );

  sCmd52Request.address = uRegister;

  sCmd52Request.busDirection = WLANSAL_DIRECTION_READ;

  sCmd52Request.dataPtr = (v_U8_t *) pControlBlock->pRegBuffer[eRegType];
  
  sCmd52Request.function = WLANSSC_SDIOFUNCTION0;

  if( VOS_STATUS_SUCCESS != WLANSAL_Cmd52( pControlBlock->hSALHandle,
                                           &sCmd52Request ) )
  {
    return VOS_STATUS_E_FAILURE;
  }

  /* Copy over the value from the dma buffer                               */
  *pValue = *((v_U8_t *) pControlBlock->pRegBuffer[eRegType]);

  SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO, "Read func0 register ptr: %x for ctrl blk : %x; register: %x", 
               pValue,
               pControlBlock,
               uRegister ));

  return VOS_STATUS_SUCCESS;

} /* WLANSSC_ReadRegisterFuncZero() */


/**
 @brief WLANSSC_WriteRegisterFuncZero is used to write a register value to 
 the Libra device using function zero

 NOTE: This will *always* write 8-bits since func0 SIF registers are all 
 8-bit wide.

 @param Handle: SSC control block to operate on

 @param uRegister: Register address to write to

 @param pValue: location of the write data (will take 8-bits from here)

 @see WLANSSC_ReadRegisterFuncZero

 @return Result of the function call
*/
static VOS_STATUS WLANSSC_WriteRegisterFuncZero
(
  WLANSSC_ControlBlockType *pControlBlock,
  v_U32_t                   uRegister,
  v_U8_t                   *pValue,
  WLANSSC_RegBufferType     eRegType
)
{
  WLANSAL_Cmd52ReqType   sCmd52Request;
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  WLANSSC_ASSERT( NULL != pControlBlock );

  WLANSSC_ASSERT( NULL != pValue );

  WLANSSC_ASSERT( WLANSSC_MAX_REGBUFFER > eRegType );

  /* Copy over the value into the dma buffer                               */
  *((v_U8_t *) pControlBlock->pRegBuffer[eRegType]) = *pValue;

  vos_mem_zero( &sCmd52Request, sizeof(WLANSAL_Cmd52ReqType) );

  sCmd52Request.address = uRegister;

  sCmd52Request.busDirection = WLANSAL_DIRECTION_WRITE;

  sCmd52Request.dataPtr = (v_U8_t *) pControlBlock->pRegBuffer[eRegType];

  sCmd52Request.function = WLANSSC_SDIOFUNCTION0;

  if( VOS_STATUS_SUCCESS != WLANSAL_Cmd52( pControlBlock->hSALHandle,
                                           &sCmd52Request ) )
  {
    return VOS_STATUS_E_FAILURE;
  }

  SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO, "Write func0 register ptr: %x for ctrl blk : %x; register: %x", 
               pValue,
               pControlBlock,
               uRegister ));

  return VOS_STATUS_SUCCESS;

} /* WLANSSC_WriteRegisterFuncZero() */


/**
 @brief WLANSSC_ResetChip is used to hard reset the Libra device

  NOTE: power-up init sequence might be required to make the chip operational
  again 

 @param Handle: SSC control block to operate on


 @see

 @return Result of the function call
*/
static VOS_STATUS WLANSSC_ResetChip
(
  WLANSSC_ControlBlockType *pControlBlock
)
{
  v_U32_t uRegValue, uResetCount = 0;
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  WLANSSC_ASSERT( NULL != pControlBlock );

  WLANSSC_ASSERT( VOS_TRUE == WLANSSC_ISCONTEXTVALID( pControlBlock ) );

  SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO, "Resetting Libra device for ctrl blk : %x", 
               pControlBlock));

  /* Do we need to write to cccr io_reset?                                 */

  /* Reset the WLAN CSR access engine in SIF                               */
  if( VOS_STATUS_SUCCESS != WLANSSC_ReadRegister(pControlBlock,
                                                 QWLAN_SIF_SIF_TOP_CONTROL_REG,
                                                 &uRegValue,
                                                 WLANSSC_TX_REGBUFFER) )
  {
    return VOS_STATUS_E_FAILURE;
  }

  uRegValue |= QWLAN_SIF_SIF_TOP_CONTROL_CSR_ACC_FIFO_MGNT_RESET_MASK;

  if( VOS_STATUS_SUCCESS != WLANSSC_WriteRegister(pControlBlock,
                                                  QWLAN_SIF_SIF_TOP_CONTROL_REG,
                                                  &uRegValue,
                                                  WLANSSC_TX_REGBUFFER) )
  {
    return VOS_STATUS_E_FAILURE;
  }

  SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO, "Waiting for SIF CSR engine reset"));

  while( WLANSSC_RESETLOOPCOUNT > uResetCount++ )
  {
    if( VOS_STATUS_SUCCESS != WLANSSC_ReadRegister(pControlBlock,
                                                   QWLAN_SIF_SIF_TOP_CONTROL_REG,
                                                   &uRegValue,
                                                   WLANSSC_TX_REGBUFFER) )
    {
      return VOS_STATUS_E_FAILURE;
    }

    if( !(QWLAN_SIF_SIF_TOP_CONTROL_CSR_ACC_FIFO_MGNT_RESET_MASK & uRegValue))
    {
      /* Reset complete                                                    */
      break;
    }
    else
    {
      /* Need to wait and keep polling                                     */
      vos_sleep(WLANSSC_RESETLOOPWAIT);
    }
  }

  SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO, "SIF CSR engine reset complete"));

  /* Reset the WLAN CSR access engine in SIF                               */
  if( VOS_STATUS_SUCCESS != WLANSSC_ReadRegister(pControlBlock,
                                                 QWLAN_SIF_SIF_TOP_CONTROL_REG,
                                                 &uRegValue,
                                                 WLANSSC_TX_REGBUFFER) )
  {
    return VOS_STATUS_E_FAILURE;
  }

  uRegValue |= (QWLAN_SIF_SIF_TOP_CONTROL_RX_FIFO_MGNT_SOFT_RESET_MASK |
                QWLAN_SIF_SIF_TOP_CONTROL_TX_FIFO_MGNT_SOFT_RESET_MASK);

  if( VOS_STATUS_SUCCESS != WLANSSC_WriteRegister(pControlBlock,
                                                  QWLAN_SIF_SIF_TOP_CONTROL_REG,
                                                  &uRegValue,
                                                  WLANSSC_TX_REGBUFFER) )
  {
    return VOS_STATUS_E_FAILURE;
  }

  SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO, "Waiting for SIF tx/rx engine reset"));

  while( WLANSSC_RESETLOOPCOUNT > uResetCount++ )
  {
    if( VOS_STATUS_SUCCESS != WLANSSC_ReadRegister(pControlBlock,
                                                   QWLAN_SIF_SIF_TOP_CONTROL_REG,
                                                   &uRegValue,
                                                   WLANSSC_TX_REGBUFFER) )
    {
      return VOS_STATUS_E_FAILURE;
    }

    if( !( (QWLAN_SIF_SIF_TOP_CONTROL_RX_FIFO_MGNT_SOFT_RESET_MASK |
            QWLAN_SIF_SIF_TOP_CONTROL_TX_FIFO_MGNT_SOFT_RESET_MASK) & uRegValue) )
    {
      /* Reset complete                                                    */
      break;
    }
    else
    {
      /* Need to wait and keep polling                                     */
      vos_sleep(WLANSSC_RESETLOOPWAIT);
    }
  }

  SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO, "SIF reset complete"));

  return VOS_STATUS_SUCCESS;

} /* WLANSSC_ResetChip() */


/**
 @brief WLANSSC_EnableTx is used to enable the tx flow in SSC: 
 this will resume all outgoing data packets as they become available

 @param Handle: SSC control block to operate on

 @see WLANSSC_DisableTx

 @return Result of the function call
*/
static VOS_STATUS WLANSSC_EnableTx
(
  WLANSSC_ControlBlockType *pControlBlock
)
{
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /* Start tx to trigger tx resume: regardless of whether tx pending       */
  if( VOS_STATUS_SUCCESS != WLANSSC_StartTransmit( (WLANSSC_HandleType) pControlBlock ) )
  {
    WLANSSC_ASSERT( 0 );
    return VOS_STATUS_E_FAILURE;
  }

  return VOS_STATUS_SUCCESS;

} /* WLANSSC_EnableTx() */


/**
 @brief WLANSSC_DisableTx is used to disable the tx flow in SSC: 
 this will prevent all outgoing data packets

 @param Handle: SSC control block to operate on

 @see WLANSSC_EnableTx

 @return Result of the function call
*/
static VOS_STATUS WLANSSC_DisableTx
(
  WLANSSC_ControlBlockType *pControlBlock
)
{
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /** No action for now: just the tx suspended mask should prevent tx from
      getting scheduled at all.
  */

  return VOS_STATUS_SUCCESS;

} /* WLANSSC_DisableTx() */


/**
 @brief WLANSSC_EnableRx is used to enable the rx flow in SSC: 
 this will resume all incoming data as available

 @param Handle: SSC control block to operate on

 @see WLANSSC_DisableRx

 @return Result of the function call
*/
static VOS_STATUS WLANSSC_EnableRx
(
  WLANSSC_ControlBlockType *pControlBlock
)
{
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /* We enable all rx interrupts here                                      */
  if( VOS_STATUS_SUCCESS != WLANSSC_EnableInterrupt(pControlBlock,
                                                    QWLAN_SIF_SIF_INT_EN_RX_FIFO_DATA_AVAIL_INTR_EN_MASK) )
  {
    SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "Error enabling interrupts"));
    return VOS_STATUS_E_FAILURE;
  }

  return VOS_STATUS_SUCCESS;

} /* WLANSSC_EnableRx() */


/**
 @brief WLANSSC_DisableRx is used to disable the rx flow in SSC: 
 this will prevent all incoming data

 @param Handle: SSC control block to operate on

 @see WLANSSC_EnableRx

 @return Result of the function call
*/
static VOS_STATUS WLANSSC_DisableRx
(
  WLANSSC_ControlBlockType *pControlBlock
)
{
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /* We disable all rx interrupts here                                     */
  if( VOS_STATUS_SUCCESS != WLANSSC_DisableInterrupt(pControlBlock,
                                                     QWLAN_SIF_SIF_INT_EN_RX_FIFO_DATA_AVAIL_INTR_EN_MASK) )
  {
    SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "Error disabling interrupts"));
    return VOS_STATUS_E_FAILURE;
  }

  return VOS_STATUS_SUCCESS;

} /* WLANSSC_DisableRx() */


/**
 @brief WLANSSC_SendData is used to transmit data over to the TX FIFO on the
 Libra chip for the given SSC instance

 @param Handle: SSC control block to operate on

 @param pBuffer: pointer to data buffer to transmit

 @param uDataSize: length of data in bytes in the buffer

 @see WLANSSC_ReceiveData

 @return Result of the function call
*/
static VOS_STATUS WLANSSC_SendData
(
  WLANSSC_ControlBlockType *pControlBlock,
  v_U8_t                   *pBuffer,
  v_U32_t                   uDataSize
)
{
  WLANSAL_Cmd53ReqType   sCmd53Request;
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  WLANSSC_ASSERT( NULL != pControlBlock );

  WLANSSC_ASSERT( NULL != pBuffer );

  vos_mem_zero( &sCmd53Request, sizeof(WLANSAL_Cmd53ReqType) );

  /* TX FIFO                                                               */
  sCmd53Request.address = QWLAN_SIF_TX_FIFO_ACC_PTR_REG;

  /* FIFO addresses are all fixed reads                                    */
  sCmd53Request.addressHandle = WLANSAL_ADDRESS_FIXED;

  sCmd53Request.busCompleteCB = WLANSSC_TxCompleteCallback;

  sCmd53Request.busDirection = WLANSAL_DIRECTION_WRITE;

  /* For now, always synchronous                                           */
  sCmd53Request.callSync = WLANSAL_CALL_SYNC;

  /* If length is less than one block then just use byte mode transfer     */
  sCmd53Request.mode = 
    (uDataSize < WLANSSC_BLOCKSIZE) ? WLANSAL_MODE_BYTE : WLANSAL_MODE_BLOCK;

  sCmd53Request.dataPtr = pBuffer;

  sCmd53Request.dataSize = uDataSize;

  sCmd53Request.function = WLANSSC_SDIOFUNCTION1;

  if( VOS_STATUS_SUCCESS != WLANSAL_Cmd53( pControlBlock->hSALHandle,
                                           &sCmd53Request ) )
  {
    return VOS_STATUS_E_FAILURE;
  }

  /* Update statistics                                                     */
  pControlBlock->sStatsInfo.uNumTxCmd53++;

  SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO, "Send data ptr: %x for ctrl blk : %x; size: %d", 
               pBuffer,
               pControlBlock,
               uDataSize ));

  return VOS_STATUS_SUCCESS;

} /* WLANSSC_SendData() */



/**
 @brief WLANSSC_TransmitCompleteNotifyAndReturnStatus is used to pass a 
 tx complete to a registered client and return the status passed in

 This is a helper function make code more compact

 @param Handle: SSC control block to operate on

 @see 

 @return 
  VOS_STATUS passed in

*/
static VOS_STATUS WLANSSC_TransmitCompleteNotifyAndReturnStatus
(
  WLANSSC_ControlBlockType *pControlBlock,
  VOS_STATUS                eStatusToBeReturned
)
{
  /* Notify client about tx complete failure                             */
  WLANSSC_ASSERT( NULL!= pControlBlock->stClientCbacks.pfnTxCompleteCback );

  pControlBlock->stClientCbacks.pfnTxCompleteCback( pControlBlock->pTxChain, 
                                                    eStatusToBeReturned, 
                                                    pControlBlock->stClientCbacks.UserData, 
                                                    pControlBlock->TxCompleteUserData );

  pControlBlock->pTxChain = NULL;
  pControlBlock->TxCompleteUserData = NULL;

  return eStatusToBeReturned;

} /* WLANSSC_TransmitCompleteNotifyAndReturnStatus() */



#ifdef WLANSSC_TX_AGGREGATION_INTERNAL 

/**
 @brief WLANSSC_TransmitChainInternal is used to gather data from client and
 send a bus request to send this data to Libra.

 This walks the chain one packet at a time and transmits the data to the
 SAL.

 @param Handle: SSC control block to operate on

 @param uMaxAllowedPktSize: Maxmimum data allowed by scheduler to grab
                            from client

 @see 

 @return 
          E_EMPTY indicates there was no packet to send
          SUCCESS indicates that a packet was enqueued
          FAILURE indicates that there was an error

*/
static VOS_STATUS WLANSSC_TransmitChainInternal
(
  WLANSSC_ControlBlockType *pControlBlock,
  v_U32_t                   uMaxAllowedPktSize
)
{
  v_U16_t                         u16PacketLength;
  WLANSSC_TxStartDescriptorType  *pTxStartDescriptor;
  v_U16_t                         u16PayloadSize = 0;
  v_SIZE_t                        sDummy = 0;
  vos_pkt_t                      *pTxPacket = NULL;
  VOS_STATUS                      eStatus = VOS_STATUS_SUCCESS;
  v_BOOL_t                        bUrgent;
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO, "SSC Walking packet chain for ctrl blk : %x", 
               pControlBlock));

  /* Get packet chain from client                                          */
  WLANSSC_ASSERT( NULL != 
                  pControlBlock->stClientCbacks.pfnGetMultipleTxPacketCback );

  /* There must be no pending pkt chain                                    */
  WLANSSC_ASSERT( NULL == pControlBlock->pTxChain );

  if( VOS_FALSE == pControlBlock->stClientCbacks.pfnGetMultipleTxPacketCback( uMaxAllowedPktSize,
                                                                              &(pControlBlock->pTxChain),
                                                                              pControlBlock->stClientCbacks.UserData,
                                                                              &(pControlBlock->TxCompleteUserData),
                                                                              &bUrgent) )
  {
    /* No more data to send - clear out tx pending flag                    */
    SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO, "No more data from TL"));
  }
  else
  {
    SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "TL has more data than can be chained in one shot by SSC - optimize buffer size!"));

    /* Serialize another message to SSC to resume operations after cmd53   */
    if( VOS_STATUS_SUCCESS != WLANSSC_StartTransmit( (WLANSSC_HandleType) pControlBlock ) )
    {
      WLANSSC_ASSERT( 0 );
      return VOS_STATUS_E_FAILURE;
    }
  }

  /* If pkt chain is NULL, then there are no pkts to send                  */
  if( NULL == pControlBlock->pTxChain )
  {
    return VOS_STATUS_E_EMPTY;
  }

  /* Extra protection removed to avoid overhead                            */
#ifdef FEATURE_LIBRA_UNSUPPORTED
  /* Clear buffer (after SD) to avoid confusing DXE from previous runs   */
  vos_mem_zero( &pControlBlock->pTxBuffer[sizeof(WLANSSC_TxStartDescriptorType)],
        WLANSSC_MAXTXBUFSIZE-sizeof(WLANSSC_TxStartDescriptorType) );
#endif /* FEATURE_LIBRA_UNSUPPORTED */

  pTxPacket = pControlBlock->pTxChain;

  while( NULL != pTxPacket )
  {
    /* Get the length of the packet for header prep                        */
    if( VOS_STATUS_SUCCESS != vos_pkt_get_packet_length( pTxPacket, 
                                                         &u16PacketLength ) ) 
    {
      WLANSSC_ASSERT( 0 );
      return VOS_STATUS_E_FAILURE;
    }

    /* Update statistics                                                   */
    pControlBlock->sStatsInfo.uNumTxFrames++;

    /* Sanity check to make sure we are not writing beyond allowed size    */
    WLANSSC_ASSERT( (WLANSSC_MAXTXBUFSIZE-sizeof(WLANSSC_TxStartDescriptorType)) >= (u16PayloadSize + u16PacketLength) );

    SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO, "SSC extracting packet: %x from chain : %x of length: %d", 
                 pTxPacket, 
                 pControlBlock->pTxChain , 
                 u16PacketLength));

    /** Extract data from the packet chain into tx buffer beyond start desc
    *   Extract into the tx buffer beyond the last packet.
    */
    sDummy = 0;

    if( VOS_STATUS_SUCCESS != vos_pkt_extract_data( pTxPacket,
                                                    0, 
                                                    &pControlBlock->pTxBuffer[sizeof(WLANSSC_TxStartDescriptorType) + u16PayloadSize],
                                                    &sDummy ) ) 
    {
      WLANSSC_ASSERT( 0 );
      return VOS_STATUS_E_FAILURE;
    }

  /* Important - next DXE header *must* start at 4-byte aligned location */
  u16PayloadSize += ( (u16PacketLength + 0x3) & ~0x3 );

  SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO, "SSC tx payload after extraction and alignment: %d", 
         u16PayloadSize));

    /* Walk the chain to the next packet                                   */
    eStatus = vos_pkt_walk_packet_chain( pTxPacket, &pTxPacket, VOS_FALSE);

    if( (VOS_STATUS_SUCCESS != eStatus) &&
        (VOS_STATUS_E_EMPTY != eStatus) )
    {
      SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "SSC Walking packet chain returned status : %d", 
                   eStatus));
      WLANSSC_ASSERT( 0 );

      return WLANSSC_TransmitCompleteNotifyAndReturnStatus( pControlBlock,
                                                            VOS_STATUS_E_FAILURE );
    }
  }

  SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO, "SSC aggregated tx packet length: %d", 
               u16PayloadSize));

  /* Prepare the header for the buffer                                     */
  pTxStartDescriptor = (WLANSSC_TxStartDescriptorType *) &pControlBlock->pTxBuffer[0];

  pTxStartDescriptor->controlInfo.sCtrlBits.uframeLength = u16PayloadSize;

  /* Compute the payload size to the nearest block size including SD       */
  u16PayloadSize = ( (u16PayloadSize + sizeof(WLANSSC_TxStartDescriptorType) + WLANSSC_BLOCKSIZE - 1)/WLANSSC_BLOCKSIZE ) * 
    WLANSSC_BLOCKSIZE;

  if( ( VOS_FALSE == bUrgent ) && ( u16PayloadSize < WLANSSC_TX_DELAY_THRESHOLD ) )
  {
      vos_sleep_us( 1000 ); //1ms
  }

  /* Send the packet chain over the bus                                    */
  if( VOS_STATUS_SUCCESS != WLANSSC_SendData( pControlBlock,
                                                &pControlBlock->pTxBuffer[0],
                                              u16PayloadSize ) )
  {
    WLANSSC_ASSERT( 0 );
    
    return WLANSSC_TransmitCompleteNotifyAndReturnStatus( pControlBlock,
                                                          VOS_STATUS_E_FAILURE );
  }
  
  /* Invoke the callback right away since it is a sync access              */
  return WLANSSC_TransmitCompleteNotifyAndReturnStatus( pControlBlock,
                                                        VOS_STATUS_SUCCESS );


} /* WLANSSC_TransmitChainInternal() */

#else

/**
 @brief WLANSSC_TransmitSingleInternal is used to gather data from client and
 send a bus request to send this data to Libra.

 This walks the chain one packet at a time and transmits the data to the
 SAL.

 @param Handle: SSC control block to operate on

 @param uMaxAllowedPktSize: Maxmimum data allowed by scheduler to grab
                            from client

 @see 

 @return 
          E_EMPTY indicates there was no packet to send
          SUCCESS indicates that a packet was enqueued
          FAILURE indicates that there was an error

*/
static VOS_STATUS WLANSSC_TransmitSingleInternal
(
  WLANSSC_ControlBlockType *pControlBlock,
  v_U32_t                   uMaxAllowedPktSize
)
{
  v_U16_t                         u16PacketLength;
  WLANSSC_TxStartDescriptorType  *pTxStartDescriptor;
  v_U16_t                         u16PayloadSize = 0;
  v_SIZE_t                        sDummy = 0;
  vos_pkt_t                      *pTxPacket = NULL;
  VOS_STATUS                      eStatus = VOS_STATUS_SUCCESS;
  v_BOOL_t                        bUrgent;
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO, "SSC Walking packet chain for ctrl blk : %x", 
               pControlBlock));

  /* Get packet chain from client                                          */
  WLANSSC_ASSERT( NULL != 
                  pControlBlock->stClientCbacks.pfnGetMultipleTxPacketCback );

  /* There must be no pending pkt chain                                    */
  WLANSSC_ASSERT( NULL == pControlBlock->pTxChain );

  if( VOS_FALSE == pControlBlock->stClientCbacks.pfnGetMultipleTxPacketCback( uMaxAllowedPktSize,
                                                                              &(pControlBlock->pTxChain),
                                                                              pControlBlock->stClientCbacks.UserData,
                                                                              &(pControlBlock->TxCompleteUserData),
                                                                              &bUrgent) )
  {
    /* No more data to send - clear out tx pending flag                    */
    SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO, "No more data from TL"));
  }
  else
  {
    SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "TL has more data than can be chained in one shot by SSC - optimize buffer size!"));

    /* Serialize another message to SSC to resume operations after cmd53   */
    if( VOS_STATUS_SUCCESS != WLANSSC_StartTransmit( (WLANSSC_HandleType) pControlBlock ) )
    {
      WLANSSC_ASSERT( 0 );
      return VOS_STATUS_E_FAILURE;
    }
  }

  /* If pkt chain is NULL, then there are no pkts to send                  */
  if( NULL == pControlBlock->pTxChain )
  {
    return VOS_STATUS_E_EMPTY;
  }

  /* Extra protection removed to avoid overhead                            */
#ifdef FEATURE_LIBRA_UNSUPPORTED
  /* Clear buffer (after SD) to avoid confusing DXE from previous runs   */
  vos_mem_zero( &pControlBlock->pTxBuffer[sizeof(WLANSSC_TxStartDescriptorType)],
        WLANSSC_MAXTXBUFSIZE-sizeof(WLANSSC_TxStartDescriptorType) );
#endif /* FEATURE_LIBRA_UNSUPPORTED */

  pTxPacket = pControlBlock->pTxChain;

  while( NULL != pTxPacket )
  {
    /* Get the length of the packet for header prep                        */
    if( VOS_STATUS_SUCCESS != vos_pkt_get_packet_length( pTxPacket, 
                                                         &u16PacketLength ) ) 
    {
      WLANSSC_ASSERT( 0 );
      return VOS_STATUS_E_FAILURE;
    }

    /* Update statistics                                                   */
    pControlBlock->sStatsInfo.uNumTxFrames++;

    /* Sanity check to make sure we are not writing beyond allowed size    */
    WLANSSC_ASSERT( (WLANSSC_MAXTXBUFSIZE-sizeof(WLANSSC_TxStartDescriptorType)) >= (u16PayloadSize + u16PacketLength) );

    SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO, "SSC extracting packet: %x from chain : %x of length: %d", 
                 pTxPacket, 
                 pControlBlock->pTxChain , 
                 u16PacketLength));

    /** Extract data from the packet chain into tx buffer beyond start desc
    *   Extract into the tx buffer beyond the last packet.
    */
    sDummy = 0;

    if( VOS_STATUS_SUCCESS != vos_pkt_extract_data( pTxPacket,
                                                    0, 
                                                    &pControlBlock->pTxBuffer[sizeof(WLANSSC_TxStartDescriptorType) + u16PayloadSize],
                                                    &sDummy ) ) 
    {
      WLANSSC_ASSERT( 0 );
      return VOS_STATUS_E_FAILURE;
    }

    /* Important - next DXE header *must* start at 4-byte aligned location */
    u16PayloadSize += ( (u16PacketLength + 0x3) & ~0x3 );

    SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO, "SSC tx payload after extraction and alignment: %d", 
         u16PayloadSize));

    /* Prepare the header for the buffer                                     */
    pTxStartDescriptor = (WLANSSC_TxStartDescriptorType *) &pControlBlock->pTxBuffer[0];
   
    pTxStartDescriptor->controlInfo.sCtrlBits.uframeLength = u16PayloadSize;

    /* Compute the payload size to the nearest block size including SD       */
    u16PayloadSize = ( (u16PayloadSize + sizeof(WLANSSC_TxStartDescriptorType) + WLANSSC_BLOCKSIZE - 1)/WLANSSC_BLOCKSIZE ) * WLANSSC_BLOCKSIZE;
   
    /* Send the packet chain over the bus                                    */
    if( VOS_STATUS_SUCCESS != WLANSSC_SendData( pControlBlock, &pControlBlock->pTxBuffer[0],
        u16PayloadSize ) )
    {
         return WLANSSC_TransmitCompleteNotifyAndReturnStatus( pControlBlock,
                                                               VOS_STATUS_E_FAILURE );
    }

    u16PayloadSize = 0;

    /* Walk the chain to the next packet                                   */
    eStatus = vos_pkt_walk_packet_chain( pTxPacket, &pTxPacket, VOS_FALSE);

    if( (VOS_STATUS_SUCCESS != eStatus) &&
        (VOS_STATUS_E_EMPTY != eStatus) )
    {
      SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "SSC Walking packet chain returned status : %d", 
                   eStatus));
      WLANSSC_ASSERT( 0 );

      return WLANSSC_TransmitCompleteNotifyAndReturnStatus( pControlBlock,
                                                            VOS_STATUS_E_FAILURE );
    }
  }

  /* Invoke the callback right away since it is a sync access              */
  return (WLANSSC_TransmitCompleteNotifyAndReturnStatus( pControlBlock,
                                                        VOS_STATUS_SUCCESS ));

} /* WLANSSC_TransmitSingleInternal() */


#endif


/**
 @brief WLANSSC_Transmit is used to gather data from client and send a
 bus request to send this data to Libra.

 Based on the aggregation feature defined, this calls the relevant internal
 function to transmit the data

 @param Handle: SSC control block to operate on

 @param uMaxAllowedPktSize: Maxmimum data allowed by scheduler to grab
                            from client

 @see 

 @return 
          E_EMPTY indicates there was no packet to send
          SUCCESS indicates that a packet was enqueued
          FAILURE indicates that there was an error

*/
static VOS_STATUS WLANSSC_Transmit
(
  WLANSSC_ControlBlockType *pControlBlock,
  v_U32_t                   uMaxAllowedPktSize
)
{
  v_U32_t     uAdjustedMaxAllowedPktSize;
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /** Adjust the passed in size based on: 
  *   a) Max allowed size by scheduler
  *   b) reduced by the SD that needs to go at the head of a tx aggr frame
  *   c) reduced by worst-case DXE alignment bytes between upper layer pkts
  *
  *   This reduces the data that can come from upper layers and can waste
  *   this extra space on the tx payload; but given that we cannot predict
  *   the number of pkts from upper layer, we need this safety to not go over
  *   the allocated tx buffer
  */
  uAdjustedMaxAllowedPktSize = uMaxAllowedPktSize - 
    sizeof(WLANSSC_TxStartDescriptorType) -
    ( (uMaxAllowedPktSize/WLANSSC_MINPKTSIZE) * WLANSSC_MAXTXDXEALIGNMENT );

  /* Adjust max allowed pkt size to account for start descriptor           */

#ifdef WLANSSC_TX_AGGREGATION_INTERNAL 
  return WLANSSC_TransmitChainInternal( pControlBlock,
                                        uAdjustedMaxAllowedPktSize );
#else
  return WLANSSC_TransmitSingleInternal( pControlBlock,
                                        uAdjustedMaxAllowedPktSize );
#endif

} /* WLANSSC_Transmit() */


/**
 @brief WLANSSC_Receive is used to pull data from the Libra device and 
 push them up the stack.
 If no data is currently available in the rx buffer, data is pulled over the
 bus.
 If a partial frame is available, this is dumped into a tx buffer.

 It is very important to pull complete blocks since the ED is at the end of
 a block.

 When this function returns, pending target data should be 0 (unless we
 are out of memory)

 @param Handle: SSC control block to operate on

 @param uMaxAllowedPktSize: Maxmimum data allowed by scheduler to grab
                            from target

 @see 

 @return 
          E_NOMEM indicates there was an out-of-memory situation
          SUCCESS indicates that receive data has been processed
          FAILURE indicates that there was an error
          E_BUSY indicates that a request is pending
          E_AGAIN indicates that rx needs to yield to tx due to scheduling

*/
static VOS_STATUS WLANSSC_Receive
(
  WLANSSC_ControlBlockType *pControlBlock,
  v_U32_t                   uMaxAllowedPktSize
)
{
  v_U32_t         uDataToRead = 0;

  VOS_STATUS      eStatus          = VOS_STATUS_SUCCESS;
  v_U32_t         uReceiveDataSize = 0;
  v_BOOL_t        bProcessMoreData = VOS_FALSE;
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  WLANSSC_ASSERT( NULL != 
                  pControlBlock->stClientCbacks.pfnRxPacketHandlerCback );

  /* Scheduler should not kick rx in if we cannot guarantee getting ED     */
  WLANSSC_ASSERT( WLANSSC_MAXPKTSIZE <= uMaxAllowedPktSize );

  SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO, "Receive status enter.. pending: %d; currdata: %d; currpos: %d", 
               pControlBlock->sRxBufferInfo.uPendingTargetData,
               pControlBlock->sRxBufferInfo.uCurrentRxDataSize,
               pControlBlock->sRxBufferInfo.uCurrentRxDataPosition ));

  /* If we are suspended, we bail out and wait to be resumed for rx        */
  if( WLANSSC_RXSUSPENDEDMASK & pControlBlock->uSuspendedFlowMask )
  {
    SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO,"SSC suspended: no rx possible"));
    return VOS_STATUS_E_BUSY;
  }

  /* If we have no data to process, read some from chip                    */
  if( 0 == pControlBlock->sRxBufferInfo.uCurrentRxDataSize )
  {
    SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO,"Rx notification obtained via interrupt status reg"));

    uDataToRead = ( (pControlBlock->sRxBufferInfo.uPendingTargetData + WLANSSC_BLOCKSIZE - 1)/WLANSSC_BLOCKSIZE ) * WLANSSC_BLOCKSIZE;

    /* Make sure the data read is at least one pkt but less than rx buf    */
    uDataToRead = VOS_MAX( WLANSSC_MAXPKTSIZE,
                           ( VOS_MIN((uDataToRead),
                                     WLANSSC_MAXRXBUFSIZE) ) );

    /* Receive data over the bus to process                                */
    if( VOS_STATUS_SUCCESS != WLANSSC_ReceiveData( pControlBlock,
                                                   &pControlBlock->pRxBuffer[0],
                                                   uDataToRead,
                                                   WLANSSC_SYNC_BUSACCESS ) )
    {
      WLANSSC_ASSERT( 0 );
      return VOS_STATUS_E_FAILURE;
    }

    /* Since this was sync access we update the control block with rx info */
    pControlBlock->sRxBufferInfo.uCurrentRxDataPosition = 0;
    pControlBlock->sRxBufferInfo.uCurrentRxDataSize = uDataToRead;
  }

  /* While there is data on the target, keep processing!                   */
  do
  {
    bProcessMoreData = VOS_FALSE;

    /* Process the data we already have in our rx buffer                   */
    eStatus = WLANSSC_ProcessRxData(pControlBlock);

    switch( eStatus )
    {
      case VOS_STATUS_E_NOMEM:
        /* Nothing to do for now: wait for callback                        */
        SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO, "Out of memory for rx!"));

        /* Set pending data to get fresh data at least once after resume   */
        if( 0 != pControlBlock->sRxBufferInfo.uPendingTargetData )
        {
          pControlBlock->sRxBufferInfo.uPendingTargetData = WLANSSC_MAXPKTSIZE; 
        }
        return eStatus;

      case VOS_STATUS_E_AGAIN:
        /* Nothing to do for now: wait till tx completes                   */
        SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO, "Rx yielding to tx!"));

        /* Set pending data to get fresh data at least once after resume   */
        if( 0 != pControlBlock->sRxBufferInfo.uPendingTargetData )
        {
          pControlBlock->sRxBufferInfo.uPendingTargetData = WLANSSC_MAXPKTSIZE; 
        }
        return eStatus;

      case VOS_STATUS_E_INVAL:
        SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "Improper Length Packet. Already Handled. Ignore"));
        break;

      case VOS_STATUS_SUCCESS:
        SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO, "All rx data processed successfully"));
        break;

      case VOS_STATUS_E_RESOURCES:
        SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO, "Not enough resources to complete rx buffer"));

        /* Adjust the buffer for a later read to complete the frame        */
        if( pControlBlock->sRxBufferInfo.uPendingTargetData >
            (uMaxAllowedPktSize - pControlBlock->sRxBufferInfo.uCurrentRxDataSize) )
        {          
          /** Move memory over since there is plenty of room at head        
          *   Everything prior to the current position should be done with
          */
          vos_mem_move( &pControlBlock->pRxBuffer[0],
                        &pControlBlock->pRxBuffer[pControlBlock->sRxBufferInfo.uCurrentRxDataPosition],
                        pControlBlock->sRxBufferInfo.uCurrentRxDataSize - pControlBlock->sRxBufferInfo.uCurrentRxDataPosition);
          
          pControlBlock->sRxBufferInfo.uCurrentRxDataSize = pControlBlock->sRxBufferInfo.uCurrentRxDataSize - pControlBlock->sRxBufferInfo.uCurrentRxDataPosition;
          pControlBlock->sRxBufferInfo.uCurrentRxDataPosition = 0;
        }

        break;

      case VOS_STATUS_E_FAILURE:
      default:
        WLANSSC_ASSERT( 0 );
        SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "Unexpected status code: %d", eStatus));
    } /* eStatus */


    /* Based on result of processing, read more, or bail out               */
    if( 0 < pControlBlock->sRxBufferInfo.uPendingTargetData )
    {
      /* Read from current position: there should be enough space          */
      SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO, "Reading more data into rx buffer from position: %d; size: %d", 
                   pControlBlock->sRxBufferInfo.uCurrentRxDataPosition,
                   pControlBlock->sRxBufferInfo.uPendingTargetData));

      /* Take the minimum of max allowed pkt size and pending data         */
      uReceiveDataSize = VOS_MIN( uMaxAllowedPktSize - pControlBlock->sRxBufferInfo.uCurrentRxDataSize, 
                                  pControlBlock->sRxBufferInfo.uPendingTargetData );

      WLANSSC_ASSERT( 0 < uReceiveDataSize );

      pControlBlock->sRxBufferInfo.uPendingTargetData -= uReceiveDataSize;

      /* Always round up to the next block size                              */
      uReceiveDataSize = ( (uReceiveDataSize + WLANSSC_BLOCKSIZE - 1)/WLANSSC_BLOCKSIZE ) * WLANSSC_BLOCKSIZE;

      /** We guarantee elsewhere that uCurrentRxDataSize has enough
      *   tail room to take the pendingtargetdata
      */
      WLANSSC_ASSERT( WLANSSC_MAXRXBUFSIZE >= 
                      (uReceiveDataSize + pControlBlock->sRxBufferInfo.uCurrentRxDataSize) );

      if( VOS_STATUS_SUCCESS != WLANSSC_ReceiveData( pControlBlock,
                                                     &pControlBlock->pRxBuffer[pControlBlock->sRxBufferInfo.uCurrentRxDataSize],
                                                     uReceiveDataSize,
                                                     WLANSSC_SYNC_BUSACCESS ) )
      {
        WLANSSC_ASSERT( 0 );
        return VOS_STATUS_E_FAILURE;
      }

      /* Update the controlblock with the received data info               */
      pControlBlock->sRxBufferInfo.uCurrentRxDataSize += uReceiveDataSize;

      bProcessMoreData = VOS_TRUE;        
    }

  } while( VOS_TRUE == bProcessMoreData );


  /* We now yield to the scheduler to see if anything else needs to run    */

  SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO, "Receive status exit.. pending: %d; currdata: %d; currpos: %d", 
               pControlBlock->sRxBufferInfo.uPendingTargetData,
               pControlBlock->sRxBufferInfo.uCurrentRxDataSize,
               pControlBlock->sRxBufferInfo.uCurrentRxDataPosition ));

  return eStatus;

} /* WLANSSC_Receive() */


/**
 @brief WLANSSC_ProcessRxData is used to process the rx data that has been
 pulled over the bus and stored in the SSC control block

 @param pControlBlock: SSC control block to operate on

 @see WLANSSC_Receive

 @return Result of the function call:
         VOS_STATUS_SUCCESS     - operation succeeded
         VOS_STATUS_E_NOMEM     - memory not available to push packets up;
                                  need to wait for callback from VOS to 
                                  resume (suspend rx)
         VOS_STATUS_E_RESOURCES - rxbuffer does not contain enough data to
                                  complete processing: read pending data
                                  from target
         VOS_STATUS_E_FAILURE   - unexpected error (should not happen!)
         VOS_STATUS_E_AGAIN     - rx needs to yield to tx; process rx later
*/
static VOS_STATUS WLANSSC_ProcessRxData
(
  WLANSSC_ControlBlockType *pControlBlock
)
{
  v_U32_t                          uSDPosition;
  WLANSSC_RxStartDescriptorType   *pRxSD = NULL;
  WLANSSC_RxEndDescriptorType     *pRxED = NULL;
  VOS_STATUS                       eStatus = VOS_STATUS_SUCCESS;
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  WLANSSC_ASSERT( NULL != pControlBlock );

  WLANSSC_ASSERT( 0 < pControlBlock->sRxBufferInfo.uCurrentRxDataSize );

  /** a) Parse each SD and ED and merge with temp rx frame if needed
      b) Retrieve any complete packets and push them up the stack
      c) Store any partial frames in the temp buffer
      d) Store last ED interrupt status in control block
      e) Enable rx_data_avail interrupt again
  */

  WLANSSC_ASSERT( (WLANSSC_MAXRXBUFSIZE - sizeof(WLANSSC_RxStartDescriptorType)) >
                   pControlBlock->sRxBufferInfo.uCurrentRxDataPosition );

  pControlBlock->sSchedulerInfo.uRxProcessedCnt++;

  /* If out of memory, that should cause tx scheduling; otherwise, force   */
  if( WLANSSC_RXYIELDTOTXTHRESHOLD < pControlBlock->sSchedulerInfo.uRxProcessedCnt )
  {
    /* Forces tx to schedule if necessary                                  */
    pControlBlock->sSchedulerInfo.uRxProcessedCnt = 0;
    return VOS_STATUS_E_AGAIN;
  }

  /* Initialize SD to the current location of the Rx buffer                */
  uSDPosition = pControlBlock->sRxBufferInfo.uCurrentRxDataPosition;

  /* Traverse the rx buffer while we can get an SD                         */
  while( (uSDPosition + sizeof(WLANSSC_RxStartDescriptorType)) < 
          pControlBlock->sRxBufferInfo.uCurrentRxDataSize )
  {
    pRxSD = (WLANSSC_RxStartDescriptorType *) &pControlBlock->pRxBuffer[uSDPosition];

    /* Update the SD position every time relative to the previous one      */
    uSDPosition = uSDPosition + sizeof(WLANSSC_RxEndDescriptorType) +
      WLANSSC_GetNextRxEDFromBuffer( (v_U8_t * )pRxSD );

    SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO, "New SD position: %d", uSDPosition));

    WLANSSC_ASSERT( 0 < uSDPosition );

    if( pControlBlock->sRxBufferInfo.uCurrentRxDataSize <
        uSDPosition )
    {
      /* We do not have sufficient bytes to retrieve ED: request more data */
      pControlBlock->sRxBufferInfo.uPendingTargetData = 
        VOS_MAX( uSDPosition - pControlBlock->sRxBufferInfo.uCurrentRxDataSize,
                 pRxSD->u16BMURxWQByteCount );

      /* Get out of the while loop handling more rx frames                 */
      eStatus = VOS_STATUS_E_RESOURCES;
      break;
    }

    /* We have an ED to process                                            */
    pRxED = (WLANSSC_RxEndDescriptorType *) &pControlBlock->pRxBuffer[uSDPosition - sizeof(WLANSSC_RxEndDescriptorType)];

    switch( pRxSD->controlInfo.sCtrlBits.uStartDescriptorCode )
    {
      case WLANSSC_NONEPENDING_RXSDCODE:
        /** Padding frame
            Trash this frame, but update the control block
        */
        SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO, "Padding frame"));

        /* Update statistics                                               */
        pControlBlock->sStatsInfo.uNumRxPadding++;

        pControlBlock->sRxBufferInfo.uPendingTargetData = VOS_MAX( pRxED->controlInfo1.sCtrlBits1.uBMURxWQByteCount,
                                                                   pControlBlock->sRxBufferInfo.uPendingTargetData );
        break;

      case WLANSSC_UNDERFLOWREMAINDER_RXSDCODE:
        /** Remainder - check for temp buffer, append and send up if needed
        */
        SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO, "Remainder frame"));

        /* Update statistics                                                   */
        pControlBlock->sStatsInfo.uNumRxRemainder++;

        WLANSSC_ASSERT( NULL != pControlBlock->pTempRxFrame );

        if ( NULL != pControlBlock->pTempRxFrame )
        {            
           eStatus = WLANSSC_PrepareRxPkt( pControlBlock, 
                                        (v_U8_t *)pRxSD + sizeof(WLANSSC_RxStartDescriptorType),
                                        pRxED->controlInfo0.sCtrlBits0.uActualXferLength,
                                        0 /* indicates a remainder frame */);

           if( VOS_STATUS_SUCCESS != eStatus )
           {
               /* Get out of the switch: should not notify upper layer.       */
               break;
           }
        }          

        if( pRxSD->controlInfo.sCtrlBits.uFrameLength ==
            pRxED->controlInfo0.sCtrlBits0.uActualXferLength )
        {
          /* Complete frame: pass up ONLY if TempRx is NULL. 
           **/
          if ( NULL != pControlBlock->pTempRxFrame ) 
          {
             eStatus = WLANSSC_NotifyRxPkt( pControlBlock );
          }
          pControlBlock->sRxBufferInfo.uPendingTargetData = VOS_MAX( pRxED->controlInfo1.sCtrlBits1.uBMURxWQByteCount,
                                                                     pControlBlock->sRxBufferInfo.uPendingTargetData );

          /* Extra check on ED code to see if Rx fifo might have a pkt     */
          if( (0 == pControlBlock->sRxBufferInfo.uPendingTargetData) &&
              (WLANSSC_NONE_RXEDCODE != pRxED->controlInfo0.sCtrlBits0.uEndDescriptorCode) )
          {
            SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO, "ED code indicates more data! : %d ", 
                         pRxED->controlInfo0.sCtrlBits0.uEndDescriptorCode));
            WLANSSC_ASSERT( WLANSSC_UNDERFLOWERROR_RXEDCODE != 
                            pRxED->controlInfo0.sCtrlBits0.uEndDescriptorCode );

            /* Set pending data to MTU to try to retrieve the packet       */
            pControlBlock->sRxBufferInfo.uPendingTargetData = WLANSSC_MAXPKTSIZE;
          }
        }
        else
        {
          /* More pending on the target: just wait for the next frame      */
          pControlBlock->sRxBufferInfo.uPendingTargetData =
            VOS_MAX( pRxSD->controlInfo.sCtrlBits.uFrameLength - pRxED->controlInfo0.sCtrlBits0.uActualXferLength,
                     pRxED->controlInfo1.sCtrlBits1.uBMURxWQByteCount );
        }

        break;

      case WLANSSC_WATERMARKREACHED_RXSDCODE:
      case WLANSSC_COMPLETEFRAME_RXSDCODE:
        /** Check for partial frame, dump to temp frame if needed
        */
        SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO, "New frame"));

        /* Regardless of whether it was fully received, dump into vos pkt  */
        eStatus = WLANSSC_PrepareRxPkt( pControlBlock, 
                                        (v_U8_t *)pRxSD + sizeof(WLANSSC_RxStartDescriptorType),
                                        pRxED->controlInfo0.sCtrlBits0.uActualXferLength,
                                        pRxSD->controlInfo.sCtrlBits.uFrameLength /* indicates a new frame */);


        if(( VOS_STATUS_SUCCESS != eStatus ) && (VOS_STATUS_E_INVAL != eStatus))
        {
          /* Get out of the switch: should not notify upper layer. For E_INVAL we will continue the Loop          */
          break;
        }

        if( pRxSD->controlInfo.sCtrlBits.uFrameLength ==
            pRxED->controlInfo0.sCtrlBits0.uActualXferLength )
        {
          /* Update statistics                                             */
          pControlBlock->sStatsInfo.uNumRxFrames++;

          /* Complete frame: pass up ONLY if result isnt E_INVAL. For Packets more than MTU size 
           *  VOS will return E_INVAL where in which we need to Drop the Current Buffer and continue
           *  Extracting next Buffer. */
          if (VOS_STATUS_E_INVAL != eStatus)
          {
             eStatus = WLANSSC_NotifyRxPkt( pControlBlock );
          }
          pControlBlock->sRxBufferInfo.uPendingTargetData = VOS_MAX( pRxED->controlInfo1.sCtrlBits1.uBMURxWQByteCount,
                                                                     pControlBlock->sRxBufferInfo.uPendingTargetData );

          /* Extra check on ED code to see if Rx fifo might have a pkt     */
          if( (0 == pControlBlock->sRxBufferInfo.uPendingTargetData) &&
              (WLANSSC_NONE_RXEDCODE != pRxED->controlInfo0.sCtrlBits0.uEndDescriptorCode) )
          {
            SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO, "ED code indicates more data! : %d ", 
                         pRxED->controlInfo0.sCtrlBits0.uEndDescriptorCode));
            WLANSSC_ASSERT( WLANSSC_UNDERFLOWERROR_RXEDCODE != 
                            pRxED->controlInfo0.sCtrlBits0.uEndDescriptorCode );

            /* Set pending data to MTU to try to retrieve the packet       */
            pControlBlock->sRxBufferInfo.uPendingTargetData = WLANSSC_MAXPKTSIZE;
          }
        }
        else
        {
          /* Update statistics                                             */
          pControlBlock->sStatsInfo.uNumRxPartial++;

          /** More pending on target: we have already pushed into a buffer
              so we just wait for remainder
          */
          pControlBlock->sRxBufferInfo.uPendingTargetData =
            VOS_MAX( pRxSD->controlInfo.sCtrlBits.uFrameLength - pRxED->controlInfo0.sCtrlBits0.uActualXferLength,
                     pRxED->controlInfo1.sCtrlBits1.uBMURxWQByteCount );
        }
        break;

      default:
        WLANSSC_ASSERT( 0 );
        SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "Invalid SD code", 
                     pRxSD->controlInfo.sCtrlBits.uStartDescriptorCode));
        eStatus = VOS_STATUS_E_FAILURE;

    } /* pRxSD->sCtrlBits.uStartDescriptorCode */

    /** If any of the above operations in the switch failed, we should not
        update our uCurrentRxDataPosition since we want to resume next time
        from the previous SD! E_INVAL is exception. Ignore and continue Fetch Next
        Frame as previous Frame was of Invalid Length ( > MTU Len)
    */
    if(( VOS_STATUS_SUCCESS != eStatus ) && (VOS_STATUS_E_INVAL != eStatus))
    {
      break;
    }

    /* Need to store the last SD we reached (but didn't complete)          */
    pControlBlock->sRxBufferInfo.uCurrentRxDataPosition = uSDPosition;

  }

  SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO, "SD Position after traversing buffer: %d ", uSDPosition));

  /* If we have reached the end of the buffer, then reset the rx info      */
  if(( VOS_STATUS_SUCCESS == eStatus  ) || (VOS_STATUS_E_INVAL == eStatus))
  {
    WLANSSC_ASSERT( uSDPosition == pControlBlock->sRxBufferInfo.uCurrentRxDataSize );

    SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO, "End of buffer reached"));

    /* Extra protection removed to reduce overhead on data path            */
#ifdef FEATURE_LIBRA_UNSUPPORTED
    /* Zero out our rx buffer to be clean                                  */
    vos_mem_zero( &pControlBlock->pRxBuffer[0], WLANSSC_MAXRXBUFSIZE );
#endif /* FEATURE_LIBRA_UNSUPPORTED */

    pControlBlock->sRxBufferInfo.uCurrentRxDataPosition = 0;
    pControlBlock->sRxBufferInfo.uCurrentRxDataSize = 0;
    /* updating the status to success so that we can enable the Rx interrupt
    again*/
    eStatus = VOS_STATUS_SUCCESS;
  }

  return eStatus;

} /* WLANSSC_ProcessRxData () */


/**
 @brief WLANSSC_ReceiveData is used to receive data over SDIO from the RX
 FIFO on the Libra chip for the given SSC instance

 @param Handle: SSC control block to operate on

 @param pBuffer: pointer to data buffer to receive data into

 @param uDataSize: length of data in bytes to receive (caller allocates mem)

 @see WLANSSC_SendData

 @return Result of the function call
*/
static VOS_STATUS WLANSSC_ReceiveData
(
  WLANSSC_ControlBlockType *pControlBlock,
  v_U8_t                   *pBuffer,
  v_U32_t                   uDataSize,
  WLANSSC_BusAccessType     eBusAccess
)
{
  WLANSAL_Cmd53ReqType   sCmd53Request;
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  WLANSSC_ASSERT( NULL != pControlBlock );

  WLANSSC_ASSERT( NULL != pBuffer );

  vos_mem_zero( &sCmd53Request, sizeof(WLANSAL_Cmd53ReqType) );

  /* RX FIFO                                                               */
  sCmd53Request.address = QWLAN_SIF_RXF_ACC_PTR_REG;

  /* FIFO addresses are all fixed reads                                    */
  sCmd53Request.addressHandle = WLANSAL_ADDRESS_FIXED;

  sCmd53Request.busDirection = WLANSAL_DIRECTION_READ;

    /* If length is less than one block then just use byte mode transfer     */
  sCmd53Request.mode = 
    (uDataSize < WLANSSC_BLOCKSIZE) ? WLANSAL_MODE_BYTE : WLANSAL_MODE_BLOCK;

  sCmd53Request.dataPtr = pBuffer;

  sCmd53Request.dataSize = uDataSize;

  sCmd53Request.function = WLANSSC_SDIOFUNCTION1;

  switch( eBusAccess )
  {
    case WLANSSC_SYNC_BUSACCESS:
      sCmd53Request.callSync = WLANSAL_CALL_SYNC;
      sCmd53Request.busCompleteCB = NULL;
      break;

    case WLANSSC_ASYNC_BUSACCESS:
      sCmd53Request.callSync = WLANSAL_CALL_ASYNC;
      sCmd53Request.busCompleteCB = WLANSSC_RxCompleteCallback;
      break;

    default:
      WLANSSC_ASSERT( 0 );
      return VOS_STATUS_E_FAILURE;
  } /* eBusAccess */

  if( VOS_STATUS_SUCCESS != WLANSAL_Cmd53( pControlBlock->hSALHandle,
                                           &sCmd53Request ) )
  {
    return VOS_STATUS_E_FAILURE;
  }

  /* Update statistics                                                   */
  pControlBlock->sStatsInfo.uNumRxCmd53++;

  SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO, "Receive data ptr: %x for ctrl blk : %x; size: %d", 
               pBuffer,
               pControlBlock,
               uDataSize));

  return VOS_STATUS_SUCCESS;

} /* WLANSSC_ReceiveData() */


/**
 @brief WLANSSC_NotifyRxPkt is used to pass a given buffer to the client via
 callbacks registered.

 This uses VOS buffers to perform the transaction.

 NOTE: this sets the stored controlblock temprxframe to NULL so that the
 next one can be prepared.

 @param Handle: SSC control block to operate on

 @see WLANSSC_PrepareRxPkt

 @return Status of the function call
*/
static VOS_STATUS WLANSSC_NotifyRxPkt
(
  WLANSSC_ControlBlockType *pControlBlock
)
{
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  WLANSSC_ASSERT( NULL != pControlBlock );

  /* Notify the client                                                     */
  WLANSSC_ASSERT( NULL != pControlBlock->stClientCbacks.pfnRxPacketHandlerCback );

  WLANSSC_ASSERT( NULL != pControlBlock->pTempRxFrame );

  pControlBlock->stClientCbacks.pfnRxPacketHandlerCback( pControlBlock->pTempRxFrame,
                                                         pControlBlock->stClientCbacks.UserData );

  /* We no longer own this frame; we prepare for next rx pkt               */
  pControlBlock->pTempRxFrame = NULL;

  return VOS_STATUS_SUCCESS;

} /* WLANSSC_NotifyRxPkt() */


/**
 @brief WLANSSC_PrepareRxPkt is used to prepare a given buffer for passing
 up the stack

 This uses VOS buffers to perform the transaction. The VOS pkt is stored in
 the control block pending notification

 Once upper layers have been notified, the temprxframe should be set to NULL
 As long as this is not NULL, PrepareRxPkt will keep appending to the 
 existing frame/chain

 TODO: once chaining is supported, add that capability here; not needed for
 now.

 @param Handle: SSC control block to operate on

 @param pBuffer: pointer to data buffer to send data

 @param uCurrentDataSize: length of data available in buffer

 @param uCompletePacketSize: expected length of the complete packet
                             (used to create new packet of required size)
                             if 0, indicates remainder for previous fragment

 @see WLANSSC_NotifyRxPkt

 @return Status of the function call
*/
static VOS_STATUS WLANSSC_PrepareRxPkt
(
  WLANSSC_ControlBlockType *pControlBlock,
  v_U8_t                   *pBuffer,
  v_U32_t                   uCurrentDataSize,
  v_U32_t                   uCompletePacketSize
)
{
  /* Stores the size of data in the temp rx frame (to pop out if needed)   */
  v_U16_t      u16TempRxFrameSize = 0;

  /* Differs from uCompletePacketSize for remainder frames                 */
  v_U32_t      uComputedCompletePacketSize = 0;

  /* vos_push_tail() is not supported: this is used as workaround          */
  vos_pkt_t   *pTailFrame = NULL;
  VOS_STATUS   eStatus = VOS_STATUS_SUCCESS;
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  WLANSSC_ASSERT( NULL != pControlBlock );

  WLANSSC_ASSERT( NULL != pBuffer );

  SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO, "Preparing frame") );

  if( 0 < uCompletePacketSize )
  {
    /* This is a complete frame: no need for further computation           */
    uComputedCompletePacketSize = uCompletePacketSize;
  }
  else
  {
    /* We should have a previously stored temp frame                       */
    WLANSSC_ASSERT( NULL != pControlBlock->pTempRxFrame );

    /* We need to retrieve previous packet length (stored in control blk)  */
    if( VOS_STATUS_SUCCESS != vos_pkt_get_packet_length( pControlBlock->pTempRxFrame, 
                                                         &u16TempRxFrameSize ) ) 
    {
      SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR,"Error computing pkt length; default to current data size",
                  uCurrentDataSize));
      u16TempRxFrameSize = 0;
    }

    /* Add the current data size for the remainder to get complete length  */
    uComputedCompletePacketSize = u16TempRxFrameSize + uCurrentDataSize;
  }

  if( NULL == pControlBlock->pMemAvailFrame )
  {
    SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO, "Creating new packet"));

    /* Need to create a new packet for this buffer                         */
    eStatus = vos_pkt_get_packet(&pTailFrame,
                                 VOS_PKT_TYPE_RX_RAW, 
                                 uComputedCompletePacketSize, 
                                 1, /* Number of packets  */
                                 VOS_FALSE, 
                                 WLANSSC_MemoryAvailableCallback,
                                 (v_PVOID_t)pControlBlock );
    /* Return NO_MEM when VOS is out of Resources but Return INVAL for Inavlid Packet Length ( > MTU len) */
    if (eStatus == VOS_STATUS_E_RESOURCES)
    {
      return VOS_STATUS_E_NOMEM;
    }
    else if (eStatus == VOS_STATUS_E_INVAL)
    {
        return eStatus;
    }
  }
  else
  {
    /* We are resuming after being suspended - reuse the MemAvailFrame     */

    SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO, "Using frame from VOS mem avail callback"));

    pTailFrame = pControlBlock->pMemAvailFrame;
    pControlBlock->pMemAvailFrame = NULL;
  }

  WLANSSC_ASSERT( NULL != pTailFrame );

  /* Add the buffer to the packet                                          */
  if( VOS_STATUS_SUCCESS != vos_pkt_push_head( pTailFrame,
                                               pBuffer,
                                               uCurrentDataSize ) )
  {
    WLANSSC_ASSERT( 0 );
    SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "SSC push head failed on pkt: %x; size: %d", 
                 pTailFrame,
                 uCurrentDataSize));
  }

  if( 0 < uCompletePacketSize )
  {
    /* Complete frame: we store in control block                           */
    WLANSSC_ASSERT( NULL == pControlBlock->pTempRxFrame );

    pControlBlock->pTempRxFrame = pTailFrame;
  }
  else
  {
    /* Need to push the original temp frame on top of tailframe            */
    WLANSSC_ASSERT( NULL != pControlBlock->pTempRxFrame );

    if( WLANSSC_MAXPKTSIZE < u16TempRxFrameSize )
    {
      /* Make sure we push less than scratch buffer size                   */
      u16TempRxFrameSize = WLANSSC_MAXPKTSIZE;
    }

    /* Extract the temp frame into scratch buffer                          */
    if( VOS_STATUS_SUCCESS != vos_pkt_pop_head( pControlBlock->pTempRxFrame, 
                                                &pControlBlock->aRxScratchBuffer[0],
                                                u16TempRxFrameSize ) )
    {
      WLANSSC_ASSERT( 0 );
      SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "SSC pop head failed on pkt: %x; size: %d", 
                   pControlBlock->pTempRxFrame,
                   u16TempRxFrameSize));
    }

    /* Free the temp frame                                                 */
    (v_VOID_t) vos_pkt_return_packet( pControlBlock->pTempRxFrame );
    pControlBlock->pTempRxFrame = NULL;

    /* Push the scratch buffer on the head of tail frame                   */
    if( VOS_STATUS_SUCCESS != vos_pkt_push_head( pTailFrame,
                                                 &pControlBlock->aRxScratchBuffer[0],
                                                 u16TempRxFrameSize ) )
    {
      WLANSSC_ASSERT( 0 );
      SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "SSC push head failed on pkt: %x; size: %d", 
                   pTailFrame,
                   u16TempRxFrameSize));
    }

    /* Store this value back in the temp frame                             */
    pControlBlock->pTempRxFrame = pTailFrame;
  }

  return VOS_STATUS_SUCCESS;

} /* WLANSSC_PrepareRxPkt() */


/**
 @brief WLANSSC_HandleInterrupt is used to retrieve the current interrupt
 status of the Libra and process the interrupts accordingly.

 Important note: the interrupt handlers might potentially cause state changes

 @param Handle: SSC control block to operate on

 @see 

 @return Result of the function call

*/
static VOS_STATUS WLANSSC_HandleInterrupt
(
  WLANSSC_ControlBlockType *pControlBlock
)
{
  v_U32_t   uInterruptsEnabled = 0;
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /* Use interrupt snapshot variable to read                               */
  if( VOS_STATUS_SUCCESS != WLANSSC_ReadRegister(pControlBlock,
                                                 QWLAN_SIF_SIF_INT_STATUS_REG,
                                                 &(pControlBlock->uInterruptSnapshot),
                                                 WLANSSC_INT_REGBUFFER) )
  {
    return VOS_STATUS_E_FAILURE;
  }

  /* The best way to see if rx data is enabled is to check the chip       */
  if( VOS_STATUS_SUCCESS != WLANSSC_ReadRegister(pControlBlock,
                                                 QWLAN_SIF_SIF_INT_EN_REG,
                                                 &(uInterruptsEnabled),
                                                 WLANSSC_INT_REGBUFFER) )
  {
    return VOS_STATUS_E_FAILURE;
  }

  /* Interrupt bits are not masked by the enabled bits: we do it on host   */
  pControlBlock->uInterruptSnapshot = ( uInterruptsEnabled &
                                        pControlBlock->uInterruptSnapshot );

  /* Process the interrupt received                                        */
  return WLANSSC_ProcessInterrupt( pControlBlock );  

} /* WLANSSC_HandleInterrupt() */



#ifdef FEATURE_WLAN_UNSUPPORTED
/**
 @brief WLANSSC_GetRxPktInterruptStatus is used to retrieve the interrupt
 status from the current rx buffer.
 This should only be used when a new rx buffer has just been received, so
 as not to provide stale information.

 This will store the new interrupt status observed in the rx buffer within
 the control block.

 Some data must have been read into the rx buffer before calling this 
 function.

 If at least one ED was found, the function returns the last ED in the
 buffer; otherwise, the function indicates how much more data needs to be
 read from the target to retrieve at least one ED

 @param pControlBlock: SSC control block to operate on

 @see WLANSSC_ProcessInterrupt

 @return 0: Interrupt Status Updated Succesfully
         !=0: value indicates bytes still pending to retrieve ED
*/
static v_U32_t WLANSSC_GetRxPktInterruptStatus
(
  WLANSSC_ControlBlockType *pControlBlock
)
{
  v_U32_t                          uSDPosition;
  WLANSSC_RxEndDescriptorType     *pRxED = NULL;
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  WLANSSC_ASSERT( NULL != pControlBlock );

  /* Data must have been read prior to calling this function               */
  WLANSSC_ASSERT( pControlBlock->sRxBufferInfo.uCurrentRxDataSize >
                  pControlBlock->sRxBufferInfo.uCurrentRxDataPosition );

  /* Reset the interrupt snapshot                                          */
  pControlBlock->uInterruptSnapshot = WLANSSC_INVALIDINTERRUPTSNAPSHOT;

  /* Initialize SD to the current location of the Rx buffer                */
  WLANSSC_ASSERT( (WLANSSC_MAXRXBUFSIZE - sizeof(WLANSSC_RxStartDescriptorType)) >
                   pControlBlock->sRxBufferInfo.uCurrentRxDataPosition );

  uSDPosition = pControlBlock->sRxBufferInfo.uCurrentRxDataPosition;

  /* Traverse the rx buffer to find the last ED                            */
  while( ( (uSDPosition + sizeof(WLANSSC_RxStartDescriptorType)) < 
          pControlBlock->sRxBufferInfo.uCurrentRxDataSize) &&
         ( WLANSSC_GetFrameLengthFromSD( ((WLANSSC_RxStartDescriptorType *) &pControlBlock->pRxBuffer[uSDPosition]) ) <= 
          (pControlBlock->sRxBufferInfo.uCurrentRxDataSize - uSDPosition) ) )
  {
    /* Next SD is immediately after the prev ED (relative to previous SD)  */
    uSDPosition = uSDPosition + sizeof(WLANSSC_RxEndDescriptorType) +
      WLANSSC_GetNextRxEDFromBuffer( &pControlBlock->pRxBuffer[uSDPosition] );

    WLANSSC_ASSERT( ( 0 < uSDPosition ) && 
                    ( (pControlBlock->sRxBufferInfo.uCurrentRxDataSize) >= 
                      uSDPosition ) );

    pRxED = (WLANSSC_RxEndDescriptorType *)&pControlBlock->pRxBuffer[uSDPosition - sizeof(WLANSSC_RxEndDescriptorType)];
  }

  /* Now we should have a pointer to the last ED if available              */
  if( NULL == pRxED )
  {
    SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO,"Insufficient rx data for ED %d", pControlBlock->sRxBufferInfo.uCurrentRxDataSize));

    if( (uSDPosition + sizeof(WLANSSC_RxStartDescriptorType)) < pControlBlock->sRxBufferInfo.uCurrentRxDataSize )
    {
      v_U32_t    uUnderFlowBytes;

      /* If we have at least enough to get the SD, compute actual frame length */
      uUnderFlowBytes = 
        (WLANSSC_GetFrameLengthFromSD( ((WLANSSC_RxStartDescriptorType *) &pControlBlock->pRxBuffer[uSDPosition]) )) -
        (pControlBlock->sRxBufferInfo.uCurrentRxDataSize - uSDPosition);
      return uUnderFlowBytes;
    }
    else
    {
      /* If not even SD is available, guess as MTU                         */
      return WLANSSC_MAXPKTSIZE;
    }
  }

  /* ED bits are not masked by the enabled bits: we do it on host          */
  pControlBlock->uInterruptSnapshot = ( pControlBlock->uInterruptEnableMask &
                                        (v_U32_t)( (pRxED->controlInfo1.sCtrlBits1.interruptInfo.uInterruptStatus)
                                                   & WLANSSC_EDINTERRUPTMASK ) );

  SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO, "New interrupt status: %x for Control block: %x", 
               pControlBlock->uInterruptSnapshot,
               pControlBlock));

  return 0;

} /* WLANSSC_GetRxPktInterruptStatus() */

#endif /* FEATURE_WLAN_UNSUPPORTED */


/**
 @brief WLANSSC_GetNextRxEDFromBuffer is used to retrieve the location of
 the next ED from the given Rx buffer.

 The SD is *always* expected to be at the beginning of the buffer!

 It is up to caller to ensure that the expected number of bytes are present
 in the buffer passed

 @param pu8RxBuffer: pointer to byte array containing the rx buffer

 @see WLANSSC_ProcessInterrupt

 @return Location of the ED within the buffer passed
         0 indicates that the location was not found
*/
static v_U32_t WLANSSC_GetNextRxEDFromBuffer
(
  v_U8_t   *pu8RxBuffer
)
{
  v_U32_t                          uPosition = 0;
  WLANSSC_RxStartDescriptorType   *pRxSD = NULL;
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  WLANSSC_ASSERT( NULL != pu8RxBuffer );

  pRxSD = (WLANSSC_RxStartDescriptorType *) pu8RxBuffer;

  if( (WLANSSC_SYNC_SEQ_DWORD_0 == pRxSD->uSyncSeqDword0) &&
      (WLANSSC_SYNC_SEQ_DWORD_1 == pRxSD->uSyncSeqDword1) )
  {
    /* Found a valid SD to operate on                                      */
    if( WLANSSC_NONEPENDING_RXSDCODE == pRxSD->controlInfo.sCtrlBits.uStartDescriptorCode )
    {
      /* This is just padding, so ED is at the end of the block            */
      uPosition = WLANSSC_BLOCKSIZE - sizeof(WLANSSC_RxEndDescriptorType);
    }
    else
    {
      /** This means ED is at the end of the last block containing data     

          We compute number of blocks based on frame length, SD and ED
          and accommodate padding, since ED is at the end of the block
          Finally we subtract the length of the ED from the entire count
          to get the location of the ED start
      */
      uPosition = ( ( ( pRxSD->controlInfo.sCtrlBits.uFrameLength + sizeof(WLANSSC_RxStartDescriptorType) +
                        sizeof(WLANSSC_RxEndDescriptorType) + (WLANSSC_BLOCKSIZE - 1) ) / 
                      WLANSSC_BLOCKSIZE ) * WLANSSC_BLOCKSIZE ) - sizeof(WLANSSC_RxEndDescriptorType);
    }
  }
  else
  {
    /* Should not happen                                                   */
    WLANSSC_ASSERT( 0 );
    SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "Invalid Rx SD"));
    uPosition = 0;
  }

  return uPosition;

} /* WLANSSC_GetNextRxEDFromBuffer() */



#ifdef FEATURE_WLAN_UNSUPPORTED
/**
 @brief WLANSSC_GetFrameLengthFromSD is used to get the length of the frame
 from the SD

 It is up to caller to ensure that the expected number of bytes are present
 in the buffer passed

 @param pRxSD: pointer to Rx Start Descriptor

 @see

 @return Size of the frame described by the SD
*/
static v_U32_t WLANSSC_GetFrameLengthFromSD
(
  WLANSSC_RxStartDescriptorType   *pRxSD
)
{
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  WLANSSC_ASSERT( NULL != pRxSD );

  return ( ( ( pRxSD->controlInfo.sCtrlBits.uFrameLength + sizeof(WLANSSC_RxStartDescriptorType) +
                        sizeof(WLANSSC_RxEndDescriptorType) + (WLANSSC_BLOCKSIZE - 1) ) / 
                      WLANSSC_BLOCKSIZE ) * WLANSSC_BLOCKSIZE );

} /* WLANSSC_GetFrameLengthFromSD() */
#endif /* FEATURE_WLAN_UNSUPPORTED */


/**
 @brief WLANSSC_ProcessInterrupt is used to handle the specific
 interrupts signaled by the SIF hardware block

 This invokes the corresponding interrupt handler for the interrupt
 in a specific order.

 The interrupt status should have been retrieved at this point.

 @param pControlBlock: SSC control block to operate on

 @see WLANSSC_GetRxPktInterruptStatus

 @return Result of the function call
*/
static VOS_STATUS WLANSSC_ProcessInterrupt
(
  WLANSSC_ControlBlockType *pControlBlock
)
{
  WLANSSC_InterruptType   eInterrupt;
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  WLANSSC_ASSERT( NULL != pControlBlock );

  SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO, "Processing interrupt status: %x for Control block: %x", 
               pControlBlock->uInterruptSnapshot,
               pControlBlock));

  WLANSSC_ASSERT( WLANSSC_INVALIDINTERRUPTSNAPSHOT !=
                  pControlBlock->uInterruptSnapshot );

  /* Clear all interrupts from the target                                  */
  if( VOS_STATUS_SUCCESS != WLANSSC_ClearAllInterrupts( pControlBlock ) )
  {
    WLANSSC_ASSERT( 0 );
    return VOS_STATUS_E_FAILURE;
  }

  /* ASIC Interrupt has special treatment: one callback per entire mask    */
  if( pControlBlock->uInterruptSnapshot & pControlBlock->uASICInterruptMask )
  {
    /* Clear ALL ASIC interrupts from the pending mask                     */
    pControlBlock->uInterruptSnapshot &= ~pControlBlock->uASICInterruptMask;

    /* Invoke the ASIC interrupt callback                                  */
    WLANSSC_ASSERT( NULL!= pControlBlock->stClientCbacks.pfnASICInterruptIndicationCback );
    
    pControlBlock->stClientCbacks.pfnASICInterruptIndicationCback( pControlBlock->stClientCbacks.UserData );
  }

  /* Loop through all SSC interrupts and process them one by one           */
  for( eInterrupt = WLANSSC_MIN_INTERRUPT; 
       eInterrupt < WLANSSC_MAX_INTERRUPT;
       eInterrupt++ )
  {
    if( pControlBlock->uInterruptSnapshot & 
        gWLANSSC_InterruptHandlerTable[eInterrupt].uInterruptMask )
    {
      WLANSSC_ASSERT( NULL != gWLANSSC_InterruptHandlerTable[eInterrupt].pfnInterruptHandler );

      /* Execute the interrupt handler if needed                           */
      if( VOS_STATUS_SUCCESS != (gWLANSSC_InterruptHandlerTable[eInterrupt].pfnInterruptHandler)(pControlBlock) )
      {
        WLANSSC_ASSERT( 0 );
      }    
      
      /* Clear the interrupt from the pending mask                         */
      pControlBlock->uInterruptSnapshot &= 
        ~gWLANSSC_InterruptHandlerTable[eInterrupt].uInterruptMask;
    }
  }

  /* Reset the interrupt snapshot                                          */
  pControlBlock->uInterruptSnapshot = WLANSSC_INVALIDINTERRUPTSNAPSHOT;

  return VOS_STATUS_SUCCESS;

} /* WLANSSC_ProcessInterrupt () */


/*---------------------------------------------------------------------------
 * Callbacks with external entities (SAL, VOS)
 * ------------------------------------------------------------------------*/

/**
 @brief WLANSSC_MemoryAvailableCallback is used to be notified that memory
 is available with VOS for further operations

 @param Handle: SSC control block to operate on

 @see

 @return Result of the function call
*/
static VOS_STATUS WLANSSC_MemoryAvailableCallback
(
  vos_pkt_t       *pPacket,
  v_PVOID_t        UserData
)
{
  vos_msg_t                   sMessage;
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO, "SSC memavail handler invoked: serializing to tx thread"));

  vos_mem_zero( &sMessage, sizeof(vos_msg_t) );
  
  sMessage.bodyptr = UserData;
  sMessage.type = WLANSSC_MEMAVAIL_MESSAGE;
  sMessage.bodyval = (v_U32_t) pPacket;
  
  return vos_tx_mq_serialize(VOS_MQ_ID_SSC, &sMessage);
  
} /* WLANSSC_MemoryAvailableCallback() */


/**
 @brief WLANSSC_InterruptHandlerCallback is used to be notified that an
 interrupt was received from the SDIO bus driver.

 @param Handle: SSC control block to operate on

 @see

 @return Result of the function call
*/
static VOS_STATUS WLANSSC_InterruptHandlerCallback
(
  v_PVOID_t   pUnusedBySSC,
  v_PVOID_t   pSSCHandle
)
{
  WLANSSC_ControlBlockType    *pControlBlock;
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO, "SSC interrupt handler invoked"));

  WLANSSC_ASSERT( NULL != pSSCHandle );

  pControlBlock = (WLANSSC_ControlBlockType *) pSSCHandle;

  WLANSSC_ASSERT( VOS_TRUE == WLANSSC_ISCONTEXTVALID( pControlBlock ) );

  /* Lock the rx path before proceeding                                    */
  WLANSSC_LOCKRX( pControlBlock );

  /* Necessary overhead to ensure chip did not get suspended meanwhile        */
  if( VOS_FALSE == pControlBlock->bChipSuspended )
  {
    /* Update statistics                                                     */
    pControlBlock->sStatsInfo.uNumInterrupts++;

    /* Clear the interrupt snapshot in preparation for reading a new one     */
    pControlBlock->uInterruptSnapshot = WLANSSC_INVALIDINTERRUPTSNAPSHOT;

    if( VOS_STATUS_SUCCESS != WLANSSC_HandleInterrupt( pControlBlock ) )
    {
      /* Should never happen!                                                */
      WLANSSC_ASSERT( 0 );
      SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR,"Interrupt handling failed!"));
    }
  }

  /* Unlock before leaving                                                 */
  WLANSSC_UNLOCKRX( pControlBlock );

  return VOS_STATUS_SUCCESS;

} /* WLANSSC_InterruptHandlerCallback() */


/**
 @brief WLANSSC_TxCompleteCallback is used to be notified that a tx request
 has been serviced by the SDIO bus driver

 Note: currently we restrict this from enqueuing any blocking calls in this
 context

 @param Handle: SSC control block to operate on

 @see

 @return Result of the function call
*/
static VOS_STATUS WLANSSC_TxCompleteCallback
(
  v_PVOID_t   pUnusedBySSC,
  VOS_STATUS  RequestStatus,
  v_PVOID_t   pSSCHandle
)
{
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  WLANSSC_ASSERT( 0 );  /* Has been disabled for now since we do sync      */

  return VOS_STATUS_E_NOSUPPORT;

} /* WLANSSC_TxCompleteCallback() */


/**
 @brief WLANSSC_RxCompleteCallback is used to be notified that a receive
 request has been serviced by the SDIO bus driver.

 Note: we currently restrict the rx complete handler from making any blocking
 calls in this context.

 @param Handle: SSC control block to operate on

 @see

 @return Result of the function call
*/
static VOS_STATUS WLANSSC_RxCompleteCallback
(
  v_PVOID_t   pUnusedBySSC,
  VOS_STATUS  RequestStatus,
  v_PVOID_t   pSSCHandle
)
{
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  WLANSSC_ASSERT( 0 );  /* Has been disabled for now since we do sync      */

  return VOS_STATUS_E_NOSUPPORT;

} /* WLANSSC_RxCompleteCallback() */


/*---------------------------------------------------------------------------
 * State machine event handlers
 * ------------------------------------------------------------------------*/

/**
 @brief WLANSSC_OpenEventHandler is used to handle the Open event

 @param Handle: SSC control block to operate on

 @see

 @return Result of the function call
*/
static VOS_STATUS WLANSSC_OpenEventHandler
(
  WLANSSC_ControlBlockType *pControlBlock
)
{

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /* Change state                                                          */
  WLANSSC_TransitionState( pControlBlock,
                           WLANSSC_OPEN_STATE );

  return VOS_STATUS_SUCCESS;

} /* WLANSSC_OpenEventHandler() */


/**
 @brief WLANSSC_FatalErrorEventHandler is used to handle any fatal error
 from which the SSC cannot recover

 @param Handle: SSC control block to operate on

 @see

 @return Result of the function call
*/
static VOS_STATUS WLANSSC_FatalErrorEventHandler
(
  WLANSSC_ControlBlockType *pControlBlock
)
{

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /* Indicate to client if required                                        */
  if( NULL != pControlBlock->stClientCbacks.pfnFatalErrorIndicationCback )
  {
    pControlBlock->stClientCbacks.pfnFatalErrorIndicationCback(WLANSSC_UNKNOWN_REASON,
                                                               pControlBlock->stClientCbacks.UserData);
  }

  SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "Fatal error interrupt received!"));

  /* Change back to CLOSED_STATE and wait for a reset                      */
  WLANSSC_TransitionState( pControlBlock, 
                           WLANSSC_CLOSED_STATE );

  return VOS_STATUS_SUCCESS;

} /* WLANSSC_FatalErrorEventHandler() */


/**
 @brief WLANSSC_ResetEventHandler is used to reset the SSC and prepare to
 be started.
 SSC must already be Open at the time of Reset.

 @param Handle: SSC control block to operate on

 @see

 @return Result of the function call
*/
static VOS_STATUS WLANSSC_ResetEventHandler
(
  WLANSSC_ControlBlockType *pControlBlock
)
{

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /* This event is currently not expected                                  */
  WLANSSC_ASSERT( 0 );

  /* Change back to CLOSED_STATE and wait for a reset                      */
  WLANSSC_TransitionState( pControlBlock, 
                           WLANSSC_OPEN_STATE );

  return VOS_STATUS_SUCCESS;

} /* WLANSSC_ResetEventHandler() */


/**
 @brief WLANSSC_CloseEventHandler is used to handle the Close event

 @param Handle: SSC control block to operate on

 @see

 @return Result of the function call
*/
static VOS_STATUS WLANSSC_CloseEventHandler
(
  WLANSSC_ControlBlockType *pControlBlock
)
{

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /* Change state                                                          */
  WLANSSC_TransitionState( pControlBlock,
                           WLANSSC_CLOSED_STATE );

  return VOS_STATUS_SUCCESS;

} /* WLANSSC_CloseEventHandler() */


/**
 @brief WLANSSC_StartEventHandler is used to handle the Start event
 This registers with the SAL and initializes the Libra device as well as
 enables the relevant interrupts.

 @param Handle: SSC control block to operate on

 @see

 @return Result of the function call
*/
static VOS_STATUS WLANSSC_StartEventHandler
(
  WLANSSC_ControlBlockType *pControlBlock
)
{
  WLANSAL_SscRegType   sSSCRegistration;
  WLANSAL_CardInfoType sCardInfo;
  v_U32_t              uRegValue;
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /* Register callbacks, etc with SAL                                      */

  /* Bus complete callback will be registered with each request            */
  sSSCRegistration.busCompleteCB = NULL;

  sSSCRegistration.interruptCB = WLANSSC_InterruptHandlerCallback;

  /* This user data comes back with each request complete!!                */
  sSSCRegistration.sscUsrData = (v_PVOID_t) pControlBlock;

  if( VOS_STATUS_SUCCESS != WLANSAL_RegSscCBFunctions( pControlBlock->hSALHandle,
                                                       &sSSCRegistration ) )
  {
    return VOS_STATUS_E_FAILURE;
  }

  /* Get the current card information                                      */
  if( VOS_STATUS_SUCCESS != WLANSAL_CardInfoQuery( pControlBlock->hSALHandle,
                                                   &sCardInfo ) )
  {
    return VOS_STATUS_E_FAILURE;
  }

  /* Configure the card in 4-bit mode if needed                            */

  /* Set block size if needed                                              */
  if( WLANSSC_BLOCKSIZE != sCardInfo.blockSize )
  {
    SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO, "Updating blocksize from %d to %d", 
                 sCardInfo.blockSize, 
                 WLANSSC_BLOCKSIZE));
    sCardInfo.blockSize = WLANSSC_BLOCKSIZE;
  }

  /* Do we need to set the clock here?                                     */
  if( WLANSSC_SDIOCLOCKRATE != sCardInfo.clockRate )
  {
    SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO, "Updating clock rate from %d to %d", 
                 sCardInfo.clockRate, 
                 WLANSSC_SDIOCLOCKRATE));
    sCardInfo.clockRate = WLANSSC_SDIOCLOCKRATE;
  }

  /* Do we need to configure the clock edge here?                          */

  /* Set everything back                                                   */
  if( VOS_STATUS_SUCCESS != WLANSAL_CardInfoUpdate( pControlBlock->hSALHandle,
                                                    &sCardInfo ) )
  {
    return VOS_STATUS_E_FAILURE;
  }

  /* Enable function 1 if needed                                           */

  /* Reset the chip and start the configuration                            */
  if( VOS_STATUS_SUCCESS != WLANSSC_ResetChip( pControlBlock ) )
  {
    return VOS_STATUS_E_FAILURE;
  }
  
  /* TX endianness                                                         */
  uRegValue = QWLAN_SIF_TXF_MGNT_CONFIG_TX_DATA_ENDIAN_CFG_MASK;

  if( VOS_STATUS_SUCCESS != WLANSSC_WriteRegister(pControlBlock,
                                                  QWLAN_SIF_TXF_MGNT_CONFIG_REG,
                                                  &uRegValue,
                                                  WLANSSC_TX_REGBUFFER) )
  {
    return VOS_STATUS_E_FAILURE;
  }

  /* TX timeout                                                            */
  uRegValue = WLANSSC_TXFIFOFULLDURATIONTIMEOUT * WLANSSC_APPDOMAINCLOCKFREQ;

  if( VOS_STATUS_SUCCESS != WLANSSC_WriteRegister(pControlBlock,
                                                  QWLAN_SIF_TXF_FULL_TIMEOUT_CONFIG_REG,
                                                  &uRegValue,
                                                  WLANSSC_TX_REGBUFFER) )
  {
    return VOS_STATUS_E_FAILURE;
  }

  /* Rx frm fill threshold                                                 */
  uRegValue = WLANSSC_BLOCKSIZE;

  if( VOS_STATUS_SUCCESS != WLANSSC_WriteRegister(pControlBlock,
                                                  QWLAN_SIF_RX_FRM_FILL_THRESHOLD_REG,
                                                  &uRegValue,
                                                  WLANSSC_TX_REGBUFFER) )
  {
    return VOS_STATUS_E_FAILURE;
  }

  /* Rx wm thresholds                                                      */
  uRegValue = WLANSSC_BLOCKSIZE/2;

  if( VOS_STATUS_SUCCESS != WLANSSC_WriteRegister(pControlBlock,
                                                  QWLAN_SIF_RXF_WATERMARK_REG,
                                                  &uRegValue,
                                                  WLANSSC_TX_REGBUFFER) )
  {
    return VOS_STATUS_E_FAILURE;
  }

  /* Rx endianness and enabling advance recovery + blk xfer hold back      */
  uRegValue = QWLAN_SIF_RXF_MGNT_CONFIG_ADVANCED_RECOVERY_MASK |
    QWLAN_SIF_RXF_MGNT_CONFIG_RX_DATA_ENDIAN_CFG_MASK |
    QWLAN_SIF_RXF_MGNT_CONFIG_RX_BLK_XFER_HOLD_BACK_MASK;

  if( VOS_STATUS_SUCCESS != WLANSSC_WriteRegister(pControlBlock,
                                                  QWLAN_SIF_RXF_MGNT_CONFIG_REG,
                                                  &uRegValue,
                                                  WLANSSC_TX_REGBUFFER) )
  {
    return VOS_STATUS_E_FAILURE;
  }

  /* Hold back timer per SIF guide needs to be set to 0x3FFFFFF            */
  uRegValue = QWLAN_SIF_RX_FIFO_DATA_FILL_TIMEOUT_PERIOD_RXF_DATA_FILL_TIMEOUT_PERIOD_MASK;

  if( VOS_STATUS_SUCCESS != WLANSSC_WriteRegister(pControlBlock,
                                                  QWLAN_SIF_RX_FIFO_DATA_FILL_TIMEOUT_PERIOD_REG,
                                                  &uRegValue,
                                                  WLANSSC_TX_REGBUFFER) )
  {
    return VOS_STATUS_E_FAILURE;
  }

  /* Rx event generation                                                   */
  uRegValue = QWLAN_SIF_RX_INT_EVENT_CONFIG_RX_FIFO_WATERMARK_DATA_AVAIL_MASK |
    QWLAN_SIF_RX_INT_EVENT_CONFIG_REMAINDER_DATA_AVAIL_MASK |
    QWLAN_SIF_RX_INT_EVENT_CONFIG_COMPLETE_RX_FRAME_DATA_AVAIL_MASK |
    QWLAN_SIF_RX_INT_EVENT_CONFIG_RX_FIFO_EMPTY_INT_CLR_ON_TRANSFER_INACTIVE_MASK |
    QWLAN_SIF_RX_INT_EVENT_CONFIG_RX_FIFO_EMPTY_INT_CLR_ON_IO_ABORT_MASK;

  if( VOS_STATUS_SUCCESS != WLANSSC_WriteRegister(pControlBlock,
                                                  QWLAN_SIF_RX_INT_EVENT_CONFIG_REG,
                                                  &uRegValue,
                                                  WLANSSC_TX_REGBUFFER) )
  {
    return VOS_STATUS_E_FAILURE;
  }

  /* Should we write to SIF BMU cmd? BAL should ideally deal with this     */

  //The following power save requirment is only for RF boards. They are don't care for FPGA and baseband EVB  
  /* Read the current value                                   */
  if( VOS_STATUS_SUCCESS != WLANSSC_ReadRegisterFuncZero(pControlBlock,
                                                         QWLAN_SIF_BAR4_WLAN_PWR_SAVE_CONTROL_REG_REG,
                                                         (v_U8_t*)&uRegValue,
                                                         WLANSSC_TX_REGBUFFER) )
  {
    SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "Error reading QWLAN_SIF_BAR4_WLAN_PWR_SAVE_CONTROL_REG_REG"));
    return VOS_STATUS_E_FAILURE;
  }
  
  uRegValue |= QWLAN_SIF_BAR4_WLAN_PWR_SAVE_CONTROL_REG_TRSW_SUPPLY_CTRL_1_MASK;
  
  if( VOS_STATUS_SUCCESS != WLANSSC_WriteRegisterFuncZero(pControlBlock,
                                                          QWLAN_SIF_BAR4_WLAN_PWR_SAVE_CONTROL_REG_REG,
                                                          (v_U8_t*)&uRegValue,
                                                          WLANSSC_TX_REGBUFFER) )
  {
    SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "Error writing register QWLAN_SIF_BAR4_WLAN_PWR_SAVE_CONTROL_REG_REG"));
    return VOS_STATUS_E_FAILURE;
  }
  
  /* Read the current value                                   */
  if( VOS_STATUS_SUCCESS != WLANSSC_ReadRegisterFuncZero(pControlBlock,
                                                         QWLAN_SIF_BAR4_WLAN_CONTROL_REG_REG,
                                                         (v_U8_t*)&uRegValue,
                                                         WLANSSC_TX_REGBUFFER) )
  {
    SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "Error reading QWLAN_SIF_BAR4_WLAN_CONTROL_REG_REG"));
    return VOS_STATUS_E_FAILURE;
  }
  
  uRegValue &= ~QWLAN_SIF_BAR4_WLAN_CONTROL_REG_TRSW_SUPPLY_CTRL_0_MASK;
  uRegValue &= ~(QWLAN_SIF_BAR4_WLAN_CONTROL_REG_SDIOC_CMD_ACTIVE_CHECK_DISABLE_MASK);	
  
  if( VOS_STATUS_SUCCESS != WLANSSC_WriteRegisterFuncZero(pControlBlock,
                                                          QWLAN_SIF_BAR4_WLAN_CONTROL_REG_REG,
                                                          (v_U8_t*)&uRegValue,
                                                          WLANSSC_TX_REGBUFFER) )
  {
    SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "Error writing register QWLAN_SIF_BAR4_WLAN_CONTROL_REG_REG"));
    return VOS_STATUS_E_FAILURE;
  }

  /* Store the main interrupts for later usage                                */
  pControlBlock->uInterruptEnableMask = QWLAN_SIF_SIF_INT_EN_RX_UNDERFLOW_FRM_ERR_INTR_EN_MASK |
    QWLAN_SIF_SIF_INT_EN_TX_FRM_CRC_ERR_INTR_EN_MASK |
    QWLAN_SIF_SIF_INT_EN_TX_FRM_DXE_ERR_INTR_EN_MASK |
    QWLAN_SIF_SIF_INT_EN_TX_FIFO_FULL_TIMEOUT_INTR_EN_MASK |
    QWLAN_SIF_SIF_INT_EN_TX_PKT_OUT_OF_SYNC_INTR_EN_MASK |
    QWLAN_SIF_SIF_INT_EN_TX_CMD53_XACTION_LT_PKT_LEN_ERR_INTR_EN_MASK |
    QWLAN_SIF_SIF_INT_STATUS_RX_PKT_XFER_UNDERFLOW_INTR_MASK;

  /* Enable all the basic interrupts at this point on chip and with SAL    */
  if( VOS_STATUS_SUCCESS != WLANSSC_EnableInterrupt( pControlBlock,
                                                     QWLAN_SIF_SIF_INT_EN_RX_FIFO_DATA_AVAIL_INTR_EN_MASK |
                                                     QWLAN_SIF_SIF_INT_EN_RX_UNDERFLOW_FRM_ERR_INTR_EN_MASK |
                                                     QWLAN_SIF_SIF_INT_EN_TX_FRM_CRC_ERR_INTR_EN_MASK |
                                                     QWLAN_SIF_SIF_INT_EN_TX_FRM_DXE_ERR_INTR_EN_MASK |
                                                     QWLAN_SIF_SIF_INT_EN_TX_FIFO_FULL_TIMEOUT_INTR_EN_MASK |
                                                     QWLAN_SIF_SIF_INT_EN_TX_PKT_OUT_OF_SYNC_INTR_EN_MASK |
                                                     QWLAN_SIF_SIF_INT_EN_TX_CMD53_XACTION_LT_PKT_LEN_ERR_INTR_EN_MASK |
                                                     QWLAN_SIF_SIF_INT_STATUS_RX_PKT_XFER_UNDERFLOW_INTR_MASK ) )
  {
    SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "Error enabling interrupts"));
    return VOS_STATUS_E_FAILURE;
  }


  /* Do we need to enable global interrupt or any other Libra interrupt?   */

  /* Change state                                                          */
  WLANSSC_TransitionState( pControlBlock,
                           WLANSSC_READY_STATE );

  return VOS_STATUS_SUCCESS;

} /* WLANSSC_StartEventHandler() */


/**
 @brief WLANSSC_StopEventHandler is used to handle the Stop event
 This deregisters with the SAL and disables any registers, etc on Libra.

 @param Handle: SSC control block to operate on

 @see

 @return Result of the function call
*/
static VOS_STATUS WLANSSC_StopEventHandler
(
  WLANSSC_ControlBlockType *pControlBlock
)
{
  WLANSAL_SscRegType   sSSCRegistration;
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /* Disable all the basic interrupts at this point on chip and with SAL   */
  if( VOS_STATUS_SUCCESS != WLANSSC_DisableInterrupt( pControlBlock,
                                                      QWLAN_SIF_SIF_INT_EN_RX_FIFO_DATA_AVAIL_INTR_EN_MASK |
                                                      QWLAN_SIF_SIF_INT_EN_RX_UNDERFLOW_FRM_ERR_INTR_EN_MASK |
                                                      QWLAN_SIF_SIF_INT_EN_TX_FRM_CRC_ERR_INTR_EN_MASK |
                                                      QWLAN_SIF_SIF_INT_EN_TX_FRM_DXE_ERR_INTR_EN_MASK |
                                                      QWLAN_SIF_SIF_INT_EN_TX_FIFO_FULL_TIMEOUT_INTR_EN_MASK |
                                                      QWLAN_SIF_SIF_INT_EN_TX_PKT_OUT_OF_SYNC_INTR_EN_MASK |
                                                      QWLAN_SIF_SIF_INT_EN_TX_CMD53_XACTION_LT_PKT_LEN_ERR_INTR_EN_MASK |
                                                      QWLAN_SIF_SIF_INT_STATUS_RX_PKT_XFER_UNDERFLOW_INTR_MASK ) )
  {
    SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "Error disabling interrupts"));
    return VOS_STATUS_E_FAILURE;
  }

  /* Reset the permanent interrupt mask                                       */
  pControlBlock->uInterruptEnableMask = 0;

  /* Do we need to disable global interrupt or any other Libra interrupt?  */

  /* Disable function 0 if needed                                          */

  /* It is expected that chip power will be cut off - nothing more for now */

  /* Deregister callbacks, etc with SAL: is this okay?                     */
  sSSCRegistration.busCompleteCB = NULL;
  sSSCRegistration.interruptCB = NULL;
  sSSCRegistration.sscUsrData = NULL;

  if( VOS_STATUS_SUCCESS != WLANSAL_RegSscCBFunctions( pControlBlock->hSALHandle,
                                                       &sSSCRegistration ) )
  {
    return VOS_STATUS_E_FAILURE;
  }

  /* Change state                                                          */
  WLANSSC_TransitionState( pControlBlock,
                           WLANSSC_OPEN_STATE );

  return VOS_STATUS_SUCCESS;

} /* WLANSSC_StopEventHandler() */


/**
 @brief WLANSSC_R_CloseEventHandler is used to handle the Close event in
 Ready state

 @param Handle: SSC control block to operate on

 @see

 @return Result of the function call
*/
static VOS_STATUS WLANSSC_R_CloseEventHandler
(
  WLANSSC_ControlBlockType *pControlBlock
)
{

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /* Invoke the Stop handler first since we are in ready state             */
  if( VOS_STATUS_SUCCESS != WLANSSC_ExecuteEvent( pControlBlock, 
                                                  WLANSSC_STOP_EVENT ) )
  {
    /* Should never happen!                                                */
    WLANSSC_ASSERT( 0 );

    return VOS_STATUS_E_FAILURE;
  }

  /* Then invoke the Close handler                                         */
  if( VOS_STATUS_SUCCESS != WLANSSC_ExecuteEvent( pControlBlock, 
                                                  WLANSSC_CLOSE_EVENT ) )
  {
    /* Should never happen!                                                */
    WLANSSC_ASSERT( 0 );

    return VOS_STATUS_E_FAILURE;
  }

  /* We should be in CLOSED state now                                      */
  WLANSSC_ASSERT( WLANSSC_CLOSED_STATE == pControlBlock->eState );

  return VOS_STATUS_SUCCESS;

} /* WLANSSC_R_CloseEventHandler() */


/**
 @brief WLANSSC_R_SuspendEventHandler is used to handle the Suspend event in
 Ready state

 @param Handle: SSC control block to operate on

 @see

 @return Result of the function call
*/
static VOS_STATUS WLANSSC_R_SuspendEventHandler
(
  WLANSSC_ControlBlockType *pControlBlock
)
{

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /* Since we have no pending request, we honor the request right away     */

  if( WLANSSC_ALLSUSPENDEDMASK == pControlBlock->uSuspendedFlowMask )
  {
    /* If all flows are disabled, turn off everything and change state     */

    if( VOS_STATUS_SUCCESS != WLANSSC_DisableTx( pControlBlock ) )
    {
      /* Should never happen!                                              */
      WLANSSC_ASSERT( 0 );
      return VOS_STATUS_E_FAILURE;
    }
    
    if( VOS_STATUS_SUCCESS != WLANSSC_DisableRx( pControlBlock ) )
    {
      /* Should never happen!                                              */
      WLANSSC_ASSERT( 0 );      
      return VOS_STATUS_E_FAILURE;
    }

    /* Change state                                                        */
    WLANSSC_TransitionState( pControlBlock,
                             WLANSSC_SUSPENDED_STATE );
  } 
  else if( WLANSSC_RXSUSPENDEDMASK & pControlBlock->uSuspendedFlowMask )
  {
    /* No need to change state - we can still service tx operations        */
    if( VOS_STATUS_SUCCESS != WLANSSC_DisableRx( pControlBlock ) )
    {
      /* Should never happen!                                              */
      WLANSSC_ASSERT( 0 );

      return VOS_STATUS_E_FAILURE;
    }
  } 
  else if( WLANSSC_TXSUSPENDEDMASK & pControlBlock->uSuspendedFlowMask )
  {
    /* No need to change state - we can still service rx operations        */
    if( VOS_STATUS_SUCCESS != WLANSSC_DisableTx( pControlBlock ) )
    {
      /* Should never happen!                                              */
      WLANSSC_ASSERT( 0 );
      return VOS_STATUS_E_FAILURE;
    }
  }
  else
  {
    /* Why is this event being triggered?                                  */
    WLANSSC_ASSERT( 0 );
    return VOS_STATUS_E_FAILURE;
  }

  return VOS_STATUS_SUCCESS;

} /* WLANSSC_R_SuspendEventHandler() */


/**
 @brief WLANSSC_R_ResumeEventHandler is used to handle the Resume event in
 Ready state

 @param Handle: SSC control block to operate on

 @see

 @return Result of the function call
*/
static VOS_STATUS WLANSSC_R_ResumeEventHandler
(
  WLANSSC_ControlBlockType *pControlBlock
)
{

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /** Since we are in ready state, only one flow needed to be resumed, which
      implies the mask should now be 0 (since in ready state only one can
      be pending any way)
  */
  WLANSSC_ASSERT( 0 == pControlBlock->uSuspendedFlowMask );

  return VOS_STATUS_SUCCESS;

} /* WLANSSC_R_ResumeEventHandler() */


/**
 @brief WLANSSC_R_TransmitEventHandler is used to handle the Transmit event 
 in Ready state

 Essentially this is triggered when the scheduler decides to trigger the
 tx action.

 @param Handle: SSC control block to operate on

 @see

 @return Result of the function call
*/
static VOS_STATUS WLANSSC_R_TransmitEventHandler
(
  WLANSSC_ControlBlockType *pControlBlock
)
{
  VOS_STATUS   eStatus;
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /* If suspended we bail out: a later resume on tx should start tx again  */
  if( WLANSSC_TXSUSPENDEDMASK & pControlBlock->uSuspendedFlowMask )
  {
    SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "Tx triggered when suspended!"));

    /* Return success just to be nice                                      */
    return VOS_STATUS_SUCCESS;
  }

  /* Since we have no pending request, we honor the request right away     */

  //TODO: replace max buf size with a query to scheduler for allowed size
  eStatus = WLANSSC_Transmit( pControlBlock,
                              WLANSSC_MAXTXBUFSIZE );

  if( VOS_STATUS_E_EMPTY == eStatus )
  {
    SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "Tx triggered without pending packets"));
  }
  else if( VOS_STATUS_SUCCESS != eStatus )
  {
    WLANSSC_ASSERT( 0 );
    SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "Tx Request failed!"));
    return VOS_STATUS_E_FAILURE;
  }

  return VOS_STATUS_SUCCESS;

} /* WLANSSC_R_TransmitEventHandler() */


/**
 @brief WLANSSC_R_InterruptEventHandler is used to handle the interrupt event 
 in Ready state

 Essentially this is triggered when the scheduler decides to trigger the
 interrupt action.

 It is important to note that currently we will be reading interrupt status
 via data reception (piggyback status on rx ED).

 @param Handle: SSC control block to operate on

 @see

 @return Result of the function call
*/
static VOS_STATUS WLANSSC_R_InterruptEventHandler
(
  WLANSSC_ControlBlockType *pControlBlock
)
{
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /* Since we have no pending request, we honor the request right away     */

  /* Interrupts are handled directly via interrupt callback now            */
  WLANSSC_ASSERT( 0 );

  return VOS_STATUS_E_NOSUPPORT;

} /* WLANSSC_R_InterruptEventHandler() */


/**
 @brief WLANSSC_R_ReceiveEventHandler is used to handle the Receive event 
 in Ready state

 Essentially this is triggered when the scheduler decides to trigger the
 rx action.

 @param Handle: SSC control block to operate on

 @see

 @return Result of the function call
*/
static VOS_STATUS WLANSSC_R_ReceiveEventHandler
(
  WLANSSC_ControlBlockType *pControlBlock
)
{
  VOS_STATUS   eStatus;
  vos_msg_t    sMessage;
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /* Since we have no pending request, we honor the request right away     */

  /* Process rx data (if needed)                                           */
  eStatus = WLANSSC_Receive( pControlBlock,
                             WLANSSC_MAXRXBUFSIZE );

  switch( eStatus )
  {
    case VOS_STATUS_SUCCESS:
      /* Enable rx again since all data has been processed                 */
      WLANSSC_EnableRx( pControlBlock );
      break;

    case VOS_STATUS_E_BUSY:
      /* Nothing to do for now: SSC_Resume should cause the resume         */
      break;

    case VOS_STATUS_E_NOMEM:
      /* Nothing to do for now: VOSS callback should cause the resume      */
      break;

    case VOS_STATUS_E_AGAIN:
      /* Serialize message into tx (end of queue) to allow others to run   */
      vos_mem_zero( &sMessage, sizeof(vos_msg_t) );
  
      sMessage.bodyptr = (v_PVOID_t)pControlBlock;
      sMessage.type = WLANSSC_RXPENDING_MESSAGE;
      sMessage.bodyval = 0;
  
      if( VOS_STATUS_SUCCESS != vos_tx_mq_serialize(VOS_MQ_ID_SSC, &sMessage) )
      {
        WLANSSC_ASSERT(0);
        SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR,"Serialize into tx failed!"));
      }
      break;

    default:
      /* Ignore */
      SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR,"Receive event status not handled"));
  } /* eStatus */

  return VOS_STATUS_SUCCESS;

} /* WLANSSC_R_ReceiveEventHandler() */


/**
 @brief WLANSSC_SD_CloseEventHandler is used to handle the close event 
 in Suspended state

 @param Handle: SSC control block to operate on

 @see

 @return Result of the function call
*/
static VOS_STATUS WLANSSC_SD_CloseEventHandler
(
  WLANSSC_ControlBlockType *pControlBlock
)
{
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /* Invoke the Stop handler first since we are in suspended state         */
  if( VOS_STATUS_SUCCESS != WLANSSC_ExecuteEvent( pControlBlock, 
                                                  WLANSSC_STOP_EVENT ) )
  {
    /* Should never happen!                                                */
    WLANSSC_ASSERT( 0 );

    return VOS_STATUS_E_FAILURE;
  }

  /* Then invoke the Close handler                                         */
  if( VOS_STATUS_SUCCESS != WLANSSC_ExecuteEvent( pControlBlock, 
                                                  WLANSSC_CLOSE_EVENT ) )
  {
    /* Should never happen!                                                */
    WLANSSC_ASSERT( 0 );

    return VOS_STATUS_E_FAILURE;
  }

  /* We should be in CLOSED state now                                      */
  WLANSSC_ASSERT( WLANSSC_CLOSED_STATE == pControlBlock->eState );

  return VOS_STATUS_SUCCESS;

} /* WLANSSC_SD_CloseEventHandler() */


/**
 @brief WLANSSC_SD_StopEventHandler is used to handle the Stop event in 
 suspended state.
 This deregisters with the SAL and disables any registers, etc on Libra.

 @param Handle: SSC control block to operate on

 @see

 @return Result of the function call
*/
static VOS_STATUS WLANSSC_SD_StopEventHandler
(
  WLANSSC_ControlBlockType *pControlBlock
)
{
  WLANSAL_SscRegType   sSSCRegistration;
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /* Disable all the basic interrupts at this point on chip and with SAL   */
  if( VOS_STATUS_SUCCESS != WLANSSC_DisableInterrupt( pControlBlock,
                                                      QWLAN_SIF_SIF_INT_EN_RX_FIFO_DATA_AVAIL_INTR_EN_MASK |
                                                      QWLAN_SIF_SIF_INT_EN_RX_UNDERFLOW_FRM_ERR_INTR_EN_MASK |
                                                      QWLAN_SIF_SIF_INT_EN_TX_FRM_CRC_ERR_INTR_EN_MASK |
                                                      QWLAN_SIF_SIF_INT_EN_TX_FRM_DXE_ERR_INTR_EN_MASK |
                                                      QWLAN_SIF_SIF_INT_EN_TX_FIFO_FULL_TIMEOUT_INTR_EN_MASK |
                                                      QWLAN_SIF_SIF_INT_EN_TX_PKT_OUT_OF_SYNC_INTR_EN_MASK |
                                                      QWLAN_SIF_SIF_INT_EN_TX_CMD53_XACTION_LT_PKT_LEN_ERR_INTR_EN_MASK |
                                                      QWLAN_SIF_SIF_INT_STATUS_RX_PKT_XFER_UNDERFLOW_INTR_MASK ) )
  {
    SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "Error disabling interrupts"));
    return VOS_STATUS_E_FAILURE;
  }

  /* Do we need to disable global interrupt or any other Libra interrupt?  */

  /* Disable function 0 if needed                                          */

  /* It is expected that chip power will be cut off - nothing more for now */

  /* Deregister callbacks, etc with SAL: is this okay?                     */
  sSSCRegistration.busCompleteCB = NULL;
  sSSCRegistration.interruptCB = NULL;
  sSSCRegistration.sscUsrData = NULL;

  if( VOS_STATUS_SUCCESS != WLANSAL_RegSscCBFunctions( pControlBlock->hSALHandle,
                                                       &sSSCRegistration ) )
  {
    return VOS_STATUS_E_FAILURE;
  }

  /* Change state                                                          */
  WLANSSC_TransitionState( pControlBlock,
                           WLANSSC_OPEN_STATE );

  return VOS_STATUS_SUCCESS;

} /* WLANSSC_SD_StopEventHandler() */


/**
 @brief WLANSSC_SD_ResumeEventHandler is used to handle the Resume event in
 suspended state

 @param Handle: SSC control block to operate on

 @see

 @return Result of the function call
*/
static VOS_STATUS WLANSSC_SD_ResumeEventHandler
(
  WLANSSC_ControlBlockType *pControlBlock
)
{

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /* Transition back to busy state and continue as before                  */
  WLANSSC_TransitionState( pControlBlock,
                           WLANSSC_READY_STATE );

  return VOS_STATUS_SUCCESS;

} /* WLANSSC_SD_ResumeEventHandler() */


/*---------------------------------------------------------------------------
 * Interrupt handlers implementation
 * ------------------------------------------------------------------------*/

/**
 @brief WLANSSC_FatalInterruptHandler is used to handle the interrupts
 which need a full reset for recovery (including upper layers)

 This will put the SSC in closed state and notify the client.

 @param Handle: SSC control block to operate on

 @see

 @return Result of the function call
*/
static VOS_STATUS WLANSSC_FatalInterruptHandler
(
  WLANSSC_ControlBlockType *pControlBlock
)
{
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  WLANSSC_ASSERT( NULL != pControlBlock );

  if( VOS_STATUS_SUCCESS != WLANSSC_ExecuteEvent( pControlBlock, 
                                                  WLANSSC_FATALERROR_EVENT ) )
  {
    /* Should never happen!                                                */
    WLANSSC_ASSERT( 0 );

    return VOS_STATUS_E_FAILURE;
  }

  return VOS_STATUS_SUCCESS;

} /* WLANSSC_FatalInterruptHandler() */


/**
 @brief WLANSSC_UnexpectedInterruptHandler is used to handle the interrupts
 which are unexpected.

 This will put the SSC in closed state and notify the client.

 @param Handle: SSC control block to operate on

 @see

 @return Result of the function call
*/
static VOS_STATUS WLANSSC_UnexpectedInterruptHandler
(
  WLANSSC_ControlBlockType *pControlBlock
)
{
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  WLANSSC_ASSERT( NULL != pControlBlock );

  /* This interrupt should never be generated                              */
  WLANSSC_ASSERT( 0 );

  /* Notify the upper layers just in case                                  */
  if( VOS_STATUS_SUCCESS != WLANSSC_ExecuteEvent( pControlBlock, 
                                                  WLANSSC_FATALERROR_EVENT ) )
  {
    /* Should never happen!                                                */
    WLANSSC_ASSERT( 0 );

    return VOS_STATUS_E_FAILURE;
  }

  return VOS_STATUS_SUCCESS;

} /* WLANSSC_UnexpectedInterruptHandler() */


/**
 @brief WLANSSC_TxFrmCRCErrInterruptHandler is used to handle the CRC errors
 on the transmit side as notified by the target

 This just discards the current transmit by notifying upper layers of a 
 failure, reset the Tx FIFO and proceed as before

 @param Handle: SSC control block to operate on

 @see

 @return Result of the function call
*/
static VOS_STATUS WLANSSC_TxFrmCRCErrInterruptHandler
(
  WLANSSC_ControlBlockType *pControlBlock
)
{
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  WLANSSC_ASSERT( NULL != pControlBlock );

  /* This interrupt should never be generated                              */
  WLANSSC_ASSERT( 0 );

  /* Call the Tx complete event with a failure                             */
  WLANSSC_ASSERT( NULL!= pControlBlock->stClientCbacks.pfnTxCompleteCback );

  pControlBlock->stClientCbacks.pfnTxCompleteCback( pControlBlock->pTxChain, 
                                                    VOS_STATUS_E_FAILURE, 
                                                    pControlBlock->stClientCbacks.UserData, 
                                                    pControlBlock->TxCompleteUserData );

  pControlBlock->pTxChain = NULL;
  pControlBlock->TxCompleteUserData = NULL;

  /* No need to reset the FIFO - just start a new transaction and see      */

  WLANSSC_TransitionState( pControlBlock, 
                           WLANSSC_READY_STATE );

  return VOS_STATUS_SUCCESS;

} /* WLANSSC_TxFrmCRCErrInterruptHandler() */


/**
 @brief WLANSSC_PMUWakeupInterruptHandler is used to handle the PMU wakeup
 interrupt

 @param Handle: SSC control block to operate on

 @see

 @return Result of the function call
*/
static VOS_STATUS WLANSSC_PMUWakeupInterruptHandler
(
  WLANSSC_ControlBlockType *pControlBlock
)
{
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  WLANSSC_ASSERT( NULL != pControlBlock );

  /* This interrupt should never be generated                              */
  WLANSSC_ASSERT( 0 );

  return VOS_STATUS_E_PERM;

} /* WLANSSC_PMUWakeupInterruptHandler() */


/**
 @brief WLANSSC_RxFIFOFullHandler is used to handle the Rx FIFO Full
 interrupt.

 @param Handle: SSC control block to operate on

 @see

 @return Result of the function call
*/
static VOS_STATUS WLANSSC_RxFIFOFullHandler
(
  WLANSSC_ControlBlockType *pControlBlock
)
{
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  WLANSSC_ASSERT( NULL != pControlBlock );

  SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "Rx FIFO Full: scheduler needs optimizing"));

  /** For now, this interrupt is not handled; but if need be, this can feed
      into the scheduler to optimize performance
  */

  return VOS_STATUS_SUCCESS;

} /* WLANSSC_RxFIFOFullHandler() */


/**
 @brief WLANSSC_RxFIFODataAvailInterruptHandler is used to handle the Rx FIFO
 data available interrupt.

 This is the primary interrupt that notifies us that there is more data
 available on the target.

 @param Handle: SSC control block to operate on

 @see

 @return Result of the function call
*/
static VOS_STATUS WLANSSC_RxFIFODataAvailInterruptHandler
(
  WLANSSC_ControlBlockType *pControlBlock
)
{
  vos_msg_t    sMessage;
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  WLANSSC_ASSERT( NULL != pControlBlock );

  /* Disable this interrupt until data is processed                        */
  WLANSSC_DisableRx( pControlBlock );

  /* We serialize into tx thread all the rx activity                       */
  SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO, "SSC rx data avail handler invoked: serializing to tx thread"));

  vos_mem_zero( &sMessage, sizeof(vos_msg_t) );
  
  sMessage.bodyptr = (v_PVOID_t)pControlBlock;
  sMessage.type = WLANSSC_RXPENDING_MESSAGE;
  sMessage.bodyval = 0;
  
  return vos_tx_mq_serialize(VOS_MQ_ID_SSC, &sMessage);

} /* WLANSSC_RxFIFODataAvailInterruptHandler() */


/**
 @brief WLANSSC_RxFIFOEmptyInterruptHandler is used to handle the Rx FIFO
 empty interrupt.

 This is the notification that no more data is available on the target.
 We should normally not concern ourselves with this interrupt since we use
 the receive data to piggyback interrupt status anyway.

 @param Handle: SSC control block to operate on

 @see

 @return Result of the function call
*/
static VOS_STATUS WLANSSC_RxFIFOEmptyInterruptHandler
(
  WLANSSC_ControlBlockType *pControlBlock
)
{
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  WLANSSC_ASSERT( NULL != pControlBlock );

  SSCLOGE(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_ERROR, "Rx FIFO empty interrupt"));

  return VOS_STATUS_SUCCESS;

} /* WLANSSC_RxFIFOEmptyInterruptHandler() */



#ifdef FEATURE_WLAN_UNSUPPORTED
/**
 @brief WLANSSC_ASICInterruptHandler is used to handle the ASIC interrupt
 
 This is to notify the client of an ASIC interrupt

 @param Handle: SSC control block to operate on

 @see

 @return Result of the function call
*/
static VOS_STATUS WLANSSC_ASICInterruptHandler
(
  WLANSSC_ControlBlockType *pControlBlock
)
{
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  WLANSSC_ASSERT( NULL != pControlBlock );

  SSCLOG1(VOS_TRACE( VOS_MODULE_ID_SSC, VOS_TRACE_LEVEL_INFO, "ASIC interrupt"));

  WLANSSC_ASSERT( NULL!= pControlBlock->stClientCbacks.pfnASICInterruptIndicationCback );

  pControlBlock->stClientCbacks.pfnASICInterruptIndicationCback( pControlBlock->stClientCbacks.UserData );

  return VOS_STATUS_SUCCESS;

} /* WLANSSC_ASICInterruptHandler() */
#endif /* FEATURE_WLAN_UNSUPPORTED */



/**
 @brief WLANSSC_RxPktXferUnderflowInterruptHandler is used to handle the Rx 
 pkt transfer underflow interrupt.

 @param Handle: SSC control block to operate on

 @see

 @return Result of the function call
*/
static VOS_STATUS WLANSSC_RxPktXferUnderflowInterruptHandler
(
  WLANSSC_ControlBlockType *pControlBlock
)
{
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  WLANSSC_ASSERT( NULL != pControlBlock );

  /* This interrupt should never be generated                              */
  WLANSSC_ASSERT( 0 );

  /* Notify the upper layers just in case                                  */
  if( VOS_STATUS_SUCCESS != WLANSSC_ExecuteEvent( pControlBlock, 
                                                  WLANSSC_FATALERROR_EVENT ) )
  {
    /* Should never happen!                                                */
    WLANSSC_ASSERT( 0 );

    return VOS_STATUS_E_FAILURE;
  }

  return VOS_STATUS_SUCCESS;

} /* WLANSSC_RxPktXferUnderflowInterruptHandler() */

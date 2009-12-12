/** ------------------------------------------------------------------------- *
    ------------------------------------------------------------------------- *


    \file halTypes.h

    \brief This header captures types that must be shared in common with individual
            module headers before inclusion into halCommonApi.h.

    $Id$

    Copyright (C) 2006 Airgo Networks, Incorporated


   ========================================================================== */

#ifndef HALTYPES_H
#define HALTYPES_H
#include "wlan_qct_bal.h"
#include "libraDefs.h"
#include "palTypes.h"


#define OFFSET_OF(structType,fldName)   (&((structType*)0)->fldName)

/** ------------------------------------------------------------------------- *

    \typedef tHalHandle

    \brief Handle to the HAL.  The HAL handle is returned by the HAL after it
    is opened (by calling halOpen).

    -------------------------------------------------------------------------- */
typedef void *tHalHandle;

// define a value for an invalid HAL handle.....
#define HAL_INVALID_HAL_HANDLE ( NULL )


/** ------------------------------------------------------------------------- *

    \enum eHalStatus

    \brief Enumeration of all status codes returned by the higher level
    HAL interface functions.

    -------------------------------------------------------------------------- */
typedef enum
{
    eHAL_STATUS_SUCCESS,

    // general failure.  This status applies to all failure that are not covered
    // by more specific return codes.
    eHAL_STATUS_FAILURE,
    eHAL_STATUS_FAILED_ALLOC,
    eHAL_STATUS_RESOURCES,

    // the HAL has not been opened and a HAL function is being attempted.
    eHAL_STATUS_NOT_OPEN,

    // function failed due to the card being removed...
    eHAL_STATUS_CARD_NOT_PRESENT,

    //halInterrupt status
    eHAL_STATUS_INTERRUPT_ENABLED,
    eHAL_STATUS_INTERRUPT_DISABLED,
    eHAL_STATUS_NO_INTERRUPTS,
    eHAL_STATUS_INTERRUPT_PRESENT,
    eHAL_STATUS_ALL_INTERRUPTS_PROCESSED,
    eHAL_STATUS_INTERRUPT_NOT_PROCESSED,        //interrupt cleared but no Isr to process

    // a parameter on the PAL function call is not valid.
    eHAL_STATUS_INVALID_PARAMETER,

    // the PAL has not been initialized...
    eHAL_STATUS_NOT_INITIALIZED,

    // Error codes for PE-HAL message API
    eHAL_STATUS_INVALID_STAIDX,
    eHAL_STATUS_INVALID_BSSIDX,
    eHAL_STATUS_STA_TABLE_FULL,             // No space to add more STA, sta table full.
    eHAL_STATUS_BSSID_TABLE_FULL,
    eHAL_STATUS_DUPLICATE_BSSID,
    eHAL_STATUS_DUPLICATE_STA,
    eHAL_STATUS_BSSID_INVALID,
    eHAL_STATUS_STA_INVALID,
    eHAL_STATUS_INVALID_KEYID,
    eHAL_STATUS_INVALID_SIGNATURE,

    //DXE
    eHAL_STATUS_DXE_FAILED_NO_DESCS,
    eHAL_STATUS_DXE_CHANNEL_NOT_CONFIG,         // Channel not configured
    eHAL_STATUS_DXE_CHANNEL_MISUSE,             // Specified operation inconsistent w/ configuration
    eHAL_STATUS_DXE_VIRTUAL_MEM_ALLOC_ERROR,    //
    eHAL_STATUS_DXE_SHARED_MEM_ALLOC_ERROR,     //
    eHAL_STATUS_DXE_INVALID_CHANNEL,
    eHAL_STATUS_DXE_INVALID_CALLBACK,
    eHAL_STATUS_DXE_INCONSISTENT_DESC_COUNT,
    eHAL_STATUS_DXE_XFR_QUEUE_ERROR,
    eHAL_STATUS_DXE_INVALID_BUFFER,
    eHAL_STATUS_DXE_INCOMPLETE_PACKET,
    eHAL_STATUS_DXE_INVALID_PARAMETER,
    eHAL_STATUS_DXE_CH_ALREADY_CONFIGURED,
    eHAL_STATUS_DXE_USB_INVALID_EP,
    eHAL_STATUS_DXE_GEN_ERROR,


    // status codes added for the ImageValidate library
    eHAL_STATUS_E_NULL_VALUE,
    eHAL_STATUS_E_FILE_NOT_FOUND,
    eHAL_STATUS_E_FILE_INVALID_CONTENT,
    eHAL_STATUS_E_MALLOC_FAILED,
    eHAL_STATUS_E_FILE_READ_FAILED,
    eHAL_STATUS_E_IMAGE_INVALID,
    eHAL_STATUS_E_IMAGE_UNSUPPORTED,

    // status code returned by device memory calls when memory is
    // not aligned correctly.
    eHAL_STATUS_DEVICE_MEMORY_MISALIGNED,          // memory access is not aligned on a 4 byte boundary
    eHAL_STATUS_DEVICE_MEMORY_LENGTH_ERROR,        // memory access is not a multiple of 4 bytes

    // Generic status code to indicate network congestion.
    eHAL_STATUS_NET_CONGESTION,

    // various status codes for Rx packet dropped conditions...  Note the Min and Max
    // enums that bracked the Rx Packet Dropped status codes.   There is code that
    // looks at the various packet dropped conditions so make sure these min / max
    // enums remain accurate.
    eHAL_STATUS_RX_PACKET_DROPPED,
    eHAL_STATUS_RX_PACKET_DROPPED_MIN = eHAL_STATUS_RX_PACKET_DROPPED,
    eHAL_STATUS_RX_PACKET_DROPPED_NULL_DATA,
    eHAL_STATUS_RX_PACKET_DROPPED_WDS_FRAME,
    eHAL_STATUS_RX_PACKET_DROPPED_FILTERED,
    eHAL_STATUS_RX_PACKET_DROPPED_GROUP_FROM_SELF,
    eHAL_STATUS_RX_PACKET_DROPPED_MAX = eHAL_STATUS_RX_PACKET_DROPPED_GROUP_FROM_SELF,

    // Status indicating that PMU did not power up and hence indicative of the fact that the clocks are not on
    eHAL_STATUS_PMU_NOT_POWERED_UP,

    // Queuing code for BA message API
    eHAL_STATUS_BA_ENQUEUED,        // packets have been buffered in Host
    eHAL_STATUS_BA_INVALID,

    // A-MPDU/BA related Error codes
    eHAL_STATUS_BA_RX_BUFFERS_FULL,
    eHAL_STATUS_BA_RX_MAX_SESSIONS_REACHED,
    eHAL_STATUS_BA_RX_INVALID_SESSION_ID,

    // !!LAC - can we rework the code so these are not needed?
    eHAL_STATUS_BA_RX_DROP_FRAME,
    eHAL_STATUS_BA_RX_INDICATE_FRAME,
    eHAL_STATUS_BA_RX_ENQUEUE_FRAME,

    // PMC return codes.
    eHAL_STATUS_PMC_PENDING,
    eHAL_STATUS_PMC_DISABLED,
    eHAL_STATUS_PMC_NOT_NOW,
    eHAL_STATUS_PMC_AC_POWER,
    eHAL_STATUS_PMC_SYS_ERROR,
    eHAL_STATUS_PMC_CANNOT_ENTER_IMPS,
    eHAL_STATUS_PMC_ALREADY_IN_IMPS,

    eHAL_STATUS_HEARTBEAT_TMOUT,
    eHAL_STATUS_NTH_BEACON_DELIVERY,

    //CSR
    eHAL_STATUS_CSR_WRONG_STATE,

    // DPU
    eHAL_STATUS_DPU_DESCRIPTOR_TABLE_FULL,
    eHAL_STATUS_DPU_MICKEY_TABLE_FULL,

    // HAL-FW messages
    eHAL_STATUS_FW_MSG_FAILURE,                // Error in Hal-FW message interface
    eHAL_STATUS_FW_MSG_TIMEDOUT,
    eHAL_STATUS_FW_MSG_INVALID,
    eHAL_STATUS_FW_SEND_MSG_FAILED,
    eHAL_STATUS_FW_PS_BUSY,

    eHAL_STATUS_TIMER_START_FAILED,
    eHAL_STATUS_TIMER_STOP_FAILED,

    eHAL_STATUS_UMA_DESCRIPTOR_TABLE_FULL,

    eHAL_STATUS_SET_CHAN_ALREADY_ON_REQUESTED_CHAN,

    // not a real status.  Just a way to mark the maximum in the enum.
    eHAL_STATUS_MAX

} eHalStatus;


// macro to check for SUCCESS value of the halStatus
#define HAL_STATUS_SUCCESS( variable ) ( eHAL_STATUS_SUCCESS == ( variable ) )

/// Bit value data structure
typedef enum sHalBitVal  // For Bit operations
{
    eHAL_CLEAR,
    eHAL_SET
}tHalBitVal;

// -------------------------------------------------------------
/// MMH APIs
enum {
   eHI_PRI,
   ePROT,
   eDBG
};

/// System role definition
// FIXME - Change the type
typedef enum eSystemRole
{
    eSYSTEM_UNKNOWN_ROLE,
    eSYSTEM_AP_ROLE,
    eSYSTEM_STA_IN_IBSS_ROLE,
    eSYSTEM_STA_ROLE
} tSystemRole;

// ---------------------------------------
// Channel Bonding Sideband configuration
// ---------------------------------------
typedef enum sHalCBsidebandType
{
    eHAL_SIDEBAND_CENTER=0,
    eHAL_SIDEBAND_LOWER,
    eHAL_SIDEBAND_UPPER,
    eHAL_SIDEBAND_COPY
}tHalCBsidebandType;


/// HAL states
typedef enum {
	eHAL_IDLE,
    eHAL_INIT,
    eHAL_CFG, //CFG download completed.
    eHAL_STARTED, //halProcessStartEvent compelted.
    eHAL_SYS_READY, //Sys_ready msg received from HDD.
    eHAL_NORMAL, //Sys_ready msg received from HDD and halProcessStartEvent completed.
} tHAL_STATE;




// Type to define softmac mode (also system mode)
typedef enum
{
    //3- Promisc, 2 - Scan, 1 - Learn  0 - Normal
    eHAL_SYS_MODE_NORMAL = 0,
    eHAL_SYS_MODE_LEARN,
    eHAL_SYS_MODE_SCAN,
    eHAL_SYS_MODE_PROMISC
} eHalSysMode;




// HAL frame types.  Used on the TxRx APIs and the
// corresponding PAL routines.
typedef enum {

    HAL_TXRX_FRM_RAW,
    HAL_TXRX_FRM_ETH2,
    HAL_TXRX_FRM_802_3,
    HAL_TXRX_FRM_802_11_MGMT,
    HAL_TXRX_FRM_802_11_CTRL,
    HAL_TXRX_FRM_802_11_DATA,
    HAL_TXRX_FRM_IGNORED,   //This frame will be dropped
    HAL_TXRX_FRM_MAX

} eFrameType;


typedef enum
{
    ANI_TXDIR_IBSS = 0,
    ANI_TXDIR_TODS,
    ANI_TXDIR_FROMDS,
    ANI_TXDIR_WDS

} eFrameTxDir;

typedef enum
{
    eRF_BAND_UNKNOWN = 0,
    eRF_BAND_2_4_GHZ = 1,
    eRF_BAND_5_GHZ = 2
} eRfBandMode;


#ifndef __offsetof
#define __offsetof(type, field) ((tANI_U32)(&((type *)0)->field))
#endif

#ifndef offsetof
#define offsetof(type, field) __offsetof(type, field)
#endif


#endif


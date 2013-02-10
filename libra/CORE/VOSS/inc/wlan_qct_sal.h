#ifndef WLAN_QCT_SAL_H
#define WLAN_QCT_SAL_H

/**=========================================================================
  
  @file  wlan_qct_sal.h
  
  @brief WLAN SDIO ABSTRACTION LAYER EXTERNAL API
               
   This file contains the external API exposed by the wlan SDIO abstraction layer module.
   Copyright (c) 2008 Qualcomm Technologies, Inc. All Rights Reserved.
   Qualcomm Technologies Confidential and Proprietary
========================================================================*/

/*===========================================================================

                      EDIT HISTORY FOR FILE


  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.


  $Header:$ $DateTime: $ $Author: $


when           who        what, where, why
--------    ---         ----------------------------------------------------------
05/21/08    schang      Created module.

===========================================================================*/

/*===========================================================================

                          INCLUDE FILES FOR MODULE

===========================================================================*/

/*----------------------------------------------------------------------------
 * Include Files
 * -------------------------------------------------------------------------*/
#include "vos_api.h"

#ifdef __cplusplus
extern "C" {
#endif
/*----------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * -------------------------------------------------------------------------*/
#ifdef PLATFORM_FFA
#define WLANSAL_PLATFORM_FFA_FIXED_SLOT   1
#endif /* PLATFORM_FFA */

/* SDIO Clock speed available type
 * SDBUS module supports clock speed
 * 100KHz, 400KHz, 20MHz, 25MHz
 * SDHC supports clock speed
 * ID_MODE 144KHz, INIT_MODE 16MHz, DATA_RANSFER_MODE 20Mhz, HIGH_SPEED_MODE 49MHz */
/* SDHC 144KHz, call SDBUS SD_DEFAULT_CARD_ID_CLOCK_RATE, 100KHz */
#define WLANSAL_SDIO_CSPEED_ID_MODE       144
/* SDHC 144KHz, call SDBUS SD_LOW_SPEED_RATE, 400 KHz */
#define WLANSAL_SDIO_CSPEED_LOW_MODE      400
/* SDHC 20MHz, call SDBUS SD_FULL_SPEED_RATE, 25MHz */
#define WLANSAL_SDIO_CSPEED_FULL_MODE     25000
/* SDHC 49.152MHZ, SDBUS NOT Support */
#define WLANSAL_SDIO_CSPEED_HIGH_MODE     49000

/*----------------------------------------------------------------------------
 *  Type Declarations
 * -------------------------------------------------------------------------*/

/* SSC callback functions typedef */
/*----------------------------------------------------------------------------

   @brief Interrupt callback type has to be registered to SAL from SSC.
        Any lower layer interrupt is happen this function will be invoked.

   @param v_PVOID_t pAdapter
        Global adapter handle

   @return General status code
        VOS_STATUS_SUCCESS      Success
        VOS_STATUS_E_FAILURE    SSC Cback function pointer is not valid
        
----------------------------------------------------------------------------*/
typedef VOS_STATUS
(*WLANSAL_InterruptCBType)(v_PVOID_t pAdapter, v_PVOID_t usrData);

/*----------------------------------------------------------------------------

   @brief Bus request operation is done this callback function will be invoked.
        This callback process is only needed with ASYNC functions call

   @param v_PVOID_t pAdapter
        Global adapter handle
        
   @return General status code
        VOS_STATUS_SUCCESS      Success
        VOS_STATUS_E_FAILURE    HAL Cback function pointer is not valid
        
----------------------------------------------------------------------------*/
typedef VOS_STATUS
(*WLANSAL_BusCompleteCBType)(v_PVOID_t pAdapter, VOS_STATUS busStatus, v_PVOID_t usrdata);

/* Bus direction, read or write */
typedef enum
{
   WLANSAL_DIRECTION_READ = 0,
   WLANSAL_DIRECTION_WRITE = 1
} WLANSAL_DIRECTION_TYPE;

/* Byte mode or block mode */
typedef enum
{
   WLANSAL_MODE_BYTE,
   WLANSAL_MODE_BLOCK
} WLANSAL_MODE_TYPE;

/* Synchronous or asynchronous function call */
typedef enum
{
   WLANSAL_CALL_SYNC,
   WLANSAL_CALL_ASYNC
} WLANSAL_CALL_TYPE;

/* address handle */
typedef enum
{
   WLANSAL_ADDRESS_FIXED,
   WLANSAL_ADDRESS_INCREMENT
} WLANSAL_ADDRESS_HANDLE_TYPE;

/* Function Supported */
typedef enum
{
   WLANSAL_FUNCTION_ZERO,
   WLANSAL_FUNCTION_ONE
} WLANSAL_AVAILABLE_FUNCTION_TYPE;

/* Availble block size */
typedef enum
{
   WLANSAL_AVAILABLE_BLOCK_SIZE_32,
   WLANSAL_AVAILABLE_BLOCK_SIZE_64,
   WLANSAL_AVAILABLE_BLOCK_SIZE_128,
   WLANSAL_AVAILABLE_BLOCK_SIZE_256
} WLANSAL_AVAILABLE_BLOCK_SIZE_TYPE;

/* Card present status changed notification path
 * If Card is removed from slot or put into slot
 * Notification may routed to SAL or SDBUS */
typedef enum
{
   WLANSAL_NOTF_PATH_SDBUS,
   WLANSAL_NOTF_PATH_SAL
} WLANSAL_NOTF_PATH_T;


/* To request CMD52 client have to pass these information to SAL */
typedef struct
{
   WLANSAL_DIRECTION_TYPE            busDirection;      // BUS direction, Read or write direction
   WLANSAL_AVAILABLE_FUNCTION_TYPE   function;
   v_U32_t                           address;           // Target address
   v_U8_t                           *dataPtr;           // Data pointer
} WLANSAL_Cmd52ReqType;

/* To request CMD53 client have to pass these information to SAL */
typedef struct
{
   WLANSAL_DIRECTION_TYPE            busDirection;      // BUS direction, Read or write direction
   WLANSAL_CALL_TYPE                 callSync;          // Sync or Async call
   WLANSAL_BusCompleteCBType         busCompleteCB;     // If bus request is done, this callback have to be issued with asynchronous function call
   WLANSAL_AVAILABLE_FUNCTION_TYPE   function;
   WLANSAL_ADDRESS_HANDLE_TYPE       addressHandle;
   WLANSAL_MODE_TYPE                 mode;
   v_U32_t                           address;           // Target address
   v_U32_t                           dataSize;          // Read or write data size
   v_U8_t                           *dataPtr;           // Data pointer
} WLANSAL_Cmd53ReqType;

/* SSC has to register CB functions and handles with SscRegType structure */
typedef struct
{
   WLANSAL_InterruptCBType      interruptCB;     // Interrupt CB function PTR
   WLANSAL_BusCompleteCBType    busCompleteCB;   // Bus complete CB function PTR
   v_PVOID_t                    sscUsrData;
} WLANSAL_SscRegType;

/* To enable or disable interrupt, SSC re-register CB function if needed */
typedef struct
{
   v_U32_t                      clockRate;       // Card clock rate KHz
   v_U32_t                      blockSize;       // Card block size
   /* TBD */                                     // If need any else, put in here
} WLANSAL_CardInfoType;

/*-------------------------------------------------------------------------
 *Function declarations and documenation
 *-------------------------------------------------------------------------*/

/*=========================================================================
 * Interactions with BAL
 *=========================================================================*/ 
/*----------------------------------------------------------------------------

   @brief Open SAL Module.
        Allocate internal resources, Initialize LOCK element,
        and allocate SDIO handle

   @param v_PVOID_t pAdapter
        Global adapter handle

   @param v_U32_t   sdBusDCtxt
        Platform specific device context

   @return General status code
        VOS_STATUS_SUCCESS      Open Success
        VOS_STATUS_E_NOMEM      Open Fail, Resource alloc fail
        VOS_STATUS_E_FAILURE    Open Fail, Unknown reason        
      
----------------------------------------------------------------------------*/
VOS_STATUS WLANSAL_Open
(
   v_PVOID_t pAdapter,
   v_U32_t   sdBusDCtxt
);

/*----------------------------------------------------------------------------

   @brief Start SAL module.
        Probing SDIO interface, get and store card information

   @param v_PVOID_t pAdapter
        Global adapter handle

   @return General status code
        VOS_STATUS_SUCCESS       Start Success
        VOS_STATUS_E_FAILURE     Start Fail, BAL Not open yet
        VOS_STATUS_E_INVAL       Invalid argument

----------------------------------------------------------------------------*/
VOS_STATUS WLANSAL_Start
(
   v_PVOID_t pAdapter
);

/*----------------------------------------------------------------------------

   @brief Stop SAL module. 
        Initialize internal resources

   @param  v_PVOID_t pAdapter
        Global adapter handle

   @return General status code
        VOS_STATUS_SUCCESS       Stop Success
        VOS_STATUS_E_FAILURE     Stop Fail, BAL not started   
        VOS_STATUS_E_INVAL       Invalid argument
        
----------------------------------------------------------------------------*/
VOS_STATUS WLANSAL_Stop
(
   v_PVOID_t pAdapter
);

/*----------------------------------------------------------------------------

   @brief Close SAL module. 
        Free internal resources already allocated.
        Close LOCK


   @param v_PVOID_t pAdapter
        Global adapter handle

   @return General status code
        VOS_STATUS_SUCCESS       Close Success
        VOS_STATUS_E_FAILURE     Close Fail, BAL not open 
        VOS_STATUS_E_INVAL       Invalid argument
      
----------------------------------------------------------------------------*/
VOS_STATUS WLANSAL_Close
(
   v_PVOID_t pAdapter
);

/*----------------------------------------------------------------------------

   @brief   
      - TBD

   @param v_PVOID_t pAdapter
        Global adapter handle

   @return General status code
      
----------------------------------------------------------------------------*/
VOS_STATUS WLANSAL_Reset
(
   v_PVOID_t pAdapter
);

/*=========================================================================
 * END Interactions with BAL
 *=========================================================================*/ 

/*=========================================================================
 * General Functions
 *=========================================================================*/ 
/*----------------------------------------------------------------------------

   @brief SDIO CMD52 Read or write one byte at a time

   @param v_PVOID_t pAdapter
        Global adapter handle
        
   @param  WLANSAL_Cmd52ReqType *cmd52Req
           WLANSAL_BUS_DIRECTION_TYPE BUS direction, Read or write direction
           v_U32_t                    Target address
           v_PVOID_t                  Data pointer
           v_PVOID_t                  SDIO internal handle
   @return General status code
        VOS_STATUS_SUCCESS       Read or write success
        VOS_STATUS_E_INVAL       cmd is not valid
        VOS_STATUS_E_FAILURE     SAL is not ready
        
----------------------------------------------------------------------------*/
VOS_STATUS WLANSAL_Cmd52
(
   v_PVOID_t             pAdapter,
   WLANSAL_Cmd52ReqType *cmd52Req
);

/*----------------------------------------------------------------------------

   @brief SDIO CMD53 Read or write multiple bytes at a time.
        Read or write can be happen with synchronous functions call or
        asynchronous function call, depends on clients request.

   @param v_PVOID_t pAdapter
        Global adapter handle
        
   @param  WLANSAL_Cmd53ReqType *cmd53Req
           WLANSAL_BUS_DIRECTION_TYPE BUS direction, Read or write direction
           WLANSAL_BUS_MODE_TYPE      Byte or block mode
           WLANSAL_BUS_EAPI_TYPE      Sync or Async call
           WLANSAL_BusCompleteCBType  If bus request is done,
                                      this callback have to be issued with
                                      asynchronous function call
           v_U32_t                    Target address
           v_U16_t                    Read or write data size
           v_PVOID_t                  Data pointer
           v_PVOID_t                  SDIO internal handle

   @return General status code
        VOS_STATUS_SUCCESS       Read or write success
        VOS_STATUS_E_INVAL       cmd is not valid
        VOS_STATUS_E_FAILURE     SAL is not ready
        
----------------------------------------------------------------------------*/
VOS_STATUS WLANSAL_Cmd53
(
   v_PVOID_t             pAdapter,
   WLANSAL_Cmd53ReqType *cmd53Req
);

/*=========================================================================
 * END General Functions
 *=========================================================================*/ 
/*=========================================================================
 * Interactions with SSC
 *=========================================================================*/ 
 
/*----------------------------------------------------------------------------

   @brief Register SSC callback functions to SAL.
        Just after SAL open DONE, callback functions have to be registered.
        Registration functions are TX complete, RX complete and interrupt happen.
        Fatal error callback function is TBD yet.

   @param v_PVOID_t pAdapter
        Global adapter handle
        
   @param WLANSAL_SscRegType
           WLANSAL_InterruptCBType      interrupt CB function PTR
           WLANSAL_BusCompleteCBType    Bus complete CB function PTR
           v_PVOID_t                    SSC handle
           v_PVOID_t                    SAL handle

   @return General status code
        VOS_STATUS_SUCCESS       Registration success
        VOS_STATUS_E_RESOURCES   SAL resources are not ready
        VOS_STATUS_E_INVAL       Invalid argument
      
----------------------------------------------------------------------------*/
VOS_STATUS WLANSAL_RegSscCBFunctions
(
   v_PVOID_t           pAdapter,
   WLANSAL_SscRegType *sscReg
);

/*----------------------------------------------------------------------------

   @brief De Register SSC callback functions from SAL.

   @param v_PVOID_t pAdapter
        Global adapter handle
        
   @return General status code
        VOS_STATUS_SUCCESS       Registration success
        VOS_STATUS_E_RESOURCES   SAL resources are not ready
        VOS_STATUS_E_INVAL       Invalid argument
      
----------------------------------------------------------------------------*/
VOS_STATUS WLANSAL_DeregSscCBFunctions
(
   v_PVOID_t           pAdapter
);

/*----------------------------------------------------------------------------

   @brief Query card information.
        Card information will be got during WLANSAL_Start.
        Card information is stored SAL internal structure,

   @param v_PVOID_t pAdapter
        Global adapter handle
        
   @param WLANSAL_CardInfoType *cardInfo
           WLANSAL_CARD_INTERFACE_TYPE  1bit or 4 bit interface
           v_U32_t                      Card clock rate
           v_U32_t                      Card block size
           v_PVOID_t                    SAL handle

   @return General status code
        VOS_STATUS_SUCCESS       Query success
        VOS_STATUS_E_RESOURCES   SAL resources are not ready
        VOS_STATUS_E_INVAL       Invalid argument
      
----------------------------------------------------------------------------*/
VOS_STATUS WLANSAL_CardInfoQuery
(
   v_PVOID_t             pAdapter,
   WLANSAL_CardInfoType *cardInfo
);

/*----------------------------------------------------------------------------

   @brief Update card information.

   @param v_PVOID_t pAdapter
        Global adapter handle
        
   @param WLANSAL_CardInfoType *cardInfo
           WLANSAL_CARD_INTERFACE_TYPE  1bit or 4 bit interface
           v_U32_t                      Card clock rate
           v_U32_t                      Card block size
           v_PVOID_t                    SAL handle

   @return General status code
        VOS_STATUS_SUCCESS       Update success
        VOS_STATUS_E_RESOURCES   SAL resources are not ready
        VOS_STATUS_E_INVAL       Invalid argument
      
----------------------------------------------------------------------------*/
VOS_STATUS WLANSAL_CardInfoUpdate
(
   v_PVOID_t             pAdapter,
   WLANSAL_CardInfoType *cardInfo
);

/*=========================================================================
 * END Interactions with SSC
 *=========================================================================*/ 

/*----------------------------------------------------------------------------

   @brief Set Card presence status path
          Card present status changed notification path
          If Card is removed from slot or put into slot
          Notification may routed to SAL or SDBUS

   @param v_PVOID_t pAdapter
        Global adapter handle

   @param WLANSAL_NOTF_PATH_T   path
        Notification Path, it may SAL or SDBUS
        
   @return General status code
        VOS_STATUS_SUCCESS       Update success
        VOS_STATUS_E_RESOURCES   SAL resources are not ready
        VOS_STATUS_E_INVAL       Invalid argument
      
----------------------------------------------------------------------------*/
VOS_STATUS WLANSAL_SetCardStatusNotfPath
(
   v_PVOID_t             pAdapter,
   WLANSAL_NOTF_PATH_T   path
);

/*----------------------------------------------------------------------------

   @brief Reinitialize LIBRA's SDIO core
          Deep sleep status is same with turn off power
          So, standard SDIO init procedure is needed

   @param v_PVOID_t pAdapter
        Global adapter handle

   @return General status code
        VOS_STATUS_SUCCESS       Update success
        VOS_STATUS_E_RESOURCES   SAL resources are not ready
        VOS_STATUS_E_INVAL       Invalid argument
      
----------------------------------------------------------------------------*/
VOS_STATUS WLANSAL_SDIOReInit
(
   v_PVOID_t             pAdapter
);

/*----------------------------------------------------------------------------

   @brief Check the SDHC support deep sleep or not
          To support deep sleep, SAL should change SDHC configurations
          Clock speed and bus bandwidth
          To chnage the configuration, interface between SAL and SDHC is needed
          Before goes into deep sleep, HDD will check there is interface or not

   @param void

   @return boolean
           VOS_TRUE   SDHC support deep sleep interface
           VOS_FALSE  SDHC does not support deep sleep interface

----------------------------------------------------------------------------*/
v_BOOL_t WLANSAL_IsSDHCSupportDeepSleep
(
   v_PVOID_t             pAdapter
);
#ifdef __cplusplus
}
#endif
#ifdef WLAN_SOFTAP_FEATURE
#ifdef LIBRA_LINUX_PC
VOS_STATUS WLANSAL_SetSDIOClock(unsigned int clk_freq);
#endif
#endif //WLAN_SOFTAP_FEATURE
#endif /* WLAN_QCT_SAL_H */


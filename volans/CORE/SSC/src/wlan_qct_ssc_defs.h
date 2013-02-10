#ifndef WLAN_QCT_SSC_DEFS_H
#define WLAN_QCT_SSC_DEFS_H

/*===========================================================================
  @file wlan_qct_ssc_defs.h

  @brief This header file provides definitions of the registers and other 
  values related to the SIF hardware block

  Please see the SSC/SIF documentation for further details.
  
  Copyright (c) 2008 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Qualcomm Technologies Confidential and Proprietary
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

/* SIF register values                                                     */
#include "qwlanhw_volans.h"

/*---------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * ------------------------------------------------------------------------*/

/* Mask that specifies whether the address is for upper 64k or not         */
#define WLANSSC_CMD_ADDR_WLAN_CSR_MASK        0x10000

/* Sync Sequence for Tx and Rx Start Descriptors                           */
#define WLANSSC_SYNC_SEQ_DWORD_0        0x7D7D7D7D
#define WLANSSC_SYNC_SEQ_DWORD_1        0x7D7D7D7E

/* Offset of interrupt status registers for both RX SD and RX SD           */
#define WLANSSC_RX_INT_STATUS_OFFSET        16




/*---------------------------------------------------------------------------
 * Type Declarations
 * ------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
   WLANSSC_TxStartDescriptorType

   This is the format of the Tx start descriptor
---------------------------------------------------------------------------*/
typedef struct
{
  /* Sync sequence is represented as 2 dwords                              */
  v_U32_t  uSyncSeqDword0;
  v_U32_t  uSyncSeqDword1;

  /* Control dword is represented as both u32 as well as bitfields         */
  union
  {
    v_U32_t uControl;

    struct
		{
			v_U32_t  uframeLength         :16;   /* Length of frame fragment     */
			v_U32_t  ustuffBits           :13;   /* Unused                       */
			v_U32_t  uframeQueueId        :2;    /* Unused                       */
			v_U32_t  ufragIndicator       :1;    /* Unused                       */
		} sCtrlBits;
  } controlInfo;
} WLANSSC_TxStartDescriptorType;

/*---------------------------------------------------------------------------
   WLANSSC_RxStartDescriptorType

   This is the format of the Rx start descriptor
---------------------------------------------------------------------------*/
typedef struct
{
  /* Sync sequence is represented as 2 dwords                              */
  v_U32_t  uSyncSeqDword0;
  v_U32_t  uSyncSeqDword1;

  /* Control dword is represented as both u32 as well as bitfields         */
  union
  {
    v_U32_t uControl;

    struct
		{
      v_U32_t uFrameLength          :16;  /* Length of frame or remainder  */

      /* Interrupt status bits                                             */
      v_U32_t uAsicInterrupt                         :1;
      v_U32_t uTxFrameCRCErrorInterrupt              :1;
      v_U32_t uTxFrameDXEErrorInterrupt              :1;
      v_U32_t uTxFIFOFullDurationTimeoutInterrupt    :1;
      v_U32_t uRxFIFOEmptyInterrupt                  :1;
      v_U32_t uRxFIFODataAvailableInterrupt          :1;
      v_U32_t uRxFrameUnderflowErrorInterrupt        :1;
      v_U32_t uRxFIFOFullInterrupt                   :1;
      v_U32_t uCSRWriteAccessErrorInterrupt          :1;
      v_U32_t uCSRReadAccessErrorInterrupt           :1;
      v_U32_t uPMUWakeupInterrupt                    :1;
      v_U32_t uHostAbortedTxFrameInterrupt           :1;
      v_U32_t uRxFrameDXEErrorInterrupt              :1;

      v_U32_t uStartDescriptorCode  :3;  /* Start descriptor code          */

		} sCtrlBits;
  } controlInfo;

  v_U16_t   u16BMURxWQByteCount;       /* Number of bytes pending on chip  */
  v_U16_t   u16RxFIFOInterruptStatus;  /* Interrupt status snapshot        */

} WLANSSC_RxStartDescriptorType;


/*---------------------------------------------------------------------------
   WLANSSC_RxEndDescriptorType

   This is the format of the Rx end descriptor
---------------------------------------------------------------------------*/
typedef struct
{
  union
  {
    v_U32_t uControl0;
    
    struct
		{
			v_U32_t uActualXferLength    :16;  /* Actual data transferred        */
	
			v_U32_t uStuffBits           :13;  /* Unused                         */
	
			v_U32_t uEndDescriptorCode   :3;   /* End descriptor code            */

		} sCtrlBits0;
  } controlInfo0;

	union
	{
		v_U32_t uControl1;

		struct
		{
			v_U16_t uBMURxWQByteCount;            /* Data pending on chip        */
	
      union
      {
        v_U16_t uInterruptStatus;

        struct
        {
          /* Interrupt status bits                                         */
          v_U16_t uAsicInterrupt                          :1;
          v_U16_t uTxFrameCRCErrorInterrupt               :1;
          v_U16_t uTxFrameDXEErrorInterrupt               :1;
          v_U16_t uTxFIFOFullDurationTimeoutInterrupt     :1;
          v_U16_t uRxFIFOEmptyInterrupt                   :1;
          v_U16_t uRxFIFODataAvailableInterrupt           :1;
          v_U16_t uRxFrameUnderflowErrorInterrupt         :1;
          v_U16_t uRxFIFOFullInterrupt                    :1;
          v_U16_t uCSRWriteAccessErrorInterrupt           :1;
          v_U16_t uCSRReadAccessErrorInterrupt            :1;
          v_U16_t uPMUWakeupInterrupt                     :1;
          v_U16_t uHostAbortedTxFrameInterrupt            :1;
          v_U16_t uRxFrameDXEErrorInterrupt               :1;
          v_U16_t uTxPktOutOfSyncInterrupt                :1;
          v_U16_t uTxCmd53XactionPktLengthErrorInterrupt  :1;
          v_U16_t uSingleWLANCSRWriteAccessErrorInterrupt :1;
        } sInterruptBits;
      } interruptInfo;
		} sCtrlBits1;
	} controlInfo1;

} WLANSSC_RxEndDescriptorType;


/*---------------------------------------------------------------------------
   WLANSSC_RxSDCodeType

   This is the enumeration of codes for the Rx start descriptor
---------------------------------------------------------------------------*/
typedef enum 
{
  WLANSSC_NONEPENDING_RXSDCODE          = 0,
  WLANSSC_WATERMARKREACHED_RXSDCODE     = 1,
  WLANSSC_COMPLETEFRAME_RXSDCODE        = 2,
  WLANSSC_UNDERFLOWREMAINDER_RXSDCODE   = 3,

  WLANSSC_MAX_RXSDCODE

} WLANSSC_RxSDCodeType;


/*---------------------------------------------------------------------------
   WLANSSC_RxEDCodeType

   This is the enumeration of codes for the Rx end descriptor
---------------------------------------------------------------------------*/
typedef enum 
{
  WLANSSC_NONE_RXEDCODE                          =  0,
  WLANSSC_PARTIALFRAMEBELOWWATERMARK_RXEDCODE    =  1,
  WLANSSC_PARTIALFRAMEABOVEWATERMARK_RXEDCODE    =  2,
  WLANSSC_COMPLETEFRAME_RXEDCODE                 =  3,
  WLANSSC_COMPLETEANDPARTIALFRAMES_RXEDCODE      =  4,
  WLANSSC_UNDERFLOWERROR_RXEDCODE                =  5,
  WLANSSC_UNDERFLOWREMAINDER_RXEDCODE            =  6,
  WLANSSC_COMPLETEREMAINDER_RXEDCODE             =  7,

  WLANSSC_MAX_RXEDCODE

} WLANSSC_RxEDCodeType;


/*---------------------------------------------------------------------------
 * Function Declarations and Documentation
 * ------------------------------------------------------------------------*/



#endif /* #ifndef WLAN_QCT_SSC_DEFS_H */

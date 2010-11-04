/*=========================================================================

  @file  wlan_qct_bal.c

  @brief WLAN BUS ABSTRACTION LAYER EXTERNAL API

   This file contains the external API exposed by the wlan bus abstraction layer module.

   Copyright 2008 (c) Qualcomm, Incorporated.  All Rights Reserved.

   Qualcomm Confidential and Proprietary.

  ========================================================================*/

/*===========================================================================

                      EDIT HISTORY FOR FILE


  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.


  $Header:$ $DateTime: $ $Author: $


when           who        what, where, why
--------    ---         ----------------------------------------------------------
05/21/08    schang      Created module.
05/05/09                Linux porting

===========================================================================*/

/* $Header$ */

/*===========================================================================

                          INCLUDE FILES FOR MODULE

===========================================================================*/

/*----------------------------------------------------------------------------
 * Include Files
 * -------------------------------------------------------------------------*/
#include <wlan_hdd_includes.h>
#include <wlan_qct_bal.h>
#include <wlan_bal_misc.h>
#include <wlan_sal_misc.h>
#include <wlan_qct_hal.h>
#ifdef ANI_CHIPSET_VOLANS
#include <qwlanhw_volans.h>
#else
#include <libra.h>
#endif
/*----------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * -------------------------------------------------------------------------*/
#ifdef VOLANS_1_0_WORKAROUND
#ifndef MSM_PLATFORM
#define WLAN_HIGH_SD_CLOCK_FREQ 32500000
#define WLAN_LOW_SD_CLOCK_FREQ 10000000
#elif defined MSM_PLATFORM_7x30
#define WLAN_HIGH_SD_CLOCK_FREQ 49152000
#define WLAN_LOW_SD_CLOCK_FREQ 16027000
#elif defined MSM_PLATFORM_7x27
#define WLAN_HIGH_SD_CLOCK_FREQ 49152000
#define WLAN_LOW_SD_CLOCK_FREQ 16000000
#elif defined MSM_PLATFORM_8660
#define WLAN_HIGH_SD_CLOCK_FREQ 48000000
#define WLAN_LOW_SD_CLOCK_FREQ 16000000
#endif
#endif /* VOLANS_1_0_WORKAROUND */
/*----------------------------------------------------------------------------
 *  Type Declarations
 * -------------------------------------------------------------------------*/
balHandleType *gbalHandle;

#ifdef VOLANS_1_0_WORKAROUND
tANI_U32 SpModeRegVal[4] = {0,0,0,0};
#endif

/*-------------------------------------------------------------------------
 *Function declarations and documenation
 *-------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------

  @brief With every TX frames add DXE header space
         and assign proper value for DXE header

  @param v_PVOID_t pAdapter  Global adapter handle
  @param vos_pkt_t *pktChainPtr Packet chain start pointer

  @return General status code
        VOS_STATUS_SUCCESS      Open Success
        VOS_STATUS_E_NOMEM      Open Fail, Resource alloc fail
        VOS_STATUS_E_FAILURE    Open Fail, Unknown reason

----------------------------------------------------------------------------*/
static VOS_STATUS balHandleDXEHeader
(
   v_PVOID_t  pAdapter,
   vos_pkt_t *pktChainPtr
)
{
   VOS_STATUS      status = VOS_STATUS_SUCCESS;
   vos_pkt_t      *currentPktPtr;
   vos_pkt_t      *followedPktPtr;
   v_VOID_t       *pData;
   v_U16_t         packetLength;
   v_U32_t         length32;

   BENTER();

   currentPktPtr = pktChainPtr;
   while(1)
   {

#ifdef WLAN_PERF
      v_U32_t uBalUserData;

      uBalUserData = 0;
      if(currentPktPtr)
      {
         /* BAL user data is set only by TL when STA is authenticated and a frame could be fast forwarded.
          * Therefore if the value returned is nonzero, BAL just returns without overhead of
          * building DxE header and calling other VOSS APIs.
          */
         vos_pkt_get_user_data_ptr( currentPktPtr, VOS_PKT_USER_DATA_ID_BAL,
                                    (v_PVOID_t)&uBalUserData);
         if(uBalUserData)
               goto skip;
       }

#endif /* WLAN_PERF */

      status = vos_pkt_get_packet_length(currentPktPtr, &packetLength);
      if(!VOS_IS_STATUS_SUCCESS(status))
      {
         BMSGERROR("%s: vos_pkt_get_packet_length Fail with %d\n", __func__, status, 0);
         BEXIT();
         return VOS_STATUS_E_FAILURE;
      }

      length32 = (v_U32_t)packetLength;
      status = vos_pkt_reserve_head(currentPktPtr, &pData, sizeof(WLANBAL_sDXEHeaderType));
      if(!VOS_IS_STATUS_SUCCESS(status))
      {
         BMSGERROR("%s: vos_pkt_reserve_head Fail with %d\n", __func__, status, 0);
         BEXIT();
         return VOS_STATUS_E_FAILURE;
      }

      //ASSUPTION except size every elements already SWAPPED
      ((WLANBAL_sDXEHeaderType *)pData)->size              = SWAP_ENDIAN_UINT32(length32);
#ifdef WLAN_PERF
skip:
      // For Android we still need to build the DXE header... so lets get to the start of the DXE header.
      // The space is allocated by TL.
      vos_pkt_peek_data( currentPktPtr, 0, &pData, 2);
#endif /* WLAN_PERF */
      ((WLANBAL_sDXEHeaderType *)pData)->descriptorControl = gbalHandle->sdioDXEConfig.TXChannel.descriptorControl;
      ((WLANBAL_sDXEHeaderType *)pData)->srcAddress        = gbalHandle->sdioDXEConfig.TXChannel.sourceAddress;
      ((WLANBAL_sDXEHeaderType *)pData)->destAddress       = gbalHandle->sdioDXEConfig.TXChannel.destinationAddress;
      ((WLANBAL_sDXEHeaderType *)pData)->nextAddress       = gbalHandle->sdioDXEConfig.TXChannel.nextAddress;

      followedPktPtr = NULL;
      status = vos_pkt_walk_packet_chain(currentPktPtr, &followedPktPtr, VOS_FALSE);
      if((VOS_STATUS_E_EMPTY == status) || (NULL == followedPktPtr))
      {
         status = VOS_STATUS_SUCCESS;
         BMSGINFO("%s: vos_pkt_walk_packet_chain returning SUCCESS\n", __func__, 0, 0);
         break;
      }
      else if(!VOS_IS_STATUS_SUCCESS(status))
      {
         BMSGERROR("%s: vos_pkt_walk_packet_chain Fail\n", __func__, 0, 0);
         break;
      }
      currentPktPtr = followedPktPtr;
   }

   BEXIT();
   return status;
}

/*----------------------------------------------------------------------------

  @brief Translate 32bit address into different 17 bit address. based SIF and
         NON-SIF address space.
         SIF address space  0x0XXXX
         NON-SIF address space 0x1YYYY + Base address

  @param v_PVOID_t pAdapter  Global adapter handle
  @param v_U32_t   inAddress input 32bit address
  @param v_U32_t  *targetAddress output targrt address
  @param v_BOOL_t  isMemory dofferenciate memory space and register space

  @return General status code
        VOS_STATUS_SUCCESS      Open Success
        VOS_STATUS_E_NOMEM      Open Fail, Resource alloc fail
        VOS_STATUS_E_FAILURE    Open Fail, Unknown reason

----------------------------------------------------------------------------*/
static VOS_STATUS balAddressTranslation
(
   v_PVOID_t   pAdapter,
   v_U32_t     inAddress,
   v_U32_t    *targetAddress,
   v_BOOL_t    isMemory
)
{
   WLANSAL_Cmd53ReqType    cmd53Req;
   v_U32_t                 baseAddress;
   v_U32_t                 targetBase;
   VOS_STATUS              status = VOS_STATUS_SUCCESS;

   VOS_ASSERT(gbalHandle);
   VOS_ASSERT(targetAddress);

   BENTER();

   baseAddress = (inAddress & WLANBAL_ADDRESS_BASE_MASK) >> WLANBAL_ADDRESS_BIT_SHIFT;
   if((0 == baseAddress) && (VOS_FALSE == isMemory))
   {
      *targetAddress = inAddress;
      BMSGINFO("%s: SIF Target address 0x%x\n", __func__, (unsigned int)*targetAddress, 0);
      BEXIT();
      return VOS_STATUS_SUCCESS;
   }

   // Check is the address translation needs to be programmed
   if(gbalHandle->currentBaseAddress != baseAddress)
   {
      targetBase             = baseAddress << WLANBAL_ADDRESS_BIT_SHIFT;
      memcpy(gbalHandle->dmaBuffer, (v_U8_t *)&targetBase, sizeof(v_U32_t));

      cmd53Req.busDirection  = WLANSAL_DIRECTION_WRITE;
      cmd53Req.callSync      = WLANSAL_CALL_SYNC;
      cmd53Req.busCompleteCB = NULL;
      cmd53Req.function      = WLANSAL_FUNCTION_ONE;
      cmd53Req.addressHandle = WLANSAL_ADDRESS_INCREMENT;
      cmd53Req.mode          = WLANSAL_MODE_BYTE;
      cmd53Req.address       = QWLAN_SIF_CSR_FIFO_MGNT_CONFIG_REG;
      cmd53Req.dataSize      = sizeof(v_U32_t);
      cmd53Req.dataPtr       = gbalHandle->dmaBuffer;
      if ((status = WLANSAL_Cmd53(pAdapter, &cmd53Req)) != VOS_STATUS_SUCCESS) {
         BMSGERROR("%s: BASE Address set fail %d", __func__, status, 0);
         BEXIT();
         return VOS_STATUS_E_FAILURE;
      }
   }

   gbalHandle->currentBaseAddress = baseAddress;
   *targetAddress = (inAddress & WLANBAL_ADDRESS_TARGET_MASK) | WLANBAL_ADDRESS_NON_SIF_MASK;

   BMSGINFO("%s: NON SIF BASE Address 0x%x, Target address 0x%x", __func__, baseAddress,
      *targetAddress);

   BEXIT();
   return VOS_STATUS_SUCCESS;
}

/*----------------------------------------------------------------------------

  @brief Request Allocate SSC context to vOSS

  @param v_U32_t   sscCtxtSize SSC Context Size
  @param v_PVOID_t *sscCtxt SSC Ctext storing pointer
  @param v_PVOID_t pAdapter  Global adapter handle

  @return General status code
        VOS_STATUS_SUCCESS      Open Success
        VOS_STATUS_E_NOMEM      Open Fail, Resource alloc fail
        VOS_STATUS_E_FAILURE    Open Fail, Unknown reason

----------------------------------------------------------------------------*/
static VOS_STATUS balSSCCtxtReqCB
(
   v_U32_t     sscCtxtSize,
   v_PVOID_t   *sscCtxt,
   v_PVOID_t   pAdapter
)
{
   VOS_STATUS    status = VOS_STATUS_SUCCESS;

   VOS_ASSERT(pAdapter);

   VOS_TRACE(VOS_MODULE_ID_BAL, VOS_TRACE_LEVEL_INFO,"%s: Enter ", __func__);
   BENTER();

   status = vos_alloc_context(pAdapter, VOS_MODULE_ID_SSC, (v_VOID_t **)sscCtxt, sscCtxtSize);
   if(!VOS_IS_STATUS_SUCCESS(status))
   {
      BMSGERROR("Invalid Handle 0x%p\n", pAdapter, 0, 0);
      BEXIT();
      return VOS_STATUS_E_INVAL;
   }

   BEXIT();
   return status;
}

/*----------------------------------------------------------------------------

  @brief Not enough resources timeout handler
         Try to reserve TX resources once again.
         If enough resources reserved, call TL Callback function

  @param v_PVOID_t pAdapter  Global adapter handle

  @return General status code
        VOS_STATUS_SUCCESS      Open Success
        VOS_STATUS_E_NOMEM      Open Fail, Resource alloc fail
        VOS_STATUS_E_FAILURE    Open Fail, Unknown reason

----------------------------------------------------------------------------*/
static v_VOID_t balGetTXResTimerExpierCB
(
   v_PVOID_t pAdapter
)
{
   VOS_STATUS     status = VOS_STATUS_SUCCESS;
   v_U32_t        availableTxBuffer;
   v_U16_t        bdBufferCount = 0;
   v_U16_t        pduBufferCount = 0;
   v_U32_t        switchBuffer;
   static v_U32_t backoffCounter = 1;

   VOS_ASSERT(gbalHandle);

   BENTER();

   status = WLANBAL_ReadRegister(pAdapter,
                                 QWLAN_SIF_BMU_BD_PDU_RSV_ALL_REG,
                                 &availableTxBuffer);

   if(!VOS_IS_STATUS_SUCCESS(status))
   {
      BMSGERROR("Invalid read reg %d, Try Again", status, 0, 0);
      BEXIT();
      return;
   }

   switchBuffer   = availableTxBuffer;
   bdBufferCount  = (v_U16_t)(switchBuffer & WLANBAL_BD_PDU_16BIT_MASK);
   pduBufferCount = (v_U16_t)((switchBuffer >> WLANBAL_PDU_BIT_SHIFT) & (WLANBAL_BD_PDU_16BIT_MASK));
   availableTxBuffer = (v_U32_t)(bdBufferCount + pduBufferCount);

   if(gbalHandle->tlReg.txResourceThreashold > (v_U32_t)(bdBufferCount + pduBufferCount))
   {
      BMSGWARN("Not enough resource try again BD %d, PDU %d, BO Counter %d", bdBufferCount, pduBufferCount, backoffCounter);
      status = vos_timer_start(&gbalHandle->timer, WLANBAL_TX_RESOURCE_TIMEOUT * backoffCounter);
      if(!VOS_IS_STATUS_SUCCESS(status))
      {
         backoffCounter = 1;
         BMSGERROR("Timer Start Fail", 0, 0, 0);
         BEXIT();
         return;
      }
      backoffCounter++;
   }
   else
   {
      gbalHandle->tlReg.txResourceCB(pAdapter, availableTxBuffer);
      backoffCounter = 1;
   }

   BEXIT();
   return;
}

/*----------------------------------------------------------------------------

  @brief Get TX frames from TL requested by SSC
         Request add and assign DXE header values

  @param v_PVOID_t pAdapter  Global adapter handle
  @param v_U32_t    maxAllowedBytes Maximum allowed bytes can be handled by SSC
  @param vos_pkt_t  **pktChainPtr Packet chain pointer storing polace
  @param v_PVOID_t  TXCompUsrData Sender User data

  @return v_BOOL_t  TRUE, there are frames have to be sent
                    FALSE, no frame left

----------------------------------------------------------------------------*/
v_BOOL_t balGetTXFramesCB
(
   v_U32_t       maxAllowedBytes,
   vos_pkt_t     **pktChainPtr,
   v_PVOID_t     pAdapter,
   v_PVOID_t     TXCompUsrData,
   v_BOOL_t*     pbUrgent
)
{
   v_BOOL_t       status    = VOS_TRUE;
   VOS_STATUS     vStatus   = VOS_STATUS_SUCCESS;

   BENTER();

#ifndef WLAN_PERF
   if(!IS_VALID_1_ARG(gbalHandle->tlReg.getTXFrameCB))
   {
      BMSGERROR("Invalid getTXFrameCB %p", gbalHandle->tlReg.getTXFrameCB, 0, 0);
      BEXIT();
      return VOS_FALSE;
   }
#endif

   status = gbalHandle->tlReg.getTXFrameCB(pAdapter, pktChainPtr, maxAllowedBytes, pbUrgent);
   // check to make sure we are not passing a NULL ptr to balHandleDXEHeader
   // This was printing a benign error inside balHandleDXEHeader. Hence adding this
   if (*pktChainPtr == NULL) {
      BMSGERROR("Warning: balGetTXFramesCB NULL pointer passed from getTxFrameCB Fail", 0, 0, 0);
      // Done no more frames to tx.
      return VOS_FALSE;
   }

   vStatus = balHandleDXEHeader(pAdapter, *pktChainPtr);
   if(!VOS_IS_STATUS_SUCCESS(vStatus))
   {
      BMSGERROR("balHandleDXEHeader Fail", 0, 0, 0);
   }

   BEXIT();
   return status;
}

/*----------------------------------------------------------------------------

  @brief Send frames complete notification from SSC to TL

  @param v_PVOID_t pAdapter  Global adapter handle
  @param VOS_STATUS    txStatus  Send status, success or fail
  @param vos_pkt_t    *pktChainPtr Packet pointer which sent, has to be frred
  @param v_PVOID_t     tlHandle  TL Handle

  @return NONE

----------------------------------------------------------------------------*/
v_VOID_t balTXCompleteCB
(
   vos_pkt_t    *pktChainPtr,
   VOS_STATUS    txStatus,
   v_PVOID_t     pAdapter,
   v_PVOID_t     tlHandle
)
{
   VOS_STATUS     status    = VOS_STATUS_SUCCESS;

   BENTER();

#ifndef WLAN_PERF
   if(!IS_VALID_1_ARG(gbalHandle->tlReg.txCompleteCB))
   {
      BMSGERROR("Invalid txCompleteCB %p", gbalHandle->tlReg.txCompleteCB, 0, 0);
      BEXIT();
      return;
   }
#endif

   status = gbalHandle->tlReg.txCompleteCB(pAdapter, pktChainPtr, txStatus);
   if(!VOS_IS_STATUS_SUCCESS(status))
   {
      BMSGERROR("TX Complete CB Fail %n", status, 0, 0);
   }

   BEXIT();
   return;
}

/*----------------------------------------------------------------------------

  @brief RX frames notification and frame route from SSC to TL

  @param v_PVOID_t pAdapter  Global adapter handle
  @param vos_pkt_t *pktChainPtr received frame pointer

  @return NONE

----------------------------------------------------------------------------*/
v_VOID_t balRecieveFramesCB
(
   vos_pkt_t    *pktChainPtr,
   v_PVOID_t     pAdapter
)
{
   VOS_STATUS     status    = VOS_STATUS_SUCCESS;

   BENTER();

   if(!IS_VALID_2_ARG(gbalHandle->tlReg.receiveFrameCB, pktChainPtr))
   {
      BMSGERROR("Invalid receiveFrameCB 0x%p ReceivedPktChain 0x%p", gbalHandle->tlReg.receiveFrameCB, pktChainPtr, 0);
      BEXIT();
      return;
   }

   status = gbalHandle->tlReg.receiveFrameCB(pAdapter, pktChainPtr);
   if(!VOS_IS_STATUS_SUCCESS(status))
   {
      BMSGERROR("RX Frame Fail %n", status, 0, 0);
   }

   return;
}

/*----------------------------------------------------------------------------

  @brief Fatal error notification from SSC to HAL

  @param v_PVOID_t pAdapter  Global adapter handle
  @param WLANSSC_ReasonCodeType reason Fatal error reason

  @return NONE

----------------------------------------------------------------------------*/
v_VOID_t balFatalErrorCB
(
   WLANSSC_ReasonCodeType reason,
   v_PVOID_t              pAdapter
)
{
   VOS_STATUS     status    = VOS_STATUS_SUCCESS;

   BENTER();

   if(!IS_VALID_1_ARG(gbalHandle->halCBacks.fatalErrorCB))
   {
      BMSGERROR("Invalid fatalErrorCB 0x%p", gbalHandle->halCBacks.fatalErrorCB, 0, 0);
      BEXIT();
      return;
   }

   status = gbalHandle->halCBacks.fatalErrorCB(pAdapter, reason);
   if(!VOS_IS_STATUS_SUCCESS(status))
   {
      BMSGERROR("Fatal Error routing Fail %n", status, 0, 0);
   }

   BEXIT();
   return;
}

/*----------------------------------------------------------------------------

  @brief  On ASIC Interrupt notification from SSC calls this function. Which
          in turn calls a HAL function to actually handle ASIC interrupts.

  @param v_PVOID_t pAdapter  Global adapter handle

  @return NONE

----------------------------------------------------------------------------*/
v_VOID_t balASICInterruptCB
(
   v_PVOID_t              pAdapter
)
{
   VOS_STATUS     status    = VOS_STATUS_SUCCESS;

   BENTER();
   if(!IS_VALID_1_ARG(gbalHandle->halCBacks.asicInterruptCB))
   {
      BMSGERROR("Invalid fatalErrorCB 0x%p", gbalHandle->halCBacks.asicInterruptCB, 0, 0);
      BEXIT();
      return;
   }

   status = gbalHandle->halCBacks.asicInterruptCB(pAdapter);
   if(!VOS_IS_STATUS_SUCCESS(status))
   {
      BMSGERROR("ASIC Interrupt routing Fail %n", status, 0, 0);
   }

   BEXIT();
   return;
}
/*=========================================================================
 * Interactions with vOSS
 *=========================================================================*/

/*----------------------------------------------------------------------------

  @brief During open process, resources are needed have to be allocated and
        internal structure have to be initialized. And have to open trigger
        SAL and SSC module

  @param v_PVOID_t pAdapter
        Global adapter handle

  @return General status code
        VOS_STATUS_SUCCESS      Open Success
        VOS_STATUS_E_NOMEM      Open Fail, Resource alloc fail
        VOS_STATUS_E_FAILURE    Open Fail, Unknown reason


----------------------------------------------------------------------------*/
VOS_STATUS WLANBAL_Open
(
   v_PVOID_t pAdapter
)
{
   VOS_STATUS     status    = VOS_STATUS_SUCCESS;
   v_PVOID_t      dummyPtr  = NULL;

   BENTER();

   VOS_ASSERT(pAdapter);

   status = vos_alloc_context(pAdapter, VOS_MODULE_ID_BAL, (v_VOID_t **)&gbalHandle, sizeof(balHandleType));
   if(!VOS_IS_STATUS_SUCCESS(status))
   {
      BMSGERROR("vOSS Resource allocation Fail", 0, 0, 0);
      BEXIT();
      return VOS_STATUS_E_NOMEM;
   }

   gbalHandle->pAdapter = pAdapter;
   status = vos_timer_init(&gbalHandle->timer, VOS_TIMER_TYPE_SW, balGetTXResTimerExpierCB, pAdapter);
   if(!VOS_IS_STATUS_SUCCESS(status))
   {
      BMSGERROR("BAL Timer init fail %d", status, 0, 0);
      BEXIT();
      return VOS_STATUS_E_FAILURE;
   }

   spin_lock_init(&gbalHandle->spinlock);

   status = WLANSSC_Open(pAdapter, &dummyPtr, balSSCCtxtReqCB, pAdapter);
   if(!VOS_IS_STATUS_SUCCESS(status))
   {
      BMSGERROR("SSC Open fail%d", status, 0, 0);
      BEXIT();
      return VOS_STATUS_E_FAILURE;
   }

   gbalHandle->dmaBuffer = (v_U8_t *)vos_mem_dma_malloc(WLANBAL_DMA_MAX_BUFFER_SIZE);
   if(NULL == gbalHandle->dmaBuffer)
   {
      BMSGERROR("Physical memory allock Fail", 0, 0, 0);
      BEXIT();
      return VOS_STATUS_E_NOMEM;
   }
   memset(gbalHandle->dmaBuffer, 0, WLANBAL_DMA_MAX_BUFFER_SIZE);

   BMSGINFO("%s: New Complete %x\n", __func__, (unsigned int)gbalHandle, 0);
   BEXIT();
   return VOS_STATUS_SUCCESS;
}

#ifdef VOLANS_1_0_WORKAROUND
VOS_STATUS WLANBAL_DisableClkGate
(
   v_PVOID_t pAdapter
)
{

  tANI_U32 uRegVal = 0;
  
  WLANBAL_ReadRegister(pAdapter, QWLAN_SIF_BAR4_WLAN_PWR_SAVE_CONTROL_REG_REG, &SpModeRegVal[0]);
  WLANBAL_ReadRegister(pAdapter, QWLAN_PMU_WMAC_SYS_CLKGATE_DISABLE_REG_REG, &SpModeRegVal[1]);
  WLANBAL_ReadRegister(pAdapter, QWLAN_SIF_BAR4_SIF_CLK_GATING_CTRL_REG_REG, &SpModeRegVal[2]);
  WLANBAL_ReadRegister(pAdapter, QWLAN_SIF_BAR4_PMU_CLK_CTRL_REG_REG, &SpModeRegVal[3]);

  uRegVal = (SpModeRegVal[0] | (QWLAN_SIF_BAR4_WLAN_PWR_SAVE_CONTROL_REG_WLAN_GLOBAL_CLK_GATE_DISABLE_MASK));
  WLANBAL_WriteRegister(pAdapter, QWLAN_SIF_BAR4_WLAN_PWR_SAVE_CONTROL_REG_REG, uRegVal);

  uRegVal = (SpModeRegVal[1] | (QWLAN_PMU_WMAC_SYS_CLKGATE_DISABLE_REG_PMU_CLKGATE_DISABLE_VECTOR_MASK |
                                QWLAN_PMU_WMAC_SYS_CLKGATE_DISABLE_REG_PMU_MIF_TEST_CLK_GATE_DISABLE_MASK |
                                QWLAN_PMU_WMAC_SYS_CLKGATE_DISABLE_REG_PMU_MIF_FUNC_CLK_GATE_DISABLE_MASK |
                                QWLAN_PMU_WMAC_SYS_CLKGATE_DISABLE_REG_PMU_MCU_GAS_CLKGATE_DISABLE_MASK |
                                QWLAN_PMU_WMAC_SYS_CLKGATE_DISABLE_REG_PMU_MCU_CLKGATE_DISABLE_MASK));
  WLANBAL_WriteRegister(pAdapter, QWLAN_PMU_WMAC_SYS_CLKGATE_DISABLE_REG_REG, uRegVal);

  uRegVal = (SpModeRegVal[2] | (QWLAN_SIF_BAR4_SIF_CLK_GATING_CTRL_REG_SIF_RXF_CLK_GATING_DISABLE_MASK |
                                QWLAN_SIF_BAR4_SIF_CLK_GATING_CTRL_REG_SIF_TXF_CLK_GATING_DISABLE_MASK |
                                QWLAN_SIF_BAR4_SIF_CLK_GATING_CTRL_REG_SIF_BAR4_SIF_DISABLE_CLK_GATING_MASK));
  WLANBAL_WriteRegister(pAdapter, QWLAN_SIF_BAR4_SIF_CLK_GATING_CTRL_REG_REG, uRegVal);
 
  uRegVal =  (SpModeRegVal[3] | (QWLAN_SIF_BAR4_PMU_CLK_CTRL_REG_SIF_BAR4_SYS_ENABLE_SIF_CLK_MASK |
                                 QWLAN_SIF_BAR4_PMU_CLK_CTRL_REG_SIF_BAR4_PMU_CONFIG_CLK_GATE_DISABLE_MASK |
                                 QWLAN_SIF_BAR4_PMU_CLK_CTRL_REG_SIF_PMU_GAS_CLK_GATE_DISABLE_MASK |
                                 QWLAN_SIF_BAR4_PMU_CLK_CTRL_REG_SIF_BAR4_ENABLE_PMU_CLK_MASK)); 
  WLANBAL_WriteRegister(pAdapter, QWLAN_SIF_BAR4_PMU_CLK_CTRL_REG_REG, uRegVal);

  return VOS_STATUS_SUCCESS;
}

VOS_STATUS WLANBAL_SoftReset
(
   v_PVOID_t pAdapter
)
{
  tANI_U32 uRegVal1 = 0, uRegVal2=0;

  WLANBAL_ReadRegister(pAdapter, QWLAN_SIF_BAR4_MRCM_OVERRIDE_SIF_SOFT_RESET_REG_REG, &uRegVal1);
  uRegVal2 = uRegVal1 | (QWLAN_SIF_BAR4_MRCM_OVERRIDE_SIF_SOFT_RESET_REG_SIF_MRCM_SYNC_RST_CLK_EN_OVERRIDE_MASK);
  WLANBAL_WriteRegister(pAdapter, QWLAN_SIF_BAR4_MRCM_OVERRIDE_SIF_SOFT_RESET_REG_REG, uRegVal2);

  WLANBAL_ReadRegister(pAdapter, QWLAN_SIF_BAR4_WLAN_CONTROL_REG_REG, &uRegVal2);
  uRegVal2 &= ~(QWLAN_SIF_BAR4_WLAN_CONTROL_REG_WLAN_SOFT_RESET_N_MASK);
  WLANBAL_WriteRegister(pAdapter, QWLAN_SIF_BAR4_WLAN_CONTROL_REG_REG, uRegVal2);

  vos_sleep(2);
  
  WLANBAL_WriteRegister(pAdapter, QWLAN_SIF_BAR4_MRCM_OVERRIDE_SIF_SOFT_RESET_REG_REG, uRegVal1);
  uRegVal2 |= (QWLAN_SIF_BAR4_WLAN_CONTROL_REG_WLAN_SOFT_RESET_N_MASK);
  WLANBAL_WriteRegister(pAdapter, QWLAN_SIF_BAR4_WLAN_CONTROL_REG_REG, uRegVal2);
  
  return VOS_STATUS_SUCCESS;
}

VOS_STATUS WLANBAL_EnableClkGate
(
   v_PVOID_t pAdapter
)
{
  tANI_U32 uRegVal = 0;

  uRegVal = (SpModeRegVal[0] & ~(QWLAN_SIF_BAR4_WLAN_PWR_SAVE_CONTROL_REG_WLAN_GLOBAL_CLK_GATE_DISABLE_MASK));
  WLANBAL_WriteRegister(pAdapter, QWLAN_SIF_BAR4_WLAN_PWR_SAVE_CONTROL_REG_REG, uRegVal);

  uRegVal = (SpModeRegVal[1] & ~( QWLAN_PMU_WMAC_SYS_CLKGATE_DISABLE_REG_PMU_CLKGATE_DISABLE_VECTOR_MASK |
                                  QWLAN_PMU_WMAC_SYS_CLKGATE_DISABLE_REG_PMU_MIF_TEST_CLK_GATE_DISABLE_MASK |
                                  QWLAN_PMU_WMAC_SYS_CLKGATE_DISABLE_REG_PMU_MIF_FUNC_CLK_GATE_DISABLE_MASK |
                                  QWLAN_PMU_WMAC_SYS_CLKGATE_DISABLE_REG_PMU_MCU_GAS_CLKGATE_DISABLE_MASK |
                                  QWLAN_PMU_WMAC_SYS_CLKGATE_DISABLE_REG_PMU_MCU_CLKGATE_DISABLE_MASK));
  WLANBAL_WriteRegister(pAdapter, QWLAN_PMU_WMAC_SYS_CLKGATE_DISABLE_REG_REG, uRegVal);

  WLANBAL_WriteRegister(pAdapter, QWLAN_SIF_BAR4_SIF_CLK_GATING_CTRL_REG_REG, SpModeRegVal[2]);

  WLANBAL_WriteRegister(pAdapter, QWLAN_SIF_BAR4_PMU_CLK_CTRL_REG_REG, SpModeRegVal[3]);

  return VOS_STATUS_SUCCESS;
}

#endif /* VOLANS_1_0_WORKAROUND */
  


/*----------------------------------------------------------------------------

  @brief Start BAL module and start trigger SAL and SSC.
        If BAL is not open return fail

  @param v_PVOID_t pAdapter
        Global adapter handle

  @return General status code
        VOS_STATUS_SUCCESS      Start Success
        VOS_STATUS_E_FAILURE    Start Fail, BAL Not open yet


----------------------------------------------------------------------------*/
VOS_STATUS WLANBAL_Start
(
   v_PVOID_t pAdapter
)
{
   WLANSSC_HandleType       sscHandle = (WLANSSC_HandleType)VOS_GET_SSC_CTXT(pAdapter);
   WLANSSC_StartParamsType  sscReg;
   VOS_STATUS               status    = VOS_STATUS_SUCCESS;
#if defined (VOLANS_BB) || defined(VOLANS_RF)
#ifdef VOLANS_1_0_WORKAROUND
   tANI_U32                 uRegVal=0;
#endif
#endif

   BENTER();

   if(!IS_VALID_1_ARG(gbalHandle))
   {
      BMSGERROR("Invalid BAL Handle %p", gbalHandle, 0, 0);
      BEXIT();
      return VOS_STATUS_E_NOMEM;
   }
   if(!IS_VALID_1_ARG(sscHandle))
   {
      BMSGERROR("Invalid SSC Handle %p", gbalHandle, 0, 0);
      BEXIT();
      return VOS_STATUS_E_NOMEM;
   }


   sscReg.pfnGetMultipleTxPacketCback           = balGetTXFramesCB;
   sscReg.pfnTxCompleteCback                    = balTXCompleteCB;
   sscReg.pfnRxPacketHandlerCback               = balRecieveFramesCB;
   sscReg.pfnASICInterruptIndicationCback       = balASICInterruptCB;
   sscReg.pfnFatalErrorIndicationCback          = balFatalErrorCB;


#if defined (VOLANS_BB) || defined(VOLANS_RF)
#ifdef VOLANS_1_0_WORKAROUND
   /* Lowering the SD clock frequency is necessary before accessing
      the below BBPLL registers. This is a workaround for the bug in 
      Volans ASIC */
   WLANSAL_SetSDIOClock(WLAN_LOW_SD_CLOCK_FREQ); 
   vos_sleep(30);
#endif /* VOLANS_1_0_WORKAROUND */
#endif /* VOLANS_BB */

   status = WLANSSC_Start(sscHandle, &sscReg, pAdapter);
   if(!VOS_IS_STATUS_SUCCESS(status))
   {
      BEXIT();
      return VOS_STATUS_E_FAILURE;
   }

   /**
    * FIXME
    * The following change MUST be removed as soon as hardware fix is in place
    */

   /* The changes for PMU_ANA_PLL1_CONFIG_REG is temporary for Volans-ASIC.
    * Currently, the chip has some issues with BBPLL, hence adding a work-around
    * to restore the same. Should be removed when proper fix inside the chip
    * is available. This configuration needs to be done one time when the driver
    * loads as soon as IO_READY is received from the SIF
    */
#if defined (VOLANS_BB) || defined(VOLANS_RF)
#ifdef VOLANS_1_0_WORKAROUND
   /* The sleep is necessary for the driver to load. Without this delay,
    * driver won't load.
    */
   vos_sleep(50);

   /* pmu register configuration for BBPLL restoration */
   status = WLANBAL_ReadRegister(pAdapter, QWLAN_PMU_PMU_ANA_PLL1_CONFIG_REG, &uRegVal);
   if(!VOS_IS_STATUS_SUCCESS(status))
   {
      BMSGERROR("PMU REGISTER QWLAN_PMU_PMU_ANA_PLL1_CONFIG_REG READ FAILED", 0, 0, 0);
      BEXIT();
      return VOS_STATUS_E_FAILURE;
   }

   /* No bits defined, hence using the raw bits */	
   uRegVal &= (~0x3);
   uRegVal |= 0x3;

   status = WLANBAL_WriteRegister(pAdapter, QWLAN_PMU_PMU_ANA_PLL1_CONFIG_REG, uRegVal);
   if(!VOS_IS_STATUS_SUCCESS(status))
   {
      BMSGERROR("PMU REGISTER QWLAN_PMU_PMU_ANA_PLL1_CONFIG_REG WRITE FAILED", 0, 0, 0);
      BEXIT();
      return VOS_STATUS_E_FAILURE;
   }

   /**
    * Increasing the delay from 5ms to 20ms. This value has been verified by VI/simulation 
	* team. Before configuring this register, all transactions happen at 19.2 MHz, after
	* writing to above register, PLL frequency is fixed and starts generating the frequency
    * at 80 MHz. Currently, it is expected to take anywhere between 5ms to tens of milliseconds.
    * VI seems to have found 20ms works. Hence changing the delay from 5ms to 20ms
    **/	
   vos_sleep(20);

   /* Reset work around */
   WLANBAL_DisableClkGate(pAdapter);
   WLANBAL_SoftReset(pAdapter);
   WLANBAL_EnableClkGate(pAdapter);

	vos_sleep(5);

   /* Increase the clock frequency back to higher value for performance */
   WLANSAL_SetSDIOClock(WLAN_HIGH_SD_CLOCK_FREQ);
   vos_sleep(15);

	
#endif /* VOLANS_1_0_WORKAROUND */
#endif /* VOLANS_BB */
   
   /* DXE Header CFG set as default */
   gbalHandle->sdioDXEConfig.TXChannel.shortDescriptor    = VOS_TRUE;
   gbalHandle->sdioDXEConfig.TXChannel.descriptorControl  = WLANBAL_DXE_TX_DESC_CTRL;
   gbalHandle->sdioDXEConfig.TXChannel.sourceAddress      = WLANBAL_DXE_TX_SRC_ADDRESS;
   gbalHandle->sdioDXEConfig.TXChannel.destinationAddress = WLANBAL_DXE_TX_DEST_ADDRESS;
   gbalHandle->sdioDXEConfig.TXChannel.nextAddress        = WLANBAL_DXE_TX_NEXT_ADDRESS;
   SWAP_ENDIAN_UINT32(gbalHandle->sdioDXEConfig.TXChannel.descriptorControl);
   SWAP_ENDIAN_UINT32(gbalHandle->sdioDXEConfig.TXChannel.sourceAddress);
   SWAP_ENDIAN_UINT32(gbalHandle->sdioDXEConfig.TXChannel.destinationAddress);
   SWAP_ENDIAN_UINT32(gbalHandle->sdioDXEConfig.TXChannel.nextAddress);

   gbalHandle->sdioDXEConfig.RXChannel.shortDescriptor    = VOS_TRUE;
   gbalHandle->sdioDXEConfig.RXChannel.descriptorControl  = WLANBAL_DXE_RX_DESC_CTRL;
   gbalHandle->sdioDXEConfig.RXChannel.sourceAddress      = WLANBAL_DXE_RX_SRC_ADDRESS;
   gbalHandle->sdioDXEConfig.RXChannel.destinationAddress = WLANBAL_DXE_RX_DEST_ADDRESS;
   gbalHandle->sdioDXEConfig.RXChannel.nextAddress        = (v_U32_t)(&(gbalHandle->sdioDXEConfig));
   SWAP_ENDIAN_UINT32(gbalHandle->sdioDXEConfig.RXChannel.descriptorControl);
   SWAP_ENDIAN_UINT32(gbalHandle->sdioDXEConfig.RXChannel.sourceAddress);
   SWAP_ENDIAN_UINT32(gbalHandle->sdioDXEConfig.RXChannel.destinationAddress);
   SWAP_ENDIAN_UINT32(gbalHandle->sdioDXEConfig.RXChannel.nextAddress);

#ifdef ANI_CHIPSET_VOLANS
	gbalHandle->sdioDXEConfig.RXHiChannel.shortDescriptor    = VOS_TRUE;
	gbalHandle->sdioDXEConfig.RXHiChannel.descriptorControl  = WLANBAL_DXE_RX_DESC_CTRL;
	gbalHandle->sdioDXEConfig.RXHiChannel.sourceAddress	   = WLANBAL_DXE_RX_HI_SRC_ADDRESS;
	gbalHandle->sdioDXEConfig.RXHiChannel.destinationAddress = WLANBAL_DXE_RX_HI_DEST_ADDRESS;
	gbalHandle->sdioDXEConfig.RXHiChannel.nextAddress 	   = (v_U32_t)(&(gbalHandle->sdioDXEConfig));
	SWAP_ENDIAN_UINT32(gbalHandle->sdioDXEConfig.RXHiChannel.descriptorControl);
	SWAP_ENDIAN_UINT32(gbalHandle->sdioDXEConfig.RXHiChannel.sourceAddress);
	SWAP_ENDIAN_UINT32(gbalHandle->sdioDXEConfig.RXHiChannel.destinationAddress);
	SWAP_ENDIAN_UINT32(gbalHandle->sdioDXEConfig.RXHiChannel.nextAddress);
#endif

   status = WLANBAL_WriteRegister(pAdapter, QWLAN_SIF_BMU_CMD_CFG_REG, QWLAN_HAL_DXE0_MASTERID);
   if(!VOS_IS_STATUS_SUCCESS(status))
   {
      BMSGERROR("BMU Config Fail BAL Start Fail", 0, 0, 0);
      BEXIT();
      return VOS_STATUS_E_FAILURE;
   }

   BEXIT();
   return status;
}

/*----------------------------------------------------------------------------

  @brief Stop BAL module. Initialize BAL internal structure.
        Trigger to stop SAL and SSC

  @param  v_PVOID_t pAdapter
        Global adapter handle

  @return General status code
        VOS_STATUS_SUCCESS      Stop Success
        VOS_STATUS_E_FAILURE    Stop Fail, BAL not started

----------------------------------------------------------------------------*/
VOS_STATUS WLANBAL_Stop
(
   v_PVOID_t pAdapter
)
{
   v_PVOID_t  sscHandle = (v_PVOID_t)VOS_GET_SSC_CTXT(pAdapter);
   VOS_STATUS status = VOS_STATUS_SUCCESS;

   BENTER();

   if (VOS_TIMER_STATE_RUNNING == vos_timer_getCurrentState(&gbalHandle->timer))
   {
      status = vos_timer_stop(&gbalHandle->timer);
   }

   if (VOS_STATUS_SUCCESS != status)
   {
      BMSGERROR("BAL timer stop failed.", 0, 0, 0);
   }

   status = WLANSSC_Stop(sscHandle);

   BEXIT();
   return status;
}

/*----------------------------------------------------------------------------

  @brief Close BAL module. Free internal resources.
        Trigger to close SAL and SSC

  @param v_PVOID_t pAdapter
        Global adapter handle

  @return General status code
        VOS_STATUS_SUCCESS      Close Success
        VOS_STATUS_E_FAILURE    Close Fail, BAL not open

----------------------------------------------------------------------------*/
VOS_STATUS WLANBAL_Close
(
   v_PVOID_t pAdapter
)
{
   v_PVOID_t          sscHandle = (v_PVOID_t)VOS_GET_SSC_CTXT(pAdapter);
   VOS_STATUS         status = VOS_STATUS_SUCCESS;

   BENTER();

   vos_mem_dma_free(gbalHandle->dmaBuffer);

   status = WLANSSC_Close(sscHandle);
   if(!VOS_IS_STATUS_SUCCESS(status))
   {
      BMSGERROR("SSC Close Fail", 0, 0, 0);
   }

   status = vos_timer_destroy(&gbalHandle->timer);
   if(!VOS_IS_STATUS_SUCCESS(status))
   {
      BMSGERROR("BAL Timer destroy fail", 0, 0, 0);
   }

   status = vos_free_context(pAdapter, VOS_MODULE_ID_BAL, (v_PVOID_t)gbalHandle);
   if(!VOS_IS_STATUS_SUCCESS(status))
   {
      BMSGERROR("BAL Context Free fail", 0, 0, 0);
   }

   status = vos_free_context(pAdapter, VOS_MODULE_ID_SSC, sscHandle);
   if(!VOS_IS_STATUS_SUCCESS(status))
   {
      BMSGERROR("SSC Context Free fail", 0, 0, 0);
   }

   BEXIT();
   return status;
}


/*----------------------------------------------------------------------------

  @brief Reset BAL. Start and Stop BAL, which in turn starts and stops SSC.

  @param v_PVOID_t pAdapter
        Global adapter handle

  @return General status code

----------------------------------------------------------------------------*/
VOS_STATUS WLANBAL_Reset
(
   v_PVOID_t pAdapter
)
{
   VOS_STATUS        status = VOS_STATUS_SUCCESS;

   BENTER();

   status = WLANBAL_Stop(pAdapter);
   if(!VOS_IS_STATUS_SUCCESS(status))
   {
      BMSGERROR("BAL Stop Fail", 0, 0, 0);
      BEXIT();
      return VOS_STATUS_E_FAILURE;
   }

   VOS_TRACE(VOS_MODULE_ID_BAL, VOS_TRACE_LEVEL_INFO,"%s: Reset bal stop done\n", __func__);
   status = WLANBAL_Start(pAdapter);
   if(!VOS_IS_STATUS_SUCCESS(status))
   {
      BMSGERROR("BAL Re Start Fail", 0, 0, 0);
      BEXIT();
      return VOS_STATUS_E_FAILURE;
   }

   VOS_TRACE(VOS_MODULE_ID_BAL, VOS_TRACE_LEVEL_INFO,"%s: Reset bal start done\n", __func__);

   BEXIT();
   return VOS_STATUS_SUCCESS;
}

/*=========================================================================
 * END Interactions with vOSS
 *=========================================================================*/

/*=========================================================================
 * Interactions with HAL
 *=========================================================================*/

/*----------------------------------------------------------------------------

  @brief Register HAL Callback functions to BAL.
        Registration functions are Interrupt happen notification function
        and fatal error happen notification function

  @param v_PVOID_t pAdapter
        Global adapter handle

  @param WLANBAL_HalRegType
        HAL Registration elements.
        ASIC interrupt callback function, Fatal error notification function
        and HAL internal data have to be registered.

  @return General status code
        VOS_STATUS_SUCCESS       Registration success
        VOS_STATUS_E_RESOURCES   BAL resources are not ready
        VOS_STATUS_E_INVAL       Invalid argument

----------------------------------------------------------------------------*/
VOS_STATUS WLANBAL_RegHalCBFunctions
(
   v_PVOID_t           pAdapter,
   WLANBAL_HalRegType *halReg
)
{
   BENTER();

   if(!IS_VALID_2_ARG(gbalHandle, halReg))
   {
      BMSGERROR("Invalid BAL Args gbalHandle %p, halReg %p", gbalHandle, halReg, 0);
      BEXIT();
      return VOS_STATUS_E_INVAL;
   }

   gbalHandle->halCBacks.asicInterruptCB = halReg->asicInterruptCB;
   gbalHandle->halCBacks.fatalErrorCB    = halReg->fatalErrorCB;

   BEXIT();
   return VOS_STATUS_SUCCESS;
}


/*----------------------------------------------------------------------------

  @brief Read register value from WLAN hardware
        Every register size is 4 byte

  @param v_PVOID_t pAdapter
        Global adapter handle

  @param v_U32_t regAddress
        Register address have to be read

  @param v_U32_t *bufferPtr
        Buffer pointer will store data

  @return General status code
        VOS_STATUS_SUCCESS       Read success
        VOS_STATUS_E_INVAL       bufferPtr is not valid
        VOS_STATUS_E_FAILURE     BAL is not ready

----------------------------------------------------------------------------*/
VOS_STATUS WLANBAL_ReadRegister
(
   v_PVOID_t  pAdapter,
   v_U32_t    regAddress,
   v_U32_t   *bufferPtr
)
{
   WLANSAL_Cmd53ReqType   cmd53;
   VOS_STATUS             status     = VOS_STATUS_SUCCESS;
   v_U32_t                targetAddress;

   BENTER();

   if(!IS_VALID_2_ARG(gbalHandle, bufferPtr))
   {
      BMSGERROR("Invalid BAL Args gbalHandle %p, bufferPtr %p", gbalHandle, bufferPtr, 0);
      BEXIT();
      return VOS_STATUS_E_INVAL;
   }

   *bufferPtr = 0;

   // Get the lock
   sd_claim_host(gbalHandle->sdio_func_dev);


   status = balAddressTranslation(pAdapter, regAddress, &targetAddress, VOS_FALSE);
   if(!VOS_IS_STATUS_SUCCESS(status))
   {
      BMSGERROR("Address Translation Fail", 0, 0, 0);
      // Release lock
      sd_release_host(gbalHandle->sdio_func_dev);
      BEXIT();
      return VOS_STATUS_E_FAILURE;
   }


   cmd53.busDirection  = WLANSAL_DIRECTION_READ;
   cmd53.callSync      = WLANSAL_CALL_SYNC;
   cmd53.busCompleteCB = NULL;
   cmd53.function      = WLANSAL_FUNCTION_ONE;
   cmd53.address       = targetAddress;
   cmd53.dataSize      = WLANBAL_ASIC_REGISTER_SIZE;
   cmd53.dataPtr       = gbalHandle->dmaBuffer;
   cmd53.addressHandle = WLANSAL_ADDRESS_INCREMENT;
   cmd53.addressHandle = WLANSAL_ADDRESS_INCREMENT;
   cmd53.mode          = WLANSAL_MODE_BYTE;
   status = WLANSAL_Cmd53(pAdapter, &cmd53);
   if(!VOS_IS_STATUS_SUCCESS(status))
   {
      BMSGERROR("Read Register fail %d", status, 0, 0);
      // Release lock
      sd_release_host(gbalHandle->sdio_func_dev);
      BEXIT();
      return status;
   }

   memcpy((v_U8_t *)bufferPtr, gbalHandle->dmaBuffer, WLANBAL_ASIC_REGISTER_SIZE);
   memset(gbalHandle->dmaBuffer, 0, WLANBAL_ASIC_REGISTER_SIZE);

   // Release lock
   sd_release_host(gbalHandle->sdio_func_dev);
   BEXIT();
   return status;
}


/*----------------------------------------------------------------------------

  @brief Read Multiple registers value from WLAN hardware
         HAL Has to give enough buffer space and
         Registers addresses have to be successful
         Every register size is 4 byte

  @param v_PVOID_t pAdapter
        Global adapter handle

  @param v_U32_t regAddress
        Register address have to be read

  @param v_U32_t *bufferPtr
        Buffer pointer will store data

  @param v_U32_t numRegisters
        Count of Registers have to be read

  @return General status code
        VOS_STATUS_SUCCESS       Read success
        VOS_STATUS_E_INVAL       bufferPtr is not valid
        VOS_STATUS_E_FAILURE     BAL is not ready

----------------------------------------------------------------------------*/
VOS_STATUS WLANBAL_ReadMultipleRegisters
(
   v_PVOID_t  pAdapter,
   v_U32_t    regAddress,
   v_U32_t   *bufferPtr,
   v_U32_t    numRegisters
)
{
   WLANSAL_Cmd53ReqType   cmd53;
   VOS_STATUS             status     = VOS_STATUS_SUCCESS;
   v_U32_t                targetAddress;
   v_U32_t                idx;

   VOS_ASSERT(gbalHandle);
   VOS_ASSERT(bufferPtr);
   VOS_ASSERT(numRegisters);
   VOS_ASSERT((WLANBAL_ASIC_REGISTER_SIZE * numRegisters) <= WLANBAL_DMA_MAX_BUFFER_SIZE);

   BENTER();

   if(0 == numRegisters)
   {
      BMSGERROR("Invalid number of register, 0 Reg count is not supported", 0, 0, 0);
      BEXIT();
      return VOS_STATUS_E_INVAL;
   }

   if(WLANBAL_ASIC_REGISTER_SIZE * numRegisters > WLANBAL_DMA_MAX_BUFFER_SIZE)
   {
      BMSGERROR("%s: Invalid number of registers, MAX buffer size is %d",
         __func__, WLANBAL_DMA_MAX_BUFFER_SIZE, 0);
      BEXIT();
      return VOS_STATUS_E_INVAL;
   }

   // Get the lock, going native
   sd_claim_host(gbalHandle->sdio_func_dev);

   status = balAddressTranslation(pAdapter, regAddress, &targetAddress, VOS_FALSE);
   if(!VOS_IS_STATUS_SUCCESS(status))
   {
      BMSGERROR("Address Translation Fail", 0, 0, 0);
      // Release lock
      sd_release_host(gbalHandle->sdio_func_dev);
      BEXIT();
      return VOS_STATUS_E_FAILURE;
   }

   cmd53.busDirection  = WLANSAL_DIRECTION_READ;
   cmd53.callSync      = WLANSAL_CALL_SYNC;
   cmd53.busCompleteCB = NULL;
   cmd53.function      = WLANSAL_FUNCTION_ONE;
   cmd53.address       = targetAddress;
   cmd53.dataSize      = WLANBAL_ASIC_REGISTER_SIZE;
   cmd53.dataPtr       = gbalHandle->dmaBuffer;
   cmd53.addressHandle = WLANSAL_ADDRESS_INCREMENT;
   cmd53.mode          = WLANSAL_MODE_BYTE;

   for(idx = 0; idx < numRegisters; idx++)
   {
      status = WLANSAL_Cmd53(pAdapter, &cmd53);
      if(!VOS_IS_STATUS_SUCCESS(status))
      {
         BMSGERROR("Read Multi Register fail %d", status, 0, 0);
         // Release lock
         sd_release_host(gbalHandle->sdio_func_dev);
         BEXIT();
         return status;
      }
      cmd53.address += WLANBAL_ASIC_REGISTER_SIZE;
      cmd53.dataPtr += WLANBAL_ASIC_REGISTER_SIZE;
   }

   memcpy((v_U8_t *)bufferPtr, gbalHandle->dmaBuffer, WLANBAL_ASIC_REGISTER_SIZE * numRegisters);
   memset(gbalHandle->dmaBuffer, 0, WLANBAL_ASIC_REGISTER_SIZE * numRegisters);
   // Release lock
   sd_release_host(gbalHandle->sdio_func_dev);
   BEXIT();
   return status;
}


/*----------------------------------------------------------------------------

  @brief Write register value from WLAN hardware
        Every register size is 4 byte

  @param v_PVOID_t pAdapter
        Global adapter handle

  @param v_U32_t regAddress
        Register address has to be write

  @param v_U32_t regData
        Data value will be written

  @return General status code
        VOS_STATUS_SUCCESS       Write success
        VOS_STATUS_E_FAILURE     BAL is not ready

----------------------------------------------------------------------------*/
VOS_STATUS WLANBAL_WriteRegister
(
   v_PVOID_t  pAdapter,
   v_U32_t    regAddress,
   v_U32_t    regData
)
{
   WLANSAL_Cmd53ReqType   cmd53;
   VOS_STATUS             status     = VOS_STATUS_SUCCESS;
   v_U32_t                writeData  = regData;
   v_U32_t                targetAddress;

   BENTER();

   if(!IS_VALID_1_ARG(gbalHandle))
   {
      BMSGERROR("Invalid BAL Args gbalHandle %p", gbalHandle, 0, 0);
      BEXIT();
      return VOS_STATUS_E_INVAL;
   }

   // Get the lock, going native
   sd_claim_host(gbalHandle->sdio_func_dev);

   status = balAddressTranslation(pAdapter, regAddress, &targetAddress, VOS_FALSE);
   if(!VOS_IS_STATUS_SUCCESS(status))
   {
      BMSGERROR("Address Translation Fail", 0, 0, 0);
      // Release lock
      sd_release_host(gbalHandle->sdio_func_dev);
      BEXIT();
      return VOS_STATUS_E_FAILURE;
   }


   memcpy(gbalHandle->dmaBuffer, (v_U8_t *)&writeData, WLANBAL_ASIC_REGISTER_SIZE);
   cmd53.busDirection  = WLANSAL_DIRECTION_WRITE;
   cmd53.callSync      = WLANSAL_CALL_SYNC;
   cmd53.busCompleteCB = NULL;
   cmd53.function      = WLANSAL_FUNCTION_ONE;
   cmd53.address       = targetAddress;
   cmd53.dataSize      = WLANBAL_ASIC_REGISTER_SIZE;
   cmd53.dataPtr       = (v_U8_t *)&writeData;
   cmd53.addressHandle = WLANSAL_ADDRESS_INCREMENT;
   cmd53.addressHandle = WLANSAL_ADDRESS_FIXED;
   cmd53.mode          = WLANSAL_MODE_BYTE;

   status = WLANSAL_Cmd53(pAdapter, &cmd53);
   if(!VOS_IS_STATUS_SUCCESS(status))
   {
      BMSGERROR("Write Register fail %d", status, 0, 0);
      // Release lock
      sd_release_host(gbalHandle->sdio_func_dev);
      BEXIT();
      return status;
   }

   memset(gbalHandle->dmaBuffer, 0, WLANBAL_ASIC_REGISTER_SIZE);

   // Release lock
   sd_release_host(gbalHandle->sdio_func_dev);
   BEXIT();
   return status;

}

/*----------------------------------------------------------------------------

  @brief Read memory from WLAN hardware

  @param v_PVOID_t pAdapter
        Global adapter handle

  @param v_U32_t memAddress
        v_U8_t length

  @param v_PVOID_t *bufferPtr

  @return General status code

----------------------------------------------------------------------------*/
static VOS_STATUS WLANBAL_ReadDeviceMemory
(
   v_PVOID_t  pAdapter,
   v_U32_t    memAddress,
   v_PVOID_t  bufferPtr,
   v_U32_t    length
)
{
   WLANSAL_Cmd53ReqType   cmd53;
   VOS_STATUS             status    = VOS_STATUS_SUCCESS;
   v_U32_t                targetAddress;
   v_U32_t                currentLength = 0;

   BENTER();

   if(!IS_VALID_2_ARG(gbalHandle, bufferPtr))
   {
      BMSGERROR( "Invalid BAL Args gbalHandle 0x%p, bufferPtr 0x%p", gbalHandle, bufferPtr, 0);
      BEXIT();
      return VOS_STATUS_E_INVAL;
   }

   if(0 == length)
   {
      BMSGERROR("Invalid Length ZERO", 0, 0, 0);
      BEXIT();
      return VOS_STATUS_E_INVAL;
   }

   // Get the lock, going native
   sd_claim_host(gbalHandle->sdio_func_dev);

   cmd53.busDirection  = WLANSAL_DIRECTION_READ;
   cmd53.callSync      = WLANSAL_CALL_SYNC;
   cmd53.busCompleteCB = NULL;
   cmd53.function      = WLANSAL_FUNCTION_ONE;
   cmd53.addressHandle = WLANSAL_ADDRESS_INCREMENT;

   status = balAddressTranslation(pAdapter, memAddress, &targetAddress, VOS_TRUE);
   if(!VOS_IS_STATUS_SUCCESS(status))
   {
      BMSGERROR("Address Translation Fail", 0, 0 ,0);
      // Release lock
      sd_release_host(gbalHandle->sdio_func_dev);
      BEXIT();
      return VOS_STATUS_E_FAILURE;
   }

   while (length > 0)
   {
       if (length > WLANBAL_DMA_MAX_BUFFER_SIZE)
           currentLength = WLANBAL_DMA_MAX_BUFFER_SIZE;
       else
           currentLength = length;
       
       cmd53.address       = targetAddress;
       cmd53.dataSize      = currentLength;
       cmd53.dataPtr       = gbalHandle->dmaBuffer;
       cmd53.mode          = WLANSAL_MODE_BLOCK;
       status = WLANSAL_Cmd53(pAdapter, &cmd53);
       if(!VOS_IS_STATUS_SUCCESS(status))
       {
           BMSGERROR("Read Memory fail %d", status, 0, 0);
           // Release lock
           sd_release_host(gbalHandle->sdio_func_dev);
           BEXIT();
           return status;
       }
       memcpy(bufferPtr, cmd53.dataPtr, currentLength);
       length -= currentLength;
       bufferPtr += currentLength;
       vos_mem_set(gbalHandle->dmaBuffer, 0, cmd53.dataSize);
    }



   // Release lock
   sd_release_host(gbalHandle->sdio_func_dev);
   BEXIT();
   return status;
}

/*----------------------------------------------------------------------------

  @brief Write memory from WLAN hardware

  @param v_U32_t   memAddress
  @param v_PVOID_t *bufferPtr
  @param v_U8_t    length

  @return General status code

----------------------------------------------------------------------------*/
static VOS_STATUS WLANBAL_WriteDeviceMemory
(
   v_PVOID_t  pAdapter,
   v_U32_t    memAddress,
   v_PVOID_t  bufferPtr,
   v_U32_t    length
)
{
   WLANSAL_Cmd53ReqType   cmd53;
   VOS_STATUS             status    = VOS_STATUS_SUCCESS;
   v_U32_t                targetAddress;
   v_U32_t                currentLength = 0;

   BENTER();

   if(!IS_VALID_2_ARG(gbalHandle, bufferPtr))
   {
      BMSGERROR("Invalid BAL Args gbalHandle 0x%p, bufferPtr 0x%p", gbalHandle, bufferPtr, 0);
      BEXIT();
      return VOS_STATUS_E_INVAL;
   }

   if(0 == length)
   {
      BMSGERROR("Invalid Length ZERO", 0, 0, 0);
      BEXIT();
      return VOS_STATUS_E_INVAL;
   }

   // Get the lock, going native
   sd_claim_host(gbalHandle->sdio_func_dev);


   cmd53.busDirection  = WLANSAL_DIRECTION_WRITE;
   cmd53.callSync      = WLANSAL_CALL_SYNC;
   cmd53.busCompleteCB = NULL;
   cmd53.function      = WLANSAL_FUNCTION_ONE;
   cmd53.addressHandle = WLANSAL_ADDRESS_INCREMENT;

   status = balAddressTranslation(pAdapter, memAddress, &targetAddress, VOS_TRUE);
   if(!VOS_IS_STATUS_SUCCESS(status))
   {
      BMSGERROR("Address Translation Fail", 0, 0, 0);
      // Release lock
      sd_release_host(gbalHandle->sdio_func_dev);
      BEXIT();
      return VOS_STATUS_E_FAILURE;
   }

   BMSGINFO("%s: Total Length %x\n", __func__, length, 0);

   while (length > 0)
   {
       if (length > WLANBAL_DMA_MAX_BUFFER_SIZE)
           currentLength = WLANBAL_DMA_MAX_BUFFER_SIZE;
       else
           currentLength = length;

       memcpy(gbalHandle->dmaBuffer, bufferPtr, currentLength);

       cmd53.address       = targetAddress;
       cmd53.dataSize      = currentLength;
       cmd53.dataPtr       = gbalHandle->dmaBuffer;
       cmd53.mode          = WLANSAL_MODE_BYTE;
       status = WLANSAL_Cmd53(pAdapter, &cmd53);
       if(!VOS_IS_STATUS_SUCCESS(status))
       {
           BMSGERROR("Write Memory fail %d", status, 0, 0);
           // Release lock
           sd_release_host(gbalHandle->sdio_func_dev);
           BEXIT();
           return status;
       }
       bufferPtr += currentLength;
       length -= currentLength;
   } 

   // Release lock
   sd_release_host(gbalHandle->sdio_func_dev);
   BEXIT();
   return status;
}


/*----------------------------------------------------------------------------

  @brief Read memory from WLAN hardware

  @param v_PVOID_t pAdapter
        Global adapter handle

  @param v_U32_t memAddress

  @param v_PVOID_t bufferPtr

  @param v_U32_t length

  @return General status code

----------------------------------------------------------------------------*/
VOS_STATUS
WLANBAL_ReadMemory
(
   v_PVOID_t  pAdapter,
   v_U32_t    memAddress,
   v_PVOID_t  bufferPtr,
   v_U32_t    length
)
{
   VOS_STATUS             status    = VOS_STATUS_SUCCESS;

   if((memAddress & WLANBAL_ADDRESS_BASE_MASK) != ((memAddress + length) & WLANBAL_ADDRESS_BASE_MASK))
   {
      //read in chunks of 64k memory to avoid memory corruption across the boundary
      v_U32_t sram64kBoundary = WLANBAL_64K_BOUNDARY;
      v_U32_t startAddr = memAddress, endAddr;
      v_U32_t totalLen = length;
      while (totalLen)
      {
         endAddr = ((startAddr + sram64kBoundary) / sram64kBoundary) * sram64kBoundary;
         if(endAddr > (memAddress + length))
         {
            endAddr = (memAddress + length);
         }

         status = WLANBAL_ReadDeviceMemory(pAdapter, (v_U32_t) (startAddr),
                                            ((v_U8_t *)bufferPtr + (startAddr - memAddress)/*offset*/),
                                            (v_U32_t) (endAddr - startAddr)/*readLen*/);
         if(status != VOS_STATUS_SUCCESS)
         {
            return status;
         }

         totalLen -= (endAddr - startAddr)/*readLen*/;
         startAddr = endAddr;
      }

   }
   else
   {
       return WLANBAL_ReadDeviceMemory(pAdapter, (v_U32_t) memAddress, (v_PVOID_t) bufferPtr, (v_U32_t) length);
   }
   return status;
}


/*----------------------------------------------------------------------------

  @brief Write memory from WLAN hardware

  @param v_U32_t   memAddress
  @param v_PVOID_t *bufferPtr
  @param v_U8_t    length

  @return General status code

----------------------------------------------------------------------------*/
VOS_STATUS
WLANBAL_WriteMemory
(
   v_PVOID_t  pAdapter,
   v_U32_t    memAddress,
   v_PVOID_t  bufferPtr,
   v_U32_t    length
)
{
   VOS_STATUS             status    = VOS_STATUS_SUCCESS;

   if((memAddress & WLANBAL_ADDRESS_BASE_MASK) != ((memAddress + length) & WLANBAL_ADDRESS_BASE_MASK))
   {
      //write in chunks of 64k memory to avoid memory corruption across the boundary
      v_U32_t sram64kBoundary = WLANBAL_64K_BOUNDARY;
      v_U32_t startAddr = memAddress, endAddr;
      v_U32_t totalLen = length;
      while (totalLen)
      {
         endAddr = ((startAddr + sram64kBoundary) / sram64kBoundary) * sram64kBoundary;
         if(endAddr > (memAddress + length))
         {
            endAddr = (memAddress + length);
         }

         status = WLANBAL_WriteDeviceMemory(pAdapter, (v_U32_t) (startAddr),
                                            (v_PVOID_t) ((v_U8_t *)bufferPtr + (startAddr - memAddress)/*offset*/),
                                            (v_U32_t) (endAddr - startAddr)/*writeLen*/);
         if(status != VOS_STATUS_SUCCESS)
         {
            return status;
         }

         totalLen -= (endAddr - startAddr)/*writeLen*/;
         startAddr = endAddr;
      }

   }
   else
   {
       return WLANBAL_WriteDeviceMemory(pAdapter, (v_U32_t) memAddress, (v_PVOID_t) bufferPtr, (v_U32_t) length);
   }
   return status;
}

/*----------------------------------------------------------------------------

  @brief Enable WLAN ASIC interrupt

  @param v_PVOID_t pAdapter
        Global adapter handle

  @return General status code

----------------------------------------------------------------------------*/
VOS_STATUS WLANBAL_EnableASICInterrupt
(
   v_PVOID_t pAdapter
)
{
   WLANSSC_HandleType     sscHandle = (WLANSSC_HandleType)VOS_GET_SSC_CTXT(pAdapter);
   VOS_STATUS             status    = VOS_STATUS_SUCCESS;

   BENTER();

   if(!IS_VALID_1_ARG(gbalHandle))
   {
      BEXIT();
      return VOS_STATUS_E_FAILURE;
   }

   status = WLANSSC_EnableASICInterrupt(sscHandle);

   BEXIT();
   return status;
}

#ifdef WLAN_PERF
VOS_STATUS WLANBAL_EnableASICInterruptEx
(
   v_PVOID_t pAdapter,
   v_U32_t uIntMask
)
{
   WLANSSC_HandleType     sscHandle = (WLANSSC_HandleType)VOS_GET_SSC_CTXT(pAdapter);
   VOS_STATUS             status    = VOS_STATUS_SUCCESS;

   return(WLANSSC_EnableASICInterruptEx(sscHandle, uIntMask));

   return status;
}
#endif /* WLAN_PERF */

/*----------------------------------------------------------------------------

  @brief Disable WLAN ASIC interrupt

  @param v_PVOID_t pAdapter
        Global adapter handle

  @return General status code

----------------------------------------------------------------------------*/
VOS_STATUS WLANBAL_DisableASICInterrupt
(
   v_PVOID_t pAdapter
)
{
   WLANSSC_HandleType     sscHandle = (WLANSSC_HandleType)VOS_GET_SSC_CTXT(pAdapter);
   VOS_STATUS             status    = VOS_STATUS_SUCCESS;

   BENTER();

   if(!IS_VALID_1_ARG(gbalHandle))
   {
      BEXIT();
      return VOS_STATUS_E_FAILURE;
   }

   status = WLANSSC_DisableASICInterrupt(sscHandle);

   BEXIT();
   return status;
}

#ifdef WLAN_PERF
VOS_STATUS WLANBAL_DisableASICInterruptEx
(
   v_PVOID_t pAdapter,
   v_U32_t uIntMask
)
{
   WLANSSC_HandleType     sscHandle = (WLANSSC_HandleType)VOS_GET_SSC_CTXT(pAdapter);

   return(WLANSSC_DisableASICInterruptEx(sscHandle, uIntMask));

}
#endif /* WLAN_PERF */

/*----------------------------------------------------------------------------

  @brief Config DXE Channel information

  @param v_PVOID_t pAdapter Global adapter handle
  @param WLANBAL_SDIODXEHeaderConfigType    ConfigInfo Default channel config infromation

  @return General status code

----------------------------------------------------------------------------*/
VOS_STATUS WLANBAL_DXEHeaderConfig
(
   v_PVOID_t                          pAdapter,
   WLANBAL_SDIODXEHeaderConfigType   *ConfigInfo
)
{
   VOS_STATUS             status    = VOS_STATUS_SUCCESS;

   VOS_ASSERT(gbalHandle);
   VOS_ASSERT(ConfigInfo);

   BENTER();

   memcpy(&gbalHandle->sdioDXEConfig, ConfigInfo, sizeof(WLANBAL_SDIODXEHeaderConfigType));

   BEXIT();
   return status;
}


/*----------------------------------------------------------------------------

  @brief Suspend BAL, Trigger SSC Suspend

  @param v_PVOID_t pAdapter
        Global adapter handle

  @return General status code

----------------------------------------------------------------------------*/
VOS_STATUS WLANBAL_Suspend
(
   v_PVOID_t pAdapter
)
{
   v_PVOID_t         sscHandle = (v_PVOID_t)VOS_GET_SSC_CTXT(pAdapter);
   VOS_STATUS        status = VOS_STATUS_SUCCESS;

   VOS_ASSERT(gbalHandle);
   VOS_ASSERT(sscHandle);

   BENTER();

   status = WLANSSC_Suspend(sscHandle, WLANSSC_ALL_FLOW);

   BEXIT();

   return status;
}


/*----------------------------------------------------------------------------

  @brief Resume BAL from Suspend, Trigger SSC Resume

  @param v_PVOID_t pAdapter
        Global adapter handle

  @return General status code

----------------------------------------------------------------------------*/
VOS_STATUS WLANBAL_Resume
(
   v_PVOID_t pAdapter
)
{
   v_PVOID_t         sscHandle = (v_PVOID_t)VOS_GET_SSC_CTXT(pAdapter);
   VOS_STATUS        status = VOS_STATUS_SUCCESS;

   VOS_ASSERT(gbalHandle);
   VOS_ASSERT(sscHandle);

   BENTER();

   status = WLANSSC_Resume(sscHandle, WLANSSC_ALL_FLOW);

   BEXIT();

   return status;
}


/*----------------------------------------------------------------------------

  @brief Suspend Entire chip, Trigger SSC Suspend Chip

  @param v_PVOID_t pAdapter
        Global adapter handle

  @return General status code
        VOS_STATUS_SUCCESS       Suspend Chip success
        VOS_STATUS_E_INVAL       Invalid Parameters

----------------------------------------------------------------------------*/
VOS_STATUS WLANBAL_SuspendChip
(
   v_PVOID_t pAdapter
)
{
   v_PVOID_t         sscHandle = (v_PVOID_t)VOS_GET_SSC_CTXT(pAdapter);
   VOS_STATUS        status = VOS_STATUS_SUCCESS;

   VOS_ASSERT(gbalHandle);
   VOS_ASSERT(sscHandle);

   BENTER();

   status = WLANSSC_SuspendChip(sscHandle);


   BEXIT();

   return status;
}

/*----------------------------------------------------------------------------

  @brief Resume entire chip from Suspend, Trigger SSC Resume chip

  @param v_PVOID_t pAdapter
        Global adapter handle

  @return General status code
        VOS_STATUS_SUCCESS       Suspend Chip success
        VOS_STATUS_E_INVAL       Invalid Parameters

----------------------------------------------------------------------------*/
VOS_STATUS WLANBAL_ResumeChip
(
   v_PVOID_t pAdapter
)
{
   v_PVOID_t         sscHandle = (v_PVOID_t)VOS_GET_SSC_CTXT(pAdapter);
   VOS_STATUS        status = VOS_STATUS_SUCCESS;

   VOS_ASSERT(gbalHandle);
   VOS_ASSERT(sscHandle);

   BENTER();

   status = WLANSSC_ResumeChip(sscHandle);

   BEXIT();

   return status;
}


/*=========================================================================
 * END Interactions with HAL
 *=========================================================================*/

/*=========================================================================
 * Interactions with TL
 *=========================================================================*/

/*----------------------------------------------------------------------------

  @brief Register TL Callback functions to BAL.
        Registration functions are Receive Frames notification function,
        Get send frame request function, send complete notification functions
        and TX resource availability notification function

  @param v_PVOID_t pAdapter
        Global adapter handle

  @param WLANBAL_TlRegType *tlReg
        TL registration element type.
        Receive frame CB, TX complete CB, get TX Frame CB, TX resources CB
        and TL internal hanlde are elements of this structure

  @return General status code
        VOS_STATUS_SUCCESS       Registration success
        VOS_STATUS_E_INVAL       Invalid argument
        VOS_STATUS_E_FAILURE     BAL is not ready

----------------------------------------------------------------------------*/
/* CODE TEST */
VOS_STATUS WLANBAL_RegTlCbFunctions
(
   v_PVOID_t          pAdapter,
   WLANBAL_TlRegType *tlReg
)
{

   VOS_ASSERT(gbalHandle);
   VOS_ASSERT(tlReg);

   BENTER();

   gbalHandle->tlReg.receiveFrameCB  = tlReg->receiveFrameCB;
   gbalHandle->tlReg.getTXFrameCB    = tlReg->getTXFrameCB;
   gbalHandle->tlReg.txCompleteCB    = tlReg->txCompleteCB;
   gbalHandle->tlReg.txResourceCB    = tlReg->txResourceCB;
   gbalHandle->tlReg.txResourceThreashold = tlReg->txResourceThreashold;

   BEXIT();
   return VOS_STATUS_SUCCESS;
}


/*----------------------------------------------------------------------------

  @brief Get available TX buffer size within WLAN ASIC.
        If TX buffer size is larger then preset threshold value,
        just pass buffer size. Otherwise, pass TX buffer size and start timer
        to wait HW handle TX buffer and make it free. After timer expired,
        read TX buffer size again. During this timer running, TL has to wait

  @param v_PVOID_t pAdapter
        Global adapter handle

  @param v_U32_t *availableTxBuffer
        Buffer pointer have to be stored TX resource size

  @return General status code
        VOS_STATUS_SUCCESS      Notify success
        VOS_STATUS_E_INVAL      Invalid argument
        VOS_STATUS_E_FAILURE    BAL is not ready

----------------------------------------------------------------------------*/
VOS_STATUS WLANBAL_GetTxResources
(
   v_PVOID_t  pAdapter,
   v_U32_t   *availableTxBuffer
)
{
   VOS_STATUS     status    = VOS_STATUS_SUCCESS;
   v_U16_t        bdBufferCount = 0;
   v_U16_t        pduBufferCount = 0;
   v_U32_t        switchBuffer;

   VOS_ASSERT(gbalHandle);
   VOS_ASSERT(availableTxBuffer);

   BENTER();

   // CAUTION!!! here the code assumes that this is called with the
   // sd claim host lock in place.
   status = WLANBAL_ReadRegister(pAdapter,
                                 QWLAN_SIF_BMU_BD_PDU_RSV_ALL_REG,
                                 availableTxBuffer);
   if(!VOS_IS_STATUS_SUCCESS(status))
   {
      BMSGERROR("%s: Get Resource Fail\n", __func__, 0, 0);
      // Release lock
      BEXIT();
      return VOS_STATUS_E_FAILURE;
   }


   switchBuffer   = *availableTxBuffer;
   bdBufferCount  = (v_U16_t)(switchBuffer & WLANBAL_BD_PDU_16BIT_MASK);
   pduBufferCount = (v_U16_t)((switchBuffer >> WLANBAL_PDU_BIT_SHIFT) & (WLANBAL_BD_PDU_16BIT_MASK));
   *availableTxBuffer = (v_U32_t)(bdBufferCount + pduBufferCount);


   BMSGWARN("%s: Threshold = %x  count=%x\n", __func__, (unsigned int)gbalHandle->tlReg.txResourceThreashold,
      (unsigned int)(bdBufferCount + pduBufferCount));

   if(gbalHandle->tlReg.txResourceThreashold > (v_U32_t)(bdBufferCount + pduBufferCount))
   {
      BMSGERROR("%s: Not enough resource try again BD %d, PDU %d", __func__, bdBufferCount,
         pduBufferCount);
      if (VOS_TIMER_STATE_STOPPED == vos_timer_getCurrentState(&gbalHandle->timer))
      {
         status = vos_timer_start(&gbalHandle->timer,
                                  WLANBAL_TX_RESOURCE_TIMEOUT);
         if(!VOS_IS_STATUS_SUCCESS(status))
         {
            BMSGERROR("%s: Timer Start Fail\n", __func__, 0, 0);
            // Release lock
            BEXIT();
            return VOS_STATUS_E_FAILURE;
         }
      }
      else
      {
         BMSGERROR("%s: Timer not stopped, cannot start\n", __func__, 0, 0);
      }
#ifdef VOLANS_PERF
      return VOS_STATUS_E_RESOURCES;
#endif /* VOLANS_PERF */
   }

   BEXIT();
   return VOS_STATUS_SUCCESS;
}

/*----------------------------------------------------------------------------

  @brief TL Notify there are TX pending frames to SSC

  @param v_PVOID_t pAdapter
        Global adapter handle

  @return General status code
        VOS_STATUS_SUCCESS      Notify success
        VOS_STATUS_E_FAILURE    BAL is not ready

----------------------------------------------------------------------------*/
VOS_STATUS WLANBAL_StartXmit
(
   v_PVOID_t pAdapter
)
{
   WLANSSC_HandleType  sscHandle = (WLANSSC_HandleType)VOS_GET_SSC_CTXT(pAdapter);

   VOS_ASSERT(sscHandle);

   if (WLANSSC_StartTransmit(sscHandle) != VOS_STATUS_SUCCESS)
   {
      BMSGERROR("%s: WLANSSC_StartTransmit Fail\n", __func__, 0, 0);
      BEXIT();
      return VOS_STATUS_E_FAILURE;
   }

   BEXIT();
   return VOS_STATUS_SUCCESS;
}


/*=========================================================================
 * END Interactions with TL
 *=========================================================================*/

/*=========================================================================
 * Interactions with SSC
 *=========================================================================*/

/*=========================================================================
 * END Interactions with SSC
 *=========================================================================*/

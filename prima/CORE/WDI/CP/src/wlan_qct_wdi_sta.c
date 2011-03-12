/*===========================================================================

                       W L A N _ Q C T _ W D I _ S T A . C

  OVERVIEW:

  This software unit holds the implementation of the WLAN Device Abstraction     
  Layer Station Table Management Entity.

  The functions externalized by this module are internal APIs for DAL Core
  and can only be called by it. 

  DEPENDENCIES:

  Are listed for each API below.


  Copyright (c) 2008 QUALCOMM Incorporated.
  All Rights Reserved.
  Qualcomm Confidential and Proprietary
===========================================================================*/

/*===========================================================================

                      EDIT HISTORY FOR FILE


  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.


   $Header$$DateTime$$Author$


  when        who     what, where, why
----------    ---    --------------------------------------------------------
2010-08-09    lti     Created module

===========================================================================*/

/*----------------------------------------------------------------------------
 * Include Files
 * -------------------------------------------------------------------------*/
#include "wlan_qct_wdi.h" 
#include "wlan_qct_wdi_i.h" 
#include "wlan_qct_wdi_sta.h" 
#include "wlan_qct_pal_api.h"
#include "wlan_qct_pal_trace.h"


/*----------------------------------------------------------------------------
 * Function definition
 * -------------------------------------------------------------------------*/
/**
 @brief WDI_STATableInit - Initializes the STA tables. 
        Allocates the necesary memory.

 
 @param  pWDICtx:         pointer to the WLAN DAL context 
  
 @see
 @return Result of the function call
*/
WDI_Status WDI_STATableInit
(
   WDI_ControlBlockType*  pWDICtx
)
{
    wpt_uint8  ucMaxStations;
    /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/    

    ucMaxStations     = (wpt_uint8) pWDICtx->ucMaxStations;
    
    /*----------------------------------------------------------------------
       Allocate the memory for sta table
    ------------------------------------------------------------------------*/
    pWDICtx->staTable = wpalMemoryAllocate(ucMaxStations * sizeof(WDI_StaStruct));

    if (NULL == pWDICtx->staTable)
    {
            
        WDI_STATableClose(pWDICtx);

        WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
                  "Error allocating memory on WDI_STATableInit"); 
        return WDI_STATUS_E_FAILURE;
    }
    
    wpalMemoryZero( pWDICtx->staTable, ucMaxStations * sizeof( WDI_StaStruct ));

    // Initialize the Self STAID to an invalid value
    pWDICtx->ucSelfStaId = WDI_STA_INVALID_IDX;

    return WDI_STATUS_SUCCESS;
}/*WDI_STATableInit*/

/**
 @brief WDI_STATableStart - resets the max and number values of 
        STAtions

 
 @param  pWDICtx:         pointer to the WLAN DAL context 
  
 @see
 @return Result of the function call
*/
WDI_Status 
WDI_STATableStart
(
    WDI_ControlBlockType*  pWDICtx
)
{
    wpt_uint8 ucMaxStations;
    /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
    
    ucMaxStations     = (wpt_uint8) pWDICtx->ucMaxStations;
 
    return WDI_STATUS_SUCCESS;
}/*WDI_STATableStart*/

/**
 @brief WDI_STATableStop - clears the sta table

 
 @param  pWDICtx:         pointer to the WLAN DAL context 
  
 @see
 @return Result of the function call
*/
WDI_Status 
WDI_STATableStop
(
    WDI_ControlBlockType*  pWDICtx
)
{
    wpt_uint8 ucMaxStations;
    /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

    /* Clean up the Self STAID */
    pWDICtx->ucSelfStaId = WDI_STA_INVALID_IDX;

    ucMaxStations     = pWDICtx->ucMaxStations;
    
    wpalMemoryZero( (void *) pWDICtx->staTable,
            ucMaxStations * sizeof( WDI_StaStruct ));

    return WDI_STATUS_SUCCESS;
}/*WDI_STATableStop*/

/**
 @brief WDI_STATableClose - frees the resources used by the STA 
        table.

 
 @param  pWDICtx:         pointer to the WLAN DAL context 
  
 @see
 @return Result of the function call
*/
WDI_Status 
WDI_STATableClose
(
  WDI_ControlBlockType*  pWDICtx
)
{
    WDI_Status status = WDI_STATUS_SUCCESS;
        
    // Free memory
    if (pWDICtx->staTable != NULL)
        wpalMemoryFree( pWDICtx->staTable);

    pWDICtx->staTable = NULL;
    return status;
}/*WDI_STATableClose*/


/**
 @brief WDI_STATableAddSta - Function to Add Station

 
 @param  pWDICtx:     pointer to the WLAN DAL context 
         pwdiParam:   station parameters  
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_STATableAddSta
(
    WDI_ControlBlockType*  pWDICtx,
    WDI_AddStaParams*      pwdiParam
)
{
    wpt_uint16       usStaIdx  = 0;
    WDI_StaStruct*   pSTATable = (WDI_StaStruct*) pWDICtx->staTable;
    /*- - - -  - - - - - - - - - - - -  - - - - - - - - - - - -  - - - - - */

    /*-----------------------------------------------------------------------
      Sanity check
      - station ids are allocated by the HAL located on RIVA SS - they must
      always be valid 
    -----------------------------------------------------------------------*/
    if (( pwdiParam->usStaIdx  == WDI_STA_INVALID_IDX) ||
        ( pwdiParam->usStaIdx >= pWDICtx->ucMaxStations ))
    {
      WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
                "Station id sent by HAL is invalid - not OK"); 
      WDI_ASSERT(0); 
      return WDI_STATUS_E_FAILURE; 
    }
    
    usStaIdx =  pwdiParam->usStaIdx;

    /*Since we are not the allocator of STA Ids but HAL is - just set flag to
      valid*/
    pSTATable[usStaIdx].valid = 1;     
    
    
    // Save the STA type - this is used for lookup
    WDI_STATableSetStaType(pWDICtx, usStaIdx, pwdiParam->ucStaType);
    WDI_STATableSetStaQosEnabled(pWDICtx, usStaIdx, 
          (wpt_uint8)(pwdiParam->ucWmmEnabled | pwdiParam->ucHTCapable) );

#ifdef WLAN_PERF
    pWDICtx->uBdSigSerialNum ++;
#endif
    
    wpalMemoryCopy(pSTATable[usStaIdx].macBSSID, 
                   pwdiParam->macBSSID, WDI_MAC_ADDR_LEN);

    /*------------------------------------------------------------------------
      Set DPU Related Information 
    ------------------------------------------------------------------------*/
    pSTATable[usStaIdx].dpuIndex              = pwdiParam->dpuIndex; 
    pSTATable[usStaIdx].dpuSig                = pwdiParam->dpuSig; 

    pSTATable[usStaIdx].bcastDpuIndex         = pwdiParam->bcastDpuIndex; 
    pSTATable[usStaIdx].bcastDpuSignature     = pwdiParam->bcastDpuSignature; 

    pSTATable[usStaIdx].bcastMgmtDpuIndex     = pwdiParam->bcastMgmtDpuIndex; 
    pSTATable[usStaIdx].bcastMgmtDpuSignature = pwdiParam->bcastMgmtDpuSignature; 

    /*Robust Mgmt Frame enabled */
    pSTATable[usStaIdx].rmfEnabled            = pwdiParam->ucRmfEnabled;

    pSTATable[usStaIdx].bssIdx                = pwdiParam->ucbssIndex;

    /* Now update the STA entry with the new MAC address */
    if(WDI_STATUS_SUCCESS != WDI_STATableSetStaAddr( pWDICtx, 
                                                     usStaIdx, 
                                                     pwdiParam->staMacAddr))
    {
       WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
                 "Failed to update station entry - internal failure");
       WDI_ASSERT(0);
       return WDI_STATUS_E_FAILURE; 
    }

    /* Now update the STA entry with the new BSSID address */
    if(WDI_STATUS_SUCCESS != WDI_STATableSetBSSID( pWDICtx, 
                                                     usStaIdx, 
                                                     pwdiParam->macBSSID))
    {
       WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
                 "Failed to update station entry - internal failure");
       WDI_ASSERT(0);
       return WDI_STATUS_E_FAILURE; 
    }

    return WDI_STATUS_SUCCESS;
}/*WDI_AddSta*/

/**
 @brief WDI_STATableDelSta - Function to Delete a Station

 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         ucStaIdx:        station to be deleted
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_STATableDelSta
(
    WDI_ControlBlockType*  pWDICtx,
    wpt_uint16             usStaIdx
)
{
    WDI_StaStruct*   pSTATable = (WDI_StaStruct*) pWDICtx->staTable;
    /*- - - -  - - - - - - - - - - - -  - - - - - - - - - - - -  - - - - - */

    /*-----------------------------------------------------------------------
      Sanity check
      - station ids are allocated by the HAL located on RIVA SS - they must
      always be valid 
    -----------------------------------------------------------------------*/
    if(( usStaIdx  == WDI_STA_INVALID_IDX )||
        ( usStaIdx >= pWDICtx->ucMaxStations ))
    {
       WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
                 "STA Id invalid on Del STA - internal failure");
       WDI_ASSERT(0);
       return WDI_STATUS_E_FAILURE; 
    }
    
    wpalMemoryZero(&pSTATable[usStaIdx], sizeof(pSTATable[usStaIdx])); 
    pSTATable->valid = 0; 
    return WDI_STATUS_SUCCESS;
}/*WDI_STATableDelSta*/


/**
 @brief WDI_STATableGetStaBSSIDAddr - Gets the BSSID associated 
        with this station

 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         ucStaIdx:        station index
         pmacBSSID:      out BSSID for this STA
  
 @see
 @return Result of the function call
*/
WDI_Status 
WDI_STATableGetStaBSSIDAddr
(
    WDI_ControlBlockType*  pWDICtx,  
    wpt_uint16             staIdx, 
    wpt_macAddr*           pmacBSSID
)
{
  WDI_StaStruct* pSTATable = (WDI_StaStruct*) pWDICtx->staTable;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  if ((staIdx < pWDICtx->ucMaxStations) && (pSTATable[staIdx].valid))
  {
     wpalMemoryCopy(*pmacBSSID, pSTATable[staIdx].macBSSID, WDI_MAC_ADDR_LEN);
     return WDI_STATUS_SUCCESS;
  }
  else
     return WDI_STATUS_E_FAILURE;
}/*WDI_STATableGetStaQosEnabled*/


/**
 @brief WDI_STATableGetStaQosEnabled - Gets is qos is enabled 
        for a sta

 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         ucStaIdx:        station index
         qosEnabled:      out qos enabled
  
 @see
 @return Result of the function call
*/
WDI_Status 
WDI_STATableGetStaQosEnabled
(
    WDI_ControlBlockType*  pWDICtx,  
    wpt_uint16             staIdx, 
    wpt_uint8*             qosEnabled
)
{
  WDI_StaStruct* pSTATable = (WDI_StaStruct*) pWDICtx->staTable;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  if ((staIdx < pWDICtx->ucMaxStations) && (pSTATable[staIdx].valid) && qosEnabled)
  {
     *qosEnabled = pSTATable[staIdx].qosEnabled;
     return WDI_STATUS_SUCCESS;
  }
  else
     return WDI_STATUS_E_FAILURE;
}/*WDI_STATableGetStaQosEnabled*/

/**
 @brief WDI_STATableSetStaQosEnabled - set qos mode for STA

 
 @param  pWDICtx:    pointer to the WLAN DAL context 
         ucStaIdx:   station index
         qosEnabled: qos enabled
  
 @see
 @return Result of the function call
*/
WDI_Status 
WDI_STATableSetStaQosEnabled
(
    WDI_ControlBlockType*  pWDICtx,  
    wpt_uint16             staIdx, 
    wpt_uint8              qosEnabled
)
{
    WDI_StaStruct* pSTATable = (WDI_StaStruct*) pWDICtx->staTable;
    if ((staIdx < pWDICtx->ucMaxStations) && (pSTATable[staIdx].valid))
    {
        pSTATable[staIdx].qosEnabled = qosEnabled;
        return WDI_STATUS_SUCCESS;
    }
    else
        return WDI_STATUS_E_FAILURE;
}/*WDI_STATableSetStaQosEnabled*/

/**
 @brief WDI_STATableGetStaType - get sta type for STA

 
 @param  pWDICtx:   pointer to the WLAN DAL context 
         ucStaIdx:  station index
         pStaType:  qos enabled
  
 @see
 @return Result of the function call
*/
WDI_Status 
WDI_STATableGetStaType
(
    WDI_ControlBlockType*  pWDICtx,  
    wpt_uint16             staIdx, 
    wpt_uint8*             pStaType
)
{
    WDI_StaStruct* pSTATable = (WDI_StaStruct*) pWDICtx->staTable;
    if ((staIdx < pWDICtx->ucMaxStations) && (pSTATable[staIdx].valid))
    {
        *pStaType = pSTATable[staIdx].ucStaType;
        return WDI_STATUS_SUCCESS;
    }
    else
        return WDI_STATUS_E_FAILURE;
}/*WDI_STATableGetStaType*/

/**
 @brief WDI_STATableSetStaType - sets sta type for STA

 
 @param  pWDICtx:   pointer to the WLAN DAL context 
         ucStaIdx:  station index
         staType:   sta type
  
 @see
 @return Result of the function call
*/
WDI_Status 
WDI_STATableSetStaType
(
    WDI_ControlBlockType*  pWDICtx,  
    wpt_uint16             staIdx, 
    wpt_uint8              staType
)
{
    WDI_StaStruct* pSTATable = (WDI_StaStruct*) pWDICtx->staTable;
    if ((staIdx < pWDICtx->ucMaxStations) && (pSTATable[staIdx].valid))
    {
        pSTATable[staIdx].ucStaType = staType;
        return WDI_STATUS_SUCCESS;
    }
    else
        return WDI_STATUS_E_FAILURE;
}/*WDI_STATableSetStaType*/


/**
 @brief WDI_CompareMacAddr - compare the MAC address

 
 @param  addr1: address 1 
         addr2: address 2  
  
 @see
 @return Result of the function call
*/
WPT_STATIC WPT_INLINE wpt_uint8
WDI_CompareMacAddr
(
  wpt_uint8 addr1[], 
  wpt_uint8 addr2[]
)
{
#if defined( _X86_ )
    wpt_uint32 align = (0x3 & ((wpt_uint32) addr1 | (wpt_uint32) addr2 ));

    if( align ==0){
        return ((*((wpt_uint16 *) &(addr1[4])) == *((wpt_uint16 *) &(addr2[4])))&&
                (*((wpt_uint32 *) addr1) == *((wpt_uint32 *) addr2)));
    }else if(align == 2){
        return ((*((wpt_uint16 *) &addr1[4]) == *((wpt_uint16 *) &addr2[4])) &&
            (*((wpt_uint16 *) &addr1[2]) == *((wpt_uint16 *) &addr2[2])) &&
            (*((wpt_uint16 *) &addr1[0]) == *((wpt_uint16 *) &addr2[0])));
    }else{
        return ( (addr1[5]==addr2[5])&&
            (addr1[4]==addr2[4])&&
            (addr1[3]==addr2[3])&&
            (addr1[2]==addr2[2])&&
            (addr1[1]==addr2[1])&&
            (addr1[0]==addr2[0]));
    }
#else
         return ( (addr1[0]==addr2[0])&&
            (addr1[1]==addr2[1])&&
            (addr1[2]==addr2[2])&&
            (addr1[3]==addr2[3])&&
            (addr1[4]==addr2[4])&&
            (addr1[5]==addr2[5]));
#endif
}/*WDI_CompareMacAddr*/


/**
 @brief WDI_STATableFindStaidByAddr - Given a station mac address, search
        for the corresponding station index from the Station Table.
 
 @param  pWDICtx:  WDI Context pointer
         staAddr:  station address
         pucStaId: output station id 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_STATableFindStaidByAddr
(
    WDI_ControlBlockType*  pWDICtx, 
    wpt_macAddr            staAddr, 
    wpt_uint8*             pucStaId
)
{
    WDI_Status wdiStatus = WDI_STATUS_E_FAILURE;
    wpt_uint8 i;
    WDI_StaStruct* pSTATable = (WDI_StaStruct*) pWDICtx->staTable;

    for (i=0; i < pWDICtx->ucMaxStations; i++, pSTATable++)
    {
        if ( (pSTATable->valid == 1) && (WDI_CompareMacAddr(pSTATable->staAddr, staAddr)) )
        {
            *pucStaId = i;
            wdiStatus = WDI_STATUS_SUCCESS;
            break;
        }
    }
    return wdiStatus;
}/*WDI_STATableFindStaidByAddr*/

/**
 @brief WDI_STATableGetStaAddr - get station address
 
 @param  pWDICtx:  WDI Context pointer
         staIdx:  station index
         pStaAddr: output station address 
  
 @see
 @return Result of the function call
*/
WDI_Status 
WDI_STATableGetStaAddr
(
    WDI_ControlBlockType*  pWDICtx,  
    wpt_uint16             staIdx, 
    wpt_uint8**            pStaAddr
)
{
    WDI_StaStruct* pSTATable = (WDI_StaStruct*) pWDICtx->staTable;
    if ((staIdx < pWDICtx->ucMaxStations) && (pSTATable[staIdx].valid))
    {
        *pStaAddr = pSTATable[staIdx].staAddr;
        return WDI_STATUS_SUCCESS;
    }
    else
        return WDI_STATUS_E_FAILURE;
}/*WDI_STATableGetStaAddr*/

/**
 @brief WDI_STATableSetStaAddr - set station address
 
 @param  pWDICtx:  WDI Context pointer
         staIdx:   station index
         pStaAddr: output station address 
  
 @see
 @return Result of the function call
*/
WDI_Status 
WDI_STATableSetStaAddr
(
    WDI_ControlBlockType*  pWDICtx,  
    wpt_uint16             staIdx, 
    wpt_macAddr            staAddr
)
{
    WDI_StaStruct* pSTATable = (WDI_StaStruct*) pWDICtx->staTable;
    if ((staIdx < pWDICtx->ucMaxStations) && (pSTATable[staIdx].valid))
    {
        wpalMemoryCopy (pSTATable[staIdx].staAddr, staAddr, 6);
        return WDI_STATUS_SUCCESS;
    }
    else
        return WDI_STATUS_E_FAILURE;
}/*WDI_STATableSetStaAddr*/

/**
 @brief WDI_STATableSetBSSID - set station corresponding BSSID
 
 @param  pWDICtx:  WDI Context pointer
         staIdx:   station index
         pStaAddr: output station address 
  
 @see
 @return Result of the function call
*/
WDI_Status 
WDI_STATableSetBSSID
(
    WDI_ControlBlockType*  pWDICtx,  
    wpt_uint16             staIdx, 
    wpt_macAddr            macBSSID
)
{
    WDI_StaStruct* pSTATable = (WDI_StaStruct*) pWDICtx->staTable;
    if ((staIdx < pWDICtx->ucMaxStations) && (pSTATable[staIdx].valid))
    {
        wpalMemoryCopy (pSTATable[staIdx].macBSSID, macBSSID, 6);
        return WDI_STATUS_SUCCESS;
    }
    else
        return WDI_STATUS_E_FAILURE;
}/*WDI_STATableSetBSSID*/

/**
 @brief WDI_STATableSetBSSIdx - set station corresponding BSS index
 
 @param  pWDICtx:  WDI Context pointer
         staIdx:   station index
         bssIdx:   BSS index 
  
 @see
 @return Result of the function call
*/
WDI_Status 
WDI_STATableSetBSSIdx
(
    WDI_ControlBlockType*  pWDICtx,  
    wpt_uint16             staIdx, 
    wpt_uint8              bssIdx
)
{
    WDI_StaStruct* pSTATable = (WDI_StaStruct*) pWDICtx->staTable;
    if ((staIdx < pWDICtx->ucMaxStations) && (pSTATable[staIdx].valid))
    {
        pSTATable[staIdx].bssIdx = bssIdx;
        return WDI_STATUS_SUCCESS;
    }
    else
        return WDI_STATUS_E_FAILURE;
}/*WDI_STATableSetBSSIdx*/


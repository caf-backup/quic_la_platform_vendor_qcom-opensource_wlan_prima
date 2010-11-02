#include <palApi.h>
#include <sirTypes.h>   // needed for tSirRetStatus
#include <vos_api.h>

#include <sirParams.h>  // needed for tSirMbMsg


// its not worth the time trying to get all the includes in place to get to
// halMmhForwardMBmsg.  if I inlude halMnt.h, I get all kids of compile errros
// for things missing from palPipes.h (asicDxe.h is looking for these).  palPipes
// is used only in Gen4 DVT code so why we would have it or need it is puzzling.
//#include <halMnt.h>

// Just declare the function extern here and save some time.
extern tSirRetStatus halMmhForwardMBmsg(void*, tSirMbMsg*);



#ifdef MEMORY_DEBUG
eHalStatus palAllocateMemory_debug( tHddHandle hHdd, void **ppMemory, tANI_U32 numBytes, tANI_U8* fileName, tANI_U32 lineNum )
{
   eHalStatus halStatus = eHAL_STATUS_SUCCESS;
   
   *ppMemory = vos_mem_malloc_debug( numBytes, fileName, lineNum );
   
   if ( NULL == *ppMemory ) 
   {
      halStatus = eHAL_STATUS_FAILURE;
   }
   
   return( halStatus );
}
#else
eHalStatus palAllocateMemory( tHddHandle hHdd, void **ppMemory, tANI_U32 numBytes )
{
   eHalStatus halStatus = eHAL_STATUS_SUCCESS;
   
   *ppMemory = vos_mem_malloc( numBytes );
   
   if ( NULL == *ppMemory ) 
   {
      halStatus = eHAL_STATUS_FAILURE;
   }
   
   return( halStatus );
}
#endif


eHalStatus palFreeMemory( tHddHandle hHdd, void *pMemory )
{
   vos_mem_free( pMemory );
   
   return( eHAL_STATUS_SUCCESS );
}

eHalStatus palFillMemory( tHddHandle hHdd, void *pMemory, tANI_U32 numBytes, tANI_BYTE fillValue )
{
   vos_mem_set( pMemory, numBytes, fillValue );
   
   return( eHAL_STATUS_SUCCESS );
}


eHalStatus palCopyMemory( tHddHandle hHdd, void *pDst, void *pSrc, tANI_U32 numBytes )
{
   vos_mem_copy( pDst, pSrc, numBytes );
   
   return( eHAL_STATUS_SUCCESS );
}



tANI_BOOLEAN palEqualMemory( tHddHandle hHdd, void *pMemory1, void *pMemory2, tANI_U32 numBytes )
{
   return( vos_mem_compare( pMemory1, pMemory2, numBytes ) );
}   


eHalStatus palPktAlloc(tHddHandle hHdd, eFrameType frmType, tANI_U16 size, void **data, void **ppPacket)
{
   eHalStatus halStatus = eHAL_STATUS_FAILURE;
   VOS_STATUS vosStatus;
   
   vos_pkt_t *pVosPacket;
   
   do 
   {
      // we are only handling the 802_11_MGMT frame type for PE/LIM.  All other frame types should be 
      // ported to use the VOSS APIs directly and should not be using this palPktAlloc API.
      VOS_ASSERT( HAL_TXRX_FRM_802_11_MGMT == frmType );
    
      if ( HAL_TXRX_FRM_802_11_MGMT != frmType ) break;
   
      // allocate one 802_11_MGMT VOS packet, zero the packet and fail the call if nothing is available.
      // if we cannot get this vos packet, fail.
      vosStatus = vos_pkt_get_packet( &pVosPacket, VOS_PKT_TYPE_TX_802_11_MGMT, size, 1, VOS_TRUE, NULL, NULL );
      if ( !VOS_IS_STATUS_SUCCESS( vosStatus ) ) break;
      
      // Reserve the space at the head of the packet for the caller.  If we cannot reserve the space
      // then we have to fail (return the packet to voss first!)
      vosStatus = vos_pkt_reserve_head( pVosPacket, data, size );
      if ( !VOS_IS_STATUS_SUCCESS( vosStatus ) ) 
      {
         vos_pkt_return_packet( pVosPacket );
         break;
      }
      
      // Everything went well if we get here.  Return the packet pointer to the caller and indicate
      // success to the caller.
      *ppPacket = (void *)pVosPacket;
      
      halStatus = eHAL_STATUS_SUCCESS;
   
   } while( 0 );
   
   return( halStatus );
}



void palPktFree( tHddHandle hHdd, eFrameType frmType, void* buf, void *pPacket)
{
   vos_pkt_t *pVosPacket = (vos_pkt_t *)pPacket;
   VOS_STATUS vosStatus;
      
   do 
   {
      VOS_ASSERT( pVosPacket );
      
      if ( !pVosPacket ) break;
      
      // we are only handling the 802_11_MGMT frame type for PE/LIM.  All other frame types should be 
      // ported to use the VOSS APIs directly and should not be using this palPktAlloc API.
      VOS_ASSERT( HAL_TXRX_FRM_802_11_MGMT == frmType );
      if ( HAL_TXRX_FRM_802_11_MGMT != frmType ) break;
      
      // return the vos packet to Voss.  Nothing to do if this fails since the palPktFree does not 
      // have a return code.
      vosStatus = vos_pkt_return_packet( pVosPacket );
      VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
      
   } while( 0 );
   
   return;
}
   


tANI_U32 palGetTickCount(tHddHandle hHdd)
{
   return( vos_timer_get_system_ticks() );
}


tANI_U32 pal_be32_to_cpu(tANI_U32 x)
{
   return( x );
}

tANI_U32 pal_cpu_to_be32(tANI_U32 x)
{
   return(( x ) );
}   

tANI_U16 pal_be16_to_cpu(tANI_U16 x)
{
   return( ( x ) );
}   
   
tANI_U16 pal_cpu_to_be16(tANI_U16 x)
{
   return( ( x ) );
}   



eHalStatus palSpinLockAlloc( tHddHandle hHdd, tPalSpinLockHandle *pHandle )
{
   eHalStatus halStatus = eHAL_STATUS_FAILURE;
   VOS_STATUS vosStatus;
   vos_lock_t *pLock;
   
   do
   {
      pLock = vos_mem_malloc( sizeof( vos_lock_t ) );
   
      if ( NULL == pLock ) break;
      
      vosStatus = vos_lock_init( pLock );
      if ( !VOS_IS_STATUS_SUCCESS( vosStatus ) )
      {
         vos_mem_free( pLock );
         break;
      }
      
      *pHandle = (tPalSpinLockHandle)pLock;
      halStatus = eHAL_STATUS_SUCCESS;
      
   } while( 0 );
   
   return( halStatus );   
}


eHalStatus palSpinLockFree( tHddHandle hHdd, tPalSpinLockHandle hSpinLock )
{
   eHalStatus halStatus = eHAL_STATUS_FAILURE;
   vos_lock_t *pLock = (vos_lock_t *)hSpinLock;
   VOS_STATUS vosStatus;
   
   vosStatus = vos_lock_destroy( pLock );
   if ( VOS_IS_STATUS_SUCCESS( vosStatus ) )
   {
      // if we successfully destroy the lock, free
      // the memory and indicate success to the caller.
      vos_mem_free( pLock );
      
      halStatus = eHAL_STATUS_SUCCESS;
   }
   return( halStatus );
}


eHalStatus palSpinLockTake( tHddHandle hHdd, tPalSpinLockHandle hSpinLock )
{
   eHalStatus halStatus = eHAL_STATUS_FAILURE;
   vos_lock_t *pLock = (vos_lock_t *)hSpinLock;
   VOS_STATUS vosStatus;
   
   vosStatus = vos_lock_acquire( pLock );
   if ( VOS_IS_STATUS_SUCCESS( vosStatus ) )
   {
      // if successfully acquire the lock, indicate success
      // to the caller.
      halStatus = eHAL_STATUS_SUCCESS;
   }
   
   return( halStatus );
}   
   




eHalStatus palSpinLockGive( tHddHandle hHdd, tPalSpinLockHandle hSpinLock )
{
   eHalStatus halStatus = eHAL_STATUS_FAILURE;
   vos_lock_t *pLock = (vos_lock_t *)hSpinLock;
   VOS_STATUS vosStatus;
   
   vosStatus = vos_lock_release( pLock );
   if ( VOS_IS_STATUS_SUCCESS( vosStatus ) )
   {
      // if successfully acquire the lock, indicate success
      // to the caller.
      halStatus = eHAL_STATUS_SUCCESS;
   }
   
   return( halStatus );
} 


//Caller of this function MUST dynamically allocate memory for pBuf because this funciton will
//free thememory.
eHalStatus palSendMBMessage(tHddHandle hHdd, void *pBuf)
{
   eHalStatus halStatus = eHAL_STATUS_FAILURE;
   tSirRetStatus sirStatus;
   v_CONTEXT_t vosContext;
   v_VOID_t *hHal;
   
   vosContext = vos_get_global_context( VOS_MODULE_ID_HDD, hHdd );
   
   hHal = vos_get_context( VOS_MODULE_ID_HAL, vosContext );
   
   sirStatus = halMmhForwardMBmsg( hHal, pBuf );
   if ( eSIR_SUCCESS == sirStatus )
   {
      halStatus = eHAL_STATUS_SUCCESS;
   }   

   palFreeMemory( hHdd, pBuf );
   
   return( halStatus );
}
  


//All semophore functions are no-op here
//PAL semaphore functions
//All functions MUST return success. If change needs to be made, please check all callers' logic
eHalStatus palSemaphoreAlloc( tHddHandle hHdd, tPalSemaphoreHandle *pHandle, tANI_S32 count )
{
    (void)hHdd;
    (void)pHandle;
    (void)count;
    if(pHandle)
    {
        *pHandle = NULL;
    }

    return (eHAL_STATUS_SUCCESS);
}

eHalStatus palSemaphoreFree( tHddHandle hHdd, tPalSemaphoreHandle hSemaphore )
{
    (void)hHdd;
    (void)hSemaphore;

    return (eHAL_STATUS_SUCCESS);
}

eHalStatus palSemaphoreTake( tHddHandle hHdd, tPalSemaphoreHandle hSemaphore )
{
    (void)hHdd;
    (void)hSemaphore;

    return (eHAL_STATUS_SUCCESS);
}

eHalStatus palSemaphoreGive( tHddHandle hHdd, tPalSemaphoreHandle hSemaphore )
{
    (void)hHdd;
    (void)hSemaphore;

    return (eHAL_STATUS_SUCCESS);
}

eHalStatus palMutexAlloc( tHddHandle hHdd, tPalSemaphoreHandle *pHandle) 
{
    (void)hHdd;
    (void)pHandle;

    if(pHandle)
    {
        *pHandle = NULL;
    }
    return (eHAL_STATUS_SUCCESS);
}

eHalStatus palMutexAllocLocked( tHddHandle hHdd, tPalSemaphoreHandle *pHandle)
{
    (void)hHdd;
    (void)pHandle;

    if(pHandle)
    {
        *pHandle = NULL;
    }
    return (eHAL_STATUS_SUCCESS);
}


eAniBoolean pal_in_interrupt(void)
{
    return (eANI_BOOLEAN_FALSE);
}

void pal_local_bh_disable(void)
{
}

void pal_local_bh_enable(void)
{
}





eHalStatus palOpen(tHddHandle hHdd)
{
   VOS_ASSERT( 0 );
   return( eHAL_STATUS_SUCCESS );
}   

eHalStatus palClose(tHddHandle hHdd)
{
   VOS_ASSERT( 0 );
   return( eHAL_STATUS_SUCCESS );
}   


eHalStatus palPreStart(tHddHandle hddHandle)
{
   VOS_ASSERT( 0 );
   return( eHAL_STATUS_SUCCESS );
}   


eHalStatus palPostStart(tHddHandle hddHandle)
{
   VOS_ASSERT( 0 );
   return( eHAL_STATUS_SUCCESS );
}   


eHalStatus palStop(tHddHandle hddHandle)
{
   VOS_ASSERT( 0 );
   return( eHAL_STATUS_SUCCESS );
}   


eHalStatus palStartHdd(tHddHandle hHdd)
{
   VOS_ASSERT( 0 );
   return( eHAL_STATUS_SUCCESS );
}   


eHalStatus palStopHdd(tHddHandle hHdd)
{
   VOS_ASSERT( 0 );
   return( eHAL_STATUS_SUCCESS );
}   


void palGetUnicastStats(tHddHandle hHdd, tANI_U32 *tx, tANI_U32 *rx)
{
	VOS_TRACE( VOS_MODULE_ID_SYS, VOS_TRACE_LEVEL_ERROR, " *** not implemented yet - HENRI ***\n" );

   //VOS_ASSERT( 0 );
}



void palMacResetDebug( tHddHandle hHdd, tANI_U32 reason )
{
   VOS_ASSERT( 0 );
}


   

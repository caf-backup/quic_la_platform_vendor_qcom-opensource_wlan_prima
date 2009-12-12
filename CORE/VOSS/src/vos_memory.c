/*===========================================================================
  @file vos_memory.c

  @brief Virtual Operating System Services Memory API

  
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
     
===========================================================================*/ 

/*---------------------------------------------------------------------------
 * Include Files
 * ------------------------------------------------------------------------*/

#include "vos_memory.h"

/*---------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * ------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
 * Type Declarations
 * ------------------------------------------------------------------------*/
  
/*---------------------------------------------------------------------------
 * Data definitions
 * ------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
 * External Function implementation
 * ------------------------------------------------------------------------*/

v_VOID_t * vos_mem_malloc( v_SIZE_t size )
{
   if (size > (1024*1024))
   {
      printk(KERN_CRIT "%s: called with arg > 1024K; passed in %d !!!\n", __FUNCTION__,size); 
       return NULL;
   }
   if (in_interrupt())
   {
      printk(KERN_CRIT "%s cannot be called from interrupt context!!!\n", __FUNCTION__);
      return NULL;
   }
   return kmalloc(size, GFP_KERNEL);
}   

v_VOID_t vos_mem_free( v_VOID_t *ptr )
{
    if (ptr == NULL)
      return;
    kfree(ptr);
}

v_VOID_t vos_mem_set( v_VOID_t *ptr, v_SIZE_t numBytes, v_BYTE_t value )
{
   if (ptr == NULL)
   {
      printk(KERN_CRIT "%s called with NULL parameter ptr\n", __FUNCTION__);
      return;
   }
   memset(ptr, value, numBytes);
}

v_VOID_t vos_mem_zero( v_VOID_t *ptr, v_SIZE_t numBytes )
{
   if (ptr == NULL)
   {
      printk(KERN_CRIT "%s called with NULL parameter ptr\n", __FUNCTION__);
      return;
   }
   memset(ptr, 0, numBytes);
   
}

v_VOID_t vos_mem_copy( v_VOID_t *pDst, const v_VOID_t *pSrc, v_SIZE_t numBytes )
{
   if ((pDst == NULL) || (pSrc==NULL))
   {
      printk(KERN_CRIT "%s called with NULL parameter\n", __FUNCTION__);
      return;
   }
   memcpy(pDst, pSrc, numBytes);
}

v_VOID_t vos_mem_move( v_VOID_t *pDst, const v_VOID_t *pSrc, v_SIZE_t numBytes )
{
   if ((pDst == NULL) || (pSrc==NULL))
   {
      printk(KERN_CRIT "%s called with NULL parameter\n", __FUNCTION__);
      return;
   }
   memmove(pDst, pSrc, numBytes);
}

v_BOOL_t vos_mem_compare( v_VOID_t *pMemory1, v_VOID_t *pMemory2, v_U32_t numBytes )
{ 
   if ((pMemory1 == NULL) || (pMemory2==NULL))
   {
      printk(KERN_CRIT "%s called with NULL parameter\n", __FUNCTION__);
      return VOS_FALSE;
   }
   return (memcmp(pMemory1, pMemory2, numBytes)?VOS_FALSE:VOS_TRUE);
}   

/*----------------------------------------------------------------------------
  
  \brief vos_mem_dma_malloc() - vOSS DMA Memory Allocation

  This function will dynamicallly allocate the specified number of bytes of 
  memory. This memory will have special attributes making it DMA friendly i.e.
  it will exist in contiguous, 32-byte aligned uncached memory. A normal 
  vos_mem_malloc does not yield memory with these attributes. 

  NOTE: the special DMA friendly memory is very scarce and this API must be
  used sparingly

  On WM, there is nothing special about this memory. SDHC allocates the 
  DMA friendly buffer and copies the data into it
  
  \param size - the number of bytes of memory to allocate.  
  
  \return Upon successful allocate, returns a non-NULL pointer to the 
  allocated memory.  If this function is unable to allocate the amount of 
  memory specified (for any reason) it returns NULL.
    
  \sa
  
  --------------------------------------------------------------------------*/
v_VOID_t* vos_mem_dma_malloc( v_SIZE_t size )
{
   if (in_interrupt())
   {
      printk(KERN_CRIT "%s cannot be called from interrupt context!!!\n", __FUNCTION__);
      return NULL;
   }
   return kmalloc(size, GFP_KERNEL);
}

/*----------------------------------------------------------------------------
  
  \brief vos_mem_dma_free() - vOSS DMA Free Memory

  This function will free special DMA friendly memory pointed to by 'ptr'.

  On WM, there is nothing special about the memory being free'd. SDHC will
  take care of free'ing the DMA friendly buffer
  
  \param ptr - pointer to the starting address of the memory to be 
               free'd.  
  
  \return Nothing
    
  \sa
  
  --------------------------------------------------------------------------*/
v_VOID_t vos_mem_dma_free( v_VOID_t *ptr )
{
    if (ptr == NULL)
      return;
    kfree(ptr);
}


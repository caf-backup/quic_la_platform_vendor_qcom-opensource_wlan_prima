
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
#include "vos_trace.h"

#ifdef MEMORY_DEBUG
#include "wlan_hdd_dp_utils.h"

hdd_list_t vosMemList;

static v_U8_t WLAN_MEM_HEADER[] =  {0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68 };
static v_U8_t WLAN_MEM_TAIL[]   =  {0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87};

struct s_vos_mem_struct
{
   hdd_list_node_t pNode;
   v_U8_t* fileName;
   unsigned int lineNum;
   unsigned int size;
   v_U8_t header[8];
};
#endif

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
#ifdef MEMORY_DEBUG
void vos_mem_init()
{
   /* Initalizing the list with maximum size of 60000 */	
   hdd_list_init(&vosMemList, 60000);  
   return; 
}

void vos_mem_exit()
{
    v_SIZE_t listSize;
    hdd_list_size(&vosMemList, &listSize);

    if(listSize)
    {
       hdd_list_node_t* pNode;
       VOS_STATUS vosStatus;

       struct s_vos_mem_struct* memStruct;
 
       VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
             "%s: List is not Empty. listSize %d ", __FUNCTION__, (int)listSize);

       do
       {
          vosStatus = hdd_list_remove_front(&vosMemList, &pNode);
          if(VOS_STATUS_SUCCESS == vosStatus)
          {
             v_U8_t* temp;
             memStruct = (struct s_vos_mem_struct*)pNode;
             temp = (v_U8_t*)(memStruct + 1);
             VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                   "Memory Leak@ File %s, @Line %d, size %d", 
                   memStruct->fileName, (int)memStruct->lineNum, memStruct->size);
          }
       }while(vosStatus == VOS_STATUS_SUCCESS);
    }
    
    hdd_list_destroy(&vosMemList);
}

v_VOID_t * vos_mem_malloc_debug( v_SIZE_t size, v_S7_t* fileName, v_U32_t lineNum)
{
   struct s_vos_mem_struct* memStruct;
   v_VOID_t* memPtr = NULL;
   v_SIZE_t new_size;

   if (size > (1024*1024))
   {
       VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
               "%s: called with arg > 1024K; passed in %d !!!", __FUNCTION__,size); 
       return NULL;
   }
   if (in_interrupt())
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
                "%s cannot be called from interrupt context!!!", __FUNCTION__);
      return NULL;
   }

   new_size = size + sizeof(struct s_vos_mem_struct) + 8; 

   memStruct = (struct s_vos_mem_struct*)kmalloc(new_size,GFP_KERNEL);

   if(memStruct != NULL)
   {
      VOS_STATUS vosStatus;

      memStruct->fileName = (v_U8_t*)fileName;
      memStruct->lineNum  = lineNum;
      memStruct->size     = size;

      vos_mem_copy(&memStruct->header[0], &WLAN_MEM_HEADER[0], sizeof(WLAN_MEM_HEADER));
      vos_mem_copy( (v_U8_t*)(memStruct + 1) + size, &WLAN_MEM_TAIL[0], sizeof(WLAN_MEM_TAIL));

      vosStatus = hdd_list_insert_front(&vosMemList, &memStruct->pNode);
      if(VOS_STATUS_SUCCESS != vosStatus)
      {
         VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, 
             "%s: Unable to insert node into List vosStatus %d\n", __FUNCTION__, vosStatus);
      }

      memPtr = (v_VOID_t*)(memStruct + 1); 
   }
   return memPtr;
}

v_VOID_t vos_mem_free( v_VOID_t *ptr )
{
    if (ptr != NULL)
    {
        VOS_STATUS vosStatus;
        struct s_vos_mem_struct* memStruct = ((struct s_vos_mem_struct*)ptr) - 1;

        vosStatus = hdd_list_remove_node(&vosMemList, &memStruct->pNode);

        if(VOS_STATUS_SUCCESS == vosStatus)
        {
            if(0 == vos_mem_compare(memStruct->header, &WLAN_MEM_HEADER[0], sizeof(WLAN_MEM_HEADER)) )
            {
               VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, 
                    "Memory Header is corrupted. MemInfo: Filename %s, LineNum %d", 
                                memStruct->fileName, (int)memStruct->lineNum);
            }
            if(0 == vos_mem_compare( (v_U8_t*)ptr + memStruct->size, &WLAN_MEM_TAIL[0], sizeof(WLAN_MEM_TAIL ) ) )
            {
               VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, 
                    "Memory Trailer is corrupted. MemInfo: Filename %s, LineNum %d", 
                                memStruct->fileName, (int)memStruct->lineNum);
            }
        }
        else
        {
            VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, 
               "%s: Not able to remove node from the list. MemInfo: Filename %s, LineNum %d", 
                    __FUNCTION__, memStruct->fileName, (int)memStruct->lineNum);
        }

        kfree((v_VOID_t*)memStruct);
    }
}
#else
v_VOID_t * vos_mem_malloc( v_SIZE_t size )
{
   if (size > (1024*1024))
   {
       VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, "%s: called with arg > 1024K; passed in %d !!!", __FUNCTION__,size); 
       return NULL;
   }
   if (in_interrupt())
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, "%s cannot be called from interrupt context!!!", __FUNCTION__);
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
#endif

v_VOID_t vos_mem_set( v_VOID_t *ptr, v_SIZE_t numBytes, v_BYTE_t value )
{
   if (ptr == NULL)
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, "%s called with NULL parameter ptr", __FUNCTION__);
      return;
   }
   memset(ptr, value, numBytes);
}

v_VOID_t vos_mem_zero( v_VOID_t *ptr, v_SIZE_t numBytes )
{
   if (ptr == NULL)
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, "%s called with NULL parameter ptr", __FUNCTION__);
      return;
   }
   memset(ptr, 0, numBytes);
   
}

v_VOID_t vos_mem_copy( v_VOID_t *pDst, const v_VOID_t *pSrc, v_SIZE_t numBytes )
{
   if ((pDst == NULL) || (pSrc==NULL))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, "%s called with NULL parameter", __FUNCTION__);
      return;
   }
   memcpy(pDst, pSrc, numBytes);
}

v_VOID_t vos_mem_move( v_VOID_t *pDst, const v_VOID_t *pSrc, v_SIZE_t numBytes )
{
   if ((pDst == NULL) || (pSrc==NULL))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, "%s called with NULL parameter", __FUNCTION__);
      return;
   }
   memmove(pDst, pSrc, numBytes);
}

v_BOOL_t vos_mem_compare( v_VOID_t *pMemory1, v_VOID_t *pMemory2, v_U32_t numBytes )
{ 
   if ((pMemory1 == NULL) || (pMemory2==NULL))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, "%s called with NULL parameter", __FUNCTION__);
      return VOS_FALSE;
   }
   return (memcmp(pMemory1, pMemory2, numBytes)?VOS_FALSE:VOS_TRUE);
}   


v_SINT_t vos_mem_compare2( v_VOID_t *pMemory1, v_VOID_t *pMemory2, v_U32_t numBytes )

{ 
   return( (v_SINT_t) memcmp( pMemory1, pMemory2, numBytes ) );
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
#ifdef MEMORY_DEBUG
v_VOID_t * vos_mem_dma_malloc_debug( v_SIZE_t size, v_U8_t* fileName, v_U32_t lineNum)
{
   struct s_vos_mem_struct* memStruct;
   v_VOID_t* memPtr = NULL;
   v_SIZE_t new_size;

   if (in_interrupt())
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, "%s cannot be called from interrupt context!!!", __FUNCTION__);
      return NULL;
   }

   new_size = size + sizeof(struct s_vos_mem_struct) + 8; 

   memStruct = (struct s_vos_mem_struct*)kmalloc(new_size,GFP_KERNEL);

   if(memStruct != NULL)
   {
      VOS_STATUS vosStatus;

      memStruct->fileName = fileName;
      memStruct->lineNum  = lineNum;
      memStruct->size     = size;

      vos_mem_copy(&memStruct->header[0], &WLAN_MEM_HEADER[0], sizeof(WLAN_MEM_HEADER));
      vos_mem_copy( (v_U8_t*)(memStruct + 1) + size, &WLAN_MEM_TAIL[0], sizeof(WLAN_MEM_TAIL));

      vosStatus = hdd_list_insert_front(&vosMemList, &memStruct->pNode);
      if(VOS_STATUS_SUCCESS != vosStatus)
      {
         VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, 
             "%s: Unable to insert node into List vosStatus %d\n", __FUNCTION__, vosStatus);
      }

      memPtr = (v_VOID_t*)(memStruct + 1); 
   }

   return memPtr;
}

v_VOID_t vos_mem_dma_free( v_VOID_t *ptr )
{
    if (ptr != NULL)
    {
        VOS_STATUS vosStatus;
        struct s_vos_mem_struct* memStruct = ((struct s_vos_mem_struct*)ptr) - 1;

        vosStatus = hdd_list_remove_node(&vosMemList, &memStruct->pNode);

        if(VOS_STATUS_SUCCESS == vosStatus)
        {
            if(0 == vos_mem_compare(memStruct->header, &WLAN_MEM_HEADER[0], sizeof(WLAN_MEM_HEADER)) )
            {
               VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, 
                    "Memory Header is corrupted. MemInfo: Filename %s, LineNum %d", 
                                memStruct->fileName, (int)memStruct->lineNum);
            }
            if(0 == vos_mem_compare( (v_U8_t*)ptr + memStruct->size, &WLAN_MEM_TAIL[0], sizeof(WLAN_MEM_TAIL ) ) )
            {
               VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, 
                    "Memory Trailer is corrupted. MemInfo: Filename %s, LineNum %d", 
                                memStruct->fileName, (int)memStruct->lineNum);
            }
        }
        else
        {
            VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, 
               "%s: Not able to remove node from the list. MemInfo: Filename %s, LineNum %d", 
                    __FUNCTION__, memStruct->fileName, (int)memStruct->lineNum);
        }
        kfree((v_VOID_t*)memStruct);
    }
}
#else
v_VOID_t* vos_mem_dma_malloc( v_SIZE_t size )
{
   if (in_interrupt())
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR, "%s cannot be called from interrupt context!!!", __FUNCTION__);
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
#endif

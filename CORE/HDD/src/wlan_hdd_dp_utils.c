/**=============================================================================
  wlan_hdd_dp_utils.c
  
  \brief      Utility functions for data path module
  
  Description...
               Copyright 2008 (c) Qualcomm, Incorporated.
               All Rights Reserved.
               Qualcomm Confidential and Proprietary.
  
  ==============================================================================**/
/* $HEADER$ */
  
/**-----------------------------------------------------------------------------
  Include files
  ----------------------------------------------------------------------------*/
#include <wlan_hdd_dp_utils.h>

/**-----------------------------------------------------------------------------
  Preprocessor definitions and constants
 ----------------------------------------------------------------------------*/
  
/**-----------------------------------------------------------------------------
  Type declarations
 ----------------------------------------------------------------------------*/
  
/**-----------------------------------------------------------------------------
  Function declarations and documenation
 ----------------------------------------------------------------------------*/


VOS_STATUS hdd_list_insert_front( hdd_list_t *pList, hdd_list_node_t *pNode )
{
   list_add( pNode, &pList->anchor );
   pList->count++;
   return VOS_STATUS_SUCCESS;
}

VOS_STATUS hdd_list_insert_back( hdd_list_t *pList, hdd_list_node_t *pNode )
{
   list_add_tail( pNode, &pList->anchor );
   pList->count++;
   return VOS_STATUS_SUCCESS;
}

VOS_STATUS hdd_list_insert_back_size( hdd_list_t *pList, hdd_list_node_t *pNode, v_SIZE_t *pSize )
{
   list_add_tail( pNode, &pList->anchor );
   pList->count++;
   *pSize = pList->count;
   return VOS_STATUS_SUCCESS;
}

VOS_STATUS hdd_list_remove_front( hdd_list_t *pList, hdd_list_node_t **ppNode )
{
   struct list_head * listptr;

   if ( list_empty( &pList->anchor ) )
   {
      return VOS_STATUS_E_EMPTY;
   }
         
   listptr = pList->anchor.next;
   *ppNode = listptr;
   list_del(pList->anchor.next);
   pList->count--;

   return VOS_STATUS_SUCCESS;
}

VOS_STATUS hdd_list_remove_back( hdd_list_t *pList, hdd_list_node_t **ppNode )
{
   struct list_head * listptr;

   if ( list_empty( &pList->anchor ) )
   {
      return VOS_STATUS_E_EMPTY;
   }

   listptr = pList->anchor.prev;
   *ppNode = listptr;
   list_del(pList->anchor.prev);
   pList->count--;

   return VOS_STATUS_SUCCESS;
}

VOS_STATUS hdd_list_remove_node( hdd_list_t *pList, hdd_list_node_t *pNodeToRemove )
{
   hdd_list_node_t *tmp;
   int found = 0;

   if ( list_empty( &pList->anchor ) )
   {
      return VOS_STATUS_E_EMPTY;
   }

    // verify that pNodeToRemove is indeed part of list pList
   list_for_each(tmp, &pList->anchor) 
   {
     if (tmp == pNodeToRemove)
     {
        found = 1;
        break;
     }
   }
   if (found == 0)
       return VOS_STATUS_E_INVAL;

   list_del(pNodeToRemove); 
   pList->count--;

   return VOS_STATUS_SUCCESS;
}

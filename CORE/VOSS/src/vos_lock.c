/*============================================================================
  FILE:         vos_lock.c

  OVERVIEW:     This source file contains definitions for vOS lock APIs
                The four APIs mentioned in this file are used for 
                initializing , acquiring, releasing and destroying a lock.
                the lock are implemented using critical sections

  DEPENDENCIES: 
 
                Copyright (c) 2007 QUALCOMM Incorporated.
                All Rights Reserved.
                Qualcomm Confidential and Proprietary
============================================================================*/

/*============================================================================
  EDIT HISTORY FOR MODULE

============================================================================*/

/*----------------------------------------------------------------------------
 * Include Files
 * -------------------------------------------------------------------------*/

#include "vos_lock.h"
#include "vos_memory.h"

/*----------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * -------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * Type Declarations
 * -------------------------------------------------------------------------*/

#define LINUX_LOCK_COOKIE 0x12345678
enum
{
   LOCK_RELEASED = 0x11223344,
   LOCK_ACQUIRED
};

/*----------------------------------------------------------------------------
 * Global Data Definitions
 * -------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * Static Variable Definitions
 * -------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
   Function Definitions and Documentation
 * -------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------
  
  \brief vos_lock_init() - initializes a vOSS lock
  
  The vos_lock_init() function initializes the specified lock. Upon 
  successful initialization, the state of the lock becomes initialized 
  and unlocked.

  A lock must be initialized by calling vos_lock_init() before it 
  may be used in any other lock functions. 
  
  Attempting to initialize an already initialized lock results in 
  a failure.
 
  \param lock - pointer to the opaque lock object to initialize
  
  \return VOS_STATUS_SUCCESS - lock was successfully initialized and 
          is ready to be used.

          VOS_STATUS_E_NOMEM - insufficient memory exists to initialize 
          the lock

          VOS_STATUS_E_BUSY - The implementation has detected an attempt 
          to reinitialize the object referenced by lock, a previously 
          initialized, but not yet destroyed, lock.

          VOS_STATUS_E_FAULT  - lock is an invalid pointer.   

          VOS_STATUS_E_FAILURE - default return value if it fails due to 
          unknown reasons

       ***VOS_STATUS_E_RESOURCES - System resources (other than memory) 
          are unavailable to initilize the lock
  \sa
   
    ( *** return value not considered yet )
  --------------------------------------------------------------------------*/
VOS_STATUS vos_lock_init ( vos_lock_t *lock )
{
	
   //check for invalid pointer
   if ( lock == NULL)
   {
       printk(KERN_CRIT "%s: NULL pointer passed in\n",__FUNCTION__);
       return VOS_STATUS_E_FAULT; 
   }
   // check for 'already initialized' lock
   if ( LINUX_LOCK_COOKIE == lock->cookie )
   {
       printk(KERN_CRIT "%s: already initialized lock\n",__FUNCTION__);
       return VOS_STATUS_E_BUSY;
   }
      
   if (in_interrupt())
   {
      printk(KERN_CRIT "%s cannot be called from interrupt context!!!\n", __FUNCTION__);
      return VOS_STATUS_E_FAULT; 
   }
      
   // initialize new lock 
   mutex_init( &lock->m_lock ); 
   lock->cookie = LINUX_LOCK_COOKIE;
   lock->state  = LOCK_RELEASED;
   lock->processID = 0;
   lock->refcount = 0;
      
   return VOS_STATUS_SUCCESS;
}

/*--------------------------------------------------------------------------
  
  \brief vos_lock_acquire() - acquires a lock

  A lock object is acquired by calling \a vos_lock_acquire().  If the lock 
  is already locked, the calling thread shall block   until the lock becomes 
  available. This operation shall return with the lock object referenced by 
  lock in the locked state with the calling thread as its owner. 
  
  \param lock - the lock object to acquire
  
  \return VOS_STATUS_SUCCESS - the lock was successfully acquired by 
          the calling thread.
  
          VOS_STATUS_E_INVAL - The value specified by lock does not refer 
          to an initialized lock object.
          
          VOS_STATUS_E_FAULT  - lock is an invalid pointer. 

          VOS_STATUS_E_FAILURE - default return value if it fails due to 
          unknown reasons
          
  \sa
  ------------------------------------------------------------------------*/
VOS_STATUS vos_lock_acquire ( vos_lock_t* lock )
{
      int rc;
      //Check for invalid pointer
      if ( lock == NULL )
      {
         printk(KERN_CRIT "%s: NULL pointer passed in\n",__FUNCTION__);
         return VOS_STATUS_E_FAULT;
      }
      // check if lock refers to an initialized object
      if ( LINUX_LOCK_COOKIE != lock->cookie )
      {
         printk(KERN_CRIT "%s: uninitialized lock\n",__FUNCTION__);
         return VOS_STATUS_E_INVAL;
      }
	      
      if (in_interrupt())
      {
         printk(KERN_CRIT "%s cannot be called from interrupt context!!!\n", __FUNCTION__);
         return VOS_STATUS_E_FAULT; 
      }
      if ((lock->processID == current->pid) && 
          (lock->state == LOCK_ACQUIRED))
      {
         lock->refcount++;
#ifdef VOS_NESTED_LOCK_DEBUG
         printk("%s: %x %d %d\n", __func__, lock, current->pid, lock->refcount);
#endif
         return VOS_STATUS_SUCCESS;
      }
      // Acquire a Lock
      rc = mutex_lock_interruptible( &lock->m_lock ); 
 
      if (rc)
        return rc;
      
#ifdef VOS_NESTED_LOCK_DEBUG
      printk("%s: %x %d\n", __func__, lock, current->pid);
#endif
      lock->processID = current->pid;
      lock->refcount++;
      lock->state    = LOCK_ACQUIRED;
      return VOS_STATUS_SUCCESS;
}


/*--------------------------------------------------------------------------
  
  \brief vos_lock_release() - releases a lock

  The \a vos_lock_release() function shall release the lock object 
  referenced by 'lock'.  

  If a thread attempts to release a lock that it unlocked or is not
  initialized, an error is returned. 

  \param lock - the lock to release
  
  \return VOS_STATUS_SUCCESS - the lock was successfully released
  
          VOS_STATUS_E_INVAL - The value specified by lock does not refer 
          to an initialized lock object.
                   
          VOS_STATUS_E_FAULT - The value specified by lock does not refer 
          to an initialized lock object.
                   
          VOS_STATUS_E_PERM - Operation is not permitted.  The calling 
          thread does not own the lock. 

          VOS_STATUS_E_FAILURE - default return value if it fails due to 
          unknown reasons
    
  \sa
  ------------------------------------------------------------------------*/
VOS_STATUS vos_lock_release ( vos_lock_t *lock )
{
      //Check for invalid pointer
      if ( lock == NULL )
      {
         printk(KERN_CRIT "%s: NULL pointer passed in\n",__FUNCTION__);
         return VOS_STATUS_E_FAULT;
      }
		
      // check if lock refers to an uninitialized object
      if ( LINUX_LOCK_COOKIE != lock->cookie )
      {
         printk(KERN_CRIT "%s: uninitialized lock\n",__FUNCTION__);
         return VOS_STATUS_E_INVAL;
      }
        
      // CurrentThread = GetCurrentThreadId(); 
      // Check thread ID of caller against thread ID
      // of the thread which acquire the lock
      if ( lock->processID != current->pid )
      {
         printk(KERN_CRIT "%s: current task pid does not match original task pid!!\n",__FUNCTION__);
#ifdef VOS_NESTED_LOCK_DEBUG
         printk("%s: Lock held by=%d being released by=%d\n", __func__, lock->processID, current->pid);
#endif

         return VOS_STATUS_E_PERM;
      }
      if ((lock->processID == current->pid) && 
          (lock->state == LOCK_ACQUIRED))
      {
         if (lock->refcount > 0) lock->refcount--;
      }
#ifdef VOS_NESTED_LOCK_DEBUG
      printk("%s: %x %d %d\n", __func__, lock, lock->processID, lock->refcount);
#endif
      if (lock->refcount) return VOS_STATUS_SUCCESS;
         
      lock->processID = 0;
      lock->refcount = 0;
      lock->state = LOCK_RELEASED;
      // Release a Lock   
      mutex_unlock( &lock->m_lock );
#ifdef VOS_NESTED_LOCK_DEBUG
      printk("%s: Freeing lock %x %d %d\n", lock, lock->processID, lock->refcount);
#endif
      return VOS_STATUS_SUCCESS;
}

/*--------------------------------------------------------------------------
  
  \brief vos_lock_destroy() - Destroys a vOSS Lock - probably not required
  for Linux. It may not be required for the caller to destroy a lock after
  usage.

  The \a vos_lock_destroy() function shall destroy the lock object 
  referenced by lock.  After a successful return from \a vos_lock_destroy()
  the lock object becomes, in effect, uninitialized.
   
  A destroyed lock object can be reinitialized using vos_lock_init(); 
  the results of otherwise referencing the object after it has been destroyed 
  are undefined.  Calls to vOSS lock functions to manipulate the lock such
  as vos_lock_acquire() will fail if the lock is destroyed.  Therefore, 
  don't use the lock after it has been destroyed until it has 
  been re-initialized.
  
  \param lock - the lock object to be destroyed.
  
  \return VOS_STATUS_SUCCESS - lock was successfully destroyed.
  
          VOS_STATUS_E_BUSY - The implementation has detected an attempt 
          to destroy the object referenced by lock while it is locked 
          or still referenced. 

          VOS_STATUS_E_INVAL - The value specified by lock is invalid.
          
          VOS_STATUS_E_FAULT  - lock is an invalid pointer. 

          VOS_STATUS_E_FAILURE - default return value if it fails due to 
          unknown reasons
  \sa
  ------------------------------------------------------------------------*/
VOS_STATUS vos_lock_destroy( vos_lock_t *lock )
{
      //Check for invalid pointer
      if ( NULL == lock )
      { 	
         printk(KERN_CRIT "%s: NULL pointer passed in\n",__FUNCTION__);
         return VOS_STATUS_E_FAULT; 
      }
      if ( LINUX_LOCK_COOKIE != lock->cookie )
      {	
         printk(KERN_CRIT "%s: uninitialized lock\n",__FUNCTION__);
         return VOS_STATUS_E_INVAL;
      }
	     // check if lock is released		   
      if (LOCK_RELEASED != lock->state)
      {
         printk(KERN_CRIT "%s: lock is not released\n",__FUNCTION__);
         return VOS_STATUS_E_BUSY;
      }

      vos_mem_zero(lock, sizeof(vos_lock_t));
         
      return VOS_STATUS_SUCCESS;
}

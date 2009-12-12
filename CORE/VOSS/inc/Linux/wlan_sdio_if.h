#ifndef WLAN_SDIO_IF_H
#define WLAN_SDIO_IF_H

/**=========================================================================
  
  @file  wlan_sdio_if.h
  
  @brief WLAN SDIO IF LAYER API
               
   Copyright (c) 2008 QUALCOMM Incorporated. All Rights Reserved.
   Qualcomm Confidential and Proprietary
========================================================================*/

/*===========================================================================

                      EDIT HISTORY FOR FILE


  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.


  $Header:$ $DateTime: $ $Author: $


when           who        what, where, why
--------    ---         ----------------------------------------------------------

===========================================================================*/

/*===========================================================================

                          INCLUDE FILES FOR MODULE

===========================================================================*/

/*----------------------------------------------------------------------------
 * Include Files
 * -------------------------------------------------------------------------*/
#include <linux/kthread.h>
#include <linux/mmc/card.h>
#include <linux/mmc/host.h>
#include <linux/mmc/sdio_func.h>

#ifdef __cplusplus
extern "C" {
#endif

/*----------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * -------------------------------------------------------------------------*/
#define LIBRA_MAN_ID              0x70
#define LIBRA_FUNC_ENABLE_TIMEOUT 10000 // 10 sec
#define WLANSAL_MAX_BLOCK_SIZE    128

#ifdef SDIO_DEBUG
#define SDENTER()   printk("%s: entered %d\n",__func__, __LINE__);
#define SDASSERT(a)  BUG_ON(!(a)) 
#else
#define SDENTER()   
#define SDASSERT(a)
#endif

#define TRUE  1
#define FALSE 0

//#define CREATE_WLAN_KTHREAD 1
/*----------------------------------------------------------------------------
 *  Type Declarations
 * -------------------------------------------------------------------------*/
typedef void (* SDIO_FUNCPTR)( struct sdio_func * ); 

typedef struct QWLAN_SDIOInfoStruct {
   SDIO_FUNCPTR func_drv_probe;
   SDIO_FUNCPTR func_drv_remove;

   struct sdio_func *func;

#ifdef CREATE_WLAN_KTHREAD
   struct task_struct *save_sdio_irq_thread;
   atomic_t kt_abort; // signal the IRQ thread to stop
#endif

} QWLAN_SDIOINFO;

static QWLAN_SDIOINFO QWLAN_SDIOInfo;

#ifdef CREATE_WLAN_KTHREAD
static DECLARE_COMPLETION(wlan_thread_exit); // Complete signalling

/*----------------------------------------------------------------------------

   @brief  Rx IRQ Kernel Thread

   @param  struct sdio_func * 

   @return Will keep running until the thread is stopped ideally.
           Keeps running in a while loop waiting for interrupt.
           Thread will stop when IRQ thread is stopped.
        
----------------------------------------------------------------------------*/
static int wlan_sdio_irq_thread_func(void *wlan_fn)
{
   struct sched_param param = { .sched_priority = 1 };
   struct sdio_func *func = (struct sdio_func *)wlan_fn;

   sched_setscheduler(current, SCHED_FIFO, &param);


   pr_err("%s: IRQ thread started \n", __func__);

   do {
      set_current_state(TASK_INTERRUPTIBLE);
      if (!kthread_should_stop()) {
         //pr_err("%s: IRQ thread sleeping, calling schedule\n", __func__);
         // Wait to be woken up by the IRQ event only.
         func->card->host->ops->enable_sdio_irq(func->card->host, 1);
         schedule();
         func->card->host->ops->enable_sdio_irq(func->card->host, 0);
         // If Remove module is called, this abort will be set
         if (0 != atomic_read(&QWLAN_SDIOInfo.kt_abort))
         {
            func->card->host->ops->enable_sdio_irq(func->card->host, 0);
            pr_err("%s: IRQ thread abort != 0\n", __func__);
            complete_and_exit(&wlan_thread_exit, 0);
            return 0;
         }
      }
      set_current_state(TASK_RUNNING);
      if (func->irq_handler) 
      {
         func->irq_handler(func);
      }
   } while (!kthread_should_stop());

   func->card->host->ops->enable_sdio_irq(func->card->host, 0);

   pr_err("%s: IRQ thread exiting \n", __func__);

   return 0;
}
#endif

/*----------------------------------------------------------------------------

   @brief QWLAN_SDIO_Configure is used to configure the SDIO characteristics

   @param The interrupt handler 

   @return -1 on failure
           0  on success.
        
----------------------------------------------------------------------------*/
static inline int QWLAN_SDIO_Configure
(
   sdio_irq_handler_t QWLAN_RxHandler,
   void (* PmuFixFn)(int *status)
)
{
   int err_ret = 0; 
   struct sdio_func *func = QWLAN_SDIOInfo.func;

   if (QWLAN_SDIOInfo.func == NULL) {
      printk("%s: Error function driver NULL\n", __func__);
      return -1;
   }

   printk("%s: Card detected already and func=%d func=%08x\n", __func__, (func)->num, (unsigned int)func);

   sdio_claim_host(func);


   // Currently block sizes are set here.
   func->max_blksize = WLANSAL_MAX_BLOCK_SIZE;
   if (sdio_set_block_size(func, WLANSAL_MAX_BLOCK_SIZE)) {
      printk("%s: Unable to set the block size.\n", __func__);
   }

   (*PmuFixFn)(&err_ret);
   if (err_ret)
   {
      printk("%s: Unable to fix PMU unblocked\n", __func__);
      sdio_release_host(func);
      return -1;
   }

   // We set this as its not setup by the libra cis currently.
   func->enable_timeout = LIBRA_FUNC_ENABLE_TIMEOUT;
   if ((err_ret = sdio_enable_func(func))) {
      printk("%s: Unable to enable function %d\n", __func__, err_ret);
      sdio_release_host(func);
      return -1;
   }

   // Setup the Rx IRQ Handler
   if (sdio_claim_irq(func, QWLAN_RxHandler)) {
      sdio_disable_func(func);
      printk("%s: Unable to claim irq.\n", __func__);
      sdio_release_host(func);
      return -1;
   }

#ifdef CREATE_WLAN_KTHREAD
   // Lets override the thread.
   QWLAN_SDIOInfo.save_sdio_irq_thread = func->card->host->sdio_irq_thread;
   atomic_set(&(QWLAN_SDIOInfo.kt_abort), 0); // Set only when we abort the thread 
   func->card->host->sdio_irq_thread = 
      kthread_run(wlan_sdio_irq_thread_func, func, "wlanksdiorqd");
#else
   sdio_release_host(func);
   printk("%s: Module SDIO probe called success %d %d.\n",
      __func__, ((struct mmc_card *)func->card)->num_info, err_ret);
#endif

   return 0;
}

/*----------------------------------------------------------------------------

   @brief Register the WLAN registers a probe and remove function with the 
          SDIO interface layer. 

   @param Two function pointers resgistered by the WLAN driver. These functions
          are called when the card is detected, probed and when the card is removed

   @return 
        
----------------------------------------------------------------------------*/
static void QWLAN_SDIO_RegisterProbe
(
   SDIO_FUNCPTR func_drv_probe,
   SDIO_FUNCPTR func_drv_remove
)
{
    QWLAN_SDIOInfo.func_drv_probe = func_drv_probe;
    QWLAN_SDIOInfo.func_drv_remove = func_drv_remove;

    printk("%s: Functions registered with SDIO\n", __func__);

    return ; 
}


/*----------------------------------------------------------------------------

   @brief Setup the WLAN Adapter   as the private data of the sdio dev.
   This will be used in the Rx handler

   @param sdio device
   @param wlan adapter. 

   @return  none
        
----------------------------------------------------------------------------*/
static void QWLAN_SDIO_SetPrivData
(
   struct sdio_func *sdio_func_dev,
   void *pAdapter
)
{
   sdio_set_drvdata(sdio_func_dev, pAdapter);
}

/*----------------------------------------------------------------------------

   @brief  Gives the WLAN adapter when passed the sdio function device

   @param the sdio function device

   @return 
        
----------------------------------------------------------------------------*/
static void *QWLAN_SDIO_GetPrivData
(
   struct sdio_func *sdio_func_dev
)
{
   return(sdio_get_drvdata(sdio_func_dev));
}


/*----------------------------------------------------------------------------

   @brief  Id table for the Libra card.
----------------------------------------------------------------------------*/
static const struct sdio_device_id QWLAN_SDIOId[] = {
    {.class = 0, .vendor = LIBRA_MAN_ID, .device = 0},
    {}
};

/*----------------------------------------------------------------------------

   @brief Probe routine registered with the SD/MMC Bus driver for the Libra card. 

   @param struct sdio_func * 
          
   @return General status code 0
        
----------------------------------------------------------------------------*/
int QWLAN_SDIO_probe(struct sdio_func *func, 
    const struct sdio_device_id *sdio_dev_id)
{
 
   printk("%s: Probe called with %d %d.\n",
       __func__, ((struct mmc_card *)func->card)->num_info, func->num);

   QWLAN_SDIOInfo.func = func;
   func->card->host->sdio_irq_thread = NULL;


   if (QWLAN_SDIOInfo.func_drv_probe) {
       printk("%s: function drv probe registered CALLED.\n", __func__);
       QWLAN_SDIOInfo.func_drv_probe(func);
   }
   else {
       printk("%s: No function drv probe registered as yet.\n", __func__);
   }

   printk("%s: Module SDIO probe called success %d %x %d multi-mode=%d.\n",
       __func__, ((struct mmc_card *)func->card)->num_info, (unsigned int)func, 
          (int)(((struct mmc_card *)func->card)->host->caps & MMC_CAP_NEEDS_POLL),
          func->card->cccr.multi_block);
   printk("%s: Module SDIO probe called success max_seg_size=%d max_blk_size=%d max_blksize=%d.\n",
	__func__, func->card->host->max_seg_size, func->card->host->max_blk_size, func->max_blksize);
   printk("%s: Module SDIO probe called success  %d\n", __func__, func->cur_blksize);

   return 0;
}

/*----------------------------------------------------------------------------

   @brief function registered with SD/MMC Bus driver to be run when card is 
      removed or called with SAL is stopped.

   @param struct sdio_func *.

   @return void
        
----------------------------------------------------------------------------*/
void QWLAN_SDIO_remove(struct sdio_func *func)
{
    printk("%s : Module remove called.\n", __func__);

    // If we have a registered WLAN drv remove function, call it now.
    if (QWLAN_SDIOInfo.func_drv_remove) {
        QWLAN_SDIOInfo.func_drv_remove(func);
    }
#ifdef CREATE_WLAN_KTHREAD

    if (QWLAN_SDIOInfo.save_sdio_irq_thread == NULL) 
    {
       printk("%s : New Module remove done.\n", __func__);
       return;
    }

    // Stop our hacked kernel thread.
    // Current workaround to restore the kernel thread.
    atomic_set(&(QWLAN_SDIOInfo.kt_abort), 1); // Set only when we abort the thread 
    mmc_signal_sdio_irq(func->card->host);
    printk("%s : Waiting for kthread exit.\n", __func__);
    wait_for_completion(&wlan_thread_exit);
    printk("%s : Restoring kthread.\n", __func__);

    func->card->host->sdio_irq_thread = QWLAN_SDIOInfo.save_sdio_irq_thread;
#else
    sdio_claim_host(func);
#endif
    sdio_release_irq(func);
    sdio_disable_func(func);
    sdio_release_host(func);
    printk("%s : Module remove exiting.\n", __func__);
}

/*----------------------------------------------------------------------------

   @brief SDIO Device Table for Probe/Remove/ID provided to the SD/MMC core 

----------------------------------------------------------------------------*/
static struct sdio_driver QWLAN_SDIOdriver = {
    .name      = "qwlansdio",
    .id_table  = QWLAN_SDIOId,
    .probe     = QWLAN_SDIO_probe,
    .remove    = QWLAN_SDIO_remove,
};

/*----------------------------------------------------------------------------

   @brief Function that will register with the SD/MMC Bus driver.

   @param void

   @return void 
        
----------------------------------------------------------------------------*/
static inline void QWLAN_SDIO_RegisterIf
(
 void
)
{
   // Register with the sd/mmc bus driver. 
   sdio_register_driver(&QWLAN_SDIOdriver);

}

/*----------------------------------------------------------------------------

   @brief FUnction that will unregister itself from the SD/MMC 

   @param void

   @return void
        
----------------------------------------------------------------------------*/
static inline void QWLAN_SDIO_UnRegisterIf
(
 void
)
{
   // Unregister with the sd/mmc bus driver. 
   sdio_unregister_driver(&QWLAN_SDIOdriver);

   return; 
}

/*----------------------------------------------------------------------------

   @brief  The function provides the SD device handle 

   @param  void 

   @return Returns the sdio device handle 
        
----------------------------------------------------------------------------*/
static inline struct sdio_func *QWLAN_GetSDIO_FuncDev
(
 void
)
{
    return (QWLAN_SDIOInfo.func);
}


/*----------------------------------------------------------------------------

   @brief  The function to read a byte on Function 0, register.

   @param  struct sdio_func *, the address to read, the status of the
           transaction. 

   @return Returns the value read
   and the error value if any in err_ret
        
----------------------------------------------------------------------------*/
static inline u8 QWLAN_SDIO_Read52
(
   struct sdio_func *sdio_func_dev,
   unsigned int     addr,
   int              *err_ret
)
{
   return (sdio_readb(sdio_func_dev, addr, err_ret));
}

/*----------------------------------------------------------------------------

   @brief  The function to write a byte on Function 0, register.

   @param  struct sdio_func *, the data to write, the address to write, 
           the status the transaction. 

   @return void
        
----------------------------------------------------------------------------*/
static void QWLAN_SDIO_Write52
(
   struct sdio_func *func, 
   u8 b,
   unsigned int addr, 
   int *err_ret
)
{
   sdio_writeb(func, b, addr, err_ret);
}

/*----------------------------------------------------------------------------

   @brief Read from the FIFO of a SDIO device

   @param struct sdio_func *, dst the buffer to read into, address to read from,
      Count is the no. of bytes to read.

   @return the byte read is returned 
        
----------------------------------------------------------------------------*/
static inline u8 QWLAN_SDIO_ReadSB
(
   struct sdio_func *func, 
   void *dst, 
   unsigned int addr,
   int count
)
{
   return (sdio_readsb(func, dst, addr, count));
}

/*----------------------------------------------------------------------------

   @brief Read a bunch of memory for a SDIO function.

   @param struct sdio_func *, the dest to read into, addr, count

   @return 
        
----------------------------------------------------------------------------*/
static inline int QWLAN_SDIO_memcpy_fromio
(
   struct sdio_func *func, 
   void *dst,
   unsigned int addr, 
   int count
)
{
   SDENTER();

   return(sdio_memcpy_fromio(func, dst, addr, count));
}

/*----------------------------------------------------------------------------

   @brief Write to a FIFO for a SDIO function.

   @param struct sdio_func *, addr,  src, count

   @return 
        
----------------------------------------------------------------------------*/
static inline int QWLAN_SDIO_WriteSB
(
   struct sdio_func *func,
   unsigned int addr,
   void *src,
   int count
)
{
   return(sdio_writesb(func, addr, src, count));
}
/*----------------------------------------------------------------------------

   @brief Write to a bunch of memory for a SDIO function.

   @param struct sdio_func *, addr, src the address to copy from, count

   @return 
        
----------------------------------------------------------------------------*/
static inline int QWLAN_SDIO_memcpy_toio
(
   struct sdio_func *func, 
   unsigned int addr,
   void *src, 
   int count
)
{
   return(sdio_memcpy_toio(func, addr, src, count));
}

/*=========================================================================
 * END Interactions with SSC
 *=========================================================================*/ 
#ifdef __cplusplus
}
#endif
#endif /* WLAN_SDIO_IF_H */


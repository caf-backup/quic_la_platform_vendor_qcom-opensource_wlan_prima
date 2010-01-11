/*!
  @file
  vos_power.c

  @brief
  This is the interface to VOSS power APIs using for power management 
  of the WLAN Libra module from the MSM PMIC. These implementation of 
  these APIs is very target dependent, also these APIs should only be
  used when the WLAN Libra module is powered from the MSM PMIC and not
  from an external independent power source

*/

/*===========================================================================

  Copyright (c) 2008 QUALCOMM Incorporated. All Rights Reserved

  Qualcomm Proprietary

  Export of this technology or software is regulated by the U.S. Government.
  Diversion contrary to U.S. law prohibited.

  All ideas, data and information contained in or disclosed by
  this document are confidential and proprietary information of
  QUALCOMM Incorporated and all rights therein are expressly reserved.
  By accepting this material the recipient agrees that this material
  and the information contained therein are held in confidence and in
  trust and will not be used, copied, reproduced in whole or in part,
  nor its contents revealed in any manner to others without the express
  written permission of QUALCOMM Incorporated.

===========================================================================*/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

$Header: $

when       who     what, where, why
--------   ---     ----------------------------------------------------------

===========================================================================*/

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/
#include <vos_power.h>
#include <mach/mpp.h>
#include <mach/vreg.h>
#include <linux/err.h>
#include <linux/delay.h>

#ifdef MSM_PLATFORM_7x30
#include <linux/mfd/pmic8058.h>
#endif

/*===========================================================================

                        DEFINITIONS AND TYPES

===========================================================================*/

#define CHIP_POWER_ON         1
#define CHIP_POWER_OFF        0

#ifdef MSM_PLATFORM_7x30
#define PMIC_VREG_WLAN2_LEVEL	2500
#define PMIC_VREG_S2_LEVEL	   1300
#define PMIC_VREG_S4_LEVEL	   2200
#define PMIC_VREG_WLAN_LEVEL	2900

struct wlan_pm8058_gpio {
  int gpio_num;
  struct pm8058_gpio gpio_cfg;
};

//PMIC8058 GPIO COnfiguration for QRF8600 bringup on 7x30 FFA/SURF
static struct wlan_pm8058_gpio wlan_gpios_power_on[] = {
  {20,{PM_GPIO_DIR_IN,  PM_GPIO_OUT_BUF_CMOS, 0, PM_GPIO_PULL_DN,    2, PM_GPIO_STRENGTH_NO,   PM_GPIO_FUNC_NORMAL, 0}},
  {21,{PM_GPIO_DIR_OUT, PM_GPIO_OUT_BUF_CMOS, 0, PM_GPIO_PULL_NO,    0, PM_GPIO_STRENGTH_HIGH, PM_GPIO_FUNC_PAIRED, 0}},
  {22,{PM_GPIO_DIR_OUT, PM_GPIO_OUT_BUF_CMOS, 1, PM_GPIO_PULL_NO,    2, PM_GPIO_STRENGTH_HIGH, PM_GPIO_FUNC_NORMAL, 0}},
  {30,{PM_GPIO_DIR_IN,  PM_GPIO_OUT_BUF_CMOS, 0, PM_GPIO_PULL_DN,    2, PM_GPIO_STRENGTH_NO,   PM_GPIO_FUNC_NORMAL, 0}},
  {31,{PM_GPIO_DIR_OUT, PM_GPIO_OUT_BUF_CMOS, 1, PM_GPIO_PULL_NO,    0, PM_GPIO_STRENGTH_HIGH, PM_GPIO_FUNC_PAIRED, 0}},
  {26,{PM_GPIO_DIR_IN,  PM_GPIO_OUT_BUF_CMOS, 1, PM_GPIO_PULL_UP_30, 2, PM_GPIO_STRENGTH_NO,   PM_GPIO_FUNC_NORMAL, 0}},
};

//PMIC8058 GPIO COnfiguration for QRF8600 shutdown on 7x30 FFA/SURF
//FIXME Do we need to confgure other GPIOS as well? 
static struct wlan_pm8058_gpio wlan_gpios_power_off[] = {
  {22,{PM_GPIO_DIR_OUT, PM_GPIO_OUT_BUF_CMOS, 0, PM_GPIO_PULL_NO,    2, PM_GPIO_STRENGTH_LOW, PM_GPIO_FUNC_NORMAL, 0}},
};

//Helper routine to power up QRF8600 on 7x30 FFA/SURF
int vos_chip_power_qrf8600(int on)
{
   int rc;

   struct vreg *vreg_wlan2 = NULL;
   struct vreg *vreg_s2 = NULL;
   struct vreg *vreg_s4 = NULL;
   struct vreg *vreg_wlan = NULL;

   printk(KERN_CRIT "%s: Power up QRF8600 WLAN Module ? %d\n",
       __func__, on);

   //2.5v Analog from LDO19
   vreg_wlan2 = vreg_get(NULL, "wlan2");
   if (IS_ERR(vreg_wlan2)) {
      printk(KERN_ERR "%s: wlan2 vreg get failed (%ld)\n",
         __func__, PTR_ERR(vreg_wlan2));
      return PTR_ERR(vreg_wlan2);
   }

   //1.3v RF; gated by externel FET (GPIO 21 & GPIO 22)
   vreg_s2 = vreg_get(NULL, "s2");
   if (IS_ERR(vreg_s2)) {
      printk(KERN_ERR "%s: s2 vreg get failed (%ld)\n",
         __func__, PTR_ERR(vreg_wlan));
      return PTR_ERR(vreg_s2);
   }

   //2.2v RF - Gated by externel FET (GPIO 31 & GPIO 32)
   vreg_s4 = vreg_get(NULL, "s4");
   if (IS_ERR(vreg_s4)) {
      printk(KERN_ERR "%s: s4 vreg get failed (%ld)\n",
         __func__, PTR_ERR(vreg_s4));
      return PTR_ERR(vreg_s4);
   }

   //2.9v PA from LDO13
   vreg_wlan = vreg_get(NULL, "wlan");
   if (IS_ERR(vreg_wlan)) {
      printk(KERN_ERR "%s: wlan vreg get failed (%ld)\n",
         __func__, PTR_ERR(vreg_wlan));
      return PTR_ERR(vreg_wlan);
   }

   if (on) 
   {
      // Configure GPIO 21 & GPIO 22
      rc = pm8058_gpio_config(wlan_gpios_power_on[0].gpio_num, &wlan_gpios_power_on[0].gpio_cfg);
      if (rc) {
         printk(KERN_ERR "%s: pmic GPIO %d config failed (%d)\n",
            __func__, wlan_gpios_power_on[0].gpio_num, rc);
         return -EIO;
      }

      rc = pm8058_gpio_config(wlan_gpios_power_on[1].gpio_num, &wlan_gpios_power_on[1].gpio_cfg);
      if (rc) {
         printk(KERN_ERR "%s: pmic GPIO %d config failed (%d)\n",
            __func__, wlan_gpios_power_on[1].gpio_num, rc);
         return -EIO;
      }

      // Cofigure GPIO 27 to be high. Without this GPIO 31, 32 will be disabled.
      rc = pm8058_gpio_config(wlan_gpios_power_on[5].gpio_num, &wlan_gpios_power_on[5].gpio_cfg);
      if (rc) {
         printk(KERN_ERR "%s: pmic GPIO %d config failed (%d)\n",
            __func__, wlan_gpios_power_on[5].gpio_num, rc);
         return -EIO;
      }

      // Configure GPIO 31 & GPIO 32
      rc = pm8058_gpio_config(wlan_gpios_power_on[3].gpio_num, &wlan_gpios_power_on[3].gpio_cfg);
      if (rc) {
         printk(KERN_ERR "%s: pmic GPIO %d config failed (%d)\n",
            __func__, wlan_gpios_power_on[3].gpio_num, rc);
         return -EIO;
      }

      rc = pm8058_gpio_config(wlan_gpios_power_on[4].gpio_num, &wlan_gpios_power_on[4].gpio_cfg);
      if (rc) {
         printk(KERN_ERR "%s: pmic GPIO %d config failed (%d)\n",
            __func__, wlan_gpios_power_on[4].gpio_num, rc);
         return -EIO;
      }

      // Configure GPIO 23 for Deep Sleep
      rc = pm8058_gpio_config(wlan_gpios_power_on[2].gpio_num, &wlan_gpios_power_on[2].gpio_cfg);
      if (rc) {
         printk(KERN_ERR "%s: pmic GPIO %d config failed (%d)\n",
            __func__, wlan_gpios_power_on[2].gpio_num, rc);
         return -EIO;
      }

      // Power up 2.5v Analog
      rc = vreg_set_level(vreg_wlan2, PMIC_VREG_WLAN2_LEVEL);
      if (rc) {
         printk(KERN_ERR "%s: wlan2 vreg set level failed (%d)\n",
            __func__, rc);
         return -EIO;
      }

      rc = vreg_enable(vreg_wlan2);
      if (rc) {
         printk(KERN_ERR "%s: wlan2 vreg enable failed (%d)\n", __func__, rc);
         return -EIO;
      }

      printk(KERN_CRIT "%s: Wait for 2.5v supply to settle\n",__func__);
      msleep_interruptible(1000);
      printk(KERN_CRIT "%s: Done Waiting for 2.5v supply to settle\n", __func__);

      // Power up 1.3v RF
      rc = vreg_set_level(vreg_s2, PMIC_VREG_S2_LEVEL);
      if (rc) {
         printk(KERN_ERR "%s: s2 vreg set level failed (%d)\n", __func__, rc);
         return -EIO;
      }

      rc = vreg_enable(vreg_s2);
      if (rc) {
         printk(KERN_ERR "%s: s2 vreg enable failed (%d)\n", __func__, rc);
         return -EIO;
      }

      // Power up 2.2v RF
      rc = vreg_set_level(vreg_s4, PMIC_VREG_S4_LEVEL);
      if (rc) {
         printk(KERN_ERR "%s: s4 vreg set level failed (%d)\n",__func__, rc);
         return -EIO;
      }

      rc = vreg_enable(vreg_s4);
      if (rc) {
         printk(KERN_ERR "%s: s4 vreg enable failed (%d)\n", __func__, rc);
         return -EIO;
      }

      // Power up 2.9v PA
      rc = vreg_set_level(vreg_wlan, PMIC_VREG_WLAN_LEVEL);
      if (rc) {
         printk(KERN_ERR "%s: wlan vreg set level failed (%d)\n", __func__, rc);
         return -EIO;
      }

      rc = vreg_enable(vreg_wlan);
      if (rc) {
         printk(KERN_ERR "%s: wlan vreg enable failed (%d)\n",__func__, rc);
         return -EIO;
      }
	} 
   else 
   {
      rc = vreg_disable(vreg_wlan);
      if (rc) {
         printk(KERN_ERR "%s: wlan vreg disable failed (%d)\n", __func__, rc);
         return -EIO;
      }

      rc = vreg_disable(vreg_s4); 
      if (rc) {
         printk(KERN_ERR "%s: s4 vreg disable failed (%d)\n", __func__, rc);
         return -EIO;
      }

      rc = vreg_disable(vreg_s2);
      if (rc) {
         printk(KERN_ERR "%s: s2 vreg disable failed (%d)\n", __func__, rc);
         return -EIO;
      }

      rc = vreg_disable(vreg_wlan2);
      if (rc) {
         printk(KERN_ERR "%s: wlan2 vreg disable failed (%d)\n", __func__, rc);
         return -EIO;
      }
	}

   return 0;
}
#endif

#ifdef MSM_PLATFORM_7x27_FFA

#define MPP_4_CHIP_PWD_L 3 //MPP4 is hood to Deep Sleep Signal 

//Helper routine to power up Libra keypad on the 7x27 FFA
int vos_chip_power_7x27_keypad( int on )
{
   struct vreg *vreg_wlan, *vreg_bt = NULL;
   int rc = 0;

   vreg_wlan = vreg_get(NULL, "wlan");
   if (IS_ERR(vreg_wlan)) {
      printk(KERN_ERR "%s: wlan vreg get failed (%ld)\n",
         __func__, PTR_ERR(vreg_wlan));
      return PTR_ERR(vreg_wlan);
   }

	vreg_bt = vreg_get(NULL, "gp6");
	if (IS_ERR(vreg_bt)) {
		printk(KERN_ERR "%s: gp6 vreg get failed (%ld)\n",
		       __func__, PTR_ERR(vreg_bt));
		return PTR_ERR(vreg_bt);
	}

   if(on) {

      //Pull deep sleep signal low first to ensure a clean power on
      //sequence. Ideally this should already be pulled low.
      mpp_config_digital_out(MPP_4_CHIP_PWD_L, 
         MPP_CFG(MPP_DLOGIC_LVL_MSMP, MPP_DLOGIC_OUT_CTRL_LOW));

      //Disable VDD_WLAN in case this is turned on already. Ideally
      //this should already be turned off.
      rc = vreg_disable(vreg_wlan);
      if (rc) {
         printk(KERN_ERR "%s: vreg disable failed (%d)\n",__func__, rc);
         return -EIO;
      }      

      /* units of mV, steps of 50 mV */
      rc = vreg_set_level(vreg_bt, 2600);
      if (rc) {
         printk(KERN_ERR "%s: vreg set level failed (%d)\n",__func__, rc);
         return -EIO;
      }
      rc = vreg_enable(vreg_bt);
      if (rc) {
         printk(KERN_ERR "%s: vreg enable failed (%d)\n",__func__, rc);
         return -EIO;
      }

      //Pull deep sleep signal high to begin with.
      mpp_config_digital_out(MPP_4_CHIP_PWD_L, 
         MPP_CFG(MPP_DLOGIC_LVL_MSMP, MPP_DLOGIC_OUT_CTRL_HIGH));

      //Set VDD_WLAN_2V6 to 1.8v first.
      rc = vreg_set_level(vreg_wlan, 1800);
      if (rc) {
         printk(KERN_ERR "%s: vreg set level failed (%d)\n", __func__, rc);
         return -EIO;
      }

      rc = vreg_enable(vreg_wlan);
      if (rc) {
         printk(KERN_ERR "%s: wlan vreg enable failed (%d)\n",__func__, rc);
         return -EIO;
      }

      //Wait for voltage to settle
      msleep_interruptible(100);

      //Set VDD_WLAN_2V6 to 2.6v
      rc = vreg_set_level(vreg_wlan, 2600);
      if (rc) {
         printk(KERN_ERR "%s: vreg set level failed (%d)\n", __func__, rc);
         return -EIO;
      }
      
      printk(KERN_ERR "%s: Enabled power supply for WLAN\n", __func__);
 
      //Wait for for voltages to settle and WLAN card to be detected. This
      //wait time can be optimized. On rare instances, I have seen significant
      //delay in card detection. That could be because of latency in polling
      //frequency.
      msleep_interruptible(1000);
   }
   else 
   {
      rc = vreg_disable(vreg_wlan);
      if (rc) {
         printk(KERN_ERR "%s: vreg disable failed (%d)\n",__func__, rc);
         return -EIO;
      }

      rc = vreg_disable(vreg_bt);
      if (rc) {
         printk(KERN_ERR "%s: vreg disable failed (%d)\n",__func__, rc);
         return -EIO;
      }

      printk(KERN_ERR "%s: Disabled power supply for WLAN\n", __func__);
   }

   return 0;
}
#endif

/*===========================================================================

                    FUNCTION PROTOTYPES

===========================================================================*/

/**
  @brief vos_chipPowerUp() - This API will power up the Libra chip

  This operation may be asynchronous. If so, the supplied callback will
  be invoked when operation is complete with the result. The callback will 
  be called with the user supplied data. If the operation is known to be 
  sync, there is no need to supply a callback and user data.

  The Libra SDIO core will have been initialized if the operation completes
  successfully

  @param status [out] : whether this operation will complete sync or async
  @param callback [in] : user supplied callback invoked when operation completes
  @param user_data [in] : user supplied context callback is called with

  @return 
  VOS_STATUS_E_INVAL - status is NULL 
  VOS_STATUS_E_FAULT - the operation needs to complete async and a callback 
                       and user_data has not been specified (status will be
                       set to VOS_CALL_ASYNC) 
  VOS_STATUS_E_ALREADY - operation needs to complete async but another request
                         is already in progress (status will be set to VOS_CALL_ASYNC)  
  VOS_STATUS_E_FAILURE - operation failed (status will be set appropriately)       
  VOS_STATUS_SUCCESS - operation completed successfully if status is SYNC (will be set)
                       OR operation started successfully if status is ASYNC (will be set)

*/
VOS_STATUS vos_chipPowerUp
(
  vos_call_status_type* status,
  vos_power_cb_type     callback,
  v_PVOID_t             user_data
)
{

#ifdef MSM_PLATFORM_7x30
   if(vos_chip_power_qrf8600(CHIP_POWER_ON))
      return VOS_STATUS_E_FAILURE;
#endif   

#ifdef MSM_PLATFORM_7x27_FFA
   if(vos_chip_power_7x27_keypad(CHIP_POWER_ON))
      return VOS_STATUS_E_FAILURE;
#endif   

   return VOS_STATUS_SUCCESS;
}

/**
  @brief vos_chipPowerDown() - This API will power down the Libra chip

  This operation may be asynchronous. If so, the supplied callback will
  be invoked when operation is complete with the result. The callback will 
  be called with the user supplied data. If the operation is known to be 
  sync, there is no need to supply a callback and user data.

  @param status [out] : whether this operation will complete sync or async
  @param callback [in] : user supplied callback invoked when operation completes
  @param user_data [in] : user supplied context callback is called with

  @return 
  VOS_STATUS_E_INVAL - status is NULL 
  VOS_STATUS_E_FAULT - the operation needs to complete async and a callback 
                       and user_data has not been specified (status will be
                       set to VOS_CALL_ASYNC) 
  VOS_STATUS_E_ALREADY - operation needs to complete async but another request
                         is already in progress (status will be set to VOS_CALL_ASYNC)  
  VOS_STATUS_E_FAILURE - operation failed (status will be set appropriately)       
  VOS_STATUS_SUCCESS - operation completed successfully if status is SYNC (will be set)
                       OR operation started successfully if status is ASYNC (will be set)

*/
VOS_STATUS vos_chipPowerDown
(
  vos_call_status_type* status,
  vos_power_cb_type     callback,
  v_PVOID_t             user_data
)
{

#ifdef MSM_PLATFORM_7x30
   if(vos_chip_power_qrf8600(CHIP_POWER_OFF))
      return VOS_STATUS_E_FAILURE;
#endif

#ifdef MSM_PLATFORM_7x27_FFA
   if(vos_chip_power_7x27_keypad(CHIP_POWER_OFF))
      return VOS_STATUS_E_FAILURE;
#endif

   return VOS_STATUS_SUCCESS;
}

/**
  @brief vos_chipReset() - This API will reset the Libra chip

  This operation may be asynchronous. If so, the supplied callback will
  be invoked when operation is complete with the result. The callback will 
  be called with the user supplied data. If the operation is known to be 
  sync, there is no need to supply a callback and user data.

  A hard reset will involve a powerDown followed by a PowerUp; a soft reset
  can potentially be accomplished by writing to some device registers

  The Libra SDIO core will have been initialized if the operation completes
  successfully

  @param status [out] : whether this operation will complete sync or async
  @param soft [in] : VOS_TRUE if a soft reset is desired 
                     VOS_FALSE for a hard reset i.e. powerDown followed by powerUp
  @param callback [in] : user supplied callback invoked when operation completes
  @param user_data [in] : user supplied context callback is called with

  @return 
  VOS_STATUS_E_INVAL - status is NULL 
  VOS_STATUS_E_NOSUPPORT - soft reset asked for but not supported
  VOS_STATUS_E_FAULT - the operation needs to complete async and a callback 
                       and user_data has not been specified (status will be
                       set to VOS_CALL_ASYNC) 
  VOS_STATUS_E_ALREADY - operation needs to complete async but another request
                         is already in progress (status will be set to VOS_CALL_ASYNC)  
  VOS_STATUS_E_FAILURE - operation failed (status will be set appropriately)       
  VOS_STATUS_SUCCESS - operation completed successfully if status is SYNC (will be set)
                       OR operation started successfully if status is ASYNC (will be set)

*/
VOS_STATUS vos_chipReset
(
  vos_call_status_type* status,
  v_BOOL_t              soft,
  vos_power_cb_type     callback,
  v_PVOID_t             user_data
)
{
   return VOS_STATUS_SUCCESS;
}


/**
  @brief vos_chipVoteOnPASupply() - This API will power up the PA supply

  This operation may be asynchronous. If so, the supplied callback will
  be invoked when operation is complete with the result. The callback will 
  be called with the user supplied data. If the operation is known to be 
  sync, there is no need to supply a callback and user data.

  @param status [out] : whether this operation will complete sync or async
  @param callback [in] : user supplied callback invoked when operation completes
  @param user_data [in] : user supplied context callback is called with

  @return 
  VOS_STATUS_E_INVAL - status is NULL 
  VOS_STATUS_E_FAULT - the operation needs to complete async and a callback 
                       and user_data has not been specified (status will be
                       set to VOS_CALL_ASYNC) 
  VOS_STATUS_E_ALREADY - operation needs to complete async but another request
                         is already in progress (status will be set to VOS_CALL_ASYNC)  
  VOS_STATUS_E_FAILURE - operation failed (status will be set appropriately)       
  VOS_STATUS_SUCCESS - operation completed successfully if status is SYNC (will be set)
                       OR operation started successfully if status is ASYNC (will be set)

*/
VOS_STATUS vos_chipVoteOnPASupply
(
  vos_call_status_type* status,
  vos_power_cb_type     callback,
  v_PVOID_t             user_data
)
{
   return VOS_STATUS_SUCCESS;
}


/**
  @brief vos_chipVoteOffPASupply() - This API will vote to turn off the 
  PA supply. Even if we succeed in voting, there is a chance PA supply will not 
  be turned off. This will be treated the same as a failure.

  This operation may be asynchronous. If so, the supplied callback will
  be invoked when operation is complete with the result. The callback will 
  be called with the user supplied data. If the operation is known to be 
  sync, there is no need to supply a callback and user data.

  @param status [out] : whether this operation will complete sync or async
  @param callback [in] : user supplied callback invoked when operation completes
  @param user_data [in] : user supplied context callback is called with

  @return 
  VOS_STATUS_E_INVAL - status is NULL 
  VOS_STATUS_E_FAULT - the operation needs to complete async and a callback 
                       and user_data has not been specified (status will be
                       set to VOS_CALL_ASYNC) 
  VOS_STATUS_E_ALREADY - operation needs to complete async but another request
                         is already in progress (status will be set to VOS_CALL_ASYNC)  
  VOS_STATUS_E_FAILURE - operation failed (status will be set appropriately) could be 
                         because the voting algorithm decided not to power down PA  
  VOS_STATUS_SUCCESS - operation completed successfully if status is SYNC (will be set)
                       OR operation started successfully if status is ASYNC (will be set)

*/
VOS_STATUS vos_chipVoteOffPASupply
(
  vos_call_status_type* status,
  vos_power_cb_type     callback,
  v_PVOID_t             user_data
)
{
   return VOS_STATUS_SUCCESS;
}


/**
  @brief vos_chipAssertDeepSleep() - This API will assert the deep 
  sleep signal to Libra

  This operation may be asynchronous. If so, the supplied callback will
  be invoked when operation is complete with the result. The callback will 
  be called with the user supplied data. If the operation is known to be 
  sync, there is no need to supply a callback and user data.

  @param status [out] : whether this operation will complete sync or async
  @param callback [in] : user supplied callback invoked when operation completes
  @param user_data [in] : user supplied context callback is called with

  @return 
  VOS_STATUS_E_INVAL - status is NULL 
  VOS_STATUS_E_FAULT - the operation needs to complete async and a callback 
                       and user_data has not been specified (status will be
                       set to VOS_CALL_ASYNC) 
  VOS_STATUS_E_ALREADY - operation needs to complete async but another request
                         is already in progress (status will be set to VOS_CALL_ASYNC)  
  VOS_STATUS_E_FAILURE - operation failed (status will be set appropriately)       
  VOS_STATUS_SUCCESS - operation completed successfully if status is SYNC (will be set)
                       OR operation started successfully if status is ASYNC (will be set)

*/
VOS_STATUS vos_chipAssertDeepSleep
(
  vos_call_status_type* status,
  vos_power_cb_type     callback,
  v_PVOID_t             user_data
)
{

#ifdef MSM_PLATFORM_7x27_FFA
   mpp_config_digital_out(MPP_4_CHIP_PWD_L, MPP_CFG(MPP_DLOGIC_LVL_MSMP, MPP_DLOGIC_OUT_CTRL_LOW));
#endif

#ifdef MSM_PLATFORM_7x30
   // Configure GPIO 23 for Deep Sleep
   int rc = pm8058_gpio_config(wlan_gpios_power_off[0].gpio_num, &wlan_gpios_power_off[0].gpio_cfg);
   if (rc) {
      printk(KERN_ERR "%s: pmic GPIO %d config failed (%d)\n",
         __func__, wlan_gpios_power_off[0].gpio_num, rc);
      return VOS_STATUS_E_FAILURE;
   }
#endif

   return VOS_STATUS_SUCCESS;
}


/**
  @brief vos_chipDeAssertDeepSleep() - This API will de-assert the deep sleep
  signal to Libra

  This operation may be asynchronous. If so, the supplied callback will
  be invoked when operation is complete with the result. The callback will 
  be called with the user supplied data. If the operation is known to be 
  sync, there is no need to supply a callback and user data.

  @param status [out] : whether this operation will complete sync or async
  @param callback [in] : user supplied callback invoked when operation completes
  @param user_data [in] : user supplied context callback is called with

  @return 
  VOS_STATUS_E_INVAL - status is NULL 
  VOS_STATUS_E_FAULT - the operation needs to complete async and a callback 
                       and user_data has not been specified (status will be
                       set to VOS_CALL_ASYNC) 
  VOS_STATUS_E_ALREADY - operation needs to complete async but another request
                         is already in progress (status will be set to VOS_CALL_ASYNC)  
  VOS_STATUS_E_FAILURE - operation failed (status will be set appropriately)       
  VOS_STATUS_SUCCESS - operation completed successfully if status is SYNC (will be set)
                       OR operation started successfully if status is ASYNC (will be set)

*/
VOS_STATUS vos_chipDeAssertDeepSleep
(
  vos_call_status_type* status,
  vos_power_cb_type     callback,
  v_PVOID_t             user_data
)
{

#ifdef MSM_PLATFORM_7x27_FFA
   mpp_config_digital_out(MPP_4_CHIP_PWD_L, MPP_CFG(MPP_DLOGIC_LVL_MSMP, MPP_DLOGIC_OUT_CTRL_HIGH));
#endif
   return VOS_STATUS_SUCCESS;
}

/**
  @brief vos_chipExitDeepSleepVREGHandler() - This API will initialize the required VREG
  after exit from deep sleep.

  This operation may be asynchronous. If so, the supplied callback will
  be invoked when operation is complete with the result. The callback will 
  be called with the user supplied data. If the operation is known to be 
  sync, there is no need to supply a callback and user data.

  @param status [out] : whether this operation will complete sync or async
  @param callback [in] : user supplied callback invoked when operation completes
  @param user_data [in] : user supplied context callback is called with

  @return 
  VOS_STATUS_E_INVAL - status is NULL 
  VOS_STATUS_E_FAULT - the operation needs to complete async and a callback 
                       and user_data has not been specified (status will be
                       set to VOS_CALL_ASYNC) 
  VOS_STATUS_E_ALREADY - operation needs to complete async but another request
                         is already in progress (status will be set to VOS_CALL_ASYNC)  
  VOS_STATUS_E_FAILURE - operation failed (status will be set appropriately)       
  VOS_STATUS_SUCCESS - operation completed successfully if status is SYNC (will be set)
                       OR operation started successfully if status is ASYNC (will be set)

*/
VOS_STATUS vos_chipExitDeepSleepVREGHandler
(
   vos_call_status_type* status,
   vos_power_cb_type     callback,
   v_PVOID_t             user_data
)
{
#ifdef MSM_PLATFORM_7x27_FFA
   struct vreg *vreg_wlan = NULL;
   int rc;

   vreg_wlan = vreg_get(NULL, "wlan");
   if (IS_ERR(vreg_wlan)) {
      printk(KERN_ERR "%s: wlan vreg get failed (%ld)\n",
            __func__, PTR_ERR(vreg_wlan));
      return VOS_STATUS_E_FAILURE;
   }

   rc = vreg_set_level(vreg_wlan, 1800);
   if (rc) {
      printk(KERN_ERR "%s: wlan vreg set level failed (%d)\n",
            __func__, rc);
      return VOS_STATUS_E_FAILURE;
   }

   rc = vreg_enable(vreg_wlan);
   if (rc) {
      printk(KERN_ERR "%s: wlan vreg enable failed (%d)\n",
            __func__, rc);
      return VOS_STATUS_E_FAILURE;
   }

   msleep_interruptible(100);

   rc = vreg_set_level(vreg_wlan, 2600);
   if (rc) {
      printk(KERN_ERR "%s: wlan vreg set level failed (%d)\n",
            __func__, rc);
      return VOS_STATUS_E_FAILURE;
   }

   msleep_interruptible(1000);

   *status = VOS_CALL_SYNC;

#endif
   return VOS_STATUS_SUCCESS;
}

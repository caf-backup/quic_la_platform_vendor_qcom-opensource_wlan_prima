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

#ifndef LIBRA_LINUX_PC
#include <mach/mpp.h>
#include <mach/vreg.h>
#include <mach/rpc_pmapp.h>
#endif //LIBRA_LINUX_PC

#include <linux/err.h>
#include <linux/delay.h>

#ifndef LIBRA_LINUX_PC
#include <mach/pmic.h>
#endif //LIBRA_LINUX_PC


#ifdef MSM_PLATFORM_7x30
#include <linux/mfd/pmic8058.h>
#include <mach/rpc_pmapp.h>
#endif

#include <vos_sched.h>

/*===========================================================================

                        DEFINITIONS AND TYPES

===========================================================================*/

#define CHIP_POWER_ON         1
#define CHIP_POWER_OFF        0

#ifdef MSM_PLATFORM_7x30

static const char* id = "WLAN";

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

   //2.5v Analog from LDO19
   vreg_wlan2 = vreg_get(NULL, "wlan2");
   if (IS_ERR(vreg_wlan2)) {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, "%s: wlan2 vreg get failed (%ld)",
         __func__, PTR_ERR(vreg_wlan2));
      return PTR_ERR(vreg_wlan2);
   }

   //1.3v RF; gated by externel FET (GPIO 21 & GPIO 22)
   vreg_s2 = vreg_get(NULL, "s2");
   if (IS_ERR(vreg_s2)) {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, "%s: s2 vreg get failed (%ld)",
         __func__, PTR_ERR(vreg_s2));
      return PTR_ERR(vreg_s2);
   }

   //2.2v RF - Gated by externel FET (GPIO 31 & GPIO 32)
   vreg_s4 = vreg_get(NULL, "s4");
   if (IS_ERR(vreg_s4)) {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, "%s: s4 vreg get failed (%ld)",
         __func__, PTR_ERR(vreg_s4));
      return PTR_ERR(vreg_s4);
   }

   //2.9v PA from LDO13
   vreg_wlan = vreg_get(NULL, "wlan");
   if (IS_ERR(vreg_wlan)) {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, "%s: wlan vreg get failed (%ld)",
         __func__, PTR_ERR(vreg_wlan));
      return PTR_ERR(vreg_wlan);
   }

   if (on) 
   {
      // Configure GPIO 21 & GPIO 22
      rc = pm8058_gpio_config(wlan_gpios_power_on[0].gpio_num, &wlan_gpios_power_on[0].gpio_cfg);
      if (rc) {
         VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, "%s: pmic GPIO %d config failed (%d)",
            __func__, wlan_gpios_power_on[0].gpio_num, rc);
         return -EIO;
      }

      rc = pm8058_gpio_config(wlan_gpios_power_on[1].gpio_num, &wlan_gpios_power_on[1].gpio_cfg);
      if (rc) {
         VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, "%s: pmic GPIO %d config failed (%d)",
            __func__, wlan_gpios_power_on[1].gpio_num, rc);
         return -EIO;
      }

      // Cofigure GPIO 27 to be high. Without this GPIO 31, 32 will be disabled.
      rc = pm8058_gpio_config(wlan_gpios_power_on[5].gpio_num, &wlan_gpios_power_on[5].gpio_cfg);
      if (rc) {
         VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, "%s: pmic GPIO %d config failed (%d)",
            __func__, wlan_gpios_power_on[5].gpio_num, rc);
         return -EIO;
      }

      // Configure GPIO 31 & GPIO 32
      rc = pm8058_gpio_config(wlan_gpios_power_on[3].gpio_num, &wlan_gpios_power_on[3].gpio_cfg);
      if (rc) {
         VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, "%s: pmic GPIO %d config failed (%d)",
            __func__, wlan_gpios_power_on[3].gpio_num, rc);
         return -EIO;
      }

      rc = pm8058_gpio_config(wlan_gpios_power_on[4].gpio_num, &wlan_gpios_power_on[4].gpio_cfg);
      if (rc) {
         VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, "%s: pmic GPIO %d config failed (%d)",
            __func__, wlan_gpios_power_on[4].gpio_num, rc);
         return -EIO;
      }

      // Configure GPIO 23 for Deep Sleep
      rc = pm8058_gpio_config(wlan_gpios_power_on[2].gpio_num, &wlan_gpios_power_on[2].gpio_cfg);
      if (rc) {
         VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, "%s: pmic GPIO %d config failed (%d)",
            __func__, wlan_gpios_power_on[2].gpio_num, rc);
         return -EIO;
      }

      // Power up 2.5v Analog
      rc = vreg_set_level(vreg_wlan2, 2400);
      if (rc) {
         VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, "%s: wlan2 vreg set level failed (%d)",
            __func__, rc);
         return -EIO;
      }

      rc = vreg_enable(vreg_wlan2);
      if (rc) {
         VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, "%s: wlan2 vreg enable failed (%d)", __func__, rc);
         return -EIO;
      }

      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, "%s: Wait for 2.5v supply to settle",__func__);
      msleep(500);

      // Power up 1.3v RF
      rc = pmapp_vreg_level_vote(id, PMAPP_VREG_S2, 1300);
      if (rc) {
         VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, "%s: s2 vreg vote level failed (%d)", __func__, rc);
         return -EIO;
      }

      rc = vreg_enable(vreg_s2);
      if (rc) {
         VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, "%s: s2 vreg enable failed (%d)", __func__, rc);
         return -EIO;
      }

      // Power up 2.2v RF
      rc = pmapp_vreg_level_vote(id, PMAPP_VREG_S4, 2200);
      if (rc) {
         VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, "%s: s4 vreg vote level failed (%d)",__func__, rc);
         return -EIO;
      }

      rc = vreg_enable(vreg_s4);
      if (rc) {
         VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, "%s: s4 vreg enable failed (%d)", __func__, rc);
         return -EIO;
      }

      // Power up 2.9v PA
      rc = vreg_set_level(vreg_wlan, 2900);
      if (rc) {
         VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, "%s: wlan vreg set level failed (%d)", __func__, rc);
         return -EIO;
      }

      rc = vreg_enable(vreg_wlan);
      if (rc) {
         VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, "%s: wlan vreg enable failed (%d)",__func__, rc);
         return -EIO;
      }

      rc = pmapp_smps_mode_vote( "WLAN", PMAPP_VREG_S4, PMAPP_SMPS_MODE_VOTE_PWM );
      if( rc ) {
         VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, "%s: Attempting to vote for PMIC SMPS mode PWM failed with (%d)",__func__, rc);
         return -EIO;
      }
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, "%s: Enabled power supply for WLAN", __func__);
		
      msleep(500);
   }
   else
   {
      rc = vreg_disable(vreg_wlan);
      if (rc) {
         VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, "%s: wlan vreg disable failed (%d)", __func__, rc);
      }

      rc = pmapp_vreg_level_vote(id, PMAPP_VREG_S4, 0);
      if (rc) {
         VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, "%s: s4 vreg vote off failed (%d)", __func__, rc);
      }

      rc = pmapp_smps_mode_vote( "WLAN", PMAPP_VREG_S4, PMAPP_SMPS_MODE_VOTE_DONTCARE );
      if( rc ) {
         VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, "%s: Attempting to vote for PMIC SMPS mode PFM failed with (%d)",__func__, rc);
      }

      rc = vreg_disable(vreg_s4); 
      if (rc) {
         VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, "%s: s4 vreg disable failed (%d)", __func__, rc);
      }

      rc = pmapp_vreg_level_vote(id, PMAPP_VREG_S2, 0);
      if (rc) {
         VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, "%s: s2 vreg vote off failed (%d)", __func__, rc);
      }

      rc = vreg_disable(vreg_s2);
      if (rc) {
         VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_WARN, "%s: s2 vreg disable failed (%d)", __func__, rc);
      }

      rc = vreg_disable(vreg_wlan2);
      if (rc) {
         VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, "%s: wlan2 vreg disable failed (%d)", __func__, rc);
      }

      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, "%s: Disabled power supply for WLAN", __func__);
   }

   return 0;
}
#endif

#ifdef MSM_PLATFORM_7x27_FFA

#define MPP_4_CHIP_PWD_L 3 //MPP4 is hooked to Deep Sleep Signal 

//Helper routine to power up Libra keypad on the 7x27 FFA
int vos_chip_power_7x27_keypad( int on )
{
   struct vreg *vreg_wlan, *vreg_bt = NULL;
   int rc = 0;
	
   vreg_wlan = vreg_get(NULL, "wlan");
   if (IS_ERR(vreg_wlan)) {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, "%s: wlan vreg get failed (%ld)",
         __func__, PTR_ERR(vreg_wlan));
      return PTR_ERR(vreg_wlan);
   }

	vreg_bt = vreg_get(NULL, "gp6");
	if (IS_ERR(vreg_bt)) {
		VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, "%s: gp6 vreg get failed (%ld)",
		       __func__, PTR_ERR(vreg_bt));
		return PTR_ERR(vreg_bt);
	}

   if(on) {

      /* units of mV, steps of 50 mV */
      rc = vreg_set_level(vreg_bt, 2600);
      if (rc) {
         VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, "%s: vreg set level failed (%d)",__func__, rc);
         return -EIO;
      }
      rc = vreg_enable(vreg_bt);
      if (rc) {
         VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, "%s: vreg enable failed (%d)",__func__, rc);
         return -EIO;
      }

      //Set VDD_WLAN_2V6 to 1.8v first.
      rc = vreg_set_level(vreg_wlan, 1800);
      if (rc) {
         VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, "%s: vreg set level failed (%d)", __func__, rc);
         return -EIO;
      }

      rc = vreg_enable(vreg_wlan);
      if (rc) {
         VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, "%s: wlan vreg enable failed (%d)",__func__, rc);
         return -EIO;
      }

      msleep(100);

      //Pull deep sleep signal high to begin with.
      rc = mpp_config_digital_out(MPP_4_CHIP_PWD_L, 
         MPP_CFG(MPP_DLOGIC_LVL_MSMP, MPP_DLOGIC_OUT_CTRL_HIGH));
      if (rc) {
         VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, "%s: MPP_4 pull high failed (%d)",__func__, rc);
         return -EIO;
      }

      //Wait for voltage to settle
      msleep(400);

      //Set VDD_WLAN_2V6 to 2.6v
      rc = vreg_set_level(vreg_wlan, 2600);
      if (rc) {
         VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, "%s: vreg set level failed (%d)", __func__, rc);
         return -EIO;
      }
      
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, "%s: Enabled power supply for WLAN", __func__);
 
      msleep(500);
   }
   else 
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, "%s: Disabled power supply for WLAN", __func__);
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
   VOS_STATUS vstatus;
   vstatus = vos_watchdog_chip_reset();
   return vstatus;
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
   int rc = mpp_config_digital_out(MPP_4_CHIP_PWD_L, 
      MPP_CFG(MPP_DLOGIC_LVL_MSMP, MPP_DLOGIC_OUT_CTRL_LOW));
   if (rc) {
	   VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, "%s: Failed to pull high MPP_4_CHIP_PWD_L (%d)",
		   __func__, rc);
	   return VOS_STATUS_E_FAILURE;
   }

#endif

#ifdef MSM_PLATFORM_7x30
   // Configure GPIO 23 for Deep Sleep
   int rc = pm8058_gpio_config(wlan_gpios_power_off[0].gpio_num, &wlan_gpios_power_off[0].gpio_cfg);
   if (rc) {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, "%s: pmic GPIO %d config failed (%d)",
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
   int rc = mpp_config_digital_out(MPP_4_CHIP_PWD_L, 
      MPP_CFG(MPP_DLOGIC_LVL_MSMP, MPP_DLOGIC_OUT_CTRL_HIGH));
   if (rc) {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, "%s: Failed to pull high MPP_4_CHIP_PWD_L (%d)",
         __func__, rc);
	   return VOS_STATUS_E_FAILURE;
   }


#endif

#ifdef MSM_PLATFORM_7x30
	// Configure GPIO 23 for Deep Sleep
	int rc = pm8058_gpio_config(wlan_gpios_power_on[2].gpio_num, &wlan_gpios_power_on[2].gpio_cfg);
	if (rc) {
		VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, "%s: pmic GPIO %d config failed (%d)",
			__func__, wlan_gpios_power_on[2].gpio_num, rc);
		return VOS_STATUS_E_FAILURE;
	}
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
   struct vreg *vreg_wlan;
   int rc;

   vreg_wlan = vreg_get(NULL, "wlan");
   if (IS_ERR(vreg_wlan)) {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, "%s: wlan vreg get failed (%ld)",
            __func__, PTR_ERR(vreg_wlan));
      return VOS_STATUS_E_FAILURE;
   }

   rc = vreg_set_level(vreg_wlan, 1800);
   if (rc) {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, "%s: wlan vreg set level failed (%d)",
            __func__, rc);
      return VOS_STATUS_E_FAILURE;
   }

   rc = vreg_enable(vreg_wlan);
   if (rc) {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, "%s: wlan vreg enable failed (%d)",
            __func__, rc);
      return VOS_STATUS_E_FAILURE;
   }

   msleep(500);

   rc = vreg_set_level(vreg_wlan, 2600);
   if (rc) {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, "%s: wlan vreg set level failed (%d)",
            __func__, rc);
      return VOS_STATUS_E_FAILURE;
   }

   msleep(500);

   *status = VOS_CALL_SYNC;

#endif

#ifdef MSM_PLATFORM_7x30
   VOS_STATUS vosStatus;
   vos_call_status_type callType;

   vosStatus = vos_chipVoteOnBBAnalogSupply(&callType, NULL, NULL);
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
   msleep(500);

#endif
   return VOS_STATUS_SUCCESS;
}

/**
  @brief vos_chipVoteOnRFSupply() - This API will power up the RF supply

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
VOS_STATUS vos_chipVoteOnRFSupply
(
  vos_call_status_type* status,
  vos_power_cb_type     callback,
  v_PVOID_t             user_data
)
{
#ifdef MSM_PLATFORM_7x30
   int rc;
   struct vreg *vreg_s2 = NULL;
   struct vreg *vreg_s4 = NULL;

   //1.3v RF;
   vreg_s2 = vreg_get(NULL, "s2");
   if (IS_ERR(vreg_s2)) {
      printk(KERN_ERR "%s: s2 vreg get failed (%ld)\n",
         __func__, PTR_ERR(vreg_s2));
      return VOS_STATUS_E_FAILURE;
   }

   //2.2v RF
   vreg_s4 = vreg_get(NULL, "s4");
   if (IS_ERR(vreg_s4)) {
      printk(KERN_ERR "%s: s4 vreg get failed (%ld)\n",
         __func__, PTR_ERR(vreg_s4));
      return VOS_STATUS_E_FAILURE;
   }

   rc = pmapp_vreg_level_vote(id, PMAPP_VREG_S2, 1300);
   if (rc) {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, "%s: s2 vreg vote "
          "level failed (%d)",__func__, rc);
      return VOS_STATUS_E_FAILURE;
   }

   rc = vreg_enable(vreg_s2);
   if (rc) {
      printk(KERN_ERR "%s: s2 vreg enable failed (%d)\n", __func__, rc);
      return VOS_STATUS_E_FAILURE;
   }

   rc = pmapp_vreg_level_vote(id, PMAPP_VREG_S4, 2200);
   if (rc) {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, "%s: s4 vreg vote "
          "level failed (%d)",__func__, rc);
      return VOS_STATUS_E_FAILURE;
   }

   rc = vreg_enable(vreg_s4);
   if (rc) {
      printk(KERN_ERR "%s: s4 vreg enable failed (%d)\n", __func__, rc);
      return VOS_STATUS_E_FAILURE;
   }

   rc = pmapp_smps_mode_vote( "WLAN", PMAPP_VREG_S4, PMAPP_SMPS_MODE_VOTE_PWM );
   if( rc ) {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, "%s: Attempting to vote for PMIC SMPS mode PWM failed with (%d)",__func__, rc);
      return VOS_STATUS_E_FAILURE;
   }

   return VOS_STATUS_SUCCESS;

#endif //MSM_PLATFORM_7x30
   
   return VOS_STATUS_SUCCESS;
}

/**
  @brief vos_chipVoteOffRFSupply() - This API will vote to turn off the 
  RF supply. Even if we succeed in voting, there is a chance RF supply will not 
  be turned off as RF rails could be shared with other modules (outside WLAN)

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
VOS_STATUS vos_chipVoteOffRFSupply
(
  vos_call_status_type* status,
  vos_power_cb_type     callback,
  v_PVOID_t             user_data
)
{
#ifdef MSM_PLATFORM_7x30

   int rc;
   struct vreg *vreg_s2;
   struct vreg *vreg_s4;

   //1.3v RF
   vreg_s2 = vreg_get(NULL, "s2");
   if (IS_ERR(vreg_s2)) {
      printk(KERN_ERR "%s: s2 vreg get failed (%ld)\n",
         __func__, PTR_ERR(vreg_s2));
      return VOS_STATUS_E_FAILURE;
   }

   //2.2v RF
   vreg_s4 = vreg_get(NULL, "s4");
   if (IS_ERR(vreg_s4)) {
      printk(KERN_ERR "%s: s4 vreg get failed (%ld)\n",
         __func__, PTR_ERR(vreg_s4));
      return VOS_STATUS_E_FAILURE;
   }

   rc = pmapp_vreg_level_vote(id, PMAPP_VREG_S2, 0);
   if (rc) {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_WARN, "%s: s2 vreg vote "
          "level failed (%d)",__func__, rc);
   }

   rc = vreg_disable(vreg_s2);
   if (rc) {
      printk(KERN_ERR "%s: s2 vreg disable failed (%d)\n", __func__, rc);
   }

   rc = pmapp_vreg_level_vote(id, PMAPP_VREG_S4, 0);
   if (rc) {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_WARN, "%s: s4 vreg vote "
          "level failed (%d)",__func__, rc);
   }

   rc = pmapp_smps_mode_vote( "WLAN", PMAPP_VREG_S4, PMAPP_SMPS_MODE_VOTE_DONTCARE );
   if( rc ) {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, "%s: Attempting to vote for PMIC SMPS mode PFM failed with (%d)",__func__, rc);
   }

   rc = vreg_disable(vreg_s4); 
   if (rc) {
      printk(KERN_ERR "%s: s4 vreg disable failed (%d)\n", __func__, rc);
   }

   return VOS_STATUS_SUCCESS;

#endif //MSM_PLATFORM_7x30

   return VOS_STATUS_SUCCESS;
}

/**
  @brief vos_chipVoteOnBBAnalogSupply() - This API will power up the I/P voltage
  used by Base band Analog.

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
VOS_STATUS vos_chipVoteOnBBAnalogSupply
(
  vos_call_status_type* status,
  vos_power_cb_type     callback,
  v_PVOID_t             user_data
)
{
#ifdef MSM_PLATFORM_7x30
   struct vreg *vreg_wlan2 = NULL;
   int rc;

   //2.5v Analog from LDO19
   vreg_wlan2 = vreg_get(NULL, "wlan2");
   if (IS_ERR(vreg_wlan2)) {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, "%s: wlan2 vreg get "
          "failed (%ld)", __func__, PTR_ERR(vreg_wlan2));
      return VOS_STATUS_E_FAILURE;
   }

   rc = vreg_set_level(vreg_wlan2, 2400);
   if (rc) {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, "%s: wlan2 vreg set "
          "level failed (%d)",__func__, rc);
      return VOS_STATUS_E_FAILURE;
   }

   rc = vreg_enable(vreg_wlan2);
   if (rc) {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, "%s: wlan2 vreg enable "
          "failed (%d)", __func__, rc);
      return VOS_STATUS_E_FAILURE;
   }
#endif
   return VOS_STATUS_SUCCESS;
}

/**
  @brief vos_chipVoteOffBBAnalogSupply() - This API will vote off the BB Analog supply.

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
VOS_STATUS vos_chipVoteOffBBAnalogSupply
(
  vos_call_status_type* status,
  vos_power_cb_type     callback,
  v_PVOID_t             user_data
)
{
#ifdef MSM_PLATFORM_7x30
   struct vreg *vreg_wlan2 = NULL;
   int rc;

   //2.5v Analog from LDO19
   vreg_wlan2 = vreg_get(NULL, "wlan2");
   if (IS_ERR(vreg_wlan2)) {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, "%s: wlan2 vreg get "
          "failed (%ld)", __func__, PTR_ERR(vreg_wlan2));
      return VOS_STATUS_E_FAILURE;
   }

   rc = vreg_disable(vreg_wlan2);
   if (rc) {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL, "%s: wlan2 vreg disable "
          "failed (%d)", __func__, rc);
      return VOS_STATUS_E_FAILURE;
   }
#endif
   return VOS_STATUS_SUCCESS;
}

/**
  @brief vos_chipVoteOnXOBuffer() - This API will vote to turn on the XO buffer from
  PMIC. This API will be used when Libra uses the TCXO from PMIC on the MSM

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
VOS_STATUS vos_chipVoteOnXOBuffer
(
  vos_call_status_type* status,
  vos_power_cb_type     callback,
  v_PVOID_t             user_data
)
{
#ifdef MSM_PLATFORM_7x30
   int rc;

   //Turn on the 19.2 MHz A0 XO buffer from PMIC8058
   rc = pmapp_clock_vote(id, PMAPP_CLOCK_ID_A0, PMAPP_CLOCK_VOTE_ON);
   if (rc) {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
         "%s: A0 clk vote on failed (%d)",__func__, rc);
      return VOS_STATUS_E_FAILURE;
   }

   //Put the clock in a pin control mode
   rc = pmapp_clock_vote(id, PMAPP_CLOCK_ID_A0, PMAPP_CLOCK_VOTE_PIN_CTRL);
   if (rc) {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
         "%s: A0 pin ctrl vote failed (%d)",__func__, rc);
      pmapp_clock_vote(id, PMAPP_CLOCK_ID_A0, PMAPP_CLOCK_VOTE_OFF);
      return VOS_STATUS_E_FAILURE;
   }

   //Allow some delay for clock to be turned on. 10 ms should suffice.
   msleep(10);

#endif

   return VOS_STATUS_SUCCESS;
}

/**
  @brief vos_chipVoteOffXOBuffer() - This API will vote off PMIC XO buffer.

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
VOS_STATUS vos_chipVoteOffXOBuffer
(
  vos_call_status_type* status,
  vos_power_cb_type     callback,
  v_PVOID_t             user_data
)
{
#ifdef MSM_PLATFORM_7x30
   int rc;

   rc = pmapp_clock_vote(id, PMAPP_CLOCK_ID_A0, PMAPP_CLOCK_VOTE_OFF);
   if (rc) {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
         "%s: A0 clk vote off failed (%d)",__func__, rc);
      return VOS_STATUS_E_FAILURE;
   }
#endif

   return VOS_STATUS_SUCCESS;
}

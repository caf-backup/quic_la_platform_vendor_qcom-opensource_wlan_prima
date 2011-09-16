#ifndef __WLAN_HDD_DEV_PWR_H
#define __WLAN_HDD_DEV_PWR_H

#include <wlan_hdd_includes.h>
#include <wlan_hdd_power.h>
#include <vos_sched.h>
#include <vos_api.h>

/*----------------------------------------------------------------------------

   @brief Registration function.
        Register suspend, resume callback functions with platform driver. 

   @param hdd_context_t pHddCtx
        Global hdd context

   @return General status code
        VOS_STATUS_SUCCESS       Registration Success
        VOS_STATUS_E_FAILURE     Registration Fail

----------------------------------------------------------------------------*/
VOS_STATUS hddRegisterPmOps(hdd_context_t *pHddCtx);

/*----------------------------------------------------------------------------

   @brief De-registration function.
        Deregister the suspend, resume callback functions with platform driver

   @param hdd_context_t pHddCtx
        Global hdd context

   @return General status code
        VOS_STATUS_SUCCESS       De-Registration Success
        VOS_STATUS_E_FAILURE     De-Registration Fail

----------------------------------------------------------------------------*/
VOS_STATUS hddDeregisterPmOps(hdd_context_t *pHddCtx);

#endif

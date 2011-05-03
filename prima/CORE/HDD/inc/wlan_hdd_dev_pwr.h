#ifndef __WLAN_HDD_DEV_PWR_H
#define __WLAN_HDD_DEV_PWR_H

#include <wlan_hdd_includes.h>
#include <wlan_hdd_power.h>
#include <vos_sched.h>
#include <vos_api.h>

/*----------------------------------------------------------------------------

   @brief Start SAL module.
        Probing SDIO interface, get and store card information

   @param v_PVOID_t pAdapter
        Global adapter handle

   @return General status code
        VOS_STATUS_SUCCESS       Start Success
        VOS_STATUS_E_FAILURE     Start Fail, BAL Not open yet
        VOS_STATUS_E_INVAL       Invalid argument

----------------------------------------------------------------------------*/
VOS_STATUS hddRegisterPmOps(hdd_adapter_t *pAdapter);

/*----------------------------------------------------------------------------

   @brief Deregistration function.
        Deregister the suspend, resume callabcks with platform driver

   @param v_PVOID_t pAdapter
        Global adapter handle

   @return General status code
        VOS_STATUS_SUCCESS       Registration Success
        VOS_STATUS_E_FAILURE     Registration Failure

----------------------------------------------------------------------------*/
VOS_STATUS hddDeregisterPmOps(hdd_adapter_t *pAdapter);

#endif

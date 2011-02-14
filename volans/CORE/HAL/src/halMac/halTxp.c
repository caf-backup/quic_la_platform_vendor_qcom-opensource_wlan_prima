#include "halTypes.h"
#include "palTypes.h"
#include "aniGlobal.h"
#ifdef WLAN_HAL_VOLANS
#include "volansDefs.h"
#else
#include "libraDefs.h"
#endif
#include "halTxp.h"
#include "sirMacProtDef.h" // tSirMacAddr
#include "halDebug.h"
#include "halAdu.h"

eHalStatus halTxp_Start(tHalHandle hHal, void *arg)
{
    tpAniSirGlobal  pMac = (tpAniSirGlobal)hHal;
	tANI_U32	value;

	halReadRegister(pMac, QWLAN_TXP_TXP_CMDF_CONTROL_REG, 
					&value);

	value |= QWLAN_TXP_TXP_CMDF_CONTROL_TPE_CAHBZ_MASK;
	
	halWriteRegister(pMac, QWLAN_TXP_TXP_CMDF_CONTROL_REG, 
					value);

#ifdef WLAN_ENABLE_BTC_ABORT_IN_TXP
	palReadRegister(pMac->hHdd, QWLAN_TXP_TXP_BTC_ACTIVE_ABORT_ENABLE_REG, &value);
	value |= QWLAN_TXP_TXP_BTC_ACTIVE_ABORT_ENABLE_BTC_ACTIVE_MASK;
	palWriteRegister(pMac, QWLAN_TXP_TXP_BTC_ACTIVE_ABORT_ENABLE_REG, value);
#endif

    return eHAL_STATUS_SUCCESS;
}


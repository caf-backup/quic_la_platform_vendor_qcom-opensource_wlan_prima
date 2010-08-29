#include "halTypes.h"
#include "palTypes.h"
#include "aniGlobal.h"
#include "libraDefs.h"
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

    // DEBUG - enable BTC abort in TXP
	palReadRegister(pMac->hHdd, QWLAN_TXP_TXP_BTC_ACTIVE_ABORT_ENABLE_REG, &value);
	value |= QWLAN_TXP_TXP_BTC_ACTIVE_ABORT_ENABLE_BTC_ACTIVE_MASK;
	palWriteRegister(pMac, QWLAN_TXP_TXP_BTC_ACTIVE_ABORT_ENABLE_REG, value);

    return eHAL_STATUS_SUCCESS;
}


/*
 * Airgo Networks, Inc proprietary. All rights reserved.
 *
 * Author:      Sandesh Goel
 * Date:        02/25/02
 * History:-
 * Date            Modified by    Modification Information
 * --------------------------------------------------------------------
 *
 */

#ifndef __SCH_API_H__
#define __SCH_API_H__

#include "halDataStruct.h"
#include "sirCommon.h"
#include "sirMacProtDef.h"

#include "aniGlobal.h"

/// Send start scan response message
extern void schSendStartScanRsp(tpAniSirGlobal pMac);

/// Set init time params
extern void schSetInitParams(tpAniSirGlobal pMac);

/// Set qos default params
extern void schUpdateQosInfo(tpAniSirGlobal pMac);

// update only the broadcast qos params
extern void schQosUpdateBroadcast(tpAniSirGlobal pMac);

// fill in the default local edca parameter into gSchEdcaParams[]
extern void schSetDefaultEdcaParams(tpAniSirGlobal pMac);

// update only local qos params
extern void schQosUpdateLocal(tpAniSirGlobal pMac);

// update the edca profile parameters
extern void schEdcaProfileUpdate(tpAniSirGlobal pMac);

/// Check for RR timer expiry
extern void schCheckRRTimerExpiry(tpAniSirGlobal pMac);

/// Set the fixed fields in a beacon frame
extern tSirRetStatus schSetFixedBeaconFields(tpAniSirGlobal pMac);

/// Initializations
extern void schInitialize(tpAniSirGlobal pMac);

/// Initialize globals
extern void schInitGlobals(tpAniSirGlobal pMac);

/// Initialize CF Poll template
extern void schInitializeCfPollTemplate(tpAniSirGlobal pMac);

/// Initialize CF End template
extern void schInitializeCfEndTemplate(tpAniSirGlobal pMac);

/// Process the transmit activity queue
extern void schProcessTxActivityQueue(tpAniSirGlobal pMac);

/// Add to the DPH activity queue
extern void schAddDphActivityQueue(tpAniSirGlobal pMac, tANI_U16, tANI_U8);

/// Add to the TX IN (DPH) activity queue
extern void schAddTxInActivityQueue(tpAniSirGlobal pMac, void *ptr);

/// Process the scheduler message queue
extern void schProcessMessageQueue(tpAniSirGlobal pMac);

/// Process the scheduler messages
extern void schProcessMessage(tpAniSirGlobal pMac,tpSirMsgQ pSchMsg);

/// Process the DPH activity queue
extern void schProcessDphActivityQueue(tpAniSirGlobal pMac);

/// The beacon Indication handler function
extern void schProcessPreBeaconInd(tpAniSirGlobal pMac);

/// Post a message to the scheduler message queue
extern tSirRetStatus schPostMessage(tpAniSirGlobal pMac, tpSirMsgQ pMsg);

#if (WNI_POLARIS_FW_PRODUCT == AP)
/// The scheduling interrupt handler
extern void schSchedulingInterruptHandler(tpAniSirGlobal pMac);

/// The scheduling end interrupt function
extern void schSchedulingEndInterruptHandler(tpAniSirGlobal pMac);

/// Function used by other Sirius modules to read CFPcount
extern tANI_U8 schGetCFPCount(tpAniSirGlobal pMac);

/// Function used by other Sirius modules to read CFPDuration remaining
extern tANI_U16 schGetCFPDurRemaining(tpAniSirGlobal pMac);

#endif

extern void schBeaconProcess(tpAniSirGlobal pMac, tpHalBufDesc pBD);
extern tSirRetStatus schBeaconEdcaProcess(tpAniSirGlobal pMac, tSirMacEdcaParamSetIE *edca);


#define SCH_RR_TIMEOUT                   (SCH_RR_TIMEOUT_MS / SYS_TICK_DUR_MS)

void schSetBeaconInterval(tpAniSirGlobal pMac);

tSirRetStatus schSendBeaconReq( tpAniSirGlobal, tANI_U8 *, tANI_U16 );


#endif

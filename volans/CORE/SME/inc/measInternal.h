#ifdef FEATURE_INNAV_SUPPORT

#ifndef __MEAS_INTERNAL_H__
#define __MEAS_INTERNAL_H__

#include "palTimer.h"
#include "csrSupport.h"
#include "vos_nvitem.h"
#include "wlan_qct_tl.h"

#include "measApi.h"

typedef struct tagMeasInNavMeasurementStruct
{
	tANI_U32                         nextMeasurementId; //a global measurement id
	tANI_U16                         numMeasurementSetsRemaining; //status of the current measurement config
	tANI_BOOLEAN                     measurementActive; //indicates that currently a measurement request has been posted and 
									     				//waiting for the results
	eMeasMeasurementStatus           prevMeasurementSetStatus; //status of the previous measurement set
	tPalTimerHandle                  hTimerMeasurement; //timer for triggering the measurement requests in the given set
	measMeasurementCompleteCallback  callback; //callback function pointer for returning the measurements
	void*                            pContext; //context of the original caller
	tANI_U32                         measurementRequestID; //original measurement request ID
	tInNavMeasurementResponse*       pMeasurementResult; //measurement results
	tInNavMeasurementConfig          measurementConfig; //current measurement request
  tANI_U8                          sessionId; //Session on which measurement is active
} tMeasInNavMeasurementStruct;

typedef struct tagMeasCmd
{
	tANI_U32						measurementID;
	measMeasurementCompleteCallback	callback;
	void*							pContext;
	tInNavMeasurementRequest		measurementReq;
} tMeasCmd;

#endif //__MEAS_INTERNAL_H__

#endif //FEATURE_INNAV_SUPPORT

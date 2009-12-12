#ifndef _HAL_BDAPI_H_
#define _HAL_BDAPI_H_

#include "halTypes.h" //tHalHandle
#include "palTypes.h"
#include "sirMacProtDef.h" // tpSirMacMgmtHdr


eHalStatus
halInitDefaultMgmtBDTemplate( tHalHandle hHal, void *arg );

eHalStatus
halInitDefaultTxBD( tANI_U8 *txBD );


// Function to build BD for Data frames.
eHalStatus
halFillTxBD(
    tHalHandle  halHandle,
    tANI_U8    *srcMac,
    tANI_U8    *dstMac,
    eFrameTxDir txDir,
    tANI_U16    proto,
    tANI_U16    length,
    tANI_U8     *pTid,
    tANI_U8    *txBd,
    eFrameType frmType,
    tANI_U32   *pAc,
    tANI_U8    *pStaId);

// Function to build BD for MGMT frame
eHalStatus
halFillTxBDMgmt(
    tHalHandle      halHandle,
    tANI_U8        *txBd,
    tpSirMacMgmtHdr mgmtHdr,
    tANI_U16        payload,
    tANI_U32       *pAc);


#endif /* _HAL_BDAPI_H_ */

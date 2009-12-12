

#ifndef WLAN_QCT_HAL_H
#define WLAN_QCT_HAL_H
#include "vos_status.h"
#include "halTypes.h"
#ifndef PALTYPES_H__

/// unsigned 8-bit types
#define tANI_U8        v_U8_t
 
/// unsigned 16-bit types
#define tANI_U16    v_U16_t    

/// unsigned 32-bit types
#define tANI_U32    v_U32_t
 
/// signed 8-bit types
#define    tANI_S8        v_S7_t
 
/// signed 16-bit types
#define tANI_S16    v_S15_t
 
/// signed 32-bit types
#define tANI_S32    v_S31_t

#define eHalStatus    VOS_STATUS

#endif

#include "halBdDefs.h"

#define QWLAN_HAL_DXE0_MASTERID  5

typedef struct sHalBdGeneric {
    /* 0x00 */
    // ENDIAN BEGIN
    tANI_U32 dpuRF : 8;
    tANI_U32 dpuSignature:3;     /* Signature on RA's DPU descriptor */
    tANI_U32 staSignature:3;
    tANI_U32 reserved : 14;
    tANI_U32 dpuNE : 1;
    tANI_U32 dpuNC : 1;
    tANI_U32 bdt : 2;                        /* BD type */
    // ENDIAN END

    /* 0x04 */
    // ENDIAN BEGIN
    tANI_U32 reserved1:32;                      
    // ENDIAN END


    /* 0x08 */
    // ENDIAN BEGIN
    tANI_U32 headPduIdx : 16;                /* Head PDU index */
    tANI_U32 tailPduIdx : 16;                /* Tail PDU index */
    // ENDIAN END

    /* 0x0c */
    // ENDIAN BEGIN
    tANI_U32 mpduHeaderLength : 8;           /* MPDU header length */
    tANI_U32 mpduHeaderOffset : 8;           /* MPDU header start offset */
    tANI_U32 mpduDataOffset : 9;             /* MPDU data start offset */
    tANI_U32 pduCount : 7;                   /* PDU count */
    // ENDIAN END

    /* 0x10 */
    // ENDIAN BEGIN
    tANI_U32 mpduLength : 16;                /* MPDU length */
    tANI_U32 reserved3:4;            /* DPU compression feedback */
    tANI_U32 tid : 4;                        /* Traffic identifier, tid */
    tANI_U32 rateIndex : 8;
    // ENDIAN END

    /* 0x14 */
    // ENDIAN BEGIN
    tANI_U32 dpuDescIdx : 8;
    tANI_U32 addr1Index : 8;  //A1 index after RxP binary search
    tANI_U32 addr2Index : 8;  //A2 index after RxP binary search
    tANI_U32 addr3Index : 8;  //A3 index after RxP binary search
    // ENDIAN END
//}__ani_attr_packed __ani_attr_aligned_4 tHalBdGeneric, *tpHalBdGeneric;
} tHalBdGeneric, *tpHalBdGeneric;


/*
 * PDU without BD
 */

typedef struct sHalPdu {
    tANI_U8 payload[124];
    tANI_U32 nextPduIdx;                     /* LSB 16 bits */
//} __ani_attr_packed __ani_attr_aligned_4 tHalPdu, *tpHalPdu;
} tHalPdu, *tpHalPdu;



#define HAL_TXBD_BDRATE_DEFAULT 0
#define HAL_TXBD_BDRATE_FIRST   1
#define HAL_TXBD_BDRATE_SECOND  2
#define HAL_TXBD_BDRATE_THIRD   3

#define HAL_FRAME_TYPE_MASK     0x30
#define HAL_FRAME_TYPE_OFFSET   0x4
#define HAL_FRAME_SUBTYPE_MASK  0x0F

#define HAL_TXBD_BD_SSN_FILL_HOST             0
#define HAL_TXBD_BD_SSN_FILL_DPU_NON_QOS    1
#define HAL_TXBD_BD_SSN_FILL_DPU_QOS        2

#define HAL_ACKPOLICY_ACK_REQUIRED        0
#define HAL_ACKPOLICY_ACK_NOTREQUIRED    1

#define HAL_BDRATE_BCDATA_FRAME            1
#define HAL_BDRATE_BCMGMT_FRAME            2
#define HAL_BDRATE_CTRL_FRAME            3
    
/* Default values for FillTx BD */
#define HAL_DEFAULT_UNICAST_ENABLED     1
#define HAL_RMF_DISABLED                 0
#define HAL_RMF_ENABLED                 1
#define HAL_NO_ENCRYPTION_DISABLED        0
#define HAL_NO_ENCRYPTION_ENABLED         1
    
#define WLANHAL_RX_BD_ADDR3_SELF_IDX      0

#define WLANHAL_TX_BD_HEADER_SIZE        sizeof(tHalTxBd)

#define WLANHAL_RX_BD_HEADER_SIZE        sizeof(tHalRxBd)

#define WLANHAL_RX_BD_HEADER_OFFSET       0

#define WLANHAL_RX_BD_GET_MPDU_H_OFFSET( _pvBDHeader )   (((tpHalRxBd)_pvBDHeader)->mpduHeaderOffset)

#define WLANHAL_RX_BD_GET_MPDU_D_OFFSET( _pvBDHeader )   (((tpHalRxBd)_pvBDHeader)->mpduDataOffset)

#define WLANHAL_RX_BD_GET_MPDU_LEN( _pvBDHeader )        (((tpHalRxBd)_pvBDHeader)->mpduLength)

#define WLANHAL_RX_BD_GET_MPDU_H_LEN( _pvBDHeader )      (((tpHalRxBd)_pvBDHeader)->mpduHeaderLength)


#define WLANHAL_RX_BD_GET_FT( _pvBDHeader )        (((tpHalRxBd)_pvBDHeader)->ft)

#define WLANHAL_RX_BD_GET_LLC( _pvBDHeader )        (((tpHalRxBd)_pvBDHeader)->llc)

#define WLANHAL_RX_BD_GET_TID( _pvBDHeader )        (((tpHalRxBd)_pvBDHeader)->tid)

#define WLANHAL_RX_BD_GET_ASF( _pvBDHeader )        (((tpHalRxBd)_pvBDHeader)->asf)

#define WLANHAL_RX_BD_GET_AEF( _pvBDHeader )           (((tpHalRxBd)_pvBDHeader)->aef)

#define WLANHAL_RX_BD_GET_LSF( _pvBDHeader )           (((tpHalRxBd)_pvBDHeader)->lsf)

#define WLANHAL_RX_BD_GET_ESF( _pvBDHeader )           (((tpHalRxBd)_pvBDHeader)->esf)

#define WLANHAL_RX_BD_GET_STA_ID( _pvBDHeader )     (((tpHalRxBd)_pvBDHeader)->addr2Index)
#define WLANHAL_RX_BD_GET_ADDR3_IDX( _pvBDHeader )     (((tpHalRxBd)_pvBDHeader)->addr3Index)
#define WLANHAL_RX_BD_GET_ADDR1_IDX( _pvBDHeader )     (((tpHalRxBd)_pvBDHeader)->addr1Index)

#define WLANHAL_TX_BD_GET_TID( _pvBDHeader )           (((tpHalTxBd)_pvBDHeader)->tid)

#define WLANHAL_TX_BD_GET_STA_ID( _pvBDHeader )     (((tpHalTxBd)_pvBDHeader)->staIndex)


#define WLANHAL_TX_BD_SET_MPDU_DATA_OFFSET( _bd, _off )        (((tpHalTxBd)_bd)->mpduDataOffset = _off)
 
#define WLANHAL_TX_BD_SET_MPDU_HEADER_OFFSET( _bd, _off )    (((tpHalTxBd)_bd)->mpduHeaderOffset = _off)

#define WLANHAL_TX_BD_SET_MPDU_HEADER_LEN( _bd, _len )        (((tpHalTxBd)_bd)->mpduHeaderLength = _len)

#define WLANHAL_TX_BD_SET_MPDU_LEN( _bd, _len )                (((tpHalTxBd)_bd)->mpduLength = _len)

#define WLANHAL_RX_BD_GET_BA_OPCODE(_pvBDHeader)        (((tpHalRxBd)_pvBDHeader)->reorderOpcode)

#define WLANHAL_RX_BD_GET_BA_FI(_pvBDHeader)            (((tpHalRxBd)_pvBDHeader)->reorderFwdIdx)

#define WLANHAL_RX_BD_GET_BA_SI(_pvBDHeader)            (((tpHalRxBd)_pvBDHeader)->reorderSlotIdx)

#define WLANHAL_RX_BD_GET_BA_CSN(_pvBDHeader)           (((tpHalRxBd)_pvBDHeader)->currentPktSeqNo)

#define WLANHAL_RX_BD_GET_BA_ESN(_pvBDHeader)           (((tpHalRxBd)_pvBDHeader)->expectedPktSeqNo)

#define WLANHAL_RX_BD_GET_RXP_FLAGS(_pvBDHeader)            (((tpHalRxBd)_pvBDHeader)->rxpFlags)

#define WLANHAL_RX_BD_ASF_SET                1 /*The value of the field when set and pkt is AMSDU*/

#define WLANHAL_RX_BD_FSF_SET               1

#define WLANHAL_RX_BD_LSF_SET               1

#define WLANHAL_RX_BD_AEF_SET               1

 
#define WLANHAL_RX_BD_LLC_PRESENT            0 /*The value of the field when LLC is present*/

#define WLANHAL_RX_BD_FT_DONE                  1 /* The value of the field when frame xtl was done*/


/*==========================================================================

  FUNCTION    WLANHAL_RxBD_GetFrameTypeSubType

  DESCRIPTION 
    Called by TL to retrieve the type/subtype of the received frame.

  DEPENDENCIES 
    TL should pass a valid RxBD buffer pointer.
    
  PARAMETERS 

    IN
    pvBDHeader:    Void pointer to the RxBD buffer.
    usFrmCtrl:the frame ctrl of the 802.11 header 
   
  RETURN VALUE
    A byte which contains both type and subtype info. LSB four bytes (b0 to b3)
    is subtype and b5-b6 is type info. 

  SIDE EFFECTS 
  
============================================================================*/

tANI_U8 WLANHAL_RxBD_GetFrameTypeSubType(v_PVOID_t _pvBDHeader, tANI_U16 usFrmCtrl);


#define HAL_TXCOMP_REQUESTED_MASK           0x1  //bit 0 for TxComp intr requested. 
#define HAL_USE_SELF_STA_REQUESTED_MASK     0x2  //bit 1 for STA overwrite with selfSta Requested.

/*==========================================================================

  FUNCTION    WLANHAL_FillTxBd

  DESCRIPTION 
    Called by PE to register as a client for management frames delivery. 

  DEPENDENCIES 
    TL must be initialized before this API can be called. 
    
  PARAMETERS 

    IN
    pAdapter:       pointer to the global adapter context;a handle to TL's 
                    control block can be extracted from its context 
    vosFrmBuf:     pointer to a vOSS buffer containing the management  
                    frame to be transmitted
    usFrmLen:       the length of the frame to be transmitted; information 
                    is already included in the vOSS buffer
    wFrmType:       the type of the frame being transmitted
    tid:            tid used to transmit this frame
    pfnCompTxFunc:  function pointer to the transmit complete routine
    voosBDHeader:   pointer to the BD header
    txFlag:  can have appropriate bit setting as required
    
                #define HAL_TXCOMP_REQUESTED_MASK           0x1  //bit 0 for TxComp intr requested. 
                #define HAL_USE_SELF_STA_REQUESTED_MASK    0x2  //bit 1 for STA overwrite with selfSta Requested.
                
    uTimestamp:     pkt timestamp

   
  RETURN VALUE
    The result code associated with performing the operation  

  SIDE EFFECTS 
  
============================================================================*/
VOS_STATUS WLANHAL_FillTxBd(void *pAdaptor, tANI_U8 typeSubtype, void *pDestMacAddr,
        tANI_U8* ptid, tANI_U8 disableFrmXtl, void *pTxBd, tANI_U8 txFlag, tANI_U32 timeStamp);

/** To swap the RxBD */
void WLANHAL_SwapRxBd(tANI_U8 *pBd);

void WLANHAL_RxAmsduBdFix(void *pVosGCtx,v_PVOID_t _pvBDHeader);

#ifdef WLAN_PERF
tANI_U32 WLANHAL_TxBdFastFwd(void *pAdaptor, tANI_U8 *pDestMac, tANI_U8 tid, tANI_U8 unicastDst,  void *pTxBd, tANI_U16);
#endif

#define tHalRxBd	halRxBd_type
#define tpHalRxBd	phalRxBd_type

#define tHalTxBd    halTxBd_type
#define tpHalTxBd    pHalTxBd_type

#endif


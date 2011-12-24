/*****************************************************************************
  FILENAME:       mpdu.h

  DESCRIPTION:    Contains all MPDU frame formatting structures

 Copyright (c) 2011 Qualcomm Atheros, Inc. 
 All Rights Reserved. 
 Qualcomm Atheros Confidential and Proprietary. 

 Copyright (C) 2006 Airgo Networks, Incorporated
*****************************************************************************/
#ifndef MPDU_H
#define MPDU_H

#include "palTypes.h"
typedef enum
{
  MGMT_FRAME_TYPE = 0 << 2,
  CTRL_FRAME_TYPE = 1 << 2,
  DATA_FRAME_TYPE = 2 << 2,
  RSVD_FRAME_TYPE = 3 << 2
}eMPDUFrameType;

typedef enum
{
  ASSOCIATION_REQ         = 0 << 4,
  ASSOCIATION_RESPONSE    = 1 << 4,
  REASSOCIATION_REQ       = 2 << 4,
  REASSOCIATION_RESPONSE  = 3 << 4,
  PROBE_REQ               = 4 << 4,
  PROBE_RESPONSE          = 5 << 4,
  /* reserved values 6-7 */
  BEACON                  = 8 << 4,
  ATIM                    = 9 << 4,
  DISASSOCIATION          = 10 << 4,
  AUTHENTICATION          = 11 << 4,
  DEAUTHENTICATION        = 12 << 4,
  /* reserved values 13 - 15 */
  MAX_MGMT_SUBTYPE        = 0xFFFF
}eMPDUFrameMgmtSubtype;

typedef enum
{
  /* reserved values 0 - 9 */
  
  POWER_SAVE_POLL         = 10 << 4,
  REQ_TO_SEND             = 11 << 4,
  CLEAR_TO_SEND           = 12 << 4,
  ACKNOWLEDGEMENT         = 13 << 4,
  CF_END                  = 14 << 4,
  CF_END_ACK              = 15 << 4,
  MAX_CTRL_SUBTYPE        = 0xFFFF
}eMPDUFrameCtrlSubtype;

typedef enum
{
  DATA                    = 0  << 4,
  DATA_CF_ACK             = 1  << 4,
  DATA_CF_POLL            = 2  << 4,
  DATA_CF_ACK_POLL        = 3  << 4,
  NO_DATA_FRAME           = 4  << 4,
  CF_ACK                  = 5  << 4,
  CF_POLL                 = 6  << 4,
  CF_ACK_POLL             = 7  << 4,
  QOS_DATA                = 8  << 4,
  QOS_DATA_CF_ACK         = 9  << 4,
  QOS_DATA_CF_POLL        = 10 << 4,
  QOS_DATA_CF_ACK_POLL    = 11 << 4,
  QOS_NO_DATA_FRAME       = 12 << 4,
  QOS_CF_ACK              = 13 << 4,
  QOS_CF_POLL             = 14 << 4,
  QOS_CF_ACK_POLL         = 15 << 4,
  MAX_DATA_SUBTYPE        = 0xFFFF
}eMPDUFrameDataSubtype;


typedef tANI_U8 tMacFrameType;
typedef tANI_U8 tMacFrameCtrl;
typedef tANI_U16 tQosControl;

typedef struct
{
  tMacFrameType frameType;   //bits 0-1 = protocol version = 0
                          //bits 2-3 = type of frame
                          //bits 4-7 = subtype of frame
  tMacFrameCtrl frameCtrl;   //bit 0 = To DS
                          //bit 1 = From DS
                          //bit 2 = More Fragments
                          //bit 3 = Retry
                          //bit 4 = Power Management
                          //bit 5 = More Data
                          //bit 6 = WEP
                          //bit 7 = Order, 0=
  tANI_U16 duration;
  tANI_U8  MACAddress1[6];
  tANI_U8  MACAddress2[6];
  tANI_U8  MACAddress3[6];
  tANI_U16 seqNum;
}sMPDUHeader;


typedef struct 
{
  /* The MAC header for AP to AP with Address4: */
  tMacFrameType frameType;   //bits 0-1 = protocol version = 0
                          //bits 2-3 = type of frame
                          //bits 4-7 = subtype of frame
  tMacFrameCtrl frameCtrl;   //bit 0 = To DS
                          //bit 1 = From DS
                          //bit 2 = More Fragments
                          //bit 3 = Retry
                          //bit 4 = Power Management
                          //bit 5 = More Data
                          //bit 6 = WEP
                          //bit 7 = Order, 0=
  tANI_U16 duration;
  tANI_U8  MACAddress1[6];
  tANI_U8  MACAddress2[6];
  tANI_U8  MACAddress3[6];
  tANI_U16 seqNum;
  tANI_U8  MACAddress4[6];
}sApToApMpduHeader;




typedef struct 
{
  /* The MAC header for AP to AP with Address4: */
  tMacFrameType frameType;   //bits 0-1 = protocol version = 0
                          //bits 2-3 = type of frame
                          //bits 4-7 = subtype of frame
  tMacFrameCtrl frameCtrl;   //bit 0 = To DS
                          //bit 1 = From DS
                          //bit 2 = More Fragments
                          //bit 3 = Retry
                          //bit 4 = Power Management
                          //bit 5 = More Data
                          //bit 6 = WEP
                          //bit 7 = Order, 0=
  tANI_U16 duration;
  tANI_U8  MACAddress1[6];
  tANI_U8  MACAddress2[6];
  tANI_U8  MACAddress3[6];
  tANI_U16 seqNum;
  tQosControl qosCtrl;
}sQosMpduHeader;

typedef struct 
{
  /* The MAC header for AP to AP with Address4: */
  tMacFrameType frameType;   //bits 0-1 = protocol version = 0
                             //bits 2-3 = type of frame
                             //bits 4-7 = subtype of frame

  tMacFrameCtrl frameCtrl;   //bit 0 = To DS
                             //bit 1 = From DS
                             //bit 2 = More Fragments
                             //bit 3 = Retry
                             //bit 4 = Power Management
                             //bit 5 = More Data
                             //bit 6 & 7 = WEP/Privacy

  tANI_U16 duration;
  tANI_U8  MACAddress1[6];
  tANI_U8  MACAddress2[6];
  tANI_U8  MACAddress3[6];
  tANI_U16 seqNum;
  tANI_U8  MACAddress4[6];
  tQosControl qosCtrl;
}sQosApMpduHeader;

typedef enum
{
    MPDU_HEADER_BASIC,
    MPDU_HEADER_WDS,            //Ap to Ap transmission
    MPDU_HEADER_QOS,
    MPDU_HEADER_QOS_WDS,        //Ap to Ap transmission
}eMpduHeaderType;

typedef union
{
    sMPDUHeader         basicMpduHdr;
    sApToApMpduHeader   wdsMpduHdr;
    sQosMpduHeader      qosMpduHdr;
    sQosApMpduHeader    qosWdsMpduHdr;
}uMpduHeader;


/*
    The following macros allow MPDU header fields to be manipulated. 
    GET_ macros return the value found in a particular field within the header variables defined above.
    SET_ macros allow a value to be set in a particular field within the header variables defined above.
    TEST_ macros test individual bits or values and return a boolean result.
    
*/


//tMPDUHeader.frameType bit fields
#define FRAME_TYPE_MASK          (0x000C)
#define FRAME_SUBTYPE_MASK       (0x00F0)
#define FRAME_TYPE_SUBTYPE_MASK  (0x00FC)

#define GET_PROTOCOL_VERSION(frameTypeVar)          (frameTypeVar & 0x03)
#define SET_PROTOCOL_VERSION(frameTypeVar, val)     ((frameTypeVar & ~0x03) | val)


#define GET_MPDU_TYPE(frameTypeVar)          (eMPDUFrameType)(frameTypeVar & FRAME_TYPE_MASK)
#define GET_MPDU_MGMT_SUBTYPE(frameTypeVar)  (eMPDUFrameMgmtSubtype)(frameTypeVar & FRAME_SUBTYPE_MASK)
#define GET_MPDU_CTRL_SUBTYPE(frameTypeVar)  (eMPDUFrameCtrlSubtype)(frameTypeVar & FRAME_SUBTYPE_MASK)
#define GET_MPDU_DATA_SUBTYPE(frameTypeVar)  (eMPDUFrameDataSubtype)(frameTypeVar & FRAME_SUBTYPE_MASK)

#define SET_MPDU_TYPE(frameTypeVar, val)          (((frameTypeVar & ~FRAME_TYPE_MASK) | (val << 2)))
#define SET_MPDU_SUBTYPE(frameTypeVar, val)       (((frameTypeVar & ~FRAME_SUBTYPE_MASK) | (val << 4)))

#define FRAME_QOS_DATA_SUBTYPE   (0x0088)
#define TEST_MPDU_QOS_DATA_SUBTYPE(frameTypeVar)  ((frameTypeVar & FRAME_QOS_DATA_SUBTYPE) == FRAME_QOS_DATA_SUBTYPE)

//qos subtypes in frameType
#define QOS_ACK_SUBTYPE_BIT      (0x0010)
#define QOS_POLL_SUBTYPE_BIT     (0x0020)
#define QOS_NON_DATA_SUBTYPE_BIT (0x0040)

#define TEST_QOS_ACK(frameTypeVar)               (frameTypeVar & QOS_ACK_SUBTYPE_BIT)
#define TEST_QOS_POLL(frameTypeVar)              (frameTypeVar & QOS_POLL_SUBTYPE_BIT)
#define TEST_QOS_NON_DATA(frameTypeVar)          (frameTypeVar & QOS_NON_DATA_SUBTYPE_BIT)

#define SET_QOS_ACK(frameTypeVar, val)           ((frameTypeVar & ~QOS_ACK_SUBTYPE_BIT) | (val << 4))
#define SET_QOS_POLL(frameTypeVar, val)          ((frameTypeVar & ~QOS_POLL_SUBTYPE_BIT) | (val << 5))
#define SET_QOS_NON_DATA(frameTypeVar, val)      ((frameTypeVar & ~QOS_NON_DATA_SUBTYPE_BIT) | (val << 6))


//tMPDUHeader.frameCtrl bit fields
#define FCTL_TO_DS        (0x01)
#define FCTL_FROM_DS      (0x02)
#define FCTL_AP_TO_AP     (0x03)
#define FCTL_MORE_FRAGS   (0x04)
#define FCTL_RETRY        (0x08)
#define FCTL_PWR_MGMT     (0x10)
#define FCTL_MORE_DATA    (0x20)
#define FCTL_PRIVACY      (0xC0)

#define TEST_TO_DS_BIT(frameCtrlVar)         ((frameCtrlVar & FCTL_TO_DS     ) ? eANI_BOOLEAN_TRUE: eANI_BOOLEAN_FALSE)
#define TEST_FROM_DS_BIT(frameCtrlVar)       ((frameCtrlVar & FCTL_FROM_DS   ) ? eANI_BOOLEAN_TRUE: eANI_BOOLEAN_FALSE)
#define TEST_AP_TO_AP(frameCtrlVar)          (((frameCtrlVar & FCTL_AP_TO_AP) == FCTL_AP_TO_AP ) ? eANI_BOOLEAN_TRUE: eANI_BOOLEAN_FALSE)
#define TEST_MORE_FRAGS_BIT(frameCtrlVar)    ((frameCtrlVar & FCTL_MORE_FRAGS) ? eANI_BOOLEAN_TRUE: eANI_BOOLEAN_FALSE)
#define TEST_RETRY_BIT(frameCtrlVar)         ((frameCtrlVar & FCTL_RETRY     ) ? eANI_BOOLEAN_TRUE: eANI_BOOLEAN_FALSE)
#define TEST_PWR_MGMT_BIT(frameCtrlVar)      ((frameCtrlVar & FCTL_PWR_MGMT  ) ? eANI_BOOLEAN_TRUE: eANI_BOOLEAN_FALSE)
#define TEST_MORE_DATA_BIT(frameCtrlVar)     ((frameCtrlVar & FCTL_MORE_DATA ) ? eANI_BOOLEAN_TRUE: eANI_BOOLEAN_FALSE)
#define GET_PRIVACY(frameCtrlVar)            ((frameCtrlVar & FCTL_PRIVACY   ) >> 6)

#define SET_TO_FROM_DS_BIT(frameCtrlVar, val)    ((frameCtrlVar & ~(FCTL_TO_DS | FCTL_FROM_DS)) | (val << 0))
#define SET_AP_TO_AP(frameCtrlVar, val)          (frameCtrlVar | FCTL_AP_TO_AP)
#define SET_MORE_FRAGS_BIT(frameCtrlVar, val)    ((frameCtrlVar & ~FCTL_MORE_FRAGS) | (val << 2))
#define SET_RETRY_BIT(frameCtrlVar, val)         ((frameCtrlVar & ~FCTL_RETRY     ) | (val << 3))
#define SET_PWR_MGMT_BIT(frameCtrlVar, val)      ((frameCtrlVar & ~FCTL_PWR_MGMT  ) | (val << 4))
#define SET_MORE_DATA_BIT(frameCtrlVar, val)     ((frameCtrlVar & ~FCTL_MORE_DATA ) | (val << 5))
#define SET_PRIVACY(frameCtrlVar, val)           ((frameCtrlVar & ~FCTL_PRIVACY   ) | (val << 6))


// tMPDUHeader.duration field 
#define FDUR_CTRL           (0x8000)  //bit 15 used to indicate field usage apart from frame subtypes
#define FDUR_DURATION_MASK  (0x7FFF)  //only bits 0 - 14 for duration value
#define FDUR_AID_MASK       (0x3FFF)  //only bits 0 - 13 for AID, only used for Power Save Poll frames

// this tests a duration field to see if it is a NAV update or an AID for a power save poll frame
#define TEST_DUR_UPDATE_VAL(dur)        ((dur & FDUR_CTRL) ? eANI_BOOLEAN_TRUE: eANI_BOOLEAN_FALSE)

#define GET_DURATION(dur)               (dur & FDUR_DURATION_MASK)
#define GET_AID(dur)                    (dur & FDUR_AID_MASK)

#define SET_DURATION(dur, val)               ((dur & ~FDUR_DURATION_MASK) | (val))
#define SET_AID(dur, val)                    ((dur & ~FDUR_AID_MASK) | (val))

//tMPDUHeader.seqNum field
#define GET_FRAG_NUM(seqNum)  ((seqNum) & 0x000F)
#define GET_SEQ_NUM(seqNum)   (((seqNum) & 0xFFF0) >> 4)

#define SET_FRAG_NUM(seqNum, val)  (((seqNum) & ~0x000F) | val)
#define SET_SEQ_NUM(seqNum, val)   (((seqNum) & ~0xFFF0) | (val << 4))

//tMPDUHeader MAC Address testing
#define TEST_UNICAST_ADDRESS(MACAddr)   ((MACAddr[0] & 0x01) ? eANI_BOOLEAN_FALSE : eANI_BOOLEAN_TRUE)
#define TEST_MULTICAST_ADDRESS(MACAddr) ((MACAddr[0] & 0x01) ? eANI_BOOLEAN_TRUE : eANI_BOOLEAN_FALSE)


//tQosControl common bits
#define GET_TID(qos)                    ((qos & 0x000F))
#define GET_ACK_POLICY(qos)             ((qos & 0x0060) >> 5)
#define GET_RSVD(qos)                   ((qos & 0x0080) >> 7)

#define SET_TID(qos, val)               ((qos & ~0x000F) | val)
#define SET_ACK_POLICY(qos, val)        ((qos & ~0x0060) | (val << 5))
#define SET_RSVD(qos, val)              ((qos & ~0x0080) | (val << 7))


//tQosControl bits set by QAP 
#define GET_EOSP(qos)                   ((qos & 0x0010) >> 4)
#define SET_EOSP(qos, val)              ((qos & ~0x0010) | (val << 4))

//tQosControl bits set by HC CF-Poll
#define GET_CF_POLL_TX_LIMIT(qos)       ((qos & 0xFF00) >> 8)
#define SET_CF_POLL_TX_LIMIT(qos, val)  ((qos & ~0xFF00) | (val << 8))

//tQosControl bits set by HC CF-other
#define GET_PS_BUFFER_STATE(qos)        GET_CF_POLL_TX_LIMIT(qos)
#define SET_PS_BUFFER_STATE(qos, val)   SET_CF_POLL_TX_LIMIT(qos, val)

//tQosControl bits set by QSTA data frames
#define GET_QSTA_EOSP(qos)              GET_EOSP(qos)
#define GET_QSTA_TXOP_DUR_REQUEST(qos)  (GET_QSTA_EOSP(qos) ? eANI_BOOLEAN_FALSE: GET_CF_POLL_TX_LIMIT(qos))
#define GET_QSTA_QUEUE_SIZE(qos)        (GET_QSTA_EOSP(qos) ? GET_CF_POLL_TX_LIMIT(qos): eANI_BOOLEAN_FALSE)

#define SET_QSTA_EOSP(qos, val)             SET_EOSP(qos, val)
#define SET_QSTA_TXOP_DUR_REQUEST(qos, val) SET_CF_POLL_TX_LIMIT(qos, val)
#define SET_QSTA_QUEUE_SIZE(qos, val)       SET_CF_POLL_TX_LIMIT(qos, val)






/*
    The following macros take a pointer to an mpdu header as a parameter and uses macros defined above.
    CHK_ macros return boolean results.
*/
// The size of the MAC Header is different if the packet has crossed (bridged) the DS from one BSS to another.
#define MAC_HDR_SIZE          (sizeof(tMPDUHeader))
#define MIN_MPDU_HDR_SIZE     (sizeof(tMPDUHeader))
#define MAX_MPDU_HDR_SIZE     (sizeof(tQosApMpduHeader))





#define CHK_FRAME_TO_DS(hPtr)       (TEST_TO_DS_BIT((hPtr)->frameCtrl))
#define CHK_FRAME_FROM_DS(hPtr)     (TEST_FROM_DS_BIT((hPtr)->frameCtrl))
#define CHK_FRAME_AP_TO_AP(hPtr)    (TEST_AP_TO_AP((hPtr)->frameCtrl))
#define CHK_FRAME_WEP_ENCRYPT(hPtr) (TEST_PRIVACY_BIT((hPtr)->frameCtrl))

#define CHK_DUR_UPDATE_VAL(hptr)    (TEST_DUR_UPDATE_VAL((hPtr)->duration) ? eANI_BOOLEAN_TRUE: eANI_BOOLEAN_FALSE)



//MPDU Header SIZE macro.
#define GET_SIZE_MPDU_HDR(hPtr)    ((GET_MPDU_QOS_DATA_SUBTYPE((hPtr)->frameType) == FRAME_QOS_DATA_SUBTYPE  \
                                     ? (CHK_FRAME_AP_TO_AP(hPtr) == eANI_BOOLEAN_TRUE)                                    \
                                       ? sizeof(tQosApMpduHeader) : sizeof(tQosMpduHeader)              \
                                     : (CHK_FRAME_AP_TO_AP(hPtr) == eANI_BOOLEAN_TRUE)                                    \
                                       ? sizeof(sApToApMpduHeader) : sizeof(tMPDUHeader)))




/*
    Addresses 

    Table 4, pg 29
      To   From   Addr    Addr    Addr    Addr   Usage 
      DS   DS      1       2       3       4
      ---  ---    ------  ------  ------  ------  ---------------------------------------
      0    0      DA      SA      BSSID   N/A     STA to STA traffic in an IBSS and QSTA-to-QSTA traffic in a QBSS
      0    1      DA      BSSID   SA      N/A     AP-to-STA traffic in a BSS
      1    0      BSSID   SA      DA      N/A     STA-to-AP traffic in a BSS
      1    1      RA      TA      DA      SA      WDS traffic between APs (AP to AP)
*/
typedef enum
{
    DS_STA_TO_STA = 0,
    DS_AP_TO_STA  = 1,
    DS_STA_TO_AP  = 2,
    DS_AP_TO_AP   = 3
}eDS;

#define GET_TX_ADDR_PTR(hPtr)     ((hPtr)->MACAddress2)
#define GET_DEST_ADDR_PTR(hPtr)   (CHK_FRAME_TO_DS(hPtr)                                 \
                                    ? ((hPtr)->MACAddress3)                              \
                                    : ((hPtr)->MACAddress1))
#define GET_SRC_ADDR_PTR(hPtr)    (CHK_FRAME_AP_TO_AP(hPtr)                              \
                                    ? (((sApToApMpduHeader *)hPtr)->MACAddress4)         \
                                    : (CHK_FRAME_FROM_DS(hPtr)                           \
                                        ? ((hPtr)->MACAddress3)                          \
                                        : ((hPtr)->MACAddress2)))

#define GET_BSSID_ADDR_PTR(hPtr)     (CHK_FRAME_TO_DS(hPtr) ?                                                    \
                                       (CHK_FRAME_FROM_DS(hPtr) ? NULL : ((hPtr)->MACAddress1)) :                \
                                       (CHK_FRAME_FROM_DS(hPtr) ? ((hPtr)->MACAddress2) : ((hPtr)->MACAddress3)) \
                                     )


//Duration stuff
#define GET_FRAME_DURATION(hPtr)         ((hPtr)->duration & FDUR_DURATION_MASK)
#define GET_PWR_SAVE_POLL_AID(hPtr)      ((hPtr)->duration & FDUR_AID_MASK)


//Priority stuff
#define GET_FRAME_PRIORITY(hPtr)   (GET_TID(((tQosMpduHeader *)hPtr)->qosCtrl))



#endif      // MPDU_H

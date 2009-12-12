/** ------------------------------------------------------------------------- *
    ------------------------------------------------------------------------- *


    \file dvtFrames.h

    \brief Captures frame configuration types.

    $Id$

    Copyright (C) 2006 Airgo Networks, Incorporated


   ========================================================================== */

#ifndef DVTFRAMES_H
#define DVTFRAMES_H



//MPDU Header Configuration
#include "mpdu.h"


typedef struct
{
    tANI_U32 min;             //minimum random value
    tANI_U32 max;             //maximum random value
}sRandomize;


typedef struct
{
    tANI_U32 min;             //minimum sequential value
    tANI_U32 max;             //maximum sequential value
}sSequential;


typedef struct
{
    tANI_U32 value;           //specific value
}sFixed;


typedef struct
{
    void *data;       //points to specific template data to be used
}sDataTemplate;



typedef enum
{
    FORMAT_NONE,
    FORMAT_RANDOMIZE,
    FORMAT_SEQUENTIAL,
    FORMAT_FIXED_VALUE,
    FORMAT_TEMPLATE,
    FORMAT_FROM_SYSTEM,     //use existing system configuration/Mibs to format this field
    MAX_FORMAT_OPTION
}eTestFormat;

typedef union
{
    sRandomize      rand;
    sSequential     seq;
    sFixed          fixed;
    sDataTemplate   dataTemplate;
    tANI_U32 reserved;                   //reserve a whole word for this union
}uTestFormat;

typedef struct
{
    eTestFormat option;
    uTestFormat params;
}sTestFormat;








typedef enum
{
    //payload options
    DVT_FRAME_SEGMENTATION,
    DVT_FRAME_LENGTH,
    DVT_FRAME_PAYLOAD_BYTE,

    //MPDU Header options
    DVT_FRAME_PROTOCOL_VER,
    DVT_FRAME_FRAME_TYPE,
    DVT_FRAME_FRAME_SUBTYPE,
    DVT_FRAME_TO_FROM_DS,
    DVT_FRAME_MORE_FRAG,
    DVT_FRAME_RETRY,
    DVT_FRAME_PWR_MGMT,
    DVT_FRAME_MORE_DATA,
    DVT_FRAME_PRIVACY,
    DVT_FRAME_DURATION_ID,
    DVT_FRAME_ADDR1,
    DVT_FRAME_ADDR2,
    DVT_FRAME_ADDR3,
    DVT_FRAME_ADDR4,
    DVT_FRAME_FRAG_NUM,
    DVT_FRAME_SEQUENCE_NUM,
    DVT_FRAME_QOS_TID,
    DVT_FRAME_QOS_RSVD,
    DVT_FRAME_QOS_ACK_POLICY,
    DVT_FRAME_RSVD,
    DVT_FRAME_QOS_EOSP,
    DVT_FRAME_QOS_TXOP_LIMIT,

    //Tx BD Options
    DVT_TX_BD_BDT,
    DVT_TX_BD_DPU_NC,
    DVT_TX_BD_DPU_NE,
    DVT_TX_BD_SW_BD_TYPE,
    DVT_TX_BD_RESERVED1,
    DVT_TX_BD_STA_SIGNATURE,
    DVT_TX_BD_DPU_SIGNATURE,
    DVT_TX_BD_DPU_RF,
    DVT_TX_BD_RESERVED3,
    DVT_TX_BD_ENABLE_AGGREGATE,
    DVT_TX_BD_OVERWRITE_PHY,
    DVT_TX_BD_FORCE_MAC_PROT,
    DVT_TX_BD_FORCE_ACK_POLICY,
    DVT_TX_BD_TX_INDICATION,
    DVT_TX_BD_KEEP_SEQ_NUM,
    DVT_TX_BD_RESERVED2,
    DVT_TX_BD_TAIL_PDU_IDX,
    DVT_TX_BD_HEAD_PDU_IDX,
    DVT_TX_BD_PDU_COUNT,
    DVT_TX_BD_MPDU_DATA_OFFSET,
    DVT_TX_BD_MPDU_HEADER_OFFSET,
    DVT_TX_BD_MPDU_HEADER_LENGTH,
    DVT_TX_BD_RATE_INDEX,
    DVT_TX_BD_TID,
    DVT_TX_BD_FRAGMENT,
    DVT_TX_BD_LAST_FRAGMENT,
    DVT_TX_BD_RESERVED4,
    DVT_TX_BD_MPDU_LENGTH,
    DVT_TX_BD_RESERVED5,
    DVT_TX_BD_TA_INDEX,
    DVT_TX_BD_RA_INDEX,
    DVT_TX_BD_DPU_DESC_IDX,
    DVT_TX_BD_TIMESTAMP,
    DVT_TX_BD_RESERVED6,
    DVT_TX_BD_RETRY_COUNT,

    MAX_DVT_FRAME_OPTION

}eFrameOptions;

typedef struct
{
    tANI_U32    hdrLength;      //MPDU header length varies with Addr4 and Qos options

    sTestFormat protocolVer;
    sTestFormat frameType;
    sTestFormat frameSubtype;
    sTestFormat toFromDs;
    sTestFormat moreFrag;       //overwritten by SoftMac
    sTestFormat retry;          //overwritten by SoftMac
    sTestFormat pwrMgmt;        //overwritten by SoftMac
    sTestFormat moreData;
    sTestFormat privacy;
    sTestFormat durationId;     //overwritten by SoftMac
    sTestFormat addr1;
    sTestFormat addr2;
    sTestFormat addr3;
    sTestFormat addr4;
    sTestFormat fragNum;        //overwritten by SoftMac
    sTestFormat sequenceNum;
    sTestFormat qosTID;
    sTestFormat qosRsvd;
    sTestFormat qosAckPolicy;
    sTestFormat Rsvd;
    sTestFormat qosEOSP;
    sTestFormat qosTxopLimit;
}sFrameHdrFormatOptions;


typedef struct
{
    sTestFormat segmentation;   //defines different ways to present the frame as a list of segments - applies to bd and frame contents
    sTestFormat length;        //defines different ways to specify the length
    sTestFormat payloadByte;       //defines different ways to create the payload
}sFramePayloadFormatOptions;


#include "halTx.h"



typedef struct
{
    /* 0x00 */
    sTestFormat bdt;                   //2 bits
    sTestFormat dpuNC;                 //1 bits
    sTestFormat dpuNE;                 //1 bits
    sTestFormat swBdType;              //2 bits
    sTestFormat reserved1;             //10 bits
    sTestFormat staSignature;          //4 bits
    sTestFormat dpuSignature;          //4 bits
    sTestFormat dpuRF;                 //8 bits

    /* 0x04 */
    sTestFormat reserved3;             //16 bits
    sTestFormat enableAggregate;       //1 bits
    sTestFormat overwritePhy;          //1 bits
    sTestFormat forceMacProt;          //2 bits
    sTestFormat forceAckPolicy;        //3 bits
    sTestFormat txIndication;          //1 bits
    sTestFormat keepSeqNum;            //1 bits
    sTestFormat reserved2;             //7 bits

    /* 0x08 */
    sTestFormat tailPduIdx;            //16 bits
    sTestFormat headPduIdx;            //16 bits

    /* 0x0c */
    sTestFormat pduCount;              //7 bits
    sTestFormat mpduDataOffset;        //9 bits
    sTestFormat mpduHeaderOffset;      //8 bits
    sTestFormat mpduHeaderLength;      //8 bits

    /* 0x10 */
    sTestFormat rateIndex;             //8 bits
    sTestFormat tid;                   //4 bits
    sTestFormat fragment;              //1 bits
    sTestFormat lastFragment;          //1 bits
    sTestFormat reserved4;             //2 bits
    sTestFormat mpduLength;            //16 bits

    /* 0x14 */
    sTestFormat reserved5;             //8 bits
    sTestFormat taIndex;               //8 bits
    sTestFormat raIndex;               //8 bits
    sTestFormat dpuDescIdx;            //8 bits

    /* 0x18 */
    sTestFormat timestamp;             //16 bits
    sTestFormat reserved6;             //8 bits
    sTestFormat retryCount;            //8 bits

    /* 0x1c */
    tANI_U8 phyCmds[20];                     /* Reserved PHY command when overwritePhy bit is set */
}sDvtTxBdFormatOptions;


#include "smacHostIf.h"
typedef tSmacBmuWqId eSmacBmuWqId;  //this type is an enumeration


#include "dvtQueues.h"

#define BD_SIZE 128
#define MAX_TX_PAYLOAD 4096

typedef struct
{
    eDvtTxQueue txQueueId;                  //transmit queue to put this on
    tANI_U32 totalNumber;                   //total number of frames to create with these parameters
    tANI_U32 threadNumber;                  //number of frames to create at the driver level within one deferred procedure call
    tANI_BOOLEAN bdPresent;                 //this option just doesn't make sense for us to test - always enabled
    eSmacBmuWqId dxeOutWQ;                  //which workqueue will the DXE transfer this to
    sFrameHdrFormatOptions hdr;             //collection of header options
    sFramePayloadFormatOptions payload;     //collection of payload options
    sDvtTxBdFormatOptions bdFormatOptions;
}sDvtFrameSpec;


typedef struct
{
    tANI_U32 min;           //absolute minumum value allowed
    tANI_U32 max;           //absolute maximum value allowed
    tANI_U32 last;          //value last used in sequential setting
}sDvtTestBounds;




#endif

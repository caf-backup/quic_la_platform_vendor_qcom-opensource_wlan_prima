/** ------------------------------------------------------------------------- *
    ------------------------------------------------------------------------- *


    \file halNv.h

    \brief halNv services

    $Id$

    Copyright (C) 2006 Airgo Networks, Incorporated


   ========================================================================== */

#ifndef HALNV_H
#define HALNV_H


#include "halNvTables.h"


typedef enum
{
    //Common Nv Fields
    NV_COMMON_PRODUCT_ID,               // 0
    NV_COMMON_PRODUCT_BANDS,            // 1
    NV_COMMON_NUM_OF_TX_CHAINS,         // 2
    NV_COMMON_NUM_OF_RX_CHAINS,         // 3
    NV_COMMON_MAC_ADDR,                 // 4
    NV_COMMON_MFG_SERIAL_NUMBER,        // 5
    NV_COMMON_WLAN_NV_REV_ID,           // 6

    NUM_NV_FIELDS,
    NV_MAX_FIELD = 0xFFFFFFFF  /* define as 4 bytes data */

}eNvField;


#define NV_FIELD_MAC_ADDR_SIZE      6
#define NV_FIELD_MFG_SN_SIZE        40
typedef enum
{
    PRODUCT_BAND_11_B_G     = 0,    //Gen6.0 is only this setting
    PRODUCT_BAND_11_A_B_G   = 1,
    PRODUCT_BAND_11_A       = 2,

    NUM_PRODUCT_BANDS
}eNvProductBands;           //NV_COMMON_PRODUCT_BANDS


typedef union
{
    //common NV fields
    tANI_U16  productId;
    tANI_U8   productBands;
    tANI_U8   wlanNvRevId;
    tANI_U8   numOfTxChains;
    tANI_U8   numOfRxChains;
    tANI_U8   macAddr[NV_FIELD_MAC_ADDR_SIZE];
    tANI_U8   mfgSN[NV_FIELD_MFG_SN_SIZE];
}uNvFields;



//format of common part of nv
typedef struct
{
    //always ensure fields are aligned to 32-bit boundaries
    tANI_U16  productId;
    tANI_U8   productBands;
    tANI_U8   wlanNvRevId; //0: WCN1312, 1: WCN1314

    tANI_U8   numOfTxChains;
    tANI_U8   numOfRxChains;
    tANI_U8   macAddr[NV_FIELD_MAC_ADDR_SIZE];

    tANI_U8   mfgSN[NV_FIELD_MFG_SN_SIZE];
}sNvFields;


typedef struct
{
    sNvFields fields;
    sNvTables tables;
}sHalNv;


#endif


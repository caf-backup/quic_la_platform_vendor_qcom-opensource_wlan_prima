/** ------------------------------------------------------------------------- * 
    ------------------------------------------------------------------------- *  

  
    \file dvtMgmt.h
  
    \brief dvtMgmt.h
  
    $Id$ 
  
  
    Copyright (c) 2011 Qualcomm Atheros, Inc. 
    All Rights Reserved. 
    Qualcomm Atheros Confidential and Proprietary. 

    Copyright (C) 2006 Airgo Networks, Incorporated
  
   ========================================================================== */

#ifndef DVTMGMT_H
#define DVTMGMT_H

typedef struct
{
    tANI_BOOLEAN b;
    tANI_BOOLEAN g;
    tANI_BOOLEAN a;
    tANI_BOOLEAN edca;
    tANI_BOOLEAN hcca;
    tANI_BOOLEAN n;
    tANI_BOOLEAN greenfield;
}sDvtWlanCapabilities;

typedef struct
{
    tANI_U8              macAddr[6];
    tANI_BOOLEAN         ap;
    tANI_BOOLEAN         configured;
    sDvtWlanCapabilities wlanCapabilities;
}sDvtSimpleMacConfig;

typedef enum
{
    DVT_BSS_INFRASTRUCTURE,     //BSS
    DVT_BSS_INDEPENDENT,        //IBSS
}eDvtBssType;

typedef struct
{
    tANI_U8             bssId[6];
    tANI_U8             reserved;
    tANI_BOOLEAN        configured;
    eDvtBssType bssType;
}sDvtSimpleBssConfig;

typedef struct
{
    tANI_U8             macAddr[6];
    tANI_U8             reserved;
    tANI_BOOLEAN        configured;
}sDvtSimpleStationConfig;


#endif


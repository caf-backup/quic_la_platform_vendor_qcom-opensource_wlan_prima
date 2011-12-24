/** ------------------------------------------------------------------------- *
    ------------------------------------------------------------------------- *


    \file halFwTypes.h

    \brief This header captures types that must be shared in common with individual
            module headers before inclusion into halCommonApi.h.

    $Id$


    Copyright (c) 2011 Qualcomm Atheros, Inc. 
    All Rights Reserved. 
    Qualcomm Atheros Confidential and Proprietary. 
  
    Copyright (C) 2006 Airgo Networks, Incorporated

   ========================================================================== */

#ifndef HALFWTYPES_H
#define HALFWTYPES_H

#include <Wlanfw_If.h>


typedef tANI_U8 tANI_BOOLEAN;

typedef enum tagAniBoolean
{
    eANI_BOOLEAN_FALSE = 0,
    eANI_BOOLEAN_TRUE,

    eANI_BOOLEAN_OFF = 0,
    eANI_BOOLEAN_ON = 1,
} eAniBoolean;



#endif


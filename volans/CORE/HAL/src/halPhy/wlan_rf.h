/**
 *

   Copyright (c) 2011 Qualcomm Atheros, Inc. 
   All Rights Reserved. 
   Qualcomm Atheros Confidential and Proprietary. 
  
   Copyright (C) 2006 Airgo Networks, Incorporated

   rf.h defines common data types for RF chips.


   Author:  Mark Nelson
   Date:    3/28/08

   History -
   Date        Modified by              Modification Information
  --------------------------------------------------------------------------

 */

#ifndef RF_H
#define RF_H

#include <halRfTypes.h>


typedef struct
{
    eRfChannels curChannel;
    tANI_U32 revId;
}tRF;


#define RF_CHIP_ID_VOLANS1      4358
#define RF_CHIP_ID_VOLANS2      4608

#define RF_CHIP_VERSION(x)      (pMac->hphy.rf.revId >= (x))

#endif /* RF_H */

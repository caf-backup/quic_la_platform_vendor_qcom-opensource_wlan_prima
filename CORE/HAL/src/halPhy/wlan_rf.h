/**
 *
   Airgo Networks, Inc proprietary.
   All Rights Reserved, Copyright 2008
   This program is the confidential and proprietary product of Airgo Networks Inc.
   Any Unauthorized use, reproduction or transfer of this program is strictly prohibited.


   rf.h defines common data types for RF chips.


   Author:	Mark Nelson
   Date:	3/28/08

   History -
   Date	       Modified by	            Modification Information
  --------------------------------------------------------------------------

 */

#ifndef RF_H
#define RF_H

#include <halRfTypes.h>

#if defined(RF_CHIP_GEMINI)
#include "rfGemini.h"
#endif



typedef enum
{
    GEMINI_CHIP = 0,
    MAX_RF_CHIPS
}eRfChipSelect;


typedef struct
{
    eRfChannels curChannel;
    tANI_U32 revId;
}tRF;




#endif /* RF_H */

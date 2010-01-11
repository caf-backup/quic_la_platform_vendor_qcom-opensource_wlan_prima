/** ------------------------------------------------------------------------- *
    ------------------------------------------------------------------------- *


    \file asicPhyDbg.h

    \brief Definitions for asicPhyDbg.c

    $Id$

    Copyright (C) 2006 Airgo Networks, Incorporated


   ========================================================================== */

#ifndef ASICPHYDBG_H
#define ASICPHYDBG_H

#include "asicTypes.h"

#define ONE_MICROSECOND             (160)
#define DEFAULT_INTERFRAME_SPACE    (ONE_MICROSECOND * 10) //10 microseconds



typedef enum
{
    PHYDBG_TX_IDLE          = 0,
    PHYDBG_TX_START         = 1,
    PHYDBG_TX_WARMUP        = 2,
    PHYDBG_TX_INFD          = 3,
    PHYDBG_TX_CMD           = 4,
    PHYDBG_TX_SVC           = 5,
    PHYDBG_TX_PYLDF         = 6,
    PHYDBG_TX_PYLDR         = 7,
    PHYDBG_TX_CRC           = 8,
    PHYDBG_TX_FLUSH         = 9,
    PHYDBG_TX_TXDONEWAIT    = 10,
    PHYDBG_TX_TIFWAIT       = 11,
    PHYDBG_TX_MAX_VAL = 0XFFFFFFFF //dummy added to change enum to 4 bytes
}ePhyDbgTxStatus;

typedef enum
{
    PHYDBG_PREAMBLE_OFDM,
    PHYDBG_PREAMBLE_GREENFIELD,
    PHYDBG_PREAMBLE_MIXED,
    PHYDBG_PREAMBLE_SHORTB,
    PHYDBG_PREAMBLE_LONGB,
    PHYDBG_PREAMBLE_MAX_VAL = 0XFFFFFFFF //dummy added to change enum to 4 bytes
}ePhyDbgPreamble;


//grab ram
#ifdef SPICA_FPGA
#define GRAB_RAM_DBLOCK_SIZE  (8 * 1024)      //number of samples in full capture
#else
#define GRAB_RAM_DBLOCK_SIZE  (8 * 1024)      //number of samples in full capture
#endif
#define MAX_REQUESTED_GRAB_RAM_SAMPLES 256   //only allow 256 samples at a time to fit within 4K buffer
#define GRAB_RAM_BUFFER_DEPTH   (8 * 1024)   //maximum grab ram size
#define LAST_GRAB_RAM_SAMPLE_INDEX (GRAB_RAM_BUFFER_DEPTH - 1)


typedef struct
{
   tIQAdc rx0;
   tIQAdc rx1;
}tGrabRamSample;


#endif


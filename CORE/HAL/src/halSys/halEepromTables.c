/** ------------------------------------------------------------------------- *
    ------------------------------------------------------------------------- *


    \file halEepromTables.c

    \brief Contains collection of table default values to use in case a table is not found in EEPROM

    $Id$

    Copyright (C) 2006 Airgo Networks, Incorporated


   ========================================================================== */

#ifndef HALEEPROMTABLES_C
#define HALEEPROMTABLES_C

#include "halEepromTables.h"
#ifdef GEN6_OBSOLETE
const sTxIQChannel        txIQTable[NUM_DEMO_CAL_CHANNELS] =
// EEPROM_TABLE_TX_IQ
{
    // typedef struct
    // {
    //     tANI_S9 center;
    //     tANI_S9 offCenter;
    //     tANI_S9 imbalance;
    // }sIQCalValues;

    //sIQCalValues sTxIQChannel[PHY_MAX_TX_CHAINS][NUM_TX_GAIN_STEPS];
    {
        {
            { 0, 0, 0 },    //gain 0
            { 0, 0, 0 },    //gain 1
            { 0, 0, 0 },    //gain 2
            { 0, 0, 0 },    //gain 3
            { 0, 0, 0 },    //gain 4
            { 0, 0, 0 },    //gain 5
            { 0, 0, 0 },    //gain 6
            { 0, 0, 0 },    //gain 7
            { 0, 0, 0 },    //gain 8
            { 0, 0, 0 },    //gain 9
            { 0, 0, 0 },    //gain 10
            { 0, 0, 0 },    //gain 11
            { 0, 0, 0 },    //gain 12
            { 0, 0, 0 },    //gain 13
            { 0, 0, 0 },    //gain 14
            { 0, 0, 0 }     //gain 15
        },  //Tx0
        {
            { 0, 0, 0 },    //gain 0
            { 0, 0, 0 },    //gain 1
            { 0, 0, 0 },    //gain 2
            { 0, 0, 0 },    //gain 3
            { 0, 0, 0 },    //gain 4
            { 0, 0, 0 },    //gain 5
            { 0, 0, 0 },    //gain 6
            { 0, 0, 0 },    //gain 7
            { 0, 0, 0 },    //gain 8
            { 0, 0, 0 },    //gain 9
            { 0, 0, 0 },    //gain 10
            { 0, 0, 0 },    //gain 11
            { 0, 0, 0 },    //gain 12
            { 0, 0, 0 },    //gain 13
            { 0, 0, 0 },    //gain 14
            { 0, 0, 0 }     //gain 15
        }   //Tx1
    }, //channel 0
    {
        {
            { 0, 0, 0 },    //gain 0
            { 0, 0, 0 },    //gain 1
            { 0, 0, 0 },    //gain 2
            { 0, 0, 0 },    //gain 3
            { 0, 0, 0 },    //gain 4
            { 0, 0, 0 },    //gain 5
            { 0, 0, 0 },    //gain 6
            { 0, 0, 0 },    //gain 7
            { 0, 0, 0 },    //gain 8
            { 0, 0, 0 },    //gain 9
            { 0, 0, 0 },    //gain 10
            { 0, 0, 0 },    //gain 11
            { 0, 0, 0 },    //gain 12
            { 0, 0, 0 },    //gain 13
            { 0, 0, 0 },    //gain 14
            { 0, 0, 0 }     //gain 15
        },  //Tx0
        {
            { 0, 0, 0 },    //gain 0
            { 0, 0, 0 },    //gain 1
            { 0, 0, 0 },    //gain 2
            { 0, 0, 0 },    //gain 3
            { 0, 0, 0 },    //gain 4
            { 0, 0, 0 },    //gain 5
            { 0, 0, 0 },    //gain 6
            { 0, 0, 0 },    //gain 7
            { 0, 0, 0 },    //gain 8
            { 0, 0, 0 },    //gain 9
            { 0, 0, 0 },    //gain 10
            { 0, 0, 0 },    //gain 11
            { 0, 0, 0 },    //gain 12
            { 0, 0, 0 },    //gain 13
            { 0, 0, 0 },    //gain 14
            { 0, 0, 0 }     //gain 15
        }   //Tx1
    }, //channel 1
    {
        {
            { 0, 0, 0 },    //gain 0
            { 0, 0, 0 },    //gain 1
            { 0, 0, 0 },    //gain 2
            { 0, 0, 0 },    //gain 3
            { 0, 0, 0 },    //gain 4
            { 0, 0, 0 },    //gain 5
            { 0, 0, 0 },    //gain 6
            { 0, 0, 0 },    //gain 7
            { 0, 0, 0 },    //gain 8
            { 0, 0, 0 },    //gain 9
            { 0, 0, 0 },    //gain 10
            { 0, 0, 0 },    //gain 11
            { 0, 0, 0 },    //gain 12
            { 0, 0, 0 },    //gain 13
            { 0, 0, 0 },    //gain 14
            { 0, 0, 0 }     //gain 15
        },  //Tx0
        {
            { 0, 0, 0 },    //gain 0
            { 0, 0, 0 },    //gain 1
            { 0, 0, 0 },    //gain 2
            { 0, 0, 0 },    //gain 3
            { 0, 0, 0 },    //gain 4
            { 0, 0, 0 },    //gain 5
            { 0, 0, 0 },    //gain 6
            { 0, 0, 0 },    //gain 7
            { 0, 0, 0 },    //gain 8
            { 0, 0, 0 },    //gain 9
            { 0, 0, 0 },    //gain 10
            { 0, 0, 0 },    //gain 11
            { 0, 0, 0 },    //gain 12
            { 0, 0, 0 },    //gain 13
            { 0, 0, 0 },    //gain 14
            { 0, 0, 0 }     //gain 15
        }   //Tx1
    }, //channel 2
    {
        {
            { 0, 0, 0 },    //gain 0
            { 0, 0, 0 },    //gain 1
            { 0, 0, 0 },    //gain 2
            { 0, 0, 0 },    //gain 3
            { 0, 0, 0 },    //gain 4
            { 0, 0, 0 },    //gain 5
            { 0, 0, 0 },    //gain 6
            { 0, 0, 0 },    //gain 7
            { 0, 0, 0 },    //gain 8
            { 0, 0, 0 },    //gain 9
            { 0, 0, 0 },    //gain 10
            { 0, 0, 0 },    //gain 11
            { 0, 0, 0 },    //gain 12
            { 0, 0, 0 },    //gain 13
            { 0, 0, 0 },    //gain 14
            { 0, 0, 0 }     //gain 15
        },  //Tx0
        {
            { 0, 0, 0 },    //gain 0
            { 0, 0, 0 },    //gain 1
            { 0, 0, 0 },    //gain 2
            { 0, 0, 0 },    //gain 3
            { 0, 0, 0 },    //gain 4
            { 0, 0, 0 },    //gain 5
            { 0, 0, 0 },    //gain 6
            { 0, 0, 0 },    //gain 7
            { 0, 0, 0 },    //gain 8
            { 0, 0, 0 },    //gain 9
            { 0, 0, 0 },    //gain 10
            { 0, 0, 0 },    //gain 11
            { 0, 0, 0 },    //gain 12
            { 0, 0, 0 },    //gain 13
            { 0, 0, 0 },    //gain 14
            { 0, 0, 0 }     //gain 15
        }   //Tx1
    }, //channel 3
    {
        {
            { 0, 0, 0 },    //gain 0
            { 0, 0, 0 },    //gain 1
            { 0, 0, 0 },    //gain 2
            { 0, 0, 0 },    //gain 3
            { 0, 0, 0 },    //gain 4
            { 0, 0, 0 },    //gain 5
            { 0, 0, 0 },    //gain 6
            { 0, 0, 0 },    //gain 7
            { 0, 0, 0 },    //gain 8
            { 0, 0, 0 },    //gain 9
            { 0, 0, 0 },    //gain 10
            { 0, 0, 0 },    //gain 11
            { 0, 0, 0 },    //gain 12
            { 0, 0, 0 },    //gain 13
            { 0, 0, 0 },    //gain 14
            { 0, 0, 0 }     //gain 15
        },  //Tx0
        {
            { 0, 0, 0 },    //gain 0
            { 0, 0, 0 },    //gain 1
            { 0, 0, 0 },    //gain 2
            { 0, 0, 0 },    //gain 3
            { 0, 0, 0 },    //gain 4
            { 0, 0, 0 },    //gain 5
            { 0, 0, 0 },    //gain 6
            { 0, 0, 0 },    //gain 7
            { 0, 0, 0 },    //gain 8
            { 0, 0, 0 },    //gain 9
            { 0, 0, 0 },    //gain 10
            { 0, 0, 0 },    //gain 11
            { 0, 0, 0 },    //gain 12
            { 0, 0, 0 },    //gain 13
            { 0, 0, 0 },    //gain 14
            { 0, 0, 0 }     //gain 15
        }   //Tx1
    }, //channel 4
    {
        {
            { 0, 0, 0 },    //gain 0
            { 0, 0, 0 },    //gain 1
            { 0, 0, 0 },    //gain 2
            { 0, 0, 0 },    //gain 3
            { 0, 0, 0 },    //gain 4
            { 0, 0, 0 },    //gain 5
            { 0, 0, 0 },    //gain 6
            { 0, 0, 0 },    //gain 7
            { 0, 0, 0 },    //gain 8
            { 0, 0, 0 },    //gain 9
            { 0, 0, 0 },    //gain 10
            { 0, 0, 0 },    //gain 11
            { 0, 0, 0 },    //gain 12
            { 0, 0, 0 },    //gain 13
            { 0, 0, 0 },    //gain 14
            { 0, 0, 0 }     //gain 15
        },  //Tx0
        {
            { 0, 0, 0 },    //gain 0
            { 0, 0, 0 },    //gain 1
            { 0, 0, 0 },    //gain 2
            { 0, 0, 0 },    //gain 3
            { 0, 0, 0 },    //gain 4
            { 0, 0, 0 },    //gain 5
            { 0, 0, 0 },    //gain 6
            { 0, 0, 0 },    //gain 7
            { 0, 0, 0 },    //gain 8
            { 0, 0, 0 },    //gain 9
            { 0, 0, 0 },    //gain 10
            { 0, 0, 0 },    //gain 11
            { 0, 0, 0 },    //gain 12
            { 0, 0, 0 },    //gain 13
            { 0, 0, 0 },    //gain 14
            { 0, 0, 0 }     //gain 15
        }   //Tx1
    }, //channel 5
    {
        {
            { 0, 0, 0 },    //gain 0
            { 0, 0, 0 },    //gain 1
            { 0, 0, 0 },    //gain 2
            { 0, 0, 0 },    //gain 3
            { 0, 0, 0 },    //gain 4
            { 0, 0, 0 },    //gain 5
            { 0, 0, 0 },    //gain 6
            { 0, 0, 0 },    //gain 7
            { 0, 0, 0 },    //gain 8
            { 0, 0, 0 },    //gain 9
            { 0, 0, 0 },    //gain 10
            { 0, 0, 0 },    //gain 11
            { 0, 0, 0 },    //gain 12
            { 0, 0, 0 },    //gain 13
            { 0, 0, 0 },    //gain 14
            { 0, 0, 0 }     //gain 15
        },  //Tx0
        {
            { 0, 0, 0 },    //gain 0
            { 0, 0, 0 },    //gain 1
            { 0, 0, 0 },    //gain 2
            { 0, 0, 0 },    //gain 3
            { 0, 0, 0 },    //gain 4
            { 0, 0, 0 },    //gain 5
            { 0, 0, 0 },    //gain 6
            { 0, 0, 0 },    //gain 7
            { 0, 0, 0 },    //gain 8
            { 0, 0, 0 },    //gain 9
            { 0, 0, 0 },    //gain 10
            { 0, 0, 0 },    //gain 11
            { 0, 0, 0 },    //gain 12
            { 0, 0, 0 },    //gain 13
            { 0, 0, 0 },    //gain 14
            { 0, 0, 0 }     //gain 15
        }   //Tx1
    }, //channel 6
    {
        {
            { 0, 0, 0 },    //gain 0
            { 0, 0, 0 },    //gain 1
            { 0, 0, 0 },    //gain 2
            { 0, 0, 0 },    //gain 3
            { 0, 0, 0 },    //gain 4
            { 0, 0, 0 },    //gain 5
            { 0, 0, 0 },    //gain 6
            { 0, 0, 0 },    //gain 7
            { 0, 0, 0 },    //gain 8
            { 0, 0, 0 },    //gain 9
            { 0, 0, 0 },    //gain 10
            { 0, 0, 0 },    //gain 11
            { 0, 0, 0 },    //gain 12
            { 0, 0, 0 },    //gain 13
            { 0, 0, 0 },    //gain 14
            { 0, 0, 0 }     //gain 15
        },  //Tx0
        {
            { 0, 0, 0 },    //gain 0
            { 0, 0, 0 },    //gain 1
            { 0, 0, 0 },    //gain 2
            { 0, 0, 0 },    //gain 3
            { 0, 0, 0 },    //gain 4
            { 0, 0, 0 },    //gain 5
            { 0, 0, 0 },    //gain 6
            { 0, 0, 0 },    //gain 7
            { 0, 0, 0 },    //gain 8
            { 0, 0, 0 },    //gain 9
            { 0, 0, 0 },    //gain 10
            { 0, 0, 0 },    //gain 11
            { 0, 0, 0 },    //gain 12
            { 0, 0, 0 },    //gain 13
            { 0, 0, 0 },    //gain 14
            { 0, 0, 0 }     //gain 15
        }   //Tx1
    }, //channel 7
    {
        {
            { 0, 0, 0 },    //gain 0
            { 0, 0, 0 },    //gain 1
            { 0, 0, 0 },    //gain 2
            { 0, 0, 0 },    //gain 3
            { 0, 0, 0 },    //gain 4
            { 0, 0, 0 },    //gain 5
            { 0, 0, 0 },    //gain 6
            { 0, 0, 0 },    //gain 7
            { 0, 0, 0 },    //gain 8
            { 0, 0, 0 },    //gain 9
            { 0, 0, 0 },    //gain 10
            { 0, 0, 0 },    //gain 11
            { 0, 0, 0 },    //gain 12
            { 0, 0, 0 },    //gain 13
            { 0, 0, 0 },    //gain 14
            { 0, 0, 0 }     //gain 15
        },  //Tx0
        {
            { 0, 0, 0 },    //gain 0
            { 0, 0, 0 },    //gain 1
            { 0, 0, 0 },    //gain 2
            { 0, 0, 0 },    //gain 3
            { 0, 0, 0 },    //gain 4
            { 0, 0, 0 },    //gain 5
            { 0, 0, 0 },    //gain 6
            { 0, 0, 0 },    //gain 7
            { 0, 0, 0 },    //gain 8
            { 0, 0, 0 },    //gain 9
            { 0, 0, 0 },    //gain 10
            { 0, 0, 0 },    //gain 11
            { 0, 0, 0 },    //gain 12
            { 0, 0, 0 },    //gain 13
            { 0, 0, 0 },    //gain 14
            { 0, 0, 0 }     //gain 15
        }   //Tx1
    }, //channel 8
    {
        {
            { 0, 0, 0 },    //gain 0
            { 0, 0, 0 },    //gain 1
            { 0, 0, 0 },    //gain 2
            { 0, 0, 0 },    //gain 3
            { 0, 0, 0 },    //gain 4
            { 0, 0, 0 },    //gain 5
            { 0, 0, 0 },    //gain 6
            { 0, 0, 0 },    //gain 7
            { 0, 0, 0 },    //gain 8
            { 0, 0, 0 },    //gain 9
            { 0, 0, 0 },    //gain 10
            { 0, 0, 0 },    //gain 11
            { 0, 0, 0 },    //gain 12
            { 0, 0, 0 },    //gain 13
            { 0, 0, 0 },    //gain 14
            { 0, 0, 0 }     //gain 15
        },  //Tx0
        {
            { 0, 0, 0 },    //gain 0
            { 0, 0, 0 },    //gain 1
            { 0, 0, 0 },    //gain 2
            { 0, 0, 0 },    //gain 3
            { 0, 0, 0 },    //gain 4
            { 0, 0, 0 },    //gain 5
            { 0, 0, 0 },    //gain 6
            { 0, 0, 0 },    //gain 7
            { 0, 0, 0 },    //gain 8
            { 0, 0, 0 },    //gain 9
            { 0, 0, 0 },    //gain 10
            { 0, 0, 0 },    //gain 11
            { 0, 0, 0 },    //gain 12
            { 0, 0, 0 },    //gain 13
            { 0, 0, 0 },    //gain 14
            { 0, 0, 0 }     //gain 15
        }   //Tx1
    }, //channel 9
    {
        {
            { 0, 0, 0 },    //gain 0
            { 0, 0, 0 },    //gain 1
            { 0, 0, 0 },    //gain 2
            { 0, 0, 0 },    //gain 3
            { 0, 0, 0 },    //gain 4
            { 0, 0, 0 },    //gain 5
            { 0, 0, 0 },    //gain 6
            { 0, 0, 0 },    //gain 7
            { 0, 0, 0 },    //gain 8
            { 0, 0, 0 },    //gain 9
            { 0, 0, 0 },    //gain 10
            { 0, 0, 0 },    //gain 11
            { 0, 0, 0 },    //gain 12
            { 0, 0, 0 },    //gain 13
            { 0, 0, 0 },    //gain 14
            { 0, 0, 0 }     //gain 15
        },  //Tx0
        {
            { 0, 0, 0 },    //gain 0
            { 0, 0, 0 },    //gain 1
            { 0, 0, 0 },    //gain 2
            { 0, 0, 0 },    //gain 3
            { 0, 0, 0 },    //gain 4
            { 0, 0, 0 },    //gain 5
            { 0, 0, 0 },    //gain 6
            { 0, 0, 0 },    //gain 7
            { 0, 0, 0 },    //gain 8
            { 0, 0, 0 },    //gain 9
            { 0, 0, 0 },    //gain 10
            { 0, 0, 0 },    //gain 11
            { 0, 0, 0 },    //gain 12
            { 0, 0, 0 },    //gain 13
            { 0, 0, 0 },    //gain 14
            { 0, 0, 0 }     //gain 15
        }   //Tx1
    }, //channel 10
    {
        {
            { 0, 0, 0 },    //gain 0
            { 0, 0, 0 },    //gain 1
            { 0, 0, 0 },    //gain 2
            { 0, 0, 0 },    //gain 3
            { 0, 0, 0 },    //gain 4
            { 0, 0, 0 },    //gain 5
            { 0, 0, 0 },    //gain 6
            { 0, 0, 0 },    //gain 7
            { 0, 0, 0 },    //gain 8
            { 0, 0, 0 },    //gain 9
            { 0, 0, 0 },    //gain 10
            { 0, 0, 0 },    //gain 11
            { 0, 0, 0 },    //gain 12
            { 0, 0, 0 },    //gain 13
            { 0, 0, 0 },    //gain 14
            { 0, 0, 0 }     //gain 15
        },  //Tx0
        {
            { 0, 0, 0 },    //gain 0
            { 0, 0, 0 },    //gain 1
            { 0, 0, 0 },    //gain 2
            { 0, 0, 0 },    //gain 3
            { 0, 0, 0 },    //gain 4
            { 0, 0, 0 },    //gain 5
            { 0, 0, 0 },    //gain 6
            { 0, 0, 0 },    //gain 7
            { 0, 0, 0 },    //gain 8
            { 0, 0, 0 },    //gain 9
            { 0, 0, 0 },    //gain 10
            { 0, 0, 0 },    //gain 11
            { 0, 0, 0 },    //gain 12
            { 0, 0, 0 },    //gain 13
            { 0, 0, 0 },    //gain 14
            { 0, 0, 0 }     //gain 15
        }   //Tx1
    }  //channel 11
};

const sRxIQChannel        rxIQTable[NUM_DEMO_CAL_CHANNELS] =
// EEPROM_TABLE_RX_IQ
{
    // typedef struct
    // {
    //     tANI_S9 center;
    //     tANI_S9 offCenter;
    //     tANI_S9 imbalance;
    // }sIQCalValues;

    //typedef sIQCalValues sRxIQChannel[PHY_MAX_RX_CHAINS][NUM_RX_GAIN_STEPS];
    {
        {
            { 0, 0, 0 },    //gain 0
            { 0, 0, 0 },    //gain 1
            { 0, 0, 0 },    //gain 2
            { 0, 0, 0 },    //gain 3
            { 0, 0, 0 },    //gain 4
            { 0, 0, 0 },    //gain 5
            { 0, 0, 0 },    //gain 6
            { 0, 0, 0 },    //gain 7
            { 0, 0, 0 },    //gain 8
            { 0, 0, 0 },    //gain 9
            { 0, 0, 0 },    //gain 10
            { 0, 0, 0 },    //gain 11
            { 0, 0, 0 },    //gain 12
            { 0, 0, 0 },    //gain 13
            { 0, 0, 0 },    //gain 14
            { 0, 0, 0 }     //gain 15
        },  //Rx0
        {
            { 0, 0, 0 },    //gain 0
            { 0, 0, 0 },    //gain 1
            { 0, 0, 0 },    //gain 2
            { 0, 0, 0 },    //gain 3
            { 0, 0, 0 },    //gain 4
            { 0, 0, 0 },    //gain 5
            { 0, 0, 0 },    //gain 6
            { 0, 0, 0 },    //gain 7
            { 0, 0, 0 },    //gain 8
            { 0, 0, 0 },    //gain 9
            { 0, 0, 0 },    //gain 10
            { 0, 0, 0 },    //gain 11
            { 0, 0, 0 },    //gain 12
            { 0, 0, 0 },    //gain 13
            { 0, 0, 0 },    //gain 14
            { 0, 0, 0 }     //gain 15
        },  //Rx1
        {
            { 0, 0, 0 },    //gain 0
            { 0, 0, 0 },    //gain 1
            { 0, 0, 0 },    //gain 2
            { 0, 0, 0 },    //gain 3
            { 0, 0, 0 },    //gain 4
            { 0, 0, 0 },    //gain 5
            { 0, 0, 0 },    //gain 6
            { 0, 0, 0 },    //gain 7
            { 0, 0, 0 },    //gain 8
            { 0, 0, 0 },    //gain 9
            { 0, 0, 0 },    //gain 10
            { 0, 0, 0 },    //gain 11
            { 0, 0, 0 },    //gain 12
            { 0, 0, 0 },    //gain 13
            { 0, 0, 0 },    //gain 14
            { 0, 0, 0 }     //gain 15
        }   //Rx2
    }, //channel 0
    {
        {
            { 0, 0, 0 },    //gain 0
            { 0, 0, 0 },    //gain 1
            { 0, 0, 0 },    //gain 2
            { 0, 0, 0 },    //gain 3
            { 0, 0, 0 },    //gain 4
            { 0, 0, 0 },    //gain 5
            { 0, 0, 0 },    //gain 6
            { 0, 0, 0 },    //gain 7
            { 0, 0, 0 },    //gain 8
            { 0, 0, 0 },    //gain 9
            { 0, 0, 0 },    //gain 10
            { 0, 0, 0 },    //gain 11
            { 0, 0, 0 },    //gain 12
            { 0, 0, 0 },    //gain 13
            { 0, 0, 0 },    //gain 14
            { 0, 0, 0 }     //gain 15
        },  //Rx0
        {
            { 0, 0, 0 },    //gain 0
            { 0, 0, 0 },    //gain 1
            { 0, 0, 0 },    //gain 2
            { 0, 0, 0 },    //gain 3
            { 0, 0, 0 },    //gain 4
            { 0, 0, 0 },    //gain 5
            { 0, 0, 0 },    //gain 6
            { 0, 0, 0 },    //gain 7
            { 0, 0, 0 },    //gain 8
            { 0, 0, 0 },    //gain 9
            { 0, 0, 0 },    //gain 10
            { 0, 0, 0 },    //gain 11
            { 0, 0, 0 },    //gain 12
            { 0, 0, 0 },    //gain 13
            { 0, 0, 0 },    //gain 14
            { 0, 0, 0 }     //gain 15
        },  //Rx1
        {
            { 0, 0, 0 },    //gain 0
            { 0, 0, 0 },    //gain 1
            { 0, 0, 0 },    //gain 2
            { 0, 0, 0 },    //gain 3
            { 0, 0, 0 },    //gain 4
            { 0, 0, 0 },    //gain 5
            { 0, 0, 0 },    //gain 6
            { 0, 0, 0 },    //gain 7
            { 0, 0, 0 },    //gain 8
            { 0, 0, 0 },    //gain 9
            { 0, 0, 0 },    //gain 10
            { 0, 0, 0 },    //gain 11
            { 0, 0, 0 },    //gain 12
            { 0, 0, 0 },    //gain 13
            { 0, 0, 0 },    //gain 14
            { 0, 0, 0 }     //gain 15
        }   //Rx2
    }, //channel 1
    {
        {
            { 0, 0, 0 },    //gain 0
            { 0, 0, 0 },    //gain 1
            { 0, 0, 0 },    //gain 2
            { 0, 0, 0 },    //gain 3
            { 0, 0, 0 },    //gain 4
            { 0, 0, 0 },    //gain 5
            { 0, 0, 0 },    //gain 6
            { 0, 0, 0 },    //gain 7
            { 0, 0, 0 },    //gain 8
            { 0, 0, 0 },    //gain 9
            { 0, 0, 0 },    //gain 10
            { 0, 0, 0 },    //gain 11
            { 0, 0, 0 },    //gain 12
            { 0, 0, 0 },    //gain 13
            { 0, 0, 0 },    //gain 14
            { 0, 0, 0 }     //gain 15
        },  //Rx0
        {
            { 0, 0, 0 },    //gain 0
            { 0, 0, 0 },    //gain 1
            { 0, 0, 0 },    //gain 2
            { 0, 0, 0 },    //gain 3
            { 0, 0, 0 },    //gain 4
            { 0, 0, 0 },    //gain 5
            { 0, 0, 0 },    //gain 6
            { 0, 0, 0 },    //gain 7
            { 0, 0, 0 },    //gain 8
            { 0, 0, 0 },    //gain 9
            { 0, 0, 0 },    //gain 10
            { 0, 0, 0 },    //gain 11
            { 0, 0, 0 },    //gain 12
            { 0, 0, 0 },    //gain 13
            { 0, 0, 0 },    //gain 14
            { 0, 0, 0 }     //gain 15
        },  //Rx1
        {
            { 0, 0, 0 },    //gain 0
            { 0, 0, 0 },    //gain 1
            { 0, 0, 0 },    //gain 2
            { 0, 0, 0 },    //gain 3
            { 0, 0, 0 },    //gain 4
            { 0, 0, 0 },    //gain 5
            { 0, 0, 0 },    //gain 6
            { 0, 0, 0 },    //gain 7
            { 0, 0, 0 },    //gain 8
            { 0, 0, 0 },    //gain 9
            { 0, 0, 0 },    //gain 10
            { 0, 0, 0 },    //gain 11
            { 0, 0, 0 },    //gain 12
            { 0, 0, 0 },    //gain 13
            { 0, 0, 0 },    //gain 14
            { 0, 0, 0 }     //gain 15
        }   //Rx2
    }, //channel 2
    {
        {
            { 0, 0, 0 },    //gain 0
            { 0, 0, 0 },    //gain 1
            { 0, 0, 0 },    //gain 2
            { 0, 0, 0 },    //gain 3
            { 0, 0, 0 },    //gain 4
            { 0, 0, 0 },    //gain 5
            { 0, 0, 0 },    //gain 6
            { 0, 0, 0 },    //gain 7
            { 0, 0, 0 },    //gain 8
            { 0, 0, 0 },    //gain 9
            { 0, 0, 0 },    //gain 10
            { 0, 0, 0 },    //gain 11
            { 0, 0, 0 },    //gain 12
            { 0, 0, 0 },    //gain 13
            { 0, 0, 0 },    //gain 14
            { 0, 0, 0 }     //gain 15
        },  //Rx0
        {
            { 0, 0, 0 },    //gain 0
            { 0, 0, 0 },    //gain 1
            { 0, 0, 0 },    //gain 2
            { 0, 0, 0 },    //gain 3
            { 0, 0, 0 },    //gain 4
            { 0, 0, 0 },    //gain 5
            { 0, 0, 0 },    //gain 6
            { 0, 0, 0 },    //gain 7
            { 0, 0, 0 },    //gain 8
            { 0, 0, 0 },    //gain 9
            { 0, 0, 0 },    //gain 10
            { 0, 0, 0 },    //gain 11
            { 0, 0, 0 },    //gain 12
            { 0, 0, 0 },    //gain 13
            { 0, 0, 0 },    //gain 14
            { 0, 0, 0 }     //gain 15
        },  //Rx1
        {
            { 0, 0, 0 },    //gain 0
            { 0, 0, 0 },    //gain 1
            { 0, 0, 0 },    //gain 2
            { 0, 0, 0 },    //gain 3
            { 0, 0, 0 },    //gain 4
            { 0, 0, 0 },    //gain 5
            { 0, 0, 0 },    //gain 6
            { 0, 0, 0 },    //gain 7
            { 0, 0, 0 },    //gain 8
            { 0, 0, 0 },    //gain 9
            { 0, 0, 0 },    //gain 10
            { 0, 0, 0 },    //gain 11
            { 0, 0, 0 },    //gain 12
            { 0, 0, 0 },    //gain 13
            { 0, 0, 0 },    //gain 14
            { 0, 0, 0 }     //gain 15
        }   //Rx2
    }, //channel 3
    {
        {
            { 0, 0, 0 },    //gain 0
            { 0, 0, 0 },    //gain 1
            { 0, 0, 0 },    //gain 2
            { 0, 0, 0 },    //gain 3
            { 0, 0, 0 },    //gain 4
            { 0, 0, 0 },    //gain 5
            { 0, 0, 0 },    //gain 6
            { 0, 0, 0 },    //gain 7
            { 0, 0, 0 },    //gain 8
            { 0, 0, 0 },    //gain 9
            { 0, 0, 0 },    //gain 10
            { 0, 0, 0 },    //gain 11
            { 0, 0, 0 },    //gain 12
            { 0, 0, 0 },    //gain 13
            { 0, 0, 0 },    //gain 14
            { 0, 0, 0 }     //gain 15
        },  //Rx0
        {
            { 0, 0, 0 },    //gain 0
            { 0, 0, 0 },    //gain 1
            { 0, 0, 0 },    //gain 2
            { 0, 0, 0 },    //gain 3
            { 0, 0, 0 },    //gain 4
            { 0, 0, 0 },    //gain 5
            { 0, 0, 0 },    //gain 6
            { 0, 0, 0 },    //gain 7
            { 0, 0, 0 },    //gain 8
            { 0, 0, 0 },    //gain 9
            { 0, 0, 0 },    //gain 10
            { 0, 0, 0 },    //gain 11
            { 0, 0, 0 },    //gain 12
            { 0, 0, 0 },    //gain 13
            { 0, 0, 0 },    //gain 14
            { 0, 0, 0 }     //gain 15
        },  //Rx1
        {
            { 0, 0, 0 },    //gain 0
            { 0, 0, 0 },    //gain 1
            { 0, 0, 0 },    //gain 2
            { 0, 0, 0 },    //gain 3
            { 0, 0, 0 },    //gain 4
            { 0, 0, 0 },    //gain 5
            { 0, 0, 0 },    //gain 6
            { 0, 0, 0 },    //gain 7
            { 0, 0, 0 },    //gain 8
            { 0, 0, 0 },    //gain 9
            { 0, 0, 0 },    //gain 10
            { 0, 0, 0 },    //gain 11
            { 0, 0, 0 },    //gain 12
            { 0, 0, 0 },    //gain 13
            { 0, 0, 0 },    //gain 14
            { 0, 0, 0 }     //gain 15
        }   //Rx2
    }, //channel 4
    {
        {
            { 0, 0, 0 },    //gain 0
            { 0, 0, 0 },    //gain 1
            { 0, 0, 0 },    //gain 2
            { 0, 0, 0 },    //gain 3
            { 0, 0, 0 },    //gain 4
            { 0, 0, 0 },    //gain 5
            { 0, 0, 0 },    //gain 6
            { 0, 0, 0 },    //gain 7
            { 0, 0, 0 },    //gain 8
            { 0, 0, 0 },    //gain 9
            { 0, 0, 0 },    //gain 10
            { 0, 0, 0 },    //gain 11
            { 0, 0, 0 },    //gain 12
            { 0, 0, 0 },    //gain 13
            { 0, 0, 0 },    //gain 14
            { 0, 0, 0 }     //gain 15
        },  //Rx0
        {
            { 0, 0, 0 },    //gain 0
            { 0, 0, 0 },    //gain 1
            { 0, 0, 0 },    //gain 2
            { 0, 0, 0 },    //gain 3
            { 0, 0, 0 },    //gain 4
            { 0, 0, 0 },    //gain 5
            { 0, 0, 0 },    //gain 6
            { 0, 0, 0 },    //gain 7
            { 0, 0, 0 },    //gain 8
            { 0, 0, 0 },    //gain 9
            { 0, 0, 0 },    //gain 10
            { 0, 0, 0 },    //gain 11
            { 0, 0, 0 },    //gain 12
            { 0, 0, 0 },    //gain 13
            { 0, 0, 0 },    //gain 14
            { 0, 0, 0 }     //gain 15
        },  //Rx1
        {
            { 0, 0, 0 },    //gain 0
            { 0, 0, 0 },    //gain 1
            { 0, 0, 0 },    //gain 2
            { 0, 0, 0 },    //gain 3
            { 0, 0, 0 },    //gain 4
            { 0, 0, 0 },    //gain 5
            { 0, 0, 0 },    //gain 6
            { 0, 0, 0 },    //gain 7
            { 0, 0, 0 },    //gain 8
            { 0, 0, 0 },    //gain 9
            { 0, 0, 0 },    //gain 10
            { 0, 0, 0 },    //gain 11
            { 0, 0, 0 },    //gain 12
            { 0, 0, 0 },    //gain 13
            { 0, 0, 0 },    //gain 14
            { 0, 0, 0 }     //gain 15
        }   //Rx2
    }, //channel 5
    {
        {
            { 0, 0, 0 },    //gain 0
            { 0, 0, 0 },    //gain 1
            { 0, 0, 0 },    //gain 2
            { 0, 0, 0 },    //gain 3
            { 0, 0, 0 },    //gain 4
            { 0, 0, 0 },    //gain 5
            { 0, 0, 0 },    //gain 6
            { 0, 0, 0 },    //gain 7
            { 0, 0, 0 },    //gain 8
            { 0, 0, 0 },    //gain 9
            { 0, 0, 0 },    //gain 10
            { 0, 0, 0 },    //gain 11
            { 0, 0, 0 },    //gain 12
            { 0, 0, 0 },    //gain 13
            { 0, 0, 0 },    //gain 14
            { 0, 0, 0 }     //gain 15
        },  //Rx0
        {
            { 0, 0, 0 },    //gain 0
            { 0, 0, 0 },    //gain 1
            { 0, 0, 0 },    //gain 2
            { 0, 0, 0 },    //gain 3
            { 0, 0, 0 },    //gain 4
            { 0, 0, 0 },    //gain 5
            { 0, 0, 0 },    //gain 6
            { 0, 0, 0 },    //gain 7
            { 0, 0, 0 },    //gain 8
            { 0, 0, 0 },    //gain 9
            { 0, 0, 0 },    //gain 10
            { 0, 0, 0 },    //gain 11
            { 0, 0, 0 },    //gain 12
            { 0, 0, 0 },    //gain 13
            { 0, 0, 0 },    //gain 14
            { 0, 0, 0 }     //gain 15
        },  //Rx1
        {
            { 0, 0, 0 },    //gain 0
            { 0, 0, 0 },    //gain 1
            { 0, 0, 0 },    //gain 2
            { 0, 0, 0 },    //gain 3
            { 0, 0, 0 },    //gain 4
            { 0, 0, 0 },    //gain 5
            { 0, 0, 0 },    //gain 6
            { 0, 0, 0 },    //gain 7
            { 0, 0, 0 },    //gain 8
            { 0, 0, 0 },    //gain 9
            { 0, 0, 0 },    //gain 10
            { 0, 0, 0 },    //gain 11
            { 0, 0, 0 },    //gain 12
            { 0, 0, 0 },    //gain 13
            { 0, 0, 0 },    //gain 14
            { 0, 0, 0 }     //gain 15
        }   //Rx2
    }, //channel 6
    {
        {
            { 0, 0, 0 },    //gain 0
            { 0, 0, 0 },    //gain 1
            { 0, 0, 0 },    //gain 2
            { 0, 0, 0 },    //gain 3
            { 0, 0, 0 },    //gain 4
            { 0, 0, 0 },    //gain 5
            { 0, 0, 0 },    //gain 6
            { 0, 0, 0 },    //gain 7
            { 0, 0, 0 },    //gain 8
            { 0, 0, 0 },    //gain 9
            { 0, 0, 0 },    //gain 10
            { 0, 0, 0 },    //gain 11
            { 0, 0, 0 },    //gain 12
            { 0, 0, 0 },    //gain 13
            { 0, 0, 0 },    //gain 14
            { 0, 0, 0 }     //gain 15
        },  //Rx0
        {
            { 0, 0, 0 },    //gain 0
            { 0, 0, 0 },    //gain 1
            { 0, 0, 0 },    //gain 2
            { 0, 0, 0 },    //gain 3
            { 0, 0, 0 },    //gain 4
            { 0, 0, 0 },    //gain 5
            { 0, 0, 0 },    //gain 6
            { 0, 0, 0 },    //gain 7
            { 0, 0, 0 },    //gain 8
            { 0, 0, 0 },    //gain 9
            { 0, 0, 0 },    //gain 10
            { 0, 0, 0 },    //gain 11
            { 0, 0, 0 },    //gain 12
            { 0, 0, 0 },    //gain 13
            { 0, 0, 0 },    //gain 14
            { 0, 0, 0 }     //gain 15
        },  //Rx1
        {
            { 0, 0, 0 },    //gain 0
            { 0, 0, 0 },    //gain 1
            { 0, 0, 0 },    //gain 2
            { 0, 0, 0 },    //gain 3
            { 0, 0, 0 },    //gain 4
            { 0, 0, 0 },    //gain 5
            { 0, 0, 0 },    //gain 6
            { 0, 0, 0 },    //gain 7
            { 0, 0, 0 },    //gain 8
            { 0, 0, 0 },    //gain 9
            { 0, 0, 0 },    //gain 10
            { 0, 0, 0 },    //gain 11
            { 0, 0, 0 },    //gain 12
            { 0, 0, 0 },    //gain 13
            { 0, 0, 0 },    //gain 14
            { 0, 0, 0 }     //gain 15
        }   //Rx2
    }, //channel 7
    {
        {
            { 0, 0, 0 },    //gain 0
            { 0, 0, 0 },    //gain 1
            { 0, 0, 0 },    //gain 2
            { 0, 0, 0 },    //gain 3
            { 0, 0, 0 },    //gain 4
            { 0, 0, 0 },    //gain 5
            { 0, 0, 0 },    //gain 6
            { 0, 0, 0 },    //gain 7
            { 0, 0, 0 },    //gain 8
            { 0, 0, 0 },    //gain 9
            { 0, 0, 0 },    //gain 10
            { 0, 0, 0 },    //gain 11
            { 0, 0, 0 },    //gain 12
            { 0, 0, 0 },    //gain 13
            { 0, 0, 0 },    //gain 14
            { 0, 0, 0 }     //gain 15
        },  //Rx0
        {
            { 0, 0, 0 },    //gain 0
            { 0, 0, 0 },    //gain 1
            { 0, 0, 0 },    //gain 2
            { 0, 0, 0 },    //gain 3
            { 0, 0, 0 },    //gain 4
            { 0, 0, 0 },    //gain 5
            { 0, 0, 0 },    //gain 6
            { 0, 0, 0 },    //gain 7
            { 0, 0, 0 },    //gain 8
            { 0, 0, 0 },    //gain 9
            { 0, 0, 0 },    //gain 10
            { 0, 0, 0 },    //gain 11
            { 0, 0, 0 },    //gain 12
            { 0, 0, 0 },    //gain 13
            { 0, 0, 0 },    //gain 14
            { 0, 0, 0 }     //gain 15
        },  //Rx1
        {
            { 0, 0, 0 },    //gain 0
            { 0, 0, 0 },    //gain 1
            { 0, 0, 0 },    //gain 2
            { 0, 0, 0 },    //gain 3
            { 0, 0, 0 },    //gain 4
            { 0, 0, 0 },    //gain 5
            { 0, 0, 0 },    //gain 6
            { 0, 0, 0 },    //gain 7
            { 0, 0, 0 },    //gain 8
            { 0, 0, 0 },    //gain 9
            { 0, 0, 0 },    //gain 10
            { 0, 0, 0 },    //gain 11
            { 0, 0, 0 },    //gain 12
            { 0, 0, 0 },    //gain 13
            { 0, 0, 0 },    //gain 14
            { 0, 0, 0 }     //gain 15
        }   //Rx2
    }, //channel 8
    {
        {
            { 0, 0, 0 },    //gain 0
            { 0, 0, 0 },    //gain 1
            { 0, 0, 0 },    //gain 2
            { 0, 0, 0 },    //gain 3
            { 0, 0, 0 },    //gain 4
            { 0, 0, 0 },    //gain 5
            { 0, 0, 0 },    //gain 6
            { 0, 0, 0 },    //gain 7
            { 0, 0, 0 },    //gain 8
            { 0, 0, 0 },    //gain 9
            { 0, 0, 0 },    //gain 10
            { 0, 0, 0 },    //gain 11
            { 0, 0, 0 },    //gain 12
            { 0, 0, 0 },    //gain 13
            { 0, 0, 0 },    //gain 14
            { 0, 0, 0 }     //gain 15
        },  //Rx0
        {
            { 0, 0, 0 },    //gain 0
            { 0, 0, 0 },    //gain 1
            { 0, 0, 0 },    //gain 2
            { 0, 0, 0 },    //gain 3
            { 0, 0, 0 },    //gain 4
            { 0, 0, 0 },    //gain 5
            { 0, 0, 0 },    //gain 6
            { 0, 0, 0 },    //gain 7
            { 0, 0, 0 },    //gain 8
            { 0, 0, 0 },    //gain 9
            { 0, 0, 0 },    //gain 10
            { 0, 0, 0 },    //gain 11
            { 0, 0, 0 },    //gain 12
            { 0, 0, 0 },    //gain 13
            { 0, 0, 0 },    //gain 14
            { 0, 0, 0 }     //gain 15
        },  //Rx1
        {
            { 0, 0, 0 },    //gain 0
            { 0, 0, 0 },    //gain 1
            { 0, 0, 0 },    //gain 2
            { 0, 0, 0 },    //gain 3
            { 0, 0, 0 },    //gain 4
            { 0, 0, 0 },    //gain 5
            { 0, 0, 0 },    //gain 6
            { 0, 0, 0 },    //gain 7
            { 0, 0, 0 },    //gain 8
            { 0, 0, 0 },    //gain 9
            { 0, 0, 0 },    //gain 10
            { 0, 0, 0 },    //gain 11
            { 0, 0, 0 },    //gain 12
            { 0, 0, 0 },    //gain 13
            { 0, 0, 0 },    //gain 14
            { 0, 0, 0 }     //gain 15
        }   //Rx2
    }, //channel 9
    {
        {
            { 0, 0, 0 },    //gain 0
            { 0, 0, 0 },    //gain 1
            { 0, 0, 0 },    //gain 2
            { 0, 0, 0 },    //gain 3
            { 0, 0, 0 },    //gain 4
            { 0, 0, 0 },    //gain 5
            { 0, 0, 0 },    //gain 6
            { 0, 0, 0 },    //gain 7
            { 0, 0, 0 },    //gain 8
            { 0, 0, 0 },    //gain 9
            { 0, 0, 0 },    //gain 10
            { 0, 0, 0 },    //gain 11
            { 0, 0, 0 },    //gain 12
            { 0, 0, 0 },    //gain 13
            { 0, 0, 0 },    //gain 14
            { 0, 0, 0 }     //gain 15
        },  //Rx0
        {
            { 0, 0, 0 },    //gain 0
            { 0, 0, 0 },    //gain 1
            { 0, 0, 0 },    //gain 2
            { 0, 0, 0 },    //gain 3
            { 0, 0, 0 },    //gain 4
            { 0, 0, 0 },    //gain 5
            { 0, 0, 0 },    //gain 6
            { 0, 0, 0 },    //gain 7
            { 0, 0, 0 },    //gain 8
            { 0, 0, 0 },    //gain 9
            { 0, 0, 0 },    //gain 10
            { 0, 0, 0 },    //gain 11
            { 0, 0, 0 },    //gain 12
            { 0, 0, 0 },    //gain 13
            { 0, 0, 0 },    //gain 14
            { 0, 0, 0 }     //gain 15
        },  //Rx1
        {
            { 0, 0, 0 },    //gain 0
            { 0, 0, 0 },    //gain 1
            { 0, 0, 0 },    //gain 2
            { 0, 0, 0 },    //gain 3
            { 0, 0, 0 },    //gain 4
            { 0, 0, 0 },    //gain 5
            { 0, 0, 0 },    //gain 6
            { 0, 0, 0 },    //gain 7
            { 0, 0, 0 },    //gain 8
            { 0, 0, 0 },    //gain 9
            { 0, 0, 0 },    //gain 10
            { 0, 0, 0 },    //gain 11
            { 0, 0, 0 },    //gain 12
            { 0, 0, 0 },    //gain 13
            { 0, 0, 0 },    //gain 14
            { 0, 0, 0 }     //gain 15
        }   //Rx2
    }, //channel 10
    {
        {
            { 0, 0, 0 },    //gain 0
            { 0, 0, 0 },    //gain 1
            { 0, 0, 0 },    //gain 2
            { 0, 0, 0 },    //gain 3
            { 0, 0, 0 },    //gain 4
            { 0, 0, 0 },    //gain 5
            { 0, 0, 0 },    //gain 6
            { 0, 0, 0 },    //gain 7
            { 0, 0, 0 },    //gain 8
            { 0, 0, 0 },    //gain 9
            { 0, 0, 0 },    //gain 10
            { 0, 0, 0 },    //gain 11
            { 0, 0, 0 },    //gain 12
            { 0, 0, 0 },    //gain 13
            { 0, 0, 0 },    //gain 14
            { 0, 0, 0 }     //gain 15
        },  //Rx0
        {
            { 0, 0, 0 },    //gain 0
            { 0, 0, 0 },    //gain 1
            { 0, 0, 0 },    //gain 2
            { 0, 0, 0 },    //gain 3
            { 0, 0, 0 },    //gain 4
            { 0, 0, 0 },    //gain 5
            { 0, 0, 0 },    //gain 6
            { 0, 0, 0 },    //gain 7
            { 0, 0, 0 },    //gain 8
            { 0, 0, 0 },    //gain 9
            { 0, 0, 0 },    //gain 10
            { 0, 0, 0 },    //gain 11
            { 0, 0, 0 },    //gain 12
            { 0, 0, 0 },    //gain 13
            { 0, 0, 0 },    //gain 14
            { 0, 0, 0 }     //gain 15
        },  //Rx1
        {
            { 0, 0, 0 },    //gain 0
            { 0, 0, 0 },    //gain 1
            { 0, 0, 0 },    //gain 2
            { 0, 0, 0 },    //gain 3
            { 0, 0, 0 },    //gain 4
            { 0, 0, 0 },    //gain 5
            { 0, 0, 0 },    //gain 6
            { 0, 0, 0 },    //gain 7
            { 0, 0, 0 },    //gain 8
            { 0, 0, 0 },    //gain 9
            { 0, 0, 0 },    //gain 10
            { 0, 0, 0 },    //gain 11
            { 0, 0, 0 },    //gain 12
            { 0, 0, 0 },    //gain 13
            { 0, 0, 0 },    //gain 14
            { 0, 0, 0 }     //gain 15
        }   //Rx2
    } //channel 11
};

const sLnaSwGainTable   lnaSwGainTable =
// EEPROM_TABLE_LNA_SW_GAIN
{
    // typedef struct
    // {
    //     tANI_S8 rxLnaOnGain;   // always R + LNA On
    //     tANI_S8 AlternateGain;  //either T + LNA On or R + LNA OFF, dependent on LNA Switchable parameter from mfg params in EEPROM.
    //                             //if LNA is switchable, then use R + LNA Off by default
    // }sLnaSwGain;

    // typedef struct
    // {
    //     tANI_U16 freq;
    //     sLnaSwGain swGains[PHY_MAX_RX_CHAINS];
    // }sLnaSwGainChannel;

    // #define MAX_LNA_SW_FREQS    16
    // typedef struct
    // {
    //     tANI_U8 nFreqs;
    //     tANI_U8 reserved[3];
    //     sLnaSwGainChannel chan[MAX_LNA_SW_FREQS];
    // }sLnaSwGainTable;

    11, { 0 },
                {
                    {   2412, { { QUASAR_MAX_GAIN_DB, QUASAR_MAX_GAIN_DB - 20 }, { QUASAR_MAX_GAIN_DB, QUASAR_MAX_GAIN_DB - 20 }, { QUASAR_MAX_GAIN_DB, QUASAR_MAX_GAIN_DB - 20 } }  },
                    {   2452, { { QUASAR_MAX_GAIN_DB, QUASAR_MAX_GAIN_DB - 20 }, { QUASAR_MAX_GAIN_DB, QUASAR_MAX_GAIN_DB - 20 }, { QUASAR_MAX_GAIN_DB, QUASAR_MAX_GAIN_DB - 20 } }  },
                    {   2484, { { QUASAR_MAX_GAIN_DB, QUASAR_MAX_GAIN_DB - 20 }, { QUASAR_MAX_GAIN_DB, QUASAR_MAX_GAIN_DB - 20 }, { QUASAR_MAX_GAIN_DB, QUASAR_MAX_GAIN_DB - 20 } }  },
                    {   4920, { { QUASAR_MAX_GAIN_DB, QUASAR_MAX_GAIN_DB - 20 }, { QUASAR_MAX_GAIN_DB, QUASAR_MAX_GAIN_DB - 20 }, { QUASAR_MAX_GAIN_DB, QUASAR_MAX_GAIN_DB - 20 } }  },
                    {   5080, { { QUASAR_MAX_GAIN_DB, QUASAR_MAX_GAIN_DB - 20 }, { QUASAR_MAX_GAIN_DB, QUASAR_MAX_GAIN_DB - 20 }, { QUASAR_MAX_GAIN_DB, QUASAR_MAX_GAIN_DB - 20 } }  },
                    {   5180, { { QUASAR_MAX_GAIN_DB, QUASAR_MAX_GAIN_DB - 20 }, { QUASAR_MAX_GAIN_DB, QUASAR_MAX_GAIN_DB - 20 }, { QUASAR_MAX_GAIN_DB, QUASAR_MAX_GAIN_DB - 20 } }  },
                    {   5320, { { QUASAR_MAX_GAIN_DB, QUASAR_MAX_GAIN_DB - 20 }, { QUASAR_MAX_GAIN_DB, QUASAR_MAX_GAIN_DB - 20 }, { QUASAR_MAX_GAIN_DB, QUASAR_MAX_GAIN_DB - 20 } }  },
                    {   5500, { { QUASAR_MAX_GAIN_DB, QUASAR_MAX_GAIN_DB - 20 }, { QUASAR_MAX_GAIN_DB, QUASAR_MAX_GAIN_DB - 20 }, { QUASAR_MAX_GAIN_DB, QUASAR_MAX_GAIN_DB - 20 } }  },
                    {   5640, { { QUASAR_MAX_GAIN_DB, QUASAR_MAX_GAIN_DB - 20 }, { QUASAR_MAX_GAIN_DB, QUASAR_MAX_GAIN_DB - 20 }, { QUASAR_MAX_GAIN_DB, QUASAR_MAX_GAIN_DB - 20 } }  },
                    {   5700, { { QUASAR_MAX_GAIN_DB, QUASAR_MAX_GAIN_DB - 20 }, { QUASAR_MAX_GAIN_DB, QUASAR_MAX_GAIN_DB - 20 }, { QUASAR_MAX_GAIN_DB, QUASAR_MAX_GAIN_DB - 20 } }  },
                    {   5825, { { QUASAR_MAX_GAIN_DB, QUASAR_MAX_GAIN_DB - 20 }, { QUASAR_MAX_GAIN_DB, QUASAR_MAX_GAIN_DB - 20 }, { QUASAR_MAX_GAIN_DB, QUASAR_MAX_GAIN_DB - 20 } }  }
                }

};

const sTxLoCorrectChannel txLoCorrectTable[NUM_DEMO_CAL_CHANNELS] =
// EEPROM_TABLE_TX_LO
{
    // typedef struct
    // {
    //     tANI_S6 iLo;
    //     tANI_S6 qLo;
    // }tTxLoCorrect;
    //typedef tTxLoCorrect sTxLoCorrectChannel[PHY_MAX_TX_CHAINS][NUM_TX_GAIN_STEPS];

    {
        {
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        }, //Tx0
        {
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        }  //Tx1
    },  //Chan { 0, 0 }
    {
        {
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        }, //Tx0
        {
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        }  //Tx1
    },  //Chan 1
    {
        {
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        }, //Tx0
        {
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        }  //Tx1
    },  //Chan 2
    {
        {
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        }, //Tx0
        {
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        }  //Tx1
    },  //Chan 3
    {
        {
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        }, //Tx0
        {
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        }  //Tx1
    },  //Chan 4
    {
        {
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        }, //Tx0
        {
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        }  //Tx1
    },  //Chan 5
    {
        {
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        }, //Tx0
        {
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        }  //Tx1
    },  //Chan 6
    {
        {
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        }, //Tx0
        {
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        }  //Tx1
    },  //Chan 7
    {
        {
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        }, //Tx0
        {
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        }  //Tx1
    },  //Chan 8
    {
        {
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        }, //Tx0
        {
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        }  //Tx1
    },  //Chan 9
    {
        {
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        }, //Tx0
        {
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        }  //Tx1
    },  //Chan 10
    {
        {
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        }, //Tx0
        {
            { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
        }  //Tx1
    }   //Chan 11
};

/*  //need to nail down how many registers need to be in this table
 const sQuasarChannelRegs  quasarRegsTable[NUM_RF_CHANNELS] =
 // EEPROM_TABLE_QUASAR_REGS
 {
     0,
 };
*/

const sRegulatoryDomains  regDomains[NUM_REG_DOMAINS] =
// EEPROM_TABLE_REGULATORY_DOMAINS_V2
{
    // typedef struct
    // {
    //     tANI_BOOLEAN enabled;
    //     tPowerdBm pwrLimit;
    // }sRegulatoryChannel;

    // typedef struct
    // {
    //     sRegulatoryChannel channels[NUM_RF_CHANNELS];
    //     t2Decimal antennaGain[NUM_RF_SUBBANDS][PHY_MAX_TX_CHAINS];
    //     t2Decimal bRatePowerOffset[NUM_2_4GHZ_CHANNELS];
    // }sRegulatoryDomains;

    //sRegulatoryDomains  regDomains[NUM_REG_DOMAINS];


    {   // REG_DOMAIN_FCC start
        { //sRegulatoryChannel start
            //enabled, pwrLimit
                               //2.4GHz Band
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_1,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_2,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_3,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_4,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_5,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_6,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_7,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_8,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_9,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_10,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_11,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_12,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_13,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_14,

                                     //4.9GHz Band
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_240,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_244,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_248,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_252,

            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_208,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_212,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_216,

                                      //5GHz Low & Mid U-NII Band
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_36,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_40,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_44,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_48,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_52,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_56,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_60,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_64,

                                     //5GHz Mid Band - ETSI
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_100,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_104,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_108,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_112,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_116,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_120,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_124,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_128,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_132,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_136,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_140,

                                     //5GHz High U-NII Band
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_149,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_153,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_157,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_161,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_165,


                                     //channel bonded channels
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_3,
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_4,
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_5,
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_6,
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_7,
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_8,
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_9,
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_10,
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_11,

            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_242,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_246,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_250,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_210,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_214,


            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_38,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_42,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_46,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_50,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_54,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_58,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_62,

            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_102
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_106
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_110
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_114
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_118
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_122
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_126
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_130
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_134
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_138

            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_151,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_155,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_159,
            {eANI_BOOLEAN_FALSE, 30}             //RF_CHAN_BOND_163

        }, //sRegulatoryChannel end

        {   //antennaGain[PHY_MAX_TX_CHAINS]
            { 0, 0 },   // RF_SUBBAND_2_4_GHZ
            { 0, 0 },   // RF_SUBBAND_5_LOW_GHZ
            { 0, 0 },   // RF_SUBBAND_5_MID_GHZ
            { 0, 0 },   // RF_SUBBAND_5_HIGH_GHZ
            { 0, 0 }    // RF_SUBBAND_4_9_GHZ
        },

        { // bRatePowerOffset start
            //2.4GHz Band
            0,                       //RF_CHAN_1,
            0,                       //RF_CHAN_2,
            0,                       //RF_CHAN_3,
            0,                       //RF_CHAN_4,
            0,                       //RF_CHAN_5,
            0,                       //RF_CHAN_6,
            0,                       //RF_CHAN_7,
            0,                       //RF_CHAN_8,
            0,                       //RF_CHAN_9,
            0,                       //RF_CHAN_10,
            0,                       //RF_CHAN_11,
            0,                       //RF_CHAN_12,
            0,                       //RF_CHAN_13,
            0,                       //RF_CHAN_14,
        } // bRatePowerOffset end
    }, // REG_DOMAIN_FCC end

    {   // REG_DOMAIN_ETSI start
        { //sRegulatoryChannel start
            //enabled, pwrLimit
                               //2.4GHz Band
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_1,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_2,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_3,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_4,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_5,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_6,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_7,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_8,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_9,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_10,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_11,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_12,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_13,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_14,

                                     //4.9GHz Band
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_240,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_244,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_248,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_252,

            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_208,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_212,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_216,

                                      //5GHz Low & Mid U-NII Band
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_36,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_40,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_44,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_48,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_52,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_56,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_60,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_64,

                                     //5GHz Mid Band - ETSI
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_100,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_104,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_108,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_112,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_116,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_120,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_124,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_128,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_132,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_136,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_140,

                                     //5GHz High U-NII Band
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_149,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_153,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_157,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_161,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_165,


                                     //channel bonded channels
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_3,
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_4,
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_5,
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_6,
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_7,
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_8,
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_9,
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_10,
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_11,

            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_242,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_246,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_250,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_210,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_214,


            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_38,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_42,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_46,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_50,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_54,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_58,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_62,

            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_102
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_106
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_110
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_114
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_118
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_122
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_126
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_130
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_134
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_138

            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_151,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_155,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_159,
            {eANI_BOOLEAN_FALSE, 30}             //RF_CHAN_BOND_163

        }, //sRegulatoryChannel end

        {   //antennaGain[PHY_MAX_TX_CHAINS]
            { 0, 0 },   // RF_SUBBAND_2_4_GHZ
            { 0, 0 },   // RF_SUBBAND_5_LOW_GHZ
            { 0, 0 },   // RF_SUBBAND_5_MID_GHZ
            { 0, 0 },   // RF_SUBBAND_5_HIGH_GHZ
            { 0, 0 }    // RF_SUBBAND_4_9_GHZ
        },

        { // bRatePowerOffset start
            //2.4GHz Band
            0,                       //RF_CHAN_1,
            0,                       //RF_CHAN_2,
            0,                       //RF_CHAN_3,
            0,                       //RF_CHAN_4,
            0,                       //RF_CHAN_5,
            0,                       //RF_CHAN_6,
            0,                       //RF_CHAN_7,
            0,                       //RF_CHAN_8,
            0,                       //RF_CHAN_9,
            0,                       //RF_CHAN_10,
            0,                       //RF_CHAN_11,
            0,                       //RF_CHAN_12,
            0,                       //RF_CHAN_13,
            0,                       //RF_CHAN_14,
        } // bRatePowerOffset end
    }, // REG_DOMAIN_ETSI end

    {   // REG_DOMAIN_JAPAN start
        { //sRegulatoryChannel start
            //enabled, pwrLimit
                               //2.4GHz Band
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_1,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_2,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_3,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_4,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_5,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_6,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_7,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_8,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_9,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_10,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_11,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_12,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_13,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_14,

                                     //4.9GHz Band
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_240,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_244,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_248,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_252,

            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_208,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_212,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_216,

                                      //5GHz Low & Mid U-NII Band
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_36,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_40,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_44,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_48,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_52,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_56,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_60,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_64,

                                     //5GHz Mid Band - ETSI
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_100,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_104,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_108,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_112,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_116,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_120,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_124,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_128,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_132,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_136,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_140,

                                     //5GHz High U-NII Band
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_149,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_153,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_157,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_161,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_165,


                                     //channel bonded channels
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_3,
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_4,
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_5,
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_6,
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_7,
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_8,
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_9,
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_10,
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_11,

            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_242,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_246,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_250,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_210,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_214,


            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_38,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_42,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_46,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_50,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_54,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_58,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_62,

            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_102
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_106
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_110
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_114
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_118
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_122
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_126
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_130
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_134
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_138

            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_151,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_155,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_159,
            {eANI_BOOLEAN_FALSE, 30}             //RF_CHAN_BOND_163

        }, //sRegulatoryChannel end

        {   //antennaGain[PHY_MAX_TX_CHAINS]
            { 0, 0 },   // RF_SUBBAND_2_4_GHZ
            { 0, 0 },   // RF_SUBBAND_5_LOW_GHZ
            { 0, 0 },   // RF_SUBBAND_5_MID_GHZ
            { 0, 0 },   // RF_SUBBAND_5_HIGH_GHZ
            { 0, 0 }    // RF_SUBBAND_4_9_GHZ
        },

        { // bRatePowerOffset start
            //2.4GHz Band
            0,                       //RF_CHAN_1,
            0,                       //RF_CHAN_2,
            0,                       //RF_CHAN_3,
            0,                       //RF_CHAN_4,
            0,                       //RF_CHAN_5,
            0,                       //RF_CHAN_6,
            0,                       //RF_CHAN_7,
            0,                       //RF_CHAN_8,
            0,                       //RF_CHAN_9,
            0,                       //RF_CHAN_10,
            0,                       //RF_CHAN_11,
            0,                       //RF_CHAN_12,
            0,                       //RF_CHAN_13,
            0,                       //RF_CHAN_14,
        } // bRatePowerOffset end
    }, // REG_DOMAIN_JAPAN end

    {   // REG_DOMAIN_WORLD start
        { //sRegulatoryChannel start
            //enabled, pwrLimit
                               //2.4GHz Band
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_1,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_2,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_3,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_4,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_5,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_6,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_7,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_8,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_9,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_10,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_11,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_12,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_13,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_14,

                                     //4.9GHz Band
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_240,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_244,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_248,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_252,

            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_208,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_212,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_216,

                                      //5GHz Low & Mid U-NII Band
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_36,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_40,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_44,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_48,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_52,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_56,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_60,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_64,

                                     //5GHz Mid Band - ETSI
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_100,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_104,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_108,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_112,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_116,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_120,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_124,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_128,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_132,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_136,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_140,

                                     //5GHz High U-NII Band
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_149,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_153,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_157,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_161,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_165,


                                     //channel bonded channels
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_3,
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_4,
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_5,
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_6,
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_7,
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_8,
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_9,
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_10,
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_11,

            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_242,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_246,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_250,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_210,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_214,


            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_38,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_42,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_46,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_50,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_54,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_58,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_62,

            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_102
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_106
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_110
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_114
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_118
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_122
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_126
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_130
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_134
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_138

            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_151,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_155,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_159,
            {eANI_BOOLEAN_FALSE, 30}             //RF_CHAN_BOND_163

        }, //sRegulatoryChannel end

        {   //antennaGain[PHY_MAX_TX_CHAINS]
            { 0, 0 },   // RF_SUBBAND_2_4_GHZ
            { 0, 0 },   // RF_SUBBAND_5_LOW_GHZ
            { 0, 0 },   // RF_SUBBAND_5_MID_GHZ
            { 0, 0 },   // RF_SUBBAND_5_HIGH_GHZ
            { 0, 0 }    // RF_SUBBAND_4_9_GHZ
        },

        { // bRatePowerOffset start
            //2.4GHz Band
            0,                       //RF_CHAN_1,
            0,                       //RF_CHAN_2,
            0,                       //RF_CHAN_3,
            0,                       //RF_CHAN_4,
            0,                       //RF_CHAN_5,
            0,                       //RF_CHAN_6,
            0,                       //RF_CHAN_7,
            0,                       //RF_CHAN_8,
            0,                       //RF_CHAN_9,
            0,                       //RF_CHAN_10,
            0,                       //RF_CHAN_11,
            0,                       //RF_CHAN_12,
            0,                       //RF_CHAN_13,
            0,                       //RF_CHAN_14,
        } // bRatePowerOffset end
    }, // REG_DOMAIN_WORLD end

    {   // REG_DOMAIN_N_AMER_EXC_FCC start
        { //sRegulatoryChannel start
            //enabled, pwrLimit
                               //2.4GHz Band
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_1,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_2,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_3,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_4,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_5,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_6,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_7,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_8,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_9,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_10,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_11,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_12,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_13,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_14,

                                     //4.9GHz Band
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_240,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_244,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_248,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_252,

            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_208,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_212,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_216,

                                      //5GHz Low & Mid U-NII Band
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_36,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_40,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_44,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_48,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_52,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_56,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_60,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_64,

                                     //5GHz Mid Band - ETSI
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_100,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_104,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_108,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_112,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_116,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_120,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_124,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_128,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_132,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_136,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_140,

                                     //5GHz High U-NII Band
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_149,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_153,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_157,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_161,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_165,


                                     //channel bonded channels
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_3,
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_4,
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_5,
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_6,
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_7,
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_8,
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_9,
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_10,
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_11,

            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_242,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_246,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_250,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_210,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_214,


            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_38,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_42,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_46,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_50,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_54,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_58,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_62,

            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_102
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_106
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_110
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_114
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_118
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_122
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_126
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_130
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_134
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_138

            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_151,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_155,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_159,
            {eANI_BOOLEAN_FALSE, 30}             //RF_CHAN_BOND_163

        }, //sRegulatoryChannel end

        {   //antennaGain[PHY_MAX_TX_CHAINS]
            { 0, 0 },   // RF_SUBBAND_2_4_GHZ
            { 0, 0 },   // RF_SUBBAND_5_LOW_GHZ
            { 0, 0 },   // RF_SUBBAND_5_MID_GHZ
            { 0, 0 },   // RF_SUBBAND_5_HIGH_GHZ
            { 0, 0 }    // RF_SUBBAND_4_9_GHZ
        },

        { // bRatePowerOffset start
            //2.4GHz Band
            0,                       //RF_CHAN_1,
            0,                       //RF_CHAN_2,
            0,                       //RF_CHAN_3,
            0,                       //RF_CHAN_4,
            0,                       //RF_CHAN_5,
            0,                       //RF_CHAN_6,
            0,                       //RF_CHAN_7,
            0,                       //RF_CHAN_8,
            0,                       //RF_CHAN_9,
            0,                       //RF_CHAN_10,
            0,                       //RF_CHAN_11,
            0,                       //RF_CHAN_12,
            0,                       //RF_CHAN_13,
            0,                       //RF_CHAN_14,
        } // bRatePowerOffset end
    },   // REG_DOMAIN_N_AMER_EXC_FCC end

    {   // REG_DOMAIN_APAC start
        { //sRegulatoryChannel start
            //enabled, pwrLimit
                               //2.4GHz Band
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_1,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_2,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_3,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_4,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_5,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_6,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_7,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_8,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_9,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_10,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_11,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_12,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_13,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_14,

                                     //4.9GHz Band
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_240,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_244,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_248,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_252,

            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_208,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_212,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_216,

                                      //5GHz Low & Mid U-NII Band
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_36,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_40,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_44,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_48,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_52,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_56,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_60,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_64,

                                     //5GHz Mid Band - ETSI
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_100,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_104,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_108,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_112,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_116,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_120,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_124,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_128,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_132,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_136,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_140,

                                     //5GHz High U-NII Band
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_149,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_153,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_157,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_161,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_165,


                                     //channel bonded channels
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_3,
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_4,
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_5,
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_6,
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_7,
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_8,
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_9,
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_10,
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_11,

            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_242,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_246,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_250,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_210,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_214,


            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_38,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_42,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_46,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_50,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_54,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_58,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_62,

            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_102
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_106
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_110
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_114
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_118
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_122
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_126
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_130
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_134
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_138

            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_151,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_155,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_159,
            {eANI_BOOLEAN_FALSE, 30}             //RF_CHAN_BOND_163

        }, //sRegulatoryChannel end

        {   //antennaGain[PHY_MAX_TX_CHAINS]
            { 0, 0 },   // RF_SUBBAND_2_4_GHZ
            { 0, 0 },   // RF_SUBBAND_5_LOW_GHZ
            { 0, 0 },   // RF_SUBBAND_5_MID_GHZ
            { 0, 0 },   // RF_SUBBAND_5_HIGH_GHZ
            { 0, 0 }    // RF_SUBBAND_4_9_GHZ
        },

        { // bRatePowerOffset start
            //2.4GHz Band
            0,                       //RF_CHAN_1,
            0,                       //RF_CHAN_2,
            0,                       //RF_CHAN_3,
            0,                       //RF_CHAN_4,
            0,                       //RF_CHAN_5,
            0,                       //RF_CHAN_6,
            0,                       //RF_CHAN_7,
            0,                       //RF_CHAN_8,
            0,                       //RF_CHAN_9,
            0,                       //RF_CHAN_10,
            0,                       //RF_CHAN_11,
            0,                       //RF_CHAN_12,
            0,                       //RF_CHAN_13,
            0,                       //RF_CHAN_14,
        } // bRatePowerOffset end
    }, // REG_DOMAIN_APAC end

    {   // REG_DOMAIN_KOREA start
        { //sRegulatoryChannel start
            //enabled, pwrLimit
                               //2.4GHz Band
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_1,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_2,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_3,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_4,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_5,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_6,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_7,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_8,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_9,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_10,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_11,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_12,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_13,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_14,

                                     //4.9GHz Band
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_240,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_244,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_248,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_252,

            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_208,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_212,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_216,

                                      //5GHz Low & Mid U-NII Band
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_36,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_40,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_44,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_48,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_52,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_56,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_60,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_64,

                                     //5GHz Mid Band - ETSI
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_100,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_104,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_108,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_112,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_116,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_120,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_124,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_128,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_132,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_136,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_140,

                                     //5GHz High U-NII Band
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_149,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_153,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_157,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_161,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_165,


                                     //channel bonded channels
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_3,
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_4,
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_5,
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_6,
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_7,
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_8,
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_9,
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_10,
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_11,

            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_242,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_246,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_250,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_210,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_214,


            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_38,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_42,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_46,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_50,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_54,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_58,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_62,

            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_102
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_106
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_110
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_114
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_118
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_122
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_126
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_130
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_134
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_138

            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_151,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_155,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_159,
            {eANI_BOOLEAN_FALSE, 30}             //RF_CHAN_BOND_163

        }, //sRegulatoryChannel end

        {   //antennaGain[PHY_MAX_TX_CHAINS]
            { 0, 0 },   // RF_SUBBAND_2_4_GHZ
            { 0, 0 },   // RF_SUBBAND_5_LOW_GHZ
            { 0, 0 },   // RF_SUBBAND_5_MID_GHZ
            { 0, 0 },   // RF_SUBBAND_5_HIGH_GHZ
            { 0, 0 }    // RF_SUBBAND_4_9_GHZ
        },

        { // bRatePowerOffset start
            //2.4GHz Band
            0,                       //RF_CHAN_1,
            0,                       //RF_CHAN_2,
            0,                       //RF_CHAN_3,
            0,                       //RF_CHAN_4,
            0,                       //RF_CHAN_5,
            0,                       //RF_CHAN_6,
            0,                       //RF_CHAN_7,
            0,                       //RF_CHAN_8,
            0,                       //RF_CHAN_9,
            0,                       //RF_CHAN_10,
            0,                       //RF_CHAN_11,
            0,                       //RF_CHAN_12,
            0,                       //RF_CHAN_13,
            0,                       //RF_CHAN_14,
        } // bRatePowerOffset end
    }, // REG_DOMAIN_KOREA end

    {   // REG_DOMAIN_HI_5GHZ start
        { //sRegulatoryChannel start
            //enabled, pwrLimit
                               //2.4GHz Band
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_1,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_2,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_3,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_4,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_5,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_6,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_7,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_8,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_9,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_10,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_11,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_12,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_13,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_14,

                                     //4.9GHz Band
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_240,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_244,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_248,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_252,

            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_208,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_212,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_216,

                                      //5GHz Low & Mid U-NII Band
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_36,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_40,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_44,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_48,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_52,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_56,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_60,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_64,

                                     //5GHz Mid Band - ETSI
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_100,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_104,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_108,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_112,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_116,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_120,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_124,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_128,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_132,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_136,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_140,

                                     //5GHz High U-NII Band
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_149,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_153,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_157,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_161,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_165,


                                     //channel bonded channels
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_3,
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_4,
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_5,
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_6,
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_7,
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_8,
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_9,
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_10,
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_11,

            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_242,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_246,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_250,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_210,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_214,


            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_38,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_42,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_46,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_50,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_54,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_58,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_62,

            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_102
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_106
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_110
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_114
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_118
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_122
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_126
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_130
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_134
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_138

            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_151,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_155,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_159,
            {eANI_BOOLEAN_FALSE, 30}             //RF_CHAN_BOND_163

        }, //sRegulatoryChannel end

        {   //antennaGain[PHY_MAX_TX_CHAINS]
            { 0, 0 },   // RF_SUBBAND_2_4_GHZ
            { 0, 0 },   // RF_SUBBAND_5_LOW_GHZ
            { 0, 0 },   // RF_SUBBAND_5_MID_GHZ
            { 0, 0 },   // RF_SUBBAND_5_HIGH_GHZ
            { 0, 0 }    // RF_SUBBAND_4_9_GHZ
        },

        { // bRatePowerOffset start
            //2.4GHz Band
            0,                       //RF_CHAN_1,
            0,                       //RF_CHAN_2,
            0,                       //RF_CHAN_3,
            0,                       //RF_CHAN_4,
            0,                       //RF_CHAN_5,
            0,                       //RF_CHAN_6,
            0,                       //RF_CHAN_7,
            0,                       //RF_CHAN_8,
            0,                       //RF_CHAN_9,
            0,                       //RF_CHAN_10,
            0,                       //RF_CHAN_11,
            0,                       //RF_CHAN_12,
            0,                       //RF_CHAN_13,
            0,                       //RF_CHAN_14,
        } // bRatePowerOffset end
    }, // REG_DOMAIN_HI_5GHZ end

    {   // REG_DOMAIN_NO_5GHZ start
        { //sRegulatoryChannel start
            //enabled, pwrLimit
                               //2.4GHz Band
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_1,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_2,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_3,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_4,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_5,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_6,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_7,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_8,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_9,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_10,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_11,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_12,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_13,
            {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_14,

                                     //4.9GHz Band
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_240,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_244,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_248,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_252,

            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_208,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_212,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_216,

                                      //5GHz Low & Mid U-NII Band
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_36,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_40,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_44,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_48,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_52,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_56,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_60,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_64,

                                     //5GHz Mid Band - ETSI
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_100,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_104,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_108,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_112,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_116,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_120,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_124,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_128,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_132,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_136,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_140,

                                     //5GHz High U-NII Band
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_149,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_153,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_157,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_161,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_165,


                                     //channel bonded channels
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_3,
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_4,
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_5,
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_6,
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_7,
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_8,
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_9,
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_10,
            {eANI_BOOLEAN_TRUE, 30},            //RF_CHAN_BOND_11,

            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_242,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_246,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_250,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_210,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_214,


            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_38,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_42,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_46,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_50,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_54,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_58,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_62,

            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_102
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_106
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_110
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_114
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_118
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_122
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_126
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_130
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_134
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_138

            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_151,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_155,
            {eANI_BOOLEAN_FALSE, 30},            //RF_CHAN_BOND_159,
            {eANI_BOOLEAN_FALSE, 30}             //RF_CHAN_BOND_163

        }, //sRegulatoryChannel end

        {   //antennaGain[PHY_MAX_TX_CHAINS]
            { 0, 0 },   // RF_SUBBAND_2_4_GHZ
            { 0, 0 },   // RF_SUBBAND_5_LOW_GHZ
            { 0, 0 },   // RF_SUBBAND_5_MID_GHZ
            { 0, 0 },   // RF_SUBBAND_5_HIGH_GHZ
            { 0, 0 }    // RF_SUBBAND_4_9_GHZ
        },

        { // bRatePowerOffset start
            //2.4GHz Band
            0,                       //RF_CHAN_1,
            0,                       //RF_CHAN_2,
            0,                       //RF_CHAN_3,
            0,                       //RF_CHAN_4,
            0,                       //RF_CHAN_5,
            0,                       //RF_CHAN_6,
            0,                       //RF_CHAN_7,
            0,                       //RF_CHAN_8,
            0,                       //RF_CHAN_9,
            0,                       //RF_CHAN_10,
            0,                       //RF_CHAN_11,
            0,                       //RF_CHAN_12,
            0,                       //RF_CHAN_13,
            0,                       //RF_CHAN_14,
        } // bRatePowerOffset end
    } // REG_DOMAIN_NO_5GHZ end
};




const tTpcParams          pwrBgParams =
// EEPROM_TABLE_TPC_PARAMS
{
    MAX_TPC_CAL_POINTS, 2, 10, 0
};

const tTpcConfig          pwrBgBand[MAX_TPC_CHANNELS] =
// EEPROM_TABLE_TPC_CONFIG
{
    //#define MAX_TPC_CAL_POINTS      (12)
    // typedef tANI_U8 tTxGainCombo;
    // typedef tANI_U8 tPowerDetect;

    // typedef union
    // {
    //     tTxGainCombo txGain;               //7-bit gain used to measure the pwrDetAdc value
    //     tANI_U8 hi8_adjustedPwrDet;        //if the MSB is set in adjustedPwrDet, then these are extra bits of precision
    // }uExtraLutBits;
    // 
    // 
    // typedef struct
    // {
    //     uExtraLutBits extraPrecision;      //union to afford backward compatibility with txGain usage
    //                                        //new usage is to allow up to 8 additional bits of precision to adjusted LUT values stored in EEPROM
    //     tPowerDetect pwrDetAdc;            //= SENSED_PWR register, which reports the 8-bit ADC
    //                                        // the stored ADC value gets shifted to 7-bits as the index to the LUT
    //     tPowerDetect adjustedPwrDet;       //7-bit value that goes into the LUT at the LUT[pwrDet] location
    //                                        //MSB set if extraPrecision.hi8_adjustedPwrDet is used
    // }tTpcCaldPowerPoint;
    
    // typedef tTpcCaldPowerPoint tTpcCaldPowerTable[PHY_MAX_TX_CHAINS][MAX_TPC_CAL_POINTS];

    //typedef tANI_S16 t2Decimal;

    // typedef struct
    // {
    //     t2Decimal min;  //sometimes used for tx0 when comparing chain powers
    //     t2Decimal max;  //sometimes used for tx1 when comparing chain powers
    // }tPowerdBmRange;        //absolute power measurement precision maintained to two decimal places

    // typedef struct
    // {
    //     tANI_U16 freq;                                          //frequency in MHz
    //     tANI_U16 reserved;
    //     tPowerdBmRange absPower;                           //Power range common to both chains
    //     tTpcCaldPowerTable empirical;                      //calibrated power points
    // }tTpcConfig;

    {
        2412, 0,  //freq, reserved
        { 0, 3175 },   //absPower
        {
            {
                { { 0x00 }, 0, 0x80 },          //cal point 0
                { { 0xF }, 0x7F, 0xFF },    //cal point 1
                { { 0xF }, 0x7F, 0xFF },    //cal point 2
                { { 0xF }, 0x7F, 0xFF },    //cal point 3
                { { 0xF }, 0x7F, 0xFF },    //cal point 4
                { { 0xF }, 0x7F, 0xFF },    //cal point 5
                { { 0xF }, 0x7F, 0xFF },    //cal point 6
                { { 0xF }, 0x7F, 0xFF },    //cal point 7
                { { 0xF }, 0x7F, 0xFF },    //cal point 8
                { { 0xF }, 0x7F, 0xFF },    //cal point 9
                { { 0xF }, 0x7F, 0xFF },    //cal point 10
                { { 0xF }, 0x7F, 0xFF },    //cal point 11
            }, //TX0
            {
                { { 0x00 }, 0, 0x80 },          //cal point 0
                { { 0xF }, 0x7F, 0xFF },    //cal point 1
                { { 0xF }, 0x7F, 0xFF },    //cal point 2
                { { 0xF }, 0x7F, 0xFF },    //cal point 3
                { { 0xF }, 0x7F, 0xFF },    //cal point 4
                { { 0xF }, 0x7F, 0xFF },    //cal point 5
                { { 0xF }, 0x7F, 0xFF },    //cal point 6
                { { 0xF }, 0x7F, 0xFF },    //cal point 7
                { { 0xF }, 0x7F, 0xFF },    //cal point 8
                { { 0xF }, 0x7F, 0xFF },    //cal point 9
                { { 0xF }, 0x7F, 0xFF },    //cal point 10
                { { 0xF }, 0x7F, 0xFF },    //cal point 11
            }  //TX1
        }   //empirical
    },    //     CHAN_1,
    {
        2484, 0,  //freq, reserved
        { 0, 3175 },   //absPower
        {
            {
                { { 0x00 }, 0, 0x80 },          //cal point 0
                { { 0xF }, 0x7F, 0xFF },    //cal point 1
                { { 0xF }, 0x7F, 0xFF },    //cal point 2
                { { 0xF }, 0x7F, 0xFF },    //cal point 3
                { { 0xF }, 0x7F, 0xFF },    //cal point 4
                { { 0xF }, 0x7F, 0xFF },    //cal point 5
                { { 0xF }, 0x7F, 0xFF },    //cal point 6
                { { 0xF }, 0x7F, 0xFF },    //cal point 7
                { { 0xF }, 0x7F, 0xFF },    //cal point 8
                { { 0xF }, 0x7F, 0xFF },    //cal point 9
                { { 0xF }, 0x7F, 0xFF },    //cal point 10
                { { 0xF }, 0x7F, 0xFF },    //cal point 11
            }, //TX0
            {
                { { 0x00 }, 0, 0x80 },          //cal point 0
                { { 0xF }, 0x7F, 0xFF },    //cal point 1
                { { 0xF }, 0x7F, 0xFF },    //cal point 2
                { { 0xF }, 0x7F, 0xFF },    //cal point 3
                { { 0xF }, 0x7F, 0xFF },    //cal point 4
                { { 0xF }, 0x7F, 0xFF },    //cal point 5
                { { 0xF }, 0x7F, 0xFF },    //cal point 6
                { { 0xF }, 0x7F, 0xFF },    //cal point 7
                { { 0xF }, 0x7F, 0xFF },    //cal point 8
                { { 0xF }, 0x7F, 0xFF },    //cal point 9
                { { 0xF }, 0x7F, 0xFF },    //cal point 10
                { { 0xF }, 0x7F, 0xFF },    //cal point 11
            }  //TX1
        }   //empirical
    },  //     CHAN_2,
    {
        0, 0,  //freq, reserved
        { 0, 0 },   //absPower
        {
            {
                { { 0 }, 0, 0 },    //cal point 0
                { { 0 }, 0, 0 },    //cal point 1
                { { 0 }, 0, 0 },    //cal point 2
                { { 0 }, 0, 0 },    //cal point 3
                { { 0 }, 0, 0 },    //cal point 4
                { { 0 }, 0, 0 },    //cal point 5
                { { 0 }, 0, 0 },    //cal point 6
                { { 0 }, 0, 0 },    //cal point 7
                { { 0 }, 0, 0 },    //cal point 8
                { { 0 }, 0, 0 },    //cal point 9
                { { 0 }, 0, 0 },    //cal point 10
                { { 0 }, 0, 0 },    //cal point 11
            }, //TX0
            {
                { { 0 }, 0, 0 },    //cal point 0
                { { 0 }, 0, 0 },    //cal point 1
                { { 0 }, 0, 0 },    //cal point 2
                { { 0 }, 0, 0 },    //cal point 3
                { { 0 }, 0, 0 },    //cal point 4
                { { 0 }, 0, 0 },    //cal point 5
                { { 0 }, 0, 0 },    //cal point 6
                { { 0 }, 0, 0 },    //cal point 7
                { { 0 }, 0, 0 },    //cal point 8
                { { 0 }, 0, 0 },    //cal point 9
                { { 0 }, 0, 0 },    //cal point 10
                { { 0 }, 0, 0 },    //cal point 11
            }  //TX1
        }   //empirical
    },  //     CHAN_3,
    {
        0, 0,  //freq, reserved
        { 0, 0 },   //absPower
        {
            {
                { { 0 }, 0, 0 },    //cal point 0
                { { 0 }, 0, 0 },    //cal point 1
                { { 0 }, 0, 0 },    //cal point 2
                { { 0 }, 0, 0 },    //cal point 3
                { { 0 }, 0, 0 },    //cal point 4
                { { 0 }, 0, 0 },    //cal point 5
                { { 0 }, 0, 0 },    //cal point 6
                { { 0 }, 0, 0 },    //cal point 7
                { { 0 }, 0, 0 },    //cal point 8
                { { 0 }, 0, 0 },    //cal point 9
                { { 0 }, 0, 0 },    //cal point 10
                { { 0 }, 0, 0 },    //cal point 11
            }, //TX0
            {
                { { 0 }, 0, 0 },    //cal point 0
                { { 0 }, 0, 0 },    //cal point 1
                { { 0 }, 0, 0 },    //cal point 2
                { { 0 }, 0, 0 },    //cal point 3
                { { 0 }, 0, 0 },    //cal point 4
                { { 0 }, 0, 0 },    //cal point 5
                { { 0 }, 0, 0 },    //cal point 6
                { { 0 }, 0, 0 },    //cal point 7
                { { 0 }, 0, 0 },    //cal point 8
                { { 0 }, 0, 0 },    //cal point 9
                { { 0 }, 0, 0 },    //cal point 10
                { { 0 }, 0, 0 },    //cal point 11
            }  //TX1
        }   //empirical
    },  //     CHAN_4,
    {
        0, 0,  //freq, reserved
        { 0, 0 },   //absPower
        {
            {
                { { 0 }, 0, 0 },    //cal point 0
                { { 0 }, 0, 0 },    //cal point 1
                { { 0 }, 0, 0 },    //cal point 2
                { { 0 }, 0, 0 },    //cal point 3
                { { 0 }, 0, 0 },    //cal point 4
                { { 0 }, 0, 0 },    //cal point 5
                { { 0 }, 0, 0 },    //cal point 6
                { { 0 }, 0, 0 },    //cal point 7
                { { 0 }, 0, 0 },    //cal point 8
                { { 0 }, 0, 0 },    //cal point 9
                { { 0 }, 0, 0 },    //cal point 10
                { { 0 }, 0, 0 },    //cal point 11
            }, //TX0
            {
                { { 0 }, 0, 0 },    //cal point 0
                { { 0 }, 0, 0 },    //cal point 1
                { { 0 }, 0, 0 },    //cal point 2
                { { 0 }, 0, 0 },    //cal point 3
                { { 0 }, 0, 0 },    //cal point 4
                { { 0 }, 0, 0 },    //cal point 5
                { { 0 }, 0, 0 },    //cal point 6
                { { 0 }, 0, 0 },    //cal point 7
                { { 0 }, 0, 0 },    //cal point 8
                { { 0 }, 0, 0 },    //cal point 9
                { { 0 }, 0, 0 },    //cal point 10
                { { 0 }, 0, 0 },    //cal point 11
            }  //TX1
        }   //empirical
    },  //     CHAN_5,
    {
        0, 0,  //freq, reserved
        { 0, 0 },   //absPower
        {
            {
                { { 0 }, 0, 0 },    //cal point 0
                { { 0 }, 0, 0 },    //cal point 1
                { { 0 }, 0, 0 },    //cal point 2
                { { 0 }, 0, 0 },    //cal point 3
                { { 0 }, 0, 0 },    //cal point 4
                { { 0 }, 0, 0 },    //cal point 5
                { { 0 }, 0, 0 },    //cal point 6
                { { 0 }, 0, 0 },    //cal point 7
                { { 0 }, 0, 0 },    //cal point 8
                { { 0 }, 0, 0 },    //cal point 9
                { { 0 }, 0, 0 },    //cal point 10
                { { 0 }, 0, 0 },    //cal point 11
            }, //TX0
            {
                { { 0 }, 0, 0 },    //cal point 0
                { { 0 }, 0, 0 },    //cal point 1
                { { 0 }, 0, 0 },    //cal point 2
                { { 0 }, 0, 0 },    //cal point 3
                { { 0 }, 0, 0 },    //cal point 4
                { { 0 }, 0, 0 },    //cal point 5
                { { 0 }, 0, 0 },    //cal point 6
                { { 0 }, 0, 0 },    //cal point 7
                { { 0 }, 0, 0 },    //cal point 8
                { { 0 }, 0, 0 },    //cal point 9
                { { 0 }, 0, 0 },    //cal point 10
                { { 0 }, 0, 0 },    //cal point 11
            }  //TX1
        }   //empirical
    },  //     CHAN_6,
    {
        0, 0,  //freq, reserved
        { 0, 0 },   //absPower
        {
            {
                { { 0 }, 0, 0 },    //cal point 0
                { { 0 }, 0, 0 },    //cal point 1
                { { 0 }, 0, 0 },    //cal point 2
                { { 0 }, 0, 0 },    //cal point 3
                { { 0 }, 0, 0 },    //cal point 4
                { { 0 }, 0, 0 },    //cal point 5
                { { 0 }, 0, 0 },    //cal point 6
                { { 0 }, 0, 0 },    //cal point 7
                { { 0 }, 0, 0 },    //cal point 8
                { { 0 }, 0, 0 },    //cal point 9
                { { 0 }, 0, 0 },    //cal point 10
                { { 0 }, 0, 0 },    //cal point 11
            }, //TX0
            {
                { { 0 }, 0, 0 },    //cal point 0
                { { 0 }, 0, 0 },    //cal point 1
                { { 0 }, 0, 0 },    //cal point 2
                { { 0 }, 0, 0 },    //cal point 3
                { { 0 }, 0, 0 },    //cal point 4
                { { 0 }, 0, 0 },    //cal point 5
                { { 0 }, 0, 0 },    //cal point 6
                { { 0 }, 0, 0 },    //cal point 7
                { { 0 }, 0, 0 },    //cal point 8
                { { 0 }, 0, 0 },    //cal point 9
                { { 0 }, 0, 0 },    //cal point 10
                { { 0 }, 0, 0 },    //cal point 11
            }  //TX1
        }   //empirical
    },  //     CHAN_7,
    {
        0, 0,  //freq, reserved
        { 0, 0 },   //absPower
        {
            {
                { { 0 }, 0, 0 },    //cal point 0
                { { 0 }, 0, 0 },    //cal point 1
                { { 0 }, 0, 0 },    //cal point 2
                { { 0 }, 0, 0 },    //cal point 3
                { { 0 }, 0, 0 },    //cal point 4
                { { 0 }, 0, 0 },    //cal point 5
                { { 0 }, 0, 0 },    //cal point 6
                { { 0 }, 0, 0 },    //cal point 7
                { { 0 }, 0, 0 },    //cal point 8
                { { 0 }, 0, 0 },    //cal point 9
                { { 0 }, 0, 0 },    //cal point 10
                { { 0 }, 0, 0 },    //cal point 11
            }, //TX0
            {
                { { 0 }, 0, 0 },    //cal point 0
                { { 0 }, 0, 0 },    //cal point 1
                { { 0 }, 0, 0 },    //cal point 2
                { { 0 }, 0, 0 },    //cal point 3
                { { 0 }, 0, 0 },    //cal point 4
                { { 0 }, 0, 0 },    //cal point 5
                { { 0 }, 0, 0 },    //cal point 6
                { { 0 }, 0, 0 },    //cal point 7
                { { 0 }, 0, 0 },    //cal point 8
                { { 0 }, 0, 0 },    //cal point 9
                { { 0 }, 0, 0 },    //cal point 10
                { { 0 }, 0, 0 },    //cal point 11
            }  //TX1
        }   //empirical
    },  //     CHAN_8,
    {
        0, 0,  //freq, reserved
        { 0, 0 },   //absPower
        {
            {
                { { 0 }, 0, 0 },    //cal point 0
                { { 0 }, 0, 0 },    //cal point 1
                { { 0 }, 0, 0 },    //cal point 2
                { { 0 }, 0, 0 },    //cal point 3
                { { 0 }, 0, 0 },    //cal point 4
                { { 0 }, 0, 0 },    //cal point 5
                { { 0 }, 0, 0 },    //cal point 6
                { { 0 }, 0, 0 },    //cal point 7
                { { 0 }, 0, 0 },    //cal point 8
                { { 0 }, 0, 0 },    //cal point 9
                { { 0 }, 0, 0 },    //cal point 10
                { { 0 }, 0, 0 },    //cal point 11
            }, //TX0
            {
                { { 0 }, 0, 0 },    //cal point 0
                { { 0 }, 0, 0 },    //cal point 1
                { { 0 }, 0, 0 },    //cal point 2
                { { 0 }, 0, 0 },    //cal point 3
                { { 0 }, 0, 0 },    //cal point 4
                { { 0 }, 0, 0 },    //cal point 5
                { { 0 }, 0, 0 },    //cal point 6
                { { 0 }, 0, 0 },    //cal point 7
                { { 0 }, 0, 0 },    //cal point 8
                { { 0 }, 0, 0 },    //cal point 9
                { { 0 }, 0, 0 },    //cal point 10
                { { 0 }, 0, 0 },    //cal point 11
            }  //TX1
        }   //empirical
    },  //     CHAN_9,
    {
        0, 0,  //freq, reserved
        { 0, 0 },   //absPower
        {
            {
                { { 0 }, 0, 0 },    //cal point 0
                { { 0 }, 0, 0 },    //cal point 1
                { { 0 }, 0, 0 },    //cal point 2
                { { 0 }, 0, 0 },    //cal point 3
                { { 0 }, 0, 0 },    //cal point 4
                { { 0 }, 0, 0 },    //cal point 5
                { { 0 }, 0, 0 },    //cal point 6
                { { 0 }, 0, 0 },    //cal point 7
                { { 0 }, 0, 0 },    //cal point 8
                { { 0 }, 0, 0 },    //cal point 9
                { { 0 }, 0, 0 },    //cal point 10
                { { 0 }, 0, 0 },    //cal point 11
            }, //TX0
            {
                { { 0 }, 0, 0 },    //cal point 0
                { { 0 }, 0, 0 },    //cal point 1
                { { 0 }, 0, 0 },    //cal point 2
                { { 0 }, 0, 0 },    //cal point 3
                { { 0 }, 0, 0 },    //cal point 4
                { { 0 }, 0, 0 },    //cal point 5
                { { 0 }, 0, 0 },    //cal point 6
                { { 0 }, 0, 0 },    //cal point 7
                { { 0 }, 0, 0 },    //cal point 8
                { { 0 }, 0, 0 },    //cal point 9
                { { 0 }, 0, 0 },    //cal point 10
                { { 0 }, 0, 0 },    //cal point 11
            }  //TX1
        }   //empirical
    },  //     CHAN_10,
    {
        0, 0,  //freq, reserved
        { 0, 0 },   //absPower
        {
            {
                { { 0 }, 0, 0 },    //cal point 0
                { { 0 }, 0, 0 },    //cal point 1
                { { 0 }, 0, 0 },    //cal point 2
                { { 0 }, 0, 0 },    //cal point 3
                { { 0 }, 0, 0 },    //cal point 4
                { { 0 }, 0, 0 },    //cal point 5
                { { 0 }, 0, 0 },    //cal point 6
                { { 0 }, 0, 0 },    //cal point 7
                { { 0 }, 0, 0 },    //cal point 8
                { { 0 }, 0, 0 },    //cal point 9
                { { 0 }, 0, 0 },    //cal point 10
                { { 0 }, 0, 0 },    //cal point 11
            }, //TX0
            {
                { { 0 }, 0, 0 },    //cal point 0
                { { 0 }, 0, 0 },    //cal point 1
                { { 0 }, 0, 0 },    //cal point 2
                { { 0 }, 0, 0 },    //cal point 3
                { { 0 }, 0, 0 },    //cal point 4
                { { 0 }, 0, 0 },    //cal point 5
                { { 0 }, 0, 0 },    //cal point 6
                { { 0 }, 0, 0 },    //cal point 7
                { { 0 }, 0, 0 },    //cal point 8
                { { 0 }, 0, 0 },    //cal point 9
                { { 0 }, 0, 0 },    //cal point 10
                { { 0 }, 0, 0 },    //cal point 11
            }  //TX1
        }   //empirical
    },  //     CHAN_11,
    {
        0, 0,  //freq, reserved
        { 0, 0 },   //absPower
        {
            {
                { { 0 }, 0, 0 },    //cal point 0
                { { 0 }, 0, 0 },    //cal point 1
                { { 0 }, 0, 0 },    //cal point 2
                { { 0 }, 0, 0 },    //cal point 3
                { { 0 }, 0, 0 },    //cal point 4
                { { 0 }, 0, 0 },    //cal point 5
                { { 0 }, 0, 0 },    //cal point 6
                { { 0 }, 0, 0 },    //cal point 7
                { { 0 }, 0, 0 },    //cal point 8
                { { 0 }, 0, 0 },    //cal point 9
                { { 0 }, 0, 0 },    //cal point 10
                { { 0 }, 0, 0 },    //cal point 11
            }, //TX0
            {
                { { 0 }, 0, 0 },    //cal point 0
                { { 0 }, 0, 0 },    //cal point 1
                { { 0 }, 0, 0 },    //cal point 2
                { { 0 }, 0, 0 },    //cal point 3
                { { 0 }, 0, 0 },    //cal point 4
                { { 0 }, 0, 0 },    //cal point 5
                { { 0 }, 0, 0 },    //cal point 6
                { { 0 }, 0, 0 },    //cal point 7
                { { 0 }, 0, 0 },    //cal point 8
                { { 0 }, 0, 0 },    //cal point 9
                { { 0 }, 0, 0 },    //cal point 10
                { { 0 }, 0, 0 },    //cal point 11
            }  //TX1
        }   //empirical
    },  //     CHAN_12,
    {
        0, 0,  //freq, reserved
        { 0, 0 },   //absPower
        {
            {
                { { 0 }, 0, 0 },    //cal point 0
                { { 0 }, 0, 0 },    //cal point 1
                { { 0 }, 0, 0 },    //cal point 2
                { { 0 }, 0, 0 },    //cal point 3
                { { 0 }, 0, 0 },    //cal point 4
                { { 0 }, 0, 0 },    //cal point 5
                { { 0 }, 0, 0 },    //cal point 6
                { { 0 }, 0, 0 },    //cal point 7
                { { 0 }, 0, 0 },    //cal point 8
                { { 0 }, 0, 0 },    //cal point 9
                { { 0 }, 0, 0 },    //cal point 10
                { { 0 }, 0, 0 },    //cal point 11
            }, //TX0
            {
                { { 0 }, 0, 0 },    //cal point 0
                { { 0 }, 0, 0 },    //cal point 1
                { { 0 }, 0, 0 },    //cal point 2
                { { 0 }, 0, 0 },    //cal point 3
                { { 0 }, 0, 0 },    //cal point 4
                { { 0 }, 0, 0 },    //cal point 5
                { { 0 }, 0, 0 },    //cal point 6
                { { 0 }, 0, 0 },    //cal point 7
                { { 0 }, 0, 0 },    //cal point 8
                { { 0 }, 0, 0 },    //cal point 9
                { { 0 }, 0, 0 },    //cal point 10
                { { 0 }, 0, 0 },    //cal point 11
            }  //TX1
        }   //empirical
    },  //     CHAN_13,
    {
        0, 0,  //freq, reserved
        { 0, 0 },   //absPower
        {
            {
                { { 0 }, 0, 0 },    //cal point 0
                { { 0 }, 0, 0 },    //cal point 1
                { { 0 }, 0, 0 },    //cal point 2
                { { 0 }, 0, 0 },    //cal point 3
                { { 0 }, 0, 0 },    //cal point 4
                { { 0 }, 0, 0 },    //cal point 5
                { { 0 }, 0, 0 },    //cal point 6
                { { 0 }, 0, 0 },    //cal point 7
                { { 0 }, 0, 0 },    //cal point 8
                { { 0 }, 0, 0 },    //cal point 9
                { { 0 }, 0, 0 },    //cal point 10
                { { 0 }, 0, 0 },    //cal point 11
            }, //TX0
            {
                { { 0 }, 0, 0 },    //cal point 0
                { { 0 }, 0, 0 },    //cal point 1
                { { 0 }, 0, 0 },    //cal point 2
                { { 0 }, 0, 0 },    //cal point 3
                { { 0 }, 0, 0 },    //cal point 4
                { { 0 }, 0, 0 },    //cal point 5
                { { 0 }, 0, 0 },    //cal point 6
                { { 0 }, 0, 0 },    //cal point 7
                { { 0 }, 0, 0 },    //cal point 8
                { { 0 }, 0, 0 },    //cal point 9
                { { 0 }, 0, 0 },    //cal point 10
                { { 0 }, 0, 0 },    //cal point 11
            }  //TX1
        }   //empirical
    }   //     CHAN_14,
};


const tRateGroupPwr       pwrOptimum[NUM_RF_SUBBANDS] =
// EEPROM_TABLE_RATE_POWER_SETTINGS
{
    //typedef tPowerdBm tRateGroupPwr[NUM_HAL_PHY_RATES];

    //tRateGroupPwr       pwrOptimum[NUM_RF_SUBBANDS];

    {
        23,     //HAL_PHY_RATE_11B_LONG_1_MBPS
        23,     //HAL_PHY_RATE_11B_LONG_2_MBPS
        22,     //HAL_PHY_RATE_11B_LONG_5_5_MBPS
        21,     //HAL_PHY_RATE_11B_LONG_11_MBPS
        23,     //HAL_PHY_RATE_11B_SHORT_2_MBPS
        22,     //HAL_PHY_RATE_11B_SHORT_5_5_MBPS
        21,     //HAL_PHY_RATE_11B_SHORT_11_MBPS
        23,     //HAL_PHY_RATE_SSF_SIMO_6_MBPS
        23,     //HAL_PHY_RATE_SSF_SIMO_9_MBPS
        23,     //HAL_PHY_RATE_SSF_SIMO_12_MBPS
        22,     //HAL_PHY_RATE_SSF_SIMO_18_MBPS
        21,     //HAL_PHY_RATE_SSF_SIMO_24_MBPS
        20,     //HAL_PHY_RATE_SSF_SIMO_36_MBPS
        19,     //HAL_PHY_RATE_SSF_SIMO_48_MBPS
        18,     //HAL_PHY_RATE_SSF_SIMO_54_MBPS
        20,     //HAL_PHY_RATE_TITAN_SSF_MIMO_72_MBPS
        19,     //HAL_PHY_RATE_TITAN_SSF_MIMO_96_MBPS
        18,     //HAL_PHY_RATE_TITAN_SSF_MIMO_108_MBPS
        23,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_12_MBPS
        23,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_18_MBPS
        23,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_20_MBPS
        23,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_24_MBPS
        22,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_36_MBPS
        22,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_40_MBPS
        21,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_42_MBPS
        21,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_48_MBPS
        20,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_72_MBPS
        20,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_80_MBPS
        19,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_84_MBPS
        19,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_96_MBPS
        18,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_108_MBPS
        18,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_120_MBPS
        17,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_126_MBPS
        23,     //HAL_PHY_RATE_TITAN_SSF_MIMO_CB_48_MBPS
        22,     //HAL_PHY_RATE_TITAN_SSF_MIMO_CB_72_MBPS
        22,     //HAL_PHY_RATE_TITAN_SSF_MIMO_CB_80_MBPS
        21,     //HAL_PHY_RATE_TITAN_SSF_MIMO_CB_84_MBPS
        21,     //HAL_PHY_RATE_TITAN_SSF_MIMO_CB_96_MBPS
        20,     //HAL_PHY_RATE_TITAN_SSF_MIMO_CB_144_MBPS
        20,     //HAL_PHY_RATE_TITAN_SSF_MIMO_CB_160_MBPS
        19,     //HAL_PHY_RATE_TITAN_SSF_MIMO_CB_168_MBPS
        19,     //HAL_PHY_RATE_TITAN_SSF_MIMO_CB_192_MBPS
        18,     //HAL_PHY_RATE_TITAN_SSF_MIMO_CB_216_MBPS
        18,     //HAL_PHY_RATE_TITAN_SSF_MIMO_CB_240_MBPS
        21,     //HAL_PHY_RATE_TITAN_ESF_MIMO_24_MBPS
        20,     //HAL_PHY_RATE_TITAN_ESF_MIMO_36_MBPS
        22,     //HAL_PHY_RATE_TITAN_ESF_MIMO_40_MBPS
        21,     //HAL_PHY_RATE_TITAN_ESF_MIMO_42_MBPS
        21,     //HAL_PHY_RATE_TITAN_ESF_MIMO_48_MBPS
        20,     //HAL_PHY_RATE_TITAN_ESF_MIMO_72_MBPS
        20,     //HAL_PHY_RATE_TITAN_ESF_MIMO_80_MBPS
        19,     //HAL_PHY_RATE_TITAN_ESF_MIMO_84_MBPS
        19,     //HAL_PHY_RATE_TITAN_ESF_MIMO_96_MBPS
        18,     //HAL_PHY_RATE_TITAN_ESF_MIMO_108_MBPS
        18,     //HAL_PHY_RATE_TITAN_ESF_MIMO_120_MBPS
        17,     //HAL_PHY_RATE_TITAN_ESF_MIMO_126_MBPS
        23,     //HAL_PHY_RATE_TITAN_ESF_SIMO_CB_24_MBPS
        22,     //HAL_PHY_RATE_TITAN_ESF_SIMO_CB_36_MBPS
        22,     //HAL_PHY_RATE_TITAN_ESF_SIMO_CB_40_MBPS
        21,     //HAL_PHY_RATE_TITAN_ESF_SIMO_CB_42_MBPS
        21,     //HAL_PHY_RATE_TITAN_ESF_SIMO_CB_48_MBPS
        20,     //HAL_PHY_RATE_TITAN_ESF_SIMO_CB_72_MBPS
        20,     //HAL_PHY_RATE_TITAN_ESF_SIMO_CB_80_MBPS
        19,     //HAL_PHY_RATE_TITAN_ESF_SIMO_CB_84_MBPS
        19,     //HAL_PHY_RATE_TITAN_ESF_SIMO_CB_96_MBPS
        18,     //HAL_PHY_RATE_TITAN_ESF_SIMO_CB_108_MBPS
        18,     //HAL_PHY_RATE_TITAN_ESF_SIMO_CB_120_MBPS
        17,     //HAL_PHY_RATE_TITAN_ESF_SIMO_CB_126_MBPS
        23,     //HAL_PHY_RATE_TITAN_ESF_MIMO_CB_48_MBPS
        22,     //HAL_PHY_RATE_TITAN_ESF_MIMO_CB_72_MBPS
        22,     //HAL_PHY_RATE_TITAN_ESF_MIMO_CB_80_MBPS
        21,     //HAL_PHY_RATE_TITAN_ESF_MIMO_CB_84_MBPS
        21,     //HAL_PHY_RATE_TITAN_ESF_MIMO_CB_96_MBPS
        20,     //HAL_PHY_RATE_TITAN_ESF_MIMO_CB_144_MBPS
        20,     //HAL_PHY_RATE_TITAN_ESF_MIMO_CB_160_MBPS
        19,     //HAL_PHY_RATE_TITAN_ESF_MIMO_CB_168_MBPS
        19,     //HAL_PHY_RATE_TITAN_ESF_MIMO_CB_192_MBPS
        18,     //HAL_PHY_RATE_TITAN_ESF_MIMO_CB_216_MBPS
        18,     //HAL_PHY_RATE_TITAN_ESF_MIMO_CB_240_MBPS
        23,     //HAL_PHY_RATE_DUP_SSF_SIMO_6_MBPS
        23,     //HAL_PHY_RATE_DUP_SSF_SIMO_9_MBPS
        23,     //HAL_PHY_RATE_DUP_SSF_SIMO_12_MBPS
        22,     //HAL_PHY_RATE_DUP_SSF_SIMO_18_MBPS
        21,     //HAL_PHY_RATE_DUP_SSF_SIMO_24_MBPS
        20,     //HAL_PHY_RATE_DUP_SSF_SIMO_36_MBPS
        19,     //HAL_PHY_RATE_DUP_SSF_SIMO_48_MBPS
        18,     //HAL_PHY_RATE_DUP_SSF_SIMO_54_MBPS
        23,     //HAL_PHY_RATE_MCS_1NSS_6_5_MBPS
        23,     //HAL_PHY_RATE_MCS_1NSS_13_MBPS
        22,     //HAL_PHY_RATE_MCS_1NSS_19_5_MBPS
        21,     //HAL_PHY_RATE_MCS_1NSS_26_MBPS
        20,     //HAL_PHY_RATE_MCS_1NSS_39_MBPS
        19,     //HAL_PHY_RATE_MCS_1NSS_52_MBPS
        18,     //HAL_PHY_RATE_MCS_1NSS_58_5_MBPS
        17,     //HAL_PHY_RATE_MCS_1NSS_65_MBPS
        23,     //HAL_PHY_RATE_MCS_1NSS_SG_7_2_MBPS
        23,     //HAL_PHY_RATE_MCS_1NSS_SG_14_4_MBPS
        22,     //HAL_PHY_RATE_MCS_1NSS_SG_21_7_MBPS
        21,     //HAL_PHY_RATE_MCS_1NSS_SG_28_9_MBPS
        20,     //HAL_PHY_RATE_MCS_1NSS_SG_43_3_MBPS
        19,     //HAL_PHY_RATE_MCS_1NSS_SG_57_8_MBPS
        18,     //HAL_PHY_RATE_MCS_1NSS_SG_65_MBPS
        17,     //HAL_PHY_RATE_MCS_1NSS_SG_72_2_MBPS
        23,     //HAL_PHY_RATE_MCS_2NSS_13_MBPS
        23,     //HAL_PHY_RATE_MCS_2NSS_26_MBPS
        22,     //HAL_PHY_RATE_MCS_2NSS_39_MBPS
        21,     //HAL_PHY_RATE_MCS_2NSS_52_MBPS
        20,     //HAL_PHY_RATE_MCS_2NSS_78_MBPS
        19,     //HAL_PHY_RATE_MCS_2NSS_104_MBPS
        18,     //HAL_PHY_RATE_MCS_2NSS_117_MBPS
        17,     //HAL_PHY_RATE_MCS_2NSS_130_MBPS
        23,     //HAL_PHY_RATE_MCS_2NSS_SG_14_444_MBPS
        23,     //HAL_PHY_RATE_MCS_2NSS_SG_28_889_MBPS
        22,     //HAL_PHY_RATE_MCS_2NSS_SG_43_333_MBPS
        21,     //HAL_PHY_RATE_MCS_2NSS_SG_57_778_MBPS
        20,     //HAL_PHY_RATE_MCS_2NSS_SG_86_667_MBPS
        19,     //HAL_PHY_RATE_MCS_2NSS_SG_115_556_MBPS
        18,     //HAL_PHY_RATE_MCS_2NSS_SG_130_MBPS
        17,     //HAL_PHY_RATE_MCS_2NSS_SG_144_444_MBPS
        23,     //HAL_PHY_RATE_MCS_1NSS_CB_13_5_MBPS
        23,     //HAL_PHY_RATE_MCS_1NSS_CB_27_MBPS
        22,     //HAL_PHY_RATE_MCS_1NSS_CB_40_5_MBPS
        21,     //HAL_PHY_RATE_MCS_1NSS_CB_54_MBPS
        20,     //HAL_PHY_RATE_MCS_1NSS_CB_81_MBPS
        19,     //HAL_PHY_RATE_MCS_1NSS_CB_108_MBPS
        18,     //HAL_PHY_RATE_MCS_1NSS_CB_121_5_MBPS
        18,     //HAL_PHY_RATE_MCS_1NSS_CB_135_MBPS
        23,     //HAL_PHY_RATE_MCS_1NSS_SG_CB_15_MBPS
        23,     //HAL_PHY_RATE_MCS_1NSS_SG_CB_30_MBPS
        22,     //HAL_PHY_RATE_MCS_1NSS_SG_CB_45_MBPS
        21,     //HAL_PHY_RATE_MCS_1NSS_SG_CB_60_MBPS
        20,     //HAL_PHY_RATE_MCS_1NSS_SG_CB_90_MBPS
        19,     //HAL_PHY_RATE_MCS_1NSS_SG_CB_120_MBPS
        17,     //HAL_PHY_RATE_MCS_1NSS_SG_CB_135_MBPS
        17,     //HAL_PHY_RATE_MCS_1NSS_SG_CB_150_MBPS
        23,     //HAL_PHY_RATE_MCS_2NSS_CB_27_MBPS
        23,     //HAL_PHY_RATE_MCS_2NSS_CB_54_MBPS
        22,     //HAL_PHY_RATE_MCS_2NSS_CB_81_MBPS
        21,     //HAL_PHY_RATE_MCS_2NSS_CB_108_MBPS
        20,     //HAL_PHY_RATE_MCS_2NSS_CB_162_MBPS
        19,     //HAL_PHY_RATE_MCS_2NSS_CB_216_MBPS
        18,     //HAL_PHY_RATE_MCS_2NSS_CB_243_MBPS
        18,     //HAL_PHY_RATE_MCS_2NSS_CB_270_MBPS
        23,     //HAL_PHY_RATE_MCS_2NSS_SG_CB_30_MBPS
        23,     //HAL_PHY_RATE_MCS_2NSS_SG_CB_60_MBPS
        22,     //HAL_PHY_RATE_MCS_2NSS_SG_CB_90_MBPS
        21,     //HAL_PHY_RATE_MCS_2NSS_SG_CB_120_MBPS
        20,     //HAL_PHY_RATE_MCS_2NSS_SG_CB_180_MBPS
        19,     //HAL_PHY_RATE_MCS_2NSS_SG_CB_240_MBPS
        18,     //HAL_PHY_RATE_MCS_2NSS_SG_CB_270_MBPS
        17,     //HAL_PHY_RATE_MCS_2NSS_SG_CB_300_MBPS
        23,     //HAL_PHY_RATE_MCS_1NSS_6_MBPS
        23,     //HAL_PHY_RATE_MCS_1NSS_SG_6_7_MBPS
        16,     //HAL_PHY_RATE_NOVA_136_5_MBPS
        16,     //HAL_PHY_RATE_NOVA_151_7_MBPS
        16,     //HAL_PHY_RATE_NOVA_283_5_MBPS
        16,     //HAL_PHY_RATE_NOVA_315_MBPS
        23,     //HAL_PHY_RATE_DUP_11B_LONG_1_MBPS
        23,     //HAL_PHY_RATE_DUP_11B_LONG_2_MBPS
        22,     //HAL_PHY_RATE_DUP_11B_LONG_5_5_MBPS
        21,     //HAL_PHY_RATE_DUP_11B_LONG_11_MBPS
        23,     //HAL_PHY_RATE_DUP_11B_SHORT_2_MBPS
        22,     //HAL_PHY_RATE_DUP_11B_SHORT_5_5_MBPS
        21      //HAL_PHY_RATE_DUP_11B_SHORT_11_MBPS
    },  //    RF_SUBBAND_2_4_GHZ,
    {
        23,     //HAL_PHY_RATE_11B_LONG_1_MBPS
        23,     //HAL_PHY_RATE_11B_LONG_2_MBPS
        22,     //HAL_PHY_RATE_11B_LONG_5_5_MBPS
        21,     //HAL_PHY_RATE_11B_LONG_11_MBPS
        23,     //HAL_PHY_RATE_11B_SHORT_2_MBPS
        22,     //HAL_PHY_RATE_11B_SHORT_5_5_MBPS
        21,     //HAL_PHY_RATE_11B_SHORT_11_MBPS
        23,     //HAL_PHY_RATE_SSF_SIMO_6_MBPS
        23,     //HAL_PHY_RATE_SSF_SIMO_9_MBPS
        23,     //HAL_PHY_RATE_SSF_SIMO_12_MBPS
        22,     //HAL_PHY_RATE_SSF_SIMO_18_MBPS
        21,     //HAL_PHY_RATE_SSF_SIMO_24_MBPS
        20,     //HAL_PHY_RATE_SSF_SIMO_36_MBPS
        19,     //HAL_PHY_RATE_SSF_SIMO_48_MBPS
        18,     //HAL_PHY_RATE_SSF_SIMO_54_MBPS
        20,     //HAL_PHY_RATE_TITAN_SSF_MIMO_72_MBPS
        19,     //HAL_PHY_RATE_TITAN_SSF_MIMO_96_MBPS
        18,     //HAL_PHY_RATE_TITAN_SSF_MIMO_108_MBPS
        23,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_12_MBPS
        23,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_18_MBPS
        23,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_20_MBPS
        23,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_24_MBPS
        22,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_36_MBPS
        22,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_40_MBPS
        21,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_42_MBPS
        21,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_48_MBPS
        20,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_72_MBPS
        20,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_80_MBPS
        19,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_84_MBPS
        19,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_96_MBPS
        18,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_108_MBPS
        18,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_120_MBPS
        17,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_126_MBPS
        23,     //HAL_PHY_RATE_TITAN_SSF_MIMO_CB_48_MBPS
        22,     //HAL_PHY_RATE_TITAN_SSF_MIMO_CB_72_MBPS
        22,     //HAL_PHY_RATE_TITAN_SSF_MIMO_CB_80_MBPS
        21,     //HAL_PHY_RATE_TITAN_SSF_MIMO_CB_84_MBPS
        21,     //HAL_PHY_RATE_TITAN_SSF_MIMO_CB_96_MBPS
        20,     //HAL_PHY_RATE_TITAN_SSF_MIMO_CB_144_MBPS
        20,     //HAL_PHY_RATE_TITAN_SSF_MIMO_CB_160_MBPS
        19,     //HAL_PHY_RATE_TITAN_SSF_MIMO_CB_168_MBPS
        19,     //HAL_PHY_RATE_TITAN_SSF_MIMO_CB_192_MBPS
        18,     //HAL_PHY_RATE_TITAN_SSF_MIMO_CB_216_MBPS
        18,     //HAL_PHY_RATE_TITAN_SSF_MIMO_CB_240_MBPS
        21,     //HAL_PHY_RATE_TITAN_ESF_MIMO_24_MBPS
        20,     //HAL_PHY_RATE_TITAN_ESF_MIMO_36_MBPS
        22,     //HAL_PHY_RATE_TITAN_ESF_MIMO_40_MBPS
        21,     //HAL_PHY_RATE_TITAN_ESF_MIMO_42_MBPS
        21,     //HAL_PHY_RATE_TITAN_ESF_MIMO_48_MBPS
        20,     //HAL_PHY_RATE_TITAN_ESF_MIMO_72_MBPS
        20,     //HAL_PHY_RATE_TITAN_ESF_MIMO_80_MBPS
        19,     //HAL_PHY_RATE_TITAN_ESF_MIMO_84_MBPS
        19,     //HAL_PHY_RATE_TITAN_ESF_MIMO_96_MBPS
        18,     //HAL_PHY_RATE_TITAN_ESF_MIMO_108_MBPS
        18,     //HAL_PHY_RATE_TITAN_ESF_MIMO_120_MBPS
        17,     //HAL_PHY_RATE_TITAN_ESF_MIMO_126_MBPS
        23,     //HAL_PHY_RATE_TITAN_ESF_SIMO_CB_24_MBPS
        22,     //HAL_PHY_RATE_TITAN_ESF_SIMO_CB_36_MBPS
        22,     //HAL_PHY_RATE_TITAN_ESF_SIMO_CB_40_MBPS
        21,     //HAL_PHY_RATE_TITAN_ESF_SIMO_CB_42_MBPS
        21,     //HAL_PHY_RATE_TITAN_ESF_SIMO_CB_48_MBPS
        20,     //HAL_PHY_RATE_TITAN_ESF_SIMO_CB_72_MBPS
        20,     //HAL_PHY_RATE_TITAN_ESF_SIMO_CB_80_MBPS
        19,     //HAL_PHY_RATE_TITAN_ESF_SIMO_CB_84_MBPS
        19,     //HAL_PHY_RATE_TITAN_ESF_SIMO_CB_96_MBPS
        18,     //HAL_PHY_RATE_TITAN_ESF_SIMO_CB_108_MBPS
        18,     //HAL_PHY_RATE_TITAN_ESF_SIMO_CB_120_MBPS
        17,     //HAL_PHY_RATE_TITAN_ESF_SIMO_CB_126_MBPS
        23,     //HAL_PHY_RATE_TITAN_ESF_MIMO_CB_48_MBPS
        22,     //HAL_PHY_RATE_TITAN_ESF_MIMO_CB_72_MBPS
        22,     //HAL_PHY_RATE_TITAN_ESF_MIMO_CB_80_MBPS
        21,     //HAL_PHY_RATE_TITAN_ESF_MIMO_CB_84_MBPS
        21,     //HAL_PHY_RATE_TITAN_ESF_MIMO_CB_96_MBPS
        20,     //HAL_PHY_RATE_TITAN_ESF_MIMO_CB_144_MBPS
        20,     //HAL_PHY_RATE_TITAN_ESF_MIMO_CB_160_MBPS
        19,     //HAL_PHY_RATE_TITAN_ESF_MIMO_CB_168_MBPS
        19,     //HAL_PHY_RATE_TITAN_ESF_MIMO_CB_192_MBPS
        18,     //HAL_PHY_RATE_TITAN_ESF_MIMO_CB_216_MBPS
        18,     //HAL_PHY_RATE_TITAN_ESF_MIMO_CB_240_MBPS
        23,     //HAL_PHY_RATE_DUP_SSF_SIMO_6_MBPS
        23,     //HAL_PHY_RATE_DUP_SSF_SIMO_9_MBPS
        23,     //HAL_PHY_RATE_DUP_SSF_SIMO_12_MBPS
        22,     //HAL_PHY_RATE_DUP_SSF_SIMO_18_MBPS
        21,     //HAL_PHY_RATE_DUP_SSF_SIMO_24_MBPS
        20,     //HAL_PHY_RATE_DUP_SSF_SIMO_36_MBPS
        19,     //HAL_PHY_RATE_DUP_SSF_SIMO_48_MBPS
        18,     //HAL_PHY_RATE_DUP_SSF_SIMO_54_MBPS
        23,     //HAL_PHY_RATE_MCS_1NSS_6_5_MBPS
        23,     //HAL_PHY_RATE_MCS_1NSS_13_MBPS
        22,     //HAL_PHY_RATE_MCS_1NSS_19_5_MBPS
        21,     //HAL_PHY_RATE_MCS_1NSS_26_MBPS
        20,     //HAL_PHY_RATE_MCS_1NSS_39_MBPS
        19,     //HAL_PHY_RATE_MCS_1NSS_52_MBPS
        18,     //HAL_PHY_RATE_MCS_1NSS_58_5_MBPS
        17,     //HAL_PHY_RATE_MCS_1NSS_65_MBPS
        23,     //HAL_PHY_RATE_MCS_1NSS_SG_7_2_MBPS
        23,     //HAL_PHY_RATE_MCS_1NSS_SG_14_4_MBPS
        22,     //HAL_PHY_RATE_MCS_1NSS_SG_21_7_MBPS
        21,     //HAL_PHY_RATE_MCS_1NSS_SG_28_9_MBPS
        20,     //HAL_PHY_RATE_MCS_1NSS_SG_43_3_MBPS
        19,     //HAL_PHY_RATE_MCS_1NSS_SG_57_8_MBPS
        18,     //HAL_PHY_RATE_MCS_1NSS_SG_65_MBPS
        17,     //HAL_PHY_RATE_MCS_1NSS_SG_72_2_MBPS
        23,     //HAL_PHY_RATE_MCS_2NSS_13_MBPS
        23,     //HAL_PHY_RATE_MCS_2NSS_26_MBPS
        22,     //HAL_PHY_RATE_MCS_2NSS_39_MBPS
        21,     //HAL_PHY_RATE_MCS_2NSS_52_MBPS
        20,     //HAL_PHY_RATE_MCS_2NSS_78_MBPS
        19,     //HAL_PHY_RATE_MCS_2NSS_104_MBPS
        18,     //HAL_PHY_RATE_MCS_2NSS_117_MBPS
        17,     //HAL_PHY_RATE_MCS_2NSS_130_MBPS
        23,     //HAL_PHY_RATE_MCS_2NSS_SG_14_444_MBPS
        23,     //HAL_PHY_RATE_MCS_2NSS_SG_28_889_MBPS
        22,     //HAL_PHY_RATE_MCS_2NSS_SG_43_333_MBPS
        21,     //HAL_PHY_RATE_MCS_2NSS_SG_57_778_MBPS
        20,     //HAL_PHY_RATE_MCS_2NSS_SG_86_667_MBPS
        19,     //HAL_PHY_RATE_MCS_2NSS_SG_115_556_MBPS
        18,     //HAL_PHY_RATE_MCS_2NSS_SG_130_MBPS
        17,     //HAL_PHY_RATE_MCS_2NSS_SG_144_444_MBPS
        23,     //HAL_PHY_RATE_MCS_1NSS_CB_13_5_MBPS
        23,     //HAL_PHY_RATE_MCS_1NSS_CB_27_MBPS
        22,     //HAL_PHY_RATE_MCS_1NSS_CB_40_5_MBPS
        21,     //HAL_PHY_RATE_MCS_1NSS_CB_54_MBPS
        20,     //HAL_PHY_RATE_MCS_1NSS_CB_81_MBPS
        19,     //HAL_PHY_RATE_MCS_1NSS_CB_108_MBPS
        18,     //HAL_PHY_RATE_MCS_1NSS_CB_121_5_MBPS
        18,     //HAL_PHY_RATE_MCS_1NSS_CB_135_MBPS
        23,     //HAL_PHY_RATE_MCS_1NSS_SG_CB_15_MBPS
        23,     //HAL_PHY_RATE_MCS_1NSS_SG_CB_30_MBPS
        22,     //HAL_PHY_RATE_MCS_1NSS_SG_CB_45_MBPS
        21,     //HAL_PHY_RATE_MCS_1NSS_SG_CB_60_MBPS
        20,     //HAL_PHY_RATE_MCS_1NSS_SG_CB_90_MBPS
        19,     //HAL_PHY_RATE_MCS_1NSS_SG_CB_120_MBPS
        17,     //HAL_PHY_RATE_MCS_1NSS_SG_CB_135_MBPS
        17,     //HAL_PHY_RATE_MCS_1NSS_SG_CB_150_MBPS
        23,     //HAL_PHY_RATE_MCS_2NSS_CB_27_MBPS
        23,     //HAL_PHY_RATE_MCS_2NSS_CB_54_MBPS
        22,     //HAL_PHY_RATE_MCS_2NSS_CB_81_MBPS
        21,     //HAL_PHY_RATE_MCS_2NSS_CB_108_MBPS
        20,     //HAL_PHY_RATE_MCS_2NSS_CB_162_MBPS
        19,     //HAL_PHY_RATE_MCS_2NSS_CB_216_MBPS
        18,     //HAL_PHY_RATE_MCS_2NSS_CB_243_MBPS
        18,     //HAL_PHY_RATE_MCS_2NSS_CB_270_MBPS
        23,     //HAL_PHY_RATE_MCS_2NSS_SG_CB_30_MBPS
        23,     //HAL_PHY_RATE_MCS_2NSS_SG_CB_60_MBPS
        22,     //HAL_PHY_RATE_MCS_2NSS_SG_CB_90_MBPS
        21,     //HAL_PHY_RATE_MCS_2NSS_SG_CB_120_MBPS
        20,     //HAL_PHY_RATE_MCS_2NSS_SG_CB_180_MBPS
        19,     //HAL_PHY_RATE_MCS_2NSS_SG_CB_240_MBPS
        18,     //HAL_PHY_RATE_MCS_2NSS_SG_CB_270_MBPS
        17,     //HAL_PHY_RATE_MCS_2NSS_SG_CB_300_MBPS
        23,     //HAL_PHY_RATE_MCS_1NSS_6_MBPS
        23,     //HAL_PHY_RATE_MCS_1NSS_SG_6_7_MBPS
        16,     //HAL_PHY_RATE_NOVA_136_5_MBPS
        16,     //HAL_PHY_RATE_NOVA_151_7_MBPS
        16,     //HAL_PHY_RATE_NOVA_283_5_MBPS
        16,     //HAL_PHY_RATE_NOVA_315_MBPS
        23,     //HAL_PHY_RATE_DUP_11B_LONG_1_MBPS
        23,     //HAL_PHY_RATE_DUP_11B_LONG_2_MBPS
        22,     //HAL_PHY_RATE_DUP_11B_LONG_5_5_MBPS
        21,     //HAL_PHY_RATE_DUP_11B_LONG_11_MBPS
        23,     //HAL_PHY_RATE_DUP_11B_SHORT_2_MBPS
        22,     //HAL_PHY_RATE_DUP_11B_SHORT_5_5_MBPS
        21      //HAL_PHY_RATE_DUP_11B_SHORT_11_MBPS
    },  //    RF_SUBBAND_5_LOW_GHZ,
    {
        23,     //HAL_PHY_RATE_11B_LONG_1_MBPS
        23,     //HAL_PHY_RATE_11B_LONG_2_MBPS
        22,     //HAL_PHY_RATE_11B_LONG_5_5_MBPS
        21,     //HAL_PHY_RATE_11B_LONG_11_MBPS
        23,     //HAL_PHY_RATE_11B_SHORT_2_MBPS
        22,     //HAL_PHY_RATE_11B_SHORT_5_5_MBPS
        21,     //HAL_PHY_RATE_11B_SHORT_11_MBPS
        23,     //HAL_PHY_RATE_SSF_SIMO_6_MBPS
        23,     //HAL_PHY_RATE_SSF_SIMO_9_MBPS
        23,     //HAL_PHY_RATE_SSF_SIMO_12_MBPS
        22,     //HAL_PHY_RATE_SSF_SIMO_18_MBPS
        21,     //HAL_PHY_RATE_SSF_SIMO_24_MBPS
        20,     //HAL_PHY_RATE_SSF_SIMO_36_MBPS
        19,     //HAL_PHY_RATE_SSF_SIMO_48_MBPS
        18,     //HAL_PHY_RATE_SSF_SIMO_54_MBPS
        20,     //HAL_PHY_RATE_TITAN_SSF_MIMO_72_MBPS
        19,     //HAL_PHY_RATE_TITAN_SSF_MIMO_96_MBPS
        18,     //HAL_PHY_RATE_TITAN_SSF_MIMO_108_MBPS
        23,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_12_MBPS
        23,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_18_MBPS
        23,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_20_MBPS
        23,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_24_MBPS
        22,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_36_MBPS
        22,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_40_MBPS
        21,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_42_MBPS
        21,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_48_MBPS
        20,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_72_MBPS
        20,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_80_MBPS
        19,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_84_MBPS
        19,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_96_MBPS
        18,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_108_MBPS
        18,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_120_MBPS
        17,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_126_MBPS
        23,     //HAL_PHY_RATE_TITAN_SSF_MIMO_CB_48_MBPS
        22,     //HAL_PHY_RATE_TITAN_SSF_MIMO_CB_72_MBPS
        22,     //HAL_PHY_RATE_TITAN_SSF_MIMO_CB_80_MBPS
        21,     //HAL_PHY_RATE_TITAN_SSF_MIMO_CB_84_MBPS
        21,     //HAL_PHY_RATE_TITAN_SSF_MIMO_CB_96_MBPS
        20,     //HAL_PHY_RATE_TITAN_SSF_MIMO_CB_144_MBPS
        20,     //HAL_PHY_RATE_TITAN_SSF_MIMO_CB_160_MBPS
        19,     //HAL_PHY_RATE_TITAN_SSF_MIMO_CB_168_MBPS
        19,     //HAL_PHY_RATE_TITAN_SSF_MIMO_CB_192_MBPS
        18,     //HAL_PHY_RATE_TITAN_SSF_MIMO_CB_216_MBPS
        18,     //HAL_PHY_RATE_TITAN_SSF_MIMO_CB_240_MBPS
        21,     //HAL_PHY_RATE_TITAN_ESF_MIMO_24_MBPS
        20,     //HAL_PHY_RATE_TITAN_ESF_MIMO_36_MBPS
        22,     //HAL_PHY_RATE_TITAN_ESF_MIMO_40_MBPS
        21,     //HAL_PHY_RATE_TITAN_ESF_MIMO_42_MBPS
        21,     //HAL_PHY_RATE_TITAN_ESF_MIMO_48_MBPS
        20,     //HAL_PHY_RATE_TITAN_ESF_MIMO_72_MBPS
        20,     //HAL_PHY_RATE_TITAN_ESF_MIMO_80_MBPS
        19,     //HAL_PHY_RATE_TITAN_ESF_MIMO_84_MBPS
        19,     //HAL_PHY_RATE_TITAN_ESF_MIMO_96_MBPS
        18,     //HAL_PHY_RATE_TITAN_ESF_MIMO_108_MBPS
        18,     //HAL_PHY_RATE_TITAN_ESF_MIMO_120_MBPS
        17,     //HAL_PHY_RATE_TITAN_ESF_MIMO_126_MBPS
        23,     //HAL_PHY_RATE_TITAN_ESF_SIMO_CB_24_MBPS
        22,     //HAL_PHY_RATE_TITAN_ESF_SIMO_CB_36_MBPS
        22,     //HAL_PHY_RATE_TITAN_ESF_SIMO_CB_40_MBPS
        21,     //HAL_PHY_RATE_TITAN_ESF_SIMO_CB_42_MBPS
        21,     //HAL_PHY_RATE_TITAN_ESF_SIMO_CB_48_MBPS
        20,     //HAL_PHY_RATE_TITAN_ESF_SIMO_CB_72_MBPS
        20,     //HAL_PHY_RATE_TITAN_ESF_SIMO_CB_80_MBPS
        19,     //HAL_PHY_RATE_TITAN_ESF_SIMO_CB_84_MBPS
        19,     //HAL_PHY_RATE_TITAN_ESF_SIMO_CB_96_MBPS
        18,     //HAL_PHY_RATE_TITAN_ESF_SIMO_CB_108_MBPS
        18,     //HAL_PHY_RATE_TITAN_ESF_SIMO_CB_120_MBPS
        17,     //HAL_PHY_RATE_TITAN_ESF_SIMO_CB_126_MBPS
        23,     //HAL_PHY_RATE_TITAN_ESF_MIMO_CB_48_MBPS
        22,     //HAL_PHY_RATE_TITAN_ESF_MIMO_CB_72_MBPS
        22,     //HAL_PHY_RATE_TITAN_ESF_MIMO_CB_80_MBPS
        21,     //HAL_PHY_RATE_TITAN_ESF_MIMO_CB_84_MBPS
        21,     //HAL_PHY_RATE_TITAN_ESF_MIMO_CB_96_MBPS
        20,     //HAL_PHY_RATE_TITAN_ESF_MIMO_CB_144_MBPS
        20,     //HAL_PHY_RATE_TITAN_ESF_MIMO_CB_160_MBPS
        19,     //HAL_PHY_RATE_TITAN_ESF_MIMO_CB_168_MBPS
        19,     //HAL_PHY_RATE_TITAN_ESF_MIMO_CB_192_MBPS
        18,     //HAL_PHY_RATE_TITAN_ESF_MIMO_CB_216_MBPS
        18,     //HAL_PHY_RATE_TITAN_ESF_MIMO_CB_240_MBPS
        23,     //HAL_PHY_RATE_DUP_SSF_SIMO_6_MBPS
        23,     //HAL_PHY_RATE_DUP_SSF_SIMO_9_MBPS
        23,     //HAL_PHY_RATE_DUP_SSF_SIMO_12_MBPS
        22,     //HAL_PHY_RATE_DUP_SSF_SIMO_18_MBPS
        21,     //HAL_PHY_RATE_DUP_SSF_SIMO_24_MBPS
        20,     //HAL_PHY_RATE_DUP_SSF_SIMO_36_MBPS
        19,     //HAL_PHY_RATE_DUP_SSF_SIMO_48_MBPS
        18,     //HAL_PHY_RATE_DUP_SSF_SIMO_54_MBPS
        23,     //HAL_PHY_RATE_MCS_1NSS_6_5_MBPS
        23,     //HAL_PHY_RATE_MCS_1NSS_13_MBPS
        22,     //HAL_PHY_RATE_MCS_1NSS_19_5_MBPS
        21,     //HAL_PHY_RATE_MCS_1NSS_26_MBPS
        20,     //HAL_PHY_RATE_MCS_1NSS_39_MBPS
        19,     //HAL_PHY_RATE_MCS_1NSS_52_MBPS
        18,     //HAL_PHY_RATE_MCS_1NSS_58_5_MBPS
        17,     //HAL_PHY_RATE_MCS_1NSS_65_MBPS
        23,     //HAL_PHY_RATE_MCS_1NSS_SG_7_2_MBPS
        23,     //HAL_PHY_RATE_MCS_1NSS_SG_14_4_MBPS
        22,     //HAL_PHY_RATE_MCS_1NSS_SG_21_7_MBPS
        21,     //HAL_PHY_RATE_MCS_1NSS_SG_28_9_MBPS
        20,     //HAL_PHY_RATE_MCS_1NSS_SG_43_3_MBPS
        19,     //HAL_PHY_RATE_MCS_1NSS_SG_57_8_MBPS
        18,     //HAL_PHY_RATE_MCS_1NSS_SG_65_MBPS
        17,     //HAL_PHY_RATE_MCS_1NSS_SG_72_2_MBPS
        23,     //HAL_PHY_RATE_MCS_2NSS_13_MBPS
        23,     //HAL_PHY_RATE_MCS_2NSS_26_MBPS
        22,     //HAL_PHY_RATE_MCS_2NSS_39_MBPS
        21,     //HAL_PHY_RATE_MCS_2NSS_52_MBPS
        20,     //HAL_PHY_RATE_MCS_2NSS_78_MBPS
        19,     //HAL_PHY_RATE_MCS_2NSS_104_MBPS
        18,     //HAL_PHY_RATE_MCS_2NSS_117_MBPS
        17,     //HAL_PHY_RATE_MCS_2NSS_130_MBPS
        23,     //HAL_PHY_RATE_MCS_2NSS_SG_14_444_MBPS
        23,     //HAL_PHY_RATE_MCS_2NSS_SG_28_889_MBPS
        22,     //HAL_PHY_RATE_MCS_2NSS_SG_43_333_MBPS
        21,     //HAL_PHY_RATE_MCS_2NSS_SG_57_778_MBPS
        20,     //HAL_PHY_RATE_MCS_2NSS_SG_86_667_MBPS
        19,     //HAL_PHY_RATE_MCS_2NSS_SG_115_556_MBPS
        18,     //HAL_PHY_RATE_MCS_2NSS_SG_130_MBPS
        17,     //HAL_PHY_RATE_MCS_2NSS_SG_144_444_MBPS
        23,     //HAL_PHY_RATE_MCS_1NSS_CB_13_5_MBPS
        23,     //HAL_PHY_RATE_MCS_1NSS_CB_27_MBPS
        22,     //HAL_PHY_RATE_MCS_1NSS_CB_40_5_MBPS
        21,     //HAL_PHY_RATE_MCS_1NSS_CB_54_MBPS
        20,     //HAL_PHY_RATE_MCS_1NSS_CB_81_MBPS
        19,     //HAL_PHY_RATE_MCS_1NSS_CB_108_MBPS
        18,     //HAL_PHY_RATE_MCS_1NSS_CB_121_5_MBPS
        18,     //HAL_PHY_RATE_MCS_1NSS_CB_135_MBPS
        23,     //HAL_PHY_RATE_MCS_1NSS_SG_CB_15_MBPS
        23,     //HAL_PHY_RATE_MCS_1NSS_SG_CB_30_MBPS
        22,     //HAL_PHY_RATE_MCS_1NSS_SG_CB_45_MBPS
        21,     //HAL_PHY_RATE_MCS_1NSS_SG_CB_60_MBPS
        20,     //HAL_PHY_RATE_MCS_1NSS_SG_CB_90_MBPS
        19,     //HAL_PHY_RATE_MCS_1NSS_SG_CB_120_MBPS
        17,     //HAL_PHY_RATE_MCS_1NSS_SG_CB_135_MBPS
        17,     //HAL_PHY_RATE_MCS_1NSS_SG_CB_150_MBPS
        23,     //HAL_PHY_RATE_MCS_2NSS_CB_27_MBPS
        23,     //HAL_PHY_RATE_MCS_2NSS_CB_54_MBPS
        22,     //HAL_PHY_RATE_MCS_2NSS_CB_81_MBPS
        21,     //HAL_PHY_RATE_MCS_2NSS_CB_108_MBPS
        20,     //HAL_PHY_RATE_MCS_2NSS_CB_162_MBPS
        19,     //HAL_PHY_RATE_MCS_2NSS_CB_216_MBPS
        18,     //HAL_PHY_RATE_MCS_2NSS_CB_243_MBPS
        18,     //HAL_PHY_RATE_MCS_2NSS_CB_270_MBPS
        23,     //HAL_PHY_RATE_MCS_2NSS_SG_CB_30_MBPS
        23,     //HAL_PHY_RATE_MCS_2NSS_SG_CB_60_MBPS
        22,     //HAL_PHY_RATE_MCS_2NSS_SG_CB_90_MBPS
        21,     //HAL_PHY_RATE_MCS_2NSS_SG_CB_120_MBPS
        20,     //HAL_PHY_RATE_MCS_2NSS_SG_CB_180_MBPS
        19,     //HAL_PHY_RATE_MCS_2NSS_SG_CB_240_MBPS
        18,     //HAL_PHY_RATE_MCS_2NSS_SG_CB_270_MBPS
        17,     //HAL_PHY_RATE_MCS_2NSS_SG_CB_300_MBPS
        23,     //HAL_PHY_RATE_MCS_1NSS_6_MBPS
        23,     //HAL_PHY_RATE_MCS_1NSS_SG_6_7_MBPS
        16,     //HAL_PHY_RATE_NOVA_136_5_MBPS
        16,     //HAL_PHY_RATE_NOVA_151_7_MBPS
        16,     //HAL_PHY_RATE_NOVA_283_5_MBPS
        16,     //HAL_PHY_RATE_NOVA_315_MBPS
        23,     //HAL_PHY_RATE_DUP_11B_LONG_1_MBPS
        23,     //HAL_PHY_RATE_DUP_11B_LONG_2_MBPS
        22,     //HAL_PHY_RATE_DUP_11B_LONG_5_5_MBPS
        21,     //HAL_PHY_RATE_DUP_11B_LONG_11_MBPS
        23,     //HAL_PHY_RATE_DUP_11B_SHORT_2_MBPS
        22,     //HAL_PHY_RATE_DUP_11B_SHORT_5_5_MBPS
        21      //HAL_PHY_RATE_DUP_11B_SHORT_11_MBPS
    },  //    RF_SUBBAND_5_MID_GHZ,
    {
        23,     //HAL_PHY_RATE_11B_LONG_1_MBPS
        23,     //HAL_PHY_RATE_11B_LONG_2_MBPS
        22,     //HAL_PHY_RATE_11B_LONG_5_5_MBPS
        21,     //HAL_PHY_RATE_11B_LONG_11_MBPS
        23,     //HAL_PHY_RATE_11B_SHORT_2_MBPS
        22,     //HAL_PHY_RATE_11B_SHORT_5_5_MBPS
        21,     //HAL_PHY_RATE_11B_SHORT_11_MBPS
        23,     //HAL_PHY_RATE_SSF_SIMO_6_MBPS
        23,     //HAL_PHY_RATE_SSF_SIMO_9_MBPS
        23,     //HAL_PHY_RATE_SSF_SIMO_12_MBPS
        22,     //HAL_PHY_RATE_SSF_SIMO_18_MBPS
        21,     //HAL_PHY_RATE_SSF_SIMO_24_MBPS
        20,     //HAL_PHY_RATE_SSF_SIMO_36_MBPS
        19,     //HAL_PHY_RATE_SSF_SIMO_48_MBPS
        18,     //HAL_PHY_RATE_SSF_SIMO_54_MBPS
        20,     //HAL_PHY_RATE_TITAN_SSF_MIMO_72_MBPS
        19,     //HAL_PHY_RATE_TITAN_SSF_MIMO_96_MBPS
        18,     //HAL_PHY_RATE_TITAN_SSF_MIMO_108_MBPS
        23,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_12_MBPS
        23,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_18_MBPS
        23,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_20_MBPS
        23,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_24_MBPS
        22,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_36_MBPS
        22,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_40_MBPS
        21,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_42_MBPS
        21,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_48_MBPS
        20,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_72_MBPS
        20,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_80_MBPS
        19,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_84_MBPS
        19,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_96_MBPS
        18,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_108_MBPS
        18,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_120_MBPS
        17,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_126_MBPS
        23,     //HAL_PHY_RATE_TITAN_SSF_MIMO_CB_48_MBPS
        22,     //HAL_PHY_RATE_TITAN_SSF_MIMO_CB_72_MBPS
        22,     //HAL_PHY_RATE_TITAN_SSF_MIMO_CB_80_MBPS
        21,     //HAL_PHY_RATE_TITAN_SSF_MIMO_CB_84_MBPS
        21,     //HAL_PHY_RATE_TITAN_SSF_MIMO_CB_96_MBPS
        20,     //HAL_PHY_RATE_TITAN_SSF_MIMO_CB_144_MBPS
        20,     //HAL_PHY_RATE_TITAN_SSF_MIMO_CB_160_MBPS
        19,     //HAL_PHY_RATE_TITAN_SSF_MIMO_CB_168_MBPS
        19,     //HAL_PHY_RATE_TITAN_SSF_MIMO_CB_192_MBPS
        18,     //HAL_PHY_RATE_TITAN_SSF_MIMO_CB_216_MBPS
        18,     //HAL_PHY_RATE_TITAN_SSF_MIMO_CB_240_MBPS
        21,     //HAL_PHY_RATE_TITAN_ESF_MIMO_24_MBPS
        20,     //HAL_PHY_RATE_TITAN_ESF_MIMO_36_MBPS
        22,     //HAL_PHY_RATE_TITAN_ESF_MIMO_40_MBPS
        21,     //HAL_PHY_RATE_TITAN_ESF_MIMO_42_MBPS
        21,     //HAL_PHY_RATE_TITAN_ESF_MIMO_48_MBPS
        20,     //HAL_PHY_RATE_TITAN_ESF_MIMO_72_MBPS
        20,     //HAL_PHY_RATE_TITAN_ESF_MIMO_80_MBPS
        19,     //HAL_PHY_RATE_TITAN_ESF_MIMO_84_MBPS
        19,     //HAL_PHY_RATE_TITAN_ESF_MIMO_96_MBPS
        18,     //HAL_PHY_RATE_TITAN_ESF_MIMO_108_MBPS
        18,     //HAL_PHY_RATE_TITAN_ESF_MIMO_120_MBPS
        17,     //HAL_PHY_RATE_TITAN_ESF_MIMO_126_MBPS
        23,     //HAL_PHY_RATE_TITAN_ESF_SIMO_CB_24_MBPS
        22,     //HAL_PHY_RATE_TITAN_ESF_SIMO_CB_36_MBPS
        22,     //HAL_PHY_RATE_TITAN_ESF_SIMO_CB_40_MBPS
        21,     //HAL_PHY_RATE_TITAN_ESF_SIMO_CB_42_MBPS
        21,     //HAL_PHY_RATE_TITAN_ESF_SIMO_CB_48_MBPS
        20,     //HAL_PHY_RATE_TITAN_ESF_SIMO_CB_72_MBPS
        20,     //HAL_PHY_RATE_TITAN_ESF_SIMO_CB_80_MBPS
        19,     //HAL_PHY_RATE_TITAN_ESF_SIMO_CB_84_MBPS
        19,     //HAL_PHY_RATE_TITAN_ESF_SIMO_CB_96_MBPS
        18,     //HAL_PHY_RATE_TITAN_ESF_SIMO_CB_108_MBPS
        18,     //HAL_PHY_RATE_TITAN_ESF_SIMO_CB_120_MBPS
        17,     //HAL_PHY_RATE_TITAN_ESF_SIMO_CB_126_MBPS
        23,     //HAL_PHY_RATE_TITAN_ESF_MIMO_CB_48_MBPS
        22,     //HAL_PHY_RATE_TITAN_ESF_MIMO_CB_72_MBPS
        22,     //HAL_PHY_RATE_TITAN_ESF_MIMO_CB_80_MBPS
        21,     //HAL_PHY_RATE_TITAN_ESF_MIMO_CB_84_MBPS
        21,     //HAL_PHY_RATE_TITAN_ESF_MIMO_CB_96_MBPS
        20,     //HAL_PHY_RATE_TITAN_ESF_MIMO_CB_144_MBPS
        20,     //HAL_PHY_RATE_TITAN_ESF_MIMO_CB_160_MBPS
        19,     //HAL_PHY_RATE_TITAN_ESF_MIMO_CB_168_MBPS
        19,     //HAL_PHY_RATE_TITAN_ESF_MIMO_CB_192_MBPS
        18,     //HAL_PHY_RATE_TITAN_ESF_MIMO_CB_216_MBPS
        18,     //HAL_PHY_RATE_TITAN_ESF_MIMO_CB_240_MBPS
        23,     //HAL_PHY_RATE_DUP_SSF_SIMO_6_MBPS
        23,     //HAL_PHY_RATE_DUP_SSF_SIMO_9_MBPS
        23,     //HAL_PHY_RATE_DUP_SSF_SIMO_12_MBPS
        22,     //HAL_PHY_RATE_DUP_SSF_SIMO_18_MBPS
        21,     //HAL_PHY_RATE_DUP_SSF_SIMO_24_MBPS
        20,     //HAL_PHY_RATE_DUP_SSF_SIMO_36_MBPS
        19,     //HAL_PHY_RATE_DUP_SSF_SIMO_48_MBPS
        18,     //HAL_PHY_RATE_DUP_SSF_SIMO_54_MBPS
        23,     //HAL_PHY_RATE_MCS_1NSS_6_5_MBPS
        23,     //HAL_PHY_RATE_MCS_1NSS_13_MBPS
        22,     //HAL_PHY_RATE_MCS_1NSS_19_5_MBPS
        21,     //HAL_PHY_RATE_MCS_1NSS_26_MBPS
        20,     //HAL_PHY_RATE_MCS_1NSS_39_MBPS
        19,     //HAL_PHY_RATE_MCS_1NSS_52_MBPS
        18,     //HAL_PHY_RATE_MCS_1NSS_58_5_MBPS
        17,     //HAL_PHY_RATE_MCS_1NSS_65_MBPS
        23,     //HAL_PHY_RATE_MCS_1NSS_SG_7_2_MBPS
        23,     //HAL_PHY_RATE_MCS_1NSS_SG_14_4_MBPS
        22,     //HAL_PHY_RATE_MCS_1NSS_SG_21_7_MBPS
        21,     //HAL_PHY_RATE_MCS_1NSS_SG_28_9_MBPS
        20,     //HAL_PHY_RATE_MCS_1NSS_SG_43_3_MBPS
        19,     //HAL_PHY_RATE_MCS_1NSS_SG_57_8_MBPS
        18,     //HAL_PHY_RATE_MCS_1NSS_SG_65_MBPS
        17,     //HAL_PHY_RATE_MCS_1NSS_SG_72_2_MBPS
        23,     //HAL_PHY_RATE_MCS_2NSS_13_MBPS
        23,     //HAL_PHY_RATE_MCS_2NSS_26_MBPS
        22,     //HAL_PHY_RATE_MCS_2NSS_39_MBPS
        21,     //HAL_PHY_RATE_MCS_2NSS_52_MBPS
        20,     //HAL_PHY_RATE_MCS_2NSS_78_MBPS
        19,     //HAL_PHY_RATE_MCS_2NSS_104_MBPS
        18,     //HAL_PHY_RATE_MCS_2NSS_117_MBPS
        17,     //HAL_PHY_RATE_MCS_2NSS_130_MBPS
        23,     //HAL_PHY_RATE_MCS_2NSS_SG_14_444_MBPS
        23,     //HAL_PHY_RATE_MCS_2NSS_SG_28_889_MBPS
        22,     //HAL_PHY_RATE_MCS_2NSS_SG_43_333_MBPS
        21,     //HAL_PHY_RATE_MCS_2NSS_SG_57_778_MBPS
        20,     //HAL_PHY_RATE_MCS_2NSS_SG_86_667_MBPS
        19,     //HAL_PHY_RATE_MCS_2NSS_SG_115_556_MBPS
        18,     //HAL_PHY_RATE_MCS_2NSS_SG_130_MBPS
        17,     //HAL_PHY_RATE_MCS_2NSS_SG_144_444_MBPS
        23,     //HAL_PHY_RATE_MCS_1NSS_CB_13_5_MBPS
        23,     //HAL_PHY_RATE_MCS_1NSS_CB_27_MBPS
        22,     //HAL_PHY_RATE_MCS_1NSS_CB_40_5_MBPS
        21,     //HAL_PHY_RATE_MCS_1NSS_CB_54_MBPS
        20,     //HAL_PHY_RATE_MCS_1NSS_CB_81_MBPS
        19,     //HAL_PHY_RATE_MCS_1NSS_CB_108_MBPS
        18,     //HAL_PHY_RATE_MCS_1NSS_CB_121_5_MBPS
        18,     //HAL_PHY_RATE_MCS_1NSS_CB_135_MBPS
        23,     //HAL_PHY_RATE_MCS_1NSS_SG_CB_15_MBPS
        23,     //HAL_PHY_RATE_MCS_1NSS_SG_CB_30_MBPS
        22,     //HAL_PHY_RATE_MCS_1NSS_SG_CB_45_MBPS
        21,     //HAL_PHY_RATE_MCS_1NSS_SG_CB_60_MBPS
        20,     //HAL_PHY_RATE_MCS_1NSS_SG_CB_90_MBPS
        19,     //HAL_PHY_RATE_MCS_1NSS_SG_CB_120_MBPS
        17,     //HAL_PHY_RATE_MCS_1NSS_SG_CB_135_MBPS
        17,     //HAL_PHY_RATE_MCS_1NSS_SG_CB_150_MBPS
        23,     //HAL_PHY_RATE_MCS_2NSS_CB_27_MBPS
        23,     //HAL_PHY_RATE_MCS_2NSS_CB_54_MBPS
        22,     //HAL_PHY_RATE_MCS_2NSS_CB_81_MBPS
        21,     //HAL_PHY_RATE_MCS_2NSS_CB_108_MBPS
        20,     //HAL_PHY_RATE_MCS_2NSS_CB_162_MBPS
        19,     //HAL_PHY_RATE_MCS_2NSS_CB_216_MBPS
        18,     //HAL_PHY_RATE_MCS_2NSS_CB_243_MBPS
        18,     //HAL_PHY_RATE_MCS_2NSS_CB_270_MBPS
        23,     //HAL_PHY_RATE_MCS_2NSS_SG_CB_30_MBPS
        23,     //HAL_PHY_RATE_MCS_2NSS_SG_CB_60_MBPS
        22,     //HAL_PHY_RATE_MCS_2NSS_SG_CB_90_MBPS
        21,     //HAL_PHY_RATE_MCS_2NSS_SG_CB_120_MBPS
        20,     //HAL_PHY_RATE_MCS_2NSS_SG_CB_180_MBPS
        19,     //HAL_PHY_RATE_MCS_2NSS_SG_CB_240_MBPS
        18,     //HAL_PHY_RATE_MCS_2NSS_SG_CB_270_MBPS
        17,     //HAL_PHY_RATE_MCS_2NSS_SG_CB_300_MBPS
        23,     //HAL_PHY_RATE_MCS_1NSS_6_MBPS
        23,     //HAL_PHY_RATE_MCS_1NSS_SG_6_7_MBPS
        16,     //HAL_PHY_RATE_NOVA_136_5_MBPS
        16,     //HAL_PHY_RATE_NOVA_151_7_MBPS
        16,     //HAL_PHY_RATE_NOVA_283_5_MBPS
        16,     //HAL_PHY_RATE_NOVA_315_MBPS
        23,     //HAL_PHY_RATE_DUP_11B_LONG_1_MBPS
        23,     //HAL_PHY_RATE_DUP_11B_LONG_2_MBPS
        22,     //HAL_PHY_RATE_DUP_11B_LONG_5_5_MBPS
        21,     //HAL_PHY_RATE_DUP_11B_LONG_11_MBPS
        23,     //HAL_PHY_RATE_DUP_11B_SHORT_2_MBPS
        22,     //HAL_PHY_RATE_DUP_11B_SHORT_5_5_MBPS
        21      //HAL_PHY_RATE_DUP_11B_SHORT_11_MBPS
    },  //    RF_SUBBAND_5_HIGH_GHZ,
    {
        23,     //HAL_PHY_RATE_11B_LONG_1_MBPS
        23,     //HAL_PHY_RATE_11B_LONG_2_MBPS
        22,     //HAL_PHY_RATE_11B_LONG_5_5_MBPS
        21,     //HAL_PHY_RATE_11B_LONG_11_MBPS
        23,     //HAL_PHY_RATE_11B_SHORT_2_MBPS
        22,     //HAL_PHY_RATE_11B_SHORT_5_5_MBPS
        21,     //HAL_PHY_RATE_11B_SHORT_11_MBPS
        23,     //HAL_PHY_RATE_SSF_SIMO_6_MBPS
        23,     //HAL_PHY_RATE_SSF_SIMO_9_MBPS
        23,     //HAL_PHY_RATE_SSF_SIMO_12_MBPS
        22,     //HAL_PHY_RATE_SSF_SIMO_18_MBPS
        21,     //HAL_PHY_RATE_SSF_SIMO_24_MBPS
        20,     //HAL_PHY_RATE_SSF_SIMO_36_MBPS
        19,     //HAL_PHY_RATE_SSF_SIMO_48_MBPS
        18,     //HAL_PHY_RATE_SSF_SIMO_54_MBPS
        20,     //HAL_PHY_RATE_TITAN_SSF_MIMO_72_MBPS
        19,     //HAL_PHY_RATE_TITAN_SSF_MIMO_96_MBPS
        18,     //HAL_PHY_RATE_TITAN_SSF_MIMO_108_MBPS
        23,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_12_MBPS
        23,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_18_MBPS
        23,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_20_MBPS
        23,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_24_MBPS
        22,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_36_MBPS
        22,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_40_MBPS
        21,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_42_MBPS
        21,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_48_MBPS
        20,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_72_MBPS
        20,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_80_MBPS
        19,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_84_MBPS
        19,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_96_MBPS
        18,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_108_MBPS
        18,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_120_MBPS
        17,     //HAL_PHY_RATE_TITAN_SSF_SIMO_CB_126_MBPS
        23,     //HAL_PHY_RATE_TITAN_SSF_MIMO_CB_48_MBPS
        22,     //HAL_PHY_RATE_TITAN_SSF_MIMO_CB_72_MBPS
        22,     //HAL_PHY_RATE_TITAN_SSF_MIMO_CB_80_MBPS
        21,     //HAL_PHY_RATE_TITAN_SSF_MIMO_CB_84_MBPS
        21,     //HAL_PHY_RATE_TITAN_SSF_MIMO_CB_96_MBPS
        20,     //HAL_PHY_RATE_TITAN_SSF_MIMO_CB_144_MBPS
        20,     //HAL_PHY_RATE_TITAN_SSF_MIMO_CB_160_MBPS
        19,     //HAL_PHY_RATE_TITAN_SSF_MIMO_CB_168_MBPS
        19,     //HAL_PHY_RATE_TITAN_SSF_MIMO_CB_192_MBPS
        18,     //HAL_PHY_RATE_TITAN_SSF_MIMO_CB_216_MBPS
        18,     //HAL_PHY_RATE_TITAN_SSF_MIMO_CB_240_MBPS
        21,     //HAL_PHY_RATE_TITAN_ESF_MIMO_24_MBPS
        20,     //HAL_PHY_RATE_TITAN_ESF_MIMO_36_MBPS
        22,     //HAL_PHY_RATE_TITAN_ESF_MIMO_40_MBPS
        21,     //HAL_PHY_RATE_TITAN_ESF_MIMO_42_MBPS
        21,     //HAL_PHY_RATE_TITAN_ESF_MIMO_48_MBPS
        20,     //HAL_PHY_RATE_TITAN_ESF_MIMO_72_MBPS
        20,     //HAL_PHY_RATE_TITAN_ESF_MIMO_80_MBPS
        19,     //HAL_PHY_RATE_TITAN_ESF_MIMO_84_MBPS
        19,     //HAL_PHY_RATE_TITAN_ESF_MIMO_96_MBPS
        18,     //HAL_PHY_RATE_TITAN_ESF_MIMO_108_MBPS
        18,     //HAL_PHY_RATE_TITAN_ESF_MIMO_120_MBPS
        17,     //HAL_PHY_RATE_TITAN_ESF_MIMO_126_MBPS
        23,     //HAL_PHY_RATE_TITAN_ESF_SIMO_CB_24_MBPS
        22,     //HAL_PHY_RATE_TITAN_ESF_SIMO_CB_36_MBPS
        22,     //HAL_PHY_RATE_TITAN_ESF_SIMO_CB_40_MBPS
        21,     //HAL_PHY_RATE_TITAN_ESF_SIMO_CB_42_MBPS
        21,     //HAL_PHY_RATE_TITAN_ESF_SIMO_CB_48_MBPS
        20,     //HAL_PHY_RATE_TITAN_ESF_SIMO_CB_72_MBPS
        20,     //HAL_PHY_RATE_TITAN_ESF_SIMO_CB_80_MBPS
        19,     //HAL_PHY_RATE_TITAN_ESF_SIMO_CB_84_MBPS
        19,     //HAL_PHY_RATE_TITAN_ESF_SIMO_CB_96_MBPS
        18,     //HAL_PHY_RATE_TITAN_ESF_SIMO_CB_108_MBPS
        18,     //HAL_PHY_RATE_TITAN_ESF_SIMO_CB_120_MBPS
        17,     //HAL_PHY_RATE_TITAN_ESF_SIMO_CB_126_MBPS
        23,     //HAL_PHY_RATE_TITAN_ESF_MIMO_CB_48_MBPS
        22,     //HAL_PHY_RATE_TITAN_ESF_MIMO_CB_72_MBPS
        22,     //HAL_PHY_RATE_TITAN_ESF_MIMO_CB_80_MBPS
        21,     //HAL_PHY_RATE_TITAN_ESF_MIMO_CB_84_MBPS
        21,     //HAL_PHY_RATE_TITAN_ESF_MIMO_CB_96_MBPS
        20,     //HAL_PHY_RATE_TITAN_ESF_MIMO_CB_144_MBPS
        20,     //HAL_PHY_RATE_TITAN_ESF_MIMO_CB_160_MBPS
        19,     //HAL_PHY_RATE_TITAN_ESF_MIMO_CB_168_MBPS
        19,     //HAL_PHY_RATE_TITAN_ESF_MIMO_CB_192_MBPS
        18,     //HAL_PHY_RATE_TITAN_ESF_MIMO_CB_216_MBPS
        18,     //HAL_PHY_RATE_TITAN_ESF_MIMO_CB_240_MBPS
        23,     //HAL_PHY_RATE_DUP_SSF_SIMO_6_MBPS
        23,     //HAL_PHY_RATE_DUP_SSF_SIMO_9_MBPS
        23,     //HAL_PHY_RATE_DUP_SSF_SIMO_12_MBPS
        22,     //HAL_PHY_RATE_DUP_SSF_SIMO_18_MBPS
        21,     //HAL_PHY_RATE_DUP_SSF_SIMO_24_MBPS
        20,     //HAL_PHY_RATE_DUP_SSF_SIMO_36_MBPS
        19,     //HAL_PHY_RATE_DUP_SSF_SIMO_48_MBPS
        18,     //HAL_PHY_RATE_DUP_SSF_SIMO_54_MBPS
        23,     //HAL_PHY_RATE_MCS_1NSS_6_5_MBPS
        23,     //HAL_PHY_RATE_MCS_1NSS_13_MBPS
        22,     //HAL_PHY_RATE_MCS_1NSS_19_5_MBPS
        21,     //HAL_PHY_RATE_MCS_1NSS_26_MBPS
        20,     //HAL_PHY_RATE_MCS_1NSS_39_MBPS
        19,     //HAL_PHY_RATE_MCS_1NSS_52_MBPS
        18,     //HAL_PHY_RATE_MCS_1NSS_58_5_MBPS
        17,     //HAL_PHY_RATE_MCS_1NSS_65_MBPS
        23,     //HAL_PHY_RATE_MCS_1NSS_SG_7_2_MBPS
        23,     //HAL_PHY_RATE_MCS_1NSS_SG_14_4_MBPS
        22,     //HAL_PHY_RATE_MCS_1NSS_SG_21_7_MBPS
        21,     //HAL_PHY_RATE_MCS_1NSS_SG_28_9_MBPS
        20,     //HAL_PHY_RATE_MCS_1NSS_SG_43_3_MBPS
        19,     //HAL_PHY_RATE_MCS_1NSS_SG_57_8_MBPS
        18,     //HAL_PHY_RATE_MCS_1NSS_SG_65_MBPS
        17,     //HAL_PHY_RATE_MCS_1NSS_SG_72_2_MBPS
        23,     //HAL_PHY_RATE_MCS_2NSS_13_MBPS
        23,     //HAL_PHY_RATE_MCS_2NSS_26_MBPS
        22,     //HAL_PHY_RATE_MCS_2NSS_39_MBPS
        21,     //HAL_PHY_RATE_MCS_2NSS_52_MBPS
        20,     //HAL_PHY_RATE_MCS_2NSS_78_MBPS
        19,     //HAL_PHY_RATE_MCS_2NSS_104_MBPS
        18,     //HAL_PHY_RATE_MCS_2NSS_117_MBPS
        17,     //HAL_PHY_RATE_MCS_2NSS_130_MBPS
        23,     //HAL_PHY_RATE_MCS_2NSS_SG_14_444_MBPS
        23,     //HAL_PHY_RATE_MCS_2NSS_SG_28_889_MBPS
        22,     //HAL_PHY_RATE_MCS_2NSS_SG_43_333_MBPS
        21,     //HAL_PHY_RATE_MCS_2NSS_SG_57_778_MBPS
        20,     //HAL_PHY_RATE_MCS_2NSS_SG_86_667_MBPS
        19,     //HAL_PHY_RATE_MCS_2NSS_SG_115_556_MBPS
        18,     //HAL_PHY_RATE_MCS_2NSS_SG_130_MBPS
        17,     //HAL_PHY_RATE_MCS_2NSS_SG_144_444_MBPS
        23,     //HAL_PHY_RATE_MCS_1NSS_CB_13_5_MBPS
        23,     //HAL_PHY_RATE_MCS_1NSS_CB_27_MBPS
        22,     //HAL_PHY_RATE_MCS_1NSS_CB_40_5_MBPS
        21,     //HAL_PHY_RATE_MCS_1NSS_CB_54_MBPS
        20,     //HAL_PHY_RATE_MCS_1NSS_CB_81_MBPS
        19,     //HAL_PHY_RATE_MCS_1NSS_CB_108_MBPS
        18,     //HAL_PHY_RATE_MCS_1NSS_CB_121_5_MBPS
        18,     //HAL_PHY_RATE_MCS_1NSS_CB_135_MBPS
        23,     //HAL_PHY_RATE_MCS_1NSS_SG_CB_15_MBPS
        23,     //HAL_PHY_RATE_MCS_1NSS_SG_CB_30_MBPS
        22,     //HAL_PHY_RATE_MCS_1NSS_SG_CB_45_MBPS
        21,     //HAL_PHY_RATE_MCS_1NSS_SG_CB_60_MBPS
        20,     //HAL_PHY_RATE_MCS_1NSS_SG_CB_90_MBPS
        19,     //HAL_PHY_RATE_MCS_1NSS_SG_CB_120_MBPS
        17,     //HAL_PHY_RATE_MCS_1NSS_SG_CB_135_MBPS
        17,     //HAL_PHY_RATE_MCS_1NSS_SG_CB_150_MBPS
        23,     //HAL_PHY_RATE_MCS_2NSS_CB_27_MBPS
        23,     //HAL_PHY_RATE_MCS_2NSS_CB_54_MBPS
        22,     //HAL_PHY_RATE_MCS_2NSS_CB_81_MBPS
        21,     //HAL_PHY_RATE_MCS_2NSS_CB_108_MBPS
        20,     //HAL_PHY_RATE_MCS_2NSS_CB_162_MBPS
        19,     //HAL_PHY_RATE_MCS_2NSS_CB_216_MBPS
        18,     //HAL_PHY_RATE_MCS_2NSS_CB_243_MBPS
        18,     //HAL_PHY_RATE_MCS_2NSS_CB_270_MBPS
        23,     //HAL_PHY_RATE_MCS_2NSS_SG_CB_30_MBPS
        23,     //HAL_PHY_RATE_MCS_2NSS_SG_CB_60_MBPS
        22,     //HAL_PHY_RATE_MCS_2NSS_SG_CB_90_MBPS
        21,     //HAL_PHY_RATE_MCS_2NSS_SG_CB_120_MBPS
        20,     //HAL_PHY_RATE_MCS_2NSS_SG_CB_180_MBPS
        19,     //HAL_PHY_RATE_MCS_2NSS_SG_CB_240_MBPS
        18,     //HAL_PHY_RATE_MCS_2NSS_SG_CB_270_MBPS
        17,     //HAL_PHY_RATE_MCS_2NSS_SG_CB_300_MBPS
        23,     //HAL_PHY_RATE_MCS_1NSS_6_MBPS
        23,     //HAL_PHY_RATE_MCS_1NSS_SG_6_7_MBPS
        16,     //HAL_PHY_RATE_NOVA_136_5_MBPS
        16,     //HAL_PHY_RATE_NOVA_151_7_MBPS
        16,     //HAL_PHY_RATE_NOVA_283_5_MBPS
        16,     //HAL_PHY_RATE_NOVA_315_MBPS
        23,     //HAL_PHY_RATE_DUP_11B_LONG_1_MBPS
        23,     //HAL_PHY_RATE_DUP_11B_LONG_2_MBPS
        22,     //HAL_PHY_RATE_DUP_11B_LONG_5_5_MBPS
        21,     //HAL_PHY_RATE_DUP_11B_LONG_11_MBPS
        23,     //HAL_PHY_RATE_DUP_11B_SHORT_2_MBPS
        22,     //HAL_PHY_RATE_DUP_11B_SHORT_5_5_MBPS
        21      //HAL_PHY_RATE_DUP_11B_SHORT_11_MBPS
    }   //    RF_SUBBAND_4_9_GHZ,
};


const tANI_U16            demoFreqs[NUM_DEMO_CAL_CHANNELS] =
{
    2412,                    // DEMO_CAL_FREQ_0,
    2484,                    // DEMO_CAL_FREQ_1,
    4920,                    // DEMO_CAL_FREQ_2,
    5080,                    // DEMO_CAL_FREQ_3,
    5170,                    // DEMO_CAL_FREQ_4,
    5320,                    // DEMO_CAL_FREQ_5,
    5500,                    // DEMO_CAL_FREQ_6,
    5700,                    // DEMO_CAL_FREQ_7,
    5745,                    // DEMO_CAL_FREQ_8,
    5805,                    // DEMO_CAL_FREQ_9,
    INVALID_RF_CHANNEL,      // DEMO_CAL_FREQ_10,
    INVALID_RF_CHANNEL       // DEMO_CAL_FREQ_11,
};

const sInitCalValues initCalValueTable =
{

    // typedef struct
    // {
    //     tANI_U8 quasarLbGains;   //bit 0 = tx_lb_gain, bit 1 = rx_lb_gain
    //     tANI_U8 rxGain;
    // }sTxLoCalClipGains;

    // typedef struct
    // {
    //     tANI_U8 rfDetGain;
    //     tANI_U8 rxGain;
    // }sTxIqCalClipGains;

    // typedef struct
    // {
    //     tANI_U8 quasarLbGains;      //bit 0 = tx_lb_gain, bit 1 = rx_lb_gain
    //     tANI_U8 txGain;             //override tx gain value = coarse & fine
    // }sRxIqCalClipGains;

    // typedef struct
    // {
    //     tANI_S9 center;
    //     tANI_S9 offCenter;
    //     tANI_S9 imbalance;
    // }sIQCalValues;

    // typedef struct
    // {
    //     tANI_S6 iLo;
    //     tANI_S6 qLo;
    // }tTxLoCorrect;

    // typedef tTxLoCorrect sTxLoCorrectChannel[PHY_MAX_TX_CHAINS][NUM_TX_GAIN_STEPS];
    // typedef sIQCalValues sTxIQChannel[PHY_MAX_TX_CHAINS][NUM_TX_GAIN_STEPS];
    // typedef sIQCalValues sRxIQChannel[PHY_MAX_RX_CHAINS][NUM_RX_GAIN_STEPS];

    // typedef tANI_U8 tDcoCorrect;

    // typedef struct
    // {
    //     tDcoCorrect IDcoCorrect;
    //     tDcoCorrect QDcoCorrect;
    //     tANI_U8     dcRange;
    // }tRxDcoCorrect;

    // typedef tRxDcoCorrect tRxDcoMatrix[PHY_MAX_RX_CHAINS][NUM_QUASAR_RX_GAIN_STEPS];

    // typedef struct
    // {
    //     eRfChannels calChannel;
    //     sTxLoCalClipGains txLoClipGains[PHY_MAX_TX_CHAINS];
    //     sTxIqCalClipGains txIqClipGains[PHY_MAX_TX_CHAINS];
    //     sRxIqCalClipGains rxIqClipGains[PHY_MAX_RX_CHAINS][NUM_RX_GAIN_STEPS];
    //     tANI_BOOLEAN useTxIqValues;
    //     tANI_BOOLEAN useRxIqValues;
    //     tANI_BOOLEAN useTxLoValues;
    //     tANI_BOOLEAN useRxDcoValues;
    //     sTxIQChannel            txIQ;
    //     sRxIQChannel            rxIQ;
    //     sTxLoCorrectChannel     txLoCorrect;
    //     tRxDcoMatrix            rxDco;
    //     tTempADCVal calTemp;
    // }sInitCalValues;
    RF_CHAN_1,  //      eRfChannels curChannel
    { { 0, 0 }, { 0, 0 } }, //     sTxLoCalClipGains txLoClipGains[PHY_MAX_TX_CHAINS];
    { { 0, 0 }, { 0, 0 } }, //     sTxIqCalClipGains txIqClipGains[PHY_MAX_TX_CHAINS];
    {
        {   //quasarLbGains, txGain
            { 0, 0 },   // RX_GAIN_STEP_0
            { 0, 0 },   // RX_GAIN_STEP_1,
            { 0, 0 },   // RX_GAIN_STEP_2,
            { 0, 0 },   // RX_GAIN_STEP_3,
            { 0, 0 },   // RX_GAIN_STEP_4,
            { 0, 0 },   // RX_GAIN_STEP_5,
            { 0, 0 },   // RX_GAIN_STEP_6,
            { 0, 0 },   // RX_GAIN_STEP_7,
            { 0, 0 },   // RX_GAIN_STEP_8,
            { 0, 0 },   // RX_GAIN_STEP_9,
            { 0, 0 },   // RX_GAIN_STEP_10,
            { 0, 0 },   // RX_GAIN_STEP_11,
            { 0, 0 },   // RX_GAIN_STEP_12,
            { 0, 0 },   // RX_GAIN_STEP_13,
            { 0, 0 },   // RX_GAIN_STEP_14,
            { 0, 0 }    // RX_GAIN_STEP_15,
        },  // PHY_RX_CHAIN_0

        {   //quasarLbGains, txGain
            { 0, 0 },   // RX_GAIN_STEP_0
            { 0, 0 },   // RX_GAIN_STEP_1,
            { 0, 0 },   // RX_GAIN_STEP_2,
            { 0, 0 },   // RX_GAIN_STEP_3,
            { 0, 0 },   // RX_GAIN_STEP_4,
            { 0, 0 },   // RX_GAIN_STEP_5,
            { 0, 0 },   // RX_GAIN_STEP_6,
            { 0, 0 },   // RX_GAIN_STEP_7,
            { 0, 0 },   // RX_GAIN_STEP_8,
            { 0, 0 },   // RX_GAIN_STEP_9,
            { 0, 0 },   // RX_GAIN_STEP_10,
            { 0, 0 },   // RX_GAIN_STEP_11,
            { 0, 0 },   // RX_GAIN_STEP_12,
            { 0, 0 },   // RX_GAIN_STEP_13,
            { 0, 0 },   // RX_GAIN_STEP_14,
            { 0, 0 }    // RX_GAIN_STEP_15,
        },  // PHY_RX_CHAIN_1

        {   //quasarLbGains, txGain
            { 0, 0 },   // RX_GAIN_STEP_0
            { 0, 0 },   // RX_GAIN_STEP_1,
            { 0, 0 },   // RX_GAIN_STEP_2,
            { 0, 0 },   // RX_GAIN_STEP_3,
            { 0, 0 },   // RX_GAIN_STEP_4,
            { 0, 0 },   // RX_GAIN_STEP_5,
            { 0, 0 },   // RX_GAIN_STEP_6,
            { 0, 0 },   // RX_GAIN_STEP_7,
            { 0, 0 },   // RX_GAIN_STEP_8,
            { 0, 0 },   // RX_GAIN_STEP_9,
            { 0, 0 },   // RX_GAIN_STEP_10,
            { 0, 0 },   // RX_GAIN_STEP_11,
            { 0, 0 },   // RX_GAIN_STEP_12,
            { 0, 0 },   // RX_GAIN_STEP_13,
            { 0, 0 },   // RX_GAIN_STEP_14,
            { 0, 0 }    // RX_GAIN_STEP_15,
        }  // PHY_RX_CHAIN_2
    }, //     sRxIqCalClipGains rxIqClipGains[PHY_MAX_RX_CHAINS][NUM_RX_GAIN_STEPS];

    eANI_BOOLEAN_FALSE, //     tANI_BOOLEAN useTxIqValues;
    eANI_BOOLEAN_FALSE, //     tANI_BOOLEAN useRxIqValues;
    eANI_BOOLEAN_FALSE, //     tANI_BOOLEAN useTxLoValues;
    eANI_BOOLEAN_FALSE, //     tANI_BOOLEAN useRxDcoValues;
    {
        {   //center, offCenter, imbalance
            { 0, 0, 0 },   // TX_GAIN_STEP_0
            { 0, 0, 0 },   // TX_GAIN_STEP_1,
            { 0, 0, 0 },   // TX_GAIN_STEP_2,
            { 0, 0, 0 },   // TX_GAIN_STEP_3,
            { 0, 0, 0 },   // TX_GAIN_STEP_4,
            { 0, 0, 0 },   // TX_GAIN_STEP_5,
            { 0, 0, 0 },   // TX_GAIN_STEP_6,
            { 0, 0, 0 },   // TX_GAIN_STEP_7,
            { 0, 0, 0 },   // TX_GAIN_STEP_8,
            { 0, 0, 0 },   // TX_GAIN_STEP_9,
            { 0, 0, 0 },   // TX_GAIN_STEP_10,
            { 0, 0, 0 },   // TX_GAIN_STEP_11,
            { 0, 0, 0 },   // TX_GAIN_STEP_12,
            { 0, 0, 0 },   // TX_GAIN_STEP_13,
            { 0, 0, 0 },   // TX_GAIN_STEP_14,
            { 0, 0, 0 }    // TX_GAIN_STEP_15,
        },  // PHY_TX_CHAIN_0

        {   //center, offCenter, imbalance
            { 0, 0, 0 },   // TX_GAIN_STEP_0
            { 0, 0, 0 },   // TX_GAIN_STEP_1,
            { 0, 0, 0 },   // TX_GAIN_STEP_2,
            { 0, 0, 0 },   // TX_GAIN_STEP_3,
            { 0, 0, 0 },   // TX_GAIN_STEP_4,
            { 0, 0, 0 },   // TX_GAIN_STEP_5,
            { 0, 0, 0 },   // TX_GAIN_STEP_6,
            { 0, 0, 0 },   // TX_GAIN_STEP_7,
            { 0, 0, 0 },   // TX_GAIN_STEP_8,
            { 0, 0, 0 },   // TX_GAIN_STEP_9,
            { 0, 0, 0 },   // TX_GAIN_STEP_10,
            { 0, 0, 0 },   // TX_GAIN_STEP_11,
            { 0, 0, 0 },   // TX_GAIN_STEP_12,
            { 0, 0, 0 },   // TX_GAIN_STEP_13,
            { 0, 0, 0 },   // TX_GAIN_STEP_14,
            { 0, 0, 0 }    // TX_GAIN_STEP_15,
        }  // PHY_TX_CHAIN_1
    }, //     sTxIQChannel            txIQ;
    {
        {   //center, offCenter, imbalance
            { 0, 0, 0 },   // RX_GAIN_STEP_0
            { 0, 0, 0 },   // RX_GAIN_STEP_1,
            { 0, 0, 0 },   // RX_GAIN_STEP_2,
            { 0, 0, 0 },   // RX_GAIN_STEP_3,
            { 0, 0, 0 },   // RX_GAIN_STEP_4,
            { 0, 0, 0 },   // RX_GAIN_STEP_5,
            { 0, 0, 0 },   // RX_GAIN_STEP_6,
            { 0, 0, 0 },   // RX_GAIN_STEP_7,
            { 0, 0, 0 },   // RX_GAIN_STEP_8,
            { 0, 0, 0 },   // RX_GAIN_STEP_9,
            { 0, 0, 0 },   // RX_GAIN_STEP_10,
            { 0, 0, 0 },   // RX_GAIN_STEP_11,
            { 0, 0, 0 },   // RX_GAIN_STEP_12,
            { 0, 0, 0 },   // RX_GAIN_STEP_13,
            { 0, 0, 0 },   // RX_GAIN_STEP_14,
            { 0, 0, 0 }    // RX_GAIN_STEP_15,
        },  // PHY_RX_CHAIN_0

        {   //center, offCenter, imbalance
            { 0, 0, 0 },   // RX_GAIN_STEP_0
            { 0, 0, 0 },   // RX_GAIN_STEP_1,
            { 0, 0, 0 },   // RX_GAIN_STEP_2,
            { 0, 0, 0 },   // RX_GAIN_STEP_3,
            { 0, 0, 0 },   // RX_GAIN_STEP_4,
            { 0, 0, 0 },   // RX_GAIN_STEP_5,
            { 0, 0, 0 },   // RX_GAIN_STEP_6,
            { 0, 0, 0 },   // RX_GAIN_STEP_7,
            { 0, 0, 0 },   // RX_GAIN_STEP_8,
            { 0, 0, 0 },   // RX_GAIN_STEP_9,
            { 0, 0, 0 },   // RX_GAIN_STEP_10,
            { 0, 0, 0 },   // RX_GAIN_STEP_11,
            { 0, 0, 0 },   // RX_GAIN_STEP_12,
            { 0, 0, 0 },   // RX_GAIN_STEP_13,
            { 0, 0, 0 },   // RX_GAIN_STEP_14,
            { 0, 0, 0 }    // RX_GAIN_STEP_15,
        },  // PHY_RX_CHAIN_1

        {   //center, offCenter, imbalance
            { 0, 0, 0 },   // RX_GAIN_STEP_0
            { 0, 0, 0 },   // RX_GAIN_STEP_1,
            { 0, 0, 0 },   // RX_GAIN_STEP_2,
            { 0, 0, 0 },   // RX_GAIN_STEP_3,
            { 0, 0, 0 },   // RX_GAIN_STEP_4,
            { 0, 0, 0 },   // RX_GAIN_STEP_5,
            { 0, 0, 0 },   // RX_GAIN_STEP_6,
            { 0, 0, 0 },   // RX_GAIN_STEP_7,
            { 0, 0, 0 },   // RX_GAIN_STEP_8,
            { 0, 0, 0 },   // RX_GAIN_STEP_9,
            { 0, 0, 0 },   // RX_GAIN_STEP_10,
            { 0, 0, 0 },   // RX_GAIN_STEP_11,
            { 0, 0, 0 },   // RX_GAIN_STEP_12,
            { 0, 0, 0 },   // RX_GAIN_STEP_13,
            { 0, 0, 0 },   // RX_GAIN_STEP_14,
            { 0, 0, 0 }    // RX_GAIN_STEP_15,
        }  // PHY_RX_CHAIN_2
    }, //     sRxIQChannel            rxIQ;
    {
        {   //iLo, qLo
            { 0, 0 },   // TX_GAIN_STEP_0
            { 0, 0 },   // TX_GAIN_STEP_1,
            { 0, 0 },   // TX_GAIN_STEP_2,
            { 0, 0 },   // TX_GAIN_STEP_3,
            { 0, 0 },   // TX_GAIN_STEP_4,
            { 0, 0 },   // TX_GAIN_STEP_5,
            { 0, 0 },   // TX_GAIN_STEP_6,
            { 0, 0 },   // TX_GAIN_STEP_7,
            { 0, 0 },   // TX_GAIN_STEP_8,
            { 0, 0 },   // TX_GAIN_STEP_9,
            { 0, 0 },   // TX_GAIN_STEP_10,
            { 0, 0 },   // TX_GAIN_STEP_11,
            { 0, 0 },   // TX_GAIN_STEP_12,
            { 0, 0 },   // TX_GAIN_STEP_13,
            { 0, 0 },   // TX_GAIN_STEP_14,
            { 0, 0 }    // TX_GAIN_STEP_15,
        },  // PHY_TX_CHAIN_0

        {   //iLo, qLo
            { 0, 0 },   // TX_GAIN_STEP_0
            { 0, 0 },   // TX_GAIN_STEP_1,
            { 0, 0 },   // TX_GAIN_STEP_2,
            { 0, 0 },   // TX_GAIN_STEP_3,
            { 0, 0 },   // TX_GAIN_STEP_4,
            { 0, 0 },   // TX_GAIN_STEP_5,
            { 0, 0 },   // TX_GAIN_STEP_6,
            { 0, 0 },   // TX_GAIN_STEP_7,
            { 0, 0 },   // TX_GAIN_STEP_8,
            { 0, 0 },   // TX_GAIN_STEP_9,
            { 0, 0 },   // TX_GAIN_STEP_10,
            { 0, 0 },   // TX_GAIN_STEP_11,
            { 0, 0 },   // TX_GAIN_STEP_12,
            { 0, 0 },   // TX_GAIN_STEP_13,
            { 0, 0 },   // TX_GAIN_STEP_14,
            { 0, 0 }    // TX_GAIN_STEP_15,
        }  // PHY_TX_CHAIN_1
    }, //     sTxLoCorrectChannel     txLoCorrect;
    {
        { // IDcoCorrect, QDcoCorrect, dcRange
            { 0, 0, 0 },    //0
            { 0, 0, 0 },    //1
            { 0, 0, 0 },    //2
            { 0, 0, 0 },    //3
            { 0, 0, 0 },    //4
            { 0, 0, 0 },    //5
            { 0, 0, 0 },    //6
            { 0, 0, 0 },    //7
            { 0, 0, 0 },    //8
            { 0, 0, 0 },    //9
            { 0, 0, 0 },    //10
            { 0, 0, 0 },    //11
            { 0, 0, 0 },    //12
            { 0, 0, 0 },    //13
            { 0, 0, 0 },    //14
            { 0, 0, 0 },    //15
            { 0, 0, 0 },    //16
            { 0, 0, 0 },    //17
            { 0, 0, 0 },    //18
            { 0, 0, 0 },    //19
            { 0, 0, 0 },    //20
            { 0, 0, 0 },    //21
            { 0, 0, 0 },    //22
            { 0, 0, 0 },    //23
            { 0, 0, 0 },    //24
            { 0, 0, 0 },    //25
            { 0, 0, 0 },    //26
            { 0, 0, 0 },    //27
            { 0, 0, 0 },    //28
            { 0, 0, 0 },    //29
            { 0, 0, 0 },    //30
            { 0, 0, 0 },    //31
            { 0, 0, 0 },    //32
            { 0, 0, 0 },    //33
            { 0, 0, 0 },    //34
            { 0, 0, 0 },    //35
            { 0, 0, 0 },    //36
            { 0, 0, 0 },    //37
            { 0, 0, 0 },    //38
            { 0, 0, 0 },    //39
            { 0, 0, 0 },    //40
            { 0, 0, 0 },    //41
            { 0, 0, 0 },    //42
            { 0, 0, 0 },    //43
            { 0, 0, 0 }     //44
        }, //PHY_RX_CHAIN_0

        { // IDcoCorrect, QDcoCorrect, dcRange
            { 0, 0, 0 },    //0
            { 0, 0, 0 },    //1
            { 0, 0, 0 },    //2
            { 0, 0, 0 },    //3
            { 0, 0, 0 },    //4
            { 0, 0, 0 },    //5
            { 0, 0, 0 },    //6
            { 0, 0, 0 },    //7
            { 0, 0, 0 },    //8
            { 0, 0, 0 },    //9
            { 0, 0, 0 },    //10
            { 0, 0, 0 },    //11
            { 0, 0, 0 },    //12
            { 0, 0, 0 },    //13
            { 0, 0, 0 },    //14
            { 0, 0, 0 },    //15
            { 0, 0, 0 },    //16
            { 0, 0, 0 },    //17
            { 0, 0, 0 },    //18
            { 0, 0, 0 },    //19
            { 0, 0, 0 },    //20
            { 0, 0, 0 },    //21
            { 0, 0, 0 },    //22
            { 0, 0, 0 },    //23
            { 0, 0, 0 },    //24
            { 0, 0, 0 },    //25
            { 0, 0, 0 },    //26
            { 0, 0, 0 },    //27
            { 0, 0, 0 },    //28
            { 0, 0, 0 },    //29
            { 0, 0, 0 },    //30
            { 0, 0, 0 },    //31
            { 0, 0, 0 },    //32
            { 0, 0, 0 },    //33
            { 0, 0, 0 },    //34
            { 0, 0, 0 },    //35
            { 0, 0, 0 },    //36
            { 0, 0, 0 },    //37
            { 0, 0, 0 },    //38
            { 0, 0, 0 },    //39
            { 0, 0, 0 },    //40
            { 0, 0, 0 },    //41
            { 0, 0, 0 },    //42
            { 0, 0, 0 },    //43
            { 0, 0, 0 }     //44
        }, //PHY_RX_CHAIN_1

        { // IDcoCorrect, QDcoCorrect, dcRange
            { 0, 0, 0 },    //0
            { 0, 0, 0 },    //1
            { 0, 0, 0 },    //2
            { 0, 0, 0 },    //3
            { 0, 0, 0 },    //4
            { 0, 0, 0 },    //5
            { 0, 0, 0 },    //6
            { 0, 0, 0 },    //7
            { 0, 0, 0 },    //8
            { 0, 0, 0 },    //9
            { 0, 0, 0 },    //10
            { 0, 0, 0 },    //11
            { 0, 0, 0 },    //12
            { 0, 0, 0 },    //13
            { 0, 0, 0 },    //14
            { 0, 0, 0 },    //15
            { 0, 0, 0 },    //16
            { 0, 0, 0 },    //17
            { 0, 0, 0 },    //18
            { 0, 0, 0 },    //19
            { 0, 0, 0 },    //20
            { 0, 0, 0 },    //21
            { 0, 0, 0 },    //22
            { 0, 0, 0 },    //23
            { 0, 0, 0 },    //24
            { 0, 0, 0 },    //25
            { 0, 0, 0 },    //26
            { 0, 0, 0 },    //27
            { 0, 0, 0 },    //28
            { 0, 0, 0 },    //29
            { 0, 0, 0 },    //30
            { 0, 0, 0 },    //31
            { 0, 0, 0 },    //32
            { 0, 0, 0 },    //33
            { 0, 0, 0 },    //34
            { 0, 0, 0 },    //35
            { 0, 0, 0 },    //36
            { 0, 0, 0 },    //37
            { 0, 0, 0 },    //38
            { 0, 0, 0 },    //39
            { 0, 0, 0 },    //40
            { 0, 0, 0 },    //41
            { 0, 0, 0 },    //42
            { 0, 0, 0 },    //43
            { 0, 0, 0 }     //44
        } //PHY_RX_CHAIN_2
    }, //     tRxDcoMatrix            rxDco;
    0  // tTempADCVal
};


#ifdef ANI_PHY_DEBUG
#include "aniGlobal.h"
#include "phyDebug.h"

void DisplayLnaSwGainTable(tHalHandle hMac, sLnaSwGainTable *tableData)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hMac;
    tANI_U32 nFreqs;

    nFreqs = tableData->nFreqs;

    if (nFreqs < MAX_LNA_SW_FREQS)
    {
        tANI_U32 i;

        phyLog(pMac,LOGE, "LNA Sw nFreqs = %d\n", nFreqs);

        for (i = 0; i < nFreqs; i++)
        {
            phyLog(pMac,LOGE, "%04d MHz: \tRx0{R+LNA=%03d Alt=%03d} \tRx1{R+LNA=%03d Alt=%03d} \tRx2{R+LNA=%03d Alt=%03d}\n",
                        tableData->chan[i].freq,
                        tableData->chan[i].swGains[PHY_RX_CHAIN_0].rxLnaOnGain,
                        tableData->chan[i].swGains[PHY_RX_CHAIN_0].alternateGain,
                        tableData->chan[i].swGains[PHY_RX_CHAIN_1].rxLnaOnGain,
                        tableData->chan[i].swGains[PHY_RX_CHAIN_1].alternateGain,
                        tableData->chan[i].swGains[PHY_RX_CHAIN_2].rxLnaOnGain,
                        tableData->chan[i].swGains[PHY_RX_CHAIN_2].alternateGain
                  );
        }
    }
    else
    {
        phyLog(pMac,LOGE, "ERROR: LNA nFreqs = %d, Out of range\n", nFreqs);
    }
}



void DisplayCalTable(tHalHandle hMac, sInitCalValues *tableData)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hMac;
    tANI_U32 i, chain;

    phyLog(pMac, LOGE, "CAL TABLE DATA:\n");
    phyLog(pMac, LOGE, "Cal Channel = %d\n", tableData->calChannel);
    phyLog(pMac, LOGE, "Cal Temp = %d\n", tableData->calTemp);


    phyLog(pMac, LOGE, "txtxLoClipGainsIQ:\n");
    for (chain = 0; chain < PHY_MAX_TX_CHAINS; chain++)
    {
        phyLog(pMac, LOGE, "\tTX%d \n", chain);

        phyLog(pMac, LOGE, "\t\ttxLb = %d \trxLb = %d \trxGain = %d\n",
               tableData->txLoClipGains[chain].quasarLbGains & BIT_0,
               (tableData->txLoClipGains[chain].quasarLbGains & BIT_1) >> 1,
               tableData->txLoClipGains[chain].rxGain
              );
    }

    phyLog(pMac, LOGE, "txIqClipGains:\n");
    for (chain = 0; chain < PHY_MAX_TX_CHAINS; chain++)
    {
        phyLog(pMac, LOGE, "\tTX%d \n", chain);

        phyLog(pMac, LOGE, "\t\trfDetGain = %d \trxGain = %d\n",
               tableData->txIqClipGains[chain].rfDetGain,
               tableData->txIqClipGains[chain].rxGain
              );
    }

    phyLog(pMac, LOGE, "rxIqClipGains:\n");
    for (chain = 0; chain < PHY_MAX_RX_CHAINS; chain++)
    {
        phyLog(pMac, LOGE, "\tRX%d \n", chain);

        for (i = 0; i < NUM_RX_GAIN_STEPS; i++)
        {
            phyLog(pMac, LOGE, "\t\ttxLb = %d \trxLb = %d \ttxGain = %d\n",
                   tableData->rxIqClipGains[chain][i].quasarLbGains & BIT_0,
                   (tableData->rxIqClipGains[chain][i].quasarLbGains & BIT_1) >> 1,
                   tableData->rxIqClipGains[chain][i].txGain
                  );
        }
    }

    phyLog(pMac, LOGE, "txIQ:\n");
    for (chain = 0; chain < PHY_MAX_TX_CHAINS; chain++)
    {
        phyLog(pMac, LOGE, "\tTX%d \n", chain);

        for (i = 0; i < NUM_TX_GAIN_STEPS; i++)
        {
            phyLog(pMac, LOGE, "\t\tcenter = 0x%X \toffCenter = 0x%X \timbalance = 0x%X\n",
                   tableData->txIQ[chain][i].center,
                   tableData->txIQ[chain][i].offCenter,
                   tableData->txIQ[chain][i].imbalance
                  );
        }
    }

    phyLog(pMac, LOGE, "rxIQ:\n");
    for (chain = 0; chain < PHY_MAX_RX_CHAINS; chain++)
    {
        phyLog(pMac, LOGE, "\tRX%d \n", chain);

        for (i = 0; i < NUM_RX_GAIN_STEPS; i++)
        {
            phyLog(pMac, LOGE, "\t\tcenter = 0x%X \toffCenter = 0x%X \timbalance = 0x%X\n",
                   tableData->rxIQ[chain][i].center,
                   tableData->rxIQ[chain][i].offCenter,
                   tableData->rxIQ[chain][i].imbalance
                  );
        }
    }

    phyLog(pMac, LOGE, "txLoCorrect:\n");
    for (chain = 0; chain < PHY_MAX_TX_CHAINS; chain++)
    {
        phyLog(pMac, LOGE, "\tTX%d \n", chain);

        for (i = 0; i < NUM_TX_GAIN_STEPS; i++)
        {
            phyLog(pMac, LOGE, "\t\tiLo = 0x%X \tqLo = 0x%X\n",
                   tableData->txLoCorrect[chain][i].iLo,
                   tableData->txLoCorrect[chain][i].qLo
                  );
        }
    }


    phyLog(pMac, LOGE, "rxDco:\n");
    for (chain = 0; chain < PHY_MAX_RX_CHAINS; chain++)
    {
        phyLog(pMac, LOGE, "\tRX%d \n", chain);

        for (i = 0; i < NUM_RX_GAIN_STEPS; i++)
        {
            phyLog(pMac, LOGE, "\t\tIDcoCorrect = 0x%X \tQDcoCorrect = 0x%X \tdcRange = 0x%X\n",
                   tableData->rxDco[chain][i].IDcoCorrect,
                   tableData->rxDco[chain][i].QDcoCorrect,
                   tableData->rxDco[chain][i].dcRange
                  );
        }
    }
}


#endif

#endif //GEN6_OBSOLETE
#endif


/**
 *
   Airgo Networks, Inc proprietary.
   All Rights Reserved, Copyright 2005
   This program is the confidential and proprietary product of Airgo Networks Inc.
   Any Unauthorized use, reproduction or transfer of this program is strictly prohibited.


   halPhyCfg.h: Contains types common to configuration and halPhy or exclusive to configuration.

   Author:  Mark Nelson
   Date:    4/6/05

   History -
   Date        Modified by              Modification Information
  --------------------------------------------------------------------------

 */

#ifndef HALPHYCFG_H
#define HALPHYCFG_H

#include <asicTPC.h>
#include <halRfTypes.h>

//Tx Power Config
//A collection of selected calibrated power points at selected frequencies.
//The algorithm does not need to know any particulars about which frequencies or cal points,
// just the linearized adjustments at the selected calibration points
#define MAX_TPC_CAL_POINTS      (4)
#define MAX_TPC_CHANNELS        (2)
#define START_TPC_CHANNEL       (2422)
#define END_TPC_CHANNEL         (2452)




#define MAX_PWR_LUT_DBM (18)
#define MIN_PWR_LUT_DBM (2)


/* The reason that MAX_PWR_LUT_DBM_2DEC_PLACES is not simply (MAX_PWR_LUT_DBM * 100) is due to the fact
    that we are interpolating the 5-bit power template index from this range compared to a LUT range of 0 to 127.
    There is an expectation that this power range is evenly divided in 0.5dBm steps.
    We expect that a commanded 13dBm would yield a power template index of 10, where a power template index of 0 would represent 8dBm.
    If we used an even 2400 to represent the max power, then the calculation for 13dBm actually returns 9:
    (127 - 0)*((1300 - 800)/(2400 - 800))+0 = 39.6875 = 39. When shifted to 5 bits, =9. Not what we wanted.
    What we need to do is find the 2-decimal place power that corresponds as closely as possible to the 127 in the 0 to 127 range.
    For the 800 to 2400 range, that comes out to 2386.5, so 2386. So again for a commanded power of 13dBm:
    (127 - 0)*((1300 - 800)/(2386 - 800))+0 = 40.0378 = 40. When shifted to 5-bits, = 10, which is what we wanted.

*/

#define MIN_PWR_LUT_DBM_2DEC_PLACES  (MIN_PWR_LUT_DBM * 100)
#define MAX_PWR_LUT_DBM_2DEC_PLACES  ((MAX_PWR_LUT_DBM * 100) - (1 + (100 * (MAX_PWR_LUT_DBM - MIN_PWR_LUT_DBM))/TPC_MEM_POWER_LUT_DEPTH))



typedef tANI_U8 tPowerDetect;        //7-bit power detect reading
typedef tANI_U8 tTxGainCombo;        //7-bit gain value used to get the power measurement
typedef tANI_U16 tTpcLutValue;       //full precision value used for internal LUT calculations - only divided to 7-bits when loading tables.


typedef struct
{
    tPowerDetect min;
    tPowerDetect max;
}tPwrTemplateRange;



/*
    The following union affords backward compatibility with txGain usage with band-specific tTpcConfig tables.
    Due to my finding that 7-bits is not enough precision, we need to reuse the txGain space as extra precision bits
    for the adjustedPwrDet. My spreadsheet shows that we need at least 4 bits more precision.
    To know which usage, the MSB of adjustedPwrDet can be set to signify the extra precision in place of the txGain, which isn't used anyway.
    We just need to be careful not to interpret a pre-existing table's txGain as extra precision.
*/

// typedef union
// {
//     tTxGainCombo txGain;               //7-bit gain used to measure the pwrDetAdc value
//     tANI_U8 hi8_adjustedPwrDet;        //if the MSB is set in adjustedPwrDet, then these are extra bits of precision
// }uExtraLutBits;


typedef struct
{
    tPowerDetect pwrDetAdc;            //= SENSED_PWR register, which reports the 8-bit ADC
                                       // the stored ADC value gets shifted to 7-bits as the index to the LUT
    tPowerDetect adjustedPwrDet;       //7-bit value that goes into the LUT at the LUT[pwrDet] location
                                       //MSB set if extraPrecision.hi8_adjustedPwrDet is used
}tTpcCaldPowerPoint;

typedef tTpcCaldPowerPoint tTpcCaldPowerTable[PHY_MAX_TX_CHAINS][MAX_TPC_CAL_POINTS];

/*
// #define EXTRA_TPC_LUT_BITS 4            //number of extra precision bits preserved when storing the adjustedPwrDet values
// #define EXTRA_TPC_LUT_MULT 16           // =2^EXTRA_TPC_LUT_BITS
// 
// //macro returns adjusted LUT value at 7 + EXTRA_TPC_LUT_BITS bits of precision
// //for pre-existing tables where the MSB of adjustedPwrDet is not set, then we simply multiply the adjustedPwrDet value by 2^EXTRA_TPC_LUT_BITS
// #define GET_FULL_PRECISION_TPC_LUT_VALUE(adjustedPwrDet, hi8_adjustedPwrDet) \
// (adjustedPwrDet & BIT_7 ? (((tTpcLutValue)hi8_adjustedPwrDet << 7) | (adjustedPwrDet & MSK_7)) : ((tTpcLutValue)adjustedPwrDet * EXTRA_TPC_LUT_MULT))
// 
// #define SET_FULL_PRECISION_TPC_LUT_VALUE(fullPrecisionLutValue, powerPointStructure)                        \
// {                                                                                                           \
//     powerPointStructure.extraPrecision.hi8_adjustedPwrDet = (fullPrecisionLutValue & (MSK_8 << 7)) >> 7;    \
//     powerPointStructure.adjustedPwrDet = (fullPrecisionLutValue & MSK_7) | BIT_7;                           \
// }
// 
*/

typedef struct
{
    t2Decimal min;  //sometimes used for comparing chain powers
    t2Decimal max;  //sometimes used for comparing chain powers
}tPowerdBmRange;        //absolute power measurement precision maintained to two decimal places


typedef struct
{
    tANI_U16 freq;                                     //frequency in MHz
    tANI_U16 reserved;
    tPowerdBmRange absPower;                           //Power range common to all chains
    tTpcCaldPowerTable empirical;                      //calibrated power points
}tTpcConfig;


typedef struct
{
    tANI_U8 numTpcCalPointsPerFreq;
    tANI_U8 numTpcCalFreqs;
    tANI_U8 calTemp;
    tANI_U8 reserved;
}tTpcParams;



//these definitions used as indexing to power per channel per rate table stored in NV
#define CB_RATE_POWER_OFFSET            0
#define CB_RATE_POWER_OFFSET_LAST_INDEX 60  //last index where we would apply the CB_RATE_POWER_OFFSET
#define NUM_RATE_POWER_GROUPS           NUM_HAL_PHY_RATES  //total number of rate power groups including the CB_RATE_POWER_OFFSET

typedef uAbsPwrPrecision tRateGroupPwr[NUM_RATE_POWER_GROUPS];



// KEEPING THESE TYPES FOR FUTURE USE
// //This structure says at a certain commanded power, we want to apply the raw adjustment values to the interpolated LUT and reinterpolate based on these.
// typedef struct
// {
//     tPowerdBm commandedPwr;     //if 0, then this point is not used, otherwise, this is an integer dbm value, such as 13.0 dBm
//     tANI_S8 dbmAdjustLut0;      //chain 0 integer +/- adjustment to the LUT value at the current ADC index for the commanded power
//     tANI_S8 dbmAdjustLut1;      //chain 1 integer +/- adjustment to the LUT value at the current ADC index for the commanded power
//     tANI_U8 reserved;           //save this for third Tx chain later
// }tTPCPowerCorrectPoint;
// 
// #define MAX_TPC_CORRECT_POWER_POINTS   5
// #define MAX_TPC_CORRECT_TEMPERATURES   4    //probably 0, 20, 40, & 60 degrees C but not necessarily
// 
// 
// //We expect these correction power points to be taken in increasing order, starting at index 0
// typedef struct
// {
//     tTPCPowerCorrectPoint adjust[MAX_TPC_CORRECT_POWER_POINTS];
//     tTempADCVal temp;
//     tANI_U8 reserved[3];
// }tTPCTempCompensation;
// 
// typedef tTPCTempCompensation tTPCTempCompSubband[MAX_TPC_CORRECT_TEMPERATURES];


//these structures allow us to store the necessary gains for calibration in NV
// typedef struct
// {
//     tANI_U8 rfLoopbackGains;   //bit 0 = tx_lb_gain, bit 1 = rx_lb_gain
//     tANI_U8 rxGain;
// }sTxLoCalClipGains;
// 
// typedef struct
// {
//     tANI_U8 rfDetGain;
//     tANI_U8 rxGain;
// }sTxIqCalClipGains;
// 
// typedef struct
// {
//     tANI_U8 rfLoopbackGains;      //bit 0 = tx_lb_gain, bit 1 = rx_lb_gain
//     tANI_U8 txGain;             //override tx gain value = coarse & fine
// }sRxIqCalClipGains;
// 
// typedef struct
// {
//     tANI_BOOLEAN    useDcoCorrection;
//     tANI_BOOLEAN    useTxLoCorrection;
//     tRxDcoCorrect   txloDcoCorrect[PHY_MAX_RX_CHAINS];
//     tRxDcoCorrect   dcoCorrection[PHY_MAX_RX_CHAINS];
//     tTxLoCorrect    txloCorrection[PHY_MAX_TX_CHAINS];
//     sTxLoCorrectChannel txloBasebandCorrection;
// }sCalTable;
// 


/* TX Power Calibration & Report Types */


typedef struct
{
    tANI_U8  temperatureAdc;                //= 5 bit temperature measured at time sample was taken
    tANI_U8  txGain;                        //= 7 bit gain value used to get the power measurement
    tANI_U8  pwrDetAdc;                     //= 8 bit ADC power detect value
    tANI_U8  reserved;
    uAbsPwrPrecision absPowerMeasured;      //= dBm measurement, will be truncated to two decimal places
}tTpcCalPoint;


typedef struct
{
    tANI_U16 numTpcCalPoints;
    tANI_U16 reserved;
    tTpcCalPoint chain[MAX_TPC_CAL_POINTS];
}tTpcChainData;


typedef struct
{
    tANI_U16 freq;                                          //frequency in MHz
    tANI_U16 reserved;
    tTpcChainData empirical[PHY_MAX_TX_CHAINS];  //TPC samples passed in
}tTpcFreqData;

typedef struct
{
    tANI_U8 numChannels;
    tANI_U8 reserved[3];
    tTpcFreqData calValues[MAX_TPC_CHANNELS];
}sTpcFreqCalTable;


typedef struct
{
    tPowerDetect lut;                   //7-bit value in the power Lookup Table
    tANI_U8 reserved[3];
    
    uAbsPwrPrecision abs;               //LUT value conversion to absolute dBm
}tTxPowerLutOutput;

typedef struct
{
    tANI_U8      gain;                  //8-bit coarse(bits 4-7) & fine(bits 0-3) gain commanded for the current index
    tPowerDetect adc;                   //8-bit power ADC sampled during the packet preamble
    tANI_U8 reserved[2];
    
    tTxPowerLutOutput indexMinMatch;    //minimum LUT matching power that satisfies the power template index setting
    tTxPowerLutOutput indexMaxMatch;    //maximum LUT matching power that satisfies the power template index setting
    tTxPowerLutOutput output;           //output power values corresponding to power ADC index
}tTxChainPower;




#endif /* HALPHYCFG_H */

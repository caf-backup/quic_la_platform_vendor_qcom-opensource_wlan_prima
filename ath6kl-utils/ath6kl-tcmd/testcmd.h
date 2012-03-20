/*
* Copyright (c) 2011-2012 Qualcomm Atheros Inc. All Rights Reserved.
* Qualcomm Atheros Proprietary and Confidential.
*/

#ifndef  TESTCMD_H_
#define  TESTCMD_H_

#include <stdint.h>

#ifdef AR6002_REV2
#define TCMD_MAX_RATES 12
#else
#define TCMD_MAX_RATES 28
#endif

#define PREPACK
#define POSTPACK __attribute__ ((packed))

#define ATH_MAC_LEN 6
#define TC_CMDS_SIZE_MAX  256

typedef enum {
	ZEROES_PATTERN = 0,
	ONES_PATTERN,
	REPEATING_10,
	PN7_PATTERN,
	PN9_PATTERN,
	PN15_PATTERN
} TX_DATA_PATTERN;

/* Continous tx
   mode : TCMD_CONT_TX_OFF - Disabling continous tx
          TCMD_CONT_TX_SINE - Enable continuous unmodulated tx
          TCMD_CONT_TX_FRAME- Enable continuous modulated tx
   freq : Channel freq in Mhz. (e.g 2412 for channel 1 in 11 g)
dataRate: 0 - 1 Mbps
          1 - 2 Mbps
          2 - 5.5 Mbps
          3 - 11 Mbps
          4 - 6 Mbps
          5 - 9 Mbps
          6 - 12 Mbps
          7 - 18 Mbps
          8 - 24 Mbps
          9 - 36 Mbps
         10 - 28 Mbps
         11 - 54 Mbps
  txPwr: Tx power in dBm[5 -11] for unmod Tx, [5-14] for mod Tx
antenna:  1 - one antenna
          2 - two antenna
Note : Enable/disable continuous tx test cmd works only when target is awake.
*/

typedef enum {
	TCMD_CONT_TX_OFF = 0,
	TCMD_CONT_TX_SINE,
	TCMD_CONT_TX_FRAME,
	TCMD_CONT_TX_TX99,
	TCMD_CONT_TX_TX100
} TCMD_CONT_TX_MODE;

typedef enum {
	TCMD_WLAN_MODE_NOHT = 0,
	TCMD_WLAN_MODE_HT20 = 1,
	TCMD_WLAN_MODE_HT40PLUS = 2,
	TCMD_WLAN_MODE_HT40MINUS = 3,
        TCMD_WLAN_MODE_CCK = 4,

        TCMD_WLAN_MODE_MAX,
        TCMD_WLAN_MODE_INVALID = TCMD_WLAN_MODE_MAX
} TCMD_WLAN_MODE;

typedef enum {
    TPC_TX_PWR = 0,
    TPC_FORCED_GAIN,
    TPC_TGT_PWR
} TPC_TYPE;

typedef PREPACK struct {
	uint32_t testCmdId;
	uint32_t mode;
	uint32_t freq;
	uint32_t dataRate;
	int32_t txPwr;
	uint32_t antenna;
	uint32_t enANI;
	uint32_t scramblerOff;
	uint32_t aifsn;
	uint16_t pktSz;
	uint16_t txPattern;
	uint32_t shortGuard;
	uint32_t numPackets;
	uint32_t wlanMode;
        uint32_t tpcm;
} POSTPACK TCMD_CONT_TX;

#define TCMD_TXPATTERN_ZERONE                 0x1
#define TCMD_TXPATTERN_ZERONE_DIS_SCRAMBLE    0x2

/* Continuous Rx
 act: TCMD_CONT_RX_PROMIS - promiscuous mode (accept all incoming frames)
      TCMD_CONT_RX_FILTER - filter mode (accept only frames with dest
                                             address equal specified
                                             mac address (set via act =3)
      TCMD_CONT_RX_REPORT  off mode  (disable cont rx mode and get the
                                          report from the last cont
                                          Rx test)

     TCMD_CONT_RX_SETMAC - set MacAddr mode (sets the MAC address for the
                                                 target. This Overrides
                                                 the default MAC address.)

*/
typedef enum {
	TCMD_CONT_RX_PROMIS = 0,
	TCMD_CONT_RX_FILTER,
	TCMD_CONT_RX_REPORT,
	TCMD_CONT_RX_SETMAC,
	TCMD_CONT_RX_SET_ANT_SWITCH_TABLE
} TCMD_CONT_RX_ACT;

typedef PREPACK struct {
	uint32_t testCmdId;
	uint32_t act;
	uint32_t enANI;
	PREPACK union {
		struct PREPACK TCMD_CONT_RX_PARA {
			uint32_t freq;
			uint32_t antenna;
			uint32_t wlanMode;
		} POSTPACK para;
		struct PREPACK TCMD_CONT_RX_REPORT {
			uint32_t totalPkt;
			int32_t rssiInDBm;
			uint32_t crcErrPkt;
			uint32_t secErrPkt;
			uint16_t rateCnt[TCMD_MAX_RATES];
			uint16_t rateCntShortGuard[TCMD_MAX_RATES];
		} POSTPACK report;
		struct PREPACK TCMD_CONT_RX_MAC {
			char addr[ATH_MAC_LEN];
			char btaddr[ATH_MAC_LEN];
                        uint16_t regDmn[2];
                        uint32_t otpWriteFlag;
		} POSTPACK mac;
		struct PREPACK TCMD_CONT_RX_ANT_SWITCH_TABLE {
			uint32_t antswitch1;
			uint32_t antswitch2;
		} POSTPACK antswitchtable;
	} POSTPACK u;
} POSTPACK TCMD_CONT_RX;

/* Force sleep/wake  test cmd
 mode: TCMD_PM_WAKEUP - Wakeup the target
       TCMD_PM_SLEEP - Force the target to sleep.
 */
typedef enum {
	TCMD_PM_WAKEUP = 1,	/* be consistent with target */
	TCMD_PM_SLEEP,
	TCMD_PM_DEEPSLEEP
} TCMD_PM_MODE;

typedef enum {
    TC_CMDS_VERSION_RESERVED=0,
    TC_CMDS_VERSION_MDK,
    TC_CMDS_VERSION_TS,
    TC_CMDS_VERSION_LAST,
} TC_CMDS_VERSION;

typedef enum {
    TC_CMDS_TS =0,
    TC_CMDS_CAL,
    TC_CMDS_TPCCAL = TC_CMDS_CAL,
    TC_CMDS_TPCCAL_WITH_OTPWRITE,
    TC_CMDS_OTPDUMP,
    TC_CMDS_OTPSTREAMWRITE,
    TC_CMDS_EFUSEDUMP,
    TC_CMDS_EFUSEWRITE,
    TC_CMDS_READTHERMAL,
} TC_CMDS_ACT;

typedef PREPACK struct {
    uint32_t   testCmdId;
    uint32_t   act;
    PREPACK union {
        uint32_t  enANI;    // to be identical to CONT_RX struct
        struct PREPACK {
            uint16_t   length;
            uint8_t    version;
            uint8_t    bufLen;
        } POSTPACK parm;
    } POSTPACK u;
} POSTPACK TC_CMDS_HDR;

typedef PREPACK struct {
    TC_CMDS_HDR  hdr;
    char buf[TC_CMDS_SIZE_MAX];
} POSTPACK TC_CMDS;

typedef PREPACK struct {
    uint32_t    testCmdId;
    uint32_t    regAddr;
    uint32_t    val;
    uint16_t    flag;
} POSTPACK TCMD_SET_REG;

typedef PREPACK struct {
	uint32_t testCmdId;
	uint32_t mode;
} POSTPACK TCMD_PM;

typedef enum {
	TCMD_CONT_TX_ID,
	TCMD_CONT_RX_ID,
	TCMD_PM_ID,
        TC_CMDS_ID,
        TCMD_SET_REG_ID,

	/*For synergy purpose we added the following tcmd id but these
	tcmd's will not go to the firmware instead we will write values
	to the NV area */

	TCMD_NIC_MAC = 100,
	TCMD_CAL_FILE_INDEX = 101,
} TCMD_ID;

typedef PREPACK struct
{
    uint32_t  testCmdId;
    char   mac_address[ATH_MAC_LEN];
} POSTPACK TCMD_NIC_MAC_S;

typedef PREPACK struct
{
       uint32_t  testCmdId;
       uint32_t  cal_file_index;
} POSTPACK TCMD_CAL_FILE_INDEX_S;

typedef PREPACK union {
	TCMD_CONT_TX contTx;
	TCMD_CONT_RX contRx;
	TCMD_PM pm;
          // New test cmds from ART/MDK ...
        TC_CMDS              tcCmds;
        TCMD_SET_REG setReg;
} POSTPACK TEST_CMD;

#ifdef __cplusplus
}
#endif

#endif /* TESTCMD_H_ */

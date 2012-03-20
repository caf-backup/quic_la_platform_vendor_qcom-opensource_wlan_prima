/*
* Copyright (c) 2011-2012 Qualcomm Atheros Inc. All Rights Reserved.
* Qualcomm Atheros Proprietary and Confidential.
*/

#include <sys/types.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/types.h>
#include <net/if.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "athtestcmd.h"
#include "libtcmd.h"
#include "testcmd.h"

const char *progname;
const char commands[] = "commands:\n"
	"--tx <sine/frame/tx99/tx100/off>\n"
	"--txfreq <Tx channel or freq(default 2412)>\n"
	"--txrate <rate index>\n"
        "--txpwr <frame/tx99/tx100: 0-30dBm,0.5dBm resolution; sine: 0-60, PCDAC vaule>\n"
	"--txantenna <1/2/0 (auto)>\n"
	"--txpktsz <pkt size, [32-1500](default 1500)>\n"
	"--txpattern <tx data pattern, 0: all zeros; 1: all ones;"
	" 2: repeating 10; 3: PN7; 4: PN9; 5: PN15\n"
	"--ani (Enable ANI. The ANI is disabled if this option is not specified)\n"
	"--scrambleroff (Disable scrambler. The scrambler is enabled by default)\n"
	"--aifsn <AIFS slots num,[0-252](Used only under '--tx frame' mode)>\n"
	"--shortguard (use short guard)\n"
	"--mode <ht40plus/ht40minus/ht20>\n"
	"--setlongpreamble <1/0>\n"
	"--numpackets <number of packets to send 0-65535>\n"
        "--tx sine --txfreq <Tx channel or freq(default 2412)>\n"
	"--rx <promis/filter/report>\n"
	"--rxfreq <Rx channel or freq(default 2412)>\n"
	"--rxantenna <1/2/0 (auto)>\n"
	"--mode <ht40plus/ht40minus>\n"
	"--pm <wakeup/sleep/deepsleep>\n"
	"--setmac <mac addr like 00:03:7f:be:ef:11>\n"
	"--SetAntSwitchTable <table1 in decimal value>"
	" <table2 in decimal value>  (Set table1=0 and table2=0 will"
	" restore the default AntSwitchTable)\n"
        "--efusedump --start <start address> --end <end address>\n"
        "--efusewrite --start <start address> --data <data> (could be one or multiple data in quotation marks)\n"
        "--otpwrite --data (could be one or multiple data in quotation marks)\n"
        "--otpdump\n";

#define A_ERR(ret, args...) printf(args); exit(ret);

#define A_FREQ_MIN              4920
#define A_FREQ_MAX              5825

#define A_CHAN0_FREQ            5000
#define A_CHAN_MAX              ((A_FREQ_MAX - A_CHAN0_FREQ)/5)

#define BG_FREQ_MIN             2412
#define BG_FREQ_MAX             2484

#define BG_CHAN0_FREQ           2407
#define BG_CHAN_MIN             ((BG_FREQ_MIN - BG_CHAN0_FREQ)/5)
#define BG_CHAN_MAX             14	/* corresponding to 2484 MHz */

#define A_20MHZ_BAND_FREQ_MAX   5000

#define INVALID_FREQ    0
#define A_OK            0

#define ATH6KL_INTERFACE "wlan0"

#define A_RATE_NUM      28
#define G_RATE_NUM      28

#define RATE_STR_LEN    20
#define VENUS_OTP_SIZE  512
typedef const char RATE_STR[RATE_STR_LEN];

const RATE_STR bgRateStrTbl[G_RATE_NUM] = {
	{"1   Mb"},
	{"2   Mb"},
	{"5.5 Mb"},
	{"11  Mb"},
	{"6   Mb"},
	{"9   Mb"},
	{"12  Mb"},
	{"18  Mb"},
	{"24  Mb"},
	{"36  Mb"},
	{"48  Mb"},
	{"54  Mb"},
	{"HT20 MCS0 6.5  Mb"},
	{"HT20 MCS1 13  Mb"},
	{"HT20 MCS2 19.5  Mb"},
	{"HT20 MCS3 26  Mb"},
	{"HT20 MCS4 39  Mb"},
	{"HT20 MCS5 52  Mb"},
	{"HT20 MCS6 58.5  Mb"},
	{"HT20 MCS7 65  Mb"},
	{"HT40 MCS0 13.5  Mb"},
	{"HT40 MCS1 27.0  Mb"},
	{"HT40 MCS2 40.5  Mb"},
	{"HT40 MCS3 54  Mb"},
	{"HT40 MCS4 81  Mb"},
	{"HT40 MCS5 108  Mb"},
	{"HT40 MCS6 121.5  Mb"},
	{"HT40 MCS7 135  Mb"}
};

static void rxReport(void *buf);
static void rx_cb(void *buf, int len);
static uint32_t freqValid(uint32_t val);
static uint16_t wmic_ieee2freq(uint32_t chan);
static void prtRateTbl(uint32_t freq);
static uint32_t rateValid(uint32_t val, uint32_t freq);
static uint32_t antValid(uint32_t val);
static uint32_t txPwrValid(TCMD_CONT_TX * txCmd);
static int ath_ether_aton(const char *orig, uint8_t * eth);
static uint32_t pktSzValid(uint32_t val);

static bool isHex(char c) {
    return (((c >= '0') && (c <= '9')) ||
            ((c >= 'A') && (c <= 'F')) ||
            ((c >= 'a') && (c <= 'f')));
}

static int usage(void)
{
	fprintf(stderr, "usage:\n%s [-i device] commands\n", progname);
	fprintf(stderr, "%s\n", commands);
	prtRateTbl(INVALID_FREQ);
	A_ERR(-1, "Incorrect usage");
}

unsigned int cmd = 0;
unsigned int act = 0;
uint16_t data_length = 0;
uint16_t efuse_begin = 0, efuse_end = (VENUS_OTP_SIZE - 1);
static TC_CMDS sTcCmds;

int main(int argc, char **argv)
{
	int c, err;
	char ifname[IFNAMSIZ];
	progname = argv[0];
	char buf[2048];
	bool resp = false;
	TCMD_CONT_TX *txCmd = (TCMD_CONT_TX *) buf;
	TCMD_CONT_RX *rxCmd = (TCMD_CONT_RX *) buf;
	TCMD_PM *pmCmd = (TCMD_PM *) buf;
        TCMD_SET_REG *setRegCmd = (TCMD_SET_REG *)buf;
        TC_CMDS  *tCmds = (TC_CMDS *)buf;
        char efuseBuf[VENUS_OTP_SIZE];
        char efuseWriteBuf[VENUS_OTP_SIZE];
        int bufferLength = sizeof(*txCmd);

	txCmd->numPackets = 0;
	txCmd->wlanMode = TCMD_WLAN_MODE_NOHT;
        txCmd->tpcm = TPC_TX_PWR;
	rxCmd->u.para.wlanMode = TCMD_WLAN_MODE_NOHT;
        rxCmd->u.para.freq = 2412;

	if (argc == 1) {
		usage();
	}

	while (1) {
		int option_index = 0;

                static struct option long_options[] = {
			{"version", 0, NULL, 'v'},
			{"interface", 1, NULL, 'i'},
			{"tx", 1, NULL, 't'},
			{"txfreq", 1, NULL, 'f'},
			{"txrate", 1, NULL, 'g'},
			{"txpwr", 1, NULL, 'h'},
			{"tgtpwr", 0, NULL, 'H'},
			{"pcdac", 1, NULL, 'I'},
			{"txantenna", 1, NULL, 'j'},
			{"txpktsz", 1, NULL, 'z'},
			{"txpattern", 1, NULL, 'e'},
			{"rx", 1, NULL, 'r'},
			{"rxfreq", 1, NULL, 'p'},
			{"rxantenna", 1, NULL, 'q'},
			{"pm", 1, NULL, 'x'},
			{"setmac", 1, NULL, 's'},
			{"ani", 0, NULL, 'a'},
			{"scrambleroff", 0, NULL, 'o'},
			{"aifsn", 1, NULL, 'u'},
			{"SetAntSwitchTable", 1, NULL, 'S'},
			{"shortguard", 0, NULL, 'G'},
			{"numpackets", 1, NULL, 'n'},
			{"mode", 1, NULL, 'M'},
			{"setlongpreamble", 1, NULL, 'l'},
			{"setreg", 1, NULL, 'R'},
			{"regval", 1, NULL, 'V'},
			{"flag", 1, NULL, 'F'},
			{"writeotp", 0, NULL, 'w'},
			{"otpregdmn", 1, NULL, 'E'},
			{"efusedump", 0, NULL, 'm'},
			{"efusewrite", 0, NULL, 'W'},
			{"start", 1, NULL, 'A'},
			{"end", 1, NULL, 'L'},
			{"data", 1, NULL, 'U'},
			{"otpwrite", 0, NULL, 'O'},
			{"otpdump", 0, NULL, 'P'},
			{"btaddr", 1, NULL, 'B'},
			{"therm", 0, NULL, 'c'},
			{0, 0, 0, 0}
		};

		c = getopt_long(argc, argv, "vi:t:f:g:h:HI:r:p:q:x:u:ao:M:A:L:mU:WOP",
                         long_options, &option_index);

		if (c == -1)
			break;

        switch (c) {
        case 'i':
            memset(ifname, '\0', 8);
            strcpy(ifname, optarg);
            break;
        case 't':
            cmd = TESTMODE_CONT_TX;
	    txCmd->testCmdId = TCMD_CONT_TX_ID;
            if (!strcmp(optarg, "sine")) {
                txCmd->mode = TCMD_CONT_TX_SINE;
            } else if (!strcmp(optarg, "frame")) {
                txCmd->mode = TCMD_CONT_TX_FRAME;
            } else if (!strcmp(optarg, "tx99")) {
                txCmd->mode = TCMD_CONT_TX_TX99;
            } else if (!strcmp(optarg, "tx100")) {
                txCmd->mode = TCMD_CONT_TX_TX100;
            } else if (!strcmp(optarg, "off")) {
                txCmd->mode = TCMD_CONT_TX_OFF;
            }else {
                cmd = 0;
            }
            break;
        case 'f':
            txCmd->freq = freqValid(atoi(optarg));
            break;
        case 'G':
            txCmd->shortGuard = 1;
            break;
        case 'M':
            if(cmd == TESTMODE_CONT_TX) {
                if (!strcmp(optarg, "ht20")) {
                    txCmd->wlanMode = TCMD_WLAN_MODE_HT20;
                } else if (!strcmp(optarg, "ht40plus")) {
                    txCmd->wlanMode = TCMD_WLAN_MODE_HT40PLUS;
                } else if (!strcmp(optarg, "ht40minus")) {
                    txCmd->wlanMode = TCMD_WLAN_MODE_HT40MINUS;
                }
            } else if(cmd == TESTMODE_CONT_RX) {
                if (!strcmp(optarg, "ht20")) {
                    rxCmd->u.para.wlanMode = TCMD_WLAN_MODE_HT20;
                } else if (!strcmp(optarg, "ht40plus")) {
                    rxCmd->u.para.wlanMode = TCMD_WLAN_MODE_HT40PLUS;
                } else if (!strcmp(optarg, "ht40minus")) {
                    rxCmd->u.para.wlanMode = TCMD_WLAN_MODE_HT40MINUS;
                }
            }
            break;
        case 'n':
            txCmd->numPackets = atoi(optarg);
            break;
        case 'g':
            /* let user input index of rateTable instead of string parse */
            txCmd->dataRate = rateValid(atoi(optarg), txCmd->freq);
            break;
        case 'h':
        {
            int txPowerAsInt;
            /* Get tx power from user.  This is given in the form of a number
             * that's supposed to be either an integer, or an integer + 0.5
             */
            double txPowerIndBm = atof(optarg);

            /*
             * Check to make sure that the number given is either an integer
             * or an integer + 0.5
             */
            txPowerAsInt = (int)txPowerIndBm;
            if (((txPowerIndBm - (double)txPowerAsInt) == 0) ||
                (((txPowerIndBm - (double)txPowerAsInt)) == 0.5) ||
                (((txPowerIndBm - (double)txPowerAsInt)) == -0.5)) {
                if (txCmd->mode != TCMD_CONT_TX_SINE) {
                    txCmd->txPwr = txPowerIndBm * 2;
                } else {
                    txCmd->txPwr = txPowerIndBm;
                }
           } else {
                printf("Bad argument to --txpwr, must be in steps of 0.5 dBm\n");
                cmd = 0;
           }

            txCmd->tpcm = TPC_TX_PWR;
        }
            break;
        case 'H':
            txCmd->tpcm = TPC_TGT_PWR;
            break;
        case 'I':
            txCmd->tpcm = TPC_FORCED_GAIN;
            txCmd->txPwr = atof(optarg);
            break;
        case 'j':
            txCmd->antenna = antValid(atoi(optarg));
            break;
        case 'z':
            txCmd->pktSz = pktSzValid(atoi(optarg));
            break;
        case 'e':
            txCmd->txPattern = atoi(optarg);
            break;
        case 'r':
            cmd = TESTMODE_CONT_RX;
	    rxCmd->testCmdId = TCMD_CONT_RX_ID;
            if (!strcmp(optarg, "promis")) {
                rxCmd->act = TCMD_CONT_RX_PROMIS;
		printf(" Its cont Rx promis mode \n");
            } else if (!strcmp(optarg, "filter")) {
                rxCmd->act = TCMD_CONT_RX_FILTER;
		printf(" Its cont Rx  filter  mode \n");
            } else if (!strcmp(optarg, "report")) {
		printf(" Its cont Rx report  mode \n");
                rxCmd->act = TCMD_CONT_RX_REPORT;
                resp = true;
            } else {
                cmd = 0;
            }
            break;
        case 'p':
            rxCmd->u.para.freq = freqValid(atoi(optarg));
            break;
        case 'q':
            rxCmd->u.para.antenna = antValid(atoi(optarg));
            break;
        case 'x':
            cmd = TESTMODE_PM;
	    pmCmd->testCmdId = TCMD_PM_ID;
            if (!strcmp(optarg, "wakeup")) {
                pmCmd->mode = TCMD_PM_WAKEUP;
            } else if (!strcmp(optarg, "sleep")) {
                pmCmd->mode = TCMD_PM_SLEEP;
            } else if (!strcmp(optarg, "deepsleep")) {
                pmCmd->mode = TCMD_PM_DEEPSLEEP;
            } else {
                cmd = 0;
            }
            break;
        case 's':
            {
                uint8_t mac[ATH_MAC_LEN];

                cmd = TESTMODE_CONT_RX;
                rxCmd->testCmdId = TCMD_CONT_RX_ID;
                rxCmd->act = TCMD_CONT_RX_SETMAC;
                if (ath_ether_aton(optarg, mac) != 0) {
                    printf("Invalid mac address format! \n");
                }
                memcpy(rxCmd->u.mac.addr, mac, ATH_MAC_LEN);
                printf("JLU: tcmd: setmac 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x\n",
                        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
                break;
            }
        case 'u':
            {
                txCmd->aifsn = atoi(optarg) & 0xff;
                printf("AIFS:%d\n", txCmd->aifsn);
            }
            break;
        case 'a':
            if(cmd == TESTMODE_CONT_TX) {
                txCmd->enANI = true;
            } else if(cmd == TESTMODE_CONT_RX) {
                rxCmd->enANI = true;
            }
            break;
        case 'o':
            txCmd->scramblerOff = true;
            break;
        case 'S':
            if (argc < 4)
                usage();
            cmd = TESTMODE_CONT_RX;
            rxCmd->testCmdId = TCMD_CONT_RX_ID;
            rxCmd->act = TCMD_CONT_RX_SET_ANT_SWITCH_TABLE;
            rxCmd->u.antswitchtable.antswitch1 = strtoul(argv[2], (char **)NULL,0);
            rxCmd->u.antswitchtable.antswitch2 = strtoul(argv[3], (char **)NULL,0);
            break;
        case 'l':
            printf("Not supported\n");
            return 0;
            break;
        case 'R':
            if (argc < 5) {
                printf("usage:athtestcmd -i wlan0 --setreg 0x1234 --regval 0x01 --flag 0\n");
            }
            cmd = TESTMODE_SETREG;
            setRegCmd->testCmdId = TCMD_SET_REG_ID;
            setRegCmd->regAddr   = strtoul(optarg, (char **)NULL, 0);//atoi(optarg);
            break;
        case 'V':
            setRegCmd->val = strtoul(optarg, (char **)NULL, 0);
            break;
        case 'F':
            setRegCmd->flag = atoi(optarg);
            break;
        case 'w':
            rxCmd->u.mac.otpWriteFlag = 1;
            break;
        case 'E':
            rxCmd->u.mac.regDmn[0] = 0xffff&(strtoul(optarg, (char **)NULL, 0));
            rxCmd->u.mac.regDmn[1] = 0xffff&(strtoul(optarg, (char **)NULL, 0)>>16);
            break;
        case 'B':
            {
                uint8_t btaddr[ATH_MAC_LEN];
                if (ath_ether_aton(optarg, btaddr) != 0) {
                    printf("Invalid mac address format! \n");
                }
                memcpy(rxCmd->u.mac.btaddr, btaddr, ATH_MAC_LEN);
                printf("JLU: tcmd: setbtaddr 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x\n",
                        btaddr[0], btaddr[1], btaddr[2], btaddr[3], btaddr[4], btaddr[5]);
            }
            break;
        case 'c':
            cmd = TESTMODE_CMDS;
            tCmds->hdr.testCmdId = TC_CMDS_ID;
            {
                tCmds->hdr.u.parm.length = 0;
                tCmds->hdr.u.parm.version = TC_CMDS_VERSION_TS;
                act = tCmds->hdr.act = TC_CMDS_READTHERMAL;//TC_CMDS_CAL_THERMALVOLT;
                resp = true;
            }
            break;
        case 'A':
            efuse_begin = atoi(optarg);
            break;

        case 'L':
            efuse_end = atoi(optarg);
            break;

        case 'U':
            {
                uint8_t* pucArg = (uint8_t*)optarg;
                uint8_t  c;
                uint8_t  strBuf[256];
                uint8_t  pos = 0;
                uint16_t length = 0;
                uint32_t  data;

                /* Sweep string to end */
                while (1) {
                    c = *pucArg++;
                    if (isHex(c)) {
                        strBuf[pos++] = c;
                    } else {
                        strBuf[pos] = '\0';
                        pos = 0;
                        sscanf(((char *)&strBuf), "%x", &data);
                        efuseWriteBuf[length++] = (data & 0xFF);

                        /* End of arg string */
                        if (c == '\0') {
                            break;
                        }
                    }
                }

                data_length = length;
            }
            break;

        case 'm':
            cmd = TESTMODE_CMDS;
            tCmds->hdr.testCmdId      = TC_CMDS_ID;
            act = tCmds->hdr.act      = TC_CMDS_EFUSEDUMP;
            resp = true;
            break;

        case 'W':
            cmd = TESTMODE_CMDS;
            tCmds->hdr.testCmdId      = TC_CMDS_ID;
            act = tCmds->hdr.act      = TC_CMDS_EFUSEWRITE;
            resp = true;
            break;

        case 'O':
            cmd = TESTMODE_CMDS;
            tCmds->hdr.testCmdId      = TC_CMDS_ID;
            act = tCmds->hdr.act      = TC_CMDS_OTPSTREAMWRITE;
            resp = true;
            break;

        case 'P':
            cmd = TESTMODE_CMDS;
            tCmds->hdr.testCmdId      = TC_CMDS_ID;
            act = tCmds->hdr.act      = TC_CMDS_OTPDUMP;
            resp = true;
            break;

        default:
            usage();
        }
     }

     if ( cmd == TESTMODE_CMDS )
     {
         if ( tCmds->hdr.act == TC_CMDS_EFUSEWRITE )
         {
            int i;
            /* Error check */
            if (data_length == 0) {
                printf("No data to write, exit..\n");
                return 0;
            } else if ((efuse_begin + data_length + 4) > TC_CMDS_SIZE_MAX) {
                printf("Exceed eFuse border: %d, exit..\n", (TC_CMDS_SIZE_MAX - 1));
                return 0;
            }

            /* PRINT */
            printf("eFuse data (%d Bytes): ", data_length);
            for (i = 0; i < data_length; i++) {
                printf("%02X ", efuseWriteBuf[i]);
            }
            printf("\n");

            /* Write address and data length */
            tCmds->buf[0] = (efuse_begin & 0xFF);
            tCmds->buf[1] = (efuse_begin >> 8) & 0xFF;
            tCmds->buf[2] = (data_length & 0xFF);
            tCmds->buf[3] = (data_length >> 8) & 0xFF;

            /* Copy data to tcmd buffer. The first 4 bytes are the ID and length */
            memcpy((void*)&(tCmds->buf[4]), (void*)&(efuseWriteBuf[0]), data_length);

            /* Construct eFuse Write */
            tCmds->hdr.u.parm.length  = (4 + data_length);
            tCmds->hdr.u.parm.version = TC_CMDS_VERSION_TS;
        }
        else if ( tCmds->hdr.act == TC_CMDS_OTPSTREAMWRITE )
        {
            int i;

            /* Error check */
            if (data_length == 0) {
                printf("No data to write, exit..\n");
                return 0;
            } else if ((data_length + 4) > TC_CMDS_SIZE_MAX) {
                printf("Exceed OTP size: %d, exit..\n", data_length);
                return 0;
            }

            /* PRINT */
            printf("Write OTP data (%d Bytes): ", data_length);
            for (i = 0; i < data_length; i++) {
                printf("%02X ", efuseWriteBuf[i]);
            }
            printf("\n");

            /* Copy data to tcmd buffer. The first 4 bytes are the ID and length */
            memcpy((void*)&(tCmds->buf[0]), (void*)&(efuseWriteBuf[0]), data_length);

            /* Construct eFuse Write */
            tCmds->hdr.u.parm.length  = data_length;
            tCmds->hdr.u.parm.version = TC_CMDS_VERSION_TS;
        }
        else if ( tCmds->hdr.act == TC_CMDS_OTPDUMP )
        {
            tCmds->hdr.u.parm.length  = 0;
            tCmds->hdr.u.parm.version = TC_CMDS_VERSION_TS;
        }
    }

    /* default bufferLength = sizeof(*txCmd)*/
    if ( cmd == TESTMODE_CONT_TX )
    {
	bufferLength = sizeof(*txCmd);
    }
    else if ( cmd == TESTMODE_CONT_RX )
    {
	bufferLength = sizeof(*rxCmd);
    }
    else if ( cmd == TESTMODE_PM )
    {
	bufferLength = sizeof(*pmCmd);
    }
    else if ( cmd == TESTMODE_SETREG)
    {
	bufferLength = sizeof(*setRegCmd);
    }
    else if ( cmd == TESTMODE_CMDS )
    {
	bufferLength = sizeof(*tCmds);
    }

     printf("Cmd %d length %d respNeeded %d\n",cmd,bufferLength,resp);

     err = tcmd_tx_init(ATH6KL_INTERFACE, rx_cb);

     if (err)
        return err;

     if ( (cmd == TESTMODE_CMDS) && (tCmds->hdr.act == TC_CMDS_EFUSEDUMP) )
     {
        int i, k;
        int blkNum;
        uint16_t efuseEnd   = efuse_end;
        uint16_t efuseBegin = efuse_begin;
        uint16_t efusePrintAnkor;
        uint16_t numPlaceHolder;

        /* Input check */
        if (efuseEnd > (VENUS_OTP_SIZE - 1)) {
            efuseEnd = (VENUS_OTP_SIZE - 1);
        }

        if (efuseBegin > efuseEnd) {
            efuseBegin = efuseEnd;
        }

        efusePrintAnkor = efuseBegin;

        blkNum = ((efuseEnd - efuseBegin) / TC_CMDS_SIZE_MAX) + 1;

        /* Request data in several trys */
        for (i = 0; i < blkNum; i++) {
            tCmds->hdr.testCmdId      = TC_CMDS_ID;
            tCmds->hdr.act            = TC_CMDS_EFUSEDUMP;
            tCmds->hdr.u.parm.length  = 4;
            tCmds->hdr.u.parm.version = TC_CMDS_VERSION_TS;

            tCmds->buf[0] = (efuseBegin & 0xFF);
            tCmds->buf[1] = (efuseBegin >> 8) & 0xFF;
            tCmds->buf[2] = (efuseEnd & 0xFF);
            tCmds->buf[3] = (efuseEnd >> 8) & 0xFF;

            if ((err = tcmd_tx(buf, bufferLength /* weak */, resp))) {
	        fprintf(stderr, "tcmd_tx had error: %s!\n", strerror(-err));
                return 0;
            }

            /* Last block? */
            //sTcCmds populated in the callback..
            if ((efuseEnd - efuseBegin + 1) < TC_CMDS_SIZE_MAX) {
                memcpy((void*)(efuseBuf + efuseBegin), (void*)&(sTcCmds.buf[0]), (efuseEnd - efuseBegin + 1));
            } else {
                memcpy((void*)(efuseBuf + efuseBegin), (void*)&(sTcCmds.buf[0]), TC_CMDS_SIZE_MAX);
            }

            /* Adjust the efuseBegin but keep efuseEnd unchanged */
            efuseBegin += TC_CMDS_SIZE_MAX;
         }

         /* Output Dump */
         printf("------------------- eFuse Dump ----------------------");
         for (i = efusePrintAnkor; i <= efuseEnd; i++) {
             /* Cosmetics */
             if (i == efusePrintAnkor) {
                 numPlaceHolder = (efusePrintAnkor & 0x0F);
                 printf("\n%04X:", (efusePrintAnkor & 0xFFF0));
                 for (k = 0; k < numPlaceHolder; k++) {
                     printf("   ");
                  }
             } else if ((i & 0x0F) == 0) {
                 printf("\n%04X:", i);
             }

             printf(" %02X", efuseBuf[i]);
         }
         printf("\n\n");
     }
     else
     {
         if ((err = tcmd_tx(buf, bufferLength /* weak */, resp))) {
	    fprintf(stderr, "tcmd_tx had error: %s!\n", strerror(-err));
          }
    }

     return 0;
}

static void rxReport(void *buf)
{
	struct TCMD_CONT_RX_REPORT *report = &((TCMD_CONT_RX *) buf)->u.report;
	uint32_t pkt = report->totalPkt;
	int32_t rssi = report->rssiInDBm;
	uint32_t crcError = report->crcErrPkt;
	uint32_t secErr = report->secErrPkt;
	uint16_t *rateCnt = report->rateCnt;
	uint16_t *rateCntShortGuard = report->rateCntShortGuard;

	printf
	    ("total pkt %d ; crcError pkt %d ; secErr pkt %d ;  average rssi %d\n",
	     pkt, crcError, secErr,
	     (int32_t) (pkt ? (rssi / (int32_t) pkt) : 0));

	printf("1Mbps     %d\n", rateCnt[0]);
	printf("2Mbps     %d\n", rateCnt[1]);
	printf("5.5Mbps   %d\n", rateCnt[2]);
	printf("11Mbps    %d\n", rateCnt[3]);
	printf("6Mbps     %d\n", rateCnt[4]);
	printf("9Mbps     %d\n", rateCnt[5]);
	printf("12Mbps    %d\n", rateCnt[6]);
	printf("18Mbps    %d\n", rateCnt[7]);
	printf("24Mbps    %d\n", rateCnt[8]);
	printf("36Mbps    %d\n", rateCnt[9]);
	printf("48Mbps    %d\n", rateCnt[10]);
	printf("54Mbps    %d\n", rateCnt[11]);
	printf("\n");
	printf("HT20 MCS0 6.5Mbps   %d (SGI: %d)\n", rateCnt[12],
	       rateCntShortGuard[12]);
	printf("HT20 MCS1 13Mbps    %d (SGI: %d)\n", rateCnt[13],
	       rateCntShortGuard[13]);
	printf("HT20 MCS2 19.5Mbps  %d (SGI: %d)\n", rateCnt[14],
	       rateCntShortGuard[14]);
	printf("HT20 MCS3 26Mbps    %d (SGI: %d)\n", rateCnt[15],
	       rateCntShortGuard[15]);
	printf("HT20 MCS4 39Mbps    %d (SGI: %d)\n", rateCnt[16],
	       rateCntShortGuard[16]);
	printf("HT20 MCS5 52Mbps    %d (SGI: %d)\n", rateCnt[17],
	       rateCntShortGuard[17]);
	printf("HT20 MCS6 58.5Mbps  %d (SGI: %d)\n", rateCnt[18],
	       rateCntShortGuard[18]);
	printf("HT20 MCS7 65Mbps    %d (SGI: %d)\n", rateCnt[19],
	       rateCntShortGuard[19]);
	printf("\n");
	printf("HT40 MCS0 13.5Mbps    %d (SGI: %d)\n", rateCnt[20],
	       rateCntShortGuard[20]);
	printf("HT40 MCS1 27.0Mbps    %d (SGI: %d)\n", rateCnt[21],
	       rateCntShortGuard[21]);
	printf("HT40 MCS2 40.5Mbps    %d (SGI: %d)\n", rateCnt[22],
	       rateCntShortGuard[22]);
	printf("HT40 MCS3 54Mbps      %d (SGI: %d)\n", rateCnt[23],
	       rateCntShortGuard[23]);
	printf("HT40 MCS4 81Mbps      %d (SGI: %d)\n", rateCnt[24],
	       rateCntShortGuard[24]);
	printf("HT40 MCS5 108Mbps     %d (SGI: %d)\n", rateCnt[25],
	       rateCntShortGuard[25]);
	printf("HT40 MCS6 121.5Mbps   %d (SGI: %d)\n", rateCnt[26],
	       rateCntShortGuard[26]);
	printf("HT40 MCS7 135Mbps     %d (SGI: %d)\n", rateCnt[27],
	       rateCntShortGuard[27]);

}

static void readThermal(void *buf,int len)
{
    TC_CMDS *tCmd;

    tCmd = (TC_CMDS *)buf;

    printf("Length rx cb rcvd %d\n",len);
    printf("act %d version %d length %d\n",tCmd->hdr.act,tCmd->hdr.u.parm.version,tCmd->hdr.u.parm.length);
    printf("chip thermal value:%d\n", tCmd->buf[0]);
    return;
}

static void cmdReply(void *buf, int len)
{
    TC_CMDS  tCmdReply;
    uint32_t act;
    uint16_t wBytes;
    int i=0;

    printf("Length rx cb rcvd %d\n",len);
    buf += 2 * sizeof(unsigned int);

    uint8_t *reply = (uint8_t*)buf;

    tCmdReply.hdr.u.parm.length = *(uint16_t *)&(reply[0]);
    tCmdReply.hdr.u.parm.version = (uint8_t)(reply[2]);
    act = tCmdReply.hdr.u.parm.version;

    /* Error Check */
    if (tCmdReply.hdr.u.parm.length > (TC_CMDS_SIZE_MAX + 1)) {
        printf("Error: Reply lenth=%d, limit=%d\n", tCmdReply.hdr.u.parm.length, TC_CMDS_SIZE_MAX);
        return;
    } else {
        printf(">> Reply length = %d, type = %d ", tCmdReply.hdr.u.parm.length, tCmdReply.hdr.u.parm.version);
    }

    if (tCmdReply.hdr.u.parm.length > 0) {
        memcpy((void*)&(tCmdReply.buf), (void*)(buf+4),  tCmdReply.hdr.u.parm.length);
        memcpy((void*)&(sTcCmds.buf[0]), (void*)&(tCmdReply.buf[0]),  tCmdReply.hdr.u.parm.length);
        sTcCmds.hdr.u.parm.length = tCmdReply.hdr.u.parm.length;
   }

    switch (act) {
        case TC_CMDS_EFUSEDUMP:
            printf("eFuse data:\n");
            break;
        case TC_CMDS_EFUSEWRITE:
            printf("(write eFuse data)\n");
            wBytes = ((sTcCmds.buf[1] << 8) | sTcCmds.buf[0]);
            printf("%d bytes written to eFuse.\n", wBytes);
            break;
        case TC_CMDS_OTPSTREAMWRITE:
            printf("(OTP stream write)\n");

            if (sTcCmds.buf[0] == A_OK) {
                printf("Write %d bytes to OTP\n", data_length);
            } else {
                printf("Failed to write OTP\n");
            }
            break;
        case TC_CMDS_OTPDUMP:
            printf("OTP Dump\n");
            if (sTcCmds.hdr.u.parm.length) {
                /* Received bytes are in sTcCmds */
                for (i = 0; i < sTcCmds.hdr.u.parm.length; i++) {
                    printf("%02x ", sTcCmds.buf[i]);
                }
                printf("\n");
            } else {
                printf("No valid stream found in OTP!\n");
            }
            break;
        default:
            printf("Invalid action!\n");
            break;
    }
}

static void rx_cb(void *buf, int len)
{
	TCMD_ID tcmd;

	if ( cmd == TESTMODE_CMDS )
        {
            if ( act == TC_CMDS_READTHERMAL )
            {
                readThermal(buf,len);
	    }
            else
            {
               cmdReply(buf,len);
            }

            return;
        }

	tcmd = * ((uint32_t *) buf + 1);

	switch (tcmd) {
	case TCMD_CONT_RX_REPORT:
		rxReport(buf);
		break;
	default:
		 break;
	}

}

static uint32_t freqValid(uint32_t val)
{
	do {
		if (val <= A_CHAN_MAX) {
			uint16_t freq;

			if (val < BG_CHAN_MIN)
				break;

			freq = wmic_ieee2freq(val);
			if (INVALID_FREQ == freq)
				break;
			else
				return freq;
		}

		if ((val == BG_FREQ_MAX) ||
		    ((val < BG_FREQ_MAX) && (val >= BG_FREQ_MIN)
		     && !((val - BG_FREQ_MIN) % 5)))
			return val;
		else if ((val >= A_FREQ_MIN) && (val < A_20MHZ_BAND_FREQ_MAX)
			 && !((val - A_FREQ_MIN) % 20))
			return val;
		else if ((val >= A_20MHZ_BAND_FREQ_MAX) && (val <= A_FREQ_MAX)
			 && !((val - A_20MHZ_BAND_FREQ_MAX) % 5))
			return val;
	} while (false);

	A_ERR(-1, "Invalid channel or freq #: %d !\n", val);
	return 0;
}

static uint32_t rateValid(uint32_t val, uint32_t freq)
{
	if (((freq >= A_FREQ_MIN) && (freq <= A_FREQ_MAX)
	     && (val >= A_RATE_NUM)) || ((freq >= BG_FREQ_MIN)
					 && (freq <= BG_FREQ_MAX)
					 && (val >= G_RATE_NUM))) {
		printf("Invalid rate value %d for frequency %d! \n", val, freq);
		prtRateTbl(freq);
		A_ERR(-1, "Invalid rate value %d for frequency %d! \n", val,
		      freq);
	}

	return val;
}

static void prtRateTbl(uint32_t freq)
{
	int i;

	for (i = 0; i < G_RATE_NUM; i++) {
		printf("<rate> %d \t \t %s \n", i, bgRateStrTbl[i]);
	}
	printf("\n");
}

/*
 * converts ieee channel number to frequency
 */
static uint16_t wmic_ieee2freq(uint32_t chan)
{
	if (chan == BG_CHAN_MAX) {
		return BG_FREQ_MAX;
	}
	if (chan < BG_CHAN_MAX) {	/* 0-13 */
		return (BG_CHAN0_FREQ + (chan * 5));
	}
	if (chan <= A_CHAN_MAX) {
		return (A_CHAN0_FREQ + (chan * 5));
	} else {
		return INVALID_FREQ;
	}
}

static uint32_t antValid(uint32_t val)
{
	if (val > 2) {
		A_ERR(-1,
		      "Invalid antenna setting! <0: auto;  1/2: ant 1/2>\n");
	}

	return val;
}

static uint32_t txPwrValid(TCMD_CONT_TX * txCmd)
{
	if (txCmd->mode == TCMD_CONT_TX_SINE) {
		if ((txCmd->txPwr >= 0) && (txCmd->txPwr <= 10))
			return txCmd->txPwr;
	} else if (txCmd->mode != TCMD_CONT_TX_OFF) {
		if ((txCmd->txPwr >= -15) && (txCmd->txPwr <= 30))
			return txCmd->txPwr;
	} else if (txCmd->mode == TCMD_CONT_TX_OFF) {
		return 0;
	}

	A_ERR(1,
	      "Invalid Tx Power value! \nTx data: [-15 - 14]dBm  \nTx sine: [-15 - 11]dBm  \n");
	return 0;
}

static uint32_t pktSzValid(uint32_t val)
{
	if ((val < 32) || (val > 1500)) {
		A_ERR(-1, "Invalid package size! < 32 - 1500 >\n");
	}
	return val;
}

#ifdef NOTYET

// Validate a hex character
static bool _is_hex(char c)
{
	return (((c >= '0') && (c <= '9')) ||
		((c >= 'A') && (c <= 'F')) || ((c >= 'a') && (c <= 'f')));
}

// Convert a single hex nibble
static int _from_hex(char c)
{
	int ret = 0;

	if ((c >= '0') && (c <= '9')) {
		ret = (c - '0');
	} else if ((c >= 'a') && (c <= 'f')) {
		ret = (c - 'a' + 0x0a);
	} else if ((c >= 'A') && (c <= 'F')) {
		ret = (c - 'A' + 0x0A);
	}
	return ret;
}

// Convert a character to lower case
static char _tolower(char c)
{
	if ((c >= 'A') && (c <= 'Z')) {
		c = (c - 'A') + 'a';
	}
	return c;
}

// Validate alpha
static bool isalpha(int c)
{
	return (((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')));
}

// Validate digit
static bool isdigit(int c)
{
	return ((c >= '0') && (c <= '9'));
}

// Validate alphanum
static bool isalnum(int c)
{
	return (isalpha(c) || isdigit(c));
}
#endif

/*------------------------------------------------------------------*/
/*
 * Input an Ethernet address and convert to binary.
 */
static int ath_ether_aton(const char *orig, uint8_t * eth)
{
	int mac[6];
	if (sscanf(orig, "%02x:%02x:%02X:%02X:%02X:%02X",
		   &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]) == 6) {
		int i;
#ifdef DEBUG
		if (*(orig + 12 + 5) != 0) {
			fprintf(stderr, "%s: trailing junk '%s'!\n", __func__,
				orig);
			return -1;
		}
#endif
		for (i = 0; i < 6; ++i)
			eth[i] = mac[i] & 0xff;
		return 0;
	}
	return -1;
}

/*
* Copyright (c) 2011-2012 Qualcomm Atheros Inc. All Rights Reserved.
* Qualcomm Atheros Proprietary and Confidential.
*/

#ifndef _LIBTCMD_H_
#define _LIBTCMD_H_

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
#include <net/if.h>
#include <signal.h>
#include <time.h>
#include <stdarg.h>

#define A_ERR(ret, args...) printf(args); exit(ret);
#define A_DBG(args...) fprintf(stderr, args);

#define TCMD_TIMEOUT 2	/* s */

enum tcmd_ep {
	TCMD_EP_TCMD,
	TCMD_EP_WMI,
};

struct tcmd_cfg {
	char iface[100];
	void (*rx_cb)(void *buf, int len);
	uint32_t ep;
#ifdef WLAN_API_NL80211
/* XXX: eventually default to libnl-2.0 API */
#ifdef LIBNL_2
#define nl_handle nl_sock
#endif
	struct nl_handle *nl_handle;
	int nl_id;
#endif
	struct sigevent sev;
	timer_t timer;
	bool timeout;
} tcmd_cfg;

/* WLAN API */
#ifdef WLAN_API_NL80211
#include "nl80211_drv.h"
#endif

/* send tcmd in buffer buf of length len. resp == true if a response by the FW
 * is required.  Returns: 0 on success, -ETIMEOUT on timeout
 */
int tcmd_tx(void *buf, int len, bool resp);

/* Initialize tcmd transport layer on given iface. Call given rx_cb on tcmd
 * response */
int tcmd_tx_init(char *iface, void (*rx_cb)(void *buf, int len));
/* same as above, but takes optional testmode endpoint (e.g. WMI vs. TCMD) */
int tcmd_init(char *iface, void (*rx_cb)(void *buf, int len), ...);
#endif /* _LIBTCMD_H_ */

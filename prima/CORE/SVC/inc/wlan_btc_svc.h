/******************************************************************************
 * wlan_btc_svc.h
 *
 ******************************************************************************/

#ifndef WLAN_BTC_SVC_H
#define WLAN_BTC_SVC_H 

void send_btc_nlink_msg (int type, int dest_pid);
int btc_activate_service(void *pAdapter);

#endif

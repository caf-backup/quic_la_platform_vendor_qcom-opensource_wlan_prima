#ifndef WLAN_HDD_FTM_H
#define WLAN_HDD_FTM_H

#include "vos_status.h"
#include "vos_mq.h"
#include "vos_api.h"
#include "msg.h"
#include "halTypes.h"
#include "vos_types.h"

#include <wlan_ptt_sock_svc.h>


#define WLAN_FTM_SUCCESS   0
#define WLAN_FTM_FAILURE   1


#define WLAN_FTM_START              1
#define WLAN_FTM_STOP               2        
#define WLAN_FTM_CMD                3


#define WLAN_FTM_PHY_CMD         100
#define SIR_HAL_FTM_CMD          10
#define QUALCOMM_MODULE_TYPE     2


typedef enum {
    WLAN_FTM_CMD_START = 1,
    WLAN_FTM_CMD_STOP,        
    WLAN_FTM_CMD_CMD
} wlan_hdd_ftm_cmds;

typedef struct ftm_hdr_s {
    v_U16_t cmd_id;

    v_U16_t data_len;

    v_U16_t respPktSize;
} ftm_hdr_t;


/* The request buffer of FTM which contains a byte of command and the request */
typedef struct wlan_hdd_ftm_payload_s {

    v_U16_t    ftm_cmd_type;
    v_U8_t    pFtmCmd[1];

}wlan_hdd_ftm_payload;

#define SIZE_OF_FTM_DIAG_HEADER_LEN 12
/* the FTM command/response structure */
typedef struct wlan_hdd_ftm_request_s
{
    v_U8_t    cmd_code;
    v_U8_t    sub_sys_id;
    v_U16_t   mode_id;
    ftm_hdr_t ftm_hdr; 
    v_U16_t   module_type;
    wlan_hdd_ftm_payload ftmpkt;
}wlan_hdd_ftm_request_t;


typedef struct wlan_hdd_ftm_response_s
{
    v_U8_t    cmd_code;
    v_U8_t    sub_sys_id;
    v_U16_t   mode_id;
    ftm_hdr_t ftm_hdr; 
    v_U16_t   ftm_err_code;
    wlan_hdd_ftm_payload ftmpkt;
}wlan_hdd_ftm_response_t;



typedef enum {
    WLAN_FTM_INITIALIZED,
    WLAN_FTM_STOPPED,
    WLAN_FTM_STARTED,
} wlan_hdd_ftm_state;

typedef struct wlan_hdd_ftm_status_s
{
    v_U8_t ftm_state;
    wlan_hdd_ftm_request_t    *pRequestBuf;
    wlan_hdd_ftm_response_t   *pResponseBuf;
    tAniNlHdr *wnl;
        /**vos event */
    vos_event_t  ftm_vos_event;
    v_BOOL_t  IsCmdPending;
} wlan_hdd_ftm_status_t;

typedef struct ftm_msg_s
{
    /* This field can be used as sequence 
        number/dialogue token for matching request/response */
    v_U16_t type;
    
    /* This guy carries the command buffer along with command id */
    void *cmd_ptr;

    v_U32_t bodyval;
} ftm_msg_t;

typedef struct ftm_rsp_msg_s
{
    v_U16_t   msgId;
    v_U16_t   msgBodyLength;
    v_U32_t   respStatus;
    v_U8_t   *msgResponse;
} ftm_rsp_msg_t;

#define FTM_SWAP16(A) ((((tANI_U16)(A) & 0xff00) >> 8) | \
                         (((tANI_U16)(A) & 0x00ff) << 8)   \
                      )

int wlan_hdd_ftm_open(hdd_adapter_t *pAdapter);
void wlan_hdd_process_ftm_cmd (hdd_adapter_t *pAdapter,tAniNlHdr *wnl);


#endif


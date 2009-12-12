/******************************************************************************
 * wlan_btc_svc.c
 *
 ******************************************************************************/

#include <wlan_nlink_srv.h>
#include <wlan_btc_svc.h>
#include <halTypes.h>
#include <vos_status.h>
#include <btcApi.h>
#include <wlan_hdd_includes.h>
#include <vos_trace.h>

// Global variables
static struct hdd_adapter_s *pAdapterHandle = NULL;

// Forward declrarion
static int btc_msg_callback (struct sk_buff * skb);

/*
 * Send a netlink message to the user space. 
 * Destination pid as zero implies broadcast
 */
void send_btc_nlink_msg (int type, int dest_pid)
{
   struct sk_buff *skb;
   struct nlmsghdr *nlh;
   tAniMsgHdr *aniHdr;
   tWlanAssocData *assocData;

   skb = alloc_skb(NLMSG_SPACE(WLAN_NL_MAX_PAYLOAD), GFP_KERNEL);
   if(skb == NULL) {
      VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
         "BTC: alloc_skb failed\n");
      return;
   }   

   nlh = (struct nlmsghdr *)skb->data;
   nlh->nlmsg_pid = 0;  /* from kernel */
   nlh->nlmsg_flags = 0;
   nlh->nlmsg_seq = 0;
   nlh->nlmsg_type = WLAN_NL_MSG_BTC;

   aniHdr = NLMSG_DATA(nlh);
   aniHdr->type = type;
   
   switch( type )
   {
      case WLAN_MODULE_UP_IND:
      case WLAN_MODULE_DOWN_IND:
      case WLAN_STA_DISASSOC_DONE_IND:
         aniHdr->length = 0; 
         nlh->nlmsg_len = NLMSG_LENGTH((sizeof(tAniMsgHdr)));
         skb_put(skb, NLMSG_SPACE(sizeof(tAniMsgHdr)));
         break;

      case WLAN_BTC_QUERY_STATE_RSP:
      case WLAN_STA_ASSOC_DONE_IND:
         aniHdr->length = sizeof(tWlanAssocData);
         nlh->nlmsg_len = NLMSG_LENGTH((sizeof(tAniMsgHdr) + sizeof(tWlanAssocData)));
         assocData = ( tWlanAssocData *)((char*)aniHdr + sizeof(tAniMsgHdr));
         if(hdd_connIsConnected(pAdapterHandle))
            assocData->channel = pAdapterHandle->conn_info.operationChannel;
         else
         assocData->channel = 0;
         skb_put(skb, NLMSG_SPACE((sizeof(tAniMsgHdr)+ sizeof(tWlanAssocData))));
         break;
      default:
         VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR, 
            "BTC: Attempt to send unknown nlink message %d\n", type);
         return;
   }

   if(dest_pid == 0)
      (void)nl_srv_bcast(skb);
   else
      (void)nl_srv_ucast(skb, dest_pid);
}

/*
 * Activate BTC handler. This will register a handler to receive
 * netlink messages addressed to WLAN_NL_MSG_BTC from user space
 */
int btc_activate_service(void *pAdapter)
{
   pAdapterHandle = (struct hdd_adapter_s*)pAdapter;  

   //Register the msg handler for msgs addressed to ANI_NL_MSG_BTC
   nl_srv_register(WLAN_NL_MSG_BTC, btc_msg_callback);
   return 0;
}

/*
 * Callback function invoked by Netlink service for all netlink
 * messages (from user space) addressed to WLAN_NL_MSG_BTC
 */
int btc_msg_callback (struct sk_buff * skb)
{
   struct nlmsghdr *nlh;
   tAniMsgHdr *msg_hdr;
   tSmeBtEvent *btEvent = NULL;

   nlh = (struct nlmsghdr *)skb->data;
   msg_hdr = NLMSG_DATA(nlh);
   
   /* Continue with parsing payload. */
   switch(msg_hdr->type)
   {
      case WLAN_BTC_QUERY_STATE_REQ:
         VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO, 
            "BTC: Received probe from BTC Service\n");
         send_btc_nlink_msg(WLAN_BTC_QUERY_STATE_RSP, nlh->nlmsg_pid);
         break;
      case WLAN_BTC_BT_EVENT_IND:
         VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO, 
            "BTC: Received Bluetooth event indication\n");
         if(msg_hdr->length != sizeof(tSmeBtEvent)) {
            VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
               "BTC: Size mismatch in BT event data\n");
            break;
         }
         btEvent = (tSmeBtEvent*)((char*)msg_hdr + sizeof(tAniMsgHdr));
         break;
      default:
         VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
            "BTC: Received Invalid Msg type [%d]\n", msg_hdr->type);
         break;
   }
   return 0;
}

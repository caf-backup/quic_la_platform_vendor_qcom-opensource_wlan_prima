#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <btces.h>
#include <wlan_btc_usr_svc.h>

pthread_mutex_t condition_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  condition_cond  = PTHREAD_COND_INITIALIZER;
btces_cb_type *g_cb_ptr = NULL;
void *g_user_data = NULL;

void inject_bt_event (btces_event_enum bt_event)
{
   btces_event_data_sync_comp_up_struct sync_conn;

   switch(bt_event)
   {
     case BTCES_EVENT_SYNC_CONNECTION_COMPLETE:
        sync_conn.addr.addr[0] = 0xFD;
        sync_conn.addr.addr[1] = 0xCB;
        sync_conn.addr.addr[2] = 0xA9;
        sync_conn.addr.addr[3] = 0x87;
        sync_conn.addr.addr[4] = 0x65;
        sync_conn.addr.addr[5] = 0x43;
        sync_conn.conn_handle = 2;
        sync_conn.conn_status = 4;
        sync_conn.link_type = 5;
        sync_conn.sco_interval = 6;
        sync_conn.sco_window =7;
        sync_conn.retrans_win = 8;
        g_cb_ptr(bt_event, (btces_event_data_union*)&sync_conn, g_user_data);
        break;

     default:
        break;
   }

}

BTCES_STATUS btces_register
(
  btces_cb_type *event_cb_ptr,
  void          *user_data
)
{
   fprintf(stdout, "btces_register called\n");
   g_cb_ptr = event_cb_ptr;
   g_user_data = user_data;
   inject_bt_event(BTCES_EVENT_SYNC_CONNECTION_COMPLETE);
   pthread_mutex_lock( &condition_mutex );
   pthread_cond_signal( &condition_cond );
   pthread_mutex_unlock( &condition_mutex );
   return BTCES_OK;
}

BTCES_STATUS btces_deregister
(
  void  **user_data_ptr
)

{
   fprintf(stdout, "btces_deregister called\n");
   return BTCES_OK;
}

BTCES_STATUS btces_state_report( void )
{
   return BTCES_OK;
}

BTCES_STATUS btces_wlan_chan( unsigned short channel_map)
{
   fprintf(stdout, "btces_wlan_chan called\n");
   return BTCES_OK;
}

btces_funcs btcesFunc = {
   .register_func = btces_register,
   .deregister_func = btces_deregister,
   .state_report_func = btces_state_report, 
   .wlan_chan_func = btces_wlan_chan
};

int main(int arg, char*argv[])
{
   
   if( (btc_svc_init(&btcesFunc)) ) {
     fprintf(stderr, "btc_svc_init failed\n");
   }
   else
   {
     pthread_mutex_lock( &condition_mutex );
     fprintf(stdout, "main thread going to sleep\n");
     pthread_cond_wait( &condition_cond, &condition_mutex );      
     fprintf(stdout, "main thread woke up\n");
     pthread_mutex_unlock( &condition_mutex );
   }

   btc_svc_deinit();
   
   return 0;
}

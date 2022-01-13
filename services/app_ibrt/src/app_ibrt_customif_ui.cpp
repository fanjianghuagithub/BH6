/***************************************************************************
 *
 * Copyright 2015-2019 BES.
 * All rights reserved. All unpublished rights reserved.
 *
 * No part of this work may be used or reproduced in any form or by any
 * means, or stored in a database or retrieval system, without prior written
 * permission of BES.
 *
 * Use of this work is governed by a license granted by BES.
 * This work contains confidential and proprietary information of
 * BES. which is protected by copyright, trade secret,
 * trademark and other intellectual property rights.
 *
 ****************************************************************************/
#include "cmsis_os.h"
#include <string.h>
#include "apps.h"
#include "app_tws_ibrt_trace.h"
#include "app_ibrt_if.h"
#include "app_ibrt_customif_ui.h"
#include "me_api.h"
#include "app_ibrt_ui.h"
#include "app_vendor_cmd_evt.h"
#include "besaud_api.h"
#include "app_battery.h"
#include "app_tws_ibrt_cmd_handler.h"
#include "app_tws_ctrl_thread.h"
#include "app_hfp.h"
#include "app_tws_if.h"
#include "app_bt_media_manager.h"
#include "app_spp.h"
#include "anc_wnr.h"
#include "app_anc.h"

#ifdef MEDIA_PLAYER_SUPPORT
#include "app_media_player.h"
#endif
#ifdef BLE_ENABLE
#include "app_ble_mode_switch.h"
#endif
#include "app_ibrt_customif_cmd.h"
#include "app_thread.h"
#include "a2dp_decoder.h"

#if defined(IBRT)
#ifdef ANC_APP
extern "C" void app_anc_sync_status(void);
extern "C" bool app_anc_work_status(void);
#endif


bool reconect_error_flg = true;
bool poweronautoanc_flg = true;
bool connected_succufll_flg = false;



static void disconnected_delay_timehandler(void const * param);
osTimerDef(DISCONNECTED_DELAY, disconnected_delay_timehandler);
static osTimerId       disconnected_delay_timer = NULL;


static void disconnected_delay_timehandler(void const * param)
{
	app_ibrt_ui_t *p_ibrt_ui = app_ibrt_ui_get_ctx();
	ibrt_ctrl_t *p_ibrt_ctrl = app_ibrt_if_get_bt_ctrl_ctx();

    if((p_ibrt_ui->active_event == IBRT_RECONNECT_EVENT)||(p_ibrt_ui->active_event == IBRT_PEER_RECONNECT_EVENT))
    {
           #if 0    
          if(p_ibrt_ctrl->current_role != IBRT_SLAVE)
          {
	             app_voice_report(APP_STATUS_INDICATION_ALEXA_STOP, 0);	 
		    // app_voice_report(APP_STATUS_INDICATION_BOTHSCAN,0);
		     app_status_indication_set(APP_STATUS_INDICATION_BOTHSCAN);  
	             app_start_10_second_timer(APP_POWEROFF_TIMER_ID);
          }
          #endif
    }
   else  if(p_ibrt_ctrl->current_role != IBRT_SLAVE)
   {
        if(app_ibrt_ui_get_enter_pairing_mode())
        {
            app_voice_report(APP_STATUS_INDICATION_BOTHSCAN,0);
	        app_status_indication_set(APP_STATUS_INDICATION_BOTHSCAN);	
        }
   }
}
void disconnected_delay_set_time(uint32_t millisec)
{
  if (disconnected_delay_timer == NULL)
  {
	 disconnected_delay_timer	= osTimerCreate(osTimer(DISCONNECTED_DELAY), osTimerOnce, NULL);
	 osTimerStop(disconnected_delay_timer);
  } 
     osTimerStart(disconnected_delay_timer, millisec);
}

void disconnected_delay_close_time(void)
{
  if (disconnected_delay_timer == NULL)
  {
	 disconnected_delay_timer	= osTimerCreate(osTimer(DISCONNECTED_DELAY), osTimerOnce, NULL);
	 osTimerStop(disconnected_delay_timer);
  } 
  else
  	{
         osTimerStop(disconnected_delay_timer);
  	}
}





static void ancsync_delay_timehandler(void const * param);
osTimerDef(ANCSYNC_GENSORDELAY, ancsync_delay_timehandler);
static osTimerId       ancsync_delay_timer = NULL;

void ancsync_Send_Cmd(void);
extern bool test_tws_connect_tone(void);
#ifdef __FJH_AUTO_ANC__
extern uint8_t auto_anc[4];
#endif
extern "C" uint8_t box_status;
static void ancsync_delay_timehandler(void const * param)
{
     //uint8_t testgamestatuscmd2 = 0;
    // uint8_t poweronautoanctest = 8;
    ibrt_ctrl_t *p_ibrt_ctrl = app_ibrt_if_get_bt_ctrl_ctx();


	if((box_status == 3))
		return;




	
#ifdef ANC_APP
					if(p_ibrt_ctrl->current_role == IBRT_MASTER)
					{
#ifdef ANC_APP
                app_anc_sync_status();
#endif
#ifdef ANC_WNR_ENABLED
                app_wnr_sync_state();
#endif
						#ifdef __FJH_AUTO_ANC__
						//app_ibrt_customif_test1_cmd_send(auto_anc,4);
						#endif		
					}
#endif
					if (p_ibrt_ctrl->current_role == IBRT_MASTER &&
						app_ibrt_ui_get_enter_pairing_mode())
					{

					      if(app_status_indication_get()!=APP_STATUS_INDICATION_BOTHSCAN)
					      {
#if defined(MEDIA_PLAYER_SUPPORT)
                                 if(reconect_error_flg == true)
                                 {
					              app_voice_report(APP_STATUS_INDICATION_BOTHSCAN, 0);
                                                
#endif                  
						          app_status_indication_set(APP_STATUS_INDICATION_BOTHSCAN);
                                  //  reconect_error_flg = false;
                                                  
						         }
					      }
					}
					else
					{
						if (p_ibrt_ctrl->current_role == IBRT_MASTER)
						{
						      if(app_ibrt_ui_get_reconnect_mobile_wait_time()!=5000)
						      {
						           if(app_status_indication_get()!=APP_STATUS_INDICATION_CLEARSUCCEED)
						           {
                                                              //  app_status_indication_set(APP_STATUS_INDICATION_CONNECTING);
						           }
							       else
					               {
                                                             //   app_stop_10_second_timer(APP_POWEROFF_TIMER_ID);
					               }
						      }
						}
						else
						{
						   TRACE(0,"%s",__func__);
						   app_stop_10_second_timer(APP_POWEROFF_TIMER_ID);
						   //if((app_status_indication_get()!=APP_STATUS_INDICATION_POWERON)&(app_status_indication_get()!=APP_STATUS_INDICATION_CONNECTED))
						   app_status_indication_set(APP_STATUS_INDICATION_INITIAL);
						}
					}	

                    if(poweronautoanc_flg == true)
                    {
                        if (p_ibrt_ctrl->current_role == IBRT_MASTER)
                        {
							//app_ibrt_customif_test1_cmd_send(&poweronautoanctest,1);		
							if(app_anc_work_status()==false)
							{
                                //app_anc_key(NULL,NULL);
							}
						}
					}
					poweronautoanc_flg = false;
					//reconect_error_flg = false;
					
}



void ancsync_delay_close_time()
{
	if (ancsync_delay_timer != NULL)
	{
	 //  ancsync_delay_timer	  = osTimerCreate(osTimer(ANCSYNC_GENSORDELAY), osTimerOnce, NULL);
	   osTimerStop(ancsync_delay_timer);
	} 
}



void ancsync_delay_set_time(uint32_t millisec)
{
// osStatus tt;
  if (ancsync_delay_timer == NULL)
  {
	 ancsync_delay_timer	= osTimerCreate(osTimer(ANCSYNC_GENSORDELAY), osTimerOnce, NULL);
	 osTimerStop(ancsync_delay_timer);
  } 
     osTimerStart(ancsync_delay_timer, millisec);
	 // TRACE(9,"osTimerStart:= %08x \n",tt);
}

void ancsync_Send_Cmd(void)
{
  APP_MESSAGE_BLOCK msg;
  msg.mod_id			= APP_MODUAL_ANCSYNC_DELAY;
  msg.msg_body.message_id = (uint32_t) NULL;
  msg.msg_body.message_ptr = (uint32_t) NULL;
  app_mailbox_put(&msg);
}




int app_ancsync_process(APP_MESSAGE_BODY * msg)
{
   ancsync_delay_set_time(1000);
   return 0;
}


void init_ancsync_thread(void)
{
	app_set_threadhandle(APP_MODUAL_ANCSYNC_DELAY, app_ancsync_process);
}

















void app_ibrt_customif_ui_vender_event_handler_ind(uint8_t evt_type, uint8_t *buffer, uint8_t length)
{
    uint8_t subcode = evt_type;
    POSSIBLY_UNUSED ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();

    switch (subcode)
    {
        case HCI_DBG_SNIFFER_INIT_CMP_EVT_SUBCODE:
            break;

        case HCI_DBG_IBRT_CONNECTED_EVT_SUBCODE:
            app_tws_if_ibrt_connected_handler();
            break;

        case HCI_DBG_IBRT_DISCONNECTED_EVT_SUBCODE:
            app_tws_if_ibrt_disconnected_handler();
            break;

        case HCI_DBG_IBRT_SWITCH_COMPLETE_EVT_SUBCODE:

            /*
             *  New Master do some special action,such as update battery to phone,
             * since TWS switch may lead to old TWS master update battery fail
             */
#if defined(SUPPORT_BATTERY_REPORT) || defined(SUPPORT_HF_INDICATORS)
            if(p_ibrt_ctrl->current_role == IBRT_MASTER)
            {
                uint8_t battery_level;
                TRACE(0,"New TWS master update battery report after tws switch");
                app_battery_get_info(NULL, &battery_level, NULL);
                app_hfp_battery_report(battery_level);
            }
#endif

            app_tws_if_tws_role_switch_complete_handler(p_ibrt_ctrl->current_role);
            break;

        case HCI_NOTIFY_CURRENT_ADDR_EVT_CODE:
            break;

        case HCI_DBG_TRACE_WARNING_EVT_CODE:
            break;

        case HCI_SCO_SNIFFER_STATUS_EVT_CODE:
            break;

        case HCI_DBG_RX_SEQ_ERROR_EVT_SUBCODE:
            break;

        case HCI_LL_MONITOR_EVT_CODE:
            break;

        case HCI_GET_TWS_SLAVE_MOBILE_RSSI_CODE:
            break;

        default:
            break;
    }
}
void app_ibrt_customif_ui_global_handler_ind(ibrt_link_type_e link_type, uint8_t evt_type, uint8_t status)
{
    ibrt_ctrl_t *p_ibrt_ctrl = app_ibrt_if_get_bt_ctrl_ctx();
//	app_ibrt_ui_t *p_ibrt_ui = app_ibrt_ui_get_ctx();



	TRACE(0,"-------> evt_type = %d  link_type = %d",evt_type,link_type);
	TRACE(0,"-------> app_ibrt_ui_get_reconnect_mobile_wait_time = %d",app_ibrt_ui_get_reconnect_mobile_wait_time());
	TRACE(0,"-------> p_ibrt_ctrl->current_role = %d",p_ibrt_ctrl->current_role);

    switch (evt_type)
    {

        case BTIF_BTEVENT_ACCESSIBLE_CHANGE:
			if(app_ibrt_ui_get_enter_pairing_mode())
			{
                             if (p_ibrt_ctrl->current_role == IBRT_SLAVE)
			     {
			  	    if((app_status_indication_get()==APP_STATUS_INDICATION_CONNECTED)||(app_status_indication_get()==APP_STATUS_INDICATION_DISCONNECTED))
					{
			          // app_voice_report(APP_STATUS_INDICATION_DISCONNECTED, 0);
					}
			     }
			     else if((app_status_indication_get()!=APP_STATUS_INDICATION_BOTHSCAN)&(app_status_indication_get()!=APP_STATUS_INDICATION_CONNECTED))
				 {
				   /*
                            app_status_indication_set(APP_STATUS_INDICATION_BOTHSCAN);

				   if (p_ibrt_ctrl->current_role != IBRT_SLAVE)
				   {
				      app_voice_report(APP_STATUS_INDICATION_BOTHSCAN,0);
				   }
				   app_start_10_second_timer(APP_POWEROFF_TIMER_ID);
				   */
                                if(reconect_error_flg)
                                {
				              app_voice_report(APP_STATUS_INDICATION_BOTHSCAN,0);
					      app_status_indication_set(APP_STATUS_INDICATION_BOTHSCAN);
                                              reconect_error_flg = false;
					 }
				 }
			}
			break;







	
        case BTIF_BTEVENT_LINK_CONNECT_CNF:// An outgoing ACL connection is up
        //fall through
        case BTIF_BTEVENT_LINK_CONNECT_IND://An incoming ACL connection is up
            if (MOBILE_LINK == link_type)
            {
                if (BTIF_BEC_NO_ERROR == status)
                {
                    app_tws_if_mobile_connected_handler(p_ibrt_ctrl->mobile_addr.address);
                    if(p_ibrt_ctrl->current_role == IBRT_SLAVE)
		   {
		           if(app_tws_ibrt_slave_ibrt_link_connected())
			   {
                                                  // app_status_indication_set(APP_STATUS_INDICATION_CONNECTED);
						  // app_stop_10_second_timer(APP_POWEROFF_TIMER_ID);
			  }
		   }
		   else
		   {
                           if(app_tws_ibrt_mobile_link_connected())
		           {
				      if(app_status_indication_get()!=APP_STATUS_INDICATION_CONNECTED)
				      {

				     }
		           }
		   }	
                }
            }
            break;
        case BTIF_BTEVENT_LINK_DISCONNECT:
            if (!app_tws_ibrt_mobile_link_connected())
            {
                app_ibrt_if_sniff_checker_reset();
            }
            if (MOBILE_LINK == link_type)
            {
                app_audio_manager_sendrequest(APP_BT_STREAM_MANAGER_STOP, BT_STREAM_VOICE, BT_DEVICE_ID_1, MAX_RECORD_NUM);
                app_audio_manager_sendrequest(APP_BT_STREAM_MANAGER_STOP, BT_STREAM_SBC, BT_DEVICE_ID_1, MAX_RECORD_NUM);
                app_tws_if_mobile_disconnected_handler(p_ibrt_ctrl->mobile_addr.address);                
                {
		   #if 0			
                   if((p_ibrt_ui->active_event == IBRT_RECONNECT_EVENT)||(p_ibrt_ui->active_event == IBRT_PEER_RECONNECT_EVENT))
                   {
                           disconnected_delay_set_time(10000);
		   }
		   else if(p_ibrt_ctrl->current_role == IBRT_MASTER)  
		   {
			              app_voice_report(APP_STATUS_INDICATION_DISCONNECTED, 0);	 
				      app_voice_report(APP_STATUS_INDICATION_BOTHSCAN,0);
			              app_status_indication_set(APP_STATUS_INDICATION_BOTHSCAN);  
				      app_start_10_second_timer(APP_POWEROFF_TIMER_ID);
		   }
		   #endif
           if(connected_succufll_flg == true)
           {
		     if(p_ibrt_ctrl->current_role != IBRT_SLAVE) 
		     {
               app_voice_report(APP_STATUS_INDICATION_DISCONNECTED, 0);	
			   app_start_10_second_timer(APP_POWEROFF_TIMER_ID);		
			   disconnected_delay_set_time(3000);
		     }
		       app_status_indication_set(APP_STATUS_INDICATION_DISCONNECTED);
		       connected_succufll_flg = false;
           }

		   
           }		
            }
            if (TWS_LINK == link_type)
            {
#ifdef MEDIA_PLAYER_SUPPORT
                app_tws_sync_prompt_manager_reset();
#endif

            }
            break;

			
        case BTIF_STACK_LINK_DISCONNECT_COMPLETE:     
			if (MOBILE_LINK == link_type)
			{ 
			 if (p_ibrt_ctrl->current_role != IBRT_SLAVE)
			 {
			   if (!app_tws_ibrt_mobile_link_connected())
			   {
			      if((app_status_indication_get()==APP_STATUS_INDICATION_CONNECTED))
				  {

				  } 
			   }
			 }
			 else
			 {
                     //app_voice_report(APP_STATUS_INDICATION_DISCONNECTED, 0);
					 //app_start_10_second_timer(APP_POWEROFF_TIMER_ID);

			 }
			}


                       if (TWS_LINK == link_type)
			{
			  // ibrt_ctrl_t *p_ibrt_ctrl=NULL;
			 
			  // p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
			
			  p_ibrt_ctrl->audio_chnl_sel = A2DP_AUDIO_CHANNEL_SELECT_LRMERGE;
			  //app_voice_report(APP_STATUS_INDICATION_DUDU, 0);
			  a2dp_audio_ch_init(A2DP_AUDIO_CHANNEL_SELECT_LRMERGE);

                          //TWS¶Ï¿ª.....
                    if (!app_tws_ibrt_mobile_link_connected())
                    {
 			           app_status_indication_set(APP_STATUS_INDICATION_DISCONNECTED);
			           app_start_10_second_timer(APP_POWEROFF_TIMER_ID);            
                    }  
			}	
            break;

        case BTIF_BTEVENT_ROLE_CHANGE:
            break;

        case BTIF_BTEVENT_BES_AUD_CONNECTED:
        {
            //tws link callback when besaud connection complete
            if (BTIF_BEC_NO_ERROR == status)
            {
		twspairfaill_clear();


ancsync_Send_Cmd();
#if 0
			
#ifdef ANC_APP
                app_anc_sync_status();
#endif
#ifdef ANC_WNR_ENABLED
                app_wnr_sync_state();
#endif
                if (p_ibrt_ctrl->current_role == IBRT_MASTER &&
                    app_ibrt_ui_get_enter_pairing_mode())
                {
#if defined(MEDIA_PLAYER_SUPPORT)
                    app_voice_report(APP_STATUS_INDICATION_CONNECTED, 0);
#endif
                    app_status_indication_set(APP_STATUS_INDICATION_BOTHSCAN);
                }
                else
                {
                    app_status_indication_set(APP_STATUS_INDICATION_CONNECTED);
                }
				#endif

                app_tws_if_tws_connected_handler();
            }

        }
        break;

        case BTIF_BTEVENT_BES_AUD_DISCONNECTED:
            app_tws_if_tws_disconnected_handler();
            break;

        case BTIF_BTEVENT_ENCRYPTION_CHANGE:
            break;

        case BTIF_BTEVENT_MODE_CHANGE:
            break;

        default:
            break;
    }
}
void app_ibrt_customif_open_box_complete_ind(void)
{

}
void app_ibrt_customif_close_box_complete_ind(void)
{

}
void app_ibrt_customif_fetch_out_complete_ind(void)
{

}
void app_ibrt_customif_put_in_complete_ind(void)
{

}
void app_ibrt_customif_wear_up_complete_ind(void)
{

}
void app_ibrt_customif_wear_down_complete_ind(void)
{

}
void app_ibrt_customif_ui_global_event_update(ibrt_event_type evt_type, ibrt_ui_state_e old_state, \
        ibrt_ui_state_e new_state,ibrt_action_e   action,\
        ibrt_ui_error_e status)
{
    switch (new_state)
    {
        case IBRT_UI_IDLE:
            if (IBRT_UI_IDLE != old_state)
            {
                //callback when UI event completed
                switch (evt_type)
                {
                    case IBRT_OPEN_BOX_EVENT:
                        app_ibrt_customif_open_box_complete_ind();
                        break;
                    case IBRT_FETCH_OUT_EVENT:
                        app_ibrt_customif_fetch_out_complete_ind();
                        break;
                    case IBRT_PUT_IN_EVENT:
                        app_ibrt_customif_put_in_complete_ind();
                        break;
                    case IBRT_CLOSE_BOX_EVENT:
                        app_ibrt_customif_close_box_complete_ind();
                        break;
                    case IBRT_WEAR_UP_EVENT:
                        app_ibrt_customif_wear_up_complete_ind();
                        break;
                    case IBRT_WEAR_DOWN_EVENT:
                        app_ibrt_customif_wear_down_complete_ind();
                        break;
                    default:
                        break;
                }
            }
            break;

        case IBRT_UI_IDLE_WAIT:
            break;

        case IBRT_UI_W4_TWS_CONNECTION:
            break;

        case IBRT_UI_W4_TWS_INFO_EXCHANGE_COMPLETE:
            break;

        case IBRT_UI_W4_TWS_BT_MSS_COMPLETE:
            break;

        case IBRT_UI_W4_SET_ENV_COMPLETE:
            break;

        case IBRT_UI_W4_MOBILE_CONNECTION:
            break;

        case IBRT_UI_W4_MOBILE_MSS_COMPLETE:
            break;

        case IBRT_UI_W4_MOBILE_ENTER_ACTIVE_MODE:
            break;

        case IBRT_UI_W4_START_IBRT_COMPLETE:
            break;

        case IBRT_UI_W4_IBRT_DATA_EXCHANGE_COMPLETE:
            break;

        case IBRT_UI_W4_TWS_SWITCH_COMPLETE:
            break;

        case IBRT_UI_W4_SM_STOP:
            break;

        default:
            break;
    }
}
/*
* custom tws switch interface
* tws switch cmd send sucess, return true, else return false
*/
bool app_ibrt_customif_ui_tws_switch(void)
{
    return app_ibrt_ui_tws_switch();
}

/*
* custom tws switching check interface
* whether doing tws switch now, return true, else return false
*/
bool app_ibrt_customif_ui_is_tws_switching(void)
{
    return app_ibrt_ui_is_tws_switching();
}

/*
* custom reconfig bd_addr
*/
void app_ibrt_customif_ui_reconfig_bd_addr(bt_bdaddr_t local_addr, bt_bdaddr_t peer_addr, ibrt_role_e nv_role)
{
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    app_ibrt_ui_t *p_ibrt_ui = app_ibrt_ui_get_ctx();

    p_ibrt_ctrl->local_addr = local_addr;
    p_ibrt_ctrl->peer_addr  = peer_addr;
    p_ibrt_ctrl->nv_role    = nv_role;

    if (!p_ibrt_ctrl->is_ibrt_search_ui)
    {
        if (IBRT_MASTER == p_ibrt_ctrl->nv_role)
        {
            p_ibrt_ctrl->peer_addr = local_addr;
            btif_me_set_bt_address(p_ibrt_ctrl->local_addr.address);
        }
        else if (IBRT_SLAVE == p_ibrt_ctrl->nv_role)
        {
            p_ibrt_ctrl->local_addr = peer_addr;
            btif_me_set_bt_address(p_ibrt_ctrl->local_addr.address);
        }
        else
        {
            ASSERT(0, "%s nv_role error", __func__);
        }
    }
    p_ibrt_ui->bonding_success = true;
}
/*custom can block connect mobile if needed*/
bool app_ibrt_customif_connect_mobile_needed_ind(void)
{
    return true;
}

void app_ibrt_customif_mobile_connected_ind(bt_bdaddr_t * addr)
{
#ifdef BLE_ENABLE
    app_ble_refresh_adv_state(BLE_ADVERTISING_INTERVAL);
#endif

    app_ibrt_if_config_keeper_mobile_update(addr);
}

void app_ibrt_customif_ibrt_connected_ind(bt_bdaddr_t * addr)
{
    app_ibrt_if_config_keeper_mobile_update(addr);
}

void app_ibrt_customif_tws_connected_ind(bt_bdaddr_t * addr)
{
    app_ibrt_if_config_keeper_tws_update(addr);
}

void app_ibrt_customif_profile_state_change_ind(uint32_t profile,uint8_t connected)
{
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    TRACE(2,"custom if profle=%x state change to =%x",profile,connected);

    switch (profile)
    {
        case BTIF_APP_A2DP_PROFILE_ID:
            if (connected)
            {
                //TRACE(0,"cutomif A2DP profile connected");
                //ibrt slave
                if (p_ibrt_ctrl->current_role == IBRT_SLAVE)
                {
                    //TO DO
                }
                //ibrt master
                else if (p_ibrt_ctrl->current_role == IBRT_MASTER)
                {
                    //TO DO
                }
                //no tws connected
                else
                {
                    //TO DO
                }
            }
            else
            {
                //TRACE(0,"cutomif A2DP profile disconnected");
                //ibrt slave
                if (p_ibrt_ctrl->current_role == IBRT_SLAVE)
                {
                    //TO DO
                }
                //ibrt master
                else if (p_ibrt_ctrl->current_role == IBRT_MASTER)
                {
                    //TO DO
                }
                //no tws connected
                else
                {
                    //TO DO
                }
            }
            break;

        case BTIF_APP_AVRCP_PROFILE_ID:
            if (connected)
            {
                //TRACE(0,"cutomif AVRCP profile connected");
                //ibrt slave
                if (p_ibrt_ctrl->current_role == IBRT_SLAVE)
                {
                    //TO DO
                }
                //ibrt master
                else if (p_ibrt_ctrl->current_role == IBRT_MASTER)
                {
                    //TO DO
                }
                //no tws connected
                else
                {
                    //TO DO
                }
            }
            else
            {
                //TRACE(0,"cutomif AVRCP profile disconnected");
                //ibrt slave
                if (p_ibrt_ctrl->current_role == IBRT_SLAVE)
                {
                    //TO DO
                }
                //ibrt master
                else if (p_ibrt_ctrl->current_role == IBRT_MASTER)
                {
                    //TO DO
                }
                //no tws connected
                else
                {
                    //TO DO
                }
            }
            break;

        case BTIF_APP_HFP_PROFILE_ID:
            if (connected)
            {
                //TRACE(0,"cutomif HFP profile connected");
                //ibrt slave
                if (p_ibrt_ctrl->current_role == IBRT_SLAVE)
                {
                    //TO DO
                    reconect_error_flg = false;
                    app_status_indication_set(APP_STATUS_INDICATION_CONNECTED);
		            app_stop_10_second_timer(APP_POWEROFF_TIMER_ID);
			        connected_succufll_flg = true;		
					
                }
                //ibrt master
                else if (p_ibrt_ctrl->current_role == IBRT_MASTER)
                {
                    //TO DO
                          reconect_error_flg = false;
					// if(app_status_indication_get()!=APP_STATUS_INDICATION_CLEARSUCCEED)
					 {
                          app_status_indication_set(APP_STATUS_INDICATION_CONNECTED);
					      app_voice_report(APP_STATUS_INDICATION_CONNECTED, 0);
					      app_stop_10_second_timer(APP_POWEROFF_TIMER_ID);
                          test_power_on_connected();
					 }
					// else
					// {
                                             //  app_stop_10_second_timer(APP_POWEROFF_TIMER_ID);

					// }
					connected_succufll_flg = true;
						  
                }
                //no tws connected
                else
                {
                    //TO DO


					app_status_indication_set(APP_STATUS_INDICATION_CONNECTED);
					app_voice_report(APP_STATUS_INDICATION_CONNECTED, 0);
					app_stop_10_second_timer(APP_POWEROFF_TIMER_ID);
					test_power_on_connected();

					
                    connected_succufll_flg = true;
                }
            }
            else
            {
                //TRACE(0,"cutomif HFP profile disconnected");
                //ibrt slave
                if (p_ibrt_ctrl->current_role == IBRT_SLAVE)
                {
                    //TO DO
  			          //app_voice_report(APP_STATUS_INDICATION_DISCONNECTED, 0);
					  app_status_indication_set(APP_STATUS_INDICATION_DISCONNECTED);
					  app_start_10_second_timer(APP_POWEROFF_TIMER_ID);                  
                }
                //ibrt master
                else if (p_ibrt_ctrl->current_role == IBRT_MASTER)
                {
                    #if  0
                    //TO DO
			          app_voice_report(APP_STATUS_INDICATION_DISCONNECTED, 0);
					//  app_status_indication_set(APP_STATUS_INDICATION_DISCONNECTED);
					 
				      app_voice_report(APP_STATUS_INDICATION_BOTHSCAN,0);
					  app_status_indication_set(APP_STATUS_INDICATION_BOTHSCAN);
					  
					  app_start_10_second_timer(APP_POWEROFF_TIMER_ID);
					  #endif
                }
                //no tws connected
                else
                {
                    //TO DO
					  app_status_indication_set(APP_STATUS_INDICATION_DISCONNECTED);
					  app_start_10_second_timer(APP_POWEROFF_TIMER_ID);        
                }

            }
            break;

        case BTIF_APP_SPP_CLIENT_AI_VOICE_ID:
        case BTIF_APP_SPP_SERVER_AI_VOICE_ID:
        case BTIF_APP_SPP_SERVER_GREEN_ID:
        case BTIF_APP_SPP_CLIENT_CCMP_ID:
        case BTIF_APP_SPP_CLIENT_RED_ID:
        case BTIF_APP_SPP_SERVER_RED_ID:
        case BTIF_APP_SPP_SERVER_TOTAD_ID:
        case BTIF_APP_SPP_SERVER_GSOUND_CTL_ID:
        case BTIF_APP_SPP_SERVER_GSOUND_AUD_ID:
        case BTIF_APP_SPP_SERVER_BES_OTA_ID:
        case BTIF_APP_SPP_SERVER_FP_RFCOMM_ID:
            if (connected)
            {
                //TRACE(0,"cutomif SPP profile connected");
                //ibrt slave
                if (p_ibrt_ctrl->current_role == IBRT_SLAVE)
                {
                    //TO DO
                }
                //ibrt master
                else if (p_ibrt_ctrl->current_role == IBRT_MASTER)
                {
                    //TO DO
                }
                //no tws connected
                else
                {
                    //TO DO
                }

            }
            else
            {
                //TRACE(0,"cutomif SPP profile disconnected");
                //ibrt slave
                if (p_ibrt_ctrl->current_role == IBRT_SLAVE)
                {
                    //TO DO
                }
                //ibrt master
                else if (p_ibrt_ctrl->current_role == IBRT_MASTER)
                {
                    //TO DO
                }
                //no tws connected
                else
                {
                    //TO DO
                }

            }
            break;

        default:
            TRACE(1,"unknown profle=%x state change",profile);
            break;
    }

}

void app_ibrt_customif_ui_pairing_set(trigger_pairing_mode_type_e trigger_type)
{
    TRACE(2,"%s: %d", __func__, __LINE__);
}

void app_ibrt_customif_ui_pairing_clear(trigger_pairing_mode_type_e trigger_type)
{
    TRACE(2,"%s: %d", __func__, __LINE__);
}

/*
* custom config main function
*/

int app_ibrt_customif_ui_start(void)
{
    ibrt_ui_config_t config;

    // zero init the config
    memset(&config, 0, sizeof(ibrt_ui_config_t));

    //freeman mode config, default should be false
    config.freeman_enable                           = false;

    //tws earphone set the same addr, UI will be flexible, default should be true
    config.tws_use_same_addr                        = true;

    //ibrt slave will reconnect to mobile if tws connect failed, default should be true
    config.slave_reconnect_enable                   = true;

    //do tws switch when wearup or weardown, must be true because MIC will be with IBRT master
    config.wear_updown_tws_switch_enable            = true;

    //pairing mode default value, default should be set false
    config.enter_pairing_mode                       = false;

    //following cases the reconnect will be fail for freeman, please set to true if you want to reconnect successful:
    //1. freeman has link key but mobile deleted the link key
    //2. freeman changed its bt address after reboot and use the new address to reconnect mobile
    config.freeman_accept_mobile_new_pairing        = false;

    //for some proj no box key, default should be false;
    config.enter_pairing_on_empty_mobile_addr       = true;

    //for some proj no box key, default should be false
    config.enter_pairing_on_reconnect_mobile_failed = true;

    //when mobile has connected, enter_pairing_on_reconnect_mobile_failed will be cleared, default false
    config.enter_pairing_on_reconnect_mobile_failed_once = false;

    //for some proj no box key, default should be false
    config.enter_pairing_on_mobile_disconnect       = true;

    //for 08 error reconnect event, default must be true
    config.disc_tws_before_reconnect_mobile         = true;
    config.wait_time_before_disc_tws                = 3000;

    //do tws switch when RSII value change, default should be true
    config.tws_switch_according_to_rssi_value       = false;

    //disable tws switch, NOT recommended to open
    config.disable_tws_switch                       = false;

    //disable tws switch, NOT recommended to open
    config.disable_stop_ibrt                        = true;

    //exchange snoop info by BLE_box, special custom config, default should be false
    config.snoop_via_ble_enable                     = false;
    //controller basband monitor
    config.lowlayer_monitor_enable                  = false;

    config.delay_exit_sniff                         = true;
    config.delay_ms_exit_sniff                      = 3000;

    config.check_plugin_excute_closedbox_event      = true;

    config.share_tws_info_done                      = false;

    config.nv_slave_enter_pairing_on_mobile_disconnect      = false;
    config.nv_slave_enter_pairing_on_empty_mobile_addr      = true;

    //only allow paired mobile device incoming when not in paring mode,default should be false
    config.mobile_incoming_filter_unpaired          = false;
    //close box debounce time config
    config.close_box_event_wait_response_timeout          = IBRT_UI_CLOSE_BOX_EVENT_WAIT_RESPONSE_TIMEOUT;

    //do tws switch when RSII value change, timer threshold
    config.role_switch_timer_threshold                    = IBRT_UI_ROLE_SWITCH_TIME_THRESHOLD;

    //do tws switch when rssi value change over threshold
    config.rssi_threshold                                 = IBRT_UI_ROLE_SWITCH_THRESHOLD_WITH_RSSI;

    //wait time before launch reconnect event
    config.reconnect_mobile_wait_response_timeout         = IBRT_UI_RECONNECT_MOBILE_WAIT_RESPONSE_TIMEOUT;

    //reconnect event internal config wait timer when tws disconnect
    config.reconnect_wait_ready_timeout                   = IBRT_UI_MOBILE_RECONNECT_WAIT_READY_TIMEOUT;
    config.reconnect_mobile_wait_ready_timeout            = IBRT_UI_MOBILE_RECONNECT_WAIT_READY_TIMEOUT;
    config.reconnect_tws_wait_ready_timeout               = IBRT_UI_TWS_RECONNECT_WAIT_READY_TIMEOUT;
    config.reconnect_ibrt_wait_response_timeout           = IBRT_UI_RECONNECT_IBRT_WAIT_RESPONSE_TIMEOUT;
    config.nv_master_reconnect_tws_wait_response_timeout  = IBRT_UI_NV_MASTER_RECONNECT_TWS_WAIT_RESPONSE_TIMEOUT;
    config.nv_slave_reconnect_tws_wait_response_timeout   = IBRT_UI_NV_SLAVE_RECONNECT_TWS_WAIT_RESPONSE_TIMEOUT;

    //pairing mode timeout config
    config.disable_bt_scan_timeout                        = IBRT_UI_DISABLE_BT_SCAN_TIMEOUT;

    //open box reconnect mobile times config
    config.open_reconnect_mobile_max_times                = IBRT_UI_OPEN_RECONNECT_MOBILE_MAX_TIMES;

    //open box reconnect tws times config
    config.open_reconnect_tws_max_times                   = IBRT_UI_OPEN_RECONNECT_TWS_MAX_TIMES;

    //connection timeout reconnect mobile times config
    config.reconnect_mobile_max_times                     = IBRT_UI_RECONNECT_MOBILE_MAX_TIMES;

    //connection timeout reconnect tws times config
    config.reconnect_tws_max_times                        = IBRT_UI_RECONNECT_TWS_MAX_TIMES;

    //connection timeout reconnect ibrt times config
    config.reconnect_ibrt_max_times                       = IBRT_UI_RECONNECT_IBRT_MAX_TIMES;

    //reconnect tws one cycle
    config.tws_reconnect_cycle                            = IBRT_TWS_RECONNECT_ONE_CYCLE;

    //reconnect mobile one cycle
    config.mobile_reconnect_cycle                         = IBRT_MOBILE_RECONNECT_ONE_CYCLE;

    //BES internal config, DO NOT modify
    config.long_private_poll_interval                     = IBRT_UI_LONG_POLL_INTERVAL;
    config.default_private_poll_interval                  = IBRT_UI_DEFAULT_POLL_INTERVAL;
    config.short_private_poll_interval                    = IBRT_UI_SHORT_POLL_INTERVAL;
    config.default_private_poll_interval_in_sco           = IBRT_UI_DEFAULT_POLL_INTERVAL_IN_SCO;
    config.short_private_poll_interval_in_sco             = IBRT_UI_SHORT_POLL_INTERVAL_IN_SCO;
    config.default_bt_tpoll                               = IBRT_TWS_BT_TPOLL_DEFAULT;

    //for fast connect when only one headset in the nearby
    config.tws_page_timeout_on_last_success               = IBRT_TWS_PAGE_TIMEOUT_ON_CONNECT_SUCCESS_LAST;
    config.tws_page_timeout_on_last_failed                = IBRT_TWS_PAGE_TIMEOUT_ON_CONNECT_FAILED_LAST;
    config.mobile_page_timeout                            = IBRT_MOBILE_PAGE_TIMEOUT;
    config.tws_page_timeout_on_reconnect_mobile_failed    = IBRT_TWS_PAGE_TIMEOUT_ON_RECONNECT_MOBILE_FAILED;
    config.tws_page_timeout_on_reconnect_mobile_success   = IBRT_TWS_PAGE_TIMEOUT_ON_RECONNECT_MOBILE_SUCCESS;

    //tws connection timeout
    config.tws_connection_timeout                         = IBRT_UI_TWS_CONNECTION_TIMEOUT;

    config.rx_seq_error_timeout                           = IBRT_UI_RX_SEQ_ERROR_TIMEOUT;
    config.rx_seq_error_threshold                         = IBRT_UI_RX_SEQ_ERROR_THRESHOLD;
    config.rx_seq_recover_wait_timeout                    = IBRT_UI_RX_SEQ_ERROR_RECOVER_TIMEOUT;

    config.rssi_monitor_timeout                           = IBRT_UI_RSSI_MONITOR_TIMEOUT;


    config.wear_updown_detect_supported                   = false;
    config.stop_ibrt_timeout                              = IBRT_UI_STOP_IBRT_TIMEOUT;


    config.radical_scan_interval_nv_slave                 = IBRT_UI_RADICAL_SAN_INTERVAL_NV_SLAVE;
    config.radical_scan_interval_nv_master                = IBRT_UI_RADICAL_SAN_INTERVAL_NV_MASTER;
    config.event_hung_timeout                             = IBRT_EVENT_HUNG_TIMEOUT;
    config.rssi_tws_switch_threshold                      = IBRT_TWS_SWITCH_RSSI_THRESHOLD;
    config.stop_ibrt_wait_time_after_tws_switch           = IBRT_STOP_IBRT_WAIT_TIME;
    config.tws_conn_failed_wait_time                      = TWS_CONN_FAILED_WAIT_TIME;

    config.sm_running_timeout                             = SM_RUNNING_TIMEOUT;
    config.peer_sm_running_timeout                        = PEER_SM_RUNNING_TIMEOUT;
    config.reconnect_peer_sm_running_timeout              = RECONNECT_PEER_SM_RUNNING_TIMEOUT;
    config.connect_no_03_timeout                          = CONNECT_NO_03_TIMEOUT;
    config.disconnect_no_05_timeout                       = DISCONNECT_NO_05_TIMEOUT;

    config.tws_switch_tx_data_protect                     = true;
    config.tws_cmd_send_timeout                           = IBRT_UI_TWS_CMD_SEND_TIMEOUT;
    config.tws_cmd_send_counter_threshold                 = IBRT_UI_TWS_COUNTER_THRESHOLD;
    config.tws_switch_stable_timeout                      = IBRT_UI_TWS_SWITCH_STABLE_TIMEOUT;

    config.invoke_event_when_box_closed                   = true;


    //if open_box/close box detect supported, may open this config to speed up connection setup
    config.tws_stay_when_close_box                        = false;
    config.free_tws_timeout                               = IBRT_UI_DISC_TWS_TIMEOUT;

    config.profile_concurrency_supported                  = false;

    config.audio_sync_mismatch_resume_version              = 2;

    app_ibrt_if_register_global_handler_ind(app_ibrt_customif_ui_global_handler_ind);
    app_ibrt_if_register_vender_handler_ind(app_ibrt_customif_ui_vender_event_handler_ind);
    app_ibrt_if_register_global_event_update_ind(app_ibrt_customif_ui_global_event_update);
    app_ibrt_if_register_link_connected_ind(app_ibrt_customif_mobile_connected_ind,
                                            app_ibrt_customif_ibrt_connected_ind,
                                            app_ibrt_customif_tws_connected_ind);
    app_ibrt_if_register_profile_state_change_ind(app_ibrt_customif_profile_state_change_ind);
    app_ibrt_if_register_connect_mobile_needed_ind(app_ibrt_customif_connect_mobile_needed_ind);
    app_ibrt_if_register_pairing_mode_ind(app_ibrt_customif_ui_pairing_set, app_ibrt_customif_ui_pairing_clear);
    app_ibrt_if_config(&config);

    if (config.delay_exit_sniff)
    {
        app_ibrt_if_sniff_checker_init(config.delay_ms_exit_sniff);
    }

    return 0;
}

#endif

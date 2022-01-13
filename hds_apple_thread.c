//========================================================================
//  add by guyu for laiyuan mcu communication
//  time:2020-6-30
//========================================================================

#define __HDS_THREAD_C_

#include "cmsis_os.h"
#include "hal_i2c.h"
#include "hal_trace.h"
#include "pmu.h"
#include "app_box.h"
#include "hds_apply_thread.h"
#include "hds_sys_status_manage.h"
#include "app_thread.h"
#include "app_ibrt_ui.h"
//#include "app_tws_if.h"
#include "apps.h"
#include "app_status_ind.h"
//#include "app_ibrt_if.h"
#include "nvrecord.h"
#include "nvrecord_dev.h"
#include "nvrecord_env.h"
#include "norflash_api.h"
#include "apps.h"
#include "app_anc.h"
#include "app_battery.h"
#include "factory_section.h"
#include "communication_svr.h"
#include "app_ibrt_ui_test.h"
//#include "app_ibrt_if.h"
#include "app_ibrt_ui_test.h"
#include "app_ibrt_customif_ui.h"
#include "app_tws_if.h"



extern void analog_aud_codec_nomute(void);
extern void analog_aud_codec_mute(void);

extern void app_singleline_enter(APP_KEY_STATUS *status, void *param);
extern void Enter_sniff_mode(void);
extern void UI_SWITCH_EN(void);
extern bool sniff_enter_flg;
//extern uint8_t sniff_enter_num;
extern bool box_cmd_enable;
//#ifdef __FJH_BT_PAIR_RESER__
extern bool EnterBTpairFlg;
//#endif
extern bool Featch_out_flg;
extern bool Auto_Cang_Power_Off;
extern bool cangout_poweroff;
extern uint8_t  app_poweroff_flag;
//extern uint8_t oldancmode;
//extern bool oldancmode_flg;

bool tws_role_switch_flg = false;
#ifdef __FJH_AUTO_ANC__
extern uint8_t auto_anc[4];
#endif
extern bool EnterBTsingpairFlg;
extern bool EnterBTpairFlg;

uint8_t msgid_fjh = 0xff;

static void hds_delay_timehandler(void const * param);
osTimerDef(HDS_DELAY, hds_delay_timehandler);
static osTimerId       hds_delay_timer = NULL;
static void hds_thread_msg_answer_btaddr(uint32_t param1,uint32_t param2);
//static void hds_thread_msg_finish_answer(uint32_t param1,uint32_t param2);
void hds_delay_msg_add(uint8_t eventmode,uint16_t timedelay,bool enflg);







uint8_t sysbtmode = sysbtmode_bt;



const uint8_t chCRCHTalbe[] =                                 // CRC 高位字节值表
{
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40
};

const uint8_t chCRCLTalbe[] =                                 // CRC 低位字节值表
{
0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7,
0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E,
0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09, 0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9,
0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC,
0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32,
0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D,
0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A, 0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38,
0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF,
0x2D, 0xED, 0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1,
0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4,
0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F, 0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB,
0x69, 0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA,
0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0,
0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97,
0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C, 0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E,
0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89,
0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83,
0x41, 0x81, 0x80, 0x40
};





uint8_t box_status = box_status_unknow;
extern bool reconect_error_flg;
extern bool poweronautoanc_flg;
//extern void ancsync_delay_close_time();
extern bool fjhtestmode_flg;

int hds_thread_msg_handler(APP_MESSAGE_BODY * msg)
{
    app_ibrt_ui_t *p_ui_ctrl = app_ibrt_ui_get_ctx();


   TRACE(5,"message_id = %x",msg->message_id);


   TRACE(5,"p_ui_ctrl->box_state-------> = %x",p_ui_ctrl->box_state);

   
  
   switch(msg->message_id)
   {
      case THREAD_MSG_INPUT_EVENT_INBOX:
	  	TRACE(5,"message_id----->THREAD_MSG_INPUT_EVENT_INBOX");
		 if(box_status == box_status_power_off)
		  break;

		
	  	//app_ibrt_ui_event_entry(IBRT_PUT_IN_EVENT);
	  	hds_status_set_cang_inout(true);

                //app_stop_10_second_timer(APP_POWEROFF_TIMER_ID);
		//app_stop_10_second_timer(APP_CONNECTED_TIMER_ID);
		
		   if(box_status == box_status_in)
		   {
                      break;
		   }		

           if(box_status != box_status_in)
           {
		    app_status_indication_set(APP_STATUS_INDICATION_BOX_IN); 
           }

           if(sysbtmode!=sysbtmode_bt)
           {
                box_status = box_status_in;
                break;
	       }
        


		
		//if(p_ui_ctrl->box_state!=IBRT_IN_BOX_CLOSED)
		//通话时把主耳放入盒子进行主从切换....
		//UI_SWITCH_EN();
		//analog_aud_codec_mute();
		if(tws_role_switch_flg == false)
		{
		   tws_role_switch_flg = true;
		   app_ibrt_ui_event_entry(IBRT_PUT_IN_EVENT);
		}
		
		
		//EnterBTpairFlg = false;
        //按键无效....
        //hal_gpio_pin_set(HAL_GPIO_PIN_P1_4);
        

       
#ifdef __FJH_ANC_TEST_ADD__
		//ANC 调试完后放入盒子合盖后关机....
		if((EnterBTpairFlg == true)&(EnterBTsingpairFlg == false))
		{
			app_reset();
		}
#endif

		

		
		#if 0
		if(Featch_out_flg == true)
		{
		  if(sniff_enter_flg == false)
	  	  {
	  	    if(box_cmd_enable == true)
	  	    {
              app_ibrt_ui_event_entry(IBRT_CLOSE_BOX_EVENT);
			  app_status_indication_set(APP_STATUS_INDICATION_CHARGING);
	  	    }
		  }
		  else
		  {
             app_reset();
		  }
		  Featch_out_flg = false;
		}
		EnterBTpairFlg = false;
		#endif


		box_status = box_status_in;
	  	break;

		
		
	  case THREAD_MSG_INPUT_EVENT_OUTBOX:
	  	TRACE(5,"message_id----->THREAD_MSG_INPUT_EVENT_OUTBOX");
	  //app_ibrt_ui_event_entry(IBRT_PUT_IN_EVENT);

	       if(box_status == box_status_power_off)
	  	break;

              if(sysbtmode!=sysbtmode_bt)
              {
                    hds_status_set_cang_inout(false);
		    box_status = box_status_out;			
                    break;
	       }

		   if(box_status == box_status_out)
		   {
                      break;
		   }

	  

	       hds_status_set_cang_inout(false);
	       //analog_aud_codec_nomute();
                if(Auto_Cang_Power_Off== true)
                {


		         // Auto_Cang_Power_Off = false;
		           cangout_poweroff = false;
                           app_shutdown();	
			   return 0;
		   }
		   if(tws_role_switch_flg == true)
		   {
		      tws_role_switch_flg = false;
		      app_ibrt_ui_event_entry(IBRT_FETCH_OUT_EVENT);
		   }
 

		
		//hal_gpio_pin_clr(HAL_GPIO_PIN_P1_4);

        //if(p_ui_ctrl->box_state!=IBRT_OUT_BOX)
        #if 0
        if(Featch_out_flg == false)
        {
	    if(sniff_enter_flg == false)
	  	{
	  	  if(box_cmd_enable == true)
	  	  {
	  	     hds_status_set_cang_onoff(true);
	  	     hds_status_set_chargemode(false);
			 if(EnterBTpairFlg == false)
			 {
               app_auto_twspair();
			 }
	  	  }
	  	}
		else
		{
           app_reset();
		}
        }
		#endif
		box_status = box_status_out;
		break;


		
	  case THREAD_MSG_UART_CMD_OPENCANG:
	    if(box_status == box_status_power_off)
	  	break;
	  	TRACE(5,"message_id----->THREAD_MSG_UART_CMD_OPENCANG");

		   if(box_status == box_status_open)
		   {
                        break;
		   }		

		
		Auto_Cang_Power_Off = false;
		reconect_error_flg = true;
		poweronautoanc_flg = true;

               if(sysbtmode!=sysbtmode_bt)
               {
                  break;
	       }

		

	  	if(sniff_enter_flg == false)
	  	{
	  	  if(box_cmd_enable == true)
	  	  {
	  	   // if(p_ui_ctrl->box_state!=IBRT_OUT_BOX)
	  	    if(Featch_out_flg == false)
	  	  	{
	  	      hds_status_set_cang_onoff(false);
	  	      hds_status_set_chargemode(false);
                      app_auto_twspair();
	  	  	}
	  	  }
	  	}
		else
		{
                 app_reset();
		}
		//sniff_enter_num = 0;
		box_status = box_status_open;
	  	break;
		
		
	  case THREAD_MSG_UART_CMD_CLOSECANG:
	  	fjhtestmode_flg = false;
	       if(box_status == box_status_power_off)
	  	break;


		   if((box_status == box_status_off))
		   {
		         if(Auto_Cang_Power_Off == false)
                         break;
		   }		
		   box_status = box_status_off;
		
	  	  Auto_Cang_Power_Off = false;
		  tws_role_switch_flg = false;
		  poweronautoanc_flg = false;
		  reconect_error_flg = false;
		  twspairfaill_clear();
		//  ancsync_delay_close_time();


        //退出返回ANC的状态
               #ifdef __FJH_AUTO_ANC__
		auto_anc[0] = 5;
		auto_anc[1] = 0;
		auto_anc[2] = 0;
		auto_anc[3] = 0;
		#endif



               if(sysbtmode!=sysbtmode_bt)
              {
                 app_reset();
                 break;
	      }


		
	  	TRACE(5,"message_id----->THREAD_MSG_UART_CMD_CLOSECANG");
        
		
		app_stop_10_second_timer(APP_POWEROFF_TIMER_ID);
		app_stop_10_second_timer(APP_CONNECTED_TIMER_ID);
		//analog_aud_codec_mute();
        //关闭ANC
        //app_anc_close_module();
		close_anc();
#ifdef __ANC_LED__
		//app_anc_set_led(false);
#endif


		EnterBTpairFlg = false;
	  	if(sniff_enter_flg == false)
	  	{
	  	  if(box_cmd_enable == true)
	  	  {
	  	   // if(p_ui_ctrl->box_state!=IBRT_IN_BOX_CLOSED)
	  	   // if(Featch_out_flg == true)
	  	   	{
	  	   // if(app_status_indication_get()!=APP_STATUS_INDICATION_CHARGING)
	  	    app_status_indication_set(APP_STATUS_INDICATION_CHARGING);
	  	    app_ibrt_ui_event_entry(IBRT_CLOSE_BOX_EVENT);
		    hds_status_set_cang_onoff(true);
		    hds_status_set_chargemode(true);

		    nv_record_flash_flush();
		    norflash_flush_all_pending_op();
			Featch_out_flg = false;
	  	   	}

			
		   // sniff_enter_num = 5;
	  	  }
	  	}
		else
		{
            //app_reset();
		}
		EnterBTpairFlg = false;
		
	  break;









		

	  case THREAD_MSG_INPUT_EVENT_I2CINT:
	  	TRACE(5,"message_id----->THREAD_MSG_INPUT_EVENT_I2CINT");
	  	Icp1205UpdataIntSts();
		i2c_event_enableint(HAL_GPIO_PIN_P1_1);
	  	break;


	  case THREAD_MSG_BATTRY_EVENT_FULLCHARGE:
	  	TRACE(5,"message_id----->THREAD_MSG_BATTRY_EVENT_FULLCHARGE");
		//if((hds_status_get_cang_onoff() == true)||(Auto_Cang_Power_Off == true))
		if(box_status !=box_status_power_off)
		{
		  Auto_Cang_Power_Off = false;
		  cangout_poweroff = false;
	  	  app_shutdown();			//充满电,关机
	  	  box_status = box_status_power_off;
		}
	  	break;


/*********************************************************************************************************************/
        //DUT
		case THREAD_MSG_UART_CMD_TEST:
		      TRACE(5,"message_id----->DUT");
		      app_ibrt_ui_event_entry(IBRT_CLOSE_BOX_EVENT);
		      osDelay(100);
		      app_poweron_factorymode_fjh();		
		      break;

       //ANC TEST
		case THREAD_MSG_UART_CMD_ANC_TEST:
			TRACE(5,"message_id----->ANC");
			app_ibrt_ui_event_entry(IBRT_CLOSE_BOX_EVENT);
			osDelay(100);
			app_bt_sing_enter_pair();		
			//app_bt_enter_pair();	 
			break;



      //单线升级或OTA
	    case THREAD_MSG_UART_CMD_UPGRADE:
		   TRACE(5,"message_id----->OTA");
	  	   sing_dowload_init();
	  	   app_singleline_enter_fan();
	  	   break;



/*********************************************************************************************************************/
        case THREAD_MSG_ENTER_SING_UART_MODE:
            app_start_tws_pairing();
			/*
			ICP1205_COMM_COM_EN();
			ICP1205_COMM_COM_WRITE();
		      hds_delay_msg_add(THREAD_MSG_LY_EVENT_ANSWER_BTADDR,300,1);*/
			break;

                case THREAD_MSG_LY_EVENT_ANSWER_BTADDR:
			 hds_thread_msg_answer_btaddr(0,0);
			 hds_delay_msg_add(THREAD_MSG_LY_EVENT_FINISH_ANSWER,300,1);
	   	break;

		
		case THREAD_MSG_LY_EVENT_GET_BTADDR:
			ICP1205_COMM_COM_DIS();
			app_start_tws_pairing();
		break;

		
		case THREAD_MSG_LY_EVENT_FINISH_ANSWER:
			//hds_thread_msg_finish_answer(0,0);
			ICP1205_COMM_COM_READ();
		break;


		case THREAD_MSG_SHIP_MODE:
		if(box_status !=box_status_power_off)
		{
		  Auto_Cang_Power_Off = false;
		  cangout_poweroff = false;
	  	  app_shutdown();			//充满电,关机	
		}
		box_status = box_status_power_off;
		break;


		case THREAD_MSG_TWS_PAIRING_MODE:
			app_start_tws_pairing();
			break;
			
/*****************用户区盒子定义*******************/
        //shuangji
	 case THREAD_MSG_UART_EVENT_MOBILE_DISCONNECTED:
	    	TRACE(5,"message_id----->THREAD_MSG_UART_EVENT_MOBILE_DISCONNECTED");
		    if(app_tws_ibrt_mobile_link_connected())
			{
			     //app_tws_ibrt_disconnect_mobile();	
		         //app_ibrt_if_disconnect_mobile_tws_link();
			     app_ibrt_ui_event_entry(IBRT_DISCONNECT_MOBILE_TWS_EVENT);
			     app_ibrt_ui_event_entry(IBRT_TWS_PAIRING_EVENT);	  
			}
			else if(app_tws_ibrt_tws_link_connected())
			{
			      if(app_tws_is_master_mode())
			      {
                                  app_ibrt_ui_event_entry(IBRT_TWS_PAIRING_EVENT);
			      }
			}
			break;
			


      //5S
	  case THREAD_MSG_UART_CMD_BTPAIR:
	  	TRACE(5,"message_id----->THREAD_MSG_UART_CMD_BTPAIR");
	  	//app_bt_enter_pair();
	  	

		
		if(app_tws_ibrt_mobile_link_connected())
		{
			 app_ibrt_ui_event_entry(IBRT_DISCONNECT_MOBILE_TWS_EVENT);
		}
		else if(app_tws_ibrt_tws_link_connected())
		{
             //app_ibrt_ui_tws_pairing_test();
			 //app_ibrt_ui_event_entry(IBRT_TWS_PAIRING_EVENT);
		}		
		app_bt_enter_pair_new();
	  	break;



	 //恢复出厂....	 10S   
	   case THREAD_MSG_UART_CMD_RESET:
	   	   TRACE(5,"message_id----->RESET");

		   app_status_indication_set(APP_STATUS_INDICATION_CLEARSUCCEED);
		   osDelay(1000);
		   app_bt_enter_pair();

		   
		   /*
		   app_status_indication_set(APP_STATUS_INDICATION_CLEARSUCCEED);
		   app_status_indication_filter_set(APP_STATUS_INDICATION_DISCONNECTED);
		   //app_ibrt_ui_event_entry(IBRT_CLOSE_BOX_EVENT);
		   if(app_tws_ibrt_mobile_link_connected())
		   {
			     app_ibrt_ui_event_entry(IBRT_DISCONNECT_MOBILE_TWS_EVENT);
				 osDelay(100);
		   }
		   app_ibrt_ui_disconnect_tws_fjh();
		   osDelay(100);
		   app_clear_pair();*/
	       break;	


       //工厂单个模式测试...  danji
		case THREAD_MSG_UART_CMD_IPHONE_PAIRING:
			app_ibrt_ui_reconnect_event_test_fjh();
			break;
/********************************************************/
		
	  default:break;


   }

   

   return 0;

}



uint16_t GetCRC16(uint8_t* pchMsg, uint16_t wDataLen)
{
        uint8_t chCRCHi = 0xFF; // 高CRC字节初始化
        uint8_t chCRCLo = 0xFF; // 低CRC字节初始化
        uint16_t wIndex;            // CRC循环中的索引

        while (wDataLen--)
        {
                // 计算CRC
                wIndex = chCRCLo ^ *pchMsg++ ;
                chCRCLo = chCRCHi ^ chCRCHTalbe[wIndex];
                chCRCHi = chCRCLTalbe[wIndex] ;
        }

        return ((chCRCHi << 8) | chCRCLo) ;
}

//回复蓝牙地址
static void hds_thread_msg_answer_btaddr(uint32_t param1,uint32_t param2)
{



    uint8_t *bt_addr = factory_section_get_bt_address();
	
	TRACE(5,"answer_btaddr %s %d",__func__,param1);
	param1 = param1;
	param2 = param2;

//	if (app_is_power_off_in_progress()) return;

     
	for (uint8_t j=0;j<3;j++)
	{
		uint8_t a[17]={0x00,0xaa,0xaa,0xaa,0xaa,0x0C,0x01,0x11,0x22,0x33,0x44,0x55,0x66,0x00,0x00,0x00,0x00};
		
		for (uint8_t i=0; i<6; i++)
			a[i+7] = bt_addr[i];
		
		uint16_t current_voltage = app_battery_current_volt();
		a[14] = current_voltage&0xff;
		a[13] = (current_voltage>>8)&0xff;

		uint16_t crc = GetCRC16(&a[5],10); 
		a[15]=crc/256;
		a[16]=crc%256;
		communication_send_buf(a,17);
	}
	hds_delay_msg_add(THREAD_MSG_LY_EVENT_FINISH_ANSWER,200,1);
}


//回复蓝牙地址完成，切换串口为接收模式
/*
static void hds_thread_msg_finish_answer(uint32_t param1,uint32_t param2)
{
	TRACE(5,"finish_answer %s %d",__func__,param1);
	param1 = param1;
	param2 = param2;
	
	//hal_gpio_pin_clr(HAL_GPIO_PIN_P1_0);
}
*/







void Entet_Singleline(void)
{
  //  hal_iomux_set_uart1();
	sing_dowload_init();
	app_singleline_enter_fan();
}



uint8_t hds_thead_msg_findid(uint32_t msgid)
{

   return 0;
}

uint32_t hds_get_delaytime(uint32_t start)
{

   return 0;
}

void hds_thread_msg_putwith_param(uint32_t msgid,uint32_t param1,uint32_t param2)
{

}


static void hds_delay_timehandler(void const * param)
{
    hds_thread_msg_send(msgid_fjh);
}


void hds_delay_set_time(uint32_t millisec)
{
  if (hds_delay_timer == NULL)
  {
	 hds_delay_timer	= osTimerCreate(osTimer(HDS_DELAY), osTimerOnce, NULL);
	 osTimerStop(hds_delay_timer);
  } 
     osTimerStart(hds_delay_timer, millisec);
}




void hds_delay_msg_add(uint8_t eventmode,uint16_t timedelay,bool enflg)
{
    msgid_fjh = eventmode;
    hds_delay_set_time(timedelay);
}





void hds_thread_msg_send(uint32_t msgid)
{
    APP_MESSAGE_BLOCK msg;
    msg.mod_id          = APP_MODUAL_BOX;
    msg.msg_body.message_id = msgid;
    msg.msg_body.message_ptr = (uint32_t) NULL;
    app_mailbox_put(&msg);
}


void hds_thread_init(void)
{
  app_set_threadhandle(APP_MODUAL_BOX, hds_thread_msg_handler);
}


void hds_tread_exit()
{


}




uint8_t bt_readaddr[6]={0x00};
bool btreadadd_flg = false;



void communication_receive__callback(uint8_t *buf, uint8_t len)
{ 
    //命令解析...
	DUMP8("%x",buf,len);
	if(len > 3)
	{
    for (uint8_t j = 0; j < (len - 3); j++)   //10 bit
    {
       if(buf[j]==0xaa)
       {
            if((buf[j+1]==0xaa)&(buf[j+2]==0xaa)&(buf[j+3]==0xaa)) 
            {
                //sucefull
                if((j+6)<len)
                {
				    //memcpy(bt_readaddr,buf[j+6],6);
				    for (uint8_t i = 0; i < 6; i++)   //10 bit
				    {
                          bt_readaddr[i] =  buf[i+j+6];

					}
					DUMP8("%x",bt_readaddr,6);
					btreadadd_flg = true;
					hds_delay_msg_add(THREAD_MSG_LY_EVENT_GET_BTADDR,100,1);
					return;
                }
				
			}	
	   } 
	}
	}
}


uint8_t box_status_test(void)
{
    return box_status;
}




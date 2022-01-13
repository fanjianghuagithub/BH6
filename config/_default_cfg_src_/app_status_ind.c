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
#include "stdbool.h"
#include "hal_trace.h"
#include "app_pwl.h"
#include "app_status_ind.h"
#include "string.h"
#include "pmu.h"
#include "hal_timer.h"
#include "app_thread.h"

enum
{
    LED_BATTERY_LOW = 0,
    DUT_TEST_MODE,
    BOX_IN_MODE,
    DUT_CLEAR_MODE,
    BOX_POWER_ON,
    BOX_TWS_CONNECTED,
    BOX_CLEAR_PHONE_LIST,
    BOX_TWS_PAIRFAILL,
    BOX_NUM = 0xff,
};


uint8_t fjh_msgled_id = BOX_NUM;
bool breath_led_2_flg = false;
bool breath_led_1_flg = false;


static APP_STATUS_INDICATION_T app_status = APP_STATUS_INDICATION_NUM;
static APP_STATUS_INDICATION_T app_status_ind_filter = APP_STATUS_INDICATION_NUM;
static APP_STATUS_INDICATION_T fjh_msgled_status = APP_STATUS_INDICATION_NUM;


const char *app_status_indication_str[] =
{
    "[POWERON]",
    "[INITIAL]",
    "[PAGESCAN]",
    "[POWEROFF]",
    "[CHARGENEED]",
    "[CHARGENEED2]",
    "[CHARGING]",
    "[FULLCHARGE]",
    /* repeatable status: */
    "[BOTHSCAN]",
    "[CONNECTING]",
    "[CONNECTED]",
    "[DISCONNECTED]",
    "[CALLNUMBER]",
    "[INCOMINGCALL]",
    "[PAIRSUCCEED]",
    "[PAIRFAIL]",
    "[HANGUPCALL]",
    "[REFUSECALL]",
    "[ANSWERCALL]",
    "[CLEARSUCCEED]",
    "[CLEARFAIL]",
    "[WARNING]",
    "[ALEXA_START]",
    "[ALEXA_STOP]",
    "[GSOUND_MIC_OPEN]",
    "[GSOUND_MIC_CLOSE]",
    "[GSOUND_NC]",
    "[INVALID]",
    "[MUTE]",
    "[TESTMODE]",
    "[TESTMODE_test]",
    "[TESTMODE1]",
    "[RING_WARNING]",
#ifdef __INTERACTION__	
    "[FINDME]",
#endif	
    "[ANC_ON]",
    "[AMB_ON]",
    "[AMB_OFF]",
    "[NUM_1]",//chenzhao
    "[NUM_2]",
    "[NUM_3]",
    
    "[MY_BUDS_FIND]",
    "[TILE_FIND]",

    "FLASH_TYPE_1",
    "FLASH_TYPE_2",
    "FLASH_TYPE_3",
    "FLASH_TYPE_4",//chenzhao

    "BOX_IN",
    "CLEARSUCCEED2",


};



static void led_delay_timehandler(void const * param);
osTimerDef(LED_GENSORDELAY, led_delay_timehandler);
static osTimerId       led_delay_timer = NULL;

void led_delay_Send_Cmd(uint32_t modeid);


static void led_delay_timehandler(void const * param)
{

     if(fjh_msgled_id == BOX_POWER_ON)
     {
            fjh_msgled_id = BOX_NUM;

            TRACE(2,"%s  fjh_msgled_status = %d",__func__, fjh_msgled_status);
			
	    if(fjh_msgled_status == APP_STATUS_INDICATION_POWERON)
	   	fjh_msgled_status = APP_STATUS_INDICATION_INITIAL;			
	    app_status_indication_set(fjh_msgled_status);
	    return;
     }

   if(fjh_msgled_id == BOX_IN_MODE)
   {
	   fjh_msgled_id = BOX_NUM;
	   TRACE(2,"%s  fjh_msgled_status = %d",__func__, fjh_msgled_status);
	   if(fjh_msgled_status == APP_STATUS_INDICATION_BOX_IN)
	   {
	           if((app_status == APP_STATUS_INDICATION_BOTHSCAN)||(app_status ==APP_STATUS_INDICATION_PAGESCAN))
	           {  
                      fjh_msgled_status = app_status;
		   }
		   else
		   {
	   	      fjh_msgled_status = APP_STATUS_INDICATION_INITIAL;
		   }   
	   }
	   app_status = APP_STATUS_INDICATION_NUM;
	   app_status_indication_set(fjh_msgled_status);
	   return;
   }


   if(fjh_msgled_id == BOX_CLEAR_PHONE_LIST)
   {
	   fjh_msgled_id = BOX_NUM;
	   TRACE(2,"%s  fjh_msgled_status = %d",__func__, fjh_msgled_status);  
	   if(fjh_msgled_status == APP_STATUS_INDICATION_CLEARSUCCEED)
	   {
	       if(app_status == APP_STATUS_INDICATION_BOTHSCAN)
	       {  
               //fjh_msgled_status = APP_STATUS_INDICATION_BOTHSCAN;
			   app_status = APP_STATUS_INDICATION_NUM;
			   app_status_indication_set(APP_STATUS_INDICATION_BOTHSCAN);
	       }
	   }
          else
          {
	        app_status_indication_set(fjh_msgled_status);
          }
	      return;
   }
   else if(fjh_msgled_id == LED_BATTERY_LOW)
   {
       fjh_msgled_id = BOX_NUM;
       if(fjh_msgled_status ==APP_STATUS_INDICATION_CHARGENEED)
       {
           if(app_status == APP_STATUS_INDICATION_BOTHSCAN)
       	   {
       	           app_status = APP_STATUS_INDICATION_NUM;
                   app_status_indication_set(APP_STATUS_INDICATION_BOTHSCAN);
       	   }
	   else  if(app_status == APP_STATUS_INDICATION_PAGESCAN)
	   {
	          app_status = APP_STATUS_INDICATION_NUM;
	          app_status_indication_set(APP_STATUS_INDICATION_BOTHSCAN);
	   }
	   else
	   {
                    app_status = APP_STATUS_INDICATION_NUM;
                    app_status_indication_set(APP_STATUS_INDICATION_INITIAL);
	   }
       }
       else
       {
                app_status = APP_STATUS_INDICATION_NUM;
                app_status_indication_set(fjh_msgled_status);
	}
   }
   else
   {
        app_status = APP_STATUS_INDICATION_NUM;
        app_status_indication_set(APP_STATUS_INDICATION_INITIAL);
   }



#if 0

   APP_STATUS_INDICATION_T old_app_status = app_status;
   if(old_app_status == APP_STATUS_INDICATION_TESTMODE)
   {
      app_status_indication_set(APP_STATUS_INDICATION_TESTMODE_test);
   }
   else if(old_app_status == APP_STATUS_INDICATION_TESTMODE_test)
   {
      app_status_indication_set(APP_STATUS_INDICATION_INITIAL);
   }
   #if 0
   else if(old_app_status == APP_STATUS_INDICATION_CLEARSUCCEED)
   {
        app_status_indication_set(APP_STATUS_INDICATION_CLEARSUCCEED2);
   }
   else if(old_app_status == APP_STATUS_INDICATION_CLEARSUCCEED2)
   {
        app_status_indication_set(APP_STATUS_INDICATION_INITIAL);
   }
   #endif
    else if(old_app_status == APP_STATUS_INDICATION_CONNECTING)
   {
        app_status_indication_set(APP_STATUS_INDICATION_INITIAL);
   }  
   else
   {
      app_status = APP_STATUS_INDICATION_NUM;
      app_status_indication_set(old_app_status);
   }
   #endif
}

void led_delay_set_time(uint32_t millisec)
{
  if (led_delay_timer == NULL)
  {
	 led_delay_timer	= osTimerCreate(osTimer(LED_GENSORDELAY), osTimerOnce, NULL);
	 osTimerStop(led_delay_timer);
  } 
     osTimerStart(led_delay_timer, millisec);
}


void led_delay_close_time(void)
{
if (led_delay_timer == NULL)
{
  // led_delay_timer	  = osTimerCreate(osTimer(LED_GENSORDELAY), osTimerOnce, NULL);
  // osTimerStop(led_delay_timer);
} 
else
{
	osTimerStop(led_delay_timer);
}
}



void led_delay_Send_Cmd(uint32_t modeid)
{
  APP_MESSAGE_BLOCK msg;
  msg.mod_id			= APP_MODUAL_LED_STATIC_DELAY;
  msg.msg_body.message_id = modeid;
  msg.msg_body.message_ptr = (uint32_t) NULL;
  app_mailbox_put(&msg);

  TRACE(2,"%s   modeid %d",__func__, modeid);
  
}



int app_led_process(APP_MESSAGE_BODY * msg)
{

   if(msg->message_id == LED_BATTERY_LOW)
   {
       led_delay_set_time(12000);
   }
   else if(msg->message_id == DUT_TEST_MODE)
   {
       led_delay_set_time(5000);
   }
   else if(msg->message_id == BOX_IN_MODE)
   {
       led_delay_set_time(1200);
   } 
   if(msg->message_id == DUT_CLEAR_MODE)
   {
       led_delay_set_time(10000);
   }  

   if(msg->message_id == BOX_POWER_ON)
   {
       led_delay_set_time(3000);
   }  
   
   if(msg->message_id == BOX_TWS_CONNECTED)
   {
       led_delay_set_time(10000);
   }
   if(msg->message_id == BOX_CLEAR_PHONE_LIST)
   {
       led_delay_set_time(1500);
   }

   if(msg->message_id == BOX_TWS_PAIRFAILL)
   {
          led_delay_set_time(13000);
   }

   

   
   return 0;
}

void init_leddelay_thread(void)
{
	app_set_threadhandle(APP_MODUAL_LED_STATIC_DELAY, app_led_process);
}

const char *status2str(uint16_t status)
{
    const char *str = NULL;

    if (status >= 0 && status < APP_STATUS_INDICATION_NUM)
    {
        str = app_status_indication_str[status];
    }
    else
    {
        str = "[UNKNOWN]";
    }

    return str;
}

int app_status_indication_filter_set(APP_STATUS_INDICATION_T status)
{
    app_status_ind_filter = status;
    return 0;
}

APP_STATUS_INDICATION_T app_status_indication_get(void)
{
    if(fjh_msgled_status == APP_STATUS_INDICATION_BOTHSCAN)
    {
         return APP_STATUS_INDICATION_BOTHSCAN;
    }
    return app_status;
}












extern void pmu_led_breathing_enable(enum HAL_IOMUX_PIN_T pin, const struct PMU_LED_BR_CFG_T *cfg);
extern void pmu_led_breathing_disable(enum HAL_IOMUX_PIN_T pin);
extern uint16_t pmu_led_breathing_enable_test(enum HAL_IOMUX_PIN_T pin, const struct PMU_LED_BR_CFG_T *cfg);
extern uint16_t pmu_led_breathing_disable_test(enum HAL_IOMUX_PIN_T pin);

int app_status_indication_set(APP_STATUS_INDICATION_T status)
{
    struct APP_PWL_CFG_T cfg0;
    struct APP_PWL_CFG_T cfg1;
	struct PMU_LED_BR_CFG_T breath_led;


    TRACE(2,"%s %d",__func__, status);

    if (app_status == status)
        return 0;

    if (app_status_ind_filter == status)
        return 0;


   if(app_status ==APP_STATUS_INDICATION_POWEROFF)	
   {
         return 0;
   }


        if((status ==APP_STATUS_INDICATION_CHARGING))
        {
             fjh_msgled_status = APP_STATUS_INDICATION_NUM;
	         fjh_msgled_id = BOX_NUM;
             led_delay_close_time();
        }

        if((status ==APP_STATUS_INDICATION_BOX_IN))
        {
             if(fjh_msgled_status != APP_STATUS_INDICATION_BOX_IN)
             {
                fjh_msgled_status = APP_STATUS_INDICATION_NUM;
	        fjh_msgled_id = BOX_NUM;
                led_delay_close_time();
             }
        }


        fjh_msgled_status = status;	
        if(fjh_msgled_id != BOX_NUM)
     	{
		    return 0;
	}




        if(app_status == APP_STATUS_INDICATION_CHARGING)
        {
                   if((status ==APP_STATUS_INDICATION_POWERON)||(status ==APP_STATUS_INDICATION_FULLCHARGE))
                   {
                   
                   }
		   else
		   {
                         return 0;
		   }
      }


#if 0

   //o??¨¹¦Ì?¨¢¨¢?e....
   if((status ==APP_STATUS_INDICATION_CHARGING))
   {



   
       if(breath_led_2_flg == false)
       {
		//   app_pwl_stop_fjh(APP_PWL_ID_0);
		//   app_pwl_stop_fjh(APP_PWL_ID_1);
           app_pwl_close();

	   
	   if(breath_led_1_flg == true)
	   {
		breath_led_1_flg = false;
		pmu_led_breathing_disable_test(HAL_IOMUX_PIN_LED1);

	   }
	   
	//   app_pwl_stop(APP_PWL_ID_0);
	//   app_pwl_stop(APP_PWL_ID_1);

	   app_status = status;
	   breath_led.fade_time_ms = 1000;
	   breath_led.off_time_ms = 500;
	   breath_led.on_time_ms = 500;
           pmu_led_breathing_enable_test(HAL_IOMUX_PIN_LED2,&breath_led);   

	   breath_led_2_flg = true;
       }
	   return 0;
   }
   else 
#endif

   
   if(status ==APP_STATUS_INDICATION_BOX_IN)
   {
       if(breath_led_1_flg == false)
       {
	//   app_pwl_stop_fjh(APP_PWL_ID_0);
	//   app_pwl_stop_fjh(APP_PWL_ID_1);
	  app_pwl_close();

	  if(breath_led_2_flg == true)
	  {
		breath_led_2_flg = false;
		 pmu_led_breathing_disable_test(HAL_IOMUX_PIN_LED2);
	   }
	   
    //   app_pwl_stop(APP_PWL_ID_0);
    //   app_pwl_stop(APP_PWL_ID_1);
   
       //app_status = status;
       breath_led.fade_time_ms = 1000;
       breath_led.off_time_ms = 500;
       breath_led.on_time_ms = 500;
       pmu_led_breathing_enable_test(HAL_IOMUX_PIN_LED1,&breath_led);   

   
       fjh_msgled_id = BOX_IN_MODE;
       breath_led_1_flg = true;
       led_delay_Send_Cmd(BOX_IN_MODE);
       }
       return 0;
   }



    if((status ==APP_STATUS_INDICATION_POWERON)||(status ==APP_STATUS_INDICATION_INITIAL)||(status ==APP_STATUS_INDICATION_CONNECTED)||(status ==APP_STATUS_INDICATION_CHARGENEED)||(status ==APP_STATUS_INDICATION_POWEROFF)||(status ==APP_STATUS_INDICATION_FULLCHARGE)||(status ==APP_STATUS_INDICATION_PAGESCAN)||(status ==APP_STATUS_INDICATION_DISCONNECTED)||(status ==APP_STATUS_INDICATION_BOTHSCAN)||(status ==APP_STATUS_INDICATION_TESTMODE)||(status ==APP_STATUS_INDICATION_TESTMODE1)||(status ==APP_STATUS_INDICATION_TESTMODE_test)||(status ==APP_STATUS_INDICATION_BOX_IN)||(status ==APP_STATUS_INDICATION_CLEARSUCCEED)||(status ==APP_STATUS_INDICATION_CLEARSUCCEED2)||(status ==APP_STATUS_INDICATION_CONNECTING)||(status ==APP_STATUS_INDICATION_CHARGING)||(status ==APP_STATUS_INDICATION_PAIRFAIL))
    {
          //¨¨???o??¨¹¦Ì?....
          /*
          if(app_status ==APP_STATUS_INDICATION_CHARGING)
          {
                pmu_led_breathing_disable(HAL_IOMUX_PIN_LED2);
	   }
          if(app_status ==APP_STATUS_INDICATION_BOX_IN)
          {
                pmu_led_breathing_disable(HAL_IOMUX_PIN_LED1);
	   }	*/

		  
           if(breath_led_2_flg == true)
           {
                           //  app_pwl_close();
                             breath_led_2_flg = false;
		             pmu_led_breathing_disable_test(HAL_IOMUX_PIN_LED2);
	    }
	    if(breath_led_1_flg == true)
	    {
	                  //   app_pwl_close();
			     breath_led_1_flg = false;
			     pmu_led_breathing_disable_test(HAL_IOMUX_PIN_LED1);
           }


		  
     }
     else
     {
           return 0;
     }


	
    if((status == APP_STATUS_INDICATION_CLEARSUCCEED))    //(status == APP_STATUS_INDICATION_CHARGENEED)
    {
        
	}
	else
	{
	    app_status = status;
	}
	
	

    memset(&cfg0, 0, sizeof(struct APP_PWL_CFG_T));
    memset(&cfg1, 0, sizeof(struct APP_PWL_CFG_T));

    app_pwl_stop(APP_PWL_ID_0);
    app_pwl_stop(APP_PWL_ID_1);
    switch (status) {


      case APP_STATUS_INDICATION_PAIRFAIL:
		  cfg1.part[0].level = 1;
		  cfg1.part[0].time = (600);
		  cfg1.part[1].level = 0;
		  cfg1.part[1].time = (600);
		  cfg1.parttotal = 2;
		  cfg1.startlevel = 1;
		  cfg1.periodic = true;
		  app_pwl_setup(APP_PWL_ID_1, &cfg1);
		  app_pwl_start(APP_PWL_ID_1);	
		  led_delay_Send_Cmd(BOX_TWS_PAIRFAILL);	
	  	break;




       case APP_STATUS_INDICATION_BOX_IN:
            cfg0.part[0].level = 1;
            cfg0.part[0].time = (1000);
            cfg0.part[1].level = 0;
            cfg0.part[1].time = (500);
	
            cfg0.parttotal = 2;
            cfg0.startlevel = 1;
            cfg0.periodic = false;
	    app_pwl_setup(APP_PWL_ID_0, &cfg0);
	    app_pwl_start(APP_PWL_ID_0);
	    led_delay_Send_Cmd(BOX_IN_MODE);
	   break;



		
        case APP_STATUS_INDICATION_POWERON:
            cfg1.part[0].level = 1;
            cfg1.part[0].time = (3000);
            cfg1.part[1].level = 0;
            cfg1.part[1].time = (100);
            cfg1.parttotal = 2;
            cfg1.startlevel = 1;
            cfg1.periodic = false;
            app_pwl_setup(APP_PWL_ID_1, &cfg1);
            app_pwl_start(APP_PWL_ID_1);
	    fjh_msgled_id = BOX_POWER_ON;
	    led_delay_Send_Cmd(BOX_POWER_ON);	
            break;



        case APP_STATUS_INDICATION_POWEROFF:
            cfg0.part[0].level = 1;
            cfg0.part[0].time = (3000);
            cfg0.part[1].level = 0;
            cfg0.part[1].time = (100);
            cfg0.parttotal = 2;
            cfg0.startlevel = 1;
            cfg0.periodic = false;
            app_pwl_setup(APP_PWL_ID_0, &cfg0);
            app_pwl_start(APP_PWL_ID_0);	
            break;


       case APP_STATUS_INDICATION_BOTHSCAN:
		   cfg0.part[0].level = 1;
		   cfg0.part[0].time = (300);
		   cfg0.part[1].level = 0;
		   cfg0.part[1].time = (300);
		   cfg0.parttotal = 2;
		   cfg0.startlevel = 1;
		   cfg0.periodic = true;   
		   cfg1.part[0].level = 0;
		   cfg1.part[0].time = (300);
		   cfg1.part[1].level = 1;
		   cfg1.part[1].time = (300);
		   cfg1.parttotal = 2;
		   cfg1.startlevel = 0;
		   cfg1.periodic = true;		   
		   app_pwl_setup(APP_PWL_ID_0, &cfg0);
		   app_pwl_start(APP_PWL_ID_0);
		   app_pwl_setup(APP_PWL_ID_1, &cfg1);
		   app_pwl_start(APP_PWL_ID_1); 	   
	    break;



         //TWS ?????D
	  case APP_STATUS_INDICATION_PAGESCAN:	
            cfg1.part[0].level = 1;
            cfg1.part[0].time = (300);
            cfg1.part[1].level = 0;
            cfg1.part[1].time = (300);
            cfg1.parttotal = 2;
            cfg1.startlevel = 1;
            cfg1.periodic = true;
            app_pwl_setup(APP_PWL_ID_1, &cfg1);
            app_pwl_start(APP_PWL_ID_1);	  	
	    break;


			
        case APP_STATUS_INDICATION_CONNECTED:
            cfg1.part[0].level = 1;
            cfg1.part[0].time = (1000);
            cfg1.part[1].level = 0;
            cfg1.part[1].time = (1000);
            cfg1.parttotal = 2;
            cfg1.startlevel = 0;
            cfg1.periodic = false;			
            app_pwl_setup(APP_PWL_ID_1, &cfg1);
            app_pwl_start(APP_PWL_ID_1);			
            break;			




	    case APP_STATUS_INDICATION_DISCONNECTED:
            case APP_STATUS_INDICATION_INITIAL:
            break;



	    		
       
			
        case APP_STATUS_INDICATION_CONNECTING:
		cfg1.part[0].level = 1;
		cfg1.part[0].time = (300);
		cfg1.part[1].level = 0;
		cfg1.part[1].time = (300);
		cfg1.parttotal = 2;
		cfg1.startlevel = 1;
		cfg1.periodic = true;
		app_pwl_setup(APP_PWL_ID_1, &cfg1);
		app_pwl_start(APP_PWL_ID_1);
		//fjh_msgled_id = BOX_POWER_ON;
		led_delay_Send_Cmd(BOX_TWS_CONNECTED);	
            break;
			

			
        case APP_STATUS_INDICATION_CHARGING:
            cfg0.part[0].level = 1;
            cfg0.part[0].time = (13000);
            cfg0.part[1].level = 0;
            cfg0.part[1].time = (100);			
            cfg0.parttotal = 2;
            cfg0.startlevel = 1;
            cfg0.periodic = false;
            app_pwl_setup(APP_PWL_ID_0, &cfg0);
            app_pwl_start(APP_PWL_ID_0);
            break;


			
        case APP_STATUS_INDICATION_FULLCHARGE:
	   cfg0.part[0].level = 0;
	   cfg0.part[0].time = (5000);
	   cfg0.parttotal = 1;
	   cfg0.startlevel = 0;
	   cfg0.periodic = true;
	   app_pwl_setup(APP_PWL_ID_0, &cfg0);
	   app_pwl_start(APP_PWL_ID_0);
            break;
			

			
        case APP_STATUS_INDICATION_CHARGENEED:
            cfg0.part[0].level = 1;
            cfg0.part[0].time = (300);
            cfg0.part[1].level = 0;
            cfg0.part[1].time = (300);
            cfg0.part[2].level = 1;
            cfg0.part[2].time = (300);
            cfg0.part[3].level = 0;
            cfg0.part[3].time = (15000);	
            cfg0.parttotal = 4;
            cfg0.startlevel = 1;
            cfg0.periodic = true;
	        app_pwl_setup(APP_PWL_ID_0, &cfg0);
	        app_pwl_start(APP_PWL_ID_0);
	        //    fjh_msgled_id = LED_BATTERY_LOW;
            //   led_delay_Send_Cmd(LED_BATTERY_LOW);
            break;

			
        case APP_STATUS_INDICATION_TESTMODE:
            cfg0.part[0].level = 1;
            cfg0.part[0].time = (5000);
            cfg0.part[1].level = 1;
            cfg0.part[1].time = (300);			
            cfg0.parttotal = 2;
            cfg0.startlevel = 1;
            cfg0.periodic = true;

            cfg1.part[0].level = 1;
            cfg1.part[0].time = (5000);
            cfg1.part[1].level = 1;
            cfg1.part[1].time = (300);
		
            cfg1.parttotal = 2;
            cfg1.startlevel = 1;
            cfg1.periodic = true;

            app_pwl_setup(APP_PWL_ID_0, &cfg0);
            app_pwl_start(APP_PWL_ID_0);
            app_pwl_setup(APP_PWL_ID_1, &cfg1);
            app_pwl_start(APP_PWL_ID_1);
			
			//led_delay_Send_Cmd(DUT_TEST_MODE);
            break;


       case APP_STATUS_INDICATION_TESTMODE_test:
            cfg0.part[0].level = 1;
            cfg0.part[0].time = (300);
            cfg0.part[1].level = 0;
            cfg0.part[1].time = (300);
            cfg0.parttotal = 2;
            cfg0.startlevel = 1;
            cfg0.periodic = true;	
            cfg1.part[0].level = 0;
            cfg1.part[0].time = (300);
            cfg1.part[1].level = 1;
            cfg1.part[1].time = (300);
            cfg1.parttotal = 2;
            cfg1.startlevel = 0;
            cfg1.periodic = true;			
            app_pwl_setup(APP_PWL_ID_0, &cfg0);
            app_pwl_start(APP_PWL_ID_0);
            app_pwl_setup(APP_PWL_ID_1, &cfg1);
            app_pwl_start(APP_PWL_ID_1);	
		//    led_delay_Send_Cmd(DUT_TEST_MODE);
	   	break;
			

			
        case APP_STATUS_INDICATION_TESTMODE1:
            cfg0.part[0].level = 1;
            cfg0.part[0].time = (300);
            cfg0.part[1].level = 0;
            cfg0.part[1].time = (300);
            cfg0.parttotal = 2;
            cfg0.startlevel = 1;
            cfg0.periodic = true;	
            cfg1.part[0].level = 1;
            cfg1.part[0].time = (300);
            cfg1.part[1].level = 0;
            cfg1.part[1].time = (300);
            cfg1.parttotal = 2;
            cfg1.startlevel = 1;
            cfg1.periodic = true;			
            app_pwl_setup(APP_PWL_ID_0, &cfg0);
            app_pwl_start(APP_PWL_ID_0);
            app_pwl_setup(APP_PWL_ID_1, &cfg1);
            app_pwl_start(APP_PWL_ID_1);	
            break;



         case APP_STATUS_INDICATION_CLEARSUCCEED:
			 cfg0.part[0].level = 1;
			 cfg0.part[0].time = (1000);
			 cfg0.part[1].level = 0;
			 cfg0.part[1].time = (300);
			 cfg0.parttotal = 2;
			 cfg0.startlevel = 1;
			 cfg0.periodic = false;

			 cfg1.part[0].level = 1;
			 cfg1.part[0].time = (1000);
			 cfg1.part[1].level = 0;
			 cfg1.part[1].time = (300);
			 cfg1.parttotal = 2;
			 cfg1.startlevel = 1;
			 cfg1.periodic = false;	 

			 
				 		 
			 app_pwl_setup(APP_PWL_ID_0, &cfg0);
			 app_pwl_start(APP_PWL_ID_0);
                         app_pwl_setup(APP_PWL_ID_1, &cfg1);
                         app_pwl_start(APP_PWL_ID_1);		
						 
			 fjh_msgled_status = APP_STATUS_INDICATION_CLEARSUCCEED;
			 fjh_msgled_id = BOX_CLEAR_PHONE_LIST;
	                 led_delay_Send_Cmd(BOX_CLEAR_PHONE_LIST);
		 	break;

			


	case APP_STATUS_INDICATION_CLEARSUCCEED2:
            cfg0.part[0].level = 1;
            cfg0.part[0].time = (300);
            cfg0.part[1].level = 0;
            cfg0.part[1].time = (300);
            cfg0.parttotal = 2;
            cfg0.startlevel = 1;
            cfg0.periodic = true;	
            cfg1.part[0].level = 0;
            cfg1.part[0].time = (300);
            cfg1.part[1].level = 1;
            cfg1.part[1].time = (300);
            cfg1.parttotal = 2;
            cfg1.startlevel = 1;
            cfg1.periodic = true;			
            app_pwl_setup(APP_PWL_ID_0, &cfg0);
            app_pwl_start(APP_PWL_ID_0);
            app_pwl_setup(APP_PWL_ID_1, &cfg1);
            app_pwl_start(APP_PWL_ID_1);	
            led_delay_Send_Cmd(DUT_CLEAR_MODE);
	    break;


			
        default:
            break;
    }
    return 0;
}

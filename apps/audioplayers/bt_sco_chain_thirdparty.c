#include "bt_sco_chain.h"
#include "speech_memory.h"
#include "speech_utils.h"
#include "hal_trace.h"
#include "audio_dump.h"
#include "Soundplus_adapter.h"


#ifdef __BES_RX__
#include "speech_cfg.h"
#include "bt_sco_chain.h"
#include "bt_sco_chain_cfg.h"
#include "bt_sco_chain_tuning.h"
#endif



#ifdef SNDP_BTDM_SUPPORT
#include "bt_sco_chain_cp.h"
#endif

//debug ¿âÊÚÈ¨
#if 1
#include "factory_section.h"
extern uint32_t __factory_start[];
//static uint8_t key[16] = {0};
#endif


#define SP_FADEIN_COUNT (2)
#define SP_MUTE_COUNT   (10)

#if defined(SCO_CP_ACCEL)
#define FADEIN_START_FRM (SP_MUTE_COUNT + 1)
#else
#define FADEIN_START_FRM (SP_MUTE_COUNT)
#endif

#define FADEIN_STOP_FRM (FADEIN_START_FRM + SP_FADEIN_COUNT)
static int sp_fadein_count;




short *aec_echo_buf = NULL;

// Use to free buffer
static short *aec_echo_buf_ptr;

#ifdef SNDP_BTDM_SUPPORT
#else
#if defined(MSBC_8K_SAMPLE_RATE)
#define SPEECH_HEAP_RESERVE_SIZE (1024 * 3)
#else
#define SPEECH_HEAP_RESERVE_SIZE (1024 * 3)
#endif
#endif

#ifdef __FJH_MIC_TEST__
#define EN_SN 0
#define main_mic 1
#define FF_mic 3
#define DIS_SN 5
uint8_t mic_test_mode = EN_SN;
#endif

#ifdef __BES_RX__
//static int speech_tx_sample_rate = 16000;
static int speech_rx_sample_rate = 16000;
//static int speech_tx_frame_len = 256;
static int speech_rx_frame_len = 256;
#if defined(SPEECH_RX_NS2FLOAT)
SpeechNs2FloatState *speech_rx_ns2float_st = NULL;
#endif
#define SPEECH_RX_NS2FLOAT_CORE 0
static SpeechConfig *speech_cfg = NULL;





int speech_rx_init(int sample_rate, int frame_len)
{
    TRACE(3,"[%s] Start, sample_rate: %d, frame_len: %d", __func__, sample_rate, frame_len);

#if defined (SPEECH_RX_NS)
    speech_rx_ns_st = speech_ns_create(sample_rate, frame_len, &speech_cfg->rx_ns);
#endif

#if defined(SPEECH_RX_NS2)
    speech_rx_ns2_st = speech_ns2_create(sample_rate, frame_len, &speech_cfg->rx_ns2);
#endif

#if defined(SPEECH_RX_NS2FLOAT)
    speech_rx_ns2float_st = speech_ns2float_create(sample_rate, frame_len, SPEECH_RX_NS2FLOAT_CORE, &speech_cfg->rx_ns2float);
#endif

#if defined(SPEECH_RX_NS3)
    speech_rx_ns3_st = ns3_create(sample_rate, frame_len, &speech_cfg->rx_ns3);
#endif

#if defined(SPEECH_RX_AGC)
    speech_rx_agc_st = agc_state_create(sample_rate, frame_len, &speech_cfg->rx_agc);
#endif

#if defined(SPEECH_RX_EQ)
    speech_rx_eq_st = eq_init(sample_rate, frame_len, &speech_cfg->rx_eq);
#endif

#if defined(SPEECH_RX_POST_GAIN)
    speech_rx_post_gain_st = speech_gain_create(sample_rate, frame_len, &speech_cfg->rx_post_gain);
#endif

    TRACE(1,"[%s] End", __func__);

    return 0;
}


int speech_rx_deinit(void)
{
    TRACE(1,"[%s] Start...", __func__);

#if defined(SPEECH_RX_POST_GAIN)
    speech_gain_destroy(speech_rx_post_gain_st);
#endif

#if defined(SPEECH_RX_EQ)
    eq_destroy(speech_rx_eq_st);
#endif

#if defined(SPEECH_RX_AGC)
    agc_state_destroy(speech_rx_agc_st);
#endif

#if defined(SPEECH_RX_NS)
    speech_ns_destroy(speech_rx_ns_st);
#endif

#if defined(SPEECH_RX_NS2)
    speech_ns2_destroy(speech_rx_ns2_st);
#endif

#if defined(SPEECH_RX_NS2FLOAT)
    speech_ns2float_destroy(speech_rx_ns2float_st);
#endif

#ifdef SPEECH_RX_NS3
    ns3_destroy(speech_rx_ns3_st);
#endif

    TRACE(1,"[%s] End", __func__);

    return 0;
}
#endif




int speech_init(int tx_sample_rate, int rx_sample_rate,
                     int tx_frame_ms, int rx_frame_ms,
                     int sco_frame_ms,
                     uint8_t *buf, int len)
{
    // we shoule keep a minmum buffer for speech heap
    // MSBC_16K_SAMPLE_RATE = 0, 560 bytes
    // MSBC_16K_SAMPLE_RATE = 1, 2568 bytes
    sp_fadein_count = 0;

	//debug¿âÊÚÈ¨
	#if 0
	if(strlen((const char *)key) == 0)
	{	
		sndp_auth_get_bt_address(((factory_section_t *)__factory_start)->head.version);  
		key[0] = 1; 
	} 
	#endif
	int ret; 
	uint8_t key[16] = {0};
	ret = soundplus_auth(key, 16); 
	TRACE(1, "<soundplus_auth> ret=%d", ret); 
	ret = soundplus_auth_status(); 
	TRACE(1, "<soundplus_auth_status> ret=%d", ret);
	//#endif

	

#ifdef SNDP_BTDM_SUPPORT
    speech_heap_init(buf,len);
#else
	
    speech_heap_init(buf, SPEECH_HEAP_RESERVE_SIZE);

    uint8_t *free_buf = buf + SPEECH_HEAP_RESERVE_SIZE;
    int free_len = len - SPEECH_HEAP_RESERVE_SIZE;

    // use free_buf for your algorithm
    memset(free_buf, 0, free_len);
#endif	

    int frame_len = SPEECH_FRAME_MS_TO_LEN(tx_sample_rate, tx_frame_ms);

    aec_echo_buf = (short *)speech_calloc(frame_len, sizeof(short));
    aec_echo_buf_ptr = aec_echo_buf;

#if defined(SNDP_DUMP_TX_DATA)
    audio_dump_init(frame_len, sizeof(int16_t), 4);
#endif

    #if defined(SCO_CP_ACCEL)
     sco_cp_init(frame_len,SPEECH_CODEC_CAPTURE_CHANNEL_NUM);
    #endif	



#ifdef __BES_RX__
	speech_cfg = (SpeechConfig *)speech_calloc(1, sizeof(SpeechConfig));
    speech_store_config(&speech_cfg_default);



    speech_rx_sample_rate = rx_sample_rate;
    speech_rx_frame_len = SPEECH_FRAME_MS_TO_LEN(rx_sample_rate, rx_frame_ms);
	
	speech_rx_init(speech_rx_sample_rate, speech_rx_frame_len);
#endif	

    return 0;
}

int speech_deinit(void)
{
#ifdef __BES_RX__
    speech_rx_deinit();
#endif

    speech_free(aec_echo_buf_ptr);
	
    #if defined(SCO_CP_ACCEL)
     sco_cp_deinit();
    #endif	
	
    size_t total = 0, used = 0, max_used = 0;
    speech_memory_info(&total, &used, &max_used);
    TRACE(3,"SPEECH MALLOC MEM: total - %d, used - %d, max_used - %d.", total, used, max_used);
    ASSERT(used == 0, "[%s] used != 0", __func__);

    return 0;
}

#if defined(BONE_SENSOR_TDM)
extern void bt_sco_get_tdm_buffer(uint8_t **buf, uint32_t *len);
#endif



void sndp_speech_fade_in_proc(short* pcm_buf,int cur_frame,int frame_len)
{
    float f32_buf = 0.0f;
	//int8_t fadein_num = frame_len * SP_FADEIN_COUNT;
	float fadein_num = (float)(frame_len * SP_FADEIN_COUNT);
    TRACE(3,"cur_frame - %d, frame_len - %d fadein_num - %d", cur_frame, frame_len,(int)fadein_num);

	
	if((cur_frame< FADEIN_START_FRM)||(cur_frame>= FADEIN_STOP_FRM))
	{
        return;
	}
	cur_frame -= FADEIN_START_FRM;
	for(int i=0;i<frame_len;i++)
	{
         f32_buf = (float)pcm_buf[i];
		 f32_buf = f32_buf/fadein_num;
		 pcm_buf[i] = (short)(f32_buf *(i+ cur_frame*frame_len)); 
	}

}

int speech_tx_process(void *pcm_buf, void *ref_buf, int *pcm_len)
{
    int16_t *pcm16_buf = (int16_t *)pcm_buf;
    int16_t *ref16_buf = (int16_t *)ref_buf;
	int pcm16_len = *pcm_len;

#ifdef __FJH_MIC_TEST__
switch(mic_test_mode)
{
	case EN_SN:
	break;

	case main_mic:
	case DIS_SN:
	   for(int i=0; i<pcm16_len/2; i++)
	   {
		   pcm16_buf[i] = pcm16_buf[i*2+1]; 	
	   }	
	   *pcm_len = pcm16_len/2;
	   return 0;
	 break;

	   
	 case FF_mic:
	   for(int i=0; i<pcm16_len/2; i++)
	   {
		   pcm16_buf[i] = pcm16_buf[i*2];		
	   }   
	   *pcm_len = pcm16_len/2;
	   return 0;
	 break;
	 
	 default:break;  
}
#endif


#if defined(SNDP_DUMP_TX_DATA)
		short tmp[240] = {0};
		audio_dump_clear_up();
		
		for(int i=0; i<pcm16_len / SPEECH_CODEC_CAPTURE_CHANNEL_NUM; i++)
			tmp[i] = pcm16_buf[i * 2];
		audio_dump_add_channel_data(0, tmp, pcm16_len / SPEECH_CODEC_CAPTURE_CHANNEL_NUM);


		for(int i=0; i<pcm16_len / SPEECH_CODEC_CAPTURE_CHANNEL_NUM; i++)
			tmp[i] = pcm16_buf[i * 2 + 1];
		audio_dump_add_channel_data(1, tmp, pcm16_len / SPEECH_CODEC_CAPTURE_CHANNEL_NUM);
		
		audio_dump_add_channel_data(2, ref16_buf, pcm16_len / SPEECH_CODEC_CAPTURE_CHANNEL_NUM);
#endif		


if(sp_fadein_count < SP_MUTE_COUNT)
{
   memset(pcm16_buf, 0, pcm16_len/SPEECH_CODEC_CAPTURE_CHANNEL_NUM*sizeof(int16_t));
   TRACE(1,"mute time = [%d].",sp_fadein_count);
}
else	
{
#if defined(SCO_CP_ACCEL)	 
     sco_cp_process(pcm16_buf,ref16_buf,pcm_len);
#endif

	if((sp_fadein_count >= FADEIN_START_FRM) && (sp_fadein_count < FADEIN_STOP_FRM))
	{
	   sndp_speech_fade_in_proc(pcm16_buf,sp_fadein_count,pcm16_len/ SPEECH_CODEC_CAPTURE_CHANNEL_NUM);
	   TRACE(1,"fade in time = [%d].",sp_fadein_count);
	}	
}
	sp_fadein_count++;

#if defined(SNDP_DUMP_TX_DATA)
	audio_dump_add_channel_data(3, pcm16_buf, pcm16_len / SPEECH_CODEC_CAPTURE_CHANNEL_NUM);
	audio_dump_run();
#endif


    return 0;
}

int32_t _speech_rx_process_(void *pcm_buf, int32_t *_pcm_len)
{
    int32_t pcm_len = *_pcm_len;

#if defined(BT_SCO_CHAIN_PROFILE)
    uint32_t start_ticks = hal_fast_sys_timer_get();
#endif

#if defined(SPEECH_RX_24BIT)
    int32_t *buf32 = (int32_t *)pcm_buf;
    int16_t *buf16 = (int16_t *)pcm_buf;
    for (int i = 0; i < pcm_len; i++) {
        buf16[i] = (int16_t)(buf32[i] >> 8);
    }
#endif

#if defined(BT_SCO_CHAIN_AUDIO_DUMP)
    // audio_dump_add_channel_data(0, pcm_buf, pcm_len);
#endif

#if defined(SPEECH_RX_NS)
    speech_ns_process(speech_rx_ns_st, pcm_buf, pcm_len);
#endif

#if defined(SPEECH_RX_NS2) 
    // fix 0dB signal
    int16_t *pcm_buf16 = (int16_t *)pcm_buf;
    for(int i=0; i<pcm_len; i++)
    {
        pcm_buf16[i] = (int16_t)(pcm_buf16[i] * 0.94);
    }
    speech_ns2_process(speech_rx_ns2_st, pcm_buf, pcm_len);
#endif

#if defined(SPEECH_RX_NS2FLOAT)
    // FIXME
    int16_t *pcm_buf16 = (int16_t *)pcm_buf;
    for(int i=0; i<pcm_len; i++)
    {
        pcm_buf16[i] = (int16_t)(pcm_buf16[i] * 0.94);
    }
    speech_ns2float_process(speech_rx_ns2float_st, pcm_buf, pcm_len);
#endif

#ifdef SPEECH_RX_NS3
    ns3_process(speech_rx_ns3_st, pcm_buf, pcm_len);
#endif

#if defined(SPEECH_RX_AGC)
    agc_process(speech_rx_agc_st, pcm_buf, pcm_len);
#endif

#if defined(SPEECH_RX_24BIT)
    for (int i = pcm_len - 1; i >= 0; i--) {
        buf32[i] = ((int32_t)buf16[i] << 8);
    }
#endif

#if defined(SPEECH_RX_EQ)
#if defined(SPEECH_RX_24BIT)
    eq_process_int24(speech_rx_eq_st, pcm_buf, pcm_len);
#else
    eq_process(speech_rx_eq_st, pcm_buf, pcm_len);
#endif
#endif

#if defined(SPEECH_RX_POST_GAIN)
    speech_gain_process(speech_rx_post_gain_st, pcm_buf, pcm_len);
#endif

#if defined(BT_SCO_CHAIN_AUDIO_DUMP)
    // audio_dump_add_channel_data(1, pcm_buf, pcm_len);
    // audio_dump_run();
#endif

    *_pcm_len = pcm_len;

#if defined(BT_SCO_CHAIN_PROFILE)
    uint32_t end_ticks = hal_fast_sys_timer_get();
    TRACE(2,"[%s] takes %d us", __FUNCTION__,
        FAST_TICKS_TO_US(end_ticks - start_ticks));
#endif

    return 0;
}




//SPEECH_PCM_T POSSIBLY_UNUSED* _rx_pcm_buf = (SPEECH_PCM_T*)rx_pcm;
int speech_rx_process(void *pcm_buf, int *pcm_len)
{
	// TRACE(2,"%s pcm_len [%d].",__func__,*pcm_len);
	 

#ifndef __BES_RX__
    short* rx_pcm = (short*) pcm_buf;
    
    for(int i=0;i< *pcm_len;i++)
    {
       rx_pcm[i] = rx_pcm[i] >> 1;  // -6db
	}

#if defined(SNDP_DUMP_RX_DATA)
	
	int16_t* rx16_pcm_buf = (int16_t*)rx_pcm;
	int _rx_pcm_len = *pcm_len;

	int16_t dump_temp[240];
	memcpy(dump_temp,rx16_pcm_buf,_rx_pcm_len * sizeof(short));
	audio_dump_init(_rx_pcm_len, sizeof(int16_t), 2);
	audio_dump_clear_up();
	audio_dump_add_channel_data(0, dump_temp,_rx_pcm_len);		
#endif

	soundplus_deal_Rx(rx_pcm,*pcm_len);


#if defined(SNDP_DUMP_RX_DATA)
	audio_dump_add_channel_data(1, rx_pcm,_rx_pcm_len);
	audio_dump_run();	
#endif

#endif


#ifdef __BES_RX__
	_speech_rx_process_(pcm_buf, (int32_t *)pcm_len);
#endif


    // Add your algorithm here
    return 0;
}


#ifdef __FJH_MIC_TEST__
void mic_test(uint8_t mic_mode)
{
     mic_test_mode = mic_mode;
}


uint8_t get_mic_mode(void)
{
    return mic_test_mode;
}
#endif

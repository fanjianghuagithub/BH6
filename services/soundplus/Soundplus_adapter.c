#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "sndp_config.inc"
#include "sndp_lib.h"
#include "Soundplus_adapter.h"
#ifndef WIN32
#include "stdbool.h"
#include "heap_api.h"
#include "hal_trace.h"
#else
typedef unsigned char               bool;
#define true                        1
#define false                       0
#define med_malloc malloc
#define med_free free
#define TRACE printf
#endif
#define DELAY		(75)
#define DELAY_NB	(370)
#define FRMLEN		(240)
#define REFBUFLEN	(DELAY + FRMLEN)
#define MIC_NUM 2
#define EQ_BYPASS 0

static bool soundplus_keyReadFlag = false;
static bool soundplus_inited = false;

void *pBuff_TX=NULL;
static short *pRef_align = NULL;
static float *pMic_In = NULL;
static float *pRef_In = NULL;

#if EQ_BYPASS
const float TX_EQ_curve_16k_perm[512] = {
#include "eq_param/BYPASS_EQ_curve_512_perm.txt"
};

const float TX_EQ_curve_8k_perm[256] = {
#include "eq_param/BYPASS_EQ_curve_256_perm.txt"
};

const float RX_EQ_curve_16k_perm[512] = {
#include "eq_param/BYPASS_EQ_curve_512_perm.txt"
};

const float RX_EQ_curve_8k_perm[256] = {
#include "eq_param/BYPASS_EQ_curve_256_perm.txt"
};
#else
const float TX_EQ_curve_16k_perm[512] = {
#include "eq_param/TX_EQ_curve_16k_perm.txt"
};

const float TX_EQ_curve_8k_perm[256] = {
#include "eq_param/TX_EQ_curve_8k_perm.txt"
};

const float RX_EQ_curve_16k_perm[512] = {
#include "eq_param/RX_EQ_curve_16k_perm.txt"
};

const float RX_EQ_curve_8k_perm[256] = {
#include "eq_param/RX_EQ_curve_8k_perm.txt"
};

#endif

int soundplus_deal_Tx(short *buf, short *ref, int buf_len, int ref_len)
{
	int i;
	float tmpAbs;
    int numsamples = buf_len/MIC_NUM;
    
	if (ref_len == FRMLEN)
	{
		memmove(pRef_align, pRef_align + ref_len, sizeof(short) * DELAY);
		memcpy(pRef_align + DELAY, ref, sizeof(short) * ref_len);
	}
	else
	{
		memmove(pRef_align, pRef_align + ref_len, sizeof(short) * DELAY_NB);
		memcpy(pRef_align + DELAY_NB, ref, sizeof(short) * ref_len);
	}

	for(i=0; i<numsamples; i++)
	{
		pMic_In[i] = buf[MIC_NUM*i+1];
		pMic_In[numsamples + i] = buf[MIC_NUM*i+0];
	}
    
    for(i=0; i<ref_len; i++)
    {
        pRef_In[i] = pRef_align[i];
    }

    Sndp_SpxEnh_Tx(pMic_In, pMic_In, pRef_In, numsamples, ref_len);

	for(i=0; i<numsamples; i++)
	{
		tmpAbs = pMic_In[i];
		if (tmpAbs < -32768) tmpAbs = -32768;
		if (tmpAbs > 32767) tmpAbs = 32767;
		buf[i] = (short)tmpAbs;
	}
	
	return 0;
}

int soundplus_deal_Rx(short *inX, int len)
{
	int i;
	float tmpAbs;
    float din[240];

    assert((int)((ConfigTab[3] & 0x1000000)>>24) == 1);

    for(i=0; i<len; i++)
    {
        din[i] = inX[i];
    }

	Sndp_SpxEnh_Rx(din, din, len);

	for(i=0; i<len; i++)
	{
		tmpAbs = din[i];
		if (tmpAbs < -32768) tmpAbs = -32768;
		if (tmpAbs > 32767) tmpAbs = 32767;
		inX[i] = (short)tmpAbs;
	}

	return 0;
}

int soundplus_auth(uint8_t* key, int Key_len)
{
	int ret = 0;
	
    if(soundplus_keyReadFlag == false)
    {
        ret = sndp_license_auth(key, Key_len);
        soundplus_keyReadFlag = true;
    }

    return ret;
}

int soundplus_auth_status()
{
	return sndp_license_status_get();
}


int soundplus_init(int NrwFlag)
{
	//recomond value:15~40 db
	float PostGmin = 30.0f;    //db

    if (soundplus_inited) {return -1;}

	// memory init, get number, malloc memory
	int memsize = Sndp_SpxEnh_MemSize(NrwFlag);
	//TRACE("<%s> memsize=%d version=%s", __func__,memsize, get_soundp_alg_ver());
	pBuff_TX = (void *)med_malloc(memsize);
	if(pBuff_TX == NULL) {return -1;}
    pMic_In = (float *)med_malloc(FRMLEN * MIC_NUM * sizeof(float));
    if(pMic_In == NULL) {return -1;}
    pRef_In = (float *)med_malloc(FRMLEN * sizeof(float));
    if(pRef_In == NULL) {return -1;}
    
    pRef_align = (short *)med_malloc(REFBUFLEN * sizeof(short));
    if(pRef_align == NULL) {return -1;}

    Sndp_SpxEnh_Init(pBuff_TX, PostGmin); 
	
	soundplus_inited = true;
	
	return 0;
}

int soundplus_deinit(void)
{
	if(pBuff_TX)
    {   
	    med_free((void *)pBuff_TX);
        pBuff_TX = NULL;
    }
    if(pMic_In)
    {   
	    med_free((void *)pMic_In);
        pMic_In = NULL;
    }
    if(pRef_In)
    {   
	    med_free((void *)pRef_In);
        pRef_In = NULL;
    }
    if(pRef_align)
    {   
	    med_free((void *)pRef_align);
        pRef_align = NULL;
    }

    soundplus_inited = false;

	return 0;
}


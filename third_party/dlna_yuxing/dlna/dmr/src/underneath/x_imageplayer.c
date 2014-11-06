/*--------------------------------------------------------------*/
/*
* Golden Yuxing CONFIDENTIAL
* Copyright (c) 2003, 2004 Yuxing Corporation.  All rights reserved.
* 
* The computer program contained herein contains proprietary information
* which is the property of Golden Yuxing Co., Ltd.  The program may be used
* and/or copied only with the written permission of Golden Yuxing Co., Ltd.
* or in accordance with the terms and conditions stipulated in the
* agreement/contract under which the programs have been supplied.
*
*    filename:               mediaPlayerEx.h
*    author(s):              valiant
*    version:                ver 0.1
*    date:					 2005/03/15
* History
* ----------------
*/
/*---------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
//#include <semaphore.h>

#include "HySDK.h"
#include "x_imageplayer.h"
#include "hitTime.h"
#include "upnp.h"


#define P_PLAYER(p)	(p->player)
#define TEMP_IMAGE_FILE		"/var/dlna_img.tmp"
#if 1
/* application/x-dtcp1;DTCP1HOST=<host>;DTCP1PORT=<port>;CONTENTFORMAT=<MIME-type> */
// BUT when playing image we dont need to take dtcp consideration at all
static void* s_Img_DownloadWithDtcp(void *arg)
{
#define IN_DATA_LEN (128 * 1024)
	t_DMR_IMGP *player = (t_DMR_IMGP *)arg;

	FILE *tmp_file = NULL;
	void *img_http_handle = NULL;
	char *content_type = NULL;
	int content_len = 0;
	int http_status = 0;
	int http_ret, ret = -1;
	
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_FEW, 0, 0);
	
	

	http_ret = UpnpOpenHttpGet(player->url, &img_http_handle, &content_type, &content_len, &http_status, 1);
	if( http_ret < 0 ) 
		goto err;

	tmp_file = fopen(TEMP_IMAGE_FILE, "w");
	if( tmp_file == NULL )
	{
		UpnpCloseHttpGet(img_http_handle);
		ret = -2;
		goto err;
	}

	char buf[IN_DATA_LEN];
	char *raw_data = NULL;
	int readed_len = 0;
	unsigned int data_len = 0;

	while(1)
	{
		raw_data = buf;
		data_len = IN_DATA_LEN;
		http_ret = UpnpReadHttpGet(img_http_handle, raw_data, &data_len, 1);
		HT_DBG_FUNC(readed_len, "readed_len = \n");
		HT_DBG_FUNC(data_len, "data_len = \n");
		if(data_len > 0 )
		{
			fwrite(raw_data, data_len, 1, tmp_file);
			readed_len += data_len;
		} 
		else 
			break;

		if( player->cancel )
			break;
	}
	
	if( content_len == readed_len )
		ret = 0;
	else
		ret = -3;
	
	UpnpCloseHttpGet(img_http_handle);
	fclose(tmp_file);

err:
	free(player->url);
	player->cancel = 0;
	player->url = NULL;
	HT_DBG_FUNC_END(ret, 0);
	return (void*)ret;
}
#endif
static int image_process_start(t_DMR_IMGP *player)
{
//	pthread_t process_pid;
	
//	if( player->url )
//		return -1;

	player->cancel = 0;
//	pthread_create(&process_pid, NULL, s_Img_DownloadWithDtcp, (void *)player);
	s_Img_DownloadWithDtcp(player);
	return 0;
}

static void image_process_stop(t_DMR_IMGP *player)
{
	if( player->url )
	{
		player->cancel = 1;
		while(player->cancel)
			usleep(50*1000);
	}
}

int DMR_IPlayer_SetUri(t_DMR_IMGP *player, char *uri, t_MEDIA_INFO *mInfo)
{
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_FEW, (int)mInfo, uri);

	int ret = -1;
	if( uri )
	{
		if( player->url )
			free(player->url);
		player->url = strdup(uri);

		if(mInfo)
			memcpy(&(player->minfo), mInfo, sizeof(t_MEDIA_INFO));
		ret = 0;
	}
	
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}

int DMR_IPlayer_Play(t_DMR_IMGP *player, int slideMode)
{
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_FEW, slideMode, 0);

	int ret = -1;
	if( player->url)
	{
		ret = image_process_start(player);
		if( ret == 0)
			ret =HySDK_IPlayer_Play(P_PLAYER(player), TEMP_IMAGE_FILE, slideMode);
	}
	
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}
int DMR_IPlayer_Zoom(t_DMR_IMGP *player, int percent)
{
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_FEW, percent, 0);
	
	int ret = HySDK_IPlayer_Zoom(P_PLAYER(player), percent);
	
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}
int DMR_IPlayer_Move(t_DMR_IMGP *player, int x, int y)
{
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_FEW, x, 0);
	
	int ret = HySDK_IPlayer_Move(P_PLAYER(player), x, y);
	
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}
int DMR_IPlayer_Rotate(t_DMR_IMGP *player, int radian)
{
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_FEW, radian, 0);
	
	int ret = HySDK_IPlayer_Rotate(P_PLAYER(player), radian);
	
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}
int DMR_IPlayer_Stop(t_DMR_IMGP *player)
{
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_FEW, 0, 0);
	
	int ret = HySDK_IPlayer_Stop(P_PLAYER(player));
	image_process_stop(player);
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}

t_MEDIA_INFO* DMR_IPlayer_GetMediaInfo(t_DMR_IMGP *player)
{
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_FEW, 0, 0);

	t_MEDIA_INFO *minfo = &(player->minfo);
	minfo->resWidth = 0;
	minfo->resHeight= 0;
	HySDK_IPlayer_GetMediaInfo(P_PLAYER(player), &(minfo->resWidth), &(minfo->resHeight));

	HT_DBG_FUNC_END((int)minfo, 0);
	return minfo;
}

void DMR_IPlayer_Release(t_DMR_IMGP *player)
{
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_FEW, (int)player, 0);
	if( player )
	{
		HySDK_IPlayer_Release(P_PLAYER(player));
		
		if( player->url)
			free(player->url);
		
		free( player );
	}
	
	HT_DBG_FUNC_END(0, 0);
}

t_DMR_IMGP* DMR_IPlayer_Create(t_PLAYER_EVENT callBack, void *user)
{
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_FEW, (int)callBack, 0);
	
	t_DMR_IMGP *me = (t_DMR_IMGP*)malloc(sizeof(t_DMR_IMGP));
	memset(me, 0, sizeof(t_DMR_IMGP));
	
	me->callback	= callBack;
	me->user		= user;
	me->player 		= HySDK_IPlayer_Create();

	me->SetUri 		= DMR_IPlayer_SetUri;
	me->Play 		= DMR_IPlayer_Play;
	me->Zoom 		= DMR_IPlayer_Zoom;
	me->Move 		= DMR_IPlayer_Move;
	me->Rotate 		= DMR_IPlayer_Rotate;
	me->Stop 		= DMR_IPlayer_Stop;
	me->Release 	= DMR_IPlayer_Release;

	HT_DBG_FUNC_END((int)me, 0);
	return me;
}





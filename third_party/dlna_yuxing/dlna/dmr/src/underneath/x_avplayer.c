

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
#include <stdlib.h> 
#include <malloc.h> 
#include <string.h> 

#include "x_avplayer.h"
#include "HySDK.h"
#include "hitTime.h"

#define P_PLAYER(p)	(p->player)


static int DMR_AVPlayer_NULL(t_DMR_AVP *player, int speed)
{
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_FEW, 0, 0);
	
	
	HT_DBG_FUNC_END(0, 0);
	return -1;
}


static int DMR_AVPlayer_SetUri(t_DMR_AVP *player, char *url, t_MEDIA_INFO *mInfo)
{
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_KEY, (int)mInfo, url);

	int ret = -1;
	if( url )
	{
		if( player->url )
			free(player->url);
		player->url = strdup(url);

		if(mInfo)
			memcpy(&(player->minfo), mInfo, sizeof(t_MEDIA_INFO));
		ret = 0;
	}
	
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}

static int DMR_AVPlayer_Play(t_DMR_AVP *player, int speed)
{
	int ret = 0;
	
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_KEY, speed, 0);

	t_MEDIA_INFO *minfo = &(player->minfo);
	ret = HySDK_AVPlayer_Play(P_PLAYER(player), player->url, minfo->majorType? minfo : NULL);
	
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}

static int DMR_AVPlayer_Stop(t_DMR_AVP *player)
{
	int ret = 0;
	
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_KEY, 0, 0);
	
	ret = HySDK_AVPlayer_Stop(P_PLAYER(player));
	
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}

static int DMR_AVPlayer_Pause(t_DMR_AVP *player)
{
	int ret = 0;
	
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_KEY, 0, 0);
	
	ret = HySDK_AVPlayer_Pause(P_PLAYER(player));
	
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}
static int DMR_AVPlayer_Resume(t_DMR_AVP *player)
{
	int ret = 0;
	
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_KEY, 0, 0);
	
	ret = HySDK_AVPlayer_Resume(P_PLAYER(player));
	
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}


static int DMR_AVPlayer_Forward(t_DMR_AVP *player, int speed)
{
	int ret = 0;
	
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_KEY, speed, 0);
	
	ret = HySDK_AVPlayer_Forward(P_PLAYER(player), speed);
	
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}
static int DMR_AVPlayer_Reverse(t_DMR_AVP *player, int speed)
{
	int ret = 0;
	
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_KEY, speed, 0);
	
	ret = HySDK_AVPlayer_Reverse(P_PLAYER(player), speed);
	
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}
static int DMR_AVPlayer_Slow(t_DMR_AVP *player, int speed)
{
	int ret = 0;
	
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_KEY, speed, 0);
	
	ret = HySDK_AVPlayer_Slow(P_PLAYER(player), speed);
	
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}
static int DMR_AVPlayer_Seek(t_DMR_AVP *player, int seekMode, long long position)
{
	int ret = 0;
	
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_KEY, seekMode, 0);
	HT_DBG_PRINTF(HT_MOD_HYSDK, HT_BIT_KEY, "position = %lld\n", position);
	
	ret = HySDK_AVPlayer_Seek(P_PLAYER(player), seekMode, position);
	
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}


static int DMR_AVPlayer_GetTotalTime(t_DMR_AVP *player, int *total)
{
	int ret = 0;
	
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_FEW, 0, 0);
	
	ret = HySDK_AVPlayer_GetTotalTime(P_PLAYER(player), total);
	
	HT_DBG_PRINTF(HT_MOD_HYSDK, HT_BIT_FEW, "*total = %d\n", *total);
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}
static int DMR_AVPlayer_GetCurrentTime(t_DMR_AVP *player, int *now)
{
	int ret = 0;
	
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_FEW, 0, 0);
	
	ret = HySDK_AVPlayer_GetCurrentTime(P_PLAYER(player), now);
	
	HT_DBG_PRINTF(HT_MOD_HYSDK, HT_BIT_FEW, "*now = %d\n", *now);
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}
static int DMR_AVPlayer_GetTotalLength(t_DMR_AVP *player, long long *total)
{
	int ret = 0;
	
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_FEW, 0, 0);
	
	ret = HySDK_AVPlayer_GetTotalLength(P_PLAYER(player), total);
	
	HT_DBG_PRINTF(HT_MOD_HYSDK, HT_BIT_FEW, "*total = %lld\n", *total);
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}
static int DMR_AVPlayer_GetCurrentLength(t_DMR_AVP *player, long long *now)
{
	int ret = 0;
	
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_FEW, 0, 0);
	
	ret = HySDK_AVPlayer_GetCurrentLength(P_PLAYER(player), now);
	
	HT_DBG_PRINTF(HT_MOD_HYSDK, HT_BIT_FEW, "*now = %lld\n", *now);
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}

static PLAYER_STATE DMR_AVPlayer_GetState(t_DMR_AVP *player)
{
	PLAYER_STATE ret = PLAYER_STATE_IDLE;
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_FEW, 0, 0);
	
	ret = HySDK_AVPlayer_GetState(P_PLAYER(player));
	
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}
static int DMR_AVPlayer_GetMediaInfo(t_DMR_AVP *player, t_MEDIA_INFO* minfo)
{
	int ret = 0;
	
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_FEW, 0, 0);
	
	ret = HySDK_AVPlayer_GetMediaInfo(P_PLAYER(player), minfo);
	
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}
static int DMR_AVPlayer_GetPlaySpeed(t_DMR_AVP *player,  char *buf, int size)
{
	int ret = 0;
	
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_FEW, 0, 0);
	
	ret = HySDK_AVPlayer_GetPlaySpeed(P_PLAYER(player), buf, size);
	
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}


/*----------------------------*/
static int DMR_Subtitle_GetCount(t_DMR_AVP* player, int *count)
{
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_KEY, 0, 0);
	
	int ret = HySDK_Subtitle_GetCount(P_PLAYER(player), count);
	
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}

static int DMR_Subtitle_GetPidViaIndex(t_DMR_AVP* player, int index, int *pid)
{
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_KEY, index, 0);
	
	int ret = HySDK_Subtitle_GetPidViaIndex(P_PLAYER(player), index, pid);
	
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}

static int DMR_Subtitle_GetLanViaIndex(t_DMR_AVP* player, int index, int *lang_code, char*language, int language_len)
{
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_KEY, 0, 0);
	
	int ret = HySDK_Subtitle_GetLanViaIndex(P_PLAYER(player), index, lang_code, language, language_len);
	
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}

static int DMR_Subtitle_GetCurrentPID(t_DMR_AVP* player, int *pid)
{
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_KEY, 0, 0);
	
	int ret = HySDK_Subtitle_GetCurrentPID(P_PLAYER(player), pid);
	
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}

static int DMR_Subtitle_SetCurrentPID(t_DMR_AVP* player,  int pid)
{
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_KEY, 0, 0);
	
	int ret = HySDK_Subtitle_SetCurrentPID(P_PLAYER(player), pid);
	
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}



static int DMR_AudioTrack_GetCount(t_DMR_AVP* player, int *count)
{
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_KEY, 0, 0);
	
	int ret = HySDK_AudioTrack_GetCount(P_PLAYER(player), count );
	
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}

static int DMR_AudioTrack_GetPidViaIndex(t_DMR_AVP* player, int index, int *pid)
{
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_KEY, 0, 0);
	
	int ret = HySDK_AudioTrack_GetPidViaIndex(P_PLAYER(player), index, pid);
	
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}

static int DMR_AudioTrack_GetLanViaIndex(t_DMR_AVP* player, int index, int *lang_code, char*language, int language_len)
{
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_KEY, 0, 0);
	
	int ret = HySDK_AudioTrack_GetLanViaIndex(P_PLAYER(player), index, lang_code, language, language_len);
	
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}

static int DMR_AudioTrack_GetCurrentPID(t_DMR_AVP* player, int *pid)
{
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_KEY, 0, 0);
	
	int ret = HySDK_AudioTrack_GetCurrentPID(P_PLAYER(player), pid );
	
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}

static int DMR_AudioTrack_SetCurrentPID(t_DMR_AVP* player, int pid)
{
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_KEY, 0, 0);
	
	int ret = HySDK_AudioTrack_SetCurrentPID(P_PLAYER(player), pid);
	
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}
/*----------------------------*/



void DMR_AVPlayer_Release(t_DMR_AVP *player)
{
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_KEY, 0, 0);

	if( player )
	{
		HySDK_AVPlayer_Release(P_PLAYER(player));
		if( player->url )
			free(player->url);
		free(player);
	}
	
	HT_DBG_FUNC_END(0, 0);
}

static int s_DMR_AVPlayer_Callback(int playerEventType, int value, int arg, void *user)
{
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_FEW, playerEventType, "playerEventType = ");
	t_DMR_AVP *player = (t_DMR_AVP *)user;
	
	if( player->callback )
		player->callback(playerEventType, value, arg, player->user);
	
	HT_DBG_FUNC_END(0, 0);

	return 0;
}
//huawei2£¤¡¤?¡ä¡ä?¡§
t_DMR_AVP* DMR_AVPlayer_Create(t_PLAYER_EVENT callBack, void *user)
{
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_KEY, 0, 0);
	
	t_DMR_AVP *me		= (t_DMR_AVP*)malloc(sizeof(t_DMR_AVP));
	memset(me, 0, sizeof(t_DMR_AVP));
	
	me->callback		= callBack;
	me->user			= user;

	me->SetUri			= DMR_AVPlayer_SetUri;
	me->Play			= DMR_AVPlayer_Play;
	me->Stop			= DMR_AVPlayer_Stop;
	me->Pause			= DMR_AVPlayer_Pause;
	me->Resume			= DMR_AVPlayer_Resume;
	
	me->FastForward		= DMR_AVPlayer_Forward;
	me->FastBackward	= DMR_AVPlayer_Reverse;
	me->SlowForward		= DMR_AVPlayer_Slow;
	me->SlowBackward	= DMR_AVPlayer_NULL;
	me->Seek			= DMR_AVPlayer_Seek;
	
	me->GetPlaySpeed	= DMR_AVPlayer_GetPlaySpeed;
	me->GetTotalLength	= DMR_AVPlayer_GetTotalLength;
	me->GetCurrentLength= DMR_AVPlayer_GetCurrentLength;
	me->GetTotalTime	= DMR_AVPlayer_GetTotalTime;
	me->GetCurrentTime	= DMR_AVPlayer_GetCurrentTime;
	
	me->GetState		= DMR_AVPlayer_GetState;
	me->GetMediaInfo	= DMR_AVPlayer_GetMediaInfo;
	
	me->Release			= DMR_AVPlayer_Release;

	me->Subtitle_GetTotalCount	= DMR_Subtitle_GetCount;
	me->Subtitle_GetPidViaIndex	= DMR_Subtitle_GetPidViaIndex;
	me->Subtitle_GetLanViaIndex	= DMR_Subtitle_GetLanViaIndex;
	me->Subtitle_GetCurrentPID	= DMR_Subtitle_GetCurrentPID;
	me->Subtitle_SetCurrentPID	= DMR_Subtitle_SetCurrentPID;

	me->AudioTrack_GetTotalCount	= DMR_AudioTrack_GetCount;
	me->AudioTrack_GetPidViaIndex	= DMR_AudioTrack_GetPidViaIndex;
	me->AudioTrack_GetLanViaIndex	= DMR_AudioTrack_GetLanViaIndex;
	me->AudioTrack_GetCurrentPID	= DMR_AudioTrack_GetCurrentPID;
	me->AudioTrack_SetCurrentPID	= DMR_AudioTrack_SetCurrentPID;
	
	me->player						= HySDK_AVPlayer_Create(s_DMR_AVPlayer_Callback, me);
	
	HT_DBG_FUNC_END((int)me, 0);
	return me;
}



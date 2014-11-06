

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
/*---------------------------------------------------------*/
#ifndef _X_AV_PLAYER_H_
#define _X_AV_PLAYER_H_

#include "dlna_type.h"
#include "HySDK.h"

#define FILE_PREFIX_HTTP		"http://"
#define FILE_PREFIX_FTP			"ftp://"
#define FILE_PREFIX_MULTICAST	"udp://"
#define FILE_PREFIX_LOCAL		"/"

typedef struct _general_media_player_ t_GM_PALYER;
struct _general_media_player_
{
	t_PLAYER_EVENT	callback;
	void*				user;
	int					player;

	t_MEDIA_INFO		minfo;
	char*				url;
	
	int (*SetUri)(t_GM_PALYER *player, char *url, t_MEDIA_INFO *mInfo);/* 播放。 uri: 指定的电影或音乐；minfo: 该媒体的相关信息，如果未知可为空 */
	int (*Play)(t_GM_PALYER *player, int speed);/* 播放。 uri: 指定的电影或音乐；minfo: 该媒体的相关信息，如果未知可为空 */
						  /*返回值: 若uri为空或参数有误则返回-1，正常情况下返回0 */

	int (*SeekPlay)(t_GM_PALYER *player, int speed);
	int (*Stop)(t_GM_PALYER *player );	 		/* 停止当前播放,播放器将进入PLAYER_STATE_IDLE状态 */
	int (*Pause)(t_GM_PALYER *player );			/* 由播放变为暂停, 播放器将进入PLAYER_STATE_PAUSE状态 */
	int (*Resume)(t_GM_PALYER *player );	 		/* 由暂停、快进、快退、慢放或慢退恢复为正常播放, 播放器将进入PLAYER_STATE_PLAY状态 */
	int (*FastForward)(t_GM_PALYER *player, int speed);	/* 快进 */
	int (*FastBackward)(t_GM_PALYER *player, int speed);	/* 快退 */
	int (*SlowForward)(t_GM_PALYER *player, int speed);	/* 慢放 */
	int (*SlowBackward)(t_GM_PALYER *player, int speed);	/* 慢退， 暂未实现 */
	/* 上面4个形参speed不见得一定执行，实际的速度通过getPlaySpeed获取，连续调用将按-1倍速、-2倍速、-4倍速、...、-64倍速(可能)、1倍速、-1倍速、...循环 */
	int (*Seek)(t_GM_PALYER *player, int method, long long offset);	/* 跳转播放。 当method=0时，offset为long long int *，否则offset为struct timeval *。*/

	int (*GetPlaySpeed)(t_GM_PALYER *player, char *speed, int size);	/* 获得当前模式下对应的播放速度。*/
	int (*GetTotalLength)(t_GM_PALYER *player, long long *total);	/* 获取媒体内容的总长度，以字节为单位。*/
	int (*GetCurrentLength)(t_GM_PALYER *player, long long *now);/* 获取当前的播放长度，以字节为单位。*/
	int (*GetTotalTime)(t_GM_PALYER *player, int *total);	/* 获取媒体内容的总长度，以毫秒为单位。*/
	int (*GetCurrentTime)(t_GM_PALYER *player, int *now);	/* 获取当前的播放长度，以毫秒为单位。*/

	PLAYER_STATE (*GetState)(t_GM_PALYER *player);			/* 获取播放器当前的状态 */
	int (*GetMediaInfo)(t_GM_PALYER *player, t_MEDIA_INFO* minfo);	/* 获取正在播放码流的详细信息，最好在收到MPCB_START_TO_PLAYBACK以后调用 */

	/* added by platform, not standard from dlna */
	int (*Subtitle_GetTotalCount)(t_GM_PALYER *player, int *count);			/* 获取媒体内容的总长度，以毫秒为单位。*/
	int (*Subtitle_GetPidViaIndex)(t_GM_PALYER *player, int index, int *pid);	/* 获取媒体内容的总长度，以毫秒为单位。*/
	int (*Subtitle_GetLanViaIndex)(t_GM_PALYER* player, int index, int *lang_code, char*language, int language_len);	/* 获取媒体内容的总长度，以毫秒为单位。*/
	int (*Subtitle_GetCurrentPID)(t_GM_PALYER *player, int *pid);		/* 获取媒体内容的总长度，以毫秒为单位。*/
	int (*Subtitle_SetCurrentPID)(t_GM_PALYER *player, int pid);	/* 获取媒体内容的总长度，以毫秒为单位。*/

	int (*AudioTrack_GetTotalCount)(t_GM_PALYER *player, int *count);			/* 获取媒体内容的总长度，以毫秒为单位。*/
	int (*AudioTrack_GetPidViaIndex)(t_GM_PALYER *player, int index, int *pid);/* 获取媒体内容的总长度，以毫秒为单位。*/
	int (*AudioTrack_GetLanViaIndex)(t_GM_PALYER* player, int index, int *lang_code, char*language, int language_len);/* 获取媒体内容的总长度，以毫秒为单位。*/
	int (*AudioTrack_GetCurrentPID)(t_GM_PALYER *player, int *pid);	/* 获取媒体内容的总长度，以毫秒为单位。*/
	int (*AudioTrack_SetCurrentPID)(t_GM_PALYER *player, int pid);	/* 获取媒体内容的总长度，以毫秒为单位。*/

	void (*Release)(t_GM_PALYER *player);					/* 关闭播放器并释放所有资源*/
} ;


#define IsPlayerPlaying(p)	(p.GetSate() !=PLAYER_STATE_IDLE)

typedef t_GM_PALYER t_DMR_AVP;
t_DMR_AVP* DMR_AVPlayer_Create(t_PLAYER_EVENT callBack, void *user);
void DMR_AVPlayer_Release(t_DMR_AVP* player);
#if 0
int DMR_AVPlayer_SetUri(t_DMR_AVP *player, char *url, t_MEDIA_INFO *mInfo);
int DMR_AVPlayer_Play(t_DMR_AVP *player, int speed);
int DMR_AVPlayer_Stop(t_DMR_AVP* player);
int DMR_AVPlayer_Pause(t_DMR_AVP* player);
int DMR_AVPlayer_Resume(t_DMR_AVP* player);
int DMR_AVPlayer_Forward(t_DMR_AVP* player, int speed);
int DMR_AVPlayer_Reverse(t_DMR_AVP* player, int speed);
int DMR_AVPlayer_Slow(t_DMR_AVP* player, int speed);
int DMR_AVPlayer_Seek(t_DMR_AVP* player, int seekMode, long long position);

int DMR_AVPlayer_GetTotalTime(t_DMR_AVP* player, int *total);
int DMR_AVPlayer_GetCurrentTime(t_DMR_AVP* player, int *now);
int DMR_AVPlayer_GetTotalLength(t_DMR_AVP* player, long long *total);
int DMR_AVPlayer_GetCurrentLength(t_DMR_AVP* player, long long *now);
PLAYER_STATE DMR_AVPlayer_GetState(t_DMR_AVP* player);
int DMR_AVPlayer_GetMediaInfo(t_DMR_AVP* player, t_MEDIA_INFO *minfo);
int DMR_AVPlayer_GetPlaySpeed(t_DMR_AVP* player,  char *buf, int size);

int DMR_Subtitle_GetCount(t_DMR_AVP* player, int *count);
int DMR_Subtitle_GetPidViaIndex(t_DMR_AVP* player, int index, int *pid);
int DMR_Subtitle_GetLanViaIndex(t_DMR_AVP* player, int index, char**language);
int DMR_Subtitle_GetCurrentPID(t_DMR_AVP* player, int *pid);
int DMR_Subtitle_SetCurrentPID(t_DMR_AVP* player, int pid);

int DMR_AudioTrack_GetCount(t_DMR_AVP* player, int *count);
int DMR_AudioTrack_GetPidViaIndex(t_DMR_AVP* player, int index, int *pid);
int DMR_AudioTrack_GetLanViaIndex(t_DMR_AVP* player, int index, char**language);
int DMR_AudioTrack_GetCurrentPID(t_DMR_AVP* player, int *pid);
int DMR_AudioTrack_SetCurrentPID(t_DMR_AVP* player, int pid);
#endif

#endif /* _X_AV_PLAYER_H_ */
/*---------------------------------------------------------*/


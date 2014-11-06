

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
	
	int (*SetUri)(t_GM_PALYER *player, char *url, t_MEDIA_INFO *mInfo);/* ���š� uri: ָ���ĵ�Ӱ�����֣�minfo: ��ý��������Ϣ�����δ֪��Ϊ�� */
	int (*Play)(t_GM_PALYER *player, int speed);/* ���š� uri: ָ���ĵ�Ӱ�����֣�minfo: ��ý��������Ϣ�����δ֪��Ϊ�� */
						  /*����ֵ: ��uriΪ�ջ���������򷵻�-1����������·���0 */

	int (*SeekPlay)(t_GM_PALYER *player, int speed);
	int (*Stop)(t_GM_PALYER *player );	 		/* ֹͣ��ǰ����,������������PLAYER_STATE_IDLE״̬ */
	int (*Pause)(t_GM_PALYER *player );			/* �ɲ��ű�Ϊ��ͣ, ������������PLAYER_STATE_PAUSE״̬ */
	int (*Resume)(t_GM_PALYER *player );	 		/* ����ͣ����������ˡ����Ż����˻ָ�Ϊ��������, ������������PLAYER_STATE_PLAY״̬ */
	int (*FastForward)(t_GM_PALYER *player, int speed);	/* ��� */
	int (*FastBackward)(t_GM_PALYER *player, int speed);	/* ���� */
	int (*SlowForward)(t_GM_PALYER *player, int speed);	/* ���� */
	int (*SlowBackward)(t_GM_PALYER *player, int speed);	/* ���ˣ� ��δʵ�� */
	/* ����4���β�speed������һ��ִ�У�ʵ�ʵ��ٶ�ͨ��getPlaySpeed��ȡ���������ý���-1���١�-2���١�-4���١�...��-64����(����)��1���١�-1���١�...ѭ�� */
	int (*Seek)(t_GM_PALYER *player, int method, long long offset);	/* ��ת���š� ��method=0ʱ��offsetΪlong long int *������offsetΪstruct timeval *��*/

	int (*GetPlaySpeed)(t_GM_PALYER *player, char *speed, int size);	/* ��õ�ǰģʽ�¶�Ӧ�Ĳ����ٶȡ�*/
	int (*GetTotalLength)(t_GM_PALYER *player, long long *total);	/* ��ȡý�����ݵ��ܳ��ȣ����ֽ�Ϊ��λ��*/
	int (*GetCurrentLength)(t_GM_PALYER *player, long long *now);/* ��ȡ��ǰ�Ĳ��ų��ȣ����ֽ�Ϊ��λ��*/
	int (*GetTotalTime)(t_GM_PALYER *player, int *total);	/* ��ȡý�����ݵ��ܳ��ȣ��Ժ���Ϊ��λ��*/
	int (*GetCurrentTime)(t_GM_PALYER *player, int *now);	/* ��ȡ��ǰ�Ĳ��ų��ȣ��Ժ���Ϊ��λ��*/

	PLAYER_STATE (*GetState)(t_GM_PALYER *player);			/* ��ȡ��������ǰ��״̬ */
	int (*GetMediaInfo)(t_GM_PALYER *player, t_MEDIA_INFO* minfo);	/* ��ȡ���ڲ�����������ϸ��Ϣ��������յ�MPCB_START_TO_PLAYBACK�Ժ���� */

	/* added by platform, not standard from dlna */
	int (*Subtitle_GetTotalCount)(t_GM_PALYER *player, int *count);			/* ��ȡý�����ݵ��ܳ��ȣ��Ժ���Ϊ��λ��*/
	int (*Subtitle_GetPidViaIndex)(t_GM_PALYER *player, int index, int *pid);	/* ��ȡý�����ݵ��ܳ��ȣ��Ժ���Ϊ��λ��*/
	int (*Subtitle_GetLanViaIndex)(t_GM_PALYER* player, int index, int *lang_code, char*language, int language_len);	/* ��ȡý�����ݵ��ܳ��ȣ��Ժ���Ϊ��λ��*/
	int (*Subtitle_GetCurrentPID)(t_GM_PALYER *player, int *pid);		/* ��ȡý�����ݵ��ܳ��ȣ��Ժ���Ϊ��λ��*/
	int (*Subtitle_SetCurrentPID)(t_GM_PALYER *player, int pid);	/* ��ȡý�����ݵ��ܳ��ȣ��Ժ���Ϊ��λ��*/

	int (*AudioTrack_GetTotalCount)(t_GM_PALYER *player, int *count);			/* ��ȡý�����ݵ��ܳ��ȣ��Ժ���Ϊ��λ��*/
	int (*AudioTrack_GetPidViaIndex)(t_GM_PALYER *player, int index, int *pid);/* ��ȡý�����ݵ��ܳ��ȣ��Ժ���Ϊ��λ��*/
	int (*AudioTrack_GetLanViaIndex)(t_GM_PALYER* player, int index, int *lang_code, char*language, int language_len);/* ��ȡý�����ݵ��ܳ��ȣ��Ժ���Ϊ��λ��*/
	int (*AudioTrack_GetCurrentPID)(t_GM_PALYER *player, int *pid);	/* ��ȡý�����ݵ��ܳ��ȣ��Ժ���Ϊ��λ��*/
	int (*AudioTrack_SetCurrentPID)(t_GM_PALYER *player, int pid);	/* ��ȡý�����ݵ��ܳ��ȣ��Ժ���Ϊ��λ��*/

	void (*Release)(t_GM_PALYER *player);					/* �رղ��������ͷ�������Դ*/
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


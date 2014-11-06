

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <search.h>
#include <unistd.h>

#include "dlna_api.h"
#include "json.h"
#include "HySDK.h"
#include "hitTime.h"
//#include "dmscp.h"

#include "x_avplayer.h"
#include "x_renderingcontrol.h"
#include "direct_huawei.h"
#include "js_eventqueue.h"
#include "coo.h"


#if 0
0:STOP,停止状态
1:PAUSE,暂停状态
2:NORMAL_PLAY,正常播放
3:TRICK_MODE,正常播放之外
4:BUFFERING,本地文件播放缓冲中
#endif

typedef enum {
	eAVPLAYER_STATE_Stopped = 0,
	eAVPLAYER_STATE_Pausing,
	eAVPLAYER_STATE_Playing,
	eAVPLAYER_STATE_Trick,
	eAVPLAYER_STATE_Buffering,
} eAVPLAYER_STATE;

#define JSE_DLNA_FUNC_ENTER		{ }							
#define JSE_DLNA_RETURN_EX(x)	return 0

static t_GM_PALYER				*pPlayer = NULL;
static t_RND_CTRL				*pRCS = NULL;
static pClassEventQueue 		s_huawei_event_queue= NULL;
static int                      s_speed = 0;

#define HUAWEI_OBJECT_HND	(void*)1
#define HUAWEI_PLAYER_ID	1

static struct json_object* s_JseDlna_json_object_new_string(const char *str)
{
	return json_object_new_string( str? str : "");
}

static void s_SendAtJson_Event_PlayMode_Change(int id, int old_mde, int old_speed, int new_mode, int new_speed)
{
	json_object *json_event_info = json_object_new_object();
	json_object_object_add(json_event_info, "type", json_object_new_string("EVENT_PLAYMODE_CHANGE"));
	json_object_object_add(json_event_info, "instance_id", json_object_new_int(id));
	json_object_object_add(json_event_info, "new_play_mode", json_object_new_int(new_mode));
	json_object_object_add(json_event_info, "new_play_rate", json_object_new_int(new_speed));
	json_object_object_add(json_event_info, "old_play_mode", json_object_new_int(old_mde));
	json_object_object_add(json_event_info, "old_play_rate", json_object_new_int(old_speed));
	const char *str = json_object_to_json_string(json_event_info);
	if( s_huawei_event_queue )
		s_huawei_event_queue->Put(s_huawei_event_queue, HUAWEI_OBJECT_HND, (char*)str);
	json_object_put(json_event_info);
}

static void s_SendAtJson_Event_Media_Error(int eventType)
{
	json_object *json_event_info;
	const char *str;
	
	json_event_info = json_object_new_object();
	json_object_object_add(json_event_info, "type", json_object_new_string("EVENT_MEDIA_ERROR"));
	json_object_object_add(json_event_info, "instance_id", json_object_new_int(HUAWEI_PLAYER_ID));
	json_object_object_add(json_event_info, "error_code", json_object_new_int(eventType));
	str = json_object_to_json_string(json_event_info);
	if( s_huawei_event_queue )
		s_huawei_event_queue->Put(s_huawei_event_queue, HUAWEI_OBJECT_HND, (char*)str);
	json_object_put(json_event_info);
}
static int s_JseDlna_huawei_Callback(int eventType, int value, int arg, void *user)
{
	json_object *json_event_info;
	const char *str;
	
	HT_DBG_FUNC_START(HT_MOD_DMR, HT_BIT_FEW, eventType, "eventType = ");
	HT_DBG_FUNC(value, "value = ");
	if( eventType == 0 )
	{
		switch(value)
		{
			case PLAYER_STATE_PLAY:
				json_event_info = json_object_new_object();
				json_object_object_add(json_event_info, "type", json_object_new_string("EVENT_MEDIA_BEGINING"));
				json_object_object_add(json_event_info, "instance_id", json_object_new_int(HUAWEI_PLAYER_ID));
				str = json_object_to_json_string(json_event_info);
				if( s_huawei_event_queue )
					s_huawei_event_queue->Put(s_huawei_event_queue, HUAWEI_OBJECT_HND, (char*)str);
				json_object_put(json_event_info);
				
                s_speed = 1;
				s_SendAtJson_Event_PlayMode_Change(HUAWEI_PLAYER_ID, eAVPLAYER_STATE_Stopped, 0, eAVPLAYER_STATE_Playing, 1);
				break;

			case PLAYER_STATE_IDLE:
                s_speed = 0;
				s_SendAtJson_Event_PlayMode_Change(HUAWEI_PLAYER_ID, eAVPLAYER_STATE_Playing, 1, eAVPLAYER_STATE_Stopped, 0);
				break;
				
			default:
				break;
		}
	}
	else 
	{
		switch(eventType)
		{
			case enum_MPLAYER_EVENT_TOEND:
				json_event_info = json_object_new_object();
				json_object_object_add(json_event_info, "type", json_object_new_string("EVENT_MEDIA_END"));
				json_object_object_add(json_event_info, "instance_id", json_object_new_int(HUAWEI_PLAYER_ID));
				str = json_object_to_json_string(json_event_info);
				if( s_huawei_event_queue )
					s_huawei_event_queue->Put(s_huawei_event_queue, HUAWEI_OBJECT_HND, (char*)str);
				json_object_put(json_event_info);
				break;
				
			case enum_MPLAYER_EVENT_DISCONNECT:
				s_SendAtJson_Event_Media_Error(104015);
				break;
				
			case enum_MPLAYER_EVENT_NO_DATA:
				s_SendAtJson_Event_Media_Error(104023);
				break;
				
			case enum_MPLAYER_EVENT_TRICKPLAY_UNSUPPORTED:
				s_SendAtJson_Event_Media_Error(104022);
				break;
				
			case enum_MPLAYER_EVENT_VIDEO_UNSUPPORTED:
			case enum_MPLAYER_EVENT_AUDIO_UNSUPPORTED:
			case enum_MPLAYER_EVENT_BOTH_UNSUPPORTED:
				s_SendAtJson_Event_Media_Error(104017);
				break;
                
            case enum_MPLAYER_EVENT_OPEN_FAILS:
                s_SendAtJson_Event_Media_Error(104019);
                break;

			case enum_MPLAYER_EVENT_TRICKTOHEAD:
				s_SendAtJson_Event_PlayMode_Change(HUAWEI_PLAYER_ID, eAVPLAYER_STATE_Trick, s_speed, eAVPLAYER_STATE_Playing, 1);
                s_speed = 1;
                break;
                
			default:
				break;
		}
	}
	HT_DBG_FUNC_END(arg, "arg = ");

	return 0;
}

static void s_JseDlna_huawei_init(void)
{
	if( pPlayer == NULL )
	{
		s_huawei_event_queue = EventQueue_Create();
		pRCS = DMR_RenderingCS_Create();
		pPlayer = DMR_AVPlayer_Create(s_JseDlna_huawei_Callback, HUAWEI_OBJECT_HND );
	}
}

static int s_JseDlna_PlayerInstance_Create( const char* aFieldName, const char* aFieldParam, char* aFieldValue, int aResult )
{
	s_JseDlna_huawei_init();
	return (int)(HUAWEI_OBJECT_HND);
//	return (int)pPlayer;
}
static int s_JseDlna_PlayerInstance_Release( const char* aFieldName, const char* aFieldParam, char* aFieldValue, int aResult )
{
	pPlayer->Release(pPlayer);
	pRCS->Release(pRCS);
	EventQueue_Release(s_huawei_event_queue);
	
	pPlayer  = NULL;
	pRCS = NULL;
	s_huawei_event_queue = NULL;
	return 0;
}

static int s_JseDlna_PlayerInstance_Switch( const char* aFieldName, const char* aFieldParam, char* aFieldValue, int aResult )
{
	return 0;
}

static int s_JseDlna_MuteState_Get( const char* aFieldName, const char* aFieldParam, char* aFieldValue, int aResult )
{
	JSE_DLNA_FUNC_ENTER
	int mute = pRCS->GetMute(pRCS, enum_CHANNEL_TYPE_MASTER);
	sprintf(aFieldValue, "%d", mute);
	JSE_DLNA_RETURN_EX(0);
}
static int s_JseDlna_MuteState_Set( const char* aFieldName, const char* aFieldParam, char* aFieldValue, int aResult )
{
	JSE_DLNA_FUNC_ENTER
	//	AVTransport *avt = (AVTransport *)g_DMRPlayer[g_CurrentPlayerIndex].pointer;
	int mute = atoi(aFieldValue);
	pRCS->SetMute(pRCS, enum_CHANNEL_TYPE_MASTER, mute);
	JSE_DLNA_RETURN_EX(0);
}
static int s_JseDlna_Volume_Get( const char* aFieldName, const char* aFieldParam, char* aFieldValue, int aResult )
{
	JSE_DLNA_FUNC_ENTER
	//	AVTransport *avt = (AVTransport *)g_DMRPlayer[g_CurrentPlayerIndex].pointer;
	int volume = pRCS->GetVolume(pRCS, enum_CHANNEL_TYPE_MASTER);
	sprintf(aFieldValue, "%d", volume);
	JSE_DLNA_RETURN_EX(0);
}
static int s_JseDlna_Volume_Set( const char* aFieldName, const char* aFieldParam, char* aFieldValue, int aResult )
{
	JSE_DLNA_FUNC_ENTER
	//	AVTransport *avt = (AVTransport *)g_DMRPlayer[g_CurrentPlayerIndex].pointer;
	int volume = atoi(aFieldValue);
	pRCS->SetVolume(pRCS, enum_CHANNEL_TYPE_MASTER, volume);
	JSE_DLNA_RETURN_EX(0);
}

typedef enum __YX_AUDIO_CHANNEL_MODE
{
	YX_AUDIO_CHANNEL_OUTPUT_STEREO = 0,
	YX_AUDIO_CHANNEL_OUTPUT_LEFT,
	YX_AUDIO_CHANNEL_OUTPUT_RIGHT,
	YX_AUDIO_CHANNEL_OUTPUT_SWAP

}YX_AUDIO_CHANNEL_MODE;
static int s_JseDlna_Audio_Channel_Set( const char* aFieldName, const char* aFieldParam, char* aFieldValue, int aResult )
{
	JSE_DLNA_FUNC_ENTER
	
	int channel = pRCS->GetAudioChannel(pRCS);
	switch( channel )
	{
		case YX_AUDIO_CHANNEL_OUTPUT_STEREO:
			channel = YX_AUDIO_CHANNEL_OUTPUT_LEFT;
			break;
			
		case YX_AUDIO_CHANNEL_OUTPUT_LEFT:
			channel = YX_AUDIO_CHANNEL_OUTPUT_RIGHT;
			break;

		case YX_AUDIO_CHANNEL_OUTPUT_RIGHT:
			channel = YX_AUDIO_CHANNEL_OUTPUT_SWAP;
			break;

		default:
			channel = YX_AUDIO_CHANNEL_OUTPUT_STEREO;
			break;
	}
	pRCS->SetAudioChannel(pRCS, channel);
	
	JSE_DLNA_RETURN_EX(0);
}
static int s_JseDlna_Audio_Channel_Get( const char* aFieldName, const char* aFieldParam, char* aFieldValue, int aResult )
{
	JSE_DLNA_FUNC_ENTER

	const char *str = NULL;
	int channel = pRCS->GetAudioChannel(pRCS);
	switch( channel )
	{
		case YX_AUDIO_CHANNEL_OUTPUT_STEREO:
			str = "stereo";
			break;
			
		case YX_AUDIO_CHANNEL_OUTPUT_LEFT:
			str = "left";
			break;

		case YX_AUDIO_CHANNEL_OUTPUT_RIGHT:
			str = "right";
			break;

		default:
			str = "mix";
			break;
	}

	strcpy(aFieldValue, str);
	JSE_DLNA_RETURN_EX(0);
}

static void avt_GetAllSubtitleInfo(char *buf, int len)
{
	int i, count;
	int ret, pid;
	char str[64];
	int lang_code;

	count = 0;
	ret = pPlayer->Subtitle_GetTotalCount(pPlayer, &count);
	if(count == 0)
	{
		strcpy(buf, "{\"subtitle_list_count\":\"0\"}");
		return;
	}

	json_object *subtitleInfoArray = json_object_new_array();
	for(i = 0; i < count; i++)
	{
		json_object *subtitleInfo = json_object_new_object();
		
		pid = 0;
		ret = pPlayer->Subtitle_GetPidViaIndex(pPlayer, i, &pid);
		json_object_object_add(subtitleInfo, "PID", json_object_new_int(pid));

		lang_code = 0;
		str[0] = 0;
		ret = pPlayer->Subtitle_GetLanViaIndex(pPlayer, i, &lang_code, str, sizeof(str));
		json_object_object_add(subtitleInfo, "language_code", s_JseDlna_json_object_new_string((char*)(&lang_code)));
		json_object_object_add(subtitleInfo, "language_eng_name", s_JseDlna_json_object_new_string(str));
		
		json_object_array_add(subtitleInfoArray, subtitleInfo);
	}
	
	json_object *subtitle = json_object_new_object();
	json_object_object_add(subtitle, "subtitle_list_count", json_object_new_int(count));
	json_object_object_add(subtitle, "subtitle_list", subtitleInfoArray);

	if( strlen(json_object_to_json_string(subtitle)) < len )
		strcpy(buf, json_object_to_json_string(subtitle));
	json_object_put(subtitle);
}
static void avt_GetCurrentSubtitleInfo(char *buf, int len)
{
	int i, count;
	int ret, now, pid;
	char str[64];
	int lang_code;

	now = 0;
	ret = pPlayer->Subtitle_GetCurrentPID(pPlayer, &now);
	count = 0;
	ret = pPlayer->Subtitle_GetTotalCount(pPlayer, &count);
	
	for(i = 0; i < count; i++)
	{
		pid = 0;
		ret = pPlayer->Subtitle_GetPidViaIndex(pPlayer, i, &pid);
		if( (ret == 0) && (pid == now) )
		{
			json_object *subtitleInfo = json_object_new_object();
			
			json_object_object_add(subtitleInfo, "PID", json_object_new_int(now));
			
			lang_code = 0;
			str[0] = 0;
			ret = pPlayer->Subtitle_GetLanViaIndex(pPlayer, i, &lang_code, str, sizeof(str));
			json_object_object_add(subtitleInfo, "language_code", s_JseDlna_json_object_new_string((char*)(&lang_code)));
			json_object_object_add(subtitleInfo, "language_eng_name", s_JseDlna_json_object_new_string(str));
			
			if( strlen(json_object_to_json_string(subtitleInfo)) < len )
				strcpy(buf, json_object_to_json_string(subtitleInfo));
			json_object_put(subtitleInfo);
			return;
		}
	}
	
	strcpy(buf, "{\"PID\":\"null\"}");
}
static int s_JseDlna_AllSubtitleInfo( const char* aFieldName, const char* aFieldParam, char* aFieldValue, int aResult )
{
	JSE_DLNA_FUNC_ENTER
	//	AVTransport *avt = (AVTransport *)g_DMRPlayer[g_CurrentPlayerIndex].pointer;
	avt_GetAllSubtitleInfo(aFieldValue, aResult);
	JSE_DLNA_RETURN_EX(0);
}
static int s_JseDlna_CurrentSubtitleInfo( const char* aFieldName, const char* aFieldParam, char* aFieldValue, int aResult )
{
	JSE_DLNA_FUNC_ENTER
	//	AVTransport *avt = (AVTransport *)g_DMRPlayer[g_CurrentPlayerIndex].pointer;
	avt_GetCurrentSubtitleInfo(aFieldValue, aResult);
	JSE_DLNA_RETURN_EX(0);
}

static void avt_GetAllAudioTrackInfo(char *buf, int len)
{
	int i, count;
	int ret, pid;
	char str[64];
	int lang_code;

	count = 0;
	ret = pPlayer->AudioTrack_GetTotalCount(pPlayer, &count);

	if( count == 0)
	{
		strcpy(buf, "{\"audio_list_count\":\"0\"}");
		return;
	}

	json_object *audioTrackInfoArray = json_object_new_array();
	for(i = 0; i < count; i++)
	{
		json_object *audioTrackInfo = json_object_new_object();

		pid = 0;
		ret = pPlayer->AudioTrack_GetPidViaIndex(pPlayer, i, &pid);
		json_object_object_add(audioTrackInfo, "PID", json_object_new_int(pid));

		lang_code = 0;
		str[0] = 0;
		ret = pPlayer->AudioTrack_GetLanViaIndex(pPlayer, i, &lang_code, str, sizeof(str));
		json_object_object_add(audioTrackInfo, "language_code", s_JseDlna_json_object_new_string((char*)(&lang_code)));
		json_object_object_add(audioTrackInfo, "language_eng_name", s_JseDlna_json_object_new_string(str));

		json_object_array_add(audioTrackInfoArray, audioTrackInfo);
	}
	json_object *audioTrack = json_object_new_object();
	json_object_object_add(audioTrack, "audio_track_list_count", json_object_new_int(count));
	json_object_object_add(audioTrack, "audio_track_list", audioTrackInfoArray);
	
	if( strlen(json_object_to_json_string(audioTrack)) < len )
		strcpy(buf, json_object_to_json_string(audioTrack));
	
	json_object_put(audioTrack);
}
static void avt_GetCurrentAudioTrackInfo(char *buf, int len)
{
	int i, count;
	int ret, now, pid;
	char str[64];
	int lang_code;

	now = 0;
	ret = pPlayer->AudioTrack_GetCurrentPID(pPlayer, &now);
	count = 0;
	ret = pPlayer->AudioTrack_GetTotalCount(pPlayer, &count);
	
	for(i = 0; i < count; i++)
	{
		pid = 0;
		ret = pPlayer->AudioTrack_GetPidViaIndex(pPlayer, i, &pid);
		if( (ret == 0) && (pid == now) )
		{
			json_object *audioTrackInfo = json_object_new_object();
			
			json_object_object_add(audioTrackInfo, "PID", json_object_new_int(now));
			
			lang_code = 0;
			str[0] = 0;
			ret = pPlayer->AudioTrack_GetLanViaIndex(pPlayer, i, &lang_code, str, sizeof(str));
			json_object_object_add(audioTrackInfo, "language_code", s_JseDlna_json_object_new_string((char*)(&lang_code)));
			json_object_object_add(audioTrackInfo, "language_eng_name", s_JseDlna_json_object_new_string(str));

			if( strlen(json_object_to_json_string(audioTrackInfo)) < len )
				strcpy(buf, json_object_to_json_string(audioTrackInfo));
			json_object_put(audioTrackInfo);
			return;
		}
	}
	strcpy(buf, "{\"PID\":\"null\"}");
}
static int s_JseDlna_AllAudioTrackInfo( const char* aFieldName, const char* aFieldParam, char* aFieldValue, int aResult )
{
	JSE_DLNA_FUNC_ENTER
	//	AVTransport *avt = (AVTransport *)g_DMRPlayer[g_CurrentPlayerIndex].pointer;
	avt_GetAllAudioTrackInfo( aFieldValue, aResult);
	JSE_DLNA_RETURN_EX(0);
}
static int s_JseDlna_CurrentAudioTrackInfo( const char* aFieldName, const char* aFieldParam, char* aFieldValue, int aResult )
{
	JSE_DLNA_FUNC_ENTER
	//	AVTransport *avt = (AVTransport *)g_DMRPlayer[g_CurrentPlayerIndex].pointer;
	avt_GetCurrentAudioTrackInfo( aFieldValue, aResult);
	JSE_DLNA_RETURN_EX(0);
}

static int s_JseDlna_HW_Op_Stb( const char* aFieldName, const char* aFieldParam, char* aFieldValue, int aResult )
{
	JSE_DLNA_FUNC_ENTER
	JSE_DLNA_RETURN_EX(0);
}
static int s_JseDlna_HW_Op_Subtitle_Select( const char* aFieldName, const char* aFieldParam, char* aFieldValue, int aResult )
{
	JSE_DLNA_FUNC_ENTER
	//AVTransport *avt = (AVTransport *)g_DMRPlayer[g_CurrentPlayerIndex].pointer;
	int pid = atoi(aFieldValue);
	pPlayer->Subtitle_SetCurrentPID(pPlayer, pid);
	JSE_DLNA_RETURN_EX(0);
}
static int s_JseDlna_HW_Op_Audiotrack_Select( const char* aFieldName, const char* aFieldParam, char* aFieldValue, int aResult )
{
	JSE_DLNA_FUNC_ENTER
	//AVTransport *avt = (AVTransport *)g_DMRPlayer[g_CurrentPlayerIndex].pointer;
	int pid = atoi(aFieldValue);
	pPlayer->AudioTrack_SetCurrentPID(pPlayer, pid);
	JSE_DLNA_RETURN_EX(0);
}
static int s_JseDlna_SubtitleEnableFlag( const char* aFieldName, const char* aFieldParam, char* aFieldValue, int aResult )
{
	JSE_DLNA_FUNC_ENTER
	JSE_DLNA_RETURN_EX(0);
}
static int s_JseDlna_Local_Play_setPostion( const char* aFieldName, const char* aFieldParam, char* aFieldValue, int aResult )
{
	JSE_DLNA_FUNC_ENTER

	HT_DBG_FUNC_START(HT_MOD_DMR, HT_BIT_FEW, 0, 0);
	HT_DBG_FUNC(0, aFieldValue);

	if( g_dlna_callback )
		g_dlna_callback(enum_DlnaCallback_SetPosition, 0, aFieldValue, 0);
	JSE_DLNA_RETURN_EX(0);
}


extern int Dmc_Huawei_GetBestUri( char *udn, char *id, char **uri);
extern int Dmc_Huawei_GetBestMediaInfo( char *udn, char *id, t_MEDIA_INFO *minfo);
/*
采用JSON的方式，传入内容的ID、位置信息，支持列表方式。
单个播放示例：{"itemID":"1-1-4","entryID":"1"}
*/
static int s_JseDlna_Local_Play( const char* aFieldName, const char* aFieldParam, char* aFieldValue, int aResult )
{
	int ret = -1;
	char *uri=NULL;
	t_MEDIA_INFO minfo={0};
	
	HT_DBG_FUNC_START(HT_MOD_DMR, HT_BIT_KEY, 0, 0);

	s_speed = 0;
	json_object *json_obj = dlna_json_tokener_parse(aFieldValue);
	//解析参数
    if(!json_obj)
    {
		ret=-2;
		goto ERR_EXIT;
    }
    
	char *itemID = (char*)json_object_get_string(json_object_object_get(json_obj, "itemID"));
	//解析参数得到ITEMID
	ret = Dmc_Huawei_GetBestUri(NULL, itemID, &uri);
	if(uri)
		ret = Dmc_Huawei_GetBestMediaInfo(NULL, itemID, &minfo);
	//得到码流信息
	json_object_put(json_obj);
	if( !uri )
	{
		ret=-3;
		goto ERR_EXIT;
	}
	
    HT_DBG_FUNC(minfo.majorType, uri);
	switch(minfo.majorType)
	{
		case 1:
		case 2:
			//pPlayer->Stop(pPlayer);
			//同过pplayer播放
			/*me->SetUri			= DMR_AVPlayer_SetUri;
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
	
	me->player						= HySDK_AVPlayer_Create(s_DMR_AVPlayer_Callback, me);*/
			pPlayer->SetUri(pPlayer, uri, &minfo);
			pPlayer->Play(pPlayer, 1);
			ret = 0;
			break;
			
		default:
			break;
	}
	
ERR_EXIT:
	
	if(ret)
		s_JseDlna_huawei_Callback(enum_MPLAYER_EVENT_OPEN_FAILS, 0, 0, HUAWEI_OBJECT_HND);
	
	if(uri) free(uri);
	
    HT_DBG_FUNC_END(ret, 0);
	return ret;
}

#define FILL_RETURN_VALUE( ret)	sprintf( aFieldValue, "%d", ret)

static int s_JseDlna_Local_Stop( const char* aFieldName, const char* aFieldParam, char* aFieldValue, int aResult )
{
	int ret = pPlayer->Stop(pPlayer);
    s_speed = 0;
	usleep(300*1000);	/* temporarily avoid latest event of stop after starting to play*/
	FILL_RETURN_VALUE(  ret );
	return ret;
}
static int s_JseDlna_Local_Pause( const char* aFieldName, const char* aFieldParam, char* aFieldValue, int aResult )
{
	int ret = pPlayer->Pause(pPlayer);
	s_SendAtJson_Event_PlayMode_Change(HUAWEI_PLAYER_ID, eAVPLAYER_STATE_Playing, 1, eAVPLAYER_STATE_Pausing, 0);
	FILL_RETURN_VALUE(  ret );
	return ret;
}
static int s_JseDlna_Local_Resume( const char* aFieldName, const char* aFieldParam, char* aFieldValue, int aResult )
{
	int ret = pPlayer->Resume(pPlayer);
	s_SendAtJson_Event_PlayMode_Change(HUAWEI_PLAYER_ID, eAVPLAYER_STATE_Pausing, 0, eAVPLAYER_STATE_Playing, 1);
	FILL_RETURN_VALUE(  ret );
	return ret;
}
static int s_JseDlna_Local_Seek( const char* aFieldName, const char* aFieldParam, char* aFieldValue, int aResult )
{
	int ret = -1;
	if(strstr(aFieldParam, "next") || strstr(aFieldParam, "previous"))
	{
		//todo playlist
	}
	else
	{
		long long target = atoll(aFieldValue);
		ret = pPlayer->Seek(pPlayer, 0, target);
	}
	FILL_RETURN_VALUE(  ret );
	return ret;
}
static int s_JseDlna_Duration_Get( const char* aFieldName, const char* aFieldParam, char* aFieldValue, int aResult )
{
	int ret, total = 0;;
	ret = pPlayer->GetTotalTime(pPlayer, &total);
	sprintf(aFieldValue, "%d", total);
	return ret;
}
static int s_JseDlna_Postion_Get( const char* aFieldName, const char* aFieldParam, char* aFieldValue, int aResult )
{
	int ret, now = 0;;
	ret = pPlayer->GetCurrentTime(pPlayer, &now);
	sprintf(aFieldValue, "%d", now);
	return ret;
}

/*
 传入倍速，可取2/4/8/16/32/-2/-4/-8/-16/-32或”normal”中的一种,示例：{“speed”:"2”}
这个调用在先调用Local_Play之后才有效。在快进过程中，EPG可以传入normal，表示恢复正常播放。
*/
static int s_JseDlna_setTrickPlayModel( const char* aFieldName, const char* aFieldParam, char* aFieldValue, int aResult )
{
    int ret;
	HT_DBG_FUNC_START(HT_MOD_DMR, HT_BIT_FEW, 0, 0);
    
    json_object *json_obj = dlna_json_tokener_parse(aFieldValue);
    if(!json_obj)
    {
        ret = -2;
        goto ERR_EXIT;
    }

    ret=0;
    char *speed = (char*)json_object_get_string(json_object_object_get(json_obj, "speed"));
    if(!strcmp(speed, "normal"))
    {
        pPlayer->Resume(pPlayer);
        s_SendAtJson_Event_PlayMode_Change(HUAWEI_PLAYER_ID, eAVPLAYER_STATE_Trick, s_speed, eAVPLAYER_STATE_Playing, 1);
        s_speed = 1;
    }
    else
    {
        int spd=atoi(speed);
        if(spd>1)
            pPlayer->FastForward(pPlayer, spd);
        else if(spd<-1)
            pPlayer->FastBackward(pPlayer, -spd);
        else
            ret = -1;
        
        if(!ret)
        {
            s_SendAtJson_Event_PlayMode_Change(HUAWEI_PLAYER_ID, eAVPLAYER_STATE_Playing, s_speed, eAVPLAYER_STATE_Trick, spd);
            s_speed = spd;
        }
    }
    json_object_put(json_obj);

    
ERR_EXIT:
    HT_DBG_FUNC_END(ret, 0);
    return ret;
}
/*
返回播放状态，可是以2/4/8/16/32/-2/-4/-8/-16/-32或”normal”中的一种。其中normal表示正常播放
示例：{“speed”:"normal”}
*/
static int s_JseDlna_getTrickPlayModel( const char* aFieldName, const char* aFieldParam, char* aFieldValue, int aResult )
{
	const char *str;
    char spd[128];
	json_object *json = json_object_new_object();
    if(!json)
        return -1;

    if(s_speed==1)
        sprintf(spd, "%s", "normal");
    else
        sprintf(spd, "%d", s_speed);
    
	json_object_object_add(json, "speed", s_JseDlna_json_object_new_string(spd));
    str = json_object_to_json_string(json);
	sprintf(aFieldValue, "%s", (char*)str);
    json_object_put(json);
    return 0;
}



static int s_Huawei_ParseJs( const char* aFieldName, const char* aFieldParam, char* aFieldValue, int aResult )
{
	int ret = 0;
	char *func = (char*)aFieldName;

	char *flag = "dlna.";
	int len = strlen(flag);
	if( strncmp(func, flag, len) == 0 )
	{
		func += len;
		if(      strcmp(func, "AllSubtitleInfo") == 0 )
			ret = s_JseDlna_AllSubtitleInfo(aFieldName, aFieldParam, aFieldValue, aResult);
		
		else if( strcmp(func, "AllAudioTrackInfo") == 0 )
			ret = s_JseDlna_AllAudioTrackInfo(aFieldName, aFieldParam, aFieldValue, aResult);
		
		else if( strcmp(func, "CurrentSubtitleInfo") == 0 )
			ret = s_JseDlna_CurrentSubtitleInfo(aFieldName, aFieldParam, aFieldValue, aResult);
		
		else if( strcmp(func, "CurrentAudioTrackInfo") == 0 )
			ret = s_JseDlna_CurrentAudioTrackInfo(aFieldName, aFieldParam, aFieldValue, aResult);
		
		else if( strcmp(func, "hw_op_stb") == 0 )
			ret = s_JseDlna_HW_Op_Stb(aFieldName, aFieldParam, aFieldValue, aResult);
		
		else if( strcmp(func, "hw_op_subtitle_select") == 0 )
			ret = s_JseDlna_HW_Op_Subtitle_Select(aFieldName, aFieldParam, aFieldValue, aResult);
		
		else if( strcmp(func, "hw_op_audiotrack_select") == 0 )
			ret = s_JseDlna_HW_Op_Audiotrack_Select(aFieldName, aFieldParam, aFieldValue, aResult);

		else if( strcmp(func, "SubtitleEnableFlag") == 0 )
			ret = s_JseDlna_SubtitleEnableFlag(aFieldName, aFieldParam, aFieldValue, aResult);

		else
			ret = -1;

		return ret;
	}

	flag = "Local_";
	len = strlen(flag);
	if( strncmp(func, flag, len) == 0 )
	{
		func += len;
		if(      strcmp(func, "Play_setPostion") == 0 )
			ret = s_JseDlna_Local_Play_setPostion(aFieldName, aFieldParam, aFieldValue, aResult);
		
		else if( strcmp(func, "Play") == 0 )
			ret = s_JseDlna_Local_Play(aFieldName, aFieldParam, aFieldValue, aResult);
		
		else if( strcmp(func, "Stop") == 0 )
			ret = s_JseDlna_Local_Stop(aFieldName, aFieldParam, aFieldValue, aResult);
		
		else if( strcmp(func, "Pause") == 0 )
			ret = s_JseDlna_Local_Pause(aFieldName, aFieldParam, aFieldValue, aResult);
		
		else if( strcmp(func, "Resume") == 0 )
			ret = s_JseDlna_Local_Resume(aFieldName, aFieldParam, aFieldValue, aResult);
		
		else if( strcmp(func, "Seek") == 0 )
			ret = s_JseDlna_Local_Seek(aFieldName, aFieldParam, aFieldValue, aResult);
		
		else
			ret = -1;

		return ret;
	}

	flag = "PlayerInstance_";
	len = strlen(flag);
	if( strncmp(func, flag, len) == 0 )
	{
		func += len;
		if(      strcmp(func, "Creat") == 0 )
			ret = s_JseDlna_PlayerInstance_Create(aFieldName, aFieldParam, aFieldValue, aResult);
		
		else if( strcmp(func, "Release") == 0 )
			ret = s_JseDlna_PlayerInstance_Release(aFieldName, aFieldParam, aFieldValue, aResult);
		
		else if( strcmp(func, "Switch") == 0 )
			ret = s_JseDlna_PlayerInstance_Switch(aFieldName, aFieldParam, aFieldValue, aResult);
		
		else
			ret = -1;

		return ret;
	}


	if( 	 strcmp(func, "MuteState_Get") == 0 )
		ret = s_JseDlna_MuteState_Get(aFieldName, aFieldParam, aFieldValue, aResult);
	
	else if( strcmp(func, "MuteState_Set") == 0 )
		ret = s_JseDlna_MuteState_Set(aFieldName, aFieldParam, aFieldValue, aResult);
	
	else if( strcmp(func, "Volume_Get") == 0 )
		ret = s_JseDlna_Volume_Get(aFieldName, aFieldParam, aFieldValue, aResult);
	
	else if( strcmp(func, "Volume_Set") == 0 )
		ret = s_JseDlna_Volume_Set(aFieldName, aFieldParam, aFieldValue, aResult);
	
	else if( strcmp(func, "Audio_Channel_Set") == 0 )
		ret = s_JseDlna_Audio_Channel_Set(aFieldName, aFieldParam, aFieldValue, aResult);
	
	else if( strcmp(func, "Audio_Channel_Get") == 0 )
		ret = s_JseDlna_Audio_Channel_Get(aFieldName, aFieldParam, aFieldValue, aResult);
	
	else if( strcmp(func, "Duration_Get") == 0 )
		ret = s_JseDlna_Duration_Get(aFieldName, aFieldParam, aFieldValue, aResult);
	
	else if( strcmp(func, "Postion_Get") == 0 )
		ret = s_JseDlna_Postion_Get(aFieldName, aFieldParam, aFieldValue, aResult);

	else if( strcmp(func, "getTrickPlayModel") == 0 )
		ret = s_JseDlna_getTrickPlayModel(aFieldName, aFieldParam, aFieldValue, aResult);
	
	else if( strcmp(func, "setTrickPlayModel") == 0 )
		ret = s_JseDlna_setTrickPlayModel(aFieldName, aFieldParam, aFieldValue, aResult);
    
	else
		ret = -1;
	
	return -1;
}

int Raw_Dmp_HuaweiJse( const char* aFieldName, const char* aFieldParam, char* aFieldValue, int aResult, int readonly )
{
	HT_DBG_FUNC_START(HT_MOD_DMR, HT_BIT_KEY, aResult, aFieldName);
	HT_DBG_FUNC(0, aFieldParam);
	HT_DBG_FUNC(0, aFieldValue);
	
	s_JseDlna_huawei_init();
	int	ret = s_Huawei_ParseJs(aFieldName, aFieldParam, aFieldValue, aResult);
	
	HT_DBG_FUNC_END(ret, aFieldValue);
	return ret;
}




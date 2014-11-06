#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include "json.h"

#include "dlna_api.h"
#include "direct_yuxing.h"
#include "hitTime.h"
#include "ixml.h"

extern int GetCurrentTransportActions( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString );
extern char *UpnpUtil_GetFirstDocumentItem( IN IXML_Document * doc,IN const char *item );
extern int GetMediaInfo( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString );
extern int GetPositionInfo( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString );
extern int GetTransportInfo( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString );

//extern void YX_CD_ParseMetadataByUri(char *metadata, char *uri, char **protocolInfo, t_MEDIA_INFO* minfo );
extern void YX_CD_ParseProtocolInfo(char *protocolInfo, t_MEDIA_INFO* minfo );

static int avt_SetAVTransportURI(AVTransport *avt, char *uri)
{
	if( avt->metadata )
	{
		free(avt->metadata);
		avt->metadata = NULL;
	}

	if( avt->url )
	{
		free(avt->url);
		avt->url = NULL;
	}

	if( uri )
		avt->url = strdup(uri);
	
	return 0;
}

static int avt_SetAVTransportURIMetaData(AVTransport *avt, char *metadata)
{
	if( avt->metadata )
	{
		free(avt->metadata);
		avt->metadata = NULL;
	}
	
	if( metadata )
		avt->metadata = strdup(metadata);	
	return 0;
}

static int avt_Play(AVTransport *avt, int speed)
{
	int ret = 0;
	
	if(speed == 1)
	{
		YX_CD_ParseProtocolInfo(avt->metadata, &(avt->minfo));
		avt->player->SetUri(avt->player, avt->url, &(avt->minfo));
		ret = avt->player->Play(avt->player, 1);
	}
	else if( speed > 0 )
		avt->player->FastForward(avt->player, speed);
	else if( speed < 0 )
		avt->player->FastBackward(avt->player, speed);
	return ret;
}

static int avt_SeekPlay(AVTransport *avt, unsigned long position)
{
	int ret = 0;
	YX_CD_ParseProtocolInfo(avt->metadata, &(avt->minfo));
	avt->player->SetUri(avt->player, avt->url, &(avt->minfo));
	ret = avt->player->SeekPlay(avt->player, position);
	return ret;
}
static int avt_Pause(AVTransport *avt)
{
	avt->player->Pause(avt->player);
	return 0;
}

static int avt_Resume(AVTransport *avt)
{
	avt->player->Resume(avt->player);
	return 0;
}

static int avt_Seek(AVTransport *avt, int mode, unsigned long long target)
{
	avt->player->Seek(avt->player, mode, target);
	return 0;
}
static int avt_Stop(AVTransport *avt)
{
	avt->player->Stop(avt->player);
	return 0;
}

#if 1
static long long avt_GetTotalLength(AVTransport *avt)
{
	long long total = 0;
	avt->player->GetTotalLength(avt->player,&total);
	return total;
}
static long long avt_GetCurrentLength(AVTransport *avt)
{
	long long current = 0;
	avt->player->GetCurrentLength(avt->player,&current);
	return current;
}

static long int avt_GetTotalTime(AVTransport *avt)
{
	int total = 0;
	avt->player->GetTotalTime(avt->player,&total);
	return total;
}
static long int avt_GetCurrentTime(AVTransport *avt)
{
	int current = 0;
	avt->player->GetCurrentTime(avt->player,&current);
	return current;
}

static struct json_object* s_DmrYx_json_object_new_string(const char *str)
{
	return json_object_new_string( str? str : "");
}

static char *avt_GetCurrentTransportActions(AVTransport *avt)
{
	IXML_Document *in = NULL, *out = NULL;
	char *errorString = NULL;
	
	GetCurrentTransportActions(in, &out, &errorString);
	char *action = UpnpUtil_GetFirstDocumentItem(out, "Actions");
	if(in)
		ixmlDocument_free(in);
	if(out)
		ixmlDocument_free(out);
	return action;
}
static char *avt_GetMediaInfo(AVTransport *avt)
{
	IXML_Document *in = NULL, *out = NULL;
	char *errorString = NULL, *value = NULL;
	char *result;
	GetMediaInfo(in, &out, &errorString);
	
	json_object *output = json_object_new_object();
	
	value = UpnpUtil_GetFirstDocumentItem(out, "CurrentMediaDuration");
	json_object_object_add(output, "CurrentMediaDuration", s_DmrYx_json_object_new_string(value));
	if(value)
		free(value);

	value = UpnpUtil_GetFirstDocumentItem(out, "AVTransportURI");
	json_object_object_add(output, "AVTransportURI", s_DmrYx_json_object_new_string(value));
	if(value)
		free(value);

	value = UpnpUtil_GetFirstDocumentItem(out, "AVTransportURIMetaData");
	json_object_object_add(output, "AVTransportURIMetaData", s_DmrYx_json_object_new_string(value));
	if(value)
		free(value);

	result = (char *)json_object_to_json_string(output);//bug!!!!
	json_object_put(output);
	if(in)
		ixmlDocument_free(in);
	if(out)
		ixmlDocument_free(out);
	return result;
}
static char *avt_GetTransportInfo(AVTransport *avt)	
{
	IXML_Document *in = NULL, *out = NULL;
	char *errorString = NULL, *value = NULL;
	
	GetTransportInfo(in, &out, &errorString);
	json_object *output = json_object_new_object();

	value = UpnpUtil_GetFirstDocumentItem(out, "CurrentTransportStatus");
	json_object_object_add(output, "CurrentTransportStatus", s_DmrYx_json_object_new_string(value));
	if(value)
		free(value);

	value = UpnpUtil_GetFirstDocumentItem(out, "CurrentSpeed");
	json_object_object_add(output, "CurrentSpeed", s_DmrYx_json_object_new_string(value));
	if(value)
		free(value);

	char *result = (char *)json_object_to_json_string(output);//bug!!!!
	json_object_put(output);
	if(in)
		ixmlDocument_free(in);
	if(out)
		ixmlDocument_free(out);
	return result;
}
static char *avt_GetPositionInfo(AVTransport *avt)
{
	IXML_Document *in = NULL, *out = NULL;
	char *errorString = NULL, *value = NULL;
	
	GetPositionInfo(in, &out, &errorString);
	json_object *output = json_object_new_object();

	value = UpnpUtil_GetFirstDocumentItem(out, "AbsTime");
	json_object_object_add(output, "AbsTime", s_DmrYx_json_object_new_string(value)); 
	if(value)
		free(value);

	value = UpnpUtil_GetFirstDocumentItem(out, "AbsCount");
	json_object_object_add(output, "AbsCount", s_DmrYx_json_object_new_string(value));  
	if(value)
		free(value);

	char *result = (char *)json_object_to_json_string(output);//bug!!!!
	json_object_put(output);
	if(in)
		ixmlDocument_free(in);
	if(out)
		ixmlDocument_free(out);
	return result;
}
#endif
static void avt_Release(AVTransport* avt)
{
	if(avt)
	{
		avt->player->Release(avt->player);
		EventQueue_Release(avt->queue);
		free(avt);
	}
}

static int avt_DispatchFunc(int hnd, char* funcName, char *param, char *value, int len)
{
	char *readStr = NULL;
	AVTransport *avt = (AVTransport *)hnd;
	
	HT_DBG_FUNC_START(HT_MOD_DMR, HT_BIT_MANY, (int)hnd, funcName);
	if(!strcmp(funcName, "SetAVTransportURI") && avt->SetAVTransportURI)
	{
		avt->SetAVTransportURI(avt, value);
	}
	else if(!strcmp(funcName, "SetAVTransportURIMetaData") && avt->SetAVTransportURIMetaData)
	{
		avt->SetAVTransportURIMetaData(avt, value);
	}
	else if(!strcmp(funcName, "Seek") && avt->Seek)
	{
		int seekMode;
		long long seekTarget;
		sscanf(value, "%d\"%lld", &seekMode, &seekTarget);
		avt->Seek(avt, seekMode, seekTarget);
	}
	else if(!strcmp(funcName, "Play") && avt->Play)
	{
		avt->Play(avt, atoi(value));
	}
	else if(!strcmp(funcName, "PlaySeek") && avt->SeekPlay)
	{
		avt->SeekPlay(avt, atoi(value));
	}
	else if(!strcmp(funcName, "Pause") && avt->Pause)
	{
		avt->Pause(avt);
	}
	else if(!strcmp(funcName, "Resume") && avt->Resume)
	{
		avt->Resume(avt);
	}
	else if(!strcmp(funcName, "Stop") && avt->Stop)
	{
		avt->Stop(avt);
	}
	else if(!strcmp(funcName, "GetCurrentTime") && avt->GetCurrentTime)
	{
		sprintf(value, "%ld", avt->GetCurrentTime(avt));
	}
	else if(!strcmp(funcName, "GetTotalTime") && avt->GetTotalTime )
	{
		sprintf(value, "%ld", avt->GetTotalTime(avt));
	}

	else if(!strcmp(funcName, "Release") && avt->Release)
	{
		avt->Release(avt);
	}
	
	else if(!strcmp(funcName, "GetCurrentTransportAction"))
	{
		readStr = avt_GetCurrentTransportActions(avt);
	}
	else if(!strcmp(funcName, "GetMediaInfo"))
	{
		readStr = avt_GetMediaInfo(avt);
	}
	else if(!strcmp(funcName, "GetTransportInfo"))
	{
		readStr = avt_GetTransportInfo(avt);
	}
	else if(!strcmp(funcName, "GetPositionInfo"))
	{
		readStr = avt_GetPositionInfo(avt);
	}
#if 0
	else if(!strcmp(funcName, "GetSubtitleInfo"))
	{
		readStr = avt_GetSubtitleInfo(avt);
	}
	else if(!strcmp(funcName, "GetAudioTrackInfo"))
	{
		readStr = avt_GetAudioTrackInfo(avt);
	}
	else if(!strcmp(funcName, "GetCurrentSubtitlePid"))
	{
		snprintf(value, len, "%u", avt_GetCurrentSubtitlePid(avt));
	}
	else if(!strcmp(funcName, "GetCurrentAudioTrackPid"))
	{
		snprintf(value, len, "%u", avt_GetCurrentAudioTrackPid(avt));
	}
	
#endif	
	else
	{
		HT_DBG_FUNC(0, "!!!!!!!!! Not Support The Func\n");
	}
	
	if(readStr)
		strncpy(value, readStr, len);
	
	HT_DBG_FUNC_END(0,value);
	return 0;
}

static int avt_event_handler(int eType, int value, int arg, void *user)
{
	int ret = -1;
	AVTransport *avt = (AVTransport *)user;

	if( !avt )
		return ret;
	
	pClassEventQueue queue = avt->queue;
	json_object *json = json_object_new_object();

	switch(eType)
	{
		case 0:
			json_object_object_add(json, "type", json_object_new_string("EVENT_PLAYMODE_CHANGE"));
			json_object_object_add(json, "instance_id", json_object_new_int((int)avt));
			json_object_object_add(json, "new_play_mode", json_object_new_int(value));
			json_object_object_add(json, "new_play_rate", json_object_new_int(arg));
			break;
			
		default:
			json_object_object_add(json, "type", json_object_new_string("EVENT_PLAYMODE_CHANGE"));
			json_object_object_add(json, "instance_id", json_object_new_int((int)avt));
			json_object_object_add(json, "new_play_mode", json_object_new_int(value));
			json_object_object_add(json, "new_play_rate", json_object_new_int(arg));
			break;
	}
	
	char *output = (char *)json_object_to_json_string(json);
	ret = queue->Put(queue, avt, output);
	json_object_put(json);
	return ret;
}

int new_AVTransport(t_PLAYER_EVENT callback, void *user)
{
	AVTransport *avt = (AVTransport *)malloc(sizeof(AVTransport));
	memset(avt, 0, sizeof(AVTransport));
	
	avt->DispatchFunc 	= avt_DispatchFunc;

	avt->SetAVTransportURI 			= avt_SetAVTransportURI;
	avt->SetAVTransportURIMetaData	= avt_SetAVTransportURIMetaData;
	
	avt->Play 			= avt_Play;
	avt->Pause 			= avt_Pause;
	avt->Resume 		= avt_Resume;
	avt->Seek 			= avt_Seek;
	avt->Stop 			= avt_Stop;
	avt->SeekPlay             = avt_SeekPlay; 
#if 1	
	avt->GetTotalTime 	= avt_GetTotalTime;
	avt->GetCurrentTime = avt_GetCurrentTime;
	avt->GetTotalLength = avt_GetTotalLength;
	avt->GetCurrentLength = avt_GetCurrentLength;
#endif	
	avt->queue			= EventQueue_Create();
	
	avt->Release 		= avt_Release;
	avt->player 		= DMR_AVPlayer_Create(avt_event_handler, avt);
	return (int)avt;
}

static int imagePlayer_SetUri(ImagePlayer *IP, char *uri)
{
	if( IP->metadata )
	{
		free(IP->metadata);
		IP->metadata = NULL;
	}

	if( IP->url )
	{
		free(IP->url);
		IP->url = NULL;
	}

	if( uri )
		IP->url = strdup(uri);
	
	return 0;
}

static int imagePlayer_SetMetadata(ImagePlayer *IP, char *metadata)
{
	if( IP->metadata )
	{
		free(IP->metadata);
		IP->metadata = NULL;
	}
	
	if( metadata )
		IP->metadata = strdup(metadata);	
	return 0;
}

static char *imagePlayer_GetMediaInfo(ImagePlayer *IP)	//bug!!!!!
{
    #if 0
	t_MEDIA_INFO* minfo = IP->player->GetMediaInfo(IP->player);

	json_object *output = json_object_new_object();
	json_object_object_add(output, "w", json_object_new_int(minfo->resWidth));
	json_object_object_add(output, "h", json_object_new_int(minfo->resHeight));
	char *result = (char *)json_object_to_json_string(output);
	json_object_put(output);
	return result;
    #endif
    return NULL;
}

static int imagePlayer_Play(ImagePlayer *IP, int slideMode)
{
	YX_CD_ParseProtocolInfo(IP->metadata, &(IP->minfo));
	IP->player->SetUri(IP->player, IP->url, &(IP->minfo));
	return IP->player->Play(IP->player, 1);
}

static int imagePlayer_Zoom(ImagePlayer *IP, int percent)
{
	return IP->player->Zoom(IP->player, percent);
}

static int imagePlayer_Move(ImagePlayer *IP, int x, int y)
{
	return IP->player->Move(IP->player, x, y);
}
static int imagePlayer_Rotate (ImagePlayer *IP, int radian)
{
	return IP->player->Rotate(IP->player, radian);
}

static int imagePlayer_Stop(ImagePlayer *IP)
{
	return IP->player->Stop(IP->player);
}
static int imagePlayer_Release(ImagePlayer *IP)
{
	if( IP )
	{
		IP->player->Release(IP->player);
		EventQueue_Release(IP->queue);
		free(IP);
	}
	return 0;
}

static int IPDispatchFunc(int hnd, char* funcName, char *param, char *value, int len)
{
	ImagePlayer *me = (ImagePlayer *)hnd;
	
	HT_DBG_FUNC_START(HT_MOD_DMR, HT_BIT_MANY, (int)hnd, funcName);

	if(!strcmp(funcName, "SetTransportURI"))
	{
		me->SetUri(me, value);
	}
	else if(!strcmp(funcName, "SetTransportURIMetaData"))
	{
		me->SetMetadata(me, value);
	}
	else if(!strcmp(funcName, "Play"))
	{
		me->Play(me, atoi(value));
	}
	else if(!strcmp(funcName, "GetImageInfo"))
	{
		char *str = me->GetMediaInfo(me);
		sprintf(value, "%s", str);
	}
	else if(!strcmp(funcName, "Stop"))
	{
		me->Stop(me);
	}
	else if(!strcmp(funcName, "Zoom"))
	{
		me->Zoom(me, atoi(value));
	}
	else if(!strcmp(funcName, "Move"))
	{
		int x, y;
		sscanf(value, "%d,%d", &x, &y);
		me->Move(me, x, y);
	}
	else if(!strcmp(funcName, "Rotate"))
	{
		me->Rotate(me, atoi(value));
	}
	else if(!strcmp(funcName, "Release"))
	{
		me->Release(me);
	}
	else
	{
	}

	HT_DBG_FUNC_END(0,value);
	return 0;
}

static int imagePlayer_event_handler(int eType, int value, int arg, void *user)
{
	int ret = -1;

	if( !user )
		return ret;
	
	ImagePlayer *me = (ImagePlayer *)user;
	pClassEventQueue queue = me->queue;
	json_object *json = json_object_new_object();

	switch(eType)
	{
		case 0:
			break;
		default:
			break;
	}
	
	char *output = (char *)json_object_to_json_string(json);
	ret = queue->Put(queue, me, output);
	json_object_put(json);
	return ret;
}

int new_ImagePlayer(t_PLAYER_EVENT callback, void *user)
{
	ImagePlayer *me = (ImagePlayer *)malloc(sizeof(ImagePlayer));
	memset(me, 0, sizeof(ImagePlayer));
	
	me->DispatchFunc = IPDispatchFunc;
	
	
	me->SetUri 		= imagePlayer_SetUri;
	me->SetMetadata	= imagePlayer_SetMetadata;
	me->Play 		= imagePlayer_Play;
	me->Zoom 		= imagePlayer_Zoom;
	me->Move 		= imagePlayer_Move;
	me->Rotate 		= imagePlayer_Rotate;
	me->Stop 		= imagePlayer_Stop;
	me->Release 	= imagePlayer_Release;
	me->GetMediaInfo= imagePlayer_GetMediaInfo;

	me->queue		= EventQueue_Create();
	
	me->player 		= DMR_IPlayer_Create(imagePlayer_event_handler, me);
	return (int)me;
}





/* rendering control */ 
typedef unsigned short	UI2;

#define STATIC_CHAR		static char
#define STATIC_INT		static int
#define STATIC_SHORT	static UI2

#define P_RND_CTRL(p)	(p->RndCtrl)


STATIC_CHAR *s_DirectYx_ListPresets(t_DYX_RC* hnd)
{
	return NULL;
}
STATIC_INT s_DirectYx_SelectPreset(t_DYX_RC* hnd, char *PresetName)
{
	return 0;
}

STATIC_SHORT s_DirectYx_GetBrightness(t_DYX_RC* hnd)
{	
	HT_DBG_FUNC_START(HT_MOD_DMR, HT_BIT_FEW, (int)hnd, 0);

	UI2 ret = (hnd->RndCtrl)->GetBrightness(P_RND_CTRL(hnd));
	
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}
STATIC_INT s_DirectYx_SetBrightness(t_DYX_RC* hnd, unsigned short DesiredBrightness)
{
	HT_DBG_FUNC_START(HT_MOD_DMR, HT_BIT_FEW, DesiredBrightness, 0);
	int ret = (hnd->RndCtrl)->SetBrightness(P_RND_CTRL(hnd), DesiredBrightness);
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}

STATIC_SHORT s_DirectYx_GetContrast(t_DYX_RC* hnd)
{
	HT_DBG_FUNC_START(HT_MOD_DMR, HT_BIT_FEW, (int)hnd, 0);
	
	UI2 ret = (hnd->RndCtrl)->GetContrast(P_RND_CTRL(hnd));
	
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}
STATIC_INT s_DirectYx_SetContrast(t_DYX_RC* hnd, unsigned short DesiredContrast)
{
	HT_DBG_FUNC_START(HT_MOD_DMR, HT_BIT_FEW, DesiredContrast, 0);
	int ret = (hnd->RndCtrl)->SetContrast(P_RND_CTRL(hnd), DesiredContrast);
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}

STATIC_SHORT s_DirectYx_GetSharpness(t_DYX_RC* hnd)
{
	HT_DBG_FUNC_START(HT_MOD_DMR, HT_BIT_FEW, (int)hnd, 0);
	
	UI2 ret = (hnd->RndCtrl)->GetSharpness(P_RND_CTRL(hnd));
	
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}
STATIC_INT s_DirectYx_SetSharpness(t_DYX_RC* hnd, unsigned short DesiredSharpness)
{
	HT_DBG_FUNC_START(HT_MOD_DMR, HT_BIT_FEW, DesiredSharpness, 0);
	int ret = (hnd->RndCtrl)->SetSharpness(P_RND_CTRL(hnd), DesiredSharpness);
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}

STATIC_SHORT s_DirectYx_GetRedVideoGain(t_DYX_RC* hnd)
{
	HT_DBG_FUNC_START(HT_MOD_DMR, HT_BIT_FEW, (int)hnd, 0);
	
	UI2 ret = (hnd->RndCtrl)->GetRedVideoGain(P_RND_CTRL(hnd));
	
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}
STATIC_INT s_DirectYx_SetRedVideoGain(t_DYX_RC* hnd, unsigned short DesiredRedVideoGain)
{
	HT_DBG_FUNC_START(HT_MOD_DMR, HT_BIT_FEW, DesiredRedVideoGain, 0);
	int ret = (hnd->RndCtrl)->SetRedVideoGain(P_RND_CTRL(hnd), DesiredRedVideoGain);
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}

STATIC_SHORT s_DirectYx_GetGreenVideoGain(t_DYX_RC* hnd)
{
	HT_DBG_FUNC_START(HT_MOD_DMR, HT_BIT_FEW, (int)hnd, 0);
	
	UI2 ret = (hnd->RndCtrl)->GetGreenVideoGain(P_RND_CTRL(hnd));
	
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}
STATIC_INT s_DirectYx_SetGreenVideoGain(t_DYX_RC* hnd, unsigned short DesiredGreenVideoGain)
{
	HT_DBG_FUNC_START(HT_MOD_DMR, HT_BIT_FEW, DesiredGreenVideoGain, 0);
	int ret = (hnd->RndCtrl)->SetGreenVideoGain(P_RND_CTRL(hnd), DesiredGreenVideoGain);
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}

STATIC_SHORT s_DirectYx_GetBlueVideoGain(t_DYX_RC* hnd)
{
	HT_DBG_FUNC_START(HT_MOD_DMR, HT_BIT_FEW, (int)hnd, 0);
	
	UI2 ret = (hnd->RndCtrl)->GetBlueVideoGain(P_RND_CTRL(hnd));
	
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}
STATIC_INT s_DirectYx_SetBlueVideoGain(t_DYX_RC* hnd, unsigned short DesiredBlueVideoGain)
{
	HT_DBG_FUNC_START(HT_MOD_DMR, HT_BIT_FEW, DesiredBlueVideoGain, 0);
	int ret = (hnd->RndCtrl)->SetBlueVideoGain(P_RND_CTRL(hnd), DesiredBlueVideoGain);
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}

STATIC_SHORT s_DirectYx_GetRedVideoBlackLevel(t_DYX_RC* hnd)
{
	HT_DBG_FUNC_START(HT_MOD_DMR, HT_BIT_FEW, (int)hnd, 0);
	
	UI2 ret = (hnd->RndCtrl)->GetRedVideoBlackLevel(P_RND_CTRL(hnd));
	
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}
STATIC_INT s_DirectYx_SetRedVideoBlackLevel(t_DYX_RC* hnd, unsigned short DesiredRedVideoBlackLevel)
{
	HT_DBG_FUNC_START(HT_MOD_DMR, HT_BIT_FEW, DesiredRedVideoBlackLevel, 0);
	int ret = (hnd->RndCtrl)->SetRedVideoBlackLevel(P_RND_CTRL(hnd), DesiredRedVideoBlackLevel);
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}

STATIC_SHORT s_DirectYx_GetGreenVideoBlackLevel(t_DYX_RC* hnd)
{
	HT_DBG_FUNC_START(HT_MOD_DMR, HT_BIT_FEW, (int)hnd, 0);
	
	UI2 ret = (hnd->RndCtrl)->GetGreenVideoBlackLevel(P_RND_CTRL(hnd));
	
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}
STATIC_INT s_DirectYx_SetGreenVideoBlackLevel(t_DYX_RC* hnd, unsigned short DesiredGreenVideoBlackLevel)
{
	HT_DBG_FUNC_START(HT_MOD_DMR, HT_BIT_FEW, DesiredGreenVideoBlackLevel, 0);
	int ret = (hnd->RndCtrl)->SetGreenVideoBlackLevel(P_RND_CTRL(hnd), DesiredGreenVideoBlackLevel);
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}

STATIC_SHORT s_DirectYx_GetBlueVideoBlackLevel(t_DYX_RC* hnd)
{
	HT_DBG_FUNC_START(HT_MOD_DMR, HT_BIT_FEW, (int)hnd, 0);
	
	UI2 ret = (hnd->RndCtrl)->GetBlueVideoBlackLevel(P_RND_CTRL(hnd));
	
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}
STATIC_INT s_DirectYx_SetBlueVideoBlackLevel(t_DYX_RC* hnd, unsigned short DesiredBlueVideoBlackLevel)
{
	HT_DBG_FUNC_START(HT_MOD_DMR, HT_BIT_FEW, DesiredBlueVideoBlackLevel, 0);
	int ret = (hnd->RndCtrl)->SetBlueVideoBlackLevel(P_RND_CTRL(hnd), DesiredBlueVideoBlackLevel);
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}

STATIC_SHORT s_DirectYx_GetColorTemperature(t_DYX_RC* hnd)
{
	HT_DBG_FUNC_START(HT_MOD_DMR, HT_BIT_FEW, (int)hnd, 0);
	
	UI2 ret = (hnd->RndCtrl)->GetColorTemperature(P_RND_CTRL(hnd));
	
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}
STATIC_INT s_DirectYx_SetColorTemperature(t_DYX_RC* hnd, unsigned short DesiredColorTemperature)
{
	HT_DBG_FUNC_START(HT_MOD_DMR, HT_BIT_FEW, DesiredColorTemperature, 0);
	int ret = (hnd->RndCtrl)->SetColorTemperature(P_RND_CTRL(hnd), DesiredColorTemperature);
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}

STATIC_SHORT s_DirectYx_GetHorizontalKeystone(t_DYX_RC* hnd)
{
	HT_DBG_FUNC_START(HT_MOD_DMR, HT_BIT_FEW, (int)hnd, 0);
	
	UI2 ret = (hnd->RndCtrl)->GetHorizontalKeystone(P_RND_CTRL(hnd));
	
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}
STATIC_INT s_DirectYx_SetHorizontalKeystone(t_DYX_RC* hnd, short DesiredHorizontalKeystone)
{
	HT_DBG_FUNC_START(HT_MOD_DMR, HT_BIT_FEW, DesiredHorizontalKeystone, 0);
	int ret = (hnd->RndCtrl)->SetHorizontalKeystone(P_RND_CTRL(hnd), DesiredHorizontalKeystone);
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}

STATIC_SHORT s_DirectYx_GetVerticalKeystone(t_DYX_RC* hnd)
{
	HT_DBG_FUNC_START(HT_MOD_DMR, HT_BIT_FEW, (int)hnd, 0);
	
	UI2 ret = (hnd->RndCtrl)->GetVerticalKeystone(P_RND_CTRL(hnd));
	
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}
STATIC_INT s_DirectYx_SetVerticalKeystone(t_DYX_RC* hnd, short DesiredVerticalKeystone)
{
	HT_DBG_FUNC_START(HT_MOD_DMR, HT_BIT_FEW, DesiredVerticalKeystone, 0);
	int ret = (hnd->RndCtrl)->SetVerticalKeystone(P_RND_CTRL(hnd), DesiredVerticalKeystone);
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}

static enum_CHANNEL_TYPE s_convert_channel(char* ch)
{
	enum_CHANNEL_TYPE Channel = 0;
	char *str = ch;

	if( 	!strcmp(str, "Master") )
		Channel = enum_CHANNEL_TYPE_MASTER;
	else if(!strcmp(str, "LF") )
		Channel = enum_CHANNEL_TYPE_LF;
	else if(!strcmp(str, "RF") )
		Channel = enum_CHANNEL_TYPE_RF;
	else if(!strcmp(str, "CF") )
		Channel = enum_CHANNEL_TYPE_CF;
	else if(!strcmp(str, "LFE") )
		Channel = enum_CHANNEL_TYPE_LFE;
	else if(!strcmp(str, "LS") )
		Channel = enum_CHANNEL_TYPE_LS;
	else if(!strcmp(str, "RS") )
		Channel = enum_CHANNEL_TYPE_RS;
	else if(!strcmp(str, "LFC") )
		Channel = enum_CHANNEL_TYPE_LFC;
	else if(!strcmp(str, "RFC") )
		Channel = enum_CHANNEL_TYPE_RFC;
	else if(!strcmp(str, "SD") )
		Channel = enum_CHANNEL_TYPE_SD;
	else if(!strcmp(str, "SL") )
		Channel = enum_CHANNEL_TYPE_SL;
	else if(!strcmp(str, "SR") )
		Channel = enum_CHANNEL_TYPE_SR;
	else if(!strcmp(str, "T") )
		Channel = enum_CHANNEL_TYPE_T;
	else if(!strcmp(str, "B") )
		Channel = enum_CHANNEL_TYPE_B;
	else
		Channel = enum_CHANNEL_TYPE_MASTER;
	
	return Channel;
}

STATIC_SHORT s_DirectYx_GetLoudness(t_DYX_RC* hnd, char* ch)
{
	HT_DBG_FUNC_START(HT_MOD_DMR, HT_BIT_FEW, (int)hnd, 0);

	enum_CHANNEL_TYPE Channel = s_convert_channel(ch);
	UI2 ret = (hnd->RndCtrl)->GetLoudness(P_RND_CTRL(hnd), Channel);
	
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}
STATIC_INT s_DirectYx_SetLoudness(t_DYX_RC* hnd, char* ch, short DesiredLoudness)
{
	HT_DBG_FUNC_START(HT_MOD_DMR, HT_BIT_FEW, DesiredLoudness, 0);
	enum_CHANNEL_TYPE Channel = s_convert_channel(ch);
	int ret = (hnd->RndCtrl)->SetLoudness(P_RND_CTRL(hnd), Channel, DesiredLoudness);
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}

STATIC_SHORT s_DirectYx_GetMute(t_DYX_RC* hnd, char* ch)
{
	HT_DBG_FUNC_START(HT_MOD_DMR, HT_BIT_FEW, (int)hnd, 0);
	
	enum_CHANNEL_TYPE Channel = s_convert_channel(ch);
	UI2 ret = (hnd->RndCtrl)->GetMute(P_RND_CTRL(hnd), Channel);
	
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}
STATIC_INT s_DirectYx_SetMute(t_DYX_RC* hnd, char* ch, int DesiredMute)
{
	HT_DBG_FUNC_START(HT_MOD_DMR, HT_BIT_FEW, DesiredMute, 0);
	enum_CHANNEL_TYPE Channel = s_convert_channel(ch);
	int ret = (hnd->RndCtrl)->SetMute(P_RND_CTRL(hnd), Channel, DesiredMute);
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}

STATIC_SHORT s_DirectYx_GetVolume(t_DYX_RC* hnd, char* ch)
{
	HT_DBG_FUNC_START(HT_MOD_DMR, HT_BIT_FEW, (int)hnd, 0);
	
	enum_CHANNEL_TYPE Channel = s_convert_channel(ch);
	UI2 ret = (hnd->RndCtrl)->GetVolume(P_RND_CTRL(hnd), Channel);
	
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}
STATIC_INT s_DirectYx_SetVolume(t_DYX_RC* hnd, char* ch, unsigned short DesiredVolume)
{
	HT_DBG_FUNC_START(HT_MOD_DMR, HT_BIT_FEW, DesiredVolume, 0);
	enum_CHANNEL_TYPE Channel = s_convert_channel(ch);
	int ret = (hnd->RndCtrl)->SetVolume(P_RND_CTRL(hnd), Channel, DesiredVolume);
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}

STATIC_SHORT s_DirectYx_GetVolumeDB(t_DYX_RC* hnd, char* ch)
{
	HT_DBG_FUNC_START(HT_MOD_DMR, HT_BIT_FEW, (int)hnd, 0);
	
	enum_CHANNEL_TYPE Channel = s_convert_channel(ch);
	UI2 ret = (hnd->RndCtrl)->GetVolumeDB(P_RND_CTRL(hnd), Channel);
	
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}
STATIC_INT s_DirectYx_SetVolumeDB(t_DYX_RC* hnd, char* ch, short DesiredVolume)
{
	HT_DBG_FUNC_START(HT_MOD_DMR, HT_BIT_FEW, DesiredVolume, 0);
	enum_CHANNEL_TYPE Channel = s_convert_channel(ch);
	int ret = (hnd->RndCtrl)->SetVolumeDB(P_RND_CTRL(hnd), Channel, DesiredVolume);
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}

STATIC_SHORT s_DirectYx_GetVolumeDBMin(t_DYX_RC* hnd, char* ch)
{
	HT_DBG_FUNC_START(HT_MOD_DMR, HT_BIT_FEW, (int)hnd, 0);
	
	enum_CHANNEL_TYPE Channel = s_convert_channel(ch);
	UI2 ret = (hnd->RndCtrl)->GetVolumeDBMin(P_RND_CTRL(hnd), Channel);
	
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}
STATIC_SHORT s_DirectYx_GetVolumeDBMax(t_DYX_RC* hnd, char* ch)
{
	HT_DBG_FUNC_START(HT_MOD_DMR, HT_BIT_FEW, (int)hnd, 0);
	
	enum_CHANNEL_TYPE Channel = s_convert_channel(ch);
	UI2 ret = (hnd->RndCtrl)->GetVolumeDBMax(P_RND_CTRL(hnd), Channel);
	
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}

STATIC_INT s_DirectYx_GetAudioChannel(t_DYX_RC* hnd)
{
	HT_DBG_FUNC_START(HT_MOD_DMR, HT_BIT_FEW, (int)hnd, 0);
	
	int ret = (hnd->RndCtrl)->GetAudioChannel(P_RND_CTRL(hnd));
	
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}
STATIC_INT s_DirectYx_SetAudioChannel(t_DYX_RC* hnd, int DesiredChannel)
{
	HT_DBG_FUNC_START(HT_MOD_DMR, HT_BIT_FEW, (int)hnd, 0);
	
	int ret = (hnd->RndCtrl)->SetAudioChannel(P_RND_CTRL(hnd), DesiredChannel);
	
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}

static void s_DirectYx_RenderingCS_Release(t_DYX_RC* hnd)
{
	HT_DBG_FUNC_START(HT_MOD_DMR, HT_BIT_FEW, (int)hnd, 0);
	if( hnd )
	{
		(hnd->RndCtrl)->Release(hnd->RndCtrl);
		free( hnd );
	}
	
	HT_DBG_FUNC_END(0, 0);	
}


static int s_DispatchFunc_Set(int hnd, char* funcName, char *param, char *value, int len)
{
	t_DYX_RC *me = (t_DYX_RC *)hnd;
	int ret = -1;
	unsigned short desired = 0;
	char* Channel = NULL;
	char* comma = NULL;

	HT_DBG_FUNC_START(HT_MOD_DMR, HT_BIT_MANY, (int)hnd, funcName);
	if( value )
	{
		comma = strchr(value, ',');
		if( comma )
		{
			*comma = 0;
			Channel = value;
			desired = (unsigned short)atoi(comma+1);
		}
		else
		{
			desired = (unsigned short)atoi(value);
		}
	}
	
	HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "Channel = %s, desired = %d \n", Channel, desired);
	
	if(!strcmp(funcName, "SetBrightness"))
	{
		ret = me->SetBrightness(me, desired);
	}
	else if(!strcmp(funcName, "SetContrast"))
	{
		ret = me->SetContrast(me, desired);
	}
	else if(!strcmp(funcName, "SetSharpness"))
	{
		ret = me->SetSharpness(me, desired);
	}
	else if(!strcmp(funcName, "SetRedVideoGain"))
	{
		ret = me->SetRedVideoGain(me, desired);
	}
	else if(!strcmp(funcName, "SetGreenVideoGain"))
	{
		ret = me->SetGreenVideoGain(me, desired);
	}
	else if(!strcmp(funcName, "SetBlueVideoGain"))
	{
		ret = me->SetBlueVideoGain(me, desired);
	}
	else if(!strcmp(funcName, "SetRedVideoBlackLevel"))
	{
		ret = me->SetRedVideoBlackLevel(me, desired);
	}
	else if(!strcmp(funcName, "SetGreenVideoBlackLevel"))
	{
		ret = me->SetGreenVideoBlackLevel(me, desired);
	}
	else if(!strcmp(funcName, "SetBlueVideoBlackLevel"))
	{
		ret = me->SetBlueVideoBlackLevel(me, desired);
	}
	else if(!strcmp(funcName, "SetColorTemperature"))
	{
		ret = me->SetColorTemperature(me, desired);
	}
	else if(!strcmp(funcName, "SetHorizontalKeystone"))
	{
		ret = me->SetHorizontalKeystone(me, desired);
	}
	else if(!strcmp(funcName, "SetVerticalKeystone"))
	{
		ret = me->SetVerticalKeystone(me, desired);
	}
	/* param */
	else if(!strcmp(funcName, "SetLoudness"))
	{
		ret = me->SetLoudness(me, Channel, desired);
	}
	else if(!strcmp(funcName, "SetMute"))
	{
		ret = me->SetMute(me, Channel, desired);
	}
	else if(!strcmp(funcName, "SetVolume"))
	{
		ret = me->SetVolume(me, Channel, desired);
	}
	else if(!strcmp(funcName, "SetVolumeDB"))
	{
		ret = me->SetVolumeDB(me, Channel, desired);
	}
	/* added */
	else if(!strcmp(funcName, "SetAudioChannel"))
	{
		ret = me->SetAudioChannel(me, desired);
	}
	else
	{
	}

	sprintf(value, "%d", ret);
	HT_DBG_FUNC_END(ret, value);
	return ret;	
}


static int s_DispatchFunc_Get(int hnd, char* funcName, char *param, char *value, int len)
{
	t_DYX_RC *me = (t_DYX_RC *)hnd;
	unsigned short ret = 0;
	char* Channel = param;
	
	if(!strcmp(funcName, "GetBrightness"))
	{
		ret = me->GetBrightness(me);
	}
	else if(!strcmp(funcName, "GetContrast"))
	{
		ret = me->GetContrast(me);
	}
	else if(!strcmp(funcName, "GetSharpness"))
	{
		ret = me->GetSharpness(me);
	}
	else if(!strcmp(funcName, "GetRedVideoGain"))
	{
		ret = me->GetRedVideoGain(me);
	}
	else if(!strcmp(funcName, "GetGreenVideoGain"))
	{
		ret = me->GetGreenVideoGain(me);
	}
	else if(!strcmp(funcName, "GetBlueVideoGain"))
	{
		ret = me->GetBlueVideoGain(me);
	}
	else if(!strcmp(funcName, "GetRedVideoBlackLevel"))
	{
		ret = me->GetRedVideoBlackLevel(me);
	}
	else if(!strcmp(funcName, "GetGreenVideoBlackLevel"))
	{
		ret = me->GetGreenVideoBlackLevel(me);
	}
	else if(!strcmp(funcName, "GetBlueVideoBlackLevel"))
	{
		ret = me->GetBlueVideoBlackLevel(me);
	}
	else if(!strcmp(funcName, "GetColorTemperature"))
	{
		ret = me->GetColorTemperature(me);
	}
	else if(!strcmp(funcName, "GetHorizontalKeystone"))
	{
		ret = me->GetHorizontalKeystone(me);
	}
	else if(!strcmp(funcName, "GetVerticalKeystone"))
	{
		ret = me->GetVerticalKeystone(me);
	}
	/* param */
	else if(!strcmp(funcName, "GetLoudness"))
	{
		ret = me->GetLoudness(me, Channel);
	}
	else if(!strcmp(funcName, "GetMute"))
	{
		ret = me->GetMute(me, Channel);
	}
	else if(!strcmp(funcName, "GetVolume"))
	{
		ret = me->GetVolume(me, Channel);
	}
	else if(!strcmp(funcName, "GetVolumeDB"))
	{
		ret = me->GetVolumeDB(me, Channel);
	}
	else if(!strcmp(funcName, "GetVolumeDBMin"))
	{
		ret = me->GetVolumeDBMin(me, Channel);
	}
	else if(!strcmp(funcName, "GetVolumeDBMax"))
	{
		ret = me->GetVolumeDBMax(me, Channel);
	}

	/* added */
	else if(!strcmp(funcName, "GetAudioChannel"))
	{
		ret = me->GetAudioChannel(me);
		//ret = (int)Channel;
	}
	else
	{
		return -1;
	}
	
	sprintf(value, "%d", ret);
	return (int)ret;	
}

static int s_DispatchFunc(int hnd, char* funcName, char *param, char *value, int len)
{
	//t_DYX_RC *me = (t_DYX_RC *)hnd;
	int ret = -1;
	
	if(!strncmp(funcName, "Set", 3))
	{
		ret = s_DispatchFunc_Set(hnd, funcName, param, value, len);
	}
	else if(!strncmp(funcName, "Get", 3))
	{
		ret = s_DispatchFunc_Get(hnd, funcName, param, value, len);
	}
	if(!strcmp(funcName, "ListPresets"))
	{
		//ret = me->ListPresets(me, xx, xx);
	}
	else if(!strcmp(funcName, "SelectPreset"))
	{
		//ret = me->ListPresets(me, xx, xx);
	}
	else
	{
	}
	
	return ret;
}

int new_RenderingCS(void)
{
	t_DYX_RC *me = (t_DYX_RC *)malloc(sizeof(t_DYX_RC));
	memset(me, 0, sizeof(t_DYX_RC));

	me->RndCtrl = DMR_RenderingCS_Create();
	
	me->DispatchFunc = s_DispatchFunc;
	
	me -> ListPresets = s_DirectYx_ListPresets;
	me -> SelectPreset = s_DirectYx_SelectPreset;
	me->GetBrightness = s_DirectYx_GetBrightness;
	me->SetBrightness = s_DirectYx_SetBrightness;

	me->GetContrast = s_DirectYx_GetContrast;
	me->SetContrast = s_DirectYx_SetContrast;
	me->GetSharpness = s_DirectYx_GetSharpness;
	me->SetSharpness = s_DirectYx_SetSharpness;
	me->GetRedVideoGain = s_DirectYx_GetRedVideoGain;
	me->SetRedVideoGain = s_DirectYx_SetRedVideoGain;
	me->GetGreenVideoGain = s_DirectYx_GetGreenVideoGain;
	me->SetGreenVideoGain = s_DirectYx_SetGreenVideoGain;
	me->GetBlueVideoGain = s_DirectYx_GetBlueVideoGain;
	me->SetBlueVideoGain = s_DirectYx_SetBlueVideoGain;
	me->GetRedVideoBlackLevel = s_DirectYx_GetRedVideoBlackLevel;
	me->SetRedVideoBlackLevel = s_DirectYx_SetRedVideoBlackLevel;
	me->GetGreenVideoBlackLevel = s_DirectYx_GetGreenVideoBlackLevel;
	me->SetGreenVideoBlackLevel = s_DirectYx_SetGreenVideoBlackLevel;
	me->GetBlueVideoBlackLevel = s_DirectYx_GetBlueVideoBlackLevel;
	me->SetBlueVideoBlackLevel = s_DirectYx_SetBlueVideoBlackLevel;
	me->GetColorTemperature = s_DirectYx_GetColorTemperature;
	me->SetColorTemperature = s_DirectYx_SetColorTemperature;
	
	me->GetHorizontalKeystone = s_DirectYx_GetHorizontalKeystone;
	me->SetHorizontalKeystone = s_DirectYx_SetHorizontalKeystone;
	me->GetVerticalKeystone = s_DirectYx_GetVerticalKeystone;
	me->SetVerticalKeystone = s_DirectYx_SetVerticalKeystone;
	me->GetLoudness = s_DirectYx_GetLoudness;
	me->SetLoudness = s_DirectYx_SetLoudness;
	
	me->SetMute = s_DirectYx_SetMute;
	me->GetMute = s_DirectYx_GetMute;
	me->SetVolume = s_DirectYx_SetVolume;
	me->GetVolume = s_DirectYx_GetVolume;
	me->GetVolumeDB = s_DirectYx_GetVolumeDB;
	me->SetVolumeDB = s_DirectYx_SetVolumeDB;
	me->GetVolumeDBMin = s_DirectYx_GetVolumeDBMin;
	me->GetVolumeDBMax = s_DirectYx_GetVolumeDBMax;

	me->GetAudioChannel = s_DirectYx_GetAudioChannel;
	me->SetAudioChannel = s_DirectYx_SetAudioChannel;

	me->Release = s_DirectYx_RenderingCS_Release;
	return (int)me;
}



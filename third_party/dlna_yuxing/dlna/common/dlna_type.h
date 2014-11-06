#ifndef _DLNA_TYPE_H_
#define _DLNA_TYPE_H_

#include <sys/types.h>	/* off_t */
#include <stddef.h>		/* size_t */
#include <time.h>		/* time_t */

#include "dlna_audio_type.h"

typedef struct _media_information_
{
	int		majorType;			/* 1 = video . 2 = audio . 3 = picture , otherwise invalid*/
	union {
		int		minorType;		/* minor1 type */
		int		pictureType;	/* .bmp .jpg .png .tif .gif ... */
		int		audioType;		/* .pcm .mp3 .mp2 .aac .mpa .wma .dts .ac3 ... */
		int		videoType;		/* .mpg .dat .vob .avi .wmv .rm .asf ... */
	}otherType;

	char			title[256];
	char			uri[1024];	/* unused */
	long long		fileSize;	/* file length */
	unsigned int	xduration;	/* total time at ms */
	char			date[32];	/* original date, really created date */

	int				bitrate;	/* if negative, has not been set */
	int				colorDepth;	/* if negative, has not been set */
	int				resWidth;
	int				resHeight;

	WAVEFORMATEX	audioInfo;	/* audio information */
	void*			reserved;	/* reserved */

	/*private for DLNA */
	int				dlnaCI;
	int				dlnaPS;

	/* private for pvr sharing */
	int				pvr_flag;
	int				pvr_contentSource;
	int				pvr_encrypted;
	char			pvr_keys[64];
} t_MEDIA_INFO;

typedef struct _c_upnp_dms_media_information_
{
	short			majorType;		/* 1 = video . 2 = audio . 3 = picture , 4 = container, otherwise invalid*/
	union {
		short		minorType;		/* minor1 type */
		short		pictureType;	/* .bmp .jpg .png .tif .gif ... */
		short		audioType;		/* .pcm .mp3 .mp2 .aac .mpa .wma .dts .ac3 ... */
		short		videoType;		/* .mpg .dat .vob .avi .wmv .rm .asf ... */
	}otherType;
	/* for audio,especially for lpcm */
//	WAVEFORMATEX	*audio_info;	/* audio information */

	char			*xx_title;
	long long		size;		/* file length */
	unsigned int	duration;	/* total time at ms */
	time_t			date;		/* original date, really created date */

	int				bitrate;	/* if negative, has not been set */
	int				colordepth;	/* if negative, has not been set */
	short			res_width;
	short			res_height;

	/* private for pvr sharing */
	void			*pp_json;
	void			*dlna_profile;
	int				parentIndex;

	char			*dc_description;
	time_t			startTime;
	char			*PvrID;
} C_UD_MI;

typedef int PVR_FS_HND;
typedef struct _c_pvr_fake_fs_t_ {
	int						mode;

	int 					(*GetInfo)(char *filename, t_MEDIA_INFO *minfo);
	PVR_FS_HND				(*Open)(char *filename, int mode);
	int 					(*Read)(PVR_FS_HND fileHnd, char *buf, size_t buflen);
	int 					(*Write)(PVR_FS_HND fileHnd, char *buf, size_t buflen);
	int 					(*Seek)(PVR_FS_HND fileHnd, off_t offset, int origin);
	int 					(*Close)(PVR_FS_HND fileHnd);
}C_PVR_FSOP;

typedef enum{
	PLAYER_STATE_IDLE = 0,				/* 空闲状态，可以调用play播放新的媒体 */
//	有意义操作: play

	PLAYER_STATE_TRANSITION,			/* 状态转换中*/

	PLAYER_STATE_CHECK,			 		/* 正在从服务器获取媒体的信息用于即将进行的播放 */
//	有意义操作: stop

	PLAYER_STATE_PLAY,					/* 正以正常速度播放媒体 */
//	有意义操作: stop、pause、ff、fb、sf、goto

	PLAYER_STATE_PAUSE,					/* 暂停 */
//	有意义操作: stop、resume

	PLAYER_STATE_FF,					/* 正在快进 */
	PLAYER_STATE_FB,					/* 正在快退 */
	PLAYER_STATE_SF,					/* 正在慢进 */
	PLAYER_STATE_SB,					/* 正在慢退 */
//	有意义操作: stop、pause、ff、fb、sf
}PLAYER_STATE;

typedef enum{
	/* the 3 definitions below must be general to any specific player */
	enum_MPLAYER_EVENT_STATE_CHANGED = 0, 	/* state of player has changed */
	enum_MPLAYER_EVENT_GOOD,				/* good information means normally play, more details please call to specific player*/
	enum_MPLAYER_EVENT_BAD, 				/* bad information means error,  more details please call to specific player*/

	/* the definitions below may general to any specific player. Of course the specific player can re-definite by its need */
	enum_MPLAYER_EVENT_OPEN_OK, 			/* ok, open file ok or connect to server */
	enum_MPLAYER_EVENT_BUFFERING, 			/* ok, need and begin to buffer data */
	enum_MPLAYER_EVENT_START,				/* ok, decoder shows first i-frame or output sound*/
	enum_MPLAYER_EVENT_TOEND, 				/* ok, normal payback to the end*/
	enum_MPLAYER_EVENT_TRICKTOEND,			/* ok, trick payback to the end, e.g, FF or SF*/
	enum_MPLAYER_EVENT_TRICKTOHEAD,			/* ok, trick payback to the head, e.g, FB or SB*/

	enum_MPLAYER_EVENT_AUDIO_UNSUPPORTED,	/* error,  unsupported audio */
	enum_MPLAYER_EVENT_VIDEO_UNSUPPORTED,	/* error,  unsupported video */
	enum_MPLAYER_EVENT_BOTH_UNSUPPORTED,	/* error,  unsupported audio & video */

	enum_MPLAYER_EVENT_PAUSE_UNSUPPORTED,	/* error,  pause unsupported*/
	enum_MPLAYER_EVENT_TRICKPLAY_UNSUPPORTED,/* error,  trickplay unsupported*/
	enum_MPLAYER_EVENT_FF_UNSUPPORTED,		/* error,  fast forward unsupported*/
	enum_MPLAYER_EVENT_FB_UNSUPPORTED,		/* error,  rewind unsupported */
	enum_MPLAYER_EVENT_SF_UNSUPPORTED,		/* error,  slow forward unsupported*/
	enum_MPLAYER_EVENT_SB_UNSUPPORTED,		/* error,  slow backward unsupported */

	enum_MPLAYER_EVENT_OPEN_FAILS,			/* error,  file not exsits or can not connect to server*/
	enum_MPLAYER_EVENT_CA_DRM_FAILS, 		/* error,  CA or DRM fails*/
	enum_MPLAYER_EVENT_NO_DATA,				/* error,  no data for a long time and decoder has to halt */
	enum_MPLAYER_EVENT_DISCONNECT,			/* error,  files not exists again or connection disconnected by server */
}enum_MPLAYER_GENERAL_EVENT;


typedef enum
{
    DLNA_EVENT_DMSLIST_UPDATE = 0,      /* DMS列表更新 */
    DLNA_EVENT_FILELIST_UPDATE,         /* 1文件列表更新 */
    DLNA_EVENT_DMR_SETMUTE,             /*2 设置静音 */
    DLNA_EVENT_DMR_SETVOLUME,           /* 3设置音量 */
    DLNA_EVENT_DMR_GETMUTE,             /*4 获取静音状态 */
    DLNA_EVENT_DMR_GETVOLUME,           /*5 获取音量 */
    DLNA_EVENT_DMR_PLAY,                /* 6播放 */
    DLNA_EVENT_DMR_PAUSE,               /*7 暂停 */
    DLNA_EVENT_DMR_RESUME,              /* 8恢复 */
    DLNA_EVENT_DMR_SEEK,                /* 9定位 */
    DLNA_EVENT_DMR_STOP,                /* 10结束 */
    DLNA_EVENT_DMR_GETPOSITIONINFO,     /*11 获取当前位置信息 */
    DLNA_EVENT_DMR_GETTRANSPORTINFO,     /* 12获取Transport信息 dinglei*/
    DLNA_EVENT_DMR_GETMEDIAINFO,     /* 13获取媒体信息 dinglei*/
    DLNA_EVENT_DMR_NONE,                /*14 无效 */
    EVENT_DLNA_CHANNEL_RESOURCE_CHANGED, /*15共享频道播放冲突事件*/
    DLNA_EVENT_DMR_SETTRANSFORMS_ZOOM, /*16设置展现的变换dinglei*/
    DLNA_EVENT_DMR_GETPRODUCTINFO,/*17：得到与待播内容关联的产品列表dinglei*///roductInfo
    DLNA_EVENT_DMR_ORDER /*订购*/


}HY_DLNA_EVENTID;

/*
	Master (Master)
	Left Front (LF)
	Right Front (RF)
	Center Front (CF)
	Low Frequency Enhancement (LFE) [Super woofer]
	Left Surround (LS)
	Right Surround (RS)
	Left of Center (LFC) [in front]
	Right of Center (RFC) [in front]
	Surround (SD) [rear]
	Side Left (SL) [left wall]
	Side Right (SR) [right wall]
	Top (T) [overhead]
	Bottom (B) [bottom]
*/
typedef enum _Channel_Type
{
	enum_CHANNEL_TYPE_MASTER = 0,
	enum_CHANNEL_TYPE_LF,
	enum_CHANNEL_TYPE_RF,
	enum_CHANNEL_TYPE_CF,
	enum_CHANNEL_TYPE_LFE,
	enum_CHANNEL_TYPE_LS,
	enum_CHANNEL_TYPE_RS,
	enum_CHANNEL_TYPE_LFC,
	enum_CHANNEL_TYPE_RFC,
	enum_CHANNEL_TYPE_SD,
	enum_CHANNEL_TYPE_SL,
	enum_CHANNEL_TYPE_SR,
	enum_CHANNEL_TYPE_T,
	enum_CHANNEL_TYPE_B,
}enum_CHANNEL_TYPE;

typedef enum _enum_dlna_ipc_type_
{
	enum_DlnaIPC_NULL = 0,	// means no ipc
	enum_DlnaIPC_InSameProcess,
	enum_DlnaIPC_StartServerByFork,
	enum_DlnaIPC_StartServerByShell,

	enum_DlnaIPC_MAXIMUM,
}enum_DlnaIPC;

typedef enum _enum_dlna_app_mode_
{
	enum_DlnaAppMode_HUAWEI = 0,
	enum_DlnaAppMode_ImitatedObject,
	enum_DlnaAppMode_TrueObject,
	enum_DlnaAppMode_HUAWEI_MAROC,
	enum_DlnaAppMode_HUAWEI_QTEL,
    enum_DlnaAppMode_HUAWEI_V1R5,

	enum_DlnaAppMode_MAXIMUM,
}enum_DlnaAppMode;

typedef enum _enum_dlna_event_
{
	enum_DlnaEvent_JsonString = 0,
	enum_DlnaEvent_OnlyVirtualKey,

	enum_DlnaEvent_MAXIMUM,
}enum_DlnaEvent;

typedef enum _enum_dlna_callback_
{
	enum_DlnaCallback_SetPosition = 0,

	enum_DlnaCallback_MAXIMUM,
}enum_DlnaCallback;


/* for raw player sends event back to dlna modules */
typedef int 	(*t_PLAYER_EVENT)(int playerEventType, int value, int arg, void *user); // playerEventType: 0 means state changed; others means player handle
/* unified interface that dlna module sends event at json up to EPG */
typedef int 	(*t_DLNA_EVENT)(enum_DlnaEvent eType, int handle, char *str);
/* for callback to middleware or iptv module */
typedef int 	(*t_DLNA_CALLBACK)(enum_DlnaCallback eType, int value, char *str, void *user);

/* dmr */
//typedef void (*DMREVRNT_CALLBACK)(int eventID, const char* eventArg, void* arg);

#include "dlna_type_private.h"

#endif /* _DLNA_TYPE_H_ */


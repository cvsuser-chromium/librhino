#ifndef __AVTRANSPORT_H__
#define __AVTRANSPORT_H__

#include "js_eventqueue.h"
#include "x_avplayer.h"
#include "x_imageplayer.h"
#include "x_renderingcontrol.h"

typedef struct _AVTransport AVTransport;

struct _AVTransport
{
	DEFAULT_DISPATCH_FUNC;
	int (*SetAVTransportURI)(AVTransport *avt, char *uri);
	int (*SetAVTransportURIMetaData)(AVTransport *avt, char *metadata);
	int (*Play)(AVTransport *avt, int speed);
	int (*Pause)(AVTransport *avt);
	int (*Resume)(AVTransport *avt);
	int (*Seek)(AVTransport *avt, int mode, unsigned long long Target);
	int (*Stop)(AVTransport *avt);
	int (*SeekPlay)(AVTransport *avt, unsigned long position);
	
	long int (*GetTotalTime)(AVTransport *avt);
	long int (*GetCurrentTime)(AVTransport *avt);
	long long (*GetTotalLength)(AVTransport *avt);
	long long (*GetCurrentLength)(AVTransport *avt);

	/* 16 functions below  not avalaibale now*/
	int (*SetNextAVTransportURI)(AVTransport *avt, char *currentURI, char *protocolInfo);
	int (*GetMediaInfo)(AVTransport *avt, char *currentURI, char *protocolInfo);
	int (*GetMediaInfo_Ext)(AVTransport *avt, char *currentURI, char *protocolInfo);
	int (*GetTransportInfo)(AVTransport *avt, char *currentURI, char *protocolInfo);
	int (*GetPositionInfo)(AVTransport *avt, char *currentURI, char *protocolInfo);
	int (*GetDeviceCapabilities)(AVTransport *avt, char *currentURI, char *protocolInfo);
	int (*GetTransportSettings)(AVTransport *avt, char *currentURI, char *protocolInfo);
	int (*Record)(AVTransport *avt, char *currentURI, char *protocolInfo);
	int (*Next)(AVTransport *avt, char *currentURI, char *protocolInfo);
	int (*Previous)(AVTransport *avt, char *currentURI, char *protocolInfo);
	
	int (*SetPlayMode)(AVTransport *avt, char *currentURI, char *protocolInfo);
	int (*SetRecordQualityMode)(AVTransport *avt, char *currentURI, char *protocolInfo);
	int (*GetCurrentTransportActions)(AVTransport *avt, char *currentURI, char *protocolInfo);
	int (*GetDRMState)(AVTransport *avt, char *currentURI, char *protocolInfo);
	int (*GetStateVariables)(AVTransport *avt, char *currentURI, char *protocolInfo);
	int (*SetStateVariables)(AVTransport *avt, char *currentURI, char *protocolInfo);

	void (*Release)(AVTransport *avt);

	/* private data */
	t_GM_PALYER*		player;
	char*				url;
	char*				metadata;
	t_MEDIA_INFO		minfo;
	pClassEventQueue	queue;
};


typedef struct _ImagePlayer ImagePlayer;
struct _ImagePlayer 
{
	DEFAULT_DISPATCH_FUNC;
	int				(*SetUri)(ImagePlayer *player, char *uri);
	int				(*SetMetadata)(ImagePlayer *player, char *metadata);
	int				(*Play)(ImagePlayer *player, int slideMode);
	int				(*Zoom)(ImagePlayer *player, int percent);
	int				(*Move)(ImagePlayer *player, int x, int y);
	int				(*Rotate)(ImagePlayer *player, int radian);
	int				(*Stop)(ImagePlayer *player);
	char*			(*GetMediaInfo)(ImagePlayer *player);
	int				(*Release)(ImagePlayer *player);
	
	/* private data */
	t_PLAYER_EVENT	callback;
	void*				user;
	t_DMR_IMGP*			player;
	char*				url;
	char*				metadata;
	t_MEDIA_INFO		minfo;
	pClassEventQueue	queue;
};


typedef struct _dirext_yuxing_rendering_control_ t_DYX_RC;
struct _dirext_yuxing_rendering_control_
{
	DEFAULT_DISPATCH_FUNC;
	char *(*ListPresets)(t_DYX_RC* RCS);
	int	(*SelectPreset)(t_DYX_RC* RCS, char *PresetName);
	unsigned short (*GetBrightness)(t_DYX_RC* RCS);
	int	(*SetBrightness)(t_DYX_RC* RCS, unsigned short DesiredBrightness);
	unsigned short (*GetContrast)(t_DYX_RC* RCS);
	int	(*SetContrast)(t_DYX_RC* RCS, unsigned short DesiredContrast);
	unsigned short (*GetSharpness)(t_DYX_RC* RCS);
	int	(*SetSharpness)(t_DYX_RC* RCS, unsigned short DesiredSharpness);
	unsigned short (*GetRedVideoGain)(t_DYX_RC* RCS);
	int	(*SetRedVideoGain)(t_DYX_RC* RCS, unsigned short DesiredRedVideoGain);
	unsigned short (*GetGreenVideoGain)(t_DYX_RC* RCS);
	int	(*SetGreenVideoGain)(t_DYX_RC* RCS, unsigned short DesiredGreenVideoGain);
	unsigned short (*GetBlueVideoGain)(t_DYX_RC* RCS);
	int	(*SetBlueVideoGain)(t_DYX_RC* RCS, unsigned short DesiredBlueVideoGain);
	unsigned short (*GetRedVideoBlackLevel)(t_DYX_RC* RCS);
	int	(*SetRedVideoBlackLevel)(t_DYX_RC* RCS, unsigned short DesiredRedVideoBlackLevel);
	unsigned short (*GetGreenVideoBlackLevel)(t_DYX_RC* RCS);
	int	(*SetGreenVideoBlackLevel)(t_DYX_RC* RCS, unsigned short DesiredGreenVideoBlackLevel);
	unsigned short (*GetBlueVideoBlackLevel)(t_DYX_RC* RCS);
	int	(*SetBlueVideoBlackLevel)(t_DYX_RC* RCS, unsigned short DesiredBlueVideoBlackLevel);
	unsigned short (*GetColorTemperature)(t_DYX_RC* RCS);
	int	(*SetColorTemperature)(t_DYX_RC* RCS, unsigned short DesiredColorTemperature);
	unsigned short (*GetHorizontalKeystone)(t_DYX_RC* RCS);
	int	(*SetHorizontalKeystone)(t_DYX_RC* RCS, short DesiredHorizontalKeystone);
	unsigned short (*GetVerticalKeystone)(t_DYX_RC* RCS);
	int	(*SetVerticalKeystone)(t_DYX_RC* RCS, short DesiredVerticalKeystone);
	unsigned short (*GetLoudness)(t_DYX_RC* RCS, char* channel);
	int	(*SetLoudness)(t_DYX_RC* RCS, char* channel, short DesiredLoudness);
	unsigned short (*GetMute)(t_DYX_RC* RCS, char* channel);
	int	(*SetMute)(t_DYX_RC* RCS, char* channel, int DesiredMute);
	unsigned short (*GetVolume)(t_DYX_RC* RCS, char* channel);
	int	(*SetVolume)(t_DYX_RC* RCS, char* channel, unsigned short DesiredVolume);
	unsigned short (*GetVolumeDB)(t_DYX_RC* RCS, char* channel);
	int	(*SetVolumeDB)(t_DYX_RC* RCS, char* channel, short DesiredVolume);
	unsigned short (*GetVolumeDBMin)(t_DYX_RC* RCS, char* channel);
	unsigned short (*GetVolumeDBMax)(t_DYX_RC* RCS, char* channel);	

	/* added by platform, not standard from dlna */
	int	(*GetAudioChannel)(t_DYX_RC* RCS);
	int	(*SetAudioChannel)(t_DYX_RC* RCS, int DesiredChannel);

	void (*Release)(t_DYX_RC* RCS);
	
	/* private data */
	t_RND_CTRL*			RndCtrl;
};

int new_AVTransport(t_PLAYER_EVENT callback, void *user);
int new_ImagePlayer(t_PLAYER_EVENT callback, void *user);
int new_RenderingCS(void);

#endif


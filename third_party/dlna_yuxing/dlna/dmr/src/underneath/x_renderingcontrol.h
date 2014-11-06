#ifndef __X_RENDERING_CONTROL_H__
#define __X_RENDERING_CONTROL_H__

#include "dlna_type.h"

typedef struct _rendering_control_ t_RND_CTRL;
struct _rendering_control_
{
	t_PLAYER_EVENT	callback;
	int					rcHandle;

	char *(*ListPresets)(t_RND_CTRL* RCS);
	int	(*SelectPreset)(t_RND_CTRL* RCS, char *PresetName);
	unsigned short (*GetBrightness)(t_RND_CTRL* RCS);
	int	(*SetBrightness)(t_RND_CTRL* RCS, unsigned short DesiredBrightness);
	unsigned short (*GetContrast)(t_RND_CTRL* RCS);
	int	(*SetContrast)(t_RND_CTRL* RCS, unsigned short DesiredContrast);
	unsigned short (*GetSharpness)(t_RND_CTRL* RCS);
	int	(*SetSharpness)(t_RND_CTRL* RCS, unsigned short DesiredSharpness);
	unsigned short (*GetRedVideoGain)(t_RND_CTRL* RCS);
	int	(*SetRedVideoGain)(t_RND_CTRL* RCS, unsigned short DesiredRedVideoGain);
	unsigned short (*GetGreenVideoGain)(t_RND_CTRL* RCS);
	int	(*SetGreenVideoGain)(t_RND_CTRL* RCS, unsigned short DesiredGreenVideoGain);
	unsigned short (*GetBlueVideoGain)(t_RND_CTRL* RCS);
	int	(*SetBlueVideoGain)(t_RND_CTRL* RCS, unsigned short DesiredBlueVideoGain);
	unsigned short (*GetRedVideoBlackLevel)(t_RND_CTRL* RCS);
	int	(*SetRedVideoBlackLevel)(t_RND_CTRL* RCS, unsigned short DesiredRedVideoBlackLevel);
	unsigned short (*GetGreenVideoBlackLevel)(t_RND_CTRL* RCS);
	int	(*SetGreenVideoBlackLevel)(t_RND_CTRL* RCS, unsigned short DesiredGreenVideoBlackLevel);
	unsigned short (*GetBlueVideoBlackLevel)(t_RND_CTRL* RCS);
	int	(*SetBlueVideoBlackLevel)(t_RND_CTRL* RCS, unsigned short DesiredBlueVideoBlackLevel);
	unsigned short (*GetColorTemperature)(t_RND_CTRL* RCS);
	int	(*SetColorTemperature)(t_RND_CTRL* RCS, unsigned short DesiredColorTemperature);
	unsigned short (*GetHorizontalKeystone)(t_RND_CTRL* RCS);
	int	(*SetHorizontalKeystone)(t_RND_CTRL* RCS, short DesiredHorizontalKeystone);
	unsigned short (*GetVerticalKeystone)(t_RND_CTRL* RCS);
	int	(*SetVerticalKeystone)(t_RND_CTRL* RCS, short DesiredVerticalKeystone);
	unsigned short (*GetLoudness)(t_RND_CTRL* RCS, enum_CHANNEL_TYPE Channel);
	int	(*SetLoudness)(t_RND_CTRL* RCS, enum_CHANNEL_TYPE Channel, short DesiredLoudness);
	unsigned short (*GetMute)(t_RND_CTRL* RCS, enum_CHANNEL_TYPE Channel);
	int	(*SetMute)(t_RND_CTRL* RCS, enum_CHANNEL_TYPE Channel, int DesiredMute);
	unsigned short (*GetVolume)(t_RND_CTRL* RCS, enum_CHANNEL_TYPE Channel);
	int	(*SetVolume)(t_RND_CTRL* RCS, enum_CHANNEL_TYPE Channel, unsigned short DesiredVolume);
	unsigned short (*GetVolumeDB)(t_RND_CTRL* RCS, enum_CHANNEL_TYPE Channel);
	int	(*SetVolumeDB)(t_RND_CTRL* RCS, enum_CHANNEL_TYPE Channel, short DesiredVolume);
	unsigned short (*GetVolumeDBMin)(t_RND_CTRL* RCS, enum_CHANNEL_TYPE Channel);
	unsigned short (*GetVolumeDBMax)(t_RND_CTRL* RCS, enum_CHANNEL_TYPE Channel);	

	/* added by platform, not standard from dlna */
	int	(*GetAudioChannel)(t_RND_CTRL* RCS);
	int	(*SetAudioChannel)(t_RND_CTRL* RCS, int DesiredChannel);

	void (*Release)(t_RND_CTRL* RCS);
};


#define EXTERN_CHAR		extern char
#define EXTERN_INT		extern int
#define EXTERN_SHORT	extern unsigned short

EXTERN_CHAR *DMR_ListPresets(t_RND_CTRL* RCS);
EXTERN_INT DMR_SelectPreset(t_RND_CTRL* RCS, char *PresetName);

EXTERN_SHORT DMR_GetBrightness(t_RND_CTRL* RCS);
EXTERN_INT DMR_SetBrightness(t_RND_CTRL* RCS, unsigned short DesiredBrightness);

EXTERN_SHORT DMR_GetContrast(t_RND_CTRL* RCS);
EXTERN_INT DMR_SetContrast(t_RND_CTRL* RCS, unsigned short DesiredContrast);

EXTERN_SHORT DMR_GetSharpness(t_RND_CTRL* RCS);
EXTERN_INT DMR_SetSharpness(t_RND_CTRL* RCS, unsigned short DesiredSharpness);

EXTERN_SHORT DMR_GetRedVideoGain(t_RND_CTRL* RCS);
EXTERN_INT DMR_SetRedVideoGain(t_RND_CTRL* RCS, unsigned short DesiredRedVideoGain);

EXTERN_SHORT DMR_GetGreenVideoGain(t_RND_CTRL* RCS);
EXTERN_INT DMR_SetGreenVideoGain(t_RND_CTRL* RCS, unsigned short DesiredGreenVideoGain);

EXTERN_SHORT DMR_GetBlueVideoGain(t_RND_CTRL* RCS);
EXTERN_INT DMR_SetBlueVideoGain(t_RND_CTRL* RCS, unsigned short DesiredBlueVideoGain);

EXTERN_SHORT DMR_GetRedVideoBlackLevel(t_RND_CTRL* RCS);
EXTERN_INT DMR_SetRedVideoBlackLevel(t_RND_CTRL* RCS, unsigned short DesiredRedVideoBlackLevel);

EXTERN_SHORT DMR_GetGreenVideoBlackLevel(t_RND_CTRL* RCS);
EXTERN_INT DMR_SetGreenVideoBlackLevel(t_RND_CTRL* RCS, unsigned short DesiredGreenVideoBlackLevel);

EXTERN_SHORT DMR_GetBlueVideoBlackLevel(t_RND_CTRL* RCS);
EXTERN_INT DMR_SetBlueVideoBlackLevel(t_RND_CTRL* RCS, unsigned short DesiredBlueVideoBlackLevel);

EXTERN_SHORT DMR_GetColorTemperature(t_RND_CTRL* RCS);
EXTERN_INT DMR_SetColorTemperature(t_RND_CTRL* RCS, unsigned short DesiredColorTemperature);

EXTERN_SHORT DMR_GetHorizontalKeystone(t_RND_CTRL* RCS);
EXTERN_INT DMR_SetHorizontalKeystone(t_RND_CTRL* RCS, short DesiredHorizontalKeystone);

EXTERN_SHORT DMR_GetVerticalKeystone(t_RND_CTRL* RCS);
EXTERN_INT DMR_SetVerticalKeystone(t_RND_CTRL* RCS, short DesiredVerticalKeystone);

EXTERN_SHORT DMR_GetLoudness(t_RND_CTRL* RCS, enum_CHANNEL_TYPE Channel);
EXTERN_INT DMR_SetLoudness(t_RND_CTRL* RCS, enum_CHANNEL_TYPE Channel, short DesiredLoudness);

EXTERN_SHORT DMR_GetMute(t_RND_CTRL* RCS, enum_CHANNEL_TYPE Channel);
EXTERN_INT DMR_SetMute(t_RND_CTRL* RCS, enum_CHANNEL_TYPE Channel, int DesiredMute);

EXTERN_SHORT DMR_GetVolume(t_RND_CTRL* RCS, enum_CHANNEL_TYPE Channel);
EXTERN_INT DMR_SetVolume(t_RND_CTRL* RCS, enum_CHANNEL_TYPE Channel, unsigned short DesiredVolume);

EXTERN_SHORT DMR_GetVolumeDB(t_RND_CTRL* RCS, enum_CHANNEL_TYPE Channel);
EXTERN_INT DMR_SetVolumeDB(t_RND_CTRL* RCS, enum_CHANNEL_TYPE Channel, short DesiredVolume);

EXTERN_SHORT DMR_GetVolumeDBMin(t_RND_CTRL* RCS, enum_CHANNEL_TYPE Channel);
EXTERN_SHORT DMR_GetVolumeDBMax(t_RND_CTRL* RCS, enum_CHANNEL_TYPE Channel);

EXTERN_INT DMR_GetAudioChannel(t_RND_CTRL* RCS);
EXTERN_INT DMR_SetAudioChannel(t_RND_CTRL* RCS, int DesiredChannel);

void DMR_RenderingCS_Release(t_RND_CTRL* RCS);
t_RND_CTRL* DMR_RenderingCS_Create(void);


#endif /* __X_RENDERING_CONTROL_H__ */


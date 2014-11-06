#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "HySDK.h"
#include "x_renderingcontrol.h"
#include "hitTime.h"



#define STATIC_CHAR		char
#define STATIC_INT		int
#define STATIC_SHORT	unsigned short

#define P_PLAYER(p)	(p->rcHandle)


STATIC_CHAR *DMR_ListPresets(t_RND_CTRL* RCS)
{
	return NULL;
}
STATIC_INT DMR_SelectPreset(t_RND_CTRL* RCS, char *PresetName)
{
	return 0;
}

STATIC_SHORT DMR_GetBrightness(t_RND_CTRL* RCS)
{	
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_KEY, (int)RCS, 0);
	
	STATIC_SHORT ret = HySDK_RC_GetBrightness(P_PLAYER(RCS));
	
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}
STATIC_INT DMR_SetBrightness(t_RND_CTRL* RCS, unsigned short DesiredBrightness)
{
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_KEY, DesiredBrightness, 0);
	STATIC_INT ret = HySDK_RC_SetBrightness(P_PLAYER(RCS), DesiredBrightness);
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}

STATIC_SHORT DMR_GetContrast(t_RND_CTRL* RCS)
{
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_KEY, (int)RCS, 0);
	
	STATIC_SHORT ret = HySDK_RC_GetContrast(P_PLAYER(RCS));
	
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}
STATIC_INT DMR_SetContrast(t_RND_CTRL* RCS, unsigned short DesiredContrast)
{
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_KEY, DesiredContrast, 0);
	STATIC_INT ret = HySDK_RC_SetContrast(P_PLAYER(RCS), DesiredContrast);
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}

STATIC_SHORT DMR_GetSharpness(t_RND_CTRL* RCS)
{
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_KEY, (int)RCS, 0);
	
	STATIC_SHORT ret = HySDK_RC_GetSharpness(P_PLAYER(RCS));
	
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}
STATIC_INT DMR_SetSharpness(t_RND_CTRL* RCS, unsigned short DesiredSharpness)
{
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_KEY, DesiredSharpness, 0);
	STATIC_INT ret = HySDK_RC_SetSharpness(P_PLAYER(RCS), DesiredSharpness);
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}

STATIC_SHORT DMR_GetRedVideoGain(t_RND_CTRL* RCS)
{
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_KEY, (int)RCS, 0);
	
	STATIC_SHORT ret = HySDK_RC_GetRedVideoGain(P_PLAYER(RCS));
	
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}
STATIC_INT DMR_SetRedVideoGain(t_RND_CTRL* RCS, unsigned short DesiredRedVideoGain)
{
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_KEY, DesiredRedVideoGain, 0);
	STATIC_INT ret = HySDK_RC_SetRedVideoGain(P_PLAYER(RCS), DesiredRedVideoGain);
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}

STATIC_SHORT DMR_GetGreenVideoGain(t_RND_CTRL* RCS)
{
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_KEY, (int)RCS, 0);
	
	STATIC_SHORT ret = HySDK_RC_GetGreenVideoGain(P_PLAYER(RCS));
	
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}
STATIC_INT DMR_SetGreenVideoGain(t_RND_CTRL* RCS, unsigned short DesiredGreenVideoGain)
{
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_KEY, DesiredGreenVideoGain, 0);
	STATIC_INT ret = HySDK_RC_SetGreenVideoGain(P_PLAYER(RCS), DesiredGreenVideoGain);
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}

STATIC_SHORT DMR_GetBlueVideoGain(t_RND_CTRL* RCS)
{
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_KEY, (int)RCS, 0);
	
	STATIC_SHORT ret = HySDK_RC_GetBlueVideoGain(P_PLAYER(RCS));
	
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}
STATIC_INT DMR_SetBlueVideoGain(t_RND_CTRL* RCS, unsigned short DesiredBlueVideoGain)
{
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_KEY, DesiredBlueVideoGain, 0);
	STATIC_INT ret = HySDK_RC_SetBlueVideoGain(P_PLAYER(RCS), DesiredBlueVideoGain);
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}

STATIC_SHORT DMR_GetRedVideoBlackLevel(t_RND_CTRL* RCS)
{
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_KEY, (int)RCS, 0);
	
	STATIC_SHORT ret = HySDK_RC_GetRedVideoBlackLevel(P_PLAYER(RCS));
	
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}
STATIC_INT DMR_SetRedVideoBlackLevel(t_RND_CTRL* RCS, unsigned short DesiredRedVideoBlackLevel)
{
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_KEY, DesiredRedVideoBlackLevel, 0);
	STATIC_INT ret = HySDK_RC_SetRedVideoBlackLevel(P_PLAYER(RCS), DesiredRedVideoBlackLevel);
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}

STATIC_SHORT DMR_GetGreenVideoBlackLevel(t_RND_CTRL* RCS)
{
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_KEY, (int)RCS, 0);
	
	STATIC_SHORT ret = HySDK_RC_GetGreenVideoBlackLevel(P_PLAYER(RCS));
	
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}
STATIC_INT DMR_SetGreenVideoBlackLevel(t_RND_CTRL* RCS, unsigned short DesiredGreenVideoBlackLevel)
{
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_KEY, DesiredGreenVideoBlackLevel, 0);
	STATIC_INT ret = HySDK_RC_SetGreenVideoBlackLevel(P_PLAYER(RCS), DesiredGreenVideoBlackLevel);
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}

STATIC_SHORT DMR_GetBlueVideoBlackLevel(t_RND_CTRL* RCS)
{
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_KEY, (int)RCS, 0);
	
	STATIC_SHORT ret = HySDK_RC_GetBlueVideoBlackLevel(P_PLAYER(RCS));
	
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}
STATIC_INT DMR_SetBlueVideoBlackLevel(t_RND_CTRL* RCS, unsigned short DesiredBlueVideoBlackLevel)
{
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_KEY, DesiredBlueVideoBlackLevel, 0);
	STATIC_INT ret = HySDK_RC_SetBlueVideoBlackLevel(P_PLAYER(RCS), DesiredBlueVideoBlackLevel);
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}

STATIC_SHORT DMR_GetColorTemperature(t_RND_CTRL* RCS)
{
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_KEY, (int)RCS, 0);
	
	STATIC_SHORT ret = HySDK_RC_GetColorTemperature(P_PLAYER(RCS));
	
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}
STATIC_INT DMR_SetColorTemperature(t_RND_CTRL* RCS, unsigned short DesiredColorTemperature)
{
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_KEY, DesiredColorTemperature, 0);
	STATIC_INT ret = HySDK_RC_SetColorTemperature(P_PLAYER(RCS), DesiredColorTemperature);
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}

STATIC_SHORT DMR_GetHorizontalKeystone(t_RND_CTRL* RCS)
{
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_KEY, (int)RCS, 0);
	
	STATIC_SHORT ret = HySDK_RC_GetHorizontalKeystone(P_PLAYER(RCS));
	
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}
STATIC_INT DMR_SetHorizontalKeystone(t_RND_CTRL* RCS, short DesiredHorizontalKeystone)
{
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_KEY, DesiredHorizontalKeystone, 0);
	STATIC_INT ret = HySDK_RC_SetHorizontalKeystone(P_PLAYER(RCS), DesiredHorizontalKeystone);
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}

STATIC_SHORT DMR_GetVerticalKeystone(t_RND_CTRL* RCS)
{
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_KEY, (int)RCS, 0);
	
	STATIC_SHORT ret = HySDK_RC_GetVerticalKeystone(P_PLAYER(RCS));
	
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}
STATIC_INT DMR_SetVerticalKeystone(t_RND_CTRL* RCS, short DesiredVerticalKeystone)
{
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_KEY, DesiredVerticalKeystone, 0);
	STATIC_INT ret = HySDK_RC_SetVerticalKeystone(P_PLAYER(RCS), DesiredVerticalKeystone);
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}

STATIC_SHORT DMR_GetLoudness(t_RND_CTRL* RCS, enum_CHANNEL_TYPE Channel)
{
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_KEY, (int)RCS, 0);
	
	STATIC_SHORT ret = HySDK_RC_GetLoudness(P_PLAYER(RCS), Channel);
	
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}
STATIC_INT DMR_SetLoudness(t_RND_CTRL* RCS, enum_CHANNEL_TYPE Channel, short DesiredLoudness)
{
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_KEY, DesiredLoudness, 0);
	STATIC_INT ret = HySDK_RC_SetLoudness(P_PLAYER(RCS), Channel, DesiredLoudness);
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}

STATIC_SHORT DMR_GetMute(t_RND_CTRL* RCS, enum_CHANNEL_TYPE Channel)
{
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_KEY, (int)RCS, 0);
	
	STATIC_SHORT ret = HySDK_RC_GetMute(P_PLAYER(RCS), Channel);
	
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}
STATIC_INT DMR_SetMute(t_RND_CTRL* RCS, enum_CHANNEL_TYPE Channel, int DesiredMute)
{
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_KEY, DesiredMute, 0);
	STATIC_INT ret = HySDK_RC_SetMute(P_PLAYER(RCS), Channel, DesiredMute);
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}

STATIC_SHORT DMR_GetVolume(t_RND_CTRL* RCS, enum_CHANNEL_TYPE Channel)
{
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_KEY, (int)RCS, 0);
	
	STATIC_SHORT ret = HySDK_RC_GetVolume(P_PLAYER(RCS), Channel);
	
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}
STATIC_INT DMR_SetVolume(t_RND_CTRL* RCS, enum_CHANNEL_TYPE Channel, unsigned short DesiredVolume)
{
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_KEY, DesiredVolume, 0);
	STATIC_INT ret = HySDK_RC_SetVolume(P_PLAYER(RCS), Channel, DesiredVolume);
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}

STATIC_SHORT DMR_GetVolumeDB(t_RND_CTRL* RCS, enum_CHANNEL_TYPE Channel)
{
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_KEY, (int)RCS, 0);
	
	STATIC_SHORT ret = HySDK_RC_GetVolumeDB(P_PLAYER(RCS), Channel);
	
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}
STATIC_INT DMR_SetVolumeDB(t_RND_CTRL* RCS, enum_CHANNEL_TYPE Channel, short DesiredVolume)
{
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_KEY, DesiredVolume, 0);
	STATIC_INT ret = HySDK_RC_SetVolumeDB(P_PLAYER(RCS), Channel, DesiredVolume);
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}

STATIC_SHORT DMR_GetVolumeDBMin(t_RND_CTRL* RCS, enum_CHANNEL_TYPE Channel)
{
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_KEY, (int)RCS, 0);
	
	STATIC_SHORT ret = HySDK_RC_GetVolumeDBMin(P_PLAYER(RCS), Channel);
	
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}
STATIC_SHORT DMR_GetVolumeDBMax(t_RND_CTRL* RCS, enum_CHANNEL_TYPE Channel)
{
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_KEY, (int)RCS, 0);
	
	STATIC_SHORT ret = HySDK_RC_GetVolumeDBMax(P_PLAYER(RCS), Channel);
	
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}

STATIC_INT DMR_GetAudioChannel(t_RND_CTRL* RCS)
{
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_KEY, (int)RCS, 0);
	
	STATIC_INT ret = HySDK_RC_GetAudioChannel(P_PLAYER(RCS));
	
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}
STATIC_INT DMR_SetAudioChannel(t_RND_CTRL* RCS, int DesiredChannel)
{
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_KEY, (int)RCS, 0);
	
	STATIC_INT ret = HySDK_RC_SetAudioChannel(P_PLAYER(RCS), DesiredChannel);
	
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}

void DMR_RenderingCS_Release(t_RND_CTRL* RCS)
{
	HT_DBG_FUNC_START(HT_MOD_HYSDK, HT_BIT_KEY, (int)RCS, 0);
	if( RCS )
	{
		free( RCS );
	}
	
	HT_DBG_FUNC_END(0, 0);	
}

t_RND_CTRL* DMR_RenderingCS_Create(void)
{
	t_RND_CTRL *rcs = (t_RND_CTRL *)malloc(sizeof(t_RND_CTRL));
	memset(rcs, 0, sizeof(t_RND_CTRL));

	rcs -> ListPresets = DMR_ListPresets;
	rcs -> SelectPreset = DMR_SelectPreset;
	rcs -> GetBrightness = DMR_GetBrightness;
	rcs -> SetBrightness = DMR_SetBrightness;

	rcs -> GetContrast = DMR_GetContrast;
	rcs -> SetContrast = DMR_SetContrast;
	rcs -> GetSharpness = DMR_GetSharpness;
	rcs -> SetSharpness = DMR_SetSharpness;
	rcs -> GetRedVideoGain = DMR_GetRedVideoGain;
	rcs -> SetRedVideoGain = DMR_SetRedVideoGain;
	rcs -> GetGreenVideoGain = DMR_GetGreenVideoGain;
	rcs -> SetGreenVideoGain = DMR_SetGreenVideoGain;
	rcs -> GetBlueVideoGain = DMR_GetBlueVideoGain;
	rcs -> SetBlueVideoGain = DMR_SetBlueVideoGain;
	rcs -> GetRedVideoBlackLevel = DMR_GetRedVideoBlackLevel;
	rcs -> SetRedVideoBlackLevel = DMR_SetRedVideoBlackLevel;
	rcs -> GetGreenVideoBlackLevel = DMR_GetGreenVideoBlackLevel;
	rcs -> SetGreenVideoBlackLevel = DMR_SetGreenVideoBlackLevel;
	rcs -> GetBlueVideoBlackLevel = DMR_GetBlueVideoBlackLevel;
	rcs -> SetBlueVideoBlackLevel = DMR_SetBlueVideoBlackLevel;
	rcs -> GetColorTemperature = DMR_GetColorTemperature;
	rcs -> SetColorTemperature = DMR_SetColorTemperature;
	
	rcs -> GetHorizontalKeystone = DMR_GetHorizontalKeystone;
	rcs -> SetHorizontalKeystone = DMR_SetHorizontalKeystone;
	rcs -> GetVerticalKeystone = DMR_GetVerticalKeystone;
	rcs -> SetVerticalKeystone = DMR_SetVerticalKeystone;
	rcs -> GetLoudness = DMR_GetLoudness;
	rcs -> SetLoudness = DMR_SetLoudness;
	
	rcs -> SetMute = DMR_SetMute;
	rcs -> GetMute = DMR_GetMute;
	rcs -> SetVolume = DMR_SetVolume;
	rcs -> GetVolume = DMR_GetVolume;
	rcs -> GetVolumeDB = DMR_GetVolumeDB;
	rcs -> SetVolumeDB = DMR_SetVolumeDB;
	rcs -> GetVolumeDBMin = DMR_GetVolumeDBMin;
	rcs -> GetVolumeDBMax = DMR_GetVolumeDBMax;

	rcs -> GetAudioChannel = DMR_GetAudioChannel;
	rcs -> SetAudioChannel = DMR_SetAudioChannel;

	rcs->Release = DMR_RenderingCS_Release;
	return rcs;
}




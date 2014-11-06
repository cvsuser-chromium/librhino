/*
 * libdlna: reference DLNA standards implementation.
 * Copyright (C) 2007 Benjamin Zores <ben@geexbox.org>
 *
 * This file is part of libdlna.
 *
 * libdlna is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * libdlna is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with libdlna; if not, write to the Free Software
 * Foundation, Inc, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "dlna_internals.h"
#include "profiles.h"
#include "containers.h"
#include "hitTime.h"
extern dlna_registered_profile_t dlna_profile_image_jpeg;
extern dlna_registered_profile_t dlna_profile_image_png;

extern dlna_registered_profile_t dlna_profile_audio_ac3;
extern dlna_registered_profile_t dlna_profile_audio_amr;
extern dlna_registered_profile_t dlna_profile_audio_atrac3;
//extern dlna_registered_profile_t dlna_profile_audio_lpcm;
extern dlna_registered_profile_t dlna_profile_audio_wav;
extern dlna_registered_profile_t dlna_profile_audio_mp3;
extern dlna_registered_profile_t dlna_profile_audio_mpeg4;
extern dlna_registered_profile_t dlna_profile_audio_wma;

extern dlna_registered_profile_t dlna_profile_av_mpeg1;
extern dlna_registered_profile_t dlna_profile_av_mpeg2;
extern dlna_registered_profile_t dlna_profile_av_mpeg4_part2;
extern dlna_registered_profile_t dlna_profile_av_mpeg4_part10;
extern dlna_registered_profile_t dlna_profile_av_wmv9;

static void
dlna_register_profile (dlna_t *dlna, dlna_registered_profile_t *profile)
{
  void **p;

  if (!dlna)
    return;

  if (!dlna->inited)
    dlna = dlna_init ();
  
  p = &dlna->first_profile;
  while (*p != NULL)
  {
    if (((dlna_registered_profile_t *) *p)->id == profile->id)
      return; /* already registered */
    p = (void *) &((dlna_registered_profile_t *) *p)->next;
  }
  *p = profile;
  profile->next = NULL;
}

void
dlna_register_all_media_profiles (dlna_t *dlna)
{
  if (!dlna)
    return;
  
  if (!dlna->inited)
    dlna = dlna_init ();
  
  dlna_register_profile (dlna, &dlna_profile_image_jpeg);
  dlna_register_profile (dlna, &dlna_profile_image_png);
//  dlna_register_profile (dlna, &dlna_profile_audio_ac3);
//  dlna_register_profile (dlna, &dlna_profile_audio_amr);
//  dlna_register_profile (dlna, &dlna_profile_audio_atrac3);
//  dlna_register_profile (dlna, &dlna_profile_audio_lpcm);
  dlna_register_profile (dlna, &dlna_profile_audio_wav);
  dlna_register_profile (dlna, &dlna_profile_audio_mp3);
//  dlna_register_profile (dlna, &dlna_profile_audio_mpeg4);
  dlna_register_profile (dlna, &dlna_profile_audio_wma);
  dlna_register_profile (dlna, &dlna_profile_av_mpeg1);
  dlna_register_profile (dlna, &dlna_profile_av_mpeg2);
  dlna_register_profile (dlna, &dlna_profile_av_mpeg4_part2);
  dlna_register_profile (dlna, &dlna_profile_av_mpeg4_part10);
  dlna_register_profile (dlna, &dlna_profile_av_wmv9);
}
#if 0
void
dlna_register_media_profile (dlna_t *dlna, dlna_media_profile_t profile)
{
  if (!dlna)
    return;
  
  if (!dlna->inited)
    dlna = dlna_init ();
  
  switch (profile)
  {
  case DLNA_PROFILE_IMAGE_JPEG:
    dlna_register_profile (dlna, &dlna_profile_image_jpeg);
    break;
  case DLNA_PROFILE_IMAGE_PNG:
    dlna_register_profile (dlna, &dlna_profile_image_png);
    break;
  case DLNA_PROFILE_AUDIO_AC3:
    dlna_register_profile (dlna, &dlna_profile_audio_ac3);
    break;
  case DLNA_PROFILE_AUDIO_AMR:
    dlna_register_profile (dlna, &dlna_profile_audio_amr);
    break;
  case DLNA_PROFILE_AUDIO_ATRAC3:
    dlna_register_profile (dlna, &dlna_profile_audio_atrac3);
    break;
  case DLNA_PROFILE_AUDIO_LPCM:
    dlna_register_profile (dlna, &dlna_profile_audio_lpcm);
    break;
  case DLNA_PROFILE_AUDIO_MP3:
    dlna_register_profile (dlna, &dlna_profile_audio_mp3);
    break;
  case DLNA_PROFILE_AUDIO_MPEG4:
    dlna_register_profile (dlna, &dlna_profile_audio_mpeg4);
    break;
  case DLNA_PROFILE_AUDIO_WMA:
    dlna_register_profile (dlna, &dlna_profile_audio_wma);
    break;
  case DLNA_PROFILE_AV_MPEG1:
    dlna_register_profile (dlna, &dlna_profile_av_mpeg1);
    break;
  case DLNA_PROFILE_AV_MPEG2:
    dlna_register_profile (dlna, &dlna_profile_av_mpeg2);
    break;
  case DLNA_PROFILE_AV_MPEG4_PART2:
    dlna_register_profile (dlna, &dlna_profile_av_mpeg4_part2);
    break;
  case DLNA_PROFILE_AV_MPEG4_PART10:
    dlna_register_profile (dlna, &dlna_profile_av_mpeg4_part10);
    break;
  case DLNA_PROFILE_AV_WMV9:
    dlna_register_profile (dlna, &dlna_profile_av_wmv9);
    break;
  default:
    break;
  }
}
#endif
dlna_t *
dlna_init (void)
{
  dlna_t *dlna;

  dlna = malloc (sizeof (dlna_t));
  dlna->inited = 1;
  dlna->verbosity = 0;
  dlna->first_profile = NULL;
  
  /* register all FFMPEG demuxers */
  av_register_all ();

  return dlna;
}

void
dlna_uninit (dlna_t *dlna)
{
  if (!dlna)
    return;

  dlna->inited = 0;
  dlna->first_profile = NULL;
  free (dlna);
}

void
dlna_set_verbosity (dlna_t *dlna, int level)
{
  if (!dlna)
    return;

  dlna->verbosity = level;
}

void
dlna_set_extension_check (dlna_t *dlna, int level)
{
  if (!dlna)
    return;

  dlna->check_extensions = level;
}

static av_codecs_t *
av_profile_get_codecs (AVFormatContext *ctx)
{
  av_codecs_t *codecs = NULL;
  unsigned int i;
  int audio_stream = -1, video_stream = -1;
 
  codecs = malloc (sizeof (av_codecs_t));

  for (i = 0; i < ctx->nb_streams; i++)
  {
    if (audio_stream == -1 &&
        ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
    {
      audio_stream = i;
      continue;
    }
    else if (video_stream == -1 &&
             ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
    {
      video_stream = i;
      continue;
    }
  }

  codecs->as = audio_stream >= 0 ? ctx->streams[audio_stream] : NULL;
  codecs->ac = audio_stream >= 0 ? ctx->streams[audio_stream]->codec : NULL;

  codecs->vs = video_stream >= 0 ? ctx->streams[video_stream] : NULL;
  codecs->vc = video_stream >= 0 ? ctx->streams[video_stream]->codec : NULL;

  /* check for at least one video stream and one audio stream in container */
  if (!codecs->ac && !codecs->vc)
  {
    free (codecs);
    return NULL;
  }
  
  return codecs;
}

static int
match_file_extension (const char *filename, const char *extensions)
{
  const char *ext, *p;
  char ext1[32], *q;

  if (!filename)
    return 0;

  ext = strrchr (filename, '.');
  if (ext)
  {
    ext++;
    p = extensions;
    for (;;)
    {
      q = ext1;
      while (*p != '\0' && *p != ',' && (q - ext1 < (int) sizeof (ext1) - 1))
        *q++ = *p++;
      *q = '\0';
      if (!strcasecmp (ext1, ext))
        return 1;
      if (*p == '\0')
        break;
      p++;
    }
  }
  
  return 0;
}

static dlna_profile_t *s_dlna_guess_media_profile (dlna_t *dlna, const char *filename)
{
	AVFormatContext *ctx;
	dlna_registered_profile_t *p;
	dlna_profile_t *profile = NULL;
	dlna_container_type_t st;
	av_codecs_t *codecs;
	
	HT_DBG_FUNC_START(HT_MOD_DMS, HT_BIT_MYRIAD, (int)dlna, filename);
	if (!dlna)
		return NULL;

	if (!dlna->inited)
		dlna = dlna_init ();

	HT_DBG_FUNC(0, "1 av_open_input_file");
	if (av_open_input_file (&ctx, filename, NULL, 0, NULL) != 0)
	{
		if (dlna->verbosity)
		{
			HT_DBG_FUNC(01, "can't open file: %s\n");
			//fprintf (stderr, "can't open file: %s\n", filename);
		}
		return NULL;
	}

	HT_DBG_FUNC((int)ctx, "2 av_find_stream_info, ctx = ");
	if (av_find_stream_info (ctx) < 0)
	{
		if (dlna->verbosity)
		{
			HT_DBG_FUNC(01, "can't find stream info\n");
			//fprintf (stderr, "can't find stream info\n");
		}
		return NULL;
	}

#ifdef HAVE_DEBUG
	dump_format (ctx, 0, NULL, 0);
#endif /* HAVE_DEBUG */

	HT_DBG_FUNC((int)ctx, "3 av_profile_get_codecs, ctx = ");
	/* grab codecs info */
	codecs = av_profile_get_codecs (ctx);
	HT_DBG_FUNC((int)codecs, "3 codecs = ");
	if (!codecs)
		return NULL;
	
	HT_DBG_FUNC((int)(codecs->ac), "4 codecs->ac = ");
	if(codecs->ac)
	{
		HT_DBG_FUNC((int)(codecs->ac->codec_id), "4 codecs->ac->codec_id = ");
		HT_DBG_FUNC((int)(codecs->ac->bit_rate), "4 codecs->ac->bit_rate = ");
		HT_DBG_FUNC((int)(codecs->ac->channels), "4 codecs->ac->channels = ");
	}
	HT_DBG_FUNC((int)(codecs->vc), "4 codecs->vc = ");
	if(codecs->vc)
	{
		HT_DBG_FUNC((int)(codecs->vc->codec_id), "4 codecs->vc->codec_id = ");
		HT_DBG_FUNC((int)(codecs->vc->bit_rate), "4 codecs->vc->bit_rate = ");
		HT_DBG_FUNC((int)(codecs->vc->width),    "4 codecs->vc->width  = ");
		HT_DBG_FUNC((int)(codecs->vc->height),   "4 codecs->vc->height = ");
	}

	/* check for container type */
	st = stream_get_container (ctx);
	HT_DBG_FUNC((int)st, "5 stream_get_container, st = ");

	p = dlna->first_profile;
	while (p)
	{
		dlna_profile_t *prof;

		if (dlna->check_extensions)
		{
			if (p->extensions)
			{
				/* check for valid file extension */
				if (!match_file_extension (filename, p->extensions))
				{
					p = p->next;
					continue;
				}
			}
		}

		prof = p->probe (ctx, st, codecs);
		if (prof)
		{
			profile = prof;
			profile->class = p->class;
			break;
		}
		p = p->next;
	}

	av_close_input_file (ctx);
	free (codecs);
	return profile;
}
dlna_profile_t *dlna_guess_media_profile (dlna_t *dlna, const char *filename)
{
	dlna_profile_t *ret;
	HT_DBG_FUNC_START(HT_MOD_DMS, HT_BIT_MYRIAD, (int)dlna, filename);
	ret = s_dlna_guess_media_profile(dlna, filename);
	HT_DBG_FUNC_END((int)ret, NULL);
	return ret;
}

/* UPnP ContentDirectory Object Item */
#define UPNP_OBJECT_ITEM_PHOTO            "object.item.imageItem.photo"
#define UPNP_OBJECT_ITEM_AUDIO            "object.item.audioItem.musicTrack"
#define UPNP_OBJECT_ITEM_VIDEO            "object.item.videoItem.movie"

char *
dlna_profile_upnp_object_item (dlna_profile_t *profile)
{
  if (!profile)
    return NULL;

  switch (profile->class)
  {
  case DLNA_CLASS_IMAGE:
    return UPNP_OBJECT_ITEM_PHOTO;
  case DLNA_CLASS_AUDIO:
    return UPNP_OBJECT_ITEM_AUDIO;
  case DLNA_CLASS_AV:
    return UPNP_OBJECT_ITEM_VIDEO;
  default:
    break;
  }

  return NULL;
}

int
stream_ctx_is_image (AVFormatContext *ctx,
                     av_codecs_t *codecs, dlna_container_type_t st)
{
  /* should only have 1 stream */
  if (ctx->nb_streams > 1)
    return 0;

  /* should be inside image container */
  if (st != CT_IMAGE)
    return 0;

  if (!codecs->vc)
    return 0;

  return 1;
}

int
stream_ctx_is_audio (av_codecs_t *codecs)
{
  /* we need an audio codec ... */
  if (!codecs->ac)
    return 0;

  /* ... but no video one */
  if (codecs->vc)
    return 0;

  return 1;
}

int
stream_ctx_is_av (av_codecs_t *codecs)
{
  if (!codecs->as || !codecs->ac || !codecs->vs || !codecs->vc)
    return 0;

  return 1;
}

char *
get_file_extension (const char *filename)
{
  char *str = NULL;

  str = strrchr (filename, '.');
  if (str)
    str++;

  return str;
}
#if 0
dlna_write_protocol_info_ex (DLNA_PROTOCOL_INFO_TYPE_HTTP,
						  DLNA_ORG_PLAY_SPEED_NORMAL,
						  DLNA_ORG_CONVERSION_NONE,
						  DLNA_ORG_OPERATION_RANGE,
						  ut->dlna_flags, entry->protocol_info) :

char *
dlna_write_protocol_info_ex (dlna_protocol_info_type_t type,
                          dlna_org_play_speed_t speed,
                          dlna_org_conversion_t ci,
                          dlna_org_operation_t op,
                          dlna_org_flags_t flags,
                          dlna_profile_t *p)
{
  char protocol[512];
  char dlna_info[448];

  if (type == DLNA_PROTOCOL_INFO_TYPE_HTTP)
    sprintf (protocol, "http-get:*:");

  strcat (protocol, p->mime);
  strcat (protocol, ":");
  
  sprintf (dlna_info, "%s=%s;%s=%.2x;%s=%d;%s=%d;%s=%.8x%.24x",
  						"DLNA.ORG_PN", p->id,
						"DLNA.ORG_OP", op, 
						"DLNA.ORG_PS", speed, 
						"DLNA.ORG_CI", ci,
						"DLNA.ORG_FLAGS", flags, 0);
  strcat (protocol, dlna_info);

  return strdup (protocol);
}
#endif

int dlna_write_protocol_info_http (char *buf, int buf_len, dlna_org_flags_t flags, dlna_profile_t *p)
{
	char *s;
	int len;

	if(p && p->mime && p->id)
	{
		s = "http-get:*:%s:DLNA.ORG_PN=%s;DLNA.ORG_OP=01;DLNA.ORG_PS=1;DLNA.ORG_CI=0;DLNA.ORG_FLAGS=%.8x%.24x";
		len = sprintf(buf, s, p->mime, p->id, flags, 0);
	}
	else if(p && p->mime)
	{
		s = "http-get:*:%s:*";
		len = sprintf(buf, s, p->mime);
	}
	else
	{
		s = "http-get:*:%s:*";
		len = sprintf(buf, s, "text/xml");
	}
		
	return len;
}

audio_profile_t
audio_profile_guess (AVCodecContext *ac)
{
  audio_profile_t ap = AUDIO_PROFILE_INVALID;
  
  if (!ac)
    return ap;

  ap = audio_profile_guess_aac (ac);
  if (ap != AUDIO_PROFILE_INVALID)
    return ap;

  ap = audio_profile_guess_ac3 (ac);
  if (ap != AUDIO_PROFILE_INVALID)
    return ap;

  ap = audio_profile_guess_amr (ac);
  if (ap != AUDIO_PROFILE_INVALID)
    return ap;

  ap = audio_profile_guess_atrac (ac);
  if (ap != AUDIO_PROFILE_INVALID)
    return ap;

  ap = audio_profile_guess_g726 (ac);
  if (ap != AUDIO_PROFILE_INVALID)
    return ap;

  ap = audio_profile_guess_lpcm (ac);
  if (ap != AUDIO_PROFILE_INVALID)
    return ap;

  ap = audio_profile_guess_mp2 (ac);
  if (ap != AUDIO_PROFILE_INVALID)
    return ap;

  ap = audio_profile_guess_mp3 (ac);
  if (ap != AUDIO_PROFILE_INVALID)
    return ap;

  ap = audio_profile_guess_wma (ac);
  if (ap != AUDIO_PROFILE_INVALID)
    return ap;

  return AUDIO_PROFILE_INVALID;
}
#if 0
int av_open_input_file(AVFormatContext **ic_ptr, const char *filename,
                       AVInputFormat *fmt,
                       int buf_size,
                       void *ap)
{
	return 0;
}

void av_close_input_file(AVFormatContext *s)
{
}

int av_find_stream_info(AVFormatContext *ic)
{
	return 0;
}
#endif


static dlna_profile_t avc_ts_mp_hd_aac_pvr = {
  .id = "AVC_TS_MP_HD_AAC",
  .mime = MIME_VIDEO_MPEG_TS,
  .label = LABEL_VIDEO_HD
};

dlna_profile_t *dlna_guess_pvr_info(dlna_t *dlna)
{
	dlna_profile_t *profile = NULL;

	
	profile = &avc_ts_mp_hd_aac_pvr;
	profile->class = DLNA_CLASS_AV;
	
	return profile;
}

#include "dms.h"
static int s_dlna_guess_usb_media_profile(dlna_t *dlna, const char *filename, C_DMS_CMI *item)
{
	int ret = -1;
	AVFormatContext *ctx = NULL;
	av_codecs_t *codecs = NULL;
	dlna_profile_t *profile = NULL;
	
	HT_DBG_FUNC_START(HT_MOD_DMS, HT_BIT_MYRIAD, (int)item, 0);// HT_BIT_MYRIAD HT_BIT_MANY
	if (av_open_input_file(&ctx, filename, NULL, 0, NULL) != 0)
		goto s_EXIT;

	ret--;
	if (av_find_stream_info (ctx) < 0)
		goto s_EXIT;

	/* grab codecs info */
	ret--;
	if(!(codecs = av_profile_get_codecs (ctx)))
		goto s_EXIT;

	if(codecs->ac)
	{
		HT_DBG_FUNC((int)(codecs->ac->codec_id), "4 codecs->ac->codec_id = ");
		HT_DBG_FUNC((int)(codecs->ac->bit_rate), "4 codecs->ac->bit_rate = ");
		HT_DBG_FUNC((int)(codecs->ac->channels), "4 codecs->ac->channels = ");
	}
	if(codecs->vc)
	{
		HT_DBG_FUNC((int)(codecs->vc->codec_id), "4 codecs->vc->codec_id = ");
		HT_DBG_FUNC((int)(codecs->vc->bit_rate), "4 codecs->vc->bit_rate = ");
		HT_DBG_FUNC((int)(codecs->vc->width),	 "4 codecs->vc->width  = ");
		HT_DBG_FUNC((int)(codecs->vc->height),	 "4 codecs->vc->height = ");
	}

	if(codecs->vc && codecs->ac)
	{
		item->color_depth		= 0;
		item->duration			= 0;
		item->bitrate			= codecs->vc->bit_rate;
		item->res_width			= codecs->vc->width;
		item->res_height		= codecs->vc->height;
		
	}
	else if(codecs->vc)
	{
		item->color_depth		= 0;
		item->duration			= 0;
		item->bitrate			= codecs->vc->bit_rate;
		item->res_width			= codecs->vc->width;
		item->res_height		= codecs->vc->height;
	}
	else if(codecs->ac)
	{
		item->duration			= 0;
		item->bitrate			= codecs->ac->bit_rate;
	}
	else
	{
	}

	/* check for container type */
	dlna_container_type_t st = stream_get_container (ctx);
	HT_DBG_FUNC((int)st, "5 stream_get_container, st = ");

	dlna_registered_profile_t *p = dlna->first_profile;
	while (p)
	{
		dlna_profile_t *prof;

		prof = p->probe (ctx, st, codecs);
		if (prof)
		{
			profile = prof;
			profile->class = p->class;
			break;
		}
		p = p->next;
	}

	item->protocol_info = (C_DMS_IPI *)profile;
	if(profile)
		ret = 0;
	
s_EXIT:	
	if(codecs)
		free(codecs);
	if(ctx)
		av_close_input_file(ctx);
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}

int dlna_guess_usb_media_profile(dlna_t *dlna, const char *filename, C_DMS_CMI *item)
{
	HT_DBG_FUNC_START(HT_MOD_DMS, HT_BIT_MANY, (int)dlna, filename);
	int ret = s_dlna_guess_usb_media_profile(dlna, filename, item);
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}

const char *dlna_get_upnp_class(int upnp_class)
{
	switch(upnp_class)
	{
		case DLNA_CLASS_IMAGE:
			return UPNP_OBJECT_ITEM_PHOTO;
		case DLNA_CLASS_AUDIO:
			return UPNP_OBJECT_ITEM_AUDIO;
		case DLNA_CLASS_AV:
			return UPNP_OBJECT_ITEM_VIDEO;
		default:
			break;
	}
	return NULL;
}

int dlna_write_protocol_info(char *buf, int buf_len, dlna_org_flags_t flags, C_DMS_IPI *p)
{
	char *s;
	int len;

	if(p && p->mime && p->id)
	{
		s = "http-get:*:%s:DLNA.ORG_PN=%s;DLNA.ORG_OP=01;DLNA.ORG_PS=1;DLNA.ORG_CI=0;DLNA.ORG_FLAGS=%.8x%.24x";
		len = sprintf(buf, s, p->mime, p->id, flags, 0);
	}
	else if(p && p->mime)
	{
		s = "http-get:*:%s:*";
		len = sprintf(buf, s, p->mime);
	}
	else
	{
		s = "http-get:*:%s:*";
		len = sprintf(buf, s, "text/xml");
	}
		
	return len;
}


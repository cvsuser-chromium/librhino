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
*    filename:              Stream_detect.c
*    author(s):              zhangmin
*    version:                ver 0.1
*    date:				2007/09/28
*
*--------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include<string.h>

#include "AvMediaInfo.h"
#include "stream_detect.h"


void stream_detect_duration(char* buf, int buf_len, MEDIA_INFO* media_info)
{
	if ( (media_info->majorType == X_MEDIA_AUDIO) && (media_info->otherType.audioType == WAVE_FORMAT_MPEGLAYER3) )
		media_info->Time_Duration.tv_sec	= mp3_parse_duration(buf, buf_len, media_info->fileSize);
#ifdef  PARSER_ADDINFO_WMV
	if ( (media_info->majorType == X_MEDIA_AUDIO) && (media_info->otherType.audioType == WAVE_FORMAT_WMA_AUDIO) )
		media_info->Time_Duration.tv_sec	= parser_wma_duration();
#endif	
	
}

int stream_detect_type(unsigned char *buffer, long bytes, MEDIA_INFO* the_media_info)
{
	printf("xxx%x  xxx%d \n",buffer[0],bytes);
	if (format_is_ts(buffer, bytes, the_media_info)) {
		printf("xxx stream is ts xxx\n");
		the_media_info->majorType			= X_MEDIA_VIDEO;
		the_media_info->otherType.videoType	= V_STREAM_MPEG2_TS;
/*TI需要进一步分析音视频格式*/
#ifdef PARSER_ADDINFO_TS
//#if 1
		parser_ts(buffer,bytes,the_media_info);
#endif
		return 0;	
	} else if (format_is_ps(buffer, bytes, the_media_info)) {
		printf("xxx stream is PS xxx\n");	
		the_media_info->majorType			= X_MEDIA_VIDEO;
		the_media_info->otherType.videoType	= V_STREAM_MPEG2_PS;
		return 0;
	} else if (format_is_wmv(buffer, bytes, the_media_info)) {
		printf("xxx stream is wmv xxx\n");
		the_media_info->majorType			= X_MEDIA_VIDEO;
		the_media_info->otherType.videoType	= V_STREAM_VIDEO_WMV;
#ifdef  PARSER_ADDINFO_WMV
//#if 1
		parser_wmv(buffer,bytes,the_media_info);
#endif
		return 0;	
	} else if (format_is_wav(buffer, bytes, the_media_info)) {
		printf("xxx stream is WAV  xxx\n");
		the_media_info->majorType			= X_MEDIA_AUDIO;
		the_media_info->otherType.audioType	= WAVE_FORMAT_DVI_ADPCM;
//#ifdef PARSER_ADDINFO_WAV
#if 1
		/*TI不支持WAV，所以设置为未知*/
		the_media_info->majorType	= X_UNKNOWN;
#endif
		return 0;	
	} else if (format_is_mp3(buffer, bytes, the_media_info)) {
		printf("xxx stream is MP3  xxx\n");
		the_media_info->majorType			= X_MEDIA_AUDIO;
		the_media_info->otherType.audioType	= WAVE_FORMAT_MPEGLAYER3;
		return 0;	

	} else {
		printf("xxx stream is unkown xxx\n");
		the_media_info->majorType	= X_UNKNOWN;
		return -1;
	}
}


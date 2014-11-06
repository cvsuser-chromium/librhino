#include <stdio.h>
#include <stdlib.h>
#include<string.h>

#include "AvMediaInfo.h"

#include "parser_mp3.h"


static int mp3_get_param(unsigned char* buffer, unsigned long bytes, Mp3Info* mp3_info);

//bits V1,L1 V1,L2 V1,L3 V2,L1 V2,L2 V2,L3
RMuint32 bitrate_sss[16][2][3] = {
	{{0  ,  0 ,  0 ,}, {0  , 0  , 0  }},
	{{32 , 32 , 32 ,}, {32 , 8  , 8  }},
	{{64 , 48 , 40 ,}, {48 , 16 , 16 }},
	{{96 , 56 , 48 ,}, {56 , 24 , 24 }},
	{{128, 64 , 56 ,}, {64 , 32 , 32 }},
	{{160, 80 , 64 ,}, {80 , 40 , 40 }},
	{{192, 96 , 80 ,}, {96 , 48 , 48 }},
	{{224, 112, 96 ,}, {112, 56 , 56 }},
	{{256, 128, 112,}, {128, 64 , 64 }},
	{{288, 160, 128,}, {144, 80 , 80 }},
	{{320, 192, 160,}, {160, 96 , 96 }},
	{{352, 224, 192,}, {176, 112, 112}},
	{{384, 256, 224,}, {192, 128, 128}},
	{{416, 320, 256,}, {224, 144, 144}},
	{{448, 384, 320,}, {256, 160, 160}},
	{{0,   0,   0,  }, {0,   0,   0  }}
};

RMuint32 samples_ss[3][3] = {
	{384 , 384 , 384 },
	{1152, 1152, 1152},
	{1152, 576 , 576 }
};

//bits MPEG1 MPEG2 MPEG2.5
RMuint32 samrate_ss[4][3] = {
	{44100, 22050, 11025},
	{48000, 24000, 12000},
	{32000, 16000, 8000 },
	{0,     0,     0,   }
};



extern int plog(char *fmt, ...);
int format_is_mp3(unsigned char *buffer, long bytes, MEDIA_INFO* the_media_info)
{
	unsigned char* p_buffer	= buffer;
	long i	= 0;
	Mp3Info mp3_info;
	
	//分析mp3头
	while (i < 5) {
		if ( (*p_buffer == 0x49) && (*(p_buffer+1) == 0x44) && (*(p_buffer+2) == 0x33) && 
			(*(p_buffer+3) == 0x03) && (*(p_buffer+4) == 0x00) ) {
			printf("confirm this media is mp3.....\n");
			mp3_get_param(p_buffer, bytes-(p_buffer-buffer), &mp3_info);
			the_media_info->Bitrate	= mp3_info.bitrate;

			plog("===========detect format is mp3,byte: %d, filesize:%d\n", bytes, the_media_info->fileSize);
			the_media_info->Time_Duration.tv_sec	= mp3_parse_duration(buffer, bytes, the_media_info->fileSize);
			plog("xxxxxxxxxxxxxxxxxxxxxxxduration : %d", the_media_info->Time_Duration.tv_sec);

			return 1;
		} else {
			p_buffer++;
			i++;
		}
	}
	//如果没有头，分析数据
	p_buffer	= buffer;
	i	= 0;
	while ((*p_buffer == '\0') && (i < bytes)) {
		i++;
		p_buffer++;
	}
	if (i >= bytes) {
		plog("this is NOT mp3...,all is 0");
		return 0;
	}
	i	= 0;
	while(i < 10) {
		if ((*p_buffer	== 0xff) && ( (*(p_buffer+1) == 0xfb) || (*(p_buffer+1) == 0xfa) ) ) {
			if (mp3_get_param(p_buffer, bytes-(p_buffer-buffer), &mp3_info))
				return 0;
			else {
				int frame_size;
				plog("begin to calc frame_size...ver:%d,bitrate:%d,samrate:%d,padding:%d...", 
					mp3_info.version, mp3_info.bitrate, mp3_info.samrate, mp3_info.padding);
				frame_size	= ( ( (mp3_info.version == 1)?144:72) * mp3_info.bitrate / mp3_info.samrate + mp3_info.padding);
				plog("end to calc frame_size...:%d...", frame_size);
				p_buffer += frame_size;
				if (i >= 5) {
					the_media_info->Bitrate	= mp3_info.bitrate;
					plog("!!!!!!!!!!!!mp3!,but have no head!!!  bitrate:%d....", mp3_info.bitrate);
					the_media_info->Time_Duration.tv_sec	= mp3_parse_duration(buffer, bytes, the_media_info->fileSize);
					plog("xxxxxxxxxxxxxxxxxxxxxxxduration : %dl", the_media_info->Time_Duration.tv_sec);

					return 1;
				}
			}

			i++;


		}		
		i++;
	}
	
	return 0;
}

static int mp3_get_param(unsigned char* buffer, unsigned long bytes, Mp3Info* mp3_info)
{
	unsigned char* p_buffer		= buffer;
	ID3v2Info* p_id3_v2_info	= (ID3v2Info*)buffer;
	int head_length;
	unsigned int frame_head;
	int bits;

	if ( (*p_buffer == 0x49) && (*(p_buffer+1) == 0x44) && (*(p_buffer+2) == 0x33) && 
		(*(p_buffer+3) == 0x03) && (*(p_buffer+4) == 0x00) ) {
		head_length	= (p_id3_v2_info->Size[0] & 0x7F) * 0x200000 + 
			(p_id3_v2_info->Size[1] & 0x7f) * 0x400 +
			(p_id3_v2_info->Size[2] & 0x7f) * 0x80 +
			(p_id3_v2_info->Size[3] & 0x7f);
		plog("ID3_head length:%d", head_length);
		p_buffer	+= (head_length + 10);
	}
	
	frame_head	= (*p_buffer) << 24 | (*(p_buffer+1) << 16) | (*(p_buffer+2) << 8) | (*(p_buffer+3) << 0);
	switch ( (frame_head >> 19) & 0x3 ) {
	case 3:
		mp3_info->version	= 1;
		break;
	case 2:
		mp3_info->version	= 2;
		break;
	case 0:
		mp3_info->version	= 3;
		break;
	default:
		mp3_info->version	= 0;
		break;
	}

	switch ( (frame_head >> 17) & 0x3 ) {
	case 1:
		mp3_info->layer	= 3;
		break;
	case 2:
		mp3_info->layer	= 2;
		break;
	case 3:
		mp3_info->layer	= 1;
		break;
	default:
		mp3_info->layer	= 0;
		break;
	}

	plog("samrate[%d][%d]", (frame_head >> 10) & 0x3, mp3_info->version-1);
	mp3_info->samrate	= samrate_ss[(frame_head >> 10) & 0x3][mp3_info->version-1];

	mp3_info->padding	= (frame_head >> 9) & 0x1;

	bits	= (frame_head >> 12) & 0xf;
	if ( (bits >= 0xf) || (bits < 0)  || (mp3_info->version == 0) || (mp3_info->layer == 0) || (mp3_info->samrate == 0) ) {
		mp3_info->bitrate	= 0;
		return 1;
	}

	

	mp3_info->bitrate	= 1000 * bitrate_sss[(frame_head >> 12) & 0xf][mp3_info->version-1][mp3_info->layer-1];
	plog("[%d][%d][%d]...", (frame_head >> 12) & 0xf, mp3_info->version-1, mp3_info->layer-1);
	plog("mp3_info->bitrate=%d...", mp3_info->bitrate);




	
	{
		int i;
		for (i = 0; i < 8; i++)
			printf("0x%x\t", *(p_buffer+i) );
		printf("\n");
	}

	return 0;
}

long mp3_parse_duration(unsigned char  *buf, long len, long fileSize)
{
	RMuint32 bitrate, samrate;
	RMuint32 samples, frames, frameSize, duration = 0;//msec
	RMuint8 *mark;
	RMuint32 loops = 0;

	Mp3Info info;
	
	plog("mp3_parse_duration len:%d, filesize:%d", len, fileSize);
	if(buf == NULL || len < 1024 || fileSize < len) {
		plog("mp3_parse_duration: param error:len:%d, filesize:%d", len, fileSize);
		return 0;
	}

	memset(&info, 0, sizeof(Mp3Info));

LOOP:
	while(buf[0] == 0x00) //略去开头的零填充
		buf ++;

	plog("mp3_parse_duration: %02x %02x %02x\n", buf[0], buf[1], buf[2]);
	if(buf[0] == 'I' && buf[1] == 'D' && buf[2] == '3') {//带有ID3v2头
		RMuint32 id3v2Size;

		id3v2Size = (((RMuint32)(buf[6]&0x7f)) << 21) + 
					(((RMuint32)(buf[7]&0x7f)) << 14) + 
					(((RMuint32)(buf[8]&0x7f)) << 7) + 
					((RMuint32)(buf[9]&0x7f)) + ((buf[5]&0x10)?20:10);
		buf += id3v2Size;
		plog("mp3_parse_duration: id3v2Size=%dl\n", id3v2Size);
	}
	if(buf[0] != 0xff || (buf[1] & 0xe0) != 0xe0) {
		plog("mp3_parse_duration: Frame sync error\n");
		return 0;
	}

	info.version = (buf[1] & 0x18) >> 3;
	info.layer = (buf[1] & 0x06) >> 1;
	info.protection = buf[1] & 0x01;
	info.bitrate = (buf[2] & 0xf0) >> 4;
	info.samrate = (buf[2] & 0x0c) >> 2;
	info.padding = (buf[2] & 0x02) >> 1;
	info.channel = (buf[3] & 0xc0) >> 6;

	if(info.layer == 0) {
		plog("mp3_parse_duration: layer error\n");
		return 0;
	}
	info.layer = 3 - info.layer;

	if(info.version == 1) {
		plog( "mp3_parse_duration: version error\n");
		return 0;
	}

	if(info.version == 0 || info.version == 2) {
		bitrate = bitrate_sss[info.bitrate][1][info.layer] * 1000;
	} else {
		bitrate = bitrate_sss[info.bitrate][0][info.layer] * 1000;
	}
	if(bitrate == 0) {
		plog( "mp3_parse_duration: bitrate error\n");
		return 0;
	}

	if(info.version == 0) {
		samrate = samrate_ss[info.samrate][2];
		samples = samples_ss[info.layer][2];
	} else if(info.version == 2) {
		samrate = samrate_ss[info.samrate][1];
		samples = samples_ss[info.layer][1];
	} else {
		samrate = samrate_ss[info.samrate][0];
		samples = samples_ss[info.layer][0];
	}
	if(samrate == 0) {
		plog( "mp3_parse_duration: samrate error\n");
		return 0;
	}

	if(info.protection)
		info.protection = 0;
	else
		info.protection = 1;
	
	if(info.version == 3) {//mpeg1
		plog( "mp3_parse_duration: mpeg1 %d\n", info.channel);
		if(info.channel == 3)
			mark = buf + 21;
		else
			mark = buf + 36;
	} else {
		plog( "mp3_parse_duration: mpeg2 %d\n", info.channel);
		if(info.channel == 3)
			mark = buf + 13;
		else
			mark = buf + 21;
	}
	if(memcmp(mark, "Xing", 4) == 0) {
		plog( "mp3_parse_duration: VBR\n");

		frames = (((RMuint32)mark[8]) << 24) + 
					(((RMuint32)mark[9]) << 16) + 
					(((RMuint32)mark[10]) << 8) + 
					((RMuint32)mark[11]);
		duration = frames * samples / samrate;
		plog( "mp3_parse_duration: frames=%dl, duration=%dl\n",
				frames, duration);
	} else {
		plog( "mp3_parse_duration: CBR\n");
		plog( "mp3_parse_duration: version=%d, samples=%dl, bitrate=%dl, samrate=%dl padding=%d\n",
				info.version, samples, bitrate, samrate, info.padding);
		frameSize = (((samples / 8) * bitrate) / samrate) + info.padding;
		plog( "mp3_parse_duration: frameSize=%dl\n", frameSize);
		duration = (RMuint32)(fileSize * 8 / bitrate);
		if(0 && loops < 10) {
			plog( "\nREPEAT\n");
			loops ++;
			buf += frameSize;
			goto LOOP;
		}
	}
	return duration;
}

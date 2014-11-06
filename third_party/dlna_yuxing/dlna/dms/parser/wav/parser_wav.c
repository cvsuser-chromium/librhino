
#include <stdio.h>
#include <stdlib.h>
#include<string.h>

#include "AvMediaInfo.h"
#include "parser_wav.h"

extern int plog(char *fmt, ...);
int format_is_wav(unsigned char *buffer, unsigned long bytes, MEDIA_INFO* the_media_info)
{
	unsigned char* p_buffer	= buffer;
	unsigned long i	= 0;

	while(i < 100) {
		if ( (*p_buffer == 'R') && (*(p_buffer+1) == 'I') && (*(p_buffer+2) == 'F') && (*(p_buffer+3) == 'F') ) {
			if (  (*(p_buffer+8) == 'W') && (*(p_buffer+9) == 'A') && (*(p_buffer+0x0a) == 'V')  && (*(p_buffer+0x0b) == 'E') 
				&& (*(p_buffer+0x0c) == 'f')  && (*(p_buffer+0x0d) == 'm')  && (*(p_buffer+0x0e) == 't')  && (*(p_buffer+0x0f) == ' ')  ) {
				WAVE_FORMAT*	wave_fmt;
				printf("confirm this media is wav.....\n");

				wave_fmt	= (WAVE_FORMAT*)(p_buffer + 0x14);
				//the_media_info->info.audioInfo	= wav_param;
				the_media_info->info.audioInfo->nChannels	= wave_fmt->wChannels;
				the_media_info->info.audioInfo->nSamplesPerSec	= wave_fmt->dwSamplesPerSec;
				the_media_info->info.audioInfo->wBitsPerSample	= wave_fmt->wBitsPerSample;
				plog("{{{{{{{{{{{{{{{{{{{lpcm:channel:%ul,samplePerSec:%ul, Bits:%ul }}}}}}}}}}}}}}}\n", 
						the_media_info->info.audioInfo->nChannels, the_media_info->info.audioInfo->nSamplesPerSec, 
						the_media_info->info.audioInfo->wBitsPerSample);

				return 1;
			} else {
				p_buffer++;
				i++;
			}
		} else {
			p_buffer++;
			i++;
		}
	}
	
	return 0;
}

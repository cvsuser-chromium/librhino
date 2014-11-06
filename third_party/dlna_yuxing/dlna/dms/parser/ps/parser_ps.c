#include <stdio.h>
#include <stdlib.h>
#include<string.h>

#include "AvMediaInfo.h"

#include "parser_ps.h"

extern int plog(char *fmt, ...);
int format_is_ps(unsigned char *buffer, unsigned long bytes, MEDIA_INFO* the_media_info)
{
	unsigned char* p_buffer	= buffer;
	unsigned long i	= 0;

	while (i < 1024) {
		//printf("0x%x\t", *p_buffer);
		if ( (*p_buffer == 0x00) && (*(p_buffer+1) == 0x00) && (*(p_buffer+2) == 0x01) && (*(p_buffer+3) == 0xBA) ) {
			plog("confirm this media is ps.....\n");

			return 1;
		} else {
			p_buffer++;
			i++;			
		}
	}
	
	return 0;
}

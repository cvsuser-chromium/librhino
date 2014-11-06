/*****************************************************************************
 * asf.c: MMS access plug-in
 *****************************************************************************
 * Copyright (C) 2001-2004 the VideoLAN team
 * $Id: asf.c 13905 2006-01-12 23:10:04Z dionoea $
 *
 * Authors: Laurent Aimar <fenrir@via.ecp.fr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/
#include <stdlib.h>
#include <string.h>

#include "AvMediaInfo.h"
#include "parser_wmv.h"

#   define __MIN(a, b)   ( ((a) < (b)) ? (a) : (b) )

typedef struct
{
    uint8_t *p_data;    // pointer on data
    int     i_data;     // number of bytes set in p_data

    int    i_size;     // size of p_data memory allocated
} var_buffer_t;

void var_buffer_free( var_buffer_t *p_buf );
void      var_buffer_initread( var_buffer_t *p_buf, void *p_data, int i_data );
uint8_t   var_buffer_get8 ( var_buffer_t *p_buf );
uint16_t  var_buffer_get16( var_buffer_t *p_buf );
uint32_t  var_buffer_get32( var_buffer_t *p_buf );
uint64_t  var_buffer_get64( var_buffer_t *p_buf );
int       var_buffer_getmemory ( var_buffer_t *p_buf, void *p_mem, int64_t i_mem );
int       var_buffer_readempty( var_buffer_t *p_buf );
void      var_buffer_getguid( var_buffer_t *p_buf, guid_t *p_guid );
static inline uint64_t GetQWLE( void const * _p );
static inline uint32_t GetDWLE( void const * _p );
 static inline uint16_t GetWLE( void const * _p );
int parser_wma_duration(void);
static int CmpGuid( const guid_t *p_guid1, const guid_t *p_guid2 )
{
    return( ( p_guid1->v1 == p_guid2->v1 &&
              p_guid1->v2 == p_guid2->v2 &&
              p_guid1->v3 == p_guid2->v3 &&
              p_guid1->v4[0] == p_guid2->v4[0] &&
              p_guid1->v4[1] == p_guid2->v4[1] &&
              p_guid1->v4[2] == p_guid2->v4[2] &&
              p_guid1->v4[3] == p_guid2->v4[3] &&
              p_guid1->v4[4] == p_guid2->v4[4] &&
              p_guid1->v4[5] == p_guid2->v4[5] &&
              p_guid1->v4[6] == p_guid2->v4[6] &&
              p_guid1->v4[7] == p_guid2->v4[7] ) ? 1 : 0 );
}

static inline uint16_t GetWLE( void const * _p )
{
	uint8_t * p = (uint8_t *)_p;
	return ( ((uint16_t)p[1] << 8) | p[0] );
}
static inline uint32_t GetDWLE( void const * _p )
{
	uint8_t * p = (uint8_t *)_p;
	return ( ((uint32_t)p[3] << 24) | ((uint32_t)p[2] << 16)
              	| ((uint32_t)p[1] << 8) | p[0] );
}
static inline uint64_t GetQWLE( void const * _p )
{
	uint8_t * p = (uint8_t *)_p;
	return ( ((uint64_t)p[7] << 56) | ((uint64_t)p[6] << 48)
              	| ((uint64_t)p[5] << 40) | ((uint64_t)p[4] << 32)
              	| ((uint64_t)p[3] << 24) | ((uint64_t)p[2] << 16)
              	| ((uint64_t)p[1] << 8) | p[0] );
}

void var_buffer_free( var_buffer_t *p_buf )
{
	if( p_buf->p_data )
    	{
      		free( p_buf->p_data );
    	}
    	p_buf->i_data = 0;
    	p_buf->i_size = 0;
}

void var_buffer_initread( var_buffer_t *p_buf, void *p_data, int i_data )
{
	p_buf->i_size = i_data;
	p_buf->i_data = 0;
    	p_buf->p_data = p_data;
}

uint8_t var_buffer_get8 ( var_buffer_t *p_buf )
{
    	uint8_t  i_byte;
    	if( p_buf->i_data >= p_buf->i_size )
    	{
        	return( 0 );
    	}
    	i_byte = p_buf->p_data[p_buf->i_data];
    	p_buf->i_data++;
    	return( i_byte );
}


uint16_t var_buffer_get16( var_buffer_t *p_buf )
{
	uint16_t i_b1, i_b2;

	i_b1 = var_buffer_get8( p_buf );
	i_b2 = var_buffer_get8( p_buf );

	return( i_b1 + ( i_b2 << 8 ) );

}

uint32_t var_buffer_get32( var_buffer_t *p_buf )
{
    	uint32_t i_w1, i_w2;

    	i_w1 = var_buffer_get16( p_buf );
    	i_w2 = var_buffer_get16( p_buf );

    	return( i_w1 + ( i_w2 << 16 ) );
}

uint64_t var_buffer_get64( var_buffer_t *p_buf )
{
    	uint64_t i_dw1, i_dw2;

    	i_dw1 = var_buffer_get32( p_buf );
    	i_dw2 = var_buffer_get32( p_buf );

    	return( i_dw1 + ( i_dw2 << 32 ) );
}

int var_buffer_getmemory ( var_buffer_t *p_buf, void *p_mem, int64_t i_mem )
{
    	int i_copy;

    	i_copy = __MIN( i_mem, p_buf->i_size - p_buf->i_data );
    	if( i_copy > 0 && p_mem != NULL)
    	{
        	memcpy( p_mem, p_buf + p_buf->i_data, i_copy );
    	}
    	if( i_copy < 0 )
    	{
        	i_copy = 0;
    	}
    	p_buf->i_data += i_copy;
    	return( i_copy );
}

int var_buffer_readempty( var_buffer_t *p_buf )
{
    	return( ( p_buf->i_data >= p_buf->i_size ) ? 1 : 0 );
}

void var_buffer_getguid( var_buffer_t *p_buf, guid_t *p_guid )
{
    	int i;

    	p_guid->v1 = var_buffer_get32( p_buf );
    	p_guid->v2 = var_buffer_get16( p_buf );
    	p_guid->v3 = var_buffer_get16( p_buf );

    	for( i = 0; i < 8; i++ )
    	{
        	p_guid->v4[i] = var_buffer_get8( p_buf );
    	}
}

/*全局变量用来存放WMV的相关信息*/
asf_header_t hdr;
void asf_HeaderParse(uint8_t *p_header, int i_header )
{
	var_buffer_t 	buffer;
	guid_t 		guid;
    	uint64_t 		i_size;
    	int         		i;

	hdr.i_file_size = 0;
	hdr.i_data_packets_count = 0;
	hdr.i_min_data_packet_size = 0;
	for( i = 0; i < 6; i++ ){
		hdr.stream[i].i_cat = ASF_STREAM_UNKNOWN;
        	hdr.stream[i].i_selected = 0;
        	hdr.stream[i].i_bitrate = -1;
    	}

	var_buffer_initread( &buffer, p_header, i_header );

	var_buffer_getguid( &buffer, &guid );

	if( !CmpGuid( &guid, &asf_object_header_guid ) ) {
		printf("xxx error asf_object_header_guid xxx\n");
    	}
    	var_buffer_getmemory( &buffer, NULL, 30 - 16 );

    	for( ;; ){
		var_buffer_getguid( &buffer, &guid );
		i_size = var_buffer_get64( &buffer );

        	if( CmpGuid( &guid, &asf_object_file_properties_guid ) ){
            		var_buffer_getmemory( &buffer, NULL, 16 );
            		hdr.i_file_size            =  GetQWLE(buffer.p_data+buffer.i_data);
            		printf("xxx i_file_size =%lld xxx\n",hdr.i_file_size);
            		var_buffer_getmemory( &buffer, NULL, 16 );
            		hdr.i_data_packets_count   = var_buffer_get64( &buffer );
            		hdr.i_time_duration = GetQWLE(buffer.p_data+buffer.i_data);
            		printf("xxx time_duration =%lld xxx\n",hdr.i_time_duration);
            		var_buffer_getmemory( &buffer, NULL, 8+8+8+4);
            		hdr.i_min_data_packet_size = var_buffer_get32( &buffer );						
            		var_buffer_getmemory( &buffer, NULL, i_size - 24 - 16 - 8 - 8 - 8 - 8-8-8-4 - 4);
        	}
        	else if( CmpGuid( &guid, &asf_object_header_extension_guid ) ){
            		/* Enter it */
            		var_buffer_getmemory( &buffer, NULL, 46 - 24 );
        	}
		else if( CmpGuid( &guid, &asf_object_extended_stream_properties ) ){
            		/* Grrrrrr */
           		int16_t i_count1, i_count2;
            		int i_subsize;
            		int i;

            		var_buffer_getmemory( &buffer, NULL, 84 - 24 );

            		i_count1 = var_buffer_get16( &buffer );
            		i_count2 = var_buffer_get16( &buffer );

            		i_subsize = 88;
            		for( i = 0; i < i_count1; i++ ){
                		int i_len;
                		var_buffer_get16( &buffer );
                		i_len = var_buffer_get16( &buffer );
                		var_buffer_getmemory( &buffer, NULL, i_len );
                		i_subsize = 4 + i_len;
            		}

            		for( i = 0; i < i_count2; i++ ){
                		int i_len;
                		var_buffer_getmemory( &buffer, NULL, 16 + 2 );
                		i_len = var_buffer_get32( &buffer );
                		var_buffer_getmemory( &buffer, NULL, i_len );

                		i_subsize += 16 + 6 + i_len;
            		}

            		printf( "extended stream properties left=%d\n",
                    							i_size - i_subsize );

            		if( i_size - i_subsize <= 24 ){
                		var_buffer_getmemory( &buffer, NULL, i_size - i_subsize );
            		}
            		/* It's a hack we just skip the first part of the object until
             		* the embed stream properties if any (ugly, but whose fault ?) */
        	}
        	else if( CmpGuid( &guid, &asf_object_stream_properties_guid ) ){
            		int     i_stream_id;
            		guid_t  stream_type;
            		var_buffer_getguid( &buffer, &stream_type );
            		var_buffer_getmemory( &buffer, NULL, 32 );
            		i_stream_id = var_buffer_get8( &buffer ) & 0x7f;

            		var_buffer_getmemory( &buffer, NULL, i_size - 24 - 32 - 16 - 1);

            		if( CmpGuid( &stream_type, &asf_object_stream_type_video ) ){
               		printf("xxx video stream[%d] found xxx\n", i_stream_id );
                		hdr.stream[i_stream_id].i_cat = ASF_STREAM_VIDEO;
            		} 
            		else if( CmpGuid( &stream_type, &asf_object_stream_type_audio ) ){
                		printf("xxx audio stream[%d] found\n", i_stream_id );
                		hdr.stream[i_stream_id].i_cat = ASF_STREAM_AUDIO;
            		}
            		else{		
				printf("xxxxunkown xxx\n");
                		hdr.stream[i_stream_id].i_cat = ASF_STREAM_UNKNOWN;
            		}
        	}
        	else if ( CmpGuid( &guid, &asf_object_bitrate_properties_guid ) ){
            		int     i_count;
            		uint8_t i_stream_id;

            		//fprintf( stderr, "bitrate properties\n" );

            		i_count = var_buffer_get16( &buffer );
            		i_size -= 2;
            		while( i_count > 0 )
            		{
                		i_stream_id = var_buffer_get16( &buffer )&0x7f;
                		hdr.stream[i_stream_id].i_bitrate =  var_buffer_get32( &buffer );
                		printf("xxx i_bitrate =%d xxx\n",hdr.stream[i_stream_id].i_bitrate);
                		i_count--;
                		i_size -= 6;
            		}
            		var_buffer_getmemory( &buffer, NULL, i_size - 24 );
        	}
        	else{
            		var_buffer_getmemory( &buffer, NULL, i_size - 24 );
        	}

	if( var_buffer_readempty( &buffer ) )
            return;
	}
}

#if 0
typedef struct
{
    int i_count;
    char **ppsz_name;
    char **ppsz_value;
} asf_object_extended_content_description_t;

asf_object_extended_content_description_t *p_ec;
static int ASF_ReadObject_extended_content_description( char *buffer)
{
    uint8_t *p_peek = buffer, *p_data;
    int i_peek;
    int i;



    p_data = &p_peek[24];

    p_ec->i_count = GetWLE( p_data ); p_data += 2;
    p_ec->ppsz_name = calloc( sizeof(char*), p_ec->i_count );
    p_ec->ppsz_value = calloc( sizeof(char*), p_ec->i_count );
    for( i = 0; i < p_ec->i_count; i++ )
    {
        int i_size;
        int i_type;
        int i_len;
#define GETSTRINGW( psz_str, i_size ) \
       psz_str = calloc( i_size/2 + 1, sizeof( char ) ); \
       for( i_len = 0; i_len < i_size/2; i_len++ ) \
       { \
           psz_str[i_len] = GetWLE( p_data + 2*i_len ); \
       } \
       psz_str[i_size/2] = '\0';

        i_size = GetWLE( p_data ); p_data += 2;
        GETSTRINGW( p_ec->ppsz_name[i], i_size );
        p_data += i_size;

        /* Grrr */
        i_type = GetWLE( p_data ); p_data += 2;
        i_size = GetWLE( p_data ); p_data += 2;
        if( i_type == 0 )
        {
            GETSTRINGW( p_ec->ppsz_value[i], i_size );
        }
        else if( i_type == 1 )
        {
            int j;
            /* Byte array */
            p_ec->ppsz_value[i] = malloc( 2*i_size + 1 );
            for( j = 0; j < i_size; j++ )
            {
                static const char hex[16] = "0123456789ABCDEF";
                p_ec->ppsz_value[i][2*j+0] = hex[p_data[0]>>4];
                p_ec->ppsz_value[i][2*j+1] = hex[p_data[0]&0xf];
            }
            p_ec->ppsz_value[i][2*i_size] = '\0';
        }
        else if( i_type == 2 )
        {
            /* Bool */
            p_ec->ppsz_value[i] = strdup( *p_data ? "true" : "false" );
        }
        else if( i_type == 3 )
        {
            /* DWord */
            asprintf( &p_ec->ppsz_value[i], "%d", GetDWLE(p_data));
        }
        else if( i_type == 4 )
        {
            /* QWord */
            asprintf( &p_ec->ppsz_value[i], "%lld", GetQWLE(p_data));
        }
        else if( i_type == 5 )
        {
            /* Word */
            asprintf( &p_ec->ppsz_value[i], "%d", GetWLE(p_data));
        }
        else
            p_ec->ppsz_value[i] = NULL;

        p_data += i_size;
        


#undef GETSTRINGW

    }

#ifdef ASF_DEBUG
    msg_Dbg( s, "read \"extended content description object\"" );
    for( i = 0; i < p_ec->i_count; i++ )
        msg_Dbg( s, "  - '%s' = '%s'",
                 p_ec->ppsz_name[i],
                 p_ec->ppsz_value[i] );
#endif
    return 0;
}
#endif

int parser_wma_duration(void)
{
	return hdr.i_time_duration/10000000;
}
int parser_wmv(unsigned char *buffer, long bytes, MEDIA_INFO* the_media_info)
{
	asf_HeaderParse(buffer,bytes);
	//printf("xxxx icat =%d  %dxxx\n",hdr.stream[2].i_cat,hdr.stream[1].i_cat);
	//如果是WMA
	if( (hdr.stream[2].i_cat == ASF_STREAM_VIDEO) || (hdr.stream[1].i_cat == ASF_STREAM_VIDEO) ||(hdr.stream[0].i_cat == ASF_STREAM_VIDEO) 
			||(hdr.stream[3].i_cat == ASF_STREAM_VIDEO) ||(hdr.stream[4].i_cat == ASF_STREAM_VIDEO) ||(hdr.stream[5].i_cat == ASF_STREAM_VIDEO) ){
		printf("xxx it is wmv ok xxx\n");
		the_media_info->majorType	= X_MEDIA_VIDEO;
		the_media_info->otherType.videoType	= V_STREAM_VIDEO_WMV;		
	}else {
		printf("xxxit is  wma ok xxx\n");
		the_media_info->majorType			= X_MEDIA_AUDIO;
		the_media_info->otherType.audioType	= WAVE_FORMAT_WMA_AUDIO;
		the_media_info->fileSize 				= hdr.i_file_size;
		the_media_info->Time_Duration.tv_sec	= hdr.i_time_duration/10000000;
		the_media_info->Time_Duration.tv_usec 	= 0;
		the_media_info->Bitrate 				= hdr.stream[0].i_bitrate;
	}
	return 0;
}

/*WMV对文件头要求比较严格，所以简单分析时候只分析前面1024字节的文件头*/
int format_is_wmv(unsigned char *buffer, unsigned long bytes, MEDIA_INFO* the_media_info)
{
	unsigned char* p_buffer	= buffer;
	unsigned long  i			= 0;
	while(i<1024){
	if ( (*p_buffer == 0x30) && (*(p_buffer+1) == 0x26) && (*(p_buffer+2) == 0xb2) && (*(p_buffer+3) == 0x75) 
			&& (*(p_buffer+4) == 0x8e) && (*(p_buffer+5) == 0x66) && (*(p_buffer+6) == 0xcf) && (*(p_buffer+7) == 0x11) 
			&& (*(p_buffer+8) == 0xa6) && (*(p_buffer+9) == 0xd9) && (*(p_buffer+10) == 0x00) && (*(p_buffer+11) == 0xaa) 
			&& (*(p_buffer+12) == 0x00) && (*(p_buffer+13) == 0x62) && (*(p_buffer+14) == 0xce) && (*(p_buffer+15) == 0x6c) 
		) {
			printf("confirm this media is wmv and wma.....\n");
			return 1;
		}
	else{
		p_buffer ++;
			i ++;
		}
	}
	return 0;
}

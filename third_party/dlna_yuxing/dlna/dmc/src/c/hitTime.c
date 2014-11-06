/*-----------------------------------------------------------------------------------------------*/
/*
* Yuxing Software CONFIDENTIAL
* Copyright (c) 2003, 2011 Yuxing Corporation.  All rights reserved.
*
* The computer program contained herein contains proprietary information
* which is the property of Yuxing Software Co., Ltd.  The program may be used
* and/or copied only with the written permission of Yuxing Software Co., Ltd.
* or in accordance with the terms and conditions stipulated in the
* agreement/contract under which the programs have been supplied.
*
*    filename:			hitTime.c
*    author(s):			yanyongmeng@gmail.com
*    version:			1.0
*    date:				2011/2/12
* History
*/
/*-----------------------------------------------------------------------------------------------*/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>

#include "hitTime.h"

unsigned int g_ht_level = 0;
unsigned int g_ht_modules = 0;
int g_ht_delay = 0;

#ifdef	ENABLE_HT_PRINTF

static int s_ht_init = 0;
//static char s_ht_ip[32] = "";
//static int s_ht_port = 0;
static int s_ht_remote = 0;
static pthread_mutex_t s_ht_mutex;

static int ErrFileHnd = -1;
static int InfoFileHnd = -1;
//static const char *errFileName = "IUpnpErrFile.txt";
//static const char *infoFileName = "IUpnpInfoFile.txt";

static t_HT_PRINTF  s_ht_printf = NULL;

static int s_HT_AddHeadFlag(char *buf, int lineNo)
{
	int ms, s, m, h;
	struct timeval time_x = {0,0};

	gettimeofday( &time_x, NULL );
	ms = time_x.tv_usec/1000;
	s  = time_x.tv_sec%60;
	m  = (time_x.tv_sec%3600)/60;
	h  = (time_x.tv_sec/3600)%24;
	return sprintf(buf, "--[%2d:%2d:%2d:%3d %11d, %4d]--", h, m, s, ms, (int)pthread_self(), lineNo );
}

static void s_HT_StdOut (char* msg, int len)
{
	if( s_ht_printf )
		s_ht_printf(msg, len);
	else
		printf ("%s", msg);
}

static void s_HT_RemoteOut(char* msg, int len)
{
}

static unsigned int s_flags[32] = {0};
void HT_Printf(enum_HT_MODULES module, enum_HT_BITS Level, const char *fileName, int lineNo, const char *funcName,
				const char* fmt, ...)
{
	va_list ap;
	char buf[8192] = "";
	int len, rc;
//	unsigned int x;

	/* check if it is initialized */
	if( s_ht_init == 0 )
		return;
#if 0
	/* check if parameter is valid */
	if (!module || !Level)
		return;
	x = module--;
	if( x^module != (x+module) )
		return;
	x = Level--;
	if( x^Level != (x+Level) )
		return;
	/* check if it is available */
	if( !(module&g_ht_modules) || !(Level&g_ht_level) )
		return;
#endif
	if( module<0 || Level<0 || module>31 || Level>31 ) {
		return;
	}
//	printf("\n module = %d, s_flags[module] = %x, g_ht_modules = %x, s_flags[module]&g_ht_modules = %x\n", module, s_flags[module], g_ht_modules, s_flags[module]&g_ht_modules);
//	printf("\n Level = %d, s_flags[Level] = %x, g_ht_level = %x, s_flags[Level]&g_ht_level = %x\n", Level, s_flags[Level], g_ht_level, s_flags[Level]&g_ht_level);
	if( !(s_flags[module]&g_ht_modules) || !(s_flags[Level]&g_ht_level) ){
		return;
	}

	len = s_HT_AddHeadFlag(buf, lineNo);

	va_start (ap, fmt);
	rc = vsnprintf (buf + len, sizeof(buf) - len, fmt, ap);
	va_end (ap);

	if (rc < 1){
		return;
	}

	len += rc;
	pthread_mutex_lock(&s_ht_mutex);
	if( s_ht_remote )
		s_HT_RemoteOut(buf, len);
	else
		s_HT_StdOut (buf, len);
	pthread_mutex_unlock (&s_ht_mutex);

	if( g_ht_delay > 0 && g_ht_delay < 101 )
	{
		usleep( g_ht_delay *1000 );
	}
	else if(g_ht_delay < -1 && g_ht_delay > -101)
	{
		int delay = random()%(-g_ht_delay);
		if(delay)
			usleep( delay *1000 );
	}
	else
	{
	}
}
void HT_Printf_Start(enum_HT_MODULES module, enum_HT_BITS Level, const char *fileName, int lineNo, const char *funcName, int x, const char *s)
{

	char *str = (char *)s;
	HT_Printf(module, Level, fileName, lineNo, funcName, "%s start: %s %d.\n\r", funcName, str? str:"null", x );
}
void HT_Printf_End(enum_HT_MODULES module, enum_HT_BITS Level, const char *fileName, int lineNo, const char *funcName, int x, const char *s)
{
	char *str = (char *)s;
	HT_Printf(module, Level, fileName, lineNo, funcName, "%s   end: %s %d.\n\r", funcName, str? str:"null", x );
}
void HT_Printf_Middle(enum_HT_MODULES module, enum_HT_BITS Level, const char *fileName, int lineNo, const char *funcName, int x, const char *s)
{
	char *str = (char *)s;
	HT_Printf(module, Level, fileName, lineNo, funcName, "%s      : %s %d.\n\r", funcName, str? str:"null", x );
}


void HT_Initialize(t_HT_PRINTF func)
{
	int i;

	s_flags[0] = 1;
	for( i=1; i<32; i++ )
		s_flags[i] = s_flags[i-1]<<1;

	s_ht_printf = func;
	pthread_mutex_init(&s_ht_mutex, NULL);
	s_ht_init = 1;
}

void HT_EnableModule(enum_HT_MODULES module)
{
	if( module<0 || module>31)
		return;
	g_ht_modules |= s_flags[module];
}

void HT_EnableLevel(enum_HT_BITS level)
{
	if( level<0 || level>31)
		return;
	g_ht_level |= s_flags[level];
}


void HT_SetRemoteOut(char *serverIp, int port)
{
	if( !serverIp || (port < 1024) )
		return;

	s_ht_remote = 0;
}

static int s_HT_Open(char *filename)
{
	int fd = 0;

	fd = open (filename, O_NONBLOCK | O_SYNC | O_NDELAY);
	return fd;
}
void HT_SetLocalFileOut(char *infoFile, char *errFile)
{
	ErrFileHnd = s_HT_Open (infoFile);
	InfoFileHnd = s_HT_Open (infoFile);
}

void HT_SetDelay(int ms)
{
	g_ht_delay = ms;
}
#endif


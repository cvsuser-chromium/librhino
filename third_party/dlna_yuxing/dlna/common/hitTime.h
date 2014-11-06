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
*    filename:			hitTime.h
*    author(s):			yanyongmeng@gmail.com
*    version:			1.0
*    date:				2011/2/12
* History
*/
/*-----------------------------------------------------------------------------------------------*/
#ifndef _HIT_TIME_H_
#define _HIT_TIME_H_

typedef enum _ht_modules_ {
	HT_MOD_UPNP = 1,
	HT_MOD_DMC,
	HT_MOD_DMS,
	HT_MOD_DMR,

	HT_MOD_APP,
	HT_MOD_HYSDK,
	HT_MOD_IPC,
} enum_HT_MODULES;

typedef enum {
	HT_BIT_KEY = 1,

	HT_BIT_FEW,
	HT_BIT_MANY,
	HT_BIT_MYRIAD,
} enum_HT_BITS;

typedef int (*t_HT_PRINTF)(char *str, int len);

extern unsigned int g_ht_modules;   // if  ENABLE_DEBUG,  g_ht_modules will control the output of diffirent module
extern unsigned int g_ht_level;     // if  ENABLE_DEBUG,  g_ht_level will control the output of diffirent level
extern int g_ht_delay;

/*----------------------------------------------------------------------*/
#ifdef DLNA_ENABLE_LOG
#define ENABLE_HT_PRINTF            // general switch
#endif

#ifdef	ENABLE_HT_PRINTF
/*----------------------------------------------------------------------*/
#define HT_DBG_PRINTF(MODULE, LEVEL, FmtStr, ... ) \
			HT_Printf(MODULE, LEVEL, __FILE__, __LINE__, __FUNCTION__, FmtStr, ##__VA_ARGS__)

#define HT_DBG_FUNC_START(MODULE, LEVEL, x, str)  \
			enum_HT_MODULES _s_ht_module = MODULE; \
			enum_HT_BITS _s_ht_bit = LEVEL;	      \
			const char *_s_ht_file = __FILE__;	\
			const char *_s_ht_function_ = __FUNCTION__;	\
			HT_Printf_Start(MODULE, LEVEL, _s_ht_file, __LINE__, _s_ht_function_,  x, str )

#define HT_DBG_FUNC_END(x, str)	\
			HT_Printf_End(_s_ht_module, _s_ht_bit, _s_ht_file, __LINE__, _s_ht_function_, x, str)

#define HT_DBG_FUNC( x, str)	\
			HT_Printf_Middle(_s_ht_module, _s_ht_bit, _s_ht_file, __LINE__, _s_ht_function_,  x, str )
#ifdef __cplusplus
  extern "C" {
#endif /* __cplusplus */

void HT_Initialize(t_HT_PRINTF func);
void HT_EnableModule(enum_HT_MODULES module);
void HT_EnableLevel(enum_HT_BITS level);
void HT_SetRemoteOut(char *serverIp, int port);
void HT_SetLocalFileOut(char *infoFile, char *errFile);
void HT_SetDelay(int ms);

void HT_Printf(enum_HT_MODULES module, enum_HT_BITS Level, const char *fileName, int lineNo, const char *funcName, const char* fmt, ...);
void HT_Printf_Start(enum_HT_MODULES module, enum_HT_BITS Level, const char *fileName, int lineNo, const char *funcName, int x, const char *str);
void HT_Printf_End(enum_HT_MODULES module, enum_HT_BITS Level, const char *fileName, int lineNo, const char *funcName, int x, const char *str);
void HT_Printf_Middle(enum_HT_MODULES module, enum_HT_BITS Level, const char *fileName, int lineNo, const char *funcName, int x, const char *str);
/*----------------------------------------------------------------------*/
#ifdef __cplusplus
}
#endif /* __cplusplus */

#else
#ifdef __cplusplus
  extern "C" {
#endif /* __cplusplus */

/*----------------------------------------------------------------------*/
#define HT_Initialize(func)							{}
#define HT_EnableModule(module)						{}
#define HT_EnableLevel(level)						{}
#define HT_SetRemoteOut(ip, port)					{}
#define HT_SetLocalFileOut(infoFile, errFile)   	{}
#define HT_SetDelay(ms)   	                        {}

#define HT_DBG_PRINTF(MODULE, LEVEL, FmtStr, ...)		{}
#define HT_DBG_FUNC_START(MODULE, LEVEL, x, str)	{}
#define HT_DBG_FUNC_END(x, str)						{}
#define HT_DBG_FUNC(x, str)							{}
/*----------------------------------------------------------------------*/
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif


#endif /* _HIT_TIME_H_ */


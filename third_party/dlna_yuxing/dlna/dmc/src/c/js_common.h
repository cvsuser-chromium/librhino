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
*    filename:			js_common.h
*    author(s):			yanyongmeng
*    version:			0.1
*    date:				2011/2/12
* History
*/
/*-----------------------------------------------------------------------------------------------*/
#ifndef __JS_COMMON_H__
#define __JS_COMMON_H__

#include "json.h"

extern struct json_object* Dlna_json_object_new_string(const char *str);
extern struct json_object* Dlna_CreateJson(void);
extern int Dlna_Json_Object_Add_String(json_object*json, char *keyword, const char *value);
extern int Dlna_Json_Object_Add_Int(json_object*json, char *keyword, int x);
extern int Dlna_CheckStringAndReturn(char *str, char *buf, int len);
extern char* Dlna_FindCharAndInc(const char *s, char c);
extern char* Dlna_GetStringBuf(int *len);


#endif /*__JS_COMMON_H__ */


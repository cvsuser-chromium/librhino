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
*    filename:			ContentDirectory.js
*    author(s):			yanyongmeng
*    version:			0.1
*    date:				2011/2/12
* History
*/
/*-----------------------------------------------------------------------------------------------*/

#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <stdio.h>

#include "js_common.h"
#include "hitTime.h"

#define JSON_BUFFER_LENGTH		1024*16

#define STRING_NULL ""
#define STRING_END	'\0'


static char s_json_string[JSON_BUFFER_LENGTH+2] = "";

struct json_object* Dlna_CreateJson(void)
{
	json_object *json = json_object_new_object();
	return json;
}

struct json_object* Dlna_json_object_new_string(const char *str)
{
	return json_object_new_string( str? str : STRING_NULL);
}

int Dlna_Json_Object_Add_String(json_object*json, char *keyword, const char *value)
{
    if( !json || !keyword || !value)
        return -1;

    json_object_object_add(json, keyword, Dlna_json_object_new_string(value));
    return 0;
}
int Dlna_Json_Object_Add_Int(json_object*json, char *keyword, int x)
{
    if( !json || !keyword )
        return -1;

    json_object_object_add(json, keyword, json_object_new_int(x));
    return 0;
}

int Dlna_CheckStringAndReturn(char *string, char *buf, int len)
{
	int ret = -1;
	int slen = strlen( string );

	if( slen <  len )
	{
		strcpy(buf, string);
		ret = 0;
	}
	return ret;
}
char* Dlna_GetStringBuf(int *len)
{
	if( *len > JSON_BUFFER_LENGTH )
		*len = JSON_BUFFER_LENGTH;
	s_json_string[0] = 0;
	return s_json_string;
}

char* Dlna_FindCharAndInc(const char *s, char c)
{
	char *find;
	
	if( s == NULL )
		return NULL;

	find = strchr(s,c);
	if( find )
	{
		*find = 0;
		find++;
	}
	return find;
}


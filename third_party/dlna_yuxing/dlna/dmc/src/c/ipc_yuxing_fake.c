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
*    filename:			js_api.c
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
#include <pthread.h>

#include "semaphore.h"
#include "js_common.h"
#include "js_eventqueue.h"

#include "dlna_api.h"
#include "dlna_api_raw.h"
#include "dlna_api_ipc.h"

#include "LinkedList.h"
#include "ipc_common.h"
#include "hitTime.h"
#include "coo.h"


typedef struct _fake_dlna_object_s_
{
	int 	handle;
	int		IsPlayback;
}t_FAKE_OBJ;

typedef enum _function_name_list_
{
	FUNC_Dlna_JsRead = 0,
	FUNC_Dlna_JsWrite,
	
	FUNC_SENDER_MAX,
}t_FUNC_NAME;

static sem_t			s_object_lock;
static LinkedList		s_object_list;
static t_IPC_RCV_FUNC	s_ipc_yxfake_server_side_func[FUNC_SENDER_MAX];

/*-------------------------------------------------*/
static void s_ipc_call_send(int func_index, t_IPC_DATA *send)
{
	Ipc_CallAndReturn(g_ipc_client, enum_IPC_MOD_YUXING_FAKE, func_index, send);
}

static int IpcMode_Dlna_JsRead_ex( const char *name, char *buf, int len)
{
	t_IPC_DATA data = {0};

	Ipc_Data_SetInt(&data,  &len);
	Ipc_Data_SetString(&data,  (char*)name);
	Ipc_Data_SetString(&data,  buf);
	
	s_ipc_call_send( FUNC_Dlna_JsRead, &data);

	char *s = Ipc_Data_GetFirstString(&data);
	int slen = data.xxbuf_len[0];
	if( buf && s && (slen <= len) )
		memcpy(buf, s, slen);	
	return 0;
}
static int IpcRcv_Dlna_JsRead(int hnd, int func_index, t_IPC_DATA *data)
{
	int len = Ipc_Data_GetInt(data);//len = 4096
	char vbuf[len+10];
	strcpy(vbuf, Ipc_Data_GetString(data, 3));
	Raw_Dlna_JsRead(Ipc_Data_GetString(data, 2), vbuf, len);

	t_IPC_DATA ack = {0};
	Ipc_Data_SetString(&ack,  vbuf);
	Ipc_Ack_Value(hnd, &ack);
	return 1;
}


static int IpcMode_Dlna_JsWrite_ex( const char *name, char *buf, int len) 
{
	t_IPC_DATA data = {0};

	Ipc_Data_SetInt(&data,  &len);
	Ipc_Data_SetString(&data,  (char*)name);
	Ipc_Data_SetString(&data,  buf);
	
	s_ipc_call_send( FUNC_Dlna_JsWrite, &data);
	return 0;
}
static int IpcRcv_Dlna_JsWrite(int hnd, int func_index, t_IPC_DATA *data)
{
	Raw_Dlna_JsWrite(Ipc_Data_GetString(data, 2), Ipc_Data_GetString(data, 3), Ipc_Data_GetInt(data));
	return 0;
}


/*-------------------------------------------------*/
static void s_Add_Object(char *buf, int IsPlayback)
{
	HT_DBG_FUNC_START(HT_MOD_IPC, HT_BIT_MANY, 0, buf);
	
	t_FAKE_OBJ *p = (t_FAKE_OBJ*)malloc(sizeof(t_FAKE_OBJ));
//	HT_DBG_FUNC(p, "p = ");
	p->IsPlayback	= IsPlayback;
//	HT_DBG_FUNC(p, "p = ");
	p->handle		= atoi(buf);
//	HT_DBG_FUNC(p, "p = ");
	ListAddTail(&s_object_list, p);
//	HT_DBG_FUNC(p, "p = ");
	
	sprintf(buf, "%d", (int)p);
	
	HT_DBG_FUNC_END((int)p, "fake = ");
}

static void s_Delete_Object(t_FAKE_OBJ *fake_hnd)
{
	ListNode *node = ListHead(&s_object_list);
	
	while( node )
	{
		if( node->item == (void*)fake_hnd )
		{
			free(fake_hnd);
			ListDelNode(&s_object_list, node, 0);
			break;
		}
		node = ListNext(&s_object_list, node);
	}
}

static int s_find_fake_hnd(int real_hnd)
{
	ListNode *node = ListHead(&s_object_list);
	
	while( node )
	{
		t_FAKE_OBJ *p = (t_FAKE_OBJ*)(node->item);
		if( p->handle == real_hnd )
			return (int)p;
		node = ListNext(&s_object_list, node);
	}
	return 0;
}

static int s_Check_IsJsPlayback( const char *name, char *buf, int len, char *out, int *IsCreated)
{
	char *identifier = "DLNA.";
	int	playback = -1;

	*IsCreated = 0;
	
	HT_DBG_FUNC_START(HT_MOD_IPC, HT_BIT_MANY, 0, name);
	if( strncasecmp(name, identifier, strlen(identifier)) == 0 )
	{
		char *dot1 = Dlna_FindCharAndInc(name, '.'); 
		char *dot2 = Dlna_FindCharAndInc(dot1, '.'); 
		char *dot3 = Dlna_FindCharAndInc(dot2, '.'); 
		
		t_FAKE_OBJ *fake = (t_FAKE_OBJ*)atoi(dot1);
		if(fake)
		{
			playback = fake->IsPlayback;
			sprintf(out, "%s%d.%s.%s", identifier, fake->handle, dot2, dot3);
			
			if( strcmp(dot2, "Release") == 0 )
				s_Delete_Object(fake);
		}
		else
		{
			char *pclass = "Class";
			int slen = strlen(pclass);
			char *p = dot2 + slen;
			
			playback = 1;
			if( strncasecmp(dot2, pclass, slen) == 0 )
			{
				*IsCreated = 1;
				if( strcmp(p,"AVPlayer") && strcmp(p,"ImagePlayer") && strcmp(p,"RenderingControl") )
					playback = 0;
			}
			else if( strcmp(dot2, "GetEvent") == 0 )
			{
				playback = 2;
				
				if( Js_EventQueue_GetEvent(buf, len) )
				{
					sprintf(out, "%s%d.%s.%s", identifier, 0, dot2, dot3);
					IpcMode_Dlna_JsRead_ex(out, buf, len);
				}

				json_object *old_json = dlna_json_tokener_parse(buf);
				if( old_json )
				{
					int hnd = json_object_get_int( json_object_object_get(old_json, "hnd") );
					const char *event = json_object_get_string( json_object_object_get(old_json, "event") );

					json_object *new_json = json_object_new_object();
					json_object_object_add(new_json, "hnd", json_object_new_int(s_find_fake_hnd(hnd)));
					json_object_object_add(new_json, "event", Dlna_json_object_new_string(event));

					const char *str = json_object_to_json_string(new_json);
					if( strlen(str) < len )
						strcpy(buf, str);
					
					json_object_put(new_json);
					json_object_put(old_json);
				}
			}
			else
			{
			}
			
			if( playback != 2 )
				sprintf(out, "%s%d.%s.%s", identifier, 0, dot2, dot3);
		}
	}

	HT_DBG_FUNC_END(playback, "playback = ");
	return playback;
}

int IpcMode_Dlna_JsRead( const char *name, char *buf, int len)
{
	int ret=-1;
	char temp[1024];
	int  IsPlayback, IsCreated;
	
	HT_DBG_FUNC_START(HT_MOD_IPC, HT_BIT_MANY, len, buf);

	IsPlayback = s_Check_IsJsPlayback(name, buf, len, temp, &IsCreated);
	
	if( IsPlayback == 1 )
		ret = Raw_Dlna_JsRead(temp, buf, len);
	else if( IsPlayback == 0 )
		ret = IpcMode_Dlna_JsRead_ex(temp, buf, len);
	else
	{
	}

	if( IsCreated )
		s_Add_Object(buf, IsPlayback);

	HT_DBG_FUNC_END(ret, buf);
	return ret;
}

int IpcMode_Dlna_JsWrite( const char *name, char *buf, int len)
{
	int ret=-1;
	char temp[1024];
	int  IsPlayback, IsCreated;
	
	HT_DBG_FUNC_START(HT_MOD_IPC, HT_BIT_MANY, len, buf);

	IsPlayback = s_Check_IsJsPlayback(name, buf, len, temp, &IsCreated);
	
	if( IsPlayback == 1)
		ret = Raw_Dlna_JsWrite(temp, buf, len);
	else if( IsPlayback == 0 )
		ret = IpcMode_Dlna_JsWrite_ex(temp, buf, len);
	else
	{
	}
	
	if( IsCreated )
		s_Add_Object(buf, IsPlayback);

	HT_DBG_FUNC_END(ret, buf);
	return ret;
}	


/*-------------------------------------------------*/
//static int s_IpcRcv_EventCallback(enum_IPC_MODULE module, enum_DlnaEvent func_index, int handle, char *str, void *user)
int IpcYxFake_Client_Func(int hnd, int func_index, t_IPC_DATA *data)
{
	enum_DlnaEvent event_type = (enum_DlnaEvent)Ipc_Data_GetInt(data);
	int *handle = (int*)Ipc_Data_GetString(data, 2);
	int ret = -1;
	
	if( g_IPC_client_side_EventHandler )
	{
		int fake_hnd = s_find_fake_hnd(*handle);
		ret = g_IPC_client_side_EventHandler(enum_IPC_MOD_YUXING_FAKE, event_type, fake_hnd, Ipc_Data_GetString(data, 3));
	}
	
	t_IPC_DATA ack = {0};
	Ipc_Data_SetInt(&ack, &ret);
	Ipc_Ack_Value(hnd, &ack);
	return 1;
}

int IpcYxFake_Server_Func(int hnd, int func_index, t_IPC_DATA *data)
{
	HT_DBG_FUNC_START(HT_MOD_IPC, HT_BIT_FEW, func_index, "func_index = ");
	int ret = s_ipc_yxfake_server_side_func[func_index](hnd, func_index, data);
	HT_DBG_FUNC_END(ret, "ret = ");
	return ret;
}	

void IpcYxFake_Client_Init(void)
{
	sem_init(&(s_object_lock),0,1);
	ListInit(&(s_object_list), 0, 0);
}

void IpcYxFake_Server_Init(void)
{
	t_IPC_RCV_FUNC *list; 

	list = s_ipc_yxfake_server_side_func;
	xxx_register_func(list, FUNC_Dlna_JsRead,	IpcRcv_Dlna_JsRead);
	xxx_register_func(list, FUNC_Dlna_JsWrite,	IpcRcv_Dlna_JsWrite);
}


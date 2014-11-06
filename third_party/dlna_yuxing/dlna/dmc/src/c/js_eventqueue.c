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
#include "upnp.h"
#include "LinkedList.h"

#include "json.h"
#include "js_eventqueue.h"
#include "js_common.h"
#include "hitTime.h"


extern char *Dlna_strdup(const char *str);

static enum_DlnaEvent s_event_type = enum_DlnaEvent_JsonString;
static t_DLNA_EVENT_EX s_JS_EventHandler = NULL;

#ifdef PUT_EVENT_WITHOUT_CACHE
static int s_Js_EventQueue_Put(void *object, char *string);
#endif

static int s_EventQueue_Put(pClassEventQueue me, void *object, char *string)
{
	int ret = -1;

#ifdef PUT_EVENT_WITHOUT_CACHE
	ret = s_Js_EventQueue_Put(object, string);
#else
	sem_wait(&(me->lock));
	
	if( s_SendMsgToBrowser )
		ret = s_SendMsgToBrowser((int)object);
	if( ret == 0 )
		ListAddTail(&(me->list), Dlna_strdup(string));
	
	sem_post(&(me->lock));
#endif	
	return ret;
}

static char *s_EventQueue_Get(pClassEventQueue me)
{	
	ListNode * node;
	char *string = NULL;
	
	sem_wait(&(me->lock));
	
	node = ListHead(&(me->list));
	if( node )
	{
		string = (char *)(node->item);
		ListDelNode(&(me->list), node, 0);
	}
	
	sem_post(&(me->lock));

	return string;
}


pClassEventQueue EventQueue_Create(void)
{
	ClassEventQueue *hnd=NULL;
	int size = sizeof(ClassEventQueue);

	hnd = (ClassEventQueue*)malloc(size);
	memset(hnd,0,size);
	if( hnd )
	{
		hnd->Put				= s_EventQueue_Put;
		hnd->Get				= s_EventQueue_Get;
		
		sem_init(&(hnd->lock),0,1);
		ListInit(&(hnd->list), 0, 0);
	}
	
	return hnd;	
}

void EventQueue_Release(pClassEventQueue me)
{
	ListNode * node;
	if(me)
	{
		sem_wait(&(me->lock));
		
		node = ListHead(&(me->list));
		while(node)
		{
			ListDelNode(&(me->list), node, 0);
			node = ListHead(&(me->list));
		}
		
		ListDestroy( &(me->list), 0);
		
		sem_post(&(me->lock));
		sem_destroy( &(me->lock) );

		free(me);
	}
}


#ifdef PUT_EVENT_WITHOUT_CACHE
static sem_t		s_dlna_event_lock;
static LinkedList	s_dlna_event_hnd_list;
static LinkedList	s_dlna_event_msg_list;
static int s_Js_EventQueue_Put(void *object, char *string)
{
	int ret = -1;
	
	HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_MANY,(int)object, string);
	if( enum_DlnaEvent_JsonString == s_event_type )
	{
		sem_wait(&s_dlna_event_lock);
		
		if( s_JS_EventHandler )
			ret = s_JS_EventHandler(enum_IPC_MOD_HUAWEI, enum_DlnaEvent_JsonString, 0, string);
		
		sem_post(&s_dlna_event_lock);
	}
	else
	{
		sem_wait(&s_dlna_event_lock);
		
		if( s_JS_EventHandler )
			ret = s_JS_EventHandler(enum_IPC_MOD_YUXING_FAKE, enum_DlnaEvent_OnlyVirtualKey, (int)object, NULL);

		if( ret == 0 )
		{
	//		int *p = (int*)malloc(sizeof(int));
	//		*p = (int)object;
	//		ListAddTail(&s_dlna_event_msg_list, p);
			ListAddTail(&s_dlna_event_msg_list, object);
			ListAddTail(&s_dlna_event_msg_list, Dlna_strdup(string));
		}
		
		sem_post(&s_dlna_event_lock);
	}
	
	HT_DBG_FUNC_END(ret, NULL);
	return ret;
}


int Js_EventQueue_GetEvent_Ex(char *buf, int len, int (*func)(int object))
{
	ListNode * node;
	char *string = NULL;
	int ret = -1;
	
	HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_MANY,len, 0);
	sem_wait(&s_dlna_event_lock);
	
	node = ListHead(&s_dlna_event_msg_list);
	if( node )
	{	
		int hnd = (int)(node->item);
		ListDelNode(&s_dlna_event_msg_list, node, 0);
		
		node = ListHead(&s_dlna_event_msg_list);
		string = (char *)(node->item);
		
		json_object *json = json_object_new_object();
		int fake = hnd;
		if( func )
			fake = func(hnd);
		json_object_object_add(json, "hnd", json_object_new_int(fake));
		json_object_object_add(json, "event", Dlna_json_object_new_string(string));
		const char *str = json_object_to_json_string(json);
		if( strlen(str) < len )
			strcpy(buf, str);
		free(string);
		json_object_put(json);
		
		ListDelNode(&s_dlna_event_msg_list, node, 0);
		ret = 0;
	}
	
	sem_post(&s_dlna_event_lock);

	HT_DBG_FUNC_END(ret, buf);
	return ret;

}

int Js_EventQueue_GetEvent(char *buf, int len)
{
	return Js_EventQueue_GetEvent_Ex(buf, len , NULL);
}


#else

#if 0
int Js_EventQueue_GetHnd(char *buf, int len)
{
	ListNode * node;
	int hnd = -1;
	
	HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_MANY,len, NULL);
	sem_wait(&s_dlna_event_lock);
	
	node = ListHead(&s_dlna_event_msg_list);
	if( node )
	{
		hnd = (int)(node->item);
		ListDelNode(&s_dlna_event_msg_list, node, 0);
	}
	
	sem_post(&s_dlna_event_lock);

	HT_DBG_FUNC_END(hnd, NULL);
	return hnd;

}
#endif

#endif

t_DLNA_CALLBACK g_dlna_callback = NULL;
static int s_event_queue_init = 0;
void EventQueue_Module_Init(enum_DlnaAppMode app_mode, t_DLNA_CALLBACK callback, t_DLNA_EVENT_EX EventHandler, enum_DlnaEvent EventHandlerType)
{
	HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_FEW, s_event_queue_init, NULL);

	if( s_event_queue_init )
		return;
	
	s_event_type = EventHandlerType;
	s_JS_EventHandler = EventHandler;
	g_dlna_callback = callback;
	
#ifdef PUT_EVENT_WITHOUT_CACHE
	sem_init(&(s_dlna_event_lock),0,1);
	ListInit(&(s_dlna_event_hnd_list), 0, 0);
	ListInit(&(s_dlna_event_msg_list), 0, 0);
#endif
	s_event_queue_init = 1;
	HT_DBG_FUNC_END(0, 0);
}

void EventQueue_Module_Destroy(void)
{
	HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_FEW, 0, NULL);

	if( s_event_queue_init == 0 )
		return;
	
#ifdef PUT_EVENT_WITHOUT_CACHE
	sem_destroy(&(s_dlna_event_lock));
	ListDestroy(&(s_dlna_event_hnd_list), 0);
	ListDestroy(&(s_dlna_event_msg_list), 0);
#endif

	s_event_queue_init = 0;

	HT_DBG_FUNC_END(0, 0);
}



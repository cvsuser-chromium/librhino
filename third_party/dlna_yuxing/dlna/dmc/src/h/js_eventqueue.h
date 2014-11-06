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
*    filename:			js_eventqueue.h
*    author(s):			yanyongmeng
*    version:			0.1
*    date:				2011/2/12
* History
*/
/*-----------------------------------------------------------------------------------------------*/
#ifndef __JS_EVENT_LIST_H__
#define __JS_EVENT_LIST_H__

#include <semaphore.h>
#include "LinkedList.h"
#include "dlna_type.h"
#include "ipc_common.h"		// just for enum_IPC_MODULE

#define PUT_EVENT_WITHOUT_CACHE	1

typedef struct _event_list_s_ *pClassEventQueue;
typedef struct _event_list_s_ {
	int				(*Put)(pClassEventQueue me, void *object, char *string);
	char*			(*Get)(pClassEventQueue me);

	sem_t			lock;
	LinkedList		list;
}ClassEventQueue;

extern t_DLNA_CALLBACK g_dlna_callback;


void EventQueue_Module_Init(enum_DlnaAppMode app_mode, t_DLNA_CALLBACK callback, t_DLNA_EVENT_EX EventHandler, enum_DlnaEvent EventHandlerType);
extern void EventQueue_Module_Destroy(void);

extern pClassEventQueue EventQueue_Create(void);
extern void EventQueue_Release(pClassEventQueue me);

#ifdef PUT_EVENT_WITHOUT_CACHE
//extern int Js_EventQueue_Put(void *object, char *string);
extern int Js_EventQueue_GetEvent_Ex(char *buf, int len, int (*func)(int object));
extern int Js_EventQueue_GetEvent(char *buf, int len);
#endif

#endif /*__JS_EVENT_LIST_H__ */


/*
 * metadata.c : GeeXboX uShare CDS Metadata DB.
 * Originally developped for the GeeXboX project.
 * Copyright (C) 2005-2007 Benjamin Zores <ben@geexbox.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#include "LinkedList.h"
#include "hitTime.h"
#include "dms.h"


/*--------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------*/
#define WAIT_LOCK(dmsas)		sem_wait(&(dmsas->lock))
#define POST_LOCK(dmsas)		sem_post(&(dmsas->lock))

typedef struct _x_async_add_unit_t_ {
	int				parent_id;
	char			*keyword;
	int				user;

	int				del;
}x_AAU;

struct _c_dms_async_share_ {
	sem_t			lock;
	sem_t			bell;

	LinkedList		list_added;
	LinkedList		list_to_add;
	
	int				flag_adding;
    int             pause;

	int				quit;  

	t_DMS_ADD_FUNC	add_func;
};

static void* s_Dms_AsyncShare_CreateUnit(int parent_id, char *keyword, int user)
{
	x_AAU *p = COO_OBJECT_NEW(x_AAU);
	if( p )
	{
		p->keyword 	= strdup(keyword);
		p->parent_id= parent_id;
		p->user		= user;
	}
	return p;
}
static void s_Dms_AsyncShare_FreeUnit(void *fpid)
{
	x_AAU *p = (x_AAU *)fpid;
	if( p )
	{
		if(p->keyword)
			free(p->keyword);
		free(p);
	}
}

static ListNode *s_Dms_AsyncShare_FindContainer_Sub(LinkedList *list, int parent_id, char *keyword)
{
	ListNode * node = ListHead(list);
	while( node )
	{
		x_AAU *p = (x_AAU*)(node->item);
		if( (p->parent_id == parent_id) && coo_str_equal( p->keyword, keyword ) && (p->del==0))
			break;
		node = ListNext(list, node);
	}
	return node;
}

static int s_Dms_AsyncShare_FindContainer(C_DMS_AS *dmsas, int parent_id, char *keyword)
{
	if(s_Dms_AsyncShare_FindContainer_Sub(&(dmsas->list_added), parent_id, keyword))
		return 1;
	if(s_Dms_AsyncShare_FindContainer_Sub(&(dmsas->list_to_add), parent_id, keyword))
		return 2;
	return 0;
}

static void s_Dms_AsyncShare_AddingLoop(void *arg)
{
	C_DMS_AS *dmsas = (C_DMS_AS *)arg;
	LinkedList *list;
 	ListNode * node;
	x_AAU *p;
	int parent_id;

	HT_DBG_FUNC_START(HT_MOD_DMS, HT_BIT_KEY, (int)arg, 0);
	while(!(dmsas->quit))
	{
		sem_wait(&(dmsas->bell));
		
		WAIT_LOCK(dmsas);
		list = &(dmsas->list_to_add);
		node = ListHead(list);
		HT_DBG_FUNC((int)node, "new node = ");
		/* no new folder at all */
		if( !node )	
		{
			POST_LOCK(dmsas);
			continue;
		}

		p = (x_AAU*)(node->item);
		/* critical one, we just delete it*/
		HT_DBG_FUNC(p->parent_id, p->keyword);
		HT_DBG_FUNC(p->del, "first: p->del = ");
		if( p->del )
		{
			s_Dms_AsyncShare_FreeUnit(p);
			ListDelNode(list, node, 0);
			POST_LOCK(dmsas);
			continue;
		}

		/* set env & new an entry */
		dmsas->flag_adding	= 1;
		parent_id = p->parent_id;
		POST_LOCK(dmsas);

		/* ok, we can add one by one */
		dmsas->add_func(dmsas, parent_id, p->keyword, p->user);
		
		/* lastly, delete or move node? */
		WAIT_LOCK(dmsas);
		ListDelNode(list, node, 0);
		HT_DBG_FUNC(p->del, "second: p->del = ");
		if( p->del )
		{
		    Raw_Dms_VirtualFileTree_DeleteChild(Raw_Dms_VirtualFileTree_GetParentID(parent_id), parent_id);
			s_Dms_AsyncShare_FreeUnit(p);
		}
		else
		{
		    ListAddTail(&(dmsas->list_added), p);
		}

		dmsas->flag_adding = 2;
		POST_LOCK(dmsas);
	}

	HT_DBG_FUNC_END(0, 0);
}

int Dms_AsyncShare_RemoveObject(C_DMS_AS * dmsas, int parent_id, char *keyword)
{
	LinkedList *list;
	ListNode * node;
	x_AAU *p;
	int removed = 0;

	HT_DBG_FUNC_START(HT_MOD_DMS, HT_BIT_FEW, parent_id, keyword);
	
	WAIT_LOCK(dmsas);
	list = &(dmsas->list_to_add);
	node = ListHead(list);
	if( node )	
	{
		p = (x_AAU*)(node->item);
		if( (p->parent_id == parent_id) && coo_str_equal(p->keyword, keyword))//adding
		{
			dmsas->flag_adding = 0;
			p->del = 1;
			removed=2;
		}

		node = ListNext(list, node);
		while( node )
		{
			p = (x_AAU*)(node->item);
			if( (p->parent_id == parent_id) && coo_str_equal(p->keyword, keyword))//to add
			{
				s_Dms_AsyncShare_FreeUnit(p);
				ListDelNode(list, node, 0);
				removed+=3;
			}
			node = ListNext(list, node);
		}
		
		if( removed )
		{
			HT_DBG_FUNC(removed, "removed from to_add_list = ");
			goto s_EXIT;
		}
	}

	list = &(dmsas->list_added);
	node = s_Dms_AsyncShare_FindContainer_Sub(list, parent_id, keyword);
	if( node )	//added
	{
		p = (x_AAU*)(node->item);
		Raw_Dms_VirtualFileTree_DeleteChild(Raw_Dms_VirtualFileTree_GetParentID(parent_id), parent_id);
		s_Dms_AsyncShare_FreeUnit(p);
		ListDelNode(list, node, 0);
		removed = 1;
		goto s_EXIT;
	}

s_EXIT:
	POST_LOCK(dmsas);
	
	HT_DBG_FUNC_END(removed, 0);
	return removed;
}

int Dms_AsyncShare_AddObject(C_DMS_AS * dmsas, int parent_id, char *keyword, int user)
{
	HT_DBG_FUNC_START(HT_MOD_DMS, HT_BIT_FEW, parent_id, keyword);
	
	WAIT_LOCK(dmsas);
	int ret =  s_Dms_AsyncShare_FindContainer(dmsas, parent_id, keyword);
	if( !ret )
	{
		void *p = s_Dms_AsyncShare_CreateUnit(parent_id, keyword, user);
		if(p)
		{
			ListAddTail(&(dmsas->list_to_add), p);
			HT_DBG_FUNC((int)p, "post bell");
			sem_post(&(dmsas->bell));
		}
	}
	POST_LOCK(dmsas);
	
	HT_DBG_FUNC_END(0, 0);
	return ret;
}

int Dms_AsyncShare_Is_Aborted(C_DMS_AS * dmsas)
{
	return (dmsas->flag_adding == 0);
}

int Dms_AsyncShare_Is_Paused(C_DMS_AS * dmsas)
{
	return (dmsas->pause);
}

void Dms_AsyncShare_SetPause(C_DMS_AS * dmsas, int mode)
{
	WAIT_LOCK(dmsas);
	dmsas->pause = mode;
	POST_LOCK(dmsas);
}

C_DMS_AS *Dms_AsyncShare_Init (t_DMS_ADD_FUNC add_func)
{
	if(!add_func)
		return NULL;
	
	pthread_t	p_tid;
	C_DMS_AS *dmsas = COO_OBJECT_NEW(C_DMS_AS);
	
	sem_init(&(dmsas->lock),0,1);
	sem_init(&(dmsas->bell),0,0);
	ListInit(&(dmsas->list_added), 0, 0);
	ListInit(&(dmsas->list_to_add), 0, 0);

	dmsas->add_func	= add_func;
	
	pthread_create( &p_tid, NULL, (VFUNCV)s_Dms_AsyncShare_AddingLoop, dmsas);
	return dmsas;
}

/*--------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------*/


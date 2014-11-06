/*
filetree.c
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "mime.h"
#include "hitTime.h"
#include "dms.h"
#include "ushare.h"


struct upnp_entry_lookup_t {
	int id;
	C_DMS_VO *entry_ptr;
};

const C_DMS_CI Container_MIME_Type = {  "object.container.storageFolder"};
t_DMS_CMP_OBJECT s_pvr_entry_cmp = NULL;


/*--------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------*/
static int s_is_invalid_id(int id)
{
	if(id < 0)
		return true;
	else
		return false;
}

int Dms_VirtualFileTree_IsFull(void)
{
	return ut->total_entries > ut->object_total_limit;
}

static int s_Dms_VirtualFileTree_AddChildEx (int parent_id, int ca_index, C_DMS_VO *child)
{
	int ret;
	struct upnp_entry_lookup_t *entry_lookup_ptr = NULL;
	COO_ARRAY *ca;

	HT_DBG_FUNC_START(HT_MOD_DMS, HT_BIT_MYRIAD, (int)parent_id, "parent_id = ");

	C_DMS_VO *parent = Dms_VirtualFileTree_GetObject(parent_id);
	ret = e_IDE_INVALID_PARENT_ID;
	if(!parent || !Dms_VirtualObject_IsContainer(parent))
		goto s_EXIT;
	
	ret = e_IDE_GENERAL;
	if(ca_index < 0 || ca_index >= x_DMS_CHILDS_LISTS_NUM)
		goto s_EXIT;

	ret = Dms_VirtualObject_IsValid(child);
	if(ret)
		goto s_EXIT;

	ut->nr_entries++;
	ut->total_entries++;
	ut->systemUpdateId++;
	parent->updateID++;
	HT_DBG_FUNC(ut->total_entries ,"ut->total_entries = ");

	child->parent	= parent;
	child->id		= ut->starting_id + ut->nr_entries;
	child->updateID	= 0;
	
	C_DMS_CCI *container = (C_DMS_CCI *)(parent->object);
	HT_DBG_FUNC((int)container, "container = ");
	ca = container->childs_lists[ca_index];
	HT_DBG_FUNC((int)ca, "ca = ");
	if(!ca)
	{
		container->childs_lists[ca_index] = coo_array_create(2, 20);
		ca = container->childs_lists[ca_index];
	}
	coo_array_append(ca, child);
	HT_DBG_FUNC(coo_array_real_count(ca), "ca->real_count = ");

	if((parent->source_type != e_SOURCE_ROOT) && (parent->source_type != e_SOURCE_USB)&& s_pvr_entry_cmp)
	{
		int i, j, count;
		count = coo_array_real_count(ca);
		for( i = 0; i < count; i++ )
		{
			for(j=i+1; j<count; j++)	
			{
				if(s_pvr_entry_cmp(coo_array_get(ca, i), coo_array_get(ca, j)) > 0) 		
					coo_array_switch(ca, i, j);
			}
		}
		
		for( i = 0; i < count; i++ )
		{
			HT_DBG_FUNC( ((C_DMS_VO *)coo_array_get(ca, i))->id, "p->id = ");
		}		
	}

	entry_lookup_ptr = (struct upnp_entry_lookup_t *)malloc (sizeof (struct upnp_entry_lookup_t));
	entry_lookup_ptr->id = child->id;
	entry_lookup_ptr->entry_ptr = child;

	if (rbsearch ((void *) entry_lookup_ptr, ut->rb) == NULL)
	{
		HT_DBG_FUNC(0,"rbsearch = NULL");
	}

	ret = child->id;
	child = NULL;
	
s_EXIT:
	if(child)
		Dms_VirtualObject_Free(child);
	HT_DBG_FUNC_END(ret, "ret = ");
	return ret;
}

int Dms_VirtualFileTree_AddChildEx (int parent_id, int ca_index, C_DMS_VO *child)
{
	Dms_WaitLock();
	int id = s_Dms_VirtualFileTree_AddChildEx(parent_id, ca_index, child);
	Dms_PostLock();
	return id;
}
/* Seperate recursive free() function in order to avoid freeing off
 * the parents child list within the freeing of the first child, as
 * the only entry which is not part of a childs list is the root entry
 */
static void s_upnp_entry_free_recursive (struct ushare_t *ut, C_DMS_VO *entry)
{
	struct upnp_entry_lookup_t entry_lookup_ptr;
	if(!entry)
		return;

	HT_DBG_FUNC_START(HT_MOD_DMS, HT_BIT_MYRIAD, (int)entry, 0);

	if(Dms_VirtualObject_IsContainer(entry))
	{
		C_DMS_CCI *container = (C_DMS_CCI *)(entry->object);
		int num, i, count;

		for(num = 0; num < x_DMS_CHILDS_LISTS_NUM; num++)
		{
			COO_ARRAY *ca = container->childs_lists[num];
			if(!ca)
				continue;
			
			count = coo_array_real_count(ca);
			for(i = 0; i < count; i++)
			{
				s_upnp_entry_free_recursive (ut, coo_array_get(ca, i));
				entry->updateID++;
			}
		}
		
	}

	ut->systemUpdateId++;
	
	entry_lookup_ptr.id = entry->id;
	entry_lookup_ptr.entry_ptr = entry;
	if (rbdelete ((void *) (&entry_lookup_ptr), ut->rb) == NULL)
	{
		HT_DBG_FUNC( entry->id, "Failed to delete the RB lookup tree");
	}
	entry->FreeObject(entry->object);
	free(entry);
	ut->total_entries--;
	HT_DBG_FUNC_END(ut->total_entries, "ut->total_entries = ");
}

static void s_upnp_entry_delete_a_child (C_DMS_VO *parent, C_DMS_VO *child)
{
	if(!parent || !child)
		return;
	
	if(!Dms_VirtualObject_IsContainer(parent))
		return;
	
	C_DMS_CCI *container = (C_DMS_CCI *)(parent->object);
	int num, i, count;

	for(num = 0; num < x_DMS_CHILDS_LISTS_NUM; num++)
	{
		COO_ARRAY *ca = container->childs_lists[num];
		if(!ca)
			continue;

		count = coo_array_real_count(ca);
		for( i = 0; i < count; i++ )
		{
			C_DMS_VO *p = coo_array_get(ca, i);
			if(p == child)
			{
				s_upnp_entry_free_recursive(ut, p);
				coo_array_delete(ca, i);
				parent->updateID++;
			}
		}
	}
}

C_DMS_VO *Dms_VirtualFileTree_GetObject(int id)
{
	struct upnp_entry_lookup_t *res, entry_lookup;

	if(id == 0) /* We do not store the root (id 0) as it is not a child */
		return ut->root_entry;

	entry_lookup.id = id;
	res = (struct upnp_entry_lookup_t *)rbfind ((void *) &entry_lookup, ut->rb);
	if (res)
	{
		return ((struct upnp_entry_lookup_t *) res)->entry_ptr;
	}

	return NULL;
}
C_DMS_VO *Dms_VirtualFileTree_GetRootObject (void)
{
	return Dms_VirtualFileTree_GetObject(0);
}

static int s_upnp_entry_find_recursive (C_DMS_VO *entry, char type, char *keyword, int deep)
{
	int id = -1;
	
	if(Dms_VirtualObject_IsContainer(entry))
	{
		C_DMS_CCI *container = (C_DMS_CCI *)(entry->object);
		int num, i, count;

		if(coo_str_equal(keyword, container->keyword))
			return entry->id;
		
		if(!deep)
			return id;
		deep--;
		
		for(num = 0; num < x_DMS_CHILDS_LISTS_NUM; num++)
		{
			COO_ARRAY *ca = container->childs_lists[num];
			if(!ca)
				continue;
			
			count = coo_array_real_count(ca);
			for(i = 0; i < count; i++)
			{
				id = s_upnp_entry_find_recursive(coo_array_get(ca, i), type, keyword, deep);
				if(id > 0)
					return id;
			}
		}
	}
	else if(Dms_VirtualObject_IsItem(entry))
	{
		C_DMS_CMI *item = (C_DMS_CMI *)(entry->object);
		
		if(coo_str_equal(keyword, item->keyword))
			return entry->id;
	}
	else
	{
	}

	return id;
}

/*--------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------*/
static C_DMS_VO *s_make_root_container(char *keyword, char *title)
{
	C_DMS_CCI *container = COO_OBJECT_NEW(C_DMS_CCI);
	container->keyword		= strdup(keyword);
	container->title		= strdup(title);
	container->mime_type	= &Container_MIME_Type;

	C_DMS_VO *entry = Dms_VirtualObject_Create(-1, container, e_OBJTYPE_CONTAINER, e_SOURCE_ROOT, NULL, Dms_VirtualObject_FreeContainerSelf);
	return entry;
}

static int s_redblack_compare (const void *pa, const void *pb, const void *config __attribute__ ((unused)))
{
	struct upnp_entry_lookup_t *a, *b;

	a = (struct upnp_entry_lookup_t *) pa;
	b = (struct upnp_entry_lookup_t *) pb;

	if (a->id < b->id)
		return -1;

	if (a->id > b->id)
		return 1;

	return 0;
}
void dms_filetree_build(struct ushare_t *p)
{
	p->rb = rbinit (s_redblack_compare, NULL);
	p->root_entry = s_make_root_container("root", "root");
	p->init = 1;
}
void dms_filetree_free(struct ushare_t *p)
{
	if(!p)	return;

	p->init = 0;
	
	s_upnp_entry_free_recursive(ut, p->root_entry);
	p->root_entry = NULL;

	rbdestroy (p->rb);
	p->rb = NULL;
}
static void HySDK_PvrMakeObjectId(C_DMS_VO*entry, int index, char *buf, int buf_len)
{
	struct json_object*json;
	C_DMS_CMI *pvr = entry->object;

	if(pvr->extension && (json = json_object_object_get(pvr->extension, "contentType")))
	{
		sprintf(buf, "%d.hw.%s", index, json_object_get_string(json));
		return;
	}
	
	sprintf(buf, "%d", index);
	buf_len = 0;
}

static void s_Raw_Dms_VirtualFileTree_DeleteChild (int parent_id, int child_id, int keep_if_grandchild)
{
	C_DMS_VO *parent, *child;

	if(s_is_invalid_id(parent_id))
		return;
	if(s_is_invalid_id(child_id))
		return;
	
	HT_DBG_FUNC_START(HT_MOD_DMS, HT_BIT_KEY, parent_id, 0);
	Dms_WaitLock();
	parent = Dms_VirtualFileTree_GetObject(parent_id);
	child  = Dms_VirtualFileTree_GetObject(child_id);
	if(keep_if_grandchild && child && Dms_VirtualObject_IsContainer(child))
	{
		int num, count = 0;
		C_DMS_CCI *container = child->object;
		for(num = 0; num < x_DMS_CHILDS_LISTS_NUM; num++)
		{
			COO_ARRAY *ca = container->childs_lists[num];
			if(ca)
				count += coo_array_real_count(ca);
		}
		if(!count)
			s_upnp_entry_delete_a_child(parent, child);
	}
	else
		s_upnp_entry_delete_a_child(parent, child);
	HT_DBG_FUNC_END(child_id, NULL);
	Dms_PostLock();
}

int Raw_Dms_VirtualFileTree_AddChild(int parent_id, char *info)
{
	int ret = e_IDE_INVALID_INFO;
	C_DMS_VO *child = NULL;

	if(s_is_invalid_id(parent_id))
		return e_IDE_INVALID_PARENT_ID;
	if(!info)
		return e_IDE_INVALID_INFO;
	
	HT_DBG_FUNC_START(HT_MOD_DMS, HT_BIT_KEY, parent_id, info);
	Dms_WaitLock();
	int x = Dms_VirtualObject_From_String(info, &child);
	if(!x)
	{
		if(Dms_VirtualObject_IsItem(child))
		{
			if(DMS_OBJCT_IS_PVR_VOD(child))
				child->MakeObjectId = HySDK_PvrMakeObjectId;
			child->FreeObject	= Dms_VirtualObject_FreeItem;
			ret = s_Dms_VirtualFileTree_AddChildEx(parent_id, 0, child);
		}
		else if(Dms_VirtualObject_IsContainer(child))
		{
			child->MakeObjectId = NULL;
			child->FreeObject	= Dms_VirtualObject_FreeContainerSelf;
			ret = s_Dms_VirtualFileTree_AddChildEx(parent_id, 0, child);
		}
		else
		{
		}
	}
		
	Dms_PostLock();
	HT_DBG_FUNC_END(ret, NULL);
	return ret;
}

void Raw_Dms_VirtualFileTree_DeleteChild (int parent_id, int child_id)
{
	s_Raw_Dms_VirtualFileTree_DeleteChild(parent_id, child_id, 0);
}
void Raw_Dms_VirtualFileTree_DeleteContainer (int parent_id, int container_id, int keep_if_child)
{
	s_Raw_Dms_VirtualFileTree_DeleteChild(parent_id, container_id, keep_if_child);
}

void Raw_Dms_VirtualFileTree_GetInfo (int id, int onlybasic, char *buf, int len)
{
	int ret;
	char *info = NULL;
	C_DMS_VO *child = NULL;

	if(s_is_invalid_id(id))
		return;
	if(len < 10)
		return;
	
	HT_DBG_FUNC_START(HT_MOD_DMS, HT_BIT_KEY, id, 0);
	Dms_WaitLock();
	child  = Dms_VirtualFileTree_GetObject(id);
	ret = Dms_VirtualObject_To_String(child, onlybasic, &info);
	int l = strlen(info);
	if(l < len)
		strcpy(buf, info);
	free(info);
	Dms_PostLock();
	HT_DBG_FUNC_END(len, buf);
}

int Raw_Dms_VirtualFileTree_GetParentID(int child_id)
{
	if(s_is_invalid_id(child_id))
		return e_IDE_INVALID_CHILD_ID;
	
	HT_DBG_FUNC_START(HT_MOD_DMS, HT_BIT_KEY, child_id, 0);
	Dms_WaitLock();
	C_DMS_VO *me = Dms_VirtualFileTree_GetObject(child_id);
	int ret = me? me->parent->id : -1;
	Dms_PostLock();
	HT_DBG_FUNC_END(ret, NULL);
	return ret;
}

int Raw_Dms_VirtualFileTree_GetChildID (int parent_id, char type, char *keyword, int is_recursive)
{
	int ret, deep;
	C_DMS_VO *parent;

	if(s_is_invalid_id(parent_id))
		return e_IDE_INVALID_PARENT_ID;
	if(!keyword)
		return e_IDE_NO_KEYWORD;
	
	HT_DBG_FUNC_START(HT_MOD_DMS, HT_BIT_KEY, parent_id, keyword);
	Dms_WaitLock();
	parent = Dms_VirtualFileTree_GetObject(parent_id);
	deep = is_recursive? 100000 : 1;
	ret = s_upnp_entry_find_recursive(parent, type, keyword, deep);
	Dms_PostLock();
	HT_DBG_FUNC_END(ret, NULL);
	return ret;
}
/*--------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------*/
void Dms_PresetPvrObjectCmp(t_DMS_CMP_OBJECT cmp_func)
{
	s_pvr_entry_cmp = cmp_func;
}



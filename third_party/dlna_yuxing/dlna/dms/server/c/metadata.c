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
#if 0

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdbool.h>

#include "mime.h"
//#include "metadata.h"
#include "util_iconv.h"
#include "trace.h"
#include "hitTime.h"
#include "HySDK.h"
#include <locale.h>
#include "dms.h"

#include "metadata.h"

#define TITLE_UNKNOWN "unknown"
#define x_SORT_CONTAINER_FIRST	1

typedef struct xml_convert_s {
	char charac;
	char *xml;
} xml_convert_t;

struct upnp_entry_lookup_t {
	int id;
	struct upnp_entry_t *entry_ptr;
};

typedef struct _x_fullpath_id_t_ {
	char				*fullpath;
	char				*title;
	
	C_UPNP_ENTRY		*entry;
	int					is_pvr;
	int					del;
}x_FP_ID;
static x_FP_ID* s_FPID_Create(char *fullpath, char *title, C_UPNP_ENTRY *entry, int is_pvr)
{
	x_FP_ID *p = (x_FP_ID *)malloc(sizeof(x_FP_ID));
	if( p )
	{
		memset(p, 0, sizeof(x_FP_ID));
		
		p->fullpath = strdup(fullpath);
		if(title)
			p->title	= strdup(title);
		p->entry	= entry;
		p->is_pvr	= is_pvr;
	}
	return p;
}
static void s_FPID_Release(x_FP_ID *p)
{
	if( p )
	{
		if(p->fullpath)
			free(p->fullpath);
		if(p->title)
			free(p->title);
		free(p);
	}
}

static int s_is_strings_equal (char *s1, char *s2)
{
	return ( s1 && s2 && (!strcmp(s1,s2)) )? 1: 0;
}

void DiscardTailSlash(char *str)
{
	int len;
	if( str && (len=strlen(str)) && (len>1) && (str[len-1]=='/') )
		str[len-1]= '\0';
}
static char *s_GetFullpath(char *path, char *name)
{
	char *fullpath = malloc(strlen(path) + strlen(name) +4 );
	if(fullpath )
		sprintf(fullpath, "%s/%s", path, name);
	return fullpath;
}
static char *s_GetName(char *fullpath)
{
	if( !fullpath )
		return NULL;
	char *str = strrchr(fullpath,'/');
	return str? str+1 : fullpath;
}
static char *getExtension (const char *filename)
{
	char *str =strrchr (filename, '.');
	if (str)
		str++;
	return str;
}

static bool is_valid_extension (const char *extension)
{
  if (!extension)
    return false;

  if (getMimeType (extension))
    return true;

  return false;
}

static xml_convert_t xml_convert[] = {
  {'"' , "&quot;"},
  {'&' , "&amp;"},
  {'\'', "&apos;"},
  {'<' , "&lt;"},
  {'>' , "&gt;"},
  {'\n', "&#xA;"},
  {'\r', "&#xD;"},
  {'\t', "&#x9;"},
  {0, NULL},
};

static char *get_xmlconvert (int c)
{
  int j;
  for (j = 0; xml_convert[j].xml; j++)
  {
    if (c == xml_convert[j].charac)
      return xml_convert[j].xml;
  }
  return NULL;
}

char *dms_convert_xml (char *title)
{
  char *newtitle, *s, *t, *xml;
  int nbconvert = 0;

  /* calculate extra size needed */
  for (t = (char*) title; *t; t++)
  {
    xml = get_xmlconvert (*t);
    if (xml)
      nbconvert += strlen (xml) - 1;
  }
  if (!nbconvert)
    return NULL;

  newtitle = s = (char*) malloc (strlen (title) + nbconvert + 1);

  for (t = (char*) title; *t; t++)
  {
    xml = get_xmlconvert (*t);
    if (xml)
    {
      strcpy (s, xml);
      s += strlen (xml);
    }
    else
      *s++ = *t;
  }
  *s = '\0';

  return newtitle;
}

static void s_upnp_entry_free_only_self(C_UPNP_ENTRY *entry)
{
	if(!entry)
		return;

	if(entry->fullpath)
		free (entry->fullpath);
	if(entry->title)
		free (entry->title);
	if(entry->protocol_info)
		entry->protocol_info = NULL;
	if(entry->minfo.pp_json)
		json_object_put(entry->minfo.pp_json);
	
	free(entry->childs);
	free(entry);
}
/* Seperate recursive free() function in order to avoid freeing off
 * the parents child list within the freeing of the first child, as
 * the only entry which is not part of a childs list is the root entry
 */
static void s_upnp_entry_free_recursive (struct ushare_t *ut, C_UPNP_ENTRY *entry)
{
	struct upnp_entry_t **childs;
	struct upnp_entry_lookup_t entry_lookup_ptr;
	
	if(!entry)
		return;

	HT_DBG_FUNC_START(HT_MOD_DMS, HT_BIT_MYRIAD, (int)entry, 0);

	for(childs = entry->childs; *childs; childs++)
		s_upnp_entry_free_recursive (ut, *childs);

	entry_lookup_ptr.id = entry->id;
	entry_lookup_ptr.entry_ptr = entry;
	if (rbdelete ((void *) (&entry_lookup_ptr), ut->rb) == NULL)
	{
		HT_DBG_FUNC( entry->id, "Failed to delete the RB lookup tree");
	}

	s_upnp_entry_free_only_self(entry);
	ut->total_entries--;
	
	HT_DBG_FUNC_END(ut->total_entries, "ut->total_entries = ");
}

static void s_upnp_entry_free_child (struct ushare_t *ut, C_UPNP_ENTRY *parent, C_UPNP_ENTRY *child)
{
	int i, count;

	count = parent->child_count;
	for( i = 0; i < count; i++ )
	{
		C_UPNP_ENTRY *p = parent->childs[i];
		if(p == child)
		{
			if(p->child_count != -1)
				parent->container_count--;
			
			parent->updateID++;
			ut->systemUpdateId++;
			
			s_upnp_entry_free_recursive(ut, p);

			for(count--; i < count; i++)
				parent->childs[i] = parent->childs[i+1];
			parent->child_count--;
			parent->childs[parent->child_count] = NULL;
			return;
		}
	}
}

#if (!defined(x_SORT_CONTAINER_FIRST))
static int get_list_length (void *list)
{
  void **l = list;
  int n = 0;

  while (*(l++))
    n++;

  return n;
}

static void s_upnp_entry_add_child (struct ushare_t *ut, C_UPNP_ENTRY *entry, C_UPNP_ENTRY *child, int is_item)
{
	struct upnp_entry_lookup_t *entry_lookup_ptr = NULL;
	struct upnp_entry_t **childs;
	int n;

	HT_DBG_FUNC_START(HT_MOD_DMS, HT_BIT_MYRIAD, (int)entry, "entry = ");
	
	if (!entry || !child)
		return;

	for (childs = entry->childs; *childs; childs++)
		if (*childs == child)
			return;

	ut->nr_entries++;
	child->id = ut->starting_id + ut->nr_entries;
	ut->total_entries++;
	HT_DBG_FUNC(ut->total_entries ,"ut->total_entries = ");
	n = get_list_length ((void *) entry->childs) + 1;
	entry->childs = (struct upnp_entry_t **)realloc (entry->childs, (n + 1) * sizeof (*(entry->childs)));
	entry->childs[n] = NULL;
	entry->childs[n - 1] = child;
	entry->child_count++;

	entry_lookup_ptr = (struct upnp_entry_lookup_t *)malloc (sizeof (struct upnp_entry_lookup_t));
	entry_lookup_ptr->id = child->id;
	entry_lookup_ptr->entry_ptr = child;

	if (rbsearch ((void *) entry_lookup_ptr, ut->rb) == NULL)
		log_info (_("Failed to add the RB lookup tree\n"));
	HT_DBG_FUNC_END((int)child, "child = ");
}
#else
static void s_upnp_entry_add_child (struct ushare_t *ut, C_UPNP_ENTRY *entry, C_UPNP_ENTRY *child, int is_item)
{
	struct upnp_entry_lookup_t *entry_lookup_ptr = NULL;
	struct upnp_entry_t **pOld, **pNew;
	int n, i;

	HT_DBG_FUNC_START(HT_MOD_DMS, HT_BIT_MYRIAD, (int)entry, "entry = ");
	
	if (!entry || !child)
		return;
#if 0
	for (childs = entry->childs; *childs; childs++)
		if (*childs == child)
			return;
#endif
	ut->nr_entries++;
	child->id = ut->starting_id + ut->nr_entries;
	ut->total_entries++;
	HT_DBG_FUNC(ut->total_entries ,"ut->total_entries = ");
	HT_DBG_FUNC(entry->childs_len ,"entry->childs_len = ");
	HT_DBG_FUNC(entry->child_count ,"entry->child_count = ");
	HT_DBG_FUNC(entry->container_count ,"entry->container_count = ");
	
	n = entry->child_count;
	if(entry->childs_len > (n+1))
	{
		if(is_item)
		{
			entry->childs[n] = child;
			entry->childs[n + 1] = NULL;
		}
		else
		{
			i = entry->child_count - entry->container_count;
			pOld = entry->childs + entry->child_count - 1;
			pNew = entry->childs + entry->child_count;
			while(i--)
			{
				*pNew-- = *pOld--;
				HT_DBG_FUNC((int)pNew ,"pNew = ");
			}

			entry->childs[entry->container_count] = child;
			entry->childs[n + 1] = NULL;
			
			entry->container_count++;
		}
	}
	else
	{
		entry->childs_len++;
		entry->childs_len += entry->childs_len/5;
		
		pOld = entry->childs;
		entry->childs = (struct upnp_entry_t **)malloc(entry->childs_len* sizeof (*(entry->childs)));
		memcpy(entry->childs, pOld, n * sizeof (*(entry->childs)));
		free(pOld);
		
		if(is_item)
		{
			entry->childs[n] = child;
			entry->childs[n + 1] = NULL;
		}
		else
		{
			i = entry->child_count - entry->container_count;
			pOld = entry->childs + entry->child_count - 1;
			pNew = entry->childs + entry->child_count;
			while(i--)
				*pNew-- = *pOld--;

			entry->childs[entry->container_count] = child;
			entry->childs[n + 1] = NULL;
			
			entry->container_count++;
		}
	}	
	entry->child_count++;
	entry->updateID++;
	ut->systemUpdateId++;
	
	entry_lookup_ptr = (struct upnp_entry_lookup_t *)malloc (sizeof (struct upnp_entry_lookup_t));
	entry_lookup_ptr->id = child->id;
	entry_lookup_ptr->entry_ptr = child;

	if (rbsearch ((void *) entry_lookup_ptr, ut->rb) == NULL)
		log_info (_("Failed to add the RB lookup tree\n"));

	if(is_item && child->pvr)
	{
		// 5. ordering
		int i, j, count;
		
		setlocale(LC_ALL, "hu_HU.UTF-8") ;
		count = entry->child_count;
		for( i = 0; i < count; i++ )
		{
			HT_DBG_PRINTF(HT_MOD_DMS, HT_BIT_MYRIAD, "ii=%2d: %s\r\n", i, (entry->childs[i])->title);
		}
		
		for( i = 0; i < count; i++ )
		{
			for(j = i+1; j < count; j++)	
			{
				if(strcoll((entry->childs[i])->title, (entry->childs[j])->title) > 0) 
				{
					void *item;
					
					HT_DBG_PRINTF(HT_MOD_DMS, HT_BIT_MYRIAD, "ii=%2d: %s; jj=%2d: %s\r\n", i, (entry->childs[i])->title, j, (entry->childs[j])->title);
					item = entry->childs[i];
					entry->childs[i] = entry->childs[j] ;
					entry->childs[j] = item;
				}
			}
		}

		for( i = 0; i < count; i++ )
		{
			HT_DBG_PRINTF(HT_MOD_DMS, HT_BIT_MYRIAD, "ii=%2d: %s\r\n", i, (entry->childs[i])->title);
		}
	}
	HT_DBG_FUNC_END((int)child, "child = ");
}

#endif
static void s_upnp_entry_add_child_mutex (struct ushare_t *ut, struct upnp_entry_t *parent, struct upnp_entry_t *child, int is_item)
{
    while(ut->pause_adding)
        usleep(100*1000);
    
	Dms_WaitLock();
	s_upnp_entry_add_child(ut, parent, child, is_item);
	Dms_PostLock();
}

C_UPNP_ENTRY *upnp_get_entry (struct ushare_t *ut, int id)
{
	struct upnp_entry_lookup_t *res, entry_lookup;

	if (id == 0) /* We do not store the root (id 0) as it is not a child */
		return ut->root_entry;

	entry_lookup.id = id;
	res = (struct upnp_entry_lookup_t *)rbfind ((void *) &entry_lookup, ut->rb);
	if (res)
	{
		log_verbose ("Found at %p\n",((struct upnp_entry_lookup_t *) res)->entry_ptr);
		return ((struct upnp_entry_lookup_t *) res)->entry_ptr;
	}

	log_verbose ("Not Found\n");
	return NULL;
}

C_UPNP_ENTRY *Dms_VirtualFileTree_GetObject (int id)
{
	return upnp_get_entry(ut, id);
}
/////////////////////////////////////////////////////////////////////////////////
/* Try Iconv'ing the name but if it fails the end device, may still be able to handle it */
#if 0
static char* s_upnp_entry_name(const char *name, int is_dir, int override_err)
{
	char *x, *title_or_name = NULL;
//	char *title = iconv_convert (name);
	char *title = strdup (name);

	if(title)
		title_or_name = title;
	else
	{
		if(override_err) /* ut->override_iconv_err*/
		  title_or_name = strdup (name);
		else
		  return NULL;
	}
	
	if(!is_dir)/* avoid displaying file extension */
	{
		x = strrchr (title_or_name, '.');
		if(x)	
		  *x = '\0';
	}
	
	x = convert_xml (title_or_name);/* avoid special char collided with xml keyword */
	if (x)
	{
		free (title_or_name);
		title_or_name = x;
	}
	
	if (!strcmp (title_or_name, "")) /* DIDL dc:title can't be empty */
	{
		free (title_or_name);
		title_or_name = strdup (TITLE_UNKNOWN);
	}

	return title_or_name;
}
#else

static char* s_upnp_entry_name(const char *title, int is_dir, int override_err)
{
	char c, *title_or_name = NULL;
	unsigned int max_len = 256; /* dlna: recommend <256; maximum = 1024. */
	char *name = (char*)title;
	
	is_dir = 0;
	override_err = 0;
	
	if(!title)
		return NULL;

	if(strlen(name) < max_len)
		title_or_name = strdup(name);
	else
	{
		c = name[max_len];
		name[max_len] = 0;
		title_or_name = strdup(name);
		name[max_len] = c;
	}
	
	if(title_or_name)
	{
		char *dot = strrchr(title_or_name, '.');
		if(dot && (dot != title_or_name))
			*dot = 0;
#if 0		
		x = dms_convert_xml (title_or_name);/* avoid special char collided with xml keyword */
		if(x)
		{
			free (title_or_name);
			title_or_name = x;
		}
#endif		
		if(!strcmp (title_or_name, "")) /* DIDL dc:title can't be empty */
		{
			free (title_or_name);
			title_or_name = strdup(TITLE_UNKNOWN);
		}
	}

	return title_or_name;
}

#endif

static struct mime_type_t Container_MIME_Type = { NULL, "object.container.storageFolder", NULL};
#if 0
struct stat {
	dev_t		  st_dev;	   /* device */
	ino_t		  st_ino;	   /* inode */
	mode_t		  st_mode;	   /* protection */
	nlink_t 	  st_nlink;    /* number of hard links */
	uid_t		  st_uid;	   /* user ID of owner */
	gid_t		  st_gid;	   /* group ID of owner */
	dev_t		  st_rdev;	   /* device type (if inode device) */
	off_t		  st_size;	   /* total size, in bytes */
	blksize_t	  st_blksize;  /* blocksize for filesystem I/O */
	blkcnt_t	  st_blocks;   /* number of blocks allocated */
	time_t		  st_atime;    /* time of last access */
	time_t		  st_mtime;    /* time of last modification */
	time_t		  st_ctime;    /* time of last status change */
};

struct dirent {
	ino_t			d_ino;		/* 索引号 */
	off_t			d_off;		/* 下一个偏移量 */
	unsigned short	d_reclen;	/* 本记录长度 */
	unsigned char	d_type;		/* 文件类型 */
	char			d_name[256];/* 文件名 */
}; 

#endif
static C_UPNP_ENTRY *s_upnp_entry_new (struct ushare_t *ut, const char *name, const char *fullpath, dlna_profile_t *p,
											struct stat *st, C_UPNP_ENTRY *parent,  int is_dir)
{
	if( ut->total_entries > 5000 )
		return NULL;
	
	C_UPNP_ENTRY *entry = (C_UPNP_ENTRY *) malloc (sizeof (C_UPNP_ENTRY));
	if( !entry )
		return NULL;
	
	memset(entry, 0, sizeof(C_UPNP_ENTRY));
	entry->fullpath		= fullpath ? strdup (fullpath) : NULL;
	entry->parent		= parent;

	entry->child_count	=  is_dir ? 0 : -1;
	entry->childs		= (C_UPNP_ENTRY **)malloc(sizeof (C_UPNP_ENTRY *)); 
	*(entry->childs)	= NULL;
	entry->childs_len	= 1;
	
	if(st)
	{
		entry->minfo.size 	= st->st_size;
	}

	if(is_dir)
		entry->mime_type = &Container_MIME_Type;
	else
		entry->protocol_info = p;

	char *title_or_name = s_upnp_entry_name(name, is_dir, ut->override_iconv_err);
	entry->title  = title_or_name;
	if( !title_or_name )
	{
		s_upnp_entry_free_only_self(entry);
		entry = NULL;
	}

	return entry;
}

static C_UPNP_ENTRY *s_upnp_entry_new_dlna (struct ushare_t *ut, const char *name, const char *fullpath, struct stat *st, C_UPNP_ENTRY *parent, int is_dir)
{
	C_UPNP_ENTRY *entry=NULL;
	dlna_profile_t *p=NULL;

	if(!is_dir)
	{
		p = dlna_guess_media_profile (ut->dlna, fullpath);
		if( p )
			entry = s_upnp_entry_new(ut, name, fullpath, p, st, parent, is_dir);
	}
	else
		entry = s_upnp_entry_new(ut, name, fullpath, p, st, parent, is_dir);
	
	return entry;
}
extern dlna_profile_t *dlna_guess_pvr_info(dlna_t *dlna);
static C_UPNP_ENTRY *s_upnp_entry_new_pvr (struct ushare_t *ut, const char *name, const char *fullpath, C_UPNP_ENTRY *parent, int is_dir)
{
	C_UPNP_ENTRY *entry=NULL;
	C_UD_MI minfo;

	HT_DBG_FUNC_START(HT_MOD_DMS, HT_BIT_MANY, 0, name);
	if( !HySDK_PvrGetInfo(fullpath, &minfo) )
	{
		if( minfo.xx_title )
			name = minfo.xx_title;
		
		entry = s_upnp_entry_new(ut, name, fullpath, dlna_guess_pvr_info(ut->dlna), NULL, parent, is_dir);
		if( entry )
		{
			entry->pvr = 1;
			memcpy(&(entry->minfo), &minfo, sizeof(C_UD_MI));
			entry->minfo.xx_title = NULL;
		}
		else
		{
			if( minfo.pp_json )
				json_object_put(minfo.pp_json);
		}
		
		if( minfo.xx_title )
			free(minfo.xx_title);
	}
	HT_DBG_FUNC_END((int)entry, 0);
	return entry;
}

static ListNode *s_content_find_folder(LinkedList *list, char *fullpath)
{
	ListNode * node = ListHead(list);
	while( node )
	{
		x_FP_ID *p = (x_FP_ID*)(node->item);
		if( s_is_strings_equal( p->fullpath, fullpath ) && (p->del==0))
			break;
		node = ListNext(list, node);
	}
	return node;
}

int content_find_folder(struct ushare_t *ut, char *fullpath)
{
	if(s_content_find_folder(&(ut->list_added), fullpath))
		return 1;
	if(s_content_find_folder(&(ut->list_to_add), fullpath))
		return 2;
	return 0;
}

int content_add_pvr_item(struct ushare_t *ut, char *path, char *itemName)
{
	C_UPNP_ENTRY *root, *child, *p;
	int i;
	
	if( !ut || !path || !itemName  )
		return -1;
	
	DiscardTailSlash(path);

	root = ut->root_entry;
	for( i = 0; i < root->child_count; i++ )
	{
		child = root->childs[i];
		if( strcmp(child->fullpath, path) == 0 )
		{
			char *fullpath = s_GetFullpath(path, itemName);
			p = s_upnp_entry_new_pvr(ut, itemName, fullpath, child, 0);
			if( p )
				s_upnp_entry_add_child(ut, child, p, 1);
			
			if(fullpath) 
				free(fullpath);
			return 1;
		}
	}

	return 0;
}

int content_remove_pvr_item(struct ushare_t *ut, char *path, char *itemName)
{
	C_UPNP_ENTRY *root, *child, *p;
	int i, j, k;
	
	if( !ut || !path || !itemName  )
		return -1;
	
	DiscardTailSlash(path);

	root = ut->root_entry;
	for( i = 0; i < root->child_count; i++ )
	{
		child = root->childs[i];
		if( strcmp(child->fullpath, path) )
			continue;

		k = child->child_count;
		for( j = 0; j<k; j++ )
		{
			p = child->childs[j];
			if( strcmp(s_GetName(p->fullpath), itemName) == 0 )
			{
				s_upnp_entry_free_child(ut, child, p);
				return 1;
			}
		}
	}

	return 0;
}
#if (!defined(x_SORT_CONTAINER_FIRST))
static int s_metadata_add_container (struct ushare_t *ut, struct upnp_entry_t *entry, const char *container)
{
	struct dirent **namelist;
	struct stat st;
	int n,i,run = 1;
	struct upnp_entry_t *child;
	
	n = scandir (container, &namelist, 0, alphasort);
	if (n < 0)
	{
		perror ("scandir");
		return -2;
	}

	for (i = 0; i < n; i++)
	{
		if( ut->flag_adding==0 )
			run = 0;

		if( run )
		{
			if (namelist[i]->d_name[0] == '.')
			{
			  free (namelist[i]);
			  continue;
			}

			char *fullpath = (char *)malloc (strlen (container) + strlen (namelist[i]->d_name) + 2);
			sprintf (fullpath, "%s/%s", container, namelist[i]->d_name);

			if (stat (fullpath, &st) < 0)
			{
			  free (namelist[i]);
			  free (fullpath);
			  continue;
			}

			if( ut->is_pvr )
			{
			    //if (S_ISDIR (st.st_mode))
			    {
			      child = s_upnp_entry_new_pvr(ut, namelist[i]->d_name,fullpath, entry, 0);
				  if (child)
					s_upnp_entry_add_child_mutex(ut, entry, child, 1);
			    }				
			}
			else
			{
				if (S_ISDIR (st.st_mode))
				{
				  child = s_upnp_entry_new_dlna (ut, namelist[i]->d_name, fullpath, &st, entry, true);
				  if (child)
				  {
					s_upnp_entry_add_child_mutex(ut, entry, child, 0);
				    int x = s_metadata_add_container (ut, child, fullpath);
					if( x==0 )
						run = 0;
				  }
				}
				else
				{
				  if(is_valid_extension(getExtension (namelist[i]->d_name)))
				  {
					child = s_upnp_entry_new_dlna(ut, namelist[i]->d_name, fullpath, &st, entry, false);
				    if (child)
				      s_upnp_entry_add_child_mutex(ut, entry, child, 1);	
				  }
				}				
			}

			free (fullpath);
		}

		free (namelist[i]);
	}
	free (namelist);

	return run;
}
#else
static int s_metadata_add_container_pvr (struct ushare_t *ut, struct upnp_entry_t *entry, const char *container)
{
	struct dirent **namelist;
	struct stat st;
	int n,i,run = 1;
	struct upnp_entry_t *child;
	
	n = scandir (container, &namelist, 0, alphasort);
	HT_DBG_FUNC_START(HT_MOD_DMS, HT_BIT_MANY, n, container);
	if (n < 0)
	{
		perror ("scandir");
		return -2;
	}

	for (i = 0; i < n; i++)
	{
		if( ut->flag_adding==0 )
			run = 0;

		if( run )
		{
			if (namelist[i]->d_name[0] == '.')
			{
				free (namelist[i]);
				continue;
			}

			char *fullpath = (char *)malloc (strlen (container) + strlen (namelist[i]->d_name) + 2);
			sprintf (fullpath, "%s/%s", container, namelist[i]->d_name);

			if (stat (fullpath, &st) < 0)
			{
				free (fullpath);
				free (namelist[i]);
				continue;
			}

			HT_DBG_FUNC(S_ISDIR(st.st_mode), namelist[i]->d_name);
//			if(S_ISDIR(st.st_mode))
			{
				child = s_upnp_entry_new_pvr(ut, namelist[i]->d_name,fullpath, entry, 0);
				if (child)
				  s_upnp_entry_add_child_mutex(ut, entry, child, 1);
			}
			free (fullpath);
		}
		free (namelist[i]);
	}		
	
	free (namelist);
	HT_DBG_FUNC_END(run, 0);
	return run;
}

static int s_metadata_add_container_usb (struct ushare_t *ut, struct upnp_entry_t *entry, const char *container)
{
	struct dirent **namelist;
	struct stat st;
	int n,i,run = 1;
	struct upnp_entry_t *child;
	
	HT_DBG_FUNC_START(HT_MOD_DMS, HT_BIT_MYRIAD, 0, container);
	n = scandir (container, &namelist, 0, alphasort);
	HT_DBG_FUNC(n, "n = ");
	if (n < 0)
	{
		perror ("scandir");
		return -2;
	}

	/* add item first */
	for (i = 0; i < n; i++)
	{
		if( ut->flag_adding==0 )
			run = 0;

		if( run )
		{
			if (namelist[i]->d_name[0] == '.')
			{
				free (namelist[i]);
				namelist[i] = 0;
				continue;
			}

			char *fullpath = (char *)malloc (strlen (container) + strlen (namelist[i]->d_name) + 2);
			sprintf (fullpath, "%s/%s", container, namelist[i]->d_name);

			if (stat (fullpath, &st) < 0)
			{
				free (fullpath);
				free (namelist[i]);
				namelist[i] = 0;
				continue;
			}

			HT_DBG_FUNC(S_ISREG(st.st_mode), namelist[i]->d_name);
			if(S_ISREG(st.st_mode))
			{
				if(	is_valid_extension(getExtension (namelist[i]->d_name)))
				{
					child = s_upnp_entry_new_dlna(ut, namelist[i]->d_name, fullpath, &st, entry, false);
					if (child)
						s_upnp_entry_add_child_mutex(ut, entry, child, 1);	
				}
				
				free (namelist[i]);
				namelist[i] = 0;
			}

			free (fullpath);
		}
	}

	/* then add container */
	HT_DBG_FUNC(0, "now start to process containers");
	for (i = 0; i < n; i++)
	{
		if( ut->flag_adding==0 )
			run = 0;

		if( run && namelist[i])
		{
			if(namelist[i]->d_name[0] == '.')
			{
			  free (namelist[i]);
			  continue;
			}

			char *fullpath = (char *)malloc (strlen (container) + strlen (namelist[i]->d_name) + 2);
			sprintf (fullpath, "%s/%s", container, namelist[i]->d_name);

			if(stat(fullpath, &st) < 0)
			{
			  free (fullpath);
			  free (namelist[i]);
			  continue;
			}

			HT_DBG_FUNC(S_ISDIR(st.st_mode), namelist[i]->d_name);
			if(S_ISDIR(st.st_mode))
			{
				child = s_upnp_entry_new_dlna (ut, namelist[i]->d_name, fullpath, &st, entry, true);
				if (child)
				{
					s_upnp_entry_add_child_mutex(ut, entry, child, 0);
					int x = s_metadata_add_container_usb (ut, child, fullpath);
					if( x==0 )
						run = 0;
				}
			}

			free (fullpath);
		}

		if(namelist[i])
			free(namelist[i]);
	}
	
	free (namelist);
	HT_DBG_FUNC_END(run, 0);
	return run;
}
#endif

static void s_content_add_loop(void)
{
	extern struct ushare_t *ut;
	LinkedList *list;
 	ListNode * node;
	x_FP_ID *p;
 	char *fullpath, *title;
	C_UPNP_ENTRY *entry;

	HT_DBG_FUNC_START(HT_MOD_DMS, HT_BIT_KEY, 0, 0);
	
	while(!(ut->quit))
	{
		sem_wait(&(ut->bell));
		
		Dms_WaitLock();
		list = &(ut->list_to_add);
		node = ListHead(list);
		HT_DBG_FUNC((int)node, "new node = ");
		/* no new folder at all */
		if( !node )	
		{
			Dms_PostLock();
			continue;
		}

		p = (x_FP_ID*)(node->item);
		/* critical one, we just delete it*/
		HT_DBG_FUNC(p->del, "first: p->del = ");
		if( p->del )
		{
			s_FPID_Release(p);
			ListDelNode(list, node, 0);
			Dms_PostLock();
			continue;
		}

		/* set env & new an entry */
		ut->flag_adding = 1;
		ut->is_pvr = p->is_pvr;
		fullpath = p->fullpath;
		title = p->title? p->title : s_GetName(fullpath);
		entry = s_upnp_entry_new_dlna (ut, title, fullpath, NULL, ut->root_entry, true);
		p->entry = entry;
		if( entry )
			s_upnp_entry_add_child(ut, ut->root_entry, entry, 0);
		HT_DBG_FUNC((int)(p->entry), fullpath);
		Dms_PostLock();

		/* ok, we can add one by one */
		if( entry )
		{
#if (!defined(x_SORT_CONTAINER_FIRST))
			s_metadata_add_container(ut, entry, fullpath);
#else
			if(ut->is_pvr)
				s_metadata_add_container_pvr(ut, entry, fullpath);
			else
				s_metadata_add_container_usb(ut, entry, fullpath);
#endif
		}
		
		/* lastly, delete or move node? */
		Dms_WaitLock();
		ListDelNode(list, node, 0);
		HT_DBG_FUNC(p->del, "second: p->del = ");
		if( p->del )
		{
            if(entry)
			    s_upnp_entry_free_child(ut, ut->root_entry, p->entry);
			s_FPID_Release(p);
		}
		else
		{
            if(entry)
			    ListAddTail(&(ut->list_added), p);
            else
                s_FPID_Release(p);
		}

		ut->flag_adding = 2;
		Dms_PostLock();
	}

	HT_DBG_FUNC_END(0, 0);
}
int content_add_folder(struct ushare_t *ut, char *fullpath, char *title, int is_pvr)
{
	HT_DBG_FUNC_START(HT_MOD_DMS, HT_BIT_FEW, is_pvr, fullpath);
	int ret =  content_find_folder(ut, fullpath);
	if( !ret )
	{
		x_FP_ID *p = s_FPID_Create(fullpath, title, 0, is_pvr);
		if(p)
		{
			ListAddTail(&(ut->list_to_add), p);
			HT_DBG_FUNC((int)p, "post bell");
			sem_post(&(ut->bell));
		}
	}
	HT_DBG_FUNC_END(0, 0);
	return ret;
}
int content_remove_folder(struct ushare_t *ut, char *fullpath)
{
	LinkedList *list;
	ListNode * node;
	x_FP_ID *p;
	int removed = 0;

	HT_DBG_FUNC_START(HT_MOD_DMS, HT_BIT_FEW, 0, fullpath);
	list = &(ut->list_to_add);
	node = ListHead(list);
	if( node )	
	{
		p = (x_FP_ID*)(node->item);
		if( s_is_strings_equal( p->fullpath, fullpath ))//adding
		{
			ut->flag_adding = 0;
			p->del = 1;
			removed=2;
		}

		node = ListNext(list, node);
		while( node )
		{
			p = (x_FP_ID*)(node->item);
			if( s_is_strings_equal( p->fullpath, fullpath ))//to add
			{
				s_FPID_Release(p);
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

	list = &(ut->list_added);
	node = s_content_find_folder(list, fullpath);
	if( node )	//added
	{
		p = (x_FP_ID*)(node->item);
		s_upnp_entry_free_child(ut, ut->root_entry, p->entry);
		s_FPID_Release(p);
		ListDelNode(list, node, 0);
		removed = 1;
		goto s_EXIT;
	}

s_EXIT:
	HT_DBG_FUNC_END(removed, 0);
	return removed;
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
void build_metadata_list (struct ushare_t *ut)
{
	ut->rb = rbinit (s_redblack_compare, NULL);
	ut->root_entry = s_upnp_entry_new_dlna (ut, "root", NULL, NULL, NULL, true);
	ut->init = 1;
	
	typedef void *(* VFUNCV)(void *arg);
	pthread_t	p_tid;
	pthread_create( &p_tid, NULL, (VFUNCV)s_content_add_loop, NULL);
}

void free_metadata_list (struct ushare_t *ut)
{
	if(!ut)	return;

	ut->init = 0;

	int  i;
	C_UPNP_ENTRY *root = ut->root_entry;
	for(i = 0; i < root->child_count; i++ )
		s_upnp_entry_free_recursive(ut, root->childs[i]);

	s_upnp_entry_free_only_self(root);
	ut->root_entry = NULL;

	rbdestroy (ut->rb);
	ut->rb = NULL;
}

#endif


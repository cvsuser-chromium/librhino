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
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdbool.h>

#include "dlna.h"
#include "mime.h"
#include "hitTime.h"
#include "dms.h"
#include "ushare.h"


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

typedef long     time_t;    /* 时间值time_t 为长整型的别名*/
#endif


#define x_CHILDS_LIST_HIGH		0
#define x_CHILDS_LIST_LOW		1

/*--------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------*/
static C_DMS_AS *s_dms_share_usb_dmsas = NULL;
static char *getExtension (const char *filename)
{
	char *str =strrchr (filename, '.');
	if (str)
		str++;
	return str;
}

static bool is_valid_extension (const char *filename)
{
	char *extension = getExtension(filename);
	if(!extension)
		return false;

	if(getMimeType(extension))
		return true;

	return false;
}

static C_DMS_CCI *s_Dms_ShareUsb_MakeInfo_Container (char *fullpath, char *title)
{
	if(Dms_VirtualFileTree_IsFull())
		return NULL;
	
	C_DMS_CCI *container = (Dms_VirtualObject_CreateContainerSelf(fullpath, title, &Container_MIME_Type));
	return container;
}

static C_DMS_CMI *s_Dms_ShareUsb_MakeInfo_Item (char *fullpath, char *filename, char *title, struct stat *st)
{
	if(Dms_VirtualFileTree_IsFull())
		return NULL;
	
	if(!fullpath || !filename || !st)
		return NULL;
	
    while(s_dms_share_usb_dmsas && Dms_AsyncShare_Is_Paused(s_dms_share_usb_dmsas))
        usleep(50*1000);
	
	C_DMS_CMI *item = COO_OBJECT_NEW(C_DMS_CMI);
	if(!dlna_guess_usb_media_profile(ut->dlna, fullpath, item))
	{
		item->keyword		= strdup(filename);
		item->title			= title? strdup(title) : NULL;
		item->size 			= st->st_size;
		return item;
	}
	
	Dms_VirtualObject_FreeItem(item);
	return NULL;
}

static int s_Dms_ShareUsb_AddContainer_Loop(C_DMS_AS *dmsas, int parent_id, char *container)
{
	struct dirent **namelist;
	struct stat st;
	int n,i,run = 1;
	int	child_index;
	
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
		if(Dms_AsyncShare_Is_Aborted(dmsas))
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
				if(	is_valid_extension(namelist[i]->d_name))
				{
					C_DMS_CMI *item = s_Dms_ShareUsb_MakeInfo_Item(fullpath, namelist[i]->d_name, NULL, &st);
					C_DMS_VO *entry = Dms_VirtualObject_Create(parent_id, item, e_OBJTYPE_ITEM, e_SOURCE_USB, NULL, Dms_VirtualObject_FreeItem);
					{
						char *info = NULL;
						//Dms_VirtualObject_To_String(entry, 0, &info);
						if(info)
							free(info);
					}
					child_index = Dms_VirtualFileTree_AddChildEx(parent_id, x_CHILDS_LIST_LOW, entry);
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
		if(Dms_AsyncShare_Is_Aborted(dmsas))
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
				C_DMS_CCI *container = s_Dms_ShareUsb_MakeInfo_Container(fullpath, namelist[i]->d_name);
				C_DMS_VO *entry = Dms_VirtualObject_Create(parent_id, container, e_OBJTYPE_CONTAINER, e_SOURCE_USB, NULL, Dms_VirtualObject_FreeContainerSelf);
				{
					char *info = NULL;
					//Dms_VirtualObject_To_String(entry, 0, &info);
					if(info)
						free(info);
				}
				child_index = Dms_VirtualFileTree_AddChildEx(parent_id, x_CHILDS_LIST_LOW, entry);
				if(child_index > 0)
				{
					int x = s_Dms_ShareUsb_AddContainer_Loop(dmsas, child_index, fullpath);
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
static int s_Dms_ShareUsb_AddContainer(C_DMS_AS *dmsas, int parent_id, char *fullpath, int user)
{
	user = 0;
	return s_Dms_ShareUsb_AddContainer_Loop(dmsas, parent_id, fullpath);
}
int Raw_Dms_ShareUsb_AddFolder(int parent_id, char *fullpath)
{
	return Dms_AsyncShare_AddObject(s_dms_share_usb_dmsas, parent_id, fullpath, 0);
}

int Raw_Dms_ShareUsb_RemoveFolder(int parent_id, char *fullpath)
{
	return Dms_AsyncShare_RemoveObject(s_dms_share_usb_dmsas, parent_id, fullpath);
}

int Raw_Dms_SetPause(int mode)
{
	HT_DBG_FUNC_START(HT_MOD_DMS, HT_BIT_KEY, mode, 0);
	Dms_AsyncShare_SetPause(s_dms_share_usb_dmsas, mode);
	HT_DBG_FUNC_END(mode, 0);
	return mode;
}

void Dms_ShareUsb_Init (void)
{
	s_dms_share_usb_dmsas = Dms_AsyncShare_Init(s_Dms_ShareUsb_AddContainer);
}

int Dms_VirtualFileTree_GetUsbObjectFullpath(int id, char **path)
{
	C_DMS_VO *entry = Dms_VirtualFileTree_GetObject(id);
	if(entry)
	{
		char fullpath[1024];
		C_DMS_CCI *container = entry->parent->object;
		C_DMS_CMI *usb = entry->object;
		if(container)
			sprintf(fullpath, "%s/%s", container->keyword, usb->keyword);
		else
			sprintf(fullpath, "/%s", usb->keyword);
		*path = strdup(fullpath);
		return 0;
	}
	
	return -1;
}

/*--------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------*/



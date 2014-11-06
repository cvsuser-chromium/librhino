/*
 * http.c : GeeXboX uShare Web Server handler.
 * Originally developped for the GeeXboX project.
 * Parts of the code are originated from GMediaServer from Oskar Liljeblad.
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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include "upnp.h"
#include "ushare.h"

#include "cds.h"
#include "cms.h"
#include "http.h"
#include "trace.h"

#include "hitTime.h"
#include "coo.h"
#include "dms.h"


#define SERVICE_CONTENT_TYPE "text/xml"

static C_DMS_VFO *s_dms_pvr_op = NULL;

struct web_file_t {
	int 		pvr;
	char		*fullpath;
	off_t 		file_length;
	off_t		pos;
	
	enum {
		FILE_LOCAL,
		FILE_MEMORY
	} type;
	
	union {
		struct {
			int		fd;
			int		xx;
		} local;
		struct {
			char	*contents;
			off_t	len;
		} memory;
	} detail;
};


static inline void set_info_file (struct File_Info *info, const size_t length, const char *content_type)
{
	info->file_length 		= length;
	info->last_modified		= 0;
	info->is_directory		= 0;
	info->is_readable		= 1;
	info->content_type		= ixmlCloneDOMString (content_type);
}

static int _http_get_local_file_info (const char *filename, struct File_Info *info)
{
	struct stat st;
	int ret = -1;
	char *fullpath = NULL;
	const char *mime;

	int upnp_id = object_id_to_idx (strrchr (filename, '/') + 1);
	C_UPNP_ENTRY *entry = Dms_VirtualFileTree_GetObject(upnp_id);
	if(!entry || !Dms_VirtualObject_IsItem(entry))
		goto s_EXIT;
	
	if(entry->source_type == e_SOURCE_USB)
	{
		C_DMS_CMI *usb = entry->object;
		
		ret = Dms_VirtualFileTree_GetUsbObjectFullpath(upnp_id, &fullpath);
		if(!fullpath)
			goto s_EXIT;

		if(stat(fullpath, &st) < 0)
			goto s_EXIT;

		if(access(fullpath, R_OK) < 0)
		{
			if (errno != EACCES)
				goto s_EXIT;
			info->is_readable = 0;
		}
		else
			info->is_readable = 1;	
		
		/* file exist and can be read */
		info->file_length		= st.st_size;
		info->last_modified		= st.st_mtime;
		info->is_directory		= S_ISDIR (st.st_mode);
		mime = (usb->protocol_info && usb->protocol_info->mime)? usb->protocol_info->mime : "";
		info->content_type		= ixmlCloneDOMString (mime);
		ret = 0;
	}
	else if(DMS_OBJCT_IS_PVR_VOD(entry))
	{
		C_DMS_CMI *pvr = entry->object;
		info->is_readable = 1;	
		
		/* file exist and can be read */
		info->file_length 		= pvr->size;
		info->last_modified 	= st.st_mtime;
		info->is_directory 		= 0;
		mime = (pvr->protocol_info && pvr->protocol_info->mime)? pvr->protocol_info->mime : "";
		info->content_type 		= ixmlCloneDOMString (mime);
		ret = 0;
		
		C_FILE_INFO cfi;
		bzero(&cfi, sizeof(C_FILE_INFO));
		ret = s_dms_pvr_op->GetInfo(pvr->keyword, &cfi);
		info->file_length		= cfi.file_length;
	}
	else
	{
	}
	
s_EXIT:
	if(fullpath)
		free(fullpath);
	return ret;
}

static int _http_get_info (const char *filename, struct File_Info *info)
{
	if (!filename || !info)
		return -1;

	if (!strcmp (filename, CDS_LOCATION))
	{
		set_info_file (info, CDS_DESCRIPTION_LEN, SERVICE_CONTENT_TYPE);
		return 0;
	}

	if (!strcmp (filename, CMS_LOCATION))
	{
		set_info_file (info, CMS_DESCRIPTION_LEN, SERVICE_CONTENT_TYPE);
		return 0;
	}
#if 0
	if (!strcmp (filename, MSR_LOCATION))
	{
		set_info_file (info, MSR_DESCRIPTION_LEN, SERVICE_CONTENT_TYPE);
		return 0;
	}
#endif
	int ret;
	Dms_WaitLock();
	ret = _http_get_local_file_info(filename, info);
	Dms_PostLock();
	return ret;
}

static int http_get_info (const char *filename, struct File_Info *info)
{
	int rc;
	HT_DBG_FUNC_START(HT_MOD_DMS, HT_BIT_KEY, 0,filename);
	rc = _http_get_info(filename, info);
	HT_DBG_FUNC_END(rc, 0);
	return rc;
}

static UpnpWebFileHandle get_file_memory (const char *fullpath, const char *description, const size_t length)
{
  struct web_file_t *file;

  file = malloc (sizeof (struct web_file_t));
  file->fullpath = strdup (fullpath);
  file->pos = 0;
  file->type = FILE_MEMORY;
  file->detail.memory.contents = strdup (description);
  file->detail.memory.len = length;

  return ((UpnpWebFileHandle) file);
}

static UpnpWebFileHandle _http_open_local_file(const char *filename)
{
	struct web_file_t *file = NULL;
	C_UPNP_ENTRY *entry;

	int upnp_id = object_id_to_idx (strrchr (filename, '/') + 1);
	HT_DBG_FUNC_START(HT_MOD_DMS, HT_BIT_MANY, upnp_id, filename);
	entry = Dms_VirtualFileTree_GetObject(upnp_id);
	if(!entry || !Dms_VirtualObject_IsItem(entry))
		goto s_EXIT;

	HT_DBG_FUNC(entry->source_type, 0);
	if(entry->source_type == e_SOURCE_USB)
	{
		char *fullpath = NULL;
		int fd = Dms_VirtualFileTree_GetUsbObjectFullpath(upnp_id, &fullpath);
		if(!fullpath)
			goto s_EXIT;
		
		fd = open (fullpath, O_RDONLY | O_NONBLOCK | O_SYNC | O_NDELAY);
		if (fd < 0)
		{
			free(fullpath);
			goto s_EXIT;
		}
		
		file = COO_OBJECT_NEW(struct web_file_t);
		file->fullpath			= fullpath;
		file->pos				= 0;
		file->type				= FILE_LOCAL;
		file->detail.local.fd	= fd;
	}
	else if(DMS_OBJCT_IS_PVR_VOD(entry))
	{
		C_DMS_CMI *pvr = entry->object;
		
		if(!s_dms_pvr_op)
			goto s_EXIT;

		int fd = s_dms_pvr_op->Open(pvr->keyword, 0);
		if(fd == 0)
			goto s_EXIT;

		file = COO_OBJECT_NEW(struct web_file_t);
		file->fullpath			= strdup(pvr->keyword);
		file->pos				= 0;
		file->type				= FILE_LOCAL;
		file->detail.local.fd	= fd;
		file->file_length		= pvr->size;
		file->pvr				= 1;
	}
	else
	{
	}
	
s_EXIT:
	HT_DBG_FUNC_END((int)file, 0);
	return ((UpnpWebFileHandle) file);
}


static UpnpWebFileHandle _http_open (const char *filename, enum UpnpOpenFileMode mode)
{
	if(ut->stop_http)
		return NULL;

	if (!filename)
		return NULL;

	if (mode != UPNP_READ)
		return NULL;

	if (!strcmp (filename, CDS_LOCATION))
		return get_file_memory (CDS_LOCATION, CDS_DESCRIPTION, CDS_DESCRIPTION_LEN);

	if (!strcmp (filename, CMS_LOCATION))
		return get_file_memory (CMS_LOCATION, CMS_DESCRIPTION, CMS_DESCRIPTION_LEN);
#if 0
	if (!strcmp (filename, MSR_LOCATION))
		return get_file_memory (MSR_LOCATION, MSR_DESCRIPTION, MSR_DESCRIPTION_LEN);
#endif
	UpnpWebFileHandle ret = 0;
	Dms_WaitLock();
	ret = _http_open_local_file(filename);
	Dms_PostLock();
	return ret;
}

static UpnpWebFileHandle http_open (const char *filename, enum UpnpOpenFileMode mode)
{
	UpnpWebFileHandle rc;
	HT_DBG_FUNC_START(HT_MOD_DMS, HT_BIT_KEY, mode, filename);
	rc = _http_open(filename, mode);
	HT_DBG_FUNC_END((int)rc, 0);
	return rc;
}
static int http_read (UpnpWebFileHandle fh, char *buf, size_t buflen)
{
	struct web_file_t *file = (struct web_file_t *) fh;
	ssize_t len = -1;

	if(ut->stop_http)
		return -1;

	if (!file)
		return -1;

	switch (file->type)
	{
		case FILE_LOCAL:
			if( file->pvr)
			{
				if(s_dms_pvr_op)
					len = s_dms_pvr_op->Read(file->detail.local.fd, buf, buflen);
				break;
			}
			len = read (file->detail.local.fd, buf, buflen);
			break;

		case FILE_MEMORY:
			len = (size_t) MIN (buflen, file->detail.memory.len - file->pos);
			memcpy (buf, file->detail.memory.contents + file->pos, (size_t) len);
			break;

		default:
			break;
	}

	if (len >= 0)
		file->pos += len;

	return len;
}

static int http_ReadEx (UpnpWebFileHandle fh, char *buf, size_t buflen, int *flag)
{
	struct web_file_t *file = (struct web_file_t *) fh;
	ssize_t len = -1;

	if(ut->stop_http)
		return -1;

	if (!file)
		return -1;

	if(flag)
		*flag = 0;
	
	switch (file->type)
	{
		case FILE_LOCAL:
			if( file->pvr)
			{
				if(s_dms_pvr_op)
				{
					len = s_dms_pvr_op->ReadEx(file->detail.local.fd, buf, buflen, flag);
					if (len >= 0)
						file->pos += len;
				}
				break;
			}
			len = http_read(fh, buf, buflen);
			break;

		case FILE_MEMORY:
			len = http_read(fh, buf, buflen);
			break;

		default:
			break;
	}

	return len;
}

static int http_write (UpnpWebFileHandle fh __attribute__((unused)),
            char *buf __attribute__((unused)),
            size_t buflen __attribute__((unused)))
{
  log_verbose ("http write\n");

  return 0;
}

static int http_seek (UpnpWebFileHandle fh, off_t offset, int origin)
{
  struct web_file_t *file = (struct web_file_t *) fh;
  off_t newpos = -1;

  if(ut->stop_http)
      return -1;

  if (!file)
    return -1;

  switch (origin)
  {
  case SEEK_SET:
    log_verbose ("Attempting to seek to %lld (was at %lld) in %s\n",
                offset, file->pos, file->fullpath);
    newpos = offset;
    break;
  case SEEK_CUR:
    log_verbose ("Attempting to seek by %lld from %lld in %s\n",
                offset, file->pos, file->fullpath);
    newpos = file->pos + offset;
    break;
  case SEEK_END:
    log_verbose ("Attempting to seek by %lld from end (was at %lld) in %s\n",
                offset, file->pos, file->fullpath);

    if (file->type == FILE_LOCAL)
    {
	  if( file->pvr )
	  {
		newpos = file->file_length+ offset;
		  break;
	  }
	  
      struct stat sb;
      if (stat (file->fullpath, &sb) < 0)
      {
        log_verbose ("%s: cannot stat: %s\n",
                    file->fullpath, strerror (errno));
        return -1;
      }
      newpos = sb.st_size + offset;
    }
    else if (file->type == FILE_MEMORY)
      newpos = file->detail.memory.len + offset;
    break;
  }

  switch (file->type)
  {
  case FILE_LOCAL:
    /* Just make sure we cannot seek before start of file. */
    if (newpos < 0)
    {
      log_verbose ("%s: cannot seek: %s\n", file->fullpath, strerror (EINVAL));
      return -1;
    }

	if( file->pvr )
	{
		if(s_dms_pvr_op && s_dms_pvr_op->Seek)
		{
			if(s_dms_pvr_op->Seek(file->detail.local.fd, newpos, SEEK_SET) < 0 )
			{
				return -1;
			}
		}
		else
			return -1;
		break;
	}
    /* Don't seek with origin as specified above, as file may have
       changed in size since our last stat. */
    if (lseek (file->detail.local.fd, newpos, SEEK_SET) == -1)
    {
      log_verbose ("%s: cannot seek: %s\n", file->fullpath, strerror (errno));
      return -1;
    }
    break;
  case FILE_MEMORY:
    if (newpos < 0 || newpos > file->detail.memory.len)
    {
      log_verbose ("%s: cannot seek: %s\n", file->fullpath, strerror (EINVAL));
      return -1;
    }
    break;
  }

  file->pos = newpos;

  return 0;
}

static int http_close (UpnpWebFileHandle fh)
{
	struct web_file_t *file = (struct web_file_t *) fh;

	if (!file)
		return -1;

	switch (file->type)
	{
		case FILE_LOCAL:
			if( file->pvr )
			{
				if(s_dms_pvr_op)
					s_dms_pvr_op->Close(file->detail.local.fd);
				break;
			}
			close (file->detail.local.fd);
			break;
			
		case FILE_MEMORY:
			/* no close operation */
			if (file->detail.memory.contents)
				free (file->detail.memory.contents);
			break;
			
		default:
			break;
	}

	if (file->fullpath)
		free (file->fullpath);
	free (file);

	return 0;
}

struct UpnpVirtualDirCallbacks virtual_dir_callbacks =
{
	http_get_info,
	http_open,
	http_read,
	http_write,
	http_seek,
	http_close,
	http_ReadEx
};

int Dms_PresetPvrFileOperation(C_DMS_VFO *op) /* ret = 0: success; -1: op is null; -2: at least one of Init, Open, Read and Close is null */ 
{
	if(!op)
		return -1;

	if(	   !(op->Init)
		|| !(op->Open)
		|| !(op->Read)
		|| !(op->Close) )
		return -2;
	
	s_dms_pvr_op = op;
	return 0;
}

void Dms_PvrFileOperation_Init(void)
{
	if(s_dms_pvr_op && s_dms_pvr_op->Init)
		s_dms_pvr_op->Init(1);
}


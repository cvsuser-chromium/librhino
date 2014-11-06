/*
 * metadata.h : GeeXboX uShare CDS Metadata DB header.
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

#ifndef _METADATA_H_
#define _METADATA_H_

#include <stdbool.h>
#include <sys/types.h>

#include "ushare.h"
#include "json.h"
#include "dlna_type.h"

typedef struct upnp_entry_t C_UPNP_ENTRY;
struct upnp_entry_t {
	int 				id;
	char 				*fullpath;
	char 				*title;
	
	C_UPNP_ENTRY 		*parent;
	struct upnp_entry_t	**childs;
	int					childs_len;
	int					container_count;
	int 				child_count;

	C_UD_MI				minfo;
	struct mime_type_t	*mime_type;
	dlna_profile_t		*protocol_info;

	int 				pvr;
	unsigned int		updateID;
};

C_UPNP_ENTRY *upnp_get_entry (struct ushare_t *ut, int id);
C_UPNP_ENTRY *Dms_VirtualFileTree_GetObject (int id);
#if 0
void build_metadata_list (struct ushare_t *ut);
void free_metadata_list (struct ushare_t *ut);

int content_add_pvr_item(struct ushare_t *ut, char *path, char *itemName);
int content_remove_pvr_item(struct ushare_t *ut, char *path, char *itemName);
int content_find_folder(struct ushare_t *ut, char *fullpath);
int content_add_folder(struct ushare_t *ut, char *fullpath, char *title, int is_pvr);
int content_remove_folder(struct ushare_t *ut, char *fullpath);
#endif
#endif /* _METADATA_H_ */


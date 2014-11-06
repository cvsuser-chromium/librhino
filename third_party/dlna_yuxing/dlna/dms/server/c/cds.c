/*
 * cds.c : GeeXboX uShare Content Directory Service
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

#include <stdlib.h>
#include "upnp.h"
#include "upnptools.h"

#include "ushare.h"
#include "services.h"
#include "mime.h"
#include "buffer.h"
#include "minmax.h"

#include "hitTime.h"
#include "cds.h"
#include "coo.h"
#include "dms.h"


extern int dlna_write_protocol_info(char *buf, int buf_len, dlna_org_flags_t flags, C_DMS_IPI *p);

/*---------------------------------------------------------------------*/

#define CDS_ERR_NULL						0
#define CDS_ERR_NO_SUCH_OBJECT				701
#define CDS_ERR_INVALID_SEARCH_CRITERIA		708
#define CDS_ERR_INVALID_SORT_CRITERIA		709
#define CDS_ERR_NO_SUCH_CONTAINER			710
#define CDS_ERR_CANNOT_PROCESS_REQUEST		720

#define CDS_ERR_INVALID_ARGS      			402
#define CDS_ERR_OUT_OF_MEMORY				603

#if 0
#define bool 	int
#define true 	1
#define false	0
#endif
#define XML_LEN_MAX			190*1024
#define CDS_CHAR_SPACE  	' '
#define CDS_CHAR_BRA		'('
#define CDS_CHAR_KET		')'
#define CDS_CHAR_QUOTE		'\"'

typedef enum{
	x_PARSE_ERROR = -1,
	x_PARSE_UNSUPPORTED,
	x_PARSE_OK,		
}x_PARSE;

/*---------------------------------------------------------------------*/
typedef enum _p_base_
{
	P_BASE_id = 0,
	P_BASE_parentID,
	P_BASE_refID,
	P_BASE_restricted,
	P_BASE_searchable,
	P_BASE_childCount,

	P_DC_title,
	P_DC_creator,
	P_DC_date,
	P_DC_description,

	P_UPNP_class,
	P_UPNP_artist,
	P_UPNP_actor,
	P_UPNP_author,
	P_UPNP_producer,
	P_UPNP_recordedStartDateTime,
	
	P_RES_any,
	P_RES_protocolInfo,
	P_RES_importUri,
	P_RES_size,
	P_RES_duration,
	P_RES_protection,
	P_RES_bitrate,
	P_RES_bitsPerSample,
	P_RES_sampleFrequency,
	P_RES_nrAudioChannels,
	P_RES_resolution,
	P_RES_colorDepth,

	P_BASE_MAX,
}P_BASE;
typedef struct _x_filter_property_  X_FILTER;
struct _x_filter_property_ {
	P_BASE		filter_type;
	char		*property;
};
static const X_FILTER s_filter_keyword[] = {
	{P_BASE_id,				"@id"},
	{P_BASE_parentID, 		"@parentID"},
	{P_BASE_refID,			"@refID"},
	{P_BASE_restricted,		"@restricted"},
	{P_BASE_searchable,		"@searchable"},
	{P_BASE_childCount,		"@childCount"},
		
	{P_DC_title,			"dc:title"},
	{P_DC_creator, 			"dc:creator"},
	{P_DC_date,				"dc:date"},
	{P_DC_description, 		"dc:description"},

	{P_UPNP_class,			"upnp:class"},
	{P_UPNP_artist, 		"upnp:artist"},
	{P_UPNP_actor,			"upnp:actor"},
	{P_UPNP_author,			"upnp:author"},
	{P_UPNP_producer,		"upnp:producer"},
	{P_UPNP_recordedStartDateTime,		"upnp:recordedStartDateTime"},

	{P_RES_any,				"res"},
	{P_RES_protocolInfo,	"res@protocolInfo"},
	{P_RES_importUri,		"res@importUri"},
	{P_RES_size,			"res@size"},
	{P_RES_duration, 		"res@duration"},
	{P_RES_protection, 		"res@protection"},
	{P_RES_bitrate, 		"res@bitrate"},
	{P_RES_bitsPerSample, 	"res@bitsPerSample"},
	{P_RES_sampleFrequency, "res@sampleFrequency"},
	{P_RES_nrAudioChannels, "res@nrAudioChannels"},
	{P_RES_resolution,		"res@resolution"},
	{P_RES_colorDepth,		"res@colorDepth"},
		{-1, NULL}, /*end identifier */
};

/*---------------------------------------------------------------------*/
typedef enum{
	x_PROPERTY_UPNPCLASS = 0,		
	x_PROPERTY_REFID,
	x_PROPERTY_TITLE,
	
	x_PROPERTY_RES_PROTOCOL,
	x_PROPERTY_RES_SIZE,
	x_PROPERTY_RES_BITRATE,
	x_PROPERTY_RES_DURATION,
	x_PROPERTY_RES_RESOLUTION,
}x_PROPERTY;
typedef struct _x_sech_kwd X_SECH_KWD;
struct _x_sech_kwd {
	x_PROPERTY	pro_type;
	char		*property;
};
static const X_SECH_KWD s_property_keyword[] = {
	{x_PROPERTY_UPNPCLASS,		"upnp:class"},
	{x_PROPERTY_REFID, 			"@refID"},
	{x_PROPERTY_TITLE,			"dc:title"},
		
	{x_PROPERTY_RES_SIZE,		"res@size"},
	{x_PROPERTY_RES_BITRATE,	"res@bitrate"},
		{-1, NULL}, /*end identifier */
	{x_PROPERTY_RES_PROTOCOL,	"res@protocolInfo"},
	{x_PROPERTY_RES_DURATION,	"res@duration"},
	{x_PROPERTY_RES_RESOLUTION,	"res@resolution"},
};

/*---------------------------------------------------------------------*/
typedef enum{
	x_OP_EQUAL = 0,		
	x_OP_NOT_EQUAL,
	x_OP_LITTLER,
	x_OP_LITTLER_EQU,
	x_OP_BIGGER,
	x_OP_BIGGER_EQU,
	
	x_OP_CONTAIN,
	x_OP_NOT_CONTAIN,
	x_OP_DERIVED_FROM,
	
	x_OP_EXIST,
}x_OP;

typedef struct _x_op_kwd X_OP_KWD;
struct _x_op_kwd {
	x_OP		op_type;
	char		*op;
};
static const X_OP_KWD s_op_keyword[] = {
	{x_OP_EQUAL,				"="},
	{x_OP_NOT_EQUAL, 			"!="},
	{x_OP_LITTLER_EQU,			"<="},
	{x_OP_LITTLER,				"<"},
	{x_OP_BIGGER_EQU		,	">="},
	{x_OP_BIGGER,				">"},
		
	{x_OP_CONTAIN,				"contains"},
	{x_OP_NOT_CONTAIN,			"doesNotContain"},
	{x_OP_DERIVED_FROM,			"derivedfrom"},
		
	{x_OP_EXIST,				"exists"},
	{-1, NULL}, /*end identifier */
};

/*---------------------------------------------------------------------*/
typedef struct _x_reg_exp X_REG_EXP;
struct _x_reg_exp {
	int			node_type;	/* 0: dot; 1: set */

	/* for dot */
	x_PROPERTY	pro_type;
	char		*property;
	x_OP		op_type;
	char		*op;
	char		*value;

	/* for set */
	LinkedList	set;

	/* public */
	X_REG_EXP	*pAnd;
};

typedef struct _x_parse_exp X_PARSE_EXP;
struct _x_parse_exp {
	int			node_type;
	char		*start;
	char		*end;
};

typedef struct _upnp_cds_info_
{
//	struct buffer_t		*output;
//	int					total;
//	int					result_count;
	
	int 				index;
	int					count;
	
	char				*search_criteria;
	void				*search_tree;
	
	char				*sort_criteria;
	int 				sort_array[P_BASE_MAX+2];

	char 				*filter_criteria;
	int 				filter_array[P_BASE_MAX+2];

	int					err_code;
}x_UCSI;


/*--------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------*/
static void s_cds_free_ucsi(x_UCSI *ucsi)
{
	if(ucsi->search_criteria)					
		free(ucsi->search_criteria);
	
	if(ucsi->sort_criteria)					
		free(ucsi->sort_criteria);
	
	if(ucsi->filter_criteria)					
		free(ucsi->filter_criteria);
}
#if 0
void object_id_from_idx(char *buf, int id, char *pvr_type)
{
	if(pvr_type)
//		sprintf(buf, "%d.hw.%s", id, pvr_type);
		sprintf(buf, "%d.hw.%s", id, pvr_type);
	else
		sprintf(buf, "%d", id);
}
#endif
int object_id_to_idx(char* object_id)
{
	int id;
	char *dot = strchr(object_id, '.');/* for compliant with id.pvr */

	if(dot)
		*dot = 0;
	id = atoi(object_id);
	if(dot)
		*dot = '.';

	return id;
}

/*--------------------------------------------------------------------------------------------*/
static int s_cds_parse_filter (char *filter_criteria, int *array)
{
	int count;
	char *p, *s = filter_criteria;
	const X_FILTER	*list;
	
	HT_DBG_FUNC_START(HT_MOD_DMS, HT_BIT_MANY, 0, s);

	s = coo_strtrim_ends(s);
	if(!strcmp(s, "*"))
	{
		array[P_BASE_MAX] = 1;
		count = -1;
		goto s_EXIT;
	}
	
//	memset(array, 0, array_len * 4);
	count = 0;
	while(*s)
	{
		p = strchr(s, ',');
		if(p)
			*p = 0;

		HT_DBG_FUNC(count, s);
		s = coo_strtrim_ends(s);
		list = s_filter_keyword;
		while(list->property)
		{
			if(!strcasecmp(list->property, s))
			{
				if(!array[list->filter_type])
				{
					array[list->filter_type] = 1;
					count++;

					if(!strncasecmp(s, "res@", 4))
					{
						if(array[P_RES_any] != 1)
							array[P_RES_any] = 2;
					}
				}
				break;
			}
			list++;
		}

		if(p)
			s = p + 1;
		else
			break;
	}
	
s_EXIT:
	HT_DBG_FUNC_END(count, 0);
	return count;
}

static bool filter_is_star (int *array)
{
	return array[P_BASE_MAX];
}
static bool filter_has_val (int *array, int index)
{
	return (array[index]==1);
}
static bool filter_relate_to_val (int *array, int index)
{
	return (array[index]==2);
}
static char *filter_get_str (int index)
{
	const X_FILTER	*list = &(s_filter_keyword[index]);
	return list->property;
}

/*--------------------------------------------------------------------------------------------*/
static void didl_add_header (struct buffer_t *out)
{
  buffer_appendf (out, "<%s %s>", DIDL_LITE, DIDL_NAMESPACE);
}
static void didl_add_footer (struct buffer_t *out)
{
  buffer_appendf (out, "</%s>", DIDL_LITE);
}
static void didl_add_param (struct buffer_t *out, char *param, char *value)
{
  if (value)
    buffer_appendf (out, " %s=\"%s\"", param, value);
}
static void didl_add_value (struct buffer_t *out, char *param, off_t value)
{
  buffer_appendf (out, " %s=\"%lld\"", param, value);
}
static void didl_add_full_param (struct buffer_t *out, char *param, char *value)
{
  buffer_appendf (out, "<%s>%s</%s>", param, value, param);
}

char *s_dms_convert_xml (char *src, char *dest, int dest_len)
{
	int ret, num = 0;
	ret = coo_str_GetXmlPatternChar(src, &num);
	if(!num)
		return src;
	coo_str_to_xml(src, dest, &dest_len);
	return dest;
}

//extern char *dms_convert_xml (char *title);
static int didl_add_pvr_res(struct buffer_t *out, char *filter, C_UPNP_ENTRY *p_entry)
{
    char *pold, buf[1024];
	C_DMS_CMI *pvr_entry = (C_DMS_CMI *)(p_entry->object);
	if(!pvr_entry->extension)
		return -1;	
	filter = NULL;
	
    json_object_object_foreach(pvr_entry->extension, key, val) 	{
 //       printf("\t%s: %s\n", key, json_object_get_string(val));
        pold = (char*)json_object_get_string(val);
        if(pold)
        {
            char *pnew = s_dms_convert_xml(pold, buf, sizeof(buf));
            didl_add_param(out, key, pnew);
//            didl_add_param(out, key, pold);
        }
    }
    return 0;
}

#define RES_PREFIX_LEN	4
static int didl_add_standard_res(struct buffer_t *out, int *array, C_UPNP_ENTRY *p_entry, int must)
{
	char temp[64];
	int index;
	C_DMS_CMI *entry = p_entry->object;
	
	index = P_RES_size;
	if(must || filter_has_val(array, index))
		didl_add_value(out, filter_get_str(index)+RES_PREFIX_LEN, entry->size);

	index = P_RES_bitrate;
	if(must || filter_has_val(array, index))
	{
		if( entry->bitrate > 0 )
		{
			sprintf( temp, "%d", entry->bitrate);
			didl_add_param(out, filter_get_str(index)+RES_PREFIX_LEN, temp);
		}
	}
	
	index = P_RES_resolution;
	if(must || filter_has_val(array, index))
	{
		if( entry->res_width>0 && entry->res_height>0 )
		{
			sprintf( temp, "%dx%d", entry->res_width, entry->res_height);
			didl_add_param(out, filter_get_str(index)+RES_PREFIX_LEN, temp);
		}
	}

	index = P_RES_duration;
	if(must || filter_has_val(array, index))
	{
		if( entry->duration > 0)
		{
			int total, h,m,s;
			total = entry->duration/1000;
			h = total/3600;
			s = (total%3600);
			m = s/60;
			s = s%60;
			sprintf( temp, "%d:%d:%d", h, m, s);
			didl_add_param(out, filter_get_str(index)+RES_PREFIX_LEN, temp);
		}
	}
	
	return 0;
}

static void s_make_object_id(C_DMS_VO *entry, char *buf, int buf_len)
{
	if(entry->MakeObjectId)
		entry->MakeObjectId(entry, entry->id, buf, buf_len);
	else
		sprintf(buf, "%d", entry->id);
}

const char *dlna_get_upnp_class(int upnp_class);
#define DC_PREFIX_LEN	3
#define UPNP_PREFIX_LEN	5
static void s_didl_add_item (struct buffer_t *out, C_UPNP_ENTRY *p_entry, char *protocol_info, char *restricted, x_UCSI *ucsi)
{
	const char *s;
	C_DMS_CMI *entry = p_entry->object;
	off_t parentId =  p_entry->parent? p_entry->parent->id : -1;
	const char *profile = dlna_get_upnp_class (entry->protocol_info->class);
	int index, *array = ucsi->filter_array;
	char *title, *dot = NULL, *pp, id[128], buf[1024];

	s_make_object_id(p_entry, id, sizeof(id));
//	s = "<item id=\"%s\" parentID=\"%lld\" restricted=\"%s\"><upnp:class>%s</upnp:class><upnp:recordedStartDateTime>%s</upnp:recordedStartDateTime><dc:title>%s</dc:title><dc:description>%s</dc:description>";
	s = "<item id=\"%s\" parentID=\"%lld\" restricted=\"%s\"><upnp:class>%s</upnp:class><dc:title>%s</dc:title>";
	if(p_entry->source_type == e_SOURCE_USB)
	{
		title = entry->keyword;
		dot = strrchr(title, '.');
		if(dot && (dot != title))
			*dot = 0;
	}
	else
		title = entry->title;
	buffer_appendf(out, s, id, parentId, restricted, profile, s_dms_convert_xml(title, buf, sizeof(buf)));
	if(p_entry->source_type == e_SOURCE_USB)
	{
		if(dot)
			*dot = '.';
	}

	index = P_DC_description;
	if(filter_is_star(array) || filter_has_val(array, index))
	{
		if(entry->optional && entry->optional->upnp_info &&  (pp = entry->optional->upnp_info->description))
			didl_add_full_param(out, filter_get_str(index)+DC_PREFIX_LEN, s_dms_convert_xml(pp, buf, sizeof(buf)));
	}

	index = P_UPNP_recordedStartDateTime;
	if(filter_is_star(array) || filter_has_val(array, index))
	{
		if(entry->optional && entry->optional->pvr && (pp = entry->optional->pvr->recordedStartDateTime))
			didl_add_full_param(out, filter_get_str(index)+UPNP_PREFIX_LEN, s_dms_convert_xml(pp, buf, sizeof(buf)));
	}

	if(filter_is_star(array) || filter_has_val(array, P_RES_any))
	{
		// protocolInfo is required :
		if(!protocol_info)
		{
			dlna_write_protocol_info(buf, sizeof(buf), ut->dlna_flags, (C_DMS_IPI *)(entry->protocol_info));
			protocol_info = buf;
		}
		buffer_appendf (out, "<res protocolInfo=\"%s\"", protocol_info);

		didl_add_standard_res(out, array, p_entry, 1);
		didl_add_pvr_res(out, ucsi->filter_criteria, p_entry);
		
		buffer_appendf (out, ">http://%s:%d%s/%s</res>", UpnpGetServerIpAddress(), ut->port, VIRTUAL_DIR, id);
	}
 	else if(filter_relate_to_val(array, P_RES_any))
	{
		// protocolInfo is required :
		if(!protocol_info)
		{
			dlna_write_protocol_info(buf, sizeof(buf), ut->dlna_flags, (C_DMS_IPI *)(entry->protocol_info));
			protocol_info = buf;
		}
		buffer_appendf (out, "<res protocolInfo=\"%s\"", protocol_info);
		
		didl_add_standard_res(out, array, p_entry, 0);
		
		buffer_appendf (out, ">http://%s:%d%s/%s</res>", UpnpGetServerIpAddress(), ut->port, VIRTUAL_DIR, id);
	}
	else
	{
	}
	
	buffer_appendf (out, "</item>");
}
static void s_didl_add_container (struct buffer_t *out, C_UPNP_ENTRY *p_entry, char *searchable, char *restricted)
{
	char *s, parent_id[128], my_id[128], buf[1024];
	int parentId =  p_entry->parent? p_entry->parent->id : -1;
	C_DMS_CCI *entry = (C_DMS_CCI *)(p_entry->object);
	
	s_make_object_id(p_entry, my_id, sizeof(my_id));
	HT_DBG_FUNC_START(HT_MOD_DMS, HT_BIT_MYRIAD, p_entry->id, my_id);
	int num, child_count = 0;
	for(num = 0; num < x_DMS_CHILDS_LISTS_NUM; num++)
	{
		COO_ARRAY *ca = entry->childs_lists[num];
		if(ca)
			child_count += coo_array_real_count(ca);
	}

	if(p_entry->parent)
		s_make_object_id(p_entry->parent, parent_id, sizeof(parent_id));
	else
		sprintf(parent_id, "%d", parentId);
	HT_DBG_FUNC(parentId, parent_id);
	
	HT_DBG_FUNC(child_count, entry->mime_type->upnp_class);
	
	char *pnew = s_dms_convert_xml(entry->title, buf, sizeof(buf));
	HT_DBG_FUNC(0, pnew);
	s = "<container id=\"%s\" parentID=\"%s\" childCount=\"%d\" restricted=\"%s\" searchable=\"%s\"><upnp:class>%s</upnp:class><dc:title>%s</dc:title></container>";
	buffer_appendf(out, s, my_id, parent_id, child_count, restricted, searchable, entry->mime_type->upnp_class, pnew);
	HT_DBG_FUNC(0, pnew);
	HT_DBG_FUNC_END(child_count, out->buf);
}

static int s_cds_make_result (struct action_event_t *event, C_UPNP_ENTRY *parent, x_UCSI *ucsi, COO_ARRAY *ca)
{
//	char buf[32];
	int i, ca_count, length, result_count, xml_len_max;
	C_UPNP_ENTRY *entry;
	struct buffer_t *out = buffer_new ();
	
	/* max length of xml packet */
	xml_len_max = (event->request->xmlMaxLen < XML_LEN_MAX)? event->request->xmlMaxLen : XML_LEN_MAX;

	length = 0;
	result_count = 0;
	ca_count = coo_array_real_count(ca);
	
	HT_DBG_FUNC_START(HT_MOD_DMS, HT_BIT_KEY, ca_count, 0);
	
	didl_add_header(out);
	for(i = ucsi->index; i < ca_count; i++)
	{
		/* UPnP CDS compliance : If starting index = 0 and requested count = 0  then all children must be returned */
		if( ucsi->count && (result_count >= ucsi->count) )/* only fetch the requested count number or all entries if count = 0 */
			break;

		struct buffer_t *tmp = buffer_new ();
		if(tmp)
		{
			entry = (C_UPNP_ENTRY *)coo_array_get(ca, i);
			if(Dms_VirtualObject_IsItem(entry))	/* item */
				s_didl_add_item(tmp, entry, NULL, "true", ucsi);
			else if(Dms_VirtualObject_IsContainer(entry))	/* container */
				s_didl_add_container(tmp, entry, "true", "true");
			else
			{
			}

			int num = 0;
			int slen = coo_str_GetXmlPatternChar(tmp->buf, &num);
			if(slen > 0)
				slen = length + tmp->len + (slen - num);
			else
				slen = length + tmp->len;

			if(slen < xml_len_max)
			{
				buffer_appendf(out, "%s", tmp->buf);
				length = slen;
				result_count++;
			}
			
			buffer_free (tmp);
		}
	}
	didl_add_footer(out);
	
	HT_DBG_PRINTF(HT_MOD_DMS, HT_BIT_KEY, "length: %d; out->len: %d; \n\r", length, out->len);
	HT_DBG_PRINTF(HT_MOD_DMS, HT_BIT_MYRIAD, "%s \n\r", out->buf);
	
	upnp_add_response (event, SERVICE_CDS_DIDL_RESULT, out->buf);
	buffer_free (out);
	
	upnp_add_response_int(event, SERVICE_CDS_DIDL_NUM_RETURNED, result_count);
	upnp_add_response_int(event, SERVICE_CDS_DIDL_TOTAL_MATCH, ca_count);
	upnp_add_response_int(event, SERVICE_CDS_DIDL_UPDATE_ID, parent->updateID);

	HT_DBG_FUNC_END(result_count, 0);
	return result_count;
}


/*--------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------*/
static int cds_browse_metadata (struct action_event_t *event, C_UPNP_ENTRY *entry, x_UCSI *ucsi)
{
	int update_id = 0;
	
	HT_DBG_FUNC_START(HT_MOD_DMS, HT_BIT_MANY, ucsi->index, NULL);
	if(ucsi->index)
	{
		ucsi->err_code = CDS_ERR_INVALID_ARGS;
		return -1;
	}
	
	struct buffer_t *out = buffer_new ();
	didl_add_header (out);
	if (Dms_VirtualObject_IsItem(entry)) /* item : file */
	{
		s_didl_add_item(out, entry, NULL, "false", ucsi);
		update_id = ut->systemUpdateId;
	}
	else if(Dms_VirtualObject_IsContainer(entry))
	{
		s_didl_add_container(out, entry, "true", "true");
		update_id = entry->updateID;
	}
	else
	{
	}
	didl_add_footer (out);
	
	upnp_add_response (event, SERVICE_CDS_DIDL_RESULT, out->buf);
	buffer_free (out);
	
	upnp_add_response (event, SERVICE_CDS_DIDL_NUM_RETURNED, "1");
	upnp_add_response (event, SERVICE_CDS_DIDL_TOTAL_MATCH, "1");
	upnp_add_response_int (event, SERVICE_CDS_DIDL_UPDATE_ID, update_id);

	HT_DBG_FUNC_END(1, 0);
	return 1;
}
#if 0
#else
static int cds_browse_directchildren (struct action_event_t *event, C_UPNP_ENTRY *entry, x_UCSI *ucsi)
{
	C_UPNP_ENTRY *child;

	HT_DBG_FUNC_START(HT_MOD_DMS, HT_BIT_MANY, Dms_VirtualObject_IsContainer(entry), NULL);
	
	if (!Dms_VirtualObject_IsContainer(entry)) /* item : file */
	{
		ucsi->err_code = CDS_ERR_INVALID_ARGS;
		return -1;
	}

	if(ucsi->index < 0 || ucsi->count < 0)
	{
		ucsi->err_code = CDS_ERR_INVALID_ARGS;
		return -2;
	}

	COO_ARRAY *ca = coo_array_create(20, 30);
	
	int num, i, count;
	C_DMS_CCI *container = entry->object;
	for(num = 0; num < x_DMS_CHILDS_LISTS_NUM; num++)
	{
		COO_ARRAY *ct = container->childs_lists[num];
		if(!ct)
			continue;
		
		count = coo_array_real_count(ct);
		for( i = 0; i < count; i++ )
		{
			child = coo_array_get(ct, i);
			coo_array_append(ca, child);
		}
	}	
	
	s_cds_make_result(event, entry, ucsi, ca);

	coo_array_free(ca);

	HT_DBG_FUNC_END(0, "result_count = ");
	return 0;
}

#endif
static bool cds_browse (struct action_event_t *event)
{
	C_UPNP_ENTRY *entry = NULL;
	char *object_id = NULL, *flag = NULL;
	bool metadata, ret = false;
	int err_code;
	x_UCSI	ucsi;
	memset(&ucsi, 0, sizeof(x_UCSI));
	
	HT_DBG_FUNC_START(HT_MOD_DMS, HT_BIT_KEY, event->request->xmlMaxLen, 0);
	Dms_WaitLock();

/* check if metadatas have been well inited */
	err_code = CDS_ERR_CANNOT_PROCESS_REQUEST;
	if (!ut->init)
		goto s_EXIT;

/* Retrieve Browse arguments */
	ucsi.index				= upnp_get_ui4 (event->request, 	SERVICE_CDS_ARG_START_INDEX);
	ucsi.count				= upnp_get_ui4 (event->request, 	SERVICE_CDS_ARG_REQUEST_COUNT);
	ucsi.sort_criteria		= upnp_get_string (event->request, 	SERVICE_CDS_ARG_SORT_CRIT);
	ucsi.filter_criteria	= upnp_get_string (event->request, 	SERVICE_CDS_ARG_FILTER);
	object_id				= upnp_get_string (event->request, 	SERVICE_CDS_ARG_OBJECT_ID);
	flag					= upnp_get_string (event->request, 	SERVICE_CDS_ARG_BROWSE_FLAG);
	HT_DBG_PRINTF(HT_MOD_DMS, HT_BIT_KEY, "id: %s; index: %d; count: %d;  flag: %s; filter: %s; sort: %s\n\r", 
		object_id, ucsi.index, ucsi.count, flag, ucsi.filter_criteria, ucsi.sort_criteria );
	
/* get and check objectID */
	err_code = CDS_ERR_NO_SUCH_OBJECT;
	if(!object_id)
		goto s_EXIT;

	int id = object_id_to_idx(object_id);
	entry = Dms_VirtualFileTree_GetObject (id);
	if (!entry && (id < ut->starting_id))
		entry = Dms_VirtualFileTree_GetRootObject();
	if (!entry)
		goto s_EXIT;
	
/* get and check flag */
	err_code = UPNP_SOAP_E_INVALID_ARGS;
	if (!strcmp (flag, SERVICE_CDS_BROWSE_METADATA))
		metadata = true;
	else if (!strcmp (flag, SERVICE_CDS_BROWSE_CHILDREN))
		metadata = false;
	else
		goto s_EXIT;

	s_cds_parse_filter(ucsi.filter_criteria, ucsi.filter_array);
	
/* browse */
	if (metadata)
		cds_browse_metadata(event, entry, &ucsi);
	else
		cds_browse_directchildren(event, entry, &ucsi);
	err_code = ucsi.err_code;
	if(err_code)
		goto s_EXIT;
	ret = true;

s_EXIT:
	if(object_id)	
		free(object_id);
	if(flag)	
		free(flag);
	s_cds_free_ucsi(&ucsi);

	if(err_code)
		event->request->ErrCode = err_code;
	
	Dms_PostLock();
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}


/*--------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------*/
static void s_RegExp_Release(X_REG_EXP *me)
{
	if(me->property)
		free(me->property);
	if(me->op)
		free(me->op);
	if(me->value)
		free(me->value);
	
	if(me->pAnd)
		s_RegExp_Release(me->pAnd);

	ListNode *node = ListHead(&(me->set));
	while(node)
	{
		s_RegExp_Release((X_REG_EXP *)(node->item));
		node = ListNext(&(me->set), node);
	}
	ListDestroy(&(me->set), 0);
	
	free(me);
}

static X_REG_EXP* s_RegExp_New(void)
{
	int len = sizeof(X_REG_EXP);
	X_REG_EXP *ret = malloc(len);

	memset(ret, 0, len);
	ListInit(&(ret->set), 0, free);
	return ret;
}

static char* s_parse_strtrim_head(char *str)
{
	if(!str)
		return str;

	while(CDS_CHAR_SPACE == *str)
		str++;
	
	return str;
}

static x_PARSE s_parse_find_braket(char **criteria, int *braket)
{
	x_PARSE ret = x_PARSE_ERROR;
	char *s = *criteria;
	int num_bra, num_ket;

	HT_DBG_FUNC_START(HT_MOD_DMS, HT_BIT_MYRIAD, 0, *criteria);
	num_bra = num_ket = 0;
	while(*s)
	{
		if(CDS_CHAR_BRA == *s)
		{
			num_bra++;
		}
		else if(CDS_CHAR_KET == *s)
		{
			num_ket++;
			if(num_bra == num_ket)
			{
				*criteria = s + 1;
				*braket = num_bra;
				ret = x_PARSE_OK;
				break;
			}
		}
		else
		{
		}
		s++;
	}

	HT_DBG_FUNC_END(ret, 0);
	return ret;
}

static x_PARSE s_parse_one_exp (X_REG_EXP *me, char **criteria)
{
	x_PARSE result = x_PARSE_UNSUPPORTED;	
	char *s, *p;
	int len;
	const X_SECH_KWD *list_pro = s_property_keyword;
	const X_OP_KWD *list_op = s_op_keyword;

	HT_DBG_FUNC_START(HT_MOD_DMS, HT_BIT_MYRIAD, 0, *criteria);
	s = *criteria;
	while((int)(list_pro->pro_type) != -1)
	{
		p = list_pro->property;
		len = strlen(p);
		if(!strncasecmp(p, s, len))
		{
			result = x_PARSE_ERROR;

			s += len;
			if(CDS_CHAR_SPACE != *s)
				return result;
			
			s = s_parse_strtrim_head(s);
			
			while((int)(list_op->op_type) != -1)
			{
				p = list_op->op;
				len = strlen(p);
				if(!strncasecmp(p, s, len))
				{
					s += len;
					
					if(CDS_CHAR_SPACE != *s)
						return result;
					
					s = s_parse_strtrim_head(s);

					if(list_op->op_type == x_OP_EXIST)
					{
						if(!strncasecmp(s, "true", 4))
						{
							result = x_PARSE_OK;
							if(me)
								me->value = strdup("true");
							*criteria = s + 4;
						}
						else if(!strncasecmp(s, "false", 5))
						{
							result = x_PARSE_OK;
							if(me)
								me->value = strdup("false");
							*criteria = s + 5;
						}
						else
						{
						}
					}
					else
					{
						if((CDS_CHAR_QUOTE == *s) && (p = strchr(s+1, CDS_CHAR_QUOTE)))
						{
							result = x_PARSE_OK;
							*p = 0;
							if(me)
								me->value = strdup(s+1);
							*p = CDS_CHAR_QUOTE;
							*criteria = p + 1;
						}
					}

					if(result == x_PARSE_OK)
					{
						if(me)
						{
							me->pro_type = list_pro->pro_type;
							me->op_type  = list_op->op_type;
						}
					}

					return result;
				}
				list_op++;
			}

			return result;
		}
		
		list_pro++;
	}	

	HT_DBG_FUNC_END(result, 0);
	return result;
}

static x_PARSE s_parse_find_all_ands(char **criteria, int *is_end, LinkedList *list, int *dot, int *set)
{
	x_PARSE result = x_PARSE_ERROR;	
	char *s, *p, c;
	int braket;
	X_PARSE_EXP *node;
	
	HT_DBG_FUNC_START(HT_MOD_DMS, HT_BIT_MYRIAD, 0, *criteria);
	s = *criteria;
	while(*s)
	{
		s = s_parse_strtrim_head(s);
		p = s;
		if(CDS_CHAR_BRA == *s)
		{
			result = s_parse_find_braket(&s, &braket);
			c = *s;
			*s = 0;
			HT_DBG_FUNC(result, p);
			*s = c;
			if(result != x_PARSE_OK)
				break;
			
			(*set)++;
			node = (X_PARSE_EXP*)malloc(sizeof(X_PARSE_EXP));
			node->node_type = 1;
			node->start = p;
			node->end   = s;
			ListAddTail(list, node);
		}
		else if(CDS_CHAR_QUOTE == *s) /* error */
		{
			result = x_PARSE_ERROR;
			break;
		}
		else
		{
			result = s_parse_one_exp(NULL, &s);
			c = *s;
			*s = 0;
			HT_DBG_FUNC(result, p);
			*s = c;
			if(result != x_PARSE_OK)
				break;
			
			(*dot)++;
			node = (X_PARSE_EXP*)malloc(sizeof(X_PARSE_EXP));
			node->node_type = 0;
			node->start = p;
			node->end   = s;
			ListAddTail(list, node);
		}

		if(0 == *s)/* the end */
		{
			*criteria = s;
			*is_end = 1;
			break;
		}

		if(CDS_CHAR_SPACE != *s) /* at least one space */
		{
			result = x_PARSE_ERROR;
			break;
		}
		
		s = s_parse_strtrim_head(s);
		if(0 == *s)/* the end */
		{
			*criteria = s;
			*is_end = 1;
			break;
		}
		if(!strncasecmp(s, "or ", 3))
		{
			*criteria = s + 3;
			*is_end = 0;
			break;
		}
		else if(!strncasecmp(s, "and ", 4))
		{
			s += 4;
		}
		else
		{ 
			result = x_PARSE_ERROR;
			break;
		}
	}
	
	HT_DBG_FUNC(*dot, "dot num =");
	HT_DBG_FUNC(*set, "set num =");
	HT_DBG_FUNC_END(result, 0);
	return result;
}

static x_PARSE s_parse_criteria_recursive(X_REG_EXP *parent, char *criteria)
{
	x_PARSE result;	/* -1: error; 0: not supported;  1: ok */
	X_REG_EXP *e, *e_start, *e_end;
	ListNode *node;
	X_PARSE_EXP *parse;
	char *s;
	int is_end, dot, set;
	
	HT_DBG_FUNC_START(HT_MOD_DMS, HT_BIT_MYRIAD, 0, criteria);
	
	result = x_PARSE_ERROR;
	s = criteria;
	while(*s)
	{
		LinkedList list;
		ListInit(&list, 0, free);
		
		is_end = dot = set = 0;
		result = s_parse_find_all_ands(&s, &is_end, &list, &dot, &set);
		if(result != x_PARSE_OK)
		{
			ListDestroy(&list, 1);
			goto s_END;
		}

		e_start = NULL;
		e_end = NULL;
		node = ListHead(&list);
		while(node)
		{
			parse = (X_PARSE_EXP*)(node->item);

			e = s_RegExp_New();
			if(parse->node_type) /* set */
			{
				e->node_type = 1;
				*(parse->end-1) = 0;
				result = s_parse_criteria_recursive(e, parse->start + 1);
				if(result != x_PARSE_OK)
				{
					s_RegExp_Release(e);
					ListDestroy(&list, 1);
					goto s_END;
				}
			}
			else /* dot */
			{
				e->node_type = 0;
				*(parse->end) = 0;
				result = s_parse_one_exp(e, &(parse->start));
			}

			if(!e_start)
				e_start = e;
			else
				e_end->pAnd = e;
			e_end = e;
			
			node = ListNext(&list, node);
		}
		ListDestroy(&list, 1);
		if(e_start)
			ListAddTail(&(parent->set), e_start);

		if(is_end)/* the end */
			break;
	}
	
s_END:
	HT_DBG_FUNC_END(result, 0);
	return result;
}

static bool s_Sech_DoesNeed_Int(x_OP op_type)
{
	bool ret = false;

	switch(op_type)
	{
		case x_OP_EQUAL:
		case x_OP_NOT_EQUAL:
			break;
	
		case x_OP_LITTLER:
		case x_OP_LITTLER_EQU:
		case x_OP_BIGGER:
		case x_OP_BIGGER_EQU:
			ret = true;
			break;
		
		case x_OP_CONTAIN:
		case x_OP_NOT_CONTAIN:
		case x_OP_DERIVED_FROM:
			break;
		
		case x_OP_EXIST:
			break;
																																
		default:
			break;
	}
	
	return ret;		
}

static bool s_Sech_IsMatched_Str(x_OP op_type, char*wanted_str, char *val_str)
{
	bool ret = false;

	if(!wanted_str || !val_str)
		return ret;
	
	switch(op_type)
	{
		case x_OP_EQUAL:
			if(!strcasecmp(val_str, wanted_str))
				ret = true;
			break;
			
		case x_OP_NOT_EQUAL:
			if(strcasecmp(val_str, wanted_str))
				ret = true;
			break;
	
		case x_OP_LITTLER:
		case x_OP_LITTLER_EQU:
		case x_OP_BIGGER:
		case x_OP_BIGGER_EQU:
			break;
		
		case x_OP_CONTAIN:
			if(strcasestr(val_str, wanted_str))
				ret = true;
			break;
		
		case x_OP_NOT_CONTAIN:
			if(!strcasestr(val_str, wanted_str))
				ret = true;
			break;
		
		case x_OP_DERIVED_FROM:
			if(strcasestr(val_str, wanted_str))
				ret = true;
			break;
		
		case x_OP_EXIST:
			if(!strcasecmp(val_str, wanted_str))
				ret = true;
			break;
																																
		default:
			break;
	}
	
	return ret;		
}

static bool s_Sech_IsMatched_Int(x_OP op_type, int wanted_int, int val_int)
{
	bool ret = false;
	
	switch(op_type)
	{
		case x_OP_EQUAL:
			if(val_int == wanted_int)
				ret = true;
			break;
			
		case x_OP_NOT_EQUAL:
			if(val_int != wanted_int)
				ret = true;
			break;
	
		case x_OP_LITTLER:
			if(val_int < wanted_int)
				ret = true;
			break;
	
		case x_OP_LITTLER_EQU:
			if(val_int <= wanted_int)
				ret = true;
			break;
	
		case x_OP_BIGGER:
			if(val_int > wanted_int)
				ret = true;
			break;
			
		case x_OP_BIGGER_EQU:
			if(val_int >= wanted_int)
				ret = true;
			break;
		
		case x_OP_CONTAIN:
		case x_OP_NOT_CONTAIN:
		case x_OP_DERIVED_FROM:
			break;
		
		case x_OP_EXIST:
			break;
																																
		default:
			break;
	}
	
	return ret;		
}

static bool s_Sech_IsMatched_Int64(x_OP op_type, off_t wanted_int, off_t val_int)
{
	bool ret = false;
	
	switch(op_type)
	{
		case x_OP_EQUAL:
			if(val_int == wanted_int)
				ret = true;
			break;
			
		case x_OP_NOT_EQUAL:
			if(val_int != wanted_int)
				ret = true;
			break;
	
		case x_OP_LITTLER:
			if(val_int < wanted_int)
				ret = true;
			break;
	
		case x_OP_LITTLER_EQU:
			if(val_int <= wanted_int)
				ret = true;
			break;
	
		case x_OP_BIGGER:
			if(val_int > wanted_int)
				ret = true;
			break;
			
		case x_OP_BIGGER_EQU:
			if(val_int >= wanted_int)
				ret = true;
			break;
		
		case x_OP_CONTAIN:
		case x_OP_NOT_CONTAIN:
		case x_OP_DERIVED_FROM:
			break;
		
		case x_OP_EXIST:
			break;
																																
		default:
			break;
	}
	
	return ret;		
}


static bool s_Sech_IsMatched_Item(X_REG_EXP *me, C_UPNP_ENTRY *p_entry)
{
	bool ret = false;
	char *val_str;
	int val_int;
	off_t val_long;
	C_DMS_CMI *entry = p_entry->object;
	
	switch(me->pro_type)
	{
		case x_PROPERTY_UPNPCLASS:
			val_str = (char*)dlna_get_upnp_class(entry->protocol_info->class);
			ret = s_Sech_IsMatched_Str(me->op_type, me->value, val_str);
			break;
			
		case x_PROPERTY_REFID:
			if(me->op_type == x_OP_EXIST)
				ret = s_Sech_IsMatched_Str(me->op_type, me->value, "false");
			else if(s_Sech_DoesNeed_Int(me->op_type))
				ret = s_Sech_IsMatched_Int(me->op_type, atoi(me->value), 0);
			else
				ret = s_Sech_IsMatched_Str(me->op_type, me->value, "");
			break;

		case x_PROPERTY_TITLE:
			val_str = entry->title;
			ret = s_Sech_IsMatched_Str(me->op_type, me->value, val_str);
			break;

		case x_PROPERTY_RES_PROTOCOL:
			break;

		case x_PROPERTY_RES_SIZE:
			val_long = entry->size;
			ret = s_Sech_IsMatched_Int64(me->op_type, atoll(me->value), val_long);
			break;
							
		case x_PROPERTY_RES_BITRATE:
			val_int = entry->bitrate;
			ret = s_Sech_IsMatched_Int(me->op_type, atoi(me->value), val_int);
			break;
							
		case x_PROPERTY_RES_DURATION:
			break;

		case x_PROPERTY_RES_RESOLUTION:
			break;
												
		default:
			break;
	}

	return ret;
}

static bool s_Sech_IsMatched_Container(X_REG_EXP *me, C_UPNP_ENTRY *p_entry)
{
	bool ret = false;
	char *val_str;
//	int val_int;
//	off_t val_long;
	C_DMS_CCI *container = p_entry->object;
	
	switch(me->pro_type)
	{
		case x_PROPERTY_UPNPCLASS:
			val_str = (char*)(container->mime_type->upnp_class);
			ret = s_Sech_IsMatched_Str(me->op_type, me->value, val_str);
			break;
			
		case x_PROPERTY_REFID:
			if(me->op_type == x_OP_EXIST)
				ret = s_Sech_IsMatched_Str(me->op_type, me->value, "false");
			else if(s_Sech_DoesNeed_Int(me->op_type))
				ret = s_Sech_IsMatched_Int(me->op_type, atoi(me->value), 0);
			else
				ret = s_Sech_IsMatched_Str(me->op_type, me->value, "");
			break;

		case x_PROPERTY_TITLE:
			val_str = container->title;
			ret = s_Sech_IsMatched_Str(me->op_type, me->value, val_str);
			break;
												
		default:
			break;
	}

	return ret;
}

static bool s_do_match_recursive(X_REG_EXP *me, C_UPNP_ENTRY *entry)
{
	bool ret = false;

	if(me->node_type == 0) /* dot */
	{
		if( (Dms_VirtualObject_IsItem(entry) && s_Sech_IsMatched_Item(me, entry)) ||
			(Dms_VirtualObject_IsContainer(entry) && s_Sech_IsMatched_Container(me, entry)) )
			ret = (!me->pAnd)? true : s_do_match_recursive(me->pAnd, entry);
	}
	else /*set */
	{
		ListNode *node = ListHead(&(me->set));
		while(node)
		{
			ret = s_do_match_recursive((X_REG_EXP *)(node->item), entry);
			if(ret)
				break;
			node = ListNext(&(me->set), node);
		}

		if(ret && me->pAnd)		
			ret = s_do_match_recursive(me->pAnd, entry);
	}
	
	return ret;
}

static void s_cds_search_all_recursive_ex2 (x_UCSI *ucsi, C_UPNP_ENTRY *entry, COO_ARRAY *result)
{
	int num, i, count;
	C_DMS_CCI *container = entry->object;
	for(num = 0; num < x_DMS_CHILDS_LISTS_NUM; num++)
	{
		COO_ARRAY *ct = container->childs_lists[num];
		if(!ct)
			continue;
		
		count = coo_array_real_count(ct);
		for( i = 0; i < count; i++ )
		{
			X_REG_EXP *e = (X_REG_EXP *)(ucsi->search_tree);
			C_UPNP_ENTRY *son = coo_array_get(ct, i);
			
			if(!e || s_do_match_recursive(e, son))
				coo_array_append(result, son);
			
			if(Dms_VirtualObject_IsContainer(son)) /* container */
				s_cds_search_all_recursive_ex2(ucsi, son, result);
		}		
	}	
}
#if 0
static bool test_ai_search_output(X_REG_EXP *me)
{
	bool ret = false;

	HT_DBG_FUNC_START(HT_MOD_DMS, HT_BIT_KEY, me->node_type, "me->node_type = ");
	
	if(me->node_type == 0) /* dot */
	{
		HT_DBG_FUNC(me->pro_type, "me->pro_type =");
		HT_DBG_FUNC(100, me->property);
		HT_DBG_FUNC(me->op_type, "me->op_type =");
		HT_DBG_FUNC(200, me->op);
		HT_DBG_FUNC(300, me->value);
		HT_DBG_FUNC((int)(me->pAnd), "me->pAnd =");

		if(me->pAnd)
			test_ai_search_output(me->pAnd);
	}
	else /*set */
	{
		HT_DBG_FUNC(ListSize(&(me->set)), "show me->set = ");
		ListNode *node = ListHead(&(me->set));
		while(node)
		{
			test_ai_search_output((X_REG_EXP *)(node->item));
			node = ListNext(&(me->set), node);
		}
		
		HT_DBG_FUNC((int)(me->pAnd), "show me->pAnd =");
		if(me->pAnd)
			test_ai_search_output(me->pAnd);
	}
	
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}

void test_ai_search(void)
{
	int ret = CDS_ERR_INVALID_SEARCH_CRITERIA;
	char *p, **s, *criteria[] = {
#if 0		
		"xxx",
		"xxx  yyy cccc 9999((( )))))",
		"s1 and s2 or s3 or s4 and s5",
		"((s1 and s2) or s3) or (s4 and s5)",
		"s1 and s2 or (s3 or s4) and s5",
		"(s1 and s2) or ((s3 or s4) and s5)",
#else
		"upnp:class derivedfrom \"object.item\"",
		
		"dc:title contains \"tenderness\"",
		"dc:title     =  \"tenderness\"",
		"res@size     != \"tenderness\"",
		"res@duration >= \"tenderness\"",
		"upnp:class derivedfrom \"object.item.imageItem.photo\" and (dc:title >= \"2001-10-01\" and dc:title <= \"2001-10-31\"   ) or res@size > \"32\" and res@duration < \"7\" or ((res@size  > \"32\" and   res@duration < \"7\"))   ", 	
		
		"dc:creator = \"Sting\"",
	"upnp:class derivedfrom \"object.item.imageItem.photo\" and (dc:title >= \"2001-10-01\" and dc:title <= \"2001-10-31\") or res@size > 32 and res@duration < 7 or ((res@size  > 32 and	res@duration < 7))	 ", 	
		"upnp:class derivedfrom \"object.item.imageItem.photo\" and (dc:date >= \"2001-10-01\" and dc:date <= \"2001-10-31\")",		
		"dc:creator = \"Sti\\\"ng\"",
#endif		
		NULL
	};

	ret = 10;
	while(ret--)
		printf("  \n\r");
	HT_DBG_FUNC_START(HT_MOD_DMS, HT_BIT_KEY, 0, 0);

	ret = CDS_ERR_INVALID_SEARCH_CRITERIA;
	s = criteria;
	while(*s)
	{
		p = strdup(*s);
		printf("------------- \n\r");
		printf("  \n\r");
		X_REG_EXP *root = s_RegExp_New();
		root->node_type = 1;
		x_PARSE result = s_parse_criteria_recursive(root, p);
		if(result == x_PARSE_OK)
		{
			printf("  \n\r");
			test_ai_search_output(root);
		}
		s_RegExp_Release(root);
		printf("  \n\r");
		s++;
		free(p);
	}
	
	HT_DBG_FUNC_END(ret, 0);
}
#endif

static int s_cds_search_in_cache (C_UPNP_ENTRY *entry, x_UCSI *ucsi)
{
	entry = NULL;
	ucsi = NULL;
	return -1;
}
static void s_cds_search_result_to_cache (C_UPNP_ENTRY *entry, x_UCSI *ucsi, COO_ARRAY *ca)
{
	entry = NULL;
	ucsi = NULL;
	coo_array_free(ca);
}

static bool cds_search (struct action_event_t *event)
{
	C_UPNP_ENTRY *entry;
	char *container_id = NULL;
	bool ret = false;
	int err_code;
	x_UCSI	ucsi;
	memset( &ucsi, 0, sizeof(x_UCSI));
	
	HT_DBG_FUNC_START(HT_MOD_DMS, HT_BIT_KEY, event->request->xmlMaxLen, 0);
	Dms_WaitLock();

/* check if metadatas have been well inited */
	err_code = CDS_ERR_CANNOT_PROCESS_REQUEST;
	if (!ut->init)
		goto s_EXIT;

/* Retrieve Browse arguments */
	ucsi.index				= upnp_get_ui4 (event->request, 	SERVICE_CDS_ARG_START_INDEX);
	ucsi.count				= upnp_get_ui4 (event->request, 	SERVICE_CDS_ARG_REQUEST_COUNT);
	ucsi.search_criteria	= upnp_get_string (event->request, 	SERVICE_CDS_ARG_SEARCH_CRIT);
	ucsi.sort_criteria		= upnp_get_string (event->request,	SERVICE_CDS_ARG_SORT_CRIT);
	ucsi.filter_criteria	= upnp_get_string (event->request,	SERVICE_CDS_ARG_FILTER);
	container_id			= upnp_get_string (event->request,	"ContainerID");

	HT_DBG_PRINTF(HT_MOD_DMS, HT_BIT_KEY, "id: %s; index: %d; count: %d;  search: %s; filter: %s; sort: %s\n\r", 
		container_id, ucsi.index, ucsi.count, ucsi.search_criteria, ucsi.filter_criteria, ucsi.sort_criteria );
	
/* get and check objectID */
	err_code = CDS_ERR_NO_SUCH_CONTAINER;
	if(!container_id)
		goto s_EXIT;

	int id = object_id_to_idx(container_id);
	entry = Dms_VirtualFileTree_GetObject(id);
	if (!entry && (id < ut->starting_id))
		entry = Dms_VirtualFileTree_GetRootObject();
	if (!entry)
		goto s_EXIT;

	err_code = UPNP_SOAP_E_INVALID_ARGS;
	if(!Dms_VirtualObject_IsContainer(entry)) /* not container */
		goto s_EXIT;
	if(ucsi.index < 0 || ucsi.count < 0)
		goto s_EXIT;
	
/* parse filter */
	s_cds_parse_filter(ucsi.filter_criteria, ucsi.filter_array);

/* try find in cache */
	err_code = 0;
	ret = true;
	if( s_cds_search_in_cache(entry, &ucsi) >= 0 )
		goto s_EXIT;
	
/* have to search */
	COO_ARRAY *ca = coo_array_create(20, 30);

	if(!strcmp(ucsi.search_criteria, "*"))
		s_cds_search_all_recursive_ex2(&ucsi, entry, ca);
	else
	{
		X_REG_EXP *root = s_RegExp_New();
		root->node_type = 1;
		x_PARSE result = s_parse_criteria_recursive(root, ucsi.search_criteria);
		if(result == x_PARSE_OK)
		{
			ucsi.search_tree = root;
			s_cds_search_all_recursive_ex2(&ucsi, entry, ca);
		}
		else
			err_code = CDS_ERR_INVALID_SEARCH_CRITERIA;
		s_RegExp_Release(root);
	}
	
	if(err_code)
	{
		ret = false;
		coo_array_free(ca);
	}
	else
	{
		s_cds_make_result(event, entry, &ucsi, ca);
		s_cds_search_result_to_cache(entry, &ucsi, ca);
	}


s_EXIT:
	if(container_id)					
		free(container_id);
	s_cds_free_ucsi(&ucsi);

	if(err_code)
		event->request->ErrCode = err_code;
	
	Dms_PostLock();
	HT_DBG_FUNC_END(err_code, 0);
	return ret;
}


/*--------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------*/
static bool cds_get_search_capabilities (struct action_event_t *event)
{
	char buf[1024] = {0};
	const X_SECH_KWD *list_pro = s_property_keyword;

	HT_DBG_FUNC_START(HT_MOD_DMS, HT_BIT_KEY, (int)list_pro, 0);
	while(list_pro->property)
	{
		strcat(buf, list_pro->property);
		strcat(buf, ",");
		list_pro++;
	}
	buf[strlen(buf)-1] = 0;
	
	upnp_add_response (event, SERVICE_CDS_ARG_SEARCH_CAPS, buf);
	HT_DBG_FUNC_END(event->status, buf);
	return event->status;
}

static bool cds_get_sort_capabilities (struct action_event_t *event)
{
  upnp_add_response (event, SERVICE_CDS_ARG_SORT_CAPS, "");
  return event->status;
}

static bool cds_get_system_update_id (struct action_event_t *event)
{
	upnp_add_response_int (event, SERVICE_CDS_ARG_UPDATE_ID, ut->systemUpdateId);
	return event->status;
}

/* List of UPnP ContentDirectory Service actions */
struct service_action_t cds_service_actions[] = {
  { SERVICE_CDS_ACTION_SEARCH_CAPS,	cds_get_search_capabilities },
  { SERVICE_CDS_ACTION_SORT_CAPS,	cds_get_sort_capabilities },
  { SERVICE_CDS_ACTION_UPDATE_ID,	cds_get_system_update_id },
  { SERVICE_CDS_ACTION_BROWSE,		cds_browse },
  { SERVICE_CDS_ACTION_SEARCH,		cds_search },
  { NULL, NULL }
};



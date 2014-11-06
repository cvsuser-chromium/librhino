/*
 * upnp_entry.c 
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
	 
#include "dlna.h"
#include "mime.h"
#include "hitTime.h"
#include "dms.h"


/*--------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------*/
#define RETURN_IF_NULL(p)	if(!p) return
#define FREE_POINTER(p)		if(p) free(p)

static void s_free_pointer(void *p)
{
	RETURN_IF_NULL(p);
	
	free(p);
}

static void s_upnp_entry_free_dlna_profile(x_DMS_IPI *p)
{
	RETURN_IF_NULL(p);
	
	s_free_pointer(p->id);
	s_free_pointer(p->mime);
	s_free_pointer(p->label);
	
	free(p);
}

static void s_upnp_entry_free_thumbnail(C_DMS_THUI *p)
{
	RETURN_IF_NULL(p);
	
	if(p->malloced)
		s_upnp_entry_free_dlna_profile((x_DMS_IPI*)(p->protocol_info));
	
	free(p);
}

static void s_upnp_entry_free_audio_info(WAVEFORMATEX *p)
{
	RETURN_IF_NULL(p);

	free(p);
}

static void s_upnp_entry_free_upnp_info(C_DMS_UPNI *p)
{
	RETURN_IF_NULL(p);

	s_free_pointer(p->creator);
	s_free_pointer(p->artist);
	s_free_pointer(p->actor);
	s_free_pointer(p->producer);
	s_free_pointer(p->genre);
	s_free_pointer(p->description);
	
	free(p);
}

static void s_upnp_entry_free_pvr(C_DMS_PVRI *p)
{
	RETURN_IF_NULL(p);

	s_free_pointer(p->recordedStartDateTime);
	s_free_pointer(p->recordedEndDateTime);
	
	free(p);
}

static void s_upnp_entry_free_tuner(C_DMS_TUNI *p)
{
	RETURN_IF_NULL(p);

	s_free_pointer(p->frequency);
	s_free_pointer(p->channel);
	
	free(p);
}

static void s_upnp_entry_free_optional(C_DMS_OPTI *p)
{
	RETURN_IF_NULL(p);
	
	s_upnp_entry_free_audio_info(	p->audio_info);
	s_upnp_entry_free_upnp_info(	p->upnp_info);
	s_upnp_entry_free_pvr(			p->pvr);
	s_upnp_entry_free_tuner(		p->tuner);

	free(p);
}

void Dms_VirtualObject_FreeItem(void *object)
{
	C_DMS_CMI *p = (C_DMS_CMI *)object;
	RETURN_IF_NULL(p);

	if(p->malloced)
		s_upnp_entry_free_dlna_profile((x_DMS_IPI*)(p->protocol_info));
	s_free_pointer(						p->title);
	s_free_pointer(						p->keyword);
	s_upnp_entry_free_thumbnail(		p->thumbnail);
	s_upnp_entry_free_optional(			p->optional);
	if(p->extension)
		json_object_put(				p->extension);
	s_free_pointer(						p->user_info);
	
	free(p);
}

static void s_upnp_entry_free_mime_type(x_DMS_CI *p)
{
	RETURN_IF_NULL(p);
	
	s_free_pointer(p->upnp_class);
	
	free(p);
}

void Dms_VirtualObject_FreeContainerSelf(void *object)
{
	int num;
	C_DMS_CCI *p = (C_DMS_CCI *)object;
	RETURN_IF_NULL(p);

	s_free_pointer(p->title);
	s_free_pointer(p->keyword);

	if(p->malloced)
		s_upnp_entry_free_mime_type((x_DMS_CI*)(p->mime_type));
	for(num = 0; num < x_DMS_CHILDS_LISTS_NUM; num++)
	{
		COO_ARRAY *ca = p->childs_lists[num];
		if(ca)
			coo_array_free(ca);
	}

	free(p);
}

void Dms_VirtualObject_Free(C_DMS_VO *entry)
{
	if(!entry)
		return;

	HT_DBG_FUNC_START(HT_MOD_DMS, HT_BIT_MANY, (int)entry, 0);
	HT_DBG_FUNC((int)(entry->is_item), 		"is_item = ");
	HT_DBG_FUNC((int)(entry->object),  		"object  = ");
	HT_DBG_FUNC((int)(entry->FreeObject), 	"FreeObj = ");

	if(Dms_VirtualObject_IsContainer(entry))
		Dms_VirtualObject_FreeContainerSelf(entry->object);
	else if(Dms_VirtualObject_IsItem(entry))
		Dms_VirtualObject_FreeItem(entry->object);
	else
	{
	}

	free(entry);
	HT_DBG_FUNC_END(0, 0);
}


/*--------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------*/
static void s_upnp_entry_create_thumbnail(C_DMS_THUI **pp)
{
	C_DMS_THUI *p = COO_OBJECT_NEW(C_DMS_THUI);
	RETURN_IF_NULL(p);
	*pp = p;
}

static void s_upnp_entry_create_audio_info(WAVEFORMATEX **pp)
{
	WAVEFORMATEX *p = COO_OBJECT_NEW(WAVEFORMATEX);
	RETURN_IF_NULL(p);
	*pp = p;
}

static void s_upnp_entry_create_upnp_info(C_DMS_UPNI **pp)
{
	C_DMS_UPNI *p = COO_OBJECT_NEW(C_DMS_UPNI);
	RETURN_IF_NULL(p);
	*pp = p;
}

static void s_upnp_entry_create_pvr(C_DMS_PVRI **pp)
{
	C_DMS_PVRI *p = COO_OBJECT_NEW(C_DMS_PVRI);
	RETURN_IF_NULL(p);
	*pp = p;
}

static void s_upnp_entry_create_tuner(C_DMS_TUNI **pp)
{
	C_DMS_TUNI *p = COO_OBJECT_NEW(C_DMS_TUNI);
	RETURN_IF_NULL(p);
	*pp = p;
}

static void s_upnp_entry_create_optional(C_DMS_OPTI **pp)
{
	C_DMS_OPTI *p = COO_OBJECT_NEW(C_DMS_OPTI);
	RETURN_IF_NULL(p);
	*pp = p;
	
	s_upnp_entry_create_audio_info(	&(p->audio_info));
	s_upnp_entry_create_upnp_info(	&(p->upnp_info));
	s_upnp_entry_create_pvr(		&(p->pvr));
	s_upnp_entry_create_tuner(		&(p->tuner));
}

void Dms_VirtualObject_CreateItem(C_DMS_CMI **pp, const C_DMS_IPI *protocol, const C_DMS_IPI *thumbnail_protocol)
{
	C_DMS_CMI *p = COO_OBJECT_NEW(C_DMS_CMI);
	RETURN_IF_NULL(p);
	*pp = p;
	
	p->protocol_info	= protocol;

	s_upnp_entry_create_optional(&(p->optional));
	if(thumbnail_protocol)
	{
		s_upnp_entry_create_thumbnail(&(p->thumbnail));
		p->thumbnail->protocol_info = thumbnail_protocol;
	}
}

C_DMS_CCI *Dms_VirtualObject_CreateContainerSelf(char *keyword, char *title, const C_DMS_CI *mime_type)
{
	if(!keyword || !mime_type)
		return NULL;
	
	C_DMS_CCI *p = COO_OBJECT_NEW(C_DMS_CCI);
	if(p)
	{
		p->mime_type 	= mime_type;
		p->keyword		= strdup(keyword);
		if(title)
			p->title	= strdup(title);
	}
	return p;
}

C_DMS_VO *Dms_VirtualObject_Create(int parent_id, void *me, int object_type, int source_type, t_DMS_MAKE_OBJECTID make_func, t_DMS_FREE_OBJECT free_func)
{
	if(!me)
		return NULL;

	C_DMS_VO *entry = COO_OBJECT_NEW(C_DMS_VO);
	if(entry)
	{
		entry->is_item		= object_type;
		entry->source_type	= (char)source_type;
//		entry->parent		= parent_id;
		parent_id = 0;
		entry->object		= me;

		entry->MakeObjectId = make_func;
		entry->FreeObject	= free_func;
	}
	
	return entry;
}

/*--------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------*/
void Dms_VirtualObject_Trim(C_DMS_VO *entry)
{
	if(Dms_VirtualObject_IsItem(entry))
	{
	}
	else if(Dms_VirtualObject_IsContainer(entry))
	{
	}
	else
	{
	}
}

/*--------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------*/

int Dms_VirtualObject_IsItem(C_DMS_VO *entry)
{
	return (entry->is_item == e_OBJTYPE_ITEM);
}

int Dms_VirtualObject_IsContainer(C_DMS_VO *entry)
{
	return (entry->is_item == e_OBJTYPE_CONTAINER);
}

static int s_Dms_VirtualObject_IsValid (C_DMS_VO *child)
{
	if(!child)
		return e_IDE_ENTRY_NULL;
	
	if(Dms_VirtualObject_IsContainer(child))
	{
		C_DMS_CCI *p = child->object;
		if(!p)
			return e_IDE_NO_OBJCT;
		if(!(p->keyword))
			return e_IDE_NO_KEYWORD;
		if(!(p->mime_type))
			return e_IDE_NO_MIME_TYPE;
	}
	else if(Dms_VirtualObject_IsItem(child))
	{
		C_DMS_CMI *p = child->object;
		if(!p)
			return e_IDE_NO_OBJCT;
		if(!(p->keyword))
			return e_IDE_NO_KEYWORD;
		if(!(p->protocol_info))
			return e_IDE_NO_PROTOCOL_INFO;
	}
	else
		return e_IDE_ENTRY_UNSUPPORTED;

	return 0;
}

int Dms_VirtualObject_IsValid(C_DMS_VO *entry)
{
	HT_DBG_FUNC_START(HT_MOD_DMS, HT_BIT_MYRIAD, (int)entry, 0);
	int ret = s_Dms_VirtualObject_IsValid(entry);
	HT_DBG_FUNC_END(ret, NULL);
	return ret;
}


#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <fcntl.h>

#include "upnp.h"
#include "upnptools.h"

#include "ushare.h"
#include "cfgparser.h"
#include "services.h"
#include "trace.h"
#include "http.h"

struct ushare_t *ut = NULL;
static sem_t upnp_lock;
static int	upnp_run = 0;
extern void dms_filetree_build(struct ushare_t *p);
extern void dms_filetree_free(struct ushare_t *p);
extern void Dms_ShareUsb_Init (void);

static struct ushare_t *ushare_new (void)
{
	struct ushare_t *ut = COO_OBJECT_NEW(struct ushare_t);

	ut->dlna_flags = DLNA_ORG_FLAG_STREAMING_TRANSFER_MODE |
	               DLNA_ORG_FLAG_BACKGROUND_TRANSFERT_MODE |
	               DLNA_ORG_FLAG_CONNECTION_STALL |
	               DLNA_ORG_FLAG_DLNA_V15;

//	ut->friendlyName		= strdup ("yuxing dms);
	ut->manufacturer		= strdup ("Yuxing Software Co., Ltd");
	ut->manufacturerURL		= strdup ("http://www.yuxing.com.cn/");
	ut->modelDescription	= strdup ("DLNA1.5 MediaServer");
	ut->modelName			= strdup ("Yuxing DLNA1.5 DMS");
	ut->modelNumber			= strdup ("1");
	ut->modelURL			= strdup ("http://www.yuxing.com.cn/");
	ut->serialNumber		= strdup ("001");

	ut->object_total_limit	= 5000;
	
	ut->interface 			= NULL;//strdup (DEFAULT_USHARE_IFACE);
	ut->starting_id			= STARTING_ENTRY_ID_DEFAULT;
	ut->dlna_enabled		= true;
	ut->dlna				= NULL;
	return ut;
}

static void ushare_free (struct ushare_t *p)
{
	if (!p)
		return;
	
	FREE_POINTER( p->interface );
	FREE_POINTER( p->udn );
	FREE_POINTER( p->ip );

//	if (ut->presentation)
//		buffer_free (ut->presentation);

	if (ut->dlna_enabled)
	{
		if (ut->dlna)
			dlna_uninit (ut->dlna);
		ut->dlna = NULL;
	}

	dms_filetree_free(ut);

	free (ut);
}

static void s_Dms_HandleActionRequest (struct Upnp_Action_Request *request)
{
	struct service_t *service;
	struct service_action_t *action;
	char val[256];
	uint32_t ip;

	HT_DBG_FUNC_START(HT_MOD_DMS, HT_BIT_KEY, (int)request, "request =");
	if (!request || !ut)
		goto s_EXIT;

	if (request->ErrCode != UPNP_E_SUCCESS)
	{
		HT_DBG_FUNC(request->ErrCode, "request->ErrCode =");
		goto s_EXIT;
	}

	if (strcmp (request->DevUDN + 5, ut->udn))
	{
		HT_DBG_FUNC(-10, request->DevUDN);
		goto s_EXIT;
	}

	//ip = request->CtrlPtIPAddr.s_addr;
	ip =((struct sockaddr_in*)(&request->CtrlPtIPAddr))->sin_addr.s_addr;
	ip = ntohl (ip);
	sprintf (val, "%d.%d.%d.%d", (ip >> 24) & 0xFF, (ip >> 16) & 0xFF, (ip >> 8) & 0xFF, ip & 0xFF);
	HT_DBG_FUNC(0, request->ActionName);

	if (find_service_action (request, &service, &action))
	{
		struct action_event_t event;

		event.request = request;
		event.status = true;
		event.service = service;

		if (action->function (&event) && event.status)
			request->ErrCode = UPNP_E_SUCCESS;
	}
	else
	{
		if (service) /* Invalid Action name */
			strcpy (request->ErrStr, "Unknown Service Action");
		else /* Invalid Service name */
			strcpy (request->ErrStr, "Unknown Service ID");

		request->ActionResult = NULL;
		request->ErrCode = UPNP_SOAP_E_INVALID_ACTION;
	}

s_EXIT: 
	HT_DBG_FUNC_END(request->ErrCode, "request->ErrCode = ");
}

static int dms_callback_event_handler (Upnp_EventType type, void *event, void *cookie __attribute__((unused)))
{
	HT_DBG_FUNC_START(HT_MOD_DMS, HT_BIT_MANY, type, 0);
	switch (type)
	{
		case UPNP_CONTROL_ACTION_REQUEST:
			s_Dms_HandleActionRequest ((struct Upnp_Action_Request *) event);
			break;
			
		case UPNP_CONTROL_ACTION_COMPLETE:
		case UPNP_EVENT_SUBSCRIPTION_REQUEST:
		case UPNP_CONTROL_GET_VAR_REQUEST:
			break;
			
		default:
			break;
	}

	HT_DBG_FUNC_END((int)event, 0);
	return 0;
}

#define DEFAULT_UUID "898f9738-d930-4db4-a3cf"
static char *create_udn (char *interface)
{
	unsigned char *mac;
	char *udn, *p = NULL;
	int ret = coo_util_get_mac(interface, &p);
	if(ret)
		return NULL;

	mac = (unsigned char *)p;
	udn = coo_malloc_bzero(128);
	snprintf(udn, 64, "%s-%02x%02x%02x%02x%02x%02x", DEFAULT_UUID, mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
	return udn;
}


static void s_init_dlna(struct ushare_t *ut)
{
    ut->dlna = dlna_init ();
    dlna_set_verbosity (ut->dlna, ut->verbose ? 1 : 0);
    dlna_set_extension_check (ut->dlna, 0);
    dlna_register_all_media_profiles (ut->dlna);
}



#define x_DMS_DESC_HEAD \
"<?xml version=\"1.0\" encoding=\"utf-8\"?>" \
"<root xmlns=\"urn:schemas-upnp-org:device-1-0\">" \
"  <specVersion>" \
"    <major>1</major>" \
"    <minor>0</minor>" \
"  </specVersion>" \
"  <device>" \
"    <deviceType>urn:schemas-upnp-org:device:MediaServer:1</deviceType>" 

#define x_DMS_DESC_DEVICE \
"    <friendlyName>%s</friendlyName>" \
"    <manufacturer>%s</manufacturer>" \
"    <manufacturerURL>%s</manufacturerURL>" \
"    <modelDescription>%s</modelDescription>" \
"    <modelName>%s</modelName>" \
"    <modelNumber>%s</modelNumber>" \
"    <modelURL>%s</modelURL>" \
"    <serialNumber>%s</serialNumber>" \
"    <UDN>uuid:%s</UDN>" \
"    <presentationURL>%s</presentationURL>"

#define x_DMS_DESC_ICON \
"	 <iconList>" \
"	   <icon>" \
"		 <mimetype>%s</mimetype>" \
"		 <width>%d</width>" \
"		 <height>%d</height>" \
"		 <depth>%d</depth>" \
"		 <url>%s</url>" \
"	   </icon>" \
"	 </iconList>" 

#define x_DMS_DESC_SERVICE \
"    <serviceList>" \
"      <service>" \
"        <serviceType>urn:schemas-upnp-org:service:ContentDirectory:1</serviceType>" \
"        <serviceId>urn:upnp-org:serviceId:ContentDirectory</serviceId>" \
"        <SCPDURL>%s</SCPDURL>" \
"        <controlURL>%s</controlURL>" \
"        <eventSubURL>%s</eventSubURL>" \
"      </service>" \
"	   <service>" \
"		 <serviceType>urn:schemas-upnp-org:service:ConnectionManager:1</serviceType>" \
"		 <serviceId>urn:upnp-org:serviceId:ConnectionManager</serviceId>" \
"		 <SCPDURL>%s</SCPDURL>" \
"		 <controlURL>%s</controlURL>" \
"		 <eventSubURL>%s</eventSubURL>" \
"	   </service>" \
"    </serviceList>" 

#define x_DMS_DESC_TAIL \
"  </device>" \
"</root>"

static char *s_Dms_CreateDescription (struct ushare_t *p)
{
	char *ret = NULL, *filename;
	C_DMS_CMI minfo;
	struct buffer_t *desc;

	bzero(&minfo, sizeof(C_DMS_CMI));
	desc = buffer_new();
	buffer_appendf(desc, x_DMS_DESC_HEAD);
	buffer_appendf(desc, x_DMS_DESC_DEVICE,		p->friendlyName, 
												p->manufacturer,
												p->manufacturerURL,
												p->modelDescription,
												p->modelName,
												p->modelNumber,
												p->modelURL,
												p->serialNumber,
												p->udn,
												"presentation.html");
	
  	if(p->icon && !dlna_guess_usb_media_profile(p->dlna, p->icon, &minfo))
  	{
		filename = strrchr(p->icon, '/');
		buffer_appendf(desc, x_DMS_DESC_ICON,	minfo.protocol_info->mime,
												minfo.res_width,
												minfo.res_height,
												24, /* minfo.color_depth, */
												filename);
  	}

	buffer_appendf(desc, x_DMS_DESC_SERVICE,	"/web/cds.xml",
												"/web/cds_control",
												"/web/cds_event",
												"/web/cms.xml",
												"/web/cms_control",
												"/web/cms_event");
	buffer_appendf(desc, x_DMS_DESC_TAIL);
	ret = strdup(desc->buf);
	buffer_free (desc);
	return ret;
}

static int init_upnp (struct ushare_t *p, int init_upnp_stack)
{
	char *description = NULL;
	int ret = -1;

	HT_DBG_FUNC_START(HT_MOD_DMS, HT_BIT_KEY, init_upnp_stack, 0);
	init_upnp_stack = 0;
	if (!p || !p->udn)
		goto s_EXIT;
#if 0
	description = dlna_dms_description_get (p->friendlyName,
											p->manufacturer,
											p->manufacturerURL,
											p->modelDescription,
											p->modelName,
											p->modelNumber,
											p->modelURL,
											p->serialNumber,
											p->udn,
											"presentation.html",
											"/web/cms.xml",
											"/web/cms_control",
											"/web/cms_event",
											"/web/cds.xml",
											"/web/cds_control",
											"/web/cds_event");
#else
	description = s_Dms_CreateDescription(p);
#endif
	HT_DBG_FUNC(0, description);
	ret --;
	if(!description)
		goto s_EXIT;

	p->port = UpnpGetServerPort();
	UpnpEnableWebserver (TRUE);

	ret = UpnpSetVirtualDirCallbacks (&virtual_dir_callbacks);
	if(ret != UPNP_E_SUCCESS)
	{
		HT_DBG_FUNC(ret, "UpnpSetVirtualDirCallbacks =");
		goto s_EXIT;
	}

	ret = UpnpAddVirtualDir (VIRTUAL_DIR);
	if(ret != UPNP_E_SUCCESS)
	{
		HT_DBG_FUNC(ret, "UpnpAddVirtualDir =");
		goto s_EXIT;
	}

	ret = UpnpRegisterRootDevice2 (UPNPREG_BUF_DESC, description, 0, 1, dms_callback_event_handler, NULL, &(p->dev));
	if(ret != UPNP_E_SUCCESS)
	{
		HT_DBG_FUNC(ret, "UpnpRegisterRootDevice2 1 =");
		goto s_EXIT;
	}

	ret = UpnpUnRegisterRootDevice (p->dev);
	if(ret != UPNP_E_SUCCESS)
	{
		HT_DBG_FUNC(ret, "UpnpUnRegisterRootDevice =");
		goto s_EXIT;
	}

	ret = UpnpRegisterRootDevice2 (UPNPREG_BUF_DESC, description, 0, 1, dms_callback_event_handler, NULL, &(p->dev));
	if(ret != UPNP_E_SUCCESS)
	{
		HT_DBG_FUNC(ret, "UpnpRegisterRootDevice2 2 =");
		goto s_EXIT;
	}

	ret = UpnpSendAdvertisement(p->dev, UPNP_MAX_AGE); //UPNP_MAX_AGE
	if(ret != UPNP_E_SUCCESS)
	{
		HT_DBG_FUNC(ret, "UpnpSendAdvertisement = ");
		goto s_EXIT;
	}

s_EXIT:
	if (description)
		free (description);
	HT_DBG_FUNC_END(ret, NULL);	
	return ret;
}

extern char *UpnpStackGetIfname(void);

void Dms_WaitLock(void)
{
	sem_wait(&upnp_lock);
}
void Dms_PostLock(void)
{
	sem_post(&upnp_lock);
}

static void s_Dms_Find_Str(json_object *json, char *key, char **p)
{
	const char *str;
	HT_DBG_FUNC_START(HT_MOD_DMS, HT_BIT_FEW, 0, *p);
	json_object *obj = json_object_object_get(json, key);
	if(obj && (str = json_object_get_string(obj)))
	{
		if(*p)
			free(*p);
		*p = strdup(str);
	}	
	HT_DBG_FUNC_END(0, *p);	
}
static void s_Dms_Find_Int(json_object *json, char *key, int *p)
{
	const char *str;
	HT_DBG_FUNC_START(HT_MOD_DMS, HT_BIT_FEW, *p, key);
	json_object *obj = json_object_object_get(json, key);
	if(obj && (str = json_object_get_string(obj)))
		*p = atoi(str);
	HT_DBG_FUNC_END(*p, 0);	
}


int Raw_Dms_Init(char *dms_name)
{
	int ret = -1;
	HT_DBG_FUNC_START(HT_MOD_DMS, HT_BIT_KEY,upnp_run, dms_name);
	HT_DBG_FUNC(0, "----ushare SUPPORT DLNA-----");
	HT_DBG_FUNC(0, UpnpStackGetIfname());

	ut = ushare_new ();
	if (!ut)
		goto s_EXIT;
	
	if(ut->interface)
		free(ut->interface);
	ut->interface = strdup (UpnpStackGetIfname());
	ut->dlna_enabled = true;
	s_init_dlna(ut);

	json_object *dms = dlna_json_tokener_parse(dms_name);
	if(dms)
	{
		s_Dms_Find_Str(dms,  "friendlyName",		&(ut->friendlyName));
		s_Dms_Find_Str(dms,  "manufacturer",		&(ut->manufacturer));
		s_Dms_Find_Str(dms,  "manufacturerURL", 	&(ut->manufacturerURL));
		
		s_Dms_Find_Str(dms,  "modelDescription",	&(ut->modelDescription));
		s_Dms_Find_Str(dms,  "modelName",			&(ut->modelName));
		s_Dms_Find_Str(dms,  "modelNumber", 		&(ut->modelNumber));
		s_Dms_Find_Str(dms,  "modelURL",			&(ut->modelURL));
		s_Dms_Find_Str(dms,  "serialNumber",		&(ut->serialNumber));
		s_Dms_Find_Str(dms,  "icon",				&(ut->icon));

		s_Dms_Find_Int(dms,  "object_total_limit",	&(ut->object_total_limit));
		json_object_put(dms);
	}

    if(!(ut->friendlyName) || !strcmp(coo_strtrim_ends(ut->friendlyName), ""))
    {
		char temp[128]="";
		sprintf(temp, "STB %s", UpnpGetServerIpAddress());
		ut->friendlyName = strdup(temp);
    }

	sem_init(&upnp_lock,0,1);

	Dms_PvrFileOperation_Init();
	
	dms_filetree_build (ut);
	Dms_ShareUsb_Init();	
	upnp_run = 1;
	ret = 0;
	
//extern	void Dms_SharePvr_Init (void);
//	Dms_SharePvr_Init();
	
s_EXIT:	
	HT_DBG_FUNC_END(ret, NULL);	
	return ret;
}

int Raw_Dms_Start(void)
{
	int ret = -1;
	HT_DBG_FUNC_START(HT_MOD_DMS, HT_BIT_KEY,upnp_run, 0);
	if( upnp_run )
	{
		sem_wait(&upnp_lock);

		if (ut->udn)
			free (ut->udn);
		ut->udn = create_udn (ut->interface);
		if (!ut->udn)
			goto err_1;

		if (init_upnp (ut, 0) < 0)
		{
			ushare_free (ut);
			ut = NULL;
			goto err_1;
		}
		ret = 0;
err_1:		
		sem_post(&upnp_lock);
	}
	HT_DBG_FUNC_END(ret, NULL);	
	return ret;
}

int Raw_Dms_Stop(void)
{
	HT_DBG_FUNC_START(HT_MOD_DMS, HT_BIT_KEY,upnp_run, 0);
	if( upnp_run )
	{
		sem_wait(&upnp_lock);
		UpnpUnRegisterRootDevice (ut->dev);
		sem_post(&upnp_lock);
	}
	HT_DBG_FUNC_END(0, NULL);
	return 0;
}

int Raw_Dms_SetNewName( char *new_name)
{
	HT_DBG_FUNC_START(HT_MOD_DMS, HT_BIT_KEY,upnp_run, new_name);
	if( upnp_run && new_name)
	{
		sem_wait(&upnp_lock);
		UpnpUnRegisterRootDevice (ut->dev);
		
		if(ut->friendlyName)
			free(ut->friendlyName);
		ut->friendlyName = strdup (new_name);
		
		if (init_upnp (ut, 0) < 0)
		{
			ushare_free (ut);
			ut = NULL;
		}

		sem_post(&upnp_lock);
	}	
	HT_DBG_FUNC_END(0, new_name);
	return 0;
}


int Raw_Dms_AddPvrItem(char *path, char *itemName)
{
	int ret=-3;
	
	HT_DBG_FUNC_START(HT_MOD_DMS, HT_BIT_KEY,upnp_run, itemName);
	if( upnp_run )
	{
		sem_wait(&upnp_lock);
//		ret = content_remove_pvr_item(ut, path, itemName);
//		ret = content_add_pvr_item(ut, path, itemName);
		sem_post(&upnp_lock);
	}
	HT_DBG_FUNC_END(ret, 0);
	path = NULL;
	itemName = NULL;
	return ret;
}

int Raw_Dms_RemovePvrItem(char *path, char *itemName)
{
	int ret = -3;
	
	HT_DBG_FUNC_START(HT_MOD_DMS, HT_BIT_KEY,upnp_run, itemName);
	if( upnp_run )
	{
		sem_wait(&upnp_lock);
//		ret = content_remove_pvr_item(ut, path, itemName);
		sem_post(&upnp_lock);
	}
	HT_DBG_FUNC_END(ret, 0);
	path = NULL;
	itemName = NULL;
	return ret;
}

int Raw_Dms_FindFolderShared(char *fullpath)
{
	int ret = 0;
	
	HT_DBG_FUNC_START(HT_MOD_DMS, HT_BIT_KEY,upnp_run, fullpath);
	if( upnp_run )
	{
		sem_wait(&upnp_lock);
//		ret = content_find_folder(ut, fullpath);
		sem_post(&upnp_lock);
	}
	HT_DBG_FUNC_END(ret, 0);
	fullpath = NULL;
	return ret;
}

int Raw_Dms_AddFolderShared(char *fullpath, char *title, int isPvr)
{
	int ret=-3;
	
	HT_DBG_FUNC_START(HT_MOD_DMS, HT_BIT_KEY,upnp_run, fullpath);
	if( upnp_run )
	{
		sem_wait(&upnp_lock);
//		ret = content_add_folder(ut, fullpath, title, isPvr);
		sem_post(&upnp_lock);
	}
	HT_DBG_FUNC_END(ret, title);
	fullpath = NULL;
	title = NULL;
	isPvr = 0;
	return ret;
}

int Raw_Dms_RemoveFolderShared(char *fullpath)
{
	int ret = -3;
	
	HT_DBG_FUNC_START(HT_MOD_DMS, HT_BIT_KEY,upnp_run, fullpath);
	if( upnp_run )
	{
		sem_wait(&upnp_lock);
//		ret = content_remove_folder(ut, fullpath);
		sem_post(&upnp_lock);
	}
	HT_DBG_FUNC_END(ret, 0);
	fullpath = NULL;
	return ret;
}

int Raw_Dms_SetStoppingHttp(int mode)
{
	int ret = -3;
	
	HT_DBG_FUNC_START(HT_MOD_DMS, HT_BIT_KEY,upnp_run, 0);
	if( upnp_run )
	{
		sem_wait(&upnp_lock);
        ut->stop_http = mode;
		sem_post(&upnp_lock);
	}
	HT_DBG_FUNC_END(mode, 0);
	return ret;
}


